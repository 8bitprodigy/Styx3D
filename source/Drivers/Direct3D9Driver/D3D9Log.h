/*!
	@file D3D9Log.h
	@author Anthony Rufrano (paradoxnj)
	@brief Logging class
*/
#ifndef D3D_LOG_H
#define D3D_LOG_H

#include <stdio.h>
#include "DCommon.h"

#define LOG_FILE_NAME						"d3d9driver.log"

class D3D9Log;

class D3D9Log
{
public:
	D3D9Log();
	virtual ~D3D9Log();

private:
	FILE								*m_pFile;

public:
	bool								Initialize();
	void								Shutdown();

	void								Printf(const char *fmt, ...);

public:
	static D3D9Log						*Singleton;
	static D3D9Log						*GetPtr();
};

#endif