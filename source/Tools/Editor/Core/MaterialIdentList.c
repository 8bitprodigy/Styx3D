/****************************************************************************************/
/*  MaterialIdentList.c                                                                        */
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
#include <string.h>

#include "Ram.h"

#include "MaterialIdentList.h"
#include "Util.h"


MaterialIdentList *	MaterialIdentList_Create( void )
{
	return (MaterialIdentList*) List_Create();
}

void MaterialIdentList_Destroy( MaterialIdentList **ppList )
{
	List_Destroy( (List**)ppList, MaterialIdentList_DestroyCB) ;
}

MaterialIdent *		MaterialIdentList_GetFirst( MaterialIdentList * pList, MaterialIdentIterator * Iterator )
{
	return (MaterialIdent*)List_GetFirst( pList, Iterator ) ;
}

MaterialIdent *		MaterialIdentList_GetNext( MaterialIdentList * pList, MaterialIdentIterator * Iterator )
{
	return (MaterialIdent*)List_GetNext( pList, Iterator ) ;
}

int32				MaterialIdentList_GetNumItems( MaterialIdentList * pList )
{
	return List_GetNumItems( pList ) ;
}

MaterialIdent*		MaterialIdentList_FindByName( MaterialIdentList * pList, const char* Name )
{
	MaterialIdent* pResult;
	MaterialIdentIterator it;

	pResult = List_GetFirst(pList, &it);
	while (pResult) {
		if (stricmp(pResult->MaterialName, Name) == 0) {
			break;
		}
		pResult = List_GetNext(pList, &it);
	}

	return pResult;
}

MaterialIdent*		MaterialIdentList_FindByWorldIndex( MaterialIdentList * pList, int32 WorldIndex )
{
	MaterialIdent* pResult;
	MaterialIdentIterator it;

	pResult = List_GetFirst(pList, &it);
	while (pResult) {
		if (pResult->WorldMatIdx == WorldIndex) {
			break;
		}
		pResult = List_GetNext(pList, &it);
	}

	return pResult;
}

MaterialIdent* MaterialIdentList_FindByFileIndex( MaterialIdentList * pList, int32 FileIndex)
{
	MaterialIdent* pResult;
	MaterialIdentIterator it;

	pResult = List_GetFirst(pList, &it);
	while (pResult) {
		if (pResult->FileMatIdx == FileIndex) {
			break;
		}
		pResult = List_GetNext(pList, &it);
	}

	return pResult;
}

int32 MaterialIdentList_GetWorldIndexByFileIndex(MaterialIdentList * pList, int32 FileIndex)
{
	MaterialIdent* pResult;

	pResult = MaterialIdentList_FindByFileIndex(pList, FileIndex);

	if (pResult) {
		return pResult->WorldMatIdx;
	}
	return -1;
}


MaterialIdentIterator	MaterialIdentList_Append( MaterialIdentList * pList, MaterialIdent * pMaterialIdent )
{
	if (MaterialIdentList_FindByFileIndex(pList, pMaterialIdent->FileMatIdx) == NULL) {
		return List_Append(pList, pMaterialIdent);
	}
	return NULL;
}

void MaterialIdentList_Remove( MaterialIdentList * pList, MaterialIdent * pMaterialIdent )
{
	MaterialIdent* pResult;
	MaterialIdentIterator currentIt;
	MaterialIdentIterator prevIt;

	prevIt = NULL;
	pResult = List_GetFirst(pList, &currentIt);
	while (pResult) {
		if (pResult->FileMatIdx == pMaterialIdent->FileMatIdx) {
			List_Remove(pList, prevIt, NULL);
			return;
		}
		prevIt = currentIt;
		pResult = List_GetNext(pList, &currentIt);
	}
}

int32 MaterialIdentList_EnumMaterialIdents( MaterialIdentList * pList, void * pVoid, MaterialIdentListCB Callback )
{
	return List_ForEach( pList, Callback, pVoid ) ;
}

void MaterialIdentList_DestroyCB( MaterialIdent * pMaterialIdent )
{
	jeRam_Free(pMaterialIdent);
}


/* EOF: MaterialIdentList.c */