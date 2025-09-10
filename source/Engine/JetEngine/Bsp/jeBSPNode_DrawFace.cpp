/****************************************************************************************/
/*  JEBSPNODE_DRAWFACE.C                                                                */
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
#include <stdio.h>
#include <assert.h>

#include "jeBSP._h"
#include "Dcommon.h"
#include "Camera.h"
#include "jeFrustum.h"
#include "jeIndexPoly.h"
#include "jeFaceInfo.h"
#include "jeMaterial.h"

#include "Bitmap._h"
#include "Ram.h"


//=======================================================================================
//	jeBSPNode_DrawFaceCreate
//=======================================================================================
jeBSPNode_DrawFace *jeBSPNode_DrawFaceCreate(jeBSP *BSP)
{
	jeBSPNode_DrawFace		*Face;

#ifdef DRAWFACE_USE_JE_RAM
	Face = JE_RAM_ALLOCATE_STRUCT(jeBSPNode_DrawFace);
#else
	Face = (jeBSPNode_DrawFace *)jeArray_GetNewElement(BSP->DrawFaceArray);
#endif

	if (!Face)
		return NULL;

	ZeroMem(Face);

	Face->FaceInfoIndex = JE_FACEINFO_ARRAY_NULL_INDEX;
	Face->TexVecIndex = JE_TEXVEC_ARRAY_NULL_INDEX;

	Face->BSP = BSP;

	return Face;
}

//=======================================================================================
//	jeBSPNode_DrawFaceDestroy
//=======================================================================================
void jeBSPNode_DrawFaceDestroy(jeBSPNode_DrawFace **Face, jeBSP *BSP)
{
	jeBSPNode_DrawFace		*DFace;

	assert(Face);
	assert(*Face);

	DFace = *Face;

	if (DFace->PortalObject)
		jeObject_Destroy(&DFace->PortalObject);

	if (DFace->PortalXForm)
	{
		jeRam_Free(DFace->PortalXForm);
		DFace->PortalXForm = NULL;
	}

	if (DFace->FaceInfoIndex != JE_FACEINFO_ARRAY_NULL_INDEX)
		jeFaceInfo_ArrayRemoveFaceInfo(BSP->FaceInfoArray, &DFace->FaceInfoIndex);

	if (DFace->PlaneIndex != JE_PLANEARRAY_NULL_INDEX)
		jePlaneArray_RemovePlane(BSP->PlaneArray, &DFace->PlaneIndex);

	if (DFace->TexVecIndex != JE_TEXVEC_ARRAY_NULL_INDEX)
		jeTexVec_ArrayRemoveTexVec(BSP->TexVecArray, &DFace->TexVecIndex);

	if (DFace->Poly)
	{
		int32		v;

		// Remove all the verts from the global vertarray 
		for (v=0; v< DFace->Poly->NumVerts; v++)
			jeVertArray_RemoveVert(BSP->VertArray, &DFace->Poly->Verts[v]);

		jeIndexPoly_Destroy(&DFace->Poly);
	}

	if (DFace->TVerts)
	{
		jeRam_Free(DFace->TVerts);
		DFace->TVerts = NULL;
	}

	// BEGIN - Hardware T&L - paradoxnj 4/7/2005
	//if (DFace->Lightmap)
	//	jeBSPNode_LightmapDestroy(&DFace->Lightmap, BSP);
	// END - Hardware T&L - paradoxnj 4/7/2005

#ifdef DRAWFACE_USE_JE_RAM
	jeRam_Free(*Face);
#else
	{
		jeBoolean	Ret;
		Ret = jeArray_FreeElement(BSP->DrawFaceArray, *Face);
		assert(Ret == JE_TRUE);
	}
#endif

	*Face = NULL;
}

//=======================================================================================
//	jeBSPNode_DrawFaceSetFaceInfoIndex
//=======================================================================================
jeBoolean jeBSPNode_DrawFaceSetFaceInfoIndex(jeBSPNode_DrawFace *DFace, jeBSP *BSP, jeFaceInfo_ArrayIndex Index)
{
	assert(DFace);
	assert(Index != JE_FACEINFO_ARRAY_NULL_INDEX);

	if (DFace->FaceInfoIndex != JE_FACEINFO_ARRAY_NULL_INDEX)
		jeFaceInfo_ArrayRemoveFaceInfo(BSP->FaceInfoArray, &DFace->FaceInfoIndex);

	if (!jeFaceInfo_ArrayRefFaceInfoIndex(BSP->FaceInfoArray, Index))
		return JE_FALSE;

	DFace->FaceInfoIndex = Index;
	
	return JE_TRUE;
}

typedef struct 
{
	const jePlane		*Plane;
	const jeXForm3d		*FaceXForm;
	jeWorld				*World;
	jeCamera			*Camera;
	jeFrustum			*Frustum;
} PortalMsgData;

//=======================================================================================
//	jeBSPNode_DrawFaceCreateUVInfo
//=======================================================================================
jeBoolean jeBSPNode_DrawFaceCreateUVInfo(jeBSPNode_DrawFace *Face, jeBSP *BSP)
{
	const jeTexVec		*pTexVec;
	jeTexVert			*pTVert;
	int32				v;

	if (Face->TVerts)
		jeRam_Free(Face->TVerts);

	Face->TVerts = JE_RAM_ALLOCATE_ARRAY(jeTexVert, Face->Poly->NumVerts);

	if (!Face->TVerts)	
		return JE_FALSE;

	// Grab the locked texture vectors for uv calculations
	pTexVec = jeTexVec_ArrayGetTexVecByIndex(BSP->TexVecArray, Face->TexVecIndex);
	assert(pTexVec);

	for (pTVert = Face->TVerts, v=0; v< Face->Poly->NumVerts; v++, pTVert++)
	{
		const jeVec3d	*pVert;

		pVert = jeVertArray_GetVertByIndex(BSP->VertArray, Face->Poly->Verts[v]);
		assert(pVert);

		// Get the U,V's by projecting the vert onto the texture axis that for this face
		pTVert->u = jeVec3d_DotProduct(pVert, &pTexVec->VecU);
		pTVert->v = jeVec3d_DotProduct(pVert, &pTexVec->VecV);
	}

	// Adjust the uv's as close to the origin as possible without effecting their appearance
	#if 1
	{
		jeFloat				ShiftU, ShiftV;
		int32				Width, Height;
		const jeMaterial	*pMaterial;
		const jeBitmap		*pBitmap;
#ifndef _USE_BITMAPS
		const jeMaterialSpec *pMatSpec;
		const jeTexture*	pTexture;
#endif
		const jeFaceInfo	*pFaceInfo;

		pFaceInfo = jeFaceInfo_ArrayGetFaceInfoByIndex(BSP->FaceInfoArray, Face->FaceInfoIndex);
		assert(pFaceInfo);
		pMaterial = jeMaterial_ArrayGetMaterialByIndex(BSP->MaterialArray, pFaceInfo->MaterialIndex);
#ifdef _USE_BITMAPS
		pBitmap = jeMaterial_GetBitmap(pMaterial);

		if (pBitmap)
		{
			Width = jeBitmap_Width(pBitmap);
			Height = jeBitmap_Width(pBitmap);

			ShiftU = (jeFloat)(((int32)(Face->TVerts[0].u / (jeFloat)Width))*Width);
			ShiftV = (jeFloat)(((int32)(Face->TVerts[0].v / (jeFloat)Height))*Height);

			ShiftU *= (pFaceInfo->DrawScaleU/pFaceInfo->LMapScaleU);
			ShiftV *= (pFaceInfo->DrawScaleV/pFaceInfo->LMapScaleV);

			Face->FixShiftU = ShiftU;
			Face->FixShiftV = ShiftV;

			for (pTVert = Face->TVerts, v=0; v< Face->Poly->NumVerts; v++, pTVert++)
			{
				pTVert->u -= ShiftU;
				pTVert->v -= ShiftV;
			}
		}
#else
		pMatSpec = jeMaterial_GetMaterialSpec(pMaterial);

		if (pMatSpec)
		{
			pTexture = jeMaterialSpec_GetLayerTexture(pMatSpec, 0);
			if (pTexture) {
			} else { // Keep backward compatibility
				pBitmap = jeMaterialSpec_GetLayerBitmap(pMatSpec, 0);
				if (pBitmap) {
					Width = jeBitmap_Width(pBitmap);
					Height = jeBitmap_Width(pBitmap);

					ShiftU = (jeFloat)(((int32)(Face->TVerts[0].u / (jeFloat)Width))*Width);
					ShiftV = (jeFloat)(((int32)(Face->TVerts[0].v / (jeFloat)Height))*Height);

					ShiftU *= (pFaceInfo->DrawScaleU/pFaceInfo->LMapScaleU);
					ShiftV *= (pFaceInfo->DrawScaleV/pFaceInfo->LMapScaleV);

					Face->FixShiftU = ShiftU;
					Face->FixShiftV = ShiftV;

					for (pTVert = Face->TVerts, v=0; v< Face->Poly->NumVerts; v++, pTVert++)
					{
						pTVert->u -= ShiftU;
						pTVert->v -= ShiftV;
					}
				}
			}
		}
#endif
	}
	#endif
	
	return JE_TRUE;
}

extern jeBoolean h_LeftHanded;

//=======================================================================================
//	jeBSPNode_DrawFaceRender
//=======================================================================================
void jeBSPNode_DrawFaceRender(const jeBSPNode_DrawFace *Face, jeBSP *BSP, jeBSPNode_SceneInfo *SceneInfo, uint32 ClipFlags)
{
	jeIndexPoly			*Poly;
	jeVertArray_Index	*pIVert;
	jeLVertex			LVerts1[MAX_TEMP_VERTS], LVerts2[MAX_TEMP_VERTS];
	jeLVertex			*pLVert;
	jeTLVertex			TLVerts[MAX_TEMP_VERTS];
	int32				i, NumVerts1;
	jeTexVert			*pTVert;
	jeFrustum_LClipInfo	ClipInfo;
	const jeFaceInfo	*pFaceInfo;
	const jeMaterial	*pMaterial;
	const jeBitmap		*pBitmap;
	uint32				Flags;
#ifndef _USE_BITMAPS
	const jeMaterialSpec*		pMatSpec;
//	jeTexture*			pTexture;
#endif

	assert(Face);
	assert(BSP);
	assert(SceneInfo);

	g_WorldDebugInfo.NumTransformedPolys++;

	// Get the poly pointer, and num verts to start with
	Poly = Face->Poly;
	NumVerts1 = Poly->NumVerts;

	assert(NumVerts1+4 < MAX_TEMP_VERTS);

	// Copy the verts into a nice linear array
	pLVert = LVerts1;				// Fill in this array with x,y,z,u,v,r,g,b...
	pIVert = Poly->Verts;			// index in to VertArray
	pTVert = Face->TVerts;			// Source u,v,r,g,b

	// Copy the index verts, and the uvrgb's into a jeLVertex structure
	for (i=0; i<NumVerts1; i++)
	{
		const jeVec3d	*pSrcVert;

		// FIXME:  Get rid of this crap-shit, and look directly into the array
		pSrcVert = jeVertArray_GetVertByIndex(BSP->VertArray, *pIVert);

		pLVert->X = pSrcVert->X;
		pLVert->Y = pSrcVert->Y;
		pLVert->Z = pSrcVert->Z;
	#if 0
		pLVert->r = pTVert->r;
		pLVert->g = pTVert->g;
		pLVert->b = pTVert->b;
	#endif
		
		pLVert->u = pTVert->u;
		pLVert->v = pTVert->v;

		pLVert++;
		pTVert++;
		pIVert++;
	}

	ClipInfo.NumSrcVerts = NumVerts1;
	ClipInfo.SrcVerts = LVerts1;

	ClipInfo.Work1 = LVerts1;
	ClipInfo.Work2 = LVerts2;

	ClipInfo.ClipFlags = ClipFlags;

#if 0
	// Clip the verts against the frustum
	if (0)//pFaceInfo->Flags & FACEINFO_FLAGS_GOURAUD)
	{
		// Clip UV, and RGB
		if (!jeFrustum_ClipLVertsXYZUVRGB(SceneInfo->Frustum, &ClipInfo))
			return;		// Poly was clipped away
	}
	else
#endif
	{
		// Just clip UV
		if (!jeFrustum_ClipLVertsXYZUV(SceneInfo->Frustum, &ClipInfo))
			return;		// Poly was clipped away
	}

	// BEGIN - Hardware T&L - paradoxnj 4/7/2005

	// Transform from Model to Camera Space 
	//	(The ModelToWorld and WorldToCamera XForms are combined here)
	for (pLVert = ClipInfo.DstVerts, i=0; i< ClipInfo.NumDstVerts; i++, pLVert++)
		jeXForm3d_Transform(&SceneInfo->ModelToCameraXForm, (jeVec3d*)pLVert, (jeVec3d*)pLVert);

	// END - Hardware T&L - paradoxnj 4/7/2005

	// FIXME:  All this shit can be cached out (maybe cache out bitmaps on the drawfaces?)
	pFaceInfo = jeFaceInfo_ArrayGetFaceInfoByIndex(BSP->FaceInfoArray, Face->FaceInfoIndex);
	pMaterial = jeMaterial_ArrayGetMaterialByIndex(BSP->MaterialArray, pFaceInfo->MaterialIndex);
#ifdef _USE_BITMAPS
	pBitmap = jeMaterial_GetBitmap(pMaterial);
#else
	pMatSpec = jeMaterial_GetMaterialSpec(pMaterial);
    if (pMatSpec == NULL) return;
	pBitmap = jeMaterialSpec_GetLayerBitmap(pMatSpec, 0);
#pragma message("Krouer: think to add the jeTexture support here")
#endif

	Flags = SceneInfo->DefaultRenderFlags;

#if 1
	if (Face->PortalObject)		// Special portal face
	{
		jeFrustum			PortalFrustum;
		jeVec3d				Origin;
		jeLVertex			*pVerts2;
		jePlane				FacePlane;
		PortalMsgData		MsgData;
		jeXForm3d			WorldSpaceFaceXForm;

		assert(Face->PortalObject->Methods->Type == JE_OBJECT_TYPE_PORTAL);
		assert(Face->PortalXForm);

		g_WorldDebugInfo.NumPortals++;

		jeVec3d_Clear(&Origin);

		pVerts2 = ClipInfo.DstVerts;

		// Create the frustum from the portal in camera space (This way the frustum is in camera space)
		jeFrustum_SetFromLVerts2(&PortalFrustum, pVerts2, ClipInfo.NumDstVerts, h_LeftHanded);

		// Get the face plane
		FacePlane = *jePlaneArray_GetPlaneByIndex(BSP->PlaneArray, Face->PlaneIndex);

		if (!jePlaneArray_IndexSided(Face->PlaneIndex))
			jePlane_Inverse(&FacePlane);	// Point the Normal opposite of face

		// BEGIN - Hardware T&L - paradoxnj 4/7/2005

		// Transform the plane into world space 
		jePlane_Transform(&FacePlane, &BSP->ModelToWorldXForm, &FacePlane);
		// Transform the FaceXForm into WorldSpace
		jeXForm3d_Multiply(&BSP->ModelToWorldXForm, Face->PortalXForm, &WorldSpaceFaceXForm);

		// END - Hardware T&L - paradoxnj 4/7/2005

		// Setup the message data
		//MsgData.World = (jeWorld*)Face->PortalObject->Instance;
		MsgData.Plane = &FacePlane;
		MsgData.FaceXForm = &WorldSpaceFaceXForm;
		MsgData.World = BSP->World;
		MsgData.Camera = SceneInfo->Camera;
		MsgData.Frustum = &PortalFrustum;
			
		BSP->Driver->BeginBatch();

		// Render the portal
		jeObject_SendMessage(Face->PortalObject, 0, &MsgData);
	}
#endif

	// Set up the verts
	{
		jeFloat		Alpha;

		if (pFaceInfo->Flags & FACEINFO_TRANSPARENT)
		{
			Flags |= JE_RENDER_FLAG_ALPHA;
			Alpha = pFaceInfo->Alpha;
		}
		else
		{
			// Only non transparent polys use spans
			if (BSP->DefaultContents & JE_BSP_CONTENTS_SOLID)
				Flags |= (JE_RENDER_FLAG_SWRITE | JE_RENDER_FLAG_STEST);

			Alpha = 255.0f;
		}

		for (pLVert = ClipInfo.DstVerts, i=0; i<ClipInfo.NumDstVerts; i++, pLVert++)
		{
		#if 0
			jeFloat			Val;

			Val = (jeFloat)(((uint32)Face<<1)&255);

			pLVert->r = pLVert->g = pLVert->b = Val;
		#else
			pLVert->r = pLVert->g = pLVert->b = 255.0f;
		#endif
			pLVert->a = Alpha;
		}
	}

	{
		if (h_LeftHanded)		// Big hack-a-rama
			Flags |= JE_RENDER_FLAG_COUNTER_CLOCKWISE;
	}

	// BEGIN - Hardware T&L - paradoxnj 4/7/2005

	// Transform and project the point
	jeCamera_ProjectAndClampLArray(SceneInfo->Camera, ClipInfo.DstVerts, TLVerts, ClipInfo.NumDstVerts);

	// END - Hardware T&L - paradoxnj 4/7/2005

	if (!(pFaceInfo->Flags & FACEINFO_RENDER_PORTAL_ONLY))
	{
		g_WorldDebugInfo.NumRenderedPolys++;
	
		if (pBitmap)
		{
			jeTexture	*THandle;
			jeRDriver_Layer		Layers[2];
	
			THandle = jeBitmap_GetTHandle(pBitmap);
			assert(THandle);
	
			Layers[0].THandle = THandle;
			Layers[0].Rop = Rop_Multiply;
			Layers[0].ShiftU = pFaceInfo->ShiftU;
			Layers[0].ShiftV = pFaceInfo->ShiftV;
			Layers[0].ScaleU = pFaceInfo->DrawScaleU/pFaceInfo->LMapScaleU;
			Layers[0].ScaleV = pFaceInfo->DrawScaleV/pFaceInfo->LMapScaleV;
	
			// BEGIN - Hardware T&L - paradoxnj 4/7/2005
			if (Face->Lightmap && BSP->RenderMode == RenderMode_TexturedAndLit)
			{
				
				jeBSPNode_Lightmap	*pLightmap;
	
				pLightmap = Face->Lightmap;
				assert(pLightmap->THandle);
	
				Layers[1].THandle = pLightmap->THandle;
				Layers[1].Rop = Rop_None;
				Layers[1].ShiftU = pLightmap->StartU;
				Layers[1].ShiftV = pLightmap->StartV;
				Layers[1].ScaleU = 16.0f;
				Layers[1].ScaleV = 16.0f;

				
				// BEGIN - Hardware T&L - paradoxnj 4/7/2005
				BSP->Driver->RenderWorldPoly(TLVerts, ClipInfo.NumDstVerts, Layers, 2, (void*)Face, Flags);
				//BSP->Driver->RenderWorldPoly(ClipInfo.DstVerts, ClipInfo.NumDstVerts, Layers, 1, (void*)Face, Flags);
				// END - Hardware T&L - paradoxnj 4/7/2005
			}
			else
			{
				// BEGIN - Hardware T&L - paradoxnj 4/7/2005
				BSP->Driver->RenderWorldPoly(TLVerts, ClipInfo.NumDstVerts, Layers, 1, NULL, Flags);
				//BSP->Driver->RenderWorldPoly(ClipInfo.DstVerts, ClipInfo.NumDstVerts, Layers, 1, NULL, Flags);
				// END - Hardware T&L - paradoxnj 4/7/2005
			}
		}
		else
		{
			// BEGIN - Hardware T&L - paradoxnj 4/7/2005
			BSP->Driver->RenderGouraudPoly(TLVerts, ClipInfo.NumDstVerts, Flags);
			//BSP->Driver->RenderGouraudPoly(ClipInfo.DstVerts, ClipInfo.NumDstVerts, Flags);
			// END - Hardware T&L - paradoxnj 4/7/2005
		}
	}

	if (Face->PortalObject)		
		BSP->Driver->EndBatch();

	if ((Face->TopSideFlags & TOPSIDE_CALL_CB) && BSP->DrawFaceCB)
		BSP->DrawFaceCB(TLVerts, ClipInfo.NumDstVerts, BSP->DrawFaceCBContext);
}


void jeBSPNode_DrawFaceRenderPortal(const jeBSPNode_DrawFace *Face, jeBSP *BSP, jeBSPNode_SceneInfo *SceneInfo, uint32 ClipFlags)
{
	if (Face->PortalObject)		// Special portal face
	{
		jePlane				FacePlane;
		PortalMsgData		MsgData;
		jeXForm3d			WorldSpaceFaceXForm;

		assert(Face->PortalObject->Methods->Type == JE_OBJECT_TYPE_PORTAL);
		assert(Face->PortalXForm);

		g_WorldDebugInfo.NumPortals++;

		// Get the face plane
		FacePlane = *jePlaneArray_GetPlaneByIndex(BSP->PlaneArray, Face->PlaneIndex);

		if (!jePlaneArray_IndexSided(Face->PlaneIndex))
			jePlane_Inverse(&FacePlane);	// Point the Normal opposite of face

		// BEGIN - Hardware T&L - paradoxnj 4/7/2005

		// Transform the plane into world space 
		jePlane_Transform(&FacePlane, &BSP->ModelToWorldXForm, &FacePlane);
		// Transform the FaceXForm into WorldSpace
		jeXForm3d_Multiply(&BSP->ModelToWorldXForm, Face->PortalXForm, &WorldSpaceFaceXForm);

		// END - Hardware T&L - paradoxnj 4/7/2005

		// Setup the message data
		//MsgData.World = (jeWorld*)Face->PortalObject->Instance;
		MsgData.Plane = &FacePlane;
		MsgData.FaceXForm = &WorldSpaceFaceXForm;
		MsgData.World = BSP->World;
		MsgData.Camera = SceneInfo->Camera;
		MsgData.Frustum = NULL;
			
		//BSP->Driver->BeginBatch();

		// Render the portal
		jeObject_SendMessage(Face->PortalObject, 1, &MsgData);

		//BSP->Driver->EndBatch();

		jeObject_SetRenderNextPass(Face->PortalObject, JE_TRUE);
	}
}