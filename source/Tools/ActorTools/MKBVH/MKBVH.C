/****************************************************************************************/
/*  MKBVH.C	                                                                            */
/*                                                                                      */
/*  Author:             	                                                            */
/*  Description:                                                						*/
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
// RTL includes
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Jet includes
#include "body.h"
#include "motion.h"
#include "quatern.h"
#include "ram.h"

// Local includes
#include "mkutil.h"
#include "quatern2.h"
#include "tdbody.h"
#include "mkbvh.h"

// define if only root bone should have translations
#define ROOT_XLATE_ONLY

typedef struct MkBVH_Options
{
	char BodyFile[_MAX_PATH];
	char MotionFile[_MAX_PATH];
	char BVHFile[_MAX_PATH];
	float FramesPerSecond;
} MkBVH_Options;

const MkBVH_Options DefaultOptions =
{
	"",
	"",
	"",
	30.0f,
};

// Thanks to Taylor Wilson for this bit of knowledge.  BVH requires the Y axis be up.
static void convertVectorYUp( jeVec3d *v )
{
   // Converts the given vector in place from z-up to y-up

   float temp = v->Z;
   v->Z = -v->Y;
   v->Y = temp;
}

static void StripWhiteSpaceNCopy(char* pDest, const char* pSrc, int Count)
{
	int i;
	
	assert(pDest != NULL);
	assert(pSrc != NULL);

	for(i=0;i<Count;i++)
	{
		if( (*pSrc != ' ') && (*pSrc != '\t') )
		{
			*pDest = *pSrc;
			pDest++;
		}

		// Permit copy of terminator, so check for break second
		if(*pSrc == '\0')
			break;

		pSrc++;
	}
}

static int FPutTabs(FILE* fp, int Count)
{
	int i;

	assert(fp != NULL);
	assert(Count >= 0);

	for(i=0;i<Count;i++)
	{
		if(fputc('\t', fp) != (int)'\t')
			return(i);
	}

	return(i);
}

static void GetConcatenatedMatrix(const jeBody* pBody, int Index, jeXForm3d* pMatrix)
{
	int ParentIndex;
	const char* pName;
	jeXForm3d BoneMatrix;

	assert(pBody != NULL);
	assert(jeBody_IsValid(pBody) != JE_FALSE);
	assert(pMatrix != NULL);

	jeBody_GetBone(pBody, Index, &pName, &BoneMatrix, &ParentIndex);

	if(ParentIndex != JE_BODY_NO_PARENT_BONE)
	{
		GetConcatenatedMatrix(pBody, ParentIndex, pMatrix);
	}

	jeXForm3d_Multiply(pMatrix, &BoneMatrix, pMatrix);
}

static MK_Boolean WriteBVHEndSite(FILE* fp, int Depth, jeVec3d* offset)
{
	int D;

	assert(fp != NULL);
	assert(Depth >= 0);

	D = Depth;

	// "Root name" or "JOINT name" or "End Site"
	if(FPutTabs(fp, D) != D)
		return(MK_FALSE);

	if(fputs("End Site\n", fp) < 0)
		return(MK_FALSE);

	// Open brace
	if(FPutTabs(fp, D) != D)
		return(MK_FALSE);

	if(fputs("{\n", fp) < 0)
		return(MK_FALSE);

	// Everything tabs in at this point
	D++;

	// "OFFSET X Y Z"
	if(FPutTabs(fp, D) != D)
		return(MK_FALSE);

	convertVectorYUp( offset );

	if(fprintf(fp, "OFFSET\t%f\t%f\t%f\n", offset->X, offset->Y, offset->Z) == 0)
	{
		return(MK_FALSE);
	}

	// Back out one depth
	D--;

	// Close brace
	if(FPutTabs(fp, D) != D)
		return(MK_FALSE);

	if(fputs("}\n", fp) < 0)
		return(MK_FALSE);

	return(MK_TRUE);
}

static MK_Boolean WriteBVHNodeHierarchy(FILE* fp, TopDownBody* pTDNode, int Depth, jeBody* pBody, jeMotion* pMotion)
{
	int i;
	int RChanCount, TChanCount;
	const char* pName;
	jeXForm3d Matrix;
	int ParentIndex;
	jePath* pPath;
	jeVec3d v;
#define LINE_LENGTH 1024
	char line[LINE_LENGTH];
	int D;

	assert(fp != NULL);
	assert(pTDNode != NULL);
	assert(pBody != NULL);
	assert(jeBody_IsValid(pBody) != JE_FALSE);
	assert(pMotion != NULL);
	assert(Depth >= 0);

	D = Depth;

	jeBody_GetBone(pBody, pTDNode->BoneIndex, &pName, &Matrix, &ParentIndex);

	// Check motion for any channels and which types
	RChanCount = 0;
	TChanCount = 0;
	pPath = jeMotion_GetPathNamed(pMotion, pName);
	if(pPath != NULL)
	{
		RChanCount = jePath_GetKeyframeCount(pPath, JE_PATH_ROTATION_CHANNEL);
		TChanCount = jePath_GetKeyframeCount(pPath, JE_PATH_TRANSLATION_CHANNEL);
	}

	// "Root name" or "JOINT name"
	if(FPutTabs(fp, D) != D)
		return(MK_FALSE);

	// It would appear that bvh files cannot handle spaces in the bone names.
	StripWhiteSpaceNCopy(line, pName, LINE_LENGTH);

	// Roots and No-offspring nodes print different text... sucks don't it
	if(D == 0)
	{
		if(fputs("ROOT ", fp) < 0)
			return(MK_FALSE);

		if(fputs(line, fp) < 0)
			return(MK_FALSE);
	}
	else
	{
		if(fputs("JOINT ", fp) < 0)
			return(MK_FALSE);

		if(fputs(line, fp) < 0)
			return(MK_FALSE);
	}
	if(fputs("\n", fp) < 0)
		return(MK_FALSE);

	// Open brace
	if(FPutTabs(fp, D) != D)
		return(MK_FALSE);

	if(fputs("{\n", fp) < 0)
		return(MK_FALSE);

	// Everything tabs in at this point
	D++;

	// "OFFSET X Y Z"
	if(FPutTabs(fp, D) != D)
		return(MK_FALSE);

	// Need world-space translation from parent
	v = Matrix.Translation;
#if 0 // gather all parents' attachments
	jeXForm3d_SetIdentity(&Matrix);
	GetConcatenatedMatrix(pBody, pTDNode->BoneIndex, &Matrix);
	jeVec3d_Clear(&Matrix.Translation);
	jeXForm3d_Transform(&Matrix, &v, &v);
#else
#endif

	// Convert to y-up for output to BVH
	convertVectorYUp( &v );

	if(fprintf(fp, "OFFSET\t%f\t%f\t%f\n", v.X, v.Y, v.Z) == 0)
	{
		return(MK_FALSE);
	}

	// "CHANNELS # ? ? ? ? ? ?"
	// It looks like each the rotation path channel gives 3 bvh
	// channels (Z, X, Y), and the translation path channel gives
	// 3 bvh channels (X, Y, Z) and translation gets listed first.
	i = 0;
	if(RChanCount > 0)
		i += 3;
	if(TChanCount > 0)
		i += 3;
	if(i > 0)
	{
		if(FPutTabs(fp, D) != D)
			return(MK_FALSE);

		if(fprintf(fp, "CHANNELS %d", i) == 0)
			return(MK_FALSE);

		if( (TChanCount > 0) 
#ifdef ROOT_XLATE_ONLY
			&& (ParentIndex == JE_BODY_NO_PARENT_BONE)
#endif
			)
		{
			if(fputs(" Xposition Yposition Zposition", fp) < 0)
				return(MK_FALSE);
		}

		if(RChanCount > 0)
		{
			if(fputs(" Zrotation Yrotation Xrotation", fp) < 0)
				return(MK_FALSE);
		}

		if(fputs("\n", fp) < 0)
			return(MK_FALSE);
	}

	// Write out all children or an End Site
	if(pTDNode->NumChildren == 0)
	{
		// for lack of any better offset, use this one
		if(WriteBVHEndSite(fp, D, &v) == MK_FALSE)
			return(MK_FALSE);
	}
	else
	{
		for(i=0;i<pTDNode->NumChildren;i++)
		{
			assert(pTDNode->pChildren != NULL);

			if(WriteBVHNodeHierarchy(fp, pTDNode->pChildren + i, D, pBody, pMotion) == MK_FALSE)
				return(MK_FALSE);
		}
	}

	// Back out one depth
	D--;

	// Close brace
	if(FPutTabs(fp, D) != D)
		return(MK_FALSE);

	if(fputs("}\n", fp) < 0)
		return(MK_FALSE);

	return(MK_TRUE);
}

static MK_Boolean WriteBVHHierarchy(FILE* fp, TopDownBody* pTDBody, jeBody* pBody, jeMotion* pMotion)
{
	assert(fp != NULL);
	assert(pTDBody != NULL);
	assert(pBody != NULL);
	assert(jeBody_IsValid(pBody) != JE_FALSE);
	assert(pMotion != NULL);

	if(fputs("HIERARCHY\n", fp) < 0)
		return(MK_FALSE);

	if(WriteBVHNodeHierarchy(fp, pTDBody, 0, pBody, pMotion) == MK_FALSE)
		return(MK_FALSE);
}

static MK_Boolean WriteBVHNodeMotion(FILE* fp, jeFloat KeyTime, TopDownBody* pTDNode, jeBody* pBody, jeMotion* pMotion)
{
	int i;
	int RChanCount, TChanCount;
	const char* pName;
	jeXForm3d Matrix;
	int ParentIndex;
	jePath* pPath;
	jeQuaternion q, qAttach;
	jeVec3d v, r;

	assert(fp != NULL);
	assert(pTDNode != NULL);
	assert(pBody != NULL);
	assert(jeBody_IsValid(pBody) != JE_FALSE);
	assert(pMotion != NULL);

	jeBody_GetBone(pBody, pTDNode->BoneIndex, &pName, &Matrix, &ParentIndex);

	jeQuaternion_FromMatrix(&Matrix, &qAttach);

	// Check motion for any channels and which types
	RChanCount = 0;
	TChanCount = 0;
	pPath = jeMotion_GetPathNamed(pMotion, pName);
	if(pPath != NULL)
	{
		RChanCount = jePath_GetKeyframeCount(pPath, JE_PATH_ROTATION_CHANNEL);
		TChanCount = jePath_GetKeyframeCount(pPath, JE_PATH_TRANSLATION_CHANNEL);

		// Looks like End Sites do not get motions
		jePath_SampleChannels(pPath, KeyTime, &q, &v);
	}

	if( (TChanCount > 0) 
#ifdef ROOT_XLATE_ONLY
		&& (ParentIndex == JE_BODY_NO_PARENT_BONE)
#endif
		)
	{
		// motion keys must include the attachment
		jeVec3d_Add(&v, &Matrix.Translation, &v);

		convertVectorYUp( &v );

		// X Y Z
		if(fprintf(fp, "% .2f\t% .2f\t% .2f\t", v.X, v.Y, v.Z) == 0)
			return(MK_FALSE);
	}

	if(RChanCount > 0)
	{
		// motion keys must include the attachment
		jeQuaternion_Multiply(&q, &qAttach, &q);

		jeQuaternion_ToMatrix(&q, &Matrix);
		jeXForm3d_GetEulerAngles(&Matrix, &r);
		jeVec3d_Scale(&r, (180.0f / QUATERNION_PI), &r);

		// Convert to a y-up rotation vector.
		convertVectorYUp( &r );

		if(fprintf(fp, "% .2f\t% .2f\t% .2f\t", r.Z, r.Y, r.X) == 0)
			return(MK_FALSE);
	}

	// Write out all children
	for(i=0;i<pTDNode->NumChildren;i++)
	{
		assert(pTDNode->pChildren != NULL);

		if(WriteBVHNodeMotion(fp, KeyTime, pTDNode->pChildren + i, pBody, pMotion) == MK_FALSE)
			return(MK_FALSE);
	}

	return(MK_TRUE);
}

