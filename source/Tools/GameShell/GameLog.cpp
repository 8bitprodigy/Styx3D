#include <windows.h>
#include "GameLog.h"

CGameLog								*CGameLog::Singleton = NULL;

void InitializeGameLog(const char *filename)
{
	CGameLog::Singleton = new CGameLog(filename);
}

CGameLog *CGameLog::GetPtr()
{
	return CGameLog::Singleton;
}

CGameLog::CGameLog(const char *filename)
{
	m_pLog = NULL;

	if (filename == NULL)
	{
		m_pLog = fopen("JSE.log", "wt");
		strcpy(m_FileName, "JSE.log");
	}
	else
	{
		m_pLog = fopen(filename, "wt");
		strcpy(m_FileName, filename);
	}

	fprintf(m_pLog, "Jet3D Game Shell\n");
	fprintf(m_pLog, "Copyright 2006, Paradox Software\n\n");
	fflush(m_pLog);

	fclose(m_pLog);
}

CGameLog::~CGameLog()
{
	Printf("Logging Ended...");
}

void CGameLog::Printf(const char *fmt, ...)
{
	va_list						list;
	char						buffer[1024];

	va_start(list, fmt);
	vsprintf(buffer, fmt, list);
	va_end(list);

	m_pLog = fopen(m_FileName, "at");
	if (!m_pLog)
		return;

	fprintf(m_pLog, "%s\n", buffer);
	fclose(m_pLog);
	m_pLog = NULL;
}
