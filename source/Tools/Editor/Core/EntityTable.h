/****************************************************************************************/
/*  ENTITYTABLE.H                                                                       */
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

#ifndef ENTITYTABLE_H
#define ENTITYTABLE_H

// NOTE: EntityTable is "wrap" and utility functions for Symbol.h
#include "Symbol.h"
#include "Entity.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef jeBoolean (*EntityTable_ForEachCallback)(jeSymbol *pSymbol, void *lParam);

jeSymbol_Table *	EntityTable_Create( void ) ;
void				EntityTable_Destroy( jeSymbol_Table ** ppSymbols ) ;

jeSymbol *			EntityTable_AddEntity( jeSymbol_Table * pST, const char * pszType, const char * pszName ) ;
jeBoolean			EntityTable_AddField( jeSymbol_Table * pSymbols, jeSymbol * pTypeSym, const char *Name, jeSymbol_Type Type, void *DefaultValue ) ;
jeBoolean			EntityTable_AddFieldToInstances( jeSymbol_Table * pSymbols, jeSymbol * pDef, const char * pszName, jeSymbol_Type Type, void * DefaultValue ) ;
jeSymbol *			EntityTable_CopyEntity( jeSymbol_Table * pST, jeSymbol * pEntity, const char * pszName ) ;
jeSymbol *			EntityTable_CreateType( jeSymbol_Table * pSymbols, const char * pszName ) ;
jeBoolean			EntityTable_EnumDefinitions( jeSymbol_Table * pSymbols, void * pVoid, EntityTable_ForEachCallback Callback ) ;
jeBoolean			EntityTable_EnumFields( jeSymbol_Table * pST, const char * pszType, void * pVoid, EntityTable_ForEachCallback Callback ) ;
jeSymbol *			EntityTable_FindSymbol( jeSymbol_Table * pST, const char * pszType, const char * pszName ) ;
jeSymbol *			EntityTable_GetField( jeSymbol_Table * pST, jeSymbol * pEntity, const char * pszName ) ;
jeBoolean			EntityTable_InitDefault( jeSymbol_Table * pSymbols ) ;
int32				EntityTable_ListGetNumItems( jeSymbol_List * pList ) ;
void				EntityTable_RemoveDefaultEntityField( jeSymbol_Table * pST, jeSymbol * pSymbol ) ;
void				EntityTable_RemoveEntityAndInstances( jeSymbol_Table * pST, jeSymbol * pEntityDef ) ;
jeBoolean			EntityTable_SetDefaultValue( jeSymbol_Table *pST, jeSymbol *pFieldSym, void *DefaultValue ) ;

#ifdef __cplusplus
}
#endif

#endif // Prevent multiple inclusion
/* EOF: EntityTable.h */