/****************************************************************************************/
/*  D3DCACHE.H                                                                          */
/*                                                                                      */
/*  Author:                                                                             */
/*  Description:                                                                        */
/*                                                                                      */
/*  The contents of this file are subject to the Jet3D Public License                   */
/*  Version 1.02 (the "License"); you may not use this file except in                   */
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
/****************************************************************************************/
/*  D3DCache.h                                                                          */
/*                                                                                      */
/*  Author: John Pollard                                                                */
/*  Description:                                                                        */
/*                                                                                      */
/*  Copyright (c) 1997, 1999, Eclipse Entertainment; All rights reserved.               */
/*                                                                                      */
/*  See the accompanying file LICENSE.TXT for terms on the use of this library.         */
/*  This library is distributed in the hope that it will be useful but WITHOUT          */
/*  ANY WARRANTY OF ANY KIND and without any implied warranty of MERCHANTABILITY        */
/*  or FITNESS FOR ANY PURPOSE.  Refer to LICENSE.TXT for more details.                 */
/*                                                                                      */
/****************************************************************************************/
#ifndef D3DCache_H
#define D3DCache_H

#include <Windows.h>
#include <DDraw.h>
#include <D3D.H>

#include "BaseType.h"
#include "d3d_DDMemMgr.h"

#define	D3DCACHE_MAX_NAME			256

typedef struct THandle_MipData		THandle_MipData;

typedef struct D3DCache				D3DCache;
typedef struct D3DCache_Type		D3DCache_Type;
typedef struct D3DCache_Slot		D3DCache_Slot;

void		D3DCache_InitStaticsAndGlobals(void);
D3DCache	*D3DCache_Create(const char *Name, LPDIRECTDRAW7 lpDD, DDMemMgr_Partition *Partition,jeBoolean UseStages);
void		D3DCache_Destroy(D3DCache *Cache);
jeBoolean	D3DCache_IsValid(D3DCache *Cache);
jeBoolean	D3DCache_EvictAllSurfaces(D3DCache *Cache);
D3DCache_Type *D3DCache_TypeCreate(D3DCache *Cache, int32 Width, int32 Height, int32 NumMipLevels, int32 Stage, const DDSURFACEDESC2 *ddsd);
void		D3DCache_TypeDestroy(D3DCache_Type *CacheType);
jeBoolean	D3DCache_TypeIsValid(D3DCache_Type *Type);
jeBoolean	D3DCache_CheckSlots(D3DCache *Cache);
jeBoolean	D3DCache_SlotIsValid(D3DCache_Slot *Slot);
int32		D3DCache_SetupSlot(D3DCache *Cache, D3DCache_Slot *Slot, int32 Width, int32 Height, const DDSURFACEDESC2 *SurfDesc);
D3DCache_Slot *D3DCache_TypeFindSlot(D3DCache_Type *CacheType);
void		D3DCache_TypeMissed(D3DCache_Type *CacheType,int KickFrame);

void		D3DCache_SlotSetLRU(D3DCache_Slot *Slot, uint32 LRU);
uint32		D3DCache_SlotGetLRU(D3DCache_Slot *Slot);

LPDIRECT3DTEXTURE2 D3DCache_SlotGetTexture(D3DCache_Slot *Slot);
LPDIRECTDRAWSURFACE7 D3DCache_SlotGetSurface(D3DCache_Slot *Slot);

void			D3DCache_SlotSetMipData(D3DCache_Slot *Slot, THandle_MipData *MipData);
THandle_MipData *D3DCache_SlotGetMipData(D3DCache_Slot *Slot);

void D3DCache_Update(D3DCache *Cache);

void D3DCache_TypeUsed(D3DCache_Type *CacheType);

#endif

