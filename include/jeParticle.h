/****************************************************************************************/
/*  JEPARTICLE.H                                                                        */
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
#ifndef	JE_PARTICLE_H
#define	JE_PARTICLE_H

#include "jeWorld.h"
#include "jeTypes.h"
#include "Bitmap.h"
#include "Vec3d.h"

#ifdef	__cplusplus
extern "C" {
#endif


////////////////////////////////////////////////////////////////////////////////////////
//	Particle system structs
////////////////////////////////////////////////////////////////////////////////////////
typedef struct	jeParticle			jeParticle;
typedef	struct	jeParticle_System	jeParticle_System;


typedef struct jeMaterialSpec		jeMaterialSpec;


////////////////////////////////////////////////////////////////////////////////////////
//	Prototypes
////////////////////////////////////////////////////////////////////////////////////////

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
	const jeVec3d		*Gravity );		// pull of gravity

//	Removes all references to an anchor point.
//
////////////////////////////////////////////////////////////////////////////////////////
JETAPI jeBoolean JETCC jeParticle_SystemRemoveAnchorPoint(
	jeParticle_System	*ps,				// particle system from which this anchor point will be removed
	jeVec3d				*AnchorPoint );		// the anchor point to remove

//	Process a frame of the particle system.
//
////////////////////////////////////////////////////////////////////////////////////////
JETAPI void JETCC jeParticle_SystemFrame(
	jeParticle_System	*ps,			// particle system to process
	float				DeltaTime );	// amount of elaped seconds

//	Create a particle system.
//
////////////////////////////////////////////////////////////////////////////////////////
JETAPI jeParticle_System * JETCC jeParticle_SystemCreate(
	float	MajorVersion,	// major version number of calling app
	float	MinorVersion );	// minor version number of calling app

//	Destroy a particle system.
//
////////////////////////////////////////////////////////////////////////////////////////
JETAPI void JETCC jeParticle_SystemDestroy(
	jeParticle_System	*ps );	// particle system to destroy

//	Return the current number of active particles.
//
////////////////////////////////////////////////////////////////////////////////////////
JETAPI int JETCC jeParticle_GetCount(
	jeParticle_System	*ps );	// particle system whose particle count we want

//	Destroy all particles.
//
////////////////////////////////////////////////////////////////////////////////////////
JETAPI void JETCC jeParticle_SystemRemoveAll(
	jeParticle_System	*ps );	// particle system whose particles will all be destroyed


#ifdef	__cplusplus
}
#endif
#endif
