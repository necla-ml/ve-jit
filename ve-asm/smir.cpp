/* Copyright (c) 2019 by NEC Corporation
 * This file is part of ve-jit */
#include "xtimec.h"         // _Xtime_get_ticks,
#include <unistd.h>     // sleep
#include <time.h>       // clock()
//#define _XTIME_NSECS_PER_TICK   1000
///#define _XTIME_TICKS_PER_TIME_T (long long)1000000
#include <iostream>
#include <cstdint>
using namespace std;
int main(int,char**)
{
    //cout<<" CLOCKS_PER_SEC = "<<CLOCKS_PER_SECOND<<endl;
    cout<<" clock() "<<clock()<<endl;
    cout<<" xtime : "<<_Xtime_get_ticks()<<endl;
    // OH. _Xtime_get_ticks() just massages gettimeofday
    //     and is in libnc++.a :(
    for(int i=0; i<3; ++i){
        register long long t;
        asm volatile (
                //"smir %[usec],0\n"
                "smir %[usec],%%usrcc\n"
                : [usec]"=r"(t)	// outputs
                :    	                    // inputs
                : "memory"                  // clobbers
                // ^^^^^^ try to prevent compiler code movement
                );
        cout<<"smir t,0 :"<<t<<endl;
    }
    cout<<" 3 more x, with sleep(1) after each"<<endl;
    for(int i=0; i<3; ++i){
        //register long long t;
        uint64_t t;
        float f;
        asm volatile (
                "smir %[usec],0\n"
                : [usec]"=r"(t)	// outputs
                :    	                    // inputs
                : "memory"                  // clobbers
                // ^^^^^^ try to prevent compiler code movement
                );
        asm volatile ( "smir %[usec],0\n" :[usec]"=r"(f) : :"memory" );
        cout<<"smir t,0 :"<<t<<"\tf : "<<f<<endl;
        sleep(1);
    }
    /*
     * 0000000000000000 <__clock>:
     *    0:   00 00 00 00     smir    %s63,0
     *    4:   00 00 3f 22 
     *    8:   00 00 00 00     lea     %s61,0x0(0,0)
     *    c:   00 00 3d 06 
     *   10:   00 00 00 00     and     %s61,%s61,(32)0
     *   14:   60 bd 3d 44 
     *   18:   00 00 00 00     lea.sl  %s61,0x0(0,%s61)
     *   1c:   bd 00 bd 06 
     *   20:   00 00 00 00     ld      %s61,0x0(0,%s61)
     *   24:   bd 00 3d 01 
     *   28:   00 00 00 00     subu.l  %s63,%s63,%s61
     *   2c:   bd bf 3f 58 
     *   30:   00 00 00 00     lea     %s60,0x0(0,0)
     *   34:   00 00 3c 06 
     *   38:   00 00 00 00     and     %s60,%s60,(32)0
     *   3c:   60 bc 3c 44 
     *   40:   00 00 00 00     lea.sl  %s60,0x0(0,%s60)
     *   44:   bc 00 bc 06 
     *   48:   00 00 00 00     ld      %s60,0x0(0,%s60)
     *   4c:   bc 00 3c 01 
     *   50:   00 00 00 00     cvt.d.l %s63,%s63
     *   54:   00 bf 3f 5f 
     *   58:   00 00 00 00     fmul.d  %s0,%s63,%s60
     *   5c:   bc bf 00 4d 
     *   60:   00 00 00 00     b.l     0x0(,%s10)
     *   64:   8a 00 0f 19 
     */
#if 1
    uint64_t huh;
    asm( "lea %[xx],0\n"                    // 0
           "and %[xx],%[xx],(32)0\n"        // 0
           "lea.sl %[xx],0x0(0,%[xx])\n"    // 0
           //"ld %[xx],0x0(0,%[xx])\n"  // segfault
           :[xx]"=r"(huh)
           :
           :"%s61","memory");
    cout<<" mystery value "<<(void*)huh<<endl;
#endif
    cout<<" clock() "<<clock()<<endl;
    cout<<" xtime : "<<_Xtime_get_ticks()<<endl;
    return 0;
}
#if 0
=== output ===
nc++: vec( 103): smir.cpp, line 9: Unvectorized loop.
nc++: vec( 103): smir.cpp, line 20: Unvectorized loop.
 xtime : 1526654038576354
smir t,0 :1381870
smir t,0 :1442109
smir t,0 :1502510
smir t,0 :1562313
smir t,0 :1626961
smir t,0 :1694510
 xtime : 1526654041578924
--- conclude ---
smir is process time (~60000 increments even with sleep(1) injected)
_Xtime_get_ticks is wall clock ticks
#endif
// vim: ts=4 sw=4 et cindent cino=^=l0,\:.5s,=-.5s,N-s,g.5s,b1 cinkeys=0{,0},0),\:,0#,!^F,o,O,e,0=break
