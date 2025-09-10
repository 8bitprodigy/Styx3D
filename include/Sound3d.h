/****************************************************************************************/
/*  SOUND3D.H                                                                           */
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
/****************************************************************************/
/*    FILE: Sound3d.h                                                       */
/*                                                                          */
/*    Copyright (c) 1997, Eclipse Entertainment; All rights reserved.       */
/*                                                                          */
/****************************************************************************/
#ifndef JE_SOUND3D_H
#define JE_SOUND3D_H

#include "BaseType.h"
#include "Sound.h"
#include "jeWorld.h"

#ifdef __cplusplus
extern "C" {
#endif

// JET_PUBLIC_APIS

JETAPI	void JETCC jeSound3D_GetConfig(
			const jeWorld *World, 
			const jeXForm3d *CameraTransform, 
			const jeVec3d *SoundPos, 
			jeFloat Min, 
			jeFloat Ds,
			jeFloat *Volume,
			jeFloat *Pan,
			jeFloat *Frequency);

// JET_PRIVATE_APIS

#ifdef __cplusplus
}
#endif

#endif
