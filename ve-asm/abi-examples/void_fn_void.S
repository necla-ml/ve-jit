/* XXX these lose for some platforms, I'm sure. */
#define CNAME(x) x

#define ENTRY(x) .balign 16; .globl CNAME(x
/* .type CNAME(x),%function; CNAME(x): */

#define CNAMESZ(x) X##auto_size

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


	.text
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
ENTRY(ffi_call_SYSV)
	#stack frame .cfi directives are for nice stack debug
	.cfi_startproc
	st	%fp,0x0(,%sp)
	.cfi_def_cfa_offset 0
	.cfi_offset	9,0		# ? %fp is %s9?
	st	%lr,0x8(,%sp)# %lr~%s10 Link Reg: ret addr
	# skip %tp at 0x10(,%sp)
	st	%got,0x18(,%sp)
	st	%plt,0x20(,%sp)
	lea	%got,_GLOBAL_OFFSET_TABLE_@PC_LO(-0x18)
	and	%got,%got,(32)0
	sic	%plt		# %plt := IC + 8
	lea.sl	%got,_GLOBAL_OFFSET_TABLE_@PC_HI(%plt,%got)
	or	%fp,0,%sp	# %fp := %sp
	
	.cfi_def_cfa_register	9
	S18_TO_33( st, %fp )
	
#if 0
	lea	%s13,.L.1.2auto_size&0xffffffff
	and	%s13,%s13,(32)0
	lea.sl	%sp,.L.1.2auto_size>>32(%fp,%s13)
#else
	lea	%s13,CNAMESZ(ffi_call_SYSV)&0xffffffff
	and	%s13,%s13,(32)0
	lea.sl	%sp,CNAMESZ(ffi_call_SYSV)>>32(%fp,%s13)
#endif
	brge.l.t	%sp,%sl,.L_1.EoP
	
	ld	%s61,0x18(,%tp)
	or	%s62,0,%s0
	lea	%s63,0x13b
	shm.l	%s63,0x0(%s61)
	shm.l	%sl,0x8(%s61)
	shm.l	%sp,0x10(%s61)
	monc
	or	%s0,0,%s62
.L_EoP: # end prologue
	# ? %s12 Outer Register:
	#	"Used for pointing start address of called function"

# (call preg_args, invoke function, handle return values)

.L_BoE: # begin epilogue
	S18_TO_33( ld, %fp )	# Paranoia: %s18..33 belong to caller
	# Remove the space we pushed for the args
	or	%sp,0,%fp	# callee stack frame -> Gone!
	.cfi_def_cfa	11,8
	ld	%got,0x18(,%sp)
	ld	%plt,0x20(,%sp)
	ld	%lr, 0x8(,%sp)
	ld	%fp, 0x0(,%sp)
	b.l	(,%lr)
.L_EoE: #end of epilogue
	.cfi_endproc
	.size CNAME(ffi_call_SYSV), .L_EoE - CNAME(ffi_call_SYSV)
