   1              		.ident "ncc 1.1.4 (Build 10:17:22 Apr 19 2018)"
   2              		.file "grpsum.c"
   3              		.file 1 "/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/grpsum.c"
   4              		.file 2 "/opt/nec/ve/ncc/1.1.4/include/stdc-predef.h"
   5              		.data
   6              		.balign 16
   7              		.globl	grp16
  10              	grp16:
  11 0000 FFFF0000 		.long	65535
  11      00000000 
  12 0008 0000FFFF 		.long	4294901760
  12      00000000 
  13 0010 00000000 		.long	281470681743360
  13      FFFF0000 
  14 0018 00000000 		.long	18446462598732840960
  14      0000FFFF 
  15              		.globl	msk1
  16              		.balign 4
  19              	msk1:
  20 0020 00000000 		.zero	64
  20      00000000 
  20      00000000 
  20      00000000 
  20      00000000 
  21 0060 01000000 		.int	1
  22 0064 01000000 		.int	1
  23 0068 01000000 		.int	1
  24 006c 01000000 		.int	1
  25 0070 01000000 		.int	1
  26 0074 01000000 		.int	1
  27 0078 01000000 		.int	1
  28 007c 01000000 		.int	1
  29 0080 01000000 		.int	1
  30 0084 01000000 		.int	1
  31 0088 01000000 		.int	1
  32 008c 01000000 		.int	1
  33 0090 01000000 		.int	1
  34 0094 01000000 		.int	1
  35 0098 01000000 		.int	1
  36 009c 01000000 		.int	1
  37 00a0 01000000 		.int	1
  38 00a4 00000000 		.zero	892
  38      00000000 
  38      00000000 
  38      00000000 
  38      00000000 
  39              		.globl	msk0
  40              		.balign 4
  43              	msk0:
  44 0420 01000000 		.int	1
  45 0424 01000000 		.int	1
  46 0428 01000000 		.int	1
  47 042c 01000000 		.int	1
  48 0430 01000000 		.int	1
  49 0434 01000000 		.int	1
  50 0438 01000000 		.int	1
  51 043c 01000000 		.int	1
  52 0440 01000000 		.int	1
  53 0444 01000000 		.int	1
  54 0448 01000000 		.int	1
  55 044c 01000000 		.int	1
  56 0450 01000000 		.int	1
  57 0454 01000000 		.int	1
  58 0458 01000000 		.int	1
  59 045c 01000000 		.int	1
  60 0460 01000000 		.int	1
  61 0464 00000000 		.zero	956
  61      00000000 
  61      00000000 
  61      00000000 
  61      00000000 
  62              	# ============ Begin  grpsum ============
  63              		.text
  64              		.balign 16
  65              	.L_1.0:
  66              	# line 2
   1:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/grpsum.c **** /** add 256 doubles <em>in groups of 16</em> */
   2:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/grpsum.c **** void grpsum(double* d, double* s){
  67              		.loc	1 2 0
  68              		.globl	grpsum
  70              	grpsum:
  71              		.cfi_startproc
  72 0000 00000000 		st	%fp,0x0(,%sp)
  72      8B000911 
  73              		.cfi_def_cfa_offset	0
  74              		.cfi_offset	9,0
  75 0008 08000000 		st	%lr,0x8(,%sp)
  75      8B000A11 
  76 0010 18000000 		st	%got,0x18(,%sp)
  76      8B000F11 
  77 0018 20000000 		st	%plt,0x20(,%sp)
  77      8B001011 
  78 0020 00000000 		lea	%got,_GLOBAL_OFFSET_TABLE_@PC_LO(-0x18)
  78      00680F06 
  79 0028 00000000 		and	%got,%got,(32)0
  79      608F0F44 
  80 0030 00000000 		sic	%plt
  80      00001028 
  81 0038 00000000 		lea.sl	%got,_GLOBAL_OFFSET_TABLE_@PC_HI(%plt,%got)
  81      8F908F06 
  82 0040 00000000 		or	%fp,0,%sp
  82      8B000945 
  83              		.cfi_def_cfa_register	9
  84 0048 30000000 		st	%s18,48(,%fp)
  84      89001211 
  85 0050 38000000 		st	%s19,56(,%fp)
  85      89001311 
  86 0058 40000000 		st	%s20,64(,%fp)
  86      89001411 
  87 0060 48000000 		st	%s21,72(,%fp)
  87      89001511 
  88 0068 50000000 		st	%s22,80(,%fp)
  88      89001611 
  89 0070 58000000 		st	%s23,88(,%fp)
  89      89001711 
  90 0078 60000000 		st	%s24,96(,%fp)
  90      89001811 
  91 0080 68000000 		st	%s25,104(,%fp)
  91      89001911 
  92 0088 70000000 		st	%s26,112(,%fp)
  92      89001A11 
  93 0090 78000000 		st	%s27,120(,%fp)
  93      89001B11 
  94 0098 80000000 		st	%s28,128(,%fp)
  94      89001C11 
  95 00a0 88000000 		st	%s29,136(,%fp)
  95      89001D11 
  96 00a8 90000000 		st	%s30,144(,%fp)
  96      89001E11 
  97 00b0 98000000 		st	%s31,152(,%fp)
  97      89001F11 
  98 00b8 A0000000 		st	%s32,160(,%fp)
  98      89002011 
  99 00c0 A8000000 		st	%s33,168(,%fp)
  99      89002111 
 100 00c8 A0FEFFFF 		lea	%s13,.L.1.2auto_size&0xffffffff
 100      00000D06 
 101 00d0 00000000 		and	%s13,%s13,(32)0
 101      608D0D44 
 102 00d8 FFFFFFFF 		lea.sl	%sp,.L.1.2auto_size>>32(%fp,%s13)
 102      8D898B06 
 103 00e0 48000000 		brge.l.t	%sp,%sl,.L_1.EoP
 103      888B3518 
 104 00e8 18000000 		ld	%s61,0x18(,%tp)
 104      8E003D01 
 105 00f0 00000000 		or	%s62,0,%s0
 105      80003E45 
 106 00f8 3B010000 		lea	%s63,0x13b
 106      00003F06 
 107 0100 00000000 		shm.l	%s63,0x0(%s61)
 107      BD033F31 
 108 0108 08000000 		shm.l	%sl,0x8(%s61)
 108      BD030831 
 109 0110 10000000 		shm.l	%sp,0x10(%s61)
 109      BD030B31 
 110 0118 00000000 		monc
 110      0000003F 
 111 0120 00000000 		or	%s0,0,%s62
 111      BE000045 
 112              	.L_1.EoP:
 113              	# End of prologue codes
 114              	# line 3
   3:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/grpsum.c **** 	for(int j=0; j<16; ++j){
 115              		.loc	1 3 0
 116 0128 00000000 		or	%s63,16,(0)1
 116      00103F45 
 117              	# line 4
   4:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/grpsum.c **** 		s[j] = 0.0;
 118              		.loc	1 4 0
 119 0130 00000000 		or	%s62,0,(0)1
 119      00003E45 
 120 0138 00000000 		lvl	%s63
 120      00BF00BF 
 121 0140 0000003F 		vbrd	%v63,%s62
 121      00BE008C 
 122 0148 00000000 		or	%s61,%s1,(0)1
 122      00813D45 
 123 0150 0000003F 		vst.nc	%v63,8,%s61	# *(s)
 123      BD080091 
 124              	# line 6
   5:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/grpsum.c **** 	}
   6:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/grpsum.c **** 	for(int j=0; j<16; ++j){
 125              		.loc	1 6 0
 126 0158 00000000 		or	%s60,%s1,(0)1
 126      00813C45 
 127 0160 80000000 		lea	%s59,128
 127      00003B06 
 128 0168 00000000 		adds.l	%s58,%s0,%s59
 128      BB803A59 
 129 0170 00000000 		or	%s57,%s58,(0)1
 129      00BA3945 
 130 0178 00000000 		or	%s56,%s0,(0)1
 130      00803845 
 131 0180 00010000 		lea	%s55,256
 131      00003706 
 132 0188 00000000 		adds.l	%s54,%s0,%s55
 132      B7803659 
 133 0190 00000000 		or	%s53,%s54,(0)1
 133      00B63545 
 134 0198 80010000 		lea	%s52,384
 134      00003406 
 135 01a0 00000000 		adds.l	%s51,%s0,%s52
 135      B4803359 
 136 01a8 00000000 		or	%s50,%s51,(0)1
 136      00B33245 
 137 01b0 00000000 		or	%s49,-16,(0)1
 137      00703145 
 138 01b8 F8FFFFFF 		st	%s56,-8(,%fp)	# spill
 138      89003811 
 139 01c0 F0FFFFFF 		st	%s50,-16(,%fp)	# spill
 139      89003211 
 140 01c8 E8FFFFFF 		st	%s53,-24(,%fp)	# spill
 140      89003511 
 141 01d0 E0FFFFFF 		st	%s57,-32(,%fp)	# spill
 141      89003911 
 142 01d8 D8FFFFFF 		st	%s60,-40(,%fp)	# spill
 142      89003C11 
 143 01e0 D0FFFFFF 		st	%s49,-48(,%fp)	# spill
 143      89003111 
 144 01e8 08000000 		br.l	.L_1.5
 144      00000F18 
 145              	.L_1.5:
 146              	# line 7
   7:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/grpsum.c **** 		for(int i=0; i<16; ++i){
 147              		.loc	1 7 0
 148 01f0 00000000 		or	%s63,1,(0)1
 148      00013F45 
 149 01f8 D8FFFFFF 		ld	%s62,-40(,%fp)	# restore
 149      89003E01 
 150 0200 00000000 		or	%s61,%s62,(0)1
 150      00BE3D45 
 151 0208 00000000 		lvl	%s63
 151      00BF00BF 
 152 0210 0000003F 		vld.nc	%v63,0,%s61	# *(s)
 152      BD000081 
 153 0218 00000000 		or	%s60,1,(0)1
 153      00013C45 
 154 0220 08000000 		lea	%s59,8
 154      00003B06 
 155 0228 00000000 		adds.l	%s58,%s62,%s59
 155      BBBE3A59 
 156 0230 00000000 		lvl	%s60
 156      00BC00BF 
 157 0238 0000003E 		vld.nc	%v62,0,%s58	# *(s)
 157      BA000081 
 158 0240 00000000 		or	%s57,1,(0)1
 158      00013945 
 159 0248 10000000 		lea	%s56,16
 159      00003806 
 160 0250 00000000 		adds.l	%s55,%s62,%s56
 160      B8BE3759 
 161 0258 00000000 		lvl	%s57
 161      00B900BF 
 162 0260 0000003D 		vld.nc	%v61,0,%s55	# *(s)
 162      B7000081 
 163 0268 00000000 		or	%s54,1,(0)1
 163      00013645 
 164 0270 18000000 		lea	%s53,24
 164      00003506 
 165 0278 00000000 		adds.l	%s52,%s62,%s53
 165      B5BE3459 
 166 0280 00000000 		lvl	%s54
 166      00B600BF 
 167 0288 0000003C 		vld.nc	%v60,0,%s52	# *(s)
 167      B4000081 
 168 0290 00000000 		or	%s51,16,(0)1
 168      00103345 
 169              	# line 8
   8:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/grpsum.c **** 			s[j] += d[j*16+i];
 170              		.loc	1 8 0
 171 0298 F8FFFFFF 		ld	%s50,-8(,%fp)	# restore
 171      89003201 
 172 02a0 00000000 		or	%s49,%s50,(0)1
 172      00B23145 
 173 02a8 00000000 		lvl	%s51
 173      00B300BF 
 174 02b0 0000003B 		vld.nc	%v59,8,%s49	# *(d)
 174      B1080081 
 175 02b8 00003B3A 		vfsum.d	%v58,%v59
 175      000000EC 
 176 02c0 00000000 		or	%s48,1,(0)1
 176      00013045 
 177 02c8 00000000 		lvl	%s48
 177      00B000BF 
 178 02d0 003A3F39 		vfadd.d	%v57,%v63,%v58
 178      000000CC 
 179 02d8 E0FFFFFF 		ld	%s47,-32(,%fp)	# restore
 179      89002F01 
 180 02e0 00000000 		or	%s46,%s47,(0)1
 180      00AF2E45 
 181 02e8 00000000 		lvl	%s51
 181      00B300BF 
 182 02f0 00000038 		vld.nc	%v56,8,%s46	# *(d)
 182      AE080081 
 183 02f8 00003837 		vfsum.d	%v55,%v56
 183      000000EC 
 184 0300 00000000 		or	%s45,1,(0)1
 184      00012D45 
 185 0308 00000000 		lvl	%s45
 185      00AD00BF 
 186 0310 00373E36 		vfadd.d	%v54,%v62,%v55
 186      000000CC 
 187 0318 E8FFFFFF 		ld	%s44,-24(,%fp)	# restore
 187      89002C01 
 188 0320 00000000 		or	%s43,%s44,(0)1
 188      00AC2B45 
 189 0328 00000000 		lvl	%s51
 189      00B300BF 
 190 0330 00000035 		vld.nc	%v53,8,%s43	# *(d)
 190      AB080081 
 191 0338 00003534 		vfsum.d	%v52,%v53
 191      000000EC 
 192 0340 00000000 		or	%s42,1,(0)1
 192      00012A45 
 193 0348 00000000 		lvl	%s42
 193      00AA00BF 
 194 0350 00343D33 		vfadd.d	%v51,%v61,%v52
 194      000000CC 
 195 0358 F0FFFFFF 		ld	%s41,-16(,%fp)	# restore
 195      89002901 
 196 0360 00000000 		or	%s40,%s41,(0)1
 196      00A92845 
 197 0368 00000000 		lvl	%s51
 197      00B300BF 
 198 0370 00000032 		vld.nc	%v50,8,%s40	# *(d)
 198      A8080081 
 199 0378 00003231 		vfsum.d	%v49,%v50
 199      000000EC 
 200 0380 00000000 		or	%s39,1,(0)1
 200      00012745 
 201 0388 00000000 		lvl	%s39
 201      00A700BF 
 202 0390 00313C30 		vfadd.d	%v48,%v60,%v49
 202      000000CC 
 203              	# line 9
   9:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/grpsum.c **** 		}
 204              		.loc	1 9 0
 205 0398 00000000 		or	%s38,1,(0)1
 205      00012645 
 206 03a0 18000000 		lea	%s37,24
 206      00002506 
 207 03a8 00000000 		adds.l	%s36,%s62,%s37
 207      A5BE2459 
 208 03b0 00000000 		lvl	%s38
 208      00A600BF 
 209 03b8 00000030 		vst.nc	%v48,0,%s36	# *(s)
 209      A4000091 
 210 03c0 00000000 		or	%s35,1,(0)1
 210      00012345 
 211 03c8 00000000 		or	%s34,%s55,(0)1
 211      00B72245 
 212 03d0 00000000 		lvl	%s35
 212      00A300BF 
 213 03d8 00000033 		vst.nc	%v51,0,%s34	# *(s)
 213      A2000091 
 214 03e0 00000000 		or	%s18,1,(0)1
 214      00011245 
 215 03e8 00000000 		or	%s19,%s58,(0)1
 215      00BA1345 
 216 03f0 00000000 		lvl	%s18
 216      009200BF 
 217 03f8 00000036 		vst.nc	%v54,0,%s19	# *(s)
 217      93000091 
 218 0400 00000000 		or	%s20,1,(0)1
 218      00011445 
 219 0408 00000000 		or	%s21,%s62,(0)1
 219      00BE1545 
 220 0410 00000000 		lvl	%s20
 220      009400BF 
 221 0418 00000039 		vst.nc	%v57,0,%s21	# *(s)
 221      95000091 
 222              	# line 6
   6:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/grpsum.c **** 		for(int i=0; i<16; ++i){
 223              		.loc	1 6 0
 224 0420 00020000 		lea	%s22,512
 224      00001606 
 225 0428 00000000 		adds.l	%s47,%s22,%s47
 225      AF962F59 
 226 0430 00020000 		lea	%s23,512
 226      00001706 
 227 0438 00000000 		adds.l	%s44,%s23,%s44
 227      AC972C59 
 228 0440 00020000 		lea	%s24,512
 228      00001806 
 229 0448 00000000 		adds.l	%s41,%s24,%s41
 229      A9982959 
 230 0450 00020000 		lea	%s25,512
 230      00001906 
 231 0458 00000000 		adds.l	%s50,%s25,%s50
 231      B2993259 
 232 0460 00000000 		adds.l	%s62,32,%s62
 232      BE203E59 
 233 0468 D0FFFFFF 		ld	%s26,-48(,%fp)	# restore
 233      89001A01 
 234 0470 00000000 		adds.l	%s26,4,%s26
 234      9A041A59 
 235 0478 10000000 		brgt.l	0,%s26,.L_1.4
 235      9A000118 
 236 0480 40000000 		br.l	.L_1.6
 236      00000F18 
 237              	.L_1.4:
 238 0488 F8FFFFFF 		st	%s50,-8(,%fp)	# spill
 238      89003211 
 239 0490 F0FFFFFF 		st	%s41,-16(,%fp)	# spill
 239      89002911 
 240 0498 E8FFFFFF 		st	%s44,-24(,%fp)	# spill
 240      89002C11 
 241 04a0 E0FFFFFF 		st	%s47,-32(,%fp)	# spill
 241      89002F11 
 242 04a8 D8FFFFFF 		st	%s62,-40(,%fp)	# spill
 242      89003E11 
 243 04b0 D0FFFFFF 		st	%s26,-48(,%fp)	# spill
 243      89001A11 
 244 04b8 38FDFFFF 		br.l	.L_1.5
 244      00000F18 
 245              	.L_1.6:
 246              	# line 11
  10:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/grpsum.c **** 	}
  11:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/grpsum.c **** }
 247              		.loc	1 11 0
 248              	# Start of epilogue codes
 249 04c0 30000000 		ld	%s18,48(,%fp)
 249      89001201 
 250 04c8 38000000 		ld	%s19,56(,%fp)
 250      89001301 
 251 04d0 40000000 		ld	%s20,64(,%fp)
 251      89001401 
 252 04d8 48000000 		ld	%s21,72(,%fp)
 252      89001501 
 253 04e0 50000000 		ld	%s22,80(,%fp)
 253      89001601 
 254 04e8 58000000 		ld	%s23,88(,%fp)
 254      89001701 
 255 04f0 60000000 		ld	%s24,96(,%fp)
 255      89001801 
 256 04f8 68000000 		ld	%s25,104(,%fp)
 256      89001901 
 257 0500 70000000 		ld	%s26,112(,%fp)
 257      89001A01 
 258 0508 00000000 		or	%sp,0,%fp
 258      89000B45 
 259              		.cfi_def_cfa	11,8
 260 0510 18000000 		ld	%got,0x18(,%sp)
 260      8B000F01 
 261 0518 20000000 		ld	%plt,0x20(,%sp)
 261      8B001001 
 262 0520 08000000 		ld	%lr,0x8(,%sp)
 262      8B000A01 
 263 0528 00000000 		ld	%fp,0x0(,%sp)
 263      8B000901 
 264 0530 00000000 		b.l	(,%lr)
 264      8A000F19 
 265              	.L_1.EoE:
 266              		.cfi_endproc
 267              		.set	.L.1.2auto_size,	0xfffffffffffffea0	# 352 Bytes
 269              	# ============ End  grpsum ============
 270              	# ============ Begin  grpsum2 ============
 271 0538 00000000 		.balign 16
 271      00000000 
 272              	.L_2.0:
 273              	# line 12
  12:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/grpsum.c **** void grpsum2(double* d, double* s){
 274              		.loc	1 12 0
 275              		.globl	grpsum2
 277              	grpsum2:
 278              		.cfi_startproc
 279 0540 00000000 		st	%fp,0x0(,%sp)
 279      8B000911 
 280              		.cfi_def_cfa_offset	0
 281              		.cfi_offset	9,0
 282 0548 08000000 		st	%lr,0x8(,%sp)
 282      8B000A11 
 283 0550 18000000 		st	%got,0x18(,%sp)
 283      8B000F11 
 284 0558 20000000 		st	%plt,0x20(,%sp)
 284      8B001011 
 285 0560 00000000 		lea	%got,_GLOBAL_OFFSET_TABLE_@PC_LO(-0x18)
 285      00680F06 
 286 0568 00000000 		and	%got,%got,(32)0
 286      608F0F44 
 287 0570 00000000 		sic	%plt
 287      00001028 
 288 0578 00000000 		lea.sl	%got,_GLOBAL_OFFSET_TABLE_@PC_HI(%plt,%got)
 288      8F908F06 
 289 0580 00000000 		or	%fp,0,%sp
 289      8B000945 
 290              		.cfi_def_cfa_register	9
 291 0588 30000000 		st	%s18,48(,%fp)
 291      89001211 
 292 0590 38000000 		st	%s19,56(,%fp)
 292      89001311 
 293 0598 40000000 		st	%s20,64(,%fp)
 293      89001411 
 294 05a0 48000000 		st	%s21,72(,%fp)
 294      89001511 
 295 05a8 50000000 		st	%s22,80(,%fp)
 295      89001611 
 296 05b0 58000000 		st	%s23,88(,%fp)
 296      89001711 
 297 05b8 60000000 		st	%s24,96(,%fp)
 297      89001811 
 298 05c0 68000000 		st	%s25,104(,%fp)
 298      89001911 
 299 05c8 70000000 		st	%s26,112(,%fp)
 299      89001A11 
 300 05d0 78000000 		st	%s27,120(,%fp)
 300      89001B11 
 301 05d8 80000000 		st	%s28,128(,%fp)
 301      89001C11 
 302 05e0 88000000 		st	%s29,136(,%fp)
 302      89001D11 
 303 05e8 90000000 		st	%s30,144(,%fp)
 303      89001E11 
 304 05f0 98000000 		st	%s31,152(,%fp)
 304      89001F11 
 305 05f8 A0000000 		st	%s32,160(,%fp)
 305      89002011 
 306 0600 A8000000 		st	%s33,168(,%fp)
 306      89002111 
 307 0608 E0FDFFFF 		lea	%s13,.L.2.2auto_size&0xffffffff
 307      00000D06 
 308 0610 00000000 		and	%s13,%s13,(32)0
 308      608D0D44 
 309 0618 FFFFFFFF 		lea.sl	%sp,.L.2.2auto_size>>32(%fp,%s13)
 309      8D898B06 
 310 0620 48000000 		brge.l.t	%sp,%sl,.L_2.EoP
 310      888B3518 
 311 0628 18000000 		ld	%s61,0x18(,%tp)
 311      8E003D01 
 312 0630 00000000 		or	%s62,0,%s0
 312      80003E45 
 313 0638 3B010000 		lea	%s63,0x13b
 313      00003F06 
 314 0640 00000000 		shm.l	%s63,0x0(%s61)
 314      BD033F31 
 315 0648 08000000 		shm.l	%sl,0x8(%s61)
 315      BD030831 
 316 0650 10000000 		shm.l	%sp,0x10(%s61)
 316      BD030B31 
 317 0658 00000000 		monc
 317      0000003F 
 318 0660 00000000 		or	%s0,0,%s62
 318      BE000045 
 319              	.L_2.EoP:
 320              	# End of prologue codes
 321              	# line 14
  13:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/grpsum.c **** 	s[0] = d[0];
  14:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/grpsum.c **** 	for(int i=1; i<256; ++i){
 322              		.loc	1 14 0
 323 0668 08000000 		lea	%s63,8
 323      00003F06 
 324 0670 00000000 		adds.l	%s62,%s0,%s63
 324      BF803E59 
 325              	# line 13
  13:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/grpsum.c **** 	s[0] = d[0];
 326              		.loc	1 13 0
 327 0678 00000000 		ld	%s61,0(0,%s0)	# *(d)
 327      80003D01 
 328 0680 00000000 		st	%s61,0(0,%s1)	# *(s)
 328      81003D11 
 329              	# line 14
 330              		.loc	1 14 0
 331 0688 00000000 		adds.l	%s60,%s1,(61)1
 331      3D813C59 
 332 0690 08000000 		lea	%s59,8
 332      00003B06 
 333 0698 00000000 		adds.l	%s58,%s60,%s59
 333      BBBC3A59 
 334 06a0 FF000000 		lea	%s57,255
 334      00003906 
 335 06a8 FF000000 		lea	%s56,255
 335      00003806 
 336 06b0 B0020000 		brgt.l	%s57,%s56,.L_2.1
 336      B8B90118 
 337 06b8 08000000 		br.l	.L_2.8
 337      00000F18 
 338              	.L_2.8:
 339 06c0 00000000 		or	%s55,0,(0)1
 339      00003745 
 340 06c8 00000000 		lea	%s54,_PVWORK_STATIC@GOT_LO
 340      00003606 
 341 06d0 00000000 		and	%s54,%s54,(32)0
 341      60B63644 
 342 06d8 00000000 		lea.sl	%s54,_PVWORK_STATIC@GOT_HI(%s54,%got)
 342      8FB6B606 
 343 06e0 00000000 		ld	%s54,(,%s54)
 343      B6003601 
 344 06e8 F8FFFFFF 		st	%s58,-8(,%fp)	# spill
 344      89003A11 
 345 06f0 F0FFFFFF 		st	%s62,-16(,%fp)	# spill
 345      89003E11 
 346 06f8 E8FFFFFF 		st	%s55,-24(,%fp)	# spill
 346      89003711 
 347 0700 E0FFFFFF 		st	%s54,-32(,%fp)	# spill
 347      89003611 
 348 0708 08000000 		br.l	.L_2.9
 348      00000F18 
 349              	.L_2.9:
 350 0710 FF000000 		lea	%s63,255
 350      00003F06 
 351 0718 00000000 		adds.w.sx	%s62,0,(56)0
 351      78003E4A 
 352 0720 00000000 		lvl	%s62
 352      00BE00BF 
 353 0728 0000003F 		vseq	%v63
 353      00000099 
 354 0730 003F003E 		vor	%v62,(0)1,%v63
 354      000020C5 
 355              	# line 15
  15:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/grpsum.c **** 		if(i%16){
 356              		.loc	1 15 0
 357 0738 00000000 		lvl	%s63
 357      00BF00BF 
 358 0740 003E003D 		vadds.w.sx	%v61,1,%v62
 358      000120CA 
 359 0748 00003D3C 		vdivs.w.sx	%v60,%v61,16
 359      001010EB 
 360 0750 003C003B 		vsla.w.sx	%v59,%v60,4
 360      000420E6 
 361 0758 003E003A 		vadds.w.sx	%v58,1,%v62
 361      000120CA 
 362 0760 003B3A39 		vsubs.w.sx	%v57,%v58,%v59
 362      000000DA 
 363 0768 E0FFFFFF 		ld	%s61,-32(,%fp)	# restore
 363      89003D01 
 364 0770 00000000 		or	%s60,%s61,(0)1
 364      00BD3C45 
 365 0778 00000039 		vstl.nc	%v57,4,%s60
 365      BC040093 
 366 0780 FF000000 		lea	%s59,255
 366      00003B06 
 367 0788 00000000 		or	%s58,0,(0)1
 367      00003A45 
 368 0790 E0FFFFFF 		st	%s61,-32(,%fp)	# spill
 368      89003D11 
 369 0798 58FFFFFF 		st	%s58,-168(,%fp)	# spill
 369      89003A11 
 370 07a0 50FFFFFF 		st	%s59,-176(,%fp)	# spill
 370      89003B11 
 371 07a8 08000000 		br.l	.L_2.13
 371      00000F18 
 372              	.L_2.13:
 373 07b0 E0FFFFFF 		ld	%s63,-32(,%fp)	# restore
 373      89003F01 
 374 07b8 58FFFFFF 		ld	%s62,-168(,%fp)	# restore
 374      89003E01 
 375 07c0 00000000 		ldl.sx	%s61,0(%s62,%s63)
 375      BFBE3D03 
 376              	# line 14
  14:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/grpsum.c **** 		if(i%16){
 377              		.loc	1 14 0
 378              	# line 15
 379              		.loc	1 15 0
 380 07c8 40010000 		brne.w	0,%s61,.L_2.15
 380      BD008318 
 381 07d0 08000000 		br.l	.L_2.16
 381      00000F18 
 382              	.L_2.16:
 383              	# line 18
  16:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/grpsum.c **** 			s[i] = s[i-1] + d[i];
  17:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/grpsum.c **** 		}else{
  18:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/grpsum.c **** 			s[i] = d[i];
 384              		.loc	1 18 0
 385 07d8 F0FFFFFF 		ld	%s60,-16(,%fp)	# restore
 385      89003C01 
 386 07e0 00000000 		ld	%s59,0(0,%s60)	# *(d)
 386      BC003B01 
 387 07e8 F8FFFFFF 		ld	%s58,-8(,%fp)	# restore
 387      89003A01 
 388 07f0 08000000 		st	%s59,8(0,%s58)	# *(s)
 388      BA003B11 
 389 07f8 F8FFFFFF 		st	%s58,-8(,%fp)	# spill
 389      89003A11 
 390 0800 F0FFFFFF 		st	%s60,-16(,%fp)	# spill
 390      89003C11 
 391 0808 E0FFFFFF 		st	%s63,-32(,%fp)	# spill
 391      89003F11 
 392 0810 58FFFFFF 		st	%s62,-168(,%fp)	# spill
 392      89003E11 
 393 0818 08000000 		br.l	.L_2.17
 393      00000F18 
 394              	.L_2.17:
 395              	# line 14
  14:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/grpsum.c **** 		if(i%16){
 396              		.loc	1 14 0
 397 0820 08000000 		lea	%s63,8
 397      00003F06 
 398 0828 F8FFFFFF 		ld	%s62,-8(,%fp)	# restore
 398      89003E01 
 399 0830 00000000 		adds.l	%s62,%s62,%s63
 399      BFBE3E59 
 400 0838 08000000 		lea	%s61,8
 400      00003D06 
 401 0840 F0FFFFFF 		ld	%s60,-16(,%fp)	# restore
 401      89003C01 
 402 0848 00000000 		adds.l	%s60,%s60,%s61
 402      BDBC3C59 
 403 0850 04000000 		lea	%s59,4
 403      00003B06 
 404 0858 58FFFFFF 		ld	%s58,-168(,%fp)	# restore
 404      89003A01 
 405 0860 00000000 		adds.l	%s58,%s58,%s59
 405      BBBA3A59 
 406 0868 50FFFFFF 		ld	%s57,-176(,%fp)	# restore
 406      89003901 
 407 0870 00000000 		subs.l	%s57,%s57,(63)0
 407      7FB9395B 
 408 0878 10000000 		brlt.l	0,%s57,.L_2.14
 408      B9000218 
 409 0880 30000000 		br.l	.L_2.18
 409      00000F18 
 410              	.L_2.14:
 411 0888 F8FFFFFF 		st	%s62,-8(,%fp)	# spill
 411      89003E11 
 412 0890 F0FFFFFF 		st	%s60,-16(,%fp)	# spill
 412      89003C11 
 413 0898 58FFFFFF 		st	%s58,-168(,%fp)	# spill
 413      89003A11 
 414 08a0 50FFFFFF 		st	%s57,-176(,%fp)	# spill
 414      89003911 
 415 08a8 08FFFFFF 		br.l	.L_2.13
 415      00000F18 
 416              	.L_2.18:
 417 08b0 E8FFFFFF 		ld	%s63,-24(,%fp)	# restore
 417      89003F01 
 418 08b8 10000000 		brlt.l	0,%s63,.L_2.19
 418      BF000218 
 419 08c0 20010000 		br.l	.L_2.3
 419      00000F18 
 420              	.L_2.19:
 421 08c8 04FCFFFF 		lea	%s62,-1020
 421      00003E06 
 422 08d0 00000000 		or	%s0,0,%s62
 422      BE000045 
 423 08d8 00000000 		lea	%s12,__grow_stack@PLT_LO(-24)
 423      00680C06 
 424 08e0 00000000 		and	%s12,%s12,(32)0
 424      608C0C44 
 425 08e8 00000000 		sic	%lr
 425      00000A28 
 426 08f0 00000000 		lea.sl	%s12,__grow_stack@PLT_HI(%s12,%lr)
 426      8A8C8C06 
 427 08f8 00000000 		bsic	%lr,(,%s12)		# __grow_stack
 427      8C000A08 
 428 0900 E0000000 		br.l	.L_2.3
 428      00000F18 
 429              	.L_2.15:
 430              	# line 16
  16:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/grpsum.c **** 			s[i] = s[i-1] + d[i];
 431              		.loc	1 16 0
 432 0908 F0FFFFFF 		ld	%s61,-16(,%fp)	# restore
 432      89003D01 
 433 0910 00000000 		ld	%s60,0(0,%s61)	# *(d)
 433      BD003C01 
 434 0918 F8FFFFFF 		ld	%s59,-8(,%fp)	# restore
 434      89003B01 
 435 0920 00000000 		ld	%s58,0(0,%s59)	# *(s)
 435      BB003A01 
 436 0928 00000000 		fadd.d	%s57,%s58,%s60
 436      BCBA394C 
 437 0930 08000000 		st	%s57,8(0,%s59)	# *(s)
 437      BB003911 
 438 0938 F8FFFFFF 		st	%s59,-8(,%fp)	# spill
 438      89003B11 
 439 0940 F0FFFFFF 		st	%s61,-16(,%fp)	# spill
 439      89003D11 
 440 0948 E0FFFFFF 		st	%s63,-32(,%fp)	# spill
 440      89003F11 
 441 0950 58FFFFFF 		st	%s62,-168(,%fp)	# spill
 441      89003E11 
 442 0958 C8FEFFFF 		br.l	.L_2.17
 442      00000F18 
 443              	.L_2.1:
 444              	# line 14
  14:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/grpsum.c **** 		if(i%16){
 445              		.loc	1 14 0
 446 0960 00000000 		or	%s63,1,(0)1
 446      00013F45 
 447 0968 FC030000 		lea	%s61,1020
 447      00003D06 
 448 0970 00000000 		or	%s0,0,%s61
 448      BD000045 
 449 0978 F8FFFFFF 		st	%s58,-8(,%fp)	# spill
 449      89003A11 
 450 0980 F0FFFFFF 		st	%s62,-16(,%fp)	# spill
 450      89003E11 
 451 0988 E8FFFFFF 		st	%s63,-24(,%fp)	# spill
 451      89003F11 
 452 0990 00000000 		lea	%s12,__grow_stack@PLT_LO(-24)
 452      00680C06 
 453 0998 00000000 		and	%s12,%s12,(32)0
 453      608C0C44 
 454 09a0 00000000 		sic	%lr
 454      00000A28 
 455 09a8 00000000 		lea.sl	%s12,__grow_stack@PLT_HI(%s12,%lr)
 455      8A8C8C06 
 456 09b0 00000000 		bsic	%lr,(,%s12)		# __grow_stack
 456      8C000A08 
 457              	.L_2.10:
 458 09b8 B8000000 		lea	%s63,184
 458      00003F06 
 459 09c0 00000000 		adds.l	%s62,%sp,%s63
 459      BF8B3E59 
 460 09c8 00000000 		or	%s61,%s62,(0)1
 460      00BE3D45 
 461 09d0 E0FFFFFF 		st	%s61,-32(,%fp)	# spill
 461      89003D11 
 462 09d8 38FDFFFF 		br.l	.L_2.9
 462      00000F18 
 463              	.L_2.3:
 464 09e0 B8000000 		lea	%s63,184
 464      00003F06 
 465 09e8 00000000 		adds.l	%s62,%sp,%s63
 465      BF8B3E59 
 466              	# line 21
  19:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/grpsum.c **** 		}
  20:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/grpsum.c **** 	}
  21:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/grpsum.c **** }
 467              		.loc	1 21 0
 468              	# Start of epilogue codes
 469 09f0 00000000 		or	%sp,0,%fp
 469      89000B45 
 470              		.cfi_def_cfa	11,8
 471 09f8 18000000 		ld	%got,0x18(,%sp)
 471      8B000F01 
 472 0a00 20000000 		ld	%plt,0x20(,%sp)
 472      8B001001 
 473 0a08 08000000 		ld	%lr,0x8(,%sp)
 473      8B000A01 
 474 0a10 00000000 		ld	%fp,0x0(,%sp)
 474      8B000901 
 475 0a18 00000000 		b.l	(,%lr)
 475      8A000F19 
 476              	.L_2.EoE:
 477              		.cfi_endproc
 478              		.set	.L.2.2auto_size,	0xfffffffffffffde0	# 544 Bytes
 480              	# ============ End  grpsum2 ============
 481              	# ============ Begin  grpsum3 ============
 482              		.balign 16
 483              	.L_3.0:
 484              	# line 22
  22:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/grpsum.c **** void grpsum3(double* d, double* s){
 485              		.loc	1 22 0
 486              		.globl	grpsum3
 488              	grpsum3:
 489              		.cfi_startproc
 490 0a20 00000000 		st	%fp,0x0(,%sp)
 490      8B000911 
 491              		.cfi_def_cfa_offset	0
 492              		.cfi_offset	9,0
 493 0a28 08000000 		st	%lr,0x8(,%sp)
 493      8B000A11 
 494 0a30 18000000 		st	%got,0x18(,%sp)
 494      8B000F11 
 495 0a38 20000000 		st	%plt,0x20(,%sp)
 495      8B001011 
 496 0a40 00000000 		lea	%got,_GLOBAL_OFFSET_TABLE_@PC_LO(-0x18)
 496      00680F06 
 497 0a48 00000000 		and	%got,%got,(32)0
 497      608F0F44 
 498 0a50 00000000 		sic	%plt
 498      00001028 
 499 0a58 00000000 		lea.sl	%got,_GLOBAL_OFFSET_TABLE_@PC_HI(%plt,%got)
 499      8F908F06 
 500 0a60 00000000 		or	%fp,0,%sp
 500      8B000945 
 501              		.cfi_def_cfa_register	9
 502 0a68 30000000 		st	%s18,48(,%fp)
 502      89001211 
 503 0a70 38000000 		st	%s19,56(,%fp)
 503      89001311 
 504 0a78 40000000 		st	%s20,64(,%fp)
 504      89001411 
 505 0a80 48000000 		st	%s21,72(,%fp)
 505      89001511 
 506 0a88 50000000 		st	%s22,80(,%fp)
 506      89001611 
 507 0a90 58000000 		st	%s23,88(,%fp)
 507      89001711 
 508 0a98 60000000 		st	%s24,96(,%fp)
 508      89001811 
 509 0aa0 68000000 		st	%s25,104(,%fp)
 509      89001911 
 510 0aa8 70000000 		st	%s26,112(,%fp)
 510      89001A11 
 511 0ab0 78000000 		st	%s27,120(,%fp)
 511      89001B11 
 512 0ab8 80000000 		st	%s28,128(,%fp)
 512      89001C11 
 513 0ac0 88000000 		st	%s29,136(,%fp)
 513      89001D11 
 514 0ac8 90000000 		st	%s30,144(,%fp)
 514      89001E11 
 515 0ad0 98000000 		st	%s31,152(,%fp)
 515      89001F11 
 516 0ad8 A0000000 		st	%s32,160(,%fp)
 516      89002011 
 517 0ae0 A8000000 		st	%s33,168(,%fp)
 517      89002111 
 518 0ae8 D0FDFFFF 		lea	%s13,.L.3.2auto_size&0xffffffff
 518      00000D06 
 519 0af0 00000000 		and	%s13,%s13,(32)0
 519      608D0D44 
 520 0af8 FFFFFFFF 		lea.sl	%sp,.L.3.2auto_size>>32(%fp,%s13)
 520      8D898B06 
 521 0b00 48000000 		brge.l.t	%sp,%sl,.L_3.EoP
 521      888B3518 
 522 0b08 18000000 		ld	%s61,0x18(,%tp)
 522      8E003D01 
 523 0b10 00000000 		or	%s62,0,%s0
 523      80003E45 
 524 0b18 3B010000 		lea	%s63,0x13b
 524      00003F06 
 525 0b20 00000000 		shm.l	%s63,0x0(%s61)
 525      BD033F31 
 526 0b28 08000000 		shm.l	%sl,0x8(%s61)
 526      BD030831 
 527 0b30 10000000 		shm.l	%sp,0x10(%s61)
 527      BD030B31 
 528 0b38 00000000 		monc
 528      0000003F 
 529 0b40 00000000 		or	%s0,0,%s62
 529      BE000045 
 530              	.L_3.EoP:
 531              	# End of prologue codes
 532              	# line 25
  23:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/grpsum.c **** 	double tmp = 0.0;
  24:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/grpsum.c **** 	s[0] = tmp;
  25:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/grpsum.c **** 	for(int i=1; i<256; ++i){
 533              		.loc	1 25 0
 534 0b48 08000000 		lea	%s63,8
 534      00003F06 
 535 0b50 00000000 		adds.l	%s62,%s0,%s63
 535      BF803E59 
 536              	# line 24
  24:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/grpsum.c **** 	for(int i=1; i<256; ++i){
 537              		.loc	1 24 0
 538 0b58 00000000 		or	%s61,0,(0)1
 538      00003D45 
 539 0b60 00000000 		st	%s61,0(0,%s1)	# *(s)
 539      81003D11 
 540              	# line 25
 541              		.loc	1 25 0
 542 0b68 00000000 		adds.l	%s60,%s1,(61)1
 542      3D813C59 
 543 0b70 08000000 		lea	%s59,8
 543      00003B06 
 544 0b78 00000000 		adds.l	%s58,%s60,%s59
 544      BBBC3A59 
 545 0b80 FF000000 		lea	%s57,255
 545      00003906 
 546 0b88 FF000000 		lea	%s56,255
 546      00003806 
 547 0b90 90020000 		brgt.l	%s57,%s56,.L_3.1
 547      B8B90118 
 548 0b98 08000000 		br.l	.L_3.8
 548      00000F18 
 549              	.L_3.8:
 550 0ba0 00000000 		or	%s55,0,(0)1
 550      00003745 
 551 0ba8 00000000 		lea	%s54,_PVWORK_STATIC@GOT_LO
 551      00003606 
 552 0bb0 00000000 		and	%s54,%s54,(32)0
 552      60B63644 
 553 0bb8 00000000 		lea.sl	%s54,_PVWORK_STATIC@GOT_HI(%s54,%got)
 553      8FB6B606 
 554 0bc0 00000000 		ld	%s54,(,%s54)
 554      B6003601 
 555 0bc8 F8FFFFFF 		st	%s62,-8(,%fp)	# spill
 555      89003E11 
 556 0bd0 F0FFFFFF 		st	%s58,-16(,%fp)	# spill
 556      89003A11 
 557 0bd8 E8FFFFFF 		st	%s55,-24(,%fp)	# spill
 557      89003711 
 558 0be0 E0FFFFFF 		st	%s54,-32(,%fp)	# spill
 558      89003611 
 559 0be8 08000000 		br.l	.L_3.9
 559      00000F18 
 560              	.L_3.9:
 561 0bf0 FF000000 		lea	%s63,255
 561      00003F06 
 562 0bf8 00000000 		adds.w.sx	%s62,0,(56)0
 562      78003E4A 
 563 0c00 00000000 		lvl	%s62
 563      00BE00BF 
 564 0c08 0000003F 		vseq	%v63
 564      00000099 
 565 0c10 003F003E 		vor	%v62,(0)1,%v63
 565      000020C5 
 566              	# line 26
  26:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/grpsum.c **** 		tmp = (i%16? s[i-1]: 0.0);
 567              		.loc	1 26 0
 568 0c18 00000000 		lvl	%s63
 568      00BF00BF 
 569 0c20 003E003D 		vadds.w.sx	%v61,1,%v62
 569      000120CA 
 570 0c28 00003D3C 		vdivs.w.sx	%v60,%v61,16
 570      001010EB 
 571 0c30 003C003B 		vsla.w.sx	%v59,%v60,4
 571      000420E6 
 572 0c38 003E003A 		vadds.w.sx	%v58,1,%v62
 572      000120CA 
 573 0c40 003B3A39 		vsubs.w.sx	%v57,%v58,%v59
 573      000000DA 
 574 0c48 E0FFFFFF 		ld	%s61,-32(,%fp)	# restore
 574      89003D01 
 575 0c50 00000000 		or	%s60,%s61,(0)1
 575      00BD3C45 
 576 0c58 00000039 		vstl.nc	%v57,4,%s60
 576      BC040093 
 577 0c60 FF000000 		lea	%s59,255
 577      00003B06 
 578 0c68 00000000 		or	%s58,0,(0)1
 578      00003A45 
 579 0c70 E0FFFFFF 		st	%s61,-32(,%fp)	# spill
 579      89003D11 
 580 0c78 58FFFFFF 		st	%s58,-168(,%fp)	# spill
 580      89003A11 
 581 0c80 50FFFFFF 		st	%s59,-176(,%fp)	# spill
 581      89003B11 
 582 0c88 08000000 		br.l	.L_3.13
 582      00000F18 
 583              	.L_3.13:
 584 0c90 E0FFFFFF 		ld	%s63,-32(,%fp)	# restore
 584      89003F01 
 585 0c98 58FFFFFF 		ld	%s62,-168(,%fp)	# restore
 585      89003E01 
 586 0ca0 00000000 		ldl.sx	%s61,0(%s62,%s63)
 586      BFBE3D03 
 587              	# line 25
  25:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/grpsum.c **** 		tmp = (i%16? s[i-1]: 0.0);
 588              		.loc	1 25 0
 589              	# line 26
 590              		.loc	1 26 0
 591 0ca8 40010000 		brne.w	0,%s61,.L_3.15
 591      BD008318 
 592 0cb0 08000000 		br.l	.L_3.16
 592      00000F18 
 593              	.L_3.16:
 594 0cb8 00000000 		or	%s60,0,(0)1
 594      00003C45 
 595 0cc0 48FFFFFF 		st	%s60,-184(,%fp)	# spill
 595      89003C11 
 596 0cc8 E0FFFFFF 		st	%s63,-32(,%fp)	# spill
 596      89003F11 
 597 0cd0 58FFFFFF 		st	%s62,-168(,%fp)	# spill
 597      89003E11 
 598 0cd8 08000000 		br.l	.L_3.17
 598      00000F18 
 599              	.L_3.17:
 600              	# line 27
  27:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/grpsum.c **** 		s[i] = tmp + d[i];
 601              		.loc	1 27 0
 602 0ce0 F8FFFFFF 		ld	%s63,-8(,%fp)	# restore
 602      89003F01 
 603 0ce8 00000000 		ld	%s62,0(0,%s63)	# *(d)
 603      BF003E01 
 604 0cf0 48FFFFFF 		ld	%s61,-184(,%fp)	# restore
 604      89003D01 
 605 0cf8 00000000 		fadd.d	%s60,%s62,%s61
 605      BDBE3C4C 
 606 0d00 F0FFFFFF 		ld	%s59,-16(,%fp)	# restore
 606      89003B01 
 607 0d08 08000000 		st	%s60,8(0,%s59)	# *(s)
 607      BB003C11 
 608              	# line 25
  25:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/grpsum.c **** 		tmp = (i%16? s[i-1]: 0.0);
 609              		.loc	1 25 0
 610 0d10 08000000 		lea	%s58,8
 610      00003A06 
 611 0d18 00000000 		adds.l	%s63,%s63,%s58
 611      BABF3F59 
 612 0d20 08000000 		lea	%s57,8
 612      00003906 
 613 0d28 00000000 		adds.l	%s59,%s59,%s57
 613      B9BB3B59 
 614 0d30 04000000 		lea	%s56,4
 614      00003806 
 615 0d38 58FFFFFF 		ld	%s55,-168(,%fp)	# restore
 615      89003701 
 616 0d40 00000000 		adds.l	%s55,%s55,%s56
 616      B8B73759 
 617 0d48 50FFFFFF 		ld	%s54,-176(,%fp)	# restore
 617      89003601 
 618 0d50 00000000 		subs.l	%s54,%s54,(63)0
 618      7FB6365B 
 619 0d58 10000000 		brlt.l	0,%s54,.L_3.18
 619      B6000218 
 620 0d60 30000000 		br.l	.L_3.19
 620      00000F18 
 621              	.L_3.18:
 622 0d68 F8FFFFFF 		st	%s63,-8(,%fp)	# spill
 622      89003F11 
 623 0d70 F0FFFFFF 		st	%s59,-16(,%fp)	# spill
 623      89003B11 
 624 0d78 58FFFFFF 		st	%s55,-168(,%fp)	# spill
 624      89003711 
 625 0d80 50FFFFFF 		st	%s54,-176(,%fp)	# spill
 625      89003611 
 626 0d88 08FFFFFF 		br.l	.L_3.13
 626      00000F18 
 627              	.L_3.19:
 628 0d90 E8FFFFFF 		ld	%s63,-24(,%fp)	# restore
 628      89003F01 
 629 0d98 10000000 		brlt.l	0,%s63,.L_3.20
 629      BF000218 
 630 0da0 00010000 		br.l	.L_3.3
 630      00000F18 
 631              	.L_3.20:
 632 0da8 04FCFFFF 		lea	%s62,-1020
 632      00003E06 
 633 0db0 00000000 		or	%s0,0,%s62
 633      BE000045 
 634 0db8 00000000 		lea	%s12,__grow_stack@PLT_LO(-24)
 634      00680C06 
 635 0dc0 00000000 		and	%s12,%s12,(32)0
 635      608C0C44 
 636 0dc8 00000000 		sic	%lr
 636      00000A28 
 637 0dd0 00000000 		lea.sl	%s12,__grow_stack@PLT_HI(%s12,%lr)
 637      8A8C8C06 
 638 0dd8 00000000 		bsic	%lr,(,%s12)		# __grow_stack
 638      8C000A08 
 639 0de0 C0000000 		br.l	.L_3.3
 639      00000F18 
 640              	.L_3.15:
 641              	# line 26
  26:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/grpsum.c **** 		tmp = (i%16? s[i-1]: 0.0);
 642              		.loc	1 26 0
 643 0de8 F0FFFFFF 		ld	%s61,-16(,%fp)	# restore
 643      89003D01 
 644 0df0 00000000 		ld	%s60,0(0,%s61)	# *(s)
 644      BD003C01 
 645 0df8 48FFFFFF 		st	%s60,-184(,%fp)	# spill
 645      89003C11 
 646 0e00 F0FFFFFF 		st	%s61,-16(,%fp)	# spill
 646      89003D11 
 647 0e08 E0FFFFFF 		st	%s63,-32(,%fp)	# spill
 647      89003F11 
 648 0e10 58FFFFFF 		st	%s62,-168(,%fp)	# spill
 648      89003E11 
 649 0e18 C8FEFFFF 		br.l	.L_3.17
 649      00000F18 
 650              	.L_3.1:
 651              	# line 25
  25:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/grpsum.c **** 		tmp = (i%16? s[i-1]: 0.0);
 652              		.loc	1 25 0
 653 0e20 00000000 		or	%s63,1,(0)1
 653      00013F45 
 654 0e28 FC030000 		lea	%s61,1020
 654      00003D06 
 655 0e30 00000000 		or	%s0,0,%s61
 655      BD000045 
 656 0e38 F8FFFFFF 		st	%s62,-8(,%fp)	# spill
 656      89003E11 
 657 0e40 F0FFFFFF 		st	%s58,-16(,%fp)	# spill
 657      89003A11 
 658 0e48 E8FFFFFF 		st	%s63,-24(,%fp)	# spill
 658      89003F11 
 659 0e50 00000000 		lea	%s12,__grow_stack@PLT_LO(-24)
 659      00680C06 
 660 0e58 00000000 		and	%s12,%s12,(32)0
 660      608C0C44 
 661 0e60 00000000 		sic	%lr
 661      00000A28 
 662 0e68 00000000 		lea.sl	%s12,__grow_stack@PLT_HI(%s12,%lr)
 662      8A8C8C06 
 663 0e70 00000000 		bsic	%lr,(,%s12)		# __grow_stack
 663      8C000A08 
 664              	.L_3.10:
 665 0e78 B8000000 		lea	%s63,184
 665      00003F06 
 666 0e80 00000000 		adds.l	%s62,%sp,%s63
 666      BF8B3E59 
 667 0e88 00000000 		or	%s61,%s62,(0)1
 667      00BE3D45 
 668 0e90 E0FFFFFF 		st	%s61,-32(,%fp)	# spill
 668      89003D11 
 669 0e98 58FDFFFF 		br.l	.L_3.9
 669      00000F18 
 670              	.L_3.3:
 671 0ea0 B8000000 		lea	%s63,184
 671      00003F06 
 672 0ea8 00000000 		adds.l	%s62,%sp,%s63
 672      BF8B3E59 
 673              	# line 29
  28:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/grpsum.c **** 	}
  29:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/grpsum.c **** }
 674              		.loc	1 29 0
 675              	# Start of epilogue codes
 676 0eb0 00000000 		or	%sp,0,%fp
 676      89000B45 
 677              		.cfi_def_cfa	11,8
 678 0eb8 18000000 		ld	%got,0x18(,%sp)
 678      8B000F01 
 679 0ec0 20000000 		ld	%plt,0x20(,%sp)
 679      8B001001 
 680 0ec8 08000000 		ld	%lr,0x8(,%sp)
 680      8B000A01 
 681 0ed0 00000000 		ld	%fp,0x0(,%sp)
 681      8B000901 
 682 0ed8 00000000 		b.l	(,%lr)
 682      8A000F19 
 683              	.L_3.EoE:
 684              		.cfi_endproc
 685              		.set	.L.3.2auto_size,	0xfffffffffffffdd0	# 560 Bytes
 687              	# ============ End  grpsum3 ============
 688              	# ============ Begin  grpsum4 ============
 689              		.balign 16
 690              	.L_4.0:
 691              	# line 32
  30:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/grpsum.c **** int const msk0[256]={[0]=1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1,1};
  31:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/grpsum.c **** int const msk1[256]={[16]=1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1,1};
  32:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/grpsum.c **** void grpsum4(double* d, double* s){
 692              		.loc	1 32 0
 693              		.globl	grpsum4
 695              	grpsum4:
 696              		.cfi_startproc
 697 0ee0 00000000 		st	%fp,0x0(,%sp)
 697      8B000911 
 698              		.cfi_def_cfa_offset	0
 699              		.cfi_offset	9,0
 700 0ee8 08000000 		st	%lr,0x8(,%sp)
 700      8B000A11 
 701 0ef0 18000000 		st	%got,0x18(,%sp)
 701      8B000F11 
 702 0ef8 20000000 		st	%plt,0x20(,%sp)
 702      8B001011 
 703 0f00 00000000 		lea	%got,_GLOBAL_OFFSET_TABLE_@PC_LO(-0x18)
 703      00680F06 
 704 0f08 00000000 		and	%got,%got,(32)0
 704      608F0F44 
 705 0f10 00000000 		sic	%plt
 705      00001028 
 706 0f18 00000000 		lea.sl	%got,_GLOBAL_OFFSET_TABLE_@PC_HI(%plt,%got)
 706      8F908F06 
 707 0f20 00000000 		or	%fp,0,%sp
 707      8B000945 
 708              		.cfi_def_cfa_register	9
 709 0f28 30000000 		st	%s18,48(,%fp)
 709      89001211 
 710 0f30 38000000 		st	%s19,56(,%fp)
 710      89001311 
 711 0f38 40000000 		st	%s20,64(,%fp)
 711      89001411 
 712 0f40 48000000 		st	%s21,72(,%fp)
 712      89001511 
 713 0f48 50000000 		st	%s22,80(,%fp)
 713      89001611 
 714 0f50 58000000 		st	%s23,88(,%fp)
 714      89001711 
 715 0f58 60000000 		st	%s24,96(,%fp)
 715      89001811 
 716 0f60 68000000 		st	%s25,104(,%fp)
 716      89001911 
 717 0f68 70000000 		st	%s26,112(,%fp)
 717      89001A11 
 718 0f70 78000000 		st	%s27,120(,%fp)
 718      89001B11 
 719 0f78 80000000 		st	%s28,128(,%fp)
 719      89001C11 
 720 0f80 88000000 		st	%s29,136(,%fp)
 720      89001D11 
 721 0f88 90000000 		st	%s30,144(,%fp)
 721      89001E11 
 722 0f90 98000000 		st	%s31,152(,%fp)
 722      89001F11 
 723 0f98 A0000000 		st	%s32,160(,%fp)
 723      89002011 
 724 0fa0 A8000000 		st	%s33,168(,%fp)
 724      89002111 
 725 0fa8 00000000 		lea	%s13,.L.4.2auto_size&0xffffffff
 725      00000D06 
 726 0fb0 00000000 		and	%s13,%s13,(32)0
 726      608D0D44 
 727 0fb8 00000000 		lea.sl	%sp,.L.4.2auto_size>>32(%fp,%s13)
 727      8D898B06 
 728 0fc0 48000000 		brge.l.t	%sp,%sl,.L_4.EoP
 728      888B3518 
 729 0fc8 18000000 		ld	%s61,0x18(,%tp)
 729      8E003D01 
 730 0fd0 00000000 		or	%s62,0,%s0
 730      80003E45 
 731 0fd8 3B010000 		lea	%s63,0x13b
 731      00003F06 
 732 0fe0 00000000 		shm.l	%s63,0x0(%s61)
 732      BD033F31 
 733 0fe8 08000000 		shm.l	%sl,0x8(%s61)
 733      BD030831 
 734 0ff0 10000000 		shm.l	%sp,0x10(%s61)
 734      BD030B31 
 735 0ff8 00000000 		monc
 735      0000003F 
 736 1000 00000000 		or	%s0,0,%s62
 736      BE000045 
 737              	.L_4.EoP:
 738              	# End of prologue codes
 739              	# line 35
  33:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/grpsum.c **** 	double tmp =0.0;
  34:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/grpsum.c **** 	for(int i=0; i<256; ++i){
  35:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/grpsum.c **** 		if(msk0[i]) tmp += d[i];
 740              		.loc	1 35 0
 741 1008 00000000 		lea	%s63,msk0@GOT_LO
 741      00003F06 
 742 1010 00000000 		and	%s63,%s63,(32)0
 742      60BF3F44 
 743 1018 00000000 		lea.sl	%s63,msk0@GOT_HI(%s63,%got)
 743      8FBFBF06 
 744 1020 00000000 		ld	%s63,(,%s63)
 744      BF003F01 
 745 1028 00010000 		lea	%s62,256
 745      00003E06 
 746 1030 00000000 		or	%s61,0,(0)1
 746      00003D45 
 747 1038 00000000 		lvl	%s62
 747      00BE00BF 
 748 1040 0000003F 		vbrd	%v63,%s61
 748      00BD008C 
 749 1048 00000000 		or	%s60,%s63,(0)1
 749      00BF3C45 
 750 1050 0000003E 		vldl.sx.nc	%v62,4,%s60	# msk0
 750      BC040083 
 751 1058 003E003D 		vcmps.w.sx	%v61,0,%v62
 751      000020FA 
 752 1060 003D0301 		vfmk.w.ne	%vm1,%v61
 752      000000B5 
 753 1068 00000000 		or	%s59,%s0,(0)1
 753      00803B45 
 754 1070 0000003C 		vld.nc	%v60,8,%s59	# *(d)
 754      BB080081 
 755 1078 003C3F3F 		vfadd.d	%v63,%v63,%v60,%vm1
 755      000001CC 
 756 1080 00003F3B 		vfsum.d	%v59,%v63
 756      000000EC 
 757 1088 0000003B 		lvs	%s58,%v59(0)
 757      00003A9E 
 758              	# line 37
  36:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/grpsum.c **** 	}
  37:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/grpsum.c **** 	s[0] += tmp;
 759              		.loc	1 37 0
 760 1090 00000000 		ld	%s57,0(0,%s1)	# *(s)
 760      81003901 
 761 1098 00000000 		fadd.d	%s56,%s57,%s58
 761      BAB9384C 
 762 10a0 00000000 		st	%s56,0(0,%s1)	# *(s)
 762      81003811 
 763              	# line 40
  38:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/grpsum.c **** 	tmp =0.0;
  39:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/grpsum.c **** 	for(int i=0; i<256; ++i){
  40:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/grpsum.c **** 		if(msk1[i]) tmp += d[i];
 764              		.loc	1 40 0
 765 10a8 00000000 		lea	%s55,msk1@GOT_LO
 765      00003706 
 766 10b0 00000000 		and	%s55,%s55,(32)0
 766      60B73744 
 767 10b8 00000000 		lea.sl	%s55,msk1@GOT_HI(%s55,%got)
 767      8FB7B706 
 768 10c0 00000000 		ld	%s55,(,%s55)
 768      B7003701 
 769 10c8 00010000 		lea	%s54,256
 769      00003606 
 770 10d0 00000000 		or	%s53,0,(0)1
 770      00003545 
 771 10d8 00000000 		lvl	%s54
 771      00B600BF 
 772 10e0 0000003A 		vbrd	%v58,%s53
 772      00B5008C 
 773 10e8 00000000 		or	%s52,%s55,(0)1
 773      00B73445 
 774 10f0 00000039 		vldl.sx.nc	%v57,4,%s52	# msk1
 774      B4040083 
 775 10f8 00390038 		vcmps.w.sx	%v56,0,%v57
 775      000020FA 
 776 1100 00380302 		vfmk.w.ne	%vm2,%v56
 776      000000B5 
 777 1108 00000000 		or	%s51,%s0,(0)1
 777      00803345 
 778 1110 00000037 		vld.nc	%v55,8,%s51	# *(d)
 778      B3080081 
 779 1118 00373A3A 		vfadd.d	%v58,%v58,%v55,%vm2
 779      000002CC 
 780 1120 00003A36 		vfsum.d	%v54,%v58
 780      000000EC 
 781 1128 00000036 		lvs	%s50,%v54(0)
 781      0000329E 
 782              	# line 42
  41:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/grpsum.c **** 	}
  42:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/grpsum.c **** 	s[1] += tmp;
 783              		.loc	1 42 0
 784 1130 08000000 		ld	%s49,8(0,%s1)	# *(s)
 784      81003101 
 785 1138 00000000 		fadd.d	%s48,%s49,%s50
 785      B2B1304C 
 786 1140 08000000 		st	%s48,8(0,%s1)	# *(s)
 786      81003011 
 787              	# line 43
  43:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/grpsum.c **** }
 788              		.loc	1 43 0
 789              	# Start of epilogue codes
 790 1148 00000000 		or	%sp,0,%fp
 790      89000B45 
 791              		.cfi_def_cfa	11,8
 792 1150 18000000 		ld	%got,0x18(,%sp)
 792      8B000F01 
 793 1158 20000000 		ld	%plt,0x20(,%sp)
 793      8B001001 
 794 1160 08000000 		ld	%lr,0x8(,%sp)
 794      8B000A01 
 795 1168 00000000 		ld	%fp,0x0(,%sp)
 795      8B000901 
 796 1170 00000000 		b.l	(,%lr)
 796      8A000F19 
 797              	.L_4.EoE:
 798              		.cfi_endproc
 799              		.set	.L.4.2auto_size,	0x0	# 0 Bytes
 801              	# ============ End  grpsum4 ============
 802              	# ============ Begin  grpsum5 ============
 803 1178 00000000 		.balign 16
 803      00000000 
 804              	.L_5.0:
 805              	# line 50
  44:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/grpsum.c **** 
  45:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/grpsum.c **** unsigned long grp16[4]=
  46:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/grpsum.c **** { 0x000000000000ffffUL,
  47:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/grpsum.c **** 	0x00000000ffff0000UL,
  48:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/grpsum.c **** 	0x0000ffff00000000UL,
  49:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/grpsum.c **** 	0xffff000000000000UL };
  50:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/grpsum.c **** void grpsum5(double* d, double* s){
 806              		.loc	1 50 0
 807              		.globl	grpsum5
 809              	grpsum5:
 810              		.cfi_startproc
 811 1180 00000000 		st	%fp,0x0(,%sp)
 811      8B000911 
 812              		.cfi_def_cfa_offset	0
 813              		.cfi_offset	9,0
 814 1188 08000000 		st	%lr,0x8(,%sp)
 814      8B000A11 
 815 1190 18000000 		st	%got,0x18(,%sp)
 815      8B000F11 
 816 1198 20000000 		st	%plt,0x20(,%sp)
 816      8B001011 
 817 11a0 00000000 		lea	%got,_GLOBAL_OFFSET_TABLE_@PC_LO(-0x18)
 817      00680F06 
 818 11a8 00000000 		and	%got,%got,(32)0
 818      608F0F44 
 819 11b0 00000000 		sic	%plt
 819      00001028 
 820 11b8 00000000 		lea.sl	%got,_GLOBAL_OFFSET_TABLE_@PC_HI(%plt,%got)
 820      8F908F06 
 821 11c0 00000000 		or	%fp,0,%sp
 821      8B000945 
 822              		.cfi_def_cfa_register	9
 823 11c8 30000000 		st	%s18,48(,%fp)
 823      89001211 
 824 11d0 38000000 		st	%s19,56(,%fp)
 824      89001311 
 825 11d8 40000000 		st	%s20,64(,%fp)
 825      89001411 
 826 11e0 48000000 		st	%s21,72(,%fp)
 826      89001511 
 827 11e8 50000000 		st	%s22,80(,%fp)
 827      89001611 
 828 11f0 58000000 		st	%s23,88(,%fp)
 828      89001711 
 829 11f8 60000000 		st	%s24,96(,%fp)
 829      89001811 
 830 1200 68000000 		st	%s25,104(,%fp)
 830      89001911 
 831 1208 70000000 		st	%s26,112(,%fp)
 831      89001A11 
 832 1210 78000000 		st	%s27,120(,%fp)
 832      89001B11 
 833 1218 80000000 		st	%s28,128(,%fp)
 833      89001C11 
 834 1220 88000000 		st	%s29,136(,%fp)
 834      89001D11 
 835 1228 90000000 		st	%s30,144(,%fp)
 835      89001E11 
 836 1230 98000000 		st	%s31,152(,%fp)
 836      89001F11 
 837 1238 A0000000 		st	%s32,160(,%fp)
 837      89002011 
 838 1240 A8000000 		st	%s33,168(,%fp)
 838      89002111 
 839 1248 E0FFFFFF 		lea	%s13,.L.5.2auto_size&0xffffffff
 839      00000D06 
 840 1250 00000000 		and	%s13,%s13,(32)0
 840      608D0D44 
 841 1258 FFFFFFFF 		lea.sl	%sp,.L.5.2auto_size>>32(%fp,%s13)
 841      8D898B06 
 842 1260 48000000 		brge.l.t	%sp,%sl,.L_5.EoP
 842      888B3518 
 843 1268 18000000 		ld	%s61,0x18(,%tp)
 843      8E003D01 
 844 1270 00000000 		or	%s62,0,%s0
 844      80003E45 
 845 1278 3B010000 		lea	%s63,0x13b
 845      00003F06 
 846 1280 00000000 		shm.l	%s63,0x0(%s61)
 846      BD033F31 
 847 1288 08000000 		shm.l	%sl,0x8(%s61)
 847      BD030831 
 848 1290 10000000 		shm.l	%sp,0x10(%s61)
 848      BD030B31 
 849 1298 00000000 		monc
 849      0000003F 
 850 12a0 00000000 		or	%s0,0,%s62
 850      BE000045 
 851              	.L_5.EoP:
 852              	# End of prologue codes
 853              	# line 53
  51:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/grpsum.c **** 	long msk[4];
  52:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/grpsum.c **** 	for(int i=0; i<4; ++i){
  53:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/grpsum.c **** 		msk[i] = 0;
 854              		.loc	1 53 0
 855 12a8 00000000 		or	%s63,0,(0)1
 855      00003F45 
 856 12b0 E8FFFFFF 		st	%s63,-24(0,%fp)	# msk
 856      89003F11 
 857 12b8 00000000 		or	%s63,0,(0)1
 857      00003F45 
 858 12c0 F0FFFFFF 		st	%s63,-16(0,%fp)	# msk
 858      89003F11 
 859 12c8 00000000 		or	%s63,0,(0)1
 859      00003F45 
 860 12d0 F8FFFFFF 		st	%s63,-8(0,%fp)	# msk
 860      89003F11 
 861              	# line 55
  54:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/grpsum.c **** 	}
  55:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/grpsum.c **** 	msk[0] = grp16[0];
 862              		.loc	1 55 0
 863 12d8 00000000 		lea	%s63,grp16@GOT_LO
 863      00003F06 
 864 12e0 00000000 		and	%s63,%s63,(32)0
 864      60BF3F44 
 865 12e8 00000000 		lea.sl	%s63,grp16@GOT_HI(%s63,%got)
 865      8FBFBF06 
 866 12f0 00000000 		ld	%s63,(,%s63)
 866      BF003F01 
 867 12f8 00000000 		ld	%s62,0(0,%s63)	# grp16
 867      BF003E01 
 868 1300 00000000 		or	%s61,%s62,(0)1
 868      00BE3D45 
 869 1308 E0FFFFFF 		st	%s61,-32(0,%fp)	# msk
 869      89003D11 
 870              	# line 59
  56:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/grpsum.c **** 	double tmp1= 0.0;
  57:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/grpsum.c **** 	for(int i=0; i<256; ++i){
  58:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/grpsum.c **** 		if(msk[i/64] & (1<<i%64)){
  59:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/grpsum.c **** 			tmp1 += d[i];
 871              		.loc	1 59 0
 872 1310 00010000 		lea	%s60,256
 872      00003C06 
 873 1318 00000000 		or	%s59,0,(0)1
 873      00003B45 
 874 1320 00000000 		lvl	%s60
 874      00BC00BF 
 875 1328 0000003F 		vbrd	%v63,%s59
 875      00BB008C 
 876 1330 00010000 		lea	%s58,256
 876      00003A06 
 877 1338 00000000 		adds.w.sx	%s57,%s58,(0)1
 877      00BA394A 
 878 1340 00000000 		lvl	%s57
 878      00B900BF 
 879 1348 0000003E 		vseq	%v62
 879      00000099 
 880 1350 003E003D 		vor	%v61,(0)1,%v62
 880      000020C5 
 881              	# line 58
  58:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/grpsum.c **** 			tmp1 += d[i];
 882              		.loc	1 58 0
 883 1358 00000000 		lvl	%s60
 883      00BC00BF 
 884 1360 003D003C 		vadds.w.sx	%v60,0,%v61
 884      000020CA 
 885 1368 40000000 		lea	%s56,64
 885      00003806 
 886 1370 00003C3B 		vdivs.w.sx	%v59,%v60,%s56
 886      00B810EB 
 887 1378 003B003A 		vor	%v58,(0)1,%v59
 887      000020C5 
 888 1380 00000000 		adds.l	%s55,-32,%fp
 888      89603759 
 889 1388 003A0039 		vsfa	%v57,%v58,3,%s55
 889      B70300D7 
 890 1390 00003938 		vgt	%v56,%v57,0,0
 890      000040A1 
 891 1398 003B0037 		vsla.w.sx	%v55,%v59,6
 891      000620E6 
 892 13a0 003D0036 		vadds.w.sx	%v54,0,%v61
 892      000020CA 
 893 13a8 00373635 		vsubs.w.sx	%v53,%v54,%v55
 893      000000DA 
 894 13b0 00000034 		vbrd	%v52,1
 894      0001008C 
 895 13b8 00343533 		vsll	%v51,%v52,%v53
 895      000000E5 
 896 13c0 00330032 		vor	%v50,(0)1,%v51
 896      000020C5 
 897 13c8 00323831 		vand	%v49,%v56,%v50
 897      000000C4 
 898 13d0 00310030 		vcmps.l	%v48,0,%v49
 898      000020BA 
 899 13d8 00300301 		vfmk.l.ne	%vm1,%v48
 899      000000B4 
 900              	# line 59
 901              		.loc	1 59 0
 902 13e0 00000000 		or	%s54,%s0,(0)1
 902      00803645 
 903 13e8 0000002F 		vld.nc	%v47,8,%s54	# *(d)
 903      B6080081 
 904 13f0 002F3F3F 		vfadd.d	%v63,%v63,%v47,%vm1
 904      000001CC 
 905 13f8 00003F2E 		vfsum.d	%v46,%v63
 905      000000EC 
 906 1400 00000000 		or	%s53,1,(0)1
 906      00013545 
 907 1408 00000000 		or	%s52,0,(0)1
 907      00003445 
 908 1410 00000000 		lvl	%s53
 908      00B500BF 
 909 1418 002E002D 		vfadd.d	%v45,%s52,%v46
 909      00B420CC 
 910              	# line 62
  60:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/grpsum.c **** 		}
  61:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/grpsum.c **** 	}
  62:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/grpsum.c **** 	s[0] = tmp1;
 911              		.loc	1 62 0
 912 1420 00000000 		or	%s51,1,(0)1
 912      00013345 
 913 1428 00000000 		or	%s50,%s1,(0)1
 913      00813245 
 914 1430 00000000 		lvl	%s51
 914      00B300BF 
 915 1438 0000002D 		vst.nc	%v45,0,%s50	# *(s)
 915      B2000091 
 916              	# line 63
  63:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/grpsum.c **** }
 917              		.loc	1 63 0
 918              	# Start of epilogue codes
 919 1440 00000000 		or	%sp,0,%fp
 919      89000B45 
 920              		.cfi_def_cfa	11,8
 921 1448 18000000 		ld	%got,0x18(,%sp)
 921      8B000F01 
 922 1450 20000000 		ld	%plt,0x20(,%sp)
 922      8B001001 
 923 1458 08000000 		ld	%lr,0x8(,%sp)
 923      8B000A01 
 924 1460 00000000 		ld	%fp,0x0(,%sp)
 924      8B000901 
 925 1468 00000000 		b.l	(,%lr)
 925      8A000F19 
 926              	.L_5.EoE:
 927              		.cfi_endproc
 928              		.set	.L.5.2auto_size,	0xffffffffffffffe0	# 32 Bytes
 930              	# ============ End  grpsum5 ============
 931              	.comm _PVWORK_STATIC,1020,16
 932              	.Le1.0:
