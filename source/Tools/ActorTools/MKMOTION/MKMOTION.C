/****************************************************************************************/
/*  MKMOTION.C	                                                                        */
/*                                                                                      */
/*  Author: Stephen Balkum	                                                            */
/*  Description: Motion construction from MAX export.									*/
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
#include "body.h"
#include "motion.h"
#include "ram.h"
#include "strblock.h"

#include "mkutil.h"
#include "tdbody.h"
#include "maxmath.h"
#include "mkmotion.h"

#define MK_PI 3.141592654f
#define LINE_LENGTH 5000

typedef struct MkMotion_Options
{
	char BodyFile[_MAX_PATH];
	char KeyFile[_MAX_PATH];
	char MotionFile[_MAX_PATH];
	MK_Boolean KeepStationary;
	MK_Boolean Originate;
	MK_Boolean Capitalize;
	float TimeOffset;
	jeStrBlock* pMotionRoots;
	char* pMotionName;
	char EventBoneSeparator;
	jeVec3d RootEulerAngles;
	jeVec3d RootTranslation;
	jeVec3d EulerAngles;		// rotation at read time
} MkMotion_Options;

const MkMotion_Options DefaultOptions =
{
	"",
	"",
	"",
	MK_FALSE,
	MK_FALSE,
	MK_FALSE,
	0.0f,
	NULL,
	NULL,
	'\0',						// default to no bone names on events
	{ 0.0f, 0.0f, 0.0f},
	{ 0.0f, 0.0f, 0.0f},
	{ 0.0f, 0.0f, 0.0f},
};

typedef struct
{
	jeStrBlock* pEvents;
	char name[LINE_LENGTH];
	jeXForm3d* pMatrixKeys;	// keys with respect to parent
	jeXForm3d* pWSKeys;		// keys in world space
} BoneKeyInfo;

void DestroyBoneKeyArray(BoneKeyInfo** ppInfo, int nNumBones)
{
	int i;
	BoneKeyInfo* pInfo;

	assert(ppInfo != NULL);
	pInfo = *ppInfo;
	assert(pInfo != NULL);
	assert(nNumBones > 0);

	for(i=0;i<nNumBones;i++)
	{
		if(pInfo[i].pEvents != NULL)
			jeStrBlock_Destroy(&pInfo[i].pEvents);
		if(pInfo[i].pMatrixKeys != NULL)
			jeRam_Free(pInfo[i].pMatrixKeys);
		if(pInfo[i].pWSKeys != NULL)
			jeRam_Free(pInfo[i].pWSKeys);
	}

	jeRam_Free(*ppInfo);
}

BoneKeyInfo* CreateBoneKeyArray(int nNumBones, int nNumKeys)
{
	BoneKeyInfo* pInfo;
	int i;

	assert(nNumBones > 0);
	assert(nNumKeys > 0);

	pInfo = JE_RAM_ALLOCATE_ARRAY(BoneKeyInfo, nNumBones);
	if(pInfo == NULL)
		return(NULL);

	// explicitly null the pointers
	for(i=0;i<nNumBones;i++)
	{
		pInfo[i].pEvents = NULL;
		pInfo[i].pMatrixKeys = NULL;
		pInfo[i].pWSKeys = NULL;
	}

	for(i=0;i<nNumBones;i++)
	{
		pInfo[i].pEvents = jeStrBlock_Create();
		if(pInfo[i].pEvents == NULL)
			goto CreateBoneKeyArray_Failure;

		pInfo[i].pMatrixKeys = JE_RAM_ALLOCATE_ARRAY(jeXForm3d, nNumKeys);
		if(pInfo[i].pMatrixKeys == NULL)
			goto CreateBoneKeyArray_Failure;

		pInfo[i].pWSKeys = JE_RAM_ALLOCATE_ARRAY(jeXForm3d, nNumKeys);
		if(pInfo[i].pWSKeys == NULL)
			goto CreateBoneKeyArray_Failure;
	}

	return(pInfo);

CreateBoneKeyArray_Failure:

	DestroyBoneKeyArray(&pInfo, nNumBones);
	return(NULL);
}

void StripNewLine(char* pString)
{
	assert(pString != NULL);

	while(*pString != 0)
	{
		if(*pString == '\n')
			*pString = 0;
		pString++;
	}
}

// Are two xforms the same?
jeBoolean jeXForm3d_Compare(const jeXForm3d* pM1, const jeXForm3d* pM2)
{
	jeVec3d v1, v2;

#define XFORM_COMPARE_TOLERANCE 0.001f

	jeXForm3d_GetLeft(pM1, &v1);
	jeXForm3d_GetLeft(pM2, &v2);

	if(jeVec3d_Compare(&v1, &v2, XFORM_COMPARE_TOLERANCE) == JE_FALSE)
		return(JE_FALSE);

	jeXForm3d_GetUp(pM1, &v1);
	jeXForm3d_GetUp(pM2, &v2);

	if(jeVec3d_Compare(&v1, &v2, XFORM_COMPARE_TOLERANCE) == JE_FALSE)
		return(JE_FALSE);

	jeXForm3d_GetIn(pM1, &v1);
	jeXForm3d_GetIn(pM2, &v2);

	if(jeVec3d_Compare(&v1, &v2, XFORM_COMPARE_TOLERANCE) == JE_FALSE)
		return(JE_FALSE);

	if(jeVec3d_Compare(&pM1->Translation, &pM2->Translation, XFORM_COMPARE_TOLERANCE) == JE_FALSE)
		return(JE_FALSE);

	return(JE_TRUE);
}

// TKArray cannot handle two keys with the same time, so keep incrementing the
// time by a little bit until an empty hole is found or reach the LimitDistance
jeBoolean KEYMotion_InsertEventNoDuplicateTime(jeMotion* pMotion, jeFloat tKey, const char* String, jeFloat LimitDistance)
{
	jeFloat TimeOffset;
	jeFloat KeyTime;
	jeFloat InsertKeyTime;
	const char* pEventString;

#define JE_TKA_TIME_TOLERANCE (0.00001f)
#define TIME_STEP_DISTANCE (JE_TKA_TIME_TOLERANCE * 10.0f)

	TimeOffset = 0.0f;
	while(TimeOffset < LimitDistance)
	{
		InsertKeyTime = tKey + TimeOffset;
		jeMotion_SetupEventIterator(pMotion, InsertKeyTime, InsertKeyTime + TIME_STEP_DISTANCE);
		if(jeMotion_GetNextEvent(pMotion, &KeyTime, &pEventString) == JE_FALSE)
		{
#pragma message("jeMotion_InsertEvent should take a const const*")
			return(jeMotion_InsertEvent(pMotion, InsertKeyTime, (char*)String));
		}

		TimeOffset += TIME_STEP_DISTANCE;
	}

	return(JE_FALSE);
}

ReturnCode MkMotion_DoMake(MkMotion_Options* options,MkUtil_Printf Printf)
{
	int i;
	char* pdot;
	jeBody* pBody;
	jeMotion* pMotion;
	jePath* pPath;
	jeVFile *VF=NULL;
	FILE* fp=NULL;
	ReturnCode retValue = RETURN_SUCCESS;
	int nVersion = 0;
	int j, k, Index, ParentIndex;
	int NumBones, NumFrames, FramesPerSecond;
	float SecondsPerFrame;
	char line[LINE_LENGTH];
	char name[LINE_LENGTH];
	jeXForm3d InvAttach, KeyMatrix, TmpMatrix;
	jeQuaternion Q;
	TopDownBody* pTDBody = NULL;
	int MotionRootIndex = JE_BODY_NO_PARENT_BONE;
	TDBodyHeritage BoneIsDescendent = TDBODY_IS_DESCENDENT; // default to all in the family
	jeXForm3d RootRotation;
	jeXForm3d euler;
	BoneKeyInfo* pKeyInfo = NULL;
	const BoneKeyInfo* pParentInfo;

	NumBones = 0;
	assert(options != NULL);

	if(options->BodyFile[0] == 0)
	{
		Printf("ERROR: No body file specified\n");
		MkUtil_AdjustReturnCode(&retValue, RETURN_ERROR);
		return retValue;
	}

	if(options->KeyFile[0] == 0)
	{
		Printf("ERROR: No key file specified\n");
		MkUtil_AdjustReturnCode(&retValue, RETURN_ERROR);
		return retValue;
	}

	// who knows how many times this will get used in here
	jeXForm3d_SetEulerAngles(&euler, &options->EulerAngles);

	// Here is the official fclose to use
#define FCLOSE(f) { fclose(f); f = NULL; }

	// Read the body file
	
	VF = jeVFile_OpenNewSystem(NULL,JE_VFILE_TYPE_DOS,options->BodyFile,NULL,JE_VFILE_OPEN_READONLY);
		
	if(VF == NULL)
	{
		Printf("ERROR: Could not open '%s' body file\n", options->BodyFile);
		MkUtil_AdjustReturnCode(&retValue, RETURN_ERROR);
		return retValue;
	}
	else
	{
		pBody = jeBody_CreateFromFile(VF);
		if(pBody == NULL)
		{
			Printf("ERROR: Could not create body from file '%s'\n", options->BodyFile);
			MkUtil_AdjustReturnCode(&retValue, RETURN_ERROR);
			jeVFile_Close(VF);
			VF = NULL;
			return retValue;
		}
		jeVFile_Close(VF);
		VF = NULL;
	}

	// Create the Top Down Hierarchy if needed
	if(jeStrBlock_GetCount(options->pMotionRoots) > 0)
	{
		pTDBody = TopDownBody_CreateFromBody(pBody);
		if(pTDBody == NULL)
		{
			Printf("ERROR: Could not create top down hierarchy from body.\n");
			MkUtil_AdjustReturnCode(&retValue, RETURN_ERROR);
			jeBody_Destroy(&pBody);
			return retValue;
		}
	}

	// Options permit a rotation applied to the root.  Convert the Euler angles
	// to a transform for easier use later.
	jeXForm3d_SetEulerAngles(&RootRotation, &options->RootEulerAngles);

	pMotion = jeMotion_Create(JE_TRUE);
	if(pMotion == NULL)
	{
		Printf("ERROR: Could not create motion\n");
		MkUtil_AdjustReturnCode(&retValue, RETURN_ERROR);
		goto DoMake_Cleanup;
	}

	if(options->pMotionName != NULL)
		jeMotion_SetName(pMotion, options->pMotionName);

	// Read key data
	fp = fopen(options->KeyFile, "rt");
	if(fp == NULL)
	{
		Printf("ERROR: Could not open '%s' key file\n", options->KeyFile);
		MkUtil_AdjustReturnCode(&retValue, RETURN_ERROR);
		jeMotion_Destroy(&pMotion);
		goto DoMake_Cleanup;
	}

#define FGETS_LINE_OR_QUIT(s)												\
	if(fgets(s, LINE_LENGTH, fp) == NULL)										\
	{																			\
		Printf("ERROR: Could not read from '%s' key file\n", options->KeyFile);	\
		MkUtil_AdjustReturnCode(&retValue, RETURN_ERROR);								\
		goto DoMake_Cleanup;													\
	}

	// Read and check version
	FGETS_LINE_OR_QUIT(line);
#define FILETYPE_LENGTH 7
	if(strncmp(line, "KEYEXP ", FILETYPE_LENGTH) != 0)
	{
		Printf("ERROR: '%s' is not a KEY file\n", options->KeyFile);
		MkUtil_AdjustReturnCode(&retValue, RETURN_ERROR);
		goto DoMake_Cleanup;
	}

	// check version numbers
	StripNewLine(line);
	if(strcmp(line + FILETYPE_LENGTH, "1.0") == 0)
	{
		nVersion = 0x0100;
	}
	else if(strcmp(line + FILETYPE_LENGTH, "2.0") == 0)
	{
		nVersion = 0x0200;
	}
	else if(strcmp(line + FILETYPE_LENGTH, "2.1") == 0)
	{
		nVersion = 0x0210; // added notetracks
	}
	else
	{
		Printf("ERROR: '%s' KEY file version \"%s\" is not supported\n", options->KeyFile, line + FILETYPE_LENGTH);
		MkUtil_AdjustReturnCode(&retValue, RETURN_ERROR);
		goto DoMake_Cleanup;
	}
	assert(nVersion != 0);

	// Read number of bones
	FGETS_LINE_OR_QUIT(line);
	sscanf(line, "Number of Bones = %d", &NumBones);

	// Read "Key Data"
	FGETS_LINE_OR_QUIT(line);
	if(strcmp(line, "Key Data\n") != 0)
	{
		Printf("ERROR: '%s' key file does not contain \"Key Data\"\n", options->KeyFile);
		MkUtil_AdjustReturnCode(&retValue, RETURN_ERROR);
		goto DoMake_Cleanup;
	}

	// Read number of frames
	FGETS_LINE_OR_QUIT(line);
	sscanf(line, "%d %d %d", &j, &NumFrames, &FramesPerSecond);
	NumFrames -= j;
	NumFrames++;

	SecondsPerFrame = 1.0f / (float)FramesPerSecond;

	// Allocate key data
	pKeyInfo = CreateBoneKeyArray(NumBones, NumFrames);
	if(pKeyInfo == NULL)
	{
		Printf("ERROR: Could not create key data\n");
		MkUtil_AdjustReturnCode(&retValue, RETURN_ERROR);
		goto DoMake_Cleanup;
	}

	// Read in key data
	for(k=0;k<NumBones;k++)
	{

		if (MkUtil_Interrupt())
			{
				Printf("Interrupted\n");
				MkUtil_AdjustReturnCode(&retValue, RETURN_ERROR);
				goto DoMake_Cleanup;
			}
		

		// Read bone name
		FGETS_LINE_OR_QUIT(line);
		strcpy(pKeyInfo[k].name, line + strlen("Node: "));
		
		StripNewLine(pKeyInfo[k].name);

		if(options->Capitalize != MK_FALSE)
			strupr(pKeyInfo[k].name);

		// Notes appear here
		if(nVersion >= 0x0210)
		{
			int NumNotes;

			// Read number of notes
			FGETS_LINE_OR_QUIT(line);
			sscanf(line, "Number of Notes = %d", &NumNotes);

			for(i=0;i<NumNotes;i++)
			{
				FGETS_LINE_OR_QUIT(line);

				// Strip the \n from the line
				StripNewLine(line);

				if(jeStrBlock_Append(&pKeyInfo[k].pEvents, line) == JE_FALSE)
				{
					Printf("ERROR: Could not create key data\n");
					MkUtil_AdjustReturnCode(&retValue, RETURN_ERROR);
					goto DoMake_Cleanup;
				}
			}
		}

		// Read the key data
		for(j=0;j<NumFrames;j++)
		{
			// Read raw key matrix
			FGETS_LINE_OR_QUIT(line);
			sscanf(line, "%f, %f, %f", &pKeyInfo[k].pMatrixKeys[j].AX, &pKeyInfo[k].pMatrixKeys[j].AY, &pKeyInfo[k].pMatrixKeys[j].AZ);
			FGETS_LINE_OR_QUIT(line);
			sscanf(line, "%f, %f, %f", &pKeyInfo[k].pMatrixKeys[j].BX, &pKeyInfo[k].pMatrixKeys[j].BY, &pKeyInfo[k].pMatrixKeys[j].BZ);
			FGETS_LINE_OR_QUIT(line);
			sscanf(line, "%f, %f, %f", &pKeyInfo[k].pMatrixKeys[j].CX, &pKeyInfo[k].pMatrixKeys[j].CY, &pKeyInfo[k].pMatrixKeys[j].CZ);
			FGETS_LINE_OR_QUIT(line);
			sscanf(line, "%f, %f, %f", &pKeyInfo[k].pMatrixKeys[j].Translation.X, &pKeyInfo[k].pMatrixKeys[j].Translation.Y, &pKeyInfo[k].pMatrixKeys[j].Translation.Z);
		}
	}

	FCLOSE(fp);

	// Now, go thru all the keys and generate the world space keys
	// We make the (safe) assumption that the parent bone occurs earlier in the
	// list and its WS keys are already assigned.
	for(k=0;k<NumBones;k++)
	{
		if(jeBody_GetBoneByName(pBody, pKeyInfo[k].name, &Index, &InvAttach, &ParentIndex) == JE_FALSE)
		{
			int m;
			Printf("ERROR: Could not find bone '%s' in body '%s' for key file '%s'\n", pKeyInfo[k].name, options->BodyFile, options->KeyFile);
			Printf("Body has these bones:\n");
			for (m=0; m<NumBones; m++)
				{
					const char *Name;
					jeBody_GetBone(pBody,m,&Name,&InvAttach,&ParentIndex);
					Printf("\t#%d\t\t'%s'\n",name);
				}
			MkUtil_AdjustReturnCode(&retValue, RETURN_ERROR);
			goto DoMake_Cleanup;
		}

		if(ParentIndex != JE_BODY_NO_PARENT_BONE)
		{
			jeBody_GetBone(pBody, ParentIndex, &pdot, &InvAttach, &Index);
			pParentInfo = NULL;
			for(j=0;j<k;j++)
			{
				if(strcmp(pdot, pKeyInfo[j].name) == 0)
				{
					pParentInfo = pKeyInfo + j;
					break;
				}
			}
			if(j == k)
			{
				// didn't find it?!
				Printf("ERROR: Could not find bone '%s' in key array\n", pdot);
				MkUtil_AdjustReturnCode(&retValue, RETURN_ERROR);
				goto DoMake_Cleanup;
			}
			assert(pParentInfo != NULL);

			for(j=0;j<NumFrames;j++)
			{
#ifdef KEYS_ARE_WORLD_SPACE
				pKeyInfo[k].pWSKeys[j] = pKeyInfo[k].pMatrixKeys[j];
#else
				// Why do I have to swap the matrix order here?
//				jeXForm3d_Multiply(	pParentInfo->pWSKeys + j, 
//									pKeyInfo[k].pMatrixKeys + j, 
//									pKeyInfo[k].pWSKeys + j );
				MaxMath_Multiply(	pKeyInfo[k].pMatrixKeys + j, 
									pParentInfo->pWSKeys + j, 
									pKeyInfo[k].pWSKeys + j );
#endif
			}
		}
		else
		{
			for(j=0;j<NumFrames;j++)
			{
				pKeyInfo[k].pWSKeys[j] = pKeyInfo[k].pMatrixKeys[j];
			}
		}

	}

	// Rotate the world space matrix according to the options and recalculate the MatrixKeys.
	for(k=0;k<NumBones;k++)
	{
		if (MkUtil_Interrupt())
			{
				Printf("Interrupted\n");
				MkUtil_AdjustReturnCode(&retValue, RETURN_ERROR);
				goto DoMake_Cleanup;
			}
		
		if(jeBody_GetBoneByName(pBody, pKeyInfo[k].name, &Index, &InvAttach, &ParentIndex) == JE_FALSE)
		{
			Printf("ERROR: Could not find bone '%s' in body '%s' for key file '%s'\n", name, options->BodyFile, options->KeyFile);
			MkUtil_AdjustReturnCode(&retValue, RETURN_ERROR);
			goto DoMake_Cleanup;
		}

		// apply rotation to world space
		for(j=0;j<NumFrames;j++)
		{
			// this first one should be the correct one
			jeXForm3d_Multiply(&euler, pKeyInfo[k].pWSKeys + j, pKeyInfo[k].pWSKeys + j);
		}

		if(ParentIndex != JE_BODY_NO_PARENT_BONE)
		{
			jeBody_GetBone(pBody, ParentIndex, &pdot, &InvAttach, &Index);
			pParentInfo = NULL;
			for(j=0;j<k;j++)
			{
				if(strcmp(pdot, pKeyInfo[j].name) == 0)
				{
					pParentInfo = pKeyInfo + j;
					break;
				}
			}
			if(j == k)
			{
				// didn't find it?!
				Printf("ERROR: Could not find bone '%s' in key array\n", pdot);
				MkUtil_AdjustReturnCode(&retValue, RETURN_ERROR);
				goto DoMake_Cleanup;
			}
			assert(pParentInfo != NULL);

			for(j=0;j<NumFrames;j++)
			{
				// recalc pMatrixKeys
				MaxMath_InverseMultiply(	pKeyInfo[k].pWSKeys + j, 
											pParentInfo->pWSKeys + j,
											pKeyInfo[k].pMatrixKeys + j );
			}
		}
		else
		{
			for(j=0;j<NumFrames;j++)
			{
				pKeyInfo[k].pMatrixKeys[j] = pKeyInfo[k].pWSKeys[j];
			}
		}

	}

	// Finally, let's generate the paths and add them to the motion
	for(k=0;k<NumBones;k++)
	{
		jeVec3d FirstFrameOffset;
		if (MkUtil_Interrupt())
			{
				Printf("Interrupted\n");
				MkUtil_AdjustReturnCode(&retValue, RETURN_ERROR);
				goto DoMake_Cleanup;
			}
		
		// Read bone name and get the inverse of its attachment
		strcpy(name, pKeyInfo[k].name);

		if(jeBody_GetBoneByName(pBody, name, &Index, &InvAttach, &ParentIndex) == JE_FALSE)
		{
			Printf("ERROR: Could not find bone '%s' in body '%s' for key file '%s'\n", name, options->BodyFile, options->KeyFile);
			MkUtil_AdjustReturnCode(&retValue, RETURN_ERROR);
			goto DoMake_Cleanup;
		}

		// If we have specified a root for the motion, make sure this bone
		// is a descendent.
		j = jeStrBlock_GetCount(options->pMotionRoots);
		if(j > 0)
		{
			assert(pTDBody != NULL);

			while(j > 0)
			{
				j--;

				if(jeBody_GetBoneByName(pBody, jeStrBlock_GetString(options->pMotionRoots, j), &MotionRootIndex, &TmpMatrix, &ParentIndex) == JE_FALSE)
				{
					Printf("ERROR: Could not find bone '%s' in body '%s' for motion root\n", jeStrBlock_GetString(options->pMotionRoots, j), options->BodyFile);
					
					{
						int m;
						Printf("Body has these bones:\n");
						for (m=0; m<NumBones; m++)
							{
								const char *Name;
								jeBody_GetBone(pBody,m,&Name,&InvAttach,&ParentIndex);
								Printf("\t#%d\t\t'%s'\n",m,Name);
							}
					}

					MkUtil_AdjustReturnCode(&retValue, RETURN_ERROR);
					goto DoMake_Cleanup;
				}
				BoneIsDescendent = TopDownBody_IsDescendentOf(pTDBody, MotionRootIndex, Index);
				if(BoneIsDescendent == TDBODY_IS_DESCENDENT)
					break; // found it!
			}
		}
		else
		{
			// All in the family.
			BoneIsDescendent = TDBODY_IS_DESCENDENT;
		}

		{
			int NumNotes, note, noteframe;
			unsigned int linelen;
			jeFloat notetime;
			const char* notestring;

			NumNotes = jeStrBlock_GetCount(pKeyInfo[k].pEvents);

			for(note=0;note<NumNotes;note++)
			{
				// note is valid, so string should be
				strcpy(line, jeStrBlock_GetString(pKeyInfo[k].pEvents, note));
				noteframe = atoi(line);
				notestring = strchr(line, ':');
				// the color must be there with a space following it
				if( (notestring == NULL) || (notestring[1] != ' ') )
				{
					Printf("ERROR: Bad note key \"%s\" for bone '%s' for key file '%s'\n", line, name, options->KeyFile);
					MkUtil_AdjustReturnCode(&retValue, RETURN_ERROR);
					goto DoMake_Cleanup;
				}
				notestring += 2; // this jump past the space to, at the very least, a terminator

				if(options->EventBoneSeparator != '\0')
				{
					// base string length decisions on "line", not "notestring"
					linelen = (unsigned int)strlen(line);
					linelen++; // add one for separator character

					if( (LINE_LENGTH - linelen) < (strlen(name)))
					{
						// some string too long
						Printf("ERROR: Note \"%s\" is too long for bone '%s' for key file '%s'\n", notestring, name, options->KeyFile);
						MkUtil_AdjustReturnCode(&retValue, RETURN_ERROR);
						goto DoMake_Cleanup;
					}
					line[linelen - 1] = options->EventBoneSeparator;
					line[linelen] = '\0';
					strcat(line, name);
				}

				notetime = (float)noteframe * SecondsPerFrame + options->TimeOffset;
				// insert the note, but not past this frame
				if(KEYMotion_InsertEventNoDuplicateTime(pMotion, notetime, notestring, SecondsPerFrame) == JE_FALSE)
				{
					Printf("ERROR: Could not add note \"%s\" for bone '%s' for key file '%s'\n", notestring, name, options->KeyFile);
					MkUtil_AdjustReturnCode(&retValue, RETURN_ERROR);
					goto DoMake_Cleanup;
				}
			}
		}

		pPath = jePath_Create(JE_PATH_INTERPOLATE_HERMITE, JE_PATH_INTERPOLATE_SLERP, JE_FALSE);
		if (pPath == NULL)
			{
				Printf("ERROR: Unable to create a path for bone '%s' for key file '%s'\n",name, options->KeyFile);
				MkUtil_AdjustReturnCode(&retValue, RETURN_ERROR);
				goto DoMake_Cleanup;
			}
		if(BoneIsDescendent != TDBODY_IS_NOT_DESCENDENT)
		{
			
			if(jeMotion_AddPath(pMotion, pPath, name,  &Index) == JE_FALSE)
			{
				if (jeMotion_GetPathNamed(pMotion,name) != NULL)
					{
						Printf("ERROR: More than one bone named '%s' in key file '%s'\n", name, options->KeyFile);
					}
				else
					{
						Printf("ERROR: Could not add path for bone '%s' for key file '%s'\n", name, options->KeyFile);
					}
				MkUtil_AdjustReturnCode(&retValue, RETURN_ERROR);
				goto DoMake_Cleanup;
			}
			//pPath = jeMotion_GetPath(pMotion, Index);
		}

		// Read the key data, divide out the attachment (multiply by inverse),
		// and insert the keyframe
		for(j=0;j<NumFrames;j++)
		{
			KeyMatrix = pKeyInfo[k].pMatrixKeys[j];
			if (MkUtil_Interrupt())
				{
					Printf("Interrupted\n");
					MkUtil_AdjustReturnCode(&retValue, RETURN_ERROR);
					goto DoMake_Cleanup;
				}
		
			// rotation for root only from options
			if(ParentIndex == JE_BODY_NO_PARENT_BONE)
			{
				jeXForm3d_Multiply(&RootRotation, &KeyMatrix, &KeyMatrix);
			}
			
			// permit global move of motion
			if(ParentIndex == JE_BODY_NO_PARENT_BONE)
				jeXForm3d_Translate(&KeyMatrix, options->RootTranslation.X, options->RootTranslation.Y, options->RootTranslation.Z);

			// flip the rotation direction
			TmpMatrix.Translation = KeyMatrix.Translation;
			jeQuaternion_FromMatrix(&KeyMatrix, &Q);
			Q.W = -Q.W;
			jeQuaternion_ToMatrix(&Q, &KeyMatrix);
			KeyMatrix.Translation = TmpMatrix.Translation;

			jeXForm3d_GetTranspose(&InvAttach, &TmpMatrix);
			jeXForm3d_Multiply(&TmpMatrix, &KeyMatrix, &TmpMatrix);
			Q;
			FirstFrameOffset;

			jeXForm3d_Orthonormalize(&TmpMatrix);
			jePath_InsertKeyframe(	pPath,
									JE_PATH_ROTATION_CHANNEL | JE_PATH_TRANSLATION_CHANNEL,
									(float)j / (float)FramesPerSecond + options->TimeOffset,
									&TmpMatrix);
		}

		// First, destroy path
		//if(BoneIsDescendent == MK_FALSE)
		{
			jePath_Destroy(&pPath);
		}

		// Then, check for errors
		if(j != NumFrames)
		{
			// There must have been an error
			goto DoMake_Cleanup;
		}
	}
	if(k != NumBones)
	{
		// There must have been an error
		goto DoMake_Cleanup;
	}

	if (options->MotionFile[0] == 0)
		{
			// build motion filename: keyfile.key -> keyfile.mot
			strcpy(options->MotionFile,options->KeyFile);
			pdot = strrchr(options->MotionFile, '.');
			if(pdot == NULL)
			{
				pdot = options->MotionFile + strlen(options->MotionFile);
			}
			strcpy(pdot, ".mot");
		}

#if 0
	// Rename any existing file
	{
		char bakname[_MAX_PATH];

		strcpy(bakname, options->MotionFile);
		strcat(bakname, ".bak");
		remove(bakname);
		rename(options->MotionFile, bakname);
	}
#endif

	// Write the motion
	VF = jeVFile_OpenNewSystem(NULL,JE_VFILE_TYPE_DOS,options->MotionFile,NULL,JE_VFILE_OPEN_CREATE);
	//fp = fopen(options->MotionFile, "wt");
	if(VF == NULL)
	{
		Printf("ERROR: Could not create '%s' motion file\n", options->MotionFile);
		MkUtil_AdjustReturnCode(&retValue, RETURN_ERROR);
		unlink(options->MotionFile);
		goto DoMake_Cleanup;
	}
	else
	{
		if(jeMotion_WriteToFile(pMotion, VF) == JE_FALSE)
		{
			jeVFile_Close(VF);  VF = NULL;
			Printf("ERROR: Motion file '%s' was not written correctly\n", options->MotionFile);
			MkUtil_AdjustReturnCode(&retValue, RETURN_ERROR);
			unlink(options->MotionFile);
		}
		else
		{
			if (jeVFile_Close(VF)==JE_FALSE) 
				{
					Printf("ERROR: Motion file '%s' was not written correctly\n", options->MotionFile);
					MkUtil_AdjustReturnCode(&retValue, RETURN_ERROR);
					unlink(options->MotionFile);
				}
			VF = NULL;
			Printf("SUCCESS: Motion file '%s' written successfully\n", options->MotionFile);
		}
	}

DoMake_Cleanup:

	if (MkUtil_Interrupt())
		{
			Printf("Interrupted... Unable to make motion '%s'\n",options->MotionFile);
		}
	else
		if (retValue==RETURN_ERROR)
			{
				Printf("Error.  Unable to make motion '%s'\n",options->MotionFile);
			}
	if(pKeyInfo != NULL)
		DestroyBoneKeyArray(&pKeyInfo, NumBones);

	if(fp != NULL)
		FCLOSE(fp);

	if(VF != NULL)
		jeVFile_Close(VF);

	if(pTDBody != NULL)
		TopDownBody_Destroy(&pTDBody);

	if(pMotion != NULL)
		jeMotion_Destroy(&pMotion);

	if(pBody != NULL)
		jeBody_Destroy(&pBody);

	return retValue;
}

void MkMotion_OutputUsage(MkUtil_Printf Printf)
{
	//COLS: 0         1         2         3         4         5         6         7       | 8
	Printf("\n");
	Printf("Builds a motion file from a key info file from 3DSMax.\n");
	Printf("\n");
	Printf("MKMOTION [options] /B<bodyfile> /K<keyfile> [/C] [/E[<sepchar>]] [/N<name>] [/O]\n");
	Printf("         [/R<rootbone>] [/S] [/T<time>] [/M<motionfile>]\n");
	Printf("\n");
//	Printf("/Ax,y,z       Specifies Euler angle rotations to apply to the import.  This\n");
//	Printf("              rotation should have been duplicated in the specified body.\n");
//	Printf("              The rotations are applied in z-y-x order.\n");
//	Printf("/Ax,y,z       Specifies Euler angle rotations to apply to the root bone.  The\n");
//	Printf("              rotations are applied in z-y-x order.\n");
	Printf("/B<bodyfile>  Specifies body file.  A body is required.\n");
	Printf("/C            Capitalize all motion key names.\n");
	Printf("/E[<sepchar>] Attach bone name to event strings, optionally specifying the\n");
	Printf("              separation character (default is ';').\n");
	Printf("/K<keyfile>   Specifies key file for motion.  A key file is required.\n");
	Printf("/N<name>      Specifies the name for the motion.\n");
	Printf("/Px,y,z       Specifies a positional offset for the root bone.\n");
	Printf("/O            Forces translation keys to be relative to first frame.\n");
	Printf("/R<rootbone>  Specifies a root bone for the motion.  This option can appear\n");
	Printf("              more than once to specify multiple roots.\n");
	Printf("/S            Removes all translation keys.\n");
	Printf("/T<time>      Specifies a time offset to apply to motions.\n");
	Printf("/M<motionfile>  Specifies output motion file name.  if not specified\n");
	Printf("                output will be in <keyfile prefix>.mot\n");
	Printf("\n");
	Printf("Any existing motion file will be renamed to motionfile.bak\n");
}

MkMotion_Options* MkMotion_OptionsCreate()
{
	MkMotion_Options* pOptions;

	pOptions = JE_RAM_ALLOCATE_STRUCT(MkMotion_Options);
	if(pOptions != NULL)
	{
		*pOptions = DefaultOptions;

		pOptions->pMotionRoots = jeStrBlock_Create();
		if(pOptions->pMotionRoots == NULL)
		{
			jeRam_Free(pOptions);
		}
	}

	return pOptions;
}

void MkMotion_OptionsDestroy(MkMotion_Options** ppOptions)
{
	MkMotion_Options* p;

	assert(ppOptions != NULL);
	assert(*ppOptions != NULL);

	p = *ppOptions;

	jeStrBlock_Destroy(&p->pMotionRoots);

	if(p->pMotionName != NULL)
		jeRam_Free(p->pMotionName);

	jeRam_Free(*ppOptions);

	*ppOptions = NULL;
}

ReturnCode MkMotion_ParseOptionString(MkMotion_Options* options, 
									const char* string, MK_Boolean InScript,
									MkUtil_Printf Printf)
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
/*
			// An option without a letter
			{
				char* ptext1;
				char* ptext2;

				options->EulerAngles.X = (float)strtod(string + 2, &ptext1);
				if( (ptext1 == string + 2) || (*ptext1 != ',') )
					goto EulerAngleError;

				ptext1++; // skip ','
				options->EulerAngles.Y = (float)strtod(ptext1, &ptext2);
				if( (ptext2 == ptext1) || (*ptext2 != ',') )
					goto EulerAngleError;

				ptext2++; // skip ','
				options->EulerAngles.Z = (float)strtod(ptext2, &ptext1);
				if(ptext1 == ptext2)
					goto EulerAngleError;

				// convert from degrees to radians
				options->EulerAngles.X *= (MK_PI / 180.0f);
				options->EulerAngles.Y *= (MK_PI / 180.0f);
				options->EulerAngles.Z *= (MK_PI / 180.0f);
				break;

EulerAngleError:
				retValue = RETURN_ERROR;
				Printf("ERROR: Could not convert Euler angles\n");
			}
			break;
*/
/* With recent change, this option no longer works and is no longer needed.
		case 'a':
		case 'A':
			{
				char* ptext1;
				char* ptext2;

				options->RootEulerAngles.X = (float)strtod(string + 2, &ptext1);
				if( (ptext1 == string + 2) || (*ptext1 != ',') )
					goto RootEulerAngleError;

				ptext1++; // skip ','
				options->RootEulerAngles.Y = (float)strtod(ptext1, &ptext2);
				if( (ptext2 == ptext1) || (*ptext2 != ',') )
					goto RootEulerAngleError;

				ptext2++; // skip ','
				options->RootEulerAngles.Z = (float)strtod(ptext2, &ptext1);
				if(ptext1 == ptext2)
					goto RootEulerAngleError;

				// convert from degrees to radians
				options->RootEulerAngles.X *= (MK_PI / 180.0f);
				options->RootEulerAngles.Y *= (MK_PI / 180.0f);
				options->RootEulerAngles.Z *= (MK_PI / 180.0f);
				break;

RootEulerAngleError:
				retValue = RETURN_ERROR;
				Printf("ERROR: Could not convert Root Euler angles\n");
			}
*/
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

		case 'c':
		case 'C':
			options->Capitalize = MK_TRUE;
			break;

		case 'e':
		case 'E':
			if(string[2] == 0)
				options->EventBoneSeparator = ';';
			else
				options->EventBoneSeparator = string[2];
			break;

		case 'k':
		case 'K':
			if(string[2] == 0)
			{
				NO_FILENAME_WARNING;
				retValue = RETURN_WARNING;
			}
			else
			{
				if( (InScript != MK_FALSE) && (options->KeyFile[0] != 0) )
				{
					Printf("WARNING: Key filename in script ignored\n");
					retValue = RETURN_WARNING;
				}
				else
				{
					strcpy(options->KeyFile, string + 2);
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
				if( (InScript != MK_FALSE) && (options->MotionFile[0] != 0) )
				{
					Printf("WARNING: Motion filename in script ignored\n");
					retValue = RETURN_WARNING;
				}
				else
				{
					strcpy(options->MotionFile, string + 2);
				}
			}
			break;

		case 'n':
		case 'N':
			if(strlen(string + 2) > 0)
			{
				options->pMotionName = (char*)jeRam_Allocate(strlen(string + 2) + 1);
				if(options->pMotionName == NULL)
				{
					Printf("ERROR: Could not allocate motion name\n");
					retValue = RETURN_ERROR;
				}
				else
				{
					strcpy(options->pMotionName, string + 2);
				}
			}
			break;

		case 'o':
		case 'O':
			options->Originate = MK_TRUE;
			break;

		case 'p':
		case 'P':
			{
				char* ptext1;
				char* ptext2;

				options->RootTranslation.X = (float)strtod(string + 2, &ptext1);
				if( (ptext1 == string + 2) || (*ptext1 != ',') )
					goto PositionError;

				ptext1++; // skip ','
				options->RootTranslation.Y = (float)strtod(ptext1, &ptext2);
				if( (ptext2 == ptext1) || (*ptext2 != ',') )
					goto PositionError;

				ptext2++; // skip ','
				options->RootTranslation.Z = (float)strtod(ptext2, &ptext1);
				if(ptext1 == ptext2)
					goto PositionError;

				break;

PositionError:
				retValue = RETURN_ERROR;
				Printf("ERROR: Could not convert position offset\n");
			}
			break;

		case 'r':
		case 'R':
			if(string[2] == 0)
			{
				NO_FILENAME_WARNING;
				retValue = RETURN_WARNING;
			}
			else
			{
				if(jeStrBlock_FindString(options->pMotionRoots, string + 2, &Index) == MK_FALSE)
				{
					if(jeStrBlock_Append(&options->pMotionRoots, string + 2) == MK_FALSE)
					{
						Printf("ERROR: Could not add \"%s\" root to string block\n", string + 2);
						retValue = RETURN_ERROR;
					}
				}
				else
				{
					Printf("WARNING: Duplicate root \"%s\" ignored\n", string + 2);
					retValue = RETURN_WARNING;
				}
			}
			break;

		case 's':
		case 'S':
			options->KeepStationary = MK_TRUE;
			break;

		case 't':
		case 'T':
			options->TimeOffset = (float)atof(string + 2);
			break;

		default:
			retValue = RETURN_NOACTION;
		}
	}

	return retValue;
}
