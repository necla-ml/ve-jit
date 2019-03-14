   1              		.ident "ncc 1.1.4 (Build 10:17:22 Apr 19 2018)"
   2              		.file "ldq_fn_ldq.c"
   3              		.file 1 "/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ldq_fn_ldq.c"
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
  14              	# ============ Begin  ldq_fn_ldq ============
  15              		.section	.bss
  16              		.local	ldq_ret
  17              	.comm ldq_ret,32,16
  18              		.text
  19              		.balign 16
  20              	.L_1.0:
  21              	# line 7
   1:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ldq_fn_ldq.c **** #include <stdio.h>
   2:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ldq_fn_ldq.c **** #include <complex.h>
   3:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ldq_fn_ldq.c **** 
   4:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ldq_fn_ldq.c **** extern long double complex ext_ldq_fn_ldq(long double complex ldq);
   5:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ldq_fn_ldq.c **** 
   6:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ldq_fn_ldq.c **** static long double complex ldq_ret;
   7:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ldq_fn_ldq.c **** long double complex ldq_fn_ldq(long double complex ldq) {
  22              		.loc	1 7 0
  23              		.globl	ldq_fn_ldq
  25              	ldq_fn_ldq:
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
  55 00c8 F0FDFFFF 		lea	%s13,.L.1.2auto_size&0xffffffff
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
  69 0128 00000000 		or	%s34,0,%s2
  69      82002245 
  70 0130 00000000 		or	%s35,0,%s3
  70      83002345 
  71 0138 00000000 		or	%s36,0,%s0
  71      80002445 
  72 0140 00000000 		or	%s37,0,%s1
  72      81002545 
  73 0148 B0000000 		st	%s37,176(0,%fp)	# ldq (real)
  73      89002511 
  74 0150 B0000000 		st	%s36,176(8,%fp)	# ldq (real)
  74      89082411 
  75 0158 C0000000 		st	%s35,192(0,%fp)	# ldq (imaginary)
  75      89002311 
  76 0160 C0000000 		st	%s34,192(8,%fp)	# ldq (imaginary)
  76      89082211 
  77              	# line 8
   8:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ldq_fn_ldq.c **** 	ldq_ret = ext_ldq_fn_ldq(ldq);
  78              		.loc	1 8 0
  79 0168 00000000 		or	%s63,0,%s36
  79      A4003F45 
  80 0170 00000000 		or	%s62,0,%s37
  80      A5003E45 
  81 0178 00000000 		or	%s61,0,%s34
  81      A2003D45 
  82 0180 00000000 		or	%s60,0,%s35
  82      A3003C45 
  83 0188 00000000 		or	%s0,0,%s63
  83      BF000045 
  84 0190 00000000 		or	%s1,0,%s62
  84      BE000145 
  85 0198 00000000 		or	%s2,0,%s61
  85      BD000245 
  86 01a0 00000000 		or	%s3,0,%s60
  86      BC000345 
  87 01a8 00000000 		lea	%s12,ext_ldq_fn_ldq@PLT_LO(-24)
  87      00680C06 
  88 01b0 00000000 		and	%s12,%s12,(32)0
  88      608C0C44 
  89 01b8 00000000 		sic	%lr
  89      00000A28 
  90 01c0 00000000 		lea.sl	%s12,ext_ldq_fn_ldq@PLT_HI(%s12,%lr)
  90      8A8C8C06 
  91 01c8 00000000 		bsic	%lr,(,%s12)		# ext_ldq_fn_ldq
  91      8C000A08 
  92 01d0 F0FFFFFF 		st	%s0,-16(8,%fp)	# spill
  92      89080011 
  93 01d8 F0FFFFFF 		st	%s1,-16(,%fp)	# spill
  93      89000111 
  94 01e0 E0FFFFFF 		st	%s2,-32(8,%fp)	# spill
  94      89080211 
  95 01e8 E0FFFFFF 		st	%s3,-32(,%fp)	# spill
  95      89000311 
  96 01f0 08000000 		br.l	.L_1.1
  96      00000F18 
  97              	.L_1.1:
  98 01f8 00000000 		lea	%s63,ldq_ret@GOTOFF_LO
  98      00003F06 
  99 0200 00000000 		and	%s63,%s63,(32)0
  99      60BF3F44 
 100 0208 00000000 		lea.sl	%s63,ldq_ret@GOTOFF_HI(%s63,%got)
 100      8FBFBF06 
 101 0210 F0FFFFFF 		ld	%s34,-16(8,%fp)	# restore
 101      89082201 
 102 0218 F0FFFFFF 		ld	%s35,-16(,%fp)	# restore
 102      89002301 
 103 0220 00000000 		st	%s35,0(0,%s63)	# ldq_ret (real)
 103      BF002311 
 104 0228 00000000 		st	%s34,0(8,%s63)	# ldq_ret (real)
 104      BF082211 
 105 0230 E0FFFFFF 		ld	%s36,-32(8,%fp)	# restore
 105      89082401 
 106 0238 E0FFFFFF 		ld	%s37,-32(,%fp)	# restore
 106      89002501 
 107 0240 10000000 		st	%s37,16(0,%s63)	# ldq_ret (imaginary)
 107      BF002511 
 108 0248 10000000 		st	%s36,16(8,%s63)	# ldq_ret (imaginary)
 108      BF082411 
 109              	# line 9
   9:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ldq_fn_ldq.c **** 	return ldq_ret;
 110              		.loc	1 9 0
 111 0250 00000000 		or	%s62,0,%s34
 111      A2003E45 
 112 0258 00000000 		or	%s61,0,%s35
 112      A3003D45 
 113 0260 00000000 		or	%s60,0,%s36
 113      A4003C45 
 114 0268 00000000 		or	%s59,0,%s37
 114      A5003B45 
 115 0270 00000000 		or	%s0,0,%s62
 115      BE000045 
 116 0278 00000000 		or	%s1,0,%s61
 116      BD000145 
 117 0280 00000000 		or	%s2,0,%s60
 117      BC000245 
 118 0288 00000000 		or	%s3,0,%s59
 118      BB000345 
 119              	# Start of epilogue codes
 120 0290 00000000 		or	%sp,0,%fp
 120      89000B45 
 121              		.cfi_def_cfa	11,8
 122 0298 18000000 		ld	%got,0x18(,%sp)
 122      8B000F01 
 123 02a0 20000000 		ld	%plt,0x20(,%sp)
 123      8B001001 
 124 02a8 08000000 		ld	%lr,0x8(,%sp)
 124      8B000A01 
 125 02b0 00000000 		ld	%fp,0x0(,%sp)
 125      8B000901 
 126 02b8 00000000 		b.l	(,%lr)
 126      8A000F19 
 127              	.L_1.EoE:
 128              		.cfi_endproc
 129              		.set	.L.1.2auto_size,	0xfffffffffffffdf0	# 528 Bytes
 131              	# ============ End  ldq_fn_ldq ============
 132              	# ============ Begin  ldq_fn_ldq2 ============
 133              		.balign 16
 134              	.L_2.0:
 135              	# line 11
  10:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ldq_fn_ldq.c **** }
  11:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ldq_fn_ldq.c **** long double complex ldq_fn_ldq2(long double complex ldq) {
 136              		.loc	1 11 0
 137              		.globl	ldq_fn_ldq2
 139              	ldq_fn_ldq2:
 140              		.cfi_startproc
 141 02c0 00000000 		st	%fp,0x0(,%sp)
 141      8B000911 
 142              		.cfi_def_cfa_offset	0
 143              		.cfi_offset	9,0
 144 02c8 08000000 		st	%lr,0x8(,%sp)
 144      8B000A11 
 145 02d0 18000000 		st	%got,0x18(,%sp)
 145      8B000F11 
 146 02d8 20000000 		st	%plt,0x20(,%sp)
 146      8B001011 
 147 02e0 00000000 		lea	%got,_GLOBAL_OFFSET_TABLE_@PC_LO(-0x18)
 147      00680F06 
 148 02e8 00000000 		and	%got,%got,(32)0
 148      608F0F44 
 149 02f0 00000000 		sic	%plt
 149      00001028 
 150 02f8 00000000 		lea.sl	%got,_GLOBAL_OFFSET_TABLE_@PC_HI(%plt,%got)
 150      8F908F06 
 151 0300 00000000 		or	%fp,0,%sp
 151      8B000945 
 152              		.cfi_def_cfa_register	9
 153 0308 30000000 		st	%s18,48(,%fp)
 153      89001211 
 154 0310 38000000 		st	%s19,56(,%fp)
 154      89001311 
 155 0318 40000000 		st	%s20,64(,%fp)
 155      89001411 
 156 0320 48000000 		st	%s21,72(,%fp)
 156      89001511 
 157 0328 50000000 		st	%s22,80(,%fp)
 157      89001611 
 158 0330 58000000 		st	%s23,88(,%fp)
 158      89001711 
 159 0338 60000000 		st	%s24,96(,%fp)
 159      89001811 
 160 0340 68000000 		st	%s25,104(,%fp)
 160      89001911 
 161 0348 70000000 		st	%s26,112(,%fp)
 161      89001A11 
 162 0350 78000000 		st	%s27,120(,%fp)
 162      89001B11 
 163 0358 80000000 		st	%s28,128(,%fp)
 163      89001C11 
 164 0360 88000000 		st	%s29,136(,%fp)
 164      89001D11 
 165 0368 90000000 		st	%s30,144(,%fp)
 165      89001E11 
 166 0370 98000000 		st	%s31,152(,%fp)
 166      89001F11 
 167 0378 A0000000 		st	%s32,160(,%fp)
 167      89002011 
 168 0380 A8000000 		st	%s33,168(,%fp)
 168      89002111 
 169 0388 E0FDFFFF 		lea	%s13,.L.2.2auto_size&0xffffffff
 169      00000D06 
 170 0390 00000000 		and	%s13,%s13,(32)0
 170      608D0D44 
 171 0398 FFFFFFFF 		lea.sl	%sp,.L.2.2auto_size>>32(%fp,%s13)
 171      8D898B06 
 172 03a0 48000000 		brge.l.t	%sp,%sl,.L_2.EoP
 172      888B3518 
 173 03a8 18000000 		ld	%s61,0x18(,%tp)
 173      8E003D01 
 174 03b0 00000000 		or	%s62,0,%s0
 174      80003E45 
 175 03b8 3B010000 		lea	%s63,0x13b
 175      00003F06 
 176 03c0 00000000 		shm.l	%s63,0x0(%s61)
 176      BD033F31 
 177 03c8 08000000 		shm.l	%sl,0x8(%s61)
 177      BD030831 
 178 03d0 10000000 		shm.l	%sp,0x10(%s61)
 178      BD030B31 
 179 03d8 00000000 		monc
 179      0000003F 
 180 03e0 00000000 		or	%s0,0,%s62
 180      BE000045 
 181              	.L_2.EoP:
 182              	# End of prologue codes
 183 03e8 00000000 		or	%s34,0,%s2
 183      82002245 
 184 03f0 00000000 		or	%s35,0,%s3
 184      83002345 
 185 03f8 00000000 		or	%s36,0,%s0
 185      80002445 
 186 0400 00000000 		or	%s37,0,%s1
 186      81002545 
 187 0408 B0000000 		st	%s37,176(0,%fp)	# ldq (real)
 187      89002511 
 188 0410 B0000000 		st	%s36,176(8,%fp)	# ldq (real)
 188      89082411 
 189 0418 C0000000 		st	%s35,192(0,%fp)	# ldq (imaginary)
 189      89002311 
 190 0420 C0000000 		st	%s34,192(8,%fp)	# ldq (imaginary)
 190      89082211 
 191              	# line 12
  12:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ldq_fn_ldq.c **** 	long double complex *p_ret = &ldq_ret;
 192              		.loc	1 12 0
 193 0428 00000000 		lea	%s63,ldq_ret@GOTOFF_LO
 193      00003F06 
 194 0430 00000000 		and	%s63,%s63,(32)0
 194      60BF3F44 
 195 0438 00000000 		lea.sl	%s63,ldq_ret@GOTOFF_HI(%s63,%got)
 195      8FBFBF06 
 196 0440 00000000 		or	%s62,%s63,(0)1
 196      00BF3E45 
 197              	# line 13
  13:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ldq_fn_ldq.c **** 	*p_ret = ext_ldq_fn_ldq(ldq);
 198              		.loc	1 13 0
 199 0448 00000000 		or	%s61,0,%s36
 199      A4003D45 
 200 0450 00000000 		or	%s60,0,%s37
 200      A5003C45 
 201 0458 00000000 		or	%s59,0,%s34
 201      A2003B45 
 202 0460 00000000 		or	%s58,0,%s35
 202      A3003A45 
 203 0468 00000000 		or	%s0,0,%s61
 203      BD000045 
 204 0470 00000000 		or	%s1,0,%s60
 204      BC000145 
 205 0478 00000000 		or	%s2,0,%s59
 205      BB000245 
 206 0480 00000000 		or	%s3,0,%s58
 206      BA000345 
 207 0488 F8FFFFFF 		st	%s62,-8(,%fp)	# spill
 207      89003E11 
 208 0490 00000000 		lea	%s12,ext_ldq_fn_ldq@PLT_LO(-24)
 208      00680C06 
 209 0498 00000000 		and	%s12,%s12,(32)0
 209      608C0C44 
 210 04a0 00000000 		sic	%lr
 210      00000A28 
 211 04a8 00000000 		lea.sl	%s12,ext_ldq_fn_ldq@PLT_HI(%s12,%lr)
 211      8A8C8C06 
 212 04b0 00000000 		bsic	%lr,(,%s12)		# ext_ldq_fn_ldq
 212      8C000A08 
 213 04b8 E8FFFFFF 		st	%s0,-24(8,%fp)	# spill
 213      89080011 
 214 04c0 E8FFFFFF 		st	%s1,-24(,%fp)	# spill
 214      89000111 
 215 04c8 D8FFFFFF 		st	%s2,-40(8,%fp)	# spill
 215      89080211 
 216 04d0 D8FFFFFF 		st	%s3,-40(,%fp)	# spill
 216      89000311 
 217 04d8 08000000 		br.l	.L_2.1
 217      00000F18 
 218              	.L_2.1:
 219 04e0 F8FFFFFF 		ld	%s63,-8(,%fp)	# restore
 219      89003F01 
 220 04e8 E8FFFFFF 		ld	%s34,-24(8,%fp)	# restore
 220      89082201 
 221 04f0 E8FFFFFF 		ld	%s35,-24(,%fp)	# restore
 221      89002301 
 222 04f8 00000000 		st	%s35,0(0,%s63)	# *(p_ret) (real)
 222      BF002311 
 223 0500 00000000 		st	%s34,0(8,%s63)	# *(p_ret) (real)
 223      BF082211 
 224 0508 D8FFFFFF 		ld	%s36,-40(8,%fp)	# restore
 224      89082401 
 225 0510 D8FFFFFF 		ld	%s37,-40(,%fp)	# restore
 225      89002501 
 226 0518 10000000 		st	%s37,16(0,%s63)	# *(p_ret) (imaginary)
 226      BF002511 
 227 0520 10000000 		st	%s36,16(8,%s63)	# *(p_ret) (imaginary)
 227      BF082411 
 228              	# line 14
  14:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ldq_fn_ldq.c **** 	return *p_ret;
 229              		.loc	1 14 0
 230 0528 00000000 		or	%s62,0,%s34
 230      A2003E45 
 231 0530 00000000 		or	%s61,0,%s35
 231      A3003D45 
 232 0538 00000000 		or	%s60,0,%s36
 232      A4003C45 
 233 0540 00000000 		or	%s59,0,%s37
 233      A5003B45 
 234 0548 00000000 		or	%s0,0,%s62
 234      BE000045 
 235 0550 00000000 		or	%s1,0,%s61
 235      BD000145 
 236 0558 00000000 		or	%s2,0,%s60
 236      BC000245 
 237 0560 00000000 		or	%s3,0,%s59
 237      BB000345 
 238              	# Start of epilogue codes
 239 0568 00000000 		or	%sp,0,%fp
 239      89000B45 
 240              		.cfi_def_cfa	11,8
 241 0570 18000000 		ld	%got,0x18(,%sp)
 241      8B000F01 
 242 0578 20000000 		ld	%plt,0x20(,%sp)
 242      8B001001 
 243 0580 08000000 		ld	%lr,0x8(,%sp)
 243      8B000A01 
 244 0588 00000000 		ld	%fp,0x0(,%sp)
 244      8B000901 
 245 0590 00000000 		b.l	(,%lr)
 245      8A000F19 
 246              	.L_2.EoE:
 247              		.cfi_endproc
 248              		.set	.L.2.2auto_size,	0xfffffffffffffde0	# 544 Bytes
 250              	# ============ End  ldq_fn_ldq2 ============
 251              	# ============ Begin  mt_ldq_fn_ldq ============
 252 0598 00000000 		.balign 16
 252      00000000 
 253              	.L_3.0:
 254              	# line 16
  15:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ldq_fn_ldq.c **** }
  16:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ldq_fn_ldq.c **** long double complex mt_ldq_fn_ldq(long double complex ldq) {
 255              		.loc	1 16 0
 256              		.globl	mt_ldq_fn_ldq
 258              	mt_ldq_fn_ldq:
 259              		.cfi_startproc
 260 05a0 00000000 		st	%fp,0x0(,%sp)
 260      8B000911 
 261              		.cfi_def_cfa_offset	0
 262              		.cfi_offset	9,0
 263 05a8 08000000 		st	%lr,0x8(,%sp)
 263      8B000A11 
 264 05b0 18000000 		st	%got,0x18(,%sp)
 264      8B000F11 
 265 05b8 20000000 		st	%plt,0x20(,%sp)
 265      8B001011 
 266 05c0 00000000 		lea	%got,_GLOBAL_OFFSET_TABLE_@PC_LO(-0x18)
 266      00680F06 
 267 05c8 00000000 		and	%got,%got,(32)0
 267      608F0F44 
 268 05d0 00000000 		sic	%plt
 268      00001028 
 269 05d8 00000000 		lea.sl	%got,_GLOBAL_OFFSET_TABLE_@PC_HI(%plt,%got)
 269      8F908F06 
 270 05e0 00000000 		or	%fp,0,%sp
 270      8B000945 
 271              		.cfi_def_cfa_register	9
 272 05e8 30000000 		st	%s18,48(,%fp)
 272      89001211 
 273 05f0 38000000 		st	%s19,56(,%fp)
 273      89001311 
 274 05f8 40000000 		st	%s20,64(,%fp)
 274      89001411 
 275 0600 48000000 		st	%s21,72(,%fp)
 275      89001511 
 276 0608 50000000 		st	%s22,80(,%fp)
 276      89001611 
 277 0610 58000000 		st	%s23,88(,%fp)
 277      89001711 
 278 0618 60000000 		st	%s24,96(,%fp)
 278      89001811 
 279 0620 68000000 		st	%s25,104(,%fp)
 279      89001911 
 280 0628 70000000 		st	%s26,112(,%fp)
 280      89001A11 
 281 0630 78000000 		st	%s27,120(,%fp)
 281      89001B11 
 282 0638 80000000 		st	%s28,128(,%fp)
 282      89001C11 
 283 0640 88000000 		st	%s29,136(,%fp)
 283      89001D11 
 284 0648 90000000 		st	%s30,144(,%fp)
 284      89001E11 
 285 0650 98000000 		st	%s31,152(,%fp)
 285      89001F11 
 286 0658 A0000000 		st	%s32,160(,%fp)
 286      89002011 
 287 0660 A8000000 		st	%s33,168(,%fp)
 287      89002111 
 288 0668 E0FFFFFF 		lea	%s13,.L.3.2auto_size&0xffffffff
 288      00000D06 
 289 0670 00000000 		and	%s13,%s13,(32)0
 289      608D0D44 
 290 0678 FFFFFFFF 		lea.sl	%sp,.L.3.2auto_size>>32(%fp,%s13)
 290      8D898B06 
 291 0680 48000000 		brge.l.t	%sp,%sl,.L_3.EoP
 291      888B3518 
 292 0688 18000000 		ld	%s61,0x18(,%tp)
 292      8E003D01 
 293 0690 00000000 		or	%s62,0,%s0
 293      80003E45 
 294 0698 3B010000 		lea	%s63,0x13b
 294      00003F06 
 295 06a0 00000000 		shm.l	%s63,0x0(%s61)
 295      BD033F31 
 296 06a8 08000000 		shm.l	%sl,0x8(%s61)
 296      BD030831 
 297 06b0 10000000 		shm.l	%sp,0x10(%s61)
 297      BD030B31 
 298 06b8 00000000 		monc
 298      0000003F 
 299 06c0 00000000 		or	%s0,0,%s62
 299      BE000045 
 300              	.L_3.EoP:
 301              	# End of prologue codes
 302 06c8 00000000 		or	%s34,0,%s2
 302      82002245 
 303 06d0 00000000 		or	%s35,0,%s3
 303      83002345 
 304 06d8 00000000 		or	%s36,0,%s0
 304      80002445 
 305 06e0 00000000 		or	%s37,0,%s1
 305      81002545 
 306 06e8 B0000000 		st	%s37,176(0,%fp)	# ldq (real)
 306      89002511 
 307 06f0 B0000000 		st	%s36,176(8,%fp)	# ldq (real)
 307      89082411 
 308 06f8 C0000000 		st	%s35,192(0,%fp)	# ldq (imaginary)
 308      89002311 
 309 0700 C0000000 		st	%s34,192(8,%fp)	# ldq (imaginary)
 309      89082211 
 310              	# line 17
  17:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ldq_fn_ldq.c **** 	long double complex ret = ldq+1.49;
 311              		.loc	1 17 0
 312 0708 3D0AD7A3 		lea	%s63,-1546188227
 312      00003F06 
 313 0710 00000000 		and	%s62,%s63,(32)0
 313      60BF3E44 
 314 0718 707DFF3F 		lea.sl	%s61,1073708400(,%s62)
 314      BE00BD06 
 315 0720 00000070 		lea.sl	%s60,1879048192
 315      0000BC06 
 316 0728 00000000 		or	%s38,0,%s61
 316      BD002645 
 317 0730 00000000 		or	%s39,0,%s60
 317      BC002745 
 318 0738 00000000 		fadd.q	%s40,%s36,%s38
 318      A6A4286C 
 319 0740 E0FFFFFF 		st	%s41,-32(0,%fp)	# ret (real)
 319      89002911 
 320 0748 E0FFFFFF 		st	%s40,-32(8,%fp)	# ret (real)
 320      89082811 
 321 0750 F0FFFFFF 		st	%s35,-16(0,%fp)	# ret (imaginary)
 321      89002311 
 322 0758 F0FFFFFF 		st	%s34,-16(8,%fp)	# ret (imaginary)
 322      89082211 
 323              	# line 18
  18:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ldq_fn_ldq.c **** 	return ret;
 324              		.loc	1 18 0
 325 0760 00000000 		or	%s59,0,%s40
 325      A8003B45 
 326 0768 00000000 		or	%s58,0,%s41
 326      A9003A45 
 327 0770 00000000 		or	%s57,0,%s34
 327      A2003945 
 328 0778 00000000 		or	%s56,0,%s35
 328      A3003845 
 329 0780 00000000 		or	%s0,0,%s59
 329      BB000045 
 330 0788 00000000 		or	%s1,0,%s58
 330      BA000145 
 331 0790 00000000 		or	%s2,0,%s57
 331      B9000245 
 332 0798 00000000 		or	%s3,0,%s56
 332      B8000345 
 333              	# Start of epilogue codes
 334 07a0 00000000 		or	%sp,0,%fp
 334      89000B45 
 335              		.cfi_def_cfa	11,8
 336 07a8 18000000 		ld	%got,0x18(,%sp)
 336      8B000F01 
 337 07b0 20000000 		ld	%plt,0x20(,%sp)
 337      8B001001 
 338 07b8 08000000 		ld	%lr,0x8(,%sp)
 338      8B000A01 
 339 07c0 00000000 		ld	%fp,0x0(,%sp)
 339      8B000901 
 340 07c8 00000000 		b.l	(,%lr)
 340      8A000F19 
 341              	.L_3.EoE:
 342              		.cfi_endproc
 343              		.set	.L.3.2auto_size,	0xffffffffffffffe0	# 32 Bytes
 345              	# ============ End  mt_ldq_fn_ldq ============
 346              	# ============ Begin  prt_ldq_fn_ldq ============
 347              		.section .rodata
 348              		.balign 16
 350              	.LP.__string.0:
 351 0000 25       		.byte	37
 352 0001 73       		.byte	115
 353 0002 3A       		.byte	58
 354 0003 25       		.byte	37
 355 0004 75       		.byte	117
 356 0005 20       		.byte	32
 357 0006 25       		.byte	37
 358 0007 66       		.byte	102
 359 0008 2B       		.byte	43
 360 0009 25       		.byte	37
 361 000a 66       		.byte	102
 362 000b 69       		.byte	105
 363 000c 0A       		.byte	10
 364 000d 00       		.zero	1
 365 000e 0000     		.balign 8
 367              	.LP.__string.1:
 368 0010 2F       		.byte	47
 369 0011 75       		.byte	117
 370 0012 73       		.byte	115
 371 0013 72       		.byte	114
 372 0014 2F       		.byte	47
 373 0015 75       		.byte	117
 374 0016 68       		.byte	104
 375 0017 6F       		.byte	111
 376 0018 6D       		.byte	109
 377 0019 65       		.byte	101
 378 001a 2F       		.byte	47
 379 001b 61       		.byte	97
 380 001c 75       		.byte	117
 381 001d 72       		.byte	114
 382 001e 6F       		.byte	111
 383 001f 72       		.byte	114
 384 0020 61       		.byte	97
 385 0021 2F       		.byte	47
 386 0022 34       		.byte	52
 387 0023 67       		.byte	103
 388 0024 69       		.byte	105
 389 0025 2F       		.byte	47
 390 0026 6E       		.byte	110
 391 0027 6C       		.byte	108
 392 0028 61       		.byte	97
 393 0029 62       		.byte	98
 394 002a 68       		.byte	104
 395 002b 70       		.byte	112
 396 002c 67       		.byte	103
 397 002d 2F       		.byte	47
 398 002e 6B       		.byte	107
 399 002f 72       		.byte	114
 400 0030 75       		.byte	117
 401 0031 75       		.byte	117
 402 0032 73       		.byte	115
 403 0033 2F       		.byte	47
 404 0034 76       		.byte	118
 405 0035 74       		.byte	116
 406 0036 2F       		.byte	47
 407 0037 73       		.byte	115
 408 0038 72       		.byte	114
 409 0039 63       		.byte	99
 410 003a 2F       		.byte	47
 411 003b 61       		.byte	97
 412 003c 73       		.byte	115
 413 003d 6D       		.byte	109
 414 003e 2D       		.byte	45
 415 003f 65       		.byte	101
 416 0040 78       		.byte	120
 417 0041 61       		.byte	97
 418 0042 6D       		.byte	109
 419 0043 70       		.byte	112
 420 0044 6C       		.byte	108
 421 0045 65       		.byte	101
 422 0046 73       		.byte	115
 423 0047 2F       		.byte	47
 424 0048 6C       		.byte	108
 425 0049 64       		.byte	100
 426 004a 71       		.byte	113
 427 004b 5F       		.byte	95
 428 004c 66       		.byte	102
 429 004d 6E       		.byte	110
 430 004e 5F       		.byte	95
 431 004f 6C       		.byte	108
 432 0050 64       		.byte	100
 433 0051 71       		.byte	113
 434 0052 2E       		.byte	46
 435 0053 63       		.byte	99
 436 0054 00       		.zero	1
 437              		.text
 438              		.balign 16
 439              	.L_4.0:
 440              	# line 20
  19:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ldq_fn_ldq.c **** }
  20:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ldq_fn_ldq.c **** long double complex prt_ldq_fn_ldq(long double complex ldq) {
 441              		.loc	1 20 0
 442              		.globl	prt_ldq_fn_ldq
 444              	prt_ldq_fn_ldq:
 445              		.cfi_startproc
 446 07d0 00000000 		st	%fp,0x0(,%sp)
 446      8B000911 
 447              		.cfi_def_cfa_offset	0
 448              		.cfi_offset	9,0
 449 07d8 08000000 		st	%lr,0x8(,%sp)
 449      8B000A11 
 450 07e0 18000000 		st	%got,0x18(,%sp)
 450      8B000F11 
 451 07e8 20000000 		st	%plt,0x20(,%sp)
 451      8B001011 
 452 07f0 00000000 		lea	%got,_GLOBAL_OFFSET_TABLE_@PC_LO(-0x18)
 452      00680F06 
 453 07f8 00000000 		and	%got,%got,(32)0
 453      608F0F44 
 454 0800 00000000 		sic	%plt
 454      00001028 
 455 0808 00000000 		lea.sl	%got,_GLOBAL_OFFSET_TABLE_@PC_HI(%plt,%got)
 455      8F908F06 
 456 0810 00000000 		or	%fp,0,%sp
 456      8B000945 
 457              		.cfi_def_cfa_register	9
 458 0818 30000000 		st	%s18,48(,%fp)
 458      89001211 
 459 0820 38000000 		st	%s19,56(,%fp)
 459      89001311 
 460 0828 40000000 		st	%s20,64(,%fp)
 460      89001411 
 461 0830 48000000 		st	%s21,72(,%fp)
 461      89001511 
 462 0838 50000000 		st	%s22,80(,%fp)
 462      89001611 
 463 0840 58000000 		st	%s23,88(,%fp)
 463      89001711 
 464 0848 60000000 		st	%s24,96(,%fp)
 464      89001811 
 465 0850 68000000 		st	%s25,104(,%fp)
 465      89001911 
 466 0858 70000000 		st	%s26,112(,%fp)
 466      89001A11 
 467 0860 78000000 		st	%s27,120(,%fp)
 467      89001B11 
 468 0868 80000000 		st	%s28,128(,%fp)
 468      89001C11 
 469 0870 88000000 		st	%s29,136(,%fp)
 469      89001D11 
 470 0878 90000000 		st	%s30,144(,%fp)
 470      89001E11 
 471 0880 98000000 		st	%s31,152(,%fp)
 471      89001F11 
 472 0888 A0000000 		st	%s32,160(,%fp)
 472      89002011 
 473 0890 A8000000 		st	%s33,168(,%fp)
 473      89002111 
 474 0898 D0FDFFFF 		lea	%s13,.L.4.2auto_size&0xffffffff
 474      00000D06 
 475 08a0 00000000 		and	%s13,%s13,(32)0
 475      608D0D44 
 476 08a8 FFFFFFFF 		lea.sl	%sp,.L.4.2auto_size>>32(%fp,%s13)
 476      8D898B06 
 477 08b0 48000000 		brge.l.t	%sp,%sl,.L_4.EoP
 477      888B3518 
 478 08b8 18000000 		ld	%s61,0x18(,%tp)
 478      8E003D01 
 479 08c0 00000000 		or	%s62,0,%s0
 479      80003E45 
 480 08c8 3B010000 		lea	%s63,0x13b
 480      00003F06 
 481 08d0 00000000 		shm.l	%s63,0x0(%s61)
 481      BD033F31 
 482 08d8 08000000 		shm.l	%sl,0x8(%s61)
 482      BD030831 
 483 08e0 10000000 		shm.l	%sp,0x10(%s61)
 483      BD030B31 
 484 08e8 00000000 		monc
 484      0000003F 
 485 08f0 00000000 		or	%s0,0,%s62
 485      BE000045 
 486              	.L_4.EoP:
 487              	# End of prologue codes
 488 08f8 00000000 		or	%s34,0,%s2
 488      82002245 
 489 0900 00000000 		or	%s35,0,%s3
 489      83002345 
 490 0908 00000000 		or	%s36,0,%s0
 490      80002445 
 491 0910 00000000 		or	%s37,0,%s1
 491      81002545 
 492 0918 B0000000 		st	%s37,176(0,%fp)	# ldq (real)
 492      89002511 
 493 0920 B0000000 		st	%s36,176(8,%fp)	# ldq (real)
 493      89082411 
 494 0928 C0000000 		st	%s35,192(0,%fp)	# ldq (imaginary)
 494      89002311 
 495 0930 C0000000 		st	%s34,192(8,%fp)	# ldq (imaginary)
 495      89082211 
 496              	# line 21
  21:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ldq_fn_ldq.c **** 	long double complex ret = ldq + 1.49;
 497              		.loc	1 21 0
 498 0938 3D0AD7A3 		lea	%s63,-1546188227
 498      00003F06 
 499 0940 00000000 		and	%s62,%s63,(32)0
 499      60BF3E44 
 500 0948 707DFF3F 		lea.sl	%s61,1073708400(,%s62)
 500      BE00BD06 
 501 0950 00000070 		lea.sl	%s60,1879048192
 501      0000BC06 
 502 0958 00000000 		or	%s38,0,%s61
 502      BD002645 
 503 0960 00000000 		or	%s39,0,%s60
 503      BC002745 
 504 0968 00000000 		fadd.q	%s40,%s36,%s38
 504      A6A4286C 
 505 0970 E0FFFFFF 		st	%s41,-32(0,%fp)	# ret (real)
 505      89002911 
 506 0978 E0FFFFFF 		st	%s40,-32(8,%fp)	# ret (real)
 506      89082811 
 507 0980 F0FFFFFF 		st	%s35,-16(0,%fp)	# ret (imaginary)
 507      89002311 
 508 0988 F0FFFFFF 		st	%s34,-16(8,%fp)	# ret (imaginary)
 508      89082211 
 509              	# line 22
  22:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ldq_fn_ldq.c **** 	printf("%s:%u %f+%fi\n",__FILE__,__LINE__,(double)creall(ret),(double)cimagl(ret));
 510              		.loc	1 22 0
 511 0990 00000000 		lea	%s59,.LP.__string.0@GOTOFF_LO
 511      00003B06 
 512 0998 00000000 		and	%s59,%s59,(32)0
 512      60BB3B44 
 513 09a0 00000000 		lea.sl	%s59,.LP.__string.0@GOTOFF_HI(%s59,%got)
 513      8FBBBB06 
 514 09a8 00000000 		lea	%s58,.LP.__string.1@GOTOFF_LO
 514      00003A06 
 515 09b0 00000000 		and	%s58,%s58,(32)0
 515      60BA3A44 
 516 09b8 00000000 		lea.sl	%s58,.LP.__string.1@GOTOFF_HI(%s58,%got)
 516      8FBABA06 
 517 09c0 00000000 		or	%s57,0,%s40
 517      A8003945 
 518 09c8 00000000 		or	%s56,0,%s41
 518      A9003845 
 519 09d0 00000000 		or	%s55,0,%s34
 519      A2003745 
 520 09d8 00000000 		or	%s54,0,%s35
 520      A3003645 
 521 09e0 00000000 		or	%s0,0,%s57
 521      B9000045 
 522 09e8 00000000 		or	%s1,0,%s56
 522      B8000145 
 523 09f0 00000000 		or	%s2,0,%s55
 523      B7000245 
 524 09f8 00000000 		or	%s3,0,%s54
 524      B6000345 
 525 0a00 D8FFFFFF 		st	%s59,-40(,%fp)	# spill
 525      89003B11 
 526 0a08 D0FFFFFF 		st	%s58,-48(,%fp)	# spill
 526      89003A11 
 527 0a10 00000000 		lea	%s12,creall@PLT_LO(-24)
 527      00680C06 
 528 0a18 00000000 		and	%s12,%s12,(32)0
 528      608C0C44 
 529 0a20 00000000 		sic	%lr
 529      00000A28 
 530 0a28 00000000 		lea.sl	%s12,creall@PLT_HI(%s12,%lr)
 530      8A8C8C06 
 531 0a30 00000000 		bsic	%lr,(,%s12)		# creall
 531      8C000A08 
 532              	.L_4.3:
 533 0a38 00000000 		cvt.d.q %s63,%s0
 533      0080BF0F 
 534 0a40 E0FFFFFF 		ld	%s35,-32(0,%fp)	# ret (real)
 534      89002301 
 535 0a48 E0FFFFFF 		ld	%s34,-32(8,%fp)	# ret (real)
 535      89082201 
 536 0a50 F0FFFFFF 		ld	%s37,-16(0,%fp)	# ret (imaginary)
 536      89002501 
 537 0a58 F0FFFFFF 		ld	%s36,-16(8,%fp)	# ret (imaginary)
 537      89082401 
 538 0a60 00000000 		or	%s62,0,%s34
 538      A2003E45 
 539 0a68 00000000 		or	%s61,0,%s35
 539      A3003D45 
 540 0a70 00000000 		or	%s60,0,%s36
 540      A4003C45 
 541 0a78 00000000 		or	%s59,0,%s37
 541      A5003B45 
 542 0a80 00000000 		or	%s0,0,%s62
 542      BE000045 
 543 0a88 00000000 		or	%s1,0,%s61
 543      BD000145 
 544 0a90 00000000 		or	%s2,0,%s60
 544      BC000245 
 545 0a98 00000000 		or	%s3,0,%s59
 545      BB000345 
 546 0aa0 C8FFFFFF 		st	%s63,-56(,%fp)	# spill
 546      89003F11 
 547 0aa8 00000000 		lea	%s12,cimagl@PLT_LO(-24)
 547      00680C06 
 548 0ab0 00000000 		and	%s12,%s12,(32)0
 548      608C0C44 
 549 0ab8 00000000 		sic	%lr
 549      00000A28 
 550 0ac0 00000000 		lea.sl	%s12,cimagl@PLT_HI(%s12,%lr)
 550      8A8C8C06 
 551 0ac8 00000000 		bsic	%lr,(,%s12)		# cimagl
 551      8C000A08 
 552              	.L_4.2:
 553 0ad0 00000000 		cvt.d.q %s63,%s0
 553      0080BF0F 
 554 0ad8 D0000000 		st	%s63,208(0,%sp)
 554      8B003F11 
 555 0ae0 D8FFFFFF 		ld	%s62,-40(,%fp)	# restore
 555      89003E01 
 556 0ae8 B0000000 		st	%s62,176(0,%sp)
 556      8B003E11 
 557 0af0 D0FFFFFF 		ld	%s61,-48(,%fp)	# restore
 557      89003D01 
 558 0af8 B8000000 		st	%s61,184(0,%sp)
 558      8B003D11 
 559 0b00 00000000 		or	%s60,22,(0)1
 559      00163C45 
 560 0b08 C0000000 		st	%s60,192(0,%sp)
 560      8B003C11 
 561 0b10 C8FFFFFF 		ld	%s59,-56(,%fp)	# restore
 561      89003B01 
 562 0b18 C8000000 		st	%s59,200(0,%sp)
 562      8B003B11 
 563 0b20 00000000 		or	%s0,0,%s62
 563      BE000045 
 564 0b28 00000000 		or	%s1,0,%s61
 564      BD000145 
 565 0b30 00000000 		or	%s2,0,%s60
 565      BC000245 
 566 0b38 00000000 		or	%s3,0,%s59
 566      BB000345 
 567 0b40 00000000 		or	%s4,0,%s63
 567      BF000445 
 568 0b48 00000000 		lea	%s12,printf@PLT_LO(-24)
 568      00680C06 
 569 0b50 00000000 		and	%s12,%s12,(32)0
 569      608C0C44 
 570 0b58 00000000 		sic	%lr
 570      00000A28 
 571 0b60 00000000 		lea.sl	%s12,printf@PLT_HI(%s12,%lr)
 571      8A8C8C06 
 572 0b68 00000000 		bsic	%lr,(,%s12)		# printf
 572      8C000A08 
 573 0b70 08000000 		br.l	.L_4.1
 573      00000F18 
 574              	.L_4.1:
 575              	# line 23
  23:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ldq_fn_ldq.c **** 	return ret;
 576              		.loc	1 23 0
 577 0b78 E0FFFFFF 		ld	%s35,-32(0,%fp)	# ret (real)
 577      89002301 
 578 0b80 E0FFFFFF 		ld	%s34,-32(8,%fp)	# ret (real)
 578      89082201 
 579 0b88 F0FFFFFF 		ld	%s37,-16(0,%fp)	# ret (imaginary)
 579      89002501 
 580 0b90 F0FFFFFF 		ld	%s36,-16(8,%fp)	# ret (imaginary)
 580      89082401 
 581 0b98 00000000 		or	%s63,0,%s34
 581      A2003F45 
 582 0ba0 00000000 		or	%s62,0,%s35
 582      A3003E45 
 583 0ba8 00000000 		or	%s61,0,%s36
 583      A4003D45 
 584 0bb0 00000000 		or	%s60,0,%s37
 584      A5003C45 
 585 0bb8 00000000 		or	%s0,0,%s63
 585      BF000045 
 586 0bc0 00000000 		or	%s1,0,%s62
 586      BE000145 
 587 0bc8 00000000 		or	%s2,0,%s61
 587      BD000245 
 588 0bd0 00000000 		or	%s3,0,%s60
 588      BC000345 
 589              	# Start of epilogue codes
 590 0bd8 00000000 		or	%sp,0,%fp
 590      89000B45 
 591              		.cfi_def_cfa	11,8
 592 0be0 18000000 		ld	%got,0x18(,%sp)
 592      8B000F01 
 593 0be8 20000000 		ld	%plt,0x20(,%sp)
 593      8B001001 
 594 0bf0 08000000 		ld	%lr,0x8(,%sp)
 594      8B000A01 
 595 0bf8 00000000 		ld	%fp,0x0(,%sp)
 595      8B000901 
 596 0c00 00000000 		b.l	(,%lr)
 596      8A000F19 
 597              	.L_4.EoE:
 598              		.cfi_endproc
 599              		.set	.L.4.2auto_size,	0xfffffffffffffdd0	# 560 Bytes
 601              	# ============ End  prt_ldq_fn_ldq ============
 602              	.Le1.0:
