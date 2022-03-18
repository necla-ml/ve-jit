#include "convParamxx.h" // new C++ to C header

#include <iostream>
#include <cassert>

using namespace std;
using namespace conv;

int main(int,char**) {
    printf(" test: " __FILE__ "\n");
    auto a = ConvParam_t();
    assert(a.check() == CONVCHECK_ERROR);     // crazy
    assert(a.batchNum == -1);          // 1 of the crazy values
    assert(a.dil_start == -1);

    auto b = a;
    b.toVednn();
    assert(b.dil_start == 1);
    b.toOnednn();
    assert(b.dil_start == 0);
    assert(b.check() == CONVCHECK_ERROR);
    cout<<endl;

    string o("mb8g1_ic7ih8_oc5_kh3ph5_dh1"); // dilated!
    auto c = ConvParam_t(o);
    cout<<" string "<<o
        <<"\n onednn "<<c.str()
        <<"\n     or "<<c.str_short()<<"\n"<<endl;

    c.mkDense();
    cout<<" string "<<o
        <<"\n mkDense "<<c.str()
        <<"\n     or "<<c.str_short()<<"\n"<<endl;

    auto ovr = ConvParam();
    ovr.inHeight = 111;
    c = ConvParam_t(o);
    c.mkDenseOvr( ovr ); // make dense, overriding inHeight only
    cout<<" string "<<o
        <<"\n ovr ih "<<c.str()
        <<"\n     or "<<c.str_short()<<"\n"<<endl;
    assert( c.inHeight == 111 );
    assert( c.inWidth == 8 );

    auto cc = ConvParam(o, ovr); // apply ovr "as if present in string"
    // this will behave differently, apply ih override also to
    // the missing iw.
    cout<<" string "<<o
        <<"\n ovr ih "<<cc.str()<<"\n"<<endl;
    assert( c.inHeight == 111 );
    assert( c.inWidth == 8 ); // OOPS XXX NOT WORKING AS expected


#if 0 // oh, this is in csv2examples.h
    auto n = allFeature_sz();
    cout<<"Full list of features:";
    for(int i=0; i<n; ++i){
        cout<<(i%5==0? "\n    ": "    ")<<allFeature_name(i);
        assert( allFeature_name(i) == allFeature_names()[i] );
    }
    cout<<endl;
#endif



    string k1("ih10_kh1"); // easy test case
    auto d = ConvParam_t(k1);
    auto e = d; e.mkDense();
    cout<<" string  "<<k1
        <<"\n  vednn  "<<d.str()
        <<"\n  fillin "<<d.str_short()
        <<"\n   dense "<<e.str_short()<<"\n"<<endl;

    cout<<"\nGoodbye"<<endl;
}

