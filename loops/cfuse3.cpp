/* Copyright (c) 2019 by NEC Corporation
 * This file is part of ve-jit */
/** \file
 * fuse2.cpp shows how to fuse 2 loops into vectorized index calculations.
 * It is just a basic test of the idea (verifying correctness).
 *
 * Also does pseudo-code unroll and no_unroll demos
 *
 * \todo "fuse2.cpp -all" to run a long battery of tests.
 *
 * \sa xxx.cpp for VE assembler output, introducing other header deps.
 */
#include "../fuseloop.hpp"
#include "exechash.hpp"
#include "../vechash.hpp"
#include "../stringutil.hpp"
#include "../throw.hpp" // THROW(stuff to right of cout<<), adding fn and line number
#include "../cblock.hpp" // Now we are producing 'C + intrinsics' code, for clang

#include <functional>
#include <list>
#include <vector>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <algorithm> // ve finds std::max here!
#include <unordered_set>
#include <map>
#include <array>

#include <cstring>
#include <cstddef>
#include <cassert>

#ifndef FOR
#define FOR(I,VL) for(int I=0;I<VL;++I)
#endif

using namespace std;
using namespace loop;
using namespace cprog;

// /** scope init for AsmFmtCols */
// typedef std::list<std::pair<std::string,std::string>> AsmScope;

#include "divide_by_constants_codegen_reference.c"
#if 0 // C++17:
template<typename... Args>
std::string join_sep[](char const sep, Args&&... args){
    std::ostringstream oss;
    std::string ret( ((oss << args << sep), ...).str() );
    //               ---------------------
    //               C++17 fold expression, expands to
    //               (oss<<arg1<<sep),(oss<<arg2<<sep), ...
    if(!ret.empty()) ret.pop_back();
    return ret;
}
#endif

typedef uint32_t u32;
typedef int32_t s32;
typedef uint64_t u64;
/* required for magic constant generation */
u32 ulog2(u32 v) {
    u32 r, shift;
    r =     (v > 0xFFFF) << 4; v >>= r;
    shift = (v > 0xFF  ) << 3; v >>= shift; r |= shift;
    shift = (v > 0xF   ) << 2; v >>= shift; r |= shift;
    shift = (v > 0x3   ) << 1; v >>= shift; r |= shift;
    r |= (v >> 1);
    return r;
}

struct fastdiv {
    u32 mul;
    u32 add;
    s32 shift;
    u32 _odiv;  /* save original divisor for modulo calc */
};
/* generate constants for implementing a division with multiply-add-shift */
void fastdiv_make(struct fastdiv *d, u32 divisor) {
    u32 l, r, e;
    u64 m;

    d->_odiv = divisor;
    // Modifed [ejk]
    if( positivePow2(divisor) ){
        d->mul = 1U;
        d->add = 0U;
        d->shift = positivePow2Shift(divisor);
        assert( ulog2(divisor) == (u32)d->shift );
        return;
    }
    l = ulog2(divisor);
    if (divisor & (divisor - 1)) {
        m = 1ULL << (l + 32);
        d->mul = (u32)(m / divisor);
        r = (u32)m - d->mul * divisor;
        e = divisor - r;
        if (e < (1UL << l)) {
            ++d->mul;
            d->add = 0;
        } else {
            d->add = d->mul;
        }
        d->shift = l;
    } else {
        if (divisor == 1) {
            d->mul = 0xffffffff;
            d->add = 0xffffffff;
            d->shift = 0;
        } else {
            d->mul = 0x80000000;
            d->add = 0;
            d->shift = l-1;
        }
    }
}

void other_fastdiv_methods(int const jj);

// NOTE: we do a ">>FASTDIV_C", which we can't just elide on Aurora, even if FASTDIV_C==16

/* Suppose for i:(0,ii){ for j:(o,jj) {} gets
 * gets vector indices \c a[] and \c b[]
 * with preferred vector length \c vlen.
 * How do we set the initial and induced states of a,b?
 * How do we induce the next vector state?
 * How do get the final state (possible shorter vlen)?
 *
 * Here I assume we will generate a[vlen] and b[vlen]
 * separately as i/u64 vectors.  Other options might pack
 * them in a single register as up/lo i/u32.
 * ? or maybe double-length 2*vlen of i/u32 (is this possible?)
 */
void test_vloop2(Lpi const vlen, Lpi const ii, Lpi const jj){ // for r in [0,h){ for c in [0,w] {...}}
    VecHash2 vhash(vlen);
    int const verbose=1;
    assert( vlen > 0 );
    register uint64_t iijj = (uint64_t)ii * (uint64_t)jj;
    cout<<"test_vloop2( vlen="<<vlen<<" loops 0.."<<ii<<" 0.."<<jj<<" iijj="<<iijj;

    register int vl = vlen;
    //if (cnt+vl > iijj) vl = iijj - cnt;  // we assume ii and jj loops begin at 0
    if ((uint64_t)vl > iijj) vl = iijj;
    int const vl0 = vl; // debug
    int const nloop = (iijj+vl-1) / vl;    // div_round_up(iijj,vl)
    cout<<" vl="<<vl<<" nloop="<<nloop<<endl;
    assert( (nloop > 1) == ((uint64_t)vl < iijj) );
    assert( vl0 > 0 );
    assert( ii > 0 );
    assert( jj > 0 );

    cout<<"Verify-------"<<endl;
    // generate reference index outputs
    std::vector<Vab> vabs = ref_vloop2(vlen, ii, jj, 0/*verbose*/);
    assert( vabs.size() > 0 );
    assert(vabs.size() == (size_t)(((ii*jj) +vlen -1) / vlen));
    if(verbose>=2) cout<<"   vl="<<vl<<"   ii="<<ii<<"   jj="<<jj
        <<"   iijj="<<iijj<<" vabs.size() = "<<vabs.size()<<endl;
    // Have reference vabs vectors. Now we try induction way.
    // 1. initialize: could copy vabs[0] from const data storage, or...
    //   - generate from seq + divmod.
    //   - 2-loop induction uses 3 scalar registers:
    //     - \c cnt 0.. \c iijj, and \c vl (for jit, iijj is CCC (compile-time-const))
    //     - get final \c vl from cnt, vl and iij)

    if(verbose>=1) other_fastdiv_methods(jj);

    // various misc precalculated consts and declarations.
    VVlpi a(vl), b(vl), bA(vl), bM(vl), bD(vl), aA(vl), sq(vl);
    VVlpi a0(vl), b0(vl), x(vl), y(vl);
    int iloop = 0; // mostly for debug checks, now;
    //Ulpi jj_mod_inverse_lpi   = mod_inverse((Ulpi)jj);
    //Uvlpi jj_mod_inverse_Vlpi = mod_inverse((Uvlpi)jj);
    // bA and bD are used when:
    //   iijj > vl && jj%vl!=0
    // sq is used when:
    //   iloop==0:   jj>1 && vl>=jj
    //   iloop >0:   jj%vl==0 && (special: iloop can be >= (jj/vl))
    uint64_t const jj_M = computeM_uB(jj); // for fastdiv_uB method
    cout<<" jj="<<jj;
    int jj_shift=0;
    Vlpi jj_minus_1 = 0;
    if( positivePow2(jj) ){
        jj_shift = positivePow2Shift((uint32_t)jj);
        jj_minus_1 = jj - 1;
        assert( (uint64_t)1<<jj_shift == (uint64_t)jj );
        cout<<" power of two shift is "<<jj_shift<<"    mask is "<<jj_minus_1;
    }else{
        cout<<" jj_M="<<(void*)(intptr_t)jj_M<<" shift="<<FASTDIV_C;
    }
    cout<<endl;

#if 0
    // NB: common operation is divmod(v,s,vM,vD) : v--> v%s, v/s,
    //     which has some optimizations for nice values of jj.
    // FASTDIV_C++14: &vl=std::as_const(vl)
    auto v_divmod_vs = [&vl,&jj,&jj_M](/* in*/ VVlpi const& a, Vlpi const d, /*out*/ VVlpi& div, VVlpi& mod){
        assert( (Ulpi)jj < FASTDIV_SAFEMAX ); FOR(i,vl) assert( (Uvlpi)a[i] <= FASTDIV_SAFEMAX );
        FOR(i,vl) div[i] = jj_M * a[i] >> FASTDIV_C;
        FOR(i,vl) mod[i] = a[i] - div[i]*jj;
    };
#endif
    // have_FOO and cnt_FOO : FOO register usage condition and actual use count
    bool const have_vl_over_jj = nloop>1 && vl0%jj==0;
    bool const have_bA_bD = nloop>1 && vl%jj!=0 && jj%vl!=0;
    bool const have_jjMODvl_reset      = (vl0%jj!=0 && jj%vl0==0 && nloop >jj/vl0); // case 'g'
    bool const have_jjMODvl = (vl0%jj!=0 && jj%vl0==0 && nloop>=jj/vl0);
    if( 1 || have_jjMODvl ){
        cout<<" nloop="<<nloop<<" jj/vl0="<<jj/vl0<<endl;
        if( have_jjMODvl ) assert( jj/vl0 > 1 );
        if( have_jjMODvl_reset ) assert( have_jjMODvl );
    }
    bool const have_sq = (jj!=1 && jj<vl0)                              // when iloop==0
        || have_jjMODvl_reset;                                                  // o/w
    bool const have_jj_shift = (jj!=1 && jj<vl0 && positivePow2(jj))    // when iloop==0
        || (nloop>1 && vl0%jj!=0 && jj%vl0!=0 && positivePow2(jj));     // o/w
    bool const have_jj_M = (jj>1 && jj<vl0 && !positivePow2(jj))        // when iloop==0
        || (nloop>1 && vl0%jj!=0 && jj%vl0!=0 && !positivePow2(jj));    // o/w

    int cnt_vl_over_jj=0, cnt_bA_bD=0, cnt_sq=0, cnt_jj_shift=0, cnt_jj_M=0;
    int cnt_jjMODvl=0, cnt_jjMODvl_reset=0;

    Vlpi const vl_over_jj = have_vl_over_jj? vl0/jj: 0;
    Vlpi const jj_over_vl = (have_jjMODvl_reset ? jj/vl0: 0);
    assert( !(have_vl_over_jj && have_jjMODvl) ); // never need both these constants

    int const b_period_max = 8; // how many regs can you spare?
    //int const b_period = unroll_suggest( vl, jj, b_period_max );
    auto u = unroll_suggest( vl,ii,jj, b_period_max, 0/*verbose*/ );
    // This is pinned at [max] vl, even if it may be "inefficient".
    //auto uAlt =
    unroll_suggest(u);

    cout<<" Using "<<u.suggested<<"("<<(int)u.suggested<<") for vl,ii,jj="
        <<vl<<","<<ii<<","<<jj<<endl;

    register uint64_t cnt = 0UL;
    for( ; cnt < iijj; cnt += vl )
    {
#define VL_UP0 0
#define VL_UP 1
        //cout<<"cnt "<<cnt<<" iloop "<<iloop<<" ii "<<ii<<" jj "<<jj<<endl;
        if (iloop == 0){
            if(nloop==1) assert(have_vl_over_jj==0);
            // now load the initial vector-loop registers:
            // sq[i] and jj are < FASTDIV_SAFEMAX, so we can avoid % and / operations
            FOR(i,vl) sq[i] = i;       // vseq_v
            if( jj==1 ){
                if(verbose){cout<<" a";cout.flush();};
                FOR(i,vl) a[i] = i;    // sq/jj
                FOR(i,vl) b[i] = 0;    // sq%jj
                assert(have_bA_bD==0); assert(have_sq==0); assert(have_jj_shift==0);
            }else if(jj>=vl){
                if(verbose){cout<<" b";cout.flush();}
                FOR(i,vl) a[i] = 0;    // sq < vl, so sq/jj < 1
                FOR(i,vl) b[i] = i;
                if(nloop<=1) {assert(have_bA_bD==0); assert(have_sq==0); assert(have_jj_shift==0); }
            }else if( positivePow2(jj) ){
                if(verbose){cout<<" c";cout.flush();}
                // 2 ops (shr, and)
                FOR(i,vl) a[i] = (sq[i] >> jj_shift);  // bD = bA / jj; div_vsv
                FOR(i,vl) b[i] = (sq[i] & jj_minus_1); // bM = bA % jj; mod_vsv
                if(nloop<=1) assert(have_bA_bD==0); assert(have_sq==1); assert(have_jj_shift==1);
                ++cnt_sq; ++cnt_jj_shift;
            }else{
                if(verbose){cout<<" d";cout.flush();}
                // 4 int ops (mul,shr, mul,sub)
                //v_divmod_vs( sq, jj, /*sq[]/jj*/a, /*sq[]%jj*/b );
                FOR(i,vl) a[i] = jj_M * sq[i] >> FASTDIV_C;
                FOR(i,vl) b[i] = sq[i] - a[i]*jj;
                //  OK since sq[] and jj both <= FASTDIV_SAFEMAX [(1<<21)-1]
                assert( (uint64_t)jj+vl <= (uint64_t)FASTDIV_SAFEMAX );
                // use mul_add_shr (fastdiv) approach if jj+vl>FASTDIV_SAFEMAX (1 more vector_add_scalar)
                if(nloop<=1) assert(have_bA_bD==0); assert(have_sq==1); assert(have_jj_shift==0);
                ++cnt_sq; ++cnt_jj_M;
            }
#if VL_UP0
            // Note: vl reduction must take place AFTER above use of "long" vl
            if( cnt + vl > iijj ){ // last time might have reduced vl
                vl = iijj - cnt;
                cout<<" vl reduced for last loop to "<<vl<<endl;
            }
#endif
        }else{
            assert( nloop > 1 );
#if VL_UP==1
            // if careful about vl (possible lower value for last iter) vs vl0 can update here:
            if( cnt + vl > iijj ){ // last time might have reduced vl
                vl = iijj - cnt;
                cout<<" vl reduced for last loop to "<<vl<<endl;
            }
#endif
            // 2. Induction from a->ax, b->bx
            if(vl0%jj == 0){  // avoid div,mod -----1 vec op
                // this includes cases with b_period==1 and high nloops
                if(verbose){cout<<" e";cout.flush();}
                FOR(i,vl) a[i] = a[i] + vl_over_jj;
                ++cnt_vl_over_jj; assert(have_vl_over_jj); assert(!have_bA_bD);
            }else if(jj%vl0 == 0 ){  // -------------1 or 2 vec op (conditional)
                // unroll often nice w/ have_b_period (with maybe more regs)
                assert( have_bA_bD==0); assert(have_jj_shift==0); assert(have_vl_over_jj==0);
                assert( jj > vl0 ); assert( jj/vl0 > 1 ); assert( have_jjMODvl );
                if( !have_jjMODvl_reset ){
                    // Note: this case should also be a "trivial" case for unroll suggestion
                    if(verbose){cout<<" f";cout.flush();}
                    FOR(i,vl) b[i] = b[i] + vl0;
                    ++cnt_jjMODvl; assert( have_jjMODvl );
                }else{
                    // This case is potentially faster with a partial precalc unroll
                    // The division should be done with compute_uB
                    assert( have_jjMODvl && have_jjMODvl_reset );
                    Lpi easy = iloop % jj_over_vl;       // scalar cyclic mod
                    //Lpi easy = fastmod_uB( iloop, jj_over_vl_M, jj_over_vl );
                    // #pragma..unroll(jj/vl) could be branchless
                    // can be optimized further into 3 minimal-op cases
                    if( easy ){                         // bump b[i], a[i] unchanged
                        if(verbose){cout<<" f";cout.flush();}
                        FOR(i,vl) b[i] = b[i] + vl0;
                        ++cnt_jjMODvl; assert( have_jjMODvl );
                    }else{                              // RESET b[i], bD[i]==1
                        if(verbose){cout<<" g";cout.flush();}
                        FOR(i,vl) b[i] = sq[i];
                        FOR(i,vl) a[i] = a[i] + 1;
                        ++cnt_sq; assert( have_sq==1 );
                        ++cnt_jjMODvl_reset;
                    }
                }
                if( have_jjMODvl_reset ) assert( have_jjMODvl );
                assert( !have_bA_bD );
            }else if( positivePow2(jj) ){ // ------4 vec ops (add, shr, and, add)
                if(verbose){cout<<" h";cout.flush();}
                assert( vl0%jj != 0 ); assert( jj%vl0 != 0 ); assert(have_bA_bD==1);
                assert(have_jj_shift==1); assert(have_vl_over_jj==0);
                // no...assert(have_sq==(jj>1&&jj<vl));
                FOR(i,vl) bA[i] = vl0 + b[i];            // bA = b + vl0; add_vsv
                FOR(i,vl) bD[i] = (bA[i] >> jj_shift);  // bD = bA / jj; div_vsv
                FOR(i,vl) b [i] = (bA[i] & jj_minus_1); // bM = bA % jj; mod_vsv
                FOR(i,vl) a [i] = a[i] + bD[i]; // aA = a + bD; add_vvv
                ++cnt_bA_bD; ++cnt_jj_shift; assert( have_bA_bD );
            }else{ // div-mod ---------------------6 vec ops: add (mul,shr) (mul,sub) add
                assert( jj+vl < (1<<21) );
                if(verbose){cout<<" i";cout.flush();}
                if(0){
                    FOR(i,vl) bA[i] = vl0 + b[i];           // add_vsv
                    FOR(i,vl) bD[i] = ((jj_M*bA[i]) >> FASTDIV_C);  // fastdiv_uB   : mul_vvs, shr_vs
                    FOR(i,vl) b [i] = bA[i] - bD[i]*jj;     // long-hand    : mul_vvs, sub_vvv
                    FOR(i,vl) a [i] = a[i] + bD[i];         // add_vvv
                }else if(0){ // as VE alementary ops
                    // Note:
                    // Can ">>32" can be avoided for some jj if we do a u32*u32 multiply.
                    // (Current libdivide and fastdiv suggests not,  check dev code, though)
                    FOR(i,vl) bA[i] = vl0 + b[i];

                    FOR(i,vl) bD[i] = jj_M * bA[i]; // dep -1 simul if bD ?=? bD_prev + jj_M*vl, but would an extra op later
                    FOR(i,vl) bD[i] = bD[i] >> FASTDIV_C;  // dep -1

                    FOR(i,vl) y[i] = bD[i]*jj;     // dep -1
                    FOR(i,vl) b[i] = bA[i] - y[i]; // dep -1

                    FOR(i,vl) a[i] = a[i] + bD[i];  // dep -3
                }else { // re-ordered
                    // Note:
                    // Can ">>32" can be avoided for some jj if we do a u32*u32 multiply.
                    // (Current libdivide and fastdiv suggests not,  check dev code, though)
                    FOR(i,vl) b [i] = vl0 + b[i];
                    FOR(i,vl) bD[i] = jj_M * b [i]; // dep -1 simul if bD ?=? bD_prev + jj_M*vl, but would an extra op later
                    FOR(i,vl) bD[i] = bD[i] >> FASTDIV_C;  // dep -1
                    FOR(i,vl) y[i] = bD[i]*jj;     // dep -1
                    FOR(i,vl) a[i] = a[i] + bD[i];  // dep -2
                    FOR(i,vl) b[i] = b [i] - y[i]; // dep -2
                }
                ++cnt_bA_bD; ++cnt_jj_M;
                assert(have_bA_bD); assert(have_sq==(jj>1&&jj<vl)); assert(!have_jj_shift);
                assert(!have_vl_over_jj); assert( jj+vl < (int)FASTDIV_SAFEMAX );
            }
#if VL_UP==2
            // if confused about vl vs vl0, this is safe (but induce might use too-long vl0)
            if( cnt + vl > iijj ){ // last time might have reduced vl
                vl = iijj - cnt;
                cout<<" vl reduced for last loop to "<<vl<<endl;
            }
#endif
        }
#if !VLUP0 && !VL_UP
        // Note: vl reduction must take place AFTER above use of "long" vl
        if( cnt + vl > iijj ){ // last time might have reduced vl
            vl = iijj - cnt;
            cout<<" vl reduced for last loop to "<<vl<<endl;
        }
#endif

        if(verbose>=1){
            int const n=8; // output up-to-n [ ... [up-to-n]] ints
            int const bignum = std::max( ii, jj );
            int const wide = 1 + (bignum<10? 1: bignum <100? 2: bignum<1000? 3: 4);
            cout<<"a["<<vl<<"]="<<vecprt(n,wide,a,vl)<<endl;
            cout<<"b["<<vl<<"]="<<vecprt(n,wide,b,vl)<<endl;
        }

        assert( vl == vabs[iloop].vl );
        FOR(i,vl) assert( a[i] == vabs[iloop].a[i] );
        FOR(i,vl) assert( b[i] == vabs[iloop].b[i] );
        // Alt. is a hash-value test:
        //assert( Vab::rehash(a,b,vl,0) == vabs[iloop].hash );
        //assert( Vab::rehash(a,b,vl) == vabs[iloop].hash );
        assert( vhash.hash_combine(a.data(),b.data(),vl) == vabs[iloop].hash );

        ++iloop; // just for above debug assertions
        //cout<<" next loop??? cnt+vl="<<cnt+vl<<" iijj="<<iijj<<endl;
#undef FOR
    }
    cout<<" Yay! induction formulas worked! iloop,nloop="<<iloop<<","<<nloop<<endl;
    assert( cnt == iijj );
    assert( nloop == iloop );
    //if( nloop > 1 && vl%jj!=0 && jj%vl==0 ) assert( count_special > 0 );
    assert( have_vl_over_jj     == (cnt_vl_over_jj    > 0) );
    assert( have_bA_bD          == (cnt_bA_bD         > 0) );
    assert( have_sq             == (cnt_sq            > 0) );
    assert( have_jj_shift       == (cnt_jj_shift      > 0) );
    assert( have_jj_M           == (cnt_jj_M          > 0) );
    assert( have_jjMODvl        == (cnt_jjMODvl       > 0) ); // old "special" count, case 'g' needed
    assert( have_jjMODvl_reset  == (cnt_jjMODvl_reset > 0) ); // old "special" count, case 'g' needed
}

