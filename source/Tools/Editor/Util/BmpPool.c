/****************************************************************************************/
/*  BMPPOOL.C                                                                           */
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
#include <Assert.h>
#include "BmpPool.h"
#include "Ram.h"
#include "string.h"
#include "jelist.h"

#define NAME_MAX 64

typedef struct {
	char  Name[NAME_MAX];
	HBITMAP hBitmap;
	int32 Usage;
} BmpEntry;

typedef struct BmpPool{
	List* Pool;
} BmpPool;


BmpPool* BmpPool_Create()
{
	BmpPool* pBmpPool;

	pBmpPool = (BmpPool*)JE_RAM_ALLOCATE_STRUCT( BmpPool  );
	if( pBmpPool == NULL )
	{
		return( NULL );
	}
	pBmpPool->Pool = List_Create();
	if( pBmpPool->Pool == NULL )
	{
		jeRam_Free( pBmpPool );
		return( NULL );
	}
	return( pBmpPool );
}

BmpEntry* BmpSearch( List * pList, char* CompName, ListIterator *pli )
{
	BmpEntry* pBmpEntry;

	pBmpEntry = (BmpEntry*)List_GetFirst (pList, pli);

	while( pBmpEntry != NULL )
	{
		if( stricmp( pBmpEntry->Name, CompName ) == 0 )
			return( pBmpEntry );
		pBmpEntry = (BmpEntry*)List_GetNext (pList, pli);
	}


	return( NULL );
}

BmpEntry* BmpSearchPtr( List * pList, HBITMAP hBitmap, ListIterator *pli )
{
	BmpEntry* pBmpEntry;

	pBmpEntry = (BmpEntry*)List_GetFirst (pList, pli);

	while( pBmpEntry != NULL )
	{
		if( pBmpEntry->hBitmap == hBitmap  )
			return( pBmpEntry );
		pBmpEntry = (BmpEntry*)List_GetNext (pList, pli);
	}


	return( NULL );
}


jeBoolean BmpPool_Add( BmpPool* pBmpPool, HINSTANCE	hRes, char* BmpName )
{
	BmpEntry *pNewEntry;
	int32 sLen;
	ListIterator li;

	if( BmpSearch( pBmpPool->Pool, BmpName, &li) != NULL )
		return( JE_TRUE );
#ifndef NDEBUG
	sLen = strlen( BmpName );
	assert( sLen < NAME_MAX );
#endif
	pNewEntry = JE_RAM_ALLOCATE_STRUCT( BmpEntry );
	strcpy( pNewEntry->Name, BmpName );
	pNewEntry->Usage = 0;

//	pNewEntry->hBitmap = LoadBitmap( hRes, BmpName );
	// CJP : Modified to load from file, since resources dont appear to be working.
	pNewEntry->hBitmap = LoadImage(hRes,BmpName,IMAGE_BITMAP, 0,0,LR_DEFAULTSIZE | LR_LOADFROMFILE);//LoadBitmap( hRes, BmpName );

	List_Append( pBmpPool->Pool,  pNewEntry );
	return( JE_TRUE );

	// get rid of warnings
	sLen;
}

// Added by CJP to allow bitmaps to be loaded from a file and named independently.
BOOL BmpPool_AddWithName(BmpPool* pBmpPool, HINSTANCE hRes, char* BmpFile, char* BmpName)
{
	BmpEntry *pNewEntry;
	int32 sLen;
	ListIterator li;

	if( BmpSearch( pBmpPool->Pool, BmpName, &li) != NULL )
		return( JE_TRUE );
#ifndef NDEBUG
	sLen = strlen( BmpName );
	assert( sLen < NAME_MAX );
#endif
	pNewEntry = JE_RAM_ALLOCATE_STRUCT( BmpEntry );
	strcpy( pNewEntry->Name, BmpName );
	pNewEntry->Usage = 0;

// Modified to load from file, since resources dont appear to be working.
	pNewEntry->hBitmap = LoadImage(hRes,BmpFile,IMAGE_BITMAP, 0,0,LR_DEFAULTSIZE | LR_LOADFROMFILE);//LoadBitmap( hRes, BmpName );
	List_Append( pBmpPool->Pool,  pNewEntry );
	return( JE_TRUE );

	// get rid of warnings
	sLen;
}

//  End added by CJP

HBITMAP BmpPool_Get( BmpPool* pBmpPool, char* BmpName  )
{
	BmpEntry * pBmpEntry;
	ListIterator li;


	pBmpEntry = BmpSearch( pBmpPool->Pool, BmpName, &li );
	if( pBmpEntry != NULL )
	{
		pBmpEntry->Usage++;
		return( pBmpEntry->hBitmap );
	}		
	return( NULL );
}

void BmpPool_DestroyEntryDestroy( List * pList, BmpEntry * pBmpEntry, ListIterator *pli )
{

	// remove the bitmap
	if( pBmpEntry->hBitmap != NULL )
		DeleteObject(pBmpEntry->hBitmap);
	List_Remove (pList, *pli, NULL);
	jeRam_Free( pBmpEntry );
}

jeBoolean BmpPool_Release( BmpPool* pBmpPool, char* BmpName )
{
	BmpEntry * pBmpEntry;
	ListIterator li;

	pBmpEntry = BmpSearch( pBmpPool->Pool, BmpName, &li );
	if(pBmpEntry != NULL )
	{
		assert( pBmpEntry->Usage > 0 );
		pBmpEntry->Usage--;
		if( pBmpEntry->Usage == 0 )
			BmpPool_DestroyEntryDestroy( pBmpPool->Pool, pBmpEntry, &li );
		return( JE_TRUE );
	}
	return( JE_FALSE );
}


jeBoolean BmpPool_ReleasePtr( BmpPool* pBmpPool, HBITMAP hBitmap )
{
	BmpEntry * pBmpEntry;
	ListIterator li;

	pBmpEntry = BmpSearchPtr( pBmpPool->Pool, hBitmap, &li );
	if(pBmpEntry != NULL )
	{
		assert( pBmpEntry->Usage > 0 );
		pBmpEntry->Usage--;
		if( pBmpEntry->Usage == 0 )
			BmpPool_DestroyEntryDestroy( pBmpPool->Pool, pBmpEntry, &li );
		return( JE_TRUE );
	}
	return( JE_FALSE );
}

void BmpPool_Destroy( BmpPool** hBmpPool )
{
	BmpEntry* pBmpEntry;
	BmpEntry* pNextBmpEntry;
	ListIterator li;
	ListIterator Nextli;

	if( (*hBmpPool)->Pool != NULL )
	{
		pBmpEntry = (BmpEntry*)List_GetFirst ((*hBmpPool)->Pool, &li);
		// Added by cjp : will crash on first bitmap without the following line
		Nextli = li;

		while( pBmpEntry != NULL )
		{
			pNextBmpEntry = List_GetNext ((*hBmpPool)->Pool, &Nextli);
			BmpPool_DestroyEntryDestroy( (*hBmpPool)->Pool, pBmpEntry, &li );
			pBmpEntry = pNextBmpEntry;
			li = Nextli;
		}
		List_Destroy (&(*hBmpPool)->Pool, NULL );
	}
	jeRam_Free( (*hBmpPool) );
}
