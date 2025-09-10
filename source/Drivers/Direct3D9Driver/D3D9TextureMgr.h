/****************************************************************************************/
/*  D3D9TextureMgr.h                                                                    */
/*                                                                                      */
/*  Author: Anthony Rufrano                                                             */
/*  Description: D3D9 Texture Manager                                                   */
/*                                                                                      */
/*  The contents of this file are subject to the Jet3D Public License                   */
/*  Version 1.01 (the "License"); you may not use this file except in                   */
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
/*	Modified 5/12/2005 by paradoxnj														*/
/*	jeTexture implementation															*/
/****************************************************************************************/
#ifndef D3D9_TEXTURE_MANAGER_H
#define D3D9_TEXTURE_MANAGER_H

#include <windows.h>
#include <d3d9.h>
#include <d3dx9.h>
#include "DCommon.h"

jeBoolean								D3D9_THandle_Startup();
jeBoolean								D3D9_THandle_Shutdown();

jeTexture*								DRIVERCC D3D9_THandle_Create(int32 Width, int32 Height, int32 NumMipLevels, const jeRDriver_PixelFormat *PixelFormat);
jeTexture*								DRIVERCC D3D9_THandle_CreateFromFile(jeVFile *File);
jeBoolean								DRIVERCC D3D9_THandle_Destroy(jeTexture *Handle);

jeBoolean								DRIVERCC D3D9_THandle_Lock(jeTexture *Handle, int32 MipLevel, void **Bits);
jeBoolean								DRIVERCC D3D9_THandle_Unlock(jeTexture *Handle, int32 MipLevel);

jeBoolean								DRIVERCC D3D9_THandle_GetInfo(jeTexture *Handle, int32 MipLevel, jeTexture_Info *Info);
IDirect3DTexture9						*D3D9_THandle_GetInterface(jeTexture *Handle);
int32									D3D9_THandle_GetID(jeTexture *Handle);

typedef struct jeTexture
{
	int32									id;

	jeBoolean								Active;
	IDirect3DTexture9						*pTexture;

	int32									Width;
	int32									Height;

	int32									stride;
	uint8									Log;

	D3DFORMAT								Format;
	D3DLOCKED_RECT							lock;

	jeBoolean								Locked;
	jeBoolean								Lightmap;

	uint8									*Data;
	jeBoolean								DriverOwned;
} jeTexture;

#endif
