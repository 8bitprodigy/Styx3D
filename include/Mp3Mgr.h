//jeMp3Mgr - CyRiuS

#ifndef Mp3Mgr_H
#define Mp3Mgr_H

#include "jet.h"
#include <windows.h>
#include <string.h>


#ifdef __cplusplus
extern "C" {
#endif

#define MAX_MP3			1024
#define MP3_LOAD_SUCCESS	0
#define MP3_LOAD_FAIL	1
	

typedef struct jeMp3_Def_
{
	LPSTR			szFileName;
	jeBoolean		isCutList; //Not yet implemented
	// add other stuff here later
} jeMp3_Def;
	
typedef struct //the jeMp3Mgr holds info about the vids you wanna play. files[] allows easy access
{			   //to all of your vids
	int				num_mp3s;
	int				cur_mp3;
	jeMp3_Def		files[MAX_MP3];
	HWND			mwh;

} jeMp3Mgr;

JETAPI jeMp3Mgr * JETCC jeMp3_CreateManager(HWND mainwindowhandle);
JETAPI jeBoolean JETCC jeMp3_DestroyManager(jeMp3Mgr **Mp3Mgr);

#ifdef __cplusplus
}
#endif

#endif

