/****************************************************************************************/
/*  WNDREG.CPP                                                                          */
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
// This module Registers Windows and Signitures and allows look up by either
// Actually held by CJweApp

//	by trilobite jan. 2011 - new MFC
#include "Stdafx.h"
//

#include "WndReg.h"
#include "ram.h"
#include "jwe.h"

#define DEFAULT_ARRAY_SIZE 20
typedef struct tagWindowEntry
{
	HWND 		pHwnd;
	int32		Signiture;
} WindowEntry;

typedef struct tagWindowRegister 
{
	int32	ArraySize;
	int32	EntryN;
	WindowEntry * EntryArray;
} WindowRegister ;


WindowRegister *WndReg_Create()
{
	WindowRegister * pWindowRegister ;
	
	pWindowRegister = JE_RAM_ALLOCATE_STRUCT( WindowRegister ) ;
	if( pWindowRegister == NULL )
		goto WRC_FAILURE ;

	pWindowRegister->EntryArray = JE_RAM_ALLOCATE_ARRAY( WindowEntry, DEFAULT_ARRAY_SIZE ) ;
	if( pWindowRegister->EntryArray == NULL )
		goto WRC_FAILURE ;

	pWindowRegister->ArraySize = DEFAULT_ARRAY_SIZE;
	pWindowRegister->EntryN = 0;
	return( pWindowRegister );

WRC_FAILURE:
	if( pWindowRegister->EntryArray != NULL )
		jeRam_Free( pWindowRegister->EntryArray );

	if( pWindowRegister != NULL )
		jeRam_Free( pWindowRegister );
	return( NULL );

}

void WndReg_Destroy( WindowRegister **hWndReg )
{
	if( (*hWndReg)->EntryArray != NULL )
		jeRam_Free( (*hWndReg)->EntryArray );

	jeRam_Free( (*hWndReg) );
}

jeBoolean WndReg_EnlargeArray( WindowRegister *pWndReg )
{
	int BlockN;
	WindowEntry * EntryArray;

	BlockN = pWndReg->ArraySize/ DEFAULT_ARRAY_SIZE + 1;

	EntryArray = JE_RAM_REALLOC_ARRAY( pWndReg->EntryArray, WindowEntry, BlockN * DEFAULT_ARRAY_SIZE );
	if( EntryArray == NULL )
		return( JE_FALSE );
	pWndReg->EntryArray = EntryArray;
	return( JE_TRUE );
}

jeBoolean WndReg_RegisterWindow( HWND pHwnd, int32 Signiture )
{
	WindowRegister *pWndReg = ((CJweApp*)AfxGetApp())->m_WndReg;
	if( pWndReg->EntryN ==  pWndReg->ArraySize )
	{
		if( !WndReg_EnlargeArray( pWndReg ) )
			return( JE_FALSE );
	}
	pWndReg->EntryArray[ pWndReg->EntryN].pHwnd = pHwnd;
	pWndReg->EntryArray[ pWndReg->EntryN].Signiture = Signiture;
	pWndReg->EntryN++;

	return( JE_TRUE );
}

int32 WndReg_GetSigniture( HWND pHwnd )
{
	WindowRegister *pWndReg = ((CJweApp*)AfxGetApp())->m_WndReg;
	int i;

	for( i = 0; i < pWndReg->EntryN; i++ )
	{
		if( pWndReg->EntryArray[i].pHwnd == pHwnd )
			return( pWndReg->EntryArray[i].Signiture);
	}

	return( 0 );
}

HWND  WndReg_GetWindow( int32 Signiture )
{
	WindowRegister *pWndReg = ((CJweApp*)AfxGetApp())->m_WndReg;
	int i;

	for( i = 0; i < pWndReg->EntryN; i++ )
	{
		if( pWndReg->EntryArray[i].Signiture == Signiture )
			return( pWndReg->EntryArray[i].pHwnd);
	}
	return( NULL );
}