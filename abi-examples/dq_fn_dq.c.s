   1              		.ident "ncc 1.1.4 (Build 10:17:22 Apr 19 2018)"
   2              		.file "dq_fn_dq.c"
   3              		.file 1 "/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/dq_fn_dq.c"
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
  14              	# ============ Begin  dq_fn_dq ============
  15              		.section	.bss
  16              		.local	dq_ret
  17              	.comm dq_ret,16,8
  18              		.text
  19              		.balign 16
  20              	.L_1.0:
  21              	# line 7
   1:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/dq_fn_dq.c **** #include <stdio.h>
   2:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/dq_fn_dq.c **** #include <complex.h>
   3:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/dq_fn_dq.c **** 
   4:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/dq_fn_dq.c **** extern double complex ext_dq_fn_dq(double complex dq);
   5:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/dq_fn_dq.c **** 
   6:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/dq_fn_dq.c **** static double complex dq_ret;
   7:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/dq_fn_dq.c **** double complex dq_fn_dq(double complex dq) {
  22              		.loc	1 7 0
  23              		.globl	dq_fn_dq
  25              	dq_fn_dq:
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
  55 00c8 20FEFFFF 		lea	%s13,.L.1.2auto_size&0xffffffff
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
   8:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/dq_fn_dq.c **** 	dq_ret = ext_dq_fn_dq(dq);
  70              		.loc	1 8 0
  71 0128 00000000 		lea	%s12,ext_dq_fn_dq@PLT_LO(-24)
  71      00680C06 
  72 0130 00000000 		and	%s12,%s12,(32)0
  72      608C0C44 
  73 0138 00000000 		sic	%lr
  73      00000A28 
  74 0140 00000000 		lea.sl	%s12,ext_dq_fn_dq@PLT_HI(%s12,%lr)
  74      8A8C8C06 
  75 0148 00000000 		bsic	%lr,(,%s12)		# ext_dq_fn_dq
  75      8C000A08 
  76 0150 F8FFFFFF 		st	%s0,-8(,%fp)	# spill
  76      89000011 
  77 0158 F0FFFFFF 		st	%s1,-16(,%fp)	# spill
  77      89000111 
  78 0160 08000000 		br.l	.L_1.1
  78      00000F18 
  79              	.L_1.1:
  80 0168 00000000 		lea	%s63,dq_ret@GOTOFF_LO
  80      00003F06 
  81 0170 00000000 		and	%s63,%s63,(32)0
  81      60BF3F44 
  82 0178 00000000 		lea.sl	%s63,dq_ret@GOTOFF_HI(%s63,%got)
  82      8FBFBF06 
  83 0180 F8FFFFFF 		ld	%s62,-8(,%fp)	# restore
  83      89003E01 
  84 0188 00000000 		st	%s62,0(0,%s63)	# dq_ret (real)
  84      BF003E11 
  85 0190 F0FFFFFF 		ld	%s61,-16(,%fp)	# restore
  85      89003D01 
  86 0198 08000000 		st	%s61,8(0,%s63)	# dq_ret (imaginary)
  86      BF003D11 
  87              	# line 9
   9:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/dq_fn_dq.c **** 	return dq_ret;
  88              		.loc	1 9 0
  89 01a0 00000000 		or	%s0,0,%s62
  89      BE000045 
  90 01a8 00000000 		or	%s1,0,%s61
  90      BD000145 
  91              	# Start of epilogue codes
  92 01b0 00000000 		or	%sp,0,%fp
  92      89000B45 
  93              		.cfi_def_cfa	11,8
  94 01b8 18000000 		ld	%got,0x18(,%sp)
  94      8B000F01 
  95 01c0 20000000 		ld	%plt,0x20(,%sp)
  95      8B001001 
  96 01c8 08000000 		ld	%lr,0x8(,%sp)
  96      8B000A01 
  97 01d0 00000000 		ld	%fp,0x0(,%sp)
  97      8B000901 
  98 01d8 00000000 		b.l	(,%lr)
  98      8A000F19 
  99              	.L_1.EoE:
 100              		.cfi_endproc
 101              		.set	.L.1.2auto_size,	0xfffffffffffffe20	# 480 Bytes
 103              	# ============ End  dq_fn_dq ============
 104              	# ============ Begin  dq_fn_dq2 ============
 105              		.balign 16
 106              	.L_2.0:
 107              	# line 11
  10:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/dq_fn_dq.c **** }
  11:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/dq_fn_dq.c **** double complex dq_fn_dq2(double complex dq) {
 108              		.loc	1 11 0
 109              		.globl	dq_fn_dq2
 111              	dq_fn_dq2:
 112              		.cfi_startproc
 113 01e0 00000000 		st	%fp,0x0(,%sp)
 113      8B000911 
 114              		.cfi_def_cfa_offset	0
 115              		.cfi_offset	9,0
 116 01e8 08000000 		st	%lr,0x8(,%sp)
 116      8B000A11 
 117 01f0 18000000 		st	%got,0x18(,%sp)
 117      8B000F11 
 118 01f8 20000000 		st	%plt,0x20(,%sp)
 118      8B001011 
 119 0200 00000000 		lea	%got,_GLOBAL_OFFSET_TABLE_@PC_LO(-0x18)
 119      00680F06 
 120 0208 00000000 		and	%got,%got,(32)0
 120      608F0F44 
 121 0210 00000000 		sic	%plt
 121      00001028 
 122 0218 00000000 		lea.sl	%got,_GLOBAL_OFFSET_TABLE_@PC_HI(%plt,%got)
 122      8F908F06 
 123 0220 00000000 		or	%fp,0,%sp
 123      8B000945 
 124              		.cfi_def_cfa_register	9
 125 0228 30000000 		st	%s18,48(,%fp)
 125      89001211 
 126 0230 38000000 		st	%s19,56(,%fp)
 126      89001311 
 127 0238 40000000 		st	%s20,64(,%fp)
 127      89001411 
 128 0240 48000000 		st	%s21,72(,%fp)
 128      89001511 
 129 0248 50000000 		st	%s22,80(,%fp)
 129      89001611 
 130 0250 58000000 		st	%s23,88(,%fp)
 130      89001711 
 131 0258 60000000 		st	%s24,96(,%fp)
 131      89001811 
 132 0260 68000000 		st	%s25,104(,%fp)
 132      89001911 
 133 0268 70000000 		st	%s26,112(,%fp)
 133      89001A11 
 134 0270 78000000 		st	%s27,120(,%fp)
 134      89001B11 
 135 0278 80000000 		st	%s28,128(,%fp)
 135      89001C11 
 136 0280 88000000 		st	%s29,136(,%fp)
 136      89001D11 
 137 0288 90000000 		st	%s30,144(,%fp)
 137      89001E11 
 138 0290 98000000 		st	%s31,152(,%fp)
 138      89001F11 
 139 0298 A0000000 		st	%s32,160(,%fp)
 139      89002011 
 140 02a0 A8000000 		st	%s33,168(,%fp)
 140      89002111 
 141 02a8 10FEFFFF 		lea	%s13,.L.2.2auto_size&0xffffffff
 141      00000D06 
 142 02b0 00000000 		and	%s13,%s13,(32)0
 142      608D0D44 
 143 02b8 FFFFFFFF 		lea.sl	%sp,.L.2.2auto_size>>32(%fp,%s13)
 143      8D898B06 
 144 02c0 48000000 		brge.l.t	%sp,%sl,.L_2.EoP
 144      888B3518 
 145 02c8 18000000 		ld	%s61,0x18(,%tp)
 145      8E003D01 
 146 02d0 00000000 		or	%s62,0,%s0
 146      80003E45 
 147 02d8 3B010000 		lea	%s63,0x13b
 147      00003F06 
 148 02e0 00000000 		shm.l	%s63,0x0(%s61)
 148      BD033F31 
 149 02e8 08000000 		shm.l	%sl,0x8(%s61)
 149      BD030831 
 150 02f0 10000000 		shm.l	%sp,0x10(%s61)
 150      BD030B31 
 151 02f8 00000000 		monc
 151      0000003F 
 152 0300 00000000 		or	%s0,0,%s62
 152      BE000045 
 153              	.L_2.EoP:
 154              	# End of prologue codes
 155              	# line 12
  12:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/dq_fn_dq.c **** 	double complex *p_ret = &dq_ret;
 156              		.loc	1 12 0
 157 0308 00000000 		lea	%s63,dq_ret@GOTOFF_LO
 157      00003F06 
 158 0310 00000000 		and	%s63,%s63,(32)0
 158      60BF3F44 
 159 0318 00000000 		lea.sl	%s63,dq_ret@GOTOFF_HI(%s63,%got)
 159      8FBFBF06 
 160 0320 00000000 		or	%s62,%s63,(0)1
 160      00BF3E45 
 161              	# line 13
  13:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/dq_fn_dq.c **** 	*p_ret = ext_dq_fn_dq(dq);
 162              		.loc	1 13 0
 163 0328 F8FFFFFF 		st	%s62,-8(,%fp)	# spill
 163      89003E11 
 164 0330 00000000 		lea	%s12,ext_dq_fn_dq@PLT_LO(-24)
 164      00680C06 
 165 0338 00000000 		and	%s12,%s12,(32)0
 165      608C0C44 
 166 0340 00000000 		sic	%lr
 166      00000A28 
 167 0348 00000000 		lea.sl	%s12,ext_dq_fn_dq@PLT_HI(%s12,%lr)
 167      8A8C8C06 
 168 0350 00000000 		bsic	%lr,(,%s12)		# ext_dq_fn_dq
 168      8C000A08 
 169 0358 F0FFFFFF 		st	%s0,-16(,%fp)	# spill
 169      89000011 
 170 0360 E8FFFFFF 		st	%s1,-24(,%fp)	# spill
 170      89000111 
 171 0368 08000000 		br.l	.L_2.1
 171      00000F18 
 172              	.L_2.1:
 173 0370 F8FFFFFF 		ld	%s63,-8(,%fp)	# restore
 173      89003F01 
 174 0378 F0FFFFFF 		ld	%s62,-16(,%fp)	# restore
 174      89003E01 
 175 0380 00000000 		st	%s62,0(0,%s63)	# *(p_ret) (real)
 175      BF003E11 
 176 0388 E8FFFFFF 		ld	%s61,-24(,%fp)	# restore
 176      89003D01 
 177 0390 08000000 		st	%s61,8(0,%s63)	# *(p_ret) (imaginary)
 177      BF003D11 
 178              	# line 14
  14:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/dq_fn_dq.c **** 	return *p_ret;
 179              		.loc	1 14 0
 180 0398 00000000 		or	%s0,0,%s62
 180      BE000045 
 181 03a0 00000000 		or	%s1,0,%s61
 181      BD000145 
 182              	# Start of epilogue codes
 183 03a8 00000000 		or	%sp,0,%fp
 183      89000B45 
 184              		.cfi_def_cfa	11,8
 185 03b0 18000000 		ld	%got,0x18(,%sp)
 185      8B000F01 
 186 03b8 20000000 		ld	%plt,0x20(,%sp)
 186      8B001001 
 187 03c0 08000000 		ld	%lr,0x8(,%sp)
 187      8B000A01 
 188 03c8 00000000 		ld	%fp,0x0(,%sp)
 188      8B000901 
 189 03d0 00000000 		b.l	(,%lr)
 189      8A000F19 
 190              	.L_2.EoE:
 191              		.cfi_endproc
 192              		.set	.L.2.2auto_size,	0xfffffffffffffe10	# 496 Bytes
 194              	# ============ End  dq_fn_dq2 ============
 195              	# ============ Begin  mt_dq_fn_dq ============
 196 03d8 00000000 		.balign 16
 196      00000000 
 197              	.L_3.0:
 198              	# line 16
  15:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/dq_fn_dq.c **** }
  16:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/dq_fn_dq.c **** double complex mt_dq_fn_dq(double complex dq) {
 199              		.loc	1 16 0
 200              		.globl	mt_dq_fn_dq
 202              	mt_dq_fn_dq:
 203              		.cfi_startproc
 204 03e0 00000000 		st	%fp,0x0(,%sp)
 204      8B000911 
 205              		.cfi_def_cfa_offset	0
 206              		.cfi_offset	9,0
 207 03e8 08000000 		st	%lr,0x8(,%sp)
 207      8B000A11 
 208 03f0 18000000 		st	%got,0x18(,%sp)
 208      8B000F11 
 209 03f8 20000000 		st	%plt,0x20(,%sp)
 209      8B001011 
 210 0400 00000000 		lea	%got,_GLOBAL_OFFSET_TABLE_@PC_LO(-0x18)
 210      00680F06 
 211 0408 00000000 		and	%got,%got,(32)0
 211      608F0F44 
 212 0410 00000000 		sic	%plt
 212      00001028 
 213 0418 00000000 		lea.sl	%got,_GLOBAL_OFFSET_TABLE_@PC_HI(%plt,%got)
 213      8F908F06 
 214 0420 00000000 		or	%fp,0,%sp
 214      8B000945 
 215              		.cfi_def_cfa_register	9
 216 0428 30000000 		st	%s18,48(,%fp)
 216      89001211 
 217 0430 38000000 		st	%s19,56(,%fp)
 217      89001311 
 218 0438 40000000 		st	%s20,64(,%fp)
 218      89001411 
 219 0440 48000000 		st	%s21,72(,%fp)
 219      89001511 
 220 0448 50000000 		st	%s22,80(,%fp)
 220      89001611 
 221 0450 58000000 		st	%s23,88(,%fp)
 221      89001711 
 222 0458 60000000 		st	%s24,96(,%fp)
 222      89001811 
 223 0460 68000000 		st	%s25,104(,%fp)
 223      89001911 
 224 0468 70000000 		st	%s26,112(,%fp)
 224      89001A11 
 225 0470 78000000 		st	%s27,120(,%fp)
 225      89001B11 
 226 0478 80000000 		st	%s28,128(,%fp)
 226      89001C11 
 227 0480 88000000 		st	%s29,136(,%fp)
 227      89001D11 
 228 0488 90000000 		st	%s30,144(,%fp)
 228      89001E11 
 229 0490 98000000 		st	%s31,152(,%fp)
 229      89001F11 
 230 0498 A0000000 		st	%s32,160(,%fp)
 230      89002011 
 231 04a0 A8000000 		st	%s33,168(,%fp)
 231      89002111 
 232 04a8 00000000 		lea	%s13,.L.3.2auto_size&0xffffffff
 232      00000D06 
 233 04b0 00000000 		and	%s13,%s13,(32)0
 233      608D0D44 
 234 04b8 00000000 		lea.sl	%sp,.L.3.2auto_size>>32(%fp,%s13)
 234      8D898B06 
 235 04c0 48000000 		brge.l.t	%sp,%sl,.L_3.EoP
 235      888B3518 
 236 04c8 18000000 		ld	%s61,0x18(,%tp)
 236      8E003D01 
 237 04d0 00000000 		or	%s62,0,%s0
 237      80003E45 
 238 04d8 3B010000 		lea	%s63,0x13b
 238      00003F06 
 239 04e0 00000000 		shm.l	%s63,0x0(%s61)
 239      BD033F31 
 240 04e8 08000000 		shm.l	%sl,0x8(%s61)
 240      BD030831 
 241 04f0 10000000 		shm.l	%sp,0x10(%s61)
 241      BD030B31 
 242 04f8 00000000 		monc
 242      0000003F 
 243 0500 00000000 		or	%s0,0,%s62
 243      BE000045 
 244              	.L_3.EoP:
 245              	# End of prologue codes
 246              	# line 17
  17:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/dq_fn_dq.c **** 	double complex ret = dq+1.49;
 247              		.loc	1 17 0
 248 0508 D7A3703D 		lea	%s63,1030792151
 248      00003F06 
 249 0510 00000000 		and	%s62,%s63,(32)0
 249      60BF3E44 
 250 0518 0AD7F73F 		lea.sl	%s61,1073207050(,%s62)
 250      BE00BD06 
 251 0520 00000000 		fadd.d	%s60,%s0,%s61
 251      BD803C4C 
 252              	# line 18
  18:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/dq_fn_dq.c **** 	return ret;
 253              		.loc	1 18 0
 254 0528 00000000 		or	%s0,0,%s60
 254      BC000045 
 255              	# Start of epilogue codes
 256 0530 00000000 		or	%sp,0,%fp
 256      89000B45 
 257              		.cfi_def_cfa	11,8
 258 0538 18000000 		ld	%got,0x18(,%sp)
 258      8B000F01 
 259 0540 20000000 		ld	%plt,0x20(,%sp)
 259      8B001001 
 260 0548 08000000 		ld	%lr,0x8(,%sp)
 260      8B000A01 
 261 0550 00000000 		ld	%fp,0x0(,%sp)
 261      8B000901 
 262 0558 00000000 		b.l	(,%lr)
 262      8A000F19 
 263              	.L_3.EoE:
 264              		.cfi_endproc
 265              		.set	.L.3.2auto_size,	0x0	# 0 Bytes
 267              	# ============ End  mt_dq_fn_dq ============
 268              	# ============ Begin  prt_dq_fn_dq ============
 269              		.section .rodata
 270              		.balign 16
 272              	.LP.__string.1:
 273 0000 2F       		.byte	47
 274 0001 75       		.byte	117
 275 0002 73       		.byte	115
 276 0003 72       		.byte	114
 277 0004 2F       		.byte	47
 278 0005 75       		.byte	117
 279 0006 68       		.byte	104
 280 0007 6F       		.byte	111
 281 0008 6D       		.byte	109
 282 0009 65       		.byte	101
 283 000a 2F       		.byte	47
 284 000b 61       		.byte	97
 285 000c 75       		.byte	117
 286 000d 72       		.byte	114
 287 000e 6F       		.byte	111
 288 000f 72       		.byte	114
 289 0010 61       		.byte	97
 290 0011 2F       		.byte	47
 291 0012 34       		.byte	52
 292 0013 67       		.byte	103
 293 0014 69       		.byte	105
 294 0015 2F       		.byte	47
 295 0016 6E       		.byte	110
 296 0017 6C       		.byte	108
 297 0018 61       		.byte	97
 298 0019 62       		.byte	98
 299 001a 68       		.byte	104
 300 001b 70       		.byte	112
 301 001c 67       		.byte	103
 302 001d 2F       		.byte	47
 303 001e 6B       		.byte	107
 304 001f 72       		.byte	114
 305 0020 75       		.byte	117
 306 0021 75       		.byte	117
 307 0022 73       		.byte	115
 308 0023 2F       		.byte	47
 309 0024 76       		.byte	118
 310 0025 74       		.byte	116
 311 0026 2F       		.byte	47
 312 0027 73       		.byte	115
 313 0028 72       		.byte	114
 314 0029 63       		.byte	99
 315 002a 2F       		.byte	47
 316 002b 61       		.byte	97
 317 002c 73       		.byte	115
 318 002d 6D       		.byte	109
 319 002e 2D       		.byte	45
 320 002f 65       		.byte	101
 321 0030 78       		.byte	120
 322 0031 61       		.byte	97
 323 0032 6D       		.byte	109
 324 0033 70       		.byte	112
 325 0034 6C       		.byte	108
 326 0035 65       		.byte	101
 327 0036 73       		.byte	115
 328 0037 2F       		.byte	47
 329 0038 64       		.byte	100
 330 0039 71       		.byte	113
 331 003a 5F       		.byte	95
 332 003b 66       		.byte	102
 333 003c 6E       		.byte	110
 334 003d 5F       		.byte	95
 335 003e 64       		.byte	100
 336 003f 71       		.byte	113
 337 0040 2E       		.byte	46
 338 0041 63       		.byte	99
 339 0042 00       		.zero	1
 340 0043 00000000 		.balign 8
 340      00
 342              	.LP.__string.0:
 343 0048 25       		.byte	37
 344 0049 73       		.byte	115
 345 004a 3A       		.byte	58
 346 004b 25       		.byte	37
 347 004c 75       		.byte	117
 348 004d 20       		.byte	32
 349 004e 25       		.byte	37
 350 004f 66       		.byte	102
 351 0050 2B       		.byte	43
 352 0051 25       		.byte	37
 353 0052 66       		.byte	102
 354 0053 69       		.byte	105
 355 0054 0A       		.byte	10
 356 0055 00       		.zero	1
 357              		.text
 358              		.balign 16
 359              	.L_4.0:
 360              	# line 20
  19:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/dq_fn_dq.c **** }
  20:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/dq_fn_dq.c **** double complex prt_dq_fn_dq(double complex dq) {
 361              		.loc	1 20 0
 362              		.globl	prt_dq_fn_dq
 364              	prt_dq_fn_dq:
 365              		.cfi_startproc
 366 0560 00000000 		st	%fp,0x0(,%sp)
 366      8B000911 
 367              		.cfi_def_cfa_offset	0
 368              		.cfi_offset	9,0
 369 0568 08000000 		st	%lr,0x8(,%sp)
 369      8B000A11 
 370 0570 18000000 		st	%got,0x18(,%sp)
 370      8B000F11 
 371 0578 20000000 		st	%plt,0x20(,%sp)
 371      8B001011 
 372 0580 00000000 		lea	%got,_GLOBAL_OFFSET_TABLE_@PC_LO(-0x18)
 372      00680F06 
 373 0588 00000000 		and	%got,%got,(32)0
 373      608F0F44 
 374 0590 00000000 		sic	%plt
 374      00001028 
 375 0598 00000000 		lea.sl	%got,_GLOBAL_OFFSET_TABLE_@PC_HI(%plt,%got)
 375      8F908F06 
 376 05a0 00000000 		or	%fp,0,%sp
 376      8B000945 
 377              		.cfi_def_cfa_register	9
 378 05a8 30000000 		st	%s18,48(,%fp)
 378      89001211 
 379 05b0 38000000 		st	%s19,56(,%fp)
 379      89001311 
 380 05b8 40000000 		st	%s20,64(,%fp)
 380      89001411 
 381 05c0 48000000 		st	%s21,72(,%fp)
 381      89001511 
 382 05c8 50000000 		st	%s22,80(,%fp)
 382      89001611 
 383 05d0 58000000 		st	%s23,88(,%fp)
 383      89001711 
 384 05d8 60000000 		st	%s24,96(,%fp)
 384      89001811 
 385 05e0 68000000 		st	%s25,104(,%fp)
 385      89001911 
 386 05e8 70000000 		st	%s26,112(,%fp)
 386      89001A11 
 387 05f0 78000000 		st	%s27,120(,%fp)
 387      89001B11 
 388 05f8 80000000 		st	%s28,128(,%fp)
 388      89001C11 
 389 0600 88000000 		st	%s29,136(,%fp)
 389      89001D11 
 390 0608 90000000 		st	%s30,144(,%fp)
 390      89001E11 
 391 0610 98000000 		st	%s31,152(,%fp)
 391      89001F11 
 392 0618 A0000000 		st	%s32,160(,%fp)
 392      89002011 
 393 0620 A8000000 		st	%s33,168(,%fp)
 393      89002111 
 394 0628 C0FDFFFF 		lea	%s13,.L.4.2auto_size&0xffffffff
 394      00000D06 
 395 0630 00000000 		and	%s13,%s13,(32)0
 395      608D0D44 
 396 0638 FFFFFFFF 		lea.sl	%sp,.L.4.2auto_size>>32(%fp,%s13)
 396      8D898B06 
 397 0640 48000000 		brge.l.t	%sp,%sl,.L_4.EoP
 397      888B3518 
 398 0648 18000000 		ld	%s61,0x18(,%tp)
 398      8E003D01 
 399 0650 00000000 		or	%s62,0,%s0
 399      80003E45 
 400 0658 3B010000 		lea	%s63,0x13b
 400      00003F06 
 401 0660 00000000 		shm.l	%s63,0x0(%s61)
 401      BD033F31 
 402 0668 08000000 		shm.l	%sl,0x8(%s61)
 402      BD030831 
 403 0670 10000000 		shm.l	%sp,0x10(%s61)
 403      BD030B31 
 404 0678 00000000 		monc
 404      0000003F 
 405 0680 00000000 		or	%s0,0,%s62
 405      BE000045 
 406              	.L_4.EoP:
 407              	# End of prologue codes
 408              	# line 21
  21:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/dq_fn_dq.c **** 	double complex ret = dq + 1.49;
 409              		.loc	1 21 0
 410 0688 D7A3703D 		lea	%s63,1030792151
 410      00003F06 
 411 0690 00000000 		and	%s62,%s63,(32)0
 411      60BF3E44 
 412 0698 0AD7F73F 		lea.sl	%s61,1073207050(,%s62)
 412      BE00BD06 
 413 06a0 00000000 		fadd.d	%s60,%s0,%s61
 413      BD803C4C 
 414              	# line 22
  22:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/dq_fn_dq.c **** 	printf("%s:%u %f+%fi\n",__FILE__,__LINE__,creal(ret),cimag(ret));
 415              		.loc	1 22 0
 416 06a8 00000000 		lea	%s59,.LP.__string.0@GOTOFF_LO
 416      00003B06 
 417 06b0 00000000 		and	%s59,%s59,(32)0
 417      60BB3B44 
 418 06b8 00000000 		lea.sl	%s59,.LP.__string.0@GOTOFF_HI(%s59,%got)
 418      8FBBBB06 
 419 06c0 00000000 		lea	%s58,.LP.__string.1@GOTOFF_LO
 419      00003A06 
 420 06c8 00000000 		and	%s58,%s58,(32)0
 420      60BA3A44 
 421 06d0 00000000 		lea.sl	%s58,.LP.__string.1@GOTOFF_HI(%s58,%got)
 421      8FBABA06 
 422 06d8 00000000 		or	%s0,0,%s60
 422      BC000045 
 423 06e0 F8FFFFFF 		st	%s1,-8(,%fp)	# spill
 423      89000111 
 424 06e8 F0FFFFFF 		st	%s0,-16(,%fp)	# spill
 424      89000011 
 425 06f0 E8FFFFFF 		st	%s59,-24(,%fp)	# spill
 425      89003B11 
 426 06f8 E0FFFFFF 		st	%s58,-32(,%fp)	# spill
 426      89003A11 
 427 0700 00000000 		lea	%s12,creal@PLT_LO(-24)
 427      00680C06 
 428 0708 00000000 		and	%s12,%s12,(32)0
 428      608C0C44 
 429 0710 00000000 		sic	%lr
 429      00000A28 
 430 0718 00000000 		lea.sl	%s12,creal@PLT_HI(%s12,%lr)
 430      8A8C8C06 
 431 0720 00000000 		bsic	%lr,(,%s12)		# creal
 431      8C000A08 
 432              	.L_4.3:
 433 0728 D8FFFFFF 		st	%s0,-40(,%fp)	# spill
 433      89000011 
 434 0730 F0FFFFFF 		ld	%s0,-16(,%fp)	# restore
 434      89000001 
 435 0738 F8FFFFFF 		ld	%s1,-8(,%fp)	# restore
 435      89000101 
 436 0740 F8FFFFFF 		st	%s1,-8(,%fp)	# spill
 436      89000111 
 437 0748 F0FFFFFF 		st	%s0,-16(,%fp)	# spill
 437      89000011 
 438 0750 00000000 		lea	%s12,cimag@PLT_LO(-24)
 438      00680C06 
 439 0758 00000000 		and	%s12,%s12,(32)0
 439      608C0C44 
 440 0760 00000000 		sic	%lr
 440      00000A28 
 441 0768 00000000 		lea.sl	%s12,cimag@PLT_HI(%s12,%lr)
 441      8A8C8C06 
 442 0770 00000000 		bsic	%lr,(,%s12)		# cimag
 442      8C000A08 
 443              	.L_4.2:
 444 0778 E8FFFFFF 		ld	%s63,-24(,%fp)	# restore
 444      89003F01 
 445 0780 B0000000 		st	%s63,176(0,%sp)
 445      8B003F11 
 446 0788 E0FFFFFF 		ld	%s62,-32(,%fp)	# restore
 446      89003E01 
 447 0790 B8000000 		st	%s62,184(0,%sp)
 447      8B003E11 
 448 0798 00000000 		or	%s61,22,(0)1
 448      00163D45 
 449 07a0 C0000000 		st	%s61,192(0,%sp)
 449      8B003D11 
 450 07a8 D8FFFFFF 		ld	%s60,-40(,%fp)	# restore
 450      89003C01 
 451 07b0 C8000000 		st	%s60,200(0,%sp)
 451      8B003C11 
 452 07b8 D0000000 		st	%s0,208(0,%sp)
 452      8B000011 
 453 07c0 D0FFFFFF 		st	%s0,-48(,%fp)	# spill
 453      89000011 
 454 07c8 00000000 		or	%s0,0,%s63
 454      BF000045 
 455 07d0 00000000 		or	%s1,0,%s62
 455      BE000145 
 456 07d8 00000000 		or	%s2,0,%s61
 456      BD000245 
 457 07e0 00000000 		or	%s3,0,%s60
 457      BC000345 
 458 07e8 D0FFFFFF 		ld	%s4,-48(,%fp)	# restore
 458      89000401 
 459 07f0 00000000 		lea	%s12,printf@PLT_LO(-24)
 459      00680C06 
 460 07f8 00000000 		and	%s12,%s12,(32)0
 460      608C0C44 
 461 0800 00000000 		sic	%lr
 461      00000A28 
 462 0808 00000000 		lea.sl	%s12,printf@PLT_HI(%s12,%lr)
 462      8A8C8C06 
 463 0810 00000000 		bsic	%lr,(,%s12)		# printf
 463      8C000A08 
 464 0818 08000000 		br.l	.L_4.1
 464      00000F18 
 465              	.L_4.1:
 466              	# line 23
  23:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/dq_fn_dq.c **** 	return ret;
 467              		.loc	1 23 0
 468 0820 F0FFFFFF 		ld	%s0,-16(,%fp)	# restore
 468      89000001 
 469 0828 F8FFFFFF 		ld	%s1,-8(,%fp)	# restore
 469      89000101 
 470              	# Start of epilogue codes
 471 0830 00000000 		or	%sp,0,%fp
 471      89000B45 
 472              		.cfi_def_cfa	11,8
 473 0838 18000000 		ld	%got,0x18(,%sp)
 473      8B000F01 
 474 0840 20000000 		ld	%plt,0x20(,%sp)
 474      8B001001 
 475 0848 08000000 		ld	%lr,0x8(,%sp)
 475      8B000A01 
 476 0850 00000000 		ld	%fp,0x0(,%sp)
 476      8B000901 
 477 0858 00000000 		b.l	(,%lr)
 477      8A000F19 
 478              	.L_4.EoE:
 479              		.cfi_endproc
 480              		.set	.L.4.2auto_size,	0xfffffffffffffdc0	# 576 Bytes
 482              	# ============ End  prt_dq_fn_dq ============
 483              	.Le1.0:
