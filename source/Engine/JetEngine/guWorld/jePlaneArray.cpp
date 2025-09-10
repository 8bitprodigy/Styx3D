/****************************************************************************************/
/*  JEPLANEARRAY.C                                                                      */
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
#include <math.h>
#include <memory.h>

// Public Dependents
#include "jePlaneArray.h"

// Private Dependents
#include "Ram.h"
#include "Errorlog.h"

#define COMPOSE_INDEX(i, s) (((jePlaneArray_Index)i<<1)|(jePlaneArray_Index)s)
#define PLANENUM_FROM_INDEX(i) (i>>1)
#define PLANESIDE_FROM_INDEX(i) (i&1)

#define ZeroMem(a) memset(a, 0, sizeof(*a))
#define ZeroMemArray(a, s) memset(a, 0, sizeof(*a)*s)

#define JE_PLANEARRAY_MAX_REFCOUNT			65535
#define JE_PLANEARRAY_MAX_PLANE_REFCOUNT	65535

//=======================================================================================
//=======================================================================================
typedef struct jePlaneArray_Plane
{
	uint16					RefCount;		// Max of 65535 shared refs!!!
	jePlane					Plane;
} jePlaneArray_Plane;

typedef struct jePlaneArray
{
	uint16				RefCount;

	jePlaneArray_Index	ActivePlanes;		// Total active Planes
	jePlaneArray_Index	MaxPlanes;			// Plane array size
	jePlaneArray_Plane	*Planes;			// Array of Planes

	jePlaneArray_Plane	*LastPlane;			//

#ifdef _DEBUG
	jePlaneArray		*Self;
#endif
} jePlaneArray;

static jePlaneArray_Plane *jePlaneArray_Extend(jePlaneArray *Array);

//=======================================================================================
//	jePlaneArray_Create
//=======================================================================================
jePlaneArray *jePlaneArray_Create(int32 StartPlanes)
{
	jePlaneArray		*Array;

	assert(StartPlanes < JE_PLANEARRAY_MAX_PLANES);		// This must be true

	Array = JE_RAM_ALLOCATE_STRUCT(jePlaneArray);

	if (! Array)	// Assume not enough ram
		return NULL;

	// Clear mem for array
	ZeroMem(Array);

	// Now, create the Planes
	Array->Planes = JE_RAM_ALLOCATE_ARRAY(jePlaneArray_Plane, StartPlanes);

	if (!Array->Planes)
		goto ExitWithError;

	// Clear the verts in this vert Link
	ZeroMemArray(Array->Planes, StartPlanes);

	// Store the number of verts that the first link in the Link has
	Array->MaxPlanes = (jePlaneArray_Index)StartPlanes;
	Array->ActivePlanes = 0;
	Array->LastPlane = Array->Planes;

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
			if (Array->Planes)
				jeRam_Free(Array->Planes);
			jeRam_Free(Array);
		}

		return NULL;
	}
}

//=======================================================================================
//	jePlaneArray_Destroy
//=======================================================================================
void jePlaneArray_Destroy(jePlaneArray **Array)
{
	assert(Array);
	assert(*Array);
	assert((*Array)->RefCount > 0);

	(*Array)->RefCount--;

	if ((*Array)->RefCount == 0)			// Don't destroy until refcount == 0
	{
		if ((*Array)->Planes)				// Free the planes
		{
			assert((*Array)->MaxPlanes > 0);
			jeRam_Free((*Array)->Planes);
		}
		else
		{
			assert((*Array)->MaxPlanes == 0);
		}
	
		jeRam_Free(*Array);				// Finally, free the VArray itself
	}

	*Array = NULL;
}

//======================================================================================
//	jePlaneArray_IsValid
//=======================================================================================
jeBoolean jePlaneArray_IsValid(const jePlaneArray *Array)
{
	if (!Array)
		return JE_FALSE;

#ifdef _DEBUG
	if (Array->Self != Array)
		return JE_FALSE;
#endif
	return JE_TRUE;
}

