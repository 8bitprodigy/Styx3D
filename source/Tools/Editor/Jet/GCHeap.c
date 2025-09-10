/****************************************************************************************/
/*  GCHeap.c                                                                            */
/*                                                                                      */
/*  Author: Eli Boling                                                                  */
/*  Description: Garbage collecting heap implementation                                 */
/*    This is a mark and sweep implementation of a garbage collector on a heap          */
/*    that holds fixed sized objects.  It is designed for managing medium numbers of    */
/*    objects of small to medium size.                                                  */
/*                                                                                      */
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
#include	<stdio.h>
#include	<stdlib.h>
#include	<assert.h>

#include	"ram.h"
#include	"GCHeap.h"

#define	GCHEAP_OBJECT_MARKED	0x00000001
#define	GCHEAP_OBJECT_OWNED		0x00000002
#define	GCHEAP_OBJECT_FREE		0x00000004

#define GCHEAP_OBJECT_SIGNATURE	0x424f4347	/* GCOB */

typedef	struct	Block
{
	void *			Data;
//	int				Length;
	struct Block *	Next;
}	Block;

typedef	struct	ObjectHeader
{
	unsigned long		Signature;
}	ObjectHeader;

typedef	struct	HeapObject
{
	unsigned int		Flags;
	union
	{
		struct HeapObject *	NextFreeObject;
		ObjectHeader		ObjHeader;
	}	u;
}	HeapObject;

typedef	struct	GCHeap
{
	int					ObjectSize;
	int					ExpandSize;
	GCHeap_Finalizer	Finalizer;
	Block *				Blocks;
	HeapObject *		FreeObjects;
}	GCHeap;

GCHeap *GCHeap_Create(int ObjectSize, int ExpandCount, GCHeap_Finalizer Finalizer)
{
	GCHeap *	Heap;
//	int			ExpandSize;

//	ObjectSize = max(ObjectSize, sizeof(HeapObject));
	ObjectSize += sizeof(HeapObject);
	
	Heap = jeRam_Allocate(sizeof(*Heap));
	if	(!Heap)
		return Heap;

	Heap->ObjectSize = ObjectSize;
	Heap->Finalizer = Finalizer;
	Heap->Blocks = NULL;
	Heap->FreeObjects = NULL;

#pragma message ("Should really be page aligned")
	Heap->ExpandSize = ExpandCount * ObjectSize;

	return Heap;
}

void GCHeap_Destroy(GCHeap **pHeap)
{
	GCHeap *	Heap;
	Block *		Blocks;

	Heap = *pHeap;
	
	assert(pHeap);
	assert(Heap);

	Blocks = Heap->Blocks;
	while	(Blocks)
	{
		Block *	Temp;

		Temp = Blocks->Next;
		assert(Blocks->Data);
		jeRam_Free(Blocks->Data);
		jeRam_Free(Blocks);
		Blocks = Temp;
	}
	jeRam_Free(Heap);
	*pHeap = NULL;
}

static	jeBoolean GCHeap_Expand(GCHeap *Heap)
{
	Block *			NewBlock;
	char *			p;
	char *			End;
	HeapObject *	FreeHead;
	HeapObject *	Free;

	NewBlock = jeRam_Allocate(sizeof(*NewBlock));
	if	(!NewBlock)
		return JE_FALSE;

	NewBlock->Data = jeRam_Allocate(Heap->ExpandSize);
	if	(!NewBlock->Data)
	{
		jeRam_Free(NewBlock);
		return JE_FALSE;
	}
//	NewBlock->Length = ExpandSize;
	p = NewBlock->Data;
	End = p + Heap->ExpandSize;
	FreeHead = Heap->FreeObjects;
	while	(p < End)
	{
		Free = (HeapObject *)p;
		Free->u.NextFreeObject = FreeHead;
		Free->Flags = GCHEAP_OBJECT_FREE;
		FreeHead = Free;
		p += Heap->ObjectSize;
	}

	Heap->FreeObjects = FreeHead;
	NewBlock->Next = Heap->Blocks;
	Heap->Blocks = NewBlock;

	return JE_TRUE;
}

void *GCHeap_AllocateFixed(GCHeap *Heap)
{
	HeapObject *	Obj;

	assert(Heap);

	if	(!Heap->FreeObjects)
	{
		if	(GCHeap_Expand(Heap) == JE_FALSE)
			return NULL;
	}

	assert(Heap->FreeObjects);

	Obj = Heap->FreeObjects;
	Heap->FreeObjects = Heap->FreeObjects->u.NextFreeObject;
	Obj->Flags = 0;
	Obj->u.ObjHeader.Signature = GCHEAP_OBJECT_SIGNATURE;

	return (void *)(Obj + 1);
}

void GCHeap_MarkObject(void *Object)
{
	HeapObject *	HObj;

	assert(Object);

	HObj = ((HeapObject *)Object) - 1;

	assert(HObj->u.ObjHeader.Signature == GCHEAP_OBJECT_SIGNATURE);
	assert(!(HObj->Flags & GCHEAP_OBJECT_FREE));
	
	HObj->Flags |= GCHEAP_OBJECT_MARKED;
}

void GCHeap_Sweep(GCHeap *Heap)
{
	Block *	Blocks;

	assert(Heap);

	Blocks = Heap->Blocks;
	while	(Blocks)
	{
		char *	p;
		char *	End;

		assert(Blocks->Data);
		p = Blocks->Data;
		End = p + Heap->ExpandSize;
		while	(p < End)
		{
			HeapObject *	Obj;

			Obj = (HeapObject *)p;
			if	(!(Obj->Flags & (GCHEAP_OBJECT_MARKED	|
								 GCHEAP_OBJECT_OWNED	|
								 GCHEAP_OBJECT_FREE)))
			{
				if	(Heap->Finalizer)
					(Heap->Finalizer)((void *)(Obj + 1));
				Obj->Flags |= GCHEAP_OBJECT_FREE;
				Obj->u.NextFreeObject = Heap->FreeObjects;
				Heap->FreeObjects = Obj;
			}
			else
			{
				Obj->Flags &= ~GCHEAP_OBJECT_MARKED;
			}

			p += Heap->ObjectSize;
		}
		Blocks = Blocks->Next;
	}

#pragma message ("Make a pass to shrink away blocks that have been completely evacuated")
}

