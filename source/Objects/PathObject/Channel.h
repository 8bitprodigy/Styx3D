/****************************************************************************************/
/*  CHANNEL.H                                                                           */
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
#ifndef CHANNEL_H
#define CHANNEL_H

#include "Xform3d.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_KEYS 512
#define MAX_TIME_LINES 64

typedef enum ChannelDef
{
CHANNEL_NULL = -1,
CHANNEL_POS = 0,
CHANNEL_ROT,
CHANNEL_EVENT,
MAX_CHANNELS,
};

typedef union ChannelData
{
	char		*String;
	jeXForm3d	XForm;
}ChannelData;

typedef struct Channel
{
	jeBoolean	Disabled;
	ChannelData	KeyData[MAX_KEYS];
	float		KeyList[MAX_KEYS];	// list of key times
	int			KeyCount;
	int 		KeysSelected[MAX_KEYS];	// need to allow for selection of muliple keys
}Channel;

#ifdef __cplusplus
}
#endif

#endif

