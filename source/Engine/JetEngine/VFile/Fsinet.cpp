/****************************************************************************************/
/*  FSINET.CPP                                                                          */
/*                                                                                      */
/*  Author: Eli Boling                                                                  */
/*  Description: Internet file system implementation                                    */
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
#include	<objbase.h>
#include	<urlmon.h>

#include	<stdio.h>
#include	<assert.h>

#include	"basetype.h"
#include	"ram.h"
#include	"vfile.h"
#include	"vfile._h"
#include	"ThreadQueue.h"

#include	"fsinet.h"

#include	"log.h"
#include	"ThreadLog.h"

typedef	enum
{
	STATE_READING,
	STATE_ERROR,
	STATE_DATACOMPLETE,
	STATE_COMPLETE,
}	INetFileState;
	// These should be in temporal order (except for Error) : 
	//	Reading,DataComplete,Complete

struct	Statistics
{
	int		MaxBlockSize;
	int		MinBlockSize;
	int		TotalBlocksReceived;
};

struct	INetFile : public IBindStatusCallback
{
	INetFile();
	~INetFile();
	jeBoolean				Initialize(jeBoolean InitAsDirectory);

	STDMETHODIMP			QueryInterface(REFIID riid,  void **ppvObj);
	STDMETHODIMP_(ULONG)	AddRef(void);
	STDMETHODIMP_(ULONG)	Release(void);

	STDMETHODIMP			OnStartBinding( 
		/* [in] */ DWORD dwReserved,
		/* [in] */ IBinding __RPC_FAR *pib);
	
	STDMETHODIMP			GetPriority( 
		/* [out] */ LONG __RPC_FAR *pnPriority);
	
	STDMETHODIMP	OnLowResource( 
		/* [in] */ DWORD reserved);
	
	STDMETHODIMP	OnProgress( 
		/* [in] */ ULONG ulProgress,
		/* [in] */ ULONG ulProgressMax,
		/* [in] */ ULONG ulStatusCode,
		/* [in] */ LPCWSTR szStatusText);
	
	STDMETHODIMP	OnStopBinding( 
		/* [in] */ HRESULT hresult,
		/* [unique][in] */ LPCWSTR szError);
	
	STDMETHODIMP	GetBindInfo( 
		/* [out] */ DWORD __RPC_FAR *grfBINDF,
		/* [unique][out][in] */ BINDINFO __RPC_FAR *pbindinfo);
	
	STDMETHODIMP	OnDataAvailable( 
		/* [in] */ DWORD grfBSCF,
		/* [in] */ DWORD dwSize,
		/* [in] */ FORMATETC __RPC_FAR *pformatetc,
		/* [in] */ STGMEDIUM __RPC_FAR *pstgmed);
	
	STDMETHODIMP	OnObjectAvailable( 
		/* [in] */ REFIID riid,
		/* [iid_is][in] */ IUnknown __RPC_FAR *punk);

	long MemoryFilePos(void)
	{
		return ClientPos + TrueFileBase;
	}

	uint32 BytesAvailable(void)
	{
		return (CheckedForHints == JE_TRUE) ? (long)m_dwTotalRead - MemoryFilePos() : 0;
	}

	jeBoolean	GetFullPath(char *Buff, int MaxLen) const;

	unsigned int	Signature;
	INetFileState	State;
	char *			FullPath;
	IBinding *		Binding;
	DWORD			RefCount;
	IStream *		m_spStream;
	DWORD			m_dwTotalRead;
	jeVFile *		MemoryFile;
	CRITICAL_SECTION	Lock;

	jeThreadQueue_Job *	Job;

	jeBoolean		IgnoreHints;		// Should we ignore hints completely?
	jeBoolean		HasHints;			// Does this file have hints
	jeBoolean		CheckedForHints;	// We have decided whether or not we have hints
	long			TrueFileBase;		// Position of first user bits (past hint)
	long			ClientPos;			// Client's file position
	jeVFile_Hints	Hints;
	jeVFile *		HintsFile;
	long			HintsPosition;		// Relative position into the hints

	jeBoolean		IsDirectory;		// Only set if this is really a directory
	INetFile *		Parent;				// Set to outer 

	jeVFile_RemoteFileStatistics		Stats;
};

#ifdef	__BORLANDC__
#define	jeRam_Allocate	malloc
#define	jeRam_Free	free
#endif

//	"IF01"
#define	INETFILE_SIGNATURE	0x31304649

#define	CHECK_HANDLE(H)	assert(H);assert(H->Signature == INETFILE_SIGNATURE);

INetFile::INetFile()
{
	// Don't use memset here.  It smashes the vtable entry (!?).
	FullPath = NULL;
	Binding = NULL;
	RefCount = 0;
	m_spStream = NULL;
	m_dwTotalRead = 0;
	MemoryFile = NULL;
	Signature = 0;
	TrueFileBase = 0;
	ClientPos = 0;
	Job = NULL;
	IgnoreHints = JE_FALSE;
	HasHints = JE_FALSE;
	CheckedForHints = JE_FALSE;
	Hints.HintData = NULL;
	Hints.HintDataLength = 0;
	HintsPosition = 0;
	State = STATE_READING;
	IsDirectory = JE_FALSE;
	Parent = NULL;
	memset(&Stats, 0, sizeof(Stats));
	InitializeCriticalSection(&Lock);
}

INetFile::~INetFile()
{

	// CB : this wait must be outside the critical section !
	if	(Job)
		jeThreadQueue_WaitOnJob(Job, JE_THREADQUEUE_STATUS_COMPLETED);

	EnterCriticalSection(&Lock);

	if	(Job)
		jeThreadQueue_JobDestroy(&Job);

	Signature = 0;
	if	(FullPath)
		jeRam_Free(FullPath);

	if	(HasHints == JE_TRUE)
	{
		assert(Hints.HintData != NULL);
		assert(Hints.HintDataLength > 0);
		jeRam_Free(Hints.HintData);
	}

	if	(MemoryFile)
		jeVFile_Close(MemoryFile);

	LeaveCriticalSection(&Lock);
	DeleteCriticalSection(&Lock);
}

// This function is a little pointless.  Should clean this up
jeBoolean INetFile::Initialize(jeBoolean InitAsDirectory)
{
	jeVFile_MemoryContext	Context;

	// This function is only used for non-directory files.

	if	(InitAsDirectory == JE_FALSE)
	{
		memset(&Context, 0, sizeof(Context));
		MemoryFile = jeVFile_OpenNewSystem(NULL, JE_VFILE_TYPE_MEMORY, NULL, &Context, JE_VFILE_OPEN_CREATE);
		if	(!MemoryFile)
			return JE_FALSE;
	}

	return JE_TRUE;
}

STDMETHODIMP INetFile::QueryInterface(REFIID riid,  void **ppvObj)
{
	*ppvObj = NULL;

	if	(riid == IID_IUnknown || riid == IID_IBindStatusCallback)
	{
		*ppvObj = this;
		AddRef();
		return S_OK;
	}

	return E_NOINTERFACE;
}

ULONG STDMETHODCALLTYPE INetFile::AddRef(void)
{
	return ++RefCount;
}

ULONG STDMETHODCALLTYPE INetFile::Release(void)
{
	return --RefCount;
}

STDMETHODIMP INetFile::OnStartBinding(
	DWORD /*Reserved*/,
	IBinding __RPC_FAR *pib)
{
	Binding = pib;
	if	(Binding)
		Binding->AddRef();
	return S_OK;
}

STDMETHODIMP INetFile::GetPriority( 
            /* [out] */ LONG __RPC_FAR * /*pnPriority*/)
{
//	*pnPriority = THREAD_PRIORITY_NORMAL;
	return S_OK;
}
        
STDMETHODIMP INetFile::OnLowResource( 
            /* [in] */ DWORD /*reserved*/)
{
#pragma message ("FSInet : We're not nice about low resources")
	return S_OK;
}
        
STDMETHODIMP INetFile::OnProgress( 
            /* [in] */ ULONG /*ulProgress*/,
            /* [in] */ ULONG /*ulProgressMax*/,
            /* [in] */ ULONG /*ulStatusCode*/,
            /* [in] */ LPCWSTR /*szStatusText*/)
{
	return S_OK;
}
        
STDMETHODIMP INetFile::OnStopBinding( 
            /* [in] */ HRESULT /*hresult*/,
            /* [unique][in] */ LPCWSTR /*szError*/)
{
	if	(Binding)
	{
		Binding->Release();
		Binding = NULL;
	}
	State = STATE_COMPLETE;
	return S_OK;
}
        
STDMETHODIMP INetFile::GetBindInfo( 
            /* [out] */ DWORD __RPC_FAR *grfBINDF,
            /* [unique][out][in] */ BINDINFO __RPC_FAR *pbindinfo)
{
	ULONG	Size;

	*grfBINDF = BINDF_ASYNCHRONOUS | BINDF_ASYNCSTORAGE | BINDF_NOWRITECACHE | BINDF_GETNEWESTVERSION;

	Size = pbindinfo->cbSize;
	memset(pbindinfo, 0, Size);
	pbindinfo->cbSize = Size;
	pbindinfo->dwBindVerb = BINDVERB_GET;
	
	return S_OK;
}
        
STDMETHODIMP INetFile::OnDataAvailable( 
            /* [in] */ DWORD grfBSCF,
            /* [in] */ DWORD dwSize,
            /* [in] */ FORMATETC __RPC_FAR * /*pformatetc*/,
            /* [in] */ STGMEDIUM __RPC_FAR *pstgmed)
{
	HRESULT hr = S_OK;

	if	(State == STATE_ERROR)
		return S_OK;

	EnterCriticalSection(&Lock);

	// Get the Stream passed
	if (grfBSCF & BSCF_FIRSTDATANOTIFICATION)
	{
		if (!m_spStream && pstgmed->tymed == TYMED_ISTREAM)
		{
			m_spStream = pstgmed->pstm;
			m_spStream->AddRef();
		}
	}

	DWORD dwRead = dwSize - m_dwTotalRead; // Minimum amount available that hasn't been read
	DWORD dwActuallyRead = 0;            // Placeholder for amount read during this pull

	// If there is some data to be read then go ahead and read them
	if	(m_spStream)
	{
		if	(dwRead > 0)
		{
			BYTE* 					pBytes;
			jeVFile_MemoryContext	MemoryContext;

			ThreadLog_Printf("FSInet: got %d bytes on '%s'\n", dwRead, FullPath);
			// Update statistics
			Stats.TotalBlocksReceived++;
			if	(dwRead > (DWORD)(Stats.MaxBlockSize))
				Stats.MaxBlockSize = dwRead;
			if	(dwRead < (DWORD)(Stats.MinBlockSize))
				Stats.MinBlockSize = dwRead;

			if	(jeVFile_Seek(MemoryFile, 0, JE_VFILE_SEEKEND) == JE_FALSE)
			{
				LeaveCriticalSection(&Lock);
				State = STATE_ERROR;
				return S_OK;
			}
			if	(jeVFile_Seek(MemoryFile, dwRead, JE_VFILE_SEEKCUR) == JE_FALSE)
			{
				LeaveCriticalSection(&Lock);
				State = STATE_ERROR;
				return S_OK;
			}
			jeVFile_UpdateContext(MemoryFile, &MemoryContext, sizeof(MemoryContext));
			pBytes = ((BYTE *)MemoryContext.Data) + MemoryContext.DataLength - dwRead;
			if (pBytes == NULL)
			{
				LeaveCriticalSection(&Lock);
				State = STATE_ERROR;
				return S_OK;
			}
			hr = m_spStream->Read(pBytes, dwRead, &dwActuallyRead);
			if	(!SUCCEEDED(hr))
			{
				LeaveCriticalSection(&Lock);
				State = STATE_ERROR;
				return S_OK;
			}
			m_dwTotalRead += dwActuallyRead;
			Stats.TotalBytesReceived = m_dwTotalRead;
		}
	}

	if (BSCF_LASTDATANOTIFICATION & grfBSCF)
	{
		ThreadLog_Printf("FSInet: last data notification for '%s'\n", FullPath);
		m_spStream->Release();
		State = STATE_DATACOMPLETE;
	}

	if	(CheckedForHints == JE_FALSE && State != STATE_ERROR)
	{
		jeVFile_HintsFileHeader *	HintsHeader;

		if	(m_dwTotalRead > sizeof(HintsHeader))
		{
			jeVFile_MemoryContext	MemoryContext;

			jeVFile_UpdateContext(MemoryFile, &MemoryContext, sizeof(MemoryContext));
			HintsHeader = (jeVFile_HintsFileHeader *)MemoryContext.Data;
			if	(HintsHeader->Signature != JE_VFILE_HINTSFILEHEADER_SIGNATURE)
			{
				CheckedForHints = JE_TRUE;
			}
			else
			{
				if	(HintsHeader->HintDataLength + sizeof(*HintsHeader) <= m_dwTotalRead)
				{
					Hints.HintDataLength = HintsHeader->HintDataLength;
					Hints.HintData = jeRam_Allocate(HintsHeader->HintDataLength);
					if	(Hints.HintData)
					{
						jeVFile_MemoryContext	Context;

#pragma message("FSInet : Need to clean up the hints file implementation")

						Context.Data = Hints.HintData;
						Context.DataLength = Hints.HintDataLength;
						memcpy(Hints.HintData, (char *)MemoryContext.Data + sizeof(*HintsHeader), HintsHeader->HintDataLength);
						TrueFileBase = sizeof(*HintsHeader) + HintsHeader->HintDataLength;
						HintsFile = jeVFile_OpenNewSystem(NULL,
														  JE_VFILE_TYPE_MEMORY,
														  NULL,
														  &Context,
														  JE_VFILE_OPEN_READONLY);
						if	(!HintsFile)
							State = STATE_ERROR;
														  
						ThreadLog_Printf("FSInet: got hints for '%s'\n", FullPath);
						CheckedForHints = JE_TRUE;
						HasHints = JE_TRUE;
					}
					else
					{
						State = STATE_ERROR;
					}
				}
			}
		}
		else if	(State >= STATE_DATACOMPLETE)
		{
			// Total size of the file is smaller than the hints header!
			// Now why would anyone stream that?
			CheckedForHints = JE_TRUE;
		}
	}

	LeaveCriticalSection(&Lock);

	return hr;
}
        
STDMETHODIMP INetFile::OnObjectAvailable( 
            /* [in] */ REFIID /*riid*/,
            /* [iid_is][in] */ IUnknown __RPC_FAR * /*punk*/)
{
	assert(!"Should not be getting any objects");
	return S_OK;
}

jeBoolean	INetFile::GetFullPath(char *Buff, int MaxLen) const
{
	int	Length;

	assert(FullPath);

	Length = strlen(FullPath) + 1;
	if	(Length > MaxLen)
		return JE_FALSE;

	// <> CB 2/23
	if ( strstr(FullPath,":/") ) // absolute path
	{
		strcpy(Buff, FullPath);
	}
	else // relative path
	{
		if	(Parent)
		{
			if	(Parent->GetFullPath(Buff, MaxLen - Length) == JE_FALSE)
				return JE_FALSE;
		}

		if ( strlen(Buff) > 0 )
		{
			char * EndPtr = Buff + strlen(Buff) - 1;
			if ( *EndPtr != '/' || EndPtr[-1] == ':' )
			{
				EndPtr[1] = '/';
				EndPtr[2] = 0;
			}
		}
		strcat(Buff, FullPath);
	}

	return JE_TRUE;
}

