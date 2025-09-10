/****************************************************************************************/
/*  MAKE.C																				*/
/*                                                                                      */
/*  Author: Mike Sandige	                                                            */
/*  Description: Actor make process main module.										*/
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
/* notes:
	would like to out-of-date things whose build options have changed 
	would like to out-of-date the actor if any options have changed
*/


#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <io.h>
#include <time.h>
#include <string.h>
#include <direct.h>		// _mkdir
#include "jet.h"
#include "actor.h"
#include "body.h"
#include "motion.h"
#include "make.h"
#include "AProject.h"
#include "MXScript.h"
#include "ram.h"
#include "mkbody.h"
#include "mkmotion.h"
#include "mopshell.h"
#include "mkactor.h"
#include "filepath.h"

#pragma message ("Need a force-build flag for each target")


//jeBoolean Make_CleanFlag = JE_FALSE;

#define BIG (2048)

#if 0
static void Make_PrintfCallback(const char *Fmt, ...)
{
	assert(Fmt);
 
	vprintf( Fmt, (char *)&Fmt + sizeof( Fmt ) );

}
#endif

static jeBoolean Make_GlobalInterruptFlag = JE_FALSE;

void Make_SetInterruptFlag (jeBoolean State)
{
	Make_GlobalInterruptFlag = State;
}

int MkUtil_Interrupt(void)
{
	return Make_GlobalInterruptFlag;
}


void MkUtil_AdjustReturnCode(ReturnCode* pToAdjust, ReturnCode AdjustBy)
{
	assert(pToAdjust != NULL);

	switch(AdjustBy)
	{
	case RETURN_SUCCESS:
		// do nothing
		break;

	case RETURN_WARNING:
		if(*pToAdjust == RETURN_SUCCESS)
			*pToAdjust = RETURN_WARNING;
		break;

	case RETURN_ERROR:
		if(*pToAdjust != RETURN_USAGE)
			*pToAdjust = RETURN_ERROR;
		break;

	case RETURN_USAGE:
		*pToAdjust = RETURN_USAGE;
		break;

	default:
		assert(0);
	}
}


jeBoolean Make_IsTargetOutOfDate( const char *TargetFileName, 
								  const char *SourceFileName, 
								  jeBoolean  *OutOfDate,
								  MkUtil_Printf Printf)
{
	long Handle;
	struct _finddata_t SourceData,TargetData;

	assert( Printf         != NULL );
	assert( OutOfDate      != NULL );
	assert( SourceFileName != NULL );
	assert( TargetFileName != NULL );



	Handle = (long)_findfirst( SourceFileName, &SourceData );
	if (Handle == -1)
		{
//			if (Make_CleanFlag != JE_FALSE)				
//				return JE_TRUE;  // always return out of date if in Clean mode;
			Printf("Error: Source file '%s' not found\n",SourceFileName);
			return JE_FALSE;		
		}
		
	_findclose(Handle);
	
	Handle = (long)_findfirst( TargetFileName, &TargetData );
	if (Handle == -1)
		{
//			if (Make_CleanFlag != JE_FALSE)				
//				*OutOfDate = JE_TRUE; // always return out of date if in Clean mode
//			else
				*OutOfDate = JE_TRUE; // is out of date if target isn't there.
			Printf("'%s' out of date.  (Doesn't exist)\n",TargetFileName,SourceFileName);
			return JE_TRUE;		
		}
	_findclose(Handle);

	#if 0
	if (Make_CleanFlag != JE_FALSE)				
		{									
			
			if (unlink(TargetFileName)!=0)
				{
					Printf("Error: Unable to clean '%s'\n",TargetFileName);
					return JE_FALSE;
				}
											
			*OutOfDate = JE_FALSE;		
			return JE_TRUE;				
		}
	#endif

	if (SourceData.time_write >= TargetData.time_write)
		{
			*OutOfDate = JE_TRUE;
			Printf("'%s' out of date.  (Older than '%s')\n",TargetFileName,SourceFileName);
			return JE_TRUE;
		}

	*OutOfDate = JE_FALSE;
	return JE_TRUE;
}

void Make_TargetFileName( char *TargetFileName, const char *SourceFileName, 
					const char *ObjDir, const char *NewExt )
{
	char drive[BIG];
	char dir[BIG];
	char fname[BIG];
	char ext[BIG];
	assert( SourceFileName );
	assert( ObjDir );
	assert( NewExt );
	assert( TargetFileName );
	
	_splitpath( SourceFileName, drive,dir,fname,ext);
	_makepath( TargetFileName, "",ObjDir,fname,NewExt );
}


jeBoolean Make_CopyBodyFile( const char *TargetFileName, 
						     const char *SourceFileName,
							 MkUtil_Printf Printf)
{
	jeVFile *VF;
	jeBody* pBody = NULL;
	jeBoolean Worked;

	assert( Printf         != NULL );
	assert( SourceFileName != NULL );
	assert( TargetFileName != NULL );
	Printf("\tCopying Body file '%s' into '%s'\n",SourceFileName,TargetFileName);

	VF = jeVFile_OpenNewSystem(NULL,JE_VFILE_TYPE_DOS,SourceFileName,NULL,JE_VFILE_OPEN_READONLY);
	if(VF == NULL)
	{
		Printf("ERROR: Could not open source body file '%s'\n", SourceFileName);
		return JE_FALSE;
	}
	
	pBody = jeBody_CreateFromFile(VF);
	jeVFile_Close(VF);
	if(pBody == NULL)
		{
			Printf("ERROR: Failed to load source body from file '%s'\n", SourceFileName);
			jeVFile_Close(VF);
			return JE_FALSE;
		}

	VF = jeVFile_OpenNewSystem(NULL,JE_VFILE_TYPE_DOS,TargetFileName,NULL,JE_VFILE_OPEN_CREATE);
	if(VF == NULL)
	{
		Printf("ERROR: Could not open target body file '%s'\n", TargetFileName);
		jeBody_Destroy(&pBody);
		return JE_FALSE;
	}

	Worked = jeBody_WriteToFile(pBody,VF);
	if (jeVFile_Close(VF)==JE_FALSE)
		Worked = JE_FALSE;
	jeBody_Destroy(&pBody);

	if (Worked == JE_FALSE)
		{
			Printf("Error:  Failed to write target body to file '%s'",TargetFileName);
			return JE_FALSE;
		}
	return JE_TRUE;
}

jeBoolean Make_CopyMotionFile( const char *TargetFileName, 
						     const char *SourceFileName,
							 MkUtil_Printf Printf)
{
	jeVFile *VF;
	jeMotion* pMotion = NULL;
	jeBoolean Worked;

	assert( Printf         != NULL );
	assert( SourceFileName != NULL );
	assert( TargetFileName != NULL );

	Printf("\tCopying Motion file '%s' into '%s'\n",SourceFileName,TargetFileName);

	VF = jeVFile_OpenNewSystem(NULL,JE_VFILE_TYPE_DOS,SourceFileName,NULL,JE_VFILE_OPEN_READONLY);
	if(VF == NULL)
	{
		Printf("ERROR: Could not open source motion file '%s'\n", SourceFileName);
		return JE_FALSE;
	}
	
	pMotion = jeMotion_CreateFromFile(VF);
	jeVFile_Close(VF);
	if(pMotion == NULL)
		{
			Printf("ERROR: Failed to load source motion file '%s'\n", SourceFileName);
			jeVFile_Close(VF);
			return JE_FALSE;
		}

	VF = jeVFile_OpenNewSystem(NULL,JE_VFILE_TYPE_DOS,TargetFileName,NULL,JE_VFILE_OPEN_CREATE);
	if(VF == NULL)
	{
		Printf("ERROR: Could not open target motion file '%s'\n", TargetFileName);
		jeMotion_Destroy(&pMotion);
		return JE_FALSE;
	}

	Worked = jeMotion_WriteToFile(pMotion,VF);
	if (jeVFile_Close(VF)==JE_FALSE)
		Worked = JE_FALSE;
	jeMotion_Destroy(&pMotion);

	if (Worked == JE_FALSE)
		{
			Printf("Error:  Failed to write target motion file '%s'",TargetFileName);
			return JE_FALSE;
		}
	return JE_TRUE;
}


jeBoolean Make_Body_NFO_OutOfDate( AProject *Prj, 
			jeBoolean *OutOfDate,
			MkUtil_Printf Printf)
{
	ApjBodyFormat BodyFmt;
	const char *SourceName;
	const char *ObjDir;
	char TargetName[BIG];
	
	assert( Prj       != NULL );
	assert( OutOfDate != NULL );
	assert( Printf    != NULL );
	
	*OutOfDate = JE_FALSE;

	BodyFmt = AProject_GetBodyFormat(Prj);
	if (BodyFmt != ApjBody_Max)
		return JE_TRUE;
	
	ObjDir = AProject_GetObjPath( Prj );
	if (ObjDir == NULL)
		{
			Printf("Error:  Unable to get temporary path\n");
			return JE_FALSE;
		}
	
	SourceName = AProject_GetBodyFilename (Prj);
	if (SourceName == NULL)
		{
			Printf("Error: Can't get source file name for body\n");
			return JE_FALSE;
		} 
	Make_TargetFileName( TargetName, SourceName, ObjDir,"NFO" );
	if (Make_IsTargetOutOfDate(TargetName, SourceName, OutOfDate,Printf ) == JE_FALSE)
		return JE_FALSE;
	
	return JE_TRUE;
}


jeBoolean Make_Body_BDY_OutOfDate(AProject *Prj, 
							 jeBoolean *OutOfDate,
							 MkUtil_Printf Printf)
{
	ApjBodyFormat Fmt;
	char TargetName[BIG];
	const char *SourceName;
	const char *ObjDir;

	assert( Prj       != NULL );
	assert( OutOfDate != NULL );
	assert( Printf    != NULL );
	
	
	Fmt = AProject_GetBodyFormat(Prj);
	switch (Fmt)
		{
			case (ApjBody_Max):	
				if (Make_Body_NFO_OutOfDate(Prj, OutOfDate, Printf)==JE_FALSE)
					return JE_FALSE;
				if (*OutOfDate != JE_FALSE)
					return JE_TRUE;
				break;
			case (ApjBody_Nfo):
			case (ApjBody_Bdy):
			case (ApjBody_Act):
				break;
			case (ApjBody_Invalid):
			default:
				{
					Printf("Error: unrecognized format specifier for body (%d) \n",Fmt);
					return JE_FALSE;
				}
		}

	ObjDir = AProject_GetObjPath( Prj );
	if (ObjDir == NULL)
		{
			Printf("Error: Unable to get temporary path\n");
			return JE_FALSE;
		}

	SourceName = AProject_GetBodyFilename (Prj);
	if (SourceName == NULL)
		{
			Printf("Error: Can't get source file name for body\n");
			return JE_FALSE;
		} 
	Make_TargetFileName( TargetName, SourceName, ObjDir, "BDY");

	if (Make_IsTargetOutOfDate( TargetName, SourceName, OutOfDate, Printf) == JE_FALSE)
		{	// already posted
			return JE_FALSE;
		}
	return JE_TRUE;
}

jeBoolean Make_SourceOutOfDateFromBDY( AProject *Prj, 
			const char *FileName,
			jeBoolean *OutOfDate,
			MkUtil_Printf Printf)
{
	char TargetName[BIG];
	const char *SourceName;
	const char *ObjDir;

	assert( Prj       != NULL );
	assert( OutOfDate != NULL );
	assert( Printf    != NULL );
	assert( FileName  != NULL );

	// if body needs to be made, then the source is out-of-date	
	if (Make_Body_BDY_OutOfDate( Prj, OutOfDate, Printf) == JE_FALSE)
		return JE_FALSE;
	if (*OutOfDate == JE_TRUE)
		return JE_TRUE;
	// body doesn't need to be made.  But maybe the source is older than the bdy
	
	ObjDir = AProject_GetObjPath( Prj );
	if (ObjDir == NULL)
		{
			Printf("Error: Unable to get temporary path\n");
			return JE_FALSE;
		}

	SourceName = AProject_GetBodyFilename (Prj);
	if (SourceName == NULL)
		{
			Printf("Error: Can't get source file name for body\n");
			return JE_FALSE;
		} 
	Make_TargetFileName( TargetName, SourceName, ObjDir, "BDY");

	if (Make_IsTargetOutOfDate( FileName, TargetName, OutOfDate, Printf) == JE_FALSE)
		{	// already posted
			return JE_FALSE;
		}

	// now lets see if source is out of date 

	return JE_TRUE;
}


jeBoolean Make_Motion_KEY_OutOfDate(AProject *Prj, int MotionIndex,
				jeBoolean *OutOfDate, MkUtil_Printf Printf)
{
	const char *ObjDir;
	ApjMotionFormat MotionFmt;
	const char *SourceName;
	char TargetName[BIG];

	assert( Prj       != NULL );
	assert( OutOfDate != NULL );
	assert( Printf    != NULL );
	assert( MotionIndex >= 0  );
	assert( MotionIndex < AProject_GetMotionsCount( Prj ) );
	
	*OutOfDate = JE_FALSE;
	MotionFmt = AProject_GetMotionFormat( Prj, MotionIndex );

	if ( MotionFmt != ApjMotion_Max )
		return JE_TRUE;

	ObjDir = AProject_GetObjPath( Prj );
	if (ObjDir == NULL)
		{
			Printf("Error:  Unable to get temporary path to check dependencies for motions\n");
			return JE_FALSE;
		}

	SourceName = AProject_GetMotionFilename( Prj, MotionIndex );
	if (SourceName == NULL)
		{
			Printf("Error: Can't get source file name for motion %d\n", MotionIndex);
			return JE_FALSE;
		}
	Make_TargetFileName( TargetName, SourceName, ObjDir,"KEY" );
	if (Make_IsTargetOutOfDate(TargetName, SourceName, OutOfDate, Printf ) == JE_FALSE )
		return JE_FALSE;

	return JE_TRUE;
}


jeBoolean Make_Motion_MOT_OutOfDate(AProject *Prj, int MotionIndex, 
				jeBoolean *OutOfDate, MkUtil_Printf Printf)
{
	const char *ObjDir;
	ApjMotionFormat MotionFmt;
	const char *SourceName;
	char TargetName[BIG];

	assert( Printf != NULL );
	assert( Prj != NULL );
	assert( OutOfDate != NULL );

	
	*OutOfDate = JE_FALSE;

	SourceName = AProject_GetMotionFilename( Prj, MotionIndex );
	if (SourceName == NULL)
		{
			Printf("Error: Can't get source file name for motion %d\n", MotionIndex);
			return JE_FALSE;
		}

	MotionFmt = AProject_GetMotionFormat( Prj, MotionIndex );
	switch (MotionFmt)
		{
			case (ApjMotion_Max):
				if (*OutOfDate == JE_TRUE)
					return JE_TRUE;
				if (Make_Motion_KEY_OutOfDate( Prj, MotionIndex, OutOfDate, Printf)== JE_FALSE)
					return JE_FALSE;
				if (*OutOfDate == JE_TRUE)
					return JE_TRUE;
				break;
			case (ApjMotion_Key):
				if (*OutOfDate == JE_TRUE)
					return JE_TRUE;
				break;
// motions from actors not yet supported
//			case (ApjMotion_Act):
			case (ApjMotion_Mot):
				// body needed for mkmotion
				if (Make_SourceOutOfDateFromBDY( Prj, SourceName, OutOfDate, Printf) == JE_FALSE)
					return JE_FALSE;
				break;
			case (ApjMotion_Invalid):
			default:
				{
					Printf("Error: unrecognized format specifier for motion #%d (%d) \n",MotionIndex,MotionFmt);
					return JE_FALSE;
				}
		}

	ObjDir = AProject_GetObjPath( Prj );
	if (ObjDir == NULL)
		{
			Printf("Error:  Unable to get temporary path to check dependencies for motions\n");
			return JE_FALSE;
		}

	
	Make_TargetFileName( TargetName, SourceName, ObjDir, "MOT" );
	if (Make_IsTargetOutOfDate(TargetName, SourceName, OutOfDate, Printf ) == JE_FALSE )
		return JE_FALSE;
	return JE_TRUE;
}



jeBoolean Make_AnyMotion_KEY_OutOfDate(AProject *Prj, 
				int MotionIndexCount, int *MotionIndexArray, 
				jeBoolean *OutOfDate,
				MkUtil_Printf Printf)
{
	int i;

	assert( Printf           != NULL );
	assert( Prj              != NULL );
	assert( OutOfDate        != NULL );

	*OutOfDate = JE_FALSE;
	for (i=0; i<MotionIndexCount; i++)
		{
			assert( MotionIndexArray != NULL );
			if (Make_Motion_KEY_OutOfDate( Prj, MotionIndexArray[i], OutOfDate, Printf )==JE_FALSE)
				return JE_FALSE;
			if (*OutOfDate != JE_FALSE)
				return JE_TRUE;
		}
	return JE_TRUE;
}


jeBoolean Make_AnyMotion_MOT_OutOfDate(AProject *Prj, 
				int MotionIndexCount, int *MotionIndexArray, 
				jeBoolean *OutOfDate,
				MkUtil_Printf Printf)
{
	int i;

	assert( Printf           != NULL );
	assert( Prj              != NULL );
	assert( OutOfDate        != NULL );

	*OutOfDate = JE_FALSE;
	for (i=0; i<MotionIndexCount; i++)
		{
			assert( MotionIndexArray != NULL );
			if (Make_Motion_MOT_OutOfDate( Prj, MotionIndexArray[i], OutOfDate, Printf )==JE_FALSE)
				return JE_FALSE;
			if (*OutOfDate != JE_FALSE)
				return JE_TRUE;
		}
	return JE_TRUE;
}



jeBoolean Make_MaxScript( AProject *Prj, 
			AOptions *Options, 
			int DoBody,
			int MotionIndexCount, int *MotionIndexArray, 
			MkUtil_Printf Printf )
{
#define MAXIMUM_MOTIONS_PER_EXPORT 25
	MXScript *Script = NULL;
	char TargetName[BIG];

	const char *SourceName;
	const char *ObjDir;
	const char *Max;
			
	int i;
	//int MotionCount;
	int Times;

	jeBoolean OutOfDate = JE_FALSE;

	assert( Prj != NULL );
	assert( MotionIndexCount >= 0 );
	assert( (MotionIndexCount==0) || ((MotionIndexCount > 0) && (MotionIndexArray != NULL)) );
	assert( Options != NULL );
	assert( Printf != NULL );

	ObjDir = AProject_GetObjPath( Prj );
	if (ObjDir == NULL)
		{
			Printf("Error:  Unable to get temporary path to build MAXScript\n");
			goto Make_MaxScriptError;
		}

	// first, see if we even need to do any exporting:
	if (DoBody!=JE_FALSE)
		{
			if (Make_Body_NFO_OutOfDate( Prj, &OutOfDate, Printf) == JE_FALSE)
				goto Make_MaxScriptError;
		}

	if (OutOfDate == JE_FALSE)
		{
			if (Make_AnyMotion_KEY_OutOfDate(Prj, MotionIndexCount, 
							MotionIndexArray, &OutOfDate, Printf) == JE_FALSE)
					goto Make_MaxScriptError;
		}

	if ( OutOfDate == JE_FALSE)
		{
			return JE_TRUE;
		}

	Max = AOptions_Get3DSMaxPath( Options );
	if (Max == NULL)
		{
			Printf("Error: Cant Get 3DS MAX exe file name\n");
			goto Make_MaxScriptError;
		}
	
	if (MXScript_ArePluginsInstalled(Max,Printf)==JE_FALSE)
		{
			Printf("Error: Plugin missing. Max export halted.\n");
			goto Make_MaxScriptError;
		}

	Printf("\tBuilding 3DS MAX MAXScript\n");
	
	// build the script
	Script = MXScript_StartScript(ObjDir,Printf);
	if (Script == NULL)
		{
			Printf("Error: Can't start script file\n");
			goto Make_MaxScriptError;
		}

	if (DoBody!=JE_FALSE)
		{
			if (Make_Body_NFO_OutOfDate( Prj, &OutOfDate, Printf) == JE_FALSE)
				goto Make_MaxScriptError;
			if (OutOfDate!=JE_FALSE)
				{
					SourceName = AProject_GetBodyFilename (Prj);
					if (SourceName == NULL)
						{
							Printf("Error: Can't get source file name for body\n");
							goto Make_MaxScriptError;
						} 

					Make_TargetFileName( TargetName, SourceName, ObjDir, "NFO" );
					if (MXScript_AddExport(Script,SourceName,TargetName,Printf)==JE_FALSE)
						{
							Printf("Error: Can't add body export to script file\n");
							goto Make_MaxScriptError;
						}
				}
		}

	i=0;
	Times=1;

	while (i<MotionIndexCount || (DoBody!=JE_FALSE))
		{
			int j;
			if (Script==NULL)
				{
					Printf("\tBuilding 3DS MAX MAXScript (%d)\n",Times);
					
					// build the script
					Script = MXScript_StartScript(ObjDir,Printf);
					if (Script == NULL)
						{
							Printf("Error: Can't start script file\n");
							goto Make_MaxScriptError;
						}
				}
					
			//MotionCount = AProject_GetMotionsCount( Prj );
			for (j=0; j<MAXIMUM_MOTIONS_PER_EXPORT && i<MotionIndexCount; i++,j++)
				{
					if (Make_Motion_KEY_OutOfDate(Prj, MotionIndexArray[i],&OutOfDate,Printf) == JE_FALSE)
						goto Make_MaxScriptError;

					if (OutOfDate != JE_FALSE)
						{
							SourceName = AProject_GetMotionFilename( Prj, MotionIndexArray[i] );
							if (SourceName == NULL)
								{
									Printf("Error: Can't get source file name for motion %d\n", MotionIndexArray[i]);
									goto Make_MaxScriptError;
								}
							Make_TargetFileName( TargetName, SourceName, ObjDir,"KEY" );
							if (MXScript_AddExport(Script,SourceName,TargetName,Printf)==JE_FALSE)
								{
									Printf("Error: Can't add '%s' export to script file\n",SourceName);
									goto Make_MaxScriptError;
								}
						}
				}

			if (MXScript_EndScript(Script,Printf)==JE_FALSE)
				{
					Printf("Error: Failed to close script file\n");
					goto Make_MaxScriptError;
				}
			
			// run the script
			Printf("\tRunning 3DS MAX MAXScript\n");

			if ( MXScript_RunScript( Script, Max, Printf ) == JE_FALSE)
				{
					Printf("Error: 3DS MAX export script failed\n");
					goto Make_MaxScriptError;
				}

			MXScript_Destroy( Script );
			Script = NULL;
			Times++;
			DoBody = JE_FALSE;
		}

	return JE_TRUE;



Make_MaxScriptError:
	if ( Script != NULL )
		MXScript_Destroy( Script );
	return JE_FALSE;

}


jeBoolean Make_Body_BDY(AProject *Prj, AOptions *BuildOptions, MkUtil_Printf Printf)
{
	ApjBodyFormat Fmt;
	ReturnCode RVal;
		
	char OptionString[BIG];
	char TargetName[BIG];
	char NFOName[BIG];
	const char *SourceName;
	const char *ObjDir;
	jeBoolean OutOfDate;
	
	assert( Prj      != NULL );
	assert( BuildOptions  != NULL );
	assert( Printf   != NULL );

	if (Make_Body_BDY_OutOfDate( Prj, &OutOfDate, Printf) == JE_FALSE)
		return JE_FALSE;

	if (OutOfDate == JE_FALSE)
		return JE_TRUE;
	
	Printf("\tMaking Body\n");
	
	ObjDir = AProject_GetObjPath( Prj );
	if (ObjDir == NULL)
		{
			Printf("Error: (Make_Body) Unable to get temporary path\n");
			return JE_FALSE;
		}
	
	SourceName = AProject_GetBodyFilename (Prj);
	if (SourceName == NULL)
		{
			Printf("Error: Can't get source file name for body\n");
			return JE_FALSE;
		} 

	Make_TargetFileName( TargetName, SourceName, ObjDir, "BDY");
	strcpy(NFOName,SourceName);

	Fmt = AProject_GetBodyFormat(Prj);
	switch (Fmt)
		{
			case (ApjBody_Max):	
				if (Make_MaxScript( Prj, BuildOptions, JE_TRUE, 
									0, NULL, Printf ) == JE_FALSE)
					{
						Printf("Error: unable to complete 3DS MAX script and export step for Make_Body\n");
						return JE_FALSE;
					}
				Make_TargetFileName( NFOName, SourceName, ObjDir, "NFO");
				// fall through
			case (ApjBody_Nfo):
				{
					MkBody_Options *Options;
					const char *TexturePathName;
				
					Printf("\tMaking Body '%s' from NFO '%s'\n", TargetName,NFOName);

					Options = MkBody_OptionsCreate();
					if (Options == NULL)
						{
							Printf("Error: unable to allocate option block for mkBody\n");
							return JE_FALSE;
						}
					sprintf(OptionString,"-R");
					RVal = MkBody_ParseOptionString(Options,OptionString,MK_FALSE,Printf);
					if (RVal != RETURN_SUCCESS)
						{
							Printf("Error: unable to set options into MkBody:\n");
							Printf(OptionString);
							MkBody_OptionsDestroy(&Options);
							return JE_FALSE;
						}

					sprintf(OptionString,"-C");
					RVal = MkBody_ParseOptionString(Options,OptionString,MK_FALSE,Printf);
					if (RVal != RETURN_SUCCESS)
						{
							Printf("Error: unable to set options into MkBody:\n");
							Printf(OptionString);
							MkBody_OptionsDestroy(&Options);
							return JE_FALSE;
						}
					sprintf(OptionString,"-B%s",TargetName);
					RVal = MkBody_ParseOptionString(Options,OptionString,MK_FALSE,Printf);
					if (RVal != RETURN_SUCCESS)
						{
							Printf("Error: unable to set options into MkBody:\n");
							Printf(OptionString);
							MkBody_OptionsDestroy(&Options);
							return JE_FALSE;
						}

					sprintf(OptionString,"-N%s",NFOName);
					RVal = MkBody_ParseOptionString(Options,OptionString,MK_FALSE,Printf);
					if (RVal != RETURN_SUCCESS)
						{
							Printf("Error: unable to set options into MkBody:\n");
							Printf(OptionString);
							MkBody_OptionsDestroy(&Options);
							return JE_FALSE;
						}
					
					TexturePathName = AProject_GetMaterialsPath(Prj);
					if (TexturePathName == NULL)
						{
							Printf("Error: Can't get texture path for body\n");
							MkBody_OptionsDestroy(&Options);
							return JE_FALSE;
						} 
					if (TexturePathName[0] == 0)
						{
							Printf("Warning: no material path specified\n");
						}
					else
						{
							sprintf(OptionString,"-T%s",TexturePathName);
							RVal = MkBody_ParseOptionString(Options,OptionString,MK_FALSE,Printf);
							if (RVal != RETURN_SUCCESS)
								{
									Printf("Error: unable to set options into MkBody:\n");
									Printf(OptionString);
									MkBody_OptionsDestroy(&Options);
									return JE_FALSE;
								}
						}
					
					{
						int i;
						int MatCnt = AProject_GetMaterialsCount(Prj);
						assert( MatCnt >= 0 );
						for (i=0; i<MatCnt; i++)
							{
								ApjMaterialFormat MatFmt; 
								JE_RGBA Color;
								const char *MatName;
								const char *MatFileName;
								MatFmt  = AProject_GetMaterialFormat(Prj,i);
								MatName = AProject_GetMaterialName(Prj,i);
								MatFileName = AProject_GetMaterialTextureFilename (Prj, i);

								if (MatName == NULL)
									{
										Printf("Error: unable to get extra material name %d\n",i);
										Printf(OptionString);
										MkBody_OptionsDestroy(&Options);
										return JE_FALSE;
									}
								
								if (strlen(MatName) == 0)
									{
										Printf("Error: empty extra material name %d\n",i);
										Printf(OptionString);
										MkBody_OptionsDestroy(&Options);
										return JE_FALSE;
									}
								switch (MatFmt)
									{
										case (ApjMaterial_Color):
											Color = AProject_GetMaterialTextureColor (Prj,i);
											sprintf(OptionString,"-M(RGB) %s: %f %f %f",MatName,Color.r,Color.g,Color.b);
											break;
										case (ApjMaterial_Texture):
											if (MatFileName == NULL)
												{
													Printf("Error: unable to get extra material filename %d\n",i);
													Printf(OptionString);
													MkBody_OptionsDestroy(&Options);
													return JE_FALSE;
												}
											if (strlen(MatFileName) == 0)
												{
													Printf("Error: empty extra material filename %d\n",i);
													Printf(OptionString);
													MkBody_OptionsDestroy(&Options);
													return JE_FALSE;
												}
											sprintf(OptionString,"-M(MAP) %s: %s",MatName,MatFileName);
											break;
										default:
											assert(0);
									}
								RVal = MkBody_ParseOptionString(Options,OptionString,MK_FALSE,Printf);
									if (RVal != RETURN_SUCCESS)
										{
											Printf("Error: unable to set extra material %d options into MkBody:\n",i);
											Printf(OptionString);
											MkBody_OptionsDestroy(&Options);
											return JE_FALSE;
										}
							}
					}


					RVal = MkBody_DoMake(Options,Printf);
					MkBody_OptionsDestroy(&Options);

					if (RVal == RETURN_ERROR )
						{
							Printf("Error: Failed to build '%s' file from '%s' file\n",
										TargetName,NFOName);
							return JE_FALSE;
						}
					if (RVal == RETURN_WARNING)
						{
							Printf("Warnings issued during building of '%s' from '%s'\n",
										TargetName,NFOName);
						}
					Printf("\tBody file '%s' successfully built\n",TargetName);
					return JE_TRUE;
				}
			case (ApjBody_Bdy):
				{
					if (Make_CopyBodyFile(TargetName, SourceName,Printf )==JE_FALSE)
						{
							Printf("Error: failed to copy body file '%s' into work file '%s'\n", SourceName,TargetName);
							return JE_FALSE;
						}
					Printf("\tBody file '%s' successfully prepared\n",TargetName);
					return JE_TRUE;
				}
			case (ApjBody_Act):
				{
					Printf("Error: getting body from existing actor file not yet implemented\n");
					return JE_FALSE;
				}
			case (ApjBody_Invalid):
			default:
				{
					Printf("Error: unrecognized format specifier for body\n");
					return JE_FALSE;
				}
		}
//	Printf("\tBody file '%s' successfully built\n",TargetName);
	//return JE_TRUE;
}


jeBoolean Make_Optimize_Motion( char *FromFile, char *ToFile, char *LogFile, int Level, MkUtil_Printf Printf )
{
	assert( Printf         != NULL );
	assert( FromFile       != NULL );
	assert( ToFile         != NULL );
	assert( LogFile        != NULL );
	assert( (Level >=0) && (Level<=9));

	Printf("\tCompressing Motion '%s'    (Details in log file '%s')\n",ToFile,LogFile);

	{
		char OptionString[BIG];
		MopShell_Options *Options;
		ReturnCode RVal;
	
		Options = MopShell_OptionsCreate();
		if (Options == NULL)
			{
				Printf("Error: unable to allocate option block for motion optimizer\n");
				return JE_FALSE;
			}
		sprintf(OptionString,"-O%d",Level);
		RVal = MopShell_ParseOptionString(Options,OptionString,MK_FALSE,Printf);
		if (RVal != RETURN_SUCCESS)
			{
				Printf("Error: unable to set options into motion optimizer:\n");
				Printf(OptionString);
				MopShell_OptionsDestroy(&Options);
				return JE_FALSE;
			}
		
		sprintf(OptionString,"-S%s",FromFile);
		RVal = MopShell_ParseOptionString(Options,OptionString,MK_FALSE,Printf);
		if (RVal != RETURN_SUCCESS)
			{
				Printf("Error: unable to set options into motion optimizer:\n");
				Printf(OptionString);
				MopShell_OptionsDestroy(&Options);
				return JE_FALSE;
			}
		
		sprintf(OptionString,"-D%s",ToFile);
		RVal = MopShell_ParseOptionString(Options,OptionString,MK_FALSE,Printf);
		if (RVal != RETURN_SUCCESS)
			{
				Printf("Error: unable to set options into motion optimizer:\n");
				Printf(OptionString);
				MopShell_OptionsDestroy(&Options);
				return JE_FALSE;
			}
		sprintf(OptionString,"-L%s",LogFile);
		RVal = MopShell_ParseOptionString(Options,OptionString,MK_FALSE,Printf);
		if (RVal != RETURN_SUCCESS)
			{
				Printf("Error: unable to set options into motion optimizer:\n");
				Printf(OptionString);
				MopShell_OptionsDestroy(&Options);
				return JE_FALSE;
			}
		

		RVal = MopShell_DoMake(Options,Printf);
		MopShell_OptionsDestroy(&Options);

		if (RVal == RETURN_ERROR )
			{
				Printf("Error: motion optimize failed\n");
				return JE_FALSE;
			}
		if (RVal == RETURN_WARNING)
			{
				Printf("Warnings issued during motion optimize\n");
			}
	}
	
	return JE_TRUE;	
}


jeBoolean Make_Motion_MOT( AProject *Prj, AOptions *AOptions, 
							int MotionIndex, MkUtil_Printf Printf)
// makes MOT from KEY, MOT, or ACT
{
	char OptionString[BIG];
	char TargetName[BIG];
	char FinalTargetName[BIG];
	char BodyName[BIG];
	char KEYName[BIG];

	const char *MotionName;
	const char *SourceName;
	const char *ObjDir;
	const char *BoneName;
	const char *Empty = "";
	ApjMotionFormat Fmt;
	ReturnCode RVal;
	jeBoolean OutOfDate;
	
	assert( Prj      != NULL );
	assert( Printf   != NULL );
	assert( AOptions != NULL );
	
	MotionName = AProject_GetMotionName (Prj,MotionIndex);
	if (MotionName == NULL) MotionName =Empty;

	Printf("\tMaking Motion #%d '%s'\n",MotionIndex, MotionName);
	
	if (Make_Motion_MOT_OutOfDate( Prj, MotionIndex, &OutOfDate, Printf) == JE_FALSE)
		return JE_FALSE;
	if (OutOfDate == JE_FALSE)
		{
			Printf("\tMotion #%d '%s' is up to date \n",MotionIndex, MotionName);
			return JE_TRUE;
		}

	ObjDir = AProject_GetObjPath( Prj );
	if (ObjDir == NULL)
		{
			Printf("Error: (Make_Motion) Unable to get temporary path\n");
			return JE_FALSE;
		}

	SourceName = AProject_GetBodyFilename (Prj);
	if (SourceName == NULL)
		{
			Printf("Error: Can't get source name for body\n");
			return JE_FALSE;
		} 

	Make_TargetFileName( BodyName, SourceName, ObjDir, "BDY");


	SourceName = AProject_GetMotionFilename (Prj,MotionIndex);
	if (SourceName == NULL)
		{
			Printf("Error: Can't get source file for motion #%d '%s'\n",MotionIndex,MotionName);
			return JE_FALSE;
		} 
	
	Make_TargetFileName( TargetName, SourceName, ObjDir, "MO1");
	Make_TargetFileName( FinalTargetName, SourceName, ObjDir, "MOT");
	strcpy(KEYName,SourceName);

	Fmt = AProject_GetMotionFormat(Prj,MotionIndex);
	switch (Fmt)
		{
			case (ApjMotion_Max):
				{
					int MotionIndexArray[1];
					MotionIndexArray[0] = MotionIndex;
					if (Make_MaxScript( Prj, AOptions, JE_FALSE, 1, MotionIndexArray, Printf ) == JE_FALSE)
						{
							Printf("Error: unable to complete 3DS MAX script and export step for Make_Motion\n");
							return JE_FALSE;
						}
					Make_TargetFileName( KEYName, SourceName, ObjDir, "KEY");
				}
				// fall through
			case (ApjMotion_Key):
				{
					MkMotion_Options *Options;
	
					if (Make_Body_BDY( Prj, AOptions, Printf)==JE_FALSE)
						return JE_FALSE;	
					
					MotionName = AProject_GetMotionName (Prj,MotionIndex);
					if (MotionName == NULL) MotionName =Empty;
					
					BoneName = AProject_GetMotionBone(Prj,MotionIndex);
					if (BoneName == NULL) BoneName =Empty;
					
					Printf("\tMaking Motion %d (Name = '%s'): '%s' from KEY '%s'\n", MotionIndex, MotionName, TargetName,KEYName);

					Options = MkMotion_OptionsCreate();
					if (Options == NULL)
						{
							Printf("Error: unable to allocate option block for mkMotion\n");
							return JE_FALSE;
						}
					sprintf(OptionString,"-C");
					RVal = MkMotion_ParseOptionString(Options,OptionString,MK_FALSE,Printf);
					if (RVal != RETURN_SUCCESS)
						{
							Printf("Error: unable to set options into MkMotion:\n");
							Printf(OptionString);
							MkMotion_OptionsDestroy(&Options);
							return JE_FALSE;
						}
					
					sprintf(OptionString,"-E");
					RVal = MkMotion_ParseOptionString(Options,OptionString,MK_FALSE,Printf);
					if (RVal != RETURN_SUCCESS)
						{
							Printf("Error: unable to set options into MkMotion:\n");
							Printf(OptionString);
							MkMotion_OptionsDestroy(&Options);
							return JE_FALSE;
						}
					
					sprintf(OptionString,"-M%s",TargetName);
					RVal = MkMotion_ParseOptionString(Options,OptionString,MK_FALSE,Printf);
					if (RVal != RETURN_SUCCESS)
						{
							Printf("Error: unable to set options into MkMotion:\n");
							Printf(OptionString);
							MkMotion_OptionsDestroy(&Options);
							return JE_FALSE;
						}
					
					sprintf(OptionString,"-K%s",KEYName);
					RVal = MkMotion_ParseOptionString(Options,OptionString,MK_FALSE,Printf);
					if (RVal != RETURN_SUCCESS)
						{
							Printf("Error: unable to set options into MkMotion:\n");
							Printf(OptionString);
							MkMotion_OptionsDestroy(&Options);
							return JE_FALSE;
						}
					sprintf(OptionString,"-B%s",BodyName);
					RVal = MkMotion_ParseOptionString(Options,OptionString,MK_FALSE,Printf);
					if (RVal != RETURN_SUCCESS)
						{
							Printf("Error: unable to set options into MkMotion:\n");
							Printf(OptionString);
							MkMotion_OptionsDestroy(&Options);
							return JE_FALSE;
						}
					if (MotionName[0] != 0)
						{
							sprintf(OptionString,"-N%s",MotionName);
							RVal = MkMotion_ParseOptionString(Options,OptionString,MK_FALSE,Printf);
							if (RVal != RETURN_SUCCESS)
								{
									Printf("Error: unable to set options into MkMotion:\n");
									Printf(OptionString);
									MkMotion_OptionsDestroy(&Options);
									return JE_FALSE;
								}
						}
									
					{
						char BName[BIG];
						char *BPtr;
						char *BStart;
						strcpy(BName,BoneName);
						BPtr = BName;
						do
							{
								BStart = BPtr;
								BPtr = strchr(BName,';');
								if (BStart != NULL)
									{
										if (BPtr != NULL)
											{
												*BPtr=0;
												BPtr++;
											}
										if (BStart[0] != 0)
										{
											sprintf(OptionString,"-R%s", BStart);
											RVal = MkMotion_ParseOptionString(Options,OptionString,MK_FALSE,Printf);
											if (RVal != RETURN_SUCCESS)
												{
													Printf("Error: unable to set options into MkMotion:\n");
													Printf(OptionString);
													MkMotion_OptionsDestroy(&Options);
													return JE_FALSE;
												}
										}
									}
							}
						while (BPtr != NULL);
					}	
					
					RVal = MkMotion_DoMake(Options,Printf);
					MkMotion_OptionsDestroy(&Options);
	
					if (RVal == RETURN_ERROR )
						{
							Printf("Error: Failed to build '%s' file from '%s' file\n",
										TargetName,KEYName);
							return JE_FALSE;
						}
					if (RVal == RETURN_WARNING)
						{
							Printf("Warnings issued during building of '%s' from '%s'\n",
										TargetName,KEYName);
						}
				}
				break;
			case (ApjMotion_Mot):
				if (Make_CopyMotionFile(TargetName, SourceName,Printf )==JE_FALSE)
					{
						Printf("Error: failed to copy motion fle '%s' into work file '%s'\n", SourceName,TargetName);
						return JE_FALSE;
					}
				break;
// Motions from actors not yet implemented
/*
			case (ApjMotion_Act):
				{
					Printf("Error: getting motion from existing actor file not yet implemented\n");
					return JE_FALSE;
				}
*/
			case (ApjMotion_Invalid):
			default:
				{
					Printf("Error: unrecognized format specifier for motion '%s'\n",SourceName);
					return JE_FALSE;
				}				
		}


	if (AProject_GetMotionOptimizationFlag ( Prj, MotionIndex ) == JE_FALSE)
		{
			if (Make_CopyMotionFile( FinalTargetName, TargetName, Printf)==JE_FALSE)
				{
					Printf("Error: unable to copy temporary motion file '%s' to final '%s'\n",TargetName, FinalTargetName);
					return JE_FALSE;
				}
		}
	else
		{
			char LogName[BIG];
			
			Make_TargetFileName( LogName, SourceName, ObjDir, "LOG");
	
			if (Make_Optimize_Motion( TargetName, FinalTargetName, LogName, 
						AProject_GetMotionOptimizationLevel ( Prj, MotionIndex ), Printf)==JE_FALSE)
				{
					Printf("Error: unable to optimize temporary motion file '%s' to final '%s'\n",TargetName, FinalTargetName);
					return JE_FALSE;
				}
		}
				
	return JE_TRUE;	
}



jeBoolean Make_Body(AProject *Prj, AOptions *BuildOptions, MkUtil_Printf Printf)
{
	jeBoolean OutOfDate;
	
	assert( Prj      != NULL );
	assert( BuildOptions  != NULL );
	assert( Printf   != NULL );

	if (Make_Body_BDY_OutOfDate( Prj, &OutOfDate, Printf) == JE_FALSE)
		return JE_FALSE;
	
	if (OutOfDate == JE_FALSE)
		{
			Printf("\tBody is up to date\n");
			return JE_TRUE;
		}

	if (Make_Body_BDY( Prj, BuildOptions, Printf )==JE_FALSE)
		{
			Printf("Error: Failed to make BDY file\n");
			return JE_FALSE;
		}
	
	return JE_TRUE;
}


jeBoolean Make_Actor_ACT_OutOfDate( AProject *Prj, jeBoolean *OutOfDate, MkUtil_Printf Printf)
{
	int MotionCount;
	int *MotionIndexArray;
	int i;

	assert( Prj       != NULL );
	assert( OutOfDate != NULL );
	assert( Printf    != NULL );

	*OutOfDate = JE_FALSE;

	if (Make_Body_BDY_OutOfDate( Prj, OutOfDate, Printf )==JE_FALSE)
		return JE_FALSE;
	if (*OutOfDate != JE_FALSE)
		return JE_TRUE;

	MotionCount = AProject_GetMotionsCount( Prj );
	MotionIndexArray = JE_RAM_ALLOCATE_ARRAY( int, MotionCount);
	if (MotionIndexArray == NULL)
		{
			Printf("Error: unable to get memory for Make Actor step\n");
			return JE_FALSE;
		}
	
	for (i=0; i<MotionCount; i++)
		MotionIndexArray[i] = i;
	
	if (Make_AnyMotion_MOT_OutOfDate( Prj, MotionCount, MotionIndexArray, OutOfDate, Printf )==JE_FALSE)
		{
			jeRam_Free(MotionIndexArray);
			return JE_FALSE;
		}

	jeRam_Free(MotionIndexArray);

	return JE_TRUE;
}


jeBoolean Make_Motion(AProject *Prj, AOptions *Options, 
					  int MotionCount, int *MotionIndexArray, 
					  MkUtil_Printf Printf)
{
	int i;
	jeBoolean OutOfDate;

	assert( Prj       != NULL );
	assert( Options   != NULL );
	assert( Printf    != NULL );
	assert( MotionIndexArray != NULL );

	Printf("\tMaking Motions\n");			
	if (Make_MaxScript( Prj, Options, JE_TRUE, 
					MotionCount, MotionIndexArray, Printf ) == JE_FALSE)
		{
			Printf("Error: unable to complete 3DS MAX script and export step for Make_Actor\n");
			jeRam_Free(MotionIndexArray);
			return JE_FALSE;
		}

	if (Make_AnyMotion_MOT_OutOfDate( Prj, MotionCount, MotionIndexArray, &OutOfDate, Printf )==JE_FALSE)
		return JE_FALSE;
		
	if (OutOfDate == JE_FALSE)
		{
			Printf("\tMotions are up to date\n");
			return JE_TRUE;
		}
	for (i=0; i<MotionCount; i++)
		{
			if (Make_Motion_MOT( Prj, Options, i, Printf)==JE_FALSE)
				return JE_FALSE;
		}
	Printf("\tFinished making Motions successfully\n");			
	return JE_TRUE;
}


jeBoolean Make_Actor(AProject *Prj, AOptions *Options, MkUtil_Printf Printf)
{
	int MotionCount;
	int *MotionIndexArray;
	int i;
	jeBoolean OutOfDate=JE_FALSE;
	const char *ObjDir;
	
	assert( Prj      != NULL );
	assert( Options  != NULL );
	assert( Printf   != NULL );

	{
		const char *TargetName;
		long Handle;
		struct _finddata_t TargetData;
		TargetName  = AProject_GetOutputFilename (Prj);
		if (TargetName == NULL)
			{
				Printf("Error: Can't get output filename\n");
				return JE_FALSE;
			} 
		#pragma message ("would like to test against date of apj file")
	
		Handle = (long)_findfirst( TargetName, &TargetData );
		if (Handle == -1)
			{
				Printf("Actor doesn't exist:\n",TargetName);
				OutOfDate = JE_TRUE;
			}
		else
			{
				_findclose(Handle);
			}
	}
	

		
	
	ObjDir = AProject_GetObjPath( Prj );
	if (ObjDir == NULL)
		{
			Printf("Error: Unable to get temporary path\n");
			return JE_FALSE;
		}

	if (_mkdir(ObjDir)==-1)
		{
			FILE *F;
			char TempFileName[BIG];
			sprintf(TempFileName,"%s\\tempdir.tst",ObjDir);
			F=fopen(TempFileName,"a");
			if (F==NULL)
				{
					Printf("Error: Unable to create temporary path '%s'\n",ObjDir);
					return JE_FALSE;
				}
			fclose(F);
		}

	if (OutOfDate==JE_FALSE)
		if (Make_Actor_ACT_OutOfDate( Prj, &OutOfDate, Printf )==JE_FALSE)
			return JE_FALSE;

	if (OutOfDate == JE_FALSE)
		{
			Printf("\tActor is up to date.\n");
			return JE_TRUE;
		}

	// any script stuff first
	MotionCount = AProject_GetMotionsCount( Prj );
	MotionIndexArray = JE_RAM_ALLOCATE_ARRAY( int, MotionCount);
	if (MotionIndexArray == NULL)
		{
			Printf("Error: unable to get memory for Make Actor step\n");
			return JE_FALSE;
		}
	
	for (i=0; i<MotionCount; i++)
		{
			MotionIndexArray[i] = i;
		}
	if (Make_MaxScript( Prj, Options, JE_TRUE, 
						MotionCount, MotionIndexArray, Printf ) == JE_FALSE)
		{
			Printf("Error: unable to complete 3DS MAX script and export step for Make_Actor\n");
			jeRam_Free(MotionIndexArray);
			return JE_FALSE;
		}
	
	// body
	if (Make_Body(Prj,Options,Printf)==JE_FALSE)
		{
			Printf("Error: Failed to build body.  Unable to make Actor.\n");
			jeRam_Free(MotionIndexArray);
			return JE_FALSE;
		}

	// motions
	if (Make_Motion(Prj, Options, MotionCount, MotionIndexArray, Printf)==JE_FALSE)
		{
			Printf("Error: Failed to build motions.  Unable to make Actor.\n");
			jeRam_Free(MotionIndexArray);
			return JE_FALSE;
		}

	jeRam_Free(MotionIndexArray);

	Printf("\tCombining components into final Actor...\n");
	{
		char OptionString[BIG];
		char BodyName[BIG];
		char MotionName[BIG];
		const char *SourceName;
		const char *TargetName;
	
		MkActor_Options *Options;
		ReturnCode RVal;


		SourceName = AProject_GetBodyFilename (Prj);
		if (SourceName == NULL)
			{
				Printf("Error: Can't get source name for body\n");
				return JE_FALSE;
			} 
		Make_TargetFileName( BodyName, SourceName, ObjDir, "BDY");

		TargetName  = AProject_GetOutputFilename (Prj);
		if (TargetName == NULL)
			{
				Printf("Error: Can't get output filename\n");
				return JE_FALSE;
			} 
		
		Options = MkActor_OptionsCreate();
		if (Options == NULL)
			{
				Printf("Error: unable to allocate option block for MkActor\n");
				return JE_FALSE;
			}
		sprintf(OptionString,"-A%s",TargetName);
		RVal = MkActor_ParseOptionString(Options,OptionString,MK_FALSE,Printf);
		if (RVal != RETURN_SUCCESS)
			{
				Printf("Error: unable to set options into MkActor:\n");
				Printf(OptionString);
				MkActor_OptionsDestroy(&Options);
				return JE_FALSE;
			}
		
		sprintf(OptionString,"-B%s",BodyName);
		RVal = MkActor_ParseOptionString(Options,OptionString,MK_FALSE,Printf);
		if (RVal != RETURN_SUCCESS)
			{
				Printf("Error: unable to set options into MkActor:\n");
				Printf(OptionString);
				MkActor_OptionsDestroy(&Options);
				return JE_FALSE;
			}
		
		for (i=0; i<MotionCount; i++)
			{

				SourceName = AProject_GetMotionFilename (Prj,i);
				if (SourceName == NULL)
					{
						Printf("Error: Can't get source file for motion #%d '%s'\n",i,MotionName);
						return JE_FALSE;
					} 
			
				Make_TargetFileName( MotionName, SourceName, ObjDir, "MOT");

				sprintf(OptionString,"-M%s",MotionName);
				RVal = MkActor_ParseOptionString(Options,OptionString,MK_FALSE,Printf);
				if (RVal != RETURN_SUCCESS)
					{
						Printf("Error: unable to set motion option into MkActor:\n");
						Printf(OptionString);
						MkActor_OptionsDestroy(&Options);
						return JE_FALSE;
					}
			}
						
		unlink(TargetName);		// don't want backup files

		RVal = MkActor_DoMake(Options,Printf);
		MkActor_OptionsDestroy(&Options);

		if (RVal == RETURN_ERROR )
			{
				Printf("Error: Final actor compilation failed\n");
				return JE_FALSE;
			}
		if (RVal == RETURN_WARNING)
			{
				Printf("Warnings issued during final actor compilation\n");
			}
	}

	return JE_TRUE;
	
}




jeBoolean Make_Clean(AProject *Prj, AOptions *Options, MkUtil_Printf Printf)
{
	const char *TargetName;
	const char *ObjDir;
	char DeleteName[BIG];
//	jeBoolean OutOfDate;
	long Handle;
	struct _finddata_t TargetData;
	
Options;		// remove unused parameter warning
	assert( Prj      != NULL );
	assert( Options  != NULL );
	assert( Printf   != NULL );

//	Make_CleanFlag = JE_TRUE;
//	if (Make_Actor_ACT_OutOfDate( Prj, &OutOfDate, Printf )==JE_FALSE)
//		return JE_FALSE;

	TargetName  = AProject_GetOutputFilename (Prj);
	if (TargetName == NULL)
		{
			Printf("Error: Can't get output filename\n");
			return JE_FALSE;
		} 
	
	Handle = (long) _findfirst( TargetName, &TargetData );
	if (Handle != -1)
		{
			_findclose(Handle);
			if (unlink(TargetName) != 0)
				{
					Printf("Error: Can't delete '%s'\n",TargetName);
					return JE_FALSE;
				}
			Printf("Target Actor file deleted\n");
		}
	else
		{
			Printf("Target Actor file not present\n");
		}

	ObjDir = AProject_GetObjPath( Prj );
	if (ObjDir == NULL)
		{
			Printf("Error: Unable to get temporary path\n");
			return JE_FALSE;
		}
	Make_TargetFileName( DeleteName, "*", ObjDir, "*");
	
	
	Handle = (long)_findfirst( DeleteName, &TargetData );
	if (Handle == -1)
		{
		}
	else
		{
			do 
				{
					if (TargetData.name[0]!='.')
						if (FilePath_AppendName(ObjDir,TargetData.name,DeleteName)!=JE_FALSE)
							{
								unlink(DeleteName);
							}
				}
			while (_findnext( Handle, &TargetData) == 0);
			_findclose(Handle);
		}
			

	
	Printf("Clean complete.  Temporary files deleted\n");
//	Make_CleanFlag = JE_FALSE;
	return JE_TRUE;
}



jeBoolean Make_ActorSummary(AProject *Prj, MkUtil_Printf Printf)     
	{ 
	const char *TargetName;
	jeBody *B;
	jeActor_Def *A;
	int i;
	int j;
	jeVFile *VF;
	long Handle;
	struct _finddata_t TargetData;
	
	assert( Prj    != NULL );
	assert( Printf != NULL );

	TargetName  = AProject_GetOutputFilename (Prj);
	if (TargetName == NULL)
		{
			Printf("Error: Can't get target Actor filename\n"); 
			return JE_FALSE;
		} 

	Handle = (long)_findfirst( TargetName, &TargetData );
	if (Handle == -1)
		{
			Printf("Error: Target Actor '%s' is not built\n",TargetName);
			return JE_FALSE;
		}
	_findclose(Handle);
	
	
	VF = jeVFile_OpenNewSystem(NULL,JE_VFILE_TYPE_DOS,TargetName,NULL,JE_VFILE_OPEN_READONLY);
	if (VF==NULL)
		{
			Printf("Error: Could not open target Actor file (%s).\n", TargetName);
			return JE_FALSE;
		}
	
	A = jeActor_DefCreateFromFile(VF);
	jeVFile_Close(VF);
	if (A==NULL)
		{
			Printf("Error: Failed to load actor from target actor file '%s'.\n", TargetName);
			return JE_FALSE;
		}

	B = jeActor_GetBody(A);
	if (B==NULL)
		{
			Printf("Actor has no body\n");
		}
	else
		{
			Printf("Body:\n");
			Printf("\t%d Bones\n",jeBody_GetBoneCount(B));
			for (j=0; j<jeBody_GetBoneCount(B); j++)
				{
					jeXForm3d A;
					int parent;
					const char *Name;
					jeBody_GetBone(B,j,&Name,&A,&parent);
					Printf("\t\tBone %d Name='%s'\n",j,Name);
				}
			Printf("\t%d Materials\n",jeBody_GetMaterialCount(B));
			for (j=0; j<jeBody_GetMaterialCount(B); j++)
				{
					jeMaterialSpec *MatSpec;
					const char *n;
					jeBitmap_Info BmpInfo;
					jeBitmap *Bmp = NULL;
					jeFloat r,g,b;
					int ir,ig,ib;
					jeUVMapper Mapper;

					jeBody_GetMaterial(B,j,&n,&MatSpec,&r,&g,&b,&Mapper);
                    Bmp = jeMaterialSpec_GetLayerBitmap(MatSpec, 0);
					
					ir=(int)r;
					ig=(int)g;
					ib=(int)b;
					Printf("\t\tMaterial %d Name='%s'  rgb=(%d %d %d)\n",
								j,n,ir,ig,ib);
							
					if (Bmp!=NULL)
						{
							jeBitmap_Info SecondaryInfo;

							jeBitmap_GetInfo(Bmp,&BmpInfo,&SecondaryInfo);
							Printf("\t\t         Bitmap Info:  Width=%d   Height=%d   Format ID=%d  \n",
								BmpInfo.Width,BmpInfo.Height,(int)BmpInfo.Format);
							Printf("\t\t                       Minimum Mip=%d   Maximum Mip=%d\n",
								BmpInfo.MinimumMip,BmpInfo.MaximumMip);
							if (BmpInfo.HasColorKey)
								{
									Printf("\t\t                       Color Key=%d\n",
										BmpInfo.ColorKey);
								}
							if (BmpInfo.Palette!=NULL)
								{
									Printf("\t\t                       (Palettized)\n");
								}
						}
					
				}
			Printf("\t%d Levels of Detail\n",JE_BODY_NUMBER_OF_LOD);
			for (j=0; j<JE_BODY_NUMBER_OF_LOD; j++)
				{
					int v,n,faces;
					jeBody_GetGeometryStats(B,j,&v,&faces,&n);
					Printf("\t\tLOD%d  %d Vertices   %d Normals   %d Faces\n",j,v,n,faces);
				}
		}
	Printf("%d Motions\n",jeActor_GetMotionCount(A));
	for (i=0; i<jeActor_GetMotionCount(A); i++)
		{
			jeMotion *M;
			const char *name;
			M=jeActor_GetMotionByIndex(A,i);
			name= jeMotion_GetName(M);

			{
				int Match=0;

				if (B!=NULL)
					{
						if (jeBody_GetBoneNameChecksum(B) == jeMotion_GetNameChecksum(M))
							Match = 1;
						else
							Match = 0;
					}
				if (name != NULL)
					Printf("\tMotion %d Name='%s' (%d Joints)  (%s)\n",i,
						name,jeMotion_GetPathCount(M),
						(Match==1)?"Matches Bones":"Doesn't Match Bones" );
				else
					Printf("\tMotion %d (no name) (%d Joints)  (%s)\n",i,
						jeMotion_GetPathCount(M),
						(Match==1)?"Matches Bones":"Doesn't Match Bones");
			}
		}
	return JE_TRUE;
}


