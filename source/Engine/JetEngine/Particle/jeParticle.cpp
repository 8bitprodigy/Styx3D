/****************************************************************************************/
/*  JEPARTICLE.C                                                                        */
/*                                                                                      */
/*  Author:  Eli Boling & Peter Siamidis                                                */
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
#include <memory.h>
#include <assert.h>
#include "Ram.h"
#include "jeUserPoly.h"
#include "jeParticle.h"
#include "jeVersion.h"


////////////////////////////////////////////////////////////////////////////////////////
//	Particle attricute flags
////////////////////////////////////////////////////////////////////////////////////////
#define	PARTICLE_HASVELOCITY	( 1 << 1 )
#define PARTICLE_HASGRAVITY		( 1 << 2 )


////////////////////////////////////////////////////////////////////////////////////////
//	jeParticle struct
////////////////////////////////////////////////////////////////////////////////////////
typedef struct jeParticle
{
	jeLVertex		ptclVertex;
	jeMaterialSpec	*ptclMaterial;
	jeUserPoly		*ptclPoly;
	unsigned		ptclFlags;
	jeParticle		*ptclNext;
	jeParticle		*ptclPrev;
	float			Scale;
	jeVec3d			Gravity;
	float			Alpha;
	jeVec3d			CurrentAnchorPoint;
	const jeVec3d	*AnchorPoint;
	float			ptclTime;
	float			ptclTotalTime;
	jeVec3d			ptclVelocity;
	jeWorld			*World;

} jeParticle;


////////////////////////////////////////////////////////////////////////////////////////
//	Particle system struct
////////////////////////////////////////////////////////////////////////////////////////
typedef	struct jeParticle_System
{
	jeParticle	*psParticles;

} jeParticle_System;



////////////////////////////////////////////////////////////////////////////////////////
//
//	jeParticle_SystemCreate()
//
//	Create a particle system.
//
////////////////////////////////////////////////////////////////////////////////////////
JETAPI jeParticle_System * JETCC jeParticle_SystemCreate(
	float	MajorVersion,	// major version number of calling app
	float	MinorVersion )	// minor version number of calling app
{

	// locals
	jeParticle_System	*ps;

	// fail if version numbers don't match
	if ( ( MajorVersion != JET_MAJOR_VERSION ) || ( MinorVersion != JET_MINOR_VERSION ) )
	{
		return NULL;
	}

	// allocate struct
	ps = (jeParticle_System *)jeRam_AllocateClear( sizeof( *ps ) );
	if ( ps == NULL )
	{
		return ps;
	}

	// all done
	return ps;

} // jeParticle_SystemCreate()



////////////////////////////////////////////////////////////////////////////////////////
//
//	jeParticle_Destroy()
//
//	Destroy a particle.
//
////////////////////////////////////////////////////////////////////////////////////////
static void jeParticle_Destroy(
	jeParticle	*p )	// particle to destroy
{

	// destroy the poly
	if ( p->ptclPoly != NULL )
	{
		jeWorld_RemoveUserPoly( p->World, p->ptclPoly );
		jeUserPoly_Destroy( &( p->ptclPoly ) );
	}

	// free the struct
	jeRam_Free( p );

} // jeParticle_Destroy()



////////////////////////////////////////////////////////////////////////////////////////
//
//	jeParticle_Unlink()
//
//	Remove a particle from the list of particles.
//
////////////////////////////////////////////////////////////////////////////////////////
static void jeParticle_Unlink(
	jeParticle_System	*ps,		// particle system in which particle exists
	jeParticle			*ptcl )		// particle to unlink
{

	// make list adjustments
	if ( ptcl->ptclPrev )
	{
		ptcl->ptclPrev->ptclNext = ptcl->ptclNext;
	}
	if ( ptcl->ptclNext )
	{
		ptcl->ptclNext->ptclPrev = ptcl->ptclPrev;
	}
	if ( ps->psParticles == ptcl )
	{
		ps->psParticles = ptcl->ptclNext;
	}

} // jeParticle_Unlink()



////////////////////////////////////////////////////////////////////////////////////////
//
//	jeParticle_Create()
//
//	Create a new particle.
//
////////////////////////////////////////////////////////////////////////////////////////
static jeParticle * jeParticle_Create(
	jeParticle_System	*ps,		// particle system to create it in
	jeMaterialSpec		*Texture,	// texture to use
	const jeLVertex		*Vert )		// vert data
{

	// locals
	jeParticle	*ptcl;

	// allocate struct
	ptcl = (jeParticle *)jeRam_AllocateClear( sizeof( *ptcl ) );
	if ( ptcl == NULL )
	{
		return ptcl;
	}

	// init struct
	ptcl->ptclNext = ps->psParticles;
	ps->psParticles = ptcl;
	if ( ptcl->ptclNext )
	{
		ptcl->ptclNext->ptclPrev = ptcl;
	}
	ptcl->ptclMaterial = Texture;
	ptcl->ptclVertex = *Vert;

	// all done
	return ptcl;

} // jeParticle_Create()



