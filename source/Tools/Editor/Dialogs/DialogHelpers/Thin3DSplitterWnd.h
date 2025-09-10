#if !defined(AFX_THIN3DSPLITTERWND_H__246736FD_6238_489D_BDC8_1959C7030AA1__INCLUDED_)
#define AFX_THIN3DSPLITTERWND_H__246736FD_6238_489D_BDC8_1959C7030AA1__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// Thin3DSplitterWnd.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CThin3DSplitterWnd frame with splitter

class CThin3DSplitterWnd : public CSplitterWnd
{
	DECLARE_DYNAMIC(CThin3DSplitterWnd)
public:
	CThin3DSplitterWnd();           // protected constructor used by dynamic creation

// Attributes
protected:

// Operations
public:
	long GetSplitterCx() { return m_cxSplitter; }

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CThin3DSplitterWnd)
	virtual int CThin3DSplitterWnd::HitTest(CPoint pt) const;
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CThin3DSplitterWnd();

	// Generated message map functions
	//{{AFX_MSG(CThin3DSplitterWnd)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_THIN3DSPLITTERWND_H__246736FD_6238_489D_BDC8_1959C7030AA1__INCLUDED_)
