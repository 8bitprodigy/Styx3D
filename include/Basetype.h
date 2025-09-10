/****************************************************************************************/
/*  BASETYPE.H                                                                          */
/*                                                                                      */
/*  Author:                                                                             */
/*  Description: Basic type definitions and calling convention defines                  */
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
#ifndef JE_BASETYPE_H
#define JE_BASETYPE_H
 
/*
	Some basic types defined with clear names for
	more specific data definitions
*/
 
#ifdef __cplusplus
extern "C" {
#endif


//------------------------------ 
// function types

// Krouer - change calling convention to __stdcall
// keep __fastcall for internal call perhaps engine can increase the gain by using __inline instead
// but __inline will increase the size
#define	JETCF	__fastcall
#define	JETCC	__stdcall

// paradoxnj - We don't care about static libs.  Changed to conventional DLL export
#ifdef JETENGINE_EXPORTS
#define JETAPI					_declspec(dllexport)
#else
#define JETAPI					_declspec(dllimport)
#endif

#define JETLINE __inline //added (cyrius)

//------------------------------

typedef int			jeBoolean;
#define JE_FALSE	((jeBoolean)0)
#define JE_TRUE		((jeBoolean)1)

//------------------------------

typedef float jeFloat;

typedef signed long     int32;
typedef signed short    int16;
typedef signed char     int8 ;
typedef unsigned long	uint32;
typedef unsigned short	uint16;
typedef unsigned char	uint8 ;

//------------------------------

#ifndef NULL
#define NULL													(0)
#endif

#define	JE_PI													((jeFloat)3.14159265358979323846)
#define	JE_TWOPI											((jeFloat)6.28318530717958647692)
#define	JE_HALFPI											((jeFloat)1.57079632679489661923)

#define JE_DEGS_PER_RAD								((jeFloat)0.01745329251994329576)
#define JE_RADS_PER_DEG								((jeFloat)57.2957795130823208767)

// BEGIN - 32-bit color values - paradoxnj 8/3/2005
// maps unsigned 8 bits/channel to uint32
#define JE_COLOR_ARGB(a,r,g,b)						((uint32)((((a)&0xff)<<24)|(((r)&0xff)<<16)|(((g)&0xff)<<8)|((b)&0xff)))
#define JE_COLOR_RGBA(r,g,b,a)						JE_COLOR_ARGB(a,r,g,b)
#define JE_COLOR_XRGB(r,g,b)						JE_COLOR_ARGB(0xff,r,g,b)

#define JE_COLOR_XYUV(y,u,v)						JE_COLOR_ARGB(0xff,y,u,v)
#define JE_COLOR_AYUV(a,y,u,v)						JE_COLOR_ARGB(a,y,u,v)

// maps floating point channels (0.f to 1.f range) to uint32
#define JE_COLOR_COLORVALUE(r,g,b,a)				JE_COLOR_RGBA((uint32)((r)*255.f),(uint32)((g)*255.f),(uint32)((b)*255.f),(uint32)((a)*255.f))

#define JE_COLOR_GETARGB(argb,a,r,g,b)				{a=((argb>>24)&0xff); r=((argb>>16)&0xff); g=((argb>>8)&0xff); b=((argb)&0xff); }

// END - 32-bit color values - paradoxnj 8/3/2005

// should probably be moved to trig module
__inline jeFloat jeFloat_DegToRad(jeFloat d)
{
	return d * JE_DEGS_PER_RAD;
}

__inline jeFloat jeFloat_RadToDeg(jeFloat r)
{
	return r * JE_RADS_PER_DEG;
}


//------------------------------
// macros on basic jet types

#define JE_ABS(x)										( (x) < 0 ? (-(x)) : (x) )
#define JE_CLAMP(x,lo,hi)								( (x) < (lo) ? (lo) : ( (x) > (hi) ? (hi) : (x) ) )
#define JE_CLAMP8(x)									JE_CLAMP(x,0,255)
#define JE_CLAMP16(x)									JE_CLAMP(x,0,65536)
#define JE_BOOLSAME(x,y)								( ( (x) && (y) ) || ( !(x) && !(y) ) )

#define JE_EPSILON										((jeFloat)0.000797f)
#define JE_FLOATS_EQUAL(x,y)							( JE_ABS((x) - (y)) < JE_EPSILON )
#define JE_FLOAT_ISZERO(x)								JE_FLOATS_EQUAL(x,0.0f)

// you're right... inline funcs are more useful :^)
static __inline jeFloat jeFloat_Sqr(jeFloat a)
{
	return a * a;
}

static __inline jeFloat jeFloat_Cube(jeFloat a)
{
	return a * a * a;
}

//------------------------------

// CB : what does the optimizer do with inline assembly in inline functions ?
//		will it turn off all optimizations?

static jeFloat __inline jeFloat_RoundToInt(jeFloat val) // rounds depending on how you set jeCPU_FloatControl
{
	__asm
	{
		FLD  val
		FRNDINT
		FSTP val
	}
return val;
}

static jeFloat __inline jeFloat_Sqrt(jeFloat val)
{
	__asm 
	{
		FLD  val		// 1 clock
		FSQRT			// 30-70 clocks
		FSTP val		// 2 clocks
	}
return val;
}

static jeFloat __inline jeFloat_Sin(jeFloat val)
{
	__asm 
	{
		FLD  val		// 1 clock
		FSIN			// ~ 200 clocks
		FSTP val		// 2 clocks
	}
return val;
}

static jeFloat __inline jeFloat_Cos(jeFloat val)
{
	__asm 
	{
		FLD  val		// 1 clock
		FCOS			// ~ 200 clocks
		FSTP val		// 2 clocks
	}
return val;
}

static int32 __inline jeFloat_ToInt(jeFloat f)
{
int32 i;
	__asm
	{
		FLD   f
		FISTP i
	}
return i;
}

#pragma warning (disable:4514)	// unreferenced inline function

//------------------------------

#ifdef __cplusplus
class jeUnknown
{
protected:
	virtual ~jeUnknown()						{}

public:
	virtual uint32					AddRef() = 0;
	virtual uint32					Release() = 0;
};
#endif

#ifdef __cplusplus
}
#endif

#endif
