/*
 * Copyright 1992 by Jutta Degener and Carsten Bormann, Technische
 * Universitaet Berlin.  See the accompanying file "COPYRIGHT" for
 * details.  THERE IS ABSOLUTELY NO WARRANTY FOR THIS SOFTWARE.
 */

/* $Header: /tmp_amd/presto/export/kbs/jutta/src/gsm/RCS/code.c,v 1.3 1996/07/02 09:59:05 jutta Exp $ */

#include <stdlib.h>
#include <string.h>
#include "private.h"
#include "gsm.h"

/* 
 *  4.2 FIXED POINT IMPLEMENTATION OF THE RPE-LTP CODER 
 */

void Gsm_Coder
(
	gsm_state * S,

	const s16 * s,	/* [0..159] samples		  	IN	*/

/*
 * The RPE-LTD coder works on a frame by frame basis.  The length of
 * the frame is equal to 160 samples.  Some computations are done
 * once per frame to produce at the output of the coder the
 * LARc[1..8] parameters which are the coded LAR coefficients and 
 * also to realize the inverse filtering operation for the entire
 * frame (160 samples of signal d[0..159]).  These parts produce at
 * the output of the coder:
 */

	s16	* LARc,	/* [0..7] LAR coefficients		OUT	*/

/*
 * Procedure 4.2.11 to 4.2.18 are to be executed four times per
 * frame.  That means once for each sub-segment RPE-LTP analysis of
 * 40 samples.  These parts produce at the output of the coder:
 */

	s16	* Nc,	/* [0..3] LTP lag			OUT 	*/
	s16	* bc,	/* [0..3] coded LTP gain		OUT 	*/
	s16	* Mc,	/* [0..3] RPE grid selection		OUT     */
	s16	* xmaxc,/* [0..3] Coded maximum amplitude	OUT	*/
	s16	* xMc	/* [13*4] normalized RPE samples	OUT	*/
)
{
	int	k;
	s16	* dp  = S->dp0 + 120;	/* [ -120...-1 ] */
	s16	* dpp = dp;		/* [ 0...39 ]	 */

	static s16 e[50];

	s16	so[160];

	Gsm_Preprocess			(S, s, so);
	Gsm_LPC_Analysis		(S, so, LARc);
	Gsm_Short_Term_Analysis_Filter	(S, LARc, so);

	for (k = 0; k < 4; ++k, xMc += 13)
	{
		Gsm_Long_Term_Predictor	( S,
					so+k*40, /* d      [0..39] IN	*/
					dp,	  /* dp  [-120..-1] IN	*/
					e + 5,	  /* e      [0..39] OUT	*/
					dpp,	  /* dpp    [0..39] OUT */
					Nc++,
					bc++);

		Gsm_RPE_Encoding ( S,
					e + 5,	/* e	  ][0..39][ IN/OUT */
					  *xmaxc, Mc++, xMc );
		++xmaxc;
		/*
		 * Gsm_Update_of_reconstructed_short_time_residual_signal
		 *			( dpp, e + 5, dp );
		 */

		{ int i;
		  for (i = 0; i <= 39; i++)
			dp[ i ] = GSM_ADD( e[5 + i], dpp[i] );
		}
		dp  += 40;
		dpp += 40;

	}
	(void)memcpy( (char *)S->dp0, (char *)(S->dp0 + 160),
		120 * sizeof(*S->dp0) );
}
