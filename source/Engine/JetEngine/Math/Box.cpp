/****************************************************************************************/
/*  BOX.C                                                                               */
/*                                                                                      */
/*  Author: Jason Wood                                                                  */
/*  Description: Non-axial aligned box support                                          */
/*               This implementation may have a inaccuracy which allows the test to     */
/*               return that boxes overlap, when they are actually separated by a       */
/*               small distance.                                                        */
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

#include <stdio.h>
#include <math.h>
#include <assert.h>

#include "Box.h"


/////////////////////////////////////////////////////////////////////
// jeBox_ functions


// Box needs to know what its axes are like in world space
// this involves a simplified rotation of the Box's local
// frame axes into global coord system

static void jeBox_ComputeGlobalFrameAxes(jeBox* Box)
{
	jeBoolean isOrthonormal;

	assert(Box != NULL);

	isOrthonormal = jeXForm3d_IsOrthonormal(&(Box->Transform));
	assert(isOrthonormal);

	Box->GlobalFrameAxes[0].X = Box->Transform.AX * Box->xScale;
	Box->GlobalFrameAxes[0].Y = Box->Transform.BX * Box->xScale;
	Box->GlobalFrameAxes[0].Z = Box->Transform.CX * Box->xScale;

	Box->GlobalFrameAxes[1].X = Box->Transform.AY * Box->yScale;
	Box->GlobalFrameAxes[1].Y = Box->Transform.BY * Box->yScale;
	Box->GlobalFrameAxes[1].Z = Box->Transform.CY * Box->yScale;

	Box->GlobalFrameAxes[2].X = Box->Transform.AZ * Box->zScale;
	Box->GlobalFrameAxes[2].Y = Box->Transform.BZ * Box->zScale;
	Box->GlobalFrameAxes[2].Z = Box->Transform.CZ * Box->zScale;

}


// set up an Box; call when initializing an Box or when
// the Box's scale(s) change

void jeBox_Set(jeBox* Box, float xScale, float yScale, float zScale, const jeXForm3d* Transform)
{
	jeBoolean isOrthonormal;

	assert(Box != NULL);
	assert(Transform != NULL);

	isOrthonormal = jeXForm3d_IsOrthonormal(&(Box->Transform));
	assert(isOrthonormal);

	Box->xScale = xScale;
	Box->yScale = yScale;
	Box->zScale = zScale;

	jeBox_SetXForm(Box, Transform);	
}

// set an Box's Transform

void jeBox_SetXForm(jeBox* Box, const jeXForm3d* Transform)
{
	jeBoolean isOrthonormal;

	assert(Box != NULL);
	assert(Transform != NULL);

	isOrthonormal = jeXForm3d_IsOrthonormal(Transform);
	assert(isOrthonormal);

	jeXForm3d_Copy(Transform, &(Box->Transform));

	isOrthonormal = jeXForm3d_IsOrthonormal(&(Box->Transform));
	assert(isOrthonormal);

	jeXForm3d_GetTranspose(Transform, &(Box->TransformInv));

	isOrthonormal = jeXForm3d_IsOrthonormal(&(Box->TransformInv));
	assert(isOrthonormal);

	jeBox_ComputeGlobalFrameAxes(Box);
}


// test for Box overlap between 2 Boxs
// tests for overlap between B against A and then A against B

jeBoolean jeBox_DetectCollisionBetween(const jeBox* Box1, const jeBox* Box2)
{
	int i, c;
	float radius;
	const jeBox* BoxA;
	const jeBox* BoxB;
	static jeVec3d centerToCenterVector, xformedCenterToCenterVector;
	static jeVec3d inverseXFormedGlobalFrameAxes[3];
	jeBoolean isOrthonormal;

	assert(Box1 != NULL);
	assert(Box2 != NULL);

	// assert orthonormality

	isOrthonormal = jeXForm3d_IsOrthonormal(&(Box1->Transform));
	assert(isOrthonormal);

	isOrthonormal = jeXForm3d_IsOrthonormal(&(Box2->Transform));
	assert(isOrthonormal);

	// test B against A and if necessary A against B

	for (c = 0; c < 2; c ++)
	{
		if (c == 0)
		{
			BoxA = Box1;
			BoxB = Box2;
		}

		else
		{
			BoxA = Box2;
			BoxB = Box1;
		}

		// rotate B's global frame axes by the amount A was rotated to bring it
		// back into its local coord system

		for (i = 0; i < 3; i++)
		{
			jeXForm3d_Rotate(&(BoxA->TransformInv), &(BoxB->GlobalFrameAxes[i]),
				&inverseXFormedGlobalFrameAxes[i]);
		}

		// get B's translation offset from A in global coord system

		jeVec3d_Subtract(&(BoxB->Transform.Translation), &(BoxA->Transform.Translation),
			&centerToCenterVector);

		// rotate offset by the amount A was rotated to bring it
		// back into its local coord system
		
		jeXForm3d_Rotate(&(BoxA->TransformInv), &centerToCenterVector,
			&xformedCenterToCenterVector);

		xformedCenterToCenterVector.X = (jeFloat)fabs(xformedCenterToCenterVector.X);
		xformedCenterToCenterVector.Y = (jeFloat)fabs(xformedCenterToCenterVector.Y);
		xformedCenterToCenterVector.Z = (jeFloat)fabs(xformedCenterToCenterVector.Z);

		// test every radius of BoxB
		// for every global frame-axis-aligned axis of BoxA
		// to see if overlap occurred

		// test overlap in X axis

		radius = (jeFloat)(fabs(inverseXFormedGlobalFrameAxes[0].X) +
			fabs(inverseXFormedGlobalFrameAxes[1].X) +
			fabs(inverseXFormedGlobalFrameAxes[2].X));

		if ((radius + BoxA->xScale) < xformedCenterToCenterVector.X)
			return JE_FALSE;

		// test overlap in Y axis

		radius = (jeFloat)(fabs(inverseXFormedGlobalFrameAxes[0].Y) +
			fabs(inverseXFormedGlobalFrameAxes[1].Y) +
			fabs(inverseXFormedGlobalFrameAxes[2].Y));

		if ((radius + BoxA->yScale) < xformedCenterToCenterVector.Y)
			return JE_FALSE;

		// test overlap in Z axis

		radius = (jeFloat)(fabs(inverseXFormedGlobalFrameAxes[0].Z) +
			fabs(inverseXFormedGlobalFrameAxes[1].Z) +
			fabs(inverseXFormedGlobalFrameAxes[2].Z));

		if ((radius + BoxA->zScale) < xformedCenterToCenterVector.Z)
			return JE_FALSE;

	} // c

	return JE_TRUE; // all tests checked out, overlap occurred
}
