/* Copyright (c) 2019 by NEC Corporation
 * This file is part of ve-jit */

//#include "reg-base.hpp" // what can we do with just this?
// A: Not much. without some chip knowledge, we don't even know how many
// registers we should print.
//

#include "reg-aurora.hpp"

#include <assert.h>
#include <iomanip>

using namespace std;
int main(int,char**){
    //RegisterBase r(Regid(0)); // <-- now illegal
    // print RegId properties.  (nothing here is really dynamic).
    // dynamic flags (vlen vs defMaxVlen) are within the flags
    // of RegisterBase wrapper around RegId.
    //
    // Issue: RegId converts to RegisterBase::Flags, which is bad.
    //
    cout<<" Here is a dump of chipset-specific characteristics"<<endl;
    cout<<" that use chipset-specific foo(Regid) functions."<<endl;
    cout<<" For example, some of these fields relate to the 'C' ABI for Aurora."<<endl;
    for(int i=0; i<IDlast; ++i){
        RegId r = Regid(i);
        assert(isScalar(r)+isVector(r)+isMask(r) == 1); // exactly-one-of
        cout<<"RegId "<<setw(3)<<r
            <<" "<<setw(5)<<left<<asmname(r)<<right
            <<" "<<(isScalar(r)?"S":isVector(r)?"V":"M")
            <<(abi_c_reserved(r)?" RESERVE":"        ")
            <<(abi_c_preserved(r)?" PRESERVE":"         ")
            <<(canBeArg(r,Abi::c)?" Arg":"    ")
            <<" defRegFlags="<<hex<<setw(8)<<defRegFlags(r)<<dec
            <<" "<<(int)(uint_least8_t)(cls(r))
            <<" defMaxVlen="<<defMaxVlen(r)
            <<" defAlign="<<defAlign(r)
            //<<" vlen="<<vlen(r) no such thing
            <<" bytes="<<bytes(r)   // max bytes, corresp to max vlen
            <<" align="<<align(r)   // equiv. to defAlig
            <<endl;
            ;
    }
#define RB RegisterBase
#define FLG RB::Flags
    cout<<" After mkRegistersAurora(), we can access some RegisterBase"
        "\n standard information that is rather generic.  Use and vector"
        "\n length are dynamic, but other RegisterBase flags are probably const."
        "\n Naturally, we don't know what misc and sub fields actually mean,"
        "\n if they are non-zero."
        "\n"
        "\n The generic interface just uses mkChipRegistersAurora() to"
        "\n generate a map of all RegId to RegBase objects."
        <<endl;
    cout<<static_cast<int>(RB::Cls::mask)<<endl;                // 2
    cout<<(int)(RB::Int(RB::Cls::mask))<<endl;                  // 2
    cout<<FLG::cls_t{RB::Int(RB::Cls::mask)}.z()<<endl;         // 0 no constructors -- it's z POD
    cout<<(int)(FLG::cls_t::zero() + RB::Int(RB::Cls::mask))<<endl;     // 17 ?
    cout<<FLG::cls_t::mkval(RB::Int(RB::Cls::mask))<<endl;      // 32 Yay?
    cout<<RB::shift(RB::Cls::mask)<<endl;      // 32 Yay?
    auto& regs = mkChipRegistersAurora();
    for(uint32_t i=0U; i<regs.size(); ++i){
        RegisterBase const& ri = regs(i);
        cout<<left<<setw(15)<<ri
            <<" reg "<<setw(4)<<(uint32_t)ri.rid
            <<" use "<<(uint32_t)RegisterBase::Int(ri.use()) // Int->small int type, maybe not printing nicely
            <<" cls "<<(uint32_t)RegisterBase::Int(ri.cls())
            <<" sub "<<(uint32_t)ri.sub()
            <<" vl "<<(uint32_t)ri.vlen() // or we can just cast enum to int type
            <<" misc "<<(uint32_t)ri.misc()
            <<" flags=0x"<<hex<<(uint32_t)ri.getFlags()<<dec
            <<endl;
    }
    cout<<" dynamic RegBase state examples"<<endl;
    cout<<"Now let me mark s0..1 as Use::used, and s2..s3 as Use::reserved"
        "\n%s4 with strange settings: vector length 1, used and subclass 7"
        "\n%v0 as vlen 256, %v1 as vlen 37+used, %v2 as vlen 200+reserved"
        <<endl;
    regs(IDscalar+0).set(RB::Use::used);
    regs(IDscalar+1).set(RB::Use::used);
    regs(IDscalar+2).set(RB::Use::reserved);
    regs(IDscalar+3).set(RB::Use::reserved);
    regs(IDscalar+4).set(RB::Use::used).set(RB::Enum<RB::Vlen>(1)).set(RB::Enum<RB::Sub>(7));
    // now regs(4) prints as r4:S/7[1]u
    // BETTER: TODO 
    // set_vlen(int), set_sub(int), ... or even setT<RB::Vlen>(int value)
    for(uint32_t i=IDscalar; i<IDscalar+5; ++i)
        cout<<left<<setw(15)<<regs(i)<<endl;
    for(uint32_t i=IDvector; i<IDvector+5; ++i)
        cout<<left<<setw(15)<<regs(i)<<endl;


    cout<<"\nGoodbye!"<<endl;
}
// vim: ts=4 sw=4 et cindent cino=^=l0,\:.5s,=-.5s,N-s,g.5s,h.5s,b1 cinkeys=0{,0},0),\:,0#,!^F,o,O,e,0=break
