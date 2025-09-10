/****************************************************************************************/
/*  SOZ.C                                                                               */
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
//#define LOG

#include <stdlib.h>

#include "arithc.h"
#include "soz.h"

#ifdef LOG
#include <stdio.h>
#endif

typedef struct soz
{
	arithInfo * arith;
	int total,size;
	int totmax,inc;
	uint16 * tree;
} soz;

soz * sozCreate(arithInfo * arithinfo,int size,int totmax,int inc);
void sozEncode(soz *sc, int symbol);
int sozDecode(soz *sc);
void sozHalve(soz *sc);
void sozFree(soz *sc);
void sozReset(soz *sc);

soz * sozCreate(arithInfo * arithinfo,int size,int totmax,int inc)
{
soz *sc;

    /* malloc soz structure and array for frequencies */
	if ( ! (sc = (soz *)new(soz)) ) return NULL;
	if ( ! (sc->tree = (uint16 *)newarray(uint16,size)) )
		{ destroy(sc); return(NULL); }

	sc->arith = arithinfo;
	sc->size = size;
	sc->inc = max(inc,1);
	sc->totmax = max(totmax,size+3);

	sozReset(sc);

return sc;
}

#ifdef LOG
static int symtot,symcnt=0;
#endif

void sozReset(soz *sc)
{
int i;
	for(i=0;i<sc->size;i++) sc->tree[i] = 1;
	sc->total = sc->size;

#ifdef LOG
	if ( symcnt > 0 )
		printf("average symbol = %f\n",(double)symtot/symcnt);
	symtot = symcnt = 0;
#endif
}

void sozEncode(soz *sc, int sym) /** returns flag "coded by me or not" **/
{
int low, high, i;

#ifdef LOG
symtot += sym;
symcnt ++;
#endif

low=0;
for(i=0;i<sym;i++) low += sc->tree[i];
	/** 
	*	slow, but fastest for low-valued symbols, which is cool for us.
	****/
high = low + sc->tree[sym];

arithEncode(sc->arith,low, high, sc->total);

sc->tree[sym] += sc->inc;
sc->total += sc->inc;

while (sc->total > sc->totmax )
	sozHalve(sc);
}

int sozDecode(soz *sc)
{
int	sym, target;
int low, high;
    
target = arithGet(sc->arith,sc->total);

sym = low = 0;
while( target >= sc->tree[sym] ) 
{
	low += sc->tree[sym];
	target -= sc->tree[sym];
	sym++;
}
high = low + sc->tree[sym];

arithDecode(sc->arith,low, high, sc->total);

sc->tree[sym] += sc->inc;
sc->total += sc->inc;

while (sc->total > sc->totmax )
	sozHalve(sc);

return sym;
}

void sozHalve(soz *sc)
{
int i;
	sc->total = 0;
	for(i=0;i<sc->size;i++) 
	{
		sc->tree[i] >>= 1;
		if ( sc->tree[i] == 0 ) sc->tree[i] = 1;
		sc->total += sc->tree[i];
	}
}

void sozFree(soz *sc)
{

#ifdef LOG
	if ( symcnt > 0 )
		printf("average symbol = %f\n",(double)symtot/symcnt);
	symtot = symcnt = 0;
#endif

destroy(sc->tree);
destroy(sc);
}

