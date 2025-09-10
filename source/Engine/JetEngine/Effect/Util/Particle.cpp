/****************************************************************************************/
/*  PARTICLE.C                                                                          */
/*                                                                                      */
/*  Author:  Eli Boling and Peter Siamidis                                              */
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
#include "Jet.h"
#include "Ram.h"
#include "particle.h"

#define	POLYQ

#define	PARTICLE_USER			( 1 << 0 )
#define	PARTICLE_HASVELOCITY	( 1 << 1 )
#define PARTICLE_HASGRAVITY		( 1 << 2 )
#define PARTICLE_RENDERSTYLE	JE_RENDER_DEPTH_SORT_BF | JE_RENDER_DO_NOT_OCCLUDE_OTHERS

#define	WORLDTOMETERS(w)	((w) / (jeFloat)32.0f)
#define METERSTOWORLD(m)	((m) * (jeFloat)32.0f)

#define EARTHGRAVITYMAGNITUDE	(((jeFloat)9.81f) / (jeFloat)1.0f)

typedef struct	Particle
{
	JE_LVertex			 	 ptclVertex;
	jeBitmap *		 ptclTexture;
	jeUPoly *				 ptclPoly;
	unsigned 	 	 		 ptclFlags;
	Particle *			 	 ptclNext;
	Particle *			 	 ptclPrev;
	float					 Scale;
	jeVec3d					 Gravity;
	float					Alpha;
	jeVec3d					CurrentAnchorPoint;
	const jeVec3d			*AnchorPoint;

	union
	{
		struct
		{
			jeFloat	 	 ptclTime;
			jeFloat		 ptclTotalTime;
			jeVec3d	 	 ptclVelocity;
		}	builtin;
		struct
		{
			ParticleCallBack ptclCallBack;
			void *			 ptclUserData;
		}	user;
	}	p;
}	Particle;

typedef	struct	Particle_System
{
	Particle *		psParticles;
	jeWorld *		psWorld;
	jeFloat			psLastTime;
	jeFloat			psQuantumSeconds;
}	Particle_System;

void PARTICLE_CALLINGCONVENTION Particle_SetTexture(Particle *p, jeBitmap *Texture)
{
	assert(p->ptclFlags & PARTICLE_USER);
	p->ptclTexture = Texture;
}

JE_LVertex * PARTICLE_CALLINGCONVENTION Particle_GetVertex(Particle *p)
{
	assert(p->ptclFlags & PARTICLE_USER);
	return &p->ptclVertex;
}

void * PARTICLE_CALLINGCONVENTION Particle_GetUserData(Particle  *p)
{
	assert(p->ptclFlags & PARTICLE_USER);
	return p->p.user.ptclUserData;
}

#define	QUANTUMSIZE	(1.0f / 30.0f)

Particle_System * PARTICLE_CALLINGCONVENTION Particle_SystemCreate(jeWorld *World)
{
	Particle_System *	ps;

	ps = (Particle_System *)jeRam_Allocate(sizeof(*ps));
	if	(!ps)
		return ps;

	memset(ps, 0, sizeof(*ps));
	
	ps->psWorld			= World;
	ps->psQuantumSeconds = QUANTUMSIZE;
	ps->psLastTime = 0.0f;

	return ps;
}

static	void PARTICLE_CALLINGCONVENTION	DestroyParticle(Particle_System *ps, Particle *p)
{
	if	(p->ptclFlags & PARTICLE_USER)
	{
		if	(p->p.user.ptclUserData)
			jeRam_Free(p->p.user.ptclUserData);
	}

	if	(p->ptclPoly)
		jeWorld_RemovePoly(ps->psWorld, p->ptclPoly);

	jeRam_Free(p);
}

void PARTICLE_CALLINGCONVENTION	Particle_SystemDestroy(Particle_System *ps)
{
	Particle *	ptcl;

	ptcl = ps->psParticles;
	while	(ptcl)
	{
		Particle *	temp;

		temp = ptcl->ptclNext;
		DestroyParticle(ps, ptcl);
		ptcl = temp;
	}

	jeRam_Free(ps);
}

static	void PARTICLE_CALLINGCONVENTION	UnlinkParticle(Particle_System *ps, Particle *ptcl)
{
	if	(ptcl->ptclPrev)
		ptcl->ptclPrev->ptclNext = ptcl->ptclNext;

	if	(ptcl->ptclNext)
		ptcl->ptclNext->ptclPrev = ptcl->ptclPrev;

	if	(ps->psParticles == ptcl)
		ps->psParticles = ptcl->ptclNext;
}

void PARTICLE_CALLINGCONVENTION Particle_SystemRemoveAll
(Particle_System *ps)
{
	Particle *	ptcl;

	ptcl = ps->psParticles;
	while	(ptcl)
	{
		Particle *	temp;

		temp = ptcl->ptclNext;
		UnlinkParticle(ps, ptcl);
		DestroyParticle(ps, ptcl);
		ptcl = temp;
	}
}

int32 PARTICLE_CALLINGCONVENTION	Particle_GetCount(Particle_System *ps)
{

	// locals
	Particle	*ptcl;
	int32		TotalParticleCount = 0;

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

}

void PARTICLE_CALLINGCONVENTION	Particle_SystemFrame(Particle_System *ps, jeFloat DeltaTime)
{

	jeVec3d	AnchorDelta;
//	jeFloat	DeltaT;
//	unsigned long	CurrentTime;

//	CurrentTime = timeGetTime();
//	ps->psFrameTime += CurrentTime - ps->psLastTime;

//	DeltaT = Time - ps->psLastTime;

//	while	(ps->psFrameTime >= ps->psQuantumSize)

	// the quick fix to the particle no-draw problem 
	ps->psQuantumSeconds = DeltaTime;
	//while	(DeltaTime > QUANTUMSIZE)
	{

		Particle *	ptcl;

		ptcl = ps->psParticles;
		while	(ptcl)
		{
			if	(ptcl->ptclFlags & PARTICLE_USER)
			{
				assert(ptcl->p.user.ptclCallBack);
				if	((ptcl->p.user.ptclCallBack)(ps, ptcl, ps->psQuantumSeconds) == JE_FALSE)
				{
					Particle *	temp;

					temp = ptcl->ptclNext;
					UnlinkParticle(ps, ptcl);
					DestroyParticle(ps, ptcl);
					ptcl = temp;
					continue;
				}
			}
			else
			{
//extern	JE_Engine *		Engine;
//			JE_EnginePrintf(Engine, 2, 40, "%4.2f %4.2f %4.2f", ptcl->p.builtin.ptclVelocity.X, ptcl->p.builtin.ptclVelocity.Y, ptcl->p.builtin.ptclVelocity.Z);
				ptcl->p.builtin.ptclTime -= ps->psQuantumSeconds;
				if	(ptcl->p.builtin.ptclTime <= 0.0f)
				{
					Particle *	temp;

					temp = ptcl->ptclNext;
					UnlinkParticle(ps, ptcl);
					DestroyParticle(ps, ptcl);
					ptcl = temp;
					continue;
				}
				else
				{

					// locals
					jeVec3d		DeltaPos = { 0.0f, 0.0f, 0.0f };

					// apply velocity
					if ( ptcl->ptclFlags & PARTICLE_HASVELOCITY )
					{
						jeVec3d_Scale( &( ptcl->p.builtin.ptclVelocity ), ps->psQuantumSeconds, &DeltaPos );
					}

					// apply gravity
					if ( ptcl->ptclFlags & PARTICLE_HASGRAVITY )
					{

						// locals
						jeVec3d	Gravity;

						// make gravity vector
						jeVec3d_Scale( &( ptcl->Gravity ), ps->psQuantumSeconds, &Gravity );

						// apply gravity to built in velocity and DeltaPos
						jeVec3d_Add( &( ptcl->p.builtin.ptclVelocity ), &Gravity, &( ptcl->p.builtin.ptclVelocity ) );
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
						jeVec3d_Subtract( ptcl->AnchorPoint, &( ptcl->CurrentAnchorPoint ), &AnchorDelta );
						jeVec3d_Add( (jeVec3d *)&( ptcl->ptclVertex.X ), &AnchorDelta, (jeVec3d *)&( ptcl->ptclVertex.X ) );
						jeVec3d_Copy( ptcl->AnchorPoint, &( ptcl->CurrentAnchorPoint ) );
					}

					// destroy the particle if it is in solid space
					/*{

						// locals
						JE_Contents	Contents;
						static		jeVec3d		Mins = { -1.0f, -1.0f, -1.0f };
						static		jeVec3d		Maxs = {  1.0f,  1.0f,  1.0f };

						// zap it if its in solid space
						if ( JE_WorldGetContents( ps->psWorld, (jeVec3d *)&( ptcl->ptclVertex.X ), &Mins, &Maxs, JE_COLLIDE_ALL, 0xffffffff, &Contents ) == JE_TRUE )
						{
							if ( Contents.Contents & JE_CONTENTS_SOLID )
							{

								// locals
								Particle	*temp;
	
								// destroy it
								temp = ptcl->ptclNext;
								UnlinkParticle( ps, ptcl );
								DestroyParticle( ps, ptcl );
								ptcl = temp;
								continue;
							}
						}
					}*/
				}
			}

#ifndef	POLYQ
			// set particle alpha
			assert( ptcl->p.builtin.ptclTotalTime > 0.0f );
			ptcl->ptclVertex.a = ptcl->Alpha * ( ptcl->p.builtin.ptclTime / ptcl->p.builtin.ptclTotalTime );
			jeWorld_AddPolyOnce(ps->psWorld,
								&ptcl->ptclVertex,
								1,
								ptcl->ptclTexture,
								JE_TEXTURED_POINT,
								PARTICLE_RENDERSTYLE,
								ptcl->Scale );
#else
			// set particle alpha
			assert( ptcl->p.builtin.ptclTotalTime > 0.0f );
			ptcl->ptclVertex.a = ptcl->Alpha * ( ptcl->p.builtin.ptclTime / ptcl->p.builtin.ptclTotalTime );

			if	(ptcl->ptclPoly)
				jeUPoly_SetLVertex(ptcl->ptclPoly, 0, &ptcl->ptclVertex);
#endif

			ptcl = ptcl->ptclNext;
		}

		DeltaTime -= QUANTUMSIZE;
	}

	ps->psLastTime += DeltaTime;
}


static	Particle * PARTICLE_CALLINGCONVENTION	CreateParticle(
	Particle_System *	ps,
	jeBitmap *	Texture,
	const JE_LVertex *	Vert )
{
	Particle *	ptcl;

	ptcl = (Particle *)jeRam_Allocate(sizeof(*ptcl));
	if	(!ptcl)
		return ptcl;

	memset(ptcl, 0, sizeof(*ptcl));

	ptcl->ptclNext = ps->psParticles;
					 ps->psParticles = ptcl;
	if	(ptcl->ptclNext)
		ptcl->ptclNext->ptclPrev = ptcl;
	ptcl->ptclTexture = Texture;
	ptcl->ptclVertex = *Vert;

	return ptcl;
}

// removes all references to an anchor point
jeBoolean PARTICLE_CALLINGCONVENTION	Particle_SystemRemoveAnchorPoint(
	Particle_System	*ps,				// particle system from which this anchor point will be removed
	jeVec3d			*AnchorPoint )		// the anchor point to remove
{

	// locals	
	Particle	*ptcl;
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
}

void PARTICLE_CALLINGCONVENTION	Particle_SystemAddParticle(
	Particle_System		*ps,
	jeBitmap	*Texture,
	const JE_LVertex	*Vert,
	const jeVec3d		*AnchorPoint,
	jeFloat				Time,
	const jeVec3d		*Velocity,
	float				Scale,
	const jeVec3d		*Gravity )
{

	// locals
	Particle	*ptcl;

	// create a new particle
	ptcl = CreateParticle( ps, Texture, Vert );
	if ( !ptcl )
	{
		return;
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
		jeVec3d_Copy( Velocity, &( ptcl->p.builtin.ptclVelocity ) );
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
	ptcl->p.builtin.ptclTime = Time;
	ptcl->p.builtin.ptclTotalTime = Time;
	ptcl->Alpha = Vert->a;

	// add the poly to the world
	#ifdef	POLYQ
		ptcl->ptclPoly = jeWorld_AddPoly(	ps->psWorld,
											&ptcl->ptclVertex,
											1,
											ptcl->ptclTexture,
											JE_TEXTURED_POINT,
											PARTICLE_RENDERSTYLE,
											ptcl->Scale );
	#endif
}

Particle * PARTICLE_CALLINGCONVENTION	Particle_SystemAddUserParticle(
	Particle_System *		ps,
	jeBitmap *		Texture,
	const JE_LVertex	*	Vert,
	ParticleCallBack		CallBack,
	int						UserDataSize)
{
	Particle *	ptcl;

	ptcl = CreateParticle(ps, Texture, Vert);
	if	(!ptcl)
		return ptcl;

	if	(UserDataSize)
	{
		ptcl->p.user.ptclUserData = jeRam_Allocate(UserDataSize);
		if	(!ptcl->p.user.ptclUserData)
		{
			UnlinkParticle(ps, ptcl);
			DestroyParticle(ps, ptcl);
		}
	}

	ptcl->ptclFlags |= PARTICLE_USER;
	ptcl->p.user.ptclCallBack = CallBack;

#ifdef	POLYQ
	ptcl->ptclPoly = jeWorld_AddPoly(ps->psWorld,
									&ptcl->ptclVertex,
									1,
									ptcl->ptclTexture,
									JE_TEXTURED_POINT,
									PARTICLE_RENDERSTYLE,
									1.0f );
#endif

	return ptcl;
}

void PARTICLE_CALLINGCONVENTION Particle_SystemReset(Particle_System *ps)
{
	ps->psLastTime = 0.0f;
	Particle_SystemRemoveAll(ps);
}

