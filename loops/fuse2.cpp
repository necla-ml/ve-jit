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
#include <vector>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <algorithm> // ve finds std::max here!
#include <type_traits>
#include <unordered_set>
#include <map>

#include <cstring>
#include <cstddef>
#include <cassert>

using namespace std;

typedef int Lpi; // Loop-index type
// Note other optimization might *pack* 2-loop indices differently!
// (e.g. u32 a[0]b[0] a[1]b[1] in a packed register)
// (e.g. single register with a[] concat b[] (for short double-loops)
// Here we just store a[], b[] indices in two separate registers
typedef uint64_t Vlpi; // vector-loop-index type
typedef std::vector<Vlpi> VVlpi;

typedef make_unsigned<Lpi>::type Ulpi;
typedef make_unsigned<Vlpi>::type Uvlpi;

/** Reference values for correct index outputs */
struct Vab{
    Vab( VVlpi const& asrc, VVlpi const& bsrc, int vl )
        : a(asrc), b(bsrc), vl(vl) {}
    VVlpi a;
    VVlpi b;
    int vl;    // 0 < Vabs.back().vl < vlen
};

// - precalc may save ops when done outside some external enclosing loops
//   - no precalc: induction always via formula
//   - precalc and [partial?] unroll : induction by register sequence (maybe looped)
// - unroll can be:
//   - never                            // nloop = 1
//   - full                             // full precalc unroll (low nloop)
//   - suggested value (nloop,bcyc_regs,...)
//   - any small unroll (e.g. 8)        // using some [fast?] induction formula
enum Unroll {
    UNR_NLOOP1,   ///< precalc, never unroll
    UNR_VLMODJJ,  ///< no precalc, any small unroll
    // jj%vl == 0
    UNR_JJMODVL_NORESET,  ///< no precalc, any small unroll
    UNR_JJMODVL_RESET,  ///< XXX check for bcyc_regs or nloop XXX
    // isPositivePow2(jj)
    UNR_JJPOW2_NLOOP, ///< no precalc, unroll by nloop (full unrol)
    UNR_JJPOW2_CYC,   ///< precalc b[] (and a[] for a+=const?) [partial?] unroll by bcyc_regs
    UNR_JJPOW2_BIG,   ///< no precalc, any small unroll
    UNR_NLOOP,        ///< precalc, full unroll is small
    UNR_CYC,          ///< precalc, [partial?] unroll by cyclic for b[] (and a?)
    // generic div-mod
    UNR_DIVMOD        ///< no precalc, any small unroll
};

struct Unroll_data {
    /** where did we come from? */
    int vl, ii, jj, b_period_max;
    /** what class of unrolling is suggested? */
    enum Unroll suggested;

    /** \c nloop is a multiple of \c unroll for partial unroll cases,
     * equal for full unroll,
     * but is untied for <em>any small unroll</em> case. */
    int nloop;
    /** explicit unrolling factor may be given, possibly < nloop. */
    int unroll;
    /** precalculated data vectors.
     * - if !pre[].empty()
     *   - pre.size() == 1             (init-phase uses vpre[0], any unroll/nloop)
     *   - or unroll == pre.size() > 1 (induction-phase uses vpre data)
     * - otherwise we can give a formula-based unroll>0 using no recalc
     *   - <nloop: partial unroll (still have an enclosing loop or a finishing step)
     *   - ==nloop: full unroll
     * - any unroll may or may not use pre.a[] or vpre.b[] values
     *   - pre data can be used for init- or induction-phase.
     *
     * \todo see whether init-precalc and ind-precalc can actually share pre[0]
     */
    std::vector<Vab> pre;
};

/** string up-to-n first, dots, up-to-n last of vector \c v[0..vl-1] w/ \c setw(wide) */
template<typename T>
std::string vecprt(int const n, int const wide, std::vector<T> v, int const vl){
    assert( v.size() >= (size_t)vl );
    std::ostringstream oss;
    for(int i=0; i<vl; ++i){
        if( i < n ){ oss<<" "<<setw(wide)<<v[i]; }
        if( i == n && i < vl-n ){ oss<<" ... "; }
        if( i >= n && i >= vl-n ){ oss<<" "<<setw(wide)<< v[i]; }
    }
    return oss.str();
}
inline bool constexpr positivePow2(uint64_t v) {
    return ((v & (v-1)) == 0);
}
static const int MultiplyDeBruijnBitPosition2[32] = 
{
    0, 1, 28, 2, 29, 14, 24, 3, 30, 22, 20, 15, 25, 17, 4, 8, 
    31, 27, 13, 23, 21, 19, 16, 7, 26, 12, 18, 6, 11, 5, 10, 9
};
inline int /*constexpr*/ positivePow2Shift(uint32_t v) {
    //assert( positivePow2(v)
    return MultiplyDeBruijnBitPosition2[(uint32_t)(v * 0x077CB531U) >> 27];
}

/** greatest common denominator, a,b > 0 */
inline int gcd(int a, int b)
{
    for (;;)
    {
        if (a == 0) return b;
        b %= a;
        if (b == 0) return a;
        a %= b;
    }
}

/** lowest common multiple, a,b > 0 */
inline int lcm(int a, int b)
{
    int temp = gcd(a, b);

    return temp ? (a / temp * b) : 0;
}

/** Solve for integers j, k and g=gcd(a,b) such that ja + ky = g,
 * where a,b are integer constants.
 * This is a linear equation in integers, and is solved
 * via the extended Euclid algorithm.
 */
static void inline extendedEuclid( int& k, int a, int& j, int b, int& g)
{
  int x = 1, y = 0;
  int xLast = 0, yLast = 1;
  int q, r, m, n;
  while (a != 0)
  {
    q = b / a;
    r = b % a;
    m = xLast - q * x;
    n = yLast - q * y;
    xLast = x;
    yLast = y;
    x = m;
    y = n;
    b = a;
    a = r;
  }
  g = b;
  k = xLast;
  j = yLast;
}


#include "divide_by_constants_codegen_reference.c"

