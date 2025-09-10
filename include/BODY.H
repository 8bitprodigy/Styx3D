/****************************************************************************************/
/*  BODY.H                                                                              */
/*                                                                                      */
/*  Author: Mike Sandige	                                                            */
/*  Description: Actor body interface.		                                            */
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
#ifndef JE_BODY_H
#define JE_BODY_H 

/* This object is for managing the data associated with a skeletal-based mesh, 
   a 'body'.
   This object holds the geometry for the body, the list of materials needed, 
*/

#include "BaseType.h"
#include "Vec3d.h"
#include "Xform3d.h"
#include "VFile.h"
#include "Bitmap.h"
#include "UVMap.h"

#include "jeMaterial.h"

#ifdef __cplusplus
extern "C" {
#endif

#define JE_BODY_NUMBER_OF_LOD			(16)		// 0 is highest detail (this gives 16 levels of LOD)
#define JE_BODY_NO_PARENT_BONE         (-1)   
#define JE_BODY_HIGHEST_LOD            (0)

#define JE_BODY_ROOT					(-1)	// for specifying 'root' bounding box.

enum
{
	JE_BODY_OPTIMIZE_FLAGS_NONE				= 1 << 0,
	JE_BODY_OPTIMIZE_FLAGS_VERTS			= 1 << 1,
	JE_BODY_OPTIMIZE_FLAGS_NORMALS			= 1 << 2,
	JE_BODY_OPTIMIZE_FLAGS_SORT_VERTS		= 1 << 3,		
	JE_BODY_OPTIMIZE_FLAGS_SORT_FACES		= 1 << 4
};

typedef struct jeBody jeBody;


JETAPI jeBody *JETCC jeBody_Create(void);

JETAPI void JETCC jeBody_Destroy(jeBody **B);

JETAPI jeBoolean JETCC jeBody_IsValid(const jeBody *B);

JETAPI jeBoolean JETCC jeBody_SetOptimizeFlags(jeBody* pBody, uint32 flags);
JETAPI jeBoolean JETCC jeBody_Optimize(jeBody *pBody);
JETAPI jeBoolean JETCC jeBody_GetGeometryStats(const jeBody *B, int lod, int *Vertices, int *Faces, int *Normals);

JETAPI int16 JETCC jeBody_GetBlendDataCount(const jeBody* pBody);

JETAPI jeBoolean JETCC jeBody_AddFace(	jeBody *B,
								const jeVec3d *Vertex1, const jeVec3d *Normal1, 
								jeFloat U1, jeFloat V1, int BoneIndex1,
								const jeVec3d *Vertex2, const jeVec3d *Normal2, 
									jeFloat U2, jeFloat V2, int BoneIndex2,
								const jeVec3d *Vertex3, const jeVec3d *Normal3, 
									jeFloat U3, jeFloat V3, int BoneIndex3,
								int MaterialIndex);

// If each set of verts was only a single vertex (100% weight), jeBody_AddFaceWeightedVerts
// acts like jeBody_AddFace in a much less efficient manner.
JETAPI jeBoolean JETCC jeBody_AddFaceWeightedVerts(	jeBody* pBody,
								const jeVec3d* pVerts1, const jeVec3d* pNormal1, 
									jeFloat u1, jeFloat v1, const int* pBoneIndexes1, 
									const jeFloat* pVertWeights1, int NumVerts1,
								const jeVec3d* pVerts2, const jeVec3d* pNormal2, 
									jeFloat u2, jeFloat v2, const int* pBoneIndexes2, 
									const jeFloat* pVertWeights2, int NumVerts2,
								const jeVec3d* pVerts3, const jeVec3d* pNormal3, 
									jeFloat u3, jeFloat v3, const int* pBoneIndexes3, 
									const jeFloat* pVertWeights3, int NumVerts3,
								int materialIndex);

			// Bitmap is added to body.  It's reference count is increased.  Caller still owns a pointer
			// to the bitmap, and is responsible for destroying it.
JETAPI jeBoolean JETCC jeBody_AddMaterial( jeBody *B, 
									const char *MaterialName, 
									jeMaterialSpec *Material,
									jeFloat Red, 
									jeFloat Green, 
									jeFloat Blue,
									jeUVMapper pMapper,
									int *MaterialIndex);

			// returned bitmap is a pointer to the bitmap in the body's list.  It may not be destroyed.
			// if caller would like to 'own' a copy of that bitmap pointer, it should call jeBitmap_CreateRef()
JETAPI jeBoolean JETCC jeBody_GetMaterial(const jeBody *Body, int MaterialIndex,
										const char **MaterialName,
										jeMaterialSpec **Material, jeFloat *Red, jeFloat *Green, jeFloat *Blue,
										jeUVMapper * pMapper);

			// Bitmap is set into the body.  It's reference count is increased.  Caller still owns a pointer
			// to the bitmap, and is responsible for destroying it.
JETAPI jeBoolean JETCC jeBody_SetMaterial(jeBody *Body, int MaterialIndex,
										jeMaterialSpec *Material,  jeFloat Red,  jeFloat Green,  jeFloat Blue, jeUVMapper Mapper);

JETAPI int JETCC jeBody_GetMaterialCount(const jeBody *B);

JETAPI jeBoolean JETCC jeBody_AddBone( jeBody *B, 
							int ParentBoneIndex,
							const char *BoneName, 
							const jeXForm3d *AttachmentMatrix,
							int *BoneIndex);

JETAPI jeBoolean JETCC jeBody_ComputeLevelsOfDetail( jeBody *B ,int Levels);

JETAPI int JETCC jeBody_GetBoneCount(const jeBody *B);

JETAPI void JETCC jeBody_GetBone(	const jeBody *B, 
						int BoneIndex, 
						const char **BoneName,
						jeXForm3d *Attachment, 
						int *ParentBoneIndex);

JETAPI int32 JETCC jeBody_GetBoneNameChecksum(const jeBody *B);

JETAPI void JETCC jeBody_SetBoundingBox( jeBody *B,
							int BoneIndex,		// JE_BODY_ROOT for specifing 'root' bounding box.
							const jeVec3d *MinimumBoxCorner,
							const jeVec3d *MaximumBoxCorner);
 

JETAPI jeBoolean JETCC jeBody_GetBoundingBox( const jeBody *B, 
							int BoneIndex,		// JE_BODY_ROOT for specifing 'root' bounding box.
							jeVec3d *MinimumBoxCorner,
							jeVec3d *MaximumBoxCorner);

JETAPI jeBoolean JETCC jeBody_GetBoneByName(const jeBody* B,
	const char* BoneName,
	int* pBoneIndex,
	jeXForm3d* Attachment,
	int* pParentBoneIndex);

JETAPI jeBoolean JETCC jeBody_WriteToFile(const jeBody *B, jeVFile *pFile);
JETAPI jeBody  *JETCC  jeBody_CreateFromFile(jeVFile *pFile);

JETAPI int JETCC jeBody_GetIndexedBoneVertexCount(const jeBody* pBody, int boneIndex);
JETAPI int JETCC jeBody_GetNamedBoneVertexCount(const jeBody* pBody, const char* pBoneName);

// use GetxxBoneVertexCount() functions to obtain aSize, the size of the array of vertices prior to
// calling these functions
JETAPI jeBoolean JETCC jeBody_GetIndexedBoneVertexLocations(const jeBody* pBody, int boneIndex, int aSize,
	jeVec3d* pVerts);
JETAPI jeBoolean JETCC jeBody_GetNamedBoneVertexLocations(const jeBody* pBody, const char* pBoneName, int aSize,
	jeVec3d* pVerts);

#ifdef __cplusplus
}
#endif

#endif
