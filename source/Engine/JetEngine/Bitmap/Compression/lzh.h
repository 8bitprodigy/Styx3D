/****************************************************************************************/
/*  LZH.H                                                                               */
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
#ifndef	JE_LZH_H
#define JE_LZH_H

#ifdef __cplusplus
extern "C" {
#endif

extern void lzhEncode(char *rawArray,int rawLen,
						char ** compArrayPtr,int * compLenPtr);
extern void lzhDecode(char *compArray,int compLen,
						char ** rawArrayPtr ,int * rawLenPtr);

/*******
*
	arrays passed in to lzh should be DWORD aligned.
	
	we appMalloc the return array for you.  For example :


MemoryBuffer * lzhEncodeMemoryBuffer(MemoryBuffer *mbIn)
{
	MemoryBuffer * mbOut;
		mbOut = new(MemoryBuffer);
		lzhEncode( mbIn->data , mbIn->len , &(mbOut->data), &(mbOut->len) );
	return mbOut;
}


*
********/

#ifdef __cplusplus
}
#endif

#endif

