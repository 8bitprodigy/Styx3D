/****************************************************************************************/
/*  OGGStream.h                                                                         */
/*                                                                                      */
/*  Author: Anthony Rufrano                                                             */
/*  Description: OGG Vorbis Stream Code                                                 */
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
#ifndef OGG_STREAM_H
#define OGG_STREAM_H

#include <windows.h>
#include <dsound.h>

#include "BaseType.h"
#include "VFile.h"
#include "vorbis/codec.h"
#include "vorbis/vorbisfile.h"

#define MAX_FILE_NAME						255

typedef struct jeOGGStream					jeOGGStream;

typedef struct jeOGGStream
{
	char									FileName[MAX_FILE_NAME];
	jeVFile									*File;

	IDirectSoundBuffer8						*pBuffer;

	OggVorbis_File							VorbisFile;
	vorbis_info								*VorbisInfo;

	uint8									*BufferPtr;
	uint8									*Buffer;
	uint32									BufferSize;
	uint32									NumSamples;

	jeBoolean								IsEOF;
	WAVEFORMATEX							Format;

} jeOGGStream;

jeOGGStream									*jeOGGStream_Create(jeVFile *FS, const char *filename);
void										jeOGGStream_Destroy(jeOGGStream **OGG);

uint32										jeOGGStream_Read(jeOGGStream *OGG, char *buffer, uint32 size);
void										jeOGGStream_Reset(jeOGGStream *OGG);

uint32										jeOGGStream_GetSize(jeOGGStream *OGG);
jeBoolean									jeOGGStream_IsEOF(jeOGGStream *OGG);

#endif
