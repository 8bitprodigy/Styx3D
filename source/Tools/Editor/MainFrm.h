/****************************************************************************************/
/*  MAINFRM.H                                                                           */
/*                                                                                      */
/*  Author:                                                                             */
/*  Description:                                                                        */
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

#if !defined(AFX_MAINFRM_H__37F45633_C0E1_11D2_8B41_00104B70D76D__INCLUDED_)
#define AFX_MAINFRM_H__37F45633_C0E1_11D2_8B41_00104B70D76D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#define WM_UPDATE	(WM_USER+100)

#include "Doc.h"
#include "Sound.h"
#include "Dialogs\Empty.h"		// Lazy way to have empty properties
#include "Dialogs\Groups.h"
#include "Dialogs\models.h"
#include "Dialogs\Lists.h"
#include "Dialogs\Models.h"
//	tom morris feb 2005
//#include "Dialogs\Textures.h"
//	tom morris feb 2005
#include "Dialogs\GlobalMaterials.h"
#include "Dialogs\DialogBuilderDlg.h"
#include "Dialogs\Timeline.h"
#include "TexturesDlg.h"
#include "MainFrm.h"



//	tom morris feb 2005
//#include "dockbars.h"
#include "MyBar.h"
//	end tom morris feb 2005

#include "jetdialog.h"
#include "J3DMainFrm.h"

#include "CTextToolBar.h"

//#include "tk/tkwnd.h"

// These are the tabs which contain one or more dialogs based on context
typedef enum
{
	MAINFRM_COMMANDPANEL_TEXTURES,
	MAINFRM_COMMANDPANEL_LISTS,
	MAINFRM_COMMANDPANEL_MODELS,
	MAINFRM_COMMANDPANEL_GROUPS,
	MAINFRM_COMMANDPANEL_LAST
} MAINFRM_COMMANDPANEL_TAB ;


// These are the possible dialogs for TEMPLATE
typedef enum
{
} MAINFRM_TEMPLATES ;

// These are the possible dialogs for LISTS
typedef enum
{
	MAINFRM_LISTS_TYPE,	
	MAINFRM_LISTS_GROUPS,
	MAINFRM_LISTS_MODELS,
	MAINFRM_LISTS_LAST
} MAINFRM_LISTS ;

// These are all the modeless dialogs making up the command panel
typedef enum
{
	MAINFRM_PANEL_GROUPS,
	MAINFRM_PANEL_TEXTURES,
	MAINFRM_PANEL_MODELS,
	MAINFRM_PANEL_LISTS,
	MAINFRM_PANEL_LAST
} MAINFRM_PANEL ;

// Kouer: AddObjectEx flag possible values (can be an OR of this)
#define MAINFRM_ADDOBJECT_LIST	0x0001
#define MAINFRM_ADDOBJECT_GROUP 0x0002
#define MAINFRM_ADDOBJECT_MODEL 0x0004

class CMainFrame : public CJ3DMainFrame
{
	DECLARE_DYNAMIC(CMainFrame)
public:
	CMainFrame();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMainFrame)
	public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual void RecalcLayout(BOOL bNotify = TRUE);
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	protected:
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CMainFrame();

	//List Management
	void AddSelection( CJweDoc * pDoc );
	void RemoveDeleted();
	void AddObject( Object * pObject );
	void AddObjectEx(Object *pObject, uint32 flags); // Krouer: specify where to add object in the flags
	void RenameObject( Object * pObject );
	void ResetLists( void );
	void RebuildLists( CJweDoc *pDoc );
	void InitObjectList( void );

	//Document Management
	void SetCurrentDocument( CJweDoc * pDoc );
	CJweDoc * GetCurrentDocument( void );
	void CloseCurDoc( void );

	//Commnad Pannel Management
	bool InitCommandPanel( void );
	void SetCommandPanelTab( MAINFRM_COMMANDPANEL_TAB nTab );
	void UpdatePanel( MAINFRM_PANEL ePanel );


	//Property Management
	void SetProperties( jeProperty_List * pArray );
	void UpdateProperties( jeProperty_List * pArray );
	void ResetProperties( void );
	LRESULT OnRebuildProperties( WPARAM wParam, LPARAM lParam ); //Handle Update propery Message
	void PostUpdateProperties();	//Posts and Updante Property Message

	//Subselection Management
	void EndRotateSub( Object * pObject );
	void EndMoveSub( Object * pObject );
	void SubSelectObject( Object * pObject );

	//Misc
	jeBoolean GetCurUserObjName( CString * Name );
//	jeBoolean GetCurAnimateState();
//	void SetCurAnimateState( jeBoolean bAnimate );

	jeSound_System * GetSoundSystem();
	void SetStats( const jeBSP_DebugInfo * pDebugInfo );
	void SetStatusText( const char * Text );
	void UpdateTimeDelta(  float TimeDelta );

	void SetStatusSize(jeFloat X,jeFloat Y,jeFloat Z); // Added JH 4.3.2000
	void SetStatusPos (jeFloat X,jeFloat Y,jeFloat Z); // Added JH 4.3.2000
    void Set3DViewStats(jeFloat fps, int32 nbFaces);      // Added Krouer 2006.01.29

	void SetAccelerator(); // Added JH 4.3.2000

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

//protected:  // control bar embedded members
public:
//	tommorris feb 2005
	//	CStatusBar		m_wndStatusBar;	//	moveed to private:	
	
	BOOL	CreateProgressBar(CProgressCtrl* pProgressCtrl,
			CStatusBar* pStatusBar,
			LPCTSTR szMessage,
			int nPaneIndex,
			int cxMargin,
			int cxMaxWidth,
			UINT nIDControl);
	bool	CreateStatusBar();
	CStatusBar	*GetStatusBar();
	void	ProgressBarBegin(CString strMessage);
	void	ProgressBarSetRange(int iRange, CString strMessage);
	void	ProgressBarSetStep(int iStep, CString strMessage);
	void	ProgressBarStep(CString strMessage);
	void	ProgressBarEnd();

//	CTextures		m_TextureDialog ;	//	new textures dialog
	CTexturesDlg	m_TextureDialog;
//	end tom morris feb 2005


	CTextToolBar	m_wndToolBar;
	CTextToolBar	m_wndModeBar;
	CTextToolBar	m_wndPosSizeBar;
	CTextToolBar	m_wndObjectToolBar;
	CTextToolBar	m_wndEditBar;
	CTextToolBar	m_wndFullscreenBar;
	CGroups			m_GroupDialog ;
	CModel			m_ModelsDialog ;
	CTimeLine		m_TimeLine;
	CEmpty			m_EmptyDialog ;
	CLists			m_ListsDialog ;
	CDialogBuilderDlg	m_PropertiesDlg;

	CFont 			cSmallFont; // Added JH 4.3.200
	CFont 			cMedFont;	// Added JH 4.3.200
	CFont 			cBigFont;	// Added JH 4.3.200


// Generated message map functions
protected:
	//{{AFX_MSG(CMainFrame)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnClose();
//	afx_msg void OnViewToolbar();
	afx_msg void OnViewAllmaterials();
	afx_msg void OnSizing(UINT fwSide, LPRECT pRect);
	afx_msg void OnSize(UINT nType, int cx, int cy);
//	afx_msg void OnToolsUpdateSelection();
//	afx_msg void OnUpdateToolsUpdateSelection(CCmdUI* pCmdUI);
	afx_msg void OnModeAbort();
	afx_msg void OnUpdateModeAbort(CCmdUI* pCmdUI);
//	afx_msg void OnUpdateViewToolbar(CCmdUI* pCmdUI);
	afx_msg void OnEditClear();
	afx_msg void OnMove(int x, int y);
	afx_msg void OnInitMenu(CMenu* pMenu);
//	afx_msg void OnAnimate();
//	afx_msg void OnUpdateAll();

		// EOF JH
	
	//	tom morris feb 2005
	afx_msg void OnIdrAddgroup();
	afx_msg void OnUpdateIdrAddgroup(CCmdUI *pCmdUI);
	afx_msg void OnIdrDeleteitem();
	afx_msg void OnUpdateIdrDeleteitem(CCmdUI *pCmdUI);
	//	end tom morris feb 2005

	//	tom morris may 2005
	afx_msg void OnTimer(UINT nIDEvent);
	//	end tom morris may 2005

	afx_msg BOOL OnBarCheck(UINT nID);
	afx_msg void OnUpdateControlBarMenu(CCmdUI* pCmdUI);
    afx_msg void OnUpdateObjectPosSizeItem(CCmdUI* pCmdUI);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	CJweDoc* m_pCurrentDoc;
	void		DockControlBarLeftOf( CToolBar* Bar,CToolBar* LeftOf ) ;
	CDialog *	DialogFromIndex( MAINFRM_COMMANDPANEL_TAB nTab );



//	CDialogBar					m_CommandPanel;
    CJetTabDialog               m_CommandPanel;
//	CDialogBar					m_wndObjectBar ;
//	CDialogBar					m_MaterialsBar;
//	CDialogBar					m_PropertiesBar;
	CGlobalMaterials			m_GlobalMaterials;
  

	//	tom morris feb 2005
	/*CJetBar*/CMyBar           m_PropBar;
    /*CJetBar*/CMyBar           m_ListBar;
	CStatusBar			m_wndStatusBar;
	CProgressCtrl		m_ProgressControl;
	//	end tom morris 2005


	//CJetTKBar                   m_TKBar;
    //CTkWnd                      m_TKWnd;
	MAINFRM_COMMANDPANEL_TAB	m_eCurrentTab ;
	jeSound_System *			m_SoundSystem;


	// Added JH 5.3.2000
	int			m_iAnim_State;	

	CStatic						m_sXPos;
	CStatic						m_sYPos;
	CStatic						m_sZPos;
	CStatic						m_sXSize;
	CStatic						m_sYSize;
	CStatic						m_sZSize;
	CComboBox				    m_wndObjectType;
	// EOF JH

};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MAINFRM_H__37F45633_C0E1_11D2_8B41_00104B70D76D__INCLUDED_)
