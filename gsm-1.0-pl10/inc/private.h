/*
 * Copyright 1992 by Jutta Degener and Carsten Bormann, Technische
 * Universitaet Berlin.  See the accompanying file "COPYRIGHT" for
 * details.  THERE IS ABSOLUTELY NO WARRANTY FOR THIS SOFTWARE.
 */

/*$Header: /tmp_amd/presto/export/kbs/jutta/src/gsm/RCS/private.h,v 1.6 1996/07/02 10:15:26 jutta Exp $*/

#ifndef PRIVATE_H
#define PRIVATE_H

#include <limits>

typedef short               s16;        // 16 bit signed int
typedef long int            s32;        // 32 bit signed int

typedef unsigned short      u16;        // 16 bit unsigned int
typedef unsigned long       u32;  // 32 bit unsigned int

struct gsm_state
{
    s16             dp0[ 280 ];

    s16             z1;             // preprocessing.c, Offset_com.
    s32             L_z2;           //                  Offset_com.
    int             mp;             //                  Preemphasis

    s16             u[8];           // short_term_aly_filter.c
    s16             LARpp[2][8];
    s16             j;

    s16             ltp_cut;        // long_term.c, LTP crosscorr.
    s16             nrp; /* 40 */   // long_term.c, synthesis
    s16             v[9];           // short_term.c, synthesis
    s32             msr;            // decoder.c,   Postprocessing

    char            verbose;        // only used if !NDEBUG
    char            fast;           // only used if FAST

    char            wav_fmt;        // only used if WAV49 defined
    unsigned char   frame_index;    //            odd/even chaining
    unsigned char   frame_chain;    //   half-byte to carry forward
};


#define MIN_WORD        (-32767 - 1)
#define MAX_WORD          32767

#define MIN_LONGWORD    (-2147483647 - 1)
#define MAX_LONGWORD      2147483647

#ifdef  SASR            // flag: >> is a signed arithmetic shift right
#undef  SASR
#define SASR(x, by)     ((x) >> (by))
#else
#define SASR(x, by)     ((x) >= 0 ? (x) >> (by) : (~(-((x) + 1) >> (by))))
#endif  // SASR

// Prototypes from add.c
s16 gsm_mult   (s16 a, s16 b);
s32 gsm_L_mult (s16 a, s16 b);
s16 gsm_mult_r (s16 a, s16 b);

s16 gsm_div    (s16 num, s16 denum);

s16 gsm_add    (s16 a, s16 b);
s32 gsm_L_add  (s32 a, s32 b);

s16 gsm_sub    (s16 a, s16 b);
s32 gsm_L_sub  (s32 a, s32 b);

s16 gsm_abs    (s16 a);

s16 gsm_norm   (s32 a);

s32 gsm_L_asl  (s32 a, int n);
s16 gsm_asl    (s16 a, int n);

s32 gsm_L_asr  (s32 a, int n);
s16 gsm_asr    (s16 a, int n);

// Inlined functions from add.h 

inline s16 GSM_MULT_R(s16 a, s16 b) // !(a == b == std::numeric_limits<s16>::min())
{
	return s16((s32(a) * s32(b) + 16384) / 32768);
}

inline s16 GSM_MULT(s16 a, s16 b) // !(a == b == std::numeric_limits<s16>::min())
{
	return s16((s32(a) * s32(b)) / 32768);
}

inline s32 GSM_L_MULT(s16 a, s16 b)
{
	return (s32(a) * s32(b)) << 1;
}

inline s32 GSM_L_ADD(s32 a, s32 b)
{
	s32 result;
	if (a < 0)
	{
		if (b >= 0)
			result = a + b;
		else
		{
			u32 utmp = u32(-(a + 1)) + u32(-(b + 1));
			if (utmp >= u32(std::numeric_limits<s32>::max()))
				result = std::numeric_limits<s32>::min();
			else
				result = -(s32)utmp-2;
		}
	}
	else
	{
		if (b <= 0)
			result = a + b;
		else
		{
			u32 utmp = u32(a) + u32(b);
			if (utmp >= u32(std::numeric_limits<s32>::max()))
				result = std::numeric_limits<s32>::max();
			else
				result = utmp;
		}
	}
	return result;
}

inline s16 clamp_s16(s32 a)
{
	if (a > std::numeric_limits<s16>::max())
		return std::numeric_limits<s16>::max();
	else if (a < std::numeric_limits<s16>::min())
		return std::numeric_limits<s16>::min();
	else
		return s16(a);
}

inline s16 GSM_ADD(s16 a, s16 b)
{
	s32 sum = s32(a) + s32(b);
	return clamp_s16(sum);
}

inline s16 GSM_SUB(s16 a, s16 b)
{
	s32 diff = s32(a) - s32(b);
	return clamp_s16(diff);
}

inline s16 GSM_ABS(s16 a)
{
	if (a >= 0)
		return a;
	else if (a == std::numeric_limits<s16>::min())
		return std::numeric_limits<s16>::max();
	else
		return -a;
}

