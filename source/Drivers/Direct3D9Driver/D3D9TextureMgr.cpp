/****************************************************************************************/
/*  D3D9TextureMgr.cpp                                                                  */
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
#include <stdio.h>
#include <vector>
#include "D3D9TextureMgr.h"
#include "Direct3D9Driver.h"
#include "D3D9Log.h"

extern D3D9Driver							g_D3D9Drv;

#define MAX_THANDLES						20000
jeTexture									TextureList[MAX_THANDLES];

static jeTexture *GetNextTHandle()
{
	if (LOG_LEVEL > 1)
		D3D9Log::GetPtr()->Printf("Function Call:  GetNextTHandle()");
	else
		REPORT("Function Call:  GetNextTHandle()");

	for (int i = 0; i < MAX_THANDLES; i++)
	{
		if (TextureList[i].Active == JE_FALSE)
		{
			TextureList[i].Active = JE_TRUE;
			TextureList[i].DriverOwned = JE_FALSE;
			TextureList[i].Data = NULL;
			TextureList[i].Lightmap = JE_FALSE;
			TextureList[i].Locked = JE_FALSE;
			TextureList[i].pTexture = NULL;
			return &TextureList[i];
		}
	}

	return NULL;
}

jeBoolean D3D9_THandle_Startup()
{
	if (LOG_LEVEL > 1)
		D3D9Log::GetPtr()->Printf("Function Call:  THandle_Startup()");
	else
		REPORT("Function Call:  THandle_Startup()");

	for (int i = 0; i < MAX_THANDLES; i++)
	{
		TextureList[i].id = i;
		TextureList[i].Active = JE_FALSE;
		TextureList[i].pTexture = NULL;
		TextureList[i].Lightmap = JE_FALSE;
		TextureList[i].Locked = JE_FALSE;
		TextureList[i].DriverOwned = JE_FALSE;
		TextureList[i].Data = NULL;
	}

	return JE_TRUE;
}

jeBoolean D3D9_THandle_Shutdown()
{
	if (LOG_LEVEL > 1)
		D3D9Log::GetPtr()->Printf("Function Call:  THandle_Shutdown()");
	else
		REPORT("Function Call:  THandle_Shutdown()");

	for (int j = 0; j < MAX_THANDLES; j++)
	{
		if (TextureList[j].Active == JE_TRUE)
			D3D9_THandle_Destroy(&TextureList[j]);
	}

	return JE_TRUE;
}

jeTexture* DRIVERCC D3D9_THandle_Create(int32 Width, int32 Height, int32 NumMipLevels, const jeRDriver_PixelFormat *PixelFormat)
{
	jeTexture					*Handle = NULL;
	HRESULT						hres;
	int32						Size; //, i;

	if (LOG_LEVEL > 1)
		D3D9Log::GetPtr()->Printf("Function Call:  THandle_Create()");
	else
		REPORT("Function Call:  THandle_Create()");

	Handle = GetNextTHandle();
	if (!Handle)
	{
		D3D9Log::GetPtr()->Printf("ERROR:  No more empty THandle slots!!");
		return NULL;
	}

	Handle->Active = JE_TRUE;
	Handle->Width = Width;
	Handle->Height = Height;
	Handle->Locked = JE_FALSE;

	Handle->Log = (uint8)GetLog(Width, Height);
	Handle->stride = Width ;

	Size = 1 << Handle->Log;

	if (PixelFormat->PixelFormat == JE_PIXELFORMAT_8BIT)
	{
		Handle->Format = D3DFMT_R3G3B2;
	}
	else if (PixelFormat->PixelFormat == JE_PIXELFORMAT_16BIT_555_RGB)
		{
		Handle->Format = D3DFMT_X1R5G5B5;
		}
	else if (PixelFormat->PixelFormat == JE_PIXELFORMAT_16BIT_565_RGB)
		{
		Handle->Format = D3DFMT_R5G6B5;
		}
	else if (PixelFormat->PixelFormat == JE_PIXELFORMAT_16BIT_1555_ARGB)
		{
		Handle->Format = D3DFMT_A1R5G5B5;
		}
	else if (PixelFormat->PixelFormat == JE_PIXELFORMAT_16BIT_4444_ARGB)
		{
		Handle->Format = D3DFMT_A4R4G4B4;
		}
	else if (PixelFormat->PixelFormat == JE_PIXELFORMAT_24BIT_RGB)
		{
		Handle->Format = D3DFMT_R8G8B8;
		}
	else if (PixelFormat->PixelFormat == JE_PIXELFORMAT_32BIT_XRGB)
		{
		Handle->Format = D3DFMT_X8R8G8B8;
		}
	else if (PixelFormat->PixelFormat == JE_PIXELFORMAT_32BIT_ARGB)
		{
		Handle->Format = D3DFMT_A8R8G8B8;
		}
	else
		{
		Handle->Format = D3DFMT_UNKNOWN;
		}

	hres = pDevice->CreateTexture(Width, Height, NumMipLevels, 0, Handle->Format, D3DPOOL_MANAGED, &Handle->pTexture, NULL);
	if (FAILED(hres))
	{
		D3D9Log::GetPtr()->Printf("ERROR:  Could not create texture!!");
		D3D9_THandle_Destroy(Handle);
		return NULL;
	}

	Handle->Log = (uint8)GetLog(Width, Height);
	Handle->Lightmap=JE_FALSE;

	return Handle;
}

jeTexture *DRIVERCC D3D9_THandle_CreateFromFile(jeVFile *File)
{
	uint8					*data = NULL;
	jeTexture				*Handle = NULL;
	int32					size;
	D3DXIMAGE_INFO			info;
	HRESULT					hres;
//	int						idx;

	if (LOG_LEVEL > 1)
		D3D9Log::GetPtr()->Printf("Function Call: THandle_CreateFromFile()");
	else
		REPORT("Function Call:  THandle_CreateFromFile()");

	Handle = GetNextTHandle();
	if (!Handle)
	{
		D3D9Log::GetPtr()->Printf("ERROR:  No more empty THandle slots!!");
		return NULL;
	}

	// Get the size of the data
	jeVFile_Size(File, &size);

	// Allocate the data
	Handle->Data = new uint8[size];
	if (!Handle->Data)
	{
		D3D9Log::GetPtr()->Printf("ERROR:  Out of memory!!");
		return NULL;
	}

	// Read in all the data
	jeVFile_Read(File, Handle->Data, size);

	// Setup the texture
	hres = D3DXCreateTextureFromFileInMemoryEx(pDevice, (void*)Handle->Data, size, D3DX_DEFAULT, D3DX_DEFAULT, D3DX_DEFAULT, 0, D3DFMT_UNKNOWN, D3DPOOL_MANAGED, D3DX_DEFAULT, D3DX_DEFAULT, 0, &info, NULL, &Handle->pTexture);
	if (FAILED(hres))
	{
		D3D9Log::GetPtr()->Printf("ERROR:  Could not load texture!!");

		delete [] Handle->Data;
		Handle->Data = NULL;

		return NULL;
	}

	//Delete the file data
	//delete [] data;
	//data = NULL;

	//Fill in the info
	Handle->Active = JE_TRUE;
	Handle->Height = info.Height;
	Handle->Width = info.Width;
	Handle->Format = info.Format;
	Handle->Locked = JE_FALSE;
	Handle->DriverOwned = JE_TRUE;

	//D3DXSaveTextureToFile("jetlogo.jpg", D3DXIFF_JPG, Handle->pTexture, NULL);
	return Handle;
}

jeBoolean DRIVERCC D3D9_THandle_Destroy(jeTexture *Handle)
{
	int32					id;

	if (LOG_LEVEL > 1)
		D3D9Log::GetPtr()->Printf("Function Call:  THandle_Destroy()");
	else
		REPORT("Function Call:  THandle_Destroy()");

	SAFE_RELEASE(Handle->pTexture);

	id = Handle->id;

	memset(Handle, 0, sizeof(jeTexture));

	Handle->id = id;
	Handle->Active = JE_FALSE;
	Handle->Locked = JE_FALSE;
	Handle->Lightmap = JE_FALSE;
	
	if (Handle->DriverOwned)
	{
		SAFE_DELETE_ARRAY(Handle->Data);
		Handle->DriverOwned = JE_FALSE;
	}

	return JE_TRUE;
}

jeBoolean DRIVERCC D3D9_THandle_Lock(jeTexture *Handle, int32 MipLevel, void **Bits)
{
	HRESULT						hres;

	if (LOG_LEVEL > 1)
		D3D9Log::GetPtr()->Printf("Function Call:  THandle_Lock()");
	else
		REPORT("Function Call:  THandle_Lock()");

	if (Handle->DriverOwned)
	{
		D3D9Log::GetPtr()->Printf("ERROR:  Cannot lock driver owned texture!!");
		return JE_FALSE;
	}

	hres = Handle->pTexture->LockRect(MipLevel, &Handle->lock, NULL, D3DLOCK_DISCARD);
	if (FAILED(hres))
	{
		D3D9Log::GetPtr()->Printf("ERROR:  Could not lock texture!!");
		return JE_FALSE;
	}

	*Bits = Handle->lock.pBits;
	Handle->stride=(Handle->lock.Pitch/2);

	REPORT("DEBUG:  Lock successful!!");
	return JE_TRUE;
}

jeBoolean DRIVERCC D3D9_THandle_Unlock(jeTexture *Handle, int32 MipLevel)
{
	HRESULT						hres;

	if (LOG_LEVEL > 1)
		D3D9Log::GetPtr()->Printf("Function Call:  THandle_Unlock()");
	else
		REPORT("Function Call:  THandle_Unlock()");

	if (Handle->DriverOwned)
	{
		D3D9Log::GetPtr()->Printf("ERROR:  Cannot unlock driver owned texture!!");
		return JE_FALSE;
	}

	hres = Handle->pTexture->UnlockRect(MipLevel);
	if (FAILED(hres))
	{
		D3D9Log::GetPtr()->Printf("ERROR:  Could not unlock texture!!");
		return JE_FALSE;
	}

	return JE_TRUE;
}

jeBoolean DRIVERCC D3D9_THandle_GetInfo(jeTexture *Handle, int32 MipLevel, jeTexture_Info *info)
{
	HRESULT						hres;
	D3DSURFACE_DESC				desc;

	if (LOG_LEVEL > 1)
		D3D9Log::GetPtr()->Printf("Function Call:  THandle_GetInfo()");
	else
		REPORT("Function Call:  THandle_GetInfo()");

	hres = Handle->pTexture->GetLevelDesc(MipLevel, &desc);
	if (FAILED(hres))
	{
		D3D9Log::GetPtr()->Printf("ERROR:  Could not get level description!!");
		return JE_FALSE;
	}

	info->Width = desc.Width;
	info->Height = desc.Height;
	info->Stride = 1 << Handle->Log;
	
	info->ColorKey = 0;
	info->Flags = 0;

	if (Handle->Format == D3DFMT_R3G3B2)
		info->PixelFormat.PixelFormat = JE_PIXELFORMAT_8BIT;
	else if (Handle->Format == D3DFMT_X1R5G5B5)
		info->PixelFormat.PixelFormat = JE_PIXELFORMAT_16BIT_555_RGB;
	else if (Handle->Format == D3DFMT_R5G6B5)
		info->PixelFormat.PixelFormat = JE_PIXELFORMAT_16BIT_565_RGB;
	else if (Handle->Format == D3DFMT_A1R5G5B5)
		info->PixelFormat.PixelFormat = JE_PIXELFORMAT_16BIT_1555_ARGB;
	else if (Handle->Format == D3DFMT_A4R4G4B4)
		info->PixelFormat.PixelFormat = JE_PIXELFORMAT_16BIT_4444_ARGB;
	else if (Handle->Format == D3DFMT_R8G8B8)
		info->PixelFormat.PixelFormat = JE_PIXELFORMAT_24BIT_RGB;
	else if (Handle->Format == D3DFMT_X8R8G8B8)
		info->PixelFormat.PixelFormat = JE_PIXELFORMAT_32BIT_XRGB;
	else if (Handle->Format == D3DFMT_A8R8G8B8)
		info->PixelFormat.PixelFormat = JE_PIXELFORMAT_32BIT_ARGB;
	else
		info->PixelFormat.PixelFormat = JE_PIXELFORMAT_NO_DATA;

    info->Direct = Handle->pTexture;
	return JE_TRUE;
}

IDirect3DTexture9 *D3D9_THandle_GetInterface(jeTexture *Handle)
{
	return Handle->pTexture;
}
