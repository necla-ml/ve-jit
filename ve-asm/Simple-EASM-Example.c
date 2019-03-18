// vim: et ts=4 sw=4 cindent cino=^=l0,\:0,N-s
/* Copyright (c) 2019 by NEC Corporation
 * This file is part of ve-jit */
void vcopy(double *dst, double *src, int len)
{
	double *p = src, *q = dst;

	for (; len > 0; len -= 256, p += 256, q += 256) {
		if (len < 256)
			__asm__ ("lvl	%0" : : "r" (len) );
		else
			__asm__ ("lea	%%s63,256\n"
					"lvl	%%s63" : : : "%s63" );

		__asm__ ("vld	%%v0,8,%0\n"
				"vst	%%v0,8,%1" : : "r" (p), "r" (q) : "%v0" );
	}
}
