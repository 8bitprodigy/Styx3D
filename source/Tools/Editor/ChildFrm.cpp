/****************************************************************************************/
/*  CHILDFRM.CPP                                                                        */
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

#include "JetView.h"		// Rendered view class
#include "jwe.h"
#include "View.h"
#include "Settings.h"

#include "ChildFrm.h"
#include ".\childfrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CChildFrame

IMPLEMENT_DYNCREATE(CChildFrame, CMDIChildWnd)

BEGIN_MESSAGE_MAP(CChildFrame, CMDIChildWnd)
	//{{AFX_MSG_MAP(CChildFrame)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG_MAP
    ON_WM_SIZE()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CChildFrame construction/destruction

CChildFrame::CChildFrame()
{
	// TODO: add member initialization code here
	m_bInitialized = false;
}

CChildFrame::~CChildFrame()
{
}

BOOL CChildFrame::OnCreateClient( LPCREATESTRUCT /*lpcs*/, CCreateContext* pContext)
{
	int		x, y ;
	RECT	r ;

	int iNumColsX=2,iNumColsY=2;

	int iNumCols = Settings_GetView_Nums();

	if (iNumCols == 4)
		{ iNumColsX=2;iNumColsY=2;
		}
	else if (iNumCols == 6)
		{ iNumColsX=3;iNumColsY=2;
		}
	else if (iNumCols == 9)
		{ iNumColsX=3;iNumColsY=3;
		}
	else if (iNumCols == 12)
		{ iNumColsX=4;iNumColsY=3;
		}
	else if (iNumCols == 16)
		{ iNumColsX=4;iNumColsY=4;
		}


	ShowWindow( SW_MAXIMIZE );
	if( !m_wndSplitter.CreateStatic( this, iNumColsX, iNumColsY ) )
	{
		TRACE0("CCF::OCC\n") ;
		return FALSE;
	}
	GetClientRect(&r);
	x=r.right/iNumColsY;
	y=r.bottom/iNumColsX;

	int aViewType[4];
	aViewType[0] = IDM_VIEW_FRONT;
	aViewType[1] = IDM_VIEW_TOP;
	aViewType[2] = IDM_VIEW_WIREFRAME;
	aViewType[3] = IDM_VIEW_SIDE;

// Added JH 9.3.2000
	int iSplitWnd=0;
	int iPreviewWnd= Settings_GetView_PreviewView();

	if (iPreviewWnd==1)
		iPreviewWnd = iNumColsX*iNumColsY-iNumColsX;
	else if (iPreviewWnd==2)
		iPreviewWnd = iNumColsX-1;
	else if (iPreviewWnd==3)
		iPreviewWnd = iNumColsX*iNumColsY-1;

	if (aViewType[iPreviewWnd] != IDM_VIEW_WIREFRAME) {
		int tmp = aViewType[iPreviewWnd];
		aViewType[iPreviewWnd] = IDM_VIEW_WIREFRAME;
		aViewType[2] = tmp;
	}

	for (int SplitY=0;SplitY<iNumColsY;SplitY++)
		for (int SplitX=0;SplitX<iNumColsX;SplitX++)
		{	
			if (aViewType[iSplitWnd] == IDM_VIEW_WIREFRAME)
			{
				if( !m_wndSplitter.CreateView(SplitX, SplitY, RUNTIME_CLASS(CJetView), CSize(x, y), pContext))
				{
					TRACE0("CCF::OCC\n");
					return FALSE;
				}
				else 	
				{ 
					((CJetView *)m_wndSplitter.GetPane(SplitX, SplitY))->OnViewType( IDM_VIEW_WIREFRAME );
					SetActiveView( (CView*)m_wndSplitter.GetPane(SplitX, SplitY));
				}
			}		
			else
			{
				if( !m_wndSplitter.CreateView(SplitX, SplitY, RUNTIME_CLASS(CJweView), CSize(x, y), pContext) )
				{
					TRACE0("CCF::OCC\n");
					return FALSE;	
				}
				else
				{
/*
					if ((iSplitWnd&3)==0)
						((CJweView *)m_wndSplitter.GetPane(SplitX, SplitY))->OnViewType( IDM_VIEW_FRONT ) ;
					else
					if ((iSplitWnd&3)==1)
				   		((CJweView *)m_wndSplitter.GetPane(SplitX, SplitY))->OnViewType( IDM_VIEW_TOP ) ;
					else
						((CJweView *)m_wndSplitter.GetPane(SplitX, SplitY))->OnViewType( IDM_VIEW_SIDE ) ;
*/
					((CJweView *)m_wndSplitter.GetPane(SplitX, SplitY))->OnViewType( aViewType[iSplitWnd]);
				}
			}
			iSplitWnd++;		
		}

// EOF JH	
/*	if( !m_wndSplitter.CreateView(0, 0,
		RUNTIME_CLASS(CJweView), CSize(x, y), pContext) )
	{
		TRACE0("CCF::OCC\n");
		return FALSE;
	}

	if( !m_wndSplitter.CreateView(0, 1,
		RUNTIME_CLASS(CJetView), CSize(x, y), pContext))
	{
		TRACE0("CCF::OCC\n");
		return FALSE;
	}

	if( !m_wndSplitter.CreateView(1, 0,
		RUNTIME_CLASS(CJweView), CSize(x, y), pContext))
	{
		TRACE0("CCF::OCC\n");
		return FALSE;
	}

	if( !m_wndSplitter.CreateView(1, 1,
		RUNTIME_CLASS(CJweView), CSize(x, y), pContext))
	{
		TRACE0("CCF::OCC\n");
		return FALSE;
	}

	// set the view types

	// activate the top right view
	((CJweView *)m_wndSplitter.GetPane(0,0))->OnViewType( IDM_VIEW_FRONT ) ;
	((CJweView *)m_wndSplitter.GetPane(1,0))->OnViewType( IDM_VIEW_TOP ) ;
	((CJweView *)m_wndSplitter.GetPane(1,1))->OnViewType( IDM_VIEW_SIDE ) ;
*/
	m_bInitialized = true;
	return TRUE ;
}// OnCreateClient

BOOL CChildFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	if( !CMDIChildWnd::PreCreateWindow(cs) )
		return FALSE;

	cs.style = WS_CHILD | WS_VISIBLE;

	return TRUE;
}



/////////////////////////////////////////////////////////////////////////////
// CChildFrame diagnostics

#ifdef _DEBUG
void CChildFrame::AssertValid() const
{
	CMDIChildWnd::AssertValid();
}

void CChildFrame::Dump(CDumpContext& dc) const
{
	CMDIChildWnd::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CChildFrame message handlers

void CChildFrame::ActivateFrame(int nCmdShow)
{
	nCmdShow = SW_SHOWMAXIMIZED;
    CMDIChildWnd::ActivateFrame(nCmdShow);
}

void CChildFrame::OnSize(UINT nType, int cx, int cy)
{
    CMDIChildWnd::OnSize(nType, cx, cy);

	RECT rcWnd;
	GetClientRect(&rcWnd);

	if (m_bInitialized) {
        long idx;
        long nbCol = m_wndSplitter.GetColumnCount();
        long nbRow = m_wndSplitter.GetRowCount();
        long opti = cy/nbRow;
        long mini = cy/nbRow - cy/(10*nbRow);
        for (idx=0; idx<nbRow;idx++) {
    		m_wndSplitter.SetRowInfo(idx, opti, mini);
        }
        opti = cx/nbCol;
        mini = cx/nbCol - cx/(10*nbCol);
        for (idx=0; idx<nbCol;idx++) {
		    m_wndSplitter.SetColumnInfo(idx, opti, mini);
        }
		m_wndSplitter.RecalcLayout();
	}
}
