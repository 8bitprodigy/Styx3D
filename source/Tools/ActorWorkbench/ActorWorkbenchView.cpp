// ActorWorkbenchView.cpp : implementation of the CActorWorkbenchView class
//

#include "stdafx.h"
#include "ActorWorkbench.h"

#include "ActorWorkbenchDoc.h"
#include "ActorWorkbenchView.h"
#include ".\actorworkbenchview.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CActorWorkbenchView

IMPLEMENT_DYNCREATE(CActorWorkbenchView, CView)

BEGIN_MESSAGE_MAP(CActorWorkbenchView, CView)
END_MESSAGE_MAP()

// CActorWorkbenchView construction/destruction

CActorWorkbenchView::CActorWorkbenchView()
{
	// TODO: add construction code here

}

CActorWorkbenchView::~CActorWorkbenchView()
{
}

BOOL CActorWorkbenchView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CView::PreCreateWindow(cs);
}

// CActorWorkbenchView drawing

void CActorWorkbenchView::OnDraw(CDC* /*pDC*/)
{
	CActorWorkbenchDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	// TODO: add draw code for native data here
}


// CActorWorkbenchView diagnostics

#ifdef _DEBUG
void CActorWorkbenchView::AssertValid() const
{
	CView::AssertValid();
}

void CActorWorkbenchView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CActorWorkbenchDoc* CActorWorkbenchView::GetDocument() const // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CActorWorkbenchDoc)));
	return (CActorWorkbenchDoc*)m_pDocument;
}
#endif //_DEBUG


// CActorWorkbenchView message handlers
