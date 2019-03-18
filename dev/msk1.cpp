/* Copyright (c) 2019 by NEC Corporation
 * This file is part of ve-jit */

#include <cassert>
#include <iostream>
#include <algorithm>
#include <vector>
#include <iomanip>
using namespace std;

#define AL8 __attribute__((aligned(8)))
#define STR0(s...) #s
#define STR(s...) STR0(s)
#define ASM(A,X...) asm(A X)
#define s_addr_c(S,CVP)     ASM(STR(or S,0,%0)          ,::"r"(CVP):STR(S))
#define lvl(N)              ASM(STR(lea %s13,N\n\t) \
        /*                   */ STR(lvl %s13)           ,:::"%s13")
#define pv_ld(V,S)          ASM(STR(vld V,8,S)          ,:::STR(V))/* 8~strided Sy==1 */
#define pv_st(V,S)          ASM(STR(vst V,8,S)          ,:::"memory")
void msk512_copy_ab( void* a, void* b ){
    lvl(256);
    s_addr_c(%s20,a);
    s_addr_c(%s21,b);
    pv_ld(%v0,%s20);
    pv_st(%v0,%s21);
}
int main(int, char**){
    float a[512] AL8, b[512] AL8;
    try{
        for(unsigned i=0U; i<512U; ++i){ a[i]=1000+i; b[i]=0.f; }
        cout<<"Begin bit influence test"<<endl; cout.flush();
        for(int i=0; i<10; ++i){
            for(unsigned z=0U; z<512U; ++z) b[z]=0.f;
            cout<<" a[512] @ "<<(void*)&a[0]
                <<" b[512] @ "<<(void*)&b[0]<<endl;
            if(1){
                void *pa=&a[0], *pb=&b[0];
                msk512_copy_ab(pa, pb );
            }
            if(1){ // if enabled, also fails at -O2
                std::vector<int> nz;
                for(int j=0; j<512; ++j)
                    if(b[j] != 0.f)
                        nz.push_back(j);
                cout<<" nz.size()="<<nz.size()<<endl;
            }
        }
    }catch(...){
        cout<<"\nFailed! Goodbye!"<<endl;
        return 1;
    }
    cout<<"\nOk! Goodbye!"<<endl;
    return 0;
}
/* vim: set sw=4 ts=4 et: */
