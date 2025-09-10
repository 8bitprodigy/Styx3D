/****************************************************************************************/
/*  TRIGGER.H                                                                           */
/*                                                                                      */
/*  Author: Frank Maddin                                                                */
/*  Description:    Processes triggers from event strings           		            */
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
#ifndef TRIGGER_H
#define TRIGGER_H

#include "jeWorld.h"
#include "Motion.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
	TRIGGER_FROM_MODEL = 0,
	TRIGGER_FROM_ACTOR,
	TRIGGER_FROM_ENTITY,
	TRIGGER_FROM_XFORM,
	TRIGGER_FROM_OBJECT,
	TRIGGER_FROM_OBJECT_XFORM,
}	TriggerContext;

jeBoolean Trigger_Set();
void Trigger_ParseEvent(jeWorld *World, TriggerContext Type, void *ContextData, char *EventString);
void Trigger_ProcessEvents(jeWorld *World, TriggerContext Type, void *ContextData, jeMotion *Motion, float StartTime, float EndTime);

#ifdef __cplusplus
}
#endif

#endif