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
#include "exechash.hpp"
#include "vechash.hpp"
#include "../asmfmt.hpp"
#include "../throw.hpp" // THROW(stuff to right of cout<<), adding fn and line number
#include "../codegenasm.hpp"

#include <functional>
#include <list>
#include <vector>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <algorithm> // ve finds std::max here!
#include <type_traits>
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

typedef int Lpi; // Loop-index type
// Note other optimization might *pack* 2-loop indices differently!
// (e.g. u32 a[0]b[0] a[1]b[1] in a packed register)
// (e.g. single register with a[] concat b[] (for short double-loops)
// Here we just store a[], b[] indices in two separate registers
typedef uint64_t Vlpi; // vector-loop-index type
typedef std::vector<Vlpi> VVlpi;

typedef make_unsigned<Lpi>::type Ulpi;
typedef make_unsigned<Vlpi>::type Uvlpi;

/** scope init for AsmFmtCols */
typedef std::list<std::pair<std::string,std::string>> AsmScope;

/** Reference values for correct index outputs */
struct Vab{
    Vab( VVlpi const& asrc, VVlpi const& bsrc, int vl )
        : a(asrc), b(bsrc), vl(vl), hash(0) {
        assert( asrc.size() >= (size_t)vl );
        assert( bsrc.size() >= (size_t)vl );
        }
    VVlpi a;
    VVlpi b;
    int vl;    // 0 < Vabs.back().vl < vlen
    uint64_t hash;
#if 0
    // THIS APPROACHED IS FLAWED.  I really want hash to be independent of any
    // particular choice for the simd lengths, vl.
    /** fold \c a[vl] and \c b[vl] with a prev [or seed] \c hash. \return new hash value. */
    static uint64_t rehash(VVlpi const& a, VVlpi const& b, int const vl, uint64_t hash);
    /** fold a[] and b[] into this->hash. \return updated this->hash */
    uint64_t rehash() { return hash = rehash(a, b, vl, hash); }
#endif
};
#if 0
/** - va and vb are read-only.
 * - We assume va and vb are small values that might be repeated,
 * - so we first distribute them widely in an i-dependent manner
 *   by adding them to a pseudorandom vseq.
 * - Then we add two bit-scramblings (REHASH_A and REHASH_B) of these to produce a single vector,
 * - all of whose elements are XOR'ed together to make the final hash.
 *
 * - Programming model:
 *   - Init in some parent scope:
 *     - init hash register with seed value [0]
 *     - const vector and scalar registers
 *   - Calc:
 *     - using scratch regs, by some asm convention for any "no-call" code block
 *
 * Since we accept a pair of same-size vectors, we use different multiplicative randomizers
 * for each, and add before xoring.
 *
 * \b Warning: this hash depends on the choices you make for \c vl.  It is \b not a hash
 * purely representing the data of sequences \c a[] and \c b[].
 *
 * This is no problem if the purpose is to compare to a reference calculation that is supposed
 * to have exactly the same \c vl choices.  But in general it is more useful to NOT depend on
 * the choices made for \c vl.
 */
inline uint64_t Vab::rehash(VVlpi const& va, VVlpi const& vb, int const vl, uint64_t hash){
    // Init:
    //   randomization constants
    #define REHASH_A 7664345821815920749ULL
    #define REHASH_B 1181783497276652981ULL
    #define REHASH_C 3202034522624059733ULL
    VVlpi vs(vl); // actually could re-use another mem area for all vl<256
    {
        uint64_t const r1 = REHASH_A;
        FOR(i,vl) vs[i] = r1 * i;                   // vs = r1 * i (const data)
    }
    uint64_t const r2 = REHASH_B;
    uint64_t const r3 = REHASH_C;

    // Calc:
    //   scratch vectors
    VVlpi vx(vl), vy(vl);
    //   dynamic values...
    FOR(i,vl) vx[i] = r2*(va[i] + vs[i]);       // vx = r2 * (va + vs)
    FOR(i,vl) vy[i] = r3*(vb[i] + vs[i]);       // vy = r3 * (vb + vs)
    FOR(i,vl) vx[i] = vx[i] + vy[i];            // vx = vx + vy
    FOR(i,vl) hash ^= vx[i];                    // hash = vx[0]^vx[1]^...^vx[vl-1]
    return hash;
}
//#define REHASH_A 1181783497276652981ULL
//#define REHASH_B 7664345821815920749ULL
//#define REHASH_C 3202034522624059733ULL
/** calculate scalar \c hash from vectors \c va and \c vb, tmp vector regs \c vx and \c vy and sequence register \c vs.
 * \pre \c hash is set (0 or a previous return value), and vl is set as desired.
 * \post \c a \c b unmodified, \c hash is calculated, and \c vx, \c vy and \c vs are trashed.
 * \return \c hash scalar register.
 *
 * - Faster would:
 *   - allocate in parent scope: hash and scratch regs vs and r0,r1,r2
 *   - init    %hash to zero
 *   - preload const %vs
 *   - preload const 742938285, 1181783497276652981ULL, 7664345821815920749ULL into scalar regs
 */
