
#include "ve_divmod.hpp"
#include "cblock.hpp"
#include "stringutil.hpp"

namespace cprog {

using namespace std;

int mk_FASTDIV(Cblock& cb, uint32_t const jj, uint32_t const vIn_hi/*=0*/,
        int const v/*=0,verbose*/){
    bool const verify=true; // false after code burn-in
    bool const macro_constants = false;
    bool const define_FASTDIV_SHR = false;  // #define?
    bool const const_fastdiv_SHR = false;   // uint64_t const?  [else inline the shift constant]
    int ret=0;
    ostringstream oss;
    // go up to some well-defined scope position and use a named-block to
    // record that this macro exists (try not to duplicate the definition)
    // where Cblock::define will place definitions (may need tweaking?)
    auto& scope=(cb.getName()=="body"? cb: cb["..*/body/.."]);
    string tag = OSSFMT("fastdiv_"<<jj); // "we were here before" tag
    if(v>1) cout<<"mk_FASTDIV_"<<jj<<" range "<<vIn_hi<<" to scope "<<scope.fullpath()
        //<<" <"<<scope.str()<<">"
        <<endl;;
    if(scope.find(tag)){
        if(v>1) cout<<"FASTDIV_"<<jj<<" macro already there"<<endl;
    }else{
        scope[tag].setType("TAG");
        if(v>0) cout<<"FASTDIV_"<<jj<<" new macro, input range "<<vIn_hi<<endl;
        struct fastdiv jj_fastdiv;
        uint32_t fastdiv_ops = 0U;
        {
            fastdiv_make( &jj_fastdiv, (uint32_t)jj );
            if(v>2)cout<<" mul,add,shr="<<(void*)(intptr_t)jj_fastdiv.mul
                <<","<<jj_fastdiv.add<<","<<jj_fastdiv.shift;
            if(jj_fastdiv.mul != 1) ++fastdiv_ops;
            if(jj_fastdiv.add != 0) ++fastdiv_ops;
            if(1) /*shift*/ ++fastdiv_ops;
            if(v>0) cout<<" struct fastdiv (mul,add,shr) in "<<fastdiv_ops<<" ops"<<endl;
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
            mac = OSSFMT("_ve_vmulul_vsv("<<mul<<",V)");

            string shr;
            if(define_FASTDIV_SHR){
                shr = OSSFMT("FASTDIV_"<<jj<<"_SHR");
                scope.define(shr,jitdec(FASTDIV_C));
            }else if(const_fastdiv_SHR){
                shr = OSSFMT("fastdiv_"<<jj<<"_SHR");
                cb>>OSSFMT("uint64_t const "<<mul<<" = "<<FASTDIV_C<<";");
            }else{
                shr = jitdec(FASTDIV_C);
            }
            //mac = OSSFMT("_ve_vsrl_vvs("<<mac<<","<<shr<<")/*OK over [0,"<<vIn_hi<<")*/");
            mac = OSSFMT("_ve_vsrl_vvs("<<mac<<","<<shr<<")/*OK over [0,2^"<<FASTDIV_C/2<<")*/");

            fastdiv_macro = mac;
            ret = 2;
            if(v>0) cout<<"mk_FASTDIV "<<ret<<" ops, macro="<<fastdiv_macro<<endl;
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
                if(define_FASTDIV_SHR){
                    shr = OSSFMT("FASTDIV_"<<jj<<"_SHR");
                    scope.define(shr,jitdec(jj_fastdiv.shift));
                }else if(const_fastdiv_SHR){
                    shr = OSSFMT("fastdiv_"<<jj<<"_SHR");
                    cb>>OSSFMT("uint64_t const "<<shr<<" = "<<jj_fastdiv.shift<<";");
                }else{
                    shr = jitdec(jj_fastdiv.shift);
                }
                mac=OSSFMT("_ve_vsrl_vvs("<<mac<<","<<shr<<")");
            }
            fastdiv_macro = mac;
            ret = fastdiv_ops;
            if(v>0) cout<<"mk_FASTDIV "<<ret<<" ops, macro="<<fastdiv_macro<<endl;
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

int mk_DIVMOD(Cblock& cb, uint32_t const jj, uint32_t const vIn_hi/*=0*/,
        int const v/*=0,verbose*/){
    ostringstream oss;
    // go up to some well-defined scope position and use a named-block to
    // record that this macro exists (try not to duplicate the definition)
    auto& scope=(cb.getName()=="body"? cb: cb["..*/body/.."]); // where Cblock::define will place definitions
    if(v>1) cout<<"mk_DIVMOD_"<<jj<<" range "<<vIn_hi<<" to scope "<<scope.fullpath()
        //<<" <"<<scope.str()<<">"
        <<endl;
    int nops=0;
    string tag = OSSFMT("divmod_"<<jj);
    if(scope.find(tag)){
        if(v>1) cout<<"DIVMOD_"<<jj<<" macro already there"<<endl;
    }else{
        scope[tag].setType("TAG"); // create the tag block "we were here before"
        if(v>0) cout<<"DIVMOD_"<<jj<<" new macro"<<endl;
        int nops = mk_FASTDIV(cb,jj,vIn_hi,v);
        string mac = OSSFMT(" \\\n          VDIV = FASTDIV_"<<jj<<"(V); \\\n");
        if(nops==1){
            assert(positivePow2(jj));
            if(v>1) cout<<("MASK WITH jj-1 for modulus");
            mac = OSSFMT(mac<<"          VMOD = _ve_vand_vsv("<<jithex(jj-1)<<",V)");
            ++nops;
        }else{
            // VE does not have FMA ops for any integer type.
            //     so for 12/24-bit floats could consdier exact-floating calcs,
            //     but conversion ops probably kill this idea (not tried).
            if(v>1) cout<<("MUL-SUB modulus");
            mac = OSSFMT(mac<<"          VMOD = _ve_vsubul_vvv(V,_ve_vmulul_vsv("<<jj<<",VDIV))");
            if(!isIval(jj)) mac.append(" /*is non-Ival in register?*/");
            nops+=2;
        }
        scope.define(OSSFMT("DIVMOD_"<<jj<<"(V,VDIV,VMOD)"),mac);
    }
    return nops;
}

}//cprog::
// vim: ts=4 sw=4 et cindent cino=^=l0,\:.5s,=-.5s,N-s,g.5s,b1 cinkeys=0{,0},0),\:,0#,!^F,o,O,e,0=break
