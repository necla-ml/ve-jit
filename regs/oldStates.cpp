#include "old/regStates.hpp"
#include <iostream>
#include <cassert>

namespace ve{

// ====================================================== RegDefs.hpp impls ...
// ====================================================== os << Reg_t
std::ostream& operator<<(std::ostream& os, Reg_t const f){
    static char const * f1names[] = {"FREE","RESERVED","USED","Huh"};
    static char const * f2names[] = {"SCALAR","VECTOR","VMASK","VMASK2"};
    int f1 = f&MSK_FREE;
    int f2 = (f&MSK_TYPE) >> 2;
    int f3 = (f&MSK_ATTR) >> 4;
    char const* f3attr = (f3==0? ""
            : f3==PRESERVE? "|PRESERVE"
            : "|Huh?");
    return os<<' '<<f1names[f1]<<'|'<<f2names[f2]<<f3attr;
}

// ====================================================== asmname(RegId)
static char const* asm_register_names[IDlast] = {
    "%s0",  "%s1", "%s2", "%s3", "%s4", "%s5", "%s6", "%s7", "%s8", "%s9",
    "%s10",  "%s11", "%s12", "%s13", "%s14", "%s15", "%s16", "%s17", "%s18", "%s19",
    "%s20",  "%s21", "%s22", "%s23", "%s24", "%s25", "%s26", "%s27", "%s28", "%s29",
    "%s30",  "%s31", "%s32", "%s33", "%s34", "%s35", "%s36", "%s37", "%s38", "%s39",
    "%s40",  "%s41", "%s42", "%s43", "%s44", "%s45", "%s46", "%s47", "%s48", "%s49",
    "%s50",  "%s51", "%s52", "%s53", "%s54", "%s55", "%s56", "%s57", "%s58", "%s59",
    "%s60",  "%s61", "%s62", "%s63",
    "%v0",  "%v1", "%v2", "%v3", "%v4", "%v5", "%v6", "%v7", "%v8", "%v9",
    "%v10",  "%v11", "%v12", "%v13", "%v14", "%v15", "%v16", "%v17", "%v18", "%v19",
    "%v20",  "%v21", "%v22", "%v23", "%v24", "%v25", "%v26", "%v27", "%v28", "%v29",
    "%v30",  "%v31", "%v32", "%v33", "%v34", "%v35", "%v36", "%v37", "%v38", "%v39",
    "%v40",  "%v41", "%v42", "%v43", "%v44", "%v45", "%v46", "%v47", "%v48", "%v49",
    "%v50",  "%v51", "%v52", "%v53", "%v54", "%v55", "%v56", "%v57", "%v58", "%v59",
    "%v60",  "%v61", "%v62", "%v63",
    "%vm0",  "%vm1", "%vm2", "%vm3", "%vm4", "%vm5", "%vm6", "%vm7", "%vm8", "%vm9",
    "%vm10",  "%vm11", "%vm12", "%vm13", "%vm14", "%vm15"
};
static const int n_asm_register_names = IDlast;

char const* asmname(RegId const r){
    char const* ret = "%XX";
    if( isReg(r) )
        assert( r < n_asm_register_names);
        ret = asm_register_names[r];
    return ret;
}
// ====================================================== asmname(RegId)
}//ve::
// vim: ts=4 sw=4 et cindent cino=^=l0,\:.5s,=-.5s,N-s,g.5s,h.5s,b1 cinkeys=0{,0},0),\:,0#,!^F,o,O,e,0=break
