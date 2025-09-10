/****************************************************************************************/
/*  SOUND3D.C                                                                           */
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
#include <math.h>
#include <Assert.h>

#include "Vec3d.h"
#include "XForm3d.h"
#include "Camera.h"
#include "Sound3d.h"

// Sound
typedef struct jeSound3d_Cfg
{
	jeFloat			Volume;
	jeFloat			Pan;
	jeFloat			Frequency;
} jeSound3d_Cfg;


//=====================================================================================
//	Snd3D_RoolOut
//  Snd will drop off so that it is half intensity when twice min distance
//  A 10 db reduction means half the intensity.
//  In our volume routine 0.01 represents a decible
//=====================================================================================
static void jeSound3D_RollOut(jeSound3d_Cfg *Cfg, float Dist, float Min, float Max)
{
	jeFloat Volume;
	assert( Cfg != NULL );

	if( Dist > Max )
	{
		Volume = 0.0f;
	}
	else if( Dist < Min )
	{
		Volume = 1.0f;
	}
	else
		Volume = 1.0f - (Dist/Min - 1)*0.1f;
	Cfg->Volume = Volume;
}

//=====================================================================================
//	Snd3D_Pan
//=====================================================================================
static void jeSound3D_Pan(jeSound3d_Cfg *Cfg, float FaceOffset )
{
	assert(Cfg != NULL);
	Cfg->Pan = (float)sin((double)FaceOffset )*0.1f;
}

//=====================================================================================
//	Mps is the reletive velocity in  meters per second 
//=====================================================================================
static void jeSound3D_Doppler(jeSound3d_Cfg *Cfg, float Mps ) //Modified by CyRiuS
{
	float emitted;
	float Vair;
	float part2;

	assert(Cfg != NULL);
	
	//343 mps is the velocity of sound throguh air.
	
	//F = Fe * ((V + Vo)/(V - Vs))
	//F = frequency
	//V = velocity
	//'e' = emitted
	//'o' = observer (AKA the player)
	//'s' = the source

	//TODO: make this function accept a velocity value for the player and
	//		add that as Vo in the part2 equation below
	
	Vair = 343.0f; //this is the velocity of sound through air. Replace 'V' in the above equation w/ this
	part2 = ((Vair)/(Vair - Mps));
	emitted = 0.0f;

	if(emitted < 1.0f) //the current frequency is less than 1.0f
	{
		emitted = 1.0f;
		assert(emitted != 0.0f);
		Cfg->Frequency = 1.0f + (Mps/ 343.0f );
	}
	else
	{
		emitted = Cfg->Frequency;
		assert(emitted != 0.0f);
		Cfg->Frequency = (emitted*part2)+(1.0f); //new
	}

}

//=====================================================================================
//	Snd3D_3DSound
//	This is the position of the sound translated to your view
//	coordinate system
//=====================================================================================
JETAPI	void JETCC jeSound3D_GetConfig(
		const jeWorld *World, 
		const jeXForm3d *MXForm, 
		const jeVec3d *SndPos, 
		jeFloat Min, 
		jeFloat Ds,
		jeFloat *Volume,
		jeFloat *Pan,
		jeFloat *Frequency)
{
	jeVec3d			ViewPos, LocalPos, Dist;
	jeSound3d_Cfg	Cfg;
	float			Magnitude;
	jeVec3d			Origin = {0.0f, 0.0f, 0.0f};
	jeXForm3d		CXForm;
	//int32			Leaf1, Leaf2;

	assert( World     != NULL );
	assert( MXForm    != NULL );
	assert( SndPos    != NULL );
	assert( Volume    != NULL );
	assert( Pan       != NULL );
	assert( Frequency != NULL );

	
	LocalPos = MXForm->Translation;
	// Transform the sound to view space
	jeXForm3d_GetTranspose(MXForm, &CXForm);
	jeXForm3d_Transform( &CXForm, SndPos, &ViewPos);

#if 0
	// FIXME: Need to check these and return TRUE or FALSE
	jeWorld_GetLeaf((jeWorld*)World, &LocalPos, &Leaf1);
	jeWorld_GetLeaf((jeWorld*)World, SndPos, &Leaf2);
	
	if (!jeWorld_LeafMightSeeLeaf((jeWorld*)World, Leaf1, Leaf2, 0))
	{
		Magnitude = 0.0f;
		Dist.X = 0.0f;				// Shut up compiler warning
		Cfg.Volume = 0.0f;
	}
	else
#endif
	{
		// Find the distance from the camera to the original light pos
		jeVec3d_Subtract(&LocalPos, SndPos, &Dist);

		Magnitude = jeVec3d_Length(&Dist);
		
		jeSound3D_RollOut(&Cfg, Magnitude, Min, Min*10);
	}

	jeSound3D_Pan(&Cfg, (float)atan2( (double)ViewPos.X, (double)ViewPos.Z ) );
	jeSound3D_Doppler(&Cfg, Ds);

	*Volume    = Cfg.Volume;
	*Pan       = Cfg.Pan;
	*Frequency = Cfg.Frequency;
}

