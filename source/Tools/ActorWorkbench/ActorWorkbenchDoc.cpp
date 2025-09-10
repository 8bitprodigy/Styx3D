// ActorWorkbenchDoc.cpp : implementation of the CActorWorkbenchDoc class
//

#include "stdafx.h"
#include "ActorWorkbench.h"

#include "ActorWorkbenchDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CActorWorkbenchDoc

IMPLEMENT_DYNCREATE(CActorWorkbenchDoc, CDocument)

BEGIN_MESSAGE_MAP(CActorWorkbenchDoc, CDocument)
END_MESSAGE_MAP()


// CActorWorkbenchDoc construction/destruction

CActorWorkbenchDoc::CActorWorkbenchDoc()
{
	// TODO: add one-time construction code here

}

CActorWorkbenchDoc::~CActorWorkbenchDoc()
{
}

BOOL CActorWorkbenchDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)

	return TRUE;
}




// CActorWorkbenchDoc serialization

void CActorWorkbenchDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: add storing code here
	}
	else
	{
		// TODO: add loading code here
	}
}


// CActorWorkbenchDoc diagnostics

#ifdef _DEBUG
void CActorWorkbenchDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CActorWorkbenchDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG


// CActorWorkbenchDoc commands
