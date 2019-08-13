/* Copyright (c) 2019 by NEC Corporation
 * This file is part of ve-jit */
/** \file
 * test ve_fastdiv.h . ncc adds a fair amount of "overhead" ops as it
 * vectorizes the 3 division algs, so much that the 'bounded' actually
 * ends up using about the same ops as general fastdiv.   But both
 * fastdiv loops are still 4x faster than compiler's VDIV division :)
 */
#include "ve_fastdiv.h"
#include "timer.h"

#include <iostream>
#include <iomanip>
#include <cassert>

#if defined(FTRACE) && !defined(__ve) // FTRACE **only** for VE
#warning "ignoring attempt to use FTRACE with non-VE compiler (ftrace header may be missing)"
#undef FTRACE
#endif

#ifdef FTRACE
#include <ftrace.h>
#define FTRACE_BEG(...) ftrace_region_begin(__VA_ARGS__)
#define FTRACE_END(...) ftrace_region_end(__VA_ARGS__)
#define FTRACE_IF(...) __VA_ARGS__
#else
#define FTRACE_BEG(...) do{}while(0)
#define FTRACE_END(...) do{}while(0)
#define FTRACE_IF(...) do{}while(0)
#endif

#ifndef SPEED
/** SPEED 0 means assert correctness, SPEED 1 means timing run */
#define SPEED 0
#endif

using namespace std;
int main(int,char**){
    // divMax < 2^21-1 for FASTDIV21 alg (bounded divisor case)
    // numMax can be any uint32_t
#if SPEED
    // this code vectorizes, so can cover a bigger range.
    // (but we do not check correctness)
    uint64_t const numMax = 1<<24;
    uint64_t const divMax = 1<<16;
#else
    uint64_t const numMax = 1<<18;
    uint64_t const divMax = 1<<16;
#endif
    uint64_t bogus = 1234567U;
    uint64_t normal_cyc=0ULL;
    uint64_t general_cyc=0ULL;
    uint64_t bounded_cyc=0ULL;
    uint64_t c0, c1;

    //auto rd1 = [](double const x) {return (int64_t)(x*10.) / 10.;};
#define rd1(x) (((int64_t)(x*10.))/10.)

    for(uint64_t d=1U; d<divMax; ++d){
#if SPEED
        FTRACE_BEG("standard div");
        c0 = __cycle();
#if 1 // 64-bit VDIV
        for(uint64_t num=0; num<numMax; ++num){ // num/d~200 s
            bogus += num/d; // uses VDIV (vdivu)
        }
#else // 32-bit VDIV
        uint32_t const d32 = d;
        for(uint32_t num=0; num<numMax; ++num){ // num/d32 still 4x slower
            bogus += num/d32; // uses VDIV (vdivu.w)
        }
#endif
        normal_cyc += __cycle()-c0;
        FTRACE_END("standard div");
#endif

        struct ve_fastdiv fd;       // general method
        vednn_fastdiv( &fd, d );    // divide-by-d
        uint64_t const mul = fd.mul;
        uint64_t const add = fd.add;
        uint64_t const shift = fd.shift;

        struct ve_fastdiv fd2;      // bounded method optimization
        vednn_fastdiv_bounded( &fd2, d, divMax ); // divide-by-d
        uint64_t const mul2 = fd2.mul;
        uint64_t const add2 = fd2.add;
        uint64_t const shift2 = fd2.shift;
        assert(mul2==1U || add2==0U || shift2==0U); // <= 2 ops jit
        assert(add2==0U);
#if SPEED
        FTRACE_BEG("vednn_fastdiv");
        c0 = __cycle();
        for(uint64_t num=0; num<numMax; ++num){
            bogus += (num*mul + add) >> shift;
        }
        general_cyc += __cycle()-c0;
        FTRACE_END("vednn_fastdiv");

        FTRACE_BEG("vednn_fastdiv_bounded");
        c0 = __cycle();
        for(uint64_t num=0; num<numMax; ++num){
            bogus += (num*mul2) >> shift;
        }
        bounded_cyc += __cycle()-c0;
        FTRACE_END("vednn_fastdiv_bounded");
        if(d % (divMax/100U) == 0U || d+1U == divMax ){
            cout<<(d * 100 / divMax)<<"% done (d="<<setw(8)<<d<<") ";
            cout<<" cycles: normal "<<setw(17)<<normal_cyc
                <<" general "<<setw(17)<<general_cyc
                <<" bounded "<<setw(17)<<bounded_cyc
                <<endl;
        }
#else
        for(uint64_t num=0; num<numMax; ++num){
            uint64_t const normal = num/d;
            uint64_t const general = (num * mul + add) >> shift;
            uint64_t const bounded = (num * mul2     ) >> shift2;
            assert( general == normal );
            assert( bounded == normal );
        }
#endif
    }
    cout<<" Finished unsigned n/d tests for n < "<<numMax<<" and d < "<<divMax<<endl;
#if SPEED // final conversion to seconds
    double const cyc2s = cycle2ns() * 1.e-9;
    cout<<" time(s): "
        <<" normal  "<<setw(17)<<rd1(normal_cyc *cyc2s)
        <<" general "<<setw(17)<<rd1(general_cyc*cyc2s)
        <<" bounded "<<setw(17)<<rd1(bounded_cyc*cyc2s)
        <<endl;
#else
    cout<<" All divisions were correct"<<endl;
#endif
    cout<<"\nGoodbye: bogus="<<bogus<<endl;
}
// vim: ts=4 sw=4 et cindent cino=^=l0,\:.5s,=-.5s,N-s,g.5s,b1 cinkeys=0{,0},0),\:,0#,!^F,o,O,e,0=break
