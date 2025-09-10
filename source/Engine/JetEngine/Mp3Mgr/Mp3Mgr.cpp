//Mp3Mgr
//It lets your app play MP3s

#include "Mp3Mgr_h.h"
#include "sound.h"
#include "jet.h"
#include "assert.h"


//	DEFINES AND DECLARATIONS	==============================================

static Media media;

//	FUNCTIONS	==============================================================

jeBoolean Initmp3Mgr(HWND mainwindowhandle)
{
    // Filter interface initialize?
    if( SUCCEEDED( CoInitialize( NULL )))
		return TRUE;
	
    return FALSE;
}

//
//	UnInitmp3Mgr
//	Unloads mp3Mgr
//
void UnInitmp3Mgr(HWND mainwindowhandle)
{
    CoUninitialize( );
}

//
//	mp3Mgr
//
/*	This funcion initializez mp3Mgr and connects it to the main window.
	This is necessary so that streaming audio calls will deliver audio
	to the main game window.	*/

//this is the main component and the engine should init it with the soundsystem

JETAPI jeMp3Mgr * JETCC jeMp3_CreateManager(HWND mainwindowhandle)
{
	jeMp3Mgr *Mp3Mgr;
	
	Mp3Mgr = JE_RAM_ALLOCATE_STRUCT(jeMp3Mgr);
	
	memset(Mp3Mgr, 0, sizeof(jeMp3Mgr));
	
	// Initialise COM and the application
    Initmp3Mgr(mainwindowhandle);
	InitMedia(mainwindowhandle);

	Mp3Mgr->num_mp3s = 0;
	Mp3Mgr->cur_mp3 = 0;
	Mp3Mgr->mwh = mainwindowhandle;
	
	return(Mp3Mgr);
		
}	// Mp3Mgr

JETAPI jeBoolean JETCC jeMp3_DestroyManager(jeMp3Mgr **Mp3Mgr)
{
	//destroys the Mp3Manager
	jeMp3Mgr *	mp3;
	int i;

	assert(Mp3Mgr != NULL);

	mp3 = *Mp3Mgr;

	if ( mp3 ) //check 1
	{
		if (media.pGraph) //check 2
		{
			if (mp3->num_mp3s != 0) //check 3 (just to be sure)
			{

				StopMp3();
				for(i = 0; i < mp3->num_mp3s; i++)
				{
					mp3->files[i].szFileName = NULL;
				}
		
				DeleteContentsMp3();
			}
		}

		jeRam_Free(mp3);

		return JE_TRUE;

	}

return JE_FALSE;
}


// CanPlay
//
// Return true if we can go to a playing state from our current state
//
BOOL CanPlay()
{
    return (media.state == Stopped || media.state == Paused);
}


//
// CanStop
//
// Return true if we can go to a stopped state from our current state
//
BOOL CanStop()
{
    return (media.state == Playing || media.state == Paused);
}


//
// IsInitialized
//
// Return true if we have loaded and initialized a multimedia file
//
BOOL IsInitialized()
{
    return (media.state != Uninitialized);
}


//
// ChangeStateTo
//
void ChangeStateTo( State newState )
{
    media.state = newState;
}

//
// InitMedia
//
// Initialization
//
jeBoolean InitMedia(HWND mainwindowhandle)
{
    ChangeStateTo( Uninitialized );
	
    media.hGraphNotifyEvent = NULL;
    media.pGraph = NULL;
	
    return JE_TRUE;
}

/*	WHAT THE HECK ARE FILTER GRAPHS!?	(explaination by Tom Morris)
Here is a brief tutorial from the Microsoft DirectX Media SDK.

filter graph 
A collection of filters. Typically, a filter graph contains filters that
are connected to perform a particular operation, such as playing back a media
file, or capturing video from a VCR to the hard disk.
  
filter 
A key component in the DirectShow architecture, a filter is a COM object that
supports DirectShow interfaces or base classes. It might operate on streams of
data .3
in a variety of ways, such as reading, copying, modifying, or writing the
data to a file. Sources, transform filters, and renderers are all particular
types of filters. A filter contains pins that it uses to connect to other filters. 
	
Tom's spin on filter graphs.
About the stupidest and most confusing name I ever heard. But that complaint aside,
the Windows Media Player handles audio data by streaming it straight from the data
source, through the CPU and out to the sound card. It is my understanding that the
sound data don't ever reside in memory as a static entity at a particular address.
As a result, this means that we have to use different kinds of techniques to manage
sounds processed by Mp3Mgr. Mp3Mgr accesses and uses the guts of the Windows Media
Player to manage streaming audio files, including mp3.
	  
Since the sound exists as a dynamic, streaming entity and not a pool of data, Mp3Mgr
must communicate with it by checking the status of the stream (which is always
changing). We will do this by querying the Media Player interface (which keeps tabs
on streaming status) and processing the report our query returns (by way of HRESULT
types. Right now, Mp3Mgr is very limited in the ways it can influence sound output.
Hopefully, future versions will add more versatility and functionality.
		
The following code is a total hack by me, a modification of the sample code provided
by Microsoft in its cplay.c example, found in the Microsoft DirectX Media SDK.
*/

//
// CreateFilterGraph
//
BOOL CreateFilterGraph()
{
    IMediaEvent *pME;					//	Event Pointer
    HRESULT hr;							//	for communicating with the filter graph
	
    ASSERT(media.pGraph == NULL);		//	make sure we start clean
	
    hr = CoCreateInstance(CLSID_FilterGraph,           // CLSID of object
		NULL,                         // Outer unknown
		CLSCTX_INPROC_SERVER,         // Type of server
		IID_IGraphBuilder,           // Interface wanted
		(void **) &media.pGraph);     // Returned object
    if (FAILED(hr))
	{
		media.pGraph = NULL;
		return FALSE;
    }
	
    // We use this to find out events sent by the filtergraph
	
    hr = media.pGraph->QueryInterface(IID_IMediaEvent, 
		(void **) &pME);				//	open lines of communication
    if (FAILED(hr)) 
	{
		DeleteContentsMp3();
		return FALSE;
    }
										//	establish event handle
    hr = pME->GetEventHandle((OAEVENT*) &media.hGraphNotifyEvent);
    pME->Release();
	
    if (FAILED(hr)) 
	{
		DeleteContentsMp3();
		return FALSE;
    }
	
    return TRUE;
	
} // CreateFilterGraph


// Destruction
//
// DeleteContents
//
void DeleteContentsMp3()
{
    if (media.pGraph != NULL) 
	{
		media.pGraph->Release();
		media.pGraph = NULL;
    }
	
    // this event is owned by the filter graph and is thus invalid
    media.hGraphNotifyEvent = NULL;
	
    ChangeStateTo( Uninitialized );
	
}	//	Delete Contents

//
// RenderFile
// Process the file through the appropriate filter path. This function is called by
// OpenMediaFile()
//
BOOL RenderFile( LPSTR szFileName )
{
    HRESULT hr;
    WCHAR wPath[MAX_PATH];
	
    DeleteContentsMp3();
	
    if ( !CreateFilterGraph() ) 
	{
		return FALSE; 
    } 
	
    MultiByteToWideChar( CP_ACP, 0, szFileName, -1, wPath, MAX_PATH );
	hr = media.pGraph->RenderFile(wPath, NULL);
			
    if (FAILED( hr )) 
	{
		return FALSE;
    } 
    return TRUE;
	
} // RenderFile


//
// OpenMediaFile
// This function opens and renders the specified media file.
// File..Open has been selected
//
jeBoolean OpenMediaFile(LPSTR szFile )
{
    if( RenderFile( szFile ))	//	this calls the filter graph
    {
		ChangeStateTo( Stopped );
		return JE_TRUE;
		
    }

	return JE_FALSE;
} // OpenMediaFile


//
// PlayMp3
//

void PlayMp3(long volume, jeBoolean loop)
{
	
    if( CanPlay() )
	{
		HRESULT hr;
		IMediaControl *pMC;
		IBasicAudio		*pMA;
		IMediaPosition	*pMP;
		
		// Obtain the interface to our filter graph
		hr = media.pGraph->QueryInterface(IID_IMediaControl, 
			(void **) &pMC);
		
		if( SUCCEEDED(hr) )
		{
		/*	In order to loop sounds, we will check with Media
		Player to see when the sound is almost over. When
		it's within 0.05 sec of ending, we'll stop the sound,
		rewind it and declare that the sound is ready to play
		again. The next time around your game's main loop,
		PlayMp3 will start it up again. And so it goes...
			*/
			
			//	Set volume level
			hr = media.pGraph->QueryInterface(IID_IBasicAudio,
				(void**) &pMA);
			
			if (SUCCEEDED(hr)) 
			{							/*	Set volume. 
				-10000 is silence,	0 is full volume*/
				hr = pMA->put_Volume(volume);
				pMA->Release();	//	release the interface
			}			
			
			
			if (loop == JE_TRUE)
			{			
				hr = media.pGraph->QueryInterface(IID_IMediaPosition,
					(void**) &pMP);
				if (SUCCEEDED(hr)) 
				{
					// start from last position, but rewind if near the end
					REFTIME tCurrent, tLength;
					hr = pMP->get_Duration(&tLength);
					if (SUCCEEDED(hr)) 
					{
						hr = pMP->get_CurrentPosition(&tCurrent);
						if (SUCCEEDED(hr)) 
						{
							// within 0.05 sec of end? (or past end?)
							if ((tLength - tCurrent) < 0.05) 
							{
								//	Rewind it	
								pMP->put_CurrentPosition(0);
								CanPlay();		//	It's ready to play again
							}						//	If not, you can't loop.
						}
					}
					pMP->Release();
				}
				
				
				// Ask the filter graph to play and release the interface
				hr = pMC->Run();
				pMC->Release();
				
				if( SUCCEEDED(hr) )
				{
					return;
				}
			}
			else
				// Ask the filter graph to play and release the interface
				hr = pMC->Run();
			pMC->Release();
			
			if( SUCCEEDED(hr) )
			{
				return;
			}	
		}
		
	}
	
} // PlayMp3


//
// StopMp3
//
//

void StopMp3()
{
	HRESULT hr;
	IMediaControl *pMC;
	
	// Obtain the interface to our filter graph
	hr = media.pGraph->QueryInterface(IID_IMediaControl, 
		(void **) &pMC);
	if( SUCCEEDED(hr) )
	{
		pMC->Stop();	//	stop it!
		pMC->Release();
		ChangeStateTo( Stopped );
	}
	
	return ;
} //StopMp3


//
// GetGraphEventHandle
//
// We use this to check for graph events
//
HANDLE GetGraphEventHandle()
{
    return media.hGraphNotifyEvent;
	
} // GetGraphEventHandle


//
// OnGraphNotify
//
// If the event handle is valid, then ask the graph if
// anything has happened (eg the graph has stopped...)
//
void OnGraphNotify()
{
    IMediaEvent *pME;
    long lEventCode, lParam1, lParam2;
	
    ASSERT( media.hGraphNotifyEvent != NULL );
	
    if( SUCCEEDED(media.pGraph->QueryInterface(IID_IMediaEvent, (void **) &pME)))
	{
		if( SUCCEEDED(pME->GetEvent(&lEventCode, &lParam1, &lParam2, 0))) 
		{
			if (lEventCode == EC_COMPLETE) 
			{
				StopMp3();
			}
		}
		pME->Release();
    }
}



/*===================================================================================
MP3PLAYING
This function checks to see if the current mp3 file is still playing.
=====================================================================================*/
jeBoolean Mp3Playing()
{
	HRESULT				hr;
	IMediaPosition		*pMP;
	//	query the interface
	hr = media.pGraph->QueryInterface(IID_IMediaPosition,
		(void**) &pMP);
				if (SUCCEEDED(hr)) 
				{
					REFTIME tCurrent, tLength;		//	find the max playtime
					hr = pMP->get_Duration(&tLength);
					if (SUCCEEDED(hr)) 
					{								//	where are we now?
						hr = pMP->get_CurrentPosition(&tCurrent);
						if (SUCCEEDED(hr)) 
						{
							
							//	Test to see if there is any time left
							while ((tLength - tCurrent) > 0) 
								return JE_TRUE;	// if so, still playing, buddy.
						}
					}
				}							//	when done playing...
				pMP->Release();	//	release our access to the interface
				return JE_FALSE;			//	mp3 file is all done playing.				
}	//Mp3Playing
