/****************************************************************************************/
/*  jeVec3d_Katmai.h                                                                    */
/*                                                                                      */
/*  Author: Anthony Rufrano                                                             */
/*  Description: Katmai (SSE) optimized vector math                                     */
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

#ifndef JE_VEC3D_KATMAI_H
#define JE_VEC3D_KATMAI_H

#include "BaseType.h"

typedef struct jeVec3d					jeVec3d;

jeFloat									jeVec3d_DotProduct_SSE(const jeVec3d *v1, const jeVec3d *v2);
void									jeVec3d_CrossProduct_SSE(const jeVec3d *v1, const jeVec3d *v2, jeVec3d *result);
void									jeVec3d_Normalize_SSE(jeVec3d *v1);
void									jeVec3d_Scale_SSE(const jeVec3d *v1, float scale, jeVec3d *result);
jeFloat									jeVec3d_Length_SSE(const jeVec3d *v1);
void									jeVec3d_Subtract_SSE(const jeVec3d *v1, const jeVec3d *v2, jeVec3d *result);
void									jeVec3d_Add_SSE(const jeVec3d *v1, const jeVec3d *v2, jeVec3d *result);
void									jeVec3d_AddScaled_SSE(const jeVec3d *v1, const jeVec3d *v2, float scale, jeVec3d *result);
jeFloat									jeVec3d_DistanceBetween_SSE(const jeVec3d *v1, const jeVec3d *v2);

#endif