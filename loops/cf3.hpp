#ifndef CF3_HPP
/* Copyright (c) 2019 by NEC Corporation
 * This file is part of ve-jit */
#define CF3_HPP
#include "../fuseloop.hpp"
#include "../cblock.hpp"
#include "../stringutil.hpp"
#include <regex>
#include <vector>
#include <utility>

// deprecated in C++17 (clang++ warning)
#define REGISTER

#define DEF(VAR) def(#VAR,VAR)

#define KERNEL_NONE 0
#define KERNEL_HASH 1
#define KERNEL_PRINT 2
#define KERNEL_CHECK 3
#define KERNEL_SQIJ 4 /*just an example*/
/** what kernel is default? */
#define WHICH_KERNEL KERNEL_HASH

extern "C" {
/** bool flags indicating kernel requests for certain defined variables.
 * Variable existence is flagged with tags in the Cblock tree in a 'definitions'
 * code block. */
struct KernelNeeds{
    uint8_t cnt;        ///< fd["have_cnt"]  cnt=0, vl0, 2*vl0...
    uint8_t iijj;       ///< fd["have_iijj"] const ii*jj product of loop limits
    uint8_t sq;         ///< fd["have_sq"]   const {0..vl-1} vector
    uint8_t sqij;       ///< fd["have_sqij"] {cnt..cnt+vl-1} vector
    uint8_t vl;         ///< fd["have_vl"]   current VL scalar
};

struct KernelNeeds kernel_needs(int const which);

char const* kernel_name(int const which);
}//extern "C"

/** helper routine, after using unroll_suggest for a good VL \c vl0. */
std::string cfuse2_unroll(loop::Lpi const vl0, loop::Lpi const ii, loop::Lpi const jj,
        int unroll, int cyc=0, int const which=WHICH_KERNEL, int const verbose=1);

/** front-end to \c cfuse_unroll, using \c unroll_suggest to select vector length. */
std::string cfuse2_unrollX(loop::Lpi const vlen, loop::Lpi const ii, loop::Lpi const jj,
        int const maxun, int const opt_t,
        int const which=WHICH_KERNEL, char const* ofname=nullptr);

void fuse2_kernel(cprog::Cblock& bKrn, cprog::Cblock& bDef, cprog::Cblock& bOut,
        int64_t const ii, int64_t const jj, int64_t const vl,
        std::string extraComment,
        int const which=0/*comment,VecHash2*/,
        std::string pfx="fuse2_",
        int const v=0/*verbose*/,
        std::string vA="a", std::string vB="b",
	std::string vSEQ0="sq", std::string sVL="vl"
        );

/** This \e should go into \ref stringutil.hpp, but some VE hosts have old
 * g++ with problematic regex header. This function can be used to rename
 * variables from stock code strings to rename things like 'a','b','cnt'
 * to something that makes sense in a local context.
 *
 * Ex: unrolling over image height and width might read more naturally after
 * ```auto p1 = multiReplaceWord(program, {{"a","vrx"},{"b","vry"},{"cnt","pixel"}});```
 */
inline std::string multiReplaceWord(
        std::string haystack,
        std::vector<std::pair<std::string,std::string>> replacements)
{
    std::ostringstream oss;
    std::string ret = haystack;
    for(auto& r: replacements){
        ret = std::regex_replace( ret, std::regex(OSSFMT("\\b"<<r.first<<"\\b")), r.second );
    }
    return ret;
}
// vim: ts=4 sw=4 et cindent cino=^=l0,\:.5s,=-.5s,N-s,g.5s,b1 cinkeys=0{,0},0),\:,0#,!^F,o,O,e,0=break
#endif // CF3_HPP
