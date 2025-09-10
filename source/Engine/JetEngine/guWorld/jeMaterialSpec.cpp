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

#include "DCommon.h"
// Private dependents
#include "jeMaterial._h"
#include "Errorlog.h"
#include "Array.h"
#include "Ram.h"
#include "Engine.h"

#include "jeGArray.h"

#include "jeResource.h"

#include "log.h"
#include "jeTexture.h"

//========================================================================================
//========================================================================================
#define ZeroMem(a) memset(a, 0, sizeof(*a))
#define ZeroMemArray(a, s) memset(a, 0, sizeof(*a)*s)


//=======================================================================================
//	** jeMaterialSpec **
//=======================================================================================
#define MATSPEC_DIFFUSE_FLAG	0x0001
#define MATSPEC_SPECULAR_FLAG	0x0002
#define MATSPEC_AMBIENT_FLAG	0x0004
#define MATSPEC_EMISSIVE_FLAG	0x0008
#define MATSPEC_SHADER_FLAG		0x0010
#define MATSPEC_THUMBS_FLAG		0x0020
#define MATSPEC_SIZE_FLAG		0x0040

//=======================================================================================
//	jeMaterialSpec_Create
//=======================================================================================
JETAPI jeMaterialSpec * JETCC jeMaterialSpec_Create(jeEngine* pEngine, jeResourceMgr* pResourceMgr)
{
	jeMaterialSpec	*MaterialSpec;

	MaterialSpec = JE_RAM_ALLOCATE_STRUCT(jeMaterialSpec);

	if (!MaterialSpec)
		return NULL;

	ZeroMem(MaterialSpec);
	MaterialSpec->pEngine = pEngine;

	MaterialSpec->RefCnt = 1;

	return MaterialSpec;
}

JETAPI jeBoolean JETCC jeMaterialSpec_CreateRef(jeMaterialSpec* MaterialSpec)
{
	if (MaterialSpec) {
		MaterialSpec->RefCnt++;
		return JE_TRUE;
	}
	return JE_FALSE;
}

//=======================================================================================
//	jeMaterialSpec_Destroy
//=======================================================================================
JETAPI void JETCC jeMaterialSpec_Destroy(jeMaterialSpec **MaterialSpec)
{
	int idx;
	jeMaterialSpec* pMatSpec = *MaterialSpec;

	if (!(--pMatSpec->RefCnt)) {
		// empty the layers
		for (idx=0; idx<pMatSpec->LayerCounts; idx++) {
			if (pMatSpec->pLayers[idx]) {
				if (pMatSpec->pLayers[idx]->Kind == JE_RESOURCE_BITMAP) {
					jeBitmap_Destroy(&pMatSpec->pLayers[idx]->pBitmap);
					jeResource_Delete(jeResourceMgr_GetSingleton(), pMatSpec->pLayers[idx]->Name);
				} else {
					jeResource_ReleaseResource(jeResourceMgr_GetSingleton(), pMatSpec->pLayers[idx]->Kind, pMatSpec->pLayers[idx]->Name);
				}

				jeRam_Free(pMatSpec->pLayers[idx]);
			}
		}

		// destroy the shader

		// free the resource
		jeRam_Free(*MaterialSpec);

		*MaterialSpec = NULL;
	}
}

#define MAKEFOURCC(ch0, ch1, ch2, ch3)                              \
		((uint32)(uint8)(ch0) | ((uint32)(uint8)(ch1) << 8) |   \
		((uint32)(uint8)(ch2) << 16) | ((uint32)(uint8)(ch3) << 24 ))

#define JE_MATSPEC_TAG			MAKEFOURCC('J', 'M', 'A', 'T')		// 'J' 'MAT'erial definition
#define JE_MATSPEC_VERSION		0x0001

//========================================================================================
//	jeMaterialSpec_CreateFromFile
//========================================================================================
JETAPI jeMaterialSpec* JETCC jeMaterialSpec_CreateFromFile(jeVFile *VFile, jeEngine* pEngine, jeResourceMgr *ResMgr)
{
	uint8 Version;
	uint16 Flags;
	uint32 Tag;
	jeRGBA Color;
	char ShaderName[JE_MATERIAL_MAX_NAME_SIZE];
	jeMaterialSpec	*MaterialSpec;

	MaterialSpec = JE_RAM_ALLOCATE_STRUCT(jeMaterialSpec);

	if (!MaterialSpec) {
		goto ExitInError;
	}
	ZeroMem(MaterialSpec);

	MaterialSpec->pEngine = pEngine;
	MaterialSpec->RefCnt = 1;
	MaterialSpec->Height = MaterialSpec->Width = 0;

	// Read the JMAT tag
	if (!jeVFile_Read(VFile, &Tag, sizeof(Tag))) {
		goto ExitInError;
	}
	if (Tag != JE_MATSPEC_TAG) {
		goto ExitInError;
	}

	// Read the JMAT version
	if (!jeVFile_Read(VFile, &Version, sizeof(Version))) {
		goto ExitInError;
	}
	// Error if the version is not supported
	if (Version > JE_MATSPEC_VERSION) {
		goto ExitInError;
	}

	// Read the material spec flags
	if (!jeVFile_Read(VFile, &Flags, sizeof(Flags))) {
		goto ExitInError;
	}
	MaterialSpec->Flags = Flags;

	// Read the layer count
	if (!jeVFile_Read(VFile, &Version, sizeof(Version))) {
		goto ExitInError;
	}
	MaterialSpec->LayerCounts = Version;

	// Read the color and assign them
	// 1st Read the Diffuse color if specified in the flags
	if (MaterialSpec->Flags&MATSPEC_DIFFUSE_FLAG) {
		if (!jeVFile_Read(VFile, &Color, sizeof(Color))) {
			goto ExitInError;
		}
		MaterialSpec->Diffuse = Color;
	}

	// 2nd Read the Specular color if specified in the flags
	if (MaterialSpec->Flags&MATSPEC_SPECULAR_FLAG) {
		if (!jeVFile_Read(VFile, &Color, sizeof(Color))) {
			goto ExitInError;
		}
		MaterialSpec->Specular = Color;
	}

	// 3rd Read the Ambient color if specified in the flags
	if (MaterialSpec->Flags&MATSPEC_AMBIENT_FLAG) {
		if (!jeVFile_Read(VFile, &Color, sizeof(Color))) {
			goto ExitInError;
		}
		MaterialSpec->Ambient = Color;
	}

	// 4th Read the Emissive color if specified in the flags
	if (MaterialSpec->Flags&MATSPEC_EMISSIVE_FLAG) {
		if (!jeVFile_Read(VFile, &Color, sizeof(Color))) {
			goto ExitInError;
		}
		MaterialSpec->Emissive = Color;
	}

	// Read the Shader name if specified in the flags
	if (MaterialSpec->Flags&MATSPEC_SHADER_FLAG) {
		if (!jeVFile_Read(VFile, ShaderName, JE_MATERIAL_MAX_NAME_SIZE)) {
			goto ExitInError;
		}
		MaterialSpec->pShader = (jeShader*) jeResource_GetResource(jeResourceMgr_GetSingleton(), JE_RESOURCE_SHADER, ShaderName);
	}

	if (MaterialSpec->Flags&MATSPEC_SIZE_FLAG) {
		jeVFile_Read(VFile, &MaterialSpec->Width, sizeof(MaterialSpec->Width));
		jeVFile_Read(VFile, &MaterialSpec->Height, sizeof(MaterialSpec->Height));
	}

	// Read the layer descriptions
	for (Version=0; Version<MaterialSpec->LayerCounts; Version++) {
		jeMaterialSpec_Layer* pLayer = JE_RAM_ALLOCATE_STRUCT(jeMaterialSpec_Layer);
		jeXForm3d_SetIdentity(&pLayer->XForm);

		// Read the resource type
		if (!jeVFile_Read(VFile, &pLayer->Kind, sizeof(pLayer->Kind))) {
			goto ExitInError;
		}

		// Read the layer type
		if (!jeVFile_Read(VFile, &pLayer->Type, sizeof(pLayer->Type))) {
			goto ExitInError;
		}

		// Read the layer UV mapper
		if (!jeVFile_Read(VFile, &pLayer->UVMapID, sizeof(pLayer->UVMapID))) {
			goto ExitInError;
		}

		// Read the Layer resource name
		if (!jeVFile_Read(VFile, pLayer->Name, JE_MATERIAL_MAX_NAME_SIZE)) {
			goto ExitInError;
		}

		// Create the texture from the resource manager
		pLayer->pTexture = (jeTexture*) jeResource_GetResource(jeResourceMgr_GetSingleton(), pLayer->Kind, pLayer->Name);

		if (!jeVFile_Read(VFile, &pLayer->XForm, sizeof(pLayer->XForm))) {
			goto ExitInError;
		}

		if (pLayer->Kind == JE_RESOURCE_BITMAP) {
			jeBitmap_CreateRef((jeBitmap*)pLayer->pTexture);
		}

		MaterialSpec->pLayers[Version] = pLayer;
	}

	// Write the thumbnail image
	if (MaterialSpec->Flags&MATSPEC_THUMBS_FLAG) {
		MaterialSpec->pThumbnail = JE_RAM_ALLOCATE_STRUCT(jeMaterialSpec_Thumbnail);
		if (MaterialSpec->pThumbnail == NULL) {
			goto ExitInError;
		}

		if (!jeVFile_Read(VFile, &MaterialSpec->pThumbnail->width, sizeof(MaterialSpec->pThumbnail->width))) {
			goto ExitInError;
		}
		if (!jeVFile_Read(VFile, &MaterialSpec->pThumbnail->height, sizeof(MaterialSpec->pThumbnail->height))) {
			goto ExitInError;
		}
		Tag = MaterialSpec->pThumbnail->width*MaterialSpec->pThumbnail->height*3;
		MaterialSpec->pThumbnail->contents = (uint8 *)jeRam_Allocate(Tag);
		if (!jeVFile_Read(VFile, MaterialSpec->pThumbnail->contents, Tag)) {
			goto ExitInError;
		}
	}

	if (MaterialSpec->Height == 0 || MaterialSpec->Width == 0) {
		jeMaterialSpec_Layer* pLayer;

		// calc the with and height from layers
		MaterialSpec->Flags |= MATSPEC_SIZE_FLAG;

		pLayer = MaterialSpec->pLayers[0];
		if (pLayer->Kind == JE_RESOURCE_BITMAP) {
			MaterialSpec->Width = (uint16) jeBitmap_Width((jeBitmap*)pLayer->pTexture);
			MaterialSpec->Height = (uint16) jeBitmap_Height((jeBitmap*)pLayer->pTexture);
		} else {
			jeTexture_Info texInfo;
			jeTexture_GetInfo(MaterialSpec->pEngine, pLayer->pTexture, 0, &texInfo);
			MaterialSpec->Width = (uint16) texInfo.Width;
			MaterialSpec->Height = (uint16) texInfo.Height;
		}
	}

	return MaterialSpec;


ExitInError:
	if (MaterialSpec) {
		jeMaterialSpec_Destroy(&MaterialSpec);
	}
	return NULL;
}

JETAPI jeBoolean JETCC jeMaterialSpec_WriteToFile(jeMaterialSpec* MatSpec, jeVFile *VFile)
{
	uint8 Version;
	uint32 Tag;
	assert(MatSpec);

	// Write the JMAT tag
	Tag = JE_MATSPEC_TAG;
	if (!jeVFile_Write(VFile, &Tag, sizeof(Tag))) {
		return JE_FALSE;
	}

	// Write the JMAT version
	Version = JE_MATSPEC_VERSION;
	if (!jeVFile_Write(VFile, &Version, sizeof(Version))) {
		return JE_FALSE;
	}

#pragma message ("Krouer: Remove write shader until they are implemented. Remove the line below when done")
	MatSpec->Flags &= ~MATSPEC_SHADER_FLAG;
	// Write the material spec flags
	if (!jeVFile_Write(VFile, &MatSpec->Flags, sizeof(MatSpec->Flags))) {
		return JE_FALSE;
	}

	// Write the layer count
	Version = (uint8) MatSpec->LayerCounts;
	if (!jeVFile_Write(VFile, &Version, sizeof(Version))) {
		return JE_FALSE;
	}

	// Write the colors
	// 1st Write the Diffuse color if specified in the flags
	if (MatSpec->Flags&MATSPEC_DIFFUSE_FLAG) {
		if (!jeVFile_Write(VFile, &MatSpec->Diffuse, sizeof(MatSpec->Diffuse))) {
			return JE_FALSE;
		}
	}

	// 2nd Write the Specular color if specified in the flags
	if (MatSpec->Flags&MATSPEC_SPECULAR_FLAG) {
		if (!jeVFile_Write(VFile, &MatSpec->Specular, sizeof(MatSpec->Specular))) {
			return JE_FALSE;
		}
	}

	// 3rd Write the Ambient color if specified in the flags
	if (MatSpec->Flags&MATSPEC_AMBIENT_FLAG) {
		if (!jeVFile_Write(VFile, &MatSpec->Ambient, sizeof(MatSpec->Ambient))) {
			return JE_FALSE;
		}
	}

	// 4th Write the Emissive color if specified in the flags
	if (MatSpec->Flags&MATSPEC_EMISSIVE_FLAG) {
		if (!jeVFile_Write(VFile, &MatSpec->Emissive, sizeof(MatSpec->Emissive))) {
			return JE_FALSE;
		}
	}

	// Write the Shader name if specified in the flags
	if (MatSpec->Flags&MATSPEC_SHADER_FLAG) {
		//char ShaderName[JE_MATERIAL_MAX_NAME_SIZE];
#pragma message ("Krouer: jeShader must have access on their physical name: jeShader_GetName")
		//jeShader_GetName(MatSpec->pShader, ShaderName, JE_MATERIAL_MAX_NAME_SIZE);

		/*
		if (!jeVFile_Write(VFile, ShaderName, JE_MATERIAL_MAX_NAME_SIZE)) {
			return JE_FALSE;
		}
		*/
	}

	if (MatSpec->Flags&MATSPEC_SIZE_FLAG) {
		jeVFile_Write(VFile, &MatSpec->Width, sizeof(MatSpec->Width));
		jeVFile_Write(VFile, &MatSpec->Height, sizeof(MatSpec->Height));
	}

	// Write the layers descriptions
	for (Version=0; Version<MatSpec->LayerCounts; Version++) {
		jeMaterialSpec_Layer* pLayer = MatSpec->pLayers[Version];

		// Write the resource type
		if (!jeVFile_Write(VFile, &pLayer->Kind, sizeof(pLayer->Kind))) {
			return JE_FALSE;
		}

		// Write the layer type
		if (!jeVFile_Write(VFile, &pLayer->Type, sizeof(pLayer->Type))) {
			return JE_FALSE;
		}

		// Write the layer UV mapper
		if (!jeVFile_Write(VFile, &pLayer->UVMapID, sizeof(pLayer->UVMapID))) {
			return JE_FALSE;
		}

		// Write the Layer resource name
		if (!jeVFile_Write(VFile, pLayer->Name, JE_MATERIAL_MAX_NAME_SIZE)) {
			return JE_FALSE;
		}

		// Write the XForm matrix of the layer
		if (!jeVFile_Write(VFile, &pLayer->XForm, sizeof(pLayer->XForm))) {
			return JE_FALSE;
		}
	}

	// Write the thumbnail image
	if (MatSpec->Flags&MATSPEC_THUMBS_FLAG) {
		if (!jeVFile_Write(VFile, &MatSpec->pThumbnail->width, sizeof(MatSpec->pThumbnail->width))) {
			return JE_FALSE;
		}
		if (!jeVFile_Write(VFile, &MatSpec->pThumbnail->height, sizeof(MatSpec->pThumbnail->height))) {
			return JE_FALSE;
		}
		if (!jeVFile_Write(VFile, MatSpec->pThumbnail->contents, (MatSpec->pThumbnail->width*MatSpec->pThumbnail->height*3))) {
			return JE_FALSE;
		}
	}

	return JE_TRUE;
}

