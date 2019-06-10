#ifndef INT_UTIL_H
#define INT_UTIL_H
/* Copyright (c) 2019 by NEC Corporation
 * This file is part of ve-jit */

#include <stdint.h>

#ifdef __cplusplus
extern "C" { //}
#if __cplusplus >= 201103L
#define FASTDIV_INLINE inline constexpr
#else
#define FASTDIV_INLINE inline
#endif
#else
#define FASTDIV_INLINE inline
#endif

extern int const bitcount[256];
extern int const multiplyDeBruijnBitPosition2[32];

/// \group integer arithmetic
//@{
/** divide-by-\c _odiv via mul-add-shift */
struct fastdiv {
    uint32_t mul;
    uint32_t add;
    int32_t shift;
    uint32_t _odiv;  /* save original divisor for modulo calc */
};

/** \b if positivePow2(v) && v=2**N for N>0, \b then we return N. */
inline int /*constexpr*/ positivePow2Shift(uint32_t v) {
    //assert( positivePow2(v)
    return multiplyDeBruijnBitPosition2[(uint32_t)(v * 0x077CB531U) >> 27];
}
/** unsigned 32-bit log2, required for magic constant generation */
inline uint32_t ulog2(uint32_t v) {
    uint32_t r, shift;
    r =     (v > 0xFFFF) << 4; v >>= r;
    shift = (v > 0xFF  ) << 3; v >>= shift; r |= shift;
    shift = (v > 0xF   ) << 2; v >>= shift; r |= shift;
    shift = (v > 0x3   ) << 1; v >>= shift; r |= shift;
    r |= (v >> 1);
    return r;
}
/** generate constants for implementing a division with multiply-add-shift.
 * \b MODIFIED for VE JIT, so shift is 0 to 64.
 *
 * - x86 shift was 32+N, nice for x86 ops.
 * - VE does better with a single shift
 */
void fastdiv_make(struct fastdiv *d, uint32_t divisor);
//@}

/// \group fastdiv64
/// fast division for 'small enough' numbers,
/// without intermediates overflowing 64 bits.
/// see https://github.com/lemire/constantdivisionbenchmarks, "lkk" algorithm.
/// Issue #1 comments on 64-bit-only version similar to:
///@{
#define FASTDIV_B 21 /* FASTDIV_B up to 21 is OK to keep all intermediates in 64-bit range */
/** *_uB functions were verified for its FASTDIV_SAFEMAX (took a while to run).
 * computeM_uB is two ops: divide + increment.  This might be fast enough
 * to precalculate routinely at runtime for use in loops. */
#define FASTDIV_SAFEMAX ((1U<<FASTDIV_B)-1U)
#define FASTDIV_C (2*FASTDIV_B)
/* 23 zeros and rest (41 = FASTDIV_C-1) ones */
#define FASTDIV_CMASK ((UINT64_C(1)<<FASTDIV_C)-1)

FASTDIV_INLINE uint64_t computeM_uB(uint32_t d) {
    return FASTDIV_CMASK / d + 1;
}

/** fastdiv_uB computes (a / d) given precomputed M for d>1.
 * \pre a,d < (1<<21). */
FASTDIV_INLINE uint32_t fastdiv_uB(uint32_t const a, uint64_t const M) {
    return M*a >> FASTDIV_C; // 2 ops: mul, shr
}
/** fastmod_uB computes \c (a%d) given precomputed \c M.
 * \c M is a pre-computed function of \c d, from \c computeM_uB(d)
 * Probably nicer to calc. uint64_t D=a/d using fastdiv_uB, and then R=a-D*d.
 * (retain "everythin" in u64 registers).
 * \pre \c a and \c d < (1<<21).
 * When d is a power of \f$(v&(v-1)) == 0\f$, just use shifts, \ref positivePow2.
 */
FASTDIV_INLINE uint32_t fastmod_uB(uint32_t const a, uint64_t const M, uint32_t const d) {
    // Lemire original
    //uint64_t lowbits = M * a;
    //return ((__uint128_t)lowbits * d) >> 64;
    // lowbits version
#if 0
    // ugly with 'lowbits' approach, nicer to just do the fastdiv followed by mul and sub :(
    // HOWEVER, in JIT code VE has the mask as an immediate constant!
    uint64_t lowbits = (M * a) & FASTDIV_CMASK;
    return (lowbits * d) >> FASTDIV_C; // ops: mul, and, mul, shr [4 ops]
    // divmod would be like:
    //    i = M*a;
    //    div = i>>C;
    //    j = i & C_MASK; // in parellel w/ prev op
    //    k = j * d;
    //    mod = k>>C;
#endif
    // 3-4 ops: mul, shr, mul-sub... AND get a/d too
    // (VE has no fused mul-sub for unsigned, though, so 4 ops
    return a - (M*a>>FASTDIV_C)*d;
    // divmod would be like:
    //    i = M*a;
    //    div = i>>C;
    //    j = div *d;
    //    mod = a - j;
    // so fewer ops, still 2 multiplies
}

/** Is \c a%d==0 ?
 * \p a numerator
 * \p M is \c compute_M(d)
 * \pre \c a and \c d < (1<<21).
 * If \c a is known to be a power-of-two, use \c (a&(a-1)==0).
 * Especially nice for VE because it avoids rotation (no rotate op).
 */
FASTDIV_INLINE int fastdivisible_uB(uint32_t const a, uint64_t const M) {
    return ((M*a) & FASTDIV_CMASK) <= M-1;
    // obviously much faster than the naive way using fastmod:
    // return (a - ((M*a)>>C)*d) == 0;
    //         ----fastmod_uB----
}

//#undef FASTDIV_CMASK
//#undef FASTDIV_C
//#undef FASTDIV_SAFEMAX
//#undef FASTDIV_B
//@}

#undef FASTDIV_INLINE
#ifdef __cplusplus //{
}//extern "C"
#endif
// vim: ts=4 sw=4 et cindent cino=^=l0,\:.5s,=-.5s,N-s,g.5s,b1 cinkeys=0{,0},0),\:,0#,!^F,o,O,e,0=break
#endif //INT_UTIL_H