static	void *	JETCC FSINet_FinderCreate(
	jeVFile *		/*FS*/,
	void *			/*Handle*/,
	const char *	/*FileSpec*/)
{
	return NULL;
}

static	jeBoolean	JETCC FSINet_FinderGetNextFile(void * /*Handle*/)
{
	return JE_FALSE;
}

static	jeBoolean	JETCC FSINet_FinderGetProperties(void * /*Handle*/, jeVFile_Properties * /*Props*/)
{
	return JE_FALSE;
}

static	void JETCC FSINet_FinderDestroy(void * /*Handle*/)
{
	assert(!"Not implemented");
}

WINOLEAPI CoInitializeEx(LPVOID reserved, DWORD flags);

void	MyOpenFile(jeThreadQueue_Job *Job, void *Context)
{
	INetFile *	File;
	HRESULT		hr;
	char		AbsolutePath[_MAX_PATH + _MAX_PATH];
//#pragma message("FSInet : Not dealing with paths properly")

	File = (INetFile *)Context;

#if 0
// The code here is for free threaded downloads.  We ought to be able to make
// this work.
{
		HRESULT hr = S_OK;
		IBindCtx *	BindCtx;
		IMoniker *	Moniker;
		IStream *	Stream;
		LPWSTR		WideString;
		int			Length;

		CoInitializeEx(NULL, 0);

		assert(File->FullPath);
		Length = strlen(File->FullPath);

		WideString = (LPWSTR)malloc(Length * 2 + 100);
		::MultiByteToWideChar(CP_ACP, 0, File->FullPath, -1, WideString, Length * 2 + 100);

		hr = CreateURLMoniker(NULL, WideString, &Moniker);
		if (SUCCEEDED(hr))
//			hr = CreateBindCtx(0, &BindCtx);
			hr = CreateAsyncBindCtx(0, File, NULL, &BindCtx);

//		if (SUCCEEDED(hr))
//			hr = RegisterBindStatusCallback(BindCtx, NewFile, 0, 0L);
//		if	(SUCCEEDED(hr))
//		{
//			hr = CreateAsyncBindCtx(0, NewFile, NULL, &BindCtx);
//		}
		else
			Moniker->Release();

//		if (SUCCEEDED(hr))
//			hr = IsValidURL(BindCtx, WideString, 0);

		if (SUCCEEDED(hr))
			hr = Moniker->BindToStorage(BindCtx, 0, IID_IStream, (void**)&Stream);
//		assert (SUCCEEDED(hr));
}
#else
	CoInitializeEx(NULL, 2);
//	printf("[%p] about to open the file\n", File);
	AbsolutePath[0] = 0;
	if	(File->GetFullPath(AbsolutePath, sizeof(AbsolutePath)) == JE_TRUE)
	{
//		Log_Printf("FSInet : about to open : %s\n",AbsolutePath);
		ThreadLog_Printf("FSInet : about to open : %s\n", AbsolutePath);
		hr = URLOpenStream(NULL, AbsolutePath, 0, static_cast<IBindStatusCallback *>(File));
#pragma message ("FSInet : Need to make URLOpenStream err-or handling more robust")
#if 1
		if	(hr != 0)
		{
			File->State = STATE_ERROR;
			ThreadLog_Printf("FSInet : ERROR opening : %s\n", AbsolutePath);
		}
		else
		{
			ThreadLog_Printf("FSInet : opened        : %s\n", AbsolutePath);
		}
#endif
	}
	else
	{
		File->State = STATE_ERROR;
	}
#endif

	while	(File->State == STATE_READING)
	{
		MSG		Msg;

		PeekMessage(&Msg, NULL, 0, 0, PM_NOREMOVE);
	}

	if	(File->State == STATE_ERROR)
	{
		char	Buff[1024];
		sprintf(Buff, "Error opening %s\r\n", AbsolutePath);
		OutputDebugString(Buff);
	}
	else
	{
		char	Buff[1024];
		sprintf(Buff, "Done opening  %s\r\n", AbsolutePath);
		OutputDebugString(Buff);
	}
}

static	void *	JETCC FSINet_Open(
	jeVFile *		/*FS*/,
	void *			Handle,
	const char *	Name,
	void *			/*Context*/,
	unsigned int 	OpenModeFlags)
{
	INetFile *	IFS;
	INetFile *	NewFile;
	int			Length;

	/*
		Apartment threading (single threaded) seems to work just fine for UrlOpenStream.
		If you use free threading, however, UrlOpenStream will not work.  If you use
		single threaded Apartment model (the default when using CoInitialize),
		IMoniker::BindToHost will fail with a not bindable error.  If you use free threading,
		then IMoniker::BindToHost will bind OK, and the callback will start getting
		requests, but then we fail further down the pike, and I haven't figured out
		why yet.  One problem with single threaded Apartment model is that we have to
		tickle the message queue a little or we don't get async downloads.  Hence the
		call to PeekMessage in this file.  Free threading is more efficient, and the
		file system has been built to deal with it, and as soon as we can get the
		binding to work, we should switch to it.  For now, we are single threaded
		apartment model on each download.
	*/
#pragma message ("FSInet : Really out to be free threaded")
//	CoInitializeEx(NULL, 2);
//	CoInitializeEx(NULL, 0);

	if	(!(OpenModeFlags & JE_VFILE_OPEN_READONLY))
		return NULL;
	if ( ! Name )	// <> CB 2/10
		return NULL;

	IFS = (INetFile *)Handle;
	CHECK_HANDLE(IFS);

	NewFile = new INetFile;
	if	(!NewFile)
		return NewFile;

	if	(OpenModeFlags & JE_VFILE_OPEN_DIRECTORY)
		NewFile->IsDirectory = JE_TRUE;

	if	(OpenModeFlags & JE_VFILE_OPEN_RAW)
		NewFile->IgnoreHints = JE_TRUE;

	if	(NewFile->Initialize(NewFile->IsDirectory) == JE_FALSE)
	{
		delete NewFile;
		return NULL;
	}

	Length = strlen(Name) + 2;
	NewFile->FullPath = (char *)jeRam_Allocate(Length);
	if	(!NewFile->FullPath)
	{
		delete NewFile;
		return NULL;
	}

	memcpy(NewFile->FullPath, Name, Length - 1);
#if 0 // <> CB 2/10
	if	(NewFile->IsDirectory == JE_TRUE)
		strcat(NewFile->FullPath, "/");
#endif

	NewFile->Parent = IFS;
	NewFile->Signature = INETFILE_SIGNATURE;
	NewFile->State = STATE_READING;

	if	(NewFile->IsDirectory == JE_FALSE)
	{
		ThreadLog_Printf("FSInet : starting open thread for : %s\n", NewFile->FullPath);
		NewFile->Job = jeThreadQueue_JobCreate(MyOpenFile, NewFile, NULL, 0x1000);
	#if 1 
		// we have to do this right now, because we don't have the emergency wait on job
		jeThreadQueue_PollJobs();
	#endif
	}

	return (void *)NewFile;
}

static	void *	JETCC FSINet_OpenNewSystem(
	jeVFile *		/*Base*/,
	const char *	Name,
	void *			/*Context*/,
	unsigned int 	OpenModeFlags)
{
	INetFile *	File;
	int			Length;

	if	(!(OpenModeFlags & (JE_VFILE_OPEN_READONLY | JE_VFILE_OPEN_DIRECTORY)))
		return NULL;

	File = new INetFile;
	if	(!File)
		return (void *)File;
	
	File->IsDirectory = JE_TRUE;
	if	(File->Initialize(File->IsDirectory) == JE_FALSE)
	{
		delete File;
		return NULL;
	}

	if ( ! Name ) Name = "http://"; //<> CB 2/10

	Length = strlen(Name) + 2;
	File->FullPath = (char *)jeRam_Allocate(Length);
	if	(File->FullPath == NULL)
	{
		delete File;
		return NULL;
	}
	memcpy(File->FullPath, Name, Length - 1);
#if 0  //<> CB 2/10
	strcat(File->FullPath, "/");
#endif

	File->Signature = INETFILE_SIGNATURE;
	File->State = STATE_READING;

	return (void *)File;
}

static	jeBoolean	JETCC FSINet_UpdateContext(
	jeVFile *		/*FS*/,
	void *			Handle,
	void *			Context,
	int 			ContextSize)
{
	INetFile *						File;
	jeVFile_RemoteFileStatistics *	Stats;
	
	File = (INetFile *)Handle;

	if	(File->IsDirectory == JE_TRUE)
		return JE_FALSE;
	
	CHECK_HANDLE(File);

	Stats = (jeVFile_RemoteFileStatistics *)Context;
	if	(ContextSize != sizeof(*Stats))
		return JE_FALSE;

	if	(File->State == STATE_ERROR)
		return JE_FALSE;

	*Stats = File->Stats;

	return JE_TRUE;
}

static	jeBoolean	JETCC FSINet_Close(void *Handle)
{
	INetFile *	File;
	
	File = (INetFile *)Handle;

	CHECK_HANDLE(File);

//	assert(File->State == STATE_COMPLETE);

//	<> CB moved inside the delete
//	if	(File->Job)
//		jeThreadQueue_WaitOnJob(File->Job,JE_THREADQUEUE_STATUS_COMPLETED);
	
	ThreadLog_Printf("FSInet: closing '%s'\n", File->FullPath);
	delete File;

	return JE_TRUE;
}

