/****************************************************************************************/
/*  SCONTEXT.C                                                                          */
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

//#define DEBUG

#ifdef DEBUG
#include <stdio.h>
#endif

#include "arithc.h"

typedef struct scontext
{
	arithInfo * arith;
	int escape,total,size;
	int escmax,totmax,inc;
	int nzero;
	jeBoolean noesc;
	uint16 * tree;
} scontext;

scontext * scontextCreate(arithInfo * arithinfo,int size,int escmax,int totmax,int inc,jeBoolean noesc);
void scontextAdd(scontext *sc, int symbol);
jeBoolean scontextEncode(scontext *sc, int symbol);
int scontextDecode(scontext *sc);
void scontextHalve(scontext *sc);
void scontextFree(scontext *sc);
jeBoolean scontextHas(scontext *sc,int symbol);
int scontextGetProb(scontext *sc,int symbol);

scontext * scontextCreate(arithInfo * arithinfo,int size,int escmax,int totmax,int inc,jeBoolean noesc)
{
scontext *sc;
int i;

    /* malloc scontext structure and array for frequencies */
	if ( ! (sc = (scontext *)new(scontext)) ) return NULL;
	if ( ! (sc->tree = (uint16 *)newarray(uint16,size)) )
		{ destroy(sc); return(NULL); }

	sc->arith = arithinfo;
	sc->total = 0;
	sc->escape = 1;
	sc->size = size;
	sc->nzero = size;
	sc->inc = max(inc,1);
	sc->totmax = max(totmax,size+3);
	sc->escmax = min(max(escmax,3),(size>>2)+2);
	sc->noesc = noesc;
	if ( noesc ) 
	{
		for(i=0;i<size;i++) sc->tree[i] = 1;
		sc->nzero = sc->escape = 0;
		sc->total = size;
	}

return sc;
}

void scontextAdd(scontext *sc, int sym)
{

if ( sc->tree[sym] == 0 ) 
{
	if ( sc->escape < sc->escmax ) 
	{
		sc->escape ++;
	}
	if ( (--sc->nzero) == 0 ) 
	{
		sc->escape = 0;
	}
}
else if ( sc->tree[sym] == 1 ) 
{
	if ( sc->escape > 1 ) sc->escape --;
}

sc->tree[sym] += sc->inc;
sc->total += sc->inc;

while (sc->total > sc->totmax )
	scontextHalve(sc);

}

jeBoolean scontextEncode(scontext *sc, int sym) /** returns flag "coded by me or not" **/
{
	
if ( sc->tree[sym] == 0 ) 
{
#ifdef DEBUG
	if ( sc->escape == 0 || sc->nzero < 1 ) 
	{
		BrandoError("stats: cannot code zero-probability novel symbol");
		fprintf(stderr,"sym: %d , esc: %d, nzero: %d\n",sym,sc->escape,sc->nzero);
		exit(1);
	}
#endif

	arithEncode(sc->arith,
		sc->total,
		sc->total+sc->escape,
		sc->total+sc->escape);

	scontextAdd(sc,sym);

	return JE_FALSE;
}
else 
{
	int low, high, i;

	low=0;
	for(i=0;i<sym;i++) low += sc->tree[i];
	high = low + sc->tree[sym];

	arithEncode(sc->arith,low, high, sc->total+sc->escape);

	scontextAdd(sc,sym);
}

return JE_TRUE;
}


/* returns -1 when an escape is received
 */
int scontextDecode(scontext *sc)
{
int	sym, target;
int low, high;
    
target = arithGet(sc->arith,sc->total+sc->escape);

    /* check if the escape symbol has been received */
if (target >= sc->total) 
{
	arithDecode(sc->arith,
				sc->total, 
				sc->total+sc->escape,
				sc->total+sc->escape);
	return -1;
}

sym = 0;
low = 0;
while( target >= sc->tree[sym] ) 
{
	low += sc->tree[sym];
	target -= sc->tree[sym];
	sym++;
}
high = low + sc->tree[sym];

arithDecode(sc->arith,low, high, sc->total+sc->escape);

scontextAdd(sc,sym);

return sym;
}

jeBoolean scontextHas(scontext *sc,int symbol)
{
return( scontextGetProb(sc,symbol) );
}

int scontextGetProb(scontext *sc,int symbol)
{
return( sc->tree[symbol] );
}
 
void scontextHalve(scontext *sc)
{
int i;
sc->nzero = 0;
sc->escape = 1;
sc->total = 0;
for(i=0;i<sc->size;i++) 
{
	sc->tree[i] >>= 1;
	if ( sc->tree[i] == 0 ) 
	{
		if ( sc->noesc ) sc->tree[i] = 1;
		else sc->nzero ++;
	}
	else if ( sc->tree[i] == 1 )
		sc->escape ++;

	sc->total += sc->tree[i];
}

if ( sc->nzero == 0 ) sc->escape = 0;
if ( sc->escape > sc->escmax ) sc->escape = sc->escmax;

}

void scontextFree(scontext *sc)
{
destroy(sc->tree);
destroy(sc);
}

