/****************************************************************************************/
/*  BRUSHLIST.H                                                                         */
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

#ifndef BRUSHLIST_H
#define BRUSHLIST_H

#include "Brush.h"
#include "jeWorld.h"
#include "jeList.h"
#include "VFile.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef jeBoolean (*BrushListCB)( Brush *pBrush, void * pVoid ) ;

typedef List BrushList ;
typedef ListIterator BrushIterator ;
typedef List_DestroyCallback BrushList_DestroyCallback ;

BrushList *		BrushList_Create( void ) ;
void			BrushList_Destroy( BrushList **ppList, BrushList_DestroyCallback DestroyFcn ) ;

// ACCESSORS
Brush *			BrushList_GetFirst( BrushList * pList, BrushIterator * pInterator ) ;
Brush *			BrushList_GetNext( BrushList * pList, BrushIterator * pInterator ) ;
int32			BrushList_GetNumItems( const BrushList * pList ) ;
BrushIterator	BrushList_Find( BrushList * pList, Brush * pBrush ) ;
Brush *			BrushList_FindByGeBrush( BrushList * pList, BrushIterator *Interator, jeBrush * pgeBrush );
void			BrushList_GetWorldBounds( BrushList * pList, jeExtBox * pWorldBounds ) ;

// IS
//jeBoolean		BrushList_IsVisible( BrushIterator pGI ) ;

// MODIFIERS
BrushIterator		BrushList_Append( BrushList * pList, Brush * pBrush ) ;
jeBoolean			BrushList_AppendNoDup( BrushList * pList, Brush * pBrush ) ;
void				BrushList_ClearMiscFlags( BrushList * pList, const uint32 nFlags ) ;
//					Remove does not _Destroy the brush
void				BrushList_Remove( BrushList * pList, Brush * pBrush ) ;
void				BrushList_SetMiscFlags( BrushList * pList, const uint32 nFlags ) ;

// ENUMERATION
int32				BrushList_EnumBrushes( BrushList * pList, void * pVoid, BrushListCB Callback ) ;

// FILE HANDLING
jeBoolean			BrushList_WriteToFile( BrushList * pList, Brush_WriteInfo * pWriteInfo ) ;
BrushList *			BrushList_CreateFromFile( jeVFile * pF, jePtrMgr * pPtrMgr ) ;

jeBoolean			BrushList_Reattach( BrushList * pList, Model * pModel, jeWorld * pWorld ) ;

#ifdef __cplusplus
}
#endif






#endif // Prevent multiple inclusion
/* EOF: BrushList.h */