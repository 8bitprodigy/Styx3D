/****************************************************************************************/
/*  JEUSERPOLY.H                                                                        */
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

#ifndef JE_USERPOLY_H
#define JE_USERPOLY_H

#include "Engine.h"
#include "BaseType.h"
#include "jeTypes.h"
#include "Bitmap.h"
#include "Camera.h"
#include "jeFrustum.h"

#ifdef __cplusplus
extern "C" {
#endif

//========================================================================================
//	Typedefs/#defines
//========================================================================================
typedef struct jeUserPoly				jeUserPoly;


typedef enum
{
	Type_Line,
	Type_Tri,
	Type_Quad, 
	Type_Sprite
} jeUserPoly_Type;

//========================================================================================
//	Structure defs
//========================================================================================
JETAPI jeUserPoly	* JETCC jeUserPoly_CreateTri(	const jeLVertex		*v1, 
												const jeLVertex		*v2, 
												const jeLVertex		*v3, 
												const jeMaterialSpec *Material,
												uint32				Flags);
JETAPI jeUserPoly	* JETCC jeUserPoly_CreateQuad(	const jeLVertex		*v1, 
												const jeLVertex		*v2, 
												const jeLVertex		*v3, 
												const jeLVertex		*v4, 
												const jeMaterialSpec *Material,
												uint32				Flags);
JETAPI jeUserPoly	* JETCC jeUserPoly_CreateSprite(	const jeLVertex		*v1, 
												const jeMaterialSpec *Material,
													jeFloat				Scale,
													uint32				Flags);
JETAPI jeUserPoly	* JETCC jeUserPoly_CreateLine(const jeLVertex *v1, const jeLVertex *v2, jeFloat Scale, uint32 Flags);

JETAPI jeBoolean	JETCC jeUserPoly_IsValid(const jeUserPoly *Poly);
JETAPI jeBoolean	JETCC jeUserPoly_CreateRef(jeUserPoly *Poly);
JETAPI void			JETCC jeUserPoly_Destroy(jeUserPoly **Poly);
JETAPI jeBoolean	JETCC jeUserPoly_UpdateTri(	jeUserPoly *Poly, 
												const jeLVertex *v1, 
												const jeLVertex *v2, 
												const jeLVertex *v3, 
												const jeMaterialSpec *Material);

JETAPI jeBoolean	JETCC jeUserPoly_UpdateQuad(	jeUserPoly *Poly, 
												const jeLVertex *v1, 
												const jeLVertex *v2, 
												const jeLVertex *v3, 
												const jeLVertex *v4, 
												const jeMaterialSpec *Material);

JETAPI jeBoolean	JETCC jeUserPoly_UpdateSprite(jeUserPoly *Poly, const jeLVertex *v1, const jeMaterialSpec *Material, jeFloat Scale);

JETAPI jeBoolean	JETCC jeUserPoly_UpdateLine(jeUserPoly *Poly, const jeLVertex *v1, const jeLVertex *v2, jeFloat Scale);

JETAPI jeBoolean	JETCC jeUserPoly_Render(const jeUserPoly *Poly, const jeEngine *Engine, const jeCamera *Camera, const jeFrustum *Frustum);

//========================================================================================
//	Function prototypes
//========================================================================================
#ifdef __cplusplus
}
#endif

#endif
