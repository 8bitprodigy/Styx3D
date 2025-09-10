/****************************************************************************************/
/*  PART.C                                                                              */
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

typedef struct jeParticle
{
	jeVec3d p, v, a; // pos, vel, acc
	float mass, oneOverMass;

	jeParticle_Flags flags;

	jeParticle_IntegratorFunc integratorFunc;

	float t; // time particle has been alive

}jeParticle;


/////////////////////////////////////////////////////////////////////////////////
// ctor / dtor

jeParticle* jeParticle_Create(float mass, jeVec3d* p, jeVec3d* v, jeParticle_Flags flags,
	jeParticle_IntegratorFunc integratorFunc)
{
	jeParticle* part;

	assert(p);
	assert(v);

	part = (jeParticle*)jeRam_Allocate(sizeof(jeParticle));
	if (! part) return NULL;

	part->mass = mass;
	if (part->mass < JE_EPSILON)
		part->oneOverMass = 0.f;
	else
		part->oneOverMass = 1 / mass;

	part->p = *p;
	part->v = *v;

	part->flags = flags;

	part->integratorFunc = integratorFunc;

	return part;
}

void jeParticle_Destroy(jeParticle** part)
{
	assert(part);
	assert(*part);

	jeRam_Free(*part);
	*part = NULL;
}

/////////////////////////////////////////////////////////////////////////////////
// accessors

float jeParticle_GetMass(const jeParticle* part)
{
	assert(part);

	return part->mass;
}

float jeParticle_GetOneOverMass(const jeParticle* part)
{
	assert(part);

	return part->oneOverMass;
}

jeBoolean jeParticle_GetPos(const jeParticle* part, jeVec3d* pos)
{
	assert(part);
	assert(pos);

	*pos = part->p;

	return JE_TRUE;
}

jeBoolean jeParticle_GetVel(const jeParticle* part, jeVec3d* vel)
{
	assert(part);
	assert(vel);

	*vel = part->v;

	return JE_TRUE;
}

jeBoolean jeParticle_GetAcc(const jeParticle* part, jeVec3d* acc)
{
	assert(part);
	assert(acc);

	*acc = part->a;

	return JE_TRUE;
}

jeParticle_IntegratorFunc jeParticle_GetIntegratorFunc(const jeParticle* part)
{
	assert(part);

	return part->integratorFunc;
}

float jeParticle_GetTime(const jeParticle* part)
{
	assert(part);

	return part->t;
}

jeParticle_Flags jeParticle_GetFlags(const jeParticle* part)
{
	assert(part);

	return part->flags;
}

/////////////////////////////////////////////////////////////////////////////////

jeBoolean jeParticle_SetMass(jeParticle* part, float mass)
{
	assert(part);

	part->mass = mass;
	if (part->mass < JE_EPSILON)
		part->oneOverMass = 0.f;
	else
		part->oneOverMass = 1 / mass;
	
	return JE_TRUE;
}

jeBoolean jeParticle_SetPos(jeParticle* part, const jeVec3d* pos)
{
	assert(part);
	assert(pos);

	part->p = *pos;

	return JE_TRUE;
}

jeBoolean jeParticle_SetVel(jeParticle* part, const jeVec3d* vel)
{
	assert(part);
	assert(vel);

	part->v = *vel;

	return JE_TRUE;
}

jeBoolean jeParticle_SetIntegratorFunc(jeParticle* part, jeParticle_IntegratorFunc func)
{
	assert(part);

	part->integratorFunc = func;

	return JE_TRUE;
}

jeBoolean jeParticle_SetFlags(jeParticle* part, jeParticle_Flags flags)
{
	assert(part);

	part->flags = flags;

	return JE_TRUE;
}

/////////////////////////////////////////////////////////////////////////////////
// fns

jeBoolean jeParticle_ClearAcc(jeParticle* part)
{
	assert(part);

	jeVec3d_Clear(&part->a);

	return JE_TRUE;
}

jeBoolean jeParticle_AddForce(jeParticle* part, const jeVec3d* pForce)
{
	assert(part);
	assert(pForce);

	part->a.X += part->oneOverMass * pForce->X;
	part->a.Y += part->oneOverMass * pForce->Y;
	part->a.Z += part->oneOverMass * pForce->Z;

	return JE_TRUE;
}

jeBoolean jeParticle_AddAcc(jeParticle* part, const jeVec3d* pAcc)
{
	assert(part);
	assert(pAcc);

	part->a.X += pAcc->X;
	part->a.Y += pAcc->Y;
	part->a.Z += pAcc->Z;

	return JE_TRUE;
}

jeBoolean jeParticle_UpdateTime(jeParticle* part, float dt)
{
	assert(part);
	assert(dt > JE_EPSILON);

	part->t += dt;

	return JE_TRUE;
}

/////////////////////////////////////////////////////////////////////////////////
// integrator functions

jeBoolean jeParticle_IntegratorFunc_EulerStep(jeParticle* part, float dt)
{
	jeVec3d dp, dv;

	assert(part);
	assert(dt > JE_EPSILON);

	dv.X = dt * part->a.X;
	dv.Y = dt * part->a.Y;
	dv.Z = dt * part->a.Z;

	dp.X = dt * part->v.X + dv.X;
	dp.Y = dt * part->v.Y + dv.Y;
	dp.Z = dt * part->v.Z + dv.Z;

	part->p.X += dp.X;
	part->p.Y += dp.Y;
	part->p.Z += dp.Z;

	part->v.X += dv.X;
	part->v.Y += dv.Y;
	part->v.Z += dv.Z;

	return JE_TRUE;
}

// takes 2 Euler steps, but is more accurate
jeBoolean jeParticle_IntegratorFunc_EulerMidPoint1(jeParticle* part, float dt)
{
	jeParticle midPart, tmpPart;

	assert(part);
	assert(dt > JE_EPSILON);

	tmpPart.a = part->a;
	tmpPart.v = part->v;
	tmpPart.p = part->v;

	jeParticle_IntegratorFunc_EulerStep(&tmpPart, dt);
	
	midPart.a = tmpPart.a;

	midPart.v.X = 0.5f * (part->v.X + tmpPart.v.X);
	midPart.v.Y = 0.5f * (part->v.Y + tmpPart.v.Y);
	midPart.v.Z = 0.5f * (part->v.Z + tmpPart.v.Z);

	midPart.p.X = 0.5f * (part->p.X + tmpPart.p.X);
	midPart.p.Y = 0.5f * (part->p.Y + tmpPart.p.Y);
	midPart.p.Z = 0.5f * (part->p.Z + tmpPart.p.Z);

	jeParticle_IntegratorFunc_EulerStep(&midPart, 0.5f * dt);

	part->v = midPart.v;
	part->p = midPart.p;

	return JE_TRUE;
}
