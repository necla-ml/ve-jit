/* Copyright (c) 2019 by NEC Corporation
 * This file is part of ve-jit */
/** \file
 * loop blocking strategy for Aurora.
 * Initial experiment.
 *
 * cjitConvFwd1..6 investigate just loop fusion
 * (and not even fully generic loop limits).
 *
 * Here generate a "blocking plan"
 * Usual blocking is none, vl = ow
 * Then owU128 impls block the y-loop:
 *   nY = MVL/ow;  vl = nY*ow;
 * But actually any nY <= min(MLV/ow, oh) is usable (with difft vl)
 *
 * Transform:
 *   for(0..ii) for(0..jj) for(0..kk)
 * into 6 loops (3 outer, 3 inner):
 * 	for(iOH=0..OH by nOH) for(iOW=0..OW by nOW) for(iOC=0..nOC by nOC){
 * 	  for(oh=iOH..iOH+nOH) for(ow=iOW..iOW+nOW) for(oc=iOC..iOC+nOC){
 * and then SIMD on the inner 3 loops.
 *
 * We mostly try for easy cases, where vector length is 1 (scalar)
 * or has a simple relation to the inner loop lengths.
 *
 * - Crude cost model;
 *   1. low inner kernel execution count;
 *   1. heuristic penalties for wasted ops, many/bad divisibility
 */
#include <iostream>
#include "../stringutil.hpp"
#include "../fuseloop.hpp"
using namespace std;
using namespace loop;
template<typename T> inline constexpr T divup(T const n, T const d) {return (n+d-1)/d;}
/** Split 3 loops into 3 outer + 3 inner loops by \em blocking.
 */
struct BlockingPlan{
    int ii, jj, kk;
    int iB, jB, kB;
    int iVl, jVl, kVl;      // these increase (bounded by MVL)
    int iLpo, jLpo, kLpo;
    int iLp, jLp, kLp;
    float iCost, jCost, kCost;
    int ijkOuter() const {return iLpo*jLpo*kLpo;}
    int ijkInner() const {return iLp*jLp*kLp;}
    int kerns() const {return ijkOuter() * ijkInner();}
    double ijkCost() const {return iCost+jCost+kCost;}
    // per-loop simd lengths
    int iv() const {return iVl;}
    int jv() const {return jVl/iVl;}
    int kv() const {return kVl/jVl;}
    void prt(){
        cout<<"// "
            <<iLpo<<"*To"<<ii<<","<<iB<<":"<<iLp<<"@vl"<<iVl<<" "
            <<jLpo<<"*To"<<jj<<","<<jB<<":"<<jLp<<"@vl"<<jVl<<" "
            <<kLpo<<"*To"<<kk<<","<<kB<<":"<<kLp<<"@vl"<<kVl<<" "
            <<" {o"<<ijkOuter()<<"*i"<<ijkInner()<<" "<<kerns()<<" kerns"
            <<" $"<<iCost<<"+$"<<jCost<<"+$"<<kCost
            <<" =$"<<ijkCost()<<"}"<<endl;
    }
};
// Need a better cost model.
//   
int main(int argc, char**argv){
    ostringstream oss;
    int ii=20, jj=3, kk=96;
    int a=0, opt_h=0;
    if(argc > 1){
        if (argv[1][0]=='-'){
            char *c = &argv[1][1];
            if (*c=='h'){
                cout<<" genBlock II JJ KK"<<endl;
                cout<<"   MVL = max vector length = 256 (fixed)"<<endl;
                cout<<"   II    = 1st loop a=0..II-1"<<endl;
                cout<<"   JJ    = 2nd loop b=0..JJ-1"<<endl;
                cout<<"   KK    = 3rd loop c=0..KK-1"<<endl;
                cout<<" double loop --> loop over vector registers a[VLEN], b[VLEN]"<<endl;
                cout<<" A,B,C  = calc lin comb Aa + Bb + C"<<endl;
                opt_h = 1;
            }
            ++a; // one extra arg was used up
        }
    }
    if(opt_h) return 0;

    if(argc > a+1) ii = atof(argv[a+1]);
    if(argc > a+2) jj = atof(argv[a+2]);
    if(argc > a+3) kk = atof(argv[a+3]);
    cout<<" genBlock ii="<<ii<<", jj="<<jj<<", kk="<<kk<<endl;
    int minInnerKernels = (1<<20);
#define MVL 256
    // Initial Loop:
    //      for(i=0..ii by 1)
    // --> Outer Loop: always non-simd
    // for(io=0..ii by iB) :            iLoopo = (ii+iB-1)/iB;
    //                                  iB=iVl/ii<MVL/ii
    // and Inner Loop:
    // for(i=io..min(io+iB,ii) by 1) :  iVl=1 or iB;
    //                                  iLoopi = (iB+iVl-1)/iVl
    //
    // iB in [1..ii] such that iB*ii <= MVL, or iB <= MVL/ii
    int const vv=0; // verbose looping summary?
    int const v=1;  // verbose inner summary?
    int const iBmax = max(1,min(MVL/ii, ii));
    int vl1=1;
    cout<<" iBmax ~ MVL/ii = "<<MVL/ii<<endl;
    float iCost=0.f, jCost=0.f, kCost=0.f;
    vector<BlockingPlan> vBp;
    for(int iB=1; iB<=iBmax; ++iB){
        //int iVl = iB*ii;
        //int iLp = 1;
        //if( iVl > MVL ){ iVl=MVL; iLp = (ii+iB-1)/iB; }
        // OUTER loop
        int iLpo = (ii+iB-1)/iB;
        // actually 2 cases, iVL allowed (<=MVL, with iLp=1) or not (iVl=MVL, and iLp > 1)
        for(int ic=0; ic<2; ++ic){
            int iVl=0, iLp=0;
            iCost=0.f;
            switch(ic){
              case(0): iVl=1;       // inner loop non-SIMD
                       iLp=iB;
              break;
              default:
              case(1): if(iB>1){    // inner loop SIMD
                           iVl=ve_vlen_suggest(iB);  // iB when <= MVL
                           iLp=(iB+iVl-1)/iVl;
                       }
            }
            if(iVl<=0) continue;
            if(vv>=1) cout<<" ii="<<ii<<" iB="<<iB<<char('a'+ic)
                <<" iVl="<<iVl<<" iLpo="<<iLpo<<" iLp="<<iLp;

            if(iB!=1 && iB!=ii) iCost+=0.03;
            if(iLpo * iB != ii) iCost+=0.03;
            if(iLp > 1) iCost+=0.03;
            //if(!positivePow2(iVl)) iCost+=0.;

            // jB can got from 1 to jj such that iVL*jB <= MVL, or jB <= MVL/iVL
            int const jBmax = max(1,min(MVL/iVl,jj));
            if(vv>=1)cout<<"  jBmax ~ MVL/(iVl["<<iVl<<"]) = "<<MVL/iVl<<endl;
            assert(iVl<=MVL);

            for(int jB=1; jB<=jBmax; ++jB){
                //int jVl = jB*jj*iVl;
                //int jLp = 1;
                //if( jVl > MVL ){ jVl=iVl; jLp = (jj+jB-1)/jB; }
                int jLpo = (jj+jB-1)/jB;
                int jVlPrev=0, jVl=0, jLp=0;
                for(int jc=0; jc<2; ++jc){
                    jCost = 0.f;
                    switch(jc){
                      case(0): jVl=iVl;
                               jLp=jB;
                      break;
                      case(1): if(jB>1){
                                   jVl=ve_vlen_suggest(iVl*jB);
                                   jLp=(jB+jVl-1)/jVl;
                               }
                      break;
                      default: // needs more analysis:
                      case(2): if(jB>1){
                                   jVl=ve_vlen_suggest(iB*jB);
                                   jLp=(jB+jVl-1)/jVl;
                                   iCost=0.f; jCost=0.5f;
                               }
                    }
                    if(jVl==jVlPrev) continue;
                    jVlPrev = jVl;
                    if(jVl<=0) continue;
                    if(vv>=2) cout<<" jj="<<jj<<" jB="<<jB<<char('a'+jc)
                        <<" jVl="<<jVl<<" jLpo="<<jLpo<<" jLp="<<jLp;

                    if(jB!=1 && jB!=jj) jCost+=0.03;
                    if(jLpo * jB != jj) jCost+=0.03;
                    if(jLp > 1) jCost+=0.02;
                    if(iVl>1 && !positivePow2(jVl/iVl)) jCost+=0.5;

                    // kB ...
                    int const kBmax = max(1,min(MVL/jVl,kk));
                    if(vv>=2)cout<<"  kBmax ~ MVL/(jVl["<<jVl<<"]) = "<<MVL/jVl<<endl;
                    assert(jVl<=MVL);
                    for(int kB=1; kB<=kBmax; ++kB){
                        int kLpo = (kk+kB-1)/kB;
                        int kVlPrev=0, kVl=0, kLp=0;
                        for(int kc=0; kc<2; ++kc){
                            kCost = 0.0f;
                            switch(kc){
                              case(0): kVl=jVl;
                                       kLp=kB;
                              break;
                              case(1): if(kB>1){
                                           kVl=ve_vlen_suggest(jVl*kB);
                                           kLp=(kB+kVl-1)/kVl;
                                       }
                              break;
                              default: // needs more analysis:
                              case(2): if(kB>1){
                                           kVl=ve_vlen_suggest(iB*jB*kB);
                                           kLp=(kB+kVl-1)/kVl;
                                           iCost=0.f; jCost=0.f; kCost=0.9f;
                                       }
                            }
                            if(kVl==kVlPrev) continue;
                            kVlPrev = kVl;
                            if(kVl<=0) continue;
                            assert(kVl<=MVL);

                            if(kB!=1 && kB!=kk) kCost+=0.03;
                            if(kLpo * kB != kk) kCost+=0.03;
                            if(kLp > 1) kCost+=0.01;
                            if(jVl>1 && !positivePow2(kVl/jVl)) kCost+=0.7;
                            kCost += (kVl/32) * 0.0001;
                            //kCost += kVl * 0.000001;

                            int const ijkOuter = iLpo*jLpo*kLpo;
                            int const ijkInner = iLp*jLp*kLp;
                            int const kerns = ijkOuter * ijkInner;
                            if(vv>=3) cout<<" kk="<<kk<<" kB="<<kB<<char('a'+kc)
                                <<" kVl="<<kVl<<" kLpo="<<kLpo<<" kLp="<<kLp
                                    <<" ijkOuter="<<ijkOuter<<" ijkInner="<<ijkInner
                                    //<<" kerns="<<kerns<<" cost="<<iCost+jCost+kCost
                                    <<" $"<<iCost<<"+$"<<jCost<<"+$"<<kCost<<" =$"<<iCost+jCost+kCost
                                    <<endl;
                            int const good = 0
                                + (kerns <= minInnerKernels)
                                + (kerns < minInnerKernels);
                            if(v >= 2-good){
                                cout<<(good==0?"bad  ": good==1?" ok  ": "BEST ");
                                //cout<<left<<setw(25)<<OSSFMT("iB,jB,kB="<<iB<<","<<jB<<","<<kB);
                                cout<<left<<setw(15)<<OSSFMT( //"iB,jB,kB="<<
                                        iB<<char('a'+ic)
                                        <<' '<<jB<<char('a'+jc)
                                        <<' '<<kB<<char('a'+kc));
                                cout<<iLpo<<"*To"<<ii<<","<<iB<<":"<<iLp<<"@vl"<<iVl<<" "
                                    <<jLpo<<"*To"<<jj<<","<<jB<<":"<<jLp<<"@vl"<<jVl<<" "
                                    <<kLpo<<"*To"<<kk<<","<<kB<<":"<<kLp<<"@vl"<<kVl<<" "
                                    <<" {o"<<ijkOuter<<"*i"<<ijkInner<<" "<<kerns<<" kerns"
                                    <<" $"<<iCost<<"+$"<<jCost<<"+$"<<kCost
                                    <<" =$"<<iCost+jCost+kCost
                                    <<"}"<<endl;
                            }
                            if(good>=2){
                                // do this if you only want the best [or near ties]
                                //vBp.clear();
                            }
                            if(good>=1){ // best or tied? save it!
                                BlockingPlan bp = {
                                    .ii=ii, .jj=jj, .kk=kk,
                                    .iB=iB, .jB=jB, .kB=kB,
                                    .iVl=iVl, .jVl=jVl, .kVl=kVl,
                                    .iLpo=iLpo, .jLpo=jLpo, .kLpo=kLpo,
                                    .iLp=iLp, .jLp=jLp, .kLp=kLp,
                                    .iCost=iCost, .jCost=jCost, .kCost=kCost,
                                };
                                vBp.push_back(bp);
                            }
                            if(good==2) minInnerKernels = kerns;
                        }
                    }
                }
            }
        }
    }
    cout<<"Found "<<vBp.size()<<" decompositions.  Best had just "
        <<vBp.back().kerns()<<" inner Kernel executions"<<endl;
    if(1){ // print pseudo-code loop blocking
        BlockingPlan p = vBp.back();
        p.prt();
        cout<<"int vl="<<p.kVl<<";\n";
        cout<<"typedef int vr["<<p.kVl<<"];\n";
        cout<<"vr a, b, c, sq, tmp, ta, tb, tc;\n";
        cout<<"// outer loops: "<<p.iLpo<<" "<<p.jLpo<<" "<<p.kLpo<<"\n";
        if(p.iLpo>1){
            cout<<"for(int io=0; io<"<<ii<<"; io+="<<p.iB<<"){ //"<<(ii%p.iB==0?" norem":" rem")<<"\n";
        }else{
            cout<<"{int const io=0;\n";
        }
        if(p.jLpo>1){
            cout<<" for(int jo=0; jo<"<<jj<<"; jo+="<<p.jB<<"){ //"<<(jj%p.jB==0?" norem":" rem")<<"\n";
        }else{
            cout<<" {int const jo=0;\n";
        }
        if(p.kLpo>1){
            cout<<"  for(int ko=0; ko<"<<kk<<"; ko+="<<p.kB<<"){ //"<<(kk%p.kB==0?" norem":" rem")<<" "<<p.ijkOuter()<<" outer\n";
        }else{
            cout<<"  {int const ko=0;\n";
        }
        cout<<"   FOR(n,vl) a[n] = b[n] = c[n] = ta[n] = 0;\n";
#if 0 // nosimd
        if(p.iB>1){
            cout<<"  for(int i=io; i<io+"<<p.iB<<"; ++i){ // iv="<<p.iv()<<"\n";
            cout<<"   FOR(n,vl) a[n] = ko + n%"<<p.jB*p.kB<<"; // ?\n";
        }else{
            cout<<"   for(int i=io; i==io; ++i){ // iv="<<p.iv()<<"\n";
            cout<<"    FOR(n,vl) a[n] = ko;\n";
        }
        if(p.jB>1){
            cout<<"    for(int j=jo; j<jo+"<<p.jB<<"; ++j){ // jv="<<p.jv()<<"\n";
            cout<<"     FOR(n,vl) b[n] = ji + n%"<<p.kB<<"; // ?\n";
        }else{
            cout<<"   for(int j=jo; j==jo; ++j){ // jv="<<p.jv()<<"\n";
            cout<<"    FOR(n,vl) a[n] = ko;\n";
        }
        cout<<"     for(int k=ko; k<ko+"<<p.kB<<"; ++k){ // kv="<<p.kv()<<" vl="<<p.kVl<<"\n";
        cout<<"      prtijk(i,j,k);\n";
#else // simd 'c'
        cout<<"   FOR(n,vl) sq[n] = n;\n";
        cout<<"   // inner loops: "<<p.iLp <<" "<<p.jLp <<" "<<p.kLp <<"\n";
        cout<<"   // inner vls  : "<<p.iv()<<" "<<p.jv()<<" "<<p.kv()<<"\n";
        cout<<"   // inner block: "<<p.iB  <<" "<<p.jB  <<" "<<p.kB  <<"\n";
        if(p.iB>1){
            cout<<"   for(int i=io; i<io+"<<p.iB<<"; i+="<<p.iv()<<"){    // A iB="<<p.iB<<" iv="<<p.iv()<<"\n";
            cout<<"    FOR(n,vl) a[n] = ko + sq[n]/"<<p.jB*p.kB<<";    // divmod by jB*kB="<<p.jB<<"*"<<p.kB<<"\n";
            cout<<"    FOR(n,vl) ta[n] = sq[n]%"<<p.jB*p.kB<<";\n";
        }else{
            //cout<<"  for(int i=io; i==io; ++i){ // iv="<<p.iv()<<"\n";
            cout<<"   {int const i=io;\n";
            cout<<"    FOR(n,vl) a[n] = i;\n";
        }

        if(p.iB>1 && p.jB>1){
            cout<<"    for(int j=jo; j<jo+"<<p.jB<<"; j+="<<p.jv()<<"){    // a jB="<<p.jB<<" jv="<<p.jv()<<"\n";
            cout<<"     FOR(n,vl) b[n] = jo + ta[n]/"<<p.jv()<<";\n";
        }else if(p.jB>1){
            cout<<"    for(int j=jo; j<jo+"<<p.jB<<"; j+="<<p.jv()<<"){    // b jB="<<p.jB<<" jv="<<p.jv()<<"\n";
            cout<<"     FOR(n,vl) b[n] = jo + sq[n]/"<<p.kv()<<";        // kB="<<p.kB<<" kv="<<p.kv()<<"\n";
        }else{
            cout<<"    {int const j=jo;\n";
            cout<<"    FOR(n,vl) b[n] = jo;\n";
        }

        if(p.kB>1){
            cout<<"     for(int k=ko; k<ko+"<<p.kB<<"; k+="<<p.kv()<<"){    // kv="<<p.kv()<<" vl="<<p.kVl<<"\n";
            cout<<"      FOR(n,vl) c[n] = ko + sq[n]%"<<p.kv()<<"; // mod by kB="<<p.kB<<" [kv="<<p.kv()<<"] shortcut\n";
        }else{
            cout<<"     {int const k=ko;\n";
            cout<<"      FOR(n,vl) c[n] = ko;\n";
        }

        int const ijkB = p.iB*p.jB*p.kB;
        if(ijkB%p.kVl){
            cout<<"      vl = min(vl, sq[0]+vl - "<<ijkB%p.kVl<");\n";
        }
        cout<<"      prtijk(i,j,k);\n";
        cout<<"      prt(a); prt(ta); prt(b); prt(c);\n";
        cout<<"      FOR(n,vl) sq[n] += vl;\n";
#endif
#if 0
        cout<<"   for(int il=0; il<"<<p.iLp<<"; il+="<<p.iv()/p.iLp<<"){\n";
        cout<<"    FOR(n,vl) a[n]  = io + (il*"<<p.iv()<<"+n)/"<<p.jB*p.kB<<";\n";
        cout<<"    for(int j=jo; j<jo+"<<p.jB<<"; ++j){ // jv="<<p.jv()<<"\n";
        cout<<"     FOR(n,vl) b[n]  = jo + n/"<<p.kB<<";\n";
        if(1 || p.kB>1){
            cout<<"     for(int k=ko; k<ko+"<<p.kB<<"; ++k){ // kv="<<p.kv()<<" vl="<<p.kVl<<"\n";
            //cout<<"      FOR(n,vl) c[n]  = ko + n%"<<p.kB<<";\n";
        }else{
            cout<<"     { // kv="<<p.kv()<<" vl="<<p.kVl<<"\n";
            //cout<<"      FOR(n,vl) c[n]  = ko;\n";
        }
        //cout<<"      FOR(n,vl) c[n]  = ko + n%"<<p.kB<<";\n";           // ko + 012301230123 (kB=4)
        //cout<<"      FOR(n,vl) tmp[n]= n/"<<p.kB<<";\n";                //      000011112222...
        //cout<<"      FOR(n,vl) b[n]  = jo + tmp[n]%"<<p.jB<<";\n";      // jo + 000011110000 (jB=2)
        //cout<<"      FOR(n,vl) tmp[n]= tmp[n]/"<<p.iB<<";\n";           //      000000001111111122222222
        //cout<<"      FOR(n,vl) a[n]  = io + tmp[n];\n";                 // io + --"--
        cout<<"      prt(a); prt(b); prt(c);\n";
#endif
        cout<<"}}}}}}"<<endl;
    }
#define FOR(VAR,VL) for(int VAR=0; VAR<(VL); ++VAR)
#define prt(a) do{cout<<#a<<": ";for(int u=0;u<vl;++u)cout<<' '<<a[u]; cout<<endl;}while(0)
#define prtijk(i,j,k) do{cout<<" {"<<i<<","<<j<<","<<k<<"}"<<endl;}while(0)
    if(1){
// 1*To3,3:1@vl3 1*To3,3:1@vl9 1*To3,3:1@vl27  {o1*i1 1 kerns $0+$0.5+$0.7 =$1.2}
int vl=27;
typedef int vr[27];
vr a, b, c, sq, tmp, ta, tb, tc;
// outer loops: 1 1 1
{int const io=0;
 {int const jo=0;
  {int const ko=0;
   FOR(n,vl) a[n] = b[n] = c[n] = ta[n] = 0;
   FOR(n,vl) sq[n] = n;
   // inner loops: 1 1 1
   // inner vls  : 3 3 3
   // inner block: 3 3 3
   for(int i=io; i<io+3; i+=3){    // A iB=3 iv=3
    FOR(n,vl) a[n] = ko + sq[n]/9;    // divmod by jB*kB=3*3
    FOR(n,vl) ta[n] = sq[n]%9;
    for(int j=jo; j<jo+3; j+=3){    // 0 jB=3 jv=3
     FOR(n,vl) b[n] = jo + ta[n]/3;
     for(int k=ko; k<ko+3; k+=3){    // kv=3 vl=27
      FOR(n,vl) c[n] = ko + sq[n]%3; // mod by kB=3 [kv=3] shortcut
      prtijk(i,j,k);
      prt(a); prt(ta); prt(b); prt(c);
      FOR(n,vl) sq[n] += vl;
}}}}}}
    }
if(1){
// 1*To1,1:1@vl1 1*To3,3:1@vl3 1*To3,3:1@vl9  {o1*i1 1 kerns $0+$0+$0.7 =$0.7}
int vl=9;
typedef int vr[9];
vr a, b, c, sq, tmp, ta, tb, tc;
// outer loops: 1 1 1
{int const io=0;
 {int const jo=0;
  {int const ko=0;
   FOR(n,vl) a[n] = b[n] = c[n] = ta[n] = 0;
   FOR(n,vl) sq[n] = n;
   // inner loops: 1 1 1
   // inner vls  : 1 3 3
   // inner block: 1 3 3
   {int const i=io;
    FOR(n,vl) a[n] = i;
    for(int j=jo; j<jo+3; j+=3){    // 1 jB=3 jv=3
     FOR(n,vl) b[n] = jo + sq[n]/3/*kB*/; // 
     for(int k=ko; k<ko+3; k+=3){    // kv=3 vl=9
      FOR(n,vl) c[n] = ko + sq[n]%3; // mod by kB=3 [kv=3] shortcut
      prtijk(i,j,k);
      prt(a); prt(ta); prt(b); prt(c);
      FOR(n,vl) sq[n] += vl;
}}}}}}
}
if(1){
// 1*To1,1:1@vl1 1*To3,3:1@vl3 1*To3,3:1@vl9  {o1*i1 1 kerns $0+$0+$0.7 =$0.7}
int vl=9;
typedef int vr[9];
vr a, b, c, sq, tmp, ta, tb, tc;
// outer loops: 1 1 1
{int const io=0;
 {int const jo=0;
  {int const ko=0;
   FOR(n,vl) a[n] = b[n] = c[n] = ta[n] = 0;
   FOR(n,vl) sq[n] = n;
   // inner loops: 1 1 1
   // inner vls  : 1 3 3
   // inner block: 1 3 3
   {int const i=io;
    FOR(n,vl) a[n] = i;
    for(int j=jo; j<jo+3; j+=3){    // b jB=3 jv=3
     FOR(n,vl) b[n] = jo + sq[n]/3;        // kB=3 kv=3
     for(int k=ko; k<ko+3; k+=3){    // kv=3 vl=9
      FOR(n,vl) c[n] = ko + sq[n]%3; // mod by kB=3 [kv=3] shortcut
      prtijk(i,j,k);
      prt(a); prt(ta); prt(b); prt(c);
      FOR(n,vl) sq[n] += vl;
}}}}}}
}


    cout<<"\nGoobye"<<endl;
}
// vim: ts=4 sw=4 et cindent cino=^=l0,\:.5s,=-.5s,N-s,g.5s,b1 cinkeys=0{,0},0),\:,0#,!^F,o,O,e,0=break
