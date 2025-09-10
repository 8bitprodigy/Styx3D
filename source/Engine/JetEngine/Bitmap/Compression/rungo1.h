/****************************************************************************************/
/*  RUNGO1.H                                                                            */
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
#ifndef RUNG_O1_H
#define RUNG_O1_H

#include "Utility.h"
#include "arithc.h"

typedef struct rungO1 rungO1;

rungO1 *rungO1Create(arithInfo *ari,int NumContexts);
void	rungO1Destroy(rungO1 * ro1);

void	rungO1Encode(rungO1 * ro1, int context, jeBoolean bit);
jeBoolean	rungO1Decode(rungO1 * ro1, int context);

#endif // RUNG_O1_H

