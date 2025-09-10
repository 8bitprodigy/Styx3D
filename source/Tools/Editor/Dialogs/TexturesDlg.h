/****************************************************************************************/
/*  TexturesDlg.h                                                                      */
/*                                                                                      */
/*  Author: Tom Morris                                                                  */
/*  Description:                                                                        */
/*  Date: Feb. 2005                                                                     */
/*                                                                                      */
/*  The contents of this file are subject to the Jet3D Public License                   */
/*  Version 1.02 (the "License"); you may not use this file except in                   */
/*  compliance with the License. You may obtain a copy of the License at                */
/*  http://www.jet3d.com                                                                */
/*                                                                                      */
/*  Software distributed under the License is distributed on an "AS IS"                 */
/*  basis, WITHOUT WARRANTY OF ANY KIND, either express or implied.  See                */
/*  the License for the specific language governing rights and limitations              */
/*  under the License.                                                                  */
/*                                                                                      */
/*  The Original Code is Jet3D, released December 12, 1999.                             */
/*  Copyright (C) 1996-1999 Eclipse Entertainment, L.L.C. All Rights Reserved           */
/*                                                                                      */
/****************************************************************************************/

#pragma once
#include "afxcmn.h"
#include "afxcoll.h"
#include "TextureTreeControl.h"
#include "TextureListControl.h"


/***********************************************************************
*                                class CMyThread
***********************************************************************/
class CMyThread : public CWinThread 
{
     public:
        CMyThread(AFX_THREADPROC proc, LPVOID p );
        virtual ~CMyThread( );
        static CMyThread * BeginThread(AFX_THREADPROC proc, LPVOID p);
        void Shutdown( );
		DWORD	Wait();
        enum { Error, Running, shutdown, Timeout };
     protected: // data

		CWinThread	m_hThread;
		HANDLE ShutDownEvent;
        HANDLE PauseEvent;
};
/**********************************************************************/






// CTexturesDlg dialog

class CTexturesDlg : public CDialog
{
	DECLARE_DYNAMIC(CTexturesDlg)

public:
	CTexturesDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CTexturesDlg();

// Dialog Data
	enum { IDD = IDD_TEXTURES };


protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void OnOK();

	DECLARE_MESSAGE_MAP()


private:
	CTextureListControl		m_ListCtrlTextures;
	CTextureTreeControl		m_treeCtrlGroups;	


	HTREEITEM		m_htRoot, 
					m_htMaster,
					m_htCurrentGroup;
	HTREEITEM		m_htDragItem, m_htDropItem;

	CImageList		m_ImageList32, m_ImageList64, m_ImageList128, m_ImageListGroup;

	CStringArray	m_strArrayMaster, m_strArrayCurGroup;

	CStringList		m_stringListTexturesGroups;

	const	CString		M_STR_IMAGEDIR, M_STR_MASTER;
	CString			m_strSelectedTexture;

	CRect			m_rectClient; 	
	CPoint			m_ptClick;

	bool			m_bBeingSelected, m_bSizing;
	bool			m_bWorkerThreadActive;

	int				m_iSelectedTexture;
	int				m_iNumberofGroups;
	int				m_iTextureWidth;

	MaterialList_Struct *m_pMaterialList;
	DWORD			m_iSizingStart;
	CMyDropTarget	m_DropTarget;
	CMyThread		*m_pthread; // worker thread

	void	SaveTexturesGroupsAll();
	void	RestoreTexturesGroupsAll();
	bool	InitMasterImageLists();
	bool	InitGroupImageList();
	void	DrawThumbnails(CImageList *pImageList, CStringArray	*pArray);
	void	DrawGroupThumbnailsFromWorkerThread();

	void	StartMasterWorkerThread();
	static UINT ComputeMasterWithThread(LPVOID me);
	void	StartGroupWorkerThread();
	static UINT CoumputeGroupWithThread(LPVOID me);

	virtual BOOL OnInitDialog();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnBnClickedRadioTex32();
	afx_msg void OnBnClickedRadioTex64();
	afx_msg void OnBnClickedRadioTex128();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg	void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnBnClickedButtonTexRefresh();
	afx_msg void OnCancel();
	afx_msg void OnDestroy();
public:
	bool		SetSelectedTexture(CString strTexture, CWnd *pOrigin);
	void		SetSelectedGroup(HTREEITEM htGroup);
	HTREEITEM	GetMasterTreeItem();
	HTREEITEM	GetCurGroupTreeItem();
	CString		GetSelectedTexture();
	int			GetTextureDisplayWidth();
	void		SetTextureDisplayWidth(int iWidth);
	void		SetWorkerThreadFlag(bool bActivity);
	bool		GetWorkerThreadActivity();
	void		CollapseGroupsTree();
	void		ExpandGroupTree();
	void		OnIdrDeleteitem();
	void		OnUpdateIdrDeleteitem(CCmdUI *pCmdUI);
	void		OnIdrAddgroup();
	void		OnUpdateIdrAddgroup(CCmdUI *pCmdUI);
};




