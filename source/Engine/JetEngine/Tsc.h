/****************************************************************************************/
/*  TSC.H                                                                               */
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
#ifndef TSC_H
#define TSC_H

#ifdef __cplusplus
extern "C" {
#endif

/**********
*

	routines to access the TSC

	you must run jeCPU_GetInfo() to calibrary the Hz <-> Seconds converter

	note : showPop* functions use floats, but are MMX-safe

*
********/

	// show() will Pop() two and print the delta to log()
	// does nothing unless debug is on

extern void pushTSC(void);

	// the pop reads once & pop once & take difference

extern double popTSC(void);
extern double popTSChz(void);
extern void showPopTSC(const char *tag);
extern void showPopTSCper(const char *tag,int items,const char *itemTag);

	// primitives

typedef unsigned long tsc_type [2];

extern void readTSC(unsigned long *tsc);
extern double diffTSC(const unsigned long *tsc1,const unsigned long*tsc2);

extern double timeTSC(void);	// use the TSC as a clock!

	// hz is machine independent

extern double diffTSChz(const unsigned long *tsc1,const unsigned long*tsc2);

#ifdef __cplusplus
}
#endif

#endif
