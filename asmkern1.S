# An example of relocatable code that can be loaded "anywhere"
# from a ".bin" binary blob file (no relocation info or symbols)
# NEW: demo that we can use c preprocessor
.text
.align 16
asmkern:
// args:
#define PDST %s0
#define PADD %s1
#define REPEATCOUNT %s2
	# Add 2 vectors of length 256*N
	#   s0[i] += s1[i] for i=0..256*N-1
	# Inputs:
	#   s0 = double* v0[256*N] (in vector format)
	#   s1 = double* v1[256*N] (in vector format)
	#   s2 = N, repeat count
	# Clobbers:
	#   s0--3, v0--1, VL
	breq.l.nt PDST,0,asmkern_copydone	# if null, skip copying part
	breq.l.nt REPEATCOUNT,0,asmkern_copydone	# if repeat count zero, skip copying part
// scope0{
#define BYTES_256DOUBLES %s3
#define TMP %s4
#define VDST %v0
#define VADD %v1
#define P_1 (63)0	/* positive 1, 63 zero-bits then 1 one-bit, easier to read */
	lea	TMP,256
	lvl	TMP
	sll	BYTES_256DOUBLES, TMP,3 # TMP<<3, or 256*8 [bytes]
	br.l	asmkern_loop
asmkern_next:
	adds.l	PDST,PDST,BYTES_256DOUBLES
	adds.l	PADD,PADD,BYTES_256DOUBLES
asmkern_loop:
	vld.nc	VDST,8,PDST
	vld.nc	VADD,8,PADD
	vfadd.d	VDST,VDST,VADD
	vst.nc.ot VDST,8,PDST
	subu.l	REPEATCOUNT, REPEATCOUNT, P_1
	brne.l	REPEATCOUNT, 0,asmkern_next
	svob	# vector stores had .ot, and want vld to of s0[i] to be ok after return

asmkern_copydone:
// scope0}
#undef BYTES_256DOUBLES
#undef TMP
#undef VDST
#undef VADD
// delsym PDST PADD REPEATCOUNT remove symbol in currenct scope
#undef PDST
#undef PADD
#undef REPEATCOUNT

	#
	# let's print out a Hello World 'debug' string as we exit
	# relocatable no-symbols version of write(1,"v.",2)
icbase:
// This section has pretty tough register re-use pattern.
// Maybe some things are left best done "by hand"
// scope1{
// require:
#define SYSCALL_WRITE %s0
#define FILEDESCRIPTOR %s1
#define ABS_PSTRING %s2
#define LEN %s3

// tmp-alias (original symbol hidden, unusable)
#define HERE FILEDESCRIPTOR /* a far-forward symbol */
	sic HERE
	lea  ABS_PSTRING, strstart-icbase(,HERE)
	ld1b LEN, szstring-icbase(,HERE)
// del-alias HERE --> FILEDESCRIPTOR again usable
#undef HERE

// scope1a{
#define SHMARGS %s4
	lea SYSCALL_WRITE,1		#  __NR_write=1 : in musl-libc-ve arch/ve/bits/syscall.h
	ld  SHMARGS, 0x18(,%tp)	# SHM_OFFSET
	lea FILEDESCRIPTOR,1		# fd=1 for stdout
	shm.l SYSCALL_WRITE, 0x0(SHMARGS)
	shm.l FILEDESCRIPTOR, 0x8(SHMARGS)
	shm.l ABS_PSTRING, 0x10(SHMARGS)	# ptr-to-bytes
	shm.l LEN, 0x18(SHMARGS)	# #-of-bytes
	monc
// scope1}
#undef HERE
#undef FILEDESCRIPTOR
#undef ABS_PSTRING
#undef LEN
#undef SHMARGS
	b.l	(,%lr)
	#
	# const data can be stored in the I-cache too:
# .data ###.bin file would BEGIN with .data section, oops!!!
#          actually, the .data section OVERLAYed the first bytes of the .text secion :(
# .section heredata,"",@progbits  ### also failed to make good binary blob
szstring:
	.byte strend-strstart
strstart:
	.ascii "v."
strend:
	# nas aligns with nop, but I'd like zero-fill ?
	.align 3,0	# well, if we wanted more code...
	
