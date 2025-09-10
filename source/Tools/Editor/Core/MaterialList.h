/****************************************************************************************/
/*  MATERIALLIST.H                                                                      */
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
#pragma once
#ifndef MATERIALLIST_H
#define MATERIALLIST_H

#include "jeList.h"
#include "Materials.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct MaterialList_Struct MaterialList_Struct ;
typedef ListIterator MaterialIterator ;

typedef jeBoolean (*MaterialListCB)( Material_Struct * pMaterial, void * pVoid ) ;

MaterialList_Struct *		MaterialList_Create( void ) ;
void				MaterialList_Destroy( MaterialList_Struct **ppList ) ;

// ACCESSORS
int32				MaterialList_GetNumItems( const MaterialList_Struct * pList ) ;
Material_Struct *	MaterialList_GetMaterial(		MaterialList_Struct * pList, MaterialIterator pMI ) ;
Material_Struct *	MaterialList_GetFirstMaterial(	MaterialList_Struct * pList, MaterialIterator * pMI ) ;
Material_Struct *	MaterialList_GetNextMaterial(	MaterialList_Struct * pList, MaterialIterator * pMI ) ;


Material_Struct *	MaterialList_GetCurMaterial( MaterialList_Struct* MaterialList );
void				MaterialList_SetCurMaterial( MaterialList_Struct* MaterialList, Material_Struct* Material );

// MODIFIERS
MaterialIterator	MaterialList_Append( MaterialList_Struct * pList, Material_Struct * pMaterial ) ;
jeBoolean			MaterialList_LoadFromDir( MaterialList_Struct* MaterialList, jeEngine* pEngine, jeResourceMgr* pResMgr, char* DirPath );

// ENUMERATION
int32				MaterialList_EnumMaterials( MaterialList_Struct * pList, void * pVoid, MaterialListCB Callback ) ;

// SEARCH
Material_Struct *	MaterialList_SearchByName( MaterialList_Struct* MaterialList, MaterialIterator * pMI, char* Name );


#ifdef __cplusplus
}
#endif

#endif // Prevent multiple inclusion
/* EOF: MaterialList.h */
