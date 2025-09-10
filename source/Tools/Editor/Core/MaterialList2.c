/****************************************************************************************/
/*  MATERIALLIST2.C                                                                     */
/*                                                                                      */
/*  Author:  Peter Siamidis                                                             */
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
#include "errorlog.h"
#include "ram.h"
#include "MaterialList2.h"
#include "jeList.h"


////////////////////////////////////////////////////////////////////////////////////////
//	Material struct
////////////////////////////////////////////////////////////////////////////////////////
typedef struct
{
	char		*Name;
	jeBitmap	*Preview;

} Material;


////////////////////////////////////////////////////////////////////////////////////////
//	MaterialDirectory struct
////////////////////////////////////////////////////////////////////////////////////////
typedef struct MaterialDirectory
{
	char				*Name;
	List				*ChildDirectories;
	List				*Materials;
	MaterialDirectory	*ParentDirectory;
	char				*PathName;

} MaterialDirectory;



////////////////////////////////////////////////////////////////////////////////////////
//
//	MaterialList_GetDirectoryPathName()
//
////////////////////////////////////////////////////////////////////////////////////////
static char * MaterialList_GetDirectoryPathName(
	MaterialDirectory	*Directory )	// directory whose complete path name we want
{

	// ensure valid data
	assert( Directory != NULL );
	assert( Directory->PathName != NULL );

	// return complete path name
	return Directory->PathName;

} // MaterialList_GetDirectoryPathName()



////////////////////////////////////////////////////////////////////////////////////////
//
//	MaterialList_GetDirectory()
//
////////////////////////////////////////////////////////////////////////////////////////
MaterialDirectory * MaterialList_GetDirectory(
	char				*DirectoryName,		// name of directory that we want
	MaterialDirectory	*CurDirectory )		// from where to start looking
{

	// locals
	MaterialDirectory	*NextDir;
	MaterialDirectory	*DesiredDir;
	ListIterator		ListIter;

	// ensure valid data
	assert( DirectoryName != NULL );
	assert( CurDirectory != NULL );

	// if we have found if then return its pointer
	if ( strnicmp( CurDirectory->PathName, DirectoryName, strlen( DirectoryName ) ) == 0 )
	{
		return CurDirectory;
	}

	// if we are not a match then check our children
	NextDir = List_GetFirst( CurDirectory->ChildDirectories, &ListIter );
	while ( NextDir != NULL )
	{
		DesiredDir = MaterialList_GetDirectory( DirectoryName, NextDir );
		if ( DesiredDir != NULL )
		{
			return DesiredDir;
		}
		NextDir = List_GetNext( CurDirectory->ChildDirectories, &ListIter );
	}

	// if we got to here then neither us nor any of our children are a match
	return NULL;

} // MaterialList_GetDirectory()



////////////////////////////////////////////////////////////////////////////////////////
//
//	MaterialList_CreateDirectory()
//
////////////////////////////////////////////////////////////////////////////////////////
static MaterialDirectory * MaterialList_CreateDirectory(
	char				*DirectoryName,		// name of this material directory
	MaterialDirectory	*ParentDirectory )	// this material directories parent directory
{

	// locals
	MaterialDirectory	*NewMaterialDirectory;

	// ensure valid data
	assert( DirectoryName != NULL );
	assert( strlen( DirectoryName ) > 0 );

	// create new material directory struct
	NewMaterialDirectory = (MaterialDirectory *)jeRam_AllocateClear( sizeof( *NewMaterialDirectory ) );
	if ( NewMaterialDirectory == NULL )
	{
		jeErrorLog_Add( JE_ERR_MEMORY_RESOURCE, "Could not create material directory struct" );
		goto ERROR_MaterialList_CreateDirectory;
	}

	// save directory name
	NewMaterialDirectory->Name = (char *)jeRam_Allocate( strlen( DirectoryName ) + 1 );
	if ( NewMaterialDirectory->Name == NULL )
	{
		jeErrorLog_Add( JE_ERR_MEMORY_RESOURCE, "Could not create material directory name" );
		goto ERROR_MaterialList_CreateDirectory;
	}
	strcpy( NewMaterialDirectory->Name, DirectoryName );

	// save complete path name
	if ( ParentDirectory != NULL )
	{

		// locals
		char	*ParentPath;

		// get parent path
		ParentPath = MaterialList_GetDirectoryPathName( ParentDirectory );
		assert( ParentPath != NULL );

		// allocate complete path string
		assert( strlen( ParentPath ) > 0 );
		NewMaterialDirectory->PathName = (char *)jeRam_Allocate( strlen( ParentPath ) + strlen( DirectoryName ) + 2 );
		if ( NewMaterialDirectory->PathName == NULL )
		{
			jeErrorLog_Add( JE_ERR_MEMORY_RESOURCE, "Could not create material directory name" );
			goto ERROR_MaterialList_CreateDirectory;
		}

		// save path name
		strcpy( NewMaterialDirectory->PathName, ParentPath );
		strcat( NewMaterialDirectory->PathName, "\\" );
		strcat( NewMaterialDirectory->PathName, DirectoryName );
	}
	else
	{

		// allocate complete path string
		NewMaterialDirectory->PathName = (char *)jeRam_Allocate( strlen( DirectoryName ) + 1 );
		if ( NewMaterialDirectory->PathName == NULL )
		{
			jeErrorLog_Add( JE_ERR_MEMORY_RESOURCE, "Could not create material directory name" );
			goto ERROR_MaterialList_CreateDirectory;
		}

		// save path name
		strcpy( NewMaterialDirectory->PathName, DirectoryName );
	}

	// create child directory list
	NewMaterialDirectory->ChildDirectories = List_Create();
	if ( NewMaterialDirectory->ChildDirectories == NULL )
	{
		jeErrorLog_Add( JE_ERR_INTERNAL_RESOURCE, "Could not create child directory list" );
		goto ERROR_MaterialList_CreateDirectory;
	}

	// create material list
	NewMaterialDirectory->Materials = List_Create();
	if ( NewMaterialDirectory->Materials == NULL )
	{
		jeErrorLog_Add( JE_ERR_INTERNAL_RESOURCE, "Could not create materials list" );
		goto ERROR_MaterialList_CreateDirectory;
	}

	// save parent directory
	NewMaterialDirectory->ParentDirectory = ParentDirectory;

	// all done
	return NewMaterialDirectory;

	// handle errors here
	ERROR_MaterialList_CreateDirectory:

	// destroy complete path name
	if ( NewMaterialDirectory->PathName != NULL )
	{
		jeRam_Free( NewMaterialDirectory->PathName );
	}

	// destroy directory name
	if ( NewMaterialDirectory->Name != NULL )
	{
		jeRam_Free( NewMaterialDirectory->Name );
	}

	// destroy child directory list
	if ( NewMaterialDirectory->ChildDirectories != NULL )
	{
		List_Destroy( &( NewMaterialDirectory->ChildDirectories ), NULL );
	}

	// destroy material list
	if ( NewMaterialDirectory->Materials != NULL )
	{
		List_Destroy( &( NewMaterialDirectory->Materials ), NULL );
	}

	// destroy the material directoty structure
	jeRam_Free( NewMaterialDirectory );

	// return failure
	return NULL;

} // MaterialList_CreateDirectory()



////////////////////////////////////////////////////////////////////////////////////////
//
//	MaterialList_ProcessDirectory()
//
////////////////////////////////////////////////////////////////////////////////////////
static jeBoolean MaterialList_ProcessDirectory(
	MaterialDirectory	*CurDirectory,	// current directory whose materials and child directories will be processed
	jeVFile_Finder		*CurFinder )	// finder info for the current directory
{

	// locals
	jeVFile_Properties	Properties;

	// ensure valid data
	assert( CurDirectory != NULL );
	assert( CurFinder != NULL );

	// start processing files
	while ( jeVFile_FinderGetNextFile( CurFinder ) == JE_TRUE )
	{

		// get properties of current file
		if( jeVFile_FinderGetProperties( CurFinder, &Properties ) == JE_FALSE )
		{
			jeErrorLog_Add( JE_ERR_FILEIO_READ, "Unable to get file properties" );
			return JE_FALSE;
		}

		// if its a directory then process it...
		if ( Properties.AttributeFlags & JE_VFILE_ATTRIB_DIRECTORY )
		{

			// locals
			MaterialDirectory	*NewMaterialDirectory;
			jeVFile				*NewVFile;
			jeVFile_Finder		*NewFinder;
			jeBoolean			Result;
			char				*PathName;

			// create new material directory struct
			NewMaterialDirectory = MaterialList_CreateDirectory( Properties.Name, CurDirectory );
			if ( NewMaterialDirectory == NULL )
			{
				jeErrorLog_Add( JE_ERR_INTERNAL_RESOURCE, "Could not create material directory struct" );
				goto ERROR_MaterialList_ProcessDirectory;
			}

			// get path name
			PathName = MaterialList_GetDirectoryPathName( NewMaterialDirectory );

			// open a file system for the new directory to be processed
			NewVFile = jeVFile_OpenNewSystem(
				NULL,
				JE_VFILE_TYPE_DOS,
				PathName,
				NULL,
				JE_VFILE_OPEN_READONLY | JE_VFILE_OPEN_DIRECTORY );
			if( NewVFile == NULL )
			{
				jeErrorLog_Add( JE_ERR_FILEIO_OPEN, "Unable to open materials directory" );
				goto ERROR_MaterialList_ProcessDirectory;
			}

			// create directory finder for the new directory to be processed
			NewFinder =  jeVFile_CreateFinder( NewVFile, "*.*" );
			if( NewFinder == NULL )
			{
				jeErrorLog_Add( JE_ERR_FILEIO_READ, "Unable to search materials directory" );
				jeVFile_Close( NewVFile );
				goto ERROR_MaterialList_ProcessDirectory;
			}

			// process the new directory
			Result = MaterialList_ProcessDirectory( NewMaterialDirectory, NewFinder );

			// destroy the newly created finder and file system
			jeVFile_DestroyFinder( NewFinder );
			jeVFile_Close( NewVFile );

			// fail if the directory was not succesfully processed
			if ( Result == JE_FALSE )
			{
				goto ERROR_MaterialList_ProcessDirectory;
			}

			// add this new directory to the list of current parents child directories
			List_Append( CurDirectory->ChildDirectories, NewMaterialDirectory );
		}
		// ..otherwise just process it as a file
		else
		{

			// locals
			Material	*NewMaterial;

			// create new material
			NewMaterial = (Material *)jeRam_AllocateClear( sizeof( *NewMaterial ) );
			if ( NewMaterial == NULL )
			{
				jeErrorLog_Add( JE_ERR_MEMORY_RESOURCE, "Could not create material struct" );
				goto ERROR_MaterialList_ProcessDirectory;
			}

			// setup material name
			NewMaterial->Name = (char *)jeRam_Allocate( strlen( Properties.Name ) + 1 );
			if ( NewMaterial->Name == NULL )
			{
				jeErrorLog_Add( JE_ERR_MEMORY_RESOURCE, "Could not create material name" );
				goto ERROR_MaterialList_ProcessDirectory;
			}
			strcpy( NewMaterial->Name, Properties.Name );

			// add material to this material directories list
			List_Append( CurDirectory->Materials, NewMaterial );
		}
	}

	// all done
	return JE_TRUE;

	// handle errors here
	//undone, doesnt cleanup
	ERROR_MaterialList_ProcessDirectory:

	// return failure
	return JE_FALSE;

} // MaterialList_ProcessDirectory()



////////////////////////////////////////////////////////////////////////////////////////
//
//	MaterialList_Create()
//
////////////////////////////////////////////////////////////////////////////////////////
MaterialDirectory * MaterialList2_Create(
	char	*MaterialsPath )	// path where materials are found
{

	// locals
	jeVFile				*MaterialsRootDir;
	jeVFile_Finder		*Finder;
	MaterialDirectory	*RootMaterialDirectory;

	// ensure valid data
	assert( MaterialsPath != NULL );

	// open root materials directory
	MaterialsRootDir = jeVFile_OpenNewSystem(
		NULL,
		JE_VFILE_TYPE_DOS,
		MaterialsPath,
		NULL,
		JE_VFILE_OPEN_READONLY | JE_VFILE_OPEN_DIRECTORY );
	if( MaterialsRootDir == NULL )
	{
		jeErrorLog_Add( JE_ERR_FILEIO_OPEN, "Unable to open materials directory" );
		return NULL;
	}

	// create our directory finder
	Finder =  jeVFile_CreateFinder( MaterialsRootDir, "*.*" );
	if( Finder == NULL )
	{
		jeErrorLog_Add( JE_ERR_FILEIO_READ, "Unable to search materials directory" );
		return NULL;
	}

	// create root material directory
	RootMaterialDirectory = MaterialList_CreateDirectory( MaterialsPath, NULL );
	if ( RootMaterialDirectory == NULL )
	{
		jeErrorLog_Add( JE_ERR_INTERNAL_RESOURCE, "Could not create root material directory struct" );
		return NULL;
	}

	// process for all directories
	MaterialList_ProcessDirectory( RootMaterialDirectory, Finder );

	// close the root materials directory
	jeVFile_DestroyFinder( Finder );
	jeVFile_Close( MaterialsRootDir );

	// all done
	return RootMaterialDirectory;

} // MaterialList_Create()
