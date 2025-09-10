/****************************************************************************************/
/*  OBJECTMSG.H                                                                         */
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
#ifndef __OBJECT_MSG__
#define __OBJECT_MSG__

#include "Object.h"

#ifdef __cplusplus
extern "C" {
#endif

///////////////////////////////////////////////////////
#define OBJECT_EVENT_MSG							0
typedef struct
{
	jeObject *FromObj;
	int EventType;
	char *Args;
}Object_EventData;
///////////////////////////////////////////////////////
#define OBJECT_TIMELINE_GET_KEY_DATA_MSG			10
typedef struct
{
	int TimeLineNdx;
	int ChannelNdx;
	jeXForm3d ReturnXF;
}Object_TimeKeyData;
///////////////////////////////////////////////////////
#define OBJECT_TIMELINE_SET_PLAY_MODE_MSG			11
#define OBJECT_TIMELINE_GET_PLAY_MODE_MSG			12
typedef struct
{
	int TimeLineNdx;
	jeBoolean PlayMode;
}Object_TimePlayMode;
///////////////////////////////////////////////////////
#define OBJECT_TIMELINE_GET_JEOBJECT_MSG			13
typedef struct
{
	int TimeLineNdx;
	jeObject *ReturnObj;
}Object_TimeGetObject;
///////////////////////////////////////////////////////
#define OBJECT_TIMELINE_GET_CURRENT_TIMELINE_MSG	14
typedef struct
{
	int TimeLineNdx;
	int ReturnTimeLineNdx;
}Object_TimeGetTimeLine;
///////////////////////////////////////////////////////
#define OBJECT_TIMELINE_GET_CURRENT_TIME_MSG		15
typedef struct
{
	int TimeLineNdx;
	float ReturnTime;
}Object_TimeGetTime;
///////////////////////////////////////////////////////
  

// time line specific
#define EVENT_TYPE_TIMELINE_PLAY 0
#define EVENT_TYPE_TIMELINE_STOP 1

#ifdef __cplusplus
}
#endif

#endif