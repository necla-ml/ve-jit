#ifndef SPILLABLE_BASE_HPP
#define SPILLABLE_BASE_HPP
/** \file
 * Basic Spillable Symbol Info.
 * \sa spill.hpp spill-impl.hpp regSymbol.hpp
 * \sa testSpill2.cpp, testSpill3.cpp
 *
 * \sa scopedSpillableBase.hpp for a version that is supervised by scope::SymbStates, which
 * means we don't need to worry about symbol ids, or maintaining an \c active flag.
 */

#include <iostream>
#include <cassert>

class SpillableBase;

//namespace ve {
//    template<class SYMBSTATES> class ve::Spill;
//}//ve::

/** What basic \em state describes spillable symbols?
 *
 * - Not included:
 *   - <em>reserved</em> registers (never usable for user symbols)
 *   - usable but <em>preserved</em> (for prologue/epilogue save/restore)
 *   - "const" variables
 *   - "static" variables
 *   - <em>function arguments</em>: these have their own reserved stack frame locations
 *     - pre-assigned stack frame addresses
 *     - upon entry, some may be in-register arguments, some may be memory (or both).
 *     - <em>const function arguments</em>
 *       - These have a pre-ordained memory location and are never
 *         spillable to a spill region (they can unspill, to load into a register)
 *
 * With this class, \c Spill objects, \ref spill.hpp, can run basic sanity checks.
 *
 * SpillableBase must track the accessibility/location[s] of symbol values.
 *
 * - Hierarchy:
 *   - declared: symbol declared (in some current, inner scope)
 *     - length (bytes) and alignment (when in MEM)
 *     - Do we need register-length vs memory-length?
 *     - Do we need SIMD-length? ...in what units? register-size units?
 *   - ACTIVE:   symbol in some active scope
 *     - REG:      symbol has value in a register (we don't track which register)
 *     - MEM:      symbol has value in memory (we don't track where exactly)
 *       - in MEM, but memory value old: \c getStale() > 0
 *       - in MEM, memory value ok, but not in REG (after register spill)
 * - Usage:
 *   - setREG(true) when symbol register-value changes
 *   - setMEM(true) typically automatic, during spill operations
 *   - TODO: \em unspill
 * ```
 * Reg declare:           ACTIVE, ~R ~M, stale=0
 * --- note --- OLDREG is "not-here", and is equivalent to !getREG() but with some stale RegId
 * Reg alloc:             OLDREG
 * Reg use:               if(OLDREG) OLDREG --> REG; staleness=0
 *                        else if(REG) same-as-modify
 * Reg modify:            REG --> REG; if(SPILL) {SPILL --> OLDSPILL}; ++staleness
 * Reg spill:             REG --> REG; staleness=0; set SPILL; unset OLDSPILL
 * Reg modify:            REG --> REG; if(SPILL) {SPILL --> OLDSPILL, ++staleness}
 * Reg out-of-scope:      unset REG, OLDREG, SPILL, OLDSPILL, staleness=0
 * ```
 * - OLDSPILL == SPILL && staleness>0
 *
 * - Related classes:
 *   - regSymbol.hpp : add name, register, register type, decl & use ticks
 *   - symScopeUid.hpp : scoped regSymbols
 */
class SpillableBase {
  public:
    /**  symbols constructed "\c active" ~ (in some valid scope).
     * - symbol \em location (\c Where) flags:
     *   - \c getActive(), \c inREG(), \c inMEM()
     * - allocation to a register or to a memory address <em>not here</em>.
     * - setting register value should call \c setREG(true)
     * - setting memory value should call \c setMEM(true)
     * - register value change ==> memory staleness up (if in memory)
     *
     */
    SpillableBase(unsigned const uid, int const bytes, int const align)
        : uid(uid), len(bytes), align(align),
        staleness(0U), locn(ACTIVE) {}

