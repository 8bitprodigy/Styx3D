/****************************************************************************************/
/*  SPRING.C                                                                            */
/*                                                                                      */
/*  Author:  Jason Wood                                                                 */
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

#include <assert.h>

#include "BaseType.h"
#include "Ram.h"
#include "Vec3d.h"

#include "Part.h"
#include "Spring.h"

typedef struct jeSpring
{
	jeParticle* p1, *p2;
	float Ks, Kd; // spring and damping constants
	float r0; // initial length

	jeSpring_ForceFunc forceFunc;

}jeSpring;

static void jeSpring_ComputeR0(jeSpring* pSpring)
{
	jeVec3d pos1, pos2, diff;

	jeParticle_GetPos(pSpring->p1, &pos1);
	jeParticle_GetPos(pSpring->p2, &pos2);

	jeVec3d_Subtract(&pos2, &pos1, &diff);
	pSpring->r0 = jeVec3d_Length(&diff);

	assert(pSpring->r0 > JE_EPSILON);
}

/////////////////////////////////////////////////////////////////////////////////
// ctor / dtor

jeSpring* jeSpring_Create(float Ks, float Kd, jeParticle* p1, jeParticle* p2,
	jeSpring_ForceFunc forceFunc)
{
	jeSpring* pSpring;

	assert(p1);
	assert(p2);

	assert(Ks > JE_EPSILON);
	assert(Kd > JE_EPSILON && Kd <= 1.f);
	assert(forceFunc);

	pSpring = (jeSpring*)jeRam_Allocate(sizeof(jeSpring));

	pSpring->Ks = Ks;
	pSpring->Kd = Kd;
	pSpring->p1 = p1;
	pSpring->p2 = p2;
	pSpring->forceFunc = forceFunc;

	jeSpring_ComputeR0(pSpring);

	return pSpring;
}

void jeSpring_Destroy(jeSpring** ppSpring)
{
	assert(ppSpring);
	assert(*ppSpring);

	jeRam_Free(*ppSpring);
	*ppSpring = NULL;
}

/////////////////////////////////////////////////////////////////////////////////
// accessors

jeParticle* jeSpring_GetPart1(const jeSpring* pSpring)
{
	assert(pSpring);

	return pSpring->p1;
}

jeParticle* jeSpring_GetPart2(const jeSpring* pSpring)
{
	assert(pSpring);

	return pSpring->p2;
}

float jeSpring_GetKs(const jeSpring* pSpring)
{
	assert(pSpring);

	return pSpring->Ks;
}

float jeSpring_GetKd(const jeSpring* pSpring)
{
	assert(pSpring);

	return pSpring->Kd;
}

float jeSpring_GetR0(const jeSpring* pSpring)
{
	assert(pSpring);

	return pSpring->r0;
}

jeSpring_ForceFunc jeSpring_GetForceFunc(const jeSpring* pSpring)
{
	assert(pSpring);

	return pSpring->forceFunc;
}

/////////////////////////////////////////////////////////////////////////////////

jeBoolean jeSpring_SetPart1(jeSpring* pSpring, const jeParticle* part1)
{
	assert(pSpring);
	assert(part1);

	pSpring->p1 = (jeParticle*)part1;

	jeSpring_ComputeR0(pSpring);

	return JE_TRUE;
}

jeBoolean jeSpring_SetPart2(jeSpring* pSpring, const jeParticle* part2)
{
	assert(pSpring);
	assert(part2);

	pSpring->p2 = (jeParticle*)part2;

	jeSpring_ComputeR0(pSpring);

	return JE_TRUE;
}

jeBoolean jeSpring_SetKs(jeSpring* pSpring, float Ks)
{
	assert(pSpring);
	assert(Ks > JE_EPSILON);

	pSpring->Ks = Ks;

	return JE_TRUE;
}

jeBoolean jeSpring_SetKd(jeSpring* pSpring, float Kd)
{
	assert(pSpring);
	assert(Kd > JE_EPSILON && Kd <= 1.f);

	pSpring->Kd = Kd;

	return JE_TRUE;
}

jeBoolean jeSpring_SetForceFunc(jeSpring* pSpring, jeSpring_ForceFunc forceFunc)
{
	assert(pSpring);
	assert(forceFunc);

	pSpring->forceFunc = forceFunc;

	return JE_TRUE;
}

/////////////////////////////////////////////////////////////////////////////////
// functions

// built-in force functions

// compute forces for particles attached to damped spring.
// our constraint function reads C = |x1 - x2| - r0 = 0.
jeBoolean jeSpring_ForceFunc_ComputeDamped(jeSpring* pSpring)
{
	jeVec3d pos1, pos2, dpos;
	jeVec3d v1, v2, dv;
	jeVec3d force;

	float len, C, Cdot, F;

	assert(pSpring);

	jeParticle_GetPos(pSpring->p1, &pos1);
	jeParticle_GetPos(pSpring->p2, &pos2);
	jeVec3d_Subtract(&pos1, &pos2, &dpos);
	len = jeVec3d_Normalize(&dpos);

	C = len - pSpring->r0;

	jeParticle_GetVel(pSpring->p1, &v1);
	jeParticle_GetVel(pSpring->p2, &v2);
	jeVec3d_Subtract(&v1, &v2, &dv);

	// Cdot = dC / dt = (dC / dx) * (dx / dt) =
	// ((x1 - x2) / |l|, (dx1 / dt - dx2 / dt))
	Cdot = jeVec3d_DotProduct(&dpos, &dv);

	// F = -(Ks * C + Kd * dC / Dt)
	F = -(pSpring->Ks * C + pSpring->Kd * Cdot);

	force.X = F * dpos.X;
	force.Y = F * dpos.Y;
	force.Z = F * dpos.Z;

	jeParticle_AddForce(pSpring->p1, &force);

	jeVec3d_Inverse(&force);
	jeParticle_AddForce(pSpring->p2, &force);

	return JE_TRUE;
}

jeBoolean jeSpring_ForceFunc_ComputeCriticallyDamped(jeSpring* pSpring)
{
	assert(pSpring);

	return JE_TRUE;
}
