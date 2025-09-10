/****************************************************************************************/
/*  TCLIP.H                                                                             */
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
#ifndef JE_TCLIP_H
#define JE_TCLIP_H

#include "BaseType.h"
#include "jeTypes.h"
#include "Bitmap.h"
#include "Engine.h"

#ifdef __cplusplus
extern "C" {
#endif


typedef struct jeMaterialSpec		jeMaterialSpec;

/*******

TClip is a state machine like OpenGL

you should call it like :

	_Push()
	_SetupEdges()
	_SetTexture()
	_Triangle()
	_Triangle()
	_SetTexture()
	_Triangle()
	_Triangle()
	...
	_Pop()

********/

JETAPI void JETCC jeTClip_SetupEdges(
	jeEngine *Engine,
	jeFloat	LeftEdge, 
	jeFloat RightEdge,
	jeFloat TopEdge ,
	jeFloat BottomEdge,
	jeFloat BackEdge);

JETAPI jeBoolean JETCC jeTClip_Push(void);
JETAPI jeBoolean JETCC jeTClip_Pop(void);

JETAPI jeBoolean JETCC jeTClip_SetTexture(const jeMaterialSpec * Material, int32 RenderFlags);
JETAPI void JETCC jeTClip_Triangle(const JE_LVertex TriVertex[3]);


#ifdef __cplusplus
}
#endif


#endif


