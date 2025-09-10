/*!
	@file jeEngine.h 
	
	@author Charles Bloom/John Pollard
	@brief Maintains the driver interface, as well as the bitmaps attached to the driver.

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

	@note C++ conversion done by Anthony Rufrano (paradoxnj)
*/
#ifndef JE_ENGINE_H
#define JE_ENGINE_H

#include <string>

#include "BaseType.h"
#include "jeTypes.h"

// Forward Declarations
class jeEngine;
class jeDriver_System;
class jeDriver;
class jeDriver_Mode;

class jeCamera;
class jeTexture;
class jeShader;
class jeVertexBuffer;
class jeIndexBuffer;
class jeFont;

enum jeEngine_FrameState
{
	FrameState_None = 0,
	FrameState_Begin,
};

#define JE_FONT_NORMAL				0x00000001
#define JE_FONT_BOLD				0x00000002

/*!
	@fn jeBoolean jeEngine_Initialize(HWND hWnd, std::string AppName, std::string DriverDirectory)
	@brief Initializes the jeEngine singleton
	@param[in] hWnd The window handle to initialize with
	@param[in] AppName The application name
	@param[in] DriverDirectory The directory where the driver DLL's are located.
	@return JE_TRUE on success, JE_FALSE on failure
*/
JETAPI jeBoolean JETCC jeEngine_Initialize(HWND hWnd, std::string AppName, std::string DriverDirectory);

/*!
	@class jeEngine jeEngine.h "include\jeEngine.h"
	@brief The engine core class
*/
class jeEngine : virtual public jeUnknown
{
protected:
	virtual ~jeEngine()							{}

public:
	virtual void						EnableFrameRateCounter(jeBoolean Enable) = 0;
	virtual jeBoolean					Activate(jeBoolean Active) = 0;

	virtual jeBoolean					UpdateWindow() = 0;

	virtual jeBoolean					GetFrameState(uint32 *State) = 0;

	virtual jeBoolean					SetMatrix(uint32 Type, jeXForm3d *Matrix) = 0;
	virtual jeBoolean					GetMatrix(uint32 Type, jeXForm3d *Matrix) = 0;
	virtual jeBoolean					SetCamera(jeCamera *Camera) = 0;

	virtual jeBoolean					BeginFrame(jeCamera *Camera, jeBoolean ClearScreen) = 0;
	virtual jeBoolean					EndFrame() = 0;
	virtual jeBoolean					FlushScene() = 0;

	virtual jeBoolean					DrawBitmap(jeTexture *Texture, jeRect *SrcRect, int32 x, int32 y) = 0;

	virtual jeBoolean					SetGamma(float Gamma) = 0;
	virtual jeBoolean					GetGamma(float *Gamma) = 0;
	virtual jeBoolean					UpdateGamma() = 0;

	virtual jeBoolean					Screenshot(std::string filename) = 0;
	virtual jeBoolean					Printf(jeFont *Font, int32 x, int32 y, uint32 Color, const char *fmt, ...) = 0;

	virtual jeDriver_System				*GetDriverSystem() = 0;

	virtual jeBoolean					RegisterObject(std::string objectfilename) = 0;
	virtual jeBoolean					RegisterObjects(std::string objectdir) = 0;

	virtual jeBoolean					SetDriverAndMode(jeDriver *Driver, jeDriver_Mode *Mode) = 0;
	virtual jeBoolean					ShutdownDriver() = 0;

	virtual jeTexture					*CreateTexture(int32 Width, int32 Height, jeBoolean GenerateMipLevels) = 0;
	virtual jeTexture					*CreateTextureFromFile(jeVFile *File, jeBoolean GenerateMipLevels) = 0;

	virtual jeFont						*CreateFont(int32 Height, int32 Width, uint32 Weight, jeBoolean Italic, std::string FaceName) = 0;
	
	virtual jeVertexBuffer				*CreateVertexBuffer(int32 NumVerts) = 0;
	virtual jeIndexBuffer				*CreateIndexBuffer(int32 NumIndices) = 0;

	virtual jeShader					*CreateShaderFromFile(jeVFile *File) = 0;
};

#endif
