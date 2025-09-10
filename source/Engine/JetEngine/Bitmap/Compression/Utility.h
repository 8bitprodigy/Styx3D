/****************************************************************************************/
/*  Utility.h			                                                                */
/*                                                                                      */
/*  Author: Charles Bloom                                                               */
/*  Description:  Macros				                                                */
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
#ifndef __COMPUTIL_UTILITY_H
#define __COMPUTIL_UTILITY_H

#include "BaseType.h"
#include "Ram.h"
#include "Errorlog.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>	// for memcpy,memset

#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned int uint;

/****************************************/

#define BrandoError(str)	jeErrorLog_AddString(-1,str,NULL)

#ifndef NULL
#define NULL (0)
#endif

#define sizeofpointer sizeof(void *)

#define PaddedSize(a) (((a)+3) & (~3))

#define IsOdd(a)  ( ((uint32)a)&1 )
#define SignOf(a) (((a) < 0) ? -1 : 1)

#ifndef max
#define max(a,b) ((a)>(b)?(a):(b))
#endif

#ifndef min
#define min(a,b) ((a)<(b)?(a):(b))
#endif

#define minmax(x,lo,hi) ( (x)<(lo)?(lo):( (x)>(hi)?(hi):(x)) )
#define putminmax(x,lo,hi) x = minmax(x,lo,hi)
#define putmin(x,lo) x = min(x,lo)
#define putmax(x,hi) x = max(x,hi)
#define max3(a,b,c) max(max(a,b),c)
#define max4(a,b,c,d) max(a,max3(b,c,d))
#define min3(a,b,c) min(min(a,b),c)
#define min4(a,b,c,d) min(a,min3(b,c,d))

#ifndef mabs
#define mabs(i) ((i) < 0 ? -(i) : (i))
#endif

#define isinrange(x,lo,hi)	( (x) >= (lo) && (x) <= (hi) )

#define getuint32(bptr) ( ((((uint8 *)(bptr))[0])<<24) + (((uint8 *)(bptr))[1]<<16) + (((uint8 *)(bptr))[2]<<8) + (((uint8 *)(bptr))[3]) )
#define getuint16(bptr) ( (((uint8 *)(bptr))[0]<<8) + (((uint8 *)(bptr))[1]) )

#define swapints(a,b)	do { (a) ^= (b); (b) ^= (a); (a) ^= (b); } while(0)

/****************************************/

#ifndef strofval
#define strofval(x)	(#x)
#endif

#ifndef new
#define new(type)		jeRam_AllocateClear(sizeof(type))
#endif

#ifndef destroy
#define destroy(mem)	do { if ( mem ) { jeRam_Free(mem); (mem) = NULL; } } while(0)
#endif

#ifndef newarray
#define newarray(type,num)	jeRam_AllocateClear((num)*sizeof(type))
#endif

#ifndef memclear
#define memclear(mem,size)	memset(mem,0,size);
#endif

/****************************************/
// processor-dependent virtual functions :

// you must call SetupUtility() before you use these
//	functions/pointers !!

void SetupUtility(void);

extern void (*cachetouch_r)	(const void * data,int num32s);
extern void (*cachetouch_w)	(const void * data,int num32s);

extern void (*fastmemclear32s)	(void *data,int num32s);
extern void (*fastmemcpy32s)	(char * to,const char *from,int num32s);

#ifdef WIN32
extern void (__fastcall *copy32)(char * to,const char *from);
#endif

#ifdef BUILD_BE
extern void (*copy32)(char * to, const char *from);
#endif

extern void (*copy32_8)(char * to,const char **froms);

void fastmemclear	(char *data,int len);
void fastmemcpy		(char * to,const char *from,int len);

/****************************************/

#ifdef __cplusplus
}
#endif

#endif // __COMPUTIL_UTILITY_H
