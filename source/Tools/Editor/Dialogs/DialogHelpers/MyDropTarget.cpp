// MyDropTarget.cpp: implementation of the CMyDropTarget class.
//
// (C) 2001 Reetcom / <VolkerBartheld@reetcom.de>
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MyDropTarget.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////



CMyDropTarget::CMyDropTarget()
{
	m_arDropFiles.RemoveAll();                                                    // remove all existing object from the array
	m_iNumDroppedFiles  = 0;                                                      // reset number of dropped files
	m_strDroppedFileName = _T("");	
}



CMyDropTarget::~CMyDropTarget()
{
  m_arDropFiles.RemoveAll();                                                    // remove all existing object from the array
}



DROPEFFECT CMyDropTarget::OnDragEnter(CWnd *pWnd, COleDataObject *pDataObject, DWORD dwKeyState, CPoint point)
{
  return(DROPEFFECT_COPY);
}



DROPEFFECT CMyDropTarget::OnDragOver(CWnd *pWnd, COleDataObject *pDataObject, DWORD dwKeyState, CPoint point)
{
  return(DROPEFFECT_COPY);
}


//
// process the drop action
//
BOOL CMyDropTarget::OnDrop(CWnd *pWnd, COleDataObject *pDataObject, DROPEFFECT dropEffect, CPoint point)
{
	//
	// for more info on Drag&Drop and the various Shell Data Scenarios see
	// "Transferring Shell Objects with Drag-Drop and the Clipboard" (MSDN OCT 2000 and later)
	//
	BOOL      bIsAvail, bWide;
	FORMATETC FormatEtc;
	wchar_t   *lpBufW, *pwcDropFile;
	char      *lpBuf;
	LPVOID    lpVoid;
	DROPFILE  DropFile;
	DROPFILES *pDropFiles;

	m_strDroppedFileName = _T("");
	
	m_arDropFiles.RemoveAll();                                                    // remove all existing object from the array
	m_iNumDroppedFiles  = 0;                                                      // reset number of dropped files
	pDataObject->BeginEnumFormats();                                              // begin to start the enumeration
	
	while(pDataObject->GetNextFormat(&FormatEtc))                                 // loop over all data objects that might be present
    {
		bIsAvail = pDataObject->IsDataAvailable(CF_HDROP, &FormatEtc);              // check if data is available and fill the FormatEtc struct
		if(bIsAvail && (FormatEtc.cfFormat==CF_HDROP))                              // we need format #15 (wide char list of dropped files)
		{
			CFile* pcFile = pDataObject->GetFileData(CF_HDROP, &FormatEtc);           // attach CFile object to pDataObject
			DWORD dwLength = (DWORD)pcFile->GetLength();                                     // retrieve length of data
			if( NULL == (lpBufW = (wchar_t*)(lpVoid = malloc(dwLength))) )
			{
				TRACE("Memory allocation error.");
				break;
			}
			else
			{
				if(dwLength != pcFile->Read(lpBufW, dwLength)) TRACE("File read error"); // report file err if num of bytes read doesn't match
				else
				{
					pDropFiles  =  (DROPFILES*)lpBufW;
					pwcDropFile =  DropFile.wcDropFile;                                   // set up temporary buffer for actiual dropped file
					dwLength    -= pDropFiles->pFiles;                                    // subtract to offset value from buffer
					lpBufW      += pDropFiles->pFiles/2;                                  // advance buffer to start of filename area
					lpBuf       =  (char*)lpBufW;
					bWide       =  pDropFiles->fWide;                                     // set flag if wide charset is used
					
					while( (dwLength-=bWide?2:1)>0 )                                      // loop over all files (increment by two if wide char)
					{
						if(!(*pwcDropFile++ = bWide?*lpBufW++:*lpBuf++))                    // implicit conversion to wide char if necessary
						{                                                                 // if end of filename reached,
							m_arDropFiles.SetAtGrow(m_iNumDroppedFiles++, DropFile);          // fill a (new) CArray element and increase counter
							pwcDropFile = DropFile.wcDropFile;                                // reset pointer to buffer
						}
					}
					
					PostMessage(pWnd->m_hWnd, WM_DROPACTION, 0, 0);                       // signal a file drop action
				}
				free(lpVoid);                                                           // free temporary buffer
			}
			pcFile->Close();                                                          // and perform some cleanup stuff
			delete pcFile;
			break;
		}

		bIsAvail = pDataObject->IsDataAvailable(CF_TEXT, &FormatEtc);              // check if data is available and fill the FormatEtc struct
		if(bIsAvail && (FormatEtc.cfFormat==CF_TEXT))                              // we need format #15 (wide char list of dropped files)
		{
			HGLOBAL  hGlobal;
			LPCSTR   pData;

			hGlobal=pDataObject->GetGlobalData(CF_TEXT); //Change to CF_TEXT if you are only working with text

			pData=(LPCSTR)GlobalLock(hGlobal);    
			ASSERT(pData!=NULL); 
                   
			//pWnd->SetWindowText( pData );
			m_strDroppedFileName = pData;

			GlobalUnlock(hGlobal);

			PostMessage(pWnd->m_hWnd, WM_DROPACTION, 0, 0);   

		}

    }
	
	return(m_iNumDroppedFiles>0);                                                 // return true if any files dropped
}



//
// return number of dropped files
//
int CMyDropTarget::GetNumDroppedFiles()
{
  return(m_iNumDroppedFiles);
}


CString		CMyDropTarget::GetstrDroppedFileName()
{
	return m_strDroppedFileName;
}



//
// return wide char name of first dropped file
// will return NULL if iNum out of range or no files dropped
//
wchar_t* CMyDropTarget::GetFirstDroppedFileNameW()
{
  if(m_iNumDroppedFiles>0) return(m_arDropFiles.GetAt(0).wcDropFile);
  else return(NULL);
}




//
// return wide char name of iNum dropped file (starting with 0)
// will return NULL if iNum out of range or no files dropped
//
wchar_t* CMyDropTarget::GetDroppedFileNameW(int iNum)
{                                                                               // just forward the pointer to array element
  if( (m_iNumDroppedFiles>0) && (iNum < m_iNumDroppedFiles) ) return(m_arDropFiles.GetAt(iNum).wcDropFile);
  else return(NULL);
}




//
// return ASCII char name of first dropped file
// allocates memory for file name buffer
// will return NULL if iNum out of range or no files dropped
//
char* CMyDropTarget::GetFirstDroppedFileName()
{
  if(!m_iNumDroppedFiles) return(NULL);                                         // if no files were dropped, return a NULL pointer

  char* pszDropFileName;
  wchar_t* pwc = m_arDropFiles.GetAt(0).wcDropFile;                             // define a shortcut to the first dropped file

  int iLen = WideCharToMultiByte(CP_ACP, 0, pwc, -1, NULL, 0, NULL, NULL);      // calculate the length of the name's ASCII version

  if( NULL == (pszDropFileName = (char*)malloc(iLen)) ) return(NULL);           // allocate an appropriate amount of memory
                                                                                // and do the conversion
  iLen = WideCharToMultiByte(CP_ACP, 0, pwc, -1, pszDropFileName, iLen, NULL, NULL);
  if(!iLen)                                                                     // if the conversion failed, free mem and return error
    {
    free(pszDropFileName);
    pszDropFileName = NULL;
    }

  return(pszDropFileName);
}




//
// return ASCII char name of iNum dropped file (starting with 0)
// allocates memory for file name buffer
// will return NULL if iNum out of range or no files dropped
//
char* CMyDropTarget::GetDroppedFileName(int iNum)
{
  if( (!m_iNumDroppedFiles) || (iNum >= m_iNumDroppedFiles) ) return(NULL);     // validate range of parameter

  char* pszDropFileName;
  wchar_t* pwc = m_arDropFiles.ElementAt(iNum).wcDropFile;                            // get a reference pointer (GetAt() won't work here!)

  int iLen = WideCharToMultiByte(CP_ACP, 0, pwc, -1, NULL, 0, NULL, NULL);            // get buffer length

  if( NULL == (pszDropFileName = (char*)malloc(iLen)) ) return(NULL);                 // reserve buffer
  iLen = WideCharToMultiByte(CP_ACP, 0, pwc, -1, pszDropFileName, iLen, NULL, NULL);  // do the conversion
  if(!iLen)                                                                           // report error, if conversion has failed
    {
    free(pszDropFileName);
    pszDropFileName = NULL;
    }

  return(pszDropFileName);                                                      // return pointer to (new) ASCII file name
}
