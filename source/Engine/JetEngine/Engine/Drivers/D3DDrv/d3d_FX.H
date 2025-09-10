/****************************************************************************************/
/*  D3D_Fx.h                                                                            */
/*                                                                                      */
/*  Author: John Pollard                                                                */
/*  Description: D3D renderstate wrapper                                                */
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
#ifndef D3D_FX_H
#define D3D_FX_H

#include <Windows.h>
#include <DDraw.h>
#include <D3D.h>


#include "D3D_Main.h"
#include "DCommon.h"

void D3DFx_InitStaticsAndGlobals(void);

void D3DSetTexture(int32 Stage, LPDIRECTDRAWSURFACE7 Texture);
void D3DTexturedPoly(void *Pnts, int32 NumPoints);

void D3DBilinearFilter(D3DTEXTUREFILTER Min, D3DTEXTUREFILTER Mag);
void D3DBlendEnable(BOOL Enable);

void D3DBlendFunc (D3DBLEND SFunc, D3DBLEND DFunc);

void D3DZWriteEnable (BOOL Enable);
void D3DZFunc (D3DCMPFUNC Func);
void D3DZEnable(BOOL Enable);

void D3DTexWrap(DWORD Stage, BOOL Wrap);

void D3DPolygonMode (D3DFILLMODE Mode);

#endif