#include "../regs/throw.hpp"
#include <typeinfo>
template<typename T> T mod_inverse( T const a ){
    THROW("mod_inverse<T> not implemented for "<<typeid(T).name());
    return T(0);
}
template<>
inline uint32_t mod_inverse<uint32_t>(uint32_t const a)
{
    uint32_t u = 2-a;
    uint32_t i = a-1;
    i *= i; u *= i+1;
    i *= i; u *= i+1;
    i *= i; u *= i+1;
    i *= i; u *= i+1;
    return u;
}
template<> inline int32_t mod_inverse<int32_t>(int32_t const a)
{
    return mod_inverse((uint32_t)a);
}
template<>
inline uint64_t mod_inverse<uint64_t>(uint64_t const a)
{
    uint64_t u = 2-a;
    uint64_t i = a-1;
    i *= i; u *= i+1;
    i *= i; u *= i+1;
    i *= i; u *= i+1;
    i *= i; u *= i+1;
    i *= i; u *= i+1;   // one extra?
    return u;
}
template<> inline int64_t mod_inverse<int64_t>(int64_t const a)
{
    return mod_inverse((uint64_t)a);
}
/** Generate reference vectors of vectorized 2-loop indices */
std::vector<Vab> ref_vloop2(Lpi const vlen, Lpi const ii, Lpi const jj,
                            int const verbose=1)
{
    std::vector<Vab> vabs; // fully unrolled set of reference pairs of a,b vector register

    VVlpi a(vlen), b(vlen);
    int v=0; // 0..vlen counter
    for(int64_t i=0; i<(int64_t)ii; ++i){
        for(int64_t j=0; j<(int64_t)jj; ++j){
            //cout<<"."; cout.flush();
            a[v] = i; b[v] = j;
            if( ++v >= vlen ){
                vabs.emplace_back( a, b, v );
                v = 0;
            } 
        }
    }
    cout<<vabs.size()<<" full loops of "<<vlen<<" with "<<v<<" left over"<<endl;
    if( v > 0 ){ // partial final vector
        for(int i=v; i<vlen; ++i) { a[i] = b[i] = 0; }
        vabs.emplace_back( a, b, v );
    }

    if(verbose){ // print ref result
        // pretty-printing via vecprt
        int const n=8; // output up-to-n [ ... [up-to-n]] ints
        int const bignum = std::max( ii, jj );
        int const wide = 1 + (bignum<10? 1: bignum <100? 2: bignum<1000? 3: 4);

        for(size_t l=0; l<vabs.size(); ++l){
            auto const& a = vabs[l].a;
            auto const& b = vabs[l].b;
            auto const& vl = vabs[l].vl;
            cout<<"__"<<l<<endl;
            cout<<"a_"<<l<<"["<<vl<<"]="<<vecprt(n,wide,a,vl)<<endl;
            cout<<"b_"<<l<<"["<<vl<<"]="<<vecprt(n,wide,b,vl)<<endl;
        }
    }
    return vabs;
}
// see https://github.com/lemire/constantdivisionbenchmarks, "lkk" algorithm.
// Issue #1 comments on 64-bit-only version similar to:
#define B 21 /* B up to 21 is OK to keep all intermediates in 64-bit range */
/** *_uB functions were verified for its SAFEMAX (took a while to run).
 * computeM_uB is two ops: divide + increment.  This might be fast enough
 * to precalculate routinely at runtime for use in loops. */
#define SAFEMAX ((1U<<B)-1U)
#define C (2*B)
static inline uint64_t constexpr computeM_uB(uint32_t d) {
      return ((UINT64_C(1)<<C)-1) / d + 1;
}
/** fastdiv_uB computes (a / d) given precomputed M for d>1.
 * \pre a,d < (1<<21). */
static inline constexpr uint32_t fastdiv_uB(uint32_t const a, uint64_t const M) {
    return M*a >> C; // 2 ops: mul, shr
}
/** fastmod_uB computes (a % d) given precomputed M.
 * Probably nicer to calc. uint64_t D=a/d using fastdiv_uB, and then R=a-D*d.
 * (retain "everythin" in u64 registers).
 * \pre a,d < (1<<21). */
static inline uint32_t constexpr fastmod_uB(uint32_t const a, uint64_t const M, uint32_t const d) {
    // ugly with 'lowbits' approach, nicer to just do the fastdiv followed by mul and sub :(
    return a - (M*a>>C)*d; // 3-4 ops: mul, shr, mul-sub... AND get a/d too (i.e. divmod adds 1 op to the division)
}
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

enum Unroll unroll_suggest( int const vl, int const ii, int const jj, int const nloop, int const b_period_max );

void other_fastdiv_methods(int const jj);

// NOTE: we do a ">>C", which we can't just elide on Aurora, even if C==16

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
    int const verbose=1;
    assert( vlen > 0 );
    register uint64_t iijj = (uint64_t)ii * (uint64_t)jj;
    cout<<"test_vloop2( vlen="<<vlen<<" loops 0.."<<ii<<" 0.."<<jj<<" iijj="<<iijj;

    register int vl = vlen;
    //if (cnt+vl > iijj) vl = iijj - cnt;  // simplifies for cnt=0
    if ((uint64_t)vl > iijj) vl = iijj;
    int const vl0 = vl; // debug
    int const nloop = (iijj+vl-1) / vl;    // div_round_up(iijj,vl)
    cout<<" vl="<<vl<<" nloop="<<nloop<<endl;
    assert( (nloop > 1) == ((uint64_t)vl < iijj) );
    assert( vl0 > 0 );
    assert( ii > 0 );
    assert( jj > 0 );

    // pretty-printing via vecprt
    //int const verbose = 1; // verbose
    //int const n=8; // output up-to-n [ ... [up-to-n]] ints
    //int const bignum = std::max( ii, jj );
    //int const wide = 1 + (bignum<10? 1: bignum <100? 2: bignum<1000? 3: 4);

    // Have reference vabs vectors. Now we try induction way.
    // 1. initialize: could copy vabs[0] from const data storage, or...
    //   - generate from seq + divmod.
    //   - 2-loop induction uses 3 scalar registers:
    //     - \c cnt 0.. \c iijj, and \c vl (for jit, iijj is CCC (compile-time-const))
    //     - get final \c vl from cnt, vl and iij)
#define FOR(I,VL) for(int I=0;I<VL;++I)

    cout<<"Verify-------"<<endl;
    // generate reference index outputs
    std::vector<Vab> vabs = ref_vloop2(vlen, ii, jj, 0/*verbose*/);
    assert( vabs.size() > 0 );
    assert(vabs.size() == (size_t)(((ii*jj) +vlen -1) / vlen));
    if(verbose>=2) cout<<"   vl="<<vl<<"   ii="<<ii<<"   jj="<<jj
        <<"   iijj="<<iijj<<" vabs.size() = "<<vabs.size()<<endl;

    if(verbose>=1) other_fastdiv_methods(jj);

    // various misc precalculated consts and declarations.
    VVlpi a(vl), b(vl);                 // calculated loop index vectors
    VVlpi bA(vl), bD(vl), sq(vl);       // internal vectors
    // bA and bD are used when:
    //   iijj > vl && jj%vl!=0
    // sq is used when:
    //   iloop==0:   jj>1 && vl>=jj
    //   iloop >0:   jj%vl==0 && (special: iloop can be >= (jj/vl))
    int iloop = 0;
    assert( nloop >= 1 );
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
        cout<<" jj_M="<<(void*)(intptr_t)jj_M<<" shift="<<C;
    }
    cout<<endl;
    
#if 0
    // NB: common operation is divmod(v,s,vM,vD) : v--> v%s, v/s,
    //     which has some optimizations for nice values of jj.
    // C++14: &vl=std::as_const(vl)
    auto v_divmod_vs = [&vl,&jj,&jj_M](/* in*/ VVlpi const& a, Vlpi const d, /*out*/ VVlpi& div, VVlpi& mod){
        assert( (Ulpi)jj < SAFEMAX ); FOR(i,vl) assert( (Uvlpi)a[i] <= SAFEMAX );
        FOR(i,vl) div[i] = jj_M * a[i] >> C;
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
    unroll_suggest( vl,ii,jj, nloop, b_period_max );

    register uint64_t cnt = 0UL;
    for( ; cnt < iijj; cnt += vl )
    {
        //cout<<"cnt "<<cnt<<" iloop "<<iloop<<" ii "<<ii<<" jj "<<jj<<endl;
        if (iloop == 0){
            if(nloop==1) assert(have_vl_over_jj==0);
            // now load the initial vector-loop registers:
            // sq[i] and jj are < SAFEMAX, so we can avoid % and / operations
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
                FOR(i,vl) a[i] = jj_M * sq[i] >> C;
                FOR(i,vl) b[i] = sq[i] - a[i]*jj;
                //  OK since sq[] and jj both <= SAFEMAX [(1<<21)-1]
                assert( (uint64_t)jj+vl <= (uint64_t)SAFEMAX );
                // use mul_add_shr (fastdiv) approach if jj+vl>SAFEMAX (1 more vector_add_scalar)
                if(nloop<=1) assert(have_bA_bD==0); assert(have_sq==1); assert(have_jj_shift==0);
                ++cnt_sq; ++cnt_jj_M;
            }
        }else{
            assert( nloop > 1 );
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
                FOR(i,vl) bA[i] = vl0 + b[i];            // bA = b + vl; add_vsv
                FOR(i,vl) bD[i] = (bA[i] >> jj_shift);  // bD = bA / jj; div_vsv
                FOR(i,vl) b [i] = (bA[i] & jj_minus_1); // bM = bA % jj; mod_vsv
                FOR(i,vl) a [i] = a[i] + bD[i]; // aA = a + bD; add_vvv
                ++cnt_bA_bD; ++cnt_jj_shift; assert( have_bA_bD );
            }else{ // div-mod ---------------------6 vec ops: add (mul,shr) (mul,sub) add
                if(verbose){cout<<" i";cout.flush();}
                FOR(i,vl) bA[i] = vl0 + b[i];            // add_vsv
                FOR(i,vl) bD[i] = ((jj_M*bA[i]) >> C);  // fastdiv_uB   : mul_vvs, shr_vs
                FOR(i,vl) b [i] = bA[i] - bD[i]*jj;     // long-hand    : mul_vvs, sub_vvv
                FOR(i,vl) a [i] = a[i] + bD[i];         // add_vvv
                ++cnt_bA_bD; ++cnt_jj_M;
                assert(have_bA_bD); assert(have_sq==(jj>1&&jj<vl)); assert(!have_jj_shift);
                assert(!have_vl_over_jj); assert( jj+vl < (int)SAFEMAX );
            }
        }

        // Note: vl reduction must take place AFTER above use of "long" vl
        if( cnt + vl > iijj ){ // last time might have reduced vl
            vl = iijj - cnt;
            cout<<" vl reduced for last loop to "<<vl<<endl;
        }

        if(verbose>=1){
            int const n=8; // output up-to-n [ ... [up-to-n]] ints
            int const bignum = std::max( ii, jj );
            int const wide = 1 + (bignum<10? 1: bignum <100? 2: bignum<1000? 3: 4);
            cout<<"a["<<vl<<"]="<<vecprt(n,wide,a,vl)<<endl;
            cout<<"b["<<vl<<"]="<<vecprt(n,wide,b,vl)<<endl;
        }

        FOR(i,vl) assert( a[i] == vabs[iloop].a[i] );
        FOR(i,vl) assert( b[i] == vabs[iloop].b[i] );
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

