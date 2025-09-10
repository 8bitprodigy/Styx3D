/****************************************************************************************/
/*  MODELLIST.H                                                                         */
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
#ifndef MODELLIST_H
#define MODELLIST_H

#include "jeWorld.h"
#include "jeList.h"
#include "Model.h"
#include "jePtrMgr.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef jeBoolean (*ModelListCB)( Model *pModel, void * pVoid ) ;

typedef List ModelList ;
typedef ListIterator ModelIterator ;
typedef List_DestroyCallback ModelList_DestroyCallback ;

ModelList *		ModelList_Create( void ) ;
void			ModelList_Destroy( ModelList **ppList, ModelList_DestroyCallback DestroyFcn ) ;

// ACCESSORS
int32			ModelList_GetNumItems( const ModelList * pList ) ;
//Model *			ModelList_GetModel( ModelIterator * pMI ) ;
Model *			ModelList_GetModel( ModelList * pList, ModelIterator * pMI ) ;
Model *			ModelList_GetFirst( ModelList * pList, ModelIterator * pMI ) ;
Model *			ModelList_GetNext( ModelList * pList, ModelIterator * pMI ) ;

// IS
jeBoolean		ModelList_IsModelVisible( ModelIterator pMI ) ;

// MODIFIERS
ModelIterator	ModelList_Append( ModelList * pList, Model * pModel ) ;
void			ModelList_Remove( ModelList * pList, Model * pModel ) ;

// ENUMERATION
int32			ModelList_EnumModels( ModelList * pList, void * pVoid, ModelListCB Callback ) ;
int32			ModelList_EnumBrushes( ModelList * pList, void * pVoid, BrushListCB Callback );
// CALLBACK
jeBoolean		ModelList_NumberModelsCB( Model * pModel, void * lParam ) ;

// FILE HANDLING
ModelList *		ModelList_CreateFromFile( jeVFile * pF, jePtrMgr * pPtrMgr) ;
jeBoolean		ModelList_WriteToFile( ModelList * pList, jeVFile * pF, jePtrMgr * pPtrMgr ) ;

jeBoolean		ModelList_Reattach( ModelList * pList, jeWorld * pWorld ) ;


#ifdef __cplusplus
}
#endif

#endif // Prevent multiple inclusion
/* EOF: ModelList.h */
