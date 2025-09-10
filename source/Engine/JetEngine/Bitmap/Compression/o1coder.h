/****************************************************************************************/
/*  O1CODER.H                                                                           */
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
#ifndef __COMPUTIL_O1CODER_H
#define __COMPUTIL_O1CODER_H

#include "arithc.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct oOne oOne;

extern oOne * O1coder_Init(arithInfo * ari,long numChars,long numContexts);
extern oOne * O1coder_InitMax(arithInfo * ari,long numChars,long numContexts,long max);
extern void O1coder_EncodeC(oOne * O1I,long Char,long Context);
extern long O1coder_DecodeC(oOne * O1I,long Context);
extern void O1coder_CleanUp(oOne * O1I);
extern void O1coder_Reset(oOne * O1I);

#define oOneCreate(x,y,z) 	O1coder_Init(x,y,z)
#define oOneCreateMax(x,y,z,m) O1coder_InitMax(x,y,z,m)
#define oOneEncode(x,y,z) 	O1coder_EncodeC((oOne *)x,y,z)
#define oOneDecode(x,y)   	O1coder_DecodeC((oOne *)x,y)
#define oOneFree(x)     	O1coder_CleanUp((oOne *)x)

#ifdef __cplusplus
}
#endif

#endif

