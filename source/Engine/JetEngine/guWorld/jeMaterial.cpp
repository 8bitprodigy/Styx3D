/****************************************************************************************/
/*  JEMATERIAL.C                                                                        */
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
#include <string.h>

// Public dependents
#include "jeMaterial.h"

// Private dependents
#include "jeMaterial._h"
#include "Errorlog.h"
#include "Array.h"
#include "Ram.h"
#include "Engine.h"

#include "jeGArray.h"

#include "jeResource.h"
#include "jePtrMgr._h"

#include "log.h"

//========================================================================================
//========================================================================================
#define ZeroMem(a) memset(a, 0, sizeof(*a))
#define ZeroMemArray(a, s) memset(a, 0, sizeof(*a)*s)

//=======================================================================================
//=======================================================================================
typedef struct jeMaterial
{
	char		MatName[JE_MATERIAL_MAX_NAME_SIZE];

	char		BitmapName[JE_MATERIAL_MAX_NAME_SIZE];

#ifdef _USE_BITMAPS
		jeBitmap	*Bitmap;
#else
		jeMaterialSpec	*MatSpec;	// krouer: can hold every kind of texture
#endif

	float		DrawScaleU;
	float		DrawScaleV;

	jeRGBA		Reflectivity;

	int32		RefCount;

	// krouer : add a new value in the file
	int8		ResourceKind;
} jeMaterial;

typedef struct jeMaterial_Array
{
	int32		RefCount;

	jeEngine	*Engine;

	jeArray		*Array;

} jeMaterial_Array;

typedef struct jeMaterial_Context
{
	jeEngine		*Engine;
	jeResourceMgr	*ResMgr;
	uint16			Version;
	uint16			Reserved;
} jeMaterial_Context;

//=======================================================================================
//	** jeMaterial **
//=======================================================================================

//=======================================================================================
//	jeMaterial_Create
//=======================================================================================
JETAPI jeMaterial * JETCC jeMaterial_Create(const char *MatName)
{
	jeMaterial	*Material;

	assert(strlen(MatName) < JE_MATERIAL_MAX_NAME_SIZE);

	Material = JE_RAM_ALLOCATE_STRUCT(jeMaterial);

	if (!Material)
		return NULL;

	ZeroMem(Material);

	if (!jeMaterial_CreateRef(Material))
	{
		jeRam_Free(Material);
		return NULL;
	}

	strcpy(Material->MatName, MatName);

	return Material;
}

//=======================================================================================
//	jeMaterial_CreateRef
//=======================================================================================
JETAPI jeBoolean JETCC jeMaterial_CreateRef(jeMaterial *Material)
{
	assert(Material->RefCount >= 0);

	Material->RefCount++;

	return JE_TRUE;
}

//=======================================================================================
//	jeMaterial_Destroy
//=======================================================================================
JETAPI void JETCC jeMaterial_Destroy(jeMaterial **Material)
{
	assert((*Material)->RefCount >= 0);

	(*Material)->RefCount--;
	
	if ((*Material)->RefCount == 0)
	{
#ifdef _USE_BITMAPS
		if ((*Material)->Bitmap)
			jeBitmap_Destroy(&(*Material)->Bitmap);
#else
		if ((*Material)->MatSpec)
			jeMaterialSpec_Destroy(&(*Material)->MatSpec);
#endif
		jeRam_Free(*Material);
	}

	*Material = NULL;
}

//=======================================================================================
//	jeMaterial_SetDefaults
//=======================================================================================
JETAPI void JETCC jeMaterial_SetDefaults(jeMaterial *Material)
{
	memset(Material, 0, sizeof(jeMaterial));
	Material->ResourceKind = JE_RESOURCE_BITMAP;
}

//=======================================================================================
//	jeMaterial_SetBitmap
//=======================================================================================
JETAPI jeBoolean JETCC jeMaterial_SetBitmap(jeMaterial *Mat, jeBitmap *Bitmap, const char *BitmapName)
{
	assert(Mat);
	assert(strlen(BitmapName) < JE_MATERIAL_MAX_NAME_SIZE);

#ifdef _USE_BITMAPS
	if (Mat->Bitmap)
		jeBitmap_Destroy(&Mat->Bitmap);		// Destroy any old bitmaps
	
	if (Bitmap)
		jeBitmap_CreateRef(Bitmap);			// Ref the new one (if there is one)
	
	Mat->Bitmap = Bitmap;
	strcpy(Mat->BitmapName, BitmapName);

	Mat->ResourceKind = JE_RESOURCE_BITMAP;

	return JE_TRUE;
#else
	return JE_FALSE;
#endif
}

//=======================================================================================
//	jeMaterial_GetBitmap
//=======================================================================================
JETAPI const jeBitmap * JETCC jeMaterial_GetBitmap(const jeMaterial *Mat)
{
	assert( Mat != NULL );
#ifdef _USE_BITMAPS
	return Mat->Bitmap;
#else
	return NULL;
#endif
}

//=======================================================================================
//	jeMaterial_GetMaterialSpec
//=======================================================================================
JETAPI const jeMaterialSpec	* JETCC jeMaterial_GetMaterialSpec(const jeMaterial *Mat)
{
	assert( Mat != NULL );
#ifndef _USE_BITMAPS
	return Mat->MatSpec;
#else
	return NULL;
#endif
}

//=======================================================================================
//	jeMaterial_GetName
//=======================================================================================
JETAPI const char * JETCC jeMaterial_GetName(const jeMaterial *Mat)
{
	assert( Mat != NULL );

	return Mat->MatName;
}

JETAPI const char * JETCC jeMaterial_GetBitmapName( const jeMaterial *Mat)
{
	assert( Mat != NULL );

	return Mat->BitmapName;
}

//=======================================================================================
//	** jeMaterial_Array **
//=======================================================================================

//=======================================================================================
//	jeMaterial_ArrayCreate
//=======================================================================================
JETAPI jeMaterial_Array * JETCC jeMaterial_ArrayCreate(int32 StartSize)
{
	jeMaterial_Array	*MArray;

	MArray = JE_RAM_ALLOCATE_STRUCT(jeMaterial_Array);

	if (!MArray)
		return NULL;

	ZeroMem(MArray);

	MArray->Array = jeArray_Create(sizeof(jeMaterial), StartSize, 5);

	if (!MArray->Array)
	{
		jeRam_Free(MArray);
		return NULL;
	}

	MArray->RefCount = 1;

	return MArray;
}

#define MAKEFOURCC(ch0, ch1, ch2, ch3)                              \
		((uint32)(uint8)(ch0) | ((uint32)(uint8)(ch1) << 8) |   \
		((uint32)(uint8)(ch2) << 16) | ((uint32)(uint8)(ch3) << 24 ))

#define JE_MARRAY_TAG			MAKEFOURCC('G', 'E', 'M', 'A')		// 'GE' 'M'aterial 'A'rray
#define JE_MARRAY_VERSION		0x0001

//========================================================================================
//	ReadMaterial
//========================================================================================
static jeBoolean ReadMaterial(jeVFile *File, void *Element,void *Context)
{
	jeMaterial	*Material;
	jeMaterial_Context* WorldContext = (jeMaterial_Context*) Context;
	uint16 Version = (uint16) WorldContext->Version;

	assert(Element);
	//assert(!Context);

	Material = (jeMaterial*)Element;

	// Krouer: modify the Material read procedure
	// Read the material
	//if (!jeVFile_Read(File, Material, sizeof(jeMaterial)))
	//	return JE_FALSE;

	if (!jeVFile_Read(File, Material->MatName, JE_MATERIAL_MAX_NAME_SIZE))
		return JE_FALSE;

	if (!jeVFile_Read(File, Material->BitmapName, JE_MATERIAL_MAX_NAME_SIZE))
		return JE_FALSE;
	
	if (Version==0) {
#ifndef _USE_BITMAPS
		// Open the file
		if (!jeVFile_Read(File, &Material->MatSpec, sizeof(jeBitmap*)))
			return JE_FALSE;
#else
		// Open the file
		if (!jeVFile_Read(File, &Material->Bitmap, sizeof(jeBitmap*)))
			return JE_FALSE;
	} else {
		Material->Bitmap = NULL;
#endif
	}

	if (!jeVFile_Read(File, &Material->DrawScaleU, sizeof(float)))
		return JE_FALSE;

	if (!jeVFile_Read(File, &Material->DrawScaleV, sizeof(float)))
		return JE_FALSE;

	if (!jeVFile_Read(File, &Material->Reflectivity, sizeof(jeRGBA)))
		return JE_FALSE;

	if (Version==0) {
		if (!jeVFile_Read(File, &Material->RefCount, sizeof(int32)))
			return JE_FALSE;
	}

	if (Version>0) {
		if (!jeVFile_Read(File, &Material->ResourceKind, sizeof(int8)))
			return JE_FALSE;
	} else {
		Material->ResourceKind = JE_RESOURCE_BITMAP;
	}
	Log_Printf("ReadMaterial %s: Kind is %d\n", Material->MatName, Material->ResourceKind); 

	// Icestorm: Only to be on the save side!
	Material->RefCount = 1;

	if (Version == 0) {
#ifdef _USE_BITMAPS
		// Read the bitmap in to the material
		Material->Bitmap = jeBitmap_CreateFromFile(File);
		jeResource_ExportResource(WorldContext->ResMgr, Material->ResourceKind, Material->BitmapName, Material->Bitmap);
#else
		jeBitmap* pBitmap;
		pBitmap = jeBitmap_CreateFromFile(File); 
		jeResource_ExportResource(WorldContext->ResMgr, Material->ResourceKind, Material->BitmapName, pBitmap);
		Material->MatSpec = jeMaterialSpec_Create(WorldContext->Engine, WorldContext->ResMgr);
		jeMaterialSpec_AddLayer(Material->MatSpec, 0, Material->ResourceKind, JE_MATERIAL_LAYER_BASE, 0, Material->BitmapName);
#endif
	} else {
		// Load texture from directory / pak file
		if (Material->ResourceKind == JE_RESOURCE_BITMAP) {
			Material->MatSpec = jeMaterialSpec_Create(WorldContext->Engine, WorldContext->ResMgr);
			jeMaterialSpec_AddLayer(Material->MatSpec, 0, Material->ResourceKind, JE_MATERIAL_LAYER_BASE, 0, Material->BitmapName);
		} else {
            Material->MatSpec = (jeMaterialSpec*) jeResource_GetResource(WorldContext->ResMgr, Material->ResourceKind, Material->BitmapName);
		}
	}

#ifdef _USE_BITMAPS
	if (!Material->Bitmap)
#else
	if (!Material->MatSpec)
#endif
		return JE_FALSE;

	return JE_TRUE;
}

//=======================================================================================
//	jeMaterial_ArrayCreateFromFile
//=======================================================================================
JETAPI jeMaterial_Array * JETCC jeMaterial_ArrayCreateFromFile(jeVFile *VFile, jePtrMgr *PtrMgr)
{
	uint32				Tag;
	uint16				Version;
	uint16				lVersionOffsetSize;
	jeMaterial_Context	MatCtx;
	jeMaterial_Array	*Array = NULL;

	if (PtrMgr)
	{
		if (!jePtrMgr_ReadPtr(PtrMgr, VFile, (void **)&Array))
			return NULL;

		if (Array)
		{				
			if (!jeMaterial_ArrayCreateRef(Array))
				return NULL;

			return Array;
		}
	}

	// Read header info
	if (!jeVFile_Read(VFile, &Tag, sizeof(Tag)))
		return NULL;

	if (Tag != JE_MARRAY_TAG)
		return NULL;

	if (!jeVFile_Read(VFile, &Version, sizeof(Version)))
		return NULL;

	if (Version > JE_MARRAY_VERSION)
		return NULL;

	// Read the array info
	Array = JE_RAM_ALLOCATE_STRUCT(jeMaterial_Array);

	if (!Array)
		return NULL;

	ZeroMem(Array);

	MatCtx.Version = Version;
	MatCtx.ResMgr  = jePtrMgr_GetResourceMgr(PtrMgr);
	MatCtx.Engine  = jeResourceMgr_GetEngine(MatCtx.ResMgr);
	lVersionOffsetSize = 0;
	if (Version == 0) {
		// Setup the Version 1 structure diff size
		lVersionOffsetSize = sizeof(int8);
	}

	Array->Array = jeArray_CreateFromFile(VFile, lVersionOffsetSize, ReadMaterial, &MatCtx);
	
	if (!Array->Array)
	{
		jeErrorLog_AddString(-1, "jeMaterial_ArrayCreateFromFile : jeArray_CreateFromFile failed.", NULL);
		jeRam_Free(Array);
		return NULL;
	}

	Array->RefCount = 1;

	if (PtrMgr)
	{
		// Push the ptr on the stack
		if (!jePtrMgr_PushPtr(PtrMgr, Array))
			goto ExitWithError;
	}

	return Array;

	ExitWithError:
	{
		if (Array)
		{
			if (Array->Array)
				jeArray_Destroy(&Array->Array);
		}

		jeRam_Free(Array);

		return NULL;
	}
}

//========================================================================================
//	WriteMaterial
//========================================================================================
static jeBoolean WriteMaterial(jeVFile *File, void *Element,void *Context)
{
	jeMaterial	*Material;
	jePtrMgr* WorldContext = (jePtrMgr*) Context;
	uint16 Version = (uint16) WorldContext->lData;

	assert(Element);

	Material = (jeMaterial*)Element;

	// Krouer : modify the write material procedure
	// Write out the material
	//if (!jeVFile_Write(File, Material, sizeof(jeMaterial)))
	//	return JE_FALSE;

	if (!jeVFile_Write(File, Material->MatName, JE_MATERIAL_MAX_NAME_SIZE))
		return JE_FALSE;

	if (!jeVFile_Write(File, Material->BitmapName, JE_MATERIAL_MAX_NAME_SIZE))
		return JE_FALSE;
	
	if (Version==0) {
#ifdef _USE_BITMAPS
		if (!jeVFile_Write(File, &Material->Bitmap, sizeof(jeBitmap*)))
			return JE_FALSE;
#else
		uint32 val;
		val = 0;
		if (!jeVFile_Write(File, &val, sizeof(uint32)))
			return JE_FALSE;
#endif
	}

	if (!jeVFile_Write(File, &Material->DrawScaleU, sizeof(float)))
		return JE_FALSE;

	if (!jeVFile_Write(File, &Material->DrawScaleV, sizeof(float)))
		return JE_FALSE;

	if (!jeVFile_Write(File, &Material->Reflectivity, sizeof(jeRGBA)))
		return JE_FALSE;

	if (Version==0) {
		if (!jeVFile_Write(File, &Material->RefCount, sizeof(int32)))
			return JE_FALSE;
	}

	if (Version>0) {
		Log_Printf("WriteMaterial %s: Kind is %d\n", Material->MatName, Material->ResourceKind); 
		if (!jeVFile_Write(File, &Material->ResourceKind, sizeof(int8)))
			return JE_FALSE;
	}

	if (Version==0) {
#ifdef _USE_BITMAPS
		// Write out the bitmap to the material
		if (!jeBitmap_WriteToFile(Material->Bitmap, File))
			return JE_FALSE;
#endif
	}

	return JE_TRUE;
}

