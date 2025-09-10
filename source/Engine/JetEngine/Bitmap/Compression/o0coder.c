/****************************************************************************************/
/*  O0CODER.C                                                                           */
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
#include "Context.h"

struct ozero 
{
	arithInfo * arith;
	long numChars;
	context * order0;
};

/*protos:*/
void O0coder_CleanUp(ozero * O0I);

/*functions:*/

ozero * O0coder_Init (arithInfo * ari,long numChars)
{
ozero * Ret;

if ( (Ret = (ozero*)new(ozero)) == NULL )
  return(NULL);

if ( (Ret->order0 = contextCreate(ari,numChars)) == NULL )
	{ O0coder_CleanUp(Ret); return(NULL); }

Ret->numChars = numChars;
Ret->arith = ari;

return(Ret);
}

ozero * O0coder_InitMax(arithInfo * ari,long NumChars,long totMax)
{
ozero * Ret;

if ( (Ret = (ozero*)new(ozero)) == NULL )
  return(NULL);

if ( (Ret->order0 = contextCreateMax(ari,NumChars,totMax)) == NULL )
	{ O0coder_CleanUp(Ret); return(NULL); }

Ret->numChars = NumChars;
Ret->arith = ari;

return(Ret);
}

void O0coder_EncodeC(ozero * O0I,long sym)
{

if ( ! contextEncode(O0I->order0,sym) ) {
		/* use order -1 <> todo! could be better with exclusions! */
	arithEncode(O0I->arith,sym,sym+1,O0I->numChars);
}

return;
}

void O0coder_AddC(ozero * O0I,long sym)
{
contextAdd(O0I->order0,sym);
return;
}

long O0coder_DecodeC(ozero * O0I)
{
long sym;

if ( ( sym = contextDecode(O0I->order0) ) == -1 ) {
	/* use order -1 */
	sym = arithGet(O0I->arith,O0I->numChars);
	arithDecode(O0I->arith,sym,sym+1,O0I->numChars);

	contextAdd(O0I->order0,sym);
}

return(sym);
}

void O0coder_CleanUp(ozero * O0I)
{
if ( O0I->order0 ) contextFree(O0I->order0);
destroy(O0I);
}