std::string rehash_Vab_asm(std::string va, std::string vb, std::string hash ){
    AsmFmtCols a;
    a.lcom("rehash_Vab_asm BEGINS",
            "  In: "+va+", "+vb,
            "  InOut: "+hash+" (scalar reg)",
            "  Const: vs[,r2,r3]",
            "  Scratch: vx,vy,r"
            );
    AsmScope const block = {{"r","%s41"},{"vx","%v63"},{"vy","%v62"},{"vs","%v61"}};
    a.scope(block,"rehash_Vab_asm");
    a.ins(ve_load64_opt0("r", REHASH_A));
    a.ins("vseq     vs");
    a.ins("vmulu.l  vs, "+hash+", vs",                  "vs = REHASH_A * i");
    a.ins(ve_load64_opt0("r", REHASH_B));   // ins(string) will strip comments: can use 'raw'
    a.ins("vaddu.l   vx, "+va+", vs");
    a.ins("vmulu.l   vx, r, vx",                        "vx = REHASH_B * (va+vs)");
    a.ins(ve_load64_opt0("r", REHASH_C));
    a.ins("vaddu.l   vy, "+vb+", vs");
    a.ins("vmulu.l   vy, r, vy",                        "vy = REHASH_C * (vb+vs)");
    a.com("reduce vx and vy...");
    a.ins("vaddu.l   vx, vx, vy",                       "vx = vx + vy");
    a.ins("vrxor     vy, vx",                           "vy[0] = vx[0]^vx[1]^...^vx[vl-1]");
    a.ins("lvs       r, vx(0)",                         "r <-- vy[0]");
    a.ins("xor       "+hash+", "+hash+", r",            " hash ^= r");
    a.pop_scope();
    a.lcom("rehash_Vab_asm DONE","");
    return a.str();
}
#endif

// - precalc may save ops when done outside some external enclosing loops
//   - no precalc: induction always via formula
//   - precalc and [partial?] unroll : induction by register sequence (maybe looped)
// - unroll can be:
//   - never                            // nloop = 1
//   - full                             // full precalc unroll (low nloop)
//   - suggested value (nloop,bcyc_regs,...)
//   - any small unroll (e.g. 8)        // using some [fast?] induction formula
enum Unroll {
    UNR_UNSET=0,        ///< uninitialized
    UNR_NLOOP1,         ///< precalc, never unroll
    UNR_VLMODJJ,        ///< no precalc, any small unroll
    // jj%vl == 0
    UNR_JJMODVL_NORESET, ///< no precalc, any small unroll
    UNR_JJMODVL_RESET,  ///< XXX check for bcyc_regs or nloop XXX
    // isPositivePow2(jj)
    UNR_JJPOW2_NLOOP,   ///< no precalc, unroll by nloop (full unrol)
    UNR_JJPOW2_CYC,     ///< precalc b[] (and a[] for a+=const?) [partial?] unroll by bcyc_regs
    UNR_JJPOW2_BIG,     ///< no precalc, any small unroll
    UNR_NLOOP,          ///< precalc, full unroll is small
    UNR_CYC,            ///< precalc, [partial?] unroll by cyclic for b[] (and a?)
    // generic div-mod
    UNR_DIVMOD          ///< no precalc, any small unroll
};
struct UnrollSuggest {
    /** where did we come from? \p vl ~ vector length. \p ii,jj ~ nested loop limits. */
    int vl, ii, jj, b_period_max;

    // Suggested strategy.
    /** what class of unrolling is suggested? */
    enum Unroll suggested;
    /** Vector Length Lower (may give a more efficient induction step).
     * \c unroll_suggest looks in range 100%--90% of original \c vl for
     * an alternative that might be better. */
    int vll;
    /** \c nloop is a multiple of \c unroll for partial unroll cases,
     * equal for full unroll,
     * but is untied for <em>any small unroll</em> case. */
    int nloop;
    /** explicit unrolling factor may be given, possibly < nloop. */
    int unroll;
    UnrollSuggest()
        : vl(0),ii(0),jj(0),b_period_max(0),suggested(UNR_UNSET),
        vll(0),nloop(0),unroll(0)
    {}
    UnrollSuggest(int const vl, int const ii, int const jj, int const b_period_max=8)
        : vl(vl), ii(ii), jj(jj), b_period_max(b_period_max),suggested(UNR_UNSET),
        vll(0), nloop((int)( ((int64_t)ii*jj+vl -1) / vl )), unroll(0)
    {}
};
UnrollSuggest unroll_suggest( int const vl, int const ii, int const jj, int const b_period_max,
        int const verbose = 1);
