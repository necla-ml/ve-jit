   1              		.ident "ncc 1.1.4 (Build 10:17:22 Apr 19 2018)"
   2              		.file "ldq_fn_ldq4.c"
   3              		.file 1 "/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ldq_fn_ldq4.c"
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
  14              	# ============ Begin  ldq_fn_ldq4 ============
  15              		.section	.bss
  16              		.local	ldq_ret
  17              	.comm ldq_ret,32,16
  18              		.text
  19              		.balign 16
  20              	.L_1.0:
  21              	# line 7
   1:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ldq_fn_ldq4.c **** #include <stdio.h>
   2:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ldq_fn_ldq4.c **** #include <complex.h>
   3:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ldq_fn_ldq4.c **** 
   4:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ldq_fn_ldq4.c **** extern long double complex ext_ldq_fn_ldq4(long double complex ldq, long double complex ldq2, long 
   5:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ldq_fn_ldq4.c **** 
   6:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ldq_fn_ldq4.c **** static long double complex ldq_ret;
   7:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ldq_fn_ldq4.c **** long double complex ldq_fn_ldq4(long double complex ldq, long double complex ldq2, long double comp
  22              		.loc	1 7 0
  23              		.globl	ldq_fn_ldq4
  25              	ldq_fn_ldq4:
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
  55 00c8 C0FDFFFF 		lea	%s13,.L.1.2auto_size&0xffffffff
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
  69 0128 00000000 		or	%s34,0,%s6
  69      86002245 
  70 0130 00000000 		or	%s35,0,%s7
  70      87002345 
  71 0138 00000000 		or	%s36,0,%s4
  71      84002445 
  72 0140 00000000 		or	%s37,0,%s5
  72      85002545 
  73 0148 00000000 		or	%s38,0,%s2
  73      82002645 
  74 0150 00000000 		or	%s39,0,%s3
  74      83002745 
  75 0158 00000000 		or	%s40,0,%s0
  75      80002845 
  76 0160 00000000 		or	%s41,0,%s1
  76      81002945 
  77 0168 B0000000 		st	%s41,176(0,%fp)	# ldq (real)
  77      89002911 
  78 0170 B0000000 		st	%s40,176(8,%fp)	# ldq (real)
  78      89082811 
  79 0178 C0000000 		st	%s39,192(0,%fp)	# ldq (imaginary)
  79      89002711 
  80 0180 C0000000 		st	%s38,192(8,%fp)	# ldq (imaginary)
  80      89082611 
  81 0188 D0000000 		st	%s37,208(0,%fp)	# ldq2 (real)
  81      89002511 
  82 0190 D0000000 		st	%s36,208(8,%fp)	# ldq2 (real)
  82      89082411 
  83 0198 E0000000 		st	%s35,224(0,%fp)	# ldq2 (imaginary)
  83      89002311 
  84 01a0 E0000000 		st	%s34,224(8,%fp)	# ldq2 (imaginary)
  84      89082211 
  85              	# line 8
   8:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ldq_fn_ldq4.c **** 	ldq_ret = ext_ldq_fn_ldq(ldq,ldq2,ldq3,ldq4);
  86              		.loc	1 8 0
  87 01a8 F0000000 		ld	%s43,240(0,%fp)	# ldq3 (real)
  87      89002B01 
  88 01b0 F0000000 		ld	%s42,240(8,%fp)	# ldq3 (real)
  88      89082A01 
  89 01b8 00010000 		ld	%s45,256(0,%fp)	# ldq3 (imaginary)
  89      89002D01 
  90 01c0 00010000 		ld	%s44,256(8,%fp)	# ldq3 (imaginary)
  90      89082C01 
  91 01c8 10010000 		ld	%s47,272(0,%fp)	# ldq4 (real)
  91      89002F01 
  92 01d0 10010000 		ld	%s46,272(8,%fp)	# ldq4 (real)
  92      89082E01 
  93 01d8 20010000 		ld	%s49,288(0,%fp)	# ldq4 (imaginary)
  93      89003101 
  94 01e0 20010000 		ld	%s48,288(8,%fp)	# ldq4 (imaginary)
  94      89083001 
  95 01e8 00000000 		or	%s63,0,%s40
  95      A8003F45 
  96 01f0 B8000000 		st	%s63,184(0,%sp)
  96      8B003F11 
  97 01f8 00000000 		or	%s62,0,%s41
  97      A9003E45 
  98 0200 B0000000 		st	%s62,176(0,%sp)
  98      8B003E11 
  99 0208 00000000 		or	%s61,0,%s38
  99      A6003D45 
 100 0210 C8000000 		st	%s61,200(0,%sp)
 100      8B003D11 
 101 0218 00000000 		or	%s60,0,%s39
 101      A7003C45 
 102 0220 C0000000 		st	%s60,192(0,%sp)
 102      8B003C11 
 103 0228 00000000 		or	%s59,0,%s36
 103      A4003B45 
 104 0230 D8000000 		st	%s59,216(0,%sp)
 104      8B003B11 
 105 0238 00000000 		or	%s58,0,%s37
 105      A5003A45 
 106 0240 D0000000 		st	%s58,208(0,%sp)
 106      8B003A11 
 107 0248 00000000 		or	%s57,0,%s34
 107      A2003945 
 108 0250 E8000000 		st	%s57,232(0,%sp)
 108      8B003911 
 109 0258 00000000 		or	%s56,0,%s35
 109      A3003845 
 110 0260 E0000000 		st	%s56,224(0,%sp)
 110      8B003811 
 111 0268 00000000 		or	%s55,0,%s42
 111      AA003745 
 112 0270 F8000000 		st	%s55,248(0,%sp)
 112      8B003711 
 113 0278 00000000 		or	%s54,0,%s43
 113      AB003645 
 114 0280 F0000000 		st	%s54,240(0,%sp)
 114      8B003611 
 115 0288 00000000 		or	%s53,0,%s44
 115      AC003545 
 116 0290 08010000 		st	%s53,264(0,%sp)
 116      8B003511 
 117 0298 00000000 		or	%s52,0,%s45
 117      AD003445 
 118 02a0 00010000 		st	%s52,256(0,%sp)
 118      8B003411 
 119 02a8 00000000 		or	%s51,0,%s46
 119      AE003345 
 120 02b0 18010000 		st	%s51,280(0,%sp)
 120      8B003311 
 121 02b8 00000000 		or	%s50,0,%s47
 121      AF003245 
 122 02c0 10010000 		st	%s50,272(0,%sp)
 122      8B003211 
 123 02c8 00000000 		or	%s18,0,%s48
 123      B0001245 
 124 02d0 28010000 		st	%s18,296(0,%sp)
 124      8B001211 
 125 02d8 00000000 		or	%s19,0,%s49
 125      B1001345 
 126 02e0 20010000 		st	%s19,288(0,%sp)
 126      8B001311 
 127 02e8 00000000 		or	%s0,0,%s63
 127      BF000045 
 128 02f0 00000000 		or	%s1,0,%s62
 128      BE000145 
 129 02f8 00000000 		or	%s2,0,%s61
 129      BD000245 
 130 0300 00000000 		or	%s3,0,%s60
 130      BC000345 
 131 0308 00000000 		or	%s4,0,%s59
 131      BB000445 
 132 0310 00000000 		or	%s5,0,%s58
 132      BA000545 
 133 0318 00000000 		or	%s6,0,%s57
 133      B9000645 
 134 0320 00000000 		or	%s7,0,%s56
 134      B8000745 
 135 0328 00000000 		lea	%s12,ext_ldq_fn_ldq@PLT_LO(-24)
 135      00680C06 
 136 0330 00000000 		and	%s12,%s12,(32)0
 136      608C0C44 
 137 0338 00000000 		sic	%lr
 137      00000A28 
 138 0340 00000000 		lea.sl	%s12,ext_ldq_fn_ldq@PLT_HI(%s12,%lr)
 138      8A8C8C06 
 139 0348 00000000 		bsic	%lr,(,%s12)		# ext_ldq_fn_ldq
 139      8C000A08 
 140 0350 F8FFFFFF 		st	%s0,-8(,%fp)	# spill
 140      89000011 
 141 0358 08000000 		br.l	.L_1.1
 141      00000F18 
 142              	.L_1.1:
 143 0360 F8FFFFFF 		ld	%s63,-8(,%fp)	# restore
 143      89003F01 
 144 0368 00000000 		cvt.d.w %s62,%s63
 144      00BF3E5E 
 145 0370 00000000 		cvt.q.d %s18,%s62
 145      00BE122D 
 146 0378 00000000 		lea	%s61,ldq_ret@GOTOFF_LO
 146      00003D06 
 147 0380 00000000 		and	%s61,%s61,(32)0
 147      60BD3D44 
 148 0388 00000000 		lea.sl	%s61,ldq_ret@GOTOFF_HI(%s61,%got)
 148      8FBDBD06 
 149 0390 00000000 		st	%s19,0(0,%s61)	# ldq_ret (real)
 149      BD001311 
 150 0398 00000000 		st	%s18,0(8,%s61)	# ldq_ret (real)
 150      BD081211 
 151 03a0 00000000 		or	%s60,0,(0)1
 151      00003C45 
 152 03a8 10000000 		st	%s60,16(0,%s61)	# ldq_ret (imaginary)
 152      BD003C11 
 153 03b0 00000000 		or	%s59,0,(0)1
 153      00003B45 
 154 03b8 18000000 		st	%s59,24(0,%s61)	# ldq_ret (imaginary)
 154      BD003B11 
 155              	# line 9
   9:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ldq_fn_ldq4.c **** 	return ldq_ret;
 156              		.loc	1 9 0
 157 03c0 00000000 		or	%s58,0,%s18
 157      92003A45 
 158 03c8 00000000 		or	%s57,0,%s19
 158      93003945 
 159 03d0 00000000 		or	%s56,0,(0)1
 159      00003845 
 160 03d8 00000000 		or	%s55,0,(0)1
 160      00003745 
 161 03e0 00000000 		or	%s0,0,%s58
 161      BA000045 
 162 03e8 00000000 		or	%s1,0,%s57
 162      B9000145 
 163 03f0 00000000 		or	%s2,0,%s56
 163      B8000245 
 164 03f8 00000000 		or	%s3,0,%s55
 164      B7000345 
 165              	# Start of epilogue codes
 166 0400 30000000 		ld	%s18,48(,%fp)
 166      89001201 
 167 0408 38000000 		ld	%s19,56(,%fp)
 167      89001301 
 168 0410 00000000 		or	%sp,0,%fp
 168      89000B45 
 169              		.cfi_def_cfa	11,8
 170 0418 18000000 		ld	%got,0x18(,%sp)
 170      8B000F01 
 171 0420 20000000 		ld	%plt,0x20(,%sp)
 171      8B001001 
 172 0428 08000000 		ld	%lr,0x8(,%sp)
 172      8B000A01 
 173 0430 00000000 		ld	%fp,0x0(,%sp)
 173      8B000901 
 174 0438 00000000 		b.l	(,%lr)
 174      8A000F19 
 175              	.L_1.EoE:
 176              		.cfi_endproc
 177              		.set	.L.1.2auto_size,	0xfffffffffffffdc0	# 576 Bytes
 179              	# ============ End  ldq_fn_ldq4 ============
 180              	# ============ Begin  ldq_fn_ldq4b ============
 181              		.balign 16
 182              	.L_2.0:
 183              	# line 11
  10:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ldq_fn_ldq4.c **** }
  11:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ldq_fn_ldq4.c **** long double complex ldq_fn_ldq4b(long double complex ldq, long double complex ldq2, long double com
 184              		.loc	1 11 0
 185              		.globl	ldq_fn_ldq4b
 187              	ldq_fn_ldq4b:
 188              		.cfi_startproc
 189 0440 00000000 		st	%fp,0x0(,%sp)
 189      8B000911 
 190              		.cfi_def_cfa_offset	0
 191              		.cfi_offset	9,0
 192 0448 08000000 		st	%lr,0x8(,%sp)
 192      8B000A11 
 193 0450 18000000 		st	%got,0x18(,%sp)
 193      8B000F11 
 194 0458 20000000 		st	%plt,0x20(,%sp)
 194      8B001011 
 195 0460 00000000 		lea	%got,_GLOBAL_OFFSET_TABLE_@PC_LO(-0x18)
 195      00680F06 
 196 0468 00000000 		and	%got,%got,(32)0
 196      608F0F44 
 197 0470 00000000 		sic	%plt
 197      00001028 
 198 0478 00000000 		lea.sl	%got,_GLOBAL_OFFSET_TABLE_@PC_HI(%plt,%got)
 198      8F908F06 
 199 0480 00000000 		or	%fp,0,%sp
 199      8B000945 
 200              		.cfi_def_cfa_register	9
 201 0488 30000000 		st	%s18,48(,%fp)
 201      89001211 
 202 0490 38000000 		st	%s19,56(,%fp)
 202      89001311 
 203 0498 40000000 		st	%s20,64(,%fp)
 203      89001411 
 204 04a0 48000000 		st	%s21,72(,%fp)
 204      89001511 
 205 04a8 50000000 		st	%s22,80(,%fp)
 205      89001611 
 206 04b0 58000000 		st	%s23,88(,%fp)
 206      89001711 
 207 04b8 60000000 		st	%s24,96(,%fp)
 207      89001811 
 208 04c0 68000000 		st	%s25,104(,%fp)
 208      89001911 
 209 04c8 70000000 		st	%s26,112(,%fp)
 209      89001A11 
 210 04d0 78000000 		st	%s27,120(,%fp)
 210      89001B11 
 211 04d8 80000000 		st	%s28,128(,%fp)
 211      89001C11 
 212 04e0 88000000 		st	%s29,136(,%fp)
 212      89001D11 
 213 04e8 90000000 		st	%s30,144(,%fp)
 213      89001E11 
 214 04f0 98000000 		st	%s31,152(,%fp)
 214      89001F11 
 215 04f8 A0000000 		st	%s32,160(,%fp)
 215      89002011 
 216 0500 A8000000 		st	%s33,168(,%fp)
 216      89002111 
 217 0508 B0FDFFFF 		lea	%s13,.L.2.2auto_size&0xffffffff
 217      00000D06 
 218 0510 00000000 		and	%s13,%s13,(32)0
 218      608D0D44 
 219 0518 FFFFFFFF 		lea.sl	%sp,.L.2.2auto_size>>32(%fp,%s13)
 219      8D898B06 
 220 0520 48000000 		brge.l.t	%sp,%sl,.L_2.EoP
 220      888B3518 
 221 0528 18000000 		ld	%s61,0x18(,%tp)
 221      8E003D01 
 222 0530 00000000 		or	%s62,0,%s0
 222      80003E45 
 223 0538 3B010000 		lea	%s63,0x13b
 223      00003F06 
 224 0540 00000000 		shm.l	%s63,0x0(%s61)
 224      BD033F31 
 225 0548 08000000 		shm.l	%sl,0x8(%s61)
 225      BD030831 
 226 0550 10000000 		shm.l	%sp,0x10(%s61)
 226      BD030B31 
 227 0558 00000000 		monc
 227      0000003F 
 228 0560 00000000 		or	%s0,0,%s62
 228      BE000045 
 229              	.L_2.EoP:
 230              	# End of prologue codes
 231 0568 00000000 		or	%s34,0,%s6
 231      86002245 
 232 0570 00000000 		or	%s35,0,%s7
 232      87002345 
 233 0578 00000000 		or	%s36,0,%s4
 233      84002445 
 234 0580 00000000 		or	%s37,0,%s5
 234      85002545 
 235 0588 00000000 		or	%s38,0,%s2
 235      82002645 
 236 0590 00000000 		or	%s39,0,%s3
 236      83002745 
 237 0598 00000000 		or	%s40,0,%s0
 237      80002845 
 238 05a0 00000000 		or	%s41,0,%s1
 238      81002945 
 239 05a8 B0000000 		st	%s41,176(0,%fp)	# ldq (real)
 239      89002911 
 240 05b0 B0000000 		st	%s40,176(8,%fp)	# ldq (real)
 240      89082811 
 241 05b8 C0000000 		st	%s39,192(0,%fp)	# ldq (imaginary)
 241      89002711 
 242 05c0 C0000000 		st	%s38,192(8,%fp)	# ldq (imaginary)
 242      89082611 
 243 05c8 D0000000 		st	%s37,208(0,%fp)	# ldq2 (real)
 243      89002511 
 244 05d0 D0000000 		st	%s36,208(8,%fp)	# ldq2 (real)
 244      89082411 
 245 05d8 E0000000 		st	%s35,224(0,%fp)	# ldq2 (imaginary)
 245      89002311 
 246 05e0 E0000000 		st	%s34,224(8,%fp)	# ldq2 (imaginary)
 246      89082211 
 247              	# line 12
  12:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ldq_fn_ldq4.c **** 	long double complex *p_ret = &ldq_ret;
 248              		.loc	1 12 0
 249 05e8 00000000 		lea	%s63,ldq_ret@GOTOFF_LO
 249      00003F06 
 250 05f0 00000000 		and	%s63,%s63,(32)0
 250      60BF3F44 
 251 05f8 00000000 		lea.sl	%s63,ldq_ret@GOTOFF_HI(%s63,%got)
 251      8FBFBF06 
 252 0600 00000000 		or	%s62,%s63,(0)1
 252      00BF3E45 
 253              	# line 13
  13:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ldq_fn_ldq4.c **** 	*p_ret = ext_ldq_fn_ldq(ldq,ldq2,ldq3,ldq4);
 254              		.loc	1 13 0
 255 0608 F0000000 		ld	%s43,240(0,%fp)	# ldq3 (real)
 255      89002B01 
 256 0610 F0000000 		ld	%s42,240(8,%fp)	# ldq3 (real)
 256      89082A01 
 257 0618 00010000 		ld	%s45,256(0,%fp)	# ldq3 (imaginary)
 257      89002D01 
 258 0620 00010000 		ld	%s44,256(8,%fp)	# ldq3 (imaginary)
 258      89082C01 
 259 0628 10010000 		ld	%s47,272(0,%fp)	# ldq4 (real)
 259      89002F01 
 260 0630 10010000 		ld	%s46,272(8,%fp)	# ldq4 (real)
 260      89082E01 
 261 0638 20010000 		ld	%s49,288(0,%fp)	# ldq4 (imaginary)
 261      89003101 
 262 0640 20010000 		ld	%s48,288(8,%fp)	# ldq4 (imaginary)
 262      89083001 
 263 0648 00000000 		or	%s61,0,%s40
 263      A8003D45 
 264 0650 B8000000 		st	%s61,184(0,%sp)
 264      8B003D11 
 265 0658 00000000 		or	%s60,0,%s41
 265      A9003C45 
 266 0660 B0000000 		st	%s60,176(0,%sp)
 266      8B003C11 
 267 0668 00000000 		or	%s59,0,%s38
 267      A6003B45 
 268 0670 C8000000 		st	%s59,200(0,%sp)
 268      8B003B11 
 269 0678 00000000 		or	%s58,0,%s39
 269      A7003A45 
 270 0680 C0000000 		st	%s58,192(0,%sp)
 270      8B003A11 
 271 0688 00000000 		or	%s57,0,%s36
 271      A4003945 
 272 0690 D8000000 		st	%s57,216(0,%sp)
 272      8B003911 
 273 0698 00000000 		or	%s56,0,%s37
 273      A5003845 
 274 06a0 D0000000 		st	%s56,208(0,%sp)
 274      8B003811 
 275 06a8 00000000 		or	%s55,0,%s34
 275      A2003745 
 276 06b0 E8000000 		st	%s55,232(0,%sp)
 276      8B003711 
 277 06b8 00000000 		or	%s54,0,%s35
 277      A3003645 
 278 06c0 E0000000 		st	%s54,224(0,%sp)
 278      8B003611 
 279 06c8 00000000 		or	%s53,0,%s42
 279      AA003545 
 280 06d0 F8000000 		st	%s53,248(0,%sp)
 280      8B003511 
 281 06d8 00000000 		or	%s52,0,%s43
 281      AB003445 
 282 06e0 F0000000 		st	%s52,240(0,%sp)
 282      8B003411 
 283 06e8 00000000 		or	%s51,0,%s44
 283      AC003345 
 284 06f0 08010000 		st	%s51,264(0,%sp)
 284      8B003311 
 285 06f8 00000000 		or	%s50,0,%s45
 285      AD003245 
 286 0700 00010000 		st	%s50,256(0,%sp)
 286      8B003211 
 287 0708 00000000 		or	%s18,0,%s46
 287      AE001245 
 288 0710 18010000 		st	%s18,280(0,%sp)
 288      8B001211 
 289 0718 00000000 		or	%s19,0,%s47
 289      AF001345 
 290 0720 10010000 		st	%s19,272(0,%sp)
 290      8B001311 
 291 0728 00000000 		or	%s20,0,%s48
 291      B0001445 
 292 0730 28010000 		st	%s20,296(0,%sp)
 292      8B001411 
 293 0738 00000000 		or	%s21,0,%s49
 293      B1001545 
 294 0740 20010000 		st	%s21,288(0,%sp)
 294      8B001511 
 295 0748 00000000 		or	%s0,0,%s61
 295      BD000045 
 296 0750 00000000 		or	%s1,0,%s60
 296      BC000145 
 297 0758 00000000 		or	%s2,0,%s59
 297      BB000245 
 298 0760 00000000 		or	%s3,0,%s58
 298      BA000345 
 299 0768 00000000 		or	%s4,0,%s57
 299      B9000445 
 300 0770 00000000 		or	%s5,0,%s56
 300      B8000545 
 301 0778 00000000 		or	%s6,0,%s55
 301      B7000645 
 302 0780 00000000 		or	%s7,0,%s54
 302      B6000745 
 303 0788 F8FFFFFF 		st	%s62,-8(,%fp)	# spill
 303      89003E11 
 304 0790 00000000 		lea	%s12,ext_ldq_fn_ldq@PLT_LO(-24)
 304      00680C06 
 305 0798 00000000 		and	%s12,%s12,(32)0
 305      608C0C44 
 306 07a0 00000000 		sic	%lr
 306      00000A28 
 307 07a8 00000000 		lea.sl	%s12,ext_ldq_fn_ldq@PLT_HI(%s12,%lr)
 307      8A8C8C06 
 308 07b0 00000000 		bsic	%lr,(,%s12)		# ext_ldq_fn_ldq
 308      8C000A08 
 309 07b8 F0FFFFFF 		st	%s0,-16(,%fp)	# spill
 309      89000011 
 310 07c0 08000000 		br.l	.L_2.1
 310      00000F18 
 311              	.L_2.1:
 312 07c8 F0FFFFFF 		ld	%s63,-16(,%fp)	# restore
 312      89003F01 
 313 07d0 00000000 		cvt.d.w %s62,%s63
 313      00BF3E5E 
 314 07d8 00000000 		cvt.q.d %s18,%s62
 314      00BE122D 
 315 07e0 F8FFFFFF 		ld	%s61,-8(,%fp)	# restore
 315      89003D01 
 316 07e8 00000000 		st	%s19,0(0,%s61)	# *(p_ret) (real)
 316      BD001311 
 317 07f0 00000000 		st	%s18,0(8,%s61)	# *(p_ret) (real)
 317      BD081211 
 318 07f8 00000000 		or	%s60,0,(0)1
 318      00003C45 
 319 0800 10000000 		st	%s60,16(0,%s61)	# *(p_ret) (imaginary)
 319      BD003C11 
 320 0808 00000000 		or	%s59,0,(0)1
 320      00003B45 
 321 0810 18000000 		st	%s59,24(0,%s61)	# *(p_ret) (imaginary)
 321      BD003B11 
 322              	# line 14
  14:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ldq_fn_ldq4.c **** 	return *p_ret;
 323              		.loc	1 14 0
 324 0818 00000000 		or	%s58,0,%s18
 324      92003A45 
 325 0820 00000000 		or	%s57,0,%s19
 325      93003945 
 326 0828 00000000 		or	%s56,0,(0)1
 326      00003845 
 327 0830 00000000 		or	%s55,0,(0)1
 327      00003745 
 328 0838 00000000 		or	%s0,0,%s58
 328      BA000045 
 329 0840 00000000 		or	%s1,0,%s57
 329      B9000145 
 330 0848 00000000 		or	%s2,0,%s56
 330      B8000245 
 331 0850 00000000 		or	%s3,0,%s55
 331      B7000345 
 332              	# Start of epilogue codes
 333 0858 30000000 		ld	%s18,48(,%fp)
 333      89001201 
 334 0860 38000000 		ld	%s19,56(,%fp)
 334      89001301 
 335 0868 40000000 		ld	%s20,64(,%fp)
 335      89001401 
 336 0870 48000000 		ld	%s21,72(,%fp)
 336      89001501 
 337 0878 00000000 		or	%sp,0,%fp
 337      89000B45 
 338              		.cfi_def_cfa	11,8
 339 0880 18000000 		ld	%got,0x18(,%sp)
 339      8B000F01 
 340 0888 20000000 		ld	%plt,0x20(,%sp)
 340      8B001001 
 341 0890 08000000 		ld	%lr,0x8(,%sp)
 341      8B000A01 
 342 0898 00000000 		ld	%fp,0x0(,%sp)
 342      8B000901 
 343 08a0 00000000 		b.l	(,%lr)
 343      8A000F19 
 344              	.L_2.EoE:
 345              		.cfi_endproc
 346              		.set	.L.2.2auto_size,	0xfffffffffffffdb0	# 592 Bytes
 348              	# ============ End  ldq_fn_ldq4b ============
 349              	# ============ Begin  mt_ldq_fn_ldq4 ============
 350 08a8 00000000 		.balign 16
 350      00000000 
 351              	.L_3.0:
 352              	# line 16
  15:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ldq_fn_ldq4.c **** }
  16:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ldq_fn_ldq4.c **** long double complex mt_ldq_fn_ldq4(long double complex ldq, long double complex ldq2, long double c
 353              		.loc	1 16 0
 354              		.globl	mt_ldq_fn_ldq4
 356              	mt_ldq_fn_ldq4:
 357              		.cfi_startproc
 358 08b0 00000000 		st	%fp,0x0(,%sp)
 358      8B000911 
 359              		.cfi_def_cfa_offset	0
 360              		.cfi_offset	9,0
 361 08b8 08000000 		st	%lr,0x8(,%sp)
 361      8B000A11 
 362 08c0 18000000 		st	%got,0x18(,%sp)
 362      8B000F11 
 363 08c8 20000000 		st	%plt,0x20(,%sp)
 363      8B001011 
 364 08d0 00000000 		lea	%got,_GLOBAL_OFFSET_TABLE_@PC_LO(-0x18)
 364      00680F06 
 365 08d8 00000000 		and	%got,%got,(32)0
 365      608F0F44 
 366 08e0 00000000 		sic	%plt
 366      00001028 
 367 08e8 00000000 		lea.sl	%got,_GLOBAL_OFFSET_TABLE_@PC_HI(%plt,%got)
 367      8F908F06 
 368 08f0 00000000 		or	%fp,0,%sp
 368      8B000945 
 369              		.cfi_def_cfa_register	9
 370 08f8 30000000 		st	%s18,48(,%fp)
 370      89001211 
 371 0900 38000000 		st	%s19,56(,%fp)
 371      89001311 
 372 0908 40000000 		st	%s20,64(,%fp)
 372      89001411 
 373 0910 48000000 		st	%s21,72(,%fp)
 373      89001511 
 374 0918 50000000 		st	%s22,80(,%fp)
 374      89001611 
 375 0920 58000000 		st	%s23,88(,%fp)
 375      89001711 
 376 0928 60000000 		st	%s24,96(,%fp)
 376      89001811 
 377 0930 68000000 		st	%s25,104(,%fp)
 377      89001911 
 378 0938 70000000 		st	%s26,112(,%fp)
 378      89001A11 
 379 0940 78000000 		st	%s27,120(,%fp)
 379      89001B11 
 380 0948 80000000 		st	%s28,128(,%fp)
 380      89001C11 
 381 0950 88000000 		st	%s29,136(,%fp)
 381      89001D11 
 382 0958 90000000 		st	%s30,144(,%fp)
 382      89001E11 
 383 0960 98000000 		st	%s31,152(,%fp)
 383      89001F11 
 384 0968 A0000000 		st	%s32,160(,%fp)
 384      89002011 
 385 0970 A8000000 		st	%s33,168(,%fp)
 385      89002111 
 386 0978 E0FFFFFF 		lea	%s13,.L.3.2auto_size&0xffffffff
 386      00000D06 
 387 0980 00000000 		and	%s13,%s13,(32)0
 387      608D0D44 
 388 0988 FFFFFFFF 		lea.sl	%sp,.L.3.2auto_size>>32(%fp,%s13)
 388      8D898B06 
 389 0990 48000000 		brge.l.t	%sp,%sl,.L_3.EoP
 389      888B3518 
 390 0998 18000000 		ld	%s61,0x18(,%tp)
 390      8E003D01 
 391 09a0 00000000 		or	%s62,0,%s0
 391      80003E45 
 392 09a8 3B010000 		lea	%s63,0x13b
 392      00003F06 
 393 09b0 00000000 		shm.l	%s63,0x0(%s61)
 393      BD033F31 
 394 09b8 08000000 		shm.l	%sl,0x8(%s61)
 394      BD030831 
 395 09c0 10000000 		shm.l	%sp,0x10(%s61)
 395      BD030B31 
 396 09c8 00000000 		monc
 396      0000003F 
 397 09d0 00000000 		or	%s0,0,%s62
 397      BE000045 
 398              	.L_3.EoP:
 399              	# End of prologue codes
 400 09d8 00000000 		or	%s34,0,%s6
 400      86002245 
 401 09e0 00000000 		or	%s35,0,%s7
 401      87002345 
 402 09e8 00000000 		or	%s36,0,%s4
 402      84002445 
 403 09f0 00000000 		or	%s37,0,%s5
 403      85002545 
 404 09f8 00000000 		or	%s38,0,%s2
 404      82002645 
 405 0a00 00000000 		or	%s39,0,%s3
 405      83002745 
 406              	# line 17
  17:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ldq_fn_ldq4.c **** 	long double complex ret = ldq+ldq2+ldq3+ldq4;
 407              		.loc	1 17 0
 408 0a08 00000000 		fadd.q	%s40,%s34,%s38
 408      A6A2286C 
 409              	# line 16
  16:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ldq_fn_ldq4.c **** 	long double complex ret = ldq+ldq2+ldq3+ldq4;
 410              		.loc	1 16 0
 411 0a10 00000000 		or	%s42,0,%s0
 411      80002A45 
 412 0a18 00000000 		or	%s43,0,%s1
 412      81002B45 
 413              	# line 17
 414              		.loc	1 17 0
 415 0a20 00000000 		fadd.q	%s44,%s36,%s42
 415      AAA42C6C 
 416              	# line 16
  16:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ldq_fn_ldq4.c **** 	long double complex ret = ldq+ldq2+ldq3+ldq4;
 417              		.loc	1 16 0
 418 0a28 B0000000 		st	%s43,176(0,%fp)	# ldq (real)
 418      89002B11 
 419 0a30 B0000000 		st	%s42,176(8,%fp)	# ldq (real)
 419      89082A11 
 420 0a38 C0000000 		st	%s39,192(0,%fp)	# ldq (imaginary)
 420      89002711 
 421 0a40 C0000000 		st	%s38,192(8,%fp)	# ldq (imaginary)
 421      89082611 
 422 0a48 D0000000 		st	%s37,208(0,%fp)	# ldq2 (real)
 422      89002511 
 423 0a50 D0000000 		st	%s36,208(8,%fp)	# ldq2 (real)
 423      89082411 
 424 0a58 E0000000 		st	%s35,224(0,%fp)	# ldq2 (imaginary)
 424      89002311 
 425 0a60 E0000000 		st	%s34,224(8,%fp)	# ldq2 (imaginary)
 425      89082211 
 426              	# line 17
 427              		.loc	1 17 0
 428 0a68 F0000000 		ld	%s47,240(0,%fp)	# ldq3 (real)
 428      89002F01 
 429 0a70 F0000000 		ld	%s46,240(8,%fp)	# ldq3 (real)
 429      89082E01 
 430 0a78 00000000 		fadd.q	%s48,%s44,%s46
 430      AEAC306C 
 431 0a80 00010000 		ld	%s51,256(0,%fp)	# ldq3 (imaginary)
 431      89003301 
 432 0a88 00010000 		ld	%s50,256(8,%fp)	# ldq3 (imaginary)
 432      89083201 
 433 0a90 00000000 		fadd.q	%s52,%s40,%s50
 433      B2A8346C 
 434 0a98 10010000 		ld	%s55,272(0,%fp)	# ldq4 (real)
 434      89003701 
 435 0aa0 10010000 		ld	%s54,272(8,%fp)	# ldq4 (real)
 435      89083601 
 436 0aa8 00000000 		fadd.q	%s56,%s48,%s54
 436      B6B0386C 
 437 0ab0 20010000 		ld	%s59,288(0,%fp)	# ldq4 (imaginary)
 437      89003B01 
 438 0ab8 20010000 		ld	%s58,288(8,%fp)	# ldq4 (imaginary)
 438      89083A01 
 439 0ac0 00000000 		fadd.q	%s60,%s52,%s58
 439      BAB43C6C 
 440 0ac8 E0FFFFFF 		st	%s57,-32(0,%fp)	# ret (real)
 440      89003911 
 441 0ad0 E0FFFFFF 		st	%s56,-32(8,%fp)	# ret (real)
 441      89083811 
 442 0ad8 F0FFFFFF 		st	%s61,-16(0,%fp)	# ret (imaginary)
 442      89003D11 
 443 0ae0 F0FFFFFF 		st	%s60,-16(8,%fp)	# ret (imaginary)
 443      89083C11 
 444              	# line 18
  18:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ldq_fn_ldq4.c **** 	return ret;
 445              		.loc	1 18 0
 446 0ae8 00000000 		or	%s63,0,%s56
 446      B8003F45 
 447 0af0 00000000 		or	%s62,0,%s57
 447      B9003E45 
 448 0af8 00000000 		or	%s18,0,%s60
 448      BC001245 
 449 0b00 00000000 		or	%s19,0,%s61
 449      BD001345 
 450 0b08 00000000 		or	%s0,0,%s63
 450      BF000045 
 451 0b10 00000000 		or	%s1,0,%s62
 451      BE000145 
 452 0b18 00000000 		or	%s2,0,%s18
 452      92000245 
 453 0b20 00000000 		or	%s3,0,%s19
 453      93000345 
 454              	# Start of epilogue codes
 455 0b28 30000000 		ld	%s18,48(,%fp)
 455      89001201 
 456 0b30 38000000 		ld	%s19,56(,%fp)
 456      89001301 
 457 0b38 00000000 		or	%sp,0,%fp
 457      89000B45 
 458              		.cfi_def_cfa	11,8
 459 0b40 18000000 		ld	%got,0x18(,%sp)
 459      8B000F01 
 460 0b48 20000000 		ld	%plt,0x20(,%sp)
 460      8B001001 
 461 0b50 08000000 		ld	%lr,0x8(,%sp)
 461      8B000A01 
 462 0b58 00000000 		ld	%fp,0x0(,%sp)
 462      8B000901 
 463 0b60 00000000 		b.l	(,%lr)
 463      8A000F19 
 464              	.L_3.EoE:
 465              		.cfi_endproc
 466              		.set	.L.3.2auto_size,	0xffffffffffffffe0	# 32 Bytes
 468              	# ============ End  mt_ldq_fn_ldq4 ============
 469              	# ============ Begin  prt_ldq_fn_ldq4 ============
 470              		.section .rodata
 471              		.balign 16
 473              	.LP.__string.0:
 474 0000 25       		.byte	37
 475 0001 73       		.byte	115
 476 0002 3A       		.byte	58
 477 0003 25       		.byte	37
 478 0004 75       		.byte	117
 479 0005 20       		.byte	32
 480 0006 25       		.byte	37
 481 0007 66       		.byte	102
 482 0008 2B       		.byte	43
 483 0009 25       		.byte	37
 484 000a 66       		.byte	102
 485 000b 69       		.byte	105
 486 000c 0A       		.byte	10
 487 000d 00       		.zero	1
 488 000e 0000     		.balign 8
 490              	.LP.__string.1:
 491 0010 2F       		.byte	47
 492 0011 75       		.byte	117
 493 0012 73       		.byte	115
 494 0013 72       		.byte	114
 495 0014 2F       		.byte	47
 496 0015 75       		.byte	117
 497 0016 68       		.byte	104
 498 0017 6F       		.byte	111
 499 0018 6D       		.byte	109
 500 0019 65       		.byte	101
 501 001a 2F       		.byte	47
 502 001b 61       		.byte	97
 503 001c 75       		.byte	117
 504 001d 72       		.byte	114
 505 001e 6F       		.byte	111
 506 001f 72       		.byte	114
 507 0020 61       		.byte	97
 508 0021 2F       		.byte	47
 509 0022 34       		.byte	52
 510 0023 67       		.byte	103
 511 0024 69       		.byte	105
 512 0025 2F       		.byte	47
 513 0026 6E       		.byte	110
 514 0027 6C       		.byte	108
 515 0028 61       		.byte	97
 516 0029 62       		.byte	98
 517 002a 68       		.byte	104
 518 002b 70       		.byte	112
 519 002c 67       		.byte	103
 520 002d 2F       		.byte	47
 521 002e 6B       		.byte	107
 522 002f 72       		.byte	114
 523 0030 75       		.byte	117
 524 0031 75       		.byte	117
 525 0032 73       		.byte	115
 526 0033 2F       		.byte	47
 527 0034 76       		.byte	118
 528 0035 74       		.byte	116
 529 0036 2F       		.byte	47
 530 0037 73       		.byte	115
 531 0038 72       		.byte	114
 532 0039 63       		.byte	99
 533 003a 2F       		.byte	47
 534 003b 61       		.byte	97
 535 003c 73       		.byte	115
 536 003d 6D       		.byte	109
 537 003e 2D       		.byte	45
 538 003f 65       		.byte	101
 539 0040 78       		.byte	120
 540 0041 61       		.byte	97
 541 0042 6D       		.byte	109
 542 0043 70       		.byte	112
 543 0044 6C       		.byte	108
 544 0045 65       		.byte	101
 545 0046 73       		.byte	115
 546 0047 2F       		.byte	47
 547 0048 6C       		.byte	108
 548 0049 64       		.byte	100
 549 004a 71       		.byte	113
 550 004b 5F       		.byte	95
 551 004c 66       		.byte	102
 552 004d 6E       		.byte	110
 553 004e 5F       		.byte	95
 554 004f 6C       		.byte	108
 555 0050 64       		.byte	100
 556 0051 71       		.byte	113
 557 0052 34       		.byte	52
 558 0053 2E       		.byte	46
 559 0054 63       		.byte	99
 560 0055 00       		.zero	1
 561              		.text
 562 0b68 00000000 		.balign 16
 562      00000000 
 563              	.L_4.0:
 564              	# line 20
  19:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ldq_fn_ldq4.c **** }
  20:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ldq_fn_ldq4.c **** long double complex prt_ldq_fn_ldq4(long double complex ldq, long double complex ldq2, long double 
 565              		.loc	1 20 0
 566              		.globl	prt_ldq_fn_ldq4
 568              	prt_ldq_fn_ldq4:
 569              		.cfi_startproc
 570 0b70 00000000 		st	%fp,0x0(,%sp)
 570      8B000911 
 571              		.cfi_def_cfa_offset	0
 572              		.cfi_offset	9,0
 573 0b78 08000000 		st	%lr,0x8(,%sp)
 573      8B000A11 
 574 0b80 18000000 		st	%got,0x18(,%sp)
 574      8B000F11 
 575 0b88 20000000 		st	%plt,0x20(,%sp)
 575      8B001011 
 576 0b90 00000000 		lea	%got,_GLOBAL_OFFSET_TABLE_@PC_LO(-0x18)
 576      00680F06 
 577 0b98 00000000 		and	%got,%got,(32)0
 577      608F0F44 
 578 0ba0 00000000 		sic	%plt
 578      00001028 
 579 0ba8 00000000 		lea.sl	%got,_GLOBAL_OFFSET_TABLE_@PC_HI(%plt,%got)
 579      8F908F06 
 580 0bb0 00000000 		or	%fp,0,%sp
 580      8B000945 
 581              		.cfi_def_cfa_register	9
 582 0bb8 30000000 		st	%s18,48(,%fp)
 582      89001211 
 583 0bc0 38000000 		st	%s19,56(,%fp)
 583      89001311 
 584 0bc8 40000000 		st	%s20,64(,%fp)
 584      89001411 
 585 0bd0 48000000 		st	%s21,72(,%fp)
 585      89001511 
 586 0bd8 50000000 		st	%s22,80(,%fp)
 586      89001611 
 587 0be0 58000000 		st	%s23,88(,%fp)
 587      89001711 
 588 0be8 60000000 		st	%s24,96(,%fp)
 588      89001811 
 589 0bf0 68000000 		st	%s25,104(,%fp)
 589      89001911 
 590 0bf8 70000000 		st	%s26,112(,%fp)
 590      89001A11 
 591 0c00 78000000 		st	%s27,120(,%fp)
 591      89001B11 
 592 0c08 80000000 		st	%s28,128(,%fp)
 592      89001C11 
 593 0c10 88000000 		st	%s29,136(,%fp)
 593      89001D11 
 594 0c18 90000000 		st	%s30,144(,%fp)
 594      89001E11 
 595 0c20 98000000 		st	%s31,152(,%fp)
 595      89001F11 
 596 0c28 A0000000 		st	%s32,160(,%fp)
 596      89002011 
 597 0c30 A8000000 		st	%s33,168(,%fp)
 597      89002111 
 598 0c38 D0FDFFFF 		lea	%s13,.L.4.2auto_size&0xffffffff
 598      00000D06 
 599 0c40 00000000 		and	%s13,%s13,(32)0
 599      608D0D44 
 600 0c48 FFFFFFFF 		lea.sl	%sp,.L.4.2auto_size>>32(%fp,%s13)
 600      8D898B06 
 601 0c50 48000000 		brge.l.t	%sp,%sl,.L_4.EoP
 601      888B3518 
 602 0c58 18000000 		ld	%s61,0x18(,%tp)
 602      8E003D01 
 603 0c60 00000000 		or	%s62,0,%s0
 603      80003E45 
 604 0c68 3B010000 		lea	%s63,0x13b
 604      00003F06 
 605 0c70 00000000 		shm.l	%s63,0x0(%s61)
 605      BD033F31 
 606 0c78 08000000 		shm.l	%sl,0x8(%s61)
 606      BD030831 
 607 0c80 10000000 		shm.l	%sp,0x10(%s61)
 607      BD030B31 
 608 0c88 00000000 		monc
 608      0000003F 
 609 0c90 00000000 		or	%s0,0,%s62
 609      BE000045 
 610              	.L_4.EoP:
 611              	# End of prologue codes
 612 0c98 00000000 		or	%s34,0,%s6
 612      86002245 
 613 0ca0 00000000 		or	%s35,0,%s7
 613      87002345 
 614 0ca8 00000000 		or	%s36,0,%s4
 614      84002445 
 615 0cb0 00000000 		or	%s37,0,%s5
 615      85002545 
 616 0cb8 00000000 		or	%s38,0,%s2
 616      82002645 
 617 0cc0 00000000 		or	%s39,0,%s3
 617      83002745 
 618              	# line 21
  21:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ldq_fn_ldq4.c **** 	long double complex ret = ldq+ldq2+ldq3+ldq4;
 619              		.loc	1 21 0
 620 0cc8 00000000 		fadd.q	%s40,%s34,%s38
 620      A6A2286C 
 621              	# line 20
  20:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ldq_fn_ldq4.c **** 	long double complex ret = ldq+ldq2+ldq3+ldq4;
 622              		.loc	1 20 0
 623 0cd0 00000000 		or	%s42,0,%s0
 623      80002A45 
 624 0cd8 00000000 		or	%s43,0,%s1
 624      81002B45 
 625              	# line 21
 626              		.loc	1 21 0
 627 0ce0 00000000 		fadd.q	%s44,%s36,%s42
 627      AAA42C6C 
 628              	# line 20
  20:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ldq_fn_ldq4.c **** 	long double complex ret = ldq+ldq2+ldq3+ldq4;
 629              		.loc	1 20 0
 630 0ce8 B0000000 		st	%s43,176(0,%fp)	# ldq (real)
 630      89002B11 
 631 0cf0 B0000000 		st	%s42,176(8,%fp)	# ldq (real)
 631      89082A11 
 632 0cf8 C0000000 		st	%s39,192(0,%fp)	# ldq (imaginary)
 632      89002711 
 633 0d00 C0000000 		st	%s38,192(8,%fp)	# ldq (imaginary)
 633      89082611 
 634 0d08 D0000000 		st	%s37,208(0,%fp)	# ldq2 (real)
 634      89002511 
 635 0d10 D0000000 		st	%s36,208(8,%fp)	# ldq2 (real)
 635      89082411 
 636 0d18 E0000000 		st	%s35,224(0,%fp)	# ldq2 (imaginary)
 636      89002311 
 637 0d20 E0000000 		st	%s34,224(8,%fp)	# ldq2 (imaginary)
 637      89082211 
 638              	# line 21
 639              		.loc	1 21 0
 640 0d28 F0000000 		ld	%s47,240(0,%fp)	# ldq3 (real)
 640      89002F01 
 641 0d30 F0000000 		ld	%s46,240(8,%fp)	# ldq3 (real)
 641      89082E01 
 642 0d38 00000000 		fadd.q	%s48,%s44,%s46
 642      AEAC306C 
 643 0d40 00010000 		ld	%s51,256(0,%fp)	# ldq3 (imaginary)
 643      89003301 
 644 0d48 00010000 		ld	%s50,256(8,%fp)	# ldq3 (imaginary)
 644      89083201 
 645 0d50 00000000 		fadd.q	%s52,%s40,%s50
 645      B2A8346C 
 646 0d58 10010000 		ld	%s55,272(0,%fp)	# ldq4 (real)
 646      89003701 
 647 0d60 10010000 		ld	%s54,272(8,%fp)	# ldq4 (real)
 647      89083601 
 648 0d68 00000000 		fadd.q	%s56,%s48,%s54
 648      B6B0386C 
 649 0d70 20010000 		ld	%s59,288(0,%fp)	# ldq4 (imaginary)
 649      89003B01 
 650 0d78 20010000 		ld	%s58,288(8,%fp)	# ldq4 (imaginary)
 650      89083A01 
 651 0d80 00000000 		fadd.q	%s60,%s52,%s58
 651      BAB43C6C 
 652 0d88 E0FFFFFF 		st	%s57,-32(0,%fp)	# ret (real)
 652      89003911 
 653 0d90 E0FFFFFF 		st	%s56,-32(8,%fp)	# ret (real)
 653      89083811 
 654 0d98 F0FFFFFF 		st	%s61,-16(0,%fp)	# ret (imaginary)
 654      89003D11 
 655 0da0 F0FFFFFF 		st	%s60,-16(8,%fp)	# ret (imaginary)
 655      89083C11 
 656              	# line 22
  22:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ldq_fn_ldq4.c **** 	printf("%s:%u %f+%fi\n",__FILE__,__LINE__,(double)creall(ret),(double)cimagl(ret));
 657              		.loc	1 22 0
 658 0da8 00000000 		lea	%s63,.LP.__string.0@GOTOFF_LO
 658      00003F06 
 659 0db0 00000000 		and	%s63,%s63,(32)0
 659      60BF3F44 
 660 0db8 00000000 		lea.sl	%s63,.LP.__string.0@GOTOFF_HI(%s63,%got)
 660      8FBFBF06 
 661 0dc0 00000000 		lea	%s62,.LP.__string.1@GOTOFF_LO
 661      00003E06 
 662 0dc8 00000000 		and	%s62,%s62,(32)0
 662      60BE3E44 
 663 0dd0 00000000 		lea.sl	%s62,.LP.__string.1@GOTOFF_HI(%s62,%got)
 663      8FBEBE06 
 664 0dd8 00000000 		or	%s18,0,%s56
 664      B8001245 
 665 0de0 00000000 		or	%s19,0,%s57
 665      B9001345 
 666 0de8 00000000 		or	%s20,0,%s60
 666      BC001445 
 667 0df0 00000000 		or	%s21,0,%s61
 667      BD001545 
 668 0df8 00000000 		or	%s0,0,%s18
 668      92000045 
 669 0e00 00000000 		or	%s1,0,%s19
 669      93000145 
 670 0e08 00000000 		or	%s2,0,%s20
 670      94000245 
 671 0e10 00000000 		or	%s3,0,%s21
 671      95000345 
 672 0e18 D8FFFFFF 		st	%s63,-40(,%fp)	# spill
 672      89003F11 
 673 0e20 D0FFFFFF 		st	%s62,-48(,%fp)	# spill
 673      89003E11 
 674 0e28 00000000 		lea	%s12,creall@PLT_LO(-24)
 674      00680C06 
 675 0e30 00000000 		and	%s12,%s12,(32)0
 675      608C0C44 
 676 0e38 00000000 		sic	%lr
 676      00000A28 
 677 0e40 00000000 		lea.sl	%s12,creall@PLT_HI(%s12,%lr)
 677      8A8C8C06 
 678 0e48 00000000 		bsic	%lr,(,%s12)		# creall
 678      8C000A08 
 679              	.L_4.3:
 680 0e50 00000000 		cvt.d.q %s63,%s0
 680      0080BF0F 
 681 0e58 E0FFFFFF 		ld	%s19,-32(0,%fp)	# ret (real)
 681      89001301 
 682 0e60 E0FFFFFF 		ld	%s18,-32(8,%fp)	# ret (real)
 682      89081201 
 683 0e68 F0FFFFFF 		ld	%s21,-16(0,%fp)	# ret (imaginary)
 683      89001501 
 684 0e70 F0FFFFFF 		ld	%s20,-16(8,%fp)	# ret (imaginary)
 684      89081401 
 685 0e78 00000000 		or	%s62,0,%s18
 685      92003E45 
 686 0e80 00000000 		or	%s61,0,%s19
 686      93003D45 
 687 0e88 00000000 		or	%s60,0,%s20
 687      94003C45 
 688 0e90 00000000 		or	%s59,0,%s21
 688      95003B45 
 689 0e98 00000000 		or	%s0,0,%s62
 689      BE000045 
 690 0ea0 00000000 		or	%s1,0,%s61
 690      BD000145 
 691 0ea8 00000000 		or	%s2,0,%s60
 691      BC000245 
 692 0eb0 00000000 		or	%s3,0,%s59
 692      BB000345 
 693 0eb8 C8FFFFFF 		st	%s63,-56(,%fp)	# spill
 693      89003F11 
 694 0ec0 00000000 		lea	%s12,cimagl@PLT_LO(-24)
 694      00680C06 
 695 0ec8 00000000 		and	%s12,%s12,(32)0
 695      608C0C44 
 696 0ed0 00000000 		sic	%lr
 696      00000A28 
 697 0ed8 00000000 		lea.sl	%s12,cimagl@PLT_HI(%s12,%lr)
 697      8A8C8C06 
 698 0ee0 00000000 		bsic	%lr,(,%s12)		# cimagl
 698      8C000A08 
 699              	.L_4.2:
 700 0ee8 00000000 		cvt.d.q %s63,%s0
 700      0080BF0F 
 701 0ef0 D0000000 		st	%s63,208(0,%sp)
 701      8B003F11 
 702 0ef8 D8FFFFFF 		ld	%s62,-40(,%fp)	# restore
 702      89003E01 
 703 0f00 B0000000 		st	%s62,176(0,%sp)
 703      8B003E11 
 704 0f08 D0FFFFFF 		ld	%s61,-48(,%fp)	# restore
 704      89003D01 
 705 0f10 B8000000 		st	%s61,184(0,%sp)
 705      8B003D11 
 706 0f18 00000000 		or	%s60,22,(0)1
 706      00163C45 
 707 0f20 C0000000 		st	%s60,192(0,%sp)
 707      8B003C11 
 708 0f28 C8FFFFFF 		ld	%s59,-56(,%fp)	# restore
 708      89003B01 
 709 0f30 C8000000 		st	%s59,200(0,%sp)
 709      8B003B11 
 710 0f38 00000000 		or	%s0,0,%s62
 710      BE000045 
 711 0f40 00000000 		or	%s1,0,%s61
 711      BD000145 
 712 0f48 00000000 		or	%s2,0,%s60
 712      BC000245 
 713 0f50 00000000 		or	%s3,0,%s59
 713      BB000345 
 714 0f58 00000000 		or	%s4,0,%s63
 714      BF000445 
 715 0f60 00000000 		lea	%s12,printf@PLT_LO(-24)
 715      00680C06 
 716 0f68 00000000 		and	%s12,%s12,(32)0
 716      608C0C44 
 717 0f70 00000000 		sic	%lr
 717      00000A28 
 718 0f78 00000000 		lea.sl	%s12,printf@PLT_HI(%s12,%lr)
 718      8A8C8C06 
 719 0f80 00000000 		bsic	%lr,(,%s12)		# printf
 719      8C000A08 
 720 0f88 08000000 		br.l	.L_4.1
 720      00000F18 
 721              	.L_4.1:
 722              	# line 23
  23:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/ldq_fn_ldq4.c **** 	return ret;
 723              		.loc	1 23 0
 724 0f90 E0FFFFFF 		ld	%s19,-32(0,%fp)	# ret (real)
 724      89001301 
 725 0f98 E0FFFFFF 		ld	%s18,-32(8,%fp)	# ret (real)
 725      89081201 
 726 0fa0 F0FFFFFF 		ld	%s21,-16(0,%fp)	# ret (imaginary)
 726      89001501 
 727 0fa8 F0FFFFFF 		ld	%s20,-16(8,%fp)	# ret (imaginary)
 727      89081401 
 728 0fb0 00000000 		or	%s63,0,%s18
 728      92003F45 
 729 0fb8 00000000 		or	%s62,0,%s19
 729      93003E45 
 730 0fc0 00000000 		or	%s61,0,%s20
 730      94003D45 
 731 0fc8 00000000 		or	%s60,0,%s21
 731      95003C45 
 732 0fd0 00000000 		or	%s0,0,%s63
 732      BF000045 
 733 0fd8 00000000 		or	%s1,0,%s62
 733      BE000145 
 734 0fe0 00000000 		or	%s2,0,%s61
 734      BD000245 
 735 0fe8 00000000 		or	%s3,0,%s60
 735      BC000345 
 736              	# Start of epilogue codes
 737 0ff0 30000000 		ld	%s18,48(,%fp)
 737      89001201 
 738 0ff8 38000000 		ld	%s19,56(,%fp)
 738      89001301 
 739 1000 40000000 		ld	%s20,64(,%fp)
 739      89001401 
 740 1008 48000000 		ld	%s21,72(,%fp)
 740      89001501 
 741 1010 00000000 		or	%sp,0,%fp
 741      89000B45 
 742              		.cfi_def_cfa	11,8
 743 1018 18000000 		ld	%got,0x18(,%sp)
 743      8B000F01 
 744 1020 20000000 		ld	%plt,0x20(,%sp)
 744      8B001001 
 745 1028 08000000 		ld	%lr,0x8(,%sp)
 745      8B000A01 
 746 1030 00000000 		ld	%fp,0x0(,%sp)
 746      8B000901 
 747 1038 00000000 		b.l	(,%lr)
 747      8A000F19 
 748              	.L_4.EoE:
 749              		.cfi_endproc
 750              		.set	.L.4.2auto_size,	0xfffffffffffffdd0	# 560 Bytes
 752              	# ============ End  prt_ldq_fn_ldq4 ============
 753              	.Le1.0:
