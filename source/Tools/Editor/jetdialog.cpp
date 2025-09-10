#include "stdafx.h"
#include <CommCtrl.h>

#include "jwe.h"
#include "Util.h"

#include "mainFrm.h"
#include "jetdialog.h"

BEGIN_MESSAGE_MAP(CJetTabDialog, CDialog)
    //{{AFX_MSG_MAP(CJetBar)
    ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


BOOL CJetTabDialog::Create(UINT nIDTemplate, CWnd * parent, CMainFrame * mainFrame)
{
    m_mainFrame = mainFrame;
    return CDialog::Create(nIDTemplate,parent);
}

BOOL CJetTabDialog::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT *pResult)
{
    if(m_mainFrame) {
        return m_mainFrame->OnNotify(wParam,lParam,pResult);
    }
    return CDialog::OnNotify(wParam,lParam,pResult);
}

void CJetTabDialog::OnSize(UINT nType, int cx, int cy) 
{
	CDialog::OnSize(nType, cx, cy);

	//	 by trilobite jan. 2011
	if (GetDlgItem( CPNL_TC_TABS ))
	{
		CTabCtrl * ctrl = (CTabCtrl *)(GetDlgItem( CPNL_TC_TABS ));

		if(ctrl) {
			CRect rc;
			GetClientRect(rc);
			ctrl->MoveWindow(rc);
			if(m_mainFrame) {
				ctrl->AdjustRect(FALSE, rc);

				if(m_mainFrame->m_TextureDialog.m_hWnd) {
					m_mainFrame->m_TextureDialog.MoveWindow(rc);
				}
				if(m_mainFrame->m_GroupDialog.m_hWnd) {
					m_mainFrame->m_GroupDialog.MoveWindow(rc);
				}
				if(m_mainFrame->m_ModelsDialog.m_hWnd) {
					m_mainFrame->m_ModelsDialog.MoveWindow(rc);
				}
				if(m_mainFrame->m_ListsDialog.m_hWnd) {
					m_mainFrame->m_ListsDialog.MoveWindow(rc);
				}
			}
		}
		//	by trilobite jan. 2011
	}
}
