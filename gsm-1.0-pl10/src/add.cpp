/*
 * Copyright 1992 by Jutta Degener and Carsten Bormann, Technische
 * Universitaet Berlin.  See the accompanying file "COPYRIGHT" for
 * details.  THERE IS ABSOLUTELY NO WARRANTY FOR THIS SOFTWARE.
 */

/* $Header: /tmp_amd/presto/export/kbs/jutta/src/gsm/RCS/add.c,v 1.6 1996/07/02 09:57:33 jutta Exp $ */

/*
 *  See private.h for the more commonly used macro versions.
 */

#include	<stdio.h>
#include	<assert.h>

#include	"private.h"
#include	"gsm.h"

s16 gsm_add (s16 a, s16 b)
{
	s32 sum = (s32)a + (s32)b;
	return clamp_s16(sum);
}

s16 gsm_sub (s16 a, s16 b)
{
	s32 diff = (s32)a - (s32)b;
	return clamp_s16(diff);
}

s16 gsm_mult (s16 a, s16 b)
{
	if (a == MIN_WORD && b == MIN_WORD) return MAX_WORD;
	else return s16((s32(a) * s32(b)) / 32768);
}

s16 gsm_mult_r (s16 a, s16 b)
{
	if (b == MIN_WORD && a == MIN_WORD) return MAX_WORD;
	else {
		s32 prod = s32(a) * s32(b) + 16384;
		prod >>= 15;
		return s16(prod & 0xFFFF);
	}
}

s16 gsm_abs (s16 a)
{
	return a < 0 ? (a == MIN_WORD ? MAX_WORD : -a) : a;
}

s32 gsm_L_mult (s16 a, s16 b)
{
	assert( a != MIN_WORD || b != MIN_WORD );
	return ((s32)a * (s32)b) << 1;
}

s32 gsm_L_add (s32 a, s32 b)
{
	if (a < 0) {
		if (b >= 0) return a + b;
		else {
			u32 A = (u32)-(a + 1) + (u32)-(b + 1);
			return A >= MAX_LONGWORD ? MIN_LONGWORD :-(s32)A-2;
		}
	}
	else if (b <= 0) return a + b;
	else {
		u32 A = (u32)a + (u32)b;
		return A > MAX_LONGWORD ? MAX_LONGWORD : A;
	}
}

s32 gsm_L_sub (s32 a, s32 b)
{
	if (a >= 0) {
		if (b >= 0) return a - b;
		else {
			/* a>=0, b<0 */

			u32 A = (u32)a + -(b + 1);
			return A >= MAX_LONGWORD ? MAX_LONGWORD : (A + 1);
		}
	}
	else if (b <= 0) return a - b;
	else {
		/* a<0, b>0 */  

		u32 A = (u32)-(a + 1) + b;
		return A >= MAX_LONGWORD ? MIN_LONGWORD : -(s32)A - 1;
	}
}

static unsigned char const bitoff[ 256 ] = {
	 8, 7, 6, 6, 5, 5, 5, 5, 4, 4, 4, 4, 4, 4, 4, 4,
	 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
	 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

s16 gsm_norm (s32 a)
/*
 * the number of left shifts needed to normalize the 32 bit
 * variable L_var1 for positive values on the interval
 *
 * with minimum of
 * minimum of 1073741824  (01000000000000000000000000000000) and 
 * maximum of 2147483647  (01111111111111111111111111111111)
 *
 *
 * and for negative values on the interval with
 * minimum of -2147483648 (-10000000000000000000000000000000) and
 * maximum of -1073741824 ( -1000000000000000000000000000000).
 *
 * in order to normalize the result, the following
 * operation must be done: L_norm_var1 = L_var1 << norm( L_var1 );
 *
 * (That's 'ffs', only from the left, not the right..)
 */
{
	assert(a != 0);

	if (a < 0) {
		if (a <= -1073741824) return 0;
		a = ~a;
	}

	return    a & 0xffff0000 
		? ( a & 0xff000000
		  ?  -1 + bitoff[ 0xFF & (a >> 24) ]
		  :   7 + bitoff[ 0xFF & (a >> 16) ] )
		: ( a & 0xff00
		  ?  15 + bitoff[ 0xFF & (a >> 8) ]
		  :  23 + bitoff[ 0xFF & a ] );
}

s32 gsm_L_asl (s32 a, int n)
{
	if (n >= 32) return 0;
	if (n <= -32) return -(a < 0);
	if (n < 0) return gsm_L_asr(a, -n);
	return a << n;
}

s16 gsm_asl (s16 a, int n)
{
	if (n >= 16) return 0;
	if (n <= -16) return -(a < 0);
	if (n < 0) return gsm_asr(a, -n);
	return a << n;
}

s32 gsm_L_asr (s32 a, int n)
{
	if (n >= 32) return -(a < 0);
	if (n <= -32) return 0;
	if (n < 0) return a << -n;

#	ifdef	SASR
		return a >> n;
#	else
		if (a >= 0) return a >> n;
		else return -(s32)( -(u32)a >> n );
#	endif
}

s16 gsm_asr (s16 a, int n)
{
	if (n >= 16) return -(a < 0);
	if (n <= -16) return 0;
	if (n < 0) return a << -n;

#	ifdef	SASR
		return a >> n;
#	else
		if (a >= 0) return a >> n;
		else return -(s16)( -(u16)a >> n );
#	endif
}

/* 
 *  (From p. 46, end of section 4.2.5)
 *
 *  NOTE: The following lines gives [sic] one correct implementation
 *  	  of the div(num, denum) arithmetic operation.  Compute div
 *        which is the integer division of num by denum: with denum
 *	  >= num > 0
 */

s16 gsm_div (s16 num, s16 denum)
{
	s32	L_num   = num;
	s32	L_denum = denum;
	s16		div 	= 0;
	int		k 	= 15;

	/* The parameter num sometimes becomes zero.
	 * Although this is explicitly guarded against in 4.2.5,
	 * we assume that the result should then be zero as well.
	 */

	/* assert(num != 0); */

	assert(num >= 0 && denum >= num);
	if (num == 0)
	    return 0;

	while (k--) {
		div   <<= 1;
		L_num <<= 1;

		if (L_num >= L_denum) {
			L_num -= L_denum;
			div++;
		}
	}

	return div;
}
