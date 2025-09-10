/****************************************************************************************/
/*  Properties.cpp                                                                            */
/*                                                                                      */
/*  Author:			Joachim Hellmann                                                                           */
/*  Description:                                                                       */
/*                                                                                      */

#include "stdafx.h"
#include <assert.h>

#include "Jet.h"
#include "ErrorLog.h"

#include "jwe.h"
#include "MainFrm.h"
#include "Properties.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// Dialogfeld CProperties 


CProperties::CProperties(CWnd* pParent /*=NULL*/)
	: CDialog(CProperties::IDD, pParent)
{
	//{{AFX_DATA_INIT(CProperties)
	m_author = _T("");
	m_comments = _T("");
	m_company = _T("");
	m_description = _T("");
	m_name = _T("");
	m_subject = _T("");
	m_title = _T("");
	m_type = _T("");
	m_value = _T("");
	m_version = _T("");
	//}}AFX_DATA_INIT
	UserData=nullptr;
	iUserDataEntries=0;
	m_pPropertiesIconTab = nullptr;
}


void CProperties::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CProperties)
	DDX_Text(pDX, IDC_PROP_AUTHOR, m_author);
	DDX_Text(pDX, IDC_PROP_COMMENTS, m_comments);
	DDX_Text(pDX, IDC_PROP_COMPANY, m_company);
	DDX_Text(pDX, IDC_PROP_DESCRIPTION, m_description);
	DDX_CBString(pDX, IDC_PROP_NAME, m_name);
	DDX_Text(pDX, IDC_PROP_SUBJECT, m_subject);
	DDX_Text(pDX, IDC_PROP_TITLE, m_title);
	DDX_CBString(pDX, IDC_PROP_TYPE, m_type);
	DDX_Text(pDX, IDC_PROP_VALUE, m_value);
	DDX_Text(pDX, IDC_PROP_VERSION, m_version);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CProperties, CDialog)
	//{{AFX_MSG_MAP(CProperties)
	ON_BN_CLICKED(IDC_PROPERTIES_CONTENTS, OnPropertiesContents)
	ON_BN_CLICKED(IDC_PROPERTIES_CUSTOM, OnPropertiesCustom)
	ON_BN_CLICKED(IDC_PROPERTIES_SUMMARY, OnPropertiesSummary)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_PROP_DEL, OnPropDel)
	ON_BN_CLICKED(IDC_PROP_ADD, OnPropAdd)
	ON_EN_CHANGE(IDC_PROP_VALUE, OnChangePropValue)
	ON_CBN_EDITUPDATE(IDC_PROP_NAME, OnEditupdatePropName)
	ON_CBN_SELCHANGE(IDC_PROP_TYPE, OnSelchangePropType)
	ON_NOTIFY(NM_CLICK, IDC_PROP_VALUELIST, OnClickPropValuelist)
	ON_BN_CLICKED(IDC_PROP_OK, OnPropOk)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Behandlungsroutinen für Nachrichten CProperties 

CProperties::~CProperties()
{ 
	DelUserData();
}



//---------------------------------------------------------------------
//  OnPropertiesCustom                                                                           
//	                                                                                      
//	Show Custom Properties Dialog
//
void CProperties::OnPropertiesCustom () 
{
	m_pPropertiesIconTab->Group_Display ( 0 );

	GetDlgItem( IDC_PROP_TRUE  )->ShowWindow( false );
	GetDlgItem( IDC_PROP_FALSE )->ShowWindow( false );
	
}


//---------------------------------------------------------------------
//  OnPropertiesContents                                                                           
//	                                                                                      
//	Show Contents Properties Dialog
//
void CProperties::OnPropertiesContents() 
{
	m_pPropertiesIconTab->Group_Display ( 1 );
}


//---------------------------------------------------------------------
//  OnPropertiesSummary                                                                           
//	                                                                                      
//	Show Summery Properties Dialog
//
void CProperties::OnPropertiesSummary() 
{
	m_pPropertiesIconTab->Group_Display(2);
}


//---------------------------------------------------------------------
//  OnInitDialog                                                                           
//	                                                                                      
//	Init Dialog: Setup IconTabCtrl, Insert Values for Userdefined Properties
//
BOOL CProperties::OnInitDialog()
{
	CDialog::OnInitDialog();


	m_pPropertiesIconTab = new CIconTabCtrl(this, 20, 50);

	if (!m_pPropertiesIconTab)
		return FALSE;

	m_pPropertiesIconTab->Group_AddObjects(0, "User Parameter...",
		IDC_PROPERTIES_CUSTOM, IDI_PROPERTIES_CUSTOM, IDI_PROPERTIES_CUSTOM,
		IDC_PROP_STATIC8, IDC_PROP_STATIC9, IDC_PROP_STATIC10,
		IDC_PROP_NAME, IDC_PROP_TYPE, IDC_PROP_VALUE,
		IDC_PROP_VALUELIST, IDC_PROP_ADD, IDC_PROP_DEL, IDC_PROP_TRUE, IDC_PROP_FALSE,
		-1);

	m_pPropertiesIconTab->Group_AddObjects(1, "Description...",
		IDC_PROPERTIES_CONTENTS, IDI_PROPERTIES_CONTENTS, IDI_PROPERTIES_CONTENTS,
		IDC_PROP_STATIC7, IDC_PROP_DESCRIPTION,
		-1);

	m_pPropertiesIconTab->Group_AddObjects(2, "Summary...",
		IDC_PROPERTIES_SUMMARY, IDI_PROPERTIES_SUMMARY, IDI_PROPERTIES_SUMMARY,
		IDC_PROP_STATIC1, IDC_PROP_STATIC2, IDC_PROP_STATIC3,
		IDC_PROP_STATIC4, IDC_PROP_STATIC5, IDC_PROP_STATIC6,
		IDC_PROP_TITLE, IDC_PROP_VERSION, IDC_PROP_SUBJECT,
		IDC_PROP_AUTHOR, IDC_PROP_COMPANY, IDC_PROP_COMMENTS,
		-1);

	m_pPropertiesIconTab->RecalcLayout(IDC_PROP_STATIC1);
	m_pPropertiesIconTab->SetTitleId(IDC_STATIC_PROP);

	m_pPropertiesIconTab->SetStatic(IDC_STATIC_RECT);
	
	CListCtrl* pListCtrl{};
	pListCtrl = (CListCtrl*)GetDlgItem(IDC_PROP_VALUELIST);
	if (pListCtrl)
	{
		DWORD dwStyle{};
		dwStyle = pListCtrl->GetExtendedStyle();
		if (dwStyle)
		{
			pListCtrl->SetExtendedStyle(dwStyle | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
		}
		pListCtrl->DeleteAllItems();
		pListCtrl->InsertColumn(0, "Name", LVCFMT_LEFT, 104, 2);
		pListCtrl->InsertColumn(1, "Value", LVCFMT_LEFT, 100, 2);
		pListCtrl->InsertColumn(2, "Type", LVCFMT_LEFT, 70, 2);

		if (iUserDataEntries != 0)
		{
			for (int X = 0; X < iUserDataEntries; X++)
			{
				pListCtrl->InsertItem(X, "");
				pListCtrl->SetItemText(X, 0, UserData[X].Name);
				pListCtrl->SetItemText(X, 1, UserData[X].Value);
				pListCtrl->SetItemText(X, 2, UserData[X].Type);
			}
		}
	}
	RECT	ClientRect{};
	RECT	WindowRect{};

	GetDlgItem(IDC_STATIC_RECT)->GetWindowRect(&ClientRect);
	GetWindowRect(&WindowRect);
	WindowRect.bottom = ClientRect.bottom + 3;
	MoveWindow(&WindowRect, false);

	CMainFrame* pMainFrm = nullptr;
	pMainFrm = (CMainFrame*)AfxGetMainWnd();
	if (pMainFrm)
	{
		GetDlgItem(IDC_STATIC_PROP)->SetFont(&pMainFrm->cBigFont, true);
	}

	OnPropertiesSummary();

	EnableAddUpdateCheckButton();

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX-Eigenschaftenseiten sollten FALSE zurückgeben
}


//---------------------------------------------------------------------
//  OnDestroy                                                                           
//	                                                                                      
//	Destroy Dialog, free Memory
//
void CProperties::OnDestroy() 
{
	CDialog::OnDestroy();
	
	// TODO: Code für die Behandlungsroutine für Nachrichten hier einfügen
	if (m_pPropertiesIconTab)
		delete m_pPropertiesIconTab;
}


//---------------------------------------------------------------------
//  OnPropDel                                                                           
//	                                                                                      
//	Delete UserDefined Item selected in CListCtrl
//
void CProperties::OnPropDel() 
{
	CListCtrl *pListCtrl = (CListCtrl *) GetDlgItem(IDC_PROP_VALUELIST);
	POSITION pos = pListCtrl->GetFirstSelectedItemPosition();
	if (pos != nullptr)
		{
			pListCtrl->DeleteItem (pListCtrl->GetNextSelectedItem(pos));
			EnableAddUpdateCheckButton	();
		}	
}


//---------------------------------------------------------------------
//  OnPropAdd                                                                           
//	                                                                                      
//	Add an new UserDefined Element
//
void CProperties::OnPropAdd() 
{
	CListCtrl *	pListCtrl = (CListCtrl *) GetDlgItem(IDC_PROP_VALUELIST);
	CString		cText;
	char		TempString[20];
	int			X;

	// UpdateItem
	// Test if element already exists
	GetDlgItem( IDC_PROP_NAME  )->GetWindowText(cText);
	LVFINDINFO	FindKey;
	FindKey.flags = LVFI_STRING;
	FindKey.psz   = cText;
	int Ret = pListCtrl->FindItem( &FindKey, -1 );
	
	// AddItem
	if (Ret !=-1) X=Ret;
		else 
		{ X=pListCtrl->GetItemCount();
		  pListCtrl->InsertItem(X,"" );
		}

	GetDlgItem( IDC_PROP_NAME)->GetWindowText(cText);
	pListCtrl->SetItemText( X, 0,cText);

	GetDlgItem( IDC_PROP_TYPE  )->GetWindowText(cText);
	if (cText.Compare("Boolean")==0)
		{
			if (((CButton*)GetDlgItem( IDC_PROP_TRUE))->GetCheck())
					  pListCtrl->SetItemText( X, 1, "true");	
				 else pListCtrl->SetItemText( X, 1, "false");	
		}
	else if (cText.Compare("Number")==0)
		{
		  GetDlgItem( IDC_PROP_VALUE  )->GetWindowText(cText);
		  pListCtrl->SetItemText( X, 1, _itoa(atoi(cText),TempString,10));
		}
	else if (cText.Compare("String")==0)
		{
		  GetDlgItem( IDC_PROP_VALUE  )->GetWindowText(cText);
		  pListCtrl->SetItemText( X, 1, cText);
		}


	GetDlgItem( IDC_PROP_TYPE  )->GetWindowText(cText);
	pListCtrl->SetItemText( X, 2, cText);
	EnableAddUpdateCheckButton	();
}

//---------------------------------------------------------------------
//  OnChangePropValue                                                                           
//	                                                                                      
// Value changed
//

void CProperties::OnChangePropValue() 
{
		EnableAddUpdateCheckButton	();
}


//---------------------------------------------------------------------
//  OnEditupdatePropName                                                                           
//	                                                                                      
//	Name changed
//
void CProperties::OnEditupdatePropName() 
{
	EnableAddUpdateCheckButton	();
}

//---------------------------------------------------------------------
//  OnSelchangePropType                                                                           
//	                                                                                      
//	Type changed
//
void CProperties::OnSelchangePropType() 
{
	CString		cText;	
	

	GetDlgItem( IDC_PROP_TYPE  )->GetWindowText(cText);
	if (cText.Compare("Boolean")==0)
		{
			GetDlgItem( IDC_PROP_TRUE  )->ShowWindow(true);
			GetDlgItem( IDC_PROP_FALSE )->ShowWindow(true);
			GetDlgItem( IDC_PROP_VALUE )->ShowWindow(false);			
		}
	else 
		{
			GetDlgItem( IDC_PROP_TRUE  )->ShowWindow(false);
			GetDlgItem( IDC_PROP_FALSE )->ShowWindow(false);
			GetDlgItem( IDC_PROP_VALUE )->ShowWindow(true);			
		}
	
	EnableAddUpdateCheckButton	();
}


//---------------------------------------------------------------------
//  EnableAddUpdateCheckButton                                                                           
//	                                                                                      
//	Test actual selection, activate or deactivate buttons,...
//
void CProperties::EnableAddUpdateCheckButton()
{
	CString		cText;

	GetDlgItem(IDC_PROP_NAME)->GetWindowText(cText);
	if (cText.GetLength() == 0)
	{
		GetDlgItem(IDC_PROP_ADD)->EnableWindow(false);
		return;
	}


	GetDlgItem(IDC_PROP_TYPE)->GetWindowText(cText);
	if (cText.Compare("Boolean") != 0)
	{
		GetDlgItem(IDC_PROP_VALUE)->GetWindowText(cText);
		if (cText.GetLength() == 0)
		{
			GetDlgItem(IDC_PROP_ADD)->EnableWindow(false);
			return;
		}
	}

	GetDlgItem(IDC_PROP_TYPE)->GetWindowText(cText);
	if (cText.GetLength() == 0)
	{
		GetDlgItem(IDC_PROP_ADD)->EnableWindow(false);
		return;
	}

	GetDlgItem(IDC_PROP_ADD)->EnableWindow(true);
	GetDlgItem(IDC_PROP_NAME)->GetWindowText(cText);

	// Test if element already exists
	CListCtrl* pListCtrl{};
	pListCtrl = (CListCtrl*)GetDlgItem(IDC_PROP_VALUELIST);
	if (pListCtrl)
	{
		LVFINDINFO	FindKey;
		FindKey.flags = LVFI_STRING;
		FindKey.psz = cText;
		int Ret = pListCtrl->FindItem(&FindKey, -1);

		if (Ret == -1) GetDlgItem(IDC_PROP_ADD)->SetWindowText("Add");
		else   GetDlgItem(IDC_PROP_ADD)->SetWindowText("Update");
	}
}

//---------------------------------------------------------------------
//  OnClickPropValuelist                                                                           
//	                                                                                      
//	Element in CListCtrl changed
//
void CProperties::OnClickPropValuelist(NMHDR* pNMHDR, LRESULT* pResult)
{
	// TODO: Code für die Behandlungsroutine der Steuerelement-Benachrichtigung hier einfügen
	CString		cText;

	CListCtrl* pListCtrl{};
	pListCtrl = (CListCtrl*)GetDlgItem(IDC_PROP_VALUELIST);
	if (pListCtrl)
	{
		POSITION pos = pListCtrl->GetFirstSelectedItemPosition();
		if (pos != NULL)
		{
			int nItem = pListCtrl->GetNextSelectedItem(pos);

			GetDlgItem(IDC_PROP_NAME)->SetWindowText(pListCtrl->GetItemText(nItem, 0));
			((CComboBox*)GetDlgItem(IDC_PROP_TYPE))->SelectString(-1, pListCtrl->GetItemText(nItem, 2));
			GetDlgItem(IDC_PROP_TYPE)->GetWindowText(cText);
			if (cText.Compare("Boolean") == 0)
			{
				if (pListCtrl->GetItemText(nItem, 1).Compare("true") == 0)
				{
					((CButton*)GetDlgItem(IDC_PROP_FALSE))->SetCheck(0x00);
					((CButton*)GetDlgItem(IDC_PROP_TRUE))->SetCheck(0x01);
				}
				else {
					((CButton*)GetDlgItem(IDC_PROP_FALSE))->SetCheck(0x01);
					((CButton*)GetDlgItem(IDC_PROP_TRUE))->SetCheck(0x00);
				}
			}
			else
			{
				GetDlgItem(IDC_PROP_VALUE)->SetWindowText(pListCtrl->GetItemText(nItem, 1));
			}
			EnableAddUpdateCheckButton();
			OnSelchangePropType();
		}
	}
	*pResult = 0;
}


//---------------------------------------------------------------------
//  DelUserData                                                                           
//	                                                                                      
//	Delete Memory allocated for UserData
//
int CProperties::DelUserData()
{
	if (UserData!=NULL)
		{
		  delete [] UserData;
		  UserData=NULL;
	  	  iUserDataEntries=0;
		  return true;
		}

	return false;
}


//---------------------------------------------------------------------
//  CreateUserData                                                                           
//	                                                                                      
int CProperties::CreateUserData()
{
	CListCtrl* pListCtrl{};
	pListCtrl = (CListCtrl*)GetDlgItem(IDC_PROP_VALUELIST);
	if (pListCtrl)
	{
	int iNum = pListCtrl->GetItemCount();
	
	if (UserData != NULL) DelUserData();

	UserData = new Properties_UserData[iNum];
	iUserDataEntries = iNum;
	
	for (int X=0; X<iNum; X++)
		{	
			UserData[X].Name  = pListCtrl->GetItemText( X, 0);	
			UserData[X].Value = pListCtrl->GetItemText( X, 1);	
			UserData[X].Type  = pListCtrl->GetItemText( X, 2);	
		}
	}
	return false;
}

void CProperties::OnOK() 
{
}

void CProperties::OnCancel() 
{
	
	CDialog::OnCancel();
}

void CProperties::OnPropOk() 
{
	CreateUserData();
	CDialog::OnOK();
}

//---------------------------------------------------------------------
//  Properties_WriteToFile                                                                           
//	                                                                                      
//	Write Properties to File
//
jeBoolean CProperties::Properties_WriteToFile( /*Level * pLevel,*/ jeVFile * pF, jePtrMgr * pPtrMgr)
{
	assert( jeVFile_IsValid( pF ) ) ;
	
	char	cUserDataEntries[10];

	int ret=0;

	ret =WriteVariable_String ("AUTH",m_author,pF,pPtrMgr);
	ret+=WriteVariable_String ("COMM",m_comments,pF,pPtrMgr);
	ret+=WriteVariable_String ("COMP",m_company,pF,pPtrMgr);
	ret+=WriteVariable_String ("DESC",m_description,pF,pPtrMgr);
	ret+=WriteVariable_String ("SUBJ",m_subject,pF,pPtrMgr);
	ret+=WriteVariable_String ("TITL",m_title,pF,pPtrMgr);
	ret+=WriteVariable_String ("VERS",m_version,pF,pPtrMgr);
	if (ret != (int(JE_TRUE)*7))
		return JE_FALSE;
	

	if (iUserDataEntries>0)
		{
		if (WriteVariable_String ("KEYN",_itoa(iUserDataEntries,cUserDataEntries,10),pF,pPtrMgr)==JE_FALSE)
			return JE_FALSE;
		for (int X=0;X<iUserDataEntries;X++)
			{	
				ret  = WriteVariable_String ("KEY ",UserData[X].Name,pF,pPtrMgr);
				ret += WriteVariable_String ("KVAL",UserData[X].Value,pF,pPtrMgr);
				ret += WriteVariable_String ("KTYP",UserData[X].Type,pF,pPtrMgr);
				if (ret != (int(JE_TRUE)*3))
					return JE_FALSE;
			}
		}
	return JE_TRUE;
}


//---------------------------------------------------------------------
//  WriteVariable_String                                                                           
//	                                                                                      
//	Write String to File
//
//	Format:	[HEADER][LENGTH_DATA][DATA]
//			4 Bytes  10 Bytes	  X-Bytes
//

jeBoolean CProperties::WriteVariable_String(CString Name, CString Value,jeVFile * pF, jePtrMgr * pPtrMgr)
{
	char	cLength[12];

	if(jeVFile_Write( pF, Name, Name.GetLength()) == JE_FALSE )
		{   jeErrorLog_AddString(JE_ERR_FILEIO_WRITE, "Level_WriteToFile.\n", NULL);
			return JE_FALSE;
		}
	
	sprintf_s (cLength,"%010ld",Value.GetLength());

	if(jeVFile_Write( pF,  cLength, strlen(cLength)) == JE_FALSE )
		{   jeErrorLog_AddString(JE_ERR_FILEIO_WRITE, "Level_WriteToFile.\n", NULL);
			return JE_FALSE;
		}
	if(jeVFile_Write( pF, Value, Value.GetLength()) == JE_FALSE )
		{   jeErrorLog_AddString(JE_ERR_FILEIO_WRITE, "Level_WriteToFile.\n", NULL);
			return JE_FALSE;
		}

	return JE_TRUE;
}

//---------------------------------------------------------------------
//  Properties_ReadFromFile                                                                           
//	                                                                                      
// Read Properties from File
//
jeBoolean CProperties::Properties_ReadFromFile( /*Level * pLevel,*/ jeVFile * pF, jePtrMgr * pPtrMgr)
{
	char* sText{};
	char	sHeader[5];
	char	sLength[11];
	int32	iLength{};
	int32	ActKeyNum=0;

	sHeader[4]='\0';
	sLength[10]='\0';

	for (;;)
	{
		if(jeVFile_Read( pF, sHeader , 4 ) == JE_FALSE )
			break;
		if(jeVFile_Read( pF, sLength , 10 ) == JE_FALSE )
			break;
		iLength = atoi(sLength);
		
		if (sText!=nullptr)
			delete [] sText;

		sText = new char [iLength+1];

		if (iLength >0)
			{
			  if(jeVFile_Read(pF,sText,iLength)==JE_FALSE) 
				break;
			}
		sText[iLength]='\0';

		if (strcmp (sHeader,"AUTH")==0)
			m_author = sText;
		else if (strcmp (sHeader,"COMM")==0)
			m_comments = sText;
		else if (strcmp (sHeader,"COMP")==0)
			m_company = sText;
		else if (strcmp (sHeader,"DESC")==0)
			m_description = sText;
		else if (strcmp (sHeader,"SUBJ")==0)
			m_subject = sText;
		else if (strcmp (sHeader,"TITL")==0)
			m_title = sText;
		else if (strcmp (sHeader,"VERS")==0)
			m_version = sText;
		else if (strcmp (sHeader,"KEYN")==0)
			{ 
				if (UserData!=NULL) DelUserData();
				iUserDataEntries=atoi(sText);
				UserData = new Properties_UserData[iUserDataEntries];
			}
		else if (strcmp (sHeader,"KEY ")==0)
			{	UserData[ActKeyNum].Name=sText;
			}
		else if (strcmp (sHeader,"KVAL")==0)
			{	UserData[ActKeyNum].Value=sText;
			}
		else if (strcmp (sHeader,"KTYP")==0)
			{	UserData[ActKeyNum].Type=sText;
				ActKeyNum++;
			}
	}
    if (sText!=NULL)
		delete [] sText;
	return true;
}


