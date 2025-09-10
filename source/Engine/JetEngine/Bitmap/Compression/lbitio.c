/****************************************************************************************/
/*  LBITIO.C                                                                            */
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
#include "lbitio.h"

#pragma warning(disable : 4244)

//protos:

struct LBitIOInfo * LBitIO_Init(uint8 *Array);
void LBitIO_CleanUp(struct LBitIOInfo * BII);
long LBitIO_FlushWrite(struct LBitIOInfo * BII); //returns length of data
void LBitIO_ResetArray(struct LBitIOInfo * BII,uint8 *Array);             
long LBitIO_GetPos(struct LBitIOInfo * BII);


const uint32 LBitIOReadBitMask = ((uint32)1)<<31 ;

/***
struct LBitIOInfo
	{
	uint32 BitBuffer;
	int BitsToGo;

	uint8 *BitArray;
	uint8 *BitArrayPtr;
	};
***/

void LBitIO_ResetArray(struct LBitIOInfo * BII,uint8 *Array)
{
BII->BitBuffer = 0;
BII->BitsToGo = 32;

BII->BitArray = Array;
BII->BitArrayPtr = Array;
}

/*
 *	Allocate and init a BII
 *	for reads, LBitIO_InitRead must also be called
 *
 */
struct LBitIOInfo * LBitIO_Init(uint8 *Array)
{
struct LBitIOInfo * BII;

if ( (BII = (struct LBitIOInfo *)jeRam_Allocate(sizeof(struct LBitIOInfo))) == NULL )
	return(NULL);

LBitIO_ResetArray(BII,Array);

return(BII);
}

/*
 *	Free a BII after it has been written or read from
 *	call LBitIO_FlushWrite before writing to a file
 *
 */
void LBitIO_CleanUp(struct LBitIOInfo * BII)
{
destroy(BII);
}

/*
 *  FlushWrite sticks remaining bits into BitArray
 *  must be called before writing BitArray
 *  returns length of array to write
 *
 */
long LBitIO_FlushWrite(struct LBitIOInfo * BII)
{

BII->BitBuffer <<= BII->BitsToGo;

while ( BII->BitsToGo < 32 )
	{
	*BII->BitArrayPtr++ = BII->BitBuffer>>24;
	BII->BitBuffer <<= 8;
	BII->BitsToGo += 8;
	}

BII->BitsToGo = 32;
BII->BitBuffer = 0;
/* keep going, if you like */

return( (long)( BII->BitArrayPtr - BII->BitArray ) );
}

/*
 *  GetPos returns the current BII position, in byte
 *  does not modify the BII at all
 *
 */
long LBitIO_GetPos(struct LBitIOInfo * BII)
{
long Ret;
long z;

	Ret = BII->BitArrayPtr - BII->BitArray;

	z = BII->BitsToGo;

	while ( z < 32 )
	{
		Ret++;
		z += 8;
	}

return(Ret);
}

long LBitIO_GetPosD(struct LBitIOInfo * BII)
{
long Ret;
long z;

	Ret = BII->BitArrayPtr - BII->BitArray;

	z = BII->BitsToGo;

	while ( z >= 8 )
	{
		Ret--;
		z -= 8;
	}

return(Ret);
}

uint8 * LBitIO_GetArray(struct LBitIOInfo * BII)
{
BII->BitBuffer <<= BII->BitsToGo;

while ( BII->BitsToGo < 32 )
	{
	*BII->BitArrayPtr++ = BII->BitBuffer>>24;
	BII->BitBuffer <<= 8;
	BII->BitsToGo += 8;
	}

BII->BitsToGo = 32;
BII->BitBuffer = 0;
/* keep going, if you like */

return( BII->BitArrayPtr );
}

