/****************************************************************************************/
/*  TPage.h                                                                             */
/*                                                                                      */
/*  Author: John Pollard                                                                */
/*  Description: D3D cache manager using pages                                          */
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
#ifndef D3D_TPAGE_H
#define D3D_TPAGE_H

#include <Windows.h>

#include <DDraw.h>
#include <D3D.H>

typedef struct TPage_Mgr	TPage_Mgr;
typedef	struct TPage		TPage;
typedef struct TPage_Block	TPage_Block;

//
// TPage_Mgr
//
TPage_Mgr		*TPage_MgrCreate(LPDIRECTDRAW7 lpDD, const DDSURFACEDESC2 *SurfaceDesc, int32 NumPages);
void			TPage_MgrDestroy(TPage_Mgr **TPageMgr);
jeBoolean		TPage_MgrHasTPage(TPage_Mgr *Mgr, TPage *Page);
jeBoolean		TPage_MgrAttachTPage(TPage_Mgr *Mgr, TPage *TPage);
void			TPage_MgrDetachTPage(TPage_Mgr *Mgr, TPage *TPage);
TPage_Block		*TPage_MgrFindOptimalBlock(TPage_Mgr *Mgr, uint32 LRU);

//
// TPage
//
TPage			*TPage_Create(LPDIRECTDRAW7 lpDD, const DDSURFACEDESC2 *SurfDesc);
void			TPage_CreateRef(TPage *Page);
void			TPage_Destroy(TPage **Page1);
jeBoolean		TPage_HasBlock(TPage *TPage, TPage_Block *Block);
jeBoolean		TPage_AttachBlock(TPage *Page, TPage_Block *Block);
void			TPage_DetachBlock(TPage *TPage, TPage_Block *Block);
jeBoolean		TPage_CreateSurfaces(TPage *Page, LPDIRECTDRAW7 lpDD, const DDSURFACEDESC2 *SurfDesc);
void			TPage_DestroySurfaces(TPage *Page);

//
//TPage_Block
//
TPage_Block		*TPage_BlockCreate(LPDIRECTDRAWSURFACE7 Surface, const RECT *Rect);
jeBoolean		TPage_BlockCreateRef(TPage_Block *Block);
void			TPage_BlockDestroy(TPage_Block **Block);
LPDIRECTDRAWSURFACE7	TPage_BlockGetSurface(TPage_Block *Block);
const RECT		*TPage_BlockGetRect(TPage_Block *Block);
void			TPage_BlockSetLRU(TPage_Block *Block, uint32 LRU);
void			TPage_BlockSetUserData(TPage_Block *Block, void *UserData);
void			*TPage_BlockGetUserData(TPage_Block *Block);

#endif	