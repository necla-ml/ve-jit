   1              		.ident "ncc 1.1.4 (Build 10:17:22 Apr 19 2018)"
   2              		.file "fq_fn_fq.c"
   3              		.file 1 "/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/fq_fn_fq.c"
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
  14              	# ============ Begin  fq_fn_fq ============
  15              		.section	.bss
  16              		.local	fq_ret
  17              	.comm fq_ret,8,4
  18              		.text
  19              		.balign 16
  20              	.L_1.0:
  21              	# line 7
   1:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/fq_fn_fq.c **** #include <stdio.h>
   2:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/fq_fn_fq.c **** #include <complex.h>
   3:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/fq_fn_fq.c **** 
   4:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/fq_fn_fq.c **** extern float complex ext_fq_fn_fq(float complex fq);
   5:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/fq_fn_fq.c **** 
   6:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/fq_fn_fq.c **** static float complex fq_ret;
   7:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/fq_fn_fq.c **** float complex fq_fn_fq(float complex fq) {
  22              		.loc	1 7 0
  23              		.globl	fq_fn_fq
  25              	fq_fn_fq:
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
   8:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/fq_fn_fq.c **** 	fq_ret = ext_fq_fn_fq(fq);
  70              		.loc	1 8 0
  71 0128 00000000 		lea	%s12,ext_fq_fn_fq@PLT_LO(-24)
  71      00680C06 
  72 0130 00000000 		and	%s12,%s12,(32)0
  72      608C0C44 
  73 0138 00000000 		sic	%lr
  73      00000A28 
  74 0140 00000000 		lea.sl	%s12,ext_fq_fn_fq@PLT_HI(%s12,%lr)
  74      8A8C8C06 
  75 0148 00000000 		bsic	%lr,(,%s12)		# ext_fq_fn_fq
  75      8C000A08 
  76 0150 F8FFFFFF 		st	%s0,-8(,%fp)	# spill
  76      89000011 
  77 0158 F0FFFFFF 		st	%s1,-16(,%fp)	# spill
  77      89000111 
  78 0160 08000000 		br.l	.L_1.1
  78      00000F18 
  79              	.L_1.1:
  80 0168 00000000 		lea	%s63,fq_ret@GOTOFF_LO
  80      00003F06 
  81 0170 00000000 		and	%s63,%s63,(32)0
  81      60BF3F44 
  82 0178 00000000 		lea.sl	%s63,fq_ret@GOTOFF_HI(%s63,%got)
  82      8FBFBF06 
  83 0180 F8FFFFFF 		ld	%s62,-8(,%fp)	# restore
  83      89003E01 
  84 0188 00000000 		stu	%s62,0(0,%s63)	# fq_ret (real)
  84      BF003E12 
  85 0190 F0FFFFFF 		ld	%s61,-16(,%fp)	# restore
  85      89003D01 
  86 0198 04000000 		stu	%s61,4(0,%s63)	# fq_ret (imaginary)
  86      BF003D12 
  87              	# line 9
   9:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/fq_fn_fq.c **** 	return fq_ret;
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
 103              	# ============ End  fq_fn_fq ============
 104              	# ============ Begin  fq_fn_fq2 ============
 105              		.balign 16
 106              	.L_2.0:
 107              	# line 11
  10:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/fq_fn_fq.c **** }
  11:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/fq_fn_fq.c **** float complex fq_fn_fq2(float complex fq) {
 108              		.loc	1 11 0
 109              		.globl	fq_fn_fq2
 111              	fq_fn_fq2:
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
  12:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/fq_fn_fq.c **** 	float complex *p_ret = &fq_ret;
 156              		.loc	1 12 0
 157 0308 00000000 		lea	%s63,fq_ret@GOTOFF_LO
 157      00003F06 
 158 0310 00000000 		and	%s63,%s63,(32)0
 158      60BF3F44 
 159 0318 00000000 		lea.sl	%s63,fq_ret@GOTOFF_HI(%s63,%got)
 159      8FBFBF06 
 160 0320 00000000 		or	%s62,%s63,(0)1
 160      00BF3E45 
 161              	# line 13
  13:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/fq_fn_fq.c **** 	*p_ret = ext_fq_fn_fq(fq);
 162              		.loc	1 13 0
 163 0328 F8FFFFFF 		st	%s62,-8(,%fp)	# spill
 163      89003E11 
 164 0330 00000000 		lea	%s12,ext_fq_fn_fq@PLT_LO(-24)
 164      00680C06 
 165 0338 00000000 		and	%s12,%s12,(32)0
 165      608C0C44 
 166 0340 00000000 		sic	%lr
 166      00000A28 
 167 0348 00000000 		lea.sl	%s12,ext_fq_fn_fq@PLT_HI(%s12,%lr)
 167      8A8C8C06 
 168 0350 00000000 		bsic	%lr,(,%s12)		# ext_fq_fn_fq
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
 175 0380 00000000 		stu	%s62,0(0,%s63)	# *(p_ret) (real)
 175      BF003E12 
 176 0388 E8FFFFFF 		ld	%s61,-24(,%fp)	# restore
 176      89003D01 
 177 0390 04000000 		stu	%s61,4(0,%s63)	# *(p_ret) (imaginary)
 177      BF003D12 
 178              	# line 14
  14:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/fq_fn_fq.c **** 	return *p_ret;
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
 194              	# ============ End  fq_fn_fq2 ============
 195              	# ============ Begin  mt_fq_fn_fq ============
 196 03d8 00000000 		.balign 16
 196      00000000 
 197              	.L_3.0:
 198              	# line 16
  15:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/fq_fn_fq.c **** }
  16:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/fq_fn_fq.c **** float complex mt_fq_fn_fq(float complex fq) {
 199              		.loc	1 16 0
 200              		.globl	mt_fq_fn_fq
 202              	mt_fq_fn_fq:
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
  17:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/fq_fn_fq.c **** 	float complex ret = fq+1.49;
 247              		.loc	1 17 0
 248 0508 00000000 		cvt.d.s %s63,%s1
 248      00813F0F 
 249 0510 00000000 		cvt.s.d %s62,%s63
 249      00BF3E1F 
 250 0518 00000000 		cvt.d.s %s61,%s0
 250      00803D0F 
 251 0520 D7A3703D 		lea	%s60,1030792151
 251      00003C06 
 252 0528 00000000 		and	%s59,%s60,(32)0
 252      60BC3B44 
 253 0530 0AD7F73F 		lea.sl	%s58,1073207050(,%s59)
 253      BB00BA06 
 254 0538 00000000 		fadd.d	%s57,%s61,%s58
 254      BABD394C 
 255 0540 00000000 		cvt.s.d %s56,%s57
 255      00B9381F 
 256              	# line 18
  18:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/fq_fn_fq.c **** 	return ret;
 257              		.loc	1 18 0
 258 0548 00000000 		or	%s0,0,%s56
 258      B8000045 
 259 0550 00000000 		or	%s1,0,%s62
 259      BE000145 
 260              	# Start of epilogue codes
 261 0558 00000000 		or	%sp,0,%fp
 261      89000B45 
 262              		.cfi_def_cfa	11,8
 263 0560 18000000 		ld	%got,0x18(,%sp)
 263      8B000F01 
 264 0568 20000000 		ld	%plt,0x20(,%sp)
 264      8B001001 
 265 0570 08000000 		ld	%lr,0x8(,%sp)
 265      8B000A01 
 266 0578 00000000 		ld	%fp,0x0(,%sp)
 266      8B000901 
 267 0580 00000000 		b.l	(,%lr)
 267      8A000F19 
 268              	.L_3.EoE:
 269              		.cfi_endproc
 270              		.set	.L.3.2auto_size,	0x0	# 0 Bytes
 272              	# ============ End  mt_fq_fn_fq ============
 273              	# ============ Begin  prt_fq_fn_fq ============
 274              		.section .rodata
 275              		.balign 16
 277              	.LP.__string.1:
 278 0000 2F       		.byte	47
 279 0001 75       		.byte	117
 280 0002 73       		.byte	115
 281 0003 72       		.byte	114
 282 0004 2F       		.byte	47
 283 0005 75       		.byte	117
 284 0006 68       		.byte	104
 285 0007 6F       		.byte	111
 286 0008 6D       		.byte	109
 287 0009 65       		.byte	101
 288 000a 2F       		.byte	47
 289 000b 61       		.byte	97
 290 000c 75       		.byte	117
 291 000d 72       		.byte	114
 292 000e 6F       		.byte	111
 293 000f 72       		.byte	114
 294 0010 61       		.byte	97
 295 0011 2F       		.byte	47
 296 0012 34       		.byte	52
 297 0013 67       		.byte	103
 298 0014 69       		.byte	105
 299 0015 2F       		.byte	47
 300 0016 6E       		.byte	110
 301 0017 6C       		.byte	108
 302 0018 61       		.byte	97
 303 0019 62       		.byte	98
 304 001a 68       		.byte	104
 305 001b 70       		.byte	112
 306 001c 67       		.byte	103
 307 001d 2F       		.byte	47
 308 001e 6B       		.byte	107
 309 001f 72       		.byte	114
 310 0020 75       		.byte	117
 311 0021 75       		.byte	117
 312 0022 73       		.byte	115
 313 0023 2F       		.byte	47
 314 0024 76       		.byte	118
 315 0025 74       		.byte	116
 316 0026 2F       		.byte	47
 317 0027 73       		.byte	115
 318 0028 72       		.byte	114
 319 0029 63       		.byte	99
 320 002a 2F       		.byte	47
 321 002b 61       		.byte	97
 322 002c 73       		.byte	115
 323 002d 6D       		.byte	109
 324 002e 2D       		.byte	45
 325 002f 65       		.byte	101
 326 0030 78       		.byte	120
 327 0031 61       		.byte	97
 328 0032 6D       		.byte	109
 329 0033 70       		.byte	112
 330 0034 6C       		.byte	108
 331 0035 65       		.byte	101
 332 0036 73       		.byte	115
 333 0037 2F       		.byte	47
 334 0038 66       		.byte	102
 335 0039 71       		.byte	113
 336 003a 5F       		.byte	95
 337 003b 66       		.byte	102
 338 003c 6E       		.byte	110
 339 003d 5F       		.byte	95
 340 003e 66       		.byte	102
 341 003f 71       		.byte	113
 342 0040 2E       		.byte	46
 343 0041 63       		.byte	99
 344 0042 00       		.zero	1
 345 0043 00000000 		.balign 8
 345      00
 347              	.LP.__string.0:
 348 0048 25       		.byte	37
 349 0049 73       		.byte	115
 350 004a 3A       		.byte	58
 351 004b 25       		.byte	37
 352 004c 75       		.byte	117
 353 004d 20       		.byte	32
 354 004e 25       		.byte	37
 355 004f 66       		.byte	102
 356 0050 2B       		.byte	43
 357 0051 25       		.byte	37
 358 0052 66       		.byte	102
 359 0053 2A       		.byte	42
 360 0054 49       		.byte	73
 361 0055 0A       		.byte	10
 362 0056 00       		.zero	1
 363              		.text
 364 0588 00000000 		.balign 16
 364      00000000 
 365              	.L_4.0:
 366              	# line 20
  19:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/fq_fn_fq.c **** }
  20:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/fq_fn_fq.c **** float complex prt_fq_fn_fq(float complex fq) {
 367              		.loc	1 20 0
 368              		.globl	prt_fq_fn_fq
 370              	prt_fq_fn_fq:
 371              		.cfi_startproc
 372 0590 00000000 		st	%fp,0x0(,%sp)
 372      8B000911 
 373              		.cfi_def_cfa_offset	0
 374              		.cfi_offset	9,0
 375 0598 08000000 		st	%lr,0x8(,%sp)
 375      8B000A11 
 376 05a0 18000000 		st	%got,0x18(,%sp)
 376      8B000F11 
 377 05a8 20000000 		st	%plt,0x20(,%sp)
 377      8B001011 
 378 05b0 00000000 		lea	%got,_GLOBAL_OFFSET_TABLE_@PC_LO(-0x18)
 378      00680F06 
 379 05b8 00000000 		and	%got,%got,(32)0
 379      608F0F44 
 380 05c0 00000000 		sic	%plt
 380      00001028 
 381 05c8 00000000 		lea.sl	%got,_GLOBAL_OFFSET_TABLE_@PC_HI(%plt,%got)
 381      8F908F06 
 382 05d0 00000000 		or	%fp,0,%sp
 382      8B000945 
 383              		.cfi_def_cfa_register	9
 384 05d8 30000000 		st	%s18,48(,%fp)
 384      89001211 
 385 05e0 38000000 		st	%s19,56(,%fp)
 385      89001311 
 386 05e8 40000000 		st	%s20,64(,%fp)
 386      89001411 
 387 05f0 48000000 		st	%s21,72(,%fp)
 387      89001511 
 388 05f8 50000000 		st	%s22,80(,%fp)
 388      89001611 
 389 0600 58000000 		st	%s23,88(,%fp)
 389      89001711 
 390 0608 60000000 		st	%s24,96(,%fp)
 390      89001811 
 391 0610 68000000 		st	%s25,104(,%fp)
 391      89001911 
 392 0618 70000000 		st	%s26,112(,%fp)
 392      89001A11 
 393 0620 78000000 		st	%s27,120(,%fp)
 393      89001B11 
 394 0628 80000000 		st	%s28,128(,%fp)
 394      89001C11 
 395 0630 88000000 		st	%s29,136(,%fp)
 395      89001D11 
 396 0638 90000000 		st	%s30,144(,%fp)
 396      89001E11 
 397 0640 98000000 		st	%s31,152(,%fp)
 397      89001F11 
 398 0648 A0000000 		st	%s32,160(,%fp)
 398      89002011 
 399 0650 A8000000 		st	%s33,168(,%fp)
 399      89002111 
 400 0658 D0FDFFFF 		lea	%s13,.L.4.2auto_size&0xffffffff
 400      00000D06 
 401 0660 00000000 		and	%s13,%s13,(32)0
 401      608D0D44 
 402 0668 FFFFFFFF 		lea.sl	%sp,.L.4.2auto_size>>32(%fp,%s13)
 402      8D898B06 
 403 0670 48000000 		brge.l.t	%sp,%sl,.L_4.EoP
 403      888B3518 
 404 0678 18000000 		ld	%s61,0x18(,%tp)
 404      8E003D01 
 405 0680 00000000 		or	%s62,0,%s0
 405      80003E45 
 406 0688 3B010000 		lea	%s63,0x13b
 406      00003F06 
 407 0690 00000000 		shm.l	%s63,0x0(%s61)
 407      BD033F31 
 408 0698 08000000 		shm.l	%sl,0x8(%s61)
 408      BD030831 
 409 06a0 10000000 		shm.l	%sp,0x10(%s61)
 409      BD030B31 
 410 06a8 00000000 		monc
 410      0000003F 
 411 06b0 00000000 		or	%s0,0,%s62
 411      BE000045 
 412              	.L_4.EoP:
 413              	# End of prologue codes
 414              	# line 21
  21:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/fq_fn_fq.c **** 	float complex ret = fq + 1.49;
 415              		.loc	1 21 0
 416 06b8 00000000 		cvt.d.s %s63,%s1
 416      00813F0F 
 417 06c0 00000000 		cvt.s.d %s62,%s63
 417      00BF3E1F 
 418 06c8 00000000 		cvt.d.s %s61,%s0
 418      00803D0F 
 419 06d0 D7A3703D 		lea	%s60,1030792151
 419      00003C06 
 420 06d8 00000000 		and	%s59,%s60,(32)0
 420      60BC3B44 
 421 06e0 0AD7F73F 		lea.sl	%s58,1073207050(,%s59)
 421      BB00BA06 
 422 06e8 00000000 		fadd.d	%s57,%s61,%s58
 422      BABD394C 
 423 06f0 00000000 		cvt.s.d %s56,%s57
 423      00B9381F 
 424              	# line 22
  22:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/fq_fn_fq.c **** 	printf("%s:%u %f+%f*I\n",__FILE__,__LINE__,crealf(ret),cimagf(ret));
 425              		.loc	1 22 0
 426 06f8 00000000 		lea	%s55,.LP.__string.0@GOTOFF_LO
 426      00003706 
 427 0700 00000000 		and	%s55,%s55,(32)0
 427      60B73744 
 428 0708 00000000 		lea.sl	%s55,.LP.__string.0@GOTOFF_HI(%s55,%got)
 428      8FB7B706 
 429 0710 00000000 		lea	%s54,.LP.__string.1@GOTOFF_LO
 429      00003606 
 430 0718 00000000 		and	%s54,%s54,(32)0
 430      60B63644 
 431 0720 00000000 		lea.sl	%s54,.LP.__string.1@GOTOFF_HI(%s54,%got)
 431      8FB6B606 
 432 0728 00000000 		or	%s0,0,%s56
 432      B8000045 
 433 0730 00000000 		or	%s1,0,%s62
 433      BE000145 
 434 0738 F8FFFFFF 		st	%s1,-8(,%fp)	# spill
 434      89000111 
 435 0740 F0FFFFFF 		st	%s0,-16(,%fp)	# spill
 435      89000011 
 436 0748 E8FFFFFF 		st	%s55,-24(,%fp)	# spill
 436      89003711 
 437 0750 E0FFFFFF 		st	%s54,-32(,%fp)	# spill
 437      89003611 
 438 0758 00000000 		lea	%s12,crealf@PLT_LO(-24)
 438      00680C06 
 439 0760 00000000 		and	%s12,%s12,(32)0
 439      608C0C44 
 440 0768 00000000 		sic	%lr
 440      00000A28 
 441 0770 00000000 		lea.sl	%s12,crealf@PLT_HI(%s12,%lr)
 441      8A8C8C06 
 442 0778 00000000 		bsic	%lr,(,%s12)		# crealf
 442      8C000A08 
 443              	.L_4.3:
 444 0780 00000000 		cvt.d.s %s63,%s0
 444      00803F0F 
 445 0788 F0FFFFFF 		ld	%s0,-16(,%fp)	# restore
 445      89000001 
 446 0790 F8FFFFFF 		ld	%s1,-8(,%fp)	# restore
 446      89000101 
 447 0798 D8FFFFFF 		st	%s63,-40(,%fp)	# spill
 447      89003F11 
 448 07a0 F8FFFFFF 		st	%s1,-8(,%fp)	# spill
 448      89000111 
 449 07a8 F0FFFFFF 		st	%s0,-16(,%fp)	# spill
 449      89000011 
 450 07b0 00000000 		lea	%s12,cimagf@PLT_LO(-24)
 450      00680C06 
 451 07b8 00000000 		and	%s12,%s12,(32)0
 451      608C0C44 
 452 07c0 00000000 		sic	%lr
 452      00000A28 
 453 07c8 00000000 		lea.sl	%s12,cimagf@PLT_HI(%s12,%lr)
 453      8A8C8C06 
 454 07d0 00000000 		bsic	%lr,(,%s12)		# cimagf
 454      8C000A08 
 455              	.L_4.2:
 456 07d8 00000000 		cvt.d.s %s63,%s0
 456      00803F0F 
 457 07e0 D0000000 		st	%s63,208(0,%sp)
 457      8B003F11 
 458 07e8 E8FFFFFF 		ld	%s62,-24(,%fp)	# restore
 458      89003E01 
 459 07f0 B0000000 		st	%s62,176(0,%sp)
 459      8B003E11 
 460 07f8 E0FFFFFF 		ld	%s61,-32(,%fp)	# restore
 460      89003D01 
 461 0800 B8000000 		st	%s61,184(0,%sp)
 461      8B003D11 
 462 0808 00000000 		or	%s60,22,(0)1
 462      00163C45 
 463 0810 C0000000 		st	%s60,192(0,%sp)
 463      8B003C11 
 464 0818 D8FFFFFF 		ld	%s59,-40(,%fp)	# restore
 464      89003B01 
 465 0820 C8000000 		st	%s59,200(0,%sp)
 465      8B003B11 
 466 0828 00000000 		or	%s0,0,%s62
 466      BE000045 
 467 0830 00000000 		or	%s1,0,%s61
 467      BD000145 
 468 0838 00000000 		or	%s2,0,%s60
 468      BC000245 
 469 0840 00000000 		or	%s3,0,%s59
 469      BB000345 
 470 0848 00000000 		or	%s4,0,%s63
 470      BF000445 
 471 0850 00000000 		lea	%s12,printf@PLT_LO(-24)
 471      00680C06 
 472 0858 00000000 		and	%s12,%s12,(32)0
 472      608C0C44 
 473 0860 00000000 		sic	%lr
 473      00000A28 
 474 0868 00000000 		lea.sl	%s12,printf@PLT_HI(%s12,%lr)
 474      8A8C8C06 
 475 0870 00000000 		bsic	%lr,(,%s12)		# printf
 475      8C000A08 
 476 0878 08000000 		br.l	.L_4.1
 476      00000F18 
 477              	.L_4.1:
 478              	# line 23
  23:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/fq_fn_fq.c **** 	return ret;
 479              		.loc	1 23 0
 480 0880 F0FFFFFF 		ld	%s0,-16(,%fp)	# restore
 480      89000001 
 481 0888 F8FFFFFF 		ld	%s1,-8(,%fp)	# restore
 481      89000101 
 482              	# Start of epilogue codes
 483 0890 00000000 		or	%sp,0,%fp
 483      89000B45 
 484              		.cfi_def_cfa	11,8
 485 0898 18000000 		ld	%got,0x18(,%sp)
 485      8B000F01 
 486 08a0 20000000 		ld	%plt,0x20(,%sp)
 486      8B001001 
 487 08a8 08000000 		ld	%lr,0x8(,%sp)
 487      8B000A01 
 488 08b0 00000000 		ld	%fp,0x0(,%sp)
 488      8B000901 
 489 08b8 00000000 		b.l	(,%lr)
 489      8A000F19 
 490              	.L_4.EoE:
 491              		.cfi_endproc
 492              		.set	.L.4.2auto_size,	0xfffffffffffffdd0	# 560 Bytes
 494              	# ============ End  prt_fq_fn_fq ============
 495              	.Le1.0:
