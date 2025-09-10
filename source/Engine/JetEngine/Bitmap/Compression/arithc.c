/****************************************************************************************/
/*  ARITHC.C                                                                            */
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
#include "arithc.h"
#include "arithc._h"

#pragma warning (disable : 4244)

/*functions:*/

arithInfo * arithInit(void)
{
arithInfo * ari;

if ( (ari = (arithInfo *)new(arithInfo)) == NULL )
	return(NULL);

ari->probMax = CumProbMax;
ari->probMaxSafe = CumProbMax - 512;

return(ari);
}

void arithFree(arithInfo * ari)
{
if (ari) destroy(ari);
}


void arithEncodeInit(arithInfo * ari,uint8 *outBuf)
{

ari->code  = 0;
ari->range = One;
ari->overflow_bytes = 0;
ari->queue = QUEUE_INIT_VAL;	// this is a waste of a byte

ari->outBuf = ari->outPtr = outBuf;

}

void arithEncodeReInit(arithInfo * ari,uint8 *outBuf)
{

ari->code  = 0;
ari->range = One;
ari->overflow_bytes = 0;
ari->queue = QUEUE_INIT_VAL;	// this is a waste of a byte

// ari->outBuf unchanged
ari->outPtr = outBuf;

}

void ARITHCC arithEncode(arithInfo * ari,uint32 symlow,uint32 symhigh,uint32 symtot)
{
uint32 code,range;

code    = ari->code;
range   = ari->range;

	/** we want to do :
	*		 lowincr = (range * symlow) / symtot
	*	but range & symtot can both be large , so this would overflow the register	
	*	thus we instead do:
	*
	***/

#ifdef FAST_ENCODE
	/*#*/ { uint32 r;

	r = range / symtot;

	code += r * symlow;
	range = r * (symhigh - symlow);

	/*#*/ }
#else
	/*#*/ { uint32 lowincr,r;

	r = range / symtot;

	lowincr = r * symlow;
	code += lowincr;
	if ( symhigh == symtot )	range -= lowincr;
	else 						range = r * (symhigh - symlow);

	/*#*/ }
#endif

	while( range <= MinRange ) {
		uint32 byte;
		byte = code >> SHIFT_BITS;
	
		if ( byte == 0xFF ) {
			/** the waiting queue is incremented like :
			*		(ari->queue), 0xFF, 0xFF, ... ,0xFF, code
			***/

			ari->overflow_bytes++;
		} else {
			uint32 carry;
			carry = code>>CODE_BITS;	

				/* carry == 1 or 0 : is the top bit on ?
				*	carry = byte>>8
				* if ( carry )	send nextbyte+1
				*				MinRange queue with zeros
				* else			send nextbyte
				*				MinRange queue with ones
				**/					
		
			*(ari->outPtr)++ = (ari->queue) + carry;	// propagate the carry.
			// send the queue
			if ( ari->overflow_bytes ) {
				*(ari->outPtr)++ = (uint8)(0xFF + carry);
				while ( --(ari->overflow_bytes) ) *(ari->outPtr)++ = (uint8)(0xFF + carry);
			}
			ari->queue = byte;
		}

		code = (code<<8) & CODE_MASK;
		range <<= 8;
	}

ari->code  = code;
ari->range = range;
}


uint32 arithEncodeDone(arithInfo * ari)
{

	/*****

	the minimal way is to do :

		code += MSB(range)

		clear ( code below MSB of range )
		( except when code and range are both already clear below the MSB of range )

	eg. if range is 67 we do :
	
		code += 64;
		code &= ~63;

	then we just send code bytes until the remainder is zero.

	******/

	ari->code += (ari->range)>>1;

	/* first send the queue */

	if ( ari->code & One ) {
		*(ari->outPtr)++ = ari->queue + 1;
		while ( ari->overflow_bytes-- ) *(ari->outPtr)++ = 0;
	} else {
		*(ari->outPtr)++ = ari->queue;
		while ( ari->overflow_bytes-- ) *(ari->outPtr)++ = 0xFF;
	}

	*(ari->outPtr)++ = (ari->code >> SHIFT_BITS) & 0xFF;	ari->code <<= 8;
	*(ari->outPtr)++ = (ari->code >> SHIFT_BITS) & 0xFF;	ari->code <<= 8;
	#ifdef PARANOID_ENCODEDONE
	*(ari->outPtr)++ = (ari->code >> SHIFT_BITS) & 0xFF;	ari->code <<= 8;
	*(ari->outPtr)++ = (ari->code >> SHIFT_BITS) & 0xFF;	ari->code <<= 8;
	#endif

return ari->outPtr - ari->outBuf;
}

void arithDecodeInit(arithInfo * ari,uint8 *outBuf)
{
uint8 byte;

	ari->outBuf = ari->outPtr = outBuf;

	byte = *(ari->outPtr)++;

	assert( byte == QUEUE_INIT_VAL );

	/** the code needs to be kept filled with 31 bits ;
	*	this means we cannot just read in 4 bytes.  We must read in 3,
	*	then the 7 bits of another (EXTRA_BITS == 7) , and save that last
	*	bit in the queue 
	**/

	byte = ari->queue = *(ari->outPtr)++;
	ari->code = byte >> (TAIL_EXTRA_BITS);
	ari->range = 1 << EXTRA_BITS;
}

void arithDecodeReInit(arithInfo * ari,uint8 *outBuf)
{
uint32 byte;

	// ari->outBuf
	ari->outPtr = outBuf + 1;
	byte = ari->queue = *(ari->outPtr)++;
	ari->code = byte >> (TAIL_EXTRA_BITS);
	ari->range = 1 << EXTRA_BITS;
}

/** save a repeated divide computation between arithGet and arithDecode **/
static uint32 dec_range_over_symtot;

uint32 ARITHCC arithGet(arithInfo * ari,uint32 symtot)
{
uint32 ret,range,code;

	range = ari->range;
	code = ari->code;

/** the code needs to be kept filled with 31 bits ;
*	this means we cannot just read in 4 bytes.  We must read in 3,
*	then the 7 bits of another (EXTRA_BITS == 7) , and save that last
*	bit in the queue 
*
***/

	while ( range <= MinRange ) {
		range <<= 8;
		code = (code<<8) + ((ari->queue<<EXTRA_BITS)&0xFF);	// use the top bit in the queue
		ari->queue = *(ari->outPtr)++;
		code += (ari->queue) >> (TAIL_EXTRA_BITS);
	}

	dec_range_over_symtot = range / symtot;
	ret = code / dec_range_over_symtot;
	ret = ( ret >= symtot ? symtot-1 : ret ); //<> is this really necessary?

ari->range = range;
ari->code  = code;

return ret;
}

void ARITHCC arithDecode(arithInfo * ari,uint32 symlow,uint32 symhigh,uint32 symtot)
{

#ifdef FAST_ENCODE

	ari->code -= dec_range_over_symtot * symlow;
	ari->range = dec_range_over_symtot * (symhigh - symlow);

#else

uint32 lowincr;

	lowincr = dec_range_over_symtot * symlow;
	ari->code -= lowincr;
	if ( symhigh == symtot )	ari->range -= lowincr;
	else 						ari->range = dec_range_over_symtot * (symhigh - symlow);

#endif

}

void arithDecodeDone(arithInfo * ari)
{
}

uint32 arithTellEncPos(arithInfo * ari)
{
uint32 ret;

	ret = ari->outPtr - ari->outBuf;
	ret += 1 + ari->overflow_bytes;	// # of bytes in the queue
	ret += 2;	// flush bytes
return ret;
}
uint32 arithTellDecPos(arithInfo * ari)
{
uint32 ret;
	ret = ari->outPtr - ari->outBuf;
	ret -= CODE_BYTES;
	ret += 2;
return ret;
}

uint32 arithEncodeFlush(arithInfo * ari)
{
uint32 ret;
uint8 *saveBuf;
ret = arithEncodeDone(ari);
saveBuf = ari->outBuf;
arithEncodeInit(ari,saveBuf+ret);
ari->outBuf = saveBuf;
return ret;
}

void arithDecodeRestart(arithInfo * ari,uint32 pos)
{
uint8 *saveBuf;
arithDecodeDone(ari);
saveBuf = ari->outBuf;
arithDecodeInit(ari,saveBuf+pos);
ari->outBuf = saveBuf;
}


void  arithEncByteRaw(arithInfo * ari,uint32 byte)
{
	assert(byte >= 0 && byte < 256);
	arithEncode(ari,byte,byte+1,256);
}

uint32 arithDecByteRaw(arithInfo * ari)
{
uint32 got;
uint32 range,code;
uint32 dec_range_over_symtot;

	range = ari->range;
	code = ari->code;

	while ( range <= MinRange ) {
		range <<= 8;
		code = (code<<8) + ((ari->queue<<EXTRA_BITS)&0xFF);	// use the top bit in the queue
		ari->queue = *(ari->outPtr)++;
		code += (ari->queue) >> (TAIL_EXTRA_BITS);
	}

	dec_range_over_symtot = range >> 8;
	got = code / dec_range_over_symtot;
	got = ( got >= 256 ? 255 : got ); //<> is this really necessary?

#ifdef FAST_ENCODE

	code -= dec_range_over_symtot * got;
	range = dec_range_over_symtot;

#else

{
uint32 lowincr;

	lowincr = dec_range_over_symtot * got;
	code -= lowincr;
	if ( got == 255 )	range -= lowincr;
	else 				range = dec_range_over_symtot;
}

#endif	

ari->range = range;
ari->code  = code;

return got;
}

void arithEncBit(arithInfo * ari,uint32 mid,uint32 tot,jeBoolean bit)
{
uint32 code,range;

code    = ari->code;
range   = ari->range;

	/*#*/ { uint32 r;

	r = (range / tot) * mid;

	if ( bit ) {
		code += r;
		range -= r;
	} else {
		range = r;
	}
	/*#*/ }

/** the same flush as normal Encode **/

	while( range <= MinRange ) {
		uint32 byte;
		byte = code >> SHIFT_BITS;
	
		if ( byte == 0xFF ) {
			ari->overflow_bytes++;
		} else {
			uint32 carry;
			carry = code>>CODE_BITS;	
			*(ari->outPtr)++ = (ari->queue) + carry;
			ari->queue = byte;

			if ( ari->overflow_bytes ) {
				*(ari->outPtr)++ = (uint8)(0xFF + carry);
				while ( --(ari->overflow_bytes) ) *(ari->outPtr)++ = (uint8)(0xFF + carry);
			}
		}

		code = (code<<8) & CODE_MASK;
		range <<= 8;
	}

ari->code  = code;
ari->range = range;
}

jeBoolean arithDecBit(arithInfo * ari,uint32 mid,uint32 tot)
{
jeBoolean bit;
uint32 range,code,r;

	range = ari->range;
	code = ari->code;

	while ( range <= MinRange ) {
		range <<= 8;
		code = (code<<8) + (((ari->queue)<<EXTRA_BITS)&0xFF);	// use the top bit in the queue
		ari->queue = *(ari->outPtr)++;
		code += (ari->queue) >> (TAIL_EXTRA_BITS);
	}

/**
*
*		r = (range / tot)
*
*	if ( (code / r) >= mid ) bit = 1; else bit = 0;
*
*	we eliminate one divide. this is the savings of the binary coder
*
**/

	r = (range / tot) * mid;

	if ( code >= r ) {
		bit = 1;
		code -= r;
		range -= r;
	} else {
		bit = 0;
		range = r;
	}

ari->range = range;
ari->code  = code;

return bit;
}

#define BITMODEL_TOTMAX 5000
#define BITMODEL_INC	30

void arithModelEncBit(arithInfo * ari,uint32 *p0,uint32 *pt,jeBoolean bit)
{
uint32 code,range;

code    = ari->code;
range   = ari->range;

	do {
	uint32 r;

		r = (range / (*pt)) * (*p0);

		if ( bit ) {
			code += r;
			range -= r;
		} else {
			range = r;
			(*p0) += BITMODEL_INC;
		}
	} while(0);

	(*pt) += BITMODEL_INC;
	if ( *pt > BITMODEL_TOTMAX ) {
		*p0 >>= 1; *pt >>= 1; 
		(*p0) ++; (*pt) += 2; 
	}

/** the same flush as normal Encode **/

	while( range <= MinRange ) {
		uint32 byte;
		byte = code >> SHIFT_BITS;
	
		if ( byte == 0xFF ) {
			ari->overflow_bytes++;
		} else {
			uint32 carry;
			carry = code>>CODE_BITS;	
			*(ari->outPtr)++ = (ari->queue) + carry;
			ari->queue = byte;

			if ( ari->overflow_bytes ) {
				*(ari->outPtr)++ = (uint8)(0xFF + carry);
				while ( --(ari->overflow_bytes) ) *(ari->outPtr)++ = (uint8)(0xFF + carry);
			}
		}

		code = (code<<8) & CODE_MASK;
		range <<= 8;
	}

ari->code  = code;
ari->range = range;
}

jeBoolean arithModelDecBit(arithInfo * ari,uint32 *p0,uint32 *pt)
{
jeBoolean bit;
uint32 range,code,r;

	range = ari->range;
	code = ari->code;

	while ( range <= MinRange ) {
		range <<= 8;
		code = (code<<8) + (((ari->queue)<<EXTRA_BITS)&0xFF);	// use the top bit in the queue
		ari->queue = *(ari->outPtr)++;
		code += (ari->queue) >> (TAIL_EXTRA_BITS);
	}

	r = (range / (*pt)) * (*p0);

	if ( code >= r ) {
		bit = 1;
		code -= r;
		range -= r;
	} else {
		bit = 0;
		range = r;
		(*p0) += BITMODEL_INC;
	}
	(*pt) += BITMODEL_INC;
	if ( (*pt) > BITMODEL_TOTMAX ) {
		(*p0) >>= 1; (*pt) >>= 1; 
		(*p0) ++; (*pt) += 2; 
	}

ari->range = range;
ari->code  = code;

return bit;
}


void ARITHCC arithEncBitRaw(arithInfo * ari,jeBoolean bit)
{
uint32 code,range;

code    = ari->code;
range   = ari->range;

	if ( bit ) {
		code	+= range >> 1;
		range	-= range >> 1;
	} else {
		range >>= 1;
	}

	while( range <= MinRange ) {
		uint32 byte;
		byte = code >> SHIFT_BITS;
	
		if ( byte == 0xFF ) {
			ari->overflow_bytes++;
		} else {
			uint32 carry;
			carry = code>>CODE_BITS;	
			*(ari->outPtr)++ = (ari->queue) + carry;
			ari->queue = byte;

			if ( ari->overflow_bytes ) {
				*(ari->outPtr)++ = (uint8)(0xFF + carry);
				while ( --(ari->overflow_bytes) ) *(ari->outPtr)++ = (uint8)(0xFF + carry);
			}
		}

		code = (code<<8) & CODE_MASK;
		range <<= 8;
	}

ari->code  = code;
ari->range = range;
}

jeBoolean ARITHCC arithDecBitRaw(arithInfo * ari)
{
jeBoolean bit;
uint32 range,code,r;

	range = ari->range;
	code = ari->code;

	while ( range <= MinRange ) {
		range <<= 8;
		code = (code<<8) + (((ari->queue)<<EXTRA_BITS)&0xFF);	// use the top bit in the queue
		ari->queue = *(ari->outPtr)++;
		code += (ari->queue) >> (TAIL_EXTRA_BITS);
	}

	r = range >> 1;

	if ( code >= r ) {
		bit = 1;
		code	-= r;
		range	-= r;
	} else {
		bit = 0;
		range = r;
	}

ari->range = range;
ari->code  = code;

return bit;
}
