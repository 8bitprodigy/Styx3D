/****************************************************************************************/
/*  EFFECTM.H                                                                           */
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
#ifndef EFFECTM_H
#define EFFECTM_H

#include "Jet.h"
#include "symbol.h"	//undone

#ifdef __cplusplus
	extern "C" {
#endif


////////////////////////////////////////////////////////////////////////////////////////
//	EffectManager type
////////////////////////////////////////////////////////////////////////////////////////
typedef struct EffectManager	EffectManager;


////////////////////////////////////////////////////////////////////////////////////////
//	Prototypes
////////////////////////////////////////////////////////////////////////////////////////

//	Create a new effect manager.
//
////////////////////////////////////////////////////////////////////////////////////////
EffectManager * jeEffectM_CreateManager(
	jeEngine		*Engine,		// engine to use
	jeWorld			*World,			// world to use
	jeSound_System	*Sound,			// sound system to use
	jeCamera		*Camera,		// camera to use
	jeSymbol_Table	*EntityTable,	// entity table to use
	void			*Context );		// context data

//	Destroy an effect manager.
//
////////////////////////////////////////////////////////////////////////////////////////
void jeEffectM_DestroyManager(
	EffectManager	**EManager );	// manager to zap

//	Create a new effect.
//
////////////////////////////////////////////////////////////////////////////////////////
int jeEffectM_Create(
	EffectManager	*EManager,	// effect manager to add it to
	jeSymbol		*Entity,	// enitity which contains the effect data
	int				*Id );		// where to store effect id number

//	Destroy one effect of an effect manager by id.
//
////////////////////////////////////////////////////////////////////////////////////////
jeBoolean jeEffectM_Destroy(
	EffectManager	*EManager,	// effect manager that it belongs to
	int				Id );		// effect to destroy

//	Update a effect managers effects.
//
////////////////////////////////////////////////////////////////////////////////////////
int jeEffectM_Update(
	EffectManager	*EManager,		// manager to update
	float			TimeDelta );	// amount of elased time
/*
//	EffectM_LoadWorldEffects()
//
////////////////////////////////////////////////////////////////////////////////////////
int EffectM_LoadWorldEffects(
	EffectManager	*EManager,	// effect manager that will own them
	jeWorld			*World );	// world whose entity effects will be loaded
*/

#ifdef __cplusplus
	}
#endif

#endif
