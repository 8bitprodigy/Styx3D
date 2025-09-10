/****************************************************************************************/
/*  BUILDERBUTTON.CPP                                                                   */
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
#include "jwe.h"
#include "builderbutton.h"


#include"DrawTool.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#include "MainFrm.h"
/////////////////////////////////////////////////////////////////////////////
// CBuilderButton

// Find a better place for this function !!!c
void Rect_FillGradient ( CDC * pWinCDC,RECT * ClientRect,DWORD FromColor, DWORD ToColor )
{
	int   Y=0;
	float R=0,G=0,B=0;
	float DestR=0,DestG=0,DestB=0;
	float RAdd,GAdd,BAdd;
	
	G = float(FromColor & 0xff);
	R = float((FromColor>>8) & 0xff);
	B = float((FromColor>>16) & 0xff);

	DestG = float(ToColor & 0xff);
	DestR = float((ToColor>>8) & 0xff);
	DestB = float((ToColor>>16) & 0xff);

	int XSize = ClientRect->right;

	RAdd = (DestR-R)/XSize;
	GAdd = (DestG-G)/XSize;
	BAdd = (DestB-B)/XSize;
	
	for (int X=0;X<XSize;X++)
		{
		CBrush MyBrush(RGB((int)G,(int)R,(int)B));
		CRect  myRect (Y,0,Y+1,ClientRect->bottom);
		Y++;
		R+=RAdd;
		G+=GAdd;
		B+=BAdd;

		pWinCDC->FillRect(&myRect,&MyBrush);
		}
}

CBuilderButton::CBuilderButton(int DataType, int DataId)
{
	m_DataType = DataType;
	m_DataId = DataId;
	m_state = 0;

}

CBuilderButton::~CBuilderButton()
{
}


BEGIN_MESSAGE_MAP(CBuilderButton, CButton)
	//{{AFX_MSG_MAP(CBuilderButton)
	ON_CONTROL_REFLECT(BN_CLICKED, OnClicked)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBuilderButton message handlers


void CBuilderButton::OnClicked() 
{
	CMainFrame*	pMainFrm = (CMainFrame*)AfxGetMainWnd() ;
	CJweDoc*	pDoc = pMainFrm->GetCurrentDocument() ;
	jeProperty_Data Data;
	CWnd* pWnd = GetParent();

	switch( m_DataType )
	{
	case PROPERTY_CHECK_TYPE:
	case PROPERTY_RADIO_TYPE:
		Data.Bool = GetCheck();
		if( Data.Bool == 2 )
			Data.Bool = 0;
		pDoc->SetProperty( m_DataId, m_DataType, &Data );
		break;

	case PROPERTY_COLOR_PICKER_TYPE:
	{
		COLORREF Color ;
		CColorDialog dlg;

		// get a Color
		if (dlg.DoModal() == IDOK)
		{
			// assign what it was to the current Color
			Color = dlg.GetColor() ;
			Data.Vector.X = GetRValue( Color );
			Data.Vector.Y = GetGValue( Color );
			Data.Vector.Z = GetBValue( Color );
			pDoc->SetProperty( m_DataId, m_DataType, &Data );
		}
	}
	break;

	case PROPERTY_BUTTON_TYPE:
		Data.Bool = TRUE;
		pDoc->SetProperty( m_DataId, m_DataType, &Data );
		break;

	case PROPERTY_GROUP_TYPE:
	case PROPERTY_VEC3D_GROUP_TYPE:
	case PROPERTY_COLOR_GROUP_TYPE:
		pWnd->SendMessage( WM_COMMAND, 1, GetDlgCtrlID() );
		break;
	}
}

void CBuilderButton::SetColor( COLORREF Color)
{
	mCurrentColor = Color;
	// update ourselves
	RedrawWindow();
}// SetColor

void CBuilderButton::DrawColorButton(LPDRAWITEMSTRUCT lpDrawItemStruct) 
{
	RECT ClientRect;

	// get a cdc
	CDC* pDC = CDC::FromHandle( lpDrawItemStruct->hDC );

	// now draw a solid rectangle
	pDC->FillSolidRect( &lpDrawItemStruct->rcItem, mCurrentColor);

	{
		// draw the button's text...
		CString Text;

		this->GetClientRect (&ClientRect);

		this->GetWindowText (Text);
		pDC->DrawText( Text, &ClientRect, DT_CENTER |DT_VCENTER|DT_SINGLELINE ) ;
	}
	// if we have the focus

	if( lpDrawItemStruct->itemState & ODS_FOCUS ) {
		// get a null brush
		CBrush *NullBrush = CBrush::FromHandle((HBRUSH)GetStockObject(NULL_BRUSH)), *OldBrush;

		// select the brush
		OldBrush = pDC->SelectObject( NullBrush );

		// draw a cute rectangle around it
		pDC->Rectangle(&lpDrawItemStruct->rcItem);

		// get old
		pDC->SelectObject( OldBrush );
	}
	

	hIconDown  = (HICON)::LoadImage(AfxGetApp()->m_hInstance,MAKEINTRESOURCE(IDI_DOWN),IMAGE_ICON,0,0,0);
    hIconRight = (HICON)::LoadImage(AfxGetApp()->m_hInstance,MAKEINTRESOURCE(IDI_RIGHT),IMAGE_ICON,0,0,0 );

	if( lpDrawItemStruct->itemState & ODS_SELECTED )	
	{
		  pDC->DrawEdge( &ClientRect, EDGE_SUNKEN, BF_RECT ) ;
		  pDC->DrawState(CPoint(6,(ClientRect.bottom-7)/2),CSize(0,0),hIconRight,DSS_NORMAL,(CBrush*)NULL);
	}

	else if(m_state )
	  	{
		  pDC->DrawEdge( &ClientRect, EDGE_SUNKEN, BF_RECT ) ;
		  pDC->DrawState(CPoint(6,(ClientRect.bottom-7)/2),CSize(0,0),hIconDown,DSS_NORMAL,(CBrush*)NULL);
		}
	else
		{ 		
		  pDC->DrawEdge( &ClientRect, EDGE_RAISED, BF_RECT ) ;
		  pDC->DrawState(CPoint(6,(ClientRect.bottom-7)/2),CSize(0,0),hIconRight,DSS_NORMAL,(CBrush*)NULL);
		}
	DestroyIcon(hIconDown);
	DestroyIcon(hIconRight);

}// DrawItem


//
// Should be removed, they are normaly defined in winuser.h !!!
//
#define COLOR_GRADIENTACTIVECAPTION     27
#define COLOR_GRADIENTINACTIVECAPTION   28

void CBuilderButton::DrawGroupButton(LPDRAWITEMSTRUCT lpDrawItemStruct) 
{
	RECT ClientRect;

	// get a cdc
	CDC* pDC = CDC::FromHandle( lpDrawItemStruct->hDC );

	// now draw a solid rectangle
	CBrush *ColorBrush;


	if( m_state )	
			 { ColorBrush = new CBrush (::GetSysColor(COLOR_CAPTIONTEXT));
			   DWORD ToColor=::GetSysColor(COLOR_GRADIENTACTIVECAPTION);
			   if (ToColor!=0)
					Rect_FillGradient (pDC,&lpDrawItemStruct->rcItem,::GetSysColor(COLOR_ACTIVECAPTION),ToColor);
			   else pDC->FillSolidRect( &lpDrawItemStruct->rcItem, ::GetSysColor(COLOR_ACTIVECAPTION));
			   
			 }
		else { ColorBrush = new CBrush (::GetSysColor(COLOR_INACTIVECAPTIONTEXT));
			   DWORD ToColor=::GetSysColor(COLOR_GRADIENTINACTIVECAPTION);			   
			   if (ToColor!=0)
				   Rect_FillGradient (pDC,&lpDrawItemStruct->rcItem,::GetSysColor(COLOR_INACTIVECAPTION),ToColor);
			   else
				   pDC->FillSolidRect( &lpDrawItemStruct->rcItem, ::GetSysColor(COLOR_INACTIVECAPTION));
			   
			}
	

	// draw the button's text...
	CString Text;

	this->GetClientRect (&ClientRect);

	this->GetWindowText (Text);
	if( m_state )	
		pDC->SetTextColor(::GetSysColor(COLOR_CAPTIONTEXT));
	else
		pDC->SetTextColor(::GetSysColor(COLOR_INACTIVECAPTIONTEXT));

	pDC->SetBkMode(TRANSPARENT);
	pDC->DrawText( Text, &ClientRect, DT_CENTER |DT_VCENTER|DT_SINGLELINE ) ;

	// if we have the focus

	if( lpDrawItemStruct->itemState & ODS_FOCUS ) {
		// get a null brush
		CBrush *NullBrush = CBrush::FromHandle((HBRUSH)GetStockObject(NULL_BRUSH)), *OldBrush;

		// select the brush
		OldBrush = pDC->SelectObject( NullBrush );

		// draw a cute rectangle around it
		pDC->Rectangle(&lpDrawItemStruct->rcItem);

		// get old
		pDC->SelectObject( OldBrush );
	}

    hIconDown  = (HICON)::LoadImage(AfxGetApp()->m_hInstance,MAKEINTRESOURCE(IDI_DOWN),IMAGE_ICON,0,0,0);
    hIconRight = (HICON)::LoadImage(AfxGetApp()->m_hInstance,MAKEINTRESOURCE(IDI_RIGHT),IMAGE_ICON,0,0,0 );

	if( lpDrawItemStruct->itemState & ODS_SELECTED )	
	{
		  pDC->DrawEdge( &ClientRect, EDGE_SUNKEN, BF_RECT ) ;
		  pDC->DrawState(CPoint(6,(ClientRect.bottom-7)/2),CSize(0,0),hIconRight,DSS_MONO,ColorBrush);
	}

	else if(m_state )
	  	{
		  pDC->DrawEdge( &ClientRect, EDGE_SUNKEN, BF_RECT ) ;
		  pDC->DrawState(CPoint(6,(ClientRect.bottom-7)/2),CSize(0,0),hIconDown,DSS_MONO,ColorBrush);
		}
	else
		{ 		
		  pDC->DrawEdge( &ClientRect, EDGE_RAISED, BF_RECT ) ;
		  pDC->DrawState(CPoint(6,(ClientRect.bottom-7)/2),CSize(0,0),hIconRight,DSS_MONO,ColorBrush);
		}

	delete ColorBrush;
	DestroyIcon(hIconDown);
	DestroyIcon(hIconRight);

}// DrawGroupButton


void CBuilderButton::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct) 
{
	if( m_DataType == PROPERTY_COLOR_GROUP_TYPE )
		DrawColorButton( lpDrawItemStruct );
	else 
		if( ( m_DataType == PROPERTY_GROUP_TYPE )||
		    ( m_DataType == PROPERTY_VEC3D_GROUP_TYPE )||
		    ( m_DataType == PROPERTY_TIME_GROUP_TYPE )
			)
		{ DrawGroupButton( lpDrawItemStruct );
		}
	else
		CButton::DrawItem( lpDrawItemStruct );

}

void CBuilderButton::SetState(UINT nFlags) 
{
	 m_state =nFlags;
}
