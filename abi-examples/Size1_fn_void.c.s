   1              		.ident "ncc 1.1.4 (Build 10:17:22 Apr 19 2018)"
   2              		.file "Size1_fn_void.c"
   3              		.file 1 "/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/Size1_fn_void.c"
   4              		.file 2 "/opt/nec/ve/ncc/1.1.4/include/stdc-predef.h"
   5              		.file 3 "/opt/nec/ve/ncc/1.1.4/include/stdio.h"
   6              		.file 4 "/opt/nec/ve/ncc/1.1.4/include/yvals.h"
   7              		.file 5 "/opt/nec/ve/ncc/1.1.4/include/necvals.h"
   8              		.file 6 "/opt/nec/ve/ncc/1.1.4/include/stdarg.h"
   9              		.file 7 "/opt/nec/ve/musl/include/stdio.h"
  10              		.file 8 "/opt/nec/ve/musl/include/features.h"
  11              		.file 9 "/opt/nec/ve/musl/include/bits/alltypes.h"
  12              	# ============ Begin  Size1_fn_void ============
  13              		.text
  14              		.balign 16
  15              	.L_1.0:
  16              	# line 8
   1:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/Size1_fn_void.c **** #include <stdio.h>
   2:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/Size1_fn_void.c **** 
   3:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/Size1_fn_void.c **** typedef struct { char c0; } Size1;
   4:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/Size1_fn_void.c **** 
   5:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/Size1_fn_void.c **** extern Size1 ext_Size1_fn_void();
   6:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/Size1_fn_void.c **** 
   7:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/Size1_fn_void.c **** static Size1 size1_ret;
   8:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/Size1_fn_void.c **** Size1 Size1_fn_void() {
  17              		.loc	1 8 0
  18              		.globl	Size1_fn_void
  20              	Size1_fn_void:
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
  50 00c8 10FEFFFF 		lea	%s13,.L.1.2auto_size&0xffffffff
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
  64              	# line 9
   9:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/Size1_fn_void.c **** 	Size1 s = ext_Size1_fn_void();
  65              		.loc	1 9 0
  66 0128 00000000 		adds.l	%s63,%fp,(60)1
  66      3C893F59 
  67 0130 B0000000 		st	%s63,176(0,%sp)
  67      8B003F11 
  68 0138 E8FFFFFF 		st	%s0,-24(,%fp)	# spill
  68      89000011 
  69 0140 00000000 		or	%s0,0,%s63
  69      BF000045 
  70 0148 00000000 		lea	%s12,ext_Size1_fn_void@PLT_LO(-24)
  70      00680C06 
  71 0150 00000000 		and	%s12,%s12,(32)0
  71      608C0C44 
  72 0158 00000000 		sic	%lr
  72      00000A28 
  73 0160 00000000 		lea.sl	%s12,ext_Size1_fn_void@PLT_HI(%s12,%lr)
  73      8A8C8C06 
  74 0168 00000000 		bsic	%lr,(,%s12)		# ext_Size1_fn_void
  74      8C000A08 
  75              	.L_1.2:
  76 0170 F0FFFFFF 		ld1b.sx	%s63,-16(0,%fp)	# Size1_fn_void.__unnamed.0.c0
  76      89003F05 
  77              	# line 10
  10:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/Size1_fn_void.c **** 	putchar(s.c0);
  78              		.loc	1 10 0
  79 0178 00000000 		or	%s62,%s63,(0)1
  79      00BF3E45 
  80 0180 00000000 		or	%s0,0,%s62
  80      BE000045 
  81 0188 E0FFFFFF 		st	%s63,-32(,%fp)	# spill
  81      89003F11 
  82 0190 00000000 		lea	%s12,putchar@PLT_LO(-24)
  82      00680C06 
  83 0198 00000000 		and	%s12,%s12,(32)0
  83      608C0C44 
  84 01a0 00000000 		sic	%lr
  84      00000A28 
  85 01a8 00000000 		lea.sl	%s12,putchar@PLT_HI(%s12,%lr)
  85      8A8C8C06 
  86 01b0 00000000 		bsic	%lr,(,%s12)		# putchar
  86      8C000A08 
  87 01b8 08000000 		br.l	.L_1.1
  87      00000F18 
  88              	.L_1.1:
  89              	# line 11
  11:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/Size1_fn_void.c **** 	return s;
  90              		.loc	1 11 0
  91 01c0 E8FFFFFF 		ld	%s63,-24(,%fp)	# restore
  91      89003F01 
  92 01c8 E0FFFFFF 		ld	%s62,-32(,%fp)	# restore
  92      89003E01 
  93 01d0 00000000 		st1b	%s62,0(0,%s63)
  93      BF003E15 
  94              	# Start of epilogue codes
  95 01d8 00000000 		or	%sp,0,%fp
  95      89000B45 
  96              		.cfi_def_cfa	11,8
  97 01e0 18000000 		ld	%got,0x18(,%sp)
  97      8B000F01 
  98 01e8 20000000 		ld	%plt,0x20(,%sp)
  98      8B001001 
  99 01f0 08000000 		ld	%lr,0x8(,%sp)
  99      8B000A01 
 100 01f8 00000000 		ld	%fp,0x0(,%sp)
 100      8B000901 
 101 0200 00000000 		b.l	(,%lr)
 101      8A000F19 
 102              	.L_1.EoE:
 103              		.cfi_endproc
 104              		.set	.L.1.2auto_size,	0xfffffffffffffe10	# 496 Bytes
 106              	# ============ End  Size1_fn_void ============
 107              	# ============ Begin  Size1_fn_void2 ============
 108              		.section	.bss
 109              		.local	size1_ret
 110              	.comm size1_ret,1,1
 111              		.text
 112 0208 00000000 		.balign 16
 112      00000000 
 113              	.L_2.0:
 114              	# line 13
  12:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/Size1_fn_void.c **** }
  13:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/Size1_fn_void.c **** Size1 Size1_fn_void2() {
 115              		.loc	1 13 0
 116              		.globl	Size1_fn_void2
 118              	Size1_fn_void2:
 119              		.cfi_startproc
 120 0210 00000000 		st	%fp,0x0(,%sp)
 120      8B000911 
 121              		.cfi_def_cfa_offset	0
 122              		.cfi_offset	9,0
 123 0218 08000000 		st	%lr,0x8(,%sp)
 123      8B000A11 
 124 0220 18000000 		st	%got,0x18(,%sp)
 124      8B000F11 
 125 0228 20000000 		st	%plt,0x20(,%sp)
 125      8B001011 
 126 0230 00000000 		lea	%got,_GLOBAL_OFFSET_TABLE_@PC_LO(-0x18)
 126      00680F06 
 127 0238 00000000 		and	%got,%got,(32)0
 127      608F0F44 
 128 0240 00000000 		sic	%plt
 128      00001028 
 129 0248 00000000 		lea.sl	%got,_GLOBAL_OFFSET_TABLE_@PC_HI(%plt,%got)
 129      8F908F06 
 130 0250 00000000 		or	%fp,0,%sp
 130      8B000945 
 131              		.cfi_def_cfa_register	9
 132 0258 30000000 		st	%s18,48(,%fp)
 132      89001211 
 133 0260 38000000 		st	%s19,56(,%fp)
 133      89001311 
 134 0268 40000000 		st	%s20,64(,%fp)
 134      89001411 
 135 0270 48000000 		st	%s21,72(,%fp)
 135      89001511 
 136 0278 50000000 		st	%s22,80(,%fp)
 136      89001611 
 137 0280 58000000 		st	%s23,88(,%fp)
 137      89001711 
 138 0288 60000000 		st	%s24,96(,%fp)
 138      89001811 
 139 0290 68000000 		st	%s25,104(,%fp)
 139      89001911 
 140 0298 70000000 		st	%s26,112(,%fp)
 140      89001A11 
 141 02a0 78000000 		st	%s27,120(,%fp)
 141      89001B11 
 142 02a8 80000000 		st	%s28,128(,%fp)
 142      89001C11 
 143 02b0 88000000 		st	%s29,136(,%fp)
 143      89001D11 
 144 02b8 90000000 		st	%s30,144(,%fp)
 144      89001E11 
 145 02c0 98000000 		st	%s31,152(,%fp)
 145      89001F11 
 146 02c8 A0000000 		st	%s32,160(,%fp)
 146      89002011 
 147 02d0 A8000000 		st	%s33,168(,%fp)
 147      89002111 
 148 02d8 10FEFFFF 		lea	%s13,.L.2.2auto_size&0xffffffff
 148      00000D06 
 149 02e0 00000000 		and	%s13,%s13,(32)0
 149      608D0D44 
 150 02e8 FFFFFFFF 		lea.sl	%sp,.L.2.2auto_size>>32(%fp,%s13)
 150      8D898B06 
 151 02f0 48000000 		brge.l.t	%sp,%sl,.L_2.EoP
 151      888B3518 
 152 02f8 18000000 		ld	%s61,0x18(,%tp)
 152      8E003D01 
 153 0300 00000000 		or	%s62,0,%s0
 153      80003E45 
 154 0308 3B010000 		lea	%s63,0x13b
 154      00003F06 
 155 0310 00000000 		shm.l	%s63,0x0(%s61)
 155      BD033F31 
 156 0318 08000000 		shm.l	%sl,0x8(%s61)
 156      BD030831 
 157 0320 10000000 		shm.l	%sp,0x10(%s61)
 157      BD030B31 
 158 0328 00000000 		monc
 158      0000003F 
 159 0330 00000000 		or	%s0,0,%s62
 159      BE000045 
 160              	.L_2.EoP:
 161              	# End of prologue codes
 162              	# line 14
  14:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/Size1_fn_void.c **** 	Size1 *p_ret = &size1_ret;
 163              		.loc	1 14 0
 164 0338 00000000 		lea	%s63,size1_ret@GOTOFF_LO
 164      00003F06 
 165 0340 00000000 		and	%s63,%s63,(32)0
 165      60BF3F44 
 166 0348 00000000 		lea.sl	%s63,size1_ret@GOTOFF_HI(%s63,%got)
 166      8FBFBF06 
 167              	# line 15
  15:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/Size1_fn_void.c **** 	*p_ret = ext_Size1_fn_void();
 168              		.loc	1 15 0
 169 0350 00000000 		adds.l	%s62,%fp,(60)1
 169      3C893E59 
 170 0358 B0000000 		st	%s62,176(0,%sp)
 170      8B003E11 
 171 0360 E8FFFFFF 		st	%s0,-24(,%fp)	# spill
 171      89000011 
 172 0368 00000000 		or	%s0,0,%s62
 172      BE000045 
 173 0370 E0FFFFFF 		st	%s63,-32(,%fp)	# spill
 173      89003F11 
 174 0378 00000000 		lea	%s12,ext_Size1_fn_void@PLT_LO(-24)
 174      00680C06 
 175 0380 00000000 		and	%s12,%s12,(32)0
 175      608C0C44 
 176 0388 00000000 		sic	%lr
 176      00000A28 
 177 0390 00000000 		lea.sl	%s12,ext_Size1_fn_void@PLT_HI(%s12,%lr)
 177      8A8C8C06 
 178 0398 00000000 		bsic	%lr,(,%s12)		# ext_Size1_fn_void
 178      8C000A08 
 179 03a0 08000000 		br.l	.L_2.1
 179      00000F18 
 180              	.L_2.1:
 181 03a8 F0FFFFFF 		ld1b.sx	%s63,-16(0,%fp)	# Size1_fn_void2.__unnamed.0.c0
 181      89003F05 
 182 03b0 E0FFFFFF 		ld	%s62,-32(,%fp)	# restore
 182      89003E01 
 183 03b8 00000000 		st1b	%s63,0(0,%s62)	# *(p_ret).c0
 183      BE003F15 
 184              	# line 16
  16:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/Size1_fn_void.c **** 	return *p_ret;
 185              		.loc	1 16 0
 186 03c0 E8FFFFFF 		ld	%s61,-24(,%fp)	# restore
 186      89003D01 
 187 03c8 00000000 		st1b	%s63,0(0,%s61)
 187      BD003F15 
 188              	# Start of epilogue codes
 189 03d0 00000000 		or	%sp,0,%fp
 189      89000B45 
 190              		.cfi_def_cfa	11,8
 191 03d8 18000000 		ld	%got,0x18(,%sp)
 191      8B000F01 
 192 03e0 20000000 		ld	%plt,0x20(,%sp)
 192      8B001001 
 193 03e8 08000000 		ld	%lr,0x8(,%sp)
 193      8B000A01 
 194 03f0 00000000 		ld	%fp,0x0(,%sp)
 194      8B000901 
 195 03f8 00000000 		b.l	(,%lr)
 195      8A000F19 
 196              	.L_2.EoE:
 197              		.cfi_endproc
 198              		.set	.L.2.2auto_size,	0xfffffffffffffe10	# 496 Bytes
 200              	# ============ End  Size1_fn_void2 ============
 201              	# ============ Begin  mt_Size1_fn_void ============
 202              		.section .rodata
 203              		.balign 16
 205              	.LP.mt_Size1_fn_void.s.__initializer.0:
 206 0000 61       		.byte	97
 207              		.text
 208              		.balign 16
 209              	.L_3.0:
 210              	# line 18
  17:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/Size1_fn_void.c **** }
  18:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/Size1_fn_void.c **** Size1 mt_Size1_fn_void() {
 211              		.loc	1 18 0
 212              		.globl	mt_Size1_fn_void
 214              	mt_Size1_fn_void:
 215              		.cfi_startproc
 216 0400 00000000 		st	%fp,0x0(,%sp)
 216      8B000911 
 217              		.cfi_def_cfa_offset	0
 218              		.cfi_offset	9,0
 219 0408 08000000 		st	%lr,0x8(,%sp)
 219      8B000A11 
 220 0410 18000000 		st	%got,0x18(,%sp)
 220      8B000F11 
 221 0418 20000000 		st	%plt,0x20(,%sp)
 221      8B001011 
 222 0420 00000000 		lea	%got,_GLOBAL_OFFSET_TABLE_@PC_LO(-0x18)
 222      00680F06 
 223 0428 00000000 		and	%got,%got,(32)0
 223      608F0F44 
 224 0430 00000000 		sic	%plt
 224      00001028 
 225 0438 00000000 		lea.sl	%got,_GLOBAL_OFFSET_TABLE_@PC_HI(%plt,%got)
 225      8F908F06 
 226 0440 00000000 		or	%fp,0,%sp
 226      8B000945 
 227              		.cfi_def_cfa_register	9
 228 0448 30000000 		st	%s18,48(,%fp)
 228      89001211 
 229 0450 38000000 		st	%s19,56(,%fp)
 229      89001311 
 230 0458 40000000 		st	%s20,64(,%fp)
 230      89001411 
 231 0460 48000000 		st	%s21,72(,%fp)
 231      89001511 
 232 0468 50000000 		st	%s22,80(,%fp)
 232      89001611 
 233 0470 58000000 		st	%s23,88(,%fp)
 233      89001711 
 234 0478 60000000 		st	%s24,96(,%fp)
 234      89001811 
 235 0480 68000000 		st	%s25,104(,%fp)
 235      89001911 
 236 0488 70000000 		st	%s26,112(,%fp)
 236      89001A11 
 237 0490 78000000 		st	%s27,120(,%fp)
 237      89001B11 
 238 0498 80000000 		st	%s28,128(,%fp)
 238      89001C11 
 239 04a0 88000000 		st	%s29,136(,%fp)
 239      89001D11 
 240 04a8 90000000 		st	%s30,144(,%fp)
 240      89001E11 
 241 04b0 98000000 		st	%s31,152(,%fp)
 241      89001F11 
 242 04b8 A0000000 		st	%s32,160(,%fp)
 242      89002011 
 243 04c0 A8000000 		st	%s33,168(,%fp)
 243      89002111 
 244 04c8 F0FFFFFF 		lea	%s13,.L.3.2auto_size&0xffffffff
 244      00000D06 
 245 04d0 00000000 		and	%s13,%s13,(32)0
 245      608D0D44 
 246 04d8 FFFFFFFF 		lea.sl	%sp,.L.3.2auto_size>>32(%fp,%s13)
 246      8D898B06 
 247 04e0 48000000 		brge.l.t	%sp,%sl,.L_3.EoP
 247      888B3518 
 248 04e8 18000000 		ld	%s61,0x18(,%tp)
 248      8E003D01 
 249 04f0 00000000 		or	%s62,0,%s0
 249      80003E45 
 250 04f8 3B010000 		lea	%s63,0x13b
 250      00003F06 
 251 0500 00000000 		shm.l	%s63,0x0(%s61)
 251      BD033F31 
 252 0508 08000000 		shm.l	%sl,0x8(%s61)
 252      BD030831 
 253 0510 10000000 		shm.l	%sp,0x10(%s61)
 253      BD030B31 
 254 0518 00000000 		monc
 254      0000003F 
 255 0520 00000000 		or	%s0,0,%s62
 255      BE000045 
 256              	.L_3.EoP:
 257              	# End of prologue codes
 258              	# line 19
  19:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/Size1_fn_void.c **** 	Size1 s = { 'a' };
 259              		.loc	1 19 0
 260 0528 00000000 		adds.l	%s63,%fp,(60)1
 260      3C893F59 
 261 0530 00000000 		lea	%s62,.LP.mt_Size1_fn_void.s.__initializer.0@GOTOFF_LO
 261      00003E06 
 262 0538 00000000 		and	%s62,%s62,(32)0
 262      60BE3E44 
 263 0540 00000000 		lea.sl	%s62,.LP.mt_Size1_fn_void.s.__initializer.0@GOTOFF_HI(%s62,%got)
 263      8FBEBE06 
 264 0548 00000000 		ld1b.zx	%s61,0(0,%s62)	# conflict.I8
 264      BE00BD05 
 265 0550 00000000 		st1b	%s61,0(0,%s63)	# conflict.I8
 265      BF003D15 
 266              	# line 20
  20:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/Size1_fn_void.c **** 	return s;
 267              		.loc	1 20 0
 268 0558 F0FFFFFF 		ld1b.sx	%s60,-16(0,%fp)	# s.c0
 268      89003C05 
 269 0560 00000000 		st1b	%s60,0(0,%s0)
 269      80003C15 
 270              	# Start of epilogue codes
 271 0568 00000000 		or	%sp,0,%fp
 271      89000B45 
 272              		.cfi_def_cfa	11,8
 273 0570 18000000 		ld	%got,0x18(,%sp)
 273      8B000F01 
 274 0578 20000000 		ld	%plt,0x20(,%sp)
 274      8B001001 
 275 0580 08000000 		ld	%lr,0x8(,%sp)
 275      8B000A01 
 276 0588 00000000 		ld	%fp,0x0(,%sp)
 276      8B000901 
 277 0590 00000000 		b.l	(,%lr)
 277      8A000F19 
 278              	.L_3.EoE:
 279              		.cfi_endproc
 280              		.set	.L.3.2auto_size,	0xfffffffffffffff0	# 16 Bytes
 282              	# ============ End  mt_Size1_fn_void ============
 283              	# ============ Begin  prt_Size1_fn_void ============
 284              		.section .rodata
 285 0001 00000000 		.balign 16
 285      00000000 
 285      00000000 
 285      000000
 287              	.LP.prt_Size1_fn_void.s.__initializer.1:
 288 0010 61       		.byte	97
 289 0011 00000000 		.balign 8
 289      000000
 291              	.LP.__string.0:
 292 0018 25       		.byte	37
 293 0019 73       		.byte	115
 294 001a 3A       		.byte	58
 295 001b 25       		.byte	37
 296 001c 75       		.byte	117
 297 001d 20       		.byte	32
 298 001e 53       		.byte	83
 299 001f 69       		.byte	105
 300 0020 7A       		.byte	122
 301 0021 65       		.byte	101
 302 0022 31       		.byte	49
 303 0023 7B       		.byte	123
 304 0024 25       		.byte	37
 305 0025 63       		.byte	99
 306 0026 7D       		.byte	125
 307 0027 0A       		.byte	10
 308 0028 00       		.zero	1
 309 0029 00000000 		.balign 8
 309      000000
 311              	.LP.__string.1:
 312 0030 2F       		.byte	47
 313 0031 75       		.byte	117
 314 0032 73       		.byte	115
 315 0033 72       		.byte	114
 316 0034 2F       		.byte	47
 317 0035 75       		.byte	117
 318 0036 68       		.byte	104
 319 0037 6F       		.byte	111
 320 0038 6D       		.byte	109
 321 0039 65       		.byte	101
 322 003a 2F       		.byte	47
 323 003b 61       		.byte	97
 324 003c 75       		.byte	117
 325 003d 72       		.byte	114
 326 003e 6F       		.byte	111
 327 003f 72       		.byte	114
 328 0040 61       		.byte	97
 329 0041 2F       		.byte	47
 330 0042 34       		.byte	52
 331 0043 67       		.byte	103
 332 0044 69       		.byte	105
 333 0045 2F       		.byte	47
 334 0046 6E       		.byte	110
 335 0047 6C       		.byte	108
 336 0048 61       		.byte	97
 337 0049 62       		.byte	98
 338 004a 68       		.byte	104
 339 004b 70       		.byte	112
 340 004c 67       		.byte	103
 341 004d 2F       		.byte	47
 342 004e 6B       		.byte	107
 343 004f 72       		.byte	114
 344 0050 75       		.byte	117
 345 0051 75       		.byte	117
 346 0052 73       		.byte	115
 347 0053 2F       		.byte	47
 348 0054 76       		.byte	118
 349 0055 74       		.byte	116
 350 0056 2F       		.byte	47
 351 0057 73       		.byte	115
 352 0058 72       		.byte	114
 353 0059 63       		.byte	99
 354 005a 2F       		.byte	47
 355 005b 61       		.byte	97
 356 005c 73       		.byte	115
 357 005d 6D       		.byte	109
 358 005e 2D       		.byte	45
 359 005f 65       		.byte	101
 360 0060 78       		.byte	120
 361 0061 61       		.byte	97
 362 0062 6D       		.byte	109
 363 0063 70       		.byte	112
 364 0064 6C       		.byte	108
 365 0065 65       		.byte	101
 366 0066 73       		.byte	115
 367 0067 2F       		.byte	47
 368 0068 53       		.byte	83
 369 0069 69       		.byte	105
 370 006a 7A       		.byte	122
 371 006b 65       		.byte	101
 372 006c 31       		.byte	49
 373 006d 5F       		.byte	95
 374 006e 66       		.byte	102
 375 006f 6E       		.byte	110
 376 0070 5F       		.byte	95
 377 0071 76       		.byte	118
 378 0072 6F       		.byte	111
 379 0073 69       		.byte	105
 380 0074 64       		.byte	100
 381 0075 2E       		.byte	46
 382 0076 63       		.byte	99
 383 0077 00       		.zero	1
 384              		.text
 385 0598 00000000 		.balign 16
 385      00000000 
 386              	.L_4.0:
 387              	# line 22
  21:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/Size1_fn_void.c **** }
  22:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/Size1_fn_void.c **** Size1 prt_Size1_fn_void() {
 388              		.loc	1 22 0
 389              		.globl	prt_Size1_fn_void
 391              	prt_Size1_fn_void:
 392              		.cfi_startproc
 393 05a0 00000000 		st	%fp,0x0(,%sp)
 393      8B000911 
 394              		.cfi_def_cfa_offset	0
 395              		.cfi_offset	9,0
 396 05a8 08000000 		st	%lr,0x8(,%sp)
 396      8B000A11 
 397 05b0 18000000 		st	%got,0x18(,%sp)
 397      8B000F11 
 398 05b8 20000000 		st	%plt,0x20(,%sp)
 398      8B001011 
 399 05c0 00000000 		lea	%got,_GLOBAL_OFFSET_TABLE_@PC_LO(-0x18)
 399      00680F06 
 400 05c8 00000000 		and	%got,%got,(32)0
 400      608F0F44 
 401 05d0 00000000 		sic	%plt
 401      00001028 
 402 05d8 00000000 		lea.sl	%got,_GLOBAL_OFFSET_TABLE_@PC_HI(%plt,%got)
 402      8F908F06 
 403 05e0 00000000 		or	%fp,0,%sp
 403      8B000945 
 404              		.cfi_def_cfa_register	9
 405 05e8 30000000 		st	%s18,48(,%fp)
 405      89001211 
 406 05f0 38000000 		st	%s19,56(,%fp)
 406      89001311 
 407 05f8 40000000 		st	%s20,64(,%fp)
 407      89001411 
 408 0600 48000000 		st	%s21,72(,%fp)
 408      89001511 
 409 0608 50000000 		st	%s22,80(,%fp)
 409      89001611 
 410 0610 58000000 		st	%s23,88(,%fp)
 410      89001711 
 411 0618 60000000 		st	%s24,96(,%fp)
 411      89001811 
 412 0620 68000000 		st	%s25,104(,%fp)
 412      89001911 
 413 0628 70000000 		st	%s26,112(,%fp)
 413      89001A11 
 414 0630 78000000 		st	%s27,120(,%fp)
 414      89001B11 
 415 0638 80000000 		st	%s28,128(,%fp)
 415      89001C11 
 416 0640 88000000 		st	%s29,136(,%fp)
 416      89001D11 
 417 0648 90000000 		st	%s30,144(,%fp)
 417      89001E11 
 418 0650 98000000 		st	%s31,152(,%fp)
 418      89001F11 
 419 0658 A0000000 		st	%s32,160(,%fp)
 419      89002011 
 420 0660 A8000000 		st	%s33,168(,%fp)
 420      89002111 
 421 0668 10FEFFFF 		lea	%s13,.L.4.2auto_size&0xffffffff
 421      00000D06 
 422 0670 00000000 		and	%s13,%s13,(32)0
 422      608D0D44 
 423 0678 FFFFFFFF 		lea.sl	%sp,.L.4.2auto_size>>32(%fp,%s13)
 423      8D898B06 
 424 0680 48000000 		brge.l.t	%sp,%sl,.L_4.EoP
 424      888B3518 
 425 0688 18000000 		ld	%s61,0x18(,%tp)
 425      8E003D01 
 426 0690 00000000 		or	%s62,0,%s0
 426      80003E45 
 427 0698 3B010000 		lea	%s63,0x13b
 427      00003F06 
 428 06a0 00000000 		shm.l	%s63,0x0(%s61)
 428      BD033F31 
 429 06a8 08000000 		shm.l	%sl,0x8(%s61)
 429      BD030831 
 430 06b0 10000000 		shm.l	%sp,0x10(%s61)
 430      BD030B31 
 431 06b8 00000000 		monc
 431      0000003F 
 432 06c0 00000000 		or	%s0,0,%s62
 432      BE000045 
 433              	.L_4.EoP:
 434              	# End of prologue codes
 435              	# line 23
  23:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/Size1_fn_void.c **** 	Size1 s = { 'a' };
 436              		.loc	1 23 0
 437 06c8 00000000 		adds.l	%s63,%fp,(60)1
 437      3C893F59 
 438 06d0 00000000 		lea	%s62,.LP.prt_Size1_fn_void.s.__initializer.1@GOTOFF_LO
 438      00003E06 
 439 06d8 00000000 		and	%s62,%s62,(32)0
 439      60BE3E44 
 440 06e0 00000000 		lea.sl	%s62,.LP.prt_Size1_fn_void.s.__initializer.1@GOTOFF_HI(%s62,%got)
 440      8FBEBE06 
 441 06e8 00000000 		ld1b.zx	%s61,0(0,%s62)	# conflict.I8
 441      BE00BD05 
 442 06f0 00000000 		st1b	%s61,0(0,%s63)	# conflict.I8
 442      BF003D15 
 443              	# line 24
  24:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/Size1_fn_void.c **** 	printf("%s:%u Size1{%c}\n",__FILE__,__LINE__,s.c0);
 444              		.loc	1 24 0
 445 06f8 F0FFFFFF 		ld1b.sx	%s60,-16(0,%fp)	# s.c0
 445      89003C05 
 446 0700 00000000 		or	%s59,%s60,(0)1
 446      00BC3B45 
 447 0708 C8000000 		st	%s59,200(0,%sp)
 447      8B003B11 
 448 0710 00000000 		lea	%s58,.LP.__string.0@GOTOFF_LO
 448      00003A06 
 449 0718 00000000 		and	%s58,%s58,(32)0
 449      60BA3A44 
 450 0720 00000000 		lea.sl	%s58,.LP.__string.0@GOTOFF_HI(%s58,%got)
 450      8FBABA06 
 451 0728 B0000000 		st	%s58,176(0,%sp)
 451      8B003A11 
 452 0730 00000000 		lea	%s57,.LP.__string.1@GOTOFF_LO
 452      00003906 
 453 0738 00000000 		and	%s57,%s57,(32)0
 453      60B93944 
 454 0740 00000000 		lea.sl	%s57,.LP.__string.1@GOTOFF_HI(%s57,%got)
 454      8FB9B906 
 455 0748 B8000000 		st	%s57,184(0,%sp)
 455      8B003911 
 456 0750 00000000 		or	%s56,24,(0)1
 456      00183845 
 457 0758 C0000000 		st	%s56,192(0,%sp)
 457      8B003811 
 458 0760 E8FFFFFF 		st	%s0,-24(,%fp)	# spill
 458      89000011 
 459 0768 00000000 		or	%s0,0,%s58
 459      BA000045 
 460 0770 00000000 		or	%s1,0,%s57
 460      B9000145 
 461 0778 00000000 		or	%s2,0,%s56
 461      B8000245 
 462 0780 00000000 		or	%s3,0,%s59
 462      BB000345 
 463 0788 00000000 		lea	%s12,printf@PLT_LO(-24)
 463      00680C06 
 464 0790 00000000 		and	%s12,%s12,(32)0
 464      608C0C44 
 465 0798 00000000 		sic	%lr
 465      00000A28 
 466 07a0 00000000 		lea.sl	%s12,printf@PLT_HI(%s12,%lr)
 466      8A8C8C06 
 467 07a8 00000000 		bsic	%lr,(,%s12)		# printf
 467      8C000A08 
 468 07b0 08000000 		br.l	.L_4.1
 468      00000F18 
 469              	.L_4.1:
 470              	# line 25
  25:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/Size1_fn_void.c **** 	return s;
 471              		.loc	1 25 0
 472 07b8 F0FFFFFF 		ld1b.sx	%s63,-16(0,%fp)	# s.c0
 472      89003F05 
 473 07c0 E8FFFFFF 		ld	%s62,-24(,%fp)	# restore
 473      89003E01 
 474 07c8 00000000 		st1b	%s63,0(0,%s62)
 474      BE003F15 
 475              	# Start of epilogue codes
 476 07d0 00000000 		or	%sp,0,%fp
 476      89000B45 
 477              		.cfi_def_cfa	11,8
 478 07d8 18000000 		ld	%got,0x18(,%sp)
 478      8B000F01 
 479 07e0 20000000 		ld	%plt,0x20(,%sp)
 479      8B001001 
 480 07e8 08000000 		ld	%lr,0x8(,%sp)
 480      8B000A01 
 481 07f0 00000000 		ld	%fp,0x0(,%sp)
 481      8B000901 
 482 07f8 00000000 		b.l	(,%lr)
 482      8A000F19 
 483              	.L_4.EoE:
 484              		.cfi_endproc
 485              		.set	.L.4.2auto_size,	0xfffffffffffffe10	# 496 Bytes
 487              	# ============ End  prt_Size1_fn_void ============
 488              	.Le1.0:
