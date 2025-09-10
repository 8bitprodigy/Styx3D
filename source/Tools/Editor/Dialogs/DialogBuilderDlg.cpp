/****************************************************************************************/
/*  DIALOGBUILDERDLG.CPP                                                                */
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
#include "stdafx.h"
#include "DialogBuilderDlg.h"
#include "BuilderButton.h"
#include "BuilderEdit.h"
#include "BuilderCombo.h"
#include "mainfrm.h"
#include "AFXPRIV.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


#define DIALOG_SIDEMARGIN 7
#define DIALOG_TOPMARGIN 3
#define LEVEL_MARGIN 5
#define NAME_ID_OFFSET		1000
#define BORDER_ID_OFFSET	2000
#define SPIN_ID_OFFSET		3000
#define FIELD_HEIGHT  18


/////////////////////////////////////////////////////////////////////////////
// CDialogBuilderDlg dialog

CDialogBuilderDlg::CDialogBuilderDlg(CWnd* pParent /*=nullptr*/)
	: CDialog(CDialogBuilderDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDialogBuilderDlg)
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
}

void CDialogBuilderDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDialogBuilderDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CDialogBuilderDlg, CDialog)
	//{{AFX_MSG_MAP(CDialogBuilderDlg)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_VSCROLL()
	ON_WM_KEYDOWN()
	ON_WM_SIZE()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDOWN( )
	ON_WM_SETCURSOR()
	
//	1.16.05 by tom morris - better support for new color button
	ON_MESSAGE(CPN_SELCHANGE, OnChangeColor)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDialogBuilderDlg message handlers

BOOL CDialogBuilderDlg::OnInitDialog()
{

	CDialog::OnInitDialog();

	pFieldList = nullptr;
	Bottom = 0;
	FieldN = 0;
	WndVScrollPos = 0;
	if( DlgFont.CreatePointFont( 14, "MS Sans Serif" )  == 0 )
		return( FALSE );

 	SetScollBar();
	cpDragStart.y=-1;
	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CDialogBuilderDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CDialogBuilderDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

BOOL CDialogBuilderDlg::AddStringField( FieldStruct * pField,  char * Name, int Id, int Type, float Min, float Max, float Increment )
{
	CBuilderEdit *EditField = nullptr;
	RECT	  Frame;


	//Init all controls at 0,0. FormatDlg will set them right.
	Frame.top = 0;
	Frame.left = 0;
	Frame.right = 100;
	Frame.bottom = 100;

	pField->FieldName = new CStatic;
	if( pField->FieldName == nullptr )
		goto ERR_ADDSTRINGFIELD;

	if( pField->FieldName->Create( Name, SS_LEFT, Frame, this, Id + NAME_ID_OFFSET ) == 0 )
		goto ERR_ADDSTRINGFIELD;

	pField->EditBorder = new CStatic;

	if( pField->EditBorder->Create( "", SS_SUNKEN, Frame, this, Id + BORDER_ID_OFFSET ) == 0 )
		goto ERR_ADDSTRINGFIELD;



	EditField = new CBuilderEdit( pField->Type, pField->DataId );

	if( EditField == nullptr )
		goto ERR_ADDSTRINGFIELD;

	if( EditField->Create( ES_AUTOHSCROLL|ES_CENTER | WS_CHILD | WS_EX_CLIENTEDGE   , Frame, this, Id ) == 0 ) 
		goto ERR_ADDSTRINGFIELD;

	if( Type == PROPERTY_INT_TYPE || Type == PROPERTY_FLOAT_TYPE )
	{
		pField->Spin = new CBuildSpin;
		if( pField->Spin == nullptr )
			goto ERR_ADDSTRINGFIELD;
		if( pField->Spin->Create( UDS_ALIGNRIGHT|UDS_AUTOBUDDY|UDS_ARROWKEYS, Frame, this, Id + SPIN_ID_OFFSET ) == 0 )
			goto ERR_ADDSTRINGFIELD;
		pField->Spin->SetBuddy( EditField );
		pField->Spin->SetRange( 1, 0 );
		EditField->SetValueRange( Min, Max, Increment );
	}


	pField->FieldCntl = EditField;

	return( TRUE );

ERR_ADDSTRINGFIELD:
	if( pField->FieldName != nullptr )
		delete pField->FieldName;
	if( EditField != nullptr )
		delete EditField;
	if( pField->EditBorder != nullptr )
		delete pField->EditBorder;
	if( pField->Spin != nullptr )
		delete pField->Spin;
	return( FALSE );
}

BOOL CDialogBuilderDlg::AddStaticField( FieldStruct * pField,  char * Name, int Id, int Type )
{
	CStatic *EditField = nullptr;
	RECT	  Frame;


	//Init all controls at 0,0. FormatDlg will set them right.
	Frame.top = 0;
	Frame.left = 0;
	Frame.right = 100;
	Frame.bottom = 100;

	pField->FieldName = new CStatic;
	if( pField->FieldName == nullptr )
		goto ERR_ADDSTRINGFIELD;

	if( pField->FieldName->Create( Name, SS_LEFT, Frame, this, Id + NAME_ID_OFFSET ) == 0 )
		goto ERR_ADDSTRINGFIELD;

	pField->EditBorder = nullptr;

	EditField = new CStatic;

	if( EditField == nullptr )
		goto ERR_ADDSTRINGFIELD;

	if( EditField->Create( "", ES_CENTER | WS_CHILD | WS_EX_CLIENTEDGE    , Frame, this, Id ) == 0 ) 
		goto ERR_ADDSTRINGFIELD;


	pField->FieldCntl = EditField;

	return( TRUE );

ERR_ADDSTRINGFIELD:
	if( pField->FieldName != nullptr )
		delete pField->FieldName;
	if( EditField != nullptr )
		delete EditField;
	if( pField->EditBorder != nullptr )
		delete pField->EditBorder;
	return( FALSE );
	Type;
}

BOOL CDialogBuilderDlg::AddComboField( FieldStruct * pField,  char * Name, int Id, int StringN, char **StringList )
{
	CBuilderCombo *EditField = nullptr;
	RECT	  Frame;
	int i;


	//Init all controls at 0,0. FormatDlg will set them right.
	Frame.top = 0;
	Frame.left = 0;
	Frame.right = 100;
	Frame.bottom = 100;

	pField->FieldName = new CStatic;
	if( pField->FieldName == nullptr )
		goto ERR_ADDCOMBOFIELD;

	if( pField->FieldName->Create( Name, SS_LEFT, Frame, this, Id + NAME_ID_OFFSET ) == 0 )
		goto ERR_ADDCOMBOFIELD;



	EditField = new CBuilderCombo( PROPERTY_COMBO_TYPE,  pField->DataId );

	if( EditField == nullptr )
		goto ERR_ADDCOMBOFIELD;


	if( EditField->Create( CBS_DROPDOWNLIST|WS_VSCROLL    , Frame, this, Id ) == 0 ) 
		goto ERR_ADDCOMBOFIELD;

	for( i  = 0; i < StringN; i++ )
	{
		EditField->AddString( StringList[i] );
	}

	pField->FieldCntl = EditField;

	return( TRUE );

ERR_ADDCOMBOFIELD:
	if( pField->FieldName != nullptr )
		delete pField->FieldName;
	if( EditField != nullptr )
		delete EditField;
	if( pField->EditBorder != nullptr )
		delete pField->EditBorder;
	if( pField->Spin != nullptr )
		delete pField->Spin;
	return( FALSE );
}
BOOL CDialogBuilderDlg::AddCheckBox( FieldStruct * pField, char * Name, int Id  )
{
	CBuilderButton * pButtonField;
	RECT	  Frame;


	//Init all controls at 0,0. FormatDlg will set them right.
	Frame.top = 0;
	Frame.left = 0;
	Frame.right = 100;
	Frame.bottom = 100;

	pField->FieldName = nullptr;
	pButtonField = new CBuilderButton( pField->Type, pField->DataId );
	if( pButtonField == nullptr )
		goto ERR_ADDCHECKBOX;

	if( pButtonField->Create( Name, BS_LEFTTEXT | BS_AUTO3STATE   | WS_CHILD, Frame, this, Id ) == 0)
		goto ERR_ADDCHECKBOX;

	pField->FieldCntl = pButtonField;
	return( TRUE );

ERR_ADDCHECKBOX:
	if( pButtonField != nullptr )
		delete pButtonField;
	return( FALSE );
}


//	1.16.05 by tom morris - better support for new color button
BOOL CDialogBuilderDlg::AddColorPicker( FieldStruct * pField, char * Name, int Id  )
{
	CColorButton *pButtonField = nullptr;
	RECT	  Frame;

	//Init all controls at 0,0. FormatDlg will set them right.
	Frame.top = 0;
	Frame.left = 0;
	Frame.right = 100;
	Frame.bottom = 100;

	pField->FieldName = nullptr;
	pButtonField = new CColorButton();

	if (pButtonField)
	{
		if(pButtonField->Create(Name, WS_CHILD, Frame, this, Id ))
		{
			pField->FieldCntl = pButtonField;
			pButtonField->SetTrackSelection(TRUE);
		}
		else
			goto ERR_ADDCOLORPICKER;

		return( TRUE );
	}
	else
		goto ERR_ADDCOLORPICKER;

ERR_ADDCOLORPICKER:
	if( pButtonField != nullptr )
		delete pButtonField;
	return( FALSE );
}

BOOL CDialogBuilderDlg::AddGroupButton( FieldStruct * pField, char * Name, int Id  )
{
	CBuilderButton * pButtonField;
	RECT	  Frame;
	int		  Flags;

	pField->FieldName = nullptr;
	//Init all controls at 0,0. FormatDlg will set them right.
	Frame.top = 0;
	Frame.left = 0;
	Frame.right = 100;
	Frame.bottom = 100;

	pButtonField = new CBuilderButton( pField->Type, pField->DataId );
	if( pButtonField == nullptr )
		goto ERR_ADDGROUP;


	Flags = BS_AUTOCHECKBOX | BS_PUSHLIKE |WS_CHILD;
	if( pField->Type == PROPERTY_GROUP_TYPE)
		 Flags = BS_OWNERDRAW|WS_CHILD;

	if( pButtonField->Create( Name,Flags , Frame, this, Id ) == 0)
		goto ERR_ADDGROUP;

	pField->FieldCntl = pButtonField;
	return( TRUE );

ERR_ADDGROUP:
	if( pButtonField != nullptr )
		delete pButtonField;
	return( FALSE );
}


BOOL CDialogBuilderDlg::AddVec3dGroupButton( FieldStruct * pField, char * Name, int Id  )
{
	CBuilderButton * pButtonField = nullptr;
	RECT	  Frame;
	int Flags;

	//Init all controls at 0,0. FormatDlg will set them right.
	Frame.top = 0;
	Frame.left = 0;
	Frame.right = 100;
	Frame.bottom = 100;

	pField->FieldName = new CStatic;

	if( pField->FieldName == nullptr )
		goto ERR_ADD3DGROUP;

	if( pField->FieldName->Create( Name, SS_LEFT, Frame, this, Id + 1000 ) == 0 )
		goto ERR_ADD3DGROUP;

	pButtonField = new CBuilderButton( pField->Type, pField->DataId );
	if( pButtonField == nullptr )
		goto ERR_ADD3DGROUP;

	Flags = BS_LEFTTEXT | BS_AUTOCHECKBOX | BS_PUSHLIKE |WS_CHILD;
	if(( pField->Type == PROPERTY_COLOR_GROUP_TYPE)||
	   ( pField->Type == PROPERTY_VEC3D_GROUP_TYPE)||
	   ( pField->Type == PROPERTY_TIME_GROUP_TYPE))
		 Flags |= BS_OWNERDRAW;
	
	if( pButtonField->Create( Name,  Flags, Frame, this, Id ) == 0)
		goto ERR_ADD3DGROUP;

	pField->FieldCntl = pButtonField;
	return( TRUE );

ERR_ADD3DGROUP:
	if( pButtonField != nullptr )
		delete pButtonField;
	return( FALSE );
}

BOOL CDialogBuilderDlg::AddRadioButton( FieldStruct * pField, char * Name, int Id )
{
	CBuilderButton * pButtonField;
	RECT	  Frame;


	//Init all controls at 0,0. FormatDlg will set them right.
	Frame.top = 0;
	Frame.left = 0;
	Frame.right = 100;
	Frame.bottom = 100;

	pField->FieldName = nullptr;
	pButtonField = new CBuilderButton( pField->Type, pField->DataId );
	if( pButtonField == nullptr )
		goto ERR_ADDRADIOBUTTON;

	if( pButtonField->Create( Name, BS_LEFTTEXT | BS_AUTORADIOBUTTON| WS_CHILD, Frame, this, Id ) == 0)
		goto ERR_ADDRADIOBUTTON;

	pField->FieldCntl = pButtonField;
	return( TRUE );

ERR_ADDRADIOBUTTON:
	if( pButtonField != nullptr )
		delete pButtonField;
	return( FALSE );
}

BOOL CDialogBuilderDlg::AddField( FieldStruct * pField, char * Name, PROPERTY_FIELD_TYPE Type, int Id, int DataId, jeProperty_TypeInfo *pTypeInfo )
{
	BOOL Result = TRUE;
	pField->Type = Type;
	pField->Id = Id;
	pField->FieldCntl = nullptr;
	pField->FieldName = nullptr;
	pField->EditBorder = nullptr;
	pField->Spin = nullptr;
	pField->DataId = DataId;

	switch( Type )
	{
	case PROPERTY_STRING_TYPE:
	case PROPERTY_INT_TYPE:
	case PROPERTY_FLOAT_TYPE:
		Result = AddStringField( pField, Name,Id, Type, pTypeInfo->NumInfo.Min, pTypeInfo->NumInfo.Max, pTypeInfo->NumInfo.Increment );
		break;

	case PROPERTY_STATIC_INT_TYPE:
		Result = AddStaticField( pField, Name,Id, Type );
		break;

	case PROPERTY_CHECK_TYPE:
		Result = AddCheckBox(  pField, Name,  Id );
		break;

	case PROPERTY_COLOR_PICKER_TYPE:
		Result = AddColorPicker(  pField, Name,  Id );
		break;

	case PROPERTY_GROUP_TYPE:
	case PROPERTY_BUTTON_TYPE:
		Result = AddGroupButton(  pField, Name,Id );
		break;

	case PROPERTY_VEC3D_GROUP_TYPE:
	case PROPERTY_COLOR_GROUP_TYPE:
		Result = AddVec3dGroupButton(  pField, Name,Id );
		break;

	case PROPERTY_RADIO_TYPE:
		Result = AddRadioButton(  pField, Name,Id );
		break;

	case PROPERTY_GROUP_END_TYPE:
	case PROPERTY_TIME_GROUP_TYPE:
	case PROPERTY_CHANNEL_POS_TYPE:	
	case PROPERTY_CHANNEL_EVENT_TYPE:		
	case PROPERTY_CHANNEL_ROT_TYPE:			
	case PROPERTY_CURTIME_TYPE:				
		break;

	case PROPERTY_COMBO_TYPE:
		Result = AddComboField(  pField, Name,Id, pTypeInfo->ComboInfo.StringN, pTypeInfo->ComboInfo.StringList );
		break;

	}

	return( Result );
}

void CDialogBuilderDlg::SetRadioFieldGroup( FieldStruct * pField )
{
	CBuilderButton * pButton;

	pButton = (CBuilderButton*)pField->FieldCntl;
	pButton->SetButtonStyle( BS_AUTORADIOBUTTON | WS_CHILD |WS_GROUP  );
}

BOOL CDialogBuilderDlg::SetStringField( FieldStruct *pField, char *Data, int DataSize )
{
	CBuilderEdit * pEditField;


	if( DataSize == PROPERTY_DATA_INVALID )
		return( TRUE );

	pEditField = (CBuilderEdit*)pField->FieldCntl;

	pEditField->SetWindowText( Data );
	return( TRUE );
}

BOOL CDialogBuilderDlg::SetIntField( FieldStruct *pField, int Int, int DataSize )
{
	CBuilderEdit * pEditField;
	CString	StringText;

	if( DataSize == PROPERTY_DATA_INVALID )
		return( TRUE );

	ASSERT( DataSize == sizeof( int ) );
	pEditField = (CBuilderEdit*)pField->FieldCntl;

	StringText.Format( "%d", Int) ;
	pEditField->SetWindowText( StringText.GetBuffer(0) );
	return( TRUE );
}

BOOL CDialogBuilderDlg::SetStaticIntField( FieldStruct *pField, int Int, int DataSize )
{
	CStatic * pEditField;
	CString	StringText;

	if( DataSize == PROPERTY_DATA_INVALID )
		return( TRUE );

	ASSERT( DataSize == sizeof( int ) );
	pEditField = (CStatic*)pField->FieldCntl;

	StringText.Format( "%d", Int) ;
	pEditField->SetWindowText( StringText.GetBuffer(0) );
	return( TRUE );
}

BOOL CDialogBuilderDlg::SetFloatField( FieldStruct *pField, float Float, int DataSize )
{
	CBuilderEdit * pEditField;
	CString	StringText;

	if( DataSize == PROPERTY_DATA_INVALID )
		return( TRUE );

	ASSERT( DataSize == sizeof( float ) );
	pEditField = (CBuilderEdit*)pField->FieldCntl;

	StringText.Format( "%10.4g", Float) ;
	StringText.TrimLeft(" ");
	pEditField->SetWindowText( StringText.GetBuffer(0) );
	return( TRUE );
}


BOOL CDialogBuilderDlg::SetCheckField( FieldStruct *pField, int Bool, int DataSize )
{
	CBuilderButton * pButton;

	pButton = (CBuilderButton*)pField->FieldCntl;
	if( DataSize == PROPERTY_DATA_INVALID )
	{
		pButton->SetCheck( 2 );
		return( TRUE );
	}
	ASSERT( DataSize == sizeof( int ) );

	pButton->SetCheck( Bool );
	return( TRUE );
}

BOOL CDialogBuilderDlg::SetRadioField( FieldStruct *pField, int Bool, int DataSize )
{
	CBuilderButton * pButton;


	if( DataSize != sizeof( int ) )
		return( TRUE );
	pButton = (CBuilderButton*)pField->FieldCntl;

	pButton->SetCheck( Bool );
	return( TRUE );
}

BOOL CDialogBuilderDlg::SetVec3dGroup( FieldStruct *pField, jeVec3d Vector, int DataSize )
{
	CBuilderButton * pButton;
	CString	ButtonText;

	ASSERT( DataSize == sizeof( jeVec3d ) );
	ButtonText.Format( "x:%.0f y:%.0f z:%.0f", Vector.X, Vector.Y, Vector.Z );

	pButton = (CBuilderButton*)pField->FieldCntl;


	pButton->SetWindowText( ButtonText.GetBuffer( 0 ) );
	return( TRUE );
	DataSize;
}

BOOL CDialogBuilderDlg::SetColorPickerField(  FieldStruct *pField, jeVec3d Vector, int DataSize )
{
//	CColorButton support by Tom Morris 1-16-05
//	CBuilderButton * pButton;
	CColorButton * pButton;

	if( DataSize == PROPERTY_DATA_INVALID )
		return( TRUE );
//	CColorButton support by Tom Morris 1-16-05
//	pButton = (CBuilderButton*)pField->FieldCntl;
	pButton = (CColorButton*)pField->FieldCntl;

	pButton->SetColor( RGB( Vector.X, Vector.Y, Vector.Z ) );

	return( TRUE );
}

BOOL CDialogBuilderDlg::SetRGBGroup( FieldStruct *pField, jeVec3d Vector, int DataSize )
{
	CBuilderButton * pButton;
	CString	ButtonText;

	if( DataSize == PROPERTY_DATA_INVALID )
		return( TRUE );
	ButtonText.Format( "R:%.0f G:%.01f B:%.0f", Vector.X, Vector.Y, Vector.Z );

	pButton = (CBuilderButton*)pField->FieldCntl;

	pButton->SetColor( RGB( Vector.X, Vector.Y, Vector.Z ) );

	pButton->SetWindowText( ButtonText.GetBuffer( 0 ) );
	return( TRUE );
}

BOOL CDialogBuilderDlg::SetComboField( FieldStruct *pField, char * String, int DataSize )
{
	CBuilderCombo * pCombo;
	int nIndex;

	if( DataSize == PROPERTY_DATA_INVALID )
		return( TRUE );
	pCombo = (CBuilderCombo*)pField->FieldCntl;

	nIndex = pCombo->FindString( -1, String );
	if( nIndex == CB_ERR )
		return( TRUE );
	pCombo->SetCurSel( nIndex );
	return( TRUE );
}

BOOL CDialogBuilderDlg::UpdateDataByArray( jeProperty_List *pArray )
{

	jeProperty * pDescriptor = pArray->pjeProperty;
	int i;

	ASSERT( pArray->jePropertyN == FieldN );

	for( i = 0; i < FieldN; i++ )
	{
		ASSERT( pDescriptor[i].Type == pFieldList[i].Type );
		if( !UpdateFieldData( &pFieldList[i], &pDescriptor[i].Data, pDescriptor[i].DataSize ) )
			return( FALSE );
	}
	return( TRUE );		
}

BOOL CDialogBuilderDlg::UpdateFieldData( FieldStruct *pField, jeProperty_Data *pData, int DataSize )
{
	switch( pField->Type )
	{
	case PROPERTY_STRING_TYPE:
		if( !SetStringField( pField, pData->String, DataSize ) )
			return( FALSE );
		break;

	case PROPERTY_INT_TYPE:
		if( !SetIntField( pField, pData->Int, DataSize ) )
			return( FALSE );
		break;

	case PROPERTY_STATIC_INT_TYPE:
		if( !SetStaticIntField( pField, pData->Int, DataSize ) )
			return( FALSE );
		break;

	case PROPERTY_FLOAT_TYPE:
		if( !SetFloatField( pField, pData->Float, DataSize ) )
			return( FALSE );
		break;

	case PROPERTY_CHECK_TYPE:
		if( !SetCheckField( pField, pData->Bool, DataSize ) )
			return( FALSE );
		break;

	case PROPERTY_COLOR_PICKER_TYPE:
		if( !SetColorPickerField( pField, pData->Vector, DataSize ) )
			return( FALSE );
		break;

	case PROPERTY_RADIO_TYPE:
		if( !SetRadioField( pField, pData->Bool, DataSize ) )
			return( FALSE );
		break;

	case PROPERTY_VEC3D_GROUP_TYPE:
		if( !SetVec3dGroup( pField, pData->Vector, DataSize ) )
			return( FALSE );
		break;

	case PROPERTY_COLOR_GROUP_TYPE:
		if( !SetRGBGroup( pField, pData->Vector, DataSize ) )
			return( FALSE );
		break;

	case PROPERTY_COMBO_TYPE:
		if( !SetComboField( pField, pData->String, DataSize ) )
			return( FALSE );
		break;

	case PROPERTY_GROUP_END_TYPE:
	case PROPERTY_GROUP_TYPE:
	case PROPERTY_BUTTON_TYPE:
	case PROPERTY_CHANNEL_POS_TYPE:	
	case PROPERTY_CHANNEL_EVENT_TYPE:		
	case PROPERTY_CHANNEL_ROT_TYPE:			
	case PROPERTY_CURTIME_TYPE:				
	case PROPERTY_TIME_GROUP_TYPE:
		break;
	}
	return( TRUE );
}

BOOL CDialogBuilderDlg::UpdateFieldDataById( int Id, jeProperty_Data *pData, int DataSize )
{
	FieldStruct * pField;

	pField = FindFieldById( Id );
	if( pField == nullptr )
		return( FALSE );
	return( UpdateFieldData( pField, pData, DataSize ) );
}

void CDialogBuilderDlg::DisableField( FieldStruct * pField )
{
	CEdit * pEditCtrl;
	switch( pField->Type )
	{
	case PROPERTY_INT_TYPE:
	case PROPERTY_FLOAT_TYPE:
		pField->Spin->ModifyStyle( 0, WS_DISABLED, 0 );
	case PROPERTY_STRING_TYPE:
		pEditCtrl = (CEdit *)pField->FieldCntl;
		pEditCtrl->SetReadOnly( TRUE );
	break;

	case PROPERTY_STATIC_INT_TYPE:
		break;

	default:
		pField->FieldCntl->ModifyStyle( 0, WS_DISABLED, 0 );
		break;
	}
}
BOOL CDialogBuilderDlg::BuildFromDescriptor( jeProperty_List *pArray )
{
	int i;
	int Level = 0;
	jeProperty * pDescriptor = pArray->pjeProperty;
	BOOL		bInRadioGroup = FALSE;


	FieldN = pArray->jePropertyN;
	pFieldList = (FieldStruct*)new( FieldStruct[pArray->jePropertyN] );
	if( pFieldList == nullptr )
		return( FALSE );

	for( i = 0 ; i < pArray->jePropertyN ; i++ )
	{
		if(  pDescriptor[i].Type ==	PROPERTY_TIME_GROUP_TYPE )
		{
			while( pDescriptor[i].Type !=	PROPERTY_GROUP_END_TYPE )
			{
				memset( &pFieldList[i], 0, sizeof( FieldStruct ) );
				pFieldList[i].Type = pDescriptor[i].Type;
				i++;
				ASSERT( i < pArray->jePropertyN );
			}
			memset( &pFieldList[i], 0, sizeof( FieldStruct ) );
			pFieldList[i].Type = pDescriptor[i].Type;
			continue;
		}
		if( pDescriptor[i].Type == PROPERTY_GROUP_END_TYPE )
		{
			if( bInRadioGroup )
			{
				bInRadioGroup = FALSE;
			}
			else
			{
				ASSERT( Level > 0 );
				Level--;
			}
		}
		{
			pFieldList[i].DisplayLevel = Level;
			if( !AddField( &pFieldList[i], 
							pDescriptor[i].FieldName, 
							pDescriptor[i].Type, 
							i + 1000,
							pDescriptor[i].DataId,
							&pDescriptor[i].TypeInfo						
							) )
				return( FALSE );
			if( pDescriptor[i].bDisabled )
				DisableField( &pFieldList[i] );
			if( pFieldList[i].FieldName != nullptr )
					pFieldList[i].FieldName->SetFont( &DlgFont, TRUE );
			if( pFieldList[i].FieldCntl != nullptr )
					pFieldList[i].FieldCntl->SetFont( &DlgFont, TRUE );
			if(  pDescriptor[i].Type == PROPERTY_RADIO_TYPE && !bInRadioGroup )
			{
				SetRadioFieldGroup( &pFieldList[i] );
				bInRadioGroup = TRUE ;
			}
			if( pDescriptor[i].Type == PROPERTY_GROUP_TYPE || 
				pDescriptor[i].Type == PROPERTY_VEC3D_GROUP_TYPE ||
				pDescriptor[i].Type == PROPERTY_COLOR_GROUP_TYPE )
			{
				pFieldList[i].DisplayChildren = FALSE;
				Level++;
			}
		}
	}
	FormatDialog();
	UpdateDataByArray( pArray );
	return( TRUE );
}

void CDialogBuilderDlg::Reset()
{
	int i;

	for( i = 0; i < FieldN; i++ )
	{
		if( pFieldList[i].FieldName != nullptr )
			delete pFieldList[i].FieldName;
		if(pFieldList[i].FieldCntl != nullptr )
			delete pFieldList[i].FieldCntl;
		if( pFieldList[i].EditBorder != nullptr )
			delete pFieldList[i].EditBorder;
		if( pFieldList[i].Spin != nullptr )
			delete pFieldList[i].Spin;
	}
	if( pFieldList != nullptr )
		delete pFieldList;
	pFieldList = nullptr;
	FieldN = 0;
}

BOOL CDialogBuilderDlg::ShowField( FieldStruct* pField, int nCmdShow  )
{
	if( pField->FieldName != nullptr )
		pField->FieldName->ShowWindow( nCmdShow );
	if( pField->FieldCntl != nullptr )
		pField->FieldCntl->ShowWindow( nCmdShow );
	if( pField->EditBorder != nullptr )
		pField->EditBorder->ShowWindow( nCmdShow );
	if( pField->Spin != nullptr )
		pField->Spin->ShowWindow( nCmdShow );
	return( TRUE );
}

BOOL CDialogBuilderDlg::FormatField(FieldStruct* pField, int *Height, int Level )
{
	int x, y, cx, cy;
	RECT ClientRect;
	RECT WindowRect;
	RECT FieldRect;

	GetClientRect( &ClientRect );

	
	*Height += DIALOG_SIDEMARGIN;
	y = *Height;
	x = ClientRect.left + DIALOG_SIDEMARGIN + (LEVEL_MARGIN * Level );
	cx = (ClientRect.right - x) - DIALOG_SIDEMARGIN ;
	cy = FIELD_HEIGHT;

	if( pField->FieldName != nullptr )
	{
		int NameCx;
		if( pField->Type == PROPERTY_STATIC_INT_TYPE )
			NameCx = cx * 2 /3;
		else
			NameCx = cx /3;
		pField->FieldName->GetWindowRect( &WindowRect );
		pField->FieldName->GetClientRect( &FieldRect );
		pField->FieldName->MoveWindow( x, y, NameCx, cy, TRUE );
		x +=NameCx;
		cx -= NameCx;
	}

	if( pField->Spin != nullptr )
	{
		int spinX;

		spinX = ClientRect.right - DIALOG_SIDEMARGIN -16 ;
		pField->Spin->MoveWindow( spinX, y-2, 16, cy+4, TRUE );
		cx -= 16;
	}

	if( pField->FieldCntl != nullptr )
	{
		pField->FieldCntl->MoveWindow( x, y, cx, cy, TRUE );
	}
	if( pField->EditBorder != nullptr )
	{
		x -= 2;
		y -= 2;
		cx += 4;
		cy += 4;

		pField->EditBorder->MoveWindow( x, y, cx, cy, TRUE );
	}
	*Height += FIELD_HEIGHT;
	return( TRUE );
}

void CDialogBuilderDlg::SetScollBar()
{
	RECT ClientRect;
	RECT WindowRect;
	int	max;

	GetClientRect( &ClientRect );

	GetWindowRect( &WindowRect );
	if( ClientRect.bottom > Bottom )
	{
		SetScrollRange( SB_VERT, 0, 0, TRUE );
		ShowScrollBar( SB_VERT, FALSE );
		ScrollWindow( 0, WndVScrollPos, nullptr, nullptr );
		SetScrollPos( SB_VERT, 0, false ) ;
		WndVScrollPos = 0;
	}
	else
	{
		ShowScrollBar( SB_VERT, FALSE );
		max = Bottom - ClientRect.bottom;
		SetScrollRange( SB_VERT, 0, max, TRUE );
		if( WndVScrollPos > max )
		{
			ScrollWindow( 0, WndVScrollPos - max , nullptr, nullptr );
			SetScrollPos( SB_VERT, max, TRUE );
			WndVScrollPos = max;
		}
	}
}


BOOL CDialogBuilderDlg::FormatDialog()
{
	int i{};
	int Level = 0;
	int CurrentHeight = -WndVScrollPos;

	for( i = 0; i < FieldN; i++ )
	{
		if( pFieldList[i].Type == PROPERTY_GROUP_END_TYPE ||
			pFieldList[i].Type == PROPERTY_CHANNEL_POS_TYPE ||
			pFieldList[i].Type == PROPERTY_CHANNEL_ROT_TYPE ||
			pFieldList[i].Type == PROPERTY_CHANNEL_EVENT_TYPE ||
			pFieldList[i].Type == PROPERTY_CURTIME_TYPE ||
			pFieldList[i].Type == PROPERTY_TIME_GROUP_TYPE
			)
			continue;
		if( pFieldList[i].DisplayLevel > Level )
		{
			if( !ShowField( &pFieldList[i], SW_HIDE ) )
				return( FALSE );
			continue;
		}
		else
		if( pFieldList[i].DisplayLevel < Level )
			Level = pFieldList[i].DisplayLevel;
		if( !FormatField( &pFieldList[i], &CurrentHeight, Level ) )
			return( FALSE );
		if( ( pFieldList[i].Type == PROPERTY_GROUP_TYPE ||
			  pFieldList[i].Type == PROPERTY_VEC3D_GROUP_TYPE ||
			  pFieldList[i].Type == PROPERTY_COLOR_GROUP_TYPE)
			&& pFieldList[i].DisplayChildren )
			Level++;
			if( !ShowField( &pFieldList[i], SW_SHOW ) )
				return( FALSE );
	}
	Bottom = CurrentHeight + WndVScrollPos;
	SetScollBar();
	return( TRUE );
}


BOOL CDialogBuilderDlg::HandleGroupMsg( FieldStruct* pField, int nCode )
{
	CBuilderButton *pButton;

	if( nCode == BN_CLICKED )
	{
		pField->DisplayChildren = !pField->DisplayChildren;
		pButton = (CBuilderButton *)pField->FieldCntl;
		// Added JH 25.3.2000
		if( ( pField->Type == PROPERTY_GROUP_TYPE )||
		    ( pField->Type == PROPERTY_VEC3D_GROUP_TYPE )||
		    ( pField->Type == PROPERTY_TIME_GROUP_TYPE )||
			( pField->Type == PROPERTY_COLOR_GROUP_TYPE )
			)
			pButton->SetState(pField->DisplayChildren);
		FormatDialog();
		Invalidate();
	return( TRUE );
	}
	return( FALSE );
}

BOOL CDialogBuilderDlg::HandleComboMsg( FieldStruct* pField, int nCode )
{
	if( nCode == CBN_SELENDOK )
	{
		CString Text;
		jeProperty_Data Data;
		CComboBox *pCombo;
		CMainFrame*	pMainFrm = (CMainFrame*)AfxGetMainWnd() ;
		CJweDoc*	pDoc = pMainFrm->GetCurrentDocument() ;
		int curSel;

		pCombo = (CComboBox *)pField->FieldCntl;
		curSel = pCombo->GetCurSel();
		if( curSel >= 0 )
		{
			pCombo->GetLBText( curSel, Text );
			Data.String = Text.GetBuffer(0);
			if( Data.String )
				pDoc->SetProperty( pField->DataId, pField->Type, &Data );
		}
		return( TRUE );
	}
	return( FALSE );
}

BOOL CDialogBuilderDlg::HandleFieldMsg( FieldStruct* pField, int nCode )
{
	BOOL Result = FALSE;
	switch( pField->Type )
	{

	case PROPERTY_GROUP_TYPE:
	case PROPERTY_VEC3D_GROUP_TYPE:
	case PROPERTY_COLOR_GROUP_TYPE:
		Result = HandleGroupMsg( pField, nCode );
		break;

	case PROPERTY_STRING_TYPE:
	case PROPERTY_INT_TYPE:
	case PROPERTY_STATIC_INT_TYPE:
	case PROPERTY_FLOAT_TYPE:
	case PROPERTY_CHECK_TYPE:
	case PROPERTY_COLOR_PICKER_TYPE:
	case PROPERTY_RADIO_TYPE:
	case PROPERTY_GROUP_END_TYPE:

	case PROPERTY_BUTTON_TYPE:
	case PROPERTY_TIME_GROUP_TYPE:
	case PROPERTY_CHANNEL_POS_TYPE:	
	case PROPERTY_CHANNEL_EVENT_TYPE:		
	case PROPERTY_CHANNEL_ROT_TYPE:			
	case PROPERTY_CURTIME_TYPE:				
		break;

	case PROPERTY_COMBO_TYPE:
		Result = HandleComboMsg( pField, nCode );
		break;
	}
	return( Result );
}

BOOL CDialogBuilderDlg::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	FieldStruct *foundField;

	if( wParam == 1 && lParam == 0 )
	{
		CWnd * ActiveWnd;
		CBuilderEdit * pEdit;

		ActiveWnd = GetFocus();
		if(ActiveWnd->IsKindOf(RUNTIME_CLASS(CBuilderEdit)) )
		{
			pEdit = (CBuilderEdit *)ActiveWnd;
			pEdit->SetProperty();
		}
		return(JE_TRUE );
	}

	foundField = FindFieldById( LOWORD( wParam)  );
	if( foundField != nullptr )
		if( HandleFieldMsg( foundField, HIWORD( wParam) ) )
			return( TRUE );

	return CDialog::OnCommand(wParam, lParam);
}


FieldStruct * CDialogBuilderDlg::FindFieldById( int Id )
{
	int i;

	for( i = 0; i < FieldN ; i ++ )
	{
		if( pFieldList[i].Id == (UINT)Id )
		{
			return( &pFieldList[i] );
		}
	}
	return( nullptr );
}

void CDialogBuilderDlg::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	int Id;
	CBuilderEdit *pEditCtrl;
	FieldStruct * pFoundField;
	int StartingScrollPos;
	int NewScrollPos;

	if( pScrollBar == nullptr )
	{
		StartingScrollPos = NewScrollPos = GetScrollPos( SB_VERT );
		switch( nSBCode )
		{
			// scroll texture list one texture at a time
			case SB_LINEDOWN:
			{
				NewScrollPos = NewScrollPos +1;
				break;
			}
			case SB_LINEUP:
			{
				NewScrollPos = NewScrollPos - 1;
				break;
			}

			// scroll texture list on page of textures at a time
			case SB_PAGEDOWN:
			{
				NewScrollPos = NewScrollPos +FIELD_HEIGHT;
				break;
			}
			case SB_PAGEUP:
			{
				NewScrollPos = NewScrollPos -FIELD_HEIGHT;
				break;
			}

			// scroll texture list based on slider bar
			case SB_THUMBTRACK:
			{
					NewScrollPos = nPos;
				break;
			}
		}
		if( NewScrollPos != StartingScrollPos )
		{
			ScrollWindow( 0, WndVScrollPos - NewScrollPos , nullptr, nullptr );
			WndVScrollPos = NewScrollPos;
			SetScrollPos( SB_VERT, NewScrollPos, true ) ;
		}
	}
	else
	if( nSBCode == SB_THUMBPOSITION )
	{
	// TODO: Add your message handler code here and/or call default
		Id = pScrollBar->GetDlgCtrlID();
		Id -= SPIN_ID_OFFSET;
		pFoundField = FindFieldById( Id );
		if( pFoundField != nullptr )
		{   
			pEditCtrl = (CBuilderEdit*)pFoundField->FieldCntl;
			pEditCtrl->Increment( nPos );
		}
	}
	CDialog::OnVScroll(nSBCode, nPos, pScrollBar);
}


void CDialogBuilderDlg::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	// TODO: Add your message handler code here and/or call default
	
	CDialog::OnKeyDown(nChar, nRepCnt, nFlags);
}

BOOL CDialogBuilderDlg::PreTranslateMessage(MSG* pMsg) 
{
	// TODO: Add your specialized code here and/or call the base class

#pragma message( "Hack to eliminate arrow problems. Still don't know what causes thsis (REMOVED JH 26.3.2000)")		
/*	if( pMsg->message == WM_KEYDOWN  
		&& ( pMsg->wParam == 40 || pMsg->wParam == 38 || pMsg->wParam == 34 || pMsg->wParam == 33 ))
		return( TRUE );
*/	return CDialog::PreTranslateMessage(pMsg);
}

// Added by Incarnadine to have it redraw the controls
// when the window is resized.
void CDialogBuilderDlg::OnSize(UINT nType, int cx, int cy) 
{
	CDialog::OnSize(nType, cx, cy);
	FormatDialog();	
}

// Added JH 25.3.2000
void CDialogBuilderDlg::OnMouseMove(UINT nFlags, CPoint point)
{
	CDialog::OnMouseMove(nFlags, point);
	if ((nFlags&&MK_LBUTTON )&&(cpDragStart.y!=-1))
		{ // Scroll if needed
		 int ActPos = GetScrollPos(SB_VERT);
		 int Max,Min;
		 GetScrollRange(SB_VERT, &Min,&Max);
	
		 ActPos -= point.y-cpDragStart.y;
		 
		 if (ActPos <0)   ActPos=0;
		 if (ActPos >Max) ActPos=Max;

		 ScrollWindow( 0, WndVScrollPos - ActPos , nullptr, nullptr );
		 WndVScrollPos = ActPos;
		 SetScrollPos( SB_VERT, ActPos, true ) ;
		 cpDragStart=point;
		}
	else
		{ cpDragStart.y=-1;
		}

}

void CDialogBuilderDlg::OnLButtonDown( UINT nFlags, CPoint point)
{
	cpDragStart=point;
}



BOOL CDialogBuilderDlg::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
	int Max,Min;
	GetScrollRange(SB_VERT, &Min,&Max);

	if ((pWnd==this)&&(Max!=0))
	{	::SetCursor(AfxGetApp()->LoadCursor(IDC_HAND_OPEN) );
		return TRUE;
	}

	return CDialog::OnSetCursor(pWnd, nHitTest, message);
}
// EOFJH

//	1.16.05 by tom morris - better support for new color button
LONG CDialogBuilderDlg::OnChangeColor(UINT lParam, LONG wParam)
{
	FieldStruct *pFoundField = nullptr;
	pFoundField = FindFieldById( LOWORD( lParam)  );
	if(pFoundField)
	{
		CMainFrame*	pMainFrm = nullptr;
		CJweDoc*	pDoc = nullptr; 
		PROPERTY_FIELD_TYPE fieldType = pFoundField->Type ;
		UINT dataID = pFoundField->DataId;

		pMainFrm = (CMainFrame*)AfxGetMainWnd() ;

		if (pMainFrm)
		{
			pDoc = pMainFrm->GetCurrentDocument() ;
			if (pDoc)
			{
				jeProperty_Data Data;
				CColorButton	*pColorButton = nullptr;

				pColorButton = (CColorButton*)pFoundField->FieldCntl;
				if (pColorButton)
				{
					if (pColorButton->IsKindOf(RUNTIME_CLASS(CColorButton)))
					{
						Data.Vector.X = GetRValue(pColorButton->GetColor());
						Data.Vector.Y = GetGValue(pColorButton->GetColor());
						Data.Vector.Z = GetBValue(pColorButton->GetColor());
	
						pDoc->SetProperty( dataID, fieldType, &Data );
					}
				}
			}
		}
	}
	return TRUE ;
}






