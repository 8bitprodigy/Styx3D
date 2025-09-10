/****************************************************************************************/
/*  JERESOURCE.C                                                                        */
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
//
//	Used to manage a list of resources.
//
#include <memory.h>
#include <assert.h>
#include <string.h>
#include "Ram.h"
#include "jeChain.h"
#include "Util.h"
#include "jeResource.h"
#include "Errorlog.h" // Added by Incarnadine
#include "Log.h"
#include "Engine.h"

#ifdef BUILD_BE
#define stricmp strcasecmp
#endif

////////////////////////////////////////////////////////////////////////////////////////
//	jeResourceMgr struct
////////////////////////////////////////////////////////////////////////////////////////
typedef struct jeResourceMgr
{
	jeChain	*List;
	int		RefCount;

	jeEngine* Engine;
} jeResourceMgr;


////////////////////////////////////////////////////////////////////////////////////////
//	jeResource struct
////////////////////////////////////////////////////////////////////////////////////////
typedef struct
{
	char	*Name;
    uint32  Type;
	void	*Data;
	int32	RefCount;
	jeBoolean OpenDir;	// [MLB-ICE] Added by Icestorm

} jeResource;


static jeResourceMgr* g_pSingleResource = NULL;

JETAPI jeResourceMgr* JETCC jeResourceMgr_GetSingleton()
{
	return g_pSingleResource;
}


////////////////////////////////////////////////////////////////////////////////////////
//
//	jeResource_MgrCreate()
//
//	Create a resource manager.
//
////////////////////////////////////////////////////////////////////////////////////////
JETAPI jeResourceMgr * JETCC jeResource_MgrCreate(jeEngine* pEngine)
{
	// locals
	jeResourceMgr	*ResourceMgr;

	// create resource list struct
	ResourceMgr = (jeResourceMgr *)jeRam_AllocateClear( sizeof( *ResourceMgr ) );
	if ( ResourceMgr == NULL )
	{
		return NULL;
	}

	g_pSingleResource = ResourceMgr;

	// init struct
	ResourceMgr->List = jeChain_Create();
	if ( ResourceMgr->List == NULL )
	{
		jeResource_MgrDestroy( &ResourceMgr );
		return NULL;
	}

	// set ref count
	ResourceMgr->RefCount = 1;

	ResourceMgr->Engine = pEngine;

	// all done
	return ResourceMgr;

} // jeResource_MgrCreate()

JETAPI jeBoolean JETCC jeResourceMgr_SetEngine(jeResourceMgr *ResourceMgr, jeEngine* pEngine)
{
	ResourceMgr->Engine = pEngine;
	return JE_TRUE;
}

JETAPI jeEngine* JETCC jeResourceMgr_GetEngine(const jeResourceMgr *ResourceMgr)
{
	return ResourceMgr->Engine;
}

////////////////////////////////////////////////////////////////////////////////////////
//
//	jeResource_MgrIncRefcount()
//
//	Increment a resource managers ref count.
//
////////////////////////////////////////////////////////////////////////////////////////
JETAPI int32 JETCC jeResource_MgrIncRefcount(jeResourceMgr *ResourceMgr )	// manager whose ref count will be incremented
{
	// ensure valid data
	assert( ResourceMgr != NULL );

	// increment ref count
	ResourceMgr->RefCount++;

	// all done
	return ResourceMgr->RefCount;

} // jeResource_MgrIncRefcount()


// [MLB-ICE]
////////////////////////////////////////////////////////////////////////////////////////
//
//  Close a vfile(directory) of Resource Manager
//     by Icestorm
//
////////////////////////////////////////////////////////////////////////////////////////
static jeBoolean JETCC jeResource_CloseDirectory(jeResource* Resource)
{
	jeVFile* pFS = NULL;

	// clean up open directories
	pFS = (jeVFile *)Resource->Data;
	if((pFS == NULL) || (!jeVFile_Destroy(&pFS)))
	{
		jeErrorLog_AddString(JE_ERR_SYSTEM_RESOURCE, ":jeResource_CloseDirectory: jeVFile_Destroy", Resource->Name);
		return JE_FALSE;
	}
	return JE_TRUE;
}
// [MLB-ICE] EOB


////////////////////////////////////////////////////////////////////////////////////////
//
//	jeResource_MgrDestroy()
//
//	Destroy a resource manager.
//
////////////////////////////////////////////////////////////////////////////////////////
JETAPI void JETCC jeResource_MgrDestroy(
	jeResourceMgr	**DeadResourceMgr )	// manager to zap
{

	// locals
	jeResourceMgr	*ResourceMgr;
	jeResource		*CurResource;
	jeChain_Link	*CurNode;
	jeChain_Link	*NextNode;	// [MLB-ICE] Added by Icestorm

	// ensure valid data
	assert( DeadResourceMgr != NULL );
	assert( *DeadResourceMgr != NULL );

	// get list
	ResourceMgr = *DeadResourceMgr;

	// dont destroy it if ref count is not zero
	ResourceMgr->RefCount--;
	assert( ResourceMgr->RefCount >= 0 );
	if ( ResourceMgr->RefCount > 0 )
	{
		*DeadResourceMgr = NULL;
		return;
	}

	// destroy list
	if ( ResourceMgr->List != NULL )
	{

		// free data for each node
		CurNode = jeChain_GetFirstLink( ResourceMgr->List );
		while ( CurNode != NULL )
		{
			// get node data
			CurResource = (jeResource *)jeChain_LinkGetLinkData( CurNode );
			assert( CurResource != NULL );

			// [MLB-ICE]
			NextNode = jeChain_LinkGetNext( CurNode );	
			if ( CurResource->OpenDir==JE_TRUE )		// Cleanup open dirs
				jeResource_CloseDirectory(CurResource);
			// [MLB-ICE] EOB

			// free name
			if ( CurResource->Name != NULL )
			{
				jeRam_Free( CurResource->Name );
			}

			// free resource struct
			jeRam_Free( CurResource );

			// get next node
			CurNode = NextNode;  //CurNode = jeChain_LinkGetNext( CurNode );  [MLB-ICE]
		}

		// destroy the list itself
		jeChain_Destroy( &( ResourceMgr->List ) );

		ResourceMgr->List = NULL;
	}

	// free main struct
	jeRam_Free( ResourceMgr );

	// zap pointer
	*DeadResourceMgr = NULL;

	g_pSingleResource = NULL;

} // jeResource_MgrDestroy()



////////////////////////////////////////////////////////////////////////////////////////
//
//	jeResource_GetNode()
//
//	Get a node by name.
//
////////////////////////////////////////////////////////////////////////////////////////
static jeChain_Link * jeResource_GetNode(
	jeResourceMgr	*ResourceMgr,	// resource list to get it from
    uint32          Type,           // resource type
	char			*Name )			// resource name
{

	// locals
	uint32			Count;
	jeChain_Link	*CurNode;

	if (g_pSingleResource == NULL) {
		return NULL;
	}

	// ensure valid data
	assert( ResourceMgr != NULL );
	assert( ResourceMgr->List != NULL );
	assert( Name != NULL );

	// fail if resource list is empty
	Count = jeChain_GetLinkCount( ResourceMgr->List );
	if ( Count == 0 )
	{
		return NULL;
	}

	CurNode = jeChain_GetFirstLink( ResourceMgr->List );
    while ( CurNode )
	{
		// locals
		jeResource	*Resource;
		int			Compare;

		// get data
		assert( CurNode != NULL );
		Resource = (jeResource *)jeChain_LinkGetLinkData( CurNode );
		assert( Resource != NULL );

		// get compare value
		Compare = stricmp( Name, Resource->Name );

		// if we have found it then return it
        if ( Compare == 0 && (Type==0 || (Type > 0 && Type == Resource->Type)))
		{
			return CurNode;
		}

        CurNode = jeChain_LinkGetNext( CurNode );
	}

	// if we got to here then it was not found
	return NULL;

} // jeResource_GetNode()