////////////////////////////////////////////////////////////////////////////////////////
//
//	jeParticle_SystemDestroy()
//
//	Destroy a particle system.
//
////////////////////////////////////////////////////////////////////////////////////////
JETAPI void JETCC jeParticle_SystemDestroy(
	jeParticle_System	*ps )	// particle system to destroy
{

	// locals
	jeParticle	*ptcl;

	// zap all particles
	ptcl = ps->psParticles;
	while ( ptcl != NULL )
	{

		// locals
		jeParticle	*temp;

		// destroy this particle
		temp = ptcl->ptclNext;
		jeParticle_Destroy( ptcl );
		ptcl = temp;
	}

	// free particle system
	jeRam_Free( ps );

} // jeParticle_SystemDestroy()



////////////////////////////////////////////////////////////////////////////////////////
//
//	jeParticle_SystemRemoveAll()
//
//	Destroy all particles.
//
////////////////////////////////////////////////////////////////////////////////////////
JETAPI void JETCC jeParticle_SystemRemoveAll(
	jeParticle_System	*ps )	// particle system whose particles will all be destroyed
{

	// locals
	jeParticle	*ptcl;

	// zap all particles
	ptcl = ps->psParticles;
	while ( ptcl != NULL )
	{

		// locals
		jeParticle	*temp;

		// kill this particle and go to next one
		temp = ptcl->ptclNext;
		jeParticle_Unlink( ps, ptcl );
		jeParticle_Destroy( ptcl );
		ptcl = temp;
	}

} // jeParticle_SystemRemoveAll()



////////////////////////////////////////////////////////////////////////////////////////
//
//	jeParticle_GetCount()
//
//	Return the current number of active particles.
//
////////////////////////////////////////////////////////////////////////////////////////
JETAPI int JETCC jeParticle_GetCount(
	jeParticle_System	*ps )	// particle system whose particle count we want
{

	// locals
	jeParticle	*ptcl;
	int			TotalParticleCount = 0;

	// ensure valid data
	assert( ps != NULL );

	// count up how many particles are active in this particle system
	ptcl = ps->psParticles;
	while ( ptcl )
	{
		ptcl = ptcl->ptclNext;
		TotalParticleCount++;
	}

	// return the active count
	return TotalParticleCount;

} // jeParticle_GetCount()



////////////////////////////////////////////////////////////////////////////////////////
//
//	jeParticle_SystemFrame()
//
//	Process a frame of the particle system.
//
////////////////////////////////////////////////////////////////////////////////////////
JETAPI void JETCC jeParticle_SystemFrame(
	jeParticle_System	*ps,			// particle system to process
	float				DeltaTime )		// amount of elaped seconds
{

	// locals
	jeParticle	*ptcl;

	// process all particles
	ptcl = ps->psParticles;
	while ( ptcl != NULL )
	{

		// adjust particles life remaining
		ptcl->ptclTime -= DeltaTime;

		// destroy the particle..
		if ( ptcl->ptclTime <= 0.0f )
		{

			// locals
			jeParticle	*temp;

			// destroy this particle
			temp = ptcl->ptclNext;
			jeParticle_Unlink( ps, ptcl );
			jeParticle_Destroy( ptcl );
			ptcl = temp;
			continue;
		}
		// ...or process it
		else
		{

			// locals
			jeVec3d	DeltaPos = { 0.0f, 0.0f, 0.0f };

			// apply velocity
			if ( ptcl->ptclFlags & PARTICLE_HASVELOCITY )
			{
				jeVec3d_Scale( &( ptcl->ptclVelocity ), DeltaTime, &DeltaPos );
			}

			// apply gravity
			if ( ptcl->ptclFlags & PARTICLE_HASGRAVITY )
			{

				// locals
				jeVec3d	Gravity;

				// make gravity vector
				jeVec3d_Scale( &( ptcl->Gravity ), DeltaTime, &Gravity );

				// apply gravity to built in velocity and DeltaPos
				jeVec3d_Add( &( ptcl->ptclVelocity ), &Gravity, &( ptcl->ptclVelocity ) );
				jeVec3d_Add( &DeltaPos, &Gravity, &DeltaPos );
			}
			
			// apply DeltaPos to particle position
			if (	( ptcl->ptclFlags & PARTICLE_HASVELOCITY ) ||
					( ptcl->ptclFlags & PARTICLE_HASGRAVITY ) )
			{
				jeVec3d_Add( (jeVec3d *)&( ptcl->ptclVertex.X ), &DeltaPos, (jeVec3d *)&( ptcl->ptclVertex.X ) );
			}

			// make the particle follow its anchor point if it has one
			if ( ptcl->AnchorPoint != NULL )
			{

				// locals
				jeVec3d	AnchorDelta;

				jeVec3d_Subtract( ptcl->AnchorPoint, &( ptcl->CurrentAnchorPoint ), &AnchorDelta );
				jeVec3d_Add( (jeVec3d *)&( ptcl->ptclVertex.X ), &AnchorDelta, (jeVec3d *)&( ptcl->ptclVertex.X ) );
				jeVec3d_Copy( ptcl->AnchorPoint, &( ptcl->CurrentAnchorPoint ) );
			}

			// destroy the particle if it is in solid space
			//undone
		}

		// adjust particle alpha
		assert( ptcl->ptclTotalTime > 0.0f );
		assert( ptcl->ptclPoly != NULL );
		ptcl->ptclVertex.a = ptcl->Alpha * ( ptcl->ptclTime / ptcl->ptclTotalTime );
		assert( ptcl->ptclVertex.a >= 0.0f );
		assert( ptcl->ptclVertex.a <= 255.0f );
		jeUserPoly_UpdateSprite( ptcl->ptclPoly, &ptcl->ptclVertex, ptcl->ptclMaterial, ptcl->Scale );

		// get next particle
		ptcl = ptcl->ptclNext;
	}

} // jeParticle_SystemFrame()



////////////////////////////////////////////////////////////////////////////////////////
//
//	jeParticle_SystemRemoveAnchorPoint()
//
//	Removes all references to an anchor point.
//
////////////////////////////////////////////////////////////////////////////////////////
JETAPI jeBoolean JETCC jeParticle_SystemRemoveAnchorPoint(
	jeParticle_System	*ps,				// particle system from which this anchor point will be removed
	jeVec3d				*AnchorPoint )		// the anchor point to remove
{

	// locals	
	jeParticle	*ptcl;
	jeBoolean	AtLeastOneFound = JE_FALSE;

	// ensure valid data
	assert( ps != NULL );
	assert( AnchorPoint != NULL );

	// eliminate achnor point from all particles in this particle system
	ptcl = ps->psParticles;
	while ( ptcl != NULL )
	{
		if ( ptcl->AnchorPoint == AnchorPoint )
		{
			ptcl->AnchorPoint = NULL;
			AtLeastOneFound = JE_TRUE;
		}
		ptcl = ptcl->ptclNext;
	}

	// all done
	return AtLeastOneFound;

} // jeParticle_SystemRemoveAnchorPoint()



////////////////////////////////////////////////////////////////////////////////////////
//
//	jeParticle_SystemAddParticle()
//
//	Add a new paticle to the particle system.
//
////////////////////////////////////////////////////////////////////////////////////////
JETAPI jeBoolean JETCC jeParticle_SystemAddParticle(
	jeParticle_System	*ps,			// particle system to add it to
	const jeWorld		*World,			// world to add it to
	jeMaterialSpec		*Texture,		// texture to use
	const jeLVertex		*Vert,			// vert info
	const jeVec3d		*AnchorPoint,	// anchor point
	float				Time,			// how many seconds it will last
	const jeVec3d		*Velocity,		// velocity
	float				Scale,			// art scale
	const jeVec3d		*Gravity )		// pull of gravity
{

	// locals
	jeParticle	*ptcl;

	// create a new particle
	ptcl = jeParticle_Create( ps, Texture, Vert );
	if ( ptcl == NULL )
	{
		return JE_FALSE;
	}

	// setup gravity
	if ( Gravity != NULL )
	{
		jeVec3d_Copy( Gravity, &( ptcl->Gravity ) );
		ptcl->ptclFlags |= PARTICLE_HASGRAVITY;
	}

	// setup velocity
	if ( Velocity != NULL )
	{
		jeVec3d_Copy( Velocity, &( ptcl->ptclVelocity ) );
		ptcl->ptclFlags |= PARTICLE_HASVELOCITY;
	}

	// setup the anchor point
	if ( AnchorPoint != NULL )
	{
		jeVec3d_Copy( AnchorPoint, &( ptcl->CurrentAnchorPoint ) );
		ptcl->AnchorPoint = AnchorPoint;
	}

	// setup remaining data
	ptcl->Scale = Scale;
	ptcl->ptclTime = Time;
	ptcl->ptclTotalTime = Time;
	ptcl->Alpha = Vert->a;
	ptcl->World = (jeWorld *)World;

	// add the poly to the world
	ptcl->ptclPoly = jeUserPoly_CreateSprite(	&ptcl->ptclVertex,
												ptcl->ptclMaterial,
												ptcl->Scale,
												JE_RENDER_FLAG_ALPHA | JE_RENDER_FLAG_NO_ZWRITE );
	if ( jeWorld_AddUserPoly( ptcl->World, ptcl->ptclPoly, JE_FALSE ) == JE_FALSE )
	{
		jeUserPoly_Destroy( &( ptcl->ptclPoly ) );
		return JE_FALSE;
	}

	// all done
	return JE_TRUE;

} // jeParticle_SystemAddParticle()
