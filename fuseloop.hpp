#include <sstream>
#include <vector>
#include <type_traits>
#include <cstdint>
#include <cassert>

namespace loop {

typedef int64_t Lpi; // Loop-index type
// Note other optimization might *pack* 2-loop indices differently!
// (e.g. u32 a[0]b[0] a[1]b[1] in a packed register)
// (e.g. single register with a[] concat b[] (for short double-loops)
// Here we just store a[], b[] indices in two separate registers
typedef uint64_t Vlpi; // vector-loop-index type
typedef std::vector<Vlpi> VVlpi;

typedef std::make_unsigned<Lpi>::type Ulpi;
typedef std::make_unsigned<Vlpi>::type Uvlpi;

enum Unroll {
    UNR_UNSET=0,        ///< uninitialized
    UNR_NLOOP1,         ///< precalc, never unroll
    UNR_VLMODJJ,        ///< no precalc, any small unroll
    // jj%vl == 0
    UNR_JJMODVL_NORESET, ///< no precalc, any small unroll
    UNR_JJMODVL_RESET,  ///< XXX check for bcyc_regs or nloop XXX
    // isPositivePow2(jj)
    UNR_JJPOW2_NLOOP,   ///< no precalc, unroll by nloop (full unrol)
    UNR_JJPOW2_CYC,     ///< precalc b[] (and a[] for a+=const?) [partial?] unroll by bcyc_regs
    UNR_JJPOW2_BIG,     ///< no precalc, any small unroll
    UNR_NLOOP,          ///< precalc, full unroll is small
    UNR_CYC,            ///< precalc, [partial?] unroll by cyclic for b[] (and a?)
    // generic div-mod
    UNR_DIVMOD          ///< no precalc, any small unroll
};

/** Describe a vectorized loop fusion/unrolling strategy.
 *
 * - precalc may save ops when done outside some external enclosing loops
 *   - no precalc: induction always via formula
 *   - precalc and [partial?] unroll : induction by register sequence (maybe looped)
 * - unroll can be:
 *   - never                            // nloop = 1
 *   - full                             // full precalc unroll (low nloop)
 *   - suggested value (nloop,bcyc_regs,...)
 *   - any small unroll (e.g. 8)        // using some [fast?] induction formula
 */
struct UnrollSuggest {
    /** where did we come from? \p vl ~ vector length. \p ii,jj ~ nested loop limits. */
    int vl, ii, jj, b_period_max;

    // Suggested strategy.
    /** what class of unrolling is suggested? */
    enum Unroll suggested;
    /** Vector Length Lower (may give a more efficient induction step).
     * \c unroll_suggest looks in range 100%--90% of original \c vl for
     * an alternative that might be better. */
    int vll;
    /** \c nloop is a multiple of \c unroll for partial unroll cases,
     * equal for full unroll,
     * but is untied for <em>any small unroll</em> case. */
    int nloop;
    /** explicit unrolling factor may be given, possibly < nloop. */
    int unroll;
    UnrollSuggest()
        : vl(0),ii(0),jj(0),b_period_max(0),suggested(UNR_UNSET),
        vll(0),nloop(0),unroll(0)
    {}
    UnrollSuggest(int const vl, int const ii, int const jj, int const b_period_max=8)
        : vl(vl), ii(ii), jj(jj), b_period_max(b_period_max),suggested(UNR_UNSET),
        vll(0), nloop((int)( ((int64_t)ii*jj+vl -1) / vl )), unroll(0)
    {}
};

/** make loop unrolling suggestions.
 *
 * Outputs a descriptive suggestion to \c cout.
 *
 * \return enum value describing the type of unrolling that could be done.
 *
 * \todo a separate function returning a struct describing precalc,
 * complete with precalculated data vectors
 * and maybe some precalc constants (ex. magic values, special shifts).
 */
UnrollSuggest unroll_suggest( int const vl, int const ii, int const jj, int const b_period_max,
        int const verbose = 1);

/** Scan vl (or vl-1) to \c vl_min for an efficient loop induction.
 * - Usage:
 *   - first call \c unroll_suggest(vl,ii,jj,b_period_max)
 *   - optionally call this function with above result for
 *     [potentially] an alternate reduced-vl strategy.
 * - If \c u.suggest == UNR_UNSET (default-constructed), then
 *   scan from \c vl downwards; otherwise scan from vl-1 downwards.
 * - If \c vl_min is not in range [1,\c u.vl], then
 *   set vl_min at max(0.90*vl,1);
 * \return unroll suggestion at an efficient, possibly reduced, vector length.
 * \post if a nicer alt is found, \c u.vll records this reduce \c u.vl
 */
UnrollSuggest unroll_suggest( UnrollSuggest & u, int vl_min=0 );

char const* name( enum Unroll const unr );
char const* desc( enum Unroll const unr );
std::ostream& operator<<(std::ostream& os, enum Unroll unr);
std::string str(UnrollSuggest const& u, std::string const& pfx="");
std::ostream& operator<<(std::ostream& os, UnrollSuggest const& u);

/** Reference values for correct index outputs */
struct Vab{
    Vab( VVlpi const& asrc, VVlpi const& bsrc, int vl )
        : a(asrc), b(bsrc), vl(vl), hash(0) {
        assert( asrc.size() >= (size_t)vl );
        assert( bsrc.size() >= (size_t)vl );
        }
    VVlpi a;
    VVlpi b;
    int vl;    // 0 < Vabs.back().vl < vlen
    uint64_t hash;
#if 0
    // THIS APPROACHED IS FLAWED.  I really want hash to be independent of any
    // particular choice for the simd lengths, vl.
    /** fold \c a[vl] and \c b[vl] with a prev [or seed] \c hash. \return new hash value. */
    static uint64_t rehash(VVlpi const& a, VVlpi const& b, int const vl, uint64_t hash);
    /** fold a[] and b[] into this->hash. \return updated this->hash */
    uint64_t rehash() { return hash = rehash(a, b, vl, hash); }
#endif
};
struct UnrollData : public UnrollSuggest {
    /** precalculated data vectors.
     * - if !pre[].empty()
     *   - pre.size() == 1             (init-phase uses vpre[0], any unroll/nloop)
     *   - or unroll == pre.size() > 1 (induction-phase uses vpre data)
     * - otherwise we can give a formula-based unroll>0 using no recalc
     *   - <nloop: partial unroll (still have an enclosing loop or a finishing step)
     *   - ==nloop: full unroll
     * - any unroll may or may not use pre.a[] or vpre.b[] values
     *   - pre data can be used for init- or induction-phase.
     *
     * \todo see whether init-precalc and ind-precalc can actually share pre[0]
     */
    std::vector<Vab> pre;
};

/** Generate reference vectors of vectorized 2-loop indices */
std::vector<Vab> ref_vloop2(Lpi const vlen, Lpi const ii, Lpi const jj,
        int const verbose=1);

}//loop::

// vim: ts=4 sw=4 et cindent cino=^=l0,\:.5s,=-.5s,N-s,g.5s,b1 cinkeys=0{,0},0),\:,0#,!^F,o,O,e,0=break
