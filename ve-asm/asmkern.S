# An example of relocatable code that can be loaded "anywhere"
# from a ".bin" binary blob file (no relocation info or symbols)
.text
.align 16
asmkern:
	# Add 2 vectors of length 256*N
	#   s0[i] += s1[i] for i=0..256*N-1
	# Inputs:
	#   s0 = double* v0[256*N] (in vector format)
	#   s1 = double* v1[256*N] (in vector format)
	#   s2 = N, repeat count
	# Clobbers:
	#   s0--3, v0--1, VL
	breq.l.nt %s0,0,asmkern_copydone	# if null, skip copying part
	breq.l.nt %s2,0,asmkern_copydone	# if repeat count zero, skip copying part
	lea	%s3,256	# any difference?
	lvl	%s3
	sll	%s3,%s3,3
	br.l	asmkern_loop
asmkern_next:
	adds.l	%s0,%s0,%s3
	adds.l	%s1,%s1,%s3
asmkern_loop:
	vld.nc	%v0,8,%s0
	vld.nc	%v1,8,%s1
	vfadd.d	%v0,%v0,%v1
	vst.nc.ot %v0,8,%s0
	subu.l	%s3,%s3,(63)0
	brne.l	%s3,0,asmkern_next
	svob	# vector stores had .ot, and want vld to of s0[i] to be ok after return

asmkern_copydone:

	#
	# let's print out a Hello World 'debug' string as we exit
	# relocatable no-symbols version of write(1,"v.",2)
icbase:
	sic %s1
	lea  %s2, strstart-icbase(,%s1)
	ld1b %s3, szstring-icbase(,%s1)
	ld  %s4, 0x18(,%tp)	# SHM_OFFSET
	lea %s0,1		#  __NR_write=1 : in musl-libc-ve arch/ve/bits/syscall.h
	shm.l %s0, 0x0(%s4)
	lea %s1,1		# fd=1 for stdout
	shm.l %s1, 0x8(%s4)
	shm.l %s2, 0x10(%s4)	# ptr-to-bytes
	shm.l %s3, 0x18(%s4)	# #-of-bytes
	monc

	b.l	(,%lr)
	#
	# const data can be stored in the I-cache too:
szstring:
	.byte strend-strstart
strstart:
	.ascii "v."
strend:
	.align 3	# well, if we wanted more code...
	
