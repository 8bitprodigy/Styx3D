#ifndef OGL_TEXTURES_H
#define OGL_TEXTURES_H

#include <windows.h>
#include <gl/gl.h>
#include <gl/glu.h>

#include "DCommon.h"

#define MAX_TEXTURE_HANDLES						20000
#define THANDLE_MAX_MIP_LEVELS					16

#define THANDLE_UPDATE							0x00000001
#define THANDLE_TRANSPARENCY					0x00000002
#define THANDLE_LOCKED							0x00000004
#define THANDLE_UPDATE_LIGHTMAP					0x00000008

typedef struct jeTexture
{
	jeBoolean							Active;

	GLint								Width, Height, MipLevels;
	GLint								PaddedWidth, PaddedHeight;

	jeRDriver_PixelFormat				Format;

	GLuint								Flags;
	GLuint								TextureID;

	GLubyte								*Data[THANDLE_MAX_MIP_LEVELS];
	GLfloat								InvScale;

	jeTexture							*PalHandle;
} jeTexture;

extern jeTexture						TextureHandles[MAX_TEXTURE_HANDLES];

// Private Functions
jeBoolean								THandle_Startup();
jeBoolean								THandle_Shutdown();

void									FreeAllTextureHandles();
jeBoolean								THandle_Update(jeTexture *THandle);
void									THandle_DownloadLightmap(jeTexture *THandle, jeRDriver_LMapCBInfo *LInfo);

// Public Functions
jeBoolean DRIVERCC						OGLDrv_Reset();

jeTexture *DRIVERCC						OGLDrv_THandle_Create(int32 Width, int32 Height, int32 NumMipLevels, const jeRDriver_PixelFormat *PixelFormat);
jeTexture *DRIVERCC						OGLDrv_THandle_CreateFromFile(jeVFile *File);

jeBoolean DRIVERCC						OGLDrv_THandle_Destroy(jeTexture *THandle);

jeBoolean DRIVERCC						OGLDrv_THandle_Lock(jeTexture *THandle, int32 MipLevel, void **Data);
jeBoolean DRIVERCC						OGLDrv_THandle_Unlock(jeTexture *THandle, int32 MipLevel);

jeBoolean DRIVERCC						OGLDrv_THandle_GetInfo(jeTexture *THandle, int32 MipLevel, jeTexture_Info *Info);

extern GLint SnapToPower2(GLint Width);

// End of header
#endif
