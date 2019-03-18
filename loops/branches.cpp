/* Copyright (c) 2019 by NEC Corporation
 * This file is part of ve-jit */
#include <cstdlib>
#include <iostream>
using namespace std;
int main(int,char**){
    int i=std::rand();
    if( i == 0 ) goto izero;
    cout<<" i nonzero";
    goto done;
izero:
    cout<<" i zero";
done:
    if( i > 5 ) goto igt5;
    cout<<" !(i>5)";
    goto done2;
igt5:
    cout<<" i>5";
done2:
    cout<<endl;
}

