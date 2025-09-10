/****************************************************************************************/
/*  MaterialIdentList.H                                                                        */
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

#ifndef MATERIALIDENTLIST_H
#define MATERIALIDENTLIST_H

#include "jeList.h"

#ifdef __cplusplus
extern "C" {
#endif
#include "jeMaterial.h"

typedef struct tabMaterialIdent
{
	char MaterialName[JE_MATERIAL_MAX_NAME_SIZE];
	char BitmapName[JE_MATERIAL_MAX_NAME_SIZE];
	int32 FileMatIdx;
	int32 WorldMatIdx;
} MaterialIdent;

typedef jeBoolean (*MaterialIdentListCB)( MaterialIdent *pMaterialIdent, void * pVoid );

typedef List					MaterialIdentList ;
typedef ListIterator			MaterialIdentIterator ;
typedef List_DestroyCallback	MaterialIdentList_DestroyCallback ;

MaterialIdentList *	MaterialIdentList_Create( void ) ;
void			MaterialIdentList_Destroy( MaterialIdentList **ppList ) ;

// ACCESSORS
MaterialIdent *		MaterialIdentList_GetFirst( MaterialIdentList * pList, MaterialIdentIterator * Iterator ) ;
MaterialIdent *		MaterialIdentList_GetNext( MaterialIdentList * pList, MaterialIdentIterator * Iterator ) ;

int32				MaterialIdentList_GetNumItems( MaterialIdentList * pList ) ;
MaterialIdent*		MaterialIdentList_FindByName( MaterialIdentList * pList, const char* Name ) ;
MaterialIdent*		MaterialIdentList_FindByWorldIndex( MaterialIdentList * pList, int32 WorldIndex ) ;
MaterialIdent*		MaterialIdentList_FindByFileIndex( MaterialIdentList * pList, int32 FileIndex) ;

// MODIFIERS
MaterialIdentIterator	MaterialIdentList_Append( MaterialIdentList * pList, MaterialIdent * pMaterialIdent ) ;
void				MaterialIdentList_Remove( MaterialIdentList * pList, MaterialIdent * pMaterialIdent ) ;


// ENUMERATION
int32			MaterialIdentList_EnumMaterialIdents( MaterialIdentList * pList, void * pVoid, MaterialIdentListCB Callback ) ;

// CALLBACK
void			MaterialIdentList_DestroyCB( MaterialIdent * pMaterialIdent ) ;

// ACCESSOR
int32			MaterialIdentList_GetWorldIndexByFileIndex(MaterialIdentList * pList, int32 FileIndex);

#ifdef __cplusplus
}
#endif

#endif // Prevent multiple inclusion

/* EOF: MaterialIdentList.h */