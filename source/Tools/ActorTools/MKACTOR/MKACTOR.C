/****************************************************************************************/
/*  MKACTOR.C                                                                           */
/*                                                                                      */
/*  Author: Stephen Balkum	                                                            */
/*  Description: Actor construction from body and motions.								*/
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "actor.h"
#include "ram.h"
#include "strblock.h"
#include "mkactor.h"

typedef struct MkActor_Options
{
	MK_Boolean ConcatenateActor;
	char ActorFile[_MAX_PATH];
	char BodyFile[_MAX_PATH];
	jeStrBlock* pMotionFileBlock;
} MkActor_Options;

// Use this to initialize the default settings
const static MkActor_Options DefaultOptions = 
{
	MK_FALSE,
	"",
	"",
	NULL,
};

#define BAILOUT {MkUtil_AdjustReturnCode(&retValue, RETURN_ERROR);if (pActorDef!=NULL) {jeActor_DefDestroy(&pActorDef);} return retValue;}


ReturnCode MkActor_DoMake(MkActor_Options* options,MkUtil_Printf Printf)
{
	ReturnCode retValue = RETURN_SUCCESS;
	jeVFile *VF;
	//jeActor* pActor = NULL;
	jeActor_Def* pActorDef = NULL;

	// Actor file must be specified
	if(options->ActorFile[0] == 0)
		{
			Printf("ERROR: Must specify an actor file\n");
			BAILOUT;
		}

	// Create the Actor from file or from scratch
	if(options->ConcatenateActor != MK_FALSE)
		{
			VF = jeVFile_OpenNewSystem(NULL,JE_VFILE_TYPE_DOS,options->ActorFile,NULL,JE_VFILE_OPEN_READONLY);
			if(VF == NULL)
				{
					Printf("ERROR: Could not open '%s' to concatenate\n", options->ActorFile);
					BAILOUT;
				}
			pActorDef = jeActor_DefCreateFromFile(VF);
			jeVFile_Close(VF);
			if(pActorDef == NULL)
				{
					Printf("ERROR: Could not create actor from file '%s'\n", options->ActorFile);
					BAILOUT;
				}
		}

	if(pActorDef == NULL)
		{
			pActorDef = jeActor_DefCreate();
		}
	
	if(pActorDef == NULL)
		{
			Printf("ERROR: Could not create Actor\n");
			BAILOUT;
		}

	// Read the body file
	if(options->BodyFile[0] != 0)
		{
			jeBody* pBody = NULL;

			VF = jeVFile_OpenNewSystem(NULL,JE_VFILE_TYPE_DOS,options->BodyFile,NULL,JE_VFILE_OPEN_READONLY);
			if(VF == NULL)
			{
				Printf("ERROR: Could not open '%s' body file\n", options->BodyFile);
				BAILOUT;
			}
			else
			{
				pBody = jeBody_CreateFromFile(VF);
				jeVFile_Close(VF);
				if(pBody == NULL)
				{
					Printf("ERROR: Could not create body from file '%s'\n", options->BodyFile);
					BAILOUT;
				}
			}

			if(jeActor_GetBody(pActorDef) != NULL)
			{
				Printf("ERROR: An existing body is being replaced\n");
				BAILOUT;
			}

			jeActor_SetBody(pActorDef, pBody);
		}

	// Read the motions
	if(jeStrBlock_GetCount(options->pMotionFileBlock) > 0)
		{
			int i, Count, Index;
			const char* filename;
			jeMotion* pMotion;

			Count = jeStrBlock_GetCount(options->pMotionFileBlock);
			for(i=0;i<Count;i++)
			{
				if (MkUtil_Interrupt())
					{
						Printf("Interrupted\n");
						BAILOUT;
					}
		
				filename = jeStrBlock_GetString(options->pMotionFileBlock, i);
				VF = jeVFile_OpenNewSystem(NULL,JE_VFILE_TYPE_DOS,filename,NULL,JE_VFILE_OPEN_READONLY);
				if(VF == NULL)
				{
					Printf("ERROR: Could not open '%s' motion file\n", filename);
					BAILOUT;
				}
				else
				{
					pMotion = jeMotion_CreateFromFile(VF);
					jeVFile_Close(VF);
					if(pMotion == NULL)
					{
						Printf("ERROR: Could not create motion from file '%s'\n", filename);
						BAILOUT;
					}
					else
					{
						if(jeActor_AddMotion(pActorDef, pMotion, &Index) == MK_FALSE)
							{
								Printf("ERROR: Motion file '%s' was not added\n", filename);
								jeMotion_Destroy(&pMotion);
								BAILOUT;
							}
					}
				}
			}
		}

	// Rename any existing actor file
	{
		char bakname[_MAX_PATH];

		strcpy(bakname, options->ActorFile);
		strcat(bakname, ".bak");
		remove(bakname);
		rename(options->ActorFile, bakname);
	}

	// Write the actor
	VF = jeVFile_OpenNewSystem(NULL,JE_VFILE_TYPE_DOS,options->ActorFile,NULL,JE_VFILE_OPEN_CREATE);
	if(VF == NULL)
	{
		Printf("ERROR: Could not create '%s' actor file\n", options->ActorFile);
		unlink(options->ActorFile);
		BAILOUT;
	}
	else
	{
		if(jeActor_DefWriteToFile(pActorDef, VF) == JE_FALSE)
		{
			Printf("ERROR: Actor file '%s' was not written correctly\n", options->ActorFile);
			unlink(options->ActorFile);			
			BAILOUT;
		}
		else
		{
			if (jeVFile_Close(VF) == JE_FALSE)
				{
					Printf("ERROR: Actor file '%s' was not written correctly\n", options->ActorFile);
					unlink(options->ActorFile);			
					BAILOUT;
				}
			else
				{
					Printf("SUCCESS: Actor file '%s' written successfully\n", options->ActorFile);
				}
		}
	}

	jeActor_DefDestroy(&pActorDef); // cleans up all motions and body
	
	return retValue;
}

void MkActor_OutputUsage(MkUtil_Printf Printf)
{
	//COLS: 0         1         2         3         4         5         6         7       | 8
	Printf("\n");
	Printf("Builds an actor library from zero or more motions and an optional body.\n");
	Printf("\n");
	Printf("MKACTOR [options] [/C] [/A<actorfile>] [/B<bodyfile>]\n");
	Printf("        [/M<motionfile1> /M<motionfile2> ...]\n");
	Printf("\n");
	Printf("/C:             Concatenate with existing actor file.\n");
	Printf("/A<actorfile>   Specifies actor file.\n");
	Printf("/B<bodyfile>    Specifies body file.\n");
	Printf("/M<motionfile>  Specifies motion file.\n");
	Printf("\n");
	Printf("Any existing actor file will be renamed to actorfile.bak\n");
}

MkActor_Options* MkActor_OptionsCreate()
{
	MkActor_Options* pOptions;

	pOptions = JE_RAM_ALLOCATE_STRUCT(MkActor_Options);
	if(pOptions != NULL)
	{
		*pOptions = DefaultOptions;

		pOptions->pMotionFileBlock = jeStrBlock_Create();
		if(pOptions->pMotionFileBlock == NULL)
		{
			jeRam_Free(pOptions);
		}
	}

	return pOptions;
}

void MkActor_OptionsDestroy(MkActor_Options** ppOptions)
{
	MkActor_Options* p;

	assert(ppOptions != NULL);
	assert(*ppOptions != NULL);

	p = *ppOptions;

	jeStrBlock_Destroy(&p->pMotionFileBlock);

	jeRam_Free(*ppOptions);

	*ppOptions = NULL;
}

ReturnCode MkActor_ParseOptionString(MkActor_Options* options, const char* string, MK_Boolean InScript,MkUtil_Printf Printf)
{
	ReturnCode retValue = RETURN_SUCCESS;
	int Index;

	assert(options != NULL);
	assert(string != NULL);

#define NO_FILENAME_WARNING Printf("WARNING: '%s' specified with no filename\n", string)

	if( (string[0] == '-') || (string[0] == '/') )
	{
		switch(string[1])
		{
		case 'c':
		case 'C':
			options->ConcatenateActor = MK_TRUE;
			break;

		case 'a':
		case 'A':
			if(string[2] == 0)
			{
				NO_FILENAME_WARNING;
				retValue = RETURN_WARNING;
			}
			else
			{
				if( (InScript != MK_FALSE) && (options->ActorFile[0] != 0) )
				{
					Printf("WARNING: Actor filename in script ignored\n");
					retValue = RETURN_WARNING;
				}
				else
				{
					strcpy(options->ActorFile, string + 2);
				}
			}
			break;

		case 'b':
		case 'B':
			if(string[2] == 0)
			{
				NO_FILENAME_WARNING;
				retValue = RETURN_WARNING;
			}
			else
			{
				if( (InScript != MK_FALSE) && (options->BodyFile[0] != 0) )
				{
					Printf("WARNING: Body filename in script ignored\n");
					retValue = RETURN_WARNING;
				}
				else
				{
					strcpy(options->BodyFile, string + 2);
				}
			}
			break;

		case 'm':
		case 'M':
			if(string[2] == 0)
			{
				NO_FILENAME_WARNING;
				retValue = RETURN_WARNING;
			}
			else
			{
				if(jeStrBlock_FindString(options->pMotionFileBlock, string + 2, &Index) == MK_FALSE)
				{
					if(jeStrBlock_Append(&options->pMotionFileBlock, string + 2) == MK_FALSE)
					{
						Printf("ERROR: Could not add '%s' motion file to motion file string block\n", string + 2);
						retValue = RETURN_ERROR;
					}
				}
				else
				{
					Printf("WARNING: Duplicate motion file ignored\n");
					retValue = RETURN_WARNING;
				}
			}
			break;

		default:
			retValue = RETURN_NOACTION;
		}
	}

	return retValue;
}
