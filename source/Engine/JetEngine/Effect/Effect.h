/****************************************************************************************/
/*  EFFECT.H                                                                            */
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
////////////////////////////////////////////////////////////////////////////////////////
//
//	In order to add a new effect it must:
//	1) have its header listed in here
//	2) have its type listed in the "EffectType" struct
//	3) have its interface listed in the "EffectList" struct
//
////////////////////////////////////////////////////////////////////////////////////////
#ifndef EFFECT_H
#define EFFECT_H


////////////////////////////////////////////////////////////////////////////////////////
//	IF YOU ADD AN EFFECT ITS HEADER MUST BE LISTED HERE
////////////////////////////////////////////////////////////////////////////////////////
#include "Jet.h"
#include "Spray.h"

#ifdef __cplusplus
	extern "C" {
#endif


////////////////////////////////////////////////////////////////////////////////////////
//	Effect death callback
////////////////////////////////////////////////////////////////////////////////////////
typedef void (*EffectDeathCallback)( int ID, void* Context );


////////////////////////////////////////////////////////////////////////////////////////
//	Effect system pointer
////////////////////////////////////////////////////////////////////////////////////////
typedef struct	EffectSystem	EffectSystem;


////////////////////////////////////////////////////////////////////////////////////////
//	Supported effect types (IF YOU ADD AN EFFECT ITS TYPE MUST BE LISTED HERE)
////////////////////////////////////////////////////////////////////////////////////////
typedef enum
{
	Effect_STARTOFLIST = 0,
	Effect_Spray,

} EffectType;


////////////////////////////////////////////////////////////////////////////////////////
//	Prototypes
////////////////////////////////////////////////////////////////////////////////////////

//	Create the effect system.
//
////////////////////////////////////////////////////////////////////////////////////////
EffectSystem * jeEffect_SystemCreate(
	jeEngine		*EngineToUse,	// engine to use
	jeWorld			*WorldToUse,	// world to use
	jeSound_System	*SoundToUse,	// sound system to use
	jeCamera		*CameraToUse );	// camera to use

//	Destroy the effect system.
//
////////////////////////////////////////////////////////////////////////////////////////
void jeEffect_SystemDestroy(
	EffectSystem	**System );	// effect system to zap

//	Changed the world that the effect system uses
//
////////////////////////////////////////////////////////////////////////////////////////
jeBoolean jeEffect_ChangeWorld(
	EffectSystem	*ESystem,	// effect system whose world will change
	jeWorld			*World );	// the new world that will be used

//	Create an effect and returns its unique identifier.
//
////////////////////////////////////////////////////////////////////////////////////////
jeBoolean jeEffect_New(
	EffectSystem		*System,		// effect system to add it to
	EffectType			Type,			// type of effect
	void				*Data,			// effect data
	int					AttachTo,		// identifier of effect to attach to
	EffectDeathCallback	DeathCallback,	// death callback function
	void				*DeathContext,	// context data
	int					*Id );			// where to save effect id

//	Removes an effect.
//
////////////////////////////////////////////////////////////////////////////////////////
jeBoolean jeEffect_Delete(
	EffectSystem	*System,	// effect system to delete it from
	int				ID );		// id of effect to delete

//	Process all current effects.
//
////////////////////////////////////////////////////////////////////////////////////////
jeBoolean jeEffect_SystemFrame(
	EffectSystem	*System,		// effect system to process a frame for
	float			TimeDelta );	// amount of elapsed time

//	Returns true if the effect is still alive, false if it isn't.
//
////////////////////////////////////////////////////////////////////////////////////////
jeBoolean jeEffect_StillExists(
	EffectSystem	*System,	// effect system to search in
	int				ID );		// effect id to verify

//	Modify an effect.
//
////////////////////////////////////////////////////////////////////////////////////////
jeBoolean jeEffect_Modify(
	EffectSystem	*System,	// effect system in which it exists
	EffectType		Type,		// type of effect
	void			*Data,		// new effect data
	int				ID,			// id of effect to modify
	uint32			Flags );	// how to use the new data

//	Pause/unpause an effect.
//
////////////////////////////////////////////////////////////////////////////////////////
jeBoolean jeEffect_SetPause(
	EffectSystem	*System,	// effect system in which it exists
	int				ID,			// id of effect to modify
	jeBoolean		Pause );	// new pause state

//	Get a pointer to the engine that this effect system is tied to.
//
////////////////////////////////////////////////////////////////////////////////////////
jeEngine * jeEffect_GetEngine(
	EffectSystem	*ESystem );	// effect system whose engine we want

//	Get a pointer to the world that this effect system is tied to.
//
////////////////////////////////////////////////////////////////////////////////////////
jeWorld * jeEffect_GetWorld(
	EffectSystem	*ESystem );	// effect system whose world we want

//	Get a pointer to the camera that this effect system is tied to.
//
////////////////////////////////////////////////////////////////////////////////////////
jeCamera * jeEffect_GetCamera(
	EffectSystem	*ESystem );	// effect system whose camera we want


#ifdef __cplusplus
	}
#endif

#endif
