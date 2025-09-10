/****************************************************************************************/
/*  TIMER.C                                                                             */
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
#include "Timer.h"
#include <assert.h>

FILE * timerFP = NULL;

int timerCount = 0;

int timerIsStarted = 0;
double time_Master = 0.0;
static tsc_type tsc_Master;

void Timer_Start(void)
{
assert( ! timerIsStarted);
timerIsStarted = 1;
readTSC(tsc_Master);
}
void Timer_Stop(void)
{
tsc_type tsc_Master2;
readTSC(tsc_Master2);
assert(timerIsStarted);
timerIsStarted = 0;
time_Master += diffTSC(tsc_Master,tsc_Master2);
}
