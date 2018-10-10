
//#include "reg-impl.hpp" // try to keep this completely generic. "?" for nonstandard fields
#include "reg-base.hpp"
#include <iostream>
#include <sstream>
#include <stdexcept>
using namespace std;

static char const* uses[] = {"free", "used", "reserved"};

std::ostream& operator<<(std::ostream& os, RegisterBase const& r){
    ostringstream oss;
    auto u = r.flags.use.z();
    if( u <= sizeof(uses)/sizeof(char*) ) oss<<uses[u];         // known
    else                                  oss<<"Use:"<<u;       // unknown

    //auto cls = r.flags.cls.z();
    return os<<oss.str();
}

std::runtime_error invalid_RegId(char const* file, int line, RegId id){
    ostringstream oss;
    oss<<" ERROR: "<<file<<':'<<line<<" RegId "<<id<<" out-of-range"<<endl;
    return std::runtime_error(oss.str());
}

// vim: ts=4 sw=4 et cindent cino=^=l0,\:.5s,=-.5s,N-s,g.5s,h.5s,b1 cinkeys=0{,0},0),\:,0#,!^F,o,O,e,0=break
