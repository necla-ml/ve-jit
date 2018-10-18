
#include "spill-impl2.hpp"
#include "symScopeUid.hpp"
#include "scopedSpillableBase.hpp"

#include "reg-aurora.hpp"

#include <cassert>
#include <iostream>
#include <map>
using namespace std;

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

//class SpillableSym; // ScopedSpillableSym is for now hardwired to ParSymbol<ScopedSpillableSym> ?
//namespace scope{
class DemoSymbStates;
//}//scope::

struct Tester{
    static void test1();
    static void test2();
    static void test3();
};

// I think I need to construct Psym first, then invoke RegSymbol(Psym&, ...)
class RegSymbol :
    public scope::ParSymbol<ScopedSpillableBase>
{
  public:
    typedef RegisterBase Rb;
    typedef Rb::Cls      Cls; // Cls::scalar Cls::vector Cls::mask Cls::none
    typedef scope::ParSymbol<ScopedSpillableBase> Psym;
    friend std::ostream& operator<<(std::ostream& os, RegSymbol const& x);
    friend DemoSymbStates;

    // republish...
    typedef Psym::Base Base; /** i.e. ScopedSpillableBase */

    void init(char const* name, uint64_t const tick){
        name_ = name;
        t_decl = tick;
    }

    // create RegSymbol active, but no assigned register
    template<typename... Arg>
    RegSymbol(unsigned symId, unsigned scope
            //,char const* name, uint64_t const tick//, RegId const regid
            , Arg&&... arg // even though always empty...
            )
        : Psym(symId, scope, defBytes(Rb::Cls::scalar), defAlign(Rb::Cls::scalar)),
        name_("huh?"),
        t_decl(0U),
        t_sym(0U),
        regId_(invalidReg())
        { init(arg...); }

    char const* name() const {return name_;}
    RegId regId() const      {return regId_;}
    uint64_t tDecl() const   {return t_decl;}
    uint64_t tSym() const    {return t_sym;} // useful?
  protected:
    RegSymbol& setReg(RegId const rid,uint64_t const tick){         ///< init or copy symbol --> register
        if(valid(rid)){
            regId_ = rid;
            t_sym = tick;
            assert( tick >= t_sym );
            Psym::setREG(true);
        }else{
            regId_ = invalidReg();
            Psym::setREG(false);
        }
    }
    RegSymbol& tRead(uint64_t const tick){      ///< annotate register read event
        assert(getActive() && valid(regId_) && Psym::getREG() && "symbol not in register?");
        assert( tick >= t_sym );
        t_sym = tick;
    }
    RegSymbol& tWrite(uint64_t const tick){     ///< annotate register write event
        assert(getActive() && valid(regId_) && Psym::getREG() && "symbol not in register?");
        assert( tick >= t_sym );
        t_sym = tick;
    }
        
  public:
    template<typename SET>
        void chk_different_name(SET const& psyms) const{
            for( auto psym: psyms ){
                //std::cout<<" psym"<<psym; std::cout.flush();
                if( this->symId() == psym->symId() )
                    continue; // we can never clash with ourself (safety)
                char const* existing_name = psym->name_;
                if( strcmp(this->name_, existing_name) == 0 ){
                    THROW("Symbol "<<name_<<" already exists in current scope");
                }
            }
        }


  private:
    char const* name_;          ///< declared symbol name
    uint64_t t_decl;      ///< declaration time tick
    uint64_t t_sym;             ///< last-used tick
    RegId regId_;               ///< register Id \sa reg-base.hpp
    friend std::ostream& operator<<(std::ostream& os, RegSymbol const& x);
};

std::ostream& operator<<(std::ostream& os, RegSymbol const& x){
    os<<" Rs{"<<x.name_<<",t"<<x.t_decl<<",u"<<x.t_sym<<",r";
    if(x.regId_ == invalidReg()) os<<"?";
    else                         os<<(int)x.regId_;
    os<<"}'"<<dynamic_cast<RegSymbol::Psym const&>(x)<<"'"<<endl;
    return os;
}

/**
 * Regset provides low-level RegId<-->symId pairing.
 *
 * State is synchronized by SYMBSTATES to match individual RegSymbol info.
 * SYMBSTATES::syms maps symId-->RegSymbol,
 * and you use SYMBSTATES::psym(symId) to obtain the RegSymbol.
 *
 * - compare with:
 *   - \c SymbStates (\c DemoSymStates) symId<-->scope (high-level)
 *     - \c SymScopeUid, scope<-->symids (low-level data structure)
 *
 * - Just as a symId maps to a single scope,
 *   - a symId maps to a single RegId.
 *   - a symId can be reassigned to another RegId
 *   - a symId is still 'assigned' to a Reg when it is spilled
 *     - but RegSymbol::getREG() is false. See \ref ScopedSpillableBase.
 *   - a symId can be either \e strong-assigned or \e weak-assigned to reg.
 *     - (just like a symId can be in either \e active or \e stale scope)
 *
 * === \e Strong-assign : symId <--> RegId association
 *
 * - When a RegId \e r is \e write-assigned to a given [new] symId \e s,
 *   all other symids must be dissociated.
 *   - Here:
 *     - \post \c regmap(r) has a \b single entry, \e s.
 *   - Elsewhere:
 *     - other symids get setREG(false).
 *     - if other symids are scope-active, they \b must be spilled.
 *     - if other symids are scope-inactive, that is OK (they may or may
 *       not still be in spill, but can be gc'ed).
 * - A RegId may be write-assigned to the same unique symbol quickly.
 *   - i.e. calculating a symbol value in a number of assembler statements.
 *   - but for some chipsets this might lead to slower code, so...
 * - Consider a symbol \e s with previous register assignment
 *   \f$r_prev(s)\f$that may be \e weak- or \e strong-assigned (or absent).
 * - Suppose \e s is to be \e write-assigned.
 * - We need to allocate and \e strong-assign a register \f$r_new\f$ to \e s:
 * - The steps might be ordered as follows for fully-unrolled jit code:
 *   1. Look for \c mt registers.
 *      - (first, because (3.) \e might in some cases cause pipeline stalls?).
 *      - \e select one, \f$r_new(s)\f$, (staleness? round-robin?)
 *      - Now \f$r_new(s)\f$ is a register with \b no
 *        \e strong- or \e weak-assign symbols.
 *      - Erase \e s's mapping to \f$r_prev(s)\f$
 *      - \e strong-assign \e s to \f$r_new(s)\f$.
 *   2. O/w, if \e s is already \c strong-assigned, use the same register.
 *      - Some chipsets may take a performance hit for this,
 *        but still it is probably better than spilling a register to memory.
 *      - so this step might be skipped.
 *   3. O/w, if \e s is \e weak-assigned to \f$r_prev(s)\f$ and there is no
 *       \e strong-assign for \f$r_prev(s)\f$. Use \f$r_new(s) = r_prev(s)\f$.
 *       - (This \e might cause pipeline stalls, so do it after (3b) for unrolled-jit code)
 *       - Promote \e s's mapping to \f$r_prev(s)\f$ from \e weak- to \e strong-assign.
 *   4. O/w, look for registers with \b no \e strong-assign.  (They must
 *       have \e weak-assigns, because we already looked for empties in (1.))
 *       - \e select one, \f$r_new(s)\f$, (staleness? round-robin?)
 *         - prefer not to re-use \f$r_prev(s)\f$ (differet from (3.)).
 *       - Erase \e s's mapping to \f$r_prev(s)\f$
 *       - \e strong-assign \e s to \f$r_new(s)\f$.
 *   5. O/w, step (2.), if we skipped it before.
 *   6. O/w look at all symIds in \e strong-register assignments
 *      - \e select one, \e x, (staleness? round-robin?)
 *      - spill \e x, if nec., (\ref ScopedSpillableBase),
 *      - demote \e x's previous \e strong-assign to a \e weak-assign,
 *      - and \e strong-assign new symId \e s to register \e x.
 *
 * Issues:
 *
 * - 'stale' for selecting spill symbol is based on symbol usage time
 * - 'stale' for selecting register \e might be based on register usage time.
 *   - for simplicity, \b require:
*      - \e strong-assign registers update the symbol-usage time information.
*      - all assembler \e read ops also update the symbol time.
*    - register staleness is defined as:
*      - the staleness of its unique \e strong-assign symbol.
*      - o/w, the minimum staleness of its \e weak-assign symbol.
*      - o/w the register is \c mt, always preferred for use.
 *
 * === \e Weak-assign : symId <--> RegId association
 *
 * - When a strong RegId association kicks out previous \e strong-assign
 *   symIds, we still remember the last-used register association.
 * - Also when a strong Reg
 *   - Those other symbols have \e strong-assign --> \e weak-assign.
 *   - akin to \c SymScopeUid maintaining \e stale-scope information,
 *     which (at least in the API) could be re-activated.
 *
 */
//template<class SYMBSTATES> // hard-wired to DemoSymbStates
class Regset {
  public:
    typedef RegisterBase Rb;
    typedef DemoSymbStates SYMBSTATES;
    typedef std::unordered_set<unsigned> HashSet;
    friend class SYMBSTATES;
  private:
    decltype(mkChipRegisters()) chipregs; // initialize first!
    SYMBSTATES *ssym;
    std::vector<RegId> scalars;
  protected:
    std::map<RegId, std::forward_list<unsigned>> regmap;
    std::map<symId, RegId> symmap;
    std::vector<RegId> mt;      ///< empty
  public:
    void assign(unsigned const symId, RegId
  public:
    Regset(SYMBSTATES *ssym, int nScalars)
        : chipregs(mkChipRegisters())
          , ssym(ssym)
          , scalars(init_some_scalars(nScalars))
          , regmap(init_regmap())
          , symmap()
          , mt()
    {}
    Regset(SYMBSTATES *ssym)
        : chipregs(mkChipRegisters())
          , ssym(ssym)
          , scalars(init_scalars())
          , regmap(init_regmap())
          , symmap()
          , mt()
    {}

    std::vector<RegId> const&
        Mt() const {
            return mt;}
    std::forward_list<unsigned> const&
        symIds(RegId const r) const {
            assert( valid(r) );
            auto found = regmap.find(r);
            if (found==regmap.end()){
                THROW("Regset::symIds(RegId="<<r<<") does not know about that RegId");
            }
            assert( scalars.find(r) != scalars.end() );
            chk(); }

  private:
    void chk(){
        assert( scalars.size() == regmap.size() );
        assert( mt.size() <= scalars.size() );
        // A symbol can never be in 2 forward_lists,
        // and (stronger) can never occur twice in any list
        std::unordered_set<unsigned> seen;
        for(auto const& m: regmap){
            for(auto const& s: m->second){
                assert( seen.find(s) == seen.end() );
                seen.insert(s);
            }
        }
    }
    auto getFreeScalarRegs(){
        std::vector<uint32_t> ret;
        for(uint32_t i=0U; i<chipregs.size(); ++i){
            RegisterBase const& rb = chipregs(i);
            if( rb.cls() == Rb::Cls::scalar && rb.free()
                    && !isReserved(RegId(i), Abi::c)
                    && !isPreserved(RegId(i), Abi::c) ){
                std::cout<<" Free Scalar Reg: "<<chipregs(i)<<" asmname "<<asmname(rb.rid)<<std::endl;
                std::cout.flush();
                ret.push_back(i);
            }
        }
        return ret;
    }
    auto init_scalars(){
        std::vector<RegId> ret;
        std::vector<uint32_t> const regs = getFreeScalarRegs();
        std::transform( regs.begin(), regs.end(),
                std::back_inserter(ret),
                [](uint32_t const r) {return RegId(r);} );
        return ret;
    }
    auto init_some_scalars(unsigned nScalarRegs){
        std::vector<RegId> ret;
        auto const regs = getFreeScalarRegs();
        for(unsigned cnt=0U; cnt<nScalarRegs && cnt<regs.size(); ++cnt){
            ret.push_back(RegId(regs(cnt)));
        }
        return ret;
    }
    std::map<RegId, std::forward_list<unsigned>> init_regmap(){
        std::map<RegId, std::forward_list<unsigned>> ret;
        for(auto const r: scalars) ret.emplace(r,std::forward_list());
        return ret;
    }
};

class Counters {
  public:
    Counters() : t_(0U) {}
    uint64_t nextTick(){ return ++t_; }
  private:
    uint64_t t_;
};

/** Try to join up some symbol features from regSymbol2.hpp, like named symbols
 * and an easier spill mechanism.
 */
class DemoSymbStates
    : protected scope::SymbStates<RegSymbol>
    , private Counters
{
  public:
    typedef scope::SymbStates<RegSymbol> Ssym;
    //typedef Ssym::Psym Psym;
    typedef RegSymbol Psym;
    typedef RegSymbol Ppsym;
    typedef RegSymbol Rs;

  public: // republish
    auto const& psym(unsigned s) const { return Ssym::psym(s);}
    auto & psym(unsigned s)       { return Ssym::psym(s);}
    auto & fpsym(unsigned s)      { return Ssym::fpsym(s);}

  public: // change!!!
    //template<typename... Arg> inline
        unsigned newsym(
                char const* name
                //, Arg&&... arg
                ) {
            auto const symId = Ssym::newsym( name, nextTick() );
            // this should call Ssym::Psym(symId,scope,name,tick);
            // i.e. RegSymbol constructor
            return symId;
        }


  private:
    // psym/fpsym is ParSymbol<ScopedSpillableBase> with:
    //   ParSymbol:           uid/symId(), scope, getActive,
    //   ScopedSpillableBase: getREG, getMEM
    //   (Spillable:          getBytes, getAlign)
    // rs (RegSymbol) map for:
    //   name, RegId, staleness, 
    //std::map<unsigned, Rs> rsyms;
    // Can we store Rs instead of Base in SymbStates::syms ??
  public:
    Rs const& rsym(unsigned symId);
    Regset<DemoSymbStates> regset; ///< hard-wired to a set of scalar regs now.

  public:
    friend class ve::Spill<DemoSymbStates>;

    DemoSymbStates(unsigned nScalars=4U)
        : Ssym(),
        regset(this),
        spill(this)
    {
        //init_scalars(nScalars);
    }
    ~DemoSymbStates() {
        //Base::end_scope();
    }
  public: // republish some functions
    auto symIdAnyScopeSorted() const { return ssu.symIdAnyScopeSorted(); }
    template<typename... Arg>
        unsigned newsym(Arg&&... arg){
            auto const symId = Ssym::newsym(arg..., nextTick());
            return symId;
        }
  public:
    RegId allocScalar();                    ///< allocate from \c scalars, spill if nec.
    RegId allocScalar(unsigned const symId);///< allocScalar + setReg
    void setReg(unsigned const symId, RegId const rid); ///< copy symbol R-->R
    void read(unsigned const symId);        ///< annotate register read event
    void write(unsigned const symId);       ///< annotate register write event */

  private:
    friend bool chk_order(DemoSymbStates const& ssym, std::initializer_list<unsigned> const& i);
    /** The spill manager lifetime should be less than the psyms map lifetime */
    ve::Spill<DemoSymbStates> spill;
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
};

using namespace scope;

#include <typeinfo>
void Tester::test1(){
    typedef DemoSymbStates Ssym;
    typedef Ssym::Psym Psym;
    TEST("Construct some symbols");
    {
        Ssym ssym;
        auto x = ssym.newsym("x");
        assert(x==1U);
        //ASSERTTHROW( ssym.psym(1U).getActive() );
        auto y = ssym.newsym("y");
        assert(y==2U);
        cout<<endl;
        assert( ssym.psym(1U).symId() == 1U );
        assert( ssym.psym(2U).symId() == 2U );
        cout<<ssym.psym(1U)<<endl; cout.flush();
        cout<<ssym.psym(2U)<<endl; cout.flush();

        {
            Psym const& x = ssym.psym(1);
            cout<<"Psym const & x = "<<(void*)&x<<" type "<<typeid(x).name()<<endl; cout.flush();
#if 0
            cout<<" x                    = "<<x<<endl;
            cout<<" Since x is ParSymbol<BASE>..."<<endl;
            cout<<" x.getActive() = "<<x.getActive()<<endl; cout.flush();
            cout<<" x.symId()     = "<<x.symId()<<endl;
            cout<<" x.scope()     = "<<x.scope()<<endl;
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
#endif
        }
        {   // alt name for public access during testing...
            Psym & x = ssym.fpsym(1);
            cout<<"Psym & x = "<<(void*)&x<<" type "<<typeid(x).name()<<endl; cout.flush();
            cout<<" x.getActive() = "<<x.getActive()<<endl; cout.flush();
        }
        { // todo this should be protected (not compilable)
            //ScopedSpillableBase& x = ssym.ssb(1);
            ScopedSpillableBase& x = ssym.fpsym(1).base();
            cout<<"ScopedSpillableSymbol & x = "<<(void*)&x<<" type "<<typeid(x).name()<<endl; cout.flush();
            cout<<" x.getActive() = "<<x.getActive()<<endl; cout.flush();
        }
        { // test setREG and setMEM function of ParSymbol<ScopedSpillableBase>
            // these functions are in ScopedSpillableBase
            Psym & x = ssym.fpsym(1);
            x.setREG(true);
#define ssb psym
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
#undef ssb
            cout<<endl;
        }
    }
}

int main(int,char**){
    cout<<"======== test1() ==========="<<endl;
    Tester::test1();
    //cout<<"======== test2() ==========="<<endl;
    //Tester::test2();
    //cout<<"======== test3() ==========="<<endl;
    //Tester::test3();
    cout<<"\nGoodbye - " __FILE__ " ran "<<testNum<<" tests"<<endl;
    return 0;
}
// vim: ts=4 sw=4 et cindent cino=^=l0,\:.5s,=-.5s,N-s,g.5s,h.5s,b1 cinkeys=0{,0},0),\:,0#,!^F,o,O,e,0=break syntax=cpp.doxygen
