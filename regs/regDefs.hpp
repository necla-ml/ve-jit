#ifndef REGDEFS_HPP
#define REGDEFS_HPP
/** \file
 * \todo split regDefs.hpp into generic and processor-specific bits.  This has
 *       begun in reg-base.hpp and reg-aurora.hpp (testRegBase.cpp)
 */
#include <iosfwd>
#include <cstdint>

namespace ve {

/** RegId == IDlast is "unassigned" */
typedef int_least16_t RegId;

/** Generic register attributes.
 * - Usage:
 *   - RESERVED, USED, MSK_FREE
 * - Function-level requirements:
 *   - PRESERVE (callee must save)
 * - register class:
 *   - SCALAR (64-bit), VECTOR(256*8-byte), VMASK(256-bit)
 *   - VMASK2 (pair of VM registers, for packed-vector ops)
 *
 * All machines **must** provide
 * <em>FREE, RESERVED, USED, MSK_FREE, SHIFT_TYPE, SCALAR, MSK_TYPE</em>
 */
enum Reg_t : int {
    // generic flags:
    FREE=0, RESERVED=1, USED=2, MSK_FREE=3,
    SHIFT_TYPE=2, SCALAR=0<<2, MSK_TYPE=7<<2,
    // VE-specific flags:
    SCALAR2=1<<2, VECTOR=2<<2, VMASK=3<<2, VMASK2=4<<2,
    PRESERVE=1<<4, MSK_ATTR=1<<4,
    SHIFT_VLEN=5, MSK_VLEN=255<<SHIFT_VLEN // if unknown, use zero, interpreted as "max length vector/mask"
};
#define REG_T(I) static_cast<ve::Reg_t>(I)

std::ostream& operator<<(std::ostream& os, Reg_t const f);

int const MVL = 256;    // maximum vector length (in scalar-reg-size=byte8 units)
int const SCALAR_BYTES = 8;

// TODO replace with RegId typed enum, names IDfoo --> Rid::foo, RidLast::foo, RidN::foo, ...
// OR Regid const IDfoo ?
enum NREG : RegId { s=64, v=64, vm=16 };
enum StartId_e {
    IDscalar= 0,               IDscalar_last=IDscalar+NREG::s-1, // 0..63
    IDvector= IDscalar_last+1, IDvector_last=IDvector+NREG::v-1, // 64-127
    IDvmask = IDvector_last+1, IDvmask_last =IDvmask +NREG::vm-1,// 128-143
    IDlast  = IDvmask_last + 1, IDnone=IDlast };                 // 144
#if 0 // TODO
enum RID : RegId {
    s=0,        sEnd=s+NREG:s,      // [0..64)
    v=sEnd,     vEnd=v+NREG:v,      // [64..128)
    vm=vEnd,    vmEnd=vm+NREG:vm,   // [128..144)
    X=vmEnd };                      // 144 X ~ default out-of-range value
#endif

/** handy testers, but perhaps not optimal */
inline bool constexpr isScalar(RegId const r){ return r<NREG::s; }
inline bool constexpr isVector(RegId const r){ return r>=IDvector && r < IDvector_last+1; }
inline bool constexpr isMask(RegId const r){ return r>=IDvmask && r < IDvmask_last+1; }
inline bool constexpr isReg(RegId const r){ return r<IDlast; }
inline constexpr bool valid(RegId const r){ return r<IDlast; }
inline constexpr RegId invalidReg() { return IDlast; }

constexpr int bytes(Reg_t const r);
constexpr int align(Reg_t const r);
constexpr Reg_t regType(Reg_t const r){ return static_cast<Reg_t>(r&MSK_TYPE); }

constexpr int vlen(Reg_t const r){return (r&MSK_VLEN) >> SHIFT_VLEN;}
constexpr Reg_t regVlen(int vl){ return static_cast<Reg_t>((vl<<5)&MSK_VLEN); }

/** Id--> assembler register name, asmname(0)-->"%s0" */
/** scalar, vector, vmask regs each have ids in range [0,64+64+16=143) */
char const* asmname(RegId const r);
constexpr Reg_t defReg_t(RegId const r);
constexpr int bytes(RegId const r);
constexpr int align(RegId const r);

inline constexpr Reg_t defReg_t(RegId const r){
    return ( r <= IDscalar_last )? SCALAR
        : ( r <= IDvector_last )? VECTOR
        : ( r <= IDvmask_last  )? VMASK
        : FREE;
}

inline constexpr int align(Reg_t const flags){
    return (flags&MSK_TYPE)==SCALAR2? 16U: 8U;
    //return (flags&MSK_TYPE)==SCALAR? 8
    //    : ( flags&MSK_TYPE)==VECTOR? 8
    //    : 8;
}
inline constexpr int align(RegId const r){
    // we assume base type of SCALAR / VECTOR / VMASK, all same align.
    return 8U;
}

#if 0
inline int bytes(Reg_t flags){
    assert( (flags&MSK_TYPE) << SHIFT_TYPE < 5 );
#if 0
    static const int defBytes[8] = { 8, // SCALAR
        16,                     // SCALAR2
        MVL*SCALAR_BYTES,       // VECTOR
        MVL/8,                  // VMASK
        2*MVL/8,                // VMASK2
        0,0,0                   // [unused]
    };
#endif
    if( regType(flags)==SCALAR )
        return 8;
    if( regType(flags)==SCALAR2 )      // e.g. long double
        return 16;
    if( regType(flags)==VECTOR ){
        int vlen = ((flags&MSK_VLEN) >> SHIFT_VLEN) * SCALAR_BYTES;
        // vlen==0 is interpreted as "max length"
        if( vlen==0 ) vlen = MVL * SCALAR_BYTES;
        return vlen;
    }
    // NB: mask registers always treated as full length
    if( regType(flags)==VMASK )
        return MVL/8;       // mask register holds MVL bits
    if( regType(flags)==VMASK2 )
        return (2*MVL)/8;   // VMASK2, covering 2 %vm registers
    assert(false);
    return 0;
}
inline constexpr int bytes(RegId const r){
    // we assume base type of SCALAR / VECTOR / VMASK, all same align.
    // and for VECTOR, we assume max-length (since we have no vlen knowledge)
    if( r <= IDscalar_last ) return 8;
    if( r <= IDvector_last ) return MVL*SCALAR_BYTES;
    if( r <= IDvmask_last  ) return MVL/8;
    assert(false);
    return 0;
}
#else
inline constexpr int bytes(Reg_t flags){
    return regType(flags)==SCALAR? 8
        : regType(flags)==SCALAR2? 16
        : regType(flags)==VMASK? MVL/8
        : regType(flags)==VMASK2? (2*MVL)/8
        : regType(flags)==VECTOR?
        /*   */ ((flags&MSK_VLEN)? vlen(flags): MVL) * SCALAR_BYTES
        : 0/*hopefully never!*/
        ;
}
inline constexpr int bytes(RegId const r){
    return r <= IDscalar_last? 8
        : r <= IDvector_last? MVL*SCALAR_BYTES
        : r <= IDvmask_last? MVL/8
        : 0/*hopefully never*/
        ;
}
#endif

}//ve::
// vim: ts=4 sw=4 et cindent cino=^=l0,\:.5s,=-.5s,N-s,g.5s,h.5s,b1 cinkeys=0{,0},0),\:,0#,!^F,o,O,e,0=break
#endif // REGDEFS_HPP
