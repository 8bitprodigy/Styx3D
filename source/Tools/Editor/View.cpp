/****************************************************************************************/
/*  VIEW.CPP                                                                            */
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
 On: 12/27/99 8:58:41 PM
 Comments: 1) Added menu items and handlers for "Clone Selected"
----------------------------------------------------------------------------------------*/


#include "stdafx.h"

#include "jwe.h"
#include "MainFrm.h"
#include "Ram.h"
#include "Util.h"

#include "View.h"
//	by trilobite	Jan. 2011
//#include "Memdc.h"
#include <afxglobals.h>	//	contains CMemDC class

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define AUTOSCROLL_MARGIN		4
#define AUTOSCROLL_TIMER		1
#define AUTOSCROLL_PERIOD		100
#define	AUTOSCROLL_DISTANCE		8

int CJweView::m_CXDRAG = 2 ;
int CJweView::m_CYDRAG = 2 ;

/////////////////////////////////////////////////////////////////////////////
// CJweView

IMPLEMENT_DYNCREATE(CJweView, CView)

BEGIN_MESSAGE_MAP(CJweView, CView)
	//{{AFX_MSG_MAP(CJweView)
	ON_WM_ERASEBKGND()
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_SIZE()
	ON_WM_LBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
	ON_WM_SETCURSOR()
	ON_WM_RBUTTONUP()
	ON_WM_MOUSEWHEEL()
	ON_COMMAND(IDM_VIEW_ZOOMIN, OnViewZoomin)
	ON_UPDATE_COMMAND_UI(IDM_VIEW_ZOOMIN, OnUpdateViewZoomin)
	ON_COMMAND(IDM_VIEW_ZOOMOUT, OnViewZoomout)
	ON_UPDATE_COMMAND_UI(IDM_VIEW_ZOOMOUT, OnUpdateViewZoomout)
	ON_COMMAND(IDM_VIEW_CENTERSELCTION, OnViewCenterselction)
	ON_WM_TIMER()
	ON_WM_RBUTTONDOWN()
	//	tom morris may 2005
	ON_MESSAGE(WM_JWEVIEW_AUTOSAVETIMER,OnAutosaveTimer)
	//	end tom morris may 2005
//---------------------------------------------------
// Added DJT
//---------------------------------------------------
	ON_COMMAND(ID_EDIT_CLONE, OnEditClone)
	ON_UPDATE_COMMAND_UI(ID_EDIT_CLONE, OnUpdateEditClone)
	ON_WM_MBUTTONDOWN()
	ON_WM_MBUTTONUP()
//---------------------------------------------------
// End DJT
//---------------------------------------------------

	//}}AFX_MSG_MAP
	// Standard printing commands
	//ON_COMMAND(ID_FILE_PRINT, CView::OnFilePrint)
	//ON_COMMAND(ID_FILE_PRINT_DIRECT, CView::OnFilePrint)
	//ON_COMMAND(ID_FILE_PRINT_PREVIEW, CView::OnFilePrintPreview)

	ON_COMMAND_RANGE( IDM_VIEW_TOP, IDM_VIEW_RESERVED4, OnViewType)
	ON_UPDATE_COMMAND_UI_RANGE( IDM_VIEW_TOP, IDM_VIEW_RESERVED4, OnUpdateViewType)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CJweView construction/destruction

CJweView::CJweView()	: m_nViewType(0), m_pOrtho(NULL), m_bCaptured(false), m_ScrollType(VIEW_SCROLL_NONE),
							m_bDragging(false), m_Mode(VIEW_MODE_NONE),
							m_bInit(false), m_bPanning(false)
							

{
	m_CXDRAG = ::GetSystemMetrics( SM_CXDRAG ) ;
	m_CYDRAG = ::GetSystemMetrics( SM_CYDRAG ) ;
}


CJweView::~CJweView()
{
}

BOOL CJweView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CView::PreCreateWindow(cs);
}



//////////////////////////////////////////////////////////////////////////////////////
//	PostNcDestroy	-	tom morris may 2005
//	
//////////////////////////////////////////////////////////////////////////////////////
void CJweView::PostNcDestroy() 
{
	//	three views use this class. Run this code only for a single view.
	//	otherwise this will be done three times for each document
	if (m_nViewType == IDM_VIEW_TOP)	
	{
		//	AutoSave destruction
		CFileFind fileFind;
		BOOL	bFoundFile = FALSE;
		CString strAutosaveDir = Settings_GetAutosaveDirectory();

		CString fileWildCard = strAutosaveDir +  _T("\\*.JDB");

		bFoundFile = fileFind.FindFile(fileWildCard.GetBuffer(1),0);
		if (bFoundFile)
		{
			CString	message = _T("jDesigner Autosave / AutoRecovery services\n\n");
			message += _T("jDesigner is about to delete BACKUP file(s)\n");
			message +=_T("saved during this session by jDesigner's AutoSave feature.\n\n");
			message += _T("Choose OK to delete backup file(s) (Recommended)\n");
			message += _T("Choose CANCEL to leave the file(s) undisturbed\n\n");

			int RESULT = AfxMessageBox(message, MB_OKCANCEL | MB_ICONEXCLAMATION);
			switch (RESULT)
			{
			case IDOK:
				{
					while(fileFind.FindNextFile() !=0)
					{
						DeleteFile(fileFind.GetFilePath().GetBuffer(1));
					}	

					DeleteFile(fileFind.GetFilePath().GetBuffer(1));
					break;
				}
			case IDCANCEL:
				break;		//	do nothing and abort close
			}
		}
	}
	CView::PostNcDestroy();
}

//////////////////////////////////////////////////////////////////////////////////////
//	OnAutosaveTimer	-	tom morris may 2005
//	
//////////////////////////////////////////////////////////////////////////////////////
LRESULT CJweView::OnAutosaveTimer(WPARAM w, LPARAM l)
{
	//	***********************************************************
	//	Disable Autosave while testing stuff that crashes on saving
	//	return;
	//	***********************************************************

	// BEGIN - Disable auto save option - paradoxnj 8/11/2005
	//  Check if disabled
	if (Settings_GetAutosaveDisabled() == TRUE)
		return 1;
	// END - Disable auto save option - paradoxnj 8/11/2005

	//	three views use this class. Run this code only for a single view.
	//	otherwise this will be done three times for each call
	if (m_nViewType == IDM_VIEW_TOP)	
	{
		CMainFrame	*pMainFrame = NULL;
		pMainFrame = (CMainFrame*)AfxGetMainWnd();
		CString		strAutosaveDir = Settings_GetAutosaveDirectory();

		if (pMainFrame)
		{
			CString strAutosaving = _T("Autosaving...");	
			//	indicate autosaving progress on status bar	
			pMainFrame->ProgressBarBegin(strAutosaving);
			pMainFrame->ProgressBarSetRange(200, strAutosaving);
			pMainFrame->ProgressBarSetStep(10, strAutosaving);

			pMainFrame->ProgressBarStep(strAutosaving);		//	10

			CFileFind fileFind;
			//	check to see if autosave dir exists
#if _MFC_VER < 0x0700
			// Compile with VC 6
			if(fileFind.FindFile(strAutosaveDir.GetBuffer(0))==FALSE) 
			{
				//	create directory
				if(CreateDirectory(strAutosaveDir.GetBuffer(0),NULL) ==0)
				{
					;
				}
			}
#else
			if(fileFind.FindFile(strAutosaveDir.GetBuffer())==FALSE) 
			{
				//	create directory
				if(CreateDirectory(strAutosaveDir.GetBuffer(),NULL) ==0)
				{
					;
				}
			}
#endif
			pMainFrame->ProgressBarStep(strAutosaving);	//	20	
			pMainFrame->ProgressBarStep(strAutosaving);	//	30	

			// Now, check for document name and extension
			CString		findStringJDB = ".JDB";
			CString		findStringJ3D = ".j3d";
			CString		fname;
			CString		fileTitle = GetDocument()->GetTitle();	//	get our file name
			CString		filePath = GetDocument()->GetPathName();

			if (fileTitle.Find(findStringJDB, 0) > 1)	//	is it a *.JDB file?
			{
				fname = (strAutosaveDir+"\\"+fileTitle); 
			}

			if (filePath.Find(findStringJ3D, 0) > 1)	//	is it a *.j3d file?
			{
				//	delete the j3d extension
				fileTitle.Delete(fileTitle.Find(findStringJ3D, 0),4);
				//	add the TLB extension
				fname = (strAutosaveDir+"\\"+fileTitle +".JDB"); 
			}
			pMainFrame->ProgressBarStep(strAutosaving);	//	40	
			if (fname.GetLength() == 0)
				fname = (strAutosaveDir+"\\"+fileTitle +".JDB"); 

			pMainFrame->ProgressBarStep(strAutosaving);	//	50	
			pMainFrame->ProgressBarStep(strAutosaving);	//	60	

			pMainFrame->ProgressBarStep(strAutosaving);	//	70	

			GetDocument()->OnSaveDocument(fname);  //call store function

			pMainFrame->ProgressBarStep(strAutosaving);		//	80	
			pMainFrame->ProgressBarStep(strAutosaving);		//	90
			pMainFrame->ProgressBarStep(strAutosaving);		//	100
			pMainFrame->ProgressBarEnd();
			//	return to normal message reporting on the status bar
			pMainFrame->GetStatusBar()->SetPaneText(0, "Ready");
		}

	}	//	if (m_nViewType == IDM_VIEW_TOP)...
	return 1;
}


void CJweView::OnViewType(UINT nID)
{
	Ortho_ViewType ovt ;

	m_nViewType = nID ;		// View Type (Top, Side, Front) also Menu ID

	switch( nID )
	{
	case IDM_VIEW_TOP :		ovt = Ortho_ViewTop ;	break ;
	case IDM_VIEW_FRONT :	ovt = Ortho_ViewFront ;	break ;
	case IDM_VIEW_SIDE:		ovt = Ortho_ViewSide ;	break ;
	default:
		ASSERT( 0 ) ;
		ovt = Ortho_ViewTop ;
	}
	Ortho_SetViewType( m_pOrtho, ovt ) ;

	Ortho_ResetSettings( m_pOrtho, Ortho_GetWidth( m_pOrtho ), Ortho_GetHeight( m_pOrtho ) ) ;
//	Render_ResizeView (VCam, Width, Height);
//	Ortho_SetCameraPos( m_pOrtho, &SaveCameraPos);
//	Ortho_SetAnglesRPY(VCam, &SaveOrientation);
//	Ortho_SetZoom (VCam, ZoomFactor);


}// OnViewType
	

// Added JH 11.3.2000
Ortho	*CJweView::GetOrtho()
{ return m_pOrtho;
}


void CJweView::OnUpdateViewType( CCmdUI* pCmdUI )
{
	pCmdUI->Enable( TRUE ) ;
	pCmdUI->SetCheck( pCmdUI->m_nID == m_nViewType ) ;

}// OnUpdateViewType





/////////////////////////////////////////////////////////////////////////////
// CJweView drawing

#define TRACE_PERF

//	begin tom morris feb 2005
void CJweView::OnDraw(CDC* pDC)
{
	CRect	r ;
	CJweDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	const jeExtBox * pNewBrushBounds;

#ifdef TRACE_PERF
    char msg[80];
    DWORD start = GetTickCount();
#endif

	//	prepare for flicker-free view drawing
	//	by trilobite	Jan. 2011
	//CMemDC pMemDC(pDC);

	GetClientRect( &r ) ;

	//	by trilobite	Jan. 2011
	CMemDC pMemDC(*pDC, &r);
	CDC* pLocalDC = &pMemDC.GetDC();
	//

	CBrush MyBrush(Settings_GetGridBk());
	
	//	by trilobite	Jan. 2011
	//pMemDC->FillRect(&r,&MyBrush);
	pLocalDC->FillRect(&r,&MyBrush);
	//
	CBrush		SelBrush( RGB(255,0,0) ) ;

	if( !pDoc->m_bLoaded )
		return;
	//	by trilobite	Jan. 2011
	/*
	pDoc->DrawGrid(pMemDC, m_pOrtho);
	pDoc->DrawConstructorLine(pMemDC, m_pOrtho );
	pDoc->DrawObjects( pMemDC, m_pOrtho );
	pDoc->DrawSelected( pMemDC, m_pOrtho );
	if( m_Mode == VIEW_MODE_ROTATE_HANDLE  || m_Mode == VIEW_MODE_ROTATE_SUBSELECTION )
	{
		DrawRotateBox(pMemDC->GetSafeHdc());
		pDoc->DrawSelectAxis( m_pOrtho, pMemDC->GetSafeHdc());
	}
	else
		pDoc->DrawSelectBounds( pMemDC, m_pOrtho );

	pDoc->DrawOrthoName( pMemDC, m_pOrtho );
	*/
	//	continuing
	pDoc->DrawGrid(pLocalDC, m_pOrtho);
	pDoc->DrawConstructorLine(pLocalDC, m_pOrtho );
	pDoc->DrawObjects( pLocalDC, m_pOrtho );
	pDoc->DrawSelected( pLocalDC, m_pOrtho );
	if( m_Mode == VIEW_MODE_ROTATE_HANDLE  || m_Mode == VIEW_MODE_ROTATE_SUBSELECTION )
	{
		DrawRotateBox(pLocalDC->GetSafeHdc());
		pDoc->DrawSelectAxis( m_pOrtho, pLocalDC->GetSafeHdc());
	}
	else
		pDoc->DrawSelectBounds( pLocalDC, m_pOrtho );

	pDoc->DrawOrthoName( pLocalDC, m_pOrtho );



	pNewBrushBounds = pDoc->GetNewBrushBounds();
	
	if( jeExtBox_IsValid( pNewBrushBounds ) )
	{
		Ortho_WorldToViewRect( m_pOrtho, pNewBrushBounds, (Rect*)&r );
		//BY TRILOBITE
		//pMemDC->FrameRect( &r, &SelBrush ) ;
		//pDoc->PrintRectDimensions (pMemDC,m_pOrtho,pNewBrushBounds);	// Added JH 3.3.2000
		pLocalDC->FrameRect( &r, &SelBrush ) ;
		pDoc->PrintRectDimensions (pLocalDC,m_pOrtho,pNewBrushBounds);	// Added JH 3.3.2000

	}
	else
	{
		if( m_Mode == VIEW_MODE_SELECT_RECT )
		{

			r.left	= m_ptAnchor.x ;
			r.top	= m_ptAnchor.y ;
			r.right	= m_ptLastMouse.x ;
			r.bottom = m_ptLastMouse.y ;
			r.NormalizeRect() ;

			//	by trilobite	Jan. 2011
			//pMemDC->FrameRect( &r, &SelBrush ) ;
			pLocalDC->FrameRect( &r, &SelBrush ) ;
			//
		}
		if( this==GetParentFrame()->GetActiveView() )
		{
			GetClientRect( &r ) ;
			CBrush RedBrush( RGB(255,0,0) ) ;
			//	by trilobite	Jan. 2011
			//pMemDC->FrameRect( &r, &RedBrush );
			pLocalDC->FrameRect( &r, &RedBrush );
			//
		}
	}

#ifdef TRACE_PERF
    sprintf(msg, "CJweView::OnDraw Perf=%u\n", GetTickCount() - start);
    OutputDebugString(msg);
#endif
}// OnDraw



//void CJweView::OnDraw(CDC* pDC)
//{
//	CRect	r ;
//	CJweDoc* pDoc = GetDocument();
//	ASSERT_VALID(pDoc);
//	const jeExtBox * pNewBrushBounds;
//
//	CRect totalAreaWnd;				//	prepare for flicker-free view drawing
//	GetClientRect(&totalAreaWnd);
//	//	Create a memory bufffer DC and draw to THAT
//	CMemDC memDC(pDC, totalAreaWnd, totalAreaWnd, &m_ViewBuffer );
//
//
//	if (m_ViewBuffer.IsDirty())		//	If it's empty or uninitialized, let's draw to it
//	{
//		GetClientRect( &r ) ;
//		CBrush MyBrush(Settings_GetGridBk());
//		memDC.GetSafeCDC()->FillRect(&r,&MyBrush);
//
//		CBrush		SelBrush( RGB(255,0,0) ) ;
//
//		if( !pDoc->m_bLoaded )
//			return;
//		pDoc->DrawGrid(memDC.GetSafeCDC(), m_pOrtho);
//		pDoc->DrawConstructorLine( memDC.GetSafeCDC(), m_pOrtho );
//		pDoc->DrawObjects( memDC.GetSafeCDC(), m_pOrtho );
//		pDoc->DrawSelected( memDC.GetSafeCDC(), m_pOrtho );
//		if( m_Mode == VIEW_MODE_ROTATE_HANDLE  || m_Mode == VIEW_MODE_ROTATE_SUBSELECTION )
//		{
//			DrawRotateBox( memDC.m_hDC );
//			pDoc->DrawSelectAxis( m_pOrtho, memDC.m_hDC );
//		}
//		else
//			pDoc->DrawSelectBounds( memDC.GetSafeCDC(), m_pOrtho );
//		pDoc->DrawOrthoName( memDC.GetSafeCDC(), m_pOrtho );
//
//		pNewBrushBounds = pDoc->GetNewBrushBounds();
//		if( jeExtBox_IsValid( pNewBrushBounds ) )
//		{
//			Ortho_WorldToViewRect( m_pOrtho, pNewBrushBounds, (Rect*)&r );
//			memDC.GetSafeCDC()->FrameRect( &r, &SelBrush ) ;
//
//			pDoc->PrintRectDimensions (memDC.GetSafeCDC(),m_pOrtho,pNewBrushBounds);	// Added JH 3.3.2000
//		}else
//			if( m_Mode == VIEW_MODE_SELECT_RECT )
//			{
//
//				r.left	= m_ptAnchor.x ;
//				r.top	= m_ptAnchor.y ;
//				r.right	= m_ptLastMouse.x ;
//				r.bottom = m_ptLastMouse.y ;
//				r.NormalizeRect() ;
//
//				memDC.GetSafeCDC()->FrameRect( &r, &SelBrush ) ;
//
//			}
//			if( this==GetParentFrame()->GetActiveView() )
//			{
//				GetClientRect( &r ) ;
//				CBrush RedBrush( RGB(255,0,0) ) ;
//				memDC.GetSafeCDC()->FrameRect( &r, &RedBrush );
//			}
//			//	once all drawing is done,
//			if (!memDC.CopyToScreen(0))
//				MessageBeep(0);;					//	flip the buffer to the screen
//	}
//}// OnDraw

//	end tom morris feb 2005



/*	ORIG
/////////////////////////////////////////////////////////////////////////////
// CJweView drawing

void CJweView::OnDraw(CDC* pDC)
{
	CRect	r ;
	CJweDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	const jeExtBox * pNewBrushBounds;

	// Added 28.3.2000 Double Buffering :)
	CDC  *pdrawDC = CDC::FromHandle(pDC->m_hDC);
	CMemDC memDC(pdrawDC);
	CDC  *pDCNew = &memDC;
		
	GetClientRect( &r ) ;
	CBrush MyBrush(Settings_GetGridBk());
	memDC->FillRect(&r,&MyBrush);
	// EOF JH

	CBrush		SelBrush( RGB(255,0,0) ) ;

	if( !pDoc->m_bLoaded )
		return;
	pDoc->DrawGrid( pDCNew, m_pOrtho);
	pDoc->DrawConstructorLine( pDCNew, m_pOrtho );
	pDoc->DrawObjects( pDCNew, m_pOrtho );
	pDoc->DrawSelected( pDCNew, m_pOrtho );
	if( m_Mode == VIEW_MODE_ROTATE_HANDLE  || m_Mode == VIEW_MODE_ROTATE_SUBSELECTION )
	{
		DrawRotateBox( pDCNew->m_hDC );
		pDoc->DrawSelectAxis( m_pOrtho, pDCNew->m_hDC );
	}
	else
		pDoc->DrawSelectBounds( pDCNew, m_pOrtho );
	pDoc->DrawOrthoName( pDCNew, m_pOrtho );

	pNewBrushBounds = pDoc->GetNewBrushBounds();
	if( jeExtBox_IsValid( pNewBrushBounds ) )
	{
		Ortho_WorldToViewRect( m_pOrtho, pNewBrushBounds, (Rect*)&r );
		pDCNew->FrameRect( &r, &SelBrush ) ;

		pDoc->PrintRectDimensions (pDCNew,m_pOrtho,pNewBrushBounds);	// Added JH 3.3.2000
	}else
	if( m_Mode == VIEW_MODE_SELECT_RECT )
	{
			
		r.left	= m_ptAnchor.x ;
		r.top	= m_ptAnchor.y ;
		r.right	= m_ptLastMouse.x ;
		r.bottom = m_ptLastMouse.y ;
		r.NormalizeRect() ;

		pDCNew->FrameRect( &r, &SelBrush ) ;

	}
	if( this==GetParentFrame()->GetActiveView() )
	{
		GetClientRect( &r ) ;
		CBrush RedBrush( RGB(255,0,0) ) ;
		pDCNew->FrameRect( &r, &RedBrush );
	}

}// OnDraw
*/

/////////////////////////////////////////////////////////////////////////////
// CJweView printing
/*
BOOL CJweView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation
	return DoPreparePrinting(pInfo);
}

void CJweView::OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo)
{
	// TODO: add extra initialization before printing
}

void CJweView::OnEndPrinting(CDC* pDC, CPrintInfo* pInfo)
{
	// TODO: add cleanup after printing
}
*/
/////////////////////////////////////////////////////////////////////////////
// CJweView diagnostics

#ifdef _DEBUG
void CJweView::AssertValid() const
{
	CView::AssertValid();
}

void CJweView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CJweDoc* CJweView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CJweDoc)));
	return (CJweDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CJweView message handlers

BOOL CJweView::OnEraseBkgnd(CDC* pDC) 
{
	//	tom morris feb 2005
	return false;
	//	end tom morris feb 2005

	CRect rect ;
	GetClientRect( &rect ) ;
	pDC->FillSolidRect( &rect, Settings_GetGridBk( ) ) ;
	return true ;
}// OnEraseBkgnd


int32 CJweView::GetViewSigniture()
{
	switch( Ortho_GetViewType( m_pOrtho ) ) 
	{
	case Ortho_ViewFront:
		return( 'FRNT' );

	case Ortho_ViewSide:
		return( 'SIDE' );

	case Ortho_ViewTop:
		return( 'OTOP' );

	default:
		ASSERT(0 );
		break;
	}
	return( 0 );
}

void CJweView::OnInitialUpdate() 
{
	jeVec3d	Angles = { 0.0f, 0.0f, 0.0f } ;
	jeVec3d CameraPos = { 0.0f, 0.0f, 0.0f } ;
	CView::OnInitialUpdate();
	
	// Get saved state information from the DOC
	Ortho_SetZoom( m_pOrtho, 1.0f ) ;
	Ortho_SetAngles( m_pOrtho, &Angles ) ;
	Ortho_SetCameraPos( m_pOrtho, &CameraPos ) ;

	m_bInit = true ;	// This flag prevents activate view from setting a doc that isn't init'ed
	((CMainFrame*)AfxGetMainWnd())->SetCurrentDocument( GetDocument() ) ;

	WndReg_RegisterWindow( GetSafeHwnd(), GetViewSigniture() );
	// Update grid status bar (DOC)

}// OnInitialUpdate

int CJweView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	m_pOrtho = Ortho_Create( ) ;
	if( m_pOrtho == NULL )
		return -1 ;
	// TODO: Add your specialized creation code here
	
	return 0;
}

void CJweView::OnDestroy() 
{
	CView::OnDestroy();
	
	if( m_pOrtho )
		Ortho_Destroy( &m_pOrtho ) ;
}// OnDestroy

void CJweView::OnSize(UINT nType, int cx, int cy) 
{
	CView::OnSize(nType, cx, cy);
	
	Ortho_ResizeView( m_pOrtho, cx, cy);
	Ortho_UpdateWorldBounds( m_pOrtho ) ;

}// OnSize

void CJweView::OnLButtonDown(UINT nFlags, CPoint point) 
{
	CJweDoc	* pDoc = GetDocument() ;
	if( m_Mode == VIEW_MODE_DRAG_BRUSH_HEIGHT )
	{

		ASSERT( pDoc->isPlaceBrushMode() );

		ShowCursor(TRUE);
		ReleaseCapture();
		m_bCaptured =  false;
		m_bDragging = false;
		pDoc->PlaceBrush( JE_FALSE );
		m_Mode = VIEW_MODE_NONE;
		return;
	}

	if( pDoc->isPlaceLightMode() )
		m_Mode = VIEW_MODE_PLACELIGHT;
	else
	if( pDoc->isPlaceBrushMode() )
		m_Mode = VIEW_MODE_PLACEBRUSH;

	SetCapture( ) ;
	m_bCaptured = true ;
	m_ptAnchor = point ;
	m_ptLastMouse = point ;
	
	CView::OnLButtonDown(nFlags, point);
}// OnLButtonDown

int32 CJweView::GetAutoScrollRegion( POINT * ptCursor )
{
	CRect rect ;
	int32 ScrollType = VIEW_SCROLL_NONE;


	if( m_Mode == VIEW_MODE_DRAG_BRUSH_HEIGHT )
		return( VIEW_SCROLL_NONE );
	GetClientRect( &rect ) ;

	if( rect.PtInRect( *ptCursor ) )
		return( VIEW_SCROLL_NONE );

	if( ptCursor->x < rect.left + AUTOSCROLL_MARGIN )
		ScrollType |= VIEW_SCROLL_LEFT;
	else
	if( ptCursor->x > rect.right - AUTOSCROLL_MARGIN )
		ScrollType |= VIEW_SCROLL_RIGHT;

	if( ptCursor->y < rect.top + AUTOSCROLL_MARGIN )
		ScrollType |= VIEW_SCROLL_UP;
	else
	if( ptCursor->y > rect.bottom - AUTOSCROLL_MARGIN )
		ScrollType |= VIEW_SCROLL_DOWN;
	return( ScrollType );
}

void CJweView::SetAutoScroll( int32 ScrollRegion )
{
	if( ScrollRegion == VIEW_SCROLL_NONE )
	{
		m_ScrollType = VIEW_SCROLL_NONE;
		KillTimer( AUTOSCROLL_TIMER );
	}
	else
	{
		if( m_ScrollType == VIEW_SCROLL_NONE )
		{
			SetTimer( AUTOSCROLL_TIMER, AUTOSCROLL_PERIOD, NULL );
			m_ScrollType = ScrollRegion;
		}
	}
}

void CJweView::SetUpRotateBox(jeExtBox  *pSelBounds)
{
	float dx, dy;

	jeExtBox_GetTranslation( pSelBounds, &m_Center3d );
	Ortho_WorldToView( m_pOrtho, &m_Center3d, (Point*)&m_SelCenter );
	dx = (float)(m_ptAnchor.x - m_SelCenter.x);
	dy = (float)(m_ptAnchor.y - m_SelCenter.y);
	m_RotateRadius = (float)sqrt( double((dx*dx) + (dy*dy)) );
	m_RotateBox.left = m_SelCenter.x - (int)m_RotateRadius;
	m_RotateBox.right = m_SelCenter.x + (int)m_RotateRadius;
	m_RotateBox.top = m_SelCenter.y - (int)m_RotateRadius;
	m_RotateBox.bottom = m_SelCenter.y + (int)m_RotateRadius;
}

void CJweView::DrawRotateBox( HDC hDC)
{
	HBRUSH				hOldBrush ;
	COLORREF			coBackGround ;
	COLORREF			coOld ;
	Rect				Box;
	Point				CursorHandle;
	float					dx, dy;
	float				Dist;
	

	coBackGround = Settings_GetSelectedBk() ;
	hOldBrush = SelectBrush( hDC, GetStockObject( NULL_BRUSH ) ) ;
	coOld = SetBkColor( hDC, coBackGround ) ;
	Ellipse(
		hDC,
		m_RotateBox.left,
		m_RotateBox.top,
		m_RotateBox.right,
		m_RotateBox.bottom
		);
	SetBkColor( hDC, coOld ) ;
	SelectBrush( hDC, hOldBrush ) ;

	Box.Left = m_ptAnchor.x - 4;
	Box.Right = m_ptAnchor.x + 4;
	Box.Top = m_ptAnchor.y - 4;
	Box.Bottom = m_ptAnchor.y + 4;
	Ellipse(
		hDC,
		Box.Left,
		Box.Top,
		Box.Right,
		Box.Bottom
		);

	dx = (float)(m_ptLastMouse.x - m_SelCenter.x);
	dy = (float)(m_ptLastMouse.y - m_SelCenter.y);
	Dist = (float)sqrt( double((dx*dx) + (dy*dy)) );
	CursorHandle.X = (int)(dx*m_RotateRadius/Dist + m_SelCenter.x);
	CursorHandle.Y = (int)(dy*m_RotateRadius/Dist + m_SelCenter.y);

	Box.Left = CursorHandle.X - 4;
	Box.Right = CursorHandle.X + 4;
	Box.Top = CursorHandle.Y - 4;
	Box.Bottom = CursorHandle.Y + 4;
	Ellipse(
		hDC,
		Box.Left,
		Box.Top,
		Box.Right,
		Box.Bottom
		);

}


void CJweView::SetBeginDragViewMode(POINT ptCursor)
{
	jeBoolean bHasSelection;
	jeBoolean bHasSubSelection;
	jeBoolean		bShiftHeld ;
	jeBoolean		bControlHeld ;
	jeExtBox		WorldBox ;
	jeExtBox		SubSelBox ;
	SELECT_HANDLE	Handle ;
	SELECT_HANDLE	SubHandle ;
	DOC_HANDLE_MODE	HandleMode;
	int32			SubModFlags = 0;
	CJweDoc	* pDoc = GetDocument() ;
	DOC_CONSTRUCTORS Constructor;

	bShiftHeld = Util_IsKeyDown( VK_SHIFT ) ;
	bControlHeld = Util_IsKeyDown( VK_CONTROL ) ;
	bHasSelection = pDoc->HasSelections( &WorldBox );
	if( bHasSelection )
		Handle = pDoc->ViewPointHandle( m_pOrtho, (Point*)&m_ptAnchor, &WorldBox ) ;
	else
		Handle = Select_None;
	bHasSubSelection = pDoc->HasSubSelections( &SubSelBox );
	if( bHasSubSelection )
		SubModFlags = pDoc->SubSelXFormModFlags();
	SubHandle = pDoc->SubViewPointHandle( m_pOrtho, (Point*)&m_ptAnchor, &SubSelBox ) ;
	Constructor = pDoc->ViewPointConstructor( m_pOrtho, (Point*)&m_ptAnchor );

	
	ASSERT( m_Mode == VIEW_MODE_NONE  || m_Mode == VIEW_MODE_PLACEBRUSH);
	
	if( pDoc->isPlaceBrushMode() )							//Are we in Drag New Brush Rect Mode
		m_Mode = VIEW_MODE_DRAG_BRUSH_RECT;
	else
	if( bHasSubSelection && (SubModFlags & SubSelect_Rotate )&& SubHandle != Select_None )
	{
		jeExtBox SelBounds;

		pDoc->BeginRotateSub(  ) ;
		if( pDoc->HasSubSelections( &SelBounds ) )
			SetUpRotateBox(&SelBounds);
		m_Mode = VIEW_MODE_ROTATE_SUBSELECTION;
	}
	else
	if( bHasSubSelection && (SubModFlags & SubSelect_Move )&& Ortho_IsViewPointInWorldBox( m_pOrtho, m_ptAnchor.x, m_ptAnchor.y, &SubSelBox ))
	{
		pDoc->BeginMoveSub(  ) ;
		m_Mode = VIEW_MODE_MOVE_SUBSELECTION;
	}
	else
	if( pDoc->IsVertexManipulationMode() && bShiftHeld )	//Are we Dragging a Vertext
	{
		m_Mode = VIEW_MODE_DRAG_VERTEX;
		pDoc->BeginMoveVerts( m_pOrtho ) ;
	}
	else
	if( bHasSelection && Handle != Select_None )			//Are we Dragging a handle
	{
		if( pDoc->BeginMoveHandle( m_pOrtho, Handle, &HandleMode ) )
		{
			switch( HandleMode )
			{
			case DOC_HANDLE_SIZE:
				m_Mode = VIEW_MODE_SIZE_HANDLE;
				break;

			case DOC_HANDLE_ROTATE:
				jeExtBox SelBounds;

				m_Mode = VIEW_MODE_ROTATE_HANDLE;
				if( pDoc->HasSelections( &SelBounds ) )
					SetUpRotateBox(&SelBounds);
				break;
			case DOC_HANDLE_SHEAR:
				m_Mode = VIEW_MODE_SHEAR_HANDLE;
				break;
			}
			m_SizeType = Handle ;
		}
	}	
	else													//Are we Dragging a selction
	if( bHasSelection && Ortho_IsViewPointInWorldBox( m_pOrtho, m_ptAnchor.x, m_ptAnchor.y, &WorldBox ) )
	{						
		m_Mode = VIEW_MODE_MOVE_SELECTION;
		m_SizeType = Select_NearestCornerHandle( m_pOrtho, (Point*)&ptCursor, &WorldBox ) ;
		pDoc->BeginMove( m_pOrtho, m_SizeType, bShiftHeld ) ;
	}
	else
	if( Constructor != DOC_NO_CONSTRUCTOR )
	{
		switch( Constructor )
		{
		case DOC_VERTICAL_CONSTRUCTOR:
			m_Mode = VIEW_DRAG_VCONSTRUCTOR;
			break;

		case DOC_HORIZONTAL_CONSTRUCTOR:
			m_Mode = VIEW_DRAG_HCONSTRUCTOR;
			break;

		case DOC_BOTH_CONSTRUCTOR:
			m_Mode = VIEW_DRAG_BCONSTRUCTOR;
			break;

		default:
			ASSERT(0);
			break;
		}			
	}
	else
		m_Mode = VIEW_MODE_SELECT_RECT;
	
}
void CJweView::Pan( jeVec3d *pWorldDistance )
{
	m_bPanning = true;
   	jeVec3d_Scale( pWorldDistance, -1.0f, pWorldDistance ) ;
	Ortho_MoveCamera( m_pOrtho, pWorldDistance ) ;
	Invalidate(false);		//TRUE
}

jeBoolean CJweView::IsBeginDrag( POINT ptCursor )
{
	return(		m_Mode != VIEW_MODE_PLACELIGHT	&& //We are not placing a light
				false == m_bDragging			&& //We are not already draging
		( abs( ptCursor.x - m_ptAnchor.x) > m_CXDRAG || //Either we have moved enough in the X
		   abs( ptCursor.y - m_ptAnchor.y) > m_CYDRAG )); // Or moved enough in the Y
}

void CJweView::Drag( POINT ptCursor, jeVec3d *pWorldDistance )
{
	CJweDoc	* pDoc = GetDocument() ;

	switch( m_Mode )
	{
	case VIEW_MODE_MOVE_SELECTION:
		pDoc->MoveSelected( m_SizeType, pWorldDistance ) ;
		break;

	case VIEW_MODE_MOVE_SUBSELECTION:
		pDoc->MoveSelectedSub( m_SizeType, pWorldDistance ) ;
		break;

	case VIEW_MODE_ROTATE_SUBSELECTION:
		pDoc->RotateSelectedSub( m_pOrtho,  (Point*)&ptCursor, (Point*)&m_ptAnchor ) ;
		break;

	case VIEW_MODE_ROTATE_HANDLE:
		{
		
			pDoc->MoveHandle( m_pOrtho, pWorldDistance, m_SizeType, (Point*)&ptCursor, (Point*)&m_ptAnchor, &m_Center3d ) ;
			{
				RECT dirtyRect = m_RotateBox;
				Rect_Inflate( (Rect*)&dirtyRect, 4, 4 );
				InvalidateRect( &dirtyRect, false ); // TRUE
			}
		}
		break;

	case VIEW_MODE_SIZE_HANDLE:
	case VIEW_MODE_SHEAR_HANDLE:
		pDoc->MoveHandle( m_pOrtho, pWorldDistance, m_SizeType, (Point*)&ptCursor, (Point*)&m_ptAnchor, &m_Center3d ) ;
		break;

	case VIEW_MODE_DRAG_VERTEX:
		pDoc->MoveVerts( m_pOrtho, pWorldDistance ) ;
		break;

	case VIEW_MODE_DRAG_BRUSH_HEIGHT:
		{
			CPoint ScreenPt;
			m_ptVertualMouse.x += ptCursor.x - m_ptAnchor.x;
			m_ptVertualMouse.y += ptCursor.y - m_ptAnchor.y;

			pDoc->SetNewBrushHeight( m_pOrtho, (Point*)&m_ptVertualMouse, (Point*)&m_ptAnchor);
			ScreenPt = m_ptAnchor;
			ClientToScreen( &ScreenPt ) ;
			SetCursorPos( ScreenPt.x, ScreenPt.y );
		}
		break;

	case VIEW_MODE_DRAG_BRUSH_RECT:
		{
			pDoc->SetNewBrushBound( m_pOrtho, (Point*)&ptCursor, (Point*)&m_ptAnchor );
		}
		break;

	case VIEW_DRAG_HCONSTRUCTOR:
		pDoc->MoveConstructor( m_pOrtho, DOC_HORIZONTAL_CONSTRUCTOR, (Point*)&ptCursor, (Point*)&m_ptAnchor );
		break;

	case VIEW_DRAG_VCONSTRUCTOR:
		pDoc->MoveConstructor( m_pOrtho, DOC_VERTICAL_CONSTRUCTOR, (Point*)&ptCursor, (Point*)&m_ptAnchor );
		break;

	case VIEW_DRAG_BCONSTRUCTOR:
		pDoc->MoveConstructor( m_pOrtho, DOC_BOTH_CONSTRUCTOR, (Point*)&ptCursor, (Point*)&m_ptAnchor );
		break;

	case VIEW_MODE_SELECT_RECT:
			Invalidate(false);		//TRUE
			break;
	}
}

void CJweView::OnMouseMove(UINT nFlags, CPoint point) 
{
	jeVec3d			WorldDistance ;
	POINT			ptCursor ;
	int				dx, dy ;


	if( m_bCaptured )
	{	
		::GetCursorPos( &ptCursor ) ;
		ScreenToClient( &ptCursor ) ;

		OnSetCursor( this, HTCLIENT, 0 ) ;
		dx = ptCursor.x - m_ptLastMouse.x ;
		dy = ptCursor.y - m_ptLastMouse.y ;
		if( dx==0 && dy == 0 )
			return;

		Ortho_ViewToWorldDistance( m_pOrtho, dx, dy, &WorldDistance ) ;

		if( Util_IsKeyDown( VK_SPACE ) )
		{
			if( m_bZooming )
			{
				DoZoom (-dy * 0.01f) ;
				m_ptLastMouse = ptCursor ;
				return;
			}

			Pan( &WorldDistance );
			m_ptLastMouse = ptCursor ;
			return;
		}

// Added JH
		if (m_bPanning && (eMouseRightButton)Settings_GetMouse_RightBut()==mbrPaning)
			{
				Pan( &WorldDistance );
				m_ptLastMouse = ptCursor ;
				return;
			}
// EOF JH

		m_bPanning = false;
		if( m_bZooming )
		{
			m_bZooming = false;
			m_bCaptured = false;
			ReleaseCapture();
			return;
		}
		if( IsBeginDrag( ptCursor ) )
		{
			SetBeginDragViewMode( ptCursor );
			m_bDragging = true ;

		}

		if( m_bDragging )
		{
			int32 ScrollRegion;

			ScrollRegion = GetAutoScrollRegion( &ptCursor );
			if( ScrollRegion != m_ScrollType )
			{
				SetAutoScroll( ScrollRegion );
			}
			Drag(ptCursor, &WorldDistance);
		}
		m_ptLastMouse = ptCursor ;

	}// Mouse Move Captured

#define WITH_DJT_HOTSELECT
#ifdef WITH_DJT_HOTSELECT
	else if (MouseSettings_GetHotSelect())
	{
		CView * pActiveView = GetParentFrame()->GetActiveView();

		// Mouse not captured, 
		// if this view is not active, then activate it
		if (this != pActiveView)
		{
			GetParentFrame()->SetActiveView(this, TRUE);
		}
	}
#endif


	CView::OnMouseMove(nFlags, point);
}// OnMouseMove

void CJweView::DragEnd(CPoint point)
{
	CJweDoc* pDoc = GetDocument() ;
	jeExtBox	WorldBounds ;
	jeBoolean	bControlHeld ;

	bControlHeld = Util_IsKeyDown( VK_CONTROL ) ;
	switch( m_Mode )
	{
	case VIEW_MODE_DRAG_VERTEX:
		pDoc->EndMoveVerts() ;
		m_Mode = VIEW_MODE_NONE;
		break;

	case VIEW_MODE_MOVE_SELECTION:
		pDoc->EndMove( ) ;
		m_Mode = VIEW_MODE_NONE;
		break;

	case VIEW_MODE_MOVE_SUBSELECTION:
		pDoc->EndMoveSub( ) ;
		m_Mode = VIEW_MODE_NONE;
		break;

	case VIEW_MODE_ROTATE_SUBSELECTION:
		pDoc->EndRotateSub( ) ;
		m_Mode = VIEW_MODE_NONE;
		{
			RECT dirtyRect = m_RotateBox;
			Rect_Inflate( (Rect*)&dirtyRect, 4, 4 );
			InvalidateRect( &dirtyRect, false ); // TRUE
		}
		break;

	case VIEW_MODE_ROTATE_HANDLE:
		{
			RECT dirtyRect = m_RotateBox;
			Rect_Inflate( (Rect*)&dirtyRect, 4, 4 );
			InvalidateRect( &dirtyRect, false ); //TRUE
		}

	case VIEW_MODE_SIZE_HANDLE:
	case VIEW_MODE_SHEAR_HANDLE:
		pDoc->EndMoveHandle( ) ;
		m_Mode = VIEW_MODE_NONE;
		break;

	case VIEW_MODE_DRAG_BRUSH_RECT:
		SetCapture( ) ;
		m_bCaptured = true ;
		m_ptAnchor = point ;
		m_ptVertualMouse = point;
		m_ptLastMouse = point ;
		m_bDragging = true ;
		m_Mode = VIEW_MODE_DRAG_BRUSH_HEIGHT;
		ShowCursor(FALSE);
		break;

	case VIEW_DRAG_HCONSTRUCTOR:
	case VIEW_DRAG_VCONSTRUCTOR:
	case VIEW_DRAG_BCONSTRUCTOR:
		m_Mode = VIEW_MODE_NONE;
		break;

	case VIEW_MODE_SELECT_RECT:
		Invalidate(false) ;		//TRUE // Get rid of rectangle -- might sel nothing
		Ortho_ViewToWorldRect( m_pOrtho, (Point*)&m_ptAnchor, (Point*)&m_ptLastMouse, &WorldBounds ) ;
		pDoc->RectangleSelect( &WorldBounds, bControlHeld ) ;
		m_Mode = VIEW_MODE_NONE;
		break;

	}
}

void CJweView::OnLButtonUp(UINT nFlags, CPoint point) 
{
	jeBoolean	bControlHeld ;
	jeVec3d		World ;

	if( m_bCaptured )
	{
		CJweDoc* pDoc = GetDocument() ;

		bControlHeld = Util_IsKeyDown( VK_CONTROL ) ;

		ReleaseCapture() ;
		m_bCaptured = false ;
		if( m_ScrollType != VIEW_SCROLL_NONE )
			KillTimer(AUTOSCROLL_TIMER );
		m_ScrollType = VIEW_SCROLL_NONE;

   		if( m_bDragging )
		{
			m_bDragging = false ;
			DragEnd(point);
			pDoc->UpdateStats();
		}
		else 
		{
			Ortho_ViewToWorld( m_pOrtho, point.x, point.y, &World ) ;
			if( m_Mode == VIEW_MODE_NONE )
			{
				if( !m_bPanning )
					pDoc->Select( m_pOrtho, (Point*)&point, LEVEL_TOGGLE, bControlHeld ) ;
			}
			else
			{
				pDoc->PlaceAtPoint( m_pOrtho, (Point*)&point, JE_FALSE );
				m_Mode = VIEW_MODE_NONE;
			}
		}

		if( m_bPanning )
		{
			m_bPanning = false;
		}
	}
	CView::OnLButtonUp(nFlags, point);
}// OnLButtonUp

BOOL CJweView::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
	POINT		ptCursor ;
	CJweDoc* pDoc = GetDocument() ;

	if( HTCLIENT == nHitTest )
	{
		if(
			( m_bCaptured && Util_IsKeyDown( VK_SPACE ) )	||// Panning?
// Added JH
			(m_bPanning && (eMouseRightButton)Settings_GetMouse_RightBut()==mbrPaning )	// Panning?
// EOF JH
			)
		{
			::SetCursor( ::LoadCursor( 0, IDC_SIZEALL ) ) ;
		}
		else
		{
			{

				::GetCursorPos( &ptCursor ) ;
				ScreenToClient( &ptCursor ) ;

				pDoc->SetCursor( m_pOrtho, &ptCursor ) ;
			}
		}
		return true ;
	}
	return CView::OnSetCursor(pWnd, nHitTest, message);
}// OnSetCursor
// TEST CODE
void CJweView::OnRButtonUp(UINT nFlags, CPoint point) 
{
	CJweDoc* pDoc = GetDocument() ;

	if( m_Mode == VIEW_MODE_PLACELIGHT ||  m_Mode == VIEW_MODE_PLACEBRUSH )
	{
		pDoc->PlaceAtPoint( m_pOrtho,(Point *)&point, JE_TRUE );
		m_Mode = VIEW_MODE_NONE;
	}
	if( m_bZooming )
	{
		ReleaseCapture( ) ;
		m_bCaptured = false ;
		m_bZooming = false;
	}
// Added JH
	if( m_bPanning )
		{
			ReleaseCapture( ) ;
			m_bCaptured = false ;
			m_bPanning = false;
		}

// EOF JH
	CView::OnRButtonUp(nFlags, point);
}
// END TEST CODE

//---------------------------------------------------
// Edited DJT
// 
// Mouse wheel's action contingent upon
// MouseSettings_GetWheelState()
//---------------------------------------------------
BOOL CJweView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt) 
{
	eMouseWheel  eState = MouseSettings_GetWheelState();

	switch (eState)
	{
		case mwZoom:
			if( zDelta > 0 )
				DoZoom (0.1f) ;
			else
				DoZoom (-0.1f) ;
			break;

		case mwDisabled:
		default:
			break;
	}
	return CView::OnMouseWheel(nFlags, zDelta, pt);
}// OnMouseWheel
//---------------------------------------------------
// End DJT
//---------------------------------------------------


void CJweView::DoZoom( jeFloat fZoomInc )
{
	Ortho_ZoomChange( m_pOrtho, fZoomInc ) ;

	InvalidateRect( NULL, false ) ;		//TRUE

}// DoZoom

void CJweView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
	jeExtBox *	pWorldBounds ;
	Rect		ViewRect ;
	if( pHint == NULL )
	{
		Invalidate(false) ;	// EMPTY
		return ;
	}
	
	pWorldBounds = (jeExtBox*)pHint ;
	if( Ortho_TestWorldToViewRect( m_pOrtho, pWorldBounds, &ViewRect ) )
	{
		// Adjust for handles
		InflateRect( (LPRECT)&ViewRect, HALFHANDLESIZE, HALFHANDLESIZE ) ;
		InvalidateRect( (LPCRECT)&ViewRect, false ) ; //TRUE
	}
	lHint;
	pSender;
}// OnUpdate

void CJweView::OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView) 
{
	Invalidate( false ) ;	//TRUE // Force our red-frame on active to draw
	CView::OnActivateView(bActivate, pActivateView, pDeactiveView);
	if( m_bInit && this == pActivateView )
	{
		//((CMainFrame*)AfxGetMainWnd())->SetCurrentDocument( GetDocument() ) ;
	}
}// OnActivateView

void CJweView::OnViewZoomin() 
{
	DoZoom( 0.1f ) ;
}// OnViewZoomin

void CJweView::OnUpdateViewZoomin(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable( true ) ;
}//OnUpdateViewZoomin

void CJweView::OnViewZoomout() 
{
	DoZoom( -0.1f ) ;
}// OnViewZoomout

void CJweView::OnUpdateViewZoomout(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable( true ) ;
}// OnUpdateViewZoomout

void CJweView::SetCameraPos( jeVec3d * Pos )
{
	Ortho_SetCameraPos( m_pOrtho, Pos );
	Invalidate(false);	// TRUE
}

void CJweView::AbortMode()
{
	if( m_bCaptured )
	{
		ReleaseCapture() ;
		m_bCaptured = false ;
		ShowCursor( TRUE );
		if( m_ScrollType != VIEW_SCROLL_NONE )
			KillTimer(AUTOSCROLL_TIMER );
		m_ScrollType = VIEW_SCROLL_NONE;
		m_Mode = VIEW_MODE_NONE;
		if( m_bDragging )
		{
			m_bDragging = false ;
		}
	}
	Invalidate(false);	//TRUE
}

void CJweView::OnViewCenterselction() 
{
	CJweDoc * pDoc;

	pDoc = GetDocument();
	ASSERT( pDoc != NULL );

	pDoc->CenterViewsOnSelection(  );
}

void CJweView::OnTimer(UINT nIDEvent) 
{
	if( nIDEvent == AUTOSCROLL_TIMER )
	{
		int x = 0;
		int y = 0;
		jeVec3d WorldDistance;
		CPoint point;
		
		point = m_ptLastMouse;
		if( m_ScrollType & VIEW_SCROLL_UP )
			y = -AUTOSCROLL_DISTANCE;
		else
		if( m_ScrollType & VIEW_SCROLL_DOWN )
			y = AUTOSCROLL_DISTANCE;
		
		if( m_ScrollType & VIEW_SCROLL_LEFT )
			x = -AUTOSCROLL_DISTANCE;
		else
		if( m_ScrollType & VIEW_SCROLL_RIGHT )
			x = AUTOSCROLL_DISTANCE;


		m_ptLastMouse.x -= x;
		m_ptLastMouse.y -= y;
		m_ptAnchor.x -= x;
		m_ptAnchor.y -= y;
		m_SelCenter.x -= x;
		m_SelCenter.y -= y;
		m_RotateBox.left -= x;
		m_RotateBox.right -= x;
		m_RotateBox.top -= y;
		m_RotateBox.bottom -= y;
		Ortho_ViewToWorldDistance( m_pOrtho, x, y, &WorldDistance ) ;
		Ortho_MoveCamera( m_pOrtho, &WorldDistance ) ;
		OnMouseMove(0, point);
		Invalidate(false);	//FALSE
	}
	CView::OnTimer(nIDEvent);
}

void CJweView::OnRButtonDown(UINT nFlags, CPoint point) 
{
	CJweDoc* pDoc = GetDocument() ;

	Rect MenuRect = { 4, 4, 128, 20 };  // Need to figure out true text box

// Added JH
	if ((eMouseRightButton)Settings_GetMouse_RightBut()==mbrPaning)
		{
			SetCapture( ) ;
			m_bCaptured = true ;
//			m_bZooming = true;
			m_bPanning = true;
			m_ptLastMouse = point ;
		}
// EOF JH

	if( Rect_IsPointIn( &MenuRect, (Point *) &point ) )
	{
		SetFocus();
		ShowMenu(point);
		Invalidate(false); //TRUE
		return;
	}

	if( Util_IsKeyDown( VK_SPACE ) )
	{
		SetCapture( ) ;
		m_bCaptured = true ;
		m_bZooming = true;
		m_ptLastMouse = point ;
	}
	else
	if( m_Mode == VIEW_MODE_DRAG_BRUSH_HEIGHT )
	{

		ASSERT( pDoc->isPlaceBrushMode() );

		ShowCursor(TRUE);
		ReleaseCapture();
		m_bCaptured =  false;
		m_bDragging = false;
		pDoc->PlaceBrush( JE_TRUE );
		m_Mode = VIEW_MODE_NONE;
		return;
	}
	if( pDoc->isPlaceBrushMode() )
		m_Mode = VIEW_MODE_PLACEBRUSH;
	
	CView::OnRButtonDown(nFlags, point);
}

void CJweView::ShowMenu( CPoint point) 
{
	CMenu ContextMenu;
	CMenu *SubMenu;

	ClientToScreen( &point );
	ContextMenu.LoadMenu( IDR_VIEW );
	SubMenu = ContextMenu.GetSubMenu( 0 );
	SubMenu->TrackPopupMenu( TPM_LEFTALIGN, point.x, point.y, this, NULL );
	
}


//---------------------------------------------------
// Added DJT
//---------------------------------------------------
void CJweView::OnUpdateEditClone(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(Level_HasSelections(GetDocument()->GetLevel())) ;
}

void CJweView::OnEditClone() 
{
	jeProperty_List *pArray;
	CMainFrame *	pMainFrm;
	CJweDoc	*       pDoc = GetDocument();
	Level *         pLevel = pDoc->GetLevel();


	if( JE_FALSE == Select_DupAndDeselectSelections(pLevel))
		return ;

	pMainFrm = (CMainFrame*)AfxGetMainWnd();
	pMainFrm->AddSelection(pDoc);

	// Nudge the new cloned selection
	jeVec3d WorldDistance;
	Ortho_ViewToWorldDistance(m_pOrtho, 16, 16, &WorldDistance);
	pDoc->MoveSelected((SELECT_HANDLE)0, &WorldDistance);

	pArray = Select_BuildDescriptor(pLevel);
	pMainFrm->SetProperties(pArray);			
	pMainFrm->UpdatePanel( MAINFRM_PANEL_LISTS ) ;
}


void CJweView::OnMButtonUp(UINT nFlags, CPoint point) 
{
	CView::OnMButtonUp(nFlags, point);
}

void CJweView::OnMButtonDown(UINT nFlags, CPoint point) 
{
	CJweDoc* pDoc = GetDocument() ;
	eMouseMiddleButton  eState = MouseSettings_GetMiddleButtonState();

	switch (eState)
	{
		case mbSelectAll:
			pDoc->SelectAll(JE_TRUE);
			break;

		case mbSelectNone:
			pDoc->DeselectAll(JE_TRUE);
			break;

		case mbDisabled:
		default:
			break;
	}
	
	CView::OnRButtonDown(nFlags, point);
}
//---------------------------------------------------
// End DJT
//---------------------------------------------------
