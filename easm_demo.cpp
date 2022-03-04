#include <iostream>
#include <cstdint>
#include <cassert>

#include "asmfmt_fwd.hpp"

using namespace std;

int main(int argc, char**argv){
    uint64_t u;
    // nc++ easm_demo.cpp && ./a.out
    // clang++ -std=gnu++14 -target linux-ve -O3 -mllvm -show-spill-message-vec -fno-vectorize -fno-unroll-loops -fno-slp-vectorize -fno-crash-diagnostics easm_demo.cpp -O0 -Wall && ./a.out
    asm(    "xor %0,%0,%0\n\t"
            "addu.l %0,1,%0\n\t"
            :"=r"(u) : : );
    assert(u==1);
    uint64_t v=77U;
    asm(    "addu.l %0,%1,%1\n\t"
            :"=r"(v)  : "0r"(u):  );
    assert(v==2);
    asm(    "addu.l %0,%0,%0\n\t"
            :"+r"(v) :"r"(v) : );
    assert(v==4);
    asm(    "or %0,%1,%2\n\t"
            "vxor  %%v0,%%v0,%%v0\n\t"
            :"+r"(u) :"%r"(0x5500), "r"(v) : );
    assert(u==0x5504);
    // vector registers do not seems to be supported.
    cout<<"\nGoodbye"<<endl;
}
// vim: ts=4 sw=4 et cindent cino=^=l0,\:.5s,=-.5s,N-s,g.5s,b1 cinkeys=0{,0},0),\:,0#,!^F,o,O,e,0=break
