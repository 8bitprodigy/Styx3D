/****************************************************************************************/
/*  JEPLANE.C                                                                           */
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

// Public Dependents
#include "jePlane.h"

// Private Dependents
#include "Errorlog.h"

//====================================================================================
//	jePlane_SetFromVerts
//====================================================================================
JETAPI void	JETCC jePlane_SetFromVerts(jePlane *Plane, const jeVec3d *V1, const jeVec3d *V2, const jeVec3d *V3)
{
	jeVec3d		Vect1, Vect2;
	
	// Get the 2 vectors to derive the normal
	jeVec3d_Subtract(V1, V2, &Vect1);
	jeVec3d_Subtract(V3, V2, &Vect2);
	
	// The normal is the cross between these 2 vectors
	jeVec3d_CrossProduct(&Vect1, &Vect2, &Plane->Normal);
	jeVec3d_Normalize(&Plane->Normal);

	// Get the planes distance from the origin, by projecting a vert on the plane
	// along the plane normal, to the origin...
	Plane->Dist = jeVec3d_DotProduct(V1, &Plane->Normal);

	// Finally, get the plane type
	Plane->Type = jePlane_TypeFromUnitVector(&Plane->Normal);
}

//=====================================================================================
//	jePlane_Inverse
//=====================================================================================
JETAPI void	JETCC jePlane_Inverse(jePlane *Plane)
{
	assert(Plane);

	jeVec3d_Inverse(&Plane->Normal);
	Plane->Dist = -Plane->Dist;
}

//====================================================================================
//	jePlane_Rotate
//====================================================================================
JETAPI void	JETCC jePlane_Rotate(const jePlane *In, const jeXForm3d *XForm, jePlane *Out)
{
	assert(In);
	assert(XForm);
	assert(Out);

	jeXForm3d_Rotate(XForm, &In->Normal, &Out->Normal);

	Out->Dist = In->Dist;
	Out->Type = Type_Any;
}

//====================================================================================
//	jePlane_Transform
//====================================================================================
JETAPI  void JETCC jePlane_Transform(const jePlane *In, const jeXForm3d *XForm, jePlane *Out)
{
	jeVec3d		PointOnPlane;

	assert(In);
	assert(XForm);
	assert(Out);

	// Put a point on the plane
	jeVec3d_Scale(&In->Normal, In->Dist, &PointOnPlane);
	// Transform the point
	jeXForm3d_Transform(XForm, &PointOnPlane, &PointOnPlane);
	// Rotate the plane
	jeXForm3d_Rotate(XForm, &In->Normal, &Out->Normal);
	// Find the Dist of the new plane by projecting the transformed point on the new plane
	Out->Dist = jeVec3d_DotProduct(&Out->Normal, &PointOnPlane);

	Out->Type = Type_Any;
}

//====================================================================================
//	jePlane_TransformRenorm
//====================================================================================
JETAPI  void JETCC jePlane_TransformRenorm(const jePlane *In, const jeXForm3d *XForm, jePlane *Out)
{
	jeVec3d		PointOnPlane;

	assert(In);
	assert(XForm);
	assert(Out);

	// Put a point on the plane
	jeVec3d_Scale(&In->Normal, In->Dist, &PointOnPlane);
	// Transform the point
	jeXForm3d_Transform(XForm, &PointOnPlane, &PointOnPlane);
	// Rotate the plane
	jeXForm3d_Rotate(XForm, &In->Normal, &Out->Normal);
	jeVec3d_Normalize(&Out->Normal); // if XForm isn't normalized, need to renorm here
	// Find the Dist of the new plane by projecting the transformed point on the new plane
	Out->Dist = jeVec3d_DotProduct(&Out->Normal, &PointOnPlane);

	Out->Type = Type_Any;
}

//=====================================================================================
//	jePlane_TypeFromUnitVector
//=====================================================================================
jePlane_Type jePlane_TypeFromUnitVector(const jeVec3d *V1)
{
	float	X, Y, Z;

	assert(V1);

	X = (float)fabs(V1->X);
	Y = (float)fabs(V1->Y);
	Z = (float)fabs(V1->Z);

	if (X == 1.0f)			
		return Type_X;			// Axial aligned on X
	else if (Y == 1.0f)			
		return Type_Y;			// Axial aligned on Y
	else if (Z == 1.0f)
		return Type_Z;			// Axial aligned on Z
	else if (X >= Y && X >= Z)	
		return Type_AnyX;		// Non Axial, X dominent axis
	else if (Y >= X && Y >= Z)
		return Type_AnyY;		// Non Axial, Y dominent axis
	else
		return Type_AnyZ;		// Non Axial, Z dominent axis
}

