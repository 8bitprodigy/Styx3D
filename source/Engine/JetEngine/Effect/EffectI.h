/****************************************************************************************/
/*  EFFECTI.H                                                                           */
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
#ifndef EFFECTI_H
#define EFFECTI_H

#include "Jet.h"
#include "Particle.h"

#ifdef __cplusplus
	extern "C" {
#endif


////////////////////////////////////////////////////////////////////////////////////////
//	Resources available to all effects
////////////////////////////////////////////////////////////////////////////////////////
typedef struct
{
	jeEngine		*Engine;	// engine to use
	jeWorld			*World;		// world to use
	jeSound_System	*Sound;		// sound system to use
	jeCamera		*Camera;	// camera to use
	Particle_System	*PS;		// particle system

} EffectResource;


////////////////////////////////////////////////////////////////////////////////////////
//	Effect interface
////////////////////////////////////////////////////////////////////////////////////////
typedef char *		( *EffectI_GetName )( void );
typedef void *		( *EffectI_Create )( EffectResource *Resource, int TypeID );
typedef void		( *EffectI_Destroy )( void **ResourceToZap );
typedef void *		( *EffectI_Add )( void *Resource, void *Data, void *AttachTo );
typedef void		( *EffectI_Remove )( void *Resource, void *Data );
typedef jeBoolean	( *EffectI_Process )( void *Resource, float TimeDelta, void *Data );
typedef jeBoolean	( *EffectI_Frame )( void *Resource, float TimeDelta );
typedef jeBoolean	( *EffectI_Modify )( void *Resource, void *Data, void *NewData, uint32 Flags );
typedef void		( *EffectI_Pause )( void *Resource, void *Data, jeBoolean Pause );
typedef	struct Effect_Interface
{
	EffectI_GetName		GetName;	// get name of the effect
	EffectI_Create		Create;		// does all initial setup for an effect type
	EffectI_Destroy		Destroy;	// completely removes an effect type, performs all clean up
	EffectI_Add			Add;		// adds an individual effect
	EffectI_Remove		Remove;		// removes an individual effect
	EffectI_Process		Process;	// performs processing for an individual effect
	EffectI_Frame		Frame;		// performs once-per-frame procesing for all effects of a type
	EffectI_Modify		Modify;		// adjust the effect
	EffectI_Pause		Pause;		// adjust pause state
	int32				Size;		// size of the effects structure

}	Effect_Interface;


#ifdef __cplusplus
	}
#endif

#endif
