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



// ChildView.h : interface of the CJetView class
//

#include "Jet.h"


#pragma once


// CJetView window

class CJetView : public CWnd
{
// Construction
public:
	CJetView();

// Overrides
	protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

//	destruction
public:
	virtual ~CJetView();

//	Generated message map functions
protected:
	afx_msg void OnPaint();
	DECLARE_MESSAGE_MAP()
public:
	bool	StartTimer();
	bool	StopTimer();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);



//	Implementation
private:
	bool		m_bReadyToRender;		//	render gatekeeper flag
	LARGE_INTEGER	m_LIOldTick, m_LIFreq;	//	time-calc vars
		CRect		m_rectMainView;
			JE_Rect			m_Rect;				//	camera rect
//	message handlers -- called by the MFC framewowrk
public:
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnDestroy();
	afx_msg void OnClose();

};

