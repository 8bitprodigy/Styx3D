/****************************************************************************************/
/*  EFFECTC.H                                                                           */
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
#ifndef EFFECTC_H
#define EFFECTC_H

#include "Jet.h"
#include "symbol.h"	//undone

#ifdef __cplusplus
	extern "C" {
#endif


////////////////////////////////////////////////////////////////////////////////////////
//	Clip styles
////////////////////////////////////////////////////////////////////////////////////////
#define EFFECTC_CLIP_LEAF			( 1 << 0 )
#define EFFECTC_CLIP_LINEOFSIGHT	( 1 << 1 )
#define EFFECTC_CLIP_SEMICIRCLE		( 1 << 2 )


////////////////////////////////////////////////////////////////////////////////////////
//	Prototypes
////////////////////////////////////////////////////////////////////////////////////////

//	Picks a random float within the supplied range.
//
////////////////////////////////////////////////////////////////////////////////////////
extern float EffectC_Frand(
	float Low,		// minimum value
	float High );	// maximum value

//	Create a transform from two vectors.
//
////////////////////////////////////////////////////////////////////////////////////////
extern void EffectC_XFormFromVector(
	jeVec3d		*Source,	// source point
	jeVec3d		*Target,	// where we are looking at
	jeXForm3d	*Out );		// resultant transformation

//	Returns true if point is visible, false if it isn't.
//
////////////////////////////////////////////////////////////////////////////////////////
extern jeBoolean EffectC_IsPointVisible(
	jeWorld		*World,			// world in which to party
	jeCamera	*Camera,		// camera location data
	jeVec3d		*Target,		// target point
	int32		Leaf,			// target leaf
	uint32		ClipStyle );	// clipping styles to use

//	Get a bones correct transform.
//
////////////////////////////////////////////////////////////////////////////////////////
extern jeBoolean EffectC_GetBoneLoc(
	jeActor		*pActor,	// actor in which bone exists
	const char	*BoneName,	// name of bone
	jeXForm3d	*Kw );		// where to store transform

//	Rotate a transform.
//
////////////////////////////////////////////////////////////////////////////////////////
void EffectC_XfRotateX(
	jeXForm3d	*XfIn,		// xf to rotatte
	float		Radians,	// how much ro rotate it
	jeXForm3d	*XfOut );	// where to store new xf

//	Determines if a string is NULL, accounting for additional Jet3D editor posibilities.
//
////////////////////////////////////////////////////////////////////////////////////////
jeBoolean EffectC_IsStringNull(
	char	*String );	// string to check

//	Checks if a color struct contains valid data.
//
////////////////////////////////////////////////////////////////////////////////////////
jeBoolean EffectC_IsColorGood(
	JE_RGBA	*Color );	// color to check

//	Get data from an entity field.
//
////////////////////////////////////////////////////////////////////////////////////////
jeBoolean EffectC_GetFieldData( 
	jeSymbol_Table	*pEntityTable,	// entity table to search thru
	jeSymbol		*pEntity,		// entity we are interested in
	char			*FieldName,		// field in the entity that we are interested in
	void			*Data,			// where to store data
	int 			DataLength,		// expected size of data
	jeSymbol_Type 	Type );			// type of data


#ifdef __cplusplus
	}
#endif

#endif
