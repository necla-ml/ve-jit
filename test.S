
.text
.align 16
PretendJITadd256:
	# Pretend s0 = double* v0[256] (in vector format)
	# Pretend s1 = double* v1[256] (in vector format)
	# Clobbers s2
	# Output s0 += s1 [0..255]
	#ldu.w	%s2,256 # no instruction ldu.w
	lea	%s2,256	# any difference?
	lvl	%s2
	vld	%v0,8,%s0
	vld	%v1,8,%s1
	vfadd.d	%v0,%v0,%v1
PretendJitadd256_len4:
	.4byte	. - PretendJITadd256
	
	
