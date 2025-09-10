#include <windows.h>
#include <assert.h>
#include "D3D9Log.h"

D3D9Log									*D3D9Log::Singleton = NULL;

D3D9Log *D3D9Log::GetPtr()
{
	if (D3D9Log::Singleton == NULL)
		D3D9Log::Singleton = new D3D9Log();

	return D3D9Log::Singleton;
}

D3D9Log::D3D9Log()
{
	m_pFile = NULL;
	Initialize();
}

D3D9Log::~D3D9Log()
{
	if (m_pFile)
		fclose(m_pFile);

	m_pFile = NULL;
}

bool D3D9Log::Initialize()
{
	assert(m_pFile == NULL);

	m_pFile = fopen(LOG_FILE_NAME, "wt");
	if (!m_pFile)
		return false;

	fprintf(m_pFile, "Direct3D 9 Driver\n");
	fprintf(m_pFile, "Copyright 2006, Paradox Software\n");

	fprintf(m_pFile, "\nLogging started...\n");
	fclose(m_pFile);
	m_pFile = NULL;

	return true;
}

void D3D9Log::Shutdown()
{
	Printf("\nLogging ended...");
	delete this;
}

void D3D9Log::Printf(const char *fmt, ...)
{
	va_list						list;
	char						buffer[1024];

	assert(m_pFile == NULL);

	va_start(list, fmt);
	vsprintf(buffer, fmt, list);
	va_end(list);

	m_pFile = fopen(LOG_FILE_NAME, "at");
	assert(m_pFile != NULL);

	fprintf(m_pFile, "%s\n", buffer);
	fflush(m_pFile);
	fclose(m_pFile);
	m_pFile = NULL;

#ifdef _DEBUG
	OutputDebugString(buffer);
#endif
}
