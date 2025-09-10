/****************************************************************************************/
/*  ARRAY.C                                                                             */
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
/*}{************************/

/*
 * Array
 *
 * by cbloom , January-February, 1999
 * fixed by jpollard, Feb 1999.
 * jpollard - jeArray_Create was not setting Signature1/2 - 03/05/1999
 * jpollard - Fixed NumFreedElements check in jeArray_IsValid - 03/05/1999
 *
 */

#include <string.h>
#include <assert.h>

#include "Array.h"
#include "Errorlog.h"
#include "log.h"

/*
 *  jeArray
 *		variable length array system
 *		with defragmenter
 *    does auto-extending of memory space in case you use more space than
 *     expected or if you don't know how much you will need
 *	HunkLength is forced to be a multiple of 4
 *
 *	jeArray is slightly above a 'root' level object.  
 *		We sit only above 'Ram', 'list', and 'mempool' in the heirarchy
 *
 */

/*}{************ Internal protos & macros ************/

#if 0	// in jet
#include "ram.h"
#define RamCalloc(size)			jeRam_AllocateClear(size)
#define RamFree(mem)			jeRam_Free(mem)
#define RamRealloc(mem,size)	jeRam_Realloc(mem,size)
#else
#include <stdlib.h>
#define RamCalloc(size)			calloc(1,size)
#define RamFree(mem)			free(mem)
#define RamRealloc(mem,size)	realloc(mem,size)
#endif

#ifndef memclear
#define memclear(mem,size)		memset(mem,0,size)
#endif

jeBoolean jeArray_IsValid(const jeArray * Array);

#ifndef max
#define max(a,b) (((a)>(b))?(a):(b))  
#endif
/*}{************ Structs ************/

#pragma message("jeArray element structure is bloated")

typedef struct Element Element;
struct Element
{
	jeArray_Index	Index;		// @@ could compute this with one subtract & one divide
	jeBoolean		IsInUse;	// @@ this is redundant; look in the IndexTable[] for null
	uint8			Data[1];
};

#define ElementHeaderSize	((uint32)&(((Element *)0)->Data))

#define Hunk2Element(ptr)	((Element *)( ((uint32)(ptr)) - ElementHeaderSize ))
#define Element2Hunk(h)		((void *)( (uint32)(ptr) + ElementHeaderSize ))

typedef struct MemBlock MemBlock;
struct MemBlock
{
	MemBlock * Next;
	char * Memory;
	int CurItem,NumItems;
	jeArray_Index BaseIndex;
};

#define jeArray_Signature	((uint32)0xABADF00D)
#define JE_ARRAY_VERSION	0x0000

struct jeArray
{
	uint32 Signature1;
	int RefCount;
	int HunkLength,ElementLength;
	jeArray_Index NextBaseIndex;
	MemBlock * CurMemBlock; // jump into the list
	MemBlock * MemList; // list of memblocks
	int AutoExtendNumItems;
	int NumFreedElements,MaxNumFreedElements;
	void ** FreedElements;
	uint32 Signature2;
	void ** IndexTable;	// points to *Data*
	jeArray_Index MaxIndex;	// the actual max index could be smaller than this!
	jeArray_Index IndexTableLen;
};

/*}{************* Structs ***********/

JETAPI jeArray * JETCC jeArray_Create (int32 HunkLength, int32 NumHunks, int32 AutoExtendNumItems)
{
	jeArray * Array;

	if ( (Array = (jeArray *)RamCalloc(sizeof(jeArray))) == NULL )
	  return(NULL);

	Array->HunkLength = (HunkLength + 3)&(~3);
	Array->ElementLength = ElementHeaderSize + Array->HunkLength;
	Array->CurMemBlock = NULL;
	Array->MemList = NULL;
	Array->NumFreedElements = 0;
	Array->MaxNumFreedElements = 16;
	Array->AutoExtendNumItems = AutoExtendNumItems;
	Array->RefCount = 1;
	Array->NextBaseIndex = 0;
	Array->IndexTableLen = ((Array->ElementLength * NumHunks + AutoExtendNumItems)>>2) + 16;

	Array->Signature1 = jeArray_Signature;
	Array->Signature2 = jeArray_Signature;

	Array->MaxIndex = 0;

	if ( (Array->FreedElements = (void **)RamCalloc(Array->MaxNumFreedElements*sizeof(void *))) == NULL )
	{
		RamFree(Array);
		return(NULL);
	}

	if ( (Array->IndexTable = (void **)RamCalloc(Array->IndexTableLen*sizeof(void *))) == NULL )
	{
		RamFree(Array->FreedElements);
		RamFree(Array);
		return(NULL);
	}

	if ( ! jeArray_Extend(Array,NumHunks) )
	{
		RamFree(Array->FreedElements);
		RamFree(Array->IndexTable);
		RamFree(Array);
		return(NULL);
	}

	return Array;
}

JETAPI void JETCC jeArray_Destroy(jeArray ** pArray)
{
	MemBlock	*CurMemBlock;
	MemBlock	*NextMemBlock;

	assert(pArray);
	assert(*pArray);

	assert((*pArray)->RefCount > 0);

	(*pArray)->RefCount--;

	if ((*pArray)->RefCount == 0 )
	{
		CurMemBlock = (*pArray)->MemList;

		while(CurMemBlock)
		{
			RamFree(CurMemBlock->Memory);
			NextMemBlock = CurMemBlock->Next;
			RamFree(CurMemBlock);
			CurMemBlock = NextMemBlock;
		}

		RamFree((*pArray)->IndexTable);
		RamFree((*pArray)->FreedElements);
		RamFree((*pArray));
	}

	*pArray = NULL;
}

JETAPI void JETCC jeArray_CreateRef(jeArray * Array)
{
	Array->RefCount ++;
}

JETAPI void JETCC jeArray_Reset(jeArray * Array)
{
	MemBlock * MB;

	Array->NumFreedElements = 0;
	Array->CurMemBlock = Array->MemList;

	for( MB = Array->MemList; MB ; MB = MB->Next)
	{
		MB->CurItem = 0;
		memclear(MB->Memory,MB->NumItems * Array->ElementLength);
	}
}

JETAPI jeBoolean JETCC jeArray_Extend(jeArray * Array, int32 NumHunks)
{
	MemBlock * MB;

	MB = Array->MemList;
	while ( MB )
	{
		if ( MB->CurItem < MB->NumItems )
		{
			Array->CurMemBlock = MB;
			return JE_TRUE;
		}
		MB = MB->Next;
	}

	if ( (MB = (MemBlock *)RamCalloc(sizeof(MemBlock))) == NULL )
		return JE_FALSE;

	MB->CurItem = 0;
	MB->NumItems = NumHunks;

	if ( (MB->Memory = (char *)RamCalloc(NumHunks * Array->ElementLength)) == NULL )
	{
		RamFree(MB);
		return JE_FALSE;
	}

	MB->BaseIndex = Array->NextBaseIndex;
	Array->NextBaseIndex += NumHunks;

	MB->Next = Array->MemList;
	Array->MemList = MB;

	Array->CurMemBlock = MB;

	return JE_TRUE;
}

/*}{************************/

// GetNewElement and FreeElement should be SIGNIFICANTLY
// faster than malloc() & free() .  We currently have that.

JETAPI void * JETCC jeArray_GetNewElement(jeArray * Array)
{
	jeArray_Index I;
	MemBlock * MB;
	Element * E;

	if ( Array->NumFreedElements > 0 )
	{
		Array->NumFreedElements--;
		E = (Element *)Array->FreedElements[Array->NumFreedElements];
		assert(E);
		// E->Index should already be set up
		E->IsInUse = JE_TRUE;
		Array->IndexTable[E->Index] = E->Data;
		Array->MaxIndex = max(Array->MaxIndex,E->Index);
		return E->Data;
	}

	if ( (MB = Array->CurMemBlock) == NULL )
		return NULL;

	if ( MB->CurItem == MB->NumItems )
	{
		if ( ! jeArray_Extend(Array,Array->AutoExtendNumItems) )
			return NULL;
		MB = Array->CurMemBlock;
		assert( MB->CurItem < MB->NumItems );
	}

	I = MB->CurItem + MB->BaseIndex;

	E = (Element *)(MB->Memory + MB->CurItem * Array->ElementLength);
	E->Index = I;
	E->IsInUse = JE_TRUE;

	MB->CurItem ++;

	if ( I >= Array->IndexTableLen )
	{
	void * New;
	int NextLen;
		NextLen = Array->IndexTableLen + (Array->AutoExtendNumItems*2) + 257;
		New = RamRealloc(Array->IndexTable,NextLen*sizeof(void *));
		if ( ! New )
			return NULL;
		Array->IndexTableLen = NextLen;
		Array->IndexTable = (void **)New;
	}
	Array->IndexTable[I] = E->Data;

	Array->MaxIndex = max(Array->MaxIndex,I);

	return E->Data;
}

JETAPI jeBoolean JETCC jeArray_FreeElement(jeArray * Array,void * Hunk)
{
	Element * E;

	assert( jeArray_IsValid(Array) );
	assert( Hunk );

	E = Hunk2Element(Hunk);

	assert( E->IsInUse );
	assert( E->Index >= 0 && E->Index < Array->NextBaseIndex );

	E->IsInUse = JE_FALSE;

	// we could use a link list of freed hunks, with the list mempool :^)

	if ( Array->NumFreedElements >= Array->MaxNumFreedElements )
	{
	void * New;
		New = RamRealloc(Array->FreedElements,(Array->MaxNumFreedElements<<1)*sizeof(void *));
		if ( ! New )
			return JE_FALSE;
		Array->MaxNumFreedElements <<= 1;
		Array->FreedElements = (void **)New;
	}

	memclear(Hunk,Array->HunkLength);

	Array->IndexTable[E->Index] = NULL;
	Array->FreedElements[Array->NumFreedElements] = E;
	Array->NumFreedElements++;

return JE_TRUE;
}

/*}{************************/

// _GetElement() and _GetElementIndex() are the work-horses
// these should be as lean as possible !

JETAPI jeArray_Index JETCC jeArray_GetElementIndex(void * H)
{
Element * E;
	assert(H);
	E = Hunk2Element(H);
return E->Index;
}

JETAPI void * JETCC jeArray_GetElement(jeArray * Array,jeArray_Index Index)	// == Array[Index]
{
void * H;
	assert(jeArray_IsValid(Array));
	if ( Index > Array->MaxIndex )
		return NULL;
	H = Array->IndexTable[Index];
	// H could be NULL
return H;
}

JETAPI void * JETCC jeArray_GetNextElement(jeArray * Array,void * H)
{
Element * E;
jeArray_Index i;
	if ( H )
	{
		E = Hunk2Element(H);
		i = E->Index + 1;
	}
	else
	{
		i = 0;
	}
	assert( Array->MaxIndex < Array->IndexTableLen );
	
	if ( i > Array->MaxIndex )
		return NULL;
	while ( ! Array->IndexTable[i] )
	{
		i++;
		if ( i > Array->MaxIndex )
			return NULL;
	}
	H = Array->IndexTable[i];
#ifdef _DEBUG
	E = Hunk2Element(H);
	assert( E->IsInUse );
#endif
return H;
}

JETAPI jeArray_Index JETCC jeArray_GetNextIndex(jeArray * Array, jeArray_Index Index)
{
	if (Index == JE_ARRAY_NULL_INDEX)
		Index = 0;
	else
		Index++;

	assert( Array->MaxIndex < Array->IndexTableLen );
	while (!Array->IndexTable[Index])
	{
		Index++;
		if ( Index > Array->MaxIndex)
			return JE_ARRAY_NULL_INDEX;
	}
	
	return Index;
}

/** // old school!
MemBlock * MB;
int I;

	// <> optimize this for Index == LastIndex + 1

	MB = Array->MemList;
	while( MB )
	{
		I = Index - MB->BaseIndex;
		if ( I < MB->NumItems )
		{
		Element * E;
			E = (Element *)(MB->Memory + I * Array->ElementLength);
			if ( ! E->IsInUse )
				return NULL;
			assert(E->Index == Index);
			return E->Data;
		}
		MB = MB->Next;
	}
return NULL;
***/

/*}{************************/

JETAPI jeArray * JETCC jeArray_CreateFromFile(jeVFile * File, uint16 lVersionSizeOffset, jeArray_IOFunc ElementReader,void *ReaderContext)
{
	jeArray			*Array;
	jeArray_Index	Index;
	MemBlock		*MB;
	Element			*E;
	uint16			Version;

	if ( (Array = (jeArray *)RamCalloc(sizeof(jeArray))) == NULL )
	  return(NULL);

	if ( ! jeVFile_Read(File,&(Array->Signature1),sizeof(Array->Signature1)) )
		return NULL;

	if ( Array->Signature1 != jeArray_Signature )
	{
		jeErrorLog_AddString(-1,"jeArray_CreateFromFile : Signatures don't match!",NULL);
		return NULL;
	}

	if ( ! jeVFile_Read(File,&Version,sizeof(Version)) )
		return NULL;

	if (Version != JE_ARRAY_VERSION)
		return NULL;

	if ( ! jeVFile_Read(File,&(Array->HunkLength),sizeof(Array->HunkLength)) )
		return NULL;

	if ( ! jeVFile_Read(File,&(Array->MaxIndex),sizeof(Array->MaxIndex)) )
		return NULL;

	// add to the HunkLength the difference due to version change
	Array->HunkLength += lVersionSizeOffset;

	Array->ElementLength = ElementHeaderSize + Array->HunkLength;
	Array->CurMemBlock = NULL;
	Array->MemList = NULL;
	Array->NumFreedElements = 0;
	Array->MaxNumFreedElements = 16;
	Array->AutoExtendNumItems = 32768 / Array->ElementLength;
	Array->RefCount = 1;
	Array->NextBaseIndex = 0;
	Array->IndexTableLen = Array->MaxIndex + 1 + Array->AutoExtendNumItems + 16;
	
	Log_Printf("Array count %d : Elt size %d (Header %d - Total %d)\n", Array->MaxIndex, Array->HunkLength, ElementHeaderSize, Array->ElementLength);

	Array->Signature1 = jeArray_Signature;
	Array->Signature2 = jeArray_Signature;

	if ( (Array->FreedElements = (void **)RamCalloc(Array->MaxNumFreedElements*sizeof(void *))) == NULL )
		goto fail;

	if ( (Array->IndexTable = (void **)RamCalloc(Array->IndexTableLen*sizeof(void *))) == NULL )
		goto fail;

	if ( ! jeArray_Extend(Array,Array->IndexTableLen) )
		goto fail;

	MB = Array->MemList;
	assert(MB->NumItems > (int)Array->MaxIndex );

	// read in the array
	// hook up the indextable
	// hook up the freed hunks

	for(Index = 0;Index <= Array->MaxIndex; Index++ )
	{
		uint8 UseFlag;

		if ( ! jeVFile_Read(File,&UseFlag,sizeof(UseFlag)) )
			return NULL;

		E = (Element *)(MB->Memory + Index * Array->ElementLength);

		E->Index = Index;

		if ( UseFlag )
		{
			E->IsInUse = JE_TRUE;

			ElementReader(File,E->Data,ReaderContext);

			Array->IndexTable[Index] = E->Data;
			MB->CurItem++;
		}
		else
		{
			E->IsInUse = JE_FALSE;

			if ( Array->NumFreedElements >= Array->MaxNumFreedElements )
			{
				void * New;

				New = RamRealloc(Array->FreedElements,(Array->MaxNumFreedElements<<1)*sizeof(void *));
				if ( ! New )
					return NULL;
				Array->MaxNumFreedElements <<= 1;
				Array->FreedElements = (void **)New;
			}

			Array->IndexTable[Index] = NULL;

			Array->FreedElements[Array->NumFreedElements] = E;
			Array->NumFreedElements++;
		}
	}

	return Array;

	fail:

	if ( Array->MemList )
	{
		jeArray_Destroy(&Array);
	}
	else
	{
		if ( Array->FreedElements ) RamFree(Array->FreedElements);
		if ( Array->IndexTable ) RamFree(Array->IndexTable);
		RamFree(Array);
	}

	return NULL;
}

void jeArray_AddMemBlocks(jeArray *Array,MemBlock *MB)
{
	MemBlock * Next;
	Next = MB->Next;

	// add to head

	MB->Next = Array->MemList;
	Array->MemList = MB;
	
	if ( Next )
		jeArray_AddMemBlocks(Array,Next);
}

void jeArray_SortMemBlocks(jeArray *Array)
{
	MemBlock *MB;
	MB = Array->MemList;
	Array->MemList = NULL;
	jeArray_AddMemBlocks(Array,MB);
}

JETAPI jeBoolean JETCC jeArray_WriteToFile(const jeArray * Array,jeVFile * File, jeArray_IOFunc ElementWriter,void *WriterContext)
{
	MemBlock		*MB;
	jeArray_Index	Index;
	uint16			Version;

	assert( jeArray_IsValid(Array) );
	assert( File && ElementWriter );

	if ( ! jeVFile_Write(File,&(Array->Signature1),sizeof(Array->Signature1)) )
		return JE_FALSE;

	Version = JE_ARRAY_VERSION;

	if ( ! jeVFile_Write(File,&Version,sizeof(Version)) )
		return JE_FALSE;

	if ( ! jeVFile_Write(File,&(Array->HunkLength),sizeof(Array->HunkLength)) )
		return JE_FALSE;

	if ( ! jeVFile_Write(File,&(Array->MaxIndex),sizeof(Array->MaxIndex)) )
		return JE_FALSE;

	Log_Printf("Array count %d : Elt size %d\n", Array->MaxIndex, Array->HunkLength);

	// we should lock a critical section on the array right here ; we assume that
	//	after the sort it stays sorted

	// Since memblocs are put on the front of the list, reverse them so indexs will be right
	jeArray_SortMemBlocks((jeArray *)Array);

	Index = 0;

	for(MB = Array->MemList;MB;MB=MB->Next)
	{
		Element *E;
		int		i;
		uint8	UseFlag;

		assert( MB->BaseIndex == Index );

		for(i=0;i<MB->NumItems && Index <= Array->MaxIndex;i++)
		{
			E = (Element *)(MB->Memory + i * Array->ElementLength);

			UseFlag = E->IsInUse ? 0xFF : 0;

			if ( ! jeVFile_Write(File,&UseFlag,sizeof(UseFlag)) )
				return JE_FALSE;

			if ( UseFlag )
			{
				if ( ! ElementWriter(File,E->Data,WriterContext) )
					return JE_FALSE;
			}

			Index ++;
		}
	}

	assert( Index <= (Array->MaxIndex + 1) );

	for( ;Index <= Array->MaxIndex ; Index++ )
	{
		uint8 UseFlag = 0;

		if ( ! jeVFile_Write(File,&UseFlag,sizeof(UseFlag)) )
			return JE_FALSE;
	}

	// Put them back into original order
	jeArray_SortMemBlocks((jeArray *)Array);

	return JE_TRUE;
}

/*}{************************/

// this is the do-nothing implementation of defragmenting :^)

struct jeArray_Defragmenter
{
	int nada;
};

JETAPI jeArray_Defragmenter * JETCC jeArray_DefragmentStart(jeArray * Array)
{
	static jeArray_Defragmenter Hack;
	// <>
	return &Hack;
}

JETAPI jeArray_Index JETCC jeArray_Defragment(jeArray_Defragmenter * D, jeArray_Index I)
{
	// <>
	return I;
}

JETAPI void JETCC jeArray_DefragmentEnd(jeArray_Defragmenter * D)
{
	// <>
	return;
}

/*}{************************/

jeBoolean jeArray_IsValid(const jeArray * Array)
{
	if ( ! Array ) return JE_FALSE;
	if ( Array->Signature1 != jeArray_Signature ) return JE_FALSE;
	if ( Array->Signature2 != jeArray_Signature ) return JE_FALSE;
	if ( Array->NumFreedElements < 0 || Array->AutoExtendNumItems < 0
		|| Array->MaxNumFreedElements < 0 || Array->HunkLength < 0 )
		return JE_FALSE;
	if ( Array->NumFreedElements > Array->MaxNumFreedElements ) return JE_FALSE;

	if ( Array->MaxIndex > Array->IndexTableLen ) return JE_FALSE;

	return JE_TRUE;
}

/*}{************************/

