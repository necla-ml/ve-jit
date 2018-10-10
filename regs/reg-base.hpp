#ifndef REG_BASE_HPP
#define REG_BASE_HPP

#include "bitfield.hpp"
#include "throw.hpp"
#include <cstdint>
#include <stdexcept>

enum RegId : int_least16_t {};

std::runtime_error invalid_RegId(char const* file, int line, RegId id);

template<typename Int>
constexpr RegId Regid(Int const i) {
    return static_cast<RegId>(
            static_cast<std::underlying_type<RegId>::type>(
                i ));
}

/** \group std Register functions here.
 * - Chip impls should try to make these inline.
 * - These give standard or "default" values.
 * - Actual bytes/alignment may differ, based on [current] vlen or sub-register settings.
 *   - Get actual values by querying RegisterBase object.
 *   - Ex. you might have double-length types (long double, complex double)
 *   - or partial register types (float_hi, float_lo, packed_float, int8, int16, ...)
 *
 * Implementing these functions once allow you to implement several
 * RegisterBase functions fairly generically.
 */
//@{
/** Abi::none is for fully-assembler "register calling convention".
 * - Abi::none ...
 *   - cannot call any 'C' functions.
 *   - works in callers stack frame
 *     (so cannot spill anything to memory)
 *   - may use \em any \b unreserved registers freely.
 *   - may use dynamic linker to get addresses, (so s10 and s12 are also user-registers)
 *   - return value in %s0 typical, but may require client inline-asm to get other outputs
 */
enum class Abi : uint8_t {none, c};
constexpr bool isReg(RegId const r);
constexpr bool valid(RegId const r);
constexpr RegId invalidReg();

constexpr bool isScalar(RegId const r); ///< typically, RegisterBase::flag.use == Use::SCALAR
constexpr bool isVector(RegId const r);
constexpr bool isMask(RegId const r);

constexpr bool isReserved (RegId const r, Abi const abi=Abi::none);     ///< "don't touch me"
constexpr bool isPreserved(RegId const r, Abi const abi=Abi::none);     ///< "restore on function exit"
constexpr bool canBeArg(RegId const r, Abi const abi=Abi::none);        ///< might be used after prologue

/** scalar, vector, vmask regs each have ids in range [0,64+64+16=143) */
constexpr int defBytes(RegId const r);
constexpr int defAlign(RegId const r);

#if 1 // in ChipRegisters object!
/** Id--> assembler register name (Ex: asmname(0)-->"%s0").
 * Note: this must be defined in the reg-CHIP.hpp/cpp file */
char const* asmname(RegId const r);
#endif

//@} (end group std Register functions)

/** RegisterBase marries some of the constant properties from \ref reg-base.hpp
 * and provides bitfields that \em can be used to simple \em state.
 * Ex. Cls ~ type Sub ~ subclass, Vlen ~ [current] vector length, Misc ~ free/used/...
 */
class RegisterBase {
  public:
    typedef RegisterBase Me;
    typedef uint_least32_t flags_t; ///< short for Flags::StorageType

    // Flags
    BEGIN_BITFIELD_TYPE(Flags,flags_t)
        ADD_BITFIELD_MEMBER(use, 0, 4)
        ADD_BITFIELD_MEMBER(cls, 4, 4)
        ADD_BITFIELD_MEMBER(sub, 8, 8)
        ADD_BITFIELD_MEMBER(vlen, 16, 12)
        ADD_BITFIELD_MEMBER(misc, 28, 4)
        END_BITFIELD_TYPE();

    //static_assert( sizeof(Flags) == 128, "unexpected sizeof Flags bitfield union" );

    enum class Use : uint_least8_t { free=0, used=1, reserved=2, };   ///< one-of

    /** register class (none=~0, scalar=0, vector, mask, ... double-length register?)
     * - default set may not be present
     * - specific architectures can add enums via...
     *   - <TT>enum Class const SCALAR2 = static_cast<Class>(3);</TT> etc.
     */
    enum class Cls: uint_least8_t { none=255, scalar=0, vector=1, mask=2 };

    /** DEFAULT subclass is always max-length object storable in register.
     * - Can distinguish "equiv." types, like U64 vs. DOUBLE.
     * - Might represent sub-parts: FLOAT_HI, FLOAT_LO, FLOAT_PACKED, U8, U8_PACKED?
     */
    enum class Sub: uint_least8_t { def=0 };

    /** Vector register length is always in terms of base SCALAR register size.
     * - So \em not in units of \c Sub -register size.
     * - DEF means "maxVlen"
     *   - i.e. 256 for ve VECTOR, 4 for ve MSK, 1 for SCALAR and others
     */
    enum class Vlen: uint_least16_t { def=0 };

    enum class Misc : uint_least8_t { def=0 };

    typedef std::underlying_type<Vlen>::type vlen_t;

  public: // const data member
    RegId const rid;

  public: // helpers
    template<class EnumType, typename Int> static constexpr
        EnumType
        Enum(Int const i) {
            static_assert(std::is_enum<EnumType>::value==true, "Enum<T>(i) : T must be an enum type");
            return static_cast<EnumType>(i);
        }
    template<class EnumType> static constexpr
        typename std::underlying_type<EnumType>::type
        Int(EnumType const e) {
            static_assert(std::is_enum<EnumType>::value==true, "Enum<T>(i) : T must be an enum type");
            return static_cast<typename std::underlying_type<EnumType>::type>(e);
        }
    /** return or-able bitfield value associated with an enum item.
     * Useful when initializing Flags. */
    static constexpr Flags::StorageType shift( Use e ) {
        return Flags::use_t::mkval( Use(e) ); }
    static constexpr Flags::StorageType shift( Cls e ) {
        return Flags::cls_t::mkval( Int(e) ); }
    static constexpr Flags::StorageType shift( Sub e ) {
        return Flags::sub_t::mkval( Int(e) ); }
    static constexpr Flags::StorageType shift( Vlen e ) {
        return Flags::vlen_t::mkval( Int(e) ); }
    static constexpr Flags::StorageType shift( Misc e ) {
        return Flags::misc_t::mkval( Int(e) ); }

  public: // const data members
    // ///< max VECTOR register length, in units of SCALAR reg length!
    // std::underlying_type<Vlen>::type const maxVlen;

    /** construct default [max len] register */
    RegisterBase( RegId rid );
    /** define a specialty register */
    //RegisterBase( RegId rid, Flags::StorageType flags );
    RegisterBase( RegId rid, Flags const&& flags );
    friend std::ostream& operator<<(std::ostream& os, RegisterBase const& r);

  protected:
    Flags flags;
    std::runtime_error err_reserved(){
        std::ostringstream oss;
        oss<<" reg "<<rid<<" is Use::reserved";
        return std::runtime_error(oss.str());
    }
    /** throw if reserved */
    void nonconst(){
        if(flags.use == Int(Use::reserved)){
            throw err_reserved();
        }
    }
        
  public: // const getters
    /** \group utility */
    //@{
    /** remap Vlen::def (0) to some the default max len.
     * ? may need to rethink for "special" Cls+Sub combos !!!
     * See reg-impl.hpp. */
    std::underlying_type<Vlen>::type vlen() const;
    //{
    //    auto val = static_cast<std::underlying_type<Vlen>::type>( flags.vlen.z() );
    //    if(val==0) val=1;
    //    return val;
    //}
    std::underlying_type<Misc>::type misc() const {
        return static_cast<std::underlying_type<Misc>::type>( flags.misc.z() );
    }
    std::underlying_type<Misc>::type getFlags() const {
        return (Flags::StorageType)flags;
    }
    //@}
    /** \group getters, comparators */
    //@{
    bool operator==(Use const use) const {return flags.use == Int(use);}
    bool operator==(Cls const cls) {return flags.cls == Int(cls);}
    bool operator==(Sub const sub) {return flags.sub == Int(sub);}
    bool operator==(Vlen const vlen) {return flags.vlen == Int(vlen);}
    bool operator==(Misc const misc) {return flags.misc == Int(misc);}
    // std enum values have shortcuts
    bool free()     const {return (*this) == Use::free;}
    bool reserved() const {return flags.use == Int(Use::reserved);}
    bool used()     const {return flags.use == Int(Use::used);}
    // you can also access entire bitfields
    Use use() const {return static_cast<Use>(flags.use.z());}
    Cls cls() const {return static_cast<Cls>(flags.cls.z());}
    //@}

    /** \group setters, throw if == Use::reserved */
    //@{
    /** Usage can only go \em to Use::reserved.
     * - \c flags.use==u invokes \c BitFieldMember::operator=(T=uint_least32_t).
     * - After that this object becomes "const" (throw on attempt to modify?)
     * - \c set(Use::reserved) must be the last \c set operation you do.
     */
    Me& set(Use const u) {
        if(flags.use==Int(Use::reserved)) nonconst(); // throw
        flags.use = Int(u);
        return *this;
    }
    Me& set(Cls const c) {nonconst(); flags.cls = Int(c); return *this;}
    Me& set(Sub const s) {nonconst(); flags.sub = Int(s); return *this;}
    Me& set(Vlen const v) {nonconst(); flags.vlen = Int(v); return *this;}
    Me& set(Misc const m) {nonconst(); flags.misc = Int(m); return *this;}
    // Vlen may sometimes be a plain integer...
    //template<typename Int> static constexpr Vlen toVlen(Int const i) {return static_cast<Vlen>(i);}
    template<typename Int>
        Flags& setVlen(Int const i) {flags.vlen = Enum<Vlen>(i); return flags;}
    //@}

    //Use use() const {return flags.use;} ///< as x.use()==X::Use::FREE
    // using u = RegisterBase::Use;
    // if( x==u::FREE ) {...}

    // ve also has SCALAR2, VECTOR, VMASK, VMASK2
    //Cls type() = 0;             ///< SCALAR-->0
    //int bytes() = 0;            ///< register max bytes
    //int mAlign() = 0;           ///< memory image alignment
    //int mBytes() = 0;           ///< memory image bytes <= bytes() * vlen()
    friend class ChipRegistersAurora;
};// class RegisterBase

/** export Flags class type */
typedef RegisterBase::Flags RegFlags;

/** \group Register Flag helpers
 */
//@{
/** every chip has default max-length register types */
constexpr RegisterBase::Cls cls(RegId const r);
/** and default flags settings for every register */
constexpr RegisterBase::Flags::StorageType defRegFlags(RegId const r);
/** maxVlen (usually 1) is # of scalar-size registers.
 * - \c Cls::vector for aurora will use 256 (even for packed_float extension \c Cls)
 * - \c Cls::mask --> 4  (4*64 = 256 bits, since vector maxVlen is 256)
 * - ? scalar2 and vmask2 might double this, for "2 \em consecutive {even,odd}" registers
 */
constexpr RegisterBase::vlen_t defMaxVlen(RegId const r);

/** actual max vector length may be overridden for special \c Cls, \c Sub combos.
 * RegFlags is a union, so not a "LiteralType", so cannot be constexpr.
 * But RegisterBase::flags_t (a.k.a. RegisterBase::Flags::StorageType)
 * is some integer type. */
constexpr RegisterBase::vlen_t maxVlen(RegisterBase::flags_t const r);

//@}

/** RegId --> RegisterBase interface */
class ChipRegisters {
public:
    //set up chip register information (chip-specific).
    // Assume default C ABI. */
    //static ChipRegisters& Instance();   ///< A singleton object
    // no copy/move constructor
    ChipRegisters(ChipRegisters const&) = delete;
    ChipRegisters(ChipRegisters&&) = delete;
    ChipRegisters& operator=(ChipRegisters const&) = delete;
    ChipRegisters& operator=(ChipRegisters&&) = delete;
    virtual uint32_t size() const = 0;
protected:
    ChipRegisters() {}
public:
    //ChipRegisters() = 0;
    virtual ~ChipRegisters() {}
    virtual RegisterBase const& operator()(RegId r) const = 0; ///< RegId --> RegisterBase
    virtual RegisterBase      & operator()(RegId r)       = 0; ///< RegId --> RegisterBase
    virtual RegisterBase const& operator()(uint32_t const r) const = 0; ///< uint32_t --> RegisterBase
    virtual RegisterBase      & operator()(uint32_t const r)       = 0; ///< uint32_t --> RegisterBase
    virtual char const* asmname(RegId r) = 0; ///< assembly mnemonic
};
ChipRegisters& mkChipRegisters(); ///< generic, override per target


// /** default constructor constructs max-length registers */
// ohoh. defRegFlags uses processor-specific info!
//inline RegisterBase::RegisterBase( RegId rid )
//    : rid(rid), flags(defRegFlags(rid)) // throw if !valid(rid)
//{}

// Again, maxVlen is chip-specific...
//inline std::underlying_type<RegisterBase::Vlen>::type RegisterBase::vlen() const {
//    return flags.vlen.z() != Int(Vlen::def)
//        ? static_cast<std::underlying_type<Vlen>::type>(flags.vlen.z())
//        : maxVlen(flags);
//}

// if no special register subtypes, use:
//inline constexpr RegisterBase::vlen_t maxVlen(RegFlags const r){
//    return r.vlen;
//}

// vim: ts=4 sw=4 et cindent cino=^=l0,\:.5s,=-.5s,N-s,g.5s,h.5s,b1 cinkeys=0{,0},0),\:,0#,!^F,o,O,e,0=break
#endif // REG_BASE_HPP
