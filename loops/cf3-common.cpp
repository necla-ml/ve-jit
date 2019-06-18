/* Copyright (c) 2019 by NEC Corporation
 * This file is part of ve-jit */
#include "cf3.hpp"

using namespace std;
struct LoopSplit parseLoopSplit(char const* arg){
    // parse up to 4 ':'-separated numbers from string like
    // [a:[b:[c:]]]d
    uint32_t a=0U, b=0U, c=0U, d=0U, u;
    char sep;
    if(arg && arg[0]!= '\0'){
        istringstream iss(arg);
        // see https://en.cppreference.com/w/cpp/io/basic_ios/good
        // for meaning of fail vs good
        iss>>u; if(iss.fail()) {cout<<"A"; goto done; }
        a=0U; b=0U; c=u; d=u;                 // D --> 0..0..d..d
        // is first number OK and followed by a next number ":%u" ?
        iss>>sep; if(!iss.good() || sep!=':') {cout<<"B"; goto done; }
        iss>>u; if(iss.fail()) {cout<<"C"; goto done; }
        a=d; b=d; c=u; d=u;                   // A:D --> a..a..d..d
        iss>>sep; if(!iss.good() || sep!=':') {cout<<"D"; goto done; }
        iss>>u; if(iss.fail()) {cout<<"E"; goto done; }
        /*a=a;*/ b=c; c=u; d=u;               // A:B:D --> a..b..d..d
        iss>>sep; if(!iss.good() || sep!=':') {cout<<"F"; goto done; }
        iss>>u; if(iss.fail()) {cout<<"G"; goto done; }
        /*a=a; b=b;*/ c=d; d=u;               // A:B:C:D --> a..b..c..d
    }
done:
    cout<<" Raw LoopSplit = "<<a<<", "<<b<<", "<<c<<", "<<d<<endl;
    struct LoopSplit ret = {a,b,c,d}; // do not check for a<=b<=c<=d
    return ret;
}
ostream& operator<<(ostream& os, struct LoopSplit const& ls){
    os<<"for("<<ls.z<<"..";
    if(1 || ls.lo!=0U){
        os<<ls.lo;
        if(ls.lo<ls.z || ls.lo>ls.end) os<<":error:";
        os<<"..";
    }
    if(1 || ls.hi!=ls.end){
        os<<ls.hi;
        if(ls.hi<ls.lo || ls.hi>ls.end) os<<":error:";
        os<<"..";
    }
    os<<ls.end<<")";
    return os;
}
// vim: ts=4 sw=4 et cindent cino=^=l0,\:.5s,=-.5s,N-s,g.5s,b1 cinkeys=0{,0},0),\:,0#,!^F,o,O,e,0=break
