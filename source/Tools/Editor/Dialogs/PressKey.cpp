// PressKey.cpp: Implementierungsdatei
//

#include "stdafx.h"
#include "jwe.h"
#include "PressKey.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// Dialogfeld CPressKey 


CPressKey::CPressKey(CWnd* pParent /*=NULL*/)
	: CDialog(CPressKey::IDD, pParent)
{
	//{{AFX_DATA_INIT(CPressKey)
		// HINWEIS: Der Klassen-Assistent fügt hier Elementinitialisierung ein
	//}}AFX_DATA_INIT
}


void CPressKey::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPressKey)
		// HINWEIS: Der Klassen-Assistent fügt hier DDX- und DDV-Aufrufe ein
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPressKey, CDialog)
	//{{AFX_MSG_MAP(CPressKey)
	ON_WM_KEYDOWN()
	ON_WM_KEYUP()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Behandlungsroutinen für Nachrichten CPressKey 


BOOL CPressKey::OnInitDialog() 
{
   CDialog::OnInitDialog();
	
	ActKeyCode=-1;
	KeyAlt=0;
	KeyCrtl=0;
	KeyShift=0;
   return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE

}

void CPressKey::OnOK() 
{
	CDialog::OnOK();
}

void CPressKey::OnCancel() 
{


	CDialog::OnCancel();
}

void CPressKey::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	// TODO: Code für die Behandlungsroutine für Nachrichten hier einfügen und/oder Standard aufrufen
	char	cStringName[200];

	if (nFlags&KF_ALTDOWN) KeyAlt=1;
	if (nChar==VK_CONTROL) KeyCrtl=1;
	if (nChar==VK_SHIFT) KeyShift=1;

	GetKeyNameText ((nFlags<<16)+nChar,cStringName,199);
    GetDlgItem( IDC_EDIT_KEY_INFO)->SetWindowText(cStringName);

	CDialog::OnKeyDown(nChar, nRepCnt, nFlags);
	
}

void CPressKey::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	// TODO: Code für die Behandlungsroutine für Nachrichten hier einfügen und/oder Standard aufrufen
 
	if (ActKeyCode!=-1)
	{ ActKeyCode=0;
	}
	ActKeyCode = (nChar<<16)|(KeyAlt<<2)|(KeyCrtl<<1)|(KeyShift);
	CDialog::OnOK();
}

LONG	CPressKey::GetKeyCode()
{
	return ActKeyCode;
}



