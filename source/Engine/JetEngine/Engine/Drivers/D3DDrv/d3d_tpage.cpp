/****************************************************************************************/
/*  TPage.cpp                                                                           */
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
#include <Windows.h>
#include <Assert.h>
#include <Stdio.h>

#include "BaseType.h"
#include "d3d_TPage.h"

// These must be a power of 2!!!
#define TPAGE_WIDTH			16					// Width of TPAges
#define TPAGE_HEIGHT		16					// Height of TPages

#define	TPAGE_GRID_X		16					// Must be <= TPAGE_WIDTH, and power of 2
#define TPAGE_GRID_Y		16					// Must be <= TPAGE_HEIGHT, and power of 2

// NOTE - Blocks Width/Height that go into TPages MUST be <= AlignX/AlignY!!!

typedef struct TPage_Block
{
	int32					RefCount;			// Number of references to this object

	LPDIRECTDRAWSURFACE7	Surface;			// The DD surface for this Block

	RECT					Rect;				// The Rect into the surface that this block can use

	uint32					LRU;				// Set to the TPage->TPageMgr->LRU when accesed...

	void					*UserData;

	struct TPage_Block		*Prev;
	struct TPage_Block		*Next;

} TPage_Block;

typedef struct TPage
{
	int32					RefCount;

	DDSURFACEDESC2			SurfaceDesc;		// Surface description of this page (note all blocks must use this format!!!)
	LPDIRECTDRAWSURFACE7	Surface;			// The DD surface for this TPage

	TPage_Block				*Blocks;			//	Linked list of blocks

	uint32					LRU;

	struct TPage			*Prev;
	struct TPage			*Next;
} TPage;

typedef struct TPage_Mgr
{
	int32					NumPages;

	TPage					*TPages;			// Linked list of TPages

	LPDIRECTDRAW7			lpDD;				// DD object used to create surfaces
} TPage_Mgr;

//============================================================================
//	 *** TPage_Mgr ***
//============================================================================

//============================================================================
//	TPage_MgrCreate
//============================================================================
TPage_Mgr *TPage_MgrCreate(LPDIRECTDRAW7 lpDD, const DDSURFACEDESC2 *SurfaceDesc, int32 NumPages)
{
	TPage_Mgr* TPageMgr{};
	int32		i{};

	TPageMgr = (TPage_Mgr*)malloc(sizeof(TPage_Mgr));

	if (!TPageMgr)
		return nullptr;

	memset(TPageMgr, 0, sizeof(TPage_Mgr));

	// Remeber the DD object
	TPageMgr->lpDD = lpDD;
	// Ref the dd object
	lpDD->AddRef();			

	TPageMgr->NumPages = NumPages;

	// Create the pages
	for (i=0; i<NumPages; i++)
	{
		TPage* Page{};

		Page = TPage_Create(lpDD, SurfaceDesc);

		if (!Page)
			goto ExitWithError;

		if (!TPage_MgrAttachTPage(TPageMgr, Page))
			goto ExitWithError;
	}

	return TPageMgr;

	ExitWithError:
	{
		if (TPageMgr)
			TPage_MgrDestroy(&TPageMgr);
		return nullptr;
	}
}

//============================================================================
//	TPage_MgrDestroy
//============================================================================
void TPage_MgrDestroy(TPage_Mgr **TPageMgr)
{
	TPage_Mgr* Mgr{};
	TPage* Page{}, * Next{};

	assert(TPageMgr);

	Mgr = *TPageMgr;

	assert(Mgr);
	
	// Free the pages
	for (Page = Mgr->TPages; Page; Page = Next)
	{
		Next = Page->Next;

		TPage_MgrDetachTPage(Mgr, Page);
		TPage_Destroy(&Page);
	}

	assert(Mgr->TPages == nullptr);

	// Release our ref on the DD object
	Mgr->lpDD->Release();			

	free(*TPageMgr);

	*TPageMgr = nullptr;
}

//============================================================================
//	TPage_MgrHasTPage
//============================================================================
jeBoolean TPage_MgrHasTPage(TPage_Mgr *Mgr, TPage *Page)
{
	TPage* Page2{};

	assert(Mgr);
	assert(Page);

	for (Page2 = Mgr->TPages; Page2; Page2 = Page2->Next)
	{
		if (Page2 == Page)
		{
			return JE_TRUE;
		}
	}
	
	return JE_FALSE;
}

//============================================================================
//	TPage_MgrAttachTPage
//	NOTE - A TPage can only attach to one TPage_Mgr, and only ONCE!!
//============================================================================
jeBoolean TPage_MgrAttachTPage(TPage_Mgr *Mgr, TPage *TPage)
{
	assert(TPage_MgrHasTPage(Mgr, TPage) == JE_FALSE);
	assert(TPage->Prev == nullptr);
	assert(TPage->Next == nullptr);

	if (Mgr->TPages)
		Mgr->TPages->Prev = TPage;

	TPage->Prev = nullptr;
	TPage->Next = Mgr->TPages;
	Mgr->TPages = TPage;

	return JE_TRUE;
}

//============================================================================
//	TPage_MgrDetachTPage
//============================================================================
void TPage_MgrDetachTPage(TPage_Mgr *Mgr, TPage *TPage)
{
	assert(Mgr);
	assert(TPage);
	assert(TPage_MgrHasTPage(Mgr, TPage) == JE_TRUE);

	if (TPage->Next)
		TPage->Next->Prev = TPage->Prev;

	if (TPage->Prev)
		TPage->Prev->Next = TPage->Prev;
	else
	{
		// If we get here, this better be the first TPage in the list!
		assert(Mgr->TPages == TPage);
		Mgr->TPages = TPage->Next;
	}

	TPage->Next = nullptr;
	TPage->Prev = nullptr;
}

//============================================================================
//	TPage_MgrFindOptimalBlock
//============================================================================
TPage_Block *TPage_MgrFindOptimalBlock(TPage_Mgr *Mgr, uint32 LRU)
{
#if 0
	TPage		*Page, *BestPage;
	TPage_Block	*Block, *BestBlock;
	uint32		BestLRU;

	// We really should make a TPage_GetOptimalBlock...

	// First, find the page that has the highest LRU
	BestLRU = 0;
	BestPage = Mgr->TPages;

	for (Page = Mgr->TPages; Page; Page = Page->Next)
	{
		if (Page->LRU > BestLRU && Page->NumFull)
		{
			BestPage = Page;
			BestLRU = Page->LRU;
		}
	}

	// Now, find the block with the lowest LRU in this page
	BestBlock = BestPage->Blocks;
	BestLRU = 0xffffffff;

	for (Block = BestPage->Blocks; Block; Block = Block->Next)
	{
		if (Block->LRU < BestLRU)
		{
			BestBlock = Block;
			BestLRU = Block->LRU;
		}
	}
	/*
	if (BestBlock->LRU == LRU)
		BestPage->NumFull++;
	else
	*/
		BestBlock->LRU = LRU;
	BestPage->LRU = LRU;
#else
	TPage* Page{};
	TPage_Block* Block{}, * BestBlock{};
	uint32		BestLRU{};

	// We really should make a TPage_GetOptimalBlock...
	BestBlock = nullptr;
	BestLRU = 0xffffffff;

	for (Page = Mgr->TPages; Page; Page = Page->Next)
	{
		for (Block = Page->Blocks; Block; Block = Block->Next)
		{
			if (Block->LRU < BestLRU)
			{
				BestBlock = Block;
				BestLRU = Block->LRU;
			}
		}
	}

	BestBlock->LRU = LRU;
#endif

	return BestBlock;
}

//============================================================================
//	   *** TPage ***
//============================================================================

//============================================================================
//	TPage_Create
//============================================================================
TPage *TPage_Create(LPDIRECTDRAW7 lpDD, const DDSURFACEDESC2 *SurfDesc)
{
	TPage* Page{};
	int32	w{}, h{};

	Page = (TPage*)malloc(sizeof(TPage));

	if (!Page)
		return nullptr;

	memset(Page, 0, sizeof(TPage));

	Page->SurfaceDesc = *SurfDesc;

	TPage_CreateRef(Page);		// Create the very first ref

	if (!TPage_CreateSurfaces(Page, lpDD, SurfDesc))
	{
		free(Page);
		return nullptr;
	}

	// Create the blocks
	for (h=0; h<TPAGE_HEIGHT/16; h++)
	{
		for (w=0; w<TPAGE_WIDTH/16; w++)
		{
			TPage_Block* Block{};
			RECT			Rect{};

			Rect.left = w*TPAGE_GRID_X;
			Rect.right = Rect.left+(TPAGE_GRID_X-1);
			Rect.top = h*TPAGE_GRID_Y;
			Rect.bottom = Rect.top+(TPAGE_GRID_Y-1);

			Block = TPage_BlockCreate(Page->Surface, &Rect);

			if (!Block)
				goto ExitWithError;

			if (!TPage_AttachBlock(Page, Block))
				goto ExitWithError;
		}
	}

	return Page;

	ExitWithError:
	{
		if (Page)
			TPage_Destroy(&Page);

		return nullptr;
	}
}

//============================================================================
//	TPage_CreateRef
//============================================================================
void TPage_CreateRef(TPage *Page)
{
	assert(Page->RefCount >= 0);		// Refs can == 0, because thats what they are when TPages are first created

	Page->RefCount++;
}

//============================================================================
//	TPage_Destroy
//============================================================================
void TPage_Destroy(TPage **Page1)
{
	TPage* Page{};
	TPage_Block* Block{}, * Next{};

	assert(Page1);
	Page = *Page1;

	assert(Page);
	assert(Page->RefCount > 0);

	Page->RefCount--;

	if (Page->RefCount > 0)
		return;

	// Destroy any dd surfaces for this page
	TPage_DestroySurfaces(Page);

	// Destroy all the blocks this page has
	for (Block = Page->Blocks; Block; Block = Next)
	{
		Next = Block->Next;

		TPage_DetachBlock(Page, Block);
		TPage_BlockDestroy(&Block);
	}

	assert(Page->Blocks == nullptr);

	free(*Page1);

	*Page1 = nullptr;
}

//============================================================================
//	TPage_HasBlock
//============================================================================
jeBoolean TPage_HasBlock(TPage *TPage, TPage_Block *Block)
{
	TPage_Block* Block2{};

	assert(TPage);
	assert(Block);

	for (Block2 = TPage->Blocks; Block2; Block2 = Block2->Next)
	{
		if (Block2 == Block)
		{
			return JE_TRUE;
		}
	}

	return JE_FALSE;
}

//============================================================================
//	TPage_AttachBlock
//	NOTE - A Block can only attach to one TPage, and only ONCE!!
//============================================================================
jeBoolean TPage_AttachBlock(TPage *Page, TPage_Block *Block)
{
	assert(TPage_HasBlock(Page, Block) == JE_FALSE);
	assert(Block->Prev == nullptr);
	assert(Block->Next == nullptr);

	// Insert the block into the list of blocks for this Page
	if (Page->Blocks)
		Page->Blocks->Prev = Block;

	Block->Prev = nullptr;
	Block->Next = Page->Blocks;

	Page->Blocks = Block;

	return JE_TRUE;
}

//============================================================================
//	TPage_DetachBlock
//============================================================================
void TPage_DetachBlock(TPage *TPage, TPage_Block *Block)
{
	assert(TPage);
	assert(Block);
	assert(TPage_HasBlock(TPage, Block) == JE_TRUE);

	if (Block->Next)
		Block->Next->Prev = Block->Prev;

	if (Block->Prev)
		Block->Prev->Next = Block->Prev;
	else
	{
		// If we get here, this better be the first Block in the list!
		assert(TPage->Blocks == Block);
		TPage->Blocks = Block->Next;
	}

	// Reset the Block link
	Block->Next = nullptr;
	Block->Prev = nullptr;
}

//=====================================================================================
//	TPage_CreateSurfaces
//=====================================================================================
jeBoolean TPage_CreateSurfaces(TPage *Page, LPDIRECTDRAW7 lpDD, const DDSURFACEDESC2 *SurfDesc)
{
	HRESULT					Hr{};
	DDSURFACEDESC2			ddsd{};

	assert(Page);

	memcpy(&ddsd, SurfDesc, sizeof(DDSURFACEDESC2));

	ddsd.dwSize = sizeof(DDSURFACEDESC2);
	ddsd.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT;

	ddsd.ddsCaps.dwCaps = DDSCAPS_TEXTURE;
	ddsd.ddsCaps.dwCaps2 = DDSCAPS2_TEXTUREMANAGE | DDSCAPS2_HINTDYNAMIC;

	ddsd.ddsCaps.dwCaps3 = 0;
	ddsd.ddsCaps.dwCaps4 = 0;

	ddsd.dwWidth = TPAGE_WIDTH;
	ddsd.dwHeight = TPAGE_HEIGHT;

	Hr = lpDD->CreateSurface(&ddsd, &Page->Surface, nullptr);

	if (Hr != DD_OK) 
		return JE_FALSE;

	return JE_TRUE;		// All good dude
}


//=====================================================================================
//	TPage_DestroySurfaces
//=====================================================================================
void TPage_DestroySurfaces(TPage *Page)
{
	assert(Page);

	if (Page->Surface)
	{
		Page->Surface->Release();
		Page->Surface = nullptr;
	}
}

//============================================================================
//	*** TPage_Block ***
//============================================================================

//============================================================================
//	TPage_BlockCreate
//============================================================================
TPage_Block *TPage_BlockCreate(LPDIRECTDRAWSURFACE7 Surface, const RECT *Rect)
{
	TPage_Block* Block{};

	Block = (TPage_Block*)malloc(sizeof(TPage_Block));

	if (!Block)
		return nullptr;

	memset(Block, 0, sizeof(TPage_Block));

	Surface->AddRef();		// Ref the surface

	// Save off the surface, texture, and rect into the surface
	Block->Surface = Surface;
	Block->Rect = *Rect;

	TPage_BlockCreateRef(Block);		// Create very first ref

	return Block;
}

//============================================================================
//	TPage_BlockCreateRef
//============================================================================
jeBoolean TPage_BlockCreateRef(TPage_Block *Block)
{
	assert(Block);

	Block->RefCount++;

	return JE_TRUE;
}

//============================================================================
//	TPage_BlockDestroy
//============================================================================
void TPage_BlockDestroy(TPage_Block **Block)
{
	TPage_Block* Block2{};

	assert(Block);
	
	Block2 = *Block;

	assert(Block2);
	assert(Block2->RefCount > 0);

	Block2->RefCount--;

	if (Block2->RefCount > 0)
		return;

	// Destroy references to the surface 
	if (Block2->Surface)
		Block2->Surface->Release();

	// Free the block
	free(Block2);

	*Block = nullptr;
}

//============================================================================
//	TPage_BlockGetSurface
//============================================================================
LPDIRECTDRAWSURFACE7 TPage_BlockGetSurface(TPage_Block *Block)
{
	assert(Block);

	return Block->Surface;
}

//============================================================================
//	TPage_BlockGetRect
//============================================================================
const RECT *TPage_BlockGetRect(TPage_Block *Block)
{
	assert(Block);

	return &Block->Rect;
}

//============================================================================
//	TPage_BlockSetLRU
//============================================================================
void TPage_BlockSetLRU(TPage_Block *Block, uint32 LRU)
{
	assert(Block);

	Block->LRU = LRU;
}

//============================================================================
//	TPage_BlockSetUserData
//============================================================================
void TPage_BlockSetUserData(TPage_Block *Block, void *UserData)
{
	assert(Block);

	Block->UserData = UserData;
}

//============================================================================
//	TPage_BlockGetUserData
//============================================================================
void *TPage_BlockGetUserData(TPage_Block *Block)
{
	assert(Block);

	return Block->UserData;
}
