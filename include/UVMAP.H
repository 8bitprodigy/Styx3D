/****************************************************************************************/
/*  UVMAP.H                                                                             */
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

#ifndef UVMAP_H
#define UVMAP_H

#include "jeTypes.h"

#ifdef __cplusplus
extern "C" {
#endif

/**

	built-in function pointers:

	jeUVMap_Reflection
	jeUVMap_Refraction
	jeUVMap_Projection

**/

///////////////////////////////////////////////////////////////////////////////////////
// CB for an array mapper

typedef jeBoolean (JETCC *jeUVMapper) (const jeXForm3d* pXForm,
	jeLVertex* pVerts,const jeVec3d* pNormals, int nVerts);

typedef jeBoolean (*jeUVMapVertex) (const jeXForm3d* pXForm,
	jeVertex* pVerts,int nVerts);

///////////////////////////////////////////////////////////////////////////////////////
// functions

// these functions take an array of JE_LVertices and normals
JETAPI jeBoolean JETCC jeUVMap_Reflection(const jeXForm3d* pXForm, jeLVertex* pVerts, const jeVec3d* pNormals, int nverts);
									// xform is (transpose of) camera xform
JETAPI jeBoolean JETCC jeUVMap_Refraction(const jeXForm3d* pXForm, jeLVertex* pVerts, const jeVec3d* pNormals, int nverts);
JETAPI jeBoolean JETCC jeUVMap_Projection(const jeXForm3d* pXForm, jeLVertex* pVerts, const jeVec3d* pNormals, int nverts);

#ifdef __cplusplus
}
#endif

#endif

