/****************************************************************************************/
/*  BODY._H                                                                             */
/*                                                                                      */
/*  Author: Mike Sandige	                                                            */
/*  Description:  Exports private BODY data structures for "friends".					*/
/*                                                                                      */
/*  The contents of this file are subject to the Genesis3D Public License               */
/*  Version 1.01 (the "License"); you may not use this file except in                   */
/*  compliance with the License. You may obtain a copy of the License at                */
/*  http://www.genesis3d.com                                                            */
/*                                                                                      */
/*  Software distributed under the License is distributed on an "AS IS"                 */
/*  basis, WITHOUT WARRANTY OF ANY KIND, either express or implied.  See                */
/*  the License for the specific language governing rights and limitations              */
/*  under the License.                                                                  */
/*                                                                                      */
/*  The Original Code is Genesis3D, released March 25, 1999.                            */
/*  Genesis3D Version 1.1 released November 15, 1999                                 */
/*  Copyright (C) 1999 WildTangent, Inc. All Rights Reserved           */
/*                                                                                      */
/****************************************************************************************/
#ifndef GE_BODY__H
#define GE_BODY__H 

#include "BaseType.h"
#include "XForm3D.h"
#include "Body.h"
#include "StrBlock.h"
#include "Bitmap.h"


#ifdef __cplusplus
extern "C" {
#endif

#define GE_BODY_INDEX_MAX (0xEFFF)

#define GE_BODY_REALLY_BIG_NUMBER (9e9f)		// bigger than any skin point

#define GE_BODY_HIGHEST_LOD_MASK	( 1 << GE_BODY_HIGHEST_LOD )
#define GE_BODY_BBOX_LOD_MASK		( 1 << GE_BODY_NUMBER_OF_LOD ) // bounding box mask


typedef int16 geBody_Index;

typedef struct geBody_XSkinVertex
{
	geVec3d			XPoint;
	geFloat			XU,XV;
	int8			LevelOfDetailMask;
	geBody_Index	BoneIndex;
} geBody_XSkinVertex;

typedef struct geBody_Normal
{
	geVec3d			Normal;
	int8			LevelOfDetailMask;
	geBody_Index	BoneIndex;
} geBody_Normal;

typedef struct geBody_Bone
{
	geVec3d			BoundingBoxMin;
	geVec3d			BoundingBoxMax;
	geXForm3d       AttachmentMatrix;
	geBody_Index	ParentBoneIndex;
} geBody_Bone;
		
typedef struct geBody_Triangle
{
	geBody_Index	VtxIndex[3];
	geBody_Index	NormalIndex[3];
	geBody_Index	MaterialIndex;
	//geBody_Index    FaceNormal;
} geBody_Triangle;
			
typedef struct geBody_TriangleList
{
	geBody_Index	  FaceCount;
	geBody_Triangle  *FaceArray;		// Sorted by MaterialIndex
} geBody_TriangleList;

typedef struct geBody_Material
{
	geBitmap *Bitmap;
	geFloat Red,Green,Blue;
} geBody_Material;

typedef struct geBody
{
	geVec3d				  BoundingBoxMin;
	geVec3d				  BoundingBoxMax;

	geBody_Index		  XSkinVertexCount;
	geBody_XSkinVertex	 *XSkinVertexArray;		// Sorted by BoneIndex

	geBody_Index		  SkinNormalCount;
	geBody_Normal		 *SkinNormalArray;

	geBody_Index		  BoneCount;
	geBody_Bone			 *BoneArray;
	geStrBlock			 *BoneNames;

	geBody_Index		  MaterialCount;
	geBody_Material		 *MaterialArray;
	geStrBlock			 *MaterialNames;		

	int					  LevelsOfDetail;
	geBody_TriangleList	  SkinFaces[GE_BODY_NUMBER_OF_LOD];
	
	geBody				 *IsValid;
} geBody;

#if defined(DEBUG) || !defined(NDEBUG)
geBoolean GENESISCC geBody_SanityCheck(const geBody *B);
#endif



#ifdef __cplusplus
}
#endif

#endif