/* Use these if necessary:

#define GSM_MULT_R(a, b)       gsm_mult_r(a, b)
#define GSM_MULT(a, b)         gsm_mult(a, b)
#define GSM_L_MULT(a, b)       gsm_L_mult(a, b)

#define GSM_L_ADD(a, b)        gsm_L_add(a, b)
#define GSM_ADD(a, b)          gsm_add(a, b)
#define GSM_SUB(a, b)          gsm_sub(a, b)

#define GSM_ABS(a)             gsm_abs(a)

*/

// More prototypes from implementations..
void Gsm_Coder
(
	gsm_state * S,
	const s16 * s,    // [0..159] samples               IN
	s16 *       LARc, // [0..7] LAR coefficients        OUT
	s16 *       Nc,   // [0..3] LTP lag                 OUT
	s16 *       bc,   // [0..3] coded LTP gain          OUT
	s16 *       Mc,   // [0..3] RPE grid selection      OUT
	s16 *       xmaxc,// [0..3] Coded maximum amplitude OUT
	s16 *       xMc   // [13*4] normalized RPE samples  OUT
);

void Gsm_Long_Term_Predictor // 4x for 160 samples
(
	gsm_state * S,
	const s16 * d,    // [0..39]   residual signal    IN
	const s16 * dp,   // [-120..-1] d'                IN
	s16 *       e,    // [0..40]                      OUT
	s16 *       dpp,  // [0..40]                      OUT
	s16 *       Nc,   // correlation lag              OUT
	s16 *       bc    // gain factor                  OUT
);

void Gsm_LPC_Analysis
(
	gsm_state * S,
	s16 * s,       // 0..159 signals      IN/OUT
	s16 * LARc     // 0..7   LARc's       OUT
);

void Gsm_Preprocess
(
	gsm_state * S,
	const s16 * s,
	s16 * so
);

void Gsm_Encoding
(
	gsm_state * S,
	s16 * e,    
	s16 * ep,   
	s16 * xmaxc,
	s16 * Mc,   
	s16 * xMc
);

void Gsm_Short_Term_Analysis_Filter
(
	gsm_state * S,
	const s16 * LARc, // coded log area ratio [0..7]  IN
	s16 *       d     // st res. signal [0..159]      IN/OUT
);

void Gsm_Decoder
(
	gsm_state * S,
	const s16 LARcr[8],
	const s16 Ncr[4],
	const s16 bcr[4],
	const s16 Mcr[4],
	const s16 xmaxcr[4],
	const s16 xMcr[52],
	s16 *     s // [0..159] OUT
);

void Gsm_Long_Term_Synthesis_Filtering
(
	gsm_state * S,
	s16         Ncr,
	s16         bcr,
	const s16 * erp,    // [0..39]                IN
	s16 *       drp     // [-120..-1] IN, [0..40] OUT
);

void Gsm_RPE_Decoding
(
	gsm_state * S,
	s16         xmaxcr,
	s16         Mcr,
	const s16 * xMcr,   // [0..12], 3 bits        IN
	s16 *       erp     // [0..39]                OUT
);

void Gsm_RPE_Encoding
(
	gsm_state * S,
	s16 *       e,      // -5..-1][0..39][40..44     IN/OUT
	s16 &       xmaxc,  //                              OUT
	s16 *       Mc,     //                              OUT
	s16 *       xMc     // [0..12]                      OUT
);

void Gsm_Short_Term_Synthesis_Filter
(
	gsm_state * S,
	const s16 * LARcr,  // log area ratios [0..7]  IN
	const s16 * drp,    // received d [0...39]     IN
	s16 *       s       // signal   s [0..159]    OUT
);

void Gsm_Update_of_reconstructed_short_time_residual_signal
(
	const s16 * dpp,    // [0...39]     IN
	const s16 * ep,     // [0...39]     IN
	s16 *       dp      // [-120...-1]  IN/OUT
);

// Tables from table.c
extern const s16 gsm_A[8], gsm_B[8], gsm_MIC[8], gsm_MAC[8];
extern const s16 gsm_INVA[8];
extern const s16 gsm_DLB[4], gsm_QLB[4];
extern const s16 gsm_H[11];
extern const s16 gsm_NRFAC[8];
extern const s16 gsm_FAC[8];

// Debugging
#ifdef NDEBUG

#define  gsm_debug_words(a, b, c, d)             // nil
#define  gsm_debug_longwords(a, b, c, d)         // nil
#define  gsm_debug_word(a, b)                    // nil
#define  gsm_debug_longword(a, b)                // nil

#else   // !NDEBUG => DEBUG

void gsm_debug_words     (char * name, int, int, s16 *);
void gsm_debug_longwords (char * name, int, int, s32 *);
void gsm_debug_longword  (char * name, s32);
void gsm_debug_word      (char * name, s16);

#endif // !NDEBUG

#endif  // PRIVATE_H
