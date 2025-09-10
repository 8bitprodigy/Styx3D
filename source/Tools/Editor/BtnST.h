/****************************************************************************************/
/*  BtnST.CPP                                                                         */
/*                                                                                      */
/*  Author:	davide calabro		                                                                          */
/*  Description:   Flat Button Class                                                                  */
/*                                                                                      */
/****************************************************************************************/

#ifndef _BTNST_H
#define _BTNST_H

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

// CBtnST.h : header file
//


//TOM
//#include "MemDC.h"
#include <afxglobals.h>	//	contains CMemDC class

// Comment this if you don't want that CButtonST hilights itself
// also when the window is inactive (like happens in Internet Explorer)
#define ST_LIKEIE

// Comment this if you don't want to use CMemDC class
#define ST_USE_MEMDC

/////////////////////////////////////////////////////////////////////////////
// CButtonST window

class CButtonST : public CButton
{
// Construction
public:
    CButtonST();
	~CButtonST();
    enum {ST_ALIGN_HORIZ, ST_ALIGN_VERT, ST_ALIGN_HORIZ_RIGHT};

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(CButtonST)
	public:
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void PreSubclassWindow();
	virtual LRESULT DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
public:
	void DrawTransparent(BOOL bRepaint = FALSE);

	BOOL GetDefault();

	void SetTooltipText(int nId, BOOL bActivate = TRUE);
	void SetTooltipText(CString* spText, BOOL bActivate = TRUE);
	void ActivateTooltip(BOOL bEnable = TRUE);

	BOOL SetBtnCursor(int nCursorId = -1);

	void SetFlatFocus(BOOL bDrawFlatFocus, BOOL bRepaint = FALSE);
	BOOL GetFlatFocus();

	void SetDefaultActiveFgColor(BOOL bRepaint = FALSE);
	void SetActiveFgColor(COLORREF crNew, BOOL bRepaint = FALSE);
	const COLORREF GetActiveFgColor();
	
	void SetDefaultActiveBgColor(BOOL bRepaint = FALSE);
	void SetActiveBgColor(COLORREF crNew, BOOL bRepaint = FALSE);
	const COLORREF GetActiveBgColor();
	
	void SetDefaultInactiveFgColor(BOOL bRepaint = FALSE);
	void SetInactiveFgColor(COLORREF crNew, BOOL bRepaint = FALSE);
	const COLORREF GetInactiveFgColor();

	void SetDefaultInactiveBgColor(BOOL bRepaint = FALSE);
	void SetInactiveBgColor(COLORREF crNew, BOOL bRepaint = FALSE);
	const COLORREF GetInactiveBgColor();

	void SetShowText(BOOL bShow = TRUE);
	BOOL GetShowText();

	void SetAlign(int nAlign);
	int GetAlign();

	void SetFlat(BOOL bState = TRUE);
	BOOL GetFlat();

	void DrawBorder(BOOL bEnable = TRUE);
	void SetIcon(int nIconInId, int nIconOutId = NULL);
	void SetIcon(HICON hIconIn, HICON hIconOut = NULL);

	static const short GetVersionI();
	static const char* GetVersionC();

protected:
    //{{AFX_MSG(CButtonST)
	afx_msg void OnCaptureChanged(CWnd *pWnd);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnSysColorChange();
	//}}AFX_MSG

	afx_msg HBRUSH CtlColor(CDC* pDC, UINT nCtlColor);

	DECLARE_MESSAGE_MAP()
private:
	void DrawTheIcon(CDC* pDC, CString* title, RECT* rcItem, CRect* captionRect, BOOL IsPressed, BOOL IsDisabled);
	void InitToolTip();
	void PaintBk(CDC* pDC);

	int m_nAlign;
	BOOL m_bShowText;
	BOOL m_bDrawBorder;
	BOOL m_bIsFlat;
	BOOL m_MouseOnButton;
	BOOL m_bDrawFlatFocus;

	HCURSOR m_hCursor;
	CToolTipCtrl m_ToolTip;

	HICON m_hIconIn;
	HICON m_hIconOut;
	BYTE m_cyIcon;
	BYTE m_cxIcon;

	CDC m_dcBk;
	CBitmap m_bmpBk;
	CBitmap* m_pbmpOldBk;
	BOOL m_bDrawTransparent;

	BOOL m_bIsDefault;

	COLORREF  m_crInactiveBg;
    COLORREF  m_crInactiveFg;
    COLORREF  m_crActiveBg;
    COLORREF  m_crActiveFg;


//	TOM
								//	a buffer for the part of drawing which does not change
//	tom morris feb 2005 -- commented out	
//	CBufferMemDC m_ViewBuffer;
							//	a buffer which is used for flicker free redrawing
							//	on top of the background





};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif
