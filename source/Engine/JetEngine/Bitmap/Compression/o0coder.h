/****************************************************************************************/
/*  O0CODER.H                                                                           */
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
#ifndef JE_COMPRESSION_O0CODER_H
#define JE_COMPRESSION_O0CODER_H

#include "arithc.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ozero ozero;

extern ozero * O0coder_Init(arithInfo *ari,long NumChars);
extern ozero * O0coder_InitMax(arithInfo *ari,long NumChars,long TotProbMax);
extern void O0coder_EncodeC(ozero * O0I,long Char);
extern long O0coder_DecodeC(ozero * O0I);
extern void O0coder_CleanUp(ozero * O0I);
extern void O0coder_AddC(ozero * O0I,long Char);

#define ozeroCreate(x,y) (ozero *)O0coder_Init(x,y)
#define ozeroCreateMax(x,y,z) (ozero *)O0coder_InitMax(x,y,z)
#define ozeroEncode(x,y) O0coder_EncodeC((ozero *)x,y)
#define ozeroAdd(x,y) 	 O0coder_AddC((ozero *)x,y)
#define ozeroDecode(x)   O0coder_DecodeC((ozero *)x)
#define ozeroFree(x)     O0coder_CleanUp((ozero *)x)

#define oZero 			ozero
#define oZeroCreate 	ozeroCreate
#define oZeroCreateMax	ozeroCreateMax
#define oZeroEncode 	ozeroEncode
#define oZeroAdd    	ozeroAdd
#define oZeroDecode 	ozeroDecode
#define oZeroFree		ozeroFree

#ifdef __cplusplus
}
#endif

#endif