void test_vloop2_no_unrollX(Lpi const vlen, Lpi const ii, Lpi const jj){ // for r in [0,h){ for c in [0,w] {...}}
    int verbose=1;
    assert( vlen > 0 );
    register uint64_t iijj = (uint64_t)ii * (uint64_t)jj;
    cout<<"test_vloop2( vlen="<<vlen<<" loops 0.."<<ii<<" 0.."<<jj<<" iijj="<<iijj;

    register int vl = vlen;
    //if (cnt+vl > iijj) vl = iijj - cnt;  // simplifies for cnt=0
    if ((uint64_t)vl > iijj) vl = iijj;
    int const vl0 = vl; // debug
    int const nloop = (iijj+vl-1) / vl;    // div_round_up(iijj,vl)
    cout<<" vl="<<vl<<" nloop="<<nloop<<endl;
    assert( (nloop > 1) == ((uint64_t)vl < iijj) );
    assert( vl0 > 0 );
    assert( ii > 0 );
    assert( jj > 0 );


    // Have reference vabs vectors. Now we try induction way.
    // 1. initialize: could copy vabs[0] from const data storage, or...
    //   - generate from seq + divmod.
    //   - 2-loop induction uses 3 scalar registers:
    //     - \c cnt 0.. \c iijj, and \c vl (for jit, iijj is CCC (compile-time-const))
    //     - get final \c vl from cnt, vl and iij)
#define FOR(I,VL) for(int I=0;I<VL;++I)

    cout<<"Verify-------"<<endl;
    // generate reference index outputs
    std::vector<Vab> vabs = ref_vloop2(vlen, ii, jj, 1/*verbose*/);
    assert( vabs.size() > 0 );
    assert(vabs.size() == (size_t)(((ii*jj) +vlen -1) / vlen));
    if(verbose>=2) cout<<"   vl="<<vl<<"   ii="<<ii<<"   jj="<<jj
        <<"   iijj="<<iijj<<" vabs.size() = "<<vabs.size()<<endl;

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
    cout<<" jj="<<jj;
    int jj_shift=0;
    Vlpi jj_minus_1 = 0;
    if( positivePow2(jj) ){
        jj_shift = positivePow2Shift((uint32_t)jj);
        jj_minus_1 = jj - 1;
        assert( (uint64_t)1<<jj_shift == (uint64_t)jj );
        cout<<" power of two shift is "<<jj_shift<<"    mask is "<<jj_minus_1;
    }else{
        cout<<" jj_M="<<(void*)(intptr_t)jj_M<<" shift="<<C;
    }
    cout<<endl;
    
#if 0
    // NB: common operation is divmod(v,s,vM,vD) : v--> v%s, v/s,
    //     which has some optimizations for nice values of jj.
    // C++14: &vl=std::as_const(vl)
    auto v_divmod_vs = [&vl,&jj,&jj_M](/* in*/ VVlpi const& a, Vlpi const d, /*out*/ VVlpi& div, VVlpi& mod){
        assert( (Ulpi)jj < SAFEMAX ); FOR(i,vl) assert( (Uvlpi)a[i] <= SAFEMAX );
        FOR(i,vl) div[i] = jj_M * a[i] >> C;
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
    Vlpi const jj_over_vl       = (have_jjMODvl_reset? jj/vl0: 0);
    uint64_t const jj_over_vl_M = (have_jjMODvl_reset? computeM_uB(jj_over_vl): 0);
    assert( !(have_vl_over_jj && have_jjMODvl) ); // never need both these constants

    cout<<"=== // no-unroll regs (generic loop):"<<endl;
    cout<<"=== //        %cnt        : scalar : count 0..iijj-1"<<endl;
    cout<<"=== //        %iloop?     : scalar : count 0..iijj-1 by vl"<<endl;
    cout<<"=== //        %vl         : scalar : vector length register"<<endl;
    cout<<"=== // Oh. Fully generic would need ii and jj in scalar regs too"<<endl;
    cout<<"=== //                 (ii could be re-used for iijj)"<<endl;
    if(have_vl_over_jj)
        cout<<"=== //        %vl_over_jj : scalar : vl/jj"<<endl;
    cout<<"=== //        %a, %b          : vector : outer, inner loop indices [outputs]"<<endl;
    if(have_sq)
        cout<<"=== //        %sq         : vector : 0,1,...vl"<<endl;
    if(have_bA_bD)
        cout<<"=== //        %bA, %bD    : vector : tmp vector registers"<<endl;
    cout<<"=== // no-unroll constants: vlen, ii, jj and..."<<endl;
    cout<<"=== //        iijj        : const  : outer * inner fused-loop count"<<endl;
    if(have_jj_shift){
        cout<<"=== //        jj_shift    : const  : div-by-jj right-shift"<<endl;
        cout<<"=== //        jj_minus_1  : const  : div-by-jj mask"<<endl;
    }
    if(have_jj_shift){
        cout<<"=== //        jj_M        : const  : div-by-jj multiplier"<<endl;
        cout<<"=== //        C           : const  : div-by-jj right-shift"<<endl;
    }

    //
    // are we doing any b_period or nloop precalc?
    int const b_period_max = 8; // how many regs can you spare?
    //int const b_period = unroll_suggest( vl, jj, b_period_max );
    auto const suggest = unroll_suggest( vl,ii,jj, nloop, b_period_max );

#if 0    // later ...
    // use \c suggestion to create Unroll_data
    auto const unr = mkUnroll_data( vl, ii, jj, b_period_max );
#else // for now...
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
    cout<<"=== // scalar init:"<<endl;
    cout<<"=== //   register int vl = "<<vl0<<"; // lower 10 bits used for LVL"<<endl;
    cout<<"=== //   register uint64_t iijj = "<<iijj<<"; // combined loops high limit"<<endl;
    cout<<"=== //   register uint64_t cnt = 0; // cnt 0..iijj-1 by vl"<<endl;
    cout<<"=== //   register uint64_t iloop = 0; // 0..nloops, maybe not required?"<<endl;
    if( have_jjMODvl_reset )
        cout<<"=== //   XOR jjmodvl_cycle, 0, 0         # jj_MODvl_cyc = 0\n";
    assert( !(have_vl_over_jj && have_jjMODvl) );
    //int const bigfrac_vl_jj = (have_jjMODvl? jj_over_vl: have_vl_over_jj? vl_over_jj: bogus);
    //cout<<"=== //   register uint64_t bigfrac_vl_jj = "<<bigfrac_vl_jj<<"; // immediate";
    // reformulate as iloop==0 init-code, and
    // then jump into loop (after induction formula)
    //    (more assembly-like)

    register uint64_t cnt = 0UL;


    verbose = 0; // cleans up the pseudocode output a bit
    // during loop, output pseudocode just once
    bool onceI = true; // induce-block code output at most once
    bool onceK = true; // kernel-block code output exactly most once
    bool onceL = true; // loop continueation code output exactly once
    // if no loop, some loop block are not required
    if(nloop==1){ onceI=false; onceL=false; } // no code for these blocks

    // We are writing
    //     ```for( ; cnt < iijj; cnt += vl )```
    //          a bit differently now...
    // INIT-BLOCK
    int iloop = 0;
    if(iloop==0) { // cnt==0 equiv iloop==0
        cout<<"=== // INIT_BLOCK:               # iloop=cnt=0\n";
        if(nloop==1) assert(have_vl_over_jj==0);
        // now load the initial vector-loop registers:
        // sq[i] and jj are < SAFEMAX, so we can avoid % and / operations
        if(have_sq){
            FOR(i,vl) sq[i] = i;       // vseq_v
            cout<<"=== //   VSEQ sq             # sq[i]=i\n";
        }
        if( jj==1 ){
            FOR(i,vl) a[i] = i;    // sq/jj   or perhaps change have_sq?
            FOR(i,vl) b[i] = 0;    // sq%jj
            assert(have_bA_bD==0); assert(have_sq==0); assert(have_jj_shift==0);
            cout<<"=== //   VSEQ a             # a[i] = i\n"
                <<"=== //   VBRD b, 0          # b[i] = 0\n";

        }else if(jj>=vl){
            if(verbose)cout<<" b";
            FOR(i,vl) a[i] = 0;    // sq < vl, so sq/jj < 1
            FOR(i,vl) b[i] = i;
            if(nloop<=1) {assert(have_bA_bD==0); assert(have_sq==0); assert(have_jj_shift==0); }
            cout<<"=== //   VBRD a, 0          # a[i] = 0\n"
                <<"=== //   VSEQ b             # b[i] = i\n";
        }else if( positivePow2(jj) ){
            if(verbose)cout<<" c";
            // 2 ops (shr, and)
            FOR(i,vl) a[i] = (sq[i] >> jj_shift);  // bD = bA / jj; div_vsv
            FOR(i,vl) b[i] = (sq[i] & jj_minus_1); // bM = bA % jj; mod_vsv
            if(nloop<=1) assert(have_bA_bD==0); assert(have_sq==1); assert(have_jj_shift==1);
            ++cnt_sq; ++cnt_jj_shift;
            cout<<"=== //   VSRL a, sq, jjshift    # a[i] = sq[i] >> jj_shift\n"
                <<"=== //   VAND b, sq, jj_minus_1 # b[i] = sq[i] & jj_minus_1\n";
        }else{
            if(verbose)cout<<" d";
            // 4 int ops (mul,shr, mul,sub)
            //v_divmod_vs( sq, jj, /*sq[]/jj*/a, /*sq[]%jj*/b );
            FOR(i,vl) a[i] = jj_M * sq[i] >> C;
            FOR(i,vl) b[i] = sq[i] - a[i]*jj;
            //  OK since sq[] and jj both <= SAFEMAX [(1<<21)-1]
            assert( (uint64_t)jj+vl <= (uint64_t)SAFEMAX );
            // use mul_add_shr (fastdiv) approach if jj+vl>SAFEMAX (1 more vector_add_scalar)
            if(nloop<=1) assert(have_bA_bD==0); assert(have_sq==1); assert(have_jj_shift==0);
            ++cnt_sq; ++cnt_jj_M;
            cout<<"=== //               # FOR(i,vl) a[i] = jj_M * sq[i] >> C;\n"
                <<"=== //               # FOR(i,vl) b[i] = sq[i] - a[i]*jj;\n";
            cout<<"=== //   VMPY tmp, sq, jj_M     # tmp[i] = sq[i] * jj_M\n"
                <<"=== //   VSRL a, tmp, C         # a[i] = tmp[i] >> C\n"
                <<"=== //   VMPY tmp2, a, jj       # tmp2[i] = a[i] * jj\n"
                <<"=== //   VSUB b, sq, tmp2       # b[i] = sq[i] - tmp2[i]\n";
        }
    }
    //if(onceI) cout<<"=== //   BR KERNEL_BLOCK\n"; // skip induce-block if we will have one
    goto KERNEL_BLOCK;
    // INDUCE-BLOCK
INDUCE:
    if(onceI) cout<<"=== // #INDUCE: (fall-through)\n";
    if(1){
        assert( nloop > 1 );
        // 2. Induction from a->ax, b->bx
        if(vl0%jj == 0){  // avoid div,mod -----1 vec op
            // this includes cases with b_period==1 and high nloops
            if(verbose){cout<<" e";cout.flush();}
            FOR(i,vl) a[i] = a[i] + vl_over_jj;
            ++cnt_vl_over_jj; assert(have_vl_over_jj); assert(!have_bA_bD);
            if(onceI) cout<<"=== //   VADD a, vl_over_jj, a       # add imm/Sy\n";
        }else if(jj%vl0 == 0 ){  // -------------1 or 2 vec op (conditional)
            // unroll often nice w/ have_b_period (with maybe more regs)
            assert( have_bA_bD==0); assert(have_jj_shift==0); assert(have_vl_over_jj==0);
            assert( jj > vl0 ); assert( jj/vl0 > 1 ); assert( have_jjMODvl );
            if( !have_jjMODvl_reset ){
                // Note: this case should also be a "trivial" case for unroll suggestion
                if(verbose){cout<<" f";cout.flush();}
                FOR(i,vl) b[i] = b[i] + vl0;
                ++cnt_jjMODvl; assert( have_jjMODvl );
                if(onceI) cout<<"=== //   VADD b, vl0, b          # add imm/Sy\n";
            }else{
                // This case is potentially faster with a partial precalc unroll
                // The division should be done with compute_uB
                assert( have_jjMODvl && have_jjMODvl_reset );
                //Lpi easy = iloop % jj_over_vl;       // scalar cyclic mod
                Lpi easy = fastmod_uB( iloop, jj_over_vl_M, jj_over_vl );
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
                if(onceI){
                    //
                    // Hmm. # of scalar/conditional ops is rather large.
                    //      This may not even be a speedup over divmod!
                    //
                    if(0)cout<<"=== //            # easy = iloop % jj_over_vl\n"
                        <<"=== //            # ... as a cyclic counter register\n";
                    //scalar_mod_pseudo( "easy", iloop, jj_over_vl );
                    //  Hmm. easy is really a cyclic counter, which is easier
                    //  to implement than modulus

                    cout<<"=== //   VADD tmpinc, 1, jjmodvl_cycle         # iloop%jj_over_vl\n"
                        <<"=== //   VADD tmp2, -jj_over_vl, jjmodvl_cycle # impl as cyc counter\n"
                        <<"=== //   CMOV.EQ jjmodvl_cycle, tmp2, tmpinc\n";
                    // could also CMOV two scalars and always have 2 vec ops
                    //    b += (easy? vl0 : -jj_over_vl*vl0);
                    //    a += (easy?   0 : 1);
                    cout<<"=== //     BREQ JJMODVL_RESET\n";
                    cout<<"=== //     VADD b, vl0, b              # b[i] += vl0 (easy)\n";
                    //cout<<"=== //   VOR jjmodvl_cycle, tmpinc, tmpinc
                    cout<<"=== //     BR JJMODVL_DONE\n";
                    cout<<"=== //   JJMODVL_RESET:\n";
                    cout<<"=== //     VOR  b, sq, sq              # b[i] = sq[i] (reset)\n";
                    cout<<"=== //     VADD a, 1, a                # a[i] += 1\n";
                    //cout<<"=== //   VOR jjmodvl_cycle, tmp2     # jjmodvl_cycle = 0
                    cout<<"=== //   JJMODVL_DONE:\n";
                }
            }
            if( have_jjMODvl_reset ) assert( have_jjMODvl );
            assert( !have_bA_bD );
        }else if( positivePow2(jj) ){ // ------4 vec ops (add, shr, and, add)
            if(verbose){cout<<" h";cout.flush();}
            assert( vl0%jj != 0 ); assert( jj%vl0 != 0 ); assert(have_bA_bD==1);
            assert(have_jj_shift==1); assert(have_vl_over_jj==0);
            // no...assert(have_sq==(jj>1&&jj<vl));
            FOR(i,vl) bA[i] = vl0 + b[i];            // bA = b + vl; add_vsv
            FOR(i,vl) bD[i] = (bA[i] >> jj_shift);  // bD = bA / jj; div_vsv
            FOR(i,vl) b [i] = (bA[i] & jj_minus_1); // bM = bA % jj; mod_vsv
            FOR(i,vl) a [i] = a[i] + bD[i]; // aA = a + bD; add_vvv
            ++cnt_bA_bD; ++cnt_jj_shift; assert( have_bA_bD );
            if(onceI){
                if(0)cout<<"=== //     # bA[i] = vl0 + b[i];            // bA = b + vl; add_vsv\n"
                    <<"=== //     # bD[i] = (bA[i] >> jj_shift);  // bD = bA / jj; div_vsv\n"
                    <<"=== //     # b [i] = (bA[i] & jj_minus_1); // bM = bA % jj; mod_vsv\n"
                    <<"=== //     # a [i] = a[i] + bD[i]; // aA = a + bD; add_vvv\n";
                cout<<"=== //   VADD bA, vl0, b         # bA[i] = vl0(I/R) + b[i]\n"
                    <<"=== //   VSRL bD, bA, jj_shift   # bD[i] = bA[i] >> jj_shift(I)\n"
                    <<"=== //   VAND b, bA, jj_minus_1  # b[i] = bA[i] & jj_minus_1(I)\n"
                    <<"=== //   VADD a, a, bD           # a[i] += bD[i]\n";
            }
        }else{ // div-mod ---------------------6 vec ops: add (mul,shr) (mul,sub) add
            if(verbose){cout<<" i";cout.flush();}
            FOR(i,vl) bA[i] = vl0 + b[i];            // add_vsv
            FOR(i,vl) bD[i] = ((jj_M*bA[i]) >> C);  // fastdiv_uB   : mul_vvs, shr_vs
            FOR(i,vl) b [i] = bA[i] - bD[i]*jj;     // long-hand    : mul_vvs, sub_vvv
            FOR(i,vl) a [i] = a[i] + bD[i];         // add_vvv
            ++cnt_bA_bD; ++cnt_jj_M;
            assert(have_bA_bD); assert(have_sq==(jj>1&&jj<vl)); assert(!have_jj_shift);
            assert(!have_vl_over_jj); assert( jj+vl < (int)SAFEMAX );
            if(onceI){
                if(0)cout<<"=== //      # bA[i] = vl0 + b[i];            // add_vsv\n"
                    <<"=== //      # bD[i] = ((jj_M*bA[i]) >> C);  // fastdiv_uB   : mul_vvs, shr_vs\n"
                    <<"=== //      # b [i] = bA[i] - bD[i]*jj;     // long-hand    : mul_vvs, sub_vvv\n"
                    <<"=== //      # a [i] = a[i] + bD[i];         // add_vvv\n";
                cout<<"=== //   VADD bA, vl0, b         # bA[i] = vl0(I/R) + b[i]\n"
                    <<"=== //   VMPY tmp1, bA, jj_M\n"
                    <<"=== //   VSRL bD, tmp1, C,       # bD[i] = tmp1[i] >> C(I/R)\n"
                    <<"=== //   VMPY tmp2, jj, bD\n"
                    <<"=== //   VSUB b, bA, tmp2        # b[i] -= bD[i]*jj(I/R)\n"
                    <<"=== //   VADD a, a, bD           # a[i] += bD[i]\n";
            }
        }
        if(onceI)
            cout<<"=== //   BR KERNEL_BLOCK;\n"
                <<"=== // EXIT_LOOP:\n";
        onceI = false;
    }
    // Note: vl reduction must take place AFTER above use of "long" vl
    if( cnt + vl > iijj ){ // last time might have reduced vl
        vl = iijj - cnt;
        cout<<" vl reduced for last loop to "<<vl<<endl;
    }
KERNEL_BLOCK:
    if(onceK) cout<<"=== // #KERNEL_BLOCK: (fallthrough)\n"
        <<"=== //        # <your code here, using a[] b[] loop-index vectors\n";
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
    FOR(i,vl) assert( a[i] == vabs[iloop].a[i] );
    FOR(i,vl) assert( b[i] == vabs[iloop].b[i] );
    onceK = false;

    // DONE_CHECK
    if(onceL && nloop > 1){
        cout<<"=== // DONE_CHECK:\n"
            <<"=== //   ADD cnt, vl, cnt        # or add immediate const vl0\n"
            <<"=== //   CMP cnt, iijj           # imm iijj or reg if too big\n"
            //<<"=== //   BR.LT INDUCE            #\n"
            //<<"=== //                   #END_LOOP:\n"
            <<"=== //   BR.GE EXIT_LOOP         # else fall-through to KERNEL_BLOCK\n"
            ;
        onceL = false;
    }
    ++iloop; // needed only sometimes
    cnt += vl;
    if( cnt < iijj ){
        goto INDUCE;
    }
    // LOOP_DONE ...

#undef FOR
    cout<<" Yay! induction formulas worked! iloop,nloop="<<iloop<<","<<nloop<<endl;
    assert( cnt == iijj );
    assert( nloop == iloop );
    assert( have_vl_over_jj     == (cnt_vl_over_jj    > 0) );
    assert( have_bA_bD          == (cnt_bA_bD         > 0) );
    assert( have_sq             == (cnt_sq            > 0) );
    assert( have_jj_shift       == (cnt_jj_shift      > 0) );
    assert( have_jj_M           == (cnt_jj_M          > 0) );
    assert( have_jjMODvl        == (cnt_jjMODvl       > 0) ); // old "special" count, case 'g' needed
    assert( have_jjMODvl_reset  == (cnt_jjMODvl_reset > 0) ); // old "special" count, case 'g' needed
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
                cout<<"=== svl    %vl"<<endl;
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
        cout<<"===     svl  %vl"<<endl;
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
/** make loop unrolling suggestions.
 *
 * Outputs a descriptive suggestion to \c cout.
 *
 * \return enum value describing the type of unrolling that could be done.
 *
 * \todo a separate function returning a struct describing precalc,
 * complete with precalculated data vectors
 * and maybe some precalc constants (ex. magic values, special shifts).
 */
enum Unroll unroll_suggest( int const vl, int const ii, int const jj, int const nloop, int const b_period_max ){
    enum Unroll ret = UNR_DIVMOD;

    bool const jj_pow2 = positivePow2(jj);
    int const jj_shift = positivePow2Shift((uint32_t)jj);
    // Note: I began with a simple cyclic case, jj%vl==0.
    //   In general, the period for b[] vectors is lcm(vl,jj)/vl
    //   Ex. vl=6, jj=8 --> lcm(6,8)/6=24/6 = 4 b[0] cycle={0,6,4,2}
    //   Ex. vl=6, jj=9 --> lcm(6,9)/6=18/6 = 3 b[0] cycle={0,6,3}
    //   Ex. vl=9, jj=6 --> lcm(6,9)/9=18/9 = 2 b[0] cycle={0,3}
    int const lcm_vljj = lcm(vl,jj);
    int const b_period = lcm_vljj / vl;
    char const * b_period_pow2 = (b_period > 1 && positivePow2(b_period)? " 2^k": "");
    // opt: if nloop is low, can also precalc (whether or not it is periodic)
    int const bcyc_regs = (nloop<b_period? nloop: b_period);
    bool const have_b_period = true //jj>1 /*&& jj>=b_period*/
        && bcyc_regs > 1 && bcyc_regs < b_period_max
        && !(nloop>1 && vl%jj==0)
        //&& !have_jjMODvl
        && !(nloop>1 && vl%jj!=0 && jj%vl!=0) // ???
        && !(b_period>1 && !(nloop>1 && jj%vl==0) && jj_pow2 && bcyc_regs<b_period_max)
        ;
    // print unrolling suggestion
#if 0
    if( nloop <= bcyc_regs )
        cout<<" suggest full unroll(nloop="<<nloop<<") : jj="<<jj
            <<", period="<<b_period<<", cyc_regs="<<bcyc_regs<<endl;
    else
        cout<<" suggest looped unroll(bcyc_regs="<<bcyc_regs<<") : jj="<<jj
            <<", period="<<b_period<<", nloop="<<nloop<<endl;
#else
    // We cannot nicely do a generic loop cycling over S registers, because Aurora
    // has no load instructions like Sx = S[Sw], where Sw is a cyclic register index.
    //
    // Aurora has register-indirect addressing M[V[Sw]] only for some vector ops,
    // and even there it is not loading register values, but memory values.
    //
    // if have_jjMODvl, this is an easy generic-loop case (treated here)
    // else if have_b_period and nloop > bcyc_regs, this is a "Partial Unroll"
    //     (using a set of predefined bcyc_regs vector registers)
    // else if nloop < b_period_max, this is a "full unroll" that can re-use precalc a[] & b[]
    // else if nloop < b_period, a full unroll would use too many regs to precalc a[] & b[]
    //
    // The following logic **suggests** that unrolling has 3 case:
    //    1. full precalc unroll                        (nicest case)
    //    2. partial precalc unroll (still looped)      (fairly nice)
    //    3. full unroll (no precalc)                   (always possible)
    // 1. and 2. are worth considering when:
    //    - ii,jj loops are both enclosed in outer loops
    //    - OR partial precalc reduces full unroll code size greatly.
    // Any unroll can be chosen for trivial updates:
    //    - have vl%jj==0
    //    - have jj%vl==0
    // Precalc for a jj_pow2 case may or may not be good.
    if( nloop == 1 ){
        ret = UNR_NLOOP1;
        cout<<" A.vl,ii,jj="<<vl<<","<<ii<<","<<jj<<" nloop="<<nloop
            <<", b_period="<<b_period<<b_period_pow2<<" no loop [precalc, no unroll]"<<endl;
    }else if( vl%jj == 0 ){
        ret = UNR_VLMODJJ;
        cout<<" B.vl,ii,jj="<<vl<<","<<ii<<","<<jj<<" nloop="<<nloop
            <<", b_period="<<b_period
            <<" has a trivial vl%jj==0 update [no precalc, any small unroll]"
            <<endl;
        //assert( !have_b_period );
    }else if( nloop > 1 && jj%vl == 0 ){
        //ret = UNR_JJMODVL_NORESET; // XXX FIXME fastest case
        ret = UNR_JJMODVL_RESET; // XXX FIXME
        cout<<" C.vl,ii,jj="<<vl<<","<<ii<<","<<jj<<" nloop="<<nloop
            <<", b_period="<<b_period<<b_period_pow2
            <<" has a trivial jj%vl==0 update [no precalc, any small unroll]"
            <<endl;
        //assert( !have_b_period );
        //assert("Never got case B"==nullptr);
    }else if( jj_pow2 ){
        if(nloop < b_period_max){
            ret = UNR_JJPOW2_NLOOP;
            cout<<" D.vl,ii,jj="<<vl<<","<<ii<<","<<jj<<" nloop="<<nloop
                <<", b_period="<<b_period<<b_period_pow2<<", bcyc_regs="<<bcyc_regs
                <<" has jj=2^"<<jj_shift<<" with precalc unroll(nloop="<<nloop<<")"
                <<endl;
        }else if(bcyc_regs < b_period_max){
            ret = UNR_JJPOW2_CYC;
            cout<<" E.vl,ii,jj="<<vl<<","<<ii<<","<<jj<<" nloop="<<nloop
                <<", b_period="<<b_period<<b_period_pow2<<", bcyc_regs="<<bcyc_regs
                <<" has jj=2^"<<jj_shift<<" with precalc unroll(bcyc_regs="<<bcyc_regs<<")"
                <<endl;
            //assert( have_b_period );
        }else{
            ret = UNR_JJPOW2_BIG;
            cout<<" F.vl,ii,jj="<<vl<<","<<ii<<","<<jj<<" nloop="<<nloop
                <<", b_period="<<b_period<<b_period_pow2<<", bcyc_regs="<<bcyc_regs
                <<" has jj=2^"<<jj_shift<<" easy update, but large period [no precalc, any small unroll]"
                <<endl;
        }
    }else if( nloop < b_period_max ){ // small nloop, any b_period
        ret = UNR_NLOOP;
        cout<<" G.vl,ii,jj="<<vl<<","<<ii<<","<<jj<<" nloop="<<nloop
            <<", b_period="<<b_period<<b_period_pow2
            <<" suggest full precalc unroll(nloop="<<nloop<<")\n"
            <<"     Then a[]-b[] induction is 2 ops total, mov/mov from precalc regs to working"
            <<endl;
        // no. also ok for non-cyclic and low nloop ... assert( have_b_period );
    }else if( bcyc_regs < b_period_max ){ // small b_period, high nloop
        ret = UNR_CYC;
        cout<<" H.vl,ii,jj="<<vl<<","<<ii<<","<<jj<<" nloop="<<nloop
            <<", b_period="<<b_period<<b_period_pow2
            <<" suggest partial precalc unroll(b_period="<<b_period<<"\n"
            <<"   b[] and a[]-INCREMENT cycle through precalc values\n"
            <<"     Then a[]-b[] induction is 2 ops total, mov/add from precalc regs to working"
            <<endl;
        // no...assert( have_b_period );
        //assert(" never get to H"==nullptr);
    }else{ // nloop and b_period both high OR this is a simpler case
        ret = UNR_DIVMOD;
        cout<<" I.vl,ii,jj="<<vl<<","<<ii<<","<<jj<<" nloop="<<nloop
            <<", b_period="<<b_period<<b_period_pow2<<" both high:"
            <<" full unroll(nloop="<<nloop<<") [no precalc] still possible"
            <<endl;
        assert( !have_b_period );
    }
#endif
#if 0
    // example code for precalc-unroll
    // bcyc and acyc may be precalculated registers (or possibly loaded from .rodata)
    VVlpi bcyc[bcyc_regs][vl], aDcyc[bcyc_regs][vl];
    if( have_b_period ){
        // precalculate either acyc ~      a[...][vl] and bcyc ~ b[...][vl]
        //           or        acyc ~ aDelta[...][vl] and bcyc ~ b[...][vl] (nloop <= b_period)
        assert( jj > 1 );
        assert( b_period > 1 );
        assert( !jj_pow2 );
        for(int cyc=0; cyc < bcyc_regs && cnt < iijj; cnt+=vl, ++cyc )
        {
            VVlpi a0[vl];
            int const bogus=666;
            if(cyc==0){
                if(jj>=vl) FOR(i,vl) { a[i]=0; b[i]=i; }
                else FOR(i,vl) { a[i] = jj_M*sq[i] >> C; b[i] = sq[i] - a[i]*jj;
                    ++cnt_sq; ++cnt_jj_M;
                    assert( have_sq ); assert( have_jj_M );
                }
                FOR(i,vl) acyc[0/*cyc*/][i] = a[i]; // final value, if nloop<b_period
            }else{
                assert( nloop > 1 );
                assert( !(vl%jj==0) );
                if( vl%jj==0 ) assert( !(jj%vl==0) );
                FOR(i,vl) bA[i] = vl + b[i];            // add_vsv
                FOR(i,vl) bD[i] = ((jj_M*bA[i]) >> C);  // fastdiv_uB   : mul_vvs, shr_vs
                FOR(i,vl) b [i] = bA[i] - bD[i]*jj;     // long-hand    : mul_vvs, sub_vvv
                FOR(i,vl) a [i] = a[i] + bD[i];         // add_vvv
                if( nloop < b_period ){
                    FOR(i,vl) acyc[cyc][i] = a[i];
                }else{
                    FOR(i,vl) acyc[cyc][i] = bD[i];
                    if( cyc == b_period ){ // change acyc[0] to a delta value too...
                        FOR(i,vl) acyc[0][i] = acyc[0][i] - a[i];
                    }
                }
                ++cnt_bA_bD; ++cnt_jj_M;
                assert( have_bA_bD ); assert( have_jj_M );
            }
        }
    }
#endif
    return ret;
}
/** \fn unroll_suggest
 *
 * What about vector barrel rotate (Aurora VMV instruction)...
 *
 * Idea:
 *
 *      for some jj, esp if vl+jj<256
 *      the bM,bD divmod operation might be OPTIMIZED to rot
 *      if you are willing to modify vlen or have extended-length
 *      rotation.  NB: this is rotation across vector indices.
 *      Here I consider register-only implementations.
 *
 * Conclusion:
 *
 *    - Works nicely if we have a double-vector rotate (we don't)
 *      or a rotate mod other values than MVL=256.
 *      Aurora doesn't, and the useful rotation cases
 *      already have a low op-count update.
 *
 *    - A fast long-vector rotation can be simulating by calculating a
 *      cyclic start index and doing a single \b memory load.
 *    - This depends on existence of non-trivial cases... we show one below
 *      -  i' = i+shift; if(i'>period) i'-=period   // add, conditional subtract
 *      -  b' = read vector from data[i']           // memory load
 *      -  and then modulo (mul,sub) and a' (add)
 *    - which is \em likely about the \em same speed as the divmod_uB
 *      -  cf. worst case 'i' with 6 ops, no conditionals, no mem access
 *
 *
 * Analysis: What is the complexity of simulating the rotation? 
 *
 * Ex. ```./fuse2 -t 256 1000 25; ./fuse2 -t 256,200,50;```
 * suggests
 * ```
 *  I.vl,ii,jj=256,1000,25 nloop=98, b_period=25 both high: full unroll(nloop=98) [no precalc] still possible
 *  I.vl,ii,jj=256,200,50 nloop=40, b_period=25 both high: full unroll(nloop=40) [no precalc] still possible
 * ```
 * but we could unroll by any small amount as follows:
 *
 * - store b[256+25] as (Vz[25],Vy[256]), so current b[vl] is stored in Vy register.
 * - b[] may update like [0 1 2...]     // a cyclic sequence, mod jj
 *                   --> [6 7 8...]
 *                   --> [12 13 14...]
 * - this is like a long-rotate-right by 6 vector indices, of a 256+25-long (Vz,Vy) "vector" register
 * - Aurora vector rotate is "Vector Move", VMV
 *   - Vx[i=0..vl] = Vz(mod((unsigned)(Sy+1),MVL)))
 *   - no rotate across two vector registers
 * - Could simulate long-register VMV:
 *   - rsh = 19                                 // const index shift
 *   - SyL = 6 = b_period - Sy                  // const up-shift for b'[i] = b[i-SyL]
 *   - VsqR = [ ?..? 0,1,2,...b_period-1 ]      // const vector
 *   - // 
 *   - Sy = rsh [19]                            // initial Sy
 *   - SyR = MVL - b_period + Sy                // initial shift for VsqR --> b'[0] = VsqR[SyR]
 *   - VMy = VM [ 1,1,..1 (19x) 0..0 ]
 *   - VM~y = VM [ 0..0 (19x) 1..1 ]
 *   - b[i] = fastmod(vsq[], b_period)          // initial b[i]
 *   - // Induction inputs: SyR, b[vl]
 *   - b  = [ b0 b1 ... b_vl ]                  // input; precond vl > Sy
 *   - // Induction:
 *   - Sy=Sy+SyL                                // Sy subsequent updates
 *   - VMV b', b, Sy, vl-b_period+rsh            // b' = [ ?bogus? (19x) b0 b1 ... b_{vl-Sy} ]
 *   - SyR += SyL; if( SyR > MVL ) SyR -= b_period; // SyR subsequent updates
 *   - VMV first, SyL, Sy                       // shift in correct values for ?bogus? region
 *   - VMRG b, b', first, VMy                   // next value of b[]
 * - But this is already has more instruction count than the full recalc
 *
 * - if jj is not a multiple of vl (or other way), then we do not have equally
 *   sampled values 0..jj in the b[] vector, so no single-vector Aurora VMV suffices.
 * - You can \b only do rotate method easily if \f$Vcyc = (vl/b_period)*b_period + b_period\f$
 *   can EXACTLY equal 256 in a SINGLE Aurora VMV, though.
 *   - but this happens for vl > jj only for jj already a power of two (already fast)
 *   - and implies jj is a power of two,
 *     - so we already have a fast update (vl%jj==0 or otherwise)
 *
 * Conclusion: Aurora does not allow a fast vlen ~ MVL rotation method
 *
 */


int main(int argc,char**argv){
    int vl = 8;
    int h=20, w=3;
    int opt_t=1, opt_u=0, opt_l=0, opt_h=0, opt_m=0;
    int a=0;
    if(argc > 1){
        // actually only the last -[tlu] option is used
        for( ; argv[a+1][0]=='-'; ++a){
            char *c = &argv[1][1];
            for( ; *c != '\0'; ++c){
                if(*c=='h'){
                    cout<<" fuse2lin [-h|t|l|u] VLEN H W"<<endl;
                    cout<<"  -t    just test correctness"<<endl;
                    cout<<"  -a    alt test correctness"<<endl;
                    cout<<"  -l    [default] pseudo-asm-code for loops (+correctness)"<<endl;
                    cout<<"  -u    pseudo-asm-code for unrolled loops (+correctness)"<<endl;
                    cout<<"  -m    try for extended-range (a/d) ~ a*M>>N forms"<<endl;
                    cout<<"  -h    this help"<<endl;
                    cout<<"   VLEN = vector length"<<endl;
                    cout<<"   I    = 1st loop a=0..i-1"<<endl;
                    cout<<"   J    = 2nd loop b=0..j-1"<<endl;
                    cout<<" double loop --> loop over vector registers a[VLEN], b[VLEN]"<<endl;
                    opt_h = 1;
                }else if(*c=='t'){ opt_t=2; opt_l=0; opt_u=0;
                }else if(*c=='a'){ opt_t=1; opt_l=0; opt_u=0;
                }else if(*c=='l'){ opt_t=0; opt_l=1; opt_u=0;
                }else if(*c=='u'){ opt_t=0; opt_l=0; opt_u=1;
                }else if(*c=='m'){ opt_m=1;
                }
            }
        }
    }
    cout<<" args: a = "<<a<<endl;
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
        else if(opt_t==2) test_vloop2_no_unrollX(vl,h,w);
        if(opt_u) test_vloop2_unroll(vl,h,w);
        if(opt_l) test_vloop2_no_unroll(vl,h,w); // C++ code more like asm generic loop
    }
    cout<<"\nGoodbye"<<endl;
    return 0;
}
