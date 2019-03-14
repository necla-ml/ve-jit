   1              		.ident "ncc 1.1.4 (Build 10:17:22 Apr 19 2018)"
   2              		.file "void_fn_many.c"
   3              		.file 1 "/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/void_fn_many.c"
   4              		.file 2 "/opt/nec/ve/ncc/1.1.4/include/stdc-predef.h"
   5              		.file 3 "/opt/nec/ve/ncc/1.1.4/include/stdio.h"
   6              		.file 4 "/opt/nec/ve/ncc/1.1.4/include/yvals.h"
   7              		.file 5 "/opt/nec/ve/ncc/1.1.4/include/necvals.h"
   8              		.file 6 "/opt/nec/ve/ncc/1.1.4/include/stdarg.h"
   9              		.file 7 "/opt/nec/ve/musl/include/stdio.h"
  10              		.file 8 "/opt/nec/ve/musl/include/features.h"
  11              		.file 9 "/opt/nec/ve/musl/include/bits/alltypes.h"
  12              		.file 10 "/opt/nec/ve/ncc/1.1.4/include/stdint.h"
  13              		.file 11 "/opt/nec/ve/musl/include/stdint.h"
  14              		.file 12 "/opt/nec/ve/musl/include/bits/stdint.h"
  15              	# ============ Begin  void_fn_many ============
  16              		.text
  17              		.balign 16
  18              	.L_1.0:
  19              	# line 18
   1:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/void_fn_many.c **** #include <stdio.h>
   2:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/void_fn_many.c **** #include <stdint.h>
   3:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/void_fn_many.c **** 
   4:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/void_fn_many.c **** typedef uint_least64_t U;
   5:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/void_fn_many.c **** typedef void (*void_fn_many_t)(
   6:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/void_fn_many.c **** 		U i0, U i1, U i2, U i3, U i4, U i5, U i6, U i7,
   7:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/void_fn_many.c **** 		uint_least8_t u8, int_least8_t s8, uint_least16_t u16, int_least16_t s16,
   8:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/void_fn_many.c **** 		uint_least32_t u32, int_least32_t s32, uint_least64_t u64, int_least64_t s64);
   9:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/void_fn_many.c **** 
  10:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/void_fn_many.c **** extern void ext_void_fn_many(
  11:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/void_fn_many.c **** 		U i0, U i1, U i2, U i3, U i4, U i5, U i6, U i7,
  12:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/void_fn_many.c **** 		uint_least8_t u8, int_least8_t s8, uint_least16_t u16, int_least16_t s16,
  13:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/void_fn_many.c **** 		uint_least32_t u32, int_least32_t s32, uint_least64_t u64, int_least64_t s64);
  14:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/void_fn_many.c **** 
  15:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/void_fn_many.c **** void void_fn_many(
  16:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/void_fn_many.c **** 		U i0, U i1, U i2, U i3, U i4, U i5, U i6, U i7,
  17:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/void_fn_many.c **** 		uint_least8_t u8, int_least8_t s8, uint_least16_t u16, int_least16_t s16,
  18:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/void_fn_many.c **** 		uint_least32_t u32, int_least32_t s32, uint_least64_t u64, int_least64_t s64){
  20              		.loc	1 18 0
  21              		.globl	void_fn_many
  23              	void_fn_many:
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
  53 00c8 A0FDFFFF 		lea	%s13,.L.1.2auto_size&0xffffffff
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
  67              	# line 19
  19:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/void_fn_many.c **** 	ext_void_fn_many( i0, i2, i4, i6, i1, i3, i5, i7,
  68              		.loc	1 19 0
  69 0128 F0000000 		ld1b.zx	%s63,240(0,%fp)	# u8
  69      8900BF05 
  70 0130 F0000000 		st	%s63,240(0,%sp)
  70      8B003F11 
  71 0138 F8000000 		ld1b.sx	%s62,248(0,%fp)	# s8
  71      89003E05 
  72 0140 F8000000 		st	%s62,248(0,%sp)
  72      8B003E11 
  73 0148 00010000 		ld2b.zx	%s61,256(0,%fp)	# u16
  73      8900BD04 
  74 0150 00010000 		st	%s61,256(0,%sp)
  74      8B003D11 
  75 0158 08010000 		ld2b.sx	%s60,264(0,%fp)	# s16
  75      89003C04 
  76 0160 08010000 		st	%s60,264(0,%sp)
  76      8B003C11 
  77 0168 10010000 		ldl.zx	%s59,272(0,%fp)	# u32
  77      8900BB03 
  78 0170 10010000 		st	%s59,272(0,%sp)
  78      8B003B11 
  79 0178 18010000 		ldl.sx	%s58,280(0,%fp)	# s32
  79      89003A03 
  80 0180 18010000 		st	%s58,280(0,%sp)
  80      8B003A11 
  81 0188 20010000 		ld	%s57,288(0,%fp)	# u64
  81      89003901 
  82 0190 20010000 		st	%s57,288(0,%sp)
  82      8B003911 
  83 0198 28010000 		ld	%s56,296(0,%fp)	# s64
  83      89003801 
  84 01a0 28010000 		st	%s56,296(0,%sp)
  84      8B003811 
  85 01a8 F8FFFFFF 		st	%s1,-8(,%fp)	# spill
  85      89000111 
  86 01b0 00000000 		or	%s1,0,%s2
  86      82000145 
  87 01b8 00000000 		or	%s2,0,%s4
  87      84000245 
  88 01c0 F0FFFFFF 		st	%s3,-16(,%fp)	# spill
  88      89000311 
  89 01c8 00000000 		or	%s3,0,%s6
  89      86000345 
  90 01d0 F8FFFFFF 		ld	%s4,-8(,%fp)	# restore
  90      89000401 
  91 01d8 E8FFFFFF 		st	%s5,-24(,%fp)	# spill
  91      89000511 
  92 01e0 F0FFFFFF 		ld	%s5,-16(,%fp)	# restore
  92      89000501 
  93 01e8 E8FFFFFF 		ld	%s6,-24(,%fp)	# restore
  93      89000601 
  94 01f0 00000000 		lea	%s12,ext_void_fn_many@PLT_LO(-24)
  94      00680C06 
  95 01f8 00000000 		and	%s12,%s12,(32)0
  95      608C0C44 
  96 0200 00000000 		sic	%lr
  96      00000A28 
  97 0208 00000000 		lea.sl	%s12,ext_void_fn_many@PLT_HI(%s12,%lr)
  97      8A8C8C06 
  98 0210 00000000 		bsic	%lr,(,%s12)		# ext_void_fn_many
  98      8C000A08 
  99 0218 08000000 		br.l	.L_1.1
  99      00000F18 
 100              	.L_1.1:
 101              	# line 21
  20:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/void_fn_many.c **** 			u8, s8, u16, s16, u32, s32, u64, s64 );
  21:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/void_fn_many.c **** }
 102              		.loc	1 21 0
 103              	# Start of epilogue codes
 104 0220 00000000 		or	%sp,0,%fp
 104      89000B45 
 105              		.cfi_def_cfa	11,8
 106 0228 18000000 		ld	%got,0x18(,%sp)
 106      8B000F01 
 107 0230 20000000 		ld	%plt,0x20(,%sp)
 107      8B001001 
 108 0238 08000000 		ld	%lr,0x8(,%sp)
 108      8B000A01 
 109 0240 00000000 		ld	%fp,0x0(,%sp)
 109      8B000901 
 110 0248 00000000 		b.l	(,%lr)
 110      8A000F19 
 111              	.L_1.EoE:
 112              		.cfi_endproc
 113              		.set	.L.1.2auto_size,	0xfffffffffffffda0	# 608 Bytes
 115              	# ============ End  void_fn_many ============
 116              	# ============ Begin  mt_void_fn_many ============
 117              		.balign 16
 118              	.L_2.0:
 119              	# line 25
  22:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/void_fn_many.c **** void mt_void_fn_many(
  23:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/void_fn_many.c **** 		U i0, U i1, U i2, U i3, U i4, U i5, U i6, U i7,
  24:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/void_fn_many.c **** 		uint_least8_t u8, int_least8_t s8, uint_least16_t u16, int_least16_t s16,
  25:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/void_fn_many.c **** 		uint_least32_t u32, int_least32_t s32, uint_least64_t u64, int_least64_t s64){
 120              		.loc	1 25 0
 121              		.globl	mt_void_fn_many
 123              	mt_void_fn_many:
 124              		.cfi_startproc
 125 0250 00000000 		st	%fp,0x0(,%sp)
 125      8B000911 
 126              		.cfi_def_cfa_offset	0
 127              		.cfi_offset	9,0
 128 0258 08000000 		st	%lr,0x8(,%sp)
 128      8B000A11 
 129 0260 18000000 		st	%got,0x18(,%sp)
 129      8B000F11 
 130 0268 20000000 		st	%plt,0x20(,%sp)
 130      8B001011 
 131 0270 00000000 		lea	%got,_GLOBAL_OFFSET_TABLE_@PC_LO(-0x18)
 131      00680F06 
 132 0278 00000000 		and	%got,%got,(32)0
 132      608F0F44 
 133 0280 00000000 		sic	%plt
 133      00001028 
 134 0288 00000000 		lea.sl	%got,_GLOBAL_OFFSET_TABLE_@PC_HI(%plt,%got)
 134      8F908F06 
 135 0290 00000000 		or	%fp,0,%sp
 135      8B000945 
 136              		.cfi_def_cfa_register	9
 137 0298 30000000 		st	%s18,48(,%fp)
 137      89001211 
 138 02a0 38000000 		st	%s19,56(,%fp)
 138      89001311 
 139 02a8 40000000 		st	%s20,64(,%fp)
 139      89001411 
 140 02b0 48000000 		st	%s21,72(,%fp)
 140      89001511 
 141 02b8 50000000 		st	%s22,80(,%fp)
 141      89001611 
 142 02c0 58000000 		st	%s23,88(,%fp)
 142      89001711 
 143 02c8 60000000 		st	%s24,96(,%fp)
 143      89001811 
 144 02d0 68000000 		st	%s25,104(,%fp)
 144      89001911 
 145 02d8 70000000 		st	%s26,112(,%fp)
 145      89001A11 
 146 02e0 78000000 		st	%s27,120(,%fp)
 146      89001B11 
 147 02e8 80000000 		st	%s28,128(,%fp)
 147      89001C11 
 148 02f0 88000000 		st	%s29,136(,%fp)
 148      89001D11 
 149 02f8 90000000 		st	%s30,144(,%fp)
 149      89001E11 
 150 0300 98000000 		st	%s31,152(,%fp)
 150      89001F11 
 151 0308 A0000000 		st	%s32,160(,%fp)
 151      89002011 
 152 0310 A8000000 		st	%s33,168(,%fp)
 152      89002111 
 153 0318 00000000 		lea	%s13,.L.2.2auto_size&0xffffffff
 153      00000D06 
 154 0320 00000000 		and	%s13,%s13,(32)0
 154      608D0D44 
 155 0328 00000000 		lea.sl	%sp,.L.2.2auto_size>>32(%fp,%s13)
 155      8D898B06 
 156 0330 48000000 		brge.l.t	%sp,%sl,.L_2.EoP
 156      888B3518 
 157 0338 18000000 		ld	%s61,0x18(,%tp)
 157      8E003D01 
 158 0340 00000000 		or	%s62,0,%s0
 158      80003E45 
 159 0348 3B010000 		lea	%s63,0x13b
 159      00003F06 
 160 0350 00000000 		shm.l	%s63,0x0(%s61)
 160      BD033F31 
 161 0358 08000000 		shm.l	%sl,0x8(%s61)
 161      BD030831 
 162 0360 10000000 		shm.l	%sp,0x10(%s61)
 162      BD030B31 
 163 0368 00000000 		monc
 163      0000003F 
 164 0370 00000000 		or	%s0,0,%s62
 164      BE000045 
 165              	.L_2.EoP:
 166              	# End of prologue codes
 167              	# line 27
  26:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/void_fn_many.c **** 	/* mt */
  27:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/void_fn_many.c **** }
 168              		.loc	1 27 0
 169              	# Start of epilogue codes
 170 0378 00000000 		or	%sp,0,%fp
 170      89000B45 
 171              		.cfi_def_cfa	11,8
 172 0380 18000000 		ld	%got,0x18(,%sp)
 172      8B000F01 
 173 0388 20000000 		ld	%plt,0x20(,%sp)
 173      8B001001 
 174 0390 08000000 		ld	%lr,0x8(,%sp)
 174      8B000A01 
 175 0398 00000000 		ld	%fp,0x0(,%sp)
 175      8B000901 
 176 03a0 00000000 		b.l	(,%lr)
 176      8A000F19 
 177              	.L_2.EoE:
 178              		.cfi_endproc
 179              		.set	.L.2.2auto_size,	0x0	# 0 Bytes
 181              	# ============ End  mt_void_fn_many ============
 182              	# ============ Begin  prt_void_fn_many ============
 183              		.section .rodata
 184              		.balign 16
 186              	.LP.__string.0:
 187 0000 25       		.byte	37
 188 0001 73       		.byte	115
 189 0002 3A       		.byte	58
 190 0003 25       		.byte	37
 191 0004 75       		.byte	117
 192 0005 20       		.byte	32
 193 0006 73       		.byte	115
 194 0007 75       		.byte	117
 195 0008 6D       		.byte	109
 196 0009 20       		.byte	32
 197 000a 25       		.byte	37
 198 000b 6C       		.byte	108
 199 000c 6C       		.byte	108
 200 000d 64       		.byte	100
 201 000e 0A       		.byte	10
 202 000f 00       		.zero	1
 203              		.balign 8
 205              	.LP.__string.1:
 206 0010 2F       		.byte	47
 207 0011 75       		.byte	117
 208 0012 73       		.byte	115
 209 0013 72       		.byte	114
 210 0014 2F       		.byte	47
 211 0015 75       		.byte	117
 212 0016 68       		.byte	104
 213 0017 6F       		.byte	111
 214 0018 6D       		.byte	109
 215 0019 65       		.byte	101
 216 001a 2F       		.byte	47
 217 001b 61       		.byte	97
 218 001c 75       		.byte	117
 219 001d 72       		.byte	114
 220 001e 6F       		.byte	111
 221 001f 72       		.byte	114
 222 0020 61       		.byte	97
 223 0021 2F       		.byte	47
 224 0022 34       		.byte	52
 225 0023 67       		.byte	103
 226 0024 69       		.byte	105
 227 0025 2F       		.byte	47
 228 0026 6E       		.byte	110
 229 0027 6C       		.byte	108
 230 0028 61       		.byte	97
 231 0029 62       		.byte	98
 232 002a 68       		.byte	104
 233 002b 70       		.byte	112
 234 002c 67       		.byte	103
 235 002d 2F       		.byte	47
 236 002e 6B       		.byte	107
 237 002f 72       		.byte	114
 238 0030 75       		.byte	117
 239 0031 75       		.byte	117
 240 0032 73       		.byte	115
 241 0033 2F       		.byte	47
 242 0034 76       		.byte	118
 243 0035 74       		.byte	116
 244 0036 2F       		.byte	47
 245 0037 73       		.byte	115
 246 0038 72       		.byte	114
 247 0039 63       		.byte	99
 248 003a 2F       		.byte	47
 249 003b 61       		.byte	97
 250 003c 73       		.byte	115
 251 003d 6D       		.byte	109
 252 003e 2D       		.byte	45
 253 003f 65       		.byte	101
 254 0040 78       		.byte	120
 255 0041 61       		.byte	97
 256 0042 6D       		.byte	109
 257 0043 70       		.byte	112
 258 0044 6C       		.byte	108
 259 0045 65       		.byte	101
 260 0046 73       		.byte	115
 261 0047 2F       		.byte	47
 262 0048 76       		.byte	118
 263 0049 6F       		.byte	111
 264 004a 69       		.byte	105
 265 004b 64       		.byte	100
 266 004c 5F       		.byte	95
 267 004d 66       		.byte	102
 268 004e 6E       		.byte	110
 269 004f 5F       		.byte	95
 270 0050 6D       		.byte	109
 271 0051 61       		.byte	97
 272 0052 6E       		.byte	110
 273 0053 79       		.byte	121
 274 0054 2E       		.byte	46
 275 0055 63       		.byte	99
 276 0056 00       		.zero	1
 277              		.text
 278 03a8 00000000 		.balign 16
 278      00000000 
 279              	.L_3.0:
 280              	# line 31
  28:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/void_fn_many.c **** void prt_void_fn_many(
  29:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/void_fn_many.c **** 		U i0, U i1, U i2, U i3, U i4, U i5, U i6, U i7,
  30:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/void_fn_many.c **** 		uint_least8_t u8, int_least8_t s8, uint_least16_t u16, int_least16_t s16,
  31:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/void_fn_many.c **** 		uint_least32_t u32, int_least32_t s32, uint_least64_t u64, int_least64_t s64){
 281              		.loc	1 31 0
 282              		.globl	prt_void_fn_many
 284              	prt_void_fn_many:
 285              		.cfi_startproc
 286 03b0 00000000 		st	%fp,0x0(,%sp)
 286      8B000911 
 287              		.cfi_def_cfa_offset	0
 288              		.cfi_offset	9,0
 289 03b8 08000000 		st	%lr,0x8(,%sp)
 289      8B000A11 
 290 03c0 18000000 		st	%got,0x18(,%sp)
 290      8B000F11 
 291 03c8 20000000 		st	%plt,0x20(,%sp)
 291      8B001011 
 292 03d0 00000000 		lea	%got,_GLOBAL_OFFSET_TABLE_@PC_LO(-0x18)
 292      00680F06 
 293 03d8 00000000 		and	%got,%got,(32)0
 293      608F0F44 
 294 03e0 00000000 		sic	%plt
 294      00001028 
 295 03e8 00000000 		lea.sl	%got,_GLOBAL_OFFSET_TABLE_@PC_HI(%plt,%got)
 295      8F908F06 
 296 03f0 00000000 		or	%fp,0,%sp
 296      8B000945 
 297              		.cfi_def_cfa_register	9
 298 03f8 30000000 		st	%s18,48(,%fp)
 298      89001211 
 299 0400 38000000 		st	%s19,56(,%fp)
 299      89001311 
 300 0408 40000000 		st	%s20,64(,%fp)
 300      89001411 
 301 0410 48000000 		st	%s21,72(,%fp)
 301      89001511 
 302 0418 50000000 		st	%s22,80(,%fp)
 302      89001611 
 303 0420 58000000 		st	%s23,88(,%fp)
 303      89001711 
 304 0428 60000000 		st	%s24,96(,%fp)
 304      89001811 
 305 0430 68000000 		st	%s25,104(,%fp)
 305      89001911 
 306 0438 70000000 		st	%s26,112(,%fp)
 306      89001A11 
 307 0440 78000000 		st	%s27,120(,%fp)
 307      89001B11 
 308 0448 80000000 		st	%s28,128(,%fp)
 308      89001C11 
 309 0450 88000000 		st	%s29,136(,%fp)
 309      89001D11 
 310 0458 90000000 		st	%s30,144(,%fp)
 310      89001E11 
 311 0460 98000000 		st	%s31,152(,%fp)
 311      89001F11 
 312 0468 A0000000 		st	%s32,160(,%fp)
 312      89002011 
 313 0470 A8000000 		st	%s33,168(,%fp)
 313      89002111 
 314 0478 30FEFFFF 		lea	%s13,.L.3.2auto_size&0xffffffff
 314      00000D06 
 315 0480 00000000 		and	%s13,%s13,(32)0
 315      608D0D44 
 316 0488 FFFFFFFF 		lea.sl	%sp,.L.3.2auto_size>>32(%fp,%s13)
 316      8D898B06 
 317 0490 48000000 		brge.l.t	%sp,%sl,.L_3.EoP
 317      888B3518 
 318 0498 18000000 		ld	%s61,0x18(,%tp)
 318      8E003D01 
 319 04a0 00000000 		or	%s62,0,%s0
 319      80003E45 
 320 04a8 3B010000 		lea	%s63,0x13b
 320      00003F06 
 321 04b0 00000000 		shm.l	%s63,0x0(%s61)
 321      BD033F31 
 322 04b8 08000000 		shm.l	%sl,0x8(%s61)
 322      BD030831 
 323 04c0 10000000 		shm.l	%sp,0x10(%s61)
 323      BD030B31 
 324 04c8 00000000 		monc
 324      0000003F 
 325 04d0 00000000 		or	%s0,0,%s62
 325      BE000045 
 326              	.L_3.EoP:
 327              	# End of prologue codes
 328              	# line 32
  32:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/void_fn_many.c **** 	printf("%s:%u sum %lld\n",__FILE__,__LINE__,
 329              		.loc	1 32 0
 330 04d8 00000000 		addu.l	%s63,%s0,%s1
 330      81803F48 
 331 04e0 00000000 		addu.l	%s62,%s2,%s63
 331      BF823E48 
 332 04e8 00000000 		addu.l	%s61,%s3,%s62
 332      BE833D48 
 333 04f0 00000000 		addu.l	%s60,%s4,%s61
 333      BD843C48 
 334 04f8 00000000 		addu.l	%s59,%s5,%s60
 334      BC853B48 
 335 0500 00000000 		addu.l	%s58,%s6,%s59
 335      BB863A48 
 336 0508 00000000 		addu.l	%s57,%s7,%s58
 336      BA873948 
 337 0510 00000000 		lea	%s56,.LP.__string.0@GOTOFF_LO
 337      00003806 
 338 0518 00000000 		and	%s56,%s56,(32)0
 338      60B83844 
 339 0520 00000000 		lea.sl	%s56,.LP.__string.0@GOTOFF_HI(%s56,%got)
 339      8FB8B806 
 340 0528 B0000000 		st	%s56,176(0,%sp)
 340      8B003811 
 341 0530 00000000 		lea	%s55,.LP.__string.1@GOTOFF_LO
 341      00003706 
 342 0538 00000000 		and	%s55,%s55,(32)0
 342      60B73744 
 343 0540 00000000 		lea.sl	%s55,.LP.__string.1@GOTOFF_HI(%s55,%got)
 343      8FB7B706 
 344 0548 B8000000 		st	%s55,184(0,%sp)
 344      8B003711 
 345 0550 F0000000 		ld1b.zx	%s54,240(0,%fp)	# u8
 345      8900B605 
 346 0558 00000000 		or	%s53,%s54,(0)1
 346      00B63545 
 347 0560 00000000 		addu.l	%s52,%s57,%s53
 347      B5B93448 
 348 0568 F8000000 		ld1b.sx	%s51,248(0,%fp)	# s8
 348      89003305 
 349 0570 00000000 		or	%s50,%s51,(0)1
 349      00B33245 
 350 0578 00000000 		addu.l	%s49,%s52,%s50
 350      B2B43148 
 351 0580 00010000 		ld2b.zx	%s48,256(0,%fp)	# u16
 351      8900B004 
 352 0588 00000000 		or	%s47,%s48,(0)1
 352      00B02F45 
 353 0590 00000000 		addu.l	%s46,%s49,%s47
 353      AFB12E48 
 354 0598 08010000 		ld2b.sx	%s45,264(0,%fp)	# s16
 354      89002D04 
 355 05a0 00000000 		or	%s44,%s45,(0)1
 355      00AD2C45 
 356 05a8 00000000 		addu.l	%s43,%s46,%s44
 356      ACAE2B48 
 357 05b0 10010000 		ldl.zx	%s42,272(0,%fp)	# u32
 357      8900AA03 
 358 05b8 00000000 		or	%s41,%s42,(0)1
 358      00AA2945 
 359 05c0 00000000 		addu.l	%s40,%s43,%s41
 359      A9AB2848 
 360 05c8 18010000 		ldl.sx	%s39,280(0,%fp)	# s32
 360      89002703 
 361 05d0 00000000 		or	%s38,%s39,(0)1
 361      00A72645 
 362 05d8 00000000 		addu.l	%s37,%s40,%s38
 362      A6A82548 
 363 05e0 20010000 		ld	%s36,288(0,%fp)	# u64
 363      89002401 
 364 05e8 00000000 		addu.l	%s35,%s37,%s36
 364      A4A52348 
 365 05f0 28010000 		ld	%s34,296(0,%fp)	# s64
 365      89002201 
 366 05f8 00000000 		or	%s18,%s34,(0)1
 366      00A21245 
 367 0600 00000000 		addu.l	%s19,%s35,%s18
 367      92A31348 
 368 0608 00000000 		or	%s20,%s19,(0)1
 368      00931445 
 369 0610 C8000000 		st	%s20,200(0,%sp)
 369      8B001411 
 370 0618 00000000 		or	%s21,32,(0)1
 370      00201545 
 371 0620 C0000000 		st	%s21,192(0,%sp)
 371      8B001511 
 372 0628 00000000 		or	%s0,0,%s56
 372      B8000045 
 373 0630 00000000 		or	%s1,0,%s55
 373      B7000145 
 374 0638 00000000 		or	%s2,0,%s21
 374      95000245 
 375 0640 00000000 		or	%s3,0,%s20
 375      94000345 
 376 0648 00000000 		lea	%s12,printf@PLT_LO(-24)
 376      00680C06 
 377 0650 00000000 		and	%s12,%s12,(32)0
 377      608C0C44 
 378 0658 00000000 		sic	%lr
 378      00000A28 
 379 0660 00000000 		lea.sl	%s12,printf@PLT_HI(%s12,%lr)
 379      8A8C8C06 
 380 0668 00000000 		bsic	%lr,(,%s12)		# printf
 380      8C000A08 
 381 0670 08000000 		br.l	.L_3.1
 381      00000F18 
 382              	.L_3.1:
 383              	# line 35
  33:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/void_fn_many.c **** 			(long long int)(i0+i1+i2+i3+i4+i5+i6+i7
  34:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/void_fn_many.c **** 				+ u8+s8+u16+s16+u32+s32+u64+s64) );
  35:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/void_fn_many.c **** }
 384              		.loc	1 35 0
 385              	# Start of epilogue codes
 386 0678 30000000 		ld	%s18,48(,%fp)
 386      89001201 
 387 0680 38000000 		ld	%s19,56(,%fp)
 387      89001301 
 388 0688 40000000 		ld	%s20,64(,%fp)
 388      89001401 
 389 0690 48000000 		ld	%s21,72(,%fp)
 389      89001501 
 390 0698 00000000 		or	%sp,0,%fp
 390      89000B45 
 391              		.cfi_def_cfa	11,8
 392 06a0 18000000 		ld	%got,0x18(,%sp)
 392      8B000F01 
 393 06a8 20000000 		ld	%plt,0x20(,%sp)
 393      8B001001 
 394 06b0 08000000 		ld	%lr,0x8(,%sp)
 394      8B000A01 
 395 06b8 00000000 		ld	%fp,0x0(,%sp)
 395      8B000901 
 396 06c0 00000000 		b.l	(,%lr)
 396      8A000F19 
 397              	.L_3.EoE:
 398              		.cfi_endproc
 399              		.set	.L.3.2auto_size,	0xfffffffffffffe30	# 464 Bytes
 401              	# ============ End  prt_void_fn_many ============
 402              	# ============ Begin  wrap_void_fn_many ============
 403 06c8 00000000 		.balign 16
 403      00000000 
 404              	.L_4.0:
 405              	# line 39
  36:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/void_fn_many.c **** void wrap_void_fn_many(
  37:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/void_fn_many.c **** 		U i0, U i1, U i2, U i3, U i4, U i5, U i6, U i7,
  38:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/void_fn_many.c **** 		uint_least8_t u8, int_least8_t s8, uint_least16_t u16, int_least16_t s16,
  39:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/void_fn_many.c **** 		uint_least32_t u32, int_least32_t s32, uint_least64_t u64, int_least64_t s64){
 406              		.loc	1 39 0
 407              		.globl	wrap_void_fn_many
 409              	wrap_void_fn_many:
 410              		.cfi_startproc
 411 06d0 00000000 		st	%fp,0x0(,%sp)
 411      8B000911 
 412              		.cfi_def_cfa_offset	0
 413              		.cfi_offset	9,0
 414 06d8 08000000 		st	%lr,0x8(,%sp)
 414      8B000A11 
 415 06e0 18000000 		st	%got,0x18(,%sp)
 415      8B000F11 
 416 06e8 20000000 		st	%plt,0x20(,%sp)
 416      8B001011 
 417 06f0 00000000 		lea	%got,_GLOBAL_OFFSET_TABLE_@PC_LO(-0x18)
 417      00680F06 
 418 06f8 00000000 		and	%got,%got,(32)0
 418      608F0F44 
 419 0700 00000000 		sic	%plt
 419      00001028 
 420 0708 00000000 		lea.sl	%got,_GLOBAL_OFFSET_TABLE_@PC_HI(%plt,%got)
 420      8F908F06 
 421 0710 00000000 		or	%fp,0,%sp
 421      8B000945 
 422              		.cfi_def_cfa_register	9
 423 0718 30000000 		st	%s18,48(,%fp)
 423      89001211 
 424 0720 38000000 		st	%s19,56(,%fp)
 424      89001311 
 425 0728 40000000 		st	%s20,64(,%fp)
 425      89001411 
 426 0730 48000000 		st	%s21,72(,%fp)
 426      89001511 
 427 0738 50000000 		st	%s22,80(,%fp)
 427      89001611 
 428 0740 58000000 		st	%s23,88(,%fp)
 428      89001711 
 429 0748 60000000 		st	%s24,96(,%fp)
 429      89001811 
 430 0750 68000000 		st	%s25,104(,%fp)
 430      89001911 
 431 0758 70000000 		st	%s26,112(,%fp)
 431      89001A11 
 432 0760 78000000 		st	%s27,120(,%fp)
 432      89001B11 
 433 0768 80000000 		st	%s28,128(,%fp)
 433      89001C11 
 434 0770 88000000 		st	%s29,136(,%fp)
 434      89001D11 
 435 0778 90000000 		st	%s30,144(,%fp)
 435      89001E11 
 436 0780 98000000 		st	%s31,152(,%fp)
 436      89001F11 
 437 0788 A0000000 		st	%s32,160(,%fp)
 437      89002011 
 438 0790 A8000000 		st	%s33,168(,%fp)
 438      89002111 
 439 0798 A0FDFFFF 		lea	%s13,.L.4.2auto_size&0xffffffff
 439      00000D06 
 440 07a0 00000000 		and	%s13,%s13,(32)0
 440      608D0D44 
 441 07a8 FFFFFFFF 		lea.sl	%sp,.L.4.2auto_size>>32(%fp,%s13)
 441      8D898B06 
 442 07b0 48000000 		brge.l.t	%sp,%sl,.L_4.EoP
 442      888B3518 
 443 07b8 18000000 		ld	%s61,0x18(,%tp)
 443      8E003D01 
 444 07c0 00000000 		or	%s62,0,%s0
 444      80003E45 
 445 07c8 3B010000 		lea	%s63,0x13b
 445      00003F06 
 446 07d0 00000000 		shm.l	%s63,0x0(%s61)
 446      BD033F31 
 447 07d8 08000000 		shm.l	%sl,0x8(%s61)
 447      BD030831 
 448 07e0 10000000 		shm.l	%sp,0x10(%s61)
 448      BD030B31 
 449 07e8 00000000 		monc
 449      0000003F 
 450 07f0 00000000 		or	%s0,0,%s62
 450      BE000045 
 451              	.L_4.EoP:
 452              	# End of prologue codes
 453              	# line 40
  40:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/void_fn_many.c **** 	prt_void_fn_many( i0, i2, i4, i6, i1, i3, i5, i7,
 454              		.loc	1 40 0
 455 07f8 F0000000 		ld1b.zx	%s63,240(0,%fp)	# u8
 455      8900BF05 
 456 0800 F0000000 		st	%s63,240(0,%sp)
 456      8B003F11 
 457 0808 F8000000 		ld1b.sx	%s62,248(0,%fp)	# s8
 457      89003E05 
 458 0810 F8000000 		st	%s62,248(0,%sp)
 458      8B003E11 
 459 0818 00010000 		ld2b.zx	%s61,256(0,%fp)	# u16
 459      8900BD04 
 460 0820 00010000 		st	%s61,256(0,%sp)
 460      8B003D11 
 461 0828 08010000 		ld2b.sx	%s60,264(0,%fp)	# s16
 461      89003C04 
 462 0830 08010000 		st	%s60,264(0,%sp)
 462      8B003C11 
 463 0838 10010000 		ldl.zx	%s59,272(0,%fp)	# u32
 463      8900BB03 
 464 0840 10010000 		st	%s59,272(0,%sp)
 464      8B003B11 
 465 0848 18010000 		ldl.sx	%s58,280(0,%fp)	# s32
 465      89003A03 
 466 0850 18010000 		st	%s58,280(0,%sp)
 466      8B003A11 
 467 0858 20010000 		ld	%s57,288(0,%fp)	# u64
 467      89003901 
 468 0860 20010000 		st	%s57,288(0,%sp)
 468      8B003911 
 469 0868 28010000 		ld	%s56,296(0,%fp)	# s64
 469      89003801 
 470 0870 28010000 		st	%s56,296(0,%sp)
 470      8B003811 
 471 0878 F8FFFFFF 		st	%s1,-8(,%fp)	# spill
 471      89000111 
 472 0880 00000000 		or	%s1,0,%s2
 472      82000145 
 473 0888 00000000 		or	%s2,0,%s4
 473      84000245 
 474 0890 F0FFFFFF 		st	%s3,-16(,%fp)	# spill
 474      89000311 
 475 0898 00000000 		or	%s3,0,%s6
 475      86000345 
 476 08a0 F8FFFFFF 		ld	%s4,-8(,%fp)	# restore
 476      89000401 
 477 08a8 E8FFFFFF 		st	%s5,-24(,%fp)	# spill
 477      89000511 
 478 08b0 F0FFFFFF 		ld	%s5,-16(,%fp)	# restore
 478      89000501 
 479 08b8 E8FFFFFF 		ld	%s6,-24(,%fp)	# restore
 479      89000601 
 480 08c0 00000000 		lea	%s12,prt_void_fn_many@PLT_LO(-24)
 480      00680C06 
 481 08c8 00000000 		and	%s12,%s12,(32)0
 481      608C0C44 
 482 08d0 00000000 		sic	%lr
 482      00000A28 
 483 08d8 00000000 		lea.sl	%s12,prt_void_fn_many@PLT_HI(%s12,%lr)
 483      8A8C8C06 
 484 08e0 00000000 		bsic	%lr,(,%s12)		# prt_void_fn_many
 484      8C000A08 
 485 08e8 08000000 		br.l	.L_4.1
 485      00000F18 
 486              	.L_4.1:
 487              	# line 42
  41:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/void_fn_many.c **** 			u8, s8, u16, s16, u32, s32, u64, s64 );
  42:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/void_fn_many.c **** }
 488              		.loc	1 42 0
 489              	# Start of epilogue codes
 490 08f0 00000000 		or	%sp,0,%fp
 490      89000B45 
 491              		.cfi_def_cfa	11,8
 492 08f8 18000000 		ld	%got,0x18(,%sp)
 492      8B000F01 
 493 0900 20000000 		ld	%plt,0x20(,%sp)
 493      8B001001 
 494 0908 08000000 		ld	%lr,0x8(,%sp)
 494      8B000A01 
 495 0910 00000000 		ld	%fp,0x0(,%sp)
 495      8B000901 
 496 0918 00000000 		b.l	(,%lr)
 496      8A000F19 
 497              	.L_4.EoE:
 498              		.cfi_endproc
 499              		.set	.L.4.2auto_size,	0xfffffffffffffda0	# 608 Bytes
 501              	# ============ End  wrap_void_fn_many ============
 502              	# ============ Begin  getfoo ============
 503              		.balign 16
 504              	.L_5.0:
 505              	# line 43
  43:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/void_fn_many.c **** void_fn_many_t getfoo() {
 506              		.loc	1 43 0
 507              		.globl	getfoo
 509              	getfoo:
 510              		.cfi_startproc
 511 0920 00000000 		st	%fp,0x0(,%sp)
 511      8B000911 
 512              		.cfi_def_cfa_offset	0
 513              		.cfi_offset	9,0
 514 0928 08000000 		st	%lr,0x8(,%sp)
 514      8B000A11 
 515 0930 18000000 		st	%got,0x18(,%sp)
 515      8B000F11 
 516 0938 20000000 		st	%plt,0x20(,%sp)
 516      8B001011 
 517 0940 00000000 		lea	%got,_GLOBAL_OFFSET_TABLE_@PC_LO(-0x18)
 517      00680F06 
 518 0948 00000000 		and	%got,%got,(32)0
 518      608F0F44 
 519 0950 00000000 		sic	%plt
 519      00001028 
 520 0958 00000000 		lea.sl	%got,_GLOBAL_OFFSET_TABLE_@PC_HI(%plt,%got)
 520      8F908F06 
 521 0960 00000000 		or	%fp,0,%sp
 521      8B000945 
 522              		.cfi_def_cfa_register	9
 523 0968 30000000 		st	%s18,48(,%fp)
 523      89001211 
 524 0970 38000000 		st	%s19,56(,%fp)
 524      89001311 
 525 0978 40000000 		st	%s20,64(,%fp)
 525      89001411 
 526 0980 48000000 		st	%s21,72(,%fp)
 526      89001511 
 527 0988 50000000 		st	%s22,80(,%fp)
 527      89001611 
 528 0990 58000000 		st	%s23,88(,%fp)
 528      89001711 
 529 0998 60000000 		st	%s24,96(,%fp)
 529      89001811 
 530 09a0 68000000 		st	%s25,104(,%fp)
 530      89001911 
 531 09a8 70000000 		st	%s26,112(,%fp)
 531      89001A11 
 532 09b0 78000000 		st	%s27,120(,%fp)
 532      89001B11 
 533 09b8 80000000 		st	%s28,128(,%fp)
 533      89001C11 
 534 09c0 88000000 		st	%s29,136(,%fp)
 534      89001D11 
 535 09c8 90000000 		st	%s30,144(,%fp)
 535      89001E11 
 536 09d0 98000000 		st	%s31,152(,%fp)
 536      89001F11 
 537 09d8 A0000000 		st	%s32,160(,%fp)
 537      89002011 
 538 09e0 A8000000 		st	%s33,168(,%fp)
 538      89002111 
 539 09e8 00000000 		lea	%s13,.L.5.2auto_size&0xffffffff
 539      00000D06 
 540 09f0 00000000 		and	%s13,%s13,(32)0
 540      608D0D44 
 541 09f8 00000000 		lea.sl	%sp,.L.5.2auto_size>>32(%fp,%s13)
 541      8D898B06 
 542 0a00 48000000 		brge.l.t	%sp,%sl,.L_5.EoP
 542      888B3518 
 543 0a08 18000000 		ld	%s61,0x18(,%tp)
 543      8E003D01 
 544 0a10 00000000 		or	%s62,0,%s0
 544      80003E45 
 545 0a18 3B010000 		lea	%s63,0x13b
 545      00003F06 
 546 0a20 00000000 		shm.l	%s63,0x0(%s61)
 546      BD033F31 
 547 0a28 08000000 		shm.l	%sl,0x8(%s61)
 547      BD030831 
 548 0a30 10000000 		shm.l	%sp,0x10(%s61)
 548      BD030B31 
 549 0a38 00000000 		monc
 549      0000003F 
 550 0a40 00000000 		or	%s0,0,%s62
 550      BE000045 
 551              	.L_5.EoP:
 552              	# End of prologue codes
 553              	# line 44
  44:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/void_fn_many.c **** 	return prt_void_fn_many;
 554              		.loc	1 44 0
 555 0a48 00000000 		lea	%s63,prt_void_fn_many@GOT_LO
 555      00003F06 
 556 0a50 00000000 		and	%s63,%s63,(32)0
 556      60BF3F44 
 557 0a58 00000000 		lea.sl	%s63,prt_void_fn_many@GOT_HI(%s63,%got)
 557      8FBFBF06 
 558 0a60 00000000 		ld	%s63,(,%s63)
 558      BF003F01 
 559 0a68 00000000 		or	%s0,0,%s63
 559      BF000045 
 560              	# Start of epilogue codes
 561 0a70 00000000 		or	%sp,0,%fp
 561      89000B45 
 562              		.cfi_def_cfa	11,8
 563 0a78 18000000 		ld	%got,0x18(,%sp)
 563      8B000F01 
 564 0a80 20000000 		ld	%plt,0x20(,%sp)
 564      8B001001 
 565 0a88 08000000 		ld	%lr,0x8(,%sp)
 565      8B000A01 
 566 0a90 00000000 		ld	%fp,0x0(,%sp)
 566      8B000901 
 567 0a98 00000000 		b.l	(,%lr)
 567      8A000F19 
 568              	.L_5.EoE:
 569              		.cfi_endproc
 570              		.set	.L.5.2auto_size,	0x0	# 0 Bytes
 572              	# ============ End  getfoo ============
 573              	# ============ Begin  indirect_void_fn_many ============
 574              		.balign 16
 575              	.L_6.0:
 576              	# line 49
  45:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/void_fn_many.c **** }
  46:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/void_fn_many.c **** void indirect_void_fn_many(
  47:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/void_fn_many.c **** 		U i0, U i1, U i2, U i3, U i4, U i5, U i6, U i7,
  48:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/void_fn_many.c **** 		uint_least8_t u8, int_least8_t s8, uint_least16_t u16, int_least16_t s16,
  49:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/void_fn_many.c **** 		uint_least32_t u32, int_least32_t s32, uint_least64_t u64, int_least64_t s64){
 577              		.loc	1 49 0
 578              		.globl	indirect_void_fn_many
 580              	indirect_void_fn_many:
 581              		.cfi_startproc
 582 0aa0 00000000 		st	%fp,0x0(,%sp)
 582      8B000911 
 583              		.cfi_def_cfa_offset	0
 584              		.cfi_offset	9,0
 585 0aa8 08000000 		st	%lr,0x8(,%sp)
 585      8B000A11 
 586 0ab0 18000000 		st	%got,0x18(,%sp)
 586      8B000F11 
 587 0ab8 20000000 		st	%plt,0x20(,%sp)
 587      8B001011 
 588 0ac0 00000000 		lea	%got,_GLOBAL_OFFSET_TABLE_@PC_LO(-0x18)
 588      00680F06 
 589 0ac8 00000000 		and	%got,%got,(32)0
 589      608F0F44 
 590 0ad0 00000000 		sic	%plt
 590      00001028 
 591 0ad8 00000000 		lea.sl	%got,_GLOBAL_OFFSET_TABLE_@PC_HI(%plt,%got)
 591      8F908F06 
 592 0ae0 00000000 		or	%fp,0,%sp
 592      8B000945 
 593              		.cfi_def_cfa_register	9
 594 0ae8 30000000 		st	%s18,48(,%fp)
 594      89001211 
 595 0af0 38000000 		st	%s19,56(,%fp)
 595      89001311 
 596 0af8 40000000 		st	%s20,64(,%fp)
 596      89001411 
 597 0b00 48000000 		st	%s21,72(,%fp)
 597      89001511 
 598 0b08 50000000 		st	%s22,80(,%fp)
 598      89001611 
 599 0b10 58000000 		st	%s23,88(,%fp)
 599      89001711 
 600 0b18 60000000 		st	%s24,96(,%fp)
 600      89001811 
 601 0b20 68000000 		st	%s25,104(,%fp)
 601      89001911 
 602 0b28 70000000 		st	%s26,112(,%fp)
 602      89001A11 
 603 0b30 78000000 		st	%s27,120(,%fp)
 603      89001B11 
 604 0b38 80000000 		st	%s28,128(,%fp)
 604      89001C11 
 605 0b40 88000000 		st	%s29,136(,%fp)
 605      89001D11 
 606 0b48 90000000 		st	%s30,144(,%fp)
 606      89001E11 
 607 0b50 98000000 		st	%s31,152(,%fp)
 607      89001F11 
 608 0b58 A0000000 		st	%s32,160(,%fp)
 608      89002011 
 609 0b60 A8000000 		st	%s33,168(,%fp)
 609      89002111 
 610 0b68 40FDFFFF 		lea	%s13,.L.6.2auto_size&0xffffffff
 610      00000D06 
 611 0b70 00000000 		and	%s13,%s13,(32)0
 611      608D0D44 
 612 0b78 FFFFFFFF 		lea.sl	%sp,.L.6.2auto_size>>32(%fp,%s13)
 612      8D898B06 
 613 0b80 48000000 		brge.l.t	%sp,%sl,.L_6.EoP
 613      888B3518 
 614 0b88 18000000 		ld	%s61,0x18(,%tp)
 614      8E003D01 
 615 0b90 00000000 		or	%s62,0,%s0
 615      80003E45 
 616 0b98 3B010000 		lea	%s63,0x13b
 616      00003F06 
 617 0ba0 00000000 		shm.l	%s63,0x0(%s61)
 617      BD033F31 
 618 0ba8 08000000 		shm.l	%sl,0x8(%s61)
 618      BD030831 
 619 0bb0 10000000 		shm.l	%sp,0x10(%s61)
 619      BD030B31 
 620 0bb8 00000000 		monc
 620      0000003F 
 621 0bc0 00000000 		or	%s0,0,%s62
 621      BE000045 
 622              	.L_6.EoP:
 623              	# End of prologue codes
 624              	# line 50
  50:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/void_fn_many.c **** 	void_fn_many_t foo = getfoo();
 625              		.loc	1 50 0
 626 0bc8 F8FFFFFF 		st	%s0,-8(,%fp)	# spill
 626      89000011 
 627 0bd0 F0FFFFFF 		st	%s1,-16(,%fp)	# spill
 627      89000111 
 628 0bd8 E8FFFFFF 		st	%s2,-24(,%fp)	# spill
 628      89000211 
 629 0be0 E0FFFFFF 		st	%s3,-32(,%fp)	# spill
 629      89000311 
 630 0be8 D8FFFFFF 		st	%s4,-40(,%fp)	# spill
 630      89000411 
 631 0bf0 D0FFFFFF 		st	%s5,-48(,%fp)	# spill
 631      89000511 
 632 0bf8 C8FFFFFF 		st	%s6,-56(,%fp)	# spill
 632      89000611 
 633 0c00 C0FFFFFF 		st	%s7,-64(,%fp)	# spill
 633      89000711 
 634 0c08 00000000 		lea	%s12,getfoo@PLT_LO(-24)
 634      00680C06 
 635 0c10 00000000 		and	%s12,%s12,(32)0
 635      608C0C44 
 636 0c18 00000000 		sic	%lr
 636      00000A28 
 637 0c20 00000000 		lea.sl	%s12,getfoo@PLT_HI(%s12,%lr)
 637      8A8C8C06 
 638 0c28 00000000 		bsic	%lr,(,%s12)		# getfoo
 638      8C000A08 
 639              	.L_6.2:
 640              	# line 51
  51:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/void_fn_many.c **** 	(*foo)( i1, i0, i2, i3, i4, i5, i6, i7,
 641              		.loc	1 51 0
 642 0c30 F0000000 		ld1b.zx	%s63,240(0,%fp)	# u8
 642      8900BF05 
 643 0c38 F0000000 		st	%s63,240(0,%sp)
 643      8B003F11 
 644 0c40 F8000000 		ld1b.sx	%s62,248(0,%fp)	# s8
 644      89003E05 
 645 0c48 F8000000 		st	%s62,248(0,%sp)
 645      8B003E11 
 646 0c50 00010000 		ld2b.zx	%s61,256(0,%fp)	# u16
 646      8900BD04 
 647 0c58 00010000 		st	%s61,256(0,%sp)
 647      8B003D11 
 648 0c60 08010000 		ld2b.sx	%s60,264(0,%fp)	# s16
 648      89003C04 
 649 0c68 08010000 		st	%s60,264(0,%sp)
 649      8B003C11 
 650 0c70 10010000 		ldl.zx	%s59,272(0,%fp)	# u32
 650      8900BB03 
 651 0c78 10010000 		st	%s59,272(0,%sp)
 651      8B003B11 
 652 0c80 18010000 		ldl.sx	%s58,280(0,%fp)	# s32
 652      89003A03 
 653 0c88 18010000 		st	%s58,280(0,%sp)
 653      8B003A11 
 654 0c90 20010000 		ld	%s57,288(0,%fp)	# u64
 654      89003901 
 655 0c98 20010000 		st	%s57,288(0,%sp)
 655      8B003911 
 656 0ca0 28010000 		ld	%s56,296(0,%fp)	# s64
 656      89003801 
 657 0ca8 28010000 		st	%s56,296(0,%sp)
 657      8B003811 
 658 0cb0 B8FFFFFF 		st	%s0,-72(,%fp)	# spill
 658      89000011 
 659 0cb8 F0FFFFFF 		ld	%s0,-16(,%fp)	# restore
 659      89000001 
 660 0cc0 F8FFFFFF 		ld	%s1,-8(,%fp)	# restore
 660      89000101 
 661 0cc8 E8FFFFFF 		ld	%s2,-24(,%fp)	# restore
 661      89000201 
 662 0cd0 E0FFFFFF 		ld	%s3,-32(,%fp)	# restore
 662      89000301 
 663 0cd8 D8FFFFFF 		ld	%s4,-40(,%fp)	# restore
 663      89000401 
 664 0ce0 D0FFFFFF 		ld	%s5,-48(,%fp)	# restore
 664      89000501 
 665 0ce8 C8FFFFFF 		ld	%s6,-56(,%fp)	# restore
 665      89000601 
 666 0cf0 C0FFFFFF 		ld	%s7,-64(,%fp)	# restore
 666      89000701 
 667 0cf8 B8FFFFFF 		ld	%s12,-72(,%fp)	# restore
 667      89000C01 
 668 0d00 B8FFFFFF 		st	%s12,-72(,%fp)	# spill
 668      89000C11 
 669 0d08 00000000 		bsic	%lr,(,%s12)
 669      8C000A08 
 670 0d10 08000000 		br.l	.L_6.1
 670      00000F18 
 671              	.L_6.1:
 672              	# line 53
  52:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/void_fn_many.c **** 			u8, s8, u16, s16, u32, s32, u64, s64 );
  53:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/void_fn_many.c **** }
 673              		.loc	1 53 0
 674              	# Start of epilogue codes
 675 0d18 00000000 		or	%sp,0,%fp
 675      89000B45 
 676              		.cfi_def_cfa	11,8
 677 0d20 18000000 		ld	%got,0x18(,%sp)
 677      8B000F01 
 678 0d28 20000000 		ld	%plt,0x20(,%sp)
 678      8B001001 
 679 0d30 08000000 		ld	%lr,0x8(,%sp)
 679      8B000A01 
 680 0d38 00000000 		ld	%fp,0x0(,%sp)
 680      8B000901 
 681 0d40 00000000 		b.l	(,%lr)
 681      8A000F19 
 682              	.L_6.EoE:
 683              		.cfi_endproc
 684              		.set	.L.6.2auto_size,	0xfffffffffffffd40	# 704 Bytes
 686              	# ============ End  indirect_void_fn_many ============
 687              	.Le1.0:
