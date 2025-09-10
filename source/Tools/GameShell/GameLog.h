/*!
	@file GameLog.h
	@author Anthony Rufrano (paradoxnj)
	@brief Logging class
*/
#ifndef GAMELOG_H
#define GAMELOG_H

#include <stdio.h>

class CGameLog;

#define GLOG(x)					CGameLog::GetPtr()->Printf(x)

void							InitializeGameLog(const char *filename);

class CGameLog
{
public:
	CGameLog(const char *filename);
	virtual ~CGameLog();

private:
	FILE						*m_pLog;
	char						m_FileName[256];

public:
	void						Printf(const char *fmt, ...);

public:
	static CGameLog				*Singleton;
	static CGameLog				*GetPtr();
};

#endif
