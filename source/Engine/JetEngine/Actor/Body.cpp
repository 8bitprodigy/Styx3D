/****************************************************************************************/
/*  BODY.C                                                                              */
/*                                                                                      */
/*  Author: Mike Sandige	                                                            */
/*  Description: Actor body implementation.                                             */
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

#include <assert.h>						//assert()
#include <math.h> 						//fabs()
#include <stdlib.h>						//qsort()

#include "Body.h"
#include "BODY._H"
#include "Ram.h"
#include "Errorlog.h"
#include "Log.h"

#include "jeResource.h"

#define MAX(aa,bb)   ( (aa)>(bb)?(aa):(bb) )
#define MIN(aa,bb)   ( (aa)<(bb)?(aa):(bb) )

// disable "Unreferenced local function has been removed" warning.
#pragma warning (disable:4505)


#if defined(DEBUG) || !defined(NDEBUG)
JETAPI jeBoolean JETCC jeBody_SanityCheck(const jeBody *B)
{
	int i,j,k;
	int Lod,FaceCount,VertexCount,NormalCount,BoneCount;
	jeBody_XSkinVertex *SV;
	jeBody_Bone *Bone;
	jeBody_Normal *N;

	Lod = B->LevelsOfDetail;
	VertexCount = B->XSkinVertexCount;
	NormalCount = B->SkinNormalCount;
	BoneCount   = B->BoneCount;

	if (B->MaterialNames == NULL )
		return JE_FALSE;
	if (B->MaterialCount != jeStrBlock_GetCount(B->MaterialNames))
		return JE_FALSE;

	if (B->BoneNames == NULL)
		return JE_FALSE;
	if (B->BoneCount != jeStrBlock_GetCount(B->BoneNames))
		return JE_FALSE;

	if ((B->XSkinVertexArray == NULL) && (B->XSkinVertexCount>0))
		return JE_FALSE;
	if ((B->SkinNormalArray == NULL) && (B->SkinNormalCount>0))
		return JE_FALSE;
	if ((B->BoneArray == NULL) && (B->BoneCount>0))
		return JE_FALSE;
	if ((B->MaterialArray == NULL) && (B->MaterialCount>0))
		return JE_FALSE;


	for (i=0; i<Lod; i++)
		{
			jeBody_Triangle *F;
			FaceCount = B->SkinFaces[i].FaceCount;
			for (j=0,F=B->SkinFaces[i].FaceArray; j<FaceCount; j++,F++)
				{
					for (k=0; k<3; k++)
						{
							if ((F->VtxIndex[k]    < 0) || (F->VtxIndex[k]    >= VertexCount  ))
								return JE_FALSE;
							if ((F->NormalIndex[k] < 0) || (F->NormalIndex[k] >= NormalCount  ))
								return JE_FALSE;
							if ((F->MaterialIndex  < 0) || (F->MaterialIndex  >= B->MaterialCount))
								return JE_FALSE;
						}
				}
		}
	for (i=0,SV = B->XSkinVertexArray; i<VertexCount; i++,SV++)
		{
			if ((SV->BoneIndex < 0) || (SV->BoneIndex >= BoneCount))
				return JE_FALSE;
		}

	for (i=0,N = B->SkinNormalArray; i<NormalCount; i++,N++)
		{
			if ((N->BoneIndex < 0) || (N->BoneIndex >= BoneCount))
				return JE_FALSE;
		}

	for (i=0,Bone = B->BoneArray; i<BoneCount; i++,Bone++)
		{
			if (Bone->ParentBoneIndex != JE_BODY_NO_PARENT_BONE)
				{
					if ((Bone->ParentBoneIndex < 0) || (Bone->ParentBoneIndex > i))
						return JE_FALSE;
				}
		}

	return JE_TRUE;
				
}
#endif


JETAPI jeBoolean JETCC jeBody_IsValid(const jeBody *B)
{
	if ( B == NULL )
		return JE_FALSE;
	if ( B -> IsValid != B )
		return JE_FALSE;
	assert( jeBody_SanityCheck(B) != JE_FALSE) ;
	return JE_TRUE;
}
	

static jeBody *JETCF jeBody_CreateNull(void)
{
	jeBody *B;
	int i;

	B = JE_RAM_ALLOCATE_STRUCT_CLEAR(jeBody);
	if ( B == NULL)
		{
			jeErrorLog_Add(JE_ERR_MEMORY_RESOURCE,"jeBody_CreateNull:  Failed to allocate space for jeBody.");
			return NULL;
		}
	B->IsValid          = NULL;
	B->XSkinVertexCount	= 0;
	B->XSkinVertexArray	= NULL;
	
	B->SkinNormalCount	= 0;
	B->SkinNormalArray	= NULL;

	B->BoneCount		= 0;
	B->BoneArray		= NULL;
	B->BoneNames		= NULL;
			
	B->MaterialCount	= 0;
	B->MaterialArray	= NULL;
	B->MaterialNames	= NULL;
	for (i=0; i<JE_BODY_NUMBER_OF_LOD; i++)
		{
			B->SkinFaces[i].FaceCount = 0;
			B->SkinFaces[i].FaceArray = NULL;
		}
	B->LevelsOfDetail = 1;

	B->optFlags = (   JE_BODY_OPTIMIZE_FLAGS_VERTS 
					| JE_BODY_OPTIMIZE_FLAGS_NORMALS 
					| JE_BODY_OPTIMIZE_FLAGS_SORT_VERTS 
					| JE_BODY_OPTIMIZE_FLAGS_SORT_FACES);

	B->IsValid = B;

	jeVec3d_Set(&(B->BoundingBoxMin),0.0f,0.0f,0.0f);
	jeVec3d_Set(&(B->BoundingBoxMax),0.0f,0.0f,0.0f);

	B->blendDataCount = 0;
	B->blendDataArray = NULL;

	return B;
}

static void JETCF jeBody_DestroyPossiblyIncompleteBody( jeBody **PB ) 
{
	jeBody *B;
	int i;

	B = *PB;
	if ( ! B )
		return;
	B->IsValid = NULL;
	if (B->XSkinVertexArray != NULL)
		{
			jeRam_Free( B->XSkinVertexArray );
			B->XSkinVertexArray = NULL;
		}
	if (B->SkinNormalArray != NULL)
		{
			jeRam_Free( B->SkinNormalArray );
			B->SkinNormalArray = NULL;
		}
	if (B->BoneNames != NULL)
		{
			jeStrBlock_Destroy(&(B->BoneNames));
			B->BoneNames = NULL;
		}
	if (B->BoneArray != NULL)
		{	
			jeRam_Free(B->BoneArray);
			B->BoneArray = NULL;
		}
	if (B->MaterialArray != NULL)
		{
			for (i=0; i<B->MaterialCount; i++)
				{
					// <> CB ; see note above
					// this doesn't seem to prevent us from crashing here
					//	when an actor has an error during _Create
					#if 1
					if ( (uint32)(B->MaterialArray[i].MatSpec) > 1 )
					#endif
						jeMaterialSpec_Destroy(&(B->MaterialArray[i].MatSpec));
					B->MaterialArray[i].MatSpec = NULL;
				}
			jeRam_Free( B->MaterialArray );
			B->MaterialArray = NULL;
		}
	if (B->MaterialNames != NULL)
		{
			jeStrBlock_Destroy(&(B->MaterialNames));
			B->MaterialNames = NULL;
		}
	
	for (i=0; i<JE_BODY_NUMBER_OF_LOD; i++)
		{
			if (B->SkinFaces[i].FaceArray != NULL)
				{
					jeRam_Free(B->SkinFaces[i].FaceArray);
					B->SkinFaces[i].FaceArray = NULL;
				}
		}

	if (B->blendDataArray != NULL)
	{
		jeRam_Free(B->blendDataArray);
		B->blendDataArray = NULL;
	}

	jeRam_Free(*PB);
	*PB = NULL;
}

JETAPI jeBody *JETCC jeBody_Create(void)
{
	jeBody *B;

	B = jeBody_CreateNull();
	if ( B == NULL)
		{
			jeErrorLog_Add(JE_ERR_MEMORY_RESOURCE, "jeBody_Create.");
			return NULL;
		}

	B->BoneNames = jeStrBlock_Create();
	if (B->BoneNames == NULL)
		{
			jeErrorLog_Add(JE_ERR_SUBSYSTEM_FAILURE, "jeBody_Create.");
			jeBody_DestroyPossiblyIncompleteBody(&B);
			return NULL;
		}
	B->MaterialNames	= jeStrBlock_Create();

	if (B->MaterialNames == NULL)
		{
			jeErrorLog_Add(JE_ERR_SUBSYSTEM_FAILURE, "jeBody_Create.");
			jeBody_DestroyPossiblyIncompleteBody(&B);
			return NULL;
		}

	assert( jeBody_SanityCheck(B) != JE_FALSE );
	return B;
}

JETAPI void JETCC jeBody_Destroy(jeBody **PB)
{
	assert(  PB != NULL );
	assert( *PB != NULL );
	assert( jeBody_IsValid(*PB) != JE_FALSE );
	jeBody_DestroyPossiblyIncompleteBody( PB );
}


JETAPI jeBoolean JETCC jeBody_GetGeometryStats(const jeBody *B, int lod, int *Vertices, int *Faces, int *Normals)
{
	assert( jeBody_IsValid(B) == JE_TRUE );
	assert( ( lod >=0 ) && ( lod < JE_BODY_NUMBER_OF_LOD ) );
	*Vertices = B->XSkinVertexCount;
	*Faces    = B->SkinFaces[lod].FaceCount;
	*Normals  = B->SkinNormalCount;
	return JE_TRUE;
}



JETAPI int JETCC jeBody_GetBoneCount(const jeBody *B)
{
	assert( B != NULL );
	assert( jeBody_IsValid(B) != JE_FALSE );
	return B->BoneCount;
}
	
JETAPI void JETCC jeBody_GetBone(const jeBody *B, 
	int BoneIndex, 
	const char **BoneName,
	jeXForm3d *Attachment,
	int *ParentBoneIndex)
{
	assert( B != NULL );
	assert( jeBody_IsValid(B) != JE_FALSE );
	assert( Attachment != NULL );
	assert( ParentBoneIndex != NULL );
	assert(  BoneName != NULL );

	assert( BoneIndex >=0 );
	assert( BoneIndex < B->BoneCount );
	*Attachment = B->BoneArray[BoneIndex].AttachmentMatrix;
	*ParentBoneIndex = B->BoneArray[BoneIndex].ParentBoneIndex;
	*BoneName = jeStrBlock_GetString(B->BoneNames,BoneIndex);
}

JETAPI int32 JETCC jeBody_GetBoneNameChecksum(const jeBody *B)
{
	assert( jeBody_IsValid(B) != JE_FALSE );
	
	if (B->BoneNames != NULL)
		{
			return jeStrBlock_GetChecksum( B->BoneNames );
		}
	else
		return 0;
}


JETAPI jeBoolean JETCC jeBody_GetBoundingBox( const jeBody *B, 
							int BoneIndex, 
							jeVec3d *MinimumBoxCorner,
							jeVec3d *MaximumBoxCorner)
{
	assert( B != NULL);
	assert( MinimumBoxCorner != NULL );
	assert( MaximumBoxCorner != NULL );
	assert( (BoneIndex >=0)            || (BoneIndex == JE_BODY_ROOT));
	assert( (BoneIndex < B->BoneCount) || (BoneIndex == JE_BODY_ROOT));
	if (BoneIndex == JE_BODY_ROOT)
		{
		#pragma message ("discontinue this?")
			*MinimumBoxCorner = B->BoundingBoxMin;
			*MaximumBoxCorner = B->BoundingBoxMax;
		}
	else
		{			
			jeBody_Bone *Bone = &(B->BoneArray[BoneIndex]);

			if (Bone->BoundingBoxMin.X > Bone->BoundingBoxMax.X)
				{
					// bone has no bounding box (hopefully because it has no geometry)  
					// This is a valid condition - not really an error.
					// it's possible that this could be an error condition.  But if it is
					// it is ignored.
					return JE_FALSE;
				}
			*MinimumBoxCorner = Bone->BoundingBoxMin;
			*MaximumBoxCorner = Bone->BoundingBoxMax;
		}
	return JE_TRUE;
}

JETAPI void JETCC jeBody_SetBoundingBox( jeBody *B, 
							int BoneIndex,
							const jeVec3d *MinimumBoxCorner,
							const jeVec3d *MaximumBoxCorner)
{
	assert( B != NULL);
	assert( MinimumBoxCorner != NULL );
	assert( MaximumBoxCorner != NULL );
	assert( (BoneIndex >=0)            || (BoneIndex == JE_BODY_ROOT));
	assert( (BoneIndex < B->BoneCount) || (BoneIndex == JE_BODY_ROOT));
	if (BoneIndex == JE_BODY_ROOT)
		{
			B->BoundingBoxMin = *MinimumBoxCorner;
			B->BoundingBoxMax = *MaximumBoxCorner;
		}
	else
		{			
			B->BoneArray[BoneIndex].BoundingBoxMin = *MinimumBoxCorner;
			B->BoneArray[BoneIndex].BoundingBoxMax = *MaximumBoxCorner;
		}
}
							



JETAPI jeBoolean JETCC jeBody_GetBoneByName(const jeBody* B,
	const char* BoneName,
	int* pBoneIndex,
	jeXForm3d* Attachment,
	int* pParentBoneIndex)
{
	assert( B != NULL );
	assert( jeBody_IsValid(B) != JE_FALSE );
	assert( Attachment != NULL );
	assert( pParentBoneIndex != NULL );
	assert( pBoneIndex != NULL );
	assert(  BoneName != NULL );

	if(jeStrBlock_FindString(B->BoneNames, BoneName, pBoneIndex) == JE_TRUE)
	{
		*Attachment = B->BoneArray[*pBoneIndex].AttachmentMatrix;
		*pParentBoneIndex = B->BoneArray[*pBoneIndex].ParentBoneIndex;

		return JE_TRUE;
	}

	return JE_FALSE;
}

JETAPI int JETCC jeBody_GetMaterialCount(const jeBody *B)
{
	assert( B != NULL );
	assert( jeBody_IsValid(B) != JE_FALSE );
	return B->MaterialCount;
}

#define JE_BODY_TOLERANCE (0.001f)

static jeBoolean JETCF jeBody_XSkinVertexCompare(
	const jeBody_XSkinVertex *SV1,
	const jeBody_XSkinVertex *SV2)
{
	assert( SV1 != NULL );
	assert( SV2 != NULL );
	if (jeVec3d_Compare( &(SV1->XPoint), &(SV2->XPoint), 
						JE_BODY_TOLERANCE) == JE_FALSE)
		{
			return JE_FALSE;
		}
	if (fabs(SV1->XU - SV2->XU) > JE_BODY_TOLERANCE)
		{
			return JE_FALSE;
		}
	if (fabs(SV1->XV - SV2->XV) > JE_BODY_TOLERANCE)
		{
			return JE_FALSE;
		}
	return JE_TRUE;
}	


static void JETCF jeBody_SwapVertexIndices( jeBody *B, jeBody_Index Index1, jeBody_Index Index2)
	// zips through all triangles, and swaps index1 and index2.
{
	int i,j,lod;
	jeBody_Index Count;
	jeBody_Triangle *T;

	assert( B!=NULL );	
	for (lod = 0; lod< JE_BODY_NUMBER_OF_LOD; lod++)
		{
			Count = B->SkinFaces[lod].FaceCount;
			for (i=0,T=B->SkinFaces[lod].FaceArray;
					i<Count; 
					i++,T++)
				{
					for (j=0; j<3; j++)	
						{	
							if (T->VtxIndex[j] == Index1)
								{
									T->VtxIndex[j] = Index2;
								}
							else
								{
									if (T->VtxIndex[j] == Index2)
										{
											T->VtxIndex[j] = Index1;
										}
								}	
						}
				}
		}
}


typedef struct 
		{
			int BoneIndex;
			int OriginalIndex;
			int ReMapIndex;
		} jeBody_SkinSortVMap;

static int jeBody_QSortSkinVertexCompare( const void *arg1, const void *arg2 )
{
	jeBody_SkinSortVMap *V1,*V2;
	assert( arg1 );
	assert( arg2 );

	V1 = (jeBody_SkinSortVMap *)arg1; 
	V2 = (jeBody_SkinSortVMap *)arg2;

	if (V1->BoneIndex < V2->BoneIndex)
		return -1;
	if (V1->BoneIndex > V2->BoneIndex)
		return 1;
	return 0;
}


static jeBoolean JETCF jeBody_SortSkinVertices( jeBody *B )
{
	jeBody_Triangle *T;
	int i,j,lod;
	int Count;
	jeBoolean AnyChanges = JE_FALSE;
	jeBody_SkinSortVMap *VertexMap;
	jeBody_XSkinVertex  *VertexCopy;

	assert( B != NULL );
	
	Count = B->XSkinVertexCount;
	VertexMap = JE_RAM_ALLOCATE_ARRAY(jeBody_SkinSortVMap,Count);
	if (VertexMap == NULL)
		{
			jeErrorLog_AddString(JE_ERR_MEMORY_RESOURCE,"jeBody_SortSkinVertices: failed to allocate array",NULL);
			return JE_FALSE;
		}
	VertexCopy = JE_RAM_ALLOCATE_ARRAY(jeBody_XSkinVertex,Count);
	if (VertexMap == NULL)
		{
			jeErrorLog_AddString(JE_ERR_MEMORY_RESOURCE,"jeBody_SortSkinVertices: failed to allocate copy of vertex array",NULL);
			jeRam_Free(VertexMap);
			return JE_FALSE;
		}
	for (i=0; i<Count; i++)
		{
			VertexMap[i].BoneIndex = B->XSkinVertexArray[i].BoneIndex;
			VertexMap[i].OriginalIndex = i;
			VertexCopy[i] = B->XSkinVertexArray[i];
		}

	qsort( VertexMap, Count, sizeof(VertexMap[0]), jeBody_QSortSkinVertexCompare);

	for (i=0; i<Count; i++)
		{
			B->XSkinVertexArray[i] = VertexCopy[VertexMap[i].OriginalIndex];
			VertexMap[VertexMap[i].OriginalIndex].ReMapIndex = i;
		}

	for (lod = 0; lod< JE_BODY_NUMBER_OF_LOD; lod++)
		{
			Count = B->SkinFaces[lod].FaceCount;
			for (i=0,T=B->SkinFaces[lod].FaceArray;
					i<Count; 
					i++,T++)
				{
					for (j=0; j<3; j++)	
						{	
							T->VtxIndex[j] = VertexMap[T->VtxIndex[j]].ReMapIndex;
						}
				}
		}

	jeRam_Free(VertexMap);
	jeRam_Free(VertexCopy);
	return JE_TRUE;

#if 0

	for (i=0; i<Count; i++)
		{
			for (j=0; j<Count-1; j++)
				{
					if (B->XSkinVertexArray[j].BoneIndex > B->XSkinVertexArray[j+1].BoneIndex)
						{
							jeBody_XSkinVertex Swap;

							Swap= B->XSkinVertexArray[j];
							B->XSkinVertexArray[j] = B->XSkinVertexArray[j+1];
							B->XSkinVertexArray[j+1] = Swap;
							jeBody_SwapVertexIndices(B,(jeBody_Index)j,(jeBody_Index)(j+1));
							AnyChanges = JE_TRUE;
						}
				}
			if (AnyChanges != JE_TRUE)
				{
					break;
				}
			AnyChanges = JE_FALSE;
		}
#endif
}

// @@	
static jeBoolean JETCF jeBody_AddSkinVertex(	jeBody *B,
	const jeVec3d *Vertex, 
	jeFloat U, jeFloat V,
	jeBody_Index BoneIndex, 
	jeBody_Index *Index,
	int16 nBlends,
	jeBody_Index bdaOffset)
{
	jeBody_Bone *Bone;
	jeBody_XSkinVertex *SV;
	jeBody_XSkinVertex NewSV;
	int i;
	assert( B != NULL );
	assert( Vertex != NULL );
	assert( Index != NULL );
	assert( jeBody_IsValid(B) != JE_FALSE );
		
	assert( B->XSkinVertexCount+1 > 0 );
	
	NewSV.XPoint = *Vertex;
	NewSV.XU     =  U;
	NewSV.XV     =  V;
	NewSV.LevelOfDetailMask = JE_BODY_HIGHEST_LOD_MASK;
	NewSV.BoneIndex = BoneIndex;
	NewSV.nBlends = nBlends;
	NewSV.bdaOffset = bdaOffset;

	
	assert( B->BoneCount > BoneIndex );
	Bone = &(B->BoneArray[BoneIndex]);
	

	if ((B->optFlags & JE_BODY_OPTIMIZE_FLAGS_VERTS))
	{
		// see if new Vertex is already in XSkinVertexArray
		for (i=0; i<B->XSkinVertexCount; i++)
			{
				SV = &(B->XSkinVertexArray[i]);
				if (SV->BoneIndex == BoneIndex && 
					SV->nBlends == nBlends && SV->bdaOffset == bdaOffset)
					{
						if (jeBody_XSkinVertexCompare(SV,&NewSV) == JE_TRUE )
							{
								*Index = (jeBody_Index)i;
								return JE_TRUE;
							}
					}
			}
	}
	// new Vertex needs to be added to XSkinVertexArray
	SV = JE_RAM_REALLOC_ARRAY( B->XSkinVertexArray ,
					jeBody_XSkinVertex, (B->XSkinVertexCount + 1) );
	if ( SV == NULL )
		{
			jeErrorLog_Add(JE_ERR_MEMORY_RESOURCE, "jeBody_AddSkinVertex.");
			return JE_FALSE;
		}
	B->XSkinVertexArray = SV;

	B->XSkinVertexArray[B->XSkinVertexCount] = NewSV;
	*Index = B->XSkinVertexCount;

	Bone->BoundingBoxMin.X = MIN(Bone->BoundingBoxMin.X,NewSV.XPoint.X);
	Bone->BoundingBoxMin.Y = MIN(Bone->BoundingBoxMin.Y,NewSV.XPoint.Y);
	Bone->BoundingBoxMin.Z = MIN(Bone->BoundingBoxMin.Z,NewSV.XPoint.Z);
	Bone->BoundingBoxMax.X = MAX(Bone->BoundingBoxMax.X,NewSV.XPoint.X);
	Bone->BoundingBoxMax.Y = MAX(Bone->BoundingBoxMax.Y,NewSV.XPoint.Y);
	Bone->BoundingBoxMax.Z = MAX(Bone->BoundingBoxMax.Z,NewSV.XPoint.Z);

	B->XSkinVertexCount ++ ;
	return JE_TRUE;
}



// @@
static jeBoolean JETCF jeBody_AddNormal( jeBody *B, 
		const jeVec3d *Normal, 
		jeBody_Index BoneIndex, 
		jeBody_Index *Index,
		int16 nBlends,
		jeBody_Index bdaOffset )
{
	jeBody_Normal *NewNormalArray;
	jeBody_Normal *N;
	jeVec3d NNorm;
	int i;

	assert(      B != NULL );
	assert( Normal != NULL );
	assert(  Index != NULL );	
	assert( jeBody_IsValid(B) != JE_FALSE );
	
	assert( B->SkinNormalCount+1 > 0 );
	NNorm = *Normal;
	jeVec3d_Normalize(&NNorm);		

	if ((B->optFlags & JE_BODY_OPTIMIZE_FLAGS_NORMALS))
	{
		// see if new normal is already in SkinNormalArray
		for (i=0, N = B->SkinNormalArray; i<B->SkinNormalCount; i++,N++)
			{
				if (N->BoneIndex == BoneIndex && 
					N->nBlends == nBlends && N->bdaOffset == bdaOffset)
					{
						if ( jeVec3d_Compare( &(N->Normal),&NNorm,JE_BODY_TOLERANCE ) == JE_TRUE )
							{
								*Index = (jeBody_Index)i;
								return JE_TRUE;
							}
					}
			}
	}

	//  new normal needs to be added to SkinNormalArray
	NewNormalArray = JE_RAM_REALLOC_ARRAY( B->SkinNormalArray,		
						jeBody_Normal,(B->SkinNormalCount+1));
	if (NewNormalArray == NULL)
		{
			jeErrorLog_Add(JE_ERR_MEMORY_RESOURCE, "jeBody_AddNormal");
			return JE_FALSE;
		}
	B->SkinNormalArray = NewNormalArray;
	B->SkinNormalArray[ B->SkinNormalCount ].Normal    = NNorm;
	B->SkinNormalArray[ B->SkinNormalCount ].BoneIndex = BoneIndex;
	B->SkinNormalArray[ B->SkinNormalCount ].LevelOfDetailMask = JE_BODY_HIGHEST_LOD_MASK;
	B->SkinNormalArray[ B->SkinNormalCount ].nBlends = nBlends;
	B->SkinNormalArray[ B->SkinNormalCount ].bdaOffset = bdaOffset;

	*Index = B->SkinNormalCount;
	B->SkinNormalCount ++ ;
	return JE_TRUE;
}



static jeBoolean JETCF jeBody_AddToFaces( jeBody *B, jeBody_Triangle *F, int DetailLevel )
{
	jeBody_Triangle *NewFaceArray;
	jeBody_TriangleList *FL;
	
	assert( B != NULL );
	assert( F != NULL );
	assert( DetailLevel >= 0);
	assert( DetailLevel < JE_BODY_NUMBER_OF_LOD );
	assert( jeBody_IsValid(B) != JE_FALSE );

	FL = &( B->SkinFaces[DetailLevel] );
	
	assert( F->MaterialIndex >= 0 );
	assert( F->MaterialIndex < B->MaterialCount );
	
	NewFaceArray = JE_RAM_REALLOC_ARRAY( FL->FaceArray, 
						jeBody_Triangle,(FL->FaceCount+1) );
	if ( NewFaceArray == NULL )
		{
			jeErrorLog_Add(JE_ERR_MEMORY_RESOURCE, "jeBody_AddToFaces");
			return JE_FALSE;
		}

	FL->FaceArray = NewFaceArray;
	
	{
		int i;
		// insertion sort new face into FaceArray keqyed on MaterialIndex
		if ((B->optFlags & JE_BODY_OPTIMIZE_FLAGS_SORT_FACES))
		{
			jeBody_Index MaterialIndex = F->MaterialIndex;
			for (i=FL->FaceCount; i>=1; i--)
				{
					if (FL->FaceArray[i-1].MaterialIndex <= MaterialIndex)
						break;
					FL->FaceArray[i] = FL->FaceArray[i-1];
				}
			FL->FaceArray[i] = *F;
		}
		else
		{
			FL->FaceArray[FL->FaceCount] = *F;
		}			
	}
	FL->FaceCount ++;

	return JE_TRUE;
}
			
JETAPI jeBoolean JETCC jeBody_SetOptimizeFlags(jeBody* pBody, uint32 flags)
{	
	assert(pBody);

	pBody->optFlags = flags;

	return JE_TRUE;
}


static int jeBody_QSortFaceCompare( const void *arg1, const void *arg2 )
{
	jeBody_Triangle  *T1,*T2;
	assert( arg1 );
	assert( arg2 );

	T1 = (jeBody_Triangle  *)arg1; 
	T2 = (jeBody_Triangle  *)arg2;

	if (T1->MaterialIndex < T2->MaterialIndex)
		return -1;
	if (T1->MaterialIndex > T2->MaterialIndex)
		return 1;
	return 0;
}


JETAPI jeBoolean JETCC jeBody_Optimize(jeBody *pBody)
{
	int lod;

	assert( pBody );
	
	// sort the verts by bone
	if (jeBody_SortSkinVertices(pBody)==JE_FALSE)
		{
			jeErrorLog_Add(JE_ERR_SUBSYSTEM_FAILURE, "jeBody_Optimize");
			return JE_FALSE;
		}
	
	//sort the polys
	for (lod = 0; lod< JE_BODY_NUMBER_OF_LOD; lod++)
		{
			jeBody_TriangleList *FL;
			FL = &( pBody->SkinFaces[lod] );
			if (FL->FaceCount>0)
				qsort( &(FL->FaceArray[0]), FL->FaceCount, sizeof(FL->FaceArray[0]), jeBody_QSortFaceCompare);
		}
	
	return JE_TRUE;
}

// ---------------------------------------------------------------------------------------
// jeBody_AddBlendData - add blend elements to a jeBody
// ---------------------------------------------------------------------------------------
// params							use
// ---------------------------------------------------------------------------------------
// pBody							pointer to jeBody structure
// weight							weighting used for blend
// pLoc								pointer to position vector of blend in bone B's local frame
// pNormal							pointer to normal vector of blend in bone B's local frame
// boneIndex					index of bone B in jeBody structure
// ---------------------------------------------------------------------------------------
// @@

jeBoolean jeBody_AddBlendData(jeBody* pBody, jeFloat weight, const jeVec3d* pLoc, const jeVec3d* pNormal, int boneIndex)
{
	jeBody_BlendData* pBD;

	assert(pBody != NULL);
	assert(pLoc != NULL);
	assert(pNormal != NULL);
	assert(boneIndex >= 0 && boneIndex < pBody->BoneCount);

	pBD = (jeBody_BlendData*)jeRam_Realloc(pBody->blendDataArray, (pBody->blendDataCount + 1) * sizeof(jeBody_BlendData));
	if(pBD == NULL)
	{
		jeErrorLog_Add(JE_ERR_MEMORY_RESOURCE, "jeBody_AddBlendData");
		return JE_FALSE;
	}
	pBody->blendDataArray = pBD;

	// reusing pBD here - ouchie ...
	pBD = &pBody->blendDataArray[pBody->blendDataCount];

	pBD->weight = weight;
	pBD->XPoint = *pLoc;
	pBD->Normal = *pNormal;
	pBD->boneIndex = (jeBody_Index)boneIndex;

	pBody->blendDataCount ++;

	return JE_TRUE;
}

JETAPI int16 JETCC jeBody_GetBlendDataCount(const jeBody* pBody)
{
	assert(pBody != NULL);

	return pBody->blendDataCount;
}

// @@
jeBoolean jeBody_AddBlendFace(jeBody* pBody,
	const jeVec3d* pVert1, const jeVec3d* pNormal1, 
	jeFloat u1, jeFloat v1, int boneIndex1, int nBlends1, int bdaOffset1,

	const jeVec3d* pVert2, const jeVec3d* pNormal2, 
	jeFloat u2, jeFloat v2, int boneIndex2, int nBlends2, int bdaOffset2,

	const jeVec3d* pVert3, const jeVec3d* pNormal3, 
	jeFloat u3, jeFloat v3, int boneIndex3, int nBlends3, int bdaOffset3,

	int materialIndex)
{
	jeBody_Triangle F;

	assert(pBody != NULL);
	assert(jeBody_IsValid(pBody) != JE_FALSE);

	assert(pVert1 != NULL);
	assert(pNormal1 != NULL);
	assert(boneIndex1 >= 0);
	assert(boneIndex1 < pBody->BoneCount);
	assert((nBlends1 + bdaOffset1) >= 0);
	assert((nBlends1 + bdaOffset1) <= pBody->blendDataCount);
	assert(bdaOffset1 >= 0);
	assert(bdaOffset1 <= pBody->blendDataCount);

	assert(pVert2 != NULL);
	assert(pNormal2 != NULL);
	assert(boneIndex2 >= 0);
	assert(boneIndex2 < pBody->BoneCount);
	assert((nBlends2 + bdaOffset2) >= 0);
	assert((nBlends2 + bdaOffset2) <= pBody->blendDataCount);
	assert(bdaOffset2 >= 0);
	assert(bdaOffset2 <= pBody->blendDataCount);

	assert(pVert3 != NULL);
	assert(pNormal3 != NULL);
	assert(boneIndex3 >= 0);
	assert(boneIndex3 < pBody->BoneCount);
	assert((nBlends3 + bdaOffset3) >= 0);
	assert((nBlends3 + bdaOffset3) <= pBody->blendDataCount);
	assert(bdaOffset3 >= 0);
	assert(bdaOffset3 <= pBody->blendDataCount);

	assert(materialIndex >= 0);
	assert(materialIndex < pBody->MaterialCount);

	// add verts

	if (jeBody_AddSkinVertex(pBody, pVert1, u1, v1, (jeBody_Index)boneIndex1, &(F.VtxIndex[0]),
		(int16)nBlends1, (jeBody_Index)bdaOffset1) == JE_FALSE)
	{
		// error already recorded
		return JE_FALSE;
	}
	if (jeBody_AddSkinVertex(pBody, pVert2, u2, v2, (jeBody_Index)boneIndex2, &(F.VtxIndex[1]),
		(int16)nBlends2, (jeBody_Index)bdaOffset2) == JE_FALSE)
	{
		// error already recorded
		return JE_FALSE;
	}
	if (jeBody_AddSkinVertex(pBody, pVert3, u3, v3, (jeBody_Index)boneIndex3, &(F.VtxIndex[2]),
		(int16)nBlends3, (jeBody_Index)bdaOffset3) == JE_FALSE)
	{
		// error already recorded
		return JE_FALSE;
	}

	// add normals

	if (jeBody_AddNormal(pBody, pNormal1, (jeBody_Index)boneIndex1, &(F.NormalIndex[0]), 
		(int16)nBlends1, (jeBody_Index)bdaOffset1) == JE_FALSE)
	{	
		// error already recorded
		return JE_FALSE;
	}
	if (jeBody_AddNormal(pBody, pNormal2, (jeBody_Index)boneIndex2, &(F.NormalIndex[1]), 
		(int16)nBlends2, (jeBody_Index)bdaOffset2) == JE_FALSE)
	{	
		// error already recorded
		return JE_FALSE;
	}
	if (jeBody_AddNormal(pBody, pNormal3, (jeBody_Index)boneIndex3, &(F.NormalIndex[2]), 
		(int16)nBlends3, (jeBody_Index)bdaOffset3) == JE_FALSE)
	{	
		// error already recorded
		return JE_FALSE;
	}

	// add face

	F.MaterialIndex = (jeBody_Index)materialIndex;
	if (jeBody_AddToFaces(pBody, &F, JE_BODY_HIGHEST_LOD ) == JE_FALSE)
	{	
		// error already recorded
		return JE_FALSE;
	}

	if ((pBody->optFlags & JE_BODY_OPTIMIZE_FLAGS_SORT_VERTS))
		if (jeBody_SortSkinVertices(pBody)==JE_FALSE)
			{
				//ignore.
			}

	return JE_TRUE;
}

jeBoolean jeBody_CompareBlendData(const jeBody_BlendData* pBD, const jeVec3d* pV, const jeVec3d* pN, jeFloat weight, jeBody_Index boneIndex)
{
	assert( pBD != NULL );
	assert( pV != NULL );

	if(jeVec3d_Compare(&pBD->XPoint, pV, JE_BODY_TOLERANCE) == JE_FALSE)
		return(JE_FALSE);

	if(jeVec3d_Compare(&pBD->Normal, pN, JE_BODY_TOLERANCE) == JE_FALSE)
		return(JE_FALSE);

	if(fabs(pBD->weight - weight) > JE_BODY_TOLERANCE)
		return(JE_FALSE);

	if(pBD->boneIndex != boneIndex)
		return(JE_FALSE);

#pragma message ("(steve)FIX ME:")
#pragma message ("Steve:  can you also make sure that the optimization flags are handled right")
#pragma message ("for the blended poly additions?  Thanks")

	return(JE_TRUE);
}

jeBoolean jeBody_FindBlendData(const jeBody* pBody, 
							   const jeVec3d* pVerts, 
							   const jeVec3d* pNormals,
							   const jeFloat* pWeights, 
							   const int* pBoneIndexes, 
							   int NumVerts, 
							   int* pBlendDataOffset) // return here if search successful
{
	int i, j;

	assert(pBody != NULL);
	assert(pVerts != NULL);
	assert(pNormals != NULL);
	assert(pWeights != NULL);
	assert(pBoneIndexes != NULL);
	assert(NumVerts > 1); // make this search useful
	assert(pBlendDataOffset != NULL);

	for(i=0;i<(pBody->blendDataCount - NumVerts);i++)
	{
		if(jeBody_CompareBlendData(pBody->blendDataArray + i, pVerts + 0, pNormals + 0, pWeights[0], (jeBody_Index)pBoneIndexes[0]) != JE_FALSE)
		{
			for(j=1;j<NumVerts;j++)
			{
				if(jeBody_CompareBlendData(pBody->blendDataArray + i + j, pVerts + j, pNormals + j, pWeights[j], (jeBody_Index)pBoneIndexes[j]) == JE_FALSE)
				{
					break;
				}
			}
			if(j == NumVerts)
			{
				// found them
				*pBlendDataOffset = i;
				return(JE_TRUE);
			}
		}
	}

	return(JE_FALSE);
}

// ---------------------------------------------------------------------------------------
// jeBody_AddBlendDatArrayWithRedundancyCheck - add some blend elements to a jeBody,
// checking to see if the same bone(s) are referenced more than once.
// ---------------------------------------------------------------------------------------
// params							use
// ---------------------------------------------------------------------------------------
// pBody								pointer to jeBody structure
// pWeights							array of weightings
// pVerts								array of vertices
// pNormals							array of normals
// pBoneIndices					array of bone indices
// num									number of blendings
// ---------------------------------------------------------------------------------------
// @@

jeBoolean jeBody_AddBlendDataArrayWithRedundancyCheck(jeBody* pBody,
	const jeFloat* pWeights, const jeVec3d* pVerts, const jeVec3d* pNormals,
	const int* pBoneIndices, int num, int* pNumActualBlends)
{
	int i, currBoneIndex, j;
	float totalWeight;
	jeBoolean* pVisitedIndices;
	jeBoolean found;

	assert(pBody != NULL);
	assert(pWeights != NULL);
	assert(pVerts != NULL);
	assert(pNormals != NULL);
	assert(pBoneIndices != NULL);
	assert(num > 0);
	assert(pNumActualBlends != NULL);

	pVisitedIndices = (jeBoolean*)jeRam_Allocate(num * sizeof(jeBoolean));
	assert(pVisitedIndices != NULL);

	for (i = 0; i < num; i ++)
		pVisitedIndices[i] = JE_FALSE;

	*pNumActualBlends = 0;

	for (i = 0; i < num; i ++)
	{
		if (pVisitedIndices[i] == JE_TRUE)
			continue;

		currBoneIndex = pBoneIndices[i];
		totalWeight = 0.0f;

		for (j = i; j < num; j ++)
		{
			if (pVisitedIndices[j] == JE_FALSE && pBoneIndices[j] == currBoneIndex)
			{
				pVisitedIndices[j] = JE_TRUE;
				totalWeight += pWeights[j];
			}
		}

		// search for an already existing blend data with these characteristics

		found = JE_FALSE;

		for (j = 0; j < pBody->blendDataCount; j ++)
		{
			if (jeBody_CompareBlendData(&pBody->blendDataArray[j], 
				&pVerts[i], &pNormals[i], totalWeight, (jeBody_Index)currBoneIndex) == JE_TRUE)
			{
				found = JE_TRUE;
				break;
			}
		}

		if (found == JE_FALSE)
		{
			if (JE_FALSE == jeBody_AddBlendData(pBody, totalWeight, 
				&pVerts[i], &pNormals[i], currBoneIndex))
			{
				return(JE_FALSE);
			}

			*pNumActualBlends ++;
		}
	}

	jeRam_Free(pVisitedIndices);

	return JE_TRUE;
}

//#define USE_STEVE

#ifndef USE_STEVE

JETAPI jeBoolean JETCC jeBody_AddFaceWeightedVerts(	jeBody* pBody,
	const jeVec3d* pVerts1, const jeVec3d* pNormals1, 
		jeFloat u1, jeFloat v1, const int* pBoneIndexes1, 
		const jeFloat* pVertWeights1, int NumVerts1,
	const jeVec3d* pVerts2, const jeVec3d* pNormals2, 
		jeFloat u2, jeFloat v2, const int* pBoneIndexes2, 
		const jeFloat* pVertWeights2, int NumVerts2,
	const jeVec3d* pVerts3, const jeVec3d* pNormals3, 
		jeFloat u3, jeFloat v3, const int* pBoneIndexes3, 
		const jeFloat* pVertWeights3, int NumVerts3,
	int materialIndex)
{
	int bdaOffset1, bdaOffset2, bdaOffset3;
	jeBoolean bResult;
	int nBlends1, nBlends2, nBlends3;
#ifdef _DEBUG
	int i;
#endif

	assert(pBody != NULL);
	assert(jeBody_IsValid(pBody) != JE_FALSE);

	assert(pVerts1 != NULL);
	assert(pNormals1 != NULL);
	assert(pVertWeights1 != NULL);
	assert(pBoneIndexes1 != NULL);
	assert(NumVerts1 > 0);
#ifdef _DEBUG
	for(i=0;i<NumVerts1;i++)
	{
		assert(pBoneIndexes1[i] >= 0);
		assert(pBoneIndexes1[i] < pBody->BoneCount);
	}
#endif

	assert(pVerts2 != NULL);
	assert(pNormals2 != NULL);
	assert(pVertWeights2 != NULL);
	assert(pBoneIndexes2 != NULL);
	assert(NumVerts2 > 0);
#ifdef _DEBUG
	for(i=0;i<NumVerts2;i++)
	{
		assert(pBoneIndexes2[i] >= 0);
		assert(pBoneIndexes2[i] < pBody->BoneCount);
	}
#endif

	assert(pVerts3 != NULL);
	assert(pNormals3 != NULL);
	assert(pVertWeights3 != NULL);
	assert(pBoneIndexes3 != NULL);
	assert(NumVerts3 > 0);
#ifdef _DEBUG
	for(i=0;i<NumVerts3;i++)
	{
		assert(pBoneIndexes3[i] >= 0);
		assert(pBoneIndexes3[i] < pBody->BoneCount);
	}
#endif

	assert(materialIndex >= 0);
	assert(materialIndex < pBody->MaterialCount);

	if (NumVerts1 > 1)
	{
		bdaOffset1 = pBody->blendDataCount;

		jeBody_AddBlendDataArrayWithRedundancyCheck(pBody,
			pVertWeights1, pVerts1, pNormals1, pBoneIndexes1, NumVerts1, &nBlends1);
	}
	else
	{
		nBlends1 = 0;
		bdaOffset1 = 0;
	}

	if (NumVerts2 > 1)
	{
		bdaOffset2 = pBody->blendDataCount;

		jeBody_AddBlendDataArrayWithRedundancyCheck(pBody,
			pVertWeights2, pVerts2, pNormals2, pBoneIndexes2, NumVerts2, &nBlends2);
	}
	else
	{
		nBlends2 = 0;
		bdaOffset2 = 0;
	}

	if (NumVerts3 > 1)
	{
		bdaOffset3 = pBody->blendDataCount;

		jeBody_AddBlendDataArrayWithRedundancyCheck(pBody,
			pVertWeights3, pVerts3, pNormals3, pBoneIndexes3, NumVerts3, &nBlends3);
	}
	else
	{
		nBlends3 = 0;
		bdaOffset3 = 0;
	}

	bResult = jeBody_AddBlendFace(pBody, 
		pVerts1, pNormals1, u1, v1, *pBoneIndexes1, nBlends1, bdaOffset1,
		pVerts2, pNormals2, u2, v2, *pBoneIndexes2, nBlends2, bdaOffset2,
		pVerts3, pNormals3, u3, v3, *pBoneIndexes3, nBlends3, bdaOffset3,
		materialIndex);

	return(bResult);
}

/////////////////////////////////////////////////////////////////////////////////
////STEVE'S ORIGINAL CODE FOLLOWS ---------------->>>>>>>>>>>>>  ////////////////
/////////////////////////////////////////////////////////////////////////////////

#else // USE_STEVE

JETAPI jeBoolean JETCC jeBody_AddFaceWeightedVerts(	jeBody* pBody,
	const jeVec3d* pVerts1, const jeVec3d* pNormals1, 
		jeFloat u1, jeFloat v1, const int* pBoneIndexes1, 
		const jeFloat* pVertWeights1, int NumVerts1,
	const jeVec3d* pVerts2, const jeVec3d* pNormals2, 
		jeFloat u2, jeFloat v2, const int* pBoneIndexes2, 
		const jeFloat* pVertWeights2, int NumVerts2,
	const jeVec3d* pVerts3, const jeVec3d* pNormals3, 
		jeFloat u3, jeFloat v3, const int* pBoneIndexes3, 
		const jeFloat* pVertWeights3, int NumVerts3,
	int materialIndex)
{
	int bdaOffset1, bdaOffset2, bdaOffset3;
	jeBoolean bResult;
	int i;
	int nBlends1, nBlends2, nBlends3;

	assert(pBody != NULL);
	assert(jeBody_IsValid(pBody) != JE_FALSE);

	assert(pVerts1 != NULL);
	assert(pNormals1 != NULL);
	assert(pVertWeights1 != NULL);
	assert(pBoneIndexes1 != NULL);
	assert(NumVerts1 > 0);
#ifdef _DEBUG
	for(i=0;i<NumVerts1;i++)
	{
		assert(pBoneIndexes1[i] >= 0);
		assert(pBoneIndexes1[i] < pBody->BoneCount);
	}
#endif

	assert(pVerts2 != NULL);
	assert(pNormals2 != NULL);
	assert(pVertWeights2 != NULL);
	assert(pBoneIndexes2 != NULL);
	assert(NumVerts2 > 0);
#ifdef _DEBUG
	for(i=0;i<NumVerts2;i++)
	{
		assert(pBoneIndexes2[i] >= 0);
		assert(pBoneIndexes2[i] < pBody->BoneCount);
	}
#endif

	assert(pVerts3 != NULL);
	assert(pNormals3 != NULL);
	assert(pVertWeights3 != NULL);
	assert(pBoneIndexes3 != NULL);
	assert(NumVerts3 > 0);
#ifdef _DEBUG
	for(i=0;i<NumVerts3;i++)
	{
		assert(pBoneIndexes3[i] >= 0);
		assert(pBoneIndexes3[i] < pBody->BoneCount);
	}
#endif

	assert(materialIndex >= 0);
	assert(materialIndex < pBody->MaterialCount);

	// Search for already existing data in blendDataArray

	if(NumVerts1 > 1)
	{
		nBlends1 = NumVerts1;

		if(JE_FALSE == jeBody_FindBlendData(pBody, 
											pVerts1, 
											pNormals1,
											pVertWeights1, 
											pBoneIndexes1, 
											NumVerts1, 
											&bdaOffset1) )
		{
			// didn't find them
			bdaOffset1 = pBody->blendDataCount;

			for(i=0;i<NumVerts1;i++)
			{
				if(JE_FALSE == jeBody_AddBlendData(pBody, pVertWeights1[i], pVerts1 + i, pNormals1 + i, pBoneIndexes1[i]))
					return(JE_FALSE);
			}
		}
	}
	else
	{
		nBlends1 = 0;
		bdaOffset1 = 0;
	}

	if(NumVerts2 > 1)
	{
		nBlends2 = NumVerts2;

		if(JE_FALSE == jeBody_FindBlendData(pBody, 
											pVerts2, 
											pNormals2,
											pVertWeights2, 
											pBoneIndexes2, 
											NumVerts2, 
											&bdaOffset2) )
		{
			// didn't find them
			bdaOffset2 = pBody->blendDataCount;

			for(i=0;i<NumVerts2;i++)
			{
				if(JE_FALSE == jeBody_AddBlendData(pBody, pVertWeights2[i], pVerts2 + i, pNormals2 + i, pBoneIndexes2[i]))
					return(JE_FALSE);
			}
		}
	}
	else
	{
		nBlends2 = 0;
		bdaOffset2 = 0;
	}

	if(NumVerts3 > 1)
	{
		nBlends3 = NumVerts3;

		if(JE_FALSE == jeBody_FindBlendData(pBody, 
											pVerts3, 
											pNormals3,
											pVertWeights3, 
											pBoneIndexes3, 
											NumVerts3, 
											&bdaOffset3) )
		{
			// didn't find them
			bdaOffset3 = pBody->blendDataCount;

			for(i=0;i<NumVerts3;i++)
			{
				if(JE_FALSE == jeBody_AddBlendData(pBody, pVertWeights3[i], pVerts3 + i, pNormals3 + i, pBoneIndexes3[i]))
					return(JE_FALSE);
			}
		}
	}
	else
	{
		nBlends3 = 0;
		bdaOffset3 = 0;
	}

	bResult = jeBody_AddBlendFace(pBody, 
		pVerts1, pNormals1, u1, v1, *pBoneIndexes1, nBlends1, bdaOffset1,
		pVerts2, pNormals2, u2, v2, *pBoneIndexes2, nBlends2, bdaOffset2,
		pVerts3, pNormals3, u3, v3, *pBoneIndexes3, nBlends3, bdaOffset3,
		materialIndex);

	return(bResult);
}

#endif // USE_STEVE

JETAPI jeBoolean JETCC jeBody_AddFace(	jeBody *B,
	const jeVec3d *Vertex1, const jeVec3d *Normal1, 
		jeFloat U1, jeFloat V1, int BoneIndex1,
	const jeVec3d *Vertex2, const jeVec3d *Normal2, 
		jeFloat U2, jeFloat V2, int BoneIndex2,
	const jeVec3d *Vertex3, const jeVec3d *Normal3, 
		jeFloat U3, jeFloat V3, int BoneIndex3,
	int MaterialIndex)
{
	jeBody_Triangle F;
	
	assert( B != NULL );
	assert( Vertex1 != NULL );
	assert( Normal1 != NULL );
	assert( jeBody_IsValid(B) != JE_FALSE );

	assert( BoneIndex1 >= 0 );
	assert( BoneIndex1 < B->BoneCount );

	assert( Vertex2 != NULL );
	assert( Normal2 != NULL );
	assert( BoneIndex2 >= 0 );
	assert( BoneIndex2 < B->BoneCount );

	assert( Vertex3 != NULL );
	assert( Normal3 != NULL );
	assert( BoneIndex3 >= 0 );
	assert( BoneIndex3 < B->BoneCount );

	assert( MaterialIndex >= 0 );
	assert(	MaterialIndex < B->MaterialCount );

	if (jeBody_AddSkinVertex(B,Vertex1,U1,V1,(jeBody_Index)BoneIndex1,&(F.VtxIndex[0]), 0, 0)==JE_FALSE)
		{	// error already recorded
			return JE_FALSE;
		}
	if (jeBody_AddSkinVertex(B,Vertex2,U2,V2,(jeBody_Index)BoneIndex2,&(F.VtxIndex[1]), 0, 0)==JE_FALSE)
		{	// error already recorded
			return JE_FALSE;
		}
	if (jeBody_AddSkinVertex(B,Vertex3,U3,V3,(jeBody_Index)BoneIndex3,&(F.VtxIndex[2]), 0, 0)==JE_FALSE)
		{	// error already recorded
			return JE_FALSE;
		}

	if (jeBody_AddNormal( B, Normal1, (jeBody_Index)BoneIndex1, &(F.NormalIndex[0]), 0, 0) == JE_FALSE)
		{	// error already recorded
			return JE_FALSE;
		}
	if (jeBody_AddNormal( B, Normal2, (jeBody_Index)BoneIndex2, &(F.NormalIndex[1]), 0, 0) == JE_FALSE)
		{	// error already recorded
			return JE_FALSE;
		}
	if (jeBody_AddNormal( B, Normal3, (jeBody_Index)BoneIndex3, &(F.NormalIndex[2]), 0, 0) == JE_FALSE)
		{	// error already recorded
			return JE_FALSE;
		}

	F.MaterialIndex = (jeBody_Index)MaterialIndex;
	if (jeBody_AddToFaces( B, &F, JE_BODY_HIGHEST_LOD ) == JE_FALSE)
		{	// error already recorded
			return JE_FALSE;
		}

	if ((B->optFlags & JE_BODY_OPTIMIZE_FLAGS_SORT_VERTS))
		if (jeBody_SortSkinVertices(B)==JE_FALSE)
			{
				//ignore
			}
		
	return JE_TRUE;
			
}


JETAPI jeBoolean JETCC jeBody_AddMaterial( jeBody *B, 
	const char *MaterialName, 
	jeMaterialSpec *Bitmap,
	jeFloat Red, jeFloat Green, jeFloat Blue,
	jeUVMapper pMapper,
	int *MaterialIndex)
{
	int FoundIndex;
	jeBody_Material *NewMaterial;
	assert( B != NULL );
	assert( MaterialIndex != NULL );
	assert( jeBody_IsValid(B) != JE_FALSE );
	assert( B->MaterialCount >= 0 );

	if (MaterialName == NULL)
		{
			jeErrorLog_Add(-1,"jeBody_AddMaterial: name can not be NULL.");
			return JE_FALSE;
		}
	if (MaterialName[0] == 0)
		{
			jeErrorLog_Add(-1,"jeBody_AddMaterial: name must have > 0 length.");
			return JE_FALSE;
		}
	if (jeStrBlock_FindString(B->MaterialNames, MaterialName, &FoundIndex) == JE_TRUE)
		{
			jeErrorLog_AddString(-1,"jeBody_AddMaterial: name already used-", MaterialName);
			return JE_FALSE;
		}
	
	
	NewMaterial = JE_RAM_REALLOC_ARRAY( B->MaterialArray, jeBody_Material,(B->MaterialCount+1) );
	if ( NewMaterial == NULL )
		{
			jeErrorLog_Add(JE_ERR_MEMORY_RESOURCE, "jeBody_AddMaterial.");
			return JE_FALSE;
		}
	
	
	B->MaterialArray = NewMaterial;
	if (jeStrBlock_Append(&(B->MaterialNames),MaterialName) == JE_FALSE)
		{
			jeErrorLog_Add(JE_ERR_MEMORY_RESOURCE, "jeBody_AddMaterial.");
			return JE_FALSE;
		}

	{
		jeBody_Material *M = &(B->MaterialArray[B->MaterialCount]);
		M->MatSpec = Bitmap;
		if (Bitmap != NULL)
			jeMaterialSpec_CreateRef(Bitmap);
		M->Red    = Red;
		M->Green  = Green;
		M->Blue   = Blue;
		M->Mapper = pMapper;

	}
	*MaterialIndex = B->MaterialCount; 
	B->MaterialCount ++;
	return JE_TRUE;
}
			
JETAPI jeBoolean JETCC jeBody_GetMaterial(const jeBody *B, int MaterialIndex,
										const char **MaterialName,
										jeMaterialSpec **Bitmap, jeFloat *Red, jeFloat *Green, jeFloat *Blue,
										jeUVMapper * pMapper)
{
	assert( B      != NULL );
	assert( jeBody_IsValid(B) != JE_FALSE );
	assert( Red    != NULL );
	assert( Green  != NULL );
	assert( Blue   != NULL );
	assert( Bitmap != NULL );
	assert(pMapper != NULL);
	assert( MaterialIndex >= 0 );
	assert( MaterialIndex < B->MaterialCount );
	assert( MaterialName != NULL );
	*MaterialName      = jeStrBlock_GetString(B->MaterialNames,MaterialIndex);

	{
		jeBody_Material *M = &(B->MaterialArray[MaterialIndex]);
		*Bitmap = M->MatSpec;
		*Red    = M->Red;
		*Green  = M->Green;
		*Blue   = M->Blue;
		*pMapper = M->Mapper;
	}
	return JE_TRUE;
}

JETAPI jeBoolean JETCC jeBody_SetMaterial(jeBody *B, int MaterialIndex,
										jeMaterialSpec *Material,  jeFloat Red,  jeFloat Green,  jeFloat Blue,
										jeUVMapper Mapper)
{
	assert( jeBody_IsValid(B) != JE_FALSE );
	assert( MaterialIndex >= 0 );
	assert( MaterialIndex < B->MaterialCount );
	{
		jeBody_Material *M = &(B->MaterialArray[MaterialIndex]);
		M->MatSpec= Material;

		M->Red    = Red;
		M->Green  = Green;
		M->Blue   = Blue;
		M->Mapper = Mapper;
	}
	return JE_TRUE;
}




JETAPI jeBoolean JETCC jeBody_AddBone( jeBody *B, 
	int ParentBoneIndex,
	const char *BoneName, 
	const jeXForm3d *AttachmentMatrix,
	int *BoneIndex)
{
	jeBody_Bone *NewBones;
	assert( B != NULL );
	assert( BoneName != NULL );
	assert( BoneIndex != NULL );
	assert( jeBody_IsValid(B) != JE_FALSE );

	assert( ParentBoneIndex < B->BoneCount );
	assert( ( ParentBoneIndex >= 0)  || (ParentBoneIndex == JE_BODY_NO_PARENT_BONE));
	assert( B->BoneCount >= 0 );
	
	NewBones = JE_RAM_REALLOC_ARRAY( B->BoneArray, 
						jeBody_Bone, (B->BoneCount+1) );
	if ( NewBones == NULL )
		{
			jeErrorLog_Add(JE_ERR_MEMORY_RESOURCE, "jeBody_AddBone.");
			return JE_FALSE;
		}
	
	B->BoneArray = NewBones;
	if (jeStrBlock_Append(&(B->BoneNames),BoneName) == JE_FALSE)
		{
			jeErrorLog_Add(JE_ERR_SUBSYSTEM_FAILURE, "jeBody_AddBone.");
			return JE_FALSE;
		}
	
	{
		jeBody_Bone *Bone = &(B->BoneArray[B->BoneCount]);
		jeVec3d_Set(&(Bone->BoundingBoxMin),
			JE_BODY_REALLY_BIG_NUMBER,JE_BODY_REALLY_BIG_NUMBER,JE_BODY_REALLY_BIG_NUMBER);
		jeVec3d_Set(&(Bone->BoundingBoxMax),
			-JE_BODY_REALLY_BIG_NUMBER,-JE_BODY_REALLY_BIG_NUMBER,-JE_BODY_REALLY_BIG_NUMBER);
		Bone->AttachmentMatrix = *AttachmentMatrix;
		Bone->ParentBoneIndex = (jeBody_Index)ParentBoneIndex;
	}
	*BoneIndex = B->BoneCount;
	B->BoneCount++;
	return JE_TRUE;
}



JETAPI jeBoolean JETCC jeBody_ComputeLevelsOfDetail( jeBody *B ,int Levels)
{
	assert( B != NULL);
	assert( Levels >= 0 );
	assert( Levels < JE_BODY_NUMBER_OF_LOD );
	assert( jeBody_IsValid(B) != JE_FALSE );
	#pragma message ("LOD code goes here:")
	B->LevelsOfDetail = JE_BODY_HIGHEST_LOD_MASK; // Levels
	Levels;
	return JE_TRUE;
}	



#define JE_BODY_GEOMETRY_NAME "Geometry"
#define JE_BODY_BITMAP_DIRECTORY_NAME "Bitmaps"

#define JE_BODY_FILE_TYPE 0x5E444F42     // 'BODY'
#define JE_BODY_FILE_VERSION 0x00F2		// Restrict version to 16 bits




static jeBoolean JETCF jeBody_ReadGeometry(jeBody *B, jeVFile *pFile)
{
	uint32 u;
	int i;

	assert( B != NULL );
	assert( pFile != NULL );
	if(jeVFile_Read(pFile, &u, sizeof(u)) == JE_FALSE)
		{	jeErrorLog_Add( JE_ERR_FILEIO_READ , "jeBody_ReadGeometry: Failed to read header.");	return JE_FALSE; }
	if (u!=JE_BODY_FILE_TYPE)
		{	jeErrorLog_Add( JE_ERR_FILEIO_FORMAT , "jeBody_ReadGeometry: bad or wrong header");  return JE_FALSE; }


	if(jeVFile_Read(pFile, &u, sizeof(u)) == JE_FALSE)
		{	jeErrorLog_Add( JE_ERR_FILEIO_READ , "jeBody_ReadGeometry: Failed to version.");	return JE_FALSE; }
	if (u!=JE_BODY_FILE_VERSION)
		{	jeErrorLog_Add( JE_ERR_FILEIO_VERSION , "jeBody_ReadGeometry: old or wrong version");   return JE_FALSE; }
	

	if(jeVFile_Read(pFile, &(B->BoundingBoxMin), sizeof(B->BoundingBoxMin)) == JE_FALSE)
		{	jeErrorLog_Add( JE_ERR_FILEIO_READ , "jeBody_ReadGeometry.");	 return JE_FALSE; }

	if(jeVFile_Read(pFile, &(B->BoundingBoxMax), sizeof(B->BoundingBoxMax)) == JE_FALSE)
		{	jeErrorLog_Add( JE_ERR_FILEIO_READ , "jeBody_ReadGeometry.");	 return JE_FALSE; }

	if(jeVFile_Read(pFile, &(B->XSkinVertexCount), sizeof(B->XSkinVertexCount)) == JE_FALSE)
		{	jeErrorLog_Add( JE_ERR_FILEIO_READ , "jeBody_ReadGeometry.");	 return JE_FALSE; }

	if (B->XSkinVertexCount>0)
		{
			u = sizeof(jeBody_XSkinVertex) * B->XSkinVertexCount;
			B->XSkinVertexArray = (jeBody_XSkinVertex *)jeRam_Allocate(u);
			if (B->XSkinVertexArray == NULL)
				{	jeErrorLog_Add( JE_ERR_MEMORY_RESOURCE , "jeBody_ReadGeometry: Failed to allocate vertex array.");   return JE_FALSE;  }
			if(jeVFile_Read(pFile, B->XSkinVertexArray, u) == JE_FALSE)
				{	jeErrorLog_Add( JE_ERR_FILEIO_READ , "jeBody_ReadGeometry: skin vertex array");	 return JE_FALSE; }
		}

	if(jeVFile_Read(pFile, &(B->SkinNormalCount), sizeof(B->SkinNormalCount)) == JE_FALSE)
		{	jeErrorLog_Add( JE_ERR_FILEIO_READ , "jeBody_ReadGeometry.");	 return JE_FALSE; }

	if (B->SkinNormalCount>0)
		{
			u = sizeof(jeBody_Normal) * B->SkinNormalCount;
			B->SkinNormalArray = (jeBody_Normal *)jeRam_Allocate(u);
			if (B->SkinNormalArray == NULL)
				{	jeErrorLog_Add( JE_ERR_MEMORY_RESOURCE , "jeBody_ReadGeometry: Failed to allocate normal array.");   return JE_FALSE;  }
			if(jeVFile_Read(pFile, B->SkinNormalArray, u) == JE_FALSE)
				{	jeErrorLog_Add( JE_ERR_FILEIO_READ , "jeBody_ReadGeometry: skin normal array.");	return JE_FALSE; }
		}

	if(jeVFile_Read(pFile, &(B->blendDataCount), sizeof(B->blendDataCount)) == JE_FALSE)
		{	jeErrorLog_Add( JE_ERR_FILEIO_READ , "jeBody_ReadGeometry.");	 return JE_FALSE; }

	if (B->blendDataCount>0)
		{
			u = sizeof(jeBody_BlendData) * B->blendDataCount;
			B->blendDataArray = (jeBody_BlendData *)jeRam_Allocate(u);
			if (B->blendDataArray == NULL)
				{	jeErrorLog_Add( JE_ERR_MEMORY_RESOURCE , "jeBody_ReadGeometry: Failed to allocate blend array.");   return JE_FALSE;  }
			if(jeVFile_Read(pFile, B->blendDataArray, u) == JE_FALSE)
				{	jeErrorLog_Add( JE_ERR_FILEIO_READ , "jeBody_ReadGeometry.");	return JE_FALSE; }
		}

	if(jeVFile_Read(pFile, &(B->BoneCount), sizeof(B->BoneCount)) == JE_FALSE)
		{	jeErrorLog_Add( JE_ERR_FILEIO_READ , "jeBody_ReadGeometry.");  return JE_FALSE; }

	if (B->BoneCount>0)
		{
			u = sizeof(jeBody_Bone) * B->BoneCount;
			B->BoneArray = (jeBody_Bone *)jeRam_Allocate(u);
			if (B->BoneArray == NULL)
				{	jeErrorLog_Add( JE_ERR_MEMORY_RESOURCE , "jeBody_ReadGeometry: Failed to allocate bone array.");   return JE_FALSE;  }
			if(jeVFile_Read(pFile, B->BoneArray, u) == JE_FALSE)
				{	jeErrorLog_Add( JE_ERR_FILEIO_READ , "jeBody_ReadGeometry.");  return JE_FALSE; }
		}

	B->BoneNames = jeStrBlock_CreateFromFile(pFile);
	if (B->BoneNames==NULL)
		{	jeErrorLog_Add( JE_ERR_SUBSYSTEM_FAILURE , "jeBody_ReadGeometry."); 	 return JE_FALSE; }
	
	if(jeVFile_Read(pFile, &(B->MaterialCount), sizeof(B->MaterialCount)) == JE_FALSE)
		{	jeErrorLog_Add( JE_ERR_FILEIO_READ , "jeBody_ReadGeometry.");	 return JE_FALSE; }

	if (B->MaterialCount > 0)
	{
		// reserve mem for B->MaterialArray as per normal
		u = sizeof(jeBody_Material) * B->MaterialCount;
		B->MaterialArray = (jeBody_Material *)jeRam_Allocate(u);
		if (B->MaterialArray == NULL)
			{	jeErrorLog_Add( JE_ERR_MEMORY_RESOURCE , "jeBody_ReadGeometry: Failed to allocate material array");   return JE_FALSE;  }

		if(jeVFile_Read(pFile, B->MaterialArray, u) == JE_FALSE)
				{	jeErrorLog_Add( JE_ERR_FILEIO_READ , "jeBody_ReadGeometry: material array");	 return JE_FALSE; }
	}

	#if 1	// <>
	// CB added this because it seems the Bitmap pointer is
	//	read in with the Material array, and is later used as a boolean
	//	for "should this material have a texture"
	for(u=0;u<(uint32)B->MaterialCount;u++)
	{
		if ( B->MaterialArray[u].MatSpec )
			B->MaterialArray[u].MatSpec = (jeMaterialSpec *)1;
	}
	#endif
			
	B->MaterialNames = jeStrBlock_CreateFromFile(pFile);
	if ( B->MaterialNames == NULL )
		{	jeErrorLog_Add( JE_ERR_SUBSYSTEM_FAILURE, "jeBody_ReadGeometry."); 	 return JE_FALSE; }

	if(jeVFile_Read(pFile, &(B->LevelsOfDetail), sizeof(B->LevelsOfDetail)) == JE_FALSE)
		{	jeErrorLog_Add( JE_ERR_FILEIO_READ , "jeBody_ReadGeometry.");	 return JE_FALSE; }

	if (B->LevelsOfDetail > JE_BODY_NUMBER_OF_LOD)
		{	jeErrorLog_Add( JE_ERR_FILEIO_FORMAT , "jeBody_ReadGeometry.");	 return JE_FALSE; }

	for (i=0; i<B->LevelsOfDetail; i++)
		{
			if(jeVFile_Read(pFile, &(u), sizeof(u)) == JE_FALSE)
				{	jeErrorLog_Add( JE_ERR_FILEIO_READ , "jeBody_ReadGeometry.");	 return JE_FALSE; }
			B->SkinFaces[i].FaceCount = (jeBody_Index)u;
			
			if (u>0)
				{
					u = sizeof(jeBody_Triangle) * u;
					B->SkinFaces[i].FaceArray = (jeBody_Triangle *)jeRam_Allocate(u);
					if (B->SkinFaces[i].FaceArray == NULL)
						{	jeErrorLog_Add( JE_ERR_MEMORY_RESOURCE , "jeBody_ReadGeometry: Failed to allocate face array.");   return JE_FALSE;  }
					if(jeVFile_Read(pFile, B->SkinFaces[i].FaceArray, u) == JE_FALSE)
						{	jeErrorLog_Add( JE_ERR_FILEIO_READ , "jeBody_ReadGeometry.");	 return JE_FALSE; }
				}
		}

	assert( jeBody_IsValid(B) != JE_FALSE );
	return JE_TRUE;
}

