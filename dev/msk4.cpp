/* Copyright (c) 2019 by NEC Corporation
 * This file is part of ve-jit */

#include <cassert>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <vector>
using namespace std;

#define AL8 __attribute__((aligned(8)))
#define STR0(s...) #s
#define STR(s...) STR0(s)
#define ASM(A,X...) asm(A X)
#define s_addr_c(S,CVP)     ASM(STR(or S,0,%0)          ,::"r"(CVP):STR(S))
#define lvl(N)              ASM(STR(lea %s13,N\n\t) \
        /*                   */ STR(lvl %s13)           ,:::"%s13")
/** load 4*64 bits at Saddr into VMa, and the next 4*64 bits into VMb.
 * For pack vector masks, VMa should be an even numbered Vector Mask register,
 * and VMb should be the next higher one. */
#define vm_ld2(VMa,VMb,Saddr) ASM(STR(ld %s12, 0(,Saddr)\n\t) STR(lvm VMa,0,%s12\n\t) \
        /*                     */ STR(ld %s13, 8(,Saddr)\n\t) STR(lvm VMa,1,%s13\n\t) \
        /*                     */ STR(ld %s12,16(,Saddr)\n\t) STR(lvm VMa,2,%s12\n\t) \
        /*                     */ STR(ld %s13,24(,Saddr)\n\t) STR(lvm VMa,3,%s13\n\t) \
        /*                     */ STR(ld %s12,32(,Saddr)\n\t) STR(lvm VMb,0,%s12\n\t) \
        /*                     */ STR(ld %s13,40(,Saddr)\n\t) STR(lvm VMb,1,%s13\n\t) \
        /*                     */ STR(ld %s12,48(,Saddr)\n\t) STR(lvm VMb,2,%s12\n\t) \
        /*                     */ STR(ld %s13,56(,Saddr)\n\t) STR(lvm VMb,3,%s13\n\t) \
        /*                     */ ,:::STR(%s12),STR(%s13),STR(VMa),STR(VMb))
#define vm_st2(VMa,VMb,Saddr) ASM( \
        STR(svm %s12,VMa,0\n\t) STR(st %s12, 0(,Saddr)\n\t) \
        STR(svm %s13,VMa,1\n\t) STR(st %s13, 8(,Saddr)\n\t) \
        STR(svm %s18,VMa,2\n\t) STR(st %s18,16(,Saddr)\n\t) \
        STR(svm %s19,VMa,3\n\t) STR(st %s19,24(,Saddr)\n\t) \
        STR(svm %s12,VMb,0\n\t) STR(st %s12,32(,Saddr)\n\t) \
        STR(svm %s13,VMb,1\n\t) STR(st %s13,40(,Saddr)\n\t) \
        STR(svm %s18,VMb,2\n\t) STR(st %s18,48(,Saddr)\n\t) \
        STR(svm %s19,VMb,3\n\t) STR(st %s19,56(,Saddr)\n\t) \
        ,:::"%s12","%s13","%s18","%s19","memory")
#define pv_ld(V,S)          ASM(STR(vld V,8,S)         ,:::STR(V)) // 8 ~ strided Sy==1
#define pv_st(V,S)          ASM(STR(vst V,8,S)         ,:::"memory")
#define pvor_msk(Vdst,Vsrc,VM) ASM(STR(pvor Vdst,(0)1,Vsrc,VM) ,:::STR(Vdst))

void msk512_copy(void* a, void* b, void* c, void* d,
        void const* msk512a, void const* msk512b, void const* msk512c){
    lvl(256);
    s_addr_c(%s20,a);
    s_addr_c(%s21,b);
    s_addr_c(%s22,c);
    s_addr_c(%s23,d);
    s_addr_c(%s24,msk512a);
    s_addr_c(%s25,msk512b);
    s_addr_c(%s26,msk512c);
    pv_ld(%v0,%s20);
    pv_ld(%v1,%s21);
    pv_ld(%v2,%s22);
    pv_ld(%v3,%s23);

    vm_ld2(%vm2,%vm3, %s24);
    vm_ld2(%vm4,%vm5, %s25);
    vm_ld2(%vm6,%vm7, %s26);

    pvor_msk(%v1,%v0,%vm2); pv_st(%v1,%s21);
    pvor_msk(%v2,%v0,%vm4); pv_st(%v2,%s22);
    pvor_msk(%v3,%v0,%vm6); pv_st(%v3,%s23);
}

void msk512_copy_ab( void const* a, void* b, void const* msk512data ){
    lvl(256);
    s_addr_c(%s20,a);
    s_addr_c(%s21,b);
    s_addr_c(%s22, msk512data);
    pv_ld(%v0,%s20);
    pv_ld(%v1,%s21);
    vm_ld2(%vm2,%vm3, %s22);
    asm("pvor %v1, (0)1,%v0, %vm2");
    pv_st(%v1,%s21);
}
int/*err status*/ cpy512_all(){
    try{
        float a[512] AL8, b[512] AL8;
        for(unsigned i=0U; i<512U; ++i){ a[i]=1000+i; b[i]=0.0f; }
        uint64_t m512_allset[8] = {~0UL,~0UL,~0UL,~0UL, ~0UL,~0UL,~0UL,~0UL};
        cout<<" cpy512_all ... "; cout.flush();
        msk512_copy_ab( &a[0], &b[0], &m512_allset[0] );
        if(1){
            uint64_t msk_cpy[8]={0,0,0,0, 0,0,0,0};
            s_addr_c(%s22,&msk_cpy[0]);
            vm_st2(%vm2,%vm3,%s22);
            cout<<" %vm2,%vm3 --> msk_cpy[8]="<<hex;
            for(int i=0; i<8; ++i) cout<<(i?',':'{')<<"0x"<<msk_cpy[i];
            cout<<"}"<<endl<<dec;
        }
        for(unsigned z=0U; z<512U; ++z){
            assert( b[z] !=  0.f );
            assert( b[z] == a[z] );
        }
        cout<<" cpy512_all ... OK!"<<endl;
    }catch(...){
        cout<<"\nFailed! Goodbye!"<<endl;
        return 1;
    }
    cout<<"\nOk! Goodbye!"<<endl;
	return 0;
}


void cpy512_123(){   
    float a[512] AL8, b[512] AL8, c[512] AL8, d[512] AL8;
    for(unsigned i=0U; i<512U; ++i){
        a[i]=1000+i; b[i]=0.0f; c[i]=0.0f; d[i]=0.0f;
    }
    uint64_t m1[8] = {0UL,0UL,0UL,0UL, 1UL<<63,0UL,0UL,0UL}; // only a[0]
    uint64_t m2[8] = {1UL<<63,0UL,0UL,0UL, 0UL,0UL,0UL,0UL}; // just a[1]
    uint64_t m3[8] = {0UL,0UL,0UL,0UL, 1UL<<62,0UL,0UL,0UL}; // just a[2]

    msk512_copy(&a[0],&b[0],&c[0],&d[0], &m1[0],&m2[0],&m3[0]);

    if(1){
        uint64_t msk_cpy[8]={0,0,0,0, 0,0,0,0};
        s_addr_c(%s22,&msk_cpy[0]);
        vm_st2(%vm2,%vm3,%s22);
        cout<<" %vm2,%vm3 --> msk_cpy[8]="<<hex;
        for(int i=0; i<8; ++i) cout<<(i?',':'{')<<"0x"<<msk_cpy[i];
        cout<<"}"<<endl<<dec;
    }
    for(unsigned i=0U; i<10U; ++i)
        cout<<"i="<<setw(4)<<i<<setw(5)<<a[i]<<setw(5)
            <<b[i]<<setw(5)<<c[i]<<setw(5)<<d[i]<<endl;
    for(unsigned i=250U; i<260U; ++i)
        cout<<"i="<<setw(4)<<i<<setw(5)<<a[i]<<setw(5)
            <<b[i]<<setw(5)<<c[i]<<setw(5)<<d[i]<<endl;
    for(unsigned i=508U; i<512U; ++i)
        cout<<"i="<<setw(4)<<i<<setw(5)<<a[i]<<setw(5)
            <<b[i]<<setw(5)<<c[i]<<setw(5)<<d[i]<<endl;
    unsigned nzb=999U, nzc=999U, nzd=999U;
    cout<<" b[i] nonzero for i=";
    for(unsigned i=0U; i<512U; ++i) if( b[i] != 0.f){ cout<<" "<<i; nzb=i; }
    cout<<"\n c[i] nonzero for i=";
    for(unsigned i=0U; i<512U; ++i) if( c[i] != 0.f){ cout<<" "<<i; nzc=i; }
    cout<<"\n d[i] nonzero for i=";
    for(unsigned i=0U; i<512U; ++i) if( d[i] != 0.f){ cout<<" "<<i; nzd=i; }
    cout<<endl;
    assert( nzb == 0U ); assert( b[0] == 1000+0 );
    assert( nzc == 1U ); assert( c[1] == 1000+1 );
    assert( nzd == 2U ); assert( d[2] == 1000+2 );
}
int main(int, char**){
    cpy512_all();
    cpy512_123();
    return 0;
}
/* vim: set sw=4 ts=4 et: */
