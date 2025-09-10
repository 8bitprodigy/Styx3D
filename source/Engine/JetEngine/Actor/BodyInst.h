/****************************************************************************************/
/*  BODYINST.H                                                                          */
/*                                                                                      */
/*  Author: Mike Sandige	                                                            */
/*  Description: Actor body instance interface.		                                    */
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
#ifndef JE_BODYINST_H
#define JE_BODYINST_H 

/* This object is for accessing and retrieving an 'instance' of the geometry
   for a body.  
   
   The retrieval is a list of drawing commands in world space or 
   in camera space.  

   An array of transforms that corresponds to the bones in the body is needed.
*/


#include "BaseType.h"
#include "Xform3d.h"
#include "Body.h"
#include "XFArray.h"
#include "Camera.h"


#ifdef __cplusplus
extern "C" {
#endif


typedef struct jeBodyInst jeBodyInst;

typedef int16 jeBodyInst_Index;

typedef enum 
{
	JE_BODYINST_FACE_TRIANGLE,
	JE_BODYINST_FACE_TRISTRIP,
	JE_BODYINST_FACE_TRIFAN
} jeBodyInst_FaceType;


typedef struct jeBodyInst_SkinVertex
{
	jeVec3d SVPoint;
	// added unxformed body skin vert member to structure for uv mapping
	jeVec3d SVW; // world-space (unxformed, unprojected) point
	jeFloat SVU,SVV;
	int	ReferenceBoneIndex;
} jeBodyInst_SkinVertex;

typedef struct jeBodyInst_Geometry 
{
	jeBodyInst_Index		 SkinVertexCount;
	jeBodyInst_SkinVertex	*SkinVertexArray;

	jeBodyInst_Index		 NormalCount;
	jeVec3d					*NormalArray;

	jeBodyInst_Index		 FaceCount;
	int32					 FaceListSize;
	jeBodyInst_Index		*FaceList;

	jeVec3d					 Maxs, Mins;
}	jeBodyInst_Geometry;

/* format for jeBodyInst_Geometry.FaceList:
	primitive type (JE_BODY_FACE_TRIANGLE,	  JE_BODY_FACE_TRISTRIP,  JE_BODY_FACE_TRIFAN )
	followed by material index
	followed by...
	case primitive 
		JE_BODY_FACE_TRIANGLE:
		  vertex index 1, normal index 1
		  vertex index 2, normal index 2
		  vertex index 3, normal index 3
		  (next primitive)
		JE_BODY_FACE_TRISTRIP:
		  triangle count
		  vertex index 1, normal index 1
		  vertex index 2, normal index 2
		  vertex index 3, normal index 3
		  vertex index 4, normal index 4
		  ...  # vertices is triangle count+2
		  (next primitive)
		JE_BODY_FACE_TRIFAN:
		  triangle count
		  vertex index 1, normal index 1
		  vertex index 2, normal index 2
		  vertex index 3, normal index 3
		  vertex index 4, normal index 4
		  ...  # vertices is triangle count+2
		  (next primitive)
*/




jeBodyInst *JETCF jeBodyInst_Create( const jeBody *B );
void JETCF jeBodyInst_Destroy(jeBodyInst **BI);

const jeBodyInst_Geometry * JETCF jeBodyInst_GetGeometry( 
								const jeBodyInst *BI,
								const jeVec3d *Scale,
								const jeXFArray *BoneXformArray,
								int LevelOfDetail,
								const jeCamera *Camera);


#ifdef __cplusplus
}
#endif

#endif
