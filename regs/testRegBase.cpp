
#include "reg-impl.hpp"
#include <assert.h>
#include <iomanip>

using namespace std;
int main(int,char**){
    RegisterBase r(Regid(0));
    for(int i=0; i<IDlast; ++i){
        RegId r = Regid(i);
        assert(isScalar(r)+isVector(r)+isMask(r) == 1); // exactly-one-of
        cout<<"RegId "<<setw(3)<<r
            <<" "<<setw(5)<<left<<asmname(r)<<right
            <<" "<<(isScalar(r)?"S":isVector(r)?"V":"M")
            <<(abi_c_reserved(r)?" RESERVE":"        ")
            <<(abi_c_preserved(r)?" PRESERVE":"         ")
            <<(canBeArg(r,Abi::c)?" Arg":"    ")
            <<" defRegFlags="<<defRegFlags(r)
            <<" "<<(int)(uint_least8_t)(cls(r))
            <<" defMaxVlen="<<defMaxVlen(r)
            <<" defAlign="<<defAlign(r)
            <<" bytes="<<bytes(r)
            <<" align="<<align(r)
            <<endl;
            ;
    }
#define RB RegisterBase
#define FLG RB::Flags
    cout<<static_cast<int>(RB::Cls::mask)<<endl;                // 2
    cout<<(int)(RB::Int(RB::Cls::mask))<<endl;                  // 2
    cout<<FLG::cls_t{RB::Int(RB::Cls::mask)}.z()<<endl;         // 0 no constructors -- it's z POD
    cout<<(int)(FLG::cls_t::zero() + RB::Int(RB::Cls::mask))<<endl;     // 17 ?
    cout<<FLG::cls_t::mkval(RB::Int(RB::Cls::mask))<<endl;      // 32 Yay?
    cout<<RB::shift(RB::Cls::mask)<<endl;      // 32 Yay?
    auto& regs = mkChipRegistersAurora();
    for(uint32_t i=0U; i<regs.size(); ++i){
        RegisterBase const& ri = regs(i);
        cout<<" reg "<<ri.rid
            <<" "<<(uint32_t)RegisterBase::Int(ri.use())
            <<" "<<(uint32_t)RegisterBase::Int(ri.cls())
            <<" "<<(uint32_t)ri.vlen()
            <<" "<<(uint32_t)ri.misc()
            <<" flags=0x"<<(uint32_t)ri.getFlags()
            <<endl;
    }

    cout<<"\nGoodbye!"<<endl;
}
// vim: ts=4 sw=4 et cindent cino=^=l0,\:.5s,=-.5s,N-s,g.5s,h.5s,b1 cinkeys=0{,0},0),\:,0#,!^F,o,O,e,0=break
