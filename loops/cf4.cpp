/* Copyright (c) 2019 by NEC Corporation
 * This file is part of ve-jit */
/** \file
 * VE clang JIT for loop fusion, cf3 extended to loops with both inner and outer scope
 *
 * inner and outer scope for these tests are provided by an outer loop(0..NTESTS=3)
 * that has been added to the 'main' of the JITted test code.
 *
 * quick test:
 * ```
 * { (cd .. && make) && make cf4 && ./cf4 -kCHECK -toa-vi.c && make a-ve -f ../bin.mk && ./a-ve; } >& x.log
 * ```
 */
#include "cf3.hpp"
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

extern "C" {
struct KernelNeeds kernel_needs(int const which){
    KernelNeeds ret={0,0,0,0,0};
    switch(which){
      case(KERNEL_NONE): break;
      case(KERNEL_HASH): ret.vl=1; ret.sq=1; ret.sqij=1; break;
      case(KERNEL_PRINT): ret.vl=1; break;
      case(KERNEL_CHECK): ret.cnt=1; ret.vl=1; ret.iijj=1; break;
      case(KERNEL_SQIJ): ret.sqij=1; break;
      default: cout<<" Warning: unhandled KERNEL type, which="<<which<<endl;
    }
    return ret;
}

char const* kernel_name(int const which){
    return (which==KERNEL_NONE? "NONE"
            :which==KERNEL_HASH? "HASH"
            :which==KERNEL_PRINT? "PRINT"
            :which==KERNEL_CHECK? "CHECK"
            :which==KERNEL_SQIJ? "SQIJ"
            :"HUH");
}

}//extern "C"

void other_fastdiv_methods(int const jj);

// /** scope init for AsmFmtCols */
// typedef std::list<std::pair<std::string,std::string>> AsmScope;

#include "divide_by_constants_codegen_reference.c"
#if 0 // C++17:
template<typename... Args>
std::string join_sep[](char const sep, Args&&... args){
    std::ostringstream oss;
    std::string ret( ((oss << args << sep), ...).str() );
    //               ---------------------
    //               C++17 fold expression, expands to
    //               (oss<<arg1<<sep),(oss<<arg2<<sep), ...
    if(!ret.empty()) ret.pop_back();
    return ret;
}
#endif

void other_fastdiv_methods(int const jj){
    int const verbose=1;
    // other fast divide approaches...
    if(verbose>=1 && !positivePow2(jj)){
        // libdivide relies on MULHI operation, which we don't have. It sometimes needs
        // more ops, but for Aurora would be correct for larger (32-bit) input range.
        magicu_info bogus = {0,0,0,0};
        assert( sizeof(uint)*CHAR_BIT == 32 );
        auto const ld = positivePow2(jj) ? bogus: compute_unsigned_magic_info( jj, 32 );
        if( ld.pre_shift==0 ){
            // NO assert( ld.post_shift==1 ); sometimes 1 or 2
            // NO  assert( ld.post_shift==0 || ld.post_shift==1 || ld.post_shift==2 );
            if( ld.post_shift==0 ){
                // never happens?
                cout<<" --> no pre or post-shift, increment="<<ld.increment
                    <<", mul="<<(void*)(intptr_t)(intptr_t)(intptr_t)ld.multiplier;
            }else{
                cout<<" --> no pre-shift, post-shift="<<ld.post_shift
                    <<",increment="<<ld.increment
                    <<", mul="<<(void*)(intptr_t)ld.multiplier;
            }
        }else{
            cout<<" OH?? ";
            cout<<" --> pre-shift="<<ld.pre_shift<<", post-shift="<<ld.post_shift
                <<",increment="<<ld.increment
                <<", mul="<<(void*)(intptr_t)ld.multiplier;
        }
    }
    if(verbose>=1){
        // Note: Aurora has shift-LEFT-add but no mul-add or shift-right-add for int vectors
        struct fastdiv jj_fastdiv;
        fastdiv_make( &jj_fastdiv, (uint32_t)jj );
        cout<<endl<<"\t"
            <<" mul,add,shr="<<(void*)(intptr_t)jj_fastdiv.mul
            <<","<<jj_fastdiv.add<<","<<jj_fastdiv.shift;
    }
    if(verbose>=1){
        Ulpi jj_mod_inverse_lpi   = mod_inverse((Ulpi)jj);
        Uvlpi jj_mod_inverse_Vlpi = mod_inverse((Uvlpi)jj);
        cout<<" jj_modinv="<<(void*)(intptr_t)jj_mod_inverse_Vlpi
            <<" or "<<(void*)(intptr_t)jj_mod_inverse_lpi;
    }
}

/* emit kernel (comment/code).
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
void fuse4_kernel(Cblock& bOuter, Cblock& bDef,
        Cblock& bKrn, Cblock& bOut,
        int64_t const ii, int64_t const jj, int64_t const vl,
        std::string extraComment,
        int const which/*=0*/ /*comment,VecHash2*/,
        std::string pfx/*="fuse2_"*/,
        int const v/*=0*/ /*verbose*/,
        string vA/*="a"*/, string vB/*="b"*/,
        string vSEQ0/*="sq"*/, string sVL/*="vl"*/
        ){
    static ostringstream oss;

    if( which==KERNEL_NONE ){ // just an optional extraComment
        bKrn["beg"]>>OSSFMT("// KERNEL("<<vA<<"["<<vl<<"],"<<vB<<"["<<vl<<"])");
        if(!extraComment.empty()) bKrn["beg"]<<extraComment;
    }else if( which==KERNEL_HASH ){
        std::string vh2 = pfx + "VecHash2";
        // constants moved upwards to beginning of scope enclosing bDef
        auto& bDefConst = bDef["..*/first"];
        // state variables at end of bDef
        auto& bDefState = bDef["last"]["vechash"];
        //cout<<"bDef @ "<<bDef.fullpath()<<endl;
        //cout<<"bDefConst @ "<<bDefConst.fullpath()<<endl;
        //cout<<"bDefState @ "<<bDefState.fullpath()<<endl;
        if(bDefState.code_str().empty()){
            string vSeq = (vSEQ0.empty()? "_ve_vseq_v()": vSEQ0);

            VecHash2::kern_C_begin(bOuter,bDefConst,bDefConst, vSeq.c_str(), vl);
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
            auto m = VecHash2::kern_C_macro("VECHASH4_KERNEL");
            bDefConst.define(m.first,m.second);
            bDefConst["have_vechash2_kernel"].setType("TAG");
        }
        auto instr = OSSFMT("VECHASH4_KERNEL("<<vA<<","<<vB<<","<<sVL<<","<<vh2<<");");
        auto node = OSSFMT(pfx<<"_VecHash2");
        bKrn[node]>>OSSFMT(left<<setw(40)<<instr
                    <<" // "<<vA<<"["<<vl<<"],"<<vB<<"["<<vl<<"] "<<extraComment);
#endif
    }else if( which==KERNEL_PRINT ){
        // XXX Does clang have any option to pass __vr efficiently ?
        bool const nice_vector_register_args = true;
        if( !nice_vector_register_args ){ // inlined...
            auto& bInc = bDef.getRoot()["**/includes"];
            if(!bInc.find("stdio.h")) bInc["stdio.h"]>>"#include <stdio.h>";
            bKrn["beg"]>>OSSFMT("// KERNEL("<<vA<<"["<<vl<<"],"<<vB<<"["<<vl<<"])");
            if(!extraComment.empty()) bKrn["beg"]<<" "<<extraComment;
            bKrn["prt"]>>"printf(\"a={\");"
                >>"for(int i=0;i<"<<sVL<<";++i){ printf(\"%llu%s\","
                >>"    (long long unsigned)_ve_lvs_svs_u64("<<vA<<",i),"
                >>"    (i%8==0? \"\\n   \":\" \")); }"
                >>"printf(\"}\\n\");"
                >>"printf(\"b={\");"
                >>"for(int i=0;i<"<<sVL<<";++i){ printf(\"%llu%s\","
                >>"    (long long unsigned)_ve_lvs_svs_u64("<<vB<<",i),"
                >>"    (i%8==0? \"i\\n   \":\" \")); }"
                >>"printf(\"}\\n\");"
                ;
        }else{ // as function call (KERNEL_PRINT does not need speed)
            auto& bInc = bDef.getRoot()["**/includes"];
            if(!bInc.find("stdio.h")) bInc["stdio.h"]>>"#include <stdio.h>";
            auto& bDefKernelFn = bDef["..*/fns/first"];
            if(bDefKernelFn.find("cfuse4_kernel_print")==nullptr){
                CBLOCK_SCOPE(cfuse4_kernel_print,
                        "void cfuse4_kernel_print(__vr const a, __vr const b,"
                        "\n        uint64_t const vl)",bDefKernelFn.getRoot(),bDefKernelFn);
                cfuse4_kernel_print
                    >>"int const terse=1;"
                    >>"char const* linesep=\"\\n      \";"
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
            bKrn["prt"]<<"cfuse4_kernel_print("<<vA<<", "<<vB<<", "<<sVL<<");";
            if(!extraComment.empty()) bKrn["prt"]<<" // "<<extraComment;
        }
        if(!bOut.find("out_once")){
            bOut>>"printf(\"cfuse KERNEL_PRINT done!\\n\");";
            bOut["out_once"].setType("TAG");
        }
    }else if( which==KERNEL_CHECK ){
        // as function call (KERNEL_PRINT does not need speed)
        auto& bInc = bDef.getRoot()["**/includes"];
        if(!bInc.find("stdio.h")) bInc["stdio.h"]>>"#include <stdio.h>";
        if(!bInc.find("assert.h")) bInc["assert.h"]>>"#include <assert.h>";
        auto& bDefKernelFn = bDef["..*/fns/first"];
        if(bDefKernelFn.find("cfuse4_kernel_check")==nullptr){
            CBLOCK_SCOPE(cfuse4_kernel_check,
                    "void "
                    "\n__attribute__((noinline))"
                    "\ncfuse4_kernel_check(__vr const a, __vr const b,"
                    "\n        uint64_t const cnt, uint64_t const vl, uint64_t const jj)"
                    ,
                    bDefKernelFn.getRoot(),bDefKernelFn);
            cfuse4_kernel_check
                >>"for(uint64_t i=0;i<vl;++i){"
                >>"    assert( _ve_lvs_svs_u64(a,i) == (cnt+i)/jj );"
                >>"    assert( _ve_lvs_svs_u64(b,i) == (cnt+i)%jj );"
                >>"}"
                ;
        }
        bKrn["prt"]<<"cfuse4_kernel_check("<<vA<<", "<<vB<<", cnt, "<<sVL<<", jj);";
        if(!extraComment.empty()) bKrn["prt"]<<" // "<<extraComment;
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
        THROW(OSSFMT("unknown kernel type "<<which<<" in fuse4_kernel"));
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

// for r in [0,h){ for c in [0,w] {...}
void test_vloop2_no_unrollX(Lpi const vlen, Lpi const ii, Lpi const jj,
        int const opt_t, int const which/*=WHICH_KERNEL*/,
	char const* ofname/*=nullptr*/)
{
    ostringstream oss; // reusable allocation, for CBLK and OSSFMT macros
    ExecHash tr;
    //CodeGenAsm cg;
    //std::string code;
    int const b_period_max = 8; // how many regs can you spare?

    // This is pinned at [max] vl, even if it may be "inefficient".
    auto u = unroll_suggest( vlen,ii,jj, b_period_max );
    int vl = u.vl;
    auto uAlt = unroll_suggest(u);  // suggest an alternate nice vector length, sometimes.

    cout<<"\nUnrolls:"<<str(u,"\nOrig: ")<<endl;
    if(uAlt.suggested==UNR_UNSET) cout<<"Alt:  "<<name(uAlt.suggested)<<endl;
    else cout<<str(uAlt,"\nAlt:  ")<<endl;
    cout<<endl;

    if(opt_t==3){ // we FORCE the alternate strategy (if it exists)
        if(u.vll) // equiv uAlt.suggested != UNR_UNSET
        {
            assert( uAlt.suggested != UNR_UNSET );
            cout<<" (forcing alt. strategy)"<<endl;
            //vl = u.vll; unroll_suggest(u);
            u = uAlt;
            vl = u.vl;
        }else{
            cout<<" (no really great alt.strategy)"<<endl;
        }
    }
    int const vl0 = vl;
    cout<<__FUNCTION__<<" using vl0="<<vl<<endl;
    other_fastdiv_methods(jj);

    {
        VecHash2 vhash(vl);
        int verbose=1;
        assert( vl > 0 );
        REGISTER uint64_t iijj = (uint64_t)ii * (uint64_t)jj;
        cout<<"fuse2 -t "<<setw(4)<<vl<<" "<<setw(4)<<ii<<" "<<setw(4)<<jj
            <<"   # for(0.."<<ii<<") for(0.."<<jj<<") --> a[VL],b[VL] for VL<="<<vl<<endl;

        //if (cnt+vl > iijj) vl = iijj - cnt;  // simplifies for cnt=0
        if ((uint64_t)vl > iijj) vl = iijj;
        int const nloop = (iijj+vl-1) / vl;    // div_round_up(iijj,vl)
        assert( (nloop > 1) == ((uint64_t)vl < iijj) );
        assert( vl0 > 0 );
        assert( ii > 0 );
        assert( jj > 0 );

        cout<<" Using "<<u.suggested<<" for vl,ii,jj="
            <<vl<<","<<ii<<","<<jj<<" unroll("<<u.unroll<<")"
            <<" [Alt vll="<<u.vll<<"]"<<endl;
        // Include the suggested strategy into the exechash, for future
        // productions that do unrolling (want better coverage)
        {
            std::ostringstream oss;
            oss<<"strategy "<<u.suggested;
            tr+=oss.str();
        }

        //cout<<"Verify-------"<<endl;
        // generate reference index outputs
        std::vector<Vab> vabs = ref_vloop2(vl, ii, jj, 0/*verbose*/);
        assert( vabs.size() > 0 );
        assert(vabs.size() == (size_t)(((ii*jj) +vl -1) / vl));
        if(verbose>=2) cout<<"   vl="<<vl<<"   ii="<<ii<<"   jj="<<jj
            <<"   nloop="<<nloop<<"   iijj="<<iijj
                <<" vabs.size() = "<<vabs.size()<<endl;
        // Have reference vabs vectors. Now we try induction way.
        // 1. initialize: could copy vabs[0] from const data storage, or...
        //   - generate from seq + divmod.
        //   - 2-loop induction uses 3 scalar registers:
        //     - \c cnt 0.. \c iijj, and \c vl (for jit, iijj is CCC (compile-time-const))
        //     - get final \c vl from cnt, vl and iij)
        //     - new: interpret cnt as "remain", going from iijj to one

//#define FOR(I,VL) for(int I=0;I<VL;++I)

        // various misc precalculated consts and declarations.
        VVlpi a(vl), b(vl);                 // calculated loop index vectors
        VVlpi bA(vl), bD(vl), sq(vl);       // internal vectors
        // bA and bD are used when:
        //   iijj > vl && jj%vl!=0
        // sq is used when:
        //   iloop==0:   jj>1 && vl>=jj
        //   iloop >0:   jj%vl==0 && (special: iloop can be >= (jj/vl))
        assert( nloop >= 1 );
        uint64_t const jj_M = computeM_uB(jj); // for fastdiv_uB method
        int jj_shift=0;
        Vlpi jj_minus_1 = 0;
        if( positivePow2(jj) ){
            jj_shift = positivePow2Shift((uint32_t)jj);
            jj_minus_1 = jj - 1;
            assert( (uint64_t)1<<jj_shift == (uint64_t)jj );
            cout<<" power of two shift is "<<jj_shift<<"    mask is "<<jj_minus_1;
        }else{
            cout<<" jj_M="<<(void*)(intptr_t)jj_M<<" shift="<<FASTDIV_C;
        }

#if 0
        // NB: common operation is divmod(v,s,vM,vD) : v--> v%s, v/s,
        //     which has some optimizations for nice values of jj.
        // C++14: &vl=std::as_const(vl)
        auto v_divmod_vs = [&vl,&jj,&jj_M](/* in*/ VVlpi const& a, Vlpi const d, /*out*/ VVlpi& div, VVlpi& mod){
            assert( (Ulpi)jj < FASTDIV_SAFEMAX ); FOR(i,vl) assert( (Uvlpi)a[i] <= FASTDIV_SAFEMAX );
            FOR(i,vl) div[i] = jj_M * a[i] >> FASTDIV_C;
            FOR(i,vl) mod[i] = a[i] - div[i]*jj;
        };
#endif

        // Using immediate values decreases register usage.
        // Even though VE asm ops can use certain values as immediate data,
        // Want auto mechanism to decide whether immediate is possible.
        //
        // have_FOO and cnt_FOO : FOO register usage condition and actual use count
        bool const have_vl_over_jj = nloop>1 && vl0%jj==0;
        bool const have_bA_bD = nloop>1 && vl%jj!=0 && jj%vl!=0;
        bool const have_jjMODvl_reset      = (vl0%jj!=0 && jj%vl0==0 && nloop >jj/vl0); // case 'g'
        bool const have_jjMODvl = (vl0%jj!=0 && jj%vl0==0 && nloop>=jj/vl0);
        if( 1 || have_jjMODvl ){
            cout<<" jj/vl0="<<jj/vl0<<endl;
            if( have_jjMODvl ) assert( jj/vl0 > 1 );
            if( have_jjMODvl_reset ) assert( have_jjMODvl );
        }
        bool const have_sq_init_block = jj!=1 && jj<vl0;
        bool const have_sq_indu_block = have_jjMODvl_reset;
        bool const have_sq = have_sq_init_block                             // when iloop==0
            || have_sq_indu_block;                                          // o/w
        bool const have_jj_shift = (jj!=1 && jj<vl0 && positivePow2(jj))    // when iloop==0
            || (nloop>1 && vl0%jj!=0 && jj%vl0!=0 && positivePow2(jj));     // o/w
        bool const have_jj_M = (jj>1 && jj<vl0 && !positivePow2(jj))        // when iloop==0
            || (nloop>1 && vl0%jj!=0 && jj%vl0!=0 && !positivePow2(jj));    // o/w

        int cnt_vl_over_jj=0, cnt_bA_bD=0, cnt_sq=0, cnt_jj_shift=0, cnt_jj_M=0;
        int cnt_jjMODvl=0, cnt_jjMODvl_reset=0, cnt_use_iijj=0;

        Vlpi const vl_over_jj  = have_vl_over_jj? vl0/jj: 0;
        Vlpi const jj_over_vl  = have_jjMODvl_reset? jj/vl0: 0;
        Vlpi tmod       = have_jjMODvl_reset? 0: 777; // used if have_jjMODvl_reset
        //uint64_t const jj_over_vl_M = (have_jjMODvl_reset? computeM_uB(jj_over_vl): 0);
        assert( !(have_vl_over_jj && have_jjMODvl_reset) ); // never need both these constants
        assert( !(have_jj_M && have_jjMODvl_reset) ); // never both
        //assert( !(have_jj_M && have_vl_over_jj) ); // HAPPENS ex: vl,ii,jj=9,4,3

        REGISTER uint64_t cnt = 0; // for loop counts 0 .. iijj-1

        auto kernComment = [&](){
            string def = (nloop<=1
                ? OSSFMT("sq[]=0.."<<ii*jj-1)
                : OSSFMT("cnt=a[0]*"<<jj<<"+b[0]="<<cnt));
            return def;
        };

        Cunit pr("cfuse2","C",0/*verbose*/);
        auto& inc = pr.root["includes"];
        auto& fns = pr.root["fns"];
        fns["first"]; // reserve a node so kernels can define functions
        CBLOCK_SCOPE(cfuse2,"int main(int,char**)",pr,fns["main"]);
        auto& fd = cfuse2["../first"];  //cfuse2["defines"];
        Cblock *ploop_ab=nullptr;
        {
            if(nloop<=1){
                ploop_ab = &cfuse2["loop"]["first"];
                bool const kernel_needs_cnt = which==KERNEL_CHECK;
                if(kernel_needs_cnt){
                    fd>>"int64_t const cnt=0; /*simulate for-loop index for kernel";
                }
            }else{
                //cout<<"ploop_ab, nloop="<<nloop<<endl;
                fd  >>"int64_t const iijj = (uint64_t)ii * (uint64_t)jj;"
                    <<OSSFMT(" // cnt to "<<ii<<"*"<<jj<<"="<<iijj<<" by "<<vl0);
                //cout<<"fd.str() = "<<fd.str()<<endl;
                //cout<<" setting loop_ab..."<<endl;
                CBLOCK_FOR(loop_ab,0,
                        "for(int64_t cnt=0; cnt<<iijj; cnt+=vl0)"
                        ,cfuse2["loop"]);
                ploop_ab = &loop_ab;
                //cout<<"ploop_ab is "<<ploop_ab->str()<<endl;
            }
        }
        Cblock& loop_ab = *ploop_ab;
        //auto& fp = loop_ab["../first"];
        //CBLOCK_SCOPE(fp, (nloop>1?"if(cnt==0)":""),pr,loop_ab["../first"]);
        CBLOCK_SCOPE(fp, "",pr,loop_ab["../first"]);
        cout<<"fp..beg="<<fp["../beg"].code_str()<<endl;
        auto& fi = loop_ab["last"];
        auto& fk = loop_ab["kernel"];
        //CBLOCK_SCOPE(fk,"",pr,fi);
        //auto& fl = cfuse2["last"]["first"];
        auto& fz = cfuse2["last"]["first"];

        inc >>"#include \"veintrin.h\""
            >>"#include \"stdint.h\""
            ;
        // local labels:
        string fusename = OSSFMT("fuse2_"<<vl<<"_"<<ii<<"_"<<jj);
        cfuse2
            //.def("KERNEL_BLOCK", "L_"+fusename+"_KERNEL_BLOCK")
            .DEF(vl0)
            .DEF(ii)
            .DEF(jj)
            ;

        //fd.scope(block,"vectorized double-loop --> index vectors");
        //+++++++++++++++++ constant registers +++++++++++++++++++
        //fd.ins("lvl vl0",                   "VL = vl0");
        fd>>OSSFMT("_ve_lvl(vl0);  // VL = "<<vl0);
        if(which==KERNEL_PRINT){
            if(!fd.find("have_vl")){
                fd  >>"int64_t vl = vl0;";
                fd["have_vl"].setType("TAG");
            }
        }

        if( have_sq ){
            FOR(i,vl) sq[i] = i;       // vseq_v
        }

        //+++++++++++++++++ fuse2 state registers +++++++++++++++++++
        if(SAVE_RESTORE_VLEN){
            fp>>"//int64_t vl_save = _ve_svl_s; // <--- this intrinsic was not there?";
        }
        verbose = 0; // cleans up the pseudocode output a bit
        // during loop, output pseudocode just once
        bool onceI = true; // induce-block code output at most once
        bool onceK = true; // kernel-block code output exactly most once
        bool onceL = true; // loop continueation code output exactly once
        // if no loop, some loop block are not required
        if(nloop==1){ onceI=false; onceL=false; } // no code for these blocks
        // TODO: some kernels may REQUIRE sqij[i] = a[i]*jj+b[i] "pixel index" vector too.
        //   and this has a trivial (1 add) update (no multiply needed).
        // TODO: some kernels may require fast inductive LINEAR COMBINATION,
        //   lin[i] = linA*a[i] + linB*b[i] + linC input,
        //   given jit constants linA, linB, linC,
        //   for which we sometimes can do better than brute-force calc
        // Ex. our loops are really for(i=I..I+ii)for(j=J..J+jj)
        //     or the loops represent "outer" tensor dimensions
        //     or we have some other (stride/dilation?) multiplying factors
        //        to map into a different tensor.
        bool def_sqij = false;

        string alg_descr;
        {
            std::ostringstream oss;
            oss<<" Fuse2 alg: vl,ii,jj="<<vl<<","<<ii<<","<<jj<<" nloop="<<nloop;
            if(vl0%jj==0) oss<<" vl%jj==0";
            else if(jj%vl==0) oss<<" jj%vl==0";
            else if(positivePow2(jj)) oss<<" jj=2^"<<jj_shift;
            alg_descr = oss.str();
        }
        fp>>"// "<<alg_descr;
        fp>>"//  "<<OSSFMT(u);
        fp>>OSSFMT("//  for(i=0.."<<ii<<"))for(j=0.."<<jj<<") --> __vr a, b of of i,j loop indices, VL="<<vl0);
        // We are writing
        //     ```for( ; cnt < iijj; cnt += vl )```
        //     NEW: ```for( ; cnt>0; cnt -= vl )```
        //          with gotos
        // PRECALC-BLOCK
        //  jit also puts the "first time through the loop" precalc into fp
        //  *before* the induce and kernel (fi,fk) blocks
        int iloop = 0;
        if(iloop==0) { // iloop==0 (cnt=0, or now iijj)
            //fp.lcom("INIT_BLOCK:");
            if(nloop<=1){
                fp.clear(); fp["../beg"].clear(); fp["../end"].clear();
            }
            fp>>OSSFMT("// INIT_BLOCK: nloop="<<nloop);
            if(nloop==1) assert(have_vl_over_jj==0);
            // now load the initial vector-loop registers:
            // sq[i] and jj are < FASTDIV_SAFEMAX, so we can avoid % and / operations
            std::string vREG=(nloop<=1?"__vr const ":"");
            if(nloop>1) fd>>"__vr a, b; // vector loop index registers";
            if( jj==1 ){
                tr+="init:iloop 1 jj==1";
                FOR(i,vl) a[i] = i;    // sq/jj   or perhaps change have_sq?
                FOR(i,vl) b[i] = 0;    // sq%jj
                assert(have_bA_bD==0); assert(have_sq==0); assert(have_jj_shift==0);
                fp>>vREG<<"a = _ve_vseq_v();         // a[i] = i";
                //fp>>vREG<<"b = _ve_vbrd_vs_i64(0LL); // b[i] = 0"; // is this as good as vxor?
                fp>>vREG<<"b; b=_ve_vxor_vvv(b,b);"; // typically "best way" for any CPU
            }else if(jj>=vl){
                tr+="init:iloop 1 jj>=vl";
                if(verbose)cout<<" b";
                FOR(i,vl) a[i] = 0;    // sq < vl, so sq/jj < 1
                FOR(i,vl) b[i] = i;
                if(nloop<=1) {assert(have_bA_bD==0); assert(have_sq==0); assert(have_jj_shift==0);}
                fp>>vREG<<"a = _ve_vbrd_vs_i64(0LL); // a[i] = 0";
                fp>>vREG<<"b = _ve_vseq_v();         // b[i] = i;";
            }else if( positivePow2(jj) ){
                tr+="init:iloop 1 pow2jj";
                if(verbose)cout<<" c";
                // 2 ops (shr, and)
                FOR(i,vl) a[i] = (sq[i] >> jj_shift);  // bD = bA / jj; div_vsv
                FOR(i,vl) b[i] = (sq[i] & jj_minus_1); // bM = bA % jj; mod_vsv
                if(nloop<=1) assert(have_bA_bD==0); assert(have_sq==1); assert(have_jj_shift==1);
                ++cnt_sq; ++cnt_jj_shift;
                //fp.ins("vsrl.l.zx a, sq,"+jitdec(jj_shift), "a[i]=sq[i]/jj =sq[i]>>"+jitdec(jj_shift));
                //fp.ins("vand.l    b, "+jitimm(jj_minus_1)+",sq", "b[i]=sq[i]%jj =sq[i] & (2^"+jitdec(jj_shift)+"-1)");
                //fd.DEF(jj_shift);
                //fp>>vREG<<"a = _ve_vsrl_vvs(sq, jj_shift); // a[i]=sq[i]/jj = sq[i]>>"<<jitdec(jj_shift);
                //fp>>vREG<<"b = _ve_vand_vsv("<<jithex(jj-1)<<",sq);       // b[i]=sq[i]%jj";
                //fp["last"].set(OSSFMT("DIVMOD_"<<jj<<"(a,b,sqij);"));
                fd.define(OSSFMT("DIVMOD_"<<jj<<"(A,B,SQIJ)"),
                        OSSFMT("A = _ve_vsrl_vvs(SQ, "<<jj_shift<<"); \\\n" // a[i]=sq[i]/jj
                            "                           B = _ve_vand_vsv("<<jithex(jj-1)<<",SQ)")); // b[i]=sq[i]%jj
                fp>>OSSFMT("DIVMOD_"<<jj<<"("<<vREG<<"a, "<<vREG<<"b, sq);");
            }else{
                tr+="init:iloop 1 fastdiv";
                if(verbose)cout<<" d";
                // 4 int ops (mul,shr, mul,sub)
                //v_divmod_vs( sq, jj, /*sq[]/jj*/a, /*sq[]%jj*/b );
                FOR(i,vl) a[i] = jj_M * sq[i] >> FASTDIV_C;
                FOR(i,vl) b[i] = sq[i] - a[i]*jj;
                //  OK since sq[] and jj both <= FASTDIV_SAFEMAX [(1<<21)-1]
                assert( (uint64_t)jj+vl <= (uint64_t)FASTDIV_SAFEMAX );
                // use mul_add_shr (fastdiv) approach if jj+vl>FASTDIV_SAFEMAX (1 more vector_add_scalar)
                if(nloop<=1) assert(have_bA_bD==0); assert(have_sq==1); assert(have_jj_shift==0);
                ++cnt_sq; ++cnt_jj_M;
                fp  >>vREG<<"a = FASTDIV_"<<asDec(jj)<<"(sq);                           // a[i] = sq[i]/jj"
                    >>vREG<<"b = _ve_vsubul_vvv(sq,_ve_vmulul_vsv(jj, a)); // b[i] = sq[i] - jj*a[i]";
            }
        }
        if(onceI){
            tr+="rel addr(BASE:+base register";
        }
        //cout<<" fp.cnt="<<cnt;
        goto KERNEL_BLOCK;
        // INDUCE-BLOCK
INDUCE:
        assert( nloop > 1 );
        if( iijj % vl0 ){ // special loop condition case
            tr+="induce:iijj%vl0 cnt update";
            // JIT does this within 'for' statement... this code is using goto style
            cnt += vl0; // (or vl, to guarantee hitting iijj exactly)
            //if(onceI) fi.ins("addu.l cnt, vl0, cnt", "cnt 0..iijj-1");
            //cout<<" fi.cnt="<<cnt;
            tr+="induce:iijj%vl0 last iter EARLY lower vl";
            // Careful: vl = new value for last iteration, vl0 = original = old value
            // VE has single-op MIN
            //vl = std::min( (uint64_t)vl, /*remain =*/ iijj - cnt );
            vl = std::min( (uint64_t)vl, /*remain =*/ iijj-cnt );
            if(onceI){
                if(!fd.find("have_vl")){
                    fd  >>"int64_t vl = vl0;";
                    fd["have_vl"].setType("TAG");
                }
                fi  >>"// (ii*jj)%vl0 != 0 --> last iter VL change"
                    >>"vl = (vl0<iijj-cnt? vl0: iijj-cnt); // vl = min(vl0,iijj-cnt)"
                    >>"_ve_lvl(vl);";
            }
        }
        // 2. Induction from a->ax, b->bx
        if(vl0%jj == 0){  // avoid div,mod -----1 vec op
            tr+="induce:vl0%jj a[]+=vl/jj, b[] const";
            // this includes cases with b_period==1 and high nloops
            if(verbose){cout<<" e";cout.flush();}
            FOR(i,vl) a[i] = a[i] + vl_over_jj;
            ++cnt_vl_over_jj; assert(have_vl_over_jj); assert(!have_bA_bD);
            //if(onceI && STYLE==STYLE_GOTO && fi[".."].code_str().empty() ) fi[".."]>>"INDUCE:    // vl0%jj==0";
            if(onceI){
                // VADD vx, vy/sy/I, vz [,vm]
                //fi.ins("vadd a, "+(vl/jj>127?"vlojj":jitdec(vl/jj))+",a"
                //        ,        "a[i] += (vl/jj="+jitdec(vl0/jj)+"), b[] unchanged");
                if(vl/jj>127){
                    fd>>OSSFMT("int64_t const vlojj = "<<vl0/jj<<"; // vl/jj="<<vl<<"/"<<jj);
                }else{
                    fd.define("vlojj",OSSFMT(vl0/jj<<"/*vl/jj*/"));
                }
                fi>>OSSFMT("a =_ve_vadduw_vsv(vlojj, a);"
                        <<" // a[i] += (vl/jj="<<vl0/jj<<"), b[] unchanged");
            }
        }else if(jj%vl0 == 0 ){  // -------------1 or 2 vec op (conditional)
            // unroll often nice w/ have_b_period (with maybe more regs)
            assert( have_bA_bD==0); assert(have_jj_shift==0); assert(have_vl_over_jj==0);
            assert( jj > vl0 ); assert( jj/vl0 > 1 ); assert( have_jjMODvl );
            if( !have_jjMODvl_reset ){
                tr+="induce:jj%vl0 A: b[]+=vl0, a[] const";
                // Note: this case should also be a "trivial" case for unroll suggestion
                if(verbose){cout<<" f";cout.flush();}
                FOR(i,vl) b[i] = b[i] + vl0;
                ++cnt_jjMODvl; assert( have_jjMODvl );
                //if(onceI && STYLE==STYLE_GOTO && fi[".."].code_str().empty() ) fi[".."]>>"INDUCE:    // jj%vl0 == 0";
                if(onceI) //fi.ins("add b, "+jitdec(vl0)+",b",    "b[i] += vl0, a[] const");
                /**/ CBLK(fi,"b = _ve_vadduw_vsv("<<jitdec(vl0)<<",b); // b[i] += vl0, a[] const");
            }else{
                // This case is potentially faster with a partial precalc unroll
                // The division should be done with compute_uB
                assert( have_jjMODvl && have_jjMODvl_reset );
                // recall: have_jjMODvl_reset [HERE] is
                // (vl0%jj!=0 && jj%vl0==0 && nloop >jj/vl0), so nloop > jj/vl0 > 1
                // ==> Cannot optimize loop exit by querying the "reset" condition.
#if 0
                //Lpi easy = iloop % jj_over_vl;       // scalar cyclic mod
                Lpi easy = fastmod_uB( iloop, jj_over_vl_M, jj_over_vl );
                // #pragma..unroll(jj/vl0) could be branchless
                // can be optimized further into 3 minimal-op cases
                if( easy ){                         // bump b[i], a[i] unchanged
                    if(verbose){cout<<" f";cout.flush();}
                    FOR(i,vl) b[i] = b[i] + vl0;
                    ++cnt_jjMODvl; assert( have_jjMODvl );
                }else{                              // RESET b[i], bD[i]==1
                    if(verbose){cout<<" g";cout.flush();}
                    FOR(i,vl) b[i] = sq[i];
                    FOR(i,vl) a[i] = a[i] + 1;
                    ++cnt_sq; assert( have_sq==1 );
                    ++cnt_jjMODvl_reset;
                }
#else
                // iloop % jj_over_vl is implemented as a cyclic [0,jj/vl0) count
                if(jj/vl0==2){                          // cyclic period 2 counter
                    tr+="induce:jj%vl0 case: tmod toggle";
                    assert(jj/vl0==2);
                    tmod = 1-tmod;
                }else if(positivePow2(jj/vl0)){         // cyclic power-of-2 counter
                    tr+="induce:jj%vl0 tmod shift,and";
                    uint64_t const shift = positivePow2Shift((uint32_t)(jj/vl0));
                    uint64_t const mask  = (1ULL<<shift) - 1U;
                    tmod = (tmod+1U) & mask;
                }else{                                  // cyclic period jj/vl0 counter
                    tr+="induce:jj%vl0 tmod cmov";
                    tmod += 1;
                    int64_t const tmp = tmod - jj_over_vl;
                    if(tmp==0) tmod=tmp; // cmov tmod,tmp,tmp
                }

                if( tmod ){
                    tr+="induce:jj%vl0 tmod: b[]+=vl0";
                    if(verbose){cout<<" f";cout.flush();}
                    FOR(i,vl) b[i] = b[i] + vl0;
                    ++cnt_jjMODvl; assert( have_jjMODvl );
                }else{                              // RESET b[i], bD[i]==1
                    tr+="induce:jj%vl0 !tmod (reset): b[]=0,1,..., and a[]+=1";
                    if(verbose){cout<<" g";cout.flush();}
                    FOR(i,vl) b[i] = sq[i];
                    FOR(i,vl) a[i] = a[i] + 1;
                    ++cnt_sq; assert( have_sq==1 );
                    ++cnt_jjMODvl_reset;
                }
#endif
                if(onceI){
                    fd>>OSSFMT("int64_t tmod = 0; // tmod=0, cycling < (jj/vl="
                            <<jj/vl<<")");
                    CBLK(fi,"// jj%vl0==0 : iloop%(jj/vl0) check via cyclic tmod < jj/vl0="<<jj/vl0);
                    if(jj/vl0==2){
                        //if(onceI && STYLE==STYLE_GOTO && fi[".."].code_str().empty() ) fi[".."]>>"INDUCE:    // period jj/vl0=2";
                        fi>>"tmod=~tmod;";
                    }else if(positivePow2(jj/vl0)){     // cyclic power-of-2 counter
                        //if(onceI && STYLE==STYLE_GOTO && fi[".."].code_str().empty() ) fi[".."]>>"INDUCE:    // period jj/vl0=2^N";
                        uint64_t const shift = positivePow2Shift((uint32_t)(jj/vl0));
                        uint64_t const mask  = (1ULL<<shift) - 1U;
                        CBLK(fi,"tmod = (tmod+1) & "<<jithex(mask)<<"; // cyclic power-of-2 counter");
                    }else{                              // cyclic period jj/vl0 counter
                        //if(onceI && STYLE==STYLE_GOTO && fi[".."].code_str().empty() ) fi[".."]>>OSSFMT("INDUCE:    // period jj/vl0="<<jj/vl0);
                        fi  >>OSSFMT("++tmod;               // tmod period jj/vl0 = "<<jj/vl0)
                            >>OSSFMT("if(tmod=="<<setw(3)<<jj/vl0<<") tmod=0; // should generate cmov reset tmod=0?");
                    }
                    //fi.ins("beq.w.t tmod,"+reladdr("RESET"));
                    //fi.ins("vadd b, b,vl0",             "b[i] += vl0 (easy, a[] unchanged)")
                    //    .ins("b "+reladdr("INDUCE_DONE"), "branch around reset case");
                    //fi.lab("RESET")
                    //    .ins("or b,0,sq",               "b[i] = sq[i] (reset case)")
                    //    .ins("vadd a,1,a",              "a[i] += 1");
                    //fi.lab("INDUCE_DONE");
                    //fi.pop_scope();
                    fi  >>"if(tmod){" // using mk_scope or CBLOCK_SCOPE is entirely optional...
                        >>"    b = _ve_vaddul_vsv(vl0,b); // b[i] += vl0 (easy, a[] unchanged)"
                        >>"}else{"
                        >>"    a = _ve_vaddul_vsv(1,a);   // a[i] += 1"
                        >>"    b = sq;                    // b[i] = sq[i] (reset case)"
                        >>"}";
                }
            }
            if( have_jjMODvl_reset ) assert( have_jjMODvl );
            assert( !have_bA_bD );
        }else if( positivePow2(jj) ){ // ------4 vec ops (add, shr, and, add)
            tr+="induce:pow2(jj): a[], b[] via shift+mask";
            if(verbose){cout<<" h";cout.flush();}
            assert( vl0%jj != 0 ); assert( jj%vl0 != 0 ); assert(have_bA_bD==1);
            assert(have_jj_shift==1); assert(have_vl_over_jj==0);
            // no...assert(have_sq==(jj>1&&jj<vl0));
#if 0
            FOR(i,vl) bA[i] = vl0 + b[i];           // bA = b + vl0; add_vsv
            FOR(i,vl) bD[i] = (bA[i] >> jj_shift);  // bD = bA / jj; div_vsv
            FOR(i,vl) b [i] = (bA[i] & jj_minus_1); // bM = bA % jj; mod_vsv
            FOR(i,vl) a [i] = a[i] + bD[i]; // aA = a + bD; add_vvv
#else
            // cnt is downwards in goto style, we want upward cnt here
            uint64_t cnt_incr = cnt;
            //if(cnt_incr<5000) cout<<" div-mod cnt_incr="<<cnt_incr<<endl;
            FOR(i,vl) a [i] = (cnt_incr+i)/jj; // just do it slowly for simulation purposes
            FOR(i,vl) b [i] = (cnt_incr+i)%jj;
#endif
            ++cnt_bA_bD; ++cnt_jj_shift; assert( have_bA_bD ); // leftover, just for assertions
            if(onceI){
#if 0
                fi.ins("vaddu.l bA,"+jitdec(vl0)+",b"
                        , "bA[i] = b[i]+vl0 (jj="+jitdec(jj)+" power-of-two)")
                    .ins("vsrl.l.zx bD,bA,"+jitdec(jj_shift)
                            , "bD[i] = bA[i]>>jj_shift["+jitdec(jj_shift)+"]")
                    // note: VAND vy~M possible, while AND has sy~I, sz~M
                    .ins("vand b,"+jitimm(jj_minus_1)+",bA"
                            , "b[i] = bA[i]&jj_mask = bA[i]%"+jitdec(vl0))
                    .ins("vaddu.l a,a,bD"
                            , "a[i] += bD[i]");
#elif 0 // 4 ops, so full recalc is faster!
                fi  >>"bA = _ve_vaddul_vsv("<<jitdec(vl0)<<",b);"
                    " // bA[i] = b[i]+vl0 (jj="<<jitdec(jj)<<" power-of-two)"
                    >>"bD = _ve_vsrl_vvs(bA,"<<jitdec(jj_shift)<<";"
                    " // bD[i] = bA[i]>>jj_shift["<<jitdec(jj_shift)<<"]"
                    >>"b  = _ve_vand_vsv("<<jithex(jj_minus_1)<<",bA);"
                    " // b[i] = bA[i]&jj_mask = bA[i]%"<<jitdec(vl0)
                    >>"a = _ve_vaddul_vvv(a,bD);   // a[i] += bD[i]"
                    ;
#else
                // cf. pow2 "induction" by full recalc:
                //  sqij = _ve_vaddul_vsv(vl0,sqij);
                //  a = _ve_vsrl_vvs(sqij, jj_shift);              // a[i]=sq[i]/jj
                //  b = _ve_vand_vsv("<<jithex(jj_minus_1)<<",sq); // b[i]=sq[i]%jj
                def_sqij=true;
                fi>>"sqij = _ve_vaddul_vsv(vl0,sqij);                // sqij[i] += "<<jitdec(vl0);
                // (same as INIT_BLOCK code)
                fp.clear(); fp["../beg"].clear(); fp["../end"].clear();
                fp["last"].set(OSSFMT("DIVMOD_"<<jj<<"(a,b,sqij);"));
#endif
            }
        }else{ // div-mod ---------------------6 vec ops: add (mul,shr) (mul,sub) add
            tr+="induce: a[], b[] update via fastdiv";
            if(verbose){cout<<" i";cout.flush();}
#if 0 // now I support full-range FASTDIV_jj and DIVMOD_jj macros
            FOR(i,vl) bA[i] = vl0 + b[i];            // add_vsv
            FOR(i,vl) bD[i] = ((jj_M*bA[i]) >> FASTDIV_C);  // fastdiv_uB   : mul_vvs, shr_vs
            FOR(i,vl) b [i] = bA[i] - bD[i]*jj;     // long-hand    : mul_vvs, sub_vvv
            FOR(i,vl) a [i] = a[i] + bD[i];         // add_vvv
            // Note;
            //  this is 1 + 2 + 2 +1 = 6 vector ops. 2mul, 3add/sub, 1shift
            //  using 1 big constant, and 2 tmp vector registers
            //  WITH possibly some concurrent execution units
            //  WITH restricted u32 input range
            // cf. full recalc with sqij
            //  1(sqij update) + 3(worst case div) + 2(mod) vecops: add, mul-add-shr, mul-sub
            //  using 2 big consts, and no [explicit] tmp vector registers
            //  WITHOUT parallel execution units possible
            //  WITHOUT restricted u32 input range
            //
            //So it might be a slight bit faster than 'struct fastdiv' approach
            //
#else
            // cnt is downwards in goto style, we want upward cnt here
            uint64_t cnt_incr = cnt;
            //if(cnt_incr<5000) cout<<" div-mod cnt_incr="<<cnt_incr<<endl;
            FOR(i,vl) a [i] = (cnt_incr+i)/jj; // just do it slowly for simulation purposes
            FOR(i,vl) b [i] = (cnt_incr+i)%jj;
#endif
            ++cnt_bA_bD; ++cnt_jj_M;
            assert(have_bA_bD); assert(have_sq==(jj>1&&jj<vl0)); assert(!have_jj_shift);
            assert(!have_vl_over_jj);
            if(onceI){
#if 0
                bool rerun_init_code = (u.suggested==UNR_NLOOP || u.suggested==UNR_CYC
                        || u.suggested==UNR_CYC);
                if(rerun_init_code){
                }else{
                    fp["../beg"].set("if(cnt==0){");
                }
#endif
                // TODO XXX output (and use) DIVMOD macro, as per cf3-unroll.cpp
                if( /*0 &&*/ iijj + vl0 < FASTDIV_SAFEMAX ){ // use 'pixel register' sqij
                    // but this has less execution unit concurrency (bad dependency chain)
                    // but also has one less op, so it *might* be a speed winner.
                    // [ the next block could also do the same calc ]
                    def_sqij=true;
                    fi>>"sqij = _ve_vaddul_vsv(vl0,sqij);                // sqij[i] += "<<jitdec(vl0);
                    // (same as INIT_BLOCK code)
                    mk_DIVMOD(cfuse2,jj,iijj+vl0); // give iijj to recognize mul-shr opportunity
                    fp.clear(); fp["../beg"].clear(); fp["../end"].clear();
                    //fp["last"].set(OSSFMT(
                    //            "a = FASTDIV_"<<jj<<"(sqij); // a[i] = sqij[i]/jj recalc\n"
                    //            "b = _ve_vsubul_vvv(sqij,_ve_vmulul_vsv(jj, a)); // b[i] = sqij[i] - jj*a[i]"));
                    fp["last"].set(OSSFMT("DIVMOD_"<<jj<<"(sqij,a,b); // a[]=sqij[]/jj; b[]=sqij[]%jj;"));
                }else if(jj+vl < FASTDIV_SAFEMAX) { // o/w use safer induction formula
                    // Ex. ./cfuse2 -t 256 1500 1500, 1500 x 1500 image will be enough to overflow
                    // FASTDIV "full recalc" based on pixel number.
                    // But we can still induce based on current b[i] vector with less chance
                    // of overflow because b[i] < jj (much smaller)
                    assert( jj + vl < FASTDIV_SAFEMAX );
                    fi  >>"// FASTDIV induce from prev b to avoid overflow"
                        >>"__vr bD = FASTDIV_"<<asDec(jj)<<"(_ve_vaddul_vsv(vl0,b));   // bD[i]=(b[i]+vl0)/[jj="<<jitdec(jj)<<"]"
                        >>"a = _ve_vaddul_vvv(a, bD);                     // a[i] += bD[i]"
                        >>"b = _ve_vsubul_vvv(b, _ve_vmulul_vsv(jj, bD)); // b[i] -= jj*bD[i]"
                        ;
                }else{
                    // Ex. ./cfuse2 -t 256 3 4000001 has jj too large for 2-op FASTDIV
                    def_sqij=true;
                    fi  >>"sqij = _ve_vaddul_vsv(vl0,sqij);                // sqij[i] += "<<jitdec(vl0);
                    // (same as INIT_BLOCK code)
#if 0
                    cout<<"unhandled case: need large-number vector-division-by-constant "<<jj<<endl;
                    fp.clear(); fp["../beg"].clear(); fp["../end"].clear();
                    fp["last"].set(OSSFMT(
                                "a = _ve_vdivsl_vvs(sqij, outWidth) // a[i] = sqij[i]/jj XXX SLOW XXX\n"
                                "b = _ve_vsubul_vvv(sqij,_ve_vmulul_vsv(jj, a)); // b[i] = sqij[i] - jj*a[i]"));
                    THROW("MISSING OPTIMIZATION:\nPlease implement mul-add-shift division algorithm HERE");
#else // new mk_DIVMOD function expands to include generic cases of division by constant
                    mk_DIVMOD(cfuse2, jj, iijj+vl0); // defines FASTDIV_jj(V) and DIVMOD_jj(V,A,B,VDIV,VMOD)
                    fp.clear(); fp["../beg"].clear(); fp["../end"].clear();
                    fp["last"].set(OSSFMT("DIVMOD_"<<jj<<"(sqij,a,b); // a[]=sqij[]/jj; b[]=sqij[]%jj;"));
#endif
                }
            }
        }
#if VL_LAST_ITER==2
#if 0
        // if careful about vl (possible lower value for last iter) vs vl0 can update here:
        if( cnt + vl > iijj ){ // last time might have reduced vl
            vl = iijj - cnt;
            cout<<" vl reduced for last loop to "<<vl<<endl;
        }
#endif
        // Careful: vl = new value for last iteration, vl0 = original = old value
        if( iijj % vl0 ){
            tr+="induce:iijj%vl0 last iter LATE lower vl";
            // VE has single-op MIN
            //vl = std::min( (uint64_t)vl, /*remain =*/ iijj - cnt );
            vl = std::min( (uint64_t)vl, /*remain =*/ cnt );
            if(onceI){
                //AsmScope const block = {{"remain","%s40"}};
                //fi.scope(block, "last time reduce vl");
                //fi.ins("subu.l remain,iijj,cnt",    "remain = iijj -cnt");
                //fi.ins("mins.l vl, vl0, remain",    "vl = min(vl,remain)");
                //fi.ins("lvl vl");
                //fi.pop_scope();

                //fi.ins("mins.l vl, vl0, cnt",    "vl = min(vl,cnt)");
                //fi.ins("lvl vl");
                fi  >>"vl = (vl0<cnt? vl0: cnt); // vl = min(vl0,cnt) for last iter,"
                    >>"_ve_lvl(vl);";
            }
        }
#endif
        if(onceI){
            onceI = false;
        }

KERNEL_BLOCK:
        tr+="KERNEL_BLOCK";
        if(onceK){
            //cout<<"=== // #KERNEL_BLOCK: (fallthrough)\n"
            //<<"=== //        # <your code here, using a[] b[] loop-index vectors\n";
            fuse4_kernel(fd, fd, fk, fz, ii,jj,vl, kernComment(), which);
        }

        // KERNEL-BLOCK
        // Do something with the a[], b[] vectors of i,j loop indices...
        //cout<<"cnt "<<cnt<<" iloop "<<iloop<<" ii "<<ii<<" jj "<<jj<<endl;
        if(0){
            int const n=8; // output up-to-n [ ... [up-to-n]] ints
            int const bignum = std::max( ii, jj );
            int const wide = 1 + (bignum<10? 1: bignum <100? 2: bignum<1000? 3: 4);
            cout<<"a["<<vl<<"]="<<vecprt(n,wide,a,vl)<<endl;
            cout<<"b["<<vl<<"]="<<vecprt(n,wide,b,vl)<<endl;
        }
        // check correctness
        assert( vl == vabs[iloop].vl );
        FOR(i,vl) assert( a[i] == vabs[iloop].a[i] );
        FOR(i,vl) assert( b[i] == vabs[iloop].b[i] );
        // Alt. is a hash-value test:
        vhash.hash_combine(a.data(),b.data(),vl);
        //cout<<"iloop="<<iloop<<" vl="<<vl<<" vhash "<<vhash.u64()
        //    <<"\n ref hash "<<vabs[iloop].hash<<endl;
        assert( vhash.u64() == vabs[iloop].hash );

        onceK = false;

        // DONE_CHECK
        // simplification : whole loop should be over 'remain' as
        //
        //     remain -= vl;
        //     if(remain>0) goto INDUCE
        //
        ++iloop; // needed only sometimes
        if( nloop <= 1 ){
            //cnt += vl; // for tighter exit assertion
            //cout<<" exit A cnt="<<cnt<<endl;
        }else if(iijj % vl0){
            //cout<<" B cnt="<<cnt<<endl;
            if( vl == vl0 ){
                tr+="loop: induce next a[] b[] if vl==vl0";
                // cnt += vl; move to induce-block
                goto INDUCE;
            }
            tr+="exiting";
            // exit...
            //cout<<" exit B cnt="<<cnt<<endl;
            //cnt += vl; // just for tighter exit assertion
        }else{ // generic end-loop test
            //++cnt_use_iijj; // XXX old!
#if 0 // cnt 0..iijj
            cnt += vl0;
            if( cnt < iijj ){
                goto INDUCE;
            }
#else // cnt iijj..0
            cnt += vl0;
            //cout<<" C cnt="<<cnt<<endl;
            if( (uint64_t)cnt < iijj ){
                tr+="loop: induce next a[] b[] if cnt < iijj";
                //cout<<" again B cnt="<<cnt<<endl;
                goto INDUCE;
            }
            tr+="exiting C";
            //cout<<" exit C cnt="<<cnt<<endl;
#endif
        }

        if(onceL){
            onceL = false;
        }
        // define/initialize 'def_' needed variables...
        //if(STYLE==STYLE_GOTO && def_cnt){
        //    fd  >>"int64_t cnt = (uint64_t)ii * (uint64_t)jj;"
        //        <<OSSFMT(" // cnt=ii*jj="<<ii<<"*"<<jj<<" to 1? by "<<vl0);
        //}
        if(cnt_jj_M){
#if 0 // move toward generic (and more flexible) FASTDIV_jj and DIVMOD_jj macros
            fd  .define(OSSFMT("FASTDIV_"<<jj<<"_M"), jithex(jj_M)+"/*in reg?*/")
                .define(OSSFMT("FASTDIV_"<<jj<<"(VR)"),
                        OSSFMT("_ve_vsrl_vvs(_ve_vmulul_vsv("
                            "FASTDIV_"<<jj<<"_M, (VR)), "
                            <<FASTDIV_C<<")"));
#else
            mk_DIVMOD(cfuse2,jj,iijj+vl0);
#endif
        }
        if(cnt_sq){
            fd["..*/first"]>>"__vr const sq = _ve_vseq_v();";
        }
        if(def_sqij){
            if(cnt_sq) fd>>"__vr sqij = sq;";
            else       fd>>"__vr sqij = _ve_vseq_v();";
            //fk>>"// *this* kernel also has defined sqij[]=a[]*"<<asDec(jj)<<"+b[]";
        }else if(nloop>1){
            fp["../beg"].set("if(cnt==0){"); // this is a gross simplification,
            // but 'unrolling' version does move special init to pre-loop
            // and has OK induction/loop exit JIT code too (should really do it here too)
        }

        // LOOP_DONE ...
        if(SAVE_RESTORE_VLEN){
            //fz.ins("lvl vl_save","load VL <-- vl_save (restore VL on exit)");
            fz  >>"//_ve_lvl(vl_save); // restore VL on exit XXX when SVL op is supported!!!";
        }

#undef FOR
        cout<<" Yay! induction formulas worked! iloop,nloop="<<iloop<<","<<nloop<<endl;
        //assert( cnt == iijj || cnt == iijj/cnt*cnt );
        cout<<" end with cnt="<<cnt<<" cf. nloop*vl0="<<nloop*vl0<<endl;
        cout<<" end with cnt="<<cnt<<" cf. (nloop-1)*vl0="<<(nloop-1)*vl0<<endl;
        cout<<" end with cnt="<<cnt<<" cf. iijj/vl0*vl0="<<iijj/vl0*vl0<<endl;
        assert( (nloop<=1 && cnt==0) || (nloop>1 && (uint64_t)cnt==iijj/vl0*vl0) );
        assert( nloop == iloop );
        assert( have_vl_over_jj     == (cnt_vl_over_jj    > 0) );
        assert( have_bA_bD          == (cnt_bA_bD         > 0) );
        assert( have_sq             == (cnt_sq            > 0) );
        assert( have_jj_shift       == (cnt_jj_shift      > 0) );
        //assert( have_use_iijj       == (cnt_use_iijj      > 0) );
        assert( cnt_use_iijj == 0); // cnt iijj-->1 now
        assert( have_jj_M           == (cnt_jj_M          > 0) );
        assert( have_jjMODvl        == (cnt_jjMODvl       > 0) ); // old "special" count, case 'g' needed
        assert( have_jjMODvl_reset  == (cnt_jjMODvl_reset > 0) ); // old "special" count, case 'g' needed
        // XXX fX do not automatically form a tree. Would be nice not to have to think about
        //     ordering and where pop_scope should really occur.
        cout<<"##### Final program ################################"<<endl;
#if 0
        cout<<fd.flush();
        cout<<fp.flush();
        cout<<fi.flush();
        cout<<fk.flush();
        cout<<fl.flush();
        cout<<fz.flush();
        VecHash2::kern_asm_end(fd);
        cout<<fp.flush_all(); // kern_asm has some scope here, now
        cout<<fd.flush_all();
#else
        cout<<pr.tree();
        string prog = pr.str();
        cout<<prog;
#endif
        cout<<"##### End of program ################################"<<endl;
        // XXX multiple scope write/destroy has issues! (missing #undefs for fd right now)
        //fd.pop_scope();     // TODO: destructors should auto-pop any AsmFmtCols scopes!!!
    }
    cout<<tr.str()<<" vlen,ii,jj= "<<vlen<<" "<<ii<<" "<<jj;
    if(vl0!=vlen) cout<<" (used alt vlen "<<vl0<<")"<<endl;
    cout<<tr.dump();
    string program2 = cfuse2_no_unroll(vl0,ii,jj,which,1/*verbose*/);
    cout<<" Compare with cfuse2_no_unroll:\n"
        <<cfuse2_no_unroll(vl0,ii,jj,which,1/*verbose*/);
    if(ofname!=nullptr){
        ofstream ofs(ofname);
        ofs<<"// Autogenerated from "<<__FILE__<<" cfuse2_no_unroll(...)\n";
        ofs<<"// Possible compile:\n";
        ofs<<"//   clang -target linux-ve -O3 -fno-vectorize -fno-unroll-loops -fno-slp-vectorize -fno-crash-diagnostics tmp-vi.c"<<endl;
        ofs<<program2;
        ofs<<"// vim: ts=2 sw=2 et cindent\n";
        // cino=^=l0,\\:.5s,=-.5s,N-s,g.5s,b1 cinkeys=0{,0},0),\\:,0#,!^F,o,O,e,0=break\n"
        ofs.close();
        cout<<"// Written to file "<<ofname<<endl;
    }
}
void cf4_help_msg(){
    cout<<" cf4 -[h|t|a]* [-kSTR] [-oFILE] VLEN I J"
        <<"\n Function:"
        <<"\n double loop --> loop over vector registers a[VLEN], b[VLEN]"
        <<"\n Output optimized VE C+intrinsics code for loop fusion."
        <<"\n JIT options:"
        <<"\n         [default: verbose simulation test and -t]"
        <<"\n  -t     VE intrinsics code (no simulation test)"
        <<"\n  -a     use lower-vl alt strategy if can speed induction"
        <<"\n         - suggested for Aurora VLEN=256 runs"
        <<"\n  -uN    N=max unroll Ex. -tu8ofile for unrolled version of -tofile"
        <<"\n"
        <<"\n  -kSTR  kernel type: [CHECK]|NONE|HASH|PRINT|SQIJ"
        <<"\n  -oFILE output code filename"
        <<"\n         - Ex. -ofile-vi.c for clang-vector-i|trinsics code"
        <<"\n         - TODO cf4.sh or cf4u.sh to compile & run JIT on VE"
        <<"\n Parameters:"
        <<"\n   VLEN  = vector length"
        <<"\n   I     = 1st loop a=0..i-1"
        <<"\n   J     = 2nd loop b=0..j-1"
        <<"\n  -h     this help"
        <<endl;
}
int main(int argc,char**argv){
    int vl = 8;
    int h=20, w=3;
    int opt_t=1, opt_h=0;
    int a=0;
    uint32_t maxun=0U;
    char *ofname = nullptr;
    int which = WHICH_KERNEL;

    if(argc > 1){
        // actually only the last -[tlu] option is used
        for( ; a+1<argc && argv[a+1][0]=='-'; ++a){
            char *c = &argv[a+1][1];
            cout<<" arg : "<<c<<endl;
            for( ; *c != '\0'; ++c){
                if(*c=='h'){ cf4_help_msg(); opt_h = 1;
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
        <<" maxun="<<maxun<<endl;
    if(argc > a+1) vl = atof(argv[a+1]);
    if(argc > a+2) h  = atof(argv[a+2]);
    if(argc > a+3) w  = atof(argv[a+3]);
    cout<<"vlen="<<vl<<", h="<<h<<", w="<<w<<endl;
    assert(opt_t==1 || opt_t==2 || opt_t==3);

    uint32_t nerr=0U;
    if(opt_h == 0){
        try{
            if(maxun==0) // no unroll...
                if(opt_t==1){
                    opt_t=2;
                    test_vloop2_no_unrollX(vl,h,w,opt_t,which,ofname);
                }else{
                    cfuse2_no_unrollX(vl,h,w,opt_t,which,ofname);
                }
            else{ // unroll...
                cfuse2_unrollX(vl,h,w,maxun,opt_t,which,ofname);
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
