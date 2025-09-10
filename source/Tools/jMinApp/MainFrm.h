///////////////////////////////////////////////////////////////////////////////////
//	jMinApp ver. 1.0.b for Jet3D ver.2.x
//	July 23, 2002
//	by Tom Morris
//
//	The target for this application is PROGRAMMERS interested in  real time
//	3D rendering application development. jMinApp is an application shell that
//	exploits the rendering capabilities of Jet3D -- a real time rendering engine.
//	jMinApp demonstrates how to start your own custom applcation shell (game shell)
//	for Jet3D. jMinApp shows how to do this as an MFC application. This MFC approach
//	makes all the huge power of MFC available to you including a large variety
//	of container classes, serialization, document management, and much more...
//
//	You may freely modify this code to transform it into anything you want. Or use bits 
//	of it to build your own, unique shell.
//
//	Good luck and have fun.
//	-Tom
//	www.jet3d.com
/////////////////////////////////////////////////////////////////////////////////////


// MainFrm.h : interface of the CMainFrame class
//


#pragma once

//#include "JetView.h"
#include "Jet.h"

class CMainFrame : public CFrameWnd
{
	
public:
	CMainFrame();
protected: 
	DECLARE_DYNAMIC(CMainFrame)

// Attributes
public:

// Operations
public:

// Overrides
public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);

// Implementation
public:
	virtual ~CMainFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

private:
//		CJetView		m_wndView;
		bool			m_bJetInitializationDone;
		CMenu			m_menuMain;	

public:


	bool	RenderView(jeFloat fElapsedTime);
	bool	InitializeJet3D();
private:

	bool	SetAppPreferences();
	bool	ShutdownAll();
	bool	InitFileSystem(jeEngine* pEngine);
	CString BrowseForWorld(); 
	bool	LoadWorld();
	bool	InitEngine(HWND hWnd);
	bool	LoadDriver();

	void	GetMouseInput();
	void	MoveCamera(float speed, jeVec3d *Direction);
	void	ControlCamera(UINT nFlags, CPoint MouseDelta, short ZoomDelta);		

	bool	ShowMainMenu(bool bShow);
	bool	StartTimer();
	bool	StopTimer();

//	member variables

	//	Jet3D core
	jeEngine		*m_pEngine;			//	engine
	jeVFile         *m_pvFileSys;		//	main virtual file system
	jeResourceMgr	*m_pResourceMgr;	//	resource manager
	jePtrMgr		*m_pPtrMgr;			//	pointer manager
	jeXForm3d		m_xfCamera;			//	camera transform
	jeCamera		*m_pCamera;			//	camera object
	jeSound_System  *m_pSoundSys;		//	Sound sys
	jeWorld         *m_pWorld;			//	World
	JE_Rect			m_Rect;				//	camera rect
	CString			m_strGameName;		//	window title
	CString			m_strDriverName;	//	driver string
	CString			m_strDesiredMode;	//	driver res and bit depth

	//	Resource directories
	CString			m_strBaseDir;		//	current directory
	CString			m_strMaterialDir;	//	GlobalMaterials
	CString			m_strActorDir;		//	Actors
	CString			m_strSoundDir;		//	Sounds
	CString			m_strLevelDir;		//	Levels
	CString			m_strObjectDir;		//	Objects
	CString			m_strLevel;			//	level to load

	//	virtual file systems
	jeVFile			*m_pvfMaterialFile;
	jeVFile			*m_pvfActorFile;
	jeVFile			*m_pvfSoundFile;
	jeVFile			*m_pvfLevelFile;

	//Driver objects
	jeDriver_System *m_pDrvSys;
	jeDriver		*m_pDriver;
	jeDriver_Mode	*m_pMode;

	
	CPoint		m_ptOldMousePoint;		//	for frame-by-frame comparison
	jeVec3d		m_vecCameraPos;			//	for tracking camera from frame-to-frame
	jeFloat		m_fSpeedMultiplier;		//	for adjusting camera speed

	bool		m_bBrowseLevel;			//	to browse or not to browse

	bool		m_bJustMoved;			//	flag for tracking when the window is moved
	CRect		m_rectMainView;

	bool		m_bReadyToRender;		//	render gatekeeper flag
	bool		m_bShuttingDown;
	LARGE_INTEGER	m_LIOldTick, m_LIFreq;	//	time-calc vars



// Generated message map functions
protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSetFocus(CWnd *pOldWnd);
	DECLARE_MESSAGE_MAP()
public:

	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);


//	virtual BOOL OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext);

	afx_msg void OnSize(UINT nType, int cx, int cy);


		afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMoveForward();
	afx_msg void OnMoveBack();
	afx_msg void OnMoveLeft();
	afx_msg void OnMoveRight();
	afx_msg void OnMove(int x, int y);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);

	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);


	afx_msg void OnTimer(UINT_PTR nIDEvent);

	afx_msg void OnClose();

	afx_msg void OnDestroy();
};


