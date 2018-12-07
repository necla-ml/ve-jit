#ifndef REGSTATES_HPP
#define REGSTATES_HPP
#include "old/regDefs.hpp"
#include <forward_list>

class Tester;   ///< fwd decl, use to test private methods

namespace ve {

template<class SYMBSTATES> class Spill;

class Locn;
//class Sym;
class Regs;
class Scope;

/* Where can something be stored? */
struct Locn {
    enum Locn_t { REGISTER, ABSOLUTE, STACK };
    long l; ///< absolute address OR relative byte offset OR RegId
};

#if 0
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

    bool const isActive() const { return active; }
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
#endif // Sym

    /** construct symbol for a register */
    explicit Sym(RegId const id, char const* const name, bool const active, int/*Reg_t*/use_flags, unsigned const nz_uid);
};
std::ostream& operator<<(std::ostream& os, Sym const& sym);
std::runtime_error sym_out_of_scope(Sym const* sym);
#endif


#if 0
/** a safe Sym* checks that 'scope' is still valid every time it is dereferenced. */
class SafeSym {
  public:
    SafeSym() : sym_id(0U), regs(nullptr) {}
    /** SafeSym behaves "like" a Sym* */
    Sym const* operator->() const;
    Sym* operator->();
    Sym const& operator*() const;
    /** also has a Regs pointer, to access more state info. */
    friend std::ostream& ::ve::operator<<(std::ostream& os, SafeSym const& sym);
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
#endif // SafeSym
/** Describe registers and symbolic labels. This state evolves. */

class Regs {
  public:
    friend class ::Tester;
    //friend class SafeSym;
    //friend std::ostream& ::ve::operator<<(std::ostream& os, SafeSym const& sym);
    /** construct initial register state as for 'fn(void)',
     * after [quick] function prologue has executed. */
    Regs();
  protected:

#if 0
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
#endif
};
}//ve::
// vim: ts=4 sw=4 et cindent cino=^=l0,\:.5s,=-.5s,N-s,g.5s,h.5s,b1 cinkeys=0{,0},0),\:,0#,!^F,o,O,e,0=break
#endif // REGSTATES_HPP
