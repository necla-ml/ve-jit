
#include "spill-impl2.hpp"
#include "symScopeUid.hpp"
#include "scopedSpillableBase.hpp"

#include "reg-aurora.hpp"

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

class RegSymbol :
    protected scope::ParSymbol<ScopedSpillableBase>
{
    typedef RegisterBase Rb;
    typedef Rb::Cls      Cls; // Cls::scalar Cls::vector Cls::mask Cls::none
    typedef scope::ParSymbol<ScopedSpillableBase> Base;
    friend std::ostream& operator<<(std::ostream& os, RegSymbol const& x);
    friend DemoSymbStates;

    char const* name() const {return name_;}
    RegId regId() const      {return regid_;}
    uint64_t tDecl() const   {return t_decl;}
    uint64_t tSym() const    {return t_sym;} // useful?
  protected:
    RegSymbol& setReg(RegId const rid,uint64_t const tick){         ///< init or copy symbol --> register
        if(valid(rid)){
            regId_ = rid;
            t_sym = tick;
            assert( tick >= t_sym );
            Base::setREG(true);
        }else{
            regId_ = invalidReg();
            Base::setREG(false);
        }
    }
    RegSymbol& tRead(uint64_t const tick){      ///< annotate register read event
        assert(getActive() && valid(regid_) && Base::getREG() && "symbol not in register?");
        assert( tick >= t_sym );
        t_sym = tick;
    }
    RegSymbol& tWrite(uint64_t const tick){     ///< annotate register write event
        assert(getActive() && valid(regid_) && Base::getREG() && "symbol not in register?");
        assert( tick >= t_sym );
        t_sym = tick;
    }
        

  private:
    char const* name_;          ///< declared symbol name
    uint64_t const t_decl;      ///< declaration time tick
    uint64_t t_sym;             ///< last-used tick
    RegId regId_;               ///< register Id \sa reg-base.hpp
};

template<class SYMBSTATES>
class Regset {
  public:
    typedef RegisterBase Rb;
    Regset(SYMBSTATE *ssym, int nScalars)
        : ssym(ssym), scalars(init_some_scalars(nScalars)) {}
    Regset(SYMBSTATE *ssym)
        : scalars(init_scalars()) {}

    std::vector<RegId> unused();
    std::vector<RegId> 
  private:
    SYMBSTATES *ssym;
    std::vector<RegId> scalars;

  private:
    static chipregs = mkChipRegisters();
    static std::vector<uint32_t> getFreeScalarRegs(){
        for(uint32_t i=0U; i<chipregs.size(); ++i){
            RegisterBase const& rb = chipregs(i);
            if( rb == Rb::scalar && rb.free()
                    && !isReserved(Regid(i), Abi::c)
                    && !isPreserved(Regid(i), Abi::c) ){
                ret.push_back(i);
            }
        }
    }
    static std::vector<Regid> init_scalars(){
        std::vector<Regid> ret;
        auto const regs = getFreeScalarRegs();
        ret = std::transform( regs.begin(), regs.end(),
                std::back_inserter(scalars),
                [](regs::value_type const r) {return Regid(r);} );
        return ret;
    }
    static std::vector<Regid> init_some_scalars(unsigned nScalarRegs){
        std::vector<Regid> ret
        auto const regs = getFreeScalarRegs();
        for(unsigned cnt=0U; cnt<nScalarRegs && cnt<regs.size(); ++cnt){
            ret.push_back(RegId(regs(cnt)));
        }
    }
};

class Counters {
  public:
    Counters() : t_(0U), s_(0U)
    uint64_t nextTick(){ return ++t_; }
    unsigned nextSym(){ return ++s_; }
  private:
    uint64_t t_;
    unsigned s_;
};

/** Try to join up some symbol features from regSymbol2.hpp, like named symbols
 * and an easier spill mechanism.
 */
class DemoSymbStates
    : protected scope::SymbStates<ScopedSpillableBase>
    , private Counters
{
  public:
    typedef scope::SymbStates<ScopedSpillableBase> Base;
    typedef Base::Psym Psym;
    typedef RegSymbol Rs;

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
    Rs const& rsym(unsigned symId) r

  public:
    friend class ve::Spill<DemoSymbStates>;

    DemoSymbStates(unsigned nScalars=4U)
        : Base(), spill((Base*)this)
    {
        init_scalars(nScalars);
    }
    ~DemoSymbStates() {
        //Base::end_scope();
    }
  public: // republish some functions
    auto symIdAnyScopeSorted() const { return ssu.symIdAnyScopeSorted(); }
  public:
    RegId allocScalar();                    ///< allocate from \c scalars, spill if nec.
    RegId allocScalar(unsigned const symId);///< allocScalar + setReg
    void setReg(unsigned const symId, RegId const rid); ///< copy symbol R-->R
    void read(unsigned const symId);        ///< annotate register read event
    void write(unsigned const symId);       ///< annotate register write event */

  private:
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
}
using namespace scope;

#include <typeinfo>
void Tester::test1(){
    typedef DemoSymbStates Ssym;
    typedef Ssym::Psym Psym;
    TEST("Construct some symbols");
    {
        Ssym ssym;
        auto x = ssym.newsym();
        assert(x==1);
        //ASSERTTHROW( ssym.psym(1U).getActive() );
        auto y = ssym.newsym();
        assert( y == 1U );
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
