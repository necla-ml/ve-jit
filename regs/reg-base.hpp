#ifndef REG_BASE_HPP
#define REG_BASE_HPP
/** \file
 * This file has generic chipset flags, and forward declarations of functions that all
 * chipsets should implement.
 */

#include "bitfield.hpp"
#include "throw.hpp"
#include <cstdint>
#include <stdexcept>
#include <functional>
//#include <iosfwd> oh. throw.hpp pulls in even more


/** \group Regid(int) --> RegisterBase{RegId+flags}
 * These types describe properties that registers on manyy chipsets have.
 *
 * After a chipset defines some basic chipset-specific functions, the generic
 * objects here become usable.   For example, \c RegisterBase constructor requires
 * ```bool valid(RegId)`` and ```defRegFlags(RegId)``` to be defined.  Once that
 * is done, generic code gives you some getters/setters/printing.
 *
 * Can we take the required funcs as lambda-args?  Then we achieve full independence
 * from any chipset-specific header.
 *
 * Or we just assume the chipset impl knows what it's doing (way easier)
 * 
 */
//@{
/** RegId is a typed enum. Don't know that this strictness was worth
 * the bother, but at least a RegId will print as rNNN. */
enum RegId : int_least16_t { rBad=-1 };

// custom specialization of std::hash can be injected in namespace std
namespace std
{
    template<> struct hash<RegId>
    {
        typedef RegId argument_type;
        typedef std::size_t result_type;
        result_type operator()(argument_type const& r) const noexcept
        {
            short int rr = static_cast<std::underlying_type<RegId>::type>(r);
            return std::hash<std::underlying_type<RegId>::type>()(rr);
        }
    };
}

/** Making invalidReg the same for all chipsets reduces header dependencies */
constexpr RegId invalidReg(){
    return rBad;
}

inline std::ostream& operator<<(std::ostream& os, RegId const r){
    std::ostringstream oss; // this way setw(n) works nicely
    oss<<'r'<<static_cast<std::underlying_type<RegId>::type>(r);
    return os<<oss.str();
}

#if 0
/** Chipset-specific Basic Functions */
struct ChipRegIdsBase {
    ...
};
#endif
    

/** A \em readable cast-to-enum. */
template<typename Int>
constexpr RegId Regid(Int const i) {
    return static_cast<RegId>(
            static_cast<std::underlying_type<RegId>::type>(
                i ));
}

std::ostream& operator<<(std::ostream& os, RegId const r); ///< generic rNNN

/** RegisterBase marries some of the constant properties from \ref reg-base.hpp
 * and provides bitfields that \em can be used to simple \em state.
 * Ex. Cls ~ type Sub ~ subclass, Vlen ~ [current] vector length, Misc ~ free/used/...
 */
class RegisterBase {
  public:
    typedef RegisterBase Me;

  public: // const data member
    RegId const rid;

    /** \group bit flag types */
    //@{
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

    /** \group const flags */
    //@{
    /** \b const register class (none=~0, scalar=0, vector, mask, ...)
     * - default set may not be present
     * - specific architectures can add enums via...
     *   - <TT>enum Class const SCALAR2 = static_cast<Class>(3);</TT> etc.
     */
    enum class Cls: uint_least8_t { none=255, scalar=0, vector=1, mask=2 };

    /** \b const, chipset specific -- Aurora uses these to store ABI information
     * for RESERVED and PRESERVED registers in their 'C' ABI. */
    enum class Misc : uint_least8_t { def=0 };
    //@}

    /** \group dynamic flags */
    //@{
    /** \b dynamic usage importance free < used < reserved.
     * Note: this \c reserved flag is dynamic, perhaps meaning it is used and should not be
     * spilled because we think it might be very useful to keep it around. */
    enum class Use : uint_least8_t { free=0, used=1, reserved=2, };

    /** DEFAULT subclass is always max-length object storable in register,
     * preferring unsigned > integer > float types.
     * - Can distinguish "equiv." types, like U64 vs. DOUBLE.
     * - Might represent sub-parts: FLOAT_HI, FLOAT_LO, FLOAT_PACKED, U8, U8_PACKED
     * - signed vs unsigned for integer types
     * - 8 bits avail, so fairly flexible.
     */
    enum class Sub: uint_least8_t { def=0 };

    /** Vector register length is always in terms of base SCALAR register size.
     * - So \em not in units of \c Sub -register size.
     * - DEF means "maxVlen"
     *   - i.e. 256 for ve VECTOR, 4 for ve MSK, 1 for SCALAR and others
     */
    enum class Vlen: uint_least16_t { def=0 };
    //@}

