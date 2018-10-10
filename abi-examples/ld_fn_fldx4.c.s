   1              		.ident "ncc 1.1.4 (Build 10:17:22 Apr 19 2018)"
   2              		.file "ld_fn_fldx4.c"
   3              		.file 1 "/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ld_fn_fldx4.c"
   4              		.file 2 "/opt/nec/ve/ncc/1.1.4/include/stdc-predef.h"
   5              		.file 3 "/opt/nec/ve/ncc/1.1.4/include/stdio.h"
   6              		.file 4 "/opt/nec/ve/ncc/1.1.4/include/yvals.h"
   7              		.file 5 "/opt/nec/ve/ncc/1.1.4/include/necvals.h"
   8              		.file 6 "/opt/nec/ve/ncc/1.1.4/include/stdarg.h"
   9              		.file 7 "/opt/nec/ve/musl/include/stdio.h"
  10              		.file 8 "/opt/nec/ve/musl/include/features.h"
  11              		.file 9 "/opt/nec/ve/musl/include/bits/alltypes.h"
  12              	# ============ Begin  ld_fn_fldx4 ============
  13              		.section	.bss
  14              		.local	ldbl_ret
  15              	.comm ldbl_ret,16,16
  16              		.text
  17              		.balign 16
  18              	.L_1.0:
  19              	# line 8
   1:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ld_fn_fldx4.c **** #include <stdio.h>
   2:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ld_fn_fldx4.c **** 
   3:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ld_fn_fldx4.c **** extern long double ext_ld_fn_fldx4(float f, long double ld, float f2, long double ld2,
   4:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ld_fn_fldx4.c **** 		float f3, long double ld3, float f4, long double ld4);
   5:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ld_fn_fldx4.c **** 
   6:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ld_fn_fldx4.c **** static long double ldbl_ret;
   7:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ld_fn_fldx4.c **** long double ld_fn_fldx4(float f, long double ld, float f2, long double ld2,
   8:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ld_fn_fldx4.c **** 		float f3, long double ld3, float f4, long double ld4) {
  20              		.loc	1 8 0
  21              		.globl	ld_fn_fldx4
  23              	ld_fn_fldx4:
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
  53 00c8 70FDFFFF 		lea	%s13,.L.1.2auto_size&0xffffffff
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
  69              	# line 9
   9:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ld_fn_fldx4.c **** 	ldbl_ret = ext_ld_fn_fldx4(f+f, ld+ld, f2+f2, ld2+ld2, f3+f3, ld3+ld3, f4+f4, ld4+ld4);
  70              		.loc	1 9 0
  71 0138 00000000 		fadd.q	%s36,%s34,%s34
  71      A2A2246C 
  72 0140 00000000 		fadd.s	%s63,%s4,%s4
  72      8484BF4C 
  73              	# line 8
   8:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ld_fn_fldx4.c **** 	ldbl_ret = ext_ld_fn_fldx4(f+f, ld+ld, f2+f2, ld2+ld2, f3+f3, ld3+ld3, f4+f4, ld4+ld4);
  74              		.loc	1 8 0
  75 0148 00000000 		or	%s38,0,%s2
  75      82002645 
  76 0150 00000000 		or	%s39,0,%s3
  76      83002745 
  77              	# line 9
  78              		.loc	1 9 0
  79 0158 00000000 		fadd.q	%s40,%s38,%s38
  79      A6A6286C 
  80 0160 00000000 		fadd.s	%s62,%s0,%s0
  80      8080BE4C 
  81              	# line 8
   8:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ld_fn_fldx4.c **** 	ldbl_ret = ext_ld_fn_fldx4(f+f, ld+ld, f2+f2, ld2+ld2, f3+f3, ld3+ld3, f4+f4, ld4+ld4);
  82              		.loc	1 8 0
  83 0168 C0000000 		st	%s39,192(0,%fp)	# ld
  83      89002711 
  84 0170 C0000000 		st	%s38,192(8,%fp)	# ld
  84      89082611 
  85 0178 E0000000 		st	%s35,224(0,%fp)	# ld2
  85      89002311 
  86 0180 E0000000 		st	%s34,224(8,%fp)	# ld2
  86      89082211 
  87              	# line 9
  88              		.loc	1 9 0
  89 0188 F4000000 		ldu	%s61,244(0,%fp)	# f3
  89      89003D02 
  90 0190 00000000 		fadd.s	%s60,%s61,%s61
  90      BDBDBC4C 
  91 0198 F0000000 		st	%s60,240(0,%sp)
  91      8B003C11 
  92 01a0 00010000 		ld	%s43,256(0,%fp)	# ld3
  92      89002B01 
  93 01a8 00010000 		ld	%s42,256(8,%fp)	# ld3
  93      89082A01 
  94 01b0 00000000 		fadd.q	%s44,%s42,%s42
  94      AAAA2C6C 
  95 01b8 14010000 		ldu	%s59,276(0,%fp)	# f4
  95      89003B02 
  96 01c0 00000000 		fadd.s	%s58,%s59,%s59
  96      BBBBBA4C 
  97 01c8 10010000 		st	%s58,272(0,%sp)
  97      8B003A11 
  98 01d0 20010000 		ld	%s47,288(0,%fp)	# ld4
  98      89002F01 
  99 01d8 20010000 		ld	%s46,288(8,%fp)	# ld4
  99      89082E01 
 100 01e0 00000000 		fadd.q	%s48,%s46,%s46
 100      AEAE306C 
 101 01e8 00000000 		or	%s57,0,%s40
 101      A8003945 
 102 01f0 00000000 		or	%s56,0,%s41
 102      A9003845 
 103 01f8 00000000 		or	%s55,0,%s36
 103      A4003745 
 104 0200 00000000 		or	%s54,0,%s37
 104      A5003645 
 105 0208 00000000 		or	%s53,0,%s44
 105      AC003545 
 106 0210 08010000 		st	%s53,264(0,%sp)
 106      8B003511 
 107 0218 00000000 		or	%s52,0,%s45
 107      AD003445 
 108 0220 00010000 		st	%s52,256(0,%sp)
 108      8B003411 
 109 0228 00000000 		or	%s51,0,%s48
 109      B0003345 
 110 0230 28010000 		st	%s51,296(0,%sp)
 110      8B003311 
 111 0238 00000000 		or	%s50,0,%s49
 111      B1003245 
 112 0240 20010000 		st	%s50,288(0,%sp)
 112      8B003211 
 113 0248 E8FFFFFF 		ld	%s18,-24(,%fp)	# restore
 113      89001201 
 114 0250 F8000000 		st	%s18,248(0,%sp)
 114      8B001211 
 115 0258 E0FFFFFF 		ld	%s19,-32(,%fp)	# restore
 115      89001301 
 116 0260 18010000 		st	%s19,280(0,%sp)
 116      8B001311 
 117 0268 00000000 		or	%s0,0,%s62
 117      BE000045 
 118 0270 F8FFFFFF 		ld	%s1,-8(,%fp)	# restore
 118      89000101 
 119 0278 00000000 		or	%s2,0,%s57
 119      B9000245 
 120 0280 00000000 		or	%s3,0,%s56
 120      B8000345 
 121 0288 00000000 		or	%s4,0,%s63
 121      BF000445 
 122 0290 F0FFFFFF 		ld	%s5,-16(,%fp)	# restore
 122      89000501 
 123 0298 00000000 		or	%s6,0,%s55
 123      B7000645 
 124 02a0 00000000 		or	%s7,0,%s54
 124      B6000745 
 125 02a8 00000000 		lea	%s12,ext_ld_fn_fldx4@PLT_LO(-24)
 125      00680C06 
 126 02b0 00000000 		and	%s12,%s12,(32)0
 126      608C0C44 
 127 02b8 00000000 		sic	%lr
 127      00000A28 
 128 02c0 00000000 		lea.sl	%s12,ext_ld_fn_fldx4@PLT_HI(%s12,%lr)
 128      8A8C8C06 
 129 02c8 00000000 		bsic	%lr,(,%s12)		# ext_ld_fn_fldx4
 129      8C000A08 
 130 02d0 D0FFFFFF 		st	%s0,-48(8,%fp)	# spill
 130      89080011 
 131 02d8 D0FFFFFF 		st	%s1,-48(,%fp)	# spill
 131      89000111 
 132 02e0 08000000 		br.l	.L_1.1
 132      00000F18 
 133              	.L_1.1:
 134 02e8 00000000 		lea	%s63,ldbl_ret@GOTOFF_LO
 134      00003F06 
 135 02f0 00000000 		and	%s63,%s63,(32)0
 135      60BF3F44 
 136 02f8 00000000 		lea.sl	%s63,ldbl_ret@GOTOFF_HI(%s63,%got)
 136      8FBFBF06 
 137 0300 D0FFFFFF 		ld	%s18,-48(8,%fp)	# restore
 137      89081201 
 138 0308 D0FFFFFF 		ld	%s19,-48(,%fp)	# restore
 138      89001301 
 139 0310 00000000 		st	%s19,0(0,%s63)	# ldbl_ret
 139      BF001311 
 140 0318 00000000 		st	%s18,0(8,%s63)	# ldbl_ret
 140      BF081211 
 141              	# line 10
  10:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ld_fn_fldx4.c **** 	return ldbl_ret;
 142              		.loc	1 10 0
 143 0320 00000000 		or	%s62,0,%s18
 143      92003E45 
 144 0328 00000000 		or	%s61,0,%s19
 144      93003D45 
 145 0330 00000000 		or	%s0,0,%s62
 145      BE000045 
 146 0338 00000000 		or	%s1,0,%s61
 146      BD000145 
 147              	# Start of epilogue codes
 148 0340 30000000 		ld	%s18,48(,%fp)
 148      89001201 
 149 0348 38000000 		ld	%s19,56(,%fp)
 149      89001301 
 150 0350 00000000 		or	%sp,0,%fp
 150      89000B45 
 151              		.cfi_def_cfa	11,8
 152 0358 18000000 		ld	%got,0x18(,%sp)
 152      8B000F01 
 153 0360 20000000 		ld	%plt,0x20(,%sp)
 153      8B001001 
 154 0368 08000000 		ld	%lr,0x8(,%sp)
 154      8B000A01 
 155 0370 00000000 		ld	%fp,0x0(,%sp)
 155      8B000901 
 156 0378 00000000 		b.l	(,%lr)
 156      8A000F19 
 157              	.L_1.EoE:
 158              		.cfi_endproc
 159              		.set	.L.1.2auto_size,	0xfffffffffffffd70	# 656 Bytes
 161              	# ============ End  ld_fn_fldx4 ============
 162              	# ============ Begin  alt_ld_fn_fldx4 ============
 163              		.balign 16
 164              	.L_2.0:
 165              	# line 13
  11:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ld_fn_fldx4.c **** }
  12:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ld_fn_fldx4.c **** long double alt_ld_fn_fldx4(float f, long double ld, float f2, long double ld2,
  13:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ld_fn_fldx4.c **** 		float f3, long double ld3, float f4, long double ld4) {
 166              		.loc	1 13 0
 167              		.globl	alt_ld_fn_fldx4
 169              	alt_ld_fn_fldx4:
 170              		.cfi_startproc
 171 0380 00000000 		st	%fp,0x0(,%sp)
 171      8B000911 
 172              		.cfi_def_cfa_offset	0
 173              		.cfi_offset	9,0
 174 0388 08000000 		st	%lr,0x8(,%sp)
 174      8B000A11 
 175 0390 18000000 		st	%got,0x18(,%sp)
 175      8B000F11 
 176 0398 20000000 		st	%plt,0x20(,%sp)
 176      8B001011 
 177 03a0 00000000 		lea	%got,_GLOBAL_OFFSET_TABLE_@PC_LO(-0x18)
 177      00680F06 
 178 03a8 00000000 		and	%got,%got,(32)0
 178      608F0F44 
 179 03b0 00000000 		sic	%plt
 179      00001028 
 180 03b8 00000000 		lea.sl	%got,_GLOBAL_OFFSET_TABLE_@PC_HI(%plt,%got)
 180      8F908F06 
 181 03c0 00000000 		or	%fp,0,%sp
 181      8B000945 
 182              		.cfi_def_cfa_register	9
 183 03c8 30000000 		st	%s18,48(,%fp)
 183      89001211 
 184 03d0 38000000 		st	%s19,56(,%fp)
 184      89001311 
 185 03d8 40000000 		st	%s20,64(,%fp)
 185      89001411 
 186 03e0 48000000 		st	%s21,72(,%fp)
 186      89001511 
 187 03e8 50000000 		st	%s22,80(,%fp)
 187      89001611 
 188 03f0 58000000 		st	%s23,88(,%fp)
 188      89001711 
 189 03f8 60000000 		st	%s24,96(,%fp)
 189      89001811 
 190 0400 68000000 		st	%s25,104(,%fp)
 190      89001911 
 191 0408 70000000 		st	%s26,112(,%fp)
 191      89001A11 
 192 0410 78000000 		st	%s27,120(,%fp)
 192      89001B11 
 193 0418 80000000 		st	%s28,128(,%fp)
 193      89001C11 
 194 0420 88000000 		st	%s29,136(,%fp)
 194      89001D11 
 195 0428 90000000 		st	%s30,144(,%fp)
 195      89001E11 
 196 0430 98000000 		st	%s31,152(,%fp)
 196      89001F11 
 197 0438 A0000000 		st	%s32,160(,%fp)
 197      89002011 
 198 0440 A8000000 		st	%s33,168(,%fp)
 198      89002111 
 199 0448 60FDFFFF 		lea	%s13,.L.2.2auto_size&0xffffffff
 199      00000D06 
 200 0450 00000000 		and	%s13,%s13,(32)0
 200      608D0D44 
 201 0458 FFFFFFFF 		lea.sl	%sp,.L.2.2auto_size>>32(%fp,%s13)
 201      8D898B06 
 202 0460 48000000 		brge.l.t	%sp,%sl,.L_2.EoP
 202      888B3518 
 203 0468 18000000 		ld	%s61,0x18(,%tp)
 203      8E003D01 
 204 0470 00000000 		or	%s62,0,%s0
 204      80003E45 
 205 0478 3B010000 		lea	%s63,0x13b
 205      00003F06 
 206 0480 00000000 		shm.l	%s63,0x0(%s61)
 206      BD033F31 
 207 0488 08000000 		shm.l	%sl,0x8(%s61)
 207      BD030831 
 208 0490 10000000 		shm.l	%sp,0x10(%s61)
 208      BD030B31 
 209 0498 00000000 		monc
 209      0000003F 
 210 04a0 00000000 		or	%s0,0,%s62
 210      BE000045 
 211              	.L_2.EoP:
 212              	# End of prologue codes
 213 04a8 00000000 		or	%s34,0,%s6
 213      86002245 
 214 04b0 00000000 		or	%s35,0,%s7
 214      87002345 
 215              	# line 15
  14:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ld_fn_fldx4.c **** 	long double *p_ret = &ldbl_ret;
  15:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ld_fn_fldx4.c **** 	*p_ret = ext_ld_fn_fldx4(f+f, ld+ld, f2+f2, ld2+ld2, f3+f3, ld3+ld3, f4+f4, ld4+ld4);
 216              		.loc	1 15 0
 217 04b8 00000000 		fadd.q	%s36,%s34,%s34
 217      A2A2246C 
 218 04c0 00000000 		fadd.s	%s63,%s4,%s4
 218      8484BF4C 
 219              	# line 13
  13:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ld_fn_fldx4.c **** 	long double *p_ret = &ldbl_ret;
 220              		.loc	1 13 0
 221 04c8 00000000 		or	%s38,0,%s2
 221      82002645 
 222 04d0 00000000 		or	%s39,0,%s3
 222      83002745 
 223              	# line 15
 224              		.loc	1 15 0
 225 04d8 00000000 		fadd.q	%s40,%s38,%s38
 225      A6A6286C 
 226 04e0 00000000 		fadd.s	%s62,%s0,%s0
 226      8080BE4C 
 227              	# line 13
  13:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ld_fn_fldx4.c **** 	long double *p_ret = &ldbl_ret;
 228              		.loc	1 13 0
 229 04e8 C0000000 		st	%s39,192(0,%fp)	# ld
 229      89002711 
 230 04f0 C0000000 		st	%s38,192(8,%fp)	# ld
 230      89082611 
 231 04f8 E0000000 		st	%s35,224(0,%fp)	# ld2
 231      89002311 
 232 0500 E0000000 		st	%s34,224(8,%fp)	# ld2
 232      89082211 
 233              	# line 14
  14:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ld_fn_fldx4.c **** 	long double *p_ret = &ldbl_ret;
 234              		.loc	1 14 0
 235 0508 00000000 		lea	%s61,ldbl_ret@GOTOFF_LO
 235      00003D06 
 236 0510 00000000 		and	%s61,%s61,(32)0
 236      60BD3D44 
 237 0518 00000000 		lea.sl	%s61,ldbl_ret@GOTOFF_HI(%s61,%got)
 237      8FBDBD06 
 238              	# line 15
 239              		.loc	1 15 0
 240 0520 F4000000 		ldu	%s60,244(0,%fp)	# f3
 240      89003C02 
 241 0528 00000000 		fadd.s	%s59,%s60,%s60
 241      BCBCBB4C 
 242 0530 F0000000 		st	%s59,240(0,%sp)
 242      8B003B11 
 243 0538 00010000 		ld	%s43,256(0,%fp)	# ld3
 243      89002B01 
 244 0540 00010000 		ld	%s42,256(8,%fp)	# ld3
 244      89082A01 
 245 0548 00000000 		fadd.q	%s44,%s42,%s42
 245      AAAA2C6C 
 246 0550 14010000 		ldu	%s58,276(0,%fp)	# f4
 246      89003A02 
 247 0558 00000000 		fadd.s	%s57,%s58,%s58
 247      BABAB94C 
 248 0560 10010000 		st	%s57,272(0,%sp)
 248      8B003911 
 249 0568 20010000 		ld	%s47,288(0,%fp)	# ld4
 249      89002F01 
 250 0570 20010000 		ld	%s46,288(8,%fp)	# ld4
 250      89082E01 
 251 0578 00000000 		fadd.q	%s48,%s46,%s46
 251      AEAE306C 
 252 0580 00000000 		or	%s56,0,%s40
 252      A8003845 
 253 0588 00000000 		or	%s55,0,%s41
 253      A9003745 
 254 0590 00000000 		or	%s54,0,%s36
 254      A4003645 
 255 0598 00000000 		or	%s53,0,%s37
 255      A5003545 
 256 05a0 00000000 		or	%s52,0,%s44
 256      AC003445 
 257 05a8 08010000 		st	%s52,264(0,%sp)
 257      8B003411 
 258 05b0 00000000 		or	%s51,0,%s45
 258      AD003345 
 259 05b8 00010000 		st	%s51,256(0,%sp)
 259      8B003311 
 260 05c0 00000000 		or	%s50,0,%s48
 260      B0003245 
 261 05c8 28010000 		st	%s50,296(0,%sp)
 261      8B003211 
 262 05d0 00000000 		or	%s18,0,%s49
 262      B1001245 
 263 05d8 20010000 		st	%s18,288(0,%sp)
 263      8B001211 
 264 05e0 E8FFFFFF 		ld	%s19,-24(,%fp)	# restore
 264      89001301 
 265 05e8 F8000000 		st	%s19,248(0,%sp)
 265      8B001311 
 266 05f0 E0FFFFFF 		ld	%s20,-32(,%fp)	# restore
 266      89001401 
 267 05f8 18010000 		st	%s20,280(0,%sp)
 267      8B001411 
 268 0600 00000000 		or	%s0,0,%s62
 268      BE000045 
 269 0608 F8FFFFFF 		ld	%s1,-8(,%fp)	# restore
 269      89000101 
 270 0610 00000000 		or	%s2,0,%s56
 270      B8000245 
 271 0618 00000000 		or	%s3,0,%s55
 271      B7000345 
 272 0620 00000000 		or	%s4,0,%s63
 272      BF000445 
 273 0628 F0FFFFFF 		ld	%s5,-16(,%fp)	# restore
 273      89000501 
 274 0630 00000000 		or	%s6,0,%s54
 274      B6000645 
 275 0638 00000000 		or	%s7,0,%s53
 275      B5000745 
 276 0640 D8FFFFFF 		st	%s61,-40(,%fp)	# spill
 276      89003D11 
 277 0648 00000000 		lea	%s12,ext_ld_fn_fldx4@PLT_LO(-24)
 277      00680C06 
 278 0650 00000000 		and	%s12,%s12,(32)0
 278      608C0C44 
 279 0658 00000000 		sic	%lr
 279      00000A28 
 280 0660 00000000 		lea.sl	%s12,ext_ld_fn_fldx4@PLT_HI(%s12,%lr)
 280      8A8C8C06 
 281 0668 00000000 		bsic	%lr,(,%s12)		# ext_ld_fn_fldx4
 281      8C000A08 
 282 0670 C8FFFFFF 		st	%s0,-56(8,%fp)	# spill
 282      89080011 
 283 0678 C8FFFFFF 		st	%s1,-56(,%fp)	# spill
 283      89000111 
 284 0680 08000000 		br.l	.L_2.1
 284      00000F18 
 285              	.L_2.1:
 286 0688 D8FFFFFF 		ld	%s63,-40(,%fp)	# restore
 286      89003F01 
 287 0690 C8FFFFFF 		ld	%s18,-56(8,%fp)	# restore
 287      89081201 
 288 0698 C8FFFFFF 		ld	%s19,-56(,%fp)	# restore
 288      89001301 
 289 06a0 00000000 		st	%s19,0(0,%s63)	# *(p_ret)
 289      BF001311 
 290 06a8 00000000 		st	%s18,0(8,%s63)	# *(p_ret)
 290      BF081211 
 291              	# line 16
  16:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ld_fn_fldx4.c **** 	return *p_ret;
 292              		.loc	1 16 0
 293 06b0 00000000 		or	%s62,0,%s18
 293      92003E45 
 294 06b8 00000000 		or	%s61,0,%s19
 294      93003D45 
 295 06c0 00000000 		or	%s0,0,%s62
 295      BE000045 
 296 06c8 00000000 		or	%s1,0,%s61
 296      BD000145 
 297              	# Start of epilogue codes
 298 06d0 30000000 		ld	%s18,48(,%fp)
 298      89001201 
 299 06d8 38000000 		ld	%s19,56(,%fp)
 299      89001301 
 300 06e0 40000000 		ld	%s20,64(,%fp)
 300      89001401 
 301 06e8 00000000 		or	%sp,0,%fp
 301      89000B45 
 302              		.cfi_def_cfa	11,8
 303 06f0 18000000 		ld	%got,0x18(,%sp)
 303      8B000F01 
 304 06f8 20000000 		ld	%plt,0x20(,%sp)
 304      8B001001 
 305 0700 08000000 		ld	%lr,0x8(,%sp)
 305      8B000A01 
 306 0708 00000000 		ld	%fp,0x0(,%sp)
 306      8B000901 
 307 0710 00000000 		b.l	(,%lr)
 307      8A000F19 
 308              	.L_2.EoE:
 309              		.cfi_endproc
 310              		.set	.L.2.2auto_size,	0xfffffffffffffd60	# 672 Bytes
 312              	# ============ End  alt_ld_fn_fldx4 ============
 313              	# ============ Begin  mt_ld_fn_fldx4 ============
 314 0718 00000000 		.balign 16
 314      00000000 
 315              	.L_3.0:
 316              	# line 19
  17:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ld_fn_fldx4.c **** }
  18:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ld_fn_fldx4.c **** long double mt_ld_fn_fldx4(float f, long double ld, float f2, long double ld2,
  19:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ld_fn_fldx4.c **** 		float f3, long double ld3, float f4, long double ld4) {
 317              		.loc	1 19 0
 318              		.globl	mt_ld_fn_fldx4
 320              	mt_ld_fn_fldx4:
 321              		.cfi_startproc
 322 0720 00000000 		st	%fp,0x0(,%sp)
 322      8B000911 
 323              		.cfi_def_cfa_offset	0
 324              		.cfi_offset	9,0
 325 0728 08000000 		st	%lr,0x8(,%sp)
 325      8B000A11 
 326 0730 18000000 		st	%got,0x18(,%sp)
 326      8B000F11 
 327 0738 20000000 		st	%plt,0x20(,%sp)
 327      8B001011 
 328 0740 00000000 		lea	%got,_GLOBAL_OFFSET_TABLE_@PC_LO(-0x18)
 328      00680F06 
 329 0748 00000000 		and	%got,%got,(32)0
 329      608F0F44 
 330 0750 00000000 		sic	%plt
 330      00001028 
 331 0758 00000000 		lea.sl	%got,_GLOBAL_OFFSET_TABLE_@PC_HI(%plt,%got)
 331      8F908F06 
 332 0760 00000000 		or	%fp,0,%sp
 332      8B000945 
 333              		.cfi_def_cfa_register	9
 334 0768 30000000 		st	%s18,48(,%fp)
 334      89001211 
 335 0770 38000000 		st	%s19,56(,%fp)
 335      89001311 
 336 0778 40000000 		st	%s20,64(,%fp)
 336      89001411 
 337 0780 48000000 		st	%s21,72(,%fp)
 337      89001511 
 338 0788 50000000 		st	%s22,80(,%fp)
 338      89001611 
 339 0790 58000000 		st	%s23,88(,%fp)
 339      89001711 
 340 0798 60000000 		st	%s24,96(,%fp)
 340      89001811 
 341 07a0 68000000 		st	%s25,104(,%fp)
 341      89001911 
 342 07a8 70000000 		st	%s26,112(,%fp)
 342      89001A11 
 343 07b0 78000000 		st	%s27,120(,%fp)
 343      89001B11 
 344 07b8 80000000 		st	%s28,128(,%fp)
 344      89001C11 
 345 07c0 88000000 		st	%s29,136(,%fp)
 345      89001D11 
 346 07c8 90000000 		st	%s30,144(,%fp)
 346      89001E11 
 347 07d0 98000000 		st	%s31,152(,%fp)
 347      89001F11 
 348 07d8 A0000000 		st	%s32,160(,%fp)
 348      89002011 
 349 07e0 A8000000 		st	%s33,168(,%fp)
 349      89002111 
 350 07e8 F0FFFFFF 		lea	%s13,.L.3.2auto_size&0xffffffff
 350      00000D06 
 351 07f0 00000000 		and	%s13,%s13,(32)0
 351      608D0D44 
 352 07f8 FFFFFFFF 		lea.sl	%sp,.L.3.2auto_size>>32(%fp,%s13)
 352      8D898B06 
 353 0800 48000000 		brge.l.t	%sp,%sl,.L_3.EoP
 353      888B3518 
 354 0808 18000000 		ld	%s61,0x18(,%tp)
 354      8E003D01 
 355 0810 00000000 		or	%s62,0,%s0
 355      80003E45 
 356 0818 3B010000 		lea	%s63,0x13b
 356      00003F06 
 357 0820 00000000 		shm.l	%s63,0x0(%s61)
 357      BD033F31 
 358 0828 08000000 		shm.l	%sl,0x8(%s61)
 358      BD030831 
 359 0830 10000000 		shm.l	%sp,0x10(%s61)
 359      BD030B31 
 360 0838 00000000 		monc
 360      0000003F 
 361 0840 00000000 		or	%s0,0,%s62
 361      BE000045 
 362              	.L_3.EoP:
 363              	# End of prologue codes
 364 0848 00000000 		or	%s34,0,%s6
 364      86002245 
 365 0850 00000000 		or	%s35,0,%s7
 365      87002345 
 366              	# line 20
  20:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ld_fn_fldx4.c **** 	long double ret = f + ld + f2 + ld2 + f3 + ld3 + f4 + ld4;
 367              		.loc	1 20 0
 368 0858 00000000 		cvt.q.s %s36,%s4
 368      0084A42D 
 369              	# line 19
  19:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ld_fn_fldx4.c **** 	long double ret = f + ld + f2 + ld2 + f3 + ld3 + f4 + ld4;
 370              		.loc	1 19 0
 371 0860 00000000 		or	%s38,0,%s2
 371      82002645 
 372 0868 00000000 		or	%s39,0,%s3
 372      83002745 
 373              	# line 20
 374              		.loc	1 20 0
 375 0870 00000000 		cvt.q.s %s40,%s0
 375      0080A82D 
 376 0878 00000000 		fadd.q	%s42,%s38,%s40
 376      A8A62A6C 
 377 0880 00000000 		fadd.q	%s44,%s36,%s42
 377      AAA42C6C 
 378 0888 00000000 		fadd.q	%s46,%s34,%s44
 378      ACA22E6C 
 379              	# line 19
  19:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ld_fn_fldx4.c **** 	long double ret = f + ld + f2 + ld2 + f3 + ld3 + f4 + ld4;
 380              		.loc	1 19 0
 381 0890 C0000000 		st	%s39,192(0,%fp)	# ld
 381      89002711 
 382 0898 C0000000 		st	%s38,192(8,%fp)	# ld
 382      89082611 
 383 08a0 E0000000 		st	%s35,224(0,%fp)	# ld2
 383      89002311 
 384 08a8 E0000000 		st	%s34,224(8,%fp)	# ld2
 384      89082211 
 385              	# line 20
 386              		.loc	1 20 0
 387 08b0 F4000000 		ldu	%s63,244(0,%fp)	# f3
 387      89003F02 
 388 08b8 00000000 		cvt.q.s %s48,%s63
 388      00BFB02D 
 389 08c0 00000000 		fadd.q	%s50,%s46,%s48
 389      B0AE326C 
 390 08c8 00010000 		ld	%s53,256(0,%fp)	# ld3
 390      89003501 
 391 08d0 00010000 		ld	%s52,256(8,%fp)	# ld3
 391      89083401 
 392 08d8 00000000 		fadd.q	%s54,%s50,%s52
 392      B4B2366C 
 393 08e0 14010000 		ldu	%s62,276(0,%fp)	# f4
 393      89003E02 
 394 08e8 00000000 		cvt.q.s %s56,%s62
 394      00BEB82D 
 395 08f0 00000000 		fadd.q	%s58,%s54,%s56
 395      B8B63A6C 
 396 08f8 20010000 		ld	%s61,288(0,%fp)	# ld4
 396      89003D01 
 397 0900 20010000 		ld	%s60,288(8,%fp)	# ld4
 397      89083C01 
 398 0908 00000000 		fadd.q	%s18,%s58,%s60
 398      BCBA126C 
 399 0910 F0FFFFFF 		st	%s19,-16(0,%fp)	# ret
 399      89001311 
 400 0918 F0FFFFFF 		st	%s18,-16(8,%fp)	# ret
 400      89081211 
 401              	# line 21
  21:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ld_fn_fldx4.c **** 	return ret;
 402              		.loc	1 21 0
 403 0920 00000000 		or	%s20,0,%s18
 403      92001445 
 404 0928 00000000 		or	%s21,0,%s19
 404      93001545 
 405 0930 00000000 		or	%s0,0,%s20
 405      94000045 
 406 0938 00000000 		or	%s1,0,%s21
 406      95000145 
 407              	# Start of epilogue codes
 408 0940 30000000 		ld	%s18,48(,%fp)
 408      89001201 
 409 0948 38000000 		ld	%s19,56(,%fp)
 409      89001301 
 410 0950 40000000 		ld	%s20,64(,%fp)
 410      89001401 
 411 0958 48000000 		ld	%s21,72(,%fp)
 411      89001501 
 412 0960 00000000 		or	%sp,0,%fp
 412      89000B45 
 413              		.cfi_def_cfa	11,8
 414 0968 18000000 		ld	%got,0x18(,%sp)
 414      8B000F01 
 415 0970 20000000 		ld	%plt,0x20(,%sp)
 415      8B001001 
 416 0978 08000000 		ld	%lr,0x8(,%sp)
 416      8B000A01 
 417 0980 00000000 		ld	%fp,0x0(,%sp)
 417      8B000901 
 418 0988 00000000 		b.l	(,%lr)
 418      8A000F19 
 419              	.L_3.EoE:
 420              		.cfi_endproc
 421              		.set	.L.3.2auto_size,	0xfffffffffffffff0	# 16 Bytes
 423              	# ============ End  mt_ld_fn_fldx4 ============
 424              	# ============ Begin  prt_ld_fn_fldx4 ============
 425              		.section .rodata
 426              		.balign 16
 428              	.LP.__string.0:
 429 0000 25       		.byte	37
 430 0001 73       		.byte	115
 431 0002 3A       		.byte	58
 432 0003 25       		.byte	37
 433 0004 75       		.byte	117
 434 0005 20       		.byte	32
 435 0006 6C       		.byte	108
 436 0007 64       		.byte	100
 437 0008 20       		.byte	32
 438 0009 25       		.byte	37
 439 000a 66       		.byte	102
 440 000b 0A       		.byte	10
 441 000c 00       		.zero	1
 442 000d 000000   		.balign 8
 444              	.LP.__string.1:
 445 0010 2F       		.byte	47
 446 0011 75       		.byte	117
 447 0012 73       		.byte	115
 448 0013 72       		.byte	114
 449 0014 2F       		.byte	47
 450 0015 75       		.byte	117
 451 0016 68       		.byte	104
 452 0017 6F       		.byte	111
 453 0018 6D       		.byte	109
 454 0019 65       		.byte	101
 455 001a 2F       		.byte	47
 456 001b 61       		.byte	97
 457 001c 75       		.byte	117
 458 001d 72       		.byte	114
 459 001e 6F       		.byte	111
 460 001f 72       		.byte	114
 461 0020 61       		.byte	97
 462 0021 2F       		.byte	47
 463 0022 34       		.byte	52
 464 0023 67       		.byte	103
 465 0024 69       		.byte	105
 466 0025 2F       		.byte	47
 467 0026 6E       		.byte	110
 468 0027 6C       		.byte	108
 469 0028 61       		.byte	97
 470 0029 62       		.byte	98
 471 002a 68       		.byte	104
 472 002b 70       		.byte	112
 473 002c 67       		.byte	103
 474 002d 2F       		.byte	47
 475 002e 6B       		.byte	107
 476 002f 72       		.byte	114
 477 0030 75       		.byte	117
 478 0031 75       		.byte	117
 479 0032 73       		.byte	115
 480 0033 2F       		.byte	47
 481 0034 76       		.byte	118
 482 0035 74       		.byte	116
 483 0036 2F       		.byte	47
 484 0037 73       		.byte	115
 485 0038 72       		.byte	114
 486 0039 63       		.byte	99
 487 003a 2F       		.byte	47
 488 003b 61       		.byte	97
 489 003c 73       		.byte	115
 490 003d 6D       		.byte	109
 491 003e 2D       		.byte	45
 492 003f 65       		.byte	101
 493 0040 78       		.byte	120
 494 0041 61       		.byte	97
 495 0042 6D       		.byte	109
 496 0043 70       		.byte	112
 497 0044 6C       		.byte	108
 498 0045 65       		.byte	101
 499 0046 73       		.byte	115
 500 0047 2F       		.byte	47
 501 0048 6C       		.byte	108
 502 0049 64       		.byte	100
 503 004a 5F       		.byte	95
 504 004b 66       		.byte	102
 505 004c 6E       		.byte	110
 506 004d 5F       		.byte	95
 507 004e 66       		.byte	102
 508 004f 6C       		.byte	108
 509 0050 64       		.byte	100
 510 0051 78       		.byte	120
 511 0052 34       		.byte	52
 512 0053 2E       		.byte	46
 513 0054 63       		.byte	99
 514 0055 00       		.zero	1
 515              		.text
 516              		.balign 16
 517              	.L_4.0:
 518              	# line 24
  22:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ld_fn_fldx4.c **** }
  23:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ld_fn_fldx4.c **** long double prt_ld_fn_fldx4(float f, long double ld, float f2, long double ld2,
  24:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ld_fn_fldx4.c **** 		float f3, long double ld3, float f4, long double ld4) {
 519              		.loc	1 24 0
 520              		.globl	prt_ld_fn_fldx4
 522              	prt_ld_fn_fldx4:
 523              		.cfi_startproc
 524 0990 00000000 		st	%fp,0x0(,%sp)
 524      8B000911 
 525              		.cfi_def_cfa_offset	0
 526              		.cfi_offset	9,0
 527 0998 08000000 		st	%lr,0x8(,%sp)
 527      8B000A11 
 528 09a0 18000000 		st	%got,0x18(,%sp)
 528      8B000F11 
 529 09a8 20000000 		st	%plt,0x20(,%sp)
 529      8B001011 
 530 09b0 00000000 		lea	%got,_GLOBAL_OFFSET_TABLE_@PC_LO(-0x18)
 530      00680F06 
 531 09b8 00000000 		and	%got,%got,(32)0
 531      608F0F44 
 532 09c0 00000000 		sic	%plt
 532      00001028 
 533 09c8 00000000 		lea.sl	%got,_GLOBAL_OFFSET_TABLE_@PC_HI(%plt,%got)
 533      8F908F06 
 534 09d0 00000000 		or	%fp,0,%sp
 534      8B000945 
 535              		.cfi_def_cfa_register	9
 536 09d8 30000000 		st	%s18,48(,%fp)
 536      89001211 
 537 09e0 38000000 		st	%s19,56(,%fp)
 537      89001311 
 538 09e8 40000000 		st	%s20,64(,%fp)
 538      89001411 
 539 09f0 48000000 		st	%s21,72(,%fp)
 539      89001511 
 540 09f8 50000000 		st	%s22,80(,%fp)
 540      89001611 
 541 0a00 58000000 		st	%s23,88(,%fp)
 541      89001711 
 542 0a08 60000000 		st	%s24,96(,%fp)
 542      89001811 
 543 0a10 68000000 		st	%s25,104(,%fp)
 543      89001911 
 544 0a18 70000000 		st	%s26,112(,%fp)
 544      89001A11 
 545 0a20 78000000 		st	%s27,120(,%fp)
 545      89001B11 
 546 0a28 80000000 		st	%s28,128(,%fp)
 546      89001C11 
 547 0a30 88000000 		st	%s29,136(,%fp)
 547      89001D11 
 548 0a38 90000000 		st	%s30,144(,%fp)
 548      89001E11 
 549 0a40 98000000 		st	%s31,152(,%fp)
 549      89001F11 
 550 0a48 A0000000 		st	%s32,160(,%fp)
 550      89002011 
 551 0a50 A8000000 		st	%s33,168(,%fp)
 551      89002111 
 552 0a58 20FEFFFF 		lea	%s13,.L.4.2auto_size&0xffffffff
 552      00000D06 
 553 0a60 00000000 		and	%s13,%s13,(32)0
 553      608D0D44 
 554 0a68 FFFFFFFF 		lea.sl	%sp,.L.4.2auto_size>>32(%fp,%s13)
 554      8D898B06 
 555 0a70 48000000 		brge.l.t	%sp,%sl,.L_4.EoP
 555      888B3518 
 556 0a78 18000000 		ld	%s61,0x18(,%tp)
 556      8E003D01 
 557 0a80 00000000 		or	%s62,0,%s0
 557      80003E45 
 558 0a88 3B010000 		lea	%s63,0x13b
 558      00003F06 
 559 0a90 00000000 		shm.l	%s63,0x0(%s61)
 559      BD033F31 
 560 0a98 08000000 		shm.l	%sl,0x8(%s61)
 560      BD030831 
 561 0aa0 10000000 		shm.l	%sp,0x10(%s61)
 561      BD030B31 
 562 0aa8 00000000 		monc
 562      0000003F 
 563 0ab0 00000000 		or	%s0,0,%s62
 563      BE000045 
 564              	.L_4.EoP:
 565              	# End of prologue codes
 566 0ab8 00000000 		or	%s34,0,%s6
 566      86002245 
 567 0ac0 00000000 		or	%s35,0,%s7
 567      87002345 
 568              	# line 25
  25:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ld_fn_fldx4.c **** 	long double ret = f + ld + f2 + ld2 + f3 + ld3 + f4 + ld4;
 569              		.loc	1 25 0
 570 0ac8 00000000 		cvt.q.s %s36,%s4
 570      0084A42D 
 571              	# line 24
  24:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ld_fn_fldx4.c **** 	long double ret = f + ld + f2 + ld2 + f3 + ld3 + f4 + ld4;
 572              		.loc	1 24 0
 573 0ad0 00000000 		or	%s38,0,%s2
 573      82002645 
 574 0ad8 00000000 		or	%s39,0,%s3
 574      83002745 
 575              	# line 25
 576              		.loc	1 25 0
 577 0ae0 00000000 		cvt.q.s %s40,%s0
 577      0080A82D 
 578 0ae8 00000000 		fadd.q	%s42,%s38,%s40
 578      A8A62A6C 
 579 0af0 00000000 		fadd.q	%s44,%s36,%s42
 579      AAA42C6C 
 580 0af8 00000000 		fadd.q	%s46,%s34,%s44
 580      ACA22E6C 
 581              	# line 24
  24:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ld_fn_fldx4.c **** 	long double ret = f + ld + f2 + ld2 + f3 + ld3 + f4 + ld4;
 582              		.loc	1 24 0
 583 0b00 C0000000 		st	%s39,192(0,%fp)	# ld
 583      89002711 
 584 0b08 C0000000 		st	%s38,192(8,%fp)	# ld
 584      89082611 
 585 0b10 E0000000 		st	%s35,224(0,%fp)	# ld2
 585      89002311 
 586 0b18 E0000000 		st	%s34,224(8,%fp)	# ld2
 586      89082211 
 587              	# line 25
 588              		.loc	1 25 0
 589 0b20 F4000000 		ldu	%s63,244(0,%fp)	# f3
 589      89003F02 
 590 0b28 00000000 		cvt.q.s %s48,%s63
 590      00BFB02D 
 591 0b30 00000000 		fadd.q	%s50,%s46,%s48
 591      B0AE326C 
 592 0b38 00010000 		ld	%s53,256(0,%fp)	# ld3
 592      89003501 
 593 0b40 00010000 		ld	%s52,256(8,%fp)	# ld3
 593      89083401 
 594 0b48 00000000 		fadd.q	%s54,%s50,%s52
 594      B4B2366C 
 595 0b50 14010000 		ldu	%s62,276(0,%fp)	# f4
 595      89003E02 
 596 0b58 00000000 		cvt.q.s %s56,%s62
 596      00BEB82D 
 597 0b60 00000000 		fadd.q	%s58,%s54,%s56
 597      B8B63A6C 
 598 0b68 20010000 		ld	%s61,288(0,%fp)	# ld4
 598      89003D01 
 599 0b70 20010000 		ld	%s60,288(8,%fp)	# ld4
 599      89083C01 
 600 0b78 00000000 		fadd.q	%s18,%s58,%s60
 600      BCBA126C 
 601              	# line 26
  26:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ld_fn_fldx4.c **** 	printf("%s:%u ld %f\n",__FILE__,__LINE__,(double)ret);
 602              		.loc	1 26 0
 603 0b80 00000000 		cvt.d.q %s20,%s18
 603      0092940F 
 604 0b88 C8000000 		st	%s20,200(0,%sp)
 604      8B001411 
 605              	# line 25
  25:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ld_fn_fldx4.c **** 	long double ret = f + ld + f2 + ld2 + f3 + ld3 + f4 + ld4;
 606              		.loc	1 25 0
 607 0b90 F0FFFFFF 		st	%s19,-16(0,%fp)	# ret
 607      89001311 
 608 0b98 F0FFFFFF 		st	%s18,-16(8,%fp)	# ret
 608      89081211 
 609              	# line 26
 610              		.loc	1 26 0
 611 0ba0 00000000 		lea	%s21,.LP.__string.0@GOTOFF_LO
 611      00001506 
 612 0ba8 00000000 		and	%s21,%s21,(32)0
 612      60951544 
 613 0bb0 00000000 		lea.sl	%s21,.LP.__string.0@GOTOFF_HI(%s21,%got)
 613      8F959506 
 614 0bb8 B0000000 		st	%s21,176(0,%sp)
 614      8B001511 
 615 0bc0 00000000 		lea	%s22,.LP.__string.1@GOTOFF_LO
 615      00001606 
 616 0bc8 00000000 		and	%s22,%s22,(32)0
 616      60961644 
 617 0bd0 00000000 		lea.sl	%s22,.LP.__string.1@GOTOFF_HI(%s22,%got)
 617      8F969606 
 618 0bd8 B8000000 		st	%s22,184(0,%sp)
 618      8B001611 
 619 0be0 00000000 		or	%s23,26,(0)1
 619      001A1745 
 620 0be8 C0000000 		st	%s23,192(0,%sp)
 620      8B001711 
 621 0bf0 00000000 		or	%s0,0,%s21
 621      95000045 
 622 0bf8 00000000 		or	%s1,0,%s22
 622      96000145 
 623 0c00 00000000 		or	%s2,0,%s23
 623      97000245 
 624 0c08 00000000 		or	%s3,0,%s20
 624      94000345 
 625 0c10 00000000 		lea	%s12,printf@PLT_LO(-24)
 625      00680C06 
 626 0c18 00000000 		and	%s12,%s12,(32)0
 626      608C0C44 
 627 0c20 00000000 		sic	%lr
 627      00000A28 
 628 0c28 00000000 		lea.sl	%s12,printf@PLT_HI(%s12,%lr)
 628      8A8C8C06 
 629 0c30 00000000 		bsic	%lr,(,%s12)		# printf
 629      8C000A08 
 630 0c38 08000000 		br.l	.L_4.1
 630      00000F18 
 631              	.L_4.1:
 632              	# line 27
  27:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ld_fn_fldx4.c **** 	return ret;
 633              		.loc	1 27 0
 634 0c40 F0FFFFFF 		ld	%s19,-16(0,%fp)	# ret
 634      89001301 
 635 0c48 F0FFFFFF 		ld	%s18,-16(8,%fp)	# ret
 635      89081201 
 636 0c50 00000000 		or	%s63,0,%s18
 636      92003F45 
 637 0c58 00000000 		or	%s62,0,%s19
 637      93003E45 
 638 0c60 00000000 		or	%s0,0,%s63
 638      BF000045 
 639 0c68 00000000 		or	%s1,0,%s62
 639      BE000145 
 640              	# Start of epilogue codes
 641 0c70 30000000 		ld	%s18,48(,%fp)
 641      89001201 
 642 0c78 38000000 		ld	%s19,56(,%fp)
 642      89001301 
 643 0c80 40000000 		ld	%s20,64(,%fp)
 643      89001401 
 644 0c88 48000000 		ld	%s21,72(,%fp)
 644      89001501 
 645 0c90 50000000 		ld	%s22,80(,%fp)
 645      89001601 
 646 0c98 58000000 		ld	%s23,88(,%fp)
 646      89001701 
 647 0ca0 00000000 		or	%sp,0,%fp
 647      89000B45 
 648              		.cfi_def_cfa	11,8
 649 0ca8 18000000 		ld	%got,0x18(,%sp)
 649      8B000F01 
 650 0cb0 20000000 		ld	%plt,0x20(,%sp)
 650      8B001001 
 651 0cb8 08000000 		ld	%lr,0x8(,%sp)
 651      8B000A01 
 652 0cc0 00000000 		ld	%fp,0x0(,%sp)
 652      8B000901 
 653 0cc8 00000000 		b.l	(,%lr)
 653      8A000F19 
 654              	.L_4.EoE:
 655              		.cfi_endproc
 656              		.set	.L.4.2auto_size,	0xfffffffffffffe20	# 480 Bytes
 658              	# ============ End  prt_ld_fn_fldx4 ============
 659              	.Le1.0:
