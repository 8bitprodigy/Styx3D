/****************************************************************************************/
/*  MKBODY.C	                                                                        */
/*                                                                                      */
/*  Author: Stephen Balkum	                                                            */
/*  Description: Body construction from MAX export and textures.						*/
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
#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "body.h"
#include "quatern.h"
#include "ram.h"
#include "strblock.h"
#include "vph.h"
#include "mkbody.h"
#include "bitmap.h"
#include "jeResource.h"
#include "jeMaterial.h"

#include "maxmath.h"

#include "jeEngine.h"

float VERT_EQUALITY_TOLERANCE = 0.005f; // obtained empirically
int fCount =0;		// face count for display


#define MK_PI 3.141592654f

#define LINE_LENGTH 1000 // maximum characters read in a line of a text file

typedef struct MkBody_Options
{
	char TexturePath[_MAX_PATH];
	char NFOFile[_MAX_PATH];
	char VPHFile[_MAX_PATH];
	MK_Boolean Capitalize;
	MK_Boolean WriteTextVPH;
	MK_Boolean RotationInBody;
	char BodyFile[_MAX_PATH];
	jeVec3d EulerAngles;
	jeStrBlock *ExtraMaterials; 
	int nVersion; // [SLB 08-18-99] Easy way to pass along functions
} MkBody_Options;


// Use this to initialize the default settings
const static MkBody_Options DefaultOptions = 
{
	"",
	"",
	"",
	MK_FALSE,
	MK_FALSE,
	MK_FALSE,
	"",
	{ 0.0f, 0.0f, 0.0f},
	NULL
};

#define NAME_LENGTH 256

typedef struct
{
	char Name[NAME_LENGTH];
	int Index;
	int ParentID;
	jeXForm3d Matrix;
} BoneDetail;

typedef struct
{
	char Name[NAME_LENGTH];
	jeXForm3d NodeTM;
	jeQuaternion Q;
	jeVec3d S;
	jeVec3d T;
} NodeDetail;

typedef struct
{
	jeVec3d v;
	jeVec3d offsetv;
	float weight;
	int bone;
	int VertexGroup;
} V2VertexDetail;

typedef struct
{
	int FirstVertex;
	int NumVerts;
} V2VertexGroupDetail;

typedef struct
{
	float tu, tv;
	int NAN;
} V2TVertexDetail;

typedef struct
{
	int material;
	int smoothingGroup;
	int v[3];
	int tv[3];
} V2FaceDetail;

typedef struct 
{
	jeVec3d v;						// location
	jeVec3d n;						// normal
	float tu, tv;					// texture coords
	int bone;						// bone index
} VertexDetail;

typedef struct 
{
	int material;					// material index
	VertexDetail verts[3];			// vertex info
} FaceDetail;

typedef struct 
{
	int bone;						// bone for this vertex
	jeVec3d BSPoint;					// offset in bone space
	jeVec3d WSPoint;					// location in world space
} VPHVertex;

typedef struct
{
	jeXForm3d Matrix;
	char Name[NAME_LENGTH];
} VPHLink;

typedef struct
{
	int NumLinks;
	VPHLink* pLinks;
	int NumObjects;
	jeStrBlock* pSBObjectNames;
	int* NumVerts;
	VPHVertex** ppVerts;
} VPHData;

MkUtil_Printf Printf;

MK_Boolean FileStringSeek(char* pDest, const char* pSearchStr, FILE* fp)
{
	char* p;

	do
	{
		p = fgets(pDest, LINE_LENGTH, fp);
	}
	while( (strstr(pDest, pSearchStr) == NULL) && (p == pDest) );

	return (p == pDest) ? MK_TRUE : MK_FALSE;
}

void StripNewLine(char* pString)
{
	while(*pString != 0)
	{
		if(*pString == '\n')
			*pString = 0;
		pString++;
	}
}

void NameCopyNoNewline(char* pDest, const char* pSrc)
{
	strncpy(pDest, pSrc, NAME_LENGTH);

	StripNewLine(pDest);
}

void FreeVPHData(VPHData** ppVPHData)
{
	int j;
	VPHData* pVPHData;
	
	assert(ppVPHData != NULL);
	pVPHData = *ppVPHData;

	if(pVPHData != NULL)
	{
		jeStrBlock_Destroy(&pVPHData->pSBObjectNames);
		for(j=0;j<pVPHData->NumObjects;j++)
		{
			jeRam_Free(pVPHData->ppVerts[j]);
		}
		jeRam_Free(pVPHData->NumVerts);
		jeRam_Free(pVPHData->ppVerts);
		jeRam_Free(pVPHData->pLinks);

		jeRam_Free(*ppVPHData);
	}
}

ReturnCode ReadVPHData(MkBody_Options* options, FILE* fp, VPHData* pVPHData)
{
	ReturnCode retValue = RETURN_SUCCESS;
	VPHVertex* pVPHVerts = NULL;
	VPHVertex** ppVerts = NULL;
	VPHLink* pVPHLinks = NULL;
	char line[LINE_LENGTH];
	int NumLinks, NumObjects;
	jeStrBlock* pSBNames;
	int* pNumVerts = NULL;
	int i, j;
	char* ptext;

	assert(pVPHData != NULL);

	if(FileStringSeek(line, "Number of links", fp) == MK_FALSE)
	{
		Printf("ERROR: Could not find links in VPH file\n");
		MkUtil_AdjustReturnCode(&retValue, RETURN_ERROR);
		return retValue;
	}

	j = sscanf(line, "Number of links = %d\n", &NumLinks);
	if (j != 1)
		{
			Printf("ERROR: Could not read number of links from VPH file\n");
			Printf("Line from VPH file:\n%s\n",line);
			MkUtil_AdjustReturnCode(&retValue, RETURN_ERROR);
			return retValue;
		}
		
	pVPHLinks = JE_RAM_ALLOCATE_ARRAY(VPHLink, NumLinks);
	if(pVPHLinks == NULL)
	{
		Printf("ERROR: Could not allocate VPH links\n");
		MkUtil_AdjustReturnCode(&retValue, RETURN_ERROR);
		return retValue;
	}

	for(i=0;i<NumLinks;i++)
	{
		if(FileStringSeek(line, "NAME =", fp) == MK_FALSE)
		{
			Printf("ERROR: Could not find link %d in VPH file\n", i);
			MkUtil_AdjustReturnCode(&retValue, RETURN_ERROR);
			break;
		}

		NameCopyNoNewline(pVPHLinks[i].Name, strchr(line, '=') + 2);
		if(options->Capitalize != MK_FALSE)
			strupr(pVPHLinks[i].Name);

		if(FileStringSeek(line, "Matrix ZM", fp) == MK_FALSE)
		{
			Printf("ERROR: Could not find link %d's matrix in VPH file\n", i);
			MkUtil_AdjustReturnCode(&retValue, RETURN_ERROR);
			break;
		}

		j = fscanf(fp, "%f %f %f\n",	&pVPHLinks[i].Matrix.AX,
										&pVPHLinks[i].Matrix.AY,
										&pVPHLinks[i].Matrix.AZ);
		if(j != 3)
		{
			Printf("ERROR: Could not read link %d's matrix in VPH file\n", i);
			MkUtil_AdjustReturnCode(&retValue, RETURN_ERROR);
			break;
		}
		j = fscanf(fp, "%f %f %f\n",	&pVPHLinks[i].Matrix.BX,
										&pVPHLinks[i].Matrix.BY,
										&pVPHLinks[i].Matrix.BZ);
		if(j != 3)
		{
			Printf("ERROR: Could not read link %d's matrix in VPH file\n", i);
			MkUtil_AdjustReturnCode(&retValue, RETURN_ERROR);
			break;
		}
		j = fscanf(fp, "%f %f %f\n",	&pVPHLinks[i].Matrix.CX,
										&pVPHLinks[i].Matrix.CY,
										&pVPHLinks[i].Matrix.CZ);
		if(j != 3)
		{
			Printf("ERROR: Could not read link %d's matrix in VPH file\n", i);
			MkUtil_AdjustReturnCode(&retValue, RETURN_ERROR);
			break;
		}
		j = fscanf(fp, "%f %f %f\n",	&pVPHLinks[i].Matrix.Translation.X,
										&pVPHLinks[i].Matrix.Translation.Y,
										&pVPHLinks[i].Matrix.Translation.Z);
		if(j != 3)
		{
			Printf("ERROR: Could not read link %d's matrix in VPH file\n", i);
			MkUtil_AdjustReturnCode(&retValue, RETURN_ERROR);
			break;
		}
	}
	if(i != NumLinks)
	{
		// there must have been an error
		jeRam_Free(pVPHLinks);
		return retValue;
	}

	// setup for reading the verts
	if(FileStringSeek(line, "num_objects = ", fp) == MK_FALSE)
	{
		Printf("ERROR: Could not find links in VPH file\n");
		MkUtil_AdjustReturnCode(&retValue, RETURN_ERROR);
		jeRam_Free(pVPHLinks);
		return retValue;
	}

	if(sscanf(line, "num_objects = %d\n", &NumObjects) != 1)
	{
		Printf("ERROR: Could not read number of objects in VPH file\n");
		Printf("Line from VPH file:\n%s\n",line);
		MkUtil_AdjustReturnCode(&retValue, RETURN_ERROR);
		jeRam_Free(pVPHLinks);
		return retValue;
	}

	// setup for object names
	pSBNames = jeStrBlock_Create();
	if(pSBNames == NULL)
	{
		Printf("ERROR: Could not allocate VPH string block\n");
		MkUtil_AdjustReturnCode(&retValue, RETURN_ERROR);
		jeRam_Free(pVPHLinks);
		return retValue;
	}

	// Setup for parent link indexes
	pNumVerts = JE_RAM_ALLOCATE_ARRAY(int, NumObjects);
	if(pNumVerts == NULL)
	{
		Printf("ERROR: Could not allocate vert count data\n");
		MkUtil_AdjustReturnCode(&retValue, RETURN_ERROR);
		jeRam_Free(pVPHLinks);
		return retValue;
	}
	ppVerts = JE_RAM_ALLOCATE_ARRAY(VPHVertex*, NumObjects);
	if(ppVerts == NULL)
	{
		Printf("ERROR: Could not allocate vert list array\n");
		MkUtil_AdjustReturnCode(&retValue, RETURN_ERROR);
		jeRam_Free(pVPHLinks);
		jeRam_Free(pNumVerts);
		return retValue;
	}

	for(i=0;i<NumObjects;i++)
	{
		if(FileStringSeek(line, "NAME =", fp) == MK_FALSE)
		{
			Printf("ERROR: Could not find object %d in VPH file\n", i);
			MkUtil_AdjustReturnCode(&retValue, RETURN_ERROR);
			break;
		}

		StripNewLine(line);
		if(options->Capitalize != MK_FALSE)
			strupr(line);
		if(jeStrBlock_Append(&pSBNames, line + strlen("NAME = ")) == JE_FALSE)
		{
			Printf("ERROR: Could not append vph object %d string\n", i);
			MkUtil_AdjustReturnCode(&retValue, RETURN_ERROR);
			break;
		}

		if(FileStringSeek(line, "Number of vertices = ", fp) == MK_FALSE)
		{
			Printf("ERROR: Could not find object %d verts in VPH file\n", i);
			MkUtil_AdjustReturnCode(&retValue, RETURN_ERROR);
			break;
		}

		if(sscanf(line, "Number of vertices = %d", &pNumVerts[i]) != 1)
		{
			Printf("ERROR: Could not read number of verts in VPH file\n");
			Printf("Line from VPH file:\n%s\n",line);
			MkUtil_AdjustReturnCode(&retValue, RETURN_ERROR);
			break;
		}

		pVPHVerts = JE_RAM_ALLOCATE_ARRAY(VPHVertex, pNumVerts[i]);
		if(pVPHVerts == NULL)
		{
			Printf("ERROR: Could not allocate vert data\n");
			MkUtil_AdjustReturnCode(&retValue, RETURN_ERROR);
			break;
		}
		ppVerts[i] = pVPHVerts;

		for(j=0;j<pNumVerts[i];j++)
		{
			if(FileStringSeek(line, "rigid (", fp) == MK_FALSE)
			{
				Printf("ERROR: Could not read rigid vert %d\n", j);
				Printf("Line from VPH file:\n%s\n",line);
				MkUtil_AdjustReturnCode(&retValue, RETURN_ERROR);
				break;
			}

			if(sscanf(line, "  rigid (%d", &pVPHVerts[j].bone) != 1)
			{
				Printf("ERROR: Could not read rigid vert %d link\n", j);
				Printf("Line from VPH file:\n%s\n",line);
				MkUtil_AdjustReturnCode(&retValue, RETURN_ERROR);
				break;
			}

#pragma message ("need to test for rigid connections by looking at deform_link")
			/*
			// non-positive bone is a rigid bone and a good thing
			if(pVPHVerts[j].bone > 0)
			{
				Printf("ERROR: All links should be root or rigid\n");
				MkUtil_AdjustReturnCode(&retValue, RETURN_ERROR);
				break;
			}
			pVPHVerts[j].bone = -pVPHVerts[j].bone;
			*/

			ptext = strrchr(line, ')');
			if(ptext == NULL)
			{
				Printf("ERROR: Could not find rigid vert %d xyz\n", j);
				MkUtil_AdjustReturnCode(&retValue, RETURN_ERROR);
				break;
			}
			ptext++;

			if(sscanf(ptext, "%f %f %f", &pVPHVerts[j].BSPoint.X, &pVPHVerts[j].BSPoint.Y, &pVPHVerts[j].BSPoint.Z) != 3)
			{
				Printf("ERROR: Could not read rigid vert %d xyz\n", j);
				Printf("Line from VPH file:\n%s\n",line);
				MkUtil_AdjustReturnCode(&retValue, RETURN_ERROR);
				break;
			}
		}
		if(j != pNumVerts[i])
		{
			// there must have been an error
			break;
		}

	}
	if(i != NumObjects)
	{
		// there must have been an error
		jeStrBlock_Destroy(&pSBNames);
		for(j=0;j<i;j++)
		{
			jeRam_Free(ppVerts[j]);
		}
		jeRam_Free(pNumVerts);
		jeRam_Free(ppVerts);
		jeRam_Free(pVPHLinks);
		return retValue;
	}

	pVPHData->NumLinks = NumLinks;
	pVPHData->pLinks = pVPHLinks;
	pVPHData->NumObjects = NumObjects;
	pVPHData->pSBObjectNames = pSBNames;
	pVPHData->NumVerts = pNumVerts;
	pVPHData->ppVerts = ppVerts;

	return retValue;
}

MK_Boolean AddBones(MkBody_Options* options, jeBody* pBody, const int NumBones, BoneDetail* pBoneIDs, const int NumNodes, const NodeDetail* pNodes)
{
	int j, k;
	jeXForm3d matrix, parentmatrix;
	jeQuaternion gQ;
	jeVec3d v;

	for(k=0;k<NumBones;k++)
	{
		for(j=0;j<NumNodes;j++)
		{
			if(strcmp(pBoneIDs[k].Name, pNodes[j].Name) == 0)
				break;
		}
		if(j == NumNodes)
		{
			Printf("ERROR: Could not find '%s' node\n", pBoneIDs[k].Name);
			return MK_FALSE;
		}

		// want to strip the scale component
		jeQuaternion_ToMatrix(&pNodes[j].Q, &matrix);
		matrix.Translation = pNodes[j].T;

		if(pBoneIDs[k].ParentID != -1)
		{
			parentmatrix = pBoneIDs[pBoneIDs[k].ParentID].Matrix;
			/*
			for(j=0;j<NumNodes;j++)
			{
				if(strcmp(pBoneIDs[pBoneIDs[k].ParentID].Name, pNodes[j].Name) == 0)
					break;
			}
			if(j == NumNodes)
			{
				Printf("ERROR: Could not find '%s' node\n", pBoneIDs[pBoneIDs[k].ParentID].Name);
				return MK_FALSE;
			}

			// want to strip the scale component
			jeQuaternion_ToMatrix(&pNodes[j].Q, &parentmatrix);
			parentmatrix.Translation = pNodes[j].T;
			*/
		}
		else
		{
			jeXForm3d_SetIdentity(&parentmatrix);

			// Root is identity by definition
			//jeXForm3d_SetIdentity(&matrix);
		}

		// store world space matrix for later use
		pBoneIDs[k].Matrix = matrix;

		// divide out the parent
		MaxMath_InverseMultiply(&matrix, &parentmatrix, &matrix);

		// flip the rotation
		jeQuaternion_FromMatrix(&matrix, &gQ);
		gQ.W = -gQ.W;
		v = matrix.Translation;
		jeQuaternion_ToMatrix(&gQ, &matrix);
		matrix.Translation = v;
		jeXForm3d_Orthonormalize(&matrix);

		// To support bvh format, the body must not have a rotational
		// attachment.  This can be pushed into the motion.  Thus, the
		// attachment is just the local-space translation.
		if(options->RotationInBody == MK_FALSE)
			jeXForm3d_SetTranslation(&matrix, v.X, v.Y, v.Z);

		// get parent's index
		j = pBoneIDs[k].ParentID;
		if(j == -1)
			j = JE_BODY_NO_PARENT_BONE;
		else
			j = pBoneIDs[j].Index;

		// attach the bone
		if(jeBody_AddBone(pBody, j, pBoneIDs[k].Name, &matrix, &pBoneIDs[k].Index) == JE_FALSE)
		{
			Printf("ERROR: Bone '%s' not added\n", pBoneIDs[k].Name);
			return MK_FALSE;
		}
	}

	return MK_TRUE;
}

MK_Boolean CalculateWSVPHVerts(VPHData* pVPHData, const int NumNodes, const NodeDetail* pNodes)
{
	int i, j, k;
	VPHVertex* pVerts;
	jeXForm3d matrix, nodematrix;

	for(k=0;k<pVPHData->NumObjects;k++)
	{
		pVerts = pVPHData->ppVerts[k];

		for(j=0;j<pVPHData->NumVerts[k];j++)
		{
			if(pVerts[j].bone >= pVPHData->NumLinks)
			{
				Printf("ERROR: VPH link index out of range\n");
				return MK_FALSE;
			}

			matrix = pVPHData->pLinks[pVerts[j].bone].Matrix;

			for(i=0;i<NumNodes;i++)
			{
				if(strcmp(pNodes[i].Name, pVPHData->pLinks[pVerts[j].bone].Name) == 0)
				{
					jeQuaternion_ToMatrix(&pNodes[i].Q, &nodematrix);
					nodematrix.Translation = pNodes[i].T;
					break;
				}
			}
			if(i == NumNodes)
			{
				Printf("ERROR: Could not match link with node\n");
				return MK_FALSE;
			}

			jeXForm3d_Multiply(&matrix, &nodematrix, &matrix);

			jeXForm3d_Transform(&matrix, &pVerts[j].BSPoint, &pVerts[j].WSPoint);
		}
	}

	return MK_TRUE;
}

ReturnCode CalculateVertexNormal(int Index, int Group, int NumFaces, 
		V2FaceDetail* pFaces, V2VertexDetail* pVerts, V2VertexGroupDetail* pVertGroups,
		jeVec3d* pNormal)
{
	ReturnCode retCode = RETURN_SUCCESS;
	int j;
	int nNumNormals;
	int AnyNormals = 0;

	//-------------------------------------------------------------------
	// Calculate Vertex normals

	jeVec3d Normal;
	jeVec3d A, B;
	jeVec3d v={0.0f,0.0f,0.0f};
	jeVec3d ThisNormal={0.0f,0.0f,0.0f};

	// Average the normals for all common faces in the same smoothing group
	nNumNormals = 0;
	
	jeVec3d_Clear(&Normal);
	Normal.X = 1.0f;
	*pNormal = Normal;

	for(j=0;j<NumFaces;j++)
	{
		if (MkUtil_Interrupt())
			{
				Printf("Interrupted\n");
				MkUtil_AdjustReturnCode(&retCode, RETURN_ERROR);
				return(RETURN_ERROR);
			}

				
		if(pFaces[j].smoothingGroup == Group)
		{
			if( (pFaces[j].v[0] == Index) || 
				(pFaces[j].v[1] == Index) || 
				(pFaces[j].v[2] == Index) )
			{
				AnyNormals = 1;
				A = pVerts[ pVertGroups[ pFaces[j].v[2] ].FirstVertex ].v;
				B = pVerts[ pVertGroups[ pFaces[j].v[0] ].FirstVertex ].v;
				v = pVerts[ pVertGroups[ pFaces[j].v[1] ].FirstVertex ].v;
				jeVec3d_Subtract(&A, &v, &A);
				jeVec3d_Subtract(&B, &v, &B);
				jeVec3d_CrossProduct(&A, &B, &ThisNormal);

				#if 0
				if(jeVec3d_Length(&ThisNormal) < VERT_EQUALITY_TOLERANCE)
					{
						MkUtil_AdjustReturnCode(&retCode, RETURN_WARNING);
						Printf("WARNING: degenerate face: %f %f %f\n", v.X, v.Y, v.Z);
						jeVec3d_Clear(&ThisNormal);
						ThisNormal.X = 1.0f;
					}
				else
				#endif
					{
						jeVec3d_Add(&Normal, &ThisNormal, &Normal);
						nNumNormals++;
					}
			}
		}
	}

	if(nNumNormals == 0) 
	{
		Printf("ERROR: There is a vertex not connected to a face.\n");
		MkUtil_AdjustReturnCode(&retCode, RETURN_ERROR);
		return(RETURN_ERROR);
	}

	if (jeVec3d_Normalize(&Normal)==0.0f)
		{
			// It is possible for a sum of normals to be zero.  For example, the corner
			// of a piece of paper.  Just pick the last found normal and spit out a
			// warning.
				MkUtil_AdjustReturnCode(&retCode, RETURN_WARNING);
				Printf("WARNING: Vertex normal calculated to zero: %f %f %f\n", v.X, v.Y, v.Z);
				Normal = ThisNormal;
		}

	if (jeVec3d_Normalize(&Normal)==0.0f)
		{
			MkUtil_AdjustReturnCode(&retCode, RETURN_WARNING);
			Printf("WARNING: Tried to find a non-zero normal from local geometry, but failed.\n");
			jeVec3d_Clear(&Normal);
			Normal.X = 1.0f;
		}

	*pNormal = Normal;

	return(retCode);
}

ReturnCode V2ReadVertices(MkBody_Options* options, FILE* fp, jeBody* pBody, char* name, int NumVertices, V2VertexDetail* pVerts, VPHData* pvphdata, NodeDetail* pNodes, int NumNodes)
{
	char line[LINE_LENGTH];
	char* ptext;
	int i, j, k;
	int n;
	jeXForm3d dummyMatrix;
	jeXForm3d xmatrix;
	jeXForm3d euler;

	assert(pVerts != NULL);

	jeXForm3d_SetEulerAngles(&euler, &options->EulerAngles);

	for(k=0;k<NumVertices;k++)
	{
		if (MkUtil_Interrupt())
			{
				Printf("Interrupted\n");
				return(RETURN_ERROR);
			}

		if(fgets(line, LINE_LENGTH, fp) == NULL)
		{
			Printf("ERROR: Could not read from '%s' NFO file\n", options->NFOFile);
			goto ReadVertexFailure;
		}

		// [SLB 08-18-99] With options->nVersion <= 0x0200, only rigid verts 
		// are allowed.  Default weight is 1.  VertexGroup equals k.
		pVerts[k].weight = 1;
		pVerts[k].VertexGroup = k;
		i = sscanf(line, "%f %f %f %f %f %f",	&pVerts[k].v.X,
												&pVerts[k].v.Y,
												&pVerts[k].v.Z,
												&pVerts[k].offsetv.X,
												&pVerts[k].offsetv.Y,
												&pVerts[k].offsetv.Z);
		if(i != 6)
		{
			Printf("ERROR: Could not read vertex for mesh '%s' in '%s' NFO file\n", name, options->NFOFile);
			Printf("Line from VPH file:\n%s\n",line);
			goto ReadVertexFailure;
		}

		// apply rotation to all data at read time
		jeXForm3d_Transform(&euler, &pVerts[k].v, &pVerts[k].v);
#pragma message ("vertex offsets not used now, but when they are used, they need to be rotated")

		// to get to the bone name, search for the 6th space
		ptext = line;
		for(i=0;i<6;i++)
		{
			ptext = strchr(ptext, ' ');
			if(ptext == NULL)
			{
				Printf("ERROR: Could not read bone vertex for mesh '%s' in '%s' NFO file\n", name, options->NFOFile);
				goto ReadVertexFailure;
			}
			ptext++;
		}
		StripNewLine(ptext);

		if(options->Capitalize != MK_FALSE)
			strupr(ptext);

		if(strcmp(ptext, "-1") == 0)
		{
			VPHVertex* pVPHVerts;
			if (pvphdata==NULL)
				{
					Printf("ERROR: VPH data needed (and not loaded) for mesh '%s'\n", name);
					Printf("--- possibly this mesh has not been physiqued:   '%s'\n", name);
					goto ReadVertexFailure;
				}
				
			if(jeStrBlock_FindString(pvphdata->pSBObjectNames, name, &j) == JE_FALSE)
			{
				Printf("ERROR: Could not find vph data for mesh '%s'\n", name);
				goto ReadVertexFailure;
			}

			pVPHVerts = pvphdata->ppVerts[j];
			j = pvphdata->NumVerts[j] - 1;
			while(j >= 0)
			{
				if(jeVec3d_Compare(&pVPHVerts[j].WSPoint, &pVerts[k].v, VERT_EQUALITY_TOLERANCE) != JE_FALSE)
					break; // found it!

				j--;
			}
			if(j < 0)
			{
				// didn't find vert
				Printf("ERROR: Could not find matching vertex\n");
				goto ReadVertexFailure;
			}

			for(n=0;n<NumNodes;n++)
			{
				if(strcmp(pNodes[n].Name, pvphdata->pLinks[pVPHVerts[j].bone].Name) == 0)
					break; // found it!
			}
			if(n == NumNodes)
			{
				// didn't find vert
				Printf("ERROR: (1) Could not find node '%s' for vertex\n", pvphdata->pLinks[pVPHVerts[j].bone].Name);
				goto ReadVertexFailure;
			}

			// copy bone name
			strcpy(ptext, pNodes[n].Name);
		}
		else
		{
			for(n=0;n<NumNodes;n++)
			{
				if(strcmp(pNodes[n].Name, ptext) == 0)
					break; // found it!
			}
			if(n == NumNodes)
			{
				// didn't find vert
				Printf("ERROR: (2) Could not find node '%s' for vertex\n", ptext);
				goto ReadVertexFailure;
			}
		}

		// gotta have a node at this point
		assert( (n >= 0) && (n < NumNodes) );

		if(jeBody_GetBoneByName(pBody, ptext, &i, &dummyMatrix, &j) == JE_FALSE)
		{
			Printf("ERROR: Could not find bone '%s' for mesh '%s' in '%s' NFO file\n", ptext, name, options->NFOFile);
			goto ReadVertexFailure;
		}
		pVerts[k].bone = i;

		// Need to do a NoScale
		jeQuaternion_ToMatrix(&pNodes[n].Q, &xmatrix);
		xmatrix.Translation = pNodes[n].T;

		MaxMath_GetInverse(&xmatrix, &xmatrix);

		// fixup the vert
		MaxMath_Transform(&xmatrix, &pVerts[k].v, &pVerts[k].offsetv);
	}

	return(RETURN_SUCCESS);

ReadVertexFailure:

	return(RETURN_ERROR);
}

// [SLB 08-18-99] Same as V2ReadVertices, but added support for weights.
ReturnCode V21ReadVertices(MkBody_Options* options, FILE* fp, jeBody* pBody, char* name, int NumVertices, V2VertexDetail* pVerts, VPHData* pvphdata, NodeDetail* pNodes, int NumNodes)
{
	char line[LINE_LENGTH];
	char* ptext;
	int i, j, k;
	int n;
	int VertexGroup;
	int VertexGroupIndex;
	jeXForm3d dummyMatrix;
	jeXForm3d xmatrix;
	jeXForm3d euler;

	assert(pVerts != NULL);

	jeXForm3d_SetEulerAngles(&euler, &options->EulerAngles);

	VertexGroup = -1; // initialize behind, first vertex will increment it to zero
	for(k=0;k<NumVertices;k++)
	{
		if (MkUtil_Interrupt())
			{
				Printf("Interrupted\n");
				return(RETURN_ERROR);
			}

		if(fgets(line, LINE_LENGTH, fp) == NULL)
		{
			Printf("ERROR: Could not read from '%s' NFO file\n", options->NFOFile);
			goto V21ReadVertexFailure;
		}

		// [SLB 08-18-99] With options->nVersion >= 0x0210, blended verts 
		// are allowed.  
		i = sscanf(line, "%d %f %f %f %f %f %f %f",	&VertexGroupIndex,
													&pVerts[k].v.X,
													&pVerts[k].v.Y,
													&pVerts[k].v.Z,
													&pVerts[k].offsetv.X,
													&pVerts[k].offsetv.Y,
													&pVerts[k].offsetv.Z,
													&pVerts[k].weight);
		if(i != 8)
		{
			Printf("ERROR: Could not read vertex for mesh '%s' in '%s' NFO file\n", name, options->NFOFile);
			Printf("Line from VPH file:\n%s\n",line);
			goto V21ReadVertexFailure;
		}

		// Track the VertexGroup by watching the index within the group.
		if(VertexGroupIndex == 0)
			VertexGroup++;
		pVerts[k].VertexGroup = VertexGroup;

		// apply rotation to all data at read time
		jeXForm3d_Transform(&euler, &pVerts[k].v, &pVerts[k].v);
#pragma message ("vertex offsets not used now, but when they are used, they need to be rotated")

		// to get to the bone name, search for the 8th space
		ptext = line;
		for(i=0;i<8;i++)
		{
			ptext = strchr(ptext, ' ');
			if(ptext == NULL)
			{
				Printf("ERROR: Could not read bone vertex for mesh '%s' in '%s' NFO file\n", name, options->NFOFile);
				goto V21ReadVertexFailure;
			}
			ptext++;
		}
		StripNewLine(ptext);

		if(options->Capitalize != MK_FALSE)
			strupr(ptext);

		if(strcmp(ptext, "-1") == 0)
		{
			VPHVertex* pVPHVerts;
			if (pvphdata==NULL)
				{
					Printf("ERROR: VPH data needed (and not loaded) for mesh '%s'\n", name);
					Printf("--- possibly this mesh has not been physiqued:   '%s'\n", name);
					goto V21ReadVertexFailure;
				}
				
			if(jeStrBlock_FindString(pvphdata->pSBObjectNames, name, &j) == JE_FALSE)
			{
				Printf("ERROR: Could not find vph data for mesh '%s'\n", name);
				goto V21ReadVertexFailure;
			}

			pVPHVerts = pvphdata->ppVerts[j];
			j = pvphdata->NumVerts[j] - 1;
			while(j >= 0)
			{
				if(jeVec3d_Compare(&pVPHVerts[j].WSPoint, &pVerts[k].v, VERT_EQUALITY_TOLERANCE) != JE_FALSE)
					break; // found it!

				j--;
			}
			if(j < 0)
			{
				// didn't find vert
				Printf("ERROR: Could not find matching vertex\n");
				goto V21ReadVertexFailure;
			}

			for(n=0;n<NumNodes;n++)
			{
				if(strcmp(pNodes[n].Name, pvphdata->pLinks[pVPHVerts[j].bone].Name) == 0)
					break; // found it!
			}
			if(n == NumNodes)
			{
				// didn't find vert
				Printf("ERROR: (1) Could not find node '%s' for vertex\n", pvphdata->pLinks[pVPHVerts[j].bone].Name);
				goto V21ReadVertexFailure;
			}

			// copy bone name
			strcpy(ptext, pNodes[n].Name);
		}
		else
		{
			for(n=0;n<NumNodes;n++)
			{
				if(strcmp(pNodes[n].Name, ptext) == 0)
					break; // found it!
			}
			if(n == NumNodes)
			{
				// didn't find vert
				Printf("ERROR: (2) Could not find node '%s' for vertex\n", ptext);
				goto V21ReadVertexFailure;
			}
		}

		// gotta have a node at this point
		assert( (n >= 0) && (n < NumNodes) );

		if(jeBody_GetBoneByName(pBody, ptext, &i, &dummyMatrix, &j) == JE_FALSE)
		{
			Printf("ERROR: Could not find bone '%s' for mesh '%s' in '%s' NFO file\n", ptext, name, options->NFOFile);
			goto V21ReadVertexFailure;
		}
		pVerts[k].bone = i;

		// Need to do a NoScale
		jeQuaternion_ToMatrix(&pNodes[n].Q, &xmatrix);
		xmatrix.Translation = pNodes[n].T;

		MaxMath_GetInverse(&xmatrix, &xmatrix);

		// fixup the vert
		MaxMath_Transform(&xmatrix, &pVerts[k].v, &pVerts[k].offsetv);
	}

	return(RETURN_SUCCESS);

V21ReadVertexFailure:

	return(RETURN_ERROR);
} // V21ReadVertices

ReturnCode V2ReadTextureVertices(MkBody_Options* options, FILE* fp, char* name, int NumTVertices, V2TVertexDetail* pTVerts)
{
	char line[LINE_LENGTH];
	int i, k;

	assert(pTVerts != NULL);

	for(k=0;k<NumTVertices;k++)
	{
		if(fgets(line, LINE_LENGTH, fp) == NULL)
		{
			Printf("ERROR: Could not read from '%s' NFO file\n", options->NFOFile);
			goto ReadTextureVertexFailure;
		}

		i = sscanf(line, "%f %f",	&pTVerts[k].tu,
									&pTVerts[k].tv);
		pTVerts[k].NAN = JE_FALSE;
		if(i != 2)
		{
			if (strstr(line,"NAN") != NULL)
				{
					pTVerts[k].tu = pTVerts[k].tv = 0.0f;
					pTVerts[k].NAN = JE_TRUE;
				}
			else
				{
					Printf("ERROR: Could not read texture uv for mesh '%s' in '%s' NFO file\n", name, options->NFOFile);
					Printf("       (expected %d total uv pairs.  failed to read uv pair for vertex %d\n)",NumTVertices,k);
					Printf("Line from VPH file:\n%s\n",line);
					goto ReadTextureVertexFailure;
				}
		}

		// Need to flip the v's for some reason
		pTVerts[k].tv = 1 - pTVerts[k].tv;
	}

	return(RETURN_SUCCESS);

ReadTextureVertexFailure:

	return(RETURN_ERROR);
}

ReturnCode V2ReadFaces(MkBody_Options* options, FILE* fp, char* name, int NumFaces, V2FaceDetail* pFaces)
{
	char line[LINE_LENGTH];
	int i, k;

	assert(pFaces != NULL);

	for(k=0;k<NumFaces;k++)
	{
		if (MkUtil_Interrupt())
			{
				Printf("Interrupted\n");
				return(RETURN_ERROR);
			}
		if(fgets(line, LINE_LENGTH, fp) == NULL)
		{
			Printf("ERROR: Could not read from '%s' NFO file\n", options->NFOFile);
			goto ReadFacesFailure;
		}

		if(sscanf(line, "Face %d", &i) != 1)
		{
			Printf("ERROR: Could not read face number for '%s' mesh from '%s' NFO file\n", name, options->NFOFile);
			Printf("Line from VPH file:\n%s\n",line);
			goto ReadFacesFailure;
		}

		if(fscanf(fp, "    %d\n", &pFaces->material) != 1)
		{
			Printf("ERROR: Could not read face material for '%s' mesh from '%s' NFO file\n", name, options->NFOFile);
			goto ReadFacesFailure;
		}

		if(fscanf(fp, "    %d\n", &pFaces->smoothingGroup) != 1)
		{
			Printf("ERROR: Could not read face group for '%s' mesh from '%s' NFO file\n", name, options->NFOFile);
			goto ReadFacesFailure;
		}

		for(i=0;i<3;i++)
		{
			if(fscanf(fp, "    %d %d\n", &pFaces->v[i], &pFaces->tv[i]) != 2)
			{
				Printf("ERROR: Could not read face indices for '%s' mesh from '%s' NFO file\n", name, options->NFOFile);
				goto ReadFacesFailure;
			}
		}

		pFaces++;
	}

	return(RETURN_SUCCESS);

ReadFacesFailure:

	return(RETURN_ERROR);
}

ReturnCode V2ReadAndAddMesh(MkBody_Options* options, FILE* fp, jeBody* pBody, VPHData* pvphdata, NodeDetail* pNodes, int NumNodes)
{
	ReturnCode retValue = RETURN_SUCCESS;
	ReturnCode thisRetValue;
	char line[LINE_LENGTH];
	char name[NAME_LENGTH];
	char* ptext;
	const char* bonename;
	int NumVertices;
	int NumTVertices;
	int NumFaces;
	int i, j, k;
	V2VertexDetail* pVerts = NULL;
	V2VertexGroupDetail* pVertGroups = NULL;
	V2TVertexDetail* pTVerts = NULL;
	V2FaceDetail* pFaces = NULL;
	const jeVec3d* vertices[3];
	const V2TVertexDetail* tvertices[3];
	int bones[3];
	jeVec3d normals[3];
	jeXForm3d dummyMatrix;
	jeXForm3d xmatrix;
	// special allocations for weighted vertex faces
	int AllocSize1 = 0;
	jeVec3d* pVertices1 = NULL;
	jeVec3d* pNormals1 = NULL;
	int* pBones1 = NULL;
	jeFloat* pWeights1 = NULL;
	int AllocSize2 = 0;
	jeVec3d* pVertices2 = NULL;
	jeVec3d* pNormals2 = NULL;
	int* pBones2 = NULL;
	jeFloat* pWeights2 = NULL;
	int AllocSize3 = 0;
	jeVec3d* pVertices3 = NULL;
	jeVec3d* pNormals3 = NULL;
	int* pBones3 = NULL;
	jeFloat* pWeights3 = NULL;

#define V2RAAM_SAFE_FGETS													\
if(fgets(line, LINE_LENGTH, fp) == NULL)									\
{																			\
	Printf("ERROR: Could not read from '%s' NFO file\n", options->NFOFile);	\
	MkUtil_AdjustReturnCode(&retValue, RETURN_ERROR);								\
	goto ReadMeshClean;														\
}

	// name of mesh
	V2RAAM_SAFE_FGETS;
	StripNewLine(line);

	ptext = strstr(line, ": ");
	if(ptext != NULL)
		ptext += 2; // length of ": "
	if( (ptext == NULL) || (strlen(ptext) < 1) )
	{
		// there should be some name-like text
		Printf("ERROR: Found no node name: '%s'\n", line);
		MkUtil_AdjustReturnCode(&retValue, RETURN_ERROR);
		goto ReadMeshClean;
	}
	strncpy(name, ptext, NAME_LENGTH);

	// Vertex List

	V2RAAM_SAFE_FGETS;
	if(strcmp(line, "Vertex List\n") != 0)
	{
		Printf("ERROR: No vertex list for '%s' mesh in '%s' NFO file\n", name, options->NFOFile);
		MkUtil_AdjustReturnCode(&retValue, RETURN_ERROR);
		goto ReadMeshClean;
	}

	V2RAAM_SAFE_FGETS;
	if(sscanf(line, "Number of Vertices = %d", &NumVertices) != 1)
	{
		Printf("ERROR: Could not read number of verticies for mesh '%s' in '%s' NFO file\n", name, options->NFOFile);
		Printf("Line from VPH file:\n%s\n",line);
		MkUtil_AdjustReturnCode(&retValue, RETURN_ERROR);
		goto ReadMeshClean;
	}

	pVerts = JE_RAM_ALLOCATE_ARRAY(V2VertexDetail, NumVertices);
	if(pVerts == NULL)
	{
		Printf("ERROR: Could not allocate vertex array for mesh '%s' in '%s' NFO file\n", name, options->NFOFile);
		MkUtil_AdjustReturnCode(&retValue, RETURN_ERROR);
		goto ReadMeshClean;
	}

	if(options->nVersion >= 0x0210)
		thisRetValue = V21ReadVertices(options, fp, pBody, name, NumVertices, pVerts, pvphdata, pNodes, NumNodes);
	else
		thisRetValue = V2ReadVertices(options, fp, pBody, name, NumVertices, pVerts, pvphdata, pNodes, NumNodes);
	MkUtil_AdjustReturnCode(&retValue, thisRetValue);
	if(thisRetValue == RETURN_ERROR)
	{
		goto ReadMeshClean;
	}

	// Create Vertex Group to Vertex Translation

	k = pVerts[NumVertices - 1].VertexGroup; // last vertex should be in last group
	pVertGroups = JE_RAM_ALLOCATE_ARRAY(V2VertexGroupDetail, k + 1);
	if(pVertGroups == NULL)
	{
		Printf("ERROR: Could not allocate vertex group array for mesh '%s' in '%s' NFO file\n", name, options->NFOFile);
		MkUtil_AdjustReturnCode(&retValue, RETURN_ERROR);
		goto ReadMeshClean;
	}

	j = 0; // tracks index into pVertGroups
	k = -1; // tracks current group
	for(i=0;i<NumVertices;i++)
	{
		if(k != pVerts[i].VertexGroup)
		{
			if(j != pVerts[i].VertexGroup)
			{
				Printf("ERROR: Vertex groups in vertex array are out of sync for mesh '%s' in '%s' NFO file\n", name, options->NFOFile);
				MkUtil_AdjustReturnCode(&retValue, RETURN_ERROR);
				goto ReadMeshClean;
			}
			pVertGroups[j].FirstVertex = i;
			if(j > 0)
				pVertGroups[j - 1].NumVerts = i - pVertGroups[j - 1].FirstVertex;
			j++;
			k = pVerts[i].VertexGroup;
		}
	}
	// finish up last group
	pVertGroups[j - 1].NumVerts = i - pVertGroups[j - 1].FirstVertex;

	// Texture Vertex List

	V2RAAM_SAFE_FGETS;
	if(strcmp(line, "Texture Vertex List\n") != 0)
	{
		Printf("ERROR: No texture vertex list for '%s' mesh in '%s' NFO file\n", name, options->NFOFile);
		MkUtil_AdjustReturnCode(&retValue, RETURN_ERROR);
		goto ReadMeshClean;
	}

	V2RAAM_SAFE_FGETS;
	if(sscanf(line, "Number of Texture Vertices = %d", &NumTVertices) != 1)
	{
		Printf("ERROR: Could not read number of texture verticies for mesh '%s' in '%s' NFO file\n", name, options->NFOFile);
		Printf("Line from VPH file:\n%s\n",line);
		MkUtil_AdjustReturnCode(&retValue, RETURN_ERROR);
		goto ReadMeshClean;
	}

	pTVerts = JE_RAM_ALLOCATE_ARRAY(V2TVertexDetail, NumTVertices);
	if(pTVerts == NULL)
	{
		Printf("ERROR: Could not allocate texture vertex array for mesh '%s' in '%s' NFO file\n", name, options->NFOFile);
		MkUtil_AdjustReturnCode(&retValue, RETURN_ERROR);
		goto ReadMeshClean;
	}

	thisRetValue = V2ReadTextureVertices(options, fp, name, NumTVertices, pTVerts);
	MkUtil_AdjustReturnCode(&retValue, thisRetValue);
	if(thisRetValue == RETURN_ERROR)
	{
		goto ReadMeshClean;
	}

	// Face List

	V2RAAM_SAFE_FGETS;
	if(strcmp(line, "Face List\n") != 0)
	{
		Printf("ERROR: No face list for '%s' mesh in '%s' NFO file\n", name, options->NFOFile);
		MkUtil_AdjustReturnCode(&retValue, RETURN_ERROR);
		goto ReadMeshClean;
	}

	V2RAAM_SAFE_FGETS;
	if(sscanf(line, "Number of Faces = %d", &NumFaces) != 1)
	{
		Printf("ERROR: Could not read number of faces for mesh '%s' in '%s' NFO file\n", name, options->NFOFile);
		Printf("Line from VPH file:\n%s\n",line);
		MkUtil_AdjustReturnCode(&retValue, RETURN_ERROR);
		goto ReadMeshClean;
	}

	pFaces = JE_RAM_ALLOCATE_ARRAY(V2FaceDetail, NumFaces);
	if(pFaces == NULL)
	{
		Printf("ERROR: Could not allocate face array for mesh '%s' in '%s' NFO file\n", name, options->NFOFile);
		MkUtil_AdjustReturnCode(&retValue, RETURN_ERROR);
		goto ReadMeshClean;
	}

	retValue = V2ReadFaces(options, fp, name, NumFaces, pFaces);
	if(retValue == RETURN_ERROR)
	{
		MkUtil_AdjustReturnCode(&retValue, RETURN_ERROR);
		goto ReadMeshClean;
	}

	// Add the faces

	for(i=0;i<NumFaces;i++)
	{
		// Setup texture vertices and world space normals
		for(j=0;j<3;j++)
		{
			if (MkUtil_Interrupt())
				{
					Printf("Interrupted\n");
					MkUtil_AdjustReturnCode(&retValue, RETURN_ERROR);
					goto ReadMeshClean;
				}

			tvertices[j] = &pTVerts[pFaces[i].tv[j]];

			if (tvertices[j]->NAN == JE_TRUE)
				{	
					jeVec3d v;
					v = pVerts[ pVertGroups[ pFaces[i].v[j] ].FirstVertex ].v;
					Printf("Error: Bad uv coordinates for mesh '%s' in '%s' NFO file\n", name,options->NFOFile);
					Printf("       (uv coordinates used in vertex at coordinates (%f,%f,%f))\n",v.X,v.Y,v.Z);
					MkUtil_AdjustReturnCode(&retValue, RETURN_ERROR);
					goto ReadMeshClean;
				}

			// Calculate normals in world space here.  Both rigid and blended 
			// verts will need it.
			thisRetValue = CalculateVertexNormal(	pFaces[i].v[j], 
													pFaces[i].smoothingGroup, 
													NumFaces, pFaces, 
													pVerts, 
													pVertGroups,
													&normals[j]);
			MkUtil_AdjustReturnCode(&retValue, thisRetValue);
			if(thisRetValue == RETURN_ERROR)
			{
				Printf("ERROR: Could not calculate normal for mesh '%s' in '%s' NFO file\n", name, options->NFOFile);
				goto ReadMeshClean;
			}
		}

		if (pFaces[i].material<0)
			{
				MkUtil_AdjustReturnCode(&retValue, RETURN_ERROR);
				Printf("ERROR: no material assigned to face %d, on mesh '%s' in '%s' NFO file\n", 
						i,name, options->NFOFile);
				goto ReadMeshClean;
			}		
		if (pFaces[i].material >= jeBody_GetMaterialCount(pBody))
			{
				MkUtil_AdjustReturnCode(&retValue, RETURN_ERROR);
				Printf("ERROR: bad material index for face %d, on mesh '%s' in '%s' NFO file\n",
						i,name, options->NFOFile);
				Printf("       (index was %d, and there are only %d materials)\n", 
						pFaces[i].material,jeBody_GetMaterialCount(pBody));	
				goto ReadMeshClean;
			}

		// if face involves no weighted verts, go simple
		if( (pVertGroups[ pFaces[i].v[0] ].NumVerts == 1) &&
			(pVertGroups[ pFaces[i].v[1] ].NumVerts == 1) &&
			(pVertGroups[ pFaces[i].v[2] ].NumVerts == 1) )
		{
			// setup vertices and bones and convert normals to bone space
			for(j=0;j<3;j++)
			{
				vertices[j] = &pVerts[ pVertGroups[ pFaces[i].v[j] ].FirstVertex ].offsetv;
				bones[j] = pVerts[ pVertGroups[ pFaces[i].v[j] ].FirstVertex ].bone;

				// convert world space normals to bone space
				jeBody_GetBone(	pBody, 
								pVerts[ pVertGroups[ pFaces[i].v[j] ].FirstVertex ].bone, 
								&bonename, 
								&dummyMatrix, 
								&k);
				for(k=0;k<NumNodes;k++)
				{
					if(strcmp(pNodes[k].Name, bonename) == 0)
						break; // found it!
				}
				if(k == NumNodes)
				{
					// didn't find bone
					Printf("ERROR: (3) Could not find node '%s' for vertex\n", bonename);
					MkUtil_AdjustReturnCode(&retValue, RETURN_ERROR);
					goto ReadMeshClean;
				}

				// gotta have a node at this point
				assert( (k >= 0) && (k < NumNodes) );

				// Need to do a NoScale
				jeQuaternion_ToMatrix(&pNodes[k].Q, &xmatrix);
				xmatrix.Translation = pNodes[k].T;

				MaxMath_GetInverse(&xmatrix, &xmatrix);

				// fixup the normal to bone space
				jeVec3d_Clear(&xmatrix.Translation);
				MaxMath_Transform(&xmatrix, &normals[j], &normals[j]);
			}

			if(jeBody_AddFace(pBody,	vertices[0], &normals[0],
											tvertices[0]->tu, tvertices[0]->tv, 
											bones[0],
										vertices[1], &normals[1],
											tvertices[1]->tu, tvertices[1]->tv, 
											bones[1],
										vertices[2], &normals[2],
											tvertices[2]->tu, tvertices[2]->tv, 
											bones[2],
										pFaces[i].material) == JE_FALSE)
			{
				MkUtil_AdjustReturnCode(&retValue, RETURN_ERROR);
				Printf("ERROR: Could not add face for mesh '%s' in '%s' NFO file\n", name, options->NFOFile);
				goto ReadMeshClean;
			}
		}
		else
		{
			// Carefully use the special allocations.  Don't reallocate unless necessary.
			void* pTemp;

			// index 0
			if(AllocSize1 < pVertGroups[ pFaces[i].v[0] ].NumVerts)
			{
				pTemp = jeRam_Realloc(pVertices1, sizeof(*pVertices1) * pVertGroups[ pFaces[i].v[0] ].NumVerts);
				if(pTemp == NULL)
				{
					MkUtil_AdjustReturnCode(&retValue, RETURN_ERROR);
					Printf("ERROR: Could not realloc vertex array for weighted face for mesh '%s' in '%s' NFO file\n", name, options->NFOFile);
					goto ReadMeshClean;
				}
				pVertices1 = (jeVec3d*)pTemp;

				pTemp = jeRam_Realloc(pNormals1, sizeof(*pNormals1) * pVertGroups[ pFaces[i].v[0] ].NumVerts);
				if(pTemp == NULL)
				{
					MkUtil_AdjustReturnCode(&retValue, RETURN_ERROR);
					Printf("ERROR: Could not realloc vertex array for weighted face for mesh '%s' in '%s' NFO file\n", name, options->NFOFile);
					goto ReadMeshClean;
				}
				pNormals1 = (jeVec3d*)pTemp;

				pTemp = jeRam_Realloc(pBones1, sizeof(*pBones1) * pVertGroups[ pFaces[i].v[0] ].NumVerts);
				if(pTemp == NULL)
				{
					MkUtil_AdjustReturnCode(&retValue, RETURN_ERROR);
					Printf("ERROR: Could not realloc bone array for weighted face for mesh '%s' in '%s' NFO file\n", name, options->NFOFile);
					goto ReadMeshClean;
				}
				pBones1 = (int*)pTemp;

				pTemp = jeRam_Realloc(pWeights1, sizeof(*pWeights1) * pVertGroups[ pFaces[i].v[0] ].NumVerts);
				if(pTemp == NULL)
				{
					MkUtil_AdjustReturnCode(&retValue, RETURN_ERROR);
					Printf("ERROR: Could not realloc weight array for weighted face for mesh '%s' in '%s' NFO file\n", name, options->NFOFile);
					goto ReadMeshClean;
				}
				pWeights1 = (jeFloat*)pTemp;

				for(j=0;j<pVertGroups[ pFaces[i].v[0] ].NumVerts;j++)
				{
					pVertices1[j] = pVerts[ pVertGroups[ pFaces[i].v[0] ].FirstVertex + j ].offsetv;
					pBones1[j] = pVerts[ pVertGroups[ pFaces[i].v[0] ].FirstVertex + j ].bone;
					pWeights1[j] = pVerts[ pVertGroups[ pFaces[i].v[0] ].FirstVertex + j ].weight;

					// convert normal to each bone's space to fill array
					jeBody_GetBone(	pBody, 
									pBones1[j], 
									&bonename, 
									&dummyMatrix, 
									&k);
					for(k=0;k<NumNodes;k++)
					{
						if(strcmp(pNodes[k].Name, bonename) == 0)
							break; // found it!
					}
					if(k == NumNodes)
					{
						// didn't find bone
						Printf("ERROR: (3) Could not find node '%s' for vertex\n", bonename);
						MkUtil_AdjustReturnCode(&retValue, RETURN_ERROR);
						goto ReadMeshClean;
					}

					// gotta have a node at this point
					assert( (k >= 0) && (k < NumNodes) );

					// Need to do a NoScale
					jeQuaternion_ToMatrix(&pNodes[k].Q, &xmatrix);
					xmatrix.Translation = pNodes[k].T;

					MaxMath_GetInverse(&xmatrix, &xmatrix);

					// fixup the normal to bone space
					jeVec3d_Clear(&xmatrix.Translation);
					MaxMath_Transform(&xmatrix, &normals[0], pNormals1 + j);
				}
			}

			// index 1
			if(AllocSize2 < pVertGroups[ pFaces[i].v[1] ].NumVerts)
			{
				pTemp = jeRam_Realloc(pVertices2, sizeof(*pVertices2) * pVertGroups[ pFaces[i].v[1] ].NumVerts);
				if(pTemp == NULL)
				{
					MkUtil_AdjustReturnCode(&retValue, RETURN_ERROR);
					Printf("ERROR: Could not realloc vertex array for weighted face for mesh '%s' in '%s' NFO file\n", name, options->NFOFile);
					goto ReadMeshClean;
				}
				pVertices2 = (jeVec3d*)pTemp;

				pTemp = jeRam_Realloc(pNormals2, sizeof(*pNormals2) * pVertGroups[ pFaces[i].v[1] ].NumVerts);
				if(pTemp == NULL)
				{
					MkUtil_AdjustReturnCode(&retValue, RETURN_ERROR);
					Printf("ERROR: Could not realloc vertex array for weighted face for mesh '%s' in '%s' NFO file\n", name, options->NFOFile);
					goto ReadMeshClean;
				}
				pNormals2 = (jeVec3d*)pTemp;

				pTemp = jeRam_Realloc(pBones2, sizeof(*pBones2) * pVertGroups[ pFaces[i].v[1] ].NumVerts);
				if(pTemp == NULL)
				{
					MkUtil_AdjustReturnCode(&retValue, RETURN_ERROR);
					Printf("ERROR: Could not realloc bone array for weighted face for mesh '%s' in '%s' NFO file\n", name, options->NFOFile);
					goto ReadMeshClean;
				}
				pBones2 = (int*)pTemp;

				pTemp = jeRam_Realloc(pWeights2, sizeof(*pWeights2) * pVertGroups[ pFaces[i].v[1] ].NumVerts);
				if(pTemp == NULL)
				{
					MkUtil_AdjustReturnCode(&retValue, RETURN_ERROR);
					Printf("ERROR: Could not realloc weight array for weighted face for mesh '%s' in '%s' NFO file\n", name, options->NFOFile);
					goto ReadMeshClean;
				}
				pWeights2 = (jeFloat*)pTemp;

				for(j=0;j<pVertGroups[ pFaces[i].v[1] ].NumVerts;j++)
				{
					pVertices2[j] = pVerts[ pVertGroups[ pFaces[i].v[1] ].FirstVertex + j ].offsetv;
					pBones2[j] = pVerts[ pVertGroups[ pFaces[i].v[1] ].FirstVertex + j ].bone;
					pWeights2[j] = pVerts[ pVertGroups[ pFaces[i].v[1] ].FirstVertex + j ].weight;

					// convert normal to each bone's space to fill array
					jeBody_GetBone(	pBody, 
									pBones2[j], 
									&bonename, 
									&dummyMatrix, 
									&k);
					for(k=0;k<NumNodes;k++)
					{
						if(strcmp(pNodes[k].Name, bonename) == 0)
							break; // found it!
					}
					if(k == NumNodes)
					{
						// didn't find bone
						Printf("ERROR: (3) Could not find node '%s' for vertex\n", bonename);
						MkUtil_AdjustReturnCode(&retValue, RETURN_ERROR);
						goto ReadMeshClean;
					}

					// gotta have a node at this point
					assert( (k >= 0) && (k < NumNodes) );

					// Need to do a NoScale
					jeQuaternion_ToMatrix(&pNodes[k].Q, &xmatrix);
					xmatrix.Translation = pNodes[k].T;

					MaxMath_GetInverse(&xmatrix, &xmatrix);

					// fixup the normal to bone space
					jeVec3d_Clear(&xmatrix.Translation);
					MaxMath_Transform(&xmatrix, &normals[1], pNormals2 + j);
				}
			}

			// index 2
			if(AllocSize3 < pVertGroups[ pFaces[i].v[2] ].NumVerts)
			{
				pTemp = jeRam_Realloc(pVertices3, sizeof(*pVertices3) * pVertGroups[ pFaces[i].v[2] ].NumVerts);
				if(pTemp == NULL)
				{
					MkUtil_AdjustReturnCode(&retValue, RETURN_ERROR);
					Printf("ERROR: Could not realloc vertex array for weighted face for mesh '%s' in '%s' NFO file\n", name, options->NFOFile);
					goto ReadMeshClean;
				}
				pVertices3 = (jeVec3d*)pTemp;

				pTemp = jeRam_Realloc(pNormals3, sizeof(*pNormals3) * pVertGroups[ pFaces[i].v[2] ].NumVerts);
				if(pTemp == NULL)
				{
					MkUtil_AdjustReturnCode(&retValue, RETURN_ERROR);
					Printf("ERROR: Could not realloc vertex array for weighted face for mesh '%s' in '%s' NFO file\n", name, options->NFOFile);
					goto ReadMeshClean;
				}
				pNormals3 = (jeVec3d*)pTemp;

				pTemp = jeRam_Realloc(pBones3, sizeof(*pBones3) * pVertGroups[ pFaces[i].v[2] ].NumVerts);
				if(pTemp == NULL)
				{
					MkUtil_AdjustReturnCode(&retValue, RETURN_ERROR);
					Printf("ERROR: Could not realloc bone array for weighted face for mesh '%s' in '%s' NFO file\n", name, options->NFOFile);
					goto ReadMeshClean;
				}
				pBones3 = (int*)pTemp;

				pTemp = jeRam_Realloc(pWeights3, sizeof(*pWeights3) * pVertGroups[ pFaces[i].v[2] ].NumVerts);
				if(pTemp == NULL)
				{
					MkUtil_AdjustReturnCode(&retValue, RETURN_ERROR);
					Printf("ERROR: Could not realloc weight array for weighted face for mesh '%s' in '%s' NFO file\n", name, options->NFOFile);
					goto ReadMeshClean;
				}
				pWeights3 = (jeFloat*)pTemp;

				for(j=0;j<pVertGroups[ pFaces[i].v[2] ].NumVerts;j++)
				{
					pVertices3[j] = pVerts[ pVertGroups[ pFaces[i].v[2] ].FirstVertex + j ].offsetv;
					pBones3[j] = pVerts[ pVertGroups[ pFaces[i].v[2] ].FirstVertex + j ].bone;
					pWeights3[j] = pVerts[ pVertGroups[ pFaces[i].v[2] ].FirstVertex + j ].weight;

					// convert normal to each bone's space to fill array
					jeBody_GetBone(	pBody, 
									pBones3[j], 
									&bonename, 
									&dummyMatrix, 
									&k);
					for(k=0;k<NumNodes;k++)
					{
						if(strcmp(pNodes[k].Name, bonename) == 0)
							break; // found it!
					}
					if(k == NumNodes)
					{
						// didn't find bone
						Printf("ERROR: (3) Could not find node '%s' for vertex\n", bonename);
						MkUtil_AdjustReturnCode(&retValue, RETURN_ERROR);
						goto ReadMeshClean;
					}

					// gotta have a node at this point
					assert( (k >= 0) && (k < NumNodes) );

					// Need to do a NoScale
					jeQuaternion_ToMatrix(&pNodes[k].Q, &xmatrix);
					xmatrix.Translation = pNodes[k].T;

					MaxMath_GetInverse(&xmatrix, &xmatrix);

					// fixup the normal to bone space
					jeVec3d_Clear(&xmatrix.Translation);
					MaxMath_Transform(&xmatrix, &normals[2], pNormals3 + j);
				}
			}
			if(jeBody_AddFaceWeightedVerts(	pBody,	
											pVertices1, pNormals1,
												tvertices[0]->tu, tvertices[0]->tv, 
												pBones1, pWeights1,
												pVertGroups[ pFaces[i].v[0] ].NumVerts,
											pVertices2, pNormals2,
												tvertices[1]->tu, tvertices[1]->tv, 
												pBones2, pWeights2,
												pVertGroups[ pFaces[i].v[1] ].NumVerts,
											pVertices3, pNormals3,
												tvertices[2]->tu, tvertices[2]->tv, 
												pBones3, pWeights3,
												pVertGroups[ pFaces[i].v[2] ].NumVerts,
											pFaces[i].material) == JE_FALSE)
			{
				MkUtil_AdjustReturnCode(&retValue, RETURN_ERROR);
				Printf("ERROR: Could not add face for mesh '%s' in '%s' NFO file\n", name, options->NFOFile);
				goto ReadMeshClean;
			}
		}

		fCount++;
		if (fCount%25==0)
			Printf("\t%d Faces Processed\n",fCount);
	}

	Printf("\t%d Blend Data Elements Processed\n", jeBody_GetBlendDataCount(pBody));

ReadMeshClean:
	if(pFaces != NULL)
		jeRam_Free(pFaces);
	if(pTVerts != NULL)
		jeRam_Free(pTVerts);
	if(pVerts != NULL)
		jeRam_Free(pVerts);
	if(pVertGroups != NULL)
		jeRam_Free(pVertGroups);

	if(pVertices1 != NULL)
		jeRam_Free(pVertices1);
	if(pNormals1 != NULL)
		jeRam_Free(pNormals1);
	if(pBones1 != NULL)
		jeRam_Free(pBones1);
	if(pWeights1 != NULL)
		jeRam_Free(pWeights1);
	if(pVertices2 != NULL)
		jeRam_Free(pVertices2);
	if(pNormals2 != NULL)
		jeRam_Free(pNormals2);
	if(pBones2 != NULL)
		jeRam_Free(pBones2);
	if(pWeights2 != NULL)
		jeRam_Free(pWeights2);
	if(pVertices3 != NULL)
		jeRam_Free(pVertices3);
	if(pNormals3 != NULL)
		jeRam_Free(pNormals3);
	if(pBones3 != NULL)
		jeRam_Free(pBones3);
	if(pWeights3 != NULL)
		jeRam_Free(pWeights3);

	return(retValue);
}

static	void	SetExtension(char *Dest, const char *Src, const char *NewExt)
{
	char	Drive[_MAX_DRIVE];
	char	Dir[_MAX_DIR];
	char	Name[_MAX_FNAME];

	_splitpath(Src, Drive, Dir, Name, NULL);
	_makepath(Dest, Drive, Dir, Name, NewExt);
}


jeBoolean	MkBody_Material(MkBody_Options *options, const char *line, jeBody *pBody, int i)
{
	//	by trilobite	Jan. 2011
	//char* ptext;
	const char* ptext;

	char name[NAME_LENGTH];
	int Index;
	jeMaterialSpec *MatSpec;
	
				
	assert( line != NULL );
	assert( pBody != NULL );

	if (strchr(line,':')==NULL)
		{
			Printf("ERROR: line not formatted as expected: '%s'\n",line);
			return JE_FALSE;
		}
	if(strncmp(line, "(MAP)", 5) == 0)
	{
		ptext = strstr(line, ": ");
		if(ptext != NULL)
			ptext += 2; // length of ": "
		if( (ptext == NULL) || (strlen(ptext) < 1) )
		{
			// there should be some filename-like text
			Printf("ERROR: Found no texture map: '%s'\n", line);
			return JE_FALSE;
		}

		// prefix the texture path
		strcpy(name, options->TexturePath);
		strcat(name, ptext);

		{
			jeVFile *VF;
			jeBitmap *Bmp;
			jeBitmap_Info Info;

			char BmpFileName[_MAX_PATH];
			if (stricmp(name+strlen(name)-4,".BMP") != 0)
				{
					Printf("ERROR: Material %d does not reference a BMP.  '%s' \n",i,ptext);
					return JE_FALSE;
				}
			// Try compressed images first, then regular bitmaps
			SetExtension(BmpFileName,name,".gbm");
			VF = jeVFile_OpenNewSystem(NULL,JE_VFILE_TYPE_DOS,BmpFileName,NULL,JE_VFILE_OPEN_READONLY);
			if (VF == NULL)
			{
				VF = jeVFile_OpenNewSystem(NULL,JE_VFILE_TYPE_DOS,name,NULL,JE_VFILE_OPEN_READONLY);
				if (VF == NULL)
					{
						Printf("ERROR: Material %d, unable to locate bitmap '%s'\n",i,name);
						return JE_FALSE;
					}
			}
			else
			{
#pragma message("Take out message about using compressed bitmaps.")
				Printf("WARNING: Using compressed bitmap '%s'\n", BmpFileName);
			}
			Bmp = jeBitmap_CreateFromFile(VF);
			jeVFile_Close(VF);
			if (Bmp == NULL)
			{
				Printf("ERROR: Material %d, unable to read bitmap '%s'\n",i,name);
				return JE_FALSE;
			}
			jeBitmap_GetInfo(Bmp, &Info, NULL);
			if (Info.Format == JE_PIXELFORMAT_8BIT)
			{
				if (jeBitmap_SetColorKey(Bmp,JE_TRUE,255,JE_TRUE)==JE_FALSE)
				{
					Printf("ERROR: Material %d, unable to set color keying (transparent color) to 255 for bitmap '%s'\n",i,name);
					return JE_FALSE;
				}
			}
		
            {
				int W,H; //,TwoPower;
				W = jeBitmap_Width ( Bmp );
				H = jeBitmap_Height( Bmp );
				if (W != H)
				{
				#pragma message ("change this so that we can 'treat warnings as errors'")
					Printf("WARNING: bitmap for material %d is not square. '%s'\n",i,name);
						//return JE_FALSE;
				}
				// BEGIN - Remove texture size limitation from Actor Studio - By Jeff, added by paradoxnj - 2/08/2005
				/*#pragma message ("remove this when these limitations are removed from engine")
				for (TwoPower=1; TwoPower<=256; TwoPower*=2)
					{
						if (TwoPower==W)
						break;
					}
				if (TwoPower > 256)
					{
						Printf("Error: bitmap for material must be a power of 2 width and height.\n");
						Printf("       The size of material %d ('%s') is width=%d, height=%d\n",i,name,W,H);
						return JE_FALSE;
					}*/
				// END - Remove texture size limitation from Actor Studio - By Jeff, added by paradoxnj - 2/08/2005
			}

			strcpy(name,line+6);		// after the "(MAP) "
			*strrchr(name, ':')=0;

			MatSpec = jeMaterialSpec_Create(jeResourceMgr_GetEngine(jeResourceMgr_GetSingleton()), jeResourceMgr_GetSingleton());
#pragma message ("Krouer: change NULL to something better next time")
	        jeMaterialSpec_AddLayerFromBitmap(MatSpec, 0, Bmp, name);
	        
			if(jeBody_AddMaterial(pBody, name, MatSpec, 255.0f, 255.0f, 255.0f, NULL, &Index) == JE_FALSE)
			{
				Printf("ERROR: Could not add material %d ('%s') to body. \n",i, name);
				return JE_FALSE;
			}
		}
	}
	else if(strncmp(line, "(RGB)", 5) == 0)
	{
		float r, g, b;
		int j;

		r=g=b=255.0f;
		ptext = strrchr(line, ':');
		if(ptext != NULL)
		{
			ptext++;
			j = sscanf(ptext, "%f %f %f", &r, &g, &b);
		} 
		else 
			j=0;
		if( (ptext == NULL) || (j != 3) )
		{
			// there should be some rgb values
			Printf("ERROR: Found no texture color: '%s'\n", line);
			return JE_FALSE;
		}
		strcpy(name,line+6);      //  after the "(RGB) "
		*strrchr(name, ':')=0;
		if(jeBody_AddMaterial(pBody, name, NULL, r, g, b, NULL, &Index) == JE_FALSE)
		{
			Printf("ERROR: Could not add material to body: '%s'\n", line);
			return JE_FALSE;
		}
	}
	else
	{
		Printf("ERROR: Could not identify material type: '%s'\n", line);
		return JE_FALSE;
	}
	return JE_TRUE;
}


