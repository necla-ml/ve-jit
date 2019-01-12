
#include "velogic.hpp"
//#include "jitpage.h"
#include "asmfmt.hpp"
//#include "codegenasm.hpp"

#include <iostream>
#include <iomanip>
#include <sstream>
#include <unordered_set>
#include <vector>
#include <array>

using namespace std;

unordered_set<uint64_t> init_tvals(){
    unordered_set<uint64_t> targ; // target for various Mval,Ival PairS
    unordered_set<uint64_t> mvals;
    {
        // add all r'0+1+' mvals and their complements
        cout<<hex<<setfill('0')<<endl;
        for(uint64_t i=0, mval=0; i<64; mval=(mval<<1U)+1U, ++i ){
            mvals.insert( mval); // mval = 0... 1...
            mvals.insert(~mval); //~mval = 1... 0...
            cout<<setw(2)<<dec<<i<<hex<<" 0x"<<setw(16)<<mval<<"   "<<setw(16)<<~mval<<endl;
        }
        cout<<dec<<setfill(' ');
    }
    cout<<"\n Using "<<mvals.size()<<" mvals for binary-op targets"<<endl;
    uint64_t const one=1U;
    uint64_t mval;
    uint64_t ival;
    // generate a set of target around exact formula values
    for(int64_t ival = -64; ival<=63; ++ival){
        uint64_t const i = (uint64_t)ival;
        for(auto const m: mvals){
            std::array<uint64_t,8> exact =
            { i&m, i|m, i^m, ~(i&m), 
                i+m, i-m, (uint64_t)((int64_t)i-(int64_t)m), m<<(int)(i&0xf) };
            for(auto const e: exact){
                targ.insert( e );
                targ.insert( e+1U );
                targ.insert( e-1U );
            }
        }
    }
    cout<<" generated "<<targ.size()<<" nice targets"<<endl;
    // generate all [1,2]-bit patterns and complements
    for(int i=0; i<64; ++i){
        targ.insert(  ((uint64_t{1}<<i)  ) );
        targ.insert( ~((uint64_t{1}<<i)  ) );
        targ.insert(  ((uint64_t{0x3}<<i)) );
        targ.insert( ~((uint64_t{0x3}<<i)) );
    }
    // generate all 'or's of 4 consecutive 1's
    for(int i=0; i<60; ++i)
        for(int j=i+1; j<60; ++j)
            targ.insert( (uint64_t{1}<<i) | (uint64_t{1}<<j) );
    cout<<" now have "<<targ.size()<<", after simple bit patterns"<<endl;
    // make sure to cover full range [-1024,+1024]
    for(int64_t i=-1024; i<=1024; ++i){
        targ.insert((uint64_t)i);
    }
    cout<<" now have "<<targ.size()<<", after adding [-1024,1024]"<<endl;
    // Using 128 mvals for binary-op targets
    // generated 16299 nice targets
    // now have 17726, after simple bit patterns
    // now have 18726, after adding [-1024,1024]
    //  conclusion:   most >7bit values will go be lea (up to 2^31),
    //                and after, only a tiny fraction have 1-op load.
    //     2-op loads could search first for different
    //     instruction type for the first "lea" (lo 32bit) load.
    return targ;
}
int main(int,char**)
{
    uint64_t parm;
    auto const tvals = init_tvals();
    for(auto t: tvals){
        VeliErr e = { 7, 7, 7 };
        e = veliLoadreg(t /*,count=1U*/);
        cout<<" veliLoadreg("<<t<<" "<<e.i<<") --> "
            <<" error="<<e.error
            <<" other="<<e.other
            <<endl;
    }
    cout<<"\n All "<<tvals.size()<<" coverage tests PASSED";
    cout<<"\nGoodbye"<<endl;

}
// vim: ts=4 sw=4 et cindent cino=^=l0,\:.5s,=-.5s,N-s,g.5s,b1 cinkeys=0{,0},0),\:,0#,!^F,o,O,e,0=break
