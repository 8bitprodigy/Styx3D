#if !defined(AFX_PREFERENCES_H__B39A5E33_C026_4A0A_9D58_438BE0EA1A72__INCLUDED_)
#define AFX_PREFERENCES_H__B39A5E33_C026_4A0A_9D58_438BE0EA1A72__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// Preferences.h : Header-Datei
//
#include "CIconTabCtrl.h"
/////////////////////////////////////////////////////////////////////////////
// Dialogfeld CPreferences 

class CPreferences : public CDialog
{
// Konstruktion
public:
	CPreferences(CWnd* pParent = NULL);   // Standardkonstruktor

// Dialogfelddaten
	//{{AFX_DATA(CPreferences)
	enum { IDD = IDD_PREFERENCES };
		// HINWEIS: Der Klassen-Assistent fügt hier Datenelemente ein
	//}}AFX_DATA


// Überschreibungen
	// Vom Klassen-Assistenten generierte virtuelle Funktionsüberschreibungen
	//{{AFX_VIRTUAL(CPreferences)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV-Unterstützung
	//}}AFX_VIRTUAL

// Implementierung
protected:
	void DrawGridColor(int DlgItem, int32 Color);
	void MoveOptionObjects();
	void HideAll();
	char *TranslateKeyToText(UINT KeyCode,char *sPrefsString, int Len);
	int  InitKeyListCtrl();
	void Key_SetName();
	LONG   ActKeyCode;

	int  SaveSettings();
	int  LoadSettings();
		
	COLORREF	coSelected ;
	COLORREF	coSubSelected ;
	COLORREF	coSelectedBk ;
	COLORREF	coTemplate ;
	COLORREF	coGridBackgroud ;
	COLORREF	coGrid ;
	COLORREF	coGridSnap ;
	COLORREF	coSubtractBrush ;
	COLORREF	coSubtractNoAssoc ;
	COLORREF	coAddBrush ;
	COLORREF	coSelectedFace ;
	COLORREF	coCutBrush ;
	COLORREF	coConstructorLine ;

	CIconTabCtrl *m_pPreferencesIconTab;


	// Generierte Nachrichtenzuordnungsfunktionen
	//{{AFX_MSG(CPreferences)
		// HINWEIS: Der Klassen-Assistent fügt hier Member-Funktionen ein
		virtual void OnOK();
		virtual void OnCancel();
		virtual BOOL OnInitDialog();
		void	OnSetColor1();
		void	OnSetColor2();
		void	OnSetColor3();
		void	OnSetColor4();
		void	OnSetColor5();
		void	OnSetColor6();
		void	OnSetColor7();
		void	OnSetColor8();
		void	OnSetColor9();
		void	OnSetColor10();
		void	OnSetColor11();
		void	OnSetColor12();
		void	OnSetColor13();
		void	OnGridView();
		void	OnGridSnap();
		void	OnKeyboard();
		void	OnPath();
		void	OnMouse();
		void	OnJet();
		void	OnEngine_GetFullscreenMode();
		void	OnEngine_GetWindowMode();
		void    OnJet_Stairs();
		void    OnKey_PressKey();
		void	OnKey_Import();
		void	OnKey_Export();

		afx_msg void OnPaint();

		void    OnKey_CheckControl();
		void    OnKey_CheckShift();
		void    OnKey_CheckAlt();
		void    OnKey_Assign();
		void	OnPrefsOK();
		void	OnGlobal();
		void	OnInfo();

		afx_msg void OnChangeKeyEdit();
		afx_msg void OnClickKeys(NMHDR* pNMHDR, LRESULT* pResult);
		afx_msg void OnKeydownKeys(NMHDR* pNMHDR, LRESULT* pResult);

	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ fügt unmittelbar vor der vorhergehenden Zeile zusätzliche Deklarationen ein.

#endif // AFX_PREFERENCES_H__B39A5E33_C026_4A0A_9D58_438BE0EA1A72__INCLUDED_
