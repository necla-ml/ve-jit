   1              		.ident "ncc 1.1.4 (Build 10:17:22 Apr 19 2018)"
   2              		.file "ldbl_fn_ldbl.c"
   3              		.file 1 "/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ldbl_fn_ldbl.c"
   4              		.file 2 "/opt/nec/ve/ncc/1.1.4/include/stdc-predef.h"
   5              		.file 3 "/opt/nec/ve/ncc/1.1.4/include/stdio.h"
   6              		.file 4 "/opt/nec/ve/ncc/1.1.4/include/yvals.h"
   7              		.file 5 "/opt/nec/ve/ncc/1.1.4/include/necvals.h"
   8              		.file 6 "/opt/nec/ve/ncc/1.1.4/include/stdarg.h"
   9              		.file 7 "/opt/nec/ve/musl/include/stdio.h"
  10              		.file 8 "/opt/nec/ve/musl/include/features.h"
  11              		.file 9 "/opt/nec/ve/musl/include/bits/alltypes.h"
  12              	# ============ Begin  ldbl_fn_ldbl ============
  13              		.section	.bss
  14              		.local	ldbl_ret
  15              	.comm ldbl_ret,16,16
  16              		.text
  17              		.balign 16
  18              	.L_1.0:
  19              	# line 6
   1:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ldbl_fn_ldbl.c **** #include <stdio.h>
   2:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ldbl_fn_ldbl.c **** 
   3:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ldbl_fn_ldbl.c **** extern long double ext_ldbl_fn_ldbl(long double ld);
   4:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ldbl_fn_ldbl.c **** 
   5:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ldbl_fn_ldbl.c **** static long double ldbl_ret;
   6:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ldbl_fn_ldbl.c **** long double ldbl_fn_ldbl(long double ld) {
  20              		.loc	1 6 0
  21              		.globl	ldbl_fn_ldbl
  23              	ldbl_fn_ldbl:
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
  53 00c8 20FEFFFF 		lea	%s13,.L.1.2auto_size&0xffffffff
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
  67 0128 00000000 		or	%s34,0,%s0
  67      80002245 
  68 0130 00000000 		or	%s35,0,%s1
  68      81002345 
  69 0138 B0000000 		st	%s35,176(0,%fp)	# ld
  69      89002311 
  70 0140 B0000000 		st	%s34,176(8,%fp)	# ld
  70      89082211 
  71              	# line 7
   7:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ldbl_fn_ldbl.c **** 	ldbl_ret = ext_ldbl_fn_ldbl(ld);
  72              		.loc	1 7 0
  73 0148 00000000 		or	%s63,0,%s34
  73      A2003F45 
  74 0150 00000000 		or	%s62,0,%s35
  74      A3003E45 
  75 0158 00000000 		or	%s0,0,%s63
  75      BF000045 
  76 0160 00000000 		or	%s1,0,%s62
  76      BE000145 
  77 0168 00000000 		lea	%s12,ext_ldbl_fn_ldbl@PLT_LO(-24)
  77      00680C06 
  78 0170 00000000 		and	%s12,%s12,(32)0
  78      608C0C44 
  79 0178 00000000 		sic	%lr
  79      00000A28 
  80 0180 00000000 		lea.sl	%s12,ext_ldbl_fn_ldbl@PLT_HI(%s12,%lr)
  80      8A8C8C06 
  81 0188 00000000 		bsic	%lr,(,%s12)		# ext_ldbl_fn_ldbl
  81      8C000A08 
  82 0190 F0FFFFFF 		st	%s0,-16(8,%fp)	# spill
  82      89080011 
  83 0198 F0FFFFFF 		st	%s1,-16(,%fp)	# spill
  83      89000111 
  84 01a0 08000000 		br.l	.L_1.1
  84      00000F18 
  85              	.L_1.1:
  86 01a8 00000000 		lea	%s63,ldbl_ret@GOTOFF_LO
  86      00003F06 
  87 01b0 00000000 		and	%s63,%s63,(32)0
  87      60BF3F44 
  88 01b8 00000000 		lea.sl	%s63,ldbl_ret@GOTOFF_HI(%s63,%got)
  88      8FBFBF06 
  89 01c0 F0FFFFFF 		ld	%s34,-16(8,%fp)	# restore
  89      89082201 
  90 01c8 F0FFFFFF 		ld	%s35,-16(,%fp)	# restore
  90      89002301 
  91 01d0 00000000 		st	%s35,0(0,%s63)	# ldbl_ret
  91      BF002311 
  92 01d8 00000000 		st	%s34,0(8,%s63)	# ldbl_ret
  92      BF082211 
  93              	# line 8
   8:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ldbl_fn_ldbl.c **** 	return ldbl_ret;
  94              		.loc	1 8 0
  95 01e0 00000000 		or	%s62,0,%s34
  95      A2003E45 
  96 01e8 00000000 		or	%s61,0,%s35
  96      A3003D45 
  97 01f0 00000000 		or	%s0,0,%s62
  97      BE000045 
  98 01f8 00000000 		or	%s1,0,%s61
  98      BD000145 
  99              	# Start of epilogue codes
 100 0200 00000000 		or	%sp,0,%fp
 100      89000B45 
 101              		.cfi_def_cfa	11,8
 102 0208 18000000 		ld	%got,0x18(,%sp)
 102      8B000F01 
 103 0210 20000000 		ld	%plt,0x20(,%sp)
 103      8B001001 
 104 0218 08000000 		ld	%lr,0x8(,%sp)
 104      8B000A01 
 105 0220 00000000 		ld	%fp,0x0(,%sp)
 105      8B000901 
 106 0228 00000000 		b.l	(,%lr)
 106      8A000F19 
 107              	.L_1.EoE:
 108              		.cfi_endproc
 109              		.set	.L.1.2auto_size,	0xfffffffffffffe20	# 480 Bytes
 111              	# ============ End  ldbl_fn_ldbl ============
 112              	# ============ Begin  ldbl_fn_ldbl2 ============
 113              		.balign 16
 114              	.L_2.0:
 115              	# line 10
   9:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ldbl_fn_ldbl.c **** }
  10:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ldbl_fn_ldbl.c **** long double ldbl_fn_ldbl2(long double ld) {
 116              		.loc	1 10 0
 117              		.globl	ldbl_fn_ldbl2
 119              	ldbl_fn_ldbl2:
 120              		.cfi_startproc
 121 0230 00000000 		st	%fp,0x0(,%sp)
 121      8B000911 
 122              		.cfi_def_cfa_offset	0
 123              		.cfi_offset	9,0
 124 0238 08000000 		st	%lr,0x8(,%sp)
 124      8B000A11 
 125 0240 18000000 		st	%got,0x18(,%sp)
 125      8B000F11 
 126 0248 20000000 		st	%plt,0x20(,%sp)
 126      8B001011 
 127 0250 00000000 		lea	%got,_GLOBAL_OFFSET_TABLE_@PC_LO(-0x18)
 127      00680F06 
 128 0258 00000000 		and	%got,%got,(32)0
 128      608F0F44 
 129 0260 00000000 		sic	%plt
 129      00001028 
 130 0268 00000000 		lea.sl	%got,_GLOBAL_OFFSET_TABLE_@PC_HI(%plt,%got)
 130      8F908F06 
 131 0270 00000000 		or	%fp,0,%sp
 131      8B000945 
 132              		.cfi_def_cfa_register	9
 133 0278 30000000 		st	%s18,48(,%fp)
 133      89001211 
 134 0280 38000000 		st	%s19,56(,%fp)
 134      89001311 
 135 0288 40000000 		st	%s20,64(,%fp)
 135      89001411 
 136 0290 48000000 		st	%s21,72(,%fp)
 136      89001511 
 137 0298 50000000 		st	%s22,80(,%fp)
 137      89001611 
 138 02a0 58000000 		st	%s23,88(,%fp)
 138      89001711 
 139 02a8 60000000 		st	%s24,96(,%fp)
 139      89001811 
 140 02b0 68000000 		st	%s25,104(,%fp)
 140      89001911 
 141 02b8 70000000 		st	%s26,112(,%fp)
 141      89001A11 
 142 02c0 78000000 		st	%s27,120(,%fp)
 142      89001B11 
 143 02c8 80000000 		st	%s28,128(,%fp)
 143      89001C11 
 144 02d0 88000000 		st	%s29,136(,%fp)
 144      89001D11 
 145 02d8 90000000 		st	%s30,144(,%fp)
 145      89001E11 
 146 02e0 98000000 		st	%s31,152(,%fp)
 146      89001F11 
 147 02e8 A0000000 		st	%s32,160(,%fp)
 147      89002011 
 148 02f0 A8000000 		st	%s33,168(,%fp)
 148      89002111 
 149 02f8 10FEFFFF 		lea	%s13,.L.2.2auto_size&0xffffffff
 149      00000D06 
 150 0300 00000000 		and	%s13,%s13,(32)0
 150      608D0D44 
 151 0308 FFFFFFFF 		lea.sl	%sp,.L.2.2auto_size>>32(%fp,%s13)
 151      8D898B06 
 152 0310 48000000 		brge.l.t	%sp,%sl,.L_2.EoP
 152      888B3518 
 153 0318 18000000 		ld	%s61,0x18(,%tp)
 153      8E003D01 
 154 0320 00000000 		or	%s62,0,%s0
 154      80003E45 
 155 0328 3B010000 		lea	%s63,0x13b
 155      00003F06 
 156 0330 00000000 		shm.l	%s63,0x0(%s61)
 156      BD033F31 
 157 0338 08000000 		shm.l	%sl,0x8(%s61)
 157      BD030831 
 158 0340 10000000 		shm.l	%sp,0x10(%s61)
 158      BD030B31 
 159 0348 00000000 		monc
 159      0000003F 
 160 0350 00000000 		or	%s0,0,%s62
 160      BE000045 
 161              	.L_2.EoP:
 162              	# End of prologue codes
 163 0358 00000000 		or	%s34,0,%s0
 163      80002245 
 164 0360 00000000 		or	%s35,0,%s1
 164      81002345 
 165 0368 B0000000 		st	%s35,176(0,%fp)	# ld
 165      89002311 
 166 0370 B0000000 		st	%s34,176(8,%fp)	# ld
 166      89082211 
 167              	# line 11
  11:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ldbl_fn_ldbl.c **** 	long double *p_ret = &ldbl_ret;
 168              		.loc	1 11 0
 169 0378 00000000 		lea	%s63,ldbl_ret@GOTOFF_LO
 169      00003F06 
 170 0380 00000000 		and	%s63,%s63,(32)0
 170      60BF3F44 
 171 0388 00000000 		lea.sl	%s63,ldbl_ret@GOTOFF_HI(%s63,%got)
 171      8FBFBF06 
 172              	# line 12
  12:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ldbl_fn_ldbl.c **** 	*p_ret = ext_ldbl_fn_ldbl(ld);
 173              		.loc	1 12 0
 174 0390 00000000 		or	%s62,0,%s34
 174      A2003E45 
 175 0398 00000000 		or	%s61,0,%s35
 175      A3003D45 
 176 03a0 00000000 		or	%s0,0,%s62
 176      BE000045 
 177 03a8 00000000 		or	%s1,0,%s61
 177      BD000145 
 178 03b0 F8FFFFFF 		st	%s63,-8(,%fp)	# spill
 178      89003F11 
 179 03b8 00000000 		lea	%s12,ext_ldbl_fn_ldbl@PLT_LO(-24)
 179      00680C06 
 180 03c0 00000000 		and	%s12,%s12,(32)0
 180      608C0C44 
 181 03c8 00000000 		sic	%lr
 181      00000A28 
 182 03d0 00000000 		lea.sl	%s12,ext_ldbl_fn_ldbl@PLT_HI(%s12,%lr)
 182      8A8C8C06 
 183 03d8 00000000 		bsic	%lr,(,%s12)		# ext_ldbl_fn_ldbl
 183      8C000A08 
 184 03e0 E8FFFFFF 		st	%s0,-24(8,%fp)	# spill
 184      89080011 
 185 03e8 E8FFFFFF 		st	%s1,-24(,%fp)	# spill
 185      89000111 
 186 03f0 08000000 		br.l	.L_2.1
 186      00000F18 
 187              	.L_2.1:
 188 03f8 F8FFFFFF 		ld	%s63,-8(,%fp)	# restore
 188      89003F01 
 189 0400 E8FFFFFF 		ld	%s34,-24(8,%fp)	# restore
 189      89082201 
 190 0408 E8FFFFFF 		ld	%s35,-24(,%fp)	# restore
 190      89002301 
 191 0410 00000000 		st	%s35,0(0,%s63)	# *(p_ret)
 191      BF002311 
 192 0418 00000000 		st	%s34,0(8,%s63)	# *(p_ret)
 192      BF082211 
 193              	# line 13
  13:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ldbl_fn_ldbl.c **** 	return *p_ret;
 194              		.loc	1 13 0
 195 0420 00000000 		or	%s62,0,%s34
 195      A2003E45 
 196 0428 00000000 		or	%s61,0,%s35
 196      A3003D45 
 197 0430 00000000 		or	%s0,0,%s62
 197      BE000045 
 198 0438 00000000 		or	%s1,0,%s61
 198      BD000145 
 199              	# Start of epilogue codes
 200 0440 00000000 		or	%sp,0,%fp
 200      89000B45 
 201              		.cfi_def_cfa	11,8
 202 0448 18000000 		ld	%got,0x18(,%sp)
 202      8B000F01 
 203 0450 20000000 		ld	%plt,0x20(,%sp)
 203      8B001001 
 204 0458 08000000 		ld	%lr,0x8(,%sp)
 204      8B000A01 
 205 0460 00000000 		ld	%fp,0x0(,%sp)
 205      8B000901 
 206 0468 00000000 		b.l	(,%lr)
 206      8A000F19 
 207              	.L_2.EoE:
 208              		.cfi_endproc
 209              		.set	.L.2.2auto_size,	0xfffffffffffffe10	# 496 Bytes
 211              	# ============ End  ldbl_fn_ldbl2 ============
 212              	# ============ Begin  mt_ldbl_fn_ldbl ============
 213              		.balign 16
 214              	.L_3.0:
 215              	# line 15
  14:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ldbl_fn_ldbl.c **** }
  15:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ldbl_fn_ldbl.c **** long double mt_ldbl_fn_ldbl(long double ld) {
 216              		.loc	1 15 0
 217              		.globl	mt_ldbl_fn_ldbl
 219              	mt_ldbl_fn_ldbl:
 220              		.cfi_startproc
 221 0470 00000000 		st	%fp,0x0(,%sp)
 221      8B000911 
 222              		.cfi_def_cfa_offset	0
 223              		.cfi_offset	9,0
 224 0478 08000000 		st	%lr,0x8(,%sp)
 224      8B000A11 
 225 0480 18000000 		st	%got,0x18(,%sp)
 225      8B000F11 
 226 0488 20000000 		st	%plt,0x20(,%sp)
 226      8B001011 
 227 0490 00000000 		lea	%got,_GLOBAL_OFFSET_TABLE_@PC_LO(-0x18)
 227      00680F06 
 228 0498 00000000 		and	%got,%got,(32)0
 228      608F0F44 
 229 04a0 00000000 		sic	%plt
 229      00001028 
 230 04a8 00000000 		lea.sl	%got,_GLOBAL_OFFSET_TABLE_@PC_HI(%plt,%got)
 230      8F908F06 
 231 04b0 00000000 		or	%fp,0,%sp
 231      8B000945 
 232              		.cfi_def_cfa_register	9
 233 04b8 30000000 		st	%s18,48(,%fp)
 233      89001211 
 234 04c0 38000000 		st	%s19,56(,%fp)
 234      89001311 
 235 04c8 40000000 		st	%s20,64(,%fp)
 235      89001411 
 236 04d0 48000000 		st	%s21,72(,%fp)
 236      89001511 
 237 04d8 50000000 		st	%s22,80(,%fp)
 237      89001611 
 238 04e0 58000000 		st	%s23,88(,%fp)
 238      89001711 
 239 04e8 60000000 		st	%s24,96(,%fp)
 239      89001811 
 240 04f0 68000000 		st	%s25,104(,%fp)
 240      89001911 
 241 04f8 70000000 		st	%s26,112(,%fp)
 241      89001A11 
 242 0500 78000000 		st	%s27,120(,%fp)
 242      89001B11 
 243 0508 80000000 		st	%s28,128(,%fp)
 243      89001C11 
 244 0510 88000000 		st	%s29,136(,%fp)
 244      89001D11 
 245 0518 90000000 		st	%s30,144(,%fp)
 245      89001E11 
 246 0520 98000000 		st	%s31,152(,%fp)
 246      89001F11 
 247 0528 A0000000 		st	%s32,160(,%fp)
 247      89002011 
 248 0530 A8000000 		st	%s33,168(,%fp)
 248      89002111 
 249 0538 F0FFFFFF 		lea	%s13,.L.3.2auto_size&0xffffffff
 249      00000D06 
 250 0540 00000000 		and	%s13,%s13,(32)0
 250      608D0D44 
 251 0548 FFFFFFFF 		lea.sl	%sp,.L.3.2auto_size>>32(%fp,%s13)
 251      8D898B06 
 252 0550 48000000 		brge.l.t	%sp,%sl,.L_3.EoP
 252      888B3518 
 253 0558 18000000 		ld	%s61,0x18(,%tp)
 253      8E003D01 
 254 0560 00000000 		or	%s62,0,%s0
 254      80003E45 
 255 0568 3B010000 		lea	%s63,0x13b
 255      00003F06 
 256 0570 00000000 		shm.l	%s63,0x0(%s61)
 256      BD033F31 
 257 0578 08000000 		shm.l	%sl,0x8(%s61)
 257      BD030831 
 258 0580 10000000 		shm.l	%sp,0x10(%s61)
 258      BD030B31 
 259 0588 00000000 		monc
 259      0000003F 
 260 0590 00000000 		or	%s0,0,%s62
 260      BE000045 
 261              	.L_3.EoP:
 262              	# End of prologue codes
 263 0598 00000000 		or	%s34,0,%s0
 263      80002245 
 264 05a0 00000000 		or	%s35,0,%s1
 264      81002345 
 265 05a8 B0000000 		st	%s35,176(0,%fp)	# ld
 265      89002311 
 266 05b0 B0000000 		st	%s34,176(8,%fp)	# ld
 266      89082211 
 267              	# line 16
  16:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ldbl_fn_ldbl.c **** 	long double ret = ld+1.49;
 268              		.loc	1 16 0
 269 05b8 3D0AD7A3 		lea	%s63,-1546188227
 269      00003F06 
 270 05c0 00000000 		and	%s62,%s63,(32)0
 270      60BF3E44 
 271 05c8 707DFF3F 		lea.sl	%s61,1073708400(,%s62)
 271      BE00BD06 
 272 05d0 00000070 		lea.sl	%s60,1879048192
 272      0000BC06 
 273 05d8 00000000 		or	%s36,0,%s61
 273      BD002445 
 274 05e0 00000000 		or	%s37,0,%s60
 274      BC002545 
 275 05e8 00000000 		fadd.q	%s38,%s34,%s36
 275      A4A2266C 
 276 05f0 F0FFFFFF 		st	%s39,-16(0,%fp)	# ret
 276      89002711 
 277 05f8 F0FFFFFF 		st	%s38,-16(8,%fp)	# ret
 277      89082611 
 278              	# line 17
  17:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ldbl_fn_ldbl.c **** 	return ret;
 279              		.loc	1 17 0
 280 0600 00000000 		or	%s59,0,%s38
 280      A6003B45 
 281 0608 00000000 		or	%s58,0,%s39
 281      A7003A45 
 282 0610 00000000 		or	%s0,0,%s59
 282      BB000045 
 283 0618 00000000 		or	%s1,0,%s58
 283      BA000145 
 284              	# Start of epilogue codes
 285 0620 00000000 		or	%sp,0,%fp
 285      89000B45 
 286              		.cfi_def_cfa	11,8
 287 0628 18000000 		ld	%got,0x18(,%sp)
 287      8B000F01 
 288 0630 20000000 		ld	%plt,0x20(,%sp)
 288      8B001001 
 289 0638 08000000 		ld	%lr,0x8(,%sp)
 289      8B000A01 
 290 0640 00000000 		ld	%fp,0x0(,%sp)
 290      8B000901 
 291 0648 00000000 		b.l	(,%lr)
 291      8A000F19 
 292              	.L_3.EoE:
 293              		.cfi_endproc
 294              		.set	.L.3.2auto_size,	0xfffffffffffffff0	# 16 Bytes
 296              	# ============ End  mt_ldbl_fn_ldbl ============
 297              	# ============ Begin  prt_ldbl_fn_ldbl ============
 298              		.section .rodata
 299              		.balign 16
 301              	.LP.__string.0:
 302 0000 25       		.byte	37
 303 0001 73       		.byte	115
 304 0002 3A       		.byte	58
 305 0003 25       		.byte	37
 306 0004 75       		.byte	117
 307 0005 20       		.byte	32
 308 0006 6C       		.byte	108
 309 0007 64       		.byte	100
 310 0008 20       		.byte	32
 311 0009 25       		.byte	37
 312 000a 66       		.byte	102
 313 000b 0A       		.byte	10
 314 000c 00       		.zero	1
 315 000d 000000   		.balign 8
 317              	.LP.__string.1:
 318 0010 2F       		.byte	47
 319 0011 75       		.byte	117
 320 0012 73       		.byte	115
 321 0013 72       		.byte	114
 322 0014 2F       		.byte	47
 323 0015 75       		.byte	117
 324 0016 68       		.byte	104
 325 0017 6F       		.byte	111
 326 0018 6D       		.byte	109
 327 0019 65       		.byte	101
 328 001a 2F       		.byte	47
 329 001b 61       		.byte	97
 330 001c 75       		.byte	117
 331 001d 72       		.byte	114
 332 001e 6F       		.byte	111
 333 001f 72       		.byte	114
 334 0020 61       		.byte	97
 335 0021 2F       		.byte	47
 336 0022 34       		.byte	52
 337 0023 67       		.byte	103
 338 0024 69       		.byte	105
 339 0025 2F       		.byte	47
 340 0026 6E       		.byte	110
 341 0027 6C       		.byte	108
 342 0028 61       		.byte	97
 343 0029 62       		.byte	98
 344 002a 68       		.byte	104
 345 002b 70       		.byte	112
 346 002c 67       		.byte	103
 347 002d 2F       		.byte	47
 348 002e 6B       		.byte	107
 349 002f 72       		.byte	114
 350 0030 75       		.byte	117
 351 0031 75       		.byte	117
 352 0032 73       		.byte	115
 353 0033 2F       		.byte	47
 354 0034 76       		.byte	118
 355 0035 74       		.byte	116
 356 0036 2F       		.byte	47
 357 0037 73       		.byte	115
 358 0038 72       		.byte	114
 359 0039 63       		.byte	99
 360 003a 2F       		.byte	47
 361 003b 61       		.byte	97
 362 003c 73       		.byte	115
 363 003d 6D       		.byte	109
 364 003e 2D       		.byte	45
 365 003f 65       		.byte	101
 366 0040 78       		.byte	120
 367 0041 61       		.byte	97
 368 0042 6D       		.byte	109
 369 0043 70       		.byte	112
 370 0044 6C       		.byte	108
 371 0045 65       		.byte	101
 372 0046 73       		.byte	115
 373 0047 2F       		.byte	47
 374 0048 6C       		.byte	108
 375 0049 64       		.byte	100
 376 004a 62       		.byte	98
 377 004b 6C       		.byte	108
 378 004c 5F       		.byte	95
 379 004d 66       		.byte	102
 380 004e 6E       		.byte	110
 381 004f 5F       		.byte	95
 382 0050 6C       		.byte	108
 383 0051 64       		.byte	100
 384 0052 62       		.byte	98
 385 0053 6C       		.byte	108
 386 0054 2E       		.byte	46
 387 0055 63       		.byte	99
 388 0056 00       		.zero	1
 389              		.text
 390              		.balign 16
 391              	.L_4.0:
 392              	# line 19
  18:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ldbl_fn_ldbl.c **** }
  19:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ldbl_fn_ldbl.c **** long double prt_ldbl_fn_ldbl(long double ld) {
 393              		.loc	1 19 0
 394              		.globl	prt_ldbl_fn_ldbl
 396              	prt_ldbl_fn_ldbl:
 397              		.cfi_startproc
 398 0650 00000000 		st	%fp,0x0(,%sp)
 398      8B000911 
 399              		.cfi_def_cfa_offset	0
 400              		.cfi_offset	9,0
 401 0658 08000000 		st	%lr,0x8(,%sp)
 401      8B000A11 
 402 0660 18000000 		st	%got,0x18(,%sp)
 402      8B000F11 
 403 0668 20000000 		st	%plt,0x20(,%sp)
 403      8B001011 
 404 0670 00000000 		lea	%got,_GLOBAL_OFFSET_TABLE_@PC_LO(-0x18)
 404      00680F06 
 405 0678 00000000 		and	%got,%got,(32)0
 405      608F0F44 
 406 0680 00000000 		sic	%plt
 406      00001028 
 407 0688 00000000 		lea.sl	%got,_GLOBAL_OFFSET_TABLE_@PC_HI(%plt,%got)
 407      8F908F06 
 408 0690 00000000 		or	%fp,0,%sp
 408      8B000945 
 409              		.cfi_def_cfa_register	9
 410 0698 30000000 		st	%s18,48(,%fp)
 410      89001211 
 411 06a0 38000000 		st	%s19,56(,%fp)
 411      89001311 
 412 06a8 40000000 		st	%s20,64(,%fp)
 412      89001411 
 413 06b0 48000000 		st	%s21,72(,%fp)
 413      89001511 
 414 06b8 50000000 		st	%s22,80(,%fp)
 414      89001611 
 415 06c0 58000000 		st	%s23,88(,%fp)
 415      89001711 
 416 06c8 60000000 		st	%s24,96(,%fp)
 416      89001811 
 417 06d0 68000000 		st	%s25,104(,%fp)
 417      89001911 
 418 06d8 70000000 		st	%s26,112(,%fp)
 418      89001A11 
 419 06e0 78000000 		st	%s27,120(,%fp)
 419      89001B11 
 420 06e8 80000000 		st	%s28,128(,%fp)
 420      89001C11 
 421 06f0 88000000 		st	%s29,136(,%fp)
 421      89001D11 
 422 06f8 90000000 		st	%s30,144(,%fp)
 422      89001E11 
 423 0700 98000000 		st	%s31,152(,%fp)
 423      89001F11 
 424 0708 A0000000 		st	%s32,160(,%fp)
 424      89002011 
 425 0710 A8000000 		st	%s33,168(,%fp)
 425      89002111 
 426 0718 20FEFFFF 		lea	%s13,.L.4.2auto_size&0xffffffff
 426      00000D06 
 427 0720 00000000 		and	%s13,%s13,(32)0
 427      608D0D44 
 428 0728 FFFFFFFF 		lea.sl	%sp,.L.4.2auto_size>>32(%fp,%s13)
 428      8D898B06 
 429 0730 48000000 		brge.l.t	%sp,%sl,.L_4.EoP
 429      888B3518 
 430 0738 18000000 		ld	%s61,0x18(,%tp)
 430      8E003D01 
 431 0740 00000000 		or	%s62,0,%s0
 431      80003E45 
 432 0748 3B010000 		lea	%s63,0x13b
 432      00003F06 
 433 0750 00000000 		shm.l	%s63,0x0(%s61)
 433      BD033F31 
 434 0758 08000000 		shm.l	%sl,0x8(%s61)
 434      BD030831 
 435 0760 10000000 		shm.l	%sp,0x10(%s61)
 435      BD030B31 
 436 0768 00000000 		monc
 436      0000003F 
 437 0770 00000000 		or	%s0,0,%s62
 437      BE000045 
 438              	.L_4.EoP:
 439              	# End of prologue codes
 440 0778 00000000 		or	%s34,0,%s0
 440      80002245 
 441 0780 00000000 		or	%s35,0,%s1
 441      81002345 
 442 0788 B0000000 		st	%s35,176(0,%fp)	# ld
 442      89002311 
 443 0790 B0000000 		st	%s34,176(8,%fp)	# ld
 443      89082211 
 444              	# line 20
  20:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ldbl_fn_ldbl.c **** 	long double ret = ld + 1.49;
 445              		.loc	1 20 0
 446 0798 3D0AD7A3 		lea	%s63,-1546188227
 446      00003F06 
 447 07a0 00000000 		and	%s62,%s63,(32)0
 447      60BF3E44 
 448 07a8 707DFF3F 		lea.sl	%s61,1073708400(,%s62)
 448      BE00BD06 
 449 07b0 00000070 		lea.sl	%s60,1879048192
 449      0000BC06 
 450 07b8 00000000 		or	%s36,0,%s61
 450      BD002445 
 451 07c0 00000000 		or	%s37,0,%s60
 451      BC002545 
 452 07c8 00000000 		fadd.q	%s38,%s34,%s36
 452      A4A2266C 
 453              	# line 21
  21:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ldbl_fn_ldbl.c **** 	printf("%s:%u ld %f\n",__FILE__,__LINE__,(double)ret);
 454              		.loc	1 21 0
 455 07d0 00000000 		cvt.d.q %s59,%s38
 455      00A6BB0F 
 456 07d8 C8000000 		st	%s59,200(0,%sp)
 456      8B003B11 
 457              	# line 20
  20:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ldbl_fn_ldbl.c **** 	long double ret = ld + 1.49;
 458              		.loc	1 20 0
 459 07e0 F0FFFFFF 		st	%s39,-16(0,%fp)	# ret
 459      89002711 
 460 07e8 F0FFFFFF 		st	%s38,-16(8,%fp)	# ret
 460      89082611 
 461              	# line 21
 462              		.loc	1 21 0
 463 07f0 00000000 		lea	%s58,.LP.__string.0@GOTOFF_LO
 463      00003A06 
 464 07f8 00000000 		and	%s58,%s58,(32)0
 464      60BA3A44 
 465 0800 00000000 		lea.sl	%s58,.LP.__string.0@GOTOFF_HI(%s58,%got)
 465      8FBABA06 
 466 0808 B0000000 		st	%s58,176(0,%sp)
 466      8B003A11 
 467 0810 00000000 		lea	%s57,.LP.__string.1@GOTOFF_LO
 467      00003906 
 468 0818 00000000 		and	%s57,%s57,(32)0
 468      60B93944 
 469 0820 00000000 		lea.sl	%s57,.LP.__string.1@GOTOFF_HI(%s57,%got)
 469      8FB9B906 
 470 0828 B8000000 		st	%s57,184(0,%sp)
 470      8B003911 
 471 0830 00000000 		or	%s56,21,(0)1
 471      00153845 
 472 0838 C0000000 		st	%s56,192(0,%sp)
 472      8B003811 
 473 0840 00000000 		or	%s0,0,%s58
 473      BA000045 
 474 0848 00000000 		or	%s1,0,%s57
 474      B9000145 
 475 0850 00000000 		or	%s2,0,%s56
 475      B8000245 
 476 0858 00000000 		or	%s3,0,%s59
 476      BB000345 
 477 0860 00000000 		lea	%s12,printf@PLT_LO(-24)
 477      00680C06 
 478 0868 00000000 		and	%s12,%s12,(32)0
 478      608C0C44 
 479 0870 00000000 		sic	%lr
 479      00000A28 
 480 0878 00000000 		lea.sl	%s12,printf@PLT_HI(%s12,%lr)
 480      8A8C8C06 
 481 0880 00000000 		bsic	%lr,(,%s12)		# printf
 481      8C000A08 
 482 0888 08000000 		br.l	.L_4.1
 482      00000F18 
 483              	.L_4.1:
 484              	# line 22
  22:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ldbl_fn_ldbl.c **** 	return ret;
 485              		.loc	1 22 0
 486 0890 F0FFFFFF 		ld	%s35,-16(0,%fp)	# ret
 486      89002301 
 487 0898 F0FFFFFF 		ld	%s34,-16(8,%fp)	# ret
 487      89082201 
 488 08a0 00000000 		or	%s63,0,%s34
 488      A2003F45 
 489 08a8 00000000 		or	%s62,0,%s35
 489      A3003E45 
 490 08b0 00000000 		or	%s0,0,%s63
 490      BF000045 
 491 08b8 00000000 		or	%s1,0,%s62
 491      BE000145 
 492              	# Start of epilogue codes
 493 08c0 00000000 		or	%sp,0,%fp
 493      89000B45 
 494              		.cfi_def_cfa	11,8
 495 08c8 18000000 		ld	%got,0x18(,%sp)
 495      8B000F01 
 496 08d0 20000000 		ld	%plt,0x20(,%sp)
 496      8B001001 
 497 08d8 08000000 		ld	%lr,0x8(,%sp)
 497      8B000A01 
 498 08e0 00000000 		ld	%fp,0x0(,%sp)
 498      8B000901 
 499 08e8 00000000 		b.l	(,%lr)
 499      8A000F19 
 500              	.L_4.EoE:
 501              		.cfi_endproc
 502              		.set	.L.4.2auto_size,	0xfffffffffffffe20	# 480 Bytes
 504              	# ============ End  prt_ldbl_fn_ldbl ============
 505              	.Le1.0:
