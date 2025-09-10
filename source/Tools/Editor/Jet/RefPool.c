/****************************************************************************************/
/*  REFPOOL.C                                                                           */
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
#include	<assert.h>
#include	<stdlib.h>
#include	<string.h>

#include	"ram.h"

#include	"RefPool.h"

typedef	struct	RPFreeList
{
	struct	RPFreeList *	Next;
}	RPFreeList;

typedef	struct	RPBlock
{
	void *				Data;
	void *				DataEnd;
	struct RPBlock *	Next;
}	RPBlock;

typedef	struct	RefPool
{
	RPFreeList *	FreeList;
	RPBlock *		Blocks;
	int				Increment;
	int				Count;
}	RefPool;

RefPool *	JETCC RefPool_Create(int RefPoolIncrement)
{
	RefPool *	Pool;

	Pool = jeRam_Allocate(sizeof(*Pool));
	if	(Pool)
	{
		memset(Pool, 0, sizeof(*Pool));
		Pool->Increment = RefPoolIncrement;
	}

	return Pool;
}

void	JETCC RefPool_Destroy(RefPool **pPool)
{
	RefPool *	Pool;
	RPBlock *	Blocks;

	assert(pPool);
	assert(*pPool);

	Pool = *pPool;

	Blocks = Pool->Blocks;
	while	(Blocks)
	{
		RPBlock *	Temp;
		assert(Blocks->Data);
		jeRam_Free(Blocks->Data);
		Temp = Blocks;
		Blocks = Blocks->Next;
		jeRam_Free(Temp);
	}

	jeRam_Free(Pool);

	*pPool = NULL;
}

static	jeBoolean	JETCC ExpandPool(RefPool *Pool)
{
	RPBlock *		NewBlock;
	char *			p;
	char *			End;
	RPFreeList *	FreeHead;
	RPFreeList *	Free;

	NewBlock = jeRam_Allocate(sizeof(*NewBlock));
	if	(!NewBlock)
		return JE_FALSE;

	assert(sizeof(RPFreeList) == sizeof(void *));

	NewBlock->Data = jeRam_Allocate(Pool->Increment * sizeof(void*));
	if	(!NewBlock->Data)
	{
		jeRam_Free(NewBlock);
		return JE_FALSE;
	}

	p = NewBlock->Data;
	End = p + (Pool->Increment * sizeof(void*));
	NewBlock->DataEnd = End;
	FreeHead = Pool->FreeList;
	while	(p < End)
	{
		Free = (RPFreeList *)p;
		Free->Next = FreeHead;
		FreeHead = Free;
		p += sizeof(RPFreeList);
	}

	Pool->FreeList = FreeHead;
	NewBlock->Next = Pool->Blocks;
	Pool->Blocks = NewBlock;

	return JE_TRUE;
}

void ** JETCC RefPool_RefCreate(RefPool *Pool)
{
	void **	Result;

	assert(Pool);

	if	(!Pool->FreeList)
		if	(ExpandPool(Pool) == JE_FALSE)
			return NULL;

	assert(Pool->FreeList);
	Result = (void **)Pool->FreeList;
	Pool->FreeList = Pool->FreeList->Next;
	Pool->Count++;
	return Result;
}

void JETCC RefPool_RefDestroy(RefPool *Pool, void ***pRef)
{
	RPFreeList *	Free;

	assert(Pool);
	assert(pRef);
	assert(*pRef);

	Free = (RPFreeList *)*pRef;
	Free->Next = Pool->FreeList;
	Pool->FreeList = Free;

	Pool->Count--;

	*pRef = NULL;
}

int JETCC RefPool_GetRefCount(const RefPool *Pool)
{
	return Pool->Count;
}

static	jeBoolean	JETCC RefPool_ContainsPtr(const RefPool *Pool, const void *Ptr)
{
	RPBlock *	Blocks;

	Blocks = Pool->Blocks;
	while	(Blocks)
	{
		if	(((char *)Ptr) >= (char *)Blocks->Data && ((char *)Ptr) < (char *)Blocks->DataEnd)
			return JE_TRUE;
		Blocks = Blocks->Next;
	}
	return JE_FALSE;
}

static	RPBlock *	JETCC RefPool_FindBlock(const RefPool *Pool, const void **Ref)
{
	RPBlock *	Blocks;

	Blocks = Pool->Blocks;
	while	(Blocks)
	{
		if	(((char *)Ref) >= (char *)Blocks->Data && ((char *)Ref) < (char *)Blocks->DataEnd)
			return Blocks;
		Blocks = Blocks->Next;
	}
	return NULL;
}

void ** JETCC RefPool_GetNextRef(const RefPool *Pool, const void **Ref)
{
	RPBlock *	Blocks;

	if	(Ref == NULL)
	{
		assert(Pool->Blocks);
		Ref = (const void **)Pool->Blocks->Data;
		Blocks = Pool->Blocks;
	}
	else
	{
		assert(*Ref);
		assert(RefPool_ContainsPtr(Pool, *Ref) == JE_FALSE);

		Blocks = RefPool_FindBlock(Pool, Ref);
		assert(Blocks);
		Ref++;
	}

	while	(Blocks)
	{
		while	(((char *)Ref) < (char *)Blocks->DataEnd)
		{
			if	(*Ref && !RefPool_ContainsPtr(Pool, *Ref))
				return (void **)Ref;
			Ref++;
		}
		Blocks = Blocks->Next;
		if	(Blocks)
			Ref = (const void **)Blocks->Data;
	}

	return NULL;
}

