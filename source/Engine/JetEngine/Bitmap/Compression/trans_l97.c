/****************************************************************************************/
/*  TRANS_L97.C                                                                         */
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

#include "Utility.h"

#define MULT(x,y)	((((x)*(y))+16383)>>15)

/**********

#define ONE	(1<<15)
#define	C_A	((int)(ONE* 1.586134342  + 0.5))
#define C_B	((int)(ONE* 0.05298011854+ 0.5))
#define C_C ((int)(ONE* 0.8829110762 + 0.5))
#define C_D	((int)(ONE* 0.4435068522 + 0.5))

***********/

#define	C_A	51974
#define C_B	1736
#define C_C	28931
#define C_D	14533

void l97forward(int * to,int *from,int len)
{
int x,*ptr,*low,*high,half;

	half = len>>1;

	assert((len & 1) == 0);
	assert(half >= 2);

	low  = to; high = to + half; ptr = from;

		*high = ptr[1] - MULT(C_A,( ptr[0] + ptr[2] ));
		*low  = ptr[0] - MULT(C_B,( high[0] + high[0]));
		low++; high++; ptr += 2;

	for(x=half-2;x--;) 
	{
		*high = ptr[1] - MULT(C_A,( ptr[0] + ptr[2] ));
		*low  = ptr[0] - MULT(C_B,( high[0] + high[-1]));
		low++; high++; ptr += 2;
	}

		*high = ptr[1] - MULT(C_A,( ptr[0] + ptr[0] ));
		*low  = ptr[0] - MULT(C_B,( high[0] + high[-1]));

	low  = to; high = to + half;

		*high += MULT(C_C,( low[0] + low[1] ));
		*low  += MULT(C_D,( high[0] + high[0]));
		low++; high++;

	for(x=half-2;x--;) 
	{
		*high += MULT(C_C,( low[0] + low[1] ));
		*low  += MULT(C_D,( high[0] + high[-1]));
		low++; high++;
	}

		*high += MULT(C_C,( low[0] + low[0] ));
		*low  += MULT(C_D,( high[0] + high[-1]));

}

void l97inverse(int *to,int *from,int len)
{
int x,*ptr,*low,*high,half;

	half = len>>1;

	low  = from; high = from + half;
	for(x=0;x<half;x++) 
	{
		if ( x == 0 )		*low  -= MULT(C_D,( high[0] + high[0]));
		else				*low  -= MULT(C_D,( high[0] + high[-1]));
		low++; high++;
	}

	low  = from; high = from + half;
	for(x=0;x<half;x++) 
	{
		if ( x == half-1 )	*high -= MULT(C_C,( low[0] + low[0] ));
		else				*high -= MULT(C_C,( low[0] + low[1] ));
		low++; high++;
	}

	low  = from; high = from + half; ptr = to;
	for(x=0;x<half;x++) 
	{
		if ( x == 0 )		ptr[0] = *low  + MULT(C_B,( high[0] + high[0]));
		else				ptr[0] = *low  + MULT(C_B,( high[0] + high[-1]));
		low++; high++; ptr += 2;
	}

	low  = from; high = from + half; ptr = to;
	for(x=0;x<half;x++) 
	{
		if ( x == half-1 )	ptr[1] = *high + MULT(C_A,( ptr[0] + ptr[0] ));
		else				ptr[1] = *high + MULT(C_A,( ptr[0] + ptr[2] ));
		low++; high++; ptr += 2;
	}

}

