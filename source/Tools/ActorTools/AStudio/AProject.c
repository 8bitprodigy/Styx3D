/****************************************************************************************/
/*  APROJECT.C                                                                          */
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
/*
  AProject.cpp -- Actor Studio Project file API

  Copyright © 1998, Eclipse Entertainment
*/
#include "AProject.h"
#include "array.h"
#include "ram.h"
#include <assert.h>
#include "util.h"
#include "ErrorLog.h"
#include "FilePath.h"
#include <stdio.h>

#pragma warning(disable : 4201 4214 4115 4514)
#include <windows.h>
#pragma warning(default : 4201 4214 4115)

#define APJ_VERSION_MAJOR 0
#define APJ_VERSION_MINOR 90

// project file version string.
static const char AProject_VersionString[] = "APJ Version %d.%d";


typedef struct tag_ApjOutput
{
	char *Filename;
	ApjOutputFormat Fmt;
} ApjOutput;

typedef struct tag_ApjPaths
{
	jeBoolean ForceRelative;
	char *Materials;
	char *TempFiles;
} ApjSearchPaths;

typedef struct tag_ApjBody
{
	char *Filename;
	ApjBodyFormat Fmt;
} ApjBody;

// Entry in Materials section
typedef struct
{
	char *Name;			// Material name
	ApjMaterialFormat Fmt;  // type
	char *Filename;		// texture filename (may be NULL)
	JE_RGBA Color;		//
} ApjMaterialEntry;

// materials section
typedef struct tag_ApjMaterials
{
	int Count;
	Array *Items;	// array of ApjMaterialEntry structures
} ApjMaterials;


// Entry in motions section
typedef struct
{
	char *Name;				// motion name
	ApjMotionFormat Fmt;	// motion file format
	char *Filename;			// file that contains the motion
	jeBoolean OptFlag;		// optimization flag
	int OptLevel;			// motion optimization level
	char *Bone;				// name of root bone to grab
} ApjMotionEntry;

// motions section
typedef struct tag_ApjMotions
{
	int Count;
	Array *Items;	// Array of ApjMotionEntry structures
} ApjMotions;



struct tag_AProject
{
	ApjOutput		Output;
	ApjSearchPaths	Paths;
	ApjBody			Body;
	ApjMaterials	Materials;
	ApjMotions		Motions;
};


// extensions for body types.
// these must match the ApjBodyFormat enumeration
static const char *BodyExtensions[] = {".max", ".nfo", ".bdy", ".act"};



// Determine body format from file extension.
// Returns ApjBody_Invalid if unknown extension
ApjBodyFormat AProject_GetBodyFormatFromFilename (const char *Name)
{
	char Ext[MAX_PATH];
	int x;

	if (FilePath_GetExt (Name, Ext) != JE_FALSE)
	{
		for (x = 0; x <= ApjBody_Act; ++x)
		{
			if (stricmp (BodyExtensions[x], Ext) == 0)
			{
				return x+1;
			}
		}
	}
	return ApjBody_Invalid;
}

static const char *MotionExtensions[] = {".max", ".key", ".mot", ".act"};

ApjMotionFormat AProject_GetMotionFormatFromFilename (const char *Filename)
{
	char Ext[MAX_PATH];
	int x;

	if (FilePath_GetExt (Filename, Ext) != JE_FALSE)
	{
		for (x = 0; x < ApjMotion_TypeCount; ++x)
		{
			if (stricmp (MotionExtensions[x], Ext) == 0)
			{
				return x+1;
			}
		}
	}
	return ApjMotion_Invalid;
}


// Create empty project
AProject *AProject_Create (const char *OutputName)
{
	AProject *pProject;
	jeBoolean NoErrors;
	char OutputNameAndExt[MAX_PATH];

	assert (OutputName != NULL);

	pProject = JE_RAM_ALLOCATE_STRUCT (AProject);
	if (pProject == NULL)
	{
		jeErrorLog_AddString (JE_ERR_MEMORY_RESOURCE, "Allocating project structure",NULL);
		return NULL;
	}

	// Initialize defaults
	// Output
	pProject->Output.Filename = NULL;
	pProject->Output.Fmt = ApjOutput_Binary;

	// Paths
	pProject->Paths.ForceRelative = JE_TRUE;
	pProject->Paths.Materials = NULL;

	// Body
	pProject->Body.Filename = NULL;
	pProject->Body.Fmt = ApjBody_Max;

	// Materials
	pProject->Materials.Count = 0;
	pProject->Materials.Items = NULL;

	// Motions
	pProject->Motions.Count = 0;
	pProject->Motions.Items = NULL;

	FilePath_SetExt (OutputName, ".act", OutputNameAndExt);

	// Allocate required memory
	NoErrors = 
		((pProject->Output.Filename = Util_Strdup (OutputNameAndExt)) != NULL) &&
		((pProject->Paths.Materials = Util_Strdup ("")) != NULL) &&
		((pProject->Paths.TempFiles = Util_Strdup (".\\BldTemp")) != NULL) &&
		((pProject->Body.Filename	= Util_Strdup ("")) != NULL);
		
	// Build motion and material arrays.  Initially empty.
	NoErrors = NoErrors &&
		((pProject->Materials.Items = Array_Create (1, sizeof (ApjMaterialEntry))) != NULL);

	NoErrors = NoErrors &&
		((pProject->Motions.Items = Array_Create (1, sizeof (ApjMotionEntry))) != NULL);

	// if unsuccessful, destroy any allocated data
	if (!NoErrors)
	{
		jeErrorLog_AddString (JE_ERR_MEMORY_RESOURCE, "Initializing project structure",NULL);
		if (pProject != NULL)
		{
			AProject_Destroy (&pProject);
		}
	}

	return pProject;
}

// Free all memory allcated by project structure.
void AProject_Destroy (AProject **ppProject)
{
	AProject *pProject;

	assert (ppProject != NULL);
	pProject = *ppProject;
	assert (pProject != NULL);

	while (pProject->Materials.Count > 0)
	{
		AProject_RemoveMaterial (pProject, pProject->Materials.Count-1);
	}

	while (pProject->Motions.Count > 0)
	{
		AProject_RemoveMotion (pProject, pProject->Motions.Count-1);
	}

	if (pProject->Output.Filename != NULL)	jeRam_Free (pProject->Output.Filename);
	if (pProject->Paths.Materials != NULL)	jeRam_Free (pProject->Paths.Materials);
	if (pProject->Paths.TempFiles != NULL)	jeRam_Free (pProject->Paths.TempFiles);
	if (pProject->Body.Filename != NULL)	jeRam_Free (pProject->Body.Filename);

	if (pProject->Materials.Items != NULL)	Array_Destroy (&pProject->Materials.Items);
	if (pProject->Motions.Items != NULL)	Array_Destroy (&pProject->Motions.Items);

	jeRam_Free (*ppProject);
}

typedef enum
{
	READ_SUCCESS,
	READ_ERROR,
	READ_EOF
} ApjReadResult;

static ApjReadResult AProject_GetNonBlankLine (jeVFile *FS, char *Buffer, int BufferSize)
{
	while (!jeVFile_EOF (FS))
	{
		if (jeVFile_GetS (FS, Buffer, BufferSize) == JE_FALSE)
		{
			// some kind of error...
			return READ_ERROR;
		}

		// search for and remove any newlines
		{
			char *c = strchr (Buffer, '\n');
			if (c != NULL)
			{
				*c = '\0';
			}
		}
					
		// if the line is not blank, then return it...
		{
			char *c = Buffer;

			while ((*c != '\0') && (c < (Buffer + BufferSize)))
			{
				if (!isspace (*c))
				{
					return READ_SUCCESS;
				}
				++c;
			}
		}
		// line's blank, go get the next one...
	}
	// end of file
	return READ_EOF;
}

static jeBoolean AProject_CheckFileVersion (jeVFile *FS)
{
	char VersionString[1024];
	int VersionMajor, VersionMinor;
	int rslt;

	// read string
	if (AProject_GetNonBlankLine (FS, VersionString, sizeof (VersionString)) != READ_SUCCESS)
	{
		// error...
		jeErrorLog_AddString (JE_ERR_FILEIO_READ, "Reading project version string",NULL);
		return JE_FALSE;
	}

	// format must match project version string
	rslt = sscanf (VersionString, AProject_VersionString, &VersionMajor, &VersionMinor);
	if (rslt != 2)
	{
		jeErrorLog_AddString (JE_ERR_FILEIO_FORMAT, "Incompatible file type",NULL);
		return JE_FALSE;
	}

	// make sure we know how to read this version
	if ((VersionMajor < APJ_VERSION_MAJOR) ||
		((VersionMajor == APJ_VERSION_MAJOR) && (VersionMinor <= APJ_VERSION_MINOR)))
	{
		return JE_TRUE;
	}

	jeErrorLog_AddString (JE_ERR_FILEIO_VERSION, "Incompatible project file version",NULL);
	return JE_FALSE;
}

// Section keys
static const char Paths_Key[]			= "[Paths]";
static const char ForceRelative_Key[]	= "ForceRelative";
static const char MaterialsPath_Key[]	= "MaterialsPath";
static const char TempFilesPath_Key[]	= "TempFiles";
static const char EndPaths_Key[]		= "[EndPaths]";

static const char Output_Key[]			= "[Output]";
static const char OutputFilename_Key[]	= "Filename";
static const char OutputFormat_Key[]	= "Format";
static const char EndOutput_Key[]		= "[EndOutput]";

static const char Body_Key[]			= "[Body]";
static const char BodyFilename_Key[]	= "Filename";
static const char BodyFormat_Key[]		= "Format";
static const char EndBody_Key[]			= "[EndBody]";

static const char Materials_Key[]		= "[Materials]";
static const char MaterialsCount_Key[]	= "Count";
static const char EndMaterials_Key[]	= "[EndMaterials]";

static const char Motions_Key[]			= "[Motions]";
static const char MotionsCount_Key[]	= "Count";
static const char EndMotions_Key[]		= "[EndMotions]";

// strip leading spaces from string before copying it
static jeBoolean AProject_SetString (char **pString, const char *NewValue)
{
	const char *c = NewValue;

	while ((c != '\0') && isspace (*c))
	{
		++c;
	}

	return Util_SetString (pString, c);
}

// Load [Paths] section
static jeBoolean AProject_LoadPathsInfo (AProject *pProject, jeVFile *FS)
{

	for (;;)	// infinite loop
	{
		char Buffer[1024];
		char *c;

		if (AProject_GetNonBlankLine (FS, Buffer, sizeof (Buffer)) != READ_SUCCESS)
		{
			jeErrorLog_AddString (JE_ERR_FILEIO_READ, "Loading paths info",NULL);
			return JE_FALSE;
		}

		if (_strnicmp (Buffer, ForceRelative_Key, strlen (ForceRelative_Key)) == 0)
		{
			c = &Buffer[strlen (ForceRelative_Key)];
			// Set force relative flag if not explicitly turned off
			pProject->Paths.ForceRelative = ((*c == '\0') || (*(c+1) != '0')) ? JE_TRUE : JE_FALSE;
		}
		else if (_strnicmp (Buffer, MaterialsPath_Key, strlen (MaterialsPath_Key)) == 0)
		{
			c = &Buffer[strlen (MaterialsPath_Key)];
			AProject_SetString (&pProject->Paths.Materials, c);
		}
		else if (_strnicmp (Buffer, TempFilesPath_Key, strlen (TempFilesPath_Key)) == 0)
		{
			c = &Buffer[strlen (TempFilesPath_Key)];
			AProject_SetString (&pProject->Paths.TempFiles, c);
		}
		else if (_strnicmp (Buffer, EndPaths_Key, strlen (EndPaths_Key)) == 0)
		{
			return JE_TRUE;
		}
		else
		{
			// bad entry...
			jeErrorLog_AddString (JE_ERR_FILEIO_FORMAT, "Bad Paths section entry",NULL);
			return JE_FALSE;
		}
	}
}

static jeBoolean AProject_WritePathsInfo (const AProject *pProject, jeVFile *FS)
{
	if ((jeVFile_Printf (FS, "%s\r\n", Paths_Key) == JE_FALSE) ||
		(jeVFile_Printf (FS, "%s %c\r\n", ForceRelative_Key, (pProject->Paths.ForceRelative == JE_TRUE) ? '1' : '0') == JE_FALSE) ||
		(jeVFile_Printf (FS, "%s %s\r\n", MaterialsPath_Key, pProject->Paths.Materials) == JE_FALSE) ||
		(jeVFile_Printf (FS, "%s %s\r\n", TempFilesPath_Key, pProject->Paths.TempFiles) == JE_FALSE) ||
		(jeVFile_Printf (FS, "%s\r\n", EndPaths_Key) == JE_FALSE))
	{
		jeErrorLog_AddString (JE_ERR_FILEIO_WRITE, "Writing Paths section",NULL);
		return JE_FALSE;
	}

	return JE_TRUE;
}

static jeBoolean AProject_LoadOutputInfo (AProject *pProject, jeVFile *FS)
{
	for (;;)
	{
		char Buffer[1024];
		char *c;

		if (AProject_GetNonBlankLine (FS, Buffer, sizeof (Buffer)) != READ_SUCCESS)
		{
			jeErrorLog_AddString (JE_ERR_FILEIO_READ, "Loading output file info",NULL);
			return JE_FALSE;
		}

		if (_strnicmp (Buffer, OutputFilename_Key, strlen (OutputFilename_Key)) == 0)
		{
			c = &Buffer[strlen (OutputFilename_Key)];
			AProject_SetString (&pProject->Output.Filename, c);
		}
		else if (_strnicmp (Buffer, OutputFormat_Key, strlen (OutputFormat_Key)) == 0)
		{
			c = &Buffer[strlen (OutputFormat_Key)];
			// format assumed binary unless text specified
			pProject->Output.Fmt = ((*c == '\0') || (*(c+1) != '0')) ? ApjOutput_Binary : ApjOutput_Text;
		}
		else if (_strnicmp (Buffer, EndOutput_Key, strlen (EndOutput_Key)) == 0)
		{
			return JE_TRUE;
		}
		else
		{
			// bad entry
			jeErrorLog_AddString (JE_ERR_FILEIO_FORMAT, "Bad Output section entry",NULL);
			return JE_FALSE;
		}
	}
}

static jeBoolean AProject_WriteOutputInfo (const AProject *pProject, jeVFile *FS)
{
	if ((jeVFile_Printf (FS, "%s\r\n", Output_Key) == JE_FALSE) ||
		(jeVFile_Printf (FS, "%s %s\r\n", OutputFilename_Key, pProject->Output.Filename) == JE_FALSE) ||
		(jeVFile_Printf (FS, "%s %c\r\n", OutputFormat_Key, (pProject->Output.Fmt == ApjOutput_Binary) ? '1' : '0') == JE_FALSE) ||
		(jeVFile_Printf (FS, "%s\r\n", EndOutput_Key) == JE_FALSE))
	{
		jeErrorLog_AddString (JE_ERR_FILEIO_WRITE, "Writing Output section",NULL);
		return JE_FALSE;
	}
	return JE_TRUE;
}


static jeBoolean AProject_LoadBodyInfo (AProject *pProject, jeVFile *FS)
{
	for (;;)
	{
		char Buffer[1024];
		char *c;

		if (AProject_GetNonBlankLine (FS, Buffer, sizeof (Buffer)) != READ_SUCCESS)
		{
			jeErrorLog_AddString (JE_ERR_FILEIO_READ, "Loading Body info",NULL);
			return JE_FALSE;
		}

		if (_strnicmp (Buffer, BodyFilename_Key, strlen (BodyFilename_Key)) == 0)
		{
			c = &Buffer[strlen (BodyFilename_Key)];
			AProject_SetString (&pProject->Body.Filename, c);
			// if we haven't loaded a format yet, try to get it from the filename
			// this will be overridden by a format if it's there...
			if (pProject->Body.Fmt == ApjBody_Invalid)
			{
				pProject->Body.Fmt = AProject_GetBodyFormatFromFilename (pProject->Body.Filename);
			}
		}
		else if (_strnicmp (Buffer, BodyFormat_Key, strlen (BodyFormat_Key)) == 0)
		{
			c = &Buffer[strlen (BodyFormat_Key)];
			// Determine body file format
			if (*c != '\0')
			{
				switch (*(c+1))
				{
					case '1' : pProject->Body.Fmt = ApjBody_Max; break;
					case '2' : pProject->Body.Fmt = ApjBody_Nfo; break;
					case '3' : pProject->Body.Fmt = ApjBody_Bdy; break;
					case '4' : pProject->Body.Fmt = ApjBody_Act; break;
					default  : pProject->Body.Fmt = ApjBody_Invalid; break;
				}
			}
			else
			{
				pProject->Body.Fmt = ApjBody_Invalid;
			}

			if (pProject->Body.Fmt == ApjBody_Invalid)
			{
				jeErrorLog_AddString (JE_ERR_FILEIO_FORMAT, "Unknown body file format",NULL);
				return JE_FALSE;
			}
		}
		else if (_strnicmp (Buffer, EndBody_Key, strlen (EndBody_Key)) == 0)
		{
			if (pProject->Body.Fmt == ApjBody_Invalid)
			{
				jeErrorLog_AddString (JE_ERR_FILEIO_FORMAT, "Unknown body file format",NULL);
				return JE_FALSE;
			}
			return JE_TRUE;
		}
		else
		{
			// bad entry
			jeErrorLog_AddString (JE_ERR_FILEIO_FORMAT, "Bad Body section entry",NULL);
			return JE_FALSE;
		}
	}
}

// ugly, but it works...
static char AProject_BodyFormatToChar (ApjBodyFormat Fmt)
{
	return (char)(((int)Fmt) + '0');
}

static jeBoolean AProject_WriteBodyInfo (const AProject *pProject, jeVFile *FS)
{
	if ((jeVFile_Printf (FS, "%s\r\n", Body_Key) == JE_FALSE) ||
		(jeVFile_Printf (FS, "%s %s\r\n", BodyFilename_Key, pProject->Body.Filename) == JE_FALSE) ||
		(jeVFile_Printf (FS, "%s %c\r\n", BodyFormat_Key, AProject_BodyFormatToChar (pProject->Body.Fmt)) == JE_FALSE) ||
		(jeVFile_Printf (FS, "%s\r\n", EndBody_Key) == JE_FALSE))
	{
		jeErrorLog_AddString (JE_ERR_FILEIO_WRITE, "Writing Body section",NULL);
		return JE_FALSE;
	}
	return JE_TRUE;
}

static jeBoolean AProject_UnquoteString (char *TheString)
{
	char *c = &TheString[strlen (TheString)-1];
	if (*c != '"')
	{
		return JE_FALSE;	// no ending quote
	}
	*c = '\0';	// rip quote from the end

	if (*TheString != '"')
	{
		return JE_FALSE;	// no beginning quote
	}
	strcpy (TheString, (TheString+1));
	return JE_TRUE;
}

static jeBoolean AProject_ParseMaterial 
	(
	  char *Buffer,
	  char *Name,
	  ApjMaterialFormat *Fmt,
	  char *Filename,
	  JE_RGBA *Color
	)
{
	char *NameStr, *FmtStr, *FilenameStr;
	char *rStr, *gStr, *bStr, *aStr;

	// parse the items from the line
	if ((NameStr	= strtok (Buffer, ",")) == NULL)return JE_FALSE;
	if ((FmtStr		= strtok (NULL, ",")) == NULL)	return JE_FALSE;
	if ((FilenameStr= strtok (NULL, ",")) == NULL)	return JE_FALSE;
	if ((rStr		= strtok (NULL, ",")) == NULL)	return JE_FALSE;
	if ((gStr		= strtok (NULL, ",")) == NULL)	return JE_FALSE;
	if ((bStr		= strtok (NULL, ",")) == NULL)	return JE_FALSE;
	if ((aStr		= strtok (NULL, ",")) == NULL)	return JE_FALSE;

	// set the items
	*Fmt = (*FmtStr == '1') ? ApjMaterial_Texture : ApjMaterial_Color;

	if (AProject_UnquoteString (NameStr)	 == JE_FALSE) return JE_FALSE;
	if (AProject_UnquoteString (FilenameStr) == JE_FALSE) return JE_FALSE;

	strcpy (Name, NameStr);
	strcpy (Filename, FilenameStr);

	Color->r = (float)atof (rStr);
	Color->g = (float)atof (gStr);
	Color->b = (float)atof (bStr);
	Color->a = (float)atof (aStr);

	return JE_TRUE;
}

static jeBoolean AProject_LoadMaterialsInfo (AProject *pProject, jeVFile *FS)
{
	for (;;)
	{
		char Buffer[1024];
		char *c;

		if (AProject_GetNonBlankLine (FS, Buffer, sizeof (Buffer)) != READ_SUCCESS)
		{
			jeErrorLog_AddString (JE_ERR_FILEIO_READ, "Loading Materials info",NULL);
			return JE_FALSE;
		}

		if (_strnicmp (Buffer, MaterialsCount_Key, strlen (MaterialsCount_Key)) == 0)
		{
			int Count = 0;

			c = &Buffer[strlen (MaterialsCount_Key)];
			if (*c != '\0')
			{
				Count = atoi (c+1);
				if (Count < 0)
				{
					jeErrorLog_AddString (JE_ERR_FILEIO_FORMAT, "Negative materials count",NULL);
					return JE_FALSE;
				}
			}
			// load and add each material
			for (; Count > 0; --Count)
			{
				char Name[MAX_PATH];
				ApjMaterialFormat Fmt;
				char Filename[MAX_PATH];
				JE_RGBA Color;
				int Index;

				if (AProject_GetNonBlankLine (FS, Buffer, sizeof (Buffer)) != READ_SUCCESS)
				{
					jeErrorLog_AddString (JE_ERR_FILEIO_READ, "Loading Materials info",NULL);
					return JE_FALSE;
				}

				// parse the material's parts
				if (AProject_ParseMaterial (Buffer, Name, &Fmt, Filename, &Color) == JE_FALSE)
				{
					jeErrorLog_AddString (JE_ERR_FILEIO_FORMAT, "Bad material",NULL);
					return JE_FALSE;
				}
				// and then add the material.
				if (AProject_AddMaterial (pProject, Name, Fmt, Filename, Color.r, Color.g, Color.b, Color.a, &Index) == JE_FALSE)
				{
					return JE_FALSE;
				}
			}
		}
		else if (_strnicmp (Buffer, EndMaterials_Key, strlen (EndMaterials_Key)) == 0)
		{
			return JE_TRUE;
		}
		else
		{
			// bad entry
			jeErrorLog_AddString (JE_ERR_FILEIO_FORMAT, "Bad Materials section entry",NULL);
			return JE_FALSE;
		}
	}
}

static jeBoolean AProject_WriteMaterialsInfo (const AProject *pProject, jeVFile *FS)
{
	int i;

	if ((jeVFile_Printf (FS, "%s\r\n", Materials_Key) == JE_FALSE) ||
		(jeVFile_Printf (FS, "%s %d\r\n", MaterialsCount_Key, pProject->Materials.Count) == JE_FALSE))
	{
		goto Error;
	}

	for (i = 0; i < pProject->Materials.Count; ++i)
	{
		// format each material's information and write it
		ApjMaterialEntry *pEntry;
		char Buffer[1024];

		pEntry = Array_ItemPtr (pProject->Materials.Items, i);
		assert (pEntry->Filename != NULL);

		sprintf (Buffer, "\"%s\",%d,\"%s\",%f,%f,%f,%f", 
			pEntry->Name, pEntry->Fmt, pEntry->Filename,
			pEntry->Color.r, pEntry->Color.g, pEntry->Color.b, pEntry->Color.a);
		if (jeVFile_Printf (FS, "%s\r\n", Buffer) == JE_FALSE)
		{
			goto Error;

		}
	}

	if (jeVFile_Printf (FS, "%s\r\n", EndMaterials_Key) == JE_FALSE)
	{
		goto Error;
	}

	return JE_TRUE;
Error:
	jeErrorLog_AddString (JE_ERR_FILEIO_WRITE, "Writing Materials section",NULL);
	return JE_FALSE;
}

static jeBoolean AProject_ParseMotion 
	(
	  char *Buffer,
	  char *Name,
	  ApjMotionFormat *Fmt,
	  char *Filename,
	  jeBoolean *OptFlag,
	  int *OptLevel,
	  char *BoneName
	)
{
	char *NameStr, *FilenameStr, *FmtStr, *OptFlagStr, *OptLevelStr, *BoneNameStr;

	// parse the items from the line
	if ((NameStr		= strtok (Buffer, ",")) == NULL)return JE_FALSE;
	if ((FmtStr			= strtok (NULL, ",")) == NULL)	return JE_FALSE;
	if ((FilenameStr	= strtok (NULL, ",")) == NULL)	return JE_FALSE;
	if ((OptFlagStr		= strtok (NULL, ",")) == NULL)	return JE_FALSE;
	if ((OptLevelStr	= strtok (NULL, ",")) == NULL)	return JE_FALSE;
	if ((BoneNameStr	= strtok (NULL, ",")) == NULL)	return JE_FALSE;

	// set the items
	strcpy (Name, NameStr);
	if ((*FmtStr < '1') || (*FmtStr > '3'))
	{
		return JE_FALSE;
	}

	*Fmt = (ApjMotionFormat)(*FmtStr - '0');

	if (AProject_UnquoteString (NameStr)	== JE_FALSE) return JE_FALSE;
	if (AProject_UnquoteString (FilenameStr)== JE_FALSE) return JE_FALSE;
	if (AProject_UnquoteString (BoneNameStr)== JE_FALSE) return JE_FALSE;

	*OptFlag = (*OptFlagStr == '0') ? JE_FALSE : JE_TRUE;

	if (isdigit (*OptLevelStr))
	{
		*OptLevel = *OptLevelStr - '0';
	}
	else
	{
		*OptLevel = 0;
	}
	strcpy (Name, NameStr);
	strcpy (Filename, FilenameStr);
	strcpy (BoneName, BoneNameStr);

	return JE_TRUE;
}

static jeBoolean AProject_LoadMotionsInfo (AProject *pProject, jeVFile *FS)
{
	for (;;)
	{
		char Buffer[1024];
		char *c;

		if (AProject_GetNonBlankLine (FS, Buffer, sizeof (Buffer)) != READ_SUCCESS)
		{
			jeErrorLog_AddString (JE_ERR_FILEIO_READ, "Loading Motions info",NULL);
			return JE_FALSE;
		}

		if (_strnicmp (Buffer, MotionsCount_Key, strlen (MotionsCount_Key)) == 0)
		{
			int Count = 0;

			c = &Buffer[strlen (MotionsCount_Key)];
			if (*c != '\0')
			{
				Count = atoi (c+1);
				if (Count < 0)
				{
					jeErrorLog_AddString (JE_ERR_FILEIO_FORMAT, "Negative Motions count",NULL);
					return JE_FALSE;
				}
			}
			// load and add each motion
			for (; Count > 0; --Count)
			{
				char Name[MAX_PATH];
				char Filename[MAX_PATH];
				char BoneName[MAX_PATH];
				int OptLevel;
				jeBoolean OptFlag;
				int Index;
				ApjMotionFormat Fmt;

				if (AProject_GetNonBlankLine (FS, Buffer, sizeof (Buffer)) != READ_SUCCESS)
				{
					jeErrorLog_AddString (JE_ERR_FILEIO_READ, "Loading Motions info",NULL);
					return JE_FALSE;
				}

				// parse the motion's parts
				if (AProject_ParseMotion (Buffer, Name, &Fmt, Filename, &OptFlag, &OptLevel, BoneName) == JE_FALSE)
				{
					jeErrorLog_AddString (JE_ERR_FILEIO_FORMAT, "Bad motion",NULL);
					return JE_FALSE;
				}
				// and then add the motion
				if (AProject_AddMotion (pProject, Name, Filename, Fmt, OptFlag, OptLevel, BoneName, &Index) == JE_FALSE)
				{
					return JE_FALSE;
				}
			}
		}
		else if (_strnicmp (Buffer, EndMotions_Key, strlen (EndMotions_Key)) == 0)
		{
			return JE_TRUE;
		}
		else
		{
			// bad entry
			jeErrorLog_AddString (JE_ERR_FILEIO_FORMAT, "Bad Motions section entry",NULL);
			return JE_FALSE;
		}
	}
}

// ugly, but it works...
static char AProject_MotionFormatToChar (ApjMotionFormat Fmt)
{
	return (char)(((int)Fmt) + '0');
}

static jeBoolean AProject_WriteMotionsInfo (const AProject *pProject, jeVFile *FS)
{
	int i;

	if ((jeVFile_Printf (FS, "%s\r\n", Motions_Key) == JE_FALSE) ||
		(jeVFile_Printf (FS, "%s %d\r\n", MotionsCount_Key, pProject->Motions.Count) == JE_FALSE))
	{
		goto Error;
	}

	for (i = 0; i < pProject->Motions.Count; ++i)
	{
		// format each motion's information and write it
		ApjMotionEntry *pEntry;
		char Buffer[1024];

		pEntry = Array_ItemPtr (pProject->Motions.Items, i);
		sprintf (Buffer, "\"%s\",%c,\"%s\",%c,%d,\"%s\"", 
			pEntry->Name, AProject_MotionFormatToChar (pEntry->Fmt), 
			pEntry->Filename, (pEntry->OptFlag ? '1' : '0'), pEntry->OptLevel, pEntry->Bone);
		if (jeVFile_Printf (FS, "%s\r\n", Buffer) == JE_FALSE)
		{
			goto Error;

		}
	}

	if (jeVFile_Printf (FS, "%s\r\n", EndMotions_Key) == JE_FALSE)
	{
		goto Error;
	}

	return JE_TRUE;
Error:
	jeErrorLog_AddString (JE_ERR_FILEIO_WRITE, "Writing Motions section",NULL);
	return JE_FALSE;
}


// Project file section loader function type
typedef jeBoolean (* ApjSectionLoader) (AProject *pProject, jeVFile *FS);

typedef struct
{
	const char *SectionName;	// section name to find
	ApjSectionLoader Load;		// function that loads this section
} ApjSectionDispatchEntry;


// Table of section names and loader functions.
// Used to scan for and load project file sections.
static const ApjSectionDispatchEntry ApjSectionDispatchTable[] =
{
	{Paths_Key,		AProject_LoadPathsInfo},
	{Output_Key,	AProject_LoadOutputInfo},
	{Body_Key,		AProject_LoadBodyInfo},
	{Materials_Key,	AProject_LoadMaterialsInfo},
	{Motions_Key,	AProject_LoadMotionsInfo}
};

static int ApjNumSections = sizeof (ApjSectionDispatchTable)/sizeof (ApjSectionDispatchEntry);

// Read a project from a file.
AProject *AProject_CreateFromFile (jeVFile *FS)
{
	AProject *pProject = NULL;
	char Buffer[1024];		// any line longer than this is an error
	jeBoolean NoErrors;

	assert (FS != NULL);

	// create empty project
	NoErrors = ((pProject = AProject_Create ("")) != NULL);

	// check file version information
	NoErrors = NoErrors && (AProject_CheckFileVersion (FS) != JE_FALSE);

	// Sections can be in any order
	while (NoErrors && (jeVFile_EOF (FS) == JE_FALSE))
	{
		int Section;

		// read a line
		ApjReadResult rslt = AProject_GetNonBlankLine (FS, Buffer, sizeof (Buffer));
		switch (rslt)
		{
			case READ_ERROR :
				jeErrorLog_AddString (JE_ERR_FILEIO_READ, "Loading project",NULL);
				NoErrors = JE_FALSE;
				break;

			case READ_EOF :
				break;

			case READ_SUCCESS :
			{
				// get the section name and process that section
				jeBoolean FoundIt;
				const ApjSectionDispatchEntry *pEntry = NULL;

				// determine which section, and go read that.
				for (FoundIt = JE_FALSE, Section = 0; (FoundIt == JE_FALSE) && (Section < ApjNumSections); ++Section)
				{
					pEntry = &ApjSectionDispatchTable[Section];

					FoundIt = (_strnicmp (Buffer, pEntry->SectionName, strlen (pEntry->SectionName)) == 0);
				}

				if (FoundIt)
				{
					NoErrors = pEntry->Load (pProject, FS);
				}
				else
				{
					// didn't find a good section name
					NoErrors = JE_FALSE;
					jeErrorLog_AddString (JE_ERR_FILEIO_FORMAT, "Expected section name",NULL);
				}
				break;
			}
		}
	}

	if (!NoErrors)
	{
		// some kind of error occurred.
		// Clean up and exit
		if (pProject != NULL)
		{
			AProject_Destroy (&pProject);
		}
	}

	return pProject;
}

AProject *AProject_CreateFromFilename (const char *Filename)
{
	jeVFile *FS;
	AProject *Project;

	FS = jeVFile_OpenNewSystem (NULL, JE_VFILE_TYPE_DOS, Filename, NULL, JE_VFILE_OPEN_READONLY);
	if (FS == NULL)
	{
		// unable to open file for reading
		return NULL;
	}

	Project = AProject_CreateFromFile (FS);
	jeVFile_Close (FS);

	return Project;
}


jeBoolean AProject_WriteToFile (const AProject *pProject, jeVFile *FS)
{
	if ((jeVFile_Printf (FS, AProject_VersionString, APJ_VERSION_MAJOR, APJ_VERSION_MINOR) == JE_FALSE) ||
		(jeVFile_Printf (FS, "\r\n\r\n") == JE_FALSE))
	{
		jeErrorLog_AddString (JE_ERR_FILEIO_WRITE, "Writing version string",NULL);
		return JE_FALSE;
	}

	if ((AProject_WritePathsInfo (pProject, FS) != JE_FALSE) &&
		(jeVFile_Printf (FS, "\r\n") != JE_FALSE) &&
		(AProject_WriteOutputInfo (pProject, FS) != JE_FALSE) &&
		(jeVFile_Printf (FS, "\r\n") != JE_FALSE) &&
	    (AProject_WriteBodyInfo (pProject, FS) != JE_FALSE) &&
		(jeVFile_Printf (FS, "\r\n") != JE_FALSE) &&
		(AProject_WriteMaterialsInfo (pProject, FS) != JE_FALSE) &&
		(jeVFile_Printf (FS, "\r\n") != JE_FALSE) &&
		(AProject_WriteMotionsInfo (pProject, FS) != JE_FALSE))
	{
		return JE_TRUE;
	}
	return JE_FALSE;
}

jeBoolean AProject_WriteToFilename (const AProject *pProject, const char *Filename)
{
	jeVFile *FS;
	jeBoolean rslt;

	FS = jeVFile_OpenNewSystem (NULL, JE_VFILE_TYPE_DOS, Filename, NULL, JE_VFILE_OPEN_CREATE);
	if (FS == NULL)
	{
		// unable to open file for writing
		jeErrorLog_AddString (JE_ERR_FILEIO_WRITE, "Opening file",NULL);
		return JE_FALSE;
	}

	rslt = AProject_WriteToFile (pProject, FS);

	jeVFile_Close (FS);

	return rslt;
}

// Paths section
jeBoolean AProject_GetForceRelativePaths (const AProject *pProject)
{
	return pProject->Paths.ForceRelative;
}

jeBoolean AProject_SetForceRelativePaths (AProject *pProject, const jeBoolean Flag)
{
	pProject->Paths.ForceRelative = Flag;
	return JE_TRUE;
}


const char *AProject_GetMaterialsPath (const AProject *pProject)
{
	return pProject->Paths.Materials;
}

jeBoolean AProject_SetMaterialsPath (AProject *pProject, const char *Path)
{
	if (AProject_SetString (&pProject->Paths.Materials, Path) == JE_FALSE)
	{
		jeErrorLog_AddString (JE_ERR_MEMORY_RESOURCE, "Setting materials path",NULL);
		return JE_FALSE;
	}
	return JE_TRUE;
}

const char *AProject_GetObjPath (const AProject *pProject)
{
	return pProject->Paths.TempFiles;
}

jeBoolean AProject_SetObjPath (AProject *pProject, const char *Path)
{
	if (AProject_SetString (&pProject->Paths.TempFiles, Path) == JE_FALSE)
	{
		jeErrorLog_AddString (JE_ERR_MEMORY_RESOURCE, "Setting temp files path",NULL);
		return JE_FALSE;
	}
	return JE_TRUE;
}


const char *AProject_GetOutputFilename (const AProject *pProject)
{
	return pProject->Output.Filename;
}

jeBoolean AProject_SetOutputFilename (AProject *pProject, const char *Filename)
{
	if (AProject_SetString (&pProject->Output.Filename, Filename) == JE_FALSE)
	{
		jeErrorLog_AddString (JE_ERR_MEMORY_RESOURCE, "Setting output filename",NULL);
		return JE_FALSE;
	}
	return JE_TRUE;
}

ApjOutputFormat AProject_GetOutputFormat (const AProject *pProject)
{
	return pProject->Output.Fmt;
}

jeBoolean AProject_SetOutputFormat (AProject *pProject, const ApjOutputFormat Fmt)
{
	assert ((Fmt == ApjOutput_Text) || (Fmt == ApjOutput_Binary));

	pProject->Output.Fmt = Fmt;
	return JE_TRUE;
}

const char *AProject_GetBodyFilename (const AProject *pProject)
{
	return pProject->Body.Filename;
}

jeBoolean AProject_SetBodyFilename (AProject *pProject, const char *Filename)
{
	if (AProject_SetString (&pProject->Body.Filename, Filename) == JE_FALSE)
	{
		jeErrorLog_AddString (JE_ERR_MEMORY_RESOURCE, "Setting body filename",NULL);
		return JE_FALSE;
	}
	return JE_TRUE;
}


ApjBodyFormat AProject_GetBodyFormat (const AProject *pProject)
{
	return pProject->Body.Fmt;
}

jeBoolean AProject_SetBodyFormat (AProject *pProject, ApjBodyFormat Fmt)
{
	assert ((Fmt >= ApjBody_Invalid) && (Fmt <= ApjBody_Act));

	pProject->Body.Fmt = Fmt;
	return JE_TRUE;
}

int AProject_GetMaterialsCount (const AProject *pProject)
{
	return pProject->Materials.Count;
}

static void AProject_FreeMaterialInfo (ApjMaterialEntry *pEntry)
{
	if (pEntry->Name != NULL) jeRam_Free (pEntry->Name);
	if (pEntry->Filename != NULL) jeRam_Free (pEntry->Filename);
}

jeBoolean AProject_AddMaterial
	(
	  AProject *pProject,
	  const char *MaterialName,
	  const ApjMaterialFormat Fmt,
	  const char *TextureFilename,
	  const float Red, const float Green, const float Blue, const float Alpha,
	  int *pIndex		// returned index
	)
{
	ApjMaterialEntry *pEntry;
	int ArraySize;

	assert ((Fmt == ApjMaterial_Color) || (Fmt == ApjMaterial_Texture));

	ArraySize = Array_GetSize (pProject->Materials.Items);
	if (pProject->Materials.Count == ArraySize)
	{
		// array is full, have to extend it
		int NewSize;
		
		NewSize = Array_Resize (pProject->Materials.Items, 2*ArraySize);
		if (NewSize <= ArraySize)
		{
			// couldn't resize
			jeErrorLog_AddString (JE_ERR_MEMORY_RESOURCE, "Adding material",NULL);
			return JE_FALSE;
		}
	}
	pEntry = Array_ItemPtr (pProject->Materials.Items, pProject->Materials.Count);
	pEntry->Name = NULL;
	pEntry->Filename = NULL;

	if (((pEntry->Name = Util_Strdup (MaterialName)) == NULL) ||
		((pEntry->Filename = Util_Strdup (TextureFilename)) == NULL))
	{
		jeErrorLog_AddString (JE_ERR_MEMORY_RESOURCE, "Adding material",NULL);
		AProject_FreeMaterialInfo (pEntry);
		return JE_FALSE;
	}
	pEntry->Fmt = Fmt;
	pEntry->Color.r = Red;
	pEntry->Color.g = Green;
	pEntry->Color.b = Blue;
	pEntry->Color.a = Alpha;

	*pIndex = (pProject->Materials.Count)++;
	return JE_TRUE;
}

jeBoolean AProject_RemoveMaterial (AProject *pProject, const int Index)
{
	ApjMaterialEntry *pEntry;

	assert (Index < pProject->Materials.Count);

	pEntry = Array_ItemPtr (pProject->Materials.Items, Index);
	AProject_FreeMaterialInfo (pEntry);

	Array_DeleteAt (pProject->Materials.Items, Index);
	--(pProject->Materials.Count);

	return JE_TRUE;
}

// returns -1 if not found
int AProject_GetMaterialIndex (const AProject *pProject, const char *MaterialName)
{
	int Item;

	for (Item = 0; Item < pProject->Materials.Count; ++Item)
	{
		ApjMaterialEntry *pEntry = Array_ItemPtr (pProject->Materials.Items, Item);
		if (stricmp (pEntry->Name, MaterialName) == 0)
		{
			return Item;
		}
	}

	return -1;
}


ApjMaterialFormat AProject_GetMaterialFormat (const AProject *pProject, const int Index)
{
	ApjMaterialEntry *pEntry;

	assert (Index < pProject->Materials.Count);

	pEntry = Array_ItemPtr (pProject->Materials.Items, Index);
	return pEntry->Fmt;
}

jeBoolean AProject_SetMaterialFormat (AProject *pProject, const int Index, const ApjMaterialFormat Fmt)
{
	ApjMaterialEntry *pEntry;

	assert (Index < pProject->Materials.Count);
	assert ((Fmt == ApjMaterial_Color) || (Fmt == ApjMaterial_Texture));

	pEntry = Array_ItemPtr (pProject->Materials.Items, Index);
	pEntry->Fmt = Fmt;

	return JE_TRUE;
}

const char *AProject_GetMaterialName (const AProject *pProject, const int Index)
{
	ApjMaterialEntry *pEntry;

	assert (Index < pProject->Materials.Count);

	pEntry = Array_ItemPtr (pProject->Materials.Items, Index);
	return pEntry->Name;
}

jeBoolean AProject_SetMaterialName (AProject *pProject, const int Index, const char *MaterialName)
{
	ApjMaterialEntry *pEntry;

	assert (Index < pProject->Materials.Count);

	pEntry = Array_ItemPtr (pProject->Materials.Items, Index);
	if (AProject_SetString (&pEntry->Name, MaterialName) == JE_FALSE)
	{
		jeErrorLog_AddString (JE_ERR_MEMORY_RESOURCE, "Setting material name",NULL);
		return JE_FALSE;
	}
	assert (pEntry->Name != NULL);
	return JE_TRUE;
}

const char *AProject_GetMaterialTextureFilename (const AProject *pProject, const int Index)
{
	ApjMaterialEntry *pEntry;

	assert (Index < pProject->Materials.Count);

	pEntry = Array_ItemPtr (pProject->Materials.Items, Index);
	return pEntry->Filename;
}

jeBoolean AProject_SetMaterialTextureFilename (AProject *pProject, const int Index, const char *TextureFilename)
{
	ApjMaterialEntry *pEntry;

	assert (Index < pProject->Materials.Count);
	assert (TextureFilename != NULL);

	pEntry = Array_ItemPtr (pProject->Materials.Items, Index);
	if (AProject_SetString (&pEntry->Filename, TextureFilename) == JE_FALSE)
	{
		jeErrorLog_AddString (JE_ERR_MEMORY_RESOURCE, "Setting material filename",NULL);
		return JE_FALSE;
	}

	return JE_TRUE;
}


JE_RGBA AProject_GetMaterialTextureColor (const AProject *pProject, const int Index)
{
	ApjMaterialEntry *pEntry;

	assert (Index < pProject->Materials.Count);

	pEntry = Array_ItemPtr (pProject->Materials.Items, Index);
	return pEntry->Color;
}

jeBoolean AProject_SetMaterialTextureColor (AProject *pProject, const int Index, 
	const float Red, const float Green, const float Blue, const float Alpha)
{
	ApjMaterialEntry *pEntry;

	assert (Index < pProject->Materials.Count);

	pEntry = Array_ItemPtr (pProject->Materials.Items, Index);
	pEntry->Color.r = Red;
	pEntry->Color.g = Green;
	pEntry->Color.b = Blue;
	pEntry->Color.a = Alpha;

	return JE_TRUE;
}


// Motions section
int AProject_GetMotionsCount (const AProject *pProject)
{
	return pProject->Motions.Count;
}

static void AProject_FreeMotionInfo (ApjMotionEntry *pEntry)
{
	if (pEntry->Name != NULL) jeRam_Free (pEntry->Name);
	if (pEntry->Filename != NULL) jeRam_Free (pEntry->Filename);
	if (pEntry->Bone != NULL) jeRam_Free (pEntry->Bone);
}

jeBoolean AProject_AddMotion
	(
	  AProject *pProject,
	  const char *MotionName,
	  const char *Filename,
	  const ApjMotionFormat Fmt,
	  const jeBoolean OptFlag,
	  const int OptLevel,
	  const char *BoneName,
	  int *pIndex	// returned index
	)
{
	ApjMotionEntry *pEntry;
	int ArraySize;

	assert ((OptLevel >= 0) && (OptLevel <= 9));
	assert ((Fmt > ApjMotion_Invalid) && (Fmt < ApjMotion_TypeCount));

	ArraySize = Array_GetSize (pProject->Motions.Items);
	if (pProject->Motions.Count == ArraySize)
	{
		// array is full, have to extend it
		int NewSize;
		
		NewSize = Array_Resize (pProject->Motions.Items, 2*ArraySize);
		if (NewSize <= ArraySize)
		{
			// couldn't resize
			jeErrorLog_AddString (JE_ERR_MEMORY_RESOURCE, "Adding Motion",NULL);
			return JE_FALSE;
		}
	}
	pEntry = Array_ItemPtr (pProject->Motions.Items, pProject->Motions.Count);
	pEntry->Name = NULL;
	pEntry->Filename = NULL;
	pEntry->Bone = NULL;
	pEntry->Fmt = Fmt;

	if (((pEntry->Name = Util_Strdup (MotionName)) == NULL) ||
		((pEntry->Filename = Util_Strdup (Filename)) == NULL) ||
		((pEntry->Bone = Util_Strdup (BoneName)) == NULL))
	{
		jeErrorLog_AddString (JE_ERR_MEMORY_RESOURCE, "Adding Motion",NULL);
		AProject_FreeMotionInfo (pEntry);
		return JE_FALSE;
	}
	pEntry->OptFlag = OptFlag;
	pEntry->OptLevel = OptLevel;

	*pIndex = (pProject->Motions.Count)++;
	return JE_TRUE;
}

jeBoolean AProject_RemoveMotion (AProject *pProject, const int Index)
{
	ApjMotionEntry *pEntry;

	assert (Index < pProject->Motions.Count);

	pEntry = Array_ItemPtr (pProject->Motions.Items, Index);
	AProject_FreeMotionInfo (pEntry);

	Array_DeleteAt (pProject->Motions.Items, Index);
	--(pProject->Motions.Count);

	return JE_TRUE;
}

int AProject_GetMotionIndex (const AProject *pProject, const char *MotionName)
{
	int Item;

	for (Item = 0; Item < pProject->Motions.Count; ++Item)
	{
		ApjMotionEntry *pEntry = Array_ItemPtr (pProject->Motions.Items, Item);
		if (strcmp (pEntry->Name, MotionName) == 0)
		{
			return Item;
		}
	}

	return -1;
}


ApjMotionFormat AProject_GetMotionFormat (const AProject *pProject, const int Index)
{
	ApjMotionEntry *pEntry;

	assert (Index < pProject->Motions.Count);

	pEntry = Array_ItemPtr (pProject->Motions.Items, Index);
	return pEntry->Fmt;
}

jeBoolean AProject_SetMotionFormat (AProject *pProject, const int Index, const ApjMotionFormat Fmt)
{
	ApjMotionEntry *pEntry;

	assert (Index < pProject->Motions.Count);
	assert ((Fmt > ApjMotion_Invalid) && (Fmt < ApjMotion_TypeCount));

	pEntry = Array_ItemPtr (pProject->Motions.Items, Index);
	pEntry->Fmt = Fmt;

	return JE_TRUE;
}

const char *AProject_GetMotionName (const AProject *pProject, const int Index)
{
	ApjMotionEntry *pEntry;

	assert (Index < pProject->Motions.Count);

	pEntry = Array_ItemPtr (pProject->Motions.Items, Index);
	return pEntry->Name;
}

jeBoolean AProject_SetMotionName (AProject *pProject, const int Index, const char *MotionName)
{
	ApjMotionEntry *pEntry;

	assert (Index < pProject->Motions.Count);

	pEntry = Array_ItemPtr (pProject->Motions.Items, Index);
	if (AProject_SetString (&pEntry->Name, MotionName) == JE_FALSE)
	{
		jeErrorLog_AddString (JE_ERR_MEMORY_RESOURCE, "Setting Motion name",NULL);
		return JE_FALSE;
	}

	return JE_TRUE;
}

const char *AProject_GetMotionFilename (const AProject *pProject, const int Index)
{
	ApjMotionEntry *pEntry;

	assert (Index < pProject->Motions.Count);

	pEntry = Array_ItemPtr (pProject->Motions.Items, Index);
	return pEntry->Filename;
}

jeBoolean AProject_SetMotionFilename (AProject *pProject, const int Index, const char *Filename)
{
	ApjMotionEntry *pEntry;

	assert (Index < pProject->Motions.Count);

	pEntry = Array_ItemPtr (pProject->Motions.Items, Index);
	if (AProject_SetString (&pEntry->Filename, Filename) == JE_FALSE)
	{
		jeErrorLog_AddString (JE_ERR_MEMORY_RESOURCE, "Setting Motion filename",NULL);
		return JE_FALSE;
	}

	return JE_TRUE;
}

jeBoolean AProject_GetMotionOptimizationFlag (const AProject *pProject, const int Index)
{
	ApjMotionEntry *pEntry;

	assert (Index < pProject->Motions.Count);

	pEntry = Array_ItemPtr (pProject->Motions.Items, Index);
	return pEntry->OptFlag;
}

jeBoolean AProject_SetMotionOptimizationFlag (AProject *pProject, const int Index, const jeBoolean Flag)
{
	ApjMotionEntry *pEntry;

	assert (Index < pProject->Motions.Count);

	pEntry = Array_ItemPtr (pProject->Motions.Items, Index);

	pEntry->OptFlag = Flag;
	return JE_TRUE;
}

int AProject_GetMotionOptimizationLevel (const AProject *pProject, const int Index)
{
	ApjMotionEntry *pEntry;

	assert (Index < pProject->Motions.Count);

	pEntry = Array_ItemPtr (pProject->Motions.Items, Index);
	return pEntry->OptLevel;
}

jeBoolean AProject_SetMotionOptimizationLevel (AProject *pProject, const int Index, const int OptLevel)
{
	ApjMotionEntry *pEntry;

	assert (Index < pProject->Motions.Count);
	assert ((OptLevel >= 0) && (OptLevel <= 9));

	pEntry = Array_ItemPtr (pProject->Motions.Items, Index);
	pEntry->OptLevel = OptLevel;

	return JE_TRUE;
}

const char *AProject_GetMotionBone (const AProject *pProject, const int Index)
{
	ApjMotionEntry *pEntry;

	assert (Index < pProject->Motions.Count);

	pEntry = Array_ItemPtr (pProject->Motions.Items, Index);
	return pEntry->Bone;
}

jeBoolean AProject_SetMotionBone (AProject *pProject, const int Index, const char *BoneName)
{
	ApjMotionEntry *pEntry;

	assert (Index < pProject->Motions.Count);

	pEntry = Array_ItemPtr (pProject->Motions.Items, Index);
	if (AProject_SetString (&pEntry->Bone, BoneName) == JE_FALSE)
	{
		jeErrorLog_AddString (JE_ERR_MEMORY_RESOURCE, "Setting Motion bone",NULL);
		return JE_FALSE;
	}

	return JE_TRUE;
}
