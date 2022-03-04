
#define _POSIX_C_SOURCE 200809L

#include <stdint.h>

#include <stdio.h>

#include "../timer.h"

static double cyc2ns = 1.0; // really cycle2ns()
static inline uint64_t clock_ns() {
    return (uint64_t)(__cycle()*cyc2ns+0.5);
}
static inline unsigned long long to_ns(double avg_cyc){
    return (unsigned long long)( avg_cyc * cyc2ns );
}
static inline double to_ns_f(double avg_cyc){
    return ( avg_cyc * cyc2ns );
}
static uint64_t bogus=0;
uint64_t timeit_foo( int const reps ){ // fn_foo
  static uint64_t state=12345ULL; // minimal PCG32 impl, inc=rep|1
  uint64_t t=0;
  for(uint64_t rep=0; rep<(uint64_t)reps; ++rep){ // tloop
    uint64_t oldstate = state;
    state = state * 6364136223846793005ULL + ((uint64_t)rep|1ULL);
    oldstate =  oldstate^(oldstate >> 18u);
    uint64_t const rot = oldstate>>59u;
    uint64_t const seed = (oldstate>>rot) | (oldstate<<(64-rot)); // seed ~ a PCG random sequence
    uint64_t const t0 = __cycle(); // NOT careful. e.g. no cpuid op HERE for x86
    // --- kernel foo ----------------------------------------
    uint64_t fooOut = seed;
    for(int j=0; j<1000; ++j) fooOut ^= fooOut*23456789ULL+j;
    // --- kernel foo ----------------------------------------
    uint64_t const t1 = __cycle();
    t += t1 - t0;
    bogus ^= fooOut; // somehow update bogus using fooOut [,seed,...]
  } //tloop
  return (double)t / reps ; // return avg cycle count of 1 'kern'
} //fn_foo
int main(int argc,char**argv){ // main
  cyc2ns = cycle2ns();
  printf(" cyc2ns = %f\n", cyc2ns);
  if(1){ // callfoo
    int const foo_reps = 1000;
    double foo_cycles = timeit_foo( foo_reps );
    uint64_t foo_ns = to_ns(foo_cycles);
    printf(" %s{%llu reps avg %llu cycles = %llu ns}\n", "foo",
            (unsigned long long)foo_reps,
            (unsigned long long)foo_cycles,
            (unsigned long long)foo_ns);
  } //callfoo
  uint64_t const nrep = 1000;
  printf("bogus=%llu\n", (unsigned long long)bogus);
} //main

