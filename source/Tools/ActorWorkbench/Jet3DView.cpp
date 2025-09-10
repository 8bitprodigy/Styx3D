// Jet3DView.cpp : implementation file
//

#include "stdafx.h"
#include "ActorWorkbench.h"
#include "Jet3DView.h"
#include "ActorWorkbenchDoc.h"
#include ".\jet3dview.h"

#define TIMER_ID							1
#define TIMER_INTERVAL						40

// CJet3DView

IMPLEMENT_DYNCREATE(CJet3DView, CView)

CJet3DView::CJet3DView()
{
	m_pEngine = NULL;

	m_pCamera = NULL;
	m_pResMgr = NULL;

	m_pActorObject = NULL;
	m_pActor = NULL;
	m_pActorDef = NULL;

	m_pWorld = NULL;
	m_bInitialized = JE_FALSE;
}

CJet3DView::~CJet3DView()
{
	if (m_pActorObject)
	{
		if (m_pActorDef)
			jeActor_DefDestroy(&m_pActorDef);

		if (m_pActor)
			m_pActor = NULL;

		jeObject_DettachEngine(m_pActorObject, m_pEngine);
		jeObject_Destroy(&m_pActorObject);
	}

	if (m_pWorld)
		jeWorld_Destroy(&m_pWorld);

	if (m_pResMgr)
		jeResource_MgrDestroy(&m_pResMgr);

	if (m_pCamera)
		jeCamera_Destroy(&m_pCamera);

	if (m_pEngine)
		jeEngine_Destroy(&m_pEngine);
}

BEGIN_MESSAGE_MAP(CJet3DView, CView)
	ON_WM_TIMER()
END_MESSAGE_MAP()


// CJet3DView drawing

void CJet3DView::OnDraw(CDC* pDC)
{
	CDocument* pDoc = GetDocument();
	if (m_bInitialized)
	{
		// TODO: add draw code here
		if (!jeEngine_BeginFrame(m_pEngine, m_pCamera, JE_TRUE))
			return;

		if (m_pActor)
		{
			jeXForm3d				temp;

			temp = m_ActorXForm;
			jeXForm3d_PostRotateX(&temp, JE_HALFPI);
			jeXForm3d_PostRotateY(&temp, -JE_PI);

			jeActor_ClearPose(m_pActor, &temp);
			jeActor_Render(m_pActor, m_pEngine, m_pWorld, m_pCamera);
		}

		if (!jeEngine_EndFrame(m_pEngine))
			return;
	}
}


// CJet3DView diagnostics

#ifdef _DEBUG
void CJet3DView::AssertValid() const
{
	CView::AssertValid();
}

void CJet3DView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}
#endif //_DEBUG


// CJet3DView message handlers

void CJet3DView::OnInitialUpdate()
{
	CView::OnInitialUpdate();

	// TODO: Add your specialized code here and/or call the base class
	m_pEngine = jeEngine_Create(this->GetSafeHwnd(), "ActorWorkbench", ".");
	if (!m_pEngine)
	{
		TRACE0("Could not create engine object!!");
		return;
	}

	jeEngine_EnableFrameRateCounter(m_pEngine, JE_FALSE);
	jeEngine_RegisterDriver(m_pEngine, jeEngine_D3DDriver());
	jeEngine_RegisterObjects("Objects");
	//jeEngine_SetGamma(m_pEngine, 1.0f);	//trilobite orig
	jeEngine_SetGamma(m_pEngine, 1.5f);	//trilobite revise


	jeDriver_System *DrvSys = jeEngine_GetDriverSystem(m_pEngine);
	if (!DrvSys)
	{
		TRACE0("Could not get driver system!!");
		return;
	}

	//	by trilobite	Jan. 2011
	jeDriver *Driver = NULL;
	//for (jeDriver *Driver = jeDriver_SystemGetNextDriver(DrvSys, NULL); Driver != NULL; Driver = jeDriver_SystemGetNextDriver(DrvSys, Driver))
	for (Driver = jeDriver_SystemGetNextDriver(DrvSys, NULL); Driver != NULL; Driver = jeDriver_SystemGetNextDriver(DrvSys, Driver))
	//	
	{
		const char					*drvname = NULL;

		jeDriver_GetName(Driver, &drvname);
		//	by trilobite jan. 2011 -- reverting to engine's native (D3D) driver
		//if (drvname[0] == 'D')
		if (drvname[0] == '(')
			break;
	}

	if (Driver == NULL)
	{
		TRACE0("Could not find a valid driver!!");
		return;
	}

	//	by trilobite	Jan. 2011
	jeDriver_Mode *Mode = NULL;
	//for (jeDriver_Mode *Mode = jeDriver_GetNextMode(Driver, NULL); Mode != NULL; Mode = jeDriver_GetNextMode(Driver, Mode))
	for (Mode = jeDriver_GetNextMode(Driver, NULL); Mode != NULL; Mode = jeDriver_GetNextMode(Driver, Mode))
	//	
	{
		int32					w, h, b;

		jeDriver_ModeGetAttributes(Mode, &w, &h, &b);
		if (w == -1 && h == -1 && b == -1)
			break;
	}

	if (Mode == NULL)
	{
		TRACE0("Driver does not support windowed mode!!");
		return;
	}

	if (!jeEngine_SetDriverAndMode(m_pEngine, this->GetSafeHwnd(), Driver, Mode))
	{
		TRACE0("Could not start engine!!");
		return;
	}

	jeEngine_SetRenderMode(m_pEngine, RenderMode_TexturedAndLit);

	m_pResMgr = jeResource_MgrCreateDefault(m_pEngine);
	if (!m_pResMgr)
	{
		TRACE0("Could not create resource manager!!");
		return;
	}

	if (!InitWorld())
		return;
    
	this->SetTimer(TIMER_ID, TIMER_INTERVAL, NULL);
	m_bInitialized = JE_TRUE;
}

void CJet3DView::OnTimer(UINT_PTR nIDEvent)
{
	CView::OnTimer(nIDEvent);
	if (nIDEvent == TIMER_ID)
	{
		if (m_bInitialized)
			Invalidate(FALSE);
		//this->SetTimer(TIMER_ID, TIMER_INTERVAL, NULL);
	}
}

jeEngine * CJet3DView::GetEngine(void)
{
	return m_pEngine;
}

void CJet3DView::SetActiveActor(jeObject * Object)
{
	if (m_pActorObject)
	{
		if (m_pActorDef)
			jeActor_DefDestroy(&m_pActorDef);

		if (m_pActor)
			m_pActor = NULL;

		jeObject_DettachEngine(m_pActorObject, m_pEngine);
		jeObject_Destroy(&m_pActorObject);

		m_pActorDef = NULL;
		m_pActorObject = NULL;
	}

	m_pActorObject = Object;
	m_pActor = (jeActor*)jeObject_GetInstance(m_pActorObject);
	m_pActorDef = jeActor_GetActorDef(m_pActor);

	//jeActor_SetScale(m_pActor, 0.5f, 0.5f, 0.5f);

	//jeXForm3d_SetIdentity(&m_ActorXForm);
	jeVec3d					Pos;

	jeActor_GetXForm(m_pActor, &m_ActorXForm);
	jeVec3d_Copy(&m_ActorXForm.Translation, &Pos);
	jeVec3d_Set(&m_ActorXForm.Translation, 0.0f, 0.0f, 0.0f);
	jeXForm3d_PostRotateX(&m_ActorXForm, JE_HALFPI);
	jeXForm3d_PostRotateY(&m_ActorXForm, -JE_PI);
	jeVec3d_Copy(&Pos, &m_ActorXForm.Translation);
	jeActor_SetXForm(m_pActor, &m_ActorXForm);

	jeVec3d					in;

	m_CameraXForm = m_ActorXForm;
	jeXForm3d_GetIn(&m_CameraXForm, &in);
	jeVec3d_MA(&m_CameraXForm.Translation, -50.0f, &in, &m_CameraXForm.Translation);
	
	if (jeActor_GetMotionCount(m_pActorDef) > 0)
		m_CameraXForm.Translation.Y += 50.0f;

	jeCamera_SetXForm(m_pCamera, &m_CameraXForm);

	jeActor_AttachEngine(m_pActor, m_pEngine);
	//jeActor_ClearPose(m_pActor, &m_ActorXForm);

	jeVec3d FillLightNormal;

	jeVec3d_Set( &FillLightNormal, -0.3f, 1.0f, 0.4f );
	jeVec3d_Normalize( &FillLightNormal );

	jeActor_SetLightingOptions( m_pActor, JE_TRUE, &FillLightNormal,
		512.0f, 512.0f, 512.0f,		// Fill light
		512.0f, 512.0f, 512.0f,		// Ambient light
		JE_TRUE,					// Ambient light from floor
		0,		// no dynamic lights,
		0,
		NULL, FALSE );

	//jeWorld_AddObject(m_pWorld, m_pActorObject);
}

jeBoolean CJet3DView::InitWorld(void)
{
	RECT							r;

	if (!m_pResMgr || !m_pEngine)
		return JE_FALSE;

	m_pWorld = jeWorld_Create(m_pResMgr);
	if (!m_pWorld)
	{
		AfxMessageBox("Could not create world!!", 48, 0);
		return JE_FALSE;
	}

	jeWorld_SetEngine(m_pWorld, m_pEngine);

	m_FOV = jeFloat_DegToRad(90.0f);

	this->GetClientRect(&r);
	m_CameraRect.Top = r.top;
	m_CameraRect.Bottom = r.bottom - 1;
	m_CameraRect.Left = r.left;
	m_CameraRect.Right = r.right - 1;

	m_pCamera = jeCamera_Create(m_FOV, &m_CameraRect);
	if (!m_pCamera)
	{
		AfxMessageBox("Could not create camera!!");
		return JE_FALSE;
	}

	jeXForm3d_SetIdentity(&m_CameraXForm);
	jeXForm3d_SetTranslation(&m_CameraXForm, 0.0f, 25.0f, 50.0f);

	jeCamera_SetXForm(m_pCamera, &m_CameraXForm);

	return JE_TRUE;
}
