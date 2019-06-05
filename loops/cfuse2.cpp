/* Copyright (c) 2019 by NEC Corporation
 * This file is part of ve-jit */
/** \file
 * fuse2.cpp shows how to fuse 2 loops into vectorized index calculations.
 * It is just a basic test of the idea (verifying correctness).
 *
 * Also does pseudo-code unroll and no_unroll demos
 *
 * \todo "fuse2.cpp -all" to run a long battery of tests.
 *
 * \sa xxx.cpp for VE assembler output, introducing other header deps.
 */
#include "../fuseloop.hpp"
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

#include <cstring>
#include <cstddef>
#include <cassert>

#ifndef FOR
#define FOR(I,VL) for(int I=0;I<VL;++I)
#endif

#define DEF(VAR) def(#VAR,VAR)

#define STYLE_GOTO 0
#define STYLE_FORLOOP 1
        /** 0: `goto` 1: `for(cnt=0; cnt<ii*jj; ++cnt)` */
#define STYLE 1

#define SAVE_RESTORE_VLEN 0

#define KERNEL_NONE 0
#define KERNEL_HASH 1
#define KERNEL_PRINT 2
#define KERNEL_CHECK 3
#define KERNEL_SQIJ 4 /*just an example*/
/** what kernel? */
#define WHICH_KERNEL KERNEL_HASH

void other_fastdiv_methods(int const jj);

using namespace std;
using namespace loop;
using namespace cprog;

std::string kernel_name(int const which){
    return (which==KERNEL_NONE? "NONE"
            :which==KERNEL_HASH? "HASH"
            :which==KERNEL_PRINT? "PRINT"
            :which==KERNEL_CHECK? "CHECK"
            :which==KERNEL_SQIJ? "SQIJ"
            :"HUH");
}
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

typedef uint32_t u32;
typedef int32_t s32;
typedef uint64_t u64;
/* required for magic constant generation */
u32 ulog2(u32 v) {
    u32 r, shift;
    r =     (v > 0xFFFF) << 4; v >>= r;
    shift = (v > 0xFF  ) << 3; v >>= shift; r |= shift;
    shift = (v > 0xF   ) << 2; v >>= shift; r |= shift;
    shift = (v > 0x3   ) << 1; v >>= shift; r |= shift;
    r |= (v >> 1);
    return r;
}

struct fastdiv {
    u32 mul;
    u32 add;
    s32 shift;
    u32 _odiv;  /* save original divisor for modulo calc */
};
/* generate constants for implementing a division with multiply-add-shift.
 * **MODIFIED** for VE. */
void fastdiv_make(struct fastdiv *d, u32 divisor) {
    u32 l, r, e;
    u64 m;

    d->_odiv = divisor;
    // Modifed [ejk]
    if( positivePow2(divisor) ){
        d->mul = 1U;
        d->add = 0U;
        d->shift = positivePow2Shift(divisor);
        assert( ulog2(divisor) == (u32)d->shift );
        return;
    }
    l = ulog2(divisor);
    if (divisor & (divisor - 1)) {
        m = 1ULL << (l + 32);
        d->mul = (u32)(m / divisor);
        r = (u32)m - d->mul * divisor;
        e = divisor - r;
        if (e < (1UL << l)) {
            ++d->mul;
            d->add = 0;
        } else {
            d->add = d->mul;
        }
        d->shift = 32+l;
    } else {
        if (divisor == 1) {
            d->mul = 1;
            d->add = 0;
            d->shift = 0;
        } else {
            d->mul = 1;
            d->add = 0;
            d->shift = l-1;
        }
    }
}

/* emit kernel (comment/code).
 *
 * Interestingly, any re-usable data of the kernel should
 *  (somehow)
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
void fuse2_kernel(Cblock& bKrn, Cblock& bDef, Cblock& bOut,
        int64_t const ii, int64_t const jj, int64_t const vl,
        std::string extraComment,
        int const which=0/*comment,VecHash2*/,
        std::string pfx="fuse2_",
        int const v=0/*verbose*/,
        string vA="a", string vB="b", string vSEQ0="sq", string sVL="vl"
        ){
    static ostringstream oss;

    if( which==KERNEL_NONE ){ // just an optional extraComment
        bKrn["beg"]>>OSSFMT("// KERNEL("<<vA<<"["<<vl<<"],"<<vB<<"["<<vl<<"])");
        if(!extraComment.empty()) bKrn["beg"]<<extraComment;
    }else if( which==KERNEL_HASH ){
        if(!extraComment.empty()) bKrn["beg"]<<" "<<extraComment;
        std::string vh2 = pfx + "VecHash2";
        // constants moved upwards to beginning of scope enclosing bDef
        auto& bDefConst = bDef["..*/first"];
        // state variables at end of bDef
        auto& bDefState = bDef["last"]["vechash"];
        if(bDefState.code_str().empty()){
            string vSeq = (vSEQ0.empty()? "_ve_vseq_v()": vSEQ0);
            VecHash2::kern_C_begin(bDefConst, vSeq.c_str(), vl);
            bDefState>>OSSFMT(left<<setw(40)<<OSSFMT("int64_t "<<vh2<<" = 0;"))
                <<" // vh2({a,b}) hash output";
            bOut>>"printf(\"jit "<<vh2<<" = %llu\\n\",(long long unsigned)"<<vh2<<");";
        }
#if 0 // original: inline, many statements
        bKrn["beg"]>>OSSFMT("// KERNEL("<<vA<<"["<<vl<<"],"<<vB<<"["<<vl<<"])");
        VecHash2::kern_C(bKrn[OSSFMT(pfx<<"_VecHash2")],vA,vB,sVL,vh2);
#else // NEW: as macro
        auto m = VecHash2::kern_C_macro("VECHASH2_KERNEL");
        bDefConst.define(m.first,m.second);
        bKrn[OSSFMT(pfx<<"_VecHash2")]
            >>OSSFMT(left<<setw(40)<<OSSFMT("VECHASH2_KERNEL("<<vA<<","<<vB<<","<<sVL<<","<<vh2<<");")
                        <<" // "<<vA<<"["<<vl<<"],"<<vB<<"["<<vl<<"]");
#endif
    }else if( which==KERNEL_PRINT ){
        // XXX Does clang have any option to pass __vr efficiently ?
        bool const nice_vector_register_args = true;
        if( !nice_vector_register_args ){ // inlined...
            bKrn["beg"]>>OSSFMT("// KERNEL("<<vA<<"["<<vl<<"],"<<vB<<"["<<vl<<"])");
            if(!extraComment.empty()) bKrn["beg"]<<" "<<extraComment;
            bKrn["prt"]>>"printf(\"a={\");"
                >>"for(int i=0;i<"<<sVL<<";++i){ printf(\"%llu%c\","
                >>"    (long long unsigned)_ve_lvs_svs_u64("<<vA<<",i),"
                >>"    (i%8==0? \"\\n   \":\" \")); }"
                >>"printf(\"}\\n\");"
                >>"printf(\"b={\");"
                >>"for(int i=0;i<"<<sVL<<";++i){ printf(\"%llu%c\","
                >>"    (long long unsigned)_ve_lvs_svs_u64("<<vB<<",i),"
                >>"    (i%8==0? \"\n   \":\" \")); }"
                >>"printf(\"}\\n\");"
                ;
        }else{ // as function call (KERNEL_PRINT does not need speed)
            bDef["..*/includes"]>>"#include <stdio.h>";
            auto& bDefKernelFn = bDef["..*/fns/first"];
            if(bDefKernelFn.find("cfuse2_kernel_print")==nullptr){
                CBLOCK_SCOPE(cfuse2_kernel_print,
                        "void cfuse2_kernel_print(__vr const a, __vr const b,"
                        "\n        uint64_t const vl)",bDefKernelFn.getRoot(),bDefKernelFn);
                cfuse2_kernel_print
                    >>"printf(\"a={\");"
                    >>"for(int i=0;i<vl;++i){ printf(\"%llu%c\","
                    >>"    (long long unsigned)_ve_lvs_svs_u64("<<vA<<",i),"
                    >>"    (i%8==0? \"\\n   \":\" \")); }"
                    >>"printf(\"}\\n\");"
                    >>"printf(\"b={\");"
                    >>"for(int i=0;i<vl;++i){ printf(\"%llu%c\","
                    >>"    (long long unsigned)_ve_lvs_svs_u64("<<vB<<",i),"
                    >>"    (i%8==0? \"\n   \":\" \")); }"
                    >>"printf(\"}\\n\");"
                    ;
            }
            bKrn["prt"]<<"cfuse2_kernel_print("<<vA<<", "<<vB<<", "<<sVL<<");";
            if(!extraComment.empty()) bKrn["prt"]<<" // "<<extraComment;
        }
        bOut>>"printf(\"cfuse KERNEL_PRINT done!\\n\");";
    }else if( which==KERNEL_CHECK ){
        // as function call (KERNEL_PRINT does not need speed)
        bDef["..*/includes"]>>"#include <stdio.h>";
        bDef["..*/includes"]>>"#include <assert.h>";
        auto& bDefKernelFn = bDef["..*/fns/first"];
        if(bDefKernelFn.find("cfuse2_kernel_print")==nullptr){
            CBLOCK_SCOPE(cfuse2_kernel_check,
                    "void cfuse2_kernel_check(__vr const a, __vr const b,"
                    "\n        uint64_t const cnt, uint64_t const vl, uint64_t const jj)",
                    bDefKernelFn.getRoot(),bDefKernelFn);
            cfuse2_kernel_check
                >>"for(uint64_t i=0;i<vl;++i){"
                >>"    assert( _ve_lvs_svs_u64("<<vA<<",i) == (cnt+i)/jj );"
                >>"    assert( _ve_lvs_svs_u64("<<vB<<",i) == (cnt+i)%jj );"
                >>"}"
                ;
        }
        bKrn["prt"]<<"cfuse2_kernel_check("<<vA<<", "<<vB<<", cnt, "<<sVL<<", "<<jitdec(jj)<<");";
        if(!extraComment.empty()) bKrn["prt"]<<" // "<<extraComment;
        bOut>>"printf(\"cfuse KERNEL_CHECK done! no errors\\n\");";
    }else if( which==KERNEL_SQIJ ){
            bKrn["beg"]>>OSSFMT("// KERNEL("<<vA<<"["<<vl<<"],"<<vB<<"["<<vl<<"],sqij="<<vA<<"*jj+"<<vB<<")");
            bKrn["prt"]>>"__vr const x = STORE(0, _ve_addul_vsv(ptr,_ve_vmulul_vsv(stride,sqij)));";
            if(!extraComment.empty()) bKrn["prt"]<<" // "<<extraComment;
    }else{
        THROW(OSSFMT("unknown kernel type "<<which<<" in fuse2_kernel"));
    }
}

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
/** return FASTDIV_jj(V,JJ,VOUT) macro to produce vector \c vDiv=vReg/jj
 * \c jj is a constant divisor.
 * \pre vReg has u32 values stored in a u64 vector register.
 * \pre jj>0
 * \pre if vIn_hi>0, then vReg input values are assumed to be &lt; \c vIn_hi
 * \return number of operations required.
 *
 * \c vIn_hi optional range-restriction may give better op-codes. For fused loops
 * `for(0..ii)for(0..jj)` with some max VL, you should set \c vIn_hi=ii*jj+VL,
 * and if this is < FASTDIV_SAFEMAX we'll use 2-op mul-shr instead.
 *
 * - Method:
 *   - count ops for 'struct fastdiv' method (mul, add(?), shift)
 *   - if jj is 2^N, use shift/mask (prev method finds this solution)
 *   - if op count is 3 and range restrictions met, use computeM_uB method (mul,shift)
 */
int mk_FASTDIV(Cblock& cb, uint32_t const jj, uint32_t const vIn_hi=0){
    int const v=1; // verbosity
    bool const verify=true; // false after code burn-in
    bool const macro_constants = false;
    int ret=0;
    ostringstream oss;
    // go up to some well-defined scope position and use a named-block to
    // record that this macro exists (try not to duplicate the definition)
    // where Cblock::define will place definitions (may need tweaking?)
    auto& scope=(cb.getName()=="body"? cb: cb["..*/body/.."]);
    string tag = OSSFMT("fastdiv_"<<jj); // "we were here before" tag
    if(v) cout<<"mk_FASTDIV_"<<jj<<" range "<<vIn_hi<<" to scope "<<scope.fullpath()
        //<<" <"<<scope.str()<<">"
        <<endl;;
    if(scope.find(tag)){
        if(v) cout<<"FASTDIV_"<<jj<<" macro already there"<<endl;
    }else{
        scope[tag].setType("TAG");
        if(v) cout<<"FASTDIV_"<<jj<<" new macro, input range "<<vIn_hi<<endl;
        struct fastdiv jj_fastdiv;
        uint32_t fastdiv_ops = 0U;
        {
            fastdiv_make( &jj_fastdiv, (uint32_t)jj );
            cout<<" mul,add,shr="<<(void*)(intptr_t)jj_fastdiv.mul
                <<","<<jj_fastdiv.add<<","<<jj_fastdiv.shift;
            if(jj_fastdiv.mul != 1) ++fastdiv_ops;
            if(jj_fastdiv.add != 0) ++fastdiv_ops;
            if(1) /*shift*/ ++fastdiv_ops;
            if(v) cout<<" struct fastdiv (mul,add,shr) in "<<fastdiv_ops<<" ops"<<endl;
        }
        string fastdiv_macro;
        // Accept fastdiv_ops<=2 because 1) bigger range; 2) sometimes smaller const mult
        // Otherwise, if jj_hi is given and small enough, we use the 2-op mul-shift method.
        if(fastdiv_ops==3 && (vIn_hi>0 && vIn_hi <= FASTDIV_SAFEMAX)){
            // fastdiv_ops==3 means we don't have a power-of-two easy case, so if
            // the input vector u32's are "small", we can have a 2-op mul-shift.
            uint64_t const jj_M = computeM_uB(jj); // 42-bit fastdiv_uB multiplier
            string mac;

            string mul;
            if(macro_constants||isIval(jj_M)){
                mul = OSSFMT("FASTDIV_"<<jj<<"_MUL");
                scope.define(mul,OSSFMT("((uint64_t)"<<jithex(jj_M)<<")"));
            }else{
                mul = OSSFMT("fastdiv_"<<jj<<"_MUL");
                cb["..*/first"]>>OSSFMT("uint64_t const "<<mul<<" = "<<jithex(jj_M)<<";");
            }
            mac = OSSFMT("_ve_vmulul_vsv("<<mul<<",V);");

            string shr;
            if(1){
                shr = OSSFMT("FASTDIV_"<<jj<<"_SHR");
                scope.define(shr,jitdec(FASTDIV_C));
            }else{
                shr = OSSFMT("fastdiv_"<<jj<<"_SHR");
                cb>>OSSFMT("uint64_t const "<<mul<<" = "<<jitdec(FASTDIV_C)<<";");
            }
            //mac = OSSFMT("_ve_vsrl_vvs("<<mac<<","<<shr<<")/*OK over [0,"<<vIn_hi<<")*/");
            mac = OSSFMT("_ve_vsrl_vvs("<<mac<<","<<shr<<")/*OK over [0,2^"<<FASTDIV_C/2<<")*/");

            fastdiv_macro = mac;
            ret = 2;
            if(v) cout<<"mk_FASTDIV "<<ret<<" ops, macro="<<fastdiv_macro<<endl;
            if(verify){ // quick correctness verification
                uint32_t hi = vIn_hi;
                if(hi==0){ hi = 257*min(jj,16384U); }
                for(uint64_t i=0; i<=hi; ++i){ // NB: 64-bit i
                    assert( ((i*jj_M)>>FASTDIV_C) == i/jj );
                }
            }

        }else{ // use 'struct fastdiv' approach (3-op max, 1-op min)
            string mac;
            if(jj_fastdiv.mul != 1){
                string mul;
                if(macro_constants||isIval(jj_fastdiv.mul)){
                    mul = OSSFMT("FASTDIV_"<<jj<<"_MUL");
                    scope.define(mul,jithex(jj_fastdiv.mul));
                }else{
                    mul = OSSFMT("fastdiv_"<<jj<<"_MUL");
                    cb["..*/first"]>>"uint64_t const "<<mul<<" = "<<jithex(jj_fastdiv.mul)<<";";
                }
                mac=OSSFMT("_ve_vmulul_vsv("<<mul<<",V)");
            }else{
                mac="V";
            }
            if(jj_fastdiv.add != 0){
                string add;
                if(macro_constants||isIval(jj_fastdiv.add)){
                    add = OSSFMT("FASTDIV_"<<jj<<"_ADD");
                    scope.define(add,jitdec(jj_fastdiv.add));
                }else{
                    add = OSSFMT("fastdiv_"<<jj<<"_ADD");
                    cb>>OSSFMT("uint64_t const "<<add<<" = "<<jitdec(jj_fastdiv.add)<<";");
                }
                mac=OSSFMT("_ve_vaddul_vsv("<<add<<","<<mac<<")");
            }
            if(jj_fastdiv.shift != 0){
                string shr;
                if(1){
                    shr = OSSFMT("FASTDIV_"<<jj<<"_SHR");
                    scope.define(shr,jitdec(jj_fastdiv.shift));
                }else{
                    shr = OSSFMT("fastdiv_"<<jj<<"_SHR");
                    cb>>OSSFMT("uint64_t const "<<shr<<" = "<<jitdec(jj_fastdiv.shift)<<";");
                }
                mac=OSSFMT("_ve_vsrl_vvs("<<mac<<","<<shr<<")");
            }
            fastdiv_macro = mac;
            ret = fastdiv_ops;
            if(v) cout<<"mk_FASTDIV "<<ret<<" ops, macro="<<fastdiv_macro<<endl;
            if(verify){ // quick correctness verification
                uint32_t hi = vIn_hi;
                if(hi==0){ hi = 257*min(jj,16384U); }
                for(uint64_t i=0; i<=hi; ++i){ // NB: 64-bit i
                    assert( (((uint64_t)i*jj_fastdiv.mul+jj_fastdiv.add)>>jj_fastdiv.shift) == i/jj );
                }
            }

        }
        scope.define(OSSFMT("FASTDIV_"<<jj<<"(V)"),fastdiv_macro);
    }
    return ret;
}

/** return DIVMOD macro to produce vectors \c vDiv=vReg/jj and \c vMod=vReg%jj where
 * \c jj is a constant divisor.
 *
 * \c vIn_hi optional range-restriction may give better op-codes. For fused loops
 * `for(0..ii)for(0..jj)` with some max VL, you should set \c vIn_hi=ii*jj+VL,
 * and if this is < FASTDIV_SAFEMAX we'll use 2-op mul-shr instead.
 *
 * \pre vReg has u32 values stored in a u64 vector register.
 * \pre jj>0
 * \return number of operations required.
 *
 * - Method:
 *   - if jj is 2^N, use shift/mask
 *   - else if vReg has a range restriction, check if can use computeM_uB method (mul,shift)
 *   - else use fastdiv method (mul, add(?), shift)
 * - for modulus, use either
 *   - mask for jj=2^N
 *   - else mul-sub
 * 
 * \todo
 *  NOT CONSIDERED: are DIVMOD_jj_MUL|ADD immediate constants?
 *  Would the constants be better off in register variables?
 *  Is clang avoid possible lea crap in the inner loop code for non-immediate constants?
 *  (i.e. larger-than-u32 multipliers may use 3-op load on VE, instead of minimal 2 or 1-op sequence)
 * 
 *  If lea's are bothersome, then we would try to load constants into scalar registers
 *  in some (other?) outer block instead of having local scalar regs.
 *  VE scalars are often limited range or nice-looking bitmasks.
 * 
 *  For assembly (when will I have extend asm to beef up intrinsics?)
 *  I also have optimized scalar-load code that takes at most 2 ops and might mix
 *  execution units a little better than pure-lea approaches.
 * 
 */
int mk_DIVMOD(Cblock& cb, uint32_t const jj, uint32_t const vIn_hi=0){
    int v=1;
    ostringstream oss;
    // go up to some well-defined scope position and use a named-block to
    // record that this macro exists (try not to duplicate the definition)
    auto& scope=(cb.getName()=="body"? cb: cb["..*/body/.."]); // where Cblock::define will place definitions
    if(v) cout<<"mk_DIVMOD_"<<jj<<" range "<<vIn_hi<<" to scope "<<scope.fullpath()
        //<<" <"<<scope.str()<<">"
        <<endl;
    int nops=0;
    string tag = OSSFMT("divmod_"<<jj);
    if(scope.find(tag)){
        if(v) cout<<"DIVMOD_"<<jj<<" macro already there"<<endl;
    }else{
        scope[tag].setType("TAG"); // create the tag block "we were here before"
        if(v) cout<<"DIVMOD_"<<jj<<" new macro"<<endl;
        int nops = mk_FASTDIV(cb,jj,vIn_hi);
        string mac = OSSFMT(" \\\n          VDIV = FASTDIV_"<<jj<<"(V); \\\n");
        if(nops==1){
            assert(positivePow2(jj));
            cout<<("MASK WITH jj-1 for modulus");
            mac = OSSFMT(mac<<"          VMOD = _ve_vand_vsv(V,"<<jithex(jj-1)<<",V)");
            ++nops;
        }else{
            // VE does not have FMA ops for any integer type.
            //     so for 12/24-bit floats could consdier exact-floating calcs,
            //     but conversion ops probably kill this idea (not tried).
            cout<<("MUL-SUB modulus");
            mac = OSSFMT(mac<<"          VMOD = _ve_vsubul_vvv(V,_ve_vmulul_vsv("<<jj<<",VDIV))");
            if(!isIval(jj)) mac.append(" /*is non-Ival in register?*/");
            nops+=2;
        }
        scope.define(OSSFMT("DIVMOD_"<<jj<<"(V,VDIV,VMOD)"),mac);
    }
    return nops;
}

// NOTE: we do a ">>FASTDIV_C", which we can't just elide on Aurora, even if FASTDIV_C==16

/* Suppose for i:(0,ii){ for j:(o,jj) {} gets
 * gets vector indices \c a[] and \c b[]
 * with preferred vector length \c vlen.
 * How do we set the initial and induced states of a,b?
 * How do we induce the next vector state?
 * How do get the final state (possible shorter vlen)?
 *
 * Here I assume we will generate a[vlen] and b[vlen]
 * separately as i/u64 vectors.  Other options might pack
 * them in a single register as up/lo i/u32.
 * ? or maybe double-length 2*vlen of i/u32 (is this possible?)
 */
void test_vloop2(Lpi const vlen, Lpi const ii, Lpi const jj){ // for r in [0,h){ for c in [0,w] {...}}
    VecHash2 vhash(vlen);
    int const verbose=1;
    assert( vlen > 0 );
    register uint64_t iijj = (uint64_t)ii * (uint64_t)jj;
    cout<<"test_vloop2( vlen="<<vlen<<" loops 0.."<<ii<<" 0.."<<jj<<" iijj="<<iijj;

    register int vl = vlen;
    //if (cnt+vl > iijj) vl = iijj - cnt;  // we assume ii and jj loops begin at 0
    if ((uint64_t)vl > iijj) vl = iijj;
    int const vl0 = vl; // debug
    int const nloop = (iijj+vl-1) / vl;    // div_round_up(iijj,vl)
    cout<<" vl="<<vl<<" nloop="<<nloop<<endl;
    assert( (nloop > 1) == ((uint64_t)vl < iijj) );
    assert( vl0 > 0 );
    assert( ii > 0 );
    assert( jj > 0 );

    cout<<"Verify-------"<<endl;
    // generate reference index outputs
    std::vector<Vab> vabs = ref_vloop2(vlen, ii, jj, 0/*verbose*/);
    assert( vabs.size() > 0 );
    assert(vabs.size() == (size_t)(((ii*jj) +vlen -1) / vlen));
    if(verbose>=2) cout<<"   vl="<<vl<<"   ii="<<ii<<"   jj="<<jj
        <<"   iijj="<<iijj<<" vabs.size() = "<<vabs.size()<<endl;
    // Have reference vabs vectors. Now we try induction way.
    // 1. initialize: could copy vabs[0] from const data storage, or...
    //   - generate from seq + divmod.
    //   - 2-loop induction uses 3 scalar registers:
    //     - \c cnt 0.. \c iijj, and \c vl (for jit, iijj is CCC (compile-time-const))
    //     - get final \c vl from cnt, vl and iij)

    if(verbose>=1) other_fastdiv_methods(jj);

    // various misc precalculated consts and declarations.
    VVlpi a(vl), b(vl), bA(vl), bM(vl), bD(vl), aA(vl), sq(vl);
    VVlpi a0(vl), b0(vl), x(vl), y(vl);
    int iloop = 0; // mostly for debug checks, now;
    //Ulpi jj_mod_inverse_lpi   = mod_inverse((Ulpi)jj);
    //Uvlpi jj_mod_inverse_Vlpi = mod_inverse((Uvlpi)jj);
    // bA and bD are used when:
    //   iijj > vl && jj%vl!=0
    // sq is used when:
    //   iloop==0:   jj>1 && vl>=jj
    //   iloop >0:   jj%vl==0 && (special: iloop can be >= (jj/vl))
    uint64_t const jj_M = computeM_uB(jj); // for fastdiv_uB method
    cout<<" jj="<<jj;
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
    cout<<endl;

#if 0
    // NB: common operation is divmod(v,s,vM,vD) : v--> v%s, v/s,
    //     which has some optimizations for nice values of jj.
    // FASTDIV_C++14: &vl=std::as_const(vl)
    auto v_divmod_vs = [&vl,&jj,&jj_M](/* in*/ VVlpi const& a, Vlpi const d, /*out*/ VVlpi& div, VVlpi& mod){
        assert( (Ulpi)jj < FASTDIV_SAFEMAX ); FOR(i,vl) assert( (Uvlpi)a[i] <= FASTDIV_SAFEMAX );
        FOR(i,vl) div[i] = jj_M * a[i] >> FASTDIV_C;
        FOR(i,vl) mod[i] = a[i] - div[i]*jj;
    };
#endif
    // have_FOO and cnt_FOO : FOO register usage condition and actual use count
    bool const have_vl_over_jj = nloop>1 && vl0%jj==0;
    bool const have_bA_bD = nloop>1 && vl%jj!=0 && jj%vl!=0;
    bool const have_jjMODvl_reset      = (vl0%jj!=0 && jj%vl0==0 && nloop >jj/vl0); // case 'g'
    bool const have_jjMODvl = (vl0%jj!=0 && jj%vl0==0 && nloop>=jj/vl0);
    if( 1 || have_jjMODvl ){
        cout<<" nloop="<<nloop<<" jj/vl0="<<jj/vl0<<endl;
        if( have_jjMODvl ) assert( jj/vl0 > 1 );
        if( have_jjMODvl_reset ) assert( have_jjMODvl );
    }
    bool const have_sq = (jj!=1 && jj<vl0)                              // when iloop==0
        || have_jjMODvl_reset;                                                  // o/w
    bool const have_jj_shift = (jj!=1 && jj<vl0 && positivePow2(jj))    // when iloop==0
        || (nloop>1 && vl0%jj!=0 && jj%vl0!=0 && positivePow2(jj));     // o/w
    bool const have_jj_M = (jj>1 && jj<vl0 && !positivePow2(jj))        // when iloop==0
        || (nloop>1 && vl0%jj!=0 && jj%vl0!=0 && !positivePow2(jj));    // o/w

    int cnt_vl_over_jj=0, cnt_bA_bD=0, cnt_sq=0, cnt_jj_shift=0, cnt_jj_M=0;
    int cnt_jjMODvl=0, cnt_jjMODvl_reset=0;

    Vlpi const vl_over_jj = have_vl_over_jj? vl0/jj: 0;
    Vlpi const jj_over_vl = (have_jjMODvl_reset ? jj/vl0: 0);
    assert( !(have_vl_over_jj && have_jjMODvl) ); // never need both these constants

    int const b_period_max = 8; // how many regs can you spare?
    //int const b_period = unroll_suggest( vl, jj, b_period_max );
    auto u = unroll_suggest( vl,ii,jj, b_period_max, 0/*verbose*/ );
    // This is pinned at [max] vl, even if it may be "inefficient".
    //auto uAlt =
    unroll_suggest(u);

    cout<<" Using "<<u.suggested<<"("<<(int)u.suggested<<") for vl,ii,jj="
        <<vl<<","<<ii<<","<<jj<<endl;

    register uint64_t cnt = 0UL;
    for( ; cnt < iijj; cnt += vl )
    {
#define VL_UP0 0
#define VL_UP 1
        //cout<<"cnt "<<cnt<<" iloop "<<iloop<<" ii "<<ii<<" jj "<<jj<<endl;
        if (iloop == 0){
            if(nloop==1) assert(have_vl_over_jj==0);
            // now load the initial vector-loop registers:
            // sq[i] and jj are < FASTDIV_SAFEMAX, so we can avoid % and / operations
            FOR(i,vl) sq[i] = i;       // vseq_v
            if( jj==1 ){
                if(verbose){cout<<" a";cout.flush();};
                FOR(i,vl) a[i] = i;    // sq/jj
                FOR(i,vl) b[i] = 0;    // sq%jj
                assert(have_bA_bD==0); assert(have_sq==0); assert(have_jj_shift==0);
            }else if(jj>=vl){
                if(verbose){cout<<" b";cout.flush();}
                FOR(i,vl) a[i] = 0;    // sq < vl, so sq/jj < 1
                FOR(i,vl) b[i] = i;
                if(nloop<=1) {assert(have_bA_bD==0); assert(have_sq==0); assert(have_jj_shift==0); }
            }else if( positivePow2(jj) ){
                if(verbose){cout<<" c";cout.flush();}
                // 2 ops (shr, and)
                FOR(i,vl) a[i] = (sq[i] >> jj_shift);  // bD = bA / jj; div_vsv
                FOR(i,vl) b[i] = (sq[i] & jj_minus_1); // bM = bA % jj; mod_vsv
                if(nloop<=1) assert(have_bA_bD==0); assert(have_sq==1); assert(have_jj_shift==1);
                ++cnt_sq; ++cnt_jj_shift;
            }else{
                if(verbose){cout<<" d";cout.flush();}
                // 4 int ops (mul,shr, mul,sub)
                //v_divmod_vs( sq, jj, /*sq[]/jj*/a, /*sq[]%jj*/b );
                FOR(i,vl) a[i] = jj_M * sq[i] >> FASTDIV_C;
                FOR(i,vl) b[i] = sq[i] - a[i]*jj;
                //  OK since sq[] and jj both <= FASTDIV_SAFEMAX [(1<<21)-1]
                assert( (uint64_t)jj+vl <= (uint64_t)FASTDIV_SAFEMAX );
                // use mul_add_shr (fastdiv) approach if jj+vl>FASTDIV_SAFEMAX (1 more vector_add_scalar)
                if(nloop<=1) assert(have_bA_bD==0); assert(have_sq==1); assert(have_jj_shift==0);
                ++cnt_sq; ++cnt_jj_M;
            }
#if VL_UP0
            // Note: vl reduction must take place AFTER above use of "long" vl
            if( cnt + vl > iijj ){ // last time might have reduced vl
                vl = iijj - cnt;
                cout<<" vl reduced for last loop to "<<vl<<endl;
            }
#endif
        }else{
            assert( nloop > 1 );
#if VL_UP==1
            // if careful about vl (possible lower value for last iter) vs vl0 can update here:
            if( cnt + vl > iijj ){ // last time might have reduced vl
                vl = iijj - cnt;
                cout<<" vl reduced for last loop to "<<vl<<endl;
            }
#endif
            // 2. Induction from a->ax, b->bx
            if(vl0%jj == 0){  // avoid div,mod -----1 vec op
                // this includes cases with b_period==1 and high nloops
                if(verbose){cout<<" e";cout.flush();}
                FOR(i,vl) a[i] = a[i] + vl_over_jj;
                ++cnt_vl_over_jj; assert(have_vl_over_jj); assert(!have_bA_bD);
            }else if(jj%vl0 == 0 ){  // -------------1 or 2 vec op (conditional)
                // unroll often nice w/ have_b_period (with maybe more regs)
                assert( have_bA_bD==0); assert(have_jj_shift==0); assert(have_vl_over_jj==0);
                assert( jj > vl0 ); assert( jj/vl0 > 1 ); assert( have_jjMODvl );
                if( !have_jjMODvl_reset ){
                    // Note: this case should also be a "trivial" case for unroll suggestion
                    if(verbose){cout<<" f";cout.flush();}
                    FOR(i,vl) b[i] = b[i] + vl0;
                    ++cnt_jjMODvl; assert( have_jjMODvl );
                }else{
                    // This case is potentially faster with a partial precalc unroll
                    // The division should be done with compute_uB
                    assert( have_jjMODvl && have_jjMODvl_reset );
                    Lpi easy = iloop % jj_over_vl;       // scalar cyclic mod
                    //Lpi easy = fastmod_uB( iloop, jj_over_vl_M, jj_over_vl );
                    // #pragma..unroll(jj/vl) could be branchless
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
                }
                if( have_jjMODvl_reset ) assert( have_jjMODvl );
                assert( !have_bA_bD );
            }else if( positivePow2(jj) ){ // ------4 vec ops (add, shr, and, add)
                if(verbose){cout<<" h";cout.flush();}
                assert( vl0%jj != 0 ); assert( jj%vl0 != 0 ); assert(have_bA_bD==1);
                assert(have_jj_shift==1); assert(have_vl_over_jj==0);
                // no...assert(have_sq==(jj>1&&jj<vl));
                FOR(i,vl) bA[i] = vl0 + b[i];            // bA = b + vl0; add_vsv
                FOR(i,vl) bD[i] = (bA[i] >> jj_shift);  // bD = bA / jj; div_vsv
                FOR(i,vl) b [i] = (bA[i] & jj_minus_1); // bM = bA % jj; mod_vsv
                FOR(i,vl) a [i] = a[i] + bD[i]; // aA = a + bD; add_vvv
                ++cnt_bA_bD; ++cnt_jj_shift; assert( have_bA_bD );
            }else{ // div-mod ---------------------6 vec ops: add (mul,shr) (mul,sub) add
                assert( jj+vl < (1<<21) );
                if(verbose){cout<<" i";cout.flush();}
                if(0){
                    FOR(i,vl) bA[i] = vl0 + b[i];           // add_vsv
                    FOR(i,vl) bD[i] = ((jj_M*bA[i]) >> FASTDIV_C);  // fastdiv_uB   : mul_vvs, shr_vs
                    FOR(i,vl) b [i] = bA[i] - bD[i]*jj;     // long-hand    : mul_vvs, sub_vvv
                    FOR(i,vl) a [i] = a[i] + bD[i];         // add_vvv
                }else if(0){ // as VE alementary ops
                    // Note:
                    // Can ">>32" can be avoided for some jj if we do a u32*u32 multiply.
                    // (Current libdivide and fastdiv suggests not,  check dev code, though)
                    FOR(i,vl) bA[i] = vl0 + b[i];

                    FOR(i,vl) bD[i] = jj_M * bA[i]; // dep -1 simul if bD ?=? bD_prev + jj_M*vl, but would an extra op later
                    FOR(i,vl) bD[i] = bD[i] >> FASTDIV_C;  // dep -1

                    FOR(i,vl) y[i] = bD[i]*jj;     // dep -1
                    FOR(i,vl) b[i] = bA[i] - y[i]; // dep -1

                    FOR(i,vl) a[i] = a[i] + bD[i];  // dep -3
                }else { // re-ordered
                    // Note:
                    // Can ">>32" can be avoided for some jj if we do a u32*u32 multiply.
                    // (Current libdivide and fastdiv suggests not,  check dev code, though)
                    FOR(i,vl) b [i] = vl0 + b[i];
                    FOR(i,vl) bD[i] = jj_M * b [i]; // dep -1 simul if bD ?=? bD_prev + jj_M*vl, but would an extra op later
                    FOR(i,vl) bD[i] = bD[i] >> FASTDIV_C;  // dep -1
                    FOR(i,vl) y[i] = bD[i]*jj;     // dep -1
                    FOR(i,vl) a[i] = a[i] + bD[i];  // dep -2
                    FOR(i,vl) b[i] = b [i] - y[i]; // dep -2
                }
                ++cnt_bA_bD; ++cnt_jj_M;
                assert(have_bA_bD); assert(have_sq==(jj>1&&jj<vl)); assert(!have_jj_shift);
                assert(!have_vl_over_jj); assert( jj+vl < (int)FASTDIV_SAFEMAX );
            }
#if VL_UP==2
            // if confused about vl vs vl0, this is safe (but induce might use too-long vl0)
            if( cnt + vl > iijj ){ // last time might have reduced vl
                vl = iijj - cnt;
                cout<<" vl reduced for last loop to "<<vl<<endl;
            }
#endif
        }
#if !VLUP0 && !VL_UP
        // Note: vl reduction must take place AFTER above use of "long" vl
        if( cnt + vl > iijj ){ // last time might have reduced vl
            vl = iijj - cnt;
            cout<<" vl reduced for last loop to "<<vl<<endl;
        }
#endif

        if(verbose>=1){
            int const n=8; // output up-to-n [ ... [up-to-n]] ints
            int const bignum = std::max( ii, jj );
            int const wide = 1 + (bignum<10? 1: bignum <100? 2: bignum<1000? 3: 4);
            cout<<"a["<<vl<<"]="<<vecprt(n,wide,a,vl)<<endl;
            cout<<"b["<<vl<<"]="<<vecprt(n,wide,b,vl)<<endl;
        }

        assert( vl == vabs[iloop].vl );
        FOR(i,vl) assert( a[i] == vabs[iloop].a[i] );
        FOR(i,vl) assert( b[i] == vabs[iloop].b[i] );
        // Alt. is a hash-value test:
        //assert( Vab::rehash(a,b,vl,0) == vabs[iloop].hash );
        //assert( Vab::rehash(a,b,vl) == vabs[iloop].hash );
        assert( vhash.hash_combine(a.data(),b.data(),vl) == vabs[iloop].hash );

        ++iloop; // just for above debug assertions
        //cout<<" next loop??? cnt+vl="<<cnt+vl<<" iijj="<<iijj<<endl;
#undef FOR
    }
    cout<<" Yay! induction formulas worked! iloop,nloop="<<iloop<<","<<nloop<<endl;
    assert( cnt == iijj );
    assert( nloop == iloop );
    //if( nloop > 1 && vl%jj!=0 && jj%vl==0 ) assert( count_special > 0 );
    assert( have_vl_over_jj     == (cnt_vl_over_jj    > 0) );
    assert( have_bA_bD          == (cnt_bA_bD         > 0) );
    assert( have_sq             == (cnt_sq            > 0) );
    assert( have_jj_shift       == (cnt_jj_shift      > 0) );
    assert( have_jj_M           == (cnt_jj_M          > 0) );
    assert( have_jjMODvl        == (cnt_jjMODvl       > 0) ); // old "special" count, case 'g' needed
    assert( have_jjMODvl_reset  == (cnt_jjMODvl_reset > 0) ); // old "special" count, case 'g' needed
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
/** helper routine, after using unroll_suggest for a good VL \c vl0. */
std::string cfuse2_no_unroll(Lpi const vl0, Lpi const ii, Lpi const jj,
        int const which=WHICH_KERNEL, int const verbose=1){
    ostringstream oss;
    uint64_t iijj = (uint64_t)ii * (uint64_t)jj;
    uint64_t const vl = min(iijj,(uint64_t)vl0);
    int const nloop = (iijj+vl-1U) / vl;        // div_round_up(iijj,vl)
    string ret;
    Cunit pr("cfuse2_no_unroll","C",0/*verbose*/);
    auto& inc = pr.root["includes"];
    inc >>"#include \"veintrin.h\""
        >>"#include <stdint.h>";
    auto& fns = pr.root["fns"];
    fns["first"]; // reserve a node for optional function definitions
    CBLOCK_SCOPE(cfuse2,"int main(int,char**)",pr,fns["main"]);
    if(nloop==0){
        cfuse2>>OSSFMT("// for(0.."<<ii<<")for(0.."<<jj<<") --> NOP");
    }else{
        string pfx=OSSFMT("CFUSE2_NO_UNROLL_"<<vl<<"_"<<ii<<"_"<<jj);
        cfuse2 .DEF(pfx) .DEF(vl0) .DEF(ii) .DEF(jj) ;

        auto& fd = cfuse2["../first"]; // definitions
        string alg_descr=OSSFMT("// cfuse2_no_unroll: vl,ii,jj="<<vl<<","<<ii<<","<<jj
                <<" nloop="<<nloop<<(jj==1? " jj==1": vl0%jj==0? " vl%jj==0"
                    : jj%vl==0? " jj%vl==0": positivePow2(jj)? "jj=2^N" : ""));
        fd>>alg_descr;
        fd>>OSSFMT("_ve_lvl(vl0);  // VL = "<<vl0);

        // some kernels might ask for const sq reg to always be defined
        auto have_sq = [&fd](){ return fd.find("have_sq") != nullptr; };
        auto have_sqij = [&fd](){ return fd.find("have_sqij") != nullptr; };
        auto use_sq = [&fd,have_sq,have_sqij,&oss](){
            // if nec, define const sequence register "sq"
            if(!have_sq()){
                if(have_sqij())
                    fd["first"]["sq"]>>OSSFMT(left<<setw(40)<<"__vr const sq = sqij;")<<" // sq[i]=i";
                else
                    fd["first"]["sq"]>>OSSFMT(left<<setw(40)<<"__vr const sq = _ve_vseq_v();")<<" // sq[i]=i";
                fd["have_sq"].setType("TAG");
            }
        };
        auto use_sqij = [&fd,have_sq,have_sqij,&oss](){
            // if nec, define non-const sequence register "sq"
            if(!have_sqij()){
                if(have_sq())
                    fd["last"]["sqij"]>>OSSFMT(left<<setw(40)<<"__vr sqij = sq;")
                        <<" // sqij[i]=i";
                else
                    fd["last"]["sqij"]>>OSSFMT(left<<setw(40)<<"__vr sqij = _ve_vseq_v();")
                        <<" // sqij[i]=i";
                fd["have_sqij"].setType("TAG");
            }
        };
        auto equ_sq = [have_sq,have_sqij](std::string v){
            return v+(have_sq() ? " = sq;          "
                    : have_sqij() ? " = sqij;        " // only OK pre-loop !
                    : " = _ve_vseq_v();"); };
        auto kernComment = [have_sq,ii,jj](){
            return have_sq()? "sq[]=0.."+jitdec(ii*jj-1): "";
        };
        auto mk_divmod = [&](){ mk_DIVMOD(cfuse2,jj,iijj+vl0); };
        auto use_vl = [&fd](){
            if(! fd.find("have_vl")){
                fd["last"]["vl"]>>"int64_t vl = vl0;";
                fd["have_vl"].setType("TAG");
            }
        };

        auto& fp = cfuse2["preloop"]; // pre-loop setup
        bool fp_sets_ab = false;
        if(which==KERNEL_PRINT || which==KERNEL_CHECK || which==KERNEL_HASH)
            use_vl();
        if(which==KERNEL_SQIJ)
            use_sqij();
        if(nloop==1){
            auto& fk = fp["krn"];
            auto& fz = cfuse2["last"]["krn"];
            if(which==KERNEL_CHECK && !fd.find("have_cnt")){
                //fd>>"int64_t const cnt=0; /*simulate for-loop index for kernel";
                fd.define("cnt","0");
                fd["have_cnt"].setType("TAG");
            }
            if(which==KERNEL_SQIJ) use_sqij();
            //&& !fd.find("have_sqij")){
            //    fd>>equ_sq("__vr sqij");
            //    fd["have_sqij"].setType("TAG");
            //}
            fz>>"// [krn output]";
            fuse2_kernel(fk, fd, fz, ii,jj,vl, kernComment(), which);
            fp_sets_ab = true;
        }
        if(nloop>1){
            fp_sets_ab = true;
            fd["have_cnt"].setType("TAG");
            // && !fd.find("have_sqij")){
            //    fd>>equ_sq("__vr sqij");
            //    fd["have_sqij"].setType("TAG");
            //}
            CBLOCK_FOR(loop_ab,0,"for(int64_t cnt=0 ; /*cnt<iijj*/; /*cnt+=vl0*/)",cfuse2);
            auto& ff = loop_ab["../first"];
            auto& fk = loop_ab["krn"];
            auto& fz = cfuse2["last"]["krn"];
            fz>>"// [krn output]";
            fuse2_kernel(fk, fd, fz, ii,jj,vl, kernComment(), which);

            // manage a,b induction, vl change (loop exit?)
            auto& fi = loop_ab["iter"];
            //fi>>OSSFMT("// CFUSE2_NO_UNROLL_"<<vl<<"_"<<ii<<"_"<<jj);
            if(iijj%vl0){ // last iter has reduced VL
                use_vl();
                fi  >>"if(vl!=vl0) break;                       // easy 'done' test"
                    >>"cnt += vl0;                              // loop_ab/iter"
                    >>"vl = (vl0<iijj-cnt? vl0: iijj-cnt);      // vl = min(vl0,remain)"
                    >>"_ve_lvl(vl);";
            }else{
                fi  >>"cnt += vl0;                              // loop_ab/iter"
                    >>"if((uint64_t)cnt>=iijj) break;           // hits iijj exactly";
            }
            if(vl0%jj==0){                      // avoid div,mod -- 1 vec op
                int64_t vlojj = vl0/jj;
                cfuse2 .DEF(vlojj);
                fi  >>"a =_ve_vadds_vsv(vlojj, a);              // a[i]+="<<jitdec(vl0/jj)<<", b[] same";
            }else if(jj%vl0==0){
                if(nloop<=jj/vl0){ // !have_jjMODvl_reset
                    fi>>OSSFMT(left<<setw(40)<<OSSFMT("b = _ve_vadds_vsv("<<vl0<<",b);")
                               <<" // b[] += vl0, a[] const");
                }else{ // various nice ways to do periodic reset... [potentially better with unroll!]
                    fp["tmod"]>>"uint32_t tmod=0U; // loop induction periodic";
                    if(jj/vl0==2){
                        fi>>"tmod = ~tmod";
                    }else if(positivePow2(jj/vl0)){
                        uint64_t const shift = positivePow2Shift((uint32_t)(jj/vl0));
                        uint64_t const mask  = (1ULL<<shift) - 1U;
                        fi>>"tmod = (tmod+1) & "<<jithex(mask)<<"; // cyclic power-of-2 counter";
                    }else{
                        fi  >>OSSFMT(left<<setw(40)<<"++tmod;"<<" // tmod period jj/vl0 = "<<jj/vl0)
                            >>OSSFMT(left<<setw(40)<<OSSFMT("if(tmod=="<<jj/vl0<<") tmod=0;")
                                    <<" // cmov reset tmod=0?");
                    }
                    fi  >>"if(tmod){" // using mk_scope or CBLOCK_SCOPE is entirely optional...
                        >>"    b = _ve_vaddul_vsv(vl0,b);           // b[] += vl0 (easy, a[] unchanged)"
                        >>"}else{"
                        >>"    a = _ve_vaddul_vsv(1,a);             // a[] += 1"
                        >>"    b = sq;                              // b[] = sq[] (reset case)"
                        >>"}";
                }
#if 0
            }else if(positivePow2(jj)){
                // code block identical to full recalc DIVMOD case (mk_divmod now recognizes jj=2^N)
                //
                // induction from prev a,b is longer than full recalc!
                //  sqij = _ve_vaddul_vsv(vl0,sqij);
                //  a = _ve_vsrl_vvs(sqij, jj_shift);              // a[i]=sq[i]/jj
                //  b = _ve_vand_vsv("<<jithex(jj_minus_1)<<",sq); // b[i]=sq[i]%jj
                //def_sqij=true;
                if(jj<vl0){ // same as INIT_BLOCK code
                    fp_sets_ab = false;
                    fp>>equ_sq("__vr sqij");
                    mk_divmod();
                    ff>>OSSFMT("DIVMOD_"<<jj<<"(__vr a,__vr b,sqij); //  a[]=sq/"<<jj<<" b[]=sq%"<<jj);
                    fi>>"sqij = _ve_vaddul_vsv(vl0,sqij);      // sqij[i] += "<<jitdec(vl0);
                }else{
                    mk_divmod();
                    fp>>equ_sq("__vr sqij");
                    fi  >>"sqij = _ve_vaddul_vsv(vl0,sqij);      // sqij[i] += "<<jitdec(vl0)
                        >>OSSFMT("DIVMOD_"<<jj<<"(a,b,sqij); //  a[]=sq/"<<jj<<" b[]=sq%"<<jj);
                }
#endif
            }else{
                use_sqij();
                //if(!fd.find("have_sqij")){
                //    fd>>equ_sq("__vr sqij");
                //    fd["have_sqij"].setType("TAG");
                //}
                if(jj<vl0){ // same as INIT_BLOCK code
                    fp_sets_ab = false;
                    mk_divmod();
                    ff>>OSSFMT("DIVMOD_"<<jj<<"(__vr a,__vr b,sqij); //  a[]=sq/"<<jj<<" b[]=sq%"<<jj);
                    fi>>OSSFMT(left<<setw(40)<<"sqij = _ve_vaddul_vsv(vl0,sqij);")
                        <<" // sqij[i] += "<<jitdec(vl0);
                }else{
                    mk_divmod();
                    fi  >>OSSFMT(left<<setw(40)<<"sqij = _ve_vaddul_vsv(vl0,sqij);")
                        <<OSSFMT(" // sqij[] += "<<vl0)
                        >>OSSFMT(left<<setw(40)<<"DIVMOD_"<<jj<<"(a,b,sqij);")
                        <<OSSFMT(" //  a[]=sq/"<<jj<<" b[]=sq%"<<jj);
                }
            }
        }
        if(fp_sets_ab){
            std::string vREG=(nloop<=1?"__vr const ":"__vr ");
            if( jj==1 ){
                fp>>equ_sq(vREG+"a")<<"         // a[i] = i";
                fp>>vREG<<"b = _ve_vbrd_vs_i64(0LL); // b[i] = 0"; // libvednn way
                //fp>>vREG<<"b; b=_ve_vxor_vvv(b,b);"; // typically "best way"
            }else if(jj>=vl0){
                fp>>vREG<<"a = _ve_vbrd_vs_i64(0LL); // a[i] = 0";
                fp>>equ_sq(vREG+"b")<<"         // b[i] = i";
            }else{ // note: mk_divmod also optimizes positivePow2(jj) case
                mk_divmod();
                use_sq();
                fp>>OSSFMT(left<<setw(40)<<OSSFMT("DIVMOD_"<<jj<<"("<<vREG<<"a, "<<vREG<<"b, sq);"))
                    <<OSSFMT(" // a[]=sq/"<<jj<<" b[]=sq%"<<jj);
            }
        }
    }
    return pr.str();
}
// for r in [0,h){ for c in [0,w] {...}
void test_vloop2_no_unrollX(Lpi const vlen, Lpi const ii, Lpi const jj,
        int const opt_t, int const which=WHICH_KERNEL, char const* ofname=nullptr){
    ostringstream oss; // reusable allocation, for CBLK and OSSFMT macros
    ExecHash tr;
    //CodeGenAsm cg;
    //std::string code;
    int const b_period_max = 8; // how many regs can you spare?

    // This is pinned at [max] vl, even if it may be "inefficient".
    auto u = unroll_suggest( vlen,ii,jj, b_period_max );
    int vl = u.vl;

    auto uAlt = unroll_suggest(u);  // suggest an alternate nice vector length, sometimes.
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
        register uint64_t iijj = (uint64_t)ii * (uint64_t)jj;
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

#define FOR(I,VL) for(int I=0;I<VL;++I)

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

#if STYLE==STYLE_GOTO
        register uint64_t cnt = iijj; // goto counts iijj to one {i.e. cnt = remainder}
#else
        register uint64_t cnt = 0; // for loop counts 0 .. iijj-1
#endif

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
#if STYLE==STYLE_GOTO
        auto& fp = cfuse2["init"];        //fp>>"//cfuse2 presets";
        auto& fi = cfuse2["induce"]["body"];
        CBLOCK_SCOPE(fk,"",pr,fi);
#else
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
#endif
        //auto& fl = cfuse2["last"]["first"];
        auto& fz = cfuse2["last"]["first"];

        inc >>"#include \"veintrin.h\""
            >>"#include \"stdint.h\""
            ;
        // local labels:
        string fusename = OSSFMT("fuse2_"<<vl<<"_"<<ii<<"_"<<jj);
        cfuse2
            //.def("BASE", "L_"+fusename+"_BASE")
            //.def("INDUCE", "L_"+fusename+"_INDUCE")
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
        bool def_cnt = false; // track multiple cases requiring same var defined
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
            //std::string vREG=(STYLE==STYLE_GOTO? "__vr ":"");
#if STYLE!=STYLE_GOTO
            if(nloop>1) fd>>"__vr a, b; // vector loop index registers";
#endif
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
#if STYLE==STYLE_GOTO
            if(nloop > 1) {
                fp>>"goto KERNEL_BLOCK; // STYLE 0 ==> use goto";
            }
#endif
        }
        //cout<<" fp.cnt="<<cnt;
        goto KERNEL_BLOCK;
        // INDUCE-BLOCK
INDUCE:
        assert( nloop > 1 );
        if( iijj % vl0 ){ // special loop condition case
            tr+="induce:iijj%vl0 cnt update";
            def_cnt = true; // later we'll define cnt = ii*jj
#if STYLE==STYLE_GOTO
            cnt -= vl0; // vl0 is compile-time const, so preferred
#else
            // JIT does this within 'for' statement... this code is using goto style
            cnt += vl0; // (or vl, to guarantee hitting iijj exactly)
#endif
            //if(onceI) fi.ins("addu.l cnt, vl0, cnt", "cnt 0..iijj-1");
            //cout<<" fi.cnt="<<cnt;
            tr+="induce:iijj%vl0 last iter EARLY lower vl";
            // Careful: vl = new value for last iteration, vl0 = original = old value
            // VE has single-op MIN
            //vl = std::min( (uint64_t)vl, /*remain =*/ iijj - cnt );
#if STYLE==STYLE_GOTO
            vl = std::min( (uint64_t)vl, /*remain =*/ cnt );
#else
            vl = std::min( (uint64_t)vl, /*remain =*/ iijj-cnt );
#endif
            if(onceI){
                if(!fd.find("have_vl")){
                    fd  >>"int64_t vl = vl0;";
                    fd["have_vl"].setType("TAG");
                }
                if(STYLE==STYLE_GOTO){
                    fi  >>"INDUCE:    // (ii*jj)%vl0 != 0 --> change VL for last iteration"
                        >>"cnt -= vl0;"
                        >>"vl = (vl0<cnt? vl0: cnt); // vl = min(vl0,cnt) for last iter,"
                        >>"_ve_lvl(vl);              //";
                }else{
                    fi  >>"// (ii*jj)%vl0 != 0 --> last iter VL change"
                        >>"vl = (vl0<iijj-cnt? vl0: iijj-cnt); // vl = min(vl0,iijj-cnt)"
                        >>"_ve_lvl(vl);";
                }
            }
        }
        // 2. Induction from a->ax, b->bx
        if(vl0%jj == 0){  // avoid div,mod -----1 vec op
            tr+="induce:vl0%jj a[]+=vl/jj, b[] const";
            // this includes cases with b_period==1 and high nloops
            if(verbose){cout<<" e";cout.flush();}
            FOR(i,vl) a[i] = a[i] + vl_over_jj;
            ++cnt_vl_over_jj; assert(have_vl_over_jj); assert(!have_bA_bD);
            if(onceI && STYLE==STYLE_GOTO && fi[".."].code_str().empty() ) fi[".."]>>"INDUCE:    // vl0%jj==0";
            if(onceI){
                // VADD vx, vy/sy/I, vz [,vm]
                //fi.ins("vadd a, "+(vl/jj>127?"vlojj":jitdec(vl/jj))+",a"
                //        ,        "a[i] += (vl/jj="+jitdec(vl0/jj)+"), b[] unchanged");
                if(vl/jj>127){
                    fd>>OSSFMT("int64_t const vlojj = "<<vl0/jj<<"; // vl/jj="<<vl<<"/"<<jj);
                }else{
                    fd.define("vlojj",OSSFMT(vl0/jj<<"/*vl/jj*/"));
                }
                fi>>OSSFMT("a =_ve_vadds_vsv(vlojj, a);"
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
                if(onceI && STYLE==STYLE_GOTO && fi[".."].code_str().empty() ) fi[".."]>>"INDUCE:    // jj%vl0 == 0";
                if(onceI) //fi.ins("add b, "+jitdec(vl0)+",b",    "b[i] += vl0, a[] const");
                /**/ CBLK(fi,"b = _ve_vadds_vsv("<<jitdec(vl0)<<",b); // b[i] += vl0, a[] const");
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
                        if(onceI && STYLE==STYLE_GOTO && fi[".."].code_str().empty() ) fi[".."]>>"INDUCE:    // period jj/vl0=2";
                        fi<<"tmod=~tmod;";
                    }else if(positivePow2(jj/vl0)){     // cyclic power-of-2 counter
                        if(onceI && STYLE==STYLE_GOTO && fi[".."].code_str().empty() ) fi[".."]>>"INDUCE:    // period jj/vl0=2^N";
                        uint64_t const shift = positivePow2Shift((uint32_t)(jj/vl0));
                        uint64_t const mask  = (1ULL<<shift) - 1U;
                        CBLK(fi,"tmod = (tmod+1) & "<<jithex(mask)<<"; // cyclic power-of-2 counter");
                    }else{                              // cyclic period jj/vl0 counter
                        if(onceI && STYLE==STYLE_GOTO && fi[".."].code_str().empty() ) fi[".."]>>OSSFMT("INDUCE:    // period jj/vl0="<<jj/vl0);
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
            FOR(i,vl) bA[i] = vl0 + b[i];           // bA = b + vl0; add_vsv
            FOR(i,vl) bD[i] = (bA[i] >> jj_shift);  // bD = bA / jj; div_vsv
            FOR(i,vl) b [i] = (bA[i] & jj_minus_1); // bM = bA % jj; mod_vsv
            FOR(i,vl) a [i] = a[i] + bD[i]; // aA = a + bD; add_vvv
            ++cnt_bA_bD; ++cnt_jj_shift; assert( have_bA_bD );
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
#elif 1
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
            uint64_t cnt_incr = (STYLE==STYLE_GOTO? iijj-cnt: cnt);
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
#if STYLE==STYLE_GOTO
            //fi.lcom("...KERNEL_BLOCK fall-through, a[], b[] induced");
            fi>>"// ...KERNEL_BLOCK fall-through, a[], b[] induced";
#endif
            onceI = false;
        }

KERNEL_BLOCK:
        tr+="KERNEL_BLOCK";
        if(onceK){
            //cout<<"=== // #KERNEL_BLOCK: (fallthrough)\n"
            //<<"=== //        # <your code here, using a[] b[] loop-index vectors\n";
#if STYLE==STYLE_GOTO
            if(nloop > 1) fk[".."]<<"KERNEL_BLOCK:";
#endif
            fuse2_kernel(fk, fd, fz, ii,jj,vl, kernComment(), which);
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
#if STYLE==STYLE_GOTO
            cnt -= vl0;
            //cout<<" C cnt="<<cnt<<endl;
            if( (int64_t)cnt > 0 ){
                tr+="loop: induce next a[] b[] if cnt-=vl0 still positive";
                //cout<<" again B cnt="<<cnt<<endl;
                goto INDUCE;
            }
#else
            cnt += vl0;
            //cout<<" C cnt="<<cnt<<endl;
            if( (uint64_t)cnt < iijj ){
                tr+="loop: induce next a[] b[] if cnt < iijj";
                //cout<<" again B cnt="<<cnt<<endl;
                goto INDUCE;
            }
#endif
            tr+="exiting C";
            //cout<<" exit C cnt="<<cnt<<endl;
#endif
        }

        if(onceL){
#if STYLE==STYLE_GOTO // goto form can do trickier loop exits to prune some ops
            if(nloop==1) fl>>"// (nloop == 1, no need to check if done)";
            else if(iijj%vl0){
                //fl.lcom("DONE_CHECK A : loop again if vl is not the reduced last value");
                //fl.ins("breq"+string(nloop>2?".t":".nt")+" vl,vl0,"+reladdr("INDUCE"), "iijj%vl!=0 so vl<vl0 means DONE");
                fl  >>"// DONE_CHECK A : loop again if vl is not the reduced last value"
                    >>"if(vl==vl0) goto INDUCE; // iijj%vl!=0 so vl<vl0 means DONE";
            }else{
                //fl.lcom("DONE_CHECK B : loop again if next cnt still < iijj");
                //  fl.ins("add cnt,vl0,cnt",  "cnt += vl");
                //      .ins("brlt"+string(nloop>2?".t":".nt")+" cnt,iijj, INDUCE-.", "next a[],b[]");
                //fl.ins("subu cnt,vl0,cnt",  "cnt -= vl (or vl0)");
                //fl.ins("bgt"+string(nloop>2?".t":".nt")+" cnt,"+reladdr("INDUCE"), "next a[],b[]");
                fl  >>"DONE_CHECK B : loop again if next cnt still < iijj"
                    >>"cnt -= vl; // (or vl0)"
                    >>"if(cnt > 0) goto INDUCE; // next a[],b[]";
                def_cnt=true;
            }
#endif
            onceL = false;
        }
        // define/initialize 'def_' needed variables...
        if(STYLE==STYLE_GOTO && def_cnt){
            fd  >>"int64_t cnt = (uint64_t)ii * (uint64_t)jj;"
                <<OSSFMT(" // cnt=ii*jj="<<ii<<"*"<<jj<<" to 1? by "<<vl0);
        }
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
        }else if(nloop<=1){
            ;
        }else{
#if STYLE==STYLE_GOTO
#else
            fp["../beg"].set("if(cnt==0){");
#endif
        }

        // LOOP_DONE ...
        if(SAVE_RESTORE_VLEN){
            //fz.ins("lvl vl_save","load VL <-- vl_save (restore VL on exit)");
            fz  >>"//_ve_lvl(vl_save); // restore VL on exit XXX when SVL op is supported!!!";
        }

#undef FOR
        cout<<" Yay! induction formulas worked! iloop,nloop="<<iloop<<","<<nloop<<endl;
        //assert( cnt == iijj || cnt == iijj/cnt*cnt );
#if STYLE==STYLE_GOTO
        assert( cnt == 0 || (nloop<=1 && cnt == iijj) || (nloop>1 && cnt == iijj - iijj/vl0*vl0) ); //iijj/vl0*vl0 + vl0);
#else
        cout<<" end with cnt="<<cnt<<" cf. nloop*vl0="<<nloop*vl0<<endl;
        cout<<" end with cnt="<<cnt<<" cf. (nloop-1)*vl0="<<(nloop-1)*vl0<<endl;
        cout<<" end with cnt="<<cnt<<" cf. iijj/vl0*vl0="<<iijj/vl0*vl0<<endl;
        assert( (nloop<=1 && cnt==0) || (nloop>1 && (uint64_t)cnt==iijj/vl0*vl0) );
#endif
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
        ofs<<program2;
        ofs<<"// vim: ts=2 sw=2 et cindent\n";
        // cino=^=l0,\\:.5s,=-.5s,N-s,g.5s,b1 cinkeys=0{,0},0),\\:,0#,!^F,o,O,e,0=break\n"
        ofs.close();
        cout<<"// Written to file "<<ofname<<endl;
    }
}

/** opt0: print vector ops (and verify) */
void test_vloop2_unroll(Lpi const vlen, Lpi const ii, Lpi const jj)
{
    // for r in [0,h){ for c in [0,w] {...}}
    assert( vlen > 0 );
    register uint64_t iijj = (uint64_t)ii * (uint64_t)jj;
    cout<<"test_vloop2_unroll( vlen="<<vlen<<" loops 0.."<<ii<<" 0.."<<jj<<" iijj="<<iijj<<endl;

    // pretty-printing via vecprt
    int const n=8; // output up-to-n [ ... [up-to-n]] ints
    int const bignum = std::max( ii, jj );
    int const wide = 1 + (bignum<10? 1: bignum <100? 2: bignum<1000? 3: 4);

    // generate reference index outputs
    std::vector<Vab> vabs = ref_vloop2(vlen, ii, jj, 1/*verbose*/);

    register int vl = vlen;
    register uint64_t cnt=0;
    register uint64_t nxt;
    cout<<"=== // unrolled regs:"<<endl;
    cout<<"=== //        %iijj    : scalar : outer * inner fused-loop count"<<endl;
    cout<<"=== //        %cnt     : scalar : count 0..iijj-1"<<endl;
    cout<<"=== //        %vl      : scalar : vector length register"<<endl;
    cout<<"=== //        %a, %b   : vector : outer, inner loop indices"<<endl;
    cout<<"=== //        %bA, %bD : vector : tmp regs"<<endl;
    //if (vl > iijj) vl = iijj; //in general: if (cnt+vl > iijj) vl=iijj-cnt;
    // later! cout<<"===   lea %vl, "<<vlen<<"(,0) // initial vector len"<<endl;
    cout<<"===   lea %iijj, 0,"<<ii<<"(,0)"<<endl;
    cout<<"===   lea %vl,   0,"<<jj<<"(,0) // vl used as tmp reg"<<endl;
    cout<<"===   mulu.l %iijj, %iijj, %vl  // opt last 3 for small ii, jj !"<<endl;
    cout<<"===   or %cnt, 0, 0(,0)"<<endl;

    VVlpi a(vl), b(vl);   // vectorized loop indices
    VVlpi bA(vl), bD(vl); // local temporaries
#define FOR(I,VL) for(int I=0;I<VL;++I)
    int iloop = 0; // just for debug checks, now;
    for( ; cnt < iijj; cnt += vl)
    {
        if (cnt == 0){
            // now load the initial vector-loop registers:
            FOR(i,vl) bA[i] = i;         // vseq_v
            cout<<"===   vseq %bA           // (tmp)"<<endl;
            FOR(i,vl) b [i] = bA[i] % jj;
            FOR(i,vl) a [i] = bA[i] / jj;
            cout<<"===   vmod %b, %bA, "<<jj<<"     // b = bA % jj"<<endl;
            cout<<"===   vdiv %a, %bA, "<<jj<<"     // a = bA / jj"<<endl;
        }else{
            // 2. Induction from a->ax, b->bx
            FOR(i,vl) bA[i] = vl + b[i];      // bA = b + vl; add_vsv
            FOR(i,vl) b [i] = bA[i] % jj;     // bM = bA % jj; mod_vsv
            FOR(i,vl) bD[i] = bA[i] / jj;     // bD = bA / jj; div_vsv
            FOR(i,vl) a [i] = a[i] + bD[i];   // aA = a + bD; add_vvv
            cout<<"===   vadd %bA, %vl, %b     // bA[i] = vl + b[i] (tmp)"<<endl;
            cout<<"===   vmod %b , %bA, "<<jj<<"     // b[i] = bA[i] % jj"<<endl;
            cout<<"===   vdiv %bD, %bA, "<<jj<<"     // bD[i] = bA[i]/ jj (tmp)"<<endl;
            cout<<"===   vadd %a , %a , %bD    // a[i] += bD[i]"<<endl;
        }

        nxt = cnt+vl;
        if( nxt > iijj){ // last loop!
            vl = iijj - cnt;
            if (cnt > 0){
                cout<<"=== lea    %vl, "<<vl<<"(,0)  // LAST TIME"<<endl;
                cout<<"=== lvl    %vl"<<endl;
            }
        }

        //cout<<"==="<<endl;
        cout<<"=== // vec_loop2 unroll#"<<iloop<<", indices in %a, %b"<<endl;
        //cout<<"==="<<endl;

        cout<<"__"<<iloop<<endl;
        cout<<"Induce:      "<<vecprt(n,wide,a,vl)<<endl;
        cout<<"             "<<vecprt(n,wide,b,vl)<<endl;
        FOR(i,vl) assert( a[i] == vabs[iloop].a[i] );
        FOR(i,vl) assert( b[i] == vabs[iloop].b[i] );
        ++iloop; // just for above debug assertions
#undef FOR
    }
    cout<<" Yay! induction formulas worked!"<<endl;
}
void test_vloop2_no_unroll(Lpi const vlen, Lpi const ii, Lpi const jj)
{
    // for r in [0,h){ for c in [0,w] {...}}
    assert( vlen > 0 );
    cout<<"test_vloop2_nounroll( vlen="<<vlen<<" loops 0.."<<ii<<" 0.."<<jj<<endl;

    // pretty-printing via vecprt
    int const n=8; // output up-to-n [ ... [up-to-n]] ints
    int const bignum = std::max( ii, jj );
    int const wide = 1 + (bignum<10? 1: bignum <100? 2: bignum<1000? 3: 4);

    // generate reference index outputs
    std::vector<Vab> vabs = ref_vloop2(vlen, ii, jj, 1/*verbose*/);

    cout<<"=== // no-unroll regs (generic loop):"<<endl;
    cout<<"=== //        %iijj    : scalar : outer * inner fused-loop count"<<endl;
    cout<<"=== //        %cnt     : scalar : count 0..iijj-1"<<endl;
    cout<<"=== //        %vl      : scalar : vector length register"<<endl;
    cout<<"=== // Oh. Fully generic would need ii and jj in scalar regs too"<<endl;
    cout<<"=== //                 (ii could be re-used for iijj)"<<endl;
    cout<<"=== //        %a, %b   : vector : outer, inner loop indices"<<endl;
    cout<<"=== //        %bA, %bD : vector : tmp regs"<<endl;
    cout<<"=== // scalar init:"<<endl;
    register uint64_t iijj = (uint64_t)ii * (uint64_t)jj;
    register int vl = vlen;
    register uint64_t cnt = 0UL;
    if ((uint64_t)vl > iijj) vl = iijj; //in general: if (cnt+vl > iijj) vl=iijj-cnt;
    cout<<"===   lea %iijj, 0,"<<ii<<"(,0)"<<endl;
    cout<<"===   lea %vl,   0,"<<jj<<"(,0) // vl used as tmp reg"<<endl;
    cout<<"===   mulu.l %iijj, %iijj, %vl  // opt last 3 for small ii, jj !"<<endl;
    cout<<"===   or %cnt, 0, 0(,0)"<<endl;
    cout<<"===   lea %vl, "<<vlen<<"(,0) // initial vector len"<<endl;

    register uint64_t nxt; // loop variable (convenience) XXX
#define FOR(I,VL) for(int I=0;I<VL;++I)
    cout<<"=== // vector init:"<<endl;
    VVlpi a(vl), b(vl);   // vectorized loop indices
    VVlpi bA(vl), bD(vl); // local temporaries
    // now load the initial vector-loop registers:
    FOR(i,vl) bA[i] = i;         // vseq_v
    FOR(i,vl) b [i] = bA[i] % jj;
    FOR(i,vl) a [i] = bA[i] / jj;
    cout<<"===   vseq %bA           // (tmp)"<<endl;
    cout<<"===   vmod %b, %bA, "<<jj<<"     // b = bA % jj"<<endl;
    cout<<"===   vdiv %a, %bA, "<<jj<<"     // a = bA / jj"<<endl;
    cout<<"===   br loop_enter"<<endl;
    int iloop = 0; // just for debug checks, now;
    cout<<"=== again:              // <-- repeat loop (induce loop var)"<<endl;
again:
    nxt = cnt+vl;
    if( nxt > iijj) vl = iijj - cnt;
    if (iloop == 0){ // only need to print stuff 1st time through loop
        cout<<"===   addu.l %tmp, %cnt, %vl"<<endl;
        cout<<"===   subu.l %tmp, %tmp, %iijj"<<endl;
        cout<<"===   br.lt %tmp, L_1f-."<<endl;
        cout<<"===     or %cnt, 0, 1(0)  // re-use cnt to signal loop-end"<<endl;
        cout<<"===     subu %vl, %iijj, %cnt"<<endl;
        cout<<"===     lvl  %vl"<<endl;
        cout<<"=== L_1f:"<<endl;
    }

    //cout<<"==="<<endl;
    cout<<"=== // vec_loop2 #"<<iloop<<", indices in %a, %b"<<endl;
    //cout<<"==="<<endl;

    cout<<"__"<<iloop<<endl;
    cout<<"Induce:      "<<vecprt(n,wide,a,vl)<<endl;
    cout<<"             "<<vecprt(n,wide,b,vl)<<endl;
    FOR(i,vl) assert( a[i] == vabs[iloop].a[i] );
    FOR(i,vl) assert( b[i] == vabs[iloop].b[i] );
    cnt += vl; // or cnt = nxt;
    if (cnt >= iijj) goto done;
    if (iloop == 0){
        cout<<"=== // if ((cnt+=vl) > iijj) goto done;"<<endl;
        cout<<"===   addu.l %cnt, %cnt, %vl"<<endl;
        cout<<"===   subu.l %tmp, %cnt, %iijj"<<endl;
        cout<<"===   br.ge done"<<endl;
    }
    // 2. Induction from a->ax, b->bx
    FOR(i,vl) bA[i] = vl + b[i];      // bA = b + vl; add_vsv
    FOR(i,vl) b [i] = bA[i] % jj;     // bM = bA % jj; mod_vsv
    FOR(i,vl) bD[i] = bA[i] / jj;     // bD = bA / jj; div_vsv
    FOR(i,vl) a [i] = a[i] + bD[i];   // aA = a + bD; add_vvv
    if (iloop==0){
        cout<<"=== // Induce next a, b vector loop indices"<<endl;
        cout<<"===   vadd %bA, %vl, %b     // bA[i] = vl + b[i]"<<endl;
        cout<<"===   vmod %b , %bA, "<<jj<<"   // b[i] = bA[i] % jj"<<endl;
        cout<<"===   vdiv %bD, %bA, "<<jj<<"   // bD[i] = bA[i]/ jj"<<endl;
        cout<<"===   vadd %a , %a , %bD    // a[i] += bD[i]"<<endl;
    }
    ++iloop; // just for above debug assertions
    goto again;
#undef FOR
done:
    cout<<"=== done:"<<endl;
    cout<<" Yay! induction formulas worked!"<<endl;
}

int main(int argc,char**argv){
    int vl = 8;
    int h=20, w=3;
    int opt_t=1, opt_u=0, opt_l=0, opt_h=0, opt_m=0;
    int a=0;
    char *ofname = nullptr;
    int which = WHICH_KERNEL;
#if 0
    cout<<"jitimm ...";
    cout<<"jitimm(0) = "<<jitimm(0)<<endl;
    cout<<"jitimm(1) = "<<jitimm(1)<<endl;
    cout<<"jitimm(-1) = "<<jitimm(-1)<<endl;
    cout<<"popcount(0) = "<<popcount(0)<<endl;
    cout<<"popcount(1) = "<<popcount(1)<<endl;
    cout<<"popcount(-1) = "<<popcount(-1)<<endl;
    cout<<std::hex;
    for(uint64_t i=1,j=1; i<64; j+=j, ++i){
        cout<<" jitimm("<< j-1    <<") = "<<jitimm(j-1)<<endl;
        cout<<" jitimm("<< ~(j-1) <<") = "<<jitimm(~(j-1))<<endl;
    }
    cout<<std::dec;
#endif

    if(argc > 1){
        // actually only the last -[tlu] option is used
        for( ; a+1<argc && argv[a+1][0]=='-'; ++a){
            char *c = &argv[a+1][1];
            cout<<" arg : "<<c<<endl;
            for( ; *c != '\0'; ++c){
                if(*c=='h'){
                    cout<<" fuse2lin -[h|t|a|q|l|u|m]* [-oFILE] VLEN H W"<<endl;
                    cout<<"  -t    test correctness + VE intrinsics code (no unroll)"<<endl;
                    cout<<"  -a    use lower-vl alt strategy if can speed induction"<<endl;
                    cout<<"        (suggested for Aurora VLEN=256 runs)"<<endl;
                    cout<<"  -oSTR output code filename"<<endl;
                    cout<<"        (Ex. -ofile-vi.c for clang-vector-intrinsics code)"<<endl;
                    cout<<"  -q    quick-test correctness (simulate using C++ vectors)"<<endl;
                    cout<<"  -l    pseudo-asm for loops (+correctness)"<<endl;
                    cout<<"  -u    [WIP] pseudo-asm-code for unrolled loops (+correctness)"<<endl;
                    cout<<"  -m    try for extended-range (a/d) ~ a*M>>N forms"<<endl;
                    cout<<"  -kSTR kernel type [NONE,HASH,PRINT,CHECK,SQIJ]"<<endl;
                    cout<<"  -h    this help"<<endl;
                    cout<<"   VLEN = vector length"<<endl;
                    cout<<"   I    = 1st loop a=0..i-1"<<endl;
                    cout<<"   J    = 2nd loop b=0..j-1"<<endl;
                    cout<<" double loop --> loop over vector registers a[VLEN], b[VLEN]"<<endl;
                    opt_h = 1;
                }else if(*c=='t'){ opt_t=2; opt_l=0; opt_u=0;
                }else if(*c=='a'){ opt_t=3; opt_l=0; opt_u=0;
                }else if(*c=='q'){ opt_t=1; opt_l=0; opt_u=0;
                }else if(*c=='l'){ opt_t=0; opt_l=1; opt_u=0;
                }else if(*c=='u'){ opt_t=0; opt_l=0; opt_u=1;
                }else if(*c=='m'){ opt_m=1;
                }else if(*c=='o'){
                    size_t len = strlen(++c);
                    ofname = new char[len+1];
                    strncpy(ofname,c,len+1);
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
    cout<<" args: a = "<<a<<" opt_t="<<opt_t<<" opt_u="<<opt_u<<" opt_l="<<opt_l<<" opt_m="<<opt_m<<endl;
    cout<<" ofname = "<<(ofname? ofname: "none")<<" which="<<kernel_name(which)<<endl;
    if(argc > a+1) vl = atof(argv[a+1]);
    if(argc > a+2) h  = atof(argv[a+2]);
    if(argc > a+3) w  = atof(argv[a+3]);
    cout<<"vlen="<<vl<<", h="<<h<<", w="<<w<<endl;

    if(opt_m){
        // removed test_mod_inverse<uint32_t>();
        //test_mod_inverse<uint64_t>();
        cout<<" (mod_inverse OK)";
        exit(0);
    }

    // INCORRECT verify1();
    //cout<<" verify1 OK"<<endl;

    if(opt_h == 0){
        if(opt_t==1) test_vloop2(vl,h,w);
        else if(opt_t==2||opt_t==3) test_vloop2_no_unrollX(vl,h,w,opt_t,which,ofname);
        if(opt_u) test_vloop2_unroll(vl,h,w);
        if(opt_l) test_vloop2_no_unroll(vl,h,w); // C++ code more like asm generic loop
    }
    delete[] ofname;
    cout<<"\nGoodbye"<<endl;
    return 0;
}
// vim: ts=4 sw=4 et cindent cino=^=l0,\:.5s,=-.5s,N-s,g.5s,b1 cinkeys=0{,0},0),\:,0#,!^F,o,O,e,0=break
