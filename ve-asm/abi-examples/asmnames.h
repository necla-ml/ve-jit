#ifndef ASMNAMES_H
#define ASMNAMES_H

#define C2(X, Y)  X ## Y
#define C1(X, Y)  C2(X, Y)
#ifdef __USER_LABEL_PREFIX__
# define C(X)     C1(__USER_LABEL_PREFIX__, X)
#else
# define C(X)     X
#endif

#ifdef __APPLE__
# define L(X)     C1(L, X)
#else
# define L(X)     C1(.L, X)
#endif

#if defined(__ELF__) && defined(__PIC__)
# define PLT(X)	  X@PLT
#else
# define PLT(X)	  X
#endif

#ifdef __ELF__
# define ENDF(X)  .type	X,@function; .size X, . - X
#else
# define ENDF(X)
#endif


#define CNAME(x) C(x)

#define ENTRY(x) .balign 16; \
	.globl CNAME(x)
/* .type CNAME(x),%function; CNAME(x): */
.macro FUNC_START func
	.text
	.balign 16
CNAME(\func):
	.type	CNAME(\func), %function
	.global CNAME(\func)
.endm
#define FUNC_START(x) \
	.text; \
	.balign 16; \
CNAME(x): ; \
	.type	CNAME(x), %function; \
	.global CNAME(x);

#define CNAMESZ(x) C1(X,auto_size)

#define S18_TO_33(OP, FP) \
	OP %s18,48(,FP); \
	OP %s18,48(,FP); \
	OP %s19,56(,FP); \
	OP %s20,64(,FP); \
	OP %s21,72(,FP); \
	OP %s22,80(,FP); \
	OP %s23,88(,FP); \
	OP %s24,96(,FP); \
	OP %s25,104(,FP); \
	OP %s26,112(,FP); \
	OP %s27,120(,FP); \
	OP %s28,128(,FP); \
	OP %s29,136(,FP); \
	OP %s30,144(,FP); \
	OP %s31,152(,FP); \
	OP %s32,160(,FP); \
	OP %s33,168(,FP);

.macro set_new_stack_limit
	ld	%s61,0x18(,%tp)
	or	%s62,0,%s0
	lea	%s63,0x13b
	shm.l	%s63,0x0(%s61)
	shm.l	%sl,0x8(%s61)
	shm.l	%sp,0x10(%s61)
	monc				# trap?
	or	%s0,0,%s62
.endm

#if 0
####### caller's stack frame
#   Locals and Temporaries
# .............
#             | (Arg Stack, if needed for the call)
#    176(%fp) | Parameter Area for callee
# .............
# R S A..|
# e a r  | 168(%fp)   %s33 Callee-saved register
# g v e  |  ...        ...
# i e a  | 48(%fp)    %s18 Callee-saved register
# s      | 40(%fp)    %s17 Linkage Area Register 
# t      | 32(%fp)    %plt %s16 rocedure Linkage Table Register
# e      | 24(%fp)    %got %s15 Global Offset Table Register
# r......| 16(%fp)    %tp  %s14 Thread Pointer Register
#    8(%sp            %lr  %s10 return address (Link Register)
#    0(%sp)                %sp (or caller's $fp)
# ............ Caller Stack Frame (16-byte alignment)
# 
#    ### same, but wrt %sp for callee
# 
# ............ Callee Stack Frame (16-byte alignment)

#  This began with prologue and epilogue code appropriate for
#  functions returning void
#endif
#endif /* ASMNAMES_H */
