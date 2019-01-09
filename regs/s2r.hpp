#ifndef S2R_HPP
#define S2R_HPP
/** \file
 * injective SymId-->RegId map.
 */
#include "throw.hpp"
#include "reg-base.hpp"

#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <algorithm>    // std::find
#include <iomanip>


/**
 * S2R provides low-level bidirectional lookups for an
 * injective symId :--> RegId mapping.  The S<-->R associations
 * are peculiar in that a register may have a unique
 * \e strong symbol, multiple \e weak symbols, and any
 * number of \e unmapped symbols that can remember their
 * last known register assignment.  A symbol is either
 * never seen before, strong, weak or unmapped.
 * 
 * Registers and Symbols always start completely empty.
 * No validity checks on Register or Symbol range are done,
 * except for some checks or returns of /c invalidReg()
 *
 * Assembler read/write and symbol scope events change the symbol-register
 * associations.
 *
 * We distinguish a few \e types of association:
 *
 * - strong-assigned RegId <--> unique symbol
 * - weak-assigned   RegId <--> multiple symbol
 * - stale/empty     RegId empty
 *
 * This data structure does \b nothing for \e allocation of registers,
 * for which a user may have several [non-overlapping] pools active.
 *
 * Instead, this can monitor any global pool of RegId values, making
 * it independent of chipset. [invalidReg() must be chipset-independent.]
 * S2R begins empty.  Registers become known only through
 * \c sStrong(sid,RegId) or \c unmap(sid).
 *
 * Like symScopeUid, we maintain symId <--> RegId associations.
 *
 * This is a base class, \b not concerned with synchronizing with the
 * flags state in ScopedSpillableBase.
 *
 * Symbols get added.  Typically the symbol is first set \e strong,
 * then bumped out to \e weak, and finally becomes unmapped.  Symbols
 * are never completely forgotten. Even when unmapped, symbols remember
 * which register (if any) they came from.  The intent is to allow
 * a future symbol to re-use [if unmapped?] the same register for the
 * same symbol name (to make asm code maybe more understandable.
 *
 * header-only.
 */
class S2R
{
  public:
    typedef unsigned Sid;
    typedef RegId Rid;
    friend class S2rTester;
    static Rid const rBad = invalidReg();
    static Sid const sBad = 0U;

  private:
    /** A symbol is mapped to exactly zero or one registers. */
    std::unordered_map<Sid,Rid> sReg_;

    /** A Register can have zero or one strong symbol associations */
    std::unordered_map<Rid,Sid> strong_;

    /** A Register can have any number of weak symbols assigned to it.
     *
     * Because \c mkStrong always moves strong--> weak_ via \c push_back,
     * \c weak_ inherits a register-specific notion of 'symbol staleness'.
     * But this \e doesn't allow you to compare staleness across registers.
     * (std::unordered_set might be a faster choice then).
     */
    std::unordered_map<Rid,std::vector<Sid>> weak_;

    /** For efficiency we'll keep separate record of old and weak symbols */
    std::unordered_set<Sid> sOld_;
    std::unordered_set<Sid> sWeak_;
    //std::unordered_set<Sid,Rid> sStrong; // this is easy to interrogate
  public:
    S2R()               = default;
    S2R(S2R const&)     = delete;
  public:
    void dump() const; // to cout
    //@{
    /** Symbols have 4 disjoint states: strong, weak, unmapped, or
     * absent (ie not in our data structure).  We retain unmapped
     * symbols because they might have some <em>previous register</em>
     * info that we by default retain forever.  We'll never track
     * an unmapped symbol associated with rBad (i.e. without some
     * associated register). */
    bool isStrong   (Sid const s) const;
    bool isWeak     (Sid const s) const;
    bool isOld      (Sid const s) const;
    //@}
    //@{
    /** a RegId can map to multiple symbols, of different types, we
     * \e hasFoo(RegId) to find out if registers have \e any symbols of
     * type \e Foo associated. */
    bool hasStrong   (RegId const r) const;
    bool hasWeak     (RegId const r) const;
    bool hasUnmapped (RegId const r) const; ///< also includes completely unknown \c r
    //@}

    /** Symbol --> Register.
     * \return rBad iff \c s is unseen.
     * \throw never */
    RegId  reg       (Sid const s) const;

    //@{
    /** Register --> Symbol.  \c old is not very efficient.  */
    Sid                     strong (RegId const r) const; ///< \return sBad if none
    std::vector<Sid> const& weaks  (RegId const r) const; ///< \return empty vector if none
    std::vector<Sid>        old    (RegId const r) const; ///< \return empty vector if none
    std::unordered_map<Rid,Sid> const& strongs() const {return strong_;}
    //@}

  public: // modifying operations
    /** Given symbol \c s, make it strong to the given RegId \c r.
     * Deals with \e previous \c reg(s) \f$r_old\f$ being different or same as \c r.
     * \post \c s is the unique strong symbol for \c r.
     * \post a previous /e strong symbol for \c r moves to the /e weak list of \c r
     * \post if \f$r_old\f$ != \c r, \c s is removed from both strongs and weaks of \f$r_old\f$
     * Also works if \c s has never been seen.
     *
     * - After \c mkStrong(s,r), \c s will automatically transition to weak when a later
     *   \c mkStrong(\f$s_new\f$,r) happens.
     *   - Ex. register spill manager needs an extra register and bumps out \c s.
     * - Or it can transition away from \e strong when a later \c unmap(s) happens.
     *   - Ex. symbol scope manager does and \c end_scope()... Do we:
     *     - remove the strong mapping entry?
     *     - replace the strong mapping entry with \c s --> \c rBad ?
     *     - or just leave the strong association even if the symbol is no longer active?
     *
     * \throw if \c r==invalidReg()
     */ 
    void mkStrong(Sid const s, RegId const r);

    /** Given a symbol \c s ensure its register pairing is weak.
     * - If in \c strong_, the symbol moves \c weak_ [appended]
     * - If already \c weak_, do nothing.  [ ? reposition at end ? ]
     * \throw if \c s has no register info or is unmapped.
     * \return \c s's paired register.
     * When paired with some 'end_scope' all scope symbols transition strong-->weak
     */
    RegId mkWeak(Sid const s);

    /** unmap symbol \c s from any strong/weak register associations.
     * - if already \c isOld(s) or \c s has no info, do nothing.
     * - else move \c (s,\f$r_prev\f$) to unmapped symbol, removing its
     *   previous strong/weak register association.
     *
     * You might not need to ever unmap any symbol.
     * \return last known register associated with \c s (or invalidReg())
     * \post !isStrong(s) && !isWeak(s) [\e maybe isOld(s)]
     */
    RegId unmap(Sid const s);
    // Maybe Sid should be a real enum type so we cannot mix up RegId with symbol id
    // via c++ auto-conversions ?
    //void unmap(RegId const r){ THROW("You can't unmap(RegId)"); }

#if 0
    /** Given a register \c r ensure it has no strongly paired symbol.
     * If \c strong_ is empty already, do nothing and re
     * If \c strong_ is not empty, that symbol, \c s, moves to \c weak_ and we return \c s
     * \throw if \c s has no register info.
     * \return \c s's paired register.
     */
    void rWeak(RegId const r);
#endif
  private:
    /** internal helper.
     * \return any prev reg of \c s, or rBad if \c s is completely absent.
     * This is similar to \c unmap,
     * but instead of moving old strong-/weak-assignments to sOld_,
     * removes \c s completely from our data structure. */
    RegId erase(Sid const s);
#if 0
    /** return a sorted list of all RegId for which we have some information. */
    std::vector<RegId> allKnownRegs() const;
#endif
  public:
    /** \group generate compile-time errors if use RegId instead of Sid arg
     * - This can happen unintentionally if you 'auto' things.
     * - You need to RegId(i) to convert int_type i to RegId.
     * - Sid-->RegId does not occur because RegId is a \e nice typed enum.
     * - But RegId-->Sid conversion happens automatically.
     * - So to avoid RegId-->Sid conversions resulting from typing errors...
     */
    //@{ compile-time error for potential typos
    // A RegId \e could
    bool isStrong(RegId const r) const = delete;
    bool isWeak  (RegId const r) const = delete;
    bool isOld   (RegId const r) const = delete;
    RegId  reg   (RegId const r) const = delete;;
    //@}
};

inline RegId
S2R::reg(Sid const s) const {
    auto const sr = sReg_.find(s);
    assert( !( s==sBad && sr!=sReg_.end() ) );
    return sr==sReg_.end()? rBad: sr->second;
}
inline S2R::Sid
S2R::strong(RegId const r) const{
    auto const rs = strong_.find(r);
    return rs == strong_.end()? sBad: rs->second;
}
inline std::vector<S2R::Sid> const&
S2R::weaks(RegId const r) const{
    static std::vector<Sid> x;
    auto const sw = weak_.find(r);
    return (sw == weak_.end()? x: sw->second);
}
inline bool
S2R::isStrong(Sid const s) const {
    auto const r = reg(s); // reg(s) is rbad if s completely unknown
    return r != rBad && strong(r) == static_cast<Sid>(s); // strong(r) is sbad if r is not in strong_
}
inline bool
S2R::isWeak(Sid const s) const {
    bool ret = sWeak_.count(s); // 0 or 1
    return ret;
}
inline bool
S2R::isOld(Sid const s) const {
    bool ret = sOld_.count(s);
    return ret;
}
inline bool
S2R::hasStrong(RegId const r) const {
    return strong_.find(r) != strong_.end();
}
inline bool
S2R::hasWeak(RegId const r) const {
    return weak_.find(r) != weak_.end();
}
inline bool
S2R::hasUnmapped(RegId const r) const {
    return !hasStrong(r) && !hasWeak(r);
}
std::vector<S2R::Sid> S2R::old(RegId const r) const {
    std::vector<Sid> ret;
    for(auto const s: sOld_){
        if (reg(s) == r){
            ret.push_back(s);
        }
    }
    return ret;
}
RegId S2R::erase(Sid const s){
    RegId rPrev = rBad;
    auto const sr = sReg_.find(s);
    if(sr != sReg_.end()){
        // OK, it must be in one of strong_, weak_ or sOld_
        rPrev = sr->second;
        assert( rPrev != rBad );
        if (sOld_.erase(s) == 0){
            // it was not in sOld_ ... try to erase it from strong:
            if (strong_.erase(rPrev) == 0){
                // was mapped, wasn't old, wasn't strong... Heh! Must be weak
                // erase (s<-->r) from sWeak_ (set) and weak_ (map)
                auto const sw = sWeak_.find(s);
                assert( sw != sWeak_.end() );
                sWeak_.erase(sw);

                auto rw = weak_.find(rPrev);
                assert( rw != weak_.end() );
                auto & r_weaks = rw->second; // vector<Sid>, no 'find(key)' op
                // kinda slow, but hopefully infrequent...
                auto rws = std::find(r_weaks.begin(),r_weaks.end(),s);
                assert( rws != r_weaks.end() );
                r_weaks.erase(rws); // kinda slow, but hopefully infrequent
                // ........................................
                //if (r_weaks.empty()){ weak_.erase(rw); } // optional tidying
                // if untidied, might reduce vector allocs
            }
        }
        sReg_.erase(sr); // <--- remove completely (compare with S2R::unmap)
    }
    assert( !isStrong(s) );
    assert( !isWeak(s) );
    assert( !isOld(s) );
    return rPrev;
}
inline void
S2R::mkStrong(Sid const s, RegId const r){
    assert( s != sBad );
    assert( r != rBad );
    Sid sPrev = strong(r);
    if (sPrev != s){
        if (sPrev != sBad){ // move sPrev to back of weaks_[r] vector
            weak_[r].push_back(sPrev);
            sWeak_.insert(sPrev);
            //std::cout<<" sPrev-->weak!"<<std::endl;
            assert( isWeak(sPrev) );
        }
        erase(s);
        strong_[r] = s;
        sReg_[s] = r;
    }// else s is already strong-assigned to r (no-op)
}
inline RegId
S2R::unmap(Sid const s){
    assert( s != sBad );
    RegId const r = reg(s);
    if( r != rBad ){
        //std::cout<<" unmap(s="<<s<<" -->"<<r<<")"<<std::endl;
        if (!isOld(s)){
            // move strong/weak-assign to old .................
            if (strong_.erase(r) == 0) {
                // erase (s<-->r) from sWeak_ (set) and weak_ (map)
                assert( isWeak(s) );
                auto const sw = sWeak_.find(s);
                assert( sw != sWeak_.end() );
                sWeak_.erase(sw);

                auto rw = weak_.find(r);
                assert( rw != weak_.end() );
                auto & r_weaks = rw->second; // vector<Sid>, no 'find(key)' op
                // kinda slow, but hopefully infrequent...
                auto rws = std::find(r_weaks.begin(), r_weaks.end(), s);
                assert( rws != r_weaks.end() );
                r_weaks.erase(rws);
                // ........................................
                //if (r_weaks.empty()){ weak_.erase(rw); } // optional tidying
                // if untidied, might reduce vector allocs
                std::cout<<" unmap("<<s<<") prev "<<r<<" erased from weak_["<<r<<"]"<<std::endl;
            }
            sOld_.insert(s); // <-- just move to 'old' (compare with S2R::erase)
        }
        assert( isOld(s) );
    }
    assert(!isStrong(s) );
    assert(!isWeak(s) );
    return r;
}
#if 1
inline RegId
S2R::mkWeak(Sid const s){
    auto const sr = sReg_.find(s);
    if (sr==sReg_.end())
        THROW("Cannot mkWeak(unknown symbol "<<s<<")");
    if (isOld(s))
        THROW("Cannot mkWeak(unmapped symbol "<<s<<")");
    RegId rid = sr->second;
    if (isStrong(s)){
        strong_.erase(rid);
        weak_[rid].push_back(s);
        sWeak_.insert(s);
        assert( !isStrong(s) );
        assert( isWeak(s) );
        assert( !hasStrong(rid) );
        assert( hasWeak(rid) );
        //std::cout<<" Yay. Sid "<<s<<" is now weakly attached to "<<rid<<std::endl;
    }
    return rid;
}
#endif
void S2R::dump() const {
    std::cout<<" Reg      Syms... (S~Strong, W~Weak, O~Old)"<<std::endl;
    std::unordered_set<Rid> rDone;
    std::unordered_set<Sid> ss; // Sids appear once
    for(auto const& rs: strong_){
        Rid const r = rs.first;
        Sid const s = rs.second;
        ss.insert(s);
        std::cout<<std::setw(4)<<r<<" "<<std::setw(4)<<asmname(r)<<" S "<<std::setw(4)<<s;
        assert( isStrong(s) );
        assert( !isWeak(s) );
        assert( !isOld(s) );
        auto const rw = weak_.find(r);
        if (rw != weak_.end()){
            std::cout<<" w "; std::cout.flush();
            for(auto const wk: rw->second){
                std::cout<<" "<<std::setw(3)<<wk; std::cout.flush();
                assert( ss.count(wk) == 0 );
                // NO? XXX assert( !isStrong(wk) );
                assert( isWeak(wk) );
                assert( !isOld(wk) );
                ss.insert(wk);
            }
        }
        auto oldr = old(r);
        if( oldr.size() ){
            std::cout<<" O "; std::cout.flush();
            for(auto const o: oldr){
                std::cout<<" "<<std::setw(3)<<o; std::cout.flush();
                assert( ss.count(o) == 0 );
                assert( !isStrong(o) );
                assert( !isWeak(o) );
                assert( isOld(o) );
                ss.insert(o);
            }
        }
        rDone.insert(r);
        std::cout<<std::endl; std::cout.flush();
    }
    for (auto const& rw: weak_){
        Rid const r = rw.first;
        if (rDone.count(r) == 0){
            std::cout<<std::setw(4)<<r<<" "<<std::setw(4)<<asmname(r)<<" W ";
            std::cout.flush();
            auto const& ws = rw.second;         // vector<Sid>
            for(auto const wk: ws){
                std::cout<<" "<<std::setw(3)<<wk;
                std::cout.flush();
                assert( ss.count(wk) == 0 );
                ss.insert(wk);
                assert( !isStrong(wk) );
                assert( !isOld(wk) );
                //std::cout<<std::endl<<" isWeak("<<wk<<") ??? "; std::cout.flush();
                assert( isWeak(wk) );
            }
            auto oldr = old(r);
            if( oldr.size() ){
                std::cout<<" O "; std::cout.flush();
                for(auto const o: oldr){
                    std::cout<<" "<<std::setw(3)<<o;
                    std::cout.flush();
                    assert( ss.count(o) == 0 );
                    assert( !isStrong(o) );
                    assert( !isWeak(o) );
                    assert( isOld(o) );
                    ss.insert(o);
                }
            }
            rDone.insert(r);
            std::cout<<std::endl; std::cout.flush();
        }
    }
    for (Sid const s: sOld_){
        auto const& sr = sReg_.find(s);
        assert( sr != sReg_.end() );
        Rid const r = sr->second;
        if (rDone.count(r) == 0){
            std::cout<<std::setw(4)<<r<<" "<<std::setw(4)<<asmname(r)<<" O "<<std::setw(4)<<s;
            std::cout.flush();
            assert( ss.count(s) == 0 );
            assert( !isStrong(s) );
            assert( !isWeak(s) );
            assert( isOld(s) );
            ss.insert(s);
            rDone.insert(r);
            std::cout<<std::endl; std::cout.flush();
        }
    }
    std::cout<<std::endl;
}
//
// ------------------------------- done header : self-test follows -------------------------
//
#ifdef S2R_MAIN
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

using namespace std;

struct S2rTester {
    static void test1();
};

void S2rTester::test1() {
    TEST("construct, print, [verify?,] destruct");
    {
        S2R sr;
        sr.dump();
        assert( sr.strong_.size() == 0 );
        assert( sr.weak_.size() == 0);
        assert( sr.sReg_.size() == 0 );
        assert( sr.sOld_.size() == 0 );
        assert( sr.sWeak_.size() == 0 );
        for(unsigned i=0; i<4; ++i){
            // register
            assert(!sr.hasStrong(RegId(i)) );
            assert(!sr.hasWeak(RegId(i)) );
            assert( sr.hasUnmapped(RegId(i)) ); // all unknown RegId are 'Unmapped' XXX
            assert( sr.strong(RegId(i)) == S2R::sBad );
            assert( sr.weaks(RegId(i)).size() == 0 );
            assert( sr.old(RegId(i)).size() == 0 );
            // symbol (no symbols, does not make much sense)
            assert(!sr.isStrong(i) );
            assert(!sr.isWeak(i) );
            assert(!sr.isOld(i) );
        }
    }
    TEST("unique strong-assign registers: expect strongs");
    {
        S2R sr;
        // r 0 is valid reg number, symbols start at 1
        for(int i=0; i<4; ++i) {
            sr.mkStrong(100+i,RegId(i));
        }
        sr.dump();
        assert( sr.strong_.size() == 4 );
        assert( sr.weak_.size() == 0 );
        assert( sr.sReg_.size() == 4 );
        assert( sr.sOld_.size() == 0 );
        assert( sr.sWeak_.size() == 0 );
        for(unsigned i=0; i<4; ++i){
            // register
            assert( sr.hasStrong(RegId(i)) );
            assert(!sr.hasWeak(RegId(i)) );
            assert(!sr.hasUnmapped(RegId(i)) );
            assert( sr.strong(RegId(i)) == S2R::Sid{100+i} );
            assert( sr.weaks(RegId(i)).size() == 0 );
            assert( sr.old(RegId(i)).size() == 0 );
            // symbol
            assert( sr.isStrong(100+i) );
            assert(!sr.isWeak(100+i) );
            assert(!sr.isOld(100+i) );
        }
    }
    TEST("twice-assigned registers: expect strongs and weaks");
    {
        S2R sr;
        // r 0 is valid reg number, symbols start at 1
        for(int i=0; i<4; ++i) {
            sr.mkStrong(100+i,RegId(i));
            assert( sr.isStrong(100+i) );
        }
        for(int i=0; i<4; ++i) {
            sr.mkStrong(200+i,RegId(i));
            assert( sr.isStrong(200+i) );
            assert( sr.isWeak(100+i) );
            assert(!sr.isStrong(100+i) );
        }
        sr.dump();
        assert( sr.strong_.size() == 4 );
        assert( sr.weak_.size() == 4 );
        assert( sr.sReg_.size() == 8 );
        assert( sr.sOld_.size() == 0 );
        assert( sr.sWeak_.size() == 4 );
        for(unsigned i=0; i<4; ++i){
            // register
            assert( sr.hasStrong(RegId(i)) );
            assert( sr.hasWeak(RegId(i)) );
            assert(!sr.hasUnmapped(RegId(i)) );
            assert( sr.strong(RegId(i)) == S2R::Sid{200+i} );
            assert( sr.weaks(RegId(i)).size() == 1 );
            assert( sr.old(RegId(i)).size() == 0 );
            // symbols
            assert(!sr.isStrong(100+i) );
            assert( sr.isWeak(100+i) );
            assert(!sr.isOld(100+i) );

            assert( sr.isStrong(200+i) );
            assert(!sr.isWeak(200+i) );
            assert(!sr.isOld(200+i) );
        }
    }
    TEST("thrice-assigned registers: expect strongs and 2 weaks");
    {
        S2R sr;
        // r 0 is valid reg number, symbols start at 1
        for(int i=0; i<4; ++i) {
            sr.mkStrong(100+i,RegId(i));
            assert( sr.isStrong  (100+i) );
            assert(!sr.isWeak    (100+i) );
            assert(!sr.isOld(100+i) );
        }
        for(int i=0; i<4; ++i) {
            sr.mkStrong(200+i,RegId(i));
            assert( sr.isStrong  (200+i) );
            assert(!sr.isWeak    (200+i) );
            assert(!sr.isOld(200+i) );
            assert( sr.isWeak    (100+i) );
            assert(!sr.isStrong  (100+i) );
            assert(!sr.isOld(100+i) );
        }
        for(int i=0; i<4; ++i) {
            sr.mkStrong(300+i,RegId(i));
            assert( sr.isStrong  (300+i) );
            assert(!sr.isWeak    (300+i) );
            assert(!sr.isOld(300+i) );
            assert(!sr.isStrong  (200+i) );
            assert( sr.isWeak    (200+i) );
            assert(!sr.isOld(200+i) );
            assert(!sr.isStrong  (100+i) );
            assert( sr.isWeak    (100+i) );
            assert(!sr.isOld(100+i) );
        }
        sr.dump();
        assert( sr.strong_.size() == 4 );
        assert( sr.weak_.size() == 4 );
        assert( sr.sReg_.size() == 12 );
        assert( sr.sOld_.size() == 0 );
        assert( sr.sWeak_.size() == 8 );
        for(unsigned i=0; i<4; ++i){
            // register
            assert( sr.hasStrong  (RegId(i)) );
            assert( sr.hasWeak    (RegId(i)) );
            assert(!sr.hasUnmapped(RegId(i)) );
            assert( sr.strong(RegId(i)) == S2R::Sid{300+i} );
            assert( sr.weaks (RegId(i)).size() == 2 );
            assert( sr.old   (RegId(i)).size() == 0 );
            // symbols
            assert(!sr.isStrong  (100+i) );
            assert( sr.isWeak    (100+i) );
            assert(!sr.isOld(100+i) );

            assert(!sr.isStrong  (200+i) );
            assert( sr.isWeak    (200+i) );
            assert(!sr.isOld(200+i) );

            assert( sr.isStrong  (300+i) );
            assert(!sr.isWeak    (300+i) );
            assert(!sr.isOld(300+i) );
        }
    }
    TEST("thrice-strong, then unmap strongs: expect 2 weaks and 1 old");
    {
        S2R sr;
        // r 0 is valid reg number, symbols start at 1
        for(int i=0; i<4; ++i) {
            sr.mkStrong(100+i,RegId(i));
        }
        for(int i=0; i<4; ++i) {
            sr.mkStrong(200+i,RegId(i));
        }
        for(int i=0; i<4; ++i) {
            sr.mkStrong(300+i,RegId(i));
            assert( sr.isStrong  (300+i) );
            assert(!sr.isWeak    (300+i) );
            assert(!sr.isOld(300+i) );
            assert(!sr.isStrong  (200+i) );
            assert( sr.isWeak    (200+i) );
            assert(!sr.isOld(200+i) );
            assert(!sr.isStrong  (100+i) );
            assert( sr.isWeak    (100+i) );
            assert(!sr.isOld(100+i) );
        }
        for(int i=0; i<4; ++i) {
            sr.unmap(300+i);
            assert(!sr.isStrong  (300+i) );
            assert(!sr.isWeak    (300+i) );
            assert( sr.isOld(300+i) );
            assert(!sr.isStrong  (200+i) );
            assert( sr.isWeak    (200+i) );
            assert(!sr.isOld(200+i) );
            assert(!sr.isStrong  (100+i) );
            assert( sr.isWeak    (100+i) );
            assert(!sr.isOld(100+i) );
        }
        sr.dump();
        assert( sr.strong_.size() == 0 );
        assert( sr.weak_.size() == 4 );
        assert( sr.sReg_.size() == 12 );
        assert( sr.sOld_.size() == 4 );
        assert( sr.sWeak_.size() == 8 );

        sr.unmap(1313); // unmap non-existant symbol (works but does NOTHING)
        assert( !sr.isStrong(1313) && !sr.isWeak(1313) && !sr.isOld(1313) );

        for(unsigned i=0; i<4; ++i){
            // register
            assert(!sr.hasStrong  (RegId(i)) );
            assert( sr.hasWeak    (RegId(i)) );
            assert(!sr.hasUnmapped(RegId(i)) );

            assert( sr.strong(RegId(i)) == S2R::sBad );
            assert( sr.weaks (RegId(i)).size() == 2 );
            assert( sr.old   (RegId(i)).size() == 1 );
            // symbols
            assert(!sr.isStrong  (100+i) );
            assert( sr.isWeak    (100+i) );
            assert(!sr.isOld(100+i) );

            assert(!sr.isStrong  (200+i) );
            assert( sr.isWeak    (200+i) );
            assert(!sr.isOld(200+i) );

            assert(!sr.isStrong  (300+i) );
            assert(!sr.isWeak    (300+i) );
            assert( sr.isOld(300+i) );
        }
    }
    TEST("thrice-assigned, unmap+strong again: expect 1 strong, 2 weaks again");
    {
        S2R sr;
        // r 0 is valid reg number, symbols start at 1
        for(int i=0; i<4; ++i) {
            sr.mkStrong(100+i,RegId(i));
            assert( sr.isStrong  (100+i) );
            assert(!sr.isWeak    (100+i) );
            assert(!sr.isOld(100+i) );
        }
        for(int i=0; i<4; ++i) {
            sr.mkStrong(200+i,RegId(i));
            assert( sr.isStrong  (200+i) );
            assert(!sr.isWeak    (200+i) );
            assert(!sr.isOld(200+i) );
            assert( sr.isWeak    (100+i) );
            assert(!sr.isStrong  (100+i) );
            assert(!sr.isOld(100+i) );
        }
        for(int i=0; i<4; ++i) {
            sr.mkStrong(300+i,RegId(i));
            assert( sr.isStrong  (300+i) );
            assert(!sr.isWeak    (300+i) );
            assert(!sr.isOld(300+i) );
            assert(!sr.isStrong  (200+i) );
            assert( sr.isWeak    (200+i) );
            assert(!sr.isOld(200+i) );
            assert(!sr.isStrong  (100+i) );
            assert( sr.isWeak    (100+i) );
            assert(!sr.isOld(100+i) );
        }
        for(int i=0; i<4; ++i) {
            sr.unmap(300+i);
        }
        for(int i=0; i<4; ++i) {
            sr.mkStrong(300+i,RegId(i));
            assert( sr.isStrong  (300+i) );
            assert(!sr.isWeak    (300+i) );
            assert(!sr.isOld(300+i) );
            assert(!sr.isStrong  (200+i) );
            assert( sr.isWeak    (200+i) );
            assert(!sr.isOld(200+i) );
            assert(!sr.isStrong  (100+i) );
            assert( sr.isWeak    (100+i) );
            assert(!sr.isOld(100+i) );
        }
        sr.dump();
        assert( sr.strong_.size() == 4 );
        assert( sr.weak_.size() == 4 );
        assert( sr.sReg_.size() == 12 );
        assert( sr.sOld_.size() == 0 );
        assert( sr.sWeak_.size() == 8 );
        for(unsigned i=0; i<4; ++i){
            // register
            assert( sr.hasStrong  (RegId(i)) );
            assert( sr.hasWeak    (RegId(i)) );
            assert(!sr.hasUnmapped(RegId(i)) );
            assert( sr.strong(RegId(i)) == S2R::Sid{300+i} );
            assert( sr.weaks (RegId(i)).size() == 2 );
            assert( sr.old   (RegId(i)).size() == 0 );
            // symbols
            assert(!sr.isStrong  (100+i) );
            assert( sr.isWeak    (100+i) );
            assert(!sr.isOld(100+i) );

            assert(!sr.isStrong  (200+i) );
            assert( sr.isWeak    (200+i) );
            assert(!sr.isOld(200+i) );

            assert( sr.isStrong  (300+i) );
            assert(!sr.isWeak    (300+i) );
            assert(!sr.isOld(300+i) );
        }
    }
    TEST("thrice-assigned, unmap, wk-->strong: expect 1 each strong,weak,old");
    {
        S2R sr;
        // r 0 is valid reg number, symbols start at 1
        for(int i=0; i<4; ++i) {
            sr.mkStrong(100+i,RegId(i));
        }
        for(int i=0; i<4; ++i) {
            sr.mkStrong(200+i,RegId(i));
        }
        for(int i=0; i<4; ++i) {
            sr.mkStrong(300+i,RegId(i));
        }
        for(int i=0; i<4; ++i) {
            sr.unmap(300+i);
        }
        for(int i=0; i<4; ++i) {
            sr.mkStrong(100+i,RegId(i));
            assert( sr.isStrong  (100+i) );
            assert(!sr.isWeak    (100+i) );
            assert(!sr.isOld(100+i) );
            assert(!sr.isStrong  (200+i) );
            assert( sr.isWeak    (200+i) );
            assert(!sr.isOld(200+i) );
            assert(!sr.isStrong  (300+i) );
            assert(!sr.isWeak    (300+i) );
            assert( sr.isOld(300+i) );
        }
        sr.dump();
        assert( sr.strong_.size() == 4 );
        assert( sr.weak_.size() == 4 );
        assert( sr.sReg_.size() == 12 );
        assert( sr.sOld_.size() == 4 );
        assert( sr.sWeak_.size() == 4 );
        for(unsigned i=0; i<4; ++i){
            // register
            assert( sr.hasStrong  (RegId(i)) );
            assert( sr.hasWeak    (RegId(i)) );
            assert(!sr.hasUnmapped(RegId(i)) );
            assert( sr.strong(RegId(i)) == S2R::Sid{100+i} );
            assert( sr.weaks (RegId(i)).size() == 1 );
            assert( sr.old   (RegId(i)).size() == 1 );
            // symbols
            assert( sr.isStrong  (100+i) );
            assert(!sr.isWeak    (100+i) );
            assert(!sr.isOld(100+i) );
            assert( sr.reg(100+i) == RegId(i) );

            assert(!sr.isStrong  (200+i) );
            assert( sr.isWeak    (200+i) );
            assert(!sr.isOld(200+i) );
            assert( sr.reg(200+i) == RegId(i) );

            assert(!sr.isStrong  (300+i) );
            assert(!sr.isWeak    (300+i) );
            assert( sr.isOld(300+i) );
            assert( sr.reg(300+i) == RegId(i) );
        }
    }
    TEST("thrice-assigned, erase strong, wk-->strong: expect 1 strong, 1 weak (erase is a private fn!)");
    {
        S2R sr;
        // r 0 is valid reg number, symbols start at 1
        for(int i=0; i<4; ++i) {
            sr.mkStrong(100+i,RegId(i));
        }
        for(int i=0; i<4; ++i) {
            sr.mkStrong(200+i,RegId(i));
        }
        for(int i=0; i<4; ++i) {
            sr.mkStrong(300+i,RegId(i));
        }
        for(int i=0; i<4; ++i) {
            sr.erase(300+i);            // this is a private function
        }
        for(int i=0; i<4; ++i) {
            sr.mkStrong(100+i,RegId(i));
        }
        sr.dump();
        assert( sr.strong_.size() == 4 );
        assert( sr.weak_.size() == 4 );
        assert( sr.sWeak_.size() == 4 );
        assert( sr.sReg_.size() == 8 );
        assert( sr.sOld_.size() == 0 );
        for(unsigned i=0; i<4; ++i){
            // register
            assert( sr.hasStrong  (RegId(i)) );
            assert( sr.hasWeak    (RegId(i)) );
            assert(!sr.hasUnmapped(RegId(i)) );
            assert( sr.strong(RegId(i)) == S2R::Sid{100+i} );
            assert( sr.weaks (RegId(i)).size() == 1 );
            assert( sr.old   (RegId(i)).size() == 0 );
            // symbols
            assert( sr.isStrong  (100+i) );
            assert(!sr.isWeak    (100+i) );
            assert(!sr.isOld(100+i) );
            assert( sr.reg(100+i) == RegId(i) );

            assert(!sr.isStrong  (200+i) );
            assert( sr.isWeak    (200+i) );
            assert(!sr.isOld(200+i) );
            assert( sr.reg(200+i) == RegId(i) );

            assert(!sr.isStrong  (300+i) );
            assert(!sr.isWeak    (300+i) );
            assert(!sr.isOld(300+i) );       // erase ==> completely gone
            assert( sr.reg(300+i) == rBad );

            assert(!sr.isStrong  (400+i) );
            assert(!sr.isWeak    (400+i) );
            assert(!sr.isOld(400+i) );
            assert( sr.reg(400+i) == rBad );
        }
    }
    TEST("Regid arg when symbol id expected (now a compile-time error)");
    {
        S2R sr;
        // even better than throwing, can catch at compile time
        // error: use of deleted function ‘bool S2R::isStrong(RegId) const
        //ASSERTTHROW( sr.isStrong(rBad) );

        // also a compile-time error (cannot check nicely in tests)
        //ASSERTTHROW( sr.isStrong(RegId(17)) );

        // also a compile-time error
        //ASSERTTHROW( sr.isStrong(RegId(sr.reg(S2R::sBad))) );

        RegId r = sr.reg(S2R::sBad);
        assert( r == rBad );
    }
}
int main(int,char**)
{
    S2rTester::test1();
    cout<<"\nGoodbye -- passed "<<testNum<<" tests"<<endl;
}
#endif//S2R_MAIN
#endif // S2R_HPP
// vim: ts=4 sw=4 et cindent cino=^=l0,\:.5s,=-.5s,N-s,g.5s,h.5s,b1 cinkeys=0{,0},0),\:,0#,!^F,o,O,e,0=break syntax=cpp.doxygen
