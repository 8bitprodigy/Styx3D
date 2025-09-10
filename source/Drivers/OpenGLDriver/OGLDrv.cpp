#include <windows.h>
#include <gl/gl.h>
#include <gl/glu.h>
#include <stdio.h>
#include <math.h>

#include "OglDrv.h"
#include "OglTextures.h"

HWND							hWnd = NULL;
HDC								hDC = NULL;
HGLRC							hglRC = NULL;

FILE							*ogllog = NULL;

bool							Has_Stencil = false;
bool							Has_MultiTexture = false;
bool							fullscreen = false;

PFNGLACTIVETEXTUREARBPROC		glActiveTextureARB = NULL;
PFNGLMULTITEXCOORD4FARBPROC		glMultiTexCoord4fARB = NULL;

DRV_Window						ClientWindow;

DRV_RENDER_MODE					RenderMode = RENDER_NONE;
GLuint							Render_HardwareFlags = 0;

GLuint							boundTexture;
GLuint							boundTexture2;

GLuint							decalTexObj = 0;

GLfloat							CurrentGamma = 1.0f;

int32							LastError = 0;
char							LastErrorStr[255];		

DRV_EngineSettings				EngineSettings;

GLuint							AlphaFunc = GL_LEQUAL;
GLuint							StencilFunc = GL_LEQUAL;

GLfloat							AlphaRef = 0.0f;
GLfloat							StencilRef = 0.0f;

jeBoolean DRIVERCC OGLDrv_EnumSubDrivers(DRV_ENUM_DRV_CB *Cb, void *Context)
{
	fprintf(ogllog, "EnumSubDrivers\n");
	Cb(0, "OpenGL Driver", Context);
	return JE_TRUE;
}

jeBoolean DRIVERCC OGLDrv_EnumModes(S32 Driver, char *DriverName, DRV_ENUM_MODES_CB *Cb, void *Context)
{
	DEVMODE						mode;
	int							modecount = 0;

	fprintf(ogllog, "EnumModes\n");
	while (EnumDisplaySettings(NULL, modecount, &mode))
	{
		if (mode.dmPelsWidth <= 2048 && mode.dmPelsHeight <= 1024)
		{
			if (ChangeDisplaySettings(&mode, CDS_TEST) == DISP_CHANGE_SUCCESSFUL)
			{
				char			modename[32];

				sprintf(modename, "%dx%dx%d", mode.dmPelsWidth, mode.dmPelsHeight, mode.dmBitsPerPel);
				fprintf(ogllog, "%s\n", modename);
				Cb(modecount, modename, mode.dmPelsWidth, mode.dmPelsHeight, mode.dmBitsPerPel, Context);
			}
		}

		modecount++;
	}

	Cb(modecount, "WindowMode", -1, -1, -1, Context);
	return JE_TRUE;
}

jeRDriver_PixelFormat PixelFormats[] =
{
	{	JE_PIXELFORMAT_32BIT_ABGR,		RDRIVER_PF_3D | RDRIVER_PF_COMBINE_LIGHTMAP	},
	{	JE_PIXELFORMAT_24BIT_RGB,		RDRIVER_PF_2D | RDRIVER_PF_CAN_DO_COLORKEY	},
	{	JE_PIXELFORMAT_24BIT_RGB,		RDRIVER_PF_LIGHTMAP	}
};

#define NUM_PIXEL_FORMATS	( sizeof(PixelFormats) / sizeof(jeRDriver_PixelFormat) )

jeBoolean DRIVERCC OGLDrv_EnumPixelFormats(DRV_ENUM_PFORMAT_CB *Cb, void *Context)
{
	GLint i;

	fprintf(ogllog, "EnumPixelFormats\n");
	for(i = 0; i < NUM_PIXEL_FORMATS; i++)
	{
		if(!Cb(&PixelFormats[i], Context))
			return JE_TRUE;
	}

	return JE_TRUE;
}

jeBoolean DRIVERCC OGLDrv_Init(DRV_DriverHook *Hook)
{
	fprintf(ogllog, "Init\n");
	if (Hook->Width != -1 && Hook->Height != -1)
	{
		DEVMODE						mode;

		//Full screen
		mode.dmSize = sizeof(DEVMODE);
		mode.dmPelsWidth = Hook->Width;
		mode.dmPelsHeight = Hook->Height;
		mode.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT;

		if (ChangeDisplaySettings(&mode, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL)
		{
			fprintf(ogllog, "Could not set fullscreen mode!!\n");
			return JE_FALSE;
		}

		// Change the window's style, size and position.
		SetWindowLong(Hook->hWnd, GWL_STYLE, WS_POPUP | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);
		SetWindowPos(Hook->hWnd, HWND_TOP, 0, 0, Hook->Width, Hook->Height, SWP_FRAMECHANGED | SWP_SHOWWINDOW);

		fullscreen = true;
	}
	else
	{
		RECT					r;

		GetClientRect(Hook->hWnd, &r);

		Hook->Width = r.right - r.left;
		Hook->Height = r.bottom - r.top;
	}

	hDC = GetDC(Hook->hWnd);
	if (!hDC)
	{
		fprintf(ogllog, "Could not get device context!!\n");
		if (fullscreen)
			ChangeDisplaySettings(NULL, 0);

		return JE_FALSE;
	}

	PIXELFORMATDESCRIPTOR pfd = 
	{
		sizeof(PIXELFORMATDESCRIPTOR),
		1,
		PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER, 
		PFD_TYPE_RGBA, 
		16, 
		0,0,0,0,0,0, 
		0,0,0,0,0,0, 
		0,	// 16 bit Alpha
		16, // 16 bit Z-Buffer
		0, // 16 bit Stencil
		0, // No Auxiliary Buffer
		PFD_MAIN_PLANE,
		0,0,0,0
	};

	int pixelformat = ChoosePixelFormat(hDC, &pfd);
	if (!SetPixelFormat(hDC, pixelformat, &pfd))
	{
		fprintf(ogllog, "Could not set pixel format!!\n");
		return JE_FALSE;
	}

	DescribePixelFormat(hDC, pixelformat, sizeof(PIXELFORMATDESCRIPTOR), &pfd);

	hglRC = wglCreateContext(hDC);
	if (!hglRC)
	{
		fprintf(ogllog, "Could not get rendering context!!\n");
		if (fullscreen)
			ChangeDisplaySettings(NULL, 0);

		return JE_FALSE;
	}

	wglMakeCurrent(hDC, hglRC);

	const char *extensions = (const char*)glGetString(GL_EXTENSIONS);

	if (strstr(extensions, "GL_ARB_multitexture") != NULL)
	{
		glActiveTextureARB = (PFNGLACTIVETEXTUREARBPROC)wglGetProcAddress("glActiveTextureARB");
		glMultiTexCoord4fARB = (PFNGLMULTITEXCOORD4FARBPROC)wglGetProcAddress("glMultiTexCoord4fARB");

		if(glActiveTextureARB != NULL && glMultiTexCoord4fARB != NULL)
			Has_MultiTexture = JE_TRUE;
	}

	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	glEnable(GL_TEXTURE_2D);
	glEnable(GL_DEPTH_TEST);

	glEnable(GL_BLEND);    
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glShadeModel(GL_SMOOTH);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

	if (Has_MultiTexture)
	{
		glActiveTextureARB(GL_TEXTURE1_ARB);

		glDisable(GL_TEXTURE_1D);
		glDisable(GL_TEXTURE_2D);		

		glActiveTextureARB(GL_TEXTURE0_ARB);
	}

	glMatrixMode(GL_PROJECTION);
 	glLoadIdentity();
 	glOrtho(0.0f, (GLfloat)Hook->Width, 0.0f, (GLfloat)Hook->Height, 0.0f, 1.0f);
 	glMatrixMode(GL_MODELVIEW);
 	glLoadIdentity();
 	glViewport(0, 0, Hook->Width, Hook->Height);

	glScalef(1.0f, -1.0f, 1.0f);
	glTranslatef(0, 0 - (GLfloat)Hook->Height, 0.0);

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	return JE_TRUE;
}

jeBoolean DRIVERCC OGLDrv_Shutdown()
{
	fprintf(ogllog, "Shutdown\n");

	glFinish();

	if (fullscreen)
		ChangeDisplaySettings(NULL, 0);

	wglMakeCurrent(NULL, NULL);
	wglDeleteContext(hglRC);

	ReleaseDC(ClientWindow.hWnd, hDC);

	fclose(ogllog);

	return JE_TRUE;
}

jeBoolean DRIVERCC OGLDrv_UpdateWindow()
{
	return JE_TRUE;
}

jeBoolean DRIVERCC OGLDrv_SetActive(jeBoolean Active)
{
	return JE_TRUE;
}

jeBoolean DRIVERCC OGLDrv_BeginScene(jeBoolean Clear, jeBoolean ClearZ, RECT *WorldRect, jeBoolean WireFrame)
{
	if (Clear)
		glClear(GL_COLOR_BUFFER_BIT);
	
	if (ClearZ)
		glClear(GL_DEPTH_BUFFER_BIT);

	/*if (ClearStencil && Has_Stencil)
	{
		glClearStencil(1);
		glClear(GL_STENCIL_BUFFER_BIT);
	}*/

	return JE_TRUE;
}

jeBoolean DRIVERCC OGLDrv_EndScene()
{
	SwapBuffers(hDC);
	return JE_TRUE;
}

jeBoolean DRIVERCC OGLDrv_BeginBatch()
{
	return JE_TRUE;
}

jeBoolean DRIVERCC OGLDrv_EndBatch()
{
	return JE_TRUE;
}

jeBoolean DRIVERCC OGLDrv_RenderGouraudPoly(jeTLVertex *Pnts, int32 NumPoints, uint32 Flags)
{
	GLint i;
	GLfloat zRecip;
	jeTLVertex *pPnt = Pnts;
	GLubyte alpha;

	if(Flags & JE_RENDER_FLAG_ALPHA)
		alpha = (GLubyte)Pnts->a;
	else
		alpha = 255;
	
	glDisable(GL_TEXTURE_2D);
 	glBegin(GL_TRIANGLE_FAN);

	for(i = 0; i < NumPoints; i++)
	{
		zRecip = 1.0f / pPnt->z;   
		glColor4ub((GLubyte)pPnt->r, (GLubyte)pPnt->g, (GLubyte)pPnt->b, alpha);
		glVertex3f(pPnt->x, pPnt->y, -1.0f + zRecip);
		pPnt++;
	}

	glEnd(); 

	glEnable(GL_TEXTURE_2D); 
	return JE_TRUE;
}

jeBoolean DRIVERCC OGLDrv_RenderWorldPoly(jeTLVertex *Pnts, int32 NumPoints, jeRDriver_Layer *Layers, int32 NumLayers, void *LMapCBContext, uint32 Flags)
{
	GLfloat					zRecip;
	GLfloat					tu, tv, lu, lv;
	GLfloat					shiftU2, shiftV2;
	GLint					i;
	jeRDriver_LMapCBInfo	LMapCBInfo;
	GLfloat					shiftU, shiftV, scaleU, scaleV;
	jeTLVertex				*pPnt = Pnts;
	GLubyte					alpha;
	bool					Dynamic = false;

	if(Flags & JE_RENDER_FLAG_ALPHA) {
		alpha = (GLubyte)Pnts->a;
	} else {
		alpha = 255;
	}
	
	shiftU = Layers->ShiftU;//TexInfo->ShiftU;
	shiftV = Layers->ShiftV;//TexInfo->ShiftV;
	scaleU = 1.0f / Layers->ScaleU; //1.0f / TexInfo->DrawScaleU;
	scaleV = 1.0f / Layers->ScaleV; //1.0f / TexInfo->DrawScaleV;

	if(boundTexture != Layers[0].THandle->TextureID)
	{
		glBindTexture(GL_TEXTURE_2D, Layers[0].THandle->TextureID);
		boundTexture = Layers[0].THandle->TextureID;
	}

	if(Layers[0].THandle->Flags & THANDLE_UPDATE) {
		THandle_Update(Layers[0].THandle);
	}
	
	if( NumLayers > 1) // lightmap poly.
	{
		// Call the engine to setup the lightmap..
		OGLDRV.SetupLightmap(&LMapCBInfo, LMapCBContext);

		if(LMapCBInfo.Dynamic || Layers[1].THandle->Flags & THANDLE_UPDATE_LIGHTMAP)
		{
			THandle_DownloadLightmap(Layers[1].THandle, &LMapCBInfo);
			if(Dynamic) {
				Layers[1].THandle->Flags |= THANDLE_UPDATE_LIGHTMAP;
			} else {
				Layers[1].THandle->Flags &= ~THANDLE_UPDATE_LIGHTMAP;
			}
		}
	}

	if(NumLayers > 1)
	{
		glActiveTextureARB(GL_TEXTURE1_ARB);
		glEnable(GL_TEXTURE_2D);

		if(boundTexture2 != Layers[1].THandle->TextureID)
		{
			glBindTexture(GL_TEXTURE_2D,Layers[1].THandle->TextureID);
			boundTexture2 = Layers[1].THandle->TextureID;
		}

		if(Layers[1].THandle->Flags & THANDLE_UPDATE)
		{
			THandle_Update(Layers[1].THandle);
		}

		shiftU2 = (GLfloat)Layers[1].ShiftU;//MinU - 8.0f;
		shiftV2 = (GLfloat)Layers[1].ShiftV;//(GLfloat)LInfo->MinV - 8.0f;
	}

	pPnt = Pnts;

	glBegin(GL_TRIANGLE_FAN);	
	for(i = 0; i < NumPoints; i++)
	{	
		zRecip = 1.0f / pPnt->z;   
		tu = (pPnt->u * scaleU + shiftU);
		tv = (pPnt->v * scaleV + shiftV);

		glColor4ub((GLubyte)pPnt->r, (GLubyte)pPnt->g, (GLubyte)pPnt->b, alpha);
		glMultiTexCoord4fARB(GL_TEXTURE0_ARB, tu * Layers->THandle->InvScale * zRecip, tv * Layers->THandle->InvScale * zRecip, 0.0f, zRecip);

		if(NumLayers > 1)
		{
			lu = pPnt->u - shiftU2;
			lv = pPnt->v - shiftV2;

			glMultiTexCoord4fARB(GL_TEXTURE1_ARB, lu * Layers[1].THandle->InvScale * zRecip, lv * Layers[1].THandle->InvScale * zRecip, 0.0f, zRecip);
		}

		glVertex3f(pPnt->x, pPnt->y, -1.0f + zRecip);
		pPnt++;
	}

	glEnd(); 

	if( NumLayers > 1)
	{
		glDisable(GL_TEXTURE_2D);
		glActiveTextureARB(GL_TEXTURE0_ARB);
	}

	return JE_TRUE;
}


jeBoolean DRIVERCC OGLDrv_RenderMiscTexturePoly(jeTLVertex *Pnts, int32 NumPoints, jeRDriver_Layer *Layers, int32 NumLayers, uint32 Flags)
{
	GLint i;
	GLfloat zRecip;
	jeTLVertex *pPnt = Pnts;
	GLubyte alpha;

	if(Flags & JE_RENDER_FLAG_ALPHA)
		alpha = (GLubyte)Pnts->a;
	else
		alpha = 255;
	
	if(boundTexture != Layers[0].THandle->TextureID)
	{
		glBindTexture(GL_TEXTURE_2D, Layers[0].THandle->TextureID);
		boundTexture = Layers[0].THandle->TextureID;
	}

	if(Layers[0].THandle->Flags & THANDLE_UPDATE)
		THandle_Update(Layers[0].THandle);
	
	if (Flags & JE_RENDER_FLAG_NO_ZTEST)		// We are assuming that this is not going to change all that much
		glDisable(GL_DEPTH_TEST);

	if (Flags & JE_RENDER_FLAG_NO_ZWRITE)	// We are assuming that this is not going to change all that much
		glDepthMask(0);

	glBegin(GL_TRIANGLE_FAN);
	for(i = 0; i < NumPoints; i++)
	{
		zRecip = 1.0f / pPnt->z;                    

		glColor4ub((GLubyte)pPnt->r, (GLubyte)pPnt->g, (GLubyte)pPnt->b, alpha);
		glTexCoord4f(pPnt->u * zRecip, pPnt->v * zRecip, 0.0f, zRecip);
		glVertex3f(pPnt->x, pPnt->y, -1.0f + zRecip);
		pPnt++;
	}
	glEnd();  

	if (Flags & JE_RENDER_FLAG_NO_ZTEST)		// We are assuming that this is not going to change all that much
		glEnable(GL_DEPTH_TEST);

	if (Flags & JE_RENDER_FLAG_NO_ZWRITE)	// We are assuming that this is not going to change all that much
		glDepthMask(1);

	return JE_TRUE;
}

jeBoolean DRIVERCC OGLDrv_DrawDecal(jeTexture *THandle, RECT *SrcRect, int32 x, int32 y)
{
	RECT tmpRect, *srcRect;
	GLint width, height;
	GLfloat uClamp, vClamp;
	GLfloat uDiff = 1.0f, vDiff = 1.0f;
	RECT crect;

	GetClientRect(ClientWindow.hWnd, &crect);

	glOrtho(crect.left, crect.right, crect.bottom, crect.top, -1.0f, 4000.0f);

	if(!SrcRect)
	{
		tmpRect.left = 0;
		tmpRect.right = THandle->Width;
		tmpRect.top = 0;
		tmpRect.bottom = THandle->Height;

		srcRect = &tmpRect;

		width = (THandle->Width);
		height = (THandle->Height);
	}
	else
	{
		srcRect = SrcRect;

		width = (srcRect->right - srcRect->left);
		height = (srcRect->bottom - srcRect->top);
	}

	if(x + width <= 0 || y + height <= 0 || x >= ClientWindow.Width || y >= ClientWindow.Height)
		return JE_TRUE;

	if(x + width >= (ClientWindow.Width - 1))
		srcRect->right -= ((x + width) - (ClientWindow.Width - 1));
	
	if(y + height >= (ClientWindow.Height - 1))
		srcRect->bottom -= ((y + height) - (ClientWindow.Height - 1));
	
	if(x < 0)
	{
		srcRect->left += -x;
		x = 0;
	}

	if(y < 0)
	{
		srcRect->top += -y;
		y = 0;
	}

	if(boundTexture != THandle->TextureID)
	{
		glBindTexture(GL_TEXTURE_2D, THandle->TextureID);
		boundTexture = THandle->TextureID;
	}

	if(THandle->Flags & THANDLE_UPDATE)
		THandle_Update(THandle);
	
	glDisable(GL_DEPTH_TEST);
	glColor4f(1.0, 1.0, 1.0, 1.0);

	glShadeModel(GL_FLAT);

	if(THandle->Data[1] == NULL)
	{
		uClamp = width / (GLfloat)THandle->PaddedWidth;
		vClamp = height / (GLfloat)THandle->PaddedHeight;

		glMatrixMode(GL_TEXTURE); 
		glPushMatrix();
		glLoadIdentity();
		glTranslatef(srcRect->left / (GLfloat)THandle->PaddedWidth, srcRect->top / (GLfloat)THandle->PaddedHeight, 0.0f);

		glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 0.0);
		glVertex2i(x, y);
		glTexCoord2f(uClamp, 0.0f);
		glVertex2i(x + width, y);

		glTexCoord2f(uClamp, vClamp);
		glVertex2i(x + width, y + height);

		glTexCoord2f(0.0f, vClamp);
		glVertex2i(x, y + height);

		glEnd();  

		glPopMatrix();
		glMatrixMode(GL_MODELVIEW);
	}
	else
	{
		glPixelStorei(GL_UNPACK_ROW_LENGTH, THandle->Width);
		glPixelStorei(GL_UNPACK_SKIP_PIXELS, srcRect->left);
		glPixelStorei(GL_UNPACK_SKIP_ROWS, srcRect->top);

		glPixelZoom(1.0, -1.0);

		if(width <= 256 && height <= 256 && width == SnapToPower2(width) && height == SnapToPower2(height))
		{
			// Last chance to avoid the dreaded glDrawPixels...Yes, this is faster
			// than glDrawPixels on the ICD's I've tested.  Go figure.
			// (Could add a more complex texture upload/clamp system for
			//  width/heights other than powers of 2, but for now,
			//  this is enough complexity...Sorry, 3DFX)
			if(decalTexObj == 0)
				glGenTextures(1, &decalTexObj);
			
			glBindTexture(GL_TEXTURE_2D, decalTexObj);
			boundTexture = decalTexObj;

			glTexImage2D(GL_TEXTURE_2D, 0, 4, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, THandle->Data[1]);

			glBegin(GL_QUADS);
			glTexCoord2f(0.0f, 0.0);
			glVertex2i(x, y);

			glTexCoord2f(1.0f, 0.0f);
			glVertex2i(x + width, y);

			glTexCoord2f(1.0f, 1.0f);
			glVertex2i(x + width, y + height);

			glTexCoord2f(0.0f, 1.0f);
			glVertex2i(x, y + height);

			glEnd();
		}
		else
		{
			glPixelZoom(1.0, -1.0);
			glRasterPos2i(x, y);
			glDrawPixels(width, height, GL_RGBA, GL_UNSIGNED_BYTE, THandle->Data[1]);
		}

		glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
		glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
		glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
	} 

	glShadeModel(GL_SMOOTH);
	glEnable(GL_DEPTH_TEST); 
	gluPerspective(70.0f, ClientWindow.Width/ClientWindow.Height, 1.0f, 4000.0f);

	return JE_TRUE; 
}

jeBoolean DRIVERCC OGLDrv_Screenshot(const char *Name)
{
	unsigned char tgaHeader[18];
	GLubyte *buffer;
	FILE *fp;
	char *newName;
	int nameLen;

	buffer = new GLubyte[ClientWindow.Width * ClientWindow.Height * 3];

	glFinish();

	glReadPixels(0, 0, ClientWindow.Width, ClientWindow.Height, GL_BGR, GL_UNSIGNED_BYTE, buffer);
 
	memset(tgaHeader, 0, sizeof(tgaHeader));
	tgaHeader[2] = 2;
	tgaHeader[12] = (unsigned char)ClientWindow.Width;
	tgaHeader[13] = (unsigned char)((unsigned long)ClientWindow.Width >> 8);
	tgaHeader[14] = (unsigned char)ClientWindow.Height;
	tgaHeader[15] = (unsigned char)((unsigned long)ClientWindow.Height >> 8);
	tgaHeader[16] = 24;
 
	// Convert the extention (if one exists) to .tga.  They probably expect a .bmp.
	newName = strdup(Name);
	nameLen = (int)strlen(newName);

	if(nameLen > 3)
	{
		if(newName[nameLen - 4] == '.')
			strcpy(newName + nameLen - 3, "tga");
	}

    fp = fopen(newName, "wb");
    
	free(newName);

	if(fp == NULL) 
	{
		delete [] buffer;
        return JE_FALSE;
    }
 
    fwrite(tgaHeader, 1, 18, fp);
    fwrite(buffer, 3, ClientWindow.Width * ClientWindow.Height, fp);
    fclose(fp);
 
	delete [] buffer;
	
	return JE_TRUE;
}

jeBoolean DRIVERCC OGLDrv_GetGamma(float *Gamma)
{
	*Gamma = CurrentGamma;
	return JE_TRUE;
}

jeBoolean DRIVERCC OGLDrv_SetGamma(float Gamma)
{
	GLfloat lut[256];
	GLint	i;
	
	CurrentGamma = Gamma;
	
 	for(i = 0; i < 256; i++)
		lut[i] = (GLfloat)pow(i / 255.0, 1.0 / CurrentGamma);
	
	glPixelTransferi(GL_MAP_COLOR, GL_TRUE);
	glPixelMapfv(GL_PIXEL_MAP_R_TO_R, 256, lut);
	glPixelMapfv(GL_PIXEL_MAP_G_TO_G, 256, lut);
	glPixelMapfv(GL_PIXEL_MAP_B_TO_B, 256, lut); 

	return JE_TRUE;
}

jeBoolean DRIVERCC OGLDrv_SetMatrix(uint32 Type, jeXForm3d *Matrix)
{
	return JE_TRUE;
}

jeBoolean DRIVERCC OGLDrv_GetMatrix(uint32 Type, jeXForm3d *Matrix)
{
	return JE_TRUE;
}

jeBoolean DRIVERCC OGLDrv_SetCamera(jeCamera *Camera)
{
	return JE_TRUE;
}

jeBoolean DRIVERCC OGLDrv_GetDeviceCaps(jeDeviceCaps *DeviceCaps)
{
	DeviceCaps->SuggestedDefaultRenderFlags = JE_RENDER_FLAG_BILINEAR_FILTER;
	DeviceCaps->CanChangeRenderFlags = 0xFFFFFFFF;

	return JE_TRUE;
}

jeBoolean DRIVERCC OGLDrv_SetRenderState(uint32 state, uint32 value)
{
	switch (state)
	{
	case JE_RENDERSTATE_ENABLE_ZBUFFER:
		{
			if ((jeBoolean)value == JE_TRUE)
				glEnable(GL_DEPTH_TEST);
			else
				glDisable(GL_DEPTH_TEST);

			break;
		}
	case JE_RENDERSTATE_ENABLE_ZWRITES:
		{
			glDepthMask((GLboolean)value);
			break;
		}
	case JE_RENDERSTATE_ENABLE_ALPHABLENDING:
		{
			if ((jeBoolean)value == JE_TRUE)
				glEnable(GL_BLEND);
			else
				glDisable(GL_BLEND);

			break;
		}
	case JE_RENDERSTATE_ALPHAREF:
		{
			AlphaRef = ((float)value) / 256.0f;
			glAlphaFunc(AlphaFunc, AlphaRef);
			break;
		}
	case JE_RENDERSTATE_ALPHAFUNC:
		{
			if (value == JE_CMP_NEVER)
				AlphaFunc = GL_NEVER;
			else if (value == JE_CMP_LESS)
				AlphaFunc = GL_LESS;
			else if (value == JE_CMP_EQUAL)
				AlphaFunc = GL_EQUAL;
			else if (value == JE_CMP_LEQUAL)
				AlphaFunc = GL_LEQUAL;
			else if (value == JE_CMP_GREATER)
				AlphaFunc = GL_GREATER;
			else if (value == JE_CMP_GEQUAL)
				AlphaFunc = GL_GEQUAL;
			else if (value == JE_CMP_NEQUAL)
				AlphaFunc = GL_NOTEQUAL;
			else if (value == JE_CMP_ALWAYS)
				AlphaFunc = GL_ALWAYS;

			glAlphaFunc(AlphaFunc, AlphaRef);
			break;
		}
	case JE_RENDERSTATE_ENABLE_ALPHATESTING:
		{
			if ((jeBoolean)value == JE_TRUE)
				glEnable(GL_ALPHA_TEST);
			else
				glDisable(GL_ALPHA_TEST);

			break;
		}
	case JE_RENDERSTATE_DEPTHFUNC:
		{
			GLenum					func;
			
			if (value == JE_CMP_NEVER)
				func = GL_NEVER;
			else if (value == JE_CMP_LESS)
				func = GL_LESS;
			else if (value == JE_CMP_EQUAL)
				func = GL_EQUAL;
			else if (value == JE_CMP_LEQUAL)
				func = GL_LEQUAL;
			else if (value == JE_CMP_GREATER)
				func = GL_GREATER;
			else if (value == JE_CMP_GEQUAL)
				func = GL_GEQUAL;
			else if (value == JE_CMP_NEQUAL)
				func = GL_NOTEQUAL;
			else if (value == JE_CMP_ALWAYS)
				func = GL_ALWAYS;

			glDepthFunc(func);
			break;
		}
	case JE_RENDERSTATE_FILLMODE:
		{
			if (value == JE_FILL_POINT)
				glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
			else if (value == JE_FILL_WIREFRAME)
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			else if (value == JE_FILL_SOLID)
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

			break;
		}
	case JE_RENDERSTATE_SHADEMODE:
		{
			if (value == JE_SHADE_FLAT)
				glShadeModel(GL_FLAT);
			else if (value == JE_SHADE_GOURAUD)
				glShadeModel(GL_SMOOTH);

			break;
		}
	case JE_RENDERSTATE_CULLMODE:
		{
			if (value == JE_CULL_NONE)
				glDisable(GL_CULL_FACE);
			else if (value == JE_CULL_CW)
			{
				glEnable(GL_CULL_FACE);
				glFrontFace(GL_CW);
				glCullFace(GL_BACK);
			}
			else if (value == JE_CULL_CCW)
			{
				glEnable(GL_CULL_FACE);
				glFrontFace(GL_CCW);
				glCullFace(GL_BACK);
			}

			break;
		}
	case JE_RENDERSTATE_ENABLE_FOG:
		{
			if ((jeBoolean)value == JE_TRUE)
				glEnable(GL_FOG);
			else
				glDisable(GL_FOG);

			break;
		}
	case JE_RENDERSTATE_FOGCOLOR:
		{
			float					color[4];
			uint32					a, r, g, b;

			JE_COLOR_GETARGB(value, a, r, g, b);
			color[0] = (float)(r / 255);
			color[1] = (float)(g / 255);
			color[2] = (float)(b / 255);
			color[3] = (float)(a / 255);

			glFogfv(GL_FOG_COLOR, color);
			break;
		}
	case JE_RENDERSTATE_FOGSTART:
		{
			glFogf(GL_FOG_START, (float)value);
			break;
		}
	case JE_RENDERSTATE_FOGEND:
		{
			glFogf(GL_FOG_END, (float)value);
			break;
		}
	case JE_RENDERSTATE_HWLIGHTINGENABLE:
		{
			if ((jeBoolean)value == JE_TRUE)
				glEnable(GL_LIGHTING);
			else
				glDisable(GL_LIGHTING);

			break;
		}
	case JE_RENDERSTATE_AMBIENTLIGHT:
		{
			float					color[4];
			uint32					a, r, g, b;

			JE_COLOR_GETARGB(value, a, r, g, b);
			color[0] = (float)(r / 255);
			color[1] = (float)(g / 255);
			color[2] = (float)(b / 255);
			color[3] = (float)(a / 255);

			glLightModelfv(GL_LIGHT_MODEL_AMBIENT, color);
			break;
		}
	case JE_RENDERSTATE_ENABLE_STENCIL:
		{
			if ((jeBoolean)value == JE_TRUE)
				glEnable(GL_STENCIL_TEST);
			else
				glDisable(GL_STENCIL_TEST);

			break;
		}
	case JE_RENDERSTATE_STENCILREF:
		{
			StencilRef = (GLfloat)value;
			glStencilFunc(StencilFunc, value, 0x00000000);
			break;
		}
	case JE_RENDERSTATE_STENCILMASK:
		{
			glStencilMask(value);
			break;
		}
	case JE_RENDERSTATE_STENCILWRITEMASK:
		{
			glStencilMask(value);
			break;
		}
	case JE_RENDERSTATE_STENCILFUNC:
		{
			if (value == JE_CMP_NEVER)
				StencilFunc = GL_NEVER;
			else if (value == JE_CMP_LESS)
				StencilFunc = GL_LESS;
			else if (value == JE_CMP_EQUAL)
				StencilFunc = GL_EQUAL;
			else if (value == JE_CMP_LEQUAL)
				StencilFunc = GL_LEQUAL;
			else if (value == JE_CMP_GREATER)
				StencilFunc = GL_GREATER;
			else if (value == JE_CMP_GEQUAL)
				StencilFunc = GL_GEQUAL;
			else if (value == JE_CMP_NEQUAL)
				StencilFunc = GL_NOTEQUAL;
			else if (value == JE_CMP_ALWAYS)
				StencilFunc = GL_ALWAYS;

			glStencilFunc(StencilFunc, (GLint)StencilRef, 0x00000000);
			break;
		}
	case JE_RENDERSTATE_STENCILFAIL:
		{
			GLenum					func;

			if (value == JE_CMP_NEVER)
				func = GL_NEVER;
			else if (value == JE_CMP_LESS)
				func = GL_LESS;
			else if (value == JE_CMP_EQUAL)
				func = GL_EQUAL;
			else if (value == JE_CMP_LEQUAL)
				func = GL_LEQUAL;
			else if (value == JE_CMP_GREATER)
				func = GL_GREATER;
			else if (value == JE_CMP_GEQUAL)
				func = GL_GEQUAL;
			else if (value == JE_CMP_NEQUAL)
				func = GL_NOTEQUAL;
			else if (value == JE_CMP_ALWAYS)
				func = GL_ALWAYS;

			glStencilOp(func, func, func);
			break;
		}
	case JE_RENDERSTATE_STENCILZFAIL:
		{
			GLenum					func;

			if (value == JE_CMP_NEVER)
				func = GL_NEVER;
			else if (value == JE_CMP_LESS)
				func = GL_LESS;
			else if (value == JE_CMP_EQUAL)
				func = GL_EQUAL;
			else if (value == JE_CMP_LEQUAL)
				func = GL_LEQUAL;
			else if (value == JE_CMP_GREATER)
				func = GL_GREATER;
			else if (value == JE_CMP_GEQUAL)
				func = GL_GEQUAL;
			else if (value == JE_CMP_NEQUAL)
				func = GL_NOTEQUAL;
			else if (value == JE_CMP_ALWAYS)
				func = GL_ALWAYS;

			glStencilOp(func, func, func);
			break;
		}
	case JE_RENDERSTATE_STENCILPASS:
		{
			GLenum					func;

			if (value == JE_CMP_NEVER)
				func = GL_NEVER;
			else if (value == JE_CMP_LESS)
				func = GL_LESS;
			else if (value == JE_CMP_EQUAL)
				func = GL_EQUAL;
			else if (value == JE_CMP_LEQUAL)
				func = GL_LEQUAL;
			else if (value == JE_CMP_GREATER)
				func = GL_GREATER;
			else if (value == JE_CMP_GEQUAL)
				func = GL_GEQUAL;
			else if (value == JE_CMP_NEQUAL)
				func = GL_NOTEQUAL;
			else if (value == JE_CMP_ALWAYS)
				func = GL_ALWAYS;

			glStencilOp(func, func, func);
			break;
		}
	}

	return JE_TRUE;
}

DRV_Driver OGLDRV = 
{
	"OpenGL Driver v0.1",
	DRV_VERSION_MAJOR,
	DRV_VERSION_MINOR,

	DRV_ERROR_NONE,
	NULL,

	OGLDrv_EnumSubDrivers,
	OGLDrv_EnumModes,

	OGLDrv_EnumPixelFormats,

	OGLDrv_GetDeviceCaps,

	OGLDrv_Init,
	OGLDrv_Shutdown,
	OGLDrv_Reset,
	OGLDrv_UpdateWindow,
	OGLDrv_SetActive,

	OGLDrv_THandle_Create,
	NULL,
	OGLDrv_THandle_Destroy,

	OGLDrv_THandle_Lock,
	OGLDrv_THandle_Unlock,

	NULL,
	NULL,

	NULL,
	NULL,

	OGLDrv_THandle_GetInfo,

	OGLDrv_BeginScene,
	OGLDrv_EndScene,
	OGLDrv_BeginBatch,
	OGLDrv_EndBatch,

	OGLDrv_RenderGouraudPoly,
	OGLDrv_RenderWorldPoly,
	OGLDrv_RenderMiscTexturePoly,

	OGLDrv_DrawDecal,

	0, 0, 0,

	NULL,

	OGLDrv_Screenshot,

	OGLDrv_SetGamma,
	OGLDrv_GetGamma,

	OGLDrv_SetMatrix,
	OGLDrv_GetMatrix,
	OGLDrv_SetCamera,

	NULL,
	NULL,

	NULL,
	NULL,

	NULL,
	NULL,
	NULL,

	NULL,
	NULL,
	NULL,

	OGLDrv_SetRenderState
};

DRIVERAPI BOOL DriverHook(DRV_Driver **Driver)
{
	ogllog = fopen("OGLDrv.log", "wt");
	fprintf(ogllog, "OpenGL Driver Log\n\n");

	EngineSettings.CanSupportFlags = (DRV_SUPPORT_ALPHA | DRV_SUPPORT_COLORKEY);
	EngineSettings.PreferenceFlags = 0;

	OGLDRV.EngineSettings = &EngineSettings;
    
	*Driver = &OGLDRV;

	THandle_Startup();

	// Make sure the error string ptr is not null, or invalid!!!
    OGLDRV.LastErrorStr = LastErrorStr;

	//SetLastDrvError(DRV_ERROR_NONE, "OGL:  No error.");
	strcpy(OGLDRV.LastErrorStr, "No Error");
	OGLDRV.LastError = DRV_ERROR_NONE;

	return JE_TRUE;
}