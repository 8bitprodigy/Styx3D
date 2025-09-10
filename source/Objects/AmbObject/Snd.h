/****************************************************************************************/
/*  SND.H                                                                               */
/*                                                                                      */
/*  Author: Peter Siamidis, Frank Maddin (Modified)                                     */
/*  Description: Plays a sound                                                          */
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
#ifndef SND_H
#define SND_H

#include "Jet.h"
#include "jeWorld.h"

#ifdef __cplusplus
	extern "C" {
#endif


////////////////////////////////////////////////////////////////////////////////////////
// Data which can be modified
////////////////////////////////////////////////////////////////////////////////////////
#define SND_POS		( 1 << 0 )

typedef struct
{
	jeWorld			*World;		// world to use
	jeSound_System	*Sound;		// sound system to use
	jeCamera		*Camera;	// camera to use

} EffectResource;

////////////////////////////////////////////////////////////////////////////////////////
// Effect info
////////////////////////////////////////////////////////////////////////////////////////
typedef struct
{
	int			TypeID;				// RESERVED, this MUST be the first item in the struct
	jeSound		*Sound;				// RESERVED, pointer to the active sound
	jeBoolean	Paused;				// RESERVED, whether or not the sound is paused
	jeFloat		LastVolume;			// RESERVED, its volume the last time it was modified
	jeFloat		LastPan;			// RESERVED, its pan the last time it was modified
	jeSound_Def	*SoundDef;			// sound def to play from
	jeVec3d		Pos;				// location of the sound
	jeFloat		Min;				// min distance whithin which sound is at max volume
	jeBoolean	Loop;				// whether or not to loop it
	jeBoolean	LastLoop;			// its loop last time it was modified -- added by tom morris May 2005
	jeBoolean	IgnoreObstructions;	// if obstructions should be ignored when compting sound data

} Snd;


void		Snd_Remove( EffectResource *Resource, Snd *Data );
jeBoolean	Snd_Process( EffectResource *Resource, float TimeDelta, jeBoolean bMute, Snd *Data );

#ifdef __cplusplus
	}
#endif

#endif
