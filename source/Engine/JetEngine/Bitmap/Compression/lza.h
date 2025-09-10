/****************************************************************************************/
/*  LZA.H                                                                               */
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
#ifndef JE_LZA_H
#define JE_LZA_H

#include "BaseType.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct lzaDecoder lzaDecoder;

extern void lzaEncode(uint8 *rawArray,uint32 rawLen,uint8 **compArrayPtr,uint32 * compLenPtr);

extern lzaDecoder * lzaDecoder_Create(uint8 *compArray,uint32 TotCompLen,uint32 CurCompLen,uint8 * rawArray,int rawLen);
extern jeBoolean	lzaDecoder_Extend(lzaDecoder * Stream,uint32 AddCompLen,uint32 * pCurAvailable);
extern void			lzaDecoder_Destroy(lzaDecoder ** pStream);

/*******
*
	arrays passed in to lza should be DWORD aligned.
	
	we appMalloc the return array for you.  For example :


MemoryBuffer * lzaEncodeMemoryBuffer(MemoryBuffer *mbIn)
{
	MemoryBuffer * mbOut;
		mbOut = new(MemoryBuffer);
		lzaEncode( mbIn->data , mbIn->len , &(mbOut->data), &(mbOut->len) );
	return mbOut;
}


*
********/

#ifdef __cplusplus
}
#endif

#endif

