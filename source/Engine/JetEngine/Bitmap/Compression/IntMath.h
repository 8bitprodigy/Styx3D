/****************************************************************************************/
/*  INTMATH.H                                                                           */
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
#ifndef CRB_INTMATH_H
#define CRB_INTMATH_H

#include "Utility.h"

#ifndef ispow2
#define ispow2(x) (!( (x) & ~(-(x)) ))
#endif

extern int intlog2_noif(register uint16 N); /** truncated **/
extern int intlog2_uw(register uint16 N); /** truncated **/

extern int intlog2 (uint32 N); //truncated
extern int intlog2r(uint32 N); //rounded
extern int intlog2x10(uint32 N); // *10.0 then rounded
extern int intlog2x16(uint32 N); // *16.0 then rounded

extern uint32 square(uint32 val);	// int->int , handles overflow correctly

extern int ieeefloat_trick_log2(int x); // amazing! returns -127 for log2(0)

extern uint32 isqrt(uint32 N);

extern int GaussianRand(int val,int step);

#define ilog2 intlog2
#define ilog2r intlog2r
#define log2x10 intlog2x10
#define ilog2x10 intlog2x10
#define log2x16 intlog2x16
#define ilog2x16 intlog2x16

#endif
 