UnrollSuggest unroll_suggest( UnrollSuggest & u, int vl_min=0 );

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
    return os<<name(unr)<<"("<<(int)unr<<"){"<<desc(unr)<<"}";
}
std::string str(UnrollSuggest const& u, std::string const& pfx=""){
    std::ostringstream oss;
    if(!pfx.empty()) oss<<" "<<pfx<<" ";
    int64_t const iijj = u.ii * u.jj;
    int vl = u.vl;              // but we might suggest a lower vector length:
    if( u.vll != 0 ){
        assert( u.vll > 0 );
        assert( u.vll <= vl );
        vl = u.vll;
    }
    oss<<"vl,ii,jj="<<u.vl<<","<<u.ii<<","<<u.jj<<" "<<u.suggested;
    if(u.vl>0 && u.ii>0 && u.jj>0 && u.suggested!=UNR_UNSET && u.b_period_max>0){
        int const nloop = (iijj+vl-1) / vl;    // div_round_up(iijj,vl)
        int const lcm_vljj = lcm(vl,u.jj);
        int const b_period = lcm_vljj / vl;
        int const bcyc_regs = (nloop<b_period? nloop: b_period);

        oss<<" nloop="<<u.nloop;
        oss<<" b_period="<<b_period;
        int const unroll_any = min(nloop,u.b_period_max);
        int unroll_cyc = bcyc_regs;
        // but push cyclic up towards b_period_max
        if(unroll_cyc>0) while(unroll_cyc < u.b_period_max) unroll_cyc+=bcyc_regs;
        // if specific unroll factors are needed, print them.
        switch(u.suggested){
          case(UNR_UNSET): break;
          case(UNR_NLOOP1): oss<<" no loop"; break;
          case(UNR_VLMODJJ): oss<<" unroll("<<unroll_any<<")"; break;
                             // jj%vl == 0
          case(UNR_JJMODVL_NORESET): oss<<" unroll("<<unroll_any<<")"; break;
          case(UNR_JJMODVL_RESET): oss<<" unroll("<<unroll_any<<")"; break;
                                   // isPositivePow2(jj)
          case(UNR_JJPOW2_NLOOP): oss<<" unroll("<<nloop<<")"; assert(u.unroll==nloop); break;
          case(UNR_JJPOW2_CYC): oss<<" unroll("<<unroll_cyc<<")"; assert(u.unroll==bcyc_regs); break;
          case(UNR_JJPOW2_BIG): oss<<" unroll("<<unroll_any<<")"; break;
          case(UNR_NLOOP): oss<<" unroll by "<<nloop; assert(u.unroll==nloop); break;
          case(UNR_CYC): oss<<" unroll("<<unroll_cyc<<")"; assert(u.unroll==bcyc_regs); break;
                         // generic div-mod
          case(UNR_DIVMOD): oss<<" unroll("<<unroll_any<<")"; break;
        }
    }
    oss<<"\n";
    return oss.str();
}
std::ostream& operator<<(std::ostream& os, UnrollSuggest const& u){
    return os<<str(u);
}

struct UnrollData : public UnrollSuggest {
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
#include "divide_by_constants_codegen_reference.c"

/** Generate reference vectors of vectorized 2-loop indices */
std::vector<Vab> ref_vloop2(Lpi const vlen, Lpi const ii, Lpi const jj,
        int const verbose=1)
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
            cout<<"b_"<<l<<"["<<vl<<"]="<<vecprt(n,wide,b,vl)<<" hash"<<vabs[l].hash<<endl;
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
/* 23 zeros and rest (41 = C-1) ones */
#define CMASK ((UINT64_C(1)<<C)-1)
static inline uint64_t constexpr computeM_uB(uint32_t d) {
    return CMASK / d + 1;
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
    auto u = unroll_suggest( vl,ii,jj, b_period_max );
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
                    FOR(i,vl) bD[i] = ((jj_M*bA[i]) >> C);  // fastdiv_uB   : mul_vvs, shr_vs
                    FOR(i,vl) b [i] = bA[i] - bD[i]*jj;     // long-hand    : mul_vvs, sub_vvv
                    FOR(i,vl) a [i] = a[i] + bD[i];         // add_vvv
                }else if(0){ // as VE alementary ops
                    // Note:
                    // Can ">>32" can be avoided for some jj if we do a u32*u32 multiply.
                    // (Current libdivide and fastdiv suggests not,  check dev code, though)
                    FOR(i,vl) bA[i] = vl0 + b[i];

                    FOR(i,vl) bD[i] = jj_M * bA[i]; // dep -1 simul if bD ?=? bD_prev + jj_M*vl, but would an extra op later
                    FOR(i,vl) bD[i] = bD[i] >> C;  // dep -1

                    FOR(i,vl) y[i] = bD[i]*jj;     // dep -1
                    FOR(i,vl) b[i] = bA[i] - y[i]; // dep -1

                    FOR(i,vl) a[i] = a[i] + bD[i];  // dep -3
                }else { // re-ordered
                    // Note:
                    // Can ">>32" can be avoided for some jj if we do a u32*u32 multiply.
                    // (Current libdivide and fastdiv suggests not,  check dev code, though)
                    FOR(i,vl) b [i] = vl0 + b[i];
                    FOR(i,vl) bD[i] = jj_M * b [i]; // dep -1 simul if bD ?=? bD_prev + jj_M*vl, but would an extra op later
                    FOR(i,vl) bD[i] = bD[i] >> C;  // dep -1
                    FOR(i,vl) y[i] = bD[i]*jj;     // dep -1
                    FOR(i,vl) a[i] = a[i] + bD[i];  // dep -2
                    FOR(i,vl) b[i] = b [i] - y[i]; // dep -2
                }
                ++cnt_bA_bD; ++cnt_jj_M;
                assert(have_bA_bD); assert(have_sq==(jj>1&&jj<vl)); assert(!have_jj_shift);
                assert(!have_vl_over_jj); assert( jj+vl < (int)SAFEMAX );
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

void test_vloop2_no_unrollX(Lpi const vlen, Lpi const ii, Lpi const jj){ // for r in [0,h){ for c in [0,w] {...}}
    ExecHash tr;
    CodeGenAsm cg;
    {
        VecHash2 vhash(vlen);
        int verbose=1;
        assert( vlen > 0 );
        register uint64_t iijj = (uint64_t)ii * (uint64_t)jj;
        cout<<"fuse2 -t "<<setw(4)<<vlen<<" "<<setw(4)<<ii<<" "<<setw(4)<<jj
            <<"   # for(0.."<<ii<<") for(0.."<<jj<<") --> a[VL],b[VL] for VL<="<<vlen<<endl;

        register int vl = vlen;
        //if (cnt+vl > iijj) vl = iijj - cnt;  // simplifies for cnt=0
        if ((uint64_t)vl > iijj) vl = iijj;
        int const vl0 = vl; // debug
        int const nloop = (iijj+vl-1) / vl;    // div_round_up(iijj,vl)
        assert( (nloop > 1) == ((uint64_t)vl < iijj) );
        assert( vl0 > 0 );
        assert( ii > 0 );
        assert( jj > 0 );

        int const b_period_max = 8; // how many regs can you spare?
        //int const b_period = unroll_suggest( vl, jj, b_period_max );
        auto u = unroll_suggest( vl,ii,jj, b_period_max );
        // This is pinned at [max] vl, even if it may be "inefficient".
        //auto uAlt =
        unroll_suggest(u);

        cout<<" Using "<<u.suggested<<"("<<(int)u.suggested<<") for vl,ii,jj="
            <<vl<<","<<ii<<","<<jj<<endl;

        //cout<<"Verify-------"<<endl;
        // generate reference index outputs
        std::vector<Vab> vabs = ref_vloop2(vlen, ii, jj, 0/*verbose*/);
        assert( vabs.size() > 0 );
        assert(vabs.size() == (size_t)(((ii*jj) +vlen -1) / vlen));
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
            cout<<" jj_M="<<(void*)(intptr_t)jj_M<<" shift="<<C;
        }

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

        AsmFmtCols fd,fp,fi,fk,fl,fz;
        fp.com("fuse2 presets").setParent(&fd);
        fi.setParent(&fp);
        fk.setParent(&fp);
        fl.setParent(&fp);
        fz.setParent(&fp);

        // local labels:
        string fusename;
        {
            std::ostringstream oss;
            oss<<"fuse2_"<<vl<<"_"<<ii<<"_"<<jj;
            fusename = oss.str();
        }
        // immediate constants into registers.
        AsmScope block = {
            {"BASE", "L_"+fusename+"_BASE"},
            {"INDUCE", "L_"+fusename+"_INDUCE"},
            {"KERNEL_BLOCK", "L_"+fusename+"_KERNEL_BLOCK"},
            {"vl0",  "%s0"},
            {"ii",   "%s1"},
            {"jj",   "%s2"},
            {"base", "%s7"},
        };
#define HASH_KERNEL 1
        if(HASH_KERNEL) block.push_back({"reg_hash","%s33"});

        if(nloop>1){
            block.push_back({"cnt",  "%s3"});
        }
        if(nloop>1 && (iijj%vl0) ){
            block.push_back({"vl","%s4"});
        }
#define SAVE_RESTORE_VLEN 1
        if(SAVE_RESTORE_VLEN) block.push_back({"vl_save","%s5"});

        // ii and jj are freed very early, and can sometimes be re-used...
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

        // vector regs
        block.push_back({"a",    "%v0"});
        block.push_back({"b",    "%v1"});
        if((have_jj_M && nloop>1)){
            block.push_back({"bA","%v2"});
            block.push_back({"bD","%v3"});
        }
        if(have_jj_M){
            block.push_back({"vx","%v4"});
            block.push_back({"vy","%v5"});
        }
        if( have_sq ) block.push_back({"sq","%v6"});

        fd.scope(block,"vectorized double-loop --> index vectors");


        if(nloop>1 && (iijj%vl0) ){
            fp.ins("or  vl, vl0,vl0", "vl = vl0");
        }
        if(SAVE_RESTORE_VLEN){
            fp.ins("svl vl_save","save original VL --> vl_save [opt]");
        }
        if(HASH_KERNEL){
            fp.ins("lea reg_hash, 0");
        }
        fp.ins("lvl vl0",                   "VL = vl0");
        if( have_sq ){
            fp.ins("vseq sq");
            FOR(i,vl) sq[i] = i;       // vseq_v
        }

        if(nloop > 1 /*&& iijj%vl0==0*/ ){
            //fp.ins("xor cnt, ii,ii",                "cnt  = 0");
            if(iijj < (uint64_t{1}<<32)) fp.ins("lea cnt,"+jitdec(iijj)
                    , "cnt=ii*jj= "+jitdec(ii)+"*"+jitdec(jj)+" to 1 by "+jitdec(vl0));
            else                         fp.ins("mulu cnt, ii,jj"
                    , "cnt=ii*jj= "+jitdec(ii)+"*"+jitdec(jj)+" to 1 by "+jitdec(vl0));
        }
        fp.rcom("ii and jj regs now reusable");

        if( have_jjMODvl_reset ){
            //fp.ins("lea jjovl,"+jitdec(jj/vl),      "jjovl = jj/vl = "+jitdec(jj)+"/"+jitdec(vl));
            fp.ins("xor tmod, %s0,%s0",             "tmod=0, cycling < (jj/vl="+jitdec(jj/vl)+")");
        }else{
            if(have_vl_over_jj && !have_jj_M){
                if(vl/jj > 127)
                    fp.ins("lea vlojj,"+jitdec(vl/jj),  "A vlojj = vl/jj = "+jitdec(vl)+"/"+jitdec(jj));
            }else if( have_jj_M ){
                //fp.ins("lea jj_M,"+jitdec(jj_M), "jj_M ~ fast div by jj","XXX TODO jj_M is 42-bits");
                // oops. need a 64-bit load, could be lea,shift,lea
                //     but faster might be computeM_UB which is division + increment (2 ops)
                // init_block is long enough to absorb the division latency (probably)
                //fp.ins("div jj_M,"+string(CMASK_MVALUE)+",jj",  "fastdiv magic is");
                // oops.. the MASK immediate can only for Sz divisor :(
                assert( vl/jj <= 85 );
#if 0 // full calc is 3 instr, but maybe div has long latency
                fp.ins("or jj_M,0,"+jitimm(CMASK),  "   C_ones 2*B = "+jitdec(C)+" lsb ones");
                fp.ins("div jj_M, jj_M,jj",         "   for fastdiv A/jj, ok for B-bit A,jj");
                fp.ins("add jj_M,1,jj_M",           "jj_M = C_ones / d + 1 = 0x"+jithex(jj_M));
#else
                fp.ins(cg.load("jj_M", jj_M), "jj_M = "+jithex(jj_M)+" (fastdiv by "+jitdec(jj)+")");
#endif
            }else{
                assert( !have_jj_M && !have_vl_over_jj );
            }
        }

        cout<<" ********************** VECHASH VECHASH *********************"<<endl;
        // NOTE: fd is the parent but fp is where the code "should" really appear
        //       I think.   --- how to attache scope_parent concept to AsmFmtCols?
        if(have_sq)
            VecHash2::kern_asm_begin(fp, "%v6");
        else
            VecHash2::kern_asm_begin(fp); // it'll grab %v40 or such and hold it
        // TODO: a way for subkernel to request state registers from caller.

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
            fp.lcom("INIT_BLOCK:");
            if(nloop==1) assert(have_vl_over_jj==0);
            // now load the initial vector-loop registers:
            // sq[i] and jj are < SAFEMAX, so we can avoid % and / operations
            if( jj==1 ){
                tr+="init:iloop 1 jj==1";
                FOR(i,vl) a[i] = i;    // sq/jj   or perhaps change have_sq?
                FOR(i,vl) b[i] = 0;    // sq%jj
                assert(have_bA_bD==0); assert(have_sq==0); assert(have_jj_shift==0);
                fp.ins("vseq  a",    "a[i] = i");
                fp.ins("vbrdu b,0",  "b[i] = 0");
            }else if(jj>=vl){
                tr+="init:iloop 1 jj>=vl";
                if(verbose)cout<<" b";
                FOR(i,vl) a[i] = 0;    // sq < vl, so sq/jj < 1
                FOR(i,vl) b[i] = i;
                if(nloop<=1) {assert(have_bA_bD==0); assert(have_sq==0); assert(have_jj_shift==0);}
                fp.ins("vbrdu a,0",  "a[i] = 0");
                fp.ins("vseq  b",    "b[i] = i");
            }else if( positivePow2(jj) ){
                tr+="init:iloop 1 pow2jj";
                if(verbose)cout<<" c";
                // 2 ops (shr, and)
                FOR(i,vl) a[i] = (sq[i] >> jj_shift);  // bD = bA / jj; div_vsv
                FOR(i,vl) b[i] = (sq[i] & jj_minus_1); // bM = bA % jj; mod_vsv
                if(nloop<=1) assert(have_bA_bD==0); assert(have_sq==1); assert(have_jj_shift==1);
                ++cnt_sq; ++cnt_jj_shift;
                fp.ins("vsrl.l.zx a, sq,"+jitdec(jj_shift), "a[i]=sq[i]/jj =sq[i]>>"+jitdec(jj_shift));
                fp.ins("vand.l    b, "+jitimm(jj_minus_1)+",sq", "b[i]=sq[i]%jj =sq[i] & (2^"+jitdec(jj_shift)+"-1)");
            }else{
                tr+="init:iloop 1 fastdiv";
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
                fp.ins("vmulu.l   vx, sq,jj_M",      "fast division by jj="+jitdec(jj));
                fp.ins("vsrl.l.zx a, vx,"+jitdec(C), "  a[i] = sq[i]*jj_M >> "+jitdec(C));
                fp.ins("vmulu.l   vy, a,jj",         "form sq[i]%jj");
                fp.ins("vsubu.l   b, sq,vy",         "  b[i] = sq[i] - a[i]*jj");
            }
        }
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
            fp.ins("sic base", "BASE -> register for relocatable branching [opt: move upward]");
            fp.lab("BASE",alg_descr);
            have_base_addr = true;
        }else{
            fp.lcom(alg_descr);
        }
        if(onceI && nloop > 1) {
            fp.ins("b "+reladdr("KERNEL_BLOCK"), "goto KERNEL_BLOCK");
        }
        cout<<" fp.cnt="<<cnt;
        goto KERNEL_BLOCK;
        // INDUCE-BLOCK
INDUCE:
        if(onceI) fi.lab("INDUCE");
        if( iijj % vl0 ){ // special loop condition case
            tr+="induce:iijj%vl0 cnt update";
            //cnt += vl0;     // (vl, if you you to exacty hit iijj)
            //if(onceI) fi.ins("addu.l cnt, vl0, cnt", "cnt 0..iijj-1");
            cnt -= vl0;
            cout<<" fi.cnt="<<cnt;
            if(onceI) fi.ins("subu.l cnt, cnt, vl0", "loop counter: cnt 0..iijj-1");
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
                fi.ins("mins.l vl, vl0,cnt",        "vl = min(vl0,cnt) for last iter,");
                fi.ins("lvl vl",                    "   since iijj%vl0!=0. vl0 unchanged.");
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
                    fi.ins("vadd a, "+(vl/jj>127?"vlojj":jitdec(vl/jj))+",a"
                            ,        "a[i] += (vl/jj="+jitdec(vl0/jj)+"), b[] unchanged");
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
                    if(onceI) fi.ins("add b, "+jitdec(vl0)+",b",    "b[i] += vl0, a[] const");
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
                    fi.ins("vaddu.l bA,"+jitdec(vl0)+",b"
                            , "bA[i] = b[i]+vl0 (jj="+jitdec(jj)+" power-of-two)")
                        .ins("vsrl.l.zx bD,bA,"+jitdec(jj_shift)
                                , "bD[i] = bA[i]>>jj_shift["+jitdec(jj_shift)+"]")
                        // note: VAND vy~M possible, while AND has sy~I, sz~M
                        .ins("vand b,"+jitimm(jj_minus_1)+",bA"
                                , "b[i] = bA[i]&jj_mask = bA[i]%"+jitdec(vl0))
                        .ins("vaddu.l a,a,bD"
                                , "a[i] += bD[i]");
                }
            }else{ // div-mod ---------------------6 vec ops: add (mul,shr) (mul,sub) add
                tr+="induce: a[], b[] update via fastdiv";
                if(verbose){cout<<" i";cout.flush();}
                FOR(i,vl) bA[i] = vl0 + b[i];            // add_vsv
                FOR(i,vl) bD[i] = ((jj_M*bA[i]) >> C);  // fastdiv_uB   : mul_vvs, shr_vs
                FOR(i,vl) b [i] = bA[i] - bD[i]*jj;     // long-hand    : mul_vvs, sub_vvv
                FOR(i,vl) a [i] = a[i] + bD[i];         // add_vvv
                ++cnt_bA_bD; ++cnt_jj_M;
                assert(have_bA_bD); assert(have_sq==(jj>1&&jj<vl0)); assert(!have_jj_shift);
                assert(!have_vl_over_jj); assert( jj+vl0 < (int)SAFEMAX );
                if(onceI){
                    //local regs: bA,vx,bD,vy    input/output regs: a, b
                    //AsmScope const block = {{"tmp2","%s41"}};
                    //fi.scope(block,"generic fastdiv induction");
                    if(0){//plain: output reg != input reg
                        fi.ins("vaddu bA,vl0,b",               "bA[i] = b[i] + vl0")
                            .ins("vmulu.l   vx,bA,jj_M")
                            .ins("vsrl.l.zx bD,vx,"+jitdec(C), "bD[i]= bA[i]/[jj="+jitdec(jj)+"] fastdiv")
                            .ins("vmulu.l   vy,jj,bD")
                            .ins("vaddu.l   a, a,bD",          "a[i] += bD[i]")
                            .ins("vsubu.l   b, bA,vy",         "b[i] -= bD[i]*jj");
                    }else{//re-use regs
                        fi.ins("vaddu b ,vl0,b",               "b[i] = b[i] + vl0")
                            .ins("vmulu.l   bD,b ,jj_M")
                            .ins("vsrl.l.zx bD,bD,"+jitdec(C), "bD[i]= bA[i]/[jj="+jitdec(jj)+"] fastdiv")
                            .ins("vmulu.l   vy,jj,bD")
                            .ins("vaddu.l   a, a,bD",          "a[i] += bD[i]")
                            .ins("vsubu.l   b, b ,vy",         "b[i] -= bD[i]*jj");
                    }
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
                fi.ins("mins.l vl, vl0, cnt",    "vl = min(vl,cnt)");
                fi.ins("lvl vl");
            }
        }
#endif
        if(onceI){
            fi.lcom("...KERNEL_BLOCK fall-through, a[], b[] induced");
            onceI = false;
        }

KERNEL_BLOCK:
        tr+="KERNEL_BLOCK";
        if(onceK){ //cout<<"=== // #KERNEL_BLOCK: (fallthrough)\n"
            //<<"=== //        # <your code here, using a[] b[] loop-index vectors\n";
            if(nloop > 1) fk.lab("KERNEL_BLOCK");
            fk.com("", "KERNEL BLOCK(a[],b[])", "<YOUR CODE HERE>","");
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
                VecHash2::kern_asm(fk,"a","b","vl","reg_hash"); 
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
            if(nloop==1) fl.lcom("(nloop == 1, no need to check if done)");
            else if(iijj%vl0){
                fl.lcom("DONE_CHECK A : loop again if vl is not the reduced last value");
                fl.ins("breq"+string(nloop>2?".t":".nt")+" vl,vl0,"+reladdr("INDUCE"), "iijj%vl!=0 so vl<vl0 means DONE");
            }else{
                fl.lcom("DONE_CHECK B : loop again if next cnt still < iijj");
                //fl.ins("add cnt,vl0,cnt",  "cnt += vl");
                //    .ins("brlt"+string(nloop>2?".t":".nt")+" cnt,iijj, INDUCE-.", "next a[],b[]");
                fl.ins("subu cnt,vl0,cnt",  "cnt -= vl (or vl0)");
                fl.ins("bgt"+string(nloop>2?".t":".nt")+" cnt,"+reladdr("INDUCE"), "next a[],b[]");
            }
            onceL = false;
        }

        // LOOP_DONE ...
        if(SAVE_RESTORE_VLEN){
            fz.ins("lvl vl_save","load VL <-- vl_save (restore VL on exit)");
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
        fd.write();
        fp.write();
        fi.write();
        fk.write();
        fl.write();
        fz.write();
        VecHash2::kern_asm_end(fd);
        fd.pop_scope();
        // XXX multiple scope write/destroy has issues! (missing #undefs for fd right now)
        //fd.pop_scope();     // TODO: destructors should auto-pop any AsmFmtCols scopes!!!
    }
    cout<<tr.str()<<" vlen,ii,jj= "<<vlen<<" "<<ii<<" "<<jj<<endl;
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
UnrollSuggest unroll_suggest( int const vl, int const ii, int const jj, int const b_period_max,
        int const v/*verbose=0*/ ){
    int64_t const iijj = ii * jj;
    int const nloop = (iijj+vl-1) / vl;    // div_round_up(iijj,vl)
    enum Unroll strategy = UNR_DIVMOD;
    UnrollSuggest ret(vl,ii,jj,b_period_max);

    if(v)cout<<"\nUNROLL_SUGGEST\n";
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
        strategy = UNR_NLOOP1;
        if(v)cout<<" A.vl,ii,jj="<<vl<<","<<ii<<","<<jj<<" nloop="<<nloop<<" "
            <<strategy<<"\n\t"
            <<", b_period="<<b_period<<b_period_pow2<<" no loop [precalc, no unroll]"<<endl;
        ret.suggested = strategy;
        //ret.vll = 0; // unchanged
        //ret.nloop = // unchanged
        //ret.unroll = 0; // unchanged (0 = "any" unroll is ok, so maybe b_period_max is a good choice)
    }else if( vl%jj == 0 ){
        strategy = UNR_VLMODJJ;
        if(v)cout<<" B.vl,ii,jj="<<vl<<","<<ii<<","<<jj<<" nloop="<<nloop<<" "
            <<strategy<<"\n\t"
            <<", b_period="<<b_period
            <<" has a trivial vl%jj==0 update [no precalc, any small unroll]"
            <<endl;
        //assert( !have_b_period );
        ret.suggested = strategy;
    }else if( jj%vl == 0 ){
        //unroll = UNR_JJMODVL_NORESET; // XXX FIXME fastest case
        strategy = UNR_JJMODVL_RESET; // XXX FIXME
        if(v)cout<<" C.vl,ii,jj="<<vl<<","<<ii<<","<<jj<<" nloop="<<nloop<<" "
            <<strategy<<"\n\t"
            <<", b_period="<<b_period<<b_period_pow2
            <<" has a trivial jj%vl==0 update [no precalc, any small unroll]"
            <<endl;
        //assert( !have_b_period );
        //assert("Never got case B"==nullptr);
        ret.suggested = strategy;
    }else if( jj_pow2 ){
        if(nloop < b_period_max){
            strategy = UNR_JJPOW2_NLOOP;
            if(v)cout<<" D.vl,ii,jj="<<vl<<","<<ii<<","<<jj<<" nloop="<<nloop<<" "
                <<strategy<<"\n\t"
                <<", b_period="<<b_period<<b_period_pow2<<", bcyc_regs="<<bcyc_regs
                <<" has jj=2^"<<jj_shift<<" with precalc unroll(nloop="<<nloop<<")"
                <<endl;
        }else if(bcyc_regs < b_period_max){
            strategy = UNR_JJPOW2_CYC;
            if(v)cout<<" E.vl,ii,jj="<<vl<<","<<ii<<","<<jj<<" nloop="<<nloop<<" "
                <<strategy<<"\n\t"
                <<", b_period="<<b_period<<b_period_pow2<<", bcyc_regs="<<bcyc_regs
                <<" has jj=2^"<<jj_shift<<" with precalc unroll(bcyc_regs="<<bcyc_regs<<")"
                <<endl;
            //assert( have_b_period );
            ret.suggested = strategy;
            ret.unroll = nloop;
        }else{
            strategy = UNR_JJPOW2_BIG;
            if(v)cout<<" F.vl,ii,jj="<<vl<<","<<ii<<","<<jj<<" nloop="<<nloop<<" "
                <<strategy<<"\n\t"
                <<", b_period="<<b_period<<b_period_pow2<<", bcyc_regs="<<bcyc_regs
                <<" has jj=2^"<<jj_shift<<" easy update, but large period [no precalc, any small unroll]"
                <<endl;
            ret.suggested = strategy;
        }
    }else if( nloop < b_period_max ){ // small nloop, any b_period
        strategy = UNR_NLOOP;
        if(v)cout<<" G.vl,ii,jj="<<vl<<","<<ii<<","<<jj<<" nloop="<<nloop<<" "
            <<strategy<<"\n\t"
            <<", b_period="<<b_period<<b_period_pow2
            <<" suggest full precalc unroll(nloop="<<nloop<<")\n"
            <<"     Then a[]-b[] induction is 2 ops total, mov/mov from precalc regs to working"
            <<endl;
        ret.suggested = strategy;
        ret.unroll = nloop;
        // no. also ok for non-cyclic and low nloop ... assert( have_b_period );
    }else if( bcyc_regs < b_period_max ){ // small b_period, high nloop
        strategy = UNR_CYC;
        if(v)cout<<" H.vl,ii,jj="<<vl<<","<<ii<<","<<jj<<" nloop="<<nloop<<" "
            <<strategy<<"\n\t"
            <<", b_period="<<b_period<<b_period_pow2
            <<" suggest partial precalc unroll(b_period="<<b_period<<")\n"
            <<"   b[] and a[]-INCREMENT cycle through precalc values\n"
            <<"     Then a[]-b[] induction is 2 ops total, mov/add from precalc regs to working"
            <<endl;
        // no...assert( have_b_period );
        //assert(" never get to H"==nullptr);
        ret.suggested = strategy;
        ret.unroll = b_period; // XXX or maybe b_period * N < b_period_max ??? XXX
    }else{ // nloop and b_period both high OR this is a simpler case
        strategy = UNR_DIVMOD;
        if(v)cout<<" I.vl,ii,jj="<<vl<<","<<ii<<","<<jj<<" nloop="<<nloop<<" "
            <<strategy<<"\n\t"
            <<", b_period="<<b_period<<b_period_pow2<<" both high:"
            <<" full unroll(nloop="<<nloop<<") [no precalc] still possible"
            <<endl;
        ret.suggested = strategy;
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
/** Scan vl (or vl-1) to \c vl_min for an efficient loop induction.
 * - Usage:
 *   - first call \c unroll_suggest(vl,ii,jj,b_period_max)
 *   - optionally call this function with above result for
 *     [potentially] an alternate reduced-vl strategy.
 * - If \c u.suggest == UNR_UNSET (default-constructed), then
 *   scan from \c vl downwards; otherwise scan from vl-1 downwards.
 * - If \c vl_min is not in range [1,\c u.vl], then
 *   set vl_min at max(0.90*vl,1);
 * \return unroll suggestion at an efficient, possibly reduced, vector length.
 * \post if a nicer alt is found, \c u.vll records this reduce \c u.vl
 */
UnrollSuggest unroll_suggest( UnrollSuggest& u, int vl_min/*=0*/ ){
    assert( u.suggested != UNR_UNSET );
    double const f=0.90;
    int const vl = u.vl;
    int const vl_max = max(1,vl-1);
    if( vl_min < 1 || vl_min > vl ){ // vl_min default (or out-of-range)?
        vl_min = max( 1, (int)(f*vl) );
    }
    cout<<" checking [ "<<vl_max<<" to "<<vl_min<<" ] ..."<<endl;
    //
    auto const sugg = u.suggested; // induction strategy
    bool const jj_pow2 = positivePow2(u.jj);
    if( sugg != UNR_NLOOP1 && sugg!=UNR_VLMODJJ && sugg!=UNR_JJMODVL_RESET && !jj_pow2 ){
        // quick check for very easy cases (just print msg)
        // (always print, even for a vll < vl_min)
        int const jj = u.jj;
        if( jj < vl ){
            int const vll = vl/jj*jj;
            cout<<"   Note: vl/jj*jj = "<<vll<<" is an exact multiple of jj"
                " (vl reduced by "<<(vl-vll)<<" or "<<int((vl-vll)*1000.0/vl)*0.1<<"%)"
                <<endl;
        }else if( jj > vl && jj%vl!=0){
            // can we make jj an exact mult of vll?
            int const nup = (jj+vl-1)/vl;
            if( jj%nup == 0 ){
                int const vll = jj/nup;
                cout<<"   Note: vl = "<<jj/nup<<" would make jj an exact mult of vl"
                    " (vl reduced by "<<(vl-vll)<<" or "<<int((vl-vll)*1000.0/vl)*0.1<<"%)"
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
    cout<<"Checking vll "; cout.flush();
    for( int vll = vl_max; vll >= vl_min; --vll){
        cout<<" "<<vl; cout.flush();
        UnrollSuggest us = unroll_suggest(vll, u.ii, u.jj, u.b_period_max, 0/*verbose*/);
        if( us.suggested != UNR_DIVMOD ){
            ret = us;    // return the nice alt
            cout<<"\nALTERNATE strategy at vll="<<vll<<" ("
                <<int(vll*1000./vl)*0.1<<"% of vl)\n  "<<ret<<endl;
            u.vll = vll; // also record existence-of-alt into u
            break;
        }
    }
    cout<<endl;
    return ret;
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
                    cout<<"  -t    test correctness + VE asm code (no unroll)"<<endl;
                    cout<<"  -a    alt test correctness"<<endl;
                    cout<<"  -l    pseudo-asm-code for loops (+correctness)"<<endl;
                    cout<<"  -u    [WIP] pseudo-asm-code for unrolled loops (+correctness)"<<endl;
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
        else if(opt_t==2) test_vloop2_no_unrollX(vl,h,w);
        if(opt_u) test_vloop2_unroll(vl,h,w);
        if(opt_l) test_vloop2_no_unroll(vl,h,w); // C++ code more like asm generic loop
    }
    cout<<"\nGoodbye"<<endl;
    return 0;
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

// vim: ts=4 sw=4 et cindent cino=^=l0,\:.5s,=-.5s,N-s,g.5s,b1 cinkeys=0{,0},0),\:,0#,!^F,o,O,e,0=break
