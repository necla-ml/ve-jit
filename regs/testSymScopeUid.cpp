/** \file
 * Explore some barebones helpers for tests of \ref symScopeUid.hpp ideas.
 * (incl ideas that didn't make it into the symScopeUid.hpp header).
 *
 * These classes show some ways to use the \c SymScopeUid data structure.
 * <B>There are some levels of refinement:</B>
 *
 * 1. \c SymStates&lt;SYMBASE&gt; is a simple wrapper, providing a void* state.
 *   - with some examples of what to do with SYMBASE to add some state.
 *   - \ref symScopeUid-test.hpp
 *
 * 2. The more complex <B>\c SymbStates&lt;BASE&gt;</B> internally wraps your BASE
 *   class into a \c ParSymbol&lt;BASE&gt; data member.
 *   - \c begin_scope .. \c end_scope etc. work as here.
 *   - adds \c psym(symId) -> ParSymbol<BASE> lookup function for state.
 *   - \c newsym(Arg&&... arg) now accepts an arg pack that is forwarded to
 *     - ParSymbol&lt;BASE&gt; constructor (with some symId and scope info),
 *     - and then your BASE constructor.
 *
 * ParSymbol + SymbStates ended up being the 'official' route.
 */

//---------------------------------- symScopeUid-test.hpp  (only used here)
#include "symScopeUid.hpp"

namespace scope {
/** Mainly provides \c SymScopeUid some other 'ideas' code for testing. */
namespace detail {

// version 1: reusable uid-only sample impl
class BaseSymbolState;
template<typename SYMBASE> class SymStates; // fwd decl (in symScopeUid.hpp)

// version 2: CRTP impl

class ExBaseSym; ///< an example BASE for SymbStates (ParSymbol)
}//detail::

/** for the demo class... */
std::ostream& operator<<(std::ostream&, const detail::ExBaseSym&);

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

}//scope::

namespace scope{
namespace detail{

/** A low-level helper class allowing \c SymStates to add the \e active
 * attribute to a symbol.  This is a default template class for
 * SymStates<SYMBASE=BaseSymbolState>.  It allows SymStates to return
 * an additional opaque \c state.
 */
struct BaseSymbolState {
  public:
    //BaseSymbolState() : uid(0U), scope(0U), state(nullptr), active(false) {}
    bool getActive() const {return this->active;}
    unsigned symId() const {return this->uid;}
    unsigned scope() const {return this->scope_;}
    unsigned const uid;
    unsigned const scope_;
    void *state;              // arbitrary client state
  protected:
    template<class SYMSTATE> friend class SymStates;
    //void setUid(unsigned const uid)           {this->uid = uid;}
    //void setScope(unsigned const scope)       {this->scope = scope;}
    void setActive(bool const active)         {this->active = active;}
    BaseSymbolState(unsigned uid, unsigned scope)
        : uid(uid), scope_(scope), state(nullptr), active(true) {
            assert(uid > 0U);
            assert(scope > 0U);
        }
  private:
    bool active;
};

/** Wrap a \c SymScopeUid with a map of symbol id --> state.
 *
 * Allowed states must at least provide the functionality of SYMBASE, which
 * might add user-defined state to a \c BaseSymbolState.
 *
 * Basic usage is:
 *
 * - <B>\c begin_scope</B>, Now you can inquire about:
 *   - current \c scope() (some number)
 *   - number of symbols in current scope \c nScopeSymbols()
 *     - (or total symbols \c nSymbols)
 *     - or you can \c prtCurrentSymbols
 * - <B>auto x = \c newsym</B>,
 *   - you can now asck for \c scopeOf(x)
 *   - you now cat ask if \c active(x)
 *     - \c x begins active, and remains so until a matching \c end_scope.
 *       - unless you \c delsym(x) in this or any contained scope.
 *         - you can only delsym(x) once.  Then !active(x).
 * - \c stateOf(x).state = new MyState(...) [see below!],
 *   - because I really know nothing about your grand plans for \c x.
 *   - and I don't want to know anything about your grand plans for \c x.
 * - \c end_scope
 *   - Now the \em current scope becomes stale
 *     - all contained variables are set to inactive
 *
 * - For 'linear' programs \c begin_scope -- \c end_scope is all you need!
 * - You \em can \c activate_scope(stale scope id).
 *   - To re-enter a sub-block during a loop construct, for example.
 *     - re-using old symbols for induction, say.
 * - It is important to reactivate in correct order, because what happens is
 *   - the stale scope is \e moved from the list of stale scopes to become
 *     the current active scope, as if we had done a \e begin_scope.
 *     - internal state reactivated symbols is... Oh. That's your problem.
 *
 * Now back to \c stateOf(x).state = new MyState(...) 
 *
 * - Ex. 1
 *   - Suppose you want to derive from \c BaseSymbolState
 *     and insulate the user from every seeing that void *state.
 *   - We provide an example in \c BaseSymbol<DERIVED>, where the
 *     SYMBASE functionality is beefed up with the ability to cast
 *     up to your own DERIVED class, such as the \c ExDerivedSym class
 *     that allows you attach a character string ExDerivedSym::name
 *     as an example of additional state.
 * - Ex. 2
 *   - \e Better. Use your client class as a base class, as in ParSymbol<BASE>
 *
 */
template<class SYMBASE/*=BaseSymbolState*/>
class SymStates{
  private:
    SymScopeUid ssu;
    /** symbol uid --> external symbol state */
    std::unordered_map<unsigned/*symId*/,SYMBASE> syms;
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
            ret = psym->second.scope();
        }
        assert( ret == ssu.scopeOf(symId) );
        return ret;
    }
    BaseSymbolState const& stateOf(unsigned const symId) const {
        auto const psym = syms.find(symId);
        assert( psym != syms.end() );
        return psym->second.state;
    }
    /** return \b scope of \c symId.
     * Note: diff't from bool \c stateOf(symId).getActive()
     */
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
    template<typename... Arg>
    unsigned newsym(Arg&&... arg) {
        std::cout<<" ssu.newsym()..."<<std::endl; std::cout.flush();
        unsigned const symId = ssu.newsym();
        std::cout<<" ssu.newsym() --> symId = "<<symId<<std::endl; std::cout.flush();
        assert( ssu.active(symId) );
        assert( ssu.active(symId) == this->scope() );
        std::cout<<"u"<<symId; std::cout.flush();
        unsigned const scope = this->scope();
        std::cout<<"s"<<scope; std::cout.flush();
        assert( syms.find(symId) == syms.end() );
        std::cout<<" ssu.newsym() insert..."<<std::endl; std::cout.flush();
        syms.insert( std::make_pair(symId, SYMBASE{symId,scope,arg...}) ); // ???
        std::cout<<" ssu.newsym() insert DONE..."<<std::endl; std::cout.flush();
        assert( syms.find(symId) != syms.end() );
        assert( syms.find(symId)->second.symId() == symId );
        assert( syms.find(symId)->second.scope() == scope );
        assert( syms.find(symId)->second.getActive() == true );
        std::cout<<"+"; std::cout.flush();
        assert( ssu.active(symId) == scope );
        assert( this->active(symId) == scope );
        std::cout<<"+"; std::cout.flush();
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
            assert( found->second.symId() == sym );
            //assert( syms[sym].active == true );
            assert( found->second.scope() == scope );
            if( !found->second.active ) std::cout<<"!";
            sep = ",";
        }
        std::cout<<"}";
        std::cout.flush();
    }
};//class SymStates

/** Maybe...
 * \tparm DERIVED is a derived class, such as:
 * ```
 * class Derived : public BaseSymbolState<Derived> {
 *   Derived(<arglist>) : BaseSymbolState<Derived>(uid,scope,this?)
 *   ...
 * }
 * ```
 * .. just an idea ..
 */
class ExDerivedSym;
template<class DERIVED=ExDerivedSym>
struct BaseSymbol {
  public:
    //typedef DERIVED Derived;
    //BaseSymbolState() : uid(0U), scope(0U), state(nullptr), active(false) {}
    bool getActive() const {return this->active;}
    unsigned const uid;
    unsigned const scope;
    virtual ~BaseSymbol(){}
    /** cast to DERIVED */
    DERIVED & der() { return static_cast<DERIVED&>(*this); }
    DERIVED const& der() const { return static_cast<DERIVED const&>(*this); }
    //void *state;              // arbitrary client state
  protected:
    template<class SYMSTATE> friend class SymStates;
    //void setUid(unsigned const uid)           {this->uid = uid;}
    //void setScope(unsigned const scope)       {this->scope = scope;}
    void setActive(bool const active)         {this->active = active;}
    BaseSymbol(unsigned uid, unsigned scope)
        : uid(uid), scope(scope)
          //, state(nullptr)
          , active(true) {
              assert(uid > 0U);
              assert(scope > 0U);
          }
    BaseSymbol(void*,unsigned uid, unsigned scope)
        :BaseSymbol(uid,scope) {}
  private:
    bool active;
};
/** This is a derived class that takes an additional \c name to construct */
class ExDerivedSym : public BaseSymbol<ExDerivedSym> {
  public:
    virtual ~ExDerivedSym(){}
    template<typename... Arg>
        ExDerivedSym(unsigned uid, unsigned scope, Arg&&... arg)
        : BaseSymbol(uid, scope)                    // 1st two args ---> Base class constructor
          //NO , ExDerivedSym(std::forward<Arg>(arg)...) // delegate other args to "our" constructor
        {
            this->init(std::forward<Arg>(arg)...);
        }
  protected:
    char const* name;
  private:
    void init(char const* name)
    { this->name = name; }
};

/** This is an example BASE class.  In practice, many more features
 * would be provided automatically (including perhaps requiring a symbol "name").
 *
 * Some [std] features would augment ParSymbol<BASE>, and special features
 * [ex. register usage, arg memory locations, spill memory locations, ...]
 * would go into BASE.
 */
class ExBaseSym{
  public:
    typedef ParSymbol<ExBaseSym> Psym;
    typedef SymbStates<Psym> Ssym;
    typedef void Base;
    friend Psym;
    friend Ssym;
    friend std::ostream& scope::operator<<(std::ostream& os, ExBaseSym const& x);

    virtual ~ExBaseSym() {}
  protected:
    ExBaseSym() : /*psym(psym),*/ name("noname?") {
        // for demo with ParSymbol let's allow this!
        //THROW("ExBaseSym constructor missing args: please use newsym(ExBaseSym_args...)");
    }
    ExBaseSym(char const* name) : name(name) {
        //chk_name_unique_in_current_scope(); NO! parent is not fully initialized at this point!
    }
    /** return the ParSymbol, which has scope+uid, and maybe a SymbStates ptr.
     * For example our low-level symbol id is parent()->uid.
     */
    Psym const *parent() const { return dynamic_cast<Psym const*>(this);}
    /** might return null */
    //Ssym const *symids() const { return (dynamic_cast<Psym const*>(this))->ssym;}
  private:
    /** throw on duplicate symbol name in scame scope.
     * Cannot have 2 duplicate symbol names [with different uids] in same scope.
     * NEW: PsymSet is a set of Psym const* (not a set of symid).
     */
    template<typename SET>
        void chk_different_name(SET const& ids) const;

    void setActive(bool set=true) {} // nothing to do, but we were told about it.
    char const* name;
};

template<typename SET>
void ExBaseSym::chk_different_name(SET const& psyms) const
{
    //std::cout<<" check_different_name("<<this->name<<", {";
    //for( auto psym: psyms ){ std::cout<<" "<<psym; }
    //std::cout<<" })"<<std::endl; std::cout.flush();
    for( auto psym: psyms ){
        std::cout<<" psym"<<psym; std::cout.flush();
        if( psym->symId() == parent()->symId() )       // NB: skip ourself when checking for duplicates!
            continue; // we can never clash with ourself (safety)
        char const* existing_name = psym->name;
        //std::cout<<" name="<<this->name<<" existing_name="<<existing_name<<std::endl; std::cout.flush();
        if( strcmp(this->name, existing_name) == 0 ){
            THROW("Symbol "<<name<<" already exists in current scope");
        }
    }
}

}//detail::

std::ostream& operator<<(std::ostream& os, detail::ExBaseSym const& x)
{
    return os<<"ExBaseSym{"<<x.name<<"}";
}

#if 0
template<class BASE>
    template<typename... Arg>
inline ParSymbol<BASE>::ParSymbol(/*Ssym *ssym,*/ unsigned uid, unsigned scope, Arg&&... arg)
    : BASE(detail::bogusname(uid).c_str(), std::forward<Arg>(arg)...)
      , uid(uid), scope(scope), ssym(nullptr), active(true)
{
    int const verbose=1;
    if(verbose){
        std::cout<<" Par(id"<<uid<<",sc"<<scope<<")" // <<ssym@"<<(void*)ssym
            <<"'"<<BASE::name<<"'"; std::cout.flush();
    }
}
#endif

}//scope::
//---------------------------------- END symScopeUid-test.hpp  (only used here)

//---------------------------------- test program
#include <iostream>
using namespace std;

using namespace scope;
using namespace scope::detail;

#if 0
namespace scope {

// just for demo...
inline std::string bogusname(unsigned const uid){
    std::ostringstream oss;
    oss<<uid;
    return oss.str();
}
// interesting that this somewhat different-looking constructor can be supplied like
// a specialization, out-of-class...
template<class BASE>
    template<typename... Arg>
inline ParSymbol<BASE>::ParSymbol(/*Ssym *ssym,*/ unsigned uid, unsigned scope, Arg&&... arg)
    : BASE(bogusname(uid).c_str(), std::forward<Arg>(arg)...)
      , uid(uid), scope(scope), ssym(nullptr), active(true)
{
    int const verbose=1;
    if(verbose){
        std::cout<<" Par(id"<<uid<<",sc"<<scope<<")" // <<ssym@"<<(void*)ssym
            <<"'"<<BASE::name<<"'"; std::cout.flush();
    }
}
}//scope::
#endif

static int testNum=0;
#define TEST(MSG) do{ \
    ++testNum; \
    std::cout<<"@@@ TEST "<<testNum<<" @@@ "; \
    if(MSG) std::cout<<MSG; \
    std::cout<<std::endl; \
}while(0)

#if 0
struct VerboseSymbolState {
    void setActive(bool state) {
        this->active = active;
        cout<<(active?"+":"-");
    }
    bool getActive() const { return active; }
  private:
    bool active;
};
#endif

void test1(){
    //typedef SymScopeUid<> Ssu;
    //typedef SymScopeUid<VerboseSymbolState> Ssu;
    typedef SymScopeUid Ssu;
    TEST("Construct SymScopeUid");
    {
        Ssu ssu;
        ssu.prtCurrentSymbols();
        assert( ssu.scope() == 1U );
        assert( ssu.nScopeSymbols() == 0U );
        assert( ssu.scopeOf(1U) == 0U );
        assert( ssu.active(1U) == 0U );
        cout<<endl;
    }
    TEST("global scope");
    {
        Ssu ssu;
        auto x = ssu.newsym();
        assert( x!=0U );
        auto y = ssu.newsym();
        assert( y!=0U );
        assert( y!=x );
        assert( ssu.scopeOf(x) == 1U );
        assert( ssu.scopeOf(y) == 1U );
        assert( ssu.active(x) == 1U );
        assert( ssu.active(y) == 1U );
        cout<<endl; ssu.prtCurrentSymbols(); cout<<endl;
        auto zscope = ssu.begin_scope();
        assert( ssu.scope() == 2U );
        auto z = ssu.newsym();
        assert( z > y );
        assert( ssu.scopeOf(x) == 1U );
        assert( ssu.scopeOf(y) == 1U );
        assert( ssu.active(x) == 1U );
        assert( ssu.active(y) == 1U );
        assert( ssu.scopeOf(z) == 2U );
        assert( ssu.active(z) == 2U );
        cout<<endl; ssu.prtCurrentSymbols(); cout<<endl;
        ssu.end_scope();
        cout<<endl; ssu.prtCurrentSymbols(); cout<<endl;
        assert( ssu.scopeOf(x) == 1U );
        assert( ssu.scopeOf(y) == 1U );
        assert( ssu.active(x) == 1U );
        assert( ssu.active(y) == 1U );
        assert( ssu.scopeOf(z) == 2U );
        cout<<" ssu.active(z) = "<<ssu.active(z)<<endl;
        assert( ssu.active(z) == 0U ); // <---

        ssu.activate_scope(zscope);
        assert( ssu.scopeOf(z) == 2U );
        assert( ssu.active(z) == 2U );
        ssu.end_scope();
        assert( ssu.scopeOf(z) == 2U );
        assert( ssu.active(z) == 0U );

    }
}
void test2(){
    //typedef SymScopeUid<> Ssu;
    //typedef SymScopeUid<VerboseSymbolState> Ssu;
    typedef SymStates<> Sst;
    TEST("SymStates<> Construct SymScopeUid");
    {
        Sst sst;
        sst.prtCurrentSymbols();
        assert( sst.scope() == 1U );
        assert( sst.nScopeSymbols() == 0U );
        assert( sst.scopeOf(1U) == 0U );
        assert( sst.active(1U) == 0U );
        cout<<endl;
    }
    TEST("SymStates<> global scope");
    {
        Sst sst;
        auto x = sst.newsym();
        assert( x!=0U );
        auto y = sst.newsym();
        assert( y!=0U );
        assert( y!=x );
        assert( sst.scopeOf(x) == 1U );
        assert( sst.scopeOf(y) == 1U );
        assert( sst.active(x) == 1U );
        assert( sst.active(y) == 1U );
        cout<<endl; sst.prtCurrentSymbols(); cout<<endl;
        auto zscope = sst.begin_scope();
        cout<<"scope="<<zscope<<" "; cout.flush();
        sst.prtCurrentSymbols(); cout<<endl;
        assert( sst.scope() == 2U );
        auto z = sst.newsym();
        cout<<"z="<<z<<endl; cout.flush();
        sst.prtCurrentSymbols(); cout<<endl; cout.flush();
        cout.flush();
        cout<<" sst.active(z)="<<sst.active(z)<<endl;
        cout.flush();
        assert( sst.active(z) );
        assert( z > y );
        assert( sst.scopeOf(x) == 1U );
        assert( sst.scopeOf(y) == 1U );
        assert( sst.active(x) == 1U );
        assert( sst.active(y) == 1U );
        assert( sst.scopeOf(z) == 2U );
        assert( sst.active(z) == 2U ); // oops?
        sst.end_scope();
        cout<<endl; sst.prtCurrentSymbols(); cout<<endl;
        assert( sst.scopeOf(x) == 1U );
        assert( sst.scopeOf(y) == 1U );
        assert( sst.active(x) == 1U );
        assert( sst.active(y) == 1U );
        assert( sst.scopeOf(z) == 2U );
        cout<<" sst.active(z) = "<<sst.active(z)<<endl;
        assert( sst.active(z) == 0U ); // <---

        sst.activate_scope(zscope);
        assert( sst.scopeOf(z) == 2U );
        assert( sst.active(z) == 2U );
        sst.end_scope();
        assert( sst.scopeOf(z) == 2U );
        assert( sst.active(z) == 0U );

    }
}
void test2b(){
    // the new ParSymbol<BASE> also has active(), getActive() and uid members...
    typedef SymStates<ParSymbol<ExBaseSym>> Sst;
    TEST("SymStates<ParSymbol<ExBaseSym> Construct SymScopeUid");
    {
        Sst sst;
        sst.prtCurrentSymbols();
        assert( sst.scope() == 1U );
        assert( sst.nScopeSymbols() == 0U );
        assert( sst.scopeOf(1U) == 0U );
        assert( sst.active(1U) == 0U );
        cout<<endl;
    }
    // Any more is doomed, because ParSymbol constructor requires
    // that the first arg be a back-pointer to the SymStates object.
    //
    // This is just a minor difference and was patched by providing
    // a back-pointerless constructor, that assigned a bogusname
    // to any symbol.
    //
    // Named symbols use class SymbStates instead.
    TEST("SymStates<ParSymbol<ExBaseSym> global scope");
    {
        Sst sst;
        cout<<" sst.newsym..."<<endl; cout.flush();
        auto x = sst.newsym();
        cout<<" sst.newsym... DONE"<<endl; cout.flush();
        assert( x!=0U );
        auto y = sst.newsym();
        assert( y!=0U );
        assert( y!=x );
        assert( sst.scopeOf(x) == 1U );
        assert( sst.scopeOf(y) == 1U );
        assert( sst.active(x) == 1U );
        assert( sst.active(y) == 1U );
        cout<<endl; sst.prtCurrentSymbols(); cout<<endl;
    }
    TEST("SymStates<ParSymbol<ExBaseSym> sub scope");
    {
        Sst sst;
        auto x = sst.newsym();
        auto y = sst.newsym();
        cout<<endl; sst.prtCurrentSymbols(); cout<<endl;
        auto zscope = sst.begin_scope();
        cout<<"scope="<<zscope<<" "; cout.flush();
        sst.prtCurrentSymbols(); cout<<endl;
        assert( sst.scope() == 2U );
        auto z = sst.newsym();
        cout<<"z="<<z<<endl; cout.flush();
        sst.prtCurrentSymbols(); cout<<endl; cout.flush();
        cout.flush();
        cout<<" sst.active(z)="<<sst.active(z)<<endl;
        cout.flush();
        assert( sst.active(z) );
        assert( z > y );
        assert( sst.scopeOf(x) == 1U );
        assert( sst.scopeOf(y) == 1U );
        assert( sst.active(x) == 1U );
        assert( sst.active(y) == 1U );
        assert( sst.scopeOf(z) == 2U );
        assert( sst.active(z) == 2U ); // oops?
        sst.end_scope();
        cout<<endl; sst.prtCurrentSymbols(); cout<<endl;
        assert( sst.scopeOf(x) == 1U );
        assert( sst.scopeOf(y) == 1U );
        assert( sst.active(x) == 1U );
        assert( sst.active(y) == 1U );
        assert( sst.scopeOf(z) == 2U );
        cout<<" sst.active(z) = "<<sst.active(z)<<endl;
        assert( sst.active(z) == 0U ); // <---

        sst.activate_scope(zscope);
        assert( sst.scopeOf(z) == 2U );
        assert( sst.active(z) == 2U );
        sst.end_scope();
        assert( sst.scopeOf(z) == 2U );
        assert( sst.active(z) == 0U );

    }
}
void test3(){
    //typedef SymScopeUid<> Ssu;
    //typedef SymScopeUid<VerboseSymbolState> Ssu;
    //typedef SymStates<> Sst;
    typedef SymbStates<ParSymbol<ExBaseSym>> Ssym;
    //typedef ParSymbol<ExBaseSym> Psym;
    TEST("SymStates<ParSymbol<ExBaseSym> Construct SymScopeUid");
    {
        Ssym ssym;
        ssym.prtCurrentSymbols();
        assert( ssym.scope() == 1U );
        assert( ssym.nScopeSymbols() == 0U );
        assert( ssym.scopeOf(1U) == 0U );
        assert( ssym.active(1U) == 0U );
        cout<<endl;
    }
    TEST("SymStates<ParSymbol<ExBaseSym> Add 2 symbols to Global scope");
    {
        Ssym ssym;
        cout<<"Init syms "; ssym.prtCurrentSymbols(); cout<<endl;
        assert( ssym.scope() == 1U );
        assert( ssym.nScopeSymbols() == 0U );
        assert( ssym.scopeOf(1U) == 0U );
        assert( ssym.active(1U) == 0U );

        auto x = ssym.newsym("x");
        assert( x!=0U );
        auto y = ssym.newsym("y");
        assert( y!=0U );
        assert( y!=x );
        assert( ssym.scopeOf(x) == 1U );
        assert( ssym.scopeOf(y) == 1U );
        assert( ssym.active(x) == 1U );
        assert( ssym.active(y) == 1U );
        cout<<"\nFinal syms "; ssym.prtCurrentSymbols(); cout<<endl;
    }
    TEST("SymStates<ParSymbol<ExBaseSym> global scope");
    {
        Ssym ssym;
        auto x = ssym.newsym("x");
        auto y = ssym.newsym("y");
        cout<<endl; ssym.prtCurrentSymbols(); cout<<endl;
        auto zscope = ssym.begin_scope();
        cout<<"scope="<<zscope<<" "; cout.flush();
        ssym.prtCurrentSymbols(); cout<<endl;
        assert( ssym.scope() == 2U );
        auto z = ssym.newsym("z");
        cout<<"z="<<z<<endl; cout.flush();
        ssym.prtCurrentSymbols(); cout<<endl; cout.flush();
        cout.flush();
        cout<<" ssym.active(z)="<<ssym.active(z)<<endl;
        cout.flush();
        assert( ssym.active(z) );
        assert( z > y );
        assert( ssym.scopeOf(x) == 1U );
        assert( ssym.scopeOf(y) == 1U );
        assert( ssym.active(x) == 1U );
        assert( ssym.active(y) == 1U );
        assert( ssym.scopeOf(z) == 2U );
        assert( ssym.active(z) == 2U );
        cout<<"scope "<<ssym.scope(); ssym.prtCurrentSymbols(); cout<<endl;
        ssym.end_scope();
        cout<<"scope "<<ssym.scope(); ssym.prtCurrentSymbols(); cout<<endl;
        assert( ssym.scopeOf(x) == 1U );
        assert( ssym.scopeOf(y) == 1U );
        assert( ssym.active(x) == 1U );
        assert( ssym.active(y) == 1U );
        assert( ssym.scopeOf(z) == 2U );
        cout<<" ssym.active(z) = "<<ssym.active(z)<<endl;
        assert( ssym.active(z) == 0U ); // <---

        ssym.activate_scope(zscope);
        assert( ssym.scopeOf(z) == 2U );
        assert( ssym.active(z) == 2U );
        ssym.end_scope();
        assert( ssym.scopeOf(z) == 2U );
        assert( ssym.active(z) == 0U );

    }
}

int main(int,char**){
    cout<<"======== test1() ==========="<<endl;
    test1();
    cout<<"======== test2() ==========="<<endl;
    test2();
    cout<<"======== test2b() =========="<<endl;
    test2b();
    cout<<"======== test3() ==========="<<endl;
    test3();
    cout<<"\nGoodbye - ran "<<testNum<<" tests"<<endl;
    return 0;
}
// vim: ts=4 sw=4 et cindent cino=^l0,\:.5s,=-.5s,N-s,g.5s,h.5s,b1 cinkeys=0{,0},0),\:,0#,!^F,o,O,e,0=break syntax=cpp.doxygen
