#ifndef CD_H
#define CD_H

#ifdef _WIN32
	#include <windows.H>
#else
	#include <stdint.h>
	typedef uint32_t UINT;
	typedef uint32_t DWORD;
	typedef int32_t INT;
	typedef uint8_t BYTE;
	typedef void* HWND;
#endif

#include "BaseType.h"

#ifdef __cplusplus
extern "C" {
#endif

UINT      OpenCDPlayer( void);
void      StopCDPlayer( UINT id);
void      CloseCDPlayer(UINT id);
geBoolean GetCDPosition(UINT Id, INT *Track, INT *Minute, INT *Second);

DWORD	PlayCDTrack(UINT wDeviceID,
					HWND hWndNotify,
					BYTE bTrack,
					int  startMin,
					int  startSec,
					int	 stopMin,
					int	 stopSec);

#ifndef SINGLE_BUILD_FOR_TEST
void TestCd(void);
#endif

#ifdef __cplusplus
}
#endif

#endif
