/** \file
 * simple demo of VE inline assembler vs C code,
 * to look at the produced assembly code. */
#ifndef OPT
#define OPT 1
#endif
#include <stdint.h>
//uint64_t sl3 __attribute__((used)) (uint64_t const x);
//void dbl_vadd256_count(double* s0, double*s1, unsigned long count);
//
inline uint64_t sl3i(uint64_t const x){
	return x<<3;
}
uint64_t sl3 __attribute__((used)) (uint64_t const x){
	return x<<3;
}
void dbl_vadd256_count_0(double* s0, double*s1, unsigned long count){
	unsigned long const x=256U;
	//unsigned long const y=x<<3;
	unsigned long const y = sl3(x);
	if(count==0) goto ret;
loop:
	for(unsigned i=0U; i<256; ++i)
		s0[i] += s1[i];
	if(--count==0) goto ret;
	s0 = (double*)( (char*)s0 + y );
	s1 = (double*)( (char*)s1 + y );
	goto loop;
ret:
	asm("nop");
}
void dbl_vadd256_count(double* s0, double*s1, unsigned long count){
	//unsigned long const x=256U;
	//unsigned long const y=x<<3;
	//unsigned long const y = sl3(x);
	asm(
			"lea	%s3,256\n"
			"\tlvl	%s3\n"
			"\tsll	%s3,%s3,3\n"
	   );
	asm(
			// If we were a raw asm function:
			//asm("beq.l.nt	%s2,0(,%s10)");	// if(count==0)return
			"breq.l.nt	%s2,0,ret\n"

			"\tbr.l		loop\n"
			"next:\n"
			"\tadds.l	%s0,%s0,%s3\n"
			"\tadds.l	%s1,%s1,%s3\n"
			"loop:\n"
			"\tvld.nc	%v0,8,%s0\n"
			"\tvld.nc	%v1,8,%s1\n"
			"\tvfadd.d	%v0,%v0,%v1\n"
			"\tvst.nc.ot	%v0,8,%s0\n"
			"\tsubu.l	%s3,%s3,(63)0\n"
			"\tbrne.l	%s3,0,next\n"
			"\tsvob\n"
			"ret:\n"
	   );
}


