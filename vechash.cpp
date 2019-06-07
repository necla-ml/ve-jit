/* Copyright (c) 2019 by NEC Corporation
 * This file is part of ve-jit */
#include "vechash.hpp"
#include "codegenasm.hpp" // really?
#include "stringutil.hpp"
#include "asmfmt.hpp"
#include "cblock.hpp"
#include <list>
#include <utility>
#include <string>
#include <sstream>

#include <iostream>
using namespace std;

namespace scramble64 {
    uint64_t const r1 = 7664345821815920749ULL;
    uint64_t const r2 = 1181783497276652981ULL;
    uint64_t const r3 = 3202034522624059733ULL;
}

static std::vector<std::pair<int,int>> const vechash_scalar_order{{40,63},{39,34},{7,0},{33,18},{12,13}};
static std::vector<std::pair<int,int>> const vechash_vector_order{{40,63},{39,0}};

/** custom register assignment order, hopefully distinctive from ve_propose_reg defaults */
static void vechash_scalar(std::string variable, AsmScope& block, AsmFmtCols const& a){
    ve_propose_reg(variable,block,a,"%s",vechash_vector_order);
}
static void vechash_vector(std::string variable, AsmScope& block, AsmFmtCols const& a){
    ve_propose_reg(variable,block,a,"%v",vechash_vector_order);
}

/** Note: assume that current vl is max we'll every see, for purposes
 * of initializes vs={0,1,2,...vl} */
void VecHash::kern_asm_begin( AsmFmtCols &a, char const* client_vs/*=nullptr*/,
        uint32_t const seed/*=0*/ ){
    if(0){
        cout<<" VecHash::kern_asm_begin"<<endl;
        auto vs = a.def_words_starting("%s");
        auto vv = a.def_words_starting("%v");
        cout<<" parent scalars = {";for(auto s:vs) cout<<" "<<s; cout<<" }"<<endl;
        std::string free_scalar_register = free_pfx( vs, "%s", {{0,7},{63,34}} );
        cout<<" I suggest free_scalar_register = "<<free_scalar_register<<endl;
        cout<<" parent vectors = {";for(auto s:vv) cout<<" "<<s; cout<<" }"<<endl;
    }
    AsmScope block = {}; //{"hashval","%s40"},
    vechash_scalar("rnd64a",block,a);   // const (may move up)
    vechash_scalar("rnd64b",block,a);   // const (can also be colocated)
    vechash_scalar("vh_j",  block,a);   // state (cannot migrate up)
    vechash_vector("vh_x",  block,a);   // state (cannot migrate up)
    if(client_vs) block.push_back({"vh_vs",client_vs});
    else          vechash_vector ("vh_vs",block,a);
    a.scope(block,"VecHash::kern_asm registers");
    // const regs
    if(!client_vs){
        // vh_vs really should be 0..MVL-1, for most safety.
        a.ins("lvl rnd64a; lea rnd64b,256; svl rnd64b; vseq vh_vs; lvl rnd64a"
                ,"vh_vs=0,1,..,MVL-1");
    }
    a.ins(ve_load64("rnd64a", scramble64::r1));
    a.ins(ve_load64("rnd64b", scramble64::r2));
    // state (R/W regs)
    a.ins(ve_load64("vh_j", (uint64_t)seed<<32));
    a.ins("vbrd vh_x,0");
    a.com("VecHash : init done");
}
void VecHash::kern_asm( AsmFmtCols &a,
        std::string va, std::string vl, std::string hash ){
    a.lcom("VecHash : kernel begins",
            "  in: "+va+", "+vl,
            "  inout: "+hash+" (scalar reg)",
            "  state: vh_j",
            "  const: rnd64a, rnd64b, vh_vs",
            "  scratch: vh_r, vh_vx, vh_vy, vh_vz"
          );
    // TODO: resolve any conflicts of scope regs with ANY 
    AsmScope const block = {{"vh_r","%s63"}
        ,{"vh_vx","%v63"},{"vh_vy","%v62"}//,{"vh_vz","%v63"}
    };
    a.scope(block,"vechash2::kern_asm");
    a.ins("vaddu.l  vh_vy, vh_j, vh_vs");
    a.ins("vmulu.l  vh_vy, rnd64a, vh_vz",           "scramble64::r1 * (j+vs[])");
    a.ins("vmulu.l  vh_vx, rnd64b, /**/"+va,         "scramble64::r2 * "+va+"[]");
    a.ins("vaddu.l  vh_vx, vh_vx, vh_vz",           "sum");
    a.ins("vrxor    vh_vy, vh_vx");
    a.ins("lvs      vh_r, vh_vy(0)",                "r = xor-reduction(vx)");
    a.ins("addu.l   vh_j, vh_j, vl",                "state j += vl");
    a.com("output: modified hash value "+hash);
    a.ins("xor      /**/"+hash+", "+hash+", vh_r",  hash+" ^= r");
    a.pop_scope();
    a.lcom("VecHash : kernel done");
}
void VecHash::kern_asm_end( AsmFmtCols &a ){
    a.pop_scope();
}

/** init registers for VecHash2 assembler kernel.
 * Current vector length \b MUST be the maximum vector length of this hash_combiner.
 * \p seed an optional seed value.
 * \note if client has {0,...,vl} available this could be passed (saving register vs[])
 *
 * TODO: look at all scopes of parent 'a' to allocate non-conflicting state regs
 */
void VecHash2::kern_asm_begin( AsmFmtCols &ro_regs, AsmFmtCols &state,
        char const* client_vs/*=nullptr*/, uint32_t const seed/*=0*/ )
{
    // read-only register constants (move up as far as possible)
    AsmScope block; // empty
    {
        // vechash has no subroutines, so try for a distinct group of regs
        // const
        vechash_scalar("rnd64a",block,state);   // const (may move up)
        vechash_scalar("rnd64b",block,state);   // const (can also be colocated)
        vechash_scalar("rnd64c",block,state);   // const (can also be colocated)
        if(client_vs) block.push_back({"vh_vs",client_vs}); // ideally 0,1,...,MVL-1
        else          vechash_vector ("vh_vs",block,state);
        ro_regs.scope(block,"const: vechash2");
        if(!client_vs){
            ro_regs.ins("svl rnd64a#rnd* as tmp regs..; lea rnd64b,256; lvl rnd64b; vseq vh_vs; svl rnd64a"
                    ,"..vh_vs=0,1,..,MVL-1");
        }
        ro_regs.ins(ve_load64("rnd64a", scramble64::r1),"scramble64::r1");
        ro_regs.ins(ve_load64("rnd64b", scramble64::r2),"scramble64::r2");
        ro_regs.ins(ve_load64("rnd64c", scramble64::r3),"scramble64::r3");
    }
    block.clear();
    // state register init
    {
        vechash_scalar("vh2_j",  block,state);  // state (cannot migrate up)
        // TBD: vechash_vector("vh_x",  block,state);   // state (cannot migrate up)
        state.scope(block,"state: vechash2");
        // const
        // Note: our persistent context registers are now published
        //       within the client's scope stack ...
        //       we pop_scope them during kern_asm_end
        // state
        state.ins(ve_load64("vh2_j", (uint64_t)seed<<32),"j=(u64)seed<<32");
        // TBD: vector state
        //state.ins("vbrdl vh_x,0","vechash2 state init DONE");
    }
}
void VecHash2::kern_C_begin( cprog::Cblock &defines,
        char const* client_vs/*=nullptr*/, uint32_t const seed/*=0*/ )
{
    ostringstream oss;
    defines
        >>OSSFMT("uint64_t const rnd64a = "<<scramble64::r1<<"ULL;")
        >>OSSFMT("uint64_t const rnd64b = "<<scramble64::r2<<"ULL;")
        >>OSSFMT("uint64_t const rnd64c = "<<scramble64::r3<<"ULL;")
        ;
    auto& state = defines["last"]["vechash"];
    cout<<"kern_C_begin("<<defines.fullpath()<<",...)"<<endl;
    state>>OSSFMT(left<<setw(40)<<OSSFMT("uint64_t vh2_j = "<<jithex((uint64_t)seed<<32)<<"ULL;")<<" // vh2 state");
    if(client_vs){ // client has a vseq=0..MVL ?
        defines.define("vh_vs",client_vs);
    }else{
        state<<OSSFMT("_ve_lvl(256);\n"
                "__vr const vh_vs = _ve_vseq_v()"<<client_vs<<"; // vh2 vseq");
    }
}
void VecHash2::kern_C( cprog::Cblock &parent,
        std::string va, std::string vb, std::string vl, std::string hash)
{
    ostringstream oss;
    CBLOCK_SCOPE(vh,"",parent.getRoot(),parent);
    vh  >>"// vechash2 : kernel begins"
        >>OSSFMT("//  in: 2 u64 vectors "<<va<<", "<<vb<<", VL="<<vl)
        >>OSSFMT("//  inout: "<<hash<<" (scalar reg)")
        >>OSSFMT("//  state: vh2_j")
        >>OSSFMT("//  const: rnd64a, rnd64b, rnd64c, vh_vs")
        >>OSSFMT("//  scratch: vh2_r, vh2_vx, vh2_vy, vh2_vz")
        ;
    vh  >>OSSFMT("__vr vh2_vx = _ve_vmulul_vsv(rnd64b,"<<va<<");")
        >>"__vr vh2_vy = _ve_vaddul_vsv(vh2_j,vh_vs); // init state j"
        >>OSSFMT("__vr vh2_vz = _ve_vmulul_vsv(rnd64b,"<<vb<<");")
        >>"vh2_vy = _ve_vmulul_vsv(rnd64a,vh2_vy);"
        >>"vh2_vx = _ve_vaddul_vvv(vh2_vx,vh2_vz);"
        >>"vh2_vz = _ve_vaddul_vvv(vh2_vx,vh2_vy);    // vz ~ sum of xyz scrambles"
        >>"vh2_vx = _ve_vsuml_vv(vh2_vz);             // vrxor missing"
        >>"uint64_t vh2_r = _ve_lvs_svs_u64(vh2_vx,0);"
        >>OSSFMT("vh2_j += "<<vl)
        >>OSSFMT(hash<<" = "<<hash<<" ^ vh2_r")
        ;
}
std::pair<std::string,std::string> VecHash2::kern_C_macro(std::string macname)
{
    std::string mac,def;
    ostringstream oss;
    mac=OSSFMT(macname<<"(VA,VB,VL,HASH)");
    oss <<"do{ \\\n"
        //"    asm(\"# vechash2 BEGIN\"); \\\n"
        "    /* vechash2 : kernel begins */ \\\n"
        "    /*  in: 2 u64 vectors VA, VB, common VL */ \\\n"
        "    /*  inout: HASH (scalar reg) */ \\\n"
        "    /*  state: vh2_j (scalar) */ \\\n"
        "    /*  const: rnd64a, rnd64b, rnd64c, vh_vs */ \\\n"
        "    /*  scratch vectors: vh2_vx, vh2_vy, vh2_vz */ \\\n"
        "    /*  scratch scalars: vh2_r */ \\\n"
        "    __vr vh2_vx = _ve_vmulul_vsv(rnd64b,VA); \\\n"
        "    __vr vh2_vy = _ve_vaddul_vsv(vh2_j,vh_vs); /* init state j */ \\\n"
        "    __vr vh2_vz = _ve_vmulul_vsv(rnd64b,VB); \\\n"
        "    vh2_vy = _ve_vmulul_vsv(rnd64a,vh2_vy); \\\n"
        "    vh2_vx = _ve_vaddul_vvv(vh2_vx,vh2_vz); \\\n"
        "    vh2_vz = _ve_vaddul_vvv(vh2_vx,vh2_vy);    /* vz ~ sum of xyz scrambles */ \\\n"
        "    /*vh2_vx = _ve_vrxor_vv(vh2_vz);*/         /* missing instruction ? */ \\\n"
        "    vh2_vx = _ve_vsuml_vv(vh2_vz);             /* wanted vrxor */ \\\n"
        "    uint64_t vh2_r = _ve_lvs_svs_u64(vh2_vx,0); \\\n"
        "    vh2_j += VL; \\\n"
        "    HASH = HASH ^ vh2_r; \\\n"
        //"    asm(\"# vechash2 END\"); \\\n"
        "}while(0)"
        ;
    def = oss.str();
    //return make_pair(mac,def);
    return {mac,def};
}
void VecHash2::kern_C_end( cprog::Cblock &cb ){
}
/** input strings are registers.
 * \c va, \c vb (vectors) of length \c vl (scalar) are to be mixed in
 * with our current hash context. The resulting hash value is placed into
 * \c hash (scalar). Client gives us one \c tmp reg (which will hold the
 * xor-reduction value upon exit). */
void VecHash2::kern_asm( AsmFmtCols &a,
        std::string va, std::string vb, std::string vl, std::string hash,
        std::string tmp ){
    a.lcom("vechash2 : kernel begins",
            "  in: "+va+", "+vb+", "+vl,
            "  inout: "+hash+" (scalar reg)",
            "  state: vh2_j",
            "  const: rnd64a, rnd64b, rnd64c, vh_vs",
            "  scratch: vh2_r, vh2_vx, vh2_vy, vh2_vz"
          );
    AsmScope block; // empty
    //ve_propose_reg("vh2_r" ,block,a,SCALAR_TMP);
    ve_propose_reg("vh2_r" ,block,a,SCALAR_TMP);
    ve_propose_reg("vh2_vx",block,a,VECTOR_TMP);
    ve_propose_reg("vh2_vy",block,a,VECTOR_TMP);
    ve_propose_reg("vh2_vz",block,a,VECTOR_TMP);
    a.scope(block,string{"vechash2::kern_asm"});
    // a[], b[] --> vector hash for a,b at sequence position j
    a.ins("vmulu.l  vh2_vx, rnd64b, /**/"+va,       "scramble64::r2 * "+va+"[]");
    a.ins("vaddu.l  vh2_vy, vh2_j, vh_vs",          "init for state j");
    a.ins("vmulu.l  vh2_vz, rnd64c, /**/"+vb,       "scramble64::r3 * "+vb+"[]");
    a.ins("vmulu.l  vh2_vy, rnd64a, vh2_vy",        "scramble64::r1 * (j+vs[])");
    a.ins("vaddu.l  vh2_vx, vh2_vx, vh2_vz");
    a.ins("vaddu.l  vh2_vz, vh2_vx, vh2_vy",        "vz ~ sum xyz scrambles");
    //
    // ---------------- BEWARE -------------------
    //
    // I had a tough bug to crack once, here.
    // LVL = "Load Vector Length"  <-- use this to set the vector length
    // SVL = "Save Vector Length"  ( **not** "Set" )
    //
    // In gdb, you can double-check vl with "info reg vl"
    //
    // vector hash --> scalar hash (xor-reduce)
    a.ins("vrxor    vh2_vx, vh2_vz",                "vy[0] = vz[0]^vz[1]^...");
    a.ins("lvs      vh2_r, vh2_vx(0)",              "r = xor-reduction(vx)");
    a.ins("addu.l   vh2_j, "+vl+", vh2_j",          "state j += vl (seq pos)");
    a.com("output: modified hash value "+hash);
    a.ins("xor      /**/"+hash+", "+hash+", vh2_r", hash+" ^= r");
    a.pop_scope();
    a.lcom("vechash2 : kernel done");
}
void VecHash2::kern_asm_end( AsmFmtCols &a ){
    a.pop_scope();
}
// vim: ts=4 sw=4 et cindent cino=^=l0,\:.5s,=-.5s,N-s,g.5s,b1 cinkeys=0{,0},0),\:,0#,!^F,o,O,e,0=break
