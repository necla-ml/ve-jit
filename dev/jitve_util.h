#ifndef JITVE_UTIL_H
#define JITVE_UTIL_H
/* Copyright (c) 2019 by NEC Corporation
 * This file is part of ve-jit */
/** \file
 * These are 'C' support function for jitve_foo.c tests.
 * \sa jitpage.h and asmfmt.hpp for later versions of these functions
 */
#ifdef JITPAGE_H
#error "Error: When using jitpage.h, you should not use jitve_util.h"
#else
#warning "jitve_util.h is deprecated -- used for some old tests.  Please switch to jitpage.h"
#endif

#include <stdint.h> /* uint64_t */
#include <stdlib.h> /* size_t */

void asm2bin(char const* basename, char const* cpp_asm_code);
/** load a .bin file into page[4k] buffer.
 * assert .bin in <= 4k bytes first.
 * return */
size_t bin2page(char const* basename, char const* page);
/** more flexible API.  Return NULL on error. */
typedef struct {
	char const* addr;
    size_t const len;
} Jitpage;
Jitpage bin2jitpage(char const* basename);
void jitpage_free( Jitpage *jitpage );
/** reproduce hexdump -C "canonical hex+ASCII" output format */
void hexdump(char const* page, size_t sz);
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
 * NEEDS thorough testing !!!!!!!!
 *
 * string buffer \c mconst bounds \b unchecked !
 * \pre \c mconst can hold >= 6 characters
 *      (incl space for terminating nul)
 */
int strMconst(char *mconst,uint64_t const parm);
/* vim: set ts=4 sw=4 et: */
#endif // JITVE_UTIL_H
