#ifndef CJIT_H
#define CJIT_H
/* Copyright (c) 2019 by NEC Corporation
 * This file is part of ve-jit */
#warning "deprecated : see jitpage.h jitpage.c"

#ifdef __cplusplus
extern "C" {
//#warning "Good, jitpage.h is extern 'C' for C++ compile"
#endif
    /// \group misc C utilities
    /// accumulate them here, for now
    //@{
    /** try \c dlopen(relpath,opt), and if that fails, try \c realpath(relpath).
     * \return NULL on error */
    void * dlopen_rel(char const* const relpath, int opt);

    /** debug dump of a .so library, from dlopen[_rel] */
    void dl_dump(void const* const dlopen_handle);
    //@}
#ifdef __cplusplus
} // extern "C"
#endif

// vim: ts=4 sw=4 et cindent cino=^=l0,\:.5s,=-.5s,N-s,g.5s,b1 cinkeys=0{,0},0),\:,0#,!^F,o,O,e,0=break
#endif // CJIT_H
