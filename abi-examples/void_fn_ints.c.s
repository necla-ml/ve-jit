   1              		.ident "ncc 1.1.4 (Build 10:17:22 Apr 19 2018)"
   2              		.file "void_fn_ints.c"
   3              		.file 1 "/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/void_fn_ints.c"
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
  15              	# ============ Begin  void_fn_ints ============
  16              		.text
  17              		.balign 16
  18              	.L_1.0:
  19              	# line 10
   1:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/void_fn_ints.c **** #include <stdio.h>
   2:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/void_fn_ints.c **** #include <stdint.h>
   3:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/void_fn_ints.c **** 
   4:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/void_fn_ints.c **** extern void ext_void_fn_ints(
   5:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/void_fn_ints.c **** 		uint_least8_t u8, int_least8_t s8, uint_least16_t u16, int_least16_t s16,
   6:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/void_fn_ints.c **** 		uint_least32_t u32, int_least32_t s32, uint_least64_t u64, int_least64_t s64);
   7:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/void_fn_ints.c **** 
   8:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/void_fn_ints.c **** void void_fn_ints(
   9:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/void_fn_ints.c **** 		uint_least8_t u8, int_least8_t s8, uint_least16_t u16, int_least16_t s16,
  10:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/void_fn_ints.c **** 		uint_least32_t u32, int_least32_t s32, uint_least64_t u64, int_least64_t s64){
  20              		.loc	1 10 0
  21              		.globl	void_fn_ints
  23              	void_fn_ints:
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
  53 00c8 10FEFFFF 		lea	%s13,.L.1.2auto_size&0xffffffff
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
  67              	# line 11
  11:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/void_fn_ints.c **** 	ext_void_fn_ints( u8, s8, u16, s16, u32, s32, u64, s64 );
  68              		.loc	1 11 0
  69 0128 00000000 		lea	%s12,ext_void_fn_ints@PLT_LO(-24)
  69      00680C06 
  70 0130 00000000 		and	%s12,%s12,(32)0
  70      608C0C44 
  71 0138 00000000 		sic	%lr
  71      00000A28 
  72 0140 00000000 		lea.sl	%s12,ext_void_fn_ints@PLT_HI(%s12,%lr)
  72      8A8C8C06 
  73 0148 00000000 		bsic	%lr,(,%s12)		# ext_void_fn_ints
  73      8C000A08 
  74 0150 08000000 		br.l	.L_1.1
  74      00000F18 
  75              	.L_1.1:
  76              	# line 12
  12:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/void_fn_ints.c **** }
  77              		.loc	1 12 0
  78              	# Start of epilogue codes
  79 0158 00000000 		or	%sp,0,%fp
  79      89000B45 
  80              		.cfi_def_cfa	11,8
  81 0160 18000000 		ld	%got,0x18(,%sp)
  81      8B000F01 
  82 0168 20000000 		ld	%plt,0x20(,%sp)
  82      8B001001 
  83 0170 08000000 		ld	%lr,0x8(,%sp)
  83      8B000A01 
  84 0178 00000000 		ld	%fp,0x0(,%sp)
  84      8B000901 
  85 0180 00000000 		b.l	(,%lr)
  85      8A000F19 
  86              	.L_1.EoE:
  87              		.cfi_endproc
  88              		.set	.L.1.2auto_size,	0xfffffffffffffe10	# 496 Bytes
  90              	# ============ End  void_fn_ints ============
  91              	# ============ Begin  void_fn_ints2 ============
  92 0188 00000000 		.balign 16
  92      00000000 
  93              	.L_2.0:
  94              	# line 15
  13:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/void_fn_ints.c **** void void_fn_ints2(
  14:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/void_fn_ints.c **** 		uint_least8_t u8, int_least8_t s8, uint_least16_t u16, int_least16_t s16,
  15:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/void_fn_ints.c **** 		uint_least32_t u32, int_least32_t s32, uint_least64_t u64, int_least64_t s64){
  95              		.loc	1 15 0
  96              		.globl	void_fn_ints2
  98              	void_fn_ints2:
  99              		.cfi_startproc
 100 0190 00000000 		st	%fp,0x0(,%sp)
 100      8B000911 
 101              		.cfi_def_cfa_offset	0
 102              		.cfi_offset	9,0
 103 0198 08000000 		st	%lr,0x8(,%sp)
 103      8B000A11 
 104 01a0 18000000 		st	%got,0x18(,%sp)
 104      8B000F11 
 105 01a8 20000000 		st	%plt,0x20(,%sp)
 105      8B001011 
 106 01b0 00000000 		lea	%got,_GLOBAL_OFFSET_TABLE_@PC_LO(-0x18)
 106      00680F06 
 107 01b8 00000000 		and	%got,%got,(32)0
 107      608F0F44 
 108 01c0 00000000 		sic	%plt
 108      00001028 
 109 01c8 00000000 		lea.sl	%got,_GLOBAL_OFFSET_TABLE_@PC_HI(%plt,%got)
 109      8F908F06 
 110 01d0 00000000 		or	%fp,0,%sp
 110      8B000945 
 111              		.cfi_def_cfa_register	9
 112 01d8 30000000 		st	%s18,48(,%fp)
 112      89001211 
 113 01e0 38000000 		st	%s19,56(,%fp)
 113      89001311 
 114 01e8 40000000 		st	%s20,64(,%fp)
 114      89001411 
 115 01f0 48000000 		st	%s21,72(,%fp)
 115      89001511 
 116 01f8 50000000 		st	%s22,80(,%fp)
 116      89001611 
 117 0200 58000000 		st	%s23,88(,%fp)
 117      89001711 
 118 0208 60000000 		st	%s24,96(,%fp)
 118      89001811 
 119 0210 68000000 		st	%s25,104(,%fp)
 119      89001911 
 120 0218 70000000 		st	%s26,112(,%fp)
 120      89001A11 
 121 0220 78000000 		st	%s27,120(,%fp)
 121      89001B11 
 122 0228 80000000 		st	%s28,128(,%fp)
 122      89001C11 
 123 0230 88000000 		st	%s29,136(,%fp)
 123      89001D11 
 124 0238 90000000 		st	%s30,144(,%fp)
 124      89001E11 
 125 0240 98000000 		st	%s31,152(,%fp)
 125      89001F11 
 126 0248 A0000000 		st	%s32,160(,%fp)
 126      89002011 
 127 0250 A8000000 		st	%s33,168(,%fp)
 127      89002111 
 128 0258 10FEFFFF 		lea	%s13,.L.2.2auto_size&0xffffffff
 128      00000D06 
 129 0260 00000000 		and	%s13,%s13,(32)0
 129      608D0D44 
 130 0268 FFFFFFFF 		lea.sl	%sp,.L.2.2auto_size>>32(%fp,%s13)
 130      8D898B06 
 131 0270 48000000 		brge.l.t	%sp,%sl,.L_2.EoP
 131      888B3518 
 132 0278 18000000 		ld	%s61,0x18(,%tp)
 132      8E003D01 
 133 0280 00000000 		or	%s62,0,%s0
 133      80003E45 
 134 0288 3B010000 		lea	%s63,0x13b
 134      00003F06 
 135 0290 00000000 		shm.l	%s63,0x0(%s61)
 135      BD033F31 
 136 0298 08000000 		shm.l	%sl,0x8(%s61)
 136      BD030831 
 137 02a0 10000000 		shm.l	%sp,0x10(%s61)
 137      BD030B31 
 138 02a8 00000000 		monc
 138      0000003F 
 139 02b0 00000000 		or	%s0,0,%s62
 139      BE000045 
 140              	.L_2.EoP:
 141              	# End of prologue codes
 142              	# line 16
  16:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/void_fn_ints.c **** 	u8+=1; s8-=1; u16+=1; s16-=1; u32+=1; s32-=1; u64*=2; s64/=2;
 143              		.loc	1 16 0
 144 02b8 00000000 		or	%s63,%s3,(0)1
 144      00833F45 
 145 02c0 00000000 		or	%s62,%s2,(0)1
 145      00823E45 
 146 02c8 00000000 		or	%s61,%s1,(0)1
 146      00813D45 
 147 02d0 00000000 		or	%s60,%s0,(0)1
 147      00803C45 
 148 02d8 00000000 		adds.w.sx	%s59,%s60,(63)0
 148      7FBC3B4A 
 149 02e0 00000000 		sll	%s58,%s59,56
 149      BB383A65 
 150 02e8 00000000 		srl	%s57,%s58,56
 150      BA383975 
 151 02f0 00000000 		subs.w.sx	%s56,%s61,(63)0
 151      7FBD385A 
 152 02f8 00000000 		sll	%s55,%s56,56
 152      B8383765 
 153 0300 00000000 		sra.l	%s54,%s55,56
 153      B7383677 
 154 0308 00000000 		adds.w.sx	%s53,%s62,(63)0
 154      7FBE354A 
 155 0310 00000000 		sll	%s52,%s53,48
 155      B5303465 
 156 0318 00000000 		srl	%s51,%s52,48
 156      B4303375 
 157 0320 00000000 		subs.w.sx	%s50,%s63,(63)0
 157      7FBF325A 
 158 0328 00000000 		sll	%s49,%s50,48
 158      B2303165 
 159 0330 00000000 		sra.l	%s48,%s49,48
 159      B1303077 
 160 0338 00000000 		subs.w.sx	%s47,%s5,(63)0
 160      7F852F5A 
 161 0340 00000000 		addu.w	%s46,%s4,(63)0
 161      7F84AE48 
 162 0348 02000000 		lea	%s45,2
 162      00002D06 
 163 0350 00000000 		mulu.l	%s44,%s6,%s45
 163      AD862C49 
 164 0358 02000000 		lea	%s43,2
 164      00002B06 
 165 0360 00000000 		divs.l	%s42,%s7,%s43
 165      AB872A7F 
 166              	# line 17
  17:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/void_fn_ints.c **** 	ext_void_fn_ints( u8, s8, u16, s16, u32, s32, u64, s64 );
 167              		.loc	1 17 0
 168 0368 00000000 		or	%s0,0,%s57
 168      B9000045 
 169 0370 00000000 		or	%s1,0,%s54
 169      B6000145 
 170 0378 00000000 		or	%s2,0,%s51
 170      B3000245 
 171 0380 00000000 		or	%s3,0,%s48
 171      B0000345 
 172 0388 00000000 		or	%s4,0,%s46
 172      AE000445 
 173 0390 00000000 		or	%s5,0,%s47
 173      AF000545 
 174 0398 00000000 		or	%s6,0,%s44
 174      AC000645 
 175 03a0 00000000 		or	%s7,0,%s42
 175      AA000745 
 176 03a8 00000000 		lea	%s12,ext_void_fn_ints@PLT_LO(-24)
 176      00680C06 
 177 03b0 00000000 		and	%s12,%s12,(32)0
 177      608C0C44 
 178 03b8 00000000 		sic	%lr
 178      00000A28 
 179 03c0 00000000 		lea.sl	%s12,ext_void_fn_ints@PLT_HI(%s12,%lr)
 179      8A8C8C06 
 180 03c8 00000000 		bsic	%lr,(,%s12)		# ext_void_fn_ints
 180      8C000A08 
 181 03d0 08000000 		br.l	.L_2.1
 181      00000F18 
 182              	.L_2.1:
 183              	# line 18
  18:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/void_fn_ints.c **** }
 184              		.loc	1 18 0
 185              	# Start of epilogue codes
 186 03d8 00000000 		or	%sp,0,%fp
 186      89000B45 
 187              		.cfi_def_cfa	11,8
 188 03e0 18000000 		ld	%got,0x18(,%sp)
 188      8B000F01 
 189 03e8 20000000 		ld	%plt,0x20(,%sp)
 189      8B001001 
 190 03f0 08000000 		ld	%lr,0x8(,%sp)
 190      8B000A01 
 191 03f8 00000000 		ld	%fp,0x0(,%sp)
 191      8B000901 
 192 0400 00000000 		b.l	(,%lr)
 192      8A000F19 
 193              	.L_2.EoE:
 194              		.cfi_endproc
 195              		.set	.L.2.2auto_size,	0xfffffffffffffe10	# 496 Bytes
 197              	# ============ End  void_fn_ints2 ============
 198              	# ============ Begin  mt_void_fn_ints ============
 199 0408 00000000 		.balign 16
 199      00000000 
 200              	.L_3.0:
 201              	# line 21
  19:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/void_fn_ints.c **** void mt_void_fn_ints(
  20:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/void_fn_ints.c **** 		uint_least8_t u8, int_least8_t s8, uint_least16_t u16, int_least16_t s16,
  21:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/void_fn_ints.c **** 		uint_least32_t u32, int_least32_t s32, uint_least64_t u64, int_least64_t s64){
 202              		.loc	1 21 0
 203              		.globl	mt_void_fn_ints
 205              	mt_void_fn_ints:
 206              		.cfi_startproc
 207 0410 00000000 		st	%fp,0x0(,%sp)
 207      8B000911 
 208              		.cfi_def_cfa_offset	0
 209              		.cfi_offset	9,0
 210 0418 08000000 		st	%lr,0x8(,%sp)
 210      8B000A11 
 211 0420 18000000 		st	%got,0x18(,%sp)
 211      8B000F11 
 212 0428 20000000 		st	%plt,0x20(,%sp)
 212      8B001011 
 213 0430 00000000 		lea	%got,_GLOBAL_OFFSET_TABLE_@PC_LO(-0x18)
 213      00680F06 
 214 0438 00000000 		and	%got,%got,(32)0
 214      608F0F44 
 215 0440 00000000 		sic	%plt
 215      00001028 
 216 0448 00000000 		lea.sl	%got,_GLOBAL_OFFSET_TABLE_@PC_HI(%plt,%got)
 216      8F908F06 
 217 0450 00000000 		or	%fp,0,%sp
 217      8B000945 
 218              		.cfi_def_cfa_register	9
 219 0458 30000000 		st	%s18,48(,%fp)
 219      89001211 
 220 0460 38000000 		st	%s19,56(,%fp)
 220      89001311 
 221 0468 40000000 		st	%s20,64(,%fp)
 221      89001411 
 222 0470 48000000 		st	%s21,72(,%fp)
 222      89001511 
 223 0478 50000000 		st	%s22,80(,%fp)
 223      89001611 
 224 0480 58000000 		st	%s23,88(,%fp)
 224      89001711 
 225 0488 60000000 		st	%s24,96(,%fp)
 225      89001811 
 226 0490 68000000 		st	%s25,104(,%fp)
 226      89001911 
 227 0498 70000000 		st	%s26,112(,%fp)
 227      89001A11 
 228 04a0 78000000 		st	%s27,120(,%fp)
 228      89001B11 
 229 04a8 80000000 		st	%s28,128(,%fp)
 229      89001C11 
 230 04b0 88000000 		st	%s29,136(,%fp)
 230      89001D11 
 231 04b8 90000000 		st	%s30,144(,%fp)
 231      89001E11 
 232 04c0 98000000 		st	%s31,152(,%fp)
 232      89001F11 
 233 04c8 A0000000 		st	%s32,160(,%fp)
 233      89002011 
 234 04d0 A8000000 		st	%s33,168(,%fp)
 234      89002111 
 235 04d8 00000000 		lea	%s13,.L.3.2auto_size&0xffffffff
 235      00000D06 
 236 04e0 00000000 		and	%s13,%s13,(32)0
 236      608D0D44 
 237 04e8 00000000 		lea.sl	%sp,.L.3.2auto_size>>32(%fp,%s13)
 237      8D898B06 
 238 04f0 48000000 		brge.l.t	%sp,%sl,.L_3.EoP
 238      888B3518 
 239 04f8 18000000 		ld	%s61,0x18(,%tp)
 239      8E003D01 
 240 0500 00000000 		or	%s62,0,%s0
 240      80003E45 
 241 0508 3B010000 		lea	%s63,0x13b
 241      00003F06 
 242 0510 00000000 		shm.l	%s63,0x0(%s61)
 242      BD033F31 
 243 0518 08000000 		shm.l	%sl,0x8(%s61)
 243      BD030831 
 244 0520 10000000 		shm.l	%sp,0x10(%s61)
 244      BD030B31 
 245 0528 00000000 		monc
 245      0000003F 
 246 0530 00000000 		or	%s0,0,%s62
 246      BE000045 
 247              	.L_3.EoP:
 248              	# End of prologue codes
 249              	# line 23
  22:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/void_fn_ints.c **** 	/* mt */
  23:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/void_fn_ints.c **** }
 250              		.loc	1 23 0
 251              	# Start of epilogue codes
 252 0538 00000000 		or	%sp,0,%fp
 252      89000B45 
 253              		.cfi_def_cfa	11,8
 254 0540 18000000 		ld	%got,0x18(,%sp)
 254      8B000F01 
 255 0548 20000000 		ld	%plt,0x20(,%sp)
 255      8B001001 
 256 0550 08000000 		ld	%lr,0x8(,%sp)
 256      8B000A01 
 257 0558 00000000 		ld	%fp,0x0(,%sp)
 257      8B000901 
 258 0560 00000000 		b.l	(,%lr)
 258      8A000F19 
 259              	.L_3.EoE:
 260              		.cfi_endproc
 261              		.set	.L.3.2auto_size,	0x0	# 0 Bytes
 263              	# ============ End  mt_void_fn_ints ============
 264              	# ============ Begin  prt_void_fn_ints ============
 265              		.section .rodata
 266              		.balign 16
 268              	.LP.__string.0:
 269 0000 25       		.byte	37
 270 0001 73       		.byte	115
 271 0002 3A       		.byte	58
 272 0003 25       		.byte	37
 273 0004 75       		.byte	117
 274 0005 20       		.byte	32
 275 0006 73       		.byte	115
 276 0007 75       		.byte	117
 277 0008 6D       		.byte	109
 278 0009 20       		.byte	32
 279 000a 25       		.byte	37
 280 000b 6C       		.byte	108
 281 000c 6C       		.byte	108
 282 000d 64       		.byte	100
 283 000e 0A       		.byte	10
 284 000f 00       		.zero	1
 285              		.balign 8
 287              	.LP.__string.1:
 288 0010 2F       		.byte	47
 289 0011 75       		.byte	117
 290 0012 73       		.byte	115
 291 0013 72       		.byte	114
 292 0014 2F       		.byte	47
 293 0015 75       		.byte	117
 294 0016 68       		.byte	104
 295 0017 6F       		.byte	111
 296 0018 6D       		.byte	109
 297 0019 65       		.byte	101
 298 001a 2F       		.byte	47
 299 001b 61       		.byte	97
 300 001c 75       		.byte	117
 301 001d 72       		.byte	114
 302 001e 6F       		.byte	111
 303 001f 72       		.byte	114
 304 0020 61       		.byte	97
 305 0021 2F       		.byte	47
 306 0022 34       		.byte	52
 307 0023 67       		.byte	103
 308 0024 69       		.byte	105
 309 0025 2F       		.byte	47
 310 0026 6E       		.byte	110
 311 0027 6C       		.byte	108
 312 0028 61       		.byte	97
 313 0029 62       		.byte	98
 314 002a 68       		.byte	104
 315 002b 70       		.byte	112
 316 002c 67       		.byte	103
 317 002d 2F       		.byte	47
 318 002e 6B       		.byte	107
 319 002f 72       		.byte	114
 320 0030 75       		.byte	117
 321 0031 75       		.byte	117
 322 0032 73       		.byte	115
 323 0033 2F       		.byte	47
 324 0034 76       		.byte	118
 325 0035 74       		.byte	116
 326 0036 2F       		.byte	47
 327 0037 73       		.byte	115
 328 0038 72       		.byte	114
 329 0039 63       		.byte	99
 330 003a 2F       		.byte	47
 331 003b 61       		.byte	97
 332 003c 73       		.byte	115
 333 003d 6D       		.byte	109
 334 003e 2D       		.byte	45
 335 003f 65       		.byte	101
 336 0040 78       		.byte	120
 337 0041 61       		.byte	97
 338 0042 6D       		.byte	109
 339 0043 70       		.byte	112
 340 0044 6C       		.byte	108
 341 0045 65       		.byte	101
 342 0046 73       		.byte	115
 343 0047 2F       		.byte	47
 344 0048 76       		.byte	118
 345 0049 6F       		.byte	111
 346 004a 69       		.byte	105
 347 004b 64       		.byte	100
 348 004c 5F       		.byte	95
 349 004d 66       		.byte	102
 350 004e 6E       		.byte	110
 351 004f 5F       		.byte	95
 352 0050 69       		.byte	105
 353 0051 6E       		.byte	110
 354 0052 74       		.byte	116
 355 0053 73       		.byte	115
 356 0054 2E       		.byte	46
 357 0055 63       		.byte	99
 358 0056 00       		.zero	1
 359              		.text
 360 0568 00000000 		.balign 16
 360      00000000 
 361              	.L_4.0:
 362              	# line 26
  24:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/void_fn_ints.c **** void prt_void_fn_ints(
  25:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/void_fn_ints.c **** 		uint_least8_t u8, int_least8_t s8, uint_least16_t u16, int_least16_t s16,
  26:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/void_fn_ints.c **** 		uint_least32_t u32, int_least32_t s32, uint_least64_t u64, int_least64_t s64){
 363              		.loc	1 26 0
 364              		.globl	prt_void_fn_ints
 366              	prt_void_fn_ints:
 367              		.cfi_startproc
 368 0570 00000000 		st	%fp,0x0(,%sp)
 368      8B000911 
 369              		.cfi_def_cfa_offset	0
 370              		.cfi_offset	9,0
 371 0578 08000000 		st	%lr,0x8(,%sp)
 371      8B000A11 
 372 0580 18000000 		st	%got,0x18(,%sp)
 372      8B000F11 
 373 0588 20000000 		st	%plt,0x20(,%sp)
 373      8B001011 
 374 0590 00000000 		lea	%got,_GLOBAL_OFFSET_TABLE_@PC_LO(-0x18)
 374      00680F06 
 375 0598 00000000 		and	%got,%got,(32)0
 375      608F0F44 
 376 05a0 00000000 		sic	%plt
 376      00001028 
 377 05a8 00000000 		lea.sl	%got,_GLOBAL_OFFSET_TABLE_@PC_HI(%plt,%got)
 377      8F908F06 
 378 05b0 00000000 		or	%fp,0,%sp
 378      8B000945 
 379              		.cfi_def_cfa_register	9
 380 05b8 30000000 		st	%s18,48(,%fp)
 380      89001211 
 381 05c0 38000000 		st	%s19,56(,%fp)
 381      89001311 
 382 05c8 40000000 		st	%s20,64(,%fp)
 382      89001411 
 383 05d0 48000000 		st	%s21,72(,%fp)
 383      89001511 
 384 05d8 50000000 		st	%s22,80(,%fp)
 384      89001611 
 385 05e0 58000000 		st	%s23,88(,%fp)
 385      89001711 
 386 05e8 60000000 		st	%s24,96(,%fp)
 386      89001811 
 387 05f0 68000000 		st	%s25,104(,%fp)
 387      89001911 
 388 05f8 70000000 		st	%s26,112(,%fp)
 388      89001A11 
 389 0600 78000000 		st	%s27,120(,%fp)
 389      89001B11 
 390 0608 80000000 		st	%s28,128(,%fp)
 390      89001C11 
 391 0610 88000000 		st	%s29,136(,%fp)
 391      89001D11 
 392 0618 90000000 		st	%s30,144(,%fp)
 392      89001E11 
 393 0620 98000000 		st	%s31,152(,%fp)
 393      89001F11 
 394 0628 A0000000 		st	%s32,160(,%fp)
 394      89002011 
 395 0630 A8000000 		st	%s33,168(,%fp)
 395      89002111 
 396 0638 30FEFFFF 		lea	%s13,.L.4.2auto_size&0xffffffff
 396      00000D06 
 397 0640 00000000 		and	%s13,%s13,(32)0
 397      608D0D44 
 398 0648 FFFFFFFF 		lea.sl	%sp,.L.4.2auto_size>>32(%fp,%s13)
 398      8D898B06 
 399 0650 48000000 		brge.l.t	%sp,%sl,.L_4.EoP
 399      888B3518 
 400 0658 18000000 		ld	%s61,0x18(,%tp)
 400      8E003D01 
 401 0660 00000000 		or	%s62,0,%s0
 401      80003E45 
 402 0668 3B010000 		lea	%s63,0x13b
 402      00003F06 
 403 0670 00000000 		shm.l	%s63,0x0(%s61)
 403      BD033F31 
 404 0678 08000000 		shm.l	%sl,0x8(%s61)
 404      BD030831 
 405 0680 10000000 		shm.l	%sp,0x10(%s61)
 405      BD030B31 
 406 0688 00000000 		monc
 406      0000003F 
 407 0690 00000000 		or	%s0,0,%s62
 407      BE000045 
 408              	.L_4.EoP:
 409              	# End of prologue codes
 410              	# line 27
  27:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/void_fn_ints.c **** 	printf("%s:%u sum %lld\n",__FILE__,__LINE__,
 411              		.loc	1 27 0
 412 0698 00000000 		or	%s63,%s7,(0)1
 412      00873F45 
 413 06a0 00000000 		sll	%s62,%s5,32
 413      85203E65 
 414 06a8 00000000 		srl	%s61,%s62,32
 414      BE203D75 
 415 06b0 00000000 		or	%s60,%s3,(0)1
 415      00833C45 
 416 06b8 00000000 		or	%s59,%s2,(0)1
 416      00823B45 
 417 06c0 00000000 		or	%s58,%s1,(0)1
 417      00813A45 
 418 06c8 00000000 		or	%s57,%s0,(0)1
 418      00803945 
 419 06d0 00000000 		adds.w.sx	%s56,%s58,%s57
 419      B9BA384A 
 420 06d8 00000000 		adds.w.sx	%s55,%s59,%s56
 420      B8BB374A 
 421 06e0 00000000 		adds.w.sx	%s54,%s60,%s55
 421      B7BC364A 
 422 06e8 00000000 		sll	%s53,%s54,32
 422      B6203565 
 423 06f0 00000000 		srl	%s52,%s53,32
 423      B5203475 
 424 06f8 00000000 		addu.w	%s51,%s4,%s52
 424      B484B348 
 425 0700 00000000 		addu.w	%s50,%s61,%s51
 425      B3BDB248 
 426 0708 00000000 		or	%s49,%s50,(0)1
 426      00B23145 
 427 0710 00000000 		addu.l	%s48,%s6,%s49
 427      B1863048 
 428 0718 00000000 		addu.l	%s47,%s63,%s48
 428      B0BF2F48 
 429 0720 00000000 		or	%s46,%s47,(0)1
 429      00AF2E45 
 430 0728 C8000000 		st	%s46,200(0,%sp)
 430      8B002E11 
 431 0730 00000000 		lea	%s45,.LP.__string.0@GOTOFF_LO
 431      00002D06 
 432 0738 00000000 		and	%s45,%s45,(32)0
 432      60AD2D44 
 433 0740 00000000 		lea.sl	%s45,.LP.__string.0@GOTOFF_HI(%s45,%got)
 433      8FADAD06 
 434 0748 B0000000 		st	%s45,176(0,%sp)
 434      8B002D11 
 435 0750 00000000 		lea	%s44,.LP.__string.1@GOTOFF_LO
 435      00002C06 
 436 0758 00000000 		and	%s44,%s44,(32)0
 436      60AC2C44 
 437 0760 00000000 		lea.sl	%s44,.LP.__string.1@GOTOFF_HI(%s44,%got)
 437      8FACAC06 
 438 0768 B8000000 		st	%s44,184(0,%sp)
 438      8B002C11 
 439 0770 00000000 		or	%s43,27,(0)1
 439      001B2B45 
 440 0778 C0000000 		st	%s43,192(0,%sp)
 440      8B002B11 
 441 0780 00000000 		or	%s0,0,%s45
 441      AD000045 
 442 0788 00000000 		or	%s1,0,%s44
 442      AC000145 
 443 0790 00000000 		or	%s2,0,%s43
 443      AB000245 
 444 0798 00000000 		or	%s3,0,%s46
 444      AE000345 
 445 07a0 00000000 		lea	%s12,printf@PLT_LO(-24)
 445      00680C06 
 446 07a8 00000000 		and	%s12,%s12,(32)0
 446      608C0C44 
 447 07b0 00000000 		sic	%lr
 447      00000A28 
 448 07b8 00000000 		lea.sl	%s12,printf@PLT_HI(%s12,%lr)
 448      8A8C8C06 
 449 07c0 00000000 		bsic	%lr,(,%s12)		# printf
 449      8C000A08 
 450 07c8 08000000 		br.l	.L_4.1
 450      00000F18 
 451              	.L_4.1:
 452              	# line 29
  28:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/void_fn_ints.c **** 			(long long int)(u8+s8+u16+s16+u32+s32+u64+s64));
  29:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/void_fn_ints.c **** }
 453              		.loc	1 29 0
 454              	# Start of epilogue codes
 455 07d0 00000000 		or	%sp,0,%fp
 455      89000B45 
 456              		.cfi_def_cfa	11,8
 457 07d8 18000000 		ld	%got,0x18(,%sp)
 457      8B000F01 
 458 07e0 20000000 		ld	%plt,0x20(,%sp)
 458      8B001001 
 459 07e8 08000000 		ld	%lr,0x8(,%sp)
 459      8B000A01 
 460 07f0 00000000 		ld	%fp,0x0(,%sp)
 460      8B000901 
 461 07f8 00000000 		b.l	(,%lr)
 461      8A000F19 
 462              	.L_4.EoE:
 463              		.cfi_endproc
 464              		.set	.L.4.2auto_size,	0xfffffffffffffe30	# 464 Bytes
 466              	# ============ End  prt_void_fn_ints ============
 467              	.Le1.0:
