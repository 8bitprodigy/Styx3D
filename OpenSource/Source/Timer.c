/****************************************************************************************/
/*  Timer                                                                               */
/*                                                                                      */
/*  Author: Charles Bloom                                                               */
/*  Description: A nice little profiling utility                                        */
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
/*Genesis3D Version 1.1 released November 15, 1999                            */
/*  Copyright (C) 1999 WildTangent, Inc. All Rights Reserved           */
/*                                                                                      */
/****************************************************************************************/
#include <stdio.h>

#include "Timer.h"


FILE * timerFP;// = stdout;

int timerCount = 0;

double time_Master = 0.0;
static tsc_type tsc_Master;


void Timer_Start(void)
{
    readTSC(tsc_Master);
}

void Timer_Stop(void)
{
    tsc_type tsc_Master2;
    readTSC(tsc_Master2);
    time_Master += diffTSC(tsc_Master,tsc_Master2);
}
