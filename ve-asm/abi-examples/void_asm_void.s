   1              	# 1 "/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/void_asm_void.S"
   1              	#include "asmnames.h"
   1              	/* Copyright (C) 2016-2018 NEC Corporation. */
   1              	#ifndef ASMNAMES_H
   2              	#define ASMNAMES_H
   3              	
   4              	#define C2(X, Y)  X ## Y
   5              	#define C1(X, Y)  C2(X, Y)
   6              	#ifdef __USER_LABEL_PREFIX__
   7              	# define C(X)     C1(__USER_LABEL_PREFIX__, X)
   8              	#else
   9              	# define C(X)     X
  10              	#endif
  11              	
  12              	#ifdef __APPLE__
  13              	# define L(X)     C1(L, X)
  14              	#else
  15              	# define L(X)     C1(.L, X)
  16              	#endif
  17              	
  18              	#if defined(__ELF__) && defined(__PIC__)
  19              	# define PLT(X)	  X@PLT
  20              	#else
  21              	# define PLT(X)	  X
  22              	#endif
  23              	
  24              	#ifdef __ELF__
  25              	# define ENDF(X)  .type	X,@function; .size X, . - X
  26              	#else
  27              	# define ENDF(X)
  28              	#endif
  29              	
  30              	
  31              	#define CNAME(x) C(x)
  32              	
  33              	#define ENTRY(x) .balign 16; \
  34              		.globl CNAME(x)
  35              	/* .type CNAME(x),%function; CNAME(x): */
  36              	.macro FUNC_START func
  37              		.text
  38              		.balign 16
  39              	CNAME(\func):
  41              		.global CNAME(\func)
  42              	.endm
  43              	#define FUNC_START(x) \
  44              		.text; \
  45              		.balign 16; \
  46              	CNAME(x): ; \
  48              		.global CNAME(x);
  49              	
  50              	#define CNAMESZ(x) C1(X,auto_size)
  51              	
  52              	#define S18_TO_33(OP, FP) \
  53              		OP %s18,48(,FP); \
  54              		OP %s18,48(,FP); \
  55              		OP %s19,56(,FP); \
  56              		OP %s20,64(,FP); \
  57              		OP %s21,72(,FP); \
  58              		OP %s22,80(,FP); \
  59              		OP %s23,88(,FP); \
  60              		OP %s24,96(,FP); \
  61              		OP %s25,104(,FP); \
  62              		OP %s26,112(,FP); \
  63              		OP %s27,120(,FP); \
  64              		OP %s28,128(,FP); \
  65              		OP %s29,136(,FP); \
  66              		OP %s30,144(,FP); \
  67              		OP %s31,152(,FP); \
  68              		OP %s32,160(,FP); \
  69              		OP %s33,168(,FP);
  70              	
  71              	.macro set_new_stack_limit
  72              		ld	%s61,0x18(,%tp)
  73              		or	%s62,0,%s0
  74              		lea	%s63,0x13b
  75              		shm.l	%s63,0x0(%s61)
  76              		shm.l	%sl,0x8(%s61)
  77              		shm.l	%sp,0x10(%s61)
  78              		monc				# trap?
  79              		or	%s0,0,%s62
  80              	.endm
  81              	
  82              	#if 0
   2              	#if 0
   3              	####### caller's stack frame
   4              	#   Locals and Temporaries
   5              	# .............
   6              	#             | (Arg Stack, if needed for the call)
   7              	#    176(%fp) | Parameter Area for callee
   8              	# .............
   9              	# R S A..|
  10              	# e a r  | 168(%fp)   %s33 Callee-saved register
  11              	# g v e  |  ...        ...
  12              	# i e a  | 48(%fp)    %s18 Callee-saved register
  13              	# s      | 40(%fp)    %s17 Linkage Area Register 
  14              	# t      | 32(%fp)    %plt %s16 rocedure Linkage Table Register
  15              	# e      | 24(%fp)    %got %s15 Global Offset Table Register
  16              	# r......| 16(%fp)    %tp  %s14 Thread Pointer Register
  17              	#    8(%sp            %lr  %s10 return address (Link Register)
  18              	#    0(%sp)                %sp (or caller's $fp)
  19              	# ............ Caller Stack Frame (16-byte alignment)
  20              	# 
  21              	#    ### same, but wrt %sp for callee
  22              	# 
  23              	# ............ Callee Stack Frame (16-byte alignment)
  24              	
  25              	#  This began with prologue and epilogue code appropriate for
  26              	#  functions returning void
  27              	#endif
  28              	#if 0
  29              		.text
  30              		.balign 16
  31              	ffi_call_SYSV:
  33              		.global ffi_call_SYSV
  34              	#elif 0
  35              	FUNC_START(ffi_call_SYSV)
  36              	#else
  37              	FUNC_START ffi_call_SYSV
  38              	#endif
  39              	.L_BoP:
  40              		#stack frame .cfi directives are for nice stack debug
  41              		.cfi_startproc
  42 0000 00000000 		st	%fp,0x0(,%sp)
  42      8B000911 
  43              		.cfi_def_cfa_offset 0
  44              		.cfi_offset	9,0		# ? %fp is %s9?
  45 0008 08000000 		st	%lr,0x8(,%sp)# %lr~%s10 Link Reg: ret addr
  45      8B000A11 
  46              		# skip %tp at 0x10(,%sp)
  47 0010 18000000 		st	%got,0x18(,%sp)
  47      8B000F11 
  48 0018 20000000 		st	%plt,0x20(,%sp)
  48      8B001011 
  49 0020 00000000 		lea	%got,_GLOBAL_OFFSET_TABLE_@PC_LO(-0x18)
  49      00680F06 
  50 0028 00000000 		and	%got,%got,(32)0
  50      608F0F44 
  51 0030 00000000 		sic	%plt		# %plt := IC + 8
  51      00001028 
  52 0038 00000000 		lea.sl	%got,_GLOBAL_OFFSET_TABLE_@PC_HI(%plt,%got)
  52      8F908F06 
  53 0040 00000000 		or	%fp,0,%sp	# %fp := %sp
  53      8B000945 
  54              		
  55              		.cfi_def_cfa_register	9
  56 0048 30000000 		S18_TO_33( st, %fp )
  56      89001211 
  56      30000000 
  56      89001211 
  56      38000000 
  57              		
  58              	#if 1
  59              		# set new stack pointer
  60              		# This is how the compiler allocates stack space for various
  61              		# asm-examples test cases.  It knows the required stack size
  62              		# and sets a .L.1.2auto_size as the STACK SPACE REQUIREMENT.
  63              		# Naively, this is 176 + sz(ParameterArea) + "locals and temporaries"
  64              		#
  65              		# We load a 32-bit value and adjust $sp to ??? MSBs + %fp + LSBs
  66              		# The compiler sets this to a negative value such as 0xfffffffffffffe10      # 496 Bytes
  67              		# is this 176[to RSA] + Parameter Area size?
  68              		#   not quite: asm/void_vn_void.c.s has 0xfffffffffffffe50      # 432 Bytes
  69              		#   body calls an extern void_fn_void ,, so it might need GOT and PLT info on stack ??
  70              		#
  71              		# Good: mt_void_fn_void has auto_size from ncc set as 0x0     # 0 Bytes
  72              		#
  73              		# don't see the lea.sl suffix meaning.
  74              		#
  75              		# If we new exactly how much we needed,
  76              		#    lea %sp, - <need stack size for func>(,%fp)
  77              		# for libffi, this is some size for the ffi_prep_args call? or what?
  78 00d0 00000000 		lea	%s13,CNAMESZ(ffi_call_SYSV)&0xffffffff
  78      00000D06 
  79 00d8 00000000 		and	%s13,%s13,(32)0
  79      608D0D44 
  80 00e0 00000000 		lea.sl	%sp,CNAMESZ(ffi_call_SYSV)>>32(%fp,%s13)
  80      8D898B06 
  81              	#endif
  82 00e8 00000000 		brge.l.t	%sp,%sl,.L_1.EoP
  82      888B3518 
  83 00f0 18000000 		set_new_stack_limit
  83      8E003D01 
  83      00000000 
  83      80003E45 
  83      3B010000 
  84              	.L_EoP:
  85              		# end prologue
  86              		# ? %s12 Outer Register:
  87              		#	"Used for pointing start address of called function"
  88              	
  89              	# (call preg_args, invoke function, handle return values)
  90              	.L_BoE: # begin epilogue
  91 0130 30000000 		S18_TO_33( ld, %fp )	# Paranoia: %s18..33 belong to caller
  91      89001201 
  91      30000000 
  91      89001201 
  91      38000000 
  92              		# Remove the space we pushed for the args
  93 01b8 00000000 		or	%sp,0,%fp	# callee stack frame -> Gone!
  93      89000B45 
  94              		.cfi_def_cfa	11,8
  95 01c0 18000000 		ld	%got,0x18(,%sp)
  95      8B000F01 
  96 01c8 20000000 		ld	%plt,0x20(,%sp)
  96      8B001001 
  97 01d0 08000000 		ld	%lr, 0x8(,%sp)
  97      8B000A01 
  98 01d8 00000000 		ld	%fp, 0x0(,%sp)
  98      8B000901 
  99 01e0 00000000 		b.l	(,%lr)
  99      8A000F19 
 100              	.L_EoE: //#end of epilogue
 101              		.cfi_endproc
 102              		.set CNAMESZ(ffi_call_SYSV), 0 # extra stack space requirement for our temporaries
 103              		#.size ffi_call_SYSV, .-CNAME(ffi_call_SYSV) # only for COFF??
 104              	
