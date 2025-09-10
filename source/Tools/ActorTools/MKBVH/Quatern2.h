/****************************************************************************************/
/*  QUATERN2.H	                                                                            */
/*                                                                                      */
/*  Author:             	                                                            */
/*  Description:                                                						*/
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
#ifndef QUATERN2_H
#define QUATERN2_H

#include "quatern.h"

void Quaternion_GetEulerZXY(jeQuaternion* Q, float* pZ, float* pX, float* pY);

#endif // QUATERN2_H