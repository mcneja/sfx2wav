/*
 * Copyright 1992 by Jutta Degener and Carsten Bormann, Technische
 * Universitaet Berlin.  See the accompanying file "COPYRIGHT" for
 * details.  THERE IS ABSOLUTELY NO	WARRANTY FOR THIS SOFTWARE.
 */

/*$Header: /home/kbs/jutta/src/gsm/gsm-1.0/inc/RCS/gsm.h,v 1.11 1996/07/05 18:02:56 jutta Exp $*/

#ifndef GSM_H
#define GSM_H

#include <stdio.h> /* for FILE * */

/*
 *  Interface
 */

typedef struct gsm_state *      gsm;
typedef short                   gsm_signal;             // signed 16 bit
typedef unsigned char           gsm_byte;
typedef gsm_byte                gsm_frame[33];          // 33 * 8 bits

#define GSM_MAGIC               0xD                     // 13 kbit/s RPE-LTP

#define GSM_PATCHLEVEL          10
#define GSM_MINOR               0
#define GSM_MAJOR               1

enum GSM_OPT
{
	GSM_OPT_VERBOSE = 1,
	GSM_OPT_FAST,
	GSM_OPT_LTP_CUT,
	GSM_OPT_WAV49,
	GSM_OPT_FRAME_INDEX,
	GSM_OPT_FRAME_CHAIN
};

gsm  gsm_create  ();
void gsm_destroy (gsm);

int  gsm_print   (FILE *, gsm, gsm_byte *);
int  gsm_option  (gsm, int, int *);

void gsm_encode  (gsm, const gsm_signal *, gsm_byte   *);
int  gsm_decode  (gsm, const gsm_byte   *, gsm_signal *);

int  gsm_explode (gsm, const gsm_byte   *, gsm_signal *);
void gsm_implode (gsm, const gsm_signal *, gsm_byte   *);

#endif /* GSM_H */
