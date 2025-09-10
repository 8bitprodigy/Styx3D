/****************************************************************************************/
/*  AOPTIONS.C																			*/
/*                                                                                      */
/*  Author: Jim Mischel		                                                            */
/*  Description: Actor studio/builder INI file options API.								*/
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
#include "AOptions.h"
#include "ram.h"
#include <assert.h>

// Include windows for profile reading stuff...
#pragma warning(disable : 4201 4214 4115 4514)
#include <windows.h>
#pragma warning(default : 4201 4214 4115)

struct tag_AOptions
{
	char ViewerPath[MAX_PATH];
	char MaxPath[MAX_PATH];
	jeBoolean OptFlag;
	int OptLevel;
};

static const char SectionName[]	= "AStudio";
static const char ViewerKey[]	= "ViewerPath";
static const char MaxKey[]		= "MaxPath";
static const char OptFlagKey[]	= "OptFlag";
static const char OptLevelKey[]	= "OptLevel";

AOptions *AOptions_Create (void)
{
	AOptions *Options = JE_RAM_ALLOCATE_STRUCT (AOptions);
	if (Options != NULL)
	{
		strcpy (Options->ViewerPath, "c:\\Program Files\\Eclipse\\Jet3D\\ActView.exe");
		strcpy (Options->MaxPath, "c:\\3dsmax2\\3dsmax.exe");
		Options->OptFlag = JE_TRUE;
		Options->OptLevel = 4;
	}
	return Options;
}


AOptions *AOptions_CreateFromFile (const char *IniFilename)
{
	AOptions *Options = AOptions_Create ();
	if (Options != NULL)
	{
		int Flag;
		// Use default names for these two.
		// Probably at some time should try to find them at install?
		GetPrivateProfileString (SectionName, ViewerKey, Options->ViewerPath, Options->ViewerPath, MAX_PATH, IniFilename);
		GetPrivateProfileString (SectionName, MaxKey, Options->MaxPath, Options->MaxPath, MAX_PATH, IniFilename);
		Flag = GetPrivateProfileInt (SectionName, OptFlagKey, Options->OptFlag, IniFilename);
		Options->OptFlag = Flag ? JE_TRUE : JE_FALSE;
		Options->OptLevel = GetPrivateProfileInt (SectionName, OptLevelKey, Options->OptLevel, IniFilename);
	}

	return Options;
}

void AOptions_Destroy (AOptions **pOptions)
{
	jeRam_Free (*pOptions);
}

jeBoolean AOptions_WriteToFile (const AOptions *Options, const char *IniFilename)
{
	char OptLevelString[2] = "0";

	WritePrivateProfileString (SectionName, ViewerKey, Options->ViewerPath, IniFilename);
	WritePrivateProfileString (SectionName, MaxKey, Options->MaxPath, IniFilename);
	
	WritePrivateProfileString (SectionName, OptFlagKey, Options->OptFlag ? "1" : "0", IniFilename);

	if ((Options->OptLevel >= 0) && (Options->OptLevel <= 9))
	{
		*OptLevelString = (char)(Options->OptLevel + '0');
	}
	WritePrivateProfileString (SectionName, OptLevelKey, OptLevelString, IniFilename);

	return JE_TRUE;
}


const char *AOptions_GetViewerPath (const AOptions *Options)
{
	return Options->ViewerPath;
}

jeBoolean AOptions_SetViewerPath (AOptions *Options, const char *ViewerPath)
{
	strcpy (Options->ViewerPath, ViewerPath);
	return JE_TRUE;
}

const char *AOptions_Get3DSMaxPath (const AOptions *Options)
{
	return Options->MaxPath;
}

jeBoolean AOptions_Set3DSMaxPath (AOptions *Options, const char *MaxPath)
{
	strcpy (Options->MaxPath, MaxPath);
	return JE_TRUE;
}


jeBoolean AOptions_GetMotionOptimizationFlag (const AOptions *Options)
{
	return Options->OptFlag;
}

jeBoolean AOptions_SetMotionOptimizationFlag (AOptions *Options, jeBoolean Flag)
{
	Options->OptFlag = Flag;
	return JE_TRUE;
}

int AOptions_GetMotionOptimizationLevel (const AOptions *Options)
{
	return Options->OptLevel;
}

jeBoolean AOptions_SetMotionOptimizationLevel (AOptions *Options, int OptLevel)
{
	assert (OptLevel >= 0);
	assert (OptLevel <= 9);

	Options->OptLevel = OptLevel;
	return JE_TRUE;
}
