/****************************************************************************************/
/*  CONTEXT.H                                                                           */
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
#ifndef __COMPUTIL_CONTEXT_H
#define __COMPUTIL_CONTEXT_H

#include "BaseType.h"
#include "arithc.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct context context;

extern context * contextCreate(arithInfo * arithinfo,int alphabet_size);
extern context * contextCreateMax(arithInfo * arithinfo,int length,int totalMax);
extern jeBoolean contextEncode(context *pContext, int symbol); 	/* returns "was char written?" */
extern int  contextDecode(context *pContext); 							/* returns -1 for escape; after escape you MUST call contextAdd! */
extern void contextAdd   (context *pContext, int symbol); 	/* same as Decode_GotC */
extern void contextHalve (context *pContext); 							/* halve counts */
extern void contextFree  (context *pContext); 							/* purge */
extern jeBoolean contextHas   (context *pContext,int symbol);

/******** not for the general public: ********/

extern void contextGetInterval(context *pContext, int *pLow, int *pHigh, int symbol);
extern int contextGetCumProb(context *pContext,int symbol); 
extern int contextGetProb(context *pContext,int symbol); 

/******** same as normal encode/decode , but use cap as NumSymbols;
		i.e. symbol < cap always *******/

extern jeBoolean contextEncodeCapped(context *pContext, int symbol,int cap);
extern int contextDecodeCapped(context *pContext,int cap);

#ifdef __cplusplus
}
#endif

#endif

