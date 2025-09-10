/****************************************************************************************/
/*  Scene.h                                                                             */
/*                                                                                      */
/*  Author: John Pollard                                                                */
/*  Description: Begin/EndScene code, etc                                               */
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
/****************************************************************************************/
#ifndef D3D_SCENE_H
#define D3D_SCENE_H

#include <Windows.H>

#include "DCommon.h"

#define RENDER_NONE				0
#define RENDER_WORLD			1
#define RENDER_MESHES			2
#define RENDER_MODELS			3

extern int32 RenderMode;
extern uint32 Scene_CurrentFrame;

BOOL DRIVERCC D3DBeginScene(BOOL Clear, BOOL ClearZ, RECT *WorldRect, jeBoolean Wireframe);
BOOL DRIVERCC D3DEndScene(void);

#endif

