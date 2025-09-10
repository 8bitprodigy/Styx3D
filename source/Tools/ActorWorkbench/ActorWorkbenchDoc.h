// ActorWorkbenchDoc.h : interface of the CActorWorkbenchDoc class
//


#pragma once

class CActorWorkbenchDoc : public CDocument
{
protected: // create from serialization only
	CActorWorkbenchDoc();
	DECLARE_DYNCREATE(CActorWorkbenchDoc)

// Attributes
public:

// Operations
public:

// Overrides
	public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);

// Implementation
public:
	virtual ~CActorWorkbenchDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()
};


