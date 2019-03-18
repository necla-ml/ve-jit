#ifndef VFOR_H
#define VFOR_H
/* Copyright (c) 2019 by NEC Corporation
 * This file is part of ve-jit */

/** \macro SHORTLOOP ~ loop length less than simd length */
#if defined(__ve) && (defined(__cplusplus) || __STDC_VERSION__ >= 199901L)
#define SHORTLOOP _Pragma("_NEC shortloop")
#else
#define SHORTLOOP
#endif

/** \macro FOR(VAR,LEN) ~ generic for loop, 0..LEN-1 */
#ifndef FOR
#ifdef __cplusplus
#include <type_traits>
#define FOR(VAR,VLEN) for( \
        std::remove_cv<decltype(VLEN)>::type VAR=0; \
        (VAR) < (VLEN); \
        ++(VAR) )
#else
#define FOR(VAR,VLEN) for( \
        int VAR=0; \
        (VAR) < (VLEN); \
        ++(VAR) )
#endif
#endif

/** \macro VFOR(VAR,LEN) ~ FOR + short loop hint (if available) */
#define VFOR(V,L) SHORTLOOP FOR(V,L)

#endif // VFOR_H
