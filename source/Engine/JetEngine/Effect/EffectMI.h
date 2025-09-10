/****************************************************************************************/
/*  EFFECTMI.H                                                                          */
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
////////////////////////////////////////////////////////////////////////////////////////
//
//	EffectMI.h
//
//	Effect manager interface definition.
//
////////////////////////////////////////////////////////////////////////////////////////
#ifndef EFFECTMI_H
#define EFFECTMI_H

#include "EffectM.h"
#include "IndexList.h"
#include "Effect.h"
#include "TPool.h"
#include "SPool.h"

#ifdef __cplusplus
	extern "C" {
#endif


////////////////////////////////////////////////////////////////////////////////////////
//	EffectManager struct
////////////////////////////////////////////////////////////////////////////////////////
typedef struct EffectManager
{
	EffectSystem	*ESystem;		// effect system the manager uses
	TexturePool		*TPool;			// texture pool the manager uses
	SoundPool		*SPool;			// sound pool the manager uses
	IndexList		*EffectList;	// list of active effects
	jeSymbol_Table	*EntityTable;	// entity table
	void			*Context;		// context data

} EffectManager;


////////////////////////////////////////////////////////////////////////////////////////
//	Root list struct
////////////////////////////////////////////////////////////////////////////////////////
typedef struct
{
	int			Type;				// type of effect
	int32		EffectCount;		// how many individual effects make up this effect
	int32		*EffectList;		// id list of those effects
	float		TimeDelta;			// seconds elapsed since last update
	void		*Custom;			// custom data
	jeBoolean	Terminate;			// termination flag
	jeSymbol	*Entity;			// user data

} EffectM_Root;


////////////////////////////////////////////////////////////////////////////////////////
//	EffectMI interface
////////////////////////////////////////////////////////////////////////////////////////
typedef jeBoolean	( *EffectMI_Create )( EffectManager *Manager, EffectM_Root *Root );
typedef void		( *EffectMI_Destroy )( EffectManager *Manager, EffectM_Root *Root );
typedef jeBoolean	( *EffectMI_Update )( EffectManager *Manager, EffectM_Root *Root );
typedef	struct EffectM_Interface
{
	EffectMI_Create		Create;
	EffectMI_Destroy	Destroy;
	EffectMI_Update		Update;

}	EffectM_Interface;


#ifdef __cplusplus
	}
#endif

#endif
