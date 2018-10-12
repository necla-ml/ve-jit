
#include "reg-base.hpp"
#include <iostream>
#include <sstream>
#include <stdexcept>
using namespace std;

std::ostream& operator<<(std::ostream& os, RegId const r){
    return os<<'r'<<static_cast<std::underlying_type<RegId>::type>(r);
}
std::ostream& operator<<(std::ostream& os, RegisterBase::Use const u){
    if( u == RegisterBase::Use::free ) /* nop */ ;
    //    <^K>.M· <^K>*X× <^K>Rg® <^K>o/ø <^K>Cu¤ <^K>>» <^K><<«
    // (char)182    215     174     248     164     171    187
    else if( u == RegisterBase::Use::used ) os<<(char)187;    // just some mark
    else if (u == RegisterBase::Use::reserved) os<<(char)215; // like e'x'cluded
    else os<<static_cast<RegisterBase::flags_t>(u);
    return os;
}
std::ostream& operator<<(std::ostream& os, RegisterBase::Cls const c){
    if (c==RegisterBase::Cls::none) /*nop*/ ;
    else if (c==RegisterBase::Cls::scalar) os<<'S';
    else if (c==RegisterBase::Cls::vector) os<<'V';
    else if (c==RegisterBase::Cls::mask) os<<'M';
    else os<<'c'<<static_cast<RegisterBase::flags_t>(c);
    return os;
}
std::ostream& operator<<(std::ostream& os, RegisterBase::Sub const s){
    if (s != RegisterBase::Sub::def) os<<'/'<<static_cast<RegisterBase::flags_t>(s);
    return os;
}  
std::ostream& operator<<(std::ostream& os, RegisterBase::Vlen const vl){
    if (RegisterBase::Int(vl) != 0) os<<'['<<static_cast<RegisterBase::flags_t>(vl)<<']';
    return os;
}
std::ostream& operator<<(std::ostream& os, RegisterBase::Misc const m){
    if (m != RegisterBase::Misc::def) os<<'m'<<static_cast<RegisterBase::flags_t>(m);
    return os;
}

//static char const* uses[] = {"free", "used", "reserved"};
std::ostream& operator<<(std::ostream& os, RegisterBase const& r){
    ostringstream oss;
#if 0
    auto u = r.flags.use.z();
    if( u <= sizeof(uses)/sizeof(char*) ) oss<<uses[u];         // known
    else                                  oss<<"Use:"<<u;       // unknown
    //auto cls = r.flags.cls.z();
#else
    //   <--- static ----------------> <----- dynamic---------------------->
    oss<<r.rid<<':'<<r.cls()<<r.misc()<<r.sub()<<r.vlen()<<r.use();
    //<<vlen()<<misc();
#endif
    return os<<oss.str();
}

std::runtime_error invalid_RegId(char const* file, int line, RegId id){
    ostringstream oss;
    oss<<" ERROR: "<<file<<':'<<line<<" RegId "<<id<<" out-of-range"<<endl;
    return std::runtime_error(oss.str());
}

// vim: ts=4 sw=4 et cindent cino=^=l0,\:.5s,=-.5s,N-s,g.5s,h.5s,b1 cinkeys=0{,0},0),\:,0#,!^F,o,O,e,0=break syntax=cpp.doxygen
