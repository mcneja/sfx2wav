/*
 * Copyright 1992 by Jutta Degener and Carsten Bormann, Technische
 * Universitaet Berlin.  See the accompanying file "COPYRIGHT" for
 * details.  THERE IS ABSOLUTELY NO WARRANTY FOR THIS SOFTWARE.
 */

/* $Header: /tmp_amd/presto/export/kbs/jutta/src/gsm/RCS/decode.c,v 1.1 1992/10/28 00:15:50 jutta Exp $ */

#include "private.h"

/*
 *  4.3 FIXED POINT IMPLEMENTATION OF THE RPE-LTP DECODER
 */

static void Postprocessing(gsm_state * S, s16 * s)
{
	for (int k = 0; k < 160; ++k)
	{
		// s_out[k] = 2*s[k] + s_out[k-1]*7045/8192:
		S->msr *= 7045;
		S->msr += 16384*s32(s[k]);
		S->msr += 4096;
		S->msr /= 8192;
		s[k] = clamp_s16(S->msr);
	}
}

void Gsm_Decoder
(
	gsm_state * S,

	const s16   LARcr[8],
	const s16   Ncr[4],
	const s16   bcr[4],
	const s16   Mcr[4],
	const s16   xmaxcr[4],
	const s16   xMcr[52],

	s16 *       s	// [0..159] OUT
)
{
	s16 erp[40], wt[160];
	s16 * drp = S->dp0 + 120;

	for (int j = 0; j < 4; ++j)
	{
		Gsm_RPE_Decoding(S, xmaxcr[j], Mcr[j], xMcr + 13*j, erp);
		Gsm_Long_Term_Synthesis_Filtering(S, Ncr[j], bcr[j], erp, drp);

		for (int k = 0; k < 40; ++k)
			wt[ j*40 + k ] =  drp[ k ];
	}

	Gsm_Short_Term_Synthesis_Filter( S, LARcr, wt, s );
	Postprocessing(S, s);
}
