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


#if defined(__ve) && defined(__clang__)
#define VE_INTRINSICS 1
#else
#define VE_INTRINSICS 0
#endif

#if VE_INTRINSICS
#include "velintrin.h"
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
static u64 tmp[NUM_NUMS];
static u64 tmp2[NUM_NUMS];

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
        tmp[i] = 0;
        tmp2[i] = 0;
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
        tmp[i] = num[i];
        tmp2[i] = 0;
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
        tmp[i] = 0;
        tmp2[i] = 0;
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
        tmp[i] = 0;
        tmp2[i] = 0;
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
void use_value(u32 *v, int const vl) {
#if 1 // orig
    cookie += v[(v[0]+13)%vl];
#else // since use_value(pvec,vl) is outside timing loop now...
    for(int i=0; i<vl; ++i){
        cookie += v[i];
    }
#endif
}
void use_value(u64 *v, int const vl) {
    cookie += v[(v[0]+8)%vl];
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
#if VE_INTRINSICS
    double vfdiv_npot_cyc = 0;
    double vfd21_npot_cyc = 0;
    __vr vcookie=_vel_vbrdl_vsl(0,256);
#endif
    u64 t0, t1;
    int s, r, i, j;
    int series_len;
    printf(" cyc2ns = %f __cycle=%llu, __cycle=%llu\n", cyc2ns, (llu)__cycle(), (llu)__cycle());

    builtin_npot_cyc = builtin_pot_cyc = 0;
    branching_npot_cyc = branching_pot_cyc = 0;
    branchless_npot_cyc = branchless_pot_cyc = 0;
    fd21_npot_cyc = fd21_pot_cyc = 0;

#define U_MOD_VEC 0
#define VEC_MOD_U 1
#define VEC_DIVMOD_U 2

//#define TEST U_MOD_VEC
#define TEST VEC_MOD_U

#if TEST == U_MOD_VEC
#elif TEST == VEC_MOD_U
#elif TEST == VEC_DIVMOD_U
#else
#error "unimplemented TEST"
#endif
    for (s = 8; s >= 0; --s) {
        series_len = 1 << s;
        for (r = 0; r < NUM_RUNS; ++r) {
            /* built-in NPOT */
            fill_arrays();
			asm volatile ("###built-in NPOT");
            t0 = __cycle();
#if TEST == U_MOD_VEC
            for (i = 0; i < NUM_NUMS; ++i) {
                for (j=0; j<series_len; ++j)
                    tmp2[j] += num[i] % den[j];
            }
#elif TEST == VEC_MOD_U
            // ncc 1544.1 ns
            // clang 2926.4 ns
            for (i = 0; i < NUM_NUMS; ++i) {
                for (j=0; j<series_len; ++j)
                    tmp2[j] += num[j] % den[i];
            }
#elif TEST == VEC_DIVMOD_U
#endif
            t1 = __cycle();
            use_value(tmp2,series_len);
            builtin_npot_cyc += (double)(t1 - t0) / NUM_NUMS;

            /* fd21 NPOT */
            fill_arrays21();
			asm volatile ("###1");
            t0 = __cycle();
#if TEST == U_MOD_VEC
            // ncc 440.1 ns
            // clang 535.2 ns
            for (u64 ii = 0; ii < NUM_NUMS; ++ii) {
                uint64_t const ni = num[ii];
                for (j = 0; j < series_len; ++j)
                    tmp2[j] += ni - ((ni*vfd_mul[j])>>FASTDIV_C)
                        /*     */ * vfd_odiv[j];
            }
#elif TEST == VEC_MOD_U
            for (i=0; i<NUM_NUMS; ++i) {
                u64 const d = den[i];
                for (j=0; j<series_len; ++j){
                    //tmp[j] = num[i]*vfd_mul[i] >> FASTDIV_C; // tmp=num/d
                    //tmp2[j] += num[i]-tmp[j]*d; // tmp2+=[num-num/d*d = num%d]
                    tmp2[j] += num[i] - (num[i]*vfd_mul[i] >> FASTDIV_C)
                        /*         */ * d;
                }
            }
#endif
            t1 = __cycle();
            use_value(tmp2,series_len);
            fd21_npot_cyc += (double)(t1 - t0) / NUM_NUMS;


#if VE_INTRINSICS
            /* native VDIV VEC */
            assert( series_len <= 256 );
            fill_arrays21();
            int const vl = series_len;
			asm volatile ("###VEDIV");
#if 0
            t0 = __cycle();
            { // 45.3 ns
                __vr const v_odiv = _vel_vld_vssl(8,vfd_odiv,vl);
                for (u64 ii = 0; ii < NUM_NUMS; ++ii) {
                    u64 n = num[ii];
                    //for (j = 0; j < series_len; ++j) {
                    //    v = _fastmod21(v, fd21+ii);
                    //}
                    __vr v = _vel_vbrdl_vsl(n,vl);
                    // naive VDIV  vlen=256 --> 46 ns
                    __vr d = _vel_vdivul_vvvl(v, v_odiv, vl);
                    __vr m = _vel_vmulul_vvvl(d, v_odiv, vl);
                    v = _vel_vsubul_vvvl(v,m,vl);
                    //use_value(v);
                    vcookie = _vel_vxor_vvvl(vcookie,v,vl);
                }
            }
            t1 = __cycle();
#else
#if TEST == U_MOD_VEC
            { // 45.1 ns
                __vr const v_odiv = _vel_vld_vssl(8,vfd_odiv,vl); // den[j]
                __vr vtmp2 = _vel_vld_vssl(8,tmp2,vl);
                t0 = __cycle();
                for (u64 ii = 0; ii < NUM_NUMS; ++ii) {
                    uint64_t const ni = num[ii];
                    //__vr v = _vel_vbrdl_vsl(ni,vl);   // div(scalar,vector) OK
                    // VECTORIZED: for (j=0; j<series_len; ++j)
                    {
                        //__vr vd = _vel_vdivul_vvvl(v, v_odiv,vl);  // ni/den[j]
                        __vr vd = _vel_vdivul_vsvl(ni,v_odiv,vl);  // ni/den[j]
                        __vr vx = _vel_vmulul_vvvl(vd,v_odiv,vl);
                        __vr md = _vel_vsubul_vsvl(ni,vx,vl);       // ni%den[j]
                        vtmp2 = _vel_vaddul_vvvl(vtmp2,md,vl); // tmp2 += ni%den[j]
                        //vcookie = _vel_vxor_vvvl(vcookie,vtmp2,vl);
                    }
                }
                t1 = __cycle();
                _vel_vst_vssl(vtmp2,8,tmp2,vl);
            }
#elif TEST == VEC_MOD_U
            { // 45.1 ns
                __vr const vn = _vel_vld_vssl(8,tmp,vl);       // vn=num[j]
                __vr vtmp2 = _vel_vld_vssl(8,tmp2,vl);
                t0 = __cycle();
                for (u64 ii = 0; ii < NUM_NUMS; ++ii) {
                    u64 const d = den[ii];
                    // VECTORIZED: for (j=0; j<series_len; ++j)
                    {
                        __vr vd = _vel_vdivul_vvsl(vn, d,vl);   // vd=vn/d
                        __vr vx = _vel_vmulul_vsvl( d,vd,vl);
                        __vr vm = _vel_vsubul_vvvl(vn,vx,vl);  // ni%den[j]
                        vtmp2 = _vel_vaddul_vvvl(vtmp2,vm,vl); // tmp2 += vn%d
                        //vcookie = _vel_vxor_vvvl(vcookie,vtmp2,vl);
                    }
                }
                t1 = __cycle();
                _vel_vst_vssl(vtmp2,8,tmp2,vl);
            }
#elif TEST == VEC_DIVMOD_U
#endif
            use_value(tmp2,series_len);
#endif
            vfdiv_npot_cyc += (double)(t1 - t0) / NUM_NUMS;

            /* fd21 VEC */
            assert( series_len <= 256 );
            fill_arrays21();
			asm volatile ("###VE21");
#if 0
            t0 = __cycle();
            { // 11.6 ns
                __vr const v_odiv = _vel_vld_vssl(8,vfd_odiv,vl); // den[j]
                __vr const v_mult = _vel_vld_vssl(8,vfd_mul,vl);
                for (u64 ii = 0; ii < NUM_NUMS; ++ii) {
                    u64 n = num[ii];
                    //for (j = 0; j < series_len; ++j) {
                    //    v = _fastmod21(v, fd21+ii);
                    //}
                    __vr v = _vel_vbrdl_vsl(n,vl);
                    // vectorized FASTDIV vlen=256 --> 11 ns
                    __vr x = _vel_vmulul_vvvl(v, v_mult, vl);
                    __vr d = _vel_vsrl_vvsl(x, FASTDIV_C, vl);
                    __vr m = _vel_vmulul_vvvl(d, v_odiv, vl);
                    v = _vel_vsubul_vvvl(v,z,vl);
                    //use_value(v);
                    vcookie = _vel_vxor_vvv(vcookie,v,vl);
                }
            }
            t1 = __cycle();
#else
#if TEST == U_MOD_VEC
            { // 8.7 ns (>6x speedup)
                __vr const v_odiv = _vel_vld_vssl(8,vfd_odiv,vl); // den[j]
                __vr const v_mult = _vel_vld_vssl(8,vfd_mul,vl);
                __vr vtmp2 = _vel_vld_vssl(8,tmp2,vl);
                t0 = __cycle();
                for (u64 ii = 0; ii < NUM_NUMS; ++ii) {
                    uint64_t const ni = num[ii];
                    //__vr const v = _vel_vbrdl_vsl(ni,vl);
                    // VECTORIZED: for (j=0; j<series_len; ++j)
                    {
                        __vr x = _vel_vmulul_vsvl(ni, v_mult, vl);  // FASTDIV21
                        __vr d = _vel_vsrl_vvsl(x, FASTDIV_C, vl); // ni/den[j]
                        __vr m = _vel_vmulul_vvvl(d, v_odiv,vl);
                        __vr md = _vel_vsubul_vsvl(ni,m,vl);        // ni%den[j]
                        vtmp2 = _vel_vaddul_vvvl(vtmp2,md,vl); // tmp2 += ni%den[j]
                        //vcookie = _vel_vxor_vvvl(vcookie,vtmp2,vl);
                    }
                }
                t1 = __cycle();
                _vel_vst_vssl(vtmp2,8,tmp2,vl);
            }
#elif TEST == VEC_MOD_U
            { // 5.6 ns
                __vr const vn = _vel_vld_vssl(8,tmp,vl);       // vn=num[j]
                __vr vtmp2 = _vel_vld_vssl(8,tmp2,vl);
                t0 = __cycle();
                for (u64 ii = 0; ii < NUM_NUMS; ++ii) {
                    u64 const d = den[ii];
                    // VECTORIZED: for (j=0; j<series_len; ++j)
                    {
                        __vr vf = _vel_vmulul_vsvl(vfd_mul[i],vn,vl); //FASTDIV21
                        __vr vd = _vel_vsrl_vvsl(vf,FASTDIV_C,vl);    //vn/d
                        //__vr vd = _vel_vdivul_vvsl(vn, d,vl);   // vd=vn/d
                        __vr vx = _vel_vmulul_vsvl( d,vd,vl);
                        __vr vm = _vel_vsubul_vvvl(vn,vx,vl);  // ni%den[j]
                        vtmp2 = _vel_vaddul_vvvl(vtmp2,vm,vl); // tmp2 += vn%d
                        //vcookie = _vel_vxor_vvvl(vcookie,vtmp2,vl);
                    }
                }
                t1 = __cycle();
                _vel_vst_vssl(vtmp2,8,tmp2,vl);
            }
#elif TEST == VEC_DIVMOD_U
#endif
            use_value(tmp2,series_len);
#endif
            vfd21_npot_cyc += (double)(t1 - t0) / NUM_NUMS;
#endif // VE_INTRINSICS

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
#if VE_INTRINSICS
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
#if VE_INTRINSICS
        if(vfdiv_npot_cyc      !=0)
            printf("vfdiv_npot_cyc      : %.1f ns\n", to_ns_f(vfdiv_npot_cyc));
        if(vfd21_npot_cyc      !=0)
            printf("vfd21_npot_cyc      : %.1f ns\n", to_ns_f(vfd21_npot_cyc));
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
#if VE_INTRINSICS
    vcookie += _vel_vsuml_vvl(vcookie,256);
    cookie64 += _vel_lvsl_svs(vcookie,0);
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
#if 0
Results:
    VE Aurora (host aurora-ds02)

cyc2ns = 1.250000 __cycle=2975890480067434, __cycle=2975890480067438
clang++ -O3 ...                    nc++ -std=gnu++11 -O3
series_len = 256                              g++
----------------------------       --------   --------
builtin_npot_cyc    : 3654.4ns     3667.3ns   1789.5ns  for(){% operator}
fd21_npot_cyc       : 1454.1 ns    1454.1ns   538.7 ns  for(){multiply-shift}
vfdiv_npot_cyc      : 45.3 ns                           vbrd,vdiv,vmul,vsub
vfd21_npot_cyc      : 11.6 ns                           
branchless_npot_cyc : 1634.7 ns    1669.1ns   694.4 ns
series_len = 128
----------------------------
builtin_npot_cyc    : 1833.2ns
fd21_npot_cyc       : 720.9 ns
vfdiv_npot_cyc       : 23.2 ns
vfd21_npot_cyc       : 8.1 ns
branchless_npot_cyc : 815.1 ns
series_len = 64
----------------------------
builtin_npot_cyc    : 915.2ns
fd21_npot_cyc       : 351.3 ns
vfdiv_npot_cyc       : 18.6 ns
vfd21_npot_cyc       : 7.4 ns
branchless_npot_cyc : 401.9 ns
series_len = 32
----------------------------
builtin_npot_cyc    : 456.3ns
fd21_npot_cyc       : 166.5 ns
vfdiv_npot_cyc       : 17.0 ns
vfd21_npot_cyc       : 7.4 ns
branchless_npot_cyc : 195.4 ns
series_len = 16
----------------------------
builtin_npot_cyc    : 226.8ns
fd21_npot_cyc       : 70.9 ns
vfdiv_npot_cyc       : 16.4 ns
vfd21_npot_cyc       : 7.4 ns
branchless_npot_cyc : 89.2 ns
series_len = 8
----------------------------
builtin_npot_cyc    : 112.0ns
fd21_npot_cyc       : 34.9 ns
vfdiv_npot_cyc       : 16.0 ns
vfd21_npot_cyc       : 7.4 ns
branchless_npot_cyc : 42.2 ns
series_len = 4
----------------------------
builtin_npot_cyc    : 54.6ns
fd21_npot_cyc       : 17.5 ns
vfdiv_npot_cyc       : 15.8 ns
vfd21_npot_cyc       : 7.4 ns
branchless_npot_cyc : 24.3 ns
series_len = 2
----------------------------
builtin_npot_cyc    : 26.0ns
fd21_npot_cyc       : 10.2 ns
vfdiv_npot_cyc       : 15.8 ns
vfd21_npot_cyc       : 7.4 ns
branchless_npot_cyc : 13.1 ns
series_len = 1
----------------------------
builtin_npot_cyc    : 11.6ns
fd21_npot_cyc       : 6.2 ns
vfdiv_npot_cyc       : 15.8 ns
vfd21_npot_cyc       : 7.4 ns
branchless_npot_cyc : 8.5 ns
cookie=7064013899
#endif
#if 0
Operations: VL=256
(1) C-loop tmp2[] += ni%den[]       TEST==U_MOD_VEC
(2) C-loop tmp2[] += num[]%d        TEST==VEC_MOD_U
VEC%VEC also works
Impls:
a. C code '%'
b. C code fastdiv21
c. clang _vel_vdiv
d. clang _vel_vmul+vsrl
Timings (ns)
        clang   ncc     gcc
        ------  ------  ------
1a.  |  2925.9  1519.5   561.2
1b.  |  3532.7  1401.5   179.7
1c.  |    45.1      x
1d.  |     8.5      x
2a.  |  2926.4   1544.1  561.4 
2b.  |   535.2    440.1   48.4
2c.  |    45.1       x
2d.  |     5.6       x
run on aurora-ds02
    clang and ncc on VE
    gcc on x86
#endif
// vim: ts=4 sw=4 et cindent cino=^=l0,\:.5s,=-.5s,N-s,g.5s,b1 cinkeys=0{,0},0),\:,0#,!^F,o,O,e,0=break
