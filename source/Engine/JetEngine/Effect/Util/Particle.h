/****************************************************************************************/
/*  PARTICLE.H                                                                          */
/*                                                                                      */
/*  Author:  Eli Boling                                                                 */
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
#ifndef	PARTICLE_H
#define	PARTICLE_H

#ifdef	__cplusplus
extern "C" {
#endif

/*
	The particle system is designed as a container, control dispatcher and raw level rendering
	manager for particles.  The idea is to have a flexible system in which you can easily add
	temporary particles to a world, yet be able to extend the system when the default behaviour
	for particles isn't sufficient for your needs.

	The Particle_System keeps track of real time elapsed between calls to its frame update function
	Particle_SystemFrame.  This time is broken into fixed size quanta which are used to age particles.
	The fixed size quanta are present in case we want to implement physics for particles, either
	in the builtin support for particles, or in the extensions to particles.

	To use a Particle_System, you create it, setting its quantum size, and then call
	Particle_SystemFrame once each rendering frame on the system.  You can add builtin particles
	to the system with Particle_SystemAddParticle.  This allows you to add basic particles, which
	can move in a limited fashion and have an age.  Once the particle ages past its limit, the
	Particle_System destroys it automatically.  Particles are rendered once per frame.

	To extend the Particle_System, we introduce the notion of user particles.  These particles
	do not have age, but instead have a callback function which is called once per quantum.
	The user particles have user data which is allocated when they are created, and which is
	accessible to the callback function.  The callback function governs when the particle is
	destroyed by returning JE_TRUE to keep the particle and JE_FALSE to destroy the particle.

	The combined system allows the client to fire and forget particles that are simple in nature,
	and removes the need for managing the life and storage of particles which require higher
	level control, all through the same API.
*/

#define	PARTICLE_CALLINGCONVENTION	__fastcall

typedef struct	Particle		Particle;
typedef	struct	Particle_System	Particle_System;

/*
	A callback takes a Particle_System, a Particle and a count of seconds (float)
	that have elapsed since the last time the particle callback was called.  Note that
	the callback could be called multiple times per frame, depending on the quantum size
	that the particle system was constructed with, and the frame rate of the target machine.
	Hence, it is really quite important that the callback NOT render anything.  The
	callback should return JE_TRUE if the particle is to remain alive, JE_FALSE if the
	particle should be destroyed this frame.  If JE_FALSE is returned, the particle will
	be destroyed in the current frame without being rendered.
*/
typedef	jeBoolean	(PARTICLE_CALLINGCONVENTION *ParticleCallBack)(Particle_System *, Particle *, jeFloat);

void PARTICLE_CALLINGCONVENTION Particle_SetTexture(Particle *p, jeBitmap *Texture);
	// Sets the texture on an existing particle.  Valid for user particles only.

JE_LVertex * PARTICLE_CALLINGCONVENTION Particle_GetVertex(Particle *p);
	// Gets the vertex from an existing particle.  Valid for user particles only.

void * PARTICLE_CALLINGCONVENTION Particle_GetUserData(Particle  *p);
	// Gets a pointer to the user data for an existing particle.  Valid for user particles only.

//Particle_System * PARTICLE_CALLINGCONVENTION Particle_SystemCreate(JE_World *World, int milliseconds);
Particle_System * PARTICLE_CALLINGCONVENTION Particle_SystemCreate(jeWorld *World);
	// Creates a particle system.  'milliseconds' is the quantum size for the particle frames.

void PARTICLE_CALLINGCONVENTION Particle_SystemDestroy(Particle_System *ps);
	// Destroys the particle system.

void PARTICLE_CALLINGCONVENTION Particle_SystemReset(Particle_System *ps);
	// Resets the particle system

int32 PARTICLE_CALLINGCONVENTION	Particle_GetCount(Particle_System *ps);
	// returns the count of how many particles are active in a particle system

void PARTICLE_CALLINGCONVENTION Particle_SystemFrame(Particle_System *ps, jeFloat DeltaTime);
	// Causes a rendering pass on the particle system.  Particle callbacks are called, and
	// particles are updated.

// removes all references to an anchor point
jeBoolean PARTICLE_CALLINGCONVENTION	Particle_SystemRemoveAnchorPoint(
	Particle_System	*ps,				// particle system from which this anchor point will be removed
	jeVec3d			*AnchorPoint );		// the anchor point to remove

void PARTICLE_CALLINGCONVENTION Particle_SystemAddParticle(
	Particle_System *		ps,				// System to add the particle to
	jeBitmap *		Texture,		// Texture to draw with
	const JE_LVertex	*	Vert,			// Vertex at which and how to draw the sprite
	const jeVec3d			*AnchorPoint,	// anchor point
	jeFloat					Time,			// How long this particle should live, in seconds
	const jeVec3d *			Velocity,		// Velocity vector to use to move the particle
	float					Scale,			// scale of particle art
	const jeVec3d			*Gravity );		// gravity which will act on the particle
	// Adds a builtin particle type to the system

Particle * PARTICLE_CALLINGCONVENTION Particle_SystemAddUserParticle(
	Particle_System *		ps,			// System to add the user particle to
	jeBitmap			*	Texture,	// Texture to draw with
	const JE_LVertex	*	Vert,		// Vertex at which and how to draw the sprite
	ParticleCallBack		CallBack,	// Callback function to call to update the particle
	int						UserDataSize);	// Size of user data to allocate for the particle

void PARTICLE_CALLINGCONVENTION Particle_SystemRemoveAll(
	Particle_System *		ps);		// System to kill

#ifdef	__cplusplus
}
#endif
#endif