static	jeBoolean	JETCC FSINet_GetS(void *Handle, void *Buff, int MaxLen)
{
	INetFile *	File;
//	DWORD		BytesRead;
//	BOOL		Result;
//	char *		p;
//	char *		End;

	assert(Buff);
	assert(MaxLen != 0);

	File = (INetFile *)Handle;

	CHECK_HANDLE(File);

	if	(File->State == STATE_ERROR)
		return JE_FALSE;

#pragma message ("FSINet_GetS: Not implemented")

#if 0
	Result = ReadFile(File->FileHandle, Buff, MaxLen - 1, &BytesRead, NULL);
	if	(BytesRead == 0)
	{
		return JE_FALSE;
	}

	End = (char *)Buff + BytesRead;
	p = Buff;
	while	(p < End)
	{
		/*
		  This code will terminate a line on one of three conditions:
			\r	Character changed to \n, next char set to 0
			\n	Next char set to 0
			\r\n	First \r changed to \n.  \n changed to 0.
		*/
		if	(*p == '\r')
		{
			int Skip = 0;
			
			*p = '\n';		// set end of line
			p++;			// and skip to next char
			// If the next char is a newline, then skip it too  (\r\n case)
			if (*p == '\n')
			{
				Skip = 1;
			}
			*p = '\0';
			// Set the file pointer back a bit since we probably overran
			SetFilePointer(File->FileHandle, -(int)(BytesRead - ((p + Skip) - (char *)Buff)), NULL, FILE_CURRENT); 
			assert(p - (char *)Buff <= MaxLen);
			return JE_TRUE;
		}
		else if	(*p == '\n')
		{
			// Set the file pointer back a bit since we probably overran
			p++;
			SetFilePointer(File->FileHandle, -(int)(BytesRead - (p - (char *)Buff)), NULL, FILE_CURRENT); 
			*p = '\0';
			assert(p - (char *)Buff <= MaxLen);
			return JE_TRUE;
		}
		p++;
	}
#endif
	return JE_FALSE;
}

static	jeBoolean	JETCC FSINet_BytesAvailable(void *Handle, long *Count)
{
	INetFile *	File;
	MSG			Msg;

	assert(Count);

	File = (INetFile *)Handle;

	CHECK_HANDLE(File);

	if	(File->State == STATE_ERROR)
		return JE_FALSE;

	PeekMessage(&Msg, NULL, 0, 0, PM_NOREMOVE);

	if	(File->Job)
	{
		if	(!jeThreadQueue_WaitOnJob(File->Job, JE_THREADQUEUE_STATUS_RUNNING))
		{
			jeErrorLog_AddString((jeErrorLog_ErrorClassType)-1,"FSInet : Wait on job failed!",File->FullPath);
			return JE_FALSE;
		}
	}

	EnterCriticalSection(&File->Lock);
	*Count = File->BytesAvailable();
	LeaveCriticalSection(&File->Lock);

	return JE_TRUE;
}

extern "C" {
	extern jeVFile * Hack_VFS_File;
};

static	jeBoolean	JETCC FSINet_Read(void *Handle, void *Buff, uint32 Count)
{
	INetFile *	File;
	jeBoolean	DataAlreadyAvailable;

	assert(Buff);
	assert(Count != 0);

	File = (INetFile *)Handle;

	CHECK_HANDLE(File);

	assert(!Hack_VFS_File || jeVFile_IsValid(Hack_VFS_File));

	if	(((File->State >= STATE_DATACOMPLETE ) && File->BytesAvailable() < Count) ||
		 (File->State == STATE_ERROR))
	{
		if	(File->State == STATE_ERROR)
			jeErrorLog_AddString((jeErrorLog_ErrorClassType)-1,"FSInet : Read on file with error",File->FullPath);
		else
		{
			jeErrorLog_AddString((jeErrorLog_ErrorClassType)-1,"FSInet : Read past EOF", File->FullPath);
			assert(0);
		}
		return JE_FALSE;
	}

	assert(!Hack_VFS_File || jeVFile_IsValid(Hack_VFS_File));
	
	if	(!jeThreadQueue_WaitOnJob(File->Job,JE_THREADQUEUE_STATUS_RUNNING))
	{
		jeErrorLog_AddString((jeErrorLog_ErrorClassType)-1,"FSInet : Wait on job failed!", File->FullPath);
		return JE_FALSE;
	}

	assert(!Hack_VFS_File || jeVFile_IsValid(Hack_VFS_File));

#pragma message("FSInet : doesn't check for VFHH and handle hints!!!")

	if	(File->BytesAvailable() < Count)
	{
		ThreadLog_Printf("FSInet: stalling in read for %d bytes, %d available on '%s'\n", Count, File->BytesAvailable(), File->FullPath);
		DataAlreadyAvailable = JE_FALSE;
	}
	else
		DataAlreadyAvailable = JE_TRUE;

	// Block until we've got enough bytes
	while	(File->BytesAvailable() < Count)
	{
		jeThreadQueue_Sleep(1);
	
		if	(((File->State >= STATE_DATACOMPLETE ) && 
					File->BytesAvailable() < Count) ||
			 (File->State == STATE_ERROR))
		{
			if	(File->State == STATE_ERROR)
			{
				jeErrorLog_AddString((jeErrorLog_ErrorClassType)-1,"FSInet : Read on file with error",File->FullPath);
			}
			else
			{
				jeErrorLog_AddString((jeErrorLog_ErrorClassType)-1,"FSInet : Read past EOF",File->FullPath);
				assert(0);
			}
			return JE_FALSE;
		}
	}
	
	if	(DataAlreadyAvailable == JE_FALSE)
		ThreadLog_Printf("FSInet: finished wait for data on '%s'\n", File->FullPath);

	assert(!Hack_VFS_File || jeVFile_IsValid(Hack_VFS_File)); // @@ !

#pragma message ("FSInet : Investigate strange behaviour with sleep in FSINet_Read")
		// Behaviour seems better with the sleep out of the loop!??
//		jeThreadQueue_Sleep(1);

	EnterCriticalSection(&File->Lock);

	assert(File->BytesAvailable() >= Count);
	jeVFile_Seek(File->MemoryFile, File->MemoryFilePos(), JE_VFILE_SEEKSET);
	jeVFile_Read(File->MemoryFile, Buff, Count);
	jeVFile_Seek(File->MemoryFile, File->m_dwTotalRead, JE_VFILE_SEEKSET);
	File->ClientPos += Count;

	LeaveCriticalSection(&File->Lock);

	assert(!Hack_VFS_File || jeVFile_IsValid(Hack_VFS_File));

	return JE_TRUE;
}

static	jeBoolean	JETCC FSINet_Write(void * /*Handle*/, const void * /*Buff*/, int /*Count*/)
{
	return JE_FALSE;
}

static	jeBoolean	JETCC FSINet_Seek(void *Handle, int Where, jeVFile_Whence Whence)
{
	INetFile *	File;
	long		FinalPos = 0;
	long		MemoryFileSize;

	File = (INetFile *)Handle;

	if	(File->IsDirectory == JE_TRUE)
		return JE_FALSE;
	
	CHECK_HANDLE(File);

	EnterCriticalSection(&File->Lock);

	jeVFile_Size(File->MemoryFile, &MemoryFileSize);

	switch	(Whence)
	{
	case	JE_VFILE_SEEKCUR:
		FinalPos = File->MemoryFilePos() + Where;
		break;

	case	JE_VFILE_SEEKEND:
		if	(File->State < STATE_DATACOMPLETE)
		{
			LeaveCriticalSection(&File->Lock);
			return JE_FALSE;
		}

		FinalPos = MemoryFileSize - Where;
		break;

	case	JE_VFILE_SEEKSET:
		FinalPos = File->TrueFileBase + Where;
		break;

	default:
		assert(!"Unknown seek kind");
	}

	if	(FinalPos > MemoryFileSize)
	{
		LeaveCriticalSection(&File->Lock);
	
		// <> CB ; used to just return false
		// Block until we've got enough bytes

		#pragma message("Fsinet : Seek past EOF ! Grudgingly allowed..")
		jeErrorLog_AddString((jeErrorLog_ErrorClassType)-1,
			"Fsinet : Seek past EOF ! Grudgingly allowed..",NULL);

		if	(FinalPos > MemoryFileSize)
			ThreadLog_Printf("FSInet: stalling in seek on '%s'\n", File->FullPath);

		while	(FinalPos > MemoryFileSize)
		{
			jeThreadQueue_PollJobs();
			jeThreadQueue_Sleep(1);

			EnterCriticalSection(&File->Lock);
			jeVFile_Size(File->MemoryFile, &MemoryFileSize);
			
			if	((File->State >= STATE_DATACOMPLETE ) ||
				 (File->State == STATE_ERROR))
			{
				LeaveCriticalSection(&File->Lock);
				return JE_FALSE;
			}

			LeaveCriticalSection(&File->Lock);
		}
		
		EnterCriticalSection(&File->Lock);
	}

	File->ClientPos = FinalPos - File->TrueFileBase;

	LeaveCriticalSection(&File->Lock);

	return JE_TRUE;
}

static	jeBoolean	JETCC FSINet_EOF(const void *Handle)
{
	INetFile *	File;

	File = (INetFile *)Handle;

	CHECK_HANDLE(File);

	assert( File->MemoryFilePos() <= (long)File->m_dwTotalRead );

	if	((File->State >= STATE_DATACOMPLETE ) &&
		 (File->MemoryFilePos() == (long)File->m_dwTotalRead))
	{
		return JE_TRUE;
	}

	return JE_FALSE;
}

static	jeBoolean	JETCC FSINet_Tell(const void *Handle, long *Position)
{
	const INetFile *	File;

	File = (INetFile *)Handle;

	CHECK_HANDLE(File);

	if	(File->State == STATE_ERROR)
		return JE_FALSE;

	*Position = File->ClientPos;

	return JE_TRUE;
}

static	jeBoolean	JETCC FSINet_Size(const void *Handle, long * Size)
{
	INetFile *	File;
	MSG Msg;

	File = (INetFile *)Handle;

	CHECK_HANDLE(File);

	if	(File->State == STATE_ERROR)
		return JE_FALSE;

	if	(File->State < STATE_DATACOMPLETE) // can only return size when complete!
		return JE_FALSE;

	PeekMessage(&Msg, NULL, 0, 0, PM_NOREMOVE);

	EnterCriticalSection(&File->Lock);
	*Size = File->m_dwTotalRead;
	LeaveCriticalSection(&File->Lock);

	return JE_TRUE;
}

static	jeBoolean	JETCC FSINet_GetProperties(const void *Handle, jeVFile_Properties *Properties)
{
	const INetFile *	File;
	jeVFile_Attributes	Attribs;

	assert(Properties);

	File = (INetFile *)Handle;

	CHECK_HANDLE(File);

	if	(File->State == STATE_ERROR)
		return JE_FALSE;

	Attribs = JE_VFILE_ATTRIB_READONLY | JE_VFILE_ATTRIB_REMOTE;
	
	#pragma message("FSINet : GetProperties says nothing about _DIRECTORY !")
	
	if	(File->IsDirectory) // <> CB 2/11 !
		Attribs |= JE_VFILE_ATTRIB_DIRECTORY;

	Properties->Time.Time1 = 0;
	Properties->Time.Time2 = 0;
	
	Properties->AttributeFlags 		 = Attribs;
	Properties->Size		  		 = 0;

	Properties->Name[0] = 0; // <> CB 2/11 !

	return File->GetFullPath(Properties->Name, sizeof(Properties->Name));
}

static	jeBoolean	JETCC FSINet_SetSize(void * /*Handle*/, long /*size*/)
{
	return JE_FALSE;
}

static	jeBoolean	JETCC FSINet_SetAttributes(void * /*Handle*/, jeVFile_Attributes /*Attributes*/)
{
	return JE_FALSE;
}

static	jeBoolean	JETCC FSINet_SetTime(void * /*Handle*/, const jeVFile_Time * /*Time*/)
{
	return JE_FALSE;
}

static	jeVFile *	JETCC FSINet_GetHintsFile(void *Handle)
{
	INetFile *	File;

	File = (INetFile *)Handle;

	if	(File->IsDirectory == JE_TRUE)
		return NULL;
	
	CHECK_HANDLE(File);

	if	(File->State == STATE_ERROR)
		return NULL;

	if	(File->IgnoreHints == JE_TRUE)
		return NULL;

	if	(File->CheckedForHints == JE_FALSE)
		ThreadLog_Printf("FSInet: stalling in GetHintsFile on '%s'\n", File->FullPath);

	while	(File->CheckedForHints == JE_FALSE)
	{
		jeThreadQueue_JobStatus Status;

		Status = jeThreadQueue_JobGetStatus(File->Job);
		if ( Status == JE_THREADQUEUE_STATUS_WAITINGTOBEGIN )
		{
			jeThreadQueue_PollJobs();
		}
		if ( Status == JE_THREADQUEUE_STATUS_WAITINGFORTHREAD )
		{
			jeThreadQueue_PollJobs();
		}
		jeThreadQueue_Sleep(1);
	}

	if	(File->HasHints == JE_TRUE)
	{
		assert(File->HintsFile != NULL);
		return File->HintsFile;
	}

	return NULL;
}

static	jeBoolean	JETCC FSINet_FileExists(jeVFile * /*FS*/, void *Handle, const char * /*Name*/)
{
	INetFile *	File;

	File = (INetFile *)Handle;

//	if	(File != (void *)INETFILE_SIGNATURE)
//		return JE_TRUE;

//	CHECK_HANDLE(File);

//	assert(!"Not implemented");
//	return JE_FALSE;
#pragma message ("FSInet : FileExists is hacked to make jeVFile_Open function")
	return JE_TRUE;
}

static	jeBoolean	JETCC FSINet_Disperse(
	jeVFile *	/*FS*/,
	void *		/*Handle*/,
	const char * /*Directory*/)
{
	return JE_FALSE;
}

static	jeBoolean	JETCC FSINet_DeleteFile(jeVFile * /*FS*/, void * /*Handle*/, const char * /*Name*/)
{
	return JE_FALSE;
}

static	jeBoolean	JETCC FSINet_RenameFile(jeVFile * /*FS*/, void * /*Handle*/, const char * /*Name*/, const char * /*NewName*/)
{
	return JE_FALSE;
}

static	jeVFile_SystemAPIs	FSINet_APIs =
{
	FSINet_FinderCreate,
	FSINet_FinderGetNextFile,
	FSINet_FinderGetProperties,
	FSINet_FinderDestroy,

	FSINet_OpenNewSystem,
	FSINet_UpdateContext,
	FSINet_Open,
	FSINet_DeleteFile,
	FSINet_RenameFile,
	FSINet_FileExists,
	FSINet_Disperse,
	FSINet_Close,

	FSINet_GetS,
	FSINet_BytesAvailable,
	FSINet_Read,
	FSINet_Write,
	FSINet_Seek,
	FSINet_EOF,
	FSINet_Tell,
	FSINet_Size,

	FSINet_GetProperties,

	FSINet_SetSize,
	FSINet_SetAttributes,
	FSINet_SetTime,

	FSINet_GetHintsFile,

};

const jeVFile_SystemAPIs *JETCC FSINet_GetAPIs(void)
{
	return &FSINet_APIs;
}

