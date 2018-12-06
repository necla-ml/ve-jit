#ifndef SCOPED_SPILLABLE_BASE_HPP
#define SCOPED_SPILLABLE_BASE_HPP
/** \file
 * Basic Scoped Spillable Symbol Info.
 * Compared with \ref spillable-base.hpp, we give up control of an \e active symbol concept,
 * since \ref symScopeUid.hpp has a SymbStates class that will assign symbol ids, scope ids,
 * and tell us about whether things are in active scope or not.
 *
 * It is a base class in the sense that it supports only the attributes required
 * for a register that can be spilled (i.e. \e bytes length and \e align).
 *
 * No name, No register type, No register id, nothing.
 */

#include "symScopeUid.hpp"

#include <iostream>
#include <cassert>
#include <typeinfo> // for debug

class ScopedSpillableBase; // active state and symbol id to be managed by scope::SymbStates
class RegSymbol;           // fwd decl of testing class (friend)

/** This simplifies \c SpillableBase by removing 'active' and 'uid' from
 * a spillable symbols domain.
 *
 *
 * In contrast to \c SpillableBase, for \c ScopedSpillableBase the uid is assigned by
 * \c scope::SymbStates, which constructs BASE classes that:
 *
 * - obtain their uid from ParSymbol::uid (better, symId()) so we don't have a 'uid' field
 * - have a ParSymbol::scope and
 * - have control over the \e active flag (so we don't confuse this orthogonal concept into our \c Locn enum)
 *
 * Other than that, most of the code for \c ScopedSpillableBase is identical to \c SpillableBase.
 * Oh. our constructor is private, because the scope manager, scope::SymbStates, gives us a
 * \c newsym function that creates new symbols (like \c ScopedSpillableBase symbols).
 *
 * \c ScopedSpillableBase is ultimately managed by \c scope::SymbStates.
 * \e active and \e uid state of the old \ref SpillableBase have move into
 * \c scope::ParSymbol, so that symbols are really \c scope::ParSymbol<ScopedSpillableBase>.
 * Symbols are created within \c scope::SymbStates<ScopedSpillableBase> \c begin_scope
 * and \c end_scope.
 *
 * - \b only via \c newsym we get constructed.
 *   - initially active
 *   - after construction we can find out our uid,scope and query our active state.
 * - via \c delsym or \c end_scope, we are \e told about going out of scope.
 * - \ref symScopeUid.hpp retains all symbols by design, even stale, out-of-scope ones.
 *   But we probably don't care about extended symbol lifetime here.
 *
 * \sa SpillableBase for what a basic \e spillable symbol needs to do.
 *
 * Note: for now you \e must use ParSymbol<ScopedSpillableBase> exactly like that.
 * For more generality, this class could be templated on a ParSymbol<BASE> where
 * BASE is a type derived from SpillableBase.
 */
class ScopedSpillableBase {

    /** \group additions to SpillableBase
     * separation of concerns wrt symScopeUid.hpp */
    ///@{
  public:
    typedef scope::ParSymbol<ScopedSpillableBase> Ps; ///< parent Ps::uid and Ps::scope assigned "from above"
    friend Ps;
    friend RegSymbol; // for test access to setActive(bool)
    virtual ~ScopedSpillableBase() {}

    /** return the scope::ParSymbol, which has scope+uid, and maybe a SymbStates ptr.
     * For example our low-level symbol id is parent()->uid.
     */
    //Ps const *parent() const { return dynamic_cast<Ps const*>(this);}
    Ps const *parent() const {
        //std::cout<<"parent() type "<<typeid(Ps).name()<<std::endl; std::cout.flush();
        return dynamic_cast<Ps const*>(this);
    }
    typedef scope::SymbStates<scope::ParSymbol<ScopedSpillableBase>> Ss;
    friend Ss;
    friend class RegSymbol;
    friend class SpillableRegSym;
#if 0
    /** might return null */
    Ss const *symids() const {
        return parent()->ssym;}
#endif

    unsigned scope() const {return this->parent()->scope();}
    // symId(), below, also changes
    /** symbol unique id (now from scope::detail::ParSymbol parent) */
    unsigned symId() const          {return this->parent()->symId();}
  private:
    /** throw on duplicate symbol name in scame scope.
     * Cannot have 2 duplicate symbol names [with different uids] in same scope.
     * Note: compare our name only to \c ids that don't match our \c parent()->uid !
     * This function must be reimplemented in all derived classes.
     * (We don't even have a 'name' field here)
     */
    template<typename SymIdSet>
        void chk_different_name(SymIdSet const& ids) const {
            ;// no-op, we don't have a name so assume no conflict
        }

    ///@}
  protected:
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

  public:
    /** Are we in valid scope? */
    //bool getActive() const          {return (locn&ACTIVE);}
    bool getActive() const          {return parent()->getActive();}
    int getBytes() const            {return this->len;}
    int getAlign() const            {return this->align;}
    // spill-specific ...
    bool getREG() const             { return (locn&REG); }
    bool getMEM() const             { return (locn&MEM); }
    Where getWhere() const          {return this->locn;}

  protected:
    ScopedSpillableBase& setWhere(Where const w) {
        //assert( w & ~(ACTIVE|REG|MEM) == 0 );
        // perhaps allow derived classes to ADD flags
        this->locn = w;
        return *this;
    }

  private:
    /** We no longer control scope-active setting.
     * Therefore this is accessible only to \c Ps
     * (which via scope::SymbStates controls this call carefully)
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
        std::cout<<"Spillable::setActive("<<set<<") getActive="<<getActive()<<std::endl;
        setREG(false).setMEM(false);
        std::cout<<"Spillable::setActive("<<set<<") deact REG and MEM"<<std::endl;
        // next line is done by parent, before coming here:
        //locn = static_cast<Where>( set? locn|ACTIVE: locn&~ACTIVE );
        assert( !getREG() && !getMEM() );
        // !getActive() will happen real soon, in ParSymbol
        return *this;
    }
  public:
    /** call \c setREG(true) when the register value associated with this symbol is set or is changed,
     * and \c setREG(false) when the register is dissociated from this symbol. */
    ScopedSpillableBase& setREG(bool set=true) {
        //auto a = this->getActive();
        //std::cout<<" a="<<a<<std::endl; std::cout.flush();
        assert( this->getActive() );  // when we are going false, we are already deactivated in ParSymbol!
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
    //os<<"SpillableBae{"<<x.symId()<<(x.getActive()?"+":"-")
    //    <<"l"<<x.len<<"a"<<x.align<<"s"<<x.getStale();
    os<<"Spillable{l"<<x.len<<"a"<<x.align<<"s"<<x.getStale();
    os<<(x.getREG()? "R":"~R");
    if(x.getMEM()) os<<(x.getStale()? "~M": "M");
    return os<<"}";
}

// vim: ts=4 sw=4 et cindent cino=^=l0,\:.5s,=-.5s,N-s,g.5s,h.5s,b1 cinkeys=0{,0},0),\:,0#,!^F,o,O,e,0=break syntax=cpp.doxygen
#endif // SCOPED_SPILLABLE_BASE_HPP
