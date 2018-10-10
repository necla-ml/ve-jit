#ifndef ASM_HPP
#define ASM_HPP
/** \file
 * \deprecated replaced by symbStates.hpp (rewrite)
 */
#include <forward_list>
#include <unordered_set>
#include <unordered_map>
#include <stack>
#include <vector>
#include <cstdint>
#include <cassert>
#include <stdexcept>
#include <iosfwd>
#include <type_traits>

class Tester;   ///< fwd decl, use to test private methods

namespace ve {

/** Generic register attributes.
 * - Usage:
 *   - RESERVED, USED, MSK_FREE
 * - Function-level requirements:
 *   - PRESERVE (callee must save)
 * - register class:
 *   - SCALAR (64-bit), VECTOR(256*8-byte), VMASK(256-bit)
 *   - VMASK2 (pair of VM registers, for packed-vector ops)
 */
enum Reg_t : int {
    FREE=0, RESERVED=1, USED=2, MSK_FREE=3,
    SCALAR=0<<2, VECTOR=1<<2, VMASK=2<<2, VMASK2=3<<2, MSK_TYPE=3<<2,
    PRESERVE=1<<4, MSK_ATTR=1<<4
};
#define REG_T(I) static_cast<Reg_t>(I)
#if 0
/* allow logical ops on Reg_t to remain in domain-of-Reg_t (int) */
#define ECAST(T,V) static_cast<T>(V)
#define UNDER(T) std::underlying_type<T>::type
#define UNDERCAST(T,V) static_cast<UNDER(T)>(V)
#define ENUM_OPS(T) \
    inline T operator~ (T a) { return ~UNDERCAST(T,a); } \
inline T operator| (T a, T b) { return ECAST(T, \
        (UNDERCAST(T,a) | UNDERCAST(T,b))   ); } \
inline T operator& (T a, T b) { return UNDERCAST(T,a) & UNDERCAST(T,b); } \
inline T operator^ (T a, T b) { return UNDERCAST(T,a) ^ UNDERCAST(T,b); } \
ENUM_OPS(Reg_t)
//#undef ENUM_OPS
//#undef UNDERCAST
//#undef UNDER
#endif

std::ostream& operator<<(std::ostream& os, Reg_t const f);

int const MVL = 256;    // maximum vector length (in byte8 units)

/** RegId == IDlast is "unassigned" */
typedef uint_least16_t RegId;

// TODO replace with RegId typed enum, names IDfoo --> Rid::foo, RidLast::foo, RidN::foo, ...
// OR Regid const IDfoo ?
enum NREG : RegId { s=64, v=64, vm=16 };
enum StartId_e {
    IDscalar= 0,               IDscalar_last=IDscalar+NREG::s-1, // 0..63
    IDvector= IDscalar_last+1, IDvector_last=IDvector+NREG::v-1, // 64-127
    IDvmask = IDvector_last+1, IDvmask_last =IDvmask +NREG::vm-1,// 128-143
    IDlast  = IDvmask_last + 1 };                                // 144
#if 0 // TODO
enum RID : RegId {
    s=0,        sEnd=s+NREG:s,      // [0..64)
    v=sEnd,     vEnd=v+NREG:v,      // [64..128)
    vm=vEnd,    vmEnd=vm+NREG:vm,   // [128..144)
    X=vmEnd };                      // 144 X ~ default out-of-range value
#endif



/** handy testers, but perhaps not optimal */
bool constexpr isScalar(RegId const r){ return r<NREG::s; }
bool constexpr isVector(RegId const r){ return r>IDvector && r < IDvector_last+1; }
bool constexpr isMask(RegId const r){ return r>IDvmask && r < IDvmask_last+1; }
bool constexpr isReg(RegId const r){ return r<IDlast; }

/** scalar, vector, vmask regs each have ids in range [0,64+64+16=143) */
/** Id--> assembler register name, asmname(0)-->"%s0" */
char const* asmname(RegId const r);

class Locn;
class Sym;
class Regs;
class Spill;
class Scope;

/* Where can something be stored? */
struct Locn {
    enum Locn_t { REGISTER, ABSOLUTE, STACK };
    long l; ///< absolute address OR relative byte offset OR RegId
};
/** Symbol name and state.
 * - declare RESERVED (internal only)
 * - declare arg: TODO
 * - scope
 * - declare local:
 * - use local:
 * - spill arg
 * - modify arg ?
 * - spill local
 * - macro-ops
 * - macro-op tmp symbols
 */
struct Sym {
    Sym(char const* name);///< inactive symbol (no reg/mem yet, for testing)

