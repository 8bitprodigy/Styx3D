/****************************************************************************************/
/*  BOX.H                                                                               */
/*                                                                                      */
/*  Author: Jason Wood                                                                  */
/*  Description: Box is a 3D Oriented Bounding Box                                      */
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
//
// This implementation may have a inaccuracy which allows
// the test to return that boxes overlap, when they are actually
// separated by a small distance.
//

#if !defined (JE_BOX_H)
#define JE_BOX_H

#include "Vec3d.h"
#include "Xform3d.h"

typedef struct jeBox
{
	// all member variables are **PRIVATE**
	// the Box's scales along the Box's local frame axes

	float xScale, yScale, zScale;

	// the Box's local frame origin lies at (0, 0, 0) in local space
	//
	// these are the scaled Box axes in the global frame
	 
	jeVec3d GlobalFrameAxes[3];

	// the transformation that takes the Box's axes from local space
	// to global space, and its inverse

	jeXForm3d Transform, TransformInv;

}jeBox;

/////////////////////////////////////////////////////////////////////////////
// call this to set up an Box for the first time or when the Box's
// local frame axes scale(s) change
void jeBox_Set(jeBox* Box, float xScale, float yScale, float zScale, const jeXForm3d* Transform);


// call this to set the Box's transformation matrix (does not change the
// scales of the Box's local frame axes)
void jeBox_SetXForm(jeBox* Box, const jeXForm3d* Transform);


// returns JE_TRUE if the boxes overlap, JE_FALSE otherwise
jeBoolean jeBox_DetectCollisionBetween(const jeBox* Box1, const jeBox* Box2);

#endif
