/****************************************************************************************/
/*  RUNTRANS.H                                                                          */
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
#ifndef CRB_RUN_TRANSFORM_H
#define CRB_RUN_TRANSFORM_H

/** RunPack does a full arith on the runlengths ; it creates a literal array
*	with no run-flags in it; this is optimal for post-coding of the literals
*  RunPack : Array[ArrayLen] -> Literals[NumLiterals] + PackedRuns[NumRuns,PackedLen]
* UnRunpack: Literals + PackedRuns -> Array[ArrayLen]
*
******/

extern jeBoolean RunPack(uint8 *Array,uint32 ArrayLen,uint8 * Literals,
  uint32 * NumLiteralsPtr,uint8 * PackedRunArray,uint32 * NumRunsPtr,
  uint32 * RunPackedLenPtr);

extern jeBoolean UnRunPack(uint8 *Array,uint32 ArrayLen,uint8 * Literals,
  uint8 * PackedRunArray);

/**
* 
* runTransform is a uint8 <-> uint8 reversible transform
*
* both return the len of the comp array
*
*****/

extern int doRunTransform(uint8 *raw,int rawLen,uint8 *comp);
extern int unRunTransform(uint8 *raw,int rawLen,uint8 *comp);

#endif

