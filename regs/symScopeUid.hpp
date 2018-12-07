#ifndef SYMSCOPEUID_HPP
#define SYMSCOPEUID_HPP
/** \file
 * Header-only Scoped-Symbol utility classes.
 *
 * TestBed : 1 basic data structure, and 2 approaches to add client-state
 *           data to symbols.
 *
 * Should eventually cooperate with regSymbol.hpp that has core symbol state,
 * and a first stab at registers.
 *
 * \todo Register state should eventually be based on reg-aurora.hpp + reg-base.hpp
 * w/ mkChipRegisters to set up basic register state,
 */

#include "throw.hpp"
#include <unordered_set>
#include <unordered_map>
#include <map>
#include <forward_list>
#include <cassert>
#include <iostream>
#include <cstring>
#include <vector>
#include <algorithm> // sort
#include <typeinfo> // debug!

//fwd decl for friendship only
class Regset;
//template<class SYMBSTATES> class Regset;

// fwd decls for friend status w/ ParSymbol or SymbStates
class DemoSymbStates; // for development & test progs
namespace ve {
template<class SYMBSTATES> class Spill;
}

namespace scope {

// todo use this typedef in this file!
//typedef unsigned SymId;
// Perhaps eventually, like RegId
//enum SymId : int_least16_t {};

// version 3: simple user-supplied BASE for symbol state
/** \c ParSymbol supplies the "state" functions required by \c SymbStates.
 * - const uid  (symId)
 * - const scope
 * - active flag, getActive(), goes false via end_scope or delsym.
 */
template<class BASE> class ParSymbol;

template<class BASE>
std::ostream& operator<<(std::ostream& os, ParSymbol<BASE> const& x);

/** Main scope function.  This is based on detail::SymScopeUid and ParSymbol.
 * - begin_scope, newsym, end_scope are the main functions. */
template<class BASE> class SymbStates;


namespace detail {

/** Data structure -- active/stale symbol/scope relationships */
class SymScopeUid;

// fwd decl some friends in symScopeUid-test.hpp:
class BaseSymbolState;
template<typename SYMBASE=BaseSymbolState> class SymStates; // fwd decl

}//detail::

namespace detail {
/** simple symbol scopes.
 *
 * Scopes and symbols are represented as unsigned uids.
 *
 * This is a low-level class, used within more useful objects,
 * \b Derived objects will add some sort of \em state tracking,
 * \e adding some sort of <TT>symbol id --> symbol state tracking.
 *
 * - Active scopes form a stack representing nested scopes.
 *   - Basic ops:
 *     - unsigned scopeId = \c begin_scope(),
 *     - \c end_scope() moves the scope to a stale list.
 *     - symbols in stale scopes have \c active(symId) return 0
 *     - scopes can be reactivated with \c activate_scope(scopeId)
 * - Scopes contain symbol uids.
 * - symbols are created in "current" active scope, and cannot
 *   change scope.
 *   - Basic ops:
 *     - unsigned sym = \c newsym();
 *     - \c scopeof(symId) always returns this scope.
 *     - \c active(symId) returns this [until scope stale]
 *
 * By maintaining stale scopes, symbols logically have an
 * active/inactive status.  If scopes come back active, all
 * their symbols become active, so the user can maintain
 * some persistent symbol state.  When using \c activate_scope(),
 * it is up to the user to maintain proper nestedness, by
 * reactivating innermost scopes first.
 *
 * While parent scope for active scopes is implicit in the stack structure
 * (forward_list), stale scopes \b forget any notion of parent scope.
 *
 */
class SymScopeUid {
  public:
    template<class SYMBASE> friend class SymStates;
    template<class BASE>    friend class scope::SymbStates;
    typedef std::unordered_set<unsigned> HashSet;
  protected:
    unsigned maxidSym;
    unsigned maxidSco;

    /** symbol uid --> scope uid */
    std::unordered_map<unsigned,unsigned> symScope;

    struct ScopeSymbols{
        unsigned scope;
        HashSet syms;
        explicit ScopeSymbols(unsigned scope)
            : scope(scope), syms()
        {}
    };
    /** stack of {scope,syms[]}, nested active symbol sets */
    std::forward_list<ScopeSymbols> scopes;
    std::unordered_set<unsigned> activeSco;

    std::forward_list<ScopeSymbols> stales;
    std::unordered_set<unsigned> staleSco;
  public:
    /** push a new symbol declaration scope */
    unsigned begin_scope() {
        ++maxidSco;
        scopes.emplace_front(maxidSco);
        activeSco.insert(maxidSco);
        assert( scopes.front().scope == maxidSco );
        return maxidSco;
    }
    /** pop symbol scope [move it to stale scopes] */
    void end_scope() {
        assert( !scopes.empty() );
        stales.emplace_front(scopes.front());
        scopes.pop_front();
        assert( !scopes.empty() );
        unsigned mv = stales.front().scope;
        activeSco.erase(mv);
        staleSco.insert(mv);
    }
    /** reactivate \c stale scope (throw if not a stale scope).
     * \note It is up to the user to activate scopes in proper
     * order -- stale scopes can be added back in any order. */
    void activate_scope( unsigned const stale );
    /** set up global scope with no active symbols */
    SymScopeUid()
        : maxidSym(0U), maxidSco(0U), symScope()
          , scopes(), activeSco()
          , stales(), staleSco()
    {
        begin_scope();
        assert( maxidSco == 1U );
        assert( this->scope() == 1U );
    }
#if 0 // erase out-of-scope symbols?
    void end_scope() {
        ScopeSymbols const& oldScopeSymbols = scopes.front();
        unsigned oldScope = oldScopeSymbols.scope;
        activeSco.erase(oldScope);
        for(unsigned sym : oldScopeSymbols.syms  ){
            auto const& found = activeSym.find(sym);
            assert( found != activeSym.end() );
            assert( found.second == oldScope );
            activeSym.erase(found);
        }
        scopes.pop_front();
    }
#endif
    /** return symbol scope (possibly stale), or zero if \c symId unknown */
    unsigned scopeOf(unsigned const symId) const {
        unsigned ret = 0;
        auto found = symScope.find(symId);
        if( found != symScope.end() ){
            ret = found->second;
            assert( ret != 0U );
        }
        return ret;
    }
    /** return active symbol scope, or zero if stale/unknown.
     * Note that this refers to scope-related 'active' only.
     * So ParSymbol can be told \c eraseSym(symId) and have
     * its active setting go false before the \c end_scope. */
    unsigned active(unsigned const symId) const {
        unsigned scope = scopeOf(symId);
        if(scope && activeSco.find(scope) == activeSco.end() ){
            scope = 0U;
        }
        return scope;
    }
    /** define a new symbol in current scope, without creating any symbol object */
    unsigned newsym() {
        ++maxidSym;
        assert( maxidSym != 0U );
        assert( !scopes.empty() );
        ScopeSymbols& curScopeSymbols = scopes.front();
        unsigned scope = curScopeSymbols.scope;
        HashSet& syms = curScopeSymbols.syms;
        assert( syms.find(maxidSym) == syms.end() );
        syms.insert(maxidSym);
        symScope[maxidSym] = scope;
        return maxidSym;
    }
    /** delete sym uid in current scope (o/w throw).
     * [alt.] wait for \c end_scope() to auto-delete. */
    void delsym(unsigned const symId);
    /** return current scope uid */
    unsigned scope() const {
        assert( ! scopes.empty() );
        return scopes.front().scope;
    }
    /** return number of symbols defined in current (innermost) scope, active or not. */
    size_t nScopeSymbols() const{
        assert( !scopes.empty() );
        return scopes.front().syms.size();
    }
    void prtCurrentSymbols() const;
    /** sorted symIds of all symbols in active scopes */
    std::vector<unsigned> symIdAnyScopeSorted() const;
    std::vector<unsigned> symIdStaleSorted() const;
};

}//detail::

//===================================================================================//
/** ParSymbol provides uid+scope and an active flag (\e only).
 * Other details are handled by the \tparm BASE class, that gets passed an
 * arg pack.  Ex. BASE=ScopedSpillableBase.
 */
template<class BASE>
class ParSymbol : public BASE {
  public:
    typedef BASE Base;
    typedef ParSymbol<BASE> Psym;
    friend BASE;
    friend SymbStates<ParSymbol<BASE>>;
    friend class ::DemoSymbStates; // for development & test progs
    template<class SYMSTATE> friend class detail::SymStates; // for testSymScopeUid.cpp

    virtual ~ParSymbol() {}
    BASE const& base() const {return *this;} // just for convenience
    BASE      & base()       {return *this;}
    bool getActive() const {return this->active;}
    unsigned symId() const {return this->uid;}
    unsigned scope() const {return this->scope_;}
    unsigned const uid;
    unsigned const scope_;
  protected:
    /** \c ParSymbol grabs uid and scope, forwarding \e unrecognized args
     * to the BASE constructor.  Base constructor has \b no access
     * to uid or scope, since we are not yet fully constructed. */
    template<typename... Arg>
        ParSymbol(unsigned uid, unsigned scope, Arg&&... arg);

    void setActive(bool const active) {
        //std::cout<<"ParSymbol::setActive("<<active<<")  "; std::cout.flush();
        if(active) this->active = active;
        BASE::setActive(active);        // order important, so BASE can check more invariants!
        if(!active) this->active = active;
    }
    bool active;
};

/** Main public interface for scoped symbols.
 *
 * \tparm BASE must provide a function
 * ```
 *   template<typename SymIdSet> void chk_different_name(SymIdSet const& ids) const;
 * ```
 * that throws if some duplicate symbol id (as per ParSymbol<BASE>::uid) is to be disallowed.
 * (It can be empty if you don't care).
 *
 * Other than that, it can hold whatever: Ex. symbol name, creation tick, usage ticks, register
 * assignment, memory location, ...  We will pass all "extra" constructor args in \c newsym()
 * down to your BASE class, without interpreting them.
 *
 * If the symbolId order is important [some tests], you can use ordered=1.
 * Perhaps we can support different orders, like order-by staleness?
 */
//template<class BASE/*=ExBaseSym*/> //, bool ordered/*=0*/ >
template<class PARSYMBOL>
class SymbStates{
  public:
    //typedef ParSymbol<BASE> Psym;
    //typedef SymbStates<BASE> Ssym;
    typedef PARSYMBOL Psym;
    typedef SymbStates<PARSYMBOL> Ssym;
    typedef typename PARSYMBOL::Base Base;
    typedef std::unordered_map<unsigned/*symId*/,Psym> SymIdMap;

    //friend Base;
    //friend ve::Spill<SymbStates<Base>>;
    friend ::ve::Spill<Ssym>;
    //template<class SYMBSTATES> friend class ::Regset;
    friend class ::Regset;
  protected:
    detail::SymScopeUid ssu;
    /** symbol uid --> external symbol state.  This data structure \em owns all the
     * Psym objects. */
    SymIdMap syms;
  public:
    //
    //  TODO expose getting list of active scopes from ssu.scopes (by checking ssu.activeSco[scope])
    //       expose getting list of syms in a scope (from ssu.activeSco)
    //       etc as needed.
    unsigned begin_scope() { return ssu.begin_scope(); }
    void end_scope() {
        assert( !ssu.scopes.empty() );
        for(auto symId: ssu.scopes.front().syms){ // from current scope
            auto const& psym = syms.find(symId);
            assert( psym != syms.end() );
            //assert(psym.getActive() == true); // you can delsym(symId)
            std::cout<<" end"<<symId<<" "<<psym->second<<std::endl; std::cout.flush();
            psym->second.setActive(false);
        }
        // next line last to retain more invariants
        ssu.end_scope(); // NOW [after deactivating] move ssu current scope to ssu.stales
    }

    /** args are passed verbatim down to PARSYMBOL */
    template<typename... Arg> unsigned newsym(Arg&&... arg);

    /** premature deactivation of any active symbol (from any scope).
     * Expected use is to free a register as early as possible. */
    void delsym(unsigned const symId){
        auto const psym = syms.find(symId);
        assert( psym != syms.end() );
        assert( psym->second.getActive() ); // fast check for 'active'
        assert( this->active(symId) ); // same, but adds a small consistency check
        //assert( ssu.active(symId) ); // if it must be active in *innermost* scope
        psym->second.setActive(false);
    }

    /** current active scope number */
    unsigned scope() const { return ssu.scope(); }
    /** return number of symbols defined in current (innermost) scope, active or not. */
    size_t nScopeSymbols() { return ssu.nScopeSymbols(); }
    /** since beginning of time. \c delsym and \c end_scope deactivate, but don't remove. */
    size_t nSymbols() { return syms.size(); }
    void prtCurrentSymbols(int verbose=0) const;
    /** global sets of active/stale symbols */
    std::vector<unsigned> symIdAnyScopeSorted() const {return ssu.symIdAnyScopeSorted();}
    std::vector<unsigned> symIdStaleSorted() const {return ssu.symIdStaleSorted();}

    /** tricky behavior, try to stick with begin/end_scope */
    void activate_scope(unsigned const stale) {
        ssu.activate_scope(stale);
        assert( !ssu.scopes.empty() );
        for(auto const symId: ssu.scopes.front().syms ){
            auto const psym = syms.find(symId);
            assert( psym != syms.end() );
            assert( psym->second.getActive() == false);
            psym->second.setActive(true);
        }
    }

    /** return \b scope of \e any \c symId, or 0 if the symbol has never been created.
     * \c scopeOf doesn't mind if the scope is stale, or if the symbol is inactive. */
    unsigned scopeOf(unsigned const symId) const {
        unsigned ret = 0U;
        auto const psym = syms.find(symId);
        if( psym != syms.end() ){
            ret = psym->second.scope();
        }
        assert( ret == ssu.scopeOf(symId) );
        return ret;
    }
    /** return \b scope of an active symbol
     * (zero if symbol is stale or scope is inactive). */
    unsigned active(unsigned const symId) const {
        unsigned ret = 0U;
        auto psym = syms.find(symId);
        if( psym != syms.end() && psym->second.getActive() ){
            ret = psym->second.scope();
            //std::cout<<" syms.find("<<symId<<")->second.scope() = "<<syms.find(symId)->second.scope();
        }
        //std::cout<<"active:"<<ret<<" ?= "<<ssu.active(symId)<<std::endl; std::cout.flush();
        //if( ret != ssu.active(symId) ){
        //    THROW("active:"<<ret<<"!="<<ssu.active(symId));
        //}
        assert( ret == ssu.active(symId) );
        return ret;
    }
    /** Look up symId, returning ParSymbol<BASE> */
    Psym const& psym(unsigned const symId) const {
        auto const psym = syms.find(symId);
        if( psym == syms.end() ){
            THROW( "psym("<<symId<<") doesn't exist!");
        }
        assert( psym != syms.end() );
        return psym->second;
    }
#if 0
    detail::SymScopeUid::HashSet const& scopeSymUids() const {
        assert( !ssu.scopes.empty() );
        return ssu.scopes.front().syms;
    }
#endif
#if 0 // for internal use, cause bugs when used from within newsym (inconsistent state)
    // but should be ok for client use.
    std::vector<Psym const*> psymsActiveIn( unsigned scop ){
        std::vector<Psym const*> scop_psyms;
        // find it in active-scope list
        std::cout<<" A "; std::cout.flush();
        auto const ssIter = std::find_if( ssu.scopes.begin(), ssu.scopes.end(),
                [scop](detail::SymScopeUid::ScopeSymbols const& ss)
                { return ss.scope == scop; } );
        if (ssIter != ssu.scopes.end()){
            std::cout<<" B "; std::cout.flush();
            //scop_psyms.reserve(ssIter->syms.size());
            detail::SymScopeUid::HashSet const& hs = ssIter->syms;
            //std::transform( ssIter->syms.begin(), ssIter->syms.end(),
            std::cout<<" BB "; std::cout.flush();
            for(auto const s: hs){
                std::cout<<"\n\ts="<<s<<std::endl; std::cout.flush();
                Psym const& p = this->psym(s);
                scop_psyms.push_back(&p);
            }
            //std::transform( hs.begin(), hs.end(),
            //        std::back_inserter(scop_psyms),
            //        [this](unsigned symId)->Psym const*
            //        {std::cout<<" "<<symId<<":"<<this->psym(symId)<<"\n"; std::cout.flush();
            //        return &this->psym(symId);} );
            for(auto p: scop_psyms) std::cout<<"  p:"<<*p; std::cout.flush();
            std::cout<<" C "; std::cout.flush();
        }
        return scop_psyms;
    }
#endif
  protected:
    Psym      & psym(unsigned const symId)       {
        auto const psym = syms.find(symId);
        if( psym == syms.end() ){
            THROW( "psym("<<symId<<") doesn't exist!");
        }
        assert( psym != syms.end() );
        return psym->second;
    }
    /** "friend psym", to help compiler know the non-const psym is really needed :( */
    Psym      & fpsym(unsigned const symId)      {
        auto const psym = syms.find(symId);
        if( psym == syms.end() ){
            THROW( "fpsym("<<symId<<") doesn't exist!");
        }
        assert( psym != syms.end() );
        return psym->second;
    }
};//class SymbStates

inline void detail::SymScopeUid::activate_scope( unsigned const stale ){
    // oh, need a prev iter...
    auto prev = stales.before_begin();
    auto found = prev;
    for( ; ++found != stales.end(); prev=found){
        if(found->scope == stale) break;
    }
    if( found == stales.end() ){
        // XXX make this a fn call
        THROW("cannot activate non-stale scope"<<stale);
    }
    scopes.emplace_front(*found);
    stales.erase_after(prev);
    unsigned mv = scopes.front().scope;
    activeSco.insert(mv);
    staleSco.erase(mv);
}
inline void detail::SymScopeUid::delsym(unsigned const symId){
    ScopeSymbols& curScopeSymbols = scopes.front();
    unsigned scope = curScopeSymbols.scope;
    HashSet& syms = curScopeSymbols.syms;
    auto const iter = syms.find(symId);
    if( iter == syms.end() ){
        THROW("delsym("<<symId<<") of symbol not in current scope="<<scope);
    }
    syms.erase(iter);
    symScope.erase(symId);
}
inline void detail::SymScopeUid::prtCurrentSymbols() const{
    ScopeSymbols const& curScopeSymbols = scopes.front();
    unsigned scope = curScopeSymbols.scope;
    HashSet const& curSyms = curScopeSymbols.syms;
    std::cout<<" CurrentScope"<<scope;
    char const* sep = "{";
    for(auto const sym : curSyms ){
        std::cout<<sep<<sym;
        sep = ",";
    }
    std::cout<<"}";
}
inline std::vector<unsigned> detail::SymScopeUid::symIdAnyScopeSorted() const {
    size_t nSym = 0U;
    size_t nSco = 0U;
    for(auto const& sco: scopes){
        ++nSco;
        nSym += sco.syms.size();
    }
    std::cout<<" count "<<nSym<<" syms in "<<nSco<<" active scopes"<<std::endl;
    std::vector<unsigned> ret;
    ret.reserve(nSym);
    for(auto const& sco: scopes){
        for(unsigned sym: sco.syms){
            ret.push_back(sym);
        }
    }
    std::sort(ret.begin(), ret.end());
    return ret;
}
inline std::vector<unsigned> detail::SymScopeUid::symIdStaleSorted() const {
    size_t nSym = 0U;
    size_t nSta = 0U;
    for(auto const& sta: stales){
        ++nSta;
        nSym += sta.syms.size();
    }
    std::cout<<" count "<<nSym<<" syms in "<<nSta<<" stale scopes"<<std::endl;
    std::vector<unsigned> ret;
    ret.reserve(nSym);
    for(auto const& sta: stales){
        for(unsigned sym: sta.syms){
            ret.push_back(sym);
        }
    }
    std::sort(ret.begin(), ret.end());
    return ret;
}

template<class BASE>
    template<typename... Arg> inline
ParSymbol<BASE>::ParSymbol(unsigned uid, unsigned scope, Arg&&... arg)
    : BASE(std::forward<Arg>(arg)...)
      , uid(uid), scope_(scope), active(true)
{
    int const verbose=1;
    if(verbose){
        std::cout<<" +Par(id"<<uid<<",sc"<<scope<<")";
//#if !defined(__ve) // nc++ had trouble finding the appropriate operator, sometimes
        Base const& b = *this;
        std::cout<<"'"<<(decltype(b))b<<"'";
//#endif
        std::cout.flush();
    }
}

template<class BASE> inline 
std::ostream& operator<<(std::ostream& os, ParSymbol<BASE> const& x){
    return os
        <<"ParSymbol"
        <<(x.getActive()?'+':'-')
        <<x.symId()
        <<"s"<<x.scope()
        <<"<"<<x.base()<<">}";
}

template<class PARSYMBOL> 
template<typename... Arg> inline
unsigned SymbStates<PARSYMBOL>::newsym(Arg&&... arg) {
    // get a new symbol id
    assert( !ssu.scopes.empty() );
    unsigned const symId = ssu.newsym(); // symId is already push onto the ssu !!
    assert( ssu.active(symId) );
    assert( ssu.active(symId) == this->scope() );
    std::cout<<"u"<<symId; std::cout.flush();
    unsigned const scope = this->scope();
    std::cout<<"s"<<scope; std::cout.flush();
    assert( syms.find(symId) == syms.end() );

    auto nameclash = [this](Psym const& ps, detail::SymScopeUid::HashSet const& uids)->void {
        std::vector<Psym const*> uid_psyms;
        uid_psyms.reserve(uids.size());
        for(auto u: uids) {
            if (u!=ps.symId()){
                //std::cout<<" nameclash checking "<<this->psym(u)<<std::endl; std::cout.flush();
                uid_psyms.push_back(&(this->psym(u)));
            }
        }
        ps.chk_different_name(uid_psyms);
    };

    // Psym (or its BASE) may place restrictions on detecting illegal dups in
    // same scope.  (No need to extend to clashes in upper?)
    // ERROR: if symbol hides one in current scope
    try{
        // construct ParSymbol symbol+scope (and user's BASE state)
        Psym psym{symId,scope,arg...};
        //std::cout<<" newsym-->"<<psym<<std::endl;
        nameclash(psym, ssu.scopes.front().syms);
        // finalize the symbol
        syms.emplace( std::make_pair(symId, std::move(psym)) ); // ???
    }
    catch(...){
        std::cout<<" Error creating newsym at "<<__FILE__<<":"<<__LINE__<<std::endl;
        throw;
        // or else could ssu.delsym and try to carry on ???
    }
    assert( syms.find(symId) != syms.end() );
    assert( syms.find(symId)->second.symId() == symId );
    assert( syms.find(symId)->second.scope() == scope );
    assert( syms.find(symId)->second.getActive() == true );
    //std::cout<<"+"; std::cout.flush();
    assert( ssu.active(symId) == scope );
    assert( this->active(symId) == scope );
    //std::cout<<"+"; std::cout.flush();

    // WARN: if symbol hides one in enclosing scope
    bool doit=false;      // current scope we already checked -- dups were an ERROR
    Psym const& oursym = psym(symId);
    for(auto const& sc: ssu.scopes){
        if(doit){ // enclosing scopes...
            try{
                nameclash(oursym, sc.syms);
            }catch(...){
                // nameclash with syms in higher scope is non-fatal
                std::cout<<" Warning: symbol id"<<symId<<" in scope "<<ssu.scope()
                    <<" hides one of same name in enclosing scope "<<sc.scope<<std::endl;
            }
        }
        doit=true;
    }
    return symId;
}
template<class PARSYMBOL> inline
void SymbStates<PARSYMBOL>::prtCurrentSymbols(int verbose/*=0*/) const {
    std::cout<<" CurrentScope"; std::cout.flush();
    assert( !ssu.scopes.empty() );
    detail::SymScopeUid::ScopeSymbols const& curScopeSymbols = ssu.scopes.front();
    unsigned scope = curScopeSymbols.scope;
    std::cout<<scope; std::cout.flush();
    detail::SymScopeUid::HashSet const& curSyms = curScopeSymbols.syms;
    char const* sep = "{";
    if(curSyms.empty()) std::cout<<sep;
    else for(auto const sym : curSyms ){
        std::cout<<sep<<sym; std::cout.flush();
        // is it active right now?
        auto found = syms.find(sym);
        assert( found != syms.end() );
        assert( found->second.symId() == sym );
        //assert( syms[sym].active == true );
        assert( found->second.scope() == scope );
        if( verbose<=0 && !found->second.getActive() ) std::cout<<"!";
        //if(verbose>0) {std::cout<<found->second; std::cout.flush();}
        //if(verbose>0) {std::cout<<"="<<found->second.base(); std::cout.flush();}
        if(verbose>0) {std::cout<<"="<<found->second; std::cout.flush();}
        sep = ", ";
    }
    std::cout<<"}";
    std::cout.flush();
}

}//scope::
// vim: ts=4 sw=4 et cindent cino=^=l0,\:.5s,=-.5s,N-s,g.5s,h.5s,b1 cinkeys=0{,0},0),\:,0#,!^F,o,O,e,0=break syntax=cpp
#endif // SYMSCOPEUID_HPP
