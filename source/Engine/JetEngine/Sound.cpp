/****************************************************************************************/
/*  SOUND.C                                                                             */
/*                                                                                      */
/*  Author:                                                                             */
/*  Description:                                                                        */
/*                                                                                      */
/*  The contents of this file are subject to the Jet3D Public License                   */
/*  Version 1.02 (the "License"); you may not use this file except in                   */
/*  compliance with the License. You may obtain a copy of the License at                */
/*  http://www.jet3d.com                                                                */
/*                                                                                      */
/*  Software distributed under the License is distributed on an "AS IS"                 */
/*  basis, WITHOUT WARRANTY OF ANY KIND, either express or implied.  See                */
/*  the License for the specific language governing rights and limitations              */
/*  under the License.                                                                  */
/*                                                                                      */
/*  The Original Code is Jet3D, released December 12, 1999.                             */
/*  Copyright (C) 1996-1999 Eclipse Entertainment, L.L.C. All Rights Reserved           */
/*                                                                                      */
/****************************************************************************************/
#include	<windows.h>
#include	<dsound.h>
#include	<stdio.h>
#include	<assert.h>

#include	"BaseType.h"
#include	"ErrorLog.h"
#include	"VFile.h"
#include	"Sound.h"
#include	"Mp3Mgr.h" //cyrius
#include	"Mp3Mgr_h.h"
#include	"Ram.h"

// BEGIN - Ogg Streamer - paradoxnj 4/17/2005
#include	"OGGStream.h"
#include	"jeChain.h"

#define STREAM_BUFFER_SECONDS				1.0f

typedef struct	StreamChannel	StreamChannel;
// END - Ogg Streamer - paradoxnj 4/17/2005

typedef struct	SoundManager	SoundManager;
typedef struct  Channel			Channel;

// BEGIN - Upgrade to DirectSound 8 - paradoxnj 4/15/2005
#pragma comment(lib, "dsound.lib")
// END - Upgrade to DirectSound 8 - paradoxnj 4/15/2005

typedef struct jeSound_System
{
	jeBoolean		Active;
	SoundManager	*SoundM;
	jeFloat			GlobalVolume;
	jeMp3Mgr		*Mp3M;
} jeSound_System;

typedef struct jeSound_Cfg
{
	jeFloat			Volume;
	jeFloat			Pan;
	jeFloat			Frequency;
} jeSound_Cfg;


/*
	The interfaces here allow an application to write sound data to
	abstract channels which are then to be mixed.  The interfaces here
	require two things.  First, that the application create only one
	sound manager per instance, and second that the type of sound data
	being passed into the sound channels remains constant.  That is,
	the format of the binary information is all one format from
	one sound to another; the application cannot combine RIFF and WAV
	formats in a single channel.
*/
/*
	Call these ones only once per application:
*/

static SoundManager *	CreateSoundManager(HWND hWnd);
static void		DestroySoundManager(SoundManager *sm);

static BOOL		jeSound_FillSoundChannel(SoundManager *sm, jeVFile *File, unsigned int* Handle );
static BOOL		jeSound_StartSoundChannel( SoundManager *sm, unsigned int Handle, jeSound_Cfg *cfg, int loop, unsigned int* sfx);
static BOOL		jeSound_StopSoundChannel(Channel *channel);
static BOOL		jeSound_FreeAllChannels(SoundManager *sm);
static BOOL		jeSound_FreeChannel(SoundManager *sm, Channel *channel);
static BOOL		jeSound_ModifyChannel( Channel *channel, jeSound_Cfg *cfg );
static int		jeSound_ChannelPlaying( Channel *channel );
//	added by tom morris May 2005
static	DWORD	jeSound_ChannelGetBufferStatus( Channel *channel);
//	end add
static Channel*	jeSound_GetChannel( SoundManager *sm, unsigned int ID );

jeBoolean		OpenMediaFile(LPSTR szFile );
void			DeleteContentsMp3();
void			PlayMp3(long volume, jeBoolean loop); 
void			StopMp3();
jeBoolean		Mp3Playing();

// BEGIN - OGG Streamer - paradoxnj 4/17/2005
typedef struct StreamChannel
{
	LPDIRECTSOUNDBUFFER8	buffer;

	jeSound_Cfg				cfg;
	DSBCAPS					Caps;

	jeOGGStream				*OGG;
	jeBoolean				Looping;

	DWORD					LastReadPos;
	DWORD					BytesPlayed;
	DWORD					DataCursor;
} StreamChannel;

static StreamChannel		*CreateStreamChannel(jeVFile *FS, const char *filename);
static void					FreeStreamChannel(StreamChannel *ch);

static jeBoolean			PlayStream(StreamChannel *ch);
static jeBoolean			StopStream(StreamChannel *ch);
// END - OGG Streamer - paradoxnj 4/17/2005

typedef struct Channel
{
	LPDIRECTSOUNDBUFFER8	buffer;
	unsigned int			ID;
	int						BaseFreq;
	jeSound_Cfg				cfg;
	void *					Data;
	struct Channel			*next;
	struct Channel			*nextDup;
} Channel;

typedef struct	SoundManager
{
	int						smChannelCount;
	unsigned int			smNextChannelID;

	LPDIRECTSOUNDBUFFER 	smPrimaryChannel;
	Channel*				smChannels;

	// BEGIN - OGG Streamer - paradoxnj 4/17/2005
	HANDLE					Stream_Update_Thread;

	jeChain					*StreamList;
	jeChain					*StreamPlayList;

	CRITICAL_SECTION		UpdateSection;
	HANDLE					TermEvent;

	//	by trilobite jan. 2011
	//jeBoolean				IsInitialized;
	bool					IsInitialized;
	// END - OGG Streamer - paradoxnj 4/17/2005
}   SoundManager;


// BEGIN - OGG Streamer - paradoxnj 4/17/2005
static DWORD WINAPI StreamUpdateFunction(LPVOID Context);
static jeBoolean UpdateSoundBuffer(StreamChannel *stream);
static uint8 GetSilenceData(jeOGGStream *OGG);
static void FillBuffer(StreamChannel *sc);
// END - OGG Streamer - paradoxnj 4/17/2005

#pragma message ("move these globals into the sound system struct")
// BEGIN - Upgrade to DirectSound 8 - paradoxnj 4/14/2005
static	LPDIRECTSOUND8			lpDirectSound;

//static  HMODULE					hmodDirectSound = NULL;
// END - Upgrade to DirectSound 8 - paradoxnj 4/14/2005

//=====================================================================================
//	jeSound_SystemCreate
//=====================================================================================
JETAPI	jeSound_System * JETCC jeSound_CreateSoundSystem(HWND hWnd)
{
	//	by trilobite	Jan. 2011
	//	jeSound_System		*SoundSystem;
	jeSound_System		*SoundSystem = NULL;
	//

	SoundSystem = JE_RAM_ALLOCATE_STRUCT(jeSound_System);
	if (!SoundSystem)
	{
		jeErrorLog_Add(JE_ERR_MEMORY_RESOURCE, "jeSound_CreateSoundSystem.");
		return NULL;
	}

	memset(SoundSystem, 0, sizeof(jeSound_System));
	
	// Initialize the sound system
	SoundSystem->SoundM = CreateSoundManager(hWnd);
	if (!SoundSystem->SoundM)
	{
		jeRam_Free(SoundSystem);
		jeErrorLog_Add(JE_ERR_SUBSYSTEM_FAILURE, "jeSound_CreateSoundSystem:  Failed to create sound system.");
		return NULL;
	}
	
	//Mp3Mgr integration (CyRiuS)
	SoundSystem->Mp3M = jeMp3_CreateManager(hWnd);

	if (!SoundSystem->Mp3M)
	{
		jeRam_Free(SoundSystem);
		jeErrorLog_Add(JE_ERR_SUBSYSTEM_FAILURE, "jeSound_CreateSoundSystem:  Failed to create mp3 manager.");
		return NULL;
	}

	SoundSystem->GlobalVolume = 1.0f;

	return SoundSystem;
}

//=====================================================================================
//	jeSound_SetHwnd
//=====================================================================================
JETAPI	jeBoolean JETCC jeSound_SetHwnd(HWND hWnd)
{
	HRESULT Res;
	if (lpDirectSound)
		{
			//#pragma message ("uses global, and doesn't assert if it's bad.")
			assert( lpDirectSound );
			Res = IDirectSound_SetCooperativeLevel(lpDirectSound, hWnd,DSSCL_NORMAL);
			if (Res != DS_OK)
				{
					jeErrorLog_Add(JE_ERR_WINDOWS_API_FAILURE,"jeSound_SetHwnd:  IDirectSound_SetCooperativeLevel failed.");
					return JE_FALSE;
				}
		}
	return JE_TRUE;
}

//=====================================================================================
//	jeSound_SystemFree
//=====================================================================================
JETAPI	void JETCC jeSound_DestroySoundSystem(jeSound_System *Sound)
{
	assert(Sound != NULL);

	// Shutdown the sound system
	jeMp3_DestroyManager(&Sound->Mp3M); //cyrius
	DestroySoundManager(Sound->SoundM);

	Sound->SoundM = NULL;

	jeRam_Free(Sound);
}

//=====================================================================================
//	Sound_LoadSound
//=====================================================================================
//JETAPI	jeSound_Def *jeSound_LoadSoundDef(jeSound_System *SoundS, const char *Path, const char *FileName)
JETAPI	jeSound_Def * JETCC jeSound_LoadSoundDef(jeSound_System *SoundS, jeVFile *File)
{
	unsigned int SoundDef = 0;

	assert(SoundS != NULL);

	if (!jeSound_FillSoundChannel(SoundS->SoundM, File, &SoundDef))
		{
			jeErrorLog_Add(JE_ERR_SUBSYSTEM_FAILURE,"jeSound_LoadSoundDef.");
			return NULL;
		}
	
	return (jeSound_Def *)SoundDef;
}

//=====================================================================================
//	Mp3_LoadSound
//=====================================================================================
JETAPI int JETCC jeMp3_LoadSound(jeSound_System *SoundS, char * filename, int ref)
{
	assert(SoundS != NULL);

	SoundS->Mp3M->num_mp3s++;
	SoundS->Mp3M->cur_mp3 = ref;
	SoundS->Mp3M->files[ref].szFileName = filename;
	
	return MP3_LOAD_SUCCESS;
}


//=====================================================================================
//	Sound_FreeSound
//=====================================================================================
JETAPI	jeBoolean JETCC jeSound_FreeSoundDef(jeSound_System *SoundS, jeSound_Def *SoundDef)
{
	Channel*	Channel;

	assert(SoundS != NULL);
	assert(SoundDef != 0);

	Channel = jeSound_GetChannel(SoundS->SoundM, (unsigned int)SoundDef);

	if (!Channel)
		{
			jeErrorLog_Add(JE_ERR_SEARCH_FAILURE,"jeSound_FreeSoundDef:  Sound not found.");
			return JE_FALSE;
		}

	if (!jeSound_FreeChannel(SoundS->SoundM, Channel))
		{
			jeErrorLog_Add(JE_ERR_SUBSYSTEM_FAILURE,"jeSound_FreeSoundDef.");
			return JE_FALSE;
		}
	return JE_TRUE;
}

//=====================================================================================
//	Sound_SetGlobalVolume
//=====================================================================================
JETAPI	jeBoolean JETCC jeSound_SetMasterVolume( jeSound_System *SoundS, jeFloat Volume )
{
	assert ( SoundS );
	SoundS->GlobalVolume = Volume;
	return( JE_TRUE );
}
	
//=====================================================================================
//	Sound_PlaySound
//=====================================================================================
JETAPI	jeSound * JETCC jeSound_PlaySoundDef(jeSound_System *SoundS, 
							jeSound_Def *SoundDef, 
							jeFloat Volume, 
							jeFloat Pan, 
							jeFloat Frequency, 
							jeBoolean Loop)
{
	unsigned int Sound;
	jeSound_Cfg LocalCfg;

	LocalCfg.Volume		= Volume;
	LocalCfg.Pan		= Pan;
	LocalCfg.Frequency  = Frequency;

	LocalCfg.Volume *= SoundS->GlobalVolume;
	if (!jeSound_StartSoundChannel(SoundS->SoundM, (unsigned int)SoundDef, &LocalCfg, (BOOL)Loop, &Sound))
	{
		jeErrorLog_Add(JE_ERR_SUBSYSTEM_FAILURE,"jeSound_PlaySoundDef.");
		return NULL;
	}

	return (jeSound *)Sound;
}

//=====================================================================================
//	Mp3_PlaySound
//=====================================================================================
JETAPI	int JETCC jeMp3_PlaySound(jeSound_System *SoundS, int song_number, long Volume, jeBoolean Loop)
{
	assert(SoundS != NULL);
	
	if(!OpenMediaFile(SoundS->Mp3M->files[song_number].szFileName))
	{
		jeErrorLog_Add(JE_ERR_SUBSYSTEM_FAILURE, "jeMp3_PlaySound: cant load sound from disk");
		return MP3_LOAD_FAIL;
	}

	PlayMp3(Volume, Loop);

	return MP3_LOAD_SUCCESS;
}
	
//=====================================================================================
//	Sound_StopSound
//=====================================================================================
JETAPI	jeBoolean JETCC jeSound_StopSound(jeSound_System *SoundS, jeSound *Sound)
{
	Channel*	Channel;

	assert(SoundS != NULL);
	assert(Sound  != NULL);	

	Channel = jeSound_GetChannel(SoundS->SoundM, (unsigned int)Sound);

	if (!Channel)
		{
			jeErrorLog_Add(JE_ERR_SEARCH_FAILURE,"jeSound_StopSound:  Sound not playing.");
			return JE_FALSE;
		}

	if (jeSound_StopSoundChannel(Channel)==JE_FALSE)
		{
			jeErrorLog_Add(JE_ERR_SUBSYSTEM_FAILURE,"jeSound_StopSound:  Sound failed to stop.");
			return JE_FALSE;
		}
	return JE_TRUE;	
}

//=====================================================================================
//	Sound_ModifySound
//=====================================================================================
JETAPI	jeBoolean JETCC jeSound_ModifySound(jeSound_System *SoundS, 
								jeSound *Sound,jeFloat Volume, 
								jeFloat Pan, 
								jeFloat Frequency)
{
	Channel*	Channel;
	jeSound_Cfg	LocalCfg;

	assert(SoundS != NULL);
	assert(Sound  != NULL);	

	Channel = jeSound_GetChannel(SoundS->SoundM, (unsigned int)Sound);

	if (!Channel)
		{
			jeErrorLog_Add(JE_ERR_SEARCH_FAILURE,"jeSound_ModifySound:  Sound not found.");
			return JE_FALSE;
		}

	LocalCfg.Volume    = Volume;
	LocalCfg.Pan       = Pan;
	LocalCfg.Frequency = Frequency;
	LocalCfg.Volume *= SoundS->GlobalVolume;
	if ( jeSound_ModifyChannel(Channel, &LocalCfg) == JE_FALSE)
		{
			jeErrorLog_Add(JE_ERR_SUBSYSTEM_FAILURE,"jeSound_ModifySound:  Failed to modify channel.");
			return JE_FALSE;
		}
	return JE_TRUE;
}

//=====================================================================================
//	Sound_SoundIsPlaying
//=====================================================================================
JETAPI	jeBoolean JETCC jeSound_SoundIsPlaying(jeSound_System *SoundS, jeSound *Sound)
{
	Channel*	Channel;

	assert(SoundS != NULL);
	assert(Sound  != NULL);	

	Channel = jeSound_GetChannel(SoundS->SoundM, (unsigned int)Sound);

	if (!Channel)
		{
			return JE_FALSE;
		}

	return jeSound_ChannelPlaying(Channel);
}


//=====================================================================================
//	jeSound_GetStatus
//	added by tom morris May 2005
//	returns full DirectSound status flags
//=====================================================================================
JETAPI	int	JETCC jeSound_GetStatus(jeSound_System *pSoundSys, jeSound *pSound)
{
	Channel*	pChannel = NULL;

	assert(pSoundSys != NULL);
	assert(pSound  != NULL);	

	pChannel = jeSound_GetChannel(pSoundSys->SoundM, (unsigned int)pSound);

	if (!pChannel)
		{
			return 0;
		}

	return jeSound_ChannelGetBufferStatus(pChannel);
	//	possible flags include:
	//	DSBSTATUS_BUFFERLOST	The buffer is lost and must be restored before it can be played or locked. 
	//	DSBSTATUS_LOOPING		The buffer is being looped. If this value is not set, the buffer will stop
	//							when it reaches the end of the sound data. This value is returned only in 
	//							combination with DSBSTATUS_PLAYING.
	//	DSBSTATUS_PLAYING		The buffer is playing. If this value is not set, the buffer is stopped. 
	//	DSBSTATUS_LOCSOFTWARE	The buffer is playing in software. Set only for buffers created with the DSBCAPS_LOCDEFER flag.
	//	DSBSTATUS_LOCHARDWARE	The buffer is playing in hardware. Set only for buffers created with the DSBCAPS_LOCDEFER flag.
	//	DSBSTATUS_TERMINATED	The buffer was prematurely terminated by the voice manager and is not 
	//							playing. Set only for buffers created with the DSBCAPS_LOCDEFER flag.
}


//=====================================================================================
//=====================================================================================

static	BOOL DSParseWaveResource(const void *pvRes, WAVEFORMATEX **ppWaveHeader,
                         BYTE **ppbWaveData,DWORD *pcbWaveSize)
{
    DWORD *pdw;
    DWORD *pdwEnd;
    DWORD dwRiff;
    DWORD dwType;
    DWORD dwLength;

    if (ppWaveHeader)
        *ppWaveHeader = NULL;

    if (ppbWaveData)
        *ppbWaveData = NULL;

    if (pcbWaveSize)
        *pcbWaveSize = 0;

    pdw = (DWORD *)pvRes;
    dwRiff = *pdw++;
    dwLength = *pdw++;
    dwType = *pdw++;

    if (dwRiff != mmioFOURCC('R', 'I', 'F', 'F'))
        {
			jeErrorLog_Add(JE_ERR_BAD_PARAMETER,"DSParseWaveResource: not RIFF format.");
			goto exit;      // not even RIFF
		}

    if (dwType != mmioFOURCC('W', 'A', 'V', 'E'))
        {
			jeErrorLog_Add(JE_ERR_BAD_PARAMETER,"DSParseWaveResource: not WAVE format.");
			goto exit;      // not a WAV
		}

    pdwEnd = (DWORD *)((BYTE *)pdw + dwLength-4);

    while (pdw < pdwEnd)
    {
        dwType = *pdw++;
        dwLength = *pdw++;

        switch (dwType)
        {
        case mmioFOURCC('f', 'm', 't', ' '):
            if (ppWaveHeader && !*ppWaveHeader)
            {
                if (dwLength < sizeof(WAVEFORMAT))
                    {
						jeErrorLog_Add(JE_ERR_BAD_PARAMETER,"DSParseWaveResource: not proper WAV format.");
						goto exit;      // not a WAV
					}

                *ppWaveHeader = (WAVEFORMATEX *)pdw;

                if ((!ppbWaveData || *ppbWaveData) &&
                    (!pcbWaveSize || *pcbWaveSize))
                {
                    return TRUE;
                }
            }
            break;

        case mmioFOURCC('d', 'a', 't', 'a'):
            if ((ppbWaveData && !*ppbWaveData) ||
                (pcbWaveSize && !*pcbWaveSize))
            {
                if (ppbWaveData)
                    *ppbWaveData = (LPBYTE)pdw;

                if (pcbWaveSize)
                    *pcbWaveSize = dwLength;

                if (!ppWaveHeader || *ppWaveHeader)
                    return TRUE;
            }
            break;
        }

        pdw = (DWORD *)((BYTE *)pdw + ((dwLength+1)&~1));
    }

exit:
    return FALSE;
}

static	BOOL DSFillSoundBuffer(IDirectSoundBuffer8 *pDSB, BYTE *pbWaveData, DWORD cbWaveSize)
{
	assert( pDSB );
	assert( pbWaveData );
	assert( cbWaveSize );

    {
        LPVOID pMem1, pMem2;
        DWORD dwSize1, dwSize2;

        if (SUCCEEDED(IDirectSoundBuffer8_Lock(pDSB, 0, cbWaveSize,
            &pMem1, &dwSize1, &pMem2, &dwSize2, 0)))
        {
            ZeroMemory(pMem1, dwSize1);
            CopyMemory(pMem1, pbWaveData, dwSize1);

            if ( 0 != dwSize2 )
                CopyMemory(pMem2, pbWaveData+dwSize1, dwSize2);

            IDirectSoundBuffer8_Unlock(pDSB, pMem1, dwSize1, pMem2, dwSize2);
            return TRUE;
        }
		else
		{
			jeErrorLog_Add(JE_ERR_WINDOWS_API_FAILURE,"DSFillSoundBuffer: IDirectSoundBuffer_Lock failed.");
			return FALSE;
		}
    }
}


DSCAPS			dsCaps;
static	SoundManager *	CreateSoundManager(HWND hWnd )
{
	WAVEFORMATEX	pcmwf;
	DSBUFFERDESC	dsbdesc;
	HRESULT			hres;
	//	by trilobite	Jan. 2011
	//SoundManager *	sm;
	SoundManager *	sm = NULL;
	//
	// BEGIN - Upgrade to DirectSound 8 - paradoxnj 4/15/2005
	DWORD			channels, samplesPerSec, bitsPerSample, id;
	// END - Upgrade to DirectSound 8 - paradoxnj 4/15/2005

	// BEGIN - Upgrade to DirectSound 8 - paradoxnj 4/15/2005
	hres = DirectSoundCreate8(NULL, &lpDirectSound, NULL);
	if	(hres != DS_OK)
	{
		// failed somehow
		jeErrorLog_Add(JE_ERR_SOUND_RESOURCE,"CreateSoundManager: Could not initialize Direct Sound 8.");
//		FreeLibrary (hmodDirectSound);
		return NULL;
	}

	IDirectSound8_GetCaps(lpDirectSound, &dsCaps);

	// END - Upgrade to DirectSound 8 - paradoxnj 4/15/2005

	sm = (SoundManager*)jeRam_Allocate(sizeof(*sm));
	if	(!sm)
	{
		jeErrorLog_Add(JE_ERR_MEMORY_RESOURCE,"CreateSoundManager.");
		IDirectSound8_Release(lpDirectSound);
	//	FreeLibrary (hmodDirectSound);
		return NULL;
	}
	sm->smChannelCount = 0;
	sm->smNextChannelID = 1;
	sm->smChannels = NULL;

	// BEGIN - OGG Streamer - paradoxnj 4/17/2005
	InitializeCriticalSection(&sm->UpdateSection);
	sm->TermEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	
	sm->StreamList = jeChain_Create();
	sm->StreamPlayList = jeChain_Create();

	sm->Stream_Update_Thread = CreateThread(NULL, 4096, StreamUpdateFunction, (void*)sm, 0, &id);
	// END - OGG Streamer - paradoxnj 4/17/2005

	// BEGIN - Upgrade to DirectSound 8 - paradoxnj 4/15/2005
	if (dsCaps.dwFlags & DSCAPS_PRIMARYSTEREO)
		channels = 2;
	else
		channels = 1;

	if (dsCaps.dwFlags & DSCAPS_PRIMARY16BIT)
	{
		bitsPerSample = 16;
		samplesPerSec = 44100;
	}
	else
	{
		bitsPerSample = 8;
		samplesPerSec = 22050;
	}

	memset(&pcmwf, 0, sizeof(WAVEFORMATEX));
	pcmwf.wFormatTag = WAVE_FORMAT_PCM;
	pcmwf.nChannels = (WORD)channels;
	pcmwf.nSamplesPerSec = samplesPerSec;
	pcmwf.nBlockAlign = 4;
	pcmwf.wBitsPerSample = 16;
	pcmwf.nAvgBytesPerSec = pcmwf.nSamplesPerSec * pcmwf.nBlockAlign;

	memset(&dsbdesc, 0, sizeof(DSBUFFERDESC));
	dsbdesc.dwSize = sizeof(DSBUFFERDESC);
	dsbdesc.dwFlags = DSBCAPS_PRIMARYBUFFER | DSBCAPS_CTRL3D;
	//dsbdesc.dwBufferBytes = 0; //dwBufferBytes and lpwfxFormat must be set this way.
	//dsbdesc.lpwfxFormat = NULL;

	if (SUCCEEDED(IDirectSound8_SetCooperativeLevel(lpDirectSound, hWnd, DSSCL_PRIORITY)))
	{
		if (SUCCEEDED(IDirectSound8_CreateSoundBuffer(lpDirectSound, &dsbdesc, &sm->smPrimaryChannel, NULL)))
		{	//	by trilobite	Jan. 2011
			sm->IsInitialized = true;
			//
			return sm;
		}
		
		jeErrorLog_Add(JE_ERR_WINDOWS_API_FAILURE,"CreateSoundManager: IDirectSound_CreateSoundBuffer failed.");
		IDirectSound8_Release(lpDirectSound);
		//FreeLibrary (hmodDirectSound);
	}
	// END - Upgrade to DirectSound 8 - paradoxnj 4/15/2005

	jeErrorLog_Add(JE_ERR_WINDOWS_API_FAILURE,"CreateSoundManager: IDirectSound_SetCooperativeLevel failed.");
	
	jeRam_Free(sm);
	return NULL;
}

static	BOOL CreateChannel(DSBUFFERDESC *dsBD, Channel** chanelPtr)
{
	Channel* channel;
	LPDIRECTSOUNDBUFFER				lpBuff;

	channel = (Channel*)jeRam_Allocate( sizeof( Channel ) );
	if	( channel == NULL )
	{
		jeErrorLog_Add(JE_ERR_MEMORY_RESOURCE, "CreateChannel.");
		return( FALSE );
	}

	// BEGIN - Upgrade to DirectSound 8 - paradoxnj 4/15/2005
	if (FAILED(IDirectSound8_CreateSoundBuffer(lpDirectSound, dsBD, &lpBuff, NULL)))
	{
		jeErrorLog_Add(JE_ERR_WINDOWS_API_FAILURE, "CreateChannel: IDirectSound_CreateSoundBuffer failed.");
		return FALSE;
	}

	if (FAILED(IDirectSoundBuffer8_QueryInterface(lpBuff, IID_IDirectSoundBuffer8, (void**)&channel->buffer)))
	{
		jeErrorLog_Add(JE_ERR_WINDOWS_API_FAILURE, "CreateChannel:  IDirectSoundBuffer8_QueryInterface failed.");
		return FALSE;
	}

	if (FAILED(IDirectSoundBuffer8_GetFrequency(channel->buffer, (DWORD*)&channel->BaseFreq)))
	{
		jeErrorLog_Add(JE_ERR_WINDOWS_API_FAILURE, "CreateChannel: IDirectSound_GetFrequency failed.");
		return FALSE;
	}
	// END - Upgrade to DirectSound 8 - paradoxnj

	channel->next = NULL;
	channel->nextDup = NULL;
	channel->ID = 0;
	channel->cfg.Volume = 1.0f;
	channel->cfg.Pan = 0.0f;
	channel->cfg.Frequency = 0.0f;
//	channel->name = Name;

	*chanelPtr = channel;
	return( TRUE );
}

//static	BOOL GetSoundData( char* Name, unsigned char** dataPtr)
static	BOOL GetSoundData( jeVFile *File, unsigned char** dataPtr)
{
//	FILE * f;
	int32 Size;
	uint8 *data;
//	int32		CurPos;

#if 0
	f = fopen(Name, "rb");
	
	if (!f)
	{
		jeErrorLog_Add(JE_ERR_FILEIO_OPEN, "GetSoundData.");
		return FALSE;
	}
#endif

#if 0
	CurPos = ftell (f);				// Save the startinf pos into this function
	fseek (f, 0, SEEK_END);			// Seek to end
	Size = ftell (f);				// Get End (this will be the size)
	fseek (f, CurPos, SEEK_SET);	// Restore file position
#endif

	if	(jeVFile_Size(File, &Size) == JE_FALSE)
		{
			jeErrorLog_Add(JE_ERR_FILEIO_READ,"GetSoundData: failed to get size of sound file.");
			return FALSE;
		}

	data = (uint8*)jeRam_Allocate(Size);

	if (!data) 
	{
		jeErrorLog_Add(JE_ERR_MEMORY_RESOURCE, "GetSoundData.");
		return FALSE;
	}
	
	if	(jeVFile_Read(File, data, Size) == JE_FALSE)
	{
		jeErrorLog_Add(JE_ERR_FILEIO_READ,"GetSoundData: failed to read sound data.");
		jeRam_Free(data);
		return FALSE;
	}

//	fread(data, Size, 1, f);

//	fclose(f);
	*dataPtr = data;
	return( TRUE );
}

static	BOOL ParseData( const uint8* data, DSBUFFERDESC* dsBD, BYTE ** pbWaveData )
{

	//Parse the Data
	memset(dsBD, 0, sizeof(DSBUFFERDESC));

	dsBD->dwSize = sizeof(DSBUFFERDESC);
	//dsBD->dwFlags = DSBCAPS_GLOBALFOCUS | DSBCAPS_LOCHARDWARE | DSBCAPS_GETCURRENTPOSITION2 | DSBCAPS_CTRL3D | DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLFREQUENCY | DSBCAPS_MUTE3DATMAXDISTANCE;
	dsBD->dwFlags = DSBCAPS_STATIC | DSBCAPS_GETCURRENTPOSITION2 | DSBCAPS_CTRLFREQUENCY | DSBCAPS_CTRLPAN | DSBCAPS_CTRLVOLUME ;// | DSBCAPS_CTRLDEFAULT;
	if	(!DSParseWaveResource(data, &dsBD->lpwfxFormat, pbWaveData, &dsBD->dwBufferBytes))
	{
		jeErrorLog_Add(JE_ERR_SUBSYSTEM_FAILURE, "ParseData.");
		return FALSE;
	}

	return( TRUE );

}

static	BOOL jeSound_FillSoundChannel(SoundManager *sm, jeVFile *File, unsigned int* Handle )
{
	DSBUFFERDESC	dsBD;
	INT NumBytes;
	uint8		*data = NULL;
	BYTE *			pbWaveData;
	Channel* channel;

	assert( Handle );
	assert( sm );

	*Handle = 0;
	
	if(!GetSoundData( File, &data ))
		{
			jeErrorLog_Add(JE_ERR_SUBSYSTEM_FAILURE,"jeSound_FillSoundChannel.");
			return( FALSE );
		}

	if( !ParseData( data, &dsBD, &pbWaveData ) )
	{
		jeErrorLog_Add(JE_ERR_SUBSYSTEM_FAILURE,"jeSound_FillSoundChannel.");
		jeRam_Free(data);
		return( FALSE );
	}

	NumBytes = dsBD.dwBufferBytes;
	
	//Create the channel
	if	(!CreateChannel(&dsBD, &channel))
	{
		jeErrorLog_Add(JE_ERR_SUBSYSTEM_FAILURE,"jeSound_FillSoundChannel.");
		jeRam_Free(data);
		return FALSE;
	}
	channel->next = sm->smChannels;
	channel->ID = sm->smNextChannelID++;
	channel->Data = data;

	sm->smChannels = channel;
	sm->smChannelCount++;

	//Fill the channel
	if (!DSFillSoundBuffer(channel->buffer, pbWaveData, NumBytes))
		{
			jeErrorLog_Add(JE_ERR_SUBSYSTEM_FAILURE,"jeSound_FillSoundChannel.");
			return FALSE;
		}
	
	*Handle = channel->ID;
	return TRUE;
}


static	void StopDupBuffers( Channel* channel )
{
	Channel* dupChannel, *prevChannel;

	assert( channel );

	dupChannel = channel->nextDup;
	prevChannel = channel;
	while( dupChannel )
	{
		IDirectSoundBuffer8_Stop(dupChannel->buffer);
		dupChannel = dupChannel->nextDup;
	}
}

static	void ClearDupBuffers( Channel* channel )
{
	Channel* dupChannel, *prevChannel;
	assert( channel );

	dupChannel = channel->nextDup;
	prevChannel = channel;
	while( dupChannel )
	{
		if( !jeSound_ChannelPlaying( dupChannel ) )
		{
			prevChannel->nextDup = dupChannel->nextDup;
			IDirectSoundBuffer8_Release(dupChannel->buffer);
//			free( dupChannel );
			jeRam_Free(dupChannel);
			dupChannel = prevChannel->nextDup;
		}
		else
		{
			prevChannel = dupChannel;
			dupChannel = dupChannel->nextDup;
		}
	}
}

static	BOOL jeSound_FreeAllChannels(SoundManager *sm)
{
	int Error;
	
	//	by trilobite	Jan. 2011
	//Channel* channel, *nextChannel;
	Channel* channel = NULL, *nextChannel = NULL;
	//

	channel = sm->smChannels;
	while( channel )
	{
		nextChannel = channel->next;
		StopDupBuffers( channel );
		ClearDupBuffers( channel );
		Error = IDirectSoundBuffer_Stop(channel->buffer);
		if (Error != DS_OK)
		{
			jeErrorLog_Add(JE_ERR_WINDOWS_API_FAILURE, "jeSound_FreeAllChannels: IDirectSoundBuffer_Stop failed.");
			return FALSE;
		}
		// BEGIN - Upgrade to DirectSound 8 - paradoxnj 4/15/2005
		Error = IDirectSoundBuffer8_Release(channel->buffer);
		if (Error != DS_OK)
		{
			jeErrorLog_Add(JE_ERR_WINDOWS_API_FAILURE, "jeSound_FreeAllChannels: IDirectSound_Release failed.");
			return FALSE;
		}
		// END - Upgrade to DirectSound 8 - paradoxnj 4/15/2005

		if	(channel->Data)
			jeRam_Free(channel->Data);
		jeRam_Free(channel);
		channel = nextChannel;
	}
	sm->smChannels = NULL;
	sm->smChannelCount = 0;

	return TRUE;
}


static	BOOL jeSound_FreeChannel(SoundManager *sm, Channel* channel)
{
	int Error;
	Channel*prevChannel = NULL, *curChannel;
	assert( channel );
	assert( sm );

	{
		StopDupBuffers( channel );
		ClearDupBuffers( channel );
		// BEGIN - Upgrade to DirectSound8 - paradoxnj 4/15/2005
		Error = IDirectSoundBuffer8_Stop(channel->buffer);
		if (Error != DS_OK)
		{
			jeErrorLog_Add(JE_ERR_WINDOWS_API_FAILURE, "jeSound_FreeChannel: IDirectSoundBuffer_Stop failed.");
			return FALSE;
		}

		// BEGIN - Bug fix:  Sounds don't stop playing when level is unloaded... - paradoxnj 5/10/2005
		/*Error = IDirectSoundBuffer8_Release(channel->buffer);
		if (Error != DS_OK)
		{
			jeErrorLog_Add(JE_ERR_WINDOWS_API_FAILURE, "jeSound_FreeChannel: IDirectSound_Release failed.");
			return FALSE;
		}*/
		IDirectSoundBuffer8_Release(channel->buffer);
		// END - Bug fix:  Sounds don't stop playing when level is unloaded... - paradoxnj 5/10/2005

		// END - Upgrade to DirectSound 8 - paradoxnj 4/15/2005

		if( channel->Data )
			jeRam_Free(channel->Data);

		curChannel = sm->smChannels;
		while( curChannel && curChannel != channel )
		{
			prevChannel = curChannel;
			curChannel = curChannel->next;
		}
		if( curChannel )
		{
			if( prevChannel )
				prevChannel->next = curChannel->next;
			else
				sm->smChannels = curChannel->next;
			jeRam_Free(curChannel);
		}
	}

	return TRUE;
}

static	Channel* ReloadData(void *Data)
{
	DSBUFFERDESC	dsBD;
	BYTE *			pbWaveData;
	INT NumBytes;
	Channel* channel;

	if( !ParseData( (const uint8*)Data, &dsBD, &pbWaveData ) )
		{
			jeErrorLog_Add(JE_ERR_SUBSYSTEM_FAILURE,"ReloadData");
			return( NULL );
		}

	NumBytes = dsBD.dwBufferBytes;
	
	//Create the channel
	if( !CreateChannel(&dsBD, &channel ) )
		{
			jeErrorLog_Add(JE_ERR_SUBSYSTEM_FAILURE,"ReloadData");
			return NULL;
		}

	//Fill the channel
	if ( !DSFillSoundBuffer(channel->buffer, pbWaveData, NumBytes))
		{
			jeErrorLog_Add(JE_ERR_SUBSYSTEM_FAILURE,"ReloadData");
			return NULL;
		}
	return( channel );
}

static	BOOL DupChannel( SoundManager *sm, Channel* channel, Channel** dupChannelPtr )
{
	Channel* dupChannel;
	IDirectSoundBuffer			*pBuffer;
	HRESULT Error;

	assert( sm );
	assert( channel );
	assert( dupChannelPtr );

	*dupChannelPtr = NULL;
	dupChannel =  (Channel*)jeRam_Allocate( sizeof(Channel ) );
	if( dupChannel == NULL )
	{
		jeErrorLog_Add(JE_ERR_MEMORY_RESOURCE, "DupChannel" );
		return FALSE;
	}

	// BEGIN - Upgrade to DirectSound 8 - paradoxnj 4/15/2005
	Error = IDirectSound8_DuplicateSoundBuffer( lpDirectSound, (LPDIRECTSOUNDBUFFER)channel->buffer, &pBuffer);
	if( Error != DS_OK )
	{
		jeRam_Free(dupChannel);
		dupChannel = ReloadData( channel->Data );
		if( dupChannel == NULL )
		{
			jeErrorLog_Add(JE_ERR_SUBSYSTEM_FAILURE, "DupChannel");
			return FALSE;
		}
	}

	IDirectSoundBuffer_QueryInterface(pBuffer, IID_IDirectSoundBuffer8, (void**)&dupChannel->buffer);
	// END - Upgrade to DirectSound 8 - paradoxnj 4/15/2005

	dupChannel->ID =  sm->smNextChannelID++;
	dupChannel->next = NULL;
	dupChannel->nextDup = channel->nextDup;
	dupChannel->cfg = channel->cfg;
	dupChannel->Data = channel->Data;
	channel->nextDup = dupChannel;
	*dupChannelPtr = dupChannel;
	return( TRUE );
}

static	BOOL	jeSound_StartSoundChannel( SoundManager *sm, unsigned int Handle, jeSound_Cfg *cfg, int loop, unsigned int* sfx)
{
	HRESULT	hres;
	Channel* channel, *dupChannel;
	
	assert( sm );
	assert( cfg );

	if( Handle == 0 )
		{
			jeErrorLog_Add(JE_ERR_BAD_PARAMETER,"jeSound_StartSoundChannel: bad handle (0).");
			return( FALSE );
		}
	channel = jeSound_GetChannel( sm, Handle );
	//Clear all non-playing duplicate buffers.
	if (!channel)
		{
			jeErrorLog_Add(JE_ERR_INTERNAL_RESOURCE,"jeSound_StartSoundChannel: no channel available.");
			return ( FALSE );
		}
	ClearDupBuffers(channel);
	//If the main buffer is playing and all non-playing dups have been cleared
	//we need a new duplicate.
	if( jeSound_ChannelPlaying( channel ) )
	{
		if(!DupChannel( sm,channel, &dupChannel ) )
			{
				jeErrorLog_Add(JE_ERR_SUBSYSTEM_FAILURE,"jeSound_StartSoundChannel.");
				return( FALSE );
			}
		channel = dupChannel;
	}
	if( !jeSound_ModifyChannel( channel, cfg ) )
		{
			jeErrorLog_Add(JE_ERR_SUBSYSTEM_FAILURE,"jeSound_StartSoundChannel.");
			return( FALSE );
		}

	// BEGIN - Upgrade to DirectSound 8 - paradoxnj 4/15/2005
	IDirectSoundBuffer8_SetCurrentPosition(channel->buffer, 0);
	hres = IDirectSoundBuffer_Play( channel->buffer,
				  				   0,
				  				   0,
				  				   loop ? DSBPLAY_LOOPING : 0);

	if	(hres == DS_OK)
	{
		if( sfx )
			*sfx = channel->ID;
		return TRUE;
	}
	// END - Upgrade to DirectSound 8 - paradoxnj 4/15/2005

	jeErrorLog_Add(JE_ERR_WINDOWS_API_FAILURE,"jeSound_StartSoundChannel: IDirectSoundBuffer_Play failed.");
	return FALSE;
}

static	BOOL jeSound_StopSoundChannel(Channel* channel)
{
	HRESULT	hres;

	assert(channel);

	// BEGIN - Upgrade to DirectSound 8 - paradoxnj 4/15/2005
	hres = IDirectSoundBuffer8_Stop(channel->buffer);
	// END - Upgrade to DirectSound 8 - paradoxnj 4/15/2005
	if	(hres == DS_OK)
		return TRUE;

	jeErrorLog_Add(JE_ERR_WINDOWS_API_FAILURE,"jeSound_StopSoundChannel: IDirectSoundBuffer_Stop failed.");
	return FALSE;
}

static	void DestroySoundManager(SoundManager *sm)
{
	//	by trilobite	Jan. 2011
	sm->IsInitialized = false;	//	a switch to stop StreamUpdateFunction from processing streams while sm is shutting down
	//StreamChannel *snd;
	StreamChannel *snd = NULL;
	//
	assert( sm );

	jeSound_FreeAllChannels( sm );

	// BEGIN - OGG Streamer - paradoxnj 4/28/2005
	if (sm->Stream_Update_Thread)
		CloseHandle(sm->Stream_Update_Thread);

	for (snd = (StreamChannel*)jeChain_GetNextLinkData(sm->StreamPlayList, NULL); snd != NULL; snd = (StreamChannel*)jeChain_GetNextLinkData(sm->StreamPlayList, snd))
	{
		DWORD				status;

		IDirectSoundBuffer8_GetStatus(snd->buffer, &status);
		if (status & DSBSTATUS_PLAYING)
			IDirectSoundBuffer8_Stop(snd->buffer);

		IDirectSoundBuffer8_Release(snd->buffer);
		snd->buffer = NULL;
	}

	for (snd = (StreamChannel*)jeChain_GetNextLinkData(sm->StreamList, NULL); snd != NULL; snd = (StreamChannel*)jeChain_GetNextLinkData(sm->StreamList, snd))
	{
		IDirectSoundBuffer8_Release(snd->buffer);
		snd->buffer = NULL;

		jeRam_Free(snd->OGG);
		jeRam_Free(snd);
	}

	jeChain_Destroy(&sm->StreamPlayList);
	jeChain_Destroy(&sm->StreamList);
	// END - OGG Streamer - paradoxnj 4/28/2005

	// BEGIN - Upgrade to DirectSound 8 - paradoxnj 4/15/2005
	if (sm->smPrimaryChannel != NULL)
		IDirectSoundBuffer8_Release(sm->smPrimaryChannel);

	if (lpDirectSound != NULL)
		IDirectSound8_Release(lpDirectSound);
	// END - Upgrade to DirectSound 8 - paradoxnj 4/15/2005

	jeRam_Free(sm);

	//	by trilobite	Jan. 2011
//	ZeroMemory(sm, sizeof(SoundManager));	//	don't do this...
	(SoundManager*)jeRam_AllocateClear(sizeof(*sm));
}

static	BOOL	jeSound_ModifyChannel( Channel *channel, jeSound_Cfg *cfg )
{
	int Error, Vol, Pan, Freq;
	assert( channel );
	assert( cfg     );	

	ClearDupBuffers(channel);
	// BEGIN - Upgrade to DirectSound 8 - paradoxnj 4/15/2005
	if( cfg->Volume != channel->cfg.Volume )
	{
		Vol = (DWORD)((1.0 - cfg->Volume  ) * DSBVOLUME_MIN);

		Error = IDirectSoundBuffer8_SetVolume(channel->buffer, Vol);
		if (Error != DS_OK)
		{
			jeErrorLog_Add(JE_ERR_WINDOWS_API_FAILURE,"jeSound_ModifyChannel: IDirectSoundBuffer_SetVolume failed.");
			return FALSE;
		}
		
		channel->cfg.Volume = cfg->Volume;
	}

	if( cfg->Pan != channel->cfg.Pan )
	{
		Pan = (int)(cfg->Pan  * DSBPAN_RIGHT);

		Error = IDirectSoundBuffer8_SetPan(channel->buffer, Pan);
		if (Error != DS_OK)
		{
			jeErrorLog_Add(JE_ERR_WINDOWS_API_FAILURE,"jeSound_ModifyChannel: IDirectSoundBuffer_SetVolume failed.");
			return FALSE;
		}
		
		channel->cfg.Pan = cfg->Pan;
	}


	if( cfg->Frequency != channel->cfg.Frequency )
	{

		Freq = (DWORD)(channel->BaseFreq * cfg->Frequency);
		Error = IDirectSoundBuffer8_SetFrequency(channel->buffer, Freq);
		if (Error != DS_OK)
		{
			jeErrorLog_Add(JE_ERR_WINDOWS_API_FAILURE,"jeSound_ModifyChannel: IDirectSoundBuffer_SetFrequency failed.");
			return FALSE;
		}
		channel->cfg.Frequency = cfg->Frequency;
	}
	// END - Upgrade to DirectSound 8 - paradoxnj 4/15/2005

	return TRUE;
}

static	int	jeSound_ChannelPlaying( Channel *channel )
{
	DWORD	dwStatus = 0;
	DWORD	dwError = 0;

	assert( channel );

	// BEGIN - Upgrade to DirectSound 8 - paradoxnj 4/15/2005
	dwError = IDirectSoundBuffer8_GetStatus( channel->buffer, &dwStatus);
	if( dwError != DS_OK)
		{
			//jeErrorLog_Add(JE_ERR_WINDOWS_API_FAILURE,"jeSound_ModifyChannel: IDirectSoundBuffer_GetStatus failed.");
			return 0;
		}
	// END - Upgrade to DirectSound 8 - paradoxnj 4/15/2005

	return( dwStatus & DSBSTATUS_PLAYING );
}

///////////////////////////////////////////////////////////////////////////////////////
//	jeSound_ChannelGetBufferStatus
//	added by tom morris May 2005
//	returns full DirectSound status flags
///////////////////////////////////////////////////////////////////////////////////////
static	DWORD	jeSound_ChannelGetBufferStatus( Channel *channel)
{
	DWORD	dwStatus = 0;
	DWORD	dwError = 0;

	assert( channel );

	dwError = IDirectSoundBuffer8_GetStatus( channel->buffer, &dwStatus);
	if( dwError != DS_OK)
		{
			return 0;
		}
	//	possible flags include:
	//	DSBSTATUS_BUFFERLOST	The buffer is lost and must be restored before it can be played or locked. 
	//	DSBSTATUS_LOOPING		The buffer is being looped. If this value is not set, the buffer will stop
	//							when it reaches the end of the sound data. This value is returned only in 
	//							combination with DSBSTATUS_PLAYING.
	//	DSBSTATUS_PLAYING		The buffer is playing. If this value is not set, the buffer is stopped. 
	//	DSBSTATUS_LOCSOFTWARE	The buffer is playing in software. Set only for buffers created with the DSBCAPS_LOCDEFER flag.
	//	DSBSTATUS_LOCHARDWARE	The buffer is playing in hardware. Set only for buffers created with the DSBCAPS_LOCDEFER flag.
	//	DSBSTATUS_TERMINATED	The buffer was prematurely terminated by the voice manager and is not 
	//							playing. Set only for buffers created with the DSBCAPS_LOCDEFER flag.
	return( dwStatus);
}



static	Channel* jeSound_GetChannel( SoundManager *sm, unsigned int ID )
{
	Channel* dupChannel;
	Channel* channel = sm->smChannels;

	while( channel )
	{
		if( channel->ID == ID )
			break;
		dupChannel = channel->nextDup;
		while( dupChannel )
		{
			if( dupChannel->ID == ID )
				break;
			dupChannel = dupChannel->nextDup;
		}
		if( dupChannel )
			return( dupChannel );
		channel = channel->next;
	}
	return( channel );
}



// BEGIN - OGG Streamer - paradoxnj 4/17/2005
static StreamChannel *CreateStreamChannel(jeVFile *FS, const char *filename)
{
	StreamChannel				*sc = NULL;
	IDirectSoundBuffer			*buf = NULL;
	DSBUFFERDESC				desc;
//	HRESULT						hres;

	sc = JE_RAM_ALLOCATE_STRUCT(StreamChannel);
	if (!sc)
		return NULL;

	sc->OGG = jeOGGStream_Create(FS, filename);
	if (!sc->OGG)
	{
		jeRam_Free(sc);
		sc = NULL;

		return NULL;
	}

	ZeroMemory(&desc, sizeof(DSBUFFERDESC));

	desc.dwSize = sizeof(DSBUFFERDESC);

	FillBuffer(sc);
}

static jeBoolean UpdateSoundBuffer(StreamChannel *stream)
{
	DWORD						read_cursor, write_cursor;
	DWORD						data_to_copy;
	LPVOID						data1, data2;
	DWORD						size1, size2;
	HRESULT						hres;

	hres = IDirectSoundBuffer8_GetCurrentPosition(stream->OGG->pBuffer, &read_cursor, &write_cursor);
	if (FAILED(hres))
		return JE_FALSE;

	if (read_cursor > stream->LastReadPos)
		stream->BytesPlayed += read_cursor - stream->LastReadPos;
	else
		stream->BytesPlayed += (stream->Caps.dwBufferBytes - stream->LastReadPos) + read_cursor;

	if (stream->BytesPlayed >= jeOGGStream_GetSize(stream->OGG))
	{
		if (stream->Looping)
			stream->BytesPlayed -= jeOGGStream_GetSize(stream->OGG);
		else
		{
			IDirectSoundBuffer8_Stop(stream->buffer);
			return JE_TRUE;
		}
	}

	if (stream->DataCursor < read_cursor)
		data_to_copy = read_cursor - stream->DataCursor;
	else
		data_to_copy = (stream->Caps.dwBufferBytes - stream->DataCursor) + read_cursor;

	if (data_to_copy > (stream->Caps.dwBufferBytes / 2))
		data_to_copy = stream->Caps.dwBufferBytes / 2;

	hres = IDirectSoundBuffer8_Lock(stream->buffer, stream->DataCursor, data_to_copy, &data1, &size1, &data2, &size2, 0);
	if (FAILED(hres))
		return JE_FALSE;

	if (jeOGGStream_IsEOF(stream->OGG))
	{
		memset(data1, GetSilenceData(stream->OGG), size1);
		if (size2)
			memset(data2, GetSilenceData(stream->OGG), size2);

		stream->DataCursor += (size1 + size2);
	}
	else
	{
		uint32						bytes_read = 0;

		bytes_read = jeOGGStream_Read(stream->OGG, (char*)data1, size1);
		if (bytes_read == 0)
			return JE_FALSE;

		stream->DataCursor += bytes_read;
		if (data2 && (size1 == bytes_read))
		{
			bytes_read = jeOGGStream_Read(stream->OGG, (char*)data2, size2);
			if (bytes_read == 0)
				return JE_FALSE;

			stream->DataCursor += bytes_read;
		}
	}

	hres = IDirectSoundBuffer8_Unlock(stream->buffer, data1, size1, data2, size2);
	if (FAILED(hres))
		return JE_FALSE;

	if (stream->Looping && jeOGGStream_IsEOF(stream->OGG))
		jeOGGStream_Reset(stream->OGG);

	stream->DataCursor %= stream->Caps.dwBufferBytes;
	stream->LastReadPos = read_cursor;

	return JE_TRUE;
}

static DWORD WINAPI StreamUpdateFunction(LPVOID Context)
{
	//	by trilobite	Jan. 2011
	//SoundManager				*sm = (SoundManager*)Context;
	SoundManager				*sm = NULL;
	sm = (SoundManager*)Context;
	//
	static int					ServiceStreams = 0;

	//	by trilobite	Jan. 2011
	if (sm)
	{
		while (1)
		{
			Sleep(50);

			//	by trilobite jan. 2011	//	note. jeBoolean evaluates to TRUE after sm->IsInitialized is
										//	set to JE_FALSE and sm destroyed
										//	so using bool instead of jeBoolean or BOOL
			if (!sm->IsInitialized)	
				return 1;

				EnterCriticalSection(&sm->UpdateSection);

				if ((ServiceStreams++) % 4 == 1)
				{
					jeChain_Link				*Link = NULL;

					for (Link = jeChain_GetFirstLink(sm->StreamList); Link != NULL; Link = jeChain_LinkGetNext(Link))
					{
						StreamChannel			*stream = (StreamChannel*)jeChain_LinkGetLinkData(Link);
						if (!UpdateSoundBuffer(stream))
							continue;
					}
				}
	
			LeaveCriticalSection(&sm->UpdateSection);
		}
	}

	return 0;
}

static uint8 GetSilenceData(jeOGGStream *OGG)
{
	if (OGG->Format.wBitsPerSample == 8)
		return 0x80;
	else if (OGG->Format.wBitsPerSample == 16)
		return 0x00;

	return 0;
}

static void FillBuffer(StreamChannel *sc)
{
	void					*data1;
	DWORD					size1;
	uint32					bytes_read;
	HRESULT					hres;

	hres = IDirectSoundBuffer8_Lock(sc->buffer, 0, 0, &data1, &size1, NULL, NULL, DSBLOCK_ENTIREBUFFER);
	if (FAILED(hres))
		return;

	bytes_read = jeOGGStream_Read(sc->OGG, (char*)data1, size1);
	if (bytes_read == 0)
		return;

	sc->DataCursor += bytes_read;
	sc->DataCursor %= size1;

	if (bytes_read < size1)
		memset((uint8*)data1 + bytes_read, GetSilenceData(sc->OGG), size1 - bytes_read);

	hres = IDirectSoundBuffer8_Unlock(sc->buffer, data1, size1, NULL, 0);
	if (FAILED(hres))
		return;
}

// END - OGG Streamer - paradoxnj 4/17/2005