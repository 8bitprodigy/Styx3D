/*!
	@file jeResource.h
	
	@author
	@brief The Resource Manager definition

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

	@par Documentation
	@ref resourceMgr "The Resource Manager page"
*/

/*! @page resourceMgr The Resource Manager

	This object is designed to help resource to be loaded from a PAK/VFS file or
	a folder in direct access.

	@par Resource
	A resource can be a folder or a file. In the file, we have subcategorized actors,
	bitmaps, shaders, sounds and musics.

    @par Resource Manager
	A #jeResourceMgr maintains a list of all resources that was requested by the user
	application. The resource is strongly linked to the application.

	@par The Default Directory Identifiers are:
	@anchor defaultDirs 
	<ul>
	<li><b>Sounds</b> to store all sounds files</li>
	<li><b>GlobalMaterials</b> to store all bitmap files</li>
	<li><b>Actors</b> to store all actors files</li>
	<li><b>Shaders</b> to store all shaders files</li>
	</ul>
	The identifiers have been choose identical to directories because it simplifies the link
	between the identifier and the directory. These identifers by default are linked with their
	respective directory. When PAK file will be introduce, developpers will have the ability to
	release their content in a single file.
	@par Setting up the GlobalMaterials from a PAK file
	@code
    //create resource manager 
    jeResourceMgr *ResourceMgr = jeResource_MgrCreate(); 
    
    //open virtual file systems 
    VFile* FS = jeVFile_OpenNewSystem(NULL,JE_VFILE_TYPE_DOS,"Bitmaps.JetPak", 
                                      NULL,JE_VFILE_OPEN_READONLY); 

    VFile* VFS = jeVFile_OpenNewSystem(FS,JE_VFILE_TYPE_VIRTUAL,NULL,NULL, 
                                       JE_VFILE_OPEN_READONLY|JE_VFILE_OPEN_DIRECTORY); 

    //add file system to resource manager 
    jeResource_AddVFile( ResourceMgr, "GlobalMaterials", VFS); 
	@endcode
	@par Setting up the GlobalMaterials from a directory
	@code
    //create resource manager 
    jeResourceMgr *ResourceMgr = jeResource_MgrCreate(); 

    //Add the GlobaMaterials directory to resource
    jeResource_OpenDirectory(ResourceMgr, "./GlobalMaterials", GlobalMaterials);
	@endcode

	@par The 4 default Directories are:
	<ul>
	<li><b>Sounds</b> to store all sounds files</li>
	<li><b>GlobalMaterials</b> to store all bitmap files</li>
	<li><b>Actors</b> to store all actors files</li>
	<li><b>Shaders</b> to store all shaders files</li>
	</ul>
*/

#ifndef	JE_RESOURCE_H
#define JE_RESOURCE_H

#include "BaseType.h"

#ifdef __cplusplus
extern "C" {
#endif


/*! @name Resource kinds
	@brief Possible value for Type parameter of jeResource_GetResource() */
/*@{*/
/*! @def JE_RESOURCE_ANY
	@brief The resource is anything
*/
#define JE_RESOURCE_ANY		0x0000
/*! @def JE_RESOURCE_VFS
	@brief The resource is a #jeVFile pointer
*/
#define JE_RESOURCE_VFS		0x0001
/*! @def JE_RESOURCE_BITMAP
	@brief The resource is a #jeBitmap pointer
*/
#define JE_RESOURCE_BITMAP	0x0002
/*! @def JE_RESOURCE_SHADER
	@brief The resource is a #jeShader pointer
*/
#define JE_RESOURCE_SHADER	0x0004
/*! @def JE_RESOURCE_SOUND
	@brief The resource is a #jeSound pointer
*/
#define JE_RESOURCE_SOUND	0x0005
/*! @def JE_RESOURCE_ACTOR
	@brief The resource is a #jeActor pointer
*/
#define JE_RESOURCE_ACTOR	0x0006
/*! @def JE_RESOURCE_MATERIAL
	@brief The resource is a #jeMaterialSpec Material pointer build from JMAT file
*/
#define JE_RESOURCE_MATERIAL	0x0010
/*! @def JE_RESOURCE_TEXTURE
	@brief The resource is a #jeTexture Texture pointer build from image file by driver
*/
#define JE_RESOURCE_TEXTURE		0x0011
/*@}*/

/*! @typedef jeResourceMgr
	@brief The jeResourceMgr struct
*/
typedef struct jeResourceMgr	jeResourceMgr;

typedef struct jeEngine	jeEngine;

////////////////////////////////////////////////////////////////////////////////////////
//	Resource manager functions
////////////////////////////////////////////////////////////////////////////////////////

/*! @fn jeResourceMgr* jeResource_MgrCreate(jeEngine* pEngine)
	@brief Create a resource manager.
	@param pEngine The current jeEngine instance
	@return The jeResourceMgr instance if succeed

	The @p pEngine parameter is need to built texture resource.
*/
JETAPI jeResourceMgr* JETCC jeResource_MgrCreate(jeEngine* pEngine);

/*!	@fn int jeResource_MgrIncRefcount(jeResourceMgr* ResourceMgr)
	@brief Increment a resource managers reference/usage count.
	@param ResourceMgr Manager whose ref count will be incremented
	@return The new reference counter value
*/
JETAPI int32 JETCC jeResource_MgrIncRefcount(jeResourceMgr* ResourceMgr );

/*! @fn void JETCC jeResource_MgrDestroy(jeResourceMgr** DeadResourceMgr)
	@brief Destroy a resource manager.
	@param DeadResourceMgr Manager to zap

	The Resource Manager is zapped oly if its reference counter is zero.<br>
	The function first decrement the ref count.
*/
JETAPI void JETCC jeResource_MgrDestroy(jeResourceMgr** DeadResourceMgr);
   
/*! @fn jeBoolean jeResourceMgr_SetEngine(jeEngine* pEngine)
	@brief Assign the current jeEngine instance to the jeResourceMgr
	@param pEngine The current jeEngine instance
	@return JE_TRUE when succeed
*/
JETAPI jeBoolean JETCC jeResourceMgr_SetEngine(jeResourceMgr *ResourceMgr, jeEngine* pEngine);

JETAPI jeEngine* JETCC jeResourceMgr_GetEngine(const jeResourceMgr *ResourceMgr);

JETAPI jeResourceMgr* JETCC jeResourceMgr_GetSingleton();

////////////////////////////////////////////////////////////////////////////////////////
//	Generic resource functions
////////////////////////////////////////////////////////////////////////////////////////

//	Add a new resource.
//
////////////////////////////////////////////////////////////////////////////////////////
JETAPI jeBoolean JETCC jeResource_Add(
	jeResourceMgr	*ResourceMgr,	// resource manager to add it to
	char			*Name,			// name
    uint32          Type,           // type
	void			*Data );		// data

/*! @brief Get an existing resource.
	@param ResourceMgr The resource manager to delete it from
	@param Name The resource name
    @deprecated Replace by jeResource_GetResource
*/
JETAPI void * JETCC jeResource_Get(
	jeResourceMgr	*ResourceMgr,	// resource manager to get it from
	char			*Name );		// resource name

/*! @brief Delete an existing resource.
	
	Decrement the @p Name identified resource reference counter and remove it when reach zero.

	@param ResourceMgr The resource manager to delete it from
	@param Name The resource name
	@return 0 if the resource is fully deleted else return the reference counter value

    @deprecated Replace by jeResource_ReleaseResource
*/
////////////////////////////////////////////////////////////////////////////////////////
JETAPI int JETCC jeResource_Delete(jeResourceMgr *ResourceMgr, char *Name );




////////////////////////////////////////////////////////////////////////////////////////
//	VFile specific resource functions
////////////////////////////////////////////////////////////////////////////////////////

//	Add a new VFile resource.
//
////////////////////////////////////////////////////////////////////////////////////////
JETAPI jeBoolean JETCC jeResource_AddVFile(
	jeResourceMgr	*ResourceMgr,	// resource list to add it to
	char			*Name,			// name
	jeVFile			*Data );		// data

//	Get an existing VFile resource.
//
////////////////////////////////////////////////////////////////////////////////////////
JETAPI jeVFile * JETCC jeResource_GetVFile(
	jeResourceMgr	*ResourceMgr,	// resource list to get it from
	char			*Name );		// name

//	Delete an existing VFile resource.
//
////////////////////////////////////////////////////////////////////////////////////////
JETAPI int JETCC jeResource_DeleteVFile(
	jeResourceMgr	*ResourceMgr,	// resource list to delete it from
	char			*Name );		// name

/*! @fn jeBoolean jeResource_OpenDirectory(jeResourceMgr* pResourceMgr, char* DirName, char* ResName)
	@brief Open a vfile(directory) for Resource Manager (WITH AutoRemove on ResMgrDestroy)
	@param pResourceMgr The Resource Manager instance
	@param DirName Path of directory
	@param ResName Alias of this resource
	@return JE_TRUE when succeed
	@author Icestorm
*/
JETAPI jeBoolean JETCC jeResource_OpenDirectory(jeResourceMgr* pResourceMgr, char* DirName, char* ResName);

/*! @fn jeResourceMgr* jeResource_MgrCreateDefault(jeEngine* pEngine)
	@brief Create a Resource Manager using the default paths.
	@param pEngine The current jeEngine
	@return The jeResourceMgr instance if succeed
	@author Incarnadine modified by Icestorm [Added autoremove], modified by Krouer for pEngine parameter
	@see jeResource_OpenDirectory

    It makes usage of jeResource_OpenDirectory to create its @ref defaultDirs "4 defaults directories".
*/
JETAPI jeResourceMgr* JETCC jeResource_MgrCreateDefault(jeEngine* pEngine);

/*! @fn void* jeResource_GetResource(jeResourceMgr *ResourceMgr, int32 Type, char *Name);
	@brief Create a resource from memory or disk
	
	Read resource from disk and create them from the founded file

	@note The name can contains in first the directory of the resource.
	@see @ref defaultDirs "Default directories"
    @param[in] ResourceMgr The resource list to get it from
	@param[in] Type The awaited resource pointer (material, shader, actor, bitmap, ...) type
	@param[in] Name The resource name
	@return The resource pointer to be casted following the flag of @a Type or NULL if failed
*/
JETAPI void* JETCC jeResource_GetResource(jeResourceMgr *ResourceMgr, int32 Type, char *Name);

/*! @fn void jeResource_ExportResource(jeResourceMgr *ResourceMgr, int32 Type, char *Name);
	@brief Save a resource to its format on the disk

	@remark Use when older level are load in the engine.

    @param[in] ResourceMgr The resource list to get it from
	@param[in] Type The awaited resource pointer (shader, actor, bitmap, ...) type
	@param[in] Name The resource name

	@note Only bitmaps export is currently implemented and bmp generated file aren't BMP format files.
*/
JETAPI void JETCC jeResource_ExportResource(jeResourceMgr *ResourceMgr, int32 Type, char *Name, void* Data);

/*! @fn void* jeResource_Release(jeResourceMgr *ResourceMgr, int32 Type, char *Name);
	@brief Release the identified resource
	
	Decrement the resource usage and if reach zero, remove the resource from the managed pool.
	Try to call the Destroy of the resource depending of the Type param.<br>Superseed the jeResource_Delete
	function.

	@note The name can contains in first the directory of the resource.
	@see @ref defaultDirs "Default directories"
    @param[in] ResourceMgr The resource list to get it from
	@param[in] Type The resource (material, shader, actor, bitmap, ...) type
	@param[in] Name The resource name identifier
	@return JE_TRUE when succeed
*/
JETAPI jeBoolean JETCC jeResource_ReleaseResource(jeResourceMgr *ResourceMgr, int32 Type, char *Name);

#ifdef __cplusplus
}
#endif

#endif
