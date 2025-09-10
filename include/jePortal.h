/****************************************************************************************/
/*  JEPORTAL.H                                                                          */
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

#ifndef JEPORTAL_H
#define JEPORTAL_H

#include "jePoly.h"
#include "jePlane.h"
#include "jeFrustum.h"
#include "Camera.h"
#include "jeProperty.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct	jePortal			jePortal;

typedef jeBoolean JETCC jePortal_RenderFunc(jePortal *Portal, const jePlane *Plane, const jeXForm3d *FaceXForm, void *Context, jeCamera *Camera, jeFrustum *Frustum);

typedef struct jePortal
{
	int32				RefCount;

	jeXForm3d			XForm;
	int32				Recursion;
	jePortal_RenderFunc	*RenderFunc;
} jePortal;

JETAPI jePortal	* JETCC jePortal_Create();
JETAPI jeBoolean	JETCC jePortal_CreateRef(jePortal *Portal);
JETAPI void			JETCC jePortal_Destroy(jePortal **Portal);
JETAPI jeBoolean	JETCC jePortal_IsValid(const jePortal *Portal);

#ifdef __cplusplus
}
#endif

#endif
