/****************************************************************************************/
/*  CODER.H                                                                             */
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
#ifndef __CODER_H
#define __CODER_H

#include "arithc.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct coder coder;

typedef struct coderParams 
{
	coder *coderPtr; // Modified to CJP to be a little less confusing than coder * coder..
	int *band,*parent;
	int w,h,fullW;
	int bandN;
	int nextmask,bitshift;
} coderParams;

typedef jeBoolean (*coderInit) (coder *);
typedef void (*coderFree) (coder *);
typedef void (*coderFlush) (coder *);
typedef void (*coderEncodeBandBP) (coderParams *p);
typedef void (*coderDecodeBandBP) (coderParams *p);

struct coder 
{
	const char * name;
	coderInit init;
	coderFree free;
	coderEncodeBandBP encodeBandBP;	
	coderDecodeBandBP decodeBandBP;
	coderFlush flush;	/** optional ; set to null if you need no flushing **/

	void * data;	/** setup by init, killed by free **/
	arithInfo * arith;	/** controlled by the global coder routines **/
	uint32 wStopLen;
};

/** coder settings are passed via stopLen and coderN in the jeWavelet structures **/

extern coder * coderCreateWrite(int coderN,uint8 * compArray);
extern coder * coderCreateRead( int coderN,uint8 * compArray,int stopLen);
extern coder * coderCreateReadNoInit(int coderN,int stopLen);
extern int	coderFlushWrite(coder *);
extern void coderFlushRead(coder *);
extern void coderDestroy(coder *);

#define coderGetPos(c)			arithTellEncPos(c->arith)
#define coderGetPosd(c)		arithTellDecPos(c->arith)
#define coderStopD(c)	( coderGetPosd(c) >= (c)->wStopLen )

extern void coderEncodeDPCM(coder *c,int *plane,int width,int height,int rowpad);
extern void coderDecodeDPCM(coder *c,int *plane,int width,int height,int rowpad);

extern void coderCodeDPCM1d(coder *c,int *plane,int width,jeBoolean Decode);

#define CODE_MAX_BPN			(24)
#define CODE_MAX_VAL			(1<<CODE_MAX_BPN)

// a common coding question:
#define isneg(x)	((x)<0)
#define signbit(x)	(isneg(x)?1:0)

// here's an adaptive binary arithmetic coder for your viewing pleasure:

#define __BITMODEL_INC 20
#define __BITMODEL_TOT 4000
#define bitModelInit(P0,PT)		do { (P0) = 1; (PT) = 2; } while(0);
#define bitModel(bit,P0,PT)		do { (PT) += __BITMODEL_INC; if (!(bit)) (P0) += __BITMODEL_INC;  if ( (PT) > (__BITMODEL_TOT) ) { \
										 (PT) >>= 1; (P0) >>= 1; P0++; PT += 2; } } while(0)
#define bitEnc(bit,ari,P0,PT)	do { arithEncBit(ari,P0,PT,bit);	bitModel(bit,P0,PT); } while(0)
#define bitDec(bit,ari,P0,PT)	do { bit = arithDecBit(ari,P0,PT);	bitModel(bit,P0,PT); } while(0)

/*** tune me ! **/

extern int tune_param;

/** the master list of coders: **/

extern const int num_coders;
extern const coder * coder_list[];

#ifdef __cplusplus
}
#endif


#endif //CODER_H

