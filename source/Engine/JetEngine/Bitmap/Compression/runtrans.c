/****************************************************************************************/
/*  RUNTRANS.C                                                                          */
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
#include "Utility.h"
#include "arithc.h"
#include "o0coder.h"

#define RT_FLAGLEN	8

int doRunTransform(uint8 *raw,int rawLen,uint8 *comp)
{
uint8 *rptr,*cptr,*rptr_done;
int gotc,lastc,runner;

rptr = raw; cptr = comp;
rptr_done = rptr + rawLen;
lastc = -1; runner = 0;

while ( rptr < rptr_done ) {
	*cptr++ = gotc = *rptr++;
	if ( gotc == lastc ) runner++;
	else { runner = 1; lastc = gotc; }

	if ( runner == RT_FLAGLEN ) {
		runner = 0;
		while( (gotc = *rptr++) == lastc && rptr < rptr_done ) {
			if ( ++runner == 0xFF ) {
				*cptr++ = 0xFF; runner = 0;
			}
		}
		*cptr++ = runner;
		*cptr++ = gotc;
		runner = 1; lastc = gotc;
	}
}

return cptr - comp;
}

int unRunTransform(uint8 *raw,int rawLen,uint8 *comp)
{
uint8 *rptr,*cptr,*rptr_done;
int gotc,lastc,runner;

rptr = raw; cptr = comp;
rptr_done = rptr + rawLen;
lastc = -1; runner = 0;

while ( rptr < rptr_done ) {
	gotc = *cptr++;
	if ( gotc == lastc ) {
		*rptr++ = lastc;
		runner++;
		if ( runner == RT_FLAGLEN ) {
			while ( (runner = *cptr++) == 0xFF ) {
				while(runner--) *rptr++ = lastc;
			}
			while(runner--) *rptr++ = lastc;
			runner = 0; lastc = -1;
		}
	} else {
		lastc = *rptr++ = gotc;
		runner = 1;
	}
}

return cptr - comp;
}

#define RP_MaxLength	64
#define RP_NumLengths	(RP_MaxLength+1)
#define RP_FlagLen		11 /* <> try 8 instead */

jeBoolean RunPack(uint8 *Array,uint32 ArrayLen,uint8 * Literals,
  uint32 * NumLiteralsPtr,uint8 * PackedRunArray,uint32 * NumRunsPtr,
  uint32 * RunPackedLenPtr)
{
uint8 *ArrayEnd,*CurArrayPtr,*LiteralsPtr;
int LastC,GotC,Runner,NumRuns;
arithInfo * ari;
ozero * O0I;

if ( (ari = arithInit()) == NULL ) return 0;
arithEncodeInit(ari,PackedRunArray); 

if ( (O0I = O0coder_Init(ari,RP_NumLengths)) == NULL )
  { arithFree(ari); return(0); }

NumRuns = 0;
LiteralsPtr = Literals;
CurArrayPtr = Array;
ArrayEnd = Array + ArrayLen;

LastC = -1; Runner = 0;
while ( CurArrayPtr < ArrayEnd )
  {
  *LiteralsPtr++ = GotC = *CurArrayPtr++;
  if ( GotC == LastC ) Runner++;
  else { Runner = 1; LastC = GotC; }

  if ( Runner == RP_FlagLen )
    {
    Runner = 0;
    while( (GotC = *CurArrayPtr++) == LastC )
      {
      if ( ++Runner == RP_MaxLength )
        { O0coder_EncodeC(O0I,Runner); NumRuns++; Runner = 0; }
      if ( CurArrayPtr == ArrayEnd )
        { O0coder_EncodeC(O0I,Runner); NumRuns++; goto RP_Done; }
      }
    O0coder_EncodeC(O0I,Runner); NumRuns++;
    *LiteralsPtr++ = GotC;
    Runner = 1; LastC = GotC;
    }
  }

RP_Done:

*RunPackedLenPtr = arithEncodeDone(ari);

O0coder_CleanUp(O0I); arithFree(ari);

*NumRunsPtr = NumRuns;
*NumLiteralsPtr = LiteralsPtr - Literals;

return(1);
}

jeBoolean UnRunPack(uint8 *Array,uint32 ArrayLen,uint8 * Literals,
  uint8 * PackedRunArray)
{
uint8 *LiteralsPtr,*ArrayPtrEnd,*CurArrayPtr;
int LastC,GotC,Runner;
arithInfo * ari;
ozero * O0I;

if ( (ari = arithInit()) == NULL ) return(0);

arithDecodeInit(ari,PackedRunArray);

if ( (O0I = O0coder_Init(ari,RP_NumLengths)) == NULL )
  { arithFree(ari); return(0); }

CurArrayPtr = Array;
LiteralsPtr = Literals;
ArrayPtrEnd = Array + ArrayLen;

LastC = -1; Runner = 0;
while( CurArrayPtr < ArrayPtrEnd )
  {
  GotC = *LiteralsPtr++;
  if ( GotC == LastC )
    {
    *CurArrayPtr++ = LastC;
    Runner++;
    if ( Runner == RP_FlagLen )
      {
      while( (Runner = O0coder_DecodeC(O0I)) == RP_MaxLength )
        {
        while(Runner--) *CurArrayPtr++ = LastC;
        }
      while(Runner--) *CurArrayPtr++ = LastC;
      Runner = 0; LastC = -1;
      }
    }
  else
    {
    *CurArrayPtr++ = GotC;
    LastC = GotC; Runner = 1;
    }
  }

arithDecodeDone(ari);

O0coder_CleanUp(O0I); arithFree(ari);

return(1);
}
