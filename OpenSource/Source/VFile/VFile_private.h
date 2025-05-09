/****************************************************************************************/
/*  VFILE._H                                                                             */
/*                                                                                      */
/*  Author: Eli Boling                                                                  */
/*  Description: Systems internal interfaces for vfiles                                 */
/*                                                                                      */
/*  The contents of this file are subject to the Genesis3D Public License               */
/*  Version 1.01 (the "License"); you may not use this file except in                   */
/*  compliance with the License. You may obtain a copy of the License at                */
/*  http://www.genesis3d.com                                                            */
/*                                                                                      */
/*  Software distributed under the License is distributed on an "AS IS"                 */
/*  basis, WITHOUT WARRANTY OF ANY KIND, either express or implied.  See                */
/*  the License for the specific language governing rights and limitations              */
/*  under the License.                                                                  */
/*                                                                                      */
/*  The Original Code is Genesis3D, released March 25, 1999.                            */
/*  Genesis3D Version 1.1 released November 15, 1999                                 */
/*  Copyright (C) 1999 WildTangent, Inc. All Rights Reserved           */
/*                                                                                      */
/****************************************************************************************/
#ifndef	VFILE__H
#define VFILE__H

#include	"GETypes.h"
#include	"VFile.h"

typedef void      *(GENESISCC *geVFile_FinderCreateFN)(       geVFile *FileSystem, void               *Handle,      const char *FileSpec);
typedef geBoolean  (GENESISCC *geVFile_FinderGetNextFileFN)(  void    *Handle);
typedef geBoolean  (GENESISCC *geVFile_FinderGetPropertiesFN)(void    *Handle,     geVFile_Properties *Properties);
typedef void       (GENESISCC *geVFile_FinderDestroyFN)(      void    *Handle);

typedef void      *(GENESISCC *geVFile_OpenNewSystemFN)(
	      geVFile *FS,
	const char    *Name,
	      void    *Context,
	      uint32   OpenModeFlags
);

typedef geBoolean (GENESISCC *geVFile_UpdateContextFN)(      geVFile *FS,           void               *Handle,       void           *Context,          int        ContextSize);

typedef	void * 	(GENESISCC *geVFile_OpenFN)(
	      geVFile *FS,
	      void    *Handle,
	const char    *Name,
	      void    *Context,
	      uint32   OpenModeFlags
);

typedef geBoolean  (GENESISCC *geVFile_DeleteFileFN)(         geVFile *FS,           void               *Handle, const char           *FileName); 
typedef geBoolean  (GENESISCC *geVFile_RenameFileFN)(         geVFile *FS,           void               *Handle, const char           *FileName,  const char      *NewFileName);
typedef geBoolean  (GENESISCC *geVFile_FileExistsFN)(         geVFile *FS,           void               *Handle, const char           *FileName);
typedef geBoolean  (GENESISCC *geVFile_DisperseFN)(           geVFile *FS,           void               *Handle, const char           *Directory,       geBoolean  Recursive);
typedef void 	  (GENESISCC *geVFile_CloseFN)(              void    *Handle);

typedef geBoolean  (GENESISCC *geVFile_GetSFN)(               void    *Handle,       void               *Buff,         int             MaxSize);
typedef geBoolean  (GENESISCC *geVFile_ReadFN)(               void    *Handle,       void               *Buff,         int             Count);
typedef geBoolean  (GENESISCC *geVFile_WriteFN)(              void    *Handle, const void               *Buff,         int             Count);
typedef geBoolean  (GENESISCC *geVFile_SeekFN)(               void    *Handle,       int                 Where,        geVFile_Whence  Whence);
typedef geBoolean  (GENESISCC *geVFile_EOFFN)(          const void    *Handle);
typedef geBoolean  (GENESISCC *geVFile_TellFN)(         const void    *Handle,       int32              *Position);
typedef geBoolean  (GENESISCC *geVFile_GetPropertiesFN)(const void    *Handle,       geVFile_Properties *Properties);

typedef geBoolean  (GENESISCC *geVFile_SizeFN)(         const void    *Handle,       int32              *Size);
typedef geBoolean  (GENESISCC *geVFile_SetSizeFN)(            void    *Handle,       int32               Size);
typedef geBoolean  (GENESISCC *geVFile_SetAttributesFN)(      void    *Handle,       geVFile_Attributes  Attributes);
typedef geBoolean  (GENESISCC *geVFile_SetTimeFN)(            void    *Handle, const geVFile_Time       *Time);
typedef geBoolean  (GENESISCC *geVFile_SetHintsFN)(           void    *Handle, const geVFile_Hints      *Hints);

typedef	struct	
geVFile_SystemAPIs
{
	geVFile_FinderCreateFN		FinderCreate;
	geVFile_FinderGetNextFileFN	FinderGetNextFile;
	geVFile_FinderGetPropertiesFN FinderGetProperties;
	geVFile_FinderDestroyFN		FinderDestroy;

	geVFile_OpenNewSystemFN		OpenNewSystem;
	geVFile_UpdateContextFN		UpdateContext;
	geVFile_OpenFN				Open;
	geVFile_DeleteFileFN		DeleteFile;
	geVFile_RenameFileFN		RenameFile;
	geVFile_FileExistsFN		FileExists;
	geVFile_DisperseFN			Disperse;
	geVFile_CloseFN				Close;

	geVFile_GetSFN				GetS;
	geVFile_ReadFN				Read;
	geVFile_WriteFN				Write;
	geVFile_SeekFN				Seek;
	geVFile_EOFFN				Eof;
	geVFile_TellFN				Tell;
	geVFile_SizeFN				Size;

	geVFile_GetPropertiesFN		GetProperties;

	geVFile_SetSizeFN			SetSize;
	geVFile_SetAttributesFN		SetAttributes;
	geVFile_SetTimeFN			SetTime;
	geVFile_SetHintsFN			SetHints;
}	
geVFile_SystemAPIs;

geBoolean GENESISCC 
VFile_RegisterFileSystem(
	const geVFile_SystemAPIs     *APIs,
	      geVFile_TypeIdentifier *Type
);

#endif

