/****************************************************************************************/
/*  OGGStream.c                                                                         */
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
#include <assert.h>
#include <string.h>
#include "OGGStream.h"
#include "Ram.h"

static size_t vorbis_read_func(void *ptr, size_t size, size_t nmemb, void *datasource)
{
	int32					pos, newpos;
	jeVFile					*File = (jeVFile*)datasource;
	size_t					bytestoread;

	if (!size || !nmemb)
		return 0;

	bytestoread = size * nmemb;

	jeVFile_Tell(File, &pos);
	jeVFile_Read(File, ptr, bytestoread);
	jeVFile_Tell(File, &newpos);

	if ((newpos - pos) == bytestoread)
		return bytestoread;
	
	return bytestoread - (newpos - pos);
}

static int vorbis_seek_func(void *datasource, ogg_int64_t offset, int whence)
{
	jeVFile						*File = (jeVFile*)datasource;
	jeVFile_Whence				w;

	switch (whence)
	{
	case SEEK_SET:
		{
			w = JE_VFILE_SEEKSET;
			break;
		}
	case SEEK_CUR:
		{
			w = JE_VFILE_SEEKCUR;
			break;
		}
	case SEEK_END:
		{
			w = JE_VFILE_SEEKEND;
			break;
		}
	default:
		return 0;
	}

	return jeVFile_Seek(File, (int)offset, w);
}

static int vorbis_close_func(void *datasource)
{
	jeVFile					*File = (jeVFile*)datasource;

	jeVFile_Close(File);
	return 0;
}

static long vorbis_tell_func(void *datasource)
{
	jeVFile					*File = (jeVFile*)datasource;
	int32					t;

	jeVFile_Tell(File, &t);
	return t;
}

jeOGGStream *jeOGGStream_Create(jeVFile *FS, const char *filename)
{
	jeOGGStream				*ogg = NULL;
	ov_callbacks			ov;

	ogg = (jeOGGStream*)JE_RAM_ALLOCATE_STRUCT(jeOGGStream);
	if (!ogg)
		return NULL;

	memset(ogg, 0, sizeof(jeOGGStream));

	if (FS != NULL)
	{
		ogg->File = jeVFile_Open(FS, filename, JE_VFILE_OPEN_READONLY);
	}
	else
	{
		ogg->File = jeVFile_OpenNewSystem(NULL, JE_VFILE_TYPE_DOS, filename, NULL, JE_VFILE_OPEN_READONLY);
	}

	if (!ogg->File)
	{
		jeRam_Free(ogg);
		ogg = NULL;
		return NULL;
	}

	ov.read_func = vorbis_read_func;
	ov.seek_func = vorbis_seek_func;
	ov.close_func = vorbis_close_func;
	ov.tell_func = vorbis_tell_func;

	if (ov_open_callbacks(ogg->File, &ogg->VorbisFile, NULL, 0, ov) < 0)
	{
		jeVFile_Close(ogg->File);
		ogg->File = NULL;

		jeRam_Free(ogg);
		ogg = NULL;
		
		return NULL;
	}

	ogg->VorbisInfo = ov_info(&ogg->VorbisFile, -1);
	ogg->NumSamples = (uint32)ov_pcm_total(&ogg->VorbisFile, -1);
	
	ogg->Format.wFormatTag = WAVE_FORMAT_PCM;
	ogg->Format.nChannels = ogg->VorbisInfo->channels;
	ogg->Format.nSamplesPerSec = ogg->VorbisInfo->rate;
	ogg->Format.wBitsPerSample = 16;
	ogg->Format.nBlockAlign = ogg->Format.nChannels * ogg->Format.wBitsPerSample / 8;
	ogg->Format.nAvgBytesPerSec = ogg->Format.nSamplesPerSec * ogg->Format.nBlockAlign;
	ogg->Format.cbSize = 0;

	ogg->Buffer = NULL;
	ogg->BufferPtr = NULL;
	ogg->BufferSize = 0;
	//strcpy(ogg->FileName, filename);
	strcpy_s(ogg->FileName, filename);
	ogg->IsEOF = JE_FALSE;
	
	return ogg;
}

void jeOGGStream_Destroy(jeOGGStream **OGG)
{
	assert(OGG != NULL);

	ov_clear(&(*OGG)->VorbisFile);

	if ((*OGG)->File)
		jeVFile_Close((*OGG)->File);

	jeRam_Free((*OGG));
	(*OGG) = NULL;
}

uint32 jeOGGStream_Read(jeOGGStream *OGG, char *buffer, uint32 size)
{
	char					*pBuffer = buffer;
	uint32					bytes_read = 0;
	int						section = 0;

	assert(OGG != NULL);

	while (bytes_read < size && !OGG->IsEOF)
	{
		int32				ret;

		ret = ov_read(&OGG->VorbisFile, pBuffer, size - bytes_read, 0, 2, 1, &section);
		if (ret == 0 || section != 0)
			OGG->IsEOF = JE_TRUE;
		else if (ret < 0)
			return 0;
		
		bytes_read += ret;
		pBuffer += ret;
	}

	return bytes_read;
}

void jeOGGStream_Reset(jeOGGStream *OGG)
{
	assert(OGG != NULL);

	OGG->IsEOF = JE_FALSE;
	ov_pcm_seek(&OGG->VorbisFile, 0);
}

uint32 jeOGGStream_GetSize(jeOGGStream *OGG)
{
	assert(OGG != NULL);

	return OGG->NumSamples * OGG->Format.nChannels * OGG->Format.wBitsPerSample / 8;
}

jeBoolean jeOGGStream_IsEOF(jeOGGStream *OGG)
{
	assert(OGG != NULL);

	return OGG->IsEOF;
}
