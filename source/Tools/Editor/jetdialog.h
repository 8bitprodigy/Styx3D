#ifndef JETDIALOG_H
#define JETDIALOG_H

#include "afxwin.h"

class CMainFrame;

class CJetTabDialog : public CDialog
{
public:
    CJetTabDialog() : CDialog() { m_mainFrame = NULL; }
    BOOL Create( UINT nIDTemplate, CWnd* pParentWnd, CMainFrame * mainFrame );

    virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
protected:
    //{{AFX_MSG(CMyBar)
        afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG
    DECLARE_MESSAGE_MAP()

    CMainFrame * m_mainFrame;
};

#endif