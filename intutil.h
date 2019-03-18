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
#define FASTDIV_INLINE
#endif

extern int const bitcount[256];
extern int const multiplyDeBruijnBitPosition2[32];

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
/** fastmod_uB computes (a % d) given precomputed M.
 * Probably nicer to calc. uint64_t D=a/d using fastdiv_uB, and then R=a-D*d.
 * (retain "everythin" in u64 registers).
 * \pre a,d < (1<<21). */
FASTDIV_INLINE uint32_t fastmod_uB(uint32_t const a, uint64_t const M, uint32_t const d) {
    // ugly with 'lowbits' approach, nicer to just do the fastdiv followed by mul and sub :(
    // 3-4 ops: mul, shr, mul-sub... AND get a/d too (i.e. divmod adds 1 op to the division)
    return a - (M*a>>FASTDIV_C)*d;
}
#undef FASTDIV_CMAKE
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
