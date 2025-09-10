/****************************************************************************************/
/*  MODEL.H                                                                             */
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

#ifndef MODEL_H
#define MODEL_H

#include "BaseType.h"
#include "Brush.h"
#include "jeTypes.h"	// RGBA
#include "jeModel.h"
#include "BrushList.h"
#include "Group.h"
#include "jePtrMgr.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MODEL_REATTACH_GOOD	((uint32)(-1))


typedef struct tagModelReattachInfo
{
	jeModel *	pModel ;
	jeWorld *	pWorld;
	uint32		IndexTag ;
} ModelReattachInfo ;
	
typedef struct tagModel Model ;
typedef int32 ModelID ;

#define MODEL_MAXNAMELENGTH (31) 
#define MODEL_NEXTID		(-1)
#define MODEL_LASTID		(-1)

#define MODEL_VERSION		(1)

Model *			Model_Create( Group * pGroup, const char * const pszName, int32 nNumber ) ;
void			Model_Destroy( Model ** ppModel ) ;
char  *			Model_CreateDefaultName( void );

// ACCESSORS
jeModel *		Model_GetguModel( Model * pModel ) ;
Brush	*		Model_FindBrush( Model * pModel, jeBrush *pgeBrush );
jeBoolean		Model_IsLocked( Model * pModel );

// MODIFIERS
jeBoolean		Model_AddBrush( Model * pModel, Brush * pBrush ) ;
jeBoolean		Model_AddBrushWorld( Model * pModel, Brush * pBrush, jeBoolean bBrushUpdate, jeBoolean bLightUpdate );
void			Model_RemoveBrush( Model * pModel, Brush * pBrush ) ;
void			Model_RemoveBrushWorld( Model * pModel, Brush * pBrush );
void			Model_Move( Model * pModel, const jeVec3d * pWorldDistance ) ;
void			Model_Rotate( Model * pModel, ORTHO_AXIS RAxis, jeFloat RadianAngle, const jeVec3d * pRotationCenter );
void			Model_SetXForm( Model * pModel, const jeXForm3d * XForm );
void			Model_UpdateBounds( Model * pModel ) ;
void			Model_Size( Model * pModel, const jeExtBox * pSelectedBounds, const jeFloat hScale, const jeFloat vScale, SELECT_HANDLE eSizeType, ORTHO_AXIS HAxis, ORTHO_AXIS VAxis ) ;
void			Model_SetLocked( Model * pModel, jeBoolean bLocked );
void			Model_FreeBrushList( Model * pModel );
void			Model_RestoreBrush( Model * pModel);

// ACCESSORS
jeBoolean		Model_GetXForm( const Model * pModel, jeXForm3d * XForm );
jeBoolean		Model_GetWorldAxialBounds( const Model * pModel, jeExtBox *pExtBox ) ;
jeObject	*	Model_GetjeObject( const Model * pModel );
jeProperty_List *	Model_BuildDescriptor( Model * pModel );
BrushList	*	Model_GetBrushList( Model * pModel );

// IS
jeBoolean	Model_IsInRect( const Model * pModel, jeExtBox *pSelRect, jeBoolean bSelEncompeses );

// FILE HANDLING
Model *			Model_CreateFromFile( jeVFile * pF, const int32 nVersion, jePtrMgr * pPtrMgr ) ;
jeBoolean		Model_WriteToFile( Model * pModel, Brush_WriteInfo * pWriteInfo) ;

// CALLBACK
jeBoolean		Model_ReattachCB( Model * pModel, void * lParam ) ;

// ENUMS
int32			Model_EnumBrushes( Model * pModel, void * pVoid, BrushListCB Callback );

// DEBUGGING
#ifdef _DEBUG
jeBoolean		Model_IsValid( const Model * pModel ) ;
#endif ;

#ifdef __cplusplus
}
#endif

#endif // Prevent multiple inclusion
/* EOF: Model.h */