ReturnCode MkBody_DoMake(MkBody_Options* options,MkUtil_Printf PrintfCallback)
{
	Printf = PrintfCallback;
	fCount =0;

	int nVersion = 0;
	ReturnCode retValue = RETURN_SUCCESS;
	ReturnCode newValue;
	FILE* fp;
	jeVFile *VF;
	jeBody* pBody = NULL;
	char line[LINE_LENGTH];
	char vlgfile[_MAX_PATH];
	VPHData* pvphdata = NULL;
	int i, j, k,Count;
	int Index;
	char* ptext;
	int NumBones;
	BoneDetail* pBoneIDs;
	int NumNodes;
	NodeDetail* pNodes;
	VertexDetail* pfacevert;
	char name[NAME_LENGTH];
	FaceDetail face;
	int NumMeshes;
	VPHVertex* pVPHVerts;
	jeXForm3d euler;
	jeQuaternion eulerq;

	// body and NFO filenames must be specified
	if(options->BodyFile[0] == 0)
	{
		Printf("ERROR: Must specify a body file\n");
		MkUtil_AdjustReturnCode(&retValue, RETURN_ERROR);
		remove(vlgfile);
		return retValue;
	}
	if(options->NFOFile[0] == 0)
	{
		Printf("ERROR: Must specify an NFO file\n");
		MkUtil_AdjustReturnCode(&retValue, RETURN_ERROR);
		remove(vlgfile);
		return retValue;
	}

	// Process the VPH file if there is one
	if(options->VPHFile[0] != 0)
	{
//		GetTempFileName(".", "VLG", 0, vlgfile);
		strcpy(vlgfile, options->VPHFile);
		strcat(vlgfile, ".VLG");
		i = vphmain(options->VPHFile, vlgfile);
		if(options->WriteTextVPH != MK_FALSE)
		{
			strcpy(line, options->VPHFile);
			ptext = strchr(line, '.');
			if(ptext != NULL)
				ptext[1] = 0;
			strcat(line, "vlg");
			remove(line);
			rename(vlgfile, line);
			// now use the new file
			strcpy(options->VPHFile, line);
		}
		else
		{
			// now use the new file
			strcpy(options->VPHFile, vlgfile);
		}

		if(i != 0)
		{
			Printf("ERROR: Could not read the VPH file\n");
			MkUtil_AdjustReturnCode(&retValue, RETURN_ERROR);
			remove(vlgfile);
			return retValue;
		}
	}

	// who knows how many times this will get used in here
	jeXForm3d_SetEulerAngles(&euler, &options->EulerAngles);
	jeQuaternion_FromMatrix(&euler, &eulerq);

	pBody = jeBody_Create();

	if(pBody != NULL)
	{

		jeBody_SetOptimizeFlags(pBody,	JE_BODY_OPTIMIZE_FLAGS_VERTS | 
										JE_BODY_OPTIMIZE_FLAGS_NORMALS);


		if(options->VPHFile[0] != 0)
		{
			// Read vph data
			fp = fopen(options->VPHFile, "rt");
			if(fp == NULL)
			{
				Printf("ERROR: Could not open '%s' NFO file\n", options->NFOFile);
				MkUtil_AdjustReturnCode(&retValue, RETURN_ERROR);
				jeBody_Destroy(&pBody);
				remove(vlgfile);
				return retValue;
			}

			pvphdata = JE_RAM_ALLOCATE_STRUCT(VPHData);
			if(pvphdata == NULL)
			{
				Printf("ERROR: Could not allocate VPHData\n");
				MkUtil_AdjustReturnCode(&retValue, RETURN_ERROR);
				jeBody_Destroy(&pBody);
				remove(vlgfile);
				return retValue;
			}
			newValue = ReadVPHData(options, fp, pvphdata);
			fclose(fp);
			remove(vlgfile); // done with it now
			MkUtil_AdjustReturnCode(&retValue, newValue);
			if(retValue == RETURN_ERROR)
			{
				Printf("ERROR: Could not read '%s' VPH data\n", options->VPHFile);
				jeBody_Destroy(&pBody);
				jeRam_Free(pvphdata);
				return retValue;
			}
		}

		// Read nfo data
		fp = fopen(options->NFOFile, "rt");
		if(fp == NULL)
		{
			Printf("ERROR: Could not open '%s' NFO file\n", options->NFOFile);
			MkUtil_AdjustReturnCode(&retValue, RETURN_ERROR);
			jeBody_Destroy(&pBody);
			FreeVPHData(&pvphdata);
			return retValue;
		}

#define SET_ERROR_CLOSE_RETURN					\
	MkUtil_AdjustReturnCode(&retValue, RETURN_ERROR);	\
	fclose(fp);									\
	fp = NULL;									\
	jeBody_Destroy(&pBody);						\
	FreeVPHData(&pvphdata);						\
	return retValue

#define FGETS_LINE_OR_CLOSE_AND_RETURN(s)										\
	if(fgets(s, LINE_LENGTH, fp) == NULL)										\
	{																			\
		Printf("ERROR: Could not read from '%s' NFO file\n", options->NFOFile);	\
		SET_ERROR_CLOSE_RETURN;													\
	}

		// Read and check version
		FGETS_LINE_OR_CLOSE_AND_RETURN(line);
		if(strncmp(line, "NFO ", 4) != 0)
		{
			Printf("ERROR: '%s' is not an NFO file\n", options->NFOFile);
			SET_ERROR_CLOSE_RETURN;
		}

		// check version numbers
		StripNewLine(line);
		if(strcmp(line + 4, "1.0") == 0)
		{
			nVersion = 0x0100;
		}
		else if(strcmp(line + 4, "2.0") == 0)
		{
			nVersion = 0x0200;
		}
		else if(strcmp(line + 4, "2.1") == 0)
		{
			nVersion = 0x0210;
		}
		else
		{
			Printf("ERROR: '%s' NFO file version '%s' is not supported\n", options->NFOFile, line + 4);
			SET_ERROR_CLOSE_RETURN;
		}
		assert(nVersion != 0);
		options->nVersion = nVersion;

		// Start with the materials
		FGETS_LINE_OR_CLOSE_AND_RETURN(line);
		if(strcmp(line, "Material List\n") != 0)
		{
			Printf("ERROR: No material list in '%s' NFO file\n", options->NFOFile);
			SET_ERROR_CLOSE_RETURN;
		}

		FGETS_LINE_OR_CLOSE_AND_RETURN(line);
		if(sscanf(line, "Number of Materials = %d", &Count) != 1)
		{
			Printf("ERROR: Could not read number of materials in '%s' NFO file\n", options->NFOFile);
			Printf("Line from VPH file:\n%s\n",line);
			SET_ERROR_CLOSE_RETURN;
		}

		// Add materials
		for (i=0; i<Count; i++)
		{
			if (MkUtil_Interrupt())
				{
					Printf("Interrupted\n");
					SET_ERROR_CLOSE_RETURN;
				}
			FGETS_LINE_OR_CLOSE_AND_RETURN(line);
			StripNewLine(line);
			if (MkBody_Material(options,line,pBody,i)==JE_FALSE)
				{
					Printf("ERROR: Unable to add material %d ('%s')\n",i,line);
					SET_ERROR_CLOSE_RETURN;
				}
		}
		

		// EXTRA materials
		{
			int Last = Count + jeStrBlock_GetCount(options->ExtraMaterials); 
			int i;
			for ( i=Count; i<Last; i++)	
				{
					const char* MatLine;
					MatLine = jeStrBlock_GetString(options->ExtraMaterials,i-Count);
					if (MkBody_Material( options, MatLine, pBody,i)==0)
						{
							Printf("ERROR: Unable to add extra material %d ('%s')\n",i-Count,MatLine);
							SET_ERROR_CLOSE_RETURN;
						}
				}
		}

		// Bone List
		FGETS_LINE_OR_CLOSE_AND_RETURN(line);
		if(strcmp(line, "Bone List\n") != 0)
		{
			Printf("ERROR: No bone list in '%s' NFO file\n", options->NFOFile);
			SET_ERROR_CLOSE_RETURN;
		}

		FGETS_LINE_OR_CLOSE_AND_RETURN(line);
		if(sscanf(line, "Number of Bones = %d", &NumBones) != 1)
		{
			Printf("ERROR: Could not read number of bones in '%s' NFO file\n", options->NFOFile);
			Printf("Line from VPH file:\n%s\n",line);
			SET_ERROR_CLOSE_RETURN;
		}

		pBoneIDs = JE_RAM_ALLOCATE_ARRAY(BoneDetail, NumBones);
		if(pBoneIDs == NULL)
		{
			Printf("ERROR: Could not allocate bone array\n", options->NFOFile);
			SET_ERROR_CLOSE_RETURN;
		}

		// Have a new allocation, so need a new clean up macro

#undef SET_ERROR_CLOSE_RETURN

#define SET_ERROR_CLOSE_RETURN					\
	MkUtil_AdjustReturnCode(&retValue, RETURN_ERROR);	\
	fclose(fp);									\
	fp = NULL;									\
	jeBody_Destroy(&pBody);						\
	FreeVPHData(&pvphdata);						\
	jeRam_Free(pBoneIDs);						\
	return retValue

		for(i=0;i<NumBones;i++)
		{
			if (MkUtil_Interrupt())
				{
					Printf("Interrupted\n");
					break;
				}
			if(fgets(line, LINE_LENGTH, fp) == NULL)
			{
				Printf("ERROR: Could not read from '%s' NFO file\n", options->NFOFile);
				break;
			}
			StripNewLine(line);

			j = (int)strlen("Bone: ");
			if((int)strlen(line) < j)
			{
				// there should be some name-like text
				Printf("ERROR: Found no bone name: '%s'\n", line);
				break;
			}
			strncpy(pBoneIDs[i].Name, line + j, NAME_LENGTH);
			if(options->Capitalize != MK_FALSE)
				strupr(pBoneIDs[i].Name);

			if(fgets(line, LINE_LENGTH, fp) == NULL)
			{
				Printf("ERROR: Could not read from '%s' NFO file\n", options->NFOFile);
				break;
			}
			StripNewLine(line);

			j = (int)strlen("Parent: ");
			if((int)strlen(line) < j)
			{
				// there should be a number
				Printf("ERROR: Found no bone index: '%s'\n", line);
				break;
			}
			if(sscanf(line + j, "%d", &pBoneIDs[i].ParentID) != 1)
			{
				Printf("ERROR: Read no bone index: '%s'\n", line);
				break;
			}
		}
		if(i != NumBones)
		{
			// must have been an error
			SET_ERROR_CLOSE_RETURN;
		}

		// Node List
		FGETS_LINE_OR_CLOSE_AND_RETURN(line);
		if(strcmp(line, "Node Transform Matrix List\n") != 0)
		{
			Printf("ERROR: No node list in '%s' NFO file\n", options->NFOFile);
			SET_ERROR_CLOSE_RETURN;
		}

		FGETS_LINE_OR_CLOSE_AND_RETURN(line);
		if(sscanf(line, "Number of Nodes = %d", &NumNodes) != 1)
		{
			Printf("ERROR: Could not read number of nodes in '%s' NFO file\n", options->NFOFile);
			Printf("Line from VPH file:\n%s\n",line);
			SET_ERROR_CLOSE_RETURN;
		}

		pNodes = JE_RAM_ALLOCATE_ARRAY(NodeDetail, NumNodes);
		if(pNodes == NULL)
		{
			Printf("ERROR: Could not allocate node array\n", options->NFOFile);
			SET_ERROR_CLOSE_RETURN;
		}

		// Have a new allocation, so need a new clean up macro

#undef SET_ERROR_CLOSE_RETURN

#define SET_ERROR_CLOSE_RETURN					\
	MkUtil_AdjustReturnCode(&retValue, RETURN_ERROR);	\
	fclose(fp);									\
	fp = NULL;									\
	jeBody_Destroy(&pBody);						\
	FreeVPHData(&pvphdata);						\
	jeRam_Free(pBoneIDs);						\
	jeRam_Free(pNodes);							\
	return retValue

		for(i=0;i<NumNodes;i++)
		{
			if (MkUtil_Interrupt())
				{
					Printf("Interrupted\n");
					break;
				}
			if(fgets(line, LINE_LENGTH, fp) == NULL)
			{
				Printf("ERROR: Could not read from '%s' NFO file\n", options->NFOFile);
				break;
			}
			StripNewLine(line);

			ptext = strstr(line, ": ");
			if(ptext != NULL)
				ptext += 2; // length of ": "
			if( (ptext == NULL) || (strlen(ptext) < 1) )
			{
				// there should be some name-like text
				Printf("ERROR: Found no node name: '%s'\n", line);
				break;
			}
			strncpy(pNodes[i].Name, ptext, NAME_LENGTH);
			if(options->Capitalize != MK_FALSE)
				strupr(pNodes[i].Name);

			// Read TM Matrix
			j = fscanf(fp, "%f, %f, %f,\n",	&pNodes[i].NodeTM.AX,
											&pNodes[i].NodeTM.AY,
											&pNodes[i].NodeTM.AZ);
			if(j != 3)
			{
				Printf("ERROR: Could not read node %d's matrix\n", i);
				break;
			}
			j = fscanf(fp, "%f, %f, %f,\n",	&pNodes[i].NodeTM.BX,
											&pNodes[i].NodeTM.BY,
											&pNodes[i].NodeTM.BZ);
			if(j != 3)
			{
				Printf("ERROR: Could not read node %d's matrix\n", i);
				break;
			}
			j = fscanf(fp, "%f, %f, %f,\n",	&pNodes[i].NodeTM.CX,
											&pNodes[i].NodeTM.CY,
											&pNodes[i].NodeTM.CZ);
			if(j != 3)
			{
				Printf("ERROR: Could not read node %d's matrix\n", i);
				break;
			}
			j = fscanf(fp, "%f, %f, %f,\n",	&pNodes[i].NodeTM.Translation.X,
											&pNodes[i].NodeTM.Translation.Y,
											&pNodes[i].NodeTM.Translation.Z);
			if(j != 3)
			{
				Printf("ERROR: Could not read node %d's matrix\n", i);
				break;
			}

			// jeQuaternion
			j = fscanf(fp, "Q(w,x,y,z): %f %f %f %f\n",	&pNodes[i].Q.W,
														&pNodes[i].Q.X,
														&pNodes[i].Q.Y,
														&pNodes[i].Q.Z);
			if(j != 4)
			{
				Printf("ERROR: Could not read node %d's quaternion\n", i);
				break;
			}
			jeQuaternion_Normalize(&pNodes[i].Q); // can't believe I have to do this!

			// Scale
			j = fscanf(fp, "S(x,y,z): %f %f %f\n",	&pNodes[i].S.X,
													&pNodes[i].S.Y,
													&pNodes[i].S.Z);
			if(j != 3)
			{
				Printf("ERROR: Could not read node %d's scale\n", i);
				break;
			}

			// Translation
			j = fscanf(fp, "T(x,y,z): %f %f %f\n",	&pNodes[i].T.X,
													&pNodes[i].T.Y,
													&pNodes[i].T.Z);
			if(j != 3)
			{
				Printf("ERROR: Could not read node %d's translation\n", i);
				break;
			}

			// apply rotation to all data at read time
			jeXForm3d_Orthonormalize(&pNodes[i].NodeTM);
			jeXForm3d_Multiply(&euler, &pNodes[i].NodeTM, &pNodes[i].NodeTM);
			jeQuaternion_Multiply(&eulerq, &pNodes[i].Q, &pNodes[i].Q);
			jeXForm3d_Transform(&euler, &pNodes[i].S, &pNodes[i].S);
			jeXForm3d_Transform(&euler, &pNodes[i].T, &pNodes[i].T);
		}
		if(i != NumNodes)
		{
			// must have been an error
			SET_ERROR_CLOSE_RETURN;
		}

		// Add bones to body
		if(AddBones(options, pBody, NumBones, pBoneIDs, NumNodes, pNodes) == MK_FALSE)
		{
			// must have been an error
			SET_ERROR_CLOSE_RETURN;
		}

		// Fix up the VPH verts
		if(pvphdata != NULL)
		{
			if(CalculateWSVPHVerts(pvphdata, NumNodes, pNodes) == MK_FALSE)
			{
				// must have been an error
				SET_ERROR_CLOSE_RETURN;
			}
		}

		// Look for Bounding Box, if not found, move on
#define BBOX_STRING "Bounding Box: "
		FGETS_LINE_OR_CLOSE_AND_RETURN(line);
		while(strstr(line, BBOX_STRING) != NULL)
		{
			// Bounding Box: name
			// minx, miny, minz
			// maxx, maxy, maxz
			// tm
			// q
			// s
			// t
#define NUM_CUBE_POINTS 8
			jeVec3d points[NUM_CUBE_POINTS];
			jeVec3d min, max, t;
			jeXForm3d matrix, parentmatrix;
			jeQuaternion q;

			// read bone
			ptext = line + strlen(BBOX_STRING);
			StripNewLine(ptext);
			if(options->Capitalize != MK_FALSE)
				strupr(ptext);

			for(j=0;j<NumBones;j++)
			{
				if(strcmp(pBoneIDs[j].Name, ptext) == 0)
				{
					break;
				}
			}
			if(j == NumBones)
			{
				// didn't find bone for this box, attach it to the root
				Index = JE_BODY_ROOT;
				jeXForm3d_SetIdentity(&parentmatrix);
			}
			else
			{
				if(jeBody_GetBoneByName(pBody, ptext, &Index, &matrix, &i) == JE_FALSE)
				{
					Printf("ERROR: bone '%s' not in body\n", ptext);
					SET_ERROR_CLOSE_RETURN;
				}

				parentmatrix = pBoneIDs[j].Matrix;
			}

			// read min
			j = fscanf(fp, "%f %f %f\n",	&min.X,
											&min.Y,
											&min.Z);
			if(j != 3)
			{
				Printf("ERROR: Could not read bounding box's min\n");
				SET_ERROR_CLOSE_RETURN;
			}

			// read max
			j = fscanf(fp, "%f %f %f\n",	&max.X,
											&max.Y,
											&max.Z);
			if(j != 3)
			{
				Printf("ERROR: Could not read bounding box's max\n");
				SET_ERROR_CLOSE_RETURN;
			}

			// populate cube points
			for(j=0;j<NUM_CUBE_POINTS;j++)
			{
				// j's bits determine where to get each component
				// x from bit 2, y bit 1, z bit 0
				if(j & 4)
					points[j].X = min.X;
				else
					points[j].X = max.X;
				if(j & 2)
					points[j].Y = min.Y;
				else
					points[j].Y = max.Y;
				if(j & 1)
					points[j].Z = min.Z;
				else
					points[j].Z = max.Z;
			}

			// read matrix
			// we have been ignoring the max matrix and creating one
			// from the quat and translation

			// WS Matrix
			FGETS_LINE_OR_CLOSE_AND_RETURN(line); // matrix row 1
			FGETS_LINE_OR_CLOSE_AND_RETURN(line); // matrix row 2
			FGETS_LINE_OR_CLOSE_AND_RETURN(line); // matrix row 3
			FGETS_LINE_OR_CLOSE_AND_RETURN(line); // matrix row 4

			// Quaternion
			j = fscanf(fp, "Q(w,x,y,z): %f %f %f %f\n",	&q.W,
														&q.X,
														&q.Y,
														&q.Z);
			if(j != 4)
			{
				Printf("ERROR: Could not read box's quaternion\n");
				SET_ERROR_CLOSE_RETURN;
			}

			// Scale
			FGETS_LINE_OR_CLOSE_AND_RETURN(line);

			// Translation
			j = fscanf(fp, "T(x,y,z): %f %f %f\n",	&t.X,
													&t.Y,
													&t.Z);
			if(j != 3)
			{
				Printf("ERROR: Could not read box's translation\n", i);
				break;
			}

			// compute matrix
			jeQuaternion_Normalize(&q);
			jeQuaternion_ToMatrix(&q, &matrix);
			matrix.Translation = t;

			// apply rotation to all data at read time
			jeXForm3d_Multiply(&euler, &matrix, &matrix);

			// move to bone's coord space
			MaxMath_InverseMultiply(&matrix, &parentmatrix, &matrix);

			// invert matrix to transform points into bone space
			jeXForm3d_GetTranspose(&parentmatrix, &parentmatrix);

			// transform the points
			jeXForm3d_TransformVecArray(&parentmatrix, points, points, NUM_CUBE_POINTS);

			// find the min's and max's
			min = points[0];
			max = points[0];
			for(j=1;j<NUM_CUBE_POINTS;j++)
			{
				if(points[j].X < min.X)
					min.X = points[j].X;
				if(points[j].Y < min.Y)
					min.Y = points[j].Y;
				if(points[j].Z < min.Z)
					min.Z = points[j].Z;
				if(points[j].X > max.X)
					max.X = points[j].X;
				if(points[j].Y > max.Y)
					max.Y = points[j].Y;
				if(points[j].Z > max.Z)
					max.Z = points[j].Z;
			}

			// now set the bounding box
			jeBody_SetBoundingBox(pBody, Index, &min, &max);

			// read next line for expectant mesh list or another bounding box
			FGETS_LINE_OR_CLOSE_AND_RETURN(line);
		}

		// Mesh List
		if(strcmp(line, "Mesh List\n") != 0)
		{
			Printf("ERROR: No mesh list in '%s' NFO file\n", options->NFOFile);
			SET_ERROR_CLOSE_RETURN;
		}

		FGETS_LINE_OR_CLOSE_AND_RETURN(line);
		if(sscanf(line, "Number of meshes = %d", &NumMeshes) != 1)
		{
			Printf("ERROR: Could not read number of meshes in '%s' NFO file\n", options->NFOFile);
			Printf("Line from VPH file:\n%s\n",line);
			SET_ERROR_CLOSE_RETURN;
		}

		if(nVersion >= 0x0200)
		{
			while(NumMeshes > 0)
			{
				ReturnCode rval = V2ReadAndAddMesh(options, fp, pBody, pvphdata, pNodes, NumNodes);
				if (rval == RETURN_ERROR)
				{
					SET_ERROR_CLOSE_RETURN;
				}
				MkUtil_AdjustReturnCode(&retValue, rval);
				NumMeshes--;
			}
			Printf("\t%d Faces Processed\n",fCount);

		}
		else if(nVersion == 0x0100)
		{
			while(NumMeshes > 0)
			{
				if (MkUtil_Interrupt())
					{
						Printf("Interrupted\n");
						break;
					}
		
				// name of mesh
				FGETS_LINE_OR_CLOSE_AND_RETURN(line);
				StripNewLine(line);

				ptext = strstr(line, ": ");
				if(ptext != NULL)
					ptext += 2; // length of ": "
				if( (ptext == NULL) || (strlen(ptext) < 1) )
				{
					// there should be some name-like text
					Printf("ERROR: Found no node name: '%s'\n", line);
					break;
				}
				strncpy(name, ptext, NAME_LENGTH);

				// Face List
				FGETS_LINE_OR_CLOSE_AND_RETURN(line);
				if(strcmp(line, "Face List\n") != 0)
				{
					Printf("ERROR: No face list in '%s' NFO file\n", options->NFOFile);
					SET_ERROR_CLOSE_RETURN;
				}

				FGETS_LINE_OR_CLOSE_AND_RETURN(line);
				if(sscanf(line, "Number of Faces = %d", &k) != 1)
				{
					Printf("ERROR: Could not read number of faces in '%s' NFO file\n", options->NFOFile);
					Printf("Line from VPH file:\n%s\n",line);
					SET_ERROR_CLOSE_RETURN;
				}

				while(k > 0)
				{
					// ignore face number
					FGETS_LINE_OR_CLOSE_AND_RETURN(line);

					if(fscanf(fp, "    %d\n", &face.material) != 1)
					{
						Printf("ERROR: Could not read material for face '%s'\n", line);
						SET_ERROR_CLOSE_RETURN;
					}

					for(i=0;i<3;i++)
					{
						pfacevert = face.verts + i;
						j = fscanf(fp, "    %f %f %f %f %f %f %f %f %d\n",	&pfacevert->v.X,
																			&pfacevert->v.Y,
																			&pfacevert->v.Z,
																			&pfacevert->n.X,
																			&pfacevert->n.Y,
																			&pfacevert->n.Z,
																			&pfacevert->tu,
																			&pfacevert->tv,
																			&pfacevert->bone);
						if(j != 9)
						{
							Printf("ERROR: Could not read coords for face '%s'\n", line);
							SET_ERROR_CLOSE_RETURN;
						}

						// apply rotation to all data at read time
						jeXForm3d_Transform(&euler, &pfacevert->v, &pfacevert->v);
						jeXForm3d_Transform(&euler, &pfacevert->n, &pfacevert->n);

						// Textures need to be flipped from 3dsmax
						pfacevert->tv = 1 - pfacevert->tv;

						// If this vert is not assigned to a bone, find one.
						if(pfacevert->bone == -1)
						{
							int n;
							jeXForm3d dummyx, xmatrix;

							if(jeStrBlock_FindString(pvphdata->pSBObjectNames, name, &j) == JE_FALSE)
							{
								Printf("ERROR: Could not find vph data for mesh '%s'\n", name);
								SET_ERROR_CLOSE_RETURN;
							}

							pVPHVerts = pvphdata->ppVerts[j];
							j = pvphdata->NumVerts[j] - 1;
							while(j >= 0)
							{
								if(jeVec3d_Compare(&pVPHVerts[j].WSPoint, &pfacevert->v, VERT_EQUALITY_TOLERANCE) != JE_FALSE)
									break; // found it!

								j--;
							}
							if(j < 0)
							{
								// didn't find vert
								Printf("ERROR: Could not find matching vertex\n");
								SET_ERROR_CLOSE_RETURN;
							}

							for(n=0;n<NumNodes;n++)
							{
								if(strcmp(pNodes[n].Name, pvphdata->pLinks[pVPHVerts[j].bone].Name) == 0)
									break; // found it!
							}
							if(n == NumNodes)
							{
								// didn't find vert
								Printf("ERROR: (4) Could not find node '%s' for vertex\n", pvphdata->pLinks[pVPHVerts[j].bone].Name);
								SET_ERROR_CLOSE_RETURN;
							}

							// Need to do a NoScale
							jeQuaternion_ToMatrix(&pNodes[n].Q, &xmatrix);
							xmatrix.Translation = pNodes[n].T;

							MaxMath_GetInverse(&xmatrix, &xmatrix);

							// fixup the vert
							MaxMath_Transform(&xmatrix, &pfacevert->v, &pfacevert->v);

							// fixup the normal
							jeVec3d_Clear(&xmatrix.Translation);
							MaxMath_Transform(&xmatrix, &pfacevert->v, &pfacevert->v);

							// find the vert's bone
							if(jeBody_GetBoneByName(pBody, pNodes[n].Name, &pfacevert->bone, &dummyx, &n) == JE_FALSE)
							{
								Printf("ERROR: Could not find bone '%s' for vertex\n", pNodes[n].Name);
								SET_ERROR_CLOSE_RETURN;
							}
						}
					}
					fCount++;
					if (fCount%25==0)
						Printf("\t%d Faces Processed\n",fCount);

					if(jeBody_AddFace(pBody,	&face.verts[0].v, &face.verts[0].n, 
												face.verts[0].tu, face.verts[0].tv, 
												face.verts[0].bone,
											&face.verts[1].v, &face.verts[1].n, 
												face.verts[1].tu, face.verts[1].tv, 
												face.verts[1].bone,
											&face.verts[2].v, &face.verts[2].n, 
												face.verts[2].tu, face.verts[2].tv, 
												face.verts[2].bone,
											face.material) == JE_FALSE)
					{
						Printf("ERROR: Could not add face for '%s'\n", line);
						SET_ERROR_CLOSE_RETURN;
					}

					k--;
				}

				NumMeshes--;
			}
		}

		Printf("Optimizing body geometry of actor\n");
		if (jeBody_Optimize(pBody)==JE_FALSE)
			{
				Printf("Error:  Unable to optimize body geometry of actor\n");
				SET_ERROR_CLOSE_RETURN;
			}


		assert(fp != NULL);
		fclose(fp);
		fp = NULL;

		// Rename any existing body file
		{
			char bakname[_MAX_PATH];

			strcpy(bakname, options->BodyFile);
			strcat(bakname, ".bak");
			remove(bakname);
			rename(options->BodyFile, bakname);
		}

		// Write the body
		VF = jeVFile_OpenNewSystem(NULL,JE_VFILE_TYPE_DOS,options->BodyFile,NULL,JE_VFILE_OPEN_CREATE);
		if(VF == NULL)
		{
			Printf("ERROR: Could not open output file '%s'\n", options->BodyFile);
			unlink(options->BodyFile);
			MkUtil_AdjustReturnCode(&retValue, RETURN_ERROR);
		}
		else
		{
			if(jeBody_WriteToFile(pBody, VF) == JE_FALSE)
			{
				Printf("ERROR: Body file '%s' was not written correctly\n", options->BodyFile);
				unlink(options->BodyFile);
				MkUtil_AdjustReturnCode(&retValue, RETURN_ERROR);
			}
			else
			{
				if (jeVFile_Close(VF) == JE_FALSE)
					{
						Printf("ERROR: Body file '%s' was not written correctly\n", options->BodyFile);
						unlink(options->BodyFile);
						MkUtil_AdjustReturnCode(&retValue, RETURN_ERROR);
					}
				else
					{
						Printf("SUCCESS: Body file '%s' written successfully\n", options->BodyFile);
					}
			}
		}

		jeBody_Destroy(&pBody);
		FreeVPHData(&pvphdata);
		jeRam_Free(pBoneIDs);
		jeRam_Free(pNodes);
	}
	else
	{
		if (MkUtil_Interrupt())
			{
				Printf("Interrupted... Could not create Body\n");
			}
		else
			{
				Printf("ERROR: Could not create Body\n");
			}
		MkUtil_AdjustReturnCode(&retValue, RETURN_ERROR);
	}

	remove(vlgfile);

	return retValue;
}

void MkBody_OutputUsage(MkUtil_Printf PrintfCallback)
{
	Printf = PrintfCallback;
	//COLS: 0         1         2         3         4         5         6         7       | 8
	Printf("\n");
	Printf("Builds a body from Eclipse NFO and Physique data from 3DSMax.\n");
	Printf("\n");
	Printf("MKBODY [options] /B<bodyfile> /N<nfofile> /V<vphfile> [/A] [/C] [/R]\n");
	Printf("       [/T<texturepath>]\n");
	Printf("\n");
//	Printf("/Ax,y,z         Specifies Euler angle rotations to apply to the import.  This\n");
//	Printf("                rotation should be duplicated in calls to mkmotion that use\n");
//	Printf("                this body.  The rotations are applied in z-y-x order.\n");
	Printf("/B<bodyfile>    Specifies body file.\n");
	Printf("/C              Capitalize all node names.\n");
	Printf("/N<nfofile>     Specifies the Eclipse NFO file.\n");
	Printf("/R              Permit rotational attachments in the body.\n");
	Printf("/T<texturepath> Specifies the path to append to all texture maps.\n");
	Printf("/V<vphfile>     Specifies the Physique vph file.\n");
	Printf("\n");
	Printf("Any existing body file will be renamed to bodyfile.bak\n");
}