//=====================================================================================
//	jePlane_GetAAVectors	(Axial aligned vectors)
//=====================================================================================
JETAPI jeBoolean JETCC jePlane_GetAAVectors(const jePlane *Plane, jeVec3d *Xv, jeVec3d *Yv)
{
	int32	BestAxis;
	float	Dot, Best;
	int32	i;

	assert(Plane);
	assert(Xv);
	assert(Yv);
	
	Best = 0.0f;
	BestAxis = -1;
	
	for (i=0 ; i<3 ; i++)
	{
		Dot = (float)fabs(jeVec3d_GetElement((jeVec3d*)&Plane->Normal, i));

		if (Dot > Best)
		{
			Best = Dot;
			BestAxis = i;
		}
	}

	switch(BestAxis)
	{
		case 0:						// X
			Xv->X = 0.0f;
			Xv->Y = 0.0f;
			Xv->Z = 1.0f;

			Yv->X = 0.0f;
			Yv->Y =-1.0f;
			Yv->Z = 0.0f;
			break;
		case 1:						// Y
			Xv->X = 1.0f;
			Xv->Y = 0.0f;
			Xv->Z = 0.0f;

			Yv->X = 0.0f;
			Yv->Y = 0.0f;
			Yv->Z = 1.0f;
			break;
		case 2:						// Z
			Xv->X = 1.0f;
			Xv->Y = 0.0f;
			Xv->Z = 0.0f;

			Yv->X = 0.0f;
			Yv->Y =-1.0f;
			Yv->Z = 0.0f;
			break;

		default:
			jeErrorLog_AddString(-1,"jePlane_GetAAVectors: No Axis found.\n", "");
			return JE_FALSE;
	}

	return JE_TRUE;
}

//=====================================================================================
//	jePlane_PointDistance
//=====================================================================================
float PLANE_CC jePlane_PointDistance(const jePlane *Plane, const jeVec3d *Point)
{
   assert(Plane);
   assert(Point);

	return jeVec3d_DotProduct(Point, &Plane->Normal) - Plane->Dist;
}

//=====================================================================================
//	jePlane_PointDistanceFast
//	NOTE - This assumes the plane is facing positive!  If not, you MUST reverse the 
//		returned distance!!!
//=====================================================================================
float PLANE_CC jePlane_PointDistanceFast(const jePlane *Plane, const jeVec3d *Point)
{
   float	Dist;

   assert(Plane);
   assert(Point);

   switch (Plane->Type)
   {
	   case Type_X:
           Dist = (Point->X - Plane->Dist);
           break;
	   case Type_Y:
           Dist = (Point->Y - Plane->Dist);
           break;
	   case Type_Z:
           Dist = (Point->Z - Plane->Dist);
           break;
	      
       default:
           Dist = jeVec3d_DotProduct(Point, &Plane->Normal) - Plane->Dist;
           break;
    }

    return Dist;
}


//=======================================================================================
//	jePlane_BoxSide
//=======================================================================================
jePlane_Side jePlane_BoxSide(const jePlane *Plane, const jeExtBox *Box, jeFloat Epsilon)
{
	jePlane_Side	Side;
	int32			i;
	jeVec3d			Corners[2];
	jeFloat			Dist1, Dist2;
	jeVec3d			*Mins, *Maxs;

	assert(Plane);
	assert(Box);
		
	Mins = &((jeExtBox*)Box)->Min;
	Maxs = &((jeExtBox*)Box)->Max;

	// Axial planes are easy
	if (Plane->Type < 3)
	{
		Side = 0;

		if (jeVec3d_GetElement(Maxs, Plane->Type) > Plane->Dist+Epsilon)
			Side |= PSIDE_FRONT;
		if (jeVec3d_GetElement(Mins, Plane->Type) < Plane->Dist-Epsilon)
			Side |= PSIDE_BACK;

		return Side;
	}
	
	for (i=0 ; i<3 ; i++)
	{
		if (jeVec3d_GetElement(&((jePlane*)Plane)->Normal, i) < 0)
		{
			jeVec3d_SetElement(&Corners[0], i, jeVec3d_GetElement(Mins, i));
			jeVec3d_SetElement(&Corners[1], i, jeVec3d_GetElement(Maxs, i));
		}
		else
		{
			jeVec3d_SetElement(&Corners[1], i, jeVec3d_GetElement(Mins, i));
			jeVec3d_SetElement(&Corners[0], i, jeVec3d_GetElement(Maxs, i));
		}
	}

	Dist1 = jeVec3d_DotProduct(&Plane->Normal, &Corners[0]) - Plane->Dist;
	Dist2 = jeVec3d_DotProduct(&Plane->Normal, &Corners[1]) - Plane->Dist;

	Side = 0;

	if (Dist1 >= Epsilon)
		Side = PSIDE_FRONT;
	if (Dist2 < Epsilon)
		Side |= PSIDE_BACK;

	return Side;
}

//====================================================================================
//	jePlane_Compare
//====================================================================================
jeBoolean jePlane_Compare(const jePlane *Plane1, const jePlane *Plane2, float NEpsilon, float DEpsilon)
{
	assert(Plane1);
	assert(Plane2);

	if (fabs(Plane1->Normal.X - Plane2->Normal.X) < NEpsilon && 
		fabs(Plane1->Normal.Y - Plane2->Normal.Y) < NEpsilon && 
		fabs(Plane1->Normal.Z - Plane2->Normal.Z) < NEpsilon && 
		fabs(Plane1->Dist - Plane2->Dist) < DEpsilon)
			return JE_TRUE;

	return JE_FALSE;
}

