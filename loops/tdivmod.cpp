/* Copyright (c) 2019 by NEC Corporation
 * This file is part of ve-jit */

/* \file
 * see https://stackoverflow.com/questions/11040646/faster-modulus-in-c-c
 *
 * These measurements don't quite reflect JIT implementations
 */
#if 0
/*
Branchless non-power-of-two modulus is possible by precomputing magic constants
at run-time, to implement division using a multiply-add-shift.

This is roughly 2x faster than the built-in modulo operator % on my Intel Core
i5.

I'm surprised it's not more dramatic, as x86 CPU div instructions can have
latencies as high as 80-90 cycles for 64-bit division on some CPUs, compared to
mul at 3 cycles and bitwise ops at 1 cycle each.

Proof of concept and timings shown below. series_len refers to the number of
modulus ops performed in series on a single var. That's to prevent the CPU from
hiding latencies through parallelization.
*/
#endif

//#include "intutil.h"
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include "timer.h"
#include "intutil.hpp"
#if defined(__ve)
#include "veintrin.h"
#endif

typedef int32_t s32;
typedef uint32_t u32;
typedef uint64_t u64;
typedef  int64_t s64;
typedef long long unsigned llu;

#define NUM_NUMS 2048
#define NUM_RUNS 500
#define MAX_DEN 1024

//#define MAX_NUM UINT32_MAX 
/* intutil.h FASTDIV (mul+shift) limit so intermediates fit in 64 bits */
#define MAX_NUM FASTDIV_SAFEMAX

#if 0 // moved to intutil.h
#ifdef __ve
struct fastdiv {
    u64 mul;
    u64 add;
    s64 shift;
    u64 _odiv;  /* save original divisor for modulo calc */
};
#else
struct fastdiv {
    u32 mul;
    u32 add;
    s32 shift;
    u32 _odiv;  /* save original divisor for modulo calc */
};
#endif
#endif


// based on intutil.hpp method
struct fastdiv21 {
    u64 mul;
    u32 add;    /* always zero */
    u32 _odiv;  /* save original divisor for modulo calc */
};

static u32 num[NUM_NUMS];
static u32 den[NUM_NUMS];
static struct fastdiv fd[NUM_NUMS];
static struct fastdiv21 fd21[NUM_NUMS];
// alternate as struct-of-vectors, in prep for vectorized measurements
static u64 vfd_mul[NUM_NUMS];
static u64 vfd_add[NUM_NUMS];
static u64 vfd_shift[NUM_NUMS];
static u64 vfd_odiv[NUM_NUMS];

/* generate constants for implementing a division with multiply-add-shift */
void fastdiv_make(struct fastdiv *d, u32 divisor) {
    u32 l, r, e;
    u64 m;

    d->_odiv = divisor;
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
#if defined(__ve)
    d->shift += 32;
#endif
}
void fastdiv21_make(struct fastdiv21 *d, u32 divisor) {
    d->_odiv = divisor;
    d->mul = computeM_uB(divisor);
    d->add = 0;
}

/* 0: use function that checks for a power-of-2 modulus (speedup for POTs)
 * 1: use inline macro */
#define FASTMOD_BRANCHLESS 0

#if defined(__ve)
//#define FASTDIV(v,d) ((u32)(( (u64)(v)* (u32)((d)->mul) + (d)->add) >> 32) >> (d)->shift)
// equiv[x86] ~10% faster for VE (1088 ns @ 128)
//#define FASTDIV(v,d) ((u64)(( (u64)(v)* (u64)((d)->mul) + (d)->add) >> 32) >> (d)->shift)
// --> 995 ns for ve
//#define FASTDIV(v,d) ((u64)(( (u64)(v)* (u64)((d)->mul) + (d)->add) >> (32 + (d)->shift)))
#define FASTDIV(v,d) ((u64)(( (u64)(v)* (u64)((d)->mul) + (d)->add) >> ((d)->shift)))
#else
#define FASTDIV(v,d) ((u32)(( (u64)(v)* (u32)((d)->mul) + (d)->add) >> 32) >> (d)->shift)
// equiv[x86] ~10% faster for VE (1088 ns @ 128)
//#define FASTDIV(v,d) ((u64)(( (u64)(v)* (u64)((d)->mul) + (d)->add) >> 32) >> (d)->shift)
#endif
#define _fastmod(v,d) ((v) - FASTDIV((v),(d)) * (d)->_odiv)

//#define fastdiv21(v,d) ((u32)(((u64)(v) * (d)->mul + (d)->add) >> 32) >> 10) // 294 ns
//#define fastdiv21(v,d) ((u32)(((u64)(v) * (d)->mul + 0) >> 32) >> 10) // 256 ns
//#define fastdiv21(v,d) (((u64)(v)*(d)->mul) >> FASTDIV_C) // 256 ns
//#define fastdiv21(v,d) ((u32) ( ((u64)(v) * ((u64)(d)->mul) ) >> FASTDIV_C) ) // 256 ns
//
//  BEST on VE : series len 128 at 710 ns per
#define fastdiv21(v,d) ( ( ((u64)(v) * ((u64)(d)->mul) ) >> FASTDIV_C) ) // 256 ns
#define _fastmod21(v,d) ((v) - fastdiv21((v),(d)) * (d)->_odiv)

#if FASTMOD_BRANCHLESS
#define fastmod(v,d) _fastmod((v),(d))
#else
u32 fastmod(u32 v, struct fastdiv *d) {
    if (d->mul == 0x80000000) {
        return (v & ((1 << d->shift) - 1));
    }
    return _fastmod(v,d);
}
#endif

u32 random32(u32 upper_bound) {
	static u64 r64=12345ULL;
    //return arc4random_uniform(upper_bound);
    return ((r64*=2862933555777941757UL)+=13U) % upper_bound;
}

u32 random32_range(u32 lower_bound, u32 upper_bound) {
    return random32(upper_bound - lower_bound) + lower_bound;
}

void fill_arrays() {
    int i;
    for (i = 0; i < NUM_NUMS; ++i) {
        num[i] = random32_range(MAX_DEN, MAX_NUM);
        den[i] = random32_range(1, MAX_DEN);
        fastdiv_make(&fd[i], den[i]);
        vfd_mul[i] = fd[i].mul;
        vfd_add[i] = fd[i].add;
        vfd_shift[i] = fd[i].shift;
        vfd_odiv[i] = fd[i]._odiv;
    }
}
void fill_arrays21() {
    int i;
    for (i = 0; i < NUM_NUMS; ++i) {
        num[i] = random32_range(MAX_DEN, MAX_NUM);
        den[i] = random32_range(1, MAX_DEN);
        fastdiv21_make(&fd21[i], den[i]);
        vfd_mul[i] = fd21[i].mul;
        vfd_add[i] = 0;
        vfd_shift[i] = 42;
        vfd_odiv[i] = fd21[i]._odiv;
    }
}


// pot ~ Power Of Two !
void fill_arrays_pot() {
    u32 log_bound, rand_log;
    int i;

    log_bound = ulog2(MAX_DEN);
    for (i = 0; i < NUM_NUMS; ++i) {
        num[i] = random32_range(MAX_DEN, MAX_NUM);
        rand_log = random32(log_bound) + 1;
        den[i] = 1 << rand_log;
        fastdiv_make(&fd[i], den[i]);
        vfd_mul[i] = fd[i].mul;
        vfd_add[i] = fd[i].add;
        vfd_shift[i] = fd[i].shift;
        vfd_odiv[i] = fd[i]._odiv;
    }
}

void fill_arrays21_pot() {
    u32 log_bound, rand_log;
    int i;

    log_bound = ulog2(MAX_DEN);
    for (i = 0; i < NUM_NUMS; ++i) {
        num[i] = random32_range(MAX_DEN, MAX_NUM);
        rand_log = random32(log_bound) + 1;
        den[i] = 1 << rand_log;
        fastdiv21_make(&fd21[i], den[i]);
        vfd_mul[i] = fd21[i].mul;
        vfd_add[i] = 0;
        vfd_shift[i] = 42;
        vfd_odiv[i] = fd21[i]._odiv;
    }
}

static double const cyc2ns = cycle2ns();
u64 clock_ns() {
#if 0
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec*1000000000 + tv.tv_usec*1000;
#else
    return (u64)(__cycle()*cyc2ns+0.5);
#endif
}
unsigned long long to_ns(double avg_cyc){
    return (unsigned long long)( avg_cyc * cyc2ns );
}
double to_ns_f(double avg_cyc){
    return ( avg_cyc * cyc2ns );
}


/* hash of results to prevent gcc from optimizing out our ops */
static u32 cookie = 0;
static u64 cookie64 = 0;
void use_value(u32 v) {
    cookie += v;
}
void use_value(u64 v) {
    cookie64 += v;
}

int main(int argc, char **arg) {
    double builtin_npot_cyc;
    double builtin_pot_cyc;
    double branching_npot_cyc;
    double branching_pot_cyc;
    double branchless_npot_cyc;
    double branchless_pot_cyc;
    double fd21_npot_cyc;
    double fd21_pot_cyc;
#if defined(__ve)
    double vfdiv_npot_cyc = 0;
    double vfd21_npot_cyc = 0;
    __vr vcookie=_ve_vbrd_vs_i64(0);
#endif
    u64 t0, t1;
    int s, r, i, j;
    int series_len;
    printf(" cyc2ns = %f __cycle=%llu, __cycle=%llu\n", cyc2ns, (llu)__cycle(), (llu)__cycle());

    builtin_npot_cyc = builtin_pot_cyc = 0;
    branching_npot_cyc = branching_pot_cyc = 0;
    branchless_npot_cyc = branchless_pot_cyc = 0;
    fd21_npot_cyc = fd21_pot_cyc = 0;

    for (s = 8; s >= 0; --s) {
        series_len = 1 << s;
        for (r = 0; r < NUM_RUNS; ++r) {
            /* built-in NPOT */
            fill_arrays();
			asm volatile ("###built-in NPOT");
            t0 = __cycle();
            for (i = 0; i < NUM_NUMS; ++i) {
                u32 v = num[i];
                for (j = 0; j < series_len; ++j) {
                    v %= den[i];
                }
                use_value(v);
            }
            t1 = __cycle();
            builtin_npot_cyc += (double)(t1 - t0) / NUM_NUMS;

            /* fd21 NPOT */
            fill_arrays21();
			asm volatile ("###1");
            t0 = __cycle();
            for (u64 ii = 0; ii < NUM_NUMS; ++ii) {
                u64 v = num[ii];
                //for (j = 0; j < series_len; ++j) {
                //    v = _fastmod21(v, fd21+ii);
                //    // equiv ?
                //    //v = fastmod_uB(v,fd[ii].mul,fd[ii]._odiv); // slower for VE
                //}
                //  rewrite:
                //for (j = 0; j < series_len; ++j)
                //    v = v - ((v*fd21[ii].mul)>>FASTDIV_C)*fd21[ii]._odiv;
                // rewrite: avoid struct, use vectors directly (same VE speed)
                for (j = 0; j < series_len; ++j)
                    v = v - ((v*vfd_mul[ii])>>FASTDIV_C) * vfd_odiv[ii];
                use_value(v);
            }
            t1 = __cycle();
            fd21_npot_cyc += (double)(t1 - t0) / NUM_NUMS;

#if 1 && defined(__ve)
            /* native VDIV VEC */
            assert( series_len <= 256 );
            fill_arrays21();
			asm volatile ("###VEDIV");
            t0 = __cycle();
            _ve_lvl(series_len);
            __vr const v_odiv = _ve_vld_vss(8,vfd_odiv);
            for (u64 ii = 0; ii < NUM_NUMS; ++ii) {
                u64 n = num[ii];
                //for (j = 0; j < series_len; ++j) {
                //    v = _fastmod21(v, fd21+ii);
                //}
                __vr v = _ve_vbrd_vs_i64(n);
                // naive VDIV  vlen=256 --> 46 ns
                //__vr d = _ve_vdivul_vvv(v, v_odiv);
                __vr d = _ve_vdivul_vvs(v, 13);
                __vr m = _ve_vmulul_vvv(d, v_odiv);
                v = _ve_vsubul_vvv(v,m);
                //use_value(v);
                vcookie = _ve_vxor_vvv(vcookie,v);
            }
            t1 = __cycle();
            vfdiv_npot_cyc += (double)(t1 - t0) / NUM_NUMS;

            /* fd21 VEC */
            assert( series_len <= 256 );
            fill_arrays21();
			asm volatile ("###VE21");
            t0 = __cycle();
            _ve_lvl(series_len);
            __vr const v_mult = _ve_vld_vss(8,vfd_mul);
            for (u64 ii = 0; ii < NUM_NUMS; ++ii) {
                u64 n = num[ii];
                //for (j = 0; j < series_len; ++j) {
                //    v = _fastmod21(v, fd21+ii);
                //}
                __vr v = _ve_vbrd_vs_i64(n);
                // vectorized FASTDIV vlen=256 --> 11 ns
                __vr x = _ve_vmulul_vvv(v, v_mult);
                __vr y = _ve_vsrl_vvs(x, FASTDIV_C);
                __vr z = _ve_vmulul_vvv(y, v_odiv);
                v = _ve_vsubul_vvv(v,z);
                //use_value(v);
                vcookie = _ve_vxor_vvv(vcookie,v);
            }
            t1 = __cycle();
            vfd21_npot_cyc += (double)(t1 - t0) / NUM_NUMS;
#endif

            /* branchless NPOT */
            fill_arrays();
			asm volatile ("###branchless NPOT");
            t0 = __cycle();
            for (i = 0; i < NUM_NUMS; ++i) {
                u64 v = num[i]; // u64 here is big win for VE
                for (j = 0; j < series_len; ++j) {
                    v = _fastmod(v, fd+i);
                }
                use_value(v);
            }
            t1 = __cycle();
            branchless_npot_cyc += (double)(t1 - t0) / NUM_NUMS;

#if 0 // VE "best" for series_len = 128
            // builtin_npot_cyc    : 1825 ns
            // fd21_npot_cyc       : 713 ns
            // branchless_npot_cyc : 815 ns
#endif
#if 0
            /* built-in POT */
            fill_arrays_pot();
			asm volatile ("###built-in POT");
            t0 = __cycle();
            for (i = 0; i < NUM_NUMS; ++i) {
                u32 v = num[i];
                for (j = 0; j < series_len; ++j) {
                    v %= den[i];
                }
                use_value(v);
            }
            t1 = __cycle();
            builtin_pot_cyc += (double)(t1 - t0) / NUM_NUMS;

            /* fd21 POT */
            fill_arrays_pot();
			asm volatile ("###2");
            t0 = __cycle();
            for (i = 0; i < NUM_NUMS; ++i) {
                u64 v = num[i]; // no spped diff for u64/u32 [x86]
                for (j = 0; j < series_len; ++j) {
                    v = _fastmod21(v, fd+i);
                }
                use_value(v);
            }
            t1 = __cycle();
            fd21_pot_cyc += (double)(t1 - t0) / NUM_NUMS;

            /* branching NPOT */
            fill_arrays();
			asm volatile ("###branching NPOT");
            t0 = __cycle();
            for (i = 0; i < NUM_NUMS; ++i) {
                u32 v = num[i];
                for (j = 0; j < series_len; ++j) {
                    v = fastmod(v, fd+i);
                }
                use_value(v);
            }
            t1 = __cycle();
            branching_npot_cyc += (double)(t1 - t0) / NUM_NUMS;

            /* branching POT */
            fill_arrays_pot();
			asm volatile ("###branching POT");
            t0 = __cycle();
            for (i = 0; i < NUM_NUMS; ++i) {
                u32 v = num[i];
                for (j = 0; j < series_len; ++j) {
                    v = fastmod(v, fd+i);
                }
                use_value(v);
            }
            t1 = __cycle();
            branching_pot_cyc += (double)(t1 - t0) / NUM_NUMS;

            /* branchless NPOT */
            fill_arrays();
			asm volatile ("###branchless NPOT");
            t0 = __cycle();
            for (i = 0; i < NUM_NUMS; ++i) {
                u32 v = num[i];
                for (j = 0; j < series_len; ++j) {
                    v = _fastmod(v, fd+i);
                }
                use_value(v);
            }
            t1 = __cycle();
            branchless_npot_cyc += (double)(t1 - t0) / NUM_NUMS;

            /* branchless POT */
            fill_arrays_pot();
			asm volatile ("###branchless POT");
            t0 = __cycle();
            for (i = 0; i < NUM_NUMS; ++i) {
                u32 v = num[i];
                for (j = 0; j < series_len; ++j) {
                    v = _fastmod(v, fd+i);
                }
                use_value(v);
            }
            t1 = __cycle();
            branchless_pot_cyc += (double)(t1 - t0) / NUM_NUMS;
#endif

        }

        builtin_npot_cyc /= NUM_RUNS;
        builtin_pot_cyc /= NUM_RUNS;
        branching_npot_cyc /= NUM_RUNS;
        branching_pot_cyc /= NUM_RUNS;
        branchless_npot_cyc /= NUM_RUNS;
        branchless_pot_cyc /= NUM_RUNS;
        fd21_npot_cyc /= NUM_RUNS;
#if defined(__ve)
        vfdiv_npot_cyc /= NUM_RUNS;
        vfd21_npot_cyc /= NUM_RUNS;
#endif
        fd21_pot_cyc /= NUM_RUNS;

        printf("series_len = %d\n", series_len);
        printf("----------------------------\n");
        if(builtin_npot_cyc    != 0)
            printf("builtin_npot_cyc    : %.1fns\n", to_ns_f(builtin_npot_cyc));
        if(builtin_pot_cyc     != 0)
            printf("builtin_pot_cyc     : %.1f ns\n", to_ns_f(builtin_pot_cyc));
        if(fd21_npot_cyc       !=0)
            printf("fd21_npot_cyc       : %.1f ns\n", to_ns_f(fd21_npot_cyc));
#if defined(__ve)
        if(vfdiv_npot_cyc      !=0)
            printf("vfdiv_npot_cyc       : %.1f ns\n", to_ns_f(vfdiv_npot_cyc));
        if(vfd21_npot_cyc      !=0)
            printf("vfd21_npot_cyc       : %.1f ns\n", to_ns_f(vfd21_npot_cyc));
#endif
        if(fd21_pot_cyc        !=0)
            printf("fd21_pot_cyc        : %.1f ns\n", to_ns_f(fd21_pot_cyc));
        if(branching_npot_cyc  !=0)
            printf("branching_npot_cyc  : %.1f ns\n", to_ns_f(branching_npot_cyc));
        if(branching_pot_cyc   !=0)
            printf("branching_pot_cyc   : %.1f ns\n", to_ns_f(branching_pot_cyc));
        if(branchless_npot_cyc !=0)
            printf("branchless_npot_cyc : %.1f ns\n", to_ns_f(branchless_npot_cyc));
        if(branchless_pot_cyc  !=0)
            printf("branchless_pot_cyc  : %.1f ns\n\n", to_ns_f(branchless_pot_cyc));
    }
#if defined(__ve)
    vcookie += _ve_vsuml_vv(vcookie);
    cookie64 += _ve_lvs_svs_u64(vcookie,0);
#endif
    printf("cookie=%llu\n", (llu)(cookie+cookie64));
}

#if 0
Results
Intel Core i5 (MacBookAir7,2), macOS 10.11.6, clang 8.0.0

series_len = 32
----------------------------
builtin_npot_cyc    : 218 ns
builtin_pot_cyc     : 225 ns
branching_npot_cyc  : 115 ns
branching_pot_cyc   : 42 ns
branchless_npot_cyc : 110 ns
branchless_pot_cyc  : 110 ns

series_len = 16
----------------------------
builtin_npot_cyc    : 87 ns
builtin_pot_cyc     : 89 ns
branching_npot_cyc  : 47 ns
branching_pot_cyc   : 19 ns
branchless_npot_cyc : 45 ns
branchless_pot_cyc  : 45 ns

series_len = 8
----------------------------
builtin_npot_cyc    : 32 ns
builtin_pot_cyc     : 34 ns
branching_npot_cyc  : 18 ns
branching_pot_cyc   : 10 ns
branchless_npot_cyc : 17 ns
branchless_pot_cyc  : 17 ns

series_len = 4
----------------------------
builtin_npot_cyc    : 15 ns
builtin_pot_cyc     : 16 ns
branching_npot_cyc  : 8 ns
branching_pot_cyc   : 3 ns
branchless_npot_cyc : 7 ns
branchless_pot_cyc  : 7 ns

series_len = 2
----------------------------
builtin_npot_cyc    : 8 ns
builtin_pot_cyc     : 7 ns
branching_npot_cyc  : 4 ns
branching_pot_cyc   : 2 ns
branchless_npot_cyc : 2 ns
branchless_pot_cyc  : 2 ns
#endif
// vim: ts=4 sw=4 et cindent cino=^=l0,\:.5s,=-.5s,N-s,g.5s,b1 cinkeys=0{,0},0),\:,0#,!^F,o,O,e,0=break
