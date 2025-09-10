/****************************************************************************************/
/*  SELECT.H                                                                            */
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

/* Open Source Revision -----------------------------------------------------------------
 By: Dennis Tierney (DJT) dtierney@oneoverz.com
 On: 12/27/99 8:50:01 PM
 Comments: 1) Select_KindsSelected() - Return selection mask.
           2) Select_All() - Select all.
----------------------------------------------------------------------------------------*/

#pragma once

#ifndef SELECT_H
#define SELECT_H

#include "Defs.h"
#include "ExtBox.h"
#include "Jet.h"
#include "Level.h"
#include "jwObject.h"
#include "Ortho.h"
#include "Point.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SELECT_INVALID_NNUMBER	 -1
typedef	enum {
	SELECT_RESULT_NONE,
	SELECT_RESULT_CHANGED,
	SELECT_RESULT_SUBSELECT
} SELECT_RESULT;

jeBoolean		Select_IsCorner( SELECT_HANDLE SelectHandle ) ;
jeBoolean		Select_IsPointOverVertex( const Ortho * pOrtho, const Point * pViewPt, Level * pLevel ) ;

SELECT_RESULT	Select_ClosestThing( Level * pLevel, const Ortho * pOrtho, const Point * pViewPt, LEVEL_STATE eState, jeExtBox * pWorldBounds, MODE eMode, jeBoolean bControl_Held ) ;
jeBoolean		Select_Face(Level * pLevel, const jeCamera * pCamera,  const Point * pViewPt, uint32 *c1, uint32 *c2 ) ;
jeBoolean		Select_CreateModel( Level * pLevel, const char * pszName ) ;
jeBoolean		Select_CreateSelectedUndo( Level * pLevel, UNDO_TYPES Type ) ;
jeBoolean		Select_Delete( Level * pLevel, jeExtBox * pWorldBounds ) ;
jeBoolean		Select_DeselectAll( Level * pLevel, jeExtBox * pWorldBounds ) ;
jeBoolean		Select_DupAndDeselectSelections( Level * pLevel ) ;
jeBoolean		Select_Dup ( Level * pLevel ) ; // Added JH 25.03.2000
jeBoolean		Select_DragBeginSub( Level * pLevel );
jeBoolean		Select_DragEndSub( Level * pLevel );
jeBoolean		Select_DragBegin( Level * pLevel );
jeBoolean		Select_DragEnd( Level * pLevel );
jeBoolean		Select_MoveSelectedVert( Level * pLevel, jeVec3d * dWorldDist, jeExtBox * WorldBounds );
// implemented new version - see below - DJT 
//jeBoolean		Select_HasSelected( Level * pLevel, OBJECT_KIND eKinds ) ;
//
jeBoolean		Select_HasSelectedVerts( Level * pLevel );
jeBoolean		Select_DeselectAllVerts( Level *pLevel );
jeBoolean		Select_DeselectAllFaces( Level *pLevel );
jeBoolean		Select_AllFaces( Level * pLevel );
void			Select_NextFace( Level * pLevel );
void			Select_PrevFace( Level * pLevel );
void			Select_ApplyCurMaterial( Level * pLevel );
jeBoolean		Select_GetEntityField( Level * pLevel, jeSymbol *FieldSymbol, void *pData, int32 DataSize );
void			Select_GetFaceInfo( Level * pLevel, jeFaceInfo *pFaceInfo, int32 *BlankFieldFlag );
void			Select_GetLightInfo( Level * pLevel, LightInfo *LightInfo, int32 *pBlankFieldFlag  );
//	Goes through the selection 
//  If the selection has differet types returns NULL
//	If the selection has same types but different names return NULL
//  If the selection has same type with same name return the name but nNumber set to SELECT_INVALID_NNUMBER
//  If the selctiion has only one thing it returns the name and the nNumber
const char  *	Select_GetName( Level * pLevel, int32 *nNumber );
SELECT_HANDLE	Select_NearestCornerHandle( Ortho * pOrtho, Point * pViewPt, jeExtBox * pWorldBox ) ;
jeBoolean		Select_Rectangle( Level * pLevel, jeExtBox *pSelBox, jeBoolean bSelEncompeses, int32 Mask, jeExtBox *Bounds ); 
void			Select_SetFaceInfo( Level * pLevel, jeFaceInfo *pFaceInfo, int32 BlankFieldFlag );
void			Select_SetLightInfo( Level * pLevel, LightInfo *pLightInfo, int32 BlankFieldFlag );
void			Select_SetEntityField( Level * pLevel, jeSymbol *FieldSymbol, void *pData, int32 DataSize );
void			Select_SetName( Level * pLevel, const char * Name ) ;
jeBoolean		Select_VertsInRectangle( Level * pLevel, jeExtBox *pSelBox, jeBoolean bSelEncompeses, jeExtBox *Bounds ) ;
SELECT_HANDLE	Select_ViewPointHandle( Ortho * pOrtho, Point * pViewPt, jeExtBox * pWorldBox ) ;
const char	*   Select_GetFirstEntityType( Level * pLevel );
jeBoolean		Select_IsEdge( SELECT_HANDLE SelectHandle );
void			Select_GetBrushInfo( Level * pLevel, uint32 *Contents, int32 *pBlankFieldFlag );
void			Select_SetBrushInfo( Level * pLevel, uint32 Contents, int32 FieldFlag );
jeProperty_List * Select_BuildDescriptor( Level * pLevel );
void			Select_SetProperty( int DataId, int DataType, jeProperty_Data * pData );

//---------------------------------------------------
// Added DJT
//---------------------------------------------------
jeBoolean       Select_All(Level * pLevel, int32 Mask, jeExtBox *Bounds);
int32           Select_KindsSelected(Level * pLevel);
//---------------------------------------------------
// End DJT
//---------------------------------------------------


#ifdef __cplusplus
}
#endif

#endif // Prevent multiple inclusion
/* EOF: Select.h */