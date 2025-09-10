/****************************************************************************************/
/*  FACELIST.H                                                                          */
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
#ifndef FACELIST_H
#define FACELIST_H

#include "jeList.h"
#include "jeBrush.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef List FaceList ;
typedef ListIterator FaceIterator ;
typedef List_DestroyCallback FaceList_DestroyCallback ;
typedef jeBoolean (*FaceListCB)( jeBrush_Face *pFace, void * pVoid ) ;

FaceList *		FaceList_Create( void ) ;
void			FaceList_Destroy( FaceList **ppList, FaceList_DestroyCallback DestroyFcn ) ;

// ACCESSORS
jeBrush_Face *	FaceList_GetFace( FaceIterator pMI ) ;
jeBrush_Face *	FaceList_GetFirstFace( FaceList * pList, FaceIterator * pMI ) ;
jeBrush_Face *	FaceList_GetNextFace( FaceList * pList, FaceIterator * pMI ) ;
int32			FaceList_GetNumFace( FaceList * pList );


// MODIFIERS
FaceIterator	FaceList_Append( FaceList * pList, jeBrush_Face * pFace ) ;
void			FaceList_Remove( FaceList * pList, jeBrush_Face * pFace ) ;

//SEARCH

jeBoolean FaceList_Search( FaceList * pList, jeBrush_Face*  pFace, FaceIterator * fI );
//ITERATION
int FaceList_Enum(FaceList * pList, FaceListCB CallbackFcn, void *lParam);
#ifdef __cplusplus
}
#endif

#endif // Prevent multiple inclusion
/* EOF: FaceList.h */
