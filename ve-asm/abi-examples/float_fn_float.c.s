   1              		.ident "ncc 1.1.4 (Build 10:17:22 Apr 19 2018)"
   2              		.file "float_fn_float.c"
   3              		.file 1 "/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/float_fn_float.c"
   4              		.file 2 "/opt/nec/ve/ncc/1.1.4/include/stdc-predef.h"
   5              		.file 3 "/opt/nec/ve/ncc/1.1.4/include/stdio.h"
   6              		.file 4 "/opt/nec/ve/ncc/1.1.4/include/yvals.h"
   7              		.file 5 "/opt/nec/ve/ncc/1.1.4/include/necvals.h"
   8              		.file 6 "/opt/nec/ve/ncc/1.1.4/include/stdarg.h"
   9              		.file 7 "/opt/nec/ve/musl/include/stdio.h"
  10              		.file 8 "/opt/nec/ve/musl/include/features.h"
  11              		.file 9 "/opt/nec/ve/musl/include/bits/alltypes.h"
  12              	# ============ Begin  float_fn_float ============
  13              		.section	.bss
  14              		.local	float_ret
  15              	.comm float_ret,4,4
  16              		.text
  17              		.balign 16
  18              	.L_1.0:
  19              	# line 6
   1:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/float_fn_float.c **** #include <stdio.h>
   2:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/float_fn_float.c **** 
   3:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/float_fn_float.c **** extern float ext_float_fn_float(float f);
   4:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/float_fn_float.c **** 
   5:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/float_fn_float.c **** static float float_ret;
   6:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/float_fn_float.c **** float float_fn_float(float f) {
  20              		.loc	1 6 0
  21              		.globl	float_fn_float
  23              	float_fn_float:
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
  53 00c8 30FEFFFF 		lea	%s13,.L.1.2auto_size&0xffffffff
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
  67              	# line 7
   7:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/float_fn_float.c **** 	float_ret = ext_float_fn_float(f);
  68              		.loc	1 7 0
  69 0128 00000000 		lea	%s12,ext_float_fn_float@PLT_LO(-24)
  69      00680C06 
  70 0130 00000000 		and	%s12,%s12,(32)0
  70      608C0C44 
  71 0138 00000000 		sic	%lr
  71      00000A28 
  72 0140 00000000 		lea.sl	%s12,ext_float_fn_float@PLT_HI(%s12,%lr)
  72      8A8C8C06 
  73 0148 00000000 		bsic	%lr,(,%s12)		# ext_float_fn_float
  73      8C000A08 
  74 0150 F8FFFFFF 		st	%s0,-8(,%fp)	# spill
  74      89000011 
  75 0158 08000000 		br.l	.L_1.1
  75      00000F18 
  76              	.L_1.1:
  77 0160 00000000 		lea	%s63,float_ret@GOTOFF_LO
  77      00003F06 
  78 0168 00000000 		and	%s63,%s63,(32)0
  78      60BF3F44 
  79 0170 00000000 		lea.sl	%s63,float_ret@GOTOFF_HI(%s63,%got)
  79      8FBFBF06 
  80 0178 F8FFFFFF 		ld	%s62,-8(,%fp)	# restore
  80      89003E01 
  81 0180 00000000 		stu	%s62,0(0,%s63)	# float_ret
  81      BF003E12 
  82              	# line 8
   8:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/float_fn_float.c **** 	return float_ret;
  83              		.loc	1 8 0
  84 0188 00000000 		or	%s0,0,%s62
  84      BE000045 
  85              	# Start of epilogue codes
  86 0190 00000000 		or	%sp,0,%fp
  86      89000B45 
  87              		.cfi_def_cfa	11,8
  88 0198 18000000 		ld	%got,0x18(,%sp)
  88      8B000F01 
  89 01a0 20000000 		ld	%plt,0x20(,%sp)
  89      8B001001 
  90 01a8 08000000 		ld	%lr,0x8(,%sp)
  90      8B000A01 
  91 01b0 00000000 		ld	%fp,0x0(,%sp)
  91      8B000901 
  92 01b8 00000000 		b.l	(,%lr)
  92      8A000F19 
  93              	.L_1.EoE:
  94              		.cfi_endproc
  95              		.set	.L.1.2auto_size,	0xfffffffffffffe30	# 464 Bytes
  97              	# ============ End  float_fn_float ============
  98              	# ============ Begin  float_fn_float2 ============
  99              		.balign 16
 100              	.L_2.0:
 101              	# line 10
   9:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/float_fn_float.c **** }
  10:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/float_fn_float.c **** float float_fn_float2(float f) {
 102              		.loc	1 10 0
 103              		.globl	float_fn_float2
 105              	float_fn_float2:
 106              		.cfi_startproc
 107 01c0 00000000 		st	%fp,0x0(,%sp)
 107      8B000911 
 108              		.cfi_def_cfa_offset	0
 109              		.cfi_offset	9,0
 110 01c8 08000000 		st	%lr,0x8(,%sp)
 110      8B000A11 
 111 01d0 18000000 		st	%got,0x18(,%sp)
 111      8B000F11 
 112 01d8 20000000 		st	%plt,0x20(,%sp)
 112      8B001011 
 113 01e0 00000000 		lea	%got,_GLOBAL_OFFSET_TABLE_@PC_LO(-0x18)
 113      00680F06 
 114 01e8 00000000 		and	%got,%got,(32)0
 114      608F0F44 
 115 01f0 00000000 		sic	%plt
 115      00001028 
 116 01f8 00000000 		lea.sl	%got,_GLOBAL_OFFSET_TABLE_@PC_HI(%plt,%got)
 116      8F908F06 
 117 0200 00000000 		or	%fp,0,%sp
 117      8B000945 
 118              		.cfi_def_cfa_register	9
 119 0208 30000000 		st	%s18,48(,%fp)
 119      89001211 
 120 0210 38000000 		st	%s19,56(,%fp)
 120      89001311 
 121 0218 40000000 		st	%s20,64(,%fp)
 121      89001411 
 122 0220 48000000 		st	%s21,72(,%fp)
 122      89001511 
 123 0228 50000000 		st	%s22,80(,%fp)
 123      89001611 
 124 0230 58000000 		st	%s23,88(,%fp)
 124      89001711 
 125 0238 60000000 		st	%s24,96(,%fp)
 125      89001811 
 126 0240 68000000 		st	%s25,104(,%fp)
 126      89001911 
 127 0248 70000000 		st	%s26,112(,%fp)
 127      89001A11 
 128 0250 78000000 		st	%s27,120(,%fp)
 128      89001B11 
 129 0258 80000000 		st	%s28,128(,%fp)
 129      89001C11 
 130 0260 88000000 		st	%s29,136(,%fp)
 130      89001D11 
 131 0268 90000000 		st	%s30,144(,%fp)
 131      89001E11 
 132 0270 98000000 		st	%s31,152(,%fp)
 132      89001F11 
 133 0278 A0000000 		st	%s32,160(,%fp)
 133      89002011 
 134 0280 A8000000 		st	%s33,168(,%fp)
 134      89002111 
 135 0288 20FEFFFF 		lea	%s13,.L.2.2auto_size&0xffffffff
 135      00000D06 
 136 0290 00000000 		and	%s13,%s13,(32)0
 136      608D0D44 
 137 0298 FFFFFFFF 		lea.sl	%sp,.L.2.2auto_size>>32(%fp,%s13)
 137      8D898B06 
 138 02a0 48000000 		brge.l.t	%sp,%sl,.L_2.EoP
 138      888B3518 
 139 02a8 18000000 		ld	%s61,0x18(,%tp)
 139      8E003D01 
 140 02b0 00000000 		or	%s62,0,%s0
 140      80003E45 
 141 02b8 3B010000 		lea	%s63,0x13b
 141      00003F06 
 142 02c0 00000000 		shm.l	%s63,0x0(%s61)
 142      BD033F31 
 143 02c8 08000000 		shm.l	%sl,0x8(%s61)
 143      BD030831 
 144 02d0 10000000 		shm.l	%sp,0x10(%s61)
 144      BD030B31 
 145 02d8 00000000 		monc
 145      0000003F 
 146 02e0 00000000 		or	%s0,0,%s62
 146      BE000045 
 147              	.L_2.EoP:
 148              	# End of prologue codes
 149              	# line 11
  11:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/float_fn_float.c **** 	float *p_ret = &float_ret;
 150              		.loc	1 11 0
 151 02e8 00000000 		lea	%s63,float_ret@GOTOFF_LO
 151      00003F06 
 152 02f0 00000000 		and	%s63,%s63,(32)0
 152      60BF3F44 
 153 02f8 00000000 		lea.sl	%s63,float_ret@GOTOFF_HI(%s63,%got)
 153      8FBFBF06 
 154              	# line 12
  12:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/float_fn_float.c **** 	*p_ret = ext_float_fn_float(f);
 155              		.loc	1 12 0
 156 0300 F8FFFFFF 		st	%s63,-8(,%fp)	# spill
 156      89003F11 
 157 0308 00000000 		lea	%s12,ext_float_fn_float@PLT_LO(-24)
 157      00680C06 
 158 0310 00000000 		and	%s12,%s12,(32)0
 158      608C0C44 
 159 0318 00000000 		sic	%lr
 159      00000A28 
 160 0320 00000000 		lea.sl	%s12,ext_float_fn_float@PLT_HI(%s12,%lr)
 160      8A8C8C06 
 161 0328 00000000 		bsic	%lr,(,%s12)		# ext_float_fn_float
 161      8C000A08 
 162 0330 F0FFFFFF 		st	%s0,-16(,%fp)	# spill
 162      89000011 
 163 0338 08000000 		br.l	.L_2.1
 163      00000F18 
 164              	.L_2.1:
 165 0340 F8FFFFFF 		ld	%s63,-8(,%fp)	# restore
 165      89003F01 
 166 0348 F0FFFFFF 		ld	%s62,-16(,%fp)	# restore
 166      89003E01 
 167 0350 00000000 		stu	%s62,0(0,%s63)	# *(p_ret)
 167      BF003E12 
 168              	# line 13
  13:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/float_fn_float.c **** 	return *p_ret;
 169              		.loc	1 13 0
 170 0358 00000000 		or	%s0,0,%s62
 170      BE000045 
 171              	# Start of epilogue codes
 172 0360 00000000 		or	%sp,0,%fp
 172      89000B45 
 173              		.cfi_def_cfa	11,8
 174 0368 18000000 		ld	%got,0x18(,%sp)
 174      8B000F01 
 175 0370 20000000 		ld	%plt,0x20(,%sp)
 175      8B001001 
 176 0378 08000000 		ld	%lr,0x8(,%sp)
 176      8B000A01 
 177 0380 00000000 		ld	%fp,0x0(,%sp)
 177      8B000901 
 178 0388 00000000 		b.l	(,%lr)
 178      8A000F19 
 179              	.L_2.EoE:
 180              		.cfi_endproc
 181              		.set	.L.2.2auto_size,	0xfffffffffffffe20	# 480 Bytes
 183              	# ============ End  float_fn_float2 ============
 184              	# ============ Begin  mt_float_fn_float ============
 185              		.balign 16
 186              	.L_3.0:
 187              	# line 15
  14:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/float_fn_float.c **** }
  15:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/float_fn_float.c **** float mt_float_fn_float(float f) {
 188              		.loc	1 15 0
 189              		.globl	mt_float_fn_float
 191              	mt_float_fn_float:
 192              		.cfi_startproc
 193 0390 00000000 		st	%fp,0x0(,%sp)
 193      8B000911 
 194              		.cfi_def_cfa_offset	0
 195              		.cfi_offset	9,0
 196 0398 08000000 		st	%lr,0x8(,%sp)
 196      8B000A11 
 197 03a0 18000000 		st	%got,0x18(,%sp)
 197      8B000F11 
 198 03a8 20000000 		st	%plt,0x20(,%sp)
 198      8B001011 
 199 03b0 00000000 		lea	%got,_GLOBAL_OFFSET_TABLE_@PC_LO(-0x18)
 199      00680F06 
 200 03b8 00000000 		and	%got,%got,(32)0
 200      608F0F44 
 201 03c0 00000000 		sic	%plt
 201      00001028 
 202 03c8 00000000 		lea.sl	%got,_GLOBAL_OFFSET_TABLE_@PC_HI(%plt,%got)
 202      8F908F06 
 203 03d0 00000000 		or	%fp,0,%sp
 203      8B000945 
 204              		.cfi_def_cfa_register	9
 205 03d8 30000000 		st	%s18,48(,%fp)
 205      89001211 
 206 03e0 38000000 		st	%s19,56(,%fp)
 206      89001311 
 207 03e8 40000000 		st	%s20,64(,%fp)
 207      89001411 
 208 03f0 48000000 		st	%s21,72(,%fp)
 208      89001511 
 209 03f8 50000000 		st	%s22,80(,%fp)
 209      89001611 
 210 0400 58000000 		st	%s23,88(,%fp)
 210      89001711 
 211 0408 60000000 		st	%s24,96(,%fp)
 211      89001811 
 212 0410 68000000 		st	%s25,104(,%fp)
 212      89001911 
 213 0418 70000000 		st	%s26,112(,%fp)
 213      89001A11 
 214 0420 78000000 		st	%s27,120(,%fp)
 214      89001B11 
 215 0428 80000000 		st	%s28,128(,%fp)
 215      89001C11 
 216 0430 88000000 		st	%s29,136(,%fp)
 216      89001D11 
 217 0438 90000000 		st	%s30,144(,%fp)
 217      89001E11 
 218 0440 98000000 		st	%s31,152(,%fp)
 218      89001F11 
 219 0448 A0000000 		st	%s32,160(,%fp)
 219      89002011 
 220 0450 A8000000 		st	%s33,168(,%fp)
 220      89002111 
 221 0458 00000000 		lea	%s13,.L.3.2auto_size&0xffffffff
 221      00000D06 
 222 0460 00000000 		and	%s13,%s13,(32)0
 222      608D0D44 
 223 0468 00000000 		lea.sl	%sp,.L.3.2auto_size>>32(%fp,%s13)
 223      8D898B06 
 224 0470 48000000 		brge.l.t	%sp,%sl,.L_3.EoP
 224      888B3518 
 225 0478 18000000 		ld	%s61,0x18(,%tp)
 225      8E003D01 
 226 0480 00000000 		or	%s62,0,%s0
 226      80003E45 
 227 0488 3B010000 		lea	%s63,0x13b
 227      00003F06 
 228 0490 00000000 		shm.l	%s63,0x0(%s61)
 228      BD033F31 
 229 0498 08000000 		shm.l	%sl,0x8(%s61)
 229      BD030831 
 230 04a0 10000000 		shm.l	%sp,0x10(%s61)
 230      BD030B31 
 231 04a8 00000000 		monc
 231      0000003F 
 232 04b0 00000000 		or	%s0,0,%s62
 232      BE000045 
 233              	.L_3.EoP:
 234              	# End of prologue codes
 235              	# line 16
  16:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/float_fn_float.c **** 	float ret = f+1.49;
 236              		.loc	1 16 0
 237 04b8 00000000 		cvt.d.s %s63,%s0
 237      00803F0F 
 238 04c0 D7A3703D 		lea	%s62,1030792151
 238      00003E06 
 239 04c8 00000000 		and	%s61,%s62,(32)0
 239      60BE3D44 
 240 04d0 0AD7F73F 		lea.sl	%s60,1073207050(,%s61)
 240      BD00BC06 
 241 04d8 00000000 		fadd.d	%s59,%s63,%s60
 241      BCBF3B4C 
 242 04e0 00000000 		cvt.s.d %s58,%s59
 242      00BB3A1F 
 243              	# line 17
  17:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/float_fn_float.c **** 	return ret;
 244              		.loc	1 17 0
 245 04e8 00000000 		or	%s0,0,%s58
 245      BA000045 
 246              	# Start of epilogue codes
 247 04f0 00000000 		or	%sp,0,%fp
 247      89000B45 
 248              		.cfi_def_cfa	11,8
 249 04f8 18000000 		ld	%got,0x18(,%sp)
 249      8B000F01 
 250 0500 20000000 		ld	%plt,0x20(,%sp)
 250      8B001001 
 251 0508 08000000 		ld	%lr,0x8(,%sp)
 251      8B000A01 
 252 0510 00000000 		ld	%fp,0x0(,%sp)
 252      8B000901 
 253 0518 00000000 		b.l	(,%lr)
 253      8A000F19 
 254              	.L_3.EoE:
 255              		.cfi_endproc
 256              		.set	.L.3.2auto_size,	0x0	# 0 Bytes
 258              	# ============ End  mt_float_fn_float ============
 259              	# ============ Begin  prt_float_fn_float ============
 260              		.section .rodata
 261              		.balign 16
 263              	.LP.__string.0:
 264 0000 25       		.byte	37
 265 0001 73       		.byte	115
 266 0002 3A       		.byte	58
 267 0003 25       		.byte	37
 268 0004 75       		.byte	117
 269 0005 20       		.byte	32
 270 0006 62       		.byte	98
 271 0007 6F       		.byte	111
 272 0008 6F       		.byte	111
 273 0009 6C       		.byte	108
 274 000a 20       		.byte	32
 275 000b 25       		.byte	37
 276 000c 64       		.byte	100
 277 000d 0A       		.byte	10
 278 000e 00       		.zero	1
 279 000f 00       		.balign 8
 281              	.LP.__string.1:
 282 0010 2F       		.byte	47
 283 0011 75       		.byte	117
 284 0012 73       		.byte	115
 285 0013 72       		.byte	114
 286 0014 2F       		.byte	47
 287 0015 75       		.byte	117
 288 0016 68       		.byte	104
 289 0017 6F       		.byte	111
 290 0018 6D       		.byte	109
 291 0019 65       		.byte	101
 292 001a 2F       		.byte	47
 293 001b 61       		.byte	97
 294 001c 75       		.byte	117
 295 001d 72       		.byte	114
 296 001e 6F       		.byte	111
 297 001f 72       		.byte	114
 298 0020 61       		.byte	97
 299 0021 2F       		.byte	47
 300 0022 34       		.byte	52
 301 0023 67       		.byte	103
 302 0024 69       		.byte	105
 303 0025 2F       		.byte	47
 304 0026 6E       		.byte	110
 305 0027 6C       		.byte	108
 306 0028 61       		.byte	97
 307 0029 62       		.byte	98
 308 002a 68       		.byte	104
 309 002b 70       		.byte	112
 310 002c 67       		.byte	103
 311 002d 2F       		.byte	47
 312 002e 6B       		.byte	107
 313 002f 72       		.byte	114
 314 0030 75       		.byte	117
 315 0031 75       		.byte	117
 316 0032 73       		.byte	115
 317 0033 2F       		.byte	47
 318 0034 76       		.byte	118
 319 0035 74       		.byte	116
 320 0036 2F       		.byte	47
 321 0037 73       		.byte	115
 322 0038 72       		.byte	114
 323 0039 63       		.byte	99
 324 003a 2F       		.byte	47
 325 003b 61       		.byte	97
 326 003c 73       		.byte	115
 327 003d 6D       		.byte	109
 328 003e 2D       		.byte	45
 329 003f 65       		.byte	101
 330 0040 78       		.byte	120
 331 0041 61       		.byte	97
 332 0042 6D       		.byte	109
 333 0043 70       		.byte	112
 334 0044 6C       		.byte	108
 335 0045 65       		.byte	101
 336 0046 73       		.byte	115
 337 0047 2F       		.byte	47
 338 0048 66       		.byte	102
 339 0049 6C       		.byte	108
 340 004a 6F       		.byte	111
 341 004b 61       		.byte	97
 342 004c 74       		.byte	116
 343 004d 5F       		.byte	95
 344 004e 66       		.byte	102
 345 004f 6E       		.byte	110
 346 0050 5F       		.byte	95
 347 0051 66       		.byte	102
 348 0052 6C       		.byte	108
 349 0053 6F       		.byte	111
 350 0054 61       		.byte	97
 351 0055 74       		.byte	116
 352 0056 2E       		.byte	46
 353 0057 63       		.byte	99
 354 0058 00       		.zero	1
 355              		.text
 356              		.balign 16
 357              	.L_4.0:
 358              	# line 19
  18:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/float_fn_float.c **** }
  19:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/float_fn_float.c **** float prt_float_fn_float(float f) {
 359              		.loc	1 19 0
 360              		.globl	prt_float_fn_float
 362              	prt_float_fn_float:
 363              		.cfi_startproc
 364 0520 00000000 		st	%fp,0x0(,%sp)
 364      8B000911 
 365              		.cfi_def_cfa_offset	0
 366              		.cfi_offset	9,0
 367 0528 08000000 		st	%lr,0x8(,%sp)
 367      8B000A11 
 368 0530 18000000 		st	%got,0x18(,%sp)
 368      8B000F11 
 369 0538 20000000 		st	%plt,0x20(,%sp)
 369      8B001011 
 370 0540 00000000 		lea	%got,_GLOBAL_OFFSET_TABLE_@PC_LO(-0x18)
 370      00680F06 
 371 0548 00000000 		and	%got,%got,(32)0
 371      608F0F44 
 372 0550 00000000 		sic	%plt
 372      00001028 
 373 0558 00000000 		lea.sl	%got,_GLOBAL_OFFSET_TABLE_@PC_HI(%plt,%got)
 373      8F908F06 
 374 0560 00000000 		or	%fp,0,%sp
 374      8B000945 
 375              		.cfi_def_cfa_register	9
 376 0568 30000000 		st	%s18,48(,%fp)
 376      89001211 
 377 0570 38000000 		st	%s19,56(,%fp)
 377      89001311 
 378 0578 40000000 		st	%s20,64(,%fp)
 378      89001411 
 379 0580 48000000 		st	%s21,72(,%fp)
 379      89001511 
 380 0588 50000000 		st	%s22,80(,%fp)
 380      89001611 
 381 0590 58000000 		st	%s23,88(,%fp)
 381      89001711 
 382 0598 60000000 		st	%s24,96(,%fp)
 382      89001811 
 383 05a0 68000000 		st	%s25,104(,%fp)
 383      89001911 
 384 05a8 70000000 		st	%s26,112(,%fp)
 384      89001A11 
 385 05b0 78000000 		st	%s27,120(,%fp)
 385      89001B11 
 386 05b8 80000000 		st	%s28,128(,%fp)
 386      89001C11 
 387 05c0 88000000 		st	%s29,136(,%fp)
 387      89001D11 
 388 05c8 90000000 		st	%s30,144(,%fp)
 388      89001E11 
 389 05d0 98000000 		st	%s31,152(,%fp)
 389      89001F11 
 390 05d8 A0000000 		st	%s32,160(,%fp)
 390      89002011 
 391 05e0 A8000000 		st	%s33,168(,%fp)
 391      89002111 
 392 05e8 20FEFFFF 		lea	%s13,.L.4.2auto_size&0xffffffff
 392      00000D06 
 393 05f0 00000000 		and	%s13,%s13,(32)0
 393      608D0D44 
 394 05f8 FFFFFFFF 		lea.sl	%sp,.L.4.2auto_size>>32(%fp,%s13)
 394      8D898B06 
 395 0600 48000000 		brge.l.t	%sp,%sl,.L_4.EoP
 395      888B3518 
 396 0608 18000000 		ld	%s61,0x18(,%tp)
 396      8E003D01 
 397 0610 00000000 		or	%s62,0,%s0
 397      80003E45 
 398 0618 3B010000 		lea	%s63,0x13b
 398      00003F06 
 399 0620 00000000 		shm.l	%s63,0x0(%s61)
 399      BD033F31 
 400 0628 08000000 		shm.l	%sl,0x8(%s61)
 400      BD030831 
 401 0630 10000000 		shm.l	%sp,0x10(%s61)
 401      BD030B31 
 402 0638 00000000 		monc
 402      0000003F 
 403 0640 00000000 		or	%s0,0,%s62
 403      BE000045 
 404              	.L_4.EoP:
 405              	# End of prologue codes
 406              	# line 20
  20:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/float_fn_float.c **** 	float ret = f + 1.49;
 407              		.loc	1 20 0
 408 0648 00000000 		cvt.d.s %s63,%s0
 408      00803F0F 
 409 0650 D7A3703D 		lea	%s62,1030792151
 409      00003E06 
 410 0658 00000000 		and	%s61,%s62,(32)0
 410      60BE3D44 
 411 0660 0AD7F73F 		lea.sl	%s60,1073207050(,%s61)
 411      BD00BC06 
 412 0668 00000000 		fadd.d	%s59,%s63,%s60
 412      BCBF3B4C 
 413 0670 00000000 		cvt.s.d %s58,%s59
 413      00BB3A1F 
 414              	# line 21
  21:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/float_fn_float.c **** 	printf("%s:%u bool %d\n",__FILE__,__LINE__,ret);
 415              		.loc	1 21 0
 416 0678 00000000 		cvt.d.s %s57,%s58
 416      00BA390F 
 417 0680 C8000000 		st	%s57,200(0,%sp)
 417      8B003911 
 418 0688 00000000 		lea	%s56,.LP.__string.0@GOTOFF_LO
 418      00003806 
 419 0690 00000000 		and	%s56,%s56,(32)0
 419      60B83844 
 420 0698 00000000 		lea.sl	%s56,.LP.__string.0@GOTOFF_HI(%s56,%got)
 420      8FB8B806 
 421 06a0 B0000000 		st	%s56,176(0,%sp)
 421      8B003811 
 422 06a8 00000000 		lea	%s55,.LP.__string.1@GOTOFF_LO
 422      00003706 
 423 06b0 00000000 		and	%s55,%s55,(32)0
 423      60B73744 
 424 06b8 00000000 		lea.sl	%s55,.LP.__string.1@GOTOFF_HI(%s55,%got)
 424      8FB7B706 
 425 06c0 B8000000 		st	%s55,184(0,%sp)
 425      8B003711 
 426 06c8 00000000 		or	%s54,21,(0)1
 426      00153645 
 427 06d0 C0000000 		st	%s54,192(0,%sp)
 427      8B003611 
 428 06d8 00000000 		or	%s0,0,%s56
 428      B8000045 
 429 06e0 00000000 		or	%s1,0,%s55
 429      B7000145 
 430 06e8 00000000 		or	%s2,0,%s54
 430      B6000245 
 431 06f0 00000000 		or	%s3,0,%s57
 431      B9000345 
 432 06f8 F8FFFFFF 		st	%s58,-8(,%fp)	# spill
 432      89003A11 
 433 0700 00000000 		lea	%s12,printf@PLT_LO(-24)
 433      00680C06 
 434 0708 00000000 		and	%s12,%s12,(32)0
 434      608C0C44 
 435 0710 00000000 		sic	%lr
 435      00000A28 
 436 0718 00000000 		lea.sl	%s12,printf@PLT_HI(%s12,%lr)
 436      8A8C8C06 
 437 0720 00000000 		bsic	%lr,(,%s12)		# printf
 437      8C000A08 
 438 0728 08000000 		br.l	.L_4.1
 438      00000F18 
 439              	.L_4.1:
 440              	# line 22
  22:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/float_fn_float.c **** 	return ret;
 441              		.loc	1 22 0
 442 0730 F8FFFFFF 		ld	%s0,-8(,%fp)	# restore
 442      89000001 
 443              	# Start of epilogue codes
 444 0738 00000000 		or	%sp,0,%fp
 444      89000B45 
 445              		.cfi_def_cfa	11,8
 446 0740 18000000 		ld	%got,0x18(,%sp)
 446      8B000F01 
 447 0748 20000000 		ld	%plt,0x20(,%sp)
 447      8B001001 
 448 0750 08000000 		ld	%lr,0x8(,%sp)
 448      8B000A01 
 449 0758 00000000 		ld	%fp,0x0(,%sp)
 449      8B000901 
 450 0760 00000000 		b.l	(,%lr)
 450      8A000F19 
 451              	.L_4.EoE:
 452              		.cfi_endproc
 453              		.set	.L.4.2auto_size,	0xfffffffffffffe20	# 480 Bytes
 455              	# ============ End  prt_float_fn_float ============
 456              	.Le1.0:
