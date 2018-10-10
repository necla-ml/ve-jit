   1              		.ident "ncc 1.1.4 (Build 10:17:22 Apr 19 2018)"
   2              		.file "Simple-EASM-Example.c"
   3              		.file 1 "/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/Simple-EASM-Example.c"
   4              		.file 2 "/opt/nec/ve/ncc/1.1.4/include/stdc-predef.h"
   5              	# ============ Begin  vcopy ============
   6              		.text
   7              		.balign 16
   8              	.L_1.0:
   9              	# line 3
   1:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/Simple-EASM-Example.c **** // vim: et ts=4 sw=4 cindent cino=^=l0,\:0,N-s
   2:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/Simple-EASM-Example.c **** void vcopy(double *dst, double *src, int len)
   3:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/Simple-EASM-Example.c **** {
  10              		.loc	1 3 0
  11              		.globl	vcopy
  13              	vcopy:
  14              		.cfi_startproc
  15 0000 00000000 		st	%fp,0x0(,%sp)
  15      8B000911 
  16              		.cfi_def_cfa_offset	0
  17              		.cfi_offset	9,0
  18 0008 08000000 		st	%lr,0x8(,%sp)
  18      8B000A11 
  19 0010 18000000 		st	%got,0x18(,%sp)
  19      8B000F11 
  20 0018 20000000 		st	%plt,0x20(,%sp)
  20      8B001011 
  21 0020 00000000 		lea	%got,_GLOBAL_OFFSET_TABLE_@PC_LO(-0x18)
  21      00680F06 
  22 0028 00000000 		and	%got,%got,(32)0
  22      608F0F44 
  23 0030 00000000 		sic	%plt
  23      00001028 
  24 0038 00000000 		lea.sl	%got,_GLOBAL_OFFSET_TABLE_@PC_HI(%plt,%got)
  24      8F908F06 
  25 0040 00000000 		or	%fp,0,%sp
  25      8B000945 
  26              		.cfi_def_cfa_register	9
  27 0048 30000000 		st	%s18,48(,%fp)
  27      89001211 
  28 0050 38000000 		st	%s19,56(,%fp)
  28      89001311 
  29 0058 40000000 		st	%s20,64(,%fp)
  29      89001411 
  30 0060 48000000 		st	%s21,72(,%fp)
  30      89001511 
  31 0068 50000000 		st	%s22,80(,%fp)
  31      89001611 
  32 0070 58000000 		st	%s23,88(,%fp)
  32      89001711 
  33 0078 60000000 		st	%s24,96(,%fp)
  33      89001811 
  34 0080 68000000 		st	%s25,104(,%fp)
  34      89001911 
  35 0088 70000000 		st	%s26,112(,%fp)
  35      89001A11 
  36 0090 78000000 		st	%s27,120(,%fp)
  36      89001B11 
  37 0098 80000000 		st	%s28,128(,%fp)
  37      89001C11 
  38 00a0 88000000 		st	%s29,136(,%fp)
  38      89001D11 
  39 00a8 90000000 		st	%s30,144(,%fp)
  39      89001E11 
  40 00b0 98000000 		st	%s31,152(,%fp)
  40      89001F11 
  41 00b8 A0000000 		st	%s32,160(,%fp)
  41      89002011 
  42 00c0 A8000000 		st	%s33,168(,%fp)
  42      89002111 
  43 00c8 C0FEFFFF 		lea	%s13,.L.1.2auto_size&0xffffffff
  43      00000D06 
  44 00d0 00000000 		and	%s13,%s13,(32)0
  44      608D0D44 
  45 00d8 FFFFFFFF 		lea.sl	%sp,.L.1.2auto_size>>32(%fp,%s13)
  45      8D898B06 
  46 00e0 48000000 		brge.l.t	%sp,%sl,.L_1.EoP
  46      888B3518 
  47 00e8 18000000 		ld	%s61,0x18(,%tp)
  47      8E003D01 
  48 00f0 00000000 		or	%s62,0,%s0
  48      80003E45 
  49 00f8 3B010000 		lea	%s63,0x13b
  49      00003F06 
  50 0100 00000000 		shm.l	%s63,0x0(%s61)
  50      BD033F31 
  51 0108 08000000 		shm.l	%sl,0x8(%s61)
  51      BD030831 
  52 0110 10000000 		shm.l	%sp,0x10(%s61)
  52      BD030B31 
  53 0118 00000000 		monc
  53      0000003F 
  54 0120 00000000 		or	%s0,0,%s62
  54      BE000045 
  55              	.L_1.EoP:
  56              	# End of prologue codes
  57 0128 00000000 		or	%s63,%s2,(0)1
  57      00823F45 
  58              	# line 4
   4:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/Simple-EASM-Example.c **** 	double *p = src, *q = dst;
  59              		.loc	1 4 0
  60 0130 00000000 		or	%s62,%s1,(0)1
  60      00813E45 
  61 0138 00000000 		or	%s61,%s0,(0)1
  61      00803D45 
  62              	# line 6
   5:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/Simple-EASM-Example.c **** 
   6:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/Simple-EASM-Example.c **** 	for (; len > 0; len -= 256, p += 256, q += 256) {
  63              		.loc	1 6 0
  64 0140 F8FFFFFF 		st	%s63,-8(,%fp)	# spill
  64      89003F11 
  65 0148 F0FFFFFF 		st	%s61,-16(,%fp)	# spill
  65      89003D11 
  66 0150 E8FFFFFF 		st	%s62,-24(,%fp)	# spill
  66      89003E11 
  67 0158 10000000 		brlt.w	0,%s2,.L_1.7
  67      82008218 
  68 0160 48010000 		br.l	.L_1.3
  68      00000F18 
  69              	.L_1.7:
  70 0168 F8FFFFFF 		st	%s63,-8(,%fp)	# spill
  70      89003F11 
  71 0170 F0FFFFFF 		st	%s61,-16(,%fp)	# spill
  71      89003D11 
  72 0178 E8FFFFFF 		st	%s62,-24(,%fp)	# spill
  72      89003E11 
  73 0180 08000000 		br.l	.L_1.2
  73      00000F18 
  74              	.L_1.2:
  75              	# line 7
   7:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/Simple-EASM-Example.c **** 		if (len < 256)
  76              		.loc	1 7 0
  77 0188 00010000 		lea	%s63,256
  77      00003F06 
  78 0190 F8FFFFFF 		ld	%s62,-8(,%fp)	# restore
  78      89003E01 
  79 0198 F0000000 		brlt.w	%s62,%s63,.L_1.5
  79      BFBE8218 
  80 01a0 08000000 		br.l	.L_1.6
  80      00000F18 
  81              	.L_1.6:
  82              	# line 10
   8:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/Simple-EASM-Example.c **** 			__asm__ ("lvl	%0" : : "r" (len) );
   9:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/Simple-EASM-Example.c **** 		else
  10:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/Simple-EASM-Example.c **** 			__asm__ ("lea	%%s63,256\n"
  83              		.loc	1 10 0
  84 01a8 60FFFFFF 		st	%s63,-160(,%fp)	# spill
  84      89003F11 
  85              	#APP
  86 01b0 00010000 		lea	%s63,256
  86      00003F06 
  87 01b8 00000000 	lvl	%s63
  87      00BF00BF 
  88              	#NO_APP
  89 01c0 60FFFFFF 		ld	%s63,-160(,%fp)	# restore
  89      89003F01 
  90 01c8 F8FFFFFF 		st	%s62,-8(,%fp)	# spill
  90      89003E11 
  91 01d0 08000000 		br.l	.L_1.4
  91      00000F18 
  92              	.L_1.4:
  93              	# line 13
  11:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/Simple-EASM-Example.c **** 					"lvl	%%s63" : : : "%s63" );
  12:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/Simple-EASM-Example.c **** 
  13:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/Simple-EASM-Example.c **** 		__asm__ ("vld	%%v0,8,%0\n"
  94              		.loc	1 13 0
  95 01d8 E8FFFFFF 		ld	%s63,-24(,%fp)	# restore
  95      89003F01 
  96 01e0 00000000 		or	%s62,%s63,(0)1
  96      00BF3E45 
  97 01e8 F0FFFFFF 		ld	%s61,-16(,%fp)	# restore
  97      89003D01 
  98 01f0 00000000 		or	%s60,%s61,(0)1
  98      00BD3C45 
  99              	#APP
 100 01f8 00000000 		vld	%v0,8,%s62
 100      BE084081 
 101 0200 00000000 	vst	%v0,8,%s60
 101      BC084091 
 102              	#NO_APP
 103              	# line 6
   6:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/Simple-EASM-Example.c **** 		if (len < 256)
 104              		.loc	1 6 0
 105 0208 00080000 		lea	%s59,2048
 105      00003B06 
 106 0210 00000000 		adds.l	%s58,%s63,%s59
 106      BBBF3A59 
 107 0218 00000000 		or	%s63,%s58,(0)1
 107      00BA3F45 
 108 0220 00080000 		lea	%s57,2048
 108      00003906 
 109 0228 00000000 		adds.l	%s56,%s61,%s57
 109      B9BD3859 
 110 0230 00000000 		or	%s61,%s56,(0)1
 110      00B83D45 
 111 0238 00010000 		lea	%s55,256
 111      00003706 
 112 0240 F8FFFFFF 		ld	%s54,-8(,%fp)	# restore
 112      89003601 
 113 0248 00000000 		subs.w.sx	%s54,%s54,%s55
 113      B7B6365A 
 114 0250 10000000 		brlt.w	0,%s54,.L_1.8
 114      B6008218 
 115 0258 28000000 		br.l	.L_1.1
 115      00000F18 
 116              	.L_1.8:
 117 0260 F8FFFFFF 		st	%s54,-8(,%fp)	# spill
 117      89003611 
 118 0268 F0FFFFFF 		st	%s61,-16(,%fp)	# spill
 118      89003D11 
 119 0270 E8FFFFFF 		st	%s63,-24(,%fp)	# spill
 119      89003F11 
 120 0278 10FFFFFF 		br.l	.L_1.2
 120      00000F18 
 121              	.L_1.1:
 122 0280 28000000 		br.l	.L_1.3
 122      00000F18 
 123              	.L_1.5:
 124              	# line 8
   8:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/Simple-EASM-Example.c **** 			__asm__ ("lvl	%0" : : "r" (len) );
 125              		.loc	1 8 0
 126 0288 00000000 		or	%s63,%s62,(0)1
 126      00BE3F45 
 127              	#APP
 128 0290 00000000 		lvl	%s63
 128      00BF00BF 
 129              	#NO_APP
 130 0298 F8FFFFFF 		st	%s62,-8(,%fp)	# spill
 130      89003E11 
 131 02a0 38FFFFFF 		br.l	.L_1.4
 131      00000F18 
 132              	.L_1.3:
 133              	# line 16
  14:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/Simple-EASM-Example.c **** 				"vst	%%v0,8,%1" : : "r" (p), "r" (q) : "%v0" );
  15:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/Simple-EASM-Example.c **** 	}
  16:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/Simple-EASM-Example.c **** }
 134              		.loc	1 16 0
 135              	# Start of epilogue codes
 136 02a8 00000000 		or	%sp,0,%fp
 136      89000B45 
 137              		.cfi_def_cfa	11,8
 138 02b0 18000000 		ld	%got,0x18(,%sp)
 138      8B000F01 
 139 02b8 20000000 		ld	%plt,0x20(,%sp)
 139      8B001001 
 140 02c0 08000000 		ld	%lr,0x8(,%sp)
 140      8B000A01 
 141 02c8 00000000 		ld	%fp,0x0(,%sp)
 141      8B000901 
 142 02d0 00000000 		b.l	(,%lr)
 142      8A000F19 
 143              	.L_1.EoE:
 144              		.cfi_endproc
 145              		.set	.L.1.2auto_size,	0xfffffffffffffec0	# 320 Bytes
 147              	# ============ End  vcopy ============
 148              	.Le1.0:
