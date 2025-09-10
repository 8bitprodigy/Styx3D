#include <assert.h>
#include "GameMgr.h"
#include "GameLog.h"
#include "ScriptMgr.h"

CGameMgr									*CGameMgr::Singleton = NULL;

CGameMgr * CGameMgr::GetPtr()
{
	if (CGameMgr::Singleton == NULL)
		CGameMgr::Singleton = new CGameMgr();

	return CGameMgr::Singleton;
}

CGameMgr::CGameMgr()
{
	m_pEngine = NULL;
	m_pCamera = NULL;
	m_pWorld = NULL;

	m_pPtrMgr = NULL;
	m_pResMgr = NULL;

	m_hWnd = NULL;
	m_Width = m_Height = m_BPP = -1;
	strcpy(m_DriverName, "Direct3D9");

	this->register_func("EnableFrameRateCounter", cpp_method(this, CGameMgr, EOSEnableFrameRateCounter));
	this->register_func("SetGamma", cpp_method(this, CGameMgr, EOSSetGamma));
	this->register_func("SetDriver", cpp_method(this, CGameMgr, EOSSetDriver));
	this->register_func("SetDriverMode", cpp_method(this, CGameMgr, EOSSetDriverMode));
	this->register_func("LoadWorld", cpp_method(this, CGameMgr, EOSLoadWorld));
}

CGameMgr::~CGameMgr()
{
	Shutdown();
}

bool CGameMgr::Release()
{
	GLOG("CGameMgr - Releasing memory...");

	delete this;
	return true;
}

jeBoolean CGameMgr::Initialize(HWND hWnd)
{
	GLOG("CGameMgr - Initializing game manager...");

	m_pEngine = jeEngine_Create(hWnd, "JetShell", ".");
	if (!m_pEngine)
	{
		GLOG("CGameMgr - Could not create engine!!");
		return JE_FALSE;
	}

	jeEngine_RegisterDriver(m_pEngine, jeEngine_D3DDriver());

	m_hWnd = hWnd;

	//CScriptMgr::GetPtr()->SetGlobal("Game", (eosobject*)this);

	GLOG("CGameMgr - Loading main script...");
	CScriptMgr::GetPtr()->LoadScript(".\\Scripts\\JetMain.eos");
	CScriptMgr::GetPtr()->Call("JetMain");

	return JE_TRUE;
}

void CGameMgr::Shutdown()
{
	GLOG("CGameMgr - Shutting down game manager...");

	if (m_pWorld)
		jeWorld_Destroy(&m_pWorld);

	if (m_pCamera)
		jeCamera_Destroy(&m_pCamera);

	if (m_pResMgr)
		jeResource_MgrDestroy(&m_pResMgr);

	if (m_pPtrMgr)
		jePtrMgr_Destroy(&m_pPtrMgr);

	if (m_pEngine)
		jeEngine_Destroy(&m_pEngine);

	m_pEngine = NULL;
}

void CGameMgr::SetDriver(const char *drivername)
{
	CGameLog::GetPtr()->Printf("CGameMgr - Setting driver to %s...", drivername);
	strcpy(m_DriverName, drivername);
}

jeBoolean CGameMgr::SetDriverMode(int32 w, int32 h, int32 b)
{
	jeDriver_System						*DrvSys = NULL;
	jeDriver							*Driver = NULL;
	jeDriver_Mode						*Mode = NULL;
	int32								width, height, bpp;
	char								drv;

	GLOG("CGameMgr - Preparing video mode...");
	assert(m_pEngine != NULL);

	if (w == 0)
		w = -1;

	if (h == 0)
		h = -1;

	if (b == 0)
		b = -1;

	DrvSys = jeEngine_GetDriverSystem(m_pEngine);
	if (!DrvSys)
	{
		GLOG("CGameMgr - Could not get driver system!!");
		return JE_FALSE;
	}

	if (!strcmp(m_DriverName, "Direct3D9"))
		drv = 'D';
	else if (!strcmp(m_DriverName, "OpenGL"))
		drv = 'O';
	else if (!strcmp(m_DriverName, "Direct3D7"))
		drv = '(';

	for (Driver = jeDriver_SystemGetNextDriver(DrvSys, NULL); Driver != NULL; Driver = jeDriver_SystemGetNextDriver(DrvSys, Driver))
	{
		const char						*drvname = NULL;

		jeDriver_GetName(Driver, &drvname);
		if (drvname[0] == drv)
			break;
	}

	if (Driver == NULL)
	{
		GLOG("Could not find requested driver!!");
		DrvSys = NULL;
		return JE_FALSE;
	}

	for (Mode = jeDriver_GetNextMode(Driver, NULL); Mode != NULL; Mode = jeDriver_GetNextMode(Driver, Mode))
	{
		jeDriver_ModeGetAttributes(Mode, &width, &height, &bpp);
		if (width == w && height == h && bpp == b)
			break;
	}

	if (Mode == NULL)
	{
		GLOG("Could not find a valid video mode!!");
		Driver = NULL;
		DrvSys = NULL;
		return JE_FALSE;
	}

	if (!jeEngine_SetDriverAndMode(m_pEngine, m_hWnd, Driver, Mode))
	{
		GLOG("Could not activate engine!!");
		Mode = NULL;
		Driver = NULL;
		DrvSys = NULL;
		return JE_FALSE;
	}

	jeEngine_RegisterObjects("Objects");

	jeRect						r;

	r.Left = 0;
	r.Top = 0;
	r.Bottom = 800 - 1;
	r.Right = 600 - 1;

	m_pCamera = jeCamera_Create(2.0f, &r);

	jeXForm3d_SetIdentity(&m_CameraXForm);
	jeCamera_SetXForm(m_pCamera, &m_CameraXForm);

	m_pPtrMgr = jePtrMgr_Create();
	if (!m_pPtrMgr)
		return JE_FALSE;

	m_pResMgr = jeResource_MgrCreateDefault(m_pEngine);
	if (!m_pResMgr)
		return JE_FALSE;

	m_LastTime = timeGetTime();

	return JE_TRUE;
}

jeBoolean CGameMgr::LoadWorld(const char *filename)
{
	jeVFile						*File = NULL;

	if (!m_pPtrMgr || !m_pResMgr)
		return JE_FALSE;

	m_pWorld = jeWorld_CreateFromEditorFile(filename, m_pPtrMgr, m_pResMgr);
	if (!m_pWorld)
		return JE_FALSE;

	jeWorld_SetEngine(m_pWorld, m_pEngine);

	jeWorld_RebuildBSP(m_pWorld, BSP_OPTIONS_CSG_BRUSHES, Logic_Smart, 5);
	jeWorld_RebuildLights(m_pWorld);

	return JE_TRUE;
}

jeBoolean CGameMgr::Frame()
{
	DWORD						currTime;
	float						deltaTime;

	assert(m_pEngine != NULL);

	currTime = timeGetTime();
	deltaTime = ((float)(currTime - m_LastTime)) * 0.001f;

	if (!jeEngine_BeginFrame(m_pEngine, m_pCamera, JE_TRUE))
		return JE_FALSE;

	if (m_pWorld)
	{
		jeWorld_Frame(m_pWorld, deltaTime);
		jeWorld_Render(m_pWorld, m_pCamera, NULL);
	}

	if (!jeEngine_EndFrame(m_pEngine))
		return JE_FALSE;

	m_LastTime = currTime;

	return JE_TRUE;
}

void CGameMgr::EOSEnableFrameRateCounter()
{
	jeBoolean					enable;

	enable = (jeBoolean)exe->pop()->geti(0);
	jeEngine_EnableFrameRateCounter(m_pEngine, enable);
}

void CGameMgr::EOSSetGamma()
{
	float						gamma;

	gamma = exe->pop()->getd(0);
	jeEngine_SetGamma(m_pEngine, gamma);
}

void CGameMgr::EOSSetDriver()
{
	std::string					name;

	name = exe->pop()->gets(0);
	SetDriver(name.c_str());
}

void CGameMgr::EOSSetDriverMode()
{
	int32						w, h, b;

	w = exe->pop()->geti(0);
	h = exe->pop()->geti(0);
	b = exe->pop()->geti(0);

	SetDriverMode(w, h, b);
}

void CGameMgr::EOSLoadWorld()
{
	std::string					filename;

	filename = exe->pop()->gets(0);
	LoadWorld(filename.c_str());
}
