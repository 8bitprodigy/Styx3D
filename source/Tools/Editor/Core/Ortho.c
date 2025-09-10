/****************************************************************************************/
/*  ORTHO.C                                                                             */
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
#include <Float.h>

#pragma warning(disable : 4201 4214 4115)
#include <Windows.h>
#include <Windowsx.h>
#pragma warning(default : 4201 4214 4115; disable : 4514)

#include "ExtBox.h"
#include "jet.h"
#include "Ram.h"
#include "../Resource.h"
#include "Units.h"
#include "Util.h"

#include "Ortho.h"

#define SIGNATURE						(0xFDB97531)
#define ORTHO_MAX_NAME_LENGTH			(32)
#define ORTHO_MAXPOINTSPERFACE			(64)
#define ORTHO_DEFAULTSELECTTOLERANCE	(64)

typedef struct tagOrtho
{
#ifdef _DEBUG
	int					nSignature ;
#endif
	jeVec3d				Angles ;
	jeVec3d				CamPos ;
	char				szName[ORTHO_MAX_NAME_LENGTH] ;
	
	struct
	{
		BITMAPINFOHEADER	bmiHeader;
		RGBQUAD				bmiColors[256];
	} BMI;

	HBITMAP			hDibSec;
	uint32			Flags;
	uint8		*	pBits;
	uint32			ViewType;
	jeFloat			ZoomFactor;
	int32			nPixelSelectThreshold ;
	jeFloat			fWorldSelectThreshold ;
	jeFloat			fWorldHandleSelectThreshold ;
//	jeVec3d			Vpn, Vright, Vup ;
//	jeFloat			roll, pitch, yaw;
	jePlane			FrustPlanes[4];
	jeFloat			FieldOfView;
	jeFloat			XCenter, YCenter ;
	jeFloat			YScreenScale, XScreenScale ;
	jeFloat			MaxScale ;
	jeFloat			MaxScaleInv ;
	jeFloat			SpeedScale ;
	jeExtBox		WorldBounds ;
	long			Width ;
	long			Height;
} Ortho ;



Ortho * Ortho_Create( void )
{
	Ortho * pOrtho ;
	
	pOrtho = JE_RAM_ALLOCATE_STRUCT( Ortho ) ;
	if( pOrtho != NULL )
	{
		memset( pOrtho, 0, sizeof *pOrtho ) ;	
		assert( (pOrtho->nSignature = SIGNATURE) == SIGNATURE ) ;	// ASSIGN
		pOrtho->nPixelSelectThreshold = ORTHO_DEFAULTSELECTTOLERANCE ;
	}
	pOrtho->ZoomFactor = 1.0f;
	pOrtho->Height = 10;
	pOrtho->Width = 10;
	return pOrtho ;
}// Ortho_Create


void Ortho_Destroy( Ortho ** ppOrtho ) 
{
	assert( ppOrtho != NULL ) ;
	assert( SIGNATURE == (*ppOrtho)->nSignature ) ;

	if( (*ppOrtho)->hDibSec != NULL )
	{
		DeleteObject( (*ppOrtho)->hDibSec ) ;
		(*ppOrtho)->hDibSec = NULL ;
	}
	assert( ((*ppOrtho)->nSignature = 0) == 0 ) ;	// CLEAR

	jeRam_Free( *ppOrtho ) ;
}// Ortho_Destroy

//
// ACCESSORS
//

jeFloat Ortho_GetGridDistance( const Ortho * pOrtho )
{
	jeExtBox	Distance ;
	ORTHO_AXIS	HAxis ;
	float		Extent ;

	// determine grid size for minimum 10 pixels between grid lines
	Ortho_ViewToWorld( pOrtho, 0, 0, &Distance.Min ) ;
	Ortho_ViewToWorld( pOrtho, 10, 0, &Distance.Max ) ;
	HAxis = Ortho_GetHorizontalAxis( pOrtho ) ;
	Extent = Util_geExtBox_GetExtent( &Distance, HAxis ) ;
	if( Extent < 1.0f )
		Extent = 1.0f ;

	return Util_NearestLowerPowerOf2( Extent ) ;

}// Ortho_GetGridDistance


ORTHO_AXIS Ortho_GetHorizontalAxis( const Ortho * pOrtho )
{
	assert( pOrtho != NULL ) ;
	assert( SIGNATURE == pOrtho->nSignature ) ;

	switch( pOrtho->ViewType )
	{
	case Ortho_ViewFront :	return Ortho_Axis_X ;
	case Ortho_ViewSide :	return Ortho_Axis_Z ;
	case Ortho_ViewTop :	return Ortho_Axis_X ;
	}
	assert( 0 ) ;
	return Ortho_Axis_Last ;

}// Ortho_GetHorizontalAxis

ORTHO_AXIS Ortho_GetOrthogonalAxis( const Ortho * pOrtho )
{
	assert( pOrtho != NULL ) ;
	assert( SIGNATURE == pOrtho->nSignature ) ;
	
	switch( pOrtho->ViewType )
	{
	case Ortho_ViewFront :	return Ortho_Axis_Z ;
	case Ortho_ViewSide :	return Ortho_Axis_X ;
	case Ortho_ViewTop :	return Ortho_Axis_Y ;
	}
	assert( 0 ) ;
	return 0 ;
}// Ortho_GetOrthoganalAxis

jeFloat Ortho_GetRotationFromView( const Ortho * pOrtho, Point *pMousePt, Point *pAnchor, Point * pSelCenter )
{
	jeFloat	fRadians ;
	jeVec3d CtoM;
	jeVec3d CtoA;
	jeVec3d Cross;
	jeFloat	fCos;

	assert( pOrtho != NULL ) ;
	assert( SIGNATURE == pOrtho->nSignature ) ;

	jeVec3d_Set( &CtoM,
		(float)(pSelCenter->X - pMousePt->X),
		(float)(pSelCenter->Y - pMousePt->Y),
		0.0f
	);
	jeVec3d_Normalize( &CtoM );

	jeVec3d_Set( &CtoA,
		(float)(pSelCenter->X - pAnchor->X),
		(float)(pSelCenter->Y - pAnchor->Y),
		0.0f
	);
	jeVec3d_Normalize( &CtoA );
	if( jeVec3d_Compare( &CtoA, &CtoM, 0.001f) )
		return( 0.0f );
	fCos = jeVec3d_DotProduct( &CtoA, &CtoM );
	if( fCos == -1.0f )
		fRadians =M_PI;	
	else
		fRadians = (float)acos( fCos );
	jeVec3d_CrossProduct( &CtoA, &CtoM, &Cross);
	if( pOrtho->ViewType != Ortho_ViewSide  )
	{
		if( Cross.Z > 0 )
			fRadians = (2.0f * M_PI) - fRadians;
	}
	else
	if( Cross.Z < 0 )
	{
		fRadians = (2.0f * M_PI) - fRadians;
	}
	assert( fRadians * fRadians >= 0.0f );

	return fRadians ;
}// Ortho_GetRotationFromViewDelta

ORTHO_AXIS Ortho_GetVerticalAxis( const Ortho * pOrtho )
{
	assert( pOrtho != NULL ) ;
	assert( SIGNATURE == pOrtho->nSignature ) ;

	switch( pOrtho->ViewType )
	{
	case Ortho_ViewFront :	return Ortho_Axis_Y ;
	case Ortho_ViewSide :	return Ortho_Axis_Y ;
	case Ortho_ViewTop :	return Ortho_Axis_Z ;
	}
	assert( 0 ) ;
	return Ortho_Axis_Last ;

}// Ortho_GetVerticalAxis

Ortho_ViewType Ortho_GetViewType( const Ortho * pOrtho )
{
	assert( pOrtho != NULL ) ;
	assert( SIGNATURE == pOrtho->nSignature ) ;

	return	pOrtho->ViewType ;
}// Ortho_GetViewType

long Ortho_GetWidth( const Ortho * pOrtho )
{
	assert( pOrtho != NULL ) ;
	assert( SIGNATURE == pOrtho->nSignature ) ;

	return pOrtho->Width ;
}// Ortho_GetWidth

long Ortho_GetHeight( const Ortho * pOrtho )
{
	assert( pOrtho != NULL ) ;
	assert( SIGNATURE == pOrtho->nSignature ) ;

	return pOrtho->Height ;
}// Ortho_GetHeight

const char * Ortho_GetName( const Ortho * pOrtho )
{
	assert( pOrtho != NULL ) ;
	assert( SIGNATURE == pOrtho->nSignature ) ;

	return pOrtho->szName ;
}// Ortho_GetName


int32 Ortho_GetViewSelectThreshold( Ortho * pOrtho )
{
	assert( pOrtho != NULL ) ;
	assert( SIGNATURE == pOrtho->nSignature ) ;

	return pOrtho->nPixelSelectThreshold ;
}// Ortho_GetViewSelectThreshold

jeFloat Ortho_GetWorldSelectThreshold( const Ortho * pOrtho )
{
	assert( pOrtho != NULL ) ;
	assert( SIGNATURE == pOrtho->nSignature ) ;

	return pOrtho->fWorldSelectThreshold ;
}// Ortho_GetWorldSelectThreshold

jeFloat Ortho_GetWorldHandleSelectThreshold( const Ortho * pOrtho )
{
	assert( pOrtho != NULL ) ;
	assert( SIGNATURE == pOrtho->nSignature ) ;

	return pOrtho->fWorldHandleSelectThreshold ;
}// Ortho_GetWorldHandleSelectThreshold

//
// IS
//
jeBoolean Ortho_IsViewPointInWorldBox( const Ortho * pOrtho, const int x, const int y, const jeExtBox * pWorldBox )
{
	jeExtBox	Box ;
	jeVec3d		World ;
	ORTHO_AXIS	OrthoAxis ;

	assert( pOrtho != NULL ) ;
	assert( SIGNATURE == pOrtho->nSignature ) ;
	assert( pWorldBox != NULL ) ;

	Box = *pWorldBox ;
	Ortho_ViewToWorld( pOrtho, x, y, &World ) ;
	
	// The remaining axis is set to extremes
	OrthoAxis = Ortho_GetOrthogonalAxis( pOrtho ) ;
	jeVec3d_SetElement( &Box.Min, OrthoAxis, -FLT_MAX ) ;
	jeVec3d_SetElement( &Box.Max, OrthoAxis, FLT_MAX ) ;

	return jeExtBox_ContainsPoint( &Box, &World ) ;

}// Ortho_IsViewPointInWorldBox

//
// MODIFIERS
//
void Ortho_MoveCamera( Ortho * pOrtho, const jeVec3d * pDelta )
{
	assert( pOrtho != NULL ) ;
	assert( SIGNATURE == pOrtho->nSignature ) ;

	jeVec3d_Add( &pOrtho->CamPos, pDelta, &pOrtho->CamPos ) ;
	Ortho_UpdateWorldBounds( pOrtho );

}// Ortho_MoveCamera

void Ortho_ResetSettings( Ortho * pOrtho, long vx, long vy )
{
	assert( pOrtho != NULL ) ;
	assert( SIGNATURE == pOrtho->nSignature ) ;
	
	Ortho_ResizeView( pOrtho, vx, vy ) ;

	// Compute and set zoom factor
	Ortho_SetZoom( pOrtho, pOrtho->Width / 640.0f ) ;
	Ortho_SetAnglesRPY( pOrtho, 0.0f, M_PI, 0.0f ) ;
	jeVec3d_Clear( &pOrtho->CamPos ) ;

#if 0
	jeVec3d_Clear (&v->Vpn);
	jeVec3d_Clear (&v->Vright);
	jeVec3d_Clear (&v->Vup);

	v->roll = 0.0f;
	v->pitch	=M_PI;
	v->yaw	=0.0f;

	mRoll[0][0]=1;  mRoll[0][1]=0;  mRoll[0][2]=0;
	mRoll[1][0]=0;  mRoll[1][1]=1;  mRoll[1][2]=0;
	mRoll[2][0]=0;	mRoll[2][1]=0;  mRoll[2][2]=1;
	mPitch[0][0]=1; mPitch[0][1]=0;	mPitch[0][2]=0;
	mPitch[1][0]=0; mPitch[1][1]=1; mPitch[1][2]=0;
	mPitch[2][0]=0; mPitch[2][1]=0; mPitch[2][2]=1;
	mYaw[0][0]=1;	mYaw[0][1]=0;	mYaw[0][2]=0;
	mYaw[1][0]=0;	mYaw[1][1]=1;	mYaw[1][2]=0;
	mYaw[2][0]=0;	mYaw[2][1]=0;	mYaw[2][2]=1;

	jeVec3d_Clear (&v->CamPos);
#endif

	Ortho_UpdateWorldBounds( pOrtho ) ;

}// Ortho_ResetSettings

void Ortho_ResizeView( Ortho * pOrtho, long vx, long vy )
{
	HDC		ViewDC;
	assert( pOrtho != NULL ) ;
	assert( SIGNATURE == pOrtho->nSignature ) ;

	vx=(vx+3)&~3;	//Align scan delta

	if( vx && vy )
	{
		if( pOrtho->hDibSec != NULL )
		{
			DeleteObject( pOrtho->hDibSec ) ;
			pOrtho->hDibSec = NULL ;
		}

		//Force top-down 8-bit bitmap of size WINDOW_WIDTH*WINDOW_HEIGHT.
		pOrtho->BMI.bmiHeader.biSize         = sizeof(BITMAPINFOHEADER);
		pOrtho->BMI.bmiHeader.biPlanes       = 1;
		pOrtho->BMI.bmiHeader.biBitCount     = 16;
		pOrtho->BMI.bmiHeader.biCompression  = BI_RGB;
		pOrtho->BMI.bmiHeader.biSizeImage    = 0;
		pOrtho->BMI.bmiHeader.biClrUsed      = 0;
		pOrtho->BMI.bmiHeader.biClrImportant = 0;
		pOrtho->BMI.bmiHeader.biWidth        = vx;
		pOrtho->BMI.bmiHeader.biHeight       = -vy;    // Minus for top-down.

		ViewDC = CreateCompatibleDC( NULL ) ;
		assert(ViewDC);

		pOrtho->hDibSec = CreateDIBSection
		(
			ViewDC, 
			(BITMAPINFO *)&pOrtho->BMI, 
			DIB_RGB_COLORS, 
			(void **)&pOrtho->pBits, 
			NULL, 
			0
		);
		assert( pOrtho->hDibSec ) ;

		DeleteDC( ViewDC ) ;

//		//allocate a 32 bit zbuffer
//		v->pZBuffer	=(uint32 *)	jeRam_Allocate(sizeof(uint32) * (vx*vy));
	}

	pOrtho->FieldOfView		= 2.0f;	//fixed for now?
	pOrtho->XScreenScale	= ((jeFloat)vx) / pOrtho->FieldOfView ;
	pOrtho->YScreenScale	= ((jeFloat)vy) / pOrtho->FieldOfView ;
	pOrtho->MaxScale		= max( pOrtho->XScreenScale, pOrtho->YScreenScale ) ;
	pOrtho->MaxScaleInv		= 1.0f / pOrtho->MaxScale ;
	pOrtho->XCenter			= ((jeFloat)vx) / 2.0f - 0.5f ;
	pOrtho->YCenter			= ((jeFloat)vy) / 2.0f - 0.5f ;
	pOrtho->Width			= vx;
	pOrtho->Height			= vy;
#if 0
	if(v->ViewType < VIEWTOP)
	{
		if(v->NewEdges)
			jeRam_Free (v->NewEdges);
		if(v->RemoveEdges)
			jeRam_Free (v->RemoveEdges);

		v->NewEdges			=(Edge *)jeRam_Allocate(vy*sizeof(Edge));
		v->RemoveEdges		=(Edge **)jeRam_Allocate(vy*sizeof(Edge *));
	}
#endif
}// Ortho_ResizeView

void Ortho_SetAngles( Ortho * pOrtho, const jeVec3d * pAngles )
{
	assert( pOrtho != NULL ) ;
	assert( SIGNATURE == pOrtho->nSignature ) ;

	pOrtho->Angles = *pAngles ;
}// Ortho_SetAngles

void Ortho_SetAnglesRPY( Ortho * pOrtho, jeFloat roll, jeFloat pitch, jeFloat yaw )
{
	assert( pOrtho != NULL ) ;
	assert( SIGNATURE == pOrtho->nSignature ) ;

	pOrtho->Angles.X = roll ;
	pOrtho->Angles.Y = pitch ;
	pOrtho->Angles.Z = yaw ;

}// Ortho_SetAnglesRPY

void Ortho_SetBoxOrthogonalToMax( const Ortho * pOrtho, jeExtBox * pBox )
{
	ORTHO_AXIS	OrthoAxis ;
	assert( pOrtho != NULL ) ;
	assert( SIGNATURE == pOrtho->nSignature ) ;
	assert( pBox != NULL ) ;


	OrthoAxis = Ortho_GetOrthogonalAxis( pOrtho ) ;
	jeVec3d_SetElement( &pBox->Min, OrthoAxis, -FLT_MAX ) ;
	jeVec3d_SetElement( &pBox->Max, OrthoAxis, FLT_MAX ) ;

}//Ortho_SetBoxOrthogonalToMax

void Ortho_SetCameraPos( Ortho * pOrtho, const jeVec3d * pPos )
{
	assert( pOrtho != NULL ) ;
	assert( SIGNATURE == pOrtho->nSignature ) ;

	pOrtho->CamPos = *pPos ;
	Ortho_UpdateWorldBounds( pOrtho ) ;
}// Ortho_SetCameraPos

void Ortho_SetSelectThreshold( Ortho * pOrtho, const int nPixels )
{
	assert( pOrtho != NULL ) ;
	assert( SIGNATURE == pOrtho->nSignature ) ;

	pOrtho->nPixelSelectThreshold = nPixels ;
}// Ortho_SetSelectThreshold

void Ortho_SetViewType( Ortho * pOrtho, const Ortho_ViewType vt )
{
	int	nID ;
	assert( pOrtho != NULL ) ;
	assert( SIGNATURE == pOrtho->nSignature ) ;

	pOrtho->ViewType = vt ;
	nID = IDS_FRONT ;
	switch( vt )
	{
	case Ortho_ViewFront :	nID = IDS_FRONT ;	break ;
	case Ortho_ViewSide :	nID = IDS_SIDE ;	break ;
	case Ortho_ViewTop :	nID = IDS_TOP ;		break ;
	}
	Util_GetRcString( pOrtho->szName, nID ) ;

}// Ortho_SetViewType

void Ortho_SetZoom( Ortho * pOrtho, const jeFloat zf )
{
	assert( pOrtho != NULL ) ;
	assert( SIGNATURE == pOrtho->nSignature ) ;

	pOrtho->ZoomFactor = zf ;

	pOrtho->fWorldSelectThreshold = (float)pOrtho->nPixelSelectThreshold  ; 
	pOrtho->fWorldHandleSelectThreshold = HANDLESIZE  ; 
	Ortho_UpdateWorldBounds( pOrtho ) ;
}// Ortho_SetZoom

void Ortho_UpdateWorldBounds( Ortho * pOrtho )
{
	ORTHO_AXIS	OrthoAxis ;
	assert( pOrtho != NULL ) ;
	assert( SIGNATURE == pOrtho->nSignature ) ;

	switch( pOrtho->ViewType )
	{
		case Ortho_ViewTop :
			Ortho_ViewToWorld( pOrtho, 0, 0, &pOrtho->WorldBounds.Min ) ;
			Ortho_ViewToWorld( pOrtho, pOrtho->Width-1, pOrtho->Height-1, &pOrtho->WorldBounds.Max ) ;
			break;

		case Ortho_ViewFront :
			Ortho_ViewToWorld( pOrtho, 0, pOrtho->Height-1, &pOrtho->WorldBounds.Min ) ;
			Ortho_ViewToWorld( pOrtho, pOrtho->Width-1, 0, &pOrtho->WorldBounds.Max ) ;
			break;

		case Ortho_ViewSide :
			Ortho_ViewToWorld( pOrtho, 0, pOrtho->Height-1, &pOrtho->WorldBounds.Min ) ;
			Ortho_ViewToWorld( pOrtho, pOrtho->Width-1, 0, &pOrtho->WorldBounds.Max ) ;
			break ;
	}

	OrthoAxis = Ortho_GetOrthogonalAxis( pOrtho ) ;		// Remaing access set to extremes
	jeVec3d_SetElement( &pOrtho->WorldBounds.Min, OrthoAxis, -FLT_MAX ) ;
	jeVec3d_SetElement( &pOrtho->WorldBounds.Max, OrthoAxis, FLT_MAX ) ;

}// Ortho_UpdateWorldBounds


void Ortho_ZoomChange( Ortho * pOrtho, const jeFloat fFactor )
{
	jeFloat fNewZoom;
	jeFloat fDist;

	assert( pOrtho != NULL ) ;
	assert( SIGNATURE == pOrtho->nSignature ) ;

	fNewZoom = pOrtho->ZoomFactor * (1.0f + fFactor ) ;

	fDist = fNewZoom * (jeFloat)pOrtho->Width ;
	if( ((fDist < 1.0f) && (fNewZoom < pOrtho->ZoomFactor )) ||
		((fDist > 100000.0f) && (fNewZoom > pOrtho->ZoomFactor )) )
	{
		// either way too small or way too big,
		// and trying to make it worse
		MessageBeep( (UINT)-1 ) ;
	}
	else
	{
		Ortho_SetZoom( pOrtho, fNewZoom ) ;
	}
}// Ortho_ZoomChange

//
// COORDINATES AND TRANSLATION
//

// Return world position at center of view
void Ortho_GetViewCenter( const Ortho * pOrtho, jeVec3d * pCenter )
{
	jeVec3d TopLeft;
	jeVec3d BottomRight;

	assert( pOrtho != NULL ) ;
	assert( SIGNATURE == pOrtho->nSignature ) ;

	Ortho_ViewToWorld( pOrtho, 0, 0, &TopLeft ) ;
	Ortho_ViewToWorld( pOrtho, pOrtho->Width-1, pOrtho->Height-1, &BottomRight ) ;
	jeVec3d_Add( &TopLeft, &BottomRight, pCenter );
	jeVec3d_Scale( pCenter, 0.5f, pCenter ) ;
}// Ortho_GetViewCenter


void Ortho_ViewToWorld( const Ortho * pOrtho, const int x, const int y, jeVec3d *pW )
/*
  XY view coordinate transformed to world coordinate, depending on view.

    Mouse Coordinates
    -----------------

		   |
		   |
 		   |
	-------+-------> +X
		   |
		   |
		   |
		   +Y


        Top View				  Front View				   Side View
  	    --------				  ----------				   ---------
									   +Y						   +Y
		   |						   |						   |
		   |						   |						   |
		   |						   |						   |
	-------+-------> +X			-------+-------> +X			-------+-------> +Z
		   |						   |						   |
		   |						   |						   |
		   |						   |						   |
  		   +Z
*/
{
	jeFloat	ZoomInv= 1.0f / pOrtho->ZoomFactor ;

	assert( pOrtho != NULL ) ;
	assert( SIGNATURE == pOrtho->nSignature ) ;

	switch( pOrtho->ViewType )
	{
		case Ortho_ViewTop :
			jeVec3d_Set( pW, (x - pOrtho->XCenter), 0.0f, (y - pOrtho->YCenter)) ;
			jeVec3d_Scale( pW, ZoomInv, pW ) ;
			jeVec3d_Add( pW, &pOrtho->CamPos, pW ) ;
			break;

		case Ortho_ViewFront :
			jeVec3d_Set( pW, (x - pOrtho->XCenter), -(y - pOrtho->YCenter), 0.0f ) ;
			jeVec3d_Scale( pW, ZoomInv, pW ) ;
			jeVec3d_Add( pW, &pOrtho->CamPos, pW ) ;
			break;

		case Ortho_ViewSide :
			jeVec3d_Set( pW, 0.0f, -(y - pOrtho->YCenter), (x - pOrtho->XCenter) ) ;
			jeVec3d_Scale( pW, ZoomInv, pW ) ;
			jeVec3d_Add( pW, &pOrtho->CamPos, pW ) ;
			break;
#if 0
		default :
		{
			jeVec3d_Set 
			(
				wp,
				-(x -v->XCenter)*(v->MaxScreenScaleInv), 
				-(y -v->YCenter)*(v->MaxScreenScaleInv), 
				1.0f
			);
			jeVec3d_Normalize(wp);
			break;
		}
#endif // 3D
	}
}// Ortho_ViewToWorld

void Ortho_ViewToWorldRect( const Ortho * pOrtho, const Point * pV1, const Point * pV2, jeExtBox * pWorldBox )
{
	jeVec3d	Vec1 ;
	jeVec3d Vec2 ;
	assert( pOrtho != NULL ) ;
	assert( SIGNATURE == pOrtho->nSignature ) ;
	assert( pV1 != NULL ) ;
	assert( pV2 != NULL ) ;
	assert( pWorldBox != NULL ) ;
	
	Ortho_ViewToWorld( pOrtho, pV1->X, pV1->Y, &Vec1 ) ;
	Ortho_ViewToWorld( pOrtho, pV2->X, pV2->Y, &Vec2 ) ;
	Util_geExtBox_InitFromTwoPoints( pWorldBox, &Vec1, &Vec2 ) ;
	Ortho_SetBoxOrthogonalToMax( pOrtho, pWorldBox ) ;

}// Ortho_ViewToWorldRect

void Ortho_ViewToWorldDistance( const Ortho * pOrtho, const int x, const int y, jeVec3d *pW )
{
	jeFloat	ZoomInv= 1.0f / pOrtho->ZoomFactor ;
	assert( pOrtho != NULL ) ;
	assert( SIGNATURE == pOrtho->nSignature ) ;

	switch( pOrtho->ViewType )
	{
		case Ortho_ViewTop :
			jeVec3d_Set( pW, (jeFloat)x, 0.0f, (jeFloat)y ) ;
			jeVec3d_Scale( pW, ZoomInv, pW ) ;
			break;

		case Ortho_ViewFront :
			jeVec3d_Set( pW, (jeFloat)x, (jeFloat)-y, 0.0f ) ;
			jeVec3d_Scale( pW, ZoomInv, pW ) ;
			break;

		case Ortho_ViewSide :
			jeVec3d_Set( pW, 0.0f, (jeFloat)-y, (jeFloat)x ) ;
			jeVec3d_Scale( pW, ZoomInv, pW ) ;
			break;
	}
}// Ortho_ViewToWorldDistance


void Ortho_WorldToView( const Ortho * pOrtho, const jeVec3d * pW, Point * pPt )
{
	jeVec3d ptView;
	assert( pOrtho != NULL ) ;
	assert( SIGNATURE == pOrtho->nSignature ) ;
	assert( pW != NULL ) ;
	assert( pPt != NULL ) ;

	switch( pOrtho->ViewType )
	{
		case Ortho_ViewTop :
			jeVec3d_Subtract( pW, &pOrtho->CamPos, &ptView ) ;
			jeVec3d_Scale( &ptView, pOrtho->ZoomFactor, &ptView ) ;
			pPt->X = (int32)(pOrtho->XCenter + ptView.X ) ;
			pPt->Y = (int32)(pOrtho->YCenter + ptView.Z ) ;
			break;

		case Ortho_ViewFront :
			jeVec3d_Subtract( pW, &pOrtho->CamPos, &ptView ) ;
			jeVec3d_Scale( &ptView, pOrtho->ZoomFactor, &ptView ) ;
			pPt->X = (int32)(pOrtho->XCenter + ptView.X ) ;
			pPt->Y = (int32)(pOrtho->YCenter - ptView.Y ) ;
			break;

		case Ortho_ViewSide :
			jeVec3d_Subtract( pW, &pOrtho->CamPos, &ptView ) ;
			jeVec3d_Scale( &ptView, pOrtho->ZoomFactor, &ptView ) ;
			pPt->X = (int32)(pOrtho->XCenter + ptView.Z ) ;
			pPt->Y = (int32)(pOrtho->YCenter - ptView.Y ) ;
			break;
	}
}// Ortho_WorldToView


void Ortho_WorldToViewRect( const Ortho * pOrtho, const jeExtBox * pWorldBox, Rect * pViewRect )
{
	assert( pOrtho != NULL ) ;
	assert( SIGNATURE == pOrtho->nSignature ) ;
	assert( pWorldBox != NULL ) ;
	assert( pViewRect != NULL ) ;

	Ortho_WorldToView( pOrtho, &pWorldBox->Min, &pViewRect->TopLeft ) ;
	Ortho_WorldToView( pOrtho, &pWorldBox->Max, &pViewRect->BotRight ) ;
	Rect_Normalize( pViewRect ) ;
	if( pViewRect->Bottom == pViewRect->Top )
		pViewRect->Bottom++;
	if( pViewRect->Left == pViewRect->Right )
		pViewRect->Right++;

}// Ortho_WorldToViewRect

jeBoolean Ortho_TestWorldToViewRect( const Ortho * pOrtho, const jeExtBox * pWorldBox, Rect * pViewRect )
{
	jeBoolean	bIntersects ;
	jeExtBox	IntersectBounds ;
	assert( pOrtho != NULL ) ;
	assert( SIGNATURE == pOrtho->nSignature ) ;
	assert( pWorldBox != NULL ) ;
	assert( pViewRect != NULL ) ;
	
	bIntersects = Util_geExtBox_Intersection( &pOrtho->WorldBounds, pWorldBox, &IntersectBounds ) ;
	if( bIntersects )
	{
		Ortho_WorldToViewRect( pOrtho, &IntersectBounds, pViewRect ) ;
	}
	return bIntersects ;
}// Ortho_TestWorldToViewRect



/* EOF: Ortho.c */
