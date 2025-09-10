#include "CIconTabCtrl.h"
#include "resource.h"

#if !defined(AFX_PROPERTIES_H__88F8410B_2FAE_4DFC_BA30_B7D979E26C30__INCLUDED_)
#define AFX_PROPERTIES_H__88F8410B_2FAE_4DFC_BA30_B7D979E26C30__INCLUDED_
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// Properties.h : Header-Datei
//

/////////////////////////////////////////////////////////////////////////////
// Dialogfeld CProperties 

struct Properties_UserData
{	CString	Name;
	CString Type;
	CString Value;
};


class CProperties : public CDialog
{
// Konstruktion
public:
	CProperties(CWnd* pParent = NULL);   // Standardkonstruktor
	~CProperties();

// Dialogfelddaten
	//{{AFX_DATA(CProperties)
	enum { IDD = IDD_PROPERTIES };
	CString	m_author;
	CString	m_comments;
	CString	m_company;
	CString	m_description;
	CString	m_name;
	CString	m_subject;
	CString	m_title;
	CString	m_type;
	CString	m_value;
	CString	m_version;
	//}}AFX_DATA


// Überschreibungen
	// Vom Klassen-Assistenten generierte virtuelle Funktionsüberschreibungen
	//{{AFX_VIRTUAL(CProperties)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV-Unterstützung
	//}}AFX_VIRTUAL

// Implementierung
protected:

	// Generierte Nachrichtenzuordnungsfunktionen
	//{{AFX_MSG(CProperties)
	afx_msg void OnPropertiesContents();
	afx_msg void OnPropertiesCustom();
	afx_msg void OnPropertiesSummary();
	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy();
	afx_msg void OnPropDel();
	afx_msg void OnPropAdd();
	afx_msg void OnChangePropValue();
	afx_msg void OnEditupdatePropName();
	afx_msg void OnSelchangePropType();
	afx_msg void OnClickPropValuelist(NMHDR* pNMHDR, LRESULT* pResult);
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnPropOk();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	CIconTabCtrl *	m_pPropertiesIconTab;
	void			EnableAddUpdateCheckButton();
	int				DelUserData();
	int				CreateUserData();

	Properties_UserData			*UserData;
	int				iUserDataEntries;
	jeBoolean		WriteVariable_String(CString Name, CString Value,jeVFile * pF, jePtrMgr * pPtrMgr);
public:

	jeBoolean Properties_ReadFromFile(jeVFile * pF, jePtrMgr * pPtrMgr);
	jeBoolean Properties_WriteToFile (jeVFile * pF, jePtrMgr * pPtrMgr);

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ fügt unmittelbar vor der vorhergehenden Zeile zusätzliche Deklarationen ein.

#endif // AFX_PROPERTIES_H__88F8410B_2FAE_4DFC_BA30_B7D979E26C30__INCLUDED_
