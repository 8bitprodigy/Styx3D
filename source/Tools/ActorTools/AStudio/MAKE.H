/****************************************************************************************/
/*  MAKE.H																				*/
/*                                                                                      */
/*  Author: Mike Sandige	                                                            */
/*  Description: Actor make process main module.										*/
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
#ifndef MAKE_H
#define MAKE_H

#include "basetype.h"
#include "mkutil.h"
#include "AProject.h"
#include "AOptions.h"

#ifdef __cplusplus
extern "C" {
#endif


jeBoolean Make_Body(AProject *Prj, AOptions *Options, MkUtil_Printf Printf);

jeBoolean Make_Motion(AProject *Prj, AOptions *Options, 
					int Count, int *MotionIndexArray, MkUtil_Printf Printf);

jeBoolean Make_Actor(AProject *Prj, AOptions *Options, MkUtil_Printf Printf);

jeBoolean Make_Clean(AProject *Prj, AOptions *Options, MkUtil_Printf Printf);

void Make_SetInterruptFlag (jeBoolean State);

jeBoolean Make_ActorSummary(AProject *Prj, MkUtil_Printf Printf);

#ifdef __cplusplus
}
#endif

#endif

