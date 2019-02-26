#include "vechash.hpp"
#include "../codegenasm.hpp"
#include <list>
#include <utility>
#include <string>
#include <sstream>

namespace scramble64 {
    uint64_t const r1 = 7664345821815920749ULL;
    uint64_t const r2 = 1181783497276652981ULL;
    uint64_t const r3 = 3202034522624059733ULL;
}

/** \b NOT optimized! */
std::string ve_load64_opt0(std::string s, uint64_t v){
    uint32_t const vlo = uint32_t(v);
    uint32_t const vhi = uint32_t(uint32_t(v>>32) + ((int32_t)v<0? 1: 0));
    std::ostringstream oss;
    bool const is31bit = ( (v&(uint64_t)0x000000007fffFFFFULL) == v );
    char const * comment=" sign-extended ";
    if( is31bit                                 // 31-bit v>=0 is OK for lea
            || ( (int)vlo<0 && (int)vhi==-1 ))  // if v<0 sign-extended int32_t, also happy
    {
        if( is31bit ) comment=" ";
        oss <<"\tlea    "<<s<<", "<<jithex(vlo)<<"\n";
    }else{
        oss <<"\tlea    "<<s<<", "<<jithex(vlo)<<"\n"
            <<"\tor     "<<s<<", 0, (32)0\n"
            <<"\tlea.sl "<<s<<", "<<jithex(vhi)<<"(,"<<s<<")";
        comment=" ve_load64_opt0 ";
    }
    oss<<" #"<<comment<<s<<" = "<<jithex(v);
    return oss.str();
}

#if 0
    void hash_combine( uint64_t const* v, int const vl ){
        assert( vl > 0 && vl <= mvl ); // vl==0 OK, but wasted work.
        using namespace scramble64;  // r1, r2, r3 scramblers
        //FOR(i,vl) vs[i] = i;
        FOR(i,vl) vz[i] = j + vs[i];
        FOR(i,vl) vx[i] = r2 * v[i];            // hash v[]
        FOR(i,vl) vz[i] = r1 * vz[i];           // hash vs[]=j..j+vl-1
        FOR(i,vl) vx[i] = vx[i] + vy[i];        // add hash_v[]
        FOR(i,vl) vx[i] = vx[i] + vz[i];        // add hash_vs[]
        // for add hash_*[], we xor-reduce needs to be done right now :(
        r = 0;
        FOR(i,vl) r ^= vx[i];
        hashVal ^= r;                           // hashVal ^= vx[0]^vx[1]^...^vx[vl-1]
        j += vl;
    }
#endif
void VecHash::kern_asm_begin( AsmFmtCols &a, char const* client_vs/*=nullptr*/,
        uint32_t const seed/*=0*/ ){
    typedef std::list<std::pair<std::string,std::string>> AsmScope;
    char const* vs_register = (client_vs? client_vs: "%v40");
    AsmScope const block = {//{"hashval","%s40"},
        {"vh_j","%s40"}
        ,{"vh_r1","%s41"},{"vh_r2","%s42"}
        ,{"vh_vs",vs_register}
    };
    a.scope(block,"VecHash::kern_asm registers");
    // const regs
    a.ins(ve_load64_opt0("vh_r1", scramble64::r1));
    a.ins(ve_load64_opt0("vh_r2", scramble64::r2));
    a.ins("vh_vseq     vh_vs");       // const, vs={0,1,2,...mvl}
    // state (R/W regs)
    a.ins(ve_load64_opt0("vh_j", (uint64_t)seed<<32));
    a.ins("vbrd vh_x,0");
    a.com("VecHash : init done");
}
void VecHash::kern_asm( AsmFmtCols &a,
        std::string va, std::string vl, std::string hash ){
    a.lcom("VecHash : kernel begins",
            "  in: "+va+", "+vl,
            "  inout: "+hash+" (scalar reg)",
            "  state: vh_j",
            "  const: vh_r1, vh_r2, vh_vs",
            "  scratch: vh_r, vh_vx, vh_vy, vh_vz"
          );
    typedef std::list<std::pair<std::string,std::string>> AsmScope;
    // TODO: resolve any conflicts of scope regs with ANY 
    AsmScope const block = {{"vh_r","%s63"}
        ,{"vh_vx","%v63"},{"vh_vy","%v62"}//,{"vh_vz","%v63"}
    };
    a.scope(block,"vechash2::kern_asm");
    a.ins("vaddu.l  vh_vy, vh_j, vh_vs");
    a.ins("vmulu.l  vh_vy, vh_r1, vh_vz",           "scramble64::r1 * (j+vs[])");
    a.ins("vmulu.l  vh_vx, vh_r2, /**/"+va,         "scramble64::r2 * "+va+"[]");
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
void VecHash2::kern_asm_begin( AsmFmtCols &a, char const* client_vs/*=nullptr*/, uint32_t const seed/*=0*/ ){
    typedef std::list<std::pair<std::string,std::string>> AsmScope;
    char const* vs_register = (client_vs? client_vs: "%v40");
    AsmScope const block = {//{"hashval","%s40"},
        {"vh2_j","%s40"}
        ,{"vh2_r1","%s41"},{"vh2_r2","%s42"},{"vh2_r3","%s43"}
        ,{"vh2_vs",vs_register}
    };
    a.scope(block,"vechash2::kern_asm state registers");
    a.ins(ve_load64_opt0("vh2_r1", scramble64::r1));
    //a.ins(ve_load64_opt0("hashval", 0)); // oh no. we hash_combine into a client register
    a.ins(ve_load64_opt0("vh2_r2", scramble64::r2));
    a.ins(ve_load64_opt0("vh2_j", (uint64_t)seed<<32));
    a.ins(ve_load64_opt0("vh2_r3", scramble64::r3));
    a.ins("vh2_vseq     vh2_vs");       // const, vs={0,1,2,...mvl}
    a.com("vechash2 : init done");
}
/// input strings are registers
void VecHash2::kern_asm( AsmFmtCols &a,
        std::string va, std::string vb, std::string vl, std::string hash ){
    a.lcom("vechash2 : kernel begins",
            "  in: "+va+", "+vb+", "+vl,
            "  inout: "+hash+" (scalar reg)",
            "  state: vh2_j",
            "  const: vh2_r1, vh2_r2, vh2_r3, vh2_vs",
            "  scratch: vh2_r, vh2_vx, vh2_vy, vh2_vz"
          );
    typedef std::list<std::pair<std::string,std::string>> AsmScope;
    // TODO: resolve any conflicts of scope regs with ANY 
    AsmScope const block = {{"vh2_r","%s63"}
        ,{"vh2_vx","%v61"},{"vh2_vy","%v62"},{"vh2_vz","%v63"}
    };
    a.scope(block,"vechash2::kern_asm");
    a.ins("vmulu.l  vh2_vx, vh2_r2, /**/"+va,       "scramble64::r2 * "+va+"[]");
    a.ins("vaddu.l  vh2_vy, vh2_j, vh2_vs");
    a.ins("vmulu.l  vh2_vz, vh2_r3, /**/"+vb,       "scramble64::r3 * "+vb+"[]");
    a.ins("vmulu.l  vh2_vy, vh2_r1, vh2_vy",        "scramble64::r1 * (j+vs[])");
    a.ins("vaddu.l  vh2_vx, vh2_vx, vh2_vz");
    a.ins("vaddu.l  vh2_vx, vh2_vx, vh2_vy",        "vx ~ sum of scrambles");
    a.ins("vrxor    vh2_vy, vh2_vx");
    a.ins("lvs      vh2_r, vh2_vx(0)",              "r = xor-reduction(vx)");
    a.ins("addu.l   vh2_j, vl, vh2_j",              "state j += vl");
    a.com("output: modified hash value "+hash);
    a.ins("xor      /**/"+hash+", "+hash+", vh2_r", hash+" ^= r");
    a.pop_scope();
    a.lcom("vechash2 : kernel done");
}
void VecHash2::kern_asm_end( AsmFmtCols &a ){
    a.pop_scope();
}
// vim: ts=4 sw=4 et cindent cino=^=l0,\:.5s,=-.5s,N-s,g.5s,b1 cinkeys=0{,0},0),\:,0#,!^F,o,O,e,0=break
