#include "old/convParam.hpp" // this approach used a template parameter for dil_start
//#include "convParamxx.h"

#include <iostream>
#include <cassert>

using namespace std;
using namespace convparam;
//using namespace conv;

int main(int,char**) {
    printf(" test: " __FILE__ "\n");
    auto a = ConvOnednn();
    assert(a.check() == 2);     // crazy
    assert(a.batchNum == 0);          // 1 of the crazy values
    assert(a.dil_start == 0);

    auto b = ConvVednn();
    assert(b.check() == 2);
    assert(b.batchNum == 0);
    assert(b.dil_start == 1);

    string o("mb8g1_ic7ih8_oc5_kh3ph1_dh1"); // dilated!
    auto c = ConvOnednn(o);
    cout<<" string "<<o<<"\n onednn "<<c.str()
        <<"\n     or "<<c.str_short()<<endl;

    auto d = ConvVednn(o);
    cout<<" string "<<o<<"\n  vednn "<<d.str()
        <<"\n     or "<<d.str_short()<<endl;

#if 0 // oh, this is in csv2examples.h
    auto n = allFeature_sz();
    cout<<"Full list of features:";
    for(int i=0; i<n; ++i){
        cout<<(i%5==0? "\n    ": "    ")<<allFeature_name(i);
        assert( allFeature_name(i) == allFeature_names()[i] );
    }
    cout<<endl;
#endif

    cout<<"\nGoodbye"<<endl;
}

