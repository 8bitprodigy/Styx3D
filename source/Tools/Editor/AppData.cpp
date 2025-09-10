/****************************************************************************************/
/*  APPDATA.CPP                                                                         */
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
// This module holds application data that can be treated as "Global" but is
// actually held by CJweApp
#include <Assert.h>
//	by trilobite jan. 2011
#include "stdafx.h"
//
#include <Windowsx.h>

//	by trilobite jan. 2011
//#include "stdafx.h"
//
#include "jwe.h"
#include "Ram.h"

#include "AppData.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

typedef struct tagAppData
{
	HBITMAP	hBMHandleCorner ;
	HBITMAP hBMHandleEdge ;
	HBITMAP hBMRotateTL ;
	HBITMAP hBMRotateTR ;
	HBITMAP hBMRotateBL ;
	HBITMAP hBMRotateBR ;
	HBITMAP hBMShearLR ;
	HBITMAP hBMShearTB ;
	HBITMAP hBMVertex ;
	HBITMAP hBMSelectedVertex ;
} AppData ;


AppData * AppData_Create( void )
{
	AppData *	pAppData ;
	HINSTANCE	hRes{} ;
	
	pAppData = JE_RAM_ALLOCATE_STRUCT( AppData ) ;
	if( pAppData == nullptr )
		goto ADC_FAILURE ;

	hRes = AfxGetResourceHandle() ;
	memset( pAppData, 0, sizeof *pAppData ) ;
	pAppData->hBMHandleCorner = ::LoadBitmap( hRes, MAKEINTRESOURCE(IDB_HANDLECORNER) ) ;
	if( pAppData->hBMHandleCorner == nullptr )
		goto ADC_FAILURE ;

	pAppData->hBMHandleEdge = ::LoadBitmap( hRes, MAKEINTRESOURCE(IDB_HANDLEEDGE) ) ;
	if( pAppData->hBMHandleEdge == nullptr )
		goto ADC_FAILURE ;

	pAppData->hBMRotateTL = ::LoadBitmap( hRes, MAKEINTRESOURCE( IDB_ROTATEHANDLETL ) ) ;
	if( pAppData->hBMRotateTL == nullptr )
		goto ADC_FAILURE ;

	pAppData->hBMRotateTR = ::LoadBitmap( hRes, MAKEINTRESOURCE( IDB_ROTATEHANDLETR ) ) ;
	if( pAppData->hBMRotateTR == nullptr )
		goto ADC_FAILURE ;

	pAppData->hBMRotateBL = ::LoadBitmap( hRes, MAKEINTRESOURCE( IDB_ROTATEHANDLEBL ) ) ;
	if( pAppData->hBMRotateBL == nullptr )
		goto ADC_FAILURE ;

	pAppData->hBMRotateBR = ::LoadBitmap( hRes, MAKEINTRESOURCE( IDB_ROTATEHANDLEBR ) ) ;
	if( pAppData->hBMRotateBR == nullptr )
		goto ADC_FAILURE ;

	pAppData->hBMShearLR = ::LoadBitmap( hRes, MAKEINTRESOURCE( IDB_SHEARHANDLELR ) ) ;
	if( pAppData->hBMShearLR == nullptr )
		goto ADC_FAILURE ;

	pAppData->hBMShearTB = ::LoadBitmap( hRes, MAKEINTRESOURCE( IDB_SHEARHANDLETB ) ) ;
	if( pAppData->hBMShearTB == nullptr )
		goto ADC_FAILURE ;

	pAppData->hBMVertex = ::LoadBitmap( hRes, MAKEINTRESOURCE( IDB_VERTEX ) ) ;
	if( pAppData->hBMVertex == nullptr )
		goto ADC_FAILURE ;

	pAppData->hBMSelectedVertex = ::LoadBitmap( hRes, MAKEINTRESOURCE( IDB_SELVERTEX ) ) ;
	if( pAppData->hBMSelectedVertex == nullptr )
		goto ADC_FAILURE ;

	return pAppData ;

ADC_FAILURE :
	if( pAppData != nullptr )
		AppData_Destroy( &pAppData ) ;

	return nullptr ;

}// AppData_Create

void AppData_Destroy( AppData ** ppAppData )
{
	AppData * pAppData{} ;
	assert( ppAppData != nullptr ) ;
	pAppData = *ppAppData ;
	assert( pAppData != nullptr ) ;

	if( pAppData->hBMHandleCorner != nullptr )
		DeleteBitmap( pAppData->hBMHandleCorner ) ;

	if( pAppData->hBMHandleEdge != nullptr )
		DeleteBitmap( pAppData->hBMHandleEdge ) ;

	if( pAppData->hBMRotateBL != nullptr )
		DeleteBitmap( pAppData->hBMRotateBL ) ;

	if( pAppData->hBMRotateBR != nullptr )
		DeleteBitmap( pAppData->hBMRotateBR ) ;

	if( pAppData->hBMRotateTL != nullptr )
		DeleteBitmap( pAppData->hBMRotateTL ) ;

	if( pAppData->hBMRotateTR != nullptr )
		DeleteBitmap( pAppData->hBMRotateTR ) ;

	if( pAppData->hBMShearLR != nullptr )
		DeleteBitmap( pAppData->hBMShearLR ) ;

	if( pAppData->hBMShearTB != nullptr )
		DeleteBitmap( pAppData->hBMShearTB ) ;

	if( pAppData->hBMVertex != nullptr )
		DeleteBitmap( pAppData->hBMVertex ) ;

	if( pAppData->hBMSelectedVertex != nullptr )
		DeleteBitmap( pAppData->hBMSelectedVertex ) ;

	jeRam_Free( *ppAppData ) ;

}// AppData_Destroy


// ACCESSORS

uint32 AppData_GetHandleCornerBitmap( void )
{
	return (uint32)((CJweApp*)AfxGetApp())->m_pAppData->hBMHandleCorner ;
}// AppData_GetHandleCornerBitmap

uint32 AppData_GetHandleEdgeBitmap( void )
{
	return (uint32)((CJweApp*)AfxGetApp())->m_pAppData->hBMHandleEdge ;
}// AppData_GetHandleEdgeBitmap

uint32 AppData_GetHandleRotateTL( void )
{
	return (uint32)((CJweApp*)AfxGetApp())->m_pAppData->hBMRotateTL ;
}// AppData_GetHandleRotateTL

uint32 AppData_GetHandleRotateTR( void )
{
	return (uint32)((CJweApp*)AfxGetApp())->m_pAppData->hBMRotateTR ;
}// AppData_GetHandleRotateTR

uint32 AppData_GetHandleRotateBL( void )
{
	return (uint32)((CJweApp*)AfxGetApp())->m_pAppData->hBMRotateBL ;
}// AppData_GetHandleRotateBL

uint32 AppData_GetHandleRotateBR( void )
{
	return (uint32)((CJweApp*)AfxGetApp())->m_pAppData->hBMRotateBR ;
}// AppData_GetHandleRotateBL

uint32 AppData_GetHandleShearLR( void )
{
	return (uint32)((CJweApp*)AfxGetApp())->m_pAppData->hBMShearLR ;
}// AppData_GetHandleShearLR

uint32 AppData_GetHandleShearTB( void )
{
	return (uint32)((CJweApp*)AfxGetApp())->m_pAppData->hBMShearTB ;
}// AppData_GetHandleShearTB

uint32 AppData_GetVertex( void )
{
	return (uint32)((CJweApp*)AfxGetApp())->m_pAppData->hBMVertex ;
}//AppData_GetVertex

uint32 AppData_GetSelectedVertex( void )
{
	return (uint32)((CJweApp*)AfxGetApp())->m_pAppData->hBMSelectedVertex ;
}//AppData_GetSelectedVertex

