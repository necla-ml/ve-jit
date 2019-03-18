/* Copyright (c) 2019 by NEC Corporation
 * This file is part of ve-jit */

#include <cassert>
#include <iostream>
#include <algorithm>
#include <vector>
#define AL8 __attribute__((aligned(8)))
using namespace std;
void msk512_copy_cd( void* a, void* b ){
    for(int i=0; i<256; ++i){
        ((uint64_t*)b)[i] = ((uint64_t*)a)[i];
    }
}
int main(int, char**){
    float a[512] AL8, b[512] AL8;
    try{
        for(unsigned i=0U; i<512U; ++i){ a[i]=1000+i; b[i]=0.f; }
        cout<<"Begin test"<<endl; cout.flush();
        for(int i=0; i<10; ++i){
            for(unsigned z=0U; z<512U; ++z) b[z]=0.f;
            cout<<" a[512] @ "<<(void*)&a[0]
                <<" b[512] @ "<<(void*)&b[0]<<endl;
            if(1){
                void *pa=&a[0], *pb=&b[0];
                msk512_copy_cd(pa,pb);
            }
            if(1){
                std::vector<int> nz;
                asm("fencei":::"memory"); // find me in asm output
                // error in following, leads to SIGBUS at -O2
                for(int j=0; j<512; ++j){
                    if(b[j] != 0.f) nz.push_back(j);
                }
                cout<<" set a["<<i<<"] ==> change in output vector @ ";
                if(nz.empty()) cout<<"None?"<<endl;
                else for(auto j: nz) cout<<" "<<j;
                cout<<endl;
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
