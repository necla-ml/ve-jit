#include "vechash.hpp"
#include "../codegenasm.hpp"
#include "../stringutil.hpp"
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
    ve_propose_reg(variable,block,a,"%v",vechash_vector_order);
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
    if(client_vs) block.push_back({"vh_vseq",client_vs});
    else          vechash_vector ("vh_vseq",block,a);
    a.scope(block,"VecHash::kern_asm registers");
    // const regs
    if(!client_vs){
        // vh_vseq really should be 0..MVL-1, for most safety.
        a.ins("svl rnd64a; lea rnd64b,256; lvl rnd64b; vseq vh_vseq; lvl rnd64a"
                ,"vh_vseq=0,1,..,MVL-1");
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
        if(client_vs) block.push_back({"vh_vseq",client_vs}); // ideally 0,1,...,MVL-1
        else          vechash_vector ("vh_vseq",block,state);
        ro_regs.scope(block,"const: vechash2");
        if(!client_vs)
            ro_regs.ins("svl rnd64a; lea rnd64b,256; lvl rnd64b; vseq vh_vseq; lvl rnd64a"
                    ,"0,1,..,MVL-1");
        ro_regs.ins(ve_load64("rnd64a", scramble64::r1),"scramble64::r1");
        ro_regs.ins(ve_load64("rnd64b", scramble64::r2),"scramble64::r2");
        ro_regs.ins(ve_load64("rnd64c", scramble64::r3),"scramble64::r3");
    }
    block.clear();
    // state register init
    {
        vechash_scalar("vh2_j",  block,state);  // state (cannot migrate up)
        vechash_vector("vh_x",  block,state);   // state (cannot migrate up)
        state.scope(block,"state: vechash2");
        // const
        // Note: our persistent context registers are now published
        //       within the client's scope stack ...
        //       we pop_scope them during kern_asm_end
        // state
        state.ins(ve_load64("vh2_j", (uint64_t)seed<<32));
        state.ins("vbrdl vh_x,0","vechash2 state init DONE");
    }
}
/** input strings are registers.
 * \c va, \c vb (vectors) of length \c vl (scalar) are to be mixed in
 * with our current hash context. The resulting hash value is placed into
 * \c hash (scalar). */
void VecHash2::kern_asm( AsmFmtCols &a,
        std::string va, std::string vb, std::string vl, std::string hash ){
    a.lcom("vechash2 : kernel begins",
            "  in: "+va+", "+vb+", "+vl,
            "  inout: "+hash+" (scalar reg)",
            "  state: vh2_j",
            "  const: rnd64a, rnd64b, rnd64c, vh2_vs",
            "  scratch: vh2_r, vh2_vx, vh2_vy, vh2_vz"
          );
    AsmScope block; // empty
    ve_propose_reg("vh2_r" ,block,a,SCALAR_TMP);
    ve_propose_reg("vh2_vx",block,a,VECTOR_TMP);
    ve_propose_reg("vh2_vy",block,a,VECTOR_TMP);
    ve_propose_reg("vh2_vz",block,a,VECTOR_TMP);
    a.scope(block,"vechash2::kern_asm");
    a.ins("vmulu.l  vh2_vx, rnd64b, /**/"+va,       "scramble64::r2 * "+va+"[]");
    a.ins("vaddu.l  vh2_vy, vh2_j, vh2_vs",         "init for state j");
    a.ins("vmulu.l  vh2_vz, rnd64c, /**/"+vb,       "scramble64::r3 * "+vb+"[]");
    a.ins("vmulu.l  vh2_vy, rnd64a, vh2_vy",        "scramble64::r1 * (j+vs[])");
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
