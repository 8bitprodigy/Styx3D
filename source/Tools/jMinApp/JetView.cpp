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


// JetView.cpp : implementation of the CJetView class
//

#include "stdafx.h"
#include "jMinApp.h"
#include "JetView.h"
#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//	timer for cycling render loops -- in milliseconds
#define TIMER_SPEED		40
//	width and height for windowed _DEBUG display -- in pixels
#define	DEBUG_WIDTH		400
#define	DEBUG_HEIGHT	300





// CJetView

////////////////////////////////////////////////////////////////////////////////
//	SubLarge
//	for timing calculations
////////////////////////////////////////////////////////////////////////////////
static void SubLarge(LARGE_INTEGER *start, LARGE_INTEGER *end, LARGE_INTEGER *delta)
{
	_asm 
	{
		mov ebx,dword ptr [start]
		mov esi,dword ptr [end]

		mov eax,dword ptr [esi+0]
		sub eax,dword ptr [ebx+0]

		mov edx,dword ptr [esi+4]
		sbb edx,dword ptr [ebx+4]

		mov ebx,dword ptr [delta]
		mov dword ptr [ebx+0],eax
		mov dword ptr [ebx+4],edx
	}
}

////////////////////////////////////////////////////////////////////////////////
//	Construction / Destruction
//
////////////////////////////////////////////////////////////////////////////////
CJetView::CJetView()
{
	m_bReadyToRender = false;
	
	
}

//	destruction
CJetView::~CJetView()
{
}

//	intercept messages sent around by the MFC framework
BEGIN_MESSAGE_MAP(CJetView, CWnd)
	ON_WM_PAINT()
	ON_WM_CREATE()
	ON_WM_TIMER()
	ON_WM_DESTROY()
	ON_WM_CLOSE()

END_MESSAGE_MAP()


////////////////////////////////////////////////////////////////////////////////
//	PreCreateWindow
//
////////////////////////////////////////////////////////////////////////////////
BOOL CJetView::PreCreateWindow(CREATESTRUCT& cs) 
{
	if (!CWnd::PreCreateWindow(cs))
		return FALSE;

//	modify the window style for the display window

#ifdef _DEBUG	//	windowed display
	cs.lpszName = _T("jMinApp_DEBUG");
//	cs.dwExStyle |= WS_EX_TOPMOST;
//	cs.style &=~WS_VISIBLE;
	cs.style&=~WS_BORDER;
	cs.style&=~WS_CAPTION;
	cs.style&=~WS_THICKFRAME;
	cs.dwExStyle&=~WS_EX_CLIENTEDGE;
	cs.lpszClass = AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS | CS_OWNDC | CS_SAVEBITS, ::LoadCursor(NULL, IDC_ARROW), 
		(HBRUSH)GetStockObject(NULL_BRUSH));
#endif

#ifdef NDEBUG	//	fullscreen
	cs.lpszName = _T("jMinApp");
	cs.dwExStyle |= WS_EX_TOPMOST;
	cs.style &=~WS_VISIBLE;
	cs.style &= SW_SHOWMAXIMIZED;
	cs.lpszClass = AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS /*| CS_OWNDC*/ | CS_SAVEBITS, NULL, 
		(HBRUSH)GetStockObject(NULL_BRUSH));
#endif

	return TRUE;
}


////////////////////////////////////////////////////////////////////////////////
//	OnCreate
//
////////////////////////////////////////////////////////////////////////////////
int CJetView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	CMainFrame	*pMainFrame = NULL;
	pMainFrame = (CMainFrame*)AfxGetMainWnd();

	if (m_hWnd)
	{
		CRect	rectMainView;
		//	get the monitor's native resolution
		int		iScreenWidth = ::GetSystemMetrics(SM_CXSCREEN);
		int		iScreenHeight = ::GetSystemMetrics(SM_CYSCREEN);

		m_rectMainView.SetRect(0, 0, iScreenWidth, iScreenHeight);


		//	modify the window attributes... one last time
#ifdef _DEBUG	//	windowed display

		m_Rect.Top = m_rectMainView.top;
		m_Rect.Left = m_rectMainView.left;
		m_Rect.Bottom = m_rectMainView.bottom;
		m_Rect.Right =  m_rectMainView.right;
		
//		SetWindowPos(NULL, m_rectMainView.left , m_rectMainView.top,
//			m_rectMainView.Width(),
//			m_rectMainView.Height(),
//			/*SWP_NOCOPYBITS | SWP_NOZORDER |*/ WS_EX_TOPMOST |SWP_SHOWWINDOW  );

//			ShowWindow(SW_SHOWMAXIMIZED);
		
#endif


#ifdef NDEBUG	//	fullscreen


		//	now change our res string to reflfect the monitor's native res
//		m_strDesiredMode.Format(_T("%dx%dx16"), iScreenWidth, iScreenHeight);

		// remove the caption of this window:
		LONG style = ::GetWindowLong(m_hWnd,GWL_STYLE);
		style&=~WS_CAPTION;
		style&=~WS_THICKFRAME;
		::SetWindowLong(m_hWnd,GWL_STYLE,style);

		ShowWindow(SW_SHOWMAXIMIZED);
		ShowCursor(FALSE);

		m_Rect.Top = 0;
		m_Rect.Left = 0;
		m_Rect.Bottom = iScreenHeight -1;
		m_Rect.Right = iScreenWidth - 1;

		SetWindowPos(&wndTopMost, 0, 0,
			iScreenWidth,
			iScreenHeight,
			/*SWP_NOCOPYBITS | SWP_NOZORDER |*/ WS_EX_TOPMOST | SW_SHOWMAXIMIZED);	

#endif
/*		//	get the prefs from the jMinApp.ini and set prefs accordingly

		if (pMainFrame)
		{
			if (!SetAppPreferences())
			{			
				AfxMessageBox("Failed to set app preferences.\n\n Shutting Down...");
				pMainFrame->PostMessage(WM_CLOSE);
				return -1;
			}

			//	if the BROWSE line is set to 'true' in jMinapp.ini,
			//	open a CFileDialog and browse for the level we want.
			if (m_bBrowseLevel)
			{
				CString		strBlankPath = _T("");
				CString		strBrowsePath = strBlankPath;

				strBrowsePath = BrowseForWorld();
				if (strBrowsePath != strBlankPath)
					m_strLevel = strBrowsePath;
			}

			if (!m_pEngine)	
			{	
				//	initialize the engine
				if (!InitEngine(this->GetSafeHwnd()))
				{			
					AfxMessageBox("Failed to initialize Engine.\n\n Shutting Down...");
					pMainFrame->PostMessage(WM_CLOSE);
					return -1;
				}
			}

			//	register world objects located in the 'objects' dir
			if (!jeEngine_RegisterObjects(m_strObjectDir.GetBuffer(m_strObjectDir.GetLength())))
			{			
				AfxMessageBox("Failed to register objects.\n\n Shutting Down...");
				pMainFrame->PostMessage(WM_CLOSE);
				return -1;
			}

			//	initialize the many different file systems
			if (!InitFileSystem(m_pEngine))
			{			
				AfxMessageBox("Failed to initialize file system.\n\n Shutting Down...");
				pMainFrame->PostMessage(WM_CLOSE);
				return -1;
			}

			//	load the world
			if (!LoadWorld())
			{
				AfxMessageBox("Failed to load world.\n\n Shutting Down...");
				ShutdownAll();
				return -1;
			}
		}	//	if (pMainFrame...

		//	start off with cursor at window center
		GetClientRect(&rectMainView);
		ClientToScreen(rectMainView);
		::SetCursorPos(rectMainView.Width()/2, rectMainView.Height()/2);

		//	initialize the cusor point member at screen center
		CPoint	point(0,0);
		GetCursorPos(&point);
		m_ptOldMousePoint = point;

		//	initialize timer member variables
		QueryPerformanceCounter(&m_LIOldTick);
		QueryPerformanceFrequency(&m_LIFreq);

		//	set rendering gatekeeper
		m_bReadyToRender = true;

		//	start the timer (necessary with this style of MFC app).
		//	the timer triggers each render cycle -- sorta like the Main Loop.
		if (!StartTimer())
		{
			AfxMessageBox("Failed to start timer.\n\n Shutting Down...");
			ShutdownAll();
			return -1;
		}
		return 0;
*/
		return 0;
	}	//	if (m_hWnd...

	return -1;
}


