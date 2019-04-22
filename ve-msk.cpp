#include "ve-msk.hpp"
#include <iostream>

using namespace std;

ostream& operator<<(ostream&os, Msk256 const& m256){
    for(int i=0; i<256; ++i){
        int s=i/64, r=63-i%64;
        os<<(s>0 && r==63?" ":"")
            <<((m256.m[s] & (uint64_t{1}<<r))? '1':'0'); }
    return os;
}

ostream& operator<<(ostream&os, Msk512 const& m512){
#if 1
    os<<"\na="<<std::hex;
    for(int i=0;i<4;++i) os<<(i==0?'{':',')<<"0x"<<m512.a.m[i];
    os<<"}\nb=";
    for(int i=0;i<4;++i) os<<(i==0?'{':',')<<"0x"<<m512.b.m[i];
    os<<'}'<<std::dec;
    os<<"\na: "<<m512.a;
    os<<"\nb: "<<m512.b;
    for(int x=0; x<512; ++x){
        if(x%64==0) os<<"\n "<<x/64<<":";
        os<<(m512.get(x)? '1':'0');
    }
#else
    for(int x=0; x<256; ++x){
        int s=x/64, r=x%64;
        os<<((m512.a.m[s] & (uint64_t{1}<<r))? '1':'0')
            <<((m512.b.m[s] & (uint64_t{1}<<r))? '1':'0');
    }
#endif
    return os;
}
/* vim: set sw=4 ts=4 et: */
