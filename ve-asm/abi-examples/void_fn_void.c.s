   1              		.ident "ncc 1.1.4 (Build 10:17:22 Apr 19 2018)"
   2              		.file "void_fn_void.c"
   3              		.file 1 "/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/void_fn_void.c"
   4              		.file 2 "/opt/nec/ve/ncc/1.1.4/include/stdc-predef.h"
   5              		.file 3 "/opt/nec/ve/ncc/1.1.4/include/stdio.h"
   6              		.file 4 "/opt/nec/ve/ncc/1.1.4/include/yvals.h"
   7              		.file 5 "/opt/nec/ve/ncc/1.1.4/include/necvals.h"
   8              		.file 6 "/opt/nec/ve/ncc/1.1.4/include/stdarg.h"
   9              		.file 7 "/opt/nec/ve/musl/include/stdio.h"
  10              		.file 8 "/opt/nec/ve/musl/include/features.h"
  11              		.file 9 "/opt/nec/ve/musl/include/bits/alltypes.h"
  12              	# ============ Begin  void_fn_void ============
  13              		.text
  14              		.balign 16
  15              	.L_1.0:
  16              	# line 5
   1:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/void_fn_void.c **** #include <stdio.h>
   2:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/void_fn_void.c **** 
   3:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/void_fn_void.c **** extern void ext_void_fn_void();
   4:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/void_fn_void.c **** 
   5:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/void_fn_void.c **** void void_fn_void() {
  17              		.loc	1 5 0
  18              		.globl	void_fn_void
  20              	void_fn_void:
  21              		.cfi_startproc
  22 0000 00000000 		st	%fp,0x0(,%sp)
  22      8B000911 
  23              		.cfi_def_cfa_offset	0
  24              		.cfi_offset	9,0
  25 0008 08000000 		st	%lr,0x8(,%sp)
  25      8B000A11 
  26 0010 18000000 		st	%got,0x18(,%sp)
  26      8B000F11 
  27 0018 20000000 		st	%plt,0x20(,%sp)
  27      8B001011 
  28 0020 00000000 		lea	%got,_GLOBAL_OFFSET_TABLE_@PC_LO(-0x18)
  28      00680F06 
  29 0028 00000000 		and	%got,%got,(32)0
  29      608F0F44 
  30 0030 00000000 		sic	%plt
  30      00001028 
  31 0038 00000000 		lea.sl	%got,_GLOBAL_OFFSET_TABLE_@PC_HI(%plt,%got)
  31      8F908F06 
  32 0040 00000000 		or	%fp,0,%sp
  32      8B000945 
  33              		.cfi_def_cfa_register	9
  34 0048 30000000 		st	%s18,48(,%fp)
  34      89001211 
  35 0050 38000000 		st	%s19,56(,%fp)
  35      89001311 
  36 0058 40000000 		st	%s20,64(,%fp)
  36      89001411 
  37 0060 48000000 		st	%s21,72(,%fp)
  37      89001511 
  38 0068 50000000 		st	%s22,80(,%fp)
  38      89001611 
  39 0070 58000000 		st	%s23,88(,%fp)
  39      89001711 
  40 0078 60000000 		st	%s24,96(,%fp)
  40      89001811 
  41 0080 68000000 		st	%s25,104(,%fp)
  41      89001911 
  42 0088 70000000 		st	%s26,112(,%fp)
  42      89001A11 
  43 0090 78000000 		st	%s27,120(,%fp)
  43      89001B11 
  44 0098 80000000 		st	%s28,128(,%fp)
  44      89001C11 
  45 00a0 88000000 		st	%s29,136(,%fp)
  45      89001D11 
  46 00a8 90000000 		st	%s30,144(,%fp)
  46      89001E11 
  47 00b0 98000000 		st	%s31,152(,%fp)
  47      89001F11 
  48 00b8 A0000000 		st	%s32,160(,%fp)
  48      89002011 
  49 00c0 A8000000 		st	%s33,168(,%fp)
  49      89002111 
  50 00c8 50FEFFFF 		lea	%s13,.L.1.2auto_size&0xffffffff
  50      00000D06 
  51 00d0 00000000 		and	%s13,%s13,(32)0
  51      608D0D44 
  52 00d8 FFFFFFFF 		lea.sl	%sp,.L.1.2auto_size>>32(%fp,%s13)
  52      8D898B06 
  53 00e0 48000000 		brge.l.t	%sp,%sl,.L_1.EoP
  53      888B3518 
  54 00e8 18000000 		ld	%s61,0x18(,%tp)
  54      8E003D01 
  55 00f0 00000000 		or	%s62,0,%s0
  55      80003E45 
  56 00f8 3B010000 		lea	%s63,0x13b
  56      00003F06 
  57 0100 00000000 		shm.l	%s63,0x0(%s61)
  57      BD033F31 
  58 0108 08000000 		shm.l	%sl,0x8(%s61)
  58      BD030831 
  59 0110 10000000 		shm.l	%sp,0x10(%s61)
  59      BD030B31 
  60 0118 00000000 		monc
  60      0000003F 
  61 0120 00000000 		or	%s0,0,%s62
  61      BE000045 
  62              	.L_1.EoP:
  63              	# End of prologue codes
  64              	# line 6
   6:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/void_fn_void.c **** 	ext_void_fn_void();
  65              		.loc	1 6 0
  66 0128 00000000 		lea	%s12,ext_void_fn_void@PLT_LO(-24)
  66      00680C06 
  67 0130 00000000 		and	%s12,%s12,(32)0
  67      608C0C44 
  68 0138 00000000 		sic	%lr
  68      00000A28 
  69 0140 00000000 		lea.sl	%s12,ext_void_fn_void@PLT_HI(%s12,%lr)
  69      8A8C8C06 
  70 0148 00000000 		bsic	%lr,(,%s12)		# ext_void_fn_void
  70      8C000A08 
  71 0150 08000000 		br.l	.L_1.1
  71      00000F18 
  72              	.L_1.1:
  73              	# line 7
   7:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/void_fn_void.c **** }
  74              		.loc	1 7 0
  75              	# Start of epilogue codes
  76 0158 00000000 		or	%sp,0,%fp
  76      89000B45 
  77              		.cfi_def_cfa	11,8
  78 0160 18000000 		ld	%got,0x18(,%sp)
  78      8B000F01 
  79 0168 20000000 		ld	%plt,0x20(,%sp)
  79      8B001001 
  80 0170 08000000 		ld	%lr,0x8(,%sp)
  80      8B000A01 
  81 0178 00000000 		ld	%fp,0x0(,%sp)
  81      8B000901 
  82 0180 00000000 		b.l	(,%lr)
  82      8A000F19 
  83              	.L_1.EoE:
  84              		.cfi_endproc
  85              		.set	.L.1.2auto_size,	0xfffffffffffffe50	# 432 Bytes
  87              	# ============ End  void_fn_void ============
  88              	# ============ Begin  mt_void_fn_void ============
  89 0188 00000000 		.balign 16
  89      00000000 
  90              	.L_2.0:
  91              	# line 8
   8:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/void_fn_void.c **** void mt_void_fn_void() {
  92              		.loc	1 8 0
  93              		.globl	mt_void_fn_void
  95              	mt_void_fn_void:
  96              		.cfi_startproc
  97 0190 00000000 		st	%fp,0x0(,%sp)
  97      8B000911 
  98              		.cfi_def_cfa_offset	0
  99              		.cfi_offset	9,0
 100 0198 08000000 		st	%lr,0x8(,%sp)
 100      8B000A11 
 101 01a0 18000000 		st	%got,0x18(,%sp)
 101      8B000F11 
 102 01a8 20000000 		st	%plt,0x20(,%sp)
 102      8B001011 
 103 01b0 00000000 		lea	%got,_GLOBAL_OFFSET_TABLE_@PC_LO(-0x18)
 103      00680F06 
 104 01b8 00000000 		and	%got,%got,(32)0
 104      608F0F44 
 105 01c0 00000000 		sic	%plt
 105      00001028 
 106 01c8 00000000 		lea.sl	%got,_GLOBAL_OFFSET_TABLE_@PC_HI(%plt,%got)
 106      8F908F06 
 107 01d0 00000000 		or	%fp,0,%sp
 107      8B000945 
 108              		.cfi_def_cfa_register	9
 109 01d8 30000000 		st	%s18,48(,%fp)
 109      89001211 
 110 01e0 38000000 		st	%s19,56(,%fp)
 110      89001311 
 111 01e8 40000000 		st	%s20,64(,%fp)
 111      89001411 
 112 01f0 48000000 		st	%s21,72(,%fp)
 112      89001511 
 113 01f8 50000000 		st	%s22,80(,%fp)
 113      89001611 
 114 0200 58000000 		st	%s23,88(,%fp)
 114      89001711 
 115 0208 60000000 		st	%s24,96(,%fp)
 115      89001811 
 116 0210 68000000 		st	%s25,104(,%fp)
 116      89001911 
 117 0218 70000000 		st	%s26,112(,%fp)
 117      89001A11 
 118 0220 78000000 		st	%s27,120(,%fp)
 118      89001B11 
 119 0228 80000000 		st	%s28,128(,%fp)
 119      89001C11 
 120 0230 88000000 		st	%s29,136(,%fp)
 120      89001D11 
 121 0238 90000000 		st	%s30,144(,%fp)
 121      89001E11 
 122 0240 98000000 		st	%s31,152(,%fp)
 122      89001F11 
 123 0248 A0000000 		st	%s32,160(,%fp)
 123      89002011 
 124 0250 A8000000 		st	%s33,168(,%fp)
 124      89002111 
 125 0258 00000000 		lea	%s13,.L.2.2auto_size&0xffffffff
 125      00000D06 
 126 0260 00000000 		and	%s13,%s13,(32)0
 126      608D0D44 
 127 0268 00000000 		lea.sl	%sp,.L.2.2auto_size>>32(%fp,%s13)
 127      8D898B06 
 128 0270 48000000 		brge.l.t	%sp,%sl,.L_2.EoP
 128      888B3518 
 129 0278 18000000 		ld	%s61,0x18(,%tp)
 129      8E003D01 
 130 0280 00000000 		or	%s62,0,%s0
 130      80003E45 
 131 0288 3B010000 		lea	%s63,0x13b
 131      00003F06 
 132 0290 00000000 		shm.l	%s63,0x0(%s61)
 132      BD033F31 
 133 0298 08000000 		shm.l	%sl,0x8(%s61)
 133      BD030831 
 134 02a0 10000000 		shm.l	%sp,0x10(%s61)
 134      BD030B31 
 135 02a8 00000000 		monc
 135      0000003F 
 136 02b0 00000000 		or	%s0,0,%s62
 136      BE000045 
 137              	.L_2.EoP:
 138              	# End of prologue codes
 139              	# line 9
   9:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/void_fn_void.c **** }
 140              		.loc	1 9 0
 141              	# Start of epilogue codes
 142 02b8 00000000 		or	%sp,0,%fp
 142      89000B45 
 143              		.cfi_def_cfa	11,8
 144 02c0 18000000 		ld	%got,0x18(,%sp)
 144      8B000F01 
 145 02c8 20000000 		ld	%plt,0x20(,%sp)
 145      8B001001 
 146 02d0 08000000 		ld	%lr,0x8(,%sp)
 146      8B000A01 
 147 02d8 00000000 		ld	%fp,0x0(,%sp)
 147      8B000901 
 148 02e0 00000000 		b.l	(,%lr)
 148      8A000F19 
 149              	.L_2.EoE:
 150              		.cfi_endproc
 151              		.set	.L.2.2auto_size,	0x0	# 0 Bytes
 153              	# ============ End  mt_void_fn_void ============
 154              	# ============ Begin  prt_void_fn_void ============
 155              		.section .rodata
 156              		.balign 16
 158              	.LP.__string.0:
 159 0000 25       		.byte	37
 160 0001 73       		.byte	115
 161 0002 3A       		.byte	58
 162 0003 25       		.byte	37
 163 0004 75       		.byte	117
 164 0005 0A       		.byte	10
 165 0006 00       		.zero	1
 166 0007 00       		.balign 8
 168              	.LP.__string.1:
 169 0008 2F       		.byte	47
 170 0009 75       		.byte	117
 171 000a 73       		.byte	115
 172 000b 72       		.byte	114
 173 000c 2F       		.byte	47
 174 000d 75       		.byte	117
 175 000e 68       		.byte	104
 176 000f 6F       		.byte	111
 177 0010 6D       		.byte	109
 178 0011 65       		.byte	101
 179 0012 2F       		.byte	47
 180 0013 61       		.byte	97
 181 0014 75       		.byte	117
 182 0015 72       		.byte	114
 183 0016 6F       		.byte	111
 184 0017 72       		.byte	114
 185 0018 61       		.byte	97
 186 0019 2F       		.byte	47
 187 001a 34       		.byte	52
 188 001b 67       		.byte	103
 189 001c 69       		.byte	105
 190 001d 2F       		.byte	47
 191 001e 6E       		.byte	110
 192 001f 6C       		.byte	108
 193 0020 61       		.byte	97
 194 0021 62       		.byte	98
 195 0022 68       		.byte	104
 196 0023 70       		.byte	112
 197 0024 67       		.byte	103
 198 0025 2F       		.byte	47
 199 0026 6B       		.byte	107
 200 0027 72       		.byte	114
 201 0028 75       		.byte	117
 202 0029 75       		.byte	117
 203 002a 73       		.byte	115
 204 002b 2F       		.byte	47
 205 002c 76       		.byte	118
 206 002d 74       		.byte	116
 207 002e 2F       		.byte	47
 208 002f 73       		.byte	115
 209 0030 72       		.byte	114
 210 0031 63       		.byte	99
 211 0032 2F       		.byte	47
 212 0033 61       		.byte	97
 213 0034 73       		.byte	115
 214 0035 6D       		.byte	109
 215 0036 2D       		.byte	45
 216 0037 65       		.byte	101
 217 0038 78       		.byte	120
 218 0039 61       		.byte	97
 219 003a 6D       		.byte	109
 220 003b 70       		.byte	112
 221 003c 6C       		.byte	108
 222 003d 65       		.byte	101
 223 003e 73       		.byte	115
 224 003f 2F       		.byte	47
 225 0040 76       		.byte	118
 226 0041 6F       		.byte	111
 227 0042 69       		.byte	105
 228 0043 64       		.byte	100
 229 0044 5F       		.byte	95
 230 0045 66       		.byte	102
 231 0046 6E       		.byte	110
 232 0047 5F       		.byte	95
 233 0048 76       		.byte	118
 234 0049 6F       		.byte	111
 235 004a 69       		.byte	105
 236 004b 64       		.byte	100
 237 004c 2E       		.byte	46
 238 004d 63       		.byte	99
 239 004e 00       		.zero	1
 240              		.text
 241 02e8 00000000 		.balign 16
 241      00000000 
 242              	.L_3.0:
 243              	# line 10
  10:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/void_fn_void.c **** void prt_void_fn_void() {
 244              		.loc	1 10 0
 245              		.globl	prt_void_fn_void
 247              	prt_void_fn_void:
 248              		.cfi_startproc
 249 02f0 00000000 		st	%fp,0x0(,%sp)
 249      8B000911 
 250              		.cfi_def_cfa_offset	0
 251              		.cfi_offset	9,0
 252 02f8 08000000 		st	%lr,0x8(,%sp)
 252      8B000A11 
 253 0300 18000000 		st	%got,0x18(,%sp)
 253      8B000F11 
 254 0308 20000000 		st	%plt,0x20(,%sp)
 254      8B001011 
 255 0310 00000000 		lea	%got,_GLOBAL_OFFSET_TABLE_@PC_LO(-0x18)
 255      00680F06 
 256 0318 00000000 		and	%got,%got,(32)0
 256      608F0F44 
 257 0320 00000000 		sic	%plt
 257      00001028 
 258 0328 00000000 		lea.sl	%got,_GLOBAL_OFFSET_TABLE_@PC_HI(%plt,%got)
 258      8F908F06 
 259 0330 00000000 		or	%fp,0,%sp
 259      8B000945 
 260              		.cfi_def_cfa_register	9
 261 0338 30000000 		st	%s18,48(,%fp)
 261      89001211 
 262 0340 38000000 		st	%s19,56(,%fp)
 262      89001311 
 263 0348 40000000 		st	%s20,64(,%fp)
 263      89001411 
 264 0350 48000000 		st	%s21,72(,%fp)
 264      89001511 
 265 0358 50000000 		st	%s22,80(,%fp)
 265      89001611 
 266 0360 58000000 		st	%s23,88(,%fp)
 266      89001711 
 267 0368 60000000 		st	%s24,96(,%fp)
 267      89001811 
 268 0370 68000000 		st	%s25,104(,%fp)
 268      89001911 
 269 0378 70000000 		st	%s26,112(,%fp)
 269      89001A11 
 270 0380 78000000 		st	%s27,120(,%fp)
 270      89001B11 
 271 0388 80000000 		st	%s28,128(,%fp)
 271      89001C11 
 272 0390 88000000 		st	%s29,136(,%fp)
 272      89001D11 
 273 0398 90000000 		st	%s30,144(,%fp)
 273      89001E11 
 274 03a0 98000000 		st	%s31,152(,%fp)
 274      89001F11 
 275 03a8 A0000000 		st	%s32,160(,%fp)
 275      89002011 
 276 03b0 A8000000 		st	%s33,168(,%fp)
 276      89002111 
 277 03b8 30FEFFFF 		lea	%s13,.L.3.2auto_size&0xffffffff
 277      00000D06 
 278 03c0 00000000 		and	%s13,%s13,(32)0
 278      608D0D44 
 279 03c8 FFFFFFFF 		lea.sl	%sp,.L.3.2auto_size>>32(%fp,%s13)
 279      8D898B06 
 280 03d0 48000000 		brge.l.t	%sp,%sl,.L_3.EoP
 280      888B3518 
 281 03d8 18000000 		ld	%s61,0x18(,%tp)
 281      8E003D01 
 282 03e0 00000000 		or	%s62,0,%s0
 282      80003E45 
 283 03e8 3B010000 		lea	%s63,0x13b
 283      00003F06 
 284 03f0 00000000 		shm.l	%s63,0x0(%s61)
 284      BD033F31 
 285 03f8 08000000 		shm.l	%sl,0x8(%s61)
 285      BD030831 
 286 0400 10000000 		shm.l	%sp,0x10(%s61)
 286      BD030B31 
 287 0408 00000000 		monc
 287      0000003F 
 288 0410 00000000 		or	%s0,0,%s62
 288      BE000045 
 289              	.L_3.EoP:
 290              	# End of prologue codes
 291              	# line 11
  11:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/void_fn_void.c **** 	printf("%s:%u\n",__FILE__,__LINE__);
 292              		.loc	1 11 0
 293 0418 00000000 		lea	%s63,.LP.__string.0@GOTOFF_LO
 293      00003F06 
 294 0420 00000000 		and	%s63,%s63,(32)0
 294      60BF3F44 
 295 0428 00000000 		lea.sl	%s63,.LP.__string.0@GOTOFF_HI(%s63,%got)
 295      8FBFBF06 
 296 0430 B0000000 		st	%s63,176(0,%sp)
 296      8B003F11 
 297 0438 00000000 		lea	%s62,.LP.__string.1@GOTOFF_LO
 297      00003E06 
 298 0440 00000000 		and	%s62,%s62,(32)0
 298      60BE3E44 
 299 0448 00000000 		lea.sl	%s62,.LP.__string.1@GOTOFF_HI(%s62,%got)
 299      8FBEBE06 
 300 0450 B8000000 		st	%s62,184(0,%sp)
 300      8B003E11 
 301 0458 00000000 		or	%s61,11,(0)1
 301      000B3D45 
 302 0460 C0000000 		st	%s61,192(0,%sp)
 302      8B003D11 
 303 0468 00000000 		or	%s0,0,%s63
 303      BF000045 
 304 0470 00000000 		or	%s1,0,%s62
 304      BE000145 
 305 0478 00000000 		or	%s2,0,%s61
 305      BD000245 
 306 0480 00000000 		lea	%s12,printf@PLT_LO(-24)
 306      00680C06 
 307 0488 00000000 		and	%s12,%s12,(32)0
 307      608C0C44 
 308 0490 00000000 		sic	%lr
 308      00000A28 
 309 0498 00000000 		lea.sl	%s12,printf@PLT_HI(%s12,%lr)
 309      8A8C8C06 
 310 04a0 00000000 		bsic	%lr,(,%s12)		# printf
 310      8C000A08 
 311 04a8 08000000 		br.l	.L_3.1
 311      00000F18 
 312              	.L_3.1:
 313              	# line 12
  12:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/void_fn_void.c **** }
 314              		.loc	1 12 0
 315              	# Start of epilogue codes
 316 04b0 00000000 		or	%sp,0,%fp
 316      89000B45 
 317              		.cfi_def_cfa	11,8
 318 04b8 18000000 		ld	%got,0x18(,%sp)
 318      8B000F01 
 319 04c0 20000000 		ld	%plt,0x20(,%sp)
 319      8B001001 
 320 04c8 08000000 		ld	%lr,0x8(,%sp)
 320      8B000A01 
 321 04d0 00000000 		ld	%fp,0x0(,%sp)
 321      8B000901 
 322 04d8 00000000 		b.l	(,%lr)
 322      8A000F19 
 323              	.L_3.EoE:
 324              		.cfi_endproc
 325              		.set	.L.3.2auto_size,	0xfffffffffffffe30	# 464 Bytes
 327              	# ============ End  prt_void_fn_void ============
 328              	.Le1.0:
