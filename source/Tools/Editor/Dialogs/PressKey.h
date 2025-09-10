#if !defined(AFX_PRESSKEY_H__DE2DE90C_1C18_4CC3_A94B_24F4A873DA31__INCLUDED_)
#define AFX_PRESSKEY_H__DE2DE90C_1C18_4CC3_A94B_24F4A873DA31__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PressKey.h : Header-Datei
//

/////////////////////////////////////////////////////////////////////////////
// Dialogfeld CPressKey 

class CPressKey : public CDialog
{
// Konstruktion
public:
	CPressKey(CWnd* pParent = NULL);   // Standardkonstruktor

	LONG	GetKeyCode();

// Dialogfelddaten
	//{{AFX_DATA(CPressKey)
	enum { IDD = IDD_PRESS_A_KEY };
		// HINWEIS: Der Klassen-Assistent fügt hier Datenelemente ein
	//}}AFX_DATA


// Überschreibungen
	// Vom Klassen-Assistenten generierte virtuelle Funktionsüberschreibungen
	//{{AFX_VIRTUAL(CPressKey)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV-Unterstützung
	//}}AFX_VIRTUAL


// Implementierung
protected:
	UINT	ActKeyCode;	
	int KeyAlt;
	int KeyCrtl;
	int KeyShift;

	// Generierte Nachrichtenzuordnungsfunktionen
	//{{AFX_MSG(CPressKey)
		virtual void OnOK();
		virtual void OnCancel();
		virtual BOOL OnInitDialog();
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ fügt unmittelbar vor der vorhergehenden Zeile zusätzliche Deklarationen ein.

#endif // AFX_PRESSKEY_H__DE2DE90C_1C18_4CC3_A94B_24F4A873DA31__INCLUDED_