JETAPI jeBody *JETCC jeBody_CreateFromFile(jeVFile *pFile)
{
	jeBody  *B = NULL;
	int i;

	jeVFile *VFile = NULL;
	jeVFile *SubFile = NULL;
	jeVFile *BitmapDirectory = NULL;
	
	assert( pFile != NULL );

	SubFile = NULL;
	BitmapDirectory = NULL;

	VFile = jeVFile_OpenNewSystem(pFile,JE_VFILE_TYPE_VIRTUAL, NULL, 
									NULL, JE_VFILE_OPEN_DIRECTORY | JE_VFILE_OPEN_READONLY);
	if (VFile == NULL)
	{
		jeErrorLog_Add( JE_ERR_FILEIO_READ , "jeBody_CreateFromFile: Failed to open subsystem.");
		goto CreateError;
	}
	
	SubFile = jeVFile_Open(VFile,JE_BODY_GEOMETRY_NAME,JE_VFILE_OPEN_READONLY);
	if (SubFile == NULL)
	{
		jeErrorLog_Add( JE_ERR_FILEIO_READ , "jeBody_CreateFromFile: Failed to open geometry subfile.");
		goto CreateError;
	}

	B = jeBody_CreateNull();
	if (B==NULL)
	{
		jeErrorLog_Add( JE_ERR_SUBSYSTEM_FAILURE , "jeBody_CreateFromFile: Failed to create empty body.");
		goto CreateError;
	}

	{
		jeVFile * LZFS;

		LZFS =  jeVFile_OpenNewSystem(SubFile,JE_VFILE_TYPE_LZ, NULL, NULL,JE_VFILE_OPEN_READONLY);
		if ( ! LZFS )
		{
			jeErrorLog_Add( JE_ERR_FILEIO_READ , "jeBody_CreateFromFile: Failed to open compressed subfile.");
			goto CreateError;
		}

		if ( ! jeBody_ReadGeometry(B,LZFS) )
		{
			jeErrorLog_Add( JE_ERR_SUBSYSTEM_FAILURE , "jeBody_CreateFromFile: Failed to read body geometry.");
			goto CreateError;
		}

		if ( ! jeVFile_Close(LZFS) )
		{
			jeErrorLog_Add( JE_ERR_FILEIO_CLOSE , "jeBody_CreateFromFile: Failed to close compressed subfile.");
			goto CreateError;
		}
	}

	if (!jeVFile_Close(SubFile))
	{
		jeErrorLog_Add( JE_ERR_FILEIO_CLOSE , "jeBody_CreateFromFile: Failed to close geometry subfile.");
		goto CreateError;
	}

	BitmapDirectory = jeVFile_Open(VFile,JE_BODY_BITMAP_DIRECTORY_NAME, 
									JE_VFILE_OPEN_DIRECTORY | JE_VFILE_OPEN_READONLY);
	if (BitmapDirectory == NULL)
	{
		jeErrorLog_Add( JE_ERR_FILEIO_READ , "jeBody_CreateFromFile: Failed to open bitmap subdirectory.");
		goto CreateError;
	}
	
	for (i=0; i<B->MaterialCount; i++)
	{
		jeBody_Material *M;
		M = &(B->MaterialArray[i]);

		if (M->MatSpec != NULL)
		{
			char FName[1000];
			sprintf(FName,"%d",i);
			
			M->MatSpec = NULL;

			SubFile = jeVFile_Open(BitmapDirectory,FName,JE_VFILE_OPEN_READONLY);
			if (SubFile == NULL)
			{
				jeErrorLog_AddString( JE_ERR_FILEIO_READ , "jeBody_CreateFromFile: Failed to open bitmap subfile:",FName);
				goto CreateError;
			}

			M->MatSpec = jeMaterialSpec_Create(jeResourceMgr_GetEngine(jeResourceMgr_GetSingleton()), jeResourceMgr_GetSingleton());
			if (M->MatSpec == NULL)
			{
				jeErrorLog_AddString( JE_ERR_SUBSYSTEM_FAILURE , "jeBody_CreateFromFile: Failed to read bitmap:",FName);
				goto CreateError;
			}
			jeMaterialSpec_AddLayerFromFile(M->MatSpec, 0, SubFile, JE_TRUE, 255);
			if (!jeVFile_Close(SubFile))
			{
				jeErrorLog_AddString( JE_ERR_FILEIO_CLOSE , "jeBody_CreateFromFile: Failed to close bitmap subfile:",FName);
				goto CreateError;
			}
		}
	}
	if (!jeVFile_Close(BitmapDirectory))
	{
		jeErrorLog_Add( JE_ERR_FILEIO_CLOSE , "jeBody_CreateFromFile: Failed to close bitmap directory.");
		goto CreateError;
	}
	if (!jeVFile_Close(VFile))
	{
		jeErrorLog_Add( JE_ERR_FILEIO_CLOSE , "jeBody_CreateFromFile: Failed to close body subsystem.");
		goto CreateError;
	}
	return B;

CreateError:
	jeBody_DestroyPossiblyIncompleteBody(&B);
	if (SubFile != NULL)
		jeVFile_Close(SubFile);
	if (BitmapDirectory != NULL)
		jeVFile_Close(BitmapDirectory);
	if (VFile != NULL)
		jeVFile_Close(VFile);
	return NULL;
}



JETAPI jeBoolean JETCC jeBody_WriteGeometry(const jeBody *B,jeVFile *pFile)
{
	uint32 u;
	int i;

	assert( B != NULL );
	assert( pFile != NULL );
	assert( jeBody_IsValid(B) != JE_FALSE );

	// Write the format flag
	u = JE_BODY_FILE_TYPE;
	if(jeVFile_Write(pFile, &u, sizeof(u)) == JE_FALSE)
		{	jeErrorLog_Add( JE_ERR_FILEIO_WRITE , "jeBody_WriteGeometry.");	return JE_FALSE; }

	// Write the version
	u = JE_BODY_FILE_VERSION;
	if(jeVFile_Write(pFile, &u, sizeof(u)) == JE_FALSE)
		{	jeErrorLog_Add( JE_ERR_FILEIO_WRITE , "jeBody_WriteGeometry.");	return JE_FALSE; }
	
	if(jeVFile_Write(pFile, &(B->BoundingBoxMin), sizeof(B->BoundingBoxMin)) == JE_FALSE)
		{	jeErrorLog_Add( JE_ERR_FILEIO_WRITE , "jeBody_WriteGeometry.");	return JE_FALSE; }

	if(jeVFile_Write(pFile, &(B->BoundingBoxMax), sizeof(B->BoundingBoxMax)) == JE_FALSE)
		{	jeErrorLog_Add( JE_ERR_FILEIO_WRITE , "jeBody_WriteGeometry.");	return JE_FALSE; }

	if(jeVFile_Write(pFile, &(B->XSkinVertexCount), sizeof(B->XSkinVertexCount)) == JE_FALSE)
		{	jeErrorLog_Add( JE_ERR_FILEIO_WRITE , "jeBody_WriteGeometry.");	return JE_FALSE; }

	assert( (B->XSkinVertexCount==0) || (B->XSkinVertexArray!=NULL));
	
	if (B->XSkinVertexCount>0)
		{
			u = sizeof(jeBody_XSkinVertex) * B->XSkinVertexCount;
			if(jeVFile_Write(pFile, B->XSkinVertexArray, u) == JE_FALSE)
				{	jeErrorLog_Add( JE_ERR_FILEIO_WRITE , "jeBody_WriteGeometry.");	return JE_FALSE; }
		}

	if(jeVFile_Write(pFile, &(B->SkinNormalCount), sizeof(B->SkinNormalCount)) == JE_FALSE)
		{	jeErrorLog_Add( JE_ERR_FILEIO_WRITE , "jeBody_WriteGeometry.");	return JE_FALSE; }

	if (B->SkinNormalCount>0)
		{
			u = sizeof(jeBody_Normal) * B->SkinNormalCount;
			if(jeVFile_Write(pFile, B->SkinNormalArray, u) == JE_FALSE)
				{	jeErrorLog_Add( JE_ERR_FILEIO_WRITE , "jeBody_WriteGeometry.");	return JE_FALSE; }
		}

	if(jeVFile_Write(pFile, &(B->blendDataCount), sizeof(B->blendDataCount)) == JE_FALSE)
		{	jeErrorLog_Add( JE_ERR_FILEIO_WRITE , "jeBody_WriteGeometry.");	return JE_FALSE; }

	if (B->blendDataCount>0)
		{
			u = sizeof(jeBody_BlendData) * B->blendDataCount;
			if(jeVFile_Write(pFile, B->blendDataArray, u) == JE_FALSE)
				{	jeErrorLog_Add( JE_ERR_FILEIO_WRITE , "jeBody_WriteGeometry.");	return JE_FALSE; }
		}

	if(jeVFile_Write(pFile, &(B->BoneCount), sizeof(B->BoneCount)) == JE_FALSE)
		{	jeErrorLog_Add( JE_ERR_FILEIO_WRITE , "jeBody_WriteGeometry.");	return JE_FALSE; }

	if (B->BoneCount>0)
		{
			u = sizeof(jeBody_Bone) * B->BoneCount;
			if(jeVFile_Write(pFile, B->BoneArray, u) == JE_FALSE)
				{	jeErrorLog_Add( JE_ERR_FILEIO_WRITE , "jeBody_WriteGeometry.");	return JE_FALSE; }
		}

	if (jeStrBlock_WriteToFile(B->BoneNames,pFile)==JE_FALSE)
		{	jeErrorLog_Add( JE_ERR_SUBSYSTEM_FAILURE , "jeBody_WriteGeometry."); 	return JE_FALSE; }
	
	if(jeVFile_Write(pFile, &(B->MaterialCount), sizeof(B->MaterialCount)) == JE_FALSE)
		{	jeErrorLog_Add( JE_ERR_FILEIO_WRITE , "jeBody_WriteGeometry.");	return JE_FALSE; }

	if (B->MaterialCount>0)
		{
			u = sizeof(jeBody_Material) * B->MaterialCount;
			if(jeVFile_Write(pFile, B->MaterialArray, u) == JE_FALSE)
				{	jeErrorLog_Add( JE_ERR_FILEIO_WRITE , "jeBody_WriteGeometry.");	return JE_FALSE; }
		}
	
	if (jeStrBlock_WriteToFile(B->MaterialNames,pFile)==JE_FALSE)
		{	jeErrorLog_Add( JE_ERR_SUBSYSTEM_FAILURE , "jeBody_WriteGeometry."); 	return JE_FALSE; }
	
	if(jeVFile_Write(pFile, &(B->LevelsOfDetail), sizeof(B->LevelsOfDetail)) == JE_FALSE)
		{	jeErrorLog_Add( JE_ERR_FILEIO_WRITE , "jeBody_WriteGeometry.");	return JE_FALSE; }

	for (i=0; i<B->LevelsOfDetail; i++)
		{
			u = B->SkinFaces[i].FaceCount;
			if(jeVFile_Write(pFile, &(u), sizeof(u)) == JE_FALSE)
				{	jeErrorLog_Add( JE_ERR_FILEIO_WRITE , "jeBody_WriteGeometry.");	return JE_FALSE; }
			if (u>0)
				{
					u = sizeof(jeBody_Triangle) * u;
					if(jeVFile_Write(pFile, B->SkinFaces[i].FaceArray, u) == JE_FALSE)
						{	jeErrorLog_Add( JE_ERR_FILEIO_WRITE , "jeBody_WriteGeometry.");	return JE_FALSE; }
				}
		}
	return JE_TRUE;
}


JETAPI jeBoolean JETCC jeBody_WriteToFile(const jeBody *B, jeVFile *pFile)
{
	int i;
	jeVFile *VFile;
	jeVFile *SubFile;
	jeVFile *BitmapDirectory;

	assert( jeBody_IsValid(B) != JE_FALSE );
	assert( pFile != NULL );

	VFile = jeVFile_OpenNewSystem(pFile,JE_VFILE_TYPE_VIRTUAL, NULL, 
									NULL, JE_VFILE_OPEN_DIRECTORY | JE_VFILE_OPEN_CREATE);
	if (VFile == NULL)
		{	jeErrorLog_Add( JE_ERR_FILEIO_OPEN , "jeBody_WriteToFile: Failed to open body subsystem.");	goto WriteError;}
	
	SubFile = jeVFile_Open(VFile,JE_BODY_GEOMETRY_NAME,JE_VFILE_OPEN_CREATE);
	if (SubFile == NULL)
		{	jeErrorLog_Add( JE_ERR_FILEIO_OPEN , "jeBody_WriteToFile: Failed to open subfile.");	goto WriteError;}

	{
	jeVFile * LZFS;

	LZFS = jeVFile_OpenNewSystem(SubFile,JE_VFILE_TYPE_LZ, NULL, NULL, JE_VFILE_OPEN_CREATE);
	if ( ! LZFS )
		{	jeErrorLog_Add( JE_ERR_FILEIO_OPEN , "jeBody_WriteToFile: Failed to open compressed file.");	goto WriteError;}

	if ( ! jeBody_WriteGeometry(B,LZFS) )
		{	jeErrorLog_Add( JE_ERR_SUBSYSTEM_FAILURE , "jeBody_WriteToFile: Failed to write body geometry.");	goto WriteError;}

	Log_Printf("Actor : Body : Geometry : ");
	if ( ! jeVFile_Close(LZFS) )
		{	jeErrorLog_Add( JE_ERR_FILEIO_CLOSE , "jeBody_WriteToFile: Failed to close compressed file.");	goto WriteError;}

	}

	if (jeVFile_Close(SubFile)==JE_FALSE)
		{	jeErrorLog_Add( JE_ERR_FILEIO_CLOSE , "jeBody_WriteToFile: Failed to close subfile.");	goto WriteError;}
		
	BitmapDirectory = jeVFile_Open(VFile,JE_BODY_BITMAP_DIRECTORY_NAME, 
									JE_VFILE_OPEN_DIRECTORY | JE_VFILE_OPEN_CREATE);
	if (BitmapDirectory == NULL)
		{	jeErrorLog_Add( JE_ERR_FILEIO_OPEN , "jeBody_WriteToFile: Failed to open bitmap subdir.");	goto WriteError;}
	
	for (i=0; i<B->MaterialCount; i++)
	{
		jeBody_Material *M;
		M = &(B->MaterialArray[i]);

		if (M->MatSpec != NULL)
		{
			char FName[1000];
			sprintf(FName,"%d",i);

			SubFile = jeVFile_Open(BitmapDirectory,FName,JE_VFILE_OPEN_CREATE);
			if (SubFile == NULL)
			{
				jeErrorLog_AddString( JE_ERR_FILEIO_OPEN , "jeBody_WriteToFile: Failed to open bitmap file:",FName);
				goto WriteError;
			}

			if (jeMaterialSpec_WriteToFile(M->MatSpec, SubFile)==JE_FALSE)
			{
				jeErrorLog_AddString( JE_ERR_FILEIO_WRITE , "jeBody_WriteToFile: Failed to write bitmap:",FName);
				goto WriteError;
			}
					
			if (jeVFile_Close(SubFile)==JE_FALSE)
			{
				jeErrorLog_AddString( JE_ERR_FILEIO_CLOSE , "jeBody_WriteToFile: Failed to close bitmap:",FName);
				goto WriteError;
			}
		}
	}
	if (jeVFile_Close(BitmapDirectory)==JE_FALSE)
		{	jeErrorLog_Add( JE_ERR_FILEIO_CLOSE , "jeBody_WriteToFile: Failed to close bitmap subdir.");	goto WriteError;}
	if (jeVFile_Close(VFile)==JE_FALSE)
		{	jeErrorLog_Add( JE_ERR_FILEIO_CLOSE , "jeBody_WriteToFile: Failed to close body subsystem.");	goto WriteError;}
	
	return JE_TRUE;
	WriteError:
		return JE_FALSE;
}

///////////////////////////////////////////////////////////////////////////////////////
// exposed geometry APIs

JETAPI int JETCC jeBody_GetIndexedBoneVertexCount(const jeBody* pBody, int boneIndex)
{
	int i, n;

	assert(pBody);

	if (boneIndex < 0 || boneIndex >= pBody->BoneCount)
		return 0;

	for (n = 0, i = 0; i < pBody->XSkinVertexCount; i ++)
	{
		if (pBody->XSkinVertexArray[i].BoneIndex == (jeBody_Index)boneIndex)
		{
			n ++;
		}
	}

	return n;
}

JETAPI int JETCC jeBody_GetNamedBoneVertexCount(const jeBody* pBody, const char* pBoneName)
{
	int i, n;
	int boneIndex;

	assert(pBody);
	assert(pBoneName);

	if (! jeStrBlock_FindString(pBody->MaterialNames, pBoneName, &boneIndex))
		return JE_FALSE;

	for (n = 0, i = 0; i < pBody->XSkinVertexCount; i ++)
	{
		if (pBody->XSkinVertexArray[i].BoneIndex == (jeBody_Index)boneIndex)
		{
			n ++;
		}
	}

	return n;
}

// local space functions

JETAPI jeBoolean JETCC jeBody_GetIndexedBoneVertexLocations(const jeBody* pBody, int boneIndex, int aSize,
	jeVec3d* pVerts)
{
	int n;
	jeBody_Index i;

	assert(pBody);
	assert(pVerts);

	if (boneIndex < 0 || boneIndex >= pBody->BoneCount)
		return JE_FALSE;

	for (n = 0, i = 0; i < pBody->XSkinVertexCount; i ++)
	{
		if (pBody->XSkinVertexArray[i].BoneIndex == (jeBody_Index)boneIndex)
		{
			if (n == aSize)
				return JE_FALSE;

			pVerts[n].X = pBody->XSkinVertexArray[i].XPoint.X;
			pVerts[n].Y = pBody->XSkinVertexArray[i].XPoint.Y;
			pVerts[n].Z = pBody->XSkinVertexArray[i].XPoint.Z;

			n ++;
		}
	}

	return JE_TRUE;
}

JETAPI jeBoolean JETCC jeBody_GetNamedBoneVertexLocations(const jeBody* pBody, const char* pBoneName, int aSize,
	jeVec3d* pVerts)
{
	int n, boneIndex;
	jeBody_Index i;

	assert(pBody);
	assert(pBoneName);
	assert(pVerts);

	if (! jeStrBlock_FindString(pBody->MaterialNames, pBoneName, &boneIndex))
		return JE_FALSE;

	for (n = 0, i = 0; i < pBody->XSkinVertexCount; i ++)
	{
		if (pBody->XSkinVertexArray[i].BoneIndex == (jeBody_Index)boneIndex)
		{
			if (n == aSize)
				return JE_FALSE;

			pVerts[n].X = pBody->XSkinVertexArray[i].XPoint.X;
			pVerts[n].Y = pBody->XSkinVertexArray[i].XPoint.Y;
			pVerts[n].Z = pBody->XSkinVertexArray[i].XPoint.Z;

			n ++;
		}
	}

	return JE_TRUE;
}
