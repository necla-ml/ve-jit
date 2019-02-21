#ifndef JITPAGE_H
#define JITPAGE_H
/** \file
 * C helpers for jit pages and assembling .S to .bin .
 *
 * While \ref asmfmt.hpp has some C++ stuff,
 * someimes you may want to create JIT code via C functions.
 *
 * Some functions are specific to VE assembler code (nas utils, .bin files).
 * Others might have utils for VE+intrinsics 'C' code.
 */

#ifdef JITVVE_UTIL_H
#error "Please do not mix up old jitve_util.h API with newer jitpage.h API"
#endif

#include <stdint.h> // uint64_t
#include <stddef.h> // size_t

#ifdef __cplusplus
extern "C" {
//#warning "Good, jitpage.h is extern 'C' for C++ compile"
#endif
    struct JitPage_s {
        void* mem;  ///< bin2jitpage set this non-NULL with read/write/exec permission
        size_t len; ///< span of memory addresses
        /* current posn of unused addresses (initially zero).  The user will typically
         * maintain this if "real" jit assembly is done. */
        //size_t pos;
        /** \c v verbosity: never(-1), print errors(0), warnings(1), debug(>=2).
         * Remember this setting, from \c bin2jitpage, for convenience. */
        int verbosity;
    };
    typedef struct JitPage_s JitPage;

#if 0 // not a good idea to have statics in libraries
    /** Make \ref jitpage.h functions never(-1), print errors(0), warnings(1), debug(>=2).
     * Default is debug(2). */
    void jitpage_verbose(int const verbose);
#endif

    /** \c cpp_asm_code --> basename.S --> basename.bin  using `make -f bin.mk`
     * Given \c cpp_asm_code, create files basename.S and compiled binary blob
     * basename.bin.
     * \return nonzero if error.
     */
    int asm2bin(char const* basename, char const* const cpp_asm_code, int const verbose);

    /** Read basename.bin file into an executable code page.
     * \note basename should NOT have the .bin suffix.
     * \return code page (or NULL) as in \c jitpage->addr.
     * Use jitpage_free(JitPage) to unmap the executable page.
     * The page comes with PROT_WRITE|PROT_READ|PROT_EXEC
     * Sets \c jitpage->verbosity, which persists for later jitpage funcs.
     * \c verbosity : never(-1), print errors(0), warnings(1), debug(>=2).
     */
    char* bin2jitpage(char const* basename, JitPage *jitpage, int const verbosity);

    /** attempt to PROT_READ|PROT_EXEC the page.
     * (executable self-modifying code would require I-Cache flushing on Aurora).
     */
    void jitpage_readexec(JitPage *jitpage);

    /** munmap the exectuable code page.
     * - \c verbosity as per \c jitpage->verbosity:
     *   -  never(-1), print errors(0), warnings(1), debug(>=2).
     * \return status nonzero on error
     *   - (NULL input values are not an error, dup free ok)
     */
    int jitpage_free(JitPage* jitpage);

    /** reproduce hexdump -C "canonical hex+ASCII" output format */
    void hexdump(char const* page, size_t sz);

    /** try recursive mkdir, the return access(path,W_OK).
     * \return \b true if write access \b denied. */
    int createDirectoryAnyDepth(char const *path);

    /** create and M-string constant.
     * nas can use some constants in format (N)M.
     * Here N=repeat count 0..64
     * and M is 0|1.
     * The constant is read as "N high-position M-bits"
     * ... followed by all-(~M) bits.
     *
     * \ret 0/1 for fail/success
     * If we return 1, then mconst is set to the string.
     *
     * string buffer \c mconst bounds \b unchecked !
     * \pre \c mconst must be at least 6 chars long (5+terminal nul)
     */
    int strMconst(char *mconst, uint64_t const parm);

#define JIT_DLFUNCS 1
#if JIT_DLFUNCS
    /** safety-wrapper around dlopen, converting relpath to abspath
     * if necessary.
     * \p opt=RTLD_NOW or RTLD_LAZY
     */
    void * dlopen_rel(char const* const relpath, int opt);

    /** walk symbol table of dlopen handle \p handle, dumping to \c stdout. */
    void dl_dump(void * const handle);

#endif

#ifdef __cplusplus
} // extern "C"
#endif

// vim: ts=4 sw=4 et cindent cino=^=l0,\:.5s,=-.5s,N-s,g.5s,b1 cinkeys=0{,0},0),\:,0#,!^F,o,O,e,0=break
#endif // JITPAGE_H
