/*!
	@file jeMaterial.h 
	
	@author John Pollard
	@brief Material accessor and Material array usage

	@par Licence
	The contents of this file are subject to the Jet3D Public License       
	Version 1.02 (the "License"); you may not use this file except in         
	compliance with the License. You may obtain a copy of the License at       
	http://www.jet3d.com                                                        
                                                                             
	@par
	Software distributed under the License is distributed on an "AS IS"           
	basis, WITHOUT WARRANTY OF ANY KIND, either express or implied.  See           
	the License for the specific language governing rights and limitations          
	under the License.                                                               
                                                                                  
	@par
	The Original Code is Jet3D, released December 12, 1999.                            
	Copyright (C) 1996-1999 Eclipse Entertainment, L.L.C. All Rights Reserved           
*/

#ifndef JE_MATERIAL_H
#define JE_MATERIAL_H

#include "jeTypes.h"
#include "Array.h"
#include "Bitmap.h"
#include "VFile.h"
#include "Engine.h"
#include "jePtrMgr.h"

#ifdef __cplusplus
extern "C" {
#endif

//=======================================================================================
//	
//=======================================================================================
/*! @def JE_MATERIAL_ARRAY_NULL_INDEX
	@brief Define the NULL index indicator
*/
#define JE_MATERIAL_ARRAY_NULL_INDEX	JE_ARRAY_NULL_INDEX

/*! @def JE_MATERIAL_MAX_NAME_SIZE
	@brief Define the maximal lenght of a material name
*/
#define JE_MATERIAL_MAX_NAME_SIZE		256

//=======================================================================================
//	Function prototypes
//=======================================================================================
/*! @name jeMaterialSpec related functions and data
	@{
*/
/*! @def JE_MATERIALSPEC_DIFFUSE_INDEX
	@brief Define the diffuse color index for function jeMaterialSpec_SetColor
*/
#define JE_MATERIALSPEC_DIFFUSE_INDEX		0
/*! @def JE_MATERIALSPEC_SPECULAR_INDEX
	@brief Define the specular color index for function jeMaterialSpec_SetColor
*/
#define JE_MATERIALSPEC_SPECULAR_INDEX		1
/*! @def JE_MATERIALSPEC_AMBIENT_INDEX
	@brief Define the ambient color index for function jeMaterialSpec_SetColor
*/
#define JE_MATERIALSPEC_AMBIENT_INDEX		2
/*! @def JE_MATERIALSPEC_EMISSIVE_INDEX
	@brief Define the emissive color index for function jeMaterialSpec_SetColor
*/
#define JE_MATERIALSPEC_EMISSIVE_INDEX		3

typedef struct jeTexture jeTexture;
typedef struct jeShader jeShader;
typedef struct jeXForm3d jeXForm3d;

typedef enum jeMaterialSpec_LayerType
{
    JE_MATERIAL_LAYER_BASE=0,
    JE_MATERIAL_LAYER_ALPHA
} jeMaterialSpec_LayerType;

typedef struct jeMaterialSpec_Thumbnail
{
	uint8  width;
	uint8  height;
	uint8* contents;
} jeMaterialSpec_Thumbnail;

/*! @typedef jeMaterialSpec
*   @brief A JMAT file content description
*   @see jeMaterial
*	@see jeTexture
*/
typedef struct							jeMaterialSpec		jeMaterialSpec;	

/*! @fn jeMaterialSpec* jeMaterialSpec_Create()
*   @brief Create an empty jeMaterialSpec instance
	@param pEngine The engine associate with this material
	@param pResourceMgr The resource manager for accessing dependancies
	@return The jeMaterialSpec instance created if succeed, NULL otherwise
*   @see jeMaterial
*	@see jeTexture
*/
JETAPI jeMaterialSpec* JETCC jeMaterialSpec_Create(jeEngine* pEngine, jeResourceMgr* pResourceMgr);

JETAPI jeBoolean JETCC jeMaterialSpec_CreateRef(jeMaterialSpec* MaterialSpec);

/*! @fn jeMaterialSpec* jeMaterialSpec_CreateFromFile(jeVFile *VFile, jePtrMgr *PtrMgr)
*   @brief Create a jeMaterialSpec instance from the JMAT file
	@param VFile The file already opened for read operations
	@param pEngine The engine associate with this material
	@param ResMgr The resource manager instance
	@return The jeMaterialSpec instance created if succeed, NULL otherwise
*   @see jeMaterial
*	@see jeTexture
*/
JETAPI jeMaterialSpec* JETCC jeMaterialSpec_CreateFromFile(jeVFile *VFile, jeEngine* pEngine, jeResourceMgr *ResMgr);

/*! @fn void jeMaterialSpec_Destroy(jeMaterialSpec **ppMaterialSpec);
*   @brief Destroy the jeMaterialSpec instance
	@param ppMaterialSpec The jeMaterialSpec instance pointer address
	@note The *ppMaterialSpec is set to NULL before returning
*/
JETAPI void JETCC jeMaterialSpec_Destroy(jeMaterialSpec **ppMaterialSpec);

/*! @fn jeBoolean jeMaterialSpec_WriteToFile(jeMaterialSpec* MatSpec, jeVFile *VFile)
*   @brief Write the jeMaterialSpec instance into a JMAT file
	@param MatSpec The jeMaterialSpec instance to write
	@param VFile The JMAT file already opened for write operations
	@return JE_TRUE if succeed, JE_FALSE otherwise
*/
JETAPI jeBoolean JETCC jeMaterialSpec_WriteToFile(jeMaterialSpec* MatSpec, jeVFile *VFile);

/*! @fn jeBoolean jeMaterialSpec_AddLayer(jeMaterialSpec* MatSpec, int32 layerIndex, int32 Kind, jeMaterialSpec_LayerType layerType, int32 layerMapper, const char* LayerName)
*   @brief Add a layer from a resource identifier to the current jeMaterialSpec instance
	@param MatSpec The jeMaterialSpec instance to modify
	@param layerIndex The layer index 0 based of the jeMaterialSpec to modify
    @param Kind The resource type identifier
    @param layerType The layer behavior identifier
    @param layerMapper The UV mapper identifier
    @param LayerNam The layer resource identifier
	@return JE_TRUE if succeed, JE_FALSE otherwise

    @todo check if layerType is not a duplicate of layerIndex
    @see jeResource.h for the Resource type list of possible values
*/
JETAPI jeBoolean JETCC jeMaterialSpec_AddLayer(jeMaterialSpec* MatSpec, int32 layerIndex, int32 Kind, jeMaterialSpec_LayerType layerType, int32 layerMapper, const char* LayerName);

/*! @fn jeBoolean jeMaterialSpec_AddLayerFromFile(jeMaterialSpec* MatSpec, int32 layerIndex, jeVFile *File, jeBoolean UseColorKey, uint32 ColorKey)
*   @brief Add a layer from a file to the current jeMaterialSpec instance
	@param MatSpec The jeMaterialSpec instance to modify
	@param layerIndex The layer index 0 based of the jeMaterialSpec to modify
    @param File The file that contains the layer definition (jeTexture or jeBitmap)
    @param UseColorKey Does the new material must use a colorkey, valid only if the File contains a jeBitmap
    @param ColorKey The Color key palette index value
	@return JE_TRUE if succeed, JE_FALSE otherwise
*/
JETAPI jeBoolean JETCC jeMaterialSpec_AddLayerFromFile(jeMaterialSpec* MatSpec, int32 layerIndex, jeVFile *File, jeBoolean UseColorKey, uint32 ColorKey);

/*! @fn jeBoolean jeMaterialSpec_AddLayerFromBitmap(jeMaterialSpec* MatSpec, int32 layerIndex, jeBitmap* Bitmap)
*   @brief Add a layer from a file to the current jeMaterialSpec instance
	@param MatSpec The jeMaterialSpec instance to modify
	@param layerIndex The layer index 0 based of the jeMaterialSpec to modify
    @param pBitmap The layer content provided by a jeBitmap instance already initialised
    @param ResName The resource name identifer for the jeBitmap
	@return JE_TRUE if succeed, JE_FALSE otherwise
*/
JETAPI jeBoolean JETCC jeMaterialSpec_AddLayerFromBitmap(jeMaterialSpec* MatSpec, int32 layerIndex, jeBitmap* Bitmap, const char* ResName);

/*! @fn jeBoolean jeMaterialSpec_RemoveLayer(jeMaterialSpec* MatSpec, int32 layerIndex)
    @brief Remove the layer identified by its index from the current jeMaterialSpec
	@param MatSpec The jeMaterialSpec instance to modify
	@param layerIndex The layer index 0 based of the jeMaterialSpec to remove
	@return JE_TRUE if succeed, JE_FALSE otherwise
*/
JETAPI jeBoolean JETCC jeMaterialSpec_RemoveLayer(jeMaterialSpec* MatSpec, int32 layerIndex);

/*! @fn jeBoolean jeMaterialSpec_SetLayerTransform(jeMaterialSpec* MatSpec, int32 layerIndex, jeXForm3d* LayerXFrom)
    @brief Change the transform matrix of the layer identified by its index from the current jeMaterialSpec
	@param MatSpec The jeMaterialSpec instance to modify
	@param layerIndex The layer index 0 based of the jeMaterialSpec to modify
    @param LayerXFrom The transform matrix to set
	@return JE_TRUE if succeed, JE_FALSE otherwise
*/
JETAPI jeBoolean JETCC jeMaterialSpec_SetLayerTransform(jeMaterialSpec* MatSpec, int32 layerIndex, jeXForm3d* LayerXFrom);

JETAPI jeBoolean JETCC jeMaterialSpec_SetShader(jeMaterialSpec* MatSpec, jeShader* Shader);

JETAPI jeBoolean JETCC jeMaterialSpec_SetColor(jeMaterialSpec* MatSpec, int32 ColorIndex, jeRGBA* Color);

JETAPI jeBoolean JETCC jeMaterialSpec_SetThumbnail(jeMaterialSpec* MatSpec, jeMaterialSpec_Thumbnail* pThumb);

JETAPI uint32 JETCC jeMaterialSpec_GetLayerCount(const jeMaterialSpec* MatSpec);

JETAPI jeTexture* JETCC jeMaterialSpec_GetLayerTexture(const jeMaterialSpec* MatSpec, int32 layerIndex);

JETAPI jeBitmap* JETCC jeMaterialSpec_GetLayerBitmap(const jeMaterialSpec* MatSpec, int32 layerIndex);

JETAPI jeXForm3d* JETCC jeMaterialSpec_GetLayerTransform(const jeMaterialSpec* MatSpec, int32 layerIndex);

JETAPI jeShader* JETCC jeMaterialSpec_GetShader(const jeMaterialSpec* MatSpec);

JETAPI jeRGBA* JETCC jeMaterialSpec_GetColor(const jeMaterialSpec* MatSpec, int32 colorIndex);

JETAPI uint32 JETCC jeMaterialSpec_GetColors(const jeMaterialSpec* MatSpec, jeRGBA* Diffuse, jeRGBA* Specular, jeRGBA* Ambient, jeRGBA* Emissive);

JETAPI jeMaterialSpec_Thumbnail* JETCC jeMaterialSpec_GetThumbnail(const jeMaterialSpec* MatSpec);

JETAPI uint32 JETCC jeMaterialSpec_Height(const jeMaterialSpec* MatSpec);

JETAPI uint32 JETCC jeMaterialSpec_Width(const jeMaterialSpec* MatSpec);

/*!@}*/

/*! @name jeMaterial related functions and data
	@{
*/
/*! @typedef jeMaterial
*   @brief A reference to a Material used by the Engine
*/
typedef struct							jeMaterial			jeMaterial;

/*! @fn void jeMaterial_Destroy(jeMaterial **ppMaterial)
*   @brief Destroy the current jeMaterial
*   @param[in] Material The jeMaterial struct to destroy
	@note The *ppMaterial is set to NULL before returning
*/
JETAPI void					JETCC jeMaterial_Destroy(jeMaterial **ppMaterial);

/*! @fn jeBoolean jeMaterial_CreateRef(jeMaterial *Material)
*   @brief Reference the current Material
*   @param[in] Material The Material to increment its reference counter
*   @return JE_TRUE if success, JE_FALSE otherwise
*/
JETAPI jeBoolean			JETCC jeMaterial_CreateRef(jeMaterial *Material);

/*! @fn jeMaterial* jeMaterial_Create(const char *MatName)
*   @brief Create a new Material.
*   @param[in] MatName The name of the new Material.
*   @return The jeMaterial created. NULL if failed.
*/
JETAPI jeMaterial			* JETCC jeMaterial_Create(const char *MatName);
JETAPI jeBoolean			JETCC jeMaterial_SetBitmap(jeMaterial *Mat, jeBitmap *Bitmap, const char *BitmapName);
JETAPI const jeBitmap	* JETCC jeMaterial_GetBitmap(const jeMaterial *Mat);

/*! @fn const char* jeMaterial_GetName( const jeMaterial *Mat)
*   @brief Read the name of the current Material.
*   @param[in] Mat The jeMaterial to read its name.
*   @return The name if succeed, NULL otherwise.
*/
JETAPI const char			* JETCC jeMaterial_GetName( const jeMaterial *Mat);

/*! @fn const char* jeMaterial_GetBitmapName( const jeMaterial *Mat)
*   @brief Read the name of the Bitmap linked with the current Material
*   @param[in] Mat The jeMaterial to read its bitmap name
*   @return The bitmap name if succeed, NULL otherwise
*	@author Bruno Pettorelli (krouer@jet3d.com)
*/
JETAPI const char			* JETCC jeMaterial_GetBitmapName( const jeMaterial *Mat);

/*! @fn const jeMaterialSpec* jeMaterial_GetMaterialSpec( const jeMaterial *Mat)
*   @brief Grant access to the jeMaterialSpec of the current Material
*   @param[in] Mat The jeMaterial to access its jeMaterialSpec
*   @return The jeMaterialSpec member if succeed, NULL otherwise
*	@author Bruno Pettorelli (krouer@jet3d.com)
*/
JETAPI const jeMaterialSpec	* JETCC jeMaterial_GetMaterialSpec(const jeMaterial *Mat);
/*!@}*/

/*! @name jeMaterial_Array related functions and data
	@{
*/
/*! @typedef jeMaterial_Array
*   @brief An Array of jeMaterial struct
*   @see jeArray
*/
typedef struct							jeMaterial_Array	jeMaterial_Array;	

/*! @typedef jeMaterial_Array
*   @brief The index type
*   @see jeArray_Index
*/
typedef jeArray_Index					jeMaterial_ArrayIndex;

/*! @fn jeMaterial_Array* jeMaterial_ArrayCreate(int32 StartMaterials)
*   @brief Create a material array
*   @param[in] StartMaterials The count of jeMaterial the array will hold
*   @return A jeMaterial_Array instance if succeed otherwise NULL
*/
JETAPI jeMaterial_Array		* JETCC jeMaterial_ArrayCreate(int32 StartMaterials);

/*! @fn jeMaterial_Array* jeMaterial_ArrayCreateFromFile(jeVFile *VFile, jePtrMgr *PtrMgr)
*   @brief Create a material array from a file
*   @param[in] VFile The file where to read array data
*	@param[in] PtrMgr The pointer manager to use
*   @return A jeMaterial_Array instance if succeed otherwise NULL
*/
JETAPI jeMaterial_Array		* JETCC jeMaterial_ArrayCreateFromFile(jeVFile *VFile, jePtrMgr *PtrMgr);

/*! @fn jeBoolean jeMaterial_ArrayWriteToFile(jeMaterial_Array *MatArray, jeVFile *VFile, jePtrMgr *PtrMgr)
*   @brief Write a material array to a file
*	@param[in] MatArray The jeMaterial_Array to write
*   @param[in] VFile The file where to read array data
*	@param[in] PtrMgr The pointer manager to use
*   @return JE_TRUE if succeed, JE_FALSE otherwise
*/
JETAPI jeBoolean			JETCC jeMaterial_ArrayWriteToFile(jeMaterial_Array *MatArray, jeVFile *VFile, jePtrMgr *PtrMgr);

/*! @fn jeBoolean jeMaterial_ArrayCreateRef(jeMaterial_Array *MatArray)
*   @brief Increment the jeMaterial_Array reference counter
*	@param[in] MatArray The jeMaterial_Array to modify
*   @return JE_TRUE if succeed, JE_FALSE otherwise
*/
JETAPI jeBoolean			JETCC jeMaterial_ArrayCreateRef(jeMaterial_Array *MatArray);

/*! @fn void jeMaterial_ArrayDestroy(jeMaterial_Array **ppArray)
*   @brief Decrement the reference counter, destroy the instance if reach 0 and reset the address to NULL in all cases
*	@param[in,out] ppArray The address of jeMaterial_Array to destroy when its reference counter reach 0
*   @return JE_TRUE if succeed, JE_FALSE otherwise
	@note The *ppArray is set to NULL before returning
*/
JETAPI void					JETCC jeMaterial_ArrayDestroy(jeMaterial_Array **ppArray);
JETAPI jeMaterial_ArrayIndex JETCC jeMaterial_ArrayCreateMaterial(jeMaterial_Array *MatArray, const char *MatName);
JETAPI void					JETCC jeMaterial_ArrayDestroyMaterial(jeMaterial_Array *MatArray, jeMaterial_ArrayIndex *Index);
JETAPI const jeMaterial * JETCC jeMaterial_ArrayGetMaterialByIndex(const jeMaterial_Array *Array, jeMaterial_ArrayIndex Index);
JETAPI jeMaterial_ArrayIndex JETCC jeMaterial_ArrayGetMaterialIndex(const jeMaterial_Array *Array, const jeMaterial *Material);
JETAPI jeBoolean			JETCC jeMaterial_ArraySetMaterialBitmap(jeMaterial_Array *Array, jeMaterial_ArrayIndex Index, jeBitmap *Bitmap, const char *BitmapName);
JETAPI jeMaterial			* JETCC jeMaterial_ArrayGetNextMaterial(jeMaterial_Array *Array, const jeMaterial *Start);
JETAPI jeBoolean			JETCC jeMaterial_ArraySetMaterialSpec(jeMaterial_Array *Array, jeMaterial_ArrayIndex Index, jeMaterialSpec *MatSpec, const char *MatName);
/*!@}*/


#ifdef __cplusplus
}
#endif

#endif
