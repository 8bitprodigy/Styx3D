/****************************************************************************************/
/*  Render.cpp                                                                          */
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
#include <Windows.h>
#include <stdio.h>

#include "D3DDrv.h"
#include "DCommon.h"
#include "d3d_Render.h"
#include "d3d_Scene.h"
#include "d3d_GSpan.h"
#include "D3D_Fx.h"
#include "D3DCache.h"
#include "D3D_Err.h"
#include "d3d_THandle.h"

#include "d3d_PCache.h"

#define SNAP_VERT(v)  ( ( v )  = ( float )( ( long )( ( v ) * 16 ) ) / 16.0f )

jeBoolean DRIVERCC RenderGouraudPoly(jeTLVertex *Pnts, int32 NumPoints, uint32 Flags)
{
	if(!D3DInfo.RenderingIsOK)
		return	TRUE;

	if (!PCache_InsertGouraudPoly(Pnts, NumPoints, Flags))
		return JE_FALSE;

	if (Flags & JE_RENDER_FLAG_FLUSHBATCH)
		PCache_FlushALLBatches();

	return TRUE;
}

jeBoolean DRIVERCC RenderWorldPoly(jeTLVertex *Pnts, int32 NumPoints, jeRDriver_Layer *Layers, int32 NumLayers, void *LMapCBContext, uint32 Flags)
{
	if(!D3DInfo.RenderingIsOK)
	{
		return	TRUE;
	}
	
	D3DDRV.NumRenderedPolys++;
	
	// Insert the poly into the world cache, for later rendering
	PCache_InsertWorldPoly(Pnts, NumPoints, Layers, NumLayers, LMapCBContext, Flags);

	if (Flags & JE_RENDER_FLAG_FLUSHBATCH)
		PCache_FlushALLBatches();

	return TRUE;
}

jeBoolean DRIVERCC RenderMiscTexturePoly(jeTLVertex *Pnts, int32 NumPoints, jeRDriver_Layer *Layers, int32 NumLayers, uint32 Flags)
{
	if(!D3DInfo.RenderingIsOK)
	{
		return	TRUE;
	}
				
	PCache_InsertMiscPoly(Pnts, NumPoints, Layers, NumLayers, Flags);

	if (Flags & JE_RENDER_FLAG_FLUSHBATCH)
		PCache_FlushALLBatches();

	return TRUE;
}

jeBoolean DRIVERCC DrawDecal(jeTexture *THandle, RECT *SRect, int32 x, int32 y)
{
	RECT	SRect2{}, * pSRect{};
	int32	Width{}, Height{};
	HRESULT	ddrval{};

	if(!D3DInfo.RenderingIsOK)
		return	TRUE;

	if (!SRect)
	{
		SRect2.left = 0;
		SRect2.right = THandle->Width;
		SRect2.top = 0;
		SRect2.bottom = THandle->Height;
		pSRect = &SRect2;
		Width = (THandle->Width);
		Height = (THandle->Height);
	}
	else
	{
		pSRect = SRect;
		Width = (pSRect->right - pSRect->left)+1;
		Height = (pSRect->bottom - pSRect->top)+1;
	}
	
	if (x + Width <= 0)
		return TRUE;
	if (y + Height <= 0)
		return TRUE;

	if (x >= D3DDrv_ClientWindow.Width)
		return TRUE;
	
	if (y >= D3DDrv_ClientWindow.Height)
		return TRUE;
	
	if (x + Width >= (D3DDrv_ClientWindow.Width-1))
		pSRect->right -= ((x + Width) - (D3DDrv_ClientWindow.Width-1));
	if (y + Height >= (D3DDrv_ClientWindow.Height-1))
		pSRect->bottom -= ((y + Height) - (D3DDrv_ClientWindow.Height-1));

	if (x < 0)
	{
		pSRect->left += -x;
		x=0;
	}
	if (y < 0)
	{
		pSRect->top += -y;
		y=0;
	}
	
#if 0
	ddrval = D3DInfo.lpBackBuffer->BltFast(x, y, THandle->MipData[0].Surface, pSRect, DDBLTFAST_SRCCOLORKEY | DDBLTFAST_WAIT);
#else
	RECT	DRect{};

	Width = (pSRect->right - pSRect->left);
	Height = (pSRect->bottom - pSRect->top);

	DRect.left = x;
	DRect.right = x+Width;
	DRect.top = y;
	DRect.bottom = y+Height;
	
	ddrval= D3DInfo.lpBackBuffer->Blt(&DRect, THandle->MipData[0].Surface, pSRect, 
		             (DDBLT_KEYSRC | DDBLT_WAIT), NULL);

	if(ddrval==DDERR_SURFACELOST)
	{
		if (!D3DMain_RestoreAllSurfaces())
			return	JE_FALSE;
	}
	//D3DInfo.lpBackBuffer->Blt(&DRect, Decals[Handle].Surface, pSRect, (DDBLT_WAIT), NULL);
#endif

	return JE_TRUE;
}

