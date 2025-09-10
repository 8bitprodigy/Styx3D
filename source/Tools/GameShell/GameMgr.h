/*!
	@file GameMgr.h
	@author Anthony Rufrano (paradoxnj)
	@brief The game manager
*/
#ifndef GAMEMGR_H
#define GAMEMGR_H

#include <windows.h>
#include "jet.h"
#include "eosscript.h"

#define MAX_DRIVER_NAME				64

class CGameMgr;

class CGameMgr : public eosobject
{
public:
	CGameMgr();
	virtual ~CGameMgr();

public:
	jeEngine						*m_pEngine;
	HWND							m_hWnd;

	jePtrMgr						*m_pPtrMgr;
	jeResourceMgr					*m_pResMgr;

	char							m_DriverName[MAX_DRIVER_NAME];
	int32							m_Width, m_Height, m_BPP;

	jeWorld							*m_pWorld;

	jeCamera						*m_pCamera;
	jeXForm3d						m_CameraXForm;

	DWORD							m_LastTime;

public:
	bool							Release();

	jeBoolean						Initialize(HWND hWnd);
	void							Shutdown();

	void							SetDriver(const char *drivername);
	jeBoolean						SetDriverMode(int32 w, int32 h, int32 b);

	jeBoolean						LoadWorld(const char *filename);

	jeBoolean						Frame();

public:
	static CGameMgr					*Singleton;
	static CGameMgr					*GetPtr();

public:
	// Script exports
	void							EOSSetDriver();
	void							EOSSetDriverMode();

	void							EOSEnableFrameRateCounter();
	void							EOSSetGamma();

	void							EOSLoadWorld();
};

#endif