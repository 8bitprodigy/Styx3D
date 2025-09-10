// Thin3DSplitterWnd.cpp : implementation file
//

#include "stdafx.h"
//#include "..\resource.h"
#include "Thin3DSplitterWnd.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CThin3DSplitterWnd

IMPLEMENT_DYNAMIC(CThin3DSplitterWnd, CSplitterWnd)

CThin3DSplitterWnd::CThin3DSplitterWnd()
{
	m_cxSplitter = m_cySplitter = 4;
	m_cxBorderShare = m_cyBorderShare = 0;
	m_cxSplitterGap = m_cySplitterGap = 4;
	m_cxBorder = m_cyBorder = 1;
}

CThin3DSplitterWnd::~CThin3DSplitterWnd()
{
}

BEGIN_MESSAGE_MAP(CThin3DSplitterWnd, CSplitterWnd)
	//{{AFX_MSG_MAP(CThin3DSplitterWnd)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CThin3DSplitterWnd message handlers

int CThin3DSplitterWnd::HitTest(CPoint /*pt*/) const
{
	ASSERT_VALID (this);
	return 0;       // don't let the user see the mouse hits
}