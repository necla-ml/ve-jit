   1              		.ident "ncc 1.1.4 (Build 10:17:22 Apr 19 2018)"
   2              		.file "cpp.c"
   3              		.file 1 "/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/cpp.c"
   4              		.file 2 "/opt/nec/ve/ncc/1.1.4/include/stdc-predef.h"
   5              		.file 3 "/opt/nec/ve/ncc/1.1.4/include/stdio.h"
   6              		.file 4 "/opt/nec/ve/ncc/1.1.4/include/yvals.h"
   7              		.file 5 "/opt/nec/ve/ncc/1.1.4/include/necvals.h"
   8              		.file 6 "/opt/nec/ve/ncc/1.1.4/include/stdarg.h"
   9              		.file 7 "/opt/nec/ve/musl/include/stdio.h"
  10              		.file 8 "/opt/nec/ve/musl/include/features.h"
  11              		.file 9 "/opt/nec/ve/musl/include/bits/alltypes.h"
  12              	# ============ Begin  main ============
  13              		.section .rodata
  14              		.balign 16
  16              	.LP.__string.13:
  17 0000 20       		.byte	32
  18 0001 41       		.byte	65
  19 0002 00       		.zero	1
  20 0003 00000000 		.balign 8
  20      00
  22              	.LP.__string.12:
  23 0008 25       		.byte	37
  24 0009 73       		.byte	115
  25 000a 00       		.zero	1
  26 000b 00000000 		.balign 8
  26      00
  28              	.LP.__string.11:
  29 0010 25       		.byte	37
  30 0011 64       		.byte	100
  31 0012 25       		.byte	37
  32 0013 64       		.byte	100
  33 0014 00       		.zero	1
  34 0015 000000   		.balign 8
  36              	.LP.__string.0:
  37 0018 25       		.byte	37
  38 0019 73       		.byte	115
  39 001a 00       		.zero	1
  40 001b 00000000 		.balign 8
  40      00
  42              	.LP.__string.14:
  43 0020 25       		.byte	37
  44 0021 64       		.byte	100
  45 0022 25       		.byte	37
  46 0023 64       		.byte	100
  47 0024 00       		.zero	1
  48 0025 000000   		.balign 8
  50              	.LP.__string.1:
  51 0028 20       		.byte	32
  52 0029 42       		.byte	66
  53 002a 78       		.byte	120
  54 002b 00       		.zero	1
  55 002c 00000000 		.balign 8
  57              	.LP.__string.9:
  58 0030 25       		.byte	37
  59 0031 73       		.byte	115
  60 0032 00       		.zero	1
  61 0033 00000000 		.balign 8
  61      00
  63              	.LP.__string.10:
  64 0038 20       		.byte	32
  65 0039 41       		.byte	65
  66 003a 00       		.zero	1
  67 003b 00000000 		.balign 8
  67      00
  69              	.LP.__string.7:
  70 0040 25       		.byte	37
  71 0041 73       		.byte	115
  72 0042 00       		.zero	1
  73 0043 00000000 		.balign 8
  73      00
  75              	.LP.__string.8:
  76 0048 20       		.byte	32
  77 0049 42       		.byte	66
  78 004a 77       		.byte	119
  79 004b 6F       		.byte	111
  80 004c 72       		.byte	114
  81 004d 6C       		.byte	108
  82 004e 64       		.byte	100
  83 004f 00       		.zero	1
  84              		.balign 8
  86              	.LP.__string.5:
  87 0050 25       		.byte	37
  88 0051 73       		.byte	115
  89 0052 00       		.zero	1
  90 0053 00000000 		.balign 8
  90      00
  92              	.LP.__string.6:
  93 0058 20       		.byte	32
  94 0059 43       		.byte	67
  95 005a 79       		.byte	121
  96 005b 00       		.zero	1
  97 005c 00000000 		.balign 8
  99              	.LP.__string.4:
 100 0060 48       		.byte	72
 101 0061 65       		.byte	101
 102 0062 6C       		.byte	108
 103 0063 6C       		.byte	108
 104 0064 6F       		.byte	111
 105 0065 00       		.zero	1
 106 0066 0000     		.balign 8
 108              	.LP.__string.2:
 109 0068 25       		.byte	37
 110 0069 73       		.byte	115
 111 006a 00       		.zero	1
 112 006b 00000000 		.balign 8
 112      00
 114              	.LP.__string.3:
 115 0070 20       		.byte	32
 116 0071 41       		.byte	65
 117 0072 00       		.zero	1
 118              		.text
 119              		.balign 16
 120              	.L_1.0:
 121              	# line 7
   1:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/cpp.c **** #include <stdio.h>
   2:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/cpp.c **** #define A(...) printf("%s"," A"); printf(__VA_ARGS__);
   3:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/cpp.c **** #define B(s,...) printf("%s"," B" #s); A(__VA_ARGS__)
   4:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/cpp.c **** #define C(s,...) printf("%s"," C" #s); B(world,__VA_ARGS__)
   5:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/cpp.c **** #define D(b,c,...) B(b,"Hello") C(c,__VA_ARGS__) A(__VA_ARGS__)
   6:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/cpp.c **** 
   7:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/cpp.c **** int main(int argc, char**argv){
 122              		.loc	1 7 0
 123              		.globl	main
 125              	main:
 126              		.cfi_startproc
 127 0000 00000000 		st	%fp,0x0(,%sp)
 127      8B000911 
 128              		.cfi_def_cfa_offset	0
 129              		.cfi_offset	9,0
 130 0008 08000000 		st	%lr,0x8(,%sp)
 130      8B000A11 
 131 0010 18000000 		st	%got,0x18(,%sp)
 131      8B000F11 
 132 0018 20000000 		st	%plt,0x20(,%sp)
 132      8B001011 
 133 0020 00000000 		lea	%got,_GLOBAL_OFFSET_TABLE_@PC_LO(-0x18)
 133      00680F06 
 134 0028 00000000 		and	%got,%got,(32)0
 134      608F0F44 
 135 0030 00000000 		sic	%plt
 135      00001028 
 136 0038 00000000 		lea.sl	%got,_GLOBAL_OFFSET_TABLE_@PC_HI(%plt,%got)
 136      8F908F06 
 137 0040 00000000 		or	%fp,0,%sp
 137      8B000945 
 138              		.cfi_def_cfa_register	9
 139 0048 30000000 		st	%s18,48(,%fp)
 139      89001211 
 140 0050 38000000 		st	%s19,56(,%fp)
 140      89001311 
 141 0058 40000000 		st	%s20,64(,%fp)
 141      89001411 
 142 0060 48000000 		st	%s21,72(,%fp)
 142      89001511 
 143 0068 50000000 		st	%s22,80(,%fp)
 143      89001611 
 144 0070 58000000 		st	%s23,88(,%fp)
 144      89001711 
 145 0078 60000000 		st	%s24,96(,%fp)
 145      89001811 
 146 0080 68000000 		st	%s25,104(,%fp)
 146      89001911 
 147 0088 70000000 		st	%s26,112(,%fp)
 147      89001A11 
 148 0090 78000000 		st	%s27,120(,%fp)
 148      89001B11 
 149 0098 80000000 		st	%s28,128(,%fp)
 149      89001C11 
 150 00a0 88000000 		st	%s29,136(,%fp)
 150      89001D11 
 151 00a8 90000000 		st	%s30,144(,%fp)
 151      89001E11 
 152 00b0 98000000 		st	%s31,152(,%fp)
 152      89001F11 
 153 00b8 A0000000 		st	%s32,160(,%fp)
 153      89002011 
 154 00c0 A8000000 		st	%s33,168(,%fp)
 154      89002111 
 155 00c8 30FEFFFF 		lea	%s13,.L.1.2auto_size&0xffffffff
 155      00000D06 
 156 00d0 00000000 		and	%s13,%s13,(32)0
 156      608D0D44 
 157 00d8 FFFFFFFF 		lea.sl	%sp,.L.1.2auto_size>>32(%fp,%s13)
 157      8D898B06 
 158 00e0 48000000 		brge.l.t	%sp,%sl,.L_1.EoP
 158      888B3518 
 159 00e8 18000000 		ld	%s61,0x18(,%tp)
 159      8E003D01 
 160 00f0 00000000 		or	%s62,0,%s0
 160      80003E45 
 161 00f8 3B010000 		lea	%s63,0x13b
 161      00003F06 
 162 0100 00000000 		shm.l	%s63,0x0(%s61)
 162      BD033F31 
 163 0108 08000000 		shm.l	%sl,0x8(%s61)
 163      BD030831 
 164 0110 10000000 		shm.l	%sp,0x10(%s61)
 164      BD030B31 
 165 0118 00000000 		monc
 165      0000003F 
 166 0120 00000000 		or	%s0,0,%s62
 166      BE000045 
 167              	.L_1.EoP:
 168              	# End of prologue codes
 169              	# line 10
   8:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/cpp.c **** 	int x = 1;
   9:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/cpp.c **** 	int y = 2;
  10:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/cpp.c **** 	D(x,y,"%d%d",x,y);
 170              		.loc	1 10 0
 171 0128 00000000 		lea	%s63,.LP.__string.0@GOTOFF_LO
 171      00003F06 
 172 0130 00000000 		and	%s63,%s63,(32)0
 172      60BF3F44 
 173 0138 00000000 		lea.sl	%s63,.LP.__string.0@GOTOFF_HI(%s63,%got)
 173      8FBFBF06 
 174 0140 B0000000 		st	%s63,176(0,%sp)
 174      8B003F11 
 175 0148 00000000 		lea	%s62,.LP.__string.1@GOTOFF_LO
 175      00003E06 
 176 0150 00000000 		and	%s62,%s62,(32)0
 176      60BE3E44 
 177 0158 00000000 		lea.sl	%s62,.LP.__string.1@GOTOFF_HI(%s62,%got)
 177      8FBEBE06 
 178 0160 B8000000 		st	%s62,184(0,%sp)
 178      8B003E11 
 179 0168 00000000 		or	%s0,0,%s63
 179      BF000045 
 180 0170 00000000 		or	%s1,0,%s62
 180      BE000145 
 181 0178 00000000 		lea	%s12,printf@PLT_LO(-24)
 181      00680C06 
 182 0180 00000000 		and	%s12,%s12,(32)0
 182      608C0C44 
 183 0188 00000000 		sic	%lr
 183      00000A28 
 184 0190 00000000 		lea.sl	%s12,printf@PLT_HI(%s12,%lr)
 184      8A8C8C06 
 185 0198 00000000 		bsic	%lr,(,%s12)		# printf
 185      8C000A08 
 186              	.L_1.9:
 187 01a0 00000000 		lea	%s63,.LP.__string.2@GOTOFF_LO
 187      00003F06 
 188 01a8 00000000 		and	%s63,%s63,(32)0
 188      60BF3F44 
 189 01b0 00000000 		lea.sl	%s63,.LP.__string.2@GOTOFF_HI(%s63,%got)
 189      8FBFBF06 
 190 01b8 B0000000 		st	%s63,176(0,%sp)
 190      8B003F11 
 191 01c0 00000000 		lea	%s62,.LP.__string.3@GOTOFF_LO
 191      00003E06 
 192 01c8 00000000 		and	%s62,%s62,(32)0
 192      60BE3E44 
 193 01d0 00000000 		lea.sl	%s62,.LP.__string.3@GOTOFF_HI(%s62,%got)
 193      8FBEBE06 
 194 01d8 B8000000 		st	%s62,184(0,%sp)
 194      8B003E11 
 195 01e0 00000000 		or	%s0,0,%s63
 195      BF000045 
 196 01e8 00000000 		or	%s1,0,%s62
 196      BE000145 
 197 01f0 00000000 		lea	%s12,printf@PLT_LO(-24)
 197      00680C06 
 198 01f8 00000000 		and	%s12,%s12,(32)0
 198      608C0C44 
 199 0200 00000000 		sic	%lr
 199      00000A28 
 200 0208 00000000 		lea.sl	%s12,printf@PLT_HI(%s12,%lr)
 200      8A8C8C06 
 201 0210 00000000 		bsic	%lr,(,%s12)		# printf
 201      8C000A08 
 202              	.L_1.8:
 203 0218 00000000 		lea	%s63,.LP.__string.4@GOTOFF_LO
 203      00003F06 
 204 0220 00000000 		and	%s63,%s63,(32)0
 204      60BF3F44 
 205 0228 00000000 		lea.sl	%s63,.LP.__string.4@GOTOFF_HI(%s63,%got)
 205      8FBFBF06 
 206 0230 B0000000 		st	%s63,176(0,%sp)
 206      8B003F11 
 207 0238 00000000 		or	%s0,0,%s63
 207      BF000045 
 208 0240 00000000 		lea	%s12,printf@PLT_LO(-24)
 208      00680C06 
 209 0248 00000000 		and	%s12,%s12,(32)0
 209      608C0C44 
 210 0250 00000000 		sic	%lr
 210      00000A28 
 211 0258 00000000 		lea.sl	%s12,printf@PLT_HI(%s12,%lr)
 211      8A8C8C06 
 212 0260 00000000 		bsic	%lr,(,%s12)		# printf
 212      8C000A08 
 213              	.L_1.7:
 214 0268 00000000 		lea	%s63,.LP.__string.5@GOTOFF_LO
 214      00003F06 
 215 0270 00000000 		and	%s63,%s63,(32)0
 215      60BF3F44 
 216 0278 00000000 		lea.sl	%s63,.LP.__string.5@GOTOFF_HI(%s63,%got)
 216      8FBFBF06 
 217 0280 B0000000 		st	%s63,176(0,%sp)
 217      8B003F11 
 218 0288 00000000 		lea	%s62,.LP.__string.6@GOTOFF_LO
 218      00003E06 
 219 0290 00000000 		and	%s62,%s62,(32)0
 219      60BE3E44 
 220 0298 00000000 		lea.sl	%s62,.LP.__string.6@GOTOFF_HI(%s62,%got)
 220      8FBEBE06 
 221 02a0 B8000000 		st	%s62,184(0,%sp)
 221      8B003E11 
 222 02a8 00000000 		or	%s0,0,%s63
 222      BF000045 
 223 02b0 00000000 		or	%s1,0,%s62
 223      BE000145 
 224 02b8 00000000 		lea	%s12,printf@PLT_LO(-24)
 224      00680C06 
 225 02c0 00000000 		and	%s12,%s12,(32)0
 225      608C0C44 
 226 02c8 00000000 		sic	%lr
 226      00000A28 
 227 02d0 00000000 		lea.sl	%s12,printf@PLT_HI(%s12,%lr)
 227      8A8C8C06 
 228 02d8 00000000 		bsic	%lr,(,%s12)		# printf
 228      8C000A08 
 229              	.L_1.6:
 230 02e0 00000000 		lea	%s63,.LP.__string.7@GOTOFF_LO
 230      00003F06 
 231 02e8 00000000 		and	%s63,%s63,(32)0
 231      60BF3F44 
 232 02f0 00000000 		lea.sl	%s63,.LP.__string.7@GOTOFF_HI(%s63,%got)
 232      8FBFBF06 
 233 02f8 B0000000 		st	%s63,176(0,%sp)
 233      8B003F11 
 234 0300 00000000 		lea	%s62,.LP.__string.8@GOTOFF_LO
 234      00003E06 
 235 0308 00000000 		and	%s62,%s62,(32)0
 235      60BE3E44 
 236 0310 00000000 		lea.sl	%s62,.LP.__string.8@GOTOFF_HI(%s62,%got)
 236      8FBEBE06 
 237 0318 B8000000 		st	%s62,184(0,%sp)
 237      8B003E11 
 238 0320 00000000 		or	%s0,0,%s63
 238      BF000045 
 239 0328 00000000 		or	%s1,0,%s62
 239      BE000145 
 240 0330 00000000 		lea	%s12,printf@PLT_LO(-24)
 240      00680C06 
 241 0338 00000000 		and	%s12,%s12,(32)0
 241      608C0C44 
 242 0340 00000000 		sic	%lr
 242      00000A28 
 243 0348 00000000 		lea.sl	%s12,printf@PLT_HI(%s12,%lr)
 243      8A8C8C06 
 244 0350 00000000 		bsic	%lr,(,%s12)		# printf
 244      8C000A08 
 245              	.L_1.5:
 246 0358 00000000 		lea	%s63,.LP.__string.9@GOTOFF_LO
 246      00003F06 
 247 0360 00000000 		and	%s63,%s63,(32)0
 247      60BF3F44 
 248 0368 00000000 		lea.sl	%s63,.LP.__string.9@GOTOFF_HI(%s63,%got)
 248      8FBFBF06 
 249 0370 B0000000 		st	%s63,176(0,%sp)
 249      8B003F11 
 250 0378 00000000 		lea	%s62,.LP.__string.10@GOTOFF_LO
 250      00003E06 
 251 0380 00000000 		and	%s62,%s62,(32)0
 251      60BE3E44 
 252 0388 00000000 		lea.sl	%s62,.LP.__string.10@GOTOFF_HI(%s62,%got)
 252      8FBEBE06 
 253 0390 B8000000 		st	%s62,184(0,%sp)
 253      8B003E11 
 254 0398 00000000 		or	%s0,0,%s63
 254      BF000045 
 255 03a0 00000000 		or	%s1,0,%s62
 255      BE000145 
 256 03a8 00000000 		lea	%s12,printf@PLT_LO(-24)
 256      00680C06 
 257 03b0 00000000 		and	%s12,%s12,(32)0
 257      608C0C44 
 258 03b8 00000000 		sic	%lr
 258      00000A28 
 259 03c0 00000000 		lea.sl	%s12,printf@PLT_HI(%s12,%lr)
 259      8A8C8C06 
 260 03c8 00000000 		bsic	%lr,(,%s12)		# printf
 260      8C000A08 
 261              	.L_1.4:
 262 03d0 00000000 		lea	%s63,.LP.__string.11@GOTOFF_LO
 262      00003F06 
 263 03d8 00000000 		and	%s63,%s63,(32)0
 263      60BF3F44 
 264 03e0 00000000 		lea.sl	%s63,.LP.__string.11@GOTOFF_HI(%s63,%got)
 264      8FBFBF06 
 265 03e8 B0000000 		st	%s63,176(0,%sp)
 265      8B003F11 
 266 03f0 00000000 		or	%s62,1,(0)1
 266      00013E45 
 267 03f8 B8000000 		st	%s62,184(0,%sp)
 267      8B003E11 
 268 0400 00000000 		or	%s61,2,(0)1
 268      00023D45 
 269 0408 C0000000 		st	%s61,192(0,%sp)
 269      8B003D11 
 270 0410 00000000 		or	%s0,0,%s63
 270      BF000045 
 271 0418 00000000 		or	%s1,0,%s62
 271      BE000145 
 272 0420 00000000 		or	%s2,0,%s61
 272      BD000245 
 273 0428 00000000 		lea	%s12,printf@PLT_LO(-24)
 273      00680C06 
 274 0430 00000000 		and	%s12,%s12,(32)0
 274      608C0C44 
 275 0438 00000000 		sic	%lr
 275      00000A28 
 276 0440 00000000 		lea.sl	%s12,printf@PLT_HI(%s12,%lr)
 276      8A8C8C06 
 277 0448 00000000 		bsic	%lr,(,%s12)		# printf
 277      8C000A08 
 278              	.L_1.3:
 279 0450 00000000 		lea	%s63,.LP.__string.12@GOTOFF_LO
 279      00003F06 
 280 0458 00000000 		and	%s63,%s63,(32)0
 280      60BF3F44 
 281 0460 00000000 		lea.sl	%s63,.LP.__string.12@GOTOFF_HI(%s63,%got)
 281      8FBFBF06 
 282 0468 B0000000 		st	%s63,176(0,%sp)
 282      8B003F11 
 283 0470 00000000 		lea	%s62,.LP.__string.13@GOTOFF_LO
 283      00003E06 
 284 0478 00000000 		and	%s62,%s62,(32)0
 284      60BE3E44 
 285 0480 00000000 		lea.sl	%s62,.LP.__string.13@GOTOFF_HI(%s62,%got)
 285      8FBEBE06 
 286 0488 B8000000 		st	%s62,184(0,%sp)
 286      8B003E11 
 287 0490 00000000 		or	%s0,0,%s63
 287      BF000045 
 288 0498 00000000 		or	%s1,0,%s62
 288      BE000145 
 289 04a0 00000000 		lea	%s12,printf@PLT_LO(-24)
 289      00680C06 
 290 04a8 00000000 		and	%s12,%s12,(32)0
 290      608C0C44 
 291 04b0 00000000 		sic	%lr
 291      00000A28 
 292 04b8 00000000 		lea.sl	%s12,printf@PLT_HI(%s12,%lr)
 292      8A8C8C06 
 293 04c0 00000000 		bsic	%lr,(,%s12)		# printf
 293      8C000A08 
 294              	.L_1.2:
 295 04c8 00000000 		lea	%s63,.LP.__string.14@GOTOFF_LO
 295      00003F06 
 296 04d0 00000000 		and	%s63,%s63,(32)0
 296      60BF3F44 
 297 04d8 00000000 		lea.sl	%s63,.LP.__string.14@GOTOFF_HI(%s63,%got)
 297      8FBFBF06 
 298 04e0 B0000000 		st	%s63,176(0,%sp)
 298      8B003F11 
 299 04e8 00000000 		or	%s62,1,(0)1
 299      00013E45 
 300 04f0 B8000000 		st	%s62,184(0,%sp)
 300      8B003E11 
 301 04f8 00000000 		or	%s61,2,(0)1
 301      00023D45 
 302 0500 C0000000 		st	%s61,192(0,%sp)
 302      8B003D11 
 303 0508 00000000 		or	%s0,0,%s63
 303      BF000045 
 304 0510 00000000 		or	%s1,0,%s62
 304      BE000145 
 305 0518 00000000 		or	%s2,0,%s61
 305      BD000245 
 306 0520 00000000 		lea	%s12,printf@PLT_LO(-24)
 306      00680C06 
 307 0528 00000000 		and	%s12,%s12,(32)0
 307      608C0C44 
 308 0530 00000000 		sic	%lr
 308      00000A28 
 309 0538 00000000 		lea.sl	%s12,printf@PLT_HI(%s12,%lr)
 309      8A8C8C06 
 310 0540 00000000 		bsic	%lr,(,%s12)		# printf
 310      8C000A08 
 311 0548 08000000 		br.l	.L_1.1
 311      00000F18 
 312              	.L_1.1:
 313              	# line 11
  11:/usr/uhome/aurora/4gi/nlabhpg/kruus/vt/src/asm-examples/cpp.c **** 	return 0;
 314              		.loc	1 11 0
 315 0550 00000000 		or	%s63,0,(0)1
 315      00003F45 
 316 0558 00000000 		or	%s0,0,%s63
 316      BF000045 
 317              	# Start of epilogue codes
 318 0560 00000000 		or	%sp,0,%fp
 318      89000B45 
 319              		.cfi_def_cfa	11,8
 320 0568 18000000 		ld	%got,0x18(,%sp)
 320      8B000F01 
 321 0570 20000000 		ld	%plt,0x20(,%sp)
 321      8B001001 
 322 0578 08000000 		ld	%lr,0x8(,%sp)
 322      8B000A01 
 323 0580 00000000 		ld	%fp,0x0(,%sp)
 323      8B000901 
 324 0588 00000000 		b.l	(,%lr)
 324      8A000F19 
 325              	.L_1.EoE:
 326              		.cfi_endproc
 327              		.set	.L.1.2auto_size,	0xfffffffffffffe30	# 464 Bytes
 329              	# ============ End  main ============
 330              	.Le1.0:
