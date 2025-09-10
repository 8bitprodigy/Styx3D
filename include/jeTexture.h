/*!
	@file jeTexture.h 
	
	@author Anthony Rufrano (paradoxnj)
	@brief Hardware textures

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
#ifndef JE_TEXTURE_H
#define JE_TEXTURE_H

#include "BaseType.h"
#include "DCommon.h"
#include "VFile.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct jeEngine						jeEngine;

/*!
	@fn jeTexture *jeTexture_Create(jeEngine *Engine, int32 Width, int32 Height, int32 MipLevels, jeRDriver_PixelFormat *Format)
	@brief Creates a hardware texture
	@param[in] Engine The engine to create from
	@param[in] Width The width of the texture
	@param[in] Height The height of the texture
	@param[in] MipLevels The number of mip levels to generate
	@param[in] Format The pixel format to use
	@return The new texture
*/
JETAPI jeTexture * JETCC jeTexture_Create(jeEngine *Engine, int32 Width, int32 Height, int32 MipLevels, jeRDriver_PixelFormat *Format);

/*!
	@fn jeTexture *jeTexture_CreateFromFile(jeEngine *Engine, jeVFile *File)
	@brief Creates a hardware texture from file
	@param[in] Engine The engine to create from
	@param[in] File The file that contains the texture data
	@return The new texture
	@note This function can support the following file formats:  BMP, TGA, DDS, JPG, and PNG)
*/
JETAPI jeTexture * JETCC jeTexture_CreateFromFile(jeEngine *Engine, jeVFile *File);

/*!
	@fn jeBoolean jeTexture_Destroy(jeEngine *Engine, jeTexture *Texture)
	@brief Destroys a hardware texture
	@param[in] Engine The engine that holds the driver
	@param[in] Texture The texture to destroy
	@return JE_TRUE on success, JE_FALSE on failure
	@note Failure will result in a memory leak!!!
*/
JETAPI jeBoolean JETCC jeTexture_Destroy(jeEngine *Engine, jeTexture *Texture);

/*!
	@fn jeBoolean jeTexture_Lock(jeEngine *Engine, jeTexture *Texture, int32 MipLevel, void **data)
	@brief Locks a texture for writing
	@param[in] Engine The engine that holds the driver
	@param[in] Texture The texture to lock
	@param[in] MipLevel The mip level to lock
	@param[out] data The bitmap data
	@return JE_TRUE on success, JE_FALSE on failure
*/
JETAPI jeBoolean JETCC jeTexture_Lock(jeEngine *Engine, jeTexture *Texture, int32 MipLevel, void **data);

/*!
	@fn jeBoolean jeTexture_Unlock(jeEngine *Engine, jeTexture *Texture, int32 MipLevel)
	@brief Unlocks a texture
	@param[in] Engine The engine that holds the driver
	@param[in] Texture The texture to unlock
	@param[in] MipLevel The mip level to unlock
	@return JE_TRUE on success, JE_FALSE on failure
*/
JETAPI jeBoolean JETCC jeTexture_Unlock(jeEngine *Engine, jeTexture *Texture, int32 MipLevel);

/*!
	@fn jeBoolean jeTexture_GetInfo(jeEngine *Engine, jeTexture *Texture, int32 MipLevel, jeTexture_Info *TextureInfo)
	@brief Gets a texture's information
	@param[in] Engine The engine that holds the driver
	@param[in] Texture The texture to query
	@param[in] MipLevel The mip level to query
	@param[out] TextureInfo The texture's information
	@return JE_TRUE on success, JE_FALSE on failure
*/
JETAPI jeBoolean JETCC jeTexture_GetInfo(jeEngine *Engine, jeTexture *Texture, int32 MipLevel, jeTexture_Info *TextureInfo);

// End of header
#ifdef __cplusplus
}
#endif

#endif