MkBody_Options* MkBody_OptionsCreate()
{
	MkBody_Options* pOptions;

	pOptions = JE_RAM_ALLOCATE_STRUCT(MkBody_Options);
	if(pOptions != NULL)
	{
		*pOptions = DefaultOptions;
	}
	pOptions->ExtraMaterials = jeStrBlock_Create();
	if (pOptions->ExtraMaterials == NULL)
		{
			jeRam_Free(pOptions);
			pOptions = NULL;
		}

	pOptions->nVersion = 0;

	return pOptions;
}

void MkBody_OptionsDestroy(MkBody_Options** ppOptions)
{
	assert(ppOptions != NULL);
	assert(*ppOptions != NULL);

	jeStrBlock_Destroy( &((*ppOptions)->ExtraMaterials) );
	jeRam_Free(*ppOptions);

	*ppOptions = NULL;
}

ReturnCode MkBody_ParseOptionString(MkBody_Options* options, 
							const char* string, MK_Boolean InScript,
							MkUtil_Printf PrintfCallback)
{

	Printf = PrintfCallback;
	ReturnCode retValue = RETURN_SUCCESS;

	assert(options != NULL);
	assert(string != NULL);

#define NO_FILENAME_WARNING Printf("WARNING: '%s' specified with no filename\n", string)

	if( (string[0] == '-') || (string[0] == '/') )
	{
		switch(string[1])
		{
/*
		// Although we once thought we wanted this option, we no longer do.
		// It has already caused one mistake by leaving it in, so it leaves.
		case 'a':
		case 'A':
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
			// hidden feature to diddle with vector compare tolerance
			VERT_EQUALITY_TOLERANCE = (float)atof(string + 2);
			break;

		case 'n':
		case 'N':
			if(string[2] == 0)
			{
				NO_FILENAME_WARNING;
				retValue = RETURN_WARNING;
			}
			else
			{
				if( (InScript != MK_FALSE) && (options->NFOFile[0] != 0) )
				{
					Printf("WARNING: NFO filename in script ignored\n");
					retValue = RETURN_WARNING;
				}
				else
				{
					strcpy(options->NFOFile, string + 2);
				}
			}
			break;

		case 'r':
		case 'R':
			options->RotationInBody = MK_TRUE;
			break;

		case 't':
		case 'T':
			if(string[2] == 0)
			{
				NO_FILENAME_WARNING;
				retValue = RETURN_WARNING;
			}
			else
			{
				if( (InScript != MK_FALSE) && (options->TexturePath[0] != 0) )
				{
					Printf("WARNING: texture path in script ignored\n");
					retValue = RETURN_WARNING;
				}
				else
				{
					int len;

					strcpy(options->TexturePath, string + 2);

					// Be sure there is a slash at the end
					len = (int)strlen(options->TexturePath);
					if( (len > 0) && (options->TexturePath[len - 1] != '/') )
						strcat(options->TexturePath, "/");
				}
			}
			break;

		case 'M':
		case 'm':
			if(string[2] == 0)
			{
				NO_FILENAME_WARNING;
				retValue = RETURN_WARNING;
			}
			else
			{
				if (jeStrBlock_Append(&(options->ExtraMaterials),string+2)==JE_FALSE)
					{
						Printf("ERROR: Unable to store extra material name '%s'\n",string+2);
						retValue = RETURN_ERROR;
					}
			}
			break;

		case 'V':
			options->WriteTextVPH = MK_TRUE;
			// Fall thru to get VPH filename
		case 'v':
			if(string[2] == 0)
			{
				NO_FILENAME_WARNING;
				retValue = RETURN_WARNING;
			}
			else
			{
				if( (InScript != MK_FALSE) && (options->VPHFile[0] != 0) )
				{
					Printf("WARNING: VPH filename in script ignored\n");
					retValue = RETURN_WARNING;
				}
				else
				{
					strcpy(options->VPHFile, string + 2);
				}
			}
			break;

		default:
			retValue = RETURN_NOACTION;
		}
	}

	return retValue;
}
