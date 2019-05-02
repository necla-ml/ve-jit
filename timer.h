#ifndef __TIMER_H__
#define __TIMER_H__

#include <sys/time.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>
#include <unistd.h> //sleep

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __ve__

	inline unsigned long long
		__cycle()
        {
			// Aurora-as-manual-v3.2 states extended asm is available for C/C++,
			// but I find nc++ does not support extended asm.	
			//    - still true for nc++-2.1.24
			// AHAA -- you need to compile with -std=gnu++11
#if 0 && defined(__cplusplus)
			// nc++-2.1.24
			// 	does not support [__]volatile[__]
			// 	does not support extended asm
			asm("lhm.l %s0,0x1000");
			return;
#else
			// ncc supports both volatile and extended asm syntax
			void *vehva = (void *)0x1000;
			unsigned long long val; // 'register' incompatible with C++17
			asm volatile ("lhm.l %0,0(%1)":"=r"(val):"r"(vehva));
			return val;
#endif
		}

#else

#if defined(__x86_64__)

	inline unsigned long long __cycle()
	{
		unsigned hi, lo;
		__asm__ __volatile__ ("rdtsc" : "=a"(lo), "=d"(hi));
		return ( (unsigned long long)lo)|( ((unsigned long long)hi)<<32 );
	}

#elif defined(__i386__)

	inline unsigned long long __cycle()
	{
		unsigned long long ret;
		__asm__ __volatile__("rdtsc" : "=A" (ret));
		return ret;
	}

#else

#error "Must be define the function which get processor cycles, here."

#endif
#endif

	inline double
		__clock()
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
