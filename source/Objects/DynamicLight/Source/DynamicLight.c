/****************************************************************************************/
/*  DYNAMICLIGHT.C                                                                      */
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
#ifdef WIN32
#pragma warning ( disable : 4115 )
#include <windows.h>
#pragma warning ( default : 4115 )
#endif

#ifdef BUILD_BE
#include <image.h>
#include <Resources.h>
#endif

#include <assert.h>
#include <float.h>
#include "VFile.h"
#include "jeProperty.h"
#include "Ram.h"
#include "jeResource.h"
#include "jeWorld.h"
#include "DynamicLight.h"
#include "Resource.h"
#include "Errorlog.h"


#define DYNAMICLIGHT_VERSION 1

//Royce
#define OBJ_PERSIST_SIZE 5000
//---

////////////////////////////////////////////////////////////////////////////////////////
//	Property list stuff
////////////////////////////////////////////////////////////////////////////////////////
enum
{

	//
	DYNAMICLIGHT_RADIUS_ID = PROPERTY_LOCAL_DATATYPE_START,
	DYNAMICLIGHT_BRIGHTNESS_ID,

	// color
	DYNAMICLIGHT_COLORGROUP_ID,
	DYNAMICLIGHT_COLOR_ID,
	DYNAMICLIGHT_COLORRED_ID,
	DYNAMICLIGHT_COLORGREEN_ID,
	DYNAMICLIGHT_COLORBLUE_ID,
	DYNAMICLIGHT_COLORGROUPEND_ID,

	//
	DYNAMICLIGHT_CASTSHADOW_ID,
	DYNAMICLIGHT_LAST_ID

};
enum
{

	//
	DYNAMICLIGHT_RADIUS_INDEX = 0,
	DYNAMICLIGHT_BRIGHTNESS_INDEX,

	// color
	DYNAMICLIGHT_COLORGROUP_INDEX,
	DYNAMICLIGHT_COLOR_INDEX,
	DYNAMICLIGHT_COLORRED_INDEX,
	DYNAMICLIGHT_COLORGREEN_INDEX,
	DYNAMICLIGHT_COLORBLUE_INDEX,
	DYNAMICLIGHT_COLORGROUPEND_INDEX,

	//
	DYNAMICLIGHT_CASTSHADOW_INDEX,
	DYNAMICLIGHT_LAST_INDEX

};


////////////////////////////////////////////////////////////////////////////////////////
//	Globals
////////////////////////////////////////////////////////////////////////////////////////
#ifdef WIN32
static HINSTANCE		hClassInstance = NULL;
#endif

#ifdef BUILD_BE
static image_id			hClassInstance = NULL;
#endif

static jeProperty		DynamicLightProperties[DYNAMICLIGHT_LAST_INDEX];
static jeProperty_List	DynamicLightPropertyList = { DYNAMICLIGHT_LAST_INDEX, &( DynamicLightProperties[0] ) };


////////////////////////////////////////////////////////////////////////////////////////
//	Defaults
////////////////////////////////////////////////////////////////////////////////////////
#define DYNAMICLIGHT_DEFAULT_RADIUS			50.0f
#define DYNAMICLIGHT_DEFAULT_BRIGHTNESS		20.0f
#define DYNAMICLIGHT_DEFAULT_COLORRED		128.0f
#define DYNAMICLIGHT_DEFAULT_COLORGREEN		128.0f
#define DYNAMICLIGHT_DEFAULT_COLORBLUE		128.0f
#define DYNAMICLIGHT_DEFAULT_CASTSHADOW		JE_FALSE


////////////////////////////////////////////////////////////////////////////////////////
//	Object data
////////////////////////////////////////////////////////////////////////////////////////
typedef struct DynamicLight
{
	jeWorld			*World;
	jeResourceMgr	*ResourceMgr;
	jeEngine		*Engine;
	int				RefCount;
	jeXForm3d		Xf;
	jeLight			*Light;
	jeVec3d			Color;
	float			Radius;
	float			Brightness;
	jeBoolean		CastShadow;
	jeBoolean		LoadedFromDisk;

} DynamicLight;



////////////////////////////////////////////////////////////////////////////////////////
//
//	DynamicLight_Destroy()
//
////////////////////////////////////////////////////////////////////////////////////////
static jeBoolean DynamicLight_Destroy(
	DynamicLight	*Object )	// object from which light will be created
{

	// ensure valid data
	assert( Object != NULL );

	// remove light from world
	assert( Object->Light != NULL );
	assert( Object->World != NULL );
	if ( jeWorld_RemoveDLight( Object->World, Object->Light ) == JE_FALSE )
	{
		jeErrorLog_Add( JE_ERR_INTERNAL_RESOURCE, NULL );
		return JE_FALSE;
	}

	// destroy light
	jeLight_Destroy( &Object->Light );

	// all done
	return JE_TRUE;

} // DynamicLight_Destroy()



////////////////////////////////////////////////////////////////////////////////////////
//
//	DynamicLight_Create()
//
////////////////////////////////////////////////////////////////////////////////////////
static jeBoolean DynamicLight_Create(
	DynamicLight	*Object )	// object from which light will be created
{

	// locals
	jeBoolean	Result;

	// ensure valid data
	assert( Object != NULL );

	// create light
	Object->Light = jeLight_Create();
	if ( Object->Light == NULL )
	{
		jeErrorLog_Add( JE_ERR_INTERNAL_RESOURCE, NULL );
		return JE_FALSE;
	}

	// set light attributes
	Result = jeLight_SetAttributes(	Object->Light,
									&( Object->Xf.Translation ),
									&( Object->Color ),
									Object->Radius, 
									Object->Brightness, 
									JE_LIGHT_FLAG_FAST_LIGHTING_MODEL );	//undone, dont know flags for cast shadow
#pragma message ("shadow flags")	

	if ( Result == JE_FALSE )
	{
		jeErrorLog_Add( JE_ERR_INTERNAL_RESOURCE, NULL );
		jeLight_Destroy( &( Object->Light ) );
		return JE_FALSE;
	}

	// add it to the world
	Result = jeWorld_AddDLight( Object->World, Object->Light );
	if ( Result == JE_FALSE )
	{
		jeErrorLog_Add( JE_ERR_INTERNAL_RESOURCE, NULL );
		jeLight_Destroy( &( Object->Light ) );
		return JE_FALSE;
	}

	// all done
	return JE_TRUE;

} // DynamicLight_Create()




#ifdef WIN32

////////////////////////////////////////////////////////////////////////////////////////
//
//	Util_LoadLibraryString()
//
////////////////////////////////////////////////////////////////////////////////////////
static char * Util_LoadLibraryString(
	HINSTANCE		hInstance,
	unsigned int	ID )
{

	// locals
	#define		MAX_STRING_SIZE	255
	static char	StringBuf[MAX_STRING_SIZE];
	char		*NewString;
	int			Size;

	// ensure valid data
	assert( hInstance != NULL );
	assert( ID >= 0 );

	// get resource string
	Size = LoadString( hInstance, ID, StringBuf, MAX_STRING_SIZE );
	if ( Size <= 0 )
	{
		jeErrorLog_Add( JE_ERR_INTERNAL_RESOURCE, NULL );
		return NULL;
	}

	// copy resource string
	NewString = jeRam_Allocate( Size + 1 );
	if ( NewString == NULL )
	{
		jeErrorLog_Add( JE_ERR_MEMORY_RESOURCE, NULL );
		return NULL;
	}
	strcpy( NewString, StringBuf );

	// all done
	return NewString;

} // Util_LoadLibraryString()

#endif

#ifdef BUILD_BE

#include <stdio.h>
static char *Util_LoadLibraryString(image_id libhinst, int32 resid)
{
	BResources resourcefile;
	int result;
	char *rcbuffer;
 	image_info info;
	size_t outSize;
	
	// locals
	#define		MAX_STRING_SIZE	255
	static char	stringbuffer[MAX_STRING_SIZE];


	assert(libhinst > 0);
	assert(resid);

///	hResources = (image_id)hStringResources ;
	
	if(get_image_info(libhinst,&info) != B_OK)
		return NULL;
		
	BFile* resFile = new BFile(info.name , B_READ_ONLY);
	
	resourcefile.SetTo(resFile,false);

	char* loadedString = (char *)resourcefile.FindResource((int)'DATA', 		/*** DEPRECATED ***/
								  resid, 
								  &outSize);
	
	//
	//	Note that if we did't allocate space and copy the string, then we
	//	would be limited to having one string loaded at a time. Or we would
	//	setup some kind of revolving buffer.  Either of these options is
	//	risky and could eventually cause a problem elsewhere... 	 LF
	//
 
	// Allocate memory for the string
	rcbuffer = (char*)jeRam_Allocate(strlen(loadedString) + 1);
	strcpy(rcbuffer, loadedString);
 
#ifndef NDEBUG
	memset(stringbuffer, 0xFF, MAX_STRING_SIZE + 1);
#endif
 
	printf("Read %s\n" , rcbuffer);
	// return the allocated string
	return (rcbuffer);
}//Util_LoadLibraryString

#endif


////////////////////////////////////////////////////////////////////////////////////////
//
//	Init_Class()
//
////////////////////////////////////////////////////////////////////////////////////////
void Init_Class(
#ifdef WIN32
	HINSTANCE	hInstance )	// dll instance handle
#endif
#ifdef BUILD_BE
	image_id	hInstance )	// dll instance handle
#endif
{
#ifdef WIN32
	// ensure valid data
	assert( hInstance != NULL );
#endif
#ifdef BUILD_BE
	// ensure valid data
	assert( hInstance > 0 );
#endif
	// save hinstance
	hClassInstance = hInstance;

	// setup radius property
	jeProperty_FillFloat(	&( DynamicLightProperties[DYNAMICLIGHT_RADIUS_INDEX] ),
							Util_LoadLibraryString( hClassInstance, IDS_RADIUS ),
							DYNAMICLIGHT_DEFAULT_RADIUS,
							DYNAMICLIGHT_RADIUS_ID,
							0.1f, FLT_MAX, 5.0f );

	// setup brightness property
	jeProperty_FillFloat(	&( DynamicLightProperties[DYNAMICLIGHT_BRIGHTNESS_INDEX] ),
							Util_LoadLibraryString( hClassInstance, IDS_BRIGHTNESS ),
							DYNAMICLIGHT_DEFAULT_BRIGHTNESS,
							DYNAMICLIGHT_BRIGHTNESS_ID,
							0.1f, FLT_MAX, 5.0f );


	////////////////////////////////////////////////////////////////////////////////////////
	//	Color properties
	////////////////////////////////////////////////////////////////////////////////////////

	// start color group
	jeProperty_FillGroup(	&( DynamicLightProperties[DYNAMICLIGHT_COLORGROUP_INDEX] ),
							Util_LoadLibraryString( hClassInstance, IDS_COLORGROUP ),
							DYNAMICLIGHT_COLORGROUP_INDEX );

	// setup color property
	{
		jeVec3d	Color = { DYNAMICLIGHT_DEFAULT_COLORRED, DYNAMICLIGHT_DEFAULT_COLORGREEN, DYNAMICLIGHT_DEFAULT_COLORBLUE };
		jeProperty_FillColorPicker(	&( DynamicLightProperties[DYNAMICLIGHT_COLOR_INDEX] ),
									Util_LoadLibraryString( hClassInstance, IDS_COLOR ),
									&Color,
									DYNAMICLIGHT_COLOR_ID );
	}

	// setup color red property
	jeProperty_FillFloat(	&( DynamicLightProperties[DYNAMICLIGHT_COLORRED_INDEX] ),
							Util_LoadLibraryString( hClassInstance, IDS_COLORRED ),
							DYNAMICLIGHT_DEFAULT_COLORRED,
							DYNAMICLIGHT_COLORRED_ID,
							0.0f, 255.0f, 1.0f );

	// setup ambient light green property
	jeProperty_FillFloat(	&( DynamicLightProperties[DYNAMICLIGHT_COLORGREEN_INDEX] ),
							Util_LoadLibraryString( hClassInstance, IDS_COLORGREEN ),
							DYNAMICLIGHT_DEFAULT_COLORGREEN,
							DYNAMICLIGHT_COLORGREEN_ID,
							0.0f, 255.0f, 1.0f );

	// setup ambient light blue property
	jeProperty_FillFloat(	&( DynamicLightProperties[DYNAMICLIGHT_COLORBLUE_INDEX] ),
							Util_LoadLibraryString( hClassInstance, IDS_COLORBLUE ),
							DYNAMICLIGHT_DEFAULT_COLORBLUE,
							DYNAMICLIGHT_COLORBLUE_ID,
							0.0f, 255.0f, 1.0f );

	// end color group
	jeProperty_FillGroupEnd( &( DynamicLightProperties[DYNAMICLIGHT_COLORGROUPEND_INDEX] ), DYNAMICLIGHT_COLORGROUPEND_ID );


	////////////////////////////////////////////////////////////////////////////////////////
	//	Misc properties
	////////////////////////////////////////////////////////////////////////////////////////

	// setup cast shadow flag
	jeProperty_FillCheck(	&( DynamicLightProperties[DYNAMICLIGHT_CASTSHADOW_INDEX] ),
							Util_LoadLibraryString( hClassInstance, IDS_CASTSHADOW ),
							DYNAMICLIGHT_DEFAULT_CASTSHADOW,
							DYNAMICLIGHT_CASTSHADOW_ID );

	// final init
	DynamicLightPropertyList.jePropertyN = DYNAMICLIGHT_LAST_INDEX;

} // Init_Class()

////////////////////////////////////////////////////////////////////////////////////////
//
//	DeInit_Class()
//
////////////////////////////////////////////////////////////////////////////////////////
void DeInit_Class(
	void )	// no parameters
{

	// zap instance pointer
	hClassInstance = NULL;

} // DeInit_Class()



////////////////////////////////////////////////////////////////////////////////////////
//
//	CreateInstance()
//
////////////////////////////////////////////////////////////////////////////////////////
void * JETCC CreateInstance(
	void )	// no parameters
{

	// locals
	DynamicLight	*Object;

	// allocate struct
	Object = (DynamicLight *)jeRam_AllocateClear( sizeof( *Object ) );
	if ( Object == NULL )
	{
		jeErrorLog_Add( JE_ERR_MEMORY_RESOURCE, NULL );
		return NULL;
	}

	// get default settings
	Object->Radius = DYNAMICLIGHT_DEFAULT_RADIUS;
	Object->Brightness = DYNAMICLIGHT_DEFAULT_BRIGHTNESS;
	Object->Color.X = DYNAMICLIGHT_DEFAULT_COLORRED;
	Object->Color.Y = DYNAMICLIGHT_DEFAULT_COLORGREEN;
	Object->Color.Z = DYNAMICLIGHT_DEFAULT_COLORBLUE;
	Object->CastShadow = DYNAMICLIGHT_DEFAULT_CASTSHADOW;

	// init remaining fields
	jeXForm3d_SetIdentity( &Object->Xf );
	Object->RefCount = 1;

	// all done
	return Object;

} // CreateInstance()



////////////////////////////////////////////////////////////////////////////////////////
//
//	CreateRef()
//
////////////////////////////////////////////////////////////////////////////////////////
void JETCC CreateRef(
	void	*Instance )	// instance data
{

	// locals
	DynamicLight	*Object;
	
	// get object
	Object = (DynamicLight *)Instance;
	assert( Object != NULL );

	// adjust object ref count
	Object->RefCount++;

} // CreateRef()



////////////////////////////////////////////////////////////////////////////////////////
//
//	Destroy()
//
////////////////////////////////////////////////////////////////////////////////////////
jeBoolean JETCC Destroy(
	void	**Instance )	// pointer to instance data
{

	// locals
	DynamicLight	*Object;

	// ensure valid data
	assert( Instance != NULL );

	// get object
	Object = (DynamicLight *)*Instance;
	assert( Object != NULL );
	assert( Object->RefCount > 0 );

	// do nothing if ref count is not at zero
	Object->RefCount--;
	if ( Object->RefCount > 0 )
	{
		return JE_FALSE;
	}

	// make sure everything has been properly destroyed
	assert( Object->World == NULL );
	assert( Object->Engine == NULL );
	assert( Object->ResourceMgr == NULL );
	assert( Object->Light == NULL );

	// free struct
	jeRam_Free( Object );

	// zap pointer
	*Instance = NULL;

	// all done
	return JE_TRUE;

} // Destroy()



////////////////////////////////////////////////////////////////////////////////////////
//
//	Render()
//
////////////////////////////////////////////////////////////////////////////////////////
jeBoolean JETCC Render(
	const void				*Instance,	// object instance data
	const jeWorld			*World,		// world
	const jeEngine			*Engine,	// engine
	const jeCamera			*Camera,				// camera
	const jeFrustum			*CameraSpaceFrustum, 	// frustum
	jeObject_RenderFlags	RenderFlags)
{

	// all done
	return JE_TRUE;

	// eliminate warnings
	Instance;
	World;
	Engine;
	Camera;
	CameraSpaceFrustum;
	RenderFlags;
} // Render()



////////////////////////////////////////////////////////////////////////////////////////
//
//	AttachWorld()
//
////////////////////////////////////////////////////////////////////////////////////////
jeBoolean JETCC AttachWorld(
	void	*Instance,	// object instance data
	jeWorld	*World )	// world
{

	// locals
	DynamicLight	*Object;

	
	// ensure valid data
	assert( Instance != NULL );
	assert( World != NULL );

	// get object
	Object = (DynamicLight *)Instance;

	// save world pointer
	Object->World = World;

	// save an instance of the resource manager
	Object->ResourceMgr = jeWorld_GetResourceMgr( World );
	assert( Object->ResourceMgr != NULL );

	// create light
	if ( DynamicLight_Create( Object ) == JE_FALSE )
	{
		jeErrorLog_Add( JE_ERR_INTERNAL_RESOURCE, NULL );
		jeResource_MgrDestroy( &( Object->ResourceMgr ) );
		Object->World = NULL;
		return JE_FALSE;
	}

	// all done
	return JE_TRUE;

} // AttachWorld()



////////////////////////////////////////////////////////////////////////////////////////
//
//	DettachWorld()
//
////////////////////////////////////////////////////////////////////////////////////////
jeBoolean JETCC DettachWorld(
	void	*Instance,	// object instance data
	jeWorld	*World )	// world
{

	// locals
	DynamicLight	*Object;

	// ensure valid data
	assert( Instance != NULL );
	assert( World != NULL );

	// get object
	Object = (DynamicLight *)Instance;
	assert( Object->World == World );

	// destroy light
	DynamicLight_Destroy( Object );

	// destroy our instance of the resource manager
	jeResource_MgrDestroy( &( Object->ResourceMgr ) );

	// zap world pointer
	Object->World = NULL;

	// all done
	return JE_TRUE;

	// eliminate warnings
	World;

} // DettachWorld()



////////////////////////////////////////////////////////////////////////////////////////
//
//	AttachEngine()
//
////////////////////////////////////////////////////////////////////////////////////////
jeBoolean JETCC AttachEngine(
	void		*Instance,	// object instance data
	jeEngine	*Engine )	// engine
{

	// locals
	DynamicLight	*Object;

	// ensure valid data
	assert( Instance != NULL );
	assert( Engine != NULL );

	// get object data
	Object = (DynamicLight *)Instance;

	// save engine pointer
	Object->Engine = Engine;

	// all done
	return JE_TRUE;

} // AttachEngine()



////////////////////////////////////////////////////////////////////////////////////////
//
//	DettachEngine()
//
////////////////////////////////////////////////////////////////////////////////////////
jeBoolean JETCC DettachEngine(
	void		*Instance,	// object instance data
	jeEngine	*Engine )	// engine
{

	// locals
	DynamicLight	*Object;

	// ensure valid data
	assert( Instance != NULL );
	assert( Engine != NULL );

	// get object data
	Object = (DynamicLight *)Instance;
	assert( Object->Engine == Engine );

	// zap engine pointer
	Object->Engine = NULL;

	// all done
	return JE_TRUE;

	// eliminate warnings
	Engine;

} // DettachEngine()



////////////////////////////////////////////////////////////////////////////////////////
//
//	AttachSoundSystem()
//
////////////////////////////////////////////////////////////////////////////////////////
jeBoolean JETCC AttachSoundSystem(
	void			*Instance,		// object instance data
	jeSound_System	*SoundSystem )	// sound system
{

	// ensure valid data
	assert( Instance != NULL );
	assert( SoundSystem != NULL );

	// all done
	return JE_TRUE;

	// elminate warnings
	Instance;
	SoundSystem;

} // AttachSoundSystem()



////////////////////////////////////////////////////////////////////////////////////////
//
//	DettachSoundSystem()
//
////////////////////////////////////////////////////////////////////////////////////////
jeBoolean JETCC DettachSoundSystem(
	void			*Instance,		// object instance data
	jeSound_System	*SoundSystem )	// sound system
{

	// ensure valid data
	assert( Instance != NULL );
	assert( SoundSystem != NULL );

	// all done
	return JE_TRUE;

	// elminate warnings
	Instance;
	SoundSystem;

} // DettachSoundSystem()



////////////////////////////////////////////////////////////////////////////////////////
//
//	Collision()
//
///////////////////////////////////////////////////////////////////////////////////////
jeBoolean JETCC Collision(
	const jeObject	*Object,
	const jeExtBox	*Box,
	const jeVec3d	*Front,
	const jeVec3d	*Back,
	jeVec3d			*Impact,
	jePlane			*Plane )
{

	// ensure valid data
	assert( Object != NULL );
	//assert( Box != NULL );  Removed by Incarnadine.  Box CAN be NULL.
	assert( Front != NULL );
	assert( Back != NULL );
	//assert( Impact != NULL ); Removed by Icestorm. Impact&Plane CAN be NULL.
	//assert( Plane != NULL );

	// all done
	return JE_FALSE;

	// eliminate warnings
	Object;
	Box;
	Front;
	Back;
	Impact;
	Plane;

} // Collision()



////////////////////////////////////////////////////////////////////////////////////////
//
//	GetExtBox()
//
///////////////////////////////////////////////////////////////////////////////////////
jeBoolean JETCC GetExtBox(
	const void	*Instance,	// object instance data
	jeExtBox	*BBox )		// where to store extent box
{

	// locals
	DynamicLight	*Object;
	jeVec3d			Pos;

	// ensure valid data
	assert( Instance != NULL );
	assert( BBox != NULL );

	// get object data
	Object = (DynamicLight *)Instance;

	// save extent box
	Pos = Object->Xf.Translation;
	jeExtBox_Set (  BBox, 
					Pos.X - 5.0f, Pos.Y - 5.0f, Pos.Z - 5.0f,
					Pos.X + 5.0f, Pos.Y + 5.0f, Pos.Z + 5.0f );

	// all done
	return JE_TRUE;

} // GetExtBox()



////////////////////////////////////////////////////////////////////////////////////////
//
//	CreateFromFile()
//
///////////////////////////////////////////////////////////////////////////////////////
void * JETCC CreateFromFile(
	jeVFile		*File,		// vfile to use
	jePtrMgr *PtrMgr )	// pointer manager
{

	// locals
	DynamicLight	*Object;
	jeBoolean		Result = JE_TRUE;
	BYTE Version;
	uint32 Tag;
	
	// ensure valid data
	assert( File != NULL );

	// allocate struct
	Object = (DynamicLight *)jeRam_AllocateClear( sizeof( *Object ) );
	if ( Object == NULL )
	{
		jeErrorLog_Add( JE_ERR_MEMORY_RESOURCE, NULL );
		return NULL;
	}

	// init struct
	Object->RefCount = 1;
	Object->LoadedFromDisk = JE_TRUE;

	if(!jeVFile_Read(File, &Tag, sizeof(Tag)))
	{
		jeErrorLog_Add( JE_ERR_SYSTEM_RESOURCE, NULL );
		goto ERROR_CreateFromFile;
	}

	if (Tag == FILE_UNIQUE_ID)
	{
		if (!jeVFile_Read(File, &Version, sizeof(Version)))
		{
    		jeErrorLog_Add( JE_ERR_SYSTEM_RESOURCE, NULL );
		    goto ERROR_CreateFromFile;
		}
	}
	else
	{
		Version = 1;
		jeVFile_Seek(File,-((int)sizeof(Tag)),JE_VFILE_SEEKCUR);
	}

	if (Version >= 1)
	{
	    // read data
	    Result &= jeVFile_Read( File, &( Object->Xf ), sizeof( Object->Xf ) );
	    Result &= jeVFile_Read( File, &( Object->Color ), sizeof( Object->Color ) );
        Result &= jeVFile_Read( File, &( Object->Radius ), sizeof( Object->Radius ) );
        Result &= jeVFile_Read( File, &( Object->Brightness ), sizeof( Object->Brightness ) );

		// fail if there was an error
	    if ( Result == JE_FALSE )
		{
		    jeErrorLog_Add( JE_ERR_SYSTEM_RESOURCE, NULL );
		    goto ERROR_CreateFromFile;
		}
	}

	// all done
	return Object;

	// handle errors
	ERROR_CreateFromFile:

	// free object
	jeRam_Free( Object );

	// return error
	return NULL;
	PtrMgr;

} // CreateFromFile()


////////////////////////////////////////////////////////////////////////////////////////
//
//	WriteToFile()
//
///////////////////////////////////////////////////////////////////////////////////////
jeBoolean JETCC WriteToFile(
	const void	*Instance,
	jeVFile		*File,
	jePtrMgr *PtrMgr )
{

	// locals
	DynamicLight	*Object;
	jeBoolean		Result = JE_TRUE;
	BYTE Version = DYNAMICLIGHT_VERSION;
	uint32 Tag = FILE_UNIQUE_ID;

	// ensure valid data
	assert( Instance != NULL );
	assert( File != NULL );

	// get object data
	Object = (DynamicLight *)Instance;

	// write Version
	Result &= jeVFile_Write( File, &Tag, sizeof(Tag));
	Result &= jeVFile_Write( File, &Version, sizeof(Version) );


	// write xform
	Result &= jeVFile_Write( File, &( Object->Xf ), sizeof( Object->Xf ) );

	// write color
	Result &= jeVFile_Write( File, &( Object->Color ), sizeof( Object->Color ) );

	// write radius
	Result &= jeVFile_Write( File, &( Object->Radius ), sizeof( Object->Radius ) );

	// write brightness
	Result &= jeVFile_Write( File, &( Object->Brightness ), sizeof( Object->Brightness ) );

	// log errors
	if ( Result != JE_TRUE )
	{
		jeErrorLog_Add( JE_ERR_SYSTEM_RESOURCE, NULL );
	}

	// all done
	return Result;
	PtrMgr;

	// eliminate warnings
} // WriteToFile()


////////////////////////////////////////////////////////////////////////////////////////
//
//	GetPropertyList()
//
///////////////////////////////////////////////////////////////////////////////////////
jeBoolean JETCC GetPropertyList(
	void			*Instance,	// object instance data
	jeProperty_List	**List)		// where to save property list pointer
{

	// locals
	DynamicLight	*Object;

	// ensure valid data
	assert( Instance != NULL );
	assert( List != NULL );

	// get object data
	Object = (DynamicLight *)Instance;

	// setup property list
	DynamicLightProperties[DYNAMICLIGHT_RADIUS_INDEX].Data.Float = Object->Radius;
	DynamicLightProperties[DYNAMICLIGHT_BRIGHTNESS_INDEX].Data.Float = Object->Brightness;
	DynamicLightProperties[DYNAMICLIGHT_COLOR_INDEX].Data.Vector.X = Object->Color.X;
	DynamicLightProperties[DYNAMICLIGHT_COLOR_INDEX].Data.Vector.X = Object->Color.Y;
	DynamicLightProperties[DYNAMICLIGHT_COLOR_INDEX].Data.Vector.X = Object->Color.Z;
	DynamicLightProperties[DYNAMICLIGHT_COLORRED_INDEX].Data.Float = Object->Color.X;
	DynamicLightProperties[DYNAMICLIGHT_COLORGREEN_INDEX].Data.Float = Object->Color.Y;
	DynamicLightProperties[DYNAMICLIGHT_COLORBLUE_INDEX].Data.Float = Object->Color.Z;
	DynamicLightProperties[DYNAMICLIGHT_CASTSHADOW_INDEX].Data.Bool = Object->CastShadow;

	// copy property list
	*List = jeProperty_ListCopy( &DynamicLightPropertyList );
	if ( *List == NULL )
	{
		jeErrorLog_Add( JE_ERR_INTERNAL_RESOURCE, NULL );
		return JE_FALSE;
	}

	// all done
	return JE_TRUE;

} // GetPropertyList()



