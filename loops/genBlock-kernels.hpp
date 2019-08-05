#ifndef GENBLOCK_KERNELS_HPP
/* Copyright (c) 2019 by NEC Corporation
 * This file is part of ve-jit */
/** \file
 * Fused-triple-loop JIT kernel abstraction and default instances.
 */
#define GENBLOCK_KERNELS_HPP
#include "../cblock.hpp"
#include <iostream>

// deprecated in C++17 (clang++ warning)
#define REGISTER
// util for cblock.hpp
#define DEF(VAR) def(#VAR,VAR)

#define KRNBLK3_NONE 0
#define KRNBLK3_HASH 1
#define KRNBLK3_PRINT 2
#define KRNBLK3_CHECK 3
/** what kernel is default? */
#define WHICH_KERNEL KRNBLK3_CHECK

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

	struct KernelNeeds krnblk3_needs(int const which); ///< \deprecated
	char const*        krnblk3_name(int const which);  ///< \deprecated
}//extern "C"

struct KrnBlk3Abs {
	KrnBlk3Abs(std::string pfx="Fl_") : pfx(pfx) {}
	virtual ~KrnBlk3Abs() = 0;
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
			int64_t const klo, int64_t const kk,
			/* optional */
			int64_t const vl,
			std::string extraComment,
			int const v=0                              ///< verbose?
			) const = 0;
	std::string pfx;    ///< name local vars to disambiguate kernels. Must not be empty
	static std::ostringstream oss;              ///< for \ref OSSFMT
};
inline KrnBlk3Abs::~KrnBlk3Abs() {} // compulsory, even though empty

/** Fused-Loop VARiableS.
 * Code outside fused-loop-kernel might want to override variable names
 * used from within JIT kernels.  Ex. more descriptive variable names than
 * \c a and \c b, or multiple fused-loop-optimizations that should not use
 * conflicting variable names.
 */
struct KrnBlk3_vars {
#define KRNBLK3_ARGS \
	std::string vA, std::string vB, std::string vC, \
	std::string vSEQ0, std::string sVL, \
	std::string vSQIJ
#define KRNBLK3_CONSTRUCTOR_ARGS \
	std::string vA="a", std::string vB="b", std::string vC="c", \
	std::string vSEQ0="sq", std::string sVL="vl", \
	std::string vSQIJ="sqij"
#define KRNBLK3_CONSTRUCTOR_ARGS2 \
	std::string vA/*="a"*/, std::string vB/*="b"*/, std::string vC/*="c"*/, \
	std::string vSEQ0/*="sq"*/, std::string sVL/*="vl"*/, \
	std::string vSQIJ/*="sqij"*/
	/** code around the fused loop may name things differently.
	 * @param vA        ["a"] vector of for loop /e i indices
	 * @param vB        ["b"] vector of for loop /e j indices
	 * @param vC        ["c"] vector of for loop /e j indices
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
	KrnBlk3_vars( KRNBLK3_CONSTRUCTOR_ARGS )
		: vA(vA),vB(vB),vC(vC),vSEQ0(vSEQ0),sVL(sVL),vSQIJ(vSQIJ)
		  //{ std::cout<<" +vA="<<vA<<" +sVL="<<sVL<<std::endl; }
	{}
	void vars( KRNBLK3_CONSTRUCTOR_ARGS );
	std::string vA;
	std::string vB;
	std::string vC;
	std::string vSEQ0;
	std::string sVL;
	std::string vSQIJ;
};

/** add standardized string values to KrnBlk3Abs (still abstract class).
 * Feel free to set the \c pfx to something more descriptive, after construction.
 */
struct KrnBlk3: public KrnBlk3Abs, public KrnBlk3_vars
{
    /** constructor, always gives you the default \c pfx.
     * But you can set \c pfx manually after construction. */
    KrnBlk3(
            cprog::Cblock& bOuter, cprog::Cblock& bInner,
            KRNBLK3_CONSTRUCTOR_ARGS)
        : KrnBlk3Abs(), KrnBlk3_vars(vA,vB,vSEQ0,sVL,vSQIJ)
          , outer(bOuter), inner(bInner)
    { //std::cout<<" %vA="<<vA<<" %vB="<<vB<<" %vSEQ0="<<vSEQ0<<" %sVL_="<<sVL_<<" %vSQIJ="<<vSQIJ<<std::endl;
    }
    ~KrnBlk3() override {}
    cprog::Cblock& outer;
    cprog::Cblock& inner;
};

/** factory */
KrnBlk3* mkBlockingTestKernel(int const which,
        cprog::Cblock& outer, cprog::Cblock& inner,
        KRNBLK3_CONSTRUCTOR_ARGS);

//
// inlines .........................................
//
inline void KrnBlk3_vars::vars( KRNBLK3_ARGS ){
    this->vA = vA;
    this->vB = vB;
    this->vSEQ0 = vSEQ0;
    this->sVL = sVL;
    this->vSQIJ = vSQIJ;
}
/** \class KrnBlk3Abs
 * emit kernel code that operates on THREE vectorized loop-index registers.
 *
 * This is an abstract base class. \sa FusedLoopKernelAbs.
 */
/** \fn KrnBlk3Abs::needs()
 * Return flags telling optimizer about input data this kernel desires.
 * \sa FusedLoopKernel
 */

// vim: ts=4 sw=4 et cindent cino=^=l0,\:.5s,=-.5s,N-s,g.5s,b1 cinkeys=0{,0},0),\:,0#,!^F,o,O,e,0=break
#endif // GENBLOCK_KERNELS_HPP
