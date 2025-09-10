// ActorWorkbenchView.h : interface of the CActorWorkbenchView class
//


#pragma once
#include "ActorWorkbenchDoc.h"

class CActorWorkbenchView : public CView
{
protected: // create from serialization only
	CActorWorkbenchView();
	DECLARE_DYNCREATE(CActorWorkbenchView)

// Attributes
public:
	CActorWorkbenchDoc* GetDocument() const;

// Operations
public:

// Overrides
	public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:

// Implementation
public:
	virtual ~CActorWorkbenchView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // debug version in ActorWorkbenchView.cpp
inline CActorWorkbenchDoc* CActorWorkbenchView::GetDocument() const
   { return reinterpret_cast<CActorWorkbenchDoc*>(m_pDocument); }
#endif

