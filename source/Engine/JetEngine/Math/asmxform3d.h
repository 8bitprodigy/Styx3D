/****************************************************************************************/
/*  ASMXFORM3D.H                                                                        */
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
#ifndef JE_ASMXFORM_H
#define JE_ASMXFORM_H

#include "Xform3d.h"

#ifdef __cplusplus
extern "C" {
#endif

void JETCC jeXForm3d_TransformVecArrayKatmai(const jeXForm3d *XForm, const jeVec3d *Source, jeVec3d *Dest, int32 Count);

void JETCC jeXForm3d_TransformArrayKatmai(const jeXForm3d *XForm,
										   const jeVec3d *Source,
										   jeVec3d *Dest,
										   int32 SourceStride,
										   int32 DestStride,
										   int32 Count);

void JETCC jeXForm3d_TransformArrayX86(const jeXForm3d *XForm,
										const jeVec3d *Source,
										jeVec3d *Dest,
										int32 SourceStride,
										int32 DestStride,
										int32 Count);

void JETCC jeXForm3d_TransformVecArrayX86(const jeXForm3d *XForm, const jeVec3d *Source, jeVec3d *Dest, int32 Count);

void JETCC jeXForm3d_TransformVecArray3DNow(const jeXForm3d *XForm, const jeVec3d *Source, jeVec3d *Dest, int32 Count);

void JETCC jeXForm3d_TransformArray3DNow(const jeXForm3d *XForm,
										  const jeVec3d *Source,
										  jeVec3d *Dest,
										  int32 SourceStride,
										  int32 DestStride,
										  int32 Count);


#ifdef __cplusplus
}
#endif

#endif // JE_ASMXFORM_H