    char const* name;
    bool active;                ///< sym can be \c active w/ \c id (if in register)
    RegId id;                   ///< register id (or IDlast)
    int use;                    ///< flags: 'or' of Reg_t
    unsigned uid;               ///< uid at decl point
    uint64_t t_decl;            ///< decl tick, also is Sym uid
    uint64_t t_sym;             ///< NEW: distinct from Regs t_use

    typedef enum { NONE=0, ARG=1, SPILL=2 } Where;
    Where mem;                  ///< if not reg, may have original/alt location in memory.
    unsigned scope;             ///< scope uid; 1=function, 2,3...=sub-blocks (never re-use these)
    int staleness;              ///< how many times might reg value changed since spill/mem value?

    // alignment and len specs may change if we are able to provide more information
    // about the scalar or vector type (e.g. float vs double vs packed-float vs uint32_t etc)
    int align;                  ///< default align = 8
    int len;                    ///< len(bytes) defaults to max (VECTOR:MVL*8, VMASK:4*8, VMASK2:8*8)
    // [maybe above block belongs in Regs?]

    bool isActive() const { return active; }
    /** location could be register and/or memory */
    bool isReg() const { return active && id<IDlast; }
    bool isMem() const {
        bool ismem = active && id >= IDlast;
        assert( !(ismem && mem==NONE) );
        return ismem;
    }
#if 0
    /** Inform that any mem area is stale, without quite releasing the memory. */
    void regchange() {
        assert( active && "regchange symbol must be an active register" );
        assert( regs->usage(id) != RESERVED );
        staleness = (mem==NONE? 0: staleness+1);
    }
    /** Inform that spill area is no longer needed, memory might be re-used. */
    void unspill();
    /** assign a symbol to a register of \c type SCALAR/VECTOR/VMASK */
    explicit Sym(enum Reg_t type, char const* name, Regs* regs);
#endif

    /** construct symbol for a register */
    explicit Sym(RegId const id, char const* const name, bool const active, int/*Reg_t*/use_flags, unsigned const nz_uid);
};
std::ostream& operator<<(std::ostream& os, Sym const& sym);

std::runtime_error sym_out_of_scope(Sym const* sym);

/** a safe Sym* checks that 'scope' is still valid every time it is dereferenced. */
class SafeSym {
  public:
    SafeSym() : sym_id(0U), regs(nullptr) {}
    /** SafeSym behaves "like" a Sym* */
    Sym const* operator->() const;
    Sym* operator->();
    Sym const& operator*() const;
    /** also has a Regs pointer, to access more state info. */
    friend std::ostream& operator<<(std::ostream& os, SafeSym const& sym);
    /** getters for Regs-related symbol info */
    int reg_flags() const;    ///< Regs::Usage::flags (or 0)
    uint64_t time() const;    ///< Regs::xxxTime stamp
    unsigned id() const { return this->sym_id; }
    // /** return ptr to any known symbol (or throw) */
    //Sym* find(unsigned *pscope=nullptr);
    //Sym const* find(unsigned *pscope=nullptr) const;
    /** return ptr to active-scope symbol (or throw) */
    Sym* lookup(unsigned *pscope=nullptr);
    Sym const* lookup(unsigned *pscope=nullptr) const;
  private:
    friend class Regs;
    //Sym& operator*() { assert(sym); return *sym; }
    SafeSym(Sym* sym, Regs *regs) : sym_id(sym->uid), regs(regs) {}
    //Sym *sym; // Issue: this is not owned and can disappear
    unsigned sym_id;
    Regs *regs;
};
//std::ostream& operator<<(std::ostream& os, SafeSym const& sym); // not req'd [friend decl]
std::runtime_error sym_unset(SafeSym const* ssym);
std::runtime_error invalid_RegId(char const* file, int line, RegId id);

/** Track spill area usage and requirements.
 *
 * After fn prologue, the spill area is above the parameter area,
 * and below the frame pointer.  You can allocate "local variables"
 * in this memory area.
 *
 * Before we emit any code, we will know the total amount of stack
 * space for spill/local vars (and use this in function prologue
 * to add a new stack frame)
 */
struct Spill {
    // Sym object must provide:
    //          int align;      > 0, power-of-two
    //          int len;        > 0
    //Spill(Regs* regs) : regs(regs), bottom(0), use() {}
    typedef struct {
        Sym *sym;       ///< nullptr if area is free
        int offset;     ///< -ve value, wrt %fp
        int len;        ///< byte length of memory
    } Region;
    /** Spill a sym that is currently in a register.
     * Default align is always 8, but might be overridden if
     * more detailed info (ffilib, float/double/packed) info is
     * available.
     */
    void spill(Sym *sym, int align=8);
    /** remove a Sym memory area & agglomerate adjacent empty regions */
    void erase(Sym const* sym);
    /** iterate over symbols, removing Spill copies of all in-register syms.
     * A. Whenever sym->mem is NONE, it is in-register (and cannot be here).
     * B. Whenever sym->staleness is > 0, the in-register version is more
     *    recent than the last spilled version.
     * Here we change all case (B) to case (A), erasing all stale spill copies.
     */
    void gc();
    /** Read a sym from spill into given [free] register.
     * (TENTATIVE)
     * (may need to adjust to match RegId class?) */
    void read(Sym *sym, RegId const id);
  private:
    //Regs *regs;
    int bottom;                 ///< track max size of 'local variable' area
    // bottom should aligndown during prologue generation to align 16
    typedef std::forward_list<Region> Uses;
    /// Note that before code emit, we can re-order that offset/len assignment
    /// at will during 'spill' so that we don't fragment things too much.
    Uses use;       ///< locals are maintained in order of \c offset

    /** create a new storage area and return iter to new \c Uses entry. */
    Uses::const_iterator newspill(Sym *sym);
    /** store reg in spill region (code gen!) */
    void emit_spill( Sym *sym, Uses::const_iterator& at );

    /** return a 'prev' iterator after which a right-sized
     * hole could be filled. \c size >0, \c align is 2^n.
     * \c use.before_begin() or last item are allowed return values.
     */
    Uses::const_iterator find_hole( int const len, int const align ) const;
    void contiguity_check() const;
};
/* symbols can have auto-scope [eventually] */
class Scope {
  public:
    Scope(int scope_id) : uid(scope_id) {}
    ~Scope();                   ///< release symbols, regs, mem
  private:
    friend class Regs;
    unsigned uid;               ///< unique scope_id
    //std::vector<Sym> syms;
    std::forward_list<Sym> syms;
    /** remove [a subscope] */
};

/** Describe registers and their associated symbolic labels. This state evolves. */
class Regs {
  public:
    friend class ::Tester;
    friend class SafeSym;
    friend std::ostream& ::ve::operator<<(std::ostream& os, SafeSym const& sym);
    /** construct initial register state as for 'fn(void)',
     * after [quick] function prologue has executed. */
    Regs();
  protected:

    /** Usage entries associate Sym with register */
    struct Usage {
        //
        // TODO
        //      A. move 'use', which has TYPE INFO into Sym class
        //         - allow declare to precede "use" [setting a value]           DONE!
        //      B. move 'timestamp' info into Usage class
        //         - allow no-op spilling of an oldest inactive+USED reg
        //
        //int use;                ///< sum of Reg_t flag bits
        Sym *sym;               ///< symbol name & state
        //uint64_t t_decl;        ///< decl     tick TODO *this* should be in Sym !
        uint64_t t_used;        ///< last-use tick
        //int scope;
        //int idx;                ///< Sym is at scopes[scope].syms[idx]
        inline bool isFree() const       { assert(sym); return (sym->use&MSK_FREE)==FREE; }
        inline bool isReserved() const   { assert(sym); return (sym->use&MSK_FREE)==RESERVED; }
        inline bool isUsed() const       { assert(sym); return (sym->use&MSK_FREE)==USED; }
        inline bool mustPreserve() const { assert(sym); return (sym->use&PRESERVE); }
      private: // utils, with no sanity checks
        friend class Regs;
        inline void setFree() { sym->use = (sym->use&~MSK_FREE); sym=nullptr; }
    };

    /** Spill entries associate Sym with memory */
    Spill spill;
    Usage s[64]; ///< reg id 0-63 TODO change these to std::array
    Usage v[64]; ///< reg id 64-127
    Usage m[16]; ///< reg id 128-143
    Usage& usage(RegId const id){
        if(id <= IDscalar_last)      return s[id-IDscalar];
        else if(id <= IDvector_last) return v[id-IDvector];
        else if(id <= IDvmask_last)  return m[id-IDvmask];
        else throw invalid_RegId(__PRETTY_FUNCTION__,__LINE__,id);
    }
    Usage const& usage(RegId const id) const {
        if(id <= IDscalar_last)      return s[id-IDscalar];
        else if(id <= IDvector_last) return v[id-IDvector];
        else if(id <= IDvmask_last)  return m[id-IDvmask];
        else throw invalid_RegId(__PRETTY_FUNCTION__,__LINE__,id);
    }

    /** to detect lru register of each type */
    uint64_t tick;
    /** setvl in range 0..MVL (0 => MVL, maybe?). if op veclen differs,
     * we need to setvl (and warn?). */
    int veclen;

    //uint64_t sTime[64];
    //uint64_t vTime[64];
    //uint64_t mTime[64];
    uint64_t time(RegId const id) const { return usage(id).t_used; }
    uint64_t& time(RegId const id) { return usage(id).t_used; }
    void free_register( RegId const id );

    typedef struct { Sym *sym; unsigned scope_uid; } SymPtrInfo;
  private:
    friend class SafeSym;
    std::unordered_set<int> active_scopes;
    std::unordered_map<unsigned,SymPtrInfo> active_symbols;
    unsigned scope_uid;
    unsigned sym_uid;
    /** All \c Sym exist in some \c scopes, with scopes.front() being the innermost
     * scope (returned by \c scope() function). Scopes form a chain all the
     * way back some root scope.  Regs pertains to a single function, so root
     * scope is function scope. */
    std::forward_list<Scope> scopes;
    Scope& scope()              { return scopes.front(); }
    const Scope& scope() const  { return scopes.front(); }
    //typedef struct { Sym* sym; int nscope; } Lookup;
    //Lookup lookup(char const* name);
    Sym* lookup(char const* name, int *nscope=nullptr) const;
    Sym* lookup(unsigned uid, unsigned *scope=nullptr) const;
    void scope_init();  ///< init function-global scope
  public:
    uint64_t now() const {return tick;}
    static uint64_t const NEVER = ~uint64_t{0}; ///< big number, end of time

    /** begin a new symbol scope */
    void scope_begin();
    /** end current [innermost] symbol scope, freeing all its reg/mem use.
     * \throw if you try to delete the root scope.
     * This invalidates all SafeSym you have created with \c alloc. */
    void scope_end();