//=======================================================================================
//	jeMaterial_ArrayWriteToFile
//=======================================================================================
JETAPI jeBoolean JETCC jeMaterial_ArrayWriteToFile(jeMaterial_Array *MatArray, jeVFile *VFile, jePtrMgr *PtrMgr)
{
	uint32				Tag;
	uint16				Version;

	if (PtrMgr)	
	{
		uint32		Count;
			
		if (!jePtrMgr_WritePtr(PtrMgr, VFile, (void*)MatArray, &Count))
			return JE_FALSE;

		if (Count)		// Ptr was on stack, so return 
			return JE_TRUE;
	}

	// Write TAG
	Tag = JE_MARRAY_TAG;

	if (!jeVFile_Write(VFile, &Tag, sizeof(Tag)))
		return JE_FALSE;

	// Write version
	Version = JE_MARRAY_VERSION;

	if (!jeVFile_Write(VFile, &Version, sizeof(Version)))
		return JE_FALSE;

	PtrMgr->lData = Version;

	if (!jeArray_WriteToFile(MatArray->Array, VFile, WriteMaterial, PtrMgr))
		return JE_FALSE;

	if (PtrMgr)
	{
		// Push the ptr on the stack
		if (!jePtrMgr_PushPtr(PtrMgr, (void*)MatArray))
			return JE_FALSE;
	}

	return JE_TRUE;
}

//=======================================================================================
//	jeMaterialArray_CreateRef
//=======================================================================================
JETAPI jeBoolean JETCC jeMaterial_ArrayCreateRef(jeMaterial_Array *MatArray)
{
	assert(MatArray);
	assert(MatArray->RefCount >= 0);

	MatArray->RefCount++;

	return JE_TRUE;
}

//=======================================================================================
//	jeMaterial_ArrayDestroy
//=======================================================================================
JETAPI void JETCC jeMaterial_ArrayDestroy(jeMaterial_Array **Array)
{
	assert(Array);
	assert(*Array);
	assert((*Array)->RefCount > 0);
	assert((*Array)->Array);

	(*Array)->RefCount--;

	if ((*Array)->RefCount > 0)
		return;

	// Icestorm: Don't forget those poor Bitmaps ;)
	{
		jeMaterial	*Material = jeMaterial_ArrayGetNextMaterial(*Array, NULL);

		while (Material)
		{
#ifdef _USE_BITMAPS
			if (Material->Bitmap)
			{
				if ((*Array)->Engine)
					if (!jeEngine_RemoveBitmap((*Array)->Engine, Material->Bitmap))
						jeErrorLog_AddString(-1, "jeMaterial_ArrayDestroy:  jeEngine_RemoveBitmap failed.", NULL);

				jeBitmap_Destroy(&Material->Bitmap);
			}
#else
			if (Material->MatSpec)
			{
				jeBitmap* pBitmap = jeMaterialSpec_GetLayerBitmap(Material->MatSpec, 0);
				if (pBitmap && (*Array)->Engine) {
					if (!jeEngine_RemoveBitmap((*Array)->Engine, pBitmap)) {
						jeErrorLog_AddString(-1, "jeMaterial_ArrayDestroy:  jeEngine_RemoveBitmap failed.", NULL);
					}
				}

				jeMaterialSpec_Destroy(&Material->MatSpec);
			}
#endif

			Material = jeMaterial_ArrayGetNextMaterial(*Array, Material);
		}
	}

	jeArray_Destroy(&(*Array)->Array);

	if ((*Array)->Engine)
		jeEngine_Destroy(&(*Array)->Engine);	// Icestorm

	jeRam_Free(*Array);

	*Array = NULL;
}

//=======================================================================================
//	jeMaterial_ArrayCreateMaterial
//=======================================================================================
JETAPI jeMaterial_ArrayIndex JETCC jeMaterial_ArrayCreateMaterial(jeMaterial_Array *MatArray, const char *MatName)
{
	jeMaterial	*Material;

	assert(strlen(MatName) < JE_MATERIAL_MAX_NAME_SIZE);

	Material = (jeMaterial*)jeArray_GetNewElement(MatArray->Array);

	if (!Material)
		return JE_MATERIAL_ARRAY_NULL_INDEX;

	jeMaterial_SetDefaults(Material);

	strcpy(Material->MatName, MatName);

	return jeArray_GetElementIndex(Material);
}

//=======================================================================================
//	jeMaterial_ArrayDestroyMaterial
//=======================================================================================
JETAPI void JETCC jeMaterial_ArrayDestroyMaterial(jeMaterial_Array *MatArray, jeMaterial_ArrayIndex *Index)
{
	jeMaterial	*Material;
	jeBoolean	Ret;

	Material = (jeMaterial*)jeArray_GetElement(MatArray->Array, *Index);

	assert(Material);

#ifdef _USE_BITMAPS
	// Icestorm : Don't forget it's bitmap!
	if (Material->Bitmap)
	{
		if (MatArray->Engine)
			if (!jeEngine_RemoveBitmap(MatArray->Engine, Material->Bitmap))
				jeErrorLog_AddString(-1, "jeMaterial_ArrayDestroyMaterial:  jeEngine_RemoveBitmap failed.", NULL);
		jeBitmap_Destroy(&Material->Bitmap);
	}
#else
	if (Material->MatSpec)
	{
		if (MatArray->Engine) {
			jeBitmap* pBitmap;
			pBitmap = jeMaterialSpec_GetLayerBitmap(Material->MatSpec, 0);
			if (!jeEngine_RemoveBitmap(MatArray->Engine, pBitmap))
				jeErrorLog_AddString(-1, "jeMaterial_ArrayDestroyMaterial:  jeEngine_RemoveBitmap failed.", NULL);
		}
		jeMaterialSpec_Destroy(&Material->MatSpec);
	}
#endif

	Ret = jeArray_FreeElement(MatArray->Array, (void*)Material);
	assert(Ret == JE_TRUE);

	*Index = JE_MATERIAL_ARRAY_NULL_INDEX;
}

//=======================================================================================
//	jeMaterial_ArrayGetMaterialByIndex
//=======================================================================================
JETAPI const jeMaterial * JETCC jeMaterial_ArrayGetMaterialByIndex(const jeMaterial_Array *Array, jeMaterial_ArrayIndex Index)
{
	return (jeMaterial*)jeArray_GetElement(Array->Array, Index);
}

//=======================================================================================
//	jeMaterial_ArrayGetMaterialIndex
//=======================================================================================
JETAPI jeMaterial_ArrayIndex JETCC jeMaterial_ArrayGetMaterialIndex(const jeMaterial_Array *Array, const jeMaterial *Material)
{
	assert(Array);

	return jeArray_GetElementIndex((void*)Material);
}

//=======================================================================================
//	jeMaterial_ArraySetMaterialBitmap
//=======================================================================================
JETAPI jeBoolean JETCC jeMaterial_ArraySetMaterialBitmap(jeMaterial_Array *Array, jeMaterial_ArrayIndex Index, jeBitmap *Bitmap, const char *BitmapName)
{
	jeMaterial	*Material;

	Material = (jeMaterial*)jeArray_GetElement(Array->Array, Index);

	if (!Material)
		return JE_FALSE;

#ifdef _USE_BITMAPS
	// Remove any old bitmaps in this material
	if (Array->Engine && Material->Bitmap)
	{
		if (!jeEngine_RemoveBitmap(Array->Engine, Material->Bitmap))
		{
			jeErrorLog_AddString(-1, "jeMaterial_ArraySetMaterialBitmap:  jeEngine_RemoveBitmap failed.", NULL);
			return JE_FALSE;
		}
	}

	if (!jeMaterial_SetBitmap(Material, Bitmap, BitmapName))
	{
		jeErrorLog_AddString(-1, "jeMaterial_ArraySetMaterialBitmap:  jeMaterial_SetBitmap failed.", NULL);
		return JE_FALSE;
	}

	if (Array->Engine)
	{
		if (!jeEngine_AddBitmap(Array->Engine, Material->Bitmap, JE_ENGINE_BITMAP_TYPE_3D))
		{
			jeErrorLog_AddString(-1, "jeMaterial_ArraySetMaterialBitmap:  jeEngine_RemoveBitmap failed.", NULL);
			return JE_FALSE;
		}
	}

	//Material->BitmapKind = JE_RESOURCE_BITMAP;

	return JE_TRUE;
#else
	return JE_FALSE;
#endif
}

JETAPI jeBoolean JETCC jeMaterial_ArraySetMaterialSpec(jeMaterial_Array *Array, jeMaterial_ArrayIndex Index, jeMaterialSpec *MatSpec, const char *BitmapName)
{
	jeBitmap*	pBitmap;
	jeMaterial* pMaterial;

	pMaterial = (jeMaterial*)jeArray_GetElement(Array->Array, Index);

	if (!pMaterial)
		return JE_FALSE;

#ifndef _USE_BITMAPS
	if (pMaterial->MatSpec) {
		// remove the bitmap if any, if texture don't do anything here
		pBitmap = jeMaterialSpec_GetLayerBitmap(pMaterial->MatSpec, 0);
		if (pBitmap && Array->Engine) {
			if (!jeEngine_RemoveBitmap(Array->Engine, pBitmap))
			{
				jeErrorLog_AddString(-1, "jeMaterial_ArraySetMaterialBitmap:  jeEngine_RemoveBitmap failed.", NULL);
				return JE_FALSE;
			}
		}
	}

	jeMaterialSpec_CreateRef(MatSpec);
	pMaterial->MatSpec = MatSpec;

    pMaterial->ResourceKind = JE_RESOURCE_MATERIAL;
    strcpy(pMaterial->BitmapName, BitmapName);

	// now add the new bitmap if any, nothing to do if the spec contains jeTexture
	pBitmap = jeMaterialSpec_GetLayerBitmap(pMaterial->MatSpec, 0);
	if (pBitmap && Array->Engine)
	{
		if (!jeEngine_AddBitmap(Array->Engine, pBitmap, JE_ENGINE_BITMAP_TYPE_3D))
		{
			jeErrorLog_AddString(-1, "jeMaterial_ArraySetMaterialBitmap:  jeEngine_RemoveBitmap failed.", NULL);
			return JE_FALSE;
		}
	}
	
	return JE_TRUE;
#else
	return JE_FALSE;
#endif
}

//=======================================================================================
//	jeMaterial_ArrayGetNextMaterial
//=======================================================================================
JETAPI jeMaterial * JETCC jeMaterial_ArrayGetNextMaterial(jeMaterial_Array *Array, const jeMaterial *Start)
{
	return (jeMaterial*)jeArray_GetNextElement(Array->Array, (void*)Start);
}

//=======================================================================================
//	jeMaterial_ArraySetEngine
//=======================================================================================
jeBoolean jeMaterial_ArraySetEngine(jeMaterial_Array *Array, jeEngine *Engine)
{
	jeMaterial		*Material;

	Material = NULL;
	while (Material = jeMaterial_ArrayGetNextMaterial(Array, Material))
	{
#ifdef _USE_BITMAPS
		if (!Material->Bitmap)
			continue;

		// If there was a previous engine, remove the bitmap from that one, and attach it to the new one
		if (Array->Engine)
		{
			if (!jeEngine_RemoveBitmap(Array->Engine, Material->Bitmap))
			{
				jeErrorLog_AddString(-1, "jeMaterial_ArraySetEngine:  jeEngine_RemoveBitmap failed.", NULL);
				return JE_FALSE;
			}
		}

		if (!jeEngine_AddBitmap(Engine, Material->Bitmap, JE_ENGINE_BITMAP_TYPE_3D))
		{
			jeErrorLog_AddString(-1, "jeMaterial_ArraySetEngine:  jeEngine_RemoveBitmap failed.", NULL);
			return JE_FALSE;
		}
#else
#pragma message("Krouer: Must take into account the specification of the new texture mgr")
		jeBitmap* pBitmap;

		if (!Material->MatSpec) {
			continue;
		}

		pBitmap = jeMaterialSpec_GetLayerBitmap(Material->MatSpec, 0);
		// nothing to do if spec uses jeTexture
		if (pBitmap) {
			// If there was a previous engine, remove the bitmap from that one, and attach it to the new one
			if (Array->Engine)
			{
				if (!jeEngine_RemoveBitmap(Array->Engine, pBitmap))
				{
					jeErrorLog_AddString(-1, "jeMaterial_ArraySetEngine:  jeEngine_RemoveBitmap failed.", NULL);
					return JE_FALSE;
				}
			}

			if (!jeEngine_AddBitmap(Engine, pBitmap, JE_ENGINE_BITMAP_TYPE_3D))
			{
				jeErrorLog_AddString(-1, "jeMaterial_ArraySetEngine:  jeEngine_RemoveBitmap failed.", NULL);
				return JE_FALSE;
			}
		}
#endif
	}

	if (Array->Engine)
		jeEngine_Destroy(&Array->Engine);	// Icestorm: We want to be sure Engine will be valid all the time!

	Array->Engine = Engine;
	jeEngine_CreateRef(Engine);	// Icestorm

	return JE_TRUE;
}

/*
#if 1

#define JE_MATERIAL_LAYER_FLAG_BITMAP		(1<<0)
#define JE_MATERIAL_LAYER_FLAG_LIGHTMAP		(1<<1)

typedef struct jeMaterial_Layer
{
	uint8				Flags;
	jeBitmap			*Bitmap;

	// Local ScaleU/ScaleV to this layer only
	jeFloat				ShiftU;
	jeFloat				ShiftV;
	jeFloat				ScaleU;
	jeFloat				ScaleV;
} jeMaterial_Layer;

#define JE_MATERIAL_FLAG_VIS_PORTAL			(1<<0)

typedef struct jeMaterial
{
	uint16				RefCount;
	jeMaterial			*Parent;

	uint32				Flags;

	uint32				AmbientColor;	// RGBA (8 bits for each component, 0..255)

	// Scale and rotation of all layers
	jeFloat				BaseShiftU;
	jeFloat				BaseShiftV;
	jeFloat				BaseScaleU;
	jeFloat				BaseScaleV;
	jeFloat				BaseRotate;

	uint8				NumLayers;		// Maximum of 255 layers 
	jeMaterial_Layer	*Layers;		// Array of allocated layers
} jeMaterial;

#else

#define JE_MATERIAL_HAS_BASESCALEUVR	(1<<0)			// Material has Scale UVR in material

typedef struct jeMaterial
{
	jeMaterial_Def			*Def;

	uint16					RefCount;

	uint32					Flags;
	void					*Data;

} jeMaterial;

typedef struct 
{
	jeFloat		ShiftU;
	jeFloat		ShiftV;
} jeMaterial_BaseShiftUV;

typedef struct 
{
	jeFloat		ScaleU;
	jeFloat		ScaleV;
} jeMaterial_BaseScaleUV;

typedef struct 
{
	jeFloat		Rotate;
} jeMaterial_BaseRotate;

uint32 MaterialFieldSizes[] = {
	sizeof(jeMaterial_BaseScaleUVR),
};

//=======================================================================================
//	jeMaterial_GetBaseScaleUV
//=======================================================================================
void jeMaterial_GetBaseScaleUV(jeMaterial *Material, jeFloat *ScaleU, jeFloat *ScaleV)
{
	assert(Material);
	assert(Material->Def);
	assert(ScaleU);
	assert(ScaleV);

	Flags = Material->Flags;

	if (Flags & JE_MATERIAL_HAS_BASESCALEUVR)
	{
		uint8		*Data;
		int32		i;
		uint32		Mask;

		Data = Material->Data;

		// Add up all the sizes of the valid fields to get the offset of the curent field
		for (i=0, Mask = 0; Mask <= JE_MATERIAL_HAS_BASESCALEUVR; i++, Mask+=Mask)
		{
			if (Flags & Mask)
				Data += MaterialFieldSizes[i];
		}

		*ScaleU = (jeMaterial_BaseScaleUVR*)Data)->ScaleU;
		*ScaleV = (jeMaterial_BaseScaleUVR*)Data)->ScaleV;
	}
	else
	{
		Def = Material->Def;

		*ScaleU = Def->BaseScaleU;
		*ScaleV = Def->BaseScaleV;
	}
}
#endif

//=======================================================================================
//	jeMaterial_GetLayer
//=======================================================================================
jeMaterial_Layer *jeMaterial_GetLayer(jeMaterial *Material, uint8 Layer)
{
	assert(Material);

}
*/

