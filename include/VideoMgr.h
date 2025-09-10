//jeVidMgr - CyRiuS

#ifndef VideoMgr_H
#define VideoMgr_H

#include "jet.h"
#include <windows.h>
#include <string.h>


#ifdef __cplusplus
extern "C" {
#endif

#define MAX_VIDS	35
#define JEMSG_VIDEO_NOTIFY  WM_USER+13
	
	

typedef struct jeVideo_Def_
{
	LPSTR szFileName;
	// add other stuff here later
} jeVideo_Def;
	
typedef struct //the jeVidMgr holds info about the vids you wanna play. files[] allows easy access
{			   //to all of your vids
	int				numvids;
	int				curvid;
	jeVideo_Def		files[MAX_VIDS];
	HWND			mwh;

} jeVidMgr;

JETAPI jeVidMgr * JETCC jeVideo_CreateManager(HWND mainwindowhandle);
JETAPI jeBoolean JETCC jeVideo_DestroyManager(jeVidMgr **VideoMgr);
JETAPI void JETCC jeVideo_Notify();

JETAPI void JETCC jeVideo_Open(jeVidMgr *VidMgr, LPSTR szFile );
JETAPI void JETCC jeVideo_Play (jeVidMgr *VidMgr, int vid);
JETAPI jeBoolean JETCC jeVideo_IsPlaying();

#ifdef __cplusplus
}
#endif

#endif

