/*
 * Copyright 1992 by Jutta Degener and Carsten Bormann, Technische
 * Universitaet Berlin.  See the accompanying file "COPYRIGHT" for
 * details.  THERE IS ABSOLUTELY NO WARRANTY FOR THIS SOFTWARE.
 */

/* $Header: /tmp_amd/presto/export/kbs/jutta/src/gsm/RCS/lpc.c,v 1.5 1994/12/30 23:14:54 jutta Exp $ */

#include <stdio.h>
#include <assert.h>

#include "private.h"
#include "gsm.h"

/*
 *  4.2.4 .. 4.2.7 LPC ANALYSIS SECTION
 */

/* 4.2.4 */


static void Autocorrelation (
	s16     * s,		/* [0..159]	IN/OUT  */
 	s32 * L_ACF)	/* [0..8]	OUT     */
/*
 *  The goal is to compute the array L_ACF[k].  The signal s[i] must
 *  be scaled in order to avoid an overflow situation.
 */
{
	int	k, i;

	s16		temp, smax, scalauto;

#ifdef	USE_FLOAT_MUL
	float		float_s[160];
#endif

	/*  Dynamic scaling of the array  s[0..159]
	 */

	/*  Search for the maximum.
	 */
	smax = 0;
	for (k = 0; k <= 159; k++) {
		temp = GSM_ABS( s[k] );
		if (temp > smax) smax = temp;
	}

	/*  Computation of the scaling factor.
	 */
	if (smax == 0) scalauto = 0;
	else {
		assert(smax > 0);
		scalauto = 4 - gsm_norm( (s32)smax << 16 );/* sub(4,..) */
	}

	/*  Scaling of the array s[0...159]
	 */

	if (scalauto > 0) {

# ifdef USE_FLOAT_MUL
#   define SCALE(n)	\
	case n: for (k = 0; k <= 159; k++) \
			float_s[k] = (float)	\
				(s[k] = GSM_MULT_R(s[k], 16384 >> (n-1)));\
		break;
# else 
#   define SCALE(n)	\
	case n: for (k = 0; k <= 159; k++) \
			s[k] = GSM_MULT_R( s[k], 16384 >> (n-1) );\
		break;
# endif /* USE_FLOAT_MUL */

		switch (scalauto) {
		SCALE(1)
		SCALE(2)
		SCALE(3)
		SCALE(4)
		}
# undef	SCALE
	}
# ifdef	USE_FLOAT_MUL
	else for (k = 0; k <= 159; k++) float_s[k] = (float) s[k];
# endif

	/*  Compute the L_ACF[..].
	 */
	{
# ifdef	USE_FLOAT_MUL
		float * sp = float_s;
		float   sl = *sp;

#		define STEP(k)	 L_ACF[k] += (s32)(sl * sp[ -(k) ]);
# else
		s16  * sp = s;
		s16    sl = *sp;

#		define STEP(k)	 L_ACF[k] += ((s32)sl * sp[ -(k) ]);
# endif

#	define NEXTI	 sl = *++sp


	for (k = 9; k--; L_ACF[k] = 0) ;

	STEP (0);
	NEXTI;
	STEP(0); STEP(1);
	NEXTI;
	STEP(0); STEP(1); STEP(2);
	NEXTI;
	STEP(0); STEP(1); STEP(2); STEP(3);
	NEXTI;
	STEP(0); STEP(1); STEP(2); STEP(3); STEP(4);
	NEXTI;
	STEP(0); STEP(1); STEP(2); STEP(3); STEP(4); STEP(5);
	NEXTI;
	STEP(0); STEP(1); STEP(2); STEP(3); STEP(4); STEP(5); STEP(6);
	NEXTI;
	STEP(0); STEP(1); STEP(2); STEP(3); STEP(4); STEP(5); STEP(6); STEP(7);

	for (i = 8; i <= 159; i++) {

		NEXTI;

		STEP(0);
		STEP(1); STEP(2); STEP(3); STEP(4);
		STEP(5); STEP(6); STEP(7); STEP(8);
	}

	for (k = 9; k--; L_ACF[k] <<= 1) ; 

	}
	/*   Rescaling of the array s[0..159]
	 */
	if (scalauto > 0) {
		assert(scalauto <= 4); 
		for (k = 160; k--; *s++ <<= scalauto) ;
	}
}

#if defined(USE_FLOAT_MUL) && defined(FAST)

static void Fast_Autocorrelation (
	s16 * s,		/* [0..159]	IN/OUT  */
 	s32 * L_ACF)	/* [0..8]	OUT     */
{
	int	k, i;
	float f_L_ACF[9];
	float scale;

	float          s_f[160];
	float *sf = s_f;

	for (i = 0; i < 160; ++i) sf[i] = s[i];
	for (k = 0; k <= 8; k++) {
		float L_temp2 = 0;
		float *sfl = sf - k;
		for (i = k; i < 160; ++i) L_temp2 += sf[i] * sfl[i];
		f_L_ACF[k] = L_temp2;
	}
	scale = MAX_LONGWORD / f_L_ACF[0];

	for (k = 0; k <= 8; k++) {
		L_ACF[k] = f_L_ACF[k] * scale;
	}
}
#endif	/* defined (USE_FLOAT_MUL) && defined (FAST) */