static MK_Boolean WriteBVHMotion(FILE* fp, float FramesPerSecond, TopDownBody* pTDBody, jeBody* pBody, jeMotion* pMotion)
{
	int i;
	jeFloat StartKeyTime, EndKeyTime;
	jeFloat KeyTime;
	float SecondsPerFrame;

	assert(FramesPerSecond > 0.00001);
	assert(fp != NULL);
	assert(pTDBody != NULL);
	assert(pBody != NULL);
	assert(jeBody_IsValid(pBody) != JE_FALSE);
	assert(pMotion != NULL);

	SecondsPerFrame = 1.0f / FramesPerSecond;

	if(jeMotion_GetTimeExtents(pMotion, &StartKeyTime, &EndKeyTime) == JE_FALSE)
		return(MK_FALSE);

	// Give a little threshold for reaching the end
	EndKeyTime -= 0.00001f;

	if(fputs("MOTION\n", fp) < 0)
		return(MK_FALSE);

	// Exactly how many frames will there be?
	i = 0;
	do
	{
		// avoid as much rounding error as possible
		KeyTime = StartKeyTime + (float)i * SecondsPerFrame;
		i++;
	}
	while(KeyTime < EndKeyTime);

	if(fprintf(fp, "Frames:     %d\n", i) <= 0)
		return(MK_FALSE);

	if(fprintf(fp, "Frame Time: %f\n", SecondsPerFrame) <= 0)
		return(MK_FALSE);

	i = 0;
	do
	{
		// avoid as much rounding error as possible
		KeyTime = StartKeyTime + (float)i * SecondsPerFrame;
		if(WriteBVHNodeMotion(fp, KeyTime, pTDBody, pBody, pMotion) == MK_FALSE)
			return(MK_FALSE);
		if(fputs("\n", fp) < 0)
			return(MK_FALSE);
		i++;
	}
	while(KeyTime < EndKeyTime);

	return(MK_TRUE);
}

ReturnCode MkBVH_DoMake(MkBVH_Options* options,MkUtil_Printf Printf)
{
	jeBody* pBody = NULL;
	jeMotion* pMotion = NULL;
	FILE* fp=NULL;
	jeVFile *VF=NULL;
	ReturnCode retValue = RETURN_SUCCESS;
	TopDownBody* pTDBody = NULL;

	// All files must be specified.
	if(options->BodyFile[0] == 0)
	{
		Printf("ERROR: No body file specified\n");
		MkUtil_AdjustReturnCode(&retValue, RETURN_ERROR);
		return retValue;
	}

	if(options->MotionFile[0] == 0)
	{
		Printf("ERROR: No motion file specified\n");
		MkUtil_AdjustReturnCode(&retValue, RETURN_ERROR);
		return retValue;
	}

	if(options->BVHFile[0] == 0)
	{
		Printf("ERROR: No BVH file specified\n");
		MkUtil_AdjustReturnCode(&retValue, RETURN_ERROR);
		return retValue;
	}

	// Read the body file
	VF = jeVFile_OpenNewSystem(NULL,JE_VFILE_TYPE_DOS,options->BodyFile,NULL,JE_VFILE_OPEN_READONLY);
	//fp = fopen(options->BodyFile, "rt");
	if(VF == NULL)
	{
		Printf("ERROR: Could not open %s body file\n", options->BodyFile);
		MkUtil_AdjustReturnCode(&retValue, RETURN_ERROR);
		return retValue;
	}
	else
	{
		pBody = jeBody_CreateFromFile(VF);
		jeVFile_Close(VF);
		VF = NULL;
		if(pBody == NULL)
		{
			Printf("ERROR: Could not create body from file %s\n", options->BodyFile);
			MkUtil_AdjustReturnCode(&retValue, RETURN_ERROR);
			goto DOMAKE_CLEAN;
		}
	}

	// Read the motion file
	VF = jeVFile_OpenNewSystem(NULL,JE_VFILE_TYPE_DOS,options->MotionFile,NULL,JE_VFILE_OPEN_READONLY);
	//fp = fopen(options->MotionFile, "rt");
	if(VF == NULL)
	{
		Printf("ERROR: Could not open %s motion file\n", options->MotionFile);
		MkUtil_AdjustReturnCode(&retValue, RETURN_ERROR);
		return retValue;
	}
	else
	{
		pMotion = jeMotion_CreateFromFile(VF);
		jeVFile_Close(VF);
		VF = NULL;
		if(pMotion == NULL)
		{
			Printf("ERROR: Could not create motion from file %s\n", options->MotionFile);
			MkUtil_AdjustReturnCode(&retValue, RETURN_ERROR);
			goto DOMAKE_CLEAN;
		}
	}

	// Create the Top Down Hierarchy
	pTDBody = TopDownBody_CreateFromBody(pBody);
	if(pTDBody == NULL)
	{
		Printf("ERROR: Could not create top down hierarchy from body.\n");
		MkUtil_AdjustReturnCode(&retValue, RETURN_ERROR);
		goto DOMAKE_CLEAN;
	}

	// Rename any existing file
	{
		char bakname[_MAX_PATH];

		strcpy(bakname, options->BVHFile);
		strcat(bakname, ".bak");
		remove(bakname);
		rename(options->BVHFile, bakname);
	}

	// Open BVH file
	fp = fopen(options->BVHFile, "wt");
	if(fp == NULL)
	{
		Printf("ERROR: Failed to create BVH file %s.\n", options->BVHFile);
		MkUtil_AdjustReturnCode(&retValue, RETURN_ERROR);
		goto DOMAKE_CLEAN;
	}
	
	// Write the Hierarchy
	if(WriteBVHHierarchy(fp, pTDBody, pBody, pMotion) == MK_FALSE)
	{
		// some error
		Printf("ERROR: Failed to write BVH file %s.\n", options->BVHFile);
		MkUtil_AdjustReturnCode(&retValue, RETURN_ERROR);
		goto DOMAKE_CLEAN;
	}

	// Write the Motion
	if(WriteBVHMotion(fp, options->FramesPerSecond, pTDBody, pBody, pMotion) == MK_FALSE)
	{
		// some error
		Printf("ERROR: Failed to write BVH file %s.\n", options->BVHFile);
		MkUtil_AdjustReturnCode(&retValue, RETURN_ERROR);
		goto DOMAKE_CLEAN;
	}

	// Close the BVH file
	fclose(fp);
	fp = NULL;

DOMAKE_CLEAN:

	if(fp != NULL)
		fclose(fp);
	if(VF != NULL)
		jeVFile_Close(VF);
	if(pTDBody != NULL)
		TopDownBody_Destroy(&pTDBody);
	if(pMotion != NULL)
		jeMotion_Destroy(&pMotion);
	if(pBody != NULL)
		jeBody_Destroy(&pBody);

	if(retValue == RETURN_SUCCESS)
	{
		Printf("SUCCESS: BVH file %s written successfully\n", options->BVHFile);
	}

	return retValue;
}

void MkBVH_OutputUsage(MkUtil_Printf Printf)
{
	//COLS: 0         1         2         3         4         5         6         7       | 8
	Printf("\n");
	Printf("Builds a BVH file from a body and motion file.\n");
	Printf("\n");
	Printf("MKBVH [options] /B<bodyfile> /M<motionfile> /V<bvhfile>\n");
	Printf("\n");
	Printf("/B<bodyfile>    Specifies body file.  A body is required.\n");
	Printf("/M<motionfile>  Specifies motion file.  A motion is required.\n");
	Printf("/V<bvhfile>     Specifies output filename.  A filename is required.\n");
	Printf("\n");
	Printf("Any existing BVH file will be renamed to bvhfile.bak\n");
}

MkBVH_Options* MkBVH_OptionsCreate()
{
	MkBVH_Options* pOptions;

	pOptions = JE_RAM_ALLOCATE_STRUCT(MkBVH_Options);
	if(pOptions != NULL)
	{
		*pOptions = DefaultOptions;
	}

	return pOptions;
}

void MkBVH_OptionsDestroy(MkBVH_Options** ppOptions)
{
	MkBVH_Options* p;

	assert(ppOptions != NULL);
	assert(*ppOptions != NULL);

	p = *ppOptions;

	jeRam_Free(*ppOptions);

	*ppOptions = NULL;
}

ReturnCode MkBVH_ParseOptionString(MkBVH_Options* options, 
						const char* string, MK_Boolean InScript,
						MkUtil_Printf Printf)
{							
	ReturnCode retValue = RETURN_SUCCESS;

	assert(options != NULL);
	assert(string != NULL);

#define NO_FILENAME_WARNING Printf("WARNING: %s specified with no filename\n", string)

	if( (string[0] == '-') || (string[0] == '/') )
	{
		switch(string[1])
		{
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

		case 'r':
		case 'R':
			{
				char* pEnd;
				options->FramesPerSecond = (float)strtod(string + 2, &pEnd);
				if(pEnd == string + 2)
				{
					Printf("ERROR: Frames per second not understood\n");
					retValue = RETURN_ERROR;
				}
			}
			break;

		case 'v':
		case 'V':
			if(string[2] == 0)
			{
				NO_FILENAME_WARNING;
				retValue = RETURN_WARNING;
			}
			else
			{
				if( (InScript != MK_FALSE) && (options->BVHFile[0] != 0) )
				{
					Printf("WARNING: BVH filename in script ignored\n");
					retValue = RETURN_WARNING;
				}
				else
				{
					strcpy(options->BVHFile, string + 2);
				}
			}
			break;

		default:
			retValue = RETURN_NOACTION;
		}
	}

	return retValue;
}
