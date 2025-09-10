/****************************************************************************************/
/*  jeModel.C                                                                           */
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
#include <assert.h>
#include <memory.h> // memset

#pragma message (" Clean up Add/Remove code.  There are some leaks on error...")

#include "Dcommon.h"
#include "Engine.h"

// Public Dependents
#include "jeModel.h"

// Private dependents
#include "Errorlog.h"
#include "Camera.h"
#include "jeBSP.h"
#include "Ram.h"
#include "jeFrustum.h"
#include "Bitmap._h"
#include "jeChain.h"
#include "Errorlog.h"

//========================================================================================
//========================================================================================
#define ZeroMem(a) memset(a, 0, sizeof(*a))
#define ZeroMemArray(a, s) memset(a, 0, sizeof(*a)*s)

typedef struct jeModel
{
	int32					RefCount;

	uint16					Flags;

	// The model creates these arrays...
	jeBSP					*BSPTree;		// BSPTree for this model
	jeChain					*Brushes;

	jeFaceInfo_Array		*FaceInfoArray;

	jeBSP_RenderMode		RenderMode;

	jeBrush_Contents		DefaultContents;
	jeXForm3d				XForm;			// Models current XForm

} jeModel;

//========================================================================================
//========================================================================================
static jeBoolean jeModel_WriteHeader(const jeModel *Model, jeVFile *VFile);
static jeBoolean jeModel_ReadHeader(jeModel *Model, jeVFile *VFile);


static jeBoolean WriteBrush(jeVFile *VFile, void **LinkData, void *Context, jePtrMgr *PtrMgr);
static jeBoolean jeModel_WriteBrushes(const jeModel *Model, jeVFile *VFile, jePtrMgr *PtrMgr);

static jeBoolean ReadBrush(jeVFile *VFile, void **LinkData, void *Context, jePtrMgr *PtrMgr);
static jeBoolean jeModel_ReadBrushes(jeModel *Model, jeVFile *VFile, jePtrMgr *PtrMgr);

//========================================================================================
//	jeModel_DestroyBrushes
//========================================================================================
static void jeModel_DestroyBrushes(jeModel *Model)
{
	jeBrush		*Brush;

	assert(Model);
	assert(Model->Brushes);

	Brush = NULL;

	while(Brush = (jeBrush*)jeChain_GetNextLinkData(Model->Brushes, Brush))
	{
		jeChain_RemoveLinkData(Model->Brushes, Brush);
		jeBrush_Destroy(&Brush);
	}

	jeChain_Destroy(&Model->Brushes);
}

//========================================================================================
//	jeModel_Create
//========================================================================================
JETAPI jeModel * JETCC jeModel_Create(void)
{
	jeModel		*Model;

	Model = JE_RAM_ALLOCATE_STRUCT(jeModel);

	if (!Model)
		return NULL;

	ZeroMem(Model);

	Model->RefCount = 1;
	
	Model->RenderMode = RenderMode_TexturedAndLit;

	// Create the brush chain (model owns this chain)
	Model->Brushes = jeChain_Create();

	if (!Model->Brushes)
		goto ExitWithError;

	// Set the models XForm to identity
	jeXForm3d_SetIdentity(&Model->XForm);

	// Set the default contents
	Model->DefaultContents = JE_BSP_CONTENTS_SOLID;

	// Create an "empty" BSPTree
	Model->BSPTree = jeBSP_Create();

	if (!Model->BSPTree)
		goto ExitWithError;

	return Model;

	ExitWithError:
	{
		if (Model)
		{
			if (Model->Brushes)
				jeChain_Destroy(&Model->Brushes);

			if (Model->BSPTree)
				jeBSP_Destroy(&Model->BSPTree);

			jeRam_Free(Model);
		}

		return NULL;
	}
}

//========================================================================================
//	jeModel_CreateFromFile
//========================================================================================
JETAPI jeModel * JETCC jeModel_CreateFromFile(jeVFile *VFile, jePtrMgr *PtrMgr)
{
	jeModel		*Model = NULL;

	if (PtrMgr)
	{
		if (!jePtrMgr_ReadPtr(PtrMgr, VFile, (void **)&Model))
			return NULL;

		if (Model)
		{
			if (!jeModel_CreateRef(Model))
				return NULL;

			return Model;		// Ptr found in stack, return it
		}
	}

	Model = JE_RAM_ALLOCATE_STRUCT(jeModel);

	if (!Model)
		return NULL;

	ZeroMem(Model);

	Model->RefCount = 1;

	Model->RenderMode = RenderMode_TexturedAndLit;

	// Read in the header info
	if (!jeModel_ReadHeader(Model, VFile))
		goto ExitWithError;

	if (!jeModel_ReadBrushes(Model, VFile, PtrMgr))
		goto ExitWithError;

	// Create an empty BSPTree
	Model->BSPTree = jeBSP_Create();

	if (!Model->BSPTree)
		goto ExitWithError;

	jeBSP_SetDefaultContents(Model->BSPTree, Model->DefaultContents);
	jeBSP_SetXForm(Model->BSPTree, &Model->XForm);

	if (PtrMgr) {
		jeBSP_SetEngine(Model->BSPTree, jeResourceMgr_GetEngine(jePtrMgr_GetResourceMgr(PtrMgr)));
	}

	if (PtrMgr)
	{
		// Push the ptr on the stack
		if (!jePtrMgr_PushPtr(PtrMgr, Model))
			goto ExitWithError;
	}

	return Model;

	ExitWithError:
	{
		if (Model)
			jeModel_Destroy(&Model);

		return NULL;
	}
}

//========================================================================================
//	jeModel_WriteToFile
//========================================================================================
JETAPI jeBoolean JETCC jeModel_WriteToFile(const jeModel *Model, jeVFile *VFile, jePtrMgr *PtrMgr)
{
	if (PtrMgr)
	{
		uint32		Count;

		if (!jePtrMgr_WritePtr(PtrMgr, VFile, (void*)Model, &Count))
			return JE_FALSE;

		if (Count)
			return JE_TRUE;		// Ptr was on stack, so return
	}

	// Write out header info
	if (!jeModel_WriteHeader(Model, VFile))
	{
		jeErrorLog_AddString(-1, "jeModel_WriteToFile:  jeModel_WriteHeader failed.\n", NULL);
		return JE_FALSE;
	}

	if (!jeModel_WriteBrushes(Model, VFile, PtrMgr))
		return JE_FALSE;

	if (PtrMgr)
	{
		// Push the ptr on the stack
		if (!jePtrMgr_PushPtr(PtrMgr, (void*)Model))
			return JE_FALSE;
	}

	return JE_TRUE;
}

//========================================================================================
//	jeModel_CreateRef
//========================================================================================
JETAPI jeBoolean JETCC jeModel_CreateRef(jeModel *Model)
{
	assert(Model);
	
	Model->RefCount++;

	return JE_TRUE;
}

//========================================================================================
//	jeModel_Destroy
//========================================================================================
JETAPI void JETCC jeModel_Destroy(jeModel **Model)
{
	assert(Model);
	assert(*Model);
	assert((*Model)->RefCount > 0);

	(*Model)->RefCount--;

	if ((*Model)->RefCount == 0)
	{
		// Destroy Brushes
		jeModel_DestroyBrushes(*Model);

		// Destroy the bsp tree
		if ((*Model)->BSPTree)
			jeBSP_Destroy(&(*Model)->BSPTree);

		if ((*Model)->FaceInfoArray)
			jeFaceInfo_ArrayDestroy(&(*Model)->FaceInfoArray);
	}

	jeRam_Free(*Model);
}

//========================================================================================
//	jeModel_SetArrays
//========================================================================================
JETAPI void JETCC jeModel_SetArrays(jeModel *Model, jeFaceInfo_Array *FArray, jeMaterial_Array *MArray, jeChain *LChain, jeChain *DLChain)
{
	jeChain_Link		*Link;

	assert(jeFaceInfo_ArrayIsValid(FArray));

	// Tell the current set of brushes the new FaceInfoArray
	for (Link = jeChain_GetFirstLink(Model->Brushes); Link; Link = jeChain_LinkGetNext(Link))
	{
		jeBrush		*Brush;

		Brush = (jeBrush*)jeChain_LinkGetLinkData(Link);

		if (!jeBrush_SetFaceInfoArray(Brush, FArray))
			goto ExitWithError;
	}

	// Update the BSP
	if (!jeBSP_SetArrays(Model->BSPTree, FArray, MArray, LChain, DLChain))
		goto ExitWithError;

	if (Model->FaceInfoArray)
		jeFaceInfo_ArrayDestroy(&Model->FaceInfoArray);

	if (FArray)
	{
		// Ref the array so we can keep it out of this scope
		if (!jeFaceInfo_ArrayCreateRef(FArray))
			goto ExitWithError;
		
		// Remember the array so we can set new incoming brushes to the array
		Model->FaceInfoArray = FArray;	
	}

	return;

	ExitWithError:
	{
		#pragma message ("This function needs to be able to fail, currently it CAN'T!")
		assert(0);
	}
}

//========================================================================================
//	jeModel_AddBrush
//========================================================================================
JETAPI jeBoolean JETCC jeModel_AddBrush(jeModel *Model, jeBrush *Brush, jeBoolean Update, jeBoolean AutoLight)
{
	assert(jeChain_FindLink(Model->Brushes, Brush) == NULL);

	if (!jeBrush_SetFaceInfoArray(Brush, Model->FaceInfoArray))
		return JE_FALSE;

	if (!jeChain_AddLinkData(Model->Brushes, Brush))
		return JE_FALSE;

	if (!jeBrush_CreateRef(Brush))		// Ref it
		return JE_FALSE;

	if (Update)
	{
		if (!jeBSP_AddBrush(Model->BSPTree, Brush, AutoLight))
		{
			jeErrorLog_AddString(-1, "jeModel_AddBrush:  jeBSP_AddBrush failed.", NULL);
			return JE_FALSE;
		}
	}

	return JE_TRUE;
}

//========================================================================================
//	jeModel_RemoveBrush
//========================================================================================
JETAPI jeBoolean JETCC jeModel_RemoveBrush(jeModel *Model, jeBrush *Brush, jeBoolean Update)
{
	assert(jeChain_FindLink(Model->Brushes, Brush));

	if (!jeChain_RemoveLinkData(Model->Brushes, Brush))
		return JE_FALSE;

	if (Update)
	{
		assert(Model->BSPTree);
		
		if (jeBSP_HasBrush(Model->BSPTree, Brush))
		{
			if (!jeBSP_RemoveBrush(Model->BSPTree, Brush))
			{
				jeErrorLog_AddString(-1, "jeModel_RemoveBrush:  jeModel_RemoveBrush failed.", NULL);
				assert(0);
				return JE_FALSE;
			}
		}
	}

	jeBrush_Destroy(&Brush);	// De-Ref

	return JE_TRUE;
}

//========================================================================================
//	jeModel_UpdateBrush
//		Makes the brush current in the bsp
//========================================================================================
JETAPI jeBoolean JETCC jeModel_UpdateBrush(jeModel *Model, jeBrush *Brush, jeBoolean AutoLight)
{
	assert(Model);
	assert(jeChain_FindLink(Model->Brushes, Brush));

	if (!jeBSP_UpdateBrush(Model->BSPTree, Brush, AutoLight))
	{
		jeErrorLog_AddString(-1, "jeModel_UpdateBrush:  jeBSP_UpdateBrush failed.", NULL);
		return JE_FALSE;
	}

	return JE_TRUE;
}

//========================================================================================
//	jeModel_GetNextBrush
//========================================================================================
JETAPI jeBrush * JETCC jeModel_GetNextBrush(const jeModel *Model, const jeBrush *Start)
{
	assert(Model);
	assert(Model->Brushes);

	return (jeBrush *)jeChain_GetNextLinkData(Model->Brushes, (void*)Start);
}

//========================================================================================
//	jeModel_HasBrush
//========================================================================================
JETAPI jeBoolean JETCC jeModel_HasBrush(const jeModel *Model, const jeBrush *Brush)
{
	return (jeChain_FindLink(Model->Brushes, (void*)Brush) != NULL);
}

//========================================================================================
//	jeModel_UpdateBrushFace
//========================================================================================
JETAPI jeBoolean JETCC jeModel_UpdateBrushFace(jeModel *Model, const jeBrush_Face *Face, jeBoolean AutoLight)
{
	assert(Model);
	assert(Face);

	return jeBSP_UpdateBrushFace(Model->BSPTree, Face, AutoLight);
}

//=======================================================================================
//	jeModel_AddObject
//	Adds an object to the Models BSP tree.  The visible objects are then rendered
//	upon calling jeBSP_Render...
//=======================================================================================
JETAPI jeBoolean JETCC jeModel_AddObject(jeModel *Model, jeObject *Object)
{
	assert(Model);

	return jeBSP_AddObject(Model->BSPTree, Object);
}

//=======================================================================================
//	jeModel_RemoveObject
//=======================================================================================
JETAPI jeBoolean JETCC jeModel_RemoveObject(jeModel *Model, jeObject *Object)
{
	assert(Model);

	return jeBSP_RemoveObject(Model->BSPTree, Object);
}

//=======================================================================================
//	jeModel_RebuildightsFromPoint
//=======================================================================================
JETAPI jeBoolean JETCC jeModel_RebuildLightsFromPoint(jeModel *Model, const jeVec3d *Pos, jeFloat Radius)
{
	assert(Model);
	assert(Pos);

	return jeBSP_RebuildLightsFromPoint(Model->BSPTree, Pos, Radius);
}

//========================================================================================
//	jeModel_RebuilBSP
//========================================================================================
JETAPI jeBoolean JETCC jeModel_RebuildBSP(jeModel *Model, 
										jeBSP_Options Options, 
										jeBSP_Logic Logic, 
										jeBSP_LogicBalance LogicBalance)
{
	if (!jeChain_GetLinkCount(Model->Brushes))
		return JE_TRUE;		// No brushes in pool

	jeBSP_RebuildGeometry(	Model->BSPTree, 
							Model->Brushes, 
							Options, 
							Logic, 
							LogicBalance);

	return JE_TRUE;
}

//========================================================================================
//	jeModel_RebuildBSPFaces
//========================================================================================
JETAPI jeBoolean JETCC jeModel_RebuildBSPFaces(jeModel *Model)
{
	assert(Model);

	if (!jeBSP_RebuildFaces(Model->BSPTree))
		return JE_FALSE;

	return JE_TRUE;
}

//========================================================================================
//	jeModel_PatchLighting
//========================================================================================
JETAPI jeBoolean JETCC jeModel_PatchLighting(jeModel *Model)
{
	assert(Model);

	return jeBSP_PatchLighting(Model->BSPTree);
}

//========================================================================================
//	jeModel_Rebuildights
//========================================================================================
JETAPI jeBoolean JETCC jeModel_RebuildLights(jeModel *Model)
{
	assert(Model);

	return jeBSP_RebuildLights(Model->BSPTree);
}

//========================================================================================
//	jeModel_SetEngine
//========================================================================================
JETAPI jeBoolean JETCC jeModel_SetEngine(jeModel *Model, jeEngine *Engine)
{
	jeBSP_SetEngine(Model->BSPTree, Engine);

	return JE_TRUE;
}

//========================================================================================
//	jeModel_SetWorld
//========================================================================================
JETAPI jeBoolean	JETCC jeModel_SetWorld(jeModel *Model, jeWorld *World)
{
	jeBSP_SetWorld(Model->BSPTree, World);

	return JE_TRUE;
}


//========================================================================================
//	jeModel_SetRenderOptions
//========================================================================================
JETAPI jeBoolean JETCC jeModel_SetRenderOptions(jeModel *Model, jeBSP_RenderMode RenderMode)
{
	Model->RenderMode = RenderMode;

	// Set the render options
	if (!jeBSP_SetRenderMode(Model->BSPTree,Model->RenderMode))
		return JE_FALSE;

	return JE_TRUE;
}

//========================================================================================
//	jeModel_SetDefaultContents
//========================================================================================
JETAPI jeBoolean JETCC jeModel_SetDefaultContents(jeModel *Model, jeBrush_Contents DefaultContents)
{
	Model->DefaultContents = DefaultContents;

	// Set the render options
	if (!jeBSP_SetDefaultContents(Model->BSPTree, DefaultContents))
		return JE_FALSE;

	return JE_TRUE;
}

//========================================================================================
//	jeModel_Render
//========================================================================================
JETAPI jeBoolean JETCC jeModel_Render(jeModel *Model, jeCamera *Camera, jeFrustum *CameraSpaceFrustum)
{
	assert(Model);
	assert(Camera);
	assert(CameraSpaceFrustum);

	// Render the models BSP tree
	if (!jeBSP_RenderAndVis(Model->BSPTree, Camera, CameraSpaceFrustum))
		return JE_FALSE;

	return JE_TRUE;
}

//========================================================================================
//	jeModel_RayIntersectsBrushes
//========================================================================================
JETAPI jeBoolean JETCC jeModel_RayIntersectsBrushes(const jeModel *Model, const jeVec3d *Front, const jeVec3d *Back, jeBrushRayInfo *Info)
{
	assert(Model);

	return jeBSP_RayIntersectsBrushes(Model->BSPTree, Front, Back, Info);
}

//========================================================================================
//	jeModel_SetBrushFaceCB
//========================================================================================
JETAPI jeBoolean JETCC jeModel_SetBrushFaceCB(jeModel *Model, jeBSPNode_DrawFaceCB *CB, void *Context)
{
	assert(Model);

	return jeBSP_SetBrushFaceCB(Model->BSPTree, CB, Context);
}

//========================================================================================
//	jeModel_SetBrushFaceCBOnOff
//========================================================================================
JETAPI jeBoolean JETCC jeModel_SetBrushFaceCBOnOff(jeModel *Model, const jeBrush_Face *Face, jeBoolean OnOff)
{
	assert(Model);
	assert(Face);

	return jeBSP_SetBrushFaceCBOnOff(Model->BSPTree, Face, OnOff);
}

jeBSP_DebugInfo	FakeBSPInfo;

//========================================================================================
//	jeModel_GetBSPDebugInfo
//========================================================================================
JETAPI const jeBSP_DebugInfo * JETCC jeModel_GetBSPDebugInfo(const jeModel *Model)
{
	assert(Model);

	return jeBSP_GetDebugInfo(Model->BSPTree);
}

//========================================================================================
//	jeModel_Collision
//	Returns JE_TRUe if there was a collision, JE_FALSE otherwise
//========================================================================================
JETAPI jeBoolean JETCC jeModel_Collision(	const jeModel	*Model, 
										const jeExtBox	*Box, 
										const jeVec3d	*Front, 
										const jeVec3d	*Back, 
										jeVec3d			*Impact, 
										jePlane			*Plane)
{
	assert(Model);
	assert(Front && Back);
	//assert(Impact); // Icestorm: They CAN be NULL
	//assert(Plane);

	return jeBSP_Collision(Model->BSPTree, Box, Front, Back, Impact, Plane);
}

// Added by Icestorm
//========================================================================================
//	jeModel_ChangeBoxCollision
//	Returns JE_TRUE if there was a collision, JE_FALSE otherwise
//========================================================================================
JETAPI jeBoolean JETCC jeModel_ChangeBoxCollision(	const jeModel	*Model, 
												const jeVec3d	*Pos, 
												const jeExtBox	*FrontBox, 
												const jeExtBox	*BackBox, 
												jeExtBox		*ImpactBox, 
												jePlane			*Plane)
{
	assert(Model);
	assert(FrontBox && BackBox);
	assert(Pos);
	//assert(ImpactBox);// Icestorm: They CAN be NULL
	//assert(Plane);

	return jeBSP_ChangeBoxCollision(Model->BSPTree, Pos, FrontBox, BackBox, ImpactBox, Plane);
}

//========================================================================================
//	jeModel_SetXForm
//========================================================================================
JETAPI jeBoolean JETCC jeModel_SetXForm(jeModel *Model, const jeXForm3d *XForm)
{
	// Save the XForm
	Model->XForm = *XForm;
	
	if (!jeBSP_SetXForm(Model->BSPTree, XForm))
		return JE_FALSE;
	
	return JE_TRUE;
}

//========================================================================================
//	jeModel_GetXForm
//========================================================================================
JETAPI const jeXForm3d * JETCC jeModel_GetXForm(jeModel *Model )
{
	return &Model->XForm;
}

//========================================================================================
//	****** local static functions ********
//========================================================================================

#define MAKEFOURCC(ch0, ch1, ch2, ch3)                              \
		((DWORD)(BYTE)(ch0) | ((DWORD)(BYTE)(ch1) << 8) |   \
		((DWORD)(BYTE)(ch2) << 16) | ((DWORD)(BYTE)(ch3) << 24 ))

#define JU_MODEL_TAG				MAKEFOURCC('G', 'E', 'M', 'F')		// 'GE' 'M'odel 'F'ile
#define JU_MODEL_VERSION			0x0000

//========================================================================================
//	jeModel_WriteHeader
//========================================================================================
static jeBoolean jeModel_WriteHeader(const jeModel *Model, jeVFile *VFile)
{
	uint32		Tag;
	uint16		Version;

	assert(Model);
	assert(VFile);

	// Write TAG
	Tag = JU_MODEL_TAG;

	if (!jeVFile_Write(VFile, &Tag, sizeof(Tag)))
		return JE_FALSE;

	// Write version
	Version = JU_MODEL_VERSION;

	if (!jeVFile_Write(VFile, &Version, sizeof(Version)))
		return JE_FALSE;

	// Save off the flags
	if (!jeVFile_Write(VFile, &Model->Flags, sizeof(Model->Flags)))
		return JE_FALSE;

	// Save DefaultContents
	if (!jeVFile_Write(VFile, &Model->DefaultContents, sizeof(Model->DefaultContents)))
		return JE_FALSE;

	// Save XForm
	if (!jeVFile_Write(VFile, &Model->XForm, sizeof(Model->XForm)))
		return JE_FALSE;

	return JE_TRUE;
}

//========================================================================================
// jeModel_ReadHeader
//========================================================================================
static jeBoolean jeModel_ReadHeader(jeModel *Model, jeVFile *VFile)
{
	uint32		Tag;
	uint16		Version;

	assert(Model);
	assert(VFile);

	if (!jeVFile_Read(VFile, &Tag, sizeof(Tag)))
		return JE_FALSE;

	if (Tag != JU_MODEL_TAG)
		return JE_FALSE;

	if (!jeVFile_Read(VFile, &Version, sizeof(Version)))
		return JE_FALSE;

	if (Version != JU_MODEL_VERSION)
		return JE_FALSE;

	// Read Flags
	if (!jeVFile_Read(VFile, &Model->Flags, sizeof(Model->Flags)))
		return JE_FALSE;

	// Read DefaultContents
	if (!jeVFile_Read(VFile, &Model->DefaultContents, sizeof(Model->DefaultContents)))
		return JE_FALSE;

	// Read XForm
	if (!jeVFile_Read(VFile, &Model->XForm, sizeof(Model->XForm)))
		return JE_FALSE;

	return JE_TRUE;

}

//========================================================================================
//	ReadBrush
//========================================================================================
static jeBoolean ReadBrush(jeVFile *VFile, void **LinkData, void *Context, jePtrMgr *PtrMgr)
{
	*LinkData = jeBrush_CreateFromFile(VFile, PtrMgr);
		
	if (!(*LinkData))
		return JE_FALSE;

	// Icestorm: Double ref. (see jeBrush_CreateFromFile)
	//if (!jeBrush_CreateRef(*LinkData))		// Ref it
	//	return JE_FALSE;

	return JE_TRUE;
}


//========================================================================================
//	WriteBrush
//========================================================================================
static jeBoolean WriteBrush(jeVFile *VFile, void **LinkData, void *Context, jePtrMgr *PtrMgr)
{
	if (!jeBrush_WriteToFile((jeBrush *)*LinkData, VFile, PtrMgr))
		return JE_FALSE;

	return JE_TRUE;
}



//========================================================================================
//	jeModel_ReadBrushes
//========================================================================================
static jeBoolean jeModel_ReadBrushes(jeModel *Model, jeVFile *VFile, jePtrMgr *PtrMgr)
{
	// Read in the brush chain (The model owns/created this chain)
	Model->Brushes = jeChain_CreateFromFile(VFile, ReadBrush, NULL, PtrMgr);
	
	if (!Model->Brushes)
		return JE_FALSE;

	return JE_TRUE;
}

//========================================================================================
//	jeModel_WriteBrushes
//========================================================================================
static jeBoolean jeModel_WriteBrushes(const jeModel *Model, jeVFile *VFile, jePtrMgr *PtrMgr)
{
	// Write out brush chain (The model owns/created this chain)
	if (!jeChain_WriteToFile(Model->Brushes, VFile, WriteBrush, NULL, PtrMgr))
	{
		jeErrorLog_AddString(-1, "jeModel_WriteToFile:  jeChain_WriteToFile failed for Brushes.\n", NULL);
		return JE_FALSE;
	}

	return JE_TRUE;
}

