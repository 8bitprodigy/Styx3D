/****************************************************************************************/
/*  JEOBJECTIO.C                                                                        */
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
#include <string.h>

#include "jeWorld.h"
#include "Object.h"
#include "Ram.h"

typedef struct jeObjectIO
	{
	jeWorld *pWorld;
	//jeVFile *File;
	}jeObjectIO;

jeObjectIO *jeObjectIO_Create(jeWorld *pWorld)
	{
	jeObjectIO *jeIO;

	assert(pWorld);

	jeIO = (jeObjectIO *)jeRam_Allocate(sizeof(jeObjectIO));
	if (jeIO == NULL)
		{
		return NULL;
		}

	jeIO->pWorld = pWorld;
	return (jeIO);
	}

void jeObjectIO_Destroy(jeObjectIO **jeIO)
	{
	assert(jeIO);

	jeRam_Free(*jeIO);
	*jeIO = NULL;
	}

jeBoolean jeObjectIO_WriteObject(jeObjectIO *jeIO, jeVFile *File, jePtrMgr *PtrMgr, jeObject *Obj)
	{
	const char *Name;
	int32 StrSize;

	assert(jeIO);
	assert(File);
	assert(Obj);

	Name = jeObject_GetName(Obj);

	if (Name == NULL)
		{
		Name = "NULL";
		}
	
	StrSize = strlen(Name)+1;
	if (jeVFile_Write(File, &StrSize, sizeof(StrSize)) == 0)
		{
		return JE_FALSE;
		}

	if (jeVFile_Write(File, Name, StrSize) == 0)
		{
		return JE_FALSE;
		}

	return JE_TRUE;
	}

jeBoolean jeObjectIO_ReadObject(jeObjectIO *jeIO, jeVFile *File, jePtrMgr *PtrMgr, jeObject **Obj)
	{
	char Name[1024];
	jeObject *FoundObj;
	int32 StrSize;

	assert(jeIO);
	assert(File);
	assert(Obj);

	if (jeVFile_Read(File, &StrSize, sizeof(StrSize)) == 0)
		{
		return JE_FALSE;
		}

	if (jeVFile_Read(File, Name, StrSize) == 0)
		{
		return JE_FALSE;
		}

	if (strcmp(Name, "NULL") == 0)
		{
		*Obj = NULL;
		return JE_TRUE;
		}
	
	if (FoundObj = jeWorld_FindObjectFromName(jeIO->pWorld, Name))
		{
		*Obj = FoundObj;
		jeObject_CreateRef(*Obj); // ref it
		return JE_TRUE;
		}
	else
		{
		jeVFile *ObjFile;
		jeVFile *ObjDir;

		ObjDir = jeWorld_GetObjectDirectory(jeIO->pWorld);
		if (ObjDir == NULL)
			{
			return JE_FALSE;
			}

		ObjFile = jeVFile_Open(ObjDir, Name, JE_VFILE_OPEN_READONLY);
		if (ObjFile)
			{
			// IMPORTANT: jeObject_CreateFromFile() automatically
			// reads an object header and calls the correct object
			// method CreateFromFile()
			*Obj = jeObject_CreateFromFile(ObjFile, PtrMgr);
			if (*Obj == NULL)
				{
				return JE_FALSE;
				}
			return JE_TRUE;
			}
		}

	return JE_FALSE;
	}

