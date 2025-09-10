/****************************************************************************************/
/*  Render.h                                                                            */
/*                                                                                      */
/*  Author: John Pollard                                                                */
/*  Description: Code to render polys under D3D                                         */
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
#ifndef D3D_RENDER_H
#define D3D_RENDER_H

#include <Windows.h>

#include "DCommon.h"

jeBoolean DRIVERCC RenderGouraudPoly(jeTLVertex *Pnts, int32 NumPoints, uint32 Flags);
jeBoolean DRIVERCC RenderWorldPoly(jeTLVertex *Pnts, int32 NumPoints, jeRDriver_Layer *Layers, int32 NumLayers, void *LMapCBContext, uint32 Flags);
jeBoolean DRIVERCC RenderMiscTexturePoly(jeTLVertex *Pnts, int32 NumPoints, jeRDriver_Layer *Layers, int32 NumLayers, uint32 Flags);
jeBoolean DRIVERCC DrawDecal(jeTexture *THandle, RECT *SRect, int32 x, int32 y);

#endif
