#ifndef SCOPED_SPILLABLE_BASE_HPP
#define SCOPED_SPILLABLE_BASE_HPP
/** \file
 * Basic Scoped Spillable Symbol Info.
 * Compared with \ref spillable-base.hpp, we give up control of an \e active symbol concept,
 * since \ref symScopeUid.hpp has a SymbStates class that will assign symbol ids, scope ids,
 * and tell us about whether things are in active scope or not.
 */

#include "symScopeUid.hpp"

#include <iostream>
#include <cassert>

class ScopedSpillableBase; // active state and symbol id to be managed by scope::SymbStates

/** This simplifies \c SpillableBase by removing 'active' and 'uid' from a spillable symbols domain.
 *
 * \sa SpillableBase for a description of what a basic \e spillable symbol does have.
 * Basically it has flags for Locn (none/register/memory).
 *
 * In contrast to \c SpillableBase, for \c ScopedSpillableBase the uid is assigned by
 * \c scope::SymbStates, which constructs BASE classes that:
 *
 * - obtain their uid from ParSymbol::uid (so we don't have a 'uid' field)
 * - have a ParSymbol::scope and
 * - have control over the \e active flag (so we don't confuse this orthogonal concept into our \c Locn enum)
 *
 * Other than that, most of the code for \c ScopedSpillableBase is identical to \c SpillableBase.
 * Oh. our constructor is private, because the scope manager, scope::SymbStates, gives us a
 * \c newsym function that creates new symbols (like \c ScopedSpillableBase symbols).
 *
 * \c ScopedSpillableBase is managed by scope::SymbStates.
 *
 * - \b only via \c newsym we get constructed.
 *   - initially active
 *   - after construction we can find out our uid,scope and query our active state.
 * - via \c delsym or \c end_scope, we are \e told about going out of scope.
 * - \ref symScopeUid.hpp retains all symbols by design, even stale, out-of-scope ones.
 *   But we probably don't care about extended symbol lifetime here.
 */
class ScopedSpillableBase {

    /** \group additions to SpillableBase
     * separation of concerns wrt symScopeUid.hpp */
    ///@{
  public:
    typedef scope::ParSymbol<ScopedSpillableBase> Ps; ///< parent Ps::uid and Ps::scope assigned "from above"
    typedef scope::SymbStates<ScopedSpillableBase> Ss;
    friend Ps;
    friend Ss;
    virtual ~ScopedSpillableBase() {}

    /** return the ParSymbol, which has scope+uid, and maybe a SymbStates ptr.
     * For example our low-level symbol id is parent()->uid.
     */
    Ps const *parent() const { return dynamic_cast<Ps const*>(this);}
    /** might return null */
    Ss const *symids() const { return (dynamic_cast<Ps const*>(this))->ssym;}

    unsigned scope() const {return this->parent()->scope;}
    // symId(), below, also changes
  private:
    /** throw on duplicate symbol name in scame scope.
     * Cannot have 2 duplicate symbol names [with different uids] in same scope.
     * Note: compare our name only to \c ids that don't match our \c parent()->uid !
     * This function must be reimplemented in all derived classes.
     * (We don't even have a 'name' field here)
     */
    template<typename SymIdSet>
        void chk_different_name(SymIdSet const& ids) const;

    ///@}

    /**  symbols constructed "\c active" ~ (in some valid scope).
     * - symbol \em location (\c Where) flags:
     *   - \c getActive(), \c inREG(), \c inMEM()
     * - allocation to a register or to a memory address <em>not here</em>.
     * - setting register value should call \c setREG(true)
     * - setting memory value should call \c setMEM(true)
     * - register value change ==> memory staleness up (if in memory)
     */
    ScopedSpillableBase(int const bytes, int const align)
        : len(bytes), align(align), staleness(0U), locn(NONE) {}

    friend std::ostream& operator<<(std::ostream& os, ScopedSpillableBase const& x);
    //template<class SYMBSTATES> friend class ve::Spill;

    /** The \e where concept is more detailed than the ParSymbol boolean \e active
     * flag. Do we mirror the parent 'active' here or not?  Let's remove scope-active
     * from ScopedSpillableBase.
     *
     * Here, ACTIVE is orthogonal to NONE/REG/MEM and has move up into ParSymbol.
     */
    typedef enum : int { NONE=0, /*ACTIVE=1,*/ REG=2, MEM=4
        /*, ARGREG=8, ARGMEM=16*/
    } Where;
    Where getWhere() const          {return this->locn;}
    ScopedSpillableBase& setWhere(Where const w) {
        //assert( w & ~(ACTIVE|REG|MEM) == 0 );
        // perhaps allow derived classes to ADD flags
        this->locn = w;
        return *this;
    }

    /** symbol unique id (now from scope::detail::ParSymbol parent) */
    unsigned symId() const          {return this->parent()->uid;}
    /** Are we in valid scope? */
    //bool getActive() const          {return (locn&ACTIVE);}
    bool getActive() const          {return parent()->getActive();}
    int getBytes() const            {return this->len;}
    int getAlign() const            {return this->align;}
    // spill-specific ...
    bool getREG() const             { return (locn&REG); }
    bool getMEM() const             { return (locn&MEM); }
  private:
    /** We no longer control scope-active setting.
     *
     * When exiting scope, symbols should \c setActive(false) this is done
     * during \c SymbStates \c end_scope() or \c delsym(symId)
     *
     * If you need to retain register content across blocks inside a loop,
     * move the symbol scope upward in scope!
     *
     * Whenever a symbol goes <B>in or out</B> of scope,
     *   we \c setREG(false) and \c setMEM(false).   (Content is unknown)
     */
    ScopedSpillableBase& setActive(bool set=true) {
        setREG(false).setMEM(false);
        // next line is done by parent, before coming here:
        //locn = static_cast<Where>( set? locn|ACTIVE: locn&~ACTIVE );
        assert( !getActive() && !getREG() && !getMEM() );
        return *this;
    }
  public:
    /** call \c setREG(true) when the register value associated with this symbol is set or is changed,
     * and \c setREG(false) when the register is dissociated from this symbol. */
    ScopedSpillableBase& setREG(bool set=true) {
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
     *
     * Here, no Symbol is useful out of its \c SymbStates scope.  Its spill
     * memory is completely free.  Its register is fully reassignable.
     */
    ScopedSpillableBase& setMEM(bool set=true) {
        assert( this->getActive() );
        locn = static_cast<Where>(set? locn|MEM: locn&~MEM);
        //cout<<" setMEM-->"<<*this<<" "; cout.flush();
        assert( this->getMEM() == set );
        return *this;
    }
    /** when getMEM(), staleness can increase to signal memory holes
     * associated with symbols that might be better to re-use. */
    unsigned getStale() const       {return this->staleness;}
    ScopedSpillableBase& unStale()        { staleness = 0U;
        return *this; }
    ScopedSpillableBase& incStale()       { ++staleness;
        return *this; }
    //---------------------------------------------------------
  protected:
    int len;
    int align;
    unsigned staleness;  // this is only used for Spill (fancier abilities)
    Where locn;
};

std::ostream& operator<<(std::ostream& os, ScopedSpillableBase const& x){
    os<<"Sym{"<<x.symId()<<(x.getActive()?"+":"-")
        <<"l"<<x.len<<"a"<<x.align<<"s"<<x.getStale();
    // register changed without mem update --> "staleness"
    //if(x.getREG()) os<<"R";
    //if(x.wasREG()) os<<"~R";
    os<<(x.getREG()? "R":"~R");
    if(x.getMEM()) os<<(x.getStale()? "~M": "M");
    return os<<"}";
}

// vim: ts=4 sw=4 et cindent cino=^=l0,\:.5s,=-.5s,N-s,g.5s,h.5s,b1 cinkeys=0{,0},0),\:,0#,!^F,o,O,e,0=break syntax=cpp.doxygen
#endif // SCOPED_SPILLABLE_BASE_HPP