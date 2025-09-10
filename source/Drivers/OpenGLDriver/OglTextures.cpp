#include <stdio.h>
#include <math.h>
#include "OglTextures.h"
#include "OglDrv.h"

jeTexture							TextureHandles[MAX_TEXTURE_HANDLES];
extern FILE							*ogllog;

static void CkBlit24_32(GLubyte *dstPtr, GLint dstWidth, GLint dstHeight, GLubyte *srcPtr, GLint srcWidth, GLint srcHeight)
{
	GLint width, height;
	GLubyte *nextLine;

	memset(dstPtr, 0x00, dstWidth * dstHeight * 4);

	for(height = 0; height < srcHeight; height++)
	{
		nextLine = (dstPtr + (dstWidth * 4));

		for(width = 0; width < srcWidth; width++)
		{
			if(!(*srcPtr == 0x00 && *(srcPtr + 1) == 0x00 && *(srcPtr + 2) == 0x01))
			{
				*dstPtr = *srcPtr;
				*(dstPtr + 1) = *(srcPtr + 1);
				*(dstPtr + 2) = *(srcPtr + 2);
				*(dstPtr + 3) = 0xFF;
			}

			srcPtr += 3;
			dstPtr += 4;
		}

		dstPtr = nextLine;
	}
}

GLint SnapToPower2(GLint Width)
{
	if(Width > 1 && Width <= 2) 
	{
		Width = 2;
	}
	else if(Width > 2 && Width <= 4)
	{
		Width = 4;
	}
	else if(Width > 4 && Width <= 8) 
	{
		Width = 8;
	}
	else if(Width > 8 && Width <= 16)
	{
		Width =16;
	}
	else if(Width > 16 && Width <= 32)
	{
		Width = 32;
	}
	else if(Width > 32 && Width <= 64) 
	{
		Width = 64;
	}
	else if(Width > 64 && Width <= 128) 
	{
		Width = 128;
	}
	else if(Width > 128 && Width <= 256) 
	{
		Width = 256;
	}
	else if(Width > 256 && Width <= 512) 
	{
		Width = 512;
	}
	else if(Width > 512 && Width <= 1024) 
	{
		Width = 1024;
	}
	else if(Width > 1024 && Width <= 2048) 
	{
		Width = 2048;
	}

	return Width;
}

static uint32 Log2(uint32 P2)
{
	uint32		p = 0;
	int32		i = 0;

	for (i = P2; i > 0; i>>=1)
		p++;

	return (p-1);
}

static int32 GetLog(int32 Width, int32 Height)
{
	int32	LWidth = SnapToPower2(max(Width, Height));
	return Log2(LWidth);
} 


jeBoolean THandle_Startup()
{
	for (int i = 0; i < MAX_TEXTURE_HANDLES; i++)
	{
		memset(&TextureHandles[i], 0, sizeof(jeTexture));
		TextureHandles[i].Active = JE_FALSE;
	}

	return JE_TRUE;
}

jeBoolean THandle_Shutdown()
{
	FreeAllTextureHandles();
	return JE_TRUE;
}

void FreeAllTextureHandles()
{
	for (int i = 0; i < MAX_TEXTURE_HANDLES; i++)
	{
		if (TextureHandles[i].Active)
		{
			OGLDrv_THandle_Destroy(&TextureHandles[i]);
		}
	}
}

static jeTexture *FindTextureHandle()
{
	for (int i = 0; i < MAX_TEXTURE_HANDLES; i++)
	{
		if (TextureHandles[i].Active == JE_FALSE)
		{
			TextureHandles[i].Active = JE_TRUE;
			return &TextureHandles[i];
		}
	}

	return NULL;
}

jeBoolean DRIVERCC OGLDrv_THandle_Destroy(jeTexture *THandle)
{
	fprintf(ogllog, "THandle_Destroy\n");

	if (THandle->Active == JE_FALSE)
		return JE_TRUE;

	if (THandle->PalHandle != NULL)
		OGLDrv_THandle_Destroy(THandle->PalHandle);

	glDeleteTextures(1, &THandle->TextureID);

	for (int i = 0; i < THANDLE_MAX_MIP_LEVELS; i++)
	{
		if (THandle->Data[i] != NULL)
		{
			delete [] THandle->Data[i];
			THandle->Data[i] = NULL;
		}
	}

	THandle->Active = JE_FALSE;
	return JE_TRUE;
}

jeTexture *DRIVERCC OGLDrv_THandle_Create(int32 Width, int32 Height, int32 NumMipLevels, const jeRDriver_PixelFormat *Format)
{
	int32						SWidth, SHeight;
	jeTexture					*Texture = NULL;
	GLubyte						Log;

	fprintf(ogllog, "THandle_Create\n");
	Texture = FindTextureHandle();
	if (!Texture)
	{
		fprintf(ogllog, "No available THandles!!\n");
		return NULL;
	}

	if (Format->Flags & RDRIVER_PF_3D)
	{
		SWidth = SnapToPower2(Width);
		SHeight = SnapToPower2(Height);

		if (Width != SWidth)
		{
			Texture->Active = JE_FALSE;
			fprintf(ogllog, "Texture is not power of 2 (Width)\n");
			return NULL;
		}

		if (Height != SHeight)
		{
			Texture->Active = JE_FALSE;
			fprintf(ogllog, "Texture is not power of 2 (Height)\n");
			return NULL;
		}
	}

	Texture->MipLevels = NumMipLevels;
	Texture->Width = Width;
	Texture->Height = Height;
	Texture->Format = *Format;
	Texture->Flags = 0;

	Log = (GLubyte)GetLog(Width, Height);

	if (Texture->Format.Flags & RDRIVER_PF_2D)
	{
		Texture->PaddedWidth = SnapToPower2(Width);
		Texture->PaddedHeight = SnapToPower2(Height);
	}
	else if (Texture->Format.Flags & RDRIVER_PF_3D)
	{
		Texture->InvScale = 1.0f / ((GLfloat)(1 << Log));
	}
	else if (Texture->Format.Flags & RDRIVER_PF_PALETTE)
	{
	}
	else
	{
		Texture->Flags |= THANDLE_UPDATE_LIGHTMAP;
		Texture->InvScale = 1.0f / ((GLfloat)((1 << Log) << 4));
	}

	glGenTextures(1, &Texture->TextureID);
	return Texture;
}

jeBoolean DRIVERCC OGLDrv_THandle_GetInfo(jeTexture *THandle, int32 MipLevel, jeTexture_Info *Info)
{
	fprintf(ogllog, "THandle_GetInfo\n");

	if (!THandle->Active)
		return JE_FALSE;

	Info->Width = THandle->Width >> MipLevel;
	Info->Height = THandle->Height >> MipLevel;
	Info->Stride = Info->Width;
	Info->Flags = 0;
	Info->PixelFormat = THandle->Format;

	if (THandle->Format.Flags & RDRIVER_PF_CAN_DO_COLORKEY)
	{
		Info->Flags = RDRIVER_THANDLE_HAS_COLORKEY;
		Info->ColorKey = 1;
	}
	else
	{
		Info->ColorKey = 0;
	}
	
	return JE_TRUE;
}

jeBoolean DRIVERCC OGLDrv_THandle_Lock(jeTexture *THandle, int32 MipLevel, void **Data)
{
	fprintf(ogllog, "THandle_Lock\n");

	if (THandle->Data[MipLevel] != NULL)
	{
		THandle->Flags |= (THANDLE_LOCKED << MipLevel);
		*Data = THandle->Data[MipLevel];
		return JE_TRUE;
	}

	if (THandle->Format.PixelFormat == JE_PIXELFORMAT_32BIT_ABGR)
	{
		GLint					mipWidth, mipHeight;

		if (MipLevel == 0)
		{
			mipWidth = THandle->Width;
			mipHeight = THandle->Height;
		}
		else
		{
			mipWidth = (THandle->Width / (int)pow(2.0, (double)MipLevel));
			mipHeight = (THandle->Height / (int)pow(2.0, (double)MipLevel));
		}

		THandle->Data[MipLevel] = new GLubyte[mipWidth * mipHeight * 4];
	}
	else if (THandle->Format.PixelFormat == JE_PIXELFORMAT_24BIT_RGB)
	{
		THandle->Data[MipLevel] = new GLubyte[THandle->Width * THandle->Height * 3];
	}
	else if (THandle->Format.PixelFormat == JE_PIXELFORMAT_16BIT_1555_ARGB)
	{
		THandle->Data[MipLevel] = new GLubyte[THandle->Width * THandle->Height * 2];
	}
	else if (THandle->Format.PixelFormat == JE_PIXELFORMAT_8BIT)
	{
		THandle->Data[MipLevel] = new GLubyte[THandle->Width * THandle->Height];
	}
	else
	{
		*Data = NULL;
		return JE_FALSE;
	}

	THandle->Flags |= (THANDLE_LOCKED << MipLevel);
	*Data = THandle->Data[MipLevel];

	return JE_TRUE;
}

jeBoolean DRIVERCC OGLDrv_THandle_Unlock(jeTexture *THandle, int32 MipLevel)
{
	fprintf(ogllog, "THandle_Unlock\n");
	if (!(THandle->Flags & (THANDLE_LOCKED << MipLevel)))
	{
		fprintf(ogllog, "Not locked!!\n");
		return JE_FALSE;
	}

	if (THandle->Data[MipLevel] == NULL)
	{
		fprintf(ogllog, "Invalid mip level!!\n");
		return JE_FALSE;
	}

	THandle->Flags &= ~(THANDLE_LOCKED << MipLevel);

	if (MipLevel == 0)
		THandle->Flags |= THANDLE_UPDATE;

	return JE_TRUE;
}

jeBoolean THandle_Update(jeTexture *THandle)
{
	int							baseLog2;
	unsigned char				*ColorPal = NULL;

	if (THandle->Format.Flags & RDRIVER_PF_2D)
	{
		switch (THandle->Format.PixelFormat)
		{
		case JE_PIXELFORMAT_24BIT_RGB:
			{
				GLubyte			*dest = NULL;

				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

				dest = new GLubyte[THandle->PaddedWidth * THandle->PaddedHeight * 4];
				CkBlit24_32(dest, THandle->PaddedWidth, THandle->PaddedHeight, THandle->Data[0], THandle->Width, THandle->Height);

				glTexImage2D(GL_TEXTURE_2D, 0, 4, THandle->PaddedWidth, THandle->PaddedHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, dest);

				delete [] dest;
				dest = NULL;
				
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

				break;
			}
		case JE_PIXELFORMAT_32BIT_ABGR:
			{
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

				gluBuild2DMipmaps(GL_TEXTURE_2D, 4, THandle->Width, THandle->Height, GL_RGBA, GL_UNSIGNED_BYTE, THandle->Data[0]);

				break;
			}
		/*case JE_PIXELFORMAT_16BIT_1555_ARGB:
			{
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

				glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
				glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

				glTexImage2D(  GL_TEXTURE_2D,0,4,THandle->Width,THandle->Height, 0, GL_ABGR_EXT, GL_UNSIGNED_SHORT_5_5_5_1_EXT, THandle->Data[0]); 

				break;
			}
		case JE_PIXELFORMAT_8BIT:
			{
				char* dataOut;

				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

				baseLog2 = GetLog(THandle->Width,THandle->Height);
				
				glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

				glTexImage2D(GL_TEXTURE_2D, 0, GL_COLOR_INDEX8_EXT, THandle->Width, THandle->Height, 0, GL_COLOR_INDEX, GL_UNSIGNED_BYTE, &THandle->Data[0]);

				THandle_Lock(THandle->PalHandle,0, &ColorPal);
				glColorTable(GL_TEXTURE_2D, GL_RGBA, THandle->PalHandle->Width, GL_RGBA, GL_UNSIGNED_BYTE, ColorPal);
				THandle_UnLock(THandle->PalHandle,0);

				break;
			}*/
		default:
			{
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

				gluBuild2DMipmaps(GL_TEXTURE_2D, 3, THandle->Width, THandle->Height, GL_RGB, GL_UNSIGNED_BYTE, THandle->Data[0]);
				break;
			}
		}
	}

	THandle->Flags &= ~THANDLE_UPDATE;
	return JE_TRUE;
}

void THandle_DownloadLightmap(jeTexture *THandle, jeRDriver_LMapCBInfo *LInfo)
{
	GLubyte					*tempBits = NULL;

	OGLDrv_THandle_Lock(THandle, 0, (void**)tempBits);
	memcpy(tempBits, LInfo->RGBLight[0], THandle->Width * THandle->Height * 3);
	OGLDrv_THandle_Unlock(THandle, 0);
}

jeBoolean DRIVERCC OGLDrv_Reset()
{
	fprintf(ogllog, "Drv_Reset\n");
	FreeAllTextureHandles();
	return JE_TRUE;
}
