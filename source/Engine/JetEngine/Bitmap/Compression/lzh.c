/****************************************************************************************/
/*  LZH.C                                                                               */
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

/*{*/

#define DO_NONGREEDY_LOOKAHEAD
#define AMORTIZED_HASHING
#define UNROLLED_MATCH_LOOP

#ifndef _LOG
#define _LOG
#endif

#include "Utility.h"
#include "IntMath.h"
#include "huffa.h"
#include "Log.h"
#include <string.h>

	// could be adaptive (!)
#define bitsPerMatch 	(16)
#define bitsPerLiteral	(7)

//consts:

#define minMatchLen		(4)		// the min-match-len is stuck at 4 because we use "uint32" peeks 
								//	to speed up matching.  We'd be better off at 3.

#define matchLenEscape 	(250)		// not very sensitive

#define MINIMUM_RAW_LEN	(10)

#define OFFSET_BITS		(8)			// 12 bits is 16 Megs of memory

//structs:
struct lookupNode {
	struct lookupNode * next;
	struct lookupNode * prev;
	uint32 firstFour;
	uint8 *ptrPlusFour;
};

#define HASHSIZE	(1<<16)
#define HASHFUNC(x) ( ( x ^ ( x >> 15 ) ) & 0x0000FFFF )

static int nonGreedy_ratioS10 = (((bitsPerMatch+bitsPerLiteral)<<10)/bitsPerMatch);

#ifdef AMORTIZED_HASHING
#define nodesCheckedMax 100		// = 100 hurts about 0.01 bpp from infinity <> !
#endif

#define lookupHunkSize 	((1<<(OFFSET_BITS+8))-2)

#ifdef _DEBUG
#define inline
#else
#ifdef _MSC_VER
#define inline __inline
#else
#define inline
#endif
#endif

// encoder-only
static long rawOutLen;
static long lookupHunkNext;
static jeBoolean lookupFreeFlag;

static struct lookupNode ** lookupTable;
static struct lookupNode * lookupHunk;
static uint8 * rawLZArray;
static uint8 *Lits,*Lens,*Offs,*Mats;
int numLits,numLens,numOffs,numMats;

#define CleanUp(str) assert((str) == NULL)

//protos:
static void Encode(uint8 *rawArray,int rawLen);
static void Decode(uint8 *rawArray,int rawLen);

static void encodeMatchLen(int gotMatchLen);
static int decodeMatchLen(void);

static void codeMatchFlagInit(void);
void encodeMatchFlagFlush(void);
static void encodeMatchFlag(jeBoolean bit);
static jeBoolean decodeMatchFlag(void);

static void encodeOffset(int offset);
static int  decodeOffset(void);

static void addLookupNode(uint8 *rawPtr);
static int tellMatchLen(uint8 *MatchVsPtr1,uint8 *MatchVsPtr2);
static void findMatch(uint8 *rawPtr,int * pgotMatchLen,uint8 **pgotMatchPtr);

static void lzhInit(int rawLen);
static void lzhFree(void);

/*}{**************/

void lzhEncode(uint8 *rawArray,int rawLen,uint8 **compArrayPtr,int * compLenPtr)
{
uint8 *huffPtr;
uint8 *compArray;
uint32 huffLen;
jeBoolean success;

rawOutLen = lookupHunkNext = 0;
lookupFreeFlag = JE_FALSE;

lookupTable = NULL;
lookupHunk = NULL;
rawLZArray = NULL;

Lits = Lens = Offs = Mats = NULL;
numLits = numLens = numOffs = numMats = 0;

assert( (((uint32)rawArray)&3) == 0 );

if ( (compArray = (uint8*)jeRam_Allocate(rawLen + 16384)) == NULL )
	CleanUp("AllocMem failed!");

*compArrayPtr = compArray;

if ( rawLen < MINIMUM_RAW_LEN )
{
	((uint32 *)compArray)[0] = rawLen;
	compArray += 4;
	memcpy(compArray,rawArray,rawLen);
	*compLenPtr = rawLen + 4;
	return;
}

if ( (lookupTable = (struct lookupNode **)newarray(void *,HASHSIZE)) == NULL )
	CleanUp("AllocMem failed!");

if ( (lookupHunk = (struct lookupNode *)jeRam_Allocate(sizeof(struct lookupNode)*min(lookupHunkSize,rawLen+10))) == NULL )
	CleanUp("AllocMem failed!");

if ( (rawLZArray = (uint8*)jeRam_Allocate(rawLen)) == NULL )
	CleanUp("AllocMem failed!");

lzhInit(rawLen);

Encode(rawArray,rawLen);

encodeMatchFlagFlush();

((uint32 *)compArray)[0] = rawLen;
((uint32 *)compArray)[1] = numLits;
((uint32 *)compArray)[2] = numLens;
((uint32 *)compArray)[3] = numOffs;
((uint32 *)compArray)[4] = numMats;

huffPtr = compArray + 20;

success = HuffArray(Lits,numLits,huffPtr,&huffLen,HUFFA_TYPE_O0NB); huffPtr += huffLen;
assert(success);
	Log_Printf("Lits : %d -> %d\n",numLits,huffLen);
success = HuffArray(Lens,numLens,huffPtr,&huffLen,HUFFA_TYPE_O0NB); huffPtr += huffLen;
assert(success);
	Log_Printf("Lens : %d -> %d\n",numLens,huffLen);
success = HuffArray(Offs,numOffs,huffPtr,&huffLen,HUFFA_TYPE_O0NB); huffPtr += huffLen;
assert(success);
	Log_Printf("Offs : %d -> %d\n",numOffs,huffLen);
success = HuffArray(Mats,numMats,huffPtr,&huffLen,HUFFA_TYPE_O0NB); huffPtr += huffLen;
assert(success);
	Log_Printf("Mats : %d -> %d\n",numMats,huffLen);

memcpy(huffPtr,rawLZArray,rawOutLen); huffPtr += rawOutLen;

*compLenPtr = (int)(huffPtr - compArray);

lzhFree();

jeRam_Free(lookupTable);
jeRam_Free(lookupHunk);
jeRam_Free(rawLZArray);

}

void lzhDecode(uint8 *compArray,int compLen,uint8 ** rawArrayPtr,int * rawLenPtr)
{
uint32 huffLen,rawLen;
uint8 * rawArray,*huffPtr;
jeBoolean success;

assert( (((uint32)compArray)&3) == 0 );

huffPtr = compArray;

rawLen = *((uint32 *)huffPtr); huffPtr += 4;

*rawLenPtr = rawLen;

if ( (rawArray = (uint8*)jeRam_Allocate(rawLen+1024)) == NULL )
	CleanUp("AllocMem failed!");
*rawArrayPtr = rawArray;

if ( rawLen < MINIMUM_RAW_LEN )
{
	memcpy(rawArray,huffPtr,rawLen);
	return;
}

lzhInit(rawLen);

numLits = *((uint32 *)huffPtr); huffPtr += 4;
numLens = *((uint32 *)huffPtr); huffPtr += 4;
numOffs = *((uint32 *)huffPtr); huffPtr += 4;
numMats = *((uint32 *)huffPtr); huffPtr += 4;

success = HuffArray(Lits,numLits,huffPtr,&huffLen,HUFFA_TYPE_DEC); huffPtr += huffLen;
assert(success);
	Log_Printf("D : Lits : %d -> %d\n",numLits,huffLen);
success = HuffArray(Lens,numLens,huffPtr,&huffLen,HUFFA_TYPE_DEC); huffPtr += huffLen;
assert(success);
	Log_Printf("D : Lens : %d -> %d\n",numLens,huffLen);
success = HuffArray(Offs,numOffs,huffPtr,&huffLen,HUFFA_TYPE_DEC); huffPtr += huffLen;
assert(success);
	Log_Printf("D : Offs : %d -> %d\n",numOffs,huffLen);
success = HuffArray(Mats,numMats,huffPtr,&huffLen,HUFFA_TYPE_DEC); huffPtr += huffLen;
assert(success);
	Log_Printf("D : Mats : %d -> %d\n",numMats,huffLen);

rawLZArray = huffPtr;
numLits = numLens = numOffs = numMats = 0;

Decode(rawArray,rawLen);

lzhFree();

}

/*}{**************/

static void lzhInit(int rawLen)
{

codeMatchFlagInit();

if ( (Lits = (uint8 *)malloc(rawLen)) == NULL )
	CleanUp("malloc failed!");

if ( (Lens = (uint8 *)malloc(rawLen)) == NULL )
	CleanUp("malloc failed!");

if ( (Offs = (uint8 *)malloc(rawLen)) == NULL )
	CleanUp("malloc failed!");

if ( (Mats = (uint8 *)malloc(rawLen/8)) == NULL )
	CleanUp("malloc failed!");

}

static void lzhFree(void)
{

free(Lits);
free(Lens);
free(Offs);
free(Mats);

}

/*}{**************/

static void Encode(uint8 *rawArray,int rawLen)
{
int gotMatchLen,offset;
uint8 *rawPtr,*rawArrayPtrDone,*gotMatchPtr;
#ifdef DO_NONGREEDY_LOOKAHEAD
uint8 * gotMatchPtrNext;
int gotMatchLenNext;
#endif

	rawPtr = rawArray;
	rawArrayPtrDone = rawArray + rawLen;

	while ( rawPtr < rawArrayPtrDone )
	{
		findMatch(rawPtr,&gotMatchLen,&gotMatchPtr);

		if ( gotMatchLen >= minMatchLen )
		{
	#ifdef DO_NONGREEDY_LOOKAHEAD
			findMatch(rawPtr+1,&gotMatchLenNext,&gotMatchPtrNext);

			if ( gotMatchLenNext >= ((nonGreedy_ratioS10 * gotMatchLen + 512)>>10) - 1 )
			{
				/* add literal */

				encodeMatchFlag(0);
				addLookupNode(rawPtr);
				Lits[numLits++] = *rawPtr++;

				/* set match to peeked match */
				gotMatchLen = gotMatchLenNext;
				gotMatchPtr = gotMatchPtrNext;
			}
	#endif /* DO_NONGREEDY_LOOKAHEAD */

			encodeMatchFlag(1);
			encodeMatchLen(gotMatchLen);

			offset = rawPtr - gotMatchPtr ;
			assert( offset <= lookupHunkSize );
			encodeOffset(offset >> 8);
			rawLZArray[rawOutLen++] = offset & 0xFF;

			/* add lookup nodes */
			while(gotMatchLen--)
				addLookupNode(rawPtr++);

		}
		else
		{
			encodeMatchFlag(0);
			addLookupNode(rawPtr);
			Lits[numLits++] = *rawPtr++;
		}
	}
}

/*}{**************/

static void Decode(uint8 *rawArray,int rawLen)
{
uint8 *rawPtr,*rawPtrDone;

	rawPtr = rawArray;
	rawPtrDone = rawArray + rawLen;

	while ( rawPtr < rawPtrDone )
	{
		if ( ! decodeMatchFlag() )
		{
			*rawPtr++ = Lits[numLits++];
		}
		else
		{
			int CurOff,gotMatchLen;
			uint8 *refPtr;

			gotMatchLen = decodeMatchLen();

			CurOff = decodeOffset();
			CurOff <<= 8;
			CurOff += (*rawLZArray++);

			refPtr = rawPtr - CurOff;

			assert( refPtr >= rawArray );

			while(gotMatchLen--)
				*rawPtr++ = *refPtr++;
		}
	}
}

/*}{**************/

static int buf,bcount;

static void codeMatchFlagInit(void)
{
	buf = 0;
	bcount = 8;
}

void encodeMatchFlagFlush(void)
{
	if ( bcount == 8 )
		return;

	buf <<= bcount;
	assert(buf < 256);
	Mats[numMats++] = buf;
}

static void inline encodeMatchFlag(jeBoolean bit)
{
	buf += buf + bit;
	if ( --bcount == 0 )
	{
		assert(buf < 256);
		Mats[numMats++] = buf;
		buf = 0;
		bcount = 8;
	}
}

static jeBoolean inline decodeMatchFlag(void)
{
	if ( bcount == 8 )
	{
		buf = Mats[numMats++];
		bcount = 0;
	}
	bcount++;
	buf <<= 1;
return ( buf & 0x100 );
}

/*}{**************/

static void inline encodeOffset(int offset)
{
	Offs[numOffs++] = offset;
}

static int inline decodeOffset(void)
{
	return Offs[numOffs++];
}

/*}{**************/

static void inline encodeMatchLen(int gotMatchLen)
{
int matchLenOut;
	matchLenOut = gotMatchLen - minMatchLen;
	if ( matchLenOut >= matchLenEscape )
	{
		Lens[numLens++] = matchLenEscape;

		matchLenOut -= matchLenEscape;

		while ( matchLenOut >= 0xFF )
		{
			rawLZArray[rawOutLen++] = 0xFF;
			matchLenOut -= 0xFF;
		}
		rawLZArray[rawOutLen++] = matchLenOut;
	}
	else
	{
		Lens[numLens++] = matchLenOut;
	}
}

static int inline decodeMatchLen(void)
{
int gotMatchLen;

	gotMatchLen = Lens[numLens++];

	if ( gotMatchLen == matchLenEscape )
	{
		int matchLenTemp;
		do {
			matchLenTemp = *rawLZArray++;
			gotMatchLen += matchLenTemp;
		} while ( matchLenTemp == 0xFF );
	}
	gotMatchLen += minMatchLen;
return gotMatchLen;
}

/*}{**************/

static void inline addLookupNode(uint8 *rawPtr)
{
struct lookupNode *node;
uint32 curFirstFour,hash;

	node = & lookupHunk[lookupHunkNext++];
	if ( lookupFreeFlag )	
	{
		// lookup is full, must free as we go
		if ( node->prev == NULL )
		{
			curFirstFour = node->firstFour;
			hash = HASHFUNC(curFirstFour);
			lookupTable[hash] = node->next;
		}
		else
		{
			node->prev->next = node->next;
		}
		if ( node->next ) node->next->prev = node->prev;
	}
	if ( lookupHunkNext == lookupHunkSize ) 
	{
		lookupHunkNext = 0;
		lookupFreeFlag = JE_TRUE;
	}
	node->ptrPlusFour = rawPtr + 4;
	node->firstFour = curFirstFour = getuint32(rawPtr);
	hash = HASHFUNC(curFirstFour);
	node->prev = NULL;
	node->next = lookupTable[hash];
	lookupTable[hash] = node;
	if ( node->next ) node->next->prev = node;
}

/*}{**************/

static int inline tellMatchLen(uint8 *MatchVsPtr1,uint8 *MatchVsPtr2)
{
int matchLen;

#ifdef UNROLLED_MATCH_LOOP
			uint8 * Match2Base = MatchVsPtr2;

#define MATCHER (*MatchVsPtr1++ != *MatchVsPtr2++)||

			MATCHER MATCHER MATCHER MATCHER MATCHER MATCHER MATCHER MATCHER MATCHER
			MATCHER MATCHER MATCHER MATCHER MATCHER MATCHER MATCHER MATCHER MATCHER
			MATCHER MATCHER MATCHER MATCHER MATCHER MATCHER MATCHER MATCHER MATCHER
			MATCHER MATCHER MATCHER MATCHER MATCHER MATCHER MATCHER MATCHER MATCHER
			MATCHER MATCHER MATCHER MATCHER MATCHER MatchVsPtr2++;

			matchLen = MatchVsPtr2 - Match2Base + 3;

			if ( matchLen == 45 )
			{
				MatchVsPtr2--;
				while ( *MatchVsPtr1++ == *MatchVsPtr2++ )
					matchLen++;
			}

#else /* no UNROLLED_MATCH_LOOP */

			matchLen = 4;
			while ( *MatchVsPtr1++ == *MatchVsPtr2++ )
				matchLen++;

#endif /* match loop */

return matchLen;
}

static void findMatch(uint8 *rawPtr,int * pgotMatchLen,uint8 **pgotMatchPtr)
{
uint8 *gotMatchPtr;
long gotMatchLen,matchLen;
uint32 curFirstFour;
struct lookupNode *node;
long hash;
uint8 *rawPtrPlusFour;
#ifdef AMORTIZED_HASHING
long nodesChecked;
#endif


	curFirstFour = getuint32(rawPtr);
	hash = HASHFUNC(curFirstFour);
	node = lookupTable[hash];

#ifdef AMORTIZED_HASHING
	nodesChecked = 0;
#endif

	gotMatchPtr = NULL;
	gotMatchLen = 0;
	rawPtrPlusFour = rawPtr + 4;
	while(node)
	{
		if ( node->firstFour == curFirstFour )
		{
			matchLen = tellMatchLen(node->ptrPlusFour,rawPtrPlusFour);

			if ( matchLen > gotMatchLen )
			{
				gotMatchLen = matchLen;
				gotMatchPtr = node->ptrPlusFour;
			}

#ifdef AMORTIZED_HASHING
			nodesChecked++;
			if ( nodesChecked == nodesCheckedMax ) break;
#endif
		}
		node = node->next;
	}

	if ( gotMatchPtr ) gotMatchPtr -= 4;
	*pgotMatchLen = gotMatchLen;
	*pgotMatchPtr = gotMatchPtr;
}

/*}*/