////////////////////////////////////////////////////////////////////////////////////////
//
//	SetProperty()
//
///////////////////////////////////////////////////////////////////////////////////////
jeBoolean JETCC SetProperty(
	void				*Instance,	// object instance data
	int32				FieldID,	// id of field to be changed
	PROPERTY_FIELD_TYPE	DataType,	// type of data
	jeProperty_Data		*pData )	// new data
{

	// locals
	DynamicLight	*Object;
	jeBoolean		AdjustDynamicLightProperties = JE_FALSE;
	jeBoolean		Result = JE_TRUE;

	// ensure valid data
	assert( Instance != NULL );
	assert( pData != NULL );

	// get object data
	Object = (DynamicLight *)Instance;

	// process field id
	switch ( FieldID )
	{

		// adjust radius
		case DYNAMICLIGHT_RADIUS_ID:
		{
			assert( DataType == PROPERTY_FLOAT_TYPE );
			Object->Radius = pData->Float;
			AdjustDynamicLightProperties = JE_TRUE;
			break;
		}

		// adjust brightness
		case DYNAMICLIGHT_BRIGHTNESS_ID:
		{
			assert( DataType == PROPERTY_FLOAT_TYPE );
			Object->Brightness = pData->Float;
			AdjustDynamicLightProperties = JE_TRUE;
			break;
		}

		// adjust color
		case DYNAMICLIGHT_COLOR_ID:
		{
			assert( DataType == PROPERTY_COLOR_PICKER_TYPE );
			Object->Color.X = pData->Vector.X;
			Object->Color.Y = pData->Vector.Y;
			Object->Color.Z = pData->Vector.Z;
			AdjustDynamicLightProperties = JE_TRUE;
			break;
		}
		case DYNAMICLIGHT_COLORRED_ID:
		{
			assert( DataType == PROPERTY_FLOAT_TYPE );
			Object->Color.X = pData->Float;
			AdjustDynamicLightProperties = JE_TRUE;
			break;
		}
		case DYNAMICLIGHT_COLORGREEN_ID:
		{
			assert( DataType == PROPERTY_FLOAT_TYPE );
			Object->Color.Y = pData->Float;
			AdjustDynamicLightProperties = JE_TRUE;
			break;
		}
		case DYNAMICLIGHT_COLORBLUE_ID:
		{
			assert( DataType == PROPERTY_FLOAT_TYPE );
			Object->Color.Z = pData->Float;
			AdjustDynamicLightProperties = JE_TRUE;
			break;
		}

		// adjust cast shadow flag
		case DYNAMICLIGHT_CASTSHADOW_ID:
		{
			assert( DataType == PROPERTY_CHECK_TYPE );
			Object->CastShadow = pData->Bool;
			AdjustDynamicLightProperties = JE_TRUE;
			break;
		}

		// if we got to here then its an unsupported field
		default:
		{
			assert( 0 );
			return JE_FALSE;
			break;
		}
	}

	// adjust dynamic light properties if required
	if ( AdjustDynamicLightProperties == JE_TRUE )
	{
		if (Object->Light)
			{
			Result = jeLight_SetAttributes(	Object->Light,
											&( Object->Xf.Translation ),
											&( Object->Color ),
											Object->Radius, 
											Object->Brightness, 
											JE_LIGHT_FLAG_FAST_LIGHTING_MODEL );	//undone dont know flags for cast shadow
			}
#pragma message ("shadow flags")	
	}

	// all done
	return Result;

	// eliminate warnings
	DataType;

} // SetProperty()



////////////////////////////////////////////////////////////////////////////////////////
//
//	SetXForm()
//
///////////////////////////////////////////////////////////////////////////////////////
jeBoolean JETCC SetXForm(
	void			*Instance,	// object instance data
	const jeXForm3d	*Xf )		// new xform
{

	// locals
	DynamicLight	*Object;

	// ensure valid data
	assert( Instance != NULL );
	assert( Xf != NULL );

	// get object data
	Object = (DynamicLight *)Instance;

	// save xform
	Object->Xf = *Xf;

	// adjust light
	if (Object->Light)
		{
		return jeLight_SetAttributes( Object->Light,
									&( Object->Xf.Translation ),
									&( Object->Color ),
									Object->Radius, 
									Object->Brightness, 
									JE_LIGHT_FLAG_FAST_LIGHTING_MODEL );	//undone dont know flags
		}

	return JE_TRUE;
#pragma message ("shadow flags")	

} // SetXForm()



////////////////////////////////////////////////////////////////////////////////////////
//
//	GetXForm()
//
///////////////////////////////////////////////////////////////////////////////////////
jeBoolean JETCC GetXForm(
	const void	*Instance,	// object instance data
	jeXForm3d	*Xf )		// where to store xform
{

	// locals
	DynamicLight	*Object;

	// ensure valid data
	assert( Instance != NULL );
	assert( Xf != NULL );

	// get object data
	Object = (DynamicLight *)Instance;

	// save xform
	*Xf = Object->Xf;

	// all done
	return JE_TRUE;

} // GetXForm()



////////////////////////////////////////////////////////////////////////////////////////
//
//	GetXFormModFlags()
//
///////////////////////////////////////////////////////////////////////////////////////
int	JETCC GetXFormModFlags(
	const void	*Instance )	// object instance data
{

	// return xform mod flags
	return JE_OBJECT_XFORM_TRANSLATE;

	// eliminate warnings
	Instance;

} // GetXFormModFlags()



////////////////////////////////////////////////////////////////////////////////////////
//
//	GetChildren()
//
///////////////////////////////////////////////////////////////////////////////////////
jeBoolean JETCC GetChildren(
	const void	*Instance,
	jeObject	*Children,
	int			MaxNumChildren )
{

	// all done
	return JE_TRUE;

	// eliminate warnings
	Instance;
	Children;
	MaxNumChildren;

} // GetChildren()



////////////////////////////////////////////////////////////////////////////////////////
//
//	AddChild()
//
///////////////////////////////////////////////////////////////////////////////////////
jeBoolean JETCC AddChild(
	void			*Instance,
	const jeObject	*Child )
{

	// all done
	return JE_TRUE;

	// eliminate warnings
	Instance;
	Child;

} // AddChild()



////////////////////////////////////////////////////////////////////////////////////////
//
//	RemoveChild()
//
///////////////////////////////////////////////////////////////////////////////////////
jeBoolean JETCC RemoveChild(
	void			*Instance,
	const jeObject	*Child )
{

	// all done
	return JE_TRUE;

	// eliminate warnings
	Instance;
	Child;

} // RemoveChild()



////////////////////////////////////////////////////////////////////////////////////////
//
//	EditDialog()
//
///////////////////////////////////////////////////////////////////////////////////////
jeBoolean JETCC EditDialog(
	void	*Instance,
#ifdef WIN32
	HWND	Parent )
#endif
#ifdef BUILD_BE
	class G3DView* Parent)
#endif
{

	// all done
	return JE_TRUE;

	// eliminate warnings
	Instance;
	Parent;

} // EditDialog()



////////////////////////////////////////////////////////////////////////////////////////
//
//	Frame()
//
///////////////////////////////////////////////////////////////////////////////////////
jeBoolean JETCC Frame(
	void	*Instance,
	float	TimeDelta )
{
	
	// ensure valid data
	assert( Instance != NULL );

	// all done
	return JE_TRUE;

	// eliminate warnings
	Instance;
	TimeDelta;

} // Frame()



////////////////////////////////////////////////////////////////////////////////////////
//
//	SendAMessage()
//
///////////////////////////////////////////////////////////////////////////////////////
jeBoolean JETCC SendAMessage(
	void	*Instance,	// object instance data
	int32	Msg,		// message id
	void	*Data )		// message data
{

	// all done
	return JE_FALSE;

	// eliminate warnings
	Instance;
	Msg;
	Data;

} // SendAMessage()

//Royce
////////////////////////////////////////////////////////////////////////////////////////
//
//	DuplicateInstance()
//
///////////////////////////////////////////////////////////////////////////////////////
void * JETCC DuplicateInstance(void * Instance)
{
	jeVFile *ramdisk, *ramfile;
	jeVFile_MemoryContext vfsmemctx;
	jeObject* newDLight = NULL;
	jePtrMgr *ptrMgr = NULL;

	vfsmemctx.Data = jeRam_Allocate(OBJ_PERSIST_SIZE); //"I dunno, 100K sounds good."
	vfsmemctx.DataLength = OBJ_PERSIST_SIZE;

	if (!vfsmemctx.Data) {
		jeErrorLog_AddString(JE_ERR_SUBSYSTEM_FAILURE, "Unable to allocate enough RAM to duplicate this object", NULL);
		return NULL;
	}

	ramdisk = jeVFile_OpenNewSystem
	(
		NULL, 
		(jeVFile_TypeIdentifier) (JE_VFILE_TYPE_MEMORY|JE_VFILE_TYPE_VIRTUAL),
		"Memory",
		NULL,
		JE_VFILE_OPEN_CREATE|JE_VFILE_OPEN_DIRECTORY
	);

	if (!ramdisk) {
		jeErrorLog_AddString(JE_ERR_SUBSYSTEM_FAILURE, "Unable to create a VFile Memory Directory", NULL);
		jeRam_Free(vfsmemctx.Data);
		return NULL;
	}

	ramfile = jeVFile_Open(ramdisk, "tempObject", JE_VFILE_OPEN_CREATE);

	if (!ramfile) {
		jeErrorLog_AddString(JE_ERR_SUBSYSTEM_FAILURE, "Unable to create a VFile Memory File", NULL);
		jeVFile_Close(ramdisk);
		jeRam_Free(vfsmemctx.Data);
		return NULL;
	}
	ptrMgr = jePtrMgr_Create();

	if (!ptrMgr) {
		jeErrorLog_AddString(JE_ERR_SUBSYSTEM_FAILURE, "Unable to create a Pointer Manager", NULL);
		jeVFile_Close(ramfile);
		jeVFile_Close(ramdisk);
		jeRam_Free(vfsmemctx.Data);
		return NULL;
	}

	if (!WriteToFile(Instance, ramfile, jePtrMgr_Create())) {
		jeErrorLog_AddString(JE_ERR_SUBSYSTEM_FAILURE, "Unable to write the object to a temp VFile Memory File", NULL);
		jeVFile_Close(ramfile);
		jeVFile_Close(ramdisk);
		jeRam_Free(vfsmemctx.Data);
		return NULL;
	}

	if (!jeVFile_Rewind(ramfile)) {
		jeErrorLog_AddString(JE_ERR_SUBSYSTEM_FAILURE, "Unable to rewind the temp VFile Memory File", NULL);
		jeVFile_Close(ramfile);
		jeVFile_Close(ramdisk);
		jeRam_Free(vfsmemctx.Data);
		return NULL;
	}

	newDLight = (jeObject *)CreateFromFile(ramfile, ptrMgr);
	if (!newDLight) {
		jeErrorLog_AddString(JE_ERR_SUBSYSTEM_FAILURE, "Unable to reade the object back from a temp VFile Memory File", NULL);
		jeVFile_Close(ramfile);
		jeVFile_Close(ramdisk);
		jeRam_Free(vfsmemctx.Data);
		return NULL;
	}

	jeVFile_Close(ramfile);
	jeVFile_Close(ramdisk);

	jeRam_Free(vfsmemctx.Data);

	return( newDLight );
}
//---

// Icestorm
jeBoolean	JETCC ChangeBoxCollision(const void *Instance,const jeVec3d *Pos, const jeExtBox *FrontBox, const jeExtBox *BackBox, jeExtBox *ImpactBox, jePlane *Plane)
{
	return( JE_FALSE );Plane;ImpactBox;BackBox;FrontBox;Pos;Instance;
}