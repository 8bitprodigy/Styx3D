/****************************************************************************************/
/*  EFFECT.C                                                                            */
/*                                                                                      */
/*  Author:  Peter Siamidis                                                             */
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

//	API to control special effects.
//	jeEffect_SystemFrame() must be called once per frame.

//#define	EFFECTSYSTEMTIMINGON
#ifdef EFFECTSYSTEMTIMINGON
#include <windows.h>
#endif
#include <memory.h>
#include <assert.h>
#include "Ram.h"
#include "Effect.h"
#include "IndexList.h"
#include "Errorlog.h"


////////////////////////////////////////////////////////////////////////////////////////
//	Hardwired stuff
////////////////////////////////////////////////////////////////////////////////////////
#define	EFFECT_TIMEDELTACAP	0.5f
#define EFFECT_LISTINCSIZE	50


////////////////////////////////////////////////////////////////////////////////////////
//	Effect interface list (IF YOU ADD AN EFFECT ITS INTERFACE MUST BE LISTED HERE)
////////////////////////////////////////////////////////////////////////////////////////
Effect_Interface *EffectInterfaceList[] =
{
	NULL,
	&Spray_Interface,

};
#define EffectCount	( sizeof( EffectInterfaceList ) / sizeof( EffectInterfaceList[0] ) )


////////////////////////////////////////////////////////////////////////////////////////
//	Top level effect struct
////////////////////////////////////////////////////////////////////////////////////////
typedef struct
{
	EffectType			Type;			// effect type
	jeBoolean			Paused;			// whether or not the effect is paused
	int					Slot;			// slot which this effect occupies
	void				*Data;			// effect data
	EffectDeathCallback	DeathCallback;	// death callback function
	void				*DeathContext;	// death context data

} EffectRoot;


////////////////////////////////////////////////////////////////////////////////////////
//	Effect system struct
////////////////////////////////////////////////////////////////////////////////////////
typedef struct EffectSystem
{
	IndexList		*EffectList;					// list of all effect root structs
	int				ActiveEffects[EffectCount];		// how many effects are currently active
	void			*ResourceList[EffectCount];		// resource list pointers
	EffectResource	Resource;						// list of effect resources

} EffectSystem;



////////////////////////////////////////////////////////////////////////////////////////
//
//	jeEffect_SystemCreate()
//
//	Create an effect system and return a pointer to it.
//
////////////////////////////////////////////////////////////////////////////////////////
EffectSystem * jeEffect_SystemCreate(
	jeEngine		*EngineToUse,	// engine to use
	jeWorld			*WorldToUse,	// world to use
	jeSound_System	*SoundToUse,	// sound system to use
	jeCamera		*CameraToUse )	// camera to use
{

	// locals
	EffectSystem	*System;
	int				i;

	// ensure valid data
	assert( EngineToUse != NULL );
	assert( WorldToUse != NULL );
	assert( CameraToUse != NULL );

	// allocate effect system struct and init it
	System = jeRam_AllocateClear( sizeof( *System ) );
	if ( System == NULL )
	{
		jeErrorLog_AddString( JE_ERR_MEMORY_RESOURCE, "jeEffect_SystemCreate: failed to create effect system struct.", NULL );
		return NULL;
	}		

	// save resource list
	System->Resource.Engine = EngineToUse;
	System->Resource.World = WorldToUse;
	System->Resource.Sound = SoundToUse;
	System->Resource.Camera = CameraToUse;

	// create particle system
	System->Resource.PS = Particle_SystemCreate( System->Resource.World );
	if ( System->Resource.PS == NULL )
	{
		jeErrorLog_AddString( JE_ERR_INTERNAL_RESOURCE, "jeEffect_SystemCreate: failed to create particle system.", NULL );
		jeEffect_SystemDestroy( &System );
		return NULL;
	}

	// create effect list
	System->EffectList = IndexList_Create( EFFECT_LISTINCSIZE, EFFECT_LISTINCSIZE );
	if ( System->EffectList == NULL )
	{
		jeErrorLog_AddString( JE_ERR_INTERNAL_RESOURCE, "jeEffect_SystemCreate: failed to create indexed effect list.", NULL );
		jeEffect_SystemDestroy( &System );
		return NULL;
	}

	// call all effect create functions
	for ( i = 1; i < EffectCount; i++ )
	{
		System->ResourceList[i] = EffectInterfaceList[i]->Create( &( System->Resource ), i );
		if ( System->ResourceList[i] == NULL )
		{
			jeErrorLog_AddString( JE_ERR_INTERNAL_RESOURCE, "jeEffect_SystemCreate: failed to initialize an effect subsystem.", NULL );
			jeEffect_SystemDestroy( &System );
			return NULL;
		}		
	}

	// all done
	return System;

} // jeEffect_SystemCreate()



////////////////////////////////////////////////////////////////////////////////////////
//
//	jeEffect_SystemDestroy()
//
//	Destroy an effect system.
//
////////////////////////////////////////////////////////////////////////////////////////
void jeEffect_SystemDestroy(
	EffectSystem	**System )	// effect system to zap
{

	// locals
	EffectSystem	*DeadSystem;
	EffectRoot		*Root;
	int				i;
	int				ListSize;
	jeBoolean		Result;

	// ensure valid data
	assert( System != NULL );
	assert( *System != NULL );

	// get dead effect system pointer
	DeadSystem = *System;

	// destroy all active effects
	if ( DeadSystem->EffectList != NULL )
	{

		// remove all current effects
		ListSize = IndexList_GetListSize( DeadSystem->EffectList );
		for ( i = 0; i < ListSize; i++ )
		{
			Root = IndexList_GetElement( DeadSystem->EffectList, i );
			if ( Root != NULL )
			{
				Result = jeEffect_Delete( DeadSystem, i );
				assert( Result == JE_TRUE );
			}
		}

		// destroy the list itself
		IndexList_Destroy( &( DeadSystem->EffectList ) );

		// call all effect interface destroy functions
		for ( i = 1; i < EffectCount; i++ )
		{
			assert( DeadSystem->ActiveEffects[i] == 0 );
			assert( DeadSystem->ResourceList[i] != NULL );
			EffectInterfaceList[i]->Destroy( &( DeadSystem->ResourceList[i] ) );
		}
	}

	// free particle system
	if ( DeadSystem->Resource.PS != NULL )
	{
		Particle_SystemDestroy( DeadSystem->Resource.PS );
		DeadSystem->Resource.PS = NULL;
	}

	// free system struct memory
	jeRam_Free( DeadSystem );
	System = NULL;

} // jeEffect_SystemDestroy()



////////////////////////////////////////////////////////////////////////////////////////
//
//	jeEffect_ChangeWorld()
//
//	Changed the world that the effect system uses
//
////////////////////////////////////////////////////////////////////////////////////////
jeBoolean jeEffect_ChangeWorld(
	EffectSystem	*ESystem,	// effect system whose world will change
	jeWorld			*World )	// the new world that will be used
{

	// locals
	int				i;
	int				ListSize;
	EffectRoot		*Root;
	jeBoolean		Result;
	Particle_System	*PS;

	// ensure valid data
	assert( ESystem != NULL );
	assert( World != NULL );

	// remove all current effects
	ListSize = IndexList_GetListSize( ESystem->EffectList );
	for ( i = 0; i < ListSize; i++ )
	{
		Root = IndexList_GetElement( ESystem->EffectList, i );
		if ( Root != NULL )
		{
			Result = jeEffect_Delete( ESystem, i );
			assert( Result == JE_TRUE );
		}
	}

	// make sure everything was zapped
	#ifndef	NDEBUG
	for ( i = 1; i < EffectCount; i++ )
	{
		assert( ESystem->ActiveEffects[i] == 0 );
	}
	#endif

	// save new world pointer
	ESystem->Resource.World = World;

	// recreate particle system
	assert( ESystem->Resource.PS != NULL );
	PS = Particle_SystemCreate( ESystem->Resource.World );
	if ( PS == NULL )
	{
		jeErrorLog_AddString( JE_ERR_INTERNAL_RESOURCE, "jeEffect_ChangeWorld: failed to recreate particle system.", NULL );
		return JE_FALSE;
	}
	Particle_SystemDestroy( ESystem->Resource.PS );
	ESystem->Resource.PS = PS;

	// all done
	return JE_TRUE;

} // jeEffect_ChangeWorld()



////////////////////////////////////////////////////////////////////////////////////////
//
//	jeEffect_New()
//
//	Create an effect and return its unique identifier, or return 0 if the effect
//	was not created.
//
////////////////////////////////////////////////////////////////////////////////////////
jeBoolean jeEffect_New(
	EffectSystem		*System,		// effect system to add it to
	EffectType			Type,			// type of effect
	void				*Data,			// effect data
	int					AttachTo,		// identifier of effect to attach to
	EffectDeathCallback	DeathCallback,	// death callback function
	void				*DeathContext,	// context data
	int					*Id )			// where to save effect id
{

	// locals
	Effect_Interface	*Effect;
	EffectRoot			*Root;
	void				*AttachData = NULL;
	int					Slot;

	// ensure valid data
	assert( System != NULL );
	assert( System->EffectList != NULL );
	assert( Data != NULL );

	// pick the effect interface that we will use
	Effect = EffectInterfaceList[Type];
	assert( Effect != NULL );

	// get an empty slot
	Slot = IndexList_GetEmptySlot( System->EffectList );
	if ( Slot == -1 )
	{
		jeErrorLog_AddString( JE_ERR_INTERNAL_RESOURCE, "jeEffect_New: failed to find available effect slot.", NULL );
		return JE_FALSE;
	}

	// if it wants to attach to another effect then verify that it still exists
	if ( AttachTo > 0 )
	{
		AttachData = IndexList_GetElement( System->EffectList, AttachTo );
		if ( AttachData == NULL )
		{
			jeErrorLog_AddString( JE_ERR_INVALID_PARMS, "jeEffect_New: AttachTo effect does not exist.", NULL );
			return JE_FALSE;
		}
	}

	// allocate root data space and init it
	Root = jeRam_AllocateClear( sizeof( EffectRoot ) );
	if ( Root == NULL )
	{
		jeErrorLog_AddString( JE_ERR_MEMORY_RESOURCE, "jeEffect_New: failed to create effect root struct.", NULL );
		return JE_FALSE;
	}
	Root->Paused = JE_FALSE;
	Root->Type = Type;

	// mark the type flag
	*( (int *)( Data ) ) = Root->Type;

	// add effect
	Root->Data = Effect->Add( System->ResourceList[Type], Data, AttachData );
	if ( Root->Data == NULL )
	{
		jeErrorLog_AddString( JE_ERR_INTERNAL_RESOURCE, "jeEffect_New: failed to add an effect.", NULL );
		jeRam_Free( Root );
		return JE_FALSE;
	}

	// add new effect to the list
	IndexList_AddElement( System->EffectList, Slot, Root );

	// adjust effect count
	System->ActiveEffects[Root->Type]++;

	// save callback info
	Root->DeathCallback = DeathCallback;
	Root->DeathContext = DeathContext;

	// save effect id number
	Root->Slot = Slot;
	if ( Id != NULL )
	{
		*Id = Slot;
	}

	// all done
	assert( Slot >= 0 );
	return JE_TRUE;

} // jeEffect_New()



////////////////////////////////////////////////////////////////////////////////////////
//
//	jeEffect_Delete()
//
//	Removes an effect.
//
////////////////////////////////////////////////////////////////////////////////////////
jeBoolean jeEffect_Delete(
	EffectSystem	*System,	// effect system to delete it from
	int				ID )		// id of effect to delete
{

	// locals
	Effect_Interface	*Effect;
	EffectRoot			*Root;

	// ensure valid data
	assert( System != NULL );
	assert( System->EffectList != NULL );

	// remove the effect
	Root = IndexList_GetElement( System->EffectList, ID );
	if ( Root != NULL )
	{

		// call its death callback if required
		if ( Root->DeathCallback != NULL )
		{
			Root->DeathCallback( Root->Slot, Root->DeathContext );
			Root->DeathCallback = NULL;
		}

		// remove the effecet
		Effect = EffectInterfaceList[Root->Type];
		Effect->Remove( System->ResourceList[Root->Type], Root->Data );
		System->ActiveEffects[Root->Type]--;

		// free root data
		jeRam_Free( Root );
		IndexList_DeleteElement( System->EffectList, ID );
		return JE_TRUE;
	}

	// if we got to here then the effect was not found
	return JE_FALSE;

} // jeEffect_Delete()



