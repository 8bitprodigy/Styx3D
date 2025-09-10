/****************************************************************************************/
/*  JEVERTARRAY.C                                                                       */
/*                                                                                      */
/*  Author:  John Pollard                                                               */
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
#include <assert.h>
#include <stdio.h>
#include <memory.h>
#include <math.h>

// Public dependents
#include "jeVertArray.h"

// Private dependents
#include "Ram.h"
#include "Errorlog.h"

//=======================================================================================
//=======================================================================================
#define ZeroMem(a) memset(a, 0, sizeof(*a))
#define ZeroMemArray(a, s) memset(a, 0, sizeof(*a)*s)

#define VERTARRAY_MAX_REFCOUNT			(65535)
#define VERTARRAY_MAX_VERT_REFCOUNT		(255)

#define VCOMPARE_EPSILON				(0.01f)

//=======================================================================================
//=======================================================================================
typedef struct jeVertArray_Vert
{
	uint8					RefCount;	// Max of 255 shared verts in any one spot!!!
	jeVec3d					Vert;
} jeVertArray_Vert;

typedef struct jeVertArray
{
	uint16				RefCount;

	jeVertArray_Index	ActiveVerts;		// Total active verts
	jeVertArray_Index	MaxVerts;			// Vert array size
	jeVertArray_Vert	*Verts;				// Array of verts

	jeVertArray_Vert	*LastVert;			//

#ifdef _DEBUG
	jeVertArray			*Self;
#endif
} jeVertArray;

static jeVertArray_Vert *jeVertArray_Extend(jeVertArray *Array);

//=======================================================================================
//	jeVertArray_Create
//=======================================================================================
JETAPI jeVertArray * JETCC jeVertArray_Create(int32 StartVerts)
{
	jeVertArray		*VArray;

	assert(StartVerts < JE_VERTARRAY_MAX_VERTS);		// This must be true

	VArray = JE_RAM_ALLOCATE_STRUCT(jeVertArray);

	if (!VArray)	// Assume not enough ram
	{
		jeErrorLog_AddString(JE_ERR_MEMORY_RESOURCE, "jeVertArray_Create:  Out of ram for varray.", NULL);
		return NULL;
	}

	// Clear mem for varray
	ZeroMem(VArray);

	// Now, create the verts
	VArray->Verts = JE_RAM_ALLOCATE_ARRAY(jeVertArray_Vert, StartVerts);

	if (!VArray->Verts)
	{
		jeErrorLog_AddString(JE_ERR_MEMORY_RESOURCE, "jeVertArray_Create:  Out of ram for verts.", NULL);
		goto ExitWithError;
	}

	// Clear the verts in this vert Link
	ZeroMemArray(VArray->Verts, StartVerts);

	// Store the number of verts that the first link in the Link has
	VArray->MaxVerts = (jeVertArray_Index)StartVerts;
	VArray->ActiveVerts = 0;
	VArray->LastVert = VArray->Verts;

	VArray->RefCount = 1;

#ifdef _DEBUG
	VArray->Self = VArray;
#endif
   
	return VArray;			// Done

	// Error
	ExitWithError:
	{
		if (VArray)
		{
			if (VArray->Verts)
				jeRam_Free(VArray->Verts);
			jeRam_Free(VArray);
		}

		return NULL;
	}
}

//=======================================================================================
//	jeVertArray_CreateFromFile
//=======================================================================================
JETAPI jeVertArray * JETCC jeVertArray_CreateFromFile(jeVFile *VFile)
{
	jeVertArray		*Array;

	assert(VFile);

	Array = JE_RAM_ALLOCATE_STRUCT(jeVertArray);

	if (!Array)	// Assume not enough ram
	{
		jeErrorLog_AddString(JE_ERR_MEMORY_RESOURCE, "jeVertArray_CreateFromFile:  Out of ram for varray.", NULL);
		return NULL;
	}

	// Clear mem for varray
	ZeroMem(Array);

	// Read in array size
	if (!jeVFile_Read(VFile, &Array->MaxVerts, sizeof(Array->MaxVerts)))
		return NULL;
	
	// Read in number of active elements
	if (!jeVFile_Read(VFile, &Array->ActiveVerts, sizeof(Array->MaxVerts)))
		return NULL;

	// Now, create the vert array off the array size
	Array->Verts = JE_RAM_ALLOCATE_ARRAY(jeVertArray_Vert, Array->MaxVerts);

	if (!Array->Verts)
	{
		jeErrorLog_AddString(JE_ERR_MEMORY_RESOURCE, "jeVertArray_CreateFromFile:  Out of ram for verts.", NULL);
		goto ExitWithError;
	}

	// Clear the verts in this vert Link
	ZeroMemArray(Array->Verts, Array->MaxVerts);

	// Read in the array
	if (!jeVFile_Read(VFile, Array->Verts, sizeof(Array->Verts[0])*Array->MaxVerts))
		return NULL;

	// Store the number of verts that the first link in the Link has
	Array->LastVert = Array->Verts;

	Array->RefCount = 1;

#ifdef _DEBUG
	Array->Self = Array;
#endif
   
	return Array;			// Done

	// Error
	ExitWithError:
	{
		if (Array)
		{
			if (Array->Verts)
				jeRam_Free(Array->Verts);
			jeRam_Free(Array);
		}

		return NULL;
	}
}

//=======================================================================================
//	jeVertArray_WriteToFile
//=======================================================================================
JETAPI jeBoolean JETCC jeVertArray_WriteToFile(const jeVertArray *Array, jeVFile *VFile)
{
	assert(Array);
	assert(VFile);

	// Write out total size of the ENTIRE array
	if(!jeVFile_Write(VFile, &Array->MaxVerts, sizeof(Array->MaxVerts)))
		return JE_FALSE;

	// Write out number of active elements in the array
	if(!jeVFile_Write(VFile, &Array->ActiveVerts, sizeof(Array->ActiveVerts)))
		return JE_FALSE;

	// Write out the array it self
	if (!jeVFile_Write(VFile, Array->Verts, sizeof(Array->Verts[0])*Array->MaxVerts))
		return JE_FALSE;
	
	return JE_TRUE;
}

//=======================================================================================
//	jeVertArray_Destroy
//=======================================================================================
JETAPI void JETCC jeVertArray_Destroy(jeVertArray **VArray)
{
	assert(VArray);
	assert(*VArray);
	assert((*VArray)->RefCount > 0);

	(*VArray)->RefCount--;

	if ((*VArray)->RefCount == 0)		// Don't destroy until refcount == 0
	{
		if ((*VArray)->Verts)				// Free the verts 
		{
			assert((*VArray)->MaxVerts > 0);
			jeRam_Free((*VArray)->Verts);
		}
		else
		{
			assert((*VArray)->MaxVerts == 0);
		}

		jeRam_Free(*VArray);				// Finally, free the VArray itself
	}

	*VArray = NULL;
}

//=======================================================================================
//	jeVertArray_IsValid
//=======================================================================================
JETAPI jeBoolean JETCC jeVertArray_IsValid(const jeVertArray *VArray)
{
	if (!VArray)
		return JE_FALSE;

#ifdef _DEBUG
	if (VArray->Self != VArray)
		return JE_FALSE;
#endif
	return JE_TRUE;
}

//=======================================================================================
//	jeVertArray_Extend
//=======================================================================================
jeVertArray_Vert * jeVertArray_Extend(jeVertArray *Array)
{
	uint32				NewSize;
	jeVertArray_Vert	*v;

	if (Array->MaxVerts >= JE_VERTARRAY_MAX_VERTS)
		return NULL;		// No more space left	

	// At this point, there should be enough space to extend by at least one element
	NewSize = Array->MaxVerts<<1;

	if (NewSize <= Array->MaxVerts)		
		NewSize = JE_VERTARRAY_MAX_VERTS;		// Must have wrapped
	
	if (NewSize > JE_VERTARRAY_MAX_VERTS)		
		NewSize = JE_VERTARRAY_MAX_VERTS;	

	assert(NewSize > Array->MaxVerts);

	Array->Verts = (jeVertArray_Vert *)jeRam_Realloc(Array->Verts, NewSize*sizeof(jeVertArray_Vert));

	assert(Array->Verts);
	if (!Array->Verts)
		return NULL;

	v = &Array->Verts[Array->MaxVerts];				// Get the first vert in the new space

	// Clear new memory allocated...
	memset(v, 0, sizeof(jeVertArray_Vert)*(NewSize-Array->MaxVerts));

	Array->MaxVerts = (jeVertArray_Index)NewSize;	// Get the new number of max verts

	return v;										// Return the first vert in the new space
}

