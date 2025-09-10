/****************************************************************************************/
/*  HUFFA.C                                                                             */
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

/* #define DEBUG */

#include "Utility.h"
#include "StrUtil.h"
#include "lbitio.h"
#include "huffman2.h"
#include "CodeUtil.h"
#include "runtrans.h"

#pragma warning(disable : 4244 4018)

#include "huffa.h" /* for type defines */

/*** protos **/

#define CleanUp(exitmess) { HuffExitMess = exitmess; goto EndOfFunc; }

long MaxIn(long *Array,long Len); /* find max utility */

jeBoolean HuffArray(uint8 *RawArray,uint32 RawLen,
		 uint8 *HuffArray,uint32 * HuffArrayLenPtr,int Type);

jeBoolean O0HuffArray(uint8 *RawArray,uint32 RawLen,
		 uint8 *HuffArray,uint32 * HuffArrayLenPtr,jeBoolean CompressFlag);

jeBoolean O0HuffArrayNoBlock(uint8 *RawArray,uint32 RawLen,
							 uint8 *HuffArray,uint32 * HuffArrayLenPtr,jeBoolean CompressFlag);

jeBoolean O0HuffArrayBII_RT(uint8 *rawArray,uint32 rawLen,struct LBitIOInfo * BII,jeBoolean cFlag);
jeBoolean O0HuffArrayBII(uint8 *RawArray,uint32 RawLen,struct LBitIOInfo * BII,jeBoolean CompressFlag);
jeBoolean O0HuffArrayBII_block(uint8 *RawArray,uint32 RawLen,struct LBitIOInfo * BII,jeBoolean CompressFlag);
jeBoolean O0HuffArrayBII_noblock(uint8 *RawArray,uint32 RawLen,struct LBitIOInfo * BII,jeBoolean CompressFlag);

jeBoolean O1HuffArray(uint8 *RawArray,uint32 RawLen,
							 uint8 *HuffArray,uint32 * HuffArrayLenPtr,jeBoolean CompressFlag);

/*********************** choose order 0 or 1 or raw ***********/

jeBoolean HuffArray(uint8 *RawArray,uint32 RawLen,
		 uint8 *HuffArray,uint32 * HuffArrayLenPtr,int Type)
{
char * HuffExitMess = 0;

if ( RawLen == 0 ) return JE_TRUE;

if ( Type == HUFFA_TYPE_BEST )
	{
	uint8 *TypePtr;
	uint32 BestLen,CurLen;

	TypePtr = HuffArray++;

	Type = HUFFA_TYPE_NONE;
	BestLen = RawLen;

	if ( ! O0HuffArray(RawArray,RawLen,HuffArray,&CurLen,JE_TRUE) ) return(0);

	if ( CurLen < BestLen ) { BestLen = CurLen; Type = HUFFA_TYPE_O0; }

	if ( ! O0HuffArrayNoBlock(RawArray,RawLen,HuffArray,&CurLen,JE_TRUE) ) return(0);

	if ( CurLen < BestLen ) { BestLen = CurLen; Type = HUFFA_TYPE_O0NB; }

	if ( ! O1HuffArray(RawArray,RawLen,HuffArray,&CurLen,JE_TRUE) ) return(0);
	
	if ( CurLen < BestLen ) { BestLen = CurLen; Type = HUFFA_TYPE_O1; }

	if ( Type == HUFFA_TYPE_O1 ) { } /* done */
	else if ( Type == HUFFA_TYPE_O0 )
		{
		if ( ! O0HuffArray(RawArray,RawLen,HuffArray,&CurLen,1) ) return(0);
		}
	else if ( Type == HUFFA_TYPE_NONE )
		{
		memcpy(HuffArray,RawArray,RawLen);
		CurLen = RawLen;
		}

	*TypePtr = Type;
	*HuffArrayLenPtr = CurLen+1;
	}
else
	{
	jeBoolean success=JE_FALSE,CompressFlag;

	if ( Type == HUFFA_TYPE_DEC )
		CompressFlag = JE_FALSE;
	else
		CompressFlag = JE_TRUE;

	if ( CompressFlag )
		*HuffArray++ = Type;
	else
		Type = *HuffArray++;

	switch(Type)
		{
		case HUFFA_TYPE_O1:
			success = O1HuffArray(RawArray,RawLen,HuffArray,HuffArrayLenPtr,CompressFlag);
			break;
		case HUFFA_TYPE_O0:
			success = O0HuffArray(RawArray,RawLen,HuffArray,HuffArrayLenPtr,CompressFlag);
			break;
		case HUFFA_TYPE_O0NB:
			success = O0HuffArrayNoBlock(RawArray,RawLen,HuffArray,HuffArrayLenPtr,CompressFlag);
			break;
		case HUFFA_TYPE_NONE:
			if ( CompressFlag )
				memcpy(HuffArray,RawArray,RawLen);
			else
				memcpy(RawArray,HuffArray,RawLen);
			*HuffArrayLenPtr = RawLen;
			success = JE_TRUE;
			break;
		default:
			CleanUp("Got invalid type flag");
			break;	
		}

	*HuffArrayLenPtr += 1;
	
	return(success);
	}

EndOfFunc:

if ( HuffExitMess )
  {
  BrandoError(HuffExitMess);
  return(JE_FALSE);
  }
else
  {
  return(JE_TRUE);
  }
}

