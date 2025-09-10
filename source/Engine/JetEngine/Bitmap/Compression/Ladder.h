/****************************************************************************************/
/*  LADDER.H                                                                            */
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
#ifndef LADDER_H
#define LADDER_H

#include "rungae.h"

struct srung
{
	rung_t r0,r1;
	uint32 p0;
	uint32 pad;		// make it 16 bytes
};

// RUNG_SHIFTS <= CumProbMaxBits
#define RUNG_SHIFTS			(10)
#define RUNG_ONE			(1<<RUNG_SHIFTS)
#define RUNG_HALF			(RUNG_ONE>>1)

extern const srung ladder[];

extern srung const * const pladder;

extern int ladder_start;

#endif // LADDER_H

