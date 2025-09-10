/****************************************************************************************/
/*  PUPPET.H																			*/
/*                                                                                      */
/*  Author: Mike Sandige	                                                            */
/*  Description: Puppet interface.										.				*/
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
#ifndef JE_PUPPET_H
#define JE_PUPPET_H

#include "Motion.h"
#include "Camera.h"
#include "Body.h"
#include "Pose.h"
#include "ExtBox.h"			// jeExtBox for jePuppet_RenderThroughFrustum

#include "VFile.h"
#include "UVMap.h"

#include "jeFrustum.h"
#include "Engine.h"
#include "jeWorld.h"

#ifdef __cplusplus
extern "C" {
#endif


typedef struct jePuppet jePuppet;

//	[MacroArt::Begin]
//	Thanks Dee(cryscan@home.net)	
float	JETCF jePuppet_GetAlpha(const jePuppet *P);
void	JETCF jePuppet_SetAlpha(jePuppet *P,float Alpha);
//	[MacroArt::End]

jePuppet* JETCF jePuppet_Create(jeVFile *TextureFS, const jeBody *B, jeEngine *pEngine);

void JETCF jePuppet_Destroy(jePuppet **P);

jeBoolean jePuppet_RenderThroughFrustum(const jePuppet *P, 
					const jePose		*Joints, 
					const jeExtBox		*Box, 
					jeEngine			*Engine, 
					const jeWorld		*World,
					const jeCamera		*Camera, 
					const jeFrustum		*Frustum,
					jeBoolean			updateStaticLighting);
	
jeBoolean jePuppet_Render(const jePuppet *P,
					const jePose		*Joints,
					jeEngine			*Engine, 
					const jeWorld		*World,
					const jeCamera		*Camera, 
					jeExtBox			*Box,
					jeBoolean			updateStaticLighting);

int JETCF jePuppet_GetMaterialCount( jePuppet *P );
jeBoolean jePuppet_GetMaterial( jePuppet *P, int MaterialIndex,
									jeMaterialSpec **Bitmap, 
									jeFloat *Red, jeFloat *Green, jeFloat *Blue,
									jeUVMapper * pMapper);
jeBoolean jePuppet_SetMaterial(jePuppet *P, int MaterialIndex, jeMaterialSpec *Bitmap, 
										jeFloat Red, jeFloat Green, jeFloat Blue, jeUVMapper Mapper);

void	  jePuppet_SetShadow(jePuppet *P, jeBoolean DoShadow, jeFloat Scale, 
						const jeMaterialSpec *ShadowMap,int BoneIndex);

void	  jePuppet_GetLightingOptions(const jePuppet *P,
	jeBoolean *UseFillLight,
	jeVec3d *FillLightNormal,
	jeFloat *FillLightRed,				
	jeFloat *FillLightGreen,				
	jeFloat *FillLightBlue,				
	jeFloat *AmbientLightRed,			
	jeFloat *AmbientLightGreen,			
	jeFloat *AmbientLightBlue,			
	jeBoolean *UseAmbientLightFromFloor,
	int32 *MaximumDynamicLightsToUse,
	int32 *MaximumStaticLightsToUse,	
	int32 *LightReferenceBoneIndex,
	jeBoolean *PerBoneLighting
	);

void	  jePuppet_SetLightingOptions(jePuppet *P,
	jeBoolean UseFillLight,
	const jeVec3d *FillLightNormal,
	jeFloat FillLightRed,				// 0 .. 255
	jeFloat FillLightGreen,				// 0 .. 255
	jeFloat FillLightBlue,				// 0 .. 255
	jeFloat AmbientLightRed,			// 0 .. 255
	jeFloat AmbientLightGreen,			// 0 .. 255
	jeFloat AmbientLightBlue,			// 0 .. 255
	jeBoolean AmbientLightFromFloor,
	int MaximumDynamicLightsToUse,		// 0 for none
	int MaximumStaticLightsToUse, // 0 for none
	int LightReferenceBoneIndex,
	int PerBoneLighting);

jeEngine* JETCF jePuppet_GetEngine(jePuppet *P);

#ifdef __cplusplus
}
#endif


#endif
