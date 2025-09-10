/****************************************************************************************/
/*  BODYINST.C                                                                          */
/*                                                                                      */
/*  Author: Mike Sandige	                                                            */
/*  Description: Actor body instance implementation.                                    */
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

#include "BODY._H"
#include "BodyInst.h"
#include "Ram.h"
#include "Errorlog.h"
#include "StrBlock.h"

#include "Camera._h"


typedef struct jeBodyInst
{
	const jeBody			*BodyTemplate;
	jeBodyInst_Geometry		 ExportGeometry;
	int						 LastLevelOfDetail;
	jeBodyInst_Index		 FaceCount;
} jeBodyInst;




void JETCF jeBodyInst_PostScale(const jeXForm3d *M,const jeVec3d *S,jeXForm3d *Scaled)
{
	Scaled->AX = M->AX * S->X;
	Scaled->BX = M->BX * S->X;
	Scaled->CX = M->CX * S->X;

	Scaled->AY = M->AY * S->Y;
	Scaled->BY = M->BY * S->Y;
	Scaled->CY = M->CY * S->Y;

	Scaled->AZ = M->AZ * S->Z;
	Scaled->BZ = M->BZ * S->Z;
	Scaled->CZ = M->CZ * S->Z;
	Scaled->Translation = M->Translation;
}


jeBodyInst *JETCF jeBodyInst_Create(const jeBody *B)
{
	jeBodyInst *BI;
	assert( B != NULL );
	assert( jeBody_IsValid(B) != JE_FALSE );
	
	BI = JE_RAM_ALLOCATE_STRUCT_CLEAR(jeBodyInst);
	if (BI == NULL)
		{
			jeErrorLog_Add(JE_ERR_MEMORY_RESOURCE, "jeBodyInst_Create.");
			return NULL;
		}
	BI->BodyTemplate = B;
	{
		jeBodyInst_Geometry *G = &(BI->ExportGeometry);
		G->SkinVertexCount =0;
		G->SkinVertexArray = NULL;
		
		G->NormalCount = 0;
		G->NormalArray = NULL;
		
		G->FaceCount = (jeBody_Index) 0;
		G->FaceListSize = 0; 
		G->FaceList = NULL;
	}

	BI->LastLevelOfDetail   = -1;
	BI->FaceCount =  0;

	return BI;
}
			

void JETCF jeBodyInst_Destroy( jeBodyInst **BI)
{
	jeBodyInst_Geometry *G;
	assert( BI != NULL );
	assert( *BI != NULL );
	G = &( (*BI)->ExportGeometry );
	if (G->SkinVertexArray != NULL )
		{
			jeRam_Free( G->SkinVertexArray );
			G->SkinVertexArray = NULL;
		}
	if (G->NormalArray != NULL )
		{
			jeRam_Free( G->NormalArray );
			G->NormalArray = NULL;
		}
	if (G->FaceList != NULL )
		{
			jeRam_Free( G->FaceList );
			G->FaceList = NULL;
		}
	jeRam_Free( *BI );
	*BI = NULL;
}



#define JE_BODYINST_FACELIST_SIZE_FOR_TRIANGLE (8)

static jeBodyInst_Geometry * JETCF jeBodyInst_GetGeometryPrep(	
	jeBodyInst *BI, 
	int LevelOfDetail)
{
	const jeBody *B;
	jeBodyInst_Geometry *G;
	LevelOfDetail;		// unused param
	
	assert( BI != NULL );
	assert( jeBody_IsValid(BI->BodyTemplate) != JE_FALSE );
	B = BI->BodyTemplate;

	G = &(BI->ExportGeometry);
	assert( G  != NULL );

	if (G->SkinVertexCount != B->XSkinVertexCount)
		{
			if (G->SkinVertexArray!=NULL)
				{
					jeRam_Free(G->SkinVertexArray);
				}
			G->SkinVertexArray = JE_RAM_ALLOCATE_ARRAY_CLEAR(jeBodyInst_SkinVertex,B->XSkinVertexCount);
			if ( G->SkinVertexArray == NULL )
				{
					jeErrorLog_Add(JE_ERR_MEMORY_RESOURCE, "jeBodyInst_GetGeometryPrep.");
					G->SkinVertexCount = 0;
					return NULL;
				}
			G->SkinVertexCount  = B->XSkinVertexCount;
		}

	if (G->NormalCount != B->SkinNormalCount)
		{
			if (G->NormalArray!=NULL)
				{
					jeRam_Free(G->NormalArray);
				}
			G->NormalArray = JE_RAM_ALLOCATE_ARRAY_CLEAR( jeVec3d,B->SkinNormalCount);
			if ( G->NormalArray == NULL )
				{
					jeErrorLog_Add(JE_ERR_MEMORY_RESOURCE, "jeBodyInst_GetGeometryPrep.");
					G->NormalCount = 0;
					return NULL;
				}
			G->NormalCount  = B->SkinNormalCount;
		}

	if (BI->FaceCount != B->SkinFaces[JE_BODY_HIGHEST_LOD].FaceCount)
		{
			if (G->FaceList!=NULL)
				{
					jeRam_Free(G->FaceList);
				}
			G->FaceListSize = sizeof(jeBody_Index) * 
					B->SkinFaces[JE_BODY_HIGHEST_LOD].FaceCount * 
					JE_BODYINST_FACELIST_SIZE_FOR_TRIANGLE;
			G->FaceList = JE_RAM_ALLOCATE_ARRAY_CLEAR(jeBody_Index,
							B->SkinFaces[JE_BODY_HIGHEST_LOD].FaceCount * 
							JE_BODYINST_FACELIST_SIZE_FOR_TRIANGLE);
			if ( G->FaceList == NULL )
				{
					jeErrorLog_Add(JE_ERR_MEMORY_RESOURCE, "jeBodyInst_GetGeometryPrep.");
					BI->FaceCount = 0;
					return NULL;
				}
			BI->FaceCount = B->SkinFaces[JE_BODY_HIGHEST_LOD].FaceCount;
		}
	return G;
}

const jeBodyInst_Geometry * JETCF jeBodyInst_GetGeometry(
	const jeBodyInst *BI, 
	const jeVec3d *ScaleVector,
	const jeXFArray *BoneTransformArray,
	int LevelOfDetail,
	const jeCamera *Camera)
{
	jeBodyInst_Geometry *G;
	const jeBody *B;
	jeXForm3d *BoneXFArray;
	int      BoneXFCount;
	jeBody_Index BoneIndex;

	assert( BI != NULL );
	assert( BoneTransformArray != NULL );
	assert( jeBody_IsValid(BI->BodyTemplate) != JE_FALSE );
	
	G = jeBodyInst_GetGeometryPrep((jeBodyInst *)BI,LevelOfDetail);
	if (G == NULL)
		{
			jeErrorLog_Add(JE_ERR_SUBSYSTEM_FAILURE,"jeBodyInst_GetGeometry.");
			return NULL;
		}
		

	B = BI->BodyTemplate;

	BoneXFArray = jeXFArray_GetElements(BoneTransformArray,&BoneXFCount);
	if ( BoneXFArray == NULL)
		{
			jeErrorLog_Add(JE_ERR_SUBSYSTEM_FAILURE,"jeBodyInst_GetGeometry.");
			return NULL;
		}
	if (BoneXFCount != B->BoneCount)
		{	
			jeErrorLog_Add(JE_ERR_SUBSYSTEM_FAILURE,"jeBodyInst_GetGeometry.");
			return NULL;
		}


	{	
		int i,LevelOfDetailBit;
	
		if (Camera != NULL)
		{
			// transform and project all appropriate points
			jeBody_XSkinVertex *S;
			jeBodyInst_SkinVertex  *D;
			LevelOfDetailBit = 1 << LevelOfDetail;
			BoneIndex = -1;  // S->BoneIndex won't ever be this.
			jeVec3d_Set(&(G->Maxs), -JE_BODY_REALLY_BIG_NUMBER, -JE_BODY_REALLY_BIG_NUMBER, -JE_BODY_REALLY_BIG_NUMBER );
			jeVec3d_Set(&(G->Mins), JE_BODY_REALLY_BIG_NUMBER, JE_BODY_REALLY_BIG_NUMBER, JE_BODY_REALLY_BIG_NUMBER );
			for (i=B->XSkinVertexCount,S=B->XSkinVertexArray,D=G->SkinVertexArray; 
				 i>0; 
				 i--,S++,D++)
				{
					jeXForm3d ObjectToCamera;
					if (S->BoneIndex!=BoneIndex)
						{ //Keep XSkinVertexArray sorted by BoneIndex for best performance
							BoneIndex = S->BoneIndex;
							jeXForm3d_Multiply(		jeCamera_XForm(Camera), 
													&(BoneXFArray[BoneIndex]),
													&ObjectToCamera);
							jeBodyInst_PostScale(&ObjectToCamera,ScaleVector,&ObjectToCamera);
						}
					if ( S->LevelOfDetailMask && LevelOfDetailBit )
						{
							jeVec3d *VecDestPtr = &(D->SVPoint);
// @@@
							if (S->nBlends == 0)
							{
								jeXForm3d_Transform(  &(ObjectToCamera), &(S->XPoint),VecDestPtr);
							}

							else // we need to do some blending
							{
								int iblend;
								jeVec3d worldLoc, blendLoc;

								jeVec3d_Clear(&blendLoc);

								for (iblend = S->bdaOffset; 
									iblend < (S->nBlends + S->bdaOffset); iblend ++)
								{
									jeVec3d ScaledPoint;
									jeBody_BlendData* pBD = &B->blendDataArray[iblend];

									assert(pBD != NULL);

									ScaledPoint.X = pBD->XPoint.X * ScaleVector->X;
									ScaledPoint.Y = pBD->XPoint.Y * ScaleVector->Y;
									ScaledPoint.Z = pBD->XPoint.Z * ScaleVector->Z;

									// get world space loc of individual blend vert
									jeXForm3d_Transform(&BoneXFArray[pBD->boneIndex],
										//&pBD->XPoint, &worldLoc);
										&ScaledPoint,&worldLoc);

									// add weighted worldLoc to blendLoc

									blendLoc.X += pBD->weight * worldLoc.X;
									blendLoc.Y += pBD->weight * worldLoc.Y;
									blendLoc.Z += pBD->weight * worldLoc.Z;
								}

								// now do the world to camera space xform

								jeXForm3d_Transform(jeCamera_XForm(Camera), &blendLoc, VecDestPtr);
							}


							#ifdef ONE_OVER_Z_PIPELINE
							jeCamera_ProjectZ( Camera, VecDestPtr, VecDestPtr);
							#else
							jeCamera_Project( Camera, VecDestPtr, VecDestPtr);
							#endif
							D->SVU = S->XU;
							D->SVV = S->XV;

							D->SVW = S->XPoint; // -JFW

							if (VecDestPtr->X > G->Maxs.X ) G->Maxs.X = VecDestPtr->X;
							if (VecDestPtr->X < G->Mins.X ) G->Mins.X = VecDestPtr->X;
							if (VecDestPtr->Y > G->Maxs.Y ) G->Maxs.Y = VecDestPtr->Y;
							if (VecDestPtr->Y < G->Mins.Y ) G->Mins.Y = VecDestPtr->Y;
							if (VecDestPtr->Z > G->Maxs.Z ) G->Maxs.Z = VecDestPtr->Z;
							if (VecDestPtr->Z < G->Mins.Z ) G->Mins.Z = VecDestPtr->Z;
							D->ReferenceBoneIndex = BoneIndex;
						}
				}
		} // camera != NULL

// @@ NEED TO ADD BLEND CODE HERE
#pragma message("blend doesn't work for a NULL camera")
		else  // camera is NULL
		{
			// transform all appropriate points
			jeBody_XSkinVertex *S;
			jeBodyInst_SkinVertex  *D;
			LevelOfDetailBit = 1 << LevelOfDetail;
			BoneIndex = -1;  // S->BoneIndex won't ever be this.
			jeVec3d_Set(&(G->Maxs), -JE_BODY_REALLY_BIG_NUMBER, -JE_BODY_REALLY_BIG_NUMBER, -JE_BODY_REALLY_BIG_NUMBER );
			jeVec3d_Set(&(G->Mins), JE_BODY_REALLY_BIG_NUMBER, JE_BODY_REALLY_BIG_NUMBER, JE_BODY_REALLY_BIG_NUMBER );
			
			for (i=B->XSkinVertexCount,S=B->XSkinVertexArray,D=G->SkinVertexArray; 
				 i>0; 
				 i--,S++,D++)
				{
					jeXForm3d ObjectToWorld;
					if (S->BoneIndex!=BoneIndex)
						{ //Keep XSkinVertexArray sorted by BoneIndex for best performance
							BoneIndex = S->BoneIndex;
							jeBodyInst_PostScale(&BoneXFArray[BoneIndex],ScaleVector,&ObjectToWorld);

						}
					if ( S->LevelOfDetailMask && LevelOfDetailBit )
						{
							jeVec3d *VecDestPtr = &(D->SVPoint);
							jeXForm3d_Transform(  &(ObjectToWorld),
												&(S->XPoint),VecDestPtr);
							D->SVU = S->XU;
							D->SVV = S->XV;

							D->SVW = S->XPoint; // -JFW

							if (VecDestPtr->X > G->Maxs.X ) G->Maxs.X = VecDestPtr->X;
							if (VecDestPtr->X < G->Mins.X ) G->Mins.X = VecDestPtr->X;
							if (VecDestPtr->Y > G->Maxs.Y ) G->Maxs.Y = VecDestPtr->Y;
							if (VecDestPtr->Y < G->Mins.Y ) G->Mins.Y = VecDestPtr->Y;
							if (VecDestPtr->Z > G->Maxs.Z ) G->Maxs.Z = VecDestPtr->Z;
							if (VecDestPtr->Z < G->Mins.Z ) G->Mins.Z = VecDestPtr->Z;
							D->ReferenceBoneIndex = BoneIndex;
						}
				}
		} // camera is NULL

			{
				jeBody_Normal *S;
				jeVec3d *D;
				// rotate all appropriate normals
				for (i=B->SkinNormalCount,S=B->SkinNormalArray,D=G->NormalArray;
					 i>0; 
					 i--,S++,D++)
				{
					if ( S->LevelOfDetailMask && LevelOfDetailBit )
					{
						if (S->nBlends == 0)
						{
							jeXForm3d_Rotate(&(BoneXFArray[S->BoneIndex]),
								&(S->Normal),D);
						}
						
						else
						{
							int iblend;
							jeVec3d xNormal;

							jeVec3d_Clear(D);

							for (iblend = S->bdaOffset; 
								iblend < (S->nBlends + S->bdaOffset); iblend ++)
							{
								jeBody_BlendData* pBD = &B->blendDataArray[iblend];

								assert(pBD != NULL);

								// transform normal into world space
								jeXForm3d_Rotate(&BoneXFArray[pBD->boneIndex],
									&pBD->Normal, &xNormal);

								// add weighted normal to blendNormal

								D->X += pBD->weight * xNormal.X;
								D->Y += pBD->weight * xNormal.Y;
								D->Z += pBD->weight * xNormal.Z;
							}

							jeVec3d_Normalize(D);							
						}
						
					}
				}
			}

	}


	if (LevelOfDetail != BI->LastLevelOfDetail)
	{
		// build face list to export
		int i,j;
		jeBody_Index Count;
		const jeBody_Triangle *T;
		jeBody_Index *D;
		Count = B->SkinFaces[LevelOfDetail].FaceCount;

		for (i=0,T=B->SkinFaces[LevelOfDetail].FaceArray,D=G->FaceList;
				i<Count; 
				i++,T++,B++)
			{
				*D = JE_BODYINST_FACE_TRIANGLE;
				D++;
				*D = T->MaterialIndex;
				D++;
				for (j=0; j<3; j++)
					{
						*D = T->VtxIndex[j];
						D++;
						*D = T->NormalIndex[j];
						D++;
					}
			}
		assert( ((uint32)D) - ((uint32)G->FaceList) == (uint32)(G->FaceListSize) );
		G->FaceCount = Count;
		((jeBodyInst *)BI)->LastLevelOfDetail = LevelOfDetail;
	}



	return G;
}	
