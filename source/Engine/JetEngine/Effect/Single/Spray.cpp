/****************************************************************************************/
/*  SPRAY.C                                                                             */
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
//	Spray effect.
//
//	Create a particle spray.
//	Does not support attachments.
//	
#include <memory.h>
#include <assert.h>
#include "EffectI.h"
#include "EffectC.h"
#include "Ram.h"
#include "Errorlog.h"
#include "Spray.h"


////////////////////////////////////////////////////////////////////////////////////////
//	Resource struct
////////////////////////////////////////////////////////////////////////////////////////
typedef struct
{
	EffectResource	*ExternalResource;	// external resource list
	int				TypeID;				// effect type id

} SprayResource;


////////////////////////////////////////////////////////////////////////////////////////
//	Interface setup
////////////////////////////////////////////////////////////////////////////////////////
static char *		Spray_GetName( void );
static void *		Spray_Create( EffectResource *Resource, int TypeID );
static void			Spray_Destroy( void ** ResourceToZap );
static void *		Spray_Add( SprayResource *Resource, Spray *Data, Spray *AttachData );
static void			Spray_Remove( SprayResource *Resource, Spray *Data );
static jeBoolean	Spray_Process( SprayResource *Resource, float TimeDelta, Spray *Data );
static jeBoolean	Spray_Frame( SprayResource *Resource, float TimeDelta );
static jeBoolean	Spray_Modify( SprayResource *Resource, Spray *Data, Spray *NewData, uint32 Flags );
static void			Spray_Pause( SprayResource *Resource, Spray *Data, jeBoolean Pause );
Effect_Interface	Spray_Interface =
{
	Spray_GetName,
	Spray_Create,
	Spray_Destroy,
	Spray_Add,
	Spray_Remove,
	Spray_Process,
	Spray_Frame,
	Spray_Modify,
	Spray_Pause,
	sizeof( Spray )
};



////////////////////////////////////////////////////////////////////////////////////////
//
//	Spray_GetName()
//
////////////////////////////////////////////////////////////////////////////////////////
static char * Spray_GetName(
	void )	// no parameters
{
	return "Spray";

} // Spray_GetName()



////////////////////////////////////////////////////////////////////////////////////////
//
//	Spray_Create()
//
////////////////////////////////////////////////////////////////////////////////////////
static void * Spray_Create(
	EffectResource	*ExternalResource,	// external resources
	int				TypeID )			// effect type id
{

	// locals
	SprayResource	*Resource;

	// ensure valid data
	assert( ExternalResource != NULL );
	assert( TypeID > 0 );

	// allocate the resource list struct
	Resource = jeRam_AllocateClear( sizeof( *Resource ) );
	if ( Resource == NULL )
	{
		jeErrorLog_AddString( JE_ERR_MEMORY_RESOURCE, "Spray_Create: failed to create SprayResource struct.", NULL );
		return NULL;
	}

	// save passed resource list
	Resource->ExternalResource = ExternalResource;

	// save type id
	Resource->TypeID = TypeID;

	// all done
	return Resource;

} // Spray_Create()



////////////////////////////////////////////////////////////////////////////////////////
//
//	Spray_Destroy()
//
////////////////////////////////////////////////////////////////////////////////////////
static void Spray_Destroy(
	void	**ResourceToZap )	// resource list 
{

	// locals
	SprayResource	*Resource;

	// ensure valid data
	assert( ResourceToZap != NULL );
	assert( *ResourceToZap != NULL );

	// get resource list
	Resource = *ResourceToZap;

	// zap resource list
	jeRam_Free( Resource );
	ResourceToZap = NULL;

} // Spray_Destroy()



////////////////////////////////////////////////////////////////////////////////////////
//
//	Spray_Add()
//
////////////////////////////////////////////////////////////////////////////////////////
static void * Spray_Add(
	SprayResource	*Resource,		// available resources
	Spray			*Data,			// data of effect
	Spray			*AttachData )	// data of effect to attach to
{

	// locals
	Spray		*NewData;
	jeBoolean	Result;

	// ensure valid data
	assert( Resource != NULL );
	assert( Data != NULL );

	// fail if type ids don't match
	if ( Resource->TypeID != Data->TypeID )
	{
		jeErrorLog_AddString( JE_ERR_INVALID_PARMS, "Spray_Add: effect struct and effect type don't match.", NULL );
		return NULL;
	}

	// allocate Spray data
	NewData = jeRam_Allocate( sizeof( *NewData ) );
	if ( NewData == NULL )
	{
		jeErrorLog_AddString( JE_ERR_MEMORY_RESOURCE, "Spray_Add: failed to create Spray struct.", NULL );
		return NULL;
	}

	// copy passed data
	memcpy( NewData, Data, sizeof( *NewData ) );

	// fail if we have bad data
	if ( EffectC_IsColorGood( &( NewData->ColorMin ) ) == JE_FALSE )
	{
		jeErrorLog_AddString( JE_ERR_INVALID_PARMS, "Spray_Add: effect not added, ColorMin is bad.", NULL );
		return NULL;
	}
	if ( EffectC_IsColorGood( &( NewData->ColorMax ) ) == JE_FALSE )
	{
		jeErrorLog_AddString( JE_ERR_INVALID_PARMS, "Spray_Add: effect not added, ColorMax is bad.", NULL );
		return NULL;
	}
	if ( NewData->SprayLife < 0.0f )
	{
		jeErrorLog_AddString( JE_ERR_INVALID_PARMS, "Spray_Add: effect not added, SprayLife is negative.", NULL );
		return NULL;
	}
	if ( NewData->DistanceMin < 0.0f )
	{
		jeErrorLog_AddString( JE_ERR_INVALID_PARMS, "Spray_Add: effect not added, DistanceMin is negative.", NULL );
		return NULL;
	}
	if ( NewData->DistanceMax < NewData->DistanceMin )
	{
		jeErrorLog_AddString( JE_ERR_INVALID_PARMS, "Spray_Add: effect not added, DistanceMax is less than DistanceMin.", NULL );
		return NULL;
	}
	if ( NewData->Texture == NULL )
	{
		jeErrorLog_AddString( JE_ERR_INVALID_PARMS, "Spray_Add: effect not added, no texture provided.", NULL );
		return NULL;
	}
	if ( NewData->Rate <= 0.0f )
	{
		jeErrorLog_AddString( JE_ERR_INVALID_PARMS, "Spray_Add: effect not added, Rate is negative.", NULL );
		return NULL;
	}
	if ( NewData->MinScale <= 0.0f )
	{
		jeErrorLog_AddString( JE_ERR_INVALID_PARMS, "Spray_Add: effect not added, MinScale is <= 0.", NULL );
		return NULL;
	}
	if ( NewData->MaxScale < NewData->MinScale )
	{
		jeErrorLog_AddString( JE_ERR_INVALID_PARMS, "Spray_Add: effect not added, MaxScale is less than MinScale.", NULL );
		return NULL;
	}
	if ( NewData->MinSpeed < 0.0f )
	{
		jeErrorLog_AddString( JE_ERR_INVALID_PARMS, "Spray_Add: effect not added, MinSpeed is negative.", NULL );
		return NULL;
	}
	if ( NewData->MaxSpeed < NewData->MinSpeed )
	{
		jeErrorLog_AddString( JE_ERR_INVALID_PARMS, "Spray_Add: effect not added, MaxSpeed is less than MinSpeed.", NULL );
		return NULL;
	}
	if ( NewData->MinUnitLife <= 0.0f )
	{
		jeErrorLog_AddString( JE_ERR_INVALID_PARMS, "Spray_Add: effect not added, MinUnitLife is <= 0.", NULL );
		return NULL;
	}
	if( NewData->MaxUnitLife < NewData->MinUnitLife )
	{
		jeErrorLog_AddString( JE_ERR_INVALID_PARMS, "Spray_Add: effect not added, MaxUnitLife is less than MinUnitLife.", NULL );
		return NULL;
	}
	if ( NewData->SourceVariance < 0 )
	{
		jeErrorLog_AddString( JE_ERR_INVALID_PARMS, "Spray_Add: effect not added, SourceVariance is negative.", NULL );
		return NULL;
	}
	if( NewData->DestVariance < 0 )
	{
		jeErrorLog_AddString( JE_ERR_INVALID_PARMS, "Spray_Add: effect not added, DestVariance is negative.", NULL );
		return NULL;
	}

	// setup defaults
	NewData->TimeRemaining = 0.0f;
	NewData->PolyCount = 0.0f;

	// setup particle gravity
	if ( jeVec3d_Length( &( NewData->Gravity ) ) > 0.0f )
	{
		NewData->ParticleGravity = &( NewData->Gravity );
	}
	else
	{
		NewData->ParticleGravity = NULL;
	}

	// save the transform
	EffectC_XFormFromVector( &( NewData->Source ), &( NewData->Dest ), &( NewData->Xf ) );

	// setup default vertex data
	NewData->Vertex.u = 0.0f;
	NewData->Vertex.v = 0.0f;
	NewData->Vertex.r = 255.0f;
	NewData->Vertex.g = 255.0f;
	NewData->Vertex.b = 255.0f;

	// calculate leaf value
	Result = jeWorld_GetLeaf( Resource->ExternalResource->World, &( NewData->Source ), &( NewData->Leaf ) );
	assert( Result == JE_TRUE );

	// all done
	return NewData;

	// get rid of warnings
	AttachData;

} // Spray_Add()



////////////////////////////////////////////////////////////////////////////////////////
//
//	Spray_Remove()
//
////////////////////////////////////////////////////////////////////////////////////////
static void Spray_Remove(
	SprayResource	*Resource,	// available resources
	Spray			*Data )		// effect data
{

	// ensure valid data
	assert( Resource != NULL );
	assert( Data != NULL );

	// remove particle anchor
	if ( Data->AnchorPoint != NULL )
	{
		assert( Resource->ExternalResource != NULL );
		assert( Resource->ExternalResource->PS != NULL );
		Particle_SystemRemoveAnchorPoint( Resource->ExternalResource->PS, Data->AnchorPoint );
	}

	// free effect data
	jeRam_Free( Data );

	// get rid of warnings
	Resource;

} // Spray_Remove()



////////////////////////////////////////////////////////////////////////////////////////
//
//	Spray_Process()
//
//	Perform processing on an indivual effect. A return of JE_FALSE means that the
//	effect needs to be removed.
//
////////////////////////////////////////////////////////////////////////////////////////
static jeBoolean Spray_Process(
	SprayResource	*Resource,		// available resources
	float			TimeDelta,		// elapsed time
	Spray			*Data )			// effect data
{

	// locals
	jeVec3d		Velocity;
	jeVec3d		Left, Up;
	jeVec3d		Source, Dest;
	jeXForm3d	CameraXf;
	float		Scale;
	float		UnitLife;
	float		Distance;
	float		Adjustment = 1.0f;
	float		NewPolyCount = 0.0f;

	// ensure valid data
	assert( Resource != NULL );
	assert( TimeDelta > 0.0f );
	assert( Data != NULL );

	// adjust spray life, killing it if required
	if ( Data->SprayLife > 0.0f )
	{
		Data->SprayLife -= TimeDelta;
		if ( Data->SprayLife <= 0.0f )
		{
			return JE_FALSE;
		}
	}

	// do nothing if its paused
	if ( Data->Paused == JE_TRUE )
	{
		return JE_TRUE;
	}

	// do nothing if it isn't visible
	if ( EffectC_IsPointVisible(	Resource->ExternalResource->World,
									Resource->ExternalResource->Camera,
									&( Data->Source ),
									Data->Leaf,
									EFFECTC_CLIP_LEAF | EFFECTC_CLIP_SEMICIRCLE ) == JE_FALSE )
	{
		return JE_TRUE;
	}

	// get camera xform
	jeCamera_GetXForm( Resource->ExternalResource->Camera, &CameraXf );

	// perform level of detail processing if required
	if ( Data->DistanceMax > 0.0f )
	{

		// do nothing if its too far away
		Distance = jeVec3d_DistanceBetween( &( Data->Source ), &( CameraXf.Translation ) );
		if ( Distance > Data->DistanceMax )
		{
			return JE_TRUE;
		}

		// determine polygon adjustment amount
		if ( ( Data->DistanceMin > 0.0f ) && ( Distance > Data->DistanceMin ) )
		{
			Adjustment = ( 1.0f - ( ( Distance - Data->DistanceMin ) / ( Data->DistanceMax - Data->DistanceMin ) ) );
		}
	}

	// determine how many polys need to be added taking level fo detail into account
	Data->TimeRemaining += TimeDelta;
	while ( Data->TimeRemaining >= Data->Rate )
	{
		Data->TimeRemaining -= Data->Rate;
		NewPolyCount += 1.0f;
	}
	assert( Adjustment >= 0.0f );
	assert( Adjustment <= 1.0f );
	NewPolyCount *= Adjustment;
	Data->PolyCount += NewPolyCount;

	// add new textures
	while ( Data->PolyCount > 0 )
	{

		// adjust poly remaining count
		Data->PolyCount -= 1.0f;

		// pick a source point
		if ( Data->SourceVariance > 0 )
		{
			jeXForm3d_GetLeft( &( Data->Xf ), &Left );
			jeXForm3d_GetUp( &( Data->Xf ), &Up );
			jeVec3d_Scale( &Left, (float)Data->SourceVariance * EffectC_Frand( -1.0f, 1.0f ), &Left );
			jeVec3d_Scale( &Up, (float)Data->SourceVariance * EffectC_Frand( -1.0f, 1.0f ), &Up );
			jeVec3d_Add( &Left, &Up, &Source );
			jeVec3d_Add( &( Data->Source ), &Source, &Source );
		}
		else
		{
			jeVec3d_Copy( &( Data->Source ), &Source );
		}
		Data->Vertex.X = Source.X;
		Data->Vertex.Y = Source.Y;
		Data->Vertex.Z = Source.Z;

		// pick a destination point
		if ( Data->DestVariance > 0 )
		{
			jeXForm3d_GetLeft( &( Data->Xf ), &Left );
			jeXForm3d_GetUp( &( Data->Xf ), &Up );
			jeVec3d_Scale( &Left, (float)Data->DestVariance * EffectC_Frand( -1.0f, 1.0f ), &Left );
			jeVec3d_Scale( &Up, (float)Data->DestVariance * EffectC_Frand( -1.0f, 1.0f ), &Up );
			jeVec3d_Add( &Left, &Up, &Dest );
			jeVec3d_Add( &( Data->Dest ), &Dest, &Dest );
		}
		else
		{
			jeVec3d_Copy( &( Data->Dest ), &Dest );
		}

		// set velocity
		if ( Data->MinSpeed > 0.0f )
		{
			jeVec3d_Subtract( &Dest, &Source, &Velocity );
			jeVec3d_Normalize( &Velocity );
			jeVec3d_Scale( &Velocity, EffectC_Frand( Data->MinSpeed, Data->MaxSpeed ), &Velocity );
		}
		else
		{
			jeVec3d_Set( &Velocity, 0.0f, 0.0f, 0.0f );
		}

		// set scale
		Scale = EffectC_Frand( Data->MinScale, Data->MaxScale );

		// set life
		UnitLife = EffectC_Frand( Data->MinUnitLife, Data->MaxUnitLife );

		// setup color
		Data->Vertex.r = EffectC_Frand( Data->ColorMin.r, Data->ColorMax.r );
		Data->Vertex.g = EffectC_Frand( Data->ColorMin.g, Data->ColorMax.g );
		Data->Vertex.b = EffectC_Frand( Data->ColorMin.b, Data->ColorMax.b );
		Data->Vertex.a = EffectC_Frand( Data->ColorMin.a, Data->ColorMax.a );

		// add the new particle
		Particle_SystemAddParticle(	Resource->ExternalResource->PS,
									Data->Texture,
									&( Data->Vertex ),
									Data->AnchorPoint,
									UnitLife,
									&Velocity,
									Scale,
									Data->ParticleGravity );
	}

	// all done
	return JE_TRUE;

} // Spray_Process()



////////////////////////////////////////////////////////////////////////////////////////
//
//	Spray_Frame()
//
//	Perform once-per-frame processing for all effects of this type.
//
////////////////////////////////////////////////////////////////////////////////////////
static jeBoolean Spray_Frame(
	SprayResource	*Resource,		// available resources
	float			TimeDelta )		// elapsed time
{

	// all done
	return JE_TRUE;

	// get rid of warnings
	Resource;
	TimeDelta;

} // Spray_Frame()



////////////////////////////////////////////////////////////////////////////////////////
//
//	Spray_Modify()
//
//	Adjust the effect.
//
////////////////////////////////////////////////////////////////////////////////////////
jeBoolean Spray_Modify(
	SprayResource	*Resource,	// available resources
	Spray			*Data,		// effect data
	Spray			*NewData,	// new data
	uint32			Flags )		// user flags
{

	// locals
	jeBoolean	Result;
	jeBoolean	RecalculateLeaf = JE_FALSE;

	// ensure valid data
	assert( Resource != NULL );
	assert( Data != NULL );
	assert( NewData != NULL );

	// adjust source and dest together
	if ( Flags & SPRAY_FOLLOWTAIL )
	{
		jeVec3d_Copy( &( Data->Source ), &( Data->Dest ) );
		jeVec3d_Copy( &( NewData->Source ), &( Data->Source ) );
		RecalculateLeaf = JE_TRUE;
	}

	// adjust source
	if ( Flags & SPRAY_SOURCE )
	{
		jeVec3d_Copy( &( NewData->Source ), &( Data->Source ) );
		RecalculateLeaf = JE_TRUE;
	}

	// adjust source
	if ( Flags & SPRAY_DEST )
	{
		jeVec3d_Copy( &( NewData->Dest ), &( Data->Dest ) );
		RecalculateLeaf = JE_TRUE;
	}

	// calculate leaf value
 	if ( RecalculateLeaf == JE_TRUE )
	{
		EffectC_XFormFromVector( &( Data->Source ), &( Data->Dest ), &( Data->Xf ) );
		Result = jeWorld_GetLeaf( Resource->ExternalResource->World, &( Data->Source ), &( Data->Leaf ) );
		assert( Result == JE_TRUE );
	}

	// all done
	return JE_TRUE;

} // Spray_Modify()



////////////////////////////////////////////////////////////////////////////////////////
//
//	Spray_Pause()
//
//	Pause/unpause effect.
//
////////////////////////////////////////////////////////////////////////////////////////
void Spray_Pause(
	SprayResource	*Resource,	// available resources
	Spray			*Data,		// effect data
	jeBoolean		Pause )		// new pause state
{

	// ensure valid data
	assert( Resource != NULL );
	assert( Data != NULL );

	// set pause flag
	Data->Paused = Pause;

	// get rid of warnings
	Resource;

} // Spray_Pause()
