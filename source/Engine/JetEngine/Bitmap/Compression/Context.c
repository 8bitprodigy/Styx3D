/****************************************************************************************/
/*  CONTEXT.C                                                                           */
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
#include "arithc.h"
#include "Context.h"

#define BACK(i)			((i) & ((i) - 1))	
#define FORW(i)			((i) + ((i) & - (i)))

typedef struct context {
	arithInfo * arith;
	int tree_size;		/* length of tree and current length */
	int escapeP;			/* no. symbols with frequency=1 */
	int total;				/* total of all frequencies in tree */
	int totalMax,escapeMax;
	uint16 * tree;			/* Fenwick's binary index tree */
} context;

context * contextCreate(arithInfo * arithinfo,int length); /** pass alphabet size **/
void contextAdd(context *pContext, int symbol);
jeBoolean contextEncode(context *pContext, int symbol);
int contextDecode(context *pContext);
void contextHalve(context *pContext);
void contextFree(context *pContext);

jeBoolean contextEncodeCapped(context *pContext, int symbol,int cap);
int contextDecodeCapped(context *pContext,int cap);

void contextGetInterval(context *pContext, int *pLow, int *pHigh, int symbol);
int contextGetCumProb(context *pContext,int symbol);
int contextGetProb(context *pContext,int symbol);

context * contextCreate(arithInfo * arithinfo,int length) /** pass alphabet size **/
{
context	*pContext;
int i;
int	size;

    /*
     * increment length to accommodate the fact 
     * that symbol 0 is stored at pos 1 in the array.
     */
length++;

    /* round length up to next power of two */
size = 1;
while (size < length)	size += size;

    /* malloc context structure and array for frequencies */
if ((pContext = (context *) new(context)) == NULL) return(NULL);
if ((pContext->tree = (uint16 *) jeRam_Allocate((size+1)*sizeof(uint16))) == NULL)
	{ destroy(pContext); return(NULL); }

pContext->arith = arithinfo;
pContext->total = 0;		/* total frequency */
pContext->totalMax = arithinfo->safeProbMax;
pContext->escapeMax = max( (length >> 2), 3);
pContext->tree_size = size;	/* no. symbols before growing */
    
    /* initialise contents of tree array to zero */
for (i = 0; i < size; i++) pContext->tree[i] = 0;

pContext->escapeP = 1;

return pContext;	    		/* return a pointer to the context */
}

context * contextCreateMax(arithInfo * arithinfo,int length,int totalMax)
{
context *pContext;
if ( (pContext = contextCreate(arithinfo,length)) == NULL )
	return(NULL);

if ( totalMax < pContext->totalMax )
	pContext->totalMax = totalMax;

return(pContext);
}

/*
 *
 * install a new symbol in a context's frequency table
 * returns 0 if successful, TOO_MANY_SYMBOLS or NO_MEMORY if install fails
 *
 */
void contextAdd(context *pContext, int symbol)
{
int i;

symbol++;	/* increment because first symbol at position one */

i = symbol;	    			/* update elements in tree below */

do
	{
	pContext->tree[i] ++;
	i = FORW(i);
  } while (i < pContext->tree_size);

if ( pContext->escapeP < pContext->escapeMax ) pContext->escapeP ++;
pContext->total ++;

while (pContext->total+pContext->escapeP > pContext->totalMax )
	contextHalve(pContext);


}

jeBoolean contextEncode(context *pContext, int symbol) /** returns flag "coded by me or not" **/
{
int low, high;

contextGetInterval(pContext, &low, &high, symbol+1);
	
if (low == high)
  {
	assert( pContext->escapeP > 0 );

	/* contextEncode the escape symbol if unknown symbol */
	arithEncode(pContext->arith,
		pContext->total,
		pContext->total+pContext->escapeP,
		pContext->total+pContext->escapeP);

	contextAdd(pContext,symbol);

	return JE_FALSE;
  }

arithEncode(pContext->arith,low, high, pContext->total+pContext->escapeP);

if (high-low == 1 && pContext->escapeP > 1) pContext->escapeP --;

symbol++;
while (symbol<pContext->tree_size)
  {
	pContext->tree[symbol] ++;
	symbol = FORW(symbol);
  }
pContext->total ++;

while (pContext->total+pContext->escapeP > pContext->totalMax )
	contextHalve(pContext);

return JE_TRUE;
}


jeBoolean contextEncodeCapped(context *pContext, int symbol,int cap)
{
int low,high,total;

total = contextGetCumProb(pContext,cap+1);

contextGetInterval(pContext, &low, &high, symbol+1);
	
if (low == high)
  {
	assert( pContext->escapeP > 0 );

	/* contextEncode the escape symbol if unknown symbol */
	arithEncode(pContext->arith,
		total,
		total+pContext->escapeP,
		total+pContext->escapeP);

	contextAdd(pContext,symbol);

	return JE_FALSE;
  }

arithEncode(pContext->arith,low, high,total+pContext->escapeP);

if (high-low == 1 && pContext->escapeP > 1) pContext->escapeP --;

symbol++;
while (symbol<pContext->tree_size)
  {
	pContext->tree[symbol] ++;
	symbol = FORW(symbol);
  }
pContext->total ++;

while (pContext->total+pContext->escapeP > pContext->totalMax )
	contextHalve(pContext);

return JE_TRUE;
}


/*
 *
 * contextDecode function is passed a context, and returns a symbol
 * returns -1 when an escape is received
 *
 */
int contextDecode(context *pContext)
{
int	mid, symbol, i, target;
int low, high;
    
target = arithGet(pContext->arith,pContext->total+pContext->escapeP);

    /* check if the escape symbol has been received */
if (target >= pContext->total) {
	arithDecode(pContext->arith,
				pContext->total, 
			  pContext->total+pContext->escapeP,
			  pContext->total+pContext->escapeP);
	return -1;
  }

/***

symbol = 0;
mid = pContext->tree_size >> 1;
while (mid > 0)
  {
	if (pContext->tree[symbol+mid] <= target)
		{
	  symbol = symbol+mid;
	  target -= pContext->tree[symbol];
		}
	mid >>= 1;
  }

***/

symbol = 0;
/* <> target -= tree[0]; */
target ++;
mid = pContext->tree_size >> 1;
while (mid > 0)
  {
	if (target > pContext->tree[symbol+mid])
		{
	  symbol = symbol+mid;
	  target -= pContext->tree[symbol];
		}
	mid >>= 1;
  }
/* <> symbol ++ */
    
 /* 
  * pass in symbol and symbol+1 instead of symbol-1 and symbol to
  * account for array starting at 1 not 0 
  */
i = symbol+1;
contextGetInterval(pContext, &low, &high, i);

arithDecode(pContext->arith,low, high, pContext->total+pContext->escapeP);

if (high-low == 1 && pContext->escapeP > 1) pContext->escapeP --;

/* increment the symbol's frequency count */
pContext->tree[i] ++;
i = FORW(i);
while (i<pContext->tree_size)
  {
	pContext->tree[i] ++;
	i = FORW(i);
  }
pContext->total ++;

while (pContext->total+pContext->escapeP > pContext->totalMax )
	contextHalve(pContext);

return symbol;
}

int contextDecodeCapped(context *pContext,int cap)
{
int	mid, symbol, i, target;
int low, high,total;

total = contextGetCumProb(pContext,cap+1);
    
target = arithGet(pContext->arith,total+pContext->escapeP);

if (target >= total)
  {
	arithDecode(pContext->arith,
				total, 
			  total+pContext->escapeP,
			  total+pContext->escapeP);
	return -1;
  }

symbol = 0;
target ++;
mid = pContext->tree_size >> 1;
while (mid > 0)
  {
	if (target > pContext->tree[symbol+mid])
		{
	  symbol = symbol+mid;
	  target -= pContext->tree[symbol];
		}
	mid >>= 1;
	}

i = symbol+1;
contextGetInterval(pContext, &low, &high, i);

arithDecode(pContext->arith,low, high, total+pContext->escapeP);

if (high-low == 1 && pContext->escapeP > 1) pContext->escapeP --;

/* increment the symbol's frequency count */
pContext->tree[i] ++;
i = FORW(i);
while (i<pContext->tree_size)
  {
	pContext->tree[i] ++;
	i = FORW(i);
  }
pContext->total ++;

while (pContext->total+pContext->escapeP > pContext->totalMax )
	contextHalve(pContext);

return symbol;
}

/*
 *
 * get the low and high limits of the frequency interval
 * occupied by a symbol.
 * this function is faster than calculating the upper bound of the two 
 * symbols individually as it exploits the shared parents of s and s-1.
 *
 */
void contextGetInterval(context *pContext, int *pLow, int *pHigh, int symbol)
{
    int low, high, shared, parent;

    /* calculate first part of high path */
    high = pContext->tree[symbol];
    parent = BACK(symbol);
    
    /* calculate first part of low path */
    symbol--;
    low = 0;
    while (symbol != parent) {
			low += pContext->tree[symbol];
			symbol = BACK(symbol);
    }

    /* sum the shared part of the path back to root */
    shared = 0;
    while (symbol > 0) {
			shared += pContext->tree[symbol];
			symbol = BACK(symbol);
    }

    *pLow		= shared+low;
    *pHigh	= shared+high;
}

int contextGetProb(context *pContext,int symbol)
{
int low,high;
contextGetInterval(pContext,&low,&high,symbol);
return high-low;
} 

jeBoolean contextHas(context *pContext,int symbol)
{
int low,high;
contextGetInterval(pContext,&low,&high,symbol);
return ( low != high );
}

int contextGetCumProb(context *pContext,int symbol)
{
int prob;

symbol--;
prob = 0;
while (symbol > 0)
  {
	prob += pContext->tree[symbol];
	symbol = BACK(symbol);
  }
return(prob);
}
 

/*
 *
 * contextHalve is responsible for halving all the frequency counts in a 
 * context.
 * halves context in linear time by keeping track of the old and new 
 * values of certain parts of the array
 * also recalculates the number of singletons in the new halved context.
 *
 */

void contextHalve(context *pContext)
{
    int	old_values[32], new_values[32];
    int	i, zero_count, temp, sum_old, sum_new;

    pContext->escapeP = 1;
  for (i = 1; i < pContext->tree_size; i++)
    {
	temp = i;

	/* work out position to store element in old and new values arrays */
	for (zero_count = 0; !(temp&1); temp >>= 1)
	    zero_count++;

	/* move through context halving as you go */
	old_values[zero_count] = pContext->tree[i];
	for (temp = zero_count-1, sum_old = 0, sum_new = 0; temp >=0; temp--)
	{
	    sum_old += old_values[temp];
	    sum_new += new_values[temp];
	}
	pContext->tree[i] -= sum_old;
	pContext->total -= (pContext->tree[i]>>1);
	pContext->tree[i] -= (pContext->tree[i]>>1);
	if (pContext->tree[i] == 1) pContext->escapeP ++;
	pContext->tree[i] += sum_new;
	      
	new_values[zero_count] = pContext->tree[i];
    }

if ( pContext->escapeP > pContext->escapeMax ) pContext->escapeP = pContext->escapeMax;
if ( pContext->total+pContext->escapeP > pContext->totalMax ) {
		pContext->escapeP = (pContext->totalMax*2/3) - pContext->total;
		if ( pContext->escapeP < 0 ) pContext->escapeP = 1;

		assert( pContext->total+pContext->escapeP <= pContext->totalMax );
	}

}

void contextFree(context *pContext)
{
if ( ! pContext ) return;
jeRam_Free(pContext->tree);
destroy(pContext);
}

