// DocManager.cpp : implementation file
//

#include "stdafx.h"
#include "ExtFileDialog.h"
#include "DocManagerEx.h" // the header with the class declaration



static void AppendFilterSuffix(CString& filter, OPENFILENAME& ofn,
	CDocTemplate* pTemplate, CString* pstrDefaultExt)
{
	ASSERT_VALID(pTemplate);
	ASSERT_KINDOF(CDocTemplate, pTemplate);

	CString strFilterExt, strFilterName;
	if (pTemplate->GetDocString(strFilterExt, CDocTemplate::filterExt) &&
	 !strFilterExt.IsEmpty() &&
	 pTemplate->GetDocString(strFilterName, CDocTemplate::filterName) &&
	 !strFilterName.IsEmpty())
	{
		// a file based document template - add to filter list
#ifndef _MAC
		ASSERT(strFilterExt[0] == '.');
#endif
		if (pstrDefaultExt != NULL)
		{
			// set the default extension
#ifndef _MAC
			*pstrDefaultExt = ((LPCTSTR)strFilterExt) + 1;  // skip the '.'
#else
			*pstrDefaultExt = strFilterExt;
#endif
			ofn.lpstrDefExt = (LPTSTR)(LPCTSTR)(*pstrDefaultExt);
			ofn.nFilterIndex = ofn.nMaxCustFilter + 1;  // 1 based number
		}

		// add to filter
		filter += strFilterName;
		ASSERT(!filter.IsEmpty());  // must have a file type name
		filter += (TCHAR)'\0';  // next string please
#ifndef _MAC
		filter += (TCHAR)'*';
#endif
		filter += strFilterExt;
		filter += (TCHAR)'\0';  // next string please
		ofn.nMaxCustFilter++;
	}
}


/////////////////////////////////////////////////////////////////////////////
// CDocManagerEx

IMPLEMENT_DYNAMIC(CDocManagerEx, CDocManager)

CDocManagerEx::CDocManagerEx()
{
	// Krouer: push the current directory
	GetCurrentDirectory(2048, m_szCurrentPath.GetBuffer(2048));
	m_szCurrentPath.ReleaseBuffer();
}

CDocManagerEx::~CDocManagerEx()
{
	OutputDebugString("CDocManagerEx::~CDocManagerEx()\n");
	//free(g_szCurPath);
}

BOOL CDocManagerEx::DoPromptFileName(CString& fileName, UINT nIDSTitle, DWORD lFlags, BOOL bOpenFileDialog, CDocTemplate* pTemplate)
{
	CExtFileDialog dlgFile(bOpenFileDialog,bOpenFileDialog); // this is the only modified line! 

	CString title;
	VERIFY(title.LoadString(nIDSTitle));

	dlgFile.m_ofn.Flags |= lFlags;

	CString strFilter;
	CString strDefault;
	if (pTemplate != NULL)
	{
		ASSERT_VALID(pTemplate);
		AppendFilterSuffix(strFilter, dlgFile.m_ofn, pTemplate, &strDefault);
	}
	else
	{
		// do for all doc template
		POSITION pos = m_templateList.GetHeadPosition();
		BOOL bFirst = TRUE;
		while (pos != NULL)
		{
			CDocTemplate* pTemplate = (CDocTemplate*)m_templateList.GetNext(pos);
			AppendFilterSuffix(strFilter, dlgFile.m_ofn, pTemplate,
				bFirst ? &strDefault : NULL);
			bFirst = FALSE;
		}
	}

	// append the "*.*" all files filter
	CString allFilter;
	VERIFY(allFilter.LoadString(AFX_IDS_ALLFILTER));
	strFilter += allFilter;
	strFilter += (TCHAR)'\0';   // next string please
#ifndef _MAC
	strFilter += _T("*.*");
#else
	strFilter += _T("****");
#endif
	strFilter += (TCHAR)'\0';   // last string
	dlgFile.m_ofn.nMaxCustFilter++;

	dlgFile.m_ofn.lpstrFilter = strFilter;
#ifndef _MAC
	dlgFile.m_ofn.lpstrTitle = title;
#else
	dlgFile.m_ofn.lpstrPrompt = title;
#endif
	dlgFile.m_ofn.lpstrFile = fileName.GetBuffer(_MAX_PATH);

	dlgFile.m_preview= bOpenFileDialog; // this is the only modified line, too :) 

	BOOL bResult = dlgFile.DoModal() == IDOK ? TRUE : FALSE;
	fileName.ReleaseBuffer();

	// Krouer: pop the current directory
	SetCurrentDirectory(m_szCurrentPath);
	
/*
#ifdef _DEBUG
	char maxpath[256];
	GetCurrentDirectory(256, maxpath);
	OutputDebugString(maxpath);
	OutputDebugString("\n");
#endif
*/
	return bResult;
}

const TCHAR myIconIndexFmt[]   = _T(",%d");
const TCHAR myDefaultIconFmt[] = _T("%s\\DefaultIcon");

BOOL SetRegKey(LPCTSTR lpszKey, LPCTSTR lpszValue, LPCTSTR lpszValueName = NULL)
{
	if (lpszValueName == NULL)
	{
		if (::RegSetValue(HKEY_CLASSES_ROOT, lpszKey, REG_SZ,
			  lpszValue, lstrlen(lpszValue) * sizeof(TCHAR)) != ERROR_SUCCESS)
		{
			return FALSE;
		}
		return TRUE;
	}
	else
	{
		HKEY hKey;

		if(::RegCreateKey(HKEY_CLASSES_ROOT, lpszKey, &hKey) == ERROR_SUCCESS)
		{
			LONG lResult = ::RegSetValueEx(hKey, lpszValueName, 0, REG_SZ,
				(CONST BYTE*)lpszValue, (lstrlen(lpszValue) + 1) * sizeof(TCHAR));

			if(::RegCloseKey(hKey) == ERROR_SUCCESS && lResult == ERROR_SUCCESS)
				return TRUE;
		}
		return FALSE;
	}
}


// Krouer: Register the MaterialFile, Prefab file icons
void CDocManagerEx::RegisterOtherFileTypes()
{
    CString strTypeName;
    CString strFilterExt;
    CString strTypeId;
    CString strFormat;
    CString strPathName;

    AfxGetModuleShortFileName(AfxGetInstanceHandle(), strPathName);

    UINT nID = IDR_GWFTYPE+1;
    strFormat.LoadString(NULL, nID);
    while (!strFormat.IsEmpty()) {
        // Extract the needed information
        AfxExtractSubString(strTypeName, strFormat, (int)CDocTemplate::regFileTypeName);
        AfxExtractSubString(strTypeId, strFormat, (int)CDocTemplate::regFileTypeId);
        AfxExtractSubString(strFilterExt, strFormat, (int)CDocTemplate::filterExt);

		CString strDefaultIconCommandLine = strPathName;
		CString strIconIndex;
		HICON hIcon = ::ExtractIcon(AfxGetInstanceHandle(), strPathName, nID-IDR_GWFTYPE+1);
		if (hIcon != NULL)
		{
			strIconIndex.Format(myIconIndexFmt, nID-IDR_GWFTYPE+1);
			DestroyIcon(hIcon);
		}
		else
		{
			strIconIndex.Format(myIconIndexFmt, 0);
		}
		strDefaultIconCommandLine += strIconIndex;

        // Register the Type of the file
        SetRegKey(strTypeId, strTypeName);

        // Register the default icons
        CString strTemp;
        strTemp.Format(myDefaultIconFmt, (LPCTSTR)strTypeId);
		SetRegKey(strTemp, strDefaultIconCommandLine);

        // Register the extension
        SetRegKey(strFilterExt, strTypeId);

        // Go to the next file extension
        strFormat.Empty();
        strFormat.LoadString(NULL, ++nID);
    }
}

