#ifndef FL6_KERNELS_HPP
/* Copyright (c) 2019 by NEC Corporation
 * This file is part of ve-jit */
/** \file
 * Fused-loop JIT kernel abstraction and default instances.
 */
#define FL6_KERNELS_HPP
#include "../cblock.hpp"
//#include "../fuseloop.hpp"
//#include "../stringutil.hpp"
//#include <regex>
//#include <vector>
//#include <utility>
#include <iostream>

// deprecated in C++17 (clang++ warning)
#define REGISTER
// util for cblock.hpp
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
        uint8_t cnt;        ///< fd["have_cnt"]  cnt=0, vl0, 2*vl0, etc, also defined at exit of fused-loop scope
        uint8_t iijj;       ///< fd["have_iijj"] const ii*jj product of loop limits
        uint8_t sq;         ///< fd["have_sq"]   const {0..vl-1} vector
        uint8_t sqij;       ///< fd["have_sqij"] {cnt..cnt+vl-1} vector
        uint8_t vl;         ///< fd["have_vl"]   current VL scalar
    };

    struct KernelNeeds kernel_needs(int const which); ///< \deprecated
    char const*        kernel_name(int const which);  ///< \deprecated
}//extern "C"

struct FusedLoopKernelAbs {
    FusedLoopKernelAbs(std::string pfx="Fl_") : pfx(pfx) {}
    virtual ~FusedLoopKernelAbs() = 0;
    virtual char const* name() const = 0;            ///< print name
    virtual struct KernelNeeds needs() const = 0;
    /** add support for kernel into JIT code tree.
     * \p bOuter  function scope (around outer loops, unique)
     * \p bInner  at beginning of inner scope, unique
     * \p bDef    inner loop block with kernel invocations, multiple possible
     * \p bKrn    invoke the kernel (may change as \c emit calls occur)
     * \p bOut    final 'output' block (sometimes need this to avoid
     *            having compiler optimize away all kernel calls)
     * Since bOuter and bInner are "const" throughout a function scope,
     * they should be set up by concrete class constructors for readability.
     */
    virtual void emit(
            //cprog::Cblock& bOuter, cprog::Cblock& bInner,
            cprog::Cblock& bDef, cprog::Cblock& bKrn, cprog::Cblock& bOut,
            /* \e ALWAYS variables */
            int64_t const ilo, int64_t const ii,
            int64_t const jlo, int64_t const jj,
            /* optional */
            int64_t const vl,
            std::string extraComment,
            int const v=0                              ///< verbose?
            ) const = 0;
    std::string pfx;    ///< name local vars to disambiguate kernels. Must not be empty
    static std::ostringstream oss;              ///< for \ref OSSFMT
};
inline FusedLoopKernelAbs::~FusedLoopKernelAbs() {} // compulsory, even though empty

/** Fused-Loop VARiableS.
 * Code outside fused-loop-kernel might want to override variable names
 * used from within JIT kernels.  Ex. more descriptive variable names than
 * \c a and \c b, or multiple fused-loop-optimizations that should not use
 * conflicting variable names.
 */
