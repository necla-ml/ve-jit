#include "conv_features.h"
#include "csv2examples.hpp"
#include <iostream>
#include <iomanip>

using namespace std;
using namespace expt; // C++ namespace for AllRunsxx, ...

string s0 = R"EOF(
param,name,layertype,layer,best,reps,threads,ms,Gflops,err,ops
mb1g8_ic8ih8oc8oh8kh1,"wip",*,libvednn-std:dil1_str1_pad0_ker1_T,0,9,8,0.345,0.002971,0,1000
mb1g8_ic8ih8oc8oh8kh1,"wip",I,cnvFwd-d1p0,0,9,8,0.033,0.031,0,1000
mb1g8_ic8ih8oc8oh8kh1,"wip",I,cnvFwd-d1p0_owU128,0,9,8,0.031,0.033,0,1000
mb1g8_ic8ih8oc8oh8kh1,"wip",I,cnvFwd-d1p0_owU128,0,9,8,0.031,0.033,0,1000
mb1g8_ic8ih8oc8oh8kh1,"wip",I,cnvFwd-vecC,0,9,8,0.044,0.023,0,1000
mb1g8_ic8ih8oc8oh8kh1,"wip",I,cnvFwd-vecC,0,9,8,0.044,0.023,0,1000
mb1g8_ic8ih8oc8oh8kh1,"wip",I,cnvFwd-vecC,0,9,8,0.044,0.023,0,1000
mb1g8_ic8ih8oc8oh8kh1,"wip",J,cjitConvFwd1q,0,9,8,0.005,0.212,0,1000
mb1g8_ic8ih8oc8oh8kh1,,R,gemm-Ref,0,1,8,0.231,0.004426,0,1000
mb1g8_ic8ih96iw198_oc8oh24ow49_kh3sh4,"wip",*,libvednn-std:owU128_T,0,9,8,0.317,0.534,0,170000
mb1g8_ic8ih96iw198_oc8oh24ow49_kh3sh4,"wip",I,cnvFwd-d1p0,0,9,8,0.048,3.562,0,170000
mb1g8_ic8ih96iw198_oc8oh24ow49_kh3sh4,"wip",J,unroll_cjitConvFwd6,0,9,8,0.011,15.487,0,170000
mb1g8_ic8ih96iw198_oc8oh24ow49_kh3sh4,,R,gemm-Ref,0,1,8,0.375,0.452,0,170000
)EOF" ;

string s9 = R"EOF(
param,name,layertype,layer,best,reps,threads,ms,Gflops,err,ops
mb1g8_ic8ih8oc8oh8kh1,"wip",*,libvednn-std:dil1_str1_pad0_ker1_T,0,9,8,0.345,0.002971,0,1000
mb1g8_ic8ih8oc8oh8kh1,"wip",I,cnvFwd-d1p0,0,9,8,0.033,0.031,0,1000
mb1g8_ic8ih8oc8oh8kh1,"wip",I,cnvFwd-d1p0_owU128,0,9,8,0.031,0.033,0,1000
mb1g8_ic8ih8oc8oh8kh1,"wip",I,cnvFwd-d1s1p0,0,9,8,0.033,0.031,0,1000
mb1g8_ic8ih8oc8oh8kh1,"wip",I,cnvFwd-d1s1p0_owU128,0,9,8,0.031,0.033,0,1000
mb1g8_ic8ih8oc8oh8kh1,"wip",I,cnvFwd-d1s1pS,0,9,8,0.031,0.033,0,1000
mb1g8_ic8ih8oc8oh8kh1,"wip",I,cnvFwd-def,0,9,8,0.034,0.03,0,1000
mb1g8_ic8ih8oc8oh8kh1,"wip",I,cnvFwd-gemm:0,0,9,8,0.086,0.012,0,1000
mb1g8_ic8ih8oc8oh8kh1,"wip",I,cnvFwd-owU128,0,9,8,0.032,0.032,0,1000
mb1g8_ic8ih8oc8oh8kh1,"wip",I,cnvFwd-owU128_T,0,9,8,0.062,0.016,0,1000
mb1g8_ic8ih8oc8oh8kh1,"wip",I,cnvFwd-p0k1,0,9,8,0.031,0.033,0,1000
mb1g8_ic8ih8oc8oh8kh1,"wip",I,cnvFwd-p0k1_owU128,0,9,8,0.031,0.033,0,1000
mb1g8_ic8ih8oc8oh8kh1,"wip",I,cnvFwd-s1p0k1,0,9,8,0.032,0.032,0,1000
mb1g8_ic8ih8oc8oh8kh1,"wip",I,cnvFwd-s1p0k1_T,0,9,8,0.06,0.017,0,1000
mb1g8_ic8ih8oc8oh8kh1,"wip",I,cnvFwd-vecC,0,9,8,0.044,0.023,0,1000
mb1g8_ic8ih8oc8oh8kh1,"wip",I,cnvFwd-vecC-d1p0k1,0,9,8,0.039,0.026,0,1000
mb1g8_ic8ih8oc8oh8kh1,"wip",I,cnvFwd-vecC-d1p0k1cU1024,0,9,8,0.046,0.022,0,1000
mb1g8_ic8ih8oc8oh8kh1,"wip",J,cjitConvFwd1q,0,9,8,0.005,0.212,0,1000
mb1g8_ic8ih8oc8oh8kh1,"wip",J,cjitConvFwd6,1,9,8,0.005,0.22,0,1000
mb1g8_ic8ih8oc8oh8kh1,"wip",J,unroll_cjitConvFwd1q,0,9,8,0.005,0.215,0,1000
mb1g8_ic8ih8oc8oh8kh1,"wip",J,unroll_cjitConvFwd6,0,9,8,0.005,0.22,0,1000
mb1g8_ic8ih8oc8oh8kh1,,R,gemm-Ref,0,1,8,0.231,0.004426,0,1000
mb1g8_ic8ih96iw198_oc8oh24ow49_kh3sh4,"wip",*,libvednn-std:owU128_T,0,9,8,0.317,0.534,0,170000
mb1g8_ic8ih96iw198_oc8oh24ow49_kh3sh4,"wip",I,cnvFwd-d1p0,0,9,8,0.048,3.562,0,170000
mb1g8_ic8ih96iw198_oc8oh24ow49_kh3sh4,"wip",I,cnvFwd-d1p0_owU128,0,9,8,0.046,3.716,0,170000
mb1g8_ic8ih96iw198_oc8oh24ow49_kh3sh4,"wip",I,cnvFwd-def,0,9,8,0.057,2.973,0,170000
mb1g8_ic8ih96iw198_oc8oh24ow49_kh3sh4,"wip",I,cnvFwd-gemm:1,0,9,8,0.407,0.416,0,170000
mb1g8_ic8ih96iw198_oc8oh24ow49_kh3sh4,"wip",I,cnvFwd-owU128,0,9,8,0.046,3.716,0,170000
mb1g8_ic8ih96iw198_oc8oh24ow49_kh3sh4,"wip",I,cnvFwd-owU128_T,0,9,8,0.063,2.672,0,170000
mb1g8_ic8ih96iw198_oc8oh24ow49_kh3sh4,"wip",I,cnvFwd-vecC,0,9,8,0.586,0.289,0,170000
mb1g8_ic8ih96iw198_oc8oh24ow49_kh3sh4,"wip",J,cjitConvFwd1q,0,9,8,0.011,15.988,0,170000
mb1g8_ic8ih96iw198_oc8oh24ow49_kh3sh4,"wip",J,cjitConvFwd6,0,9,8,0.011,16.002,0,170000
mb1g8_ic8ih96iw198_oc8oh24ow49_kh3sh4,"wip",J,unroll_cjitConvFwd1q,1,9,8,0.011,16.012,0,170000
mb1g8_ic8ih96iw198_oc8oh24ow49_kh3sh4,"wip",J,unroll_cjitConvFwd6,0,9,8,0.011,15.487,0,170000
mb1g8_ic8ih96iw198_oc8oh24ow49_kh3sh4,,R,gemm-Ref,0,1,8,0.375,0.452,0,170000
mb2g16_ic16ih256oc16oh254kh3,"wip",*,libvednn-std:mb-dil1_str1_pad0_ker3_iw2XU256_ow2X_ioaligned,0,9,8,7.435,4.998,0,3.7e+07
mb2g16_ic16ih256oc16oh254kh3,"wip",I,cnvFwd-d1p0,0,9,8,5.798,6.409,0,3.7e+07
mb2g16_ic16ih256oc16oh254kh3,"wip",I,cnvFwd-d1p0_xw,0,9,8,0.645,57.647,0,3.7e+07
mb2g16_ic16ih256oc16oh254kh3,"wip",I,cnvFwd-d1s1p0,0,9,8,6.321,5.879,0,3.7e+07
mb2g16_ic16ih256oc16oh254kh3,"wip",I,cnvFwd-d1s1p0_xw,0,9,8,0.645,57.645,0,3.7e+07
mb2g16_ic16ih256oc16oh254kh3,"wip",I,cnvFwd-d1s1p0k3iw2XU256_ow2X_ioaligned,0,9,8,7.101,5.233,0,3.7e+07
mb2g16_ic16ih256oc16oh254kh3,"wip",I,cnvFwd-d1s1p0k3iw2XU256_ow2X_ioaligned_xw,0,9,8,1.459,25.471,0,3.7e+07
mb2g16_ic16ih256oc16oh254kh3,"wip",I,cnvFwd-def,0,9,8,7.251,5.125,0,3.7e+07
mb2g16_ic16ih256oc16oh254kh3,"wip",I,cnvFwd-def_xw,0,9,8,1.359,27.338,0,3.7e+07
mb2g16_ic16ih256oc16oh254kh3,"wip",I,cnvFwd-gemm:1sw1,0,1,8,106.414,0.349,0,3.7e+07
mb2g16_ic16ih256oc16oh254kh3,"wip",J,cjitConvFwd1q,0,9,8,0.567,65.526,0,3.7e+07
mb2g16_ic16ih256oc16oh254kh3,"wip",J,cjitConvFwd6,1,9,8,0.566,65.65,0,3.7e+07
mb2g16_ic16ih256oc16oh254kh3,"wip",J,mb-cjitConvFwd1q,0,9,8,6.333,5.868,0,3.7e+07
mb2g16_ic16ih256oc16oh254kh3,"wip",J,mb-cjitConvFwd6,0,9,8,6.866,5.412,0,3.7e+07
mb2g16_ic16ih256oc16oh254kh3,"wip",J,mb-unroll_cjitConvFwd1q,0,9,8,6.547,5.676,0,3.7e+07
mb2g16_ic16ih256oc16oh254kh3,"wip",J,mb-unroll_cjitConvFwd6,0,9,8,6.599,5.631,0,3.7e+07
mb2g16_ic16ih256oc16oh254kh3,"wip",J,unroll_cjitConvFwd1q,0,9,8,0.567,65.492,0,3.7e+07
mb2g16_ic16ih256oc16oh254kh3,"wip",J,unroll_cjitConvFwd6,0,9,8,0.567,65.574,0,3.7e+07
mb2g16_ic16ih256oc16oh254kh3,,R,gemm-Ref,0,1,8,3.345,11.108,0,3.7e+07
)EOF" ;

void test(){
    // This is a "friend" function, so we can proceed stepwise for the data
    // preprocessing of AllRunsxx.
    string s = s0;
    AllRunsxx ar(s);
    cout<<"\n\n freshly constructed dump()"<<endl;
    ar.dump();

    cout<<"\n\n ar.filter() of vrr.size() "<<ar.vrr.size()<<", expts.size() "<<ar.expts.size()<<endl;
    ar.filter(/*verbose*/false);
    ar.dump();

    if (0) { // for more detail
        cout<<"\n\n ar.massage_merge_dups() of vrr.size() "<<ar.vrr.size()<<", expts.size() "<<ar.expts.size()<<endl;
        ar.massage_merge_dups(/*verbose*/false);
        ar.dump();
    }

    cout<<"\n\n ar.massage() of vrr.size() "<<ar.vrr.size()<<", expts.size() "<<ar.expts.size()<<endl;
    ar.massage();
    ar.dump();

    cout<<"\n\n ar.group() of vrr.size() "<<ar.vrr.size()<<", expts.size() "<<ar.expts.size()<<endl;
    ar.group(/*verbose*/false);
    ar.dump(/*raw*/true);
    ar.dump(); // expts[] dump

    cout<<"========================================="<<endl;
    s = s9; // a longer test.  Let's skip the preprocessing details
    cout<<"\ns9 vrr construct..."<<endl;
    AllRunsxx ar9(s);
    cout<<"s9 vrr group..."<<endl;
    ar9.group();
    cout<<"s9 vrr dump"<<endl;
    ar9.dump(/*raw*/true); // vrr dump
    cout<<"\n s9 vrr dump"<<endl;
    ar9.dump(); // expts[] dump

    // print full list of features
    auto n = allFeature_sz;
    cout<<"Full list of "<<n<<" features:";
    for(int i=0; i<n; ++i){
        cout<<(i%5==0? "\n    ": "    ")<<setw(15)<<allFeature_name(i);
        assert( allFeature_name(i) == allFeature_names[i] );
    }
    cout<<endl;

}
void test2()
{
    cout<<"\ntest2 BEGIN"<<endl;
    string s = s9; // a longer test.  Let's skip the preprocessing details
    cout<<s9<<endl;
    AllRunsxx ar9(s);
    ar9.group();
    ar9.dump();
    if(1) {
        // Check default-alloc+construction
        ExptFloat *pef = ar9.exptFloat_alloc();
        ExptFloat& ef = *pef;
        cout<<"ef={"<<ef.c<<","<<ef.w<<","<<(void*)ef.expt<<","<<(void*)ef.y<<"}"<<endl;
#if 0 // initially, expt all zero
        for(int i=0; i<ef.c; ++i){
            cout<<"expt["<<i<<"]={";
            for(int j=0; j<ef.w; ++j) cout<<' '<<ef.expt[i*ef.w+j];
            cout<<"}"<<endl;
        }
        cout<<"y={";
        for(int j=0; j<ef.w; ++j) cout<<' '<<ef.y[j];
        cout<<"}"<<endl;
#endif
        ar9.exptFloat_free(pef);
    }
    cout<<"\ntest2 END"<<endl;
}
void test3()
{
    cout<<"\ntest2 BEGIN"<<endl;
    string s = s9; // a longer test.  Let's skip the preprocessing details
    cout<<s9<<endl;
    AllRunsxx ar9(s);
    ar9.group();
    ar9.dump();
    if(1) { // go through Expt data
        int ops_mismatch = 0;
        double eps = 0.03; // csv printout has approximate values!
        for(int i=0; i<ar9.expt_sz(); ++i){
            Expt const* e = ar9.expt(i);
            double ops_ef = e->ops;
            ConvParamRaw const* c = &e->c;
            double ops_count = count_ops(c);
            //
            // Initially, I found some deviations arising from the low
            // precision of .csv printout.  So I changed csv2example.cpp
            // "massage" to recalculate exact op counts.
            //
            if (ops_count > 0.0) {
                bool print = false;
                double rel_err = (ops_ef - ops_count) / ops_count;
                if (rel_err < -eps || rel_err > +eps) {
                    ++ops_mismatch;
                    cout<<"OHOH, ";
                    print = true;
                }else if (ops_ef != ops_count){
                    cout<<"INFO: ";
                    print = true;
                }
                if(print){
                    cout<<"expt["<<i<<"] ef.ops = "<<ops_ef
                            <<", but count_ops(&ef.c) gives "<<ops_count
                            <<" rel_err="<<rel_err
                            <<" %-diff of "<<100.*(rel_err-1.0)
                            <<endl;
                }
            }
        }
        if (ops_mismatch)
            assert("op count error!"==nullptr);
        else
            cout<<"\nGood: op counts (csv file) were consistent with exact"
                    " counts, within "<<1e-2*(int)(eps*1e4)<<" %\n"<<endl;
    }

    if(1) {
        ExptFloat *pef = ar9.exptFloat_alloc();
        ExptFloat& ef = *pef;

        size_t exptnum = 0;
        ar9.exptFloat(exptnum, pef /*, "norm_gf"*/);
        cout<<"exptnum="<<exptnum<<", params "<<ar9.getFactorParam()[exptnum]<<endl;
        cout<<"ef={"<<ef.c<<","<<ef.w<<","<<(void*)ef.expt<<","<<(void*)ef.y<<"}"<<endl;
        cout<<"expt["<<ef.w<<"]={";
        for(int j=0; j<ef.w; ++j) cout<<' '<<ef.expt[j];
        cout<<"}"<<endl;
        cout<<"\ny(norm_gf)["<<ef.c<<"] max_gf="<<ef.max_gf<<"\n  ={";
        for(int j=0; j<ef.c; ++j) cout<<' '<<ef.y[j];
        cout<<"}"<<endl;
#if 0 // oops, private
        ar9.get_y(exptnum, ef.y, "gf"); // switch to Gflops raw measurement
#else
        ar9.exptFloat(exptnum, pef, "gf");
#endif
        cout<<"y( Gflops)["<<ef.c<<"] max_gf="<<ef.max_gf<<"\n  ={";
        for(int j=0; j<ef.c; ++j) cout<<' '<<ef.y[j];
        cout<<"}"<<endl;
        ar9.exptFloat_free(pef);
    }
    cout<<"\ntest2 END"<<endl;
}
int main(int,char**){
    test();
    test2();
    test3();
    cout<<"\nGoodbye"<<endl;
}
// vim: et ts=4 sw=4 cindent cino=+=)50,+2s,^=lg0,\:0,N-s,E-s syntax=cpp.doxygen
