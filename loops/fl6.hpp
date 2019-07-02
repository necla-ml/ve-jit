#ifndef FL6_HPP
/* Copyright (c) 2019 by NEC Corporation
 * This file is part of ve-jit */
/** \file
 * Fused-loop JIT kernel abstraction and default instances.
 */
#define FL6_HPP
#include "fl6-kernels.hpp"
#include "../fuseloop.hpp"
#include <cstdint>
#include <cassert>
#include <iosfwd>
//#include "../fuseloop.hpp"
//#include "../stringutil.hpp"
//#include <regex>
//#include <vector>
//#include <utility>

/** z<=lo<=hi<=end for(z--lo)for(lo--hi)for(hi--end) loop split.
 * Originally loops were just for(0--end).
 *
 * \todo LoopSplit should be a generic vector<int> with ':'-separated parse.
 * original has uint32_t loop limits, and int might need some special handling ???
 */
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

struct FusedLoopOpt {
    bool const altvl; /// perform a search for alternate lower VL (otherwise use stated VL as-is)
};

/** loop splitting involves \c for(ilo..ihi)for(jlo..jhi) loops */
void cf5_kernel(cprog::Cblock& bKrn, cprog::Cblock& bDef, cprog::Cblock& bOut,
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

/** produce fused-loop test program.
 *
 * @param vlen max VL: if +ve, use stated value; if -ve, search for lower VL;
 *         if 0, use -256 (appropriate for VE)
 */
std::string fl6_no_unrollX(loop::Lpi const vlen,
        LoopSplit const& lsii, LoopSplit const& lsjj,
        int const opt_t, int const which=WHICH_KERNEL,
        //FusedLoopKernel const& krn,
        char const* ofname=nullptr);

/** prototype fused-loop 'main' test program.
 * XXX fix construction (scaffold vs 'new' to allocate ?)*/
struct FusedLoopTest{
    FusedLoopTest(FusedLoopKernel& krn, std::string name="", int const v=0/*verbose*/)
        : pr((name.empty()?std::string{"FusedLoopTest"}:name), "C", v),
        krn(krn), outer_(nullptr), inner_(nullptr) {/*not yet usable*/}
    cprog::Cblock& outer();     ///< outside outer loops, often func scope
    cprog::Cblock& inner();     ///< where the fused-loop goes
    cprog::Cunit pr;
    FusedLoopKernel& krn;
  protected:
    cprog::Cblock* outer_;       ///< outside outer loops, often top-level function scope
    cprog::Cblock* inner_;       ///< where the fused-loop goes
};

class Fl6test final : public FusedLoopTest
{
  public:
    Fl6test(FusedLoopKernel& krn, int const v=0/*verbose*/);
};

/** New fuse-loop test program. */
std::string fl6_no_unrollY(LoopSplit const& lsii, LoopSplit const& lsjj,
        FusedLoopKernel& krn, loop::Lpi const vlen=0,
        char const* ofname=nullptr, int const v=0/*verbose*/);

std::string fl6_unrollY(LoopSplit const& lsii, LoopSplit const& lsjj,
        int const maxun, FusedLoopKernel& krn, loop::Lpi const vlen=0,
        char const* ofname=nullptr, int v=0/*verbose*/);

//
// inlines
//
inline cprog::Cblock& FusedLoopTest::outer() {
    assert(outer_!=nullptr);
    //std::cout<<" outer @ "<<outer_->fullpath()<<std::endl;
    //std::cout<<" inner @ "<<inner_->fullpath()<<std::endl;
    return *outer_;
    //return pr.root.at("**/fl6test/body");
    //return pr.root.at("fns/fl6test/body");
}
/** where the fused-loop goes */
inline cprog::Cblock& FusedLoopTest::inner() {
    assert(inner_!=nullptr);
    //std::cout<<" outer @ "<<outer_->fullpath()<<std::endl;
    //std::cout<<" inner @ "<<inner_->fullpath()<<std::endl;
    return *inner_;
    //return outer().at("**/loop2/body");
    //return outer().at("loop1/body/loop2/body");
}

// vim: ts=4 sw=4 et cindent cino=^=l0,\:.5s,=-.5s,N-s,g.5s,b1 cinkeys=0{,0},0),\:,0#,!^F,o,O,e,0=break
#endif // FL6_HPP
