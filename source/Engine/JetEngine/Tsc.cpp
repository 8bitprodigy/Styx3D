/****************************************************************************************/
/*  TSC.C                                                                               */
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
#include <stdio.h>	//sprintf
#include <assert.h>

#include "BaseType.h"
#include "Log.h"
#include "Cpu.h"
#include "Tsc.h"

#define TSC_STACK_SIZE (1024)

typedef struct { tsc_type tsc; } tscNode;

static tscNode tscStack[TSC_STACK_SIZE];
static tscNode * tscStackPtr = tscStack;

void pushTSC(void)
{
	readTSC(tscStackPtr->tsc);
	tscStackPtr++;
	assert( ((int)(tscStackPtr - tscStack)/sizeof(tscNode)) < TSC_STACK_SIZE );
}

double popTSC(void)
{
tsc_type tsc;
	readTSC(tsc);
	tscStackPtr--;
	assert( tscStackPtr >= tscStack );
return diffTSC(tscStackPtr->tsc,tsc);
}

double popTSChz(void)
{
tsc_type tsc;
	readTSC(tsc);
	tscStackPtr--;
	assert( tscStackPtr >= tscStack );
return diffTSChz(tscStackPtr->tsc,tsc);
}

void showPopTSC(const char *tag)
{
double time;

	jeCPU_PauseMMX();
	time = popTSC();
		Log_Printf("%s : %f seconds\n",tag,time);
	jeCPU_ResumeMMX();
}

void showPopTSCper(const char *tag,int items,const char *itemTag)
{
double time,hz,per;

	jeCPU_PauseMMX();
	hz = popTSChz();
	time = hz * jeCPU_SecondsPerClock;
	per = (time/(double)items);
	
	Log_Printf("%s : %f secs = %2.1f cycles / %s = ",tag,time,hz/items,itemTag);

	if ( per < 0.0001 ) 
	{
		Log_Printf("%d %ss /sec\n",(int)(1.0/per),itemTag);
	}
	else
	{
		Log_Printf("%f %ss /sec\n",(1.0/per),itemTag);
	}
	jeCPU_ResumeMMX();
}

void readTSC(uint32 *tsc)
{
#ifdef WIN32
	__asm 
	{
		//_emit 0x0F
		//_emit 0x31
		rdtsc
		mov EBX,tsc
		mov DWORD PTR [EBX + 0],EDX
		mov DWORD PTR [EBX + 4],EAX
	}
#endif
#ifdef BUILD_BE
	__asm__ __volatile__
	(
	"
		push %%ebx 
		rdtsc
		mov %0, %%ebx
		mov %%edx, (%%ebx)
		mov %%eax, 4(%%ebx)
		pop %%ebx 
	"
	: : "m" (tsc) : "%ebx" , "%eax", "%edx" );
#endif

}

double timeTSC(void)
{
uint32 tsc[2];
	readTSC(tsc);
return (tsc[0]*4294967296.0 + (double)tsc[1])*jeCPU_SecondsPerClock;
}

double diffTSC(const uint32 *tsc1,const uint32 *tsc2)
{
double time1,time2;
	time1 = ((double)tsc2[0] - (double)tsc1[0])*4294967296.0;
	time1 *= jeCPU_SecondsPerClock;
	time2 = (double)tsc2[1] - (double)tsc1[1];
	time2 *= jeCPU_SecondsPerClock;
return time1 + time2;
}

double diffTSChz(const uint32 *tsc1,const uint32 *tsc2)
{
double hz;
	hz = ((double)tsc2[0] - (double)tsc1[0])*4294967296.0 + ((double)tsc2[1] - (double)tsc1[1]);
return hz;
}
