/*!
	@file jeTexture.cpp
	
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
#include "Engine.h"
#include "jeTexture.h"
#include "Engine._h"

JETAPI jeTexture * JETCC jeTexture_Create(jeEngine *Engine, int32 Width, int32 Height, int32 MipLevels, jeRDriver_PixelFormat *Format)
{
	return Engine->DriverInfo.RDriver->THandle_Create(Width, Height, MipLevels, Format);
}

JETAPI jeTexture * JETCC jeTexture_CreateFromFile(jeEngine *Engine, jeVFile *File)
{
	return Engine->DriverInfo.RDriver->THandle_CreateFromFile(File);
}

JETAPI jeBoolean JETCC jeTexture_Destroy(jeEngine *Engine, jeTexture *Texture)
{
	return Engine->DriverInfo.RDriver->THandle_Destroy(Texture);
}

JETAPI jeBoolean JETCC jeTexture_Lock(jeEngine *Engine, jeTexture *Texture, int32 MipLevel, void **data)
{
	return Engine->DriverInfo.RDriver->THandle_Lock(Texture, MipLevel, data);
}

JETAPI jeBoolean JETCC jeTexture_Unlock(jeEngine *Engine, jeTexture *Texture, int32 MipLevel)
{
	return Engine->DriverInfo.RDriver->THandle_UnLock(Texture, MipLevel);
}

JETAPI jeBoolean JETCC jeTexture_GetInfo(jeEngine *Engine, jeTexture *Texture, int32 MipLevel, jeTexture_Info *TextureInfo)
{
	return Engine->DriverInfo.RDriver->THandle_GetInfo(Texture, MipLevel, TextureInfo);
}
