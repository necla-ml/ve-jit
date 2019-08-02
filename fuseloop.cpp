#include "fuseloop.hpp"
#include "vechash.hpp"    // VecHash2 (hash trace of reference loop execution for ref_vloop2)
#include "stringutil.hpp" // vecprt, lcm[from intutil.hpp]
#include <iomanip>

#ifndef MVL
#define MVL 256
#endif

namespace loop {

using namespace std;

static char const* unrollSuggestNames[] = {
    "UNR_UNSET",
    "UNR_NLOOP1",
    "UNR_VLMODJJ",
    "UNR_JJMODVL_NORESET",
    "UNR_JJMODVL_RESET",
    "UNR_JJPOW2_NLOOP",
    "UNR_JJPOW2_CYC",
    "UNR_JJPOW2_BIG",
    "UNR_NLOOP",
    "UNR_CYC",
    "UNR_DIVMOD" };
static int const nNames = sizeof(unrollSuggestNames) / sizeof(char const*);
static char const* unrollSuggestDescr[] = {
    "uninitialized",
    "no loop [precalc, never unroll]",
    "trivial vl%jj==0 update [no precalc, any small unroll]",
    "trivial jj%vl==0 update [no precalc, any small unroll]",
    "trivial jj%vl==0 update w/ reset [no precalc, any small unroll] XXX check for bcyc_regs or nloop XXX",
    "jj=2^N [precalc, full unroll by nloop]",
    "jj=2^N [precalc, [partial?] cyclic unroll]",
    "jj=2^N large period [easy updated, no precalc, any small unroll)",
    "full precalc [induce via mov+mov] full unroll",
    "precalc b[] [and a?], [partial?] cyclic unroll [induce by mov+add]",
    "no precalc, any small unroll [induce via divmod (slowest)]" };
static_assert( sizeof(unrollSuggestDescr) == sizeof(unrollSuggestNames), "mismatched array sizes");

char const* name( enum Unroll const unr ){
    assert( (int)unr >= 0 && (int)unr < nNames );
    return unrollSuggestNames[unr];
}
char const* desc( enum Unroll const unr ){
    assert( (int)unr >= 0 && (int)unr < nNames );
    return unrollSuggestDescr[unr];
}
std::ostream& operator<<(std::ostream& os, enum Unroll unr){
    return os<<name(unr)
        //<<"("<<(int)unr<<")"
        <<"{"<<desc(unr)<<"}";
}
int64_t ve_vlen_suggest_equ(int64_t const nitems){
    int const v=0; // verbose
    int64_t ret=MVL;
    //bool x0_check_vl = true;
    if( nitems <= MVL ){
        ret = nitems; // trivial: only once through the loop
    }else{
        int64_t const nFull = nitems/MVL;
        int64_t const nLoops = (nitems+MVL-1)/MVL;
        int64_t const rem   = nitems%MVL;
        if(v>0)cout<<" nFull="<<nFull<<" rem="<<rem<<" nLoops="<<nLoops<<endl;
        //if(rem+32 >= MVL){ // rem also large, latency already roughly equal.
        //    ret = MVL; } else
        if( nitems%nLoops == 0 ){
            // this is "good enough" for vector latency, but more importantly
            // avoids some special handling for last-time-through-loop.
            ret = nitems/nLoops;
            if(v>0)cout<<"loop 0.."<<nitems<<" vlen perfect division as "<<ret<<"*"<<nitems/ret<<endl;
            //x0_check_vl = false;
        }else{ // redistribute...
            // we need some remainder, set up a "nice" main vector length
            // Use rem as a last-time-through-loop value.
            // [ vector inductions REQUIRE last-loop vlen <= main vlen ]
            int64_t vleq = (nitems+nLoops-1) / nLoops;
            //
            // Note: above ensures that remainder (vector length for last pass)
            // is always less than vleq.
            // For example, N=257, MVL=256 should not use 128+129.
            // Instead, we propose 160+97
            //
            // Why? you will usually set up vector loop induction for the initial
            // vector length, which may then be inalid if vector length ever
            // increases (but stays OK if you shorten vector length).
            //
            // I.e. you can choose between
            // A. ignoring correct unused values, and
            // B. using incorrect [uncalculated?] extra values
            // during the last loop pass.  (A.) is much better.
            //
            if(v>0)cout<<" vleq="<<vleq<<" = "<<nitems<<"/"<<nLoops<<" = nitems/nLoops"<<endl;
            assert( vleq <= MVL );
            if( nitems%vleq != 0 ){
                //vleq = (vleq+31)/32*32;
                //if(v>0)cout<<"vleq rounded up to "<<vleq<<endl;

                ret = vleq;
                if(v>0)cout<<"loop 0.."<<nitems<<" vlen redistributed from "<<MVL<<"*"<<nFull<<"+"<<rem
                    <<" to "<<vleq<<"*"<<nitems/vleq<<"+"<<nitems%vleq<<endl;
                // Paranoia: if we somehow increased loop count, this logic has a bug
                assert( (nitems+vleq-1)/vleq == nLoops );
            }
        }
    }
    return ret;
}

int64_t ve_vlen_suggest(int64_t const nitems){
    int const v=0; // verbose
    int64_t ret=MVL;
    //bool x0_check_vl = true;
    if( nitems <= MVL ){
        ret = nitems; // trivial: only once through the loop
    }else{
        int64_t const nFull = nitems/MVL;
        int64_t const nLoops = (nitems+MVL-1)/MVL;
        int64_t const rem   = nitems%MVL;
        if(v>0)cout<<" nFull="<<nFull<<" rem="<<rem<<" nLoops="<<nLoops<<endl;
        if(rem+32 >= MVL){ // rem also large, latency already roughly equal.
            ret = MVL;
        }else if( nitems%nLoops == 0 ){
            // this is "good enough" for vector latency, but more importantly
            // avoids some special handling for last-time-through-loop.
            ret = nitems/nLoops;
            if(v>0)cout<<"loop 0.."<<nitems<<" vlen perfect division as "<<ret<<"*"<<nitems/ret<<endl;
            //x0_check_vl = false;
        }else{ // redistribute...
            // we need some remainder, set up a "nice" main vector length
            // DO NOT use rem as loop entry value, only as last-time-through value.
            // [ vector inductions REQUIRE last-loop vlen <= main vlen ]
            int64_t vleq = (nitems+nLoops-1) / nLoops;
            //
            // Note: above ensures that remainder (vector length for last pass)
            // is always less than vleq.
            // For example, N=257, MVL=256 should not use 128+129.
            // Instead, we propose 160+97
            //
            // Why? you will usually set up vector loop induction for the initial
            // vector length, which may then be inalid if vector length ever
            // increases (but stays OK if you shorten vector length).
            //
            // I.e. you can choose between
            // A. ignoring correct unused values, and
            // B. using incorrect [uncalculated?] extra values
            // during the last loop pass.  (A.) is much better.
            //
            if(v>0)cout<<" vleq="<<vleq<<" = "<<nitems<<"/"<<nLoops<<" = nitems/nLoops"<<endl;
            assert( vleq <= MVL );
            if( nitems%vleq == 0 ){
            }else{
                // guess latency increments for VE as vector length passes multiples of 32
                // so round "main" vector up to a multiple of 32 (take whatever remains as remainder)
                vleq = (vleq+31)/32*32;
                if(v>0)cout<<"vleq rounded up to "<<vleq<<endl;

                ret = vleq;
                if(v>0)cout<<"loop 0.."<<nitems<<" vlen redistributed from "<<MVL<<"*"<<nFull<<"+"<<rem
                    <<" to "<<vleq<<"*"<<nitems/vleq<<"+"<<nitems%vleq<<endl;
                // Paranoia: if we somehow increased loop count, this logic has a bug
                assert( (nitems+vleq-1)/vleq == nLoops );
            }
        }
    }
    return ret;
}

std::string str(UnrollSuggest const& u, std::string const& pfx /*=""*/){
    std::ostringstream oss;
    if(!pfx.empty()) oss<<" "<<pfx<<" ";
    int64_t const iijj = u.ii * u.jj;
    int vl = u.vl;              // but we might suggest a lower vector length:
    if( u.vll != 0 ){
        assert( u.vll > 0 );
        assert( u.vll <= vl );
        // vl = u.vll; NO -- this makes later assertions fail.
    }

    //uint64_t vlen_equ = ve_vlen_suggest(iijj);
    //oss<<" // ve_vlen_suggest(iijj)="<<vlen_equ<<" (no jj effect, just equitable loop load)"<<endl;
    oss<<"vl,ii,jj,maxun="<<u.vl<<","<<u.ii<<","<<u.jj<<","
        <<u.b_period_max<<" "<<u.suggested;
    if(u.vl>0 && u.ii>0 && u.jj>0 && u.suggested!=UNR_UNSET){
        if(u.unroll>0) oss<<" unroll="<<u.unroll;
        if(u.cycle>0) oss<<" cycle="<<u.cycle;
        int const nloop = (iijj+vl-1) / vl;    // div_round_up(iijj,vl)
        int const lcm_vljj = lcm(vl,u.jj);
        int const b_period = lcm_vljj / vl;
        int const bcyc_regs = (nloop<b_period? nloop: b_period);
        if(1){ // debug
            cout<<" vl,ii,jj,maxun="<<u.vl<<"["<<u.vll<<"],"<<u.ii<<","<<u.jj
                <<","<<u.b_period_max;
            cout<<" iijj="<<iijj;
            cout<<" nloop="<<u.nloop;
            cout<<" b_period="<<b_period;
            cout<<" bcyc_regs="<<bcyc_regs
                <<" unroll="<<u.unroll
                <<" cycle="<<u.cycle
                <<endl;
            cout.flush();
        }
        oss<<" nloop="<<u.nloop;
        if(nloop>1) oss<<" b_period="<<b_period;
        int const unroll_any = min(nloop,abs(u.b_period_max));
        int unroll_cyc = bcyc_regs; // or some small multiple
        if(bcyc_regs>0) unroll_cyc = unroll_any/bcyc_regs*bcyc_regs;
        // if specific unroll factors are needed, print them.
        switch(u.suggested){
          case(UNR_UNSET): break;
          case(UNR_NLOOP1): oss<<" no loop"; break;
          case(UNR_VLMODJJ): oss<<" unroll("<<unroll_any<<")"; break;
                             // jj%vl == 0
          case(UNR_JJMODVL_NORESET): oss<<" unroll("<<unroll_any<<")"; break;
          case(UNR_JJMODVL_RESET): oss<<" unroll("<<unroll_any<<")"; break;
                                   // isPositivePow2(jj)
          case(UNR_JJPOW2_NLOOP): oss<<" unroll("<<nloop<<")";
                                  assert(u.unroll==nloop);
          break;
          case(UNR_JJPOW2_CYC): oss<<" unroll("<<unroll_cyc<<")"
                                <<" bcyc_regs="<<bcyc_regs
                                <<" u.unroll="<<u.unroll;
                                assert(u.unroll%bcyc_regs==0);
          break;
          case(UNR_JJPOW2_BIG): oss<<" unroll("<<unroll_any<<")"; break;
          case(UNR_NLOOP): oss<<" unroll by "<<nloop;
                           assert(u.unroll==nloop);
          break;
          case(UNR_CYC): oss<<" unroll("<<unroll_cyc<<")";
                         assert(u.unroll%bcyc_regs==0);
          break;
                         // generic div-mod
          case(UNR_DIVMOD): oss<<" unroll("<<unroll_any<<")"; break;
        }
    }
    if(u.vll) oss<<" [Alt vll="<<u.vll<<"]";
    uint64_t vlen_equ = ve_vlen_suggest(iijj);
    uint64_t vlen_equ_min = ve_vlen_suggest_equ(iijj);
    oss<<"[Equ "<<vlen_equ<<" "<<vlen_equ_min<<"]";
    //oss<<"\n";
    return oss.str();
}
std::ostream& operator<<(std::ostream& os, UnrollSuggest const& u){
    return os<<str(u);
}
/** \c b_period_max and unroll limits are similar.
 * -ve implies NEVER unroll (maxun=0 or 1).
 * The limit can gaurd against precalculating to many registers.
 *
 * - ret.b_period_max is +/-, and related to ret.cycle and ret.unroll.
 * - ret.unroll is always calculated, but b_period_max -ve means you should ignore it.
 * - ret.cycle influences how much can be precalculated.
 */
UnrollSuggest unroll_suggest( int const vl, int const ii, int const jj, int b_period_max,
        int const v/*verbose=0*/ ){
    int64_t const iijj = ii * jj;
    int const nloop = (iijj+vl-1) / vl;    // div_round_up(iijj,vl)
    enum Unroll strategy = UNR_DIVMOD;
    UnrollSuggest ret(vl,ii,jj,b_period_max);

    int const maxun = (b_period_max>0? b_period_max: 0);
    if(b_period_max<0) b_period_max = -b_period_max;

    if(v>1)cout<<"\nUNROLL_SUGGEST\n";
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
    // Adjust default values:
    //ret.vll [0]
    //ret.nloop
    //ret.unroll [0]
    //ret.cycle [0]
    //
    // We cannot nicely do a generic loop cycling over S registers, because Aurora
    // has no load instructions like Sx = S[Sw], where Sw is a cyclic register index.
    //
    // Aurora has register-indirect addressing M[V[Sw]] only for some vector ops,
    // [scatter/gather]
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
        ret.suggested = strategy = UNR_NLOOP1;
        ret.vl = iijj;
        if(v>0)cout<<" A.vl,ii,jj="<<vl<<","<<ii<<","<<jj<<" nloop="<<nloop<<" "
            <<strategy<<"\n\t"
            <<", b_period="<<b_period<<b_period_pow2<<" no loop [precalc, no unroll]"<<endl;
    }else if( vl%jj == 0 ){
        ret.suggested = strategy = UNR_VLMODJJ;
        if(v>0)cout<<" B.vl,ii,jj="<<vl<<","<<ii<<","<<jj<<" nloop="<<nloop<<" "
            <<strategy<<"\n\t"
            <<", b_period="<<b_period
            <<" has a trivial vl%jj==0 update [no precalc, any small unroll]"
            <<endl;
        //assert( !have_b_period );
    }else if( jj%vl == 0 ){
        uint32_t const period = jj/vl;
        // fuse2.cpp shows 4 different update impls, one being trivial
        // debug -- NORESET is definitely attainable
        //bool const have_jjMODvl_reset = (vl0%jj!=0 && jj%vl0==0 && nloop >jj/vl0); // case 'g'
        //bool const have_jjMODvl       = (vl0%jj!=0 && jj%vl0==0 && nloop>=jj/vl0);
        //assert( have_jjMODvl );
        if((uint32_t)nloop > period){ // have_jjMODvl_reset
            //assert(have_jjMODvl_reset);
            ret.suggested = strategy = UNR_JJMODVL_RESET;
            // depending on period==2, other power-of-two, or anything else,
            // have 3 different simple updates.
            if(v>0)cout<<" C.vl,ii,jj="<<vl<<","<<ii<<","<<jj<<" nloop="<<nloop<<" "
                <<strategy<<"\n\t"
                    <<", b_period="<<b_period<<b_period_pow2
                    <<" has a simple jj%vl==0 update [no precalc, any small unroll]"
                    <<endl;
            if(period <= (uint32_t)b_period_max){
                ret.unroll = (uint32_t)b_period_max/period*period;
                ret.cycle  = period;
            }else{
                ret.unroll = b_period_max;
                ret.cycle = 0;
            }
        }else{
            // update is trivial FOR(i,vl) b[i] = b[i] + vl;
            ret.suggested = strategy = UNR_JJMODVL_NORESET;
            if(v>0)cout<<" c.vl,ii,jj="<<vl<<","<<ii<<","<<jj<<" nloop="<<nloop<<" "
                <<strategy<<"\n\t"
                    <<", b_period="<<b_period<<b_period_pow2
                    <<" has a trivial jj%vl==0 update [no precalc, any small unroll]"
                    <<endl;
            ret.unroll = min(b_period_max,nloop);
            ret.cycle = 0;
        }
        //assert( !have_b_period );
        //assert("Never got case B"==nullptr);
    }else if( jj_pow2 ){
        if(nloop < b_period_max){
            ret.suggested = strategy = UNR_JJPOW2_NLOOP;
            if(v>0)cout<<" D.vl,ii,jj="<<vl<<","<<ii<<","<<jj<<" nloop="<<nloop<<" "
                <<strategy<<"\n\t"
                <<", b_period="<<b_period<<b_period_pow2<<", bcyc_regs="<<bcyc_regs
                <<" has jj=2^"<<jj_shift<<" with precalc unroll(nloop="<<nloop<<")"
                <<endl;
            ret.unroll = nloop;
            ret.cycle = ret.unroll;
        }else if(bcyc_regs < b_period_max){
            ret.suggested = strategy = UNR_JJPOW2_CYC;
            if(v>0)cout<<" E.vl,ii,jj="<<vl<<","<<ii<<","<<jj<<" nloop="<<nloop<<" "
                <<strategy<<"\n\t"
                <<", b_period="<<b_period<<b_period_pow2<<", bcyc_regs="<<bcyc_regs
                <<" has jj=2^"<<jj_shift<<" with precalc unroll(bcyc_regs="<<bcyc_regs<<")"
                <<endl;
            //assert( have_b_period );
            ret.unroll = b_period_max/bcyc_regs*bcyc_regs;
            ret.cycle = bcyc_regs;
        }else{
            ret.suggested = strategy = UNR_JJPOW2_BIG;
            if(v>0)cout<<" F.vl,ii,jj="<<vl<<","<<ii<<","<<jj<<" nloop="<<nloop<<" "
                <<strategy<<"\n\t"
                <<", b_period="<<b_period<<b_period_pow2<<", bcyc_regs="<<bcyc_regs
                <<" has jj=2^"<<jj_shift<<" easy update, but large period [no precalc, any small unroll]"
                <<endl;
            ret.unroll = min(b_period_max,nloop);
        }
    }else if( nloop < b_period_max ){ // small nloop, any b_period
        ret.suggested = strategy = UNR_NLOOP;
        if(v>0)cout<<" G.vl,ii,jj="<<vl<<","<<ii<<","<<jj<<" nloop="<<nloop<<" "
            <<strategy<<"\n\t"
            <<", b_period="<<b_period<<b_period_pow2
            <<" suggest full precalc unroll(nloop="<<nloop<<")\n"
            <<"     Then a[]-b[] induction is 2 ops total, mov/mov from precalc regs to working"
            <<endl;
        ret.unroll = nloop;
        // no. also ok for non-cyclic and low nloop ... assert( have_b_period );
    }else if( bcyc_regs < b_period_max ){ // small b_period, high nloop
        ret.suggested = strategy = UNR_CYC;
        if(v>0)cout<<" H.vl,ii,jj="<<vl<<","<<ii<<","<<jj<<" nloop="<<nloop<<" "
            <<strategy<<"\n\t"
            <<", b_period="<<b_period<<b_period_pow2
            <<" suggest partial precalc unroll(b_period="<<b_period<<")\n"
            <<"   b[] and a[]-INCREMENT cycle through precalc values\n"
            <<"     Then a[]-b[] induction is 2 ops total, mov/add from precalc regs to working"
            <<endl;
        // no...assert( have_b_period );
        //assert(" never get to H"==nullptr);
        //ret.unroll = b_period; // XXX or maybe b_period * N < b_period_max ??? XXX
        ret.unroll = b_period_max/b_period*b_period;
        ret.cycle = b_period;
    }else{ // nloop and b_period both high OR this is a simpler case
        ret.suggested = strategy = UNR_DIVMOD;
        if(v>0)cout<<" I.vl,ii,jj="<<vl<<","<<ii<<","<<jj<<" nloop="<<nloop<<" "
            <<strategy<<"\n\t"
            <<", b_period="<<b_period<<b_period_pow2<<" both high:"
            <<" full unroll(nloop="<<nloop<<") [no precalc] still possible"
            <<endl;
        assert( !have_b_period );
        ret.unroll = min(b_period_max,nloop);
    }
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
    //cout<<" dbg: "<<strategy<<"  sugg "<<ret.suggested<<" unroll "<<ret.unroll<<endl;
    if(maxun>0 && ret.unroll==0){
        cout<<"TODO: set u.unroll to something nice [maxun="<<maxun<<"]"<<endl;
    }
    assert(ret.suggested != UNR_UNSET);
    return ret;
}
/** u modified to set u.vll \em and return corresponding UnrollSuggest for the alt vl. */
UnrollSuggest unroll_suggest( UnrollSuggest& u, int vl_min/*=0*/, int const v/*=0,verbosity*/ ){
    if( u.suggested != UNR_UNSET ){
        cout<<" Looking for an alt strategy..."<<endl;
    }
    double const f=(224./256.); //0.90
    int const vl = u.vl;
    uint64_t const iijj = u.ii * u.jj;
    uint64_t const nloop0 = (iijj+u.vl-1) / u.vl;    // div_round_up(iijj,vl)
    int const vl_equ_min = ve_vlen_suggest_equ(u.ii*u.jj); // "same" nloops, balanced vlen
    int const vl_max = max(1,(u.suggested==UNR_UNSET? vl: vl-1));

    // if vl_min is unspecified [out-of-range] set it via some HEURISTIC
    if( vl_min < 1 || vl_min > vl ){ // vl_min default (or out-of-range)?
        if(vl==MVL){
            // equitable work limit, tailored for VE:
            //vl_min = vl_equ_min;
            // if ii*jj is large, above range is too small, if we consider that
            // unroll Alt saving is accrued each time through loop.
            // So let's allow up to 5% more loops.
            // Later we'll use a better model to weed out duds [OpSave estimator]
            uint64_t const nloop = (105*nloop0+99)/100;
            uint64_t const vleq = (iijj+nloop-1) / nloop;
            vl_min = vleq; // with only "just a little more" loops allowed

        }else{ // original "fraction" behaviour
            //for general testing purposes (arbitrary vl)
            vl_min = max( 1, (int)(f*vl) );
            // above could be different -- Ex. vl_min implying same # nloops
        }
    }

    // u.b_period_max -ve prevents all loop unrolling, so ignore u.unroll
    int const unroll = (u.b_period_max>0? u.unroll: 0);
    //
    //  OpSave estimator
    //
    // Compare {N,I,K} nloops, induction-ops, kernel-ops
    // with {N',I',K'} uAlt suggestion.
    //
    // OpSaving = Ops' - Ops
    //          = N'*I'-N'*K - N*I+N*K
    //          = N'*I' - N*I  -  (N'-N)*K
    // Typical uAlt saving is I~4 to I'~1
    // and suppose kernel K is 10 ops...
    int const Kops=10;      // Suppose kernel is 10 ops XXX
    //... hmm Iops actually depends on precalc/unroll support too
    //    Ex. precalc outside of outer loops can have ret/nloop ~ zero. 
    auto Iops = [](enum Unroll const sugg){
        int ret = (sugg==UNR_DIVMOD? 4
            :sugg==UNR_JJPOW2_NLOOP || sugg==UNR_JJPOW2_NLOOP || sugg==UNR_JJPOW2_BIG? 2
            :sugg==UNR_JJMODVL_NORESET || sugg==UNR_JJMODVL_RESET? 1
            :sugg==UNR_VLMODJJ? 1
            :sugg==UNR_NLOOP1? 2
            :sugg==UNR_UNSET? 999999 // <-- always-worst
            :2);
        return ret;
    };

    auto OpEst = [&Kops,&Iops,&iijj,&unroll](enum Unroll const sugg, int const vl){
        int const iops = Iops(sugg);            // induction ops
        int const nl = (iijj+vl-1)/vl;          // num loops
        double ops = nl*iops + nl*Kops;         // induction + kernel ops
        bool const vl_changes = iijj%vl;        // 1 unroll might have extra
        if(vl_changes) ops += nl*(unroll<=1? 3.0: 3.0/unroll); // ~ 3 scalar ops
        // XXX when unroll>1, OpEst should also favor nPart==0 (a tiny bit)
        ops = (int)(ops*100.0)*0.01;
        return ops;
    };

    //double const opsOrig = nloop0 * Iops(u.suggested) + nloop0*Kops;
    double const opsOrig = OpEst(u.suggested,vl);
    auto OpSave = [&OpEst,&opsOrig](enum Unroll const sugg, int const vll){
        double ops = OpEst(sugg, vll);
        double const ret = opsOrig - ops; // +ve is GOOD, vll saves operations
        return ret;
    };

    if(v>0)cout<<" checking [ "<<vl_max<<" to "<<vl_min<<" ] ... "
        <<" nloops orig "<<(iijj+vl-1)/vl<<", @vl_max:"<<(iijj+vl_max-1)/vl_max
        <<", @vl_min:"<<(iijj+vl_min-1)/vl_min
        <<" OpSave kernel ops="<<Kops<<" unroll="<<unroll<<endl;

    //
    auto const sugg = u.suggested; // induction strategy
    bool const jj_pow2 = positivePow2(u.jj);
    if( sugg != UNR_NLOOP1 && sugg!=UNR_VLMODJJ && sugg!=UNR_JJMODVL_NORESET
            && sugg!=UNR_JJMODVL_RESET && !jj_pow2 ){
        // quick check for very easy cases (just print msg)
        // PURELY INFORMATIVE: always print, even for a vll < vl_min
        int const jj = u.jj;
        if( jj < vl ){
            int const vll = vl/jj*jj;
            if(v>0)cout<<"   Note: vl/jj*jj = "<<vll<<" is an exact multiple of jj"
                " (vl reduced by "<<(vl-vll)<<" or "<<int((vl-vll)*1000.0/vl)*0.1<<"%)"
                <<"\n         with nloops' = "<<(u.ii*u.jj+vll-1)/vll
                <<endl;
        }else if( jj > vl && jj%vl!=0){
            // can we make jj an exact mult of vll?
            int const nup = (jj+vl-1)/vl;
            if( jj%nup == 0 ){
                int const vll = jj/nup;
                if(v>0)cout<<"   Note: vl = "<<jj/nup<<" would make jj an exact mult of vl"
                    " (vl reduced by "<<(vl-vll)<<" or "<<int((vl-vll)*1000.0/vl)*0.1<<"%)"
                    <<"\n         with nloops' = "<<(u.ii*u.jj+vll-1)/vll
                    <<endl;
            }
        }
    }
    // "Efficient" list:
    //          UNR_NLOOP1              UNR_VLMODJJ
    //          UNR_JJMODVL_NORESET     UNR_JJMODVL_RESET
    //          UNR_JJPOW2_{NLOOP,CYC,BIG}
    //          UNR_CYC         (and maybe UNR_NLOOP)
    // leaving "inefficient" as:
    //          UNR_DIVMOD      (and maybe UNR_NLOOP)
    //
    // If u.vl is already "efficient", still try for a decent low-vl alternate
    UnrollSuggest ret = UnrollSuggest(); // If no good alt, ret is 'empty'
    u.vll = 0;                           //             and u.vll is zero
#if 0
    if( u.suggested == UNR_JJMODVL_NORESET )
        cout<<"  ---> UNR_DIVMOD_NORESET trivial, no better alt"<<endl;
    else if( u.suggested == UNR_NLOOP1 )
        cout<<"  ---> NLOOP1 has no better alt"<<endl;
    else if( u.suggested == UNR_VLMODJJ )
        cout<<"  ---> UNR_VLMODJJ trivial, no better alt"<<endl;
    else
#else
    if(1){
#endif
        if(v>0)cout<<"Checking vll ... orig alg "<<name(u.suggested)<<" nloops "<<(iijj+vl-1)/vl
            <<" orig ops~"<<opsOrig<<endl;
        int best_vll=0;
        int best_opsave=0;
        UnrollSuggest best_u;
        for( int vll = vl_max; vll >= vl_min; --vll){
            if(v>0){cout<<" "<<vll; cout.flush();}
            UnrollSuggest us = unroll_suggest(vll, u.ii, u.jj, u.b_period_max, 0/*verbose*/);
            int opsave = OpSave(us.suggested, vll);
            if(v>0)cout<<" ("<<int(vll*1000./vl)*0.1<<"%) ---> alg "<<name(us.suggested)
                <<" nloops "<<(iijj+vll-1)/vll<<" [save "<<opsave<<"] ";
            if( us.suggested==UNR_DIVMOD && u.suggested!=UNR_UNSET && u.suggested!=UNR_DIVMOD ){
                // this is the worst, so it cannot be an improvement (and might even have nloop higher)
                //cout<<" skipped: UNR_DIVMOD "<<endl;
                //continue;
                // BUT [new] it might lower vlen without nloop increase?
                if(v>0)cout<<" bad-orig";
            }
            if( u.suggested == UNR_JJMODVL_NORESET
                    || u.suggested == UNR_NLOOP1
                    || u.suggested == UNR_VLMODJJ){
                    //cout<<"  ---> trivial orig "<<name(u.suggested)<<", no better alt"<<endl;
                    //continue;
                    if(v>0)cout<<" trivial-orig";
            }
            if(1){
                // Is this alt any different?
                //if(us.suggested == u.suggested && us.unroll>=u.unroll){ // it can't be much better
                //    // Sometimes this can pick up iijj%vl==0 case! so NOT a good guess
                //    cout<<"  ---> skipped because it's too similar to original"
                //        <<" [save "<<opsave<<"]"<<endl;
                //    continue;
                //}
                if(us.suggested == u.suggested && opsave>=0 && iijj%vll==0){
                    if(v>0)cout<<" ACCEPTED! iijj%vll==0"<<endl;
                    if(opsave >= best_opsave){ best_opsave = opsave; best_vll = vll; best_u=us; }
                    continue;
                }
                if(u.suggested==UNR_JJMODVL_RESET /*other JJMODVL trivial, never here*/
                        && (us.suggested==UNR_JJPOW2_NLOOP || us.suggested==UNR_JJPOW2_BIG)){
                    if(v>0)cout<<"  ---> JJPOW2 without precalc (4 vec ops) never beats JJMODVL"
                        <<" [save "<<opsave<<"]"<<endl;
                    continue;
                }
                //
                // If unrolling for real, with precalculated constants,
                // UNR_NLOOP should always "equalize' the VLs using 'nice_vector_length(iijj)'.
                //
                // If NOT unrolling the loop, then the recalculation is quite a bit faster
                // for UNR_VLMODJJ.
                //
                // Here we adopt UNR_VLMODJJ even if it adds more loops, because the loop update is trivial
                //
                if(us.suggested == UNR_JJPOW2_BIG
                        || u.suggested== UNR_DIVMOD
                        || (us.suggested==UNR_VLMODJJ /*&& u.nloop == us.nloop*/) // trivial induction!
                        || (us.suggested==UNR_JJMODVL_NORESET /*&& u.nloop == us.nloop*/) // trivial!
                        ){
                    if(v>0)cout<<" alg "<<name(us.suggested)<<" nicer";
                    //ret = us;    // return the nice alt
                    //u.vll = vll; // also record existence-of-alt into u
                    //break;
                    if(opsave<=0) {
                        if(v>0)cout<<" but higher ops."<<endl;
                        continue;
                    }
                }

                // NEW: base acceptance on opsave (which assumes no unrolling/precalc)
                if(opsave>=0){
                    if(v>0)cout<<" ACCEPTED! (might save ops)"<<endl;
                    //ret = us;    // return the nice alt
                    //u.vll = vll; // also record existence-of-alt into u
                    //break;
                    if(opsave >= best_opsave){ best_opsave = opsave; best_vll = vll; best_u=us; }
                    continue;
                }
                if(v>0)cout<<endl;
            }
        }
        if(best_vll > 0){
            u.vll = best_vll;
            ret = best_u;
            if(v>0)cout<<"Best [/lowest equiv] vl "<<u.vl<<" --> "<<u.vll<<" [saved "<<best_opsave<<"]";
            //UnrollSuggest us = unroll_suggest(u.vll, u.ii, u.jj, u.b_period_max, 0/*verbose*/);
        }
    }
    // if u.vll is still unset, set it to min-VL-for-same-nloops ("equitable" vl_equ)
    if(vl==MVL && u.vll==0 && vl_equ_min<=vl_max){
        // opsave==0 picks up many of these cases via best_vll,
        // but we still might catch some 'auto-skipped' cases, I think.
        int const vl_equ = ve_vlen_suggest(u.ii*u.jj);
        if(vl_equ<=vl_max)          u.vll = vl_equ;      // VE latency-adjusted work balance
        else if(vl_equ_min<=vl_max) u.vll = vl_equ_min;  // else work balance only (not mult of 32)
        if(u.vll != 0){
            if(v>0)cout<<" vl="<<u.vll<<" ACCEPTED! (for load balance, not unroll)";
            // This is something like vl_equ round up to mult of 32
            UnrollSuggest us = unroll_suggest(u.vll, u.ii, u.jj, u.b_period_max, 0/*verbose*/);
            ret = us;      // similar alg, but slightly better load balance (~vector latencies)
            // this might actually be bad, if you want very low latency last time through!
        }
    }
    if(v>0)cout<<endl;
    return ret;
}

/** Generate reference vectors of vectorized 2-loop indices */
std::vector<Vab> ref_vloop2(Lpi const vlen, Lpi const ii, Lpi const jj,
        int const verbose/*=1*/ )
{
    std::vector<Vab> vabs; // fully unrolled set of reference pairs of a,b vector register
    VecHash2 vhash(vlen);

    VVlpi a(vlen), b(vlen);
    int v=0; // 0..vlen counter
    for(int64_t i=0; i<(int64_t)ii; ++i){
        for(int64_t j=0; j<(int64_t)jj; ++j){
            //cout<<"."; cout.flush();
            a[v] = i; b[v] = j;
            if( ++v >= vlen ){
                vabs.emplace_back( a, b, v );
                vhash.hash_combine( a.data(), b.data(), vlen );
                vabs.back().hash =  vhash.u64();
                v = 0;
            } 
        }
    }
    cout<<vabs.size()<<" full loops of "<<vlen<<" with "<<v<<" left over"<<endl;
    if( v > 0 ){ // partial final vector
        for(int i=v; i<vlen; ++i) { a[i] = b[i] = 0; }
        vabs.emplace_back( a, b, v );
        vhash.hash_combine( a.data(), b.data(), v );
        vabs.back().hash =  vhash.u64();
    }

    if(verbose>0){ // print ref result
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
            cout<<"b_"<<l<<"["<<vl<<"]="<<vecprt(n,wide,b,vl)<<" hash"<<vabs[l].hash<<endl;
        }
    }
    return vabs;
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
*   - but this happens for vl > jj only for jj already a power of two [already fast]
*   - and implies jj is a power of two,
    *     - so we already have a fast update : vl%jj==0 or otherwise
    *
    * Conclusion: Aurora does not allow a fast vlen ~ MVL rotation method
    *
    */


}//loop::
// vim: ts=4 sw=4 et cindent cino=^=l0,\:.5s,=-.5s,N-s,g.5s,b1 cinkeys=0{,0},0),\:,0#,!^F,o,O,e,0=break
