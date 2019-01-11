#ifndef VELOGIC_HPP
#define VELOGIC_HPP
/** \file logic tests for VE code gen (can run on any chip) */
#include "jitpage.h"
#include <cstdint>
#include <string>
#include <iosfwd>

extern "C" {

    /** Error return for integer tests. */
    struct VeliErr {
        uint64_t i;         ///< input value
        uint64_t error;     ///< zero if no error
        uint64_t other;     ///< other (code path? secondary output?)
    };


    /** \group VE code for integer tests, ABI=raw wrappers */
    /** Wrappers do <em>for(start..start+count) execute page</em>.
     * - require ncc to compile and run only on VE.
     * - expect a JIT test to run only a single case.
     * - so there is no \c count argument for the wrapper call
     * - Why? JIT calls use a raw ABI and should be claimed to clobber
     *   %s0--7, %s40--%s49.  There is little call overhead.
     *
     * Types of wrpiFoo:
     *
     * - SI input + internal branching
     *   - internal branches only based on SI, covering all cases as in veliFoo
     *     standalone testing.
     *   - in JIT code, only code for a single case is output
     *
     * - SI + SA(algorithm) + more branching
     *   - ex. loadreg might prefer to specify an INSTRUCTION TYPE preference
     *     load vs logical vs shift vs arithmetic vs float
     *
     * But this may get tedious to write the internal branching logic in asm.
     * So a better approach is:
     *
     * - SI input [+ SA alg] + NO internal branching
     *   - Alt. for exhaustive testing.
     *   - <B>Every input/alg subcase --> 1 JitPage,</B>
     *   - <B>all the branching logic in the 'C' wrapper</B>
     *   - i.e. the wrapper gets complicated, but the asm code stays simple
     *   - input to all input/alg subcase is \em identical,
     *   - so the 'C' branching logic only sets the proper JitPage*, and
     *     invokes a common inline asm to cause the test kernel to execute.
     *
     * This last approach may fail for algs involving loops, because of too
     * many cases for alg vs special loop sizes vs unrolling options
     * vs vector length strategy vs ... ...
    */
    //@{
#if defined(__ve) || defined(DOXYGEN)
    VeliErr     wrpiLoadreg(JitPage* page, uint64_t start, uint64_t count=1U);
#endif
    //@}
} // extern "C"

#if defined(__cplusplus)
    std::ostream& operator<<(std::ostream&, VeliErr const& e);
#endif

/** \group generic code for integer tests
 * test form :
 * ```VeliErr veliFoo(uint64_t start, uint64_t count)```
 * VE assembler ABI test program:
 * ```std::string prgiFoo(uint64_t start, uint64_t count)```
 *
 * To run the prg string runs on VE, compile to blob and call via an inline asm
 * wrapper with input args in %s0, %s1 and returning VeliError in %s2, %s3, %s4:
 * ```int wrpiFoo(uint64_t start, uint64_t count)```.
 * wrpiFoo can only be compiled for VE, since it contains ncc inline assembly.
 */
//@{

/** test loadreg logic from [start..count]. */
VeliErr     veliLoadreg(uint64_t start, uint64_t count=1U);

/** suggested VE code string, ready to compile into an ABI=jit \ref JitPage */
std::string prgiLoadreg(uint64_t start);

//@}
#endif // VELOGIC_HPP

