/****************************************************************************************/
/*  SPOUTM.C                                                                            */
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
//
//	A spout that shoots particles.
//	-for effect manager use only
//
#pragma warning ( disable : 4514 )
#include <string.h>
#include <memory.h>
#include <assert.h>
#include "Ram.h"
#include "EffectMI.h"
#include "EffectC.h"


////////////////////////////////////////////////////////////////////////////////////////
//	Custom data
////////////////////////////////////////////////////////////////////////////////////////
typedef struct
{
	jeXForm3d	Xf;
	float		TimeLeft;
	float		MinPauseTime;
	float		MaxPauseTime;
	float		PauseTime;
	jeBoolean	PauseState;

} Custom;


////////////////////////////////////////////////////////////////////////////////////////
//	Interface setup
////////////////////////////////////////////////////////////////////////////////////////
static jeBoolean	SpoutM_Create( EffectManager *EManager, EffectM_Root *Root );
static void			SpoutM_Destroy( EffectManager *EManager, EffectM_Root *Root );
static jeBoolean	SpoutM_Update( EffectManager *EManager, EffectM_Root *Root );
EffectM_Interface	SpoutM_Interface =
{
	SpoutM_Create,
	SpoutM_Destroy,
	SpoutM_Update
};



////////////////////////////////////////////////////////////////////////////////////////
//
//	SpoutM_Create()
//
////////////////////////////////////////////////////////////////////////////////////////
static jeBoolean SpoutM_Create(
	EffectManager	*EManager,	// effect manager to add it to
	EffectM_Root	*Root )		// root data
{

	// locals
	Spray		Sp;
	Custom		*Cus;
	jeVec3d		In;
	int			TextureNumber;
	char		*AlphaName, *BmpName;

	// ensure valid data
	assert( EManager != NULL );
	assert( Root != NULL );

	// init remaining root data
	Root->EffectCount = 1;
	Root->EffectList = jeRam_AllocateClear( sizeof( int ) * Root->EffectCount );
	if ( Root->EffectList == NULL )
	{
		return JE_FALSE;
	}

	// init custom data
	Root->Custom = jeRam_AllocateClear( sizeof( Custom ) );
	if ( Root->Custom == NULL )
	{
		return JE_FALSE;
	}
	Cus = Root->Custom;
	EffectC_GetFieldData( EManager->EntityTable, Root->Entity, "TotalLife", &( Cus->TimeLeft ), sizeof( Cus->TimeLeft ), JE_SYMBOL_TYPE_FLOAT );
	EffectC_GetFieldData( EManager->EntityTable, Root->Entity, "MinPauseTime", &( Cus->MinPauseTime ), sizeof( Cus->MinPauseTime ), JE_SYMBOL_TYPE_FLOAT );
	assert( Cus->MinPauseTime >= 0.0f );
	EffectC_GetFieldData( EManager->EntityTable, Root->Entity, "MaxPauseTime", &( Cus->MaxPauseTime ), sizeof( Cus->MaxPauseTime ), JE_SYMBOL_TYPE_FLOAT );
	assert( Cus->MaxPauseTime >= Cus->MinPauseTime );
	if ( Cus->MaxPauseTime > 0.0f )
	{
		Cus->PauseTime = EffectC_Frand( Cus->MinPauseTime, Cus->MaxPauseTime );
	}

	// setup spray struct
	memset( &Sp, 0, sizeof( Sp ) );
	EffectC_GetFieldData( EManager->EntityTable, Root->Entity, "MinScale", &( Sp.MinScale ), sizeof( Sp.MinScale ), JE_SYMBOL_TYPE_FLOAT );
	assert( Sp.MinScale > 0.0f );
	EffectC_GetFieldData( EManager->EntityTable, Root->Entity, "MaxScale", &( Sp.MaxScale ), sizeof( Sp.MaxScale ), JE_SYMBOL_TYPE_FLOAT );
	assert( Sp.MaxScale >= Sp.MinScale );
	EffectC_GetFieldData( EManager->EntityTable, Root->Entity, "MinSpeed", &( Sp.MinSpeed ), sizeof( Sp.MinSpeed ), JE_SYMBOL_TYPE_FLOAT );
	assert( Sp.MinSpeed > 0.0f );
	EffectC_GetFieldData( EManager->EntityTable, Root->Entity, "MaxSpeed", &( Sp.MaxSpeed ), sizeof( Sp.MaxSpeed ), JE_SYMBOL_TYPE_FLOAT );
	assert( Sp.MaxSpeed >= Sp.MinSpeed );
	EffectC_GetFieldData( EManager->EntityTable, Root->Entity, "MinUnitLife", &( Sp.MinUnitLife ), sizeof( Sp.MinUnitLife ), JE_SYMBOL_TYPE_FLOAT );
	assert( Sp.MinUnitLife > 0.0f );
	EffectC_GetFieldData( EManager->EntityTable, Root->Entity, "MaxUnitLife", &( Sp.MaxUnitLife ), sizeof( Sp.MaxUnitLife ), JE_SYMBOL_TYPE_FLOAT );
	assert( Sp.MaxUnitLife >= Sp.MinUnitLife );
	EffectC_GetFieldData( EManager->EntityTable, Root->Entity, "SourceVariance", &( Sp.SourceVariance ), sizeof( Sp.SourceVariance ), JE_SYMBOL_TYPE_INT );
	assert( Sp.SourceVariance >= 0 );
	EffectC_GetFieldData( EManager->EntityTable, Root->Entity, "DestVariance", &( Sp.DestVariance ), sizeof( Sp.DestVariance ), JE_SYMBOL_TYPE_INT );
	assert( Sp.DestVariance >= 0 );
	EffectC_GetFieldData( EManager->EntityTable, Root->Entity, "ParticleCreateRate", &( Sp.Rate ), sizeof( Sp.Rate ), JE_SYMBOL_TYPE_FLOAT );
	assert( Sp.Rate > 0.0f );
	EffectC_GetFieldData( EManager->EntityTable, Root->Entity, "ColorMin", &( Sp.ColorMin ), sizeof( Sp.ColorMin ), JE_SYMBOL_TYPE_COLOR );
	EffectC_GetFieldData( EManager->EntityTable, Root->Entity, "ColorMax", &( Sp.ColorMax ), sizeof( Sp.ColorMax ), JE_SYMBOL_TYPE_COLOR );
	Sp.ColorMin.a = 255.0f;
	Sp.ColorMax.a = 255.0f;
	EffectC_GetFieldData( EManager->EntityTable, Root->Entity, "Gravity", &( Sp.Gravity ), sizeof( Sp.Gravity ), JE_SYMBOL_TYPE_VEC3D );
	EffectC_GetFieldData( EManager->EntityTable, Root->Entity, "Position", &( Sp.Source ), sizeof( Sp.Source ), JE_SYMBOL_TYPE_VEC3D );

	// setup texture
	EffectC_GetFieldData( EManager->EntityTable, Root->Entity, "BmpName", &( BmpName ), sizeof( BmpName ), JE_SYMBOL_TYPE_STRING );
	EffectC_GetFieldData( EManager->EntityTable, Root->Entity, "AlphaName", &( AlphaName ), sizeof( AlphaName ), JE_SYMBOL_TYPE_STRING );
	TextureNumber = TPool_Add( EManager->TPool, NULL, BmpName, AlphaName );
	if ( TextureNumber == -1 )
	{
		return JE_FALSE;
	}
	Sp.Texture = TPool_Get( EManager->TPool, TextureNumber );
	if ( Sp.Texture == NULL )
	{
		return JE_FALSE;
	}

	// setup orientation
	{

		// locals
		jeVec3d	Angles;

		// setup orientation from angles
		EffectC_GetFieldData( EManager->EntityTable, Root->Entity, "Angles", &( Angles ), sizeof( Angles ), JE_SYMBOL_TYPE_VEC3D );
		jeXForm3d_SetIdentity( &( Cus->Xf ) );
		jeXForm3d_RotateX( &( Cus->Xf ), Angles.X / 57.3f );  
		jeXForm3d_RotateY( &( Cus->Xf ), ( Angles.Y - 90.0f ) / 57.3f );  
		jeXForm3d_RotateZ( &( Cus->Xf ), Angles.Z / 57.3f );
	}

	// setup dest position
	jeXForm3d_GetIn( &( Cus->Xf ), &In );
	jeVec3d_Inverse( &In );
	jeVec3d_AddScaled( &( Sp.Source ), &In, 50.0f, &( Sp.Dest ) );

	// add effect
	if ( jeEffect_New( EManager->ESystem, Effect_Spray, &Sp, 0, NULL, NULL, &( Root->EffectList[0] ) ) == JE_FALSE )
	{
		return JE_FALSE;
	}

	// all done
	return JE_TRUE;

} // SpoutM_Create()



////////////////////////////////////////////////////////////////////////////////////////
//
//	SpoutM_Destroy()
//
////////////////////////////////////////////////////////////////////////////////////////
static void SpoutM_Destroy(
	EffectManager	*EManager,	// effect manager in which it exists
	EffectM_Root	*Root )		// root data
{

	// ensure valid data
	assert( EManager != NULL );
	assert( Root != NULL );

	// all done
	return;

	// get rid of warnings
	EManager;
	Root;

} // SpoutM_Destroy()



////////////////////////////////////////////////////////////////////////////////////////
//
//	SpoutM_Update()
//
////////////////////////////////////////////////////////////////////////////////////////
static jeBoolean SpoutM_Update(
	EffectManager	*EManager,	// effect manager to add it to
	EffectM_Root	*Root )		// root data
{

	// locals
	Custom		*Cus;

	// ensure valid data
	assert( EManager != NULL );
	assert( Root != NULL );

	// get custom data
	Cus = Root->Custom;
	assert( Cus != NULL );

	// kill the effect if its time has run out
	if ( Cus->TimeLeft > 0.0f )
	{
		Cus->TimeLeft -= Root->TimeDelta;
		if ( Cus->TimeLeft <= 0.0f )
		{
			Cus->TimeLeft = 0.0f;
			return JE_FALSE;
		}
	}

	// adjust pause time
	if ( Cus->PauseTime > 0.0f )
	{
		Cus->PauseTime -= Root->TimeDelta;
		if ( Cus->PauseTime <= 0.0f )
		{
			Cus->PauseTime = EffectC_Frand( Cus->MinPauseTime, Cus->MaxPauseTime );
			Cus->PauseState = !Cus->PauseState;
			jeEffect_SetPause( EManager->ESystem, Root->EffectList[0], Cus->PauseState );
		}
	}

	// all done
	return JE_TRUE;

} // SpoutM_Update()
