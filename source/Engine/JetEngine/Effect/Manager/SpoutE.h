/****************************************************************************************/
/*  SPOUTE.H                                                                            */
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
#ifndef SPOUTE_H
#define SPOUTE_H

#include "Jet.h"
#pragma warning( disable : 4068 )


#ifdef __cplusplus
	extern "C" {
#endif


////////////////////////////////////////////////////////////////////////////////////////
// Effect info
////////////////////////////////////////////////////////////////////////////////////////
#pragma JE_Type("EM_Spout.bmp")
typedef struct EM_Spout
{
#pragma	JE_Private

#pragma JE_Published
	jeVec3d		Position;
	jeVec3d		Angles;
	float		ParticleCreateRate;
	float		MinScale;
	float		MaxScale;
	float		MinSpeed;
	float		MaxSpeed;
	float		MinUnitLife;
	float		MaxUnitLife;
	int			SourceVariance;
	int			DestVariance;
    JE_RGBA		ColorMin;
    JE_RGBA		ColorMax;
	jeVec3d		Gravity;
	char		*BmpName;
	char		*AlphaName;
	char		*TriggerName;
	float		MinPauseTime;
	float		MaxPauseTime;
	float		TotalLife;

#pragma JE_Origin( Position )
#pragma JE_Angles(Angles)			
#pragma JE_DefaultValue( ParticleCreateRate, "0.1" )
#pragma JE_DefaultValue( MinScale, "1.0" )
#pragma JE_DefaultValue( MaxScale, "1.0" )
#pragma JE_DefaultValue( MinSpeed, "10.0" )
#pragma JE_DefaultValue( MaxSpeed, "30.0" )
#pragma JE_DefaultValue( MinUnitLife, "3.0" )
#pragma JE_DefaultValue( MaxUnitLife, "6.0" )
#pragma JE_DefaultValue( SourceVariance, "0" )
#pragma JE_DefaultValue( DestVariance, "1" )
#pragma JE_DefaultValue( ColorMin, "255.0 255.0 255.0" )
#pragma JE_DefaultValue( ColorMax, "255.0 255.0 255.0" )
#pragma JE_DefaultValue( Gravity, "0.0 0.0 0.0" )
#pragma JE_DefaultValue( TotalLife, "0.0" )
#pragma JE_DefaultValue( TriggerName, "NULL" )
#pragma JE_DefaultValue( MinPauseTime, "0.0" )
#pragma JE_DefaultValue( MaxPauseTime, "0.0" )

#pragma JE_Documentation( Position, "Location of effect, if it's not hooked to an actor" )
#pragma JE_Documentation( Angles, "Direction in which particles will shoot" )
#pragma JE_Documentation( AStart, "Actor that effect is hooked to" )
#pragma JE_Documentation( ParticleCreateRate, "Every how many seconds to add a new particle" )
#pragma JE_Documentation( MinScale, "Min scale of the textures" )
#pragma JE_Documentation( MaxScale, "Max scale of the textures" )
#pragma JE_Documentation( MinSpeed, "Min speed of the textures" )
#pragma JE_Documentation( MaxSpeed, "Max speed of the textures" )
#pragma JE_Documentation( MinUnitLife, "Min life of each texture" )
#pragma JE_Documentation( MaxUnitLife, "Max life of each texture" )
#pragma JE_Documentation( SourceVariance, "How much to vary spray source point" )
#pragma JE_Documentation( DestVariance, "How much to vary spray dest point" )
#pragma JE_Documentation( ColorMin, "Minimum RGB values for each particle" )
#pragma JE_Documentation( ColorMax, "Maximum RGB values for each particle" )
#pragma JE_Documentation( Gravity, "Gravity vector to apply to each particle" )
#pragma JE_Documentation( BmpName, "Name of bitmap file to use" )
#pragma JE_Documentation( AlphaName, "Name of alpha bitmap file to use" )
#pragma JE_Documentation( TotalLife, "How many seconds this spout lasts. Set to 0 for continuous." )
#pragma JE_Documentation( TriggerName, "Name of the associated trigger" )
#pragma JE_Documentation( MinPauseTime, "Low range of randomly chosen pause time (seconds)" )
#pragma JE_Documentation( MaxPauseTime, "High range of randomly chosen pause time (seconds)" )

} EM_Spout;


#ifdef __cplusplus
	}
#endif

#pragma warning( default : 4068 )
#endif
