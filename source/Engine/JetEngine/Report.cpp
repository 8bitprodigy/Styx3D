/****************************************************************************************/
/*  REPORT.C                                                                            */
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

#ifndef DO_REPORT
#define DO_REPORT
#endif

#include <stdio.h>
#include <math.h> // for sqrt

#include "Report.h"

#ifndef _LOG
#define _LOG
#endif

#include "Log.h" 

FILE * reportFP = NULL;

void Report_DoReport(const char * str,int count,double tot,double totsqr)
{
double avg,avgsqr; 

	avg = tot/count;
	avgsqr = totsqr/count;

	Log_TeeFile(reportFP);
	Log_Printf("%-20s : avg = %2.1f (sdev = %2.1f), tot = %d\n", 
		str , avg, sqrt(avgsqr - avg*avg), (int)tot );
}
