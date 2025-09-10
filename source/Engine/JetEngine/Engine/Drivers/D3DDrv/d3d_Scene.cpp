/****************************************************************************************/
/*  Scene.cpp                                                                           */
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
#include <Windows.h>
#include <stdio.h>

#include "D3DDrv.h"
#include "DCommon.h"
#include "d3d_Scene.h"
#include "d3d_Render.h"
#include "d3d_GSpan.h"
#include "D3DCache.h"
#include "D3D_Fx.h"
#include "D3D_Main.h"
#include "d3d_PCache.h"
#include "D3D_Err.h"
#include "d3d_THandle.h"

//#define D3D_MANAGE_TEXTURES
#define SUPER_FLUSH

int32 RenderMode;
uint32 Scene_CurrentFrame;

BOOL DRIVERCC D3DBeginScene(BOOL Clear, BOOL ClearZ, RECT *WorldRect, jeBoolean WireFrame)
{
	HRESULT	Result{};

	Scene_CurrentFrame++;

	if (!D3DInfo.lpD3DDevice)
	{
		D3DMain_Log("BeginScene:  No D3D Device!.");
		return FALSE;
	}

	PCache_BeginScene();

	if (!THandle_CheckCache())
		return JE_FALSE;

	//	Watch for inactive app or minimize
	if(D3DInfo.RenderingIsOK)
	{
		if (!Main_ClearBackBuffer(Clear, ClearZ))
		{
			D3DMain_Log("D3DClearBuffers failed.");
			return FALSE;
		}
		
		D3DDRV.NumRenderedPolys = 0;
		
		Result = D3DInfo.lpD3DDevice->BeginScene();

		if (Result != D3D_OK)
		{
			D3DMain_Log("BeginScene:  D3D BeginScene Failed.\n%s.", D3DErrorToString(Result));
			return FALSE;
		}

      D3DInfo.WireFrame = WireFrame;

		D3DBilinearFilter(D3DFILTER_LINEAR, D3DFILTER_LINEAR);
   
      Result = D3DInfo.lpD3DDevice->SetRenderState(D3DRENDERSTATE_FILLMODE, D3DInfo.WireFrame?D3DFILL_WIREFRAME:D3DFILL_SOLID);
      //D3DPolygonMode (D3DInfo.WireFrame?D3DFILL_WIREFRAME:D3DFILL_SOLID);
		
		D3DZWriteEnable (TRUE);
		D3DZEnable(TRUE);
		D3DZFunc(D3DCMP_LESSEQUAL);
/*
		if(D3DInfo.Fog.Enabled)
		{
			//float fFogStart = 0.965f,fFogEnd = 1.0f;

			// 175,206, 253
			D3DInfo.lpD3DDevice->SetRenderState(D3DRENDERSTATE_FOGENABLE,true);
			D3DInfo.lpD3DDevice->SetRenderState(D3DRENDERSTATE_FOGCOLOR,((DWORD)D3DInfo.Fog.R<<16)|((DWORD)D3DInfo.Fog.G<<8)|(DWORD)D3DInfo.Fog.B);//0x00FFFFFF);//((DWORD)AppInfo.FogR<<16)|((DWORD)AppInfo.FogG<<8)|(DWORD)AppInfo.FogB
			D3DInfo.lpD3DDevice->SetRenderState(D3DRENDERSTATE_FOGTABLEMODE, D3DFOG_LINEAR);
			
  			D3DInfo.lpD3DDevice->SetRenderState(D3DRENDERSTATE_FOGTABLESTART,*(DWORD*)(&D3DInfo.Fog.Start));
  			D3DInfo.lpD3DDevice->SetRenderState(D3DRENDERSTATE_FOGTABLEEND, *(DWORD*)(&D3DInfo.Fog.End));
			D3DInfo.lpD3DDevice->SetRenderState(D3DRENDERSTATE_FOGTABLEDENSITY, (DWORD)0.004f);
		}
		else
		{
			D3DInfo.lpD3DDevice->SetRenderState(D3DRENDERSTATE_FOGENABLE,false);
		}
*/

		D3DBlendFunc (D3DBLEND_SRCALPHA, D3DBLEND_INVSRCALPHA);
		D3DBlendEnable(TRUE);
	
		if (!PCache_BeginBatch())
			return JE_FALSE;
	}

	return JE_TRUE;
}

BOOL DRIVERCC D3DEndScene(void)
{
	HRESULT		Result{};

	if (!D3DInfo.lpD3DDevice)
		return FALSE;

	if(D3DInfo.RenderingIsOK)
	{
		if (!PCache_EndBatch())
			return JE_FALSE;

		Result = D3DInfo.lpD3DDevice->EndScene();

		if (Result != D3D_OK)
		{
			D3DMain_Log("EndScene:  D3D EndScene Failed.\n%s", D3DErrorToString(Result));
			return FALSE;
		}

		if ( ! THandle_UpdateCaches() )
			return FALSE;

		if (!Main_ShowBackBuffer())
			return FALSE;
	}
	return TRUE;
}

