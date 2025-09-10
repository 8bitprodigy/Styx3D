/*!
	@file VFile.h
	
	@author Eli Boling
	@brief Virtual file interface

	@par Licence
	The contents of this file are subject to the Jet3D Public License       
	Version 1.02 (the "License"); you may not use this file except in         
	compliance with the License. You may obtain a copy of the License at       
	http://www.jet3d.com                                                        
                                                                             
	@par
	Software distributed under the License is distributed on an "AS IS"           
	basis, WITHOUT WARRANTY OF ANY KIND, either express or implied.  See           
	the License for the specific language governing rights and limitations          
	under the License.                                                               
                                                                                  
	@par
	The Original Code is Jet3D, released December 12, 1999.                            
	Copyright (C) 1996-1999 Eclipse Entertainment, L.L.C. All Rights Reserved           
*/


#ifndef	VFILE_H
#define	VFILE_H

#include	"BaseType.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Krouer: enable/disable trace in VFile */
//#define KROUERDEBUG

/*! @typedef jeVFile
	@brief A virtual file or filesystem, physic file or directory instance.
*/
typedef	struct	jeVFile			jeVFile;

//--------- Finder (Directory) --------------
typedef	struct	jeVFile_Finder	jeVFile_Finder;

typedef unsigned int    jeVFile_Attributes;

// <> is this still used?
typedef struct	jeVFile_Hints
{
	void *			HintData;
	unsigned int	HintDataLength;
}	jeVFile_Hints;

typedef	struct	jeVFile_Time
{
	unsigned long	Time1;
	unsigned long	Time2;
}	jeVFile_Time;

#define	JE_VFILE_ATTRIB_READONLY	(1UL<<0)
#define	JE_VFILE_ATTRIB_DIRECTORY	(1UL<<1)
#define	JE_VFILE_ATTRIB_REMOTE		(1UL<<2)

typedef	struct	jeVFile_Properties
{
	jeVFile_Time			Time;
	jeVFile_Attributes		AttributeFlags;
	long					Size;
//<>jeVFile_Hints			Hints;
	char					Name[1024];
}	jeVFile_Properties;

#ifdef _INC_WINDOWS
JETAPI void JETCC jeVFile_TimeToWin32FileTime(const jeVFile_Time *, LPFILETIME Win32FileTime);
	// Converts a jeVFile time to a Win32 FILETIME structure.  This API is the
	// way to get the file time into a format to do standardized date/time
	// operations on.  We do not provide date/time operations natively.
#endif

#ifdef BUILD_BE
#ifdef _SUPPORT_DEFS_H
JETAPI void JETCC jeVFile_TimeToTime_TFileTime(const jeVFile_Time *Time, bigtime_t* fileTime);
#endif
#endif

JETAPI jeVFile_Finder * JETCC jeVFile_CreateFinder(
	jeVFile *		FileSystem,
	const char *	FileSpec);
	// Creates a finder object from which you can get iterated file names.
	// This is findfirst/findnext functionality.  

JETAPI void JETCC jeVFile_DestroyFinder(jeVFile_Finder *Finder);
	// Destroys a Finder object

JETAPI jeBoolean JETCC jeVFile_FinderGetNextFile(jeVFile_Finder *Finder);
	// Tracks to the next file in the finder directory

JETAPI jeBoolean JETCC jeVFile_FinderGetProperties(const jeVFile_Finder *Finder, jeVFile_Properties *Properties);
	// Gets the file properties from a jeVFile_Finder.  You cannot set properties for
	// a file through a finder.  You have to set the properties through a jeVFile.

//--------- File System Operations ----

typedef	struct	jeVFile_MemoryContext
{
	void *	Data;
	int		DataLength;
}	jeVFile_MemoryContext;

typedef	struct	jeVFile_RemoteFileStatistics
{
	int		TotalBytesReceived;
	int		TotalBlocksReceived;
	int		MinBlockSize;
	int		MaxBlockSize;
}	jeVFile_RemoteFileStatistics;

typedef enum {
	JE_VFILE_TYPE_INVALID = 0,
	JE_VFILE_TYPE_DOS,
	JE_VFILE_TYPE_MEMORY,
	JE_VFILE_TYPE_VIRTUAL,
	JE_VFILE_TYPE_LZ,
	JE_VFILE_TYPE_FAKENET,
	JE_VFILE_TYPE_INTERNET,
	JE_VFILE_TYPE_COUNT
} jeVFile_TypeIdentifier;

// First three flags are mutually exclusive.  Combining them will result in failure
// returns for both jeVFile_OpenNewSystem and jeVFile_Open.
#define	JE_VFILE_OPEN_READONLY	 (1UL<<0)
#define	JE_VFILE_OPEN_UPDATE 	 (1UL<<1)
#define	JE_VFILE_OPEN_CREATE	 (1UL<<2)

#define JE_VFILE_OPEN_DIRECTORY  (1UL<<3)
#define	JE_VFILE_OPEN_RAW		 (1UL<<4)
				// Warning : VFile_Open_Raw is a new flag, 2/14/99 : it is not fully supported yet!
				//		use at your own risk!

JETAPI jeVFile * JETCC jeVFile_OpenNewSystem(
	jeVFile *					FS,
	jeVFile_TypeIdentifier	FileSystemType,  // { DOS, MEMORY, ETC ... },
	const char *			Name, 
	void *					Context, 
	unsigned int 			OpenModeFlags);
	// Opens a file / file system.

JETAPI jeBoolean JETCC jeVFile_UpdateContext(jeVFile *FS, void *Context, int ContextSize);

JETAPI jeVFile * JETCC jeVFile_GetContext(const jeVFile *File);
	// Returns the outer context in which File was opened.

JETAPI jeVFile * JETCC jeVFile_Open( 
	jeVFile *			FS,
	const char *	Name, 
	unsigned int 	OpenModeFlags);

#if 0
// Experimenting:
JETAPI jeVFile * JETCC jeVFile_GetSystem(jeVFile_TypeIdentifier SystemType);
JETAPI jeVFile * JETCC jeVFile_Create(
	jeVFile *		FS,
	const char *	Name, 
	unsigned int 	OpenModeFlags);
JETAPI jeVFile * JETCC jeVFile_SetParameters(jeVFile *File, void *Parameters, int ParameterSize);
JETAPI jeVFile * JETCC jeVFile_GetParameters(jeVFile *File, void *Parameters, int ParameterSize);
#endif

JETAPI jeBoolean JETCC jeVFile_Destroy(jeVFile **FS);

JETAPI void		 JETCC jeVFile_CreateRef(jeVFile *File);

/*
typedef	enum
{
	jeVFile_AppendPath,
	jeVFile_PrependPath,
}	jeVFile_SearchOrder;
*/

JETAPI jeBoolean JETCC jeVFile_AddPath(jeVFile *FS1, const jeVFile *FS2, jeBoolean Append);
	// Appends (or prepends) the path associated with FS2 into FS1.
	//    Append==JE_TRUE   causes the FS2 to be searched AFTER FS1
	//    Append==JE_FALSE  causes the FS2 to be searched BEFORE FS1

/*  perhaps jeVFile_AppendPath and jeVFile_PrependPath */


JETAPI jeBoolean JETCC jeVFile_DeleteFile(jeVFile *FS, const char *FileName);
	// Deletes a file within a file system.  Returns JE_TRUE on success, JE_FALSE
	// on failure.

JETAPI jeBoolean JETCC jeVFile_RenameFile(jeVFile *FS, const char *FileName, const char *NewName);
	// Renames a file within a file system.  Returns JE_TRUE on success, JE_FALSE
	// on failure.

JETAPI jeBoolean JETCC jeVFile_FileExists(jeVFile *FS, const char *FileName);
	// Returns JE_TRUE if the file FileName exists in FS, JE_FALSE otherwise.
	// Does not do any searching (?)

//jeVFile_VFileType jeVFile_Register( all kinds of stuff );

JETAPI jeBoolean JETCC jeVFile_Disperse(jeVFile *FS, const char *Directory);
// Always disperses recursively (falls out from implementation).

JETAPI jeBoolean JETCC jeVFile_Close (jeVFile *File);
	// closes and destroys the File

//---------- File Specific Operations -----------

typedef	enum
{
	JE_VFILE_SEEKCUR	= 0,
	JE_VFILE_SEEKEND	= 1,
	JE_VFILE_SEEKSET	= 2
}	jeVFile_Whence;

JETAPI jeBoolean JETCC jeVFile_GetS  		 (		jeVFile *File, void *Buff, int MaxLen);
												// GetS eats \n\r\0 at the end of a string
JETAPI jeBoolean JETCC jeVFile_BytesAvailable(		jeVFile *File, long *Count);
												// returns number of bytes which a Read can perform immediately
JETAPI jeBoolean JETCC jeVFile_Read  		 (		jeVFile *File, void *Buff, uint32 Count); // cjp modified from int to uint32..
JETAPI jeBoolean JETCC jeVFile_Write 		 (		jeVFile *File, const void *Buff, int Count);
JETAPI jeBoolean JETCC jeVFile_Seek  		 (		jeVFile *File, int where, jeVFile_Whence Whence);
												// do NOT seek forward on Internet files!
JETAPI jeBoolean JETCC jeVFile_Printf		 (		jeVFile *File, const char *Format, ...);
JETAPI jeBoolean JETCC jeVFile_EOF   		 (const jeVFile *File);
JETAPI jeBoolean JETCC jeVFile_Tell  		 (const jeVFile *File, long *Position);
JETAPI jeBoolean JETCC jeVFile_GetProperties (const jeVFile *File, jeVFile_Properties *Properties);
JETAPI jeBoolean JETCC jeVFile_GetName		 (const jeVFile *File, char *Buff, int MaxBuffLen);

JETAPI jeBoolean JETCC jeVFile_Size  		 (const jeVFile *File, long *Size);
JETAPI jeBoolean JETCC jeVFile_SetSize		 (		jeVFile *File, long Size);
JETAPI jeBoolean JETCC jeVFile_SetAttributes (		jeVFile *File, jeVFile_Attributes Attributes);
JETAPI jeBoolean JETCC jeVFile_SetTime		 (		jeVFile *File, const jeVFile_Time *Time);

JETAPI jeBoolean JETCC jeVFile_Rewind		 (		jeVFile *File);

JETAPI jeVFile * JETCC jeVFile_GetHintsFile	 (		jeVFile *File);
	// GetHintsFile is Deprecated; use CreateHF instead

JETAPI jeVFile * JETCC jeVFile_CreateHintsFile(		jeVFile *File);
							// if there are no hints :
							//	on a file open for read : returns NULL
							//  on a file open for write: creates a hints subfile
							//		warning : as of 2-8-99 , if you GetHintsFile for write, and
							//		then write nothing, you will get a VFHH0000 at the head of your file!
							//	destroy it when you're done


//---------------------------------------

#ifdef _DEBUG
JETAPI jeBoolean JETCC jeVFile_IsValid(const jeVFile * File);
#else
#define jeVFile_IsValid(F) (JE_TRUE)
#endif

#ifdef _DEBUG
// to avoid including windows:
#ifdef WIN32
_declspec(dllimport) void __stdcall OutputDebugStringA(const char * lpOutputString);
#define OutputDebugString OutputDebugStringA
#endif

#ifdef BUILD_BE
#define OutputDebugString// DEBUGGER
#endif

#define DebugShowOpenCount() do { char str[1024]; sprintf(str,"vfiles : %d\n",jeVFile_OpenCount()); OutputDebugString(str); } while(0)

JETAPI uint32 JETCC jeVFile_OpenCount(void);

#else

#define jeVFile_OpenCount() (0)
#define DebugShowOpenCount()

#endif

//---------------------------------------

#ifdef __cplusplus
}
#endif

#endif
