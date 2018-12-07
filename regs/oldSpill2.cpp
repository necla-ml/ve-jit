
#include "spillable-base.hpp"
#include "old/spill-impl.hpp"
#include "throw.hpp"
#include <cassert>
#include <iostream>
#include <map>
using namespace std;

class SpillableSym;
class DemoSymbStates;

struct Tester{
    static void test1();
    static void test2();
    static void test3();
};

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
 * For test purposes, we don't need any extras,
 * like preserved/reserved registers, 
 * or actual register mappings.
 *
 * An actual "Parent" symbol class would add more features!
 */
class SpillableSym : public SpillableBase
{
  public:
      friend class Tester;

      SpillableSym(/*DemoSymbStates *ssym,*/ unsigned const uid, int bytes, int align)
          : SpillableBase(uid,bytes,align) /*, ssym(ssym), staleness(0)*/ {}

      // /** map Region::symId to SpillableSym */
      // DemoSymbStates *ssym;
      //----
      //void scope_enter() {assert(getActive()==false); setActive(true);}
      //void scope_exit() {setActive(false);}
      //bool isSpillable() const { return (REG&getWhere()); }
      //Where w(int const i) const {return static_cast<Where>(i); }
};

/** Spillable objects need a way to convert symbol Ids into SpillableSym */
struct DemoSymbStates {
    typedef SpillableSym Psym;  // "parent" symbol class
    map<unsigned,Psym> psyms;   // symId --> symbol info

    DemoSymbStates()
        : psyms(), spill(this) {}
    /** The spill manager lifetime should be less than the psyms map lifetime */
    ve::Spill<DemoSymbStates> spill;

    Psym const& psym(unsigned const symId) const {
        auto found = psyms.find(symId);
        if( found == psyms.end() )
            throw("psym(symId) doesn't exist");
        return found->second;
    }
    Psym & psym(unsigned const symId) {     
        auto found = psyms.find(symId);
        if( found == psyms.end() )
            throw("psym(symId) doesn't exist");
        return found->second;
    }
    /** declare a symbol. It is active, but in neither REG nor MEM. */
    void add(Psym const& s){
        assert( psyms.find(s.uid) == psyms.end() );
        psyms.emplace( s.uid, s );
    }
    /** say symbol is in reg, but we only have 4 registers available.
     * Whenever we \c use4, we spill the first encountered not-already-spilled
     * symbol to memory (marking its register value as stale). */
    void use4(unsigned const symId){
        Psym &s = psym(symId);
        if( s.getREG() ){
            cout<<" use4:mod"<<symId<<" "; cout.flush();
            s.setREG(true); // in-register symbol modified its value.
            return;
        }
        auto nRegs = count_if(psyms.begin(), psyms.end(),
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
        auto rm = find_if( psyms.begin(), psyms.end(),
                [](map<unsigned,Psym>::value_type const& v)
                { return v.second.getREG() && v.second.getMEM() && v.second.getStale()==0; } );
        if( rm != psyms.end() ){
            cout<<" use4:easy"<<symId<<" "; cout.flush();
            assert( rm->second.getMEM() && rm->second.getStale()==0 );
            // <update machine register mapping table HERE>
            rm->second.setREG(false);
            s.setREG(true);
            return;
        }
        // else Find some register symbol and spill (or re-spill) it to memory
        auto r = find_if( psyms.begin(), psyms.end(),
                [](map<unsigned,Psym>::value_type const& v)
                { return v.second.getREG(); } );
        assert( r !=  psyms.end() );
        cout<<"< use4:(spill "<<r->first<<"):"<<symId<<" "; cout.flush();
        spill.spill(r->first);          // 'r' --> RM
        // 's' takes the register away from prev symbol 'r'
        r->second.setREG(false);        // 'r' --> ~RM
        s.setREG(true);                 // 's' --> R
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

void Tester::test1(){
    typedef DemoSymbStates Ssym;
    typedef Ssym::Psym Psym;
    TEST("Construct some symbols");
    {
        Ssym ssym;
        ASSERTTHROW( ssym.psym(1U).getActive() );
        ssym.add(Psym(1U,8,8));  // symId 1, len=8, align=8 (64-bit register)
        assert( ssym.psym(1U).uid == 1U );
        ssym.add(Psym(2U,8,8));  // symId 1, len=8, align=8 (64-bit register)
        assert( ssym.psym(2U).uid == 2U );
        cout<<ssym.psym(1U)<<endl;
        cout<<ssym.psym(2U)<<endl;
        ssym.psym(1).setREG(true);
        assert( ssym.psym(1).getREG() == true );
        ssym.psym(1).setREG(false);
        assert( ssym.psym(1).getREG() == false );
        ssym.psym(1).setREG(true);
        assert( ssym.psym(1).getREG() == true );

        ssym.psym(1).setMEM(true);
        assert( ssym.psym(1).getMEM() == true );
        assert( ssym.psym(1).getREG() == true );
        ssym.psym(1).setMEM(false);
        assert( ssym.psym(1).getMEM() == false );
        assert( ssym.psym(1).getREG() == true );
        ssym.psym(1).setMEM(true);
        assert( ssym.psym(1).getMEM() == true );
        assert( ssym.psym(1).getREG() == true );
        cout<<endl;
    }
    TEST("Construct and spill 2 [declared, unused] symbols");
    {
        Ssym ssym;
        ASSERTTHROW( ssym.psym(1U).getActive() );
        ssym.add(Psym(1U,8,8));  // symId 1, len=8, align=8 (64-bit register)
        assert( ssym.psym(1U).uid == 1U );
        ssym.add(Psym(2U,8,8));  // symId 2, len=8, align=8 (64-bit register)
        assert( ssym.psym(2U).uid == 2U );
        ssym.spill.spill(1);
        ssym.spill.spill(2);
        cout<<endl;
    }
    TEST("Construct and spill 2 [declared, used] symbols");
    {
        Ssym ssym;
        auto s=[&ssym](unsigned symId)->Ssym::Psym &
        {return ssym.psym(symId);};
        ASSERTTHROW( s(1) );
        ssym.add(Psym(1,8,8));  // symId 1
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
        ssym.add(Psym(2,8,8));  // symId 2
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
        auto s=[&ssym](unsigned symId)->Ssym::Psym &
        {return ssym.psym(symId);};

        ssym.add(Psym(1,8,8));  // symId 1
        s(1).setREG(true);      // say value is "in register"
        ssym.spill.spill(1);
        ssym.add(Psym(2,8,8));  // symId 2
        s(2).setREG(true);      // say value is "in register"
        ssym.spill.spill(2);
        ssym.spill.dump(); cout<<s(1)<<endl; cout<<s(2)<<endl;
        cout<<" reg 1 setREG(false) register used for other symbol"<<endl;
        cout<<" reg 2 setREG(true) register symbol value changed"<<endl;
        s(1).setREG(false);     // reg repurposed (~R OLDREG)
        s(2).setREG(true);      // reg-symbol modified
        /*assert( s(1).wasREG() );*/
        assert( s(1).getMEM() && !s(1).getStale() );
        assert( s(2).getREG() );
        assert( s(2).getMEM() && s(2).getStale() );
        ssym.spill.dump(); cout<<s(1)<<endl; cout<<s(2)<<endl;
        cout<<" reg 1 and 2 setREG(false).setMEM(false)";
        //s(1).setREG(false).setMEM(false);
        //s(2).setREG(false).setMEM(false);
        // order does not matter here.
        s(1).setMEM(false).setREG(false);
        s(2).setMEM(false).setREG(false);
        ssym.spill.dump(); cout<<s(1)<<endl; cout<<s(2)<<endl;
        cout<<endl;
    }
}

void Tester::test2(){
    typedef DemoSymbStates Ssym;
    typedef Ssym::Psym Psym;
    TEST("respill, no gc (re-use existing mem slots)");
    {
        Ssym ssym;
        // Shortcuts: s(1) : ref to symbol object 1
        //        spill(1) : spill symbol 1
        //          dump() : print spill region symbol assignments
        typedef Ssym::Psym S; // Symbol object
        auto s=[&ssym](unsigned symId)->S&  {return ssym.psym(symId);};
        auto spill=[&ssym](unsigned symId)  {ssym.spill.spill(symId);};
        auto dump=[&ssym]()->void           {return ssym.spill.dump();};
        auto newscalar=[&ssym](unsigned id) {return ssym.add(Psym(id,8,8));};
        auto newscalarRM=[&ssym,&s,&spill](unsigned id) {
            ssym.add(Psym(id,8,8));
            s(id).setREG(true);
            spill(id);
        };

        newscalar(1);                   // symId 1 declare
        s(1).setREG(true);              // symId 1 --> value in register
        spill(1);                       // symId 1 --> spill to mem (1 --> RM)
        newscalarRM(2);                 // symId 2 --> RM (declare, ->REG, ->MEM)
        newscalarRM(3);                 // symId 3 --> RM
        newscalarRM(4);                 // symId 4 --> RM
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
        newscalarRM(5); // spill(5); OHOH, spilled and not stale! maybe should not spill!
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
            cout<<"Good: caught exception when chk_order did not match:"<<endl;
            cout<<e.what()<<endl;
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
        auto s=[&ssym](unsigned symId)->S&  {return ssym.psym(symId);};
        auto spill=[&ssym](unsigned symId)  {ssym.spill.spill(symId);};
        auto dump=[&ssym]()->void           {return ssym.spill.dump();};
        //auto newscalar=[&ssym](unsigned id) {return ssym.add(Psym(id,8,8));};
        auto newscalarRM=[&ssym,&s,&spill](unsigned id) {
            ssym.add(Psym(id,8,8));
            s(id).setREG(true);
            spill(id);
        };

        newscalarRM(1);                 // symId 1 --> RM (declare, ->REG, ->MEM)
        newscalarRM(2);                 // symId 2 --> RM
        newscalarRM(3);                 // symId 3 --> RM
        newscalarRM(4);                 // symId 4 --> RM
        s(1).setREG(true);
        s(2).setREG(true);
        ssym.spill.gc();
        cout<<"\nInitial spill with 2-scalar begin-spill gc'ed hole:"<<endl;
        dump();
        cout<<"\nspill 1, new symbol 5, and 2:"<<endl;
        spill(1);
        cout<<"\nNote: too-large hole puts 1 into lowest compatible slot in the hole (posn 2)"<<endl;
        assert( s(1).getREG() && s(1).getMEM() );
        dump(); for(unsigned i=1U; i<=4U; ++i) cout<<s(i)<<endl;
        cout<<"\nspill new 5:"<<endl;
        newscalarRM(5); // spill(5); OHOH, spilled and not stale! maybe should not spill!
        assert( s(5).getREG() && s(5).getMEM() );
        dump(); for(unsigned i=1U; i<=5U; ++i) cout<<s(i)<<endl;
        cout<<"\nrespill 2"<<endl;
        spill(2);
        assert( s(2).getREG() && s(2).getMEM() );
        dump(); for(unsigned i=1U; i<=5U; ++i) cout<<s(i)<<endl;
        cout<<endl;
        THROW_UNLESS( chk_order(ssym,{5,1,3,4,2}), "Unexpected Spill Region Order" );
    }
    TEST("gc a mid-spill hole and reuse");
    {
        Ssym ssym;
        // Shortcuts: s(1) : ref to symbol object 1
        //        spill(1) : spill symbol 1
        //          dump() : print spill region symbol assignments
        typedef Ssym::Psym S; // Symbol object
        auto s=[&ssym](unsigned symId)->S&  {return ssym.psym(symId);};
        auto spill=[&ssym](unsigned symId)  {ssym.spill.spill(symId);};
        auto dump=[&ssym]()->void           {return ssym.spill.dump();};
        //auto newscalar=[&ssym](unsigned id) {return ssym.add(Psym(id,8,8));};
        auto newscalarRM=[&ssym,&s,&spill](unsigned id) {
            ssym.add(Psym(id,8,8));
            s(id).setREG(true);
            spill(id);
        };

        newscalarRM(1);                 // symId 1 --> RM (declare, ->REG, ->MEM)
        newscalarRM(2);                 // symId 2 --> RM
        newscalarRM(3);                 // symId 3 --> RM
        newscalarRM(4);                 // symId 4 --> RM
        s(2).setREG(true);
        s(3).setREG(true);
        ssym.spill.gc();
        cout<<" Initial spill with 2-scalar begin-spill gc'ed hole:"<<endl;
        dump();
        cout<<"\nspill 2, new symbol 5, and 1:"<<endl;
        spill(3);       // Note: re-uses old MEM slot (symbol is ~M, but spill remembers where symbol used to be)
        newscalarRM(5); // spill(5); OHOH, spilled and not stale! maybe should not spill!
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
        auto s=[&ssym](unsigned symId)->S&  {return ssym.psym(symId);};
        auto spill=[&ssym](unsigned symId)  {ssym.spill.spill(symId);};
        auto dump=[&ssym]()->void           {return ssym.spill.dump();};
        auto newscalar=[&ssym](unsigned id) {return ssym.add(Psym(id,8,8));};

        newscalar(1);                   // symId 1
        s(1).setREG(true);              // say value is "in register"
        spill(1);                       // 1-->RM (register+memory)
        newscalar(2);                   // symId 2
        s(2).setREG(true);              // 2-->R
        spill(2);                       // 2-->RM
        newscalar(3); s(3).setREG(true); spill(3);    // 3--> RM
        newscalar(4); s(4).setREG(true); spill(4);    // 4--> RM
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
        newscalar(5); s(5).setREG(true); spill(5);    // 5--> RM
        dump();
        cout<<endl;
        THROW_UNLESS( chk_order(ssym,{1,4,3,2,5}), "Unexpected Spill Region Order" );
    }
}
void Tester::test3(){
    typedef DemoSymbStates Ssym;
    typedef Ssym::Psym Psym;
    TEST("continually use regs in machine with only 4 registers");
    {
        Ssym ssym;
        // Shortcuts: s(1) : ref to symbol object 1
        //        spill(1) : spill symbol 1
        //          dump() : print spill region symbol assignments
        typedef Ssym::Psym S; // Symbol object
        auto s=[&ssym](unsigned symId)->S&  {return ssym.psym(symId);};
        //auto spill=[&ssym](unsigned symId)  {ssym.spill.spill(symId);};
        auto dump=[&ssym]()->void           {return ssym.spill.dump();};
        auto newscalar=[&ssym](unsigned id) {return ssym.add(Psym(id,8,8));};
        //auto newscalarRM=[&ssym,&s,&spill](unsigned id) {
        //    ssym.add(Psym(id,8,8));
        //    s(id).setREG(true);
        //    spill(id);
        //};
        for(unsigned i=1U; i<=10U; ++i) newscalar(i); // DECLARE 10 scalars
        for(unsigned i=1U; i<=10U; ++i){
            cout<<"\nUse symbol "<<i<<endl;     // assign register to 'i'
            ssym.use4(i);                       // (max 4 registers)
            cout<<"\nsyms{"; for(unsigned i=1U; i<=10U; ++i){cout<<"\n\t"<<i<<"\t"<<s(i);} cout<<"\n}"<<endl;
            dump();
        }
        cout<<"All symbols:"<<endl;
        for(unsigned i=1U; i<=10U; ++i){
            cout<<s(i)<<endl;
        }
        // last 4 register assignments should be in register, unspilled
        assert( ssym.spill.getBottom() == -6*8 );
        auto regOnly = count_if( ssym.psyms.begin(), ssym.psyms.end(),
                [](map<unsigned,Psym>::value_type const& v)
                { return v.second.getREG() && !v.second.getMEM(); });
        assert( regOnly == 4U );
        auto memOnly = count_if( ssym.psyms.begin(), ssym.psyms.end(),
                [](map<unsigned,Psym>::value_type const& v)
                { return !v.second.getREG() && v.second.getMEM() && v.second.getStale()==0; });
        assert( memOnly == 6U );
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
        auto s=[&ssym](unsigned symId)->S&  {return ssym.psym(symId);};
        //auto spill=[&ssym](unsigned symId)  {ssym.spill.spill(symId);};
        auto dump=[&ssym]()->void           {return ssym.spill.dump();};
        auto newscalar=[&ssym](unsigned id) {return ssym.add(Psym(id,8,8));};
        for(unsigned i=1U; i<=10U; ++i) newscalar(i); // DECLARE 10 scalars
        for(unsigned i=1U; i<=10U; ++i){
            ssym.use4(i);                       // (max 4 registers)
        }
        dump();
        cout<<"All symbols:"<<endl; for(unsigned i=1U; i<=10U; ++i) cout<<s(i)<<endl;

        cout<<" 5 and 8 symbols forgotten (but still active)"<<endl;
        s(5).setREG(false).setMEM(false);
        dump();
        cout<<"All symbols:"<<endl; for(unsigned i=1U; i<=10U; ++i) cout<<s(i)<<endl;

        cout<<"Out-of-scope {5,6,7,8}, without gc"<<endl;
        s(5).setActive(false);
        s(6).setActive(false);
        s(7).setActive(false);
        s(8).setActive(false);
        //ssym.spill.gc();
        dump();
        cout<<"All symbols:"<<endl; for(unsigned i=1U; i<=10U; ++i) cout<<s(i)<<endl;

        cout<<"add 5 new registers"<<endl;
        for(unsigned i=21U; i<=25U; ++i) newscalar(i); // DECLARE 5 more scalars
        for(unsigned i=21U; i<=25U; ++i){
            ssym.use4(i);                       // (max 4 registers)
        }
        dump();
        cout<<"All symbols:"<<endl; for(auto s: ssym.psyms) cout<<s.second<<endl;
        THROW_UNLESS( chk_order(ssym,{1,2,3,4,5,6,9,10,21}), "Unexpected Spill Region Order" );
    }
    TEST("simulate out-of-scope, with gc");
    {
        Ssym ssym;
        // Shortcuts: s(1) : ref to symbol object 1
        //        spill(1) : spill symbol 1
        //          dump() : print spill region symbol assignments
        typedef Ssym::Psym S; // Symbol object
        auto s=[&ssym](unsigned symId)->S&  {return ssym.psym(symId);};
        //auto spill=[&ssym](unsigned symId)  {ssym.spill.spill(symId);};
        auto dump=[&ssym]()->void           {return ssym.spill.dump();};
        auto newscalar=[&ssym](unsigned id) {return ssym.add(Psym(id,8,8));};
        for(unsigned i=1U; i<=10U; ++i) newscalar(i); // DECLARE 10 scalars
        for(unsigned i=1U; i<=10U; ++i){
            ssym.use4(i);                       // (max 4 registers)
        }
        dump();
        cout<<"All symbols:"<<endl; for(unsigned i=1U; i<=10U; ++i) cout<<s(i)<<endl;
        THROW_UNLESS( chk_order(ssym,{1,2,3,4,5,6}), "Unexpected Spill Region Order" );

        cout<<" 5 and 8 symbols forgotten (but still active)"<<endl;
        s(5).setREG(false).setMEM(false);
        s(8).setREG(false).setMEM(false);
        dump();
        cout<<"All symbols:"<<endl; for(unsigned i=1U; i<=10U; ++i) cout<<s(i)<<endl;

        cout<<"Out-of-scope {5,6,7,8}, and gc"<<endl;
        s(5).setActive(false);
        s(6).setActive(false);
        s(7).setActive(false);
        s(8).setActive(false);
        dump();
        ssym.spill.gc();
        dump();
        cout<<"All symbols:"<<endl; for(unsigned i=1U; i<=10U; ++i) cout<<s(i)<<endl;

        cout<<"add 5 new registers"<<endl;
        for(unsigned i=21U; i<=25U; ++i) newscalar(i); // DECLARE 5 more scalars
        for(unsigned i=21U; i<=25U; ++i){
            ssym.use4(i);                       // (max 4 registers)
        }
        dump();
        cout<<"All symbols:"<<endl; for(auto s: ssym.psyms) cout<<s.second<<endl;
        // 5 and 6 were gc'ed, 7 and 8 were in-reg when went out of scope...
        THROW_UNLESS( chk_order(ssym,{1,2,3,4,9,10,21}), "Unexpected Spill Region Order" );
    }
}

int main(int,char**){
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
    cout<<"======== test1() ==========="<<endl;
    Tester::test1();
    cout<<"======== test2() ==========="<<endl;
    Tester::test2();
    cout<<"======== test3() ==========="<<endl;
    Tester::test3();
    cout<<"\nGoodbye - " __FILE__ " ran "<<testNum<<" tests"<<endl;
    return 0;
}
// vim: ts=4 sw=4 et cindent cino=^=l0,\:.5s,=-.5s,N-s,g.5s,h.5s,b1 cinkeys=0{,0},0),\:,0#,!^F,o,O,e,0=break
