/****************************************************************************************/
/*  BitmapList.c                                                                        */
/*                                                                                      */
/*  Author: Charles Bloom                                                               */
/*  Description: Maintains a pool of bitmap pointers.                                   */
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
#include <assert.h>
#include <string.h>

#ifdef _DEBUG
#ifdef WIN32
#include <windows.h>
#include <stdio.h>
#else
#include <memory.h>
#endif
#endif

#include "BitmapList.h"
#include "Dcommon.h"
#include "Bitmap.h"
#include "Bitmap._h"
#include "List.h"
#include "MemPool.h"
#include "Errorlog.h"
#include "Ram.h"
//#include "tsc.h"

struct BitmapList
{
	Hash * HashPtr; // CJP : Modified to not be Hash* Hash
	int Members,Adds;
#ifdef _DEBUG
	BitmapList * MySelf;
#endif
};


jeBoolean BitmapList_IsValid(BitmapList *pList);

//================================================================================
//	BitmapList_Create
//================================================================================
BitmapList *BitmapList_Create(void)
{
BitmapList * pList;
	pList = (BitmapList *)jeRam_Allocate(sizeof(*pList));
	if (! pList )
		return NULL;
	memset(pList,0,sizeof(*pList));
	pList->HashPtr = Hash_Create();
	if ( ! pList->HashPtr )
	{
		jeRam_Free(pList);
		return NULL;
	}
	#ifdef _DEBUG
	pList->MySelf = pList;
	#endif
return pList;
}

//================================================================================
//	BitmapList_Destroy
//================================================================================
jeBoolean BitmapList_Destroy(BitmapList *pList)
{
jeBoolean	Ret = JE_TRUE;

	if ( ! pList )
		return JE_TRUE;

	if ( pList->HashPtr )
	{
	HashNode	*pNode;
		pNode = NULL;
		
		while( (pNode = Hash_WalkNext(pList->HashPtr,pNode)) != NULL )
		{
		jeBitmap *Bmp;
		uint32 TimesAdded;

			HashNode_GetData(pNode,(uint32 *)&Bmp,&TimesAdded);

			if (!jeBitmap_DetachDriver(Bmp, JE_TRUE))
				Ret = JE_FALSE;

			assert( pList->Members >= 1 && pList->Adds >= (int)TimesAdded );

			pList->Members --;

			assert( TimesAdded >= 1 );

			while(TimesAdded --)
			{
				assert(Bmp);
				jeBitmap_Destroy(&Bmp);
				pList->Adds --;
			}
		}

		// Finally, destroy the entire hash table
		Hash_Destroy(pList->HashPtr);
	}

	jeRam_Free(pList);

	return Ret;
}

//================================================================================
//	BitmapList_SetGamma
//================================================================================
jeBoolean BitmapList_SetGamma(BitmapList *pList, jeFloat Gamma)
{
HashNode *pNode;

	assert(BitmapList_IsValid(pList));

#ifdef _DEBUG
	//pushTSC();
#endif

	pNode = NULL;
	while( (pNode = Hash_WalkNext(pList->HashPtr,pNode)) != NULL )
	{
	jeBitmap *Bmp;
		Bmp = (jeBitmap *)HashNode_Key(pNode);

		if (!jeBitmap_SetGammaCorrection(Bmp, Gamma, JE_TRUE) )
		{
			jeErrorLog_AddString(-1,"BitmapList_SetGamma : SetGamma failed.", NULL);
			return JE_FALSE;
		}
	}
	
#ifdef _DEBUG
	//showPopTSCper("BitmapList_SetGamma",pList->MembersAttached,"bitmap");
#endif

return JE_TRUE;
}

//================================================================================
//	BitmapList_AttachAll
//================================================================================
jeBoolean BitmapList_AttachAll(BitmapList *pList, DRV_Driver *Driver, jeFloat Gamma)
{
HashNode *pNode;
int MembersAttached;

	assert(BitmapList_IsValid(pList));

	//pushTSC();

	pNode = NULL;
	MembersAttached = 0;
	while( (pNode = Hash_WalkNext(pList->HashPtr,pNode)) != NULL )
	{
	jeBitmap *Bmp;

		Bmp = (jeBitmap *)HashNode_Key(pNode);

		if (!jeBitmap_SetGammaCorrection_DontChange(Bmp, Gamma) )
		{
			jeErrorLog_AddString(-1,"BitmapList_AttachAll : SetGamma failed", NULL);
			return JE_FALSE;
		}

		if (!jeBitmap_AttachToDriver(Bmp, Driver, 0) )
		{
			jeErrorLog_AddString(-1,"BitmapList_AttachAll : AttachToDriver failed", NULL);
			return JE_FALSE;
		}

		MembersAttached ++;
	}

	//showPopTSC("BitmapList_AttachAll");

	assert( MembersAttached == pList->Members );

	return JE_TRUE;
}

//================================================================================
//	BitmapList_DetachAll
//================================================================================
jeBoolean BitmapList_DetachAll(BitmapList *pList)
{
HashNode	*pNode;
jeBoolean	Ret = JE_TRUE;
int MembersAttached;

	assert(BitmapList_IsValid(pList));

	pNode = NULL;
	while( (pNode = Hash_WalkNext(pList->HashPtr,pNode)) != NULL )
	{
	jeBitmap *Bmp;
	uint32 TimesAdded;

		HashNode_GetData(pNode,(uint32 *)&Bmp,&TimesAdded);

		if (!jeBitmap_DetachDriver(Bmp, JE_TRUE))
			Ret = JE_FALSE;
	}

	MembersAttached = 0;

	return Ret;
}

//================================================================================
//	BitmapList_CountMembers
//================================================================================
int BitmapList_CountMembers(BitmapList *pList)
{
#ifdef NDEBUG
	return pList->Members;
#else
HashNode *pNode;
int Count;

	Count = 0;
	pNode = NULL;
	while( (pNode = Hash_WalkNext(pList->HashPtr,pNode)) != NULL )
	{
		Count ++;
	}

	assert( Count == pList->Members );
	assert( pList->Adds >= pList->Members );

return Count;
#endif
}
int BitmapList_CountMembersAttached(BitmapList *pList)
{
HashNode *pNode;
int Count;

	Count = 0;
	pNode = NULL;
	while( (pNode = Hash_WalkNext(pList->HashPtr,pNode)) != NULL )
	{
	jeBitmap *Bmp;
	uint32 TimesAdded;

		HashNode_GetData(pNode,(uint32 *)&Bmp,&TimesAdded);

		if ( jeBitmap_GetTHandle(Bmp) )
			Count ++;
	}

	assert( pList->Adds >= pList->Members && pList->Members >= Count );

return Count;
}

//================================================================================
//	BitmapList_Has
//================================================================================
jeBoolean BitmapList_Has(BitmapList *pList, jeBitmap *Bitmap)
{
HashNode *pNode;
uint32 TimesAdded;

	assert(pList && Bitmap);

	pNode = Hash_Get(pList->HashPtr,(uint32)Bitmap,&TimesAdded);

	assert( pList->Adds >= (int)TimesAdded );

return (pNode && TimesAdded) ? JE_TRUE : JE_FALSE;
}

//================================================================================
//	BitmapList_Add
//================================================================================
jeBoolean BitmapList_Add(BitmapList *pList, jeBitmap *Bitmap)
{	
HashNode *pNode;
uint32 TimesAdded;

	assert(BitmapList_IsValid(pList));
	assert(Bitmap);

	// Increase reference count on this Bitmap
	jeBitmap_CreateRef(Bitmap);

	pList->Adds ++;

	if ( (pNode = Hash_Get(pList->HashPtr, (uint32)Bitmap, &TimesAdded)) != NULL )
	{
		HashNode_SetData(pNode,TimesAdded+1);
		return JE_FALSE;
	}
	else
	{
		pList->Members ++;
		Hash_Add(pList->HashPtr,(uint32)Bitmap,1);
		return JE_TRUE;
	}
}

//================================================================================
//	BitmapList_Remove
//================================================================================
jeBoolean BitmapList_Remove(BitmapList *pList,jeBitmap *Bitmap)
{
HashNode *pNode;
uint32 TimesAdded;
uint32 Key;

	assert(BitmapList_IsValid(pList));
	assert(Bitmap);

	Key = (uint32) Bitmap;
	pNode = Hash_Get(pList->HashPtr,Key,&TimesAdded);

	assert(pNode);

	pList->Adds --;
	TimesAdded --;

	if ( TimesAdded <= 0 )
	{
		if ( ! jeBitmap_DetachDriver(Bitmap, JE_TRUE) )
		{
			jeErrorLog_AddString(-1, "BitmapList_Remove:  jeBitmap_DetachDriver failed.", NULL);
			return JE_FALSE;
		}
	}

	jeBitmap_Destroy(&Bitmap);

	if ( TimesAdded <= 0 )
	{
		pList->Members --;
		Hash_DeleteNode(pList->HashPtr,pNode);
		return JE_TRUE;
	}
	else
	{
		HashNode_SetData(pNode,TimesAdded);
		return JE_FALSE;
	}
}


jeBoolean BitmapList_IsValid(BitmapList *pList)
{
	if ( ! pList ) 
		return JE_FALSE;
		
	if ( pList->Adds < pList->Members )
		return JE_FALSE;

#ifdef _DEBUG
	if ( pList->MySelf != pList )
		return JE_FALSE;
#endif

	if ( pList->Members != BitmapList_CountMembers(pList) )
		return JE_FALSE;

return JE_TRUE;
}
