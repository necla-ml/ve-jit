#ifndef REG_AURORA_HPP
#define REG_AURORA_HPP
/** \file
 * Implement standard register definition functions for Aurora.
 */

// all chip impls should include reg-base.hpp first:
#include "reg-base.hpp"
#include <type_traits>
#include <array>

/** \group Aurora details */
//@{
constexpr int MVL          = 256;               // maximum vector length (in byte8 units)
constexpr int SCALAR_BYTES = 8;

// TODO replace with RegId typed enum, names IDfoo --> Rid::foo, RidLast::foo, RidN::foo, ...
// OR Regid const IDfoo ?
//enum class NREG : std::underlying_type<RegId>::type { s=64, v=64, vm=16 };
constexpr RegId NREG_s = Regid(64);
constexpr RegId NREG_v = Regid(64);
constexpr RegId NREG_vm = Regid(16);

constexpr RegId IDscalar      = Regid(0);
constexpr RegId IDscalar_last = Regid(IDscalar+NREG_s-1);    // 0..63
constexpr RegId IDvector      = Regid(IDscalar_last+1);
constexpr RegId IDvector_last = Regid(IDvector+NREG_v-1);    // 64-127
constexpr RegId IDvmask       = Regid(IDvector_last+1);
constexpr RegId IDvmask_last  = Regid(IDvmask +NREG_vm-1);   // 128-143
constexpr RegId IDlast        = Regid(IDvmask_last + 1);
constexpr RegId IDnone        = Regid(IDlast);               // 144
//@}
//inline bool constexpr operator<(RegId const r, StartId_e const s) {return r<s;}
// XX inline constexpr operator NREG (StartId_e const s) {return static_cast<NREG>(s);}

/** \group Aurora standard functions */
//@{
inline constexpr bool isReg(RegId const r){ return r<Regid(IDlast); }
inline constexpr bool valid(RegId const r){ return r<Regid(IDlast); }
inline constexpr RegId invalidReg(){ return Regid(IDlast); }

#if 0
// naive impls, somewhat generic
inline constexpr bool isScalar(RegId const r){ return r<NREG_s; }
inline constexpr bool isVector(RegId const r){ return r>=IDvector && r < IDvector_last+1; }
inline constexpr bool isMask  (RegId const r){ return r>=IDvmask  && r < IDvmask_last+1; }
#else
// faster, for aurora only
inline constexpr bool isScalar(RegId const r){ return unsigned(r)/64 == 0; }
inline constexpr bool isVector(RegId const r){ return unsigned(r)/64 == 1; }
inline constexpr bool isMask  (RegId const r){ return unsigned(r)/16 == 8; }
#endif


/** reg-aurora lookup tables can streamline some simple ops */
namespace detail {
extern char const* aurora_regnames; ///< every 6th char begins next zero-terminated register string
//constexpr uint32_t reserve_c    =               0b111111111100000000;
constexpr uint32_t reserve_none =                 0b111100111100000000; ///< %s12,%s13 can be used for anything
constexpr uint64_t preserve_c   = 0b1111111111111111111100101100000000;
}//detail::
        
inline char const* asmname(RegId const r){
    assert(valid(r));
    return &detail::aurora_regnames[6*r];
}
inline constexpr bool abi_c_reserved(RegId const r){
    // reserved: 8--17  (some might not be preserved and have other Abi::c uses)
    return r<=17 && r>=8 /*&& (r!=10 && r!=12 && r!=13)*/;
    //return r<=17 && ((uint32_t{1}<<r) & detail::reserve_c);
}
inline constexpr bool abi_none_Reserved (RegId const r){      ///< "don't touch me"
    // default: assume Abi::none
    //return r<=17 && r>=8 && (r!=13 && r!=12 /*&& r!=10*/);
    return r<=17 && ((uint32_t{1}<<r) & detail::reserve_none);
}
/** \em reserved registers are usually set/restored during prologue/epilogue.
 * - User typically never uses them, so do not need any save/restore.
 * - \em unpreserved \em reserved registers might be used in client code,
 *   and if possible the usage matches that of the C ABI.
 *   - Ex. %lr [aka %s10] might be used to call functions.
 * - if you have <b>no external symbols/calls</b>, you can get use of
 *   %s12..%s17 by save/restore of %s14..%s17 during prologue/epilogue.
 * - stack pointer %s11=%sp can be modified for local vars, w/ support from prologue/epilogue.
 */
