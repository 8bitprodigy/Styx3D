/****************************************************************************************/
/*  JELIGHT.H                                                                           */
/*                                                                                      */
/*  Author:  John Pollard                                                               */
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
#ifndef JE_LIGHT2_H
#define JE_LIGHT2_H

#include "BaseType.h"
#include "Vec3d.h"
#include "VFile.h"
#include "jePtrMgr.h"
#include "jeTypes.h"

#ifdef __cplusplus
extern "C" {
#endif

//========================================================================================

typedef struct		jeLight		jeLight;

#define	JE_LIGHT_FLAG_SUN						(1<<0)
#define	JE_LIGHT_FLAG_PARALLEL					(1<<0) // synonym for sun
#define	JE_LIGHT_FLAG_LINEAR_FALLOFF			(1<<1)
#define	JE_LIGHT_FLAG_INVERSE_FALLOFF			(1<<2)
#define	JE_LIGHT_FLAG_INVERSE_SQUARE_FALLOFF	(1<<3)
#define JE_LIGHT_FLAG_TYPEMASK					((1<<8) - 1)

#define	JE_LIGHT_FLAG_FAST_LIGHTING_MODEL		(1<<8)

//========================================================================================
// standard utilities :

JETAPI jeLight		* JETCC jeLight_Create(void);
JETAPI jeLight		* JETCC jeLight_CreateFromFile(jeVFile *VFile, jePtrMgr *PtrMgr);
JETAPI jeLight		* JETCC jeLight_CreateFromLight(const jeLight *SrcLight);
JETAPI jeBoolean	JETCC jeLight_WriteToFile(const jeLight *Light, jeVFile *VFile, jePtrMgr *PtrMgr);
JETAPI jeBoolean	JETCC jeLight_CreateRef(jeLight *Light);
JETAPI void			JETCC jeLight_Destroy(jeLight **Light);
JETAPI jeBoolean	JETCC jeLight_IsValid(const jeLight *Light);

//========================================================================================
// a handy function to calculate the illumination for you:

JETAPI jeBoolean	JETCC jeLight_CalculateLighting(const jeLight * Light,const jeVec3d *pPos,const jeVec3d *pNormal,
													jeRGBA * pColor);

//========================================================================================

JETAPI jeBoolean	JETCC jeLight_SetAttributes(	jeLight *Light, 
									const jeVec3d *Pos, 
									const jeVec3d *Color, 
									jeFloat Radius, 
									jeFloat Brightness, 
									uint32 Flags);

JETAPI jeBoolean	JETCC jeLight_GetAttributes(const jeLight *Light, 
									jeVec3d *Pos, 
									jeVec3d *Color, 
									jeFloat *Radius, 
									jeFloat *Brightness, 
									uint32 *Flags);

JETAPI uint32		JETCC jeLight_GetFlags(const jeLight *Light);

JETAPI jeFloat    JETCC jeLight_GetRadius(const jeLight *Light); 
									// calculates the radius for non LINEAR lights
									// returns a large value for sun lights

// shortcuts to SetAttributes to set up various types of lights:

JETAPI jeBoolean	JETCC jeLight_SetSunLight(jeLight *Light, 
								const jeVec3d *DirectionToSun, 
								const jeVec3d *Color, 
								jeFloat Brightness);

JETAPI jeBoolean	JETCC jeLight_SetInverseLight(jeLight *Light, 
								const jeVec3d *Pos, 
								const jeVec3d *Color, 
								jeFloat Brightness);

JETAPI jeBoolean	JETCC jeLight_SetInverseSquaredLight(jeLight *Light, 
								const jeVec3d *Pos, 
								const jeVec3d *Color, 
								jeFloat Brightness);

//========================================================================================

#ifdef __cplusplus
}
#endif

#endif
