/****************************************************************************************/
/*  LZA.C                                                                               */
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

//@@ CRAP! Turn this debug stuff off!
//#define STUFF
//#define DO_CRC

#pragma message("lza : needs errrorr handling!")

//#define FAST_RUNLENGTH_MATCHES	// hurts compression a little, helps encoder speed

#define DO_NONGREEDY_LOOKAHEAD
#define AMORTIZED_HASHING
#define UNROLLED_MATCH_LOOP

/********

possible todos:

1. optimal parsing encoder

*********/

#include "Utility.h"
#include "arithc.h"
#include "o0coder.h"
#include "o1coder.h"
#include "IntMath.h"
#include <string.h>
#include "lza.h"
#include "rungae.h"	// rung ae is about 0.001 bpc off 
#include "crc32.h"

// consts :

#if 1  //{ // matchlen of 4

#define HeadLen			(4)		
#define getHead(ptr)	getuint32(ptr)

#define bitsPerMatch 	(16)
#define bitsPerLiteral	(7)

#else //}{

// has bugs !

#define HeadLen			(3)
#define getHead(ptr)	( ((ptr)[0]<<16) + ((ptr)[1]<<8) + ((ptr)[2]) )

#define bitsPerMatch 	(16)
#define bitsPerLiteral	(8)

#endif //}

#define FAST_RUNLENGTH_MINLEN (10)

#define minMatchLen		HeadLen		//minMatchLen >= HeadLen

#define matchLenEscape 	(100)	// not very sensitive

// we like *very* low totmaxes

#define TOTMAX_ORDER0_LITS		(1600)
#define TOTMAX_ORDER1_LITS		(800)

#define TOTMAX_LENS		(2500)
#define TOTMAX_OFFSETS	(100)

#define MINIMUM_RAW_LEN	(32)
#define PRE_LITS_LEN	(16)

#define OFFSET_BITS			(12)			// 12 bits is a 1 Meg window, which uses up to 16 Megs of memory
#define OFFSET_ALPHABET		(OFFSET_BITS + 2)
#define lookupHunkSize 		((1<<(OFFSET_BITS+8))-2)

#define MATCH_CONTEXT_BITS	(2)
#define MATCH_CONTEXT_SIZE	(1<<MATCH_CONTEXT_BITS)
#define MATCH_CONTEXT_MASK	(MATCH_CONTEXT_SIZE - 1)

#ifdef	LOG
#define logprintf	printf
#else
#define logprintf	/##/
#endif

//structs:
struct lookupNode {
	struct lookupNode * next;
	struct lookupNode * prev;
	uint32 Head;
	uint8 *ptrPastHead;
};

#define HASHSIZE	(1<<16)
#define HASHFUNC(x) ( ( x ^ ( x >> 15 ) ) & 0x0000FFFF )

#define GetLit(c)		( Stats_LitsO1 ? (uint8)oOneDecode(Stats_LitsO1,c) : (uint8)ozeroDecode(Stats_LitsO0) )
#define PutLit(c,ctx)	( Stats_LitsO1 ? oOneEncode(Stats_LitsO1,c,ctx) : ozeroEncode(Stats_LitsO0,c) )

static const uint32 nonGreedy_ratioS10 = (((bitsPerMatch+bitsPerLiteral)<<10)/bitsPerMatch);

#ifdef AMORTIZED_HASHING
#define nodesCheckedMax 100		// = 100 hurts about 0.01 bpp from infinity <> !
#endif

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
static long lookupHunkNext;
static jeBoolean lookupFreeFlag;

static struct lookupNode ** lookupTable;
static struct lookupNode * lookupHunk;

// shared stats:
static arithInfo * ari;
static ozero * Stats_Lens;
static ozero * Stats_OffsetBlock;
static ozero * Stats_LitsO0;
static oOne  * Stats_LitsO1;

static rung_t MatchFlagRung[MATCH_CONTEXT_SIZE];
static rung_t OffsetRung;

uint32 Stats_OffsetBlockAlphaBet;

#define CleanUp(str) do { jeErrorLog_AddString(-1,str,NULL); assert((str) == NULL); } while(0)

//protos:
static void Encode(uint8 *rawArray,uint32 rawLen);

static void encodeMatchLen(uint32 gotMatchLen);
static uint32 decodeMatchLen(void);

static void codeMatchFlagInit(void);
static void encodeMatchFlag(jeBoolean bit);
static jeBoolean decodeMatchFlag(void);

static void codeOffsetInit(void);
static void encodeOffset(uint32 offset);
static uint32  decodeOffset(void);

static void addLookupNode(uint8 *rawPtr);
static uint32 tellMatchLen(uint8 *MatchVsPtr1,uint8 *MatchVsPtr2);
static void findMatch(uint8 *rawPtr,uint32 * pgotMatchLen,uint8 **pgotMatchPtr);

static void lzaInit(jeBoolean o1,uint32 rawLen);
static void lzaFree(void);

void lzaEncodeSub(uint8 *rawArray,uint32 rawLen,uint8 **compArrayPtr,uint32 * compLenPtr,
						jeBoolean Order1Lits);

/*}{**************/

void lzaEncode(uint8 *rawArray,uint32 rawLen,uint8 **compArrayPtr,uint32 * compLenPtr)
{
uint8 *comp0,*comp1;
uint32 len0,len1;

	comp0 = comp1 = NULL;

	lzaEncodeSub(rawArray,rawLen,&comp0,&len0,0);
	lzaEncodeSub(rawArray,rawLen,&comp1,&len1,1);

	assert(comp0 && comp1);

	if ( len0 <= len1 )
	{
		*compArrayPtr = comp0;
		*compLenPtr = len0;
		jeRam_Free(comp1);
	}
	else
	{
		*compArrayPtr = comp1;
		*compLenPtr = len1;
		jeRam_Free(comp0);
	}
}

void lzaEncodeSub(uint8 *rawArray,uint32 rawLen,uint8 **compArrayPtr,uint32 * compLenPtr,
						jeBoolean Order1Lits)
{
uint8 *compArray;

	lookupHunkNext = 0;
	lookupFreeFlag = JE_FALSE;

	lookupTable = NULL;
	lookupHunk = NULL;

	ari = NULL;
	Stats_LitsO0 = NULL;
	Stats_LitsO1 = NULL;
	Stats_Lens = NULL;
	Stats_OffsetBlock = NULL;

	assert( (((uint32)rawArray)&3) == 0 );

	if ( (compArray = (uint8*)jeRam_Allocate(rawLen + 16384)) == NULL )
		CleanUp("AllocMem failed!");

	*compArrayPtr = compArray;

	if ( rawLen < MINIMUM_RAW_LEN )
	{
		memcpy(compArray,rawArray,rawLen);
		*compLenPtr = rawLen;
		return;
	}

	if ( (lookupTable = (struct lookupNode **)newarray(void *,HASHSIZE)) == NULL )
		CleanUp("AllocMem failed!");

	if ( (lookupHunk = (struct lookupNode *)jeRam_Allocate(sizeof(struct lookupNode)*min(lookupHunkSize,rawLen+10))) == NULL )
		CleanUp("AllocMem failed!");

	if ( (ari = arithInit()) == NULL )
		CleanUp("arithInit failed!");

#ifdef DO_CRC
	*((uint32 *)compArray) = CRC32_Array(rawArray,rawLen); compArray += 4;
#endif

	arithEncodeInit(ari,compArray);

	lzaInit(Order1Lits,rawLen);

	arithEncBitRaw(ari,Order1Lits);

	Encode(rawArray,rawLen);

	*compLenPtr = arithEncodeDone(ari);

	lzaFree();

	jeRam_Free(lookupTable);
	jeRam_Free(lookupHunk);
	
#ifdef DO_CRC
	*compLenPtr += 4;
#endif

}

/*}{***********************************/

struct lzaDecoder
{
	arithInfo * ari;
	ozero * Stats_LitsO0;
	oOne  * Stats_LitsO1;
	ozero * Stats_Lens;
	ozero * Stats_OffsetBlock;

	rung_t MatchFlagRung[MATCH_CONTEXT_SIZE];
	rung_t OffsetRung;

	uint32 CompLen,TotCompLen,rawLen;
	uint8 *rawPtr,*rawPtrDone,*rawArray;
	
#ifdef DO_CRC
	uint32 crc;
#endif
};

/***
**/

lzaDecoder * lzaDecoder_Create(uint8 *compArray,uint32 TotCompLen,uint32 CurCompLen,uint8 * rawArray,int rawLen)
{
lzaDecoder * Stream;
jeBoolean o1;

	Stream = (lzaDecoder *)new(lzaDecoder);
	assert(Stream);

	assert( (((uint32)compArray)&3) == 0 );
	assert( CurCompLen > 8 );
	assert( TotCompLen >= CurCompLen );

	Stream->rawArray	= rawArray;
	Stream->rawPtr		= rawArray;
	Stream->rawPtrDone	= rawArray + rawLen;
	Stream->rawLen		= rawLen;

	if ( rawLen < MINIMUM_RAW_LEN )
	{
		memcpy(rawArray,compArray,rawLen);
		return Stream;
	}

	Stats_LitsO0 = NULL;
	Stats_LitsO1 = NULL;
	Stats_Lens = NULL;
	Stats_OffsetBlock = NULL;
	ari = NULL;

#ifdef DO_CRC
	Stream->crc = *((uint32 *)compArray);
	compArray += 4;
	CurCompLen -= 4;
	TotCompLen -= 4;
#endif

	if ( (ari = arithInit()) == NULL )
		CleanUp("arithInit failed!");

	arithDecodeInit(ari,compArray);

	o1 = arithDecBitRaw(ari);

	lzaInit(o1,rawLen);

	Stream->ari = ari;
	Stream->Stats_LitsO1 = Stats_LitsO1;
	Stream->Stats_LitsO0 = Stats_LitsO0;
	Stream->Stats_Lens = Stats_Lens;
	Stream->Stats_OffsetBlock = Stats_OffsetBlock;

	Stream->TotCompLen = TotCompLen;
	Stream->CompLen = CurCompLen;

	memcpy(Stream->MatchFlagRung,MatchFlagRung,sizeof(rung_t)*MATCH_CONTEXT_SIZE);
	Stream->OffsetRung = OffsetRung;

#ifdef STUFF
	{
	int got;
		got = arithGet(ari,173);
		assert( got == 69 );
		arithDecode(ari,69,70,173);
	}
#endif

	{
	int i;
		for(i=0;i< PRE_LITS_LEN; i++)
		{
			*(Stream->rawPtr)++ = GetLit(' ');
		}
	}

#ifdef STUFF
	{
	int got;
		got = arithGet(ari,173);
		assert( got == 69 );
		arithDecode(ari,69,70,173);
	}
#endif

return Stream;
}

jeBoolean lzaDecoder_Extend(lzaDecoder * Stream,uint32 AddCompLen,uint32 * pCurAvailable)
{
uint8 *rawPtr,*rawPtrDone;
uint32 stopLen;

	if ( ! Stream->ari )
	{
		*pCurAvailable = (uint32)(Stream->rawPtr - Stream->rawArray );
		return JE_FALSE;
	}

	Stream->CompLen += AddCompLen;
	Stream->CompLen = min(Stream->CompLen,Stream->TotCompLen);

	ari = Stream->ari;

	Stats_LitsO1		= Stream->Stats_LitsO1;
	Stats_LitsO0		= Stream->Stats_LitsO0;
	Stats_Lens			= Stream->Stats_Lens;
	Stats_OffsetBlock	= Stream->Stats_OffsetBlock;

	memcpy(MatchFlagRung,Stream->MatchFlagRung,sizeof(rung_t)*MATCH_CONTEXT_SIZE);
	OffsetRung = Stream->OffsetRung;

	stopLen		= Stream->CompLen;
	rawPtr		= Stream->rawPtr;
	rawPtrDone	= Stream->rawPtrDone;

	assert(rawPtr <= rawPtrDone);

	if( Stream->CompLen != Stream->TotCompLen )
	{
		stopLen = Stream->CompLen - 12;
	}

	if ( arithTellDecPos(ari) <= stopLen && rawPtr < rawPtrDone )
	{
		if ( Stats_LitsO1 )
		{	
			while( arithTellDecPos(ari) <= stopLen )
			{
				if ( ! decodeMatchFlag() )
				{
					*rawPtr = (uint8)oOneDecode(Stats_LitsO1,rawPtr[-1]);
					rawPtr++;
				}
				else
				{
					int CurOff,gotMatchLen;

					if ( rawPtr >= rawPtrDone )
						break;

					gotMatchLen = decodeMatchLen();

					CurOff = decodeOffset();

					{
					uint8 *refPtr;
					refPtr = rawPtr - CurOff;

					assert( refPtr >= Stream->rawArray );

					while(gotMatchLen--)
						*rawPtr++ = *refPtr++;
					}
				}
			}
		}
		else
		{
			while( arithTellDecPos(ari) <= stopLen )
			{
				if ( ! decodeMatchFlag() )
				{
					*rawPtr = (uint8)ozeroDecode(Stats_LitsO0);
					rawPtr++;
				}
				else
				{
					int CurOff,gotMatchLen;

					if ( rawPtr >= rawPtrDone )
						break;

					gotMatchLen = decodeMatchLen();

					CurOff = decodeOffset();

					{
					uint8 *refPtr;
					refPtr = rawPtr - CurOff;

					assert( refPtr >= Stream->rawArray );

					while(gotMatchLen--)
						*rawPtr++ = *refPtr++;
					}
				}
			}
		}
	}

	Stream->rawPtr = rawPtr;
	memcpy(Stream->MatchFlagRung,MatchFlagRung,sizeof(rung_t)*MATCH_CONTEXT_SIZE);
	Stream->OffsetRung = OffsetRung;

	assert(rawPtr <= rawPtrDone);

	*pCurAvailable = (uint32)(rawPtr - Stream->rawArray );

#ifdef _DEBUG
	{
	uint32 ReadCompLen;
		ReadCompLen = arithTellDecPos(ari);
		if ( stopLen != Stream->TotCompLen )
			assert( ReadCompLen < Stream->CompLen );
	}
#endif

	if ( rawPtr >= rawPtrDone)
	{
	
#ifdef DO_CRC
	uint32 crc;
		crc = CRC32_Array(Stream->rawArray,Stream->rawLen);
		assert( crc == Stream->crc );
#endif
	
		return JE_FALSE;
	}

	return JE_TRUE;
}

void lzaDecoder_Destroy(lzaDecoder ** pStream)
{
	assert(pStream);

	if ( *pStream )
	{
	lzaDecoder * Stream = *pStream;

		if ( Stream->ari ) arithFree(Stream->ari);
		if ( Stream->Stats_LitsO0 ) ozeroFree(Stream->Stats_LitsO0);
		if ( Stream->Stats_LitsO1 ) oOneFree(Stream->Stats_LitsO1);
		if ( Stream->Stats_Lens ) ozeroFree(Stream->Stats_Lens);
		if ( Stream->Stats_OffsetBlock ) ozeroFree(Stream->Stats_OffsetBlock);

		destroy(Stream);
		*pStream = NULL;
	}
}

/*}{**************/

