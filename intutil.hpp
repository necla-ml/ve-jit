#ifndef INTUTIL_HPP
#define INTUTIL_HPP
/* Copyright (c) 2019 by NEC Corporation
 * This file is part of ve-jit */

#include "intutil.h"
#include "throw.hpp"

#include <cstdint>
#include <typeinfo>		// for an error
//#include <type_traits>	// for an error msg

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

#endif // INTUTIL_HPP
