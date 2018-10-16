
#include "spill-impl2.hpp"
#include "symScopeUid.hpp"
#include "scopedSpillableBase.hpp"
#include <cassert>
#include <iostream>
#include <map>
using namespace std;

#define STAGE 3

#if STAGE >= 0
static int testNum=0;
#define TEST(MSG) do{ \
    ++testNum; \
    std::cout<<"\n@@@ TEST "<<testNum<<" @@@ "; \
    if(MSG) std::cout<<MSG; \
    std::cout<<std::endl; \
}while(0)

#define STRINGY(FOO) #FOO
#define ASSERTTHROW( CODE ) do{ \
    int err=0; try{ CODE; } \
    catch(std::exception const &ex){ \
        cout<<" (good, exception: "<<STRINGY(CODE)<<" : "<<ex.what()<<endl; \
        ++err; } \
    catch(...){ ++err; } \
    if(err==0) THROW("Oops: expected throw from '"<<STRINGY(CODE)<<"'"); \
}while(0)
#endif // STAGE >= 0

#if STAGE >= 1
//class SpillableSym; // ScopedSpillableSym is for now hardwired to ParSymbol<ScopedSpillableSym> ?
//namespace scope{
class DemoSymbStates;
//}//scope::

struct Tester{
    static void test1();
    static void test2();
    static void test3();
};

#if 0
/** Function arg symbols may not need spilling to a spill region.
 * 'C' ABI say where function args are in register and/or memory.
 *
 * Aurora ABI assigns every function argument symbol a reserved
 * memory location.  This memory is upward-growing in stack frame,
 * whereas Spill memory is downward-growing from frame pointer.
 *
 * Spill memory max size is tracked so that function prologue can
 * set up a large-enough stack frame upon function entry.
 *
 * ===
 *
 * ScopedSpillableBase relegates symId and active state to ParSymbol.
 *
 * So ParSymbol<ScopeSpillableBase> roughly replaces SpillableBase
 */
class SpillableSym : public ScopedSpillableBase
{
  public:
    friend class Tester;

    //typedef scope::ParSymbol<ScopedSpillableBase> Base;
    //SpillableSym(unsigned uid, int bytes, int align)
    //    : Base(uid, bytes,align) {}
    SpillableSym(int bytes, int align)
        : ScopedSpillableBase(bytes,align) {}

    // /** map Region::symId to SpillableSym */
    // DemoSymbStates *ssym;
    //----
    //void scope_enter() {assert(getActive()==false); setActive(true);}
    //void scope_exit() {setActive(false);}
    //bool isSpillable() const { return (REG&getWhere()); }
    //Where w(int const i) const {return static_cast<Where>(i); }
};
#endif

//namespace scope{
/** Spillable objects need a way to convert symbol Ids into SpillableSym.  Here
 * we do not use the usual scope::SymbStates, but a simpler symbol creation
 * helper that helps us model a machine with only 4 register locations.  Recall
 * we can check all the symbols' location with getREG/getMEM.
 */
class DemoSymbStates : public scope::SymbStates<ScopedSpillableBase> {
  public:
    typedef scope::SymbStates<ScopedSpillableBase> Base;
    typedef Base::Psym Psym;

    /** The spill manager lifetime should be less than the psyms map lifetime */
    ve::Spill<DemoSymbStates> spill;
    friend class ve::Spill<DemoSymbStates>;

    DemoSymbStates()
        : Base(), spill(this) { /*Base::begin_scope();*/ }
    ~DemoSymbStates() {
        //Base::end_scope();
    }
  protected: // not used for testing here
    unsigned begin_scope();
    void end_scope();
    void activate_scope(unsigned const);
    unsigned scopeOf(unsigned const symId);
    unsigned active(unsigned const symId);

  public:
    template<typename... Arg>
        unsigned newsym(Arg&&... arg) { return Base::newsym(arg...); }

    // usually, this one is protected, but make it public for testing
    Psym const& psym(unsigned const symId) const { return Base::psym(symId); }
  public: // protected in Base, public for tests
    friend class Tester;
    Psym      & ppsym(unsigned const symId)       { return Base::psym(symId); }
  public:
    ScopedSpillableBase & scopedSpillable(unsigned const symId) {return ppsym(symId).base();}
    ScopedSpillableBase & ssb(unsigned const symId) {return scopedSpillable(symId);}

    Psym const& mkPsym(int bytes, int align){
        unsigned symId= newsym(bytes, align);
        Psym const& ret = psym(symId);
        return ret;
    }

#if 0 // THE ONLY WAY TO CONSTRUCT A PSYM IS VIA ADDSYM
    Psym mkPsym(unsigned uid, int bytes, int align){
        return Psym(nullptr, uid, scope(), bytes, align);
        //return Psym(this, uid, scope, bytes, align);
    }
    //Psym mkPsym(unsigned uid, int bytes, int align){
    //    return Psym(uid, 1, bytes, align);
    //}


    /** declare a symbol. It is active, but in neither REG nor MEM.
     * For testing, instead of a monolithic 'newsym(...)', we can construct
     * a psym, and \e then store it.  (We don't care about scopes at all, for now).
     */
    void add(Psym const& s){
        assert( syms.find(s.uid) == syms.end() );
        assert( !scopes.empty() );
        ScopeSymbols& curScopeSymbols = scopes.front();
        HashSet& scopeSyms = curScopeSymbols.syms;
        assert( scopeSyms.find(s.uid) == scopeSyms.end() );

        syms.emplace( s.uid, s );
        //syms.insert( std::make_pair(s.uid, s));
        scopeSyms[s.uid] = curScopeSymbols.scope;

    }
#endif
    size_t regOnly() const {
        return count_if( syms.begin(), syms.end(),
                [](map<unsigned,Psym>::value_type const& v)
                { return v.second.getREG() && !v.second.getMEM(); });
    }
    size_t memOnly() const {
       return count_if( syms.begin(), syms.end(),
                [](map<unsigned,Psym>::value_type const& v)
                { return !v.second.getREG() && v.second.getMEM() && v.second.getStale()==0; });
    }
#if 0 // scope::SymbStates is the only allowed controller of 'active'
    void setActive(unsigned symId, bool active){
        psym(symId).setActive(active);
    }
#endif
    auto const& getSyms() const {return Base::syms;} // unordered_map<symId,Psym>, all (dups inside ssu)
    auto symIdAnyScopeSorted() const { return ssu.symIdAnyScopeSorted(); }

    /** Into which of 4 registers do we put a new symbol?
     * Whenever we \c use4, and we have run out of registers, we spill
     * the first encountered not-already-spilled symbol to memory,
     * marking its register value as stale,
     * and assign that register to \c symId. */
    void use4(unsigned const symId){
        Psym & s = Base::psym(symId);
        if( s.getREG() ){
            cout<<" use4:mod"<<symId<<" "; cout.flush();
            s.setREG(true); // in-register symbol modified its value.
            return;
        }
        // for reproducibility, we will re-order by symId
        // (better might be staleness or ...)
        std::map<unsigned,Psym> osyms;
        for(auto const& v: syms) osyms.insert(v);
        if(0){
            cout<<" Compare syms:"<<endl;
            for(auto const& v: syms) cout<<"\t"<<v.second<<endl;
            cout<<" with osyms:"<<endl;
            for(auto const& v: osyms) cout<<"\t"<<v.second<<endl;
        }

        auto nRegs = count_if(osyms.begin(), osyms.end(),
                [](map<unsigned,Psym>::value_type const& v)
                { return v.second.getREG(); } );
        cout<<" use4:nRegs"<<nRegs<<" "; cout.flush();
        if( nRegs < 4 ){
            // have an available register? just use it
            // <update machine register mapping table HERE>
            cout<<" use4:avail"<<symId<<" "; cout.flush();
            s.setREG();
            return;
        }
        // else we need to spill some register to make room
        // Easy case: some register already has a nonstale memory copy ...
        auto rm = find_if( osyms.begin(), osyms.end(),
                [](map<unsigned,Psym>::value_type const& v)
                { return v.second.getREG() && v.second.getMEM() && v.second.getStale()==0; } );
        if( rm != osyms.end() ){
            cout<<" use4:easy"<<symId<<" "; cout.flush();
            assert( rm->second.getMEM() && rm->second.getStale()==0 );
            // <update machine register mapping table HERE>
            rm->second.setREG(false);
            s.setREG(true);
            return;
        }
        // else Find some register symbol and spill (or re-spill) it to memory
        auto r = find_if( osyms.begin(), osyms.end(),
                [](map<unsigned,Psym>::value_type const& v)
                { std::cout<<" "<<v.first<<"! ";std::cout.flush(); return v.second.getREG(); } );
        assert( r !=  osyms.end() );
        cout<<"< use4:(spill "<<r->first<<"):"<<symId<<" "; cout.flush();
        unsigned r_symid = r->first;
        assert( r_symid != symId );
        // now back to modifying syms [not osyms]...
        spill.spill(r_symid);          // 'r' --> RM
        fpsym(r_symid).setREG(false);        // 'r' --> ~RM
        // 's' takes the register away from prev symbol 'r'
        s.setREG(true);                // 's' --> R
    }
};

bool chk_order(DemoSymbStates const& ssym, std::initializer_list<unsigned> const& i){
    bool ok = true;
    auto ii = i.begin();
    for(auto const& r: ssym.spill.regions()){
        if( ii == i.end() ){
            ok = false;
            break;
        }
        if( *ii != r.symId ){
            ok = false;
            break;
        }
        ++ii;
    }
    if( ii != i.end() ){
        ok = false;
    }
    if(!ok){
        cout<<"\nExpected symId order {";
        for( auto ii: i ) cout<<" "<<ii;
        cout<<"}";
        cout<<"\nActual spill   order {";
        for( auto r: ssym.spill.regions() ) cout<<" "<<r.symId;
        cout<<"}"<<endl;
        cout.flush();
    }
    return ok;
}
//}//scope::
using namespace scope;


#include <typeinfo>
void Tester::test1(){
    typedef DemoSymbStates Ssym;
    typedef Ssym::Psym Psym;
    TEST("Construct some symbols");
    {
        Ssym ssym;
        // shorthand to construct 'test' Psym by hand...
        auto p = [&ssym](int byte, int align) -> Ssym::Psym const& {
            unsigned symId= ssym.newsym(byte,align);
            Psym const& ret = ssym.psym(symId);
            return ret;};
        ASSERTTHROW( ssym.psym(1U).getActive() );
        auto const& p1 = p(8,8);  // symId 1, [scope=0,] len=8, align=8 (64-bit register)
        assert( p1.uid == 1U );
        auto const& p2 = p(8,8);  // symId 2, [scope=0,] len=8, align=8 (64-bit register)
        assert( p2.uid == 2U );
        cout<<endl;
        assert( ssym.psym(1U).uid == 1U );
        assert( ssym.psym(2U).uid == 2U );
        cout<<ssym.psym(1U)<<endl; cout.flush();
        cout<<ssym.psym(2U)<<endl; cout.flush();

        {
            Psym const& x = ssym.psym(1);
            cout<<"Psym const & x = "<<(void*)&x<<" type "<<typeid(x).name()<<endl; cout.flush();
            cout<<" x                    = "<<x<<endl;
            cout<<" Since x is ParSymbol<BASE>..."<<endl;
            cout<<" x.getActive() = "<<x.getActive()<<endl; cout.flush();
            cout<<" x.uid         = "<<x.uid<<endl;
            cout<<" x.scope       = "<<x.scope<<endl;
            cout<<" x.base()             = "<<x.base()<<endl;
            cout<<" Since x.base() is ScopedSpillableBase..."<<endl;
            cout<<" x.base().scope()     = "<<x.base().scope()<<endl; // ScopedSpillableBase
            cout<<" x.base().symId()     = "<<x.base().symId()<<endl;
            cout<<" x.base().getActive() = "<<x.base().getActive()<<endl;
            cout<<" x.base().getBytes()  = "<<x.base().getBytes()<<endl;
            cout<<" x.base().getAlign()  = "<<x.base().getAlign()<<endl;
            cout<<" x.base().getREG()    = "<<x.base().getREG()<<endl;
            cout<<" x.base().getMEM()    = "<<x.base().getMEM()<<endl;
            cout<<" x.base().getWhere()  = "<<x.base().getWhere()<<endl;
        }
        {   // alt name for public access during testing...
            Psym & x = ssym.fpsym(1);
            cout<<"Psym & x = "<<(void*)&x<<" type "<<typeid(x).name()<<endl; cout.flush();
            cout<<" x.getActive() = "<<x.getActive()<<endl; cout.flush();
        }
        { // todo this should be protected (not compilable)
            ScopedSpillableBase& x = ssym.ssb(1); // ssym.fpsym().base();
            cout<<"ScopedSpillableSymbol & x = "<<(void*)&x<<" type "<<typeid(x).name()<<endl; cout.flush();
            cout<<" x.getActive() = "<<x.getActive()<<endl; cout.flush();
        }
        { // test setREG and setMEM function of ParSymbol<ScopedSpillableBase>
            // these functions are in ScopedSpillableBase
            Psym & x = ssym.fpsym(1);
            x.setREG(true);
            ssym.ssb(1).setREG(true);
            assert( ssym.ssb(1).getREG() == true );
            ssym.ssb(1).setREG(false);
            assert( ssym.ssb(1).getREG() == false );
            ssym.ssb(1).setREG(true);
            assert( ssym.ssb(1).getREG() == true );

            ssym.ssb(1).setMEM(true);
            assert( ssym.ssb(1).getMEM() == true );
            assert( ssym.ssb(1).getREG() == true );
            ssym.ssb(1).setMEM(false);
            assert( ssym.ssb(1).getMEM() == false );
            assert( ssym.ssb(1).getREG() == true );
            ssym.ssb(1).setMEM(true);
            assert( ssym.ssb(1).getMEM() == true );
            assert( ssym.ssb(1).getREG() == true );
            cout<<endl;
        }
    }
    TEST("Construct and spill 2 [declared, unused] symbols (spill ignored)");
    {
        Ssym ssym;
        // shorthand to construct 'test' Psym by hand...
        auto sym = [&ssym](int byte,int align){return ssym.newsym(byte,align);};
        ASSERTTHROW( ssym.psym(1U).getActive() );
        auto const x = sym(8,8);  // symId 1, len=8, align=8 (64-bit register)
        assert( ssym.psym(x).uid == 1U );
        auto const y = ssym.newsym(8,8);  // symId 2, len=8, align=8 (64-bit register)
        assert( ssym.psym(y).uid == 2U );
        cout<<" ssym.spill BEFORE : ";ssym.spill.dump();cout<<endl;
        assert( ssym.spill.regions().empty() );
        ssym.spill.spill(x);
        ssym.spill.spill(y);
        cout<<" ssym.spill AFTER : ";ssym.spill.dump();cout<<endl;
        cout<<endl;
        assert( ssym.spill.regions().empty() );
    }
    TEST("Construct and spill 2 [declared, used] symbols");
    {
        Ssym ssym;
        // shorthand to construct 'test' Psym by hand...
        auto sym = [&ssym](int byte,int align){return ssym.newsym(byte,align);};
        // normally access to setREG(bool) is denied, but here...
        auto s=[&ssym](unsigned symId)->Ssym::Psym & {return ssym.fpsym(symId);};
        ASSERTTHROW( s(1) );
        auto const x = sym(8,8);  // symId 1
        assert( x == 1 );
        assert( s(x).uid == x );
        cout<<" add 1    : "<<s(1)<<endl;
        assert(s(1).getREG()==false);
        assert(s(1).getMEM()==false);
        assert(s(1).getStale()==0);
        cout<<" setREG(1)... ";
        s(1).setREG(true);   // say value is "in register"
        cout<<" setREG(1): "<<s(1)<<endl;
        assert(s(1).getREG()==true);
        assert(s(1).getMEM()==false);
        assert(s(1).getStale()==0);
        cout<<" spill(1) ... ";
        ssym.spill.dump();
        ssym.spill.spill(1);
        ssym.spill.dump();
        cout<<" spill(1) : "<<s(1)<<endl;
        assert(s(1).getREG()==true);
        assert(s(1).getMEM()==true);
        assert(s(1).getStale()==0);
        assert(ssym.spill.getBottom() == -8);
        auto const y = sym(8,8);  // symId 2
        assert( y == 2 );
        s(2).setREG(true);   // say value is "in register"
        ssym.spill.spill(2);
        assert(s(1).getREG()==true);
        assert(s(1).getMEM()==true);
        assert(s(1).getStale()==0);
        assert(s(2).getREG()==true);
        assert(s(2).getMEM()==true);
        assert(s(2).getStale()==0);
        assert(ssym.spill.getBottom() == -16);
        cout<<" spill(2) : "<<s(2)<<endl;
        ssym.spill.dump();
        //ssym.spill.spill(2);
        cout<<endl;
    }
    TEST("Construct, use, spill, unspill, respill ");
    {
        Ssym ssym;
        // shorthand to construct 'test' Psym by hand...
        auto sym = [&ssym](int byte,int align){return ssym.newsym(byte,align);};
        // shorthand to look up a symId
        auto s=[&ssym](unsigned symId)->Ssym::Psym &
        {return ssym.fpsym(symId);};

        auto const x = sym(8,8);  // symId 1
        s(x).setREG(true);      // say value is "in register"
        ssym.spill.spill(x);
        auto const y = sym(8,8);  // symId 2
        s(y).setREG(true);      // say value is "in register"
        assert( x==1 && y==2 );
        ssym.spill.spill(y);
        ssym.spill.dump(); cout<<s(x)<<endl; cout<<s(y)<<endl;
        cout<<" reg 1 setREG(false) register used for other symbol"<<endl;
        cout<<" reg 2 setREG(true) register symbol value changed"<<endl;
        s(1).setREG(false);     // reg repurposed (~R OLDREG)
        s(2).setREG(true);      // reg-symbol modified
        /*assert( s(1).wasREG() );*/
        assert( s(x).getMEM() && !s(x).getStale() );
        assert( s(y).getREG() );
        assert( s(y).getMEM() && s(y).getStale() );
        ssym.spill.dump(); cout<<s(x)<<endl; cout<<s(y)<<endl;
        cout<<" reg 1 and 2 setREG(false).setMEM(false)";
        //s(1).setREG(false).setMEM(false);
        //s(2).setREG(false).setMEM(false);
        // order does not matter here.
        s(x).setMEM(false).setREG(false);
        s(y).setMEM(false).setREG(false);
        ssym.spill.dump(); cout<<s(1)<<endl; cout<<s(2)<<endl;
        assert(ssym.spill.getBottom() == -16); // two 8-byte holes at end
        cout<<endl;
    }
}
#endif // STAGE >= 1
#if STAGE >= 2
void Tester::test2(){
    typedef DemoSymbStates Ssym;
    //typedef Ssym::Psym Psym;
    TEST("respill, no gc (re-use existing mem slots)");
    {
        Ssym ssym;
        // Shortcuts:  s(1) : ref to symbol object 1
        //         spill(1) : spill symbol 1
        //           dump() : print spill region symbol assignments
        // p(id,byte,align) : construct
        typedef Ssym::Psym S; // Symbol object
        auto s=[&ssym](unsigned symId)->S&  {return ssym.fpsym(symId);};
        auto spill=[&ssym](unsigned symId)  {ssym.spill.spill(symId);};
        auto dump=[&ssym]()->void           {return ssym.spill.dump();};
        //auto p = [&ssym](unsigned symId, int byte, int align)->Ssym::Psym{
        //    return ssym.mkPsym(symId, byte,align); };
        unsigned lastsym = 0;
        auto newscalar=[&ssym,&lastsym](void) { return lastsym=ssym.newsym(8,8);};
        auto newscalarRM=[&ssym,&s,&spill,&lastsym](void) {
            lastsym = ssym.newsym(8,8);
            s(lastsym).setREG(true);
            spill(lastsym);
        };

        newscalar();                    // symId 1 declare
        s(1).setREG(true);              // symId 1 --> value in register
        spill(1);                       // symId 1 --> spill to mem (1 --> RM)
        newscalarRM();                  // symId 2 --> RM (declare, ->REG, ->MEM)
        newscalarRM();                  // symId 3 --> RM
        newscalarRM();                  // symId 4 --> RM
        assert( lastsym == 4 );
        s(1).setREG(true);
        s(2).setREG(true);
        cout<<"\nInitial spill begins with 2 stale slots:"<<endl;
        dump();
        cout<<"\nrespill 2, new symbol 5, and 1:"<<endl;
        cout<<"\nrespill 2: ";
        spill(2);
        assert( s(2).getREG() && s(2).getMEM() );
        dump(); for(unsigned i=1U; i<=4U; ++i) cout<<s(i)<<endl;
        cout<<" new symbol 5";
        newscalarRM(); // spill(5); OHOH, spilled and not stale! maybe should not spill!
        assert( lastsym == 5 );
        assert( s(5).getREG() && s(5).getMEM() );
        dump(); for(unsigned i=1U; i<=5U; ++i) cout<<s(i)<<endl;
        cout<<" respill 1: ";
        spill(1);
        assert( s(1).getREG() && s(1).getMEM() );
        dump(); for(unsigned i=1U; i<=5U; ++i) cout<<s(i)<<endl;
        cout<<endl;
        THROW_UNLESS( chk_order(ssym,{1,2,3,4,5}), "Unexpected Spill Region Order" );
        try{
            THROW_UNLESS( chk_order(ssym,{1,1,1,1,1}), "Unexpected Spill Region Order" );
        }catch(std::exception const& e){
            cout<<"Expected this: "<<e.what();
            cout<<"Good: caught exception when chk_order did not match:"<<endl;
        }
        cout<<"Continuing..."<<endl;
    }
    TEST("gc a begin-spill hole and reuse");
    {
        Ssym ssym;
        // Shortcuts: s(1) : ref to symbol object 1
        //        spill(1) : spill symbol 1
        //          dump() : print spill region symbol assignments
        typedef Ssym::Psym S; // Symbol object
        auto s=[&ssym](unsigned symId)->S&  {return ssym.fpsym(symId);}; // S& version exposed for testing
        auto spill=[&ssym](unsigned symId)  {ssym.spill.spill(symId);};
        auto dump=[&ssym]()->void           {return ssym.spill.dump();};
        unsigned lastsym = 0;
        //auto newscalar=[&ssym,&lastsym](void) { return lastsym=ssym.newsym(8,8);};
        auto newscalarRM=[&ssym,&s,&spill,&lastsym](void) {
            lastsym = ssym.newsym(8,8);
            s(lastsym).setREG(true);
            spill(lastsym);
        };

        newscalarRM();                 // symId 1 --> RM (declare, ->REG, ->MEM)
        newscalarRM();                 // symId 2 --> RM
        newscalarRM();                 // symId 3 --> RM
        newscalarRM();                 // symId 4 --> RM
        assert( lastsym == 4 );
        dump();
        s(1).setREG(true);      // Note: setREG here indicates that 'R'
        s(2).setREG(true);      // valued changed.  'M' now staleness.
        ssym.spill.gc();        // forget all stale Spill assignments.
        cout<<"\nInitial spill with 1st 2 of 4 spill gc'ed hole:"<<endl;
        dump();
        assert( distance(ssym.spill.regions().begin(), ssym.spill.regions().end()) == 2 );
        THROW_UNLESS( chk_order(ssym,{3,4}), "Unexpected Spill Region Order" );
        cout<<"\nspill 1, new symbol 5, and 2:"<<endl;
        spill(1);
        assert( distance(ssym.spill.regions().begin(), ssym.spill.regions().end()) == 3 );
        cout<<"\nNote: too-large hole puts 1 into lowest compatible slot in the hole (posn 2)"<<endl;
        assert( s(1).getREG() && s(1).getMEM() );
        dump(); for(unsigned i=1U; i<=4U; ++i) cout<<s(i)<<endl;
        cout<<"\nspill new 5:"<<endl;
        newscalarRM(); // spill(5); OHOH, spilled and not stale! maybe should not spill!
        assert( distance(ssym.spill.regions().begin(), ssym.spill.regions().end()) == 4 );
        THROW_UNLESS( chk_order(ssym,{5,1,3,4}), "Unexpected Spill Region Order" );
        assert( s(5).getREG() && s(5).getMEM() );
        dump(); for(unsigned i=1U; i<=5U; ++i) cout<<s(i)<<endl;
        cout<<"\nrespill 2"<<endl;
        spill(2);
        assert( s(2).getREG() && s(2).getMEM() );
        dump(); for(unsigned i=1U; i<=5U; ++i) cout<<s(i)<<endl;
        cout<<endl;
        assert( distance(ssym.spill.regions().begin(), ssym.spill.regions().end()) == 5 );
        THROW_UNLESS( chk_order(ssym,{5,1,3,4,2}), "Unexpected Spill Region Order" );
    }
    TEST("gc a mid-spill hole and reuse");
    {
        Ssym ssym;
        // Shortcuts: s(1) : ref to symbol object 1
        //        spill(1) : spill symbol 1
        //          dump() : print spill region symbol assignments
        typedef Ssym::Psym S; // Symbol object
        auto s=[&ssym](unsigned symId)->S&  {return ssym.fpsym(symId);};
        auto spill=[&ssym](unsigned symId)  {ssym.spill.spill(symId);};
        auto dump=[&ssym]()->void           {return ssym.spill.dump();};
        unsigned lastsym = 0;
        //auto newscalar=[&ssym,&lastsym](void) { return lastsym=ssym.newsym(8,8);};
        auto newscalarRM=[&ssym,&s,&spill,&lastsym](void) {
            lastsym = ssym.newsym(8,8);
            s(lastsym).setREG(true);
            spill(lastsym);
        };

        newscalarRM();                 // symId 1 --> RM (declare, ->REG, ->MEM)
        newscalarRM();                 // symId 2 --> RM
        newscalarRM();                 // symId 3 --> RM
        newscalarRM();                 // symId 4 --> RM
        assert( lastsym == 4 );
        s(2).setREG(true);      // This time we will create stale memory
        s(3).setREG(true);      // for 2nd and 3rd spilled registers.
        ssym.spill.gc();
        cout<<" Initial spill with 2nd and 3rd of 4 spill gc'ed hole:"<<endl;
        dump();
        cout<<"\nspill 2, new symbol 5, and 1:"<<endl;
        spill(3);       // Note: re-uses old MEM slot (symbol is ~M, but spill remembers where symbol used to be)
        newscalarRM(); // spill(5); OHOH, spilled and not stale! maybe should not spill!
        spill(2);
        dump(); for(unsigned i=1U; i<=5U; ++i) cout<<s(i)<<endl;
        cout<<endl;
        THROW_UNLESS( chk_order(ssym,{1,5,3,4,2}), "Unexpected Spill Region Order" );
    }
    TEST("gc mid- and end-spill holes and reuse");
    {
        Ssym ssym;
        // Shortcuts: s(1) : ref to symbol object 1
        //        spill(1) : spill symbol 1
        //          dump() : print spill region symbol assignments
        typedef Ssym::Psym S; // Symbol object
        auto s=[&ssym](unsigned symId)->S&  {return ssym.fpsym(symId);};
        auto spill=[&ssym](unsigned symId)  {ssym.spill.spill(symId);};
        auto dump=[&ssym]()->void           {return ssym.spill.dump();};
        unsigned lastsym = 0;
        auto newscalar=[&ssym,&lastsym](void) { return lastsym=ssym.newsym(8,8);};
        //auto newscalarRM=[&ssym,&s,&spill,&lastsym](void) {
        //    lastsym = ssym.newsym(8,8);
        //    s(lastsym).setREG(true);
        //    spill(lastsym);
        //};

        newscalar();                   // symId 1
        s(1).setREG(true);              // say value is "in register"
        spill(1);                       // 1-->RM (register+memory)
        newscalar();                   // symId 2
        s(2).setREG(true);              // 2-->R
        spill(2);                       // 2-->RM
        newscalar(); s(3).setREG(true); spill(3);    // 3--> RM
        newscalar(); s(4).setREG(true); spill(4);    // 4--> RM
        s(2).setREG(true);
        s(4).setREG(true);
        cout<<"\nAdd syms 1234, spill them, modify reg value of 2 & 4: "<<endl;
        dump();
        ssym.spill.gc(); for(unsigned i=1U; i<=4U; ++i) cout<<s(i)<<endl;
        cout<<"\nssym.spill.gc() :"<<endl;
        dump();
        cout<<"\nspill 4 (into first hole) :"<<endl;
        spill(4);
        cout<<"\nspill 2 (into next hole) :"<<endl;
        spill(2);
        cout<<"\nspill new 5 (expand spill size) :"<<endl;
        newscalar(); s(5).setREG(true); spill(5);    // 5--> RM
        dump();
        cout<<endl;
        THROW_UNLESS( chk_order(ssym,{1,4,3,2,5}), "Unexpected Spill Region Order" );
    }
}
#endif
#if STAGE>=3
void Tester::test3(){
    typedef DemoSymbStates Ssym;
    //typedef Ssym::Psym Psym;
    TEST("continually use regs in machine with only 4 registers");
    {
        Ssym ssym;
        // Shortcuts: s(1) : ref to symbol object 1
        //        spill(1) : spill symbol 1
        //          dump() : print spill region symbol assignments
        typedef Ssym::Psym S; // Symbol object
        auto s=[&ssym](unsigned symId)->S&  {return ssym.fpsym(symId);};
        //auto spill=[&ssym](unsigned symId)  {ssym.spill.spill(symId);};
        auto dump=[&ssym]()->void           {return ssym.spill.dump();};
        unsigned lastsym = 0;
        auto newscalar=[&ssym,&lastsym](void) { return lastsym=ssym.newsym(8,8);};

        for(unsigned i=1U; i<=10U; ++i) newscalar(); // DECLARE 10 scalars
        //cout<<"\nsyms{"; for(auto symid: ssym.symIdAnyScopeSorted()){cout<<" "<<symid;} cout<<"}"<<endl;
        cout<<"\nsyms{"; for(auto symid: ssym.symIdAnyScopeSorted()){cout<<"\n\t"<<symid<<"\t"<<s(symid);} cout<<"\n}"<<endl;

        for(unsigned i=1U; i<=10U; ++i){
            cout<<"\nUse symbol "<<i<<endl;     // assign register to 'i'
            ssym.use4(i);                       // (max 4 registers)
            cout<<"ssym.prtCurrentSymbols():   ";
            ssym.prtCurrentSymbols();
            cout<<endl; cout.flush();
            cout<<"\nsyms{"; for(unsigned i=1U; i<=10U; ++i){cout<<"\n\t"<<i<<"\t"<<s(i);} cout<<"\n}"<<endl;
            dump();
        }
        cout<<"All symbols:"<<endl;
        for(unsigned i=1U; i<=10U; ++i){
            cout<<s(i)<<endl;
        }
        // last 4 register assignments should be in register, unspilled
        assert( ssym.spill.getBottom() == -6*8 );
        assert( ssym.regOnly() == 4U );
        assert( ssym.memOnly() == 6U );
        cout<<" WARNING: s(1).setREG() should really happen during some 'unspill' routine,"<<endl;
        cout<<"   where unspill might also need to spill a register."<<endl;
        cout<<"   Actually a logical clock might select LRU registers during"<<endl;
        cout<<"   spill/unspill operations."<<endl;
        s(1).setREG(); // permitted, but you want this to happen only via 'unspill(1)' !!!
        assert( s(1).getREG() && s(1).getMEM() && s(1).getStale() );
        THROW_UNLESS( chk_order(ssym,{1,2,3,4,5,6}), "Unexpected Spill Region Order" );
    }
    TEST("simulate out-of-scope, no gc (likely not what you want)");
    {
        Ssym ssym;
        // Shortcuts: s(1) : ref to symbol object 1
        //        spill(1) : spill symbol 1
        //          dump() : print spill region symbol assignments
        typedef Ssym::Psym S; // Symbol object
        auto s=[&ssym](unsigned symId)->S&  {return ssym.fpsym(symId);};
        //auto spill=[&ssym](unsigned symId)  {ssym.spill.spill(symId);};
        auto dump=[&ssym]()->void           {return ssym.spill.dump();};
        unsigned lastsym = 0;
        auto newscalar=[&ssym,&lastsym](void) { return lastsym=ssym.newsym(8,8);};
        for(unsigned i=1U; i<=10U; ++i) newscalar(); // DECLARE 10 scalars
        for(unsigned i=1U; i<=10U; ++i){
            ssym.use4(i);                       // (max 4 registers)
        }
        cout<<"\n10 scalars, used in order, w/ only 4 registers available"<<endl;
        cout<<"\n\tAll symbols:"<<endl; for(unsigned i=1U; i<=10U; ++i) cout<<"\t"<<s(i)<<endl;
        dump();
        THROW_UNLESS( chk_order(ssym,{1,2,3,4,5,6}), "Unexpected Spill Region Order" );

        cout<<" 5 and 8 forgotten (but still active)"<<endl;
        s(5).setREG(false).setMEM(false);
        s(8).setREG(false).setMEM(false);
        dump();
        cout<<"All symbols:"<<endl; for(unsigned i=1U; i<=10U; ++i) cout<<s(i)<<endl;

        cout<<"delsym {5,6,7,8}, without gc"<<endl;
#if 0 // These are now private
        s(5).setActive(false);
        ssym.setActive(6,false);
        ssym.setActive(7,false);
        ssym.setActive(8,false);
#else // the accepted was to deactivate before end_scope is delsym
        ssym.delsym(5);
        ssym.delsym(6);
        ssym.delsym(7);
	ssym.delsym(8);
        // net effect similar to setActive(symid,false)
#endif
        //ssym.spill.gc();
        cout<<"\n\tAll symbols B:"<<endl; for(unsigned i=1U; i<=10U; ++i) cout<<"\t"<<s(i)<<endl;
        dump();

        cout<<"add 5 new registers"<<endl;
        for(unsigned i=11U; i<=15U; ++i) {newscalar();assert(lastsym==i);} // DECLARE 5 more scalars
        for(unsigned i=11U; i<=15U; ++i){
            ssym.use4(i);                       // (max 4 registers)
        }
        dump();
        cout<<"\n\tAll symbols C:"<<endl; for(unsigned i=1U; i<=15U; ++i) cout<<"\t"<<s(i)<<endl;
        THROW_UNLESS( chk_order(ssym,{1,2,3,4,5,6,9,10,11}), "Unexpected Spill Region Order" );
    }
    TEST("simulate out-of-scope, with gc");
    {
        Ssym ssym;
        // Shortcuts: s(1) : ref to symbol object 1
        //        spill(1) : spill symbol 1
        //          dump() : print spill region symbol assignments
        typedef Ssym::Psym S; // Symbol object
        auto s=[&ssym](unsigned symId)->S&  {return ssym.fpsym(symId);};
        //auto spill=[&ssym](unsigned symId)  {ssym.spill.spill(symId);};
        auto dump=[&ssym]()->void           {return ssym.spill.dump();};
        unsigned lastsym = 0;
        auto newscalar=[&ssym,&lastsym](void) { return lastsym=ssym.newsym(8,8);};

        for(unsigned i=1U; i<=10U; ++i) newscalar(); // DECLARE 10 scalars
        for(unsigned i=1U; i<=10U; ++i){
            ssym.use4(i);                       // (max 4 registers)
        }
        dump();
        cout<<"All symbols:"<<endl; for(unsigned i=1U; i<=10U; ++i) cout<<s(i)<<endl;

        cout<<" 5 and 8 symbols forgotten (but still active)"<<endl;
        s(5).setREG(false).setMEM(false);
        s(8).setREG(false).setMEM(false);
        dump();
        cout<<"All symbols:"<<endl; for(unsigned i=1U; i<=10U; ++i) cout<<s(i)<<endl;

        cout<<"Out-of-scope {5,6,7,8}, and gc"<<endl;
        ssym.delsym(5);
        ssym.delsym(6);
        ssym.delsym(7);
	ssym.delsym(8);
        dump();
        ssym.spill.gc(); // <-- this now allows MEM posns of 5, 6 to be re-used
        dump();
        cout<<"All symbols:"<<endl; for(unsigned i=1U; i<=10U; ++i) cout<<s(i)<<endl;

        cout<<"add 5 new registers"<<endl;
        for(unsigned i=11U; i<=15U; ++i){newscalar();assert(lastsym==i);} // DECLARE 5 more scalars
        for(unsigned i=11U; i<=15U; ++i){
            ssym.use4(i);                       // (max 4 registers)
        }
        dump();
        cout<<"All symbols:"<<endl; for(auto s: ssym.getSyms()) cout<<s.second<<endl;
        // 5 and 6 were gc'ed, 7 and 8 were in-reg when went out of scope...
        THROW_UNLESS( chk_order(ssym,{1,2,3,4,9,10,11}), "Unexpected Spill Region Order" );
        assert( ssym.spill.getBottom() == -56 );
    }
}
#endif //STAGE >= 3

int main(int,char**){
#if 0
    assert( ve::alignup(0,0x03) == 0 );
    assert( ve::alignup(1,0x03) == 4 );
    assert( ve::alignup(2,0x03) == 4 );
    assert( ve::alignup(3,0x03) == 4 );
    assert( ve::alignup(4,0x03) == 4 );
    assert( ve::alignup(5,0x03) == 8 );
    assert( ve::alignup(6,0x03) == 8 );
    assert( ve::alignup(7,0x03) == 8 );
    assert( ve::alignup(8,0x03) == 8 );
    assert( ve::alignup(9,0x03) == 12 );
    assert( ve::alignup(-0,0x03) == 0 );
    assert( ve::alignup(-1,0x03) == 0 );
    assert( ve::alignup(-2,0x03) == 0 );
    assert( ve::alignup(-3,0x03) == 0 );
    assert( ve::alignup(-4,0x03) == -4 );
    assert( ve::alignup(-5,0x03) == -4 );
    assert( ve::alignup(-6,0x03) == -4 );
    assert( ve::alignup(-7,0x03) == -4 );
    assert( ve::alignup(-8,0x03) == -8 );
    assert( ve::alignup(-9,0x03) == -8 );
#endif
#if STAGE >= 1
    cout<<"======== test1() ==========="<<endl;
    Tester::test1();
    //Tester t;
    //t.test1();
#endif
#if STAGE >= 2
    cout<<"======== test2() ==========="<<endl;
    Tester::test2();
#endif
#if STAGE >= 3
    cout<<"======== test3() ==========="<<endl;
    Tester::test3();
#endif
    cout<<"\nGoodbye - " __FILE__ " ran "<<testNum<<" tests"<<endl;
    return 0;
}
// vim: ts=4 sw=4 et cindent cino=^=l0,\:.5s,=-.5s,N-s,g.5s,h.5s,b1 cinkeys=0{,0},0),\:,0#,!^F,o,O,e,0=break