//=======================================================================================
//	jePlaneArray_Extend
//=======================================================================================
jePlaneArray_Plane *jePlaneArray_Extend(jePlaneArray *Array)
{
	int32				NewSize;
	jePlaneArray_Plane	*v;

	NewSize = Array->MaxPlanes<<1;

	if (NewSize > JE_PLANEARRAY_MAX_PLANES)
	{
		NewSize = JE_PLANEARRAY_MAX_PLANES;

		if (NewSize <= (int32)Array->MaxPlanes)	// Make sure it grows past original size!
		{
			assert(0);							// No more verts available
			return NULL;
		}
	}
	
	Array->Planes = (jePlaneArray_Plane *)jeRam_Realloc(Array->Planes, NewSize*sizeof(jePlaneArray_Plane));

	assert(Array->Planes);
	if (!Array->Planes)
		return NULL;

	v = &Array->Planes[Array->MaxPlanes];				// Get the first vert in the new space

	// Clear new memory allocated...
	memset(v, 0, sizeof(jePlaneArray_Plane)*(NewSize-Array->MaxPlanes));

	Array->MaxPlanes = (jePlaneArray_Index)NewSize;	// Get the new number of max verts

	return v;										// Return the first vert in the new space
}

#define	DIST_EPSILON		0.01f
#define	ANGLE_EPSILON		0.00001f
#define NORMAL_EPSILON		0.00001f

//====================================================================================
//	SnapVector
//====================================================================================
static void SnapVector(jeVec3d *Normal)
{
	int		i;

	for (i=0 ; i<3 ; i++)
	{
		if ( fabs(jeVec3d_GetElement(Normal,i) - 1.0f) < ANGLE_EPSILON )
		{
			jeVec3d_Clear(Normal);
			jeVec3d_SetElement(Normal,i, 1.0f);
			break;
		}

		if ( fabs(jeVec3d_GetElement(Normal,i) - -1.0f) < ANGLE_EPSILON )
		{
			jeVec3d_Clear(Normal);
			jeVec3d_SetElement(Normal,i, -1.0f);
			break;
		}
	}
}

//====================================================================================
//	Rint (round up int)
//====================================================================================
static float RInt(float In)
{
	return (float)floor(In + 0.5f);
}

//====================================================================================
//	SnapPlane
//====================================================================================
static void SnapPlane(jePlane *Plane)
{
	SnapVector(&Plane->Normal);

	if (fabs(Plane->Dist-RInt(Plane->Dist)) < DIST_EPSILON)
		Plane->Dist = RInt(Plane->Dist);
}

//====================================================================================
//	SidePlane
//====================================================================================
static jeBoolean SidePlane(jePlane *Plane)
{
	jePlane_Type	Type;

	Plane->Type = jePlane_TypeFromUnitVector(&Plane->Normal);

	// Get the index into the major axis
	Type = (jePlane_Type)((int)Plane->Type % 3);

	// If the major axis is negated, flip plane, so the dominent axis is positive
	if (jeVec3d_GetElement(&Plane->Normal, Type) < 0.0f)
	{
		jePlane_Inverse(Plane);
		return JE_TRUE;				// Plane was sided
	}

	return JE_FALSE;
}

//=======================================================================================
//	jePlaneArray_AddPlane
//=======================================================================================
static jePlaneArray_Index jePlaneArray_AddPlane(jePlaneArray *Array, const jePlane *Plane)
{
	jePlaneArray_Plane	*p;

	assert(jePlaneArray_IsValid(Array) == JE_TRUE);
	assert(Plane);

	if (Array->ActivePlanes >= Array->MaxPlanes)
	{
		p = jePlaneArray_Extend(Array);

		if (!p)
			return JE_PLANEARRAY_NULL_INDEX;
	}
	else 
	{
		int32				i;
		jePlaneArray_Plane	*PEnd;

		PEnd = &Array->Planes[Array->MaxPlanes];

		for (p = Array->LastPlane, i=0; i<(int32)Array->MaxPlanes; i++, p++)
		{
			if (p == PEnd)				// Wrap
				p = Array->Planes;

			if (!p->RefCount)
				break;
		}
	}

	assert(p->RefCount == 0);

	p->Plane = *Plane;
	p->RefCount++;

	Array->ActivePlanes++;

	Array->LastPlane = p;
	Array->LastPlane++;

	return (jePlaneArray_Index)((p-Array->Planes));
}