// for r in [0,h){ for c in [0,w] {...}
void test_vloop2_no_unrollX(Lpi const vlen, Lpi const ii, Lpi const jj, int const opt_t){
    ostringstream oss; // reusable allocation, for CBLK and OSSFMT macros
    ExecHash tr;
    //CodeGenAsm cg;
    //std::string code;
    // What vl should we really use? vlen or an alternative (lower) length?
    register int vl = vlen;
    int const b_period_max = 8; // how many regs can you spare?

    // This is pinned at [max] vl, even if it may be "inefficient".
    auto u = unroll_suggest( vl,ii,jj, b_period_max );

    auto uAlt = unroll_suggest(u);  // suggest an alternate nice vector length, sometimes.
    if(opt_t==3){ // we FORCE the alternate strategy
        if(u.vll) // equiv uAlt.suggested != UNR_UNSET
        {
            assert( uAlt.suggested != UNR_UNSET );
            cout<<" (forcing alt. strategy)"<<endl;
            //vl = u.vll; unroll_suggest(u);
            u = uAlt;
            vl = u.vl;
        }else{
            cout<<" (no really great alt.strategy)"<<endl;
        }
    }
    int const vl0 = vl;
    cout<<__FUNCTION__<<" using vl0="<<vl<<endl;

    {
        VecHash2 vhash(vl);
        int verbose=1;
        assert( vl > 0 );
        register uint64_t iijj = (uint64_t)ii * (uint64_t)jj;
        cout<<"fuse2 -t "<<setw(4)<<vl<<" "<<setw(4)<<ii<<" "<<setw(4)<<jj
            <<"   # for(0.."<<ii<<") for(0.."<<jj<<") --> a[VL],b[VL] for VL<="<<vl<<endl;

        //if (cnt+vl > iijj) vl = iijj - cnt;  // simplifies for cnt=0
        if ((uint64_t)vl > iijj) vl = iijj;
        int const nloop = (iijj+vl-1) / vl;    // div_round_up(iijj,vl)
        assert( (nloop > 1) == ((uint64_t)vl < iijj) );
        assert( vl0 > 0 );
        assert( ii > 0 );
        assert( jj > 0 );

        cout<<" Using "<<u.suggested<<" for vl,ii,jj="
            <<vl<<","<<ii<<","<<jj<<" unroll("<<u.unroll<<")"
            <<" [Alt vll="<<u.vll<<"]"<<endl;
        // Include the suggested strategy into the exechash, for future
        // productions that do unrolling (want better coverage)
        {
            std::ostringstream oss;
            oss<<"strategy "<<u.suggested;
            tr+=oss.str();
        }

        //cout<<"Verify-------"<<endl;
        // generate reference index outputs
        std::vector<Vab> vabs = ref_vloop2(vl, ii, jj, 0/*verbose*/);
        assert( vabs.size() > 0 );
        assert(vabs.size() == (size_t)(((ii*jj) +vl -1) / vl));
        if(verbose>=2) cout<<"   vl="<<vl<<"   ii="<<ii<<"   jj="<<jj
            <<"   nloop="<<nloop<<"   iijj="<<iijj
                <<" vabs.size() = "<<vabs.size()<<endl;
        // Have reference vabs vectors. Now we try induction way.
        // 1. initialize: could copy vabs[0] from const data storage, or...
        //   - generate from seq + divmod.
        //   - 2-loop induction uses 3 scalar registers:
        //     - \c cnt 0.. \c iijj, and \c vl (for jit, iijj is CCC (compile-time-const))
        //     - get final \c vl from cnt, vl and iij)
        //     - new: interpret cnt as "remain", going from iijj to one

#define FOR(I,VL) for(int I=0;I<VL;++I)

        // various misc precalculated consts and declarations.
        VVlpi a(vl), b(vl);                 // calculated loop index vectors
        VVlpi bA(vl), bD(vl), sq(vl);       // internal vectors
        // bA and bD are used when:
        //   iijj > vl && jj%vl!=0
        // sq is used when:
        //   iloop==0:   jj>1 && vl>=jj
        //   iloop >0:   jj%vl==0 && (special: iloop can be >= (jj/vl))
        assert( nloop >= 1 );
        uint64_t const jj_M = computeM_uB(jj); // for fastdiv_uB method
        int jj_shift=0;
        Vlpi jj_minus_1 = 0;
        if( positivePow2(jj) ){
            jj_shift = positivePow2Shift((uint32_t)jj);
            jj_minus_1 = jj - 1;
            assert( (uint64_t)1<<jj_shift == (uint64_t)jj );
            cout<<" power of two shift is "<<jj_shift<<"    mask is "<<jj_minus_1;
        }else{
            cout<<" jj_M="<<(void*)(intptr_t)jj_M<<" shift="<<FASTDIV_C;
        }

#if 0
        // NB: common operation is divmod(v,s,vM,vD) : v--> v%s, v/s,
        //     which has some optimizations for nice values of jj.
        // C++14: &vl=std::as_const(vl)
        auto v_divmod_vs = [&vl,&jj,&jj_M](/* in*/ VVlpi const& a, Vlpi const d, /*out*/ VVlpi& div, VVlpi& mod){
            assert( (Ulpi)jj < FASTDIV_SAFEMAX ); FOR(i,vl) assert( (Uvlpi)a[i] <= FASTDIV_SAFEMAX );
            FOR(i,vl) div[i] = jj_M * a[i] >> FASTDIV_C;
            FOR(i,vl) mod[i] = a[i] - div[i]*jj;
        };
#endif

        // Using immediate values decreases register usage.
        // Even though VE asm ops can use certain values as immediate data,
        // Want auto mechanism to decide whether immediate is possible.
        //
        // have_FOO and cnt_FOO : FOO register usage condition and actual use count
        bool const have_vl_over_jj = nloop>1 && vl0%jj==0;
        bool const have_bA_bD = nloop>1 && vl%jj!=0 && jj%vl!=0;
        bool const have_jjMODvl_reset      = (vl0%jj!=0 && jj%vl0==0 && nloop >jj/vl0); // case 'g'
        bool const have_jjMODvl = (vl0%jj!=0 && jj%vl0==0 && nloop>=jj/vl0);
        if( 1 || have_jjMODvl ){
            cout<<" jj/vl0="<<jj/vl0<<endl;
            if( have_jjMODvl ) assert( jj/vl0 > 1 );
            if( have_jjMODvl_reset ) assert( have_jjMODvl );
        }
        bool const have_sq_init_block = jj!=1 && jj<vl0;
        bool const have_sq_indu_block = have_jjMODvl_reset;
        bool const have_sq = have_sq_init_block                             // when iloop==0
            || have_sq_indu_block;                                          // o/w
        bool const have_jj_shift = (jj!=1 && jj<vl0 && positivePow2(jj))    // when iloop==0
            || (nloop>1 && vl0%jj!=0 && jj%vl0!=0 && positivePow2(jj));     // o/w
        bool const have_jj_M = (jj>1 && jj<vl0 && !positivePow2(jj))        // when iloop==0
            || (nloop>1 && vl0%jj!=0 && jj%vl0!=0 && !positivePow2(jj));    // o/w

        int cnt_vl_over_jj=0, cnt_bA_bD=0, cnt_sq=0, cnt_jj_shift=0, cnt_jj_M=0;
        int cnt_jjMODvl=0, cnt_jjMODvl_reset=0, cnt_use_iijj=0;

        Vlpi const vl_over_jj  = have_vl_over_jj? vl0/jj: 0;
        Vlpi const jj_over_vl  = have_jjMODvl_reset? jj/vl0: 0;
        Vlpi tmod       = have_jjMODvl_reset? 0: 777; // used if have_jjMODvl_reset
        //uint64_t const jj_over_vl_M = (have_jjMODvl_reset? computeM_uB(jj_over_vl): 0);
        assert( !(have_vl_over_jj && have_jjMODvl_reset) ); // never need both these constants
        assert( !(have_jj_M && have_jjMODvl_reset) ); // never both
        //assert( !(have_jj_M && have_vl_over_jj) ); // HAPPENS ex: vl,ii,jj=9,4,3

        register uint64_t cnt = iijj; // NEW: iijj to one [i.e. remain], rather than [0..iijj)

#if 0
        AsmFmtCols fd,fp,fi,fk,fl,fz;
        fp.com("fuse2 presets").setParent(&fd);
        fi.setParent(&fp);
        fk.setParent(&fp);
        fl.setParent(&fp);
        fz.setParent(&fp);
#else
        Cunit pr("cfuse2","C",1);
        CBLOCK_SCOPE(cfuse2,"",pr,pr.root);
        auto& fd = cfuse2["defines"];
        auto& fp = fd["preset"]; fp>>"//cfuse2 presets";
        auto& fi = fd["induce"]; // really?
        //auto& fk = fd["kernel"];
        CBLOCK_SCOPE(fk,"",pr,fd);
        fk  >>"// Even though we replace a 'C' for()for(), we use goto style"
            >>"// so that: (i) initialization and induction code is separated,"
            >>"// (ii) loop exit test can be optimized in some cases, and"
            >>"// (iii) we have a ** single insertion point ** for kernel code"
            >>"// (iv) avoid branches in or out of loop scope;"
            >>"// (v) this is the natural structure for the assembler version (fuse2.cpp)"
            >>"//"
            >>"// ... but this loop style cannot be unrolled automatically"
            >>"//     (should try out the loop option with cnt==0 as special"
            >>"//      case because maybe the compiler will peel the loop"
            >>"//      automatically --> more readable JIT code !!! )"
            ;
        //
        //   gcc apparently has loop splitting,
        //   but llvm might not:
        //
        //   http://lists.llvm.org/pipermail/llvm-bugs/2018-April/063932.html
        //
        //   "[llvm-bugs] [Bug 36990] New: LLVM is missing loop splitting"
        //
        // with little activity in the last year.
        //

        auto& fl = fd["loopend"];
        auto& fz = fd["last"];
#endif

        // important: vechash puts a scope onto fp, so later code snippets
        // fk,fl,fz should not "miss" any current scopes. (i.e. it is up
        // to us to create an appropriate tree of scopes).

        // local labels:
        string fusename;
        {
            std::ostringstream oss;
            oss<<"fuse2_"<<vl<<"_"<<ii<<"_"<<jj;
            fusename = oss.str();
        }
        cfuse2
            //.def("BASE", "L_"+fusename+"_BASE")
            .def("INDUCE", "L_"+fusename+"_INDUCE")
            .def("KERNEL_BLOCK", "L_"+fusename+"_KERNEL_BLOCK")
            .def("vl0", vl0)
            .def("FASTDIV_C",FASTDIV_C)
#if 0
            .def("FASTDIV_"+jitdec(jj)+"(VR_IN,VR_DIV,VR_REM)", "do{ \\\n"
                    "    __vr tmp1 =_ve_vmulul_vsv(VR_IN, FASTDIV_MUL_"+jitdec(jj)+");\\\n"
                    "    VR_DIV = _ve_vsrl_vsv(tmp1, "+jitdec(FASTDIV_C)+"); \\\n"
                    "    __vr tmp2 = _ve_vmulul_vsv(VR_DIV, "+jitdec(jj)+"); \\\n"
                    "    VR_REM = _ve_vsubul_vvv(VR_IN, tmp2); \\\n"
                    "}while(0)"
                )
#endif
            >>"int64_t vl;"
            >>"int64_t ii;"
            >>"int64_t jj;"
            >>"int64_t base;"
            ;

        { // illustrate use of auto-register assignments,
            // even before the scope is officially defined in an AsmFmtCols
            // UTILITY LAMBDAS
            // pre-assign some registers to fd "defines" scope
            // I don't care about speed, for now.

            // Allocations // const registers
            if( have_sq )   fd>>"int64_t sq;";
            // Allocations // state registers
            // Note that many of these are optional, so register assignments
            // for different kernels may be ... different.
#define SAVE_RESTORE_VLEN 1
#define HASH_KERNEL 1
            if(nloop>1)                     
            if(nloop>1 && (iijj%vl0) )      fd>>"int64_t vl;"; //block_alloc_scalar("vl");
            if(SAVE_RESTORE_VLEN)           fd>>"//int64_t vl_save;"; //block_alloc_scalar("vl_save");
            if(HASH_KERNEL) fd>>"//int64_t reg_hash;"; //ve_propose_reg("reg_hash",block,fd,SCALAR);
            if(HASH_KERNEL) fd>>"//int64_t tmp;"; //block,fd,SCALAR_TMP);
            if(have_jj_shift)
                fp.def("jj_shift",jj_shift);

#if 0
            // ii and jj usage ends pretty early, so [hand-optimization...]
            // we can re-use some always-there regs...
            if( have_jjMODvl_reset ){
                //block.push_back({"jjovl","%s7"});
                // new: iloop%vl as a cyclic counter simplifies things...
                block.push_back({"tmod","ii"});
                if(jj/vl>2) block.push_back({"tm0","jj"});
            }else{
                if(have_vl_over_jj && !have_jj_M){
                    if(vl/jj > 127) block.push_back({"vlojj","ii"}); // register absolutely needed
                    //else block.push_back({"vlojj",jitdec(vl/jj)}); // we'll jitdec(vl/jj) below
                }else if( have_jj_M ){
                    // For jj=1,2 don't need jj_M, so smallest jj for jj_M is 3,
                    // So vlojj is ** at most ** MVL/jj or 256/3 ~ 85
                    assert( vl/jj <= 85 );
                    // we never need vlojj in a register in this case
                    // Note: we also need the jj value if nloop > 1, so put jj_M into ii...
                    block.push_back({"jj_M", "ii"}); // not a short/nice value
                }
            }
#endif
            // this forcing of "register re-use" is of course bogus for C intrinsics XXX
            if( have_jjMODvl_reset ){
                fp.def("tmod","ii");
                if(jj/vl>2) fp.def("tm0","jj");
            }else{
                if(have_vl_over_jj && !have_jj_M){
                    if(vl/jj > 127) fp.def("vlojj","ii");
                }else if( have_jj_M ){
                    assert( vl/jj <= 85 );
                    //fp.def("jj_M","ii");
                }
            }

            // vector regs
            fd  >>"__vr a;" >>"__vr b;";
            if(have_jj_M){
                fd  >>"__vr vx;">>"__vr vy;";
            }
            if((have_jj_M && nloop>1)){
                fd  >>"__vr bA;">>"__vr bD";
            }
        }

        //fd.scope(block,"vectorized double-loop --> index vectors");
        //+++++++++++++++++ constant registers +++++++++++++++++++
        //fd.ins("lvl vl0",                   "VL = vl0");
        fd>>"_ve_lvl(vl0);  // VL = vl0";
        if( have_sq ){
            //fd.ins("vseq sq");
            fd>>"__vr const sq = _ve_vseq_v();";
            FOR(i,vl) sq[i] = i;       // vseq_v
        }

        //+++++++++++++++++ fuse2 state registers +++++++++++++++++++
        if(nloop>1 && (iijj%vl0) ){
            fp>>"vl = 0;"; //fp.ins("or  vl, vl0,vl0", "vl = vl0");
        }
        if(SAVE_RESTORE_VLEN){
            //fp.ins("svl vl_save","save original VL --> vl_save [opt]");
            fp>>"//vl_save = _ve_svl_s; // <--- this intrinsic was not there?";
        }
        if(HASH_KERNEL){
            fp>>"reg_hash = 0;"; //fp.ins("lea reg_hash, 0");
        }
        fp>>"_ve_lvl(vl0); // VL = vl0"; //fp.ins("lvl vl0",                   "VL = vl0");

        if(nloop > 1 /*&& iijj%vl0==0*/ ){
            auto comment="cnt=ii*jj= "+jitdec(ii)+"*"+jitdec(jj)+" to 1 by "+jitdec(vl0);
#if 0
            auto ops = opLoadregStrings(iijj);
            if(ops.lea.empty() && ops.log.empty() && ops.shl.empty() && ops.ari.empty()){
                fp.ins(multiReplace("OUT", "cnt", choose(ops)));
            }else{
                fp.ins("mulu cnt, ii,jj", comment);
            }
#else
            fp>>"// "<<comment;
            fp>>"cnt = (unsigned)ii * (unsigned)jj;";
#endif
        }
        fp>>"// ii and jj regs now reusable"; //fp.rcom("ii and jj regs now reusable");

        if( have_jjMODvl_reset ){
            //fp.ins("lea jjovl,"+jitdec(jj/vl),      "jjovl = jj/vl = "+jitdec(jj)+"/"+jitdec(vl));
            //fp.ins("xor tmod, %s0,%s0",             "tmod=0, cycling < (jj/vl="+jitdec(jj/vl)+")");
            CBLK(fp,"int64_t tmod = 0; // tmod=0, cycling < (jj/vl="<<jj/vl<<")");
        }else{
            // NB: jj_M and vlojj are const registers, so they go into fd code
            if(have_vl_over_jj && !have_jj_M){
                if(vl/jj > 127)
                    //fd.ins("lea vlojj,"+jitdec(vl/jj),
                    //        "A vlojj = vl/jj = "+jitdec(vl)+"/"+jitdec(jj));
                    CBLK(fd,"vlojj = "<<vl/jj<<"; // A vlojj=vl/jj="<<vl<<"/"<<jj);
            }else if( have_jj_M ){
                //fd.ins(ve_load64("jj_M",jj_M)," fastdiv by "+jitdec(jj));
                fd.def("jj_M",jithex(jj_M))<<" // fastdiv by "+asDec(jj);
            }else{
                assert( !have_jj_M && !have_vl_over_jj );
            }
        }

#if 0
        cout<<" ********************** VECHASH VECHASH *********************"<<endl;
        // NOTE: fd is the parent but fp is where the code "should" really appear
        //       I think.   --- how to attache scope_parent concept to AsmFmtCols?
        if(have_sq)
            VecHash2::kern_asm_begin(fd,fp, "sq");
        else
            VecHash2::kern_asm_begin(fd,fp); // it'll grab %v40 or such and hold it
        // TODO: a way for subkernel to request state registers from caller.
#else
        fd>>"// VecHash kernel defines?";
        fp>>"// VecHash kernel precalc?";
#endif

#if 0
        // are we doing any b_period or nloop precalc?
        int const b_period_max = 8; // how many regs can you spare?
        //int const b_period = unroll_suggest( vl, jj, b_period_max );
        auto const suggest = unroll_suggest( vl,ii,jj, b_period_max );

#if 0    // later ...
        // use \c suggestion to create UnrollData
        auto const unr = mkUnroll_data( vl, ii, jj, b_period_max );
#else // for now...
        // TODO : precal for low b_periods XXX
        // Precalc for init-phase [iloop==0] when 1<jj<vl,
        // o/w use trivial 2-op instructions to initialize them
        bool const init_precalc = (1<jj && jj< vl0);
        //
        // Precalc for induction-phase only sometimes useful...
        bool const ind_precalc = (suggest==UNR_NLOOP1
                || suggest==UNR_JJMODVL_RESET // TBD??
                || suggest==UNR_JJPOW2_CYC
                || suggest==UNR_NLOOP
                || suggest==UNR_CYC);
        // Calculate init and ind precalc data vectors
        if( init_precalc ){
            ;
        }else if( ind_precalc ){
            ;
        }
#endif
        // precalc
        //int const bigfrac_vl_jj = (have_jjMODvl? jj_over_vl: have_vl_over_jj? vl_over_jj: bogus);
        //cout<<"=== //   register uint64_t bigfrac_vl_jj = "<<bigfrac_vl_jj<<"; // immediate";
#endif
        // reformulate as iloop==0 init-code, and
        // then jump into loop (after induction formula)
        //    (more assembly-like)

        verbose = 0; // cleans up the pseudocode output a bit
        // during loop, output pseudocode just once
        bool onceI = true; // induce-block code output at most once
        bool onceK = true; // kernel-block code output exactly most once
        bool onceL = true; // loop continueation code output exactly once
        // if no loop, some loop block are not required
        if(nloop==1){ onceI=false; onceL=false; } // no code for these blocks

        // We are writing
        //     ```for( ; cnt < iijj; cnt += vl )```
        //     NEW: ```for( ; cnt>0; cnt -= vl )```
        //          with gotos
        // PRECALC-BLOCK
        //  jit also puts the "first time through the loop" precalc into fp
        //  *before* the induce and kernel (fi,fk) blocks
        int iloop = 0;
        if(iloop==0) { // iloop==0 (cnt=0, or now iijj)
            //fp.lcom("INIT_BLOCK:");
            fp>>"// INIT_BLOCK:";
            if(nloop==1) assert(have_vl_over_jj==0);
            // now load the initial vector-loop registers:
            // sq[i] and jj are < FASTDIV_SAFEMAX, so we can avoid % and / operations
            if( jj==1 ){
                tr+="init:iloop 1 jj==1";
                FOR(i,vl) a[i] = i;    // sq/jj   or perhaps change have_sq?
                FOR(i,vl) b[i] = 0;    // sq%jj
                assert(have_bA_bD==0); assert(have_sq==0); assert(have_jj_shift==0);
                fp>>"a = _ve_vseq_v();         // a[i] = i";
                fp>>"b = _ve_vbrd_vs_i64(0LL); // b[i] = 0";
            }else if(jj>=vl){
                tr+="init:iloop 1 jj>=vl";
                if(verbose)cout<<" b";
                FOR(i,vl) a[i] = 0;    // sq < vl, so sq/jj < 1
                FOR(i,vl) b[i] = i;
                if(nloop<=1) {assert(have_bA_bD==0); assert(have_sq==0); assert(have_jj_shift==0);}
                fp>>"a = _ve_vbrd_vs_i64(0LL); // a[i] = 0";
                fp>>"b = _ve_vseq_v();         // b[i] = i;";
            }else if( positivePow2(jj) ){
                tr+="init:iloop 1 pow2jj";
                if(verbose)cout<<" c";
                // 2 ops (shr, and)
                FOR(i,vl) a[i] = (sq[i] >> jj_shift);  // bD = bA / jj; div_vsv
                FOR(i,vl) b[i] = (sq[i] & jj_minus_1); // bM = bA % jj; mod_vsv
                if(nloop<=1) assert(have_bA_bD==0); assert(have_sq==1); assert(have_jj_shift==1);
                ++cnt_sq; ++cnt_jj_shift;
                //fp.ins("vsrl.l.zx a, sq,"+jitdec(jj_shift), "a[i]=sq[i]/jj =sq[i]>>"+jitdec(jj_shift));
                //fp.ins("vand.l    b, "+jitimm(jj_minus_1)+",sq", "b[i]=sq[i]%jj =sq[i] & (2^"+jitdec(jj_shift)+"-1)");
                CBLK(fp,"_ve_vsrl_vvs(a, sq, jj_shift); // a[i]=sq[i]/jj = sq[i]>>"<<jj_shift);
            }else{
                tr+="init:iloop 1 fastdiv";
                if(verbose)cout<<" d";
                // 4 int ops (mul,shr, mul,sub)
                //v_divmod_vs( sq, jj, /*sq[]/jj*/a, /*sq[]%jj*/b );
                FOR(i,vl) a[i] = jj_M * sq[i] >> FASTDIV_C;
                FOR(i,vl) b[i] = sq[i] - a[i]*jj;
                //  OK since sq[] and jj both <= FASTDIV_SAFEMAX [(1<<21)-1]
                assert( (uint64_t)jj+vl <= (uint64_t)FASTDIV_SAFEMAX );
                // use mul_add_shr (fastdiv) approach if jj+vl>FASTDIV_SAFEMAX (1 more vector_add_scalar)
                if(nloop<=1) assert(have_bA_bD==0); assert(have_sq==1); assert(have_jj_shift==0);
                ++cnt_sq; ++cnt_jj_M;
                //fp.ins("vmulu.l   vx, sq,jj_M",      "fast division by jj="+jitdec(jj));
                //fp.ins("vsrl.l.zx a, vx,"+jitdec(FASTDIV_C), "  a[i] = sq[i]*jj_M >> "+jitdec(FASTDIV_C));
                //fp.ins("vmulu.l   vy, a,jj",         "form sq[i]%jj");
                //fp.ins("vsubu.l   b, sq,vy",         "  b[i] = sq[i] - a[i]*jj");
                CBLK(fp,"vx = _ve_vmulul_vsv(sq, jj_M);    // vx[i]=sq[i]/"<<jitdec(jj)<<" via fastdiv");
                CBLK(fp,"a  = _ve_vsrl_vsv(vx, FASTDIV_C); // a[i] = sq[i]*jj_M >> "<<jitdec(FASTDIV_C));
                CBLK(fp,"vy = _ve_vmulul_vsv(a, jj);       // form sq[i]%jj");
                CBLK(fp,"b  = _ve_vsubul_vvv(sq,vy);       // b[i] = sq[i] - a[i]*jj");
            }
        }
#if 0
        // base register holds absolute address of BASE label
        bool have_base_addr = false;
        auto reladdr=[&have_base_addr](std::string label){
            std::ostringstream oss;
            if(have_base_addr){
                oss<<label+" -BASE+."<<"(,%base)";
            }else{
                oss<<label<<" /* ERROR failed to initialize BASE for relative addressing*/";
            }
            return oss.str();

        };
#endif
        string alg_descr;
        {
            std::ostringstream oss;
            oss<<" Fuse2 alg: vl,ii,jj="<<vl<<","<<ii<<","<<jj<<" nloop="<<nloop;
            if(vl0%jj==0) oss<<" vl%jj==0";
            else if(jj%vl==0) oss<<" jj%vl==0";
            else if(positivePow2(jj)) oss<<" jj=2^"<<jj_shift;
            alg_descr = oss.str();
        }
        if(onceI){
            tr+="rel addr(BASE:+base register";
            //fp.ins("sic base", "BASE -> register for relocatable branching [opt: move upward]");
            //fp.lab("BASE",alg_descr);
            fp>>"// branch to BASE ...";
            //have_base_addr = true;
        }else{
            //fp.lcom(alg_descr);
            fp>>alg_descr;
        }
        if(onceI && nloop > 1) {
            //fp.ins("b "+reladdr("KERNEL_BLOCK"), "goto KERNEL_BLOCK");
            fp>>"goto KERNEL_BLOCK;";
        }
        cout<<" fp.cnt="<<cnt;
        goto KERNEL_BLOCK;
        // INDUCE-BLOCK
INDUCE:
        if(onceI) //fi.lab("INDUCE");
            fi>>"INDUCE:";
        if( iijj % vl0 ){ // special loop condition case
            tr+="induce:iijj%vl0 cnt update";
            //cnt += vl0;     // (vl, if you you to exacty hit iijj)
            //if(onceI) fi.ins("addu.l cnt, vl0, cnt", "cnt 0..iijj-1");
            cnt -= vl0;
            cout<<" fi.cnt="<<cnt;
            if(onceI) //fi.ins("subu.l cnt, cnt, vl0", "loop counter: cnt 0..iijj-1");
              CBLK(fi,"cnt = cnt - vl0;");
        }
#define VL_LAST_ITER 1
#if VL_LAST_ITER==1
#if 0
        // if careful about vl (possible lower value for last iter) vs vl0 can update here:
        if( cnt + vl > iijj ){ // last time might have reduced vl
            vl = iijj - cnt;
            cout<<" vl reduced for last loop to "<<vl<<endl;
        }
#endif
        // Careful: vl = new value for last iteration, vl0 = original = old value
        if( iijj % vl0 ){
            tr+="induce:iijj%vl0 last iter EARLY lower vl";
            // VE has single-op MIN
            //vl = std::min( (uint64_t)vl, /*remain =*/ iijj - cnt );
            vl = std::min( (uint64_t)vl, /*remain =*/ cnt );
            if(onceI){
                //AsmScope const block = {{"remain","%s40"}};
                //fi.scope(block, "last time reduce vl");
                //fi.ins("subu.l remain,iijj,cnt",    "remain = iijj -cnt");
                //fi.ins("mins.l vl, vl0, remain",    "vl = min(vl,remain)");
                //fi.ins("lvl vl");
                //fi.pop_scope();

                //fi.ins("mins.l vl, vl0,cnt",        "vl = min(vl0,cnt) for last iter,");
                //fi.ins("lvl vl",                    "   since iijj%vl0!=0. vl0 unchanged.");
                fi  >>"vl = (vl0<cnt? vl0: cnt); // vl = min(vl0,cnt) for last iter,"
                    >>"_ve_lvl(vl);              //    since iijj%vl0!=0. vl0 unchanged.";
            }
        }
#endif
        if(1){
            assert( nloop > 1 );
            // 2. Induction from a->ax, b->bx
            if(vl0%jj == 0){  // avoid div,mod -----1 vec op
                tr+="induce:vl0%jj a[]+=vl/jj, b[] const";
                // this includes cases with b_period==1 and high nloops
                if(verbose){cout<<" e";cout.flush();}
                FOR(i,vl) a[i] = a[i] + vl_over_jj;
                ++cnt_vl_over_jj; assert(have_vl_over_jj); assert(!have_bA_bD);
                if(onceI){
                    // VADD vx, vy/sy/I, vz [,vm]
                    //fi.ins("vadd a, "+(vl/jj>127?"vlojj":jitdec(vl/jj))+",a"
                    //        ,        "a[i] += (vl/jj="+jitdec(vl0/jj)+"), b[] unchanged");
                    fi>>"a =_ve_vadds_vsv("<<(vl/jj>127?"vlojj":jitdec(vl/jj))<<", a);"
                        <<" // a[i] += (vl/jj="+jitdec(vl0/jj)+"), b[] unchanged";
                }
            }else if(jj%vl0 == 0 ){  // -------------1 or 2 vec op (conditional)
                // unroll often nice w/ have_b_period (with maybe more regs)
                assert( have_bA_bD==0); assert(have_jj_shift==0); assert(have_vl_over_jj==0);
                assert( jj > vl0 ); assert( jj/vl0 > 1 ); assert( have_jjMODvl );
                if( !have_jjMODvl_reset ){
                    tr+="induce:jj%vl0 A: b[]+=vl0, a[] const";
                    // Note: this case should also be a "trivial" case for unroll suggestion
                    if(verbose){cout<<" f";cout.flush();}
                    FOR(i,vl) b[i] = b[i] + vl0;
                    ++cnt_jjMODvl; assert( have_jjMODvl );
                    if(onceI) //fi.ins("add b, "+jitdec(vl0)+",b",    "b[i] += vl0, a[] const");
                    /**/ CBLK(fi,"b = _ve_vadds_vsv("<<jitdec(vl0)<<",b); // b[i] += vl0, a[] const");
                }else{
                    // This case is potentially faster with a partial precalc unroll
                    // The division should be done with compute_uB
                    assert( have_jjMODvl && have_jjMODvl_reset );
                    // recall: have_jjMODvl_reset [HERE] is
                    // (vl0%jj!=0 && jj%vl0==0 && nloop >jj/vl0), so nloop > jj/vl0 > 1
                    // ==> Cannot optimize loop exit by querying the "reset" condition.
#if 0
                    //Lpi easy = iloop % jj_over_vl;       // scalar cyclic mod
                    Lpi easy = fastmod_uB( iloop, jj_over_vl_M, jj_over_vl );
                    // #pragma..unroll(jj/vl0) could be branchless
                    // can be optimized further into 3 minimal-op cases
                    if( easy ){                         // bump b[i], a[i] unchanged
                        if(verbose){cout<<" f";cout.flush();}
                        FOR(i,vl) b[i] = b[i] + vl0;
                        ++cnt_jjMODvl; assert( have_jjMODvl );
                    }else{                              // RESET b[i], bD[i]==1
                        if(verbose){cout<<" g";cout.flush();}
                        FOR(i,vl) b[i] = sq[i];
                        FOR(i,vl) a[i] = a[i] + 1;
                        ++cnt_sq; assert( have_sq==1 );
                        ++cnt_jjMODvl_reset;
                    }
#else
                    // iloop % jj_over_vl is implemented as a cyclic [0,jj/vl0) count
                    if(jj/vl0==2){                          // cyclic period 2 counter
                        tr+="induce:jj%vl0 case: tmod toggle";
                        assert(jj/vl0==2);
                        tmod = 1-tmod;
                    }else if(positivePow2(jj/vl0)){         // cyclic power-of-2 counter
                        tr+="induce:jj%vl0 tmod shift,and";
                        uint64_t const shift = positivePow2Shift((uint32_t)(jj/vl0));
                        uint64_t const mask  = (1ULL<<shift) - 1U;
                        tmod = (tmod+1U) & mask;
                    }else{                                  // cyclic period jj/vl0 counter
                        tr+="induce:jj%vl0 tmod cmov";
                        tmod += 1;
                        int64_t const tmp = tmod - jj_over_vl;
                        if(tmp==0) tmod=tmp; // cmov tmod,tmp,tmp
                    }

                    if( tmod ){
                        tr+="induce:jj%vl0 tmod: b[]+=vl0";
                        if(verbose){cout<<" f";cout.flush();}
                        FOR(i,vl) b[i] = b[i] + vl0;
                        ++cnt_jjMODvl; assert( have_jjMODvl );
                    }else{                              // RESET b[i], bD[i]==1
                        tr+="induce:jj%vl0 !tmod (reset): b[]=0,1,..., and a[]+=1";
                        if(verbose){cout<<" g";cout.flush();}
                        FOR(i,vl) b[i] = sq[i];
                        FOR(i,vl) a[i] = a[i] + 1;
                        ++cnt_sq; assert( have_sq==1 );
                        ++cnt_jjMODvl_reset;
                    }
#endif
                    if(onceI){
#if 0
                        //
                        // Hmm. # of scalar/conditional ops is rather large.
                        //      This may not even be a speedup over divmod!
                        //
                        //AsmScope const block = {{"tmp","%s40"}};
                        //fi.scope(block,"vectorized double-loop --> index vectors");
                        fi.com("jj%vl0==0 : iloop%(jj/vl0) check via cyclic tmod < jj/vl0="+jitdec(jj/vl0));
                        if(jj/vl0==2){                      // cyclic period 2 counter
                            fi.ins("sub tmod, 1,tmod",              "tmod period 2 (toggle 0|1)");
                        }else if(positivePow2(jj/vl0)){     // cyclic power-of-2 counter
                            uint64_t const shift = positivePow2Shift((uint32_t)(jj/vl0));
                            uint64_t const mask  = (1ULL<<shift) - 1U;
                            //tmod = (tmod+1) & mask;
                            fi.ins("add tm0, 1,tmod",               "tmod period jj/vl0="+jitdec(jj/vl0));
                            fi.ins("and tmod, tm0,"+jitimm(mask),   "     pow2 mask to reset");
                        }else{                              // cyclic period jj/vl0 counter
                            fi.ins("add tmod, 1,tmod",              "tmod period jj/vl0="+jitdec(jj/vl0));
                            fi.ins("sub tm0,tmod,"+jitdec(jj/vl0));
                            fi.ins("cmov.eq tmod,tm0,tm0",          "cmov reset tmod=0");
                        }
                        fi.ins("beq.w.t tmod,"+reladdr("RESET"));
                        fi.ins("vadd b, b,vl0",             "b[i] += vl0 (easy, a[] unchanged)")
                            .ins("b "+reladdr("INDUCE_DONE"), "branch around reset case");
                        fi.lab("RESET")
                            .ins("or b,0,sq",               "b[i] = sq[i] (reset case)")
                            .ins("vadd a,1,a",              "a[i] += 1");
                        fi.lab("INDUCE_DONE");
                        //fi.pop_scope();
#else
                        CBLK(fi,"// jj%vl0==0 : iloop%(jj/vl0) check via cyclic tmod < jj/vl0="<<jj/vl0);
                        if(jj/vl0==2){
                            fi<<"--tmod;";
                        }else if(positivePow2(jj/vl0)){     // cyclic power-of-2 counter
                            uint64_t const shift = positivePow2Shift((uint32_t)(jj/vl0));
                            uint64_t const mask  = (1ULL<<shift) - 1U;
                            //fi.ins("add tm0, 1,tmod",               "tmod period jj/vl0="+jitdec(jj/vl0));
                            //fi.ins("and tmod, tm0,"+jitimm(mask),   "     pow2 mask to reset");
                            CBLK(fi,"tmod = (tmod+1) & "<<jithex(mask)<<"; // cyclic power-of-2 counter");
                        }else{                              // cyclic period jj/vl0 counter
                            //fi.ins("add tmod, 1,tmod",              "tmod period jj/vl0="+jitdec(jj/vl0));
                            //fi.ins("sub tm0,tmod,"+jitdec(jj/vl0));
                            //fi.ins("cmov.eq tmod,tm0,tm0",          "cmov reset tmod=0");
                            fi  >>"++tmod;  // tmod period jj/vl0 = "<<jitdec(jj/vl0)
                                >>"if(tmod=="<<jitdec(jj/vl0)<<") tmod=0; // should generate cmov reset tmod=0?";
                        }
                        //fi.ins("beq.w.t tmod,"+reladdr("RESET"));
                        fi  >>"if(tmod == 0) goto RESET;";
                        //fi.ins("vadd b, b,vl0",             "b[i] += vl0 (easy, a[] unchanged)")
                        //    .ins("b "+reladdr("INDUCE_DONE"), "branch around reset case");
                        fi  >>"b = _ve_vaddul_vsv(vl0,b);      // b[i] += vl0 (easy, a[] unchanged)"
                            >>"goto INDUCE_DONE;";
                        //fi.lab("RESET")
                        //    .ins("or b,0,sq",               "b[i] = sq[i] (reset case)")
                        //    .ins("vadd a,1,a",              "a[i] += 1");
                        //fi.lab("INDUCE_DONE");
                        //fi.pop_scope();
                        fi  >>"RESET:"
                            >>"b = sq;                  // b[i] = sq[i] (reset case)"
                            >>"a = _ve_vaddul_vsv(1,a); // a[i] += 1"
                            >>"INDUCE_DONE:";
#endif
                    }
                }
                if( have_jjMODvl_reset ) assert( have_jjMODvl );
                assert( !have_bA_bD );
            }else if( positivePow2(jj) ){ // ------4 vec ops (add, shr, and, add)
                tr+="induce:pow2(jj): a[], b[] via shift+mask";
                if(verbose){cout<<" h";cout.flush();}
                assert( vl0%jj != 0 ); assert( jj%vl0 != 0 ); assert(have_bA_bD==1);
                assert(have_jj_shift==1); assert(have_vl_over_jj==0);
                // no...assert(have_sq==(jj>1&&jj<vl0));
                FOR(i,vl) bA[i] = vl0 + b[i];           // bA = b + vl0; add_vsv
                FOR(i,vl) bD[i] = (bA[i] >> jj_shift);  // bD = bA / jj; div_vsv
                FOR(i,vl) b [i] = (bA[i] & jj_minus_1); // bM = bA % jj; mod_vsv
                FOR(i,vl) a [i] = a[i] + bD[i]; // aA = a + bD; add_vvv
                ++cnt_bA_bD; ++cnt_jj_shift; assert( have_bA_bD );
                if(onceI){
#if 0
                    fi.ins("vaddu.l bA,"+jitdec(vl0)+",b"
                            , "bA[i] = b[i]+vl0 (jj="+jitdec(jj)+" power-of-two)")
                        .ins("vsrl.l.zx bD,bA,"+jitdec(jj_shift)
                                , "bD[i] = bA[i]>>jj_shift["+jitdec(jj_shift)+"]")
                        // note: VAND vy~M possible, while AND has sy~I, sz~M
                        .ins("vand b,"+jitimm(jj_minus_1)+",bA"
                                , "b[i] = bA[i]&jj_mask = bA[i]%"+jitdec(vl0))
                        .ins("vaddu.l a,a,bD"
                                , "a[i] += bD[i]");
#else
                    fi  >>"bA = _ve_vaddul_vsv("<<jitdec(vl0)<<",b);"
                        " // bA[i] = b[i]+vl0 (jj="<<jitdec(jj)<<" power-of-two)"
                        >>"bD = _ve_vsrl_vvs(bA,"<<jitdec(jj_shift)<<";"
                        " // bD[i] = bA[i]>>jj_shift["<<jitdec(jj_shift)<<"]"
                        >>"b  = _ve_vand_vsv("<<jithex(jj_minus_1)<<",bA);"
                        " // b[i] = bA[i]&jj_mask = bA[i]%"<<jitdec(vl0)
                        >>"a = _ve_vaddul_vvv(a,bD);   // a[i] += bD[i]"
                        ;
#endif
                }
            }else{ // div-mod ---------------------6 vec ops: add (mul,shr) (mul,sub) add
                tr+="induce: a[], b[] update via fastdiv";
                if(verbose){cout<<" i";cout.flush();}
                FOR(i,vl) bA[i] = vl0 + b[i];            // add_vsv
                FOR(i,vl) bD[i] = ((jj_M*bA[i]) >> FASTDIV_C);  // fastdiv_uB   : mul_vvs, shr_vs
                FOR(i,vl) b [i] = bA[i] - bD[i]*jj;     // long-hand    : mul_vvs, sub_vvv
                FOR(i,vl) a [i] = a[i] + bD[i];         // add_vvv
                ++cnt_bA_bD; ++cnt_jj_M;
                assert(have_bA_bD); assert(have_sq==(jj>1&&jj<vl0)); assert(!have_jj_shift);
                assert(!have_vl_over_jj); assert( jj+vl0 < (int)FASTDIV_SAFEMAX );
                if(onceI){
                    //local regs: bA,vx,bD,vy    input/output regs: a, b
                    //AsmScope const block = {{"tmp2","%s41"}};
#if 0
                    //fi.scope(block,"generic fastdiv induction");
                    if(0){//plain: output reg != input reg
                        fi.ins("vaddu bA,vl0,b",               "bA[i] = b[i] + vl0")
                            .ins("vmulu.l   vx,bA,jj_M")
                            .ins("vsrl.l.zx bD,vx,"+jitdec(FASTDIV_C), "bD[i]= bA[i]/[jj="+jitdec(jj)+"] fastdiv")
                            .ins("vmulu.l   vy,jj,bD")
                            .ins("vaddu.l   a, a,bD",          "a[i] += bD[i]")
                            .ins("vsubu.l   b, bA,vy",         "b[i] -= bD[i]*jj");
                    }else{//re-use regs
                        fi.ins("vaddu b ,vl0,b",               "b[i] = b[i] + vl0")
                            .ins("vmulu.l   bD,b ,jj_M")
                            .ins("vsrl.l.zx bD,bD,"+jitdec(FASTDIV_C), "bD[i]= bA[i]/[jj="+jitdec(jj)+"] fastdiv")
                            .ins("vmulu.l   vy,jj,bD")
                            .ins("vaddu.l   a, a,bD",          "a[i] += bD[i]")
                            .ins("vsubu.l   b, b ,vy",         "b[i] -= bD[i]*jj");
                    }
#else
                    fi  >>"b = _ve_vaddul_vsv(vl0,b);          // b[i] = b[i] + vl0"
                        >>"bD= _ve_vmulul_vsv(jj_M,b);"
                        >>"bD= _ve_vsrl_vvs(bD,FASTDIV_C);     // bD[i]= bA[i]/[jj="<<jitdec(jj)<<"]"
                        >>"vy= _ve_vmulul_vvv(jj, bD);"
                        >>"a = _ve_vaddul_vvv(a, bD);          // a[i] += bD[i]"
                        >>"b = _ve_vsubul_vvv(b, vy);          // b[i] -= bD[i]*jj"
                        ;
#endif
                }
            }
        }
#if VL_LAST_ITER==2
#if 0
        // if careful about vl (possible lower value for last iter) vs vl0 can update here:
        if( cnt + vl > iijj ){ // last time might have reduced vl
            vl = iijj - cnt;
            cout<<" vl reduced for last loop to "<<vl<<endl;
        }
#endif
        // Careful: vl = new value for last iteration, vl0 = original = old value
        if( iijj % vl0 ){
            tr+="induce:iijj%vl0 last iter LATE lower vl";
            // VE has single-op MIN
            //vl = std::min( (uint64_t)vl, /*remain =*/ iijj - cnt );
            vl = std::min( (uint64_t)vl, /*remain =*/ cnt );
            if(onceI){
                //AsmScope const block = {{"remain","%s40"}};
                //fi.scope(block, "last time reduce vl");
                //fi.ins("subu.l remain,iijj,cnt",    "remain = iijj -cnt");
                //fi.ins("mins.l vl, vl0, remain",    "vl = min(vl,remain)");
                //fi.ins("lvl vl");
                //fi.pop_scope();

                //fi.ins("mins.l vl, vl0, cnt",    "vl = min(vl,cnt)");
                //fi.ins("lvl vl");
                fi  >>"vl = (vl0<cnt? vl0: cnt); // vl = min(vl0,cnt) for last iter,"
                    >>"_ve_lvl(vl);";
            }
        }
#endif
        if(onceI){
            //fi.lcom("...KERNEL_BLOCK fall-through, a[], b[] induced");
            fi>>"// ...KERNEL_BLOCK fall-through, a[], b[] induced";
            onceI = false;
        }

KERNEL_BLOCK:
        tr+="KERNEL_BLOCK";
        if(onceK){ //cout<<"=== // #KERNEL_BLOCK: (fallthrough)\n"
            //<<"=== //        # <your code here, using a[] b[] loop-index vectors\n";
            if(nloop > 1) fk<<"KERNEL_BLOCK:"; //fk.lab("KERNEL_BLOCK");
            //fk.com("", "KERNEL BLOCK(a[],b[])", "<YOUR CODE HERE>","");
            fk  >>"//"
                >>"// KERNEL BLOCK(a[],b[])"
                >>OSSFMT("//    where a[],b[] are for(.."<<ii<<")for(.."<<jj<<"){...} indices, vl<="<<vl0)
                >>"// <YOUR CODE HERE>"
                >>"//";

            // interestingly, any re-usable data of the kernel should
            //  (somehow)
            // hoist such registers to our enclosing scope (... and maybe further)
            // Ex 1:  if b[] is const, A*b[]+C vector can be hoisted outside loops.
            //        (actually will later supply a loop-fuse that ALSO optimizes
            //         a generic lin.comb(a[]*A+b[]*B+C) for the inner loop)
            // Ex 2:  if a mask register is a function of a const b[] vector,
            //        and we have found b[] to be const (except for vl changes),
            //        the mask register can be hoisted to enclosing scope (outside loops)
            //        (perhaps a significant saving)
            // Ex 2:  sq register can be hoisted (AND combined with our sq?)
            //        instead of being recalculated
            if( HASH_KERNEL ){
                //VecHash2::kern_asm(fk,"a","b","vl","reg_hash","tmp"); 
                fk>>"// VecHash2 C kernel TBD";
            }
        }

        // KERNEL-BLOCK
        // Do something with the a[], b[] vectors of i,j loop indices...
        //cout<<"cnt "<<cnt<<" iloop "<<iloop<<" ii "<<ii<<" jj "<<jj<<endl;
        if(0){
            int const n=8; // output up-to-n [ ... [up-to-n]] ints
            int const bignum = std::max( ii, jj );
            int const wide = 1 + (bignum<10? 1: bignum <100? 2: bignum<1000? 3: 4);
            cout<<"a["<<vl<<"]="<<vecprt(n,wide,a,vl)<<endl;
            cout<<"b["<<vl<<"]="<<vecprt(n,wide,b,vl)<<endl;
        }
        // check correctness
        assert( vl == vabs[iloop].vl );
        FOR(i,vl) assert( a[i] == vabs[iloop].a[i] );
        FOR(i,vl) assert( b[i] == vabs[iloop].b[i] );
        // Alt. is a hash-value test:
        vhash.hash_combine(a.data(),b.data(),vl);
        //cout<<"iloop="<<iloop<<" vl="<<vl<<" vhash "<<vhash.u64()
        //    <<"\n ref hash "<<vabs[iloop].hash<<endl;
        assert( vhash.u64() == vabs[iloop].hash );

        onceK = false;

        // DONE_CHECK
        // XXX simplification : whole loop should be over 'remain' as
        //
        //     remain -= vl;
        //     if(remain>0) goto INDUCE
        //
        ++iloop; // needed only sometimes
        if( nloop <= 1 ){
            //cnt += vl; // for tighter exit assertion
            //cout<<" exit A cnt="<<cnt<<endl;
        }else if(iijj % vl0){
            if( vl == vl0 ){
                tr+="loop: induce next a[] b[] if vl==vl0";
                // cnt += vl; move to induce-block
                goto INDUCE;
            }
            tr+="exiting";
            // exit...
            //cout<<" exit B cnt="<<cnt<<endl;
            //cnt += vl; // just for tighter exit assertion
        }else{ // generic end-loop test
            //++cnt_use_iijj; // XXX old!
#if 0 // cnt 0..iijj
            cnt += vl0;
            if( cnt < iijj ){
                goto INDUCE;
            }
#else // cnt iijj..0
            cnt -= vl0;
            if( (int64_t)cnt > 0 ){
                tr+="loop: induce next a[] b[] if cnt-=vl0 still positive";
                //cout<<" again B cnt="<<cnt<<endl;
                goto INDUCE;
            }
            tr+="exiting B";
            //cout<<" exit B cnt="<<cnt<<endl;
#endif
        }

        if(onceL){
            if(nloop==1) fl>>"// (nloop == 1, no need to check if done)";
            else if(iijj%vl0){
                //fl.lcom("DONE_CHECK A : loop again if vl is not the reduced last value");
                //fl.ins("breq"+string(nloop>2?".t":".nt")+" vl,vl0,"+reladdr("INDUCE"), "iijj%vl!=0 so vl<vl0 means DONE");
                fl  >>"// DONE_CHECK A : loop again if vl is not the reduced last value"
                    >>"if(vl==vl0) goto INDUCE; // iijj%vl!=0 so vl<vl0 means DONE";
            }else{
                //fl.lcom("DONE_CHECK B : loop again if next cnt still < iijj");
                //  fl.ins("add cnt,vl0,cnt",  "cnt += vl");
                //      .ins("brlt"+string(nloop>2?".t":".nt")+" cnt,iijj, INDUCE-.", "next a[],b[]");
                //fl.ins("subu cnt,vl0,cnt",  "cnt -= vl (or vl0)");
                //fl.ins("bgt"+string(nloop>2?".t":".nt")+" cnt,"+reladdr("INDUCE"), "next a[],b[]");
                fl  >>"DONE_CHECK B : loop again if next cnt still < iijj"
                    >>"cnt -= vl; // (or vl0)"
                    >>"if(cnt > 0) goto INDUCE; // next a[],b[]";
            }
            onceL = false;
        }

        // LOOP_DONE ...
        if(SAVE_RESTORE_VLEN){
            //fz.ins("lvl vl_save","load VL <-- vl_save (restore VL on exit)");
            fz  >>"//_ve_lvl(vl_save); // restore VL on exit XXX when SVL op is supported!!!";
        }

#undef FOR
        cout<<" Yay! induction formulas worked! iloop,nloop="<<iloop<<","<<nloop<<endl;
        //assert( cnt == iijj || cnt == iijj/cnt*cnt );
        assert( cnt == 0 || (nloop<=1 && cnt == iijj) || (nloop>1 && cnt == iijj - iijj/vl0*vl0) ); //iijj/vl0*vl0 + vl0);
        assert( nloop == iloop );
        assert( have_vl_over_jj     == (cnt_vl_over_jj    > 0) );
        assert( have_bA_bD          == (cnt_bA_bD         > 0) );
        assert( have_sq             == (cnt_sq            > 0) );
        assert( have_jj_shift       == (cnt_jj_shift      > 0) );
        //assert( have_use_iijj       == (cnt_use_iijj      > 0) );
        assert( cnt_use_iijj == 0); // cnt iijj-->1 now
        assert( have_jj_M           == (cnt_jj_M          > 0) );
        assert( have_jjMODvl        == (cnt_jjMODvl       > 0) ); // old "special" count, case 'g' needed
        assert( have_jjMODvl_reset  == (cnt_jjMODvl_reset > 0) ); // old "special" count, case 'g' needed
        // XXX fX do not automatically form a tree. Would be nice not to have to think about
        //     ordering and where pop_scope should really occur.
        cout<<"##### Final program ################################"<<endl;
#if 0
        cout<<fd.flush();
        cout<<fp.flush();
        cout<<fi.flush();
        cout<<fk.flush();
        cout<<fl.flush();
        cout<<fz.flush();
        VecHash2::kern_asm_end(fd);
        cout<<fp.flush_all(); // kern_asm has some scope here, now
        cout<<fd.flush_all();
#else
        string prog = pr.str();
        cout<<prog;
#endif
        cout<<"##### End of program ################################"<<endl;
        // XXX multiple scope write/destroy has issues! (missing #undefs for fd right now)
        //fd.pop_scope();     // TODO: destructors should auto-pop any AsmFmtCols scopes!!!
    }
    cout<<tr.str()<<" vlen,ii,jj= "<<vlen<<" "<<ii<<" "<<jj;
    if(vl0!=vlen) cout<<" (used alt vlen "<<vl0<<")"<<endl;
    cout<<tr.dump();
}

/** `for(i=0..ii)for(j=0..jj){KERNEL(i,j);}` --> `for(ij=0..ii*jj by vl){KERNEL(vri,vrj);}` */
void test_vloop2_for_loop(Lpi const vlen, Lpi const ii, Lpi const jj, int const opt_t){
    ostringstream oss; // reusable allocation, for CBLK and OSSFMT macros
    ExecHash tr;
    //CodeGenAsm cg;
    //std::string code;
    // What vl should we really use? vlen or an alternative (lower) length?
    register int vl = vlen;
    int const b_period_max = 8; // how many regs can you spare?

    // This is pinned at [max] vl, even if it may be "inefficient".
    auto u = unroll_suggest( vl,ii,jj, b_period_max );

    auto uAlt = unroll_suggest(u);  // suggest an alternate nice vector length, sometimes.
    if(opt_t==3){ // we FORCE the alternate strategy
        if(u.vll) // equiv uAlt.suggested != UNR_UNSET
        {
            assert( uAlt.suggested != UNR_UNSET );
            cout<<" (forcing alt. strategy)"<<endl;
            //vl = u.vll; unroll_suggest(u);
            u = uAlt;
            vl = u.vl;
        }else{
            cout<<" (no really great alt.strategy)"<<endl;
        }
    }
    int const vl0 = vl;
    cout<<__FUNCTION__<<" using vl0="<<vl<<endl;

    {
        VecHash2 vhash(vl);
        int verbose=1;
        assert( vl > 0 );
        register uint64_t iijj = (uint64_t)ii * (uint64_t)jj;
        cout<<"fuse2 -t "<<setw(4)<<vl<<" "<<setw(4)<<ii<<" "<<setw(4)<<jj
            <<"   # for(0.."<<ii<<") for(0.."<<jj<<") --> a[VL],b[VL] for VL<="<<vl<<endl;

        //if (cnt+vl > iijj) vl = iijj - cnt;  // simplifies for cnt=0
        if ((uint64_t)vl > iijj) vl = iijj;
        int const nloop = (iijj+vl-1) / vl;    // div_round_up(iijj,vl)
        assert( (nloop > 1) == ((uint64_t)vl < iijj) );
        assert( vl0 > 0 );
        assert( ii > 0 );
        assert( jj > 0 );

        cout<<" Using "<<u.suggested<<" for vl,ii,jj="
            <<vl<<","<<ii<<","<<jj<<" unroll("<<u.unroll<<")"
            <<" [Alt vll="<<u.vll<<"]"<<endl;
        // Include the suggested strategy into the exechash, for future
        // productions that do unrolling (want better coverage)
        {
            std::ostringstream oss;
            oss<<"strategy "<<u.suggested;
            tr+=oss.str();
        }

        //cout<<"Verify-------"<<endl;
        // generate reference index outputs
        std::vector<Vab> vabs = ref_vloop2(vl, ii, jj, 0/*verbose*/);
        assert( vabs.size() > 0 );
        assert(vabs.size() == (size_t)(((ii*jj) +vl -1) / vl));
        if(verbose>=2) cout<<"   vl="<<vl<<"   ii="<<ii<<"   jj="<<jj
            <<"   nloop="<<nloop<<"   iijj="<<iijj
                <<" vabs.size() = "<<vabs.size()<<endl;
        // Have reference vabs vectors. Now we try induction way.
        // 1. initialize: could copy vabs[0] from const data storage, or...
        //   - generate from seq + divmod.
        //   - 2-loop induction uses 3 scalar registers:
        //     - \c cnt 0.. \c iijj, and \c vl (for jit, iijj is CCC (compile-time-const))
        //     - get final \c vl from cnt, vl and iij)
        //     - new: interpret cnt as "remain", going from iijj to one

#define FOR(I,VL) for(int I=0;I<VL;++I)

        // various misc precalculated consts and declarations.
        VVlpi a(vl), b(vl);                 // calculated loop index vectors
        VVlpi bA(vl), bD(vl), sq(vl);       // internal vectors
        // bA and bD are used when:
        //   iijj > vl && jj%vl!=0
        // sq is used when:
        //   iloop==0:   jj>1 && vl>=jj
        //   iloop >0:   jj%vl==0 && (special: iloop can be >= (jj/vl))
        assert( nloop >= 1 );
        uint64_t const jj_M = computeM_uB(jj); // for fastdiv_uB method
        int jj_shift=0;
        Vlpi jj_minus_1 = 0;
        if( positivePow2(jj) ){
            jj_shift = positivePow2Shift((uint32_t)jj);
            jj_minus_1 = jj - 1;
            assert( (uint64_t)1<<jj_shift == (uint64_t)jj );
            cout<<" power of two shift is "<<jj_shift<<"    mask is "<<jj_minus_1;
        }else{
            cout<<" jj_M="<<(void*)(intptr_t)jj_M<<" shift="<<FASTDIV_C;
        }

#if 0
        // NB: common operation is divmod(v,s,vM,vD) : v--> v%s, v/s,
        //     which has some optimizations for nice values of jj.
        // C++14: &vl=std::as_const(vl)
        auto v_divmod_vs = [&vl,&jj,&jj_M](/* in*/ VVlpi const& a, Vlpi const d, /*out*/ VVlpi& div, VVlpi& mod){
            assert( (Ulpi)jj < FASTDIV_SAFEMAX ); FOR(i,vl) assert( (Uvlpi)a[i] <= FASTDIV_SAFEMAX );
            FOR(i,vl) div[i] = jj_M * a[i] >> FASTDIV_C;
            FOR(i,vl) mod[i] = a[i] - div[i]*jj;
        };
#endif

        // Using immediate values decreases register usage.
        // Even though VE asm ops can use certain values as immediate data,
        // Want auto mechanism to decide whether immediate is possible.
        //
        // have_FOO and cnt_FOO : FOO register usage condition and actual use count
        bool const have_vl_over_jj = nloop>1 && vl0%jj==0;
        bool const have_bA_bD = nloop>1 && vl%jj!=0 && jj%vl!=0;
        bool const have_jjMODvl_reset      = (vl0%jj!=0 && jj%vl0==0 && nloop >jj/vl0); // case 'g'
        bool const have_jjMODvl = (vl0%jj!=0 && jj%vl0==0 && nloop>=jj/vl0);
        if( 1 || have_jjMODvl ){
            cout<<" jj/vl0="<<jj/vl0<<endl;
            if( have_jjMODvl ) assert( jj/vl0 > 1 );
            if( have_jjMODvl_reset ) assert( have_jjMODvl );
        }
        bool const have_sq_init_block = jj!=1 && jj<vl0;
        bool const have_sq_indu_block = have_jjMODvl_reset;
        bool const have_sq = have_sq_init_block                             // when iloop==0
            || have_sq_indu_block;                                          // o/w
        bool const have_jj_shift = (jj!=1 && jj<vl0 && positivePow2(jj))    // when iloop==0
            || (nloop>1 && vl0%jj!=0 && jj%vl0!=0 && positivePow2(jj));     // o/w
        bool const have_jj_M = (jj>1 && jj<vl0 && !positivePow2(jj))        // when iloop==0
            || (nloop>1 && vl0%jj!=0 && jj%vl0!=0 && !positivePow2(jj));    // o/w

        int cnt_vl_over_jj=0, cnt_bA_bD=0, cnt_sq=0, cnt_jj_shift=0, cnt_jj_M=0;
        int cnt_jjMODvl=0, cnt_jjMODvl_reset=0, cnt_use_iijj=0;

        Vlpi const vl_over_jj  = have_vl_over_jj? vl0/jj: 0;
        Vlpi const jj_over_vl  = have_jjMODvl_reset? jj/vl0: 0;
        Vlpi tmod       = have_jjMODvl_reset? 0: 777; // used if have_jjMODvl_reset
        //uint64_t const jj_over_vl_M = (have_jjMODvl_reset? computeM_uB(jj_over_vl): 0);
        assert( !(have_vl_over_jj && have_jjMODvl_reset) ); // never need both these constants
        assert( !(have_jj_M && have_jjMODvl_reset) ); // never both
        //assert( !(have_jj_M && have_vl_over_jj) ); // HAPPENS ex: vl,ii,jj=9,4,3

        register uint64_t cnt = iijj; // NEW: iijj to one [i.e. remain], rather than [0..iijj)

#if 0
        AsmFmtCols fd,fp,fi,fk,fl,fz;
        fp.com("fuse2 presets").setParent(&fd);
        fi.setParent(&fp);
        fk.setParent(&fp);
        fl.setParent(&fp);
        fz.setParent(&fp);
#else
        Cunit pr("cfuse2","C",1);
        CBLOCK_SCOPE(cfuse2,"",pr,pr.root);
        auto& fd = cfuse2["defines"];
        auto& fp = fd["preset"]; fp>>"//cfuse2 presets";
        auto& fi = fd["induce"]; // really?
        //auto& fk = fd["kernel"];
        CBLOCK_SCOPE(fk,"",pr,fd);
        fk  >>"// Even though we replace a 'C' for()for(), we use goto style"
            >>"// so that: (i) initialization and induction code is separated,"
            >>"// (ii) loop exit test can be optimized in some cases, and"
            >>"// (iii) we have a ** single insertion point ** for kernel code"
            >>"// (iv) avoid branches in or out of loop scope;"
            >>"// (v) this is the natural structure for the assembler version (fuse2.cpp)"
            >>"//"
            >>"// ... but this loop style cannot be unrolled automatically"
            >>"//     (should try out the loop option with cnt==0 as special"
            >>"//      case because maybe the compiler will peel the loop"
            >>"//      automatically --> more readable JIT code !!! )"
            ;
        //
        //   gcc apparently has loop splitting,
        //   but llvm might not:
        //
        //   http://lists.llvm.org/pipermail/llvm-bugs/2018-April/063932.html
        //
        //   "[llvm-bugs] [Bug 36990] New: LLVM is missing loop splitting"
        //
        // with little activity in the last year.
        //

        auto& fl = fd["loopend"];
        auto& fz = fd["last"];
#endif

        // important: vechash puts a scope onto fp, so later code snippets
        // fk,fl,fz should not "miss" any current scopes. (i.e. it is up
        // to us to create an appropriate tree of scopes).

        // local labels:
        string fusename;
        {
            std::ostringstream oss;
            oss<<"fuse2_"<<vl<<"_"<<ii<<"_"<<jj;
            fusename = oss.str();
        }
        cfuse2
            //.def("BASE", "L_"+fusename+"_BASE")
            .def("INDUCE", "L_"+fusename+"_INDUCE")
            .def("KERNEL_BLOCK", "L_"+fusename+"_KERNEL_BLOCK")
            .def("vl0", vl0)
            .def("FASTDIV_C",FASTDIV_C)
#if 0
            .def("FASTDIV_"+jitdec(jj)+"(VR_IN,VR_DIV,VR_REM)", "do{ \\\n"
                    "    __vr tmp1 =_ve_vmulul_vsv(VR_IN, FASTDIV_MUL_"+jitdec(jj)+");\\\n"
                    "    VR_DIV = _ve_vsrl_vsv(tmp1, "+jitdec(FASTDIV_C)+"); \\\n"
                    "    __vr tmp2 = _ve_vmulul_vsv(VR_DIV, "+jitdec(jj)+"); \\\n"
                    "    VR_REM = _ve_vsubul_vvv(VR_IN, tmp2); \\\n"
                    "}while(0)"
                )
#endif
            >>"int64_t vl;"
            >>"int64_t ii;"
            >>"int64_t jj;"
            >>"int64_t base;"
            ;

        // For Cblock, we can just record what variables we used, and we
        // let the compiler assign registers.  For asm, though, register
        // assignments/re-use was considerably more complicated.
        if(1){ // illustrate use of auto-register assignments,
            // even before the scope is officially defined in an AsmFmtCols
            // UTILITY LAMBDAS
            // pre-assign some registers to fd "defines" scope
            // I don't care about speed, for now.

            // Allocations // const registers
            //if( have_sq )   fd>>"int64_t sq;";
            // Allocations // state registers
            // Note that many of these are optional, so register assignments
            // for different kernels may be ... different.
#define SAVE_RESTORE_VLEN 1
#define HASH_KERNEL 1
            if(nloop>1)                     
            if(nloop>1 && (iijj%vl0) )      fd>>"int64_t vl;"; //block_alloc_scalar("vl");
            if(SAVE_RESTORE_VLEN)           fd>>"//int64_t vl_save;"; //block_alloc_scalar("vl_save");
            if(HASH_KERNEL) fd>>"//int64_t reg_hash;"; //ve_propose_reg("reg_hash",block,fd,SCALAR);
            if(HASH_KERNEL) fd>>"//int64_t tmp;"; //block,fd,SCALAR_TMP);
            if(have_jj_shift)
                fp.def("jj_shift",jj_shift);

#if 0
            // ii and jj usage ends pretty early, so [hand-optimization...]
            // we can re-use some always-there regs...
            if( have_jjMODvl_reset ){
                //block.push_back({"jjovl","%s7"});
                // new: iloop%vl as a cyclic counter simplifies things...
                block.push_back({"tmod","ii"});
                if(jj/vl>2) block.push_back({"tm0","jj"});
            }else{
                if(have_vl_over_jj && !have_jj_M){
                    if(vl/jj > 127) block.push_back({"vlojj","ii"}); // register absolutely needed
                    //else block.push_back({"vlojj",jitdec(vl/jj)}); // we'll jitdec(vl/jj) below
                }else if( have_jj_M ){
                    // For jj=1,2 don't need jj_M, so smallest jj for jj_M is 3,
                    // So vlojj is ** at most ** MVL/jj or 256/3 ~ 85
                    assert( vl/jj <= 85 );
                    // we never need vlojj in a register in this case
                    // Note: we also need the jj value if nloop > 1, so put jj_M into ii...
                    block.push_back({"jj_M", "ii"}); // not a short/nice value
                }
            }
#endif
            // this forcing of "register re-use" is of course bogus for C intrinsics XXX
            if( have_jjMODvl_reset ){
                fp.def("tmod","ii");
                if(jj/vl>2) fp.def("tm0","jj");
            }else{
                if(have_vl_over_jj && !have_jj_M){
                    if(vl/jj > 127) fp.def("vlojj","ii");
                }else if( have_jj_M ){
                    assert( vl/jj <= 85 );
                    //fp.def("jj_M","ii");
                }
            }

            // vector regs
            fd  >>"__vr a;" >>"__vr b;";
            if(have_jj_M){
                fd  >>"__vr vx;">>"__vr vy;";
            }
            if((have_jj_M && nloop>1)){
                fd  >>"__vr bA;">>"__vr bD";
            }
        }
        bool cjit_have_vl=false;

        //fd.scope(block,"vectorized double-loop --> index vectors");
        //+++++++++++++++++ constant registers +++++++++++++++++++
        //fd.ins("lvl vl0",                   "VL = vl0");
        fd>>"_ve_lvl(vl0);  // VL = vl0";
        if( have_sq ){
            //fd.ins("vseq sq");
            fd>>"__vr const sq = _ve_vseq_v();";
            FOR(i,vl) sq[i] = i;       // vseq_v
        }

        //+++++++++++++++++ fuse2 state registers +++++++++++++++++++
        if(nloop>1 && (iijj%vl0) ){
            fp>>"vl = 0;"; //fp.ins("or  vl, vl0,vl0", "vl = vl0");
        }
        if(SAVE_RESTORE_VLEN){
            //fp.ins("svl vl_save","save original VL --> vl_save [opt]");
            fp>>"//vl_save = _ve_svl_s; // <--- this intrinsic was not there?";
        }
        if(HASH_KERNEL){
            fp>>"reg_hash = 0;"; //fp.ins("lea reg_hash, 0");
        }
        fp>>"_ve_lvl(vl0); // VL = vl0"; //fp.ins("lvl vl0",                   "VL = vl0");

        if(nloop > 1 /*&& iijj%vl0==0*/ ){
            auto comment="cnt=ii*jj= "+jitdec(ii)+"*"+jitdec(jj)+" to 1 by "+jitdec(vl0);
#if 0
            auto ops = opLoadregStrings(iijj);
            if(ops.lea.empty() && ops.log.empty() && ops.shl.empty() && ops.ari.empty()){
                fp.ins(multiReplace("OUT", "cnt", choose(ops)));
            }else{
                fp.ins("mulu cnt, ii,jj", comment);
            }
#else
            fp>>"// "<<comment;
            fp>>"cnt = (unsigned)ii * (unsigned)jj;";
#endif
        }
        fp>>"// ii and jj regs now reusable"; //fp.rcom("ii and jj regs now reusable");

        if( have_jjMODvl_reset ){
            //fp.ins("lea jjovl,"+jitdec(jj/vl),      "jjovl = jj/vl = "+jitdec(jj)+"/"+jitdec(vl));
            //fp.ins("xor tmod, %s0,%s0",             "tmod=0, cycling < (jj/vl="+jitdec(jj/vl)+")");
            CBLK(fp,"int64_t tmod = 0; // tmod=0, cycling < (jj/vl="<<jj/vl<<")");
        }else{
            // NB: jj_M and vlojj are const registers, so they go into fd code
            if(have_vl_over_jj && !have_jj_M){
                if(vl/jj > 127)
                    //fd.ins("lea vlojj,"+jitdec(vl/jj),
                    //        "A vlojj = vl/jj = "+jitdec(vl)+"/"+jitdec(jj));
                    CBLK(fd,"vlojj = "<<vl/jj<<"; // A vlojj=vl/jj="<<vl<<"/"<<jj);
            }else if( have_jj_M ){
                //fd.ins(ve_load64("jj_M",jj_M)," fastdiv by "+jitdec(jj));
                fd.def("jj_M",jithex(jj_M))<<" // fastdiv by "+asDec(jj);
            }else{
                assert( !have_jj_M && !have_vl_over_jj );
            }
        }

#if 0
        cout<<" ********************** VECHASH VECHASH *********************"<<endl;
        // NOTE: fd is the parent but fp is where the code "should" really appear
        //       I think.   --- how to attache scope_parent concept to AsmFmtCols?
        if(have_sq)
            VecHash2::kern_asm_begin(fd,fp, "sq");
        else
            VecHash2::kern_asm_begin(fd,fp); // it'll grab %v40 or such and hold it
        // TODO: a way for subkernel to request state registers from caller.
#else
        fd>>"// VecHash kernel defines?";
        fp>>"// VecHash kernel precalc?";
#endif

#if 0
        // are we doing any b_period or nloop precalc?
        int const b_period_max = 8; // how many regs can you spare?
        //int const b_period = unroll_suggest( vl, jj, b_period_max );
        auto const suggest = unroll_suggest( vl,ii,jj, b_period_max );

#if 0    // later ...
        // use \c suggestion to create UnrollData
        auto const unr = mkUnroll_data( vl, ii, jj, b_period_max );
#else // for now...
        // TODO : precal for low b_periods XXX
        // Precalc for init-phase [iloop==0] when 1<jj<vl,
        // o/w use trivial 2-op instructions to initialize them
        bool const init_precalc = (1<jj && jj< vl0);
        //
        // Precalc for induction-phase only sometimes useful...
        bool const ind_precalc = (suggest==UNR_NLOOP1
                || suggest==UNR_JJMODVL_RESET // TBD??
                || suggest==UNR_JJPOW2_CYC
                || suggest==UNR_NLOOP
                || suggest==UNR_CYC);
        // Calculate init and ind precalc data vectors
        if( init_precalc ){
            ;
        }else if( ind_precalc ){
            ;
        }
#endif
        // precalc
        //int const bigfrac_vl_jj = (have_jjMODvl? jj_over_vl: have_vl_over_jj? vl_over_jj: bogus);
        //cout<<"=== //   register uint64_t bigfrac_vl_jj = "<<bigfrac_vl_jj<<"; // immediate";
#endif
        // reformulate as iloop==0 init-code, and
        // then jump into loop (after induction formula)
        //    (more assembly-like)

        verbose = 0; // cleans up the pseudocode output a bit
        // during loop, output pseudocode just once
        bool onceI = true; // induce-block code output at most once
        bool onceK = true; // kernel-block code output exactly most once
        bool onceL = true; // loop continueation code output exactly once
        // if no loop, some loop block are not required
        if(nloop==1){ onceI=false; onceL=false; } // no code for these blocks

        // We are writing
        //     ```for( ; cnt < iijj; cnt += vl )```
        //     NEW: ```for( ; cnt>0; cnt -= vl )```
        //          with gotos
        // PRECALC-BLOCK
        //  jit also puts the "first time through the loop" precalc into fp
        //  *before* the induce and kernel (fi,fk) blocks
        int iloop = 0;
        if(iloop==0) { // iloop==0 (cnt=0, or now iijj)
            //fp.lcom("INIT_BLOCK:");
            fp>>"// INIT_BLOCK:";
            if(nloop==1) assert(have_vl_over_jj==0);
            // now load the initial vector-loop registers:
            // sq[i] and jj are < FASTDIV_SAFEMAX, so we can avoid % and / operations
            if( jj==1 ){
                tr+="init:iloop 1 jj==1";
                FOR(i,vl) a[i] = i;    // sq/jj   or perhaps change have_sq?
                FOR(i,vl) b[i] = 0;    // sq%jj
                assert(have_bA_bD==0); assert(have_sq==0); assert(have_jj_shift==0);
                fp>>"a = _ve_vseq_v();         // a[i] = i";
                fp>>"b = _ve_vbrd_vs_i64(0LL); // b[i] = 0";
            }else if(jj>=vl){
                tr+="init:iloop 1 jj>=vl";
                if(verbose)cout<<" b";
                FOR(i,vl) a[i] = 0;    // sq < vl, so sq/jj < 1
                FOR(i,vl) b[i] = i;
                if(nloop<=1) {assert(have_bA_bD==0); assert(have_sq==0); assert(have_jj_shift==0);}
                fp>>"a = _ve_vbrd_vs_i64(0LL); // a[i] = 0";
                fp>>"b = _ve_vseq_v();         // b[i] = i;";
            }else if( positivePow2(jj) ){
                tr+="init:iloop 1 pow2jj";
                if(verbose)cout<<" c";
                // 2 ops (shr, and)
                FOR(i,vl) a[i] = (sq[i] >> jj_shift);  // bD = bA / jj; div_vsv
                FOR(i,vl) b[i] = (sq[i] & jj_minus_1); // bM = bA % jj; mod_vsv
                if(nloop<=1) assert(have_bA_bD==0); assert(have_sq==1); assert(have_jj_shift==1);
                ++cnt_sq; ++cnt_jj_shift;
                //fp.ins("vsrl.l.zx a, sq,"+jitdec(jj_shift), "a[i]=sq[i]/jj =sq[i]>>"+jitdec(jj_shift));
                //fp.ins("vand.l    b, "+jitimm(jj_minus_1)+",sq", "b[i]=sq[i]%jj =sq[i] & (2^"+jitdec(jj_shift)+"-1)");
                CBLK(fp,"_ve_vsrl_vvs(a, sq, jj_shift); // a[i]=sq[i]/jj = sq[i]>>"<<jj_shift);
            }else{
                tr+="init:iloop 1 fastdiv";
                if(verbose)cout<<" d";
                // 4 int ops (mul,shr, mul,sub)
                //v_divmod_vs( sq, jj, /*sq[]/jj*/a, /*sq[]%jj*/b );
                FOR(i,vl) a[i] = jj_M * sq[i] >> FASTDIV_C;
                FOR(i,vl) b[i] = sq[i] - a[i]*jj;
                //  OK since sq[] and jj both <= FASTDIV_SAFEMAX [(1<<21)-1]
                assert( (uint64_t)jj+vl <= (uint64_t)FASTDIV_SAFEMAX );
                // use mul_add_shr (fastdiv) approach if jj+vl>FASTDIV_SAFEMAX (1 more vector_add_scalar)
                if(nloop<=1) assert(have_bA_bD==0); assert(have_sq==1); assert(have_jj_shift==0);
                ++cnt_sq; ++cnt_jj_M;
                //fp.ins("vmulu.l   vx, sq,jj_M",      "fast division by jj="+jitdec(jj));
                //fp.ins("vsrl.l.zx a, vx,"+jitdec(FASTDIV_C), "  a[i] = sq[i]*jj_M >> "+jitdec(FASTDIV_C));
                //fp.ins("vmulu.l   vy, a,jj",         "form sq[i]%jj");
                //fp.ins("vsubu.l   b, sq,vy",         "  b[i] = sq[i] - a[i]*jj");
                CBLK(fp,"vx = _ve_vmulul_vsv(sq, jj_M);    // vx[i]=sq[i]/"<<jitdec(jj)<<" via fastdiv");
                CBLK(fp,"a  = _ve_vsrl_vsv(vx, FASTDIV_C); // a[i] = sq[i]*jj_M >> "<<jitdec(FASTDIV_C));
                CBLK(fp,"vy = _ve_vmulul_vsv(a, jj);       // form sq[i]%jj");
                CBLK(fp,"b  = _ve_vsubul_vvv(sq,vy);       // b[i] = sq[i] - a[i]*jj");
            }
        }
#if 0
        // base register holds absolute address of BASE label
        bool have_base_addr = false;
        auto reladdr=[&have_base_addr](std::string label){
            std::ostringstream oss;
            if(have_base_addr){
                oss<<label+" -BASE+."<<"(,%base)";
            }else{
                oss<<label<<" /* ERROR failed to initialize BASE for relative addressing*/";
            }
            return oss.str();

        };
#endif
        string alg_descr;
        {
            std::ostringstream oss;
            oss<<" Fuse2 alg: vl,ii,jj="<<vl<<","<<ii<<","<<jj<<" nloop="<<nloop;
            if(vl0%jj==0) oss<<" vl%jj==0";
            else if(jj%vl==0) oss<<" jj%vl==0";
            else if(positivePow2(jj)) oss<<" jj=2^"<<jj_shift;
            alg_descr = oss.str();
        }
        if(onceI){
            tr+="rel addr(BASE:+base register";
            //fp.ins("sic base", "BASE -> register for relocatable branching [opt: move upward]");
            //fp.lab("BASE",alg_descr);
            fp>>"// branch to BASE ...";
            //have_base_addr = true;
        }else{
            //fp.lcom(alg_descr);
            fp>>alg_descr;
        }
        if(onceI && nloop > 1) {
            //fp.ins("b "+reladdr("KERNEL_BLOCK"), "goto KERNEL_BLOCK");
            fp>>"goto KERNEL_BLOCK;";
        }
        cout<<" fp.cnt="<<cnt;
        goto KERNEL_BLOCK;
        // INDUCE-BLOCK
INDUCE:
        if(onceI) //fi.lab("INDUCE");
            fi>>"INDUCE:";
        if( iijj % vl0 ){ // special loop condition case
            tr+="induce:iijj%vl0 cnt update";
            //cnt += vl0;     // (vl, if you you to exacty hit iijj)
            //if(onceI) fi.ins("addu.l cnt, vl0, cnt", "cnt 0..iijj-1");
            cnt -= vl0;
            cout<<" fi.cnt="<<cnt;
            if(onceI) //fi.ins("subu.l cnt, cnt, vl0", "loop counter: cnt 0..iijj-1");
              CBLK(fi,"cnt = cnt - vl0;");
        }
#define VL_LAST_ITER 1
#if VL_LAST_ITER==1
#if 0
        // if careful about vl (possible lower value for last iter) vs vl0 can update here:
        if( cnt + vl > iijj ){ // last time might have reduced vl
            vl = iijj - cnt;
            cout<<" vl reduced for last loop to "<<vl<<endl;
        }
#endif
        // Careful: vl = new value for last iteration, vl0 = original = old value
        if( iijj % vl0 ){
            tr+="induce:iijj%vl0 last iter EARLY lower vl";
            // VE has single-op MIN
            //vl = std::min( (uint64_t)vl, /*remain =*/ iijj - cnt );
            vl = std::min( (uint64_t)vl, /*remain =*/ cnt );
            if(onceI){
                //AsmScope const block = {{"remain","%s40"}};
                //fi.scope(block, "last time reduce vl");
                //fi.ins("subu.l remain,iijj,cnt",    "remain = iijj -cnt");
                //fi.ins("mins.l vl, vl0, remain",    "vl = min(vl,remain)");
                //fi.ins("lvl vl");
                //fi.pop_scope();

                //fi.ins("mins.l vl, vl0,cnt",        "vl = min(vl0,cnt) for last iter,");
                //fi.ins("lvl vl",                    "   since iijj%vl0!=0. vl0 unchanged.");
                fi  >>"vl = (vl0<cnt? vl0: cnt); // vl = min(vl0,cnt) for last iter,"
                    >>"_ve_lvl(vl);              //    since iijj%vl0!=0. vl0 unchanged.";
            }
        }
#endif
        if(1){
            assert( nloop > 1 );
            // 2. Induction from a->ax, b->bx
            if(vl0%jj == 0){  // avoid div,mod -----1 vec op
                tr+="induce:vl0%jj a[]+=vl/jj, b[] const";
                // this includes cases with b_period==1 and high nloops
                if(verbose){cout<<" e";cout.flush();}
                FOR(i,vl) a[i] = a[i] + vl_over_jj;
                ++cnt_vl_over_jj; assert(have_vl_over_jj); assert(!have_bA_bD);
                if(onceI){
                    // VADD vx, vy/sy/I, vz [,vm]
                    //fi.ins("vadd a, "+(vl/jj>127?"vlojj":jitdec(vl/jj))+",a"
                    //        ,        "a[i] += (vl/jj="+jitdec(vl0/jj)+"), b[] unchanged");
                    fi>>"a =_ve_vadds_vsv("<<(vl/jj>127?"vlojj":jitdec(vl/jj))<<", a);"
                        <<" // a[i] += (vl/jj="+jitdec(vl0/jj)+"), b[] unchanged";
                }
            }else if(jj%vl0 == 0 ){  // -------------1 or 2 vec op (conditional)
                // unroll often nice w/ have_b_period (with maybe more regs)
                assert( have_bA_bD==0); assert(have_jj_shift==0); assert(have_vl_over_jj==0);
                assert( jj > vl0 ); assert( jj/vl0 > 1 ); assert( have_jjMODvl );
                if( !have_jjMODvl_reset ){
                    tr+="induce:jj%vl0 A: b[]+=vl0, a[] const";
                    // Note: this case should also be a "trivial" case for unroll suggestion
                    if(verbose){cout<<" f";cout.flush();}
                    FOR(i,vl) b[i] = b[i] + vl0;
                    ++cnt_jjMODvl; assert( have_jjMODvl );
                    if(onceI) //fi.ins("add b, "+jitdec(vl0)+",b",    "b[i] += vl0, a[] const");
                    /**/ CBLK(fi,"b = _ve_vadds_vsv("<<jitdec(vl0)<<",b); // b[i] += vl0, a[] const");
                }else{
                    // This case is potentially faster with a partial precalc unroll
                    // The division should be done with compute_uB
                    assert( have_jjMODvl && have_jjMODvl_reset );
                    // recall: have_jjMODvl_reset [HERE] is
                    // (vl0%jj!=0 && jj%vl0==0 && nloop >jj/vl0), so nloop > jj/vl0 > 1
                    // ==> Cannot optimize loop exit by querying the "reset" condition.
#if 0
                    //Lpi easy = iloop % jj_over_vl;       // scalar cyclic mod
                    Lpi easy = fastmod_uB( iloop, jj_over_vl_M, jj_over_vl );
                    // #pragma..unroll(jj/vl0) could be branchless
                    // can be optimized further into 3 minimal-op cases
                    if( easy ){                         // bump b[i], a[i] unchanged
                        if(verbose){cout<<" f";cout.flush();}
                        FOR(i,vl) b[i] = b[i] + vl0;
                        ++cnt_jjMODvl; assert( have_jjMODvl );
                    }else{                              // RESET b[i], bD[i]==1
                        if(verbose){cout<<" g";cout.flush();}
                        FOR(i,vl) b[i] = sq[i];
                        FOR(i,vl) a[i] = a[i] + 1;
                        ++cnt_sq; assert( have_sq==1 );
                        ++cnt_jjMODvl_reset;
                    }
#else
                    // iloop % jj_over_vl is implemented as a cyclic [0,jj/vl0) count
                    if(jj/vl0==2){                          // cyclic period 2 counter
                        tr+="induce:jj%vl0 case: tmod toggle";
                        assert(jj/vl0==2);
                        tmod = 1-tmod;
                    }else if(positivePow2(jj/vl0)){         // cyclic power-of-2 counter
                        tr+="induce:jj%vl0 tmod shift,and";
                        uint64_t const shift = positivePow2Shift((uint32_t)(jj/vl0));
                        uint64_t const mask  = (1ULL<<shift) - 1U;
                        tmod = (tmod+1U) & mask;
                    }else{                                  // cyclic period jj/vl0 counter
                        tr+="induce:jj%vl0 tmod cmov";
                        tmod += 1;
                        int64_t const tmp = tmod - jj_over_vl;
                        if(tmp==0) tmod=tmp; // cmov tmod,tmp,tmp
                    }

                    if( tmod ){
                        tr+="induce:jj%vl0 tmod: b[]+=vl0";
                        if(verbose){cout<<" f";cout.flush();}
                        FOR(i,vl) b[i] = b[i] + vl0;
                        ++cnt_jjMODvl; assert( have_jjMODvl );
                    }else{                              // RESET b[i], bD[i]==1
                        tr+="induce:jj%vl0 !tmod (reset): b[]=0,1,..., and a[]+=1";
                        if(verbose){cout<<" g";cout.flush();}
                        FOR(i,vl) b[i] = sq[i];
                        FOR(i,vl) a[i] = a[i] + 1;
                        ++cnt_sq; assert( have_sq==1 );
                        ++cnt_jjMODvl_reset;
                    }
#endif
                    if(onceI){
#if 0
                        //
                        // Hmm. # of scalar/conditional ops is rather large.
                        //      This may not even be a speedup over divmod!
                        //
                        //AsmScope const block = {{"tmp","%s40"}};
                        //fi.scope(block,"vectorized double-loop --> index vectors");
                        fi.com("jj%vl0==0 : iloop%(jj/vl0) check via cyclic tmod < jj/vl0="+jitdec(jj/vl0));
                        if(jj/vl0==2){                      // cyclic period 2 counter
                            fi.ins("sub tmod, 1,tmod",              "tmod period 2 (toggle 0|1)");
                        }else if(positivePow2(jj/vl0)){     // cyclic power-of-2 counter
                            uint64_t const shift = positivePow2Shift((uint32_t)(jj/vl0));
                            uint64_t const mask  = (1ULL<<shift) - 1U;
                            //tmod = (tmod+1) & mask;
                            fi.ins("add tm0, 1,tmod",               "tmod period jj/vl0="+jitdec(jj/vl0));
                            fi.ins("and tmod, tm0,"+jitimm(mask),   "     pow2 mask to reset");
                        }else{                              // cyclic period jj/vl0 counter
                            fi.ins("add tmod, 1,tmod",              "tmod period jj/vl0="+jitdec(jj/vl0));
                            fi.ins("sub tm0,tmod,"+jitdec(jj/vl0));
                            fi.ins("cmov.eq tmod,tm0,tm0",          "cmov reset tmod=0");
                        }
                        fi.ins("beq.w.t tmod,"+reladdr("RESET"));
                        fi.ins("vadd b, b,vl0",             "b[i] += vl0 (easy, a[] unchanged)")
                            .ins("b "+reladdr("INDUCE_DONE"), "branch around reset case");
                        fi.lab("RESET")
                            .ins("or b,0,sq",               "b[i] = sq[i] (reset case)")
                            .ins("vadd a,1,a",              "a[i] += 1");
                        fi.lab("INDUCE_DONE");
                        //fi.pop_scope();
#else
                        CBLK(fi,"// jj%vl0==0 : iloop%(jj/vl0) check via cyclic tmod < jj/vl0="<<jj/vl0);
                        if(jj/vl0==2){
                            fi<<"--tmod;";
                        }else if(positivePow2(jj/vl0)){     // cyclic power-of-2 counter
                            uint64_t const shift = positivePow2Shift((uint32_t)(jj/vl0));
                            uint64_t const mask  = (1ULL<<shift) - 1U;
                            //fi.ins("add tm0, 1,tmod",               "tmod period jj/vl0="+jitdec(jj/vl0));
                            //fi.ins("and tmod, tm0,"+jitimm(mask),   "     pow2 mask to reset");
                            CBLK(fi,"tmod = (tmod+1) & "<<jithex(mask)<<"; // cyclic power-of-2 counter");
                        }else{                              // cyclic period jj/vl0 counter
                            //fi.ins("add tmod, 1,tmod",              "tmod period jj/vl0="+jitdec(jj/vl0));
                            //fi.ins("sub tm0,tmod,"+jitdec(jj/vl0));
                            //fi.ins("cmov.eq tmod,tm0,tm0",          "cmov reset tmod=0");
                            fi  >>"++tmod;  // tmod period jj/vl0 = "<<jitdec(jj/vl0)
                                >>"if(tmod=="<<jitdec(jj/vl0)<<") tmod=0; // should generate cmov reset tmod=0?";
                        }
                        //fi.ins("beq.w.t tmod,"+reladdr("RESET"));
                        fi  >>"if(tmod == 0) goto RESET;";
                        //fi.ins("vadd b, b,vl0",             "b[i] += vl0 (easy, a[] unchanged)")
                        //    .ins("b "+reladdr("INDUCE_DONE"), "branch around reset case");
                        fi  >>"b = _ve_vaddul_vsv(vl0,b);      // b[i] += vl0 (easy, a[] unchanged)"
                            >>"goto INDUCE_DONE;";
                        //fi.lab("RESET")
                        //    .ins("or b,0,sq",               "b[i] = sq[i] (reset case)")
                        //    .ins("vadd a,1,a",              "a[i] += 1");
                        //fi.lab("INDUCE_DONE");
                        //fi.pop_scope();
                        fi  >>"RESET:"
                            >>"b = sq;                  // b[i] = sq[i] (reset case)"
                            >>"a = _ve_vaddul_vsv(1,a); // a[i] += 1"
                            >>"INDUCE_DONE:";
#endif
                    }
                }
                if( have_jjMODvl_reset ) assert( have_jjMODvl );
                assert( !have_bA_bD );
            }else if( positivePow2(jj) ){ // ------4 vec ops (add, shr, and, add)
                tr+="induce:pow2(jj): a[], b[] via shift+mask";
                if(verbose){cout<<" h";cout.flush();}
                assert( vl0%jj != 0 ); assert( jj%vl0 != 0 ); assert(have_bA_bD==1);
                assert(have_jj_shift==1); assert(have_vl_over_jj==0);
                // no...assert(have_sq==(jj>1&&jj<vl0));
                FOR(i,vl) bA[i] = vl0 + b[i];           // bA = b + vl0; add_vsv
                FOR(i,vl) bD[i] = (bA[i] >> jj_shift);  // bD = bA / jj; div_vsv
                FOR(i,vl) b [i] = (bA[i] & jj_minus_1); // bM = bA % jj; mod_vsv
                FOR(i,vl) a [i] = a[i] + bD[i]; // aA = a + bD; add_vvv
                ++cnt_bA_bD; ++cnt_jj_shift; assert( have_bA_bD );
                if(onceI){
#if 0
                    fi.ins("vaddu.l bA,"+jitdec(vl0)+",b"
                            , "bA[i] = b[i]+vl0 (jj="+jitdec(jj)+" power-of-two)")
                        .ins("vsrl.l.zx bD,bA,"+jitdec(jj_shift)
                                , "bD[i] = bA[i]>>jj_shift["+jitdec(jj_shift)+"]")
                        // note: VAND vy~M possible, while AND has sy~I, sz~M
                        .ins("vand b,"+jitimm(jj_minus_1)+",bA"
                                , "b[i] = bA[i]&jj_mask = bA[i]%"+jitdec(vl0))
                        .ins("vaddu.l a,a,bD"
                                , "a[i] += bD[i]");
#else
                    fi  >>"bA = _ve_vaddul_vsv("<<jitdec(vl0)<<",b);"
                        " // bA[i] = b[i]+vl0 (jj="<<jitdec(jj)<<" power-of-two)"
                        >>"bD = _ve_vsrl_vvs(bA,"<<jitdec(jj_shift)<<";"
                        " // bD[i] = bA[i]>>jj_shift["<<jitdec(jj_shift)<<"]"
                        >>"b  = _ve_vand_vsv("<<jithex(jj_minus_1)<<",bA);"
                        " // b[i] = bA[i]&jj_mask = bA[i]%"<<jitdec(vl0)
                        >>"a = _ve_vaddul_vvv(a,bD);   // a[i] += bD[i]"
                        ;
#endif
                }
            }else{ // div-mod ---------------------6 vec ops: add (mul,shr) (mul,sub) add
                tr+="induce: a[], b[] update via fastdiv";
                if(verbose){cout<<" i";cout.flush();}
                FOR(i,vl) bA[i] = vl0 + b[i];            // add_vsv
                FOR(i,vl) bD[i] = ((jj_M*bA[i]) >> FASTDIV_C);  // fastdiv_uB   : mul_vvs, shr_vs
                FOR(i,vl) b [i] = bA[i] - bD[i]*jj;     // long-hand    : mul_vvs, sub_vvv
                FOR(i,vl) a [i] = a[i] + bD[i];         // add_vvv
                ++cnt_bA_bD; ++cnt_jj_M;
                assert(have_bA_bD); assert(have_sq==(jj>1&&jj<vl0)); assert(!have_jj_shift);
                assert(!have_vl_over_jj); assert( jj+vl0 < (int)FASTDIV_SAFEMAX );
                if(onceI){
                    //local regs: bA,vx,bD,vy    input/output regs: a, b
                    //AsmScope const block = {{"tmp2","%s41"}};
#if 0
                    //fi.scope(block,"generic fastdiv induction");
                    if(0){//plain: output reg != input reg
                        fi.ins("vaddu bA,vl0,b",               "bA[i] = b[i] + vl0")
                            .ins("vmulu.l   vx,bA,jj_M")
                            .ins("vsrl.l.zx bD,vx,"+jitdec(FASTDIV_C), "bD[i]= bA[i]/[jj="+jitdec(jj)+"] fastdiv")
                            .ins("vmulu.l   vy,jj,bD")
                            .ins("vaddu.l   a, a,bD",          "a[i] += bD[i]")
                            .ins("vsubu.l   b, bA,vy",         "b[i] -= bD[i]*jj");
                    }else{//re-use regs
                        fi.ins("vaddu b ,vl0,b",               "b[i] = b[i] + vl0")
                            .ins("vmulu.l   bD,b ,jj_M")
                            .ins("vsrl.l.zx bD,bD,"+jitdec(FASTDIV_C), "bD[i]= bA[i]/[jj="+jitdec(jj)+"] fastdiv")
                            .ins("vmulu.l   vy,jj,bD")
                            .ins("vaddu.l   a, a,bD",          "a[i] += bD[i]")
                            .ins("vsubu.l   b, b ,vy",         "b[i] -= bD[i]*jj");
                    }
#else
                    fi  >>"b = _ve_vaddul_vsv(vl0,b);          // b[i] = b[i] + vl0"
                        >>"bD= _ve_vmulul_vsv(jj_M,b);"
                        >>"bD= _ve_vsrl_vvs(bD,FASTDIV_C);     // bD[i]= bA[i]/[jj="<<jitdec(jj)<<"]"
                        >>"vy= _ve_vmulul_vvv(jj, bD);"
                        >>"a = _ve_vaddul_vvv(a, bD);          // a[i] += bD[i]"
                        >>"b = _ve_vsubul_vvv(b, vy);          // b[i] -= bD[i]*jj"
                        ;
#endif
                }
            }
        }
#if VL_LAST_ITER==2
#if 0
        // if careful about vl (possible lower value for last iter) vs vl0 can update here:
        if( cnt + vl > iijj ){ // last time might have reduced vl
            vl = iijj - cnt;
            cout<<" vl reduced for last loop to "<<vl<<endl;
        }
#endif
        // Careful: vl = new value for last iteration, vl0 = original = old value
        if( iijj % vl0 ){
            tr+="induce:iijj%vl0 last iter LATE lower vl";
            // VE has single-op MIN
            //vl = std::min( (uint64_t)vl, /*remain =*/ iijj - cnt );
            vl = std::min( (uint64_t)vl, /*remain =*/ cnt );
            if(onceI){
                //AsmScope const block = {{"remain","%s40"}};
                //fi.scope(block, "last time reduce vl");
                //fi.ins("subu.l remain,iijj,cnt",    "remain = iijj -cnt");
                //fi.ins("mins.l vl, vl0, remain",    "vl = min(vl,remain)");
                //fi.ins("lvl vl");
                //fi.pop_scope();

                //fi.ins("mins.l vl, vl0, cnt",    "vl = min(vl,cnt)");
                //fi.ins("lvl vl");
                fi  >>"vl = (vl0<cnt? vl0: cnt); // vl = min(vl0,cnt) for last iter,"
                    >>"_ve_lvl(vl);";
            }
        }
#endif
        if(onceI){
            //fi.lcom("...KERNEL_BLOCK fall-through, a[], b[] induced");
            fi>>"// ...KERNEL_BLOCK fall-through, a[], b[] induced";
            onceI = false;
        }

KERNEL_BLOCK:
        tr+="KERNEL_BLOCK";
        if(onceK){ //cout<<"=== // #KERNEL_BLOCK: (fallthrough)\n"
            //<<"=== //        # <your code here, using a[] b[] loop-index vectors\n";
            if(nloop > 1) fk<<"KERNEL_BLOCK:"; //fk.lab("KERNEL_BLOCK");
            //fk.com("", "KERNEL BLOCK(a[],b[])", "<YOUR CODE HERE>","");
            fk  >>"//"
                >>"// KERNEL BLOCK(a[],b[])"
                >>OSSFMT("//    where a[],b[] are for(.."<<ii<<")for(.."<<jj<<"){...} indices, vl<="<<vl0)
                >>"// <YOUR CODE HERE>"
                >>"//";

            // interestingly, any re-usable data of the kernel should
            //  (somehow)
            // hoist such registers to our enclosing scope (... and maybe further)
            // Ex 1:  if b[] is const, A*b[]+C vector can be hoisted outside loops.
            //        (actually will later supply a loop-fuse that ALSO optimizes
            //         a generic lin.comb(a[]*A+b[]*B+C) for the inner loop)
            // Ex 2:  if a mask register is a function of a const b[] vector,
            //        and we have found b[] to be const (except for vl changes),
            //        the mask register can be hoisted to enclosing scope (outside loops)
            //        (perhaps a significant saving)
            // Ex 2:  sq register can be hoisted (AND combined with our sq?)
            //        instead of being recalculated
            if( HASH_KERNEL ){
                //VecHash2::kern_asm(fk,"a","b","vl","reg_hash","tmp"); 
                fk>>"// VecHash2 C kernel TBD";
            }
        }

        // KERNEL-BLOCK
        // Do something with the a[], b[] vectors of i,j loop indices...
        //cout<<"cnt "<<cnt<<" iloop "<<iloop<<" ii "<<ii<<" jj "<<jj<<endl;
        if(0){
            int const n=8; // output up-to-n [ ... [up-to-n]] ints
            int const bignum = std::max( ii, jj );
            int const wide = 1 + (bignum<10? 1: bignum <100? 2: bignum<1000? 3: 4);
            cout<<"a["<<vl<<"]="<<vecprt(n,wide,a,vl)<<endl;
            cout<<"b["<<vl<<"]="<<vecprt(n,wide,b,vl)<<endl;
        }
        // check correctness
        assert( vl == vabs[iloop].vl );
        FOR(i,vl) assert( a[i] == vabs[iloop].a[i] );
        FOR(i,vl) assert( b[i] == vabs[iloop].b[i] );
        // Alt. is a hash-value test:
        vhash.hash_combine(a.data(),b.data(),vl);
        //cout<<"iloop="<<iloop<<" vl="<<vl<<" vhash "<<vhash.u64()
        //    <<"\n ref hash "<<vabs[iloop].hash<<endl;
        assert( vhash.u64() == vabs[iloop].hash );

        onceK = false;

        // DONE_CHECK
        // XXX simplification : whole loop should be over 'remain' as
        //
        //     remain -= vl;
        //     if(remain>0) goto INDUCE
        //
        ++iloop; // needed only sometimes
        if( nloop <= 1 ){
            //cnt += vl; // for tighter exit assertion
            //cout<<" exit A cnt="<<cnt<<endl;
        }else if(iijj % vl0){
            if( vl == vl0 ){
                tr+="loop: induce next a[] b[] if vl==vl0";
                // cnt += vl; move to induce-block
                goto INDUCE;
            }
            tr+="exiting";
            // exit...
            //cout<<" exit B cnt="<<cnt<<endl;
            //cnt += vl; // just for tighter exit assertion
        }else{ // generic end-loop test
            //++cnt_use_iijj; // XXX old!
#if 0 // cnt 0..iijj
            cnt += vl0;
            if( cnt < iijj ){
                goto INDUCE;
            }
#else // cnt iijj..0
            cnt -= vl0;
            if( (int64_t)cnt > 0 ){
                tr+="loop: induce next a[] b[] if cnt-=vl0 still positive";
                //cout<<" again B cnt="<<cnt<<endl;
                goto INDUCE;
            }
            tr+="exiting B";
            //cout<<" exit B cnt="<<cnt<<endl;
#endif
        }

        if(onceL){
            if(nloop==1) fl>>"// (nloop == 1, no need to check if done)";
            else if(iijj%vl0){
                //fl.lcom("DONE_CHECK A : loop again if vl is not the reduced last value");
                //fl.ins("breq"+string(nloop>2?".t":".nt")+" vl,vl0,"+reladdr("INDUCE"), "iijj%vl!=0 so vl<vl0 means DONE");
                fl  >>"// DONE_CHECK A : loop again if vl is not the reduced last value"
                    >>"if(vl==vl0) goto INDUCE; // iijj%vl!=0 so vl<vl0 means DONE";
            }else{
                //fl.lcom("DONE_CHECK B : loop again if next cnt still < iijj");
                //  fl.ins("add cnt,vl0,cnt",  "cnt += vl");
                //      .ins("brlt"+string(nloop>2?".t":".nt")+" cnt,iijj, INDUCE-.", "next a[],b[]");
                //fl.ins("subu cnt,vl0,cnt",  "cnt -= vl (or vl0)");
                //fl.ins("bgt"+string(nloop>2?".t":".nt")+" cnt,"+reladdr("INDUCE"), "next a[],b[]");
                fl  >>"DONE_CHECK B : loop again if next cnt still < iijj"
                    >>"cnt -= vl; // (or vl0)"
                    >>"if(cnt > 0) goto INDUCE; // next a[],b[]";
            }
            onceL = false;
        }

        // LOOP_DONE ...
        if(SAVE_RESTORE_VLEN){
            //fz.ins("lvl vl_save","load VL <-- vl_save (restore VL on exit)");
            fz  >>"//_ve_lvl(vl_save); // restore VL on exit XXX when SVL op is supported!!!";
        }

#undef FOR
        cout<<" Yay! induction formulas worked! iloop,nloop="<<iloop<<","<<nloop<<endl;
        //assert( cnt == iijj || cnt == iijj/cnt*cnt );
        assert( cnt == 0 || (nloop<=1 && cnt == iijj) || (nloop>1 && cnt == iijj - iijj/vl0*vl0) ); //iijj/vl0*vl0 + vl0);
        assert( nloop == iloop );
        assert( have_vl_over_jj     == (cnt_vl_over_jj    > 0) );
        assert( have_bA_bD          == (cnt_bA_bD         > 0) );
        assert( have_sq             == (cnt_sq            > 0) );
        assert( have_jj_shift       == (cnt_jj_shift      > 0) );
        //assert( have_use_iijj       == (cnt_use_iijj      > 0) );
        assert( cnt_use_iijj == 0); // cnt iijj-->1 now
        assert( have_jj_M           == (cnt_jj_M          > 0) );
        assert( have_jjMODvl        == (cnt_jjMODvl       > 0) ); // old "special" count, case 'g' needed
        assert( have_jjMODvl_reset  == (cnt_jjMODvl_reset > 0) ); // old "special" count, case 'g' needed
        // XXX fX do not automatically form a tree. Would be nice not to have to think about
        //     ordering and where pop_scope should really occur.
        cout<<"##### Final program ################################"<<endl;
#if 0
        cout<<fd.flush();
        cout<<fp.flush();
        cout<<fi.flush();
        cout<<fk.flush();
        cout<<fl.flush();
        cout<<fz.flush();
        VecHash2::kern_asm_end(fd);
        cout<<fp.flush_all(); // kern_asm has some scope here, now
        cout<<fd.flush_all();
#else
        string prog = pr.str();
        cout<<prog;
#endif
        cout<<"##### End of program ################################"<<endl;
        // XXX multiple scope write/destroy has issues! (missing #undefs for fd right now)
        //fd.pop_scope();     // TODO: destructors should auto-pop any AsmFmtCols scopes!!!
    }
    cout<<tr.str()<<" vlen,ii,jj= "<<vlen<<" "<<ii<<" "<<jj;
    if(vl0!=vlen) cout<<" (used alt vlen "<<vl0<<")"<<endl;
    cout<<tr.dump();
}

/** opt0: print vector ops (and verify) */
void test_vloop2_unroll(Lpi const vlen, Lpi const ii, Lpi const jj)
{
    // for r in [0,h){ for c in [0,w] {...}}
    assert( vlen > 0 );
    register uint64_t iijj = (uint64_t)ii * (uint64_t)jj;
    cout<<"test_vloop2_unroll( vlen="<<vlen<<" loops 0.."<<ii<<" 0.."<<jj<<" iijj="<<iijj<<endl;

    // pretty-printing via vecprt
    int const n=8; // output up-to-n [ ... [up-to-n]] ints
    int const bignum = std::max( ii, jj );
    int const wide = 1 + (bignum<10? 1: bignum <100? 2: bignum<1000? 3: 4);

    // generate reference index outputs
    std::vector<Vab> vabs = ref_vloop2(vlen, ii, jj, 1/*verbose*/);

    register int vl = vlen;
    register uint64_t cnt=0;
    register uint64_t nxt;
    cout<<"=== // unrolled regs:"<<endl;
    cout<<"=== //        %iijj    : scalar : outer * inner fused-loop count"<<endl;
    cout<<"=== //        %cnt     : scalar : count 0..iijj-1"<<endl;
    cout<<"=== //        %vl      : scalar : vector length register"<<endl;
    cout<<"=== //        %a, %b   : vector : outer, inner loop indices"<<endl;
    cout<<"=== //        %bA, %bD : vector : tmp regs"<<endl;
    //if (vl > iijj) vl = iijj; //in general: if (cnt+vl > iijj) vl=iijj-cnt;
    // later! cout<<"===   lea %vl, "<<vlen<<"(,0) // initial vector len"<<endl;
    cout<<"===   lea %iijj, 0,"<<ii<<"(,0)"<<endl;
    cout<<"===   lea %vl,   0,"<<jj<<"(,0) // vl used as tmp reg"<<endl;
    cout<<"===   mulu.l %iijj, %iijj, %vl  // opt last 3 for small ii, jj !"<<endl;
    cout<<"===   or %cnt, 0, 0(,0)"<<endl;

    VVlpi a(vl), b(vl);   // vectorized loop indices
    VVlpi bA(vl), bD(vl); // local temporaries
#define FOR(I,VL) for(int I=0;I<VL;++I)
    int iloop = 0; // just for debug checks, now;
    for( ; cnt < iijj; cnt += vl)
    {
        if (cnt == 0){
            // now load the initial vector-loop registers:
            FOR(i,vl) bA[i] = i;         // vseq_v
            cout<<"===   vseq %bA           // (tmp)"<<endl;
            FOR(i,vl) b [i] = bA[i] % jj;
            FOR(i,vl) a [i] = bA[i] / jj;
            cout<<"===   vmod %b, %bA, "<<jj<<"     // b = bA % jj"<<endl;
            cout<<"===   vdiv %a, %bA, "<<jj<<"     // a = bA / jj"<<endl;
        }else{
            // 2. Induction from a->ax, b->bx
            FOR(i,vl) bA[i] = vl + b[i];      // bA = b + vl; add_vsv
            FOR(i,vl) b [i] = bA[i] % jj;     // bM = bA % jj; mod_vsv
            FOR(i,vl) bD[i] = bA[i] / jj;     // bD = bA / jj; div_vsv
            FOR(i,vl) a [i] = a[i] + bD[i];   // aA = a + bD; add_vvv
            cout<<"===   vadd %bA, %vl, %b     // bA[i] = vl + b[i] (tmp)"<<endl;
            cout<<"===   vmod %b , %bA, "<<jj<<"     // b[i] = bA[i] % jj"<<endl;
            cout<<"===   vdiv %bD, %bA, "<<jj<<"     // bD[i] = bA[i]/ jj (tmp)"<<endl;
            cout<<"===   vadd %a , %a , %bD    // a[i] += bD[i]"<<endl;
        }

        nxt = cnt+vl;
        if( nxt > iijj){ // last loop!
            vl = iijj - cnt;
            if (cnt > 0){
                cout<<"=== lea    %vl, "<<vl<<"(,0)  // LAST TIME"<<endl;
                cout<<"=== lvl    %vl"<<endl;
            }
        }

        //cout<<"==="<<endl;
        cout<<"=== // vec_loop2 unroll#"<<iloop<<", indices in %a, %b"<<endl;
        //cout<<"==="<<endl;

        cout<<"__"<<iloop<<endl;
        cout<<"Induce:      "<<vecprt(n,wide,a,vl)<<endl;
        cout<<"             "<<vecprt(n,wide,b,vl)<<endl;
        FOR(i,vl) assert( a[i] == vabs[iloop].a[i] );
        FOR(i,vl) assert( b[i] == vabs[iloop].b[i] );
        ++iloop; // just for above debug assertions
#undef FOR
    }
    cout<<" Yay! induction formulas worked!"<<endl;
}
void test_vloop2_no_unroll(Lpi const vlen, Lpi const ii, Lpi const jj)
{
    // for r in [0,h){ for c in [0,w] {...}}
    assert( vlen > 0 );
    cout<<"test_vloop2_nounroll( vlen="<<vlen<<" loops 0.."<<ii<<" 0.."<<jj<<endl;

    // pretty-printing via vecprt
    int const n=8; // output up-to-n [ ... [up-to-n]] ints
    int const bignum = std::max( ii, jj );
    int const wide = 1 + (bignum<10? 1: bignum <100? 2: bignum<1000? 3: 4);

    // generate reference index outputs
    std::vector<Vab> vabs = ref_vloop2(vlen, ii, jj, 1/*verbose*/);

    cout<<"=== // no-unroll regs (generic loop):"<<endl;
    cout<<"=== //        %iijj    : scalar : outer * inner fused-loop count"<<endl;
    cout<<"=== //        %cnt     : scalar : count 0..iijj-1"<<endl;
    cout<<"=== //        %vl      : scalar : vector length register"<<endl;
    cout<<"=== // Oh. Fully generic would need ii and jj in scalar regs too"<<endl;
    cout<<"=== //                 (ii could be re-used for iijj)"<<endl;
    cout<<"=== //        %a, %b   : vector : outer, inner loop indices"<<endl;
    cout<<"=== //        %bA, %bD : vector : tmp regs"<<endl;
    cout<<"=== // scalar init:"<<endl;
    register uint64_t iijj = (uint64_t)ii * (uint64_t)jj;
    register int vl = vlen;
    register uint64_t cnt = 0UL;
    if ((uint64_t)vl > iijj) vl = iijj; //in general: if (cnt+vl > iijj) vl=iijj-cnt;
    cout<<"===   lea %iijj, 0,"<<ii<<"(,0)"<<endl;
    cout<<"===   lea %vl,   0,"<<jj<<"(,0) // vl used as tmp reg"<<endl;
    cout<<"===   mulu.l %iijj, %iijj, %vl  // opt last 3 for small ii, jj !"<<endl;
    cout<<"===   or %cnt, 0, 0(,0)"<<endl;
    cout<<"===   lea %vl, "<<vlen<<"(,0) // initial vector len"<<endl;

    register uint64_t nxt; // loop variable (convenience) XXX
#define FOR(I,VL) for(int I=0;I<VL;++I)
    cout<<"=== // vector init:"<<endl;
    VVlpi a(vl), b(vl);   // vectorized loop indices
    VVlpi bA(vl), bD(vl); // local temporaries
    // now load the initial vector-loop registers:
    FOR(i,vl) bA[i] = i;         // vseq_v
    FOR(i,vl) b [i] = bA[i] % jj;
    FOR(i,vl) a [i] = bA[i] / jj;
    cout<<"===   vseq %bA           // (tmp)"<<endl;
    cout<<"===   vmod %b, %bA, "<<jj<<"     // b = bA % jj"<<endl;
    cout<<"===   vdiv %a, %bA, "<<jj<<"     // a = bA / jj"<<endl;
    cout<<"===   br loop_enter"<<endl;
    int iloop = 0; // just for debug checks, now;
    cout<<"=== again:              // <-- repeat loop (induce loop var)"<<endl;
again:
    nxt = cnt+vl;
    if( nxt > iijj) vl = iijj - cnt;
    if (iloop == 0){ // only need to print stuff 1st time through loop
        cout<<"===   addu.l %tmp, %cnt, %vl"<<endl;
        cout<<"===   subu.l %tmp, %tmp, %iijj"<<endl;
        cout<<"===   br.lt %tmp, L_1f-."<<endl;
        cout<<"===     or %cnt, 0, 1(0)  // re-use cnt to signal loop-end"<<endl;
        cout<<"===     subu %vl, %iijj, %cnt"<<endl;
        cout<<"===     lvl  %vl"<<endl;
        cout<<"=== L_1f:"<<endl;
    }

    //cout<<"==="<<endl;
    cout<<"=== // vec_loop2 #"<<iloop<<", indices in %a, %b"<<endl;
    //cout<<"==="<<endl;

    cout<<"__"<<iloop<<endl;
    cout<<"Induce:      "<<vecprt(n,wide,a,vl)<<endl;
    cout<<"             "<<vecprt(n,wide,b,vl)<<endl;
    FOR(i,vl) assert( a[i] == vabs[iloop].a[i] );
    FOR(i,vl) assert( b[i] == vabs[iloop].b[i] );
    cnt += vl; // or cnt = nxt;
    if (cnt >= iijj) goto done;
    if (iloop == 0){
        cout<<"=== // if ((cnt+=vl) > iijj) goto done;"<<endl;
        cout<<"===   addu.l %cnt, %cnt, %vl"<<endl;
        cout<<"===   subu.l %tmp, %cnt, %iijj"<<endl;
        cout<<"===   br.ge done"<<endl;
    }
    // 2. Induction from a->ax, b->bx
    FOR(i,vl) bA[i] = vl + b[i];      // bA = b + vl; add_vsv
    FOR(i,vl) b [i] = bA[i] % jj;     // bM = bA % jj; mod_vsv
    FOR(i,vl) bD[i] = bA[i] / jj;     // bD = bA / jj; div_vsv
    FOR(i,vl) a [i] = a[i] + bD[i];   // aA = a + bD; add_vvv
    if (iloop==0){
        cout<<"=== // Induce next a, b vector loop indices"<<endl;
        cout<<"===   vadd %bA, %vl, %b     // bA[i] = vl + b[i]"<<endl;
        cout<<"===   vmod %b , %bA, "<<jj<<"   // b[i] = bA[i] % jj"<<endl;
        cout<<"===   vdiv %bD, %bA, "<<jj<<"   // bD[i] = bA[i]/ jj"<<endl;
        cout<<"===   vadd %a , %a , %bD    // a[i] += bD[i]"<<endl;
    }
    ++iloop; // just for above debug assertions
    goto again;
#undef FOR
done:
    cout<<"=== done:"<<endl;
    cout<<" Yay! induction formulas worked!"<<endl;
}

void other_fastdiv_methods(int const jj){    
    int const verbose=1;
    // other fast divide approaches...
    if(verbose>=1 && !positivePow2(jj)){
        // libdivide relies on MULHI operation, which we don't have. It sometimes needs
        // more ops, but for Aurora would be correct for larger (32-bit) input range.
        magicu_info bogus = {0,0,0,0};
        assert( sizeof(uint)*CHAR_BIT == 32 );
        auto const ld = positivePow2(jj) ? bogus: compute_unsigned_magic_info( jj, 32 );
        if( ld.pre_shift==0 ){
            // NO assert( ld.post_shift==1 ); sometimes 1 or 2
            // NO  assert( ld.post_shift==0 || ld.post_shift==1 || ld.post_shift==2 );
            if( ld.post_shift==0 ){
                // never happens?
                cout<<" --> no pre or post-shift, increment="<<ld.increment
                    <<", mul="<<(void*)(intptr_t)(intptr_t)(intptr_t)ld.multiplier;
            }else{
                cout<<" --> no pre-shift, post-shift="<<ld.post_shift
                    <<",increment="<<ld.increment
                    <<", mul="<<(void*)(intptr_t)ld.multiplier;
            }
        }else{
            cout<<" OH?? ";
            cout<<" --> pre-shift="<<ld.pre_shift<<", post-shift="<<ld.post_shift
                <<",increment="<<ld.increment
                <<", mul="<<(void*)(intptr_t)ld.multiplier;
        }
    }
    if(verbose>=1){
        // Note: Aurora has shift-LEFT-add but no mul-add or shift-right-add for int vectors
        struct fastdiv jj_fastdiv;
        fastdiv_make( &jj_fastdiv, (uint32_t)jj );
        cout<<endl<<"\t"
            <<" mul,add,shr="<<(void*)(intptr_t)jj_fastdiv.mul
            <<","<<jj_fastdiv.add<<","<<jj_fastdiv.shift;
    }
    if(verbose>=1){
        Ulpi jj_mod_inverse_lpi   = mod_inverse((Ulpi)jj);
        Uvlpi jj_mod_inverse_Vlpi = mod_inverse((Uvlpi)jj);
        cout<<" jj_modinv="<<(void*)(intptr_t)jj_mod_inverse_Vlpi
            <<" or "<<(void*)(intptr_t)jj_mod_inverse_lpi;
    }
}
int main(int argc,char**argv){
    int vl = 8;
    int h=20, w=3;
    int opt_t=1, opt_u=0, opt_l=0, opt_h=0, opt_m=0;
    int a=0;
#if 0
    cout<<"jitimm ...";
    cout<<"jitimm(0) = "<<jitimm(0)<<endl;
    cout<<"jitimm(1) = "<<jitimm(1)<<endl;
    cout<<"jitimm(-1) = "<<jitimm(-1)<<endl;
    cout<<"popcount(0) = "<<popcount(0)<<endl;
    cout<<"popcount(1) = "<<popcount(1)<<endl;
    cout<<"popcount(-1) = "<<popcount(-1)<<endl;
    cout<<std::hex;
    for(uint64_t i=1,j=1; i<64; j+=j, ++i){
        cout<<" jitimm("<< j-1    <<") = "<<jitimm(j-1)<<endl;
        cout<<" jitimm("<< ~(j-1) <<") = "<<jitimm(~(j-1))<<endl;
    }
    cout<<std::dec;
#endif

    if(argc > 1){
        // actually only the last -[tlu] option is used
        for( ; a+1<argc && argv[a+1][0]=='-'; ++a){
            char *c = &argv[1][1];
            for( ; *c != '\0'; ++c){
                if(*c=='h'){
                    cout<<" fuse2lin [-h|t|l|u] VLEN H W"<<endl;
                    cout<<"  -t    test correctness + VE intrinsics code (no unroll)"<<endl;
                    cout<<"  -a    -t with alternate strategy (lower vl)"<<endl;
                    cout<<"  -q    quick-test correctness (simulate using C++ vectors)"<<endl;
                    cout<<"  -l    pseudo-asm for loops (+correctness)"<<endl;
                    cout<<"  -u    [WIP] pseudo-asm-code for unrolled loops (+correctness)"<<endl;
                    cout<<"  -m    try for extended-range (a/d) ~ a*M>>N forms"<<endl;
                    cout<<"  -h    this help"<<endl;
                    cout<<"   VLEN = vector length"<<endl;
                    cout<<"   I    = 1st loop a=0..i-1"<<endl;
                    cout<<"   J    = 2nd loop b=0..j-1"<<endl;
                    cout<<" double loop --> loop over vector registers a[VLEN], b[VLEN]"<<endl;
                    opt_h = 1;
                }else if(*c=='t'){ opt_t=2; opt_l=0; opt_u=0;
                }else if(*c=='a'){ opt_t=3; opt_l=0; opt_u=0;
                }else if(*c=='q'){ opt_t=1; opt_l=0; opt_u=0;
                }else if(*c=='l'){ opt_t=0; opt_l=1; opt_u=0;
                }else if(*c=='u'){ opt_t=0; opt_l=0; opt_u=1;
                }else if(*c=='m'){ opt_m=1;
                }
            }
        }
    }
    cout<<" args: a = "<<a<<" opt_t="<<opt_t<<" opt_u="<<opt_u<<" opt_l="<<opt_l<<" opt_m="<<opt_m<<endl;
    if(argc > a+1) vl = atof(argv[a+1]);
    if(argc > a+2) h  = atof(argv[a+2]);
    if(argc > a+3) w  = atof(argv[a+3]);
    cout<<"vlen="<<vl<<", h="<<h<<", w="<<w<<endl;

    if(opt_m){
        // removed test_mod_inverse<uint32_t>();
        //test_mod_inverse<uint64_t>();
        cout<<" (mod_inverse OK)";
        exit(0);
    }

    // INCORRECT verify1();
    //cout<<" verify1 OK"<<endl;

    if(opt_h == 0){
        if(opt_t==1) test_vloop2(vl,h,w);
        else if(opt_t==2||opt_t==3) test_vloop2_no_unrollX(vl,h,w,opt_t);
        if(opt_u) test_vloop2_unroll(vl,h,w);
        if(opt_l) test_vloop2_no_unroll(vl,h,w); // C++ code more like asm generic loop
    }
    cout<<"\nGoodbye"<<endl;
    return 0;
}
// vim: ts=4 sw=4 et cindent cino=^=l0,\:.5s,=-.5s,N-s,g.5s,b1 cinkeys=0{,0},0),\:,0#,!^F,o,O,e,0=break