struct FLvars {
#define FLVARS_ARGS \
    std::string vA, std::string vB, \
    std::string vSEQ0, std::string sVL, \
    std::string vSQIJ
#define FLVARS_CONSTRUCTOR_ARGS \
    std::string vA="a", std::string vB="b", \
    std::string vSEQ0="sq", std::string sVL="vl", \
    std::string vSQIJ="sqij"
#define FLVARS_CONSTRUCTOR_ARGS2 \
    std::string vA/*="a"*/, std::string vB/*="b"*/, \
    std::string vSEQ0/*="sq"*/, std::string sVL/*="vl"*/, \
    std::string vSQIJ/*="sqij"*/
    /** code around the fused loop may name things differently.
     * @param vA        ["a"] vector of for loop /e i indices
     * @param vB        ["b"] vector of for loop /e j indices
     * @param vSEQ0     ["sq"] vector integer sequence 0..MVL-1
     * @param sVL       ["vl"] vector length (scalar, last krn.emit might have vl<vl0)
     * @param vSQIJ     ["sqij"] vector sequence beginning at /c cnt, length vl0 (or vl)
     *
     * - Maybe also permit special names for:
     *   - \c sVL0="vl0"  initial vector length (const, typically \#defined)
     *   - \c sCNT="cnt"  current 0,1,..,ii*jj-1 start posn (incr by vl0)
     *   - \c iijj="iijj" constant \c ii*jj=(ihi-ilo)*(jhi-jlo) scalar
     *
     * - A kernel can ignore any of these it doesn't need.
     */
    FLvars( FLVARS_CONSTRUCTOR_ARGS )
        : vA(vA),vB(vB),vSEQ0(vSEQ0),sVL(sVL),vSQIJ(vSQIJ)
          //{ std::cout<<" +vA="<<vA<<" +sVL="<<sVL<<std::endl; }
    {}
    void vars( FLVARS_CONSTRUCTOR_ARGS );
    std::string vA;
    std::string vB;
    std::string vSEQ0;
    std::string sVL;
    std::string vSQIJ;
};

/** add standardized string values to FusedLoopKernelAbs (still abstract class).
 * Feel free to set the \c pfx to something more descriptive, after construction.
 */
struct FusedLoopKernel: public FusedLoopKernelAbs, public FLvars
{
    /** constructor, always gives you the default \c pfx.
     * But you can set \c pfx manually after construction. */
    FusedLoopKernel(
            cprog::Cblock& bOuter, cprog::Cblock& bInner,
            FLVARS_CONSTRUCTOR_ARGS)
        : FusedLoopKernelAbs(), FLvars(vA,vB,vSEQ0,sVL,vSQIJ)
          , outer(bOuter), inner(bInner)
    { //std::cout<<" %vA="<<vA<<" %vB="<<vB<<" %vSEQ0="<<vSEQ0<<" %sVL_="<<sVL_<<" %vSQIJ="<<vSQIJ<<std::endl;
    }
    ~FusedLoopKernel() override {}
    cprog::Cblock& outer;
    cprog::Cblock& inner;
};
/** factory */
FusedLoopKernel* mkFusedLoopKernel(int const which,
        cprog::Cblock& outer, cprog::Cblock& inner,
        FLVARS_CONSTRUCTOR_ARGS);

/** empty kernel - OK for browsing the generating JIT code. */
struct FLKRN_none final : public FusedLoopKernel
{
    FLKRN_none(cprog::Cblock& bOuter, cprog::Cblock& bInner,
            FLVARS_CONSTRUCTOR_ARGS )
        : FusedLoopKernel(bOuter,bInner,vA,vB,vSEQ0,sVL,vSQIJ)
    {/*std::cout<<"+NONE";*/}
    ~FLKRN_none() override {} // possible check proper tree state?
    char const* name() const override { return "NONE"; }
    struct KernelNeeds needs() const override;
    void emit(cprog::Cblock& bDef,
            cprog::Cblock& bKrn, cprog::Cblock& bOut,
            int64_t const ilo, int64_t const ii,
            int64_t const jlo, int64_t const jj,
            int64_t const vl, std::string extraComment, int const v=0/*verbose*/
            ) const override;
};
/** kernel that calculates a "correctness hash" of a[],b[] fused-loop index vectors. */
struct FLKRN_hash final : public FusedLoopKernel
{
    FLKRN_hash(cprog::Cblock& bOuter, cprog::Cblock& bInner,
            FLVARS_CONSTRUCTOR_ARGS )
        : FusedLoopKernel(bOuter,bInner,vA,vB,vSEQ0,sVL,vSQIJ)
    {/*std::cout<<"+HASH"*/;}
    ~FLKRN_hash() override {} // possible check proper tree state?
    char const* name() const override { return "HASH"; }
    struct KernelNeeds needs() const override;
    void emit(cprog::Cblock& bDef,
            cprog::Cblock& bKrn, cprog::Cblock& bOut,
            int64_t const ilo, int64_t const ii,
            int64_t const jlo, int64_t const jj,
            int64_t const vl, std::string extraComment, int const v=0/*verbose*/
            ) const override;
};
/** kernel the prints the a[], b[] fused-loop index vectors. */
struct FLKRN_print final : public FusedLoopKernel
{
    FLKRN_print(cprog::Cblock& bOuter, cprog::Cblock& bInner,
            FLVARS_CONSTRUCTOR_ARGS )
        : FusedLoopKernel(bOuter,bInner,vA,vB,vSEQ0,sVL,vSQIJ)
    {std::cout<<"+PRINT";}
    ~FLKRN_print() override {} // possible check proper tree state?
    char const* name() const override { return "PRINT"; }
    struct KernelNeeds needs() const override;
    void emit(cprog::Cblock& bDef,
            cprog::Cblock& bKrn, cprog::Cblock& bOut,
            int64_t const ilo, int64_t const ii,
            int64_t const jlo, int64_t const jj,
            int64_t const vl, std::string extraComment, int const v=0/*verbose*/
            ) const override;
};
/** kernel that executes a correctness check of a[], b[] fused-loop index vectors. */
struct FLKRN_check final : public FusedLoopKernel
{
    FLKRN_check(cprog::Cblock& bOuter, cprog::Cblock& bInner,
            FLVARS_CONSTRUCTOR_ARGS )
        : FusedLoopKernel(bOuter,bInner,vA,vB,vSEQ0,sVL,vSQIJ)
    {/*std::cout<<"+CHECK";*/}
    ~FLKRN_check() override {} // possible check proper tree state?
    char const* name() const override { return "CHECK"; }
    struct KernelNeeds needs() const override;
    void emit(cprog::Cblock& bDef,
            cprog::Cblock& bKrn, cprog::Cblock& bOut,
            int64_t const ilo, int64_t const ii,
            int64_t const jlo, int64_t const jj,
            int64_t const vl, std::string extraComment, int const v=0/*verbose*/
            ) const override;
};
/** dummy kernel that wants \c KernelNeeds an sqij input vector. */
struct FLKRN_sqij final : public FusedLoopKernel
{
    FLKRN_sqij(cprog::Cblock& bOuter, cprog::Cblock& bInner,
            FLVARS_CONSTRUCTOR_ARGS )
        : FusedLoopKernel(bOuter,bInner,vA,vB,vSEQ0,sVL,vSQIJ)
    {/*std::cout<<"+SQIJ";*/}
    ~FLKRN_sqij() override {} // possible check proper tree state?
    char const* name() const override { return "SQIJ"; }
    struct KernelNeeds needs() const override;
    void emit(cprog::Cblock& bDef,
            cprog::Cblock& bKrn, cprog::Cblock& bOut,
            int64_t const ilo, int64_t const ii,
            int64_t const jlo, int64_t const jj,
            int64_t const vl, std::string extraComment, int const v=0/*verbose*/
            ) const override;
};

//
// inlines .........................................
//
inline void FLvars::vars( FLVARS_ARGS ){
    this->vA = vA;
    this->vB = vB;
    this->vSEQ0 = vSEQ0;
    this->sVL = sVL;
    this->vSQIJ = vSQIJ;
}
/** \class FusedLoopKernelAbs
 * emit kernel code that operates on two vectorized loop-index registers.
 *
 * This is an abstract base class.
 *
 * Kernels may also require the fused loop optimizer to supply other scalar or
 * vector values associated with the looping, via \c needs().
 *
 * Code to execute the kernel is produce via \c emit(...)
 *
 * Notes:
 *
 * any re-usable data of the kernel should \e somehow
 * hoist such registers to our enclosing scope (... and maybe further)
 * Ex 1:  if b[] is const, A*b[]+C vector can be hoisted outside loops.
 *        (actually will later supply a loop-fuse that ALSO optimizes
 *         a generic lin.comb(a[]*A+b[]*B+C) for the inner loop)
 * Ex 2:  if a mask register is a function of a const b[] vector,
 *        and we have found b[] to be const (except for vl changes),
 *        the mask register can be hoisted to enclosing scope (outside loops)
 *        (perhaps a significant saving)
 * Ex 2:  sq register can be hoisted (AND combined with our sq?)
 *        instead of being recalculated
 */
/** \fn FusedLoopKernelAbs::needs()
 * Return flags telling optimizer about input data this kernel desires.
 *
 * Example:
 *
 * CHECK kernel should receive ilo and jlo to allow testing.
 * Given cnt', then i'=cnt'/jj and j'=cnt'%jj.
 * These correspond to i=ilo+i' and j=jlo+j',
 * so we expect a[i]=ilo+(cnt'+i)/jj
 * and          b[i]=jlo+(cnt'+i)%jj
 *
 * - Fused loop optimizer
 *   - ALWAYS supplies constant values (like \c ilo, \c jlo, \c ii, \c jj, \c vl0)
 *     - no need to flag such values via \c KernelNeeds
 *   - CAN provide constant values (like \c iijj)
 *   - CAN provide some scalar/vector values (like \c cnt, \c vl, \c sq, \c sqij)
 * - to all kernel codes
 *
 * So CHECK kernel might ask for variables \c cnt, \c vl, and \c iijj
 */
/** \fn virtual void FusedLoopKernelAbs::emit
 *
 * emit JIT kernel (comment/code).
 *
 * Consider loops for(i=ilo..ihi)for(j=jlo..jhi)
 *
 * - define \c ii = ihi-ilo, \c jj = jhi-jlo
 * - \c vl is vector length
 * - \c a[], \c b[] vectors of indices in [ilo,ihi) or [jlo,jhi)
 * - \c cnt = 0,1..ii*jj-1 scalar (not matching a[],b[] values)
 * - \c sqij = [cnt,cnt+1,...,cnt+vl-1] vector
 *
 * - if ilo==0, this behaves 'as usual'
 *   - \c cnt matches sequential values from \c a[0]*jj+b[0]
 *   - corresponds nicely with "output pixel", e.g.
 *
 * - otherwise, we instead loop with ii=ihi-ilo, jj=jhi-jlo
 *   - expected 'output pixel' values are then (ilo+a[])*jj+b[]
 *   - typically \c i*jj+j must be calculated with vector ops
 *     as a[]*jj+b[]
 *     - only when vl0%jj==0 does \c cnt have an easy relation with
 *       \c a[]*jj+b[], because here a[]=const=cnt%vl0.
 *
 * @param bKrn   code block for kernel
 * @param bDef   code block for defines (within an enclosing-scope)
 *           We may output to bDef.getRoot()["<STAR><STAR>/includes"]
 *           function calls may go to bDef["..<STAR>/fns/first"]
 * @param bOut   code block for kernel output (post-fused-loop)
 *
 * Notes:
 *
 * any re-usable data of the kernel should \e somehow
 * hoist such registers to our enclosing scope (... and maybe further)
 * Ex 1:  if b[] is const, A*b[]+C vector can be hoisted outside loops.
 *        (actually will later supply a loop-fuse that ALSO optimizes
 *         a generic lin.comb(a[]*A+b[]*B+C) for the inner loop)
 * Ex 2:  if a mask register is a function of a const b[] vector,
 *        and we have found b[] to be const (except for vl changes),
 *        the mask register can be hoisted to enclosing scope (outside loops)
 *        (perhaps a significant saving)
 * Ex 2:  sq register can be hoisted (AND combined with our sq?)
 *        instead of being recalculated
 *
 * This function will typically use the JIT tree to remember state/status information.
 * If you maintatin internal state, this function will need to be non-const.
 */

// vim: ts=4 sw=4 et cindent cino=^=l0,\:.5s,=-.5s,N-s,g.5s,b1 cinkeys=0{,0},0),\:,0#,!^F,o,O,e,0=break
#endif // FL6_KERNELS_HPP
