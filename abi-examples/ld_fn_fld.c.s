   1              		.ident "ncc 1.1.4 (Build 10:17:22 Apr 19 2018)"
   2              		.file "ld_fn_fld.c"
   3              		.file 1 "/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ld_fn_fld.c"
   4              		.file 2 "/opt/nec/ve/ncc/1.1.4/include/stdc-predef.h"
   5              		.file 3 "/opt/nec/ve/ncc/1.1.4/include/stdio.h"
   6              		.file 4 "/opt/nec/ve/ncc/1.1.4/include/yvals.h"
   7              		.file 5 "/opt/nec/ve/ncc/1.1.4/include/necvals.h"
   8              		.file 6 "/opt/nec/ve/ncc/1.1.4/include/stdarg.h"
   9              		.file 7 "/opt/nec/ve/musl/include/stdio.h"
  10              		.file 8 "/opt/nec/ve/musl/include/features.h"
  11              		.file 9 "/opt/nec/ve/musl/include/bits/alltypes.h"
  12              	# ============ Begin  ld_fn_fld ============
  13              		.section	.bss
  14              		.local	ldbl_ret
  15              	.comm ldbl_ret,16,16
  16              		.text
  17              		.balign 16
  18              	.L_1.0:
  19              	# line 6
   1:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ld_fn_fld.c **** #include <stdio.h>
   2:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ld_fn_fld.c **** 
   3:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ld_fn_fld.c **** extern long double ext_ld_fn_fld(float f, long double ld);
   4:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ld_fn_fld.c **** 
   5:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ld_fn_fld.c **** static long double ldbl_ret;
   6:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ld_fn_fld.c **** long double ld_fn_fld(float f, long double ld) {
  20              		.loc	1 6 0
  21              		.globl	ld_fn_fld
  23              	ld_fn_fld:
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
  53 00c8 00FEFFFF 		lea	%s13,.L.1.2auto_size&0xffffffff
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
  67 0128 00000000 		or	%s34,0,%s2
  67      82002245 
  68 0130 00000000 		or	%s35,0,%s3
  68      83002345 
  69              	# line 7
   7:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ld_fn_fld.c **** 	ldbl_ret = ext_ld_fn_fld(f+f, ld+ld);
  70              		.loc	1 7 0
  71 0138 00000000 		fadd.q	%s36,%s34,%s34
  71      A2A2246C 
  72 0140 00000000 		fadd.s	%s63,%s0,%s0
  72      8080BF4C 
  73              	# line 6
   6:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ld_fn_fld.c **** 	ldbl_ret = ext_ld_fn_fld(f+f, ld+ld);
  74              		.loc	1 6 0
  75 0148 C0000000 		st	%s35,192(0,%fp)	# ld
  75      89002311 
  76 0150 C0000000 		st	%s34,192(8,%fp)	# ld
  76      89082211 
  77              	# line 7
  78              		.loc	1 7 0
  79 0158 00000000 		or	%s62,0,%s36
  79      A4003E45 
  80 0160 00000000 		or	%s61,0,%s37
  80      A5003D45 
  81 0168 00000000 		or	%s0,0,%s63
  81      BF000045 
  82 0170 F8FFFFFF 		ld	%s1,-8(,%fp)	# restore
  82      89000101 
  83 0178 00000000 		or	%s2,0,%s62
  83      BE000245 
  84 0180 00000000 		or	%s3,0,%s61
  84      BD000345 
  85 0188 00000000 		lea	%s12,ext_ld_fn_fld@PLT_LO(-24)
  85      00680C06 
  86 0190 00000000 		and	%s12,%s12,(32)0
  86      608C0C44 
  87 0198 00000000 		sic	%lr
  87      00000A28 
  88 01a0 00000000 		lea.sl	%s12,ext_ld_fn_fld@PLT_HI(%s12,%lr)
  88      8A8C8C06 
  89 01a8 00000000 		bsic	%lr,(,%s12)		# ext_ld_fn_fld
  89      8C000A08 
  90 01b0 E8FFFFFF 		st	%s0,-24(8,%fp)	# spill
  90      89080011 
  91 01b8 E8FFFFFF 		st	%s1,-24(,%fp)	# spill
  91      89000111 
  92 01c0 08000000 		br.l	.L_1.1
  92      00000F18 
  93              	.L_1.1:
  94 01c8 00000000 		lea	%s63,ldbl_ret@GOTOFF_LO
  94      00003F06 
  95 01d0 00000000 		and	%s63,%s63,(32)0
  95      60BF3F44 
  96 01d8 00000000 		lea.sl	%s63,ldbl_ret@GOTOFF_HI(%s63,%got)
  96      8FBFBF06 
  97 01e0 E8FFFFFF 		ld	%s34,-24(8,%fp)	# restore
  97      89082201 
  98 01e8 E8FFFFFF 		ld	%s35,-24(,%fp)	# restore
  98      89002301 
  99 01f0 00000000 		st	%s35,0(0,%s63)	# ldbl_ret
  99      BF002311 
 100 01f8 00000000 		st	%s34,0(8,%s63)	# ldbl_ret
 100      BF082211 
 101              	# line 8
   8:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ld_fn_fld.c **** 	return ldbl_ret;
 102              		.loc	1 8 0
 103 0200 00000000 		or	%s62,0,%s34
 103      A2003E45 
 104 0208 00000000 		or	%s61,0,%s35
 104      A3003D45 
 105 0210 00000000 		or	%s0,0,%s62
 105      BE000045 
 106 0218 00000000 		or	%s1,0,%s61
 106      BD000145 
 107              	# Start of epilogue codes
 108 0220 00000000 		or	%sp,0,%fp
 108      89000B45 
 109              		.cfi_def_cfa	11,8
 110 0228 18000000 		ld	%got,0x18(,%sp)
 110      8B000F01 
 111 0230 20000000 		ld	%plt,0x20(,%sp)
 111      8B001001 
 112 0238 08000000 		ld	%lr,0x8(,%sp)
 112      8B000A01 
 113 0240 00000000 		ld	%fp,0x0(,%sp)
 113      8B000901 
 114 0248 00000000 		b.l	(,%lr)
 114      8A000F19 
 115              	.L_1.EoE:
 116              		.cfi_endproc
 117              		.set	.L.1.2auto_size,	0xfffffffffffffe00	# 512 Bytes
 119              	# ============ End  ld_fn_fld ============
 120              	# ============ Begin  alt_ld_fn_fld ============
 121              		.balign 16
 122              	.L_2.0:
 123              	# line 10
   9:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ld_fn_fld.c **** }
  10:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ld_fn_fld.c **** long double alt_ld_fn_fld(float f, long double ld) {
 124              		.loc	1 10 0
 125              		.globl	alt_ld_fn_fld
 127              	alt_ld_fn_fld:
 128              		.cfi_startproc
 129 0250 00000000 		st	%fp,0x0(,%sp)
 129      8B000911 
 130              		.cfi_def_cfa_offset	0
 131              		.cfi_offset	9,0
 132 0258 08000000 		st	%lr,0x8(,%sp)
 132      8B000A11 
 133 0260 18000000 		st	%got,0x18(,%sp)
 133      8B000F11 
 134 0268 20000000 		st	%plt,0x20(,%sp)
 134      8B001011 
 135 0270 00000000 		lea	%got,_GLOBAL_OFFSET_TABLE_@PC_LO(-0x18)
 135      00680F06 
 136 0278 00000000 		and	%got,%got,(32)0
 136      608F0F44 
 137 0280 00000000 		sic	%plt
 137      00001028 
 138 0288 00000000 		lea.sl	%got,_GLOBAL_OFFSET_TABLE_@PC_HI(%plt,%got)
 138      8F908F06 
 139 0290 00000000 		or	%fp,0,%sp
 139      8B000945 
 140              		.cfi_def_cfa_register	9
 141 0298 30000000 		st	%s18,48(,%fp)
 141      89001211 
 142 02a0 38000000 		st	%s19,56(,%fp)
 142      89001311 
 143 02a8 40000000 		st	%s20,64(,%fp)
 143      89001411 
 144 02b0 48000000 		st	%s21,72(,%fp)
 144      89001511 
 145 02b8 50000000 		st	%s22,80(,%fp)
 145      89001611 
 146 02c0 58000000 		st	%s23,88(,%fp)
 146      89001711 
 147 02c8 60000000 		st	%s24,96(,%fp)
 147      89001811 
 148 02d0 68000000 		st	%s25,104(,%fp)
 148      89001911 
 149 02d8 70000000 		st	%s26,112(,%fp)
 149      89001A11 
 150 02e0 78000000 		st	%s27,120(,%fp)
 150      89001B11 
 151 02e8 80000000 		st	%s28,128(,%fp)
 151      89001C11 
 152 02f0 88000000 		st	%s29,136(,%fp)
 152      89001D11 
 153 02f8 90000000 		st	%s30,144(,%fp)
 153      89001E11 
 154 0300 98000000 		st	%s31,152(,%fp)
 154      89001F11 
 155 0308 A0000000 		st	%s32,160(,%fp)
 155      89002011 
 156 0310 A8000000 		st	%s33,168(,%fp)
 156      89002111 
 157 0318 F0FDFFFF 		lea	%s13,.L.2.2auto_size&0xffffffff
 157      00000D06 
 158 0320 00000000 		and	%s13,%s13,(32)0
 158      608D0D44 
 159 0328 FFFFFFFF 		lea.sl	%sp,.L.2.2auto_size>>32(%fp,%s13)
 159      8D898B06 
 160 0330 48000000 		brge.l.t	%sp,%sl,.L_2.EoP
 160      888B3518 
 161 0338 18000000 		ld	%s61,0x18(,%tp)
 161      8E003D01 
 162 0340 00000000 		or	%s62,0,%s0
 162      80003E45 
 163 0348 3B010000 		lea	%s63,0x13b
 163      00003F06 
 164 0350 00000000 		shm.l	%s63,0x0(%s61)
 164      BD033F31 
 165 0358 08000000 		shm.l	%sl,0x8(%s61)
 165      BD030831 
 166 0360 10000000 		shm.l	%sp,0x10(%s61)
 166      BD030B31 
 167 0368 00000000 		monc
 167      0000003F 
 168 0370 00000000 		or	%s0,0,%s62
 168      BE000045 
 169              	.L_2.EoP:
 170              	# End of prologue codes
 171 0378 00000000 		or	%s34,0,%s2
 171      82002245 
 172 0380 00000000 		or	%s35,0,%s3
 172      83002345 
 173              	# line 12
  11:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ld_fn_fld.c **** 	long double *p_ret = &ldbl_ret;
  12:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ld_fn_fld.c **** 	*p_ret = ext_ld_fn_fld(f+f, ld+ld);
 174              		.loc	1 12 0
 175 0388 00000000 		fadd.q	%s36,%s34,%s34
 175      A2A2246C 
 176 0390 00000000 		fadd.s	%s63,%s0,%s0
 176      8080BF4C 
 177              	# line 10
  10:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ld_fn_fld.c **** 	long double *p_ret = &ldbl_ret;
 178              		.loc	1 10 0
 179 0398 C0000000 		st	%s35,192(0,%fp)	# ld
 179      89002311 
 180 03a0 C0000000 		st	%s34,192(8,%fp)	# ld
 180      89082211 
 181              	# line 11
  11:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ld_fn_fld.c **** 	long double *p_ret = &ldbl_ret;
 182              		.loc	1 11 0
 183 03a8 00000000 		lea	%s62,ldbl_ret@GOTOFF_LO
 183      00003E06 
 184 03b0 00000000 		and	%s62,%s62,(32)0
 184      60BE3E44 
 185 03b8 00000000 		lea.sl	%s62,ldbl_ret@GOTOFF_HI(%s62,%got)
 185      8FBEBE06 
 186              	# line 12
 187              		.loc	1 12 0
 188 03c0 00000000 		or	%s61,0,%s36
 188      A4003D45 
 189 03c8 00000000 		or	%s60,0,%s37
 189      A5003C45 
 190 03d0 00000000 		or	%s0,0,%s63
 190      BF000045 
 191 03d8 F8FFFFFF 		ld	%s1,-8(,%fp)	# restore
 191      89000101 
 192 03e0 00000000 		or	%s2,0,%s61
 192      BD000245 
 193 03e8 00000000 		or	%s3,0,%s60
 193      BC000345 
 194 03f0 F0FFFFFF 		st	%s62,-16(,%fp)	# spill
 194      89003E11 
 195 03f8 00000000 		lea	%s12,ext_ld_fn_fld@PLT_LO(-24)
 195      00680C06 
 196 0400 00000000 		and	%s12,%s12,(32)0
 196      608C0C44 
 197 0408 00000000 		sic	%lr
 197      00000A28 
 198 0410 00000000 		lea.sl	%s12,ext_ld_fn_fld@PLT_HI(%s12,%lr)
 198      8A8C8C06 
 199 0418 00000000 		bsic	%lr,(,%s12)		# ext_ld_fn_fld
 199      8C000A08 
 200 0420 E0FFFFFF 		st	%s0,-32(8,%fp)	# spill
 200      89080011 
 201 0428 E0FFFFFF 		st	%s1,-32(,%fp)	# spill
 201      89000111 
 202 0430 08000000 		br.l	.L_2.1
 202      00000F18 
 203              	.L_2.1:
 204 0438 F0FFFFFF 		ld	%s63,-16(,%fp)	# restore
 204      89003F01 
 205 0440 E0FFFFFF 		ld	%s34,-32(8,%fp)	# restore
 205      89082201 
 206 0448 E0FFFFFF 		ld	%s35,-32(,%fp)	# restore
 206      89002301 
 207 0450 00000000 		st	%s35,0(0,%s63)	# *(p_ret)
 207      BF002311 
 208 0458 00000000 		st	%s34,0(8,%s63)	# *(p_ret)
 208      BF082211 
 209              	# line 13
  13:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ld_fn_fld.c **** 	return *p_ret;
 210              		.loc	1 13 0
 211 0460 00000000 		or	%s62,0,%s34
 211      A2003E45 
 212 0468 00000000 		or	%s61,0,%s35
 212      A3003D45 
 213 0470 00000000 		or	%s0,0,%s62
 213      BE000045 
 214 0478 00000000 		or	%s1,0,%s61
 214      BD000145 
 215              	# Start of epilogue codes
 216 0480 00000000 		or	%sp,0,%fp
 216      89000B45 
 217              		.cfi_def_cfa	11,8
 218 0488 18000000 		ld	%got,0x18(,%sp)
 218      8B000F01 
 219 0490 20000000 		ld	%plt,0x20(,%sp)
 219      8B001001 
 220 0498 08000000 		ld	%lr,0x8(,%sp)
 220      8B000A01 
 221 04a0 00000000 		ld	%fp,0x0(,%sp)
 221      8B000901 
 222 04a8 00000000 		b.l	(,%lr)
 222      8A000F19 
 223              	.L_2.EoE:
 224              		.cfi_endproc
 225              		.set	.L.2.2auto_size,	0xfffffffffffffdf0	# 528 Bytes
 227              	# ============ End  alt_ld_fn_fld ============
 228              	# ============ Begin  mt_ld_fn_fld ============
 229              		.balign 16
 230              	.L_3.0:
 231              	# line 15
  14:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ld_fn_fld.c **** }
  15:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ld_fn_fld.c **** long double mt_ld_fn_fld(float f, long double ld) {
 232              		.loc	1 15 0
 233              		.globl	mt_ld_fn_fld
 235              	mt_ld_fn_fld:
 236              		.cfi_startproc
 237 04b0 00000000 		st	%fp,0x0(,%sp)
 237      8B000911 
 238              		.cfi_def_cfa_offset	0
 239              		.cfi_offset	9,0
 240 04b8 08000000 		st	%lr,0x8(,%sp)
 240      8B000A11 
 241 04c0 18000000 		st	%got,0x18(,%sp)
 241      8B000F11 
 242 04c8 20000000 		st	%plt,0x20(,%sp)
 242      8B001011 
 243 04d0 00000000 		lea	%got,_GLOBAL_OFFSET_TABLE_@PC_LO(-0x18)
 243      00680F06 
 244 04d8 00000000 		and	%got,%got,(32)0
 244      608F0F44 
 245 04e0 00000000 		sic	%plt
 245      00001028 
 246 04e8 00000000 		lea.sl	%got,_GLOBAL_OFFSET_TABLE_@PC_HI(%plt,%got)
 246      8F908F06 
 247 04f0 00000000 		or	%fp,0,%sp
 247      8B000945 
 248              		.cfi_def_cfa_register	9
 249 04f8 30000000 		st	%s18,48(,%fp)
 249      89001211 
 250 0500 38000000 		st	%s19,56(,%fp)
 250      89001311 
 251 0508 40000000 		st	%s20,64(,%fp)
 251      89001411 
 252 0510 48000000 		st	%s21,72(,%fp)
 252      89001511 
 253 0518 50000000 		st	%s22,80(,%fp)
 253      89001611 
 254 0520 58000000 		st	%s23,88(,%fp)
 254      89001711 
 255 0528 60000000 		st	%s24,96(,%fp)
 255      89001811 
 256 0530 68000000 		st	%s25,104(,%fp)
 256      89001911 
 257 0538 70000000 		st	%s26,112(,%fp)
 257      89001A11 
 258 0540 78000000 		st	%s27,120(,%fp)
 258      89001B11 
 259 0548 80000000 		st	%s28,128(,%fp)
 259      89001C11 
 260 0550 88000000 		st	%s29,136(,%fp)
 260      89001D11 
 261 0558 90000000 		st	%s30,144(,%fp)
 261      89001E11 
 262 0560 98000000 		st	%s31,152(,%fp)
 262      89001F11 
 263 0568 A0000000 		st	%s32,160(,%fp)
 263      89002011 
 264 0570 A8000000 		st	%s33,168(,%fp)
 264      89002111 
 265 0578 F0FFFFFF 		lea	%s13,.L.3.2auto_size&0xffffffff
 265      00000D06 
 266 0580 00000000 		and	%s13,%s13,(32)0
 266      608D0D44 
 267 0588 FFFFFFFF 		lea.sl	%sp,.L.3.2auto_size>>32(%fp,%s13)
 267      8D898B06 
 268 0590 48000000 		brge.l.t	%sp,%sl,.L_3.EoP
 268      888B3518 
 269 0598 18000000 		ld	%s61,0x18(,%tp)
 269      8E003D01 
 270 05a0 00000000 		or	%s62,0,%s0
 270      80003E45 
 271 05a8 3B010000 		lea	%s63,0x13b
 271      00003F06 
 272 05b0 00000000 		shm.l	%s63,0x0(%s61)
 272      BD033F31 
 273 05b8 08000000 		shm.l	%sl,0x8(%s61)
 273      BD030831 
 274 05c0 10000000 		shm.l	%sp,0x10(%s61)
 274      BD030B31 
 275 05c8 00000000 		monc
 275      0000003F 
 276 05d0 00000000 		or	%s0,0,%s62
 276      BE000045 
 277              	.L_3.EoP:
 278              	# End of prologue codes
 279 05d8 00000000 		or	%s34,0,%s2
 279      82002245 
 280 05e0 00000000 		or	%s35,0,%s3
 280      83002345 
 281              	# line 16
  16:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ld_fn_fld.c **** 	long double ret = f + ld;
 282              		.loc	1 16 0
 283 05e8 00000000 		cvt.q.s %s36,%s0
 283      0080A42D 
 284 05f0 00000000 		fadd.q	%s38,%s34,%s36
 284      A4A2266C 
 285              	# line 15
  15:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ld_fn_fld.c **** 	long double ret = f + ld;
 286              		.loc	1 15 0
 287 05f8 C0000000 		st	%s35,192(0,%fp)	# ld
 287      89002311 
 288 0600 C0000000 		st	%s34,192(8,%fp)	# ld
 288      89082211 
 289              	# line 16
 290              		.loc	1 16 0
 291 0608 F0FFFFFF 		st	%s39,-16(0,%fp)	# ret
 291      89002711 
 292 0610 F0FFFFFF 		st	%s38,-16(8,%fp)	# ret
 292      89082611 
 293              	# line 17
  17:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ld_fn_fld.c **** 	return ret;
 294              		.loc	1 17 0
 295 0618 00000000 		or	%s63,0,%s38
 295      A6003F45 
 296 0620 00000000 		or	%s62,0,%s39
 296      A7003E45 
 297 0628 00000000 		or	%s0,0,%s63
 297      BF000045 
 298 0630 00000000 		or	%s1,0,%s62
 298      BE000145 
 299              	# Start of epilogue codes
 300 0638 00000000 		or	%sp,0,%fp
 300      89000B45 
 301              		.cfi_def_cfa	11,8
 302 0640 18000000 		ld	%got,0x18(,%sp)
 302      8B000F01 
 303 0648 20000000 		ld	%plt,0x20(,%sp)
 303      8B001001 
 304 0650 08000000 		ld	%lr,0x8(,%sp)
 304      8B000A01 
 305 0658 00000000 		ld	%fp,0x0(,%sp)
 305      8B000901 
 306 0660 00000000 		b.l	(,%lr)
 306      8A000F19 
 307              	.L_3.EoE:
 308              		.cfi_endproc
 309              		.set	.L.3.2auto_size,	0xfffffffffffffff0	# 16 Bytes
 311              	# ============ End  mt_ld_fn_fld ============
 312              	# ============ Begin  prt_ld_fn_fld ============
 313              		.section .rodata
 314              		.balign 16
 316              	.LP.__string.0:
 317 0000 25       		.byte	37
 318 0001 73       		.byte	115
 319 0002 3A       		.byte	58
 320 0003 25       		.byte	37
 321 0004 75       		.byte	117
 322 0005 20       		.byte	32
 323 0006 6C       		.byte	108
 324 0007 64       		.byte	100
 325 0008 20       		.byte	32
 326 0009 25       		.byte	37
 327 000a 66       		.byte	102
 328 000b 0A       		.byte	10
 329 000c 00       		.zero	1
 330 000d 000000   		.balign 8
 332              	.LP.__string.1:
 333 0010 2F       		.byte	47
 334 0011 75       		.byte	117
 335 0012 73       		.byte	115
 336 0013 72       		.byte	114
 337 0014 2F       		.byte	47
 338 0015 75       		.byte	117
 339 0016 68       		.byte	104
 340 0017 6F       		.byte	111
 341 0018 6D       		.byte	109
 342 0019 65       		.byte	101
 343 001a 2F       		.byte	47
 344 001b 61       		.byte	97
 345 001c 75       		.byte	117
 346 001d 72       		.byte	114
 347 001e 6F       		.byte	111
 348 001f 72       		.byte	114
 349 0020 61       		.byte	97
 350 0021 2F       		.byte	47
 351 0022 34       		.byte	52
 352 0023 67       		.byte	103
 353 0024 69       		.byte	105
 354 0025 2F       		.byte	47
 355 0026 6E       		.byte	110
 356 0027 6C       		.byte	108
 357 0028 61       		.byte	97
 358 0029 62       		.byte	98
 359 002a 68       		.byte	104
 360 002b 70       		.byte	112
 361 002c 67       		.byte	103
 362 002d 2F       		.byte	47
 363 002e 6B       		.byte	107
 364 002f 72       		.byte	114
 365 0030 75       		.byte	117
 366 0031 75       		.byte	117
 367 0032 73       		.byte	115
 368 0033 2F       		.byte	47
 369 0034 76       		.byte	118
 370 0035 74       		.byte	116
 371 0036 2F       		.byte	47
 372 0037 73       		.byte	115
 373 0038 72       		.byte	114
 374 0039 63       		.byte	99
 375 003a 2F       		.byte	47
 376 003b 61       		.byte	97
 377 003c 73       		.byte	115
 378 003d 6D       		.byte	109
 379 003e 2D       		.byte	45
 380 003f 65       		.byte	101
 381 0040 78       		.byte	120
 382 0041 61       		.byte	97
 383 0042 6D       		.byte	109
 384 0043 70       		.byte	112
 385 0044 6C       		.byte	108
 386 0045 65       		.byte	101
 387 0046 73       		.byte	115
 388 0047 2F       		.byte	47
 389 0048 6C       		.byte	108
 390 0049 64       		.byte	100
 391 004a 5F       		.byte	95
 392 004b 66       		.byte	102
 393 004c 6E       		.byte	110
 394 004d 5F       		.byte	95
 395 004e 66       		.byte	102
 396 004f 6C       		.byte	108
 397 0050 64       		.byte	100
 398 0051 2E       		.byte	46
 399 0052 63       		.byte	99
 400 0053 00       		.zero	1
 401              		.text
 402 0668 00000000 		.balign 16
 402      00000000 
 403              	.L_4.0:
 404              	# line 19
  18:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ld_fn_fld.c **** }
  19:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ld_fn_fld.c **** long double prt_ld_fn_fld(float f, long double ld) {
 405              		.loc	1 19 0
 406              		.globl	prt_ld_fn_fld
 408              	prt_ld_fn_fld:
 409              		.cfi_startproc
 410 0670 00000000 		st	%fp,0x0(,%sp)
 410      8B000911 
 411              		.cfi_def_cfa_offset	0
 412              		.cfi_offset	9,0
 413 0678 08000000 		st	%lr,0x8(,%sp)
 413      8B000A11 
 414 0680 18000000 		st	%got,0x18(,%sp)
 414      8B000F11 
 415 0688 20000000 		st	%plt,0x20(,%sp)
 415      8B001011 
 416 0690 00000000 		lea	%got,_GLOBAL_OFFSET_TABLE_@PC_LO(-0x18)
 416      00680F06 
 417 0698 00000000 		and	%got,%got,(32)0
 417      608F0F44 
 418 06a0 00000000 		sic	%plt
 418      00001028 
 419 06a8 00000000 		lea.sl	%got,_GLOBAL_OFFSET_TABLE_@PC_HI(%plt,%got)
 419      8F908F06 
 420 06b0 00000000 		or	%fp,0,%sp
 420      8B000945 
 421              		.cfi_def_cfa_register	9
 422 06b8 30000000 		st	%s18,48(,%fp)
 422      89001211 
 423 06c0 38000000 		st	%s19,56(,%fp)
 423      89001311 
 424 06c8 40000000 		st	%s20,64(,%fp)
 424      89001411 
 425 06d0 48000000 		st	%s21,72(,%fp)
 425      89001511 
 426 06d8 50000000 		st	%s22,80(,%fp)
 426      89001611 
 427 06e0 58000000 		st	%s23,88(,%fp)
 427      89001711 
 428 06e8 60000000 		st	%s24,96(,%fp)
 428      89001811 
 429 06f0 68000000 		st	%s25,104(,%fp)
 429      89001911 
 430 06f8 70000000 		st	%s26,112(,%fp)
 430      89001A11 
 431 0700 78000000 		st	%s27,120(,%fp)
 431      89001B11 
 432 0708 80000000 		st	%s28,128(,%fp)
 432      89001C11 
 433 0710 88000000 		st	%s29,136(,%fp)
 433      89001D11 
 434 0718 90000000 		st	%s30,144(,%fp)
 434      89001E11 
 435 0720 98000000 		st	%s31,152(,%fp)
 435      89001F11 
 436 0728 A0000000 		st	%s32,160(,%fp)
 436      89002011 
 437 0730 A8000000 		st	%s33,168(,%fp)
 437      89002111 
 438 0738 20FEFFFF 		lea	%s13,.L.4.2auto_size&0xffffffff
 438      00000D06 
 439 0740 00000000 		and	%s13,%s13,(32)0
 439      608D0D44 
 440 0748 FFFFFFFF 		lea.sl	%sp,.L.4.2auto_size>>32(%fp,%s13)
 440      8D898B06 
 441 0750 48000000 		brge.l.t	%sp,%sl,.L_4.EoP
 441      888B3518 
 442 0758 18000000 		ld	%s61,0x18(,%tp)
 442      8E003D01 
 443 0760 00000000 		or	%s62,0,%s0
 443      80003E45 
 444 0768 3B010000 		lea	%s63,0x13b
 444      00003F06 
 445 0770 00000000 		shm.l	%s63,0x0(%s61)
 445      BD033F31 
 446 0778 08000000 		shm.l	%sl,0x8(%s61)
 446      BD030831 
 447 0780 10000000 		shm.l	%sp,0x10(%s61)
 447      BD030B31 
 448 0788 00000000 		monc
 448      0000003F 
 449 0790 00000000 		or	%s0,0,%s62
 449      BE000045 
 450              	.L_4.EoP:
 451              	# End of prologue codes
 452 0798 00000000 		or	%s34,0,%s2
 452      82002245 
 453 07a0 00000000 		or	%s35,0,%s3
 453      83002345 
 454              	# line 20
  20:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ld_fn_fld.c **** 	long double ret = f + ld;
 455              		.loc	1 20 0
 456 07a8 00000000 		cvt.q.s %s36,%s0
 456      0080A42D 
 457 07b0 00000000 		fadd.q	%s38,%s34,%s36
 457      A4A2266C 
 458              	# line 21
  21:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ld_fn_fld.c **** 	printf("%s:%u ld %f\n",__FILE__,__LINE__,(double)ret);
 459              		.loc	1 21 0
 460 07b8 00000000 		cvt.d.q %s63,%s38
 460      00A6BF0F 
 461 07c0 C8000000 		st	%s63,200(0,%sp)
 461      8B003F11 
 462              	# line 19
  19:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ld_fn_fld.c **** 	long double ret = f + ld;
 463              		.loc	1 19 0
 464 07c8 C0000000 		st	%s35,192(0,%fp)	# ld
 464      89002311 
 465 07d0 C0000000 		st	%s34,192(8,%fp)	# ld
 465      89082211 
 466              	# line 20
  20:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ld_fn_fld.c **** 	long double ret = f + ld;
 467              		.loc	1 20 0
 468 07d8 F0FFFFFF 		st	%s39,-16(0,%fp)	# ret
 468      89002711 
 469 07e0 F0FFFFFF 		st	%s38,-16(8,%fp)	# ret
 469      89082611 
 470              	# line 21
 471              		.loc	1 21 0
 472 07e8 00000000 		lea	%s62,.LP.__string.0@GOTOFF_LO
 472      00003E06 
 473 07f0 00000000 		and	%s62,%s62,(32)0
 473      60BE3E44 
 474 07f8 00000000 		lea.sl	%s62,.LP.__string.0@GOTOFF_HI(%s62,%got)
 474      8FBEBE06 
 475 0800 B0000000 		st	%s62,176(0,%sp)
 475      8B003E11 
 476 0808 00000000 		lea	%s61,.LP.__string.1@GOTOFF_LO
 476      00003D06 
 477 0810 00000000 		and	%s61,%s61,(32)0
 477      60BD3D44 
 478 0818 00000000 		lea.sl	%s61,.LP.__string.1@GOTOFF_HI(%s61,%got)
 478      8FBDBD06 
 479 0820 B8000000 		st	%s61,184(0,%sp)
 479      8B003D11 
 480 0828 00000000 		or	%s60,21,(0)1
 480      00153C45 
 481 0830 C0000000 		st	%s60,192(0,%sp)
 481      8B003C11 
 482 0838 00000000 		or	%s0,0,%s62
 482      BE000045 
 483 0840 00000000 		or	%s1,0,%s61
 483      BD000145 
 484 0848 00000000 		or	%s2,0,%s60
 484      BC000245 
 485 0850 00000000 		or	%s3,0,%s63
 485      BF000345 
 486 0858 00000000 		lea	%s12,printf@PLT_LO(-24)
 486      00680C06 
 487 0860 00000000 		and	%s12,%s12,(32)0
 487      608C0C44 
 488 0868 00000000 		sic	%lr
 488      00000A28 
 489 0870 00000000 		lea.sl	%s12,printf@PLT_HI(%s12,%lr)
 489      8A8C8C06 
 490 0878 00000000 		bsic	%lr,(,%s12)		# printf
 490      8C000A08 
 491 0880 08000000 		br.l	.L_4.1
 491      00000F18 
 492              	.L_4.1:
 493              	# line 22
  22:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ld_fn_fld.c **** 	return ret;
 494              		.loc	1 22 0
 495 0888 F0FFFFFF 		ld	%s35,-16(0,%fp)	# ret
 495      89002301 
 496 0890 F0FFFFFF 		ld	%s34,-16(8,%fp)	# ret
 496      89082201 
 497 0898 00000000 		or	%s63,0,%s34
 497      A2003F45 
 498 08a0 00000000 		or	%s62,0,%s35
 498      A3003E45 
 499 08a8 00000000 		or	%s0,0,%s63
 499      BF000045 
 500 08b0 00000000 		or	%s1,0,%s62
 500      BE000145 
 501              	# Start of epilogue codes
 502 08b8 00000000 		or	%sp,0,%fp
 502      89000B45 
 503              		.cfi_def_cfa	11,8
 504 08c0 18000000 		ld	%got,0x18(,%sp)
 504      8B000F01 
 505 08c8 20000000 		ld	%plt,0x20(,%sp)
 505      8B001001 
 506 08d0 08000000 		ld	%lr,0x8(,%sp)
 506      8B000A01 
 507 08d8 00000000 		ld	%fp,0x0(,%sp)
 507      8B000901 
 508 08e0 00000000 		b.l	(,%lr)
 508      8A000F19 
 509              	.L_4.EoE:
 510              		.cfi_endproc
 511              		.set	.L.4.2auto_size,	0xfffffffffffffe20	# 480 Bytes
 513              	# ============ End  prt_ld_fn_fld ============
 514              	.Le1.0:
