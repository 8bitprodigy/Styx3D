#pragma once
#include "afxcmn.h"


// CToolBrowser dialog

class CToolBrowser : public CDialog
{
	DECLARE_DYNAMIC(CToolBrowser)

public:
	CToolBrowser(CWnd* pParent = NULL);   // standard constructor
	virtual ~CToolBrowser();

// Dialog Data
	enum { IDD = IDD_ACTOR_BROWSER };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CTreeCtrl m_TreeCtrl;
	CImageList m_ImageList;
};