//=======================================================================================
//	jePlaneArray_SharePlane
//=======================================================================================
jePlaneArray_Index jePlaneArray_SharePlane(jePlaneArray *Array, const jePlane *Plane)
{
	jePlaneArray_Plane	*pPlane;
	jePlane				Plane2;
	int32				i;
	jeBoolean			Sided;
	jePlaneArray_Index	Index;


	Plane2 = *Plane;	// Preserve plane

	SnapPlane(&Plane2);
	
	// Flip plane if needed...
	if (SidePlane(&Plane2))
		Sided = 1;
	else
		Sided = 0;

	for (pPlane = Array->Planes, i=0; i< (int32)Array->MaxPlanes; i++, pPlane++)
	{
		if (pPlane->RefCount == 0)		// Planes must share!!!!
			continue;

		if (jePlane_Compare(&pPlane->Plane, &Plane2, NORMAL_EPSILON, DIST_EPSILON))
		{
			assert(pPlane->RefCount < JE_PLANEARRAY_MAX_PLANE_REFCOUNT);
			pPlane->RefCount++;
			return COMPOSE_INDEX(i, Sided);		// Got it...
		}
	}

	Index = jePlaneArray_AddPlane(Array, &Plane2);

	return COMPOSE_INDEX(Index, Sided);
}

//=======================================================================================
//	jePlaneArray_RefPlaneByIndex
//=======================================================================================
jeBoolean jePlaneArray_RefPlaneByIndex(jePlaneArray *Array, jePlaneArray_Index Index)
{
	jePlaneArray_Plane	*Plane2;

	assert(jePlaneArray_IsValid(Array) == JE_TRUE);
	assert(Index != JE_PLANEARRAY_NULL_INDEX);

	Index = PLANENUM_FROM_INDEX(Index);

	assert(Index >= 0 && Index < JE_PLANEARRAY_MAX_PLANES);
	assert(Index >= 0 && Index < Array->MaxPlanes);

	Plane2 = &Array->Planes[Index];	
	assert(Plane2->RefCount > 0);
	assert(Plane2->RefCount < JE_PLANEARRAY_MAX_PLANE_REFCOUNT);

	if (Plane2->RefCount >= JE_PLANEARRAY_MAX_PLANE_REFCOUNT)
		return JE_FALSE;

	Plane2->RefCount++;

	return JE_TRUE;
}

//=======================================================================================
//	jePlaneArray_RemovePlane
//=======================================================================================
void jePlaneArray_RemovePlane(jePlaneArray *Array, jePlaneArray_Index *Index)
{
	jePlaneArray_Plane	*Plane;

	assert(jePlaneArray_IsValid(Array) == JE_TRUE);
	assert(*Index != JE_PLANEARRAY_NULL_INDEX);
	assert(Array->ActivePlanes > 0);

	Plane = &Array->Planes[PLANENUM_FROM_INDEX(*Index)];

	assert(Plane->RefCount > 0);

	Plane->RefCount--;

	if (Plane->RefCount == 0)
		Array->ActivePlanes--;

	*Index = JE_PLANEARRAY_NULL_INDEX;		// They should not use this plane again
}

//=======================================================================================
//	jePlaneArray_GetPlaneByIndex
//=======================================================================================
const jePlane * PLANE_CC jePlaneArray_GetPlaneByIndex(const jePlaneArray *Array, jePlaneArray_Index Index)
{
	jePlaneArray_Plane	*Plane2;

	assert(jePlaneArray_IsValid(Array) == JE_TRUE);
	assert(Index != JE_PLANEARRAY_NULL_INDEX);

	Index = PLANENUM_FROM_INDEX(Index);

	assert(Index >= 0 && Index < JE_PLANEARRAY_MAX_PLANES);
	assert(Index >= 0 && Index < Array->MaxPlanes);

	Plane2 = &Array->Planes[Index];	
	assert(Plane2->RefCount > 0);

	return &Plane2->Plane;
}
