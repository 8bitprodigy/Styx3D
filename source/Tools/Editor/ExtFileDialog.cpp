/****************************************************************************************/
/*  ExtFileDialog.CPP                                                                     */
/*                                                                                      */
/*  Author:			J.Hellmann                                                          */
/*  Description:    New Filedialog with Level info and preview                          */
/****************************************************************************************/

#include "stdafx.h"
#include "jwe.h"
#include "ExtFileDialog.h"

#include "Properties.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CExtFileDialog


//	by trilobite jan. 2011
/*	CFileDialog operations have completely changed since Vista and VC 10. Dialog templates
are no longer automatically loaded. There are possible workarounds but there is little control
over the layout of controls. If custom controls are desired here, it is a significant challenge.
Not attempting...
*/


IMPLEMENT_DYNAMIC(CExtFileDialog, CFileDialog)

CExtFileDialog::CExtFileDialog(BOOL bOpenFileDialog, LPCTSTR lpszDefExt, LPCTSTR lpszFileName,
		DWORD dwFlags, LPCTSTR lpszFilter, CWnd* pParentWnd, int preview) :
		CFileDialog(bOpenFileDialog, lpszDefExt, lpszFileName, dwFlags, lpszFilter, pParentWnd)
{
	
	if (preview) // Show Preview Window
		{	m_ofn.Flags |=OFN_ENABLETEMPLATE|OFN_EXPLORER;
			m_ofn.lpTemplateName =MAKEINTRESOURCE(IDD_EXTFILEDIALOG);
		}
	m_preview = preview;
	PreviewData = NULL;
}


CExtFileDialog::CExtFileDialog(BOOL bOpenFileDialog, BOOL Preview)
:	CFileDialog(bOpenFileDialog)
{	
	if (Preview)
		{	m_ofn.Flags |=OFN_ENABLETEMPLATE|OFN_EXPLORER;
			m_ofn.lpTemplateName =MAKEINTRESOURCE(IDD_EXTFILEDIALOG);
		}
	m_preview = Preview;
	PreviewData = NULL;
}


BEGIN_MESSAGE_MAP(CExtFileDialog, CFileDialog)
	//{{AFX_MSG_MAP(CExtFileDialog)
	ON_WM_SIZE()
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


//	by trilobite jan/ 2011. OnInitDialog() not overridable since Vista
BOOL CExtFileDialog::OnInitDialog() 
{
	CFileDialog::OnInitDialog();
	
	m_stRect.SubclassDlgItem ( IDC_STATIC_PREVIEW,this);

	return TRUE;  
}

void CExtFileDialog::OnSize(UINT nType, int cx, int cy) 
{
	CFileDialog::OnSize(nType, cx, cy);

	if (!m_preview) return;

	CRect srect{};
	CRect cancelrect{};
	CRect drect{};

	//	by trilobite jan. 2011 - testing with if statements 
	if (GetParent()->GetDlgItem(IDCANCEL))
	{
		GetParent()->GetDlgItem(IDCANCEL)->GetWindowRect(&cancelrect);
	
		if (GetDlgItem( IDC_EXTFILEDIALOG_TITLE_AUTHOR ))
		{
			GetDlgItem( IDC_EXTFILEDIALOG_TITLE_AUTHOR )->GetWindowRect(&srect);
			GetDlgItem( IDC_EXTFILEDIALOG_TITLE_AUTHOR )->GetWindowRect(&drect);
			ScreenToClient(&cancelrect);	
			ScreenToClient(&srect);	
			ScreenToClient(&drect);	
			drect.top    = srect.bottom+6;
			drect.bottom = cancelrect.bottom;
		
			if (GetDlgItem( IDC_EXTFILEDIALOG_DESCRIPTION ))
			{
   
				GetDlgItem( IDC_EXTFILEDIALOG_DESCRIPTION )->MoveWindow(&drect,true);

				if (GetDlgItem( IDC_STATIC_LINE ))
				{
					GetDlgItem( IDC_STATIC_LINE )->GetWindowRect(&drect);
					ScreenToClient(&drect);	
					drect.bottom =  cancelrect.bottom;
					GetDlgItem( IDC_STATIC_LINE )->MoveWindow(&drect,true);
				}
			}
		}
	}
	//	end trilobite jan,. 2011
}


BOOL CExtFileDialog::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
	ASSERT(pResult != NULL);

	if (!m_preview) 	return CFileDialog::OnNotify(wParam, lParam, pResult);

	// allow message map to override

	OFNOTIFY* pNotify = (OFNOTIFY*)lParam;
	switch(pNotify->hdr.code)
	{
	case CDN_SELCHANGE:
		OnFileNameChange();
		return TRUE;
	case CDN_FOLDERCHANGE:
		OnFolderChange();
		return TRUE;
		
	default:
		break;
	}
	return CFileDialog::OnNotify(wParam, lParam, pResult);
}


