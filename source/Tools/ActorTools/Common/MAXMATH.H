/****************************************************************************************/
/*  MAXMATH.H                                                                           */
/*                                                                                      */
/*  Author: Stephen Balkum	                                                            */
/*  Description:  3DS MAX compatible transformation functions.                          */
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
#ifndef MAXMATH_H
#define MAXMATH_H

#ifdef __cplusplus
extern "C"
{
#endif

void MaxMath_Transform(const jeXForm3d* M, const jeVec3d* v, jeVec3d* r);
void MaxMath_GetInverse(const jeXForm3d* A, jeXForm3d* Inv);
void MaxMath_InverseMultiply(const jeXForm3d* A, const jeXForm3d* B, jeXForm3d* M);
void MaxMath_Multiply(const jeXForm3d* M1, const jeXForm3d* M2, jeXForm3d* MProduct);

#ifdef __cplusplus
}
#endif

#endif // MAXMATH_H