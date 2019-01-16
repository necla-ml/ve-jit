#ifndef JITPAGE_H
#define JITPAGE_H
/** \file
 * C helpers for jit pages and assembling .S to .bin .
 *
 * While \ref asmfmt.hpp has some C++ stuff,
 * someimes you may want to create JIT code via C functions. */

#ifdef JITVVE_UTIL_H
#error "Please do not mix up old jitve_util.h API with newer jitpage.h API"
#endif

#include <stddef.h> // size_t
#ifdef __cplusplus__
extern "C" {
#endif
    struct JitPage {
        void* mem;  ///< bin2jitpage set this non-NULL with read/write/exec permission
        size_t len; ///< span of memory addresses
        /* current posn of unused addresses (initially zero).  The user will typically
         * maintain this if "real" jit assembly is done. */
        //size_t pos;
        /** \c v verbosity: never(-1), print errors(0), warnings(1), debug(>=2).
         * Remember this setting, from \c bin2jitpage, for convenience. */
        int verbosity;
    };

#if 0 // not a good idea to have statics in libraries
    /** Make \ref jitpage.h functions never(-1), print errors(0), warnings(1), debug(>=2).
     * Default is debug(2). */
    void jitpage_verbose(int const verbose);
#endif

    /** Read basename.bin file into an executable code page.
     * \note basename should NOT have the .bin suffix.
     * \return code page (or NULL) as in \c jitpage->addr.
     * Use jitpage_free(JitPage) to unmap the executable page.
     * The page comes with PROT_WRITE|PROT_READ|PROT_EXEC
     * Sets \c jitpage->verbosity, which persists for later jitpage funcs.
     * \c verbosity : never(-1), print errors(0), warnings(1), debug(>=2).
     */
    char* bin2jitpage(char const* basename, struct JitPage *jitpage, int const verbosity);

    /** attempt to PROT_READ|PROT_EXEC the page.
     * (executable self-modifying code would require I-Cache flushing on Aurora).
     */
    void jitpage_readexec(struct JitPage *jitpage);

    /** munmap the exectuable code page.
     * - \c verbosity as per \c jitpage->verbosity:
     *   -  never(-1), print errors(0), warnings(1), debug(>=2).
     * - \ret status nonzero on error
     *   - (NULL input values are not an error, dup free ok)
     */
    int jitpage_free(struct JitPage* jitpage);
#ifdef __cplusplus__
} // extern "C"
#endif

// vim: ts=4 sw=4 et cindent cino=^=l0,\:.5s,=-.5s,N-s,g.5s,b1 cinkeys=0{,0},0),\:,0#,!^F,o,O,e,0=break
#endif // JITPAGE_H
