//jeVidMgr - CyRiuS

#ifndef VideoMgrH_H
#define VideoMgrH_H

#include "jet.h"
#include "VideoMgr.h"
#include <windows.h>
#include <string.h>
#include <evcode.h>
#include <objbase.h>
#include <strmif.h>
#include <control.h>
#include <uuids.h>
#include "Ram.h"
#include "Errorlog.h"
#include <direct.h>	

//nclude "Mp3Mgr.h"


#ifdef __cplusplus
extern "C" {
#endif

//	DEFINES	AND DECLARATIONS ============================================
#ifdef __cplusplus
#define HELPER_RELEASE(x) { if (x) x->Release(); x = NULL; }
#else
#define HELPER_RELEASE(x) { if (x) x->lpVtbl->Release(x); x = NULL; }
#endif

typedef enum tagVIDEOMEDIA {UninitializedV, StoppedV, PausedV, PlayingV } VideoState;

typedef struct tagVIDEOMEDIASTATE
{
    VideoState				state;
	// Collection of interfaces
	IBaseFilter   *pif;
	IMediaControl *pimc;
	IMediaEventEx *pimex;
	IVideoWindow  *pivw;
	IGraphBuilder		*pGraph;


    HANDLE				hGraphNotifyEvent;
} VIDEOMEDIA;


//	FUNCTION PROTOTYPES	=================================================

jeBoolean	InitVideoMgr(HWND mainwindowhandle);
void		UnInitVideoMgr(HWND mainwindowhandle);
jeBoolean	InitVideoMedia(HWND mainwindowhandle);
void		OpenVideoFile(HWND mainwindowhandle, LPSTR szFile );
HANDLE		GetGraphEventHandleVideo( void );
void		OpenVideoFile( HWND hwnd, LPSTR szFile );


BOOL	CanPlayVideo();
BOOL	CanStopVideo();
BOOL	CanPauseVideo();
BOOL	IsInitializedVideo();
void	DeleteContentsVideo();

// Event handlers
void		PlayVideo();
void		OnVideoPause();
void		StopVideo();
void		OnVideoAbortStop();
void		OnGraphNotifyVideo();
jeBoolean	VideoPlaying();

#ifdef __cplusplus
}
#endif

#endif

