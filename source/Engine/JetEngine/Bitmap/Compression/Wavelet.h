/****************************************************************************************/
/*  WAVELET.H                                                                           */
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
#ifndef WAVELET_H
#define WAVELET_H

#include "BaseType.h"
#include "Bitmap.h"
#include "ThreadQueue.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct jeWavelet jeWavelet;
typedef struct jeWavelet_Options jeWavelet_Options;

struct jeWavelet_Options
{
	int transformN;
	int coderN;
	jeBoolean transposeLHs;
	float ratio;
	jeBoolean tblock;
};

jeWavelet * jeWavelet_Create(const jeBitmap_Info * Info,const void * Bits,const jeBitmap * Bmp,
									const jeWavelet_Options * opts);
void		jeWavelet_CreateRef(jeWavelet * w);
jeWavelet * jeWavelet_CreateEmpty(int width,int height);
jeWavelet * jeWavelet_CreateFromFile(jeBitmap * Bmp,jeVFile * File);
jeWavelet * jeWavelet_CreateFromBitmap(const jeBitmap * Bmp,const jeWavelet_Options * opts);
void 		jeWavelet_Destroy(jeWavelet ** pW);

void		jeWavelet_GetInfo( const jeWavelet *w, jeBitmap_Info * Info);
jeBoolean	jeWavelet_HasAlpha(const jeWavelet *w);

jeBoolean	jeWavelet_Compress(jeWavelet * w,const jeBitmap_Info * Info,const void * Bits,
									const jeBitmap * Bmp,const jeWavelet_Options * opts);
jeBoolean	jeWavelet_WriteToFile(const jeWavelet * W,jeVFile * File);

jeBoolean	jeWavelet_CanDecompressMips(const jeWavelet *w,const jeBitmap_Info * ToInfo );
jeBoolean	jeWavelet_Decompress(const jeWavelet * w,const jeBitmap_Info * Info,void * Bits);
jeBoolean	jeWavelet_DecompressMips(const jeWavelet * W,const jeBitmap_Info ** InfoArray,const void ** BitsArray,uint32 MipLow,uint32 MipHigh);

jeBoolean	jeWavelet_SetOptions(jeWavelet_Options *opts,int clevel,jeBoolean NeedMips,jeFloat ratio);

jeBoolean	jeWavelet_SetExpertOptions(jeWavelet_Options *opts,jeFloat Ratio,int TransformN,int CoderN,jeBoolean TransposeLHs,jeBoolean Block);

const char *jeWavelet_GetOptionsDescription(void);

jeBoolean	jeWavelet_ShouldDecompressStreaming(const jeWavelet * W);

void		jeWavelet_CheckStreaming(const jeWavelet * W);
void		jeWavelet_WaitStreaming( const jeWavelet * W);

jeThreadQueue_Job * jeWavelet_StreamingJob(const jeWavelet *W);

#ifdef __cplusplus
}
#endif


#endif


