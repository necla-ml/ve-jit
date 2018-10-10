#include <unistd.h>
#include <sys/syscall.h>
/** magic offset for VE 'monc' calls to pass values to host.
 * \sa https://github.com/veos-sxarr-NEC/musl-libc-ve
 */
#define SHM_OFFSET 0x18
#define STR0(s...) #s
#define STR(s...) STR0(s)
#define LAB(s...) STR0(s\n)
#define TTR(s...) "\t" STR0(s) "\n"
/** ? override to route assembler code elsewhere? */
#define ASM(A,X...) asm(A X)
#define DOW(X...) do{X}while(0)
#define lea_c(S,CVP)        DOW(void* cvp=CVP; ASM(STR(or  S,0,%0) ,::"r"(cvp):STR(S)); );
#define lea_cx(S,CVP)       ASM(STR(or  S,0,%0) ,::"r"(CVP):STR(S) );

#define ilprt0(SYSCALL_NUM,STRING)	ASM(TTR(lea	%s0,SYSCALL_NUM) \
        TTR(lea %s1, 1) \
        TTR(sic %s4) \
        TTR(br.l 3f) \
		LAB(1:) \
        "\t.ascii " STR(STRING) STR(\n) \
		LAB(2:) \
        TTR(.align 3) \
		LAB(3:) \
		TTR(lea %s2, 0x8(,%s4)) \
		TTR(lea %s3, 2b-1b) \
        TTR(ld  %s4, 0x18(,%tp)) \
        TTR(shm.l %s0, 0x0(%s4)) \
        TTR(shm.l %s1, 0x8(%s4)) \
        TTR(shm.l %s2, 0x10(%s4)) \
        TTR(shm.l %s3, 0x18(%s4)) \
        TTR(monc) \
        ,:::"%s0","%s1","%s2","%s3","%s4")
/** inline-print macro for relocatable code block avoiding symbols.
 * Usage: \c ilprt("foo") behaves like libc call \c puts("foo")
 * or \c write(1,"foo",3).  This macro destroys %s0--%s4.
 * \ret %s0 (probably you will just ignore it).
 */
#define ilprt(STRING) ilprt0(SYS_write,STRING)

#if 0 // TBD
// if you begin the jit codepage with a symbol table,
// or you preload a register with the absolute cdoepage address,
// you could actually do some sort of function calls
// (but this is TBD)
#define ilprt_aux(SYSCALL_NUM) ASM( \
        LAB(ilprt_func:) \
        TTR(lea %s0,SYSCALL_NUM) \
        TTR(lea %s1, 1) \
        TTR(ldu %s2,0x0a(,%lr)) \
        TTR(lea %s3,0x8(,%lr)) \
        TTR(lea %s4,7(,%lr)) \
        TTR(and.l %lr,%s4,(61)1) \
        TTR(b.l (,%lr)) \
        ,:::"%s0","%s1","%s2","%s3","%s4")
#define ilprt_call(STRING) ASM( \
        TTR(
#endif

/** \file
 * Demonstrate how to print something on Aurora without any library calls.
 * This could be used to debug JIT kernels, since the binary blob has no
 * relocatable symbols.
 */
int main(int argc,char**argv){
	// syscall version of write(fd,str,len)
	syscall(SYS_write, 1, "Hello, world!\n", 14);

	// The next also outputs a string, but with no library function call
	// (directly use SYSV_write)
	const char* str = "Hello, syscall!\n"; // 16 bytes
	//                 1234567890123456
	int const len = 16;
	int const syscall_num = SYS_write;
	int const fd = 1;
	// syscall details:
	//    https://github.com/veos-sxarr-NEC/musl-libc-ve
	// files:	
	// 	arch/ve/bits/reg.h
	// 	src/internal/ve/syscall.S
	// 	src/thread/ve/syscall_cp.S
	// 	src/internal/syscall.h
	// Note: this emulated write(fd,str,len) is not cancellable
#if 0
	lea_cx(%s0, syscall_num);
	lea_cx(%s1, fd);
	lea_cx(%s2, str);
	lea_cx(%s3, len);
#else
	asm("lea %s0,%0"::"i"(SYS_write));	// immediate values compile a bit nicer
	asm("lea %s1,%0"::"i"(1));
	asm("lea %s2,(,%0)"::"r"(str));
	asm("lea %s3,%0"::"i"(16));
#endif
	asm(
			"ld		%s4, 0x18(,%tp)\n" // 0x18 == SHM_OFFSET
			"\tshm.l	%s0, 0x0(%s4)\n"
			"\tshm.l	%s1, 0x8(%s4)\n"
			"\tshm.l	%s2, 0x10(%s4)\n"
			"\tshm.l	%s3, 0x18(%s4)\n"
			"\tmonc\n"
			//"beq.l.t	0,0(,%lr)\n" // we are not a function call
			//:::"%s46","memory"
	   );

	// Now demo an ilprt version (inline print)
	//   1. take IC, branch around "here" string
	//   2. string absolute address at IC+8 for syscall
	// This creates a fully-relocatable code+data block that can
	// print debug messages from within jit kernel code.
	//
	// NOTE: .align 3  actually does alignment to 2^3 multiple
	//
	asm("lea %s0,%0"::"i"(SYS_write));
	asm("lea %s1,%0"::"i"(1));
	asm("sic %s4");			// %s4 <-- IC
	asm("br.l afterstring\n"	// 8-byte branch instruction
		"startstring:\n"	// 0x8(,%s4)
			"\t.ascii \"Hello, ilprt!\\n\"\n"
		"endstring:\n"
			"\t.align 3\n"
		"afterstring:\n"
			"\tlea %s2,0x8(,%s4)\n"
			"\tlea %s3,endstring-startstring\n"
	   );
	asm(
			"ld		%s4, 0x18(,%tp)\n" // 0x18 == SHM_OFFSET
			"\tshm.l	%s0, 0x0(%s4)\n"
			"\tshm.l	%s1, 0x8(%s4)\n"
			"\tshm.l	%s2, 0x10(%s4)\n"
			"\tshm.l	%s3, 0x18(%s4)\n"
			"\tmonc\n"
			//"beq.l.t	0,0(,%lr)\n" // we are not a function call
			//:::"%s46","memory"
	   );

    // as a final demo, the following macros can be adapted to
    // print a "here"-string inside a jit code block:
    ilprt0(SYS_write,"hello, ilprt0!\n");
    ilprt("hi, ilprt!\n");

	return 0;
}
/* vim: set sw=4 ts=4 et: */
