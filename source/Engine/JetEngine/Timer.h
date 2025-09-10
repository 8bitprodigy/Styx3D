/****************************************************************************************/
/*  TIMER.H                                                                             */
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
#ifndef TIMER_H
#define TIMER_H

#include <stdio.h>
#include "Tsc.h"
#include "Cpu.h"

#ifdef __cplusplus
extern "C" {
#endif

//{

extern FILE * timerFP;
extern int timerCount;
extern double time_Master;

extern void Timer_Start(void);
extern void Timer_Stop(void);

#ifdef DO_TIMER	//}{

#pragma message("timer ON")

#ifdef _DEBUG //}{

#define TIMER_VARS(func)	static double time_##func =0.0;	static tsc_type tsc_##func##1,tsc_##func##2; static int nest_##func = 0

#define TIMER_P(func)	do { readTSC(tsc_##func##1); assert( nest_##func == 0 ); nest_##func ++; } while(0)
#define TIMER_Q(func)	do { jeCPU_PauseMMX(); readTSC(tsc_##func##2); assert( nest_##func == 1 ); nest_##func --; time_##func += diffTSC(tsc_##func##1,tsc_##func##2); jeCPU_ResumeMMX(); } while(0)

#define TIMER_REPORT(func)	fprintf(timerFP,"%-20s : %1.6f : %2.1f %%\n", (#func) , (time_##func)/(double)timerCount , (time_##func)*100.0/(time_Master) )

#else	//}{

#define TIMER_VARS(func)	static double time_##func =0.0;	static tsc_type tsc_##func##1,tsc_##func##2

#define TIMER_P(func)	readTSC(tsc_##func##1)
#define TIMER_Q(func)	do { jeCPU_PauseMMX(); readTSC(tsc_##func##2); time_##func += diffTSC(tsc_##func##1,tsc_##func##2); jeCPU_ResumeMMX(); } while(0)

#define TIMER_REPORT(func)	fprintf(timerFP,"%-20s : %1.6f : %2.1f %%\n", (#func) , (time_##func)/(double)timerCount , (time_##func)*100.0/(time_Master) )

#endif	//}{

#define TIMER_COUNT()	timerCount++

#define TIMER_START()	Timer_Start()
#define TIMER_STOP()	Timer_Stop()

#else	//}{

#pragma message("timer OFF")

#define TIMER_VARS(func)
#define TIMER_P(func)
#define TIMER_Q(func)
#define TIMER_REPORT(func)

#define TIMER_COUNT()

#define TIMER_START()
#define TIMER_STOP()

#endif //}{

/**********

//example usage:

TIMER_VARS(test1);
TIMER_VARS(test2);

int main(int argc,char *argv[])
{
int i,j;

		TIMER_P(test1);

		for(i=0;i<1000;i++)
		{
			TIMER_P(test2);
			j = 99/(i+1);
			TIMER_Q(test2);
		}
			
		TIMER_Q(test1);

	TIMER_REPORT(test2);
	TIMER_REPORT(test1);

return 0;
}

**********/

//}

#ifdef __cplusplus
}
#endif

#endif // TIMER_H

