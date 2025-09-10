/****************************************************************************************/
/*  HUFFMAN2.H                                                                          */
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
#ifndef CRB_HUFFMAN2_H
#define CRB_HUFFMAN2_H

/*
 *  Modular Static Huffman Routines
 * 
 *  These routines do no file IO.  They write to and from a LLBitIO structure.
 *      this structure must be independently allocated and destroyd
 *
 *    You are responsible for calling 
 *      LBitIO_FlushWrite(HI->BII);
 *    yourself.  It is no longer done in EncodeCDone
 *
 *    You must also call
 *      LBitIO_InitRead(HI->BII);
 *    yourself before doing any reads.  It is no longer done in DecodeCInit
 *
 *
 *  Notez:
 *    Huff2_BuildCodeLens changes the values in the CharCounts table!
 *
 */


/*
 * notez : you must PackCodeLens BEFORE BuildEncodeTable
 *  in order to set Min & Max CodeLen
 * if you don't PackCodeLens, call SetMinMaxCodeLen
 */

/*
 *
 * return values for BuildCodeLens :
 *		1 = all ok
 *		0 = error
 *
 */

/*
 *	WARNING :
 * 
 *		do not call Huff2_BuildFastDecodeTable &
 *			Huff2_BuildDecodeTable both on the same Huff2Info !
 *
 *		they use the same memory space, and they use it differently!
 *
 *		you will crash when Huff2_CleanUp is called !
 *
 */

#include "Utility.h"
#include "MemPool.h"
#include "lbitio.h"

struct Huff2CodeNode
  {
	uint16 Char;
	uint16 Count;
  struct Huff2CodeNode * Up;
  struct Huff2CodeNode * Down;
  };

#define HUFF2_CODE_BRANCH 0xFFFF

#define FD_MAXCHARSMADE 4
#define FD_MAXCHARSMADE_SUBONE 3
/* see ' !<>! ' marker */

struct Huff2Info
	{
	long NumSymbols;
	long GotNumSymbols;
	uint16 OneChar;
	struct LBitIOInfo * BII;

	long MinCodeLen,MaxCodeLen;
	uint32 * CodePrefixByLen;
	long * NumCodesOfLen;
	long * CodeLenTable;
	void * EnDe_codeTable;
	long EnDe_codeTableLen;

	/* FastDecode buffer */
	long NumCharsWaiting;
	uint16 CharsWaiting[FD_MAXCHARSMADE_SUBONE];

	/* BuildCodeLen stuff */
	struct Huff2CodeNode * NodeBase;
	struct Huff2CodeNode ** NodeWork;
	struct Huff2CodeNode ** MadeNodeWork;
	struct Huff2CodeNode * CodeNodeHunk;
	long CodeNodeHunkI;
	uint32 * StackArray;
	long SortType;
	void * SortWork;
	long MaxCharCount;
	};

struct FastDecodeItem
	{
	uint8 NumBitsUsed;
	uint8 NumCharsMade;
	uint16 CharsMade[FD_MAXCHARSMADE];
	};

#define HUFF2_SORT_NONE  1 // data must be pre-sorted !
#define HUFF2_SORT_RADIX 2
#define HUFF2_SORT_QSORT 3

//protos:
extern struct Huff2Info * Huff2_Init(long NumSymbols,struct LBitIOInfo * BII,long SortType);
extern void Huff2_CleanUp(struct Huff2Info *HI);

extern void Huff2_GetMaxCharCount(struct Huff2Info *HI,long * CharCounts);
extern void Huff2_ScaleCounts(struct Huff2Info *HI,long * CharCounts,long MaxVal);
extern jeBoolean Huff2_BuildCodeLens(struct Huff2Info *HI,long *CharCounts);

extern jeBoolean Huff2_BuildEncodeTable(struct Huff2Info *HI);
extern jeBoolean Huff2_BuildDecodeTable(struct Huff2Info *HI);
extern jeBoolean Huff2_BuildFastDecodeTable(struct Huff2Info *HI);

extern void Huff2_EncodeC(struct Huff2Info *HI,uint16 C);
extern uint16 Huff2_DecodeC(struct Huff2Info *HI);
extern uint16 Huff2_FastDecodeC(struct Huff2Info *HI);
extern jeBoolean Huff2_FastDecodeArray(struct Huff2Info *HI,uint8 * Array,long ArrayLen);

extern void Huff2_SetMinMaxCodeLen(struct Huff2Info *HI);

extern void Huff2_PackCodeLens(struct Huff2Info *HI);
extern void Huff2_UnPackCodeLens(struct Huff2Info *HI);                                      

extern void Huff2_PackCodeLens_Delta(struct Huff2Info *HI,long * LastCodeLens);
extern void Huff2_UnPackCodeLens_Delta(struct Huff2Info *HI,long * LastCodeLens);

/*
 * macros for faster operation
 *
 */

/********** EncodeC macros ************/

#define Huff2_EncodeC_Macro_Init(HI)                                       \
{                                                                 	       \
register uint32 * CharToCodeTable;                                 	       \
register long * CodeLenTable;                                     	       \
register struct LBitIOInfo * BII;                                 	       \
register uint32 CurCode;                                           	       \
register long CurCodeLen;                                         	       \
jeBoolean docoding=JE_TRUE;                                                        \
BII = HI->BII;                                                    	       \
CodeLenTable = HI->CodeLenTable;                                  	       \
CharToCodeTable = (uint32 *)HI->EnDe_codeTable;                    	       \
if ( HI->GotNumSymbols < 2) docoding = JE_FALSE;
/* end Huff2_EncodeC_Macro_Init */

#define Huff2_EncodeC_Macro_Done(HI)                                       \
}                                                                          \
/* end Huff2_EncodeC_Macro_Done */

#define Huff2_EncodeC_Macro(HI,Symbol)                                     \
if ( ! docoding ) { } else { CurCode 	 = CharToCodeTable[Symbol];          \
CurCodeLen = CodeLenTable[Symbol];                                         \
LBitIO_WriteBits(BII,CurCode,CurCodeLen); }                                \
/* end Huff2_EncodeC_Macro */

/********** end EncodeC macros ************/
	
/********** DecodeC macros ************/

/* no decodec macros.  use FastDecodeArray */

#endif // HUFFMAN2_H