//=======================================================================================
//	jeVertArray_AddVert
//=======================================================================================
JETAPI jeVertArray_Index JETCC jeVertArray_AddVert(jeVertArray *Array, const jeVec3d *Vert)
{
	jeVertArray_Vert	*v;

	assert(jeVertArray_IsValid(Array) == JE_TRUE);
	assert(Vert);

	if (Array->ActiveVerts >= Array->MaxVerts)
	{
		v = jeVertArray_Extend(Array);

		if (!v)
			return JE_VERTARRAY_NULL_INDEX;
	}
	else 
	{
		int32				i;
		jeVertArray_Vert	*VEnd;

		VEnd = &Array->Verts[Array->MaxVerts];

		for (v = Array->LastVert, i=0; i<Array->MaxVerts; i++, v++)
		{
			if (v == VEnd)				// Wrap
				v = Array->Verts;

			if (!v->RefCount)
				break;
		}
	}

	assert(v->RefCount == 0);

	v->Vert = *Vert;
	v->RefCount++;

	Array->ActiveVerts++;

	Array->LastVert = v;
	Array->LastVert++;

	return (jeVertArray_Index)(v-Array->Verts);
}

//=======================================================================================
//	jeVertArray_ShareVert
//=======================================================================================
JETAPI jeVertArray_Index JETCC jeVertArray_ShareVert(jeVertArray *Array, const jeVec3d *Vert)
{
	jeVertArray_Vert	*pVert;
	int32				i;

	for (pVert = Array->Verts, i=0; i< Array->MaxVerts; i++, pVert++)
	{
		if (pVert->RefCount == 0 || pVert->RefCount >= VERTARRAY_MAX_VERT_REFCOUNT)
			continue;

		if (jeVec3d_Compare(&pVert->Vert, Vert, VCOMPARE_EPSILON))
		{
			assert(pVert->RefCount < VERTARRAY_MAX_VERT_REFCOUNT);
			pVert->RefCount++;
			return (jeVertArray_Index)i;
		}
	}

	return jeVertArray_AddVert(Array, Vert);
}

//=======================================================================================
//	jeVertArray_RemoveVert
//=======================================================================================
JETAPI void JETCC jeVertArray_RemoveVert(jeVertArray *Array, jeVertArray_Index *Index)
{
	jeVertArray_Vert	*Vert;

	assert(jeVertArray_IsValid(Array) == JE_TRUE);
	assert(*Index != JE_VERTARRAY_NULL_INDEX);
	assert(Array->ActiveVerts > 0);

	Vert = &Array->Verts[*Index];

	assert(Vert->RefCount > 0);

	Vert->RefCount--;

	if (Vert->RefCount == 0)
	{
		// Decement the global number of active verts in this array
		Array->ActiveVerts--;
	}

	*Index = JE_VERTARRAY_NULL_INDEX;		// They should not use this vert again
}

//=======================================================================================
//	jeVertArray_RemoveVert
//=======================================================================================
JETAPI jeBoolean JETCC jeVertArray_RefVertByIndex(jeVertArray *Array, jeVertArray_Index Index)
{
	jeVertArray_Vert	*Vert;

	assert(jeVertArray_IsValid(Array) == JE_TRUE);
	assert(Index != JE_VERTARRAY_NULL_INDEX);
	assert(Array->ActiveVerts > 0);

	Vert = &Array->Verts[Index];

	assert(Vert->RefCount > 0);
	assert(Vert->RefCount < VERTARRAY_MAX_VERT_REFCOUNT);
	
	if (Vert->RefCount >= VERTARRAY_MAX_VERT_REFCOUNT)
		return JE_FALSE;

	Vert->RefCount++;

	return JE_TRUE;
}

//=======================================================================================
//	jeVertArray_SetVertByIndex
//=======================================================================================
JETAPI void JETCC jeVertArray_SetVertByIndex(jeVertArray *VArray, jeVertArray_Index Index, const jeVec3d *Vert)
{
	jeVertArray_Vert	*Vert2;

	assert(jeVertArray_IsValid(VArray) == JE_TRUE);
	assert(Index >= 0 && Index < VArray->MaxVerts);
	assert(Index != JE_VERTARRAY_NULL_INDEX);

	Vert2 = &VArray->Verts[Index];
	assert(Vert2->RefCount > 0);

	Vert2->Vert = *Vert;
}

//=======================================================================================
//	jeVertArray_GetVertByIndex
//=======================================================================================
JETAPI const jeVec3d * JETCC jeVertArray_GetVertByIndex(const jeVertArray *VArray, jeVertArray_Index Index)
{
	jeVertArray_Vert	*Vert2;

	assert(jeVertArray_IsValid(VArray) == JE_TRUE);
	assert(Index >= 0 && Index < VArray->MaxVerts);
	assert(Index != JE_VERTARRAY_NULL_INDEX);

	Vert2 = &VArray->Verts[Index];	
	assert(Vert2->RefCount > 0);

	return &Vert2->Vert;
}

//=======================================================================================
//	jeVertArray_GetMaxIndex
//=======================================================================================
JETAPI int16 JETCC jeVertArray_GetMaxIndex( const jeVertArray *VArray )
{
	assert(jeVertArray_IsValid(VArray) == JE_TRUE);
	return( VArray->ActiveVerts );
}

// Each X,Y,Z Element cannot be no more than +- HASH_SIZE2 (16384)

#define	HASH_SIZE		128							// Must be power of 2
#define	HASH_SIZE2		HASH_SIZE*HASH_SIZE			// Squared(HASH_SIZE)
#define HASH_SHIFT		8							// Log2(HASH_SIZE)+1

#define HASH_ELEMENT(x)	((HASH_SIZE2 + (int32)((x) + 0.5f)) >> HASH_SHIFT)

typedef struct jeVertArray_Optimizer
{	
	jeVertArray_Index	*VertexChain;		
	jeVertArray_Index	HashVerts[HASH_SIZE2];			// A vertex number, or JE_VERTARRAY_NULL_INDEX for no verts

	jeVertArray_Index	OptimizedIndexListSize;
	jeVertArray_Index	*OptimizedIndexList;
} jeVertArray_Optimizer;

//=====================================================================================
//	GetHashKeyFromVert
//=====================================================================================
static int32 GetHashKeyFromVert(jeVec3d *Vert)
{
	int32	x, y;

	x = HASH_ELEMENT(Vert->X);
	y = HASH_ELEMENT(Vert->Z);

	assert (!( x < 0 || x >= HASH_SIZE || y < 0 || y >= HASH_SIZE ));
	
	return y*HASH_SIZE + x;
}

#define INTEGRAL_EPSILON	(0.01f)

//=====================================================================================
//	WeldVert
//=====================================================================================
static jeVertArray_Index WeldVert(jeVertArray *Array, jeVertArray_Optimizer *Optimizer, jeVertArray_Vert *Verts, int32 *NumVerts, jeVec3d *Vert)
{
	jeVertArray_Index		i;
	int32					h;

	// Snap the vert
	for (h=0; h<3; h++)
	{
		int32	FVert;

		FVert = (int32)(jeVec3d_GetElement(Vert, h)+0.5f);

		if (fabs(jeVec3d_GetElement(Vert, h) - FVert) < INTEGRAL_EPSILON)
			jeVec3d_SetElement(Vert, h, (jeFloat)FVert);
	}

	// Get the HashKey
	h = GetHashKeyFromVert(Vert);

	// Search through all the verts in this chain for a match
	for (i=Optimizer->HashVerts[h]; i != JE_VERTARRAY_NULL_INDEX; i = Optimizer->VertexChain[i])
	{
		assert(i <= (*NumVerts));

		if (jeVec3d_Compare(Vert, &Verts[i].Vert, VCOMPARE_EPSILON))
		{
			assert(Verts[i].RefCount > 0 && Verts[i].RefCount < VERTARRAY_MAX_VERT_REFCOUNT);
			Verts[i].RefCount++;

			return i;
		}
	}

	// No match, add to tail

	assert((*NumVerts) < Array->ActiveVerts);

	Verts[(*NumVerts)].Vert = *Vert;
	assert(Verts[(*NumVerts)].RefCount == 0);
	Verts[(*NumVerts)].RefCount = 1;

	Optimizer->VertexChain[(*NumVerts)] = Optimizer->HashVerts[h];
	Optimizer->HashVerts[h] = (jeVertArray_Index)(*NumVerts);

	(*NumVerts)++;

	return (jeVertArray_Index)(*NumVerts)-1;
}

//=======================================================================================
//	BuildOptimizedIndexList
//=======================================================================================
static jeBoolean BuildOptimizedIndexList(jeVertArray *Array, jeVertArray_Optimizer *Optimizer)
{
	jeVertArray_Vert	*OptimizedVerts, *pVert;
	int32				NumVerts, i;
	
	// Build a new vert array, where everything vert is welded
	OptimizedVerts = JE_RAM_ALLOCATE_ARRAY(jeVertArray_Vert, Array->ActiveVerts);

	if (!OptimizedVerts)
		return JE_FALSE;

	ZeroMemArray(OptimizedVerts, Array->ActiveVerts);

	NumVerts = 0;

	for (pVert = Array->Verts, i=0; i< Array->MaxVerts; i++, pVert++)
	{
		if (!pVert->RefCount)
			continue;		// Vert is not active, skip it

		Optimizer->OptimizedIndexList[i] = WeldVert(Array, Optimizer, OptimizedVerts, &NumVerts, &pVert->Vert);

		if (Optimizer->OptimizedIndexList[i] == JE_VERTARRAY_NULL_INDEX)
		{
			jeRam_Free(OptimizedVerts);
			return JE_FALSE;
		}
	}

	// Free old vert array
	jeRam_Free(Array->Verts);

	if (NumVerts < Array->ActiveVerts)
	{
		// Shrink the new array if it is smaller then the original # of verts
		//	(This is probably the case since we have welded them together...)
		OptimizedVerts = (jeVertArray_Vert *)jeRam_Realloc(OptimizedVerts, NumVerts*sizeof(jeVertArray_Vert));
		Array->ActiveVerts = (jeVertArray_Index)NumVerts;
	}

	// Assign new array to the welded optimized verts
	Array->Verts = OptimizedVerts;

	Array->MaxVerts = Array->ActiveVerts;
	Array->LastVert = Array->Verts;

	return JE_TRUE;
}

//=======================================================================================
//	jeVertArray_CreateOptimizer
//	An Optimizer welds all the verts together in an array REALLY fast using a hash table
//		NOTE - Once you create an optimizer from an array, you MUST convert your indexes
//		over using the jeVertArray_GetOptimizedIndex API function call before using the array again...
//=======================================================================================
JETAPI jeVertArray_Optimizer * JETCC jeVertArray_CreateOptimizer(jeVertArray *Array)
{
	jeVertArray_Optimizer	*Optimizer;
	int32					i;

	assert(jeVertArray_IsValid(Array) == JE_TRUE);

	Optimizer = JE_RAM_ALLOCATE_STRUCT(jeVertArray_Optimizer);

	if (!Optimizer)
		return NULL;
	
	ZeroMem(Optimizer);
	
	Optimizer->VertexChain = JE_RAM_ALLOCATE_ARRAY(jeVertArray_Index, Array->MaxVerts);

	if (!Optimizer->VertexChain)
		goto ExitWithError;

	Optimizer->OptimizedIndexList = JE_RAM_ALLOCATE_ARRAY(jeVertArray_Index, Array->MaxVerts);

	if (!Optimizer->OptimizedIndexList)
		goto ExitWithError;

	Optimizer->OptimizedIndexListSize = Array->MaxVerts;

	// Set the defaults
	for (i=0; i< Array->MaxVerts; i++)
	{
		Optimizer->VertexChain[i] = JE_VERTARRAY_NULL_INDEX;
		Optimizer->OptimizedIndexList[i] = JE_VERTARRAY_NULL_INDEX;
	}

	for (i=0; i< HASH_SIZE2; i++)
		Optimizer->HashVerts[i] = JE_VERTARRAY_NULL_INDEX;

	if (!BuildOptimizedIndexList(Array, Optimizer))
		goto ExitWithError;

	// At this point on, they MUST use the Optimizer to convert their indexes 
	//	over before they can use the new array

	return Optimizer;

	ExitWithError:
	{
		if (Optimizer)
		{
			if (Optimizer->VertexChain)
				jeRam_Free(Optimizer->VertexChain);

			if (Optimizer->OptimizedIndexList)
				jeRam_Free(Optimizer->OptimizedIndexList);

			jeRam_Free(Optimizer);
		}
		return NULL;
	}
}

//=======================================================================================
//	jeVertArray_DestroyOptimizer
//=======================================================================================
JETAPI void JETCC jeVertArray_DestroyOptimizer(jeVertArray *Array, jeVertArray_Optimizer **Optimizer)
{
	assert(jeVertArray_IsValid(Array) == JE_TRUE);
	assert(Optimizer);
	assert(*Optimizer);

	if ((*Optimizer)->VertexChain)
		jeRam_Free((*Optimizer)->VertexChain);

	if ((*Optimizer)->OptimizedIndexList)
		jeRam_Free((*Optimizer)->OptimizedIndexList);

	jeRam_Free(*Optimizer);
	*Optimizer = NULL;
}

//=======================================================================================
//	jeVertArray_GetOptimizedIndex
//=======================================================================================
JETAPI jeVertArray_Index JETCC jeVertArray_GetOptimizedIndex(jeVertArray *Array, jeVertArray_Optimizer *Optimizer, jeVertArray_Index Index)
{
	assert(jeVertArray_IsValid(Array) == JE_TRUE);
	assert(Optimizer);
	assert(Index != JE_VERTARRAY_NULL_INDEX);
	assert(Index >= 0 && Index < Optimizer->OptimizedIndexListSize);
	
	return Optimizer->OptimizedIndexList[Index];
}

//=======================================================================================
//	jeVertArray_GetEdgeVerts
//=======================================================================================
JETAPI jeBoolean JETCC jeVertArray_GetEdgeVerts(jeVertArray_Optimizer *Optimizer, const jeVec3d *v1, const jeVec3d *v2, jeVertArray_Index *EdgeVerts, int32 *NumEdgeVerts, int32 MaxEdgeVerts)
{
	int32				x1, y1, x2, y2;
	int32				t, x, y;

	x1 = HASH_ELEMENT(v1->X);
	y1 = HASH_ELEMENT(v1->Z);

	x2 = HASH_ELEMENT(v2->X);
	y2 = HASH_ELEMENT(v2->Z);

	if (x1 > x2)
	{
		t = x1;
		x1 = x2;
		x2 = t;
	}
	if (y1 > y2)
	{
		t = y1;
		y1 = y2;
		y2 = t;
	}

	(*NumEdgeVerts) = 0;

	for (x=x1 ; x <= x2 ; x++)
	{
		for (y=y1 ; y <= y2 ; y++)
		{
			jeVertArray_Index	Index;

			for (Index = Optimizer->HashVerts[y*HASH_SIZE+x] ; Index != JE_VERTARRAY_NULL_INDEX ; Index = Optimizer->VertexChain[Index])
			{
				if ((*NumEdgeVerts) > MaxEdgeVerts)
					return JE_FALSE;

				EdgeVerts[(*NumEdgeVerts)++] = Index;
			}
		}
	}

	return JE_TRUE;
}
