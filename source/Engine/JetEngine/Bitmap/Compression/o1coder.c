/****************************************************************************************/
/*  O1CODER.C                                                                           */
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

#include <stdlib.h>

#include "Utility.h"
#include "arithc.h"
#include "o0coder.h"
#include "o1coder.h"
#include "Context.h"

struct oOne
{
	arithInfo * arith;
	long numChars,numContexts,totMax;
	oZero * order0;
	context ** order1;
};

/*protos:*/
void O1coder_CleanUp(oOne * O1I);

/*functions:*/

oOne * O1coder_InitMax(arithInfo * ari,long numChars,long numContexts,long totMax)
{
oOne * Ret;

if ( (Ret = (oOne *)new(oOne)) == NULL )
  return(NULL);

Ret->numChars = numChars;
Ret->numContexts = numContexts;
Ret->arith = ari;
Ret->totMax = totMax;

if ( (Ret->order0 = oZeroCreateMax(ari,numChars,totMax)) == NULL )
	{ O1coder_CleanUp(Ret); return(NULL); }

if ( (Ret->order1 = (context **)newarray(void *,numContexts)) == NULL )
	{ O1coder_CleanUp(Ret); return(NULL); }

return(Ret);
}

oOne * O1coder_Init (arithInfo * ari,long numChars,long numContexts)
{
return O1coder_InitMax(ari,numChars,numContexts,ari->safeProbMax);
}

void O1coder_CleanUp(oOne * O1I)
{
if ( O1I->order1 )
	{
	long i;
	for(i=0;i<O1I->numContexts;i++) if ( O1I->order1[i] ) contextFree(O1I->order1[i]);
	destroy(O1I->order1);
	}
if ( O1I->order0 ) oZeroFree(O1I->order0);
destroy(O1I);
}

void O1coder_EncodeC(oOne * O1I,long sym,long context)
{

assert( context < O1I->numContexts );

if ( O1I->order1[context] == NULL )
	if ( (O1I->order1[context] = contextCreateMax(O1I->arith,O1I->numChars,O1I->totMax)) == NULL )
		return;

if ( ! contextEncode(O1I->order1[context],sym) )
	oZeroEncode(O1I->order0,sym);

return;
}

long O1coder_DecodeC(oOne * O1I,long context)
{
long sym;

assert( context < O1I->numContexts );

if ( ! O1I->order1[context] )
	if ( (O1I->order1[context] = contextCreateMax(O1I->arith,O1I->numChars,O1I->totMax)) == NULL ) return(-1);

if ( ( sym = contextDecode(O1I->order1[context]) ) == -1 )
	{
	sym = oZeroDecode(O1I->order0);
	contextAdd(O1I->order1[context],sym);
	}

return(sym);
}

