/****************************************************************************************/
/*  D3DCACHE.CPP                                                                        */
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

// see @@ and <> for todos

#define DO_LOG

/*******

Notes on D3DCache by Charles Bloom

call _Update() at the end of a scene; it gathers stats and re-balances the cache
	for the current vis set
_Balance() tries to move memory from one type to another to better reflect the
	textures that are visible

The MipData in the Slot is the THandle it corresponds to.
The RefCount in the Type is the number of THandles of that type	
We gather the usage stats from the THandles build in SetupTexture() in PCache

NOTE : we currently maintain separate caches for texture and lightmaps !
	This would all work better if we merged the two caches
	** see USE_ONE_CACHE flag in THandle.cpp ; try it!

The DDMemMgr is just an accountant for adds and subtracts.
	We query the free memory in THandle : D3DDrvInfo.VidMemFree
	Originally set up in Main_GetVideoMemory
<> NOTE : this is not accurate !! The actual amount of vidmem used may not
	be the same as the width*height*bpp !!!
 this isn't terrible, because in the end, we actually judge free mem by
	trying to allocate.  

-----------------------------------------------------

<> Misses due to max-ing the # of slots should not cause a bias adjust!
	done : just using Diff2, no longer Diff1

<> when you have a huge number of textures on screen that are continuously missing,
then we are far from optimal : eg. you want to draw A,B,C and only have room for two;
we do : 
	up&draw A,B
	kick A ; up&draw C
	kick B ; up&draw A
	kick C ; up&draw B , etc...
eg. with a pure LRU we are always uploading, when really one of the three could stay up
all the time.  The traditional solution to this in caching is to use several "ways".  That's
a really goofy solution.  One solution in rendering is to reverse the render order each
frame : A,B,C ; C,B,A ; A,B,C ... then the cache looks like :
	up&draw A,B
	kick A ; up&draw C
	draw C,B
	kick C, up&draw A
	draw A,B
	kick A ; up&draw C
-> I'm now doing this reversing in pcache; unfortunately, partial flushes hurt this

<> There's some contention between balancing and mip biasing.  When you're looking back and
forth the balance keeps changing, so the bias will never do its work.

<> Note that we only crank Mip Bias when the total texture memory overflows the total card
space.  There can be overflows that are not due to full texture overflow. Examples :
	if the memory is fragmented, and we cannot allocate the texture type we need in the
		remaining memory

<> There's a sort of hysterisys that can happen with the mip bias : you overflow the card,
	so you crank up the bias.  Now, with the new bias, you suddenly underflow, so you kick
	the mip bias back down.  Our makes this happen once per five frames or so, instead of
	every single frame.  There's no real way to avoid this, because we can't tell how much
	memory mipping-down really saves (unless we check every rendered poly)

<> note : Mip Bias changes cause loads of "fresh" mip misses
	did : might be good to quantize the mip biases to some buckets at the last minute;
		1.0 , 1.25 , 1.5 , or something

<> tiny texture misses (eg. lightmaps) need to be handled more intelligently -
	eg. they shouldn't affect the mip bias, and they should not be so greedy for textures;
		ie. letting them miss is not so bad, and creating surfaces for them may be costly
		we need logic like this :
			1. if the card is near full, just don't bother trying to make new small slots
			2. make small slots slowly - eg. if the demand is 20 slots, make 2 per frame
				the idea is so that if you suddenly see 100 new textures, you don't make loads of slots,
					just let them miss, unless they stay on screen for a while
				(this may be automatically handled by BALANCES_PER_FRAME and OVERFLOW_MAX)
	on second thought, this may be all automagic; just need to think about it a little

********/

#include <Windows.h>
#include <Stdio.h>
#include <Assert.h>
#include <DDraw.h>
#include <D3D.h>

#include "D3DCache.h"
#include "d3d_main.h" //for GetLog
#include "d3d_thandle.h"

#include "mempool.h"

extern uint32	Scene_CurrentFrame;

// tweakable config for the cache balancing :
#define MAX_BALANCES_PER_FRAME	(12)
#define HISTORY_SIZE			(8)
#define BIAS_HISTORY_SIZE		(10)
#define BALANCE_FRAME_DELAY		(2)
#define CACHE_LINGER_FRAMES		(8)		// newly added slot won't be removed for this long
#define OVERFLOW_MAX			(8)
#define MIPBIAS_MAX				(5.0f)

// Cache types are just the different types of textures
// Textures may vary from width/height/num miplevels/etc...
// Cache types is just a way to combine them to similar types...
#define	D3DCACHE_MAX_CACHE_TYPES		(64)		
#define	D3DCACHE_MAX_CACHE_SLOTS		(1024)	// per type!

//========================================================================================================
//========================================================================================================

typedef struct D3DCache_Slot
{
	jeBoolean				IsActive;					// indicates whether it is used & set up

	struct D3DCache_Slot	*SelfCheck;

	D3DCache_Type			*CacheType;

	LPDIRECTDRAWSURFACE7	Surface;					// The DD surface for this slot

	uint32					LRU;						// Current LRU for cache slot
						
	THandle_MipData			*MipData;					// whose data do I have?		
} D3DCache_Slot;

typedef struct D3DCache_Type
{
	int32					Log;
	int32					Width;						// Width/Height
	int32					Height;
	int32					NumMipLevels;
	int32					Stage;
	int32					RefCount;					// How many references to this cache_type
														// {} when refcount is zero, nothing is set up
														// RefCount is the number of thandles of this type
	int32					Size;						// in bytes

	DDSURFACEDESC2			ddsd;						// DD surface description

	D3DCache_Slot *			SlotPtrs[D3DCACHE_MAX_CACHE_SLOTS];		// Cache slots for this Cache type
	int32					NumSlots;					// number of slots allocated

	D3DCache_Type			*SelfCheck;
	D3DCache				*Cache;

	int32					Misses,FreshMisses,Uses,AddedCountdown;
	int32					OverflowMax,OverflowHistory[HISTORY_SIZE],OverflowHistoryI;
	//int32					MissesMax,FreshMissesMax,UsesMax;
	int32					SlotNeedRating;
} D3DCache_Type;

typedef struct D3DCache
{
	struct D3DCache			*SelfCheck;

	char					Name[D3DCACHE_MAX_NAME];

	LPDIRECTDRAW7			lpDD;						// DD object for the cache manager

	DDMemMgr_Partition		*Partition;					// just counts off a pre-queried FreeMem variable

	jeBoolean				UseStages;

	D3DCache_Type			CacheTypes[D3DCACHE_MAX_CACHE_TYPES];	// CacheTypes

	int32					SoughtMem,AvailMem,CardMem;

	float					StoreMipBias[BIAS_HISTORY_SIZE];
	int						StoreMipBiasI;

	MemPool					*SlotPool;

	jeBoolean				DoMipBias;

	int32					TotNumSlots;

} D3DCache;

#ifdef DO_LOG
typedef struct D3DCacheLogType
{
	int frames;
	double updateTime;
	int slotsCreated,slotsDestroyed;
	int maxCacheTypes;

	int	TexMissesTot;
	int	LMapMissesTot;
	int	TexMissBytesTot;
	int	LMapMissBytesTot;

	int	CardMemTot,SlotMemTot,UsedMemTot;
	float MipBiasTot,MipBiasMax;
} D3DCacheLogType;

static D3DCacheLogType D3DCacheLog;
#endif


//========================================================================================================
//========================================================================================================

void	D3DCache_InitStaticsAndGlobals(void)
{
}

int32		D3DCache_SetupSlot(D3DCache *Cache, D3DCache_Slot *Slot, int32 Width, int32 Height, const DDSURFACEDESC2 *SurfDesc, jeBoolean UseStage, int32 Stage);
jeBoolean	D3DCache_FreeAllSlots(D3DCache *Cache);
void D3DCache_Balance(D3DCache *Cache);
jeBoolean D3DCache_TypeAddSlot(D3DCache_Type *pCacheType);
jeBoolean D3DCache_RemoveOldestSlot(D3DCache *Cache,D3DCache_Type *NotType,jeBoolean Critical);
void D3DCacheLog_WriteToFile(void);

//========================================================================================================
//	D3DCache_Create
//========================================================================================================
D3DCache *D3DCache_Create(const char *Name, LPDIRECTDRAW7 lpDD, DDMemMgr_Partition *Partition,jeBoolean UseStages)
{
	D3DCache* Cache{};

	assert(strlen(Name) < D3DCACHE_MAX_NAME);

	Cache = (D3DCache*)malloc(sizeof(D3DCache));

	if (!Cache)
		return nullptr;

	memset(Cache, 0, sizeof(D3DCache));

	Cache->SlotPool = MemPool_Create(sizeof(D3DCache_Slot),1024,1024);

	if ( ! Cache->SlotPool )
	{
		free(Cache);
		return nullptr;
	}

	Cache->lpDD = lpDD;

	Cache->Partition = Partition;

	Cache->UseStages = UseStages;

	Cache->SelfCheck = Cache;

	Cache->CardMem = DDMemMgr_PartitionGetTotalMem(Cache->Partition);

	strcpy_s(Cache->Name, Name);

	for(int i=0;i<BIAS_HISTORY_SIZE;i++)
	{
		Cache->StoreMipBias[i] = 1.0f;
	}

	Cache->DoMipBias = JE_TRUE;

return Cache;
}

//========================================================================================================
//	D3DCache_Destroy
//========================================================================================================
void D3DCache_Destroy(D3DCache *Cache)
{
	assert(Cache);

	D3DCache_FreeAllSlots(Cache);

	MemPool_Destroy(&(Cache->SlotPool));

	free(Cache);

#ifdef DO_LOG
	D3DCacheLog_WriteToFile();
#endif

}

//========================================================================================================
//	D3DCache_IsValid
//========================================================================================================
jeBoolean D3DCache_IsValid(D3DCache *Cache)
{
	if (!Cache)
		return JE_FALSE;

	if (Cache->SelfCheck != Cache)
		return JE_FALSE;

	return JE_TRUE;
}

//========================================================================================================
//	D3DCache_SeriousSelfCheck
//========================================================================================================
void D3DCache_SeriousSelfCheck(D3DCache *Cache)
{
	int	i{};
	D3DCache_Type* pCacheType{};
	
	assert(D3DCache_IsValid(Cache));


	int PartTotMem	= DDMemMgr_PartitionGetTotalMem(Cache->Partition);
	int PartFreeMem = DDMemMgr_PartitionGetFreeMem(Cache->Partition);
	int MemUsed = 0;

	for (pCacheType = Cache->CacheTypes, i=0; i< D3DCACHE_MAX_CACHE_TYPES; i++, pCacheType++)
	{
		if ( ! pCacheType->RefCount && ! pCacheType->NumSlots ) // nothing here
			continue;

		assert( D3DCache_TypeIsValid(pCacheType) );

		//assert( pCacheType->NumSlots >= 1 ); // NumSlots can be zero now
		//assert( pCacheType->NumSlots <= pCacheType->RefCount ); // may have destroyed texes & left slots
		assert( pCacheType->Cache == Cache );

		MemUsed += pCacheType->NumSlots * pCacheType->Size;
		
		int	k{};
		int ActiveSlots = 0;
		// count the number of slots with IsActive
		for (k=0; k< D3DCACHE_MAX_CACHE_SLOTS; k++)
		{
			D3DCache_Slot* pSlot{};

			pSlot = pCacheType->SlotPtrs[k];

			if ( k < pCacheType->NumSlots )
			{
				assert(pSlot);
				assert(pSlot->IsActive);
				assert( D3DCache_SlotIsValid(pSlot) );
				assert( pSlot->CacheType = pCacheType );
				ActiveSlots ++;
			}
			else
			{
				assert(pSlot == nullptr);
			}

		}

		assert( pCacheType->NumSlots == ActiveSlots );
	}

	assert( MemUsed == (PartTotMem - PartFreeMem) );
}

//========================================================================================================
//	D3DCache_Update
//========================================================================================================

extern float			PCache_MipBias;
extern DRV_CacheInfo	PCache_CacheInfo;

void D3DCache_TypeUsed(D3DCache_Type *CacheType)
{
	CacheType->Uses ++;
}

#ifdef DO_LOG
#include "tsc.h"
#endif

void D3DCache_Update(D3DCache *Cache)
{
	int32			i{};
	D3DCache_Type* pCacheType{};
	int32			SlotMemNonEssential{}, SlotMemEssential{}, SoughtMemExcess{};

#ifdef DO_LOG
	tsc_type tsc1{}, tsc2{};
#endif

	assert(D3DCache_IsValid(Cache));

	Cache->SoughtMem = 0;
	SlotMemNonEssential = SlotMemEssential = SoughtMemExcess = 0;

#ifdef _DEBUG
	D3DCache_SeriousSelfCheck(Cache);
#endif

#ifdef DO_LOG
	readTSC(tsc1);
#endif

	for (pCacheType = Cache->CacheTypes, i=0; i< D3DCACHE_MAX_CACHE_TYPES; i++, pCacheType++)
	{
		pCacheType->Uses = 0;
	}

	THandle_GetCacheTypeUses();

	for (pCacheType = Cache->CacheTypes, i=0; i< D3DCACHE_MAX_CACHE_TYPES; i++, pCacheType++)
	{
		if ( ! pCacheType->RefCount )
		{
			assert( pCacheType->Uses == 0 );
			SlotMemNonEssential += pCacheType->NumSlots * pCacheType->Size;
			continue;
		}
					
		if ( pCacheType->Uses > 0 )
		{
			Cache->SoughtMem += pCacheType->Uses * pCacheType->Size;
			SoughtMemExcess += (pCacheType->Uses - 1) * pCacheType->Size;
		}

		if ( pCacheType->NumSlots > 0 )
		{
			SlotMemNonEssential += (pCacheType->NumSlots - 1) * pCacheType->Size;
			SlotMemEssential	+= pCacheType->Size;
		}

		//pCacheType->UsesMax = (pCacheType->UsesMax + pCacheType->Uses)*0.5f;
		//pCacheType->MissesMax = (pCacheType->MissesMax + pCacheType->Misses)*0.5f;
		//pCacheType->FreshMissesMax = (pCacheType->FreshMissesMax + pCacheType->FreshMisses)*0.5f;
		pCacheType->OverflowMax = 0;
		pCacheType->OverflowHistory[pCacheType->OverflowHistoryI] = pCacheType->Uses - pCacheType->NumSlots;
		pCacheType->OverflowHistoryI = (pCacheType->OverflowHistoryI + 1)%HISTORY_SIZE;
		for(int j=0;j<HISTORY_SIZE;j++)
		{
			pCacheType->OverflowMax = max(pCacheType->OverflowMax,pCacheType->OverflowHistory[j]);
		}
	}
	
	//	we've not got Misses,Uses, and UsesMax on all cache types
	D3DCache_Balance(Cache);
		
	PCache_CacheInfo.CacheTypes = 0;
	for (pCacheType = Cache->CacheTypes, i=0; i< D3DCACHE_MAX_CACHE_TYPES; i++, pCacheType++)
	{
		if ( ! pCacheType->RefCount )
			continue;

		assert( pCacheType->FreshMisses <= pCacheType->Misses );

		PCache_CacheInfo.CacheMisses[PCache_CacheInfo.CacheTypes]	= pCacheType->Misses;
		PCache_CacheInfo.CacheFreshMisses[PCache_CacheInfo.CacheTypes]	= pCacheType->FreshMisses;
		PCache_CacheInfo.CacheUses[PCache_CacheInfo.CacheTypes]		= pCacheType->Uses;
		PCache_CacheInfo.CacheSlots[PCache_CacheInfo.CacheTypes]	= pCacheType->NumSlots;
		PCache_CacheInfo.CacheTypes++;

		// reset the count
		pCacheType->Misses = 0;
		pCacheType->FreshMisses = 0;

		if ( pCacheType->AddedCountdown == CACHE_LINGER_FRAMES ) // <> just balanced
		{
			// clear the history!
			for(int j=0;j<HISTORY_SIZE;j++)
				pCacheType->OverflowHistory[j] = 0;
		}

		if ( pCacheType->AddedCountdown > 0 )
			pCacheType->AddedCountdown --;
	}

	PCache_CacheInfo.CardMem = Cache->CardMem;
	PCache_CacheInfo.SlotMem = Cache->CardMem - DDMemMgr_PartitionGetFreeMem(Cache->Partition);
	PCache_CacheInfo.UsedMem = Cache->SoughtMem;

	Cache->AvailMem = SlotMemNonEssential;

	if ( Cache->DoMipBias )
	{
	float MipBias;
		MipBias = PCache_MipBias;
		{
		int Diff2;
//			Diff1  = SoughtMemExcess  - SlotMemNonEssential - 111111;
			Diff2 = Cache->SoughtMem - Cache->CardMem;
			// Diff2 doesn't account for headers & other complications,
			//	but that's probably a good thing
			if ( Diff2 > 0 )
			{
				// we overflowed the card
				MipBias += 0.33f;
			}
			else if ( Diff2 < -555555 )
			{
				// we underloaded
				/*
				if ( MipBias > 1.0f )
				{
					MipBias -= 0.1f;
					MipBias = max(MipBias,1.0f);
				}
				*/
				// just snap it back to 1.0f
				MipBias = 1.0f;
			}
		}

		Cache->StoreMipBias[Cache->StoreMipBiasI] = MipBias;
		Cache->StoreMipBiasI = (Cache->StoreMipBiasI + 1)%BIAS_HISTORY_SIZE;

	#if 1 // average of some biases
		PCache_MipBias = 0.0f;
		for(int i=0;i<BIAS_HISTORY_SIZE;i++)
		{
			PCache_MipBias += Cache->StoreMipBias[i];
		}
		PCache_MipBias *= (1.0f/BIAS_HISTORY_SIZE);
	#else // max
		PCache_MipBias = 1.0f;
		for(int i=0;i<BIAS_HISTORY_SIZE;i++)
		{
			PCache_MipBias = max(PCache_MipBias,Cache->StoreMipBias[i]);
		}
	#endif 

	#if 1 // <> quantize it to multiples of 0.25
		{
			int32 q{};
			q = (int32)(PCache_MipBias * 4 + 0.5f);
			PCache_MipBias = q * 0.25f;
		}
	#endif

		if ( PCache_MipBias > MIPBIAS_MAX )
			PCache_MipBias = MIPBIAS_MAX;
	}
	else
	{
		PCache_MipBias = 1.0f;
	}

	PCache_CacheInfo.MipBias = PCache_MipBias;
	
#ifdef DO_LOG
	readTSC(tsc2);

	D3DCacheLog.frames ++;
	D3DCacheLog.updateTime += diffTSC(tsc1,tsc2);

	D3DCacheLog.CardMemTot  += Cache->CardMem;
	D3DCacheLog.SlotMemTot  += PCache_CacheInfo.SlotMem;
	D3DCacheLog.UsedMemTot  += Cache->SoughtMem;

	D3DCacheLog.MipBiasTot += PCache_MipBias;
	D3DCacheLog.MipBiasMax = max(D3DCacheLog.MipBiasMax,PCache_MipBias);

	D3DCacheLog.TexMissesTot += PCache_CacheInfo.TexMisses;
	D3DCacheLog.TexMissBytesTot += PCache_CacheInfo.TexMissBytes;
	D3DCacheLog.LMapMissesTot += PCache_CacheInfo.LMapMisses;
	D3DCacheLog.LMapMissBytesTot += PCache_CacheInfo.LMapMissBytes;
#endif
}

void D3DCache_Balance(D3DCache *Cache)
{
	int32			i{}, ChoseMissSizeMax{}, FailedSize{}, Creations{};
	D3DCache_Type* pCacheType{}, * ChoseCacheType{};

	assert(D3DCache_IsValid(Cache));

	/***

	doing only 1 per frame means we never take a huge hit,
		but it means we take a really long time to balance when a bunch of new 
		low-mip stuff comes into view in the distance

	it seems we can do alot without taking a framerate hit ! I put
	MAX_BALANCES_PER_FRAME == 32 and it's fast!!

	***/

	PCache_CacheInfo.Balances = PCache_CacheInfo.BalancesFailed = 0;

	FailedSize = 999999999;

	for(Creations=0;Creations < MAX_BALANCES_PER_FRAME;Creations++)
	{
		// look the highest (Size * OverflowMax)

		TryAgain:

		/***

		We'd like to take account of the misses too, but misses that don't result from overflow are
			very hard to account for properly.  There are two types :

			1. when you first walk into a room, you get misses until the new textures get on the card

			2. if there are two places, each with too many textures, you'll get misses every time you
				go between them.

			Perhaps adjust for misses if there are no overflows ?

			<> use the FreshMisses !

			<> the FreshMisses aren't behaving quite as they should
				(aside from not working, they also seem to whack out the normal balances)

		***/

		ChoseMissSizeMax = 0;

		// choose a type to try to extend

		int AvailSlots = D3DInfo.MaxSurfaceCount - Cache->TotNumSlots + 1;
		
		AvailSlots = min(AvailSlots,OVERFLOW_MAX);

		for (pCacheType = Cache->CacheTypes, i=0; i< D3DCACHE_MAX_CACHE_TYPES; i++, pCacheType++)
		{
			int32 MissSizeMax{};
			int32 overflow{};

			pCacheType->SlotNeedRating = 0;

			if ( ! pCacheType->RefCount )
				continue;
			if ( pCacheType->NumSlots == D3DCACHE_MAX_CACHE_SLOTS )
				continue; // can't add any here

			// overflow is a much higher priority than fresh misses
			/*
			if ( pCacheType->OverflowMax > 0.1f )
				MissSizeMax = (int32)(pCacheType->Size * pCacheType->OverflowMax * 256.0f);
			else if ( pCacheType->FreshMissesMax > 0.1f )
				MissSizeMax = (int32)(pCacheType->Size * pCacheType->FreshMissesMax);
			*/

			overflow = pCacheType->OverflowMax;
			overflow = min(overflow,AvailSlots);

			MissSizeMax = (int32)(pCacheType->Size * overflow);

			pCacheType->SlotNeedRating = MissSizeMax;

			if ( pCacheType->Size >= FailedSize )
				continue; // rate me but don't add to me (already tried)

			if ( MissSizeMax > ChoseMissSizeMax )
			{
				ChoseMissSizeMax	= MissSizeMax;
				ChoseCacheType		= pCacheType;
			}
		}

		if ( ! ChoseMissSizeMax )
			return; // found nothing to do

		// found something that missed

		// try to add a slot to this type 

		while ( ! D3DCache_TypeAddSlot(ChoseCacheType) )
		{
			if ( ! D3DCache_RemoveOldestSlot(Cache,ChoseCacheType,JE_FALSE) )
			{
				PCache_CacheInfo.BalancesFailed ++;

				// we might have been unable to find space for a 256x256 ,
				//	but there might be a smaller type that also missed !
				//	go back and restart with a smaller max size 
				FailedSize = ChoseCacheType->Size;

				goto TryAgain;
			}
		}

		PCache_CacheInfo.Balances ++;

		// we did it!
		// clear the history of failure
		ChoseCacheType->AddedCountdown	= CACHE_LINGER_FRAMES; // can't remove from here for this many frames
		ChoseCacheType->OverflowMax = max(ChoseCacheType->OverflowMax - BALANCE_FRAME_DELAY,0);
			// <> if we subtract one here, then we could build slots for all the overflows in one frame
			//		if we subtract N, we spread the rebuild over N frames

		// loop around and do another
	}
}

//========================================================================================================
//========================================================================================================
jeBoolean D3DCache_EvictAllSurfaces(D3DCache *Cache) // called by THandle_EvictAll, from Main_RestoreAll
{
	int32			i{};
	D3DCache_Type* pCacheType{};

	assert(D3DCache_IsValid(Cache));

	for (pCacheType = Cache->CacheTypes, i=0; i< D3DCACHE_MAX_CACHE_TYPES; i++, pCacheType++)
	{
		D3DCache_Slot* pSlot{};
		int32				s{};

		assert(D3DCache_TypeIsValid(pCacheType));

		// just forces a re-upload, doesn't free any memory

		for (s=0; s< pCacheType->NumSlots; s++)
		{
			pSlot = pCacheType->SlotPtrs[s];
			pSlot->MipData = nullptr;
		}
	}
	
	return JE_TRUE;
}

//========================================================================================================
//========================================================================================================
static D3DCache_Type *D3DCache_FindCacheType(D3DCache *Cache, int32 Width, int32 Height, int32 NumMipLevels, int32 Stage, const DDSURFACEDESC2 *ddsd)
{
	int32			i{};
	D3DCache_Type* pCacheType{};

	assert(D3DCache_IsValid(Cache));

	// {} never called externally

	pCacheType = Cache->CacheTypes;

	for (i=0; i<D3DCACHE_MAX_CACHE_TYPES; i++, pCacheType++)
	{
		if (pCacheType->RefCount == 0 && pCacheType->NumSlots == 0) // this slot contains no type
			continue;

		assert(D3DCache_TypeIsValid(pCacheType));

		if (pCacheType->Width != Width)
			continue;
		if (pCacheType->Height != Height)
			continue;

		if (pCacheType->NumMipLevels != NumMipLevels)
			continue;

		if (pCacheType->Stage != Stage)
			continue;

		if (memcmp(&pCacheType->ddsd, ddsd, sizeof(DDSURFACEDESC2)))
			continue;
		
		return pCacheType;						// Found a match
	}

	return nullptr;								// Cache Type not found!!!
}

//========================================================================================================
//========================================================================================================
static D3DCache_Type *D3DCache_InsertCacheType(D3DCache *Cache, int32 Width, int32 Height, int32 NumMipLevels, int32 Stage, const DDSURFACEDESC2 *ddsd)
{
	int32			i{};
	D3DCache_Type* pCacheType{};

	assert(D3DCache_IsValid(Cache));

	// {} never called externally

	pCacheType = Cache->CacheTypes;

	for (i=0; i<D3DCACHE_MAX_CACHE_TYPES; i++, pCacheType++)
	{
		if ( pCacheType->RefCount == 0 && pCacheType->NumSlots == 0 ) // Nobody is using this slot yet
			break;	// found a vacancy
	}

#ifdef DO_LOG
	D3DCacheLog.maxCacheTypes = max(D3DCacheLog.maxCacheTypes,i);
#endif

	if (i == D3DCACHE_MAX_CACHE_TYPES)			// No types left
		return nullptr;

	memset(pCacheType,0,sizeof(*pCacheType)); // {} redundant ?

	assert( pCacheType->RefCount == 0 && pCacheType->NumSlots == 0 );

	pCacheType->Width = Width;
	pCacheType->Height = Height;
	pCacheType->NumMipLevels = NumMipLevels;
	pCacheType->Stage = Stage;

	pCacheType->ddsd = *ddsd;

	pCacheType->SelfCheck = pCacheType;
	pCacheType->Cache = Cache;

	pCacheType->Log = GetLog(Width, Height);

	// {} this Size assumes W & H are pow2's ! (and square?)
	pCacheType->Size = Width*Height*(pCacheType->ddsd.ddpfPixelFormat.dwRGBBitCount>>3);

	// <> doesn't account for NumMipLevels !?  (Neither did the old stuff)

	// Found one
	pCacheType->RefCount++;

	return pCacheType;
}

//========================================================================================================
//========================================================================================================
D3DCache_Type *D3DCache_TypeCreate(D3DCache *Cache, int32 Width, int32 Height, int32 NumMipLevels, int32 Stage, const DDSURFACEDESC2 *ddsd)
{
	D3DCache_Type* CacheType{};

	assert(D3DCache_IsValid(Cache));

	// {} called whenever a new THandle is made

	CacheType = D3DCache_FindCacheType(Cache, Width, Height, NumMipLevels, Stage, ddsd);

	if (CacheType)
	{
		// Increase the ref count on this slot, and return it
		CacheType->RefCount++;
		return CacheType;
	}

	// Could not find one allready in the list, so add a new one...
	return D3DCache_InsertCacheType(Cache, Width, Height, NumMipLevels, Stage, ddsd);
}

//========================================================================================================
//	D3DCache_FreeSlot
//========================================================================================================
void D3DCache_FreeSlot(D3DCache_Slot	*pSlot)
{
	D3DCache* Cache{};

	assert( pSlot );

	Cache = pSlot->CacheType->Cache;

	if ( pSlot->IsActive )
	{
		assert(D3DCache_SlotIsValid(pSlot));
		assert(pSlot->Surface);

		if (pSlot->Surface)
			pSlot->Surface->Release();

		if ( pSlot->MipData && pSlot->MipData->Slot == pSlot )
		{
			pSlot->MipData->KickedFrame = Scene_CurrentFrame;
			pSlot->MipData->Slot = nullptr;
		}

		assert( pSlot->CacheType->NumSlots >= 1 );
		pSlot->CacheType->NumSlots --;

		assert( Cache->TotNumSlots >= 1 );
		Cache->TotNumSlots --;

		DDMemMgr_PartitionFreeMem(pSlot->CacheType->Cache->Partition, pSlot->CacheType->Size);

	#ifdef DO_LOG
		D3DCacheLog.slotsDestroyed ++;
	#endif
	}

	memset(pSlot,0,sizeof(*pSlot));

	pSlot->SelfCheck = pSlot;

	MemPool_FreeHunk(Cache->SlotPool,pSlot);
}

//========================================================================================================
//	D3DCache_TypeDestroy
//========================================================================================================
void D3DCache_TypeDestroy(D3DCache_Type *CacheType)
{
	assert(CacheType->RefCount > 0);
	assert(D3DCache_TypeIsValid(CacheType));

	CacheType->RefCount--;

	if (CacheType->RefCount > 0)
		return;

	// <> there should be no textures pointing at this Type anymore

	#if 0 // {

	// don't free now, just let the balance free them later
	//
	// henever you rebuild the bsp, it destroys all lightmaps then remakes them
	//	we'd like to not destroy the type, just let it hang around,

	int32			k,ns;

	OutputDebugString("D3DCache_TypeDestroy : no textures of this type left, freeing slots\n");

	ns = CacheType->NumSlots;
	// Go through each slot, and free all the surfaces on them
	for (k=0; k< ns; k++)
	{
		D3DCache_FreeSlot(CacheType->SlotPtrs[k]);
		CacheType->SlotPtrs[k] = nullptr;
	}

	CacheType->NumSlots = 0;

	#endif //}
}

//========================================================================================================
//	D3DCache_TypeIsValid
//========================================================================================================
jeBoolean D3DCache_TypeIsValid(D3DCache_Type *Type)
{
	if (!Type)
		return JE_FALSE;

	// <> cheat!
	if ( Type->SelfCheck == nullptr )
		Type->SelfCheck = Type;

	if (Type->SelfCheck != Type)
		return JE_FALSE;

	if (!D3DCache_IsValid(Type->Cache))
		return JE_FALSE;

	return JE_TRUE;
}
//========================================================================================================
// D3DCache_RemoveOldestSlot
//========================================================================================================
jeBoolean D3DCache_RemoveOldestSlot(D3DCache *Cache,D3DCache_Type *NotType,jeBoolean Critical)
{
	int32			i{}, OldestIndex{};
	D3DCache_Type* pCacheType{}, * OldestType{};
	D3DCache_Slot* OldestSlot{};

	assert(D3DCache_IsValid(Cache));

	OldestSlot	= nullptr;

	// try to find a nice one to remove

	for (pCacheType	= Cache->CacheTypes, i=0; i< D3DCACHE_MAX_CACHE_TYPES; i++, pCacheType++)
	{
		D3DCache_Slot* pSlot{};
		int32			k{};

		if ( pCacheType == NotType )
			continue;

		if ( pCacheType->NumSlots == 0 )	// nothing to do
			continue;

		if ( pCacheType->RefCount >= pCacheType->NumSlots )
		{
			if ( pCacheType->NumSlots == 1 )	// can't free the last slot of a type
				continue;

			if ( pCacheType->SlotNeedRating >= NotType->SlotNeedRating )
				continue;  // I need it more that you do!

			if ( pCacheType->Uses >= pCacheType->NumSlots )
				continue;	// don't free if all slots are used!!
		
			if ( pCacheType->OverflowMax > 0.1f )
				continue;	// don't free it if recently missed

			if ( pCacheType->AddedCountdown > 0 )
				continue;	// just added one here recently, don't remove it !
		}
		else
		{
			// RefCount < NumSlots ; take me!
		}

		// find the oldest slot
		for (k=0; k< pCacheType->NumSlots; k++)
		{
			pSlot = pCacheType->SlotPtrs[k];
			
			assert( pSlot->IsActive );

			if ( ! OldestSlot || pSlot->LRU < OldestSlot->LRU )
			{
				OldestSlot = pSlot;
				OldestType = pCacheType;
				OldestIndex = k;
			}
		}
	}

	if ( ! OldestSlot )
	{
		// now look for one that's not so readily removed

		for (pCacheType	= Cache->CacheTypes, i=0; i< D3DCACHE_MAX_CACHE_TYPES; i++, pCacheType++)
		{
			D3DCache_Slot* pSlot{};
			int32			k{};

			if ( pCacheType == NotType )
				continue;

			if ( pCacheType->NumSlots <= 1 )	// can't free the last slot of a type
				continue;

			if ( pCacheType->SlotNeedRating >= NotType->SlotNeedRating )
				continue;  // I need it more that you do!

			if ( pCacheType->AddedCountdown > 0 )
				continue;	// just added one here recently, don't remove it !

			// find the oldest slot
			for (k=0; k< pCacheType->NumSlots; k++)
			{
				pSlot = pCacheType->SlotPtrs[k];
				
				assert( pSlot->IsActive );

				if ( ! OldestSlot || pSlot->LRU < OldestSlot->LRU )
				{
					OldestSlot = pSlot;
					OldestType = pCacheType;
					OldestIndex = k;
				}
			}
		}
	}

	if ( Critical && ! OldestSlot )
	{
		// now remove one at all cost!
			
		if ( ! OldestSlot )
		{
			for (pCacheType	= Cache->CacheTypes, i=0; i< D3DCACHE_MAX_CACHE_TYPES; i++, pCacheType++)
			{
				D3DCache_Slot* pSlot{};
				int32			k{};

				if ( pCacheType == NotType )
					continue;

				if ( pCacheType->NumSlots == 0 )	// nothing to do
					continue;

				// find the oldest slot
				for (k=0; k< pCacheType->NumSlots; k++)
				{
					pSlot = pCacheType->SlotPtrs[k];
					
					assert( pSlot->IsActive );

					if ( ! OldestSlot || pSlot->LRU < OldestSlot->LRU )
					{
						OldestSlot = pSlot;
						OldestType = pCacheType;
						OldestIndex = k;
					}
				}
			}
		}
	}

	if ( ! OldestSlot )
		return JE_FALSE; // nothing to free!

	if ( ! Critical )
	{
		// compare OldestType and NotType
		if ( OldestType->Uses >= OldestType->NumSlots ) // seriously try to avoid destroying this
		{
			// if both types are overflowing, don't destroy a larger one to make a smaller
			if ( OldestType->Size >= NotType->Size )
				return JE_FALSE;
		}
	}

	D3DCache_FreeSlot(OldestSlot);

	if ( OldestType->NumSlots > 0 )
	{
		D3DCache_Slot* pSlot{};

		// slide the list to keep it linear

		pSlot = OldestType->SlotPtrs[OldestType->NumSlots];
		assert( pSlot );

		OldestType->SlotPtrs[OldestIndex] = pSlot;
		OldestType->SlotPtrs[OldestType->NumSlots] = nullptr;
	}

//	OldestType->RemovedCountdown = 7; // <> don't add here again right away ?
		// <> not good : the _Balance may look like this :
		//		trying to add type 1
		//			remove type 2, remove type 3
		//			add type 1 still fails!
		//		so now re-add a type 2 & type 3
		//		hence we MUST be able to add right after a remove

return JE_TRUE;
}


//========================================================================================================
//	D3DCache_FreeAllSlots
//========================================================================================================
jeBoolean D3DCache_FreeAllSlots(D3DCache *Cache) // never called externally
{
	int32			i{};
	D3DCache_Type* pCacheType{};

	assert(D3DCache_IsValid(Cache));

#ifdef _DEBUG
	D3DMain_Log("D3DCache_FreeAllSlots starting\n");
#endif

	OutputDebugString("D3DCache_FreeAllSlots\n");

	for (pCacheType = Cache->CacheTypes, i=0; i< D3DCACHE_MAX_CACHE_TYPES; i++, pCacheType++)
	{
		D3DCache_Slot* pSlot{};
		int32			k{}, ns{};

		if ( pCacheType->RefCount == 0 && pCacheType->NumSlots == 0 )
			continue; // nothing here!

		assert(D3DCache_TypeIsValid(pCacheType));

		// Go through each slot, and free all the surfaces on them
		ns = pCacheType->NumSlots;
		for (k=0; k< ns; k++)
		{
			pSlot = pCacheType->SlotPtrs[k];
			assert(pSlot->IsActive);
			D3DCache_FreeSlot(pSlot);
		}

		assert( pCacheType->NumSlots == 0 );
	}

	DDMemMgr_PartitionReset(Cache->Partition);			// Reset the caches memory manager

	Cache->AvailMem = Cache->CardMem;
	Cache->SoughtMem = 0;

#ifdef _DEBUG
	D3DMain_Log("D3DCache_FreeAllSlots done\n");
#endif

	return JE_TRUE;
}

//========================================================================================================
//	D3DCache_WriteToFile
//========================================================================================================
void D3DCacheLog_WriteToFile(void)
{

#ifdef DO_LOG
	SYSTEMTIME		Time{};
	FILE* f{};

#pragma message("D3DCache : Logging")

	f = fopen("D3DCache.log", "a+t");

	if (!f)
		return;

	GetSystemTime(&Time);

	fprintf(f, "=======================================================\n");
	fprintf(f, "Date: %i/%i/%i, Time: %i:%02i\n", Time.wMonth, Time.wDay, Time.wYear, Time.wHour, Time.wMinute);

	fprintf(f, "Compiled : %s,%s\n",__DATE__,__TIME__);

	fprintf(f, "---------- total:\n");

	fprintf(f, "Frames = %d; Cache Types = %d; slots created/destroyed = %d/%d\n",
		D3DCacheLog.frames,D3DCacheLog.maxCacheTypes,D3DCacheLog.slotsCreated,D3DCacheLog.slotsDestroyed);

	fprintf(f, "tex misses = %d/%d , lmap misses = %d/%d\n",
		D3DCacheLog.TexMissesTot,D3DCacheLog.TexMissBytesTot,
		D3DCacheLog.LMapMissesTot,D3DCacheLog.LMapMissBytesTot);

	fprintf(f, "---------- per frame:\n");

	fprintf(f, "update = %f secs, mip bias = %f avg/%f max\n",
			D3DCacheLog.updateTime/(double)D3DCacheLog.frames,
			D3DCacheLog.MipBiasTot/(double)D3DCacheLog.frames,
			D3DCacheLog.MipBiasMax);

	fprintf(f, "mem = %f card/%f slots/%f sought\n",
			D3DCacheLog.CardMemTot/(double)D3DCacheLog.frames,
			D3DCacheLog.SlotMemTot/(double)D3DCacheLog.frames,
			D3DCacheLog.UsedMemTot/(double)D3DCacheLog.frames);

	fclose(f);
#endif

}

//========================================================================================================
//	D3DCache_TypeAddSlot
//========================================================================================================
jeBoolean D3DCache_TypeAddSlot(D3DCache_Type *pCacheType)
{
	D3DCache_Slot* pSlot{};
	uint32			Result{};

	// allocates a new slot for this type
	//	returns true if actually did an alloc
	assert( D3DCache_TypeIsValid(pCacheType) );

	if (pCacheType->RefCount <= 0) // nothing here
	{
		// <> !! why are you calling TypeAddSlot !?
		return JE_FALSE;
	}

	// NumAvail > NumTextures
	if (pCacheType->NumSlots >= pCacheType->RefCount)
		return JE_FALSE;		// This is all we need for this slot...

	if ( pCacheType->NumSlots == D3DCACHE_MAX_CACHE_SLOTS )
		return JE_FALSE;

	assert( pCacheType->Cache->TotNumSlots <= D3DInfo.MaxSurfaceCount );
	if ( pCacheType->Cache->TotNumSlots == D3DInfo.MaxSurfaceCount )
	{
		// sorry, no more slots allowed
		return JE_FALSE;
	}

	if (!DDMemMgr_PartitionAllocMem(pCacheType->Cache->Partition, pCacheType->Size))
	{
		return JE_FALSE;				// No more memory in the partition, stop now...
		// <> CB : there might actually still be memory on the card !
	}

	// find a free slot
	pSlot = (D3DCache_Slot *) MemPool_GetHunk(pCacheType->Cache->SlotPool);
	
	pSlot->SelfCheck = pSlot;
	pSlot->CacheType = pCacheType;
	pSlot->MipData = nullptr;
	pSlot->LRU = Scene_CurrentFrame - 1; //<>

	// Allocate surfaces now
	Result = D3DCache_SetupSlot(pCacheType->Cache, pSlot, pCacheType->Width, pCacheType->Height, &pCacheType->ddsd, pCacheType->Cache->UseStages, pCacheType->Stage);

	if ( Result <= 0)
	{
		// Out of ram, cancel operation, and stop allocating
		// DON'T memset it to zero, cuz someone else might have a ref to this slot!
		pSlot->Surface = nullptr;
		pSlot->IsActive = JE_FALSE;
		
		assert(Result == 0); // if not == 0, it's worse than out-of-memory !!

		DDMemMgr_PartitionFreeMem(pCacheType->Cache->Partition, pCacheType->Size);

		MemPool_FreeHunk(pCacheType->Cache->SlotPool, pSlot);

		return JE_FALSE;
	}

	pSlot->IsActive = JE_TRUE;

	assert( D3DCache_SlotIsValid(pSlot) );

	assert( pCacheType->SlotPtrs[pCacheType->NumSlots] == nullptr );

	pCacheType->SlotPtrs[pCacheType->NumSlots] = pSlot;

	// The slot was allocated, so increase the number of slots now
	pCacheType->NumSlots++;
	pCacheType->Cache->TotNumSlots ++;

	#ifdef DO_LOG
	D3DCacheLog.slotsCreated ++;
	#endif

return JE_TRUE;
}

//========================================================================================================
//	D3DCache_CheckSlots
//========================================================================================================
jeBoolean D3DCache_CheckSlots(D3DCache *Cache)
{

	// do nothing

	return JE_TRUE;
}

//========================================================================================================
//	D3DCache_SlotIsValid
//========================================================================================================
jeBoolean D3DCache_SlotIsValid(D3DCache_Slot *Slot)
{
	if (!Slot)
		return JE_FALSE;

	if (Slot->SelfCheck != Slot)
		return JE_FALSE;

	return JE_TRUE;
}

//=====================================================================================
//	D3DCache_SetupSlot
//
//	Returns -1 on failure
//	Returns 0 on out of memory
//	Returns 1 on success
//=====================================================================================
int32 D3DCache_SetupSlot(D3DCache *Cache, D3DCache_Slot *Slot, int32 Width, int32 Height, const DDSURFACEDESC2 *SurfDesc, jeBoolean UseStage, int32 Stage)
{
	LPDIRECTDRAWSURFACE7 Surface{};
	DDSURFACEDESC2		ddsd{};
	HRESULT				Hr{};
			
	assert(D3DCache_IsValid(Cache));
	assert(D3DCache_SlotIsValid(Slot));

	memcpy(&ddsd, SurfDesc, sizeof(DDSURFACEDESC2));

	ddsd.dwSize = sizeof(DDSURFACEDESC2);
	ddsd.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT;

	if (UseStage)
		ddsd.dwFlags |= DDSD_TEXTURESTAGE;

	ddsd.ddsCaps.dwCaps = DDSCAPS_TEXTURE | DDSCAPS_VIDEOMEMORY | DDSCAPS_LOCALVIDMEM;
	ddsd.ddsCaps.dwCaps2 = DDSCAPS2_HINTDYNAMIC;
	ddsd.ddsCaps.dwCaps3 = 0;
	ddsd.ddsCaps.dwCaps4 = 0;
	ddsd.dwHeight = Width;
	ddsd.dwWidth = Height;
	
	ddsd.dwTextureStage = Stage;
	
	Hr = Cache->lpDD->CreateSurface(&ddsd, &Surface, NULL);

	if(Hr != DD_OK) 
	{ 
		if (Hr == DDERR_OUTOFVIDEOMEMORY)
		{
			return 0;
		}

		return -1;
	}

	Slot->Surface = Surface;

	// Set the color key
#if 0
	{
		DDCOLORKEY			CKey;
		
		// Create the color key for this surface
		CKey.dwColorSpaceLowValue = 1;
		CKey.dwColorSpaceHighValue = 1;
		
		if (Slot->Surface->SetColorKey(DDCKEY_SRCBLT , &CKey) != DD_OK)
		{
			Slot->Surface->Release();
			Slot->Surface = nullptr;
			return -1;
		}
	}
 #endif

	return 1;		// All good dude
}


//========================================================================================================
//	D3DCache_TypeFindSlot
//========================================================================================================
void		D3DCache_TypeMissed(D3DCache_Type *CacheType,int KickFrame)
{
	// TypeFindSlot is called whenever you get a cache miss
	//	sort of a hack
	CacheType->Misses ++;

	// <> use the stage to tell whether it's a Tex or LMap
	// kind of a hack;
	// with the layer system there will be no difference between texes & lmaps
	if ( CacheType->Stage == 0 )
	{
		PCache_CacheInfo.TexMisses++;
		PCache_CacheInfo.TexMissBytes += CacheType->Size;
	}
	else
	{
		PCache_CacheInfo.LMapMisses++;
		PCache_CacheInfo.LMapMissBytes += CacheType->Size;
	}

	assert((int)Scene_CurrentFrame >= KickFrame);

	if ( Scene_CurrentFrame > 30 && (Scene_CurrentFrame - KickFrame) < 10 )
	{
		CacheType->FreshMisses ++;

		PCache_CacheInfo.TexMissesFresh ++;
	}
}

D3DCache_Slot *D3DCache_TypeFindSlot(D3DCache_Type *CacheType)
{
	D3DCache_Slot* pBestSlot{}, * pSlot{};
	int32			i{};

	assert(D3DCache_TypeIsValid(CacheType));

	if ( CacheType->NumSlots == 0 )
	{
		// no slots ! must make one

		while ( ! D3DCache_TypeAddSlot(CacheType) )
		{
			if ( ! D3DCache_RemoveOldestSlot(CacheType->Cache,CacheType,JE_TRUE) )
			{
				return nullptr;
			}
		}
	}

	assert( CacheType->NumSlots > 0 );

	// find the oldest active slot

	pBestSlot = nullptr;

	for (i=0; i<CacheType->NumSlots; i++)
	{
		pSlot = CacheType->SlotPtrs[i];

		assert(pSlot);
		assert(pSlot->IsActive);
		assert(D3DCache_SlotIsValid(pSlot));

		if ( ! pBestSlot || pSlot->LRU < pBestSlot->LRU)
		{
			pBestSlot = pSlot;
		}
	}

	assert(pBestSlot);

	// possibly kick the texture that's in there now

	if ( pBestSlot->MipData && pBestSlot->MipData->Slot == pBestSlot )
	{
		pBestSlot->MipData->KickedFrame = Scene_CurrentFrame;
		pBestSlot->MipData->Slot = nullptr;
	}

	pBestSlot->LRU = Scene_CurrentFrame - 1; // so it doesn't get kicked right away
	pBestSlot->MipData = nullptr;

	assert(D3DCache_SlotIsValid(pBestSlot));

	return pBestSlot;
}

//========================================================================================================
//	D3DCache_SlotSetMipData
//========================================================================================================
void D3DCache_SlotSetMipData(D3DCache_Slot *Slot, THandle_MipData *MipData)
{
	assert(D3DCache_SlotIsValid(Slot));

	if ( MipData )
		MipData->KickedFrame = 0; // <> eg. hasn't been kicked
	Slot->MipData = MipData;
}

//========================================================================================================
//	D3DCache_SlotGetMipData
//========================================================================================================
THandle_MipData *D3DCache_SlotGetMipData(D3DCache_Slot *Slot)
{
	assert(D3DCache_SlotIsValid(Slot));

	if ( ! Slot->IsActive )
		return nullptr;

	return Slot->MipData;
}

//========================================================================================================
//	D3DCache_SlotSetLRU
//========================================================================================================
void D3DCache_SlotSetLRU(D3DCache_Slot *Slot, uint32 LRU)
{
	assert(D3DCache_SlotIsValid(Slot));

	Slot->LRU = LRU;
}

//========================================================================================================
//	D3DCache_SlotGetLRU
//========================================================================================================
uint32 D3DCache_SlotGetLRU(D3DCache_Slot *Slot)
{
	assert(D3DCache_SlotIsValid(Slot));

	return Slot->LRU;
}

//========================================================================================================
//	D3DCache_SlotGetSurface
//========================================================================================================
LPDIRECTDRAWSURFACE7 D3DCache_SlotGetSurface(D3DCache_Slot *Slot)
{
	assert(D3DCache_SlotIsValid(Slot));

	assert( Slot->IsActive );

	return Slot->Surface;
}
