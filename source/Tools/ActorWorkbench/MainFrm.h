// MainFrm.h : interface of the CMainFrame class
//


#pragma once

#include "jet.h"
#include "ToolBrowser.h"

class CMainFrame : public CFrameWnd
{
	
protected: // create from serialization only
	CMainFrame();
	DECLARE_DYNCREATE(CMainFrame)

// Attributes
public:

// Operations
public:

// Overrides
public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

// Implementation
public:
	virtual ~CMainFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:  // control bar embedded members
	CStatusBar  m_wndStatusBar;
	CToolBar    m_wndToolBar;

	CSplitterWnd	m_wndSplitter;

	CToolBrowser	m_wndToolBrowser;
	CDialogBar		m_wndBarBrowser;

	HTREEITEM		m_RootItem;
// Generated message map functions
protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	DECLARE_MESSAGE_MAP()
	virtual BOOL OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext);
	virtual void OnSize(UINT nType, int cx, int cy);
public:
	jeBoolean LoadActor(const char * filename, const char *actorname);
	afx_msg void OnFileOpen();
};