void CExtFileDialog::OnFileNameChange()
{
	if (!m_preview) return;

	jePtrMgr	*	pPtrMgr = nullptr;
	jeVFile		*	pF		= nullptr ;	
	jeVFile		*	pFS		= nullptr ;
	jeBoolean		ret;

	long			Length{};
	

	pFS = jeVFile_OpenNewSystem
	(
		NULL, 
		JE_VFILE_TYPE_VIRTUAL,
		GetPathName(),
		NULL,
		JE_VFILE_OPEN_READONLY|JE_VFILE_OPEN_DIRECTORY
	);

	if( pFS == NULL )
		{
		  //	by trilobite	Jan. 2011 - note: custom template and controls not recognized since Vista
			if (GetDlgItem( IDC_EXTFILEDIALOG_TITLE ))
				GetDlgItem( IDC_EXTFILEDIALOG_TITLE )->SetWindowText("");
			if (GetDlgItem( IDC_EXTFILEDIALOG_TITLE_AUTHOR ))
				GetDlgItem( IDC_EXTFILEDIALOG_TITLE_AUTHOR )->SetWindowText("");
			if (GetDlgItem( IDC_EXTFILEDIALOG_DESCRIPTION ))
				GetDlgItem( IDC_EXTFILEDIALOG_DESCRIPTION )->SetWindowText("");
		  return;
		}

	CProperties	PropsDialog;				

	pF = jeVFile_Open( pFS, "LevelProperties", JE_VFILE_OPEN_READONLY);
	if( pF != NULL )
	{
		if( PropsDialog.Properties_ReadFromFile( pF, pPtrMgr ) == JE_FALSE )
		{	jeVFile_Close( pFS ) ;
			return;
		}

		if( jeVFile_Close( pF ) == JE_FALSE )
		{	jeVFile_Close( pFS ) ;
			return;
		}
	}

	if (PreviewData)
		{
		  delete PreviewData;
		  PreviewData  = nullptr;
		}

	pF = jeVFile_Open( pFS, "LevelThumbnail", JE_VFILE_OPEN_READONLY);
	if( pF != nullptr )
	{	
		ret = jeVFile_Size(pF,&Length);

		if (ret==JE_TRUE)
		{
			PreviewData = new char [Length];
			ret = jeVFile_Read(pF,PreviewData,Length);
			
			if (ret==JE_TRUE)
			{
				lpbiPreview = (LPBITMAPINFOHEADER)(PreviewData+sizeof(BITMAPFILEHEADER));
				m_stRect.SetBitmap (lpbiPreview);
			}
		}

		if( jeVFile_Close( pF ) == JE_FALSE )
		{	jeVFile_Close( pFS ) ;
			return;
		}

	}
	else
	{
		// KROUER - Fix the Open file dialog crash
		m_stRect.SetBitmap(NULL);
	}

	jeVFile_Close( pFS ) ;
	
	//	by trilobite	Jan. 2011 - custom template and controls not recognized since Vista
	if (GetDlgItem( IDC_EXTFILEDIALOG_TITLE ))
		GetDlgItem( IDC_EXTFILEDIALOG_TITLE )->SetWindowText(PropsDialog.m_title);
	if (GetDlgItem( IDC_EXTFILEDIALOG_TITLE_AUTHOR ))
		GetDlgItem( IDC_EXTFILEDIALOG_TITLE_AUTHOR )->SetWindowText(PropsDialog.m_author);
	if (GetDlgItem( IDC_EXTFILEDIALOG_DESCRIPTION ))
		GetDlgItem( IDC_EXTFILEDIALOG_DESCRIPTION )->SetWindowText(PropsDialog.m_description);
	Invalidate(true);
}


void CExtFileDialog::OnFolderChange()
{
	if (!m_preview) return;
	//	by trilobite	Jan. 2011 - custom template and controls not recognized since Vista
	if (GetDlgItem( IDC_EXTFILEDIALOG_TITLE ))
		GetDlgItem( IDC_EXTFILEDIALOG_TITLE )->SetWindowText("");
	if (GetDlgItem( IDC_EXTFILEDIALOG_TITLE_AUTHOR ))
		GetDlgItem( IDC_EXTFILEDIALOG_TITLE_AUTHOR )->SetWindowText("");
	if (GetDlgItem( IDC_EXTFILEDIALOG_DESCRIPTION ))
		GetDlgItem( IDC_EXTFILEDIALOG_DESCRIPTION )->SetWindowText("");
}

void CExtFileDialog::OnDestroy() 
{
	CFileDialog::OnDestroy();
	
	// TODO: Code für die Behandlungsroutine für Nachrichten hier einfügen
	if (PreviewData)
		{
		  delete PreviewData;
		  PreviewData  = nullptr;
		}
	
}
