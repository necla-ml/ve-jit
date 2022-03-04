#include <iostream>

#include "asmfmt_fwd.hpp"

using namespace std;

int main(int argc, char*argv){
    cout<<"Enter an integer:  "; cout.flush();
    int64_t i;
    cin>>dec>>i;
    uint64_t u = i;
    cout<<" i = "<<dec<<i<<" = "<<hex<<i<<endl
        <<" u = "<<dec<<u<<" = "<<hex<<u<<endl<<dec;
    cout<<dec<<"  ve_load64("<<u<<") -->\n"
        <<ve_load64("%s63",u)<<endl;
    //cout<<"  ve_set_vector_length("<<i%256<<",tmp) -->\n"
    //    <<ve_set_vector_length((uint64_t)i%256,"%s62")<<endl;
    AsmFmtVe af;
    uint64_t const vl = u % 256;
    af.set_vector_length(vl);
    string setvl = af.flush_all();
    cout<<dec<<"  AsmFmtVe::set_vector_length("<<vl<<",tmp) -->\n"
        <<setvl<<endl;
    cout<<"\nGoodbye"<<endl;
}
// vim: ts=4 sw=4 et cindent cino=^=l0,\:.5s,=-.5s,N-s,g.5s,b1 cinkeys=0{,0},0),\:,0#,!^F,o,O,e,0=break
