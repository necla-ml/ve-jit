   1              		.ident "ncc 1.1.4 (Build 10:17:22 Apr 19 2018)"
   2              		.file "fq_fn_8fq.c"
   3              		.file 1 "/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/fq_fn_8fq.c"
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
  14              	# ============ Begin  fq_fn_8fq ============
  15              		.section	.bss
  16              		.local	fq_ret
  17              	.comm fq_ret,8,4
  18              		.text
  19              		.balign 16
  20              	.L_1.0:
  21              	# line 7
   1:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/fq_fn_8fq.c **** #include <stdio.h>
   2:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/fq_fn_8fq.c **** #include <complex.h>
   3:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/fq_fn_8fq.c **** 
   4:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/fq_fn_8fq.c **** extern float complex ext_fq_fn_8fq(float complex a, float complex b, float complex c, float complex
   5:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/fq_fn_8fq.c **** 
   6:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/fq_fn_8fq.c **** static float complex fq_ret;
   7:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/fq_fn_8fq.c **** float complex fq_fn_8fq(float complex a, float complex b, float complex c, float complex d, float c
  22              		.loc	1 7 0
  23              		.globl	fq_fn_8fq
  25              	fq_fn_8fq:
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
  55 00c8 B0FDFFFF 		lea	%s13,.L.1.2auto_size&0xffffffff
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
  69 0128 24010000 		ldu	%s63,292(0,%fp)	# fq_fn_8fq.__unnamed.7 (real)
  69      89003F02 
  70              	# line 8
   8:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/fq_fn_8fq.c **** 	fq_ret = ext_fq_fn_8fq(a,b,c,d,e,f,g,h);
  71              		.loc	1 8 0
  72 0130 20010000 		st	%s63,288(0,%sp)
  72      8B003F11 
  73              	# line 7
   7:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/fq_fn_8fq.c **** 	fq_ret = ext_fq_fn_8fq(a,b,c,d,e,f,g,h);
  74              		.loc	1 7 0
  75 0138 2C010000 		ldu	%s62,300(0,%fp)	# fq_fn_8fq.__unnamed.7 (imaginary)
  75      89003E02 
  76              	# line 8
  77              		.loc	1 8 0
  78 0140 28010000 		st	%s62,296(0,%sp)
  78      8B003E11 
  79              	# line 7
   7:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/fq_fn_8fq.c **** 	fq_ret = ext_fq_fn_8fq(a,b,c,d,e,f,g,h);
  80              		.loc	1 7 0
  81 0148 14010000 		ldu	%s61,276(0,%fp)	# fq_fn_8fq.__unnamed.6 (real)
  81      89003D02 
  82              	# line 8
  83              		.loc	1 8 0
  84 0150 10010000 		st	%s61,272(0,%sp)
  84      8B003D11 
  85              	# line 7
   7:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/fq_fn_8fq.c **** 	fq_ret = ext_fq_fn_8fq(a,b,c,d,e,f,g,h);
  86              		.loc	1 7 0
  87 0158 1C010000 		ldu	%s60,284(0,%fp)	# fq_fn_8fq.__unnamed.6 (imaginary)
  87      89003C02 
  88              	# line 8
  89              		.loc	1 8 0
  90 0160 18010000 		st	%s60,280(0,%sp)
  90      8B003C11 
  91              	# line 7
   7:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/fq_fn_8fq.c **** 	fq_ret = ext_fq_fn_8fq(a,b,c,d,e,f,g,h);
  92              		.loc	1 7 0
  93 0168 04010000 		ldu	%s59,260(0,%fp)	# fq_fn_8fq.__unnamed.5 (real)
  93      89003B02 
  94              	# line 8
  95              		.loc	1 8 0
  96 0170 00010000 		st	%s59,256(0,%sp)
  96      8B003B11 
  97              	# line 7
   7:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/fq_fn_8fq.c **** 	fq_ret = ext_fq_fn_8fq(a,b,c,d,e,f,g,h);
  98              		.loc	1 7 0
  99 0178 0C010000 		ldu	%s58,268(0,%fp)	# fq_fn_8fq.__unnamed.5 (imaginary)
  99      89003A02 
 100              	# line 8
 101              		.loc	1 8 0
 102 0180 08010000 		st	%s58,264(0,%sp)
 102      8B003A11 
 103              	# line 7
   7:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/fq_fn_8fq.c **** 	fq_ret = ext_fq_fn_8fq(a,b,c,d,e,f,g,h);
 104              		.loc	1 7 0
 105 0188 F4000000 		ldu	%s57,244(0,%fp)	# fq_fn_8fq.__unnamed.4 (real)
 105      89003902 
 106              	# line 8
 107              		.loc	1 8 0
 108 0190 F0000000 		st	%s57,240(0,%sp)
 108      8B003911 
 109              	# line 7
   7:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/fq_fn_8fq.c **** 	fq_ret = ext_fq_fn_8fq(a,b,c,d,e,f,g,h);
 110              		.loc	1 7 0
 111 0198 FC000000 		ldu	%s56,252(0,%fp)	# fq_fn_8fq.__unnamed.4 (imaginary)
 111      89003802 
 112              	# line 8
 113              		.loc	1 8 0
 114 01a0 F8000000 		st	%s56,248(0,%sp)
 114      8B003811 
 115 01a8 00000000 		lea	%s12,ext_fq_fn_8fq@PLT_LO(-24)
 115      00680C06 
 116 01b0 00000000 		and	%s12,%s12,(32)0
 116      608C0C44 
 117 01b8 00000000 		sic	%lr
 117      00000A28 
 118 01c0 00000000 		lea.sl	%s12,ext_fq_fn_8fq@PLT_HI(%s12,%lr)
 118      8A8C8C06 
 119 01c8 00000000 		bsic	%lr,(,%s12)		# ext_fq_fn_8fq
 119      8C000A08 
 120 01d0 F8FFFFFF 		st	%s0,-8(,%fp)	# spill
 120      89000011 
 121 01d8 F0FFFFFF 		st	%s1,-16(,%fp)	# spill
 121      89000111 
 122 01e0 08000000 		br.l	.L_1.1
 122      00000F18 
 123              	.L_1.1:
 124 01e8 00000000 		lea	%s63,fq_ret@GOTOFF_LO
 124      00003F06 
 125 01f0 00000000 		and	%s63,%s63,(32)0
 125      60BF3F44 
 126 01f8 00000000 		lea.sl	%s63,fq_ret@GOTOFF_HI(%s63,%got)
 126      8FBFBF06 
 127 0200 F8FFFFFF 		ld	%s62,-8(,%fp)	# restore
 127      89003E01 
 128 0208 00000000 		stu	%s62,0(0,%s63)	# fq_ret (real)
 128      BF003E12 
 129 0210 F0FFFFFF 		ld	%s61,-16(,%fp)	# restore
 129      89003D01 
 130 0218 04000000 		stu	%s61,4(0,%s63)	# fq_ret (imaginary)
 130      BF003D12 
 131              	# line 9
   9:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/fq_fn_8fq.c **** 	return fq_ret;
 132              		.loc	1 9 0
 133 0220 00000000 		or	%s0,0,%s62
 133      BE000045 
 134 0228 00000000 		or	%s1,0,%s61
 134      BD000145 
 135              	# Start of epilogue codes
 136 0230 00000000 		or	%sp,0,%fp
 136      89000B45 
 137              		.cfi_def_cfa	11,8
 138 0238 18000000 		ld	%got,0x18(,%sp)
 138      8B000F01 
 139 0240 20000000 		ld	%plt,0x20(,%sp)
 139      8B001001 
 140 0248 08000000 		ld	%lr,0x8(,%sp)
 140      8B000A01 
 141 0250 00000000 		ld	%fp,0x0(,%sp)
 141      8B000901 
 142 0258 00000000 		b.l	(,%lr)
 142      8A000F19 
 143              	.L_1.EoE:
 144              		.cfi_endproc
 145              		.set	.L.1.2auto_size,	0xfffffffffffffdb0	# 592 Bytes
 147              	# ============ End  fq_fn_8fq ============
 148              	# ============ Begin  fq_fn_8fq2 ============
 149              		.balign 16
 150              	.L_2.0:
 151              	# line 11
  10:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/fq_fn_8fq.c **** }
  11:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/fq_fn_8fq.c **** float complex fq_fn_8fq2(float complex a, float complex b, float complex c, float complex d, float 
 152              		.loc	1 11 0
 153              		.globl	fq_fn_8fq2
 155              	fq_fn_8fq2:
 156              		.cfi_startproc
 157 0260 00000000 		st	%fp,0x0(,%sp)
 157      8B000911 
 158              		.cfi_def_cfa_offset	0
 159              		.cfi_offset	9,0
 160 0268 08000000 		st	%lr,0x8(,%sp)
 160      8B000A11 
 161 0270 18000000 		st	%got,0x18(,%sp)
 161      8B000F11 
 162 0278 20000000 		st	%plt,0x20(,%sp)
 162      8B001011 
 163 0280 00000000 		lea	%got,_GLOBAL_OFFSET_TABLE_@PC_LO(-0x18)
 163      00680F06 
 164 0288 00000000 		and	%got,%got,(32)0
 164      608F0F44 
 165 0290 00000000 		sic	%plt
 165      00001028 
 166 0298 00000000 		lea.sl	%got,_GLOBAL_OFFSET_TABLE_@PC_HI(%plt,%got)
 166      8F908F06 
 167 02a0 00000000 		or	%fp,0,%sp
 167      8B000945 
 168              		.cfi_def_cfa_register	9
 169 02a8 30000000 		st	%s18,48(,%fp)
 169      89001211 
 170 02b0 38000000 		st	%s19,56(,%fp)
 170      89001311 
 171 02b8 40000000 		st	%s20,64(,%fp)
 171      89001411 
 172 02c0 48000000 		st	%s21,72(,%fp)
 172      89001511 
 173 02c8 50000000 		st	%s22,80(,%fp)
 173      89001611 
 174 02d0 58000000 		st	%s23,88(,%fp)
 174      89001711 
 175 02d8 60000000 		st	%s24,96(,%fp)
 175      89001811 
 176 02e0 68000000 		st	%s25,104(,%fp)
 176      89001911 
 177 02e8 70000000 		st	%s26,112(,%fp)
 177      89001A11 
 178 02f0 78000000 		st	%s27,120(,%fp)
 178      89001B11 
 179 02f8 80000000 		st	%s28,128(,%fp)
 179      89001C11 
 180 0300 88000000 		st	%s29,136(,%fp)
 180      89001D11 
 181 0308 90000000 		st	%s30,144(,%fp)
 181      89001E11 
 182 0310 98000000 		st	%s31,152(,%fp)
 182      89001F11 
 183 0318 A0000000 		st	%s32,160(,%fp)
 183      89002011 
 184 0320 A8000000 		st	%s33,168(,%fp)
 184      89002111 
 185 0328 A0FDFFFF 		lea	%s13,.L.2.2auto_size&0xffffffff
 185      00000D06 
 186 0330 00000000 		and	%s13,%s13,(32)0
 186      608D0D44 
 187 0338 FFFFFFFF 		lea.sl	%sp,.L.2.2auto_size>>32(%fp,%s13)
 187      8D898B06 
 188 0340 48000000 		brge.l.t	%sp,%sl,.L_2.EoP
 188      888B3518 
 189 0348 18000000 		ld	%s61,0x18(,%tp)
 189      8E003D01 
 190 0350 00000000 		or	%s62,0,%s0
 190      80003E45 
 191 0358 3B010000 		lea	%s63,0x13b
 191      00003F06 
 192 0360 00000000 		shm.l	%s63,0x0(%s61)
 192      BD033F31 
 193 0368 08000000 		shm.l	%sl,0x8(%s61)
 193      BD030831 
 194 0370 10000000 		shm.l	%sp,0x10(%s61)
 194      BD030B31 
 195 0378 00000000 		monc
 195      0000003F 
 196 0380 00000000 		or	%s0,0,%s62
 196      BE000045 
 197              	.L_2.EoP:
 198              	# End of prologue codes
 199 0388 24010000 		ldu	%s63,292(0,%fp)	# fq_fn_8fq2.__unnamed.7 (real)
 199      89003F02 
 200              	# line 13
  12:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/fq_fn_8fq.c **** 	float complex *p_ret = &fq_ret;
  13:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/fq_fn_8fq.c **** 	*p_ret = ext_fq_fn_8fq(a,b,c,d,e,f,g,h);
 201              		.loc	1 13 0
 202 0390 20010000 		st	%s63,288(0,%sp)
 202      8B003F11 
 203              	# line 11
  11:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/fq_fn_8fq.c **** 	float complex *p_ret = &fq_ret;
 204              		.loc	1 11 0
 205 0398 2C010000 		ldu	%s62,300(0,%fp)	# fq_fn_8fq2.__unnamed.7 (imaginary)
 205      89003E02 
 206              	# line 13
 207              		.loc	1 13 0
 208 03a0 28010000 		st	%s62,296(0,%sp)
 208      8B003E11 
 209              	# line 11
  11:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/fq_fn_8fq.c **** 	float complex *p_ret = &fq_ret;
 210              		.loc	1 11 0
 211 03a8 14010000 		ldu	%s61,276(0,%fp)	# fq_fn_8fq2.__unnamed.6 (real)
 211      89003D02 
 212              	# line 13
 213              		.loc	1 13 0
 214 03b0 10010000 		st	%s61,272(0,%sp)
 214      8B003D11 
 215              	# line 11
  11:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/fq_fn_8fq.c **** 	float complex *p_ret = &fq_ret;
 216              		.loc	1 11 0
 217 03b8 1C010000 		ldu	%s60,284(0,%fp)	# fq_fn_8fq2.__unnamed.6 (imaginary)
 217      89003C02 
 218              	# line 13
 219              		.loc	1 13 0
 220 03c0 18010000 		st	%s60,280(0,%sp)
 220      8B003C11 
 221              	# line 11
  11:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/fq_fn_8fq.c **** 	float complex *p_ret = &fq_ret;
 222              		.loc	1 11 0
 223 03c8 04010000 		ldu	%s59,260(0,%fp)	# fq_fn_8fq2.__unnamed.5 (real)
 223      89003B02 
 224              	# line 13
 225              		.loc	1 13 0
 226 03d0 00010000 		st	%s59,256(0,%sp)
 226      8B003B11 
 227              	# line 11
  11:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/fq_fn_8fq.c **** 	float complex *p_ret = &fq_ret;
 228              		.loc	1 11 0
 229 03d8 0C010000 		ldu	%s58,268(0,%fp)	# fq_fn_8fq2.__unnamed.5 (imaginary)
 229      89003A02 
 230              	# line 13
 231              		.loc	1 13 0
 232 03e0 08010000 		st	%s58,264(0,%sp)
 232      8B003A11 
 233              	# line 11
  11:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/fq_fn_8fq.c **** 	float complex *p_ret = &fq_ret;
 234              		.loc	1 11 0
 235 03e8 F4000000 		ldu	%s57,244(0,%fp)	# fq_fn_8fq2.__unnamed.4 (real)
 235      89003902 
 236              	# line 13
 237              		.loc	1 13 0
 238 03f0 F0000000 		st	%s57,240(0,%sp)
 238      8B003911 
 239              	# line 11
  11:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/fq_fn_8fq.c **** 	float complex *p_ret = &fq_ret;
 240              		.loc	1 11 0
 241 03f8 FC000000 		ldu	%s56,252(0,%fp)	# fq_fn_8fq2.__unnamed.4 (imaginary)
 241      89003802 
 242              	# line 13
 243              		.loc	1 13 0
 244 0400 F8000000 		st	%s56,248(0,%sp)
 244      8B003811 
 245              	# line 12
  12:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/fq_fn_8fq.c **** 	float complex *p_ret = &fq_ret;
 246              		.loc	1 12 0
 247 0408 00000000 		lea	%s55,fq_ret@GOTOFF_LO
 247      00003706 
 248 0410 00000000 		and	%s55,%s55,(32)0
 248      60B73744 
 249 0418 00000000 		lea.sl	%s55,fq_ret@GOTOFF_HI(%s55,%got)
 249      8FB7B706 
 250 0420 00000000 		or	%s54,%s55,(0)1
 250      00B73645 
 251              	# line 13
 252              		.loc	1 13 0
 253 0428 F8FFFFFF 		st	%s54,-8(,%fp)	# spill
 253      89003611 
 254 0430 00000000 		lea	%s12,ext_fq_fn_8fq@PLT_LO(-24)
 254      00680C06 
 255 0438 00000000 		and	%s12,%s12,(32)0
 255      608C0C44 
 256 0440 00000000 		sic	%lr
 256      00000A28 
 257 0448 00000000 		lea.sl	%s12,ext_fq_fn_8fq@PLT_HI(%s12,%lr)
 257      8A8C8C06 
 258 0450 00000000 		bsic	%lr,(,%s12)		# ext_fq_fn_8fq
 258      8C000A08 
 259 0458 F0FFFFFF 		st	%s0,-16(,%fp)	# spill
 259      89000011 
 260 0460 E8FFFFFF 		st	%s1,-24(,%fp)	# spill
 260      89000111 
 261 0468 08000000 		br.l	.L_2.1
 261      00000F18 
 262              	.L_2.1:
 263 0470 F8FFFFFF 		ld	%s63,-8(,%fp)	# restore
 263      89003F01 
 264 0478 F0FFFFFF 		ld	%s62,-16(,%fp)	# restore
 264      89003E01 
 265 0480 00000000 		stu	%s62,0(0,%s63)	# *(p_ret) (real)
 265      BF003E12 
 266 0488 E8FFFFFF 		ld	%s61,-24(,%fp)	# restore
 266      89003D01 
 267 0490 04000000 		stu	%s61,4(0,%s63)	# *(p_ret) (imaginary)
 267      BF003D12 
 268              	# line 14
  14:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/fq_fn_8fq.c **** 	return *p_ret;
 269              		.loc	1 14 0
 270 0498 00000000 		or	%s0,0,%s62
 270      BE000045 
 271 04a0 00000000 		or	%s1,0,%s61
 271      BD000145 
 272              	# Start of epilogue codes
 273 04a8 00000000 		or	%sp,0,%fp
 273      89000B45 
 274              		.cfi_def_cfa	11,8
 275 04b0 18000000 		ld	%got,0x18(,%sp)
 275      8B000F01 
 276 04b8 20000000 		ld	%plt,0x20(,%sp)
 276      8B001001 
 277 04c0 08000000 		ld	%lr,0x8(,%sp)
 277      8B000A01 
 278 04c8 00000000 		ld	%fp,0x0(,%sp)
 278      8B000901 
 279 04d0 00000000 		b.l	(,%lr)
 279      8A000F19 
 280              	.L_2.EoE:
 281              		.cfi_endproc
 282              		.set	.L.2.2auto_size,	0xfffffffffffffda0	# 608 Bytes
 284              	# ============ End  fq_fn_8fq2 ============
 285              	# ============ Begin  mt_fq_fn_8fq ============
 286 04d8 00000000 		.balign 16
 286      00000000 
 287              	.L_3.0:
 288              	# line 16
  15:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/fq_fn_8fq.c **** }
  16:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/fq_fn_8fq.c **** float complex mt_fq_fn_8fq(float complex a, float complex b, float complex c, float complex d, floa
 289              		.loc	1 16 0
 290              		.globl	mt_fq_fn_8fq
 292              	mt_fq_fn_8fq:
 293              		.cfi_startproc
 294 04e0 00000000 		st	%fp,0x0(,%sp)
 294      8B000911 
 295              		.cfi_def_cfa_offset	0
 296              		.cfi_offset	9,0
 297 04e8 08000000 		st	%lr,0x8(,%sp)
 297      8B000A11 
 298 04f0 18000000 		st	%got,0x18(,%sp)
 298      8B000F11 
 299 04f8 20000000 		st	%plt,0x20(,%sp)
 299      8B001011 
 300 0500 00000000 		lea	%got,_GLOBAL_OFFSET_TABLE_@PC_LO(-0x18)
 300      00680F06 
 301 0508 00000000 		and	%got,%got,(32)0
 301      608F0F44 
 302 0510 00000000 		sic	%plt
 302      00001028 
 303 0518 00000000 		lea.sl	%got,_GLOBAL_OFFSET_TABLE_@PC_HI(%plt,%got)
 303      8F908F06 
 304 0520 00000000 		or	%fp,0,%sp
 304      8B000945 
 305              		.cfi_def_cfa_register	9
 306 0528 30000000 		st	%s18,48(,%fp)
 306      89001211 
 307 0530 38000000 		st	%s19,56(,%fp)
 307      89001311 
 308 0538 40000000 		st	%s20,64(,%fp)
 308      89001411 
 309 0540 48000000 		st	%s21,72(,%fp)
 309      89001511 
 310 0548 50000000 		st	%s22,80(,%fp)
 310      89001611 
 311 0550 58000000 		st	%s23,88(,%fp)
 311      89001711 
 312 0558 60000000 		st	%s24,96(,%fp)
 312      89001811 
 313 0560 68000000 		st	%s25,104(,%fp)
 313      89001911 
 314 0568 70000000 		st	%s26,112(,%fp)
 314      89001A11 
 315 0570 78000000 		st	%s27,120(,%fp)
 315      89001B11 
 316 0578 80000000 		st	%s28,128(,%fp)
 316      89001C11 
 317 0580 88000000 		st	%s29,136(,%fp)
 317      89001D11 
 318 0588 90000000 		st	%s30,144(,%fp)
 318      89001E11 
 319 0590 98000000 		st	%s31,152(,%fp)
 319      89001F11 
 320 0598 A0000000 		st	%s32,160(,%fp)
 320      89002011 
 321 05a0 A8000000 		st	%s33,168(,%fp)
 321      89002111 
 322 05a8 00000000 		lea	%s13,.L.3.2auto_size&0xffffffff
 322      00000D06 
 323 05b0 00000000 		and	%s13,%s13,(32)0
 323      608D0D44 
 324 05b8 00000000 		lea.sl	%sp,.L.3.2auto_size>>32(%fp,%s13)
 324      8D898B06 
 325 05c0 48000000 		brge.l.t	%sp,%sl,.L_3.EoP
 325      888B3518 
 326 05c8 18000000 		ld	%s61,0x18(,%tp)
 326      8E003D01 
 327 05d0 00000000 		or	%s62,0,%s0
 327      80003E45 
 328 05d8 3B010000 		lea	%s63,0x13b
 328      00003F06 
 329 05e0 00000000 		shm.l	%s63,0x0(%s61)
 329      BD033F31 
 330 05e8 08000000 		shm.l	%sl,0x8(%s61)
 330      BD030831 
 331 05f0 10000000 		shm.l	%sp,0x10(%s61)
 331      BD030B31 
 332 05f8 00000000 		monc
 332      0000003F 
 333 0600 00000000 		or	%s0,0,%s62
 333      BE000045 
 334              	.L_3.EoP:
 335              	# End of prologue codes
 336 0608 F4000000 		ldu	%s63,244(0,%fp)	# mt_fq_fn_8fq.__unnamed.4 (real)
 336      89003F02 
 337 0610 FC000000 		ldu	%s62,252(0,%fp)	# mt_fq_fn_8fq.__unnamed.4 (imaginary)
 337      89003E02 
 338              	# line 18
  17:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/fq_fn_8fq.c **** 	float complex ret = e;
  18:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/fq_fn_8fq.c **** 	return ret;
 339              		.loc	1 18 0
 340 0618 00000000 		or	%s0,0,%s63
 340      BF000045 
 341 0620 00000000 		or	%s1,0,%s62
 341      BE000145 
 342              	# Start of epilogue codes
 343 0628 00000000 		or	%sp,0,%fp
 343      89000B45 
 344              		.cfi_def_cfa	11,8
 345 0630 18000000 		ld	%got,0x18(,%sp)
 345      8B000F01 
 346 0638 20000000 		ld	%plt,0x20(,%sp)
 346      8B001001 
 347 0640 08000000 		ld	%lr,0x8(,%sp)
 347      8B000A01 
 348 0648 00000000 		ld	%fp,0x0(,%sp)
 348      8B000901 
 349 0650 00000000 		b.l	(,%lr)
 349      8A000F19 
 350              	.L_3.EoE:
 351              		.cfi_endproc
 352              		.set	.L.3.2auto_size,	0x0	# 0 Bytes
 354              	# ============ End  mt_fq_fn_8fq ============
 355              	# ============ Begin  sum_fq_fn_8fq ============
 356 0658 00000000 		.balign 16
 356      00000000 
 357              	.L_4.0:
 358              	# line 20
  19:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/fq_fn_8fq.c **** }
  20:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/fq_fn_8fq.c **** float complex sum_fq_fn_8fq(float complex a, float complex b, float complex c, float complex d, flo
 359              		.loc	1 20 0
 360              		.globl	sum_fq_fn_8fq
 362              	sum_fq_fn_8fq:
 363              		.cfi_startproc
 364 0660 00000000 		st	%fp,0x0(,%sp)
 364      8B000911 
 365              		.cfi_def_cfa_offset	0
 366              		.cfi_offset	9,0
 367 0668 08000000 		st	%lr,0x8(,%sp)
 367      8B000A11 
 368 0670 18000000 		st	%got,0x18(,%sp)
 368      8B000F11 
 369 0678 20000000 		st	%plt,0x20(,%sp)
 369      8B001011 
 370 0680 00000000 		lea	%got,_GLOBAL_OFFSET_TABLE_@PC_LO(-0x18)
 370      00680F06 
 371 0688 00000000 		and	%got,%got,(32)0
 371      608F0F44 
 372 0690 00000000 		sic	%plt
 372      00001028 
 373 0698 00000000 		lea.sl	%got,_GLOBAL_OFFSET_TABLE_@PC_HI(%plt,%got)
 373      8F908F06 
 374 06a0 00000000 		or	%fp,0,%sp
 374      8B000945 
 375              		.cfi_def_cfa_register	9
 376 06a8 30000000 		st	%s18,48(,%fp)
 376      89001211 
 377 06b0 38000000 		st	%s19,56(,%fp)
 377      89001311 
 378 06b8 40000000 		st	%s20,64(,%fp)
 378      89001411 
 379 06c0 48000000 		st	%s21,72(,%fp)
 379      89001511 
 380 06c8 50000000 		st	%s22,80(,%fp)
 380      89001611 
 381 06d0 58000000 		st	%s23,88(,%fp)
 381      89001711 
 382 06d8 60000000 		st	%s24,96(,%fp)
 382      89001811 
 383 06e0 68000000 		st	%s25,104(,%fp)
 383      89001911 
 384 06e8 70000000 		st	%s26,112(,%fp)
 384      89001A11 
 385 06f0 78000000 		st	%s27,120(,%fp)
 385      89001B11 
 386 06f8 80000000 		st	%s28,128(,%fp)
 386      89001C11 
 387 0700 88000000 		st	%s29,136(,%fp)
 387      89001D11 
 388 0708 90000000 		st	%s30,144(,%fp)
 388      89001E11 
 389 0710 98000000 		st	%s31,152(,%fp)
 389      89001F11 
 390 0718 A0000000 		st	%s32,160(,%fp)
 390      89002011 
 391 0720 A8000000 		st	%s33,168(,%fp)
 391      89002111 
 392 0728 00000000 		lea	%s13,.L.4.2auto_size&0xffffffff
 392      00000D06 
 393 0730 00000000 		and	%s13,%s13,(32)0
 393      608D0D44 
 394 0738 00000000 		lea.sl	%sp,.L.4.2auto_size>>32(%fp,%s13)
 394      8D898B06 
 395 0740 48000000 		brge.l.t	%sp,%sl,.L_4.EoP
 395      888B3518 
 396 0748 18000000 		ld	%s61,0x18(,%tp)
 396      8E003D01 
 397 0750 00000000 		or	%s62,0,%s0
 397      80003E45 
 398 0758 3B010000 		lea	%s63,0x13b
 398      00003F06 
 399 0760 00000000 		shm.l	%s63,0x0(%s61)
 399      BD033F31 
 400 0768 08000000 		shm.l	%sl,0x8(%s61)
 400      BD030831 
 401 0770 10000000 		shm.l	%sp,0x10(%s61)
 401      BD030B31 
 402 0778 00000000 		monc
 402      0000003F 
 403 0780 00000000 		or	%s0,0,%s62
 403      BE000045 
 404              	.L_4.EoP:
 405              	# End of prologue codes
 406              	# line 21
  21:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/fq_fn_8fq.c **** 	return a+b+c+d+e+f+g+h;
 407              		.loc	1 21 0
 408 0788 00000000 		fadd.s	%s63,%s1,%s3
 408      8381BF4C 
 409 0790 00000000 		fadd.s	%s62,%s5,%s63
 409      BF85BE4C 
 410 0798 00000000 		fadd.s	%s61,%s7,%s62
 410      BE87BD4C 
 411 07a0 00000000 		fadd.s	%s60,%s0,%s2
 411      8280BC4C 
 412 07a8 00000000 		fadd.s	%s59,%s4,%s60
 412      BC84BB4C 
 413 07b0 00000000 		fadd.s	%s58,%s6,%s59
 413      BB86BA4C 
 414              	# line 20
  20:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/fq_fn_8fq.c **** 	return a+b+c+d+e+f+g+h;
 415              		.loc	1 20 0
 416 07b8 24010000 		ldu	%s57,292(0,%fp)	# sum_fq_fn_8fq.__unnamed.7 (real)
 416      89003902 
 417 07c0 2C010000 		ldu	%s56,300(0,%fp)	# sum_fq_fn_8fq.__unnamed.7 (imaginary)
 417      89003802 
 418 07c8 14010000 		ldu	%s55,276(0,%fp)	# sum_fq_fn_8fq.__unnamed.6 (real)
 418      89003702 
 419 07d0 1C010000 		ldu	%s54,284(0,%fp)	# sum_fq_fn_8fq.__unnamed.6 (imaginary)
 419      89003602 
 420 07d8 04010000 		ldu	%s53,260(0,%fp)	# sum_fq_fn_8fq.__unnamed.5 (real)
 420      89003502 
 421 07e0 0C010000 		ldu	%s52,268(0,%fp)	# sum_fq_fn_8fq.__unnamed.5 (imaginary)
 421      89003402 
 422 07e8 F4000000 		ldu	%s51,244(0,%fp)	# sum_fq_fn_8fq.__unnamed.4 (real)
 422      89003302 
 423              	# line 21
 424              		.loc	1 21 0
 425 07f0 00000000 		fadd.s	%s50,%s58,%s51
 425      B3BAB24C 
 426 07f8 00000000 		fadd.s	%s49,%s53,%s50
 426      B2B5B14C 
 427 0800 00000000 		fadd.s	%s48,%s55,%s49
 427      B1B7B04C 
 428 0808 00000000 		fadd.s	%s47,%s57,%s48
 428      B0B9AF4C 
 429              	# line 20
  20:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/fq_fn_8fq.c **** 	return a+b+c+d+e+f+g+h;
 430              		.loc	1 20 0
 431 0810 FC000000 		ldu	%s46,252(0,%fp)	# sum_fq_fn_8fq.__unnamed.4 (imaginary)
 431      89002E02 
 432              	# line 21
 433              		.loc	1 21 0
 434 0818 00000000 		fadd.s	%s45,%s61,%s46
 434      AEBDAD4C 
 435 0820 00000000 		fadd.s	%s44,%s52,%s45
 435      ADB4AC4C 
 436 0828 00000000 		fadd.s	%s43,%s54,%s44
 436      ACB6AB4C 
 437 0830 00000000 		fadd.s	%s42,%s56,%s43
 437      ABB8AA4C 
 438 0838 00000000 		or	%s0,0,%s47
 438      AF000045 
 439 0840 00000000 		or	%s1,0,%s42
 439      AA000145 
 440              	# Start of epilogue codes
 441 0848 00000000 		or	%sp,0,%fp
 441      89000B45 
 442              		.cfi_def_cfa	11,8
 443 0850 18000000 		ld	%got,0x18(,%sp)
 443      8B000F01 
 444 0858 20000000 		ld	%plt,0x20(,%sp)
 444      8B001001 
 445 0860 08000000 		ld	%lr,0x8(,%sp)
 445      8B000A01 
 446 0868 00000000 		ld	%fp,0x0(,%sp)
 446      8B000901 
 447 0870 00000000 		b.l	(,%lr)
 447      8A000F19 
 448              	.L_4.EoE:
 449              		.cfi_endproc
 450              		.set	.L.4.2auto_size,	0x0	# 0 Bytes
 452              	# ============ End  sum_fq_fn_8fq ============
 453              	# ============ Begin  prt_fq_fn_8fq ============
 454              		.section .rodata
 455              		.balign 16
 457              	.LP.__string.1:
 458 0000 2F       		.byte	47
 459 0001 75       		.byte	117
 460 0002 73       		.byte	115
 461 0003 72       		.byte	114
 462 0004 2F       		.byte	47
 463 0005 75       		.byte	117
 464 0006 68       		.byte	104
 465 0007 6F       		.byte	111
 466 0008 6D       		.byte	109
 467 0009 65       		.byte	101
 468 000a 2F       		.byte	47
 469 000b 61       		.byte	97
 470 000c 75       		.byte	117
 471 000d 72       		.byte	114
 472 000e 6F       		.byte	111
 473 000f 72       		.byte	114
 474 0010 61       		.byte	97
 475 0011 2F       		.byte	47
 476 0012 34       		.byte	52
 477 0013 67       		.byte	103
 478 0014 69       		.byte	105
 479 0015 2F       		.byte	47
 480 0016 6E       		.byte	110
 481 0017 6C       		.byte	108
 482 0018 61       		.byte	97
 483 0019 62       		.byte	98
 484 001a 68       		.byte	104
 485 001b 70       		.byte	112
 486 001c 67       		.byte	103
 487 001d 2F       		.byte	47
 488 001e 6B       		.byte	107
 489 001f 72       		.byte	114
 490 0020 75       		.byte	117
 491 0021 75       		.byte	117
 492 0022 73       		.byte	115
 493 0023 2F       		.byte	47
 494 0024 76       		.byte	118
 495 0025 74       		.byte	116
 496 0026 2F       		.byte	47
 497 0027 73       		.byte	115
 498 0028 72       		.byte	114
 499 0029 63       		.byte	99
 500 002a 2F       		.byte	47
 501 002b 61       		.byte	97
 502 002c 73       		.byte	115
 503 002d 6D       		.byte	109
 504 002e 2D       		.byte	45
 505 002f 65       		.byte	101
 506 0030 78       		.byte	120
 507 0031 61       		.byte	97
 508 0032 6D       		.byte	109
 509 0033 70       		.byte	112
 510 0034 6C       		.byte	108
 511 0035 65       		.byte	101
 512 0036 73       		.byte	115
 513 0037 2F       		.byte	47
 514 0038 66       		.byte	102
 515 0039 71       		.byte	113
 516 003a 5F       		.byte	95
 517 003b 66       		.byte	102
 518 003c 6E       		.byte	110
 519 003d 5F       		.byte	95
 520 003e 38       		.byte	56
 521 003f 66       		.byte	102
 522 0040 71       		.byte	113
 523 0041 2E       		.byte	46
 524 0042 63       		.byte	99
 525 0043 00       		.zero	1
 526 0044 00000000 		.balign 8
 528              	.LP.__string.0:
 529 0048 25       		.byte	37
 530 0049 73       		.byte	115
 531 004a 3A       		.byte	58
 532 004b 25       		.byte	37
 533 004c 75       		.byte	117
 534 004d 20       		.byte	32
 535 004e 25       		.byte	37
 536 004f 66       		.byte	102
 537 0050 2B       		.byte	43
 538 0051 25       		.byte	37
 539 0052 66       		.byte	102
 540 0053 2A       		.byte	42
 541 0054 49       		.byte	73
 542 0055 0A       		.byte	10
 543 0056 00       		.zero	1
 544              		.text
 545 0878 00000000 		.balign 16
 545      00000000 
 546              	.L_5.0:
 547              	# line 23
  22:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/fq_fn_8fq.c **** }
  23:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/fq_fn_8fq.c **** float complex prt_fq_fn_8fq(float complex a, float complex b, float complex c, float complex d, flo
 548              		.loc	1 23 0
 549              		.globl	prt_fq_fn_8fq
 551              	prt_fq_fn_8fq:
 552              		.cfi_startproc
 553 0880 00000000 		st	%fp,0x0(,%sp)
 553      8B000911 
 554              		.cfi_def_cfa_offset	0
 555              		.cfi_offset	9,0
 556 0888 08000000 		st	%lr,0x8(,%sp)
 556      8B000A11 
 557 0890 18000000 		st	%got,0x18(,%sp)
 557      8B000F11 
 558 0898 20000000 		st	%plt,0x20(,%sp)
 558      8B001011 
 559 08a0 00000000 		lea	%got,_GLOBAL_OFFSET_TABLE_@PC_LO(-0x18)
 559      00680F06 
 560 08a8 00000000 		and	%got,%got,(32)0
 560      608F0F44 
 561 08b0 00000000 		sic	%plt
 561      00001028 
 562 08b8 00000000 		lea.sl	%got,_GLOBAL_OFFSET_TABLE_@PC_HI(%plt,%got)
 562      8F908F06 
 563 08c0 00000000 		or	%fp,0,%sp
 563      8B000945 
 564              		.cfi_def_cfa_register	9
 565 08c8 30000000 		st	%s18,48(,%fp)
 565      89001211 
 566 08d0 38000000 		st	%s19,56(,%fp)
 566      89001311 
 567 08d8 40000000 		st	%s20,64(,%fp)
 567      89001411 
 568 08e0 48000000 		st	%s21,72(,%fp)
 568      89001511 
 569 08e8 50000000 		st	%s22,80(,%fp)
 569      89001611 
 570 08f0 58000000 		st	%s23,88(,%fp)
 570      89001711 
 571 08f8 60000000 		st	%s24,96(,%fp)
 571      89001811 
 572 0900 68000000 		st	%s25,104(,%fp)
 572      89001911 
 573 0908 70000000 		st	%s26,112(,%fp)
 573      89001A11 
 574 0910 78000000 		st	%s27,120(,%fp)
 574      89001B11 
 575 0918 80000000 		st	%s28,128(,%fp)
 575      89001C11 
 576 0920 88000000 		st	%s29,136(,%fp)
 576      89001D11 
 577 0928 90000000 		st	%s30,144(,%fp)
 577      89001E11 
 578 0930 98000000 		st	%s31,152(,%fp)
 578      89001F11 
 579 0938 A0000000 		st	%s32,160(,%fp)
 579      89002011 
 580 0940 A8000000 		st	%s33,168(,%fp)
 580      89002111 
 581 0948 D0FDFFFF 		lea	%s13,.L.5.2auto_size&0xffffffff
 581      00000D06 
 582 0950 00000000 		and	%s13,%s13,(32)0
 582      608D0D44 
 583 0958 FFFFFFFF 		lea.sl	%sp,.L.5.2auto_size>>32(%fp,%s13)
 583      8D898B06 
 584 0960 48000000 		brge.l.t	%sp,%sl,.L_5.EoP
 584      888B3518 
 585 0968 18000000 		ld	%s61,0x18(,%tp)
 585      8E003D01 
 586 0970 00000000 		or	%s62,0,%s0
 586      80003E45 
 587 0978 3B010000 		lea	%s63,0x13b
 587      00003F06 
 588 0980 00000000 		shm.l	%s63,0x0(%s61)
 588      BD033F31 
 589 0988 08000000 		shm.l	%sl,0x8(%s61)
 589      BD030831 
 590 0990 10000000 		shm.l	%sp,0x10(%s61)
 590      BD030B31 
 591 0998 00000000 		monc
 591      0000003F 
 592 09a0 00000000 		or	%s0,0,%s62
 592      BE000045 
 593              	.L_5.EoP:
 594              	# End of prologue codes
 595              	# line 24
  24:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/fq_fn_8fq.c **** 	float complex ret = a+b+c+d+e+f+g+h;
 596              		.loc	1 24 0
 597 09a8 00000000 		fadd.s	%s63,%s1,%s3
 597      8381BF4C 
 598 09b0 00000000 		fadd.s	%s62,%s5,%s63
 598      BF85BE4C 
 599 09b8 00000000 		fadd.s	%s61,%s7,%s62
 599      BE87BD4C 
 600 09c0 00000000 		fadd.s	%s60,%s0,%s2
 600      8280BC4C 
 601 09c8 00000000 		fadd.s	%s59,%s4,%s60
 601      BC84BB4C 
 602 09d0 00000000 		fadd.s	%s58,%s6,%s59
 602      BB86BA4C 
 603              	# line 23
  23:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/fq_fn_8fq.c **** 	float complex ret = a+b+c+d+e+f+g+h;
 604              		.loc	1 23 0
 605 09d8 24010000 		ldu	%s57,292(0,%fp)	# prt_fq_fn_8fq.__unnamed.7 (real)
 605      89003902 
 606 09e0 2C010000 		ldu	%s56,300(0,%fp)	# prt_fq_fn_8fq.__unnamed.7 (imaginary)
 606      89003802 
 607 09e8 14010000 		ldu	%s55,276(0,%fp)	# prt_fq_fn_8fq.__unnamed.6 (real)
 607      89003702 
 608 09f0 1C010000 		ldu	%s54,284(0,%fp)	# prt_fq_fn_8fq.__unnamed.6 (imaginary)
 608      89003602 
 609 09f8 04010000 		ldu	%s53,260(0,%fp)	# prt_fq_fn_8fq.__unnamed.5 (real)
 609      89003502 
 610 0a00 0C010000 		ldu	%s52,268(0,%fp)	# prt_fq_fn_8fq.__unnamed.5 (imaginary)
 610      89003402 
 611 0a08 F4000000 		ldu	%s51,244(0,%fp)	# prt_fq_fn_8fq.__unnamed.4 (real)
 611      89003302 
 612              	# line 24
 613              		.loc	1 24 0
 614 0a10 00000000 		fadd.s	%s50,%s58,%s51
 614      B3BAB24C 
 615 0a18 00000000 		fadd.s	%s49,%s53,%s50
 615      B2B5B14C 
 616 0a20 00000000 		fadd.s	%s48,%s55,%s49
 616      B1B7B04C 
 617 0a28 00000000 		fadd.s	%s47,%s57,%s48
 617      B0B9AF4C 
 618              	# line 23
  23:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/fq_fn_8fq.c **** 	float complex ret = a+b+c+d+e+f+g+h;
 619              		.loc	1 23 0
 620 0a30 FC000000 		ldu	%s46,252(0,%fp)	# prt_fq_fn_8fq.__unnamed.4 (imaginary)
 620      89002E02 
 621              	# line 24
 622              		.loc	1 24 0
 623 0a38 00000000 		fadd.s	%s45,%s61,%s46
 623      AEBDAD4C 
 624 0a40 00000000 		fadd.s	%s44,%s52,%s45
 624      ADB4AC4C 
 625 0a48 00000000 		fadd.s	%s43,%s54,%s44
 625      ACB6AB4C 
 626 0a50 00000000 		fadd.s	%s42,%s56,%s43
 626      ABB8AA4C 
 627              	# line 25
  25:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/fq_fn_8fq.c **** 	printf("%s:%u %f+%f*I\n",__FILE__,__LINE__,crealf(ret),cimagf(ret));
 628              		.loc	1 25 0
 629 0a58 00000000 		lea	%s41,.LP.__string.0@GOTOFF_LO
 629      00002906 
 630 0a60 00000000 		and	%s41,%s41,(32)0
 630      60A92944 
 631 0a68 00000000 		lea.sl	%s41,.LP.__string.0@GOTOFF_HI(%s41,%got)
 631      8FA9A906 
 632 0a70 00000000 		lea	%s40,.LP.__string.1@GOTOFF_LO
 632      00002806 
 633 0a78 00000000 		and	%s40,%s40,(32)0
 633      60A82844 
 634 0a80 00000000 		lea.sl	%s40,.LP.__string.1@GOTOFF_HI(%s40,%got)
 634      8FA8A806 
 635 0a88 00000000 		or	%s0,0,%s47
 635      AF000045 
 636 0a90 00000000 		or	%s1,0,%s42
 636      AA000145 
 637 0a98 F8FFFFFF 		st	%s1,-8(,%fp)	# spill
 637      89000111 
 638 0aa0 F0FFFFFF 		st	%s0,-16(,%fp)	# spill
 638      89000011 
 639 0aa8 E8FFFFFF 		st	%s41,-24(,%fp)	# spill
 639      89002911 
 640 0ab0 E0FFFFFF 		st	%s40,-32(,%fp)	# spill
 640      89002811 
 641 0ab8 00000000 		lea	%s12,crealf@PLT_LO(-24)
 641      00680C06 
 642 0ac0 00000000 		and	%s12,%s12,(32)0
 642      608C0C44 
 643 0ac8 00000000 		sic	%lr
 643      00000A28 
 644 0ad0 00000000 		lea.sl	%s12,crealf@PLT_HI(%s12,%lr)
 644      8A8C8C06 
 645 0ad8 00000000 		bsic	%lr,(,%s12)		# crealf
 645      8C000A08 
 646              	.L_5.3:
 647 0ae0 00000000 		cvt.d.s %s63,%s0
 647      00803F0F 
 648 0ae8 F0FFFFFF 		ld	%s0,-16(,%fp)	# restore
 648      89000001 
 649 0af0 F8FFFFFF 		ld	%s1,-8(,%fp)	# restore
 649      89000101 
 650 0af8 D8FFFFFF 		st	%s63,-40(,%fp)	# spill
 650      89003F11 
 651 0b00 F8FFFFFF 		st	%s1,-8(,%fp)	# spill
 651      89000111 
 652 0b08 F0FFFFFF 		st	%s0,-16(,%fp)	# spill
 652      89000011 
 653 0b10 00000000 		lea	%s12,cimagf@PLT_LO(-24)
 653      00680C06 
 654 0b18 00000000 		and	%s12,%s12,(32)0
 654      608C0C44 
 655 0b20 00000000 		sic	%lr
 655      00000A28 
 656 0b28 00000000 		lea.sl	%s12,cimagf@PLT_HI(%s12,%lr)
 656      8A8C8C06 
 657 0b30 00000000 		bsic	%lr,(,%s12)		# cimagf
 657      8C000A08 
 658              	.L_5.2:
 659 0b38 00000000 		cvt.d.s %s63,%s0
 659      00803F0F 
 660 0b40 D0000000 		st	%s63,208(0,%sp)
 660      8B003F11 
 661 0b48 E8FFFFFF 		ld	%s62,-24(,%fp)	# restore
 661      89003E01 
 662 0b50 B0000000 		st	%s62,176(0,%sp)
 662      8B003E11 
 663 0b58 E0FFFFFF 		ld	%s61,-32(,%fp)	# restore
 663      89003D01 
 664 0b60 B8000000 		st	%s61,184(0,%sp)
 664      8B003D11 
 665 0b68 00000000 		or	%s60,25,(0)1
 665      00193C45 
 666 0b70 C0000000 		st	%s60,192(0,%sp)
 666      8B003C11 
 667 0b78 D8FFFFFF 		ld	%s59,-40(,%fp)	# restore
 667      89003B01 
 668 0b80 C8000000 		st	%s59,200(0,%sp)
 668      8B003B11 
 669 0b88 00000000 		or	%s0,0,%s62
 669      BE000045 
 670 0b90 00000000 		or	%s1,0,%s61
 670      BD000145 
 671 0b98 00000000 		or	%s2,0,%s60
 671      BC000245 
 672 0ba0 00000000 		or	%s3,0,%s59
 672      BB000345 
 673 0ba8 00000000 		or	%s4,0,%s63
 673      BF000445 
 674 0bb0 00000000 		lea	%s12,printf@PLT_LO(-24)
 674      00680C06 
 675 0bb8 00000000 		and	%s12,%s12,(32)0
 675      608C0C44 
 676 0bc0 00000000 		sic	%lr
 676      00000A28 
 677 0bc8 00000000 		lea.sl	%s12,printf@PLT_HI(%s12,%lr)
 677      8A8C8C06 
 678 0bd0 00000000 		bsic	%lr,(,%s12)		# printf
 678      8C000A08 
 679 0bd8 08000000 		br.l	.L_5.1
 679      00000F18 
 680              	.L_5.1:
 681              	# line 26
  26:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/fq_fn_8fq.c **** 	return ret;
 682              		.loc	1 26 0
 683 0be0 F0FFFFFF 		ld	%s0,-16(,%fp)	# restore
 683      89000001 
 684 0be8 F8FFFFFF 		ld	%s1,-8(,%fp)	# restore
 684      89000101 
 685              	# Start of epilogue codes
 686 0bf0 00000000 		or	%sp,0,%fp
 686      89000B45 
 687              		.cfi_def_cfa	11,8
 688 0bf8 18000000 		ld	%got,0x18(,%sp)
 688      8B000F01 
 689 0c00 20000000 		ld	%plt,0x20(,%sp)
 689      8B001001 
 690 0c08 08000000 		ld	%lr,0x8(,%sp)
 690      8B000A01 
 691 0c10 00000000 		ld	%fp,0x0(,%sp)
 691      8B000901 
 692 0c18 00000000 		b.l	(,%lr)
 692      8A000F19 
 693              	.L_5.EoE:
 694              		.cfi_endproc
 695              		.set	.L.5.2auto_size,	0xfffffffffffffdd0	# 560 Bytes
 697              	# ============ End  prt_fq_fn_8fq ============
 698              	.Le1.0:
