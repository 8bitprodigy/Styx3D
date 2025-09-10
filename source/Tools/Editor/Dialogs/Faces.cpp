/****************************************************************************************/
/*  FACES.CPP                                                                           */
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

#include "Doc.h"
#include "jeFaceInfo.h"
#include "jwe.h"
#include "MainFrm.h"
#include "MfcUtil.h"
#include "Units.h"

#include "Faces.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CFaces dialog


CFaces::CFaces(CWnd* pParent /*=NULL*/)
	: CDialog(CFaces::IDD, pParent)
{
	//{{AFX_DATA_INIT(CFaces)
	m_csiAngle = _T("");
	m_csfShiftU = _T("");
	m_csfShiftV = _T("");
	m_csfDrawScaleU = _T("");
	m_csfDrawScaleV = _T("");
	m_csfLMapScaleU = _T("");
	m_csfLMapScaleV = _T("");
	m_Alpha = 0.0f;
	//}}AFX_DATA_INIT
}


void CFaces::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CFaces)
	DDX_Control(pDX, FACE_CK_MIRROR, m_CkMirror);
	DDX_Control(pDX, FACE_CK_TRANSPARENT, m_ckTranparent);
	DDX_Control(pDX, FACE_CK_PORTAL, m_ckPortal);
	DDX_Control(pDX, FACE_CK_GOURAUD, m_ckGouraud);
	DDX_Text(pDX, FACE_ED_ANGLE, m_csiAngle);
	DDV_MaxChars(pDX, m_csiAngle, 6);
	DDX_Text(pDX, FACE_ED_TEXTUREX, m_csfShiftU);
	DDV_MaxChars(pDX, m_csfShiftU, 10);
	DDX_Text(pDX, FACE_ED_TEXTUREY, m_csfShiftV);
	DDV_MaxChars(pDX, m_csfShiftV, 10);
	DDX_Text(pDX, FACE_ED_DRAWSCALEX, m_csfDrawScaleU);
	DDV_MaxChars(pDX, m_csfDrawScaleU, 10);
	DDX_Text(pDX, FACE_ED_DRAWSCALEY, m_csfDrawScaleV);
	DDV_MaxChars(pDX, m_csfDrawScaleV, 10);
	DDX_Text(pDX, FACE_ED_LIGHTMAPX, m_csfLMapScaleU);
	DDV_MaxChars(pDX, m_csfLMapScaleU, 10);
	DDX_Text(pDX, FACE_ED_LIGHTMAPY, m_csfLMapScaleV);
	DDV_MaxChars(pDX, m_csfLMapScaleV, 10);
	DDX_Text(pDX, FACE_ED_TRANSPARENT, m_Alpha);
	DDV_MinMaxFloat(pDX, m_Alpha, 0.f, 255.f);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CFaces, CDialog)
	//{{AFX_MSG_MAP(CFaces)
	ON_WM_VSCROLL()
	ON_EN_KILLFOCUS(FACE_ED_TEXTUREX, OnKillfocusEdTexturex)
	ON_BN_CLICKED(FACE_CK_GOURAUD, OnCkGouraud)
	ON_EN_KILLFOCUS(FACE_ED_ANGLE, OnKillfocusEdAngle)
	ON_EN_KILLFOCUS(FACE_ED_DRAWSCALEX, OnKillfocusEdDrawscalex)
	ON_EN_KILLFOCUS(FACE_ED_DRAWSCALEY, OnKillfocusEdDrawscaley)
	ON_EN_KILLFOCUS(FACE_ED_LIGHTMAPX, OnKillfocusEdLightmapx)
	ON_EN_KILLFOCUS(FACE_ED_LIGHTMAPY, OnKillfocusEdLightmapy)
	ON_EN_KILLFOCUS(FACE_ED_TEXTUREY, OnKillfocusEdTexturey)
	ON_BN_CLICKED(FACE_CK_INVISIBLE, OnCkInvisible)
	ON_BN_CLICKED(FACE_CK_PORTAL, OnCkPortal)
	ON_EN_KILLFOCUS(FACE_ED_TRANSPARENT, OnKillfocusEdTransparent)
	ON_BN_CLICKED(FACE_CK_TRANSPARENT, OnCkTransparent)
	ON_BN_CLICKED(FACE_CK_MIRROR, OnCkMirror)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

#define SHIFTU_MIN	(-256.0f)
#define SHIFTU_MAX	(256.0f)
#define SHIFTU_INC	(0.1f)
#define DRAWSX_MIN	(0.0f)
#define DRAWSX_MAX	(20.0f)
#define DRAWSX_INC	(0.1f)
#define LIGHTMX_MIN	(0.0f)
#define LIGHTMX_MAX	(6.0f)
#define LIGHTMX_INC	(0.1f)
#define ROTATE_MIN	(0.0f)
#define ROTATE_MAX	(359.0f)
#define ROTATE_INC	(1.0f)

/////////////////////////////////////////////////////////////////////////////
// CFaces message handlers

BOOL CFaces::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	PositionDialogUnderTabs( this ) ;
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}// OnInitDialog

void CFaces::Update(CJweDoc *pDoc)
{
	jeFaceInfo	FaceInfo ;
	int32		BlankFieldFlag ;
	ASSERT( pDoc->IsKindOf( RUNTIME_CLASS(CJweDoc)) ) ;

	pDoc->GetFaceInfo( &FaceInfo, &BlankFieldFlag ) ;

	FillFields( &FaceInfo, ~BlankFieldFlag ) ;

}//Update

void CFaces::FillFields(jeFaceInfo *pFaceInfo, int32 BlankFieldFlag)
{
	ASSERT( pFaceInfo != NULL ) ;

	UpdateData( true ) ;
	
	// Clear Everything
	m_csiAngle = _T("") ;
	m_csfShiftU = _T("") ;
	m_csfShiftV = _T("") ;
	m_csfDrawScaleU = _T("") ;
	m_csfDrawScaleV = _T("") ;
	m_csfLMapScaleU = _T("") ;
	m_csfLMapScaleV = _T("") ;
	m_ckGouraud.SetCheck( false ) ;
	m_ckTranparent.SetCheck( false ) ;
	m_ckPortal.SetCheck( false ) ;

	if( BlankFieldFlag & FACE_FIELD_GOURAUD ) 
		m_ckGouraud.SetCheck( (pFaceInfo->Flags & FACEINFO_GOURAUD) ? true : false ) ;
	else
		m_ckGouraud.SetCheck( 2 ) ;
	
	if( BlankFieldFlag & FACE_FIELD_VIS_PORTAL )
		m_ckPortal.SetCheck( (pFaceInfo->Flags & FACEINFO_VIS_PORTAL) ? true : false ) ;
	else
		m_ckPortal.SetCheck( 2 ) ;

	if( BlankFieldFlag & FACE_FIELD_INVISIBLE )
		m_ckTranparent.SetCheck( (pFaceInfo->Flags & FACEINFO_TRANSPARENT) ? true : false ) ;
	else
		m_ckTranparent.SetCheck( 2 ) ;

	if( BlankFieldFlag & FACE_FIELD_MIRROR )
		m_CkMirror.SetCheck( (pFaceInfo->Flags & FACEINFO_MIRROR) ? true : false ) ;
	else
		m_CkMirror.SetCheck( 2 ) ;

	if( BlankFieldFlag & FACE_FIELD_ROTATE )
	{
		m_csiAngle.Format( "%d", UNITS_RADIANS_TO_DEGREES( pFaceInfo->Rotate ) ) ;
	}
	if( BlankFieldFlag & FACE_FIELD_SHIFTU )
	{
		m_csfShiftU.Format( "%5.2f", pFaceInfo->ShiftU ) ;
		TrimString( m_csfShiftU ) ;
	}
	if( BlankFieldFlag & FACE_FIELD_SHIFTV )
	{
		m_csfShiftV.Format( "%5.2f", pFaceInfo->ShiftV ) ;
		TrimString( m_csfShiftV ) ;
	}
	if( BlankFieldFlag & FACE_FIELD_DRAWSCALEU )
	{
		m_csfDrawScaleU.Format( "%5.2f", pFaceInfo->DrawScaleU ) ;
		TrimString( m_csfDrawScaleU ) ;
	}
	if( BlankFieldFlag & FACE_FIELD_DRAWSCALEV )
	{
		m_csfDrawScaleV.Format( "%5.2f", pFaceInfo->DrawScaleV ) ;
		TrimString( m_csfDrawScaleV ) ;
	}
	if( BlankFieldFlag & FACE_FIELD_LMAPSCALEU )
	{
		m_csfLMapScaleU.Format( "%5.2f", pFaceInfo->LMapScaleU ) ;
		TrimString( m_csfLMapScaleU ) ;
	}
	if( BlankFieldFlag & FACE_FIELD_LMAPSCALEV )
	{
		m_csfLMapScaleV.Format( "%5.2f", pFaceInfo->LMapScaleV ) ;
		TrimString( m_csfLMapScaleV ) ;
	}

	if( BlankFieldFlag & FACE_FIELD_ALPHA )
	{
		m_Alpha = pFaceInfo->Alpha  ;
	}
	

	UpdateData( false ) ;
}// FillFields

void CFaces::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	jeFaceInfo	FaceInfo ;
	CJweDoc	*	pDoc ;

	// UP/DOWN (Spin) controls come here as well as scroll-bars
	if( nSBCode == SB_ENDSCROLL  )
	{
		pDoc = ((CMainFrame*)AfxGetMainWnd())->GetCurrentDocument() ;
		ASSERT( pDoc != NULL ) ;
		UpdateData( true ) ;

		switch( pScrollBar->GetDlgCtrlID() )
		{
			case FACE_SP_TEXTUREX :
				FaceInfo.ShiftU = Increment( (jeFloat)atof( m_csfShiftU ), SHIFTU_MIN, SHIFTU_MAX, SHIFTU_INC, nPos ) ;
				m_csfShiftU.Format( "%5.2f", FaceInfo.ShiftU ) ;
				TrimString( m_csfShiftU ) ;
				pScrollBar->SendMessage( UDM_SETPOS, 0, 0L ) ;
				pDoc->SetFaceInfo( &FaceInfo, FACE_FIELD_SHIFTU ) ;
				break ;
			case FACE_SP_TEXTUREY :
				FaceInfo.ShiftV = Increment( (jeFloat)atof( m_csfShiftV ), SHIFTU_MIN, SHIFTU_MAX, SHIFTU_INC, nPos ) ;
				m_csfShiftV.Format( "%5.2f", FaceInfo.ShiftV ) ;
				TrimString( m_csfShiftV ) ;
				pScrollBar->SendMessage( UDM_SETPOS, 0, 0L ) ;
				pDoc->SetFaceInfo( &FaceInfo, FACE_FIELD_SHIFTV ) ;
				break ;
			case FACE_SP_DRAWSCALEX :
				FaceInfo.DrawScaleU = Increment( (jeFloat)atof( m_csfDrawScaleU), DRAWSX_MIN, DRAWSX_MAX, DRAWSX_INC, nPos ) ;
				m_csfDrawScaleU.Format( "%5.2f", FaceInfo.DrawScaleU ) ;
				TrimString( m_csfDrawScaleU ) ;
				pScrollBar->SendMessage( UDM_SETPOS, 0, 0L ) ;
				pDoc->SetFaceInfo( &FaceInfo, FACE_FIELD_DRAWSCALEU ) ;
				break ;
			case FACE_SP_DRAWSCALEY :
				FaceInfo.DrawScaleV = Increment( (jeFloat)atof( m_csfDrawScaleV), DRAWSX_MIN, DRAWSX_MAX, DRAWSX_INC, nPos ) ;
				m_csfDrawScaleV.Format( "%5.2f", FaceInfo.DrawScaleV ) ;
				TrimString( m_csfDrawScaleV ) ;
				pScrollBar->SendMessage( UDM_SETPOS, 0, 0L ) ;
				pDoc->SetFaceInfo( &FaceInfo, FACE_FIELD_DRAWSCALEV ) ;
				break ;
			case FACE_SP_LIGHTMAPX :
				FaceInfo.LMapScaleU = Increment( (jeFloat)atof( m_csfLMapScaleU), LIGHTMX_MIN, LIGHTMX_MAX, LIGHTMX_INC, nPos ) ;
				m_csfLMapScaleU.Format( "%5.2f", FaceInfo.LMapScaleU ) ;
				TrimString( m_csfLMapScaleU ) ;
				pScrollBar->SendMessage( UDM_SETPOS, 0, 0L ) ;
				pDoc->SetFaceInfo( &FaceInfo, FACE_FIELD_LMAPSCALEU ) ;
				break ;
			case FACE_SP_LIGHTMAPY :
				FaceInfo.LMapScaleV = Increment( (jeFloat)atof( m_csfLMapScaleV), LIGHTMX_MIN, LIGHTMX_MAX, LIGHTMX_INC, nPos ) ;
				m_csfLMapScaleV.Format( "%5.2f", FaceInfo.LMapScaleV ) ;
				TrimString( m_csfLMapScaleV ) ;
				pScrollBar->SendMessage( UDM_SETPOS, 0, 0L ) ;
				pDoc->SetFaceInfo( &FaceInfo, FACE_FIELD_LMAPSCALEV ) ;
				break ;
			case FACE_SP_ANGLE :
				FaceInfo.Rotate = Increment( (jeFloat)atoi( m_csiAngle ), ROTATE_MIN, ROTATE_MAX, ROTATE_INC, nPos ) ;
				m_csiAngle.Format( "%d", (int)FaceInfo.Rotate ) ;
				TrimString( m_csiAngle ) ;
				pScrollBar->SendMessage( UDM_SETPOS, 0, 0L ) ;
				FaceInfo.Rotate = UNITS_DEGREES_TO_RADIANS( FaceInfo.Rotate ) ;
				pDoc->SetFaceInfo( &FaceInfo, FACE_FIELD_ROTATE ) ;
				break ;

			default :
				ASSERT( 0 ) ;
				break ;
		}
		UpdateData( false ) ;
	}
}// OnVScroll

