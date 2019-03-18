/* Copyright (c) 2019 by NEC Corporation
 * This file is part of ve-jit */
#include <cstdint>
#include <iostream>
#include <iomanip>
using namespace std;
int main(int,char**){
    // cyclic 3-counter, 2 registers
    for(uint32_t i=0, j=0, k=2; i<10; ++i){
        if(i){
            uint32_t x = j+k;
            uint32_t y = j+1;
            k = j;
            j = ~x & y;
        }
        cout<<"i,j="<<setw(4)<<i<<" "<<setw(4)<<j<<endl;
    }
    cout<<"\nGoodbye"<<endl;
    return 0;
}