/************** Order 0 ******************/

#define HUFF_MINLEN 8

jeBoolean O0HuffArray(uint8 *RawArray,uint32 RawLen,
							 uint8 *HuffArray,uint32 * HuffArrayLenPtr,jeBoolean CompressFlag)
{
struct LBitIOInfo * BII;
jeBoolean ret;

if ( RawLen == 0 ) return JE_TRUE;

if ( (BII = LBitIO_Init(HuffArray)) == NULL )
	{ BrandoError("LBitIO_Init failed!"); return(0); }

if ( ! CompressFlag )
	{
	LBitIO_InitRead(BII);
	ret = O0HuffArrayBII(RawArray,RawLen,BII,CompressFlag);
	*HuffArrayLenPtr = LBitIO_GetPosD(BII);
	}
else
	{
	ret = O0HuffArrayBII(RawArray,RawLen,BII,CompressFlag);
	*HuffArrayLenPtr = LBitIO_FlushWrite(BII);
	}

LBitIO_CleanUp(BII);

return(ret);
}

jeBoolean O0HuffArrayNoBlock(uint8 *RawArray,uint32 RawLen,
							 uint8 *HuffArray,uint32 * HuffArrayLenPtr,jeBoolean CompressFlag)
{
struct LBitIOInfo * BII;
jeBoolean ret;

if ( RawLen == 0 ) return JE_TRUE;

if ( (BII = LBitIO_Init(HuffArray)) == NULL )
	{ BrandoError("LBitIO_Init failed!"); return(0); }

if ( ! CompressFlag )
	{
	LBitIO_InitRead(BII);
	ret = O0HuffArrayBII_noblock(RawArray,RawLen,BII,CompressFlag);
	*HuffArrayLenPtr = LBitIO_GetPosD(BII);
	}
else
	{
	ret = O0HuffArrayBII_noblock(RawArray,RawLen,BII,CompressFlag);
	*HuffArrayLenPtr = LBitIO_FlushWrite(BII);
	}

LBitIO_CleanUp(BII);

return(ret);
}

/** kind of a cheezy way to do the _RT **/

jeBoolean O0HuffArrayBII_RT(uint8 *rawArray,uint32 rawLen,struct LBitIOInfo * BII,jeBoolean cFlag)
{
uint8 *runArray;
uint32 runLen;
jeBoolean ret;

	if ( (runArray = (uint8*)jeRam_Allocate(rawLen + (rawLen>>3) + 1024)) == NULL )
		return JE_FALSE;

	if ( cFlag ) {
		runLen = doRunTransform(rawArray,rawLen,runArray);
		if ( runLen < rawLen ) {
			LBitIO_WriteBit(BII,1);
			cu_putExpanding_bii(runLen,BII,14,4);
			ret = O0HuffArrayBII(runArray,runLen,BII,cFlag);
		} else {
			LBitIO_WriteBit(BII,0);
			ret = O0HuffArrayBII(rawArray,rawLen,BII,cFlag);
		}
	} else {
		jeBoolean doRT;
		LBitIO_ReadBit(BII,doRT);
		if ( doRT ) {
			runLen = cu_getExpanding_bii(BII,14,4);
			ret = O0HuffArrayBII(runArray,runLen,BII,cFlag);
			unRunTransform(rawArray,rawLen,runArray);
		} else {
			ret = O0HuffArrayBII(rawArray,rawLen,BII,cFlag);
		}
	}

	destroy(runArray);

return ret;
}

#define DOBLOCK_MINLEN 1024
#define DOBLOCK_DIVISOR 4

jeBoolean O0HuffArrayBII(uint8 *RawArray,uint32 RawLen,struct LBitIOInfo * BII,jeBoolean CompressFlag)
{

if ( RawLen == 0 ) return JE_TRUE;

if ( RawLen < DOBLOCK_MINLEN )
	{
	return( O0HuffArrayBII_noblock(RawArray,RawLen,BII,CompressFlag) );
	}
else
	{
	jeBoolean doblock=0;

	if ( CompressFlag )
		{
		long MaxCount,i;
		long * CharCounts = NULL;

		if ( (CharCounts = (long *)jeRam_Allocate(256*sizeof(long))) == NULL )
			{ BrandoError("jeRam_Allocate failed!"); return(0); }

		memclear(CharCounts,256*sizeof(long));
		for(i=0;i<RawLen;i++) CharCounts[RawArray[i]] ++;
		MaxCount = CharCounts[MaxIn(CharCounts,256)];
		destroy(CharCounts);

		if ( (MaxCount*DOBLOCK_DIVISOR) >= RawLen ) doblock = 1;
		else doblock = 0;
		}

	if ( CompressFlag )	{	LBitIO_WriteBit(BII,doblock);	}
	else { LBitIO_ReadBit(BII,doblock); }

	if ( doblock ) return( O0HuffArrayBII_block(RawArray,RawLen,BII,CompressFlag) );
	else return( O0HuffArrayBII_noblock(RawArray,RawLen,BII,CompressFlag) );
	}

return(0);
}

/** this is the core routine of it all, the only one that
		actually does huffman: **/

jeBoolean O0HuffArrayBII_noblock(uint8 *RawArray,uint32 RawLen,struct LBitIOInfo * BII,jeBoolean CompressFlag)
{
struct Huff2Info * HI = NULL;
long * CharCounts = NULL;
char * HuffExitMess = NULL;

if ( RawLen == 0 ) return JE_TRUE;

#ifdef DEBUG
if ( CompressFlag )
	{
	LBitIO_WriteBits(BII,0xD,5);
	}
else
	{
	int test;
	LBitIO_ReadBits(BII,test,5);
	if ( test != 0xD ) { BrandoError("o0noblock:init:didn't get tag"); return(0); }
	}
#endif

if ( RawLen < HUFF_MINLEN )
	{
	if ( CompressFlag )
		{
		while(RawLen--)
			{
			LBitIO_WriteBits(BII,*RawArray,8); RawArray++;
			}
		}
	else
		{
		while(RawLen--)
			{
			LBitIO_ReadBits(BII,*RawArray,8); RawArray++;
			}
		}
	return(1);
	}

if ( (HI = Huff2_Init(256,BII,HUFF2_SORT_RADIX)) == NULL )
	CleanUp("Huff2_Init failed!");

if ( ! CompressFlag )
	{
	Huff2_UnPackCodeLens(HI);
	Huff2_BuildFastDecodeTable(HI);
	Huff2_FastDecodeArray(HI,RawArray,RawLen);
  }
else //Encode
	{
	long i;

	if ( (CharCounts = (long *)jeRam_Allocate(256*sizeof(long))) == NULL )
		CleanUp("jeRam_Allocate failed!");

	memclear(CharCounts,256*sizeof(long));
	for(i=0;i<RawLen;i++) CharCounts[RawArray[i]] ++;

	Huff2_ScaleCounts(HI,CharCounts,256);
	Huff2_BuildCodeLens(HI,CharCounts);
	Huff2_PackCodeLens(HI);
	Huff2_BuildEncodeTable(HI);

	Huff2_EncodeC_Macro_Init(HI);
	for(i=0;i<RawLen;i++)
		{
		Huff2_EncodeC_Macro(HI,RawArray[i]);
		}
	Huff2_EncodeC_Macro_Done(HI);

  }

CleanUp(NULL);

EndOfFunc:

#ifdef DEBUG
if ( CompressFlag )
	{
	LBitIO_WriteBits(BII,0xD,5);
	}
else
	{
	int test;
	LBitIO_ReadBits(BII,test,5);
	if ( test != 0xD ) { BrandoError("O0noblock:EOF:didn't get tag"); }
	}
#endif

destroy(CharCounts);
if ( HI ) Huff2_CleanUp(HI);

if ( HuffExitMess )
  {
  BrandoError(HuffExitMess);
  return(JE_FALSE);
  }
else
  {
  return(JE_TRUE);
  }
}

jeBoolean O0HuffArrayBII_block(uint8 *RawArray,uint32 RawLen,struct LBitIOInfo * BII,jeBoolean CompressFlag)
{
long * CharCounts = NULL;
char * HuffExitMess = NULL;
uint8 * BlockArray = NULL;
uint8 * LitArray = NULL;
uint32 BlockLen,NumLits;
uint8 MPS0,MPS1,MPS2;

if ( RawLen == 0 ) return JE_TRUE;

BlockLen = ((RawLen-1)/4) + 1;
if ( (BlockArray = (uint8*)jeRam_Allocate(BlockLen)) == NULL )
	CleanUp("block jeRam_Allocate failed");

if ( ! CompressFlag )
	{
	long bi,v,ri,li,shift;

	LBitIO_ReadBits(BII,MPS0,8);
	LBitIO_ReadBits(BII,MPS1,8);
	LBitIO_ReadBits(BII,MPS2,8);

	if ( ! O0HuffArrayBII_noblock(BlockArray,BlockLen,BII,0) )
		CleanUp("o0_noblock failed");

	NumLits=0;
	for(bi=0;bi<BlockLen;bi++)
		{
		v=0x3;
		for(li=4;li--;)
			{
			if ( ( BlockArray[bi] & v ) == v ) NumLits++;
			v <<= 2;
			}
		}

	if ( NumLits > 0 )
		{
		if ( (LitArray = (uint8*)jeRam_Allocate(NumLits)) == NULL )
			CleanUp("lits jeRam_Allocate failed");

		if (! O0HuffArrayBII_noblock(LitArray,NumLits,BII,0) )
			CleanUp("o0_noblock failed");
		}

	ri = li= 0;
	for(bi=0;bi<BlockLen;bi++)
		{
		v = BlockArray[bi];
		for(shift=6;shift>=0;shift-=2)
			{
			switch( ((v>>shift)&0x3) )
				{
				case 0: RawArray[ri++] = MPS0; break;
				case 1: RawArray[ri++] = MPS1; break;
				case 2: RawArray[ri++] = MPS2; break;
				case 3: RawArray[ri++] = LitArray[li++]; break;
				}
			if ( ri == RawLen )
				{
				if ( bi != (BlockLen-1) ) CleanUp("HuffBlock:didn't read enough blocks!");
				CleanUp(NULL);
				}
			}
		if ( li > NumLits ) CleanUp("HuffBlock:Read too many literals!");
		}
	if ( ri != RawLen ) CleanUp("HuffBlock:Didn't write enough!");
  }
else //Encode
	{
	long bi,bcnt,v,li,ri,c;

	if ( (LitArray = (uint8*)jeRam_Allocate(RawLen)) == NULL )
		CleanUp("jeRam_Allocate failed");

	if ( (CharCounts = (long *)jeRam_AllocateClear(256*sizeof(long))) == NULL )
		CleanUp("jeRam_Allocate failed!");

	for(ri=0;ri<RawLen;ri++) CharCounts[RawArray[ri]] ++;

	MPS0 = MaxIn(CharCounts,256); CharCounts[MPS0] = 0;
	MPS1 = MaxIn(CharCounts,256); CharCounts[MPS1] = 0;
	MPS2 = MaxIn(CharCounts,256); CharCounts[MPS2] = 0;

	LBitIO_WriteBits(BII,MPS0,8);
	LBitIO_WriteBits(BII,MPS1,8);
	LBitIO_WriteBits(BII,MPS2,8);

	ri = li= 0;
	for(bi=0;bi<BlockLen;bi++)
		{
		v = 0;
		for(bcnt=4;bcnt--;)
			{
			v <<= 2;
			if ( ri >= RawLen ) c = MPS0;
			else c = RawArray[ri++];

			if ( c == MPS0 ) v += 0;
			else if (c == MPS1 ) v += 1;
			else if ( c == MPS2 ) v += 2;
			else { v += 3; LitArray[li++] = c; }
			}
		BlockArray[bi] = v;
		}

	NumLits = li;

	if ( ! O0HuffArrayBII_noblock(BlockArray,BlockLen,BII,1) )
		CleanUp("o0_noblock failed");
 
	if ( ! O0HuffArrayBII_noblock(LitArray,NumLits,BII,1) )
		CleanUp("o0_noblock failed");

  }

CleanUp(NULL);

EndOfFunc:

destroy(LitArray);
destroy(BlockArray);
destroy(CharCounts);

if ( HuffExitMess )
  {
  BrandoError(HuffExitMess);
  return(JE_FALSE);
  }
else
  {
  return(JE_TRUE);
  }
}


