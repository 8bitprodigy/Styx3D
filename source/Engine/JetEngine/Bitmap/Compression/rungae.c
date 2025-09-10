/****************************************************************************************/
/*  RUNGAE.C                                                                            */
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
/*}{**********************************************************************************/

#include "Utility.h"
#include "arithc.h"
#include "arithc._h"
#include "rungae.h"

#ifndef RUNGFUNC_PREFIX
#define RUNGFUNC_PREFIX
#endif

/*}{**********************************************************************************/

#include "Ladder.h"

RUNGFUNC_PREFIX void ARITHCC rungModelInit(rung_t * rung)
{
	*rung = ladder_start;
}

RUNGFUNC_PREFIX void ARITHCC rungModelEncBit(arithInfo * ari,jeBoolean bit,rung_t * rung)
{
uint32 code,range;

code    = ari->code;
range   = ari->range;

	assert(range > 0);
	assert(range <= One );

	{
	uint32 r;
	const srung * s;
	
		s = &ladder[*rung];
		assert( s->p0 > 0 && s->p0 < RUNG_ONE );

		r = (range >> RUNG_SHIFTS) * (s->p0);

		if ( bit ) {
		
#ifdef LOG
			fprintf(encLog,"%d, 1, %d\n",*rung,s->r1);
#endif

			code += r;
			range -= r;
			*rung = s->r1;
			assert( ladder[*rung].p0 <= s->p0 );
		} else {
#ifdef LOG
			fprintf(encLog,"%d, 0, %d\n",*rung,s->r0);
#endif

			range = r;
			*rung = s->r0;
			assert( ladder[*rung].p0 >= s->p0 );
		}
	}

	assert(range > 0);
	assert(range <= One );

/** the same flush as normal Encode **/

	while( range <= MinRange ) {
		uint32 byte;
		byte = code >> SHIFT_BITS;
	
		if ( byte == 0xFF ) {
			ari->overflow_bytes++;
		} else {
			uint32 carry;
			carry = code>>CODE_BITS;	
			*(ari->outPtr)++ = (uint8)(ari->queue + carry);
			ari->queue = byte;

			if ( ari->overflow_bytes ) {
				*(ari->outPtr)++ = (uint8)(0xFF + carry);
				while ( --(ari->overflow_bytes) ) *(ari->outPtr)++ = (uint8)(0xFF + carry);
			}
		}

		code = (code<<8) & CODE_MASK;
		range <<= 8;
	}

	assert(range > 0);
	assert(range <= One );

ari->code  = code;
ari->range = range;
}

/*}{**********************************************************************************/

RUNGFUNC_PREFIX jeBoolean ARITHCC rungModelDecBit(arithInfo * ari,rung_t * rung)
{
jeBoolean bit;
uint32 range,code;

	range = ari->range;
	code = ari->code;

	assert(range > 0);
	assert(range <= One );

	while ( range <= MinRange ) {
		range <<= 8;
		code = (code<<8) + (((ari->queue)<<EXTRA_BITS)&0xFF);	// use the top bit in the queue
		ari->queue = *(ari->outPtr)++;
		code += (ari->queue) >> (TAIL_EXTRA_BITS);
	}

	assert(range > 0);
	assert(range <= One );

	{
	uint32 r;
	const srung * s;
	
		s = &ladder[*rung];
		assert( s->p0 > 0 && s->p0 < RUNG_ONE );

		r = (range >> RUNG_SHIFTS) * (s->p0);

		if ( code >= r ) {
		
#ifdef LOG
			fprintf(decLog,"%d, 1, %d\n",*rung,s->r1);
#endif

			bit = 1;
			code -= r;
			range -= r;
			*rung = s->r1;
			assert( ladder[*rung].p0 <= s->p0 );
		} else {
#ifdef LOG
			fprintf(decLog,"%d, 0, %d\n",*rung,s->r0);
#endif

			bit = 0;
			range = r;
			*rung = s->r0;
			assert( ladder[*rung].p0 >= s->p0 );
		}
	}

	assert(range > 0);
	assert(range <= One );

ari->range = range;
ari->code  = code;

return bit;
}

/*}{**********************************************************************************/