inline constexpr bool isReserved (RegId const r, Abi const abi/*=Abi::none*/ ){ ///< "don't touch me"
    return abi==Abi::none ? abi_none_Reserved(r) : abi_c_reserved(r);
}

/** \em preserved registers may need prologue/epilogue save/restore.
 * Across arbitrary function calls \b any \em non-preserved registers
 * might get clobbered! */
inline constexpr bool abi_c_preserved(RegId const r){
    // reserved: 8, 9, 11, 14, 15, 16, 17
    //return r<=33 && r>=8 && (r!=10 && r!=12 && r!=13);
    return r<=33 && ((uint64_t{1}<<r) & detail::preserve_c);
}
inline constexpr bool isPreserved(RegId const r, Abi const abi/*=Abi::none*/ ){ ///< "restore on function exit"
        return abi==Abi::none? false // any unreserved regs are fair game
            : abi_c_preserved(r)     // non-reserved regs %s18--%s33
            ;
}
inline constexpr bool canBeArg(RegId const r, Abi const abi/*=Abi::none*/ ){    ///< might be used after prologue
    return abi==Abi::none? false: r<8;
}

/** default "max length" of register \c Cls */
inline constexpr int defBytes(RegId const r){
    return r <= IDscalar_last? 8
        : r <= IDvector_last? MVL*SCALAR_BYTES
        : r <= IDvmask_last? MVL/8
        : 0/*hopefully never*/
        ;
}
inline constexpr int defBytes(RegisterBase::Cls const cls){
    return cls==RegisterBase::Cls::scalar? 8
        : cls==RegisterBase::Cls::vector? MVL*SCALAR_BYTES
        : cls==RegisterBase::Cls::mask? MVL/8
        : 0/*hopefully never*/
        ;
}

/** we assume base type of SCALAR / VECTOR / VMASK, all same align. */
inline constexpr int defAlign(RegId const r){
    return 8U;
}
inline constexpr int defAlign(RegisterBase::Cls const cls){
    return 8U;
}

#if 0 // naive
inline constexpr RegisterBase::Cls cls(RegId const r)
{
    return r <= Regid(IDscalar_last)? RegisterBase::Cls::scalar
        : r  <= Regid(IDvector_last)? RegisterBase::Cls::vector
        : r  <= Regid(IDvmask_last) ? RegisterBase::Cls::mask
        : RegisterBase::Cls::none;
}
#else // aurora
inline constexpr RegisterBase::Cls cls(RegId const r){
    return valid(r)
        ? RegisterBase::Enum<RegisterBase::Cls>( unsigned(r)/64 )
        : RegisterBase::Cls::none;
}
#endif
class ChipRegistersAurora : public ChipRegisters {
  public:
    static uint32_t constexpr sz = 64+64+16;
    static ChipRegisters& Instance(){   ///< A singleton object
        static ChipRegistersAurora obj;
        return obj;
    }
    // Note: RegBase::Misc is totally up to chipset to define
    enum Xmisc { reserve=1, preserve=2 }; ///< extra RegBase::Misc flags
    virtual ~ChipRegistersAurora() {}
    inline RegisterBase const& operator()(RegId r) const{ return regs[r]; }
    inline RegisterBase      & operator()(RegId r)      { return regs[r]; }
    inline RegisterBase const& operator()(uint32_t r) const{ return regs[r]; }
    inline RegisterBase      & operator()(uint32_t r)      { return regs[r]; }
    inline char const* asmname(RegId r) { return ::asmname(r); }
    uint32_t size() const { return sz; }
  protected:
    ChipRegistersAurora(); ///< singleton constructor (executes only once)
  private:
    //std::array<RegisterBase,IDlast> regs;
    std::array<RegisterBase,sz> regs;
};

inline ChipRegisters& mkChipRegistersAurora() {
    return ChipRegistersAurora::Instance();
}

inline ChipRegisters& mkChipRegisters() {
    return ChipRegistersAurora::Instance();
}

constexpr RegisterBase::Flags::StorageType defRegFlags(RegId const r){
    return 0
        | RegisterBase::shift(RegisterBase::Use::free)
        | RegisterBase::shift(cls(r))
        | RegisterBase::shift(RegisterBase::Sub::def)
        | RegisterBase::shift(RegisterBase::Vlen::def) // 0
        //| RegisterBase::shift(RegisterBase::Misc::def) // 0
        | RegisterBase::Flags::misc_t::mkval(
                (  abi_c_reserved (r)? ChipRegistersAurora::reserve  :0)
                | (abi_c_preserved(r)? ChipRegistersAurora::preserve   :0) );
        ;
}

inline constexpr RegisterBase::vlen_t defMaxVlen(RegId const r){
    return ( r <= IDscalar_last )? 1       // i.e. one 64-bit reg
        : ( r <= IDvector_last )? MVL      // MVL * 64-bit reg
        : ( r <= IDvmask_last  )? MVL/8/8  // # of 64-bit regs for MVL mask bits
        : 0;
}

#if 0
/** a slow impl, prefer asmname(r) */
inline std::string strname(RegId const r){
    char const* pfx = (isScalar(r)?"s":isVector(r)?"v":"vm");
    unsigned n = (isScalar(r)? r - IDscalar:
            isVector(r)? r - IDvector: r - IDvmask);
    std::ostringstream oss;
    oss<<'%'<<pfx<<n;
    return oss.str().c_str();
}
#endif

inline constexpr RegisterBase::vlen_t maxVlen(RegisterBase::flags_t const r){
    // no special subtypes for Aurora
    // illegal type for constexpr:      return RegisterBase::Flags{r}.vlen;
    return RegisterBase::Flags::vlen_t{r};
}
inline constexpr RegisterBase::vlen_t maxVlen(RegId const r){
    return  maxVlen(defRegFlags(r));
}

//inline constexpr int bytes(RegisterBase::flags_t const r){
//    return maxVlen(r) * SCALAR_BYTES;
//}
inline constexpr int bytes(RegId const r){
    return maxVlen(defRegFlags(r)) * SCALAR_BYTES;
}
//constexpr int align(RegisterBase::flags_t const r){
//    return 8U;
//}
constexpr int align(RegId const r){
    return 8U;
}
constexpr RegisterBase::Use use(RegisterBase::flags_t const r){
    return RegisterBase::Enum<RegisterBase::Use>
        (RegisterBase::Flags::use_t{r}.z());
}

#if 0
/** Id--> assembler register name, asmname(0)-->"%s0" */
/** scalar, vector, vmask regs each have ids in range [0,64+64+16=143) */
inline char const* asmname(RegId const r){ // in .cpp file?
#endif


//constexpr  defReg_t(RegId const r); --> defRegFlags(r)
//TODO
#if 0
constexpr int bytes(RegId const r);
constexpr int align(RegId const r);

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
#endif

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
#endif
#if 0
inline constexpr int bytes(Reg_t flags){
    return regType(flags)==SCALAR? 8
        : regType(flags)==SCALAR2? 16
        : regType(flags)==VMASK? MVL/8
        : regType(flags)==VMASK2? (2*MVL)/8
        : regType(flags)==VECTOR?
        ( (flags&MSK_VLEN)? ((flags&MSK_VLEN) >> SHIFT_VLEN) * SCALAR_BYTES
          : MVL * SCALAR_BYTES )
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

#if 0
/** scalar, vector, vmask regs each have ids in range [0,64+64+16=143) */
inline constexpr bool isScalar(RegId const r){} ///< typically, RegisterBase::flag.use == Use::SCALAR
inline constexpr bool isVector(RegId const r){}
inline constexpr bool isMask(RegId const r){}

inline constexpr bool isReserved(RegId const r){}       ///< "don't touch me"
inline constexpr bool isPreserved(RegId const r){}      ///< "restore on function exit"

inline constexpr int defBytes(RegId const r){}
inline constexpr int defAlign(RegId const r){}

//char const* asmname(RegId const r);
constexpr int defBytes(RegId const r);
constexpr int defAlign(RegId const r);
#endif


//@} Aurora standard functions

// vim: ts=4 sw=4 et cindent cino=^=l0,\:.5s,=-.5s,N-s,g.5s,h.5s,b1 cinkeys=0{,0},0),\:,0#,!^F,o,O,e,0=break
#endif // REG_AURORA_HPP