////////////////////////////////////////////////////////////////////////////////
//	OnDestroy
//	for the destruction of this CWnd -- nothing to do with Jet3D
////////////////////////////////////////////////////////////////////////////////
void CJetView::OnDestroy()
{
	CWnd::OnDestroy();
	
	StopTimer();
}



////////////////////////////////////////////////////////////////////////////////
//	OnClose
//	
////////////////////////////////////////////////////////////////////////////////
void CJetView::OnClose()
{
//	ShutdownAll();	//	posts WM_CLOSE message
	// TODO: Add your message handler code here and/or call default

//	CWnd::OnClose();	// commented out to prevent endless loop
}



////////////////////////////////////////////////////////////////////////////////
//	OnPaint
//	ignore
////////////////////////////////////////////////////////////////////////////////
void CJetView::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	// TODO: Add your message handler code here


	// Do not call CWnd::OnPaint() for painting messages
}




////////////////////////////////////////////////////////////////////////////////
//	StartTimer
//	used for cycling calls to render and render and render and render...
////////////////////////////////////////////////////////////////////////////////
bool CJetView::StartTimer()
{
			//	initialize timer member variables
		QueryPerformanceCounter(&m_LIOldTick);
		QueryPerformanceFrequency(&m_LIFreq);

			//	set rendering gatekeeper
		m_bReadyToRender = true;

	if (SetTimer(IDT_JETVIEW_TIMER, TIMER_SPEED, NULL) == 0)
		return false;

	return true;
}



////////////////////////////////////////////////////////////////////////////////
//	StopTimer
//
////////////////////////////////////////////////////////////////////////////////
bool CJetView::StopTimer()
{
	if (!KillTimer(IDT_JETVIEW_TIMER))
	{
		return false;
	}
	return true;
}



////////////////////////////////////////////////////////////////////////////////
//	OnTimer	-- Basically, this is the MAIN LOOP
//	called by the MFC framework
////////////////////////////////////////////////////////////////////////////////
void CJetView::OnTimer(UINT nIDEvent)
{

	CMainFrame*	pMainFrame = NULL;
	pMainFrame = (CMainFrame*)AfxGetMainWnd();
	if (pMainFrame)
	{
	if (nIDEvent == IDT_JETVIEW_TIMER)
	{
		if (m_bReadyToRender)
		{
			jeFloat			fElapsedTime(0.0f);
			LARGE_INTEGER	LICurTick, LIDeltaTick;

			//	close the gate -- so no interruptions until we're completely
			//	done with this rendering cycle
			m_bReadyToRender = false;

			//	Get timing info, get current tick
			QueryPerformanceCounter(&LICurTick);

			//	Get the difference between current tick and oldtick
			SubLarge(&m_LIOldTick, &LICurTick, &LIDeltaTick);

			//	OldTick now = CurTick
			m_LIOldTick = LICurTick;

			//	Get the time (in seconds) that past since last frame iteration
			if (LIDeltaTick.LowPart > 0)
			{
				fElapsedTime =  1.0f / (((float)m_LIFreq.LowPart / (float)LIDeltaTick.LowPart));
			}
			else 
			{		
				fElapsedTime = 0.001f;
			}
			
			//	render the view and modify the the gatekeeper flag
			//	as a result
			m_bReadyToRender = pMainFrame->RenderView(fElapsedTime);
		}
	}
	}
	CWnd::OnTimer(nIDEvent);
}




//	EOF


