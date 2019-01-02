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
/** Reference values for correct index outputs */
struct Vab{
    Vab( VVlpi const& asrc, VVlpi const& bsrc, int vl )
        : a(asrc), b(bsrc), vl(vl) {}
    VVlpi a;
    VVlpi b;
    int vl;    // 0 < Vabs.back().vl < vlen
};

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
/** *_uB functions were verified for its SAFEMAX (took a while to run) */
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

    // pretty-printing via vecprt
    //int const verbose = 1; // verbose
    //int const n=8; // output up-to-n [ ... [up-to-n]] ints
    //int const bignum = std::max( ii, jj );
    //int const wide = 1 + (bignum<10? 1: bignum <100? 2: bignum<1000? 3: 4);

    cout<<"Verify-------"<<endl;
    // generate reference index outputs
    std::vector<Vab> vabs = ref_vloop2(vlen, ii, jj, 1/*verbose*/);
    assert( vabs.size() > 0 );
    assert(vabs.size() == (size_t)(((ii*jj) +vlen -1) / vlen));

    // Have reference vabs vectors. Now we try induction way.
    // 1. initialize: could copy vabs[0] from const data storage, or...
    //   - generate from seq + divmod.
    //   - 2-loop induction uses 3 scalar registers:
    //     - \c cnt 0.. \c iijj, and \c vl (for jit, iijj is CCC (compile-time-const))
    //     - get final \c vl from cnt, vl and iij)
#define FOR(I,VL) for(int I=0;I<VL;++I)
    int const verbose=1;
    if(verbose>=2){
        cout<<"   ii="<<ii<<"   jj="<<jj<<"   iijj="<<iijj<<endl;
        //cout<<" vcnt="<<vcount<<" vcnt'"<<vcount_next<<endl;
        cout<<" vabs.size() = "<<vabs.size()<<endl;
        //cout<<"iloop="<<iloop<<" / "<<nloop<<endl;
        cout<<"   vl="<<vl<<endl;
        cout<<" iijj="<<iijj<<endl;
    }
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
    //VVlpi a0(vl), b0(vl), x(vl), aA(vl), bM(vl);
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
    
    // Note: I began with a simple cyclic case, jj%vl==0.
    //   In general, the period for b[] vectors is lcm(vl,jj)/vl
    //   Ex. vl=6, jj=8 --> lcm(6,8)/6=24/6 = 4 b[0] cycle={0,6,4,2}
    //   Ex. vl=6, jj=9 --> lcm(6,9)/6=18/6 = 3 b[0] cycle={0,6,3}
    //   Ex. vl=9, jj=6 --> lcm(6,9)/9=18/9 = 2 b[0] cycle={0,3}
    int const lcm_vljj = lcm(vl,jj);
    int const b_period = lcm_vljj / vl;
    int const b_period_max = 8; // how many regs can you spare?
    if( jj>1 /*&& jj>=b_period*/ && b_period > 1 && b_period < b_period_max ) {
        if( jj <= b_period/*??*/ ) cout<<" suggest full unroll (jj="<<jj<<", period="<<b_period<<endl;
        else cout<<" suggest unroll by "<<b_period<<endl;
        cout<<"   b[] cycles through values"<<endl;
        cout<<"   a[]-INCREMENT also cycles"<<endl;
        cout<<"     Then a[]-b[] induction is 2 ops total,\n"
            <<"     move/add from fixed (precalculated) register to working register"<<endl;
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
    // have_FOO and cnt_FOO : FOO register usage condition and actual use count
    bool const have_vl_over_jj = nloop>1 && vl%jj==0;
    //bool const have_bA_bD = (jj!=1) && (iijj > (uint64_t)vl && jj%vl != 0);
    bool const have_bA_bD = nloop>1 && vl%jj!=0 && jj%vl!=0;
    bool const have_sq = (jj!=1 && jj<vl)                               // when iloop==0
        || (nloop>jj/vl && vl%jj!=0 && jj%vl0==0);                      // o/w
    bool const have_jj_shift = (jj!=1 && jj<vl && positivePow2(jj))     // when iloop==0
        || (nloop>1 && vl%jj!=0 && jj%vl!=0 && positivePow2(jj));       // o/w
    bool const have_jj_M = (jj>1 && jj<vl && !positivePow2(jj))         // when iloop==0
        || (nloop>1 && vl%jj!=0 && jj%vl!=0 && !positivePow2(jj));      // o/w

    int cnt_vl_over_jj=0, cnt_bA_bD=0, cnt_sq=0, cnt_jj_shift=0, cnt_jj_M=0;

    Vlpi const vl_over_jj = have_vl_over_jj? vl/jj: 0;
    register uint64_t cnt = 0UL;
    int count_special = 0U;
    for( ; cnt < iijj; cnt += vl )
    {
        //cout<<"cnt "<<cnt<<" iloop "<<iloop<<" ii "<<ii<<" jj "<<jj<<endl;
        if (iloop == 0){
            if(nloop==0) assert(have_vl_over_jj==0);
            // now load the initial vector-loop registers:
            // sq[i] and jj are < SAFEMAX, so we can avoid % and / operations
            FOR(i,vl) sq[i] = i;       // vseq_v
            if( jj==1 ){
                if(verbose)cout<<" a";
                FOR(i,vl) a[i] = i;    // sq/jj
                FOR(i,vl) b[i] = 0;    // sq%jj
                assert(have_bA_bD==0); assert(have_sq==0); assert(have_jj_shift==0);
            }else if(jj>=vl){
                if(verbose)cout<<" b";
                FOR(i,vl) a[i] = 0;    // sq < vl, so sq/jj < 1
                FOR(i,vl) b[i] = i;
                if(nloop<=1) {assert(have_bA_bD==0); assert(have_sq==0); assert(have_jj_shift==0); }
            }else if( positivePow2(jj) ){
                if(verbose)cout<<" c";
                // 2 ops (shr, and)
                FOR(i,vl) a[i] = (sq[i] >> jj_shift);  // bD = bA / jj; div_vsv
                FOR(i,vl) b[i] = (sq[i] & jj_minus_1); // bM = bA % jj; mod_vsv
                if(nloop<=1) assert(have_bA_bD==0); assert(have_sq==1); assert(have_jj_shift==1);
                ++cnt_sq; ++cnt_jj_shift;
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
            }
        }else{
            assert( nloop > 1 );
            // 2. Induction from a->ax, b->bx
            if(vl%jj == 0){  // avoid div,mod -----1 vec op
                if(verbose)cout<<" e";
                FOR(i,vl) a[i] = a[i] + vl_over_jj;
                assert(have_vl_over_jj==1);
                ++cnt_vl_over_jj;
            }else if(jj%vl == 0){  // -------------1 or 2 vec op (conditional)
                assert( have_bA_bD==0); assert(have_jj_shift==0); assert(have_vl_over_jj==0);
                Lpi special = iloop % (jj/vl);                  // vector mod --> scalar mod
                // #pragma..unroll(jj/vl) could be branchless
                // can be optimized further into 3 minimal-op cases
                if( special ) { // slightly less likely // bump b[i], bD[i]==0
                    if(verbose)cout<<" f";
                    FOR(i,vl) b[i] = b[i] + vl;
                }else{                                  // reset b[i], bD[i]==1
                    if(verbose)cout<<" g";
                    assert( have_sq==1 );
                    ++count_special; //cout<<" sq with iloop>0 ";
                    FOR(i,vl) b[i] = sq[i];
                    FOR(i,vl) a[i] = a[i] + 1;
                    ++cnt_sq;
                }
            }else if( positivePow2(jj) ){ // ------4 vec ops (add, shr, and, add)
                if(verbose)cout<<" h";
                assert(have_bA_bD==1); assert(have_jj_shift==1); assert(have_vl_over_jj==0); assert(have_sq==(jj>1&&jj<vl));
                FOR(i,vl) bA[i] = vl + b[i];            // bA = b + vl; add_vsv
                FOR(i,vl) bD[i] = (bA[i] >> jj_shift);  // bD = bA / jj; div_vsv
                FOR(i,vl) b [i] = (bA[i] & jj_minus_1); // bM = bA % jj; mod_vsv
                FOR(i,vl) a [i] = a[i] + bD[i]; // aA = a + bD; add_vvv
                ++cnt_bA_bD; ++cnt_jj_shift;
            }else{ // div-mod ---------------------6 vec ops: add (mul,shr) (mul,sub) add
                if(verbose)cout<<" i";
                assert(have_bA_bD==1); assert(have_sq==(jj>1&&jj<vl)); assert(have_jj_shift==0); assert(have_vl_over_jj==0);
                assert( jj+vl < (1<<21) );
                FOR(i,vl) bA[i] = vl + b[i];            // add_vsv
                FOR(i,vl) bD[i] = ((jj_M*bA[i]) >> C);  // fastdiv_uB   : mul_vvs, shr_vs
                FOR(i,vl) b [i] = bA[i] - bD[i]*jj;     // long-hand    : mul_vvs, sub_vvv
                FOR(i,vl) a [i] = a[i] + bD[i];         // add_vvv
                ++cnt_bA_bD; ++cnt_jj_M;
            }
            // Note: for some jj, esp if vl+jj<256
            //       the bM,bD divmod operation might be OPTIMIZED to rot at longer vlen
        }

        // Note: vl reduction must take place AFTER above use of "long" vl
        if( cnt + vl > iijj ){ // last time might have reduced vl
            vl = iijj - cnt;
            cout<<" vl reduced for last loop to "<<vl<<endl;
        }

        if(1){
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
    assert( have_vl_over_jj == (cnt_vl_over_jj>0) );
    assert( have_bA_bD == (cnt_bA_bD>0) );
    assert( have_sq == (cnt_sq>0) );
    assert( have_jj_shift == (cnt_jj_shift>0) );
    assert( have_jj_M == (cnt_jj_M>0) );
    assert( (count_special>0) == (vl%jj!=0 && jj%vl0==0 && nloop>jj/vl) );
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
                    cout<<"  -l    [default] pseudo-asm-code for loops (+correctness)"<<endl;
                    cout<<"  -u    pseudo-asm-code for unrolled loops (+correctness)"<<endl;
                    cout<<"  -m    try for extended-range (a/d) ~ a*M>>N forms"<<endl;
                    cout<<"  -h    this help"<<endl;
                    cout<<"   VLEN = vector length"<<endl;
                    cout<<"   I    = 1st loop a=0..i-1"<<endl;
                    cout<<"   J    = 2nd loop b=0..j-1"<<endl;
                    cout<<" double loop --> loop over vector registers a[VLEN], b[VLEN]"<<endl;
                    opt_h = 1;
                }else if(*c=='t'){ opt_t=1; opt_l=0; opt_u=0;
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
#if 0
Alg:  a/d ~ a* (mod_inverse(d) >> shr
 sorted summary of magic inv_mod shifts...
 divisor 3 shr 33
 divisor 5 shr 34
 divisor 9 shr 33
 divisor 11 shr 35
 divisor 17 shr 36
 divisor 33 shr 35
 divisor 43 shr 35
 divisor 67 shr 33
 divisor 129 shr 35
 divisor 137 shr 34
 divisor 201 shr 33
 divisor 229 shr 38
 divisor 241 shr 36
 divisor 257 shr 40
 divisor 281 shr 35
 divisor 433 shr 36
 divisor 457 shr 38
 divisor 473 shr 35
 divisor 603 shr 33
 divisor 641 shr 32
 divisor 683 shr 33
 divisor 685 shr 34
 divisor 843 shr 35
 divisor 953 shr 34
 divisor 1145 shr 38
 divisor 1419 shr 35
 divisor 1429 shr 42
 divisor 1469 shr 42
 divisor 1777 shr 37
 divisor 1885 shr 42
 divisor 2049 shr 33
 divisor 2285 shr 38
 divisor 2731 shr 39
 Note: for 2x divisor, mul by modinv(div/2), and use shr(+1)
   But the only thing floor/ceil/mod_inv multipliers give is
   extended validity range and NO REDUCTION IN OPS.
   So for jj+vl < 1<<21, convolution algs are just fine with the
   generic *_uB algs.
Alg: a/d ~ a * ((ceil(1<<64)/d)) >> shr
 divisor 3 shr 33
 divisor 9 shr 33
 divisor 10 shr 35
 divisor 11 shr 33
 divisor 12 shr 35
 divisor 15 shr 35  (not 17)
 divisor 20 shr 35
 divisor 28 shr 36 (not 33)
 divisor 43 shr 35
 divisor 48 shr 37 (not 67)
 divisor 124 shr 38
 divisor 129 shr 35
 divisor 130 shr 39
 divisor 136 shr 39 (not 137)
 divisor 140 shr 39
 divisor 144 shr 39
 divisor 153 shr 39
 divisor 156 shr 39
 divisor 160 shr 39
 divisor 168 shr 39
 divisor 170 shr 39
 divisor 180 shr 39
 divisor 182 shr 39
 divisor 192 shr 39
 divisor 195 shr 39 (not 201)
 divisor 204 shr 39
 divisor 208 shr 39
 divisor 210 shr 39
 divisor 221 shr 39
 divisor 224 shr 39 (not 229)
 divisor 234 shr 39
 divisor 238 shr 39
 divisor 240 shr 39
 divisor 241 shr 39
 divisor 252 shr 39
 divisor 255 shr 39 (not 257)
 divisor 260 shr 39
 divisor 272 shr 39 (not 281)
 divisor 288 shr 39
 divisor 320 shr 39
 divisor 336 shr 39
 divisor 340 shr 39
 divisor 357 shr 39
 divisor 376 shr 40
 divisor 416 shr 39 (not 433, 457, 473)
 divisor 476 shr 39
 divisor 482 shr 39
 divisor 534 shr 41
 divisor 552 shr 41
 divisor 576 shr 39 (not 603, 641)
 divisor 672 shr 39
 divisor 683 shr 41 (not 685)
 divisor 712 shr 41
 divisor 714 shr 39
 divisor 736 shr 41
 divisor 765 shr 39
 divisor 768 shr 41 (not 843, 953)
 divisor 964 shr 39
 divisor 1068 shr 41
 divisor 1104 shr 41 (not 1145)
 divisor 1348 shr 42
 divisor 1366 shr 41 (not 1419)
 divisor 1428 shr 39 (not 1429, 1469)
 divisor 1568 shr 42 (not 1777)
 divisor 1778 shr 42
 divisor 1792 shr 42 (not 1885)
 divisor 1928 shr 39
 divisor 2032 shr 42
 divisor 2049 shr 41
 divisor 2050 shr 43
 divisor 2112 shr 43
 divisor 2200 shr 43
 divisor 2255 shr 43 (not 2285)
 divisor 2325 shr 43
 divisor 2359 shr 42
 divisor 2400 shr 43
 divisor 2460 shr 43
 divisor 2480 shr 43
 divisor 2542 shr 43
 divisor 2560 shr 43
 divisor 2624 shr 43
 divisor 2640 shr 43
 divisor 2696 shr 42
 divisor 2706 shr 43
 divisor 2728 shr 43 (not 2731)
 divisor 2732 shr 41
 divisor 2816 shr 43
 divisor 2976 shr 43
 divisor 3072 shr 43
floor-based M with (A+1)*M>>SHR is even more widely correct ......
 sorted summary of magic inv_mod shifts...
 divisor 1 shr 32
 divisor 2 shr 33
 divisor 3 shr 33
 divisor 4 shr 34
 divisor 6 shr 33
 divisor 7 shr 33
 divisor 8 shr 35
 divisor 10 shr 35
 divisor 12 shr 35 (everybody missing 13,14)
 divisor 15 shr 35
 divisor 16 shr 36
 divisor 20 shr 35
 divisor 24 shr 35
 divisor 28 shr 36
 divisor 29 shr 35
 divisor 30 shr 35
 divisor 32 shr 37
 divisor 40 shr 35
 divisor 48 shr 37
 divisor 56 shr 36
 divisor 58 shr 35
 divisor 60 shr 35
 divisor 64 shr 38
 divisor 73 shr 36
 divisor 96 shr 37
 divisor 113 shr 35
 divisor 116 shr 35
 divisor 120 shr 35
 divisor 124 shr 38
 divisor 127 shr 35
 divisor 128 shr 39
 divisor 130 shr 39
 divisor 136 shr 39
 divisor 140 shr 39
 divisor 144 shr 39
 divisor 153 shr 39
 divisor 156 shr 39
 divisor 160 shr 39
 divisor 168 shr 39
 divisor 170 shr 39
 divisor 180 shr 39
 divisor 182 shr 39
 divisor 192 shr 39
 divisor 195 shr 39
 divisor 204 shr 39
 divisor 208 shr 39
 divisor 210 shr 39
 divisor 221 shr 39
 divisor 224 shr 39
 divisor 234 shr 39
 divisor 238 shr 39
 divisor 240 shr 39
 divisor 241 shr 39
 divisor 248 shr 38
 divisor 252 shr 39
 divisor 255 shr 39
 divisor 256 shr 40
 divisor 260 shr 39
 divisor 272 shr 39
 divisor 273 shr 39
 divisor 280 shr 39
 divisor 288 shr 39
 divisor 306 shr 39
 divisor 312 shr 39
 divisor 315 shr 39
 divisor 320 shr 39
 divisor 336 shr 39
 divisor 340 shr 39
 divisor 360 shr 39
 divisor 364 shr 39
 divisor 376 shr 40
 divisor 384 shr 39
 divisor 390 shr 39
 divisor 408 shr 39
 divisor 420 shr 39
 divisor 442 shr 39
 divisor 448 shr 39
 divisor 455 shr 39
 divisor 468 shr 39
 divisor 480 shr 39
 divisor 504 shr 39
 divisor 510 shr 39
 divisor 511 shr 36
 divisor 512 shr 41
 divisor 520 shr 39
 divisor 534 shr 41
 divisor 544 shr 39
 divisor 546 shr 39
 divisor 552 shr 41
 divisor 585 shr 39
 divisor 595 shr 39
 divisor 630 shr 39
 divisor 640 shr 39
 divisor 680 shr 39
 divisor 683 shr 41
 divisor 712 shr 41
 divisor 723 shr 39
 divisor 728 shr 39
 divisor 736 shr 41
 divisor 752 shr 40
 divisor 768 shr 41
 divisor 780 shr 39
 divisor 816 shr 39
 divisor 819 shr 39
 divisor 840 shr 39
 divisor 884 shr 39
 divisor 910 shr 39
 divisor 936 shr 39
 divisor 960 shr 39
 divisor 1020 shr 39
 divisor 1024 shr 42
 divisor 1068 shr 41
 divisor 1088 shr 39
 divisor 1092 shr 39
 divisor 1104 shr 41
 divisor 1170 shr 39
 divisor 1205 shr 39
 divisor 1260 shr 39
 divisor 1348 shr 42
 divisor 1360 shr 39
 divisor 1365 shr 39
 divisor 1424 shr 41
 divisor 1472 shr 41
 divisor 1504 shr 40
 divisor 1536 shr 41
 divisor 1560 shr 39
 divisor 1568 shr 42
 divisor 1632 shr 39
 divisor 1638 shr 39
 divisor 1778 shr 42
 divisor 1792 shr 42
 divisor 1820 shr 39
 divisor 1920 shr 39
 divisor 2032 shr 42
 divisor 2040 shr 39
 divisor 2047 shr 41
 divisor 2048 shr 43
 divisor 2050 shr 43
 divisor 2112 shr 43
 divisor 2136 shr 41
 divisor 2169 shr 39
 divisor 2176 shr 39
 divisor 2184 shr 39
 divisor 2200 shr 43
 divisor 2208 shr 41
 divisor 2255 shr 43
 divisor 2325 shr 43
 divisor 2340 shr 39
 divisor 2359 shr 42
 divisor 2400 shr 43
 divisor 2460 shr 43
 divisor 2480 shr 43
 divisor 2520 shr 39
 divisor 2542 shr 43
 divisor 2560 shr 43
 divisor 2624 shr 43
 divisor 2640 shr 43
 divisor 2706 shr 43
 divisor 2720 shr 39
 divisor 2728 shr 43
 divisor 2730 shr 39
 divisor 2816 shr 43
 divisor 2848 shr 41
 divisor 2944 shr 41
 divisor 2976 shr 43
 divisor 3072 shr 43
So probably can ceil, floor, inv_mod, and various other "magics" for
good approximations for (a/d) as a*M>>R
#endif
       exit(0);
    }

    // INCORRECT verify1();
    //cout<<" verify1 OK"<<endl;

    if(opt_h == 0){
        if(opt_t) test_vloop2(vl,h,w);
        if(opt_u) test_vloop2_unroll(vl,h,w);
        if(opt_l) test_vloop2_no_unroll(vl,h,w); // C++ code more like asm generic loop
    }
    cout<<"\nGoodbye"<<endl;
    return 0;
}
