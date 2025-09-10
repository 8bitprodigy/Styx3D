/****************************************************************************************/
/*  PART.H                                                                              */
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

#ifndef PART_H
#define PART_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
	JE_PARTICLE_FLAGS_NONE											= 1 << 0,
	JE_PARTICLE_FLAGS_COLLIDE_RIGIDBODY					= 1 << 1,
	JE_PARTICLE_FLAGS_COLLIDE_BARRIER						= 1 << 2

}jeParticle_Flags;

typedef struct jeParticle jeParticle;

typedef jeBoolean (*jeParticle_IntegratorFunc)(jeParticle* part, float dt);

// built-in integrator functions
jeBoolean jeParticle_IntegratorFunc_EulerStep(jeParticle* part, float dt);
jeBoolean jeParticle_IntegratorFunc_EulerMidPoint1(jeParticle* part, float dt);

/////////////////////////////////////////////////////////////////////////////////
// ctor / dtor
jeParticle* jeParticle_Create(float mass, jeVec3d* p, jeVec3d* v, jeParticle_Flags flags,
	jeParticle_IntegratorFunc integratorFunc);
void jeParticle_Destroy(jeParticle** part);

/////////////////////////////////////////////////////////////////////////////////
// accessors

float jeParticle_GetMass(const jeParticle* part);
float jeParticle_GetOneOverMass(const jeParticle* part);
jeBoolean jeParticle_GetPos(const jeParticle* part, jeVec3d* pos);
jeBoolean jeParticle_GetVel(const jeParticle* part, jeVec3d* vel);
jeBoolean jeParticle_GetAcc(const jeParticle* part, jeVec3d* acc);
jeParticle_IntegratorFunc jeParticle_GetIntegratorFunc(const jeParticle* part);
float jeParticle_GetTime(const jeParticle* part);
jeParticle_Flags jeParticle_GetFlags(const jeParticle* part);

jeBoolean jeParticle_SetPos(jeParticle* part, const jeVec3d* pos);
jeBoolean jeParticle_SetMass(jeParticle* part, float mass);
jeBoolean jeParticle_SetVel(jeParticle* part, const jeVec3d* vel);
jeBoolean jeParticle_SetIntegratorFunc(jeParticle* part, jeParticle_IntegratorFunc func);
jeBoolean jeParticle_SetFlags(jeParticle* part, jeParticle_Flags flags);

/////////////////////////////////////////////////////////////////////////////////
// fns

jeBoolean jeParticle_ClearAcc(jeParticle* part);
jeBoolean jeParticle_AddForce(jeParticle* part, const jeVec3d* pForce);
jeBoolean jeParticle_AddAcc(jeParticle* part, const jeVec3d* pAcc);
jeBoolean jeParticle_UpdateTime(jeParticle* part, float dt);

#ifdef __cplusplus
}
#endif

#endif
