   1              		.ident "ncc 1.1.4 (Build 10:17:22 Apr 19 2018)"
   2              		.file "void_fn_bool.c"
   3              		.file 1 "/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/void_fn_bool.c"
   4              		.file 2 "/opt/nec/ve/ncc/1.1.4/include/stdc-predef.h"
   5              		.file 3 "/opt/nec/ve/ncc/1.1.4/include/stdio.h"
   6              		.file 4 "/opt/nec/ve/ncc/1.1.4/include/yvals.h"
   7              		.file 5 "/opt/nec/ve/ncc/1.1.4/include/necvals.h"
   8              		.file 6 "/opt/nec/ve/ncc/1.1.4/include/stdarg.h"
   9              		.file 7 "/opt/nec/ve/musl/include/stdio.h"
  10              		.file 8 "/opt/nec/ve/musl/include/features.h"
  11              		.file 9 "/opt/nec/ve/musl/include/bits/alltypes.h"
  12              		.file 10 "/opt/nec/ve/musl/include/stdbool.h"
  13              	# ============ Begin  void_fn_bool ============
  14              		.text
  15              		.balign 16
  16              	.L_1.0:
  17              	# line 9
   1:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/void_fn_bool.c **** #include <stdio.h>
   2:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/void_fn_bool.c **** 
   3:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/void_fn_bool.c **** /* compile with -std=c99 will enable C 'bool' type */
   4:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/void_fn_bool.c **** #include <stdbool.h>
   5:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/void_fn_bool.c **** 
   6:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/void_fn_bool.c **** #ifdef __bool_true_false_are_defined
   7:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/void_fn_bool.c **** extern void ext_void_fn_bool(bool b);
   8:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/void_fn_bool.c **** 
   9:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/void_fn_bool.c **** void void_fn_bool(bool b) {
  18              		.loc	1 9 0
  19              		.globl	void_fn_bool
  21              	void_fn_bool:
  22              		.cfi_startproc
  23 0000 00000000 		st	%fp,0x0(,%sp)
  23      8B000911 
  24              		.cfi_def_cfa_offset	0
  25              		.cfi_offset	9,0
  26 0008 08000000 		st	%lr,0x8(,%sp)
  26      8B000A11 
  27 0010 18000000 		st	%got,0x18(,%sp)
  27      8B000F11 
  28 0018 20000000 		st	%plt,0x20(,%sp)
  28      8B001011 
  29 0020 00000000 		lea	%got,_GLOBAL_OFFSET_TABLE_@PC_LO(-0x18)
  29      00680F06 
  30 0028 00000000 		and	%got,%got,(32)0
  30      608F0F44 
  31 0030 00000000 		sic	%plt
  31      00001028 
  32 0038 00000000 		lea.sl	%got,_GLOBAL_OFFSET_TABLE_@PC_HI(%plt,%got)
  32      8F908F06 
  33 0040 00000000 		or	%fp,0,%sp
  33      8B000945 
  34              		.cfi_def_cfa_register	9
  35 0048 30000000 		st	%s18,48(,%fp)
  35      89001211 
  36 0050 38000000 		st	%s19,56(,%fp)
  36      89001311 
  37 0058 40000000 		st	%s20,64(,%fp)
  37      89001411 
  38 0060 48000000 		st	%s21,72(,%fp)
  38      89001511 
  39 0068 50000000 		st	%s22,80(,%fp)
  39      89001611 
  40 0070 58000000 		st	%s23,88(,%fp)
  40      89001711 
  41 0078 60000000 		st	%s24,96(,%fp)
  41      89001811 
  42 0080 68000000 		st	%s25,104(,%fp)
  42      89001911 
  43 0088 70000000 		st	%s26,112(,%fp)
  43      89001A11 
  44 0090 78000000 		st	%s27,120(,%fp)
  44      89001B11 
  45 0098 80000000 		st	%s28,128(,%fp)
  45      89001C11 
  46 00a0 88000000 		st	%s29,136(,%fp)
  46      89001D11 
  47 00a8 90000000 		st	%s30,144(,%fp)
  47      89001E11 
  48 00b0 98000000 		st	%s31,152(,%fp)
  48      89001F11 
  49 00b8 A0000000 		st	%s32,160(,%fp)
  49      89002011 
  50 00c0 A8000000 		st	%s33,168(,%fp)
  50      89002111 
  51 00c8 40FEFFFF 		lea	%s13,.L.1.2auto_size&0xffffffff
  51      00000D06 
  52 00d0 00000000 		and	%s13,%s13,(32)0
  52      608D0D44 
  53 00d8 FFFFFFFF 		lea.sl	%sp,.L.1.2auto_size>>32(%fp,%s13)
  53      8D898B06 
  54 00e0 48000000 		brge.l.t	%sp,%sl,.L_1.EoP
  54      888B3518 
  55 00e8 18000000 		ld	%s61,0x18(,%tp)
  55      8E003D01 
  56 00f0 00000000 		or	%s62,0,%s0
  56      80003E45 
  57 00f8 3B010000 		lea	%s63,0x13b
  57      00003F06 
  58 0100 00000000 		shm.l	%s63,0x0(%s61)
  58      BD033F31 
  59 0108 08000000 		shm.l	%sl,0x8(%s61)
  59      BD030831 
  60 0110 10000000 		shm.l	%sp,0x10(%s61)
  60      BD030B31 
  61 0118 00000000 		monc
  61      0000003F 
  62 0120 00000000 		or	%s0,0,%s62
  62      BE000045 
  63              	.L_1.EoP:
  64              	# End of prologue codes
  65              	# line 10
  10:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/void_fn_bool.c **** 	ext_void_fn_bool(b);
  66              		.loc	1 10 0
  67 0128 00000000 		lea	%s12,ext_void_fn_bool@PLT_LO(-24)
  67      00680C06 
  68 0130 00000000 		and	%s12,%s12,(32)0
  68      608C0C44 
  69 0138 00000000 		sic	%lr
  69      00000A28 
  70 0140 00000000 		lea.sl	%s12,ext_void_fn_bool@PLT_HI(%s12,%lr)
  70      8A8C8C06 
  71 0148 00000000 		bsic	%lr,(,%s12)		# ext_void_fn_bool
  71      8C000A08 
  72 0150 08000000 		br.l	.L_1.1
  72      00000F18 
  73              	.L_1.1:
  74              	# line 11
  11:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/void_fn_bool.c **** }
  75              		.loc	1 11 0
  76              	# Start of epilogue codes
  77 0158 00000000 		or	%sp,0,%fp
  77      89000B45 
  78              		.cfi_def_cfa	11,8
  79 0160 18000000 		ld	%got,0x18(,%sp)
  79      8B000F01 
  80 0168 20000000 		ld	%plt,0x20(,%sp)
  80      8B001001 
  81 0170 08000000 		ld	%lr,0x8(,%sp)
  81      8B000A01 
  82 0178 00000000 		ld	%fp,0x0(,%sp)
  82      8B000901 
  83 0180 00000000 		b.l	(,%lr)
  83      8A000F19 
  84              	.L_1.EoE:
  85              		.cfi_endproc
  86              		.set	.L.1.2auto_size,	0xfffffffffffffe40	# 448 Bytes
  88              	# ============ End  void_fn_bool ============
  89              	# ============ Begin  mt_void_fn_bool ============
  90 0188 00000000 		.balign 16
  90      00000000 
  91              	.L_2.0:
  92              	# line 12
  12:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/void_fn_bool.c **** void mt_void_fn_bool(bool b) {
  93              		.loc	1 12 0
  94              		.globl	mt_void_fn_bool
  96              	mt_void_fn_bool:
  97              		.cfi_startproc
  98 0190 00000000 		st	%fp,0x0(,%sp)
  98      8B000911 
  99              		.cfi_def_cfa_offset	0
 100              		.cfi_offset	9,0
 101 0198 08000000 		st	%lr,0x8(,%sp)
 101      8B000A11 
 102 01a0 18000000 		st	%got,0x18(,%sp)
 102      8B000F11 
 103 01a8 20000000 		st	%plt,0x20(,%sp)
 103      8B001011 
 104 01b0 00000000 		lea	%got,_GLOBAL_OFFSET_TABLE_@PC_LO(-0x18)
 104      00680F06 
 105 01b8 00000000 		and	%got,%got,(32)0
 105      608F0F44 
 106 01c0 00000000 		sic	%plt
 106      00001028 
 107 01c8 00000000 		lea.sl	%got,_GLOBAL_OFFSET_TABLE_@PC_HI(%plt,%got)
 107      8F908F06 
 108 01d0 00000000 		or	%fp,0,%sp
 108      8B000945 
 109              		.cfi_def_cfa_register	9
 110 01d8 30000000 		st	%s18,48(,%fp)
 110      89001211 
 111 01e0 38000000 		st	%s19,56(,%fp)
 111      89001311 
 112 01e8 40000000 		st	%s20,64(,%fp)
 112      89001411 
 113 01f0 48000000 		st	%s21,72(,%fp)
 113      89001511 
 114 01f8 50000000 		st	%s22,80(,%fp)
 114      89001611 
 115 0200 58000000 		st	%s23,88(,%fp)
 115      89001711 
 116 0208 60000000 		st	%s24,96(,%fp)
 116      89001811 
 117 0210 68000000 		st	%s25,104(,%fp)
 117      89001911 
 118 0218 70000000 		st	%s26,112(,%fp)
 118      89001A11 
 119 0220 78000000 		st	%s27,120(,%fp)
 119      89001B11 
 120 0228 80000000 		st	%s28,128(,%fp)
 120      89001C11 
 121 0230 88000000 		st	%s29,136(,%fp)
 121      89001D11 
 122 0238 90000000 		st	%s30,144(,%fp)
 122      89001E11 
 123 0240 98000000 		st	%s31,152(,%fp)
 123      89001F11 
 124 0248 A0000000 		st	%s32,160(,%fp)
 124      89002011 
 125 0250 A8000000 		st	%s33,168(,%fp)
 125      89002111 
 126 0258 00000000 		lea	%s13,.L.2.2auto_size&0xffffffff
 126      00000D06 
 127 0260 00000000 		and	%s13,%s13,(32)0
 127      608D0D44 
 128 0268 00000000 		lea.sl	%sp,.L.2.2auto_size>>32(%fp,%s13)
 128      8D898B06 
 129 0270 48000000 		brge.l.t	%sp,%sl,.L_2.EoP
 129      888B3518 
 130 0278 18000000 		ld	%s61,0x18(,%tp)
 130      8E003D01 
 131 0280 00000000 		or	%s62,0,%s0
 131      80003E45 
 132 0288 3B010000 		lea	%s63,0x13b
 132      00003F06 
 133 0290 00000000 		shm.l	%s63,0x0(%s61)
 133      BD033F31 
 134 0298 08000000 		shm.l	%sl,0x8(%s61)
 134      BD030831 
 135 02a0 10000000 		shm.l	%sp,0x10(%s61)
 135      BD030B31 
 136 02a8 00000000 		monc
 136      0000003F 
 137 02b0 00000000 		or	%s0,0,%s62
 137      BE000045 
 138              	.L_2.EoP:
 139              	# End of prologue codes
 140              	# line 13
  13:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/void_fn_bool.c **** }
 141              		.loc	1 13 0
 142              	# Start of epilogue codes
 143 02b8 00000000 		or	%sp,0,%fp
 143      89000B45 
 144              		.cfi_def_cfa	11,8
 145 02c0 18000000 		ld	%got,0x18(,%sp)
 145      8B000F01 
 146 02c8 20000000 		ld	%plt,0x20(,%sp)
 146      8B001001 
 147 02d0 08000000 		ld	%lr,0x8(,%sp)
 147      8B000A01 
 148 02d8 00000000 		ld	%fp,0x0(,%sp)
 148      8B000901 
 149 02e0 00000000 		b.l	(,%lr)
 149      8A000F19 
 150              	.L_2.EoE:
 151              		.cfi_endproc
 152              		.set	.L.2.2auto_size,	0x0	# 0 Bytes
 154              	# ============ End  mt_void_fn_bool ============
 155              	# ============ Begin  prt_void_fn_bool ============
 156              		.section .rodata
 157              		.balign 16
 159              	.LP.__string.1:
 160 0000 2F       		.byte	47
 161 0001 75       		.byte	117
 162 0002 73       		.byte	115
 163 0003 72       		.byte	114
 164 0004 2F       		.byte	47
 165 0005 75       		.byte	117
 166 0006 68       		.byte	104
 167 0007 6F       		.byte	111
 168 0008 6D       		.byte	109
 169 0009 65       		.byte	101
 170 000a 2F       		.byte	47
 171 000b 61       		.byte	97
 172 000c 75       		.byte	117
 173 000d 72       		.byte	114
 174 000e 6F       		.byte	111
 175 000f 72       		.byte	114
 176 0010 61       		.byte	97
 177 0011 2F       		.byte	47
 178 0012 34       		.byte	52
 179 0013 67       		.byte	103
 180 0014 69       		.byte	105
 181 0015 2F       		.byte	47
 182 0016 6E       		.byte	110
 183 0017 6C       		.byte	108
 184 0018 61       		.byte	97
 185 0019 62       		.byte	98
 186 001a 68       		.byte	104
 187 001b 70       		.byte	112
 188 001c 67       		.byte	103
 189 001d 2F       		.byte	47
 190 001e 6B       		.byte	107
 191 001f 72       		.byte	114
 192 0020 75       		.byte	117
 193 0021 75       		.byte	117
 194 0022 73       		.byte	115
 195 0023 2F       		.byte	47
 196 0024 76       		.byte	118
 197 0025 74       		.byte	116
 198 0026 2F       		.byte	47
 199 0027 73       		.byte	115
 200 0028 72       		.byte	114
 201 0029 63       		.byte	99
 202 002a 2F       		.byte	47
 203 002b 61       		.byte	97
 204 002c 73       		.byte	115
 205 002d 6D       		.byte	109
 206 002e 2D       		.byte	45
 207 002f 65       		.byte	101
 208 0030 78       		.byte	120
 209 0031 61       		.byte	97
 210 0032 6D       		.byte	109
 211 0033 70       		.byte	112
 212 0034 6C       		.byte	108
 213 0035 65       		.byte	101
 214 0036 73       		.byte	115
 215 0037 2F       		.byte	47
 216 0038 76       		.byte	118
 217 0039 6F       		.byte	111
 218 003a 69       		.byte	105
 219 003b 64       		.byte	100
 220 003c 5F       		.byte	95
 221 003d 66       		.byte	102
 222 003e 6E       		.byte	110
 223 003f 5F       		.byte	95
 224 0040 62       		.byte	98
 225 0041 6F       		.byte	111
 226 0042 6F       		.byte	111
 227 0043 6C       		.byte	108
 228 0044 2E       		.byte	46
 229 0045 63       		.byte	99
 230 0046 00       		.zero	1
 231 0047 00       		.balign 8
 233              	.LP.__string.0:
 234 0048 25       		.byte	37
 235 0049 73       		.byte	115
 236 004a 3A       		.byte	58
 237 004b 25       		.byte	37
 238 004c 75       		.byte	117
 239 004d 20       		.byte	32
 240 004e 62       		.byte	98
 241 004f 6F       		.byte	111
 242 0050 6F       		.byte	111
 243 0051 6C       		.byte	108
 244 0052 20       		.byte	32
 245 0053 25       		.byte	37
 246 0054 64       		.byte	100
 247 0055 0A       		.byte	10
 248 0056 00       		.zero	1
 249              		.text
 250 02e8 00000000 		.balign 16
 250      00000000 
 251              	.L_3.0:
 252              	# line 14
  14:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/void_fn_bool.c **** void prt_void_fn_bool(bool b) {
 253              		.loc	1 14 0
 254              		.globl	prt_void_fn_bool
 256              	prt_void_fn_bool:
 257              		.cfi_startproc
 258 02f0 00000000 		st	%fp,0x0(,%sp)
 258      8B000911 
 259              		.cfi_def_cfa_offset	0
 260              		.cfi_offset	9,0
 261 02f8 08000000 		st	%lr,0x8(,%sp)
 261      8B000A11 
 262 0300 18000000 		st	%got,0x18(,%sp)
 262      8B000F11 
 263 0308 20000000 		st	%plt,0x20(,%sp)
 263      8B001011 
 264 0310 00000000 		lea	%got,_GLOBAL_OFFSET_TABLE_@PC_LO(-0x18)
 264      00680F06 
 265 0318 00000000 		and	%got,%got,(32)0
 265      608F0F44 
 266 0320 00000000 		sic	%plt
 266      00001028 
 267 0328 00000000 		lea.sl	%got,_GLOBAL_OFFSET_TABLE_@PC_HI(%plt,%got)
 267      8F908F06 
 268 0330 00000000 		or	%fp,0,%sp
 268      8B000945 
 269              		.cfi_def_cfa_register	9
 270 0338 30000000 		st	%s18,48(,%fp)
 270      89001211 
 271 0340 38000000 		st	%s19,56(,%fp)
 271      89001311 
 272 0348 40000000 		st	%s20,64(,%fp)
 272      89001411 
 273 0350 48000000 		st	%s21,72(,%fp)
 273      89001511 
 274 0358 50000000 		st	%s22,80(,%fp)
 274      89001611 
 275 0360 58000000 		st	%s23,88(,%fp)
 275      89001711 
 276 0368 60000000 		st	%s24,96(,%fp)
 276      89001811 
 277 0370 68000000 		st	%s25,104(,%fp)
 277      89001911 
 278 0378 70000000 		st	%s26,112(,%fp)
 278      89001A11 
 279 0380 78000000 		st	%s27,120(,%fp)
 279      89001B11 
 280 0388 80000000 		st	%s28,128(,%fp)
 280      89001C11 
 281 0390 88000000 		st	%s29,136(,%fp)
 281      89001D11 
 282 0398 90000000 		st	%s30,144(,%fp)
 282      89001E11 
 283 03a0 98000000 		st	%s31,152(,%fp)
 283      89001F11 
 284 03a8 A0000000 		st	%s32,160(,%fp)
 284      89002011 
 285 03b0 A8000000 		st	%s33,168(,%fp)
 285      89002111 
 286 03b8 30FEFFFF 		lea	%s13,.L.3.2auto_size&0xffffffff
 286      00000D06 
 287 03c0 00000000 		and	%s13,%s13,(32)0
 287      608D0D44 
 288 03c8 FFFFFFFF 		lea.sl	%sp,.L.3.2auto_size>>32(%fp,%s13)
 288      8D898B06 
 289 03d0 48000000 		brge.l.t	%sp,%sl,.L_3.EoP
 289      888B3518 
 290 03d8 18000000 		ld	%s61,0x18(,%tp)
 290      8E003D01 
 291 03e0 00000000 		or	%s62,0,%s0
 291      80003E45 
 292 03e8 3B010000 		lea	%s63,0x13b
 292      00003F06 
 293 03f0 00000000 		shm.l	%s63,0x0(%s61)
 293      BD033F31 
 294 03f8 08000000 		shm.l	%sl,0x8(%s61)
 294      BD030831 
 295 0400 10000000 		shm.l	%sp,0x10(%s61)
 295      BD030B31 
 296 0408 00000000 		monc
 296      0000003F 
 297 0410 00000000 		or	%s0,0,%s62
 297      BE000045 
 298              	.L_3.EoP:
 299              	# End of prologue codes
 300              	# line 15
  15:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/void_fn_bool.c **** 	printf("%s:%u bool %d\n",__FILE__,__LINE__,(int)b);
 301              		.loc	1 15 0
 302 0418 00000000 		or	%s63,%s0,(0)1
 302      00803F45 
 303 0420 C8000000 		st	%s63,200(0,%sp)
 303      8B003F11 
 304 0428 00000000 		lea	%s62,.LP.__string.0@GOTOFF_LO
 304      00003E06 
 305 0430 00000000 		and	%s62,%s62,(32)0
 305      60BE3E44 
 306 0438 00000000 		lea.sl	%s62,.LP.__string.0@GOTOFF_HI(%s62,%got)
 306      8FBEBE06 
 307 0440 B0000000 		st	%s62,176(0,%sp)
 307      8B003E11 
 308 0448 00000000 		lea	%s61,.LP.__string.1@GOTOFF_LO
 308      00003D06 
 309 0450 00000000 		and	%s61,%s61,(32)0
 309      60BD3D44 
 310 0458 00000000 		lea.sl	%s61,.LP.__string.1@GOTOFF_HI(%s61,%got)
 310      8FBDBD06 
 311 0460 B8000000 		st	%s61,184(0,%sp)
 311      8B003D11 
 312 0468 00000000 		or	%s60,15,(0)1
 312      000F3C45 
 313 0470 C0000000 		st	%s60,192(0,%sp)
 313      8B003C11 
 314 0478 00000000 		or	%s0,0,%s62
 314      BE000045 
 315 0480 00000000 		or	%s1,0,%s61
 315      BD000145 
 316 0488 00000000 		or	%s2,0,%s60
 316      BC000245 
 317 0490 00000000 		or	%s3,0,%s63
 317      BF000345 
 318 0498 00000000 		lea	%s12,printf@PLT_LO(-24)
 318      00680C06 
 319 04a0 00000000 		and	%s12,%s12,(32)0
 319      608C0C44 
 320 04a8 00000000 		sic	%lr
 320      00000A28 
 321 04b0 00000000 		lea.sl	%s12,printf@PLT_HI(%s12,%lr)
 321      8A8C8C06 
 322 04b8 00000000 		bsic	%lr,(,%s12)		# printf
 322      8C000A08 
 323 04c0 08000000 		br.l	.L_3.1
 323      00000F18 
 324              	.L_3.1:
 325              	# line 16
  16:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/void_fn_bool.c **** }
 326              		.loc	1 16 0
 327              	# Start of epilogue codes
 328 04c8 00000000 		or	%sp,0,%fp
 328      89000B45 
 329              		.cfi_def_cfa	11,8
 330 04d0 18000000 		ld	%got,0x18(,%sp)
 330      8B000F01 
 331 04d8 20000000 		ld	%plt,0x20(,%sp)
 331      8B001001 
 332 04e0 08000000 		ld	%lr,0x8(,%sp)
 332      8B000A01 
 333 04e8 00000000 		ld	%fp,0x0(,%sp)
 333      8B000901 
 334 04f0 00000000 		b.l	(,%lr)
 334      8A000F19 
 335              	.L_3.EoE:
 336              		.cfi_endproc
 337              		.set	.L.3.2auto_size,	0xfffffffffffffe30	# 464 Bytes
 339              	# ============ End  prt_void_fn_bool ============
 340              	.Le1.0:
