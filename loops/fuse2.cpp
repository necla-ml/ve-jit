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
        if( i < n ){ oss <<setw(wide)<< v[i]; }
        if( i == n && i < vl-n ){ oss<<" ... "; }
        if( i >= n && i >= vl-n ){ oss <<setw(wide)<< v[i]; }
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
void libdiv_show(){
}
template<typename T> // T ~ an unsigned int type
void test_mod_inverse(){
    for(T a=0U; a<1024U; ++a){
        T odd = 2U*a+1U;
        T mo  = mod_inverse(odd);
        assert( odd * mo == 1 );
        if(1){ // no... can't "just" convert divides into multiplies
            for(T t=1U; t<23U; ++t){
                assert( (t/odd) == (t - t%odd)*mo );
            }
        }
    }
    cout<<" *** test_mod_inverse() *** "<<endl;
    T const addme = 0U;
    cout<<" variant: mul, add "<<addme<<", shr"<<endl;
    std::map<T,T> magic; // div -->shr
//#define MAGIC(A,MO,ADDME,SHR) (((A*(uint64_t)MO)+ADDME)>>SHR)
//#define MAGIC(A,MO,ADDME,SHR) ((A*(uint64_t)(MO+ADDME))>>SHR)
#define MAGIC(A,MO,ADDME,SHR) ((A*(uint64_t)(MO) )>>SHR)        /* <-- desirable */
//#define MAGIC(A,MO,ADDME,SHR) (((A+1)*(uint64_t)(MO) )>>SHR)
//#define MAGIC(A,MO,ADDME,SHR) ((A*(uint64_t)(MO) + (A>>1))>>SHR) // ok for all pow 2?
//#define MAGIC(A,MO,ADDME,SHR) ((A*(uint64_t)(MO) + (A>>2))>>SHR) // ok for some new
//#define MAGIC(A,MO,ADDME,SHR) ((A*(uint64_t)(MO) + A)>>SHR)
    for(int shr=28; shr<50; ++shr){
        cout<<" mod_inv w/ shr = "<<shr<<" ..."<<endl;
        std::unordered_set<T> tried;
        std::unordered_set<T> bad;
        for(T base=1024U; base>=1024U; base+=base){
            for(T a=base-1023U,cnt=0; cnt<2048 && a!=0; ++a,++cnt){
                T const div = a;
                if(tried.find(div)!=tried.end()){
                    //cout<<" already tried div="<<div<<endl;
                    continue;
                }
                T mo;
                //mo  = mod_inverse(div);
                //  if( div%2 == 1 ) assert( div * mo == 1 );
                //mo  = mod_inverse(div/2);
                //mo = ((uint64_t(1)<<63)-1)/div +1;
                //mo = ((uint64_t(1)<<62)-1)/div +1;
                //mo = ((uint64_t(1)<<63)-1)/div +0;
                //mo = ((uint64_t(1)<<63)-1)/(div) +(div>>3);
                //mo = ((uint64_t(1)<<63)-1)/(div) +(div>>6); // 124,130,136,144,160,170
                //mo = ((uint64_t(1)<<63)-1)/(div) +(div>>7); // many in 129..255
#define U64MAX (~uint64_t(0))
                //mo = (U64MAX/div); // very good, w/ (A+1)*MO>>SHR
                //mo = ((uint64_t(1)<<62)-1)/div +0; //also decent
                //mo = ((uint64_t(1)<<63)-1)/div +0; //also decent, seen before set
                //mo = ((uint64_t(1)<<63))/div +0; //also decent, seen before set
#define POW2m1(N) ((((uint64_t(1)<<(N-1)) -1)<<1)+1)
                assert( POW2m1(64) == U64MAX );
                //mo = (U64MAX/div+1); // less good, w/ A*MO>>SHR
                mo = ((POW2m1(63)))/div +1; // ~ 3,9,10,11,12,15,20
                //mo = ((POW2m1(62)))/div +1; // ~ 5,6,10,14
                //mo = ((POW2m1(63)+POW2m1(62)))/div +0; //not great
                //mo = ((POW2m1(63)+POW2m1(61)))/div +0; // some new?
                //mo = ((POW2m1(63)+POW2m1(60)))/div +0; // some new?
                //mo = ((POW2m1(63)+POW2m1(60)+(div>>1)))/div +0; // some new?
                //mo = ((POW2m1(63)+POW2m1(61)))/div +1; // finally hit 13!
#if 0
 divisor 11 shr 35
 divisor 13 shr 34
 divisor 18 shr 36
 divisor 22 shr 35
 divisor 24 shr 36
 divisor 36 shr 36
 divisor 44 shr 35
 divisor 72 shr 36
 divisor 96 shr 38
 divisor 181 shr 37
 divisor 384 shr 40
 divisor 608 shr 41
 divisor 1152 shr 42
 divisor 1216 shr 41
 divisor 1536 shr 42
 divisor 2304 shr 42
#endif
                //mo = ((POW2m1(62)))/div +1; // 5,6,10,14
                //mo = (POW2m1(62) + POW2m1(60)+1)/div +1; // incl 9,11,12,18
                //mo = (POW2m1(62) + POW2m1(60) + POW2m1(59)+2)/div +1; // incl 6,7,10,18..21
#if 0
 sorted summary of magic inv_mod shifts...
 divisor 6 shr 34
 divisor 7 shr 34
 divisor 10 shr 35
 divisor 18 shr 36
 divisor 19 shr 35
 divisor 20 shr 35
 divisor 21 shr 34
 divisor 24 shr 36
 divisor 25 shr 35
 divisor 27 shr 36
#endif
                //mo = (POW2m1(63) + POW2m1(61) + POW2m1(60)+2)/div +1; // 3,12,14,19
                //mo = (POW2m1(62) + POW2m1(60) + POW2m1(58)+2)/div +1; incl 5,10,11,17
                //
                // so libdivide pre-shift and increment are not necessary with
                // a different search procedure, I think
                //
                tried.insert(div);
                //cout<<" trying div="<<div;
                for(T t=1U; t<8000U; ++t){
                    if( !(t/div == MAGIC(t,mo,addme,shr)) ){
                        bad.insert(div);
                        break;
                    }
                }
                //cout<<"."; cout.flush();
                if( bad.find(div) == bad.end() ){
                    for(T tt=8192U; tt>=8192U; tt+=tt){
                        for(T t=tt-1024U,cnt=0U; cnt<2048U && t!=0; ++cnt, ++t){
                            if( !(t/div == MAGIC(t,mo,addme,shr)) ){
                                bad.insert(div);
                                break;
                            }
                        }
                        if(bad.find(div)!=bad.end()) break;
                    }
                }
                //cout<<(bad.find(div)!=bad.end()? " BAD":" OK")<<endl;
                //cout.flush();

                if(0){ // no... can't "just" convert divides into multiplies
                    for(T t=1U; t<99U; ++t){
                        //cout<<" t="<<t<<" div = "<<div<<" mo = "<<mo<<endl;
                        assert( (t/div) == (t - t%div)*mo );
                        // or (t/div)*t == (t-t%div)*mo*t
                        // or (t/div)*t ==  t - t%div
                        // or t%div == t - (t/div)*t
                        // Also (t+div)%div == (t+div) - (t+div)/div*t
                        //                  == t+div - (t/div + 1)*t
                        //  or  (t+div)/div = t/div + 1 = ((t+div) - (t    )%div)*mo
                        // (t+div)*mo - t*mo = div * mo = 1
                        assert( (t/div) == (t - t%div)*mo );
                        // What is the relation between t/div and t*mo ?
                        // t/div == (t*mo - (t%div)*mo)
                        //                  ^^^^^^^ in [0,div)
                        // t*mo = t/div + (t%div)*mo
                        //                ^^^^^^^^^^ in [0,mo*div)
                        //
                        // gcd(2^N,div) = 1 == 2^N*x + div*y for some x,y (actually any?)
                        // 1 == 2^N*(t/div) + div*y*(t/div)
                        // mo == 2^N*(t/div)*mo + div*y*(t/div)
                        int const x=1; // verbose
                        if( div==1 ){
                            uint64_t u = (uint64_t)1<<32;
                            uint64_t v = (((uint64_t)t*(uint64_t)mo)); // but divide-by-one case is already trivial
                            if(x)cout<<" t,div "<<t<<","<<div<<" t\%div="<<t%div<<" t/div="<<t/div<<" "<<u<<" "<<v<<endl;
                            assert( t/div == v );
                        }
                        if( positivePow2(div) ){
                            cout<<"div="<<div<<" is a power of two"<<endl;
                            continue;
                        }
                        if( div==3 ){
                            uint64_t u = (uint64_t)1<<32;
                            uint64_t v = (((uint64_t)t*(uint64_t)mo))>>32; // boils down to mul, shr by 33 (still two ops)
                            v = v>>1;
                            if(x)cout<<" t,div "<<t<<","<<div<<" t\%div="<<t%div<<" t/div="<<t/div<<" "<<u<<" "<<v<<endl;
                            assert( t/div == v );
                        }
                        if( div==5 ){
                            uint64_t u = (uint64_t)1<<32;
                            uint64_t v = (((uint64_t)t*(uint64_t)mo))>>32;
                            v = v>>2;
                            if(x)cout<<" t,div "<<t<<","<<div<<" t\%div="<<t%div<<" t/div="<<t/div<<" "<<u<<" "<<v<<endl;
                            assert( t/div == v );
                        }
                        if( div==7 ){
                            uint64_t u = (uint64_t)1<<32;
                            // uint64_t mo2 =  multiplicative inverse with 34 bits, truncated to 33 bits ??
                            uint64_t v = (((uint64_t)(t)*(uint64_t)mo))>>32;
                            //v=((v+t+1+(t>>3)-(t>>2)-(t>>1))>>3); // fails after a while
                            v = (v+t)>>3;
                            if(x)cout<<" t,div "<<t<<","<<div<<" t\%div="<<t%div<<" t/div="<<t/div<<" "<<u<<" "<<v<<" "<<(t>>2)<<endl;
                            //assert( t/div == v );
                        }
                        if(div>7) break;
                        if( div>8 && div<16 ){
                            uint64_t u = (uint64_t)1<<32;
                            uint64_t v = (((uint64_t)t*(uint64_t)mo))>>35;
                            if(x)cout<<" t,div "<<t<<","<<div<<" t\%div="<<t%div<<" t/div="<<t/div<<" "<<u<<" "<<v<<endl;
                            assert( t/div == v );
                        }
                        //if(div>7) assert(false);
                    }
                }
                //if(div>7)break;
                //cout<<" a="<<a<<endl;
                //if(a>10) break;
            }
            if(base > 2000) break;
        }
        for(auto const t: tried){
            if(bad.find(t)==bad.end()){
                cout<<" divisor "<<t<<" OK with *mod_inv, >>"<<shr<<endl;
                magic[t] = shr;
            }
        }
    }
    cout<<" sorted summary of magic inv_mod shifts..."<<endl;
    for(auto const& m: magic){
        cout<<" divisor "<<m.first<<" shr "<<m.second<<endl;
    }
}
void verify1() {
    for(uint32_t vl = 0U; vl<1000U; ++vl){
        for(uint32_t d=1U; d<vl; d+=2U){
            uint32_t d_modinv32 = mod_inverse(d);
            for(uint64_t b=vl; b<vl+vl; ++b){
                assert( d<vl && d%2==1 );
                if(!( b/d == b * d_modinv32 )){
                    cout<<" "<<b<<"/"<<d<<"="<<b/d<<", but b*"<<d_modinv32<<" = "<<b*d_modinv32<<endl;
                }
                assert( b/d == b * d_modinv32 );    
            }
        }
    }
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
/** fastdiv computes (a / d) given precomputed M for d>1.
 * \pre a,d < (1<<21). */
static inline constexpr uint32_t fastdiv_uB(uint32_t const a, uint64_t const M) {
    return M*a >> C; // 2 ops: mul, shr
}
/** fastmod computes (a % d) given precomputed M.
 * Probably nicer to calc. uint64_t D=a/d using fastdiv_uB, and then R=a-D*d.
 * (retain "everythin" in u64 registers).
 * \pre a,d < (1<<21). */
static inline uint32_t constexpr fastmod_uB(uint32_t const a, uint64_t const M, uint32_t const d) {
    // ugly with 'lowbits' approach, nicer to just do the fastdiv followed by mul and sub :(
    return a - (M*a>>C)*d; // 3-4 ops: mul, shr, mul-sub... AND get a/d too (i.e. divmod adds 1 op to the division)
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
    cout<<"test_vloop2( vlen="<<vlen<<" loops 0.."<<ii<<" 0.."<<jj<<endl;

    // pretty-printing via vecprt
    int const n=8; // output up-to-n [ ... [up-to-n]] ints
    int const bignum = std::max( ii, jj );
    int const wide = 1 + (bignum<10? 1: bignum <100? 2: bignum<1000? 3: 4);

    // generate reference index outputs
    std::vector<Vab> vabs = ref_vloop2(vlen, ii, jj, 1/*verbose*/);
    assert( vabs.size() > 0 );
    assert(vabs.size() == (size_t)(((ii*jj) +vlen -1) / vlen));

    cout<<"Verify-------"<<endl;
    // Have reference vabs vectors. Now we try induction way.
    // 1. initialize: could copy vabs[0] from const data storage, or...
    //   - generate from seq + divmod.
    //   - 2-loop induction uses 3 scalar registers:
    //     - \c cnt 0.. \c iijj, and \c vl (for jit, iijj is CCC (compile-time-const))
    //     - get final \c vl from cnt, vl and iij)
    register uint64_t iijj = (uint64_t)ii * (uint64_t)jj;
    register int vl = vlen;
    register uint64_t cnt = 0UL;
    //if (cnt+vl > iijj) vl = iijj - cnt;  // simplifies for cnt=0
    if ((uint64_t)vl > iijj) vl = iijj;

#define FOR(I,VL) for(int I=0;I<VL;++I)
    if(0){
        cout<<"   ii="<<ii<<"   jj="<<jj<<"   iijj="<<iijj<<endl;
        //cout<<" vcnt="<<vcount<<" vcnt'"<<vcount_next<<endl;
        cout<<" vabs.size() = "<<vabs.size()<<endl;
        //cout<<"iloop="<<iloop<<" / "<<nloop<<endl;
        cout<<"   vl="<<vl<<endl;
        cout<<" cnt="<<cnt<<" iijj="<<iijj<<endl;
    }
    VVlpi a(vl), b(vl), bA(vl), bM(vl), bD(vl), aA(vl), sq(vl);
    VVlpi a0(vl), b0(vl), x(vl);
    int iloop = 0; // mostly for debug checks, now;
    Vlpi jj_minus_1 = jj - 1;  // for kase 3, positivePow2(jj)
    int jj_shift=0;            // for kase 3, positivePow2(jj)
    Ulpi jj_mod_inverse_lpi   = mod_inverse((Ulpi)jj);
    Uvlpi jj_mod_inverse_Vlpi = mod_inverse((Uvlpi)jj);
    uint64_t const jj_M = computeM_uB(jj); // for fastdiv_uB method
    //for( ; iloop < nloop; ++iloop )
    magicu_info bogus = {0,0,0,0};
    assert( sizeof(uint)*CHAR_BIT == 32 );
    auto const ld = positivePow2(jj) ? bogus: compute_unsigned_magic_info( jj, 32 );
    if( !positivePow2(jj) ){
        if( ld.pre_shift==0 ){
            // NO assert( ld.post_shift==1 ); sometimes 1 or 2
            // NO  assert( ld.post_shift==0 || ld.post_shift==1 || ld.post_shift==2 );
            if( ld.post_shift==0 ){
                // never happens?
                cout<<" jj="<<jj<<" --> no pre or post-shift, increment="<<ld.increment<<", mul="<<(void*)ld.multiplier<<" jj_modinv64="<<(void*)jj_mod_inverse_Vlpi<<endl;
            }else{
                cout<<" jj="<<jj<<" --> no pre-shift, post-shift="<<ld.post_shift<<",increment="<<ld.increment<<", mul="<<(void*)ld.multiplier<<" jj_modinv64="<<(void*)jj_mod_inverse_Vlpi<<endl;
            }
        }else{
            cout<<" OHOH ";
            cout<<" jj="<<jj<<" --> pre-shift="<<ld.pre_shift<<", post-shift="<<ld.post_shift<<",increment="<<ld.increment<<", mul="<<(void*)ld.multiplier<<" jj_modinv64="<<(void*)jj_mod_inverse_Vlpi<<endl;
        }
    }
    for( ; cnt < iijj; cnt += vl )
    {
        //cout<<"cnt "<<cnt<<" iloop "<<iloop<<" ii "<<ii<<" jj "<<jj<<endl;
        int const verbose = 1; // verbose
        int kase = // for verbose printing
            vl%jj == 0          ? 1
            : jj%vl == 0        ? 2
            : positivePow2(jj)  ? 3
            : /*jj < vl*/0      ? 4
            : jj<vl && jj%2==1  ? 5
            : 0;
        if (iloop == 0){
            // now load the initial vector-loop registers:
#if 0
            VVlpi a = vabs[0].a; // vector load from const-data section [or special]
            VVlpi b = vabs[0].b; // vector load from const-data section [or special]
            cnt += vl; // (we just "did" the first loop)
#else // init values can just use vseq + divmod induction formula
            FOR(i,vl) sq[i] = i;         // vseq_v
            FOR(i,vl) b [i] = sq[i] % jj;
            FOR(i,vl) a [i] = sq[i] / jj;
#endif
            FOR(i,vl) a0[i] = a[i];
            FOR(i,vl) b0[i] = b[i];
            if(verbose || kase!=0 ){ // for debug printing or assertions
                FOR(i,vl) bA[i] = vl + b[i];  // bA = b + vl; add_vsv
                FOR(i,vl) bM[i] = b[i] % jj;
                //FOR(i,vl) bM[i] = bA[i] % jj;
                cout<<"bA0 "<<vecprt(n,wide,bA,vl)<<endl;
                cout<<"bM0 "<<vecprt(n,wide,bM,vl)<<endl;
                FOR(i,vl) bD[i] = bA[i] / jj; // bD = bA / jj; div_vsv
            }
            if( kase==2 ){
#if 0
                Lpi special = iloop % (jj/vl);
                assert( special == 0 );
                if( special==0 ) FOR(i,vl) x[i] = sq[i];
                else          FOR(i,vl) x[i] = x[i] + vl;
                cout<<" bM "<<special<<" := "<<vecprt(n,wide,bM,vl)<<endl;
                cout<<"  x "<<special<<" := "<<vecprt(n,wide, x,vl)<<endl;
#else
                FOR(i,vl) x[i] = sq[i];                 // for debug
                FOR(i,vl) assert( bM[i] == x[i] );
#endif
                FOR(i,vl) assert( a[i] == 0 );
                FOR(i,vl) assert( b[i] == sq[i] );
            }
            if( kase==3 ){
                jj_shift = positivePow2Shift((uint32_t)jj);
                cout<<" jj="<<jj<<" power of two shift is "<<jj_shift<<"    mask is "<<jj_minus_1<<endl;
            }

            // NB: common operation is divmod(v,s,vM,vD) : v--> v%s, v/s,
            //     which has some optimizations for nice values of jj.
        }else{
            // 2. Induction from a->ax, b->bx
            if(vl%jj == 0){  // avoid div,mod ----------------------------> 1 vec op
                // jj==1 special case is included here (good)
                assert( kase == 1 );
                //FOR(i,vl) bA[i] = vl + b[i];
                //FOR(i,vl) x [i] = bA[i] % jj;
                //cout<<" bA "<<vecprt(n,wide,bA,vl)<<endl;
                //cout<<"  x "<<vecprt(n,wide, x,vl)<<endl;
                //FOR(i,vl) assert( bM[i] == x[i] ); // const
                FOR(i,vl) assert( bA[i] = vl + b[i] );          // const vector
                FOR(i,vl) assert( bM[i] == (vl+b[i]) % jj );    // const vector
                FOR(i,vl) assert( bM[i] == b[i] % jj );         // --- " ---
                FOR(i,vl) assert( bD[i] == (vl+b[i]) / jj );
                FOR(i,vl) assert( bD[i] == (Vlpi)(vl/jj) );     // const scalar
                // FOR(i,vl) aA[i] = a[i] + vl/jj; // aA = a + bD; add_vvv
                // /**/ FOR(i,vl) b[i] = bM[i]; // bNext is bM
                // /**/ FOR(i,vl) a[i] = aA[i]; // aNext is aA
                FOR(i,vl) a[i] = a[i] + vl/jj;
            }else if(jj%vl == 0){  // avoid div, scalar mod ---------------> 1 or 2 vec op
                assert( kase == 2 );
                Lpi special = iloop % (jj/vl);                  // vector mod --> scalar mod
#if 0 // debug
                FOR(i,vl) assert( bA[i] = vl + b[i] );          // const vector
#if 0
                FOR(i,vl) bM[i] = bA[i] % jj; // bM = bA % jj; mod_vsv
#elif 0
                if( special==0 ) FOR(i,vl) x[i] = sq[i];
                else             FOR(i,vl) x[i] = x[i] + vl;
                cout<<" bM "<<special<<" := "<<vecprt(n,wide,bM,vl)<<endl;
                cout<<"  x "<<special<<" := "<<vecprt(n,wide, x,vl)<<endl;
                FOR(i,vl) assert( bM[i] == x[i] );
#elif 1 // not sure whether this is fastest ......................
                if( special==0 ) FOR(i,vl) bM[i] = sq[i];
                else             FOR(i,vl) bM[i] = bM[i] + vl;
#else
                FOR(i,vl) bM[i] = b0[i] + vl*special; // bM = bA % jj; mod_vsv
#endif
                FOR(i,vl) assert( (vl+b[i])/jj == (special==0?1:0) ); // test bD[i] (all-zero or all-one)
                //FOR(i,vl) aA[i] = a[i] + (special==0);
                if( special == 0 ) a[i] = a[i] + 1;       // is this faster?

                /**/ FOR(i,vl) b[i] = bM[i]; // bNext is bM
                /**/ FOR(i,vl) a[i] = aA[i]; // aNext is aA
#else // streamlined
                // unroll(jj/vl) (if not too big) could be branchless
                FOR(i,vl) assert( bA[i] = vl + b[i] );          // const vector
                FOR(i,vl) assert( (vl+b[i])/jj == (special==0?1:0) ); // test bD[i]
                // can be optimized further into 3 minimal-op cases
                if( special ) { // slightly less likely // bump b[i], bD[i]==0
                    FOR(i,vl) b[i] = b[i] + vl;
                }else{                                  // reset b[i], bD[i]==1
                    FOR(i,vl) b[i] = sq[i];
                    FOR(i,vl) a[i] = a[i] + 1;
                }
#endif
            }else if( positivePow2(jj) ){ // -----------------> 4 vec ops (add, shr, and, add)
                assert( kase == 3 );
#if 0 // debug
                FOR(i,vl) bA[i] = vl + b[i];  // bA = b + vl; add_vsv
                FOR(i,vl) bD[i] = bA[i] / jj; // bD = bA / jj; div_vsv
                FOR(i,vl) bM[i] = bA[i] % jj; // bM = bA % jj; mod_vsv

                //FOR(i,vl) x[i] = (bA[i] >> jj_shift);
                //cout<<" bA "<<vecprt(n,wide,bA,vl)<<endl;
                //cout<<" bD "<<vecprt(n,wide,bD,vl)<<endl;
                //cout<<"  x "<<vecprt(n,wide, x,vl)<<"  <--- bA >> "<<jj_shift<<endl;
                FOR(i,vl) assert( bD[i] == (bA[i] >> jj_shift) );         // div --> shift-right

                FOR(i,vl) assert( bM[i] == bA[i] - jj*bD[i] );          // mod --> mult-sub
                FOR(i,vl) assert( bM[i] == (bA[i] & jj_minus_1) );      // mod --> mask
                FOR(i,vl) aA[i] = a[i] + bD[i]; // aA = a + bD; add_vvv
                /**/ FOR(i,vl) b[i] = bM[i]; // bNext is bM
                /**/ FOR(i,vl) a[i] = aA[i]; // aNext is aA
#else // streamlined
                FOR(i,vl) bA[i] = vl + b[i];            // bA = b + vl; add_vsv
                FOR(i,vl) bD[i] = (bA[i] >> jj_shift);  // bD = bA / jj; div_vsv
                FOR(i,vl) b [i] = (bA[i] & jj_minus_1); // bM = bA % jj; mod_vsv
                FOR(i,vl) a [i] = a[i] + bD[i]; // aA = a + bD; add_vvv
#endif
            }else if( 0 && jj < vl ){ // Attempts that I never got better than "_uB" (21-bit lkk) method
                assert( kase == 4 );
                Lpi special = ((iloop-1)*vl)%jj;
                assert( b[0] == special );
                //FOR(i,vl) assert( bA[i] == sq[i] + iloop%(jj%vl) );
                FOR(i,vl) bA[i] = vl + b[i];  // bA = b + vl; add_vsv
                FOR(i,vl)  x[i] = (sq[i] + special) % jj;
                //  Hmmm. it can be done with a rot,rot,merge (maintaining virtual 2*vl vector)
                //cout<<" bA "<<special<<" := "<<vecprt(n,wide,bA,vl)<<endl;
                cout<<" b  "<<b[0]   <<" := "<<vecprt(n,wide,b ,vl)<<endl;
                cout<<"  x "<<special<<" := "<<vecprt(n,wide, x,vl)<<endl;

                FOR(i,vl) bM[i] = bA[i] % jj; // bM = bA % jj; mod_vsv
                FOR(i,vl) bD[i] = bA[i] / jj; // bD = bA / jj; div_vsv
                FOR(i,vl) aA[i] = a[i] + bD[i]; // aA = a + bD; add_vvv
                /**/ FOR(i,vl) b[i] = bM[i]; // bNext is bM
                /**/ FOR(i,vl) a[i] = aA[i]; // aNext is aA
            }else if( 0 && jj<vl && jj%2 == 1 ){ // div-mod ------NEVER REALLY WORKED---------> mul-add/sub, 3 vec ops
                assert( kase == 5 );
#if 1
                FOR(i,vl) bA[i] = vl + b[i];  // bA = b + vl; add_vsv
#if 0 // normal div mod
                FOR(i,vl) bD[i] = bA[i] / jj; // bD = bA / jj; div_vsv
                FOR(i,vl) bM[i] = bA[i] % jj; // bM = bA % jj; mod_vsv
                FOR(i,vl) aA[i] = a[i] + bD[i]; // aA = a + bD; add_vvv
                /**/ FOR(i,vl) b[i] = bM[i]; // bNext is bM
                /**/ FOR(i,vl) a[i] = aA[i]; // aNext is aA
#elif 1 // perhaps faster?
                assert( jj * jj_mod_inverse_lpi == 1 );
                assert( (Vlpi)jj * jj_mod_inverse_Vlpi == 1 );
                FOR(i,vl) assert( 0 <= b[i] );
                FOR(i,vl) assert( b[i] < jj );
                FOR(i,vl) assert( vl <= bA[i] );
                FOR(i,vl) assert( bA[i] < vl+jj );
                if(vl>jj) FOR(i,vl) assert( bA[i] > jj );
                if(vl>jj) FOR(i,vl){
                    if(!( (Uvlpi)bA[i] / (Uvlpi)jj == (Uvlpi)bA[i] * jj_mod_inverse_Vlpi )){
                        cout
                            <<" bA[i]/jj = "<<bA[i]<<"/"<<jj<<" = "<<bA[i]/jj<<endl
                            <<" bA[i]*mi = "<<bA[i]<<"*"<<jj_mod_inverse_Vlpi<<" = "<<bA[i]*jj_mod_inverse_Vlpi<<endl;
                    }
                    assert( bA[i] * jj_mod_inverse_Vlpi != 0 );
                    assert( bA[i] / jj == bA[i] * jj_mod_inverse_Vlpi );
                    // If bA[i] / jj is ZERO, cannot mult by jj_mod_inverse.
                    // Now bA[i] >= vl, so if jj >= vl, bA[i] / jj is NOT ZERO
                }
                //if(jj<vl){
                    FOR(i,vl) bD[i] = bA[i] * jj_mod_inverse_Vlpi;       // div --> mult by mod inv of jj
                //}else{
                //    FOR(i,vl) bD[i] = bA[i]/jj; // NOT EFFICIENT FOR jj > vl
                //}
                FOR(i,vl) bM[i] = bA[i] - jj*bD[i];             // mod --> mult-sub
                FOR(i,vl) aA[i] = a[i] + bD[i]; // aA = a + bD; add_vvv
                /**/ FOR(i,vl) b[i] = bM[i]; // bNext is bM
                /**/ FOR(i,vl) a[i] = aA[i]; // aNext is aA
#elif 0
                FOR(i,vl) bD[i] = bA[i] * jj_mod_inverse_Vlpi;       // div --> mult by mod inv of jj
                FOR(i,vl) bM[i] = bA[i] - jj*bD[i];             // mod --> mult-sub
                FOR(i,vl) aA[i] = a[i] + bD[i]; // aA = a + bD; add_vvv
                /**/ FOR(i,vl) b[i] = bM[i]; // bNext is bM
                /**/ FOR(i,vl) a[i] = aA[i]; // aNext is aA
#else // streamlined
                FOR(i,vl) bD[i] = bA[i] * jj_mod_inverse_Vlpi;        // div --> mult by mod inv of jj
                FOR(i,vl) b[i] = bA[i] - jj * bD[i];                  // mul-sub for b[]!
                FOR(i,vl) a[i] = a[i]  + bD[i];
#endif
#else
                // verify range: verify1()
                FOR(i,vl) bA[i] = vl + b[i];                          // add
                FOR(i,vl) bD[i] = bA[i] * jj_mod_inverse_Vlpi;        // div --> mul
                FOR(i,vl) b[i] = bA[i] - jj * bD[i];                  // b[] w/ mul,sub (for mod)
                FOR(i,vl) a[i] = a[i]  + bD[i];                       // a[] w/ add
#endif
            }else if(1) { // libdivide method -- op count for the divide is always worse (4 or 5)
                FOR(i,vl) bA[i] = vl + b[i];  // bA = b + vl; add_vsv
                unsigned op_count = 0U;
                //FOR(i,vl) bD[i] = bA[i] / jj; // bD = bA / jj; div_vsv
                if( ld.pre_shift == 0 ){
                    FOR(i,vl) bD[i] = ((uint64_t)ld.multiplier * (uint64_t)bA[i]);
                    cout<<" bD0 := "<<vecprt(n,12,bD,vl)<<" mul, no pre-shift"<<endl;
                    ++op_count;
                }else{
                    FOR(i,vl) bD[i] = ((uint64_t)ld.multiplier * (uint64_t)(bA[i] >> ld.pre_shift));
                    cout<<" bD1 := "<<vecprt(n,12,bD,vl)<<" mul w/ pre-shift "<<ld.pre_shift<<endl;
                    ++op_count; ++op_count;
                }
                if( ld.increment ){
                    FOR(i,vl) bD[i] += ld.multiplier;
                    cout<<" bD2 := "<<vecprt(n,12,bD,vl)<<" inc"<<endl;
                    ++op_count;
                }
#if 0
                FOR(i,vl) bD[i] = bD[i] >> 32;
                ++op_count;
                cout<<" bD3 := "<<vecprt(n,12,bD,vl)<<" >>32"<<endl;
                if( ld.post_shift > 0 ){
                    FOR(i,vl) bD[i] = bD[i] >> ld.post_shift;
                    cout<<" bD4 := "<<vecprt(n,12,bD,vl)<<endl;
                    ++op_count;
                }
#else
                FOR(i,vl) bD[i] = bD[i] >> (32+ld.post_shift);
                ++op_count;
#endif
                FOR(i,vl) x[i] = bA[i] / jj;
                cout<<"  x  := "<<vecprt(n,wide, x,vl)<<" <-- expected"<<endl;
                cout<<" libdiv jj="<<jj<<" ops="<<op_count<<" "<<(op_count<2?"BETTER": op_count==2? "SAME": "WORSE")<<" wrt. fastdiv_uB"<<endl;
                FOR(i,vl) bM[i] = bA[i] - bD[i]*jj; // modulo, via the div bD
                //FOR(i,vl) bM[i] = bA[i] % jj; // modulo, via the div bD
                FOR(i,vl) aA[i] = a[i] + bD[i]; // aA = a + bD; add_vvv
                /**/ FOR(i,vl) b[i] = bM[i]; // bNext is bM
                /**/ FOR(i,vl) a[i] = aA[i]; // aNext is aA
            }else{ // div-mod ----------------------------------> 6 vec ops: add (mul,shr) (mul,sub) add
                assert( jj+vl < (1<<21) );
                //assert( kase == 0 );
#if 0
                FOR(i,vl) bA[i] = vl + b[i];  // bA = b + vl; add_vsv
#if 0
                FOR(i,vl) bD[i] = bA[i] / jj; // bD = bA / jj; div_vsv
                FOR(i,vl) bM[i] = bA[i] % jj; // bM = bA % jj; mod_vsv
#elif 1
                FOR(i,vl) bD[i] = bA[i] / jj; // bD = bA / jj; div_vsv
                FOR(i,vl) bM[i] = bA[i] % jj; // bM = bA % jj; mod_vsv
                FOR(i,vl) assert( bD[i] == ((jj_M*bA[i]) >> C) );       // fastdiv_uB for bA[]/jj
                FOR(i,vl) assert( bM[i] == bA[i] - bD[i]*jj );          // long-hand remainder
#else
                FOR(i,vl) bD[i] = ((jj_M*bA[i]) >> C);                  // fastdiv_uB for bA[]/jj       : mul, shr
                FOR(i,vl) bM[i] = bA[i] - bD[i]*jj;                     // long-hand remainder          : mul, sub
#endif
                FOR(i,vl) aA[i] = a[i] + bD[i]; // aA = a + bD; add_vvv
                /**/ FOR(i,vl) b[i] = bM[i]; // bNext is bM
                /**/ FOR(i,vl) a[i] = aA[i]; // aNext is aA
#else // streamlined
                // Note that libdivide methods can extend the range of applicability, but the
                // worst case adds an extra "pre-shift" operation.  The libdivide best case is
                // better:
                //    when pre_shift and post_shift are zero and UINT_BITS is 32, there is
                //    only a single multiply with NO SHIFT.
                // Also, the saturated_increment I think only fixes the "a very very big" case,
                // so it can be ignored [I hope].
                FOR(i,vl) bA[i] = vl + b[i];  // bA = b + vl; add_vsv   // add
                FOR(i,vl) bD[i] = ((jj_M*bA[i]) >> C);                  // fastdiv_uB for bA[]/jj       : mul, shr
                FOR(i,vl) b [i] = bA[i] - bD[i]*jj;                     // long-hand remainder          : mul, sub
                FOR(i,vl) a [i] = a[i] + bD[i];                         // add
#endif
            }
            // Note: for some jj,
            //       the bM,bD divmod operation can be OPTIMIZED to rot etc.
        }

        // Note: vl reduction must take place AFTER above use of "long" vl
        if( cnt + vl > iijj ){ // last time might have reduced vl
            vl = iijj - cnt;
            cout<<" vl reduced for last loop to "<<vl<<endl;
        }
        //cout<<" cnt="<<cnt<<" vl="<<vl<<" iijj="<<iijj<<endl;

        cout<<"__"<<iloop<<" vl,ii,jj "<<vl<<","<<ii<<","<<jj<<"  kase "<<kase<<"  vl\%ii="<<vl%ii<<"  vl\%jj="<<vl%jj<<endl;
        cout<<(iloop==0?"Init  ":"Induce")<<":  "<<vecprt(n,wide,a,vl)<<endl;
        cout<<"         "<<vecprt(n,wide,b,vl)<<endl;
        if(verbose){
            cout<<"I:bA "<<kase<<" "<<vecprt(n,wide,bA,vl)<<" <-- b+vl"<<endl;
            cout<<"I:bM "<<kase<<" "<<vecprt(n,wide,bM,vl)<<" <-- bA%jj = b'"<<endl;
            cout<<"I:bD "<<kase<<" "<<vecprt(n,wide,bD,vl)<<" <-- bA/jj"<<endl;
            cout<<"I:aA "<<kase<<" "<<vecprt(n,wide,aA,vl)<<" <-- a+:!bD ???"<<endl;
            cout<<"I:a0 "<<kase<<" "<<vecprt(n,wide,a0,vl)<<"     ii,jj="<<ii<<","<<jj<<endl;
            cout<<"I:b0 "<<kase<<" "<<vecprt(n,wide,b0,vl)<<"     vl="<<vl<<endl;
        }
        FOR(i,vl) assert( a[i] == vabs[iloop].a[i] );
        FOR(i,vl) assert( b[i] == vabs[iloop].b[i] );
        ++iloop; // just for above debug assertions
        //cout<<" next loop??? cnt+vl="<<cnt+vl<<" iijj="<<iijj<<endl;
#undef FOR
    }
    cout<<" Yay! induction formulas worked!"<<endl;
}

/** opt0: print vector ops (and verify) */
void test_vloop2_unroll(Lpi const vlen, Lpi const ii, Lpi const jj)
{
    // for r in [0,h){ for c in [0,w] {...}}
    assert( vlen > 0 );
    cout<<"test_vloop2_unroll( vlen="<<vlen<<" loops 0.."<<ii<<" 0.."<<jj<<endl;

    // pretty-printing via vecprt
    int const n=8; // output up-to-n [ ... [up-to-n]] ints
    int const bignum = std::max( ii, jj );
    int const wide = 1 + (bignum<10? 1: bignum <100? 2: bignum<1000? 3: 4);

    // generate reference index outputs
    std::vector<Vab> vabs = ref_vloop2(vlen, ii, jj, 1/*verbose*/);

    register uint64_t iijj = (uint64_t)ii * (uint64_t)jj;
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
        test_mod_inverse<uint32_t>();
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