/* 4.2.5 */

static void Reflection_coefficients
(
	const s32 * L_ACF,	// 0...8	IN
	s16 *       r		// 0...7	OUT
)
{
	int i, m, n;
	s16 temp;
	s16 ACF[9]; // 0..8
	s16 P  [9]; // 0..8
	s16 K  [9]; // 2..8

	//  Schur recursion with 16 bits arithmetic.

	if (L_ACF[0] == 0)
	{
		for (i = 8; i--; *r++ = 0) ;
		return;
	}

	assert( L_ACF[0] != 0 );
	temp = gsm_norm( L_ACF[0] );

	assert(temp >= 0 && temp < 32);

	// ? overflow ?
	for (i = 0; i <= 8; i++)
		ACF[i] = s16((L_ACF[i] << temp) / 65536);

	//   Initialize array P[..] and K[..] for the recursion.

	for (i = 1; i <= 7; i++) K[ i ] = ACF[ i ];
	for (i = 0; i <= 8; i++) P[ i ] = ACF[ i ];

	//   Compute reflection coefficients
	for (n = 1; n <= 8; n++, r++) {

		temp = P[1];
		temp = GSM_ABS(temp);
		if (P[0] < temp) {
			for (i = n; i <= 8; i++) *r++ = 0;
			return;
		}

		*r = gsm_div( temp, P[0] );

		assert(*r >= 0);
		if (P[1] > 0) *r = -*r;		// r[n] = sub(0, r[n])
		assert (*r != MIN_WORD);
		if (n == 8) return; 

		//  Schur recursion
		temp = GSM_MULT_R( P[1], *r );
		P[0] = GSM_ADD( P[0], temp );

		for (m = 1; m <= 8 - n; m++) {
			temp     = GSM_MULT_R( K[ m   ],    *r );
			P[m]     = GSM_ADD(    P[ m+1 ],  temp );

			temp     = GSM_MULT_R( P[ m+1 ],    *r );
			K[m]     = GSM_ADD(    K[ m   ],  temp );
		}
	}
}

/* 4.2.6 */

static void Transformation_to_Log_Area_Ratios (
	s16	* r 			/* 0..7	   IN/OUT */
)
/*
 *  The following scaling for r[..] and LAR[..] has been used:
 *
 *  r[..]   = integer( real_r[..]*32768. ); -1 <= real_r < 1.
 *  LAR[..] = integer( real_LAR[..] * 16384 );
 *  with -1.625 <= real_LAR <= 1.625
 */
{
	s16	temp;
	int	i;


	/* Computation of the LAR[0..7] from the r[0..7]
	 */
	for (i = 1; i <= 8; i++, r++) {

		temp = *r;
		temp = GSM_ABS(temp);
		assert(temp >= 0);

		if (temp < 22118) {
			temp >>= 1;
		} else if (temp < 31130) {
			assert( temp >= 11059 );
			temp -= 11059;
		} else {
			assert( temp >= 26112 );
			temp -= 26112;
			temp <<= 2;
		}

		*r = *r < 0 ? -temp : temp;
		assert( *r != MIN_WORD );
	}
}

/* 4.2.7 */

static void Quantization_and_coding (
	s16 * LAR    	/* [0..7]	IN/OUT	*/
)
{
	s16	temp;

	/*  This procedure needs four tables; the following equations
	 *  give the optimum scaling for the constants:
	 *  
	 *  A[0..7] = integer( real_A[0..7] * 1024 )
	 *  B[0..7] = integer( real_B[0..7] *  512 )
	 *  MAC[0..7] = maximum of the LARc[0..7]
	 *  MIC[0..7] = minimum of the LARc[0..7]
	 */

#	undef STEP
#	define	STEP( A, B, MAC, MIC )		\
		temp = GSM_MULT( A,   *LAR );	\
		temp = GSM_ADD(  temp,   B );	\
		temp = GSM_ADD(  temp, 256 );	\
		temp /= 512;	\
		*LAR  =  temp>MAC ? MAC - MIC : (temp<MIC ? 0 : temp - MIC); \
		LAR++;

	STEP(  20480,     0,  31, -32 );
	STEP(  20480,     0,  31, -32 );
	STEP(  20480,  2048,  15, -16 );
	STEP(  20480, -2560,  15, -16 );

	STEP(  13964,    94,   7,  -8 );
	STEP(  15360, -1792,   7,  -8 );
	STEP(   8534,  -341,   3,  -4 );
	STEP(   9036, -1144,   3,  -4 );

#	undef	STEP
}

void Gsm_LPC_Analysis (
	gsm_state *S,
	s16 		 * s,		/* 0..159 signals	IN/OUT	*/
        s16 		 * LARc)	/* 0..7   LARc's	OUT	*/
{
	s32	L_ACF[9];

#if defined(USE_FLOAT_MUL) && defined(FAST)
	if (S->fast) Fast_Autocorrelation (s,	  L_ACF );
	else
#endif
	Autocorrelation			  (s,	  L_ACF	);
	Reflection_coefficients		  (L_ACF, LARc	);
	Transformation_to_Log_Area_Ratios (LARc);
	Quantization_and_coding		  (LARc);
}
