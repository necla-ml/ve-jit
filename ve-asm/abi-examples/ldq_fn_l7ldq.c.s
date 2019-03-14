   1              		.ident "ncc 1.1.4 (Build 10:17:22 Apr 19 2018)"
   2              		.file "ldq_fn_l7ldq.c"
   3              		.file 1 "/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ldq_fn_l7ldq.c"
   4              		.file 2 "/opt/nec/ve/ncc/1.1.4/include/stdc-predef.h"
   5              		.file 3 "/opt/nec/ve/ncc/1.1.4/include/stdio.h"
   6              		.file 4 "/opt/nec/ve/ncc/1.1.4/include/yvals.h"
   7              		.file 5 "/opt/nec/ve/ncc/1.1.4/include/necvals.h"
   8              		.file 6 "/opt/nec/ve/ncc/1.1.4/include/stdarg.h"
   9              		.file 7 "/opt/nec/ve/musl/include/stdio.h"
  10              		.file 8 "/opt/nec/ve/musl/include/features.h"
  11              		.file 9 "/opt/nec/ve/musl/include/bits/alltypes.h"
  12              		.file 10 "/opt/nec/ve/ncc/1.1.4/include/complex.h"
  13              		.file 11 "/opt/nec/ve/ncc/1.1.4/include/_complex_builtin.h"
  14              	# ============ Begin  ldq_fn_l7ldq ============
  15              		.section	.bss
  16              		.local	ldq_ret
  17              	.comm ldq_ret,32,16
  18              		.text
  19              		.balign 16
  20              	.L_1.0:
  21              	# line 7
   1:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ldq_fn_l7ldq.c **** #include <stdio.h>
   2:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ldq_fn_l7ldq.c **** #include <complex.h>
   3:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ldq_fn_l7ldq.c **** 
   4:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ldq_fn_l7ldq.c **** extern long double complex ext_ldq_fn_l7ldq(long a,long b,long c,long d,long e,long f,long g,long d
   5:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ldq_fn_l7ldq.c **** 
   6:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ldq_fn_l7ldq.c **** static long double complex ldq_ret;
   7:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ldq_fn_l7ldq.c **** long double complex ldq_fn_l7ldq(long a,long b,long c,long d,long e,long f,long g,long double compl
  22              		.loc	1 7 0
  23              		.globl	ldq_fn_l7ldq
  25              	ldq_fn_l7ldq:
  26              		.cfi_startproc
  27 0000 00000000 		st	%fp,0x0(,%sp)
  27      8B000911 
  28              		.cfi_def_cfa_offset	0
  29              		.cfi_offset	9,0
  30 0008 08000000 		st	%lr,0x8(,%sp)
  30      8B000A11 
  31 0010 18000000 		st	%got,0x18(,%sp)
  31      8B000F11 
  32 0018 20000000 		st	%plt,0x20(,%sp)
  32      8B001011 
  33 0020 00000000 		lea	%got,_GLOBAL_OFFSET_TABLE_@PC_LO(-0x18)
  33      00680F06 
  34 0028 00000000 		and	%got,%got,(32)0
  34      608F0F44 
  35 0030 00000000 		sic	%plt
  35      00001028 
  36 0038 00000000 		lea.sl	%got,_GLOBAL_OFFSET_TABLE_@PC_HI(%plt,%got)
  36      8F908F06 
  37 0040 00000000 		or	%fp,0,%sp
  37      8B000945 
  38              		.cfi_def_cfa_register	9
  39 0048 30000000 		st	%s18,48(,%fp)
  39      89001211 
  40 0050 38000000 		st	%s19,56(,%fp)
  40      89001311 
  41 0058 40000000 		st	%s20,64(,%fp)
  41      89001411 
  42 0060 48000000 		st	%s21,72(,%fp)
  42      89001511 
  43 0068 50000000 		st	%s22,80(,%fp)
  43      89001611 
  44 0070 58000000 		st	%s23,88(,%fp)
  44      89001711 
  45 0078 60000000 		st	%s24,96(,%fp)
  45      89001811 
  46 0080 68000000 		st	%s25,104(,%fp)
  46      89001911 
  47 0088 70000000 		st	%s26,112(,%fp)
  47      89001A11 
  48 0090 78000000 		st	%s27,120(,%fp)
  48      89001B11 
  49 0098 80000000 		st	%s28,128(,%fp)
  49      89001C11 
  50 00a0 88000000 		st	%s29,136(,%fp)
  50      89001D11 
  51 00a8 90000000 		st	%s30,144(,%fp)
  51      89001E11 
  52 00b0 98000000 		st	%s31,152(,%fp)
  52      89001F11 
  53 00b8 A0000000 		st	%s32,160(,%fp)
  53      89002011 
  54 00c0 A8000000 		st	%s33,168(,%fp)
  54      89002111 
  55 00c8 A0FDFFFF 		lea	%s13,.L.1.2auto_size&0xffffffff
  55      00000D06 
  56 00d0 00000000 		and	%s13,%s13,(32)0
  56      608D0D44 
  57 00d8 FFFFFFFF 		lea.sl	%sp,.L.1.2auto_size>>32(%fp,%s13)
  57      8D898B06 
  58 00e0 48000000 		brge.l.t	%sp,%sl,.L_1.EoP
  58      888B3518 
  59 00e8 18000000 		ld	%s61,0x18(,%tp)
  59      8E003D01 
  60 00f0 00000000 		or	%s62,0,%s0
  60      80003E45 
  61 00f8 3B010000 		lea	%s63,0x13b
  61      00003F06 
  62 0100 00000000 		shm.l	%s63,0x0(%s61)
  62      BD033F31 
  63 0108 08000000 		shm.l	%sl,0x8(%s61)
  63      BD030831 
  64 0110 10000000 		shm.l	%sp,0x10(%s61)
  64      BD030B31 
  65 0118 00000000 		monc
  65      0000003F 
  66 0120 00000000 		or	%s0,0,%s62
  66      BE000045 
  67              	.L_1.EoP:
  68              	# End of prologue codes
  69              	# line 8
   8:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ldq_fn_l7ldq.c **** 	ldq_ret = ext_ldq_fn_l7ldq(a,b,c,d,e,f,g,ldq);
  70              		.loc	1 8 0
  71 0128 F0000000 		ld	%s35,240(0,%fp)	# ldq (real)
  71      89002301 
  72 0130 F0000000 		ld	%s34,240(8,%fp)	# ldq (real)
  72      89082201 
  73 0138 00010000 		ld	%s37,256(0,%fp)	# ldq (imaginary)
  73      89002501 
  74 0140 00010000 		ld	%s36,256(8,%fp)	# ldq (imaginary)
  74      89082401 
  75 0148 00000000 		or	%s63,0,%s34
  75      A2003F45 
  76 0150 F8000000 		st	%s63,248(0,%sp)
  76      8B003F11 
  77 0158 00000000 		or	%s62,0,%s35
  77      A3003E45 
  78 0160 F0000000 		st	%s62,240(0,%sp)
  78      8B003E11 
  79 0168 00000000 		or	%s61,0,%s36
  79      A4003D45 
  80 0170 08010000 		st	%s61,264(0,%sp)
  80      8B003D11 
  81 0178 00000000 		or	%s60,0,%s37
  81      A5003C45 
  82 0180 00010000 		st	%s60,256(0,%sp)
  82      8B003C11 
  83 0188 F8FFFFFF 		ld	%s7,-8(,%fp)	# restore
  83      89000701 
  84 0190 00000000 		lea	%s12,ext_ldq_fn_l7ldq@PLT_LO(-24)
  84      00680C06 
  85 0198 00000000 		and	%s12,%s12,(32)0
  85      608C0C44 
  86 01a0 00000000 		sic	%lr
  86      00000A28 
  87 01a8 00000000 		lea.sl	%s12,ext_ldq_fn_l7ldq@PLT_HI(%s12,%lr)
  87      8A8C8C06 
  88 01b0 00000000 		bsic	%lr,(,%s12)		# ext_ldq_fn_l7ldq
  88      8C000A08 
  89 01b8 E8FFFFFF 		st	%s0,-24(8,%fp)	# spill
  89      89080011 
  90 01c0 E8FFFFFF 		st	%s1,-24(,%fp)	# spill
  90      89000111 
  91 01c8 D8FFFFFF 		st	%s2,-40(8,%fp)	# spill
  91      89080211 
  92 01d0 D8FFFFFF 		st	%s3,-40(,%fp)	# spill
  92      89000311 
  93 01d8 08000000 		br.l	.L_1.1
  93      00000F18 
  94              	.L_1.1:
  95 01e0 00000000 		lea	%s63,ldq_ret@GOTOFF_LO
  95      00003F06 
  96 01e8 00000000 		and	%s63,%s63,(32)0
  96      60BF3F44 
  97 01f0 00000000 		lea.sl	%s63,ldq_ret@GOTOFF_HI(%s63,%got)
  97      8FBFBF06 
  98 01f8 E8FFFFFF 		ld	%s34,-24(8,%fp)	# restore
  98      89082201 
  99 0200 E8FFFFFF 		ld	%s35,-24(,%fp)	# restore
  99      89002301 
 100 0208 00000000 		st	%s35,0(0,%s63)	# ldq_ret (real)
 100      BF002311 
 101 0210 00000000 		st	%s34,0(8,%s63)	# ldq_ret (real)
 101      BF082211 
 102 0218 D8FFFFFF 		ld	%s36,-40(8,%fp)	# restore
 102      89082401 
 103 0220 D8FFFFFF 		ld	%s37,-40(,%fp)	# restore
 103      89002501 
 104 0228 10000000 		st	%s37,16(0,%s63)	# ldq_ret (imaginary)
 104      BF002511 
 105 0230 10000000 		st	%s36,16(8,%s63)	# ldq_ret (imaginary)
 105      BF082411 
 106              	# line 9
   9:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ldq_fn_l7ldq.c **** 	return ldq_ret;
 107              		.loc	1 9 0
 108 0238 00000000 		or	%s62,0,%s34
 108      A2003E45 
 109 0240 00000000 		or	%s61,0,%s35
 109      A3003D45 
 110 0248 00000000 		or	%s60,0,%s36
 110      A4003C45 
 111 0250 00000000 		or	%s59,0,%s37
 111      A5003B45 
 112 0258 00000000 		or	%s0,0,%s62
 112      BE000045 
 113 0260 00000000 		or	%s1,0,%s61
 113      BD000145 
 114 0268 00000000 		or	%s2,0,%s60
 114      BC000245 
 115 0270 00000000 		or	%s3,0,%s59
 115      BB000345 
 116              	# Start of epilogue codes
 117 0278 00000000 		or	%sp,0,%fp
 117      89000B45 
 118              		.cfi_def_cfa	11,8
 119 0280 18000000 		ld	%got,0x18(,%sp)
 119      8B000F01 
 120 0288 20000000 		ld	%plt,0x20(,%sp)
 120      8B001001 
 121 0290 08000000 		ld	%lr,0x8(,%sp)
 121      8B000A01 
 122 0298 00000000 		ld	%fp,0x0(,%sp)
 122      8B000901 
 123 02a0 00000000 		b.l	(,%lr)
 123      8A000F19 
 124              	.L_1.EoE:
 125              		.cfi_endproc
 126              		.set	.L.1.2auto_size,	0xfffffffffffffda0	# 608 Bytes
 128              	# ============ End  ldq_fn_l7ldq ============
 129              	# ============ Begin  alt_ldq_fn_l7ldq ============
 130 02a8 00000000 		.balign 16
 130      00000000 
 131              	.L_2.0:
 132              	# line 11
  10:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ldq_fn_l7ldq.c **** }
  11:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ldq_fn_l7ldq.c **** long double complex alt_ldq_fn_l7ldq(long a,long b,long c,long d,long e,long f,long g,long double c
 133              		.loc	1 11 0
 134              		.globl	alt_ldq_fn_l7ldq
 136              	alt_ldq_fn_l7ldq:
 137              		.cfi_startproc
 138 02b0 00000000 		st	%fp,0x0(,%sp)
 138      8B000911 
 139              		.cfi_def_cfa_offset	0
 140              		.cfi_offset	9,0
 141 02b8 08000000 		st	%lr,0x8(,%sp)
 141      8B000A11 
 142 02c0 18000000 		st	%got,0x18(,%sp)
 142      8B000F11 
 143 02c8 20000000 		st	%plt,0x20(,%sp)
 143      8B001011 
 144 02d0 00000000 		lea	%got,_GLOBAL_OFFSET_TABLE_@PC_LO(-0x18)
 144      00680F06 
 145 02d8 00000000 		and	%got,%got,(32)0
 145      608F0F44 
 146 02e0 00000000 		sic	%plt
 146      00001028 
 147 02e8 00000000 		lea.sl	%got,_GLOBAL_OFFSET_TABLE_@PC_HI(%plt,%got)
 147      8F908F06 
 148 02f0 00000000 		or	%fp,0,%sp
 148      8B000945 
 149              		.cfi_def_cfa_register	9
 150 02f8 30000000 		st	%s18,48(,%fp)
 150      89001211 
 151 0300 38000000 		st	%s19,56(,%fp)
 151      89001311 
 152 0308 40000000 		st	%s20,64(,%fp)
 152      89001411 
 153 0310 48000000 		st	%s21,72(,%fp)
 153      89001511 
 154 0318 50000000 		st	%s22,80(,%fp)
 154      89001611 
 155 0320 58000000 		st	%s23,88(,%fp)
 155      89001711 
 156 0328 60000000 		st	%s24,96(,%fp)
 156      89001811 
 157 0330 68000000 		st	%s25,104(,%fp)
 157      89001911 
 158 0338 70000000 		st	%s26,112(,%fp)
 158      89001A11 
 159 0340 78000000 		st	%s27,120(,%fp)
 159      89001B11 
 160 0348 80000000 		st	%s28,128(,%fp)
 160      89001C11 
 161 0350 88000000 		st	%s29,136(,%fp)
 161      89001D11 
 162 0358 90000000 		st	%s30,144(,%fp)
 162      89001E11 
 163 0360 98000000 		st	%s31,152(,%fp)
 163      89001F11 
 164 0368 A0000000 		st	%s32,160(,%fp)
 164      89002011 
 165 0370 A8000000 		st	%s33,168(,%fp)
 165      89002111 
 166 0378 90FDFFFF 		lea	%s13,.L.2.2auto_size&0xffffffff
 166      00000D06 
 167 0380 00000000 		and	%s13,%s13,(32)0
 167      608D0D44 
 168 0388 FFFFFFFF 		lea.sl	%sp,.L.2.2auto_size>>32(%fp,%s13)
 168      8D898B06 
 169 0390 48000000 		brge.l.t	%sp,%sl,.L_2.EoP
 169      888B3518 
 170 0398 18000000 		ld	%s61,0x18(,%tp)
 170      8E003D01 
 171 03a0 00000000 		or	%s62,0,%s0
 171      80003E45 
 172 03a8 3B010000 		lea	%s63,0x13b
 172      00003F06 
 173 03b0 00000000 		shm.l	%s63,0x0(%s61)
 173      BD033F31 
 174 03b8 08000000 		shm.l	%sl,0x8(%s61)
 174      BD030831 
 175 03c0 10000000 		shm.l	%sp,0x10(%s61)
 175      BD030B31 
 176 03c8 00000000 		monc
 176      0000003F 
 177 03d0 00000000 		or	%s0,0,%s62
 177      BE000045 
 178              	.L_2.EoP:
 179              	# End of prologue codes
 180              	# line 12
  12:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ldq_fn_l7ldq.c **** 	long double complex *p_ret = &ldq_ret;
 181              		.loc	1 12 0
 182 03d8 00000000 		lea	%s63,ldq_ret@GOTOFF_LO
 182      00003F06 
 183 03e0 00000000 		and	%s63,%s63,(32)0
 183      60BF3F44 
 184 03e8 00000000 		lea.sl	%s63,ldq_ret@GOTOFF_HI(%s63,%got)
 184      8FBFBF06 
 185 03f0 00000000 		or	%s62,%s63,(0)1
 185      00BF3E45 
 186              	# line 13
  13:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ldq_fn_l7ldq.c **** 	*p_ret = ext_ldq_fn_l7ldq(a,b,c,d,e,f,g,ldq);
 187              		.loc	1 13 0
 188 03f8 F0000000 		ld	%s35,240(0,%fp)	# ldq (real)
 188      89002301 
 189 0400 F0000000 		ld	%s34,240(8,%fp)	# ldq (real)
 189      89082201 
 190 0408 00010000 		ld	%s37,256(0,%fp)	# ldq (imaginary)
 190      89002501 
 191 0410 00010000 		ld	%s36,256(8,%fp)	# ldq (imaginary)
 191      89082401 
 192 0418 00000000 		or	%s61,0,%s34
 192      A2003D45 
 193 0420 F8000000 		st	%s61,248(0,%sp)
 193      8B003D11 
 194 0428 00000000 		or	%s60,0,%s35
 194      A3003C45 
 195 0430 F0000000 		st	%s60,240(0,%sp)
 195      8B003C11 
 196 0438 00000000 		or	%s59,0,%s36
 196      A4003B45 
 197 0440 08010000 		st	%s59,264(0,%sp)
 197      8B003B11 
 198 0448 00000000 		or	%s58,0,%s37
 198      A5003A45 
 199 0450 00010000 		st	%s58,256(0,%sp)
 199      8B003A11 
 200 0458 F8FFFFFF 		ld	%s7,-8(,%fp)	# restore
 200      89000701 
 201 0460 F0FFFFFF 		st	%s62,-16(,%fp)	# spill
 201      89003E11 
 202 0468 00000000 		lea	%s12,ext_ldq_fn_l7ldq@PLT_LO(-24)
 202      00680C06 
 203 0470 00000000 		and	%s12,%s12,(32)0
 203      608C0C44 
 204 0478 00000000 		sic	%lr
 204      00000A28 
 205 0480 00000000 		lea.sl	%s12,ext_ldq_fn_l7ldq@PLT_HI(%s12,%lr)
 205      8A8C8C06 
 206 0488 00000000 		bsic	%lr,(,%s12)		# ext_ldq_fn_l7ldq
 206      8C000A08 
 207 0490 E0FFFFFF 		st	%s0,-32(8,%fp)	# spill
 207      89080011 
 208 0498 E0FFFFFF 		st	%s1,-32(,%fp)	# spill
 208      89000111 
 209 04a0 D0FFFFFF 		st	%s2,-48(8,%fp)	# spill
 209      89080211 
 210 04a8 D0FFFFFF 		st	%s3,-48(,%fp)	# spill
 210      89000311 
 211 04b0 08000000 		br.l	.L_2.1
 211      00000F18 
 212              	.L_2.1:
 213 04b8 F0FFFFFF 		ld	%s63,-16(,%fp)	# restore
 213      89003F01 
 214 04c0 E0FFFFFF 		ld	%s34,-32(8,%fp)	# restore
 214      89082201 
 215 04c8 E0FFFFFF 		ld	%s35,-32(,%fp)	# restore
 215      89002301 
 216 04d0 00000000 		st	%s35,0(0,%s63)	# *(p_ret) (real)
 216      BF002311 
 217 04d8 00000000 		st	%s34,0(8,%s63)	# *(p_ret) (real)
 217      BF082211 
 218 04e0 D0FFFFFF 		ld	%s36,-48(8,%fp)	# restore
 218      89082401 
 219 04e8 D0FFFFFF 		ld	%s37,-48(,%fp)	# restore
 219      89002501 
 220 04f0 10000000 		st	%s37,16(0,%s63)	# *(p_ret) (imaginary)
 220      BF002511 
 221 04f8 10000000 		st	%s36,16(8,%s63)	# *(p_ret) (imaginary)
 221      BF082411 
 222              	# line 14
  14:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ldq_fn_l7ldq.c **** 	return *p_ret;
 223              		.loc	1 14 0
 224 0500 00000000 		or	%s62,0,%s34
 224      A2003E45 
 225 0508 00000000 		or	%s61,0,%s35
 225      A3003D45 
 226 0510 00000000 		or	%s60,0,%s36
 226      A4003C45 
 227 0518 00000000 		or	%s59,0,%s37
 227      A5003B45 
 228 0520 00000000 		or	%s0,0,%s62
 228      BE000045 
 229 0528 00000000 		or	%s1,0,%s61
 229      BD000145 
 230 0530 00000000 		or	%s2,0,%s60
 230      BC000245 
 231 0538 00000000 		or	%s3,0,%s59
 231      BB000345 
 232              	# Start of epilogue codes
 233 0540 00000000 		or	%sp,0,%fp
 233      89000B45 
 234              		.cfi_def_cfa	11,8
 235 0548 18000000 		ld	%got,0x18(,%sp)
 235      8B000F01 
 236 0550 20000000 		ld	%plt,0x20(,%sp)
 236      8B001001 
 237 0558 08000000 		ld	%lr,0x8(,%sp)
 237      8B000A01 
 238 0560 00000000 		ld	%fp,0x0(,%sp)
 238      8B000901 
 239 0568 00000000 		b.l	(,%lr)
 239      8A000F19 
 240              	.L_2.EoE:
 241              		.cfi_endproc
 242              		.set	.L.2.2auto_size,	0xfffffffffffffd90	# 624 Bytes
 244              	# ============ End  alt_ldq_fn_l7ldq ============
 245              	# ============ Begin  mt_ldq_fn_l7ldq ============
 246              		.balign 16
 247              	.L_3.0:
 248              	# line 16
  15:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ldq_fn_l7ldq.c **** }
  16:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ldq_fn_l7ldq.c **** long double complex mt_ldq_fn_l7ldq(long a,long b,long c,long d,long e,long f,long g,long double co
 249              		.loc	1 16 0
 250              		.globl	mt_ldq_fn_l7ldq
 252              	mt_ldq_fn_l7ldq:
 253              		.cfi_startproc
 254 0570 00000000 		st	%fp,0x0(,%sp)
 254      8B000911 
 255              		.cfi_def_cfa_offset	0
 256              		.cfi_offset	9,0
 257 0578 08000000 		st	%lr,0x8(,%sp)
 257      8B000A11 
 258 0580 18000000 		st	%got,0x18(,%sp)
 258      8B000F11 
 259 0588 20000000 		st	%plt,0x20(,%sp)
 259      8B001011 
 260 0590 00000000 		lea	%got,_GLOBAL_OFFSET_TABLE_@PC_LO(-0x18)
 260      00680F06 
 261 0598 00000000 		and	%got,%got,(32)0
 261      608F0F44 
 262 05a0 00000000 		sic	%plt
 262      00001028 
 263 05a8 00000000 		lea.sl	%got,_GLOBAL_OFFSET_TABLE_@PC_HI(%plt,%got)
 263      8F908F06 
 264 05b0 00000000 		or	%fp,0,%sp
 264      8B000945 
 265              		.cfi_def_cfa_register	9
 266 05b8 30000000 		st	%s18,48(,%fp)
 266      89001211 
 267 05c0 38000000 		st	%s19,56(,%fp)
 267      89001311 
 268 05c8 40000000 		st	%s20,64(,%fp)
 268      89001411 
 269 05d0 48000000 		st	%s21,72(,%fp)
 269      89001511 
 270 05d8 50000000 		st	%s22,80(,%fp)
 270      89001611 
 271 05e0 58000000 		st	%s23,88(,%fp)
 271      89001711 
 272 05e8 60000000 		st	%s24,96(,%fp)
 272      89001811 
 273 05f0 68000000 		st	%s25,104(,%fp)
 273      89001911 
 274 05f8 70000000 		st	%s26,112(,%fp)
 274      89001A11 
 275 0600 78000000 		st	%s27,120(,%fp)
 275      89001B11 
 276 0608 80000000 		st	%s28,128(,%fp)
 276      89001C11 
 277 0610 88000000 		st	%s29,136(,%fp)
 277      89001D11 
 278 0618 90000000 		st	%s30,144(,%fp)
 278      89001E11 
 279 0620 98000000 		st	%s31,152(,%fp)
 279      89001F11 
 280 0628 A0000000 		st	%s32,160(,%fp)
 280      89002011 
 281 0630 A8000000 		st	%s33,168(,%fp)
 281      89002111 
 282 0638 E0FFFFFF 		lea	%s13,.L.3.2auto_size&0xffffffff
 282      00000D06 
 283 0640 00000000 		and	%s13,%s13,(32)0
 283      608D0D44 
 284 0648 FFFFFFFF 		lea.sl	%sp,.L.3.2auto_size>>32(%fp,%s13)
 284      8D898B06 
 285 0650 48000000 		brge.l.t	%sp,%sl,.L_3.EoP
 285      888B3518 
 286 0658 18000000 		ld	%s61,0x18(,%tp)
 286      8E003D01 
 287 0660 00000000 		or	%s62,0,%s0
 287      80003E45 
 288 0668 3B010000 		lea	%s63,0x13b
 288      00003F06 
 289 0670 00000000 		shm.l	%s63,0x0(%s61)
 289      BD033F31 
 290 0678 08000000 		shm.l	%sl,0x8(%s61)
 290      BD030831 
 291 0680 10000000 		shm.l	%sp,0x10(%s61)
 291      BD030B31 
 292 0688 00000000 		monc
 292      0000003F 
 293 0690 00000000 		or	%s0,0,%s62
 293      BE000045 
 294              	.L_3.EoP:
 295              	# End of prologue codes
 296              	# line 17
  17:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ldq_fn_l7ldq.c **** 	long double complex ret = ldq+1.49;
 297              		.loc	1 17 0
 298 0698 F0000000 		ld	%s35,240(0,%fp)	# ldq (real)
 298      89002301 
 299 06a0 F0000000 		ld	%s34,240(8,%fp)	# ldq (real)
 299      89082201 
 300 06a8 00010000 		ld	%s37,256(0,%fp)	# ldq (imaginary)
 300      89002501 
 301 06b0 00010000 		ld	%s36,256(8,%fp)	# ldq (imaginary)
 301      89082401 
 302 06b8 3D0AD7A3 		lea	%s63,-1546188227
 302      00003F06 
 303 06c0 00000000 		and	%s62,%s63,(32)0
 303      60BF3E44 
 304 06c8 707DFF3F 		lea.sl	%s61,1073708400(,%s62)
 304      BE00BD06 
 305 06d0 00000070 		lea.sl	%s60,1879048192
 305      0000BC06 
 306 06d8 00000000 		or	%s38,0,%s61
 306      BD002645 
 307 06e0 00000000 		or	%s39,0,%s60
 307      BC002745 
 308 06e8 00000000 		fadd.q	%s40,%s34,%s38
 308      A6A2286C 
 309 06f0 E0FFFFFF 		st	%s41,-32(0,%fp)	# ret (real)
 309      89002911 
 310 06f8 E0FFFFFF 		st	%s40,-32(8,%fp)	# ret (real)
 310      89082811 
 311 0700 F0FFFFFF 		st	%s37,-16(0,%fp)	# ret (imaginary)
 311      89002511 
 312 0708 F0FFFFFF 		st	%s36,-16(8,%fp)	# ret (imaginary)
 312      89082411 
 313              	# line 18
  18:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ldq_fn_l7ldq.c **** 	return ret;
 314              		.loc	1 18 0
 315 0710 00000000 		or	%s59,0,%s40
 315      A8003B45 
 316 0718 00000000 		or	%s58,0,%s41
 316      A9003A45 
 317 0720 00000000 		or	%s57,0,%s36
 317      A4003945 
 318 0728 00000000 		or	%s56,0,%s37
 318      A5003845 
 319 0730 00000000 		or	%s0,0,%s59
 319      BB000045 
 320 0738 00000000 		or	%s1,0,%s58
 320      BA000145 
 321 0740 00000000 		or	%s2,0,%s57
 321      B9000245 
 322 0748 00000000 		or	%s3,0,%s56
 322      B8000345 
 323              	# Start of epilogue codes
 324 0750 00000000 		or	%sp,0,%fp
 324      89000B45 
 325              		.cfi_def_cfa	11,8
 326 0758 18000000 		ld	%got,0x18(,%sp)
 326      8B000F01 
 327 0760 20000000 		ld	%plt,0x20(,%sp)
 327      8B001001 
 328 0768 08000000 		ld	%lr,0x8(,%sp)
 328      8B000A01 
 329 0770 00000000 		ld	%fp,0x0(,%sp)
 329      8B000901 
 330 0778 00000000 		b.l	(,%lr)
 330      8A000F19 
 331              	.L_3.EoE:
 332              		.cfi_endproc
 333              		.set	.L.3.2auto_size,	0xffffffffffffffe0	# 32 Bytes
 335              	# ============ End  mt_ldq_fn_l7ldq ============
 336              	# ============ Begin  prt_ldq_fn_l7ldq ============
 337              		.section .rodata
 338              		.balign 16
 340              	.LP.__string.0:
 341 0000 25       		.byte	37
 342 0001 73       		.byte	115
 343 0002 3A       		.byte	58
 344 0003 25       		.byte	37
 345 0004 75       		.byte	117
 346 0005 20       		.byte	32
 347 0006 25       		.byte	37
 348 0007 66       		.byte	102
 349 0008 2B       		.byte	43
 350 0009 25       		.byte	37
 351 000a 66       		.byte	102
 352 000b 69       		.byte	105
 353 000c 0A       		.byte	10
 354 000d 00       		.zero	1
 355 000e 0000     		.balign 8
 357              	.LP.__string.1:
 358 0010 2F       		.byte	47
 359 0011 75       		.byte	117
 360 0012 73       		.byte	115
 361 0013 72       		.byte	114
 362 0014 2F       		.byte	47
 363 0015 75       		.byte	117
 364 0016 68       		.byte	104
 365 0017 6F       		.byte	111
 366 0018 6D       		.byte	109
 367 0019 65       		.byte	101
 368 001a 2F       		.byte	47
 369 001b 61       		.byte	97
 370 001c 75       		.byte	117
 371 001d 72       		.byte	114
 372 001e 6F       		.byte	111
 373 001f 72       		.byte	114
 374 0020 61       		.byte	97
 375 0021 2F       		.byte	47
 376 0022 34       		.byte	52
 377 0023 67       		.byte	103
 378 0024 69       		.byte	105
 379 0025 2F       		.byte	47
 380 0026 6E       		.byte	110
 381 0027 6C       		.byte	108
 382 0028 61       		.byte	97
 383 0029 62       		.byte	98
 384 002a 68       		.byte	104
 385 002b 70       		.byte	112
 386 002c 67       		.byte	103
 387 002d 2F       		.byte	47
 388 002e 6B       		.byte	107
 389 002f 72       		.byte	114
 390 0030 75       		.byte	117
 391 0031 75       		.byte	117
 392 0032 73       		.byte	115
 393 0033 2F       		.byte	47
 394 0034 76       		.byte	118
 395 0035 74       		.byte	116
 396 0036 2F       		.byte	47
 397 0037 73       		.byte	115
 398 0038 72       		.byte	114
 399 0039 63       		.byte	99
 400 003a 2F       		.byte	47
 401 003b 61       		.byte	97
 402 003c 73       		.byte	115
 403 003d 6D       		.byte	109
 404 003e 2D       		.byte	45
 405 003f 65       		.byte	101
 406 0040 78       		.byte	120
 407 0041 61       		.byte	97
 408 0042 6D       		.byte	109
 409 0043 70       		.byte	112
 410 0044 6C       		.byte	108
 411 0045 65       		.byte	101
 412 0046 73       		.byte	115
 413 0047 2F       		.byte	47
 414 0048 6C       		.byte	108
 415 0049 64       		.byte	100
 416 004a 71       		.byte	113
 417 004b 5F       		.byte	95
 418 004c 66       		.byte	102
 419 004d 6E       		.byte	110
 420 004e 5F       		.byte	95
 421 004f 6C       		.byte	108
 422 0050 37       		.byte	55
 423 0051 6C       		.byte	108
 424 0052 64       		.byte	100
 425 0053 71       		.byte	113
 426 0054 2E       		.byte	46
 427 0055 63       		.byte	99
 428 0056 00       		.zero	1
 429              		.text
 430              		.balign 16
 431              	.L_4.0:
 432              	# line 20
  19:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ldq_fn_l7ldq.c **** }
  20:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ldq_fn_l7ldq.c **** long double complex prt_ldq_fn_l7ldq(long a,long b,long c,long d,long e,long f,long g,long double c
 433              		.loc	1 20 0
 434              		.globl	prt_ldq_fn_l7ldq
 436              	prt_ldq_fn_l7ldq:
 437              		.cfi_startproc
 438 0780 00000000 		st	%fp,0x0(,%sp)
 438      8B000911 
 439              		.cfi_def_cfa_offset	0
 440              		.cfi_offset	9,0
 441 0788 08000000 		st	%lr,0x8(,%sp)
 441      8B000A11 
 442 0790 18000000 		st	%got,0x18(,%sp)
 442      8B000F11 
 443 0798 20000000 		st	%plt,0x20(,%sp)
 443      8B001011 
 444 07a0 00000000 		lea	%got,_GLOBAL_OFFSET_TABLE_@PC_LO(-0x18)
 444      00680F06 
 445 07a8 00000000 		and	%got,%got,(32)0
 445      608F0F44 
 446 07b0 00000000 		sic	%plt
 446      00001028 
 447 07b8 00000000 		lea.sl	%got,_GLOBAL_OFFSET_TABLE_@PC_HI(%plt,%got)
 447      8F908F06 
 448 07c0 00000000 		or	%fp,0,%sp
 448      8B000945 
 449              		.cfi_def_cfa_register	9
 450 07c8 30000000 		st	%s18,48(,%fp)
 450      89001211 
 451 07d0 38000000 		st	%s19,56(,%fp)
 451      89001311 
 452 07d8 40000000 		st	%s20,64(,%fp)
 452      89001411 
 453 07e0 48000000 		st	%s21,72(,%fp)
 453      89001511 
 454 07e8 50000000 		st	%s22,80(,%fp)
 454      89001611 
 455 07f0 58000000 		st	%s23,88(,%fp)
 455      89001711 
 456 07f8 60000000 		st	%s24,96(,%fp)
 456      89001811 
 457 0800 68000000 		st	%s25,104(,%fp)
 457      89001911 
 458 0808 70000000 		st	%s26,112(,%fp)
 458      89001A11 
 459 0810 78000000 		st	%s27,120(,%fp)
 459      89001B11 
 460 0818 80000000 		st	%s28,128(,%fp)
 460      89001C11 
 461 0820 88000000 		st	%s29,136(,%fp)
 461      89001D11 
 462 0828 90000000 		st	%s30,144(,%fp)
 462      89001E11 
 463 0830 98000000 		st	%s31,152(,%fp)
 463      89001F11 
 464 0838 A0000000 		st	%s32,160(,%fp)
 464      89002011 
 465 0840 A8000000 		st	%s33,168(,%fp)
 465      89002111 
 466 0848 D0FDFFFF 		lea	%s13,.L.4.2auto_size&0xffffffff
 466      00000D06 
 467 0850 00000000 		and	%s13,%s13,(32)0
 467      608D0D44 
 468 0858 FFFFFFFF 		lea.sl	%sp,.L.4.2auto_size>>32(%fp,%s13)
 468      8D898B06 
 469 0860 48000000 		brge.l.t	%sp,%sl,.L_4.EoP
 469      888B3518 
 470 0868 18000000 		ld	%s61,0x18(,%tp)
 470      8E003D01 
 471 0870 00000000 		or	%s62,0,%s0
 471      80003E45 
 472 0878 3B010000 		lea	%s63,0x13b
 472      00003F06 
 473 0880 00000000 		shm.l	%s63,0x0(%s61)
 473      BD033F31 
 474 0888 08000000 		shm.l	%sl,0x8(%s61)
 474      BD030831 
 475 0890 10000000 		shm.l	%sp,0x10(%s61)
 475      BD030B31 
 476 0898 00000000 		monc
 476      0000003F 
 477 08a0 00000000 		or	%s0,0,%s62
 477      BE000045 
 478              	.L_4.EoP:
 479              	# End of prologue codes
 480              	# line 21
  21:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ldq_fn_l7ldq.c **** 	long double complex ret = ldq + 1.49;
 481              		.loc	1 21 0
 482 08a8 F0000000 		ld	%s35,240(0,%fp)	# ldq (real)
 482      89002301 
 483 08b0 F0000000 		ld	%s34,240(8,%fp)	# ldq (real)
 483      89082201 
 484 08b8 00010000 		ld	%s37,256(0,%fp)	# ldq (imaginary)
 484      89002501 
 485 08c0 00010000 		ld	%s36,256(8,%fp)	# ldq (imaginary)
 485      89082401 
 486 08c8 3D0AD7A3 		lea	%s63,-1546188227
 486      00003F06 
 487 08d0 00000000 		and	%s62,%s63,(32)0
 487      60BF3E44 
 488 08d8 707DFF3F 		lea.sl	%s61,1073708400(,%s62)
 488      BE00BD06 
 489 08e0 00000070 		lea.sl	%s60,1879048192
 489      0000BC06 
 490 08e8 00000000 		or	%s38,0,%s61
 490      BD002645 
 491 08f0 00000000 		or	%s39,0,%s60
 491      BC002745 
 492 08f8 00000000 		fadd.q	%s40,%s34,%s38
 492      A6A2286C 
 493 0900 E0FFFFFF 		st	%s41,-32(0,%fp)	# ret (real)
 493      89002911 
 494 0908 E0FFFFFF 		st	%s40,-32(8,%fp)	# ret (real)
 494      89082811 
 495 0910 F0FFFFFF 		st	%s37,-16(0,%fp)	# ret (imaginary)
 495      89002511 
 496 0918 F0FFFFFF 		st	%s36,-16(8,%fp)	# ret (imaginary)
 496      89082411 
 497              	# line 22
  22:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ldq_fn_l7ldq.c **** 	printf("%s:%u %f+%fi\n",__FILE__,__LINE__,(double)creall(ret),(double)cimagl(ret));
 498              		.loc	1 22 0
 499 0920 00000000 		lea	%s59,.LP.__string.0@GOTOFF_LO
 499      00003B06 
 500 0928 00000000 		and	%s59,%s59,(32)0
 500      60BB3B44 
 501 0930 00000000 		lea.sl	%s59,.LP.__string.0@GOTOFF_HI(%s59,%got)
 501      8FBBBB06 
 502 0938 00000000 		lea	%s58,.LP.__string.1@GOTOFF_LO
 502      00003A06 
 503 0940 00000000 		and	%s58,%s58,(32)0
 503      60BA3A44 
 504 0948 00000000 		lea.sl	%s58,.LP.__string.1@GOTOFF_HI(%s58,%got)
 504      8FBABA06 
 505 0950 00000000 		or	%s57,0,%s40
 505      A8003945 
 506 0958 00000000 		or	%s56,0,%s41
 506      A9003845 
 507 0960 00000000 		or	%s55,0,%s36
 507      A4003745 
 508 0968 00000000 		or	%s54,0,%s37
 508      A5003645 
 509 0970 00000000 		or	%s0,0,%s57
 509      B9000045 
 510 0978 00000000 		or	%s1,0,%s56
 510      B8000145 
 511 0980 00000000 		or	%s2,0,%s55
 511      B7000245 
 512 0988 00000000 		or	%s3,0,%s54
 512      B6000345 
 513 0990 D8FFFFFF 		st	%s59,-40(,%fp)	# spill
 513      89003B11 
 514 0998 D0FFFFFF 		st	%s58,-48(,%fp)	# spill
 514      89003A11 
 515 09a0 00000000 		lea	%s12,creall@PLT_LO(-24)
 515      00680C06 
 516 09a8 00000000 		and	%s12,%s12,(32)0
 516      608C0C44 
 517 09b0 00000000 		sic	%lr
 517      00000A28 
 518 09b8 00000000 		lea.sl	%s12,creall@PLT_HI(%s12,%lr)
 518      8A8C8C06 
 519 09c0 00000000 		bsic	%lr,(,%s12)		# creall
 519      8C000A08 
 520              	.L_4.3:
 521 09c8 00000000 		cvt.d.q %s63,%s0
 521      0080BF0F 
 522 09d0 E0FFFFFF 		ld	%s35,-32(0,%fp)	# ret (real)
 522      89002301 
 523 09d8 E0FFFFFF 		ld	%s34,-32(8,%fp)	# ret (real)
 523      89082201 
 524 09e0 F0FFFFFF 		ld	%s37,-16(0,%fp)	# ret (imaginary)
 524      89002501 
 525 09e8 F0FFFFFF 		ld	%s36,-16(8,%fp)	# ret (imaginary)
 525      89082401 
 526 09f0 00000000 		or	%s62,0,%s34
 526      A2003E45 
 527 09f8 00000000 		or	%s61,0,%s35
 527      A3003D45 
 528 0a00 00000000 		or	%s60,0,%s36
 528      A4003C45 
 529 0a08 00000000 		or	%s59,0,%s37
 529      A5003B45 
 530 0a10 00000000 		or	%s0,0,%s62
 530      BE000045 
 531 0a18 00000000 		or	%s1,0,%s61
 531      BD000145 
 532 0a20 00000000 		or	%s2,0,%s60
 532      BC000245 
 533 0a28 00000000 		or	%s3,0,%s59
 533      BB000345 
 534 0a30 C8FFFFFF 		st	%s63,-56(,%fp)	# spill
 534      89003F11 
 535 0a38 00000000 		lea	%s12,cimagl@PLT_LO(-24)
 535      00680C06 
 536 0a40 00000000 		and	%s12,%s12,(32)0
 536      608C0C44 
 537 0a48 00000000 		sic	%lr
 537      00000A28 
 538 0a50 00000000 		lea.sl	%s12,cimagl@PLT_HI(%s12,%lr)
 538      8A8C8C06 
 539 0a58 00000000 		bsic	%lr,(,%s12)		# cimagl
 539      8C000A08 
 540              	.L_4.2:
 541 0a60 00000000 		cvt.d.q %s63,%s0
 541      0080BF0F 
 542 0a68 D0000000 		st	%s63,208(0,%sp)
 542      8B003F11 
 543 0a70 D8FFFFFF 		ld	%s62,-40(,%fp)	# restore
 543      89003E01 
 544 0a78 B0000000 		st	%s62,176(0,%sp)
 544      8B003E11 
 545 0a80 D0FFFFFF 		ld	%s61,-48(,%fp)	# restore
 545      89003D01 
 546 0a88 B8000000 		st	%s61,184(0,%sp)
 546      8B003D11 
 547 0a90 00000000 		or	%s60,22,(0)1
 547      00163C45 
 548 0a98 C0000000 		st	%s60,192(0,%sp)
 548      8B003C11 
 549 0aa0 C8FFFFFF 		ld	%s59,-56(,%fp)	# restore
 549      89003B01 
 550 0aa8 C8000000 		st	%s59,200(0,%sp)
 550      8B003B11 
 551 0ab0 00000000 		or	%s0,0,%s62
 551      BE000045 
 552 0ab8 00000000 		or	%s1,0,%s61
 552      BD000145 
 553 0ac0 00000000 		or	%s2,0,%s60
 553      BC000245 
 554 0ac8 00000000 		or	%s3,0,%s59
 554      BB000345 
 555 0ad0 00000000 		or	%s4,0,%s63
 555      BF000445 
 556 0ad8 00000000 		lea	%s12,printf@PLT_LO(-24)
 556      00680C06 
 557 0ae0 00000000 		and	%s12,%s12,(32)0
 557      608C0C44 
 558 0ae8 00000000 		sic	%lr
 558      00000A28 
 559 0af0 00000000 		lea.sl	%s12,printf@PLT_HI(%s12,%lr)
 559      8A8C8C06 
 560 0af8 00000000 		bsic	%lr,(,%s12)		# printf
 560      8C000A08 
 561 0b00 08000000 		br.l	.L_4.1
 561      00000F18 
 562              	.L_4.1:
 563              	# line 23
  23:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ldq_fn_l7ldq.c **** 	return ret;
 564              		.loc	1 23 0
 565 0b08 E0FFFFFF 		ld	%s35,-32(0,%fp)	# ret (real)
 565      89002301 
 566 0b10 E0FFFFFF 		ld	%s34,-32(8,%fp)	# ret (real)
 566      89082201 
 567 0b18 F0FFFFFF 		ld	%s37,-16(0,%fp)	# ret (imaginary)
 567      89002501 
 568 0b20 F0FFFFFF 		ld	%s36,-16(8,%fp)	# ret (imaginary)
 568      89082401 
 569 0b28 00000000 		or	%s63,0,%s34
 569      A2003F45 
 570 0b30 00000000 		or	%s62,0,%s35
 570      A3003E45 
 571 0b38 00000000 		or	%s61,0,%s36
 571      A4003D45 
 572 0b40 00000000 		or	%s60,0,%s37
 572      A5003C45 
 573 0b48 00000000 		or	%s0,0,%s63
 573      BF000045 
 574 0b50 00000000 		or	%s1,0,%s62
 574      BE000145 
 575 0b58 00000000 		or	%s2,0,%s61
 575      BD000245 
 576 0b60 00000000 		or	%s3,0,%s60
 576      BC000345 
 577              	# Start of epilogue codes
 578 0b68 00000000 		or	%sp,0,%fp
 578      89000B45 
 579              		.cfi_def_cfa	11,8
 580 0b70 18000000 		ld	%got,0x18(,%sp)
 580      8B000F01 
 581 0b78 20000000 		ld	%plt,0x20(,%sp)
 581      8B001001 
 582 0b80 08000000 		ld	%lr,0x8(,%sp)
 582      8B000A01 
 583 0b88 00000000 		ld	%fp,0x0(,%sp)
 583      8B000901 
 584 0b90 00000000 		b.l	(,%lr)
 584      8A000F19 
 585              	.L_4.EoE:
 586              		.cfi_endproc
 587              		.set	.L.4.2auto_size,	0xfffffffffffffdd0	# 560 Bytes
 589              	# ============ End  prt_ldq_fn_l7ldq ============
 590              	.Le1.0:
