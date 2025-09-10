/****************************************************************************************/
/*  SCONTEXT.H                                                                          */
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
#ifndef __COMPUTIL_SCONTEXT_H
#define __COMPUTIL_SCONTEXT_H

#include "arithc.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * scontext : for coding small-alphabet contexts
 *
 *  uses a linear array, not a binary tree
 *
 *  takes care to make sure escape is set to zero whenever the
 *   context fills up
 *
 */

typedef struct scontext scontext;

extern scontext * scontextCreate(arithInfo * arithinfo,int size,int escmax,int totmax,int inc,jeBoolean noesc);
extern void scontextAdd(scontext *sc, int symbol);
extern jeBoolean scontextEncode(scontext *sc, int symbol);	/** returns "coded?" **/
extern int scontextDecode(scontext *sc);				/** returns -1 for escape **/
extern void scontextHalve(scontext *sc);
extern void scontextFree(scontext *sc);
extern jeBoolean scontextHas(scontext *sc,int symbol);
extern int scontextGetProb(scontext *sc,int symbol);

#ifdef __cplusplus
}
#endif

#endif