    /** Flexible register declare/allocate.
     *
     * Always return a safe Sym*, in sense that we can easily check
     * if the Sym* is valid --- we want to catch uses of freed pointers,
     * when user uses a symbol that is no longer in scope.
     *
     * Always returns with \c Sym::active==false.
     *
     * When \c type is one of {SCALAR,VECTOR,VMASK,VMASK2}, we merely \em declare
     * they symbol, without allocating a register.  \c Sym::id set to
     * IDlast = <em>no register</em>. This is a simple operation.
     *
     * When \c type = USED|SCALAR... (logical-or of \c Reg_t enums)
     * then we declare the symbol
     * AND reserve a register.  This might spill other registers.
     * The returned SafeSym is an "indeterminate register", with
     * \c Sym::id = \em valid, \c Usage::used==USED+... and \c !Sym::active.
     * The register is an "indeterminate-reg".
     */
    SafeSym alloc(Reg_t type_flags, char const* name);
    /** Declare a symbol, but do not assign it a register yet.
     * - Returns with \c Sym::active==false, \c Sym::id \em invalid.
     * - Register assignment [and any reg spill] will happen "at first use".
     * - First use is detected when code emission calls \c out(SafeSym).
     */
    SafeSym declare(Reg_t type, char const* name);

  private:
    /** private helper to set initial reg assignments & flags. */
    //void declare(RegId id, char const* const name, bool const active, int/*Reg_t*/ use_flags);
    void declare(RegId id, char const* const name, bool const active, Reg_t use_flags);

    /** if an active symbol is in memory, alloc and unspill it */
    void activate( Sym* sym );

  public:
    // /** Regs needs to be told how asm ops affect registers */
    // void input( SafeSym& sym )
    /** instruction macros must inform us about register usage.
     * - Main options:
     *   1. declared + out-of-scope ---> oops
     *   2. declared + in-scope ---> allocate reg (maybe spill).
     *   3. active in-reg? update stats (last-use time)
     *   4. active + spilled --->  allocate reg (maybe spill), unspill
     */
    void out( SafeSym& ss );
    /** This register was used but not modified.  Perhaps conveys hint that
     * we don't wish to spill this register (if keep lru list). */
    void in( SafeSym& ss );

  private:
    /** get an unassigned register (or return IDlast).
     * If spill is true, punt some LRU symbol to memory. */
    RegId getfree(int/*Reg_t & MSK_TYPE*/ type, bool doSpill=true);
    /** get an unassigned register (or return IDlast) probably short lifetime */
    RegId gettmp(int/*Reg_t & MSK_TYPE*/ type, bool spill=true);

    /** Inform that any mem area is stale, without quite releasing the memory. */
    void changed(RegId const r);
    /** You might wish to rename a symbol after a change.  The old symbol
     * disappears and has a new \c Sym::name. */
    void rename(RegId const r);

    // What about Sym or overflow regs in Stack space?
};
/* Describe args and return type of a 'C' function. */
class Cffi {
    /* Map argnames to input registers/stack offsets.
     * These initial mappings are valid after function prologue
     * has executed. */
    void argname(int const argn, char const* sym);
};
class WrapAsm {
  public:
    WrapAsm(Cffi const&) {}
};
//
//
// ----------------------- inline functions -------------------------
//
//
#if 0
inline Sym* SafeSym::operator->(){
    if( regs->active_scopes.find(sym->scope) == regs->active_scopes.end() )
        throw sym_out_of_scope(this->sym);
    if( this->sym==nullptr )
        throw sym_unset(this);
    return this->sym;
}
#endif
inline Sym const& SafeSym::operator*() const {
    assert(sym_id);
    return *regs->lookup(sym_id);
}
inline Sym* SafeSym::lookup(unsigned *pscope){
    return regs->lookup(sym_id,pscope);
}
inline Sym const* SafeSym::lookup(unsigned *pscope) const{
    return regs->lookup(sym_id,pscope);
}

}//ve::
// vim: ts=4 sw=4 et cindent cino=^=l0,\:.5s,=-.5s,N-s,g.5s,h.5s,b1 cinkeys=0{,0},0),\:,0#,!^F,o,O,e,0=break
#endif //ASM_HPP
