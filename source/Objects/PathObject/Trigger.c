/****************************************************************************************/
/*  TRIGGER.C                                                                           */
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
#include <stdio.h>
#include <memory.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "Actor.h"
#include "Errorlog.h"
#include "Trigger.h"
#include "jeWorld.h"

#ifdef BUILD_BE
#define stricmp strcasecmp
#define _MAX_FNAME 512
#endif

#define MAX_EVENT_ARGS 6

struct TriggerData
{
	int dummy;
/*
	ObjMgr *OM;
	SoundPool *SPool;
	TexturePool *TPool;
	void *ESystem;
	void *EManager;
*/
}TriggerData;


jeBoolean Trigger_Set()
{
/*
	TriggerData.OM = OM;
	TriggerData.SPool = SPool;
	TriggerData.TPool = TPool;
	TriggerData.ESystem = ESystem;
	TriggerData.EManager = EManager;
*/

	return JE_TRUE;
}

static void Trigger_GetContextXForm(TriggerContext ContextType, void *ContextData, jeXForm3d *XForm)
	{
	assert(ContextData);
	assert(XForm);

	switch (ContextType)
		{
		case TRIGGER_FROM_OBJECT:	
			{
			//Object *Obj;
			//Obj = (Object*)ContextData;
			//ObjMgr_GetObjectXForm(Obj,XForm);
			break;
			}
		case TRIGGER_FROM_XFORM:	
			*XForm = *((jeXForm3d*)ContextData);
			break;
		case TRIGGER_FROM_MODEL:	
			*XForm = *((jeXForm3d*)ContextData);
			break;
		case TRIGGER_FROM_ENTITY:	
			*XForm = *((jeXForm3d*)ContextData);
			break;
		case TRIGGER_FROM_ACTOR:	
			jeActor_GetBoneTransform((jeActor*)ContextData, NULL, XForm);
			break;
		}
	}




void Trigger_ParseEvent(jeWorld *World, TriggerContext ContextType, void *ContextData, char *EventString)
{
	char *ep;
	//jeXForm3d XForm;
	int32 *etype;
	char *semi_ptr = NULL, *bone_ptr = NULL;

	// int32 versions of the id string - reverse characters account for intel endian ordering
	#define PTH		'\0HTP'
	#define MTA		'\0ATM'
	#define WAV		'\0VAW'
	#define MODL	 'LDOM'
	#define ACTR	 'RTCA'
	#define EFCT     'TCFE'
	#define LVL     '\0LVL'
	#define LSTY     'YTSL'
	#define TPTH     'HTPT'
	#define FDIN     'NIDF'
	#define FDOT     'TODF'

	char Arg[MAX_EVENT_ARGS][_MAX_FNAME];

	assert(World);
	assert(EventString);

	// if the event string came from an actor there is always a "; BoneName" attached to the end
	// account for it
	semi_ptr = strstr(EventString,";");
	if (semi_ptr)
		{
		*semi_ptr = '\0';
		bone_ptr = semi_ptr++;
		}

	ep = EventString;
	memset(Arg,0, sizeof(Arg));
	sscanf(ep, "%s %s %s %s %s %s", &Arg[0], &Arg[1], &Arg[2], &Arg[3], &Arg[4], &Arg[5], &Arg[6]);

	etype = (int32*)Arg[0];
	switch (*etype)
		{
		case LSTY:
			//Trigger_EffectLightStyle(World, ContextType, ContextData, Arg[1]);
			break;
		case WAV:
			{
			//Trigger_GetContextXForm(ContextType, ContextData, &XForm);
			//Trigger_PlaySound( &XForm, Arg[1], (float)atof(Arg[2]) );
			break;
			}
		case PTH:
			//Trigger_ActorToPath(World, ContextType, ContextData, Arg[1], Arg[2]);
			break;
		case LVL:
			//Trigger_NewLevel(World, Arg[1]);
			break;
		case MTA:
			//Trigger_ActorMotion(World, ContextType, ContextData, Arg[1], Arg[2]);
			break;
		case MODL:
			//Trigger_Model(World, Arg[1]);
			break;
		case FDOT:
			//Trigger_FadeOut(World, (float)atof(Arg[1]));
			break;
		case FDIN:
			//Trigger_FadeIn(World, (float)atof(Arg[1]));
			break;
		case TPTH:
			//Trigger_PathTime(World, Arg[1], (float)atof(Arg[2]));
			break;
		case EFCT:
			if (stricmp(Arg[1], "ambient") == 0)
				{
				//Trigger_EffectAmbient(World, ContextType, ContextData, Arg[3], Arg[2]);
				}
			else
			if (stricmp(Arg[1], "spout1") == 0)
				{
				//Trigger_EffectSpout1(World, ContextType, ContextData, Arg[3], Arg[2]);
				}
			else
			if (stricmp(Arg[1], "morph") == 0)
				{
				//Trigger_EffectMorph(World, ContextType, ContextData, Arg[3], Arg[2]);
				}
			else
			if (stricmp(Arg[1], "ink") == 0)
				{
				//if (ContextType == TRIGGER_FROM_OBJECT)
				//	ContextType = TRIGGER_FROM_OBJECT_XFORM;
				//Trigger_EffectSpout1(World, ContextType, ContextData, NULL, NULL);
				}
			break;
		default:
			break;
		}

	return;
}

void Trigger_ProcessEvents(jeWorld *World, TriggerContext ContextType, void *ContextData, jeMotion *Motion, float StartTime, float EndTime)
{
	float Time;
	char *EventString;

	assert(World);
	assert(Motion);
	assert(StartTime <= EndTime);

	jeMotion_SetupEventIterator(Motion, StartTime, EndTime);

	while( jeMotion_GetNextEvent( Motion, &Time, (const char **)&EventString ) )
		{
		Trigger_ParseEvent(World, ContextType, ContextData, EventString);
		}
}
