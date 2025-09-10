/****************************************************************************************/
/*  GLOBALMATERIALS.CPP                                                                 */
/*                                                                                      */
/*  Author:  Peter Siamidis                                                             */
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
#include <assert.h>
#include "stdafx.h"
#include "jwe.h"
#include "globalmaterials.h"
#include "errorlog.h"
#include "ram.h"
#include "MaterialList2.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
// CGlobalMaterials dialog


CGlobalMaterials::CGlobalMaterials(CWnd* pParent /*=NULL*/)
	: CDialog(CGlobalMaterials::IDD, pParent)
{
	//{{AFX_DATA_INIT(CGlobalMaterials)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CGlobalMaterials::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CGlobalMaterials)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CGlobalMaterials, CDialog)
	//{{AFX_MSG_MAP(CGlobalMaterials)
	ON_CBN_SELCHANGE(ALLMATERIALS_CB_GROUPNAME, OnSelchangeCbGroupname)
	ON_CBN_SELCHANGE(ALLMATERIALS_CB_MATERIALNAME, OnSelchangeCbMaterialname)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()



static void Rect_GetBasedOnAnother(
	RECT	*BaseRect,
	int		LeftBorder,
	int		TopBorder,
	int		RightBorder,
	int		BottomBorder,
	int		MinWidth,
	int		MinHeight,
	RECT	*Rect )
{

	// ensure valid data
	assert( BaseRect != NULL );
	assert( Rect != NULL );
	assert( LeftBorder >= 0 );
	assert( TopBorder >= 0 );
	assert( RightBorder >= 0 );
	assert( BottomBorder >= 0 );
	assert( MinHeight > 0 );
	assert( MinWidth > 0 );

	// setup horizontal rect info
	Rect->left = LeftBorder;
	Rect->right = ( BaseRect->right - BaseRect->left ) - RightBorder;
	if ( ( Rect->right - Rect->left ) < MinWidth )
	{
		Rect->right = Rect->left + MinWidth;
	}

	// setup vertical rect info
	Rect->top = TopBorder;
	Rect->bottom = ( BaseRect->bottom - BaseRect->top ) - BottomBorder;
	if ( ( Rect->bottom - Rect->top ) < MinHeight )
	{
		Rect->bottom = Rect->top + MinHeight;
	}

	// make sure the rect jives
	assert( ( Rect->bottom - Rect->top ) >= MinHeight );
	assert( ( Rect->right - Rect->left ) >= MinWidth );

} // Rect_GetBasedOnAnother()



/////////////////////////////////////////////////////////////////////////////
// CGlobalMaterials message handlers




////////////////////////////////////////////////////////////////////////////////////////
//
//	CGlobalMaterials::OnInitDialog()
//
////////////////////////////////////////////////////////////////////////////////////////
BOOL CGlobalMaterials::OnInitDialog()
{

	// locals
	RECT	SourceRect;
	RECT	DestRect;

	// do standard CDialog init
	CDialog::OnInitDialog();

	// adjust the global materials rect within the dialog bar rect
	// undone! i have no idea how to get a proper rect for the parent
	DestRect.left = 4;
	DestRect.top = 4;
	DestRect.right = 172;
	DestRect.bottom = 332;
	this->MoveWindow( &DestRect, JE_TRUE );

	// get our window rect
	this->GetWindowRect( &SourceRect );

	// create materials list dialog
	if ( m_MaterialsList.Create( IDD_MATERIALLIST, this ) == JE_FALSE )
	{
		return JE_FALSE;
	}

	// position materials list dialog
	Rect_GetBasedOnAnother( &SourceRect, 8, 120, 8, 8, 20, 20, &DestRect );
	m_MaterialsList.MoveWindow( &DestRect, JE_TRUE );

	// process materials
	{

		// locals
		//MaterialDirectory	*CurMatDir;

		// create material list
		RootMaterialDirectory = MaterialList2_Create( "TestDir" );
		if ( RootMaterialDirectory == NULL )
		{
			jeErrorLog_Add( JE_ERR_INTERNAL_RESOURCE, "Could not create material list" );
			return JE_FALSE;
		}

		//undone
		//CurMatDir = MaterialList_GetDirectory( NULL, RootMaterialDirectory );


/*
		{

			// locals
			MaterialDirectory	*DefaultGroupDir;
			char				GroupName[256];
			int					SelectedGroup;
			int					StringLength;
			int i;

undone

			// get default group name
			SelectedGroup = this->SendDlgItemMessage( ALLMATERIALS_CB_GROUPNAME, CB_GETCURSEL, 0, 0 );
			StringLength = this->SendDlgItemMessage( ALLMATERIALS_CB_GROUPNAME, CB_GETLBTEXTLEN, SelectedGroup, 0 );
			assert( StringLength < 255 );
			this->SendDlgItemMessage( ALLMATERIALS_CB_GROUPNAME, CB_GETLBTEXT, SelectedGroup, (LPARAM)GroupName );

			// get the directory for the default group
			DefaultGroupDir = MaterialList_GetDirectory( GroupName, RootMaterialDirectory );
			i = 1;
		}
*/


		// reset the contents of both the group and material lists
		this->SendDlgItemMessage( ALLMATERIALS_CB_GROUPNAME, CB_RESETCONTENT );
		this->SendDlgItemMessage( ALLMATERIALS_CB_MATERIALNAME, CB_RESETCONTENT );








		this->SendDlgItemMessage( ALLMATERIALS_CB_GROUPNAME, CB_ADDSTRING, 0, (LPARAM)( "Testing1" ) );
		this->SendDlgItemMessage( ALLMATERIALS_CB_GROUPNAME, CB_ADDSTRING, 0, (LPARAM)( "Testing2" ) );
		this->SendDlgItemMessage( ALLMATERIALS_CB_GROUPNAME, CB_ADDSTRING, 0, (LPARAM)( "Testing3" ) );
		this->SendDlgItemMessage( ALLMATERIALS_CB_GROUPNAME, CB_SETCURSEL, 0, 0 );

		this->SendDlgItemMessage( ALLMATERIALS_CB_MATERIALNAME, CB_ADDSTRING, 0, (LPARAM)( "Testing1" ) );
		this->SendDlgItemMessage( ALLMATERIALS_CB_MATERIALNAME, CB_ADDSTRING, 0, (LPARAM)( "Testing2" ) );
		this->SendDlgItemMessage( ALLMATERIALS_CB_MATERIALNAME, CB_ADDSTRING, 0, (LPARAM)( "Testing3" ) );
		this->SendDlgItemMessage( ALLMATERIALS_CB_MATERIALNAME, CB_SETCURSEL, 0, 0 );
	}

	// return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
	return TRUE;  

} // CGlobalMaterials::OnInitDialog()



void CGlobalMaterials::OnSelchangeCbGroupname() 
{

	// locals
	char	GroupName[256];
	int		SelectedIndex;
	int		StringLength;

	int i;
	
	SelectedIndex = this->SendDlgItemMessage( ALLMATERIALS_CB_GROUPNAME, CB_GETCURSEL, 0, 0 );


	StringLength = this->SendDlgItemMessage( ALLMATERIALS_CB_GROUPNAME, CB_GETLBTEXTLEN, SelectedIndex, 0 );

	assert( StringLength < 255 );


	this->SendDlgItemMessage( ALLMATERIALS_CB_GROUPNAME, CB_GETLBTEXT, SelectedIndex, (LPARAM)GroupName );



	i = 1;

}

void CGlobalMaterials::OnSelchangeCbMaterialname() 
{
	// TODO: Add your control notification handler code here
	
}
