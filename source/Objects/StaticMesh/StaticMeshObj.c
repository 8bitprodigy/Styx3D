/**
   @file StaticMeshObj.c
                                                                                      
   @author Anthony Rufrano	                                                          
   @brief Static mesh object code     		                                          
                                                                                      
	@par Licence
	The contents of this file are subject to the Jet3D Public License       
	Version 1.02 (the "License"); you may not use this file except in         
	compliance with the License. You may obtain a copy of the License at       
	http://www.jet3d.com                                                        
                                                                             
	@par
	Software distributed under the License is distributed on an "AS IS"           
	basis, WITHOUT WARRANTY OF ANY KIND, either express or implied.  See           
	the License for the specific language governing rights and limitations          
	under the License.                                                               
                                                                                  
	@par
	The Original Code is Jet3D, released December 12, 1999.                            
	Copyright (C) 1996-1999 Eclipse Entertainment, L.L.C. All Rights Reserved           
*/
#include <windows.h>
#include <assert.h>
#include <string.h>
#include <float.h>
#include "VFile.h"
#include "jeProperty.h"
#include "Ram.h"
#include "jeResource.h"
#include "jeWorld.h"
#include "StaticMeshObj.h"
//#include "Resource.h"
#include "jeStaticMesh.h"
#include "jeVersion.h"
#include "Errorlog.h"

//NOTE BY TRILOBITE
//This file is not complete. Missing many functions declared in its associated ObjectDef.c file. It will compile, but will not build due to many link errors

#define STATICMESH_OBJECT_VERSION						1

enum
{
	SMESH_FILENAME_ID = PROPERTY_LOCAL_DATATYPE_START,
	SMESH_LAST_ID
};

enum
{
	SMESH_FILENAME_INDEX = 0,
	SMESH_LAST_INDEX
};

static HINSTANCE						hObjInstance = NULL;
static jeProperty						StaticMeshProperties[SMESH_LAST_INDEX];
static jeProperty_List					StaticMeshPropertyList = { SMESH_LAST_INDEX, &(StaticMeshProperties[0]) };
static char								**FileList = NULL;
static int								TotalFiles = 0;

typedef struct StaticMeshObject
{
	jeStaticMesh						*sm;
	
	jeWorld								*World;
	jeResourceMgr						*ResMgr;
	jeEngine							*Engine;

	jeXForm3d							XForm;

	int									RefCount;
} StaticMeshObject;

static jeBoolean BuildFileList(jeResourceMgr *ResMgr)
{
	jeVFile								*Dir = NULL, *VFS = NULL;
	jeVFile_Finder						*Finder = NULL;
	int									CurrFile = 0;

	Dir = jeResource_GetVFile(ResMgr, "StaticMesh");
	if (!Dir)
	{
		if (!jeResource_OpenDirectory(ResMgr, "StaticMesh", "StaticMesh"))
			return JE_FALSE;

		Dir = jeResource_GetVFile(ResMgr, "StaticMesh");
		if (!Dir)
			return JE_FALSE;
	}

	Finder = jeVFile_CreateFinder(Dir, "*");
	if (!Finder)
		return JE_FALSE;

	while (jeVFile_FinderGetNextFile(Finder) == JE_TRUE)
	{
		jeVFile_Properties				Props;

		jeVFile_FinderGetProperties(Finder, &Props);
		if (Props.AttributeFlags & JE_VFILE_ATTRIB_DIRECTORY)
		{
			jeVFile_Finder				*DirFinder = NULL;
			jeVFile						*SubDir = NULL;

			SubDir = jeVFile_OpenNewSystem(Dir, JE_VFILE_TYPE_DOS, Props.Name, NULL, JE_VFILE_OPEN_READONLY | JE_VFILE_OPEN_DIRECTORY);
			if (!SubDir)
			{
				jeVFile_DestroyFinder(Finder);
				return JE_FALSE;
			}

			DirFinder = jeVFile_CreateFinder(SubDir, "*.jsm");
			if (!DirFinder)
			{
				jeVFile_Close(SubDir);
				jeVFile_DestroyFinder(Finder);
				return JE_FALSE;
			}

			while (jeVFile_FinderGetNextFile(DirFinder) == JE_TRUE)
			{
				TotalFiles++;
			}

			jeVFile_DestroyFinder(DirFinder);
			jeVFile_Close(SubDir);
		}
		else if (!strcmp(Props.Name, ".jetpak"))
		{
			jeVFile						*PakFile = NULL;
			jeVFile_Finder				*PakFinder = NULL;

			PakFile = jeVFile_OpenNewSystem(Dir, JE_VFILE_TYPE_VIRTUAL, Props.Name, NULL, JE_VFILE_OPEN_READONLY | JE_VFILE_OPEN_DIRECTORY);
			if (!PakFile)
			{
				jeVFile_DestroyFinder(Finder);
				return JE_FALSE;
			}

			PakFinder = jeVFile_CreateFinder(PakFile, "*.jsm");
			if (!PakFinder)
			{
				jeVFile_Close(PakFile);
				jeVFile_DestroyFinder(Finder);
				return JE_FALSE;
			}

			while (jeVFile_FinderGetNextFile(PakFinder) == JE_TRUE)
			{
				TotalFiles++;
			}

			jeVFile_DestroyFinder(PakFinder);
			jeVFile_Close(PakFile);
		}
	}

	jeVFile_DestroyFinder(Finder);

	FileList = jeRam_AllocateClear(sizeof(char*) * TotalFiles);
	if (!FileList)
		return JE_FALSE;

	Finder = jeVFile_CreateFinder(Dir, "*");
	if (!Finder)
		return JE_FALSE;

	while (jeVFile_FinderGetNextFile(Finder) == JE_TRUE)
	{
		jeVFile_Properties				Props;

		jeVFile_FinderGetProperties(Finder, &Props);
		if (Props.AttributeFlags & JE_VFILE_ATTRIB_DIRECTORY)
		{
			jeVFile_Finder				*DirFinder = NULL;
			jeVFile						*SubDir = NULL;

			SubDir = jeVFile_OpenNewSystem(Dir, JE_VFILE_TYPE_DOS, Props.Name, NULL, JE_VFILE_OPEN_READONLY | JE_VFILE_OPEN_DIRECTORY);
			if (!SubDir)
			{
				jeVFile_DestroyFinder(Finder);
				return JE_FALSE;
			}

			DirFinder = jeVFile_CreateFinder(SubDir, "*.jsm");
			if (!DirFinder)
			{
				jeVFile_Close(SubDir);
				jeVFile_DestroyFinder(Finder);
				return JE_FALSE;
			}

			while (jeVFile_FinderGetNextFile(DirFinder) == JE_TRUE)
			{
				jeVFile_Properties				FileProps;

				jeVFile_FinderGetProperties(DirFinder, &FileProps);
				FileList[CurrFile++] = FileProps.Name;
			}

			jeVFile_DestroyFinder(DirFinder);
			jeVFile_Close(SubDir);
		}
		else if (!strcmp(Props.Name, ".jetpak"))
		{
			jeVFile						*PakFile = NULL;
			jeVFile_Finder				*PakFinder = NULL;

			PakFile = jeVFile_OpenNewSystem(Dir, JE_VFILE_TYPE_VIRTUAL, Props.Name, NULL, JE_VFILE_OPEN_READONLY | JE_VFILE_OPEN_DIRECTORY);
			if (!PakFile)
			{
				jeVFile_DestroyFinder(Finder);
				return JE_FALSE;
			}

			PakFinder = jeVFile_CreateFinder(PakFile, "*.jsm");
			if (!PakFinder)
			{
				jeVFile_Close(PakFile);
				jeVFile_DestroyFinder(Finder);
				return JE_FALSE;
			}

			while (jeVFile_FinderGetNextFile(PakFinder) == JE_TRUE)
			{
				jeVFile_Properties				FileProps;

				jeVFile_FinderGetProperties(PakFinder, &FileProps);
				FileList[CurrFile++] = FileProps.Name;
			}

			jeVFile_DestroyFinder(PakFinder);
			jeVFile_Close(PakFile);
		}
	}

	jeVFile_DestroyFinder(Finder);
	return JE_TRUE;
}

void Init_Class(HINSTANCE hInstance)
{
	assert(hInstance != NULL);

	hObjInstance = hInstance;
}

void Deinit_Class()
{
	if (FileList)
	{
		int							i;

		for (i = 0; i < TotalFiles; i++)
			jeRam_Free(FileList[i]);

		jeRam_Free(FileList);
	}
}

void * JETCC CreateInstance()
{
	StaticMeshObject				*Mesh;

	Mesh = (StaticMeshObject*)jeRam_AllocateClear(sizeof(StaticMeshObject));
	if (!Mesh)
		return NULL;

	Mesh->RefCount = 1;
	Mesh->sm = NULL;
	Mesh->World = NULL;
	Mesh->Engine = NULL;
	Mesh->ResMgr = NULL;
	
	jeXForm3d_SetIdentity(&Mesh->XForm);

	return (void*)Mesh;
}

void JETCC CreateRef(void *Instance)
{
	StaticMeshObject			*Mesh = (StaticMeshObject*)Instance;

	assert(Mesh != NULL);

	Mesh->RefCount++;
}

jeBoolean JETCC Destroy(void **Instance)
{
	StaticMeshObject			**pSM = (StaticMeshObject**)Instance;

	jeStaticMesh_Destroy(&(*pSM)->sm);
	jeRam_Free((*pSM));

	(*pSM) = NULL;
	return JE_TRUE;
}

jeBoolean JETCC Render(const void *Instance, const jeWorld *World, const jeEngine *Engine, const jeCamera *Camera, const jeFrustum *CameraSpaceFrustum, jeObject_RenderFlags RenderFlags)
{
	StaticMeshObject			*Mesh = (StaticMeshObject*)Instance;


	//ORIG commented out by trilobite
	//  return jeStaticMesh_Render(Mesh->sm, (const jeEngine*)Engine, (const jeCamera*)Camera, (const jeFrustum*)CameraSpaceFrustum, &Mesh->XForm);
	//
	// EXPERIMENTAL REPLACEMENT BELOW BY trilobite
	return jeStaticMesh_Render(Mesh->sm, (jeEngine*)Engine, (jeCamera*)Camera, (jeFrustum*)CameraSpaceFrustum, &Mesh->XForm);
	
}

jeBoolean JETCC AttachWorld(void *Instance, jeWorld *World)
{
	StaticMeshObject			*Mesh = (StaticMeshObject*)Instance;

	if (Mesh->World != NULL)
		DetachWorld((void*)Mesh, Mesh->World);

	Mesh->World = World;
	Mesh->ResMgr = jeWorld_GetResourceMgr(World);

	return JE_TRUE;
}

jeBoolean JETCC DetachWorld(void *Instance, jeWorld *World)
{
	StaticMeshObject			*Mesh = (StaticMeshObject*)Instance;

	if (Mesh->World != World)
		return JE_FALSE;

	Mesh->World = NULL;
	Mesh->ResMgr = NULL;

	return JE_TRUE;
}

jeBoolean JETCC AttachEngine(void *Instance, jeEngine *Engine)
{
	StaticMeshObject			*Mesh = (StaticMeshObject*)Instance;

	if (Mesh->Engine)
		DetachEngine((void*)Mesh, Mesh->Engine);

	Mesh->Engine = Engine;
	return JE_TRUE;
}

jeBoolean JETCC DetachEngine(void *Instance, jeEngine *Engine)
{
	StaticMeshObject			*Mesh = (StaticMeshObject*)Instance;

	if (Mesh->Engine != Engine)
		return JE_FALSE;

	Mesh->Engine = NULL;

	return JE_TRUE;
}

jeBoolean JETCC AttachSoundSystem(void *Instance, jeSound_System *SoundSys)
{
	Instance;
	SoundSys;

	return JE_TRUE;
}

jeBoolean JETCC DetachSoundSystem(void *Instance, jeSound_System *SoundSys)
{
	Instance;
	SoundSys;

	return JE_TRUE;
}

jeBoolean JETCC Collision(const void *Instance, const jeExtBox *Box, const jeVec3d *Front, const jeVec3d *Back, jeVec3d *Impact, jePlane *Plane)
{
	Impact = NULL;
	Plane = NULL;
	return JE_FALSE;
}

jeBoolean JETCC GetExtBox(const void *Instance, jeExtBox *BBox)
{
	StaticMeshObject				*Mesh = (StaticMeshObject*)Instance;

	return jeStaticMesh_GetExtBox(Mesh, BBox);
}

void * JETCC CreateFromFile(jeVFile *File, jePtrMgr *PtrMgr)
{
	return NULL;
}
