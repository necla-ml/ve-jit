#ifndef REGSYMBOL2_HPP
#define REGSYMBOL2_HPP

#define OLD 0

#include "spill-impl2.hpp"
#include "symScopeUid.hpp"
#include "scopedSpillableBase.hpp"
class RegSymbol; // SpillableBase + name, rtype and regId()


/** New version of RegSymbol.
 * Instead of fleshing out a \c ScopedSpillableBase symbol, we flesh out the
 * higher-level \c scope::SymbStates<ScopedSpillableBase> object.
 *
 * - with \c name(),
 * - with a \c RegisterBase::Cls enum { [none?] scalar vector mask }, \c rType()
 * - time of decl, last use (\c t_decl, \c t_sym)
 * - and a \c regId().
 *
 * - how do I get a uid?
 *
 */
class RegSymbol :
    protected scope::ParSymbol<ScopedSpillableBase>
{
    friend class DemoSymbStates;
  private:
    /** TODO -- should be thread-safe and avoid static (possible dll issues).
     * TODO -- probably should be within reg-base.hpp (common to both symbol and register ops?) */
    static uint64_t nextTick(){
        static uint64_t t=0U; // library/multi-threading issues! should used shared_ptr version of a counter.
        return ++t;
    }
    static unsigned nextSym(){
        static uint64_t t=0U; // library/multi-threading issues! should used shared_ptr version of a counter.
        return ++t;
    }
  public:
    typedef RegisterBase Rb;
    typedef Rb::Cls      Cls; // Cls::scalar Cls::vector Cls::mask Cls::none

    //typedef ScopedSpillableBase Base;
    typedef scope::ParSymbol<ScopedSpillableBase> Base;
    friend std::ostream& operator<<(std::ostream& os, RegSymbol const& x);

    /** <em>Declare</em> symbol for a user register. \c regid is from some
     * register allocator, while \c symId is a [unique] forward counter value.
     *
     * With a register model (\ref regDefs.hpp) bytes, align and name might
     * have useful defaults based on register id (\c regid) (derived class!).
     *
     * With symbol scopes, \c regid might come from \c SymScopeUid::newsym()
     */
    explicit RegSymbol(char const* const name,
            Rb::Cls const rtype = Rb::Cls::none
            //, Rb::Sub const rsub = Rb::Sub::def
            )
        : Base(nextSym(), 1/*scope*/, defBytes(rtype), defAlign(rtype)),
        name_(name),
        regid_(invalidReg()),
        rtype_(rtype),
        rsub_(Rb::Sub::def),
        t_decl(RegSymbol::nextTick()),
        t_sym(0U)
    {
        //std::cout<<" +RS-a:"<<*this<<std::endl;
        assert(name_!=nullptr);
    }
    static int const a = 13;
    /** <em>Declare and assign</em> symbol to user register.
     * \c regid is from some register allocator [maybe a \c Spill object],
     * while \c symId is a [unique] forward counter value
     * [from a class mapping unsigned-->RegSymbol].
     *
     * Given a register model (\ref regDefs.hpp) bytes, align and name might
     * have useful defaults based on register id (\c regid) (derived class!).
     * We also supply a simplest-possible default Reg_t value based on the
     * register id.
     *
     * For example, \c regid=67 might get mapped to a Reg_t=USED|VECTOR,
     * with none of the optional PRESERVE or VLEN settings set up.
     */
    explicit RegSymbol( char const* const name, RegId const regid)
        : Base(nextSym(), 1/*scope*/, defBytes(regid), defAlign(regid))
          ,name_(name)
          ,regid_(valid(regid)? regid: invalidReg())
          ,rtype_(cls(regid))
          ,t_decl(RegSymbol::nextTick())
          ,t_sym(0U)
          {
              if(valid(regid_)){setREG(true); t_sym=t_decl;}
              std::cout<<" +RS-b "<<*this<<std::endl;
              assert(name_!=nullptr);
              assert(valid(regid_));
          }
#if 0
    /** register copying constructor. */
    explicit RegSymbol(RegSymbol const& src, unsigned const symId, char const* const newName,
            ve::RegId const regid, uint64_t const tick)
        : SpillableBase(src), name_(newName), regid_(regid), rtype_(src.rtype_), t_decl(tick), t_use(tick)
    {
        assert(name_!=nullptr);
        // in some cases regType(regid) != this regtype, so  you may need to fix by hand.
        Base::setREG(true).Base::setMEM(false);
    }
#endif

#if OLD
    /** <em>Declare+assign</em> "system" register symbols.  Registers with immutable
     * mapping or state flags, like reserved registers, preserved registers.
     * These always have default bytes/align.
     *
     * TODO: function-entry arguments in register and/or memory.
     * TODO: may change
     */
    explicit RegSymbol(unsigned const symId, char const* const name, ve::RegId const regid )
        : SpillableBase(symId, ve::bytes(ve::defReg_t(regid)), ve::align(ve::defReg_t(regid))),
        name_(name), regid_(regid), rtype_(ve::defReg_t(regid)), t_decl(0U), t_sym(0U)
    {
        //std::cout<<" +RS-c:"<<*this<<std::endl;
        assert(name_!=nullptr);
    }
#endif
#if OLD
    /** system regs, customized register type flags.
     * - Not error-checked for consistency fo regid and rtype
     * - Ex. add vector length, or double-length registers,
     *       or PRESERVE/RESERVED/USED state?  */
    explicit RegSymbol(unsigned const symId, char const* const name, ve::RegId const regid,
            ve::Reg_t rtype )
        : SpillableBase(symId, ve::bytes(rtype), ve::align(rtype)),
        name_(name), regid_(regid), rtype_(rtype), t_decl(0U), t_sym(0U)
        {
            //std::cout<<" +RS-d:"<<*this<<std::endl;
            assert(name_!=nullptr);
        }
#endif
    // ? associate a new symbol with the memory version of given symbol (whether or not stale)

    //
    // begin by exposing base class functions "as is" [then customize]
    //
    Where getWhere() const              {return Base::getWhere();}
    unsigned symId() const              {return Base::symId();}
    bool getActive() const              {return Base::getActive();}
    int getBytes() const                {return Base::getBytes();}
    int getAlign() const                {return Base::getAlign();}
    bool getREG() const                 {return Base::getREG();}
    bool getMEM() const                 {return Base::getMEM();}
    RegSymbol& setActive(bool set=true) {Base::setActive(set); return *this;}
    RegSymbol& setREG(bool set=true)    {Base::setREG(set); return *this;}
    RegSymbol& setMEM(bool set=true)    {Base::setMEM(set); return *this;}
    unsigned getStale() const           {return Base::getStale();}
    RegSymbol& unStale()                {Base::unStale(); return *this;}
    RegSymbol& incStale()               {Base::incStale(); return *this;}
    
    // overridden/modified/extended Base functions
    //
    /** register unassign (possibly tmp, or before out-of-scope). */
#if OLD
    RegSymbol& unsetReg() {Base::setREG(false).Base::setMEM(false); regid_=ve::invalidReg(); return *this;}
#else
    RegSymbol& unsetReg() {setREG(false).setMEM(false); regid_=invalidReg(); return *this;}
#endif
    /** register copy, moving symbol to given \c rid. */
    RegSymbol& setReg(RegId const rid, uint64_t const tick); ///< deprecated
    RegSymbol& setReg(RegId const rid);
    /** annotate register read event */
    RegSymbol& tRead(uint64_t const tick);
    /** annotate register write event */
    RegSymbol& tWrite(uint64_t const tick);
    //
    // new functionality
    //
    char const* name() const            {return name_;}
    RegId regId() const             {return regid_;}
#if OLD
    ve::Reg_t rType() const             {return rtype_;}
#else
    Rb::Cls rType() const             {return rtype_;}
    Rb::Sub rSub() const             {return rsub_;}
#endif
    uint64_t tDecl() const              {return t_decl;}
    uint64_t tSym() const               {return t_sym;} // useful?
    //
    RegSymbol& rename(char const* newName){name_=newName; return *this;}
    //
    // ? set vector length. \pre regType(r)==Reg_t::VECTOR.

  private:
    char const* name_;          ///< declared symbol name
    //unsigned scope;           ///< scope uid; 1=function, 2,3,...=sub-blocks (never re-use these)
    // ... see symScopeUid.hpp  symbol<-->scope relation maintained separately
    RegId regid_;               ///< register assignment (or Rb::invalidReg())
    Rb::Cls rtype_;             ///< generic register type
    Rb::Sub rsub_;              ///< register subclass (only generic value is Rb::Sub::def
    uint64_t const t_decl;      ///< declaration time tick
    uint64_t t_sym;             ///< last-used tick
};//RegSymbol

inline RegSymbol& RegSymbol::setReg(RegId const rid, uint64_t const tick)
{
    if( valid(rid) ){
        assert( tick >= t_sym );
        //t_sym = tick;
        t_sym = nextTick();
        //if( rid == regid_ ) this is OK, no warning.
        regid_ = rid;
        setREG(true);
    }else{
        unsetReg();
    }
    return *this;
}
inline RegSymbol& RegSymbol::setReg(RegId const rid)
{
    if( valid(rid) ){
        auto const tick = nextTick();
        assert( tick >= t_sym );
        //if( rid == regid_ ) this is OK, no warning.
        regid_ = rid;
        setREG(true);
    }else{
        unsetReg();
    }
    return *this;
}
inline RegSymbol& RegSymbol::tRead(uint64_t const tick){
    assert(getActive() && valid(regid_) && Base::getREG() && "symbol not in register?");
    assert(tick >= t_sym);
    t_sym=tick;
    return *this;
}
inline RegSymbol& RegSymbol::tWrite(uint64_t const tick){
    assert(getActive() && valid(regid_) && Base::getREG() && "symbol not in register?");
    assert(tick >= t_sym);
    t_sym=tick;
    Base::setREG(true); // side effect : if spilled to memory, memory value becomes stale.
    return *this;
}
std::ostream& operator<<(std::ostream& os, RegSymbol const& x){
    //std::cout<<" name@"<<(void*)x.name()<<" "; std::cout.flush();
    os<<"RegSymbol{"
        <<(x.name()!=nullptr? x.name(): "?")<<":"
        <<(x.getActive()?"+":"-")
        <<x.symId()
        ;
    if(x.getREG())
        os<<"R"; else os<<"~R";
    if(valid(x.regId()))
        os<<asmname(x.regId());
    else
        os<<"l"<<x.getBytes()<<"a"<<x.getAlign(); //<<"~R";
    if(x.getMEM()){
        if(x.getStale()) os<<"~M"<<x.getStale();
        else             os<<"M";
    }
    os<<x.rType();
    os<<"d"<<x.tDecl()<<"u"<<x.tSym();
    return os<<"}";
}

// vim: ts=4 sw=4 et cindent cino=^=l0,\:.5s,=-.5s,N-s,g.5s,h.5s,b1 cinkeys=0{,0},0),\:,0#,!^F,o,O,e,0=break syntax=cpp.doxygen
#endif // REGSYMBOL2_HPP
