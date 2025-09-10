// ToolBrowser.cpp : implementation file
//

#include "stdafx.h"
#include "ActorWorkbench.h"
#include "ToolBrowser.h"


// CToolBrowser dialog

IMPLEMENT_DYNAMIC(CToolBrowser, CDialog)
CToolBrowser::CToolBrowser(CWnd* pParent /*=NULL*/)
	: CDialog(CToolBrowser::IDD, pParent)
{
}

CToolBrowser::~CToolBrowser()
{
}

void CToolBrowser::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TREE_BROWSER, m_TreeCtrl);
}


BEGIN_MESSAGE_MAP(CToolBrowser, CDialog)
END_MESSAGE_MAP()


// CToolBrowser message handlers
