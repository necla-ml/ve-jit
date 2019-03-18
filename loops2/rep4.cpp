/* Copyright (c) 2019 by NEC Corporation
 * This file is part of ve-jit */
#include "arith.hpp"
//#include "vtypes.hpp"
#include <iostream>
#include <iomanip>
#include <cstdint>

using namespace std;
using namespace arith;

int main(int,char**){
    typedef Arith<int64_t> I64;
#define DOIT(A,...) \
    cout<<"=== "<<left<<setw(40)<<#__VA_ARGS__; cout.flush(); \
    __VA_ARGS__; \
    cout \
    <<" "<<A.flags()<<" "<<A.expr()<<" --> "<<A.v() \
    <<" rhs:"<<A.rhs() \
    <<" c:"<<setw(10)<<A.v()<<" n:"<<setw(20)<<A.n()<<" e:"<<A.e() \
    <<" decl:"<<A.decl() \
    <<endl;;
    if(0){
        DOIT(const_0, I64 const_0(0));
        DOIT(const_1, I64 const_1(1));
        DOIT(i, I64 i(1,"i",CONST));
        DOIT(j, I64 j(1,"j",NONCONST));
        {DOIT(cpy, I64 cpy(i));}
        {DOIT(cpy, I64 cpy(j));}
        {DOIT(cpy, I64 cpy(i,"cpy"));}
        {DOIT(cpy, I64 cpy(j,"cpy"));}
        {DOIT(x, I64 x(const_0+const_1));}
        {DOIT(x, I64 x(const_1+const_1*3));}
        {DOIT(x, I64 x(i+1));}
        {DOIT(x, I64 x(i*2-1));}
        {DOIT(x, I64 x(1+i));}
        {DOIT(x, I64 x(i+const_0));}
        {DOIT(x, I64 x(i+const_1));}
        {DOIT(x, I64 x(j+1));}
        {DOIT(x, I64 x(j*2-1));}
        {DOIT(x, I64 x(1+j));}
        {DOIT(x, I64 x(j+0));}
        {DOIT(x, I64 x(0+j));}
        {DOIT(x, I64 x(j+const_1));}
        {DOIT(y, I64 y(i+1,"y"));}
        {DOIT(y, I64 y(1+i,"y"));}
        {DOIT(y, I64 y(i+const_0,"y"));}
        {DOIT(y, I64 y(i+const_1,"y"));}
        {DOIT(y, I64 y(j+1,"y"));}
        {DOIT(y, I64 y(1+j,"y"));}
        {DOIT(y, I64 y(j+0,"y"));}
        {DOIT(y, I64 y(0+j,"y"));}
        {DOIT(y, I64 y(j+const_1,"y"));}
        {DOIT(a, I64 a(0,"a",NONCONST));
            DOIT(b, I64 b(0,"b",NONCONST));
            DOIT(k, I64 k(2*par(a+1)+3*par(b+3),"k"));
            DOIT(b, b=b+1);
            DOIT(a, a=a+b);
            DOIT(k, k = 2*par(a+1)+3*par(b+3));
            DOIT(b, b=b/par(i*2-1));
            DOIT(a, a=a*j);
            DOIT(a, a=a/par(j*2-1));
            DOIT(k, k = 2*par(a+1)+3*par(b+3));
        }

    }
#define EXPR(var, CODE) \
    CODE; /*cout<<endl;*/ \
    cout<<setw(40)<<#CODE<<var.flags()<<" "<<var.expr()<<endl
#define DECL(var, CODE) \
    CODE; /*cout<<endl;*/ \
    cout<<setw(40)<<#CODE<<var.flags()<<" "<<var.decl()<<endl
    if(0){
        DECL(a, I64 a(2,"a",CONST));
        DECL(b, I64 b(3,"b",CONST));
        DECL(i, I64 i(0,"i",NONCONST));
        DECL(j, I64 j(0,"j",NONCONST));
        DECL(k, I64 k(a*i+b*j+ I64(1),"k"));
        //{EXPR(x, I64 x(i=i+I64(1)));} // not working
        //{EXPR(x, I64 x(j=j+I64(1)));}
        EXPR(i, i=i+I64(1));
        EXPR(j, j=j+I64(1));
        EXPR(j, j=a+b);
        //{EXPR(tmp, I64 tmp(a*i+b*j+I64(1)));}
        EXPR(k, k = a*i+b*j+1);
        EXPR(i, i=13);
        EXPR(j, j=i);
        EXPR(k, k = a*par(i+b)*par(j-5));
        EXPR(j, j=b);
        EXPR(j, j=0+1+i+1+2);
    }
#define DEF(var, CODE) DECL(var, I64 var(CODE,#var,NONCONST));
#define CDEF(var, CODE) DECL(var, I64 var(CODE,#var));
#define LET(var, CODE) EXPR(var, var = CODE);
    if(1){
        CDEF(a, 2);
        CDEF(b, 3);
        DEF(i, 0);
        DEF(j, 0);
        DEF(k, a*i+b*j+1);
        LET(i, i+1);
        LET(j, j+1);
        LET(k, a*i+b*j+1);
        LET(i, 13);
        LET(j, i);
        LET(k, a*par(i+b)*par(j-5));
        LET(j, b);
        LET(j, 0+1+i+1+2);
    }
    cout<<"========================"<<endl;
    typedef Mul<int64_t> Mul64;
    typedef Lin<int64_t> Lin64;
    //typedef Add<int64_t> Add64;
    if(1){
        int64_t pData = 2;
        int64_t inputHeight = 3;
        int64_t inputWidth = 4;
        CDEF(pdata, pData);
        CDEF(ih, inputHeight);
        CDEF(iw, inputWidth);
        DEF(vh, inputHeight);
        DEF(vw, inputWidth);
        DEF(c, 0);
        DEF(i, 0);
        DEF(j, 0);
        {Mul64 m(I64(3),I64(4)); cout<<m<<endl;}
        {Mul64 m(j,iw); cout<<m<<endl;}
        {Mul64 m(j,iw*ih); cout<<m<<endl;}
        {Mul64 m(j,vw); cout<<m<<endl;}
        {Mul64 m(j,vw*vh); cout<<m<<endl;}
        {Lin64 lin(pdata); cout<<lin<<endl;}
        {Lin64 lin(pdata, Mul64(i,ih*iw)); cout<<lin<<endl;}
        {Lin64 lin(pdata, Mul64(c,ih*iw), Mul64(i,iw), Mul64(j,1)); cout<<lin<<endl;}
        {Lin64 lin(pdata, Mul64(c,ih*iw), Mul64(i,iw), Mul64(j,1), Mul64(2,3));
            cout<<lin<<endl;
            cout<<"getConst:"; cout.flush();
            I64 cc(lin.getConst());
            cout<<"{"<<cc.flags()<<","<<cc.v()<<","<<cc.n()<<","<<cc.e()<<"}"<<std::endl;
            cout<<"getCoeff(i):"; cout.flush();
            I64 ci(lin.getCoeff(i));
            cout<<"{"<<ci.flags()<<","<<ci.v()<<","<<ci.n()<<","<<ci.e()<<"}"<<std::endl;
            cout<<"getCoeff(j):"; cout.flush();
            I64 cj(lin.getCoeff(j));
            cout<<"{"<<cj.flags()<<","<<cj.v()<<","<<cj.n()<<","<<cj.e()<<"}"<<std::endl;
        }
        //pIn = linear(data,mul(i,ih*iw),mul(j,iw));
        //cout<<pIn<<endl;
    }

    cout<<"\nGoodbye!"<<endl;
}
// vim: sw=4 ts=4 et cindent ai cino=^=lg\:0,j1,(0,wW1 formatoptions=croql syntax=cpp.doxygen
