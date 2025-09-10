/////////////////////////////////////////////////////////////////////////////
// MyDropTarget.h: interface for the CMyDropTarget class.
// For tLife ver. 0.3 and tStudio
// Created on: 05/10/2002
// Author:  VolkerBartheld
// Company: TrilobiteWorks
// http://planetarybiology.com/trilobiteworks
//
// (C) 2001 Reetcom / <VolkerBartheld@reetcom.de>
//////////////////////////////////////////////////////////////////////////////

//	***	tPLUGIN SUPPORT	***
//	***	THIS FILE IS NOT MEANT TO BE CUSTOMIZED	***
//	*** DO NOT MODIFY UNLESS ABSOLUTELY NECESSARY	***
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MYDROPTARGET_H__0BA4F434_2F2C_11D5_AE0C_0050DA2A55E0__INCLUDED_)
#define AFX_MYDROPTARGET_H__0BA4F434_2F2C_11D5_AE0C_0050DA2A55E0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <afxole.h>
#include "DropFileArray.h"
#define WM_DROPACTION WM_APP+1 

class  CMyDropTarget : public COleDropTarget
{
public:
	CString		GetstrDroppedFileName();
	int			GetNumDroppedFiles();
	char*		GetFirstDroppedFileName();
	char*		GetDroppedFileName(int iNum=0);
	wchar_t*	GetDroppedFileNameW(int iNum=0);
	wchar_t*	GetFirstDroppedFileNameW();
	BOOL OnDrop( CWnd* pWnd, COleDataObject* pDataObject, DROPEFFECT dropEffect, CPoint point );
	DROPEFFECT OnDragOver( CWnd* pWnd, COleDataObject* pDataObject, DWORD dwKeyState, CPoint point );
	DROPEFFECT OnDragEnter( CWnd* pWnd, COleDataObject* pDataObject, DWORD dwKeyState, CPoint point );
	CMyDropTarget();
	virtual ~CMyDropTarget();
private:
	int				m_iNumDroppedFiles;
	CDropFileArray	m_arDropFiles;
	CString			m_strDroppedFileName;
};

#endif // !defined(AFX_MYDROPTARGET_H__0BA4F434_2F2C_11D5_AE0C_0050DA2A55E0__INCLUDED_)