////////////////////////////////////////////////////////////////////////////////////////
//
//	jeResource_Add()
//
//	Add a new resource.
//
////////////////////////////////////////////////////////////////////////////////////////
JETAPI jeBoolean JETCC jeResource_Add(
	jeResourceMgr	*ResourceMgr,	// resource list to add it to
	char			*Name,			// name
    uint32          Type,           // type
	void			*Data )			// data
{
	// locals
	jeChain_Link	*CurNode;
	jeChain_Link	*NewNode;
	jeResource		*CurResource;
	jeResource		*NewResource;

	// ensure valid data
	assert( ResourceMgr != NULL );
	assert( ResourceMgr->List != NULL );
	assert( Name != NULL );
	assert( Data != NULL );

	// create new resource struct
	NewResource = (jeResource *)jeRam_Allocate( sizeof( *NewResource ) );
	if ( NewResource == NULL )
	{
		return JE_FALSE;
	}
	NewResource->Name = Util_StrDup( Name );
	NewResource->Data = Data;
	NewResource->RefCount = 1;
    NewResource->Type = Type;
	NewResource->OpenDir = JE_FALSE;	// [MLB-ICE]

	// search for correct node
	CurNode = jeChain_GetFirstLink( ResourceMgr->List );
	while ( CurNode != NULL )
	{
		// get resource
		CurResource = (jeResource *)jeChain_LinkGetLinkData( CurNode );
		assert( CurResource != NULL );

		// data should not already exist
        if (stricmp( Name, CurResource->Name ) == 0 && Type == CurResource->Type) {
            CurResource->RefCount++;
            return JE_TRUE; // resource is present, don't add it a second time
        }

		// if we just passed our spot then break out
		if ( stricmp( Name, CurResource->Name ) < 0 )
		{
			break;
		}

		// get next node
		CurNode = jeChain_LinkGetNext( CurNode );
	}

	// if we reached end of list or if list is empty them just at the new link
	if ( CurNode == NULL )
	{
		return jeChain_AddLinkData( ResourceMgr->List, NewResource );
	}

	// if we got to here then just prepend this link to the current one
	NewNode = jeChain_LinkCreate( NewResource );
	if ( NewNode == NULL )
	{
		return JE_FALSE;
	}
	if ( jeChain_InsertLinkBefore( ResourceMgr->List, CurNode, NewNode ) == JE_FALSE )
	{
		jeChain_LinkDestroy( &NewNode );
		return JE_FALSE;
	}
	return JE_TRUE;

} // jeResource_Add()



////////////////////////////////////////////////////////////////////////////////////////
//
//	jeResource_Get()
//
//	Get an existing resource.
//
////////////////////////////////////////////////////////////////////////////////////////
JETAPI void * JETCC jeResource_Get(
	jeResourceMgr	*ResourceMgr,	// resource list to get it from
	char			*Name )			// resource name
{

	// locals
	jeChain_Link	*CurNode;
	jeResource		*CurResource;

	// ensure valid data
	assert( ResourceMgr != NULL );
	assert( ResourceMgr->List != NULL );
	assert( Name != NULL );

	// fail if node doesn't exist
	CurNode = jeResource_GetNode( ResourceMgr, 0, Name );
	if ( CurNode == NULL )
	{
		return NULL;
	}

	// get data
	CurResource = (jeResource *)jeChain_LinkGetLinkData( CurNode );
	assert( CurResource != NULL );
	CurResource->RefCount++;
	return CurResource->Data;

} // jeResource_Get()


////////////////////////////////////////////////////////////////////////////////////////
//
//	jeResource_Delete()
//
//	Delete an existing resource.
//
////////////////////////////////////////////////////////////////////////////////////////
JETAPI int JETCC jeResource_Delete(
	jeResourceMgr	*ResourceMgr,	// resource list to delete it from
	char			*Name )			// resource name
{
	// locals
	jeChain_Link	*CurNode;
	jeResource		*CurResource;

	if (g_pSingleResource == NULL) {
		return -1;
	}

	// ensure valid data
	assert( ResourceMgr != NULL );
	assert( ResourceMgr->List != NULL );
	assert( Name != NULL );

	// fail if node doesn't exist
	CurNode = jeResource_GetNode( ResourceMgr, 0, Name );
	if ( CurNode == NULL )
	{
		return -1;
	}

	// decrement ref count
	CurResource = (jeResource *)jeChain_LinkGetLinkData( CurNode );
	assert( CurResource != NULL );
	CurResource->RefCount--;
	assert( CurResource->RefCount >= 0 );

	// if ref count if zero then remove it from the list
	if ( CurResource->RefCount == 0 )
	{
		jeChain_RemoveLink( ResourceMgr->List, CurNode );

		// [MLB-ICE]
		// NOTE : THIS WAS NEVER HERE!!!

		// free name
		if ( CurResource->Name != NULL )
		{
			jeRam_Free( CurResource->Name );
		}

		// free resource struct
		jeRam_Free( CurResource );
		// [MLB-ICE] EOB

		return 0;
	}

	// return current ref count
	return CurResource->RefCount;

} // jeResource_Delete()



////////////////////////////////////////////////////////////////////////////////////////
//
//	jeResource_CreateVFileName()
//
//	Create a VFile resource name.
//
////////////////////////////////////////////////////////////////////////////////////////
static char * jeResource_CreateVFileName(
	char	*Name )	// name to base it on
{

	// locals
	char	*VFilePrefix = "__VFILE__";
	char	*NewName;

	// ensure valid data
	assert( Name != NULL );

	// build new name
	NewName = (char *)jeRam_Allocate( strlen( VFilePrefix ) + strlen( Name ) + 1 );
	if ( NewName == NULL )
	{
		return NULL;
	}
	strcpy( NewName, VFilePrefix );
	strcat( NewName, Name );

	// all done
	return NewName;

} // jeResource_CreateVFileName()



////////////////////////////////////////////////////////////////////////////////////////
//
//	jeResource_AddVFile()
//
//	Add a new VFile resource.
//
////////////////////////////////////////////////////////////////////////////////////////
JETAPI jeBoolean JETCC jeResource_AddVFile(
	jeResourceMgr	*ResourceMgr,	// resource list to add it to
	char			*Name,			// name
	jeVFile			*Data )			// data
{

	// locals
	char		*NewName;
	jeBoolean	Result;

	// ensure valid data
	assert( ResourceMgr != NULL );
	assert( ResourceMgr->List != NULL );
	assert( Name != NULL );
	assert( Data != NULL );

	// build new name
	NewName = jeResource_CreateVFileName( Name );
	if ( NewName == NULL )
	{
		return JE_FALSE;
	}

	// add it to the list
    Result = jeResource_Add( ResourceMgr, NewName, JE_RESOURCE_VFS, Data );
	jeRam_Free( NewName );

	// all done
	return Result;

} // jeResource_AddVFile()



////////////////////////////////////////////////////////////////////////////////////////
//
//	jeResource_GetVFile()
//
//	Get an existing VFile resource.
//
////////////////////////////////////////////////////////////////////////////////////////
JETAPI jeVFile * JETCC jeResource_GetVFile(
	jeResourceMgr	*ResourceMgr,	// resource list to get it from
	char			*Name )			// name
{

	// locals
	char		*NewName;
	jeVFile		*Data;

	// ensure valid data
	assert( ResourceMgr != NULL );
	assert( ResourceMgr->List != NULL );
	assert( Name != NULL );

	// build new name
	NewName = jeResource_CreateVFileName( Name );
	if ( NewName == NULL )
	{
		return NULL;
	}

	// get data
	Data = (jeVFile *)jeResource_Get( ResourceMgr, NewName );
	jeRam_Free( NewName );

	// all done
	return Data;

} // jeResource_GetVFile()



////////////////////////////////////////////////////////////////////////////////////////
//
//	jeResource_DeleteVFile()
//
//	Delete an existing VFile resource.
//
////////////////////////////////////////////////////////////////////////////////////////
JETAPI int JETCC jeResource_DeleteVFile(
	jeResourceMgr	*ResourceMgr,	// resource list to delete it from
	char			*Name )			// name
{

	// locals
	char		*NewName;
	int			RefCount;

	// ensure valid data
	assert( ResourceMgr != NULL );
	assert( ResourceMgr->List != NULL );
	assert( Name != NULL );

	// build new name
	NewName = jeResource_CreateVFileName( Name );
	if ( NewName == NULL )
	{
		return -1;
	}

	// delete data
	RefCount = jeResource_Delete( ResourceMgr, NewName );
	jeRam_Free( NewName );

	// all done
	return RefCount;

} // jeResource_DeleteVFile()


// [MLB-ICE]

////////////////////////////////////////////////////////////////////////////////////////
//
//	Set OpenDir flag of an existing resource 
//    by Icestorm
//
////////////////////////////////////////////////////////////////////////////////////////
static jeBoolean jeResource_SetOpenDir(
	jeResourceMgr	*ResourceMgr,	// resource list to get it from
	char			*Name )			// resource name
{

	// locals
	jeChain_Link	*CurNode;
	jeResource		*CurResource;
	char			*NewName;

	// ensure valid data
	assert( ResourceMgr != NULL );
	assert( ResourceMgr->List != NULL );
	assert( Name != NULL );

	// build new name
	NewName = jeResource_CreateVFileName( Name );
	if ( NewName == NULL )
	{
		return JE_FALSE;
	}

	// fail if node doesn't exist
    CurNode = jeResource_GetNode( ResourceMgr, JE_RESOURCE_VFS, NewName );
	jeRam_Free( NewName );

	if ( CurNode == NULL )
	{
		return JE_FALSE;
	}

	// get data
	CurResource = (jeResource *)jeChain_LinkGetLinkData( CurNode );
	assert( CurResource != NULL );
	CurResource->OpenDir=JE_TRUE;
	return JE_TRUE;

}

////////////////////////////////////////////////////////////////////////////////////////
//
//  Open a vfile(directory) for Resource Manager (WITH AutoRemove on ResMgrDestroy)
//  DirName = Path of directory
//  ResName = Alias of this resource
//     by Icestorm
//
////////////////////////////////////////////////////////////////////////////////////////
JETAPI jeBoolean JETCC jeResource_OpenDirectory(jeResourceMgr* pResourceMgr, char* DirName, char* ResName)
{
	jeVFile* pFS = NULL ;
	assert( pResourceMgr != NULL );

	pFS = jeVFile_OpenNewSystem
	(
		NULL, 
		JE_VFILE_TYPE_DOS,
		DirName,
		NULL,
		JE_VFILE_OPEN_READONLY|JE_VFILE_OPEN_DIRECTORY
	);
	if( pFS == NULL )
	{
		jeErrorLog_AddString(JE_ERR_SYSTEM_RESOURCE, "jeResource_OpenDirectory:jeVFile_OpenNewSystem", DirName);
		return JE_FALSE;
	}
	else
	{
		jeResource_AddVFile( pResourceMgr, ResName, pFS );
		jeResource_SetOpenDir( pResourceMgr, ResName);
	}
	return JE_TRUE;
}

//	Create a Resource Manager using the default paths.
//     by Incarnadine, modified by Icestorm [Added autoremove]
////////////////////////////////////////////////////////////////////////////////////////
JETAPI jeResourceMgr* JETCC jeResource_MgrCreateDefault(jeEngine* pEngine)
{
	jeResourceMgr *	pResourceMgr;
//	jeVFile			*	pFS = NULL ;

	pResourceMgr = jeResource_MgrCreate(pEngine);
	if( pResourceMgr == NULL )
		return NULL;

	// open sound vfile
	jeResource_OpenDirectory(pResourceMgr, "Sounds", "Sounds");

	// open bitmap vfile	
	jeResource_OpenDirectory(pResourceMgr, "GlobalMaterials", "GlobalMaterials");
		
	// open actors vfile	
	jeResource_OpenDirectory(pResourceMgr, "Actors", "Actors");

	// open shaders vfile
	jeResource_OpenDirectory(pResourceMgr, "Shaders", "Shaders");
	
/*	pFS = jeVFile_OpenNewSystem
	(
		NULL, 
		JE_VFILE_TYPE_DOS,
		"Sounds",
		NULL,
		JE_VFILE_OPEN_READONLY|JE_VFILE_OPEN_DIRECTORY
	);
	if( pFS == NULL )
	{
		jeErrorLog_AddString(JE_ERR_SYSTEM_RESOURCE, "Level_CreateResourceMgr:jeVFile_OpenNewSystem", "");
	}
	else
	{
		jeResource_AddVFile( pResourceMgr, "Sounds", pFS );
	}


	// open bitmap vfile	
	pFS = jeVFile_OpenNewSystem(	NULL,
									JE_VFILE_TYPE_DOS,
									"GlobalMaterials",
									NULL,
									JE_VFILE_OPEN_READONLY|JE_VFILE_OPEN_DIRECTORY );
	if ( pFS == NULL )
	{
		jeErrorLog_AddString( JE_ERR_SYSTEM_RESOURCE, "Level_CreateResourceMgr:jeVFile_OpenNewSystem", "" );
	}
	else
	{
		jeResource_AddVFile( pResourceMgr, "GlobalMaterials", pFS );
	}

	// open actors vfile	
	pFS = jeVFile_OpenNewSystem(	NULL,
									JE_VFILE_TYPE_DOS,
									"Actors",
									NULL,
									JE_VFILE_OPEN_READONLY|JE_VFILE_OPEN_DIRECTORY );
	if ( pFS == NULL )
	{
		jeErrorLog_AddString( JE_ERR_SYSTEM_RESOURCE, "Level_CreateResourceMgr:jeVFile_OpenNewSystem", "");
	}
	else
	{
		jeResource_AddVFile( pResourceMgr, "Actors", pFS );
	}*/

	return pResourceMgr;
}
// [MLB-ICE] EOB

// Krouer ;: add usefull function to read resource from disk and create them from here
// First declare all functions need to create data
#include "Bitmap.h"
#include "jeMaterial.h"

////////////////////////////////////////////////////////////////////////////////////////
//
//	jeResource_GetResource()
//
//	Get an existing resource of a directory or VFS
//
////////////////////////////////////////////////////////////////////////////////////////
JETAPI void * JETCC jeResource_GetResource(
	jeResourceMgr	*ResourceMgr,	// resource list to get it from
	int32			Type,			// resource kind
	char			*Name )			// resource name
{
	// locals
	jeChain_Link	*CurNode;
	jeResource		*CurResource;

	// ensure valid data
	assert( ResourceMgr != NULL );
	assert( ResourceMgr->List != NULL );
	assert( Name != NULL );

	// fail if node doesn't exist
	CurNode = jeResource_GetNode( ResourceMgr, Type, Name );
	if ( CurNode == NULL ) // if failed, new behavior
	{
		void* Data;
		char* Paks;
		char* ResName;
		char* CopyName;
		jeResource* DirRes;
		jeVFile* Directory;
		jeVFile* ResFile;

		CopyName = Util_StrDup(Name);

		// the resource was not already open
		Paks = strtok(CopyName, ":.");
		ResName = strtok(NULL, ":.");

		// exchange the paks and the resource if no resource found
		if (ResName==NULL) {
			ResName=Paks;
			Paks= NULL;
		}
		if (Paks && strcmp(Paks, "GlobalMaterials") == 0) {
			Paks = NULL;
		}

		if (Paks) {
			// open the first part of the name separate by :
            CurNode = jeResource_GetNode( ResourceMgr, JE_RESOURCE_VFS, jeResource_CreateVFileName(Paks) );

			// fail to find the dir/pak resource
			if (CurNode == NULL) {
				// the resource was not already open
				// open the first part of the name separate by :
				CurNode = jeResource_GetNode( ResourceMgr, JE_RESOURCE_VFS, jeResource_CreateVFileName("GlobalMaterials") );
				DirRes = (jeResource *)jeChain_LinkGetLinkData( CurNode ); 

				// now, we have to add a jeVFile to the ResourceMgr - we add a directory because normally it's already done by the editor or the game
				Directory = (jeVFile *) jeVFile_OpenNewSystem((jeVFile*) DirRes->Data,
							                                  JE_VFILE_TYPE_DOS,
							                                  Paks,
							                                  NULL,
							                                  JE_VFILE_OPEN_READONLY|JE_VFILE_OPEN_DIRECTORY);

				// add it to the resource for the next time
				jeResource_AddVFile(ResourceMgr, Paks, Directory);
				jeResource_SetOpenDir( ResourceMgr, Paks);
			} else {
				DirRes = (jeResource *)jeChain_LinkGetLinkData( CurNode );
				Directory = (jeVFile*) DirRes->Data;
			}
		} else {
			CurNode = jeResource_GetNode( ResourceMgr, JE_RESOURCE_VFS, jeResource_CreateVFileName("GlobalMaterials") );
			// open the resource container
			DirRes = (jeResource *)jeChain_LinkGetLinkData( CurNode ); 
			Directory = (jeVFile*) DirRes->Data;
		}

		// get the name of the sub resource
		if (ResName==NULL) {
			return NULL;
		}

		// Try to locate already 
		CurNode = jeResource_GetNode( ResourceMgr, Type, ResName );
		if (CurNode==NULL) {
			char* ResNameCopy;
			ResNameCopy = (char*) jeRam_Allocate(strlen(ResName)+10);
			strcpy(ResNameCopy, ResName);
            Data = NULL;
			switch (Type) {
			case JE_RESOURCE_BITMAP:
				strcat(ResNameCopy, ".bmp");
				ResFile = jeVFile_Open(Directory, ResNameCopy, JE_VFILE_OPEN_READONLY);
				if (ResFile) {
					Data = jeBitmap_CreateFromFile(ResFile);
				} else {
#ifdef _DEBUG
					Log_Printf("Replace %s by jet3d.bmp\n", ResNameCopy);
#endif
					ResFile = jeVFile_Open(Directory, "Jet3D.bmp", JE_VFILE_OPEN_READONLY);
					Data = jeBitmap_CreateFromFile(ResFile);
				}
				break;
			case JE_RESOURCE_MATERIAL:
				strcat(ResNameCopy, ".jmat");
				ResFile = jeVFile_Open(Directory, ResNameCopy, JE_VFILE_OPEN_READONLY);
				if (ResFile) {
					Data = jeMaterialSpec_CreateFromFile(ResFile, ResourceMgr->Engine, ResourceMgr);
                } else {
#ifdef _DEBUG
					Log_Printf("Replace %s by jet3d.jmat\n", ResNameCopy);
#endif
/*
                    ResFile = jeVFile_Open(Directory, "Jet3D.jmat", JE_VFILE_OPEN_READONLY);
                    Data = jeMaterialSpec_CreateFromFile(ResFile, ResourceMgr->Engine, ResourceMgr);
*/
                    Data = jeResource_GetResource(ResourceMgr, JE_RESOURCE_MATERIAL, "jet3d");
                }
				break;
			case JE_RESOURCE_TEXTURE:
				{
				char* extension = ResNameCopy + strlen(ResName);
				strcpy(extension, ".png");
				ResFile = jeVFile_Open(Directory, ResNameCopy, JE_VFILE_OPEN_READONLY);
				if (ResFile == NULL) {
					strcpy(extension, ".bmp");
					ResFile = jeVFile_Open(Directory, ResNameCopy, JE_VFILE_OPEN_READONLY);
				}
				if (ResFile != NULL) {
					Data = jeEngine_CreateTextureFromFile(ResourceMgr->Engine, ResFile);
                } else {
#ifdef _DEBUG
					Log_Printf("Replace %s by jet3d.bmp\n", ResNameCopy);
#endif
					ResFile = jeVFile_Open(Directory, "Jet3D.bmp", JE_VFILE_OPEN_READONLY);
                    if (ResFile) {
					    Data = jeEngine_CreateTextureFromFile(ResourceMgr->Engine, ResFile);
                    }
                }
				}
				break;
			default:
				ResFile = jeVFile_Open(Directory, "Jet3D.bmp", JE_VFILE_OPEN_READONLY);
                if (ResFile) {
				    Data = jeBitmap_CreateFromFile(ResFile);
                }
				break;
			}
			if (ResFile) {
				jeVFile_Close(ResFile);
            }
            if (Data) {
				jeResource_Add(ResourceMgr, Name, Type, Data);
			}
			jeRam_Free(ResNameCopy);
		} else {
			DirRes = (jeResource *)jeChain_LinkGetLinkData( CurNode );
			DirRes->RefCount++;
			Data = DirRes->Data;
		}

		jeRam_Free(CopyName);
		return Data;
	}

	// fallback in old behavior of jeResource_Get

	// get data
	CurResource = (jeResource *)jeChain_LinkGetLinkData( CurNode );
	assert( CurResource != NULL );
	CurResource->RefCount++;
	switch (Type) {
	case JE_RESOURCE_BITMAP:
        {
            jeBitmap* pBitmap = (jeBitmap*) CurResource->Data;
            jeBitmap_CreateRef(pBitmap);
        }
        break;
	case JE_RESOURCE_MATERIAL:
        {
            jeMaterialSpec* pMatSpec = (jeMaterialSpec*) CurResource->Data;
            jeMaterialSpec_CreateRef(pMatSpec);
        }
        break;
	case JE_RESOURCE_TEXTURE:
        break;
    }
	return CurResource->Data;

} // jeResource_GetResource()

// export to jet 3D bitmap format
JETAPI void JETCC jeResource_ExportResource(jeResourceMgr *ResourceMgr, int32 Type, char *Name, void* Data)
{
	// locals
	jeChain_Link	*CurNode;
	jeResource		*CurResource;
	
	jeVFile* Directory;

	CurNode = jeResource_GetNode( ResourceMgr, JE_RESOURCE_VFS, jeResource_CreateVFileName("GlobalMaterials") );
	CurResource = (jeResource *)jeChain_LinkGetLinkData( CurNode ); 
	if (CurResource->OpenDir) {
		char* ResNameCopy;
		jeVFile* ResFile;
		ResNameCopy = (char*) jeRam_Allocate(strlen(Name)+10);
		strcpy(ResNameCopy, Name);
		Directory = (jeVFile*) CurResource->Data;
		switch (Type) {
		case JE_RESOURCE_BITMAP:
			strcat(ResNameCopy, ".bmp");
			ResFile = jeVFile_Open(Directory, ResNameCopy, JE_VFILE_OPEN_READONLY);
			if (!ResFile) {
				ResFile = jeVFile_Open(Directory, ResNameCopy, JE_VFILE_OPEN_CREATE);
				jeBitmap_WriteToFile((jeBitmap*)Data, ResFile);
				jeVFile_Close(ResFile);
			}
			break;
		}
		jeRam_Free(ResNameCopy);
	}
}

// Krouer 08/16/2005
// release identified resource
JETAPI jeBoolean JETCC jeResource_ReleaseResource(jeResourceMgr *ResourceMgr, int32 Type, char *Name)
{
	// locals
	jeChain_Link	*CurNode;
	jeResource		*CurResource;

	// ensure valid data
	assert( ResourceMgr != NULL );
	assert( ResourceMgr->List != NULL );
	assert( Name != NULL );

	// fail if node doesn't exist
	CurNode = jeResource_GetNode( ResourceMgr, Type, Name );

	if (CurNode) {
		// Get access to the ressource
		CurResource = (jeResource *)jeChain_LinkGetLinkData( CurNode );
		CurResource->RefCount--;

		if (CurResource->RefCount == 0) {
			// Free the texture when fully managed from here
			if (Type == JE_RESOURCE_TEXTURE && CurResource->Data) {
				jeEngine_DestroyTexture(ResourceMgr->Engine, (jeTexture*) CurResource->Data);
				CurResource->Data = NULL;
			}

			// Remove the link
			jeChain_RemoveLink( ResourceMgr->List, CurNode );

			// free name
			if ( CurResource->Name != NULL )
			{
				jeRam_Free( CurResource->Name );
			}

			// free resource struct
			jeRam_Free( CurResource );
		}

		return JE_TRUE;
	}

	return JE_FALSE;
}