jeFloat CFaces::Increment(jeFloat fCur, jeFloat fMin, jeFloat fMax, jeFloat fInc, jeBoolean bDown )
{
	if( bDown )
		fCur -= fInc ;
	else
		fCur += fInc ;

	if( fCur < fMin )
		fCur = fMin ;
	else if( fCur > fMax )
		fCur = fMax ;
	return fCur ;
}// Increment

// HANDLE KILL FOCUS MESSAGES

void CFaces::OnKillfocusEdTexturex() 
{
	jeFaceInfo	FaceInfo ;
	CJweDoc	*	pDoc = ((CMainFrame*)AfxGetMainWnd())->GetCurrentDocument();
	ASSERT( pDoc ) ;

	UpdateData( true ) ;
	FaceInfo.ShiftU = (jeFloat)atof( m_csfShiftU ) ;
	m_csfShiftU.Format( "%5.2f", FaceInfo.ShiftU ) ;
	TrimString( m_csfShiftU ) ;
	UpdateData( false ) ;

	pDoc->SetFaceInfo( &FaceInfo, FACE_FIELD_SHIFTU ) ;
}// OnKillfocusEdTexturex

void CFaces::OnKillfocusEdTexturey() 
{
	jeFaceInfo	FaceInfo ;
	CJweDoc	*	pDoc = ((CMainFrame*)AfxGetMainWnd())->GetCurrentDocument();
	ASSERT( pDoc ) ;

	UpdateData( true ) ;
	FaceInfo.ShiftV = (jeFloat)atof( m_csfShiftV ) ;
	m_csfShiftV.Format( "%5.2f", FaceInfo.ShiftV ) ;
	TrimString( m_csfShiftV ) ;
	UpdateData( false ) ;

	pDoc->SetFaceInfo( &FaceInfo, FACE_FIELD_SHIFTV ) ;
}// OnKillfocusEdTexturey

void CFaces::OnKillfocusEdAngle() 
{
	jeFaceInfo	FaceInfo ;
	CJweDoc	*	pDoc = ((CMainFrame*)AfxGetMainWnd())->GetCurrentDocument();
	ASSERT( pDoc ) ;

	UpdateData( true ) ;
	FaceInfo.Rotate = UNITS_DEGREES_TO_RADIANS( (jeFloat)atoi( m_csiAngle ) ) ;
	pDoc->SetFaceInfo( &FaceInfo, FACE_FIELD_ROTATE ) ;
}// OnKillfocusEdAngle

void CFaces::OnKillfocusEdDrawscalex() 
{
	jeFaceInfo	FaceInfo ;
	CJweDoc	*	pDoc = ((CMainFrame*)AfxGetMainWnd())->GetCurrentDocument();
	ASSERT( pDoc ) ;

	UpdateData( true ) ;
	FaceInfo.DrawScaleU = (jeFloat)atof( m_csfDrawScaleU ) ;
	m_csfDrawScaleU.Format( "%5.2f", FaceInfo.DrawScaleU ) ;
	TrimString( m_csfDrawScaleU) ;
	UpdateData( false ) ;

	pDoc->SetFaceInfo( &FaceInfo, FACE_FIELD_DRAWSCALEU ) ;
}// OnKillfocusEdDrawscalex

void CFaces::OnKillfocusEdDrawscaley() 
{
	jeFaceInfo	FaceInfo ;
	CJweDoc	*	pDoc = ((CMainFrame*)AfxGetMainWnd())->GetCurrentDocument();
	ASSERT( pDoc ) ;

	UpdateData( true ) ;
	FaceInfo.DrawScaleV = (jeFloat)atof( m_csfDrawScaleV ) ;
	m_csfDrawScaleV.Format( "%5.2f", FaceInfo.DrawScaleV ) ;
	TrimString( m_csfDrawScaleV ) ;
	UpdateData( false ) ;

	pDoc->SetFaceInfo( &FaceInfo, FACE_FIELD_DRAWSCALEV ) ;
}// OnKillfocusEdDrawscaley

void CFaces::OnKillfocusEdLightmapx() 
{
	jeFaceInfo	FaceInfo ;
	CJweDoc	*	pDoc = ((CMainFrame*)AfxGetMainWnd())->GetCurrentDocument();
	ASSERT( pDoc ) ;

	UpdateData( true ) ;
	FaceInfo.LMapScaleU = (jeFloat)atof( m_csfLMapScaleU ) ;
	m_csfLMapScaleU.Format( "%5.2f", FaceInfo.LMapScaleU ) ;
	TrimString( m_csfLMapScaleU ) ;
	UpdateData( false ) ;

	pDoc->SetFaceInfo( &FaceInfo, FACE_FIELD_LMAPSCALEU ) ;
}// OnKillfocusEdLightmapx

void CFaces::OnKillfocusEdLightmapy() 
{
	jeFaceInfo	FaceInfo ;
	CJweDoc	*	pDoc = ((CMainFrame*)AfxGetMainWnd())->GetCurrentDocument();
	ASSERT( pDoc ) ;

	UpdateData( true ) ;
	FaceInfo.LMapScaleV = (jeFloat)atof( m_csfLMapScaleV ) ;
	m_csfLMapScaleV.Format( "%5.2f", FaceInfo.LMapScaleV ) ;
	TrimString( m_csfLMapScaleV ) ;
	UpdateData( false ) ;

	pDoc->SetFaceInfo( &FaceInfo, FACE_FIELD_LMAPSCALEV ) ;
}// OnKillfocusEdLightmapy

// HANDLE CHECK-BOX CLICKS

void CFaces::OnCkGouraud() 
{
	jeFaceInfo	FaceInfo ;
	CJweDoc	*	pDoc = ((CMainFrame*)AfxGetMainWnd())->GetCurrentDocument();
	ASSERT( pDoc ) ;

	if( m_ckGouraud.GetCheck( ) )
		FaceInfo.Flags = FACEINFO_GOURAUD ;
	else
		FaceInfo.Flags = 0 ;

	pDoc->SetFaceInfo( &FaceInfo, FACE_FIELD_GOURAUD ) ;
}// OnCkGouraud

void CFaces::OnCkInvisible() 
{
}// OnCkInvisible

void CFaces::OnCkPortal() 
{
	jeFaceInfo	FaceInfo ;
	CJweDoc	*	pDoc = ((CMainFrame*)AfxGetMainWnd())->GetCurrentDocument();
	ASSERT( pDoc ) ;

	if( m_ckPortal.GetCheck( ) )
		FaceInfo.Flags = FACEINFO_VIS_PORTAL ;
	else
		FaceInfo.Flags = 0 ;

	pDoc->SetFaceInfo( &FaceInfo, FACE_FIELD_VIS_PORTAL ) ;
}// OnCkPortal

void CFaces::OnKillfocusEdTransparent() 
{
	jeFaceInfo	FaceInfo ;
	CJweDoc	*	pDoc = ((CMainFrame*)AfxGetMainWnd())->GetCurrentDocument();
	ASSERT( pDoc ) ;

	UpdateData( true ) ;
	FaceInfo.Alpha = m_Alpha ;
	m_Alpha = FaceInfo.Alpha ;
	UpdateData( false ) ;

	pDoc->SetFaceInfo( &FaceInfo, FACE_FIELD_ALPHA ) ;
	
}

void CFaces::OnCkTransparent() 
{
	jeFaceInfo	FaceInfo ;
	CJweDoc	*	pDoc = ((CMainFrame*)AfxGetMainWnd())->GetCurrentDocument();
	ASSERT( pDoc ) ;

	if( m_ckTranparent.GetCheck( ) )
		FaceInfo.Flags = FACEINFO_TRANSPARENT ;
	else
		FaceInfo.Flags = 0 ;

	pDoc->SetFaceInfo( &FaceInfo, FACE_FIELD_INVISIBLE ) ;
	
}

void CFaces::OnCkMirror() 
{
	jeFaceInfo	FaceInfo ;
	CJweDoc	*	pDoc = ((CMainFrame*)AfxGetMainWnd())->GetCurrentDocument();
	ASSERT( pDoc ) ;

	if( m_CkMirror.GetCheck( ) )
		FaceInfo.Flags = FACEINFO_MIRROR ;
	else
		FaceInfo.Flags = 0 ;

	pDoc->SetFaceInfo( &FaceInfo, FACE_FIELD_MIRROR ) ;
	
}
