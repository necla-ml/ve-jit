#ifndef SPILL_HPP
#define SPILL_HPP
/** \file
 * Spill area tracking: template functions.
 *
 * This is used by spill-impl.hpp, which is tested by testSpill2,
 * testSpill3 and testRegSym
 *
 * \todo Spill.hpp should use reg-base.hpp instead of the grab-bag regDefs.hpp
 */

#include "regDefs.hpp"
// actually we probably only need SymId and RegId, and we can get these
// by asking that SYMBSTATES provide these typedefs.
// ... or just pull in reg-base_
#include <forward_list>

namespace ve {
template<class SYMBSTATES> class Spill;

void spill_msg_destroy(int bottom);

/** Track spill area usage and requirements.
 *
 * After fn prologue, the spill area is above the parameter area,
 * and below the frame pointer.  You can allocate "local variables"
 * in this memory area.
 *
 * Before we emit any code, we will know the total amount of stack
 * space for spill/local vars (and use this in function prologue
 * to add a new stack frame)
 *
 * \tparm SYMBSTATES \c SYMBSTATES::psym(symId) is used to convert
 *        symId --> SYMBSTATES::Psym [const] &
 * - SYMBSTATES::Psym should be a \ref SpillableBase
 * - The \c Psym class describes:
 *   - uid, align, len, \em location[in-scope?/register?/memory?],
 *     and memory-staleness
 *
 * It is expected that scope::SymbState be the usual SYMBSTATE.
 *
 * Spill maps symbol ids to memory spill offsets.  It doesn't care about
 * register assignment, even though \c p(symId) may contain such info.
 */
template<class SYMBSTATES>
class Spill {
  public:
    void dump() const;
    void contiguity_check() const;
    /** validate our data structures, or throw. */
    void validate();
    /** get max negative offset ~ size of "locals" memory. */
    int getBottom() const {return this->bottom;}
    typedef typename SYMBSTATES::Psym Sym;
    /** We maintain a downward growing list of regions. With time, the symbols
     * in smap change state: they declared, assigned to registers, acquire values,
     * modify register values, ... and sometimes they spill to memory.  Our job
     * is to manage how registers get mapped to memory offsets, and track the
     * freshness/staleness of such memory copies.  When registers get allocated,
     * we might need to search for free memory slots to spill an old register
     * with known byte-length and alignment requirements.
     */
    typedef struct {
        unsigned symId; ///< 0 if area is free
        int offset;     ///< -ve value, wrt %fp
        int len;        ///< byte length of memory
    } Region;
    typedef typename std::forward_list<Region> Rgns;
    Rgns const& regions() const {return this->use;}

    /** \p symbStates need not be fully constructed. We use symbStates to
     * dereference unsigned symId --> SYMBSTATES::Psym symbol objects. */
    Spill(SYMBSTATES *symbStates)
        : symbStates(symbStates), bottom(0), use()
    {}
    ~Spill() { if(1) spill_msg_destroy(bottom); }
    // Sym object must provide:
    //          int align;      > 0, power-of-two
    //          int len;        > 0
    //Spill(Regs* regs) : regs(regs), bottom(0), use() {}
    /** Spill a sym that is currently in a register.
     * Default align is always 8, but might be overridden if
     * more detailed info (ffilib, float/double/packed) info is
     * available.
     */
    void spill(unsigned const symId, int align=8);
    /** remove a Sym memory area */
    void erase(unsigned symId);
    // unsigned oldest_sym() ? NO: we don't access symbol 'time' concept.
    /** iterate over symbols, removing Spill copies of \b all in-register syms.
     * A. Whenever sym->locn is in-register (no need for a spill copy, esp. if spill copy stale)
     * B. Whenever sym->staleness is > 0, the in-register version is more
     *    recent than the last spilled version.
     * Here we change all case (B) to case (A), erasing all stale spill copies.
     */
    void gc();
    /** Read a sym from spill into given [free] register.
     * (TENTATIVE)
     * (may need to adjust to match RegId class?) */
    void read(unsigned const symId, RegId const id);
  private:
    Sym const& p(unsigned const symId) const;
    Sym      & p(unsigned const symId)      ;
    SYMBSTATES *symbStates;
    //Regs *regs;
    int bottom;                 ///< track max size of 'local variable' area
    // bottom should aligndown during prologue generation to align 16
    /// Note that before code emit, we can re-order that offset/len assignment
    /// at will during 'spill' so that we don't fragment things too much.
    Rgns use;       ///< locals are maintained in order of \c offset

    /** create a new storage area and return iter to new \c Rgns entry. */
    typename Rgns::const_iterator newspill(unsigned const symId);
    /** store reg in spill region (code gen!) */
    // XXX do not need 1st arg.  ('at' has the sym id already, I think)
    void emit_spill( unsigned const symId, typename Rgns::const_iterator& at );

    /** return a 'prev' iterator after which a right-sized
     * hole could be filled. \c size >0, \c align is 2^n.
     * \c use.before_begin() or last item are allowed return values.
     */
    typename Rgns::const_iterator find_hole( int const len, int const align ) const;
};//class Spill

}//ve::
#endif // SPILL_HPP
// vim: ts=4 sw=4 et cindent cino=^=l0,\:.5s,=-.5s,N-s,g.5s,b1 cinkeys=0{,0},0),\:,0#,!^F,o,O,e,0=break
