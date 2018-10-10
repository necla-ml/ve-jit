   1              		.ident "ncc 1.1.4 (Build 10:17:22 Apr 19 2018)"
   2              		.file "int_fn_void.c"
   3              		.file 1 "/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/int_fn_void.c"
   4              		.file 2 "/opt/nec/ve/ncc/1.1.4/include/stdc-predef.h"
   5              		.file 3 "/opt/nec/ve/ncc/1.1.4/include/stdio.h"
   6              		.file 4 "/opt/nec/ve/ncc/1.1.4/include/yvals.h"
   7              		.file 5 "/opt/nec/ve/ncc/1.1.4/include/necvals.h"
   8              		.file 6 "/opt/nec/ve/ncc/1.1.4/include/stdarg.h"
   9              		.file 7 "/opt/nec/ve/musl/include/stdio.h"
  10              		.file 8 "/opt/nec/ve/musl/include/features.h"
  11              		.file 9 "/opt/nec/ve/musl/include/bits/alltypes.h"
  12              	# ============ Begin  int_fn_void ============
  13              		.section	.bss
  14              		.local	int_ret
  15              	.comm int_ret,4,4
  16              		.text
  17              		.balign 16
  18              	.L_1.0:
  19              	# line 6
   1:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/int_fn_void.c **** #include <stdio.h>
   2:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/int_fn_void.c **** 
   3:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/int_fn_void.c **** extern int ext_int_fn_void();
   4:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/int_fn_void.c **** 
   5:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/int_fn_void.c **** static int int_ret;
   6:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/int_fn_void.c **** int int_fn_void() {
  20              		.loc	1 6 0
  21              		.globl	int_fn_void
  23              	int_fn_void:
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
  53 00c8 40FEFFFF 		lea	%s13,.L.1.2auto_size&0xffffffff
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
   7:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/int_fn_void.c **** 	int_ret = ext_int_fn_void();
  68              		.loc	1 7 0
  69 0128 00000000 		lea	%s12,ext_int_fn_void@PLT_LO(-24)
  69      00680C06 
  70 0130 00000000 		and	%s12,%s12,(32)0
  70      608C0C44 
  71 0138 00000000 		sic	%lr
  71      00000A28 
  72 0140 00000000 		lea.sl	%s12,ext_int_fn_void@PLT_HI(%s12,%lr)
  72      8A8C8C06 
  73 0148 00000000 		bsic	%lr,(,%s12)		# ext_int_fn_void
  73      8C000A08 
  74 0150 F8FFFFFF 		st	%s0,-8(,%fp)	# spill
  74      89000011 
  75 0158 08000000 		br.l	.L_1.1
  75      00000F18 
  76              	.L_1.1:
  77 0160 00000000 		lea	%s63,int_ret@GOTOFF_LO
  77      00003F06 
  78 0168 00000000 		and	%s63,%s63,(32)0
  78      60BF3F44 
  79 0170 00000000 		lea.sl	%s63,int_ret@GOTOFF_HI(%s63,%got)
  79      8FBFBF06 
  80 0178 F8FFFFFF 		ld	%s62,-8(,%fp)	# restore
  80      89003E01 
  81 0180 00000000 		stl	%s62,0(0,%s63)	# int_ret
  81      BF003E13 
  82              	# line 8
   8:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/int_fn_void.c **** 	return int_ret;
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
  95              		.set	.L.1.2auto_size,	0xfffffffffffffe40	# 448 Bytes
  97              	# ============ End  int_fn_void ============
  98              	# ============ Begin  int_fn_void2 ============
  99              		.balign 16
 100              	.L_2.0:
 101              	# line 10
   9:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/int_fn_void.c **** }
  10:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/int_fn_void.c **** int int_fn_void2() {
 102              		.loc	1 10 0
 103              		.globl	int_fn_void2
 105              	int_fn_void2:
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
 135 0288 30FEFFFF 		lea	%s13,.L.2.2auto_size&0xffffffff
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
  11:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/int_fn_void.c **** 	int *p_ret = &int_ret;
 150              		.loc	1 11 0
 151 02e8 00000000 		lea	%s63,int_ret@GOTOFF_LO
 151      00003F06 
 152 02f0 00000000 		and	%s63,%s63,(32)0
 152      60BF3F44 
 153 02f8 00000000 		lea.sl	%s63,int_ret@GOTOFF_HI(%s63,%got)
 153      8FBFBF06 
 154              	# line 12
  12:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/int_fn_void.c **** 	*p_ret = ext_int_fn_void();
 155              		.loc	1 12 0
 156 0300 F8FFFFFF 		st	%s63,-8(,%fp)	# spill
 156      89003F11 
 157 0308 00000000 		lea	%s12,ext_int_fn_void@PLT_LO(-24)
 157      00680C06 
 158 0310 00000000 		and	%s12,%s12,(32)0
 158      608C0C44 
 159 0318 00000000 		sic	%lr
 159      00000A28 
 160 0320 00000000 		lea.sl	%s12,ext_int_fn_void@PLT_HI(%s12,%lr)
 160      8A8C8C06 
 161 0328 00000000 		bsic	%lr,(,%s12)		# ext_int_fn_void
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
 167 0350 00000000 		stl	%s62,0(0,%s63)	# *(p_ret)
 167      BF003E13 
 168              	# line 13
  13:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/int_fn_void.c **** 	return *p_ret;
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
 181              		.set	.L.2.2auto_size,	0xfffffffffffffe30	# 464 Bytes
 183              	# ============ End  int_fn_void2 ============
 184              	# ============ Begin  mt_int_fn_void ============
 185              		.balign 16
 186              	.L_3.0:
 187              	# line 15
  14:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/int_fn_void.c **** }
  15:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/int_fn_void.c **** int mt_int_fn_void() {
 188              		.loc	1 15 0
 189              		.globl	mt_int_fn_void
 191              	mt_int_fn_void:
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
  16:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/int_fn_void.c **** 	return 99;
 236              		.loc	1 16 0
 237 04b8 63000000 		lea	%s63,99
 237      00003F06 
 238 04c0 00000000 		or	%s0,0,%s63
 238      BF000045 
 239              	# Start of epilogue codes
 240 04c8 00000000 		or	%sp,0,%fp
 240      89000B45 
 241              		.cfi_def_cfa	11,8
 242 04d0 18000000 		ld	%got,0x18(,%sp)
 242      8B000F01 
 243 04d8 20000000 		ld	%plt,0x20(,%sp)
 243      8B001001 
 244 04e0 08000000 		ld	%lr,0x8(,%sp)
 244      8B000A01 
 245 04e8 00000000 		ld	%fp,0x0(,%sp)
 245      8B000901 
 246 04f0 00000000 		b.l	(,%lr)
 246      8A000F19 
 247              	.L_3.EoE:
 248              		.cfi_endproc
 249              		.set	.L.3.2auto_size,	0x0	# 0 Bytes
 251              	# ============ End  mt_int_fn_void ============
 252              	# ============ Begin  prt_int_fn_void ============
 253              		.section .rodata
 254              		.balign 16
 256              	.LP.__string.0:
 257 0000 25       		.byte	37
 258 0001 73       		.byte	115
 259 0002 3A       		.byte	58
 260 0003 25       		.byte	37
 261 0004 75       		.byte	117
 262 0005 20       		.byte	32
 263 0006 62       		.byte	98
 264 0007 6F       		.byte	111
 265 0008 6F       		.byte	111
 266 0009 6C       		.byte	108
 267 000a 20       		.byte	32
 268 000b 25       		.byte	37
 269 000c 64       		.byte	100
 270 000d 0A       		.byte	10
 271 000e 00       		.zero	1
 272 000f 00       		.balign 8
 274              	.LP.__string.1:
 275 0010 2F       		.byte	47
 276 0011 75       		.byte	117
 277 0012 73       		.byte	115
 278 0013 72       		.byte	114
 279 0014 2F       		.byte	47
 280 0015 75       		.byte	117
 281 0016 68       		.byte	104
 282 0017 6F       		.byte	111
 283 0018 6D       		.byte	109
 284 0019 65       		.byte	101
 285 001a 2F       		.byte	47
 286 001b 61       		.byte	97
 287 001c 75       		.byte	117
 288 001d 72       		.byte	114
 289 001e 6F       		.byte	111
 290 001f 72       		.byte	114
 291 0020 61       		.byte	97
 292 0021 2F       		.byte	47
 293 0022 34       		.byte	52
 294 0023 67       		.byte	103
 295 0024 69       		.byte	105
 296 0025 2F       		.byte	47
 297 0026 6E       		.byte	110
 298 0027 6C       		.byte	108
 299 0028 61       		.byte	97
 300 0029 62       		.byte	98
 301 002a 68       		.byte	104
 302 002b 70       		.byte	112
 303 002c 67       		.byte	103
 304 002d 2F       		.byte	47
 305 002e 6B       		.byte	107
 306 002f 72       		.byte	114
 307 0030 75       		.byte	117
 308 0031 75       		.byte	117
 309 0032 73       		.byte	115
 310 0033 2F       		.byte	47
 311 0034 76       		.byte	118
 312 0035 74       		.byte	116
 313 0036 2F       		.byte	47
 314 0037 73       		.byte	115
 315 0038 72       		.byte	114
 316 0039 63       		.byte	99
 317 003a 2F       		.byte	47
 318 003b 61       		.byte	97
 319 003c 73       		.byte	115
 320 003d 6D       		.byte	109
 321 003e 2D       		.byte	45
 322 003f 65       		.byte	101
 323 0040 78       		.byte	120
 324 0041 61       		.byte	97
 325 0042 6D       		.byte	109
 326 0043 70       		.byte	112
 327 0044 6C       		.byte	108
 328 0045 65       		.byte	101
 329 0046 73       		.byte	115
 330 0047 2F       		.byte	47
 331 0048 69       		.byte	105
 332 0049 6E       		.byte	110
 333 004a 74       		.byte	116
 334 004b 5F       		.byte	95
 335 004c 66       		.byte	102
 336 004d 6E       		.byte	110
 337 004e 5F       		.byte	95
 338 004f 76       		.byte	118
 339 0050 6F       		.byte	111
 340 0051 69       		.byte	105
 341 0052 64       		.byte	100
 342 0053 2E       		.byte	46
 343 0054 63       		.byte	99
 344 0055 00       		.zero	1
 345              		.text
 346 04f8 00000000 		.balign 16
 346      00000000 
 347              	.L_4.0:
 348              	# line 18
  17:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/int_fn_void.c **** }
  18:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/int_fn_void.c **** int prt_int_fn_void() {
 349              		.loc	1 18 0
 350              		.globl	prt_int_fn_void
 352              	prt_int_fn_void:
 353              		.cfi_startproc
 354 0500 00000000 		st	%fp,0x0(,%sp)
 354      8B000911 
 355              		.cfi_def_cfa_offset	0
 356              		.cfi_offset	9,0
 357 0508 08000000 		st	%lr,0x8(,%sp)
 357      8B000A11 
 358 0510 18000000 		st	%got,0x18(,%sp)
 358      8B000F11 
 359 0518 20000000 		st	%plt,0x20(,%sp)
 359      8B001011 
 360 0520 00000000 		lea	%got,_GLOBAL_OFFSET_TABLE_@PC_LO(-0x18)
 360      00680F06 
 361 0528 00000000 		and	%got,%got,(32)0
 361      608F0F44 
 362 0530 00000000 		sic	%plt
 362      00001028 
 363 0538 00000000 		lea.sl	%got,_GLOBAL_OFFSET_TABLE_@PC_HI(%plt,%got)
 363      8F908F06 
 364 0540 00000000 		or	%fp,0,%sp
 364      8B000945 
 365              		.cfi_def_cfa_register	9
 366 0548 30000000 		st	%s18,48(,%fp)
 366      89001211 
 367 0550 38000000 		st	%s19,56(,%fp)
 367      89001311 
 368 0558 40000000 		st	%s20,64(,%fp)
 368      89001411 
 369 0560 48000000 		st	%s21,72(,%fp)
 369      89001511 
 370 0568 50000000 		st	%s22,80(,%fp)
 370      89001611 
 371 0570 58000000 		st	%s23,88(,%fp)
 371      89001711 
 372 0578 60000000 		st	%s24,96(,%fp)
 372      89001811 
 373 0580 68000000 		st	%s25,104(,%fp)
 373      89001911 
 374 0588 70000000 		st	%s26,112(,%fp)
 374      89001A11 
 375 0590 78000000 		st	%s27,120(,%fp)
 375      89001B11 
 376 0598 80000000 		st	%s28,128(,%fp)
 376      89001C11 
 377 05a0 88000000 		st	%s29,136(,%fp)
 377      89001D11 
 378 05a8 90000000 		st	%s30,144(,%fp)
 378      89001E11 
 379 05b0 98000000 		st	%s31,152(,%fp)
 379      89001F11 
 380 05b8 A0000000 		st	%s32,160(,%fp)
 380      89002011 
 381 05c0 A8000000 		st	%s33,168(,%fp)
 381      89002111 
 382 05c8 30FEFFFF 		lea	%s13,.L.4.2auto_size&0xffffffff
 382      00000D06 
 383 05d0 00000000 		and	%s13,%s13,(32)0
 383      608D0D44 
 384 05d8 FFFFFFFF 		lea.sl	%sp,.L.4.2auto_size>>32(%fp,%s13)
 384      8D898B06 
 385 05e0 48000000 		brge.l.t	%sp,%sl,.L_4.EoP
 385      888B3518 
 386 05e8 18000000 		ld	%s61,0x18(,%tp)
 386      8E003D01 
 387 05f0 00000000 		or	%s62,0,%s0
 387      80003E45 
 388 05f8 3B010000 		lea	%s63,0x13b
 388      00003F06 
 389 0600 00000000 		shm.l	%s63,0x0(%s61)
 389      BD033F31 
 390 0608 08000000 		shm.l	%sl,0x8(%s61)
 390      BD030831 
 391 0610 10000000 		shm.l	%sp,0x10(%s61)
 391      BD030B31 
 392 0618 00000000 		monc
 392      0000003F 
 393 0620 00000000 		or	%s0,0,%s62
 393      BE000045 
 394              	.L_4.EoP:
 395              	# End of prologue codes
 396              	# line 19
  19:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/int_fn_void.c **** 	printf("%s:%u bool %d\n",__FILE__,__LINE__,99);
 397              		.loc	1 19 0
 398 0628 00000000 		lea	%s63,.LP.__string.0@GOTOFF_LO
 398      00003F06 
 399 0630 00000000 		and	%s63,%s63,(32)0
 399      60BF3F44 
 400 0638 00000000 		lea.sl	%s63,.LP.__string.0@GOTOFF_HI(%s63,%got)
 400      8FBFBF06 
 401 0640 B0000000 		st	%s63,176(0,%sp)
 401      8B003F11 
 402 0648 00000000 		lea	%s62,.LP.__string.1@GOTOFF_LO
 402      00003E06 
 403 0650 00000000 		and	%s62,%s62,(32)0
 403      60BE3E44 
 404 0658 00000000 		lea.sl	%s62,.LP.__string.1@GOTOFF_HI(%s62,%got)
 404      8FBEBE06 
 405 0660 B8000000 		st	%s62,184(0,%sp)
 405      8B003E11 
 406 0668 00000000 		or	%s61,19,(0)1
 406      00133D45 
 407 0670 C0000000 		st	%s61,192(0,%sp)
 407      8B003D11 
 408 0678 63000000 		lea	%s60,99
 408      00003C06 
 409 0680 C8000000 		st	%s60,200(0,%sp)
 409      8B003C11 
 410 0688 00000000 		or	%s0,0,%s63
 410      BF000045 
 411 0690 00000000 		or	%s1,0,%s62
 411      BE000145 
 412 0698 00000000 		or	%s2,0,%s61
 412      BD000245 
 413 06a0 00000000 		or	%s3,0,%s60
 413      BC000345 
 414 06a8 00000000 		lea	%s12,printf@PLT_LO(-24)
 414      00680C06 
 415 06b0 00000000 		and	%s12,%s12,(32)0
 415      608C0C44 
 416 06b8 00000000 		sic	%lr
 416      00000A28 
 417 06c0 00000000 		lea.sl	%s12,printf@PLT_HI(%s12,%lr)
 417      8A8C8C06 
 418 06c8 00000000 		bsic	%lr,(,%s12)		# printf
 418      8C000A08 
 419 06d0 08000000 		br.l	.L_4.1
 419      00000F18 
 420              	.L_4.1:
 421              	# line 20
  20:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/int_fn_void.c **** 	return 99;
 422              		.loc	1 20 0
 423 06d8 63000000 		lea	%s63,99
 423      00003F06 
 424 06e0 00000000 		or	%s0,0,%s63
 424      BF000045 
 425              	# Start of epilogue codes
 426 06e8 00000000 		or	%sp,0,%fp
 426      89000B45 
 427              		.cfi_def_cfa	11,8
 428 06f0 18000000 		ld	%got,0x18(,%sp)
 428      8B000F01 
 429 06f8 20000000 		ld	%plt,0x20(,%sp)
 429      8B001001 
 430 0700 08000000 		ld	%lr,0x8(,%sp)
 430      8B000A01 
 431 0708 00000000 		ld	%fp,0x0(,%sp)
 431      8B000901 
 432 0710 00000000 		b.l	(,%lr)
 432      8A000F19 
 433              	.L_4.EoE:
 434              		.cfi_endproc
 435              		.set	.L.4.2auto_size,	0xfffffffffffffe30	# 464 Bytes
 437              	# ============ End  prt_int_fn_void ============
 438              	.Le1.0:
