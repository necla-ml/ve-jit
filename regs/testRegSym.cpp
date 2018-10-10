/** \file
 * Basic \ref regSymbol.hpp tests with a DemoSymbStates class.
 *
 * First thing I noticed was some need to define register sets.
 * Original work had the symbol state object construct dummy
 * symbols for every register in the machine.  But this is not
 * portable to different machines.
 *
 * With no concept of "register sets", it was handy to predefine
 * a symbol for every register, in global scope.  Then \em every
 * register allocation \em always worked by kicking out another
 * symbol.
 *
 * But really, RESERVED is a \em register state, so let's
 * revise regDefs.hpp into machine-centric and generic
 * parts, and also provide some constexpr bool f(Regid)
 * checkers, for things like RESERVED.
 *
 * - Ex. bool isReserved(regId) --> false for RESERVED and arg-related,
 *   because these \b never go into a Spill object (for local symbols)
 * - Ex. bool isPreserved(regId) --> true if save/restore needed when
 *   generating prologue/epilogue code.
 *
 * Options:
 *
 * 1. We add to the list of function-reserved regs any arg-related registers,
 * 2. OR we forego argument-passing entirely and
 *    ask the client to hand-populate registers to initialize some kernel.
 *
 * I think (2) is easiest to implement, and might fit JIT-style
 * applications nicely.   Automated generation of entire functions need
 * to do some more work, perhaps setting initial symbol states using
 * info from libffi.
 *
 * So plan is:
 *   Call using C wrapper --> C++ impl --> void fn(void) assembler
 *   kernel that has all args passed via (ANY) registers.
 *   The function still generates function prologue/epilogue
 *   that sets stack frame and [only if used] saves/restores
 *   any regs with PRESERVE status.
 *
 * Some register-state checkers need to be runtime-definable.
 * Ex. function arg registers
 */
#include "regSymbol.hpp"
#include "spill-impl.hpp"
#include "throw.hpp"
#include <cassert>
#include <iostream>
#include <map>
#include <string>
#include <cstdlib>         // rand
#include <vector>
#include <cstring>
#include <unordered_set>
using namespace std;
using namespace ve;

class SpillableRegSym;
class DemoSymbStates;

struct Tester{
    static void test1();
    static void test2();
    static void test3();
};

// I don't care about register names, make them all tmpFoo.
std::vector<char const*> randNames;
char const* randName(){
    {
        std::ostringstream oss;
        oss<<"tmp"
            <<static_cast<char>('A'+std::rand()%26)
            <<static_cast<char>('A'+std::rand()%26)
            <<static_cast<char>('A'+std::rand()%26)
            ;
        char * s = new char[oss.str().size()+1];
        strcpy(s, oss.str().c_str());
        randNames.push_back(s);
        using namespace std;
        //cout<<" tmp["<<randNames.size()<<"]"; cout.flush();
    }
    return randNames.back();
}

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
class SpillableRegSym : public RegSymbol
{
  public:
    friend class Tester;
    SpillableRegSym( SpillableRegSym const& ) = delete;
    SpillableRegSym( SpillableRegSym&& s ) noexcept : RegSymbol(std::move(s)) {}
    /** Just declare a symbol [default: unassigned scalar register with tmp name].
     * - A real program would provide useful names (here we autogenerate "tmpXYZ")
     *   - perhaps same as the symbol object variable?
     * - For demo, we also never need to assign an actual \c RegId,
     *   - so printout says %XX unless register id is explicitly set
     * */
    SpillableRegSym(unsigned const symId, uint64_t const tick,
            char const* name, Reg_t const rtype=REG_T(FREE|SCALAR))
        : RegSymbol(symId, tick, (name? name: randName()), rtype)
        {
            std::cout<<" +SRS(id="<<symId<<",tick="<<tick<<",name="<<name<<",rtype="<<rtype<<")"<<std::endl;
        }

    /** Given a free RegId [from Spill] that you wrote to, declare and assign to register. */
    SpillableRegSym(unsigned const symId, uint64_t const tick,
            RegId const rid, char const* name)
        : RegSymbol(symId, tick, (name? name: randName()), rid)
        {
            std::cout<<" +SRS(id="<<symId<<",tick="<<tick<<",name="<<name<<",rid="<<rid<<")"<<std::endl;
        }

};

/** represent a range of registers, a set of temporary registers. */
struct Counters {
    Counters()
        : tick_(0U), symidCnt(0U),
        ridBeg(30), ridEnd(IDscalar_last+1), ridCyc(ridEnd),
        ridCyc4(3) {}

    uint64_t tick_;
    unsigned symidCnt;
    RegId const ridBeg;
    RegId const ridEnd;
    RegId ridCyc;                               // cycle [30..63]
    RegId ridCyc4;                              // cycle [0,1,2,3]

    unsigned nextSym() {
        return ++symidCnt;
    }
    RegId nextRid() {
        if(++ridCyc >= ridEnd) ridCyc = ridBeg;
        return ridCyc;
    }
    RegId nextRid4() {
        if(++ridCyc4 >= 4) ridCyc4 = 0U;
        return ridCyc4;
    }
    uint64_t nextTick(){
        return ++tick_;
    }
};

#if 0
/** declare a "nicely-named" Register Symbol easily. Assume we don't get symbol
 * clashes during \c nextRid().*/
#define DSYM(DSS,VAR) SpillableRegSym VAR{DSS->nextSym(), #VAR, ve::SCALAR, DSS->nextTick()}
#endif

/** Spillable objects need a way to convert symbol Ids into SpillableRegSym */
struct DemoSymbStates : public Counters {
    typedef SpillableRegSym Psym;  // "parent" symbol class
    typedef map<unsigned,Psym> SymIdMap;
    typedef ve::Spill<DemoSymbStates> SpillType;

    DemoSymbStates()
        : Counters(), psyms(), spill(this) {}

    SymIdMap psyms;   ///< symId-->Psym (Psym=SpillableRegSym)

    /** The spill manager lifetime should be less than the psyms map lifetime */
    SpillType spill;

    // NO: only supported 1 REG <==> 1 SYMBOL (KISS)
    //map<RegId,std::vector<unsigned>> regidMap;

    Psym const& psym(unsigned const symId) const {
        auto found = psyms.find(symId);
        if( found == psyms.end() )
            throw(std::runtime_error("psym(symId) doesn't exist"));
        return found->second;
    }
    Psym & psym(unsigned const symId) {     
        auto found = psyms.find(symId);
        if( found == psyms.end() )
            throw(std::runtime_error("psym(symId) doesn't exist"));
        return found->second;
    }

    template<typename T, typename U>
        decltype(auto)
        make_pair_wrapper(T&& t, U&& u)
        {
                return std::make_pair(std::forward<T>(t),
                                                  std::forward<U>(u));
        }

    void dump() const;

    /** declare a symbol. It is active, but in neither REG nor MEM.
     * - \em all your Psym should be constructed via \c decl !
     * - we generate the tick and uid args internally,
     * - \c Psym constructor called as \c Psym(tick,uid,OtherPsymArgs...)
     * - \return symbol id \c Psym::uid, where \c psym(symId)
     * - So \c decl("a") would construct a \c Psym(nextTick(),nextSym(),"x")
     *   - and return the symbol id
     *   - symId also available via \c SpillableBase as \c psym(symId).uid
     */
    template<typename...Args>
    auto decl(Args&&... otherPsymArgs){
        auto const tick = nextTick();
        auto const symId = nextSym();
        assert( psyms.find(symId) == psyms.end() );
        //Psym s = SpillableRegSym(tick,symId,args...);
        //psyms.emplace(symId, Psym(tick,symId,"x"));
        // using Psym copy constructor...
        //psyms.emplace(symId, Psym(tick,symId,otherPsymArgs...));
        //psyms.emplace(symId, std::forward<Psym>(Psym(tick,symId,otherPsymArgs...)));
        //psyms.emplace(make_pair_wrapper(symId, Psym(tick,symId,otherPsymArgs...)));
        //psyms.emplace(std::piecewise_construct,
        //        std::forward_as_tuple(symId),
        //        std::forward_as_tuple(Psym(tick,symId,otherPsymArgs...)));
        //psyms.emplace(std::move(SymIdMap::value_type{symId, Psym(tick,symId,otherPsymArgs...)}));

        //psyms.emplace(SymIdMap::value_type{symId, Psym(tick,symId,otherPsymArgs...)});
        psyms.emplace(SymIdMap::value_type{symId, Psym(symId,tick,otherPsymArgs...)});
        return symId;
    }

    /// \group utils
    ///@{
    void syms_all(ve::RegId const r, std::vector<unsigned>& symids){
        //
        // More complex behaviour might maintain a map: RegId --> {SymIds}
        //
        symids.clear();
#if 1
        for(auto& s: psyms)
            if(s.second.regId() == r)
                symids.push_back(s.first);
#else // c++17, but not supported by nc++
        for(auto&& [symId,psym]: psyms) // c++17 "structured binding declaration"
            if( psym.regId() == r )
                symids.push_back(symId);
#endif
    }
    void syms_active(ve::RegId const r, std::vector<unsigned>& symids){
        symids.clear();
        for(auto& s: psyms){
            auto const& sObj = s.second;
            if(sObj.regId() == r && sObj.getActive())
                symids.push_back( /*symId*/s.first );
        }
    }
    void syms_activeREG(ve::RegId const r, std::vector<unsigned>& symids){
        symids.clear();
        for(auto& s: psyms){
            auto const& sObj = s.second;
            if(sObj.regId() == r && sObj.getActive() && sObj.getREG())
                symids.push_back( /*symId*/s.first );
        }
    }
#if 0
        ve::RegId const rpool[4] = {0,1,2,3};
        std::map<unsigned, std::vector<unsigned>> r2s;
        for(auto const s: symIdMap){
            auto const sid = s.first;
            auto const& sObj = s.second;
            for(auto const r: rpool)
                if(sObj.regId() == r)
                    reg2syms[r].push_back(sid);
        }
#endif
    DemoSymbStates& tRead(std::initializer_list<unsigned const> symids){
        auto tk = nextTick();
        for(auto const sym: symids) psym(sym).tRead(tk);
        return *this;
    }
    DemoSymbStates& tWrite(std::initializer_list<unsigned const> symids){
        auto tk = nextTick();
        for(auto const sym: symids) psym(sym).tWrite(tk);
        return *this;
    }
    void wr(std::initializer_list<unsigned const> w,
            std::initializer_list<unsigned const> r
            = std::initializer_list<unsigned const>() ){
        auto tk = nextTick();
        for(auto const rsym: r) psym(rsym).tRead(tk);
        for(auto const wsym: w) psym(wsym).tWrite(tk);
    }
    ///@}
    /** return a single 'best' [easiest,lru] candidate register to spill.
     * ... or invalidReg() if no in-reg symbols.
     * - We select from all known in-scope symbols
     *   - for now, ignore RESERVED registers and such complications.
     * A. Easy to spill:
     *    - symbol inactive (No register should ever be assigned/spillable)
     *    - symbol in register but no assigned register id (error?)
     *    - Whenever sym in-register & spill copy is non-stale,
     *      we can just drop the register (no-op).
          - i.e. \c unsetREG() (eventually does \c setREG(false))
     * B. Spill really needed:
     *    - All in-register syms scanned for lowest tSym() (LRU symbol)
     *
     * \return symbol id good for spilling [or invalidReg()].
     *
     * Note: could support concept of register sets, so that we can
     * first look for empty, and [only if nec.] spill, but not in
     * this demo.
     *
     * WE DO NOT SPILL ANYTHING, just suggest a spillable symId.
     *
     * - Real implementations might:
     *   - skip RESERVED registers [optional symbol state, or is this register state?]
     *   - handle register-subsets
     *
     * Note: it might make more sense to return a register + {symIds in/mapped to REG}
     */
    unsigned spillOne_orig(int const verbose=0) const{
        // more complicated: break sObj.tSym() ties by sObj.tDecl()
        std::map<uint64_t,unsigned> tSyms;
        std::map<uint64_t,unsigned> tEasy;
        cout<<" s1:"; for(auto const& s: psyms) cout<<(s.second.getREG()? "R": "r")<<(s.second.getMEM()? "M": "m");
        for(auto const& s: psyms){
            unsigned const sId = s.first;
            Psym const& sObj = s.second;
            if( !sObj.getActive() ) continue;
            if(sObj.getREG() && !ve::valid(sObj.regId())){
                std::cout<<" Warning: getREG() but regId no good???\n "; std::cout.flush();
                return s.first;
            }
            if( !sObj.getREG() ) // we can't spill symbol if it is not in a register
                continue;
            // This is a paranoid sanity check --- see after loop ...
#if 0
            auto find_sId = find_if(spill.regions().begin(), spill.regions().end(),
                    [&sId](SpillType::Region const& r){return r.symId == sId;} );
            if( find_sId != spill.regions().end() ){
                if(sObj.getMEM() && !sObj.getStale()){
                    tEasy.emplace(std::make_pair(  sObj.tSym(),  sId  ));
                }
            }
            // sanity check: (no need to look at spill if the world works properly)
            if(sObj.getMEM()) assert( find_sId != spill.regions().end() );
            if(!sObj.getMEM()) assert( find_sId == spill.regions().end() );
#elif 1 // nicer
            bool inSpill = any_of(spill.regions().begin(), spill.regions().end(),
                    [&sId](SpillType::Region const& region){return
                    region.symId == sId;});
            assert( inSpill == sObj.getMEM() ); // spill region iff symbol getMEM() flag
            if(/*inSpill &&*/ sObj.getMEM() && !sObj.getStale()){
                tEasy.emplace(std::make_pair(  sObj.tSym(),  sId  ));
                continue;
            }
#else // short, less checking
            if( sObj.getMEM() && !sObj.getStale() ){
                tEasy.emplace(std::make_pair(  sObj.tSym(),  sId  ));
                continue;
            }
#endif
            // unspilled OR spilled and stale (last resort, need to spill reg)
            tSyms.emplace(std::make_pair(  sObj.tSym(),  sId  ));
        }
        if(verbose){
            cout<<" Spillone..."<<endl;
            cout<<"   Easy{"; for(auto const& x: tEasy) cout<<" "<<x.first<<":"<<x.second; cout<<"}"<<endl;
            cout<<"   Syms{"; for(auto const& x: tSyms) cout<<" "<<x.first<<":"<<x.second; cout<<"}"<<endl;
        }
        if( !tEasy.empty() ) return tEasy.begin()->second;
        if( !tSyms.empty() ) return tSyms.begin()->second;
        std::cout<<" Ohoh: no register to spill? "<<std::endl;
        return 0;
    }
    /** spill one symbol, from amongst all existing symbols */
    unsigned spillOne(int const verbose=0) const{
        // find registers covering ALL symbols and punt to worker SpillOne
        std::unordered_set<RegId> symbolRegs;
        for(auto const& s:psyms)
            symbolRegs.insert(s.second.regId());
        std::vector<RegId> vecRegs;
        vecRegs.reserve(symbolRegs.size());
        for(auto const& s:symbolRegs)
            vecRegs.push_back(s);
        return spillOne(vecRegs,verbose);
    }
    /** show how to spill one symbol from a compile-time array of register ids.
     * - We check all \em symbols,
     * - skipping any whose \c regId() is not in the given \c regIds list
     * - to find the easiest [or lru] symbol to kick.
     *
     * \return active symId whose register is in \c regIds that could be spilled
     *
     * - Better: template<typename Predicate> spillOne(auto& pred)
     *   that checks that pred(sObj.regId()) returns true.
     *
     * \c spillOne assumes we might have multiple syms mapped to a register.
     *
     * Note: it might make more sense to return a register + optional {symIds in/mapped to REG}
     */
    unsigned/*SymId*/ spillOne(std::vector<RegId> regIds, int const verbose=0) const{
        if(verbose){
            cout<<" s1:";
            for(auto const& s: psyms) cout<<(s.second.getREG()? "R": "r")
                <<(s.second.getMEM()? "M": "m");
        }
        std::map<unsigned, std::vector<unsigned>> r2s;
        for(auto const& s: psyms){
            auto const sid = s.first;
            auto const& sObj = s.second;
            for(auto const r: regIds)
                if(sObj.regId() == r)
                    r2s[r].push_back(sid);
        }
        // NOTE: would be better if cycled through the pool (round-robin "state")
        //std::map<unsigned,unsigned> points(regIds.size());
        //for(auto& r: regIds) { points[r] = 0U; }
        //
        // Method:
        //
        // 1. We treat easy...hard cases and exit with:
        //   - someRegs[] : list of equivalently-spillable registers
        //   - r2s[someRegs[]] : list of symId for each easiest-spilled reg
        // 2. Amongst someRegs, select:
        //   - sometimes check for most MEM staleness
        //   - else if still tied check for LRUsed
        //   - else if still tied check for LRDecl
        // - Oh. fRRobin would need a persistent regIds[] pool to maintain state.
        // - Oh. fANY is always the last tie-breaker.
        std::vector<ve::RegId> someRegs;
        std::vector<unsigned> someSyms, otherSyms;
        int constexpr fANY=0, fLRUsed=1, fLRDecl=2, fMEMstale=4 /*, fRRobin=8*/;
        int ties = fLRUsed | fLRDecl | fANY;
        if(someRegs.empty()){ // trivial case: regs w/ no mapped syms
            for(auto const r: regIds){
                auto const& syms = r2s[r];
                if( syms.empty() ){
                    if(verbose) cout<<" r"<<r<<"(no syms)";
                    //return r;                       // no syms
                    someRegs.push_back(r);
                }
            }
            ties = fANY;
            if(!someRegs.empty() && verbose) cout<<"[no syms]"<<endl;
        }
        if(someRegs.empty()){   // regs w/ no active syms
            for(auto const r: regIds){
                auto & syms = r2s[r];
                someSyms.clear();
                for(auto& s: syms)
                    if(psym(s).getActive())
                        someSyms.push_back(s);  // cond : active?
                if( someSyms.empty() )          // no syms satisfy cond?
                    someRegs.push_back(r);
                syms = someSyms;                // focus on "cond" syms below
            }
            if(!someRegs.empty() && verbose) cout<<"[no active]"<<endl;
        }
        if(someRegs.empty()){ // no active syms inREG? (~ "free register")
            for(auto const r: regIds){
                auto & syms = r2s[r];
                someSyms.clear();
                for(auto& s: syms){
                    auto const& ps = psym(s);
                    assert( ps.getActive() );
                    if(ps.getREG()){
                        assert( ve::valid(psym(s).regId()));
                        someSyms.push_back(s);          // cond : [active and] inREG
                    }
                }
                if(verbose){cout<<"REG"<<r<<":";for(auto& s:someSyms)cout<<" "<<s;}
                if( someSyms.empty() )                  // no syms satisfy cond?
                    someRegs.push_back(r);              //    i.e. all syms for reg spilled to MEM
                else{
                    r2s[r] = someSyms;  // henceforth focus on activeREG symbols
                }
            }
            if(!someRegs.empty() && verbose) cout<<"[no activeREG]"<<endl;
            ties = fMEMstale | fLRUsed | fLRDecl | fANY;
        }
        if(someRegs.empty()){
            // All registers [should] have at least one active sym mapped there.
            // These syms might be in REG and/or MEM (or in REG with stale MEM)
            // Some regs are "easy" to re-use, in that they have REG=MEM value still OK
            // (You could create such regs by forcibly spilling something)
            for(auto const r: regIds){                    // all trivially-spillable? (old tEasy)
                auto & syms = r2s[r];           // (in REG && MEM, and MEM up to date)
                someSyms.clear();
                for(auto const& s: syms){
                    auto const& ps = psym(s);
                    assert( ps.getActive() );
                    if(ps.getREG() && ps.getMEM() && !ps.getStale() ){  // REG==MEM (spill is no-op)
                        assert( ve::valid(psym(s).regId()));
                        someSyms.push_back(s);          // cond : [active and] easy-spill
                    }
                }
                if( someSyms.size() == syms.size() )    // all syms satisfy cond [easy-spill] ?
                    someRegs.push_back(r);
            }
            //regIds = someRegs;  // focus only on these regs in future
            if(!someRegs.empty() && verbose) cout<<"[no-op spill]"<<endl;
            ties = fLRUsed | fLRDecl | fANY;
        }
#if 0 // all inMEM?
        if(someRegs.empty()){
            for(auto const r: regIds){                    // all inMEM?
                auto & syms = r2s[r];
                someSyms.clear();
                for(auto const& s: syms){
                    auto const& ps = psym(s);
                    assert( ps.getActive() );
                    if(ps.getMEM()){  // 
                        assert( ve::valid(psym(s).regId()));
                        someSyms.push_back(s);          // cond : [active and] inMEM
                    }
                }
                if( someSyms.size() == syms.size() ){    // all syms satisfy cond [inMEM] ?
                    someRegs.push_back(r);
                    syms = someRegs;
                }
            }
            // if(!someRegs.empty()) { break ties by staleness? usage time? decl time? } XXX
        }
#endif
        if(someRegs.empty()){
            if(verbose) cout<<"[def]"<<endl;
            for(auto const r: regIds)
                someRegs.push_back(r);
        }
        // NO assert( ! someRegs.empty() );
        if(1){
            cout<<"SpillOne";
            for(auto const& r: someRegs){
                cout<<",R"<<r<<"{";
                auto const& syms = r2s[r];
                for(auto const& s: syms) cout<<(s!=syms[0]?",":"")<<s;
                cout<<"}";
            }
            cout<<" ties{"
                <<((ties&fMEMstale) == fMEMstale? " stale":"")
                <<((ties&fLRUsed) == fLRUsed? " LRUsed":"")
                <<((ties&fLRDecl) == fLRDecl? " LRDecl":"")
                <<"}"<<endl;
        }
        // 
        // Break ties among registers by looking at min values of "GETTER"
        // amonst all associated symbols
        //
        // MINREGS(GETTER):
        // - for all someRegs,
        //   - find set of symbols with min values of RegSymbol::GETTER
        // - if unique min-valued reg,
        //   - then set reg,ok
        // - else new "best" someRegs is set of tied min-valued regs
        //
        // input: registers someRegs[]
        // output:  reg,ok=true if unique best register
        //       or reduced set of best registers
#define MINREGS( GETTER, PSASSERT ) do{ \
    typedef decltype(((RegSymbol*)nullptr)->GETTER()) Tm; \
    auto tieTm = std::numeric_limits<Tm>::max(); \
    for(auto const r: someRegs){ \
        auto const& syms = r2s[r]; \
        assert( !syms.empty() ); \
        auto minTm = std::numeric_limits<decltype(tieTm)>::max(); \
        for(auto const& s:syms){ \
            auto const& ps = psym(s); \
            PSASSERT; /*assert( ps.getMEM() );*/ \
            minTm = std::min( minTm, ps.GETTER() ); \
        } \
        if(minTm < tieTm){ \
            tiedRegs.clear(); \
            tiedRegs.push_back(r); \
            tieTm = minTm; \
        }else if(minTm == tieTm){ \
            tiedRegs.push_back(r); \
        } \
    } \
    assert( ! tiedRegs.empty() ); \
    if( tiedRegs.size() == 1U ) {reg = tiedRegs[0]; ok=true;} \
    else{ someRegs = tiedRegs; } \
}while(0)
        RegId reg = ve::invalidReg();
        bool ok=false;
        // NO assert( ! someRegs.empty() );
        if( !someRegs.empty() ){
            std::vector<RegId> tiedRegs = someRegs;
            if( !ok && (ties&fMEMstale) == fMEMstale ){
                MINREGS(getStale, assert(ps.getMEM()));
            }
            if( !ok && (ties&fLRUsed) == fLRUsed ){
                MINREGS(tSym, );
            }
            if( !ok && (ties&fLRDecl) == fLRDecl ){
                MINREGS(tDecl, );
            }
            if( !ok ){ // fANY
                for(auto const r: someRegs){
                    auto const& syms = r2s[r];
                    if( !syms.empty() ){
                        reg = syms[0];
                        ok = true; //sel = ve::invalidReg();
                        break;
                    }
                }
            }
        }
        unsigned retsymbol = 0U;
        { // given reg, return a symbol [that is mapped to the register], or zero
            if( !ok )
                std::cout<<" !ok? no register to spill? "<<std::endl;
            else if( reg == ve::invalidReg()) {
                std::cout<<" oh? no best register to spill? "<<std::endl;
            }else{
                if(verbose){
                    cout<<"\nFinal R"<<reg<<" syms{"; cout.flush();
                    assert( !r2s.empty() );
                    for(auto const& s: r2s[reg]) cout<<" "<<s;
                    cout<<"}"<<endl;
                }
                assert( !r2s[reg].empty() );
                retsymbol = r2s[reg][0]; // if mult symbols mapped, we really should "spill" them all XXX
            }
        }
        return retsymbol;
    }

    /** return # of active registers (by scanning all symbols) */
    unsigned nActiveRegs() const{
        return static_cast<unsigned>
            (
                std::count_if(psyms.begin(), psyms.end(),
                    [](auto const& p){ auto const& sObj = p.second;
                    if(sObj.getActive() && sObj.getREG()) {
                    assert(ve::valid(sObj.regId())); return true;}
                    else return false; })
            );
    }

    /** (test code) Say symbol is in reg, but only 4 registers available.
     * \c use4 assigns a register to \c symId if necessary.  If the 4 registers
     * in this pool are already used for other symbols, we spill one to memory.
     * This could mimic a <em>set of temporary registers</em>. */
    void use4(unsigned const symId){
        int const maxRegs=4;
        Psym &s = psym(symId);
        if( s.getActive() && s.getREG() ){
            cout<<" use4:mod"<<symId<<" "; cout.flush();
            //s.setREG(true); // in-register symbol modified its value.
            s.tWrite(nextTick());
            return;
        }
#if 0
        std::vector<unsigned> rpool({0,1,2,3});
        if( s.getActive() && s.getMEM() ){
            // we unspill.  [MAYBE even to previously used reg?]
            auto rid = spillOne( rpool, /*verbose*/1 );
            spill.spill

                auto nRegs = count_if(psyms.begin(), psyms.end(),
                        [](map<unsigned,Psym>::value_type const& v)
                        { return v.second.getREG(); } );
            cout<<" use4:nRegs"<<nRegs<<" "; cout.flush();
        }
#endif
        auto const nRegs = nActiveRegs();
        if( nRegs < maxRegs ){
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
};// end DemoSymbStates

void DemoSymbStates::dump() const {
    cout<<" DemoSymbStates: all "<<psyms.size()<<" symbols:"<<endl;
    for(auto const& p: psyms){
        cout<<"\t\t"<<p.second<<endl;
    }
}

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
    //typedef Ssym::Psym Psym;
    TEST("Construct some symbols");
    {
        //SpillableRegSym(unsigned const uid, char const* name,
        //          uint64_t const tick, Reg_t const rtype=REG_T(FREE|SCALAR));
        Ssym ssym;
        auto s=[&ssym](auto symId)->Ssym::Psym& {return ssym.psym(symId);};
        ASSERTTHROW( ssym.psym(1U).getActive() );
        auto const x = ssym.decl("x");          // declare unattached symbol
        cout<<" ssym.decl(\"x\") --> x="<<x<<", s(x)="<<s(x)<<endl;
        cout<<" decl "<<x<<endl;
        assert(x==1);
        assert( !ve::valid(s(x).regId()) );
        assert(ssym.nActiveRegs() == 0);        // x is active, but NOT in register


        auto myreg = ssym.nextRid();
        cout<<" next register id = "<<myreg<<endl;
        auto const y = ssym.decl(myreg,"y");    // assign register AND declare symbol
        cout<<" decl "<<y<<endl;
        assert(y==2);
        assert( ve::valid(s(y).regId()) );
        assert( s(y).getREG() );
        assert(ssym.nActiveRegs() == 1);        // y is active, AND in register
    }
    TEST("Again, with lambdas");
    {
        //SpillableRegSym(unsigned const uid, char const* name,
        //          uint64_t const tick, Reg_t const rtype=REG_T(FREE|SCALAR));
        Ssym ssym;
        // lookup symbol via symId
        auto s = [&ssym](auto symId)->Ssym::Psym& {
            return ssym.psym(symId);
        };
        // declare symbol                       declare, with no assigned register
        auto decl = [&ssym](auto name) {
            return ssym.decl(name);
        };
        // register symbol                      declare and assign register to symbol
        auto rsym = [&ssym](auto name) {
            return ssym.decl(ssym.nextRid(),name);
        };
        // register set                         set symbol to register
        // s(x).setReg(rid,tick)
        auto rset = [&ssym](ve::RegId const rid, unsigned symId){
            //std::cout<<"rset("<<rid<<","<<symId<<")..."; std::cout.flush();
            ssym.psym(symId).setReg( rid, ssym.nextTick() );
        };
        // auto-register set via nextRid()
        auto nextRid = [&ssym](unsigned symId){
            ssym.psym(symId).setReg( ssym.nextRid(), ssym.nextTick() );
        };
        auto const x = decl("x");               // declare unattached symbol
        cout<<" decl "<<s(x)<<endl;
        assert(ssym.nActiveRegs() == 0);        // x is active, but NOT in register
        auto const y = rsym("y");               // assign register AND declare symbol
        cout<<" decl "<<s(y)<<endl;
        assert(ssym.nActiveRegs() == 1);        // y is active, AND in register
        cout<<"    x "<<s(x)<<endl;
        cout<<"    y "<<s(x)<<endl;
        s(x).setReg(ssym.nextRid(), ssym.nextTick());
        cout<<" s(x).setReg... "<<s(x)<<endl;
        rset(ssym.nextRid(),x);
        cout<<" rset(nxt,x)... "<<s(x)<<endl;
        nextRid(x);
        cout<<" nextRid(x) ... "<<s(x)<<endl;
    }
    TEST("4-register dumb, cyclic register allocator...");
    {
        //SpillableRegSym(unsigned const uid, char const* name,
        //          uint64_t const tick, Reg_t const rtype=REG_T(FREE|SCALAR));
        Ssym ssym;
        //auto s = [&ssym](auto symId)->Ssym::Psym& { return ssym.psym(symId); };
        auto r = [&ssym](auto symId)->RegId       { return ssym.psym(symId).regId(); };
        //auto decl = [&ssym](auto name) { return ssym.decl(name); };
        auto r4 = [&ssym](auto name) { return ssym.decl(ssym.nextRid4(),name); };
        //auto rset = [&ssym](ve::RegId const rid, unsigned symId){ ssym.psym(symId).setReg( rid, ssym.nextTick() ); };
        //auto r4   = [&ssym](unsigned symId){ ssym.psym(symId).setReg( ssym.nextRid(), ssym.nextTick() ); };
#define R4(varname) auto const varname = r4(#varname)
        R4(a);
        R4(b);
        R4(c);
        R4(d);
        R4(e); ssym.dump();
        cout<<a<<b<<c<<d<<e<<" "<<r(a)<<" "<<r(b)<<" "<<r(c)<<" "<<r(d)<<" "<<r(e)<<endl;
        assert(r(a) == r(e));
        assert(ssym.nActiveRegs() == 5);   // OHOH, 5 is more than the 4 registers available
        cout<<"frown -- 2 active symbols can both map to a same register!\n"
            <<" (nextRid4() cyclic register 'allocator' is too dumb)"<<endl;
        // but the spill manager knows nothing about registers.
        // so we need our DemoSymbStates class to help us look for
        // unallocated registers (and if none, to spill something)
    }
    TEST("4-register demo, brutish check for spill...");
    {
        //SpillableRegSym(unsigned const uid, char const* name,
        //          uint64_t const tick, Reg_t const rtype=REG_T(FREE|SCALAR));
        Ssym ssym;
        auto s = [&ssym](auto symId)->Ssym::Psym& { return ssym.psym(symId); };
        auto r = [&ssym](auto symId)->RegId       { return ssym.psym(symId).regId(); };
        auto decl = [&ssym](auto name) { return ssym.decl(name); };
        auto r4 = [&ssym](auto name) { return ssym.decl(ssym.nextRid4(),name); };
        auto rset = [&ssym](ve::RegId const rid, unsigned symId){
            ssym.psym(symId).setReg( rid, ssym.nextTick() );
        };
        //auto r4   = [&ssym](unsigned symId){ ssym.psym(symId).setReg( ssym.nextRid(), ssym.nextTick() ); };
#define R4(varname) auto const varname = r4(#varname)
        ssym.spillOne();
        R4(a);
        ssym.spillOne();
        R4(b);
        ssym.spillOne();
        R4(c);
        ssym.spillOne();
        R4(d);
        cout<<a<<b<<c<<d<<" "<<r(a)<<" "<<r(b)<<" "<<r(c)<<" "<<r(d)<<endl;
        assert( ssym.nActiveRegs() == 4 );

        // set 'e', but now checking for spill (simple, but inefficient):
        auto rid = ssym.nextRid4();
        while( ssym.nActiveRegs() >= 4 ) {
            auto symId = ssym.spillOne();
            cout<<" selected symbol "<<s(symId)<<" to spill"<<endl;
            //
            // 3 ops to spill a reg and retrieve its stale regId for re-use
            //
            ssym.spill.spill(symId);
            // Note: now you have RegSymbol{a:+1R%s0M USED|SCALARd1u1}
            //       w/ REG and MEM identical.  Complete freeing REG ...
            s(symId).setREG(false);
            rid = ssym.psym(symId).regId(); // ... so we can re-use the reg
        }
        ssym.dump();
        auto const e = decl("e");
        rset(rid, e);
        cout<<" rset spilled rid "<<rid<<" to symbol "<<e<<endl;
        ssym.dump();
        assert(ssym.nActiveRegs() == 4);   // Yay, we spilled successfully
    }
    TEST("4-register demo, constrained reg-set for spill");
    {
        Ssym ssym;
        auto s = [&ssym](auto symId)->Ssym::Psym& { return ssym.psym(symId); };
        auto r = [&ssym](auto symId)->RegId       { return ssym.psym(symId).regId(); };
        auto decl = [&ssym](auto name) { return ssym.decl(name); };
        auto r4 = [&ssym](auto name) { return ssym.decl(ssym.nextRid4(),name); };
        auto rset = [&ssym](ve::RegId const rid, unsigned symId){
            ssym.psym(symId).setReg( rid, ssym.nextTick() );
        };
#define R4(varname) auto const varname = r4(#varname)
        R4(a);  // asm::mov(a,1)
        R4(b);  // asm::mov(b,77)
        R4(c);  // asm::add(c,a,b)
        R4(d);  // asm::add(d,a,b)
        //ssym.spillOne({2,3},1);
        if(1){ cout<<a<<b<<c<<d<<" "<<r(a)<<" "<<r(b)<<" "<<r(c)<<" "<<r(d)<<endl; }
        assert( ssym.nActiveRegs() == 4 );

        // set 'e', but now checking for spill (simple, but inefficient):
        auto rid = ssym.nextRid4();
        ssym.dump();
        if( ssym.nActiveRegs() == 4 ){
            auto symId = ssym.spillOne({2,3},1); // spill sym w/ regid in {2,3}, verbose
            cout<<" selected symbol "<<s(symId)<<" to spill"<<endl;
            // symbol in reg 2 should be oldest.
            assert( s(symId).regId() == 2 );
            ssym.spill.spill(symId);
            s(symId).setREG(false);
            rid = ssym.psym(symId).regId(); // ... so we can re-use the reg
        }
        ssym.dump();
        auto const e = decl("e");
        rset(rid, e);
        cout<<" rset spilled rid "<<rid<<" to symbol "<<e<<endl;

        ssym.dump();
        assert(ssym.nActiveRegs() == 4);   // OHOH, 5 is more than the 4 registers available
    }
#if 1
    TEST("R4 with example register read,write info");
    {
        Ssym ssym;
        auto s = [&ssym](auto symId)->Ssym::Psym& { return ssym.psym(symId); };
        //auto r = [&ssym](auto symId)->RegId       { return ssym.psym(symId).regId(); };
        //auto decl = [&ssym](auto name) { return ssym.decl(name); };
        //auto r4 = [&ssym](auto name) { return ssym.decl(ssym.nextRid4(),name); };
        auto r5 = [&ssym](auto name) {
            auto rid = ve::invalidReg();
            rid = ssym.nextRid4();
            if( ssym.nActiveRegs() >= 4 ){
                auto symId = ssym.spillOne({0,1,2,3},/*verbose*/1);
                cout<<" selected symbol "<<ssym.psym(symId)<<" to spill"<<endl;
                ssym.spill.spill(symId);
                ssym.psym(symId).setREG(false);
                rid = ssym.psym(symId).regId(); // ... so we can re-use the reg
            }
            auto symid = ssym.decl(rid,name);
            //ssym.psym(symid).setReg( rid, ssym.nextTick() );
            //ssym.use4(symid);
            return symid;
        };
        //auto rset = [&ssym](ve::RegId const rid, unsigned symId){
        //    ssym.psym(symId).setReg( rid, ssym.nextTick() );
        //};
        // assigns a "next" register to a symbol.
#if !defined(__ve)
#define R5(varname) auto const varname __attribute__((unused)) = r5(#varname)
#else
#define R5(varname) auto const varname = r5(#varname)
#endif
        R5(a); ssym.tWrite({a});       // asm::mov(a,1)
        R5(b); ssym.wr({b});           // asm::mov(b,2)
        R5(c); ssym.wr({c},{a,b});     // asm::add(c,a,b)
        R5(d); ssym.wr({d},{b,c});     // asm::add(d,b,c)
        cout<<" R4 x 4 :"<<endl;
        ssym.dump();
        // a is the LRU symbol
        R5(e); ssym.wr({e});            // asm::mov(e,1)
        assert( !s(a).getREG() );
        cout<<" R4 x 5 : a "<<s(a)<<"  e "<<s(e)<<endl;
        ssym.dump();
        R5(f); ssym.wr({f});            // asm::mov(f,1)
        //R6(g); // asm::add(g,c,e)  // PROBLEM: g uses c, so c should not be spilled.
        ssym.tRead({c,e}); R5(g); ssym.tWrite({g});       // better, but AWKWARD
        cout<<" R4 x 6 : a "<<s(a)<<"  e "<<s(e)<<endl;
        ssym.dump();
        assert( s(a).getMEM() && s(b).getMEM() && s(d).getMEM() );

        // e is no longer used
        s(e).setActive(false);  // this means 'e' is a high-prio spill item, even if used AFTER f
        ssym.tRead({c,g}); R5(h); ssym.tWrite({h});

        cout<<" R4 x final : "<<endl;
        ssym.dump();
        ssym.spill.dump();

        cout<<endl;
    }
#endif
#if 0
    TEST("Manual state adjustment of register-symbols");
    {
        Ssym ssym;
        auto s = [&ssym](auto symId)->Ssym::Psym& { return ssym.psym(symId); };
        auto r = [&ssym](auto symId)->RegId       { return ssym.psym(symId).regId(); };
        auto decl = [&ssym](auto name) { return ssym.decl(name); };
        //ssym.add(Psym(1U));  // symId 1, len=8, align=8 (64-bit register)
        // add --> SymIdMap::emplace( Psym::uid, Psym )
        //     --> psyms.emplace(psym.symId(),psym)
        //      or psyms.emplace(uid, SpillableBase(uid,bytes,align));
        // has been replace for this test by "decl"
        auto one = decl("one");
        assert( s(one).uid == 1U );
        auto two = decl("two");
        assert( s(two).uid == 2U );
        cout<<ssym.psym(one)<<" r"<<r(one)<<endl;
        cout<<ssym.psym(two)<<" r"<<r(two)<<endl;
        ssym.psym(one).setREG(true);
        assert( ssym.psym(one).getREG() == true );
        ssym.psym(one).setREG(false);
        assert( ssym.psym(one).getREG() == false );
        ssym.psym(one).setREG(true);
        assert( ssym.psym(one).getREG() == true );

        ssym.psym(one).setMEM(true);
        assert( ssym.psym(one).getMEM() == true );
        assert( ssym.psym(one).getREG() == true );
        ssym.psym(one).setMEM(false);
        assert( ssym.psym(one).getMEM() == false );
        assert( ssym.psym(one).getREG() == true );
        ssym.psym(one).setMEM(true);
        assert( ssym.psym(one).getMEM() == true );
        assert( ssym.psym(one).getREG() == true );
        cout<<ssym.psym(one)<<" r"<<r(one)<<endl;
        cout<<ssym.psym(two)<<" r"<<r(two)<<endl;
        cout<<endl;
    }
#endif
#if 0
    TEST("Construct and spill 2 [declared, unused] symbols is no-op");
    {
        Ssym ssym;
        //auto s = [&ssym](auto symId)->Ssym::Psym& { return ssym.psym(symId); };
        //auto r = [&ssym](auto symId)->RegId       { return ssym.psym(symId).regId(); };
        auto decl = [&ssym](auto name) { return ssym.decl(name); };
        ASSERTTHROW( ssym.psym(1U).getActive() );
        auto one = decl("one");
        auto two = decl("two");
        cout<<ssym.psym(one)<<"  "<<ssym.psym(two)<<endl;
        // If spill-impl.hpp is verbose, may print warnings:
        ssym.spill.spill(1);
        ssym.spill.spill(2);
        cout<<ssym.psym(one)<<"  "<<ssym.psym(two)<<endl;
    }
#endif
#if 0
    TEST("Construct and spill 2 [declared, used] symbols");
    {
        Ssym ssym;
        auto s = [&ssym](auto symId)->Ssym::Psym& { return ssym.psym(symId); };
        //auto r = [&ssym](auto symId)->RegId       { return ssym.psym(symId).regId(); };
        auto decl = [&ssym](auto name) { return ssym.decl(name); };
        auto one = decl("one");
        cout<<" add 1    : "<<s(one)<<endl;
        assert(s(one).getREG()==false);
        assert(s(one).getMEM()==false);
        assert(s(one).getStale()==0);
        cout<<" setREG(one)... ";
        s(one).setREG(true);   // say value is "in register"
        cout<<" setREG(one): "<<s(one)<<endl;
        assert(s(one).getREG()==true);
        assert(s(one).getMEM()==false);
        assert(s(one).getStale()==0);
        cout<<" spill(one) ... ";
        ssym.spill.dump();
        ssym.spill.spill(one);
        ssym.spill.dump();
        cout<<" spill(one) : "<<s(one)<<endl;
        assert(s(one).getREG()==true);
        assert(s(one).getMEM()==true);
        assert(s(one).getStale()==0);
        assert(ssym.spill.getBottom() == -8);
        auto two = decl("two");
        s(two).setREG(true);   // say value is "in register"
        ssym.spill.spill(two);
        assert(s(one).getREG()==true);
        assert(s(one).getMEM()==true);
        assert(s(one).getStale()==0);
        assert(s(two).getREG()==true);
        assert(s(two).getMEM()==true);
        assert(s(two).getStale()==0);
        assert(ssym.spill.getBottom() == -16);
        cout<<" spill(two) : "<<s(two)<<endl;
        ssym.spill.dump();
        cout<<endl;
    }
#endif
}
#if 0
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
        auto newscalar=[&ssym](unsigned id) {return ssym.add(Psym(id));};
        auto newscalarRM=[&ssym,&s,&spill](unsigned id) {
            ssym.add(Psym(id));
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
        //auto newscalar=[&ssym](unsigned id) {return ssym.add(Psym(id));};
        auto newscalarRM=[&ssym,&s,&spill](unsigned id) {
            ssym.add(Psym(id));
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
        //auto newscalar=[&ssym](unsigned id) {return ssym.add(Psym(id));};
        auto newscalarRM=[&ssym,&s,&spill](unsigned id) {
            ssym.add(Psym(id));
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
        auto newscalar=[&ssym](unsigned id) {return ssym.add(Psym(id));};

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
#endif
#if 0
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
        auto newscalar=[&ssym](unsigned id) {return ssym.add(Psym(id));};
        //auto newscalarRM=[&ssym,&s,&spill](unsigned id) {
        //    ssym.add(Psym(id));
        //    s(id).setREG(true);
        //    spill(id);
        //};
        for(unsigned i=1U; i<=10U; ++i) newscalar(i); // DECLARE 10 scalars
        for(unsigned i=1U; i<=10U; ++i){
            cout<<"\nUse symbol "<<i<<endl;     // assign register to 'i'
            ssym.use4(i);                       // (max 4 registers)
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
        auto newscalar=[&ssym](unsigned id) {return ssym.add(Psym(id));};
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
        auto newscalar=[&ssym](unsigned id) {return ssym.add(Psym(id));};
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
#endif

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
    //cout<<"======== test2() ==========="<<endl;
    //Tester::test2();
    //cout<<"======== test3() ==========="<<endl;
    //Tester::test3();
    for(auto s: randNames) delete[](s);
    randNames.clear();
    cout<<"\nGoodbye - " __FILE__ " ran "<<testNum<<" tests"<<endl;
    return 0;
}
// vim: ts=4 sw=4 et cindent cino=^=l0,\:.5s,=-.5s,N-s,g.5s,h.5s,b1 cinkeys=0{,0},0),\:,0#,!^F,o,O,e,0=break
