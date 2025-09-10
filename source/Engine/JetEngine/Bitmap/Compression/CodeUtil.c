/****************************************************************************************/
/*  CODEUTIL.C                                                                          */
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
#include "CodeUtil.h"

#pragma warning( disable:4244 )
#pragma warning( disable:4018 )
// Charles doesn't care about signed/unsigned -Ken


void cu_putEscaping_byte(uint32 val,uint8 **streamp)
{
uint8 *stream = *streamp;
	while ( val >= 0xFF ) 
	{
		*stream++ = val;
		val -= 0xFF;
	}
	*stream++ = val;
*streamp = stream;
}

uint32  cu_getEscaping_byte(uint8 **streamp)
{
uint8 *stream = *streamp;
uint32 ret,val;
	ret = 0;
	do {
		val = *stream++;
		ret += val;
	} while( val == 0xFF );
*streamp = stream;
return ret;
}

void cu_putEscaping_ari(uint32 val,arithInfo *stream,uint32 escape)	/** escape of (1<<escape_bits) **/
{
	while(val >= escape) 
	{
		arithEncode(stream,escape,escape+1,escape+1);
		val -= escape;
	}
	arithEncode(stream,val,val+1,escape+1);
}

uint32  cu_getEscaping_ari(arithInfo *stream,uint32 escape)
{
uint32 ret,val;
	ret = 0;
	do {
		val = arithGet(stream,escape+1);
		arithDecode(stream,val,val+1,escape+1);
		ret += val;
	} while ( val == escape);
return ret;
}

void cu_putExpanding_ari(uint32 val,arithInfo *stream,uint32 init_max,uint32 step_max)
{
uint32 escape;

	escape = init_max;
	while(val >= escape ) 
	{
		arithEncode(stream,escape,escape+1,escape+1);
		val -= escape;
		escape += step_max;
		if ( escape > stream->safeProbMax ) escape = stream->safeProbMax;
	}
	arithEncode(stream,val,val+1,escape+1);
}

uint32  cu_getExpanding_ari(arithInfo *stream,uint32 init_max,uint32 step_max)
{
uint32 escape,ret,val;

	escape = init_max;
	ret = 0;
	for(;;) 
	{
		val = arithGet(stream,escape+1);
		arithDecode(stream,val,val+1,escape+1);
		ret += val;
		if ( val != escape )
			break;
		escape += step_max;
	}

return ret;
}



void cu_putMulting_ari(uint32 val,arithInfo *stream,uint32 init_max,uint32 step_mult)
{
uint32 max;

	max = init_max;
	while ( val >= max ) 
	{
		arithEncBitRaw(stream,1);
		val -= max;
		max *= step_mult;
		if ( max > stream->safeProbMax ) max = stream->safeProbMax;
	}
	arithEncBitRaw(stream,0);
	arithEncode(stream,val,val+1,max);
}

uint32  cu_getMulting_ari(arithInfo *stream,uint32 init_max,uint32 step_mult)
{
uint32 max,ret;

	max = init_max;
	ret = 0;
	for(;;) 
	{
		if ( ! arithDecBitRaw(stream) ) 
		{
			uint32 val;
			val = arithGet(stream,max);
			arithDecode(stream,val,val+1,max);
			return ret+val;
		}
		ret += max;
		max *= step_mult;
		if ( max > stream->safeProbMax ) max = stream->safeProbMax;
	}
}


void cu_putExpandingSigned_ari(int val,arithInfo *stream,int init_max,int step_max)
{
jeBoolean sign;
if ( val < 0 ) { sign = 1; val = -val; } else sign = 0;
arithEncBitRaw(stream,sign);
cu_putExpanding_ari(val,stream,init_max,step_max);
}

int  cu_getExpandingSigned_ari(arithInfo *stream,int init_max,int step_max)
{
int ret;
jeBoolean sign;
sign = arithDecBitRaw(stream);
ret= cu_getExpanding_ari(stream,init_max,step_max);
if ( sign ) return - ret; else return ret;
}

void cu_putMultingSigned_ari(int val,arithInfo *stream,int init_max,int step_mult)
{
jeBoolean sign;
if ( val < 0 ) { sign = 1; val = -val; } else sign = 0;
arithEncBitRaw(stream,sign);
cu_putMulting_ari(val,stream,init_max,step_mult);
}

int  cu_getMultingSigned_ari(arithInfo *stream,int init_max,int step_mult)
{
int ret;
jeBoolean sign;
sign = arithDecBitRaw(stream);
ret= cu_getMulting_ari(stream,init_max,step_mult);
if ( sign ) return - ret; else return ret;
}

void cu_putEscaping_bii(int val,struct LBitIOInfo *stream,int escape_bits)	/** escape of (1<<escape_bits) **/
{
int escape;
	escape = (1<<escape_bits) - 1;
	while(val >= escape) {
		LBitIO_WriteBits(stream,escape,escape_bits);
		val -= escape;
	}
	LBitIO_WriteBits(stream,val,escape_bits);
}

int  cu_getEscaping_bii(struct LBitIOInfo *stream,int escape_bits)
{
int escape,ret,val;
	escape = (1<<escape_bits) - 1;
	ret = 0;
	do {
		LBitIO_ReadBits(stream,val,escape_bits);
		ret += val;
	} while ( val == escape);
return ret;
}

void cu_putExpanding_bii(int val,struct LBitIOInfo *stream,int init_bits,int step_bits)
{
int bits;
uint32 mask;

	bits = init_bits;
	mask = (1<<bits) - 1;
	while(val >= mask ) {
		LBitIO_WriteBits(stream,mask,bits);
		val -= mask;
		bits += step_bits; if ( bits > 31 ) bits = 31;
		mask = (1<<bits) - 1;
	}
	LBitIO_WriteBits(stream,val,bits);
}

int  cu_getExpanding_bii(struct LBitIOInfo *stream,int init_bits,int step_bits)
{
int bits,ret,val;
uint32 mask;

	bits = init_bits;
	ret = 0;
	do {
		mask = (1<<bits) - 1;
		LBitIO_ReadBits(stream,val,bits);
		bits += step_bits;
		ret += val;
	} while( val == mask);

return ret;
}

#pragma warning( default:4244 )
#pragma warning( default:4018 )
