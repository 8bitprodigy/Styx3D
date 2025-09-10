/****************************************************************************************/
/*  MATERIALLIST.C                                                                      */
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
#include "jelist.h"
#include <string.h>
#include "assert.h"
#include "errorlog.h"
#include "ram.h"
#include "util.h"
#include <stdio.h>
#include "materials.h"
#include "materiallist.h"

typedef struct MaterialList_Struct {
	List* pList;
	Material_Struct* CurMaterial;
}	MaterialList_Struct;


//Creates a material list.  
MaterialList_Struct* MaterialList_Create(  )
{
	MaterialList_Struct* MaterialList;

	MaterialList = JE_RAM_ALLOCATE_STRUCT( MaterialList_Struct );
	if( MaterialList == NULL )
	{
		jeErrorLog_Add( JE_ERR_MEMORY_RESOURCE, NULL );
		return( NULL );
	}
	MaterialList->pList = List_Create ();
	if( MaterialList->pList == NULL )
		return( NULL );
	MaterialList->CurMaterial = NULL;

	return( MaterialList );
}

void MaterialList_Destroy( MaterialList_Struct **hMaterialList )
{
	if( (*hMaterialList)->pList != NULL )
		List_Destroy ( &(*hMaterialList)->pList, (List_DestroyCallback) Materials_Destroy );

	jeRam_Free( (*hMaterialList) );
}

int32 MaterialList_GetNumItems( const MaterialList_Struct * pMaterials )
{
	assert( pMaterials != NULL ) ;

	return List_GetNumItems( pMaterials->pList ) ;
}// MaterialList_GetNumItems

Material_Struct *	MaterialList_GetMaterial( MaterialList_Struct * MaterialList, MaterialIterator  pMI )
{
	assert( pMI );
	assert( MaterialList );
	assert( MaterialList->pList );

	return( (Material_Struct *)List_GetData( pMI ) );
}

Material_Struct *	MaterialList_GetFirstMaterial( MaterialList_Struct * MaterialList, MaterialIterator * pMI )
{
	assert( pMI );
	assert( MaterialList );
	assert( MaterialList->pList );

	return( (Material_Struct *)List_GetFirst( MaterialList->pList, pMI ) );
}

Material_Struct *	MaterialList_GetNextMaterial( MaterialList_Struct * MaterialList, MaterialIterator * pMI )
{
	assert( pMI );
	assert( MaterialList );
	assert( MaterialList->pList );

	return( (Material_Struct *)List_GetNext( MaterialList->pList, pMI ) );
}


// Materials_DoesExist
jeBoolean Materials_DoesExist( Material_Struct * pMaterial, void * pVoid )
{
	return (strcmp(Materials_GetName(pMaterial), (char*) pVoid) != 0);
}


// Parses the diretory specified by DirPath for ".bmp" extention
// Creates a material for each found bmp and adds it to MaterialList
jeBoolean MaterialList_LoadFromDir( MaterialList_Struct* MaterialList, jeEngine* pEngine, jeResourceMgr* pResMgr, char* DirPath )
{
#define JMAT_SPEC "*.jmat"
#define BMP_SPEC "*.bmp"
	char	AppPath[255];
	jeVFile *Directory;
	jeVFile_Finder *Finder;
	jeVFile_Properties Properties;
	Material_Struct *Material;

	assert( MaterialList != NULL);
	assert( DirPath != NULL );

	Util_GetAppPath( AppPath, 255 );
	//strcat( AppPath, "\\" );
	strcat( AppPath, DirPath );
	Directory = jeVFile_OpenNewSystem(
		NULL, 
		JE_VFILE_TYPE_DOS, 
		AppPath, 
		NULL,
		JE_VFILE_OPEN_READONLY | JE_VFILE_OPEN_DIRECTORY );

	if( Directory == NULL )
	{
		jeErrorLog_AddString( JE_ERR_FILEIO_OPEN, "Unable to open directory", DirPath );
		return( JE_FALSE );
	}

#ifndef _USE_BITMAPS
	// Krouer: modify to load first JMAT file
	{
		Finder = jeVFile_CreateFinder( Directory, JMAT_SPEC );

		if( Finder == NULL )
		{
			jeErrorLog_AddString( JE_ERR_FILEIO_READ, "Unable to search directory", DirPath );
			return( JE_FALSE );
		}

		while(  jeVFile_FinderGetNextFile(Finder) )
		{
			if( jeVFile_FinderGetProperties(Finder, &Properties ) == JE_FALSE )
			{
				jeErrorLog_AddString( JE_ERR_FILEIO_READ, "Unable to search directory", DirPath );
				return( JE_FALSE );
			}
			// Krouer: Load the JMAT
			Material = Materials_LoadEx( pEngine, pResMgr, AppPath, Properties.Name );
			if(  Material == NULL )
			{
				jeErrorLog_Add( JE_ERR_FILEIO_READ, NULL );
				return( JE_FALSE );
			}
			List_Append( MaterialList->pList, Material );
		}

		jeVFile_DestroyFinder( Finder );
	}
	
	// convert the BMP left
	Finder = jeVFile_CreateFinder( Directory, BMP_SPEC );
	if( Finder != NULL )
	{
		MaterialIterator MI = NULL;
		while(  jeVFile_FinderGetNextFile(Finder) )
		{
			if( jeVFile_FinderGetProperties(Finder, &Properties ) == JE_FALSE )
			{
				jeErrorLog_AddString( JE_ERR_FILEIO_READ, "Unable to search directory", DirPath );
				return( JE_FALSE );
			}
			
			if (MaterialList_SearchByName(MaterialList, &MI, Properties.Name) == NULL) {
				// Krouer: Decode the bitmap to JMAT or load the JMAT
				Material = Materials_ConvertToJMAT( pEngine, pResMgr, AppPath, Properties.Name );

				if(  Material == NULL )
				{
					jeErrorLog_Add( JE_ERR_FILEIO_READ, NULL );
					return( JE_FALSE );
				}
				List_Append( MaterialList->pList, Material );
			}
		}
		jeVFile_DestroyFinder( Finder );
	}
#else
	// convert the BMP left
	Finder = jeVFile_CreateFinder( Directory, BMP_SPEC );
	if( Finder != NULL )
	{
		MaterialIterator MI = NULL;
		while(  jeVFile_FinderGetNextFile(Finder) )
		{
			if( jeVFile_FinderGetProperties(Finder, &Properties ) == JE_FALSE )
			{
				jeErrorLog_AddString( JE_ERR_FILEIO_READ, "Unable to search directory", DirPath );
				return( JE_FALSE );
			}
			
			Material = Materials_Load( pEngine, pResMgr, AppPath, Properties.Name );

			if(  Material == NULL )
			{
				jeErrorLog_Add( JE_ERR_FILEIO_READ, NULL );
				return( JE_FALSE );
			}
			List_Append( MaterialList->pList, Material );
		}
		jeVFile_DestroyFinder( Finder );
	}
#endif

	jeVFile_Close( Directory );
	return( JE_TRUE );
}

Material_Struct*	MaterialList_GetCurMaterial( MaterialList_Struct* MaterialList )
{
	assert( MaterialList );

	return( MaterialList->CurMaterial );
}

void MaterialList_SetCurMaterial( MaterialList_Struct* MaterialList, Material_Struct* Material )
{
	assert( MaterialList );
	assert( Material );

	MaterialList->CurMaterial = Material;
}

// ENUMERATION

int32 MaterialList_EnumMaterials( MaterialList_Struct * pList, void * pVoid, MaterialListCB Callback )
{
	assert( pList != NULL ) ;

	return List_ForEach( pList->pList, Callback, pVoid ) ;

}// MaterialList_EnumMaterials


static jeBoolean MaterialList_SearchNameCB(void *pData, void *lParam)
{
	Material_Struct* Material = (Material_Struct*)pData;
	char* Name = lParam;

	assert( pData );
	assert( lParam );

	return( !strcmp( Materials_GetName( Material ) , Name ) );
} 

Material_Struct*	MaterialList_SearchByName( MaterialList_Struct* MaterialList, MaterialIterator * pMI, char* Name )
{
	char* extChar;
	Material_Struct* Material;
	
	assert( MaterialList );
	assert( pMI );
	assert( Name );

	extChar = strrchr(Name, '.');
	if (extChar) {
		// tmp remove the extension
		*extChar = 0;
	}

	if( !List_Search ( MaterialList->pList, MaterialList_SearchNameCB, Name, &Material, pMI ) ) {
		// re-enable the extension
        if (extChar)
		    *extChar = '.';
		return( NULL );
	}

	// re-enable the extension if any
	if (extChar) {
		*extChar = '.';
	}

	return( Material );
}


/* EOF: MaterialList.c */
