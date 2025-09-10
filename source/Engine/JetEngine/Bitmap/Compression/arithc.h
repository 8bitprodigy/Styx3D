/****************************************************************************************/
/*  ARITHC.H                                                                            */
/*                                                                                      */
/*  Author:                                                                             */
/*  Description:                                                                        */
/*                                                                                      */
/*  The contents of this file are subject to the Jet3D Public License                   */
/*  Version 1.02 (the "License"); you may not use this file except in                   */
/*  compliance with the License. You may obtain a copy of the License at                */
/*  http://www.jet3d.com                                                                */
/*                                                                                      */
/*  Software distributed under the License is distributed on an "AS IS"                 */
/*  basis, WITHOUT WARRANTY OF ANY KIND, either express or implied.  See                */
/*  the License for the specific language governing rights and limitations              */
/*  under the License.                                                                  */
/*                                                                                      */
/*  The Original Code is Jet3D, released December 12, 1999.                             */
/*  Copyright (C) 1996-1999 Eclipse Entertainment, L.L.C. All Rights Reserved           */
/*                                                                                      */
/****************************************************************************************/
#ifndef CRB_ARITHC_H
#define CRB_ARITHC_H

#include "Utility.h"

/*
 * Notez:
 *
 *  "totrange" must be <= ari->FastArithCumProbMax at all times !!
 *
 */

typedef struct {
	uint32 code,range,queue;
	uint8 * outPtr;

	uint32 probMax,probMaxSafe;
	uint8 * outBuf;
	uint32 overflow_bytes;	// encoder only
} arithInfo;

#ifdef WIN32
#define ARITHCC	__fastcall
#endif

#ifdef BUILD_BE
#define ARITHCC
#endif

#define safeProbMax probMaxSafe

extern arithInfo * arithInit(void);
extern void arithFree(arithInfo * ari);

extern void arithEncodeInit(arithInfo * ari,uint8 *outBuf);
extern void ARITHCC arithEncode(arithInfo * ari,uint32 low,uint32 high,uint32 totrange);
extern uint32 arithEncodeDone(arithInfo * ari);

extern void arithDecodeInit(arithInfo * ari,uint8 *outBuf);
extern uint32 ARITHCC arithGet(arithInfo * ari,uint32 tot);
extern void ARITHCC arithDecode(arithInfo * ari,uint32 low,uint32 high,uint32 totrange);
extern void arithDecodeDone(arithInfo * ari);

extern void arithEncodeReInit(arithInfo * ari,uint8 *outBuf);	// for resuming after a flush
extern void arithDecodeReInit(arithInfo * ari,uint8 *outBuf);	// for resuming after a flush

extern void		arithEncByteRaw(arithInfo * ari,uint32 byte);
extern uint32   arithDecByteRaw(arithInfo * ari);

// these are deprecated:
extern void arithEncBit(arithInfo * ari,uint32 p0,uint32 pt,jeBoolean bit);
extern jeBoolean arithDecBit(arithInfo * ari,uint32 p0,uint32 pt);
extern void arithModelEncBit(arithInfo * ari,uint32 *p0,uint32 *pt,jeBoolean bit);
extern jeBoolean arithModelDecBit(arithInfo * ari,uint32 *p0,uint32 *pt);

extern void ARITHCC arithEncBitRaw(arithInfo * ari,jeBoolean bit);
extern jeBoolean ARITHCC arithDecBitRaw(arithInfo * ari);

extern uint32 arithTellEncPos(arithInfo * ari);
extern uint32 arithTellDecPos(arithInfo * ari);

extern uint32 arithEncodeFlush(arithInfo * ari);
extern void arithDecodeRestart(arithInfo * ari,uint32 pos);

#endif // arithc
