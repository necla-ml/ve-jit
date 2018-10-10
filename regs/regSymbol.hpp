#ifndef REGSYMBOL_HPP
#define REGSYMBOL_HPP
#include "spillable-base.hpp"
#include "regDefs.hpp"
// While regDefs.hpp is now a processor-specific include file,
// it does define a few fairly generic functions.
class RegSymbol; // SpillableBase + name, rtype and regId()

/** Flesh out a \c SpillableBase symbol with \c name,
 * register type \c rtype [,and user-settable \c regId()].
 *
 * see symScopeUid.hpp  symbol<-->scope relation maintained separately
 */
class RegSymbol : protected SpillableBase
{
  public:
    typedef SpillableBase Base;
    friend std::ostream& operator<<(std::ostream& os, RegSymbol const& x);

    /** <em>Declare</em> symbol for a user register. \c regid is from some
     * register allocator, while \c symId is a [unique] forward counter value.
     *
     * With a register model (\ref regDefs.hpp) bytes, align and name might
     * have useful defaults based on register id (\c regid) (derived class!).
     *
     * With symbol scopes, \c regid might come from \c SymScopeUid::newsym()
     */
    explicit RegSymbol(unsigned const symId, uint64_t const tick, char const* const name,
            ve::Reg_t const rtype )
        : SpillableBase(symId, ve::bytes(rtype), ve::align(rtype)),
        name_(name), regid_(ve::invalidReg()), rtype_(ve::FREE), t_decl(tick), t_sym(0U)
    {
        //std::cout<<" +RS-a:"<<*this<<std::endl;
        assert(name_!=nullptr);
    }
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
    explicit RegSymbol(unsigned const symId, uint64_t const tick, char const* const name,
            ve::RegId const regid)
        : SpillableBase(symId, ve::bytes(regid), ve::align(regid)),
        name_(name),
        regid_(ve::valid(regid)? regid: ve::invalidReg()),
        rtype_(REG_T(ve::defReg_t(regid)|ve::USED)),
        t_decl(tick),
        t_sym(tick)
    {
        if(ve::valid(regid_)) setREG(true);
        std::cout<<" +RS-b "<<*this<<std::endl;
        assert(name_!=nullptr);
        assert(ve::valid(regid_));
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
    // ? associate a new symbol with the memory version of given symbol (whether or not stale)

    //
    // begin by exposing base class functions "as is" [then customize]
    //
    using Base::uid;
    Where getWhere() const              {return Base::getWhere();}
    unsigned symId() const              {return Base::symId();}
    bool getActive() const              {return Base::getActive();}
    int getBytes() const                {return Base::getBytes();}
    int getAlign() const                {return Base::getAlign();}
    bool getREG() const                 {return Base::getREG();}
    bool getMEM() const                 {return Base::getMEM();}
    RegSymbol& setActive(bool set=true  ) {Base::setActive(set); return *this;}
    RegSymbol& setREG(bool set=true)    {Base::setREG(set); return *this;}
    RegSymbol& setMEM(bool set=true)    {Base::setMEM(set); return *this;}
    unsigned getStale() const           {return Base::getStale();}
    RegSymbol& unStale()                {Base::unStale(); return *this;}
    RegSymbol& incStale()               {Base::incStale(); return *this;}
    //
    // overridden/modified/extended Base functions
    //
    /** register unassign (possibly tmp, or before out-of-scope). */
    RegSymbol& unsetReg() {Base::setREG(false).Base::setMEM(false); regid_=ve::invalidReg(); return *this;}
    /** register copy, moving symbol to given \c rid. */
    RegSymbol& setReg(ve::RegId const rid, uint64_t const tick);
    /** annotate register read event */
    RegSymbol& tRead(uint64_t const tick);
    /** annotate register write event */
    RegSymbol& tWrite(uint64_t const tick);
    //
    // new functionality
    //
    char const* name() const            {return name_;}
    ve::RegId regId() const             {return regid_;}
    ve::Reg_t rType() const             {return rtype_;}
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
    ve::RegId regid_;           ///< register assignment (or IDlast)
    ve::Reg_t rtype_;           ///< <em>register type</em> (chip-specific, \reg regDefs.hpp for Aurora)
    uint64_t const t_decl;      ///< declaration time tick
    uint64_t t_sym;             ///< last-used tick
};//RegSymbol

inline RegSymbol& RegSymbol::setReg(ve::RegId const rid, uint64_t const tick)
{
    if( ve::valid(rid) ){
        assert( tick >= t_sym );
        t_sym = tick;
        //if( rid == regid_ ) this is OK, no warning.
        regid_ = rid;
        setREG(true);
    }else{
        unsetReg();
    }
    return *this;
}
inline RegSymbol& RegSymbol::tRead(uint64_t const tick){
    assert(getActive() && ve::valid(regid_) && Base::getREG() && "symbol not in register?");
    assert(tick >= t_sym);
    t_sym=tick;
    return *this;
}
inline RegSymbol& RegSymbol::tWrite(uint64_t const tick){
    assert(getActive() && ve::valid(regid_) && Base::getREG() && "symbol not in register?");
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
        <<x.uid
        ;
    if(x.getREG())
        os<<"R"; else os<<"~R";
    if(ve::valid(x.regId()))
        os<<ve::asmname(x.regId());
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

// vim: ts=4 sw=4 et cindent cino=^=l0,\:.5s,=-.5s,N-s,g.5s,h.5s,b1 cinkeys=0{,0},0),\:,0#,!^F,o,O,e,0=break
#endif // REGSYMBOL_HPP
