   1              		.ident "ncc 1.1.4 (Build 10:17:22 Apr 19 2018)"
   2              		.file "f_add256.c"
   3              		.file 1 "/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c"
   4              		.file 2 "/opt/nec/ve/ncc/1.1.4/include/stdc-predef.h"
   5              	# ============ Begin  fp_add256_fp2 ============
   6              		.text
   7              		.balign 16
   8              	.L_1.0:
   9              	# line 3
   1:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c **** // vim: et ts=4 sw=4 cindent cino=^=l0,\:0,N-s
   2:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c **** /** for i=0 to 255 { a[i] += b[i]; } */
   3:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c **** float* fp_add256_fp2(float* restrict a, float const* restrict b){
  10              		.loc	1 3 0
  11              		.globl	fp_add256_fp2
  13              	fp_add256_fp2:
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
  43 00c8 00000000 		lea	%s13,.L.1.2auto_size&0xffffffff
  43      00000D06 
  44 00d0 00000000 		and	%s13,%s13,(32)0
  44      608D0D44 
  45 00d8 00000000 		lea.sl	%sp,.L.1.2auto_size>>32(%fp,%s13)
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
  57              	# line 9
   4:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****     // vldu.nc %v63,4,%s62     # *(a)   -- vector load upper for float load
   5:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****     // vldu  dest, stride in bytes, base (non-cached)
   6:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****     // vldu.nc %v62,4,%s61     # *(b)
   7:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****     // vfadd.s %v61,%v63,%v62
   8:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****     // vstu.nc %v61,4,%s60     # *(a)
   9:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c **** 	for(int i=0; i<256; ++i)
  58              		.loc	1 9 0
  59 0128 00010000 		lea	%s63,256
  59      00003F06 
  60              	# line 10
  10:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c **** 		a[i] = a[i] + b[i];
  61              		.loc	1 10 0
  62 0130 00000000 		or	%s62,%s0,(0)1
  62      00803E45 
  63 0138 00000000 		lvl	%s63
  63      00BF00BF 
  64 0140 0000003F 		vldu.nc	%v63,4,%s62	# *(a)
  64      BE040082 
  65 0148 00000000 		or	%s61,%s1,(0)1
  65      00813D45 
  66 0150 0000003E 		vldu.nc	%v62,4,%s61	# *(b)
  66      BD040082 
  67 0158 003E3F3D 		vfadd.s	%v61,%v63,%v62
  67      000080CC 
  68 0160 00000000 		or	%s60,%s0,(0)1
  68      00803C45 
  69 0168 0000003D 		vstu.nc	%v61,4,%s60	# *(a)
  69      BC040092 
  70              	# line 11
  11:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****     return a;
  71              		.loc	1 11 0
  72              	# Start of epilogue codes
  73 0170 00000000 		or	%sp,0,%fp
  73      89000B45 
  74              		.cfi_def_cfa	11,8
  75 0178 18000000 		ld	%got,0x18(,%sp)
  75      8B000F01 
  76 0180 20000000 		ld	%plt,0x20(,%sp)
  76      8B001001 
  77 0188 08000000 		ld	%lr,0x8(,%sp)
  77      8B000A01 
  78 0190 00000000 		ld	%fp,0x0(,%sp)
  78      8B000901 
  79 0198 00000000 		b.l	(,%lr)
  79      8A000F19 
  80              	.L_1.EoE:
  81              		.cfi_endproc
  82              		.set	.L.1.2auto_size,	0x0	# 0 Bytes
  84              	# ============ End  fp_add256_fp2 ============
  85              	# ============ Begin  fp_sub256_fp_f ============
  86              		.balign 16
  87              	.L_2.0:
  88              	# line 14
  12:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c **** }
  13:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c **** 
  14:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c **** float* fp_sub256_fp_f(float* restrict a, float const x){
  89              		.loc	1 14 0
  90              		.globl	fp_sub256_fp_f
  92              	fp_sub256_fp_f:
  93              		.cfi_startproc
  94 01a0 00000000 		st	%fp,0x0(,%sp)
  94      8B000911 
  95              		.cfi_def_cfa_offset	0
  96              		.cfi_offset	9,0
  97 01a8 08000000 		st	%lr,0x8(,%sp)
  97      8B000A11 
  98 01b0 18000000 		st	%got,0x18(,%sp)
  98      8B000F11 
  99 01b8 20000000 		st	%plt,0x20(,%sp)
  99      8B001011 
 100 01c0 00000000 		lea	%got,_GLOBAL_OFFSET_TABLE_@PC_LO(-0x18)
 100      00680F06 
 101 01c8 00000000 		and	%got,%got,(32)0
 101      608F0F44 
 102 01d0 00000000 		sic	%plt
 102      00001028 
 103 01d8 00000000 		lea.sl	%got,_GLOBAL_OFFSET_TABLE_@PC_HI(%plt,%got)
 103      8F908F06 
 104 01e0 00000000 		or	%fp,0,%sp
 104      8B000945 
 105              		.cfi_def_cfa_register	9
 106 01e8 30000000 		st	%s18,48(,%fp)
 106      89001211 
 107 01f0 38000000 		st	%s19,56(,%fp)
 107      89001311 
 108 01f8 40000000 		st	%s20,64(,%fp)
 108      89001411 
 109 0200 48000000 		st	%s21,72(,%fp)
 109      89001511 
 110 0208 50000000 		st	%s22,80(,%fp)
 110      89001611 
 111 0210 58000000 		st	%s23,88(,%fp)
 111      89001711 
 112 0218 60000000 		st	%s24,96(,%fp)
 112      89001811 
 113 0220 68000000 		st	%s25,104(,%fp)
 113      89001911 
 114 0228 70000000 		st	%s26,112(,%fp)
 114      89001A11 
 115 0230 78000000 		st	%s27,120(,%fp)
 115      89001B11 
 116 0238 80000000 		st	%s28,128(,%fp)
 116      89001C11 
 117 0240 88000000 		st	%s29,136(,%fp)
 117      89001D11 
 118 0248 90000000 		st	%s30,144(,%fp)
 118      89001E11 
 119 0250 98000000 		st	%s31,152(,%fp)
 119      89001F11 
 120 0258 A0000000 		st	%s32,160(,%fp)
 120      89002011 
 121 0260 A8000000 		st	%s33,168(,%fp)
 121      89002111 
 122 0268 00000000 		lea	%s13,.L.2.2auto_size&0xffffffff
 122      00000D06 
 123 0270 00000000 		and	%s13,%s13,(32)0
 123      608D0D44 
 124 0278 00000000 		lea.sl	%sp,.L.2.2auto_size>>32(%fp,%s13)
 124      8D898B06 
 125 0280 48000000 		brge.l.t	%sp,%sl,.L_2.EoP
 125      888B3518 
 126 0288 18000000 		ld	%s61,0x18(,%tp)
 126      8E003D01 
 127 0290 00000000 		or	%s62,0,%s0
 127      80003E45 
 128 0298 3B010000 		lea	%s63,0x13b
 128      00003F06 
 129 02a0 00000000 		shm.l	%s63,0x0(%s61)
 129      BD033F31 
 130 02a8 08000000 		shm.l	%sl,0x8(%s61)
 130      BD030831 
 131 02b0 10000000 		shm.l	%sp,0x10(%s61)
 131      BD030B31 
 132 02b8 00000000 		monc
 132      0000003F 
 133 02c0 00000000 		or	%s0,0,%s62
 133      BE000045 
 134              	.L_2.EoP:
 135              	# End of prologue codes
 136              	# line 21
  15:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****     // vldu.nc %v63,4,%s62     # *(a)
  16:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****     // or      %s61,0,(0)1
  17:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****     // fsub.s  %s60,%s61,%s1   # -x
  18:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****     // vfadd.s %v62,%s60,%v63
  19:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****     // ^^^^^^^ add scalar to vector
  20:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****     // vstu.nc %v62,4,%s59     # *(a)
  21:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****     for(int i=0; i<256; ++i)
 137              		.loc	1 21 0
 138 02c8 00010000 		lea	%s63,256
 138      00003F06 
 139              	# line 22
  22:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****         a[i] -= x;
 140              		.loc	1 22 0
 141 02d0 00000000 		or	%s62,%s0,(0)1
 141      00803E45 
 142 02d8 00000000 		lvl	%s63
 142      00BF00BF 
 143 02e0 0000003F 		vldu.nc	%v63,4,%s62	# *(a)
 143      BE040082 
 144 02e8 00000000 		or	%s61,0,(0)1
 144      00003D45 
 145 02f0 00000000 		fsub.s	%s60,%s61,%s1
 145      81BDBC5C 
 146 02f8 003F003E 		vfadd.s	%v62,%s60,%v63
 146      00BCA0CC 
 147 0300 00000000 		or	%s59,%s0,(0)1
 147      00803B45 
 148 0308 0000003E 		vstu.nc	%v62,4,%s59	# *(a)
 148      BB040092 
 149              	# line 23
  23:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c **** }
 150              		.loc	1 23 0
 151              	# Start of epilogue codes
 152 0310 00000000 		or	%sp,0,%fp
 152      89000B45 
 153              		.cfi_def_cfa	11,8
 154 0318 18000000 		ld	%got,0x18(,%sp)
 154      8B000F01 
 155 0320 20000000 		ld	%plt,0x20(,%sp)
 155      8B001001 
 156 0328 08000000 		ld	%lr,0x8(,%sp)
 156      8B000A01 
 157 0330 00000000 		ld	%fp,0x0(,%sp)
 157      8B000901 
 158 0338 00000000 		b.l	(,%lr)
 158      8A000F19 
 159              	.L_2.EoE:
 160              		.cfi_endproc
 161              		.set	.L.2.2auto_size,	0x0	# 0 Bytes
 163              	# ============ End  fp_sub256_fp_f ============
 164              	# ============ Begin  dp_add256_dp2 ============
 165              		.balign 16
 166              	.L_3.0:
 167              	# line 25
  24:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c **** 
  25:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c **** double* dp_add256_dp2(double* restrict a, double const* restrict b){
 168              		.loc	1 25 0
 169              		.globl	dp_add256_dp2
 171              	dp_add256_dp2:
 172              		.cfi_startproc
 173 0340 00000000 		st	%fp,0x0(,%sp)
 173      8B000911 
 174              		.cfi_def_cfa_offset	0
 175              		.cfi_offset	9,0
 176 0348 08000000 		st	%lr,0x8(,%sp)
 176      8B000A11 
 177 0350 18000000 		st	%got,0x18(,%sp)
 177      8B000F11 
 178 0358 20000000 		st	%plt,0x20(,%sp)
 178      8B001011 
 179 0360 00000000 		lea	%got,_GLOBAL_OFFSET_TABLE_@PC_LO(-0x18)
 179      00680F06 
 180 0368 00000000 		and	%got,%got,(32)0
 180      608F0F44 
 181 0370 00000000 		sic	%plt
 181      00001028 
 182 0378 00000000 		lea.sl	%got,_GLOBAL_OFFSET_TABLE_@PC_HI(%plt,%got)
 182      8F908F06 
 183 0380 00000000 		or	%fp,0,%sp
 183      8B000945 
 184              		.cfi_def_cfa_register	9
 185 0388 30000000 		st	%s18,48(,%fp)
 185      89001211 
 186 0390 38000000 		st	%s19,56(,%fp)
 186      89001311 
 187 0398 40000000 		st	%s20,64(,%fp)
 187      89001411 
 188 03a0 48000000 		st	%s21,72(,%fp)
 188      89001511 
 189 03a8 50000000 		st	%s22,80(,%fp)
 189      89001611 
 190 03b0 58000000 		st	%s23,88(,%fp)
 190      89001711 
 191 03b8 60000000 		st	%s24,96(,%fp)
 191      89001811 
 192 03c0 68000000 		st	%s25,104(,%fp)
 192      89001911 
 193 03c8 70000000 		st	%s26,112(,%fp)
 193      89001A11 
 194 03d0 78000000 		st	%s27,120(,%fp)
 194      89001B11 
 195 03d8 80000000 		st	%s28,128(,%fp)
 195      89001C11 
 196 03e0 88000000 		st	%s29,136(,%fp)
 196      89001D11 
 197 03e8 90000000 		st	%s30,144(,%fp)
 197      89001E11 
 198 03f0 98000000 		st	%s31,152(,%fp)
 198      89001F11 
 199 03f8 A0000000 		st	%s32,160(,%fp)
 199      89002011 
 200 0400 A8000000 		st	%s33,168(,%fp)
 200      89002111 
 201 0408 00000000 		lea	%s13,.L.3.2auto_size&0xffffffff
 201      00000D06 
 202 0410 00000000 		and	%s13,%s13,(32)0
 202      608D0D44 
 203 0418 00000000 		lea.sl	%sp,.L.3.2auto_size>>32(%fp,%s13)
 203      8D898B06 
 204 0420 48000000 		brge.l.t	%sp,%sl,.L_3.EoP
 204      888B3518 
 205 0428 18000000 		ld	%s61,0x18(,%tp)
 205      8E003D01 
 206 0430 00000000 		or	%s62,0,%s0
 206      80003E45 
 207 0438 3B010000 		lea	%s63,0x13b
 207      00003F06 
 208 0440 00000000 		shm.l	%s63,0x0(%s61)
 208      BD033F31 
 209 0448 08000000 		shm.l	%sl,0x8(%s61)
 209      BD030831 
 210 0450 10000000 		shm.l	%sp,0x10(%s61)
 210      BD030B31 
 211 0458 00000000 		monc
 211      0000003F 
 212 0460 00000000 		or	%s0,0,%s62
 212      BE000045 
 213              	.L_3.EoP:
 214              	# End of prologue codes
 215              	# line 30
  26:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****     // vld.nc  %v63,8,%s62     # *(a)
  27:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****     // vld.nc  %v62,8,%s61     # *(b)
  28:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****     // vfadd.d %v61,%v63,%v62
  29:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****     // vst.nc  %v61,8,%s60     # *(a)
  30:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c **** 	for(int i=0; i<256; ++i)
 216              		.loc	1 30 0
 217 0468 00010000 		lea	%s63,256
 217      00003F06 
 218              	# line 31
  31:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c **** 		a[i] = a[i] + b[i];
 219              		.loc	1 31 0
 220 0470 00000000 		or	%s62,%s0,(0)1
 220      00803E45 
 221 0478 00000000 		lvl	%s63
 221      00BF00BF 
 222 0480 0000003F 		vld.nc	%v63,8,%s62	# *(a)
 222      BE080081 
 223 0488 00000000 		or	%s61,%s1,(0)1
 223      00813D45 
 224 0490 0000003E 		vld.nc	%v62,8,%s61	# *(b)
 224      BD080081 
 225 0498 003E3F3D 		vfadd.d	%v61,%v63,%v62
 225      000000CC 
 226 04a0 00000000 		or	%s60,%s0,(0)1
 226      00803C45 
 227 04a8 0000003D 		vst.nc	%v61,8,%s60	# *(a)
 227      BC080091 
 228              	# line 32
  32:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****     return a;
 229              		.loc	1 32 0
 230              	# Start of epilogue codes
 231 04b0 00000000 		or	%sp,0,%fp
 231      89000B45 
 232              		.cfi_def_cfa	11,8
 233 04b8 18000000 		ld	%got,0x18(,%sp)
 233      8B000F01 
 234 04c0 20000000 		ld	%plt,0x20(,%sp)
 234      8B001001 
 235 04c8 08000000 		ld	%lr,0x8(,%sp)
 235      8B000A01 
 236 04d0 00000000 		ld	%fp,0x0(,%sp)
 236      8B000901 
 237 04d8 00000000 		b.l	(,%lr)
 237      8A000F19 
 238              	.L_3.EoE:
 239              		.cfi_endproc
 240              		.set	.L.3.2auto_size,	0x0	# 0 Bytes
 242              	# ============ End  dp_add256_dp2 ============
 243              	# ============ Begin  v_reduce256g16_fp2 ============
 244              		.balign 16
 245              	.L_4.0:
 246              	# line 35
  33:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c **** }
  34:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c **** 
  35:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c **** void v_reduce256g16_fp2(float const* restrict a, float*restrict group_sums){
 247              		.loc	1 35 0
 248              		.globl	v_reduce256g16_fp2
 250              	v_reduce256g16_fp2:
 251              		.cfi_startproc
 252 04e0 00000000 		st	%fp,0x0(,%sp)
 252      8B000911 
 253              		.cfi_def_cfa_offset	0
 254              		.cfi_offset	9,0
 255 04e8 08000000 		st	%lr,0x8(,%sp)
 255      8B000A11 
 256 04f0 18000000 		st	%got,0x18(,%sp)
 256      8B000F11 
 257 04f8 20000000 		st	%plt,0x20(,%sp)
 257      8B001011 
 258 0500 00000000 		lea	%got,_GLOBAL_OFFSET_TABLE_@PC_LO(-0x18)
 258      00680F06 
 259 0508 00000000 		and	%got,%got,(32)0
 259      608F0F44 
 260 0510 00000000 		sic	%plt
 260      00001028 
 261 0518 00000000 		lea.sl	%got,_GLOBAL_OFFSET_TABLE_@PC_HI(%plt,%got)
 261      8F908F06 
 262 0520 00000000 		or	%fp,0,%sp
 262      8B000945 
 263              		.cfi_def_cfa_register	9
 264 0528 30000000 		st	%s18,48(,%fp)
 264      89001211 
 265 0530 38000000 		st	%s19,56(,%fp)
 265      89001311 
 266 0538 40000000 		st	%s20,64(,%fp)
 266      89001411 
 267 0540 48000000 		st	%s21,72(,%fp)
 267      89001511 
 268 0548 50000000 		st	%s22,80(,%fp)
 268      89001611 
 269 0550 58000000 		st	%s23,88(,%fp)
 269      89001711 
 270 0558 60000000 		st	%s24,96(,%fp)
 270      89001811 
 271 0560 68000000 		st	%s25,104(,%fp)
 271      89001911 
 272 0568 70000000 		st	%s26,112(,%fp)
 272      89001A11 
 273 0570 78000000 		st	%s27,120(,%fp)
 273      89001B11 
 274 0578 80000000 		st	%s28,128(,%fp)
 274      89001C11 
 275 0580 88000000 		st	%s29,136(,%fp)
 275      89001D11 
 276 0588 90000000 		st	%s30,144(,%fp)
 276      89001E11 
 277 0590 98000000 		st	%s31,152(,%fp)
 277      89001F11 
 278 0598 A0000000 		st	%s32,160(,%fp)
 278      89002011 
 279 05a0 A8000000 		st	%s33,168(,%fp)
 279      89002111 
 280 05a8 C0FDFFFF 		lea	%s13,.L.4.2auto_size&0xffffffff
 280      00000D06 
 281 05b0 00000000 		and	%s13,%s13,(32)0
 281      608D0D44 
 282 05b8 FFFFFFFF 		lea.sl	%sp,.L.4.2auto_size>>32(%fp,%s13)
 282      8D898B06 
 283 05c0 48000000 		brge.l.t	%sp,%sl,.L_4.EoP
 283      888B3518 
 284 05c8 18000000 		ld	%s61,0x18(,%tp)
 284      8E003D01 
 285 05d0 00000000 		or	%s62,0,%s0
 285      80003E45 
 286 05d8 3B010000 		lea	%s63,0x13b
 286      00003F06 
 287 05e0 00000000 		shm.l	%s63,0x0(%s61)
 287      BD033F31 
 288 05e8 08000000 		shm.l	%sl,0x8(%s61)
 288      BD030831 
 289 05f0 10000000 		shm.l	%sp,0x10(%s61)
 289      BD030B31 
 290 05f8 00000000 		monc
 290      0000003F 
 291 0600 00000000 		or	%s0,0,%s62
 291      BE000045 
 292              	.L_4.EoP:
 293              	# End of prologue codes
 294              	# line 37
  36:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****     // not vectorized
  37:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****     for(int i=0; i<16; ++i){
 295              		.loc	1 37 0
 296 0608 00000000 		or	%s63,16,(0)1
 296      00103F45 
 297              	# line 38
  38:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****         group_sums[i] = 0;
 298              		.loc	1 38 0
 299 0610 00000000 		or	%s62,0,(0)1
 299      00003E45 
 300 0618 00000000 		lvl	%s63
 300      00BF00BF 
 301 0620 0000003F 		vbrdu	%v63,%s62
 301      00BE808C 
 302 0628 00000000 		or	%s61,%s1,(0)1
 302      00813D45 
 303 0630 0000003F 		vstu.nc	%v63,4,%s61	# *(group_sums)
 303      BD040092 
 304              	# line 40
  39:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****     }
  40:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****     for(int i=0; i<256; ++i){
 305              		.loc	1 40 0
 306 0638 00010000 		lea	%s60,256
 306      00003C06 
 307 0640 00010000 		lea	%s59,256
 307      00003B06 
 308 0648 98020000 		brgt.l	%s60,%s59,.L_4.9
 308      BBBC0118 
 309 0650 08000000 		br.l	.L_4.10
 309      00000F18 
 310              	.L_4.10:
 311 0658 00000000 		or	%s58,0,(0)1
 311      00003A45 
 312 0660 00000000 		lea	%s57,_PVWORK_STATIC@GOT_LO
 312      00003906 
 313 0668 00000000 		and	%s57,%s57,(32)0
 313      60B93944 
 314 0670 00000000 		lea.sl	%s57,_PVWORK_STATIC@GOT_HI(%s57,%got)
 314      8FB9B906 
 315 0678 00000000 		ld	%s57,(,%s57)
 315      B9003901 
 316 0680 00000000 		lea	%s56,_PVWORK_STATIC@GOT_LO
 316      00003806 
 317 0688 00000000 		and	%s56,%s56,(32)0
 317      60B83844 
 318 0690 00000000 		lea.sl	%s56,_PVWORK_STATIC@GOT_HI(%s56,%got)
 318      8FB8B806 
 319 0698 00000000 		ld	%s56,(,%s56)
 319      B8003801 
 320 06a0 00080000 		lea	%s56,0x800(,%s56)
 320      B8003806 
 321 06a8 F8FFFFFF 		st	%s0,-8(,%fp)	# spill
 321      89000011 
 322 06b0 F0FFFFFF 		st	%s1,-16(,%fp)	# spill
 322      89000111 
 323 06b8 E8FFFFFF 		st	%s58,-24(,%fp)	# spill
 323      89003A11 
 324 06c0 E0FFFFFF 		st	%s57,-32(,%fp)	# spill
 324      89003911 
 325 06c8 D8FFFFFF 		st	%s56,-40(,%fp)	# spill
 325      89003811 
 326 06d0 08000000 		br.l	.L_4.11
 326      00000F18 
 327              	.L_4.11:
 328 06d8 00010000 		lea	%s63,256
 328      00003F06 
 329 06e0 00010000 		lea	%s62,256
 329      00003E06 
 330 06e8 00000000 		adds.w.sx	%s61,%s62,(0)1
 330      00BE3D4A 
 331 06f0 00000000 		lvl	%s61
 331      00BD00BF 
 332 06f8 0000003F 		vseq	%v63
 332      00000099 
 333 0700 003F003E 		vor	%v62,(0)1,%v63
 333      000020C5 
 334              	# line 41
  41:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****         group_sums[i/16] += a[i];
 335              		.loc	1 41 0
 336 0708 00000000 		lvl	%s63
 336      00BF00BF 
 337 0710 003E003D 		vadds.w.sx	%v61,0,%v62
 337      000020CA 
 338 0718 00003D3C 		vdivs.w.sx	%v60,%v61,16
 338      001010EB 
 339 0720 003C003B 		vor	%v59,(0)1,%v60
 339      000020C5 
 340 0728 003B003A 		vmuls.l	%v58,4,%v59
 340      000420DB 
 341 0730 E0FFFFFF 		ld	%s60,-32(,%fp)	# restore
 341      89003C01 
 342 0738 00000000 		or	%s59,%s60,(0)1
 342      00BC3B45 
 343 0740 0000003A 		vst.nc	%v58,8,%s59
 343      BB080091 
 344 0748 F8FFFFFF 		ld	%s58,-8(,%fp)	# restore
 344      89003A01 
 345 0750 00000000 		or	%s57,%s58,(0)1
 345      00BA3945 
 346 0758 00000039 		vldu.nc	%v57,4,%s57	# *(a)
 346      B9040082 
 347 0760 D8FFFFFF 		ld	%s56,-40(,%fp)	# restore
 347      89003801 
 348 0768 00000000 		or	%s55,%s56,(0)1
 348      00B83745 
 349 0770 00000039 		vstu.nc	%v57,4,%s55
 349      B7040092 
 350 0778 00010000 		lea	%s54,256
 350      00003606 
 351 0780 00000000 		or	%s53,0,(0)1
 351      00003545 
 352 0788 00000000 		or	%s52,0,(0)1
 352      00003445 
 353 0790 E0FFFFFF 		st	%s60,-32(,%fp)	# spill
 353      89003C11 
 354 0798 D8FFFFFF 		st	%s56,-40(,%fp)	# spill
 354      89003811 
 355 07a0 50FFFFFF 		st	%s52,-176(,%fp)	# spill
 355      89003411 
 356 07a8 48FFFFFF 		st	%s53,-184(,%fp)	# spill
 356      89003511 
 357 07b0 40FFFFFF 		st	%s54,-192(,%fp)	# spill
 357      89003611 
 358 07b8 08000000 		br.l	.L_4.15
 358      00000F18 
 359              	.L_4.15:
 360 07c0 E0FFFFFF 		ld	%s63,-32(,%fp)	# restore
 360      89003F01 
 361 07c8 50FFFFFF 		ld	%s62,-176(,%fp)	# restore
 361      89003E01 
 362 07d0 00000000 		ld	%s61,0(%s62,%s63)
 362      BFBE3D01 
 363 07d8 D8FFFFFF 		ld	%s60,-40(,%fp)	# restore
 363      89003C01 
 364 07e0 48FFFFFF 		ld	%s59,-184(,%fp)	# restore
 364      89003B01 
 365 07e8 00000000 		ldu	%s58,0(%s59,%s60)
 365      BCBB3A02 
 366              	# line 40
  40:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****         group_sums[i/16] += a[i];
 367              		.loc	1 40 0
 368              	# line 41
 369              		.loc	1 41 0
 370 07f0 F0FFFFFF 		ld	%s57,-16(,%fp)	# restore
 370      89003901 
 371 07f8 00000000 		ldu	%s56,0(%s61,%s57)	# *(group_sums)
 371      B9BD3802 
 372 0800 00000000 		fadd.s	%s55,%s56,%s58
 372      BAB8B74C 
 373 0808 00000000 		stu	%s55,0(%s61,%s57)	# *(group_sums)
 373      B9BD3712 
 374              	# line 40
  40:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****         group_sums[i/16] += a[i];
 375              		.loc	1 40 0
 376 0810 04000000 		lea	%s54,4
 376      00003606 
 377 0818 00000000 		adds.l	%s59,%s59,%s54
 377      B6BB3B59 
 378 0820 08000000 		lea	%s53,8
 378      00003506 
 379 0828 00000000 		adds.l	%s62,%s62,%s53
 379      B5BE3E59 
 380 0830 40FFFFFF 		ld	%s52,-192(,%fp)	# restore
 380      89003401 
 381 0838 00000000 		subs.l	%s52,%s52,(63)0
 381      7FB4345B 
 382 0840 10000000 		brlt.l	0,%s52,.L_4.17
 382      B4000218 
 383 0848 40000000 		br.l	.L_4.18
 383      00000F18 
 384              	.L_4.17:
 385 0850 F0FFFFFF 		st	%s57,-16(,%fp)	# spill
 385      89003911 
 386 0858 E0FFFFFF 		st	%s63,-32(,%fp)	# spill
 386      89003F11 
 387 0860 D8FFFFFF 		st	%s60,-40(,%fp)	# spill
 387      89003C11 
 388 0868 50FFFFFF 		st	%s62,-176(,%fp)	# spill
 388      89003E11 
 389 0870 48FFFFFF 		st	%s59,-184(,%fp)	# spill
 389      89003B11 
 390 0878 40FFFFFF 		st	%s52,-192(,%fp)	# spill
 390      89003411 
 391 0880 40FFFFFF 		br.l	.L_4.15
 391      00000F18 
 392              	.L_4.18:
 393 0888 E8FFFFFF 		ld	%s63,-24(,%fp)	# restore
 393      89003F01 
 394 0890 10000000 		brlt.l	0,%s63,.L_4.19
 394      BF000218 
 395 0898 E8000000 		br.l	.L_4.6
 395      00000F18 
 396              	.L_4.19:
 397 08a0 00F4FFFF 		lea	%s62,-3072
 397      00003E06 
 398 08a8 00000000 		or	%s0,0,%s62
 398      BE000045 
 399 08b0 00000000 		lea	%s12,__grow_stack@PLT_LO(-24)
 399      00680C06 
 400 08b8 00000000 		and	%s12,%s12,(32)0
 400      608C0C44 
 401 08c0 00000000 		sic	%lr
 401      00000A28 
 402 08c8 00000000 		lea.sl	%s12,__grow_stack@PLT_HI(%s12,%lr)
 402      8A8C8C06 
 403 08d0 00000000 		bsic	%lr,(,%s12)		# __grow_stack
 403      8C000A08 
 404 08d8 A8000000 		br.l	.L_4.6
 404      00000F18 
 405              	.L_4.9:
 406 08e0 00000000 		or	%s63,1,(0)1
 406      00013F45 
 407 08e8 000C0000 		lea	%s62,3072
 407      00003E06 
 408 08f0 F8FFFFFF 		st	%s0,-8(,%fp)	# spill
 408      89000011 
 409 08f8 00000000 		or	%s0,0,%s62
 409      BE000045 
 410 0900 F0FFFFFF 		st	%s1,-16(,%fp)	# spill
 410      89000111 
 411 0908 E8FFFFFF 		st	%s63,-24(,%fp)	# spill
 411      89003F11 
 412 0910 00000000 		lea	%s12,__grow_stack@PLT_LO(-24)
 412      00680C06 
 413 0918 00000000 		and	%s12,%s12,(32)0
 413      608C0C44 
 414 0920 00000000 		sic	%lr
 414      00000A28 
 415 0928 00000000 		lea.sl	%s12,__grow_stack@PLT_HI(%s12,%lr)
 415      8A8C8C06 
 416 0930 00000000 		bsic	%lr,(,%s12)		# __grow_stack
 416      8C000A08 
 417              	.L_4.12:
 418 0938 B8000000 		lea	%s63,184
 418      00003F06 
 419 0940 00000000 		adds.l	%s62,%sp,%s63
 419      BF8B3E59 
 420 0948 00000000 		or	%s61,%s62,(0)1
 420      00BE3D45 
 421 0950 00080000 		lea	%s60,2048
 421      00003C06 
 422 0958 00000000 		adds.l	%s59,%s62,%s60
 422      BCBE3B59 
 423 0960 00000000 		or	%s58,%s59,(0)1
 423      00BB3A45 
 424 0968 E0FFFFFF 		st	%s61,-32(,%fp)	# spill
 424      89003D11 
 425 0970 D8FFFFFF 		st	%s58,-40(,%fp)	# spill
 425      89003A11 
 426 0978 60FDFFFF 		br.l	.L_4.11
 426      00000F18 
 427              	.L_4.6:
 428 0980 B8000000 		lea	%s63,184
 428      00003F06 
 429 0988 00000000 		adds.l	%s62,%sp,%s63
 429      BF8B3E59 
 430              	# line 43
  42:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****     }
  43:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c **** }
 431              		.loc	1 43 0
 432              	# Start of epilogue codes
 433 0990 00000000 		or	%sp,0,%fp
 433      89000B45 
 434              		.cfi_def_cfa	11,8
 435 0998 18000000 		ld	%got,0x18(,%sp)
 435      8B000F01 
 436 09a0 20000000 		ld	%plt,0x20(,%sp)
 436      8B001001 
 437 09a8 08000000 		ld	%lr,0x8(,%sp)
 437      8B000A01 
 438 09b0 00000000 		ld	%fp,0x0(,%sp)
 438      8B000901 
 439 09b8 00000000 		b.l	(,%lr)
 439      8A000F19 
 440              	.L_4.EoE:
 441              		.cfi_endproc
 442              		.set	.L.4.2auto_size,	0xfffffffffffffdc0	# 576 Bytes
 444              	# ============ End  v_reduce256g16_fp2 ============
 445              	# ============ Begin  v_diffs16_fp ============
 446              		.balign 16
 447              	.L_5.0:
 448              	# line 45
  44:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c **** 
  45:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c **** void v_diffs16_fp( float* restrict a ){
 449              		.loc	1 45 0
 450              		.globl	v_diffs16_fp
 452              	v_diffs16_fp:
 453              		.cfi_startproc
 454 09c0 00000000 		st	%fp,0x0(,%sp)
 454      8B000911 
 455              		.cfi_def_cfa_offset	0
 456              		.cfi_offset	9,0
 457 09c8 08000000 		st	%lr,0x8(,%sp)
 457      8B000A11 
 458 09d0 18000000 		st	%got,0x18(,%sp)
 458      8B000F11 
 459 09d8 20000000 		st	%plt,0x20(,%sp)
 459      8B001011 
 460 09e0 00000000 		lea	%got,_GLOBAL_OFFSET_TABLE_@PC_LO(-0x18)
 460      00680F06 
 461 09e8 00000000 		and	%got,%got,(32)0
 461      608F0F44 
 462 09f0 00000000 		sic	%plt
 462      00001028 
 463 09f8 00000000 		lea.sl	%got,_GLOBAL_OFFSET_TABLE_@PC_HI(%plt,%got)
 463      8F908F06 
 464 0a00 00000000 		or	%fp,0,%sp
 464      8B000945 
 465              		.cfi_def_cfa_register	9
 466 0a08 30000000 		st	%s18,48(,%fp)
 466      89001211 
 467 0a10 38000000 		st	%s19,56(,%fp)
 467      89001311 
 468 0a18 40000000 		st	%s20,64(,%fp)
 468      89001411 
 469 0a20 48000000 		st	%s21,72(,%fp)
 469      89001511 
 470 0a28 50000000 		st	%s22,80(,%fp)
 470      89001611 
 471 0a30 58000000 		st	%s23,88(,%fp)
 471      89001711 
 472 0a38 60000000 		st	%s24,96(,%fp)
 472      89001811 
 473 0a40 68000000 		st	%s25,104(,%fp)
 473      89001911 
 474 0a48 70000000 		st	%s26,112(,%fp)
 474      89001A11 
 475 0a50 78000000 		st	%s27,120(,%fp)
 475      89001B11 
 476 0a58 80000000 		st	%s28,128(,%fp)
 476      89001C11 
 477 0a60 88000000 		st	%s29,136(,%fp)
 477      89001D11 
 478 0a68 90000000 		st	%s30,144(,%fp)
 478      89001E11 
 479 0a70 98000000 		st	%s31,152(,%fp)
 479      89001F11 
 480 0a78 A0000000 		st	%s32,160(,%fp)
 480      89002011 
 481 0a80 A8000000 		st	%s33,168(,%fp)
 481      89002111 
 482 0a88 00000000 		lea	%s13,.L.5.2auto_size&0xffffffff
 482      00000D06 
 483 0a90 00000000 		and	%s13,%s13,(32)0
 483      608D0D44 
 484 0a98 00000000 		lea.sl	%sp,.L.5.2auto_size>>32(%fp,%s13)
 484      8D898B06 
 485 0aa0 48000000 		brge.l.t	%sp,%sl,.L_5.EoP
 485      888B3518 
 486 0aa8 18000000 		ld	%s61,0x18(,%tp)
 486      8E003D01 
 487 0ab0 00000000 		or	%s62,0,%s0
 487      80003E45 
 488 0ab8 3B010000 		lea	%s63,0x13b
 488      00003F06 
 489 0ac0 00000000 		shm.l	%s63,0x0(%s61)
 489      BD033F31 
 490 0ac8 08000000 		shm.l	%sl,0x8(%s61)
 490      BD030831 
 491 0ad0 10000000 		shm.l	%sp,0x10(%s61)
 491      BD030B31 
 492 0ad8 00000000 		monc
 492      0000003F 
 493 0ae0 00000000 		or	%s0,0,%s62
 493      BE000045 
 494              	.L_5.EoP:
 495              	# End of prologue codes
 496              	# line 57
  46:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****     // adds.l  %s63,%s0,(62)1   ## s63 = &a[-1]  # 62(1) is -4, sizeof(float)=4
  47:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****     // lea     %s62,4
  48:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****     // adds.l  %s61,%s63,%s62   ## s61 = &a[0]
  49:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****     // or      %s60,15,(0)1     ## s60 = 15
  50:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****     // lea     %s59,4
  51:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****     // adds.l  %s58,%s61,%s59   ## s58 = &a[1]
  52:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****     // lvl     %s60             ## vector length 15
  53:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****     // vldu.nc %v63,4,%s58     # *(a+1) -- vector[1..15] (stride=sizeof(float))
  54:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****     // ldu     %s57,0(0,%s61)  # *(a)   -- scalar
  55:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****     // vfis.s  %v62,%v63,%s57
  56:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****     // vstu.nc %v62,4,%s58     # *(a)
  57:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****     for(int i=1; i<16; ++i){
 497              		.loc	1 57 0
 498 0ae8 00000000 		adds.l	%s63,%s0,(62)1
 498      3E803F59 
 499 0af0 04000000 		lea	%s62,4
 499      00003E06 
 500 0af8 00000000 		adds.l	%s61,%s63,%s62
 500      BEBF3D59 
 501 0b00 00000000 		or	%s60,15,(0)1
 501      000F3C45 
 502              	# line 58
  58:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****         a[i] = a[i] - a[i-1];
 503              		.loc	1 58 0
 504 0b08 04000000 		lea	%s59,4
 504      00003B06 
 505 0b10 00000000 		adds.l	%s58,%s61,%s59
 505      BBBD3A59 
 506 0b18 00000000 		lvl	%s60
 506      00BC00BF 
 507 0b20 0000003F 		vldu.nc	%v63,4,%s58	# *(a)
 507      BA040082 
 508 0b28 00000000 		ldu	%s57,0(0,%s61)	# *(a)
 508      BD003902 
 509 0b30 00003F3E 		vfis.s	%v62,%v63,%s57
 509      00B980DE 
 510 0b38 00000000 		or	%s56,%s58,(0)1
 510      00BA3845 
 511 0b40 0000003E 		vstu.nc	%v62,4,%s56	# *(a)
 511      B8040092 
 512              	# line 60
  59:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****     }
  60:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c **** }
 513              		.loc	1 60 0
 514              	# Start of epilogue codes
 515 0b48 00000000 		or	%sp,0,%fp
 515      89000B45 
 516              		.cfi_def_cfa	11,8
 517 0b50 18000000 		ld	%got,0x18(,%sp)
 517      8B000F01 
 518 0b58 20000000 		ld	%plt,0x20(,%sp)
 518      8B001001 
 519 0b60 08000000 		ld	%lr,0x8(,%sp)
 519      8B000A01 
 520 0b68 00000000 		ld	%fp,0x0(,%sp)
 520      8B000901 
 521 0b70 00000000 		b.l	(,%lr)
 521      8A000F19 
 522              	.L_5.EoE:
 523              		.cfi_endproc
 524              		.set	.L.5.2auto_size,	0x0	# 0 Bytes
 526              	# ============ End  v_diffs16_fp ============
 527              	# ============ Begin  v_every16th_fp2 ============
 528 0b78 00000000 		.balign 16
 528      00000000 
 529              	.L_6.0:
 530              	# line 61
  61:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c **** void v_every16th_fp2( float const* restrict a, float*restrict every){
 531              		.loc	1 61 0
 532              		.globl	v_every16th_fp2
 534              	v_every16th_fp2:
 535              		.cfi_startproc
 536 0b80 00000000 		st	%fp,0x0(,%sp)
 536      8B000911 
 537              		.cfi_def_cfa_offset	0
 538              		.cfi_offset	9,0
 539 0b88 08000000 		st	%lr,0x8(,%sp)
 539      8B000A11 
 540 0b90 18000000 		st	%got,0x18(,%sp)
 540      8B000F11 
 541 0b98 20000000 		st	%plt,0x20(,%sp)
 541      8B001011 
 542 0ba0 00000000 		lea	%got,_GLOBAL_OFFSET_TABLE_@PC_LO(-0x18)
 542      00680F06 
 543 0ba8 00000000 		and	%got,%got,(32)0
 543      608F0F44 
 544 0bb0 00000000 		sic	%plt
 544      00001028 
 545 0bb8 00000000 		lea.sl	%got,_GLOBAL_OFFSET_TABLE_@PC_HI(%plt,%got)
 545      8F908F06 
 546 0bc0 00000000 		or	%fp,0,%sp
 546      8B000945 
 547              		.cfi_def_cfa_register	9
 548 0bc8 30000000 		st	%s18,48(,%fp)
 548      89001211 
 549 0bd0 38000000 		st	%s19,56(,%fp)
 549      89001311 
 550 0bd8 40000000 		st	%s20,64(,%fp)
 550      89001411 
 551 0be0 48000000 		st	%s21,72(,%fp)
 551      89001511 
 552 0be8 50000000 		st	%s22,80(,%fp)
 552      89001611 
 553 0bf0 58000000 		st	%s23,88(,%fp)
 553      89001711 
 554 0bf8 60000000 		st	%s24,96(,%fp)
 554      89001811 
 555 0c00 68000000 		st	%s25,104(,%fp)
 555      89001911 
 556 0c08 70000000 		st	%s26,112(,%fp)
 556      89001A11 
 557 0c10 78000000 		st	%s27,120(,%fp)
 557      89001B11 
 558 0c18 80000000 		st	%s28,128(,%fp)
 558      89001C11 
 559 0c20 88000000 		st	%s29,136(,%fp)
 559      89001D11 
 560 0c28 90000000 		st	%s30,144(,%fp)
 560      89001E11 
 561 0c30 98000000 		st	%s31,152(,%fp)
 561      89001F11 
 562 0c38 A0000000 		st	%s32,160(,%fp)
 562      89002011 
 563 0c40 A8000000 		st	%s33,168(,%fp)
 563      89002111 
 564 0c48 00000000 		lea	%s13,.L.6.2auto_size&0xffffffff
 564      00000D06 
 565 0c50 00000000 		and	%s13,%s13,(32)0
 565      608D0D44 
 566 0c58 00000000 		lea.sl	%sp,.L.6.2auto_size>>32(%fp,%s13)
 566      8D898B06 
 567 0c60 48000000 		brge.l.t	%sp,%sl,.L_6.EoP
 567      888B3518 
 568 0c68 18000000 		ld	%s61,0x18(,%tp)
 568      8E003D01 
 569 0c70 00000000 		or	%s62,0,%s0
 569      80003E45 
 570 0c78 3B010000 		lea	%s63,0x13b
 570      00003F06 
 571 0c80 00000000 		shm.l	%s63,0x0(%s61)
 571      BD033F31 
 572 0c88 08000000 		shm.l	%sl,0x8(%s61)
 572      BD030831 
 573 0c90 10000000 		shm.l	%sp,0x10(%s61)
 573      BD030B31 
 574 0c98 00000000 		monc
 574      0000003F 
 575 0ca0 00000000 		or	%s0,0,%s62
 575      BE000045 
 576              	.L_6.EoP:
 577              	# End of prologue codes
 578              	# line 67
  62:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****     // or      %s63,16,(0)1
  63:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****     // lvl     %s63             # vector length 16
  64:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****     // or      %s62,%s0,(0)1    # s62 = &a[0]
  65:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****     // lea     %s61,64          # 16*sizeof(float)
  66:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****     // vldu.nc %v63,%s61,%s62  # *(a)  v63 = load( stride=64, &a[0] )
  67:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****     for(int i=0; i<16; ++i)
 579              		.loc	1 67 0
 580 0ca8 00000000 		or	%s63,16,(0)1
 580      00103F45 
 581              	# line 68
  68:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****         every[i] = a[16*i];
 582              		.loc	1 68 0
 583 0cb0 00000000 		or	%s62,%s0,(0)1
 583      00803E45 
 584 0cb8 40000000 		lea	%s61,64
 584      00003D06 
 585 0cc0 00000000 		lvl	%s63
 585      00BF00BF 
 586 0cc8 0000003F 		vldu.nc	%v63,%s61,%s62	# *(a)
 586      BEBD0082 
 587 0cd0 00000000 		or	%s60,%s1,(0)1
 587      00813C45 
 588 0cd8 0000003F 		vstu.nc	%v63,4,%s60	# *(every)
 588      BC040092 
 589              	# line 69
  69:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c **** }
 590              		.loc	1 69 0
 591              	# Start of epilogue codes
 592 0ce0 00000000 		or	%sp,0,%fp
 592      89000B45 
 593              		.cfi_def_cfa	11,8
 594 0ce8 18000000 		ld	%got,0x18(,%sp)
 594      8B000F01 
 595 0cf0 20000000 		ld	%plt,0x20(,%sp)
 595      8B001001 
 596 0cf8 08000000 		ld	%lr,0x8(,%sp)
 596      8B000A01 
 597 0d00 00000000 		ld	%fp,0x0(,%sp)
 597      8B000901 
 598 0d08 00000000 		b.l	(,%lr)
 598      8A000F19 
 599              	.L_6.EoE:
 600              		.cfi_endproc
 601              		.set	.L.6.2auto_size,	0x0	# 0 Bytes
 603              	# ============ End  v_every16th_fp2 ============
 604              	# ============ Begin  v_sumg16_fp2 ============
 605              		.balign 16
 606              	.L_7.0:
 607              	# line 71
  70:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****     
  71:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c **** void v_sumg16_fp2(float const* restrict a, float*restrict group_sums){
 608              		.loc	1 71 0
 609              		.globl	v_sumg16_fp2
 611              	v_sumg16_fp2:
 612              		.cfi_startproc
 613 0d10 00000000 		st	%fp,0x0(,%sp)
 613      8B000911 
 614              		.cfi_def_cfa_offset	0
 615              		.cfi_offset	9,0
 616 0d18 08000000 		st	%lr,0x8(,%sp)
 616      8B000A11 
 617 0d20 18000000 		st	%got,0x18(,%sp)
 617      8B000F11 
 618 0d28 20000000 		st	%plt,0x20(,%sp)
 618      8B001011 
 619 0d30 00000000 		lea	%got,_GLOBAL_OFFSET_TABLE_@PC_LO(-0x18)
 619      00680F06 
 620 0d38 00000000 		and	%got,%got,(32)0
 620      608F0F44 
 621 0d40 00000000 		sic	%plt
 621      00001028 
 622 0d48 00000000 		lea.sl	%got,_GLOBAL_OFFSET_TABLE_@PC_HI(%plt,%got)
 622      8F908F06 
 623 0d50 00000000 		or	%fp,0,%sp
 623      8B000945 
 624              		.cfi_def_cfa_register	9
 625 0d58 30000000 		st	%s18,48(,%fp)
 625      89001211 
 626 0d60 38000000 		st	%s19,56(,%fp)
 626      89001311 
 627 0d68 40000000 		st	%s20,64(,%fp)
 627      89001411 
 628 0d70 48000000 		st	%s21,72(,%fp)
 628      89001511 
 629 0d78 50000000 		st	%s22,80(,%fp)
 629      89001611 
 630 0d80 58000000 		st	%s23,88(,%fp)
 630      89001711 
 631 0d88 60000000 		st	%s24,96(,%fp)
 631      89001811 
 632 0d90 68000000 		st	%s25,104(,%fp)
 632      89001911 
 633 0d98 70000000 		st	%s26,112(,%fp)
 633      89001A11 
 634 0da0 78000000 		st	%s27,120(,%fp)
 634      89001B11 
 635 0da8 80000000 		st	%s28,128(,%fp)
 635      89001C11 
 636 0db0 88000000 		st	%s29,136(,%fp)
 636      89001D11 
 637 0db8 90000000 		st	%s30,144(,%fp)
 637      89001E11 
 638 0dc0 98000000 		st	%s31,152(,%fp)
 638      89001F11 
 639 0dc8 A0000000 		st	%s32,160(,%fp)
 639      89002011 
 640 0dd0 A8000000 		st	%s33,168(,%fp)
 640      89002111 
 641 0dd8 00FCFFFF 		lea	%s13,.L.7.2auto_size&0xffffffff
 641      00000D06 
 642 0de0 00000000 		and	%s13,%s13,(32)0
 642      608D0D44 
 643 0de8 FFFFFFFF 		lea.sl	%sp,.L.7.2auto_size>>32(%fp,%s13)
 643      8D898B06 
 644 0df0 48000000 		brge.l.t	%sp,%sl,.L_7.EoP
 644      888B3518 
 645 0df8 18000000 		ld	%s61,0x18(,%tp)
 645      8E003D01 
 646 0e00 00000000 		or	%s62,0,%s0
 646      80003E45 
 647 0e08 3B010000 		lea	%s63,0x13b
 647      00003F06 
 648 0e10 00000000 		shm.l	%s63,0x0(%s61)
 648      BD033F31 
 649 0e18 08000000 		shm.l	%sl,0x8(%s61)
 649      BD030831 
 650 0e20 10000000 		shm.l	%sp,0x10(%s61)
 650      BD030B31 
 651 0e28 00000000 		monc
 651      0000003F 
 652 0e30 00000000 		or	%s0,0,%s62
 652      BE000045 
 653              	.L_7.EoP:
 654              	# End of prologue codes
 655              	# line 84
  72:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****     float part_sums[256];
  73:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****     register float part_sum = 0.f;
  74:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****     // lea     %s63,128
  75:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****     // lvl     %s63             # vec length 128
  76:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****     // or      %s62,%s0,(0)1
  77:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****     // vldu.nc %v63,8,%s62      # v63: *(a) a[0..128) by 2   even values
  78:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****     // lea     %s61,4
  79:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****     // adds.l  %s60,%s0,%s61
  80:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****     // vldu.nc %v62,8,%s60      # v62: *(a) a[1..128) by 2   odd values
  81:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****     // vfadd.s %v61,%v63,%v62   # v61: pair_sums [0..128)
  82:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****     // or      %s59,0,(0)1      # s59: "pair_sums[-1]"
  83:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****     // vfia.s  %v60,%v61,%s59   # v60: pair_sums[i] + pair_sums[i-1] (now incremental sum)
  84:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****     for(int i=0; i<256; ++i){
 656              		.loc	1 84 0
 657 0e38 80000000 		lea	%s63,128
 657      00003F06 
 658              	# line 85
  85:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****         part_sum += a[i];
 659              		.loc	1 85 0
 660 0e40 00000000 		or	%s62,%s0,(0)1
 660      00803E45 
 661 0e48 00000000 		lvl	%s63
 661      00BF00BF 
 662 0e50 0000003F 		vldu.nc	%v63,8,%s62	# *(a)
 662      BE080082 
 663 0e58 04000000 		lea	%s61,4
 663      00003D06 
 664 0e60 00000000 		adds.l	%s60,%s0,%s61
 664      BD803C59 
 665 0e68 0000003E 		vldu.nc	%v62,8,%s60	# *(a)
 665      BC080082 
 666 0e70 003E3F3D 		vfadd.s	%v61,%v63,%v62
 666      000080CC 
 667 0e78 00000000 		or	%s59,0,(0)1
 667      00003B45 
 668 0e80 00003D3C 		vfia.s	%v60,%v61,%s59
 668      00BB80CE 
 669              	# line 92
  86:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****         // vfsub.s %v59,%v60,%v62   # v59: cum_pair_sums[i] - odds[i] (de-interleaved partial sums,
  87:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****         // adds.l  %s58,%fp,(54)1
  88:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****         // vstu.nc %v59,8,%s58     # part_sums   -- store evens of part_sums
  89:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****         // lea     %s57,4
  90:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****         // adds.l  %s56,%s58,%s57
  91:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****         // vstu.nc %v60,8,%s56     # part_sums   -- store odds of part_sums
  92:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****         part_sums[i] = part_sum;
 670              		.loc	1 92 0
 671 0e88 003E3C3B 		vfsub.s	%v59,%v60,%v62
 671      000080DC 
 672 0e90 00000000 		adds.l	%s58,%fp,(54)1
 672      36893A59 
 673 0e98 0000003B 		vstu.nc	%v59,8,%s58	# part_sums
 673      BA080092 
 674 0ea0 04000000 		lea	%s57,4
 674      00003906 
 675 0ea8 00000000 		adds.l	%s56,%s58,%s57
 675      B9BA3859 
 676 0eb0 0000003C 		vstu.nc	%v60,8,%s56	# part_sums
 676      B8080092 
 677              	# line 94
  93:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****     }
  94:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****     for(int i=0; i<16; ++i){
 678              		.loc	1 94 0
 679 0eb8 00000000 		or	%s55,16,(0)1
 679      00103745 
 680              	# line 96
  95:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****         // ... vldu.nc %v58,%s53,%s54  # part_sums, s53=stride=64, s54=addr_of_part_sums (wrt %fp)
  96:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****         group_sums[i] = part_sums[16*i];
 681              		.loc	1 96 0
 682 0ec0 00000000 		adds.l	%s54,%fp,(54)1
 682      36893659 
 683 0ec8 40000000 		lea	%s53,64
 683      00003506 
 684 0ed0 00000000 		lvl	%s55
 684      00B700BF 
 685 0ed8 0000003A 		vldu.nc	%v58,%s53,%s54	# part_sums
 685      B6B50082 
 686 0ee0 00000000 		or	%s52,%s1,(0)1
 686      00813445 
 687 0ee8 0000003A 		vstu.nc	%v58,4,%s52	# *(group_sums)
 687      B4040092 
 688              	# line 98
  97:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****     }
  98:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c **** for(int i=1; i<16; ++i){
 689              		.loc	1 98 0
 690 0ef0 00000000 		adds.l	%s51,%s1,(62)1
 690      3E813359 
 691 0ef8 04000000 		lea	%s50,4
 691      00003206 
 692 0f00 00000000 		adds.l	%s49,%s51,%s50
 692      B2B33159 
 693 0f08 00000000 		or	%s48,15,(0)1
 693      000F3045 
 694              	# line 103
  99:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****         // adjust vector length
 100:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****         // ... vldu.nc %v57,4,%s46     # *(group_sums)
 101:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****         // ldu     %s45,0(0,%s49)  # *(group_sums) "group_sums[-1]"
 102:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****         // vfis.s  %v56,%v57,%s45  # v56:groups_sums[i=1..) = v57:group_sums[i] - s45:group_sums[i-
 103:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****         group_sums[i] -= group_sums[i-1];
 695              		.loc	1 103 0
 696 0f10 04000000 		lea	%s47,4
 696      00002F06 
 697 0f18 00000000 		adds.l	%s46,%s49,%s47
 697      AFB12E59 
 698 0f20 00000000 		lvl	%s48
 698      00B000BF 
 699 0f28 00000039 		vldu.nc	%v57,4,%s46	# *(group_sums)
 699      AE040082 
 700 0f30 00000000 		ldu	%s45,0(0,%s49)	# *(group_sums)
 700      B1002D02 
 701 0f38 00003938 		vfis.s	%v56,%v57,%s45
 701      00AD80DE 
 702 0f40 00000000 		or	%s44,%s46,(0)1
 702      00AE2C45 
 703 0f48 00000038 		vstu.nc	%v56,4,%s44	# *(group_sums)
 703      AC040092 
 704              	# line 105
 104:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****     }
 105:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c **** }
 705              		.loc	1 105 0
 706              	# Start of epilogue codes
 707 0f50 00000000 		or	%sp,0,%fp
 707      89000B45 
 708              		.cfi_def_cfa	11,8
 709 0f58 18000000 		ld	%got,0x18(,%sp)
 709      8B000F01 
 710 0f60 20000000 		ld	%plt,0x20(,%sp)
 710      8B001001 
 711 0f68 08000000 		ld	%lr,0x8(,%sp)
 711      8B000A01 
 712 0f70 00000000 		ld	%fp,0x0(,%sp)
 712      8B000901 
 713 0f78 00000000 		b.l	(,%lr)
 713      8A000F19 
 714              	.L_7.EoE:
 715              		.cfi_endproc
 716              		.set	.L.7.2auto_size,	0xfffffffffffffc00	# 1024 Bytes
 718              	# ============ End  v_sumg16_fp2 ============
 719              	# ============ Begin  v_sumg16x_fp2 ============
 720              		.balign 16
 721              	.L_8.0:
 722              	# line 106
 106:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c **** void v_sumg16x_fp2(float const* restrict a, float*restrict group_sums){
 723              		.loc	1 106 0
 724              		.globl	v_sumg16x_fp2
 726              	v_sumg16x_fp2:
 727              		.cfi_startproc
 728 0f80 00000000 		st	%fp,0x0(,%sp)
 728      8B000911 
 729              		.cfi_def_cfa_offset	0
 730              		.cfi_offset	9,0
 731 0f88 08000000 		st	%lr,0x8(,%sp)
 731      8B000A11 
 732 0f90 18000000 		st	%got,0x18(,%sp)
 732      8B000F11 
 733 0f98 20000000 		st	%plt,0x20(,%sp)
 733      8B001011 
 734 0fa0 00000000 		lea	%got,_GLOBAL_OFFSET_TABLE_@PC_LO(-0x18)
 734      00680F06 
 735 0fa8 00000000 		and	%got,%got,(32)0
 735      608F0F44 
 736 0fb0 00000000 		sic	%plt
 736      00001028 
 737 0fb8 00000000 		lea.sl	%got,_GLOBAL_OFFSET_TABLE_@PC_HI(%plt,%got)
 737      8F908F06 
 738 0fc0 00000000 		or	%fp,0,%sp
 738      8B000945 
 739              		.cfi_def_cfa_register	9
 740 0fc8 30000000 		st	%s18,48(,%fp)
 740      89001211 
 741 0fd0 38000000 		st	%s19,56(,%fp)
 741      89001311 
 742 0fd8 40000000 		st	%s20,64(,%fp)
 742      89001411 
 743 0fe0 48000000 		st	%s21,72(,%fp)
 743      89001511 
 744 0fe8 50000000 		st	%s22,80(,%fp)
 744      89001611 
 745 0ff0 58000000 		st	%s23,88(,%fp)
 745      89001711 
 746 0ff8 60000000 		st	%s24,96(,%fp)
 746      89001811 
 747 1000 68000000 		st	%s25,104(,%fp)
 747      89001911 
 748 1008 70000000 		st	%s26,112(,%fp)
 748      89001A11 
 749 1010 78000000 		st	%s27,120(,%fp)
 749      89001B11 
 750 1018 80000000 		st	%s28,128(,%fp)
 750      89001C11 
 751 1020 88000000 		st	%s29,136(,%fp)
 751      89001D11 
 752 1028 90000000 		st	%s30,144(,%fp)
 752      89001E11 
 753 1030 98000000 		st	%s31,152(,%fp)
 753      89001F11 
 754 1038 A0000000 		st	%s32,160(,%fp)
 754      89002011 
 755 1040 A8000000 		st	%s33,168(,%fp)
 755      89002111 
 756 1048 80EFFFFF 		lea	%s13,.L.8.2auto_size&0xffffffff
 756      00000D06 
 757 1050 00000000 		and	%s13,%s13,(32)0
 757      608D0D44 
 758 1058 FFFFFFFF 		lea.sl	%sp,.L.8.2auto_size>>32(%fp,%s13)
 758      8D898B06 
 759 1060 48000000 		brge.l.t	%sp,%sl,.L_8.EoP
 759      888B3518 
 760 1068 18000000 		ld	%s61,0x18(,%tp)
 760      8E003D01 
 761 1070 00000000 		or	%s62,0,%s0
 761      80003E45 
 762 1078 3B010000 		lea	%s63,0x13b
 762      00003F06 
 763 1080 00000000 		shm.l	%s63,0x0(%s61)
 763      BD033F31 
 764 1088 08000000 		shm.l	%sl,0x8(%s61)
 764      BD030831 
 765 1090 10000000 		shm.l	%sp,0x10(%s61)
 765      BD030B31 
 766 1098 00000000 		monc
 766      0000003F 
 767 10a0 00000000 		or	%s0,0,%s62
 767      BE000045 
 768              	.L_8.EoP:
 769              	# End of prologue codes
 770              	# line 113
 107:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****     //float load[512];
 108:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****     float sums[512];
 109:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****     register float rt_sum = 0.f;
 110:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****     // lea     %s63,256
 111:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****     // lvl     %s63                 # vector length 256
 112:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****     // or      %s62,%s0,(0)1        # s62 = &a[0]
 113:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****     for(int i=0; i<256; ++i){
 771              		.loc	1 113 0
 772 10a8 00010000 		lea	%s63,256
 772      00003F06 
 773              	# line 115
 114:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****         // vldu.nc %v63,4,%s62     # *(a)  load packed float into v63.hi
 115:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****         sums[2*i] = a[i];
 774              		.loc	1 115 0
 775 10b0 00000000 		or	%s62,%s0,(0)1
 775      00803E45 
 776 10b8 00000000 		lvl	%s63
 776      00BF00BF 
 777 10c0 0000003F 		vldu.nc	%v63,4,%s62	# *(a)
 777      BE040082 
 778              	# line 118
 116:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****         // or      %s60,0,(0)1      # v63[-1] = 0.
 117:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****         // vfia.s  %v62,%v63,%s60   # v62[i] = v63[i-1] + v63[i]
 118:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****         rt_sum += a[i];
 779              		.loc	1 118 0
 780 10c8 00000000 		or	%s61,0,(0)1
 780      00003D45 
 781 10d0 00003F3E 		vfia.s	%v62,%v63,%s61
 781      00BD80CE 
 782              	# line 121
 119:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****         // lea/and/lea.sl/adds.l
 120:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****         // vstu.nnc %v62,8,%s57
 121:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****         sums[2*i] = rt_sum;
 783              		.loc	1 121 0
 784 10d8 80F7FFFF 		lea	%s60,-2176
 784      00003C06 
 785 10e0 00000000 		and	%s59,%s60,(32)0
 785      60BC3B44 
 786 10e8 FFFFFFFF 		lea.sl	%s58,-1(,%s59)
 786      BB00BA06 
 787 10f0 00000000 		adds.l	%s57,%fp,%s58
 787      BA893959 
 788 10f8 0000003E 		vstu.nc	%v62,8,%s57	# sums
 788      B9080092 
 789              	# line 124
 122:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****     }
 123:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****     unsigned long msk[256];
 124:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****     for(int i=0; i<256; ++i){
 790              		.loc	1 124 0
 791 1100 00010000 		lea	%s56,256
 791      00003806 
 792 1108 00010000 		lea	%s55,256
 792      00003706 
 793 1110 00000000 		adds.w.sx	%s54,%s55,(0)1
 793      00B7364A 
 794 1118 00000000 		lvl	%s54
 794      00B600BF 
 795 1120 0000003D 		vseq	%v61
 795      00000099 
 796 1128 003D003C 		vor	%v60,(0)1,%v61
 796      000020C5 
 797              	# line 145
 125:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****         // lea     %s56,256
 126:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****         // lea     %s55,256
 127:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****         // adds.w.sx       %s54,%s55,(0)1 # s54 = s55 = s56 = 256
 128:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****         // lvl     %s54                 # vector length 256
 129:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****         // vseq    %v61
 130:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****         // vor     %v60,(0)1,%v61       # v60 = v61 = 0,1,2,3...255
 131:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****         // lvl     %s56                 # vector length 256
 132:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****         // vadds.w.sx      %v59,1,%v60  # v60[i] = i+1, for i=0..255
 133:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****         // vand    %v58,(60)0,%v59      # v58    = v59 & 0x0F
 134:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****         // vcmps.w.sx      %v57,0,%v58  # v57 = (v58==0) (isn't this superfluous?)
 135:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****         // vfmk.w.eq       %vm1,%v57    # vm1[i] = (v57[i]==0)
 136:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****         // vbrd    %v56,0,%vm1          # for(i=0..255)     if(vm1[i]) v56[i] = 0 ;
 137:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****         // negm    %vm2,%vm1            # vm2 = -vm1        else
 138:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****         // vbrd    %v56,1,%vm2          # v56[i] = vm2==1   v56[i] = 1;
 139:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****         // vor     %v55,(0)1,%v56       # v55 = v56 (bloat)
 140:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****         // lea     %s53,-4224
 141:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****         // and     %s52,%s53,(32)0
 142:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****         // lea.sl  %s51,-1(,%s52)
 143:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****         // adds.l  %s50,%fp,%s51        # s50 = &msk[0]
 144:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****         // vst.nc  %v55,8,%s50          # *(s50) = v55
 145:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****         msk[i] = ((i + 1) & 0x0f)? 1: 0;
 798              		.loc	1 145 0
 799 1130 00000000 		lvl	%s56
 799      00B800BF 
 800 1138 003C003B 		vadds.w.sx	%v59,1,%v60
 800      000120CA 
 801 1140 003B003A 		vand	%v58,(60)0,%v59
 801      007C20C4 
 802 1148 003A0039 		vcmps.w.sx	%v57,0,%v58
 802      000020FA 
 803 1150 00390401 		vfmk.w.eq	%vm1,%v57
 803      000000B5 
 804 1158 00000038 		vbrd	%v56,0,%vm1
 804      0000018C 
 805 1160 00000102 		negm	%vm2,%vm1
 805      00000095 
 806 1168 00000038 		vbrd	%v56,1,%vm2
 806      0001028C 
 807 1170 00380037 		vor	%v55,(0)1,%v56
 807      000020C5 
 808 1178 80EFFFFF 		lea	%s53,-4224
 808      00003506 
 809 1180 00000000 		and	%s52,%s53,(32)0
 809      60B53444 
 810 1188 FFFFFFFF 		lea.sl	%s51,-1(,%s52)
 810      B400B306 
 811 1190 00000000 		adds.l	%s50,%fp,%s51
 811      B3893259 
 812 1198 00000037 		vst.nc	%v55,8,%s50	# msk
 812      B2080091 
 813              	# line 164
 146:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****     }
 147:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****     float grps[32];
 148:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****     int j=0;
 149:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****     for(int i=0; i<256; ++i){
 150:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****         // horrible
 151:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****         // ...                              # v54 = * &msk[0]
 152:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****         // vcmpu.l %v53,0,%v54
 153:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****         // vfmk.l.ne       %vm3,%v53        # vm3 = msk[i]==0
 154:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****         // pcvm    %s44,%vm3                # s44 = popcnt(msk)
 155:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****         // many "spill" ...
 156:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****         if( msk[i] ){
 157:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****             // ouch. ...
 158:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****             // vldu.nc %v52,8,%s40          # sums
 159:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****             // tovm    %s39,%vm3
 160:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****             // subs.w.sx       %s38,%s39,(63)0
 161:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****             // lvs     %s37,%v52(%s38)
 162:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****             // stu     %s37,-128(0,%fp)     # grps
 163:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****             grps[2*j] = sums[2*i];
 164:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****             ++j;
 814              		.loc	1 164 0
 815 11a0 00010000 		lea	%s49,256
 815      00003106 
 816              	# line 156
 156:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****             // ouch. ...
 817              		.loc	1 156 0
 818 11a8 00000000 		or	%s48,%s50,(0)1
 818      00B23045 
 819 11b0 00000000 		lvl	%s49
 819      00B100BF 
 820 11b8 00000036 		vld.nc	%v54,8,%s48	# msk
 820      B0080081 
 821 11c0 00360035 		vcmpu.l	%v53,0,%v54
 821      000020B9 
 822 11c8 00350303 		vfmk.l.ne	%vm3,%v53
 822      000000B4 
 823 11d0 00000300 		pcvm	%s47,%vm3
 823      00002FA4 
 824              	# line 163
 163:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****             ++j;
 825              		.loc	1 163 0
 826 11d8 00000000 		or	%s46,%s57,(0)1
 826      00B92E45 
 827 11e0 00000034 		vldu.nc	%v52,8,%s46	# sums
 827      AE080082 
 828 11e8 00340033 		vcp	%v51,%v52,%vm3
 828      0000038D 
 829 11f0 00000000 		adds.l	%s45,%fp,(57)1
 829      39892D59 
 830 11f8 00000000 		lvl	%s47
 830      00AF00BF 
 831 1200 00000033 		vstu.nc	%v51,8,%s45	# grps
 831      AD080092 
 832              	# line 167
 165:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****         }
 166:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****     }
 167:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****     for(int i=0; i<16; ++i){
 833              		.loc	1 167 0
 834 1208 00000000 		or	%s44,16,(0)1
 834      00102C45 
 835              	# line 174
 168:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****         // lvl 16
 169:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****         // adds.l  %s62,%fp,(57)1
 170:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****         // vldu.nc %v63,8,%s62     # grps
 171:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****         // ld      %s61,-4232(,%fp)        # restore
 172:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****         // or      %s60,%s61,(0)1
 173:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****         // vstu.nc %v63,4,%s60     # *(group_sums)
 174:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****         group_sums[i] = grps[2*i];
 836              		.loc	1 174 0
 837 1210 00000000 		or	%s43,%s45,(0)1
 837      00AD2B45 
 838 1218 00000000 		lvl	%s44
 838      00AC00BF 
 839 1220 00000032 		vldu.nc	%v50,8,%s43	# grps
 839      AB080082 
 840 1228 00000000 		or	%s42,%s1,(0)1
 840      00812A45 
 841 1230 00000032 		vstu.nc	%v50,4,%s42	# *(group_sums)
 841      AA040092 
 842              	# line 176
 175:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****     }
 176:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c **** }
 843              		.loc	1 176 0
 844              	# Start of epilogue codes
 845 1238 00000000 		or	%sp,0,%fp
 845      89000B45 
 846              		.cfi_def_cfa	11,8
 847 1240 18000000 		ld	%got,0x18(,%sp)
 847      8B000F01 
 848 1248 20000000 		ld	%plt,0x20(,%sp)
 848      8B001001 
 849 1250 08000000 		ld	%lr,0x8(,%sp)
 849      8B000A01 
 850 1258 00000000 		ld	%fp,0x0(,%sp)
 850      8B000901 
 851 1260 00000000 		b.l	(,%lr)
 851      8A000F19 
 852              	.L_8.EoE:
 853              		.cfi_endproc
 854              		.set	.L.8.2auto_size,	0xffffffffffffef80	# 4224 Bytes
 856              	# ============ End  v_sumg16x_fp2 ============
 857              	# ============ Begin  v_diffs16_dp ============
 858 1268 00000000 		.balign 16
 858      00000000 
 859              	.L_9.0:
 860              	# line 178
 177:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c **** 
 178:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c **** void v_diffs16_dp( double* restrict a ){
 861              		.loc	1 178 0
 862              		.globl	v_diffs16_dp
 864              	v_diffs16_dp:
 865              		.cfi_startproc
 866 1270 00000000 		st	%fp,0x0(,%sp)
 866      8B000911 
 867              		.cfi_def_cfa_offset	0
 868              		.cfi_offset	9,0
 869 1278 08000000 		st	%lr,0x8(,%sp)
 869      8B000A11 
 870 1280 18000000 		st	%got,0x18(,%sp)
 870      8B000F11 
 871 1288 20000000 		st	%plt,0x20(,%sp)
 871      8B001011 
 872 1290 00000000 		lea	%got,_GLOBAL_OFFSET_TABLE_@PC_LO(-0x18)
 872      00680F06 
 873 1298 00000000 		and	%got,%got,(32)0
 873      608F0F44 
 874 12a0 00000000 		sic	%plt
 874      00001028 
 875 12a8 00000000 		lea.sl	%got,_GLOBAL_OFFSET_TABLE_@PC_HI(%plt,%got)
 875      8F908F06 
 876 12b0 00000000 		or	%fp,0,%sp
 876      8B000945 
 877              		.cfi_def_cfa_register	9
 878 12b8 30000000 		st	%s18,48(,%fp)
 878      89001211 
 879 12c0 38000000 		st	%s19,56(,%fp)
 879      89001311 
 880 12c8 40000000 		st	%s20,64(,%fp)
 880      89001411 
 881 12d0 48000000 		st	%s21,72(,%fp)
 881      89001511 
 882 12d8 50000000 		st	%s22,80(,%fp)
 882      89001611 
 883 12e0 58000000 		st	%s23,88(,%fp)
 883      89001711 
 884 12e8 60000000 		st	%s24,96(,%fp)
 884      89001811 
 885 12f0 68000000 		st	%s25,104(,%fp)
 885      89001911 
 886 12f8 70000000 		st	%s26,112(,%fp)
 886      89001A11 
 887 1300 78000000 		st	%s27,120(,%fp)
 887      89001B11 
 888 1308 80000000 		st	%s28,128(,%fp)
 888      89001C11 
 889 1310 88000000 		st	%s29,136(,%fp)
 889      89001D11 
 890 1318 90000000 		st	%s30,144(,%fp)
 890      89001E11 
 891 1320 98000000 		st	%s31,152(,%fp)
 891      89001F11 
 892 1328 A0000000 		st	%s32,160(,%fp)
 892      89002011 
 893 1330 A8000000 		st	%s33,168(,%fp)
 893      89002111 
 894 1338 00000000 		lea	%s13,.L.9.2auto_size&0xffffffff
 894      00000D06 
 895 1340 00000000 		and	%s13,%s13,(32)0
 895      608D0D44 
 896 1348 00000000 		lea.sl	%sp,.L.9.2auto_size>>32(%fp,%s13)
 896      8D898B06 
 897 1350 48000000 		brge.l.t	%sp,%sl,.L_9.EoP
 897      888B3518 
 898 1358 18000000 		ld	%s61,0x18(,%tp)
 898      8E003D01 
 899 1360 00000000 		or	%s62,0,%s0
 899      80003E45 
 900 1368 3B010000 		lea	%s63,0x13b
 900      00003F06 
 901 1370 00000000 		shm.l	%s63,0x0(%s61)
 901      BD033F31 
 902 1378 08000000 		shm.l	%sl,0x8(%s61)
 902      BD030831 
 903 1380 10000000 		shm.l	%sp,0x10(%s61)
 903      BD030B31 
 904 1388 00000000 		monc
 904      0000003F 
 905 1390 00000000 		or	%s0,0,%s62
 905      BE000045 
 906              	.L_9.EoP:
 907              	# End of prologue codes
 908              	# line 190
 179:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****     // adds.l  %s63,%s0,(62)1   ## s63 = &a[-1]  # 62(1) is -4, sizeof(float)=4
 180:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****     // lea     %s62,4
 181:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****     // adds.l  %s61,%s63,%s62   ## s61 = &a[0]
 182:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****     // or      %s60,15,(0)1     ## s60 = 15
 183:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****     // lea     %s59,4
 184:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****     // adds.l  %s58,%s61,%s59   ## s58 = &a[1]
 185:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****     // lvl     %s60             ## vector length 15
 186:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****     // vldu.nc %v63,4,%s58     # *(a+1) -- vector[1..15] (stride=sizeof(float))
 187:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****     // ldu     %s57,0(0,%s61)  # *(a)   -- scalar
 188:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****     // vfis.s  %v62,%v63,%s57
 189:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****     // vstu.nc %v62,4,%s58     # *(a)
 190:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****     for(int i=1; i<16; ++i){
 909              		.loc	1 190 0
 910 1398 00000000 		adds.l	%s63,%s0,(61)1
 910      3D803F59 
 911 13a0 08000000 		lea	%s62,8
 911      00003E06 
 912 13a8 00000000 		adds.l	%s61,%s63,%s62
 912      BEBF3D59 
 913 13b0 00000000 		or	%s60,15,(0)1
 913      000F3C45 
 914              	# line 191
 191:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****         a[i] = a[i] - a[i-1];
 915              		.loc	1 191 0
 916 13b8 08000000 		lea	%s59,8
 916      00003B06 
 917 13c0 00000000 		adds.l	%s58,%s61,%s59
 917      BBBD3A59 
 918 13c8 00000000 		lvl	%s60
 918      00BC00BF 
 919 13d0 0000003F 		vld.nc	%v63,8,%s58	# *(a)
 919      BA080081 
 920 13d8 00000000 		ld	%s57,0(0,%s61)	# *(a)
 920      BD003901 
 921 13e0 00003F3E 		vfis.d	%v62,%v63,%s57
 921      00B900DE 
 922 13e8 00000000 		or	%s56,%s58,(0)1
 922      00BA3845 
 923 13f0 0000003E 		vst.nc	%v62,8,%s56	# *(a)
 923      B8080091 
 924              	# line 193
 192:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****     }
 193:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c **** }
 925              		.loc	1 193 0
 926              	# Start of epilogue codes
 927 13f8 00000000 		or	%sp,0,%fp
 927      89000B45 
 928              		.cfi_def_cfa	11,8
 929 1400 18000000 		ld	%got,0x18(,%sp)
 929      8B000F01 
 930 1408 20000000 		ld	%plt,0x20(,%sp)
 930      8B001001 
 931 1410 08000000 		ld	%lr,0x8(,%sp)
 931      8B000A01 
 932 1418 00000000 		ld	%fp,0x0(,%sp)
 932      8B000901 
 933 1420 00000000 		b.l	(,%lr)
 933      8A000F19 
 934              	.L_9.EoE:
 935              		.cfi_endproc
 936              		.set	.L.9.2auto_size,	0x0	# 0 Bytes
 938              	# ============ End  v_diffs16_dp ============
 939              	# ============ Begin  v_every16th_dp2 ============
 940 1428 00000000 		.balign 16
 940      00000000 
 941              	.L_10.0:
 942              	# line 194
 194:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c **** void v_every16th_dp2( float const* restrict a, float*restrict every){
 943              		.loc	1 194 0
 944              		.globl	v_every16th_dp2
 946              	v_every16th_dp2:
 947              		.cfi_startproc
 948 1430 00000000 		st	%fp,0x0(,%sp)
 948      8B000911 
 949              		.cfi_def_cfa_offset	0
 950              		.cfi_offset	9,0
 951 1438 08000000 		st	%lr,0x8(,%sp)
 951      8B000A11 
 952 1440 18000000 		st	%got,0x18(,%sp)
 952      8B000F11 
 953 1448 20000000 		st	%plt,0x20(,%sp)
 953      8B001011 
 954 1450 00000000 		lea	%got,_GLOBAL_OFFSET_TABLE_@PC_LO(-0x18)
 954      00680F06 
 955 1458 00000000 		and	%got,%got,(32)0
 955      608F0F44 
 956 1460 00000000 		sic	%plt
 956      00001028 
 957 1468 00000000 		lea.sl	%got,_GLOBAL_OFFSET_TABLE_@PC_HI(%plt,%got)
 957      8F908F06 
 958 1470 00000000 		or	%fp,0,%sp
 958      8B000945 
 959              		.cfi_def_cfa_register	9
 960 1478 30000000 		st	%s18,48(,%fp)
 960      89001211 
 961 1480 38000000 		st	%s19,56(,%fp)
 961      89001311 
 962 1488 40000000 		st	%s20,64(,%fp)
 962      89001411 
 963 1490 48000000 		st	%s21,72(,%fp)
 963      89001511 
 964 1498 50000000 		st	%s22,80(,%fp)
 964      89001611 
 965 14a0 58000000 		st	%s23,88(,%fp)
 965      89001711 
 966 14a8 60000000 		st	%s24,96(,%fp)
 966      89001811 
 967 14b0 68000000 		st	%s25,104(,%fp)
 967      89001911 
 968 14b8 70000000 		st	%s26,112(,%fp)
 968      89001A11 
 969 14c0 78000000 		st	%s27,120(,%fp)
 969      89001B11 
 970 14c8 80000000 		st	%s28,128(,%fp)
 970      89001C11 
 971 14d0 88000000 		st	%s29,136(,%fp)
 971      89001D11 
 972 14d8 90000000 		st	%s30,144(,%fp)
 972      89001E11 
 973 14e0 98000000 		st	%s31,152(,%fp)
 973      89001F11 
 974 14e8 A0000000 		st	%s32,160(,%fp)
 974      89002011 
 975 14f0 A8000000 		st	%s33,168(,%fp)
 975      89002111 
 976 14f8 00000000 		lea	%s13,.L.10.2auto_size&0xffffffff
 976      00000D06 
 977 1500 00000000 		and	%s13,%s13,(32)0
 977      608D0D44 
 978 1508 00000000 		lea.sl	%sp,.L.10.2auto_size>>32(%fp,%s13)
 978      8D898B06 
 979 1510 48000000 		brge.l.t	%sp,%sl,.L_10.EoP
 979      888B3518 
 980 1518 18000000 		ld	%s61,0x18(,%tp)
 980      8E003D01 
 981 1520 00000000 		or	%s62,0,%s0
 981      80003E45 
 982 1528 3B010000 		lea	%s63,0x13b
 982      00003F06 
 983 1530 00000000 		shm.l	%s63,0x0(%s61)
 983      BD033F31 
 984 1538 08000000 		shm.l	%sl,0x8(%s61)
 984      BD030831 
 985 1540 10000000 		shm.l	%sp,0x10(%s61)
 985      BD030B31 
 986 1548 00000000 		monc
 986      0000003F 
 987 1550 00000000 		or	%s0,0,%s62
 987      BE000045 
 988              	.L_10.EoP:
 989              	# End of prologue codes
 990              	# line 200
 195:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****     // or      %s63,16,(0)1
 196:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****     // lvl     %s63             # vector length 16
 197:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****     // or      %s62,%s0,(0)1    # s62 = &a[0]
 198:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****     // lea     %s61,64          # 16*sizeof(float)
 199:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****     // vldu.nc %v63,%s61,%s62  # *(a)  v63 = load( stride=64, &a[0] )
 200:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****     for(int i=0; i<16; ++i)
 991              		.loc	1 200 0
 992 1558 00000000 		or	%s63,16,(0)1
 992      00103F45 
 993              	# line 201
 201:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****         every[i] = a[16*i];
 994              		.loc	1 201 0
 995 1560 00000000 		or	%s62,%s0,(0)1
 995      00803E45 
 996 1568 40000000 		lea	%s61,64
 996      00003D06 
 997 1570 00000000 		lvl	%s63
 997      00BF00BF 
 998 1578 0000003F 		vldu.nc	%v63,%s61,%s62	# *(a)
 998      BEBD0082 
 999 1580 00000000 		or	%s60,%s1,(0)1
 999      00813C45 
 1000 1588 0000003F 		vstu.nc	%v63,4,%s60	# *(every)
 1000      BC040092 
 1001              	# line 202
 202:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c **** }
 1002              		.loc	1 202 0
 1003              	# Start of epilogue codes
 1004 1590 00000000 		or	%sp,0,%fp
 1004      89000B45 
 1005              		.cfi_def_cfa	11,8
 1006 1598 18000000 		ld	%got,0x18(,%sp)
 1006      8B000F01 
 1007 15a0 20000000 		ld	%plt,0x20(,%sp)
 1007      8B001001 
 1008 15a8 08000000 		ld	%lr,0x8(,%sp)
 1008      8B000A01 
 1009 15b0 00000000 		ld	%fp,0x0(,%sp)
 1009      8B000901 
 1010 15b8 00000000 		b.l	(,%lr)
 1010      8A000F19 
 1011              	.L_10.EoE:
 1012              		.cfi_endproc
 1013              		.set	.L.10.2auto_size,	0x0	# 0 Bytes
 1015              	# ============ End  v_every16th_dp2 ============
 1016              	# ============ Begin  v_sumg16_dp2 ============
 1017              		.balign 16
 1018              	.L_11.0:
 1019              	# line 204
 203:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****     
 204:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c **** void v_sumg16_dp2(float const* restrict a, float*restrict group_sums){
 1020              		.loc	1 204 0
 1021              		.globl	v_sumg16_dp2
 1023              	v_sumg16_dp2:
 1024              		.cfi_startproc
 1025 15c0 00000000 		st	%fp,0x0(,%sp)
 1025      8B000911 
 1026              		.cfi_def_cfa_offset	0
 1027              		.cfi_offset	9,0
 1028 15c8 08000000 		st	%lr,0x8(,%sp)
 1028      8B000A11 
 1029 15d0 18000000 		st	%got,0x18(,%sp)
 1029      8B000F11 
 1030 15d8 20000000 		st	%plt,0x20(,%sp)
 1030      8B001011 
 1031 15e0 00000000 		lea	%got,_GLOBAL_OFFSET_TABLE_@PC_LO(-0x18)
 1031      00680F06 
 1032 15e8 00000000 		and	%got,%got,(32)0
 1032      608F0F44 
 1033 15f0 00000000 		sic	%plt
 1033      00001028 
 1034 15f8 00000000 		lea.sl	%got,_GLOBAL_OFFSET_TABLE_@PC_HI(%plt,%got)
 1034      8F908F06 
 1035 1600 00000000 		or	%fp,0,%sp
 1035      8B000945 
 1036              		.cfi_def_cfa_register	9
 1037 1608 30000000 		st	%s18,48(,%fp)
 1037      89001211 
 1038 1610 38000000 		st	%s19,56(,%fp)
 1038      89001311 
 1039 1618 40000000 		st	%s20,64(,%fp)
 1039      89001411 
 1040 1620 48000000 		st	%s21,72(,%fp)
 1040      89001511 
 1041 1628 50000000 		st	%s22,80(,%fp)
 1041      89001611 
 1042 1630 58000000 		st	%s23,88(,%fp)
 1042      89001711 
 1043 1638 60000000 		st	%s24,96(,%fp)
 1043      89001811 
 1044 1640 68000000 		st	%s25,104(,%fp)
 1044      89001911 
 1045 1648 70000000 		st	%s26,112(,%fp)
 1045      89001A11 
 1046 1650 78000000 		st	%s27,120(,%fp)
 1046      89001B11 
 1047 1658 80000000 		st	%s28,128(,%fp)
 1047      89001C11 
 1048 1660 88000000 		st	%s29,136(,%fp)
 1048      89001D11 
 1049 1668 90000000 		st	%s30,144(,%fp)
 1049      89001E11 
 1050 1670 98000000 		st	%s31,152(,%fp)
 1050      89001F11 
 1051 1678 A0000000 		st	%s32,160(,%fp)
 1051      89002011 
 1052 1680 A8000000 		st	%s33,168(,%fp)
 1052      89002111 
 1053 1688 00F8FFFF 		lea	%s13,.L.11.2auto_size&0xffffffff
 1053      00000D06 
 1054 1690 00000000 		and	%s13,%s13,(32)0
 1054      608D0D44 
 1055 1698 FFFFFFFF 		lea.sl	%sp,.L.11.2auto_size>>32(%fp,%s13)
 1055      8D898B06 
 1056 16a0 48000000 		brge.l.t	%sp,%sl,.L_11.EoP
 1056      888B3518 
 1057 16a8 18000000 		ld	%s61,0x18(,%tp)
 1057      8E003D01 
 1058 16b0 00000000 		or	%s62,0,%s0
 1058      80003E45 
 1059 16b8 3B010000 		lea	%s63,0x13b
 1059      00003F06 
 1060 16c0 00000000 		shm.l	%s63,0x0(%s61)
 1060      BD033F31 
 1061 16c8 08000000 		shm.l	%sl,0x8(%s61)
 1061      BD030831 
 1062 16d0 10000000 		shm.l	%sp,0x10(%s61)
 1062      BD030B31 
 1063 16d8 00000000 		monc
 1063      0000003F 
 1064 16e0 00000000 		or	%s0,0,%s62
 1064      BE000045 
 1065              	.L_11.EoP:
 1066              	# End of prologue codes
 1067              	# line 217
 205:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****     double part_sums[256];
 206:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****     register double part_sum = 0.f;
 207:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****     // lea     %s63,128
 208:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****     // lvl     %s63             # vec length 128
 209:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****     // or      %s62,%s0,(0)1
 210:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****     // vldu.nc %v63,8,%s62      # v63: *(a) a[0..128) by 2   even values
 211:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****     // lea     %s61,4
 212:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****     // adds.l  %s60,%s0,%s61
 213:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****     // vldu.nc %v62,8,%s60      # v62: *(a) a[1..128) by 2   odd values
 214:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****     // vfadd.s %v61,%v63,%v62   # v61: pair_sums [0..128)
 215:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****     // or      %s59,0,(0)1      # s59: "pair_sums[-1]"
 216:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****     // vfia.s  %v60,%v61,%s59   # v60: pair_sums[i] + pair_sums[i-1] (now incremental sum)
 217:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****     for(int i=0; i<256; ++i){
 1068              		.loc	1 217 0
 1069 16e8 80000000 		lea	%s63,128
 1069      00003F06 
 1070              	# line 218
 218:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****         part_sum += a[i];
 1071              		.loc	1 218 0
 1072 16f0 00000000 		or	%s62,%s0,(0)1
 1072      00803E45 
 1073 16f8 00000000 		lvl	%s63
 1073      00BF00BF 
 1074 1700 0000003F 		vldu.nc	%v63,8,%s62	# *(a)
 1074      BE080082 
 1075 1708 00003F3E 		vcvt.d.s	%v62,%v63
 1075      0000008F 
 1076 1710 04000000 		lea	%s61,4
 1076      00003D06 
 1077 1718 00000000 		adds.l	%s60,%s0,%s61
 1077      BD803C59 
 1078 1720 0000003D 		vldu.nc	%v61,8,%s60	# *(a)
 1078      BC080082 
 1079 1728 00003D3C 		vcvt.d.s	%v60,%v61
 1079      0000008F 
 1080 1730 003C3E3B 		vfadd.d	%v59,%v62,%v60
 1080      000000CC 
 1081 1738 00000000 		or	%s59,0,(0)1
 1081      00003B45 
 1082 1740 00003B3A 		vfia.d	%v58,%v59,%s59
 1082      00BB00CE 
 1083              	# line 225
 219:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****         // vfsub.s %v59,%v60,%v62   # v59: cum_pair_sums[i] - odds[i] (de-interleaved partial sums,
 220:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****         // adds.l  %s58,%fp,(54)1
 221:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****         // vstu.nc %v59,8,%s58     # part_sums   -- store evens of part_sums
 222:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****         // lea     %s57,4
 223:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****         // adds.l  %s56,%s58,%s57
 224:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****         // vstu.nc %v60,8,%s56     # part_sums   -- store odds of part_sums
 225:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****         part_sums[i] = part_sum;
 1084              		.loc	1 225 0
 1085 1748 003C3A39 		vfsub.d	%v57,%v58,%v60
 1085      000000DC 
 1086 1750 00000000 		adds.l	%s58,%fp,(53)1
 1086      35893A59 
 1087 1758 00000039 		vst.nc	%v57,16,%s58	# part_sums
 1087      BA100091 
 1088 1760 08000000 		lea	%s57,8
 1088      00003906 
 1089 1768 00000000 		adds.l	%s56,%s58,%s57
 1089      B9BA3859 
 1090 1770 0000003A 		vst.nc	%v58,16,%s56	# part_sums
 1090      B8100091 
 1091              	# line 227
 226:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****     }
 227:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****     for(int i=0; i<16; ++i){
 1092              		.loc	1 227 0
 1093 1778 00000000 		or	%s55,16,(0)1
 1093      00103745 
 1094              	# line 229
 228:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****         // ... vldu.nc %v58,%s53,%s54  # part_sums, s53=stride=64, s54=addr_of_part_sums (wrt %fp)
 229:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****         group_sums[i] = part_sums[16*i];
 1095              		.loc	1 229 0
 1096 1780 00000000 		adds.l	%s54,%fp,(53)1
 1096      35893659 
 1097 1788 80000000 		lea	%s53,128
 1097      00003506 
 1098 1790 00000000 		lvl	%s55
 1098      00B700BF 
 1099 1798 00000038 		vld.nc	%v56,%s53,%s54	# part_sums
 1099      B6B50081 
 1100 17a0 00003837 		vcvt.s.d	%v55,%v56
 1100      0000009F 
 1101 17a8 00000000 		or	%s52,%s1,(0)1
 1101      00813445 
 1102 17b0 00000037 		vstu.nc	%v55,4,%s52	# *(group_sums)
 1102      B4040092 
 1103              	# line 231
 230:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****     }
 231:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****     for(int i=1; i<16; ++i){
 1104              		.loc	1 231 0
 1105 17b8 00000000 		adds.l	%s51,%s1,(62)1
 1105      3E813359 
 1106 17c0 04000000 		lea	%s50,4
 1106      00003206 
 1107 17c8 00000000 		adds.l	%s49,%s51,%s50
 1107      B2B33159 
 1108 17d0 00000000 		or	%s48,15,(0)1
 1108      000F3045 
 1109              	# line 236
 232:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****         // adjust vector length
 233:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****         // ... vldu.nc %v57,4,%s46     # *(group_sums)
 234:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****         // ldu     %s45,0(0,%s49)  # *(group_sums) "group_sums[-1]"
 235:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****         // vfis.s  %v56,%v57,%s45  # v56:groups_sums[i=1..) = v57:group_sums[i] - s45:group_sums[i-
 236:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****         group_sums[i] -= group_sums[i-1];
 1110              		.loc	1 236 0
 1111 17d8 04000000 		lea	%s47,4
 1111      00002F06 
 1112 17e0 00000000 		adds.l	%s46,%s49,%s47
 1112      AFB12E59 
 1113 17e8 00000000 		lvl	%s48
 1113      00B000BF 
 1114 17f0 00000036 		vldu.nc	%v54,4,%s46	# *(group_sums)
 1114      AE040082 
 1115 17f8 00000000 		ldu	%s45,0(0,%s49)	# *(group_sums)
 1115      B1002D02 
 1116 1800 00003635 		vfis.s	%v53,%v54,%s45
 1116      00AD80DE 
 1117 1808 00000000 		or	%s44,%s46,(0)1
 1117      00AE2C45 
 1118 1810 00000035 		vstu.nc	%v53,4,%s44	# *(group_sums)
 1118      AC040092 
 1119              	# line 238
 237:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****     }
 238:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c **** }
 1120              		.loc	1 238 0
 1121              	# Start of epilogue codes
 1122 1818 00000000 		or	%sp,0,%fp
 1122      89000B45 
 1123              		.cfi_def_cfa	11,8
 1124 1820 18000000 		ld	%got,0x18(,%sp)
 1124      8B000F01 
 1125 1828 20000000 		ld	%plt,0x20(,%sp)
 1125      8B001001 
 1126 1830 08000000 		ld	%lr,0x8(,%sp)
 1126      8B000A01 
 1127 1838 00000000 		ld	%fp,0x0(,%sp)
 1127      8B000901 
 1128 1840 00000000 		b.l	(,%lr)
 1128      8A000F19 
 1129              	.L_11.EoE:
 1130              		.cfi_endproc
 1131              		.set	.L.11.2auto_size,	0xfffffffffffff800	# 2048 Bytes
 1133              	# ============ End  v_sumg16_dp2 ============
 1134              	# ============ Begin  f_reduce256_fp ============
 1135 1848 00000000 		.balign 16
 1135      00000000 
 1136              	.L_12.0:
 1137              	# line 240
 239:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c **** 
 240:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c **** float f_reduce256_fp(float const* restrict a){
 1138              		.loc	1 240 0
 1139              		.globl	f_reduce256_fp
 1141              	f_reduce256_fp:
 1142              		.cfi_startproc
 1143 1850 00000000 		st	%fp,0x0(,%sp)
 1143      8B000911 
 1144              		.cfi_def_cfa_offset	0
 1145              		.cfi_offset	9,0
 1146 1858 08000000 		st	%lr,0x8(,%sp)
 1146      8B000A11 
 1147 1860 18000000 		st	%got,0x18(,%sp)
 1147      8B000F11 
 1148 1868 20000000 		st	%plt,0x20(,%sp)
 1148      8B001011 
 1149 1870 00000000 		lea	%got,_GLOBAL_OFFSET_TABLE_@PC_LO(-0x18)
 1149      00680F06 
 1150 1878 00000000 		and	%got,%got,(32)0
 1150      608F0F44 
 1151 1880 00000000 		sic	%plt
 1151      00001028 
 1152 1888 00000000 		lea.sl	%got,_GLOBAL_OFFSET_TABLE_@PC_HI(%plt,%got)
 1152      8F908F06 
 1153 1890 00000000 		or	%fp,0,%sp
 1153      8B000945 
 1154              		.cfi_def_cfa_register	9
 1155 1898 30000000 		st	%s18,48(,%fp)
 1155      89001211 
 1156 18a0 38000000 		st	%s19,56(,%fp)
 1156      89001311 
 1157 18a8 40000000 		st	%s20,64(,%fp)
 1157      89001411 
 1158 18b0 48000000 		st	%s21,72(,%fp)
 1158      89001511 
 1159 18b8 50000000 		st	%s22,80(,%fp)
 1159      89001611 
 1160 18c0 58000000 		st	%s23,88(,%fp)
 1160      89001711 
 1161 18c8 60000000 		st	%s24,96(,%fp)
 1161      89001811 
 1162 18d0 68000000 		st	%s25,104(,%fp)
 1162      89001911 
 1163 18d8 70000000 		st	%s26,112(,%fp)
 1163      89001A11 
 1164 18e0 78000000 		st	%s27,120(,%fp)
 1164      89001B11 
 1165 18e8 80000000 		st	%s28,128(,%fp)
 1165      89001C11 
 1166 18f0 88000000 		st	%s29,136(,%fp)
 1166      89001D11 
 1167 18f8 90000000 		st	%s30,144(,%fp)
 1167      89001E11 
 1168 1900 98000000 		st	%s31,152(,%fp)
 1168      89001F11 
 1169 1908 A0000000 		st	%s32,160(,%fp)
 1169      89002011 
 1170 1910 A8000000 		st	%s33,168(,%fp)
 1170      89002111 
 1171 1918 00000000 		lea	%s13,.L.12.2auto_size&0xffffffff
 1171      00000D06 
 1172 1920 00000000 		and	%s13,%s13,(32)0
 1172      608D0D44 
 1173 1928 00000000 		lea.sl	%sp,.L.12.2auto_size>>32(%fp,%s13)
 1173      8D898B06 
 1174 1930 48000000 		brge.l.t	%sp,%sl,.L_12.EoP
 1174      888B3518 
 1175 1938 18000000 		ld	%s61,0x18(,%tp)
 1175      8E003D01 
 1176 1940 00000000 		or	%s62,0,%s0
 1176      80003E45 
 1177 1948 3B010000 		lea	%s63,0x13b
 1177      00003F06 
 1178 1950 00000000 		shm.l	%s63,0x0(%s61)
 1178      BD033F31 
 1179 1958 08000000 		shm.l	%sl,0x8(%s61)
 1179      BD030831 
 1180 1960 10000000 		shm.l	%sp,0x10(%s61)
 1180      BD030B31 
 1181 1968 00000000 		monc
 1181      0000003F 
 1182 1970 00000000 		or	%s0,0,%s62
 1182      BE000045 
 1183              	.L_12.EoP:
 1184              	# End of prologue codes
 1185              	# line 247
 241:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****     // vldu.nc %v63,4,%s62     # *(a)
 242:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****     // vfsum.s %v62,%v63
 243:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****     // lvs     %s61,%v62(0)
 244:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****     // ld      %s60,-8(,%fp)   # restore
 245:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****     // fadd.s  %s59,%s60,%s61
 246:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****     register float ret=0.f;
 247:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****     for(int i=0; i<256; ++i)
 1186              		.loc	1 247 0
 1187 1978 00010000 		lea	%s63,256
 1187      00003F06 
 1188              	# line 248
 248:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****         ret += a[i];
 1189              		.loc	1 248 0
 1190 1980 00000000 		or	%s62,%s0,(0)1
 1190      00803E45 
 1191 1988 00000000 		lvl	%s63
 1191      00BF00BF 
 1192 1990 0000003F 		vldu.nc	%v63,4,%s62	# *(a)
 1192      BE040082 
 1193 1998 00003F3E 		vfsum.s	%v62,%v63
 1193      000080EC 
 1194 19a0 0000003E 		lvs	%s61,%v62(0)
 1194      00003D9E 
 1195              	# line 249
 249:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****     return ret;
 1196              		.loc	1 249 0
 1197 19a8 00000000 		or	%s0,0,%s61
 1197      BD000045 
 1198              	# Start of epilogue codes
 1199 19b0 00000000 		or	%sp,0,%fp
 1199      89000B45 
 1200              		.cfi_def_cfa	11,8
 1201 19b8 18000000 		ld	%got,0x18(,%sp)
 1201      8B000F01 
 1202 19c0 20000000 		ld	%plt,0x20(,%sp)
 1202      8B001001 
 1203 19c8 08000000 		ld	%lr,0x8(,%sp)
 1203      8B000A01 
 1204 19d0 00000000 		ld	%fp,0x0(,%sp)
 1204      8B000901 
 1205 19d8 00000000 		b.l	(,%lr)
 1205      8A000F19 
 1206              	.L_12.EoE:
 1207              		.cfi_endproc
 1208              		.set	.L.12.2auto_size,	0x0	# 0 Bytes
 1210              	# ============ End  f_reduce256_fp ============
 1211              	# ============ Begin  d_reduce256_fp ============
 1212              		.balign 16
 1213              	.L_13.0:
 1214              	# line 252
 250:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c **** }
 251:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c **** 
 252:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c **** double d_reduce256_fp(float const* restrict a){
 1215              		.loc	1 252 0
 1216              		.globl	d_reduce256_fp
 1218              	d_reduce256_fp:
 1219              		.cfi_startproc
 1220 19e0 00000000 		st	%fp,0x0(,%sp)
 1220      8B000911 
 1221              		.cfi_def_cfa_offset	0
 1222              		.cfi_offset	9,0
 1223 19e8 08000000 		st	%lr,0x8(,%sp)
 1223      8B000A11 
 1224 19f0 18000000 		st	%got,0x18(,%sp)
 1224      8B000F11 
 1225 19f8 20000000 		st	%plt,0x20(,%sp)
 1225      8B001011 
 1226 1a00 00000000 		lea	%got,_GLOBAL_OFFSET_TABLE_@PC_LO(-0x18)
 1226      00680F06 
 1227 1a08 00000000 		and	%got,%got,(32)0
 1227      608F0F44 
 1228 1a10 00000000 		sic	%plt
 1228      00001028 
 1229 1a18 00000000 		lea.sl	%got,_GLOBAL_OFFSET_TABLE_@PC_HI(%plt,%got)
 1229      8F908F06 
 1230 1a20 00000000 		or	%fp,0,%sp
 1230      8B000945 
 1231              		.cfi_def_cfa_register	9
 1232 1a28 30000000 		st	%s18,48(,%fp)
 1232      89001211 
 1233 1a30 38000000 		st	%s19,56(,%fp)
 1233      89001311 
 1234 1a38 40000000 		st	%s20,64(,%fp)
 1234      89001411 
 1235 1a40 48000000 		st	%s21,72(,%fp)
 1235      89001511 
 1236 1a48 50000000 		st	%s22,80(,%fp)
 1236      89001611 
 1237 1a50 58000000 		st	%s23,88(,%fp)
 1237      89001711 
 1238 1a58 60000000 		st	%s24,96(,%fp)
 1238      89001811 
 1239 1a60 68000000 		st	%s25,104(,%fp)
 1239      89001911 
 1240 1a68 70000000 		st	%s26,112(,%fp)
 1240      89001A11 
 1241 1a70 78000000 		st	%s27,120(,%fp)
 1241      89001B11 
 1242 1a78 80000000 		st	%s28,128(,%fp)
 1242      89001C11 
 1243 1a80 88000000 		st	%s29,136(,%fp)
 1243      89001D11 
 1244 1a88 90000000 		st	%s30,144(,%fp)
 1244      89001E11 
 1245 1a90 98000000 		st	%s31,152(,%fp)
 1245      89001F11 
 1246 1a98 A0000000 		st	%s32,160(,%fp)
 1246      89002011 
 1247 1aa0 A8000000 		st	%s33,168(,%fp)
 1247      89002111 
 1248 1aa8 F0FFFFFF 		lea	%s13,.L.13.2auto_size&0xffffffff
 1248      00000D06 
 1249 1ab0 00000000 		and	%s13,%s13,(32)0
 1249      608D0D44 
 1250 1ab8 FFFFFFFF 		lea.sl	%sp,.L.13.2auto_size>>32(%fp,%s13)
 1250      8D898B06 
 1251 1ac0 48000000 		brge.l.t	%sp,%sl,.L_13.EoP
 1251      888B3518 
 1252 1ac8 18000000 		ld	%s61,0x18(,%tp)
 1252      8E003D01 
 1253 1ad0 00000000 		or	%s62,0,%s0
 1253      80003E45 
 1254 1ad8 3B010000 		lea	%s63,0x13b
 1254      00003F06 
 1255 1ae0 00000000 		shm.l	%s63,0x0(%s61)
 1255      BD033F31 
 1256 1ae8 08000000 		shm.l	%sl,0x8(%s61)
 1256      BD030831 
 1257 1af0 10000000 		shm.l	%sp,0x10(%s61)
 1257      BD030B31 
 1258 1af8 00000000 		monc
 1258      0000003F 
 1259 1b00 00000000 		or	%s0,0,%s62
 1259      BE000045 
 1260              	.L_13.EoP:
 1261              	# End of prologue codes
 1262              	# line 254
 253:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****     double ret;
 254:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****     for(int i=0; i<256; ++i)
 1263              		.loc	1 254 0
 1264 1b08 00010000 		lea	%s63,256
 1264      00003F06 
 1265              	# line 255
 255:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****         ret += a[i];
 1266              		.loc	1 255 0
 1267 1b10 00000000 		or	%s62,%s0,(0)1
 1267      00803E45 
 1268 1b18 00000000 		lvl	%s63
 1268      00BF00BF 
 1269 1b20 0000003F 		vldu.nc	%v63,4,%s62	# *(a)
 1269      BE040082 
 1270 1b28 00003F3E 		vcvt.d.s	%v62,%v63
 1270      0000008F 
 1271 1b30 00003E3D 		vfsum.d	%v61,%v62
 1271      000000EC 
 1272 1b38 0000003D 		lvs	%s61,%v61(0)
 1272      00003D9E 
 1273 1b40 F8FFFFFF 		ld	%s60,-8(,%fp)	# restore
 1273      89003C01 
 1274 1b48 00000000 		fadd.d	%s59,%s60,%s61
 1274      BDBC3B4C 
 1275              	# line 256
 256:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****     return ret;
 1276              		.loc	1 256 0
 1277 1b50 00000000 		or	%s0,0,%s59
 1277      BB000045 
 1278              	# Start of epilogue codes
 1279 1b58 00000000 		or	%sp,0,%fp
 1279      89000B45 
 1280              		.cfi_def_cfa	11,8
 1281 1b60 18000000 		ld	%got,0x18(,%sp)
 1281      8B000F01 
 1282 1b68 20000000 		ld	%plt,0x20(,%sp)
 1282      8B001001 
 1283 1b70 08000000 		ld	%lr,0x8(,%sp)
 1283      8B000A01 
 1284 1b78 00000000 		ld	%fp,0x0(,%sp)
 1284      8B000901 
 1285 1b80 00000000 		b.l	(,%lr)
 1285      8A000F19 
 1286              	.L_13.EoE:
 1287              		.cfi_endproc
 1288              		.set	.L.13.2auto_size,	0xfffffffffffffff0	# 16 Bytes
 1290              	# ============ End  d_reduce256_fp ============
 1291              	# ============ Begin  d_reduce256_dp ============
 1292 1b88 00000000 		.balign 16
 1292      00000000 
 1293              	.L_14.0:
 1294              	# line 259
 257:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c **** }
 258:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c **** 
 259:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c **** double d_reduce256_dp(double* restrict a){
 1295              		.loc	1 259 0
 1296              		.globl	d_reduce256_dp
 1298              	d_reduce256_dp:
 1299              		.cfi_startproc
 1300 1b90 00000000 		st	%fp,0x0(,%sp)
 1300      8B000911 
 1301              		.cfi_def_cfa_offset	0
 1302              		.cfi_offset	9,0
 1303 1b98 08000000 		st	%lr,0x8(,%sp)
 1303      8B000A11 
 1304 1ba0 18000000 		st	%got,0x18(,%sp)
 1304      8B000F11 
 1305 1ba8 20000000 		st	%plt,0x20(,%sp)
 1305      8B001011 
 1306 1bb0 00000000 		lea	%got,_GLOBAL_OFFSET_TABLE_@PC_LO(-0x18)
 1306      00680F06 
 1307 1bb8 00000000 		and	%got,%got,(32)0
 1307      608F0F44 
 1308 1bc0 00000000 		sic	%plt
 1308      00001028 
 1309 1bc8 00000000 		lea.sl	%got,_GLOBAL_OFFSET_TABLE_@PC_HI(%plt,%got)
 1309      8F908F06 
 1310 1bd0 00000000 		or	%fp,0,%sp
 1310      8B000945 
 1311              		.cfi_def_cfa_register	9
 1312 1bd8 30000000 		st	%s18,48(,%fp)
 1312      89001211 
 1313 1be0 38000000 		st	%s19,56(,%fp)
 1313      89001311 
 1314 1be8 40000000 		st	%s20,64(,%fp)
 1314      89001411 
 1315 1bf0 48000000 		st	%s21,72(,%fp)
 1315      89001511 
 1316 1bf8 50000000 		st	%s22,80(,%fp)
 1316      89001611 
 1317 1c00 58000000 		st	%s23,88(,%fp)
 1317      89001711 
 1318 1c08 60000000 		st	%s24,96(,%fp)
 1318      89001811 
 1319 1c10 68000000 		st	%s25,104(,%fp)
 1319      89001911 
 1320 1c18 70000000 		st	%s26,112(,%fp)
 1320      89001A11 
 1321 1c20 78000000 		st	%s27,120(,%fp)
 1321      89001B11 
 1322 1c28 80000000 		st	%s28,128(,%fp)
 1322      89001C11 
 1323 1c30 88000000 		st	%s29,136(,%fp)
 1323      89001D11 
 1324 1c38 90000000 		st	%s30,144(,%fp)
 1324      89001E11 
 1325 1c40 98000000 		st	%s31,152(,%fp)
 1325      89001F11 
 1326 1c48 A0000000 		st	%s32,160(,%fp)
 1326      89002011 
 1327 1c50 A8000000 		st	%s33,168(,%fp)
 1327      89002111 
 1328 1c58 F0FFFFFF 		lea	%s13,.L.14.2auto_size&0xffffffff
 1328      00000D06 
 1329 1c60 00000000 		and	%s13,%s13,(32)0
 1329      608D0D44 
 1330 1c68 FFFFFFFF 		lea.sl	%sp,.L.14.2auto_size>>32(%fp,%s13)
 1330      8D898B06 
 1331 1c70 48000000 		brge.l.t	%sp,%sl,.L_14.EoP
 1331      888B3518 
 1332 1c78 18000000 		ld	%s61,0x18(,%tp)
 1332      8E003D01 
 1333 1c80 00000000 		or	%s62,0,%s0
 1333      80003E45 
 1334 1c88 3B010000 		lea	%s63,0x13b
 1334      00003F06 
 1335 1c90 00000000 		shm.l	%s63,0x0(%s61)
 1335      BD033F31 
 1336 1c98 08000000 		shm.l	%sl,0x8(%s61)
 1336      BD030831 
 1337 1ca0 10000000 		shm.l	%sp,0x10(%s61)
 1337      BD030B31 
 1338 1ca8 00000000 		monc
 1338      0000003F 
 1339 1cb0 00000000 		or	%s0,0,%s62
 1339      BE000045 
 1340              	.L_14.EoP:
 1341              	# End of prologue codes
 1342              	# line 261
 260:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****     double ret;
 261:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****     for(int i=0; i<256; ++i)
 1343              		.loc	1 261 0
 1344 1cb8 00010000 		lea	%s63,256
 1344      00003F06 
 1345              	# line 262
 262:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****         ret += a[i];
 1346              		.loc	1 262 0
 1347 1cc0 00000000 		or	%s62,%s0,(0)1
 1347      00803E45 
 1348 1cc8 00000000 		lvl	%s63
 1348      00BF00BF 
 1349 1cd0 0000003F 		vld.nc	%v63,8,%s62	# *(a)
 1349      BE080081 
 1350 1cd8 00003F3E 		vfsum.d	%v62,%v63
 1350      000000EC 
 1351 1ce0 0000003E 		lvs	%s61,%v62(0)
 1351      00003D9E 
 1352 1ce8 F8FFFFFF 		ld	%s60,-8(,%fp)	# restore
 1352      89003C01 
 1353 1cf0 00000000 		fadd.d	%s59,%s60,%s61
 1353      BDBC3B4C 
 1354              	# line 263
 263:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/f_add256.c ****     return ret;
 1355              		.loc	1 263 0
 1356 1cf8 00000000 		or	%s0,0,%s59
 1356      BB000045 
 1357              	# Start of epilogue codes
 1358 1d00 00000000 		or	%sp,0,%fp
 1358      89000B45 
 1359              		.cfi_def_cfa	11,8
 1360 1d08 18000000 		ld	%got,0x18(,%sp)
 1360      8B000F01 
 1361 1d10 20000000 		ld	%plt,0x20(,%sp)
 1361      8B001001 
 1362 1d18 08000000 		ld	%lr,0x8(,%sp)
 1362      8B000A01 
 1363 1d20 00000000 		ld	%fp,0x0(,%sp)
 1363      8B000901 
 1364 1d28 00000000 		b.l	(,%lr)
 1364      8A000F19 
 1365              	.L_14.EoE:
 1366              		.cfi_endproc
 1367              		.set	.L.14.2auto_size,	0xfffffffffffffff0	# 16 Bytes
 1369              	# ============ End  d_reduce256_dp ============
 1370              	.comm _PVWORK_STATIC,3072,16
 1371              	.Le1.0:
