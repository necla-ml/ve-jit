
#include "spill2-impl.hpp"
#include "symScopeUid.hpp"
#include "scopedSpillableBase.hpp"
#include "s2r.hpp"

#include "reg-aurora.hpp"

#include <cassert>
#include <iostream>
#include <map>
#include <memory> // shared_ptr
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

/** Add name, t_decl, t_sym (last use) to a symbol.
 * <B>no tight register association yet</B>
 */
class Symbol :
    public scope::ParSymbol<ScopedSpillableBase>
{
  public:
    typedef RegisterBase Rb;
    typedef Rb::Cls      Cls; // Cls::scalar Cls::vector Cls::mask Cls::none
    typedef scope::ParSymbol<ScopedSpillableBase> Psym;
    friend std::ostream& operator<<(std::ostream& os, Symbol const& x);
    friend DemoSymbStates;

    // republish...
    typedef Psym::Base Base; /** i.e. ScopedSpillableBase */

    void init(char const* name, uint64_t const tick){
        name_ = name;
        t_decl = tick;
        t_sym = tick;
    }

    // create Symbol active, but no assigned register
    template<typename... Arg>
    Symbol(unsigned symId, unsigned scope
            //,char const* name,
            //uint64_t const tick
            //, RegId const regid
            , Arg&&... arg // [name, tick]
            )
        : Psym(symId, scope, defBytes(Rb::Cls::scalar), defAlign(Rb::Cls::scalar))
        , name_("huh?")
        , t_decl(0U)
        , t_sym(0U)
        //, regId_(invalidReg())
        { init(arg...); std::cout<<"+\""<<name_<<"\",t"<<t_sym; }

    char const* name() const {return name_;}
    //RegId regId() const      {return regId_;}
    uint64_t tDecl() const   {return t_decl;}
    uint64_t tSym() const    {return t_sym;} // useful?
  protected:
#if 0 // No. Not here. (more related to s2r.hpp I think)
    Symbol& setReg(RegId const rid,uint64_t const tick){         ///< init or copy symbol --> register
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
#endif
    Symbol& tRead(uint64_t const tick){      ///< annotate register read event
        assert(getActive() /*&& valid(regId_)*/ && Psym::getREG() && "symbol not in register?");
        assert( tick >= t_sym );
        t_sym = tick;
        return *this;
    }
    Symbol& tWrite(uint64_t const tick){     ///< annotate register write event
        assert(getActive() /*&& valid(regId_)*/ && Psym::getREG() && "symbol not in register?");
        assert( tick >= t_sym );
        t_sym = tick;
        return *this;
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
    uint64_t t_decl;            ///< declaration time tick (set once, even inside loop)
    uint64_t t_sym;             ///< last-used tick (set every write or read op)
    // We DO NOT have RegId here --- s2r.hpp will now handle register mapping?
    //RegId regId_;               ///< register Id \sa reg-base.hpp
    friend std::ostream& operator<<(std::ostream& os, Symbol const& x);
};

std::ostream& operator<<(std::ostream& os, Symbol const& x){
    os<<" Rs{"<<x.name_<<",t"<<x.t_decl<<",u"<<x.t_sym<<",r";
    //if(x.regId_ == invalidReg()) os<<"?";
    //else                         os<<(int)x.regId_;
    os<<"}'"<<dynamic_cast<Symbol::Psym const&>(x)<<"'"<<endl;
    return os;
}

// custom specialization of std::hash can be injected in namespace std
namespace std
{
template<> struct hash<Symbol>
{
    typedef Symbol argument_type;
    typedef std::size_t result_type;
    result_type operator()(argument_type const& s) const noexcept
    {
        return std::hash<unsigned>()(s.symId());
    }
};
}


/** Thin wrapper (example class) around std::vector that selects a subset of Registers.
 * This one selects scalar registers that are neither reserved nor preserved in C-abi.
 * The user may want to select several groups of registers ??? */
class Regset {
  public:
    typedef RegisterBase Rb;
    typedef std::vector<RegId> SetType;
    friend bool is_disjoint( Regset const& a, Regset const& b );
    friend class Tester;
  public:
    Regset(std::vector<RegId> const& r)
        : registers(copy_sort(r)), pos_(-1) { /*assert( registers_allowed() );*/ }
    Regset()
        : registers(init(Rb::Cls::scalar))   // a default subset is usable scalar regs
        , pos_(-1)
    { assert( registers.size() > 0 ); } // init_scalars had better not be completely empty
    Regset(Rb::Cls const cls)
        : registers(init(cls))
          , pos_(-1)
    {}
    Regset(size_t const nScalars, Rb::Cls const cls=Rb::Cls::scalar)
        : registers(init(nScalars,cls))   // up to nScalars scalar regs (to test spill code easier)
          , pos_(-1)
    { assert(nScalars >= 0); assert( registers.size() <= (size_t)nScalars ); }

    /** Since Regset is very simple, we have only a few exposed functions */
    SetType::const_iterator find(RegId const r) const {
        return std::find(registers.begin(), registers.end(), RegId(r));
    }
    /** is RegId \c r within our set? */
    bool contains(RegId const r) const{
        return std::find(registers.begin(), registers.end(), RegId(r)) != registers.end();
    }
    SetType::const_iterator begin() const { return registers.begin(); }
    SetType::const_iterator end()   const { return registers.end(); }
    bool valid(RegId const r)       const{ return find(RegId(r)) != end(); }
    SetType::size_type size()       const{ return registers.size(); }
    bool empty()                    const { return registers.empty(); }
    RegId front()                   const {return registers.front();}
    RegId back()                    const {return registers.back();}
    /** an internal round-robin next (added to "Set" api) */
    RegId next()                    {return registers[++pos_ % registers.size()];}

  private:
    std::vector<RegId> copy_sort( std::vector<RegId> const& vr ){
        std::vector<RegId> cpy{vr};
        std::sort(cpy.begin(), cpy.end());
        return cpy;
    }
    static auto getFreeRegs( Rb::Cls const register_class = Rb::Cls::scalar ) {
        std::vector<RegId> ret;
        decltype(mkChipRegisters()) chipregs(mkChipRegisters()); // all
        for(uint32_t i=0U; i<chipregs.size(); ++i){
            RegisterBase const& rb = chipregs(i);
            if( rb.cls() == register_class && rb.free()
                    && !isReserved(RegId(i), Abi::c)
                    && !isPreserved(RegId(i), Abi::c) ){
                //std::cout<<" Free Reg: "<<chipregs(i)<<" asmname "<<asmname(rb.rid)<<std::endl;
                //std::cout.flush();
                ret.push_back(RegId(i));
            }
        }
        return ret;
    }
    static SetType init(Rb::Cls const cls = Rb::Cls::scalar){
        std::vector<RegId> ret;
        std::vector<RegId> const regs = getFreeRegs(cls);
        std::transform( regs.begin(), regs.end(),
                std::back_inserter(ret),
                [](RegId const r) {return r;} );
        return ret;
    }
    static SetType init(size_t const nRegs, Rb::Cls const cls = Rb::Cls::scalar){
        std::vector<RegId> ret;
        auto const regs = getFreeRegs(cls);
        for(size_t cnt=0U; cnt<nRegs && cnt<regs.size(); ++cnt){
            ret.push_back(regs.at(cnt));
        }
        return ret;
    }
  private:
    SetType const registers;   ///< internal set of registers
    int pos_;                ///< round-robin index, for \c next()
};

template<class Set1, class Set2> 
bool is_disjoint(const Set1 &set1, const Set2 &set2)
{
    if(set1.empty() || set2.empty()) return true;

    typename Set1::const_iterator 
        it1 = set1.begin(), 
        it1End = set1.end();
    typename Set2::const_iterator 
        it2 = set2.begin(), 
        it2End = set2.end();

    if(*it1 > *set2.rbegin() || *it2 > *set1.rbegin()) return true;

    while(it1 != it1End && it2 != it2End)
    {
        if(*it1 == *it2) return false;
        if(*it1 < *it2) { it1++; }
        else { it2++; }
    }

    return true;
}
bool is_disjoint( Regset const& a, Regset const& b ){
    return is_disjoint( a.registers, b.registers );
}
    


/** Symbols whose RegId is constrained to a given Regset.
 * - The regset is predefined (const) for now.
 * - Adds some utility functions to s2r.hpp.
 * Q: Use a RegsetScopedSpillable that is the union of all
 *    RegsetScopedSpillable::regset (i.e. unconstrained RegId)?
 *    Or just check all declared ResetScopedSpillable when checking
 *    for Symbol name clashes?
 *
 *
 * - Since we could have several of these (ex. scalars, tmp-scalars, vectors, masks,...)
 *   they SHARE the ref to SymbStates (which begins/ends scope)
 *
 * An alt. structure is to have DemoSymbStates manage a global scope and a global S2R,
 * and provide "Regset" as a modifier to symbol declaration/allocation routines.
 * (THIS OPTION SEEMS NICER)
 *
 * DemoSymbStates owns multiple RegsetScopedSpillable.
 * Each RegsetScopesSpillable access the SHARED global maps, R2S and SSS.
 */
class RegsetScopedSpillable {
  public:
    typedef Symbol symbol_type;
    //typedef unordered_map<S2R::Sid,symbol_type> S2Symbol; //??
    typedef scope::SymbStates<symbol_type> SSS;
    typedef unsigned Sid;
    typedef RegisterBase Rb;
    typedef std::unordered_set<unsigned> HashSet;
    friend DemoSymbStates;
    friend Tester;
  protected:
    /** link s<-->r strongly. \pre r in regset. push out any former strong-assign. \sa S2R::mkStrong */
    //template<typename SYMBSTATES>
    void mkStrong(/*SYMBSTATES & sss,*/ Sid const sid, RegId const rid) {
        assert( sss->psym(sid).getActive() ); // perhaps not necessary
        assert( regset.valid(rid) );
        auto const oldsid = sr->strong(rid);  // auto transitions to weak, but also need to setREG(false)
        sr->mkStrong(sid,rid);                // s2r transitions oldsid to weak (if nec.)
        if(oldsid) // S2R::sBad is zero
            sss->psym(oldsid).setREG(false);  // no spill object [yet], so do not setMEM(true)
        sss->psym(   sid).setREG(true);
        std::cout<<" Strong("<<sid<<":"<<rid<<"(\""<<sss->psym(sid)<<"\"))";
    }
    /** return strong-symbol of r, or S2R::sBad (0). \pre r in regset. */
    unsigned strong(RegId const r){
        assert( regset.valid(r) );
        return sr->strong(r);
    }
  public:
    RegsetScopedSpillable(Regset regset,
            S2R *psr,
            scope::SymbStates<Symbol>* psss)
            //std::shared_ptr<S2R> psr,
            //std::shared_ptr<scope::SymbStates<Symbol>> psss)
        : regset(regset)
          , sr(psr)
          , sss(psss)
    {}

  protected:
    /** \group register retrieval/sort functions
     * - Set vector<RegId> sorted in order of:
     *   1. mt [=no strong|weak], avail [=no strong], strong
     *   2. Symid (or sBad) of the symbol with latest [=highest] t_sym
     */
    //@{
    typedef std::pair<RegId,Sid> PRS; ///< <B>P</B>air-<B>R</B>egister-<B>S</B>ymbol
    typedef std::vector<std::pair<RegId,Sid>> VPRS; ///< <B>V</B>ector-<B>R</B>egister-<B>S</B>ymbol
    std::vector<RegId>& getAvail(std::vector<RegId>& ret) const {
        ret.clear();
        VPRS vprs(regset.size());
        this->sort(avail(vprs));
        if(vprs.empty()){
            for(auto const& r_s: sr->strongs()) vprs.push_back(r_s);
            this->sort(vprs);
        }
        assert( !vprs.empty() );
        for(auto const& prs: vprs){
            ret.push_back(prs.first);
        }

        return ret;
    }

  private:
    struct CmpTsym{
        S2R const& sr;
        SSS const& ss;
        CmpTsym(S2R const& sr, SSS const& ss) : sr(sr), ss(ss) {}
        /** compare 1st@ strong > weak > old of s<-->r link, 2nd@ s.t_sym */
        bool operator()( PRS const& a, PRS const& b ){
            RegId const ra = a.first;
            RegId const rb = b.first;
            unsigned const sa = a.second;
            unsigned const sb = b.second;
            unsigned const sBad = S2R::sBad;
            int ca, cb; // s<-->r association type:  0=[sBad] 1=old, 2=weak, 3=strong
            ca = (sa==sBad? 0: sr.isOld(sa)? 1: sr.isWeak(sa)? 2: 3);
            cb = (sb==sBad? 0: sr.isOld(sb)? 1: sr.isWeak(sb)? 2: 3);

            if(ca+cb == 0) return ra<rb;
            if(ca < cb) return true;
            if(cb < ca) return false;
            assert( sa != sBad && sb != sBad );

            //std::cout<<"\nca==cb and no sBad.  compare tSyms"<<std::endl;
            //if(!ss.empty())
            {
                auto const ta = ss.psym(sa).tSym();
                auto const tb = ss.psym(sb).tSym();
                //std::cout<<" cmp(ta="<<ta<<",tb="<<tb<<")";
                if(ta < tb) return true;
                if(tb < ta) return false;
            }
            return ra<rb;
        }
    };
    /** weakest linked, then LRU sort of Vector-Pair-Register-Symbol */
    void sort( VPRS& vprs ) const { // vector<pair{register,symbol}>
        std::sort( vprs.begin(), vprs.end(), CmpTsym(*this->sr, *this->sss) );
    }
    /** set arg vector to all on-strong regs and return that vector.
     * returned registers can be weak-assigned, old or never-mapped.*/
    std::vector<RegId>& avail(std::vector<RegId>& ret) const {
        ret.clear();
        for(auto const r: regset)
            if (!sr->hasStrong(r)) // sr might manage multiple regsets
                ret.push_back(r);
        return ret;
    }
    VPRS& avail(VPRS& ret) const {
        ret.clear();
        for(auto const r: regset){
            auto const s = sr->strong(r);
            if (s == S2R::sBad){
                ret.push_back({r,s});
            }
        }
        return ret;
    }
    //@}
    //@{
    /** retrieve all registers with neither strong- nor weak-assigned symbols.
     * These are <em>even more empty</em> than \c avail() registers. */
    std::vector<RegId>& mt(std::vector<RegId>& ret) const {
        ret.clear();
        for(auto const r: regset)
            if (!sr->hasStrong(r) && !sr->hasWeak(r))
                ret.push_back(r);
        return ret;
    }
    //@}
    /** return registers sorted primarily by old/weak/strong,
     * and then by [highest==latest, for weak/old] \c t_sym */

  public:
    S2R const& getS2R() const {return *sr;} /// \deprecated [lifetime badness]
  private:
    Regset const regset;     ///< set of allowed registers
    //std::shared_ptr<S2R> sr; ///< symbol<-->register links (fwd=injective, bidirectional, strong/weak/old link type)
    //std::shared_ptr<SSS> sss;
    S2R *sr;
    SSS *sss;
    
    void chk(){
        ; // TODO
        // strong- or weak-assigned symbols here MUST have getREG state true
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
    : protected scope::SymbStates<Symbol>
    , private Counters
{
  public:
    typedef RegisterBase Rb;
    typedef scope::SymbStates<Symbol> Ssym;
    //typedef Ssym::Psym Psym;
    typedef Symbol Psym;
    typedef Symbol Ppsym;
    typedef Symbol Rs;

  public: // republish
    auto const& psym(unsigned s) const { return Ssym::psym(s);}
    auto & psym(unsigned s)       { return Ssym::psym(s);}
    auto & fpsym(unsigned s)      { return Ssym::fpsym(s);}

  public: // change!!!
    /** DECLARE a symbol in current scope. \return symbol Id */
    template<typename... Arg> // Arg actually unused
        inline unsigned newsym(
                char const* name
                //, Arg&&... arg
                ) {
            auto const symId = Ssym::newsym( name, nextTick() );
            // Ex. call Ssym::Psym(symId,scope,name,tick);
            // i.e. Symbol constructor
            return symId;
        }
    struct RegsetId { size_t rsid; };
    /** Add another pool of registers. \return handle to Regset */
    struct RegsetId add(Regset &&rs) {
        regsets.push_back(RegsetScopedSpillable(rs,&this->s2r,(Ssym*)this));
        return RegsetId{regsets.size()-1};
    }
    /** ASSIGN register to symbol [in default register-set].
     * This should interrogate S2R, intersect it with the register-set, and return the
     * "next-available" assignment, possibly kicking out any previous strong-assigned symbols
     * (and spilling them to memory).
     *
     * 'C' code should not need to assign, since registers and spilling are not required.
     */
    RegId assign(unsigned sid, RegsetId const rsid){ // default = scalar reg
        static std::vector<RegId> avail; // XXX thread_local
        auto &rs = regsets.at(rsid.rsid);
        rs.getAvail(avail);
        assert( !avail.empty() );
        rs.mkStrong(sid, avail.at(0));
    }
    RegId assign(unsigned sid, Rb::Cls const cls ){ // use one of the predefined Regsets
        RegsetId rsid;
        switch((int)cls){
          case((int)Rb::Cls::scalar): rsid=regset_scalar; break;
          case((int)Rb::Cls::vector): rsid=regset_vector; break;
          case((int)Rb::Cls::mask):   rsid=regset_mask; break;
          default: THROW("unknown register class in assign(unsigned symId,Rb::Cls cls)");
        };
        this->assign(sid, rsid);
    }

  private:
    // psym/fpsym is ParSymbol<ScopedSpillableBase> with:
    //   ParSymbol:           uid/symId(), scope, getActive,
    //   ScopedSpillableBase: getREG, getMEM
    //   (Spillable:          getBytes, getAlign)
    // Ssym::syms = SymbStates::syms is (Symbol) map for:
    //   name, RegId, staleness, 
  public:
    Rs const& rsym(unsigned symId);
    static RegsetId constexpr regset_default = RegsetId{0};
    static RegsetId constexpr regset_scalar = RegsetId{0};
    static RegsetId constexpr regset_vector = RegsetId{1};
    static RegsetId constexpr regset_mask   = RegsetId{2};

  private:
    //std::vector<Regset> regsets;
    // eventually would like vector<RegsetScopeSpillable> for regsets
    std::vector<RegsetScopedSpillable> regsets;

  public:
    friend class ve::Spill<DemoSymbStates>;

    //DemoSymbStates() : DemoSymbStates( Regset(4U, Rb::Cls::scalar) ) {};
    DemoSymbStates()
        : Ssym(),
        regsets(),
        spill(this),
        //shared_ssym(std::shared_ptr<Ssym>((Ssym*)this)),
        //shared_s2r(make_shared<S2R>())
        s2r()
    {
        add(Regset(Rb::Cls::scalar));
        add(Regset(Rb::Cls::vector));
        add(Regset(Rb::Cls::mask));
        //add(RegsetScopedSpillable(Rb::Cls::scalar, shared_s2r, shared_ssym));
        //add(RegsetScopedSpillable(Rb::Cls::vector, shared_s2r, shared_ssym));
        //add(RegsetScopedSpillable(Rb::Cls::mask  , shared_s2r, shared_ssym));
        //init_scalars(nScalars);
        assert( regsets.size() == 3 );
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
    //RegId allocScalar();                    ///< allocate from \c scalars, spill if nec.
    //RegId allocScalar(unsigned const symId);///< allocScalar + setReg
    void setReg(unsigned const symId, RegId const rid); ///< copy symbol R-->R
    void read(unsigned const symId);        ///< annotate register read event
    void write(unsigned const symId);       ///< annotate register write event */

  private:
    friend bool chk_order(DemoSymbStates const& ssym, std::initializer_list<unsigned> const& i);
    /** The spill manager lifetime should be less than the psyms map lifetime */
    ve::Spill<DemoSymbStates> spill;
    //std::shared_ptr<Ssym> shared_ssym;
    //std::shared_ptr<S2R>  shared_s2r;
    S2R s2r;
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
    TEST("Regset 1 via Regset(vector<RegId> from Regset::getFreeScalarRegs())");
    {
        auto scalars = Regset::getFreeRegs(); // scalar ones, by default: Rb::Cls::scalar
        scalars.resize(3);
        scalars.push_back(Regid(37));
        scalars.push_back(Regid(33));
        // NO std::vector<RegId> vRegId(std::initializer_list<RegId>{0,1,2,3,4});
        cout<<" scalars={"; for(auto const r: scalars) cout<<" "<<r; cout<<"}"<<endl;
        Regset rs(scalars);
        cout<<" rs={"; for(auto const r: rs) cout<<" "<<r; cout<<"}"<<endl;
        assert( rs.size() == 5 );
        assert( rs.front() == Regid(0) );
        assert( rs.back() == Regid(37) ); // they are sorted by RegId
        assert( rs.find(Regid(37)) != rs.end() );
        assert( rs.find(Regid(23)) == rs.end() );

    }
    TEST("Regset 2, RegsetScopedSpillable demo class");
    {
        //auto psss = make_shared<scope::SymbStates<Symbol>>(); // this holds all symbols
        //auto psr = make_shared<S2R>(); // this holds all symbols
        auto psss = new scope::SymbStates<Symbol>();
        scope::SymbStates<Symbol> & sss = *psss;
        auto psr = new S2R();
        Regset rs(5); // this can hold 5 symbols in a subset of 5 scalar registers
        cout<<" rs={"; for(auto const r: rs) cout<<" "<<r; cout<<"}"<<endl;
        assert( rs.size() == 5 );
        assert( rs.front() == Regid(0) );
        assert( rs.back() == Regid(4) ); // they are sorted by RegId
        assert( rs.find(Regid(4)) != rs.end() );
        assert( rs.find(Regid(5)) == rs.end() );
        assert( rs.valid(Regid(4)) );
        assert( !rs.valid(Regid(invalidReg())) );
        assert( !rs.valid(Regid(37)) );
        assert( !rs.valid(Regid(999)) );
        RegsetScopedSpillable rss(rs,psr,psss);
        // normally DemoSymbStates would coordinate psr, psss lifetime
        vector<RegId> avail;
        rss.getAvail(avail);
        cout<<" avail={"; for(auto const r: avail) cout<<" "<<r; cout<<"}"<<endl;
        assert( avail.size() == 5 );
        unsigned sx,sy, tick=13;
        rss.mkStrong(/*sss,*/ sx=sss.newsym("x",++tick), Regid(avail[0]));
        rss.getAvail(avail); cout<<" avail={"; for(auto const r: avail) cout<<" "<<r; cout<<"}"<<endl;
        rss.mkStrong(/*sss,*/ sy=sss.newsym("y",++tick), avail[0]);
        rss.getS2R().dump();
        //sss.prtCurrentSymbols();
        auto vSid = sss.symIdAnyScopeSorted();
        cout<<" syms={\n"; for(auto const sid: vSid) cout<<"\t"<<sss.psym(sid); cout<<"      }"<<endl;
        //cout<<sss<<endl;
        rss.mkStrong(/*sss,*/ sss.newsym("a",++tick), avail.at(0));
        rss.getAvail(avail); cout<<" avail={"; for(auto const r: avail) cout<<" "<<r; cout<<"}"<<endl;
        rss.mkStrong(/*sss,*/ sss.newsym("b",++tick), avail.at(0));
        rss.getAvail(avail); cout<<" avail={"; for(auto const r: avail) cout<<" "<<r; cout<<"}"<<endl;
        rss.mkStrong(/*sss,*/ sss.newsym("c",++tick), avail.at(0));
        rss.getAvail(avail); cout<<" avail={"; for(auto const r: avail) cout<<" "<<r; cout<<"}"<<endl;
        rss.mkStrong(/*sss,*/ sss.newsym("d",++tick), avail.at(0));
        rss.getAvail(avail); cout<<" avail={"; for(auto const r: avail) cout<<" "<<r; cout<<"}"<<endl;
        assert( ! avail.empty() ); // is has the strongs, in t_sym order
        rss.mkStrong(/*sss,*/ sss.newsym("e",++tick), avail.at(0));
        rss.getS2R().dump();
        vSid = sss.symIdAnyScopeSorted();
        cout<<" syms={\n"; for(auto const sid: vSid) cout<<"\t"<<sss.psym(sid); cout<<"      }"<<endl;

        rss.getAvail(avail); cout<<" avail={"; for(auto const r: avail) cout<<" "<<r; cout<<"}"<<endl;
        rss.mkStrong(/*sss,*/ sss.newsym("f",++tick), avail.at(0));
        rss.getS2R().dump();
        vSid = sss.symIdAnyScopeSorted();
        cout<<" syms={\n"; for(auto const sid: vSid) cout<<"\t"<<sss.psym(sid); cout<<"      }"<<endl;

        rss.getAvail(avail); cout<<" avail={"; for(auto const r: avail) cout<<" "<<r; cout<<"}"<<endl;
        rss.mkStrong(/*sss,*/ sss.newsym("g",++tick), avail.at(0));
        rss.getS2R().dump();
        vSid = sss.symIdAnyScopeSorted();
        cout<<" syms={\n"; for(auto const sid: vSid) cout<<"\t"<<sss.psym(sid); cout<<"      }"<<endl;
        delete psr;
        delete psss;
    }
    TEST("Construct some symbols and assign to registers");
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
/** \file
 * --------------------------------------------------------------------
 * What follows is a discussion of Use Cases, and how [some other class]
 * would use the low-level SymRegid data structure.
 *
 * State is synchronized by SYMBSTATES to match individual Symbol info.
 * SYMBSTATES::syms maps symId-->Symbol,
 * and you use SYMBSTATES::psym(symId) to obtain the Symbol.
 *
 * - compare with:
 *   - \c SymbStates (\c DemoSymStates) symId<-->scope (high-level)
 *     - \c SymScopeUid, scope<-->symids (low-level data structure)
 *   - symbol \e scopes have infinite nestedness, in principle
 *   - symbol \e register-assignments have to deal with a finite
 *     number of registers (o.w. they spill).
 *   - Hmmm. or I could use the infinite-register model, and later
 *     remap to actual registers, as done in llvm.
 *     - Downside is that this requires some post-processing.
 *     - OH, BUT...
 *       - the post-processing could be restricted to rewriting
 *         the #define SYMNAME V_REG_NNNN [opt. other info]
 *         lines of the infinite register model
 *         with #define SYMNAME %s37 actual register assignments
 *   - But let's stick with Regset approach, whose policies can likely
 *     be fine-tuned to provide for decent code in the 3 main code-gen
 *     scenarios:
 *     - looping generic code
 *     - looping jit code
 *     - unrolled jit code.
 *   - These scenarios also use chipset info:
 *     - register-reuse impact (i.e. dest reg == source reg),
 *     - pipeline depth,
 *     - parallelism available for different op-types.
 *
 * - Just as a symId maps to a single scope,
 *   - a symId maps to a single RegId.
 *   - a symId can be reassigned to another RegId
 *   - a symId is still 'assigned' to a Reg when it is spilled
 *     - but Symbol::getREG() is false. See \ref ScopedSpillableBase.
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
 * ----------------------------------------------------
 */
// vim: ts=4 sw=4 et cindent cino=^=l0,\:.5s,=-.5s,N-s,g.5s,h.5s,b1 cinkeys=0{,0},0),\:,0#,!^F,o,O,e,0=break syntax=cpp.doxygen
