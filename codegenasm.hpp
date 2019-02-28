#ifndef CODEGENASM_HPP
#define CODEGENASM_HPP
#include "jit_data.h" // C data, ...
#include "stringutil.hpp"
//#include "jitpipe_fwd.hpp"  // just for multiReplace XXX
//#include "velogic.hpp" // fully-tested "load scalar reg with constant"
// Note: required code is in veliFoo.cpp (wrpiFoo.cpp for the tests)
// oh I moved it to asmfmt...
#include "asmfmt_fwd.hpp"

#include <cstdint>
#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>
#include <array>
#include <cassert>
#include <type_traits>

#include "throw.hpp"
#include <typeinfo>

/// \group integer arithmetic helpers
//@{ int arithmetic helpers
/// \group integer multiplicate inverse
/// For what M is A*M always 1? This exists if A and 2^32 (or 2^64) are relatively prime.
//@{
template<typename T> inline T mod_inverse( T const a ){
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
//@}

/** is \c v some 2^N for N>0? */
inline bool constexpr positivePow2(uint64_t v) {
    return ((v & (v-1)) == 0);
}

/// \group C++ jit utilities
/// Do we even want C versions?  Maybe C frontends should
/// just call into the C++ code that can use these.
//@{
/** A portable count-ones routine */
    template<typename T>
inline int popcount(T const n) 
{
    uint8_t *ptr = (uint8_t *)&n;
    int count = 0;
    for (unsigned i=0; i<sizeof(T); ++i) {
        count += bitcount[ptr[i]];
    }
    return count;
}

/** \b if positivePow2(v) && v=2**N for N>0, \b then we return N. */
inline int /*constexpr*/ positivePow2Shift(uint32_t v) {
    //assert( positivePow2(v)
    return multiplyDeBruijnBitPosition2[(uint32_t)(v * 0x077CB531U) >> 27];
}
//@}

/** greatest common denominator, for a,b > 0 */
    template<typename T>
inline T gcd(T a, T b)
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
    template<typename T>
inline T lcm(T a, T b)
{
    int temp = gcd(a, b);
    return temp ? (a / temp * b) : 0;
}

/** For +ve inputs a, b solve k*a + j*b = g for k and j and g=gcd(a,b) */
    template<typename T>
inline void extendedEuclid( T& k, T a, T& j, T b, T& g)
{
    T x = 1, y = 0;
    T xLast = 0, yLast = 1;
    T q, r, m, n;
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
//@} int arithmetic helpers

/** string for VE 'M' representation of a 64-bit value.
 * 'M' representation is (M)B,
 * where M is 0..63 and B is 0|1,
 * meaning "M B's followed by not-B's".
 * \throw if \c t cannot be represented in such format
 */
template<typename T>
std::string jitimm(T const t){
    int64_t i=static_cast<int64_t>(t);
    // common and special cases
    if(i==0) return std::string("(0)1");
    if(i==-1) return std::string("(0)0");
    if(i==1) return std::string("(63)0");
    // remaing cases generic
    std::ostringstream oss;
    bool const neg = i<0;
    char const* leading_bit=(neg? "1": "0");    // string for 1|0 MSB
    if(neg) i=~i;                               // now i == |t|, positive
    if(!positivePow2(i+1))
        THROW("jitimm("<<t<<") not representable as 64-bit N(B) N B's followed by rest not-B");
    int const trailing = popcount(i); // in |t|
    oss<<"("<<64-trailing<<")"<<leading_bit;
    return oss.str();
}
template<typename T>
bool isIval(T const t){
    typename std::make_signed<T>::type i = t;
    return i >= -64 && i <= 63;
}
template<typename T>
bool isMval(T const t){
    int64_t i=static_cast<int64_t>(t);
    // common and special cases
    if(i==0) return true;
    if(i==-1) return true;
    if(i==1) return true;
    // remaing cases generic
    //std::ostringstream oss;
    bool const neg = i<0;
    //char const* leading_bit=(neg? "1": "0");    // string for 1|0 MSB
    if(neg) i=~i;                               // now i == |t|, positive
    if(!positivePow2(i+1))
        //THROW("oops")
        return false;
    //int const trailing = popcount(i); // in |t|
    //oss<<"("<<64-trailing<<")"<<leading_bit;
    //return oss.str();
    return true;
}
template<typename T>
bool isimm(T const t){ // deprecated original name
    return isMval(t);
}

#if 1-1 // original version
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
#endif
/** fast integer division for \em unsigned 21-bit A/D as A*magic(D)>>42.
 * This uses \b only 64-bit arithmetic.
 * The 21-bit limit comes from needing the hi bits of a 21 x 42-bit multiply */
struct FastDiv21{
    static int const bits = 21;
    static int const rshift = 42;
    static int const safemax = uint64_t{1}<<21;
    static uint32_t const mask21 = (1<<bits)-1;
    static inline uint64_t constexpr magic(uint32_t d){
        return mask21 / d + 1;
    }
    static inline uint32_t constexpr fastdiv(uint32_t const a, uint64_t const magic){
        return a*magic>>rshift;
    }
    static inline uint32_t constexpr fastmod(uint32_t const a, uint64_t const magic, uint32_t const d){
        return a - fastdiv(a,magic) * d;
    }
    constexpr FastDiv21(uint32_t const d);
    uint64_t const _m;
    uint32_t const _d;
};
inline constexpr FastDiv21::FastDiv21(uint32_t const d) : _m(magic(d)), _d(d) {}

/** this class returns raw string for assembler ops */
struct CodeGenAsm
{
  public:
      CodeGenAsm() : used(), latest(0) {
          used.fill(0);
      }
      /** scoped tmp reg */
      struct TmpReg
      {
          TmpReg( CodeGenAsm* owner );    ///< allocate and mark used an entry
          ~TmpReg();
          //operator std::string() const { return str; }
          CodeGenAsm * const owner;
          int const tmp;
          //char const* const creg;
          std::string const str;
      };
      struct TmpReg tmp() { return TmpReg(this); }

      /** return asm string to load \c value into scalar register \c reg.
       * \todo There is tested code that covers all reasonable optimizations,
       *       and it should be used to load constants into scalar registers!!!
       */
      std::string load( std::string const reg, uint64_t value );
  private:
      /** return and mark used an entry in tmpRegs */
      int alloc() {
          int f = (latest+1)%10;
          for(int i=0; i<10; ++i){
              if( !used[f] ){
                  used[f] = 1;
                  return latest = f;
              }
          }
          THROW(" too many tmp regs in use");
      }
      void free(int const u){
          used[u] = 0;
      }
      char const* tmpRegs[10] = {
          "%s40","%s41","%s42","%s43","%s44",
          "%s41","%s42","%s43","%s44","%s45"
      };
      std::array<int,10> used;
      int latest;         ///< latest allocated reg
};

    inline CodeGenAsm::TmpReg::TmpReg( CodeGenAsm* owner )
: owner(owner), tmp(owner->alloc()), str(std::string(owner->tmpRegs[tmp]))
{
}
inline CodeGenAsm::TmpReg::~TmpReg() {
    owner->free(tmp);
}
inline std::string CodeGenAsm::load( std::string const reg, uint64_t value ){
#if 1
    return multiReplace("OUT",reg,
            choose(opLoadregStrings(value)));
#else
    std::ostringstream oss;
    if( (int64_t)value >= -64 && (int64_t)value <= 63 ){
        auto t=tmp();
        oss<<"or "<<reg<<","<<(int64_t)value<<","<<jitimm(0);
    }else if( isimm(value) ){
        auto t=tmp();
        oss<<"or "<<reg<<","<<0<<","<<jitimm(value);
    }else if( value < (uint64_t{1}<<32) ){
        oss<<"lea "<<reg<<","<<jitdec(value);
    }else if( (int64_t)value < (int64_t)(int32_t)value ){
        // sext(32-bit) can yield 64-bit value exactly
        oss<<"lea "<<reg<<","<<jitdec(value);
        //else search for xor I,M
        //else search for eqv I,M
        //else search for nnd I,M
        //else search for add I,M
        //else search for sub I,M
        //else search for mpy I,M
        // ?? 2 instruction LEA ??
    }else{
        // needs testing !!! -- often have an explicit shl !!!
        uint32_t hi = (value>>32);
        uint32_t lo = (value & ((uint64_t{1}<<32)-1));
        if((int32_t)lo >= 0){ // sext(lo) has zeros in hi word
            auto t=tmp();
            oss<<"lea.sl "<<t.str<<",0x"<<std::hex<<hi<<"#(TmpReg);";
            oss<<"lea "<<reg<<",0x"<<std::setfill('0')<<std::setw(8)<<lo<<"(,"<<t.str<<")";
        }else{
            auto t=tmp();
            oss<<"lea.sl "<<t.str<<",0x"<<std::hex<<~hi<<";"; // sext of lo will toggle hi bits to OK
            oss<<"lea "<<reg<<","<<std::setfill('0')<<std::setw(8)<<lo<<"(,"<<t.str<<")";
        }
    }
    return oss.str();
#endif
            }
// vim: ts=4 sw=4 et cindent cino=^=l0,\:.5s,=-.5s,N-s,g.5s,b1 cinkeys=0{,0},0),\:,0#,!^F,o,O,e,0=break 
#endif //CODEGENASM_HPP
