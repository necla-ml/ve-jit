   1              	# 1 "/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/test.S"
   1              	
   1              	/* Copyright (C) 2016-2018 NEC Corporation. */
   2              	.text
   3              	.align 16
   4              	PretendJITadd256:
   5              		# Pretend s0 = double* v0[256] (in vector format)
   6              		# Pretend s1 = double* v1[256] (in vector format)
   7              		# Clobbers s2
   8              		# Output s0 += s1 [0..255]
   9              		#ldu.w	%s2,256 # no instruction ldu.w
  10 0000 00010000 		lea	%s2,256	# any difference?
  10      00000206 
  11 0008 00000000 		lvl	%s2
  11      008200BF 
  12 0010 00000000 		vld	%v0,8,%s0
  12      80084081 
  13 0018 00000001 		vld	%v1,8,%s1
  13      81084081 
  14 0020 00010000 		vfadd.d	%v0,%v0,%v1
  14      000000CC 
  15              	PretendJitadd256_len4:
  16 0028 28000000 		.4byte	. - PretendJITadd256
  17              		
  18              		
