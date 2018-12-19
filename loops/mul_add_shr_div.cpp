
/* \file
 * see https://stackoverflow.com/questions/11040646/faster-modulus-in-c-c
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

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>

typedef int32_t s32;
typedef uint32_t u32;
typedef uint64_t u64;

#define NUM_NUMS 1024
#define NUM_RUNS 500
#define MAX_NUM UINT32_MAX
#define MAX_DEN 1024

struct fastdiv {
    u32 mul;
    u32 add;
    s32 shift;
    u32 _odiv;  /* save original divisor for modulo calc */
};

static u32 num[NUM_NUMS];
static u32 den[NUM_NUMS];
static struct fastdiv fd[NUM_NUMS];

/* hash of results to prevent gcc from optimizing out our ops */
static u32 cookie = 0;

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
}

/* 0: use function that checks for a power-of-2 modulus (speedup for POTs)
 * 1: use inline macro */
#define FASTMOD_BRANCHLESS 0

#define fastdiv(v,d) ((u32)(((u64)(v)*(d)->mul + (d)->add) >> 32) >> (d)->shift)
#define _fastmod(v,d) ((v) - fastdiv((v),(d)) * (d)->_odiv)

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
    return arc4random_uniform(upper_bound);
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
    }
}

void fill_arrays_pot() {
    u32 log_bound, rand_log;
    int i;

    log_bound = ulog2(MAX_DEN);
    for (i = 0; i < NUM_NUMS; ++i) {
        num[i] = random32_range(MAX_DEN, MAX_NUM);
        rand_log = random32(log_bound) + 1;
        den[i] = 1 << rand_log;
        fastdiv_make(&fd[i], den[i]);
    }
}

u64 clock_ns() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec*1000000000 + tv.tv_usec*1000;
}

void use_value(u32 v) {
    cookie += v;
}

int main(int argc, char **arg) {
    u64 builtin_npot_ns;
    u64 builtin_pot_ns;
    u64 branching_npot_ns;
    u64 branching_pot_ns;
    u64 branchless_npot_ns;
    u64 branchless_pot_ns;
    u64 t0, t1;
    u32 v;
    int s, r, i, j;
    int series_len;

    builtin_npot_ns = builtin_pot_ns = 0;
    branching_npot_ns = branching_pot_ns = 0;
    branchless_npot_ns = branchless_pot_ns = 0;

    for (s = 7; s >= 0; --s) {
        series_len = 1 << s;
        for (r = 0; r < NUM_RUNS; ++r) {
            /* built-in NPOT */
            fill_arrays();
            t0 = clock_ns();
            for (i = 0; i < NUM_NUMS; ++i) {
                v = num[i];
                for (j = 0; j < series_len; ++j) {
                    v /= den[i];
                }
                use_value(v);
            }
            t1 = clock_ns();
            builtin_npot_ns += (t1 - t0) / NUM_NUMS;

            /* built-in POT */
            fill_arrays_pot();
            t0 = clock_ns();
            for (i = 0; i < NUM_NUMS; ++i) {
                v = num[i];
                for (j = 0; j < series_len; ++j) {
                    v /= den[i];
                }
                use_value(v);
            }
            t1 = clock_ns();
            builtin_pot_ns += (t1 - t0) / NUM_NUMS;

            /* branching NPOT */
            fill_arrays();
            t0 = clock_ns();
            for (i = 0; i < NUM_NUMS; ++i) {
                v = num[i];
                for (j = 0; j < series_len; ++j) {
                    v = fastmod(v, fd+i);
                }
                use_value(v);
            }
            t1 = clock_ns();
            branching_npot_ns += (t1 - t0) / NUM_NUMS;

            /* branching POT */
            fill_arrays_pot();
            t0 = clock_ns();
            for (i = 0; i < NUM_NUMS; ++i) {
                v = num[i];
                for (j = 0; j < series_len; ++j) {
                    v = fastmod(v, fd+i);
                }
                use_value(v);
            }
            t1 = clock_ns();
            branching_pot_ns += (t1 - t0) / NUM_NUMS;

            /* branchless NPOT */
            fill_arrays();
            t0 = clock_ns();
            for (i = 0; i < NUM_NUMS; ++i) {
                v = num[i];
                for (j = 0; j < series_len; ++j) {
                    v = _fastmod(v, fd+i);
                }
                use_value(v);
            }
            t1 = clock_ns();
            branchless_npot_ns += (t1 - t0) / NUM_NUMS;

            /* branchless POT */
            fill_arrays_pot();
            t0 = clock_ns();
            for (i = 0; i < NUM_NUMS; ++i) {
                v = num[i];
                for (j = 0; j < series_len; ++j) {
                    v = _fastmod(v, fd+i);
                }
                use_value(v);
            }
            t1 = clock_ns();
            branchless_pot_ns += (t1 - t0) / NUM_NUMS;
        }

        builtin_npot_ns /= NUM_RUNS;
        builtin_pot_ns /= NUM_RUNS;
        branching_npot_ns /= NUM_RUNS;
        branching_pot_ns /= NUM_RUNS;
        branchless_npot_ns /= NUM_RUNS;
        branchless_pot_ns /= NUM_RUNS;

        printf("series_len = %d\n", series_len);
        printf("----------------------------\n");
        printf("builtin_npot_ns    : %llu ns\n", builtin_npot_ns);
        printf("builtin_pot_ns     : %llu ns\n", builtin_pot_ns);
        printf("branching_npot_ns  : %llu ns\n", branching_npot_ns);
        printf("branching_pot_ns   : %llu ns\n", branching_pot_ns);
        printf("branchless_npot_ns : %llu ns\n", branchless_npot_ns);
        printf("branchless_pot_ns  : %llu ns\n\n", branchless_pot_ns);
    }
    printf("cookie=%u\n", cookie);
}

#if 0
Results
Intel Core i5 (MacBookAir7,2), macOS 10.11.6, clang 8.0.0

series_len = 32
----------------------------
builtin_npot_ns    : 218 ns
builtin_pot_ns     : 225 ns
branching_npot_ns  : 115 ns
branching_pot_ns   : 42 ns
branchless_npot_ns : 110 ns
branchless_pot_ns  : 110 ns

series_len = 16
----------------------------
builtin_npot_ns    : 87 ns
builtin_pot_ns     : 89 ns
branching_npot_ns  : 47 ns
branching_pot_ns   : 19 ns
branchless_npot_ns : 45 ns
branchless_pot_ns  : 45 ns

series_len = 8
----------------------------
builtin_npot_ns    : 32 ns
builtin_pot_ns     : 34 ns
branching_npot_ns  : 18 ns
branching_pot_ns   : 10 ns
branchless_npot_ns : 17 ns
branchless_pot_ns  : 17 ns

series_len = 4
----------------------------
builtin_npot_ns    : 15 ns
builtin_pot_ns     : 16 ns
branching_npot_ns  : 8 ns
branching_pot_ns   : 3 ns
branchless_npot_ns : 7 ns
branchless_pot_ns  : 7 ns

series_len = 2
----------------------------
builtin_npot_ns    : 8 ns
builtin_pot_ns     : 7 ns
branching_npot_ns  : 4 ns
branching_pot_ns   : 2 ns
branchless_npot_ns : 2 ns
branchless_pot_ns  : 2 ns
#endif
