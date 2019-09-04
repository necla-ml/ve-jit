#include "fl6-kernels.hpp"
#include "../stringutil.hpp"
#include "../vechash.hpp"
using namespace std;
using namespace cprog;

static char const* kernel_names_default[] = {
    "NONE","HASH","PRINT","CHECK","SQIJ" };
static struct KernelNeeds const kernel_needs_default[] = {
    { .cnt=0, .iijj=0, .sq=0, .sqij=0, .vl=0 }, // KERNEL_NONE          dummy, to look at code
    { .cnt=0, .iijj=0, .sq=1, .sqij=1, .vl=1 }, // KERNEL_HASH          not recently used
    { .cnt=0, .iijj=0, .sq=0, .sqij=0, .vl=1 }, // KERNEL_PRINT
    { .cnt=1, .iijj=1, .sq=0, .sqij=0, .vl=1 }, // KERNEL_CHECK
    { .cnt=0, .iijj=0, .sq=0, .sqij=1, .vl=0 }  // KERNEL_SQIJ          for testing?
};

extern "C" {
    struct KernelNeeds kernel_needs(int const which){
        assert( which >= 0 && which <= KERNEL_SQIJ );
        return kernel_needs_default[which];
    }
    char const* kernel_name(int const which){
        assert( which >= 0 && which <= KERNEL_SQIJ );
        return kernel_names_default[which];
    }
}//extern "C"

std::ostringstream FusedLoopKernelAbs::oss = std::ostringstream{};

struct KernelNeeds FLKRN_none::needs() const{ return kernel_needs_default[KERNEL_NONE]; }
struct KernelNeeds FLKRN_hash::needs() const{ return kernel_needs_default[KERNEL_HASH]; }
struct KernelNeeds FLKRN_print::needs() const{ return kernel_needs_default[KERNEL_PRINT]; }
struct KernelNeeds FLKRN_check::needs() const{ return kernel_needs_default[KERNEL_CHECK]; }
struct KernelNeeds FLKRN_sqij::needs() const{ return kernel_needs_default[KERNEL_SQIJ]; }

FusedLoopKernel* mkFusedLoopKernel(int const which,
        cprog::Cblock& outer, cprog::Cblock& inner,
        FLVARS_CONSTRUCTOR_ARGS2){
    FusedLoopKernel *krn = nullptr;
#define FLKRN_ARGS outer,inner,vA,vB,vSEQ0,sVL,vSQIJ
    switch(which){
      case(KERNEL_NONE):    {krn = new FLKRN_none (FLKRN_ARGS); break;}
      case(KERNEL_HASH):    {krn = new FLKRN_hash (FLKRN_ARGS); break;}
      case(KERNEL_PRINT):   {krn = new FLKRN_print(FLKRN_ARGS); break;}
      case(KERNEL_CHECK):   {krn = new FLKRN_check(FLKRN_ARGS); break;}
      case(KERNEL_SQIJ):    {krn = new FLKRN_sqij (FLKRN_ARGS); break;}
      default:              {krn = new FLKRN_none (FLKRN_ARGS); break;}
    }
#undef FLKRN_ARGS
    return krn;
}
void FLKRN_check::emit(Cblock& bDef,
        Cblock& bKrn, Cblock& bOut,
        int64_t const ilo, int64_t const ii,
        int64_t const jlo, int64_t const jj,
        int64_t const vl, std::string extraComment, int const v/*=0, verbose*/
        ) const{
    //int const v=0; // verbose?
    // as function call (KERNEL_CHECK does not need speed)
    bool const as_function=true; // true makes asm readable, but clang can bug

    auto& bInc = bDef.getRoot()["**/includes"];
    if(!bInc.find("stdio.h")) bInc["stdio.h"]>>"#include <stdio.h>";
    if(!bInc.find("assert.h")) bInc["assert.h"]>>"#include <assert.h>";
    if(!bInc.find("stdlib.h")) bInc["stdlib.h"]>>"#include <stdlib.h>";
    if(as_function){
        auto& bDefKernelFn = bDef["..*/fns/first"];
        if(bDefKernelFn.find("fl6_kernel_check")==nullptr){
            CBLOCK_SCOPE(fl6_kernel_check,
                    "void "
                    "\n__attribute__((noinline))"
                    "\nfl6_kernel_check(__vr const a, __vr const b, uint64_t const vl,"
                    "\n        uint64_t const cnt, uint64_t const jj,"
                    "\n        uint64_t const ilo, uint64_t const jlo)"
                    ,
                    bDefKernelFn.getRoot(),bDefKernelFn);
#if 0
            fl6_kernel_check
                >>"int v="<<(v?"1;":"(vl<=0?1:0); // clang bug?")
                >>"if(v){ printf(\"fl6_kernel_check vl=%d cnt=%d jj=%d ilo=%d jlo=%d\\n\",(int)vl,(int)cnt,(int)jj,(int)ilo,(int)jlo);">>"    fflush(stdout); }"
                >>"for(uint64_t i=0;i<vl;++i){"
                >>"    if(v){ printf(\"expect a[%lu]=%lu b[%lu]=%lu\\n\",i,ilo+(cnt+i)/jj,i,jlo+(cnt+i)%jj);">>"        fflush(stdout); }"
                >>"    assert( _vel_lvsl_svs(a,i) == ilo+(cnt+i)/jj );"
                >>"    assert( _vel_lvsl_svs(b,i) == jlo+(cnt+i)%jj );"
                >>"}"
                ;
#else
            if(v>=1) fl6_kernel_check
                >>"printf(\"fl6_kernel_check vl=%d cnt=%d jj=%d ilo=%d jlo=%d\\n\",(int)vl,(int)cnt,(int)jj,(int)ilo,(int)jlo);"
                    >>"fflush(stdout);";
            fl6_kernel_check
                >>"for(uint64_t i=0;i<vl;++i){"
                >>"    int64_t const a_i = _vel_lvsl_svs(a,i);"
                >>"    int64_t const b_i = _vel_lvsl_svs(b,i);"
                >>"    int const aok = (a_i == ilo+(cnt+i)/jj);"
                >>"    int const bok = (b_i == jlo+(cnt+i)%jj);";
            fl6_kernel_check
                >>(v>=2?"    if(1){": "    if( !aok || !bok ) {")
                >>"        printf(\"expect a[%lu]=%lu b[%lu]=%lu and got %ld %ld\\n\","
                >>"               i, ilo+(cnt+i)/jj, i, jlo+(cnt+i)%jj, a_i,b_i);"
                >>"        fflush(stdout);"
                >>"    }";
            fl6_kernel_check
                >>"    if(!aok){printf(\" error: %s a[] wrong\\n\",__FILE__);}"
                >>"    if(!bok){printf(\" error: %s b[] wrong\\n\",__FILE__);}"
                >>"     if(!aok || !bok) exit(-1);"
                >>"}";
#endif
        }
        string call=OSSFMT("fl6_kernel_check("<<vA<<","<<vB<<","<<sVL<<",cnt,jj,ilo,jlo);");
        bKrn["chk"]<<OSSFMT(left<<setw(40)<<call<<" // "<<extraComment);
    }else{ // inline  (try to avoid clang bugs with arg-passing)
        auto& chk=bKrn["chk"];
        chk>>OSSFMT("for(uint64_t "<<pfx<<"_i=0; "<<pfx<<"_i<"<<sVL<<"; ++"<<pfx<<"_i){");
        if(v){ chk>>OSSFMT(""
                <<"\n    int64_t a_i=_vel_lvsl_svs("<<vA<<","<<pfx<<"_i)"
                <<            ", b_i=_vel_lvsl_svs("<<vB<<","<<pfx<<"_i);"
                <<"\n    printf(\"i=%lu got %ld %ld expect %lu %lu\\n\", "<<pfx<<"_i"<<",a_i,b_i, ilo+(cnt+"<<pfx<<"_i)/jj, jlo+(cnt+"<<pfx<<"_i)/jj );");
        }
        chk>>OSSFMT("    assert( _vel_lvsl_svs("<<vA<<","<<pfx<<"_i) == ilo+(cnt+"<<pfx<<"_i)/jj );");
        chk>>OSSFMT("    assert( _vel_lvsl_svs("<<vB<<","<<pfx<<"_i) == jlo+(cnt+"<<pfx<<"_i)%jj );");
        chk>>"}";
    }
    if(!bOut.find("out_once")){
        bOut>>"assert((uint64_t)cnt==(iijj+vl0-1)/vl0*vl0);"
            >>"printf(\"cfuse KERNEL_CHECK done! no errors\\n\");"
            >>"fflush(stdout);";
        bOut["out_once"].setType("TAG");
    }
}

void FLKRN_none::emit(Cblock& bDef,
        Cblock& bKrn, Cblock& bOut,
        int64_t const ilo, int64_t const ii,
        int64_t const jlo, int64_t const jj,
        int64_t const vl, std::string extraComment, int const v/*=0, verbose*/
        ) const{
    bKrn["beg"]>>OSSFMT("// KERNEL("<<vA<<"["<<vl<<"],"<<vB<<"["<<vl<<"])");
    if(!extraComment.empty()) bKrn["beg"]<<extraComment;
}
void FLKRN_hash::emit(Cblock& bDef,
        Cblock& bKrn, Cblock& bOut,
        int64_t const ilo, int64_t const ii,
        int64_t const jlo, int64_t const jj,
        int64_t const vl, std::string extraComment, int const v/*=0, verbose*/
        ) const{
    {
        // constants moved upwards to beginning of scope enclosing bDef
        auto& bDefConst = bDef["..*/first"];
        string vSeq = (vSEQ0.empty()? OSSFMT("_vel_vseq_vl("<<vl<<")"): vSEQ0);
        VecHash2::kern_C_begin(outer,inner,bDefConst, vSeq.c_str(), vl);
        if(!bDefConst.find("have_vechash2_kernel")){
            auto m = VecHash2::kern_C_macro("VECHASH2_KERNEL");
            bDefConst.define(m.first,m.second);
            bDefConst["have_vechash2_kernel"].setType("TAG");
        }
    }
    //std::string vh2 = pfx + "VecHash2";
    std::string vh2 = "vh2_hash"; // new kern_C_begin !
    // state variables at end of bDef
    auto& bDefState = bDef["last"]["vechash"];
    if(bDefState.code_str().empty()){
        //auto instr = OSSFMT("int64_t "<<vh2<<" = 0;");
        //bDefState>>OSSFMT(left<<setw(40)<<instr)
        //    <<" // vh2({a,b}) hash output";
        auto& bInc = bDef.getRoot()["**/includes"];
        if(!bInc.find("stdio.h")) bInc["stdio.h"]>>"#include <stdio.h>";
        if(!bOut.find("out_once")){
            bOut>>"printf(\""<<pfx<<" jit "<<vh2<<" = %llu\\n\""
                ",(long long unsigned)"<<vh2<<");";
            bOut["out_once"].setType("TAG");
        }
    }
#if 0 // original: inline, many statements
    bKrn["beg"]>>OSSFMT("// KERNEL("<<vA<<"["<<vl<<"],"<<vB<<"["<<vl<<"])");
    if(!extraComment.empty()) bKrn["beg"]<<" "<<extraComment;
    VecHash2::kern_C(bKrn[OSSFMT(pfx<<"_VecHash2")],vA,vB,sVL,vh2);
#else // NEW: as macro
    auto instr = OSSFMT("VECHASH2_KERNEL("<<vA<<","<<vB<<","<<sVL<<","<<vh2<<");");
    auto node = OSSFMT(pfx<<"_VecHash2");
    bKrn[node]>>OSSFMT(left<<setw(40)<<instr
            <<" // "<<vA<<"["<<vl<<"],"<<vB<<"["<<vl<<"] "<<extraComment);
#endif
}
void FLKRN_print::emit(Cblock& bDef,
        Cblock& bKrn, Cblock& bOut,
        int64_t const ilo, int64_t const ii,
        int64_t const jlo, int64_t const jj,
        int64_t const vl, std::string extraComment, int const v/*=0, verbose*/
        ) const{
    // XXX Does clang have any option to pass __vr efficiently ?
    auto& bInc = bDef.getRoot()["**/includes"];
    if(!bInc.find("stdio.h")) bInc["stdio.h"]>>"#include <stdio.h>";
    auto& bDefKernelFn = bDef["..*/fns/first"];
    if(bDefKernelFn.find("fl6_kernel_print")==nullptr){
        CBLOCK_SCOPE(fl6_kernel_print,
                "void fl6_kernel_print(__vr const a, __vr const b,"
                "\n        uint64_t const vl)",bDefKernelFn.getRoot(),bDefKernelFn);
        fl6_kernel_print
            >>"int const terse=1;"
            >>"char const* linesep=\"\\n      \";"
            >>"printf(\"fl6_kernel_print(a,b,vl=%lu)\\n\",vl);"
            >>"printf(\"a[%3llu]={\",(long long unsigned)vl);"
            >>"for(int i=0;i<vl;++i){"
            >>"    if(terse && vl>=16) linesep=(vl>16 && i==7? \" ...\": \"\");"
            >>"    if(terse && vl>16 && i>=8 && i<vl-8) continue;"
            >>"    printf(\"%3lld%s\",\n"
            >>"      (long long signed)_vel_lvsl_svs(a,i),"
            >>"      (i%16==15? linesep: \" \")); }"
            >>"printf(\"}\\n\");"
            >>"linesep=\"\\n   \";"
            >>"printf(\"b[%3llu]={\",(long long unsigned)vl);"
            >>"for(int i=0;i<vl;++i){"
            >>"    if(terse && vl>=16) linesep=(vl>16 && i==7? \" ...\": \"\");"
            >>"    if(terse && vl>16 && i>=8 && i<vl-8) continue;"
            >>"    printf(\"%3lld%s\",\n"
            >>"      (long long signed)_vel_lvsl_svs(b,i),"
            >>"      (i%16==15? linesep: \" \")); }"
            >>"printf(\"}\\n\");"
            ;
    }
    bKrn["prt"]<<"fl6_kernel_print("<<vA<<", "<<vB<<", "<<sVL<<");";
    if(!extraComment.empty()) bKrn["prt"]<<" // "<<extraComment;
    if(!bOut.find("out_once")){
        bOut>>"printf(\"cfuse KERNEL_PRINT done!\\n\");";
        bOut["out_once"].setType("TAG");
    }
}
void FLKRN_sqij::emit(Cblock& bDef,
        Cblock& bKrn, Cblock& bOut,
        int64_t const ilo, int64_t const ii,
        int64_t const jlo, int64_t const jj,
        int64_t const vl, std::string extraComment, int const v/*=0, verbose*/
        ) const{
    bKrn["beg"]>>OSSFMT("// KERNEL("<<vA<<"["<<vl<<"],"<<vB<<"["<<vl<<"],sqij="<<vA<<"*jj+"<<vB<<")");
    bKrn["prt"]>>OSSFMT("__vr const x = STORE(0, _vel_addul_vsvl(ptr,_vel_vmulul_vsvl(stride,sqij,"<<vl<<"),"<<vl<<"));");
    if(!extraComment.empty()) bKrn["prt"]<<" // "<<extraComment;
}
// vim: ts=4 sw=4 et cindent cino=^=l0,\:.5s,=-.5s,N-s,g.5s,b1 cinkeys=0{,0},0),\:,0#,!^F,o,O,e,0=break