/**************** Order 1 ********************/

#define MERJE_LEN 64

#define BASEC ' '

void o1len_write(struct LBitIOInfo * BII,uint32 len)
{
if ( len == 0 )
	{
	LBitIO_WriteBit(BII,0);
	}
else
	{
	uint32 nbits,mask;

	LBitIO_WriteBit(BII,1);
	len--;

	nbits = 4;
	mask = (1<<nbits)-1;
	while( len >= mask )
		{
		LBitIO_WriteBits(BII,mask,nbits);
		len -= mask;
		nbits += 4;
		if ( nbits >= 32 ) mask = 0xFFFFFFFF;
		else mask = (1<<nbits)-1;
		}
	LBitIO_WriteBits(BII,len,nbits);
	}
return;
}

uint32 o1len_read(struct LBitIOInfo *BII)
{
uint32 readval;
uint32 len;

LBitIO_ReadBit(BII,readval);
if ( readval == 0 )
	{
	return(0);
	}
else
	{
	uint32 nbits,mask;

	len = 1;

	nbits = 4;
	do
		{
		if ( nbits >= 32 ) mask = 0xFFFFFFFF;
		else mask = (1<<nbits)-1;
		LBitIO_ReadBits(BII,readval,nbits);
		len += readval;
		nbits += 4;
		} while(readval == mask);

	return(len);
	}

}

jeBoolean O1HuffArray(uint8 *RawArray,uint32 RawLen,
							 uint8 *HuffArray,uint32 * HuffArrayLenPtr,jeBoolean CompressFlag)
{
struct LBitIOInfo * BII = NULL;
char * HuffExitMess = NULL;
uint8 ** o1Arrays = NULL;
uint8 ** o1ArrayPtrs = NULL;
uint32 * o1ArrayLens = NULL;
int lc;
uint32 len;
uint8 *mergePtr;

if ( RawLen == 0 ) return JE_TRUE;

if ( (BII = LBitIO_Init(HuffArray)) == NULL )
	CleanUp("LBitIO_Init failed!");

if ( (o1Arrays = (uint8**)jeRam_Allocate(257*sizeofpointer)) == NULL )
	CleanUp("jeRam_Allocate failed!");
for(lc=0;lc<257;lc++) o1Arrays[lc] = NULL;

if ( (o1ArrayPtrs = (uint8**)jeRam_Allocate(257*sizeofpointer)) == NULL )
	CleanUp("jeRam_Allocate failed!");

if ( (o1ArrayLens = (uint32*)jeRam_Allocate(257*sizeof(uint32))) == NULL )
	CleanUp("jeRam_Allocate failed!");

for(lc=0;lc<257;lc++) o1ArrayLens[lc] = 0;

if ( ! CompressFlag )
	{
	uint32 totlen=0;

	LBitIO_InitRead(BII);

	for(lc=0;lc<257;lc++)
		{
		o1ArrayLens[lc] = len = o1len_read(BII);
		if ( len > 0 )
			{
			if ( lc != 256 ) totlen += len;

			if ( (o1Arrays[lc] = (uint8*)jeRam_Allocate(len)) == NULL )
				CleanUp("jeRam_Allocate failed!");
			o1ArrayPtrs[lc] = o1Arrays[lc];

			if ( len >= MERJE_LEN || lc == 256 )
				{
				if ( ! O0HuffArrayBII(o1Arrays[lc],len,BII,0) )
					CleanUp("O0HuffArrayBII failed");
				}
			}
		}

	if ( totlen != RawLen )
		CleanUp("total o1 lens != RawLen");

	totlen = o1ArrayLens[256];
	mergePtr = o1Arrays[256];
	for(lc=0;lc<256;lc++)
		{
		if ( o1ArrayLens[lc] < MERJE_LEN )
			{
			uint8 * aPtr;
			len = o1ArrayLens[lc];
			totlen -= len;
			aPtr = o1Arrays[lc];
			while(len--) *aPtr++ = *mergePtr++;
			}
		}

	if ( totlen != 0 )
		CleanUp("sum of merged arrays lens != MergeArrayLen");

	lc=BASEC;
	while(RawLen--) lc = *RawArray++ = *o1ArrayPtrs[lc]++;
	
	*HuffArrayLenPtr = LBitIO_GetPosD(BII);
  }
else //Encode
	{
	uint32 i;
	uint8 * CurArray;

	lc=BASEC;
	for(i=0;i<RawLen;i++) { o1ArrayLens[lc]++; lc = RawArray[i]; }

	o1ArrayLens[256] = RawLen;
	for(lc=0;lc<257;lc++)
		{
		if ( o1ArrayLens[lc] > 0 )
			{
			if ( (o1Arrays[lc] = (uint8*)jeRam_Allocate(o1ArrayLens[lc])) == NULL )
				CleanUp("jeRam_Allocate failed!");
			}
		o1ArrayPtrs[lc] = o1Arrays[lc];
		}
	o1ArrayLens[256] = 0;

	lc=BASEC;
	for(i=0;i<RawLen;i++) lc = *o1ArrayPtrs[lc]++ = RawArray[i];

	mergePtr = o1Arrays[256];

	for(lc=0;lc<257;lc++)
		{
		len = o1ArrayLens[lc];
		o1len_write(BII,len);

		if ( len >= MERJE_LEN || lc == 256 )
			{
			if ( ! O0HuffArrayBII(o1Arrays[lc],len,BII,1) )
				CleanUp("O0HuffArrayBII failed");
			}
		else
			{
			o1ArrayLens[256] += len;
			CurArray = o1Arrays[lc];
			while(len--) *mergePtr++ = *CurArray++;
			}
		}

	*HuffArrayLenPtr = LBitIO_FlushWrite(BII);
  }

CleanUp(NULL);

EndOfFunc:

destroy(o1ArrayLens);
destroy(o1ArrayPtrs);
if ( o1Arrays )
	{
	for(lc=0;lc<257;lc++) destroy( o1Arrays[lc] );
	destroy(o1Arrays);
	}
if ( BII ) LBitIO_CleanUp(BII);

if ( HuffExitMess )
  {
  BrandoError(HuffExitMess);
  return(JE_FALSE);
  }
else
  {
  return(JE_TRUE);
  }
}



long MaxIn(long *Array,long Len)
{
long Max = -1,Found = 0,i;
for(i=0;i<Len;i++) { if ( Array[i] > Max ) { Max = Array[i]; Found = i; } }
return(Found);
}