    //template<class SYMBSTATES> friend class ve::Spill;
    friend std::ostream& operator<<(std::ostream& os, SpillableBase const& x);
    typedef enum : int { NONE=0, ACTIVE=1, REG=2, MEM=4
        /*, ARGREG=8, ARGMEM=16*/
    } Where;
    Where getWhere() const          {return this->locn;}
    SpillableBase& setWhere(Where const w) {
        //assert( w & ~(ACTIVE|REG|MEM) == 0 );
        // perhaps allow derived classes to ADD flags
        this->locn = w;
        return *this;
    }

    /** symbol unique id */
    unsigned symId() const          {return this->uid;}
    /** Are we in valid scope? */
    bool getActive() const          {return (locn&ACTIVE);}
    int getBytes() const            {return this->len;}
    int getAlign() const            {return this->align;}
    // spill-specific ...
    bool getREG() const             { return (locn&REG); }
    bool getMEM() const             { return (locn&MEM); }
    /** when exiting scope, symbols should \c setActive(false).
     * This also sets \c setREG(false) and \c setMEM(false). */
    SpillableBase& setActive(bool set=true) {
        setREG(false).setMEM(false);
        locn = static_cast<Where>( set? locn|ACTIVE: locn&~ACTIVE );
        assert( !getActive() && !getREG() && !getMEM() );
        return *this;
    }
    /** call \c setREG(true) when the register value associated with this symbol is set or is changed,
     * and \c setREG(false) when the register is dissociated from this symbol. */
    SpillableBase& setREG(bool set=true) {
        assert( this->getActive() );
        locn = static_cast<Where>( set? locn|REG: locn&~REG );
        if(set && getMEM()) incStale();
        //cout<<" setREG--->"<<*this<<" "; cout.flush();
        assert( this->getREG() == set );
        return *this;
    }
    /** call \c setMEM(true) when reg-symbol value is spilled to memory,
     * and \c setMEM(false) when register is dissociated from this symbol.
     * When reg-value of symbol changes, call \c setREG(true),
     * which will \c incStale() if symbol in \c getMEM().
     * When Symbol no longer useful, \c setREG(false).setMEM(false).
     */
    SpillableBase& setMEM(bool set=true) {
        assert( this->getActive() );
        locn = static_cast<Where>(set? locn|MEM: locn&~MEM);
        //cout<<" setMEM-->"<<*this<<" "; cout.flush();
        assert( this->getMEM() == set );
        return *this;
    }
    /** when getMEM(), staleness can increase to signal memory holes
     * associated with symbols that might be better to re-use. */
    unsigned getStale() const       {return this->staleness;}
    SpillableBase& unStale()        { staleness = 0U;
        return *this; }
    SpillableBase& incStale()       { ++staleness;
        return *this; }
    //---------------------------------------------------------
    //unsigned const uid; // not a spillable requirement, just for test purposes
    unsigned const uid;       // unique id (handle used in Spill::Region::symId)
    //unsigned const scope;
    // ... see symScopeUid.hpp  symbol<-->scope relation maintained separately
  protected:
    int len;
    int align;
    //bool active;      // this is slightly different from locn!=NONE (can be in-scope and have no locn [yet])
    //ve::RegId id;   // this should not be required
    unsigned staleness;  // this is only used for Spill
    Where locn;
};

std::ostream& operator<<(std::ostream& os, SpillableBase const& x){
    os<<"Sym{"<<x.uid<<(x.getActive()?"+":"-")
        <<"l"<<x.len<<"a"<<x.align<<"s"<<x.getStale();
    // register changed without mem update --> "staleness"
    //if(x.getREG()) os<<"R";
    //if(x.wasREG()) os<<"~R";
    os<<(x.getREG()? "R":"~R");
    if(x.getMEM()) os<<(x.getStale()? "~M": "M");
    return os<<"}";
}

// vim: ts=4 sw=4 et cindent cino=^=l0,\:.5s,=-.5s,N-s,g.5s,h.5s,b1 cinkeys=0{,0},0),\:,0#,!^F,o,O,e,0=break
#endif // SPILLABLE_BASE_HPP
