   1              		.ident "ncc 1.1.4 (Build 10:17:22 Apr 19 2018)"
   2              		.file "ld_fn_fldx2.c"
   3              		.file 1 "/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ld_fn_fldx2.c"
   4              		.file 2 "/opt/nec/ve/ncc/1.1.4/include/stdc-predef.h"
   5              		.file 3 "/opt/nec/ve/ncc/1.1.4/include/stdio.h"
   6              		.file 4 "/opt/nec/ve/ncc/1.1.4/include/yvals.h"
   7              		.file 5 "/opt/nec/ve/ncc/1.1.4/include/necvals.h"
   8              		.file 6 "/opt/nec/ve/ncc/1.1.4/include/stdarg.h"
   9              		.file 7 "/opt/nec/ve/musl/include/stdio.h"
  10              		.file 8 "/opt/nec/ve/musl/include/features.h"
  11              		.file 9 "/opt/nec/ve/musl/include/bits/alltypes.h"
  12              	# ============ Begin  ld_fn_fldx2 ============
  13              		.section	.bss
  14              		.local	ldbl_ret
  15              	.comm ldbl_ret,16,16
  16              		.text
  17              		.balign 16
  18              	.L_1.0:
  19              	# line 6
   1:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ld_fn_fldx2.c **** #include <stdio.h>
   2:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ld_fn_fldx2.c **** 
   3:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ld_fn_fldx2.c **** extern long double ext_ld_fn_fldx2(float f, long double ld, float f2, long double ld2);
   4:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ld_fn_fldx2.c **** 
   5:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ld_fn_fldx2.c **** static long double ldbl_ret;
   6:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ld_fn_fldx2.c **** long double ld_fn_fldx2(float f, long double ld, float f2, long double ld2) {
  20              		.loc	1 6 0
  21              		.globl	ld_fn_fldx2
  23              	ld_fn_fldx2:
  24              		.cfi_startproc
  25 0000 00000000 		st	%fp,0x0(,%sp)
  25      8B000911 
  26              		.cfi_def_cfa_offset	0
  27              		.cfi_offset	9,0
  28 0008 08000000 		st	%lr,0x8(,%sp)
  28      8B000A11 
  29 0010 18000000 		st	%got,0x18(,%sp)
  29      8B000F11 
  30 0018 20000000 		st	%plt,0x20(,%sp)
  30      8B001011 
  31 0020 00000000 		lea	%got,_GLOBAL_OFFSET_TABLE_@PC_LO(-0x18)
  31      00680F06 
  32 0028 00000000 		and	%got,%got,(32)0
  32      608F0F44 
  33 0030 00000000 		sic	%plt
  33      00001028 
  34 0038 00000000 		lea.sl	%got,_GLOBAL_OFFSET_TABLE_@PC_HI(%plt,%got)
  34      8F908F06 
  35 0040 00000000 		or	%fp,0,%sp
  35      8B000945 
  36              		.cfi_def_cfa_register	9
  37 0048 30000000 		st	%s18,48(,%fp)
  37      89001211 
  38 0050 38000000 		st	%s19,56(,%fp)
  38      89001311 
  39 0058 40000000 		st	%s20,64(,%fp)
  39      89001411 
  40 0060 48000000 		st	%s21,72(,%fp)
  40      89001511 
  41 0068 50000000 		st	%s22,80(,%fp)
  41      89001611 
  42 0070 58000000 		st	%s23,88(,%fp)
  42      89001711 
  43 0078 60000000 		st	%s24,96(,%fp)
  43      89001811 
  44 0080 68000000 		st	%s25,104(,%fp)
  44      89001911 
  45 0088 70000000 		st	%s26,112(,%fp)
  45      89001A11 
  46 0090 78000000 		st	%s27,120(,%fp)
  46      89001B11 
  47 0098 80000000 		st	%s28,128(,%fp)
  47      89001C11 
  48 00a0 88000000 		st	%s29,136(,%fp)
  48      89001D11 
  49 00a8 90000000 		st	%s30,144(,%fp)
  49      89001E11 
  50 00b0 98000000 		st	%s31,152(,%fp)
  50      89001F11 
  51 00b8 A0000000 		st	%s32,160(,%fp)
  51      89002011 
  52 00c0 A8000000 		st	%s33,168(,%fp)
  52      89002111 
  53 00c8 D0FDFFFF 		lea	%s13,.L.1.2auto_size&0xffffffff
  53      00000D06 
  54 00d0 00000000 		and	%s13,%s13,(32)0
  54      608D0D44 
  55 00d8 FFFFFFFF 		lea.sl	%sp,.L.1.2auto_size>>32(%fp,%s13)
  55      8D898B06 
  56 00e0 48000000 		brge.l.t	%sp,%sl,.L_1.EoP
  56      888B3518 
  57 00e8 18000000 		ld	%s61,0x18(,%tp)
  57      8E003D01 
  58 00f0 00000000 		or	%s62,0,%s0
  58      80003E45 
  59 00f8 3B010000 		lea	%s63,0x13b
  59      00003F06 
  60 0100 00000000 		shm.l	%s63,0x0(%s61)
  60      BD033F31 
  61 0108 08000000 		shm.l	%sl,0x8(%s61)
  61      BD030831 
  62 0110 10000000 		shm.l	%sp,0x10(%s61)
  62      BD030B31 
  63 0118 00000000 		monc
  63      0000003F 
  64 0120 00000000 		or	%s0,0,%s62
  64      BE000045 
  65              	.L_1.EoP:
  66              	# End of prologue codes
  67 0128 00000000 		or	%s34,0,%s6
  67      86002245 
  68 0130 00000000 		or	%s35,0,%s7
  68      87002345 
  69              	# line 7
   7:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ld_fn_fldx2.c **** 	ldbl_ret = ext_ld_fn_fldx2(f+f, ld+ld, f2+f2, ld2+ld2);
  70              		.loc	1 7 0
  71 0138 00000000 		fadd.q	%s36,%s34,%s34
  71      A2A2246C 
  72 0140 00000000 		fadd.s	%s63,%s4,%s4
  72      8484BF4C 
  73              	# line 6
   6:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ld_fn_fldx2.c **** 	ldbl_ret = ext_ld_fn_fldx2(f+f, ld+ld, f2+f2, ld2+ld2);
  74              		.loc	1 6 0
  75 0148 00000000 		or	%s38,0,%s2
  75      82002645 
  76 0150 00000000 		or	%s39,0,%s3
  76      83002745 
  77              	# line 7
  78              		.loc	1 7 0
  79 0158 00000000 		fadd.q	%s40,%s38,%s38
  79      A6A6286C 
  80 0160 00000000 		fadd.s	%s62,%s0,%s0
  80      8080BE4C 
  81              	# line 6
   6:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ld_fn_fldx2.c **** 	ldbl_ret = ext_ld_fn_fldx2(f+f, ld+ld, f2+f2, ld2+ld2);
  82              		.loc	1 6 0
  83 0168 C0000000 		st	%s39,192(0,%fp)	# ld
  83      89002711 
  84 0170 C0000000 		st	%s38,192(8,%fp)	# ld
  84      89082611 
  85 0178 E0000000 		st	%s35,224(0,%fp)	# ld2
  85      89002311 
  86 0180 E0000000 		st	%s34,224(8,%fp)	# ld2
  86      89082211 
  87              	# line 7
  88              		.loc	1 7 0
  89 0188 00000000 		or	%s61,0,%s40
  89      A8003D45 
  90 0190 00000000 		or	%s60,0,%s41
  90      A9003C45 
  91 0198 00000000 		or	%s59,0,%s36
  91      A4003B45 
  92 01a0 00000000 		or	%s58,0,%s37
  92      A5003A45 
  93 01a8 00000000 		or	%s0,0,%s62
  93      BE000045 
  94 01b0 F8FFFFFF 		ld	%s1,-8(,%fp)	# restore
  94      89000101 
  95 01b8 00000000 		or	%s2,0,%s61
  95      BD000245 
  96 01c0 00000000 		or	%s3,0,%s60
  96      BC000345 
  97 01c8 00000000 		or	%s4,0,%s63
  97      BF000445 
  98 01d0 F0FFFFFF 		ld	%s5,-16(,%fp)	# restore
  98      89000501 
  99 01d8 00000000 		or	%s6,0,%s59
  99      BB000645 
 100 01e0 00000000 		or	%s7,0,%s58
 100      BA000745 
 101 01e8 00000000 		lea	%s12,ext_ld_fn_fldx2@PLT_LO(-24)
 101      00680C06 
 102 01f0 00000000 		and	%s12,%s12,(32)0
 102      608C0C44 
 103 01f8 00000000 		sic	%lr
 103      00000A28 
 104 0200 00000000 		lea.sl	%s12,ext_ld_fn_fldx2@PLT_HI(%s12,%lr)
 104      8A8C8C06 
 105 0208 00000000 		bsic	%lr,(,%s12)		# ext_ld_fn_fldx2
 105      8C000A08 
 106 0210 E0FFFFFF 		st	%s0,-32(8,%fp)	# spill
 106      89080011 
 107 0218 E0FFFFFF 		st	%s1,-32(,%fp)	# spill
 107      89000111 
 108 0220 08000000 		br.l	.L_1.1
 108      00000F18 
 109              	.L_1.1:
 110 0228 00000000 		lea	%s63,ldbl_ret@GOTOFF_LO
 110      00003F06 
 111 0230 00000000 		and	%s63,%s63,(32)0
 111      60BF3F44 
 112 0238 00000000 		lea.sl	%s63,ldbl_ret@GOTOFF_HI(%s63,%got)
 112      8FBFBF06 
 113 0240 E0FFFFFF 		ld	%s34,-32(8,%fp)	# restore
 113      89082201 
 114 0248 E0FFFFFF 		ld	%s35,-32(,%fp)	# restore
 114      89002301 
 115 0250 00000000 		st	%s35,0(0,%s63)	# ldbl_ret
 115      BF002311 
 116 0258 00000000 		st	%s34,0(8,%s63)	# ldbl_ret
 116      BF082211 
 117              	# line 8
   8:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ld_fn_fldx2.c **** 	return ldbl_ret;
 118              		.loc	1 8 0
 119 0260 00000000 		or	%s62,0,%s34
 119      A2003E45 
 120 0268 00000000 		or	%s61,0,%s35
 120      A3003D45 
 121 0270 00000000 		or	%s0,0,%s62
 121      BE000045 
 122 0278 00000000 		or	%s1,0,%s61
 122      BD000145 
 123              	# Start of epilogue codes
 124 0280 00000000 		or	%sp,0,%fp
 124      89000B45 
 125              		.cfi_def_cfa	11,8
 126 0288 18000000 		ld	%got,0x18(,%sp)
 126      8B000F01 
 127 0290 20000000 		ld	%plt,0x20(,%sp)
 127      8B001001 
 128 0298 08000000 		ld	%lr,0x8(,%sp)
 128      8B000A01 
 129 02a0 00000000 		ld	%fp,0x0(,%sp)
 129      8B000901 
 130 02a8 00000000 		b.l	(,%lr)
 130      8A000F19 
 131              	.L_1.EoE:
 132              		.cfi_endproc
 133              		.set	.L.1.2auto_size,	0xfffffffffffffdd0	# 560 Bytes
 135              	# ============ End  ld_fn_fldx2 ============
 136              	# ============ Begin  alt_ld_fn_fldx2 ============
 137              		.balign 16
 138              	.L_2.0:
 139              	# line 10
   9:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ld_fn_fldx2.c **** }
  10:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ld_fn_fldx2.c **** long double alt_ld_fn_fldx2(float f, long double ld, float f2, long double ld2) {
 140              		.loc	1 10 0
 141              		.globl	alt_ld_fn_fldx2
 143              	alt_ld_fn_fldx2:
 144              		.cfi_startproc
 145 02b0 00000000 		st	%fp,0x0(,%sp)
 145      8B000911 
 146              		.cfi_def_cfa_offset	0
 147              		.cfi_offset	9,0
 148 02b8 08000000 		st	%lr,0x8(,%sp)
 148      8B000A11 
 149 02c0 18000000 		st	%got,0x18(,%sp)
 149      8B000F11 
 150 02c8 20000000 		st	%plt,0x20(,%sp)
 150      8B001011 
 151 02d0 00000000 		lea	%got,_GLOBAL_OFFSET_TABLE_@PC_LO(-0x18)
 151      00680F06 
 152 02d8 00000000 		and	%got,%got,(32)0
 152      608F0F44 
 153 02e0 00000000 		sic	%plt
 153      00001028 
 154 02e8 00000000 		lea.sl	%got,_GLOBAL_OFFSET_TABLE_@PC_HI(%plt,%got)
 154      8F908F06 
 155 02f0 00000000 		or	%fp,0,%sp
 155      8B000945 
 156              		.cfi_def_cfa_register	9
 157 02f8 30000000 		st	%s18,48(,%fp)
 157      89001211 
 158 0300 38000000 		st	%s19,56(,%fp)
 158      89001311 
 159 0308 40000000 		st	%s20,64(,%fp)
 159      89001411 
 160 0310 48000000 		st	%s21,72(,%fp)
 160      89001511 
 161 0318 50000000 		st	%s22,80(,%fp)
 161      89001611 
 162 0320 58000000 		st	%s23,88(,%fp)
 162      89001711 
 163 0328 60000000 		st	%s24,96(,%fp)
 163      89001811 
 164 0330 68000000 		st	%s25,104(,%fp)
 164      89001911 
 165 0338 70000000 		st	%s26,112(,%fp)
 165      89001A11 
 166 0340 78000000 		st	%s27,120(,%fp)
 166      89001B11 
 167 0348 80000000 		st	%s28,128(,%fp)
 167      89001C11 
 168 0350 88000000 		st	%s29,136(,%fp)
 168      89001D11 
 169 0358 90000000 		st	%s30,144(,%fp)
 169      89001E11 
 170 0360 98000000 		st	%s31,152(,%fp)
 170      89001F11 
 171 0368 A0000000 		st	%s32,160(,%fp)
 171      89002011 
 172 0370 A8000000 		st	%s33,168(,%fp)
 172      89002111 
 173 0378 C0FDFFFF 		lea	%s13,.L.2.2auto_size&0xffffffff
 173      00000D06 
 174 0380 00000000 		and	%s13,%s13,(32)0
 174      608D0D44 
 175 0388 FFFFFFFF 		lea.sl	%sp,.L.2.2auto_size>>32(%fp,%s13)
 175      8D898B06 
 176 0390 48000000 		brge.l.t	%sp,%sl,.L_2.EoP
 176      888B3518 
 177 0398 18000000 		ld	%s61,0x18(,%tp)
 177      8E003D01 
 178 03a0 00000000 		or	%s62,0,%s0
 178      80003E45 
 179 03a8 3B010000 		lea	%s63,0x13b
 179      00003F06 
 180 03b0 00000000 		shm.l	%s63,0x0(%s61)
 180      BD033F31 
 181 03b8 08000000 		shm.l	%sl,0x8(%s61)
 181      BD030831 
 182 03c0 10000000 		shm.l	%sp,0x10(%s61)
 182      BD030B31 
 183 03c8 00000000 		monc
 183      0000003F 
 184 03d0 00000000 		or	%s0,0,%s62
 184      BE000045 
 185              	.L_2.EoP:
 186              	# End of prologue codes
 187 03d8 00000000 		or	%s34,0,%s6
 187      86002245 
 188 03e0 00000000 		or	%s35,0,%s7
 188      87002345 
 189              	# line 12
  11:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ld_fn_fldx2.c **** 	long double *p_ret = &ldbl_ret;
  12:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ld_fn_fldx2.c **** 	*p_ret = ext_ld_fn_fldx2(f+f, ld+ld, f2+f2, ld2+ld2);
 190              		.loc	1 12 0
 191 03e8 00000000 		fadd.q	%s36,%s34,%s34
 191      A2A2246C 
 192 03f0 00000000 		fadd.s	%s63,%s4,%s4
 192      8484BF4C 
 193              	# line 10
  10:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ld_fn_fldx2.c **** 	long double *p_ret = &ldbl_ret;
 194              		.loc	1 10 0
 195 03f8 00000000 		or	%s38,0,%s2
 195      82002645 
 196 0400 00000000 		or	%s39,0,%s3
 196      83002745 
 197              	# line 12
 198              		.loc	1 12 0
 199 0408 00000000 		fadd.q	%s40,%s38,%s38
 199      A6A6286C 
 200 0410 00000000 		fadd.s	%s62,%s0,%s0
 200      8080BE4C 
 201              	# line 10
  10:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ld_fn_fldx2.c **** 	long double *p_ret = &ldbl_ret;
 202              		.loc	1 10 0
 203 0418 C0000000 		st	%s39,192(0,%fp)	# ld
 203      89002711 
 204 0420 C0000000 		st	%s38,192(8,%fp)	# ld
 204      89082611 
 205 0428 E0000000 		st	%s35,224(0,%fp)	# ld2
 205      89002311 
 206 0430 E0000000 		st	%s34,224(8,%fp)	# ld2
 206      89082211 
 207              	# line 11
  11:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ld_fn_fldx2.c **** 	long double *p_ret = &ldbl_ret;
 208              		.loc	1 11 0
 209 0438 00000000 		lea	%s61,ldbl_ret@GOTOFF_LO
 209      00003D06 
 210 0440 00000000 		and	%s61,%s61,(32)0
 210      60BD3D44 
 211 0448 00000000 		lea.sl	%s61,ldbl_ret@GOTOFF_HI(%s61,%got)
 211      8FBDBD06 
 212              	# line 12
 213              		.loc	1 12 0
 214 0450 00000000 		or	%s60,0,%s40
 214      A8003C45 
 215 0458 00000000 		or	%s59,0,%s41
 215      A9003B45 
 216 0460 00000000 		or	%s58,0,%s36
 216      A4003A45 
 217 0468 00000000 		or	%s57,0,%s37
 217      A5003945 
 218 0470 00000000 		or	%s0,0,%s62
 218      BE000045 
 219 0478 F8FFFFFF 		ld	%s1,-8(,%fp)	# restore
 219      89000101 
 220 0480 00000000 		or	%s2,0,%s60
 220      BC000245 
 221 0488 00000000 		or	%s3,0,%s59
 221      BB000345 
 222 0490 00000000 		or	%s4,0,%s63
 222      BF000445 
 223 0498 F0FFFFFF 		ld	%s5,-16(,%fp)	# restore
 223      89000501 
 224 04a0 00000000 		or	%s6,0,%s58
 224      BA000645 
 225 04a8 00000000 		or	%s7,0,%s57
 225      B9000745 
 226 04b0 E8FFFFFF 		st	%s61,-24(,%fp)	# spill
 226      89003D11 
 227 04b8 00000000 		lea	%s12,ext_ld_fn_fldx2@PLT_LO(-24)
 227      00680C06 
 228 04c0 00000000 		and	%s12,%s12,(32)0
 228      608C0C44 
 229 04c8 00000000 		sic	%lr
 229      00000A28 
 230 04d0 00000000 		lea.sl	%s12,ext_ld_fn_fldx2@PLT_HI(%s12,%lr)
 230      8A8C8C06 
 231 04d8 00000000 		bsic	%lr,(,%s12)		# ext_ld_fn_fldx2
 231      8C000A08 
 232 04e0 D8FFFFFF 		st	%s0,-40(8,%fp)	# spill
 232      89080011 
 233 04e8 D8FFFFFF 		st	%s1,-40(,%fp)	# spill
 233      89000111 
 234 04f0 08000000 		br.l	.L_2.1
 234      00000F18 
 235              	.L_2.1:
 236 04f8 E8FFFFFF 		ld	%s63,-24(,%fp)	# restore
 236      89003F01 
 237 0500 D8FFFFFF 		ld	%s34,-40(8,%fp)	# restore
 237      89082201 
 238 0508 D8FFFFFF 		ld	%s35,-40(,%fp)	# restore
 238      89002301 
 239 0510 00000000 		st	%s35,0(0,%s63)	# *(p_ret)
 239      BF002311 
 240 0518 00000000 		st	%s34,0(8,%s63)	# *(p_ret)
 240      BF082211 
 241              	# line 13
  13:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ld_fn_fldx2.c **** 	return *p_ret;
 242              		.loc	1 13 0
 243 0520 00000000 		or	%s62,0,%s34
 243      A2003E45 
 244 0528 00000000 		or	%s61,0,%s35
 244      A3003D45 
 245 0530 00000000 		or	%s0,0,%s62
 245      BE000045 
 246 0538 00000000 		or	%s1,0,%s61
 246      BD000145 
 247              	# Start of epilogue codes
 248 0540 00000000 		or	%sp,0,%fp
 248      89000B45 
 249              		.cfi_def_cfa	11,8
 250 0548 18000000 		ld	%got,0x18(,%sp)
 250      8B000F01 
 251 0550 20000000 		ld	%plt,0x20(,%sp)
 251      8B001001 
 252 0558 08000000 		ld	%lr,0x8(,%sp)
 252      8B000A01 
 253 0560 00000000 		ld	%fp,0x0(,%sp)
 253      8B000901 
 254 0568 00000000 		b.l	(,%lr)
 254      8A000F19 
 255              	.L_2.EoE:
 256              		.cfi_endproc
 257              		.set	.L.2.2auto_size,	0xfffffffffffffdc0	# 576 Bytes
 259              	# ============ End  alt_ld_fn_fldx2 ============
 260              	# ============ Begin  mt_ld_fn_fldx2 ============
 261              		.balign 16
 262              	.L_3.0:
 263              	# line 15
  14:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ld_fn_fldx2.c **** }
  15:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ld_fn_fldx2.c **** long double mt_ld_fn_fldx2(float f, long double ld, float f2, long double ld2) {
 264              		.loc	1 15 0
 265              		.globl	mt_ld_fn_fldx2
 267              	mt_ld_fn_fldx2:
 268              		.cfi_startproc
 269 0570 00000000 		st	%fp,0x0(,%sp)
 269      8B000911 
 270              		.cfi_def_cfa_offset	0
 271              		.cfi_offset	9,0
 272 0578 08000000 		st	%lr,0x8(,%sp)
 272      8B000A11 
 273 0580 18000000 		st	%got,0x18(,%sp)
 273      8B000F11 
 274 0588 20000000 		st	%plt,0x20(,%sp)
 274      8B001011 
 275 0590 00000000 		lea	%got,_GLOBAL_OFFSET_TABLE_@PC_LO(-0x18)
 275      00680F06 
 276 0598 00000000 		and	%got,%got,(32)0
 276      608F0F44 
 277 05a0 00000000 		sic	%plt
 277      00001028 
 278 05a8 00000000 		lea.sl	%got,_GLOBAL_OFFSET_TABLE_@PC_HI(%plt,%got)
 278      8F908F06 
 279 05b0 00000000 		or	%fp,0,%sp
 279      8B000945 
 280              		.cfi_def_cfa_register	9
 281 05b8 30000000 		st	%s18,48(,%fp)
 281      89001211 
 282 05c0 38000000 		st	%s19,56(,%fp)
 282      89001311 
 283 05c8 40000000 		st	%s20,64(,%fp)
 283      89001411 
 284 05d0 48000000 		st	%s21,72(,%fp)
 284      89001511 
 285 05d8 50000000 		st	%s22,80(,%fp)
 285      89001611 
 286 05e0 58000000 		st	%s23,88(,%fp)
 286      89001711 
 287 05e8 60000000 		st	%s24,96(,%fp)
 287      89001811 
 288 05f0 68000000 		st	%s25,104(,%fp)
 288      89001911 
 289 05f8 70000000 		st	%s26,112(,%fp)
 289      89001A11 
 290 0600 78000000 		st	%s27,120(,%fp)
 290      89001B11 
 291 0608 80000000 		st	%s28,128(,%fp)
 291      89001C11 
 292 0610 88000000 		st	%s29,136(,%fp)
 292      89001D11 
 293 0618 90000000 		st	%s30,144(,%fp)
 293      89001E11 
 294 0620 98000000 		st	%s31,152(,%fp)
 294      89001F11 
 295 0628 A0000000 		st	%s32,160(,%fp)
 295      89002011 
 296 0630 A8000000 		st	%s33,168(,%fp)
 296      89002111 
 297 0638 F0FFFFFF 		lea	%s13,.L.3.2auto_size&0xffffffff
 297      00000D06 
 298 0640 00000000 		and	%s13,%s13,(32)0
 298      608D0D44 
 299 0648 FFFFFFFF 		lea.sl	%sp,.L.3.2auto_size>>32(%fp,%s13)
 299      8D898B06 
 300 0650 48000000 		brge.l.t	%sp,%sl,.L_3.EoP
 300      888B3518 
 301 0658 18000000 		ld	%s61,0x18(,%tp)
 301      8E003D01 
 302 0660 00000000 		or	%s62,0,%s0
 302      80003E45 
 303 0668 3B010000 		lea	%s63,0x13b
 303      00003F06 
 304 0670 00000000 		shm.l	%s63,0x0(%s61)
 304      BD033F31 
 305 0678 08000000 		shm.l	%sl,0x8(%s61)
 305      BD030831 
 306 0680 10000000 		shm.l	%sp,0x10(%s61)
 306      BD030B31 
 307 0688 00000000 		monc
 307      0000003F 
 308 0690 00000000 		or	%s0,0,%s62
 308      BE000045 
 309              	.L_3.EoP:
 310              	# End of prologue codes
 311 0698 00000000 		or	%s34,0,%s6
 311      86002245 
 312 06a0 00000000 		or	%s35,0,%s7
 312      87002345 
 313              	# line 16
  16:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ld_fn_fldx2.c **** 	long double ret = f + ld + f2 + ld2;
 314              		.loc	1 16 0
 315 06a8 00000000 		cvt.q.s %s36,%s4
 315      0084A42D 
 316              	# line 15
  15:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ld_fn_fldx2.c **** 	long double ret = f + ld + f2 + ld2;
 317              		.loc	1 15 0
 318 06b0 00000000 		or	%s38,0,%s2
 318      82002645 
 319 06b8 00000000 		or	%s39,0,%s3
 319      83002745 
 320              	# line 16
 321              		.loc	1 16 0
 322 06c0 00000000 		cvt.q.s %s40,%s0
 322      0080A82D 
 323 06c8 00000000 		fadd.q	%s42,%s38,%s40
 323      A8A62A6C 
 324 06d0 00000000 		fadd.q	%s44,%s36,%s42
 324      AAA42C6C 
 325 06d8 00000000 		fadd.q	%s46,%s34,%s44
 325      ACA22E6C 
 326              	# line 15
  15:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ld_fn_fldx2.c **** 	long double ret = f + ld + f2 + ld2;
 327              		.loc	1 15 0
 328 06e0 C0000000 		st	%s39,192(0,%fp)	# ld
 328      89002711 
 329 06e8 C0000000 		st	%s38,192(8,%fp)	# ld
 329      89082611 
 330 06f0 E0000000 		st	%s35,224(0,%fp)	# ld2
 330      89002311 
 331 06f8 E0000000 		st	%s34,224(8,%fp)	# ld2
 331      89082211 
 332              	# line 16
 333              		.loc	1 16 0
 334 0700 F0FFFFFF 		st	%s47,-16(0,%fp)	# ret
 334      89002F11 
 335 0708 F0FFFFFF 		st	%s46,-16(8,%fp)	# ret
 335      89082E11 
 336              	# line 17
  17:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ld_fn_fldx2.c **** 	return ret;
 337              		.loc	1 17 0
 338 0710 00000000 		or	%s63,0,%s46
 338      AE003F45 
 339 0718 00000000 		or	%s62,0,%s47
 339      AF003E45 
 340 0720 00000000 		or	%s0,0,%s63
 340      BF000045 
 341 0728 00000000 		or	%s1,0,%s62
 341      BE000145 
 342              	# Start of epilogue codes
 343 0730 00000000 		or	%sp,0,%fp
 343      89000B45 
 344              		.cfi_def_cfa	11,8
 345 0738 18000000 		ld	%got,0x18(,%sp)
 345      8B000F01 
 346 0740 20000000 		ld	%plt,0x20(,%sp)
 346      8B001001 
 347 0748 08000000 		ld	%lr,0x8(,%sp)
 347      8B000A01 
 348 0750 00000000 		ld	%fp,0x0(,%sp)
 348      8B000901 
 349 0758 00000000 		b.l	(,%lr)
 349      8A000F19 
 350              	.L_3.EoE:
 351              		.cfi_endproc
 352              		.set	.L.3.2auto_size,	0xfffffffffffffff0	# 16 Bytes
 354              	# ============ End  mt_ld_fn_fldx2 ============
 355              	# ============ Begin  prt_ld_fn_fldx2 ============
 356              		.section .rodata
 357              		.balign 16
 359              	.LP.__string.0:
 360 0000 25       		.byte	37
 361 0001 73       		.byte	115
 362 0002 3A       		.byte	58
 363 0003 25       		.byte	37
 364 0004 75       		.byte	117
 365 0005 20       		.byte	32
 366 0006 6C       		.byte	108
 367 0007 64       		.byte	100
 368 0008 20       		.byte	32
 369 0009 25       		.byte	37
 370 000a 66       		.byte	102
 371 000b 0A       		.byte	10
 372 000c 00       		.zero	1
 373 000d 000000   		.balign 8
 375              	.LP.__string.1:
 376 0010 2F       		.byte	47
 377 0011 75       		.byte	117
 378 0012 73       		.byte	115
 379 0013 72       		.byte	114
 380 0014 2F       		.byte	47
 381 0015 75       		.byte	117
 382 0016 68       		.byte	104
 383 0017 6F       		.byte	111
 384 0018 6D       		.byte	109
 385 0019 65       		.byte	101
 386 001a 2F       		.byte	47
 387 001b 61       		.byte	97
 388 001c 75       		.byte	117
 389 001d 72       		.byte	114
 390 001e 6F       		.byte	111
 391 001f 72       		.byte	114
 392 0020 61       		.byte	97
 393 0021 2F       		.byte	47
 394 0022 34       		.byte	52
 395 0023 67       		.byte	103
 396 0024 69       		.byte	105
 397 0025 2F       		.byte	47
 398 0026 6E       		.byte	110
 399 0027 6C       		.byte	108
 400 0028 61       		.byte	97
 401 0029 62       		.byte	98
 402 002a 68       		.byte	104
 403 002b 70       		.byte	112
 404 002c 67       		.byte	103
 405 002d 2F       		.byte	47
 406 002e 6B       		.byte	107
 407 002f 72       		.byte	114
 408 0030 75       		.byte	117
 409 0031 75       		.byte	117
 410 0032 73       		.byte	115
 411 0033 2F       		.byte	47
 412 0034 76       		.byte	118
 413 0035 74       		.byte	116
 414 0036 2F       		.byte	47
 415 0037 73       		.byte	115
 416 0038 72       		.byte	114
 417 0039 63       		.byte	99
 418 003a 2F       		.byte	47
 419 003b 61       		.byte	97
 420 003c 73       		.byte	115
 421 003d 6D       		.byte	109
 422 003e 2D       		.byte	45
 423 003f 65       		.byte	101
 424 0040 78       		.byte	120
 425 0041 61       		.byte	97
 426 0042 6D       		.byte	109
 427 0043 70       		.byte	112
 428 0044 6C       		.byte	108
 429 0045 65       		.byte	101
 430 0046 73       		.byte	115
 431 0047 2F       		.byte	47
 432 0048 6C       		.byte	108
 433 0049 64       		.byte	100
 434 004a 5F       		.byte	95
 435 004b 66       		.byte	102
 436 004c 6E       		.byte	110
 437 004d 5F       		.byte	95
 438 004e 66       		.byte	102
 439 004f 6C       		.byte	108
 440 0050 64       		.byte	100
 441 0051 78       		.byte	120
 442 0052 32       		.byte	50
 443 0053 2E       		.byte	46
 444 0054 63       		.byte	99
 445 0055 00       		.zero	1
 446              		.text
 447              		.balign 16
 448              	.L_4.0:
 449              	# line 19
  18:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ld_fn_fldx2.c **** }
  19:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ld_fn_fldx2.c **** long double prt_ld_fn_fldx2(float f, long double ld, float f2, long double ld2) {
 450              		.loc	1 19 0
 451              		.globl	prt_ld_fn_fldx2
 453              	prt_ld_fn_fldx2:
 454              		.cfi_startproc
 455 0760 00000000 		st	%fp,0x0(,%sp)
 455      8B000911 
 456              		.cfi_def_cfa_offset	0
 457              		.cfi_offset	9,0
 458 0768 08000000 		st	%lr,0x8(,%sp)
 458      8B000A11 
 459 0770 18000000 		st	%got,0x18(,%sp)
 459      8B000F11 
 460 0778 20000000 		st	%plt,0x20(,%sp)
 460      8B001011 
 461 0780 00000000 		lea	%got,_GLOBAL_OFFSET_TABLE_@PC_LO(-0x18)
 461      00680F06 
 462 0788 00000000 		and	%got,%got,(32)0
 462      608F0F44 
 463 0790 00000000 		sic	%plt
 463      00001028 
 464 0798 00000000 		lea.sl	%got,_GLOBAL_OFFSET_TABLE_@PC_HI(%plt,%got)
 464      8F908F06 
 465 07a0 00000000 		or	%fp,0,%sp
 465      8B000945 
 466              		.cfi_def_cfa_register	9
 467 07a8 30000000 		st	%s18,48(,%fp)
 467      89001211 
 468 07b0 38000000 		st	%s19,56(,%fp)
 468      89001311 
 469 07b8 40000000 		st	%s20,64(,%fp)
 469      89001411 
 470 07c0 48000000 		st	%s21,72(,%fp)
 470      89001511 
 471 07c8 50000000 		st	%s22,80(,%fp)
 471      89001611 
 472 07d0 58000000 		st	%s23,88(,%fp)
 472      89001711 
 473 07d8 60000000 		st	%s24,96(,%fp)
 473      89001811 
 474 07e0 68000000 		st	%s25,104(,%fp)
 474      89001911 
 475 07e8 70000000 		st	%s26,112(,%fp)
 475      89001A11 
 476 07f0 78000000 		st	%s27,120(,%fp)
 476      89001B11 
 477 07f8 80000000 		st	%s28,128(,%fp)
 477      89001C11 
 478 0800 88000000 		st	%s29,136(,%fp)
 478      89001D11 
 479 0808 90000000 		st	%s30,144(,%fp)
 479      89001E11 
 480 0810 98000000 		st	%s31,152(,%fp)
 480      89001F11 
 481 0818 A0000000 		st	%s32,160(,%fp)
 481      89002011 
 482 0820 A8000000 		st	%s33,168(,%fp)
 482      89002111 
 483 0828 20FEFFFF 		lea	%s13,.L.4.2auto_size&0xffffffff
 483      00000D06 
 484 0830 00000000 		and	%s13,%s13,(32)0
 484      608D0D44 
 485 0838 FFFFFFFF 		lea.sl	%sp,.L.4.2auto_size>>32(%fp,%s13)
 485      8D898B06 
 486 0840 48000000 		brge.l.t	%sp,%sl,.L_4.EoP
 486      888B3518 
 487 0848 18000000 		ld	%s61,0x18(,%tp)
 487      8E003D01 
 488 0850 00000000 		or	%s62,0,%s0
 488      80003E45 
 489 0858 3B010000 		lea	%s63,0x13b
 489      00003F06 
 490 0860 00000000 		shm.l	%s63,0x0(%s61)
 490      BD033F31 
 491 0868 08000000 		shm.l	%sl,0x8(%s61)
 491      BD030831 
 492 0870 10000000 		shm.l	%sp,0x10(%s61)
 492      BD030B31 
 493 0878 00000000 		monc
 493      0000003F 
 494 0880 00000000 		or	%s0,0,%s62
 494      BE000045 
 495              	.L_4.EoP:
 496              	# End of prologue codes
 497 0888 00000000 		or	%s34,0,%s6
 497      86002245 
 498 0890 00000000 		or	%s35,0,%s7
 498      87002345 
 499              	# line 20
  20:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ld_fn_fldx2.c **** 	long double ret = f + ld + f2 + ld2;
 500              		.loc	1 20 0
 501 0898 00000000 		cvt.q.s %s36,%s4
 501      0084A42D 
 502              	# line 19
  19:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ld_fn_fldx2.c **** 	long double ret = f + ld + f2 + ld2;
 503              		.loc	1 19 0
 504 08a0 00000000 		or	%s38,0,%s2
 504      82002645 
 505 08a8 00000000 		or	%s39,0,%s3
 505      83002745 
 506              	# line 20
 507              		.loc	1 20 0
 508 08b0 00000000 		cvt.q.s %s40,%s0
 508      0080A82D 
 509 08b8 00000000 		fadd.q	%s42,%s38,%s40
 509      A8A62A6C 
 510 08c0 00000000 		fadd.q	%s44,%s36,%s42
 510      AAA42C6C 
 511 08c8 00000000 		fadd.q	%s46,%s34,%s44
 511      ACA22E6C 
 512              	# line 21
  21:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ld_fn_fldx2.c **** 	printf("%s:%u ld %f\n",__FILE__,__LINE__,(double)ret);
 513              		.loc	1 21 0
 514 08d0 00000000 		cvt.d.q %s63,%s46
 514      00AEBF0F 
 515 08d8 C8000000 		st	%s63,200(0,%sp)
 515      8B003F11 
 516              	# line 19
  19:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ld_fn_fldx2.c **** 	long double ret = f + ld + f2 + ld2;
 517              		.loc	1 19 0
 518 08e0 C0000000 		st	%s39,192(0,%fp)	# ld
 518      89002711 
 519 08e8 C0000000 		st	%s38,192(8,%fp)	# ld
 519      89082611 
 520 08f0 E0000000 		st	%s35,224(0,%fp)	# ld2
 520      89002311 
 521 08f8 E0000000 		st	%s34,224(8,%fp)	# ld2
 521      89082211 
 522              	# line 20
  20:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ld_fn_fldx2.c **** 	long double ret = f + ld + f2 + ld2;
 523              		.loc	1 20 0
 524 0900 F0FFFFFF 		st	%s47,-16(0,%fp)	# ret
 524      89002F11 
 525 0908 F0FFFFFF 		st	%s46,-16(8,%fp)	# ret
 525      89082E11 
 526              	# line 21
 527              		.loc	1 21 0
 528 0910 00000000 		lea	%s62,.LP.__string.0@GOTOFF_LO
 528      00003E06 
 529 0918 00000000 		and	%s62,%s62,(32)0
 529      60BE3E44 
 530 0920 00000000 		lea.sl	%s62,.LP.__string.0@GOTOFF_HI(%s62,%got)
 530      8FBEBE06 
 531 0928 B0000000 		st	%s62,176(0,%sp)
 531      8B003E11 
 532 0930 00000000 		lea	%s61,.LP.__string.1@GOTOFF_LO
 532      00003D06 
 533 0938 00000000 		and	%s61,%s61,(32)0
 533      60BD3D44 
 534 0940 00000000 		lea.sl	%s61,.LP.__string.1@GOTOFF_HI(%s61,%got)
 534      8FBDBD06 
 535 0948 B8000000 		st	%s61,184(0,%sp)
 535      8B003D11 
 536 0950 00000000 		or	%s60,21,(0)1
 536      00153C45 
 537 0958 C0000000 		st	%s60,192(0,%sp)
 537      8B003C11 
 538 0960 00000000 		or	%s0,0,%s62
 538      BE000045 
 539 0968 00000000 		or	%s1,0,%s61
 539      BD000145 
 540 0970 00000000 		or	%s2,0,%s60
 540      BC000245 
 541 0978 00000000 		or	%s3,0,%s63
 541      BF000345 
 542 0980 00000000 		lea	%s12,printf@PLT_LO(-24)
 542      00680C06 
 543 0988 00000000 		and	%s12,%s12,(32)0
 543      608C0C44 
 544 0990 00000000 		sic	%lr
 544      00000A28 
 545 0998 00000000 		lea.sl	%s12,printf@PLT_HI(%s12,%lr)
 545      8A8C8C06 
 546 09a0 00000000 		bsic	%lr,(,%s12)		# printf
 546      8C000A08 
 547 09a8 08000000 		br.l	.L_4.1
 547      00000F18 
 548              	.L_4.1:
 549              	# line 22
  22:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ld_fn_fldx2.c **** 	return ret;
 550              		.loc	1 22 0
 551 09b0 F0FFFFFF 		ld	%s35,-16(0,%fp)	# ret
 551      89002301 
 552 09b8 F0FFFFFF 		ld	%s34,-16(8,%fp)	# ret
 552      89082201 
 553 09c0 00000000 		or	%s63,0,%s34
 553      A2003F45 
 554 09c8 00000000 		or	%s62,0,%s35
 554      A3003E45 
 555 09d0 00000000 		or	%s0,0,%s63
 555      BF000045 
 556 09d8 00000000 		or	%s1,0,%s62
 556      BE000145 
 557              	# Start of epilogue codes
 558 09e0 00000000 		or	%sp,0,%fp
 558      89000B45 
 559              		.cfi_def_cfa	11,8
 560 09e8 18000000 		ld	%got,0x18(,%sp)
 560      8B000F01 
 561 09f0 20000000 		ld	%plt,0x20(,%sp)
 561      8B001001 
 562 09f8 08000000 		ld	%lr,0x8(,%sp)
 562      8B000A01 
 563 0a00 00000000 		ld	%fp,0x0(,%sp)
 563      8B000901 
 564 0a08 00000000 		b.l	(,%lr)
 564      8A000F19 
 565              	.L_4.EoE:
 566              		.cfi_endproc
 567              		.set	.L.4.2auto_size,	0xfffffffffffffe20	# 480 Bytes
 569              	# ============ End  prt_ld_fn_fldx2 ============
 570              	.Le1.0:
