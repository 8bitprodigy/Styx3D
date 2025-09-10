/****************************************************************************************/
/*  DRAW3D.C                                                                            */
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

#include "Ram.h"

#include "Draw3d.h"
#include "TerrnObj.h"

typedef struct tagBrushDrawInfo
{
	jeWorld		*	pWorld ;
	jeCamera	*	pCamera ;
	jeEngine	*	pEngine ;
} BrushDragInfo ;



static jeBoolean Draw3d_BrushDragDraw( Object * pObject, void * lParam )
{
	BrushDragInfo	*	pbdi = (BrushDragInfo*)lParam ;
	Brush			*	pBrush ;
	OBJECT_KIND			Kind ;
//	static uint32		UID;
#pragma message ("What does this UID do?")

	assert( pObject != NULL ) ;
	
	Kind = Object_GetKind( pObject ) ;
	switch( Kind )
	{
	case KIND_BRUSH :
		pBrush = (Brush*)pObject ;
		if( JE_FALSE == Brush_IsInModel( pBrush ) )
		{
			jeBrush_Render( Brush_GetjeBrush( pBrush ), pbdi->pEngine, pbdi->pCamera ) ;
		}
		break ;

	case KIND_LIGHT:
	case KIND_CAMERA:
	case KIND_ENTITY:
	case KIND_USEROBJ:
		break;


	case KIND_TERRAIN:
/*
		jeTerrain_RenderPrep(Terrain_GetTerrain((Terrain*)pObject), pbdi->pCamera);
		jeTerrain_RenderThroughCamera( Terrain_GetTerrain((Terrain*)pObject), pbdi->pEngine, pbdi->pCamera, UID);
		UID++;
*/
		break;

	case KIND_MODEL:
		break;

	case KIND_CLASS:
		break;

	default:
		assert( 0 );
	}// switch
	return JE_TRUE ;
}// Draw3d_BrushDragDraw

void Draw3d_ManipulatedBrushes( Level * pLevel, jeWorld* pWorld, jeCamera* pCamera, jeEngine* pEngine )
{
	BrushDragInfo	bdi ;
	assert( pLevel != NULL ) ;
	assert( pWorld != NULL ) ;
	assert( pCamera != NULL ) ;
	assert( pEngine != NULL ) ;

	bdi.pWorld	= pWorld ;
	bdi.pEngine = pEngine ;
	bdi.pCamera = pCamera ;

	Level_EnumSelected( pLevel, &bdi, Draw3d_BrushDragDraw ) ;
}// Draw3d_ManipulatedBrushes


/* EOF: Draw3d.c */