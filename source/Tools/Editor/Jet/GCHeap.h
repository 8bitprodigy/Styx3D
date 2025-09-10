/****************************************************************************************/
/*  GCHeap.h                                                                            */
/*                                                                                      */
/*  Author: Eli Boling                                                                  */
/*  Description: Garbage collecting heap interface                                      */
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
#ifndef	GCHEAP_H
#define	GCHEAP_H

#include	"basetype.h"

#ifdef	__cplusplus
extern "C" {
#endif

typedef	struct	GCHeap	GCHeap;

typedef	void (*GCHeap_Finalizer)(void *);

GCHeap *GCHeap_Create(int ObjectSize, int ExpandCount, GCHeap_Finalizer Finalizer);
	// Create a garbage collected heap.
	//  ObjectSize is the size of the objects to allocate
	//  ExpandCount is the number of objects to grow the heap by when we
	//    fail an allocation.
	//  Finalizer is the callback function to call when we actually free an object
	//    in the sweep phase, where the object is no longer referenced.

void GCHeap_Destroy(GCHeap **Heap);
	// Destroys a GCHeap.  All memory will be tossed, regardless of mark bits.

void *GCHeap_AllocateFixed(GCHeap *Heap);
	// Allocates a fixed size object.  The object is NOT marked at this time.

void GCHeap_MarkObject(void *Object);
	// Marks an object so that it will not be collected by the sweeper.

void GCHeap_Sweep(GCHeap *Heap);
	// Sweeps the heap, looking for objects that are not marked.  These objects
	// are then added to the free list.  If the Finalizer function is non-null
	// (from GCHeap_Create), then it is called on each object that is added back
	// to the free list before the object is actually destroyed.  This function
	// clears all the mark bits.  Hence calling it twice in a row would cause ALL
	// objects in the heap to be finalized, and added back to the free list.

#ifdef	__cplusplus
}
#endif

#endif

