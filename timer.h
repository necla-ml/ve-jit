#ifndef __TIMER_H__
#define __TIMER_H__

#include <sys/time.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>
#include <unistd.h> //sleep
#include <stdio.h> //fprintf

#ifdef __cplusplus
extern "C" { //}
#endif

#if defined(__ve__) && defined(__clang__)
// clang++ supports extended asm (WOW!)
    inline unsigned long long
__cycle()
{
    void *vehva = (void *)0x1000;
    /*register*/ unsigned long long val; // 'register' incompatible with C++17
    asm volatile ("lhm.l %0,0(%1)":"=r"(val):"r"(vehva));
    return val;
}
#elif defined(__ve__) // not clang, assume ncc/nc++ ...
    inline unsigned long long
__cycle()
{
    // nc++ MUST to compile with -std=gnu++11
    // ncc works as is (__GNUC_STDC_INLINE__ is default)
#if defined(__cplusplus) && ! (defined(__GNUC__STDC_INLINE__) && __GNUC__STDC_INLINE__)
    // nc++ by default allows only the most basic inline asm syntax
    asm("lea %s0,0x1000");
    asm("lhm.l %s0,0(%s0)");
    return;
#else
    // ncc seems always to support both volatile and extended asm syntax
    // nc++ requires compilation with -std=gnu++XX for extended asm.
    void *vehva = (void *)0x1000;
    //#if defined(__cplusplus) && __cplusplus >= 201402L /*nc++ -std=c++17, no STDC_INLINE, already handled*/
#if defined(__cplusplus) && __cplusplus >= 201500L   /*nc++ -std=gnu++17*/
    unsigned long long val; // 'register' incompatible with C++17
#else
    register unsigned long long val; // C++17 should not use 'register' hint, still ok for ncc?
#endif
    asm volatile ("lhm.l %0,0(%1)":"=r"(val):"r"(vehva));
    return val;
#endif
}

#elif defined(__x86_64__)

inline unsigned long long __cycle()
{
    // simplified, not for critical use (no pipeline flush w/ acpi)
    unsigned hi, lo;
    __asm__ __volatile__ ("rdtsc" : "=a"(lo), "=d"(hi));
    return ( (unsigned long long)lo)|( ((unsigned long long)hi)<<32 );
}

#elif defined(__i386__)

inline unsigned long long __cycle()
{
    // simplified, not for critical use (no pipeline flush w/ acpi)
    unsigned long long ret;
    __asm__ __volatile__("rdtsc" : "=A" (ret));
    return ret;
}

#else
#error "Must be define the __cycle function which get processor cycles, here."
#endif

inline double __clock()
{
    struct timeval tv;
    int rv;

    rv = gettimeofday(&tv, NULL);
    if (rv) {
        fprintf(stderr, "gettimeofday() returned with %d\n", rv);
        exit(1);
    }

    return tv.tv_sec + tv.tv_usec * 1.0e-6; // return "seconds"
}

inline double cycle2ns(){
    struct timespec ts = {1, 100000000}; //100 ms
    struct timespec rem;
    errno = 0;
    double s0 = __clock();
    unsigned long long cyc0 = __cycle();
    sleep(1);
    for(int i=0; i<10; ++i){
        if( nanosleep(&ts, &rem) == 0 ) break;
        if( errno == EINTR ){
            ts.tv_sec = rem.tv_sec;
            ts.tv_nsec = rem.tv_nsec;
            errno = 0;
        }
    }
    double s1 = __clock();
    unsigned long long cyc1 = __cycle();
    return (s1-s0)*1.e9 / (cyc1-cyc0);
}


#ifdef __cplusplus
}// extern "C"
#endif
// vim: ts=4 sw=4 et cindent cino=^=l0,\:.5s,=-.5s,N-s,g.5s,b1 cinkeys=0{,0},0),\:,0#,!^F,o,O,e,0=break
#endif