static void lzaInit(jeBoolean o1,uint32 rawLen)
{

Stats_LitsO0 = NULL;
Stats_LitsO1 = NULL;
Stats_Lens = NULL;
Stats_OffsetBlock = NULL;

if ( o1 )
{
	if ( (Stats_LitsO1 = oOneCreateMax(ari,256,256,TOTMAX_ORDER1_LITS)) == NULL )
		CleanUp("Order1_Init ariled!");
}
else
{
	if ( (Stats_LitsO0 = ozeroCreateMax(ari,256,TOTMAX_ORDER0_LITS)) == NULL )
		CleanUp("Order1_Init ariled!");
}

if ( (Stats_Lens = ozeroCreateMax(ari,matchLenEscape+1,TOTMAX_LENS)) == NULL )
	CleanUp("Order1_Init ariled!");

Stats_OffsetBlockAlphaBet = intlog2((rawLen>>8)+1) + 2;
Stats_OffsetBlockAlphaBet = min(Stats_OffsetBlockAlphaBet,OFFSET_ALPHABET);

if ( (Stats_OffsetBlock = ozeroCreateMax(ari,Stats_OffsetBlockAlphaBet,TOTMAX_OFFSETS)) == NULL )
	CleanUp("ozeroCreate ariled!");

codeMatchFlagInit();
codeOffsetInit();

}

static void lzaFree(void)
{

if ( ari ) arithFree(ari);

if ( Stats_LitsO0 ) ozeroFree(Stats_LitsO0);
if ( Stats_LitsO1 ) oOneFree(Stats_LitsO1);

if ( Stats_Lens ) ozeroFree(Stats_Lens);
if ( Stats_OffsetBlock ) ozeroFree(Stats_OffsetBlock);

}

/*}{**************/

static void Encode(uint8 *rawArray,uint32 rawLen)
{
uint32 gotMatchLen,offset;
uint8 *rawPtr,*rawArrayPtrDone,*gotMatchPtr;
#ifdef DO_NONGREEDY_LOOKAHEAD
uint8 * gotMatchPtrNext;
uint32 gotMatchLenNext;
#endif

	rawPtr = rawArray;
	rawArrayPtrDone = rawArray + rawLen;

#ifdef STUFF
		arithEncode(ari,69,70,173);
#endif

	{
	int i;
		for(i=0;i< PRE_LITS_LEN; i++)
		{
			PutLit(*rawPtr++,' ');
		}
	}

#ifdef STUFF
		arithEncode(ari,69,70,173);
#endif

	while ( rawPtr < rawArrayPtrDone )
	{
	
#ifdef FAST_RUNLENGTH_MATCHES

		if ( (rawPtr + FAST_RUNLENGTH_MINLEN) <= rawArrayPtrDone )
		{
			gotMatchPtr = NULL;

			if ( getHead(rawPtr-1) == getHead(rawPtr) )
			{
				gotMatchPtr = rawPtr-1;
			}
			else if ( getHead(rawPtr) == getHead(rawPtr+1) )
			{
				gotMatchPtr = rawPtr;
			}
			else if ( getHead(rawPtr+1) == getHead(rawPtr+2) )
			{
				gotMatchPtr = rawPtr+1;
			}
			else if ( getHead(rawPtr+2) == getHead(rawPtr+3) )
			{
				gotMatchPtr = rawPtr+2;
			}

			if ( gotMatchPtr )
			{	
				gotMatchLen = HeadLen;

				{
				uint8 * vsPtr,*vsPtr2;
					vsPtr  = gotMatchPtr + HeadLen;
					vsPtr2 = gotMatchPtr + HeadLen + 1;
					while(*vsPtr++ == *vsPtr2++)
						gotMatchLen++;
				}

				if ( gotMatchLen >= FAST_RUNLENGTH_MINLEN )
				{

					while(rawPtr <= gotMatchPtr && rawPtr < rawArrayPtrDone )
					{
						encodeMatchFlag(0);
						PutLit(rawPtr[0],rawPtr[-1]);
						addLookupNode(rawPtr++);
					}

					if ( (rawPtr + gotMatchLen) > rawArrayPtrDone )
					{
						gotMatchLen = rawArrayPtrDone - rawPtr;
						if ( gotMatchLen < minMatchLen )
						{
							if ( rawPtr < rawArrayPtrDone )
							{
								encodeMatchFlag(0);
								PutLit(rawPtr[0],rawPtr[-1]);
								addLookupNode(rawPtr++);
							}
							continue;
						}
					}

					encodeMatchFlag(1);
					encodeMatchLen(gotMatchLen);

					offset = (uint32)rawPtr - (uint32)gotMatchPtr;
					encodeOffset(offset);

					rawPtr += gotMatchLen;
					addLookupNode(rawPtr-1);
					addLookupNode(rawPtr-2);
					continue;
				}
			}
		}
#endif

		findMatch(rawPtr,&gotMatchLen,&gotMatchPtr);

		if ( gotMatchLen >= minMatchLen )
		{
	#ifdef DO_NONGREEDY_LOOKAHEAD
			findMatch(rawPtr+1,&gotMatchLenNext,&gotMatchPtrNext);

			if ( gotMatchLenNext >= ((nonGreedy_ratioS10 * gotMatchLen + 512)>>10) - 1 )
			{
				/* add literal */

				encodeMatchFlag(0);
				PutLit(rawPtr[0],rawPtr[-1]);
				addLookupNode(rawPtr++);			

				/* set match to peeked match */
				gotMatchLen = gotMatchLenNext;
				gotMatchPtr = gotMatchPtrNext;
			}
	#endif /* DO_NONGREEDY_LOOKAHEAD */

			if ( (rawPtr + gotMatchLen) > rawArrayPtrDone )
			{
				gotMatchLen = rawArrayPtrDone - rawPtr;
				if ( gotMatchLen < minMatchLen )
				{
					if ( rawPtr < rawArrayPtrDone )
					{
						encodeMatchFlag(0);
						PutLit(rawPtr[0],rawPtr[-1]);
						addLookupNode(rawPtr++);
					}
					continue;
				}
			}

			encodeMatchFlag(1);
			encodeMatchLen(gotMatchLen);

			offset = (uint32)rawPtr - (uint32)gotMatchPtr;
			encodeOffset(offset);

			/* add lookup nodes */
			while(gotMatchLen--)
				addLookupNode(rawPtr++);

		}
		else
		{
			encodeMatchFlag(0);
			PutLit(rawPtr[0],rawPtr[-1]);
			addLookupNode(rawPtr++);
		}
	}

	// we only check for termination if we get a matchflag of 1
	encodeMatchFlag(1);

}

/*}{**************/

#define MatchFlagBitInit(c)		rungModelInit(&MatchFlagRung[c])
#define MatchFlagBitEnc(bit,c) 	rungModelEncBit(ari,bit,	&MatchFlagRung[c])
#define MatchFlagBitDec(c) 		rungModelDecBit(ari,		&MatchFlagRung[c])

static uint32 MatchFlag_cntx;

static void codeMatchFlagInit(void)
{
uint32 i;
	MatchFlag_cntx = 0;
	for(i=0;i<MATCH_CONTEXT_SIZE;i++)
	{
		MatchFlagBitInit(i);
	}
}

static void inline encodeMatchFlag(jeBoolean bit)
{
	assert( (bit&1) == bit );
	MatchFlagBitEnc(bit,MatchFlag_cntx);
	MatchFlag_cntx = (MatchFlag_cntx + MatchFlag_cntx + bit)&MATCH_CONTEXT_MASK;
}

static jeBoolean inline decodeMatchFlag(void)
{
jeBoolean bit;
	bit = MatchFlagBitDec(MatchFlag_cntx);
	assert( (bit&1) == bit );
	MatchFlag_cntx = (MatchFlag_cntx + MatchFlag_cntx + bit)&MATCH_CONTEXT_MASK;
return bit;
}

/*}{**************/

#define OffsetBitInit()		rungModelInit(&OffsetRung)
#define OffsetBitEnc(bit) 	rungModelEncBit(ari,bit,	&OffsetRung)
#define OffsetBitDec() 		rungModelDecBit(ari,		&OffsetRung)

static void codeOffsetInit(void)
{
OffsetBitInit();
}

static void inline encodeOffset(uint32 offset)
{
uint32 bits,msb,low;

	assert( offset > 0 );

	low = offset&0xFF;
	offset >>= 8;

	offset++;	// can't use zero cuz 0 and 1 have the same log2
	bits = intlog2(offset);

	assert( bits < Stats_OffsetBlockAlphaBet );
	ozeroEncode(Stats_OffsetBlock,bits);
	msb = (1<<bits);

	assert( msb <= offset );
	assert( msb+msb > offset );

	offset -= msb;

	/** sym < msb now **/
	msb>>=1;
	for(;msb>=1;msb>>=1) 
	{
		OffsetBitEnc((offset&msb));
	}

	arithEncByteRaw( ari, low);
}

static uint32 inline decodeOffset(void)
{
uint32 bits,msb,offset;

	bits = ozeroDecode(Stats_OffsetBlock);
	assert( bits < Stats_OffsetBlockAlphaBet );
	msb = (1<<bits);
	offset = msb - 1;

	msb >>= 1;
	for(;msb>=1;msb>>=1)
	{
		if ( OffsetBitDec() )
			offset += msb;
	}

	offset <<= 8;
	offset += arithDecByteRaw( ari );

return offset;
}

/*}{**************/

static void inline encodeMatchLen(uint32 gotMatchLen)
{
uint32 matchLenOut;
	matchLenOut = gotMatchLen - minMatchLen;
	if ( matchLenOut >= matchLenEscape )
	{
		ozeroEncode(Stats_Lens,matchLenEscape);

		matchLenOut -= matchLenEscape;

		if ( matchLenOut < 0xFF )
		{
			arithEncByteRaw( ari, matchLenOut);
		}
		else
		{
			arithEncByteRaw( ari, 0xFF);
			if ( matchLenOut < 0xFFFF )
			{
				arithEncByteRaw( ari, matchLenOut>>8);
				arithEncByteRaw( ari, matchLenOut&0xFF);
			}
			else
			{
				arithEncByteRaw( ari, 0xFF);
				arithEncByteRaw( ari, 0xFF);
				arithEncByteRaw( ari, (matchLenOut>>24)&0xFF);
				arithEncByteRaw( ari, (matchLenOut>>16)&0xFF);
				arithEncByteRaw( ari, (matchLenOut>>8)&0xFF);
				arithEncByteRaw( ari, matchLenOut&0xFF);
			}
		}
	}
	else
	{
		ozeroEncode(Stats_Lens,matchLenOut);
	}
}

static uint32 inline decodeMatchLen(void)
{
uint32 gotMatchLen;

	gotMatchLen = ozeroDecode(Stats_Lens);

	if ( gotMatchLen == matchLenEscape )
	{
		uint32 matchLenTemp;
		matchLenTemp = arithDecByteRaw( ari );
		if ( matchLenTemp == 0xFF )
		{
			matchLenTemp = arithDecByteRaw( ari );
			matchLenTemp <<= 8;
			matchLenTemp |= arithDecByteRaw( ari );
			if ( matchLenTemp == 0xFFFF )
			{
				matchLenTemp = arithDecByteRaw( ari );
				matchLenTemp <<= 8;
				matchLenTemp |= arithDecByteRaw( ari );
				matchLenTemp <<= 8;
				matchLenTemp |= arithDecByteRaw( ari );
				matchLenTemp <<= 8;
				matchLenTemp |= arithDecByteRaw( ari );
			}
		}
		gotMatchLen += matchLenTemp;
	}

	gotMatchLen += minMatchLen;

return gotMatchLen;
}

/*}{**************/

static void inline addLookupNode(uint8 *rawPtr)
{
struct lookupNode *node;
uint32 curHead,hash;

	node = & lookupHunk[lookupHunkNext++];
	if ( lookupFreeFlag )	
	{
		// lookup is full, must free as we go
		if ( node->prev == NULL )
		{
			curHead = node->Head;
			hash = HASHFUNC(curHead);
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
	node->ptrPastHead = rawPtr + HeadLen;
	node->Head = curHead = getHead(rawPtr);
	hash = HASHFUNC(curHead);
	node->prev = NULL;
	node->next = lookupTable[hash];
	lookupTable[hash] = node;
	if ( node->next ) node->next->prev = node;
}

/*}{**************/

static uint32 inline tellMatchLen(uint8 *MatchVsPtr1,uint8 *MatchVsPtr2)
{
uint32 matchLen;

#ifdef UNROLLED_MATCH_LOOP
			uint8 * Match2Base = MatchVsPtr2;

#define MATCHER (*MatchVsPtr1++ != *MatchVsPtr2++)||

			MATCHER MATCHER MATCHER MATCHER MATCHER MATCHER MATCHER MATCHER MATCHER MATCHER
			MATCHER MATCHER MATCHER MATCHER MATCHER MATCHER MATCHER MATCHER MATCHER MATCHER
			MATCHER MATCHER MATCHER MATCHER MATCHER MATCHER MATCHER MATCHER MATCHER MATCHER
			MATCHER MATCHER MATCHER MATCHER MATCHER MATCHER MATCHER MATCHER MATCHER MATCHER
			MatchVsPtr2++;

			matchLen = MatchVsPtr2 - Match2Base - 1;

			if ( matchLen == 40 )
			{
				MatchVsPtr2--;
				while ( *MatchVsPtr1++ == *MatchVsPtr2++ )
					matchLen++;
			}

#else /* no UNROLLED_MATCH_LOOP */

			matchLen = 0;
			while ( *MatchVsPtr1++ == *MatchVsPtr2++ )
				matchLen++;

#endif /* match loop */

return matchLen;
}

static void findMatch(uint8 *rawPtr,uint32 * pgotMatchLen,uint8 **pgotMatchPtr)
{
uint8 *gotMatchPtr;
uint32 gotMatchLen,matchLen;
uint32 curHead;
struct lookupNode *node;
uint32 hash;
uint8 *rawptrPastHead;
#ifdef AMORTIZED_HASHING
uint32 nodesChecked;
#endif

	curHead = getHead(rawPtr);

	hash = HASHFUNC(curHead);
	node = lookupTable[hash];

#ifdef AMORTIZED_HASHING
	nodesChecked = 0;
#endif

	gotMatchPtr = NULL;
	gotMatchLen = 0;
	rawptrPastHead = rawPtr + HeadLen;
	while(node)
	{
		if ( node->Head == curHead )
		{
			matchLen = tellMatchLen(node->ptrPastHead,rawptrPastHead) + HeadLen;

			if ( matchLen > gotMatchLen )
			{
				gotMatchLen = matchLen;
				gotMatchPtr = node->ptrPastHead;
			}

#ifdef AMORTIZED_HASHING
			nodesChecked++;
			if ( nodesChecked == nodesCheckedMax ) break;
#endif
		}
		node = node->next;
	}

	if ( gotMatchPtr ) gotMatchPtr -= HeadLen;
	*pgotMatchLen = gotMatchLen;
	*pgotMatchPtr = gotMatchPtr;
}

/*}{*******************/

/*************
	this memcpy in asm :

this is irrelevant to our speed, it's just kind of cute

			{
			uint8 *refPtr;
			refPtr = rawPtr - CurOff;

			while(gotMatchLen--)
				*rawPtr++ = *refPtr++;
			}

			__asm {

				mov ecx,gotMatchLen
				mov edi,rawPtr
				mov esi,rawPtr
				mov eax,CurOff
				sub esi,eax
				xor eax,eax

			#if 1
				mov al,[esi+0]
				mov ah,[esi+1]
				mov [edi+0],al
				mov [edi+1],ah
				mov bl,[esi+2]
				mov bh,[esi+3]
				mov [edi+2],bl
				mov [edi+3],bh
				sub ecx,4
			#endif

			#if 1
				rep movsb
			#else
			more:
				mov al, [esi]
				add esi,1
				mov [edi],al
				add edi,1
				dec ecx
				jnz more
			#endif

				mov rawPtr,edi
			}
************/

/*}*******************/
