/****************************************************************************************/
/*  CPU.H                                                                               */
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
#ifndef JE_CPU_H
#define JE_CPU_H

#include "BaseType.h"

#ifdef __cplusplus
extern "C" {
#endif

//--------

jeBoolean jeCPU_GetInfo(void);

void jeCPU_FloatControl_Push(void);
void jeCPU_FloatControl_Pop(void);
void jeCPU_FloatControl_RoundDown(void);
void jeCPU_FloatControl_RoundNearest(void);
void jeCPU_FloatControl_SinglePrecision(void);
void jeCPU_FloatControl_DoublePrecision(void);

void jeCPU_EnterMMX(void);	// wrap MMX sections with these:
void jeCPU_LeaveMMX(void);

void jeCPU_PauseMMX(void);	// to temporarily used floats inside an MMX section:
void jeCPU_ResumeMMX(void);

//-------- CPU Info:

#define	JE_CPU_HAS_RDTSC		0x0001
#define	JE_CPU_HAS_MMX  		0x0002
#define	JE_CPU_HAS_3DNOW		0x0004
#define	JE_CPU_HAS_CMOV 		0x0008
#define	JE_CPU_HAS_FCMOV		0x0010
#define	JE_CPU_HAS_KATMAI		0x0020
#define JE_CPU_HAS_CMPXCHG8B	0x0040
#define	JE_CPU_HAS_SSE2	        0x0080

extern uint32	jeCPU_Features;			// JE_CPU_HAS bitmasks
extern uint32	jeCPU_MHZ;
extern float	jeCPU_SecondsPerClock;	// == 1.0 / HZ
extern float	jeCPU_PerformanceFreq;	// Number of QueryPerformanceCounter Ticks Per Second

//--------

#ifdef __cplusplus
}
#endif

#endif // JE_CPU_H

