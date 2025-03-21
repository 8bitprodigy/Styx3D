/****************************************************************************************/
/*  BASETYPE.H                                                                          */
/*                                                                                      */
/*  Author:                                                                             */
/*  Description: Basic type definitions and calling convention defines                  */
/*                                                                                      */
/*  The contents of this file are subject to the Genesis3D Public License               */
/*  Version 1.01 (the "License"); you may not use this file except in                   */
/*  compliance with the License. You may obtain a copy of the License at                */
/*  http://www.genesis3d.com                                                            */
/*                                                                                      */
/*  Software distributed under the License is distributed on an "AS IS"                 */
/*  basis, WITHOUT WARRANTY OF ANY KIND, either express or implied.  See                */
/*  the License for the specific language governing rights and limitations              */
/*  under the License.                                                                  */
/*                                                                                      */
/*  The Original Code is Genesis3D, released March 25, 1999.                            */
/*  Genesis3D Version 1.1 released November 15, 1999                                 */
/*  Copyright (C) 1999 WildTangent, Inc. All Rights Reserved           */
/*                                                                                      */
/****************************************************************************************/
#ifndef GE_BASETYPE_H
#define GE_BASETYPE_H
 
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/******** The Genesis Calling Conventions ***********/ 

#if defined(_MSC_VER)  
  #define GENESISCC _fastcall
#elif defined(__GNUC__) && defined(__i386__)  // Only apply fastcall for x86 on GCC
  #define GENESISCC __attribute__((fastcall))
#else
  #define GENESISCC
#endif

#if defined(_WIN32) && defined(_MSC_VER)  // Microsoft Visual Studio (Windows)
  #if defined(BUILDGENESIS) && defined(GENESISDLLVERSION)
    #define GENESISAPI __declspec(dllexport)
  #elif defined(GENESISDLLVERSION)
    #define GENESISAPI __declspec(dllimport)
  #else
    #define GENESISAPI
  #endif
#elif defined(__GNUC__) || defined(__clang__)  // GCC and Clang (Linux, macOS, MSYS)
  #if defined(BUILDGENESIS) && defined(GENESISDLLVERSION)
    #define GENESISAPI __attribute__((visibility("default")))
  #else
    #define GENESISAPI
  #endif
#else  // Other compilers
  #define GENESISAPI
#endif

/******** The Basic Types ****************************/
typedef int8_t    geBoolean;
#define GE_FALSE	(0)
#define GE_TRUE	 (!GE_FALSE)

typedef float geFloat;

#ifndef NULL
#define NULL	((void *)0)
#endif

// You'll never guess how these were originally typedef'd! /s
typedef int32_t   int32;
typedef int16_t   int16;
typedef int8_t    int8 ;
typedef uint32_t  uint32;
typedef uint16_t  uint16;
typedef uint8_t   uint8 ;


/******** Macros on Genesis basic types *************/

#define GE_ABS(x)             ( (x) < 0 ? (-(x)) : (x) )
#define GE_CLAMP(x,lo,hi)		 ( (x) < (lo) ? (lo) : ( (x) > (hi) ? (hi) : (x) ) )
#define GE_CLAMP8(x)          GE_CLAMP(x,0,255)
#define GE_CLAMP16(x)         GE_CLAMP(x,0,65536)
#define GE_BOOLSAME(x,y)      ( ( (x) && (y) ) || ( !(x) && !(y) ) )

#define GE_EPSILON				    ((geFloat)0.000797f)
#define GE_FLOATS_EQUAL(x,y)	( GE_ABS((x) - (y)) < GE_EPSILON )
#define GE_FLOAT_ISZERO(x)		GE_FLOATS_EQUAL(x,0.0f)

#define	GE_PI					      ((geFloat)3.14159265358979323846f)

/****************************************************/

#ifdef __cplusplus
}
#endif

#endif
 
