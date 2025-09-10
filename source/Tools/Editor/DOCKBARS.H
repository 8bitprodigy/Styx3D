#include "sizecbar.h"

class CJetBar : public CSizingControlBar
{
public:
                                    CJetBar() 
                                    { m_wndChild = NULL; };
                                   ~CJetBar() 
                                    {};

protected:
    //{{AFX_MSG(CMyBar)
        afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG
    DECLARE_MESSAGE_MAP()
public:
        CDialog *                   m_wndChild;
};

