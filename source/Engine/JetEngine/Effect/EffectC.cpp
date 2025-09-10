/****************************************************************************************/
/*  EFFECTC.C                                                                           */
/*                                                                                      */
/*  Author:  Peter Siamidis                                                             */
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
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include "Jet.h"
#include "EffectC.h"



////////////////////////////////////////////////////////////////////////////////////////
//
//	EffectC_Frand()
//
//	Picks a random float within the supplied range.
//
////////////////////////////////////////////////////////////////////////////////////////
float EffectC_Frand(
	float Low,		// minimum value
	float High )	// maximum value
{

	// locals
	float	Range;

	// ensure valid data
	assert( High >= Low );

	// if they are the same then just return one of them
	if ( High == Low )
	{
		return Low;
	}

	// pick a random float from whithin the range
	Range = High - Low;
	return ( (float)( ( ( rand() % 1000 ) + 1 ) ) ) / 1000.0f * Range + Low;

} // EffectC_Frand()



////////////////////////////////////////////////////////////////////////////////////////
//
//	EffectC_XFormFromVector()
//
//	Create a transform from two vectors.
//
////////////////////////////////////////////////////////////////////////////////////////
void EffectC_XFormFromVector(
	jeVec3d		*Source,	// source point
	jeVec3d		*Target,	// where we are looking at
	jeXForm3d	*Out )		// where to store resultant transformation
{

	// locals
	jeVec3d	Temp, Vertical, Vect;

	// ensure valid data
	assert( Source != NULL );
	assert( Target != NULL );
	assert( Out != NULL );
	
	// create a straight up vector
	Vertical.X = 0.0f;
	Vertical.Y = 1.0f;
	Vertical.Z = 0.0f;

	// create the source vector, fudging it if its coplanar to the comparison vector
	jeVec3d_Subtract( Source, Target, &Vect );
	//assert( jeVec3d_Compare( Source, Target, 0.05f ) == JE_FALSE );
	if ( ( Vertical.X == Vect.X ) && ( Vertical.Z == Vect.Z ) )
	{
		Vertical.X += 1.0f;
	}

	// set the IN vector
	jeXForm3d_SetIdentity( Out );
	jeVec3d_Normalize( &Vect );
	Out->AZ = Vect.X;
	Out->BZ = Vect.Y;
	Out->CZ = Vect.Z;

	// use it with the in vector to get the RIGHT vector
	jeVec3d_CrossProduct( &Vertical, &Vect, &Temp );
	jeVec3d_Normalize( &Temp );

	// put the RIGHT vector in the matrix
	Out->AX = Temp.X;
	Out->BX = Temp.Y;
	Out->CX = Temp.Z;

	// use the RIGHT vector with the IN vector to get the real UP vector
	jeVec3d_CrossProduct( &Vect, &Temp, &Vertical );
	jeVec3d_Normalize( &Vertical );

	// put the UP vector in the matrix
	Out->AY = Vertical.X;
	Out->BY = Vertical.Y;
	Out->CY = Vertical.Z;
	
	// put the translation in
	Out->Translation = *Source;

} // EffectC_XFormFromVector()



////////////////////////////////////////////////////////////////////////////////////////
//
//	EffectC_IsPointVisible()
//
//	Returns true if point is visible, false if it isn't.
//
////////////////////////////////////////////////////////////////////////////////////////
jeBoolean EffectC_IsPointVisible(
	jeWorld		*World,			// world in which to party
	jeCamera	*Camera,		// camera location data
	jeVec3d		*Target,		// target point
	int32		Leaf,			// target leaf
	uint32		ClipStyle )		// clipping styles to use
{

	// ensure valid data
	assert( World != NULL );
	assert( Camera != NULL );
	assert( Target != NULL );

	// leaf check
	if ( ClipStyle & EFFECTC_CLIP_LEAF )
	{
		if ( jeWorld_MightSeeLeaf( World, Leaf ) == JE_FALSE )
		{
			return JE_FALSE;
		}
	}

	// semi circle check
	if ( ClipStyle & EFFECTC_CLIP_SEMICIRCLE )
	{

		// locals
		jeXForm3d	CameraXf;
		jeVec3d		In;
		jeVec3d		Delta;
		float		Dot;

		// get camera xform
		jeCamera_GetXForm( Camera, &CameraXf );

		// get angle between camera in vector and vector to target
		jeVec3d_Subtract( Target, &( CameraXf.Translation ), &Delta );
		jeVec3d_Normalize( &Delta );
		jeXForm3d_GetIn( &CameraXf, &In );
		Dot = jeVec3d_DotProduct( &In, &Delta );

		// check if its visible
		if ( Dot < 0.0f )
		{
			return JE_FALSE;
		}
	}

	// line of sight check
	if ( ClipStyle & EFFECTC_CLIP_LINEOFSIGHT )
	{

		// locals
		JE_Collision	Collision;
		jeXForm3d		CameraXf;

		// get camera xform
		jeCamera_GetXForm( Camera, &CameraXf );

		// check if its visible
		if ( jeWorld_Collision( World, NULL, NULL, &( CameraXf.Translation ), Target, JE_CONTENTS_SOLID, JE_COLLIDE_MODELS, 0, NULL, NULL, &Collision ) == JE_TRUE )
		{
			return JE_FALSE;
		}
	}

	// if we got to here then its visible
	return JE_TRUE;

} // EffectC_IsPointVisible()



////////////////////////////////////////////////////////////////////////////////////////
//
//	EffectC_GetBoneLoc()
//
//	Get a bones correct transform.
//
////////////////////////////////////////////////////////////////////////////////////////
jeBoolean EffectC_GetBoneLoc(
	jeActor		*pActor,	// actor in which bone exists
	const char	*BoneName,	// name of bone
	jeXForm3d	*Kw )		// where to store transform
{

	// locals
	jeXForm3d Ar, ArI, L;
	jeVec3d	tempTranslation; 

	// ensure valid data
	assert( pActor );
	assert( BoneName );
	assert( Kw );

	// get bone transform
	if( !jeActor_GetBoneTransform( pActor, BoneName, &L ) )
	{
		return JE_FALSE;
	}

	// Locator is a child of of BIP01 I must pull out the BIP01's attchment
	tempTranslation = L.Translation;
	jeVec3d_Set( &L.Translation, 0.0f, 0.0f, 0.0f );
	if( !jeActor_GetBoneAttachment( pActor, "BIP01", &Ar ) )
	{
		return JE_FALSE;
	}
	jeVec3d_Set( &Ar.Translation, 0.0f, 0.0f, 0.0f );
	jeXForm3d_GetTranspose( &Ar, &ArI );

	jeXForm3d_Multiply( &L, &ArI, Kw );

	Kw->AY = 0.0f;

	Kw->BX = 0.0f;
	Kw->BY = 1.0f;
	Kw->BZ = 0.0f;

	Kw->CY = 0.0f;

	jeXForm3d_Orthonormalize( Kw );
	Kw->Translation = tempTranslation;
	return JE_TRUE;
	
} // EffectC_GetBoneLoc()



////////////////////////////////////////////////////////////////////////////////////////
//
//	EffectC_XfRotateX()
//
//	Rotate a transform.
//
////////////////////////////////////////////////////////////////////////////////////////
void EffectC_XfRotateX(
	jeXForm3d	*XfIn,		// xf to rotatte
	float		Radians,	// how much ro rotate it
	jeXForm3d	*XfOut )	// where to store new xf
{

	// locals
	jeXForm3d	XfRot;
	jeVec3d		Pos;

	// ensure valid data
	assert( XfIn != NULL );
	assert( XfOut != NULL );

	// apply rotation
	jeVec3d_Copy( &( XfIn->Translation ), &Pos );
	jeXForm3d_SetXRotation( &XfRot, Radians );
	jeXForm3d_Multiply( XfIn, &XfRot, XfOut );
	jeVec3d_Copy( &Pos, &( XfOut->Translation ) );

} // EffectC_XfRotateX()



////////////////////////////////////////////////////////////////////////////////////////
//
//	EffectC_IsStringNull()
//
//	Determines if a string is NULL, accounting for additional editor posibilities.
//
////////////////////////////////////////////////////////////////////////////////////////
jeBoolean EffectC_IsStringNull(
	char	*String )	// string to check
{

	// first way
	if ( String == NULL )
	{
		return JE_TRUE;
	}

	// second way
	if ( strlen( String ) < 1 )
	{
		return JE_TRUE;
	}

	// third way
	if ( strnicmp( String, "<null>", 6 ) == 0 )
	{
		return JE_TRUE;
	}

	// fourth way
	if ( strnicmp( String, "NULL", 4 ) == 0 )
	{
		return JE_TRUE;
	}

	// if we got to here then the string is not null
	return JE_FALSE;

} // EffectC_IsStringNull()



////////////////////////////////////////////////////////////////////////////////////////
//
//	EffectC_IsColorGood()
//
//	Checks if a color struct contains valid data.
//
////////////////////////////////////////////////////////////////////////////////////////
jeBoolean EffectC_IsColorGood(
	JE_RGBA	*Color )	// color to check
{

	// ensure valid data
	assert( Color != NULL );
	
	// fail if any color values are out of range
	if ( ( Color->a < 0.0f ) || ( Color->a > 255.0f ) )
	{
		return JE_FALSE;
	}
	if ( ( Color->r < 0.0f ) || ( Color->r > 255.0f ) )
	{
		return JE_FALSE;
	}
	if ( ( Color->g < 0.0f ) || ( Color->g > 255.0f ) )
	{
		return JE_FALSE;
	}
	if ( ( Color->b < 0.0f ) || ( Color->b > 255.0f ) )
	{
		return JE_FALSE;
	}

	// if we got to here then the color is good
	return JE_TRUE;

} // EffectC_IsColorGood()



////////////////////////////////////////////////////////////////////////////////////////
//
//	EffectC_GetFieldData()
//
//	Get data from an entity field.
//
////////////////////////////////////////////////////////////////////////////////////////
jeBoolean EffectC_GetFieldData( 
	jeSymbol_Table	*pEntityTable,	// entity table to search thru
	jeSymbol		*pEntity,		// entity we are interested in
	char			*FieldName,		// field in the entity that we are interested in
	void			*Data,			// where to store data
	int 			DataLength,		// expected size of data
	jeSymbol_Type 	Type )			// type of data
{

	// locals
	jeSymbol	*pQualifier;
	jeSymbol	*pEntityDef;
	jeSymbol	*pField = NULL;

	// ensure valid data
	assert( pEntityTable != NULL );
	assert( pEntity != NULL );
	assert( FieldName != NULL );
	assert( Data != NULL );
	assert( DataLength > 0 );

	// get field pointer
	pQualifier = jeSymbol_GetQualifier( pEntity );
	if ( pQualifier == NULL )
	{
		return JE_FALSE;
	}
	pEntityDef = jeSymbol_TableFindSymbol( pEntityTable, pQualifier, jeSymbol_GetName( pQualifier ) );
	if( pEntityDef == NULL )
	{
		return JE_FALSE;
	}
	pField = jeSymbol_TableFindSymbol( pEntityTable, pEntityDef, FieldName );
	if ( pField == NULL )
	{
		return JE_FALSE;
	}

	// get data and return
	return jeSymbol_GetProperty( pEntity, pField, &( Data ), DataLength, Type );

} // EffectC_GetFieldData()
