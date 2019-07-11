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
#define WHICH_KERNEL KERNEL_CHECK

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
std::string cfuse2_no_unroll(loop::Lpi const vl0, loop::Lpi const ii, loop::Lpi const jj,
        int const which=WHICH_KERNEL, int const verbose=1);

// front-end to \c cfuse2_no_unroll -- vectorize `for[0,ii){for[0,j]{...}}`.
//void test_vloop2_no_unrollX(loop::Lpi const vlen, loop::Lpi const ii, loop::Lpi const jj,
//        int const opt_t, int const which=WHICH_KERNEL, char const* ofname=nullptr);
std::string cfuse2_no_unrollX(loop::Lpi const vlen, loop::Lpi const ii, loop::Lpi const jj,
        int const opt_t, int const which=WHICH_KERNEL, char const* ofname=nullptr);

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

void fuse4_kernel(cprog::Cblock& bOuter, cprog::Cblock& bKrn,
        cprog::Cblock& bDef, cprog::Cblock& bOut,
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

/** z<=lo<=hi<=end for(z--lo)for(lo--hi)for(hi--end) loop split.
 * Originally loops were just for(0--end). */
struct LoopSplit {
    uint32_t z, lo, hi, end;
    LoopSplit(uint32_t end) : z(0U),lo(0U),hi(end),end(end) {
        assert( end >= z );
    }
    LoopSplit(uint32_t z,uint32_t end) : z(z),lo(z),hi(end),end(end) {
        assert(end >= z );
    }
    LoopSplit(uint32_t z,uint32_t lo, uint32_t hi, uint32_t end) : z(z),lo(lo),hi(hi),end(end) {
        assert( lo >= z );
        assert( hi >= lo );
        assert( end >= hi );
    }
};
struct LoopSplit parseLoopSplit(char const* arg);
std::ostream& operator<<(std::ostream& os, struct LoopSplit const& ls);

/** vl0<0 means use |vl0| or a lower alternate VL. */
std::string cf5_no_unroll(int const vl0, LoopSplit const& lsii, LoopSplit const& lsjj,
        int const which=WHICH_KERNEL, int const verbose=1);

std::string cf5_no_unrollX(loop::Lpi const vlen,
        LoopSplit const& lsii, LoopSplit const& lsjj,
        int const opt_t, int const which=WHICH_KERNEL, char const* ofname=nullptr);

std::string cf5_unroll(loop::Lpi const vl0, LoopSplit const& lsii, LoopSplit const& lsjj,
        int unroll, int cyc=0, int const which=WHICH_KERNEL, int const verbose=1);

std::string cf5_unrollX(loop::Lpi const vlen, LoopSplit const& lsii, LoopSplit const& lsjj,
        int const maxun, int const opt_t,
        int const which=WHICH_KERNEL, char const* ofname=nullptr);

/** loop splitting involves \c for(ilo..ihi)for(jlo..jhi) loops */
void cf5_kernel(cprog::Cblock& bOuter, cprog::Cblock& bDef,
        cprog::Cblock& bKrn, cprog::Cblock& bOut,
        int64_t const ilo, int64_t const ii,
        int64_t const jlo, int64_t const jj,
        int64_t const vl,
        std::string extraComment,
        int const which=0 /*comment,VecHash2*/,
        std::string pfx="cf5_",
        int const v=0 /*verbose*/,
        std::string vA="a", std::string vB="b",
        std::string vSEQ0="sq", std::string sVL="vl"
        );
// vim: ts=4 sw=4 et cindent cino=^=l0,\:.5s,=-.5s,N-s,g.5s,b1 cinkeys=0{,0},0),\:,0#,!^F,o,O,e,0=break
#endif // CF3_HPP
