/* Copyright (c) 2019 by NEC Corporation
 * This file is part of ve-jit */
/** \file
 * fl6 is a code cleanup of cf5
 *
 * quick test:
 * ```
 * ./fl.sh fl6 15 3:6:9:12 9 CHECK 0 >& fl6.log
 * ```
 */
#include "fl6.hpp"
#include "../fuseloop.hpp"
#include "../ve_divmod.hpp"
#include "exechash.hpp"
#include "../vechash.hpp"
#include "../stringutil.hpp"
#include "../throw.hpp" // THROW(stuff to right of cout<<), adding fn and line number
#include "../cblock.hpp" // Now we are producing 'C + intrinsics' code, for clang

#include <functional>
#include <list>
#include <vector>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <algorithm> // ve finds std::max here!
#include <unordered_set>
#include <map>
#include <array>
#include <regex>

#include <cstring>
#include <cstddef>
#include <cassert>

#ifndef FOR
#define FOR(I,VL) for(int I=0;I<VL;++I)
#endif

#define DEF(VAR) def(#VAR,VAR)

#define SAVE_RESTORE_VLEN 0

using namespace std;
using namespace loop;
using namespace cprog;

static ostringstream oss;

Fl6test::Fl6test(int const which, int const v/*=0,verbose*/)
: FusedLoopTest(which, "Fl6test", v) // pr set up, other ptrs still NULL
{
    // upper-level tree structure, above the fused-loop.
    pr.root["first"];                           // reserve room for preamble, comments
    auto& inc = pr.root["includes"];
    inc["velintrin.h"]>>"#include \"veintrin.h\"";
    inc["velintrin.h"]>>"#include \"velintrin.h\"";
    inc["stdint.h"]>>"#include <stdint.h>";
    inc["stdio.h"]>>"#include <stdio.h>";

    // create a somewhat generic tree
    auto& fns = pr.root["fns"];
    fns["first"]; // reserve a node for optional function definitions
    CBLOCK_SCOPE(fl6test,"int main(int argc,char**argv)",pr,fns);
    // NEW: use tag function scope, for upward-find operations
    //      will require extending the find string to match tag!
    fl6test.setType("FUNCTION");
    // outer path root.at("**/fl6test/body")

    // example 'outer loops'
    fl6test>>"int const nrep = 3;";
    CBLOCK_FOR(loop1,-1,"for(int iloop1=0; iloop1<nrep; ++iloop1)",fl6test);
    CBLOCK_FOR(loop2,-1,"for(int iloop2=0; iloop2<1; ++iloop2)",loop1);

    if(v){
        fl6test["first"]>>"printf(\"fl6 outer loop\\n\");";
        loop2  >>"printf(\"fl6 inner loop @ (%d,%d)\\n\",iloop1,iloop2);";
    }

    loop2["first"]; // --> 'fd' inner setup code used by all splits

    if(v>1){
        cout<<"\nScaffold";
        cout<<pr.tree()<<endl;
    }

    this->outer_ = &fl6test;  // outer @ root.at("**/fl6test/body")
    this->inner_ = &loop2;    // inner @ outer.at("**/loop2/body")

    if(v>2){
        cout<<" outer @ "<<outer_->fullpath()<<endl;
        cout<<" inner @ "<<inner_->fullpath()<<endl;
    }

    this->krn_ = mkFusedLoopKernel(which,*outer_,*inner_ /*, defaults*/ );
}
/** How many full/partial \c vl-sized loops are implied by vl,ii,jj?
 * \return \c nloops so for(0..ii)for(0..jj) vectorizes as for(0..nloops).
 *
 * - Example: use for loop splitting:
 * - In: iilo,iihi  vl,ii,jj  Out: cntlo,cnthi
 *   - s.t. index vectors a_c[i]  i<vl, c=0..nloops-1
 *     satisfy a_c[i]<ilo for c in [0,cntlo)
 *   - (and a_c[i]<ihi for c in [0,cnthi)
 * - cntlo,cnthi can be set via:
 *   - `nloops = cnt_loops(vl,ii,jj);`
 *   - `cntlo = cnt_loops(vl,iilo,jj);`
 *   - `cnthi = cnt_loops(vl,iihi,jj);`
 * - and split loop is for(0..cntlo), for(cntlo,cnthi), for(cnthi,nloops)
 */
uint64_t constexpr cnt_loops(uint64_t const vl,
        uint64_t const ii, uint64_t const jj){
    //auto const iijj = ii*jj;            //iijj = serialized upper limit
    //auto const nloops = (iijj+vl-1)/vl; //iijj involves how many vl?
    //auto const ijlt = iilt*jj;              //ijlt = serialized iilt value
    //auto const ijlt_loops = (ijlt+vl-1)/vl; //ijlt involves how many vl?
    //return ijlt_loops;
    return (ii*jj+vl-1U)/vl;
}

void cf5_help_msg(){
    cout<<" cf5 -[h|t|a]* [-kSTR] [-oFILE] VLEN I J"
        <<"\n Function:"
        <<"\n double loop --> loop over vector registers a[VLEN], b[VLEN]"
        <<"\n Allow both loops to split off some first/last passes"
        <<"\n Output optimized VE C+intrinsics code for loop fusion."
        <<"\n JIT options:"
        <<"\n  -t     [default] VE intrinsics code"
        <<"\n  -a     use lower-vl alt strategy if can speed induction"
        <<"\n         - suggested for Aurora VLEN=256 runs"
        <<"\n  -uN    N=max unroll Ex. -tu8ofile for unrolled version of -tofile"
        <<"\n"
        <<"\n  -kSTR  kernel type: [CHECK]|NONE|HASH|PRINT|SQIJ"
        <<"\n  -oFILE output code filename"
        <<"\n         - Ex. -ofile-vi.c for clang-vector-i|trinsics code"
        <<"\n         - TODO cf5.sh or cf5u.sh to compile & run JIT on VE"
        <<"\n  -v[v...] verbosity [0]"
        <<"\n Parameters:"
        <<"\n   VLEN          vector length"
        <<"\n   [z:[l:[h:]]]I z<=l<=h<=I : 1st loops a=z..I (l,h TBD, l==z,h==I for now)"
        <<"\n   [z:[l:[h:]]]J z<=l<=h<=J : 2nd loops b=z..J (l,h TBD, l==z,h==I for now)"
        <<"\n  -h             this help"
        <<endl;
}
int main(int argc,char**argv){
    int vl = 8;
    struct LoopSplit h={0,0,20,20}, w={0,0,3,3};
    int opt_t=2, opt_h=0;
    int a=0;
    uint32_t maxun=0U;
    char *ofname = nullptr;
    int which = WHICH_KERNEL;
    int verbosity=0;

    if(argc > 1){
        // actually only the last -[tlu] option is used
        for( ; a+1<argc && argv[a+1][0]=='-'; ++a){
            char *c = &argv[a+1][1];
            cout<<" arg : "<<c<<endl;
            for( ; *c != '\0'; ++c){
                if(*c=='h'){ cf5_help_msg(); opt_h = 1;
                }else if(*c=='v'){ ++verbosity;
                }else if(*c=='t'){ opt_t=2;
                }else if(*c=='a'){ opt_t=3;
                }else if(*c=='o'){
                    size_t len = strlen(++c);
                    ofname = new char[len+1];
                    strncpy(ofname,c,len+1);
                    break;
                }else if(*c=='u'){
                    maxun=0U;
                    while(isdigit(*(c+1))) maxun = maxun*10U+(uint32_t)(*++c-'0');
                    break;
                }else if(*c=='k'){
                    std::string kern=string(++c);
                    if(kern=="NONE") which=KERNEL_NONE;
                    else if(kern=="HASH") which=KERNEL_HASH;
                    else if(kern=="PRINT") which=KERNEL_PRINT;
                    else if(kern=="CHECK") which=KERNEL_CHECK;
                    else if(kern=="SQIJ") which=KERNEL_HASH;
                    else{
                        cout<<"-kKERN "<<c<<" not supported (stays at "<<kernel_name(which)<<")"<<endl;
                    }
                    break;
                }
            }
        }
    }
    cout<<" args: a = "<<a<<" opt_t="<<opt_t<<" ofname = "
        <<(ofname? ofname: "none")<<" which="<<kernel_name(which)
        <<" maxun="<<maxun<<" verbosity "<<verbosity<<endl;
    if(argc > a+1) vl = atof(argv[a+1]);
    if(argc > a+2) h  = parseLoopSplit(argv[a+2]);
    if(argc > a+3) w  = parseLoopSplit(argv[a+3]);
    cout<<"vlen="<<vl<<", h="<<h<<", w="<<w<<endl;
    assert(opt_t==2 || opt_t==3);

    uint32_t nerr=0U;
    if(opt_h == 0){
        Fl6test fl6t{which,max(0,verbosity-2)};
        if(1){
            auto& krn = fl6t.krn();
            assert( krn.vA     == "a" );
            assert( krn.vB     == "b" );
            assert( krn.vSEQ0  == "sq" );
            assert( krn.sVL    == "vl" );
            assert( krn.vSQIJ  == "sqij" );
        }
        try{
            if(maxun==0){ // no unroll...
                fl6_no_unrollY(h,w,fl6t,vl,ofname,verbosity);
            }else{ // unroll...
                fl6_unrollY(h,w,maxun,fl6t,vl,ofname,verbosity);
            }
        }
        catch(exception& e){
            cout<<"Exception: "<<e.what()<<endl;
            ++nerr;
        }
        catch(...){
            cout<<"Unknown exception"<<endl;
            ++nerr;
        }
    }
    delete[] ofname;
    if(nerr==0) cout<<"\nGoodbye"<<endl;
    else /*  */ cout<<"\nOHOH"<<endl;
    return nerr? -1: 0;
}
// vim: ts=4 sw=4 et cindent cino=^=l0,\:.5s,=-.5s,N-s,g.5s,b1 cinkeys=0{,0},0),\:,0#,!^F,o,O,e,0=break