    typedef std::underlying_type<Vlen>::type vlen_t;
    //@}

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

#if 0 // "coupled" generic/specific headers...
    /** construct default [max len] register */
    RegisterBase( RegId rid );
    /** define a specialty register. Use \c shift to shift predefined flags
     * into the correct position */
    //RegisterBase( RegId rid, Flags::StorageType flags );
    RegisterBase( RegId rid, Flags const&& flags );
#else // "uncoupled" -- let the chipset be responsible for only defining good stuff
    /** special registers MUST be declared with preset Flags settings.
     * It is the responsibility of the chipset to only use valid rid
     * and flags, because now we cannot nicely map invalid register
     * ids to a common value, \c invalidReg(). */
    inline RegisterBase( RegId rid, Flags const&& f )
        : rid(rid), flags(f)
        {}
#endif

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
    //@}
    /** \group boolean tests and getters */
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
    flags_t getFlags() const {
        return (Flags::StorageType)flags;
    }
    // you can also access entire bitfields (as their enum type)
    Use use() const {return static_cast<Use>(flags.use.z());}
    Cls cls() const {return static_cast<Cls>(flags.cls.z());}
    Sub sub() const {return static_cast<Sub>(flags.sub.z());}
    Vlen vlen() const {return static_cast<Vlen>(flags.vlen.z());}
    Misc misc() const {return static_cast<Misc>(flags.misc.z());}
#if 0 // old
    /** remap Vlen::def (0) to some the default max len.
     * ? may need to rethink for "special" Cls+Sub combos !!!
     * See reg-impl.hpp.  No No No. This would break genericity. */
    std::underlying_type<Vlen>::type vlen() const;
    {
        auto val = static_cast<std::underlying_type<Vlen>::type>( flags.vlen.z() );
        if(val==0) val=1;
        return val;
    }
    std::underlying_type<Misc>::type misc() const {
        return static_cast<std::underlying_type<Misc>::type>( flags.misc.z() );
    }
#endif
    //@}

    /** \group setters, throw if == Use::reserved */
    //@{
    /** Usage can only go \em to Use::reserved.
     * - \c flags.use==u invokes \c BitFieldMember::operator=(T=uint_least32_t).
     * - After that this object becomes "const" (throw on attempt to modify?)
     * - \c set(Use::reserved) must be the last \c set operation you do.
     * - \b NOTE: not so sure about how Use::reserved should be interpreted.
     *            (what is the most useful? Maybe should be a spill hint?)
     *      OH. this really applies to Misc::reserve which is an Aurora-level
     *          concept.... NOT HERE !!!
     *
     * So perhaps set should do no error checks at all (leave that to chipset)
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

std::ostream& operator<<(std::ostream& os, RegisterBase::Use const u);    
std::ostream& operator<<(std::ostream& os, RegisterBase::Cls const c);    
std::ostream& operator<<(std::ostream& os, RegisterBase::Sub const s);    
std::ostream& operator<<(std::ostream& os, RegisterBase::Vlen const vl);    
std::ostream& operator<<(std::ostream& os, RegisterBase::Misc const m);    

/** RegId --> RegisterBase abstract interface.
 * Specific chipsets will \c mkChipRegisters() to return this mapping type.
 */
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

//@}

/** \group std Register functions here.
 *
 * These must be defined when you implement a chipset.
 *
 * - Chip impls should try to make these inline.
 * - These give standard or "default" values.
 * - Actual bytes/alignment may differ, based on [current] vlen or sub-register settings.
 *   - Get actual values by querying RegisterBase object.
 *   - Ex. you might have double-length types (long double, complex double)
 *   - or partial register types (float_hi, float_lo, packed_float, int8, int16, ...)
 *
 * Implementing these functions once allow you to implement several
 * RegisterBase functions fairly generically.
 * Functions that accepta RegId should throw if RegId is invalid,
 * using \c invalid_RegId to throw a runtime_error.
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
/** but only some RegId are valid */
std::runtime_error invalid_RegId(char const* file, int line, RegId id);


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

// NEW functions. Inquiry baed on RegisterBase::Cls (scalar,vector,mask [none?])
constexpr int defBytes(RegisterBase::Cls cls);
constexpr int defAlign(RegisterBase::Cls cls);



/** every chip has default max-length register types */
constexpr RegisterBase::Cls cls(RegId const r);
/** and default flags settings for every register.*/
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

ChipRegisters& mkChipRegisters(); ///< generic, override per target

//@} (end group std Register functions)


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

// vim: ts=4 sw=4 et cindent cino=^=l0,\:.5s,=-.5s,N-s,g.5s,h.5s,b1 cinkeys=0{,0},0),\:,0#,!^F,o,O,e,0=break syntax=cpp.doxygen
#endif // REG_BASE_HPP
