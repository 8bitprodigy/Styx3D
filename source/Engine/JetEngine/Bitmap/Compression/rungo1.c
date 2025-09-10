/****************************************************************************************/
/*  RUNGO1.C                                                                            */
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
#include "rungo1.h"
#include "rungae.h"

struct rungO1
{
	int numContexts;
	arithInfo *ari;
	rung_t * rungs;
};

rungO1	*	 rungO1Create(arithInfo *ari,int nc)
{
rungO1 * ro1;
int i;
	
	assert(ari && nc > 0 );

	ro1 = (rungO1 *)new(rungO1);
	if ( ! ro1 ) return NULL;

	ro1->numContexts = nc;
	ro1->ari = ari;
	ro1->rungs = (rung_t *)newarray(rung_t,nc);
	if ( ! ro1->rungs ) {
		destroy(ro1);
		return NULL;
	}

	for(i=0;i<nc;i++)
	{
		rungModelInit(&(ro1->rungs[i]));
	}

return ro1;
}

void		rungO1Destroy(rungO1 * ro1)
{
	assert(ro1 && ro1->rungs);
	destroy(ro1->rungs);
	destroy(ro1);
}

void	rungO1Encode(rungO1 * ro1, int context, jeBoolean bit)
{
	assert(ro1);
	assert( context >= 0 && context < ro1->numContexts );
	rungModelEncBit(ro1->ari,bit,&(ro1->rungs[context]));
}

jeBoolean	rungO1Decode(rungO1 * ro1, int context)
{
	assert(ro1);
	assert( context >= 0 && context < ro1->numContexts );
return rungModelDecBit(ro1->ari,&(ro1->rungs[context]));
}
