#pragma once


// CJet3DView view
#include "Jet.h"

class CJet3DView : public CView
{
	DECLARE_DYNCREATE(CJet3DView)

protected:
	CJet3DView();           // protected constructor used by dynamic creation
	virtual ~CJet3DView();

public:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	jeBoolean							m_bInitialized;

	jeEngine							*m_pEngine;
	jeCamera							*m_pCamera;

	jeObject							*m_pActorObject;
	jeActor_Def							*m_pActorDef;
	jeActor								*m_pActor;
	jeXForm3d							m_ActorXForm;

	jeRect								m_CameraRect;
	jeFloat								m_FOV;
	jeXForm3d							m_CameraXForm;

	jeWorld								*m_pWorld;
	jeResourceMgr						*m_pResMgr;

protected:
	DECLARE_MESSAGE_MAP()
public:
	virtual void OnTimer(UINT_PTR nIDEvent);
	virtual void OnInitialUpdate();
	jeEngine * GetEngine(void);
	void SetActiveActor(jeObject * Actor);
	jeBoolean InitWorld(void);
};


