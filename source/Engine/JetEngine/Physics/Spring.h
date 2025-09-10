/****************************************************************************************/
/*  SPRING.H                                                                            */
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

#ifndef SPRING_H
#define SPRING_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct jeSpring jeSpring;

typedef jeBoolean (*jeSpring_ForceFunc)(jeSpring* pSpring, float dt);

/////////////////////////////////////////////////////////////////////////////////
// ctor / dtor

jeSpring* jeSpring_Create(float Ks, float Kd, jeParticle* p1, jeParticle* p2,
	jeSpring_ForceFunc forceFunc);
void jeSpring_Destroy(jeSpring** ppSpring);

/////////////////////////////////////////////////////////////////////////////////
// accessors

jeParticle* jeSpring_GetPart1(const jeSpring* pSpring);
jeParticle* jeSpring_GetPart2(const jeSpring* pSpring);
float jeSpring_GetKs(const jeSpring* pSpring);
float jeSpring_GetKd(const jeSpring* pSpring);
float jeSpring_GetR0(const jeSpring* pSpring);
jeSpring_ForceFunc jeSpring_GetForceFunc(const jeSpring* pSpring);

jeBoolean jeSpring_SetPart1(jeSpring* pSpring, const jeParticle* part1);
jeBoolean jeSpring_SetPart2(jeSpring* pSpring, const jeParticle* part2);
jeBoolean jeSpring_SetKs(jeSpring* pSpring, float Ks);
jeBoolean jeSpring_SetKd(jeSpring* pSpring, float Kd);
jeBoolean jeSpring_SetForceFunc(jeSpring* pSpring, jeSpring_ForceFunc forceFunc);

/////////////////////////////////////////////////////////////////////////////////
// fns

jeBoolean jeSpring_ForceFunc_ComputeDamped(jeSpring* pSpring);
jeBoolean jeSpring_ForceFunc_ComputeCriticallyDamped(jeSpring* pSpring);

#ifdef __cplusplus
}
#endif

#endif
