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

Fl6test::Fl6test(FusedLoopKernel& krn, int const v/*=0,verbose*/)
: FusedLoopTest(krn, "Fl6test", v)
{
    // upper-level tree structure, above the fused-loop.
    pr.root["first"];                           // reserve room for preamble, comments
    auto& inc = pr.root["includes"];
    inc["veintrin.h"]>>"#include \"veintrin.h\"";
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

    auto& inner = loop2; // inner path outer.at("**/loop2/body")
    inner["first"]; // --> 'fd' inner setup code used by all splits

    if(v>1){
        cout<<"\nScaffold";
        cout<<pr.tree()<<endl;
    }

    this->outer_ = &fl6test;  // outer @ root.at("**/fl6test/body")
    this->inner_ = &inner;    // inner @ outer.at("**/loop2/body")

    if(v>2){
        cout<<" outer @ "<<outer_->fullpath()<<endl;
        cout<<" inner @ "<<inner_->fullpath()<<endl;
    }
}
/* emit kernel (comment/code).
 *
 * Consider loops for(i=ilo..ihi)for(j=jlo..jhi)
 *
 * - if ilo==0, this behaves 'as usual'
 *   - \c cnt matches sequential values from \c a[0]*jj+b[0]
 *   - corresponds nicely with "output pixel", e.g.
 * - otherwise, we instead loop with ii=ihi-ilo, cnt'=0..(ihi-ilo)*jj
 *     - expected 'output pixel' values are then (ilo+a[])*jj+b[]
 *     - only when vl0%jj==0 does \c cnt' have an easy relation with
 *       \c a[]*jj+b[], because here a[]=const=cnt'%vl0.
 *     - typically \c i*jj+j must be calculated with vector ops
 *       as a[]*jj+b[]
 *
 * CHECK kernel should receive ilo and jlo to allow testing.
 * Given cnt', then i'=cnt'/jj and j'=cnt'%jj.
 * These correspond to i=ilo+i' and j=jlo+j',
 * so we expect a[i]=ilo+(cnt'+i)/jj
 * and          b[i]=jlo+(cnt'+i)%jj
 *
 *
 * \p bKrn   code block for kernel
 * \p bDef   code block for defines (within an enclosing-scope)
 *           We may output to bDef.getRoot()["<STAR><STAR>/includes"]
 *           function calls may go to bDef["..<STAR>/fns/first"]
 * \p bOut   code block for kernel output (post-fused-loop)
 *
 * Notes:
 *
 * any re-usable data of the kernel should \e somehow
 * hoist such registers to our enclosing scope (... and maybe further)
 * Ex 1:  if b[] is const, A*b[]+C vector can be hoisted outside loops.
 *        (actually will later supply a loop-fuse that ALSO optimizes
 *         a generic lin.comb(a[]*A+b[]*B+C) for the inner loop)
 * Ex 2:  if a mask register is a function of a const b[] vector,
 *        and we have found b[] to be const (except for vl changes),
 *        the mask register can be hoisted to enclosing scope (outside loops)
 *        (perhaps a significant saving)
 * Ex 2:  sq register can be hoisted (AND combined with our sq?)
 *        instead of being recalculated
 */
void cf5_kernel(Cblock& bKrn, Cblock& bDef, Cblock& bOut,
        int64_t const ilo, int64_t const ii,
        int64_t const jlo, int64_t const jj,
        int64_t const vl,
        std::string extraComment,
        int const which/*=0*/ /*comment,VecHash2*/,
        std::string pfx/*="fuse2_"*/,
        int const v/*=0*/ /*verbose*/,
        string vA/*="a"*/, string vB/*="b"*/,
        string vSEQ0/*="sq"*/, string sVL/*="vl"*/
        ){

    if( which==KERNEL_NONE ){ // just an optional extraComment
        bKrn["beg"]>>OSSFMT("// KERNEL("<<vA<<"["<<vl<<"],"<<vB<<"["<<vl<<"])");
        if(!extraComment.empty()) bKrn["beg"]<<extraComment;
    }else if( which==KERNEL_HASH ){
        std::string vh2 = pfx + "VecHash2";
        // constants moved upwards to beginning of scope enclosing bDef
        auto& bDefConst = bDef["..*/first"];
        // state variables at end of bDef
        auto& bDefState = bDef["last"]["vechash"];
        if(bDefState.code_str().empty()){
            string vSeq = (vSEQ0.empty()? "_ve_vseq_v()": vSEQ0);
            VecHash2::kern_C_begin(bDefConst, vSeq.c_str(), vl);
            auto instr = OSSFMT("int64_t "<<vh2<<" = 0;");
            bDefState>>OSSFMT(left<<setw(40)<<instr)
                <<" // vh2({a,b}) hash output";
            auto& bInc = bDef.getRoot()["**/includes"];
            if(!bInc.find("stdio.h")) bInc["stdio.h"]>>"#include <stdio.h>";
            if(!bOut.find("out_once")){
                bOut>>"printf(\"jit "<<vh2<<" = %llu\\n\",(long long unsigned)"<<vh2<<");";
                bOut["out_once"].setType("TAG");
            }
        }
#if 0 // original: inline, many statements
        bKrn["beg"]>>OSSFMT("// KERNEL("<<vA<<"["<<vl<<"],"<<vB<<"["<<vl<<"])");
        if(!extraComment.empty()) bKrn["beg"]<<" "<<extraComment;
        VecHash2::kern_C(bKrn[OSSFMT(pfx<<"_VecHash2")],vA,vB,sVL,vh2);
#else // NEW: as macro
        if(!bDefConst.find("have_vechash2_kernel")){
            auto m = VecHash2::kern_C_macro("VECHASH2_KERNEL");
            bDefConst.define(m.first,m.second);
            bDefConst["have_vechash2_kernel"].setType("TAG");
        }
        auto instr = OSSFMT("VECHASH2_KERNEL("<<vA<<","<<vB<<","<<sVL<<","<<vh2<<");");
        auto node = OSSFMT(pfx<<"_VecHash2");
        bKrn[node]>>OSSFMT(left<<setw(40)<<instr
                    <<" // "<<vA<<"["<<vl<<"],"<<vB<<"["<<vl<<"] "<<extraComment);
#endif
    }else if( which==KERNEL_PRINT ){
        // XXX Does clang have any option to pass __vr efficiently ?
        auto& bInc = bDef.getRoot()["**/includes"];
        if(!bInc.find("stdio.h")) bInc["stdio.h"]>>"#include <stdio.h>";
        auto& bDefKernelFn = bDef["..*/fns/first"];
        if(bDefKernelFn.find("cf5_kernel_print")==nullptr){
            CBLOCK_SCOPE(cf5_kernel_print,
                    "void cf5_kernel_print(__vr const a, __vr const b,"
                    "\n        uint64_t const vl)",bDefKernelFn.getRoot(),bDefKernelFn);
            cf5_kernel_print
                >>"int const terse=1;"
                >>"char const* linesep=\"\\n      \";"
                >>"printf(\"cf5_kernel_print(a,b,vl=%lu)\\n\",vl);"
                >>"printf(\"a[%3llu]={\",(long long unsigned)vl);"
                >>"for(int i=0;i<vl;++i){"
                >>"    if(terse && vl>=16) linesep=(vl>16 && i==7? \" ...\": \"\");"
                >>"    if(terse && vl>16 && i>=8 && i<vl-8) continue;"
                >>"    printf(\"%3llu%s\",\n"
                >>"      (long long unsigned)_ve_lvs_svs_u64(a,i),"
                >>"      (i%16==15? linesep: \" \")); }"
                >>"printf(\"}\\n\");"
                >>"linesep=\"\\n   \";"
                >>"printf(\"b[%3llu]={\",(long long unsigned)vl);"
                >>"for(int i=0;i<vl;++i){"
                >>"    if(terse && vl>=16) linesep=(vl>16 && i==7? \" ...\": \"\");"
                >>"    if(terse && vl>16 && i>=8 && i<vl-8) continue;"
                >>"    printf(\"%3llu%s\",\n"
                >>"      (long long unsigned)_ve_lvs_svs_u64(b,i),"
                >>"      (i%16==15? linesep: \" \")); }"
                >>"printf(\"}\\n\");"
                ;
        }
        bKrn["prt"]<<"cf5_kernel_print("<<vA<<", "<<vB<<", "<<sVL<<");";
        if(!extraComment.empty()) bKrn["prt"]<<" // "<<extraComment;
        if(!bOut.find("out_once")){
            bOut>>"printf(\"cfuse KERNEL_PRINT done!\\n\");";
            bOut["out_once"].setType("TAG");
        }
    }else if( which==KERNEL_CHECK ){
        // as function call (KERNEL_CHECK does not need speed)
        int const v=0; // verbose?
        bool const as_function=true; // true makes asm readable, but clang can bug
        auto& bInc = bDef.getRoot()["**/includes"];
        if(!bInc.find("stdio.h")) bInc["stdio.h"]>>"#include <stdio.h>";
        if(!bInc.find("assert.h")) bInc["assert.h"]>>"#include <assert.h>";
        if(as_function){
            auto& bDefKernelFn = bDef["..*/fns/first"];
            if(bDefKernelFn.find("cf5_kernel_check")==nullptr){
                CBLOCK_SCOPE(cf5_kernel_check,
                        "void "
                        "\n__attribute__((noinline))"
                        "\ncf5_kernel_check(__vr const a, __vr const b, uint64_t const vl,"
                        "\n        uint64_t const cnt, uint64_t const jj,"
                        "\n        uint64_t const ilo, uint64_t const jlo)"
                        ,
                        bDefKernelFn.getRoot(),bDefKernelFn);
#if 0
                cf5_kernel_check
                    >>"int v="<<(v?"1;":"(vl<=0?1:0); // clang bug?")
                    >>"if(v){ printf(\"cf5_kernel_check vl=%d cnt=%d jj=%d ilo=%d jlo=%d\\n\",(int)vl,(int)cnt,(int)jj,(int)ilo,(int)jlo);">>"    fflush(stdout); }"
                    >>"for(uint64_t i=0;i<vl;++i){"
                    >>"    if(v){ printf(\"expect a[%lu]=%lu b[%lu]=%lu\\n\",i,ilo+(cnt+i)/jj,i,jlo+(cnt+i)%jj);">>"        fflush(stdout); }"
                    >>"    assert( _ve_lvs_svs_u64(a,i) == ilo+(cnt+i)/jj );"
                    >>"    assert( _ve_lvs_svs_u64(b,i) == jlo+(cnt+i)%jj );"
                    >>"}"
                    ;
#else
                if(v>=1) cf5_kernel_check
                    >>"printf(\"cf5_kernel_check vl=%d cnt=%d jj=%d ilo=%d jlo=%d\\n\",(int)vl,(int)cnt,(int)jj,(int)ilo,(int)jlo);"
                        >>"fflush(stdout);";
                cf5_kernel_check
                    >>"for(uint64_t i=0;i<vl;++i){"
                    >>"    int64_t const a_i = _ve_lvs_svs_u64(a,i);"
                    >>"    int64_t const b_i = _ve_lvs_svs_u64(b,i);"
                    >>"    int const aok = (a_i == ilo+(cnt+i)/jj);"
                    >>"    int const bok = (b_i == jlo+(cnt+i)%jj);";
                cf5_kernel_check
                    >>(v>=2?"    if(1){": "    if( !aok || !bok ) {")
                    >>"        printf(\"expect a[%lu]=%lu b[%lu]=%lu but got %ld %ld\\n\","
                    >>"               i, ilo+(cnt+i)/jj, i, jlo+(cnt+i)%jj, a_i,b_i);"
                    >>"        fflush(stdout);"
                    >>"    }";
                cf5_kernel_check
                    >>"    assert( aok );"
                    >>"    assert( bok );"
                    >>"}";
#endif
            }
            string call=OSSFMT("cf5_kernel_check("<<vA<<","<<vB<<","<<sVL<<",cnt,jj,ilo,jlo);");
            bKrn["chk"]<<OSSFMT(left<<setw(40)<<call<<" // "<<extraComment);
        }else{ // inline  (try to avoid clang bugs with arg-passing)
            auto& chk=bKrn["chk"];
            chk>>OSSFMT("for(uint64_t "<<pfx<<"_i=0; "<<pfx<<"_i<"<<sVL<<"; ++"<<pfx<<"_i){");
            if(v){ chk>>OSSFMT(""
                    <<"\n    int64_t a_i=_ve_lvs_svs_u64("<<vA<<","<<pfx<<"_i)"
                    <<            ", b_i=_ve_lvs_svs_u64("<<vB<<","<<pfx<<"_i);"
                    <<"\n    printf(\"i=%lu got %ld %ld expect %lu %lu\\n\", "<<pfx<<"_i"<<",a_i,b_i, ilo+(cnt+"<<pfx<<"_i)/jj, jlo+(cnt+"<<pfx<<"_i)/jj );");
            }
            chk>>OSSFMT("    assert( _ve_lvs_svs_u64("<<vA<<","<<pfx<<"_i) == ilo+(cnt+"<<pfx<<"_i)/jj );");
            chk>>OSSFMT("    assert( _ve_lvs_svs_u64("<<vB<<","<<pfx<<"_i) == jlo+(cnt+"<<pfx<<"_i)%jj );");
            chk>>"}";
        }
        if(!bOut.find("out_once")){
            bOut>>"assert((uint64_t)cnt==(iijj+vl0-1)/vl0*vl0);"
                >>"printf(\"cfuse KERNEL_CHECK done! no errors\\n\");"
                >>"fflush(stdout);";
            bOut["out_once"].setType("TAG");
        }
    }else if( which==KERNEL_SQIJ ){
        bKrn["beg"]>>OSSFMT("// KERNEL("<<vA<<"["<<vl<<"],"<<vB<<"["<<vl<<"],sqij="<<vA<<"*jj+"<<vB<<")");
        bKrn["prt"]>>"__vr const x = STORE(0, _ve_addul_vsv(ptr,_ve_vmulul_vsv(stride,sqij)));";
        if(!extraComment.empty()) bKrn["prt"]<<" // "<<extraComment;
    }else{
        THROW(OSSFMT("unknown kernel type "<<which<<" in "<<__FUNCTION__));
    }
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
        FusedLoopKernel *krn = nullptr;
        switch(which){
          case(KERNEL_NONE):    {krn = new FLKRN_none(); break;}
          case(KERNEL_HASH):    {krn = new FLKRN_hash(); break;}
          case(KERNEL_PRINT):   {krn = new FLKRN_print(); break;}
          case(KERNEL_CHECK):   {krn = new FLKRN_check(); break;}
          case(KERNEL_SQIJ):    {krn = new FLKRN_sqij(); break;}
          default:              {krn = new FLKRN_none(); break;}
        }
        assert( krn != nullptr );
        assert( krn->vA     == "a" );
        assert( krn->vB     == "b" );
        assert( krn->vSEQ0  == "sq" );
        assert( krn->sVL    == "vl" );
        assert( krn->vSQIJ  == "sqij" );
        try{
            if(maxun==0){ // no unroll...
#if 0
                fl6_no_unrollX(vl,h,w,opt_t,which,ofname);
#else
                fl6_no_unrollY(h,w,*krn,vl,ofname,verbosity);
#endif
            }else{ // unroll...
#if 0
                fl6_unrollX(vl,h,w,maxun,opt_t,which,ofname);
#else
                fl6_unrollY(h,w,maxun,*krn,vl,ofname,verbosity);
#endif
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
        delete krn;
    }
    delete[] ofname;
    if(nerr==0) cout<<"\nGoodbye"<<endl;
    else /*  */ cout<<"\nOHOH"<<endl;
    return nerr? -1: 0;
}
// vim: ts=4 sw=4 et cindent cino=^=l0,\:.5s,=-.5s,N-s,g.5s,b1 cinkeys=0{,0},0),\:,0#,!^F,o,O,e,0=break
