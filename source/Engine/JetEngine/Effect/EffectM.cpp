/****************************************************************************************/
/*  EFFECTM.C                                                                           */
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
//
//	Effect manager, designed to group multiple effects into one type of effect.
//
#pragma warning ( disable : 4514 )
#include <memory.h>
#include <assert.h>
#include "TPool.h"
#include "SPool.h"
#include "Errorlog.h"
#include "Ram.h"
#include "Effect.h"


////////////////////////////////////////////////////////////////////////////////////////
//	Settings and interface headers (IF YOU ADD AN EFFECT ITS HEADERS MUST BE LISTED HERE)
////////////////////////////////////////////////////////////////////////////////////////
#include "SpoutM.h"


//////////////////////////////////////////////////////////////////////////////////////////
//	IF YOU ADD AN EFFECT ITS INTERFACE POINTER MUST BE HERE
//////////////////////////////////////////////////////////////////////////////////////////
EffectM_Interface *EffectMInterfaceList[] =
{
	&SpoutM_Interface,
};
#define EffectMCount	( sizeof( EffectMInterfaceList ) / sizeof( EffectMInterfaceList[0] ) )


////////////////////////////////////////////////////////////////////////////////////////
//	IF YOU ADD AN EFFECT THAT WORKS IN THE EDITOR, ITS NAME MUST BE HERE
////////////////////////////////////////////////////////////////////////////////////////
static char	*EntityEffect[] = 
{
	"Spout",
};
#define EntityEffectCount	( sizeof( EntityEffect ) / sizeof( EntityEffect[0] ) )
/*static int	EntityInfo[EntityEffectCount][2] = 
{
	{ EffectM_Spout, sizeof( EM_Spout ) },
};*/



////////////////////////////////////////////////////////////////////////////////////////
//
//	jeEffectM_CreateManager()
//
//	Create a new effect manager.
//
////////////////////////////////////////////////////////////////////////////////////////
EffectManager * jeEffectM_CreateManager(
	jeEngine		*Engine,		// engine to use
	jeWorld			*World,			// world to use
	jeSound_System	*Sound,			// sound system to use
	jeCamera		*Camera,		// camera to use
	jeSymbol_Table	*EntityTable,	// entity table to use
	void			*Context )		// context data
{

	// locals
	EffectManager	*EManager = NULL;

	// ensure valid data
	assert( Engine != NULL );
	assert( World != NULL );
	assert( Sound != NULL );
	assert( Camera != NULL );
	assert( EntityTable != NULL );

	// allocate struct
	EManager = jeRam_AllocateClear( sizeof( *EManager ) );
	if ( EManager == NULL )
	{
		jeErrorLog_AddString( JE_ERR_MEMORY_RESOURCE, "jeEffectM_CreateManager: failed to create effect manager struct.", NULL );
		return NULL;
	}

	// create linked list to hold active effects
	EManager->EffectList = IndexList_Create( 50, 10 );
	if ( EManager->EffectList == NULL )
	{
		jeErrorLog_AddString( JE_ERR_INTERNAL_RESOURCE, "jeEffectM_CreateManager: failed to create effect list.", NULL );
		jeEffectM_DestroyManager( &EManager );
		return NULL;
	}

	// create an effect system
	EManager->ESystem = jeEffect_SystemCreate( Engine, World, Sound, Camera );
	if ( EManager->ESystem == NULL )
	{
		jeErrorLog_AddString( JE_ERR_INTERNAL_RESOURCE, "jeEffectM_CreateManager: failed to create effect system.", NULL );
		jeEffectM_DestroyManager( &EManager );
		return NULL;
	}

	// create texture pool
	EManager->TPool = TPool_Create( World );
	if ( EManager->TPool == NULL )
	{
		jeErrorLog_AddString( JE_ERR_INTERNAL_RESOURCE, "jeEffectM_CreateManager: failed to create texture pool.", NULL );
		jeEffectM_DestroyManager( &EManager );
		return NULL;
	}

	// create sound pool
	if ( Sound != NULL )
	{
		EManager->SPool = SPool_Create( Sound );
		if ( EManager->SPool == NULL )
		{
			jeErrorLog_AddString( JE_ERR_INTERNAL_RESOURCE, "jeEffectM_CreateManager: failed to create sound pool.", NULL );
			jeEffectM_DestroyManager( &EManager );
			return NULL;
		}
	}

	// save other relevant data
	EManager->EntityTable = EntityTable;
	EManager->Context = Context;

	// all done
	return EManager;

} // jeEffectM_CreateManager()



////////////////////////////////////////////////////////////////////////////////////////
//
//	jeEffectM_DestroyManager()
//
//	Destroy an effect manager.
//
////////////////////////////////////////////////////////////////////////////////////////
void jeEffectM_DestroyManager(
	EffectManager	**EManager )	// manager to zap
{

	// locals
	EffectManager	*DeadEManager;

	// ensure valid data
	assert( EManager != NULL );
	assert( *EManager != NULL );

	// setup effect manager pointer
	DeadEManager = *EManager;

	// free all of the managers effects
	if ( DeadEManager->EffectList != NULL )
	{

		// locals
		int	i;
		int	EffectCount;

		// kill all the effects
		EffectCount = IndexList_GetListSize( DeadEManager->EffectList );
		for ( i = 0; i < EffectCount; i++ )
		{
			jeEffectM_Destroy( DeadEManager, i );
			IndexList_DeleteElement( DeadEManager->EffectList, i );
		}

		// destroy the effect list
		IndexList_Destroy( &( DeadEManager->EffectList ) );
	}

	// destroy the effect system
	if ( DeadEManager->ESystem != NULL )
	{
		jeEffect_SystemDestroy( &( DeadEManager->ESystem ) );
	}

	// destroy the texture pool
	if ( DeadEManager->TPool != NULL )
	{
		TPool_Destroy( &( DeadEManager->TPool ) );
	}

	// destroy the sound pool
	if ( DeadEManager->SPool != NULL )
	{
		SPool_Destroy( &( DeadEManager->SPool ) );
	}

	// zap the effect manager struct
	jeRam_Free( *EManager );
	EManager = NULL;

} // jeEffectM_DestroyManager()



////////////////////////////////////////////////////////////////////////////////////////
//
//	jeEffectM_Delete()
//
//	Destroy one effect of an effect manager.
//
////////////////////////////////////////////////////////////////////////////////////////
static void jeEffectM_Delete(
	EffectManager	*EManager,	// effect manager that it belongs to
	EffectM_Root	**Root )	// root data to wipe
{

	// locals
	EffectM_Root	*DeadRoot;
	int				i;

	// ensure valid data
	assert( EManager != NULL );
	assert( Root != NULL );
	assert( *Root != NULL );

	// setup root pointer
	DeadRoot = *Root;

	// call the effects destroy function
	EffectMInterfaceList[DeadRoot->Type]->Destroy( EManager, DeadRoot );

	// free all of its associated effects and the list itself
	if ( DeadRoot->EffectList != NULL )
	{
		for ( i = 0; i < DeadRoot->EffectCount; i++ )
		{
			if ( DeadRoot->EffectList[i] != -1 )
			{
				jeEffect_Delete( EManager->ESystem, DeadRoot->EffectList[i] );
			}
		}
		jeRam_Free( DeadRoot->EffectList );
	}

	// free any custom data
	if ( DeadRoot->Custom != NULL )
	{
		jeRam_Free( DeadRoot->Custom );
	}

	// free the root
	jeRam_Free( DeadRoot );
	Root = NULL;

} // jeEffectM_Delete()



////////////////////////////////////////////////////////////////////////////////////////
//
//	jeEffectM_Destroy()
//
//	Destroy one effect of an effect manager by id.
//
////////////////////////////////////////////////////////////////////////////////////////
jeBoolean jeEffectM_Destroy(
	EffectManager	*EManager,	// effect manager that it belongs to
	int				Id )		// effect to destroy
{

	// locals
	EffectM_Root	*Root;

	// ensure valid data
	assert( EManager != NULL );
	assert( Id >= 0 );

	// get effect data
	Root = IndexList_GetElement( EManager->EffectList, Id );
	if ( Root == NULL )
	{
		return JE_FALSE;
	}

	// delete the effect
	jeEffectM_Delete( EManager, &Root );

	// all done
	return JE_TRUE;

} // jeEffectM_Destroy()



////////////////////////////////////////////////////////////////////////////////////////
//
//	jeEffectM_Create()
//
//	Create a new effect.
//
////////////////////////////////////////////////////////////////////////////////////////
jeBoolean jeEffectM_Create(
	EffectManager	*EManager,	// effect manager to add it to
	jeSymbol		*Entity,	// enitity which contains the effect data
	int				*Id )		// where to store effect id number
{

	// locals
	EffectM_Root	*Root;
	jeBoolean		Result;
	int				Slot;

	// ensure valid data
	assert( EManager != NULL );
	assert( Entity != NULL );

	// allocate root memory
	Root = jeRam_AllocateClear( sizeof( *Root ) );
	if ( Root == NULL )
	{
		return JE_FALSE;
	}

	// determine what type of effect this is //undone
	Root->Type = 0;
	Root->Entity = Entity;








	// create the effect
	Result = EffectMInterfaceList[Root->Type]->Create( EManager, Root );
	if ( Result == JE_FALSE )
	{
		jeErrorLog_AddString( JE_ERR_INTERNAL_RESOURCE, "jeEffectM_Create: failed to add effect.", NULL );
		jeEffectM_Delete( EManager, &Root );
		return JE_FALSE;
	}

	// add new effect to the list
	Slot = IndexList_GetEmptySlot( EManager->EffectList );
	if ( Slot == -1 )
	{
		jeErrorLog_AddString( JE_ERR_INTERNAL_RESOURCE, "jeEffectM_Create: failed to find empty slot for effect.", NULL );
		jeEffectM_Delete( EManager, &Root );
		return JE_FALSE;
	}
	IndexList_AddElement( EManager->EffectList, Slot, Root );

	// save id number if required
	if ( Id != NULL )
	{
		*Id = Slot;
	}

	// all done
	assert( Slot >= 0 );
	return JE_TRUE;

} // jeEffectM_Create()



////////////////////////////////////////////////////////////////////////////////////////
//
//	jeEffectM_Update()
//
//	Update an effect managers effects.
//
////////////////////////////////////////////////////////////////////////////////////////
int jeEffectM_Update(
	EffectManager	*EManager,		// manager to update
	float			TimeDelta )		// amount of elased time
{

	// locals
	EffectM_Root	*Root;
	jeBoolean		Result;
	int				EffectCount;
	int				i;
	int				EffectsProcessed;

	// ensure valid data
	assert( EManager != NULL );
	assert( TimeDelta > 0.0f );

	// update all effects
	EffectCount = IndexList_GetListSize( EManager->EffectList );
	EffectsProcessed = 0;
	for ( i = 0; i < EffectCount; i++ )
	{

		// get effect data, skipping it if its not active
		Root = IndexList_GetElement( EManager->EffectList, i );
		if ( Root == NULL )
		{
			continue;
		}
		
		// update the effect
		assert ( Root->EffectList != NULL );
		Root->TimeDelta = TimeDelta;
		Result = EffectMInterfaceList[Root->Type]->Update( EManager, Root );

		// delete this effect if required
		if ( Result == JE_FALSE )
		{
			jeEffectM_Delete( EManager, &Root );
			IndexList_DeleteElement( EManager->EffectList, i );
		}

		// adjust effects processed count
		EffectsProcessed++;
	}

	// all done
	return EffectsProcessed;

} // jeEffectM_Update()


/*
////////////////////////////////////////////////////////////////////////////////////////
//
//	EffectM_LoadWorldEffects()
//
////////////////////////////////////////////////////////////////////////////////////////
int EffectM_LoadWorldEffects(
	EffectManager	*EManager,	// effect manager that will own them
	jeWorld			*World )	// world whose entity effects will be loaded
{

	// locals
	jeEntity_EntitySet	*EntitySet;
	jeEntity			*Entity = NULL;
	int32				i;
	void				*Data;
	int32				EffectEntitiesAdded = 0;
	char				ErrorString[256];

	// ensure valid data
	assert( EManager != NULL );
	assert( World != NULL );

	// get entity set
	for ( i = 0; i < EntityEffectCount; i++ )
	{

		// get MenuInfo entity set
		EntitySet =  jeWorld_GetEntitySet( World, EntityEffect[i] );
		if( EntitySet == NULL )
		{
			continue;
		}
		
		// process all entities of the current type
		while ( ( Entity = jeEntity_EntitySetGetNextEntity( EntitySet, Entity ) ) != NULL )
		{

			// get entity data
			Data = jeEntity_GetUserData( Entity );
			assert( Data != NULL );

			// add this effect...
			assert( EntityInfo[i][1] > 0 );
			if ( EffectM_Create( EManager, EntityInfo[i][0], Data, EntityInfo[i][1] ) != -1 )
			{
				EffectEntitiesAdded++;
			}
			// ...or log an error
			else
			{
				sprintf( ErrorString, "Could not add an entity effect of type %d\n", EntityInfo[i][0] );
				jeErrorLog_Add( 0, ErrorString );
			}
		}
	}

	// all done
	return EffectEntitiesAdded;

} // EffectM_LoadWorldEffects()
*/