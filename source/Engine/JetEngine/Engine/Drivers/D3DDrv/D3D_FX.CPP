/****************************************************************************************/
/*  D3D_Fx.cpp                                                                          */
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
#include <Windows.h>
#include <stdio.h>
#include <DDraw.h>
#include <D3D.h>

#include "D3DDrv.h"
#include "DCommon.h"
#include "d3d_Render.h"
#include "d3d_Scene.h"
#include "D3D_FX.h"
#include "D3D_Main.h"
#include "D3D_Err.h"

static LPDIRECTDRAWSURFACE7	OldTexture[8];
static BOOL					OldBlend = FALSE;
static D3DBLEND				OldSFunc = D3DBLEND_ONE;
static D3DBLEND				OldDFunc = D3DBLEND_ONE;
static BOOL					OldWrap = FALSE;

void D3DFx_InitStaticsAndGlobals(void)
{
	memset(OldTexture, 0, sizeof(OldTexture));
	OldSFunc = D3DBLEND_ONE;
	OldDFunc = D3DBLEND_ONE;
	OldBlend = FALSE;
	OldWrap = FALSE;
}

//======================================================================================================
//======================================================================================================
void D3DSetTexture(int32 Stage, LPDIRECTDRAWSURFACE7 Texture)
{
	if (Texture == OldTexture[Stage])
		return;

	OldTexture[Stage] = Texture;

	D3DInfo.lpD3DDevice->SetTexture(Stage, Texture);
}
		
//======================================================================================================
//======================================================================================================
void D3DBilinearFilter(D3DTEXTUREFILTER Min, D3DTEXTUREFILTER Mag)
{
	D3DInfo.lpD3DDevice->SetTextureStageState(0, D3DTSS_MINFILTER, D3DTFN_LINEAR);
	D3DInfo.lpD3DDevice->SetTextureStageState(0, D3DTSS_MAGFILTER, D3DTFG_LINEAR);

	if (D3DInfo.CanDoMultiTexture)
	{
		D3DInfo.lpD3DDevice->SetTextureStageState(1, D3DTSS_MINFILTER, D3DTFN_LINEAR);
		D3DInfo.lpD3DDevice->SetTextureStageState(1, D3DTSS_MAGFILTER, D3DTFG_LINEAR);
	}
}

//======================================================================================================
//	D3DTexturedPoly
//======================================================================================================
void D3DTexturedPoly(void *Pnts, int32 NumPoints)
{
	D3DInfo.lpD3DDevice->DrawPrimitive(	D3DPT_TRIANGLEFAN, 
										D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX2, 
										Pnts, 
										NumPoints, 
										0 /*D3DDP_WAIT*/ );

#if 0	// For debugging!
	D3DInfo.lpD3DDevice->SetRenderState(D3DRENDERSTATE_FLUSHBATCH, 0);
	D3DInfo.lpD3DDevice->EndScene();
	D3DInfo.lpD3DDevice->BeginScene();
#endif
}	

//======================================================================================================
//======================================================================================================
void D3DBlendFunc (D3DBLEND SFunc, D3DBLEND DFunc)
{
	if (SFunc != OldSFunc)
	{
		D3DInfo.lpD3DDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND, SFunc);
		OldSFunc = SFunc;
	}
	if (DFunc != OldDFunc)
	{
		D3DInfo.lpD3DDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, DFunc);
		OldDFunc = DFunc;
	}

}

//======================================================================================================
//======================================================================================================
void D3DBlendEnable(BOOL Enable)
{
	if (OldBlend == Enable)
		return;

	D3DInfo.lpD3DDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, Enable);

	OldBlend = Enable;
}

void D3DTexWrap(DWORD Stage, BOOL Wrap)
{
	if (OldWrap == Wrap)
		return;

	OldWrap = Wrap;

	if (Wrap)
	{
		D3DInfo.lpD3DDevice->SetTextureStageState(Stage, D3DTSS_ADDRESS, D3DTADDRESS_WRAP);
		D3DInfo.lpD3DDevice->SetTextureStageState(Stage, D3DTSS_ADDRESSU, D3DTADDRESS_WRAP);
		D3DInfo.lpD3DDevice->SetTextureStageState(Stage, D3DTSS_ADDRESSV, D3DTADDRESS_WRAP);
	}
	else
	{
		D3DInfo.lpD3DDevice->SetTextureStageState(Stage, D3DTSS_ADDRESS, D3DTADDRESS_CLAMP);
		D3DInfo.lpD3DDevice->SetTextureStageState(Stage, D3DTSS_ADDRESSU, D3DTADDRESS_CLAMP);
		D3DInfo.lpD3DDevice->SetTextureStageState(Stage, D3DTSS_ADDRESSV, D3DTADDRESS_CLAMP);
	}
}

void D3DZWriteEnable (BOOL Enable)
{
	D3DInfo.lpD3DDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, Enable);
}

void D3DZFunc (D3DCMPFUNC Func)
{
	D3DInfo.lpD3DDevice->SetRenderState(D3DRENDERSTATE_ZFUNC, Func);
}

void D3DZEnable(BOOL Enable)
{
	D3DInfo.lpD3DDevice->SetRenderState(D3DRENDERSTATE_ZENABLE, Enable);
}

void D3DPolygonMode (D3DFILLMODE Mode)
{
   D3DInfo.lpD3DDevice->SetRenderState(D3DRENDERSTATE_FILLMODE, Mode);
}