////////////////////////////////////////////////////////////////////////////////////////
//
//	jeEffect_SystemFrame()
//
//	Process all current effects. Return JE_TRUE if all effects were processed, or
//	JE_FALSE if one or more of the effects were not processed.
//
////////////////////////////////////////////////////////////////////////////////////////
jeBoolean jeEffect_SystemFrame(
	EffectSystem	*System,		// effect system to process a frame for
	float			TimeDelta )		// amount of elapsed time
{

	// locals
	EffectRoot			*Root = NULL;
	Effect_Interface	*Effect;
	jeBoolean			EffectAlive;
	int					i;
	int					ListSize;
	#ifdef EFFECTSYSTEMTIMINGON
	int					Type;
	LARGE_INTEGER		Frequency;
	LARGE_INTEGER		StartTime, EndTime;
	double				TotalTime[EffectCount];
	#endif

	// ensure valid data
	assert( System != NULL );
	assert( System->EffectList != NULL );

	// do nothing if no time has elapsed
	if ( TimeDelta <= 0.0f )
	{
		return JE_FALSE;
	}

	// get total start time
	#ifdef EFFECTSYSTEMTIMINGON
	QueryPerformanceFrequency( &Frequency );
	for ( i = 1; i < EffectCount; i++ )
	{
		TotalTime[i] = 0.0f;
	}
	#endif

	// cap the time delta
	if ( TimeDelta > EFFECT_TIMEDELTACAP )
	{
		TimeDelta = EFFECT_TIMEDELTACAP;
	}

	// process all effects
	ListSize = IndexList_GetListSize( System->EffectList );
	for ( i = 0; i < ListSize; i++ )
	{

		// do nothing if it is an empty slot
		Root = IndexList_GetElement( System->EffectList, i );
		if ( Root == NULL )
		{
			continue;
		}

		// get start time
		#ifdef EFFECTSYSTEMTIMINGON
		Type = Root->Type;
		QueryPerformanceCounter( &StartTime );
		#endif

		// process the effect...
		if ( Root->Paused == JE_FALSE )
		{

			// pick the effect interface that we will use
			Effect = EffectInterfaceList[Root->Type];
			assert( Effect != NULL );

			// process this effect
			EffectAlive = Effect->Process( System->ResourceList[Root->Type], TimeDelta, Root->Data );

			// remove it if required
			if ( EffectAlive == JE_FALSE )
			{

				// call its death callback if required
				if ( Root->DeathCallback != NULL )
				{
					Root->DeathCallback( Root->Slot, Root->DeathContext );
					Root->DeathCallback = NULL;
				}

				// remove the effect
				Effect->Remove( System->ResourceList[Root->Type], Root->Data );
				System->ActiveEffects[Root->Type]--;

				// free its data
				jeRam_Free( Root );
				IndexList_DeleteElement( System->EffectList, i );
			}
		}

		// get start time
		#ifdef EFFECTSYSTEMTIMINGON
		QueryPerformanceCounter( &EndTime );
		TotalTime[Type] += ( (double)( EndTime.LowPart - StartTime.LowPart ) / (double)( Frequency.LowPart ) );
		#endif
	}

	// do once-per-frame processing
	for ( i = 1; i < EffectCount; i++ )
	{
		#ifdef EFFECTSYSTEMTIMINGON
		QueryPerformanceCounter( &StartTime );
		#endif
		EffectInterfaceList[i]->Frame( System->ResourceList[i], TimeDelta );
		#ifdef EFFECTSYSTEMTIMINGON
		QueryPerformanceCounter( &EndTime );
		TotalTime[i] += ( (double)( EndTime.LowPart - StartTime.LowPart ) / (double)( Frequency.LowPart ) );
		#endif
	}

	// do a partcle system frame
	Particle_SystemFrame( System->Resource.PS, TimeDelta );

	// output extra info
	#ifdef EFFECTSYSTEMTIMINGON
	{

		// locals
		int	TotalEffects = 0;
		int	Row = 0;
		double	CompleteTotalTime = 0.0f;

		// determine total time
		for ( i = 1; i < EffectCount; i++ )
		{
			CompleteTotalTime += TotalTime[i];
		}

		// output effect summary
		for ( i = 1; i < EffectCount; i++ )
		{
			if ( System->ActiveEffects[i] > 0 )
			{
				jeEngine_Printf( System->Resource.Engine, 0, Row++ * 10, "%s, %d, %.2fms, %.0f%%", EffectInterfaceList[i]->GetName(), System->ActiveEffects[i], TotalTime[i] * 1000, ( TotalTime[i] / CompleteTotalTime ) * 100.0f );
			}
			TotalEffects += System->ActiveEffects[i];
		}
		jeEngine_Printf( System->Resource.Engine, 0, Row++ * 10, "Total: %d, %.2fms", TotalEffects, CompleteTotalTime * 1000 );
	}
	#endif

	// all done
	return JE_TRUE;

} // jeEffect_SystemFrame()



////////////////////////////////////////////////////////////////////////////////////////
//
//	jeEffect_StillExists()
//
//	Returns true if the effect is still alive, false if it isn't.
//
////////////////////////////////////////////////////////////////////////////////////////
jeBoolean jeEffect_StillExists(
	EffectSystem	*System,	// effect system to search in
	int				ID )		// effect id to verify
{

	// locals
	void	*Data;

	// ensure valid data
	assert( System != NULL );

	// get effect data
	Data = IndexList_GetElement( System->EffectList, ID );

	// effect exists...
	if ( Data != NULL )
	{
		return JE_TRUE;
	}
	// ...or it doesn't exist
	else
	{
		return JE_FALSE;
	}

} // jeEffect_StillExists()



////////////////////////////////////////////////////////////////////////////////////////
//
//	jeEffect_Modify()
//
//	Modify an effect.
//
////////////////////////////////////////////////////////////////////////////////////////
jeBoolean jeEffect_Modify(
	EffectSystem	*System,	// effect system in which it exists
	EffectType		Type,		// type of effect
	void			*Data,		// new effect data
	int				ID,			// id of effect to modify
	uint32			Flags )		// how to use the new data
{

	// locals
	EffectRoot			*Root = NULL;
	Effect_Interface	*Effect;

	// ensure valid data
	assert( System != NULL );
	assert( System->EffectList != NULL );
	assert( Data != NULL );

	// get root data
	Root = IndexList_GetElement( System->EffectList, ID );
	if ( Root == NULL )
	{
		return JE_FALSE;
	}
	assert( Root->Type == Type );

	// pick the effect interface that we will use
	Effect = EffectInterfaceList[Root->Type];
	assert( Effect != NULL );

	// modify it
	assert( *( (int *)( Root->Data ) ) == Type );
	Effect->Modify( System->ResourceList[Root->Type], Root->Data, Data, Flags );

	// all done
	return JE_TRUE;
	
	// get rid of warnings
	Type;

} // jeEffect_Modify()



////////////////////////////////////////////////////////////////////////////////////////
//
//	jeEffect_SetPause()
//
//	Pause/unpause an effect.
//
////////////////////////////////////////////////////////////////////////////////////////
jeBoolean jeEffect_SetPause(
	EffectSystem	*System,	// effect system in which it exists
	int				ID,			// id of effect to modify
	jeBoolean		Pause )		// new pause state
{

	// locals
	EffectRoot			*Root = NULL;
	Effect_Interface	*Effect;

	// ensure valid data
	assert( System != NULL );
	assert( System->EffectList != NULL );

	// get root data
	Root = IndexList_GetElement( System->EffectList, ID );
	if ( Root == NULL )
	{
		return JE_FALSE;
	}

	// pick the effect interface that we will use
	Effect = EffectInterfaceList[Root->Type];
	assert( Effect != NULL );

	// set pause flag
	Root->Paused = Pause;

	// call the effect pause function
	Effect->Pause( System->ResourceList[Root->Type], Root->Data, Pause );

	// all done
	return JE_TRUE;

} // jeEffect_SetPause()



////////////////////////////////////////////////////////////////////////////////////////
//
//	jeEffect_GetEngine()
//
//	Get a pointer to the engine that this effect system is tied to.
//
////////////////////////////////////////////////////////////////////////////////////////
jeEngine * jeEffect_GetEngine(
	EffectSystem	*ESystem )	// effect system whose engine we want
{

	// ensure valid data
	assert( ESystem != NULL );

	// return its engine
	assert( ESystem->Resource.Engine != NULL );
	return ESystem->Resource.Engine;

} // jeEffect_GetEngine()



////////////////////////////////////////////////////////////////////////////////////////
//
//	jeEffect_GetWorld()
//
//	Get a pointer to the world that this effect system is tied to.
//
////////////////////////////////////////////////////////////////////////////////////////
jeWorld * jeEffect_GetWorld(
	EffectSystem	*ESystem )	// effect system whose world we want
{

	// ensure valid data
	assert( ESystem != NULL );

	// return its world
	assert( ESystem->Resource.World != NULL );
	return ESystem->Resource.World;

} // jeEffect_GetWorld()



////////////////////////////////////////////////////////////////////////////////////////
//
//	jeEffect_GetCamera()
//
//	Get a pointer to the camera that this effect system is tied to.
//
////////////////////////////////////////////////////////////////////////////////////////
jeCamera * jeEffect_GetCamera(
	EffectSystem	*ESystem )	// effect system whose camera we want
{

	// ensure valid data
	assert( ESystem != NULL );

	// return its camera
	assert( ESystem->Resource.Camera != NULL );
	return ESystem->Resource.Camera;

} // jeEffect_GetCamera()
