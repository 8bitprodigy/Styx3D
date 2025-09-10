/****************************************************************************************/
/*  JELIGHT.C                                                                           */
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
#include <assert.h>
#include <memory.h>

#include "jeLight.h"

#include "Errorlog.h"
#include "Ram.h"

//========================================================================================
//========================================================================================
#define ZeroMem(a) memset(a, 0, sizeof(*a))
#define ZeroMemArray(a, s) memset(a, 0, sizeof(*a)*s)

typedef struct jeLight
{
	int32		RefCount;

	jeVec3d		Pos;
	jeVec3d		Color;
	jeFloat		Radius;
	jeFloat		Brightness;

	uint32		Flags;
} jeLight;

#define MAKEFOURCC(ch0, ch1, ch2, ch3)                              \
		((uint32)(uint8)(ch0) | ((uint32)(uint8)(ch1) << 8) |   \
		((uint32)(uint8)(ch2) << 16) | ((uint32)(uint8)(ch3) << 24 ))

#define JE_LIGHT_TAG			MAKEFOURCC('G', 'E', 'L', 'F')		// 'GE' 'L'ight 'F'ile
#define JE_LIGHT_VERSION		0x0000

//========================================================================================
//	jeLight_Create
//========================================================================================
JETAPI jeLight * JETCC jeLight_Create(void)
{
	jeLight		*Light;

	Light = JE_RAM_ALLOCATE_STRUCT(jeLight);

	if (!Light)
		return NULL;

	ZeroMem(Light);

	jeVec3d_Set(&Light->Color, 1.0f, 1.0f, 1.0f);

	Light->Brightness = 1.0f;
	Light->Radius = 200.0f;

	Light->RefCount = 1;

	return Light;
}

//========================================================================================
//	jeLight_CreateFromFile
//========================================================================================
JETAPI jeLight * JETCC jeLight_CreateFromFile(jeVFile *VFile, jePtrMgr *PtrMgr)
{
	jeLight		*Light;
	uint32		Tag;
	uint16		Version;

	if (PtrMgr)
	{
		if (!jePtrMgr_ReadPtr(PtrMgr, VFile, (void **)&Light))
			return NULL;

		if (Light)
		{
			if (!jeLight_CreateRef(Light))
				return NULL;

			return Light;		// Ptr found in stack, return it
		}
	}

	// Read header info
	if (!jeVFile_Read(VFile, &Tag, sizeof(Tag)))
		return NULL;

	if (Tag != JE_LIGHT_TAG)
		return NULL;

	if (!jeVFile_Read(VFile, &Version, sizeof(Version)))
		return NULL;

	if (Version != JE_LIGHT_VERSION)
		return NULL;

	// Create and Read the light
	Light = JE_RAM_ALLOCATE_STRUCT(jeLight);

	if (!Light)
		return NULL;

	ZeroMem(Light);
	
	if (!jeVFile_Read(VFile, Light, sizeof(jeLight)))
		goto ExitWithError;

	Light->RefCount = 1;	// Icestorm: Moved it AFTER reading Light, so saved refs are ignored

	if (PtrMgr)
	{
		// Push the ptr on the stack
		if (!jePtrMgr_PushPtr(PtrMgr, Light))
			goto ExitWithError;
	}

	return Light;

	ExitWithError:
	{
		if (Light)
			jeRam_Free(Light);

		return NULL;
	}
}

//========================================================================================
//	jeLight_CreateFromLight
//========================================================================================
JETAPI jeLight * JETCC jeLight_CreateFromLight(const jeLight *SrcLight)
{
	jeLight		*Light;

	Light = JE_RAM_ALLOCATE_STRUCT(jeLight);

	if (!Light)
		return NULL;

	ZeroMem(Light);

	*Light = *SrcLight;
	Light->RefCount = 1;

	return Light;
}

//========================================================================================
//	jeLight_WriteToFile
//========================================================================================
JETAPI jeBoolean JETCC jeLight_WriteToFile(const jeLight *Light, jeVFile *VFile, jePtrMgr *PtrMgr)
{
	uint32				Tag;
	uint16				Version;

	if (PtrMgr)
	{
		uint32		Count;

		if (!jePtrMgr_WritePtr(PtrMgr, VFile, (void*)Light, &Count))
			return JE_FALSE;

		if (Count)
			return JE_TRUE;		// Ptr was on stack, so return
	}

	assert(jeLight_IsValid(Light) == JE_TRUE);

	// Write TAG
	Tag = JE_LIGHT_TAG;

	if (!jeVFile_Write(VFile, &Tag, sizeof(Tag)))
		return JE_FALSE;

	// Write version
	Version = JE_LIGHT_VERSION;

	if (!jeVFile_Write(VFile, &Version, sizeof(Version)))
		return JE_FALSE;
	
	// Write the light
	if (!jeVFile_Write(VFile, Light, sizeof(jeLight)))
		return JE_FALSE;

	if (PtrMgr)
	{
		// Push the ptr on the stack
		if (!jePtrMgr_PushPtr(PtrMgr, (void*)Light))
			return JE_FALSE;
	}

	return JE_TRUE;
}

//========================================================================================
//	jeLight_CreateRef
//========================================================================================
JETAPI jeBoolean JETCC jeLight_CreateRef(jeLight *Light)
{
	assert(jeLight_IsValid(Light) == JE_TRUE);

	assert(Light->RefCount < (0xFFFFFFFF>>1)-1);
	
	Light->RefCount++;

	return JE_TRUE;
}

//========================================================================================
//	jeLight_Destroy
//========================================================================================
JETAPI void JETCC jeLight_Destroy(jeLight **Light)
{
	assert(Light);
	assert(jeLight_IsValid(*Light) == JE_TRUE);

	(*Light)->RefCount --;

	if ((*Light)->RefCount == 0)
		jeRam_Free(*Light);

	*Light = NULL;
}

//========================================================================================
//	jeLight_IsValid
//========================================================================================
JETAPI jeBoolean JETCC jeLight_IsValid(const jeLight *Light)
{
	if (!Light)
		return JE_FALSE;

	if (Light->RefCount <= 0)
		return JE_FALSE;

	return JE_TRUE;
}

//========================================================================================
// jeLight_SetAttributes
//========================================================================================
JETAPI jeBoolean JETCC jeLight_SetAttributes(jeLight *Light, 
								const jeVec3d *Pos, 
								const jeVec3d *Color, 
								jeFloat Radius, 
								jeFloat Brightness, 
								uint32 Flags)
{
	assert(jeLight_IsValid(Light) == JE_TRUE);

	Light->Pos = *Pos;
	Light->Color = *Color;
	Light->Radius = Radius;
	Light->Brightness = Brightness;
	Light->Flags = Flags;

	Light->Radius = jeLight_GetRadius(Light);

	return JE_TRUE;
}

//========================================================================================
// jeLight_GetAttributes
//========================================================================================
JETAPI jeBoolean JETCC jeLight_GetAttributes(const jeLight *Light, 
								jeVec3d *Pos, 
								jeVec3d *Color, 
								jeFloat *Radius, 
								jeFloat *Brightness, 
								uint32 *Flags)
{
	assert(jeLight_IsValid(Light) == JE_TRUE);

	if (Pos)
		*Pos = Light->Pos;
	if (Color)
		*Color = Light->Color;
	if (Radius)
		*Radius = Light->Radius;
	if (Brightness)
		*Brightness = Light->Brightness;
	if (Flags)
		*Flags = Light->Flags;

	return JE_TRUE;
}

//========================================================================================
//	jeLight_GetFlags
//========================================================================================
JETAPI uint32 JETCC jeLight_GetFlags(const jeLight *Light)
{
	assert(jeLight_IsValid(Light) == JE_TRUE);

	return Light->Flags;
}

//========================================================================================
//	jeLight_GetRadius
//========================================================================================
JETAPI jeFloat JETCC jeLight_GetRadius(const jeLight *Light)
{
	switch( Light->Flags & JE_LIGHT_FLAG_TYPEMASK )
	{
		case JE_LIGHT_FLAG_SUN:
			return 999999999999999.0f;

		case 0: // <> for backward compatibility
		case JE_LIGHT_FLAG_LINEAR_FALLOFF:
			return Light->Radius;

		case JE_LIGHT_FLAG_INVERSE_FALLOFF:
			return Light->Brightness;

		case JE_LIGHT_FLAG_INVERSE_SQUARE_FALLOFF:
			return jeFloat_Sqrt(Light->Brightness);

		default:
			jeErrorLog_AddString(-1,"jeLight : unknown type!",NULL);
			return 0.0f;
	}
}

//========================================================================================
//	jeLight_SetSunLight
//========================================================================================
JETAPI jeBoolean JETCC jeLight_SetSunLight(jeLight *Light, 
								const jeVec3d *DirectionToSun, 
								const jeVec3d *Color, 
								jeFloat Brightness)
{
	assert(jeLight_IsValid(Light) == JE_TRUE);

	Light->Pos = *DirectionToSun;
	Light->Color = *Color;
	Light->Brightness = Brightness;
	Light->Flags = JE_LIGHT_FLAG_SUN;

	Light->Radius = jeLight_GetRadius(Light);

return JE_TRUE;
}

JETAPI jeBoolean JETCC jeLight_SetInverseLight(jeLight *Light, 
								const jeVec3d *Pos, 
								const jeVec3d *Color, 
								jeFloat Brightness)
{
	assert(jeLight_IsValid(Light) == JE_TRUE);

	Light->Pos = *Pos;
	Light->Color = *Color;
	Light->Brightness = Brightness;
	Light->Flags = JE_LIGHT_FLAG_INVERSE_FALLOFF;

	Light->Radius = jeLight_GetRadius(Light);

return JE_TRUE;
}

JETAPI jeBoolean JETCC jeLight_SetInverseSquaredLight(jeLight *Light, 
								const jeVec3d *Pos, 
								const jeVec3d *Color, 
								jeFloat Brightness)
{
	assert(jeLight_IsValid(Light) == JE_TRUE);

	Light->Pos = *Pos;
	Light->Color = *Color;
	Light->Brightness = Brightness;
	Light->Flags = JE_LIGHT_FLAG_INVERSE_SQUARE_FALLOFF;

	Light->Radius = jeLight_GetRadius(Light);

return JE_TRUE;
}

//========================================================================================
//	jeLight_CalculateLighting
//========================================================================================
JETAPI jeBoolean	JETCC jeLight_CalculateLighting(const jeLight * Light,const jeVec3d *pPos,const jeVec3d *pNormal,
													jeRGBA * pColor)
{
jeFloat scale;

	assert(jeLight_IsValid(Light));

	pColor->a = 255.0f; 

	if ( (Light->Flags & JE_LIGHT_FLAG_TYPEMASK) == JE_LIGHT_FLAG_SUN )
	{
		scale = jeVec3d_DotProduct(&(Light->Pos),pNormal);

		if ( scale < 0.0f )
		{
			pColor->r = pColor->g = pColor->b = 0.0f;
			return JE_TRUE;
		}
	}
	else
	{
	jeVec3d v;
	jeFloat len;

		jeVec3d_Subtract(&(Light->Pos),pPos,&v);

		len = jeVec3d_Normalize(&v);

		scale = jeVec3d_DotProduct(&v,pNormal);

		if ( scale < 0.0f )
		{
			pColor->r = pColor->g = pColor->b = 0.0f;
			return JE_TRUE;
		}

		switch( Light->Flags & JE_LIGHT_FLAG_TYPEMASK )
		{
			case 0: // <> for backward compatibility
			case JE_LIGHT_FLAG_LINEAR_FALLOFF:
				
				if ( len >= Light->Radius )
					scale = 0.0f;
				else
					scale *= ( 1.0f - (len / Light->Radius) );

				break;

			case JE_LIGHT_FLAG_INVERSE_FALLOFF:

				scale /= len;

				break;

			case JE_LIGHT_FLAG_INVERSE_SQUARE_FALLOFF:
			
				scale /= (len * len);

				break;

			default:
				jeErrorLog_AddString(-1,"jeLight : unknown type!",NULL);
				return JE_FALSE;			
		}
	}

	scale *= Light->Brightness;
	pColor->r = Light->Color.X * scale;
	pColor->g = Light->Color.Y * scale;
	pColor->b = Light->Color.Z * scale;

return JE_TRUE;
}
