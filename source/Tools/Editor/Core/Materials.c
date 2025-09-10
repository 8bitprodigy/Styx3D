/****************************************************************************************/
/*  MATERIALS.C                                                                         */
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
#include "vfile.h"
#include "bitmap.h"
#include <string.h>
#include "assert.h"
#include "errorlog.h"
#include "ram.h"
#include "util.h"
#include "jeWorld.h"
#include <stdio.h>
//#include "jeShader.h"
#include "bmp.h"

/* This structure contains the binding of the jeBitmaps to the editable bmps */
typedef struct Material_Struct {
	char* Name;
	char* PrimaryMaterialPath;
	union {
		jeBitmap * PrimaryMaterial;
		jeMaterialSpec* MaterialSpec;
	};
} Material_Struct;



//Creates a Material_Stuct
//Loads the bitmap specifed in the properties
//Intializes the Material struct
//Returns NULL on failure
Material_Struct *Materials_Load( jeEngine* pEngine, jeResourceMgr* pResMgr, char* DirPath, char* Name )
{
	Material_Struct *Material;
	jeVFile *MaterialFile;
	char* extStart;

	assert( DirPath != NULL );
	assert( Name != NULL );


	Material = JE_RAM_ALLOCATE_STRUCT( Material_Struct );
	if( Material == NULL )
	{
		jeErrorLog_Add( JE_ERR_MEMORY_RESOURCE, NULL );
		return( NULL );
	}

	//Temporarily remove ".bmp" from file name to copy to Material Name
	extStart = strchr( Name, '.' );
	if( extStart == NULL )
	{
		jeErrorLog_Add( JE_ERR_DATA_FORMAT, NULL );
		return( NULL );
	}
	*extStart = '\0';

	Material->Name = Util_StrDup( Name );
	if( Material->Name == NULL )
	{
		jeErrorLog_Add( JE_ERR_MEMORY_RESOURCE, NULL );
		return( NULL );
	}

	*extStart = '.';

	// Build full path to material
	//allocate enough for path, back slash, file name, terminating char
	Material->PrimaryMaterialPath = jeRam_Allocate( strlen( DirPath ) + strlen( Name ) + 2 );
	if( Material->PrimaryMaterialPath == NULL )
	{
		jeErrorLog_Add( JE_ERR_MEMORY_RESOURCE, NULL );
		return( NULL );
	}

	strcpy( Material->PrimaryMaterialPath, DirPath );
	strcat( Material->PrimaryMaterialPath, "\\" );
	strcat( Material->PrimaryMaterialPath, Name );

	//Load jeBitmap
	MaterialFile = jeVFile_OpenNewSystem(
		NULL, 
		JE_VFILE_TYPE_DOS, 
		Material->PrimaryMaterialPath, 
		NULL,
		JE_VFILE_OPEN_READONLY  );

	if( MaterialFile == NULL )
	{
		jeErrorLog_Add( JE_ERR_FILEIO_OPEN, Material->PrimaryMaterialPath );
		return( NULL );
	}

	Material->PrimaryMaterial = jeBitmap_CreateFromFile( MaterialFile );
	if( Material->PrimaryMaterial == NULL )
	{
		jeErrorLog_AddString( JE_ERR_FILEIO_READ, "Failed to create bitmap", Material->PrimaryMaterialPath );
		jeVFile_Close( MaterialFile );
		return( NULL );
	}
	if( !jeBitmap_SetMipCount(Material->PrimaryMaterial, 4 ) )
	{
		jeErrorLog_AddString( JE_ERR_SUBSYSTEM_FAILURE, "Failed to create mips", Material->PrimaryMaterialPath );
		jeBitmap_Destroy( &Material->PrimaryMaterial );
		jeVFile_Close( MaterialFile );
		return( NULL );
	}

	jeVFile_Close( MaterialFile );

	return( Material );
}

Material_Struct *Materials_ConvertToJMAT( jeEngine* pEngine, jeResourceMgr* pResMgr, char* DirPath, char* Name )
{
	Material_Struct *Material;
	jeVFile *MaterialFile;
	char* extStart;
	jeBitmap* pBmps;
	jeMaterialSpec_Thumbnail tumbs;

	assert( DirPath != NULL );
	assert( Name != NULL );

	Material = JE_RAM_ALLOCATE_STRUCT( Material_Struct );
	if( Material == NULL )
	{
		jeErrorLog_Add( JE_ERR_MEMORY_RESOURCE, NULL );
		return( NULL );
	}

	//Temporarily remove ".bmp" from file name to copy to Material Name
	extStart = strchr( Name, '.' );
	if( extStart == NULL )
	{
		jeErrorLog_Add( JE_ERR_DATA_FORMAT, NULL );
		return( NULL );
	}
	*extStart = '\0';

	Material->Name = Util_StrDup( Name );
	if( Material->Name == NULL )
	{
		jeErrorLog_Add( JE_ERR_MEMORY_RESOURCE, NULL );
		return( NULL );
	}

	*extStart = '.';

	// Build full path to material
	//allocate enough for path, back slash, file name, terminating char
	Material->PrimaryMaterialPath = jeRam_Allocate( strlen( DirPath ) + strlen( Name ) + 4 );
	if( Material->PrimaryMaterialPath == NULL )
	{
		jeErrorLog_Add( JE_ERR_MEMORY_RESOURCE, NULL );
		return( NULL );
	}

	strcpy( Material->PrimaryMaterialPath, DirPath );
	strcat( Material->PrimaryMaterialPath, "\\" );
	strcat( Material->PrimaryMaterialPath, Name );

	//Load jeBitmap
	MaterialFile = jeVFile_OpenNewSystem(
		NULL, 
		JE_VFILE_TYPE_DOS, 
		Material->PrimaryMaterialPath, 
		NULL,
		JE_VFILE_OPEN_READONLY  );

	extStart = strchr( Material->PrimaryMaterialPath, '.' );
	strcpy(extStart, ".jmat");

	if( MaterialFile == NULL )
	{
		jeErrorLog_Add( JE_ERR_FILEIO_OPEN, Material->PrimaryMaterialPath );
		return( NULL );
	}

	pBmps = jeBitmap_CreateFromFile( MaterialFile );
	if( pBmps == NULL )
	{
		jeErrorLog_AddString( JE_ERR_FILEIO_READ, "Failed to create bitmap", Material->PrimaryMaterialPath );
		jeVFile_Close( MaterialFile );
		return( NULL );
	}
	if( !jeBitmap_SetMipCount(pBmps, 4 ) )
	{
		jeErrorLog_AddString( JE_ERR_SUBSYSTEM_FAILURE, "Failed to create mips", Material->PrimaryMaterialPath );
		jeBitmap_Destroy( &pBmps );
		jeVFile_Close( MaterialFile );
		return( NULL );
	}

	// Create an empty material spec
	Material->MaterialSpec = jeMaterialSpec_Create(pEngine, pResMgr);
	jeMaterialSpec_AddLayerFromBitmap(Material->MaterialSpec, 0, pBmps, Material->Name);

	//now create the thumbnail from the bmps
	if (CreateThumbnails(pBmps, &tumbs))
	    jeMaterialSpec_SetThumbnail(Material->MaterialSpec, &tumbs);

	jeBitmap_Destroy( &pBmps );
	jeRam_Free(tumbs.contents);

	jeVFile_Close( MaterialFile );

	//Create JMAT
	MaterialFile = jeVFile_OpenNewSystem(
		NULL, 
		JE_VFILE_TYPE_DOS, 
		Material->PrimaryMaterialPath, 
		NULL,
		JE_VFILE_OPEN_CREATE  );

	jeMaterialSpec_WriteToFile(Material->MaterialSpec, MaterialFile);

	jeVFile_Close( MaterialFile );

	return( Material );
}

// Same as above but for loading jeMaterialSpec
Material_Struct *Materials_LoadEx( jeEngine* pEngine, jeResourceMgr* pResMgr, char* DirPath, char* Name )
{
	Material_Struct *Material;
	jeVFile *MaterialFile;
	char* extStart;

	assert( DirPath != NULL );
	assert( Name != NULL );


	Material = JE_RAM_ALLOCATE_STRUCT( Material_Struct );
	if( Material == NULL )
	{
		jeErrorLog_Add( JE_ERR_MEMORY_RESOURCE, NULL );
		return( NULL );
	}

	//Temporarily remove ".bmp" from file name to copy to Material Name
	extStart = strchr( Name, '.' );
	if( extStart == NULL )
	{
		jeErrorLog_Add( JE_ERR_DATA_FORMAT, NULL );
		return( NULL );
	}
	*extStart = '\0';

	Material->Name = Util_StrDup( Name );
	if( Material->Name == NULL )
	{
		jeErrorLog_Add( JE_ERR_MEMORY_RESOURCE, NULL );
		return( NULL );
	}

	*extStart = '.';

	// Build full path to material
	//allocate enough for path, back slash, file name, terminating char
	Material->PrimaryMaterialPath = jeRam_Allocate( strlen( DirPath ) + strlen( Name ) + 2 );
	if( Material->PrimaryMaterialPath == NULL )
	{
		jeErrorLog_Add( JE_ERR_MEMORY_RESOURCE, NULL );
		return( NULL );
	}

	strcpy( Material->PrimaryMaterialPath, DirPath );
	strcat( Material->PrimaryMaterialPath, "\\" );
	strcat( Material->PrimaryMaterialPath, Name );

	//Load jeBitmap
	MaterialFile = jeVFile_OpenNewSystem(
		NULL, 
		JE_VFILE_TYPE_DOS, 
		Material->PrimaryMaterialPath, 
		NULL,
		JE_VFILE_OPEN_READONLY  );

	if( MaterialFile == NULL )
	{
		jeErrorLog_Add( JE_ERR_FILEIO_OPEN, Material->PrimaryMaterialPath );
		return( NULL );
	}

	Material->MaterialSpec = jeMaterialSpec_CreateFromFile( MaterialFile, pEngine, pResMgr );
	if( Material->MaterialSpec == NULL )
	{
		jeErrorLog_AddString( JE_ERR_FILEIO_READ, "Failed to create bitmap", Material->PrimaryMaterialPath );
		jeVFile_Close( MaterialFile );
		return( NULL );
	}

	jeVFile_Close( MaterialFile );

	return( Material );
}

const char* Materials_GetName( Material_Struct* Material )
{
	assert( Material );
	assert( Material->Name );
	return( Material->Name );
}

const jeBitmap	*	Materials_GetBitmap( Material_Struct* Material )
{
	assert( Material );
	assert( Material->PrimaryMaterial);
	return( Material->PrimaryMaterial );
}

// Krouer: move slightly from BMP to JMAT
const jeMaterialSpec*	Materials_GetMaterialSpec( Material_Struct* Material )
{
	assert( Material );
	assert( Material->MaterialSpec );
	return( Material->MaterialSpec );
}

void Materials_Destroy( Material_Struct* Material )
{
	assert( Material );

	if( Material->Name != NULL )
		jeRam_Free( Material->Name );

	if( Material->PrimaryMaterialPath != NULL )
		jeRam_Free( Material->PrimaryMaterialPath );
#ifdef _USE_BITMAPS
	if( Material->PrimaryMaterial != NULL )
		jeBitmap_Destroy( &Material->PrimaryMaterial );
#else
	if (Material->MaterialSpec != NULL) {
		jeMaterialSpec_Destroy(&Material->MaterialSpec);
	}
#endif
	jeRam_Free( Material );
}
	

/* EOF: Materials.c */
