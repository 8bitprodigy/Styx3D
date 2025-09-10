//WARNING: This file is incomplete and should not be used. I may return later to finish this code
//		   But for now, dont use this file in the engine (unless some kind soul finishes it for me)
//															--Timothy Roff

/*#include <windows.h>
#include <vfw.h>
#include "jet.h"
#include "avifile.h"

PAVIFILE pAviFile;
AVIFILEINFO file_info;
AVISTREAMINFO infoAudio;

int nNumAudioStreams=0, nNumVideoStreams=0;
PGETFRAME pgf;
LPBITMAPINFOHEADER lpbi;
PAVISTREAM pAudio[MAX_AUDIO_STREAMS];
PAVISTREAM pVideo[MAX_VIDEO_STREAMS];


jeBoolean jeAVI_Init()
{
	//AviFileInit();

	return JE_TRUE;
}


jeBoolean jeAVI_Open(char *file)
{
	LONG lSize; // in bytes

	if(AVIFileOpen(&pAviFile, file, OF_READ, NULL))
	{
		// error
	}
	
	AVIFileInfo(pAviFile, &file_info, sizeof(file_info));
	
	do {
		if(AVIFileGetStream(pAviFile, &pAudio[nNumAudioStreams], streamtypeAUDIO, nNumAudioStreams))
		{
			break;
		}
	
	} while(++nNumAudioStreams < MAX_AUDIO_STREAMS);

	do {
		if(AVIFileGetStream(pAviFile, &pVideo[nNumVideoStreams], streamtypeVIDEO, nNumVideoStreams))
		{
			break;
		}
	} while(++nNumVideoStreams < MAX_VIDEO_STREAMS);
	
	if(AVIStreamInfo(pAudio, &infoAudio, sizeof(info)))
	{
		// error
	}

	if(AVIStreamReadFormat(pAudio, AVIStreamStart(pStream), NULL, &lSize))
	{
		// error
	}

	LPBYTE pChunk = new BYTE[lSize];
	if(!pChunk)
	{
		// allocation error
	}

	if(AVIStreamReadFormat(pAudio, AVIStreamStart(pStream), pChunk, &lSize))
	{
		// error
	}

	LPWAVEFORMAT pWaveFormat = (LPWAVEFORMAT)pChunk;

	//start vidstreams
	LPBYTE pChunk = new BYTE[lSize];
	if(!pChunk)
	{
		// allocation error
	}
	
	if(AVIStreamReadFormat(pVideo, AVIStreamStart(pStream), pChunk, lSize))
	{
		// error
	}

	return JE_TRUE;
}

//audio
LONG lSize;
if(AVIStreamRead(pStream, 0, AVISTREAMREAD_CONVENIENT,
   NULL, 0, &lSize, NULL))
    // error
LPBYTE pBuffer = new BYTE[lSize];
if(!pBuffer)
    // error

if(AVIStreamRead(pStream, 0, AVISTREAMREAD_CONVENIENT,
   pBuffer, lSize, NULL, NULL))
    // error


//video
pgf = AVIStreamGetFrameOpen(pStream, NULL);
if(!pgf)
    // error

// Precalculated: When stream is opened
lEndTime = AVIStreamEndTime(pStream);

// Calculated just before next frame is blitted
if(lTime <= lEndTime)
    lFrame = AVIStreamTimeToSample(pStream, lTime);
else // the video is done


With that information, it’s easy to pluck a packed DIB from the video stream. 



lpbi = (LPBITMAPINFOHEADER)AVIStreamGetFrame(pgf, lFrame);


The packed DIB is comprised of a BITMAPINFOHEADER structure, followed by the palette information if needed, and then followed by the bitmap data. All of this is just one sequential block of memory, so it’s possible to calculate the palette and bitmap pointers using pointer arithmetic.


// For 16, 24, or 32bit image formats
LPBYTE pData = lpbi + lpbi->biSize;

// For 8bit image formats
LPBYTE pData = lpbi + lpbi->biSize + 256 * sizeof(RGBQUAD);


The image data that has been extracted is only good until the next time we call AVIStreamGetFrame, so it’s important to display it to screen, copy it to a texture or bmp file, or whatever you want to do with it. I don’t go into such details here, but you’ll see how in the provided sample code.

When all of the frames have been processed, it’s important to close down the decompression system as follows.


if(AVIStreamGetFrameClose(pgf))
    // error
*/