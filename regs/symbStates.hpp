#ifndef SYMBSTATES_HPP
#define SYMBSTATES_HPP
/** \file
 * Header-only Scoped-Symbol utility classes DEMO.
 *
 * \deprecated split into spillable-base, spillable, regSymbol headers
 *
 * TestBed : 1 basic data structure, and a few approaches to add client-state
 *           data to symbols.
 *
 * 1. basic, Uid-only \c SymScopeUid
 * 2. simple \c SymStates class, w/ void* state in \c BaseSymbolState
 * 3. CRTP (not done)
 * 4. User-supplied base class for state customization, standard \em symbol features
 *    going into a \c ParSymbol parent class.
 *
 * - Crucial goal is to segregate uid and scope handling from othe complications, esp.
 *   - evolving register assignments,
 *   - register allocation
 *   - register spilling
 *   - stack memory allocations:
 *     - function arguments (upon function entry)
 *     - local variables (when spilled)
 */

#include "throw.hpp"
#include "regDefs.hpp"          //enum Reg_t:int{}
#include <unordered_set>
#include <unordered_map>
#include <forward_list>
#include <cassert>
#include <iostream>
#include <cstring>

// Simple symbols and scopes, storing uids only (no symbol or scope state)
class SymScopeUid;

// Useful symbol and scope support, customizable BASE to add symbol state.
class ExBaseSym;
template<class BASE=ExBaseSym> class ParSymbol;
template<class BASE=ExBaseSym> class SymbStates;

// Some friend classes are forward-declared
namespace ve{
template<class SYMBSTATES> class Spill;
}

/** simple symbol scopes.
 *
 * Scopes and symbols are represented as uids.
 *
 * We do not maintain any symbol state, so in practice
 * you may also want a mapping of symbol id --> symbol object
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
 *   - ?? hmmm...
 *     - SYMBASE [default is \c BaseSymbolState] is returned to user,
 *       where more state can be added in a derived class.
 *     - SYMBASE should have const uid and scope data, as well
 *       as constructors and get/setActive, like \c BaseSymbolState.
 *
 * By maintaining stale scopes, symbols logically have an
 * active/inactive status.  If scopes come back active, all
 * their symbols become active, so the user can maintain
 * some persistent symbol state.  When using \c activate_scope(),
 * it is up to the user to maintain proper nestedness, by
 * reactivating innermost scopes first.
 *
 */
class SymScopeUid {
  public:
      template<class SYMBASE> friend class SymStates;
      template<class BASE> friend class SymbStates;
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
    void activate_scope( unsigned const stale ){
#if 0
        auto found = find_if(
                staleScopes.begin(), staleScopes.end(),
                [&stale](const ScopeSymbols& ss)
                { return ss.scope == stale; });
#else // ohoh, need a prev iter...
        auto prev = stales.before_begin();
        auto found = prev;
        for( ; ++found != stales.end(); prev=found){
            if(found->scope == stale) break;
        }
#endif
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
    /** set up global scope with no active symbols */
    SymScopeUid()
        : maxidSym(0U), maxidSco(0U), symScope()
          , scopes(), activeSco()
          , stales(), staleSco()
    {
        begin_scope();
        assert( maxidSco = 1U );
        assert( this->scope() == 1U );
    }
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
    /** return active symbol scope, or zero if stale/unknown */
    unsigned active(unsigned const symId) const {
        //int const verbose = 0;
        unsigned scope = scopeOf(symId);
        //if(verbose) std::cout<<"scope:"<<scope;
        if(scope && activeSco.find(scope) == activeSco.end() ){
            //if(verbose) std::cout<<"oops";
            scope = 0U;
        }
        //if(verbose) std::cout.flush();
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
    void delsym(unsigned const symId){
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
    /** return current scope uid */
    unsigned scope() const {
        assert( ! scopes.empty() );
        return scopes.front().scope;
    }
    size_t nScopeSymbols() const{
        assert( !scopes.empty() );
        return scopes.front().syms.size();
    }
    void prtCurrentSymbols() const{
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
};//class SymScopeUid

/** Standard symbol features.
 * - uid, scope, getActive()
 * - customizable symbol state as BASE class
 * - backpointer to SymbStates<BASE>
 *
 * ParSymbol<BASE> objects are wholly owned by SymbStates<BASE>, and uniquely
 * identified with a symbol \c uid [unique identifier].  There is also a
 * \c scope uid. \c SymbStates maintains a stack of nested scopes that bring
 * symbols in/out of active state.
 *
 * When \em dereferencing symbols, we should be able to auto-check that all
 * operations are occurring upon in-scope symbols.  (Something like the
 * original SafeSym idea in asm.hpp).
 */
template<class BASE>
class ParSymbol : public BASE {
  public:
      friend BASE;
      friend SymbStates<BASE>;
      template<class SYMBSTATES> friend class ::ve::Spill; // not working?
      //template<class SYMBSTATES> friend void Spill<SYMBSTATES>::spill(unsigned const, int); // not working?
      //friend class ve::Spill<SymbStates<BASE>>;

      typedef SymbStates<BASE> Ssym;
      typedef ParSymbol<BASE> Psym;
      virtual ~ParSymbol() {}
      template<typename... Arg>
          ParSymbol(Ssym *ssym, unsigned uid, unsigned scope, Arg&&... arg);

      bool getActive() const {return this->active;}
      unsigned const uid;
      unsigned const scope;
      /** cast to DERIVED */
      //void *state;              // arbitrary client state
  protected:
  //private:
      void setActive(bool const active)         {this->active = active;}
      SymbStates<BASE> *ssym;
      bool active;
      // ------------------ all vars below are "extras", here just for experiment ----------
      // reqd for Spill: int staleness, RegId id, mem, SPILL, align, len
      ve::RegId id;
      int staleness;
      typedef enum { NONE=0, ARG=1, SPILL=2 } Where;
      Where mem;                  ///< if not reg, may have original/alt location in memory.
      int align;                  ///< default align = 8
      int len;                    ///< len(bytes) defaults to max (VECTOR:MVL*8, VMASK:4*8, VMASK2:8*8)
};

template<class BASE>
    template<typename... Arg>
inline ParSymbol<BASE>::ParSymbol(Ssym *ssym, unsigned uid, unsigned scope, Arg&&... arg)
    : BASE(/*this,*/ std::forward<Arg>(arg)...)
      , uid(uid), scope(scope), ssym(ssym), active(true)
{
    int const verbose=1;
    if(verbose){
        std::cout<<" Par(id"<<uid<<",sc"<<scope<<")" // <<ssym@"<<(void*)ssym
            <<"'"<<BASE::name<<"'"; std::cout.flush();
    }
}

template<class BASE/*=ExBaseSym*/>
class SymbStates{
  public:
      typedef SymbStates<BASE> Ssym;
      typedef ParSymbol<BASE> Psym;
      typedef BASE Base;
      friend BASE;
  private:
      SymScopeUid ssu;
      /** symbol uid --> external symbol state.  This data structure \em owns all the
       * Psym objects. */
      std::unordered_map<unsigned/*symId*/,Psym> syms;
  public:
      unsigned begin_scope() { return ssu.begin_scope(); }
      void end_scope() {
          ssu.end_scope();
          for(auto symId: ssu.stales.front().syms){
              //assert(syms[symId].getActive() == true); // maybe can deactivate syms one-by-one??
              auto const psym = syms.find(symId);
              assert( psym != syms.end() );
              psym->second.setActive(false);
          }
      }
      void activate_scope(unsigned const stale) {
          ssu.activate_scope(stale);
          for(auto const symId: ssu.scopes.front().syms ){
              auto const psym = syms.find(symId);
              assert( psym != syms.end() );
              assert( psym->second.getActive() == false);
              psym->second.setActive(true);
          }
      }
      unsigned scopeOf(unsigned const symId) const {
          unsigned ret = 0U;
          auto const psym = syms.find(symId);
          if( psym != syms.end() ){
              ret = psym->second.scope;
          }
          assert( ret == ssu.scopeOf(symId) );
          return ret;
      }
      /** return \b scope of \c symId.
       * Note: diff't from bool \c scopeOf(symId).getActive()
       */
      unsigned active(unsigned const symId) const {
          unsigned ret = 0U;
          auto psym = syms.find(symId);
          if( psym != syms.end() && psym->second.getActive() ){
              ret = psym->second.scope;
              //std::cout<<" syms.find("<<symId<<")->second.scope = "<<syms.find(symId)->second.scope;
          }
          //std::cout<<"active:"<<ret<<" ?= "<<ssu.active(symId)<<std::endl; std::cout.flush();
          //if( ret != ssu.active(symId) ){
          //    THROW("active:"<<ret<<"!="<<ssu.active(symId));
          //}
          assert( ret == ssu.active(symId) );
          return ret;
      }
      //SymScopeUid::HashSet const& scopeSymUids() const {
      //    assert( !ssu.scopes.empty() );
      //    return ssu.scopes.front().syms;
      //}
      Psym const& psym(unsigned const symId) const {
          auto const psym = syms.find(symId);
          if( psym == syms.end() ){
              THROW( "psym("<<symId<<") doesn't exist!");
          }
          assert( psym != syms.end() );
          return psym->second;
      }
      Psym      & psym(unsigned const symId)       {
          auto       psym = syms.find(symId);
          if( psym == syms.end() ){
              THROW( "psym("<<symId<<") doesn't exist!");
          }
          assert( psym != syms.end() );
          return psym->second;
      }
      template<typename... Arg>
      unsigned newsym(Arg&&... arg) {
          unsigned const symId = ssu.newsym();
          assert( ssu.active(symId) );
          assert( ssu.active(symId) == this->scope() );
          std::cout<<"u"<<symId; std::cout.flush();
          unsigned const scope = this->scope();
          std::cout<<"s"<<scope; std::cout.flush();
          assert( syms.find(symId) == syms.end() );
          //syms.emplace( symId, Psym{symId,scope,arg}... ); // ???
          try{
              //std::cout<<" create Psym..."<<std::endl;
              Psym psym{this,symId,scope,arg...};
              //std::cout<<" chk_different_name..."<<std::endl;
              psym.chk_different_name(ssu.scopes.front().syms);
              syms.emplace( std::make_pair(symId, psym) ); // ???
          }
          catch(...){
              std::cout<<" Error creating newsym at "<<__FILE__<<":"<<__LINE__<<std::endl;
              throw;
          }
          assert( syms.find(symId) != syms.end() );
          assert( syms.find(symId)->second.uid == symId );
          assert( syms.find(symId)->second.scope == scope );
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
                      oursym.chk_different_name(sc.syms);
                  }catch(...){
                      // turn this exception into a non-fatal warning
                      std::cout<<" Warning: symbol "<<oursym.name<<" in scope "<<ssu.scope()
                          <<" hides one of same name in enclosing scope "<<sc.scope<<std::endl;
                  }
              }
              doit=true;
          }
          return symId;
      }
      void delsym(unsigned const symId){
          auto const psym = syms.find(symId);
          assert( active(symId) );
          psym->second.setActive(false);
      }
      unsigned scope() const { return ssu.scope(); }
      size_t nScopeSymbols() { return ssu.nScopeSymbols(); }
      size_t nSymbols() { return syms.size(); }
      void prtCurrentSymbols() const{
          std::cout<<" CurrentScope"; std::cout.flush();
          SymScopeUid::ScopeSymbols const& curScopeSymbols = ssu.scopes.front();
          unsigned scope = curScopeSymbols.scope;
          std::cout<<scope; std::cout.flush();
          SymScopeUid::HashSet const& curSyms = curScopeSymbols.syms;
          char const* sep = "{";
          if(curSyms.empty()) std::cout<<sep;
          else for(auto const sym : curSyms ){
              std::cout<<sep<<sym; std::cout.flush();
              auto found = syms.find(sym);
              assert( found != syms.end() );
              assert( found->second.uid == sym );
              //assert( syms[sym].active == true );
              assert( found->second.scope == scope );
              if( !found->second.active ) std::cout<<"!";
              sep = ",";
          }
          std::cout<<"}";
          std::cout.flush();
      }
};//class SymbStates

#if 0
inline void ExBaseSym::chk_name_unique_in_current_scope()
{
    std::cout<<" symids()@"<<(void*)symids()<<std::endl;
    SymScopeUid::HashSet const& ids = symids()->scopeSymUids();
    for( auto id: ids ){
        if( strcmp(symids()->psym(id).name, name) == 0 ){
            THROW("Symbol "<<name<<" already exists in current scope");
        }
    }
}
#endif

// test class separation by putting the "USER CLASS" after everything else...

/** This is an example base class.  In practice, many more features
 * would be provided automatically (including perhaps requiring a symbol "name").
 *
 * Some [std] features would augment ParSymbol<BASE>, and special features
 * [ex. register usage, arg memory locations, spill memory locations, ...]
 * would go into BASE.
 */
class ExBaseSym{
  public:
      typedef SymbStates<ExBaseSym> Ssym;
      typedef ParSymbol<ExBaseSym> Psym;
      //friend class ve::Spill<SymbStates<ExBaseSym>>;
      template<class SYMBSTATES> friend class ve::Spill;
      friend Psym;
      friend Ssym;

      virtual ~ExBaseSym() {}
  protected:
      ExBaseSym() : /*psym(psym),*/ name("noname?") {
          THROW("ExBaseSym constructor missing args: please use newsym(ExBaseSym_args...)");
      }
      ExBaseSym(char const* name) : name(name) {
          //chk_name_unique_in_current_scope(); NO! parent is not fully initialized at this point!
      }
      Psym const *parent() const { return dynamic_cast<Psym const*>(this);}
      Ssym const *symids() const { return (dynamic_cast<Psym const*>(this))->ssym;}
  private:
      /** throw on duplicate symbol name in scame scope.
       * Cannot have 2 duplicate symbol names [with different uids] in same scope.
       * Note: compare our name only to \c ids that don't match our \c parent()->uid !
       */
      template<typename SymIdSet>
      void chk_different_name(SymIdSet const& ids) const;

      char const* name;
};

template<typename SymIdSet>
void ExBaseSym::chk_different_name(SymIdSet const& ids) const
{
    //std::cout<<" check_name_not_in({";
    for( auto id: ids ){ std::cout<<" "<<id; }
    //std::cout<<" }"<<std::endl;
    for( auto id: ids ){
        //std::cout<<" id"<<id; std::cout.flush();
        if( id == parent()->uid )       // NB: skip ourself when checking for duplicates!
            continue;
        char const* existing_name = symids()->psym(id).name;
        if( strcmp(name, existing_name) == 0 ){
            THROW("Symbol "<<name<<" already exists in current scope");
        }
    }
}

#endif // SYMBSTATES_HPP
// vim: ts=4 sw=4 et cindent cino=^=l0,\:.5s,=-.5s,N-s,g.5s,b1 cinkeys=0{,0},0),\:,0#,!^F,o,O,e,0=break