JETAPI uint32 JETCC jeMaterialSpec_GetLayerCount(const jeMaterialSpec* MatSpec)
{
	assert(MatSpec);
	return MatSpec->LayerCounts;
}

JETAPI jeBoolean JETCC jeMaterialSpec_AddLayer(jeMaterialSpec* MatSpec, int32 layerIndex, int32 Kind, jeMaterialSpec_LayerType layerType, int32 layerMapper, const char* LayerName)
{
	jeMaterialSpec_Layer* pLayer;
	assert(MatSpec);

	if (layerIndex>=JE_MATERIAL_MAX_LAYER || layerIndex<0) {
		return JE_FALSE;
	}

	pLayer = MatSpec->pLayers[layerIndex];
	
	if (pLayer) {
#pragma message ("Krouer: jeTexture_Destroy must have access on their physical name: jeShader_GetName")
		//jeTexture_Destroy(MatSpec->pEngine, pLayer->pTexture);
	} else {
		pLayer = JE_RAM_ALLOCATE_STRUCT(jeMaterialSpec_Layer);
		ZeroMem(pLayer);
		MatSpec->pLayers[layerIndex] = pLayer;
		MatSpec->LayerCounts++;
	}
	strcpy(pLayer->Name, LayerName);
	pLayer->Kind = (uint16) Kind;
	jeXForm3d_SetIdentity(&pLayer->XForm);

	pLayer->pTexture = (jeTexture*) jeResource_GetResource(jeResourceMgr_GetSingleton(), pLayer->Kind, pLayer->Name);

	if (pLayer->Kind == JE_RESOURCE_BITMAP) {
		jeBitmap_CreateRef((jeBitmap*)pLayer->pTexture);
	}

	pLayer->Type = (uint8) layerType;
	pLayer->UVMapID = (uint8) layerMapper;

	if (pLayer->pTexture) {
		return JE_TRUE;
	}
	return JE_FALSE;
}

JETAPI jeBoolean JETCC jeMaterialSpec_RemoveLayer(jeMaterialSpec* MatSpec, int32 layerIndex)
{
	assert(MatSpec);

	if (layerIndex>=JE_MATERIAL_MAX_LAYER || layerIndex<0) {
		return JE_FALSE;
	}

	if (MatSpec->pLayers[layerIndex]) {
#pragma message ("Krouer: jeTexture_Destroy must have access on their physical name: jeShader_GetName")
		//jeTexture_Destroy(MatSpec->pEngine, pLayer->pTexture);
		if (MatSpec->pLayers[layerIndex]->Kind == JE_RESOURCE_BITMAP) {
			jeBitmap_Destroy((jeBitmap**)&MatSpec->pLayers[layerIndex]->pTexture);
		}

		// Free the slot
		jeRam_Free(MatSpec->pLayers[layerIndex]);
		MatSpec->pLayers[layerIndex] = NULL;

		// Modify the counter
		MatSpec->LayerCounts--;
	}

	return JE_TRUE;
}

JETAPI jeBoolean JETCC jeMaterialSpec_SetLayerTransform(jeMaterialSpec* MatSpec, int32 layerIndex, jeXForm3d* LayerXFrom)
{
	assert(MatSpec);

	if (layerIndex>=JE_MATERIAL_MAX_LAYER || layerIndex<0) {
		return JE_FALSE;
	}

	if (MatSpec->pLayers[layerIndex]) {
		// assign new matrix
		MatSpec->pLayers[layerIndex]->XForm=*LayerXFrom;
		return JE_TRUE;
	}
	return JE_FALSE;
}

JETAPI jeBoolean JETCC jeMaterialSpec_SetShader(jeMaterialSpec* MatSpec, jeShader* Shader)
{
	assert(MatSpec);
	if (MatSpec->pShader == Shader) return JE_TRUE;
	if (MatSpec->pShader) {
#pragma message ("Krouer: jeShader must have access a destroy function: jeShader_Destroy")
		/* Krouer: wait for new jeShader implementation
		jeShader_Destroy(&MatSpec->pShader); */
	}
	MatSpec->pShader = Shader;
	if (MatSpec->pShader) {
#pragma message ("Krouer: jeShader must have access to a create ref: jeShader_CreateRef")
		/* Krouer: wait for new jeShader implementation
		jeShader_CreateRef(Shader); */
		MatSpec->Flags |= MATSPEC_SHADER_FLAG;
	} else {
		MatSpec->Flags &= ~MATSPEC_SHADER_FLAG;
	}
	return JE_TRUE;
}

JETAPI jeBoolean JETCC jeMaterialSpec_SetColor(jeMaterialSpec* MatSpec, int32 ColorIndex, jeRGBA* Color)
{
	assert(MatSpec);
	switch (ColorIndex) {
	case JE_MATERIALSPEC_DIFFUSE_INDEX:
		if (Color) {
			MatSpec->Flags |= MATSPEC_DIFFUSE_FLAG;
			MatSpec->Diffuse = *Color;
		} else {
			MatSpec->Flags &= ~MATSPEC_DIFFUSE_FLAG;
		}
		break;
	case JE_MATERIALSPEC_SPECULAR_INDEX:
		if (Color) {
			MatSpec->Flags |= MATSPEC_SPECULAR_FLAG;
			MatSpec->Specular = *Color;
		} else {
			MatSpec->Flags &= ~MATSPEC_SPECULAR_FLAG;
		}
		break;
	case JE_MATERIALSPEC_AMBIENT_INDEX:
		if (Color) {
			MatSpec->Flags |= MATSPEC_AMBIENT_FLAG;
			MatSpec->Ambient = *Color;
		} else {
			MatSpec->Flags &= ~MATSPEC_AMBIENT_FLAG;
		}
		break;
	case JE_MATERIALSPEC_EMISSIVE_INDEX:
		if (Color) {
			MatSpec->Flags |= MATSPEC_EMISSIVE_FLAG;
			MatSpec->Emissive = *Color;
		} else {
			MatSpec->Flags &= ~MATSPEC_EMISSIVE_FLAG;
		}
		break;
	default:
		return JE_FALSE;
	}
	return JE_TRUE;
}

JETAPI jeShader* JETCC jeMaterialSpec_GetShader(const jeMaterialSpec* MatSpec)
{
	assert(MatSpec);
	if (MATSPEC_SHADER_FLAG&MatSpec->Flags) return MatSpec->pShader;
	return NULL;
}

JETAPI jeTexture* JETCC jeMaterialSpec_GetLayerTexture(const jeMaterialSpec* MatSpec, int32 layerIndex)
{
	assert(MatSpec);

	if (layerIndex>=JE_MATERIAL_MAX_LAYER || layerIndex<0) {
		return NULL;
	}

	if (MatSpec->pLayers[layerIndex] && MatSpec->pLayers[layerIndex]->Kind != JE_RESOURCE_BITMAP) {
		// return the texture
		return MatSpec->pLayers[layerIndex]->pTexture;
	}
	return NULL;
}

JETAPI jeBitmap* JETCC jeMaterialSpec_GetLayerBitmap(const jeMaterialSpec* MatSpec, int32 layerIndex)
{
	assert(MatSpec);

	if (layerIndex>=JE_MATERIAL_MAX_LAYER || layerIndex<0) {
		return NULL;
	}

	if (MatSpec->pLayers[layerIndex] && MatSpec->pLayers[layerIndex]->Kind == JE_RESOURCE_BITMAP) {
		// return the texture
		return MatSpec->pLayers[layerIndex]->pBitmap;
	}
	return NULL;
}

JETAPI jeXForm3d* JETCC jeMaterialSpec_GetLayerTransform(const jeMaterialSpec* MatSpec, int32 layerIndex)
{
	assert(MatSpec);
	assert(MatSpec);

	if (layerIndex>=JE_MATERIAL_MAX_LAYER || layerIndex<0) {
		return NULL;
	}

	if (MatSpec->pLayers[layerIndex]) {
		// return the texture
		return &MatSpec->pLayers[layerIndex]->XForm;
	}
	return NULL;
}

JETAPI jeRGBA* JETCC jeMaterialSpec_GetColor(const jeMaterialSpec* MatSpec, int32 colorIndex)
{
	jeMaterialSpec* MS;
	assert(MatSpec);
	
	MS = (jeMaterialSpec*) MatSpec;
	if (colorIndex==JE_MATERIALSPEC_DIFFUSE_INDEX && (MatSpec->Flags&MATSPEC_DIFFUSE_FLAG)) return &MS->Diffuse;
	if (colorIndex==JE_MATERIALSPEC_SPECULAR_INDEX && (MatSpec->Flags&MATSPEC_SPECULAR_FLAG)) return &MS->Specular;
	if (colorIndex==JE_MATERIALSPEC_AMBIENT_INDEX && (MatSpec->Flags&MATSPEC_AMBIENT_FLAG)) return &MS->Ambient;
	if (colorIndex==JE_MATERIALSPEC_EMISSIVE_INDEX && (MatSpec->Flags&MATSPEC_EMISSIVE_FLAG)) return &MS->Emissive;
	return NULL;
}

JETAPI uint32 JETCC jeMaterialSpec_GetColors(const jeMaterialSpec* MatSpec, jeRGBA* Diffuse, jeRGBA* Specular, jeRGBA* Ambient, jeRGBA* Emissive)
{
	uint32 colormask;
	assert(MatSpec);

	colormask = 0;

	if (MatSpec->Flags&MATSPEC_DIFFUSE_FLAG) {
		*Diffuse = MatSpec->Diffuse;
		colormask |= MATSPEC_DIFFUSE_FLAG;
	}
	if (MatSpec->Flags&MATSPEC_SPECULAR_FLAG) {
		*Specular = MatSpec->Specular;
		colormask |= MATSPEC_SPECULAR_FLAG;
	}
	if (MatSpec->Flags&MATSPEC_AMBIENT_FLAG) {
		*Ambient = MatSpec->Ambient;
		colormask |= MATSPEC_AMBIENT_FLAG;
	}
	if (MatSpec->Flags&MATSPEC_EMISSIVE_FLAG) {
		*Emissive = MatSpec->Emissive;
		colormask |= MATSPEC_EMISSIVE_FLAG;
	}
	return colormask;
}

JETAPI jeMaterialSpec_Thumbnail* JETCC jeMaterialSpec_GetThumbnail(const jeMaterialSpec* MatSpec)
{
	assert(MatSpec);
	return MatSpec->pThumbnail;
}

JETAPI jeBoolean JETCC jeMaterialSpec_SetThumbnail(jeMaterialSpec* MatSpec, jeMaterialSpec_Thumbnail* pThumb)
{
	int32 ThumbSize;
	assert(MatSpec);
	
	MatSpec->pThumbnail = JE_RAM_ALLOCATE_STRUCT(jeMaterialSpec_Thumbnail);
	if (MatSpec->pThumbnail == NULL) {
		return JE_FALSE;
	}

	ThumbSize = pThumb->width*pThumb->height*3;
	MatSpec->pThumbnail->contents = (uint8 *) jeRam_Allocate(ThumbSize);
	if (MatSpec->pThumbnail->contents == NULL) {
		return JE_FALSE;
	}
	MatSpec->pThumbnail->width = pThumb->width;
	MatSpec->pThumbnail->height = pThumb->height;
	memcpy(MatSpec->pThumbnail->contents, pThumb->contents, ThumbSize);

	MatSpec->Flags |= MATSPEC_THUMBS_FLAG;
	return JE_TRUE;
}

JETAPI uint32 JETCC jeMaterialSpec_Height(const jeMaterialSpec* MatSpec)
{
	if (MatSpec->Height == 0 || MatSpec->Width == 0) {
		jeMaterialSpec_Layer* pLayer;

		// calc the with and height from layers
		((jeMaterialSpec*)MatSpec)->Flags |= MATSPEC_SIZE_FLAG;

		pLayer = MatSpec->pLayers[0];
		if (pLayer->Kind == JE_RESOURCE_BITMAP) {
			((jeMaterialSpec*)MatSpec)->Width = (uint16) jeBitmap_Width((jeBitmap*)pLayer->pTexture);
			((jeMaterialSpec*)MatSpec)->Height = (uint16) jeBitmap_Height((jeBitmap*)pLayer->pTexture);
		} else {
			jeTexture_Info texInfo;
			jeTexture_GetInfo(MatSpec->pEngine, pLayer->pTexture, 0, &texInfo);
			((jeMaterialSpec*)MatSpec)->Width = (uint16) texInfo.Width;
			((jeMaterialSpec*)MatSpec)->Height = (uint16) texInfo.Height;
		}
	}
	return MatSpec->Height;
}

JETAPI uint32 JETCC jeMaterialSpec_Width(const jeMaterialSpec* MatSpec)
{
	if (MatSpec->Height == 0 || MatSpec->Width == 0) {
		jeMaterialSpec_Layer* pLayer;

		// calc the with and height from layers
		((jeMaterialSpec*)MatSpec)->Flags |= MATSPEC_SIZE_FLAG;

		pLayer = MatSpec->pLayers[0];
		if (pLayer->Kind == JE_RESOURCE_BITMAP) {
			((jeMaterialSpec*)MatSpec)->Width = (uint16) jeBitmap_Width((jeBitmap*)pLayer->pTexture);
			((jeMaterialSpec*)MatSpec)->Height = (uint16) jeBitmap_Height((jeBitmap*)pLayer->pTexture);
		} else {
			jeTexture_Info texInfo;
			jeTexture_GetInfo(MatSpec->pEngine, pLayer->pTexture, 0, &texInfo);
			((jeMaterialSpec*)MatSpec)->Width = (uint16) texInfo.Width;
			((jeMaterialSpec*)MatSpec)->Height = (uint16) texInfo.Height;
		}
	}
	return MatSpec->Width;
}

JETAPI jeBoolean JETCC jeMaterialSpec_AddLayerFromFile(jeMaterialSpec* MatSpec, int32 layerIndex, jeVFile *File, jeBoolean UseColorKey, uint32 ColorKey)
{
	jeBitmap* bmp;

	if (layerIndex>=JE_MATERIAL_MAX_LAYER || layerIndex<0) {
		return JE_FALSE;
	}

	MatSpec->pLayers[layerIndex] = JE_RAM_ALLOCATE_STRUCT(jeMaterialSpec_Layer);
	if ((bmp = jeBitmap_CreateFromFile(File)) == NULL) {
		MatSpec->pLayers[layerIndex]->pTexture = jeTexture_CreateFromFile(MatSpec->pEngine, File);
		MatSpec->pLayers[layerIndex]->Kind = JE_RESOURCE_TEXTURE;
	} else {
		MatSpec->pLayers[layerIndex]->pTexture = (jeTexture*) bmp;
        jeBitmap_SetColorKey(bmp, UseColorKey, ColorKey, JE_TRUE);

        MatSpec->pLayers[layerIndex]->Kind = JE_RESOURCE_BITMAP;
		jeEngine_AddBitmap(MatSpec->pEngine, bmp, JE_ENGINE_BITMAP_TYPE_3D);
	}
	if (MatSpec->pLayers[layerIndex]->pTexture==NULL) {
		jeRam_Free(MatSpec->pLayers[layerIndex]);
		return JE_FALSE;
	}

	jeXForm3d_SetIdentity(&MatSpec->pLayers[layerIndex]->XForm);
	MatSpec->pLayers[layerIndex]->Type = 0;
	MatSpec->pLayers[layerIndex]->UVMapID = 0;
	MatSpec->LayerCounts++;

	return JE_TRUE;
}

JETAPI jeBoolean JETCC jeMaterialSpec_AddLayerFromBitmap(jeMaterialSpec* MatSpec, int32 layerIndex, jeBitmap* pBitmap, const char* ResName)
{
	if (layerIndex>=JE_MATERIAL_MAX_LAYER || layerIndex<0) {
		return JE_FALSE;
	}

    // Store the bitmap in the Texture slot
	MatSpec->pLayers[layerIndex] = JE_RAM_ALLOCATE_STRUCT(jeMaterialSpec_Layer);
	MatSpec->pLayers[layerIndex]->pTexture = (jeTexture*) pBitmap;
	MatSpec->pLayers[layerIndex]->Kind = JE_RESOURCE_BITMAP;
    // if trying to create a Material with a NULL bitmap
	if (MatSpec->pLayers[layerIndex]->pTexture==NULL) {
		jeRam_Free(MatSpec->pLayers[layerIndex]);
		return JE_FALSE;
	}

    memset(MatSpec->pLayers[layerIndex]->Name, 0, JE_MATERIAL_MAX_NAME_SIZE);
    if (ResName) {
        strcpy(MatSpec->pLayers[layerIndex]->Name, ResName);
    }

    // To avoid problem, add a ref on the bitmap
    jeBitmap_CreateRef(pBitmap);
    if (layerIndex == 0) {
        MatSpec->Width = jeBitmap_Width(pBitmap);
        MatSpec->Height = jeBitmap_Height(pBitmap);
    }

	jeXForm3d_SetIdentity(&MatSpec->pLayers[layerIndex]->XForm);
	MatSpec->pLayers[layerIndex]->Type = 0;
	MatSpec->pLayers[layerIndex]->UVMapID = 0;
	MatSpec->LayerCounts++;

	return JE_TRUE;
}