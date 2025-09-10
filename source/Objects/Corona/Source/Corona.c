/****************************************************************************************/
/*  CORONA.C                                                                            */
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
#pragma warning ( disable : 4115 )
#include <windows.h>
#pragma warning ( default : 4115 )
#include <float.h>
#include <assert.h>
#include <string.h>
#include "vfile.h"
#include "jeProperty.h"
#include "ram.h"
#include "jeResource.h"
#include "jeWorld.h"
#include "Corona.h"
#include "resource.h"
#include "Errorlog.h"
#include "ObjectMsg.h"
#include "ObjUtil.h"
#include "jeMaterial.h"
#include "jeResource.h"

#define CORONA_VERSION_NUMBER 1


//Royce
#define OBJ_PERSIST_SIZE 5000
//---

////////////////////////////////////////////////////////////////////////////////////////
//	Property list stuff
////////////////////////////////////////////////////////////////////////////////////////
enum
{

	// misc stuff
	CORONA_FADETIME_ID = PROPERTY_LOCAL_DATATYPE_START,
	CORONA_MAXVISIBLEDISTANCE_ID,
	CORONA_MINRADIUS_ID,
	CORONA_MAXRADIUS_ID,
	CORONA_MINRADIUSDISTANCE_ID,
	CORONA_MAXRADIUSDISTANCE_ID,

	// texture group
	CORONA_ARTGROUP_ID,
	CORONA_ARTSIZE_ID,
	CORONA_ARTBITMAP_ID,
	CORONA_ARTALPHA_ID,
	CORONA_ARTGROUPEND_ID,

	// end marker
	CORONA_LAST_ID

};
enum
{

	// misc stuff
	CORONA_FADETIME_INDEX = 0,
	CORONA_MAXVISIBLEDISTANCE_INDEX,
	CORONA_MINRADIUS_INDEX,
	CORONA_MAXRADIUS_INDEX,
	CORONA_MINRADIUSDISTANCE_INDEX,
	CORONA_MAXRADIUSDISTANCE_INDEX,

	// texture group
	CORONA_ARTGROUP_INDEX,
	CORONA_ARTSIZE_INDEX,
	CORONA_ARTBITMAP_INDEX,
	CORONA_ARTALPHA_INDEX,
	CORONA_ARTGROUPEND_INDEX,

	// end marker
	CORONA_LAST_INDEX

};


////////////////////////////////////////////////////////////////////////////////////////
//	Globals
////////////////////////////////////////////////////////////////////////////////////////
static HINSTANCE		hClassInstance = NULL;
static BitmapList		*AvailableArt = NULL;
static jeProperty		CoronaProperties[CORONA_LAST_INDEX];
static jeProperty_List	CoronaPropertyList = { CORONA_LAST_INDEX, &( CoronaProperties[0] ) };
static char				*NoSelection = "< none >";
static jeMaterialSpec *MatSpec;

////////////////////////////////////////////////////////////////////////////////////////
//	Defaults
////////////////////////////////////////////////////////////////////////////////////////

#define CORONA_DEFAULT_FADETIME				2.0f
#define CORONA_DEFAULT_MINRADIUS			1.0f
#define CORONA_DEFAULT_MAXRADIUS			4.0f
#define	CORONA_DEFAULT_MINRADIUSDISTANCE	100.0f
#define	CORONA_DEFAULT_MAXRADIUSDISTANCE	1000.0f
#define CORONA_DEFAULT_MAXVISIBLEDISTANCE	1000.0f


////////////////////////////////////////////////////////////////////////////////////////
//	Object data
////////////////////////////////////////////////////////////////////////////////////////
typedef struct Corona
{

	// standard stuff
	jeWorld			*World;
	jeResourceMgr	*ResourceMgr;
	jeEngine		*Engine;
	int				RefCount;
	jeBoolean		LoadedFromDisk;

	// internal stuff
	jeBitmap	*Art;				// current art
	char		*ArtName;			// current art name
	char		*SizeString;		// size string loaded from disk
	float		LastVisibleRadius;	// last visible radius
	jeFloat		DistanceToCorona;	// distance from camera to corona
	jeBoolean	Visible;			// whether or not its visible

	// user adjustable stuff
	jeXForm3d	Xf;
	char		*BitmapName;		// name of chosen bitmap
	char		*AlphaName;			// name of chosen alpha
	jeFloat		FadeTime;			// how many seconds to spend fading away the corona
    float		MinRadius;			// mix corona radius
    float		MaxRadius;			// max corona radius
	float		MinRadiusDistance;	// below this distance, corona is capped at MinRadius
	float		MaxRadiusDistance;	// above this distance, corona is capped at MaxRadius
	float		MaxVisibleDistance;	// beyond this distance the corona is not visible

} Corona;



////////////////////////////////////////////////////////////////////////////////////////
//
//	InitClass()
//
////////////////////////////////////////////////////////////////////////////////////////
jeBoolean InitClass(
	HINSTANCE	hInstance )	// dll instance handle
{

	// locals
	char	*String;

	// ensure valid data
	assert( hInstance != NULL );

	// save hinstance
	hClassInstance = hInstance;


	////////////////////////////////////////////////////////////////////////////////////////
	//	Setup start and end of texture group
	////////////////////////////////////////////////////////////////////////////////////////
	String = ObjUtil_LoadLibraryString( hClassInstance, IDS_ARTGROUP );
	if ( String == NULL )
	{
		ObjUtil_LogError( hClassInstance, JE_ERR_SUBSYSTEM_FAILURE, IDS_ERROR_GetString );
		goto ERROR_InitClass;
	}
	jeProperty_FillGroup( &( CoronaProperties[CORONA_ARTGROUP_INDEX] ), String, CORONA_ARTGROUP_ID );
	jeProperty_FillGroupEnd( &( CoronaProperties[CORONA_ARTGROUPEND_INDEX] ), CORONA_ARTGROUPEND_ID );


	////////////////////////////////////////////////////////////////////////////////////////
	//	Misc properties
	////////////////////////////////////////////////////////////////////////////////////////

	// fade time
	String = ObjUtil_LoadLibraryString( hClassInstance, IDS_FADETIME );
	if ( String == NULL )
	{
		ObjUtil_LogError( hClassInstance, JE_ERR_SUBSYSTEM_FAILURE, IDS_ERROR_GetString );
		goto ERROR_InitClass;
	}
	jeProperty_FillFloat(	&( CoronaProperties[CORONA_FADETIME_INDEX] ), String,
							CORONA_DEFAULT_FADETIME, CORONA_FADETIME_ID, 0.0f, FLT_MAX, 0.1f );

	// max visible distance
	String = ObjUtil_LoadLibraryString( hClassInstance, IDS_MAXVISIBLEDISTANCE );
	if ( String == NULL )
	{
		ObjUtil_LogError( hClassInstance, JE_ERR_SUBSYSTEM_FAILURE, IDS_ERROR_GetString );
		goto ERROR_InitClass;
	}
	jeProperty_FillFloat(	&( CoronaProperties[CORONA_MAXVISIBLEDISTANCE_INDEX] ), String,
							CORONA_DEFAULT_MAXVISIBLEDISTANCE, CORONA_MAXVISIBLEDISTANCE_ID, 1.0f, FLT_MAX, 100.0f );

	// min radius
	String = ObjUtil_LoadLibraryString( hClassInstance, IDS_MINRADIUS );
	if ( String == NULL )
	{
		ObjUtil_LogError( hClassInstance, JE_ERR_SUBSYSTEM_FAILURE, IDS_ERROR_GetString );
		goto ERROR_InitClass;
	}
	jeProperty_FillFloat(	&( CoronaProperties[CORONA_MINRADIUS_INDEX] ), String,
							CORONA_DEFAULT_MINRADIUS, CORONA_MINRADIUS_ID, 0.1f, FLT_MAX, 0.1f );

	// max radius
	String = ObjUtil_LoadLibraryString( hClassInstance, IDS_MAXRADIUS );
	if ( String == NULL )
	{
		ObjUtil_LogError( hClassInstance, JE_ERR_SUBSYSTEM_FAILURE, IDS_ERROR_GetString );
		goto ERROR_InitClass;
	}
	jeProperty_FillFloat(	&( CoronaProperties[CORONA_MAXRADIUS_INDEX] ), String,
							CORONA_DEFAULT_MAXRADIUS, CORONA_MAXRADIUS_ID, 0.1f, FLT_MAX, 0.1f );

	// min radius distance
	String = ObjUtil_LoadLibraryString( hClassInstance, IDS_MINRADIUSDISTANCE );
	if ( String == NULL )
	{
		ObjUtil_LogError( hClassInstance, JE_ERR_SUBSYSTEM_FAILURE, IDS_ERROR_GetString );
		goto ERROR_InitClass;
	}
	jeProperty_FillFloat(	&( CoronaProperties[CORONA_MINRADIUSDISTANCE_INDEX] ), String,
							CORONA_DEFAULT_MINRADIUSDISTANCE, CORONA_MINRADIUSDISTANCE_ID, 0.0f, FLT_MAX, 100.0f );

	// max radius distance
	String = ObjUtil_LoadLibraryString( hClassInstance, IDS_MAXRADIUSDISTANCE );
	if ( String == NULL )
	{
		ObjUtil_LogError( hClassInstance, JE_ERR_SUBSYSTEM_FAILURE, IDS_ERROR_GetString );
		goto ERROR_InitClass;
	}
	jeProperty_FillFloat(	&( CoronaProperties[CORONA_MAXRADIUSDISTANCE_INDEX] ), String,
							CORONA_DEFAULT_MAXRADIUSDISTANCE, CORONA_MAXRADIUSDISTANCE_ID, 0.0f, FLT_MAX, 100.0f );


	////////////////////////////////////////////////////////////////////////////////////////
	//	End marker
	////////////////////////////////////////////////////////////////////////////////////////
	CoronaPropertyList.jePropertyN = CORONA_LAST_INDEX;

	// all done
	return JE_TRUE;


	//
	//	error handling
	//
	ERROR_InitClass:

	// reset misc stuff
	hClassInstance = NULL;
	CoronaPropertyList.jePropertyN = 0;

	// return failure
	ObjUtil_LogError( hClassInstance, JE_ERR_SUBSYSTEM_FAILURE, IDS_ERROR_GetString );
	return JE_FALSE;

} // InitClass()



////////////////////////////////////////////////////////////////////////////////////////
//
//	DeInitClass()
//
////////////////////////////////////////////////////////////////////////////////////////
jeBoolean DeInitClass(
	void )	// no parameters
{

	// free available art list
	if ( AvailableArt != NULL )
	{
		ObjUtil_DestroyBitmapList( &AvailableArt );
	}

	// zap instance pointer
	hClassInstance = NULL;

	// all done
	 return JE_TRUE;

} // DeInitClass()



////////////////////////////////////////////////////////////////////////////////////////
//
//	CreateInstance()
//
////////////////////////////////////////////////////////////////////////////////////////
void * JETCC CreateInstance(
	void )	// no parameters
{

	// locals
	Corona	*Object;

	// allocate struct
	Object = jeRam_AllocateClear( sizeof( *Object ) );
	if ( Object == NULL )
	{
		ObjUtil_LogError( hClassInstance, JE_ERR_MEMORY_RESOURCE, IDS_ERROR_CreateInstance_AllocateObject );
		goto ERROR_CreateInstance;
	}

	// set defaults
	jeXForm3d_SetIdentity( &( Object->Xf ) );
	Object->RefCount = 1;
	Object->FadeTime = CORONA_DEFAULT_FADETIME;
	Object->MinRadius = CORONA_DEFAULT_MINRADIUS;
	Object->MaxRadius = CORONA_DEFAULT_MAXRADIUS;
	Object->MinRadiusDistance = CORONA_DEFAULT_MINRADIUSDISTANCE;
	Object->MaxRadiusDistance = CORONA_DEFAULT_MAXRADIUSDISTANCE;
	Object->MaxVisibleDistance = CORONA_DEFAULT_MAXVISIBLEDISTANCE;

	// all done
	return Object;


	//
	//	error handling
	//
	ERROR_CreateInstance:

	// free object
	if ( Object != NULL )
	{
		// free object itself
		jeRam_Free( Object );
		Object = NULL;
	}

	// return failure
	ObjUtil_LogError( hClassInstance, JE_ERR_SUBSYSTEM_FAILURE, IDS_ERROR_CreateInstance_Failure );
	return NULL;

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
	Corona	*Object;
	
	// get object
	Object = (Corona *)Instance;
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
	Corona	*Object;

	// ensure valid data
	assert( Instance != NULL );

	// get object
	Object = (Corona *)*Instance;
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
	const void				*Instance,				// object instance data
	const jeWorld			*World,					// world
	const jeEngine			*Engine,				// engine
	const jeCamera			*Camera,				// camera
	const jeFrustum			*CameraSpaceFrustum,	// frustum 	
	jeObject_RenderFlags	RenderFlags )			// render flags
{

	// locals
	Corona	*Object;
    jeVec3d			Delta;
    jeXForm3d		CameraXf;

	// ensure valid data
	assert( Instance != NULL );
	assert( World != NULL );
	assert( Engine != NULL );
	assert( Camera != NULL );
	assert( CameraSpaceFrustum != NULL );

	// get object
	Object = (Corona *)Instance;
	assert( Object != NULL );

	// determine corona distance and visibility
	{

		// locals
		jeCollisionInfo	CollisionInfo;
		jeExtBox		ExtBox;

		// get camera xform
		jeCamera_GetXForm( Camera, &CameraXf );

		// determine distance from corona to camera
		jeVec3d_Subtract( &( Object->Xf.Translation ), &( CameraXf.Translation ), &Delta );
		Object->DistanceToCorona = jeVec3d_Length( &Delta );

		// setup extent box
        ExtBox.Min.X = -1.0f;
        ExtBox.Min.Y = -1.0f;
        ExtBox.Min.Z = -1.0f;
        ExtBox.Max.X = 1.0f;
        ExtBox.Max.Y = 1.0f;
        ExtBox.Max.Z = 1.0f;
//!!		jeExtBox_SetToPoint ( &ExtBox, &( Object->Xf.Translation ) );
//!!		jeExtBox_ExtendToEnclose( &ExtBox, &( Object->Xf.Translation ) );

		// determine whether or not corona is visible
		if ( Object->DistanceToCorona > Object->MaxVisibleDistance )
		{
			Object->Visible = JE_FALSE;
		}
		else
		{
			Object->Visible = !(jeWorld_Collision( World, &ExtBox, &( Object->Xf.Translation ), &( CameraXf.Translation ), &CollisionInfo ));
		}
	}

	// update the art
	if (	( Object->Art != NULL ) &&
			( Object->LastVisibleRadius > 0.0f ) )
	{

		// locals
		jeUserPoly	*Poly;
		jeLVertex	Vertex;
        jeVec3d position;

		// setup vert
		Vertex.a = 255.0f;
		Vertex.r = 255.0f;
		Vertex.g = 255.0f;
		Vertex.b = 255.0f;
		Vertex.u = 0.0f;
		Vertex.v = 0.0f;
        jeVec3d_Scale(&Delta,4.0f/Object->DistanceToCorona,&position);
        jeVec3d_Add(&position,&(CameraXf.Translation),&position);
//!!		Vertex.X = Object->Xf.Translation.X;
//!!		Vertex.Y = Object->Xf.Translation.Y;
//!!		Vertex.Z = Object->Xf.Translation.Z;
        Vertex.X = position.X;
        Vertex.Y = position.Y;
        Vertex.Z = position.Z;

		// add poly
		if (!MatSpec)
    	{
	        MatSpec = jeMaterialSpec_Create(jeResourceMgr_GetEngine(jeResourceMgr_GetSingleton()), jeResourceMgr_GetSingleton());
#pragma message ("Krouer: change NULL to something better next time")
	        jeMaterialSpec_AddLayerFromBitmap(MatSpec, 0, Object->Art, NULL);
	    }
  
		Poly = jeUserPoly_CreateSprite( &Vertex, MatSpec, (Object->LastVisibleRadius * 4.0f)/Object->DistanceToCorona, 0 );
		if ( Poly == NULL )
		{
			ObjUtil_LogError( hClassInstance, JE_ERR_SUBSYSTEM_FAILURE, IDS_ERROR_Render_AddPoly );
		}
		else
		{
			jeWorld_AddUserPoly( (jeWorld *)World, Poly, JE_TRUE );
			jeUserPoly_Destroy( &Poly );
		}
	}

	// all done
	return JE_TRUE;

	// eliminate warnings
	CameraSpaceFrustum;
	RenderFlags;
	Engine;

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
	Corona	*Object;

	// ensure valid data
	assert( Instance != NULL );
	assert( World != NULL );

	// get object
	Object = (Corona *)Instance;

	// save world pointer
	Object->World = World;

	// save an instance of the resource manager
	Object->ResourceMgr = jeWorld_GetResourceMgr( World );
	assert( Object->ResourceMgr != NULL );

	// build bitmap list if required
	if ( AvailableArt == NULL )
	{

		// build list
		AvailableArt = ObjUtil_CreateBitmapList( Object->ResourceMgr, "GlobalMaterials", "*.bmp" );
		if ( AvailableArt == NULL )
		{
			ObjUtil_LogError( hClassInstance, JE_ERR_SUBSYSTEM_FAILURE, IDS_ERROR_AttachWorld_CreateBitmapList );
			goto ERROR_AttachWorld;
		}
	}

	// set object art defaults
	if ( Object->LoadedFromDisk == JE_FALSE )
	{
		Object->BitmapName = AvailableArt->Name[0];
		Object->AlphaName = AvailableArt->Name[0];
	}

	// all done
	return JE_TRUE;


	//
	//	error handling
	//
	ERROR_AttachWorld:

	// destroy bitmap list
	if ( AvailableArt != NULL )
	{
		ObjUtil_DestroyBitmapList( &AvailableArt );
	}

	// destroy our instance of the resource manager
	if ( Object->ResourceMgr != NULL )
	{
		jeResource_MgrDestroy( &( Object->ResourceMgr ) );
	}

	// return failure
	ObjUtil_LogError( hClassInstance, JE_ERR_SUBSYSTEM_FAILURE, IDS_ERROR_AttachWorld_Failure );
	return JE_FALSE;

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
	Corona	*Object;

	// ensure valid data
	assert( Instance != NULL );
	assert( World != NULL );

	// get object
	Object = (Corona *)Instance;
	assert( Object->World == World );

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
	Corona	*Object;

	// ensure valid data
	assert( Instance != NULL );
	assert( Engine != NULL );

	// get object data
	Object = (Corona *)Instance;

	// save engine pointer
	Object->Engine = Engine;

	// set properties if object was loaded from disk
	if ( Object->LoadedFromDisk == JE_TRUE )
	{

		// locals
		jeBoolean	Result = JE_TRUE;
		char		*LoadBitmapName, *LoadAlphaName;

		// reset loaded from disk flag
		Object->LoadedFromDisk = JE_FALSE;

		// save allocated string pointers
		LoadBitmapName = Object->BitmapName;
		LoadAlphaName = Object->AlphaName;
		Object->BitmapName = AvailableArt->Name[0];
		Object->AlphaName = AvailableArt->Name[0];

		// set texture group size
		ObjUtil_TextureGroupSetSize(	Object->Engine, Object->ResourceMgr,
										AvailableArt, Object->SizeString,
										&( Object->BitmapName ), &( Object->AlphaName ),
										&( Object->Art ), &( Object->ArtName ) );

		// set bitmap name
		Result &= ObjUtil_TextureGroupSetArt(	Object->Engine, Object->ResourceMgr,
												AvailableArt, LoadBitmapName, NULL,
												&( Object->BitmapName ), &( Object->AlphaName ),
												&( Object->Art ), &( Object->ArtName ) );

		// set alpha name
		Result &= ObjUtil_TextureGroupSetArt(	Object->Engine, Object->ResourceMgr,
												AvailableArt, NULL, LoadAlphaName,
												&( Object->BitmapName ), &( Object->AlphaName ),
												&( Object->Art ), &( Object->ArtName ) );

		// free strings allocated on load
		jeRam_Free( LoadBitmapName );
		jeRam_Free( LoadAlphaName );

		// log errors
		if ( Result == JE_FALSE )
		{
			goto ERROR_AttachEngine;
		}
	}

	// all done
	return JE_TRUE;


	//
	//	error handling
	//
	ERROR_AttachEngine:

	// return failure
	ObjUtil_LogError( hClassInstance, JE_ERR_SUBSYSTEM_FAILURE, IDS_ERROR_AttachEngine );
	return JE_FALSE;

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
	Corona	*Object;

	// ensure valid data
	assert( Instance != NULL );
	assert( Engine != NULL );

	// get object data
	Object = (Corona *)Instance;
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
	Corona	*Object;

	// ensure valid data
	assert( Instance != NULL );
	assert( BBox != NULL );

	// get object data
	Object = (Corona *)Instance;

	// setup extent box	//hack
	{

		// locals
		jeVec3d Pos;
		float	Size = 25.0f;

		// save extent box
		Pos = Object->Xf.Translation;
		jeExtBox_Set (  BBox, 
						Pos.X - Size, Pos.Y - Size, Pos.Z - Size,
						Pos.X + Size, Pos.Y + Size, Pos.Z + Size );
	}

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
	jePtrMgr	*PtrMgr )	// pointer manager
{

	// locals
	jeFloat	Ver;
	BYTE Version;
	uint32 Tag;
	Corona		*Object;
	jeBoolean	Result = JE_TRUE;


	// ensure valid data
	assert( File != NULL );
	assert( PtrMgr != NULL );

	// create new object
	Object = CreateInstance();
	if ( Object == NULL )
	{
		ObjUtil_LogError( hClassInstance, JE_ERR_SUBSYSTEM_FAILURE, IDS_ERROR_CreateFromFile_CreateObject );
		goto ERROR_CreateFromFile;
	}

	
	if(!jeVFile_Read(File, &Tag, sizeof(Tag)))
	{
		ObjUtil_LogError( hClassInstance, JE_ERR_SUBSYSTEM_FAILURE, IDS_ERROR_CreateFromFile_ReadData );
		goto ERROR_CreateFromFile;
	}

	if (Tag == FILE_UNIQUE_ID)
	{
		if (!jeVFile_Read( File, &Version, sizeof( Version )))
		{
		    ObjUtil_LogError( hClassInstance, JE_ERR_SUBSYSTEM_FAILURE, IDS_ERROR_CreateFromFile_ReadData );
		    goto ERROR_CreateFromFile;
		}

	}
	else
	{
		//for backwards compatibility with old object format
		jeVFile_Seek(File,-((int)sizeof(Tag)),JE_VFILE_SEEKCUR);
		if (!jeVFile_Read( File, &Ver, sizeof( Ver )))
		{
		    ObjUtil_LogError( hClassInstance, JE_ERR_SUBSYSTEM_FAILURE, IDS_ERROR_CreateFromFile_ReadData );
		    goto ERROR_CreateFromFile;
		}
		Version = 1;
		
	}
	
	if ( Version >= 1 )
	{
		//read data
		Result &= ObjUtil_ReadString( File, &( Object->SizeString ) );
	    Result &= ObjUtil_ReadString( File, &( Object->BitmapName ) );
	    Result &= ObjUtil_ReadString( File, &( Object->AlphaName ) );
	    Result &= jeVFile_Read( File, &( Object->Xf ), sizeof( Object->Xf ) );
	    Result &= jeVFile_Read( File, &( Object->FadeTime ), sizeof( Object->FadeTime ) );
	    Result &= jeVFile_Read( File, &( Object->MinRadius ), sizeof( Object->MinRadius ) );
	    Result &= jeVFile_Read( File, &( Object->MaxRadius ), sizeof( Object->MaxRadius ) );
	    Result &= jeVFile_Read( File, &( Object->MinRadiusDistance ), sizeof( Object->MinRadiusDistance ) );
	    Result &= jeVFile_Read( File, &( Object->MaxRadiusDistance ), sizeof( Object->MaxRadiusDistance ) );
	    Result &= jeVFile_Read( File, &( Object->MaxVisibleDistance ), sizeof( Object->MaxVisibleDistance ) );

	    // log errors
	    if ( Result == JE_FALSE )
		{
		    ObjUtil_LogError( hClassInstance, JE_ERR_SUBSYSTEM_FAILURE, IDS_ERROR_CreateFromFile_ReadData );
		    goto ERROR_CreateFromFile;
		}
	}
	
	// all done
	Object->LoadedFromDisk = JE_TRUE;

	return Object;


	//
	// error handling
	//
	ERROR_CreateFromFile:

	// free object
	if ( Object != NULL )
	{

		// free strings
		if ( Object->SizeString != NULL )
		{
			jeRam_Free( Object->SizeString );
		}
		if ( Object->BitmapName != NULL )
		{
			jeRam_Free( Object->BitmapName );
		}
		if ( Object->AlphaName != NULL )
		{
			jeRam_Free( Object->AlphaName );
		}

		// free object itself
		jeRam_Free( Object );
		Object = NULL;
	}

	// return error
	return NULL;

	// eliminate warnings
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
	jePtrMgr	*PtrMgr )
{

	// locals
    BYTE Version = CORONA_VERSION_NUMBER;
	uint32 Tag = FILE_UNIQUE_ID;
	Corona		*Object;
	jeBoolean	Result = JE_TRUE;

	// ensure valid data
	assert( Instance != NULL );
	assert( File != NULL );
	assert( PtrMgr != NULL );

	// get object data
	Object = (Corona *)Instance;

	// write version number
	Result &= jeVFile_Write( File, &Tag, sizeof(Tag));
	Result &= jeVFile_Write( File, &Version, sizeof( Version ) );
	
	// write out data
	Result &= ObjUtil_WriteString( File, AvailableArt->StringSizes[AvailableArt->ActiveCurSize] );
	Result &= ObjUtil_WriteString( File, Object->BitmapName );
	Result &= ObjUtil_WriteString( File, Object->AlphaName );
	Result &= jeVFile_Write( File, &( Object->Xf ), sizeof( Object->Xf ) );
	Result &= jeVFile_Write( File, &( Object->FadeTime ), sizeof( Object->FadeTime ) );
	Result &= jeVFile_Write( File, &( Object->MinRadius ), sizeof( Object->MinRadius ) );
	Result &= jeVFile_Write( File, &( Object->MaxRadius ), sizeof( Object->MaxRadius ) );
	Result &= jeVFile_Write( File, &( Object->MinRadiusDistance ), sizeof( Object->MinRadiusDistance ) );
	Result &= jeVFile_Write( File, &( Object->MaxRadiusDistance ), sizeof( Object->MaxRadiusDistance ) );
	Result &= jeVFile_Write( File, &( Object->MaxVisibleDistance ), sizeof( Object->MaxVisibleDistance ) );

	// log errors
	if ( Result == JE_FALSE )
	{
		ObjUtil_LogError( hClassInstance, JE_ERR_FILEIO_WRITE, IDS_ERROR_WriteToFile );
	}

	// all done
	return Result;

	// eliminate warnings
	PtrMgr;

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
	Corona	*Object;

	// ensure valid data
	assert( Instance != NULL );
	assert( List != NULL );

	// get object data
	Object = (Corona *)Instance;

	// set properties
	CoronaProperties[CORONA_FADETIME_INDEX].Data.Float = Object->FadeTime;
	CoronaProperties[CORONA_MAXVISIBLEDISTANCE_INDEX].Data.Float = Object->MaxVisibleDistance;
	CoronaProperties[CORONA_MINRADIUS_INDEX].Data.Float = Object->MinRadius;
	CoronaProperties[CORONA_MAXRADIUS_INDEX].Data.Float = Object->MaxRadius;
	CoronaProperties[CORONA_MINRADIUSDISTANCE_INDEX].Data.Float = Object->MinRadiusDistance;
	CoronaProperties[CORONA_MAXRADIUSDISTANCE_INDEX].Data.Float = Object->MaxRadiusDistance;

	// setup texture group properties
	{

		// locals
		char	*ArtSize, *ArtBitmap, *ArtAlpha;

		// get strings
		ArtSize = ObjUtil_LoadLibraryString( hClassInstance, IDS_ARTSIZE );
		ArtBitmap = ObjUtil_LoadLibraryString( hClassInstance, IDS_ARTBITMAP );
		ArtAlpha = ObjUtil_LoadLibraryString( hClassInstance, IDS_ARTALPHA );

		// check strings
		if ( ( ArtSize == NULL ) || ( ArtBitmap == NULL ) || ( ArtAlpha == NULL ) )
		{
			ObjUtil_LogError( hClassInstance, JE_ERR_SUBSYSTEM_FAILURE, IDS_ERROR_GetString );
			goto ERROR_GetPropertyList;
		}

		// init properties
		jeProperty_FillCombo(	&( CoronaProperties[CORONA_ARTSIZE_INDEX] ), ArtSize,
								AvailableArt->StringSizes[AvailableArt->ActiveCurSize],
								CORONA_ARTSIZE_ID, AvailableArt->SizesListSize, AvailableArt->StringSizes );
		jeProperty_FillCombo(	&( CoronaProperties[CORONA_ARTBITMAP_INDEX] ), ArtBitmap,
								Object->BitmapName, CORONA_ARTBITMAP_ID, AvailableArt->ActiveCount, AvailableArt->ActiveList );
		jeProperty_FillCombo(	&( CoronaProperties[CORONA_ARTALPHA_INDEX] ), ArtAlpha,
								Object->AlphaName, CORONA_ARTALPHA_ID, AvailableArt->ActiveCount, AvailableArt->ActiveList );
	}

	// copy property list
	*List = jeProperty_ListCopy( &CoronaPropertyList );
	if ( *List == NULL )
	{
		ObjUtil_LogError( hClassInstance, JE_ERR_SUBSYSTEM_FAILURE, IDS_ERROR_CreateFromFile_CreateObject );
		return JE_FALSE;
	}

	// reset dirty flag
	CoronaPropertyList.bDirty = JE_FALSE;

	// all done
	return JE_TRUE;


	//
	//	error handling
	//
	ERROR_GetPropertyList:

	// return failure
	ObjUtil_LogError( hClassInstance, JE_ERR_SUBSYSTEM_FAILURE, IDS_ERROR_GetPropertyList );
	return JE_FALSE;

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
	Corona	*Object;

	// ensure valid data
	assert( Instance != NULL );
	assert( pData != NULL );

	// get object data
	Object = (Corona *)Instance;

	// process field id
	switch ( FieldID )
	{

		// adjust fade time
		case CORONA_FADETIME_ID:
		{
			assert( DataType == PROPERTY_FLOAT_TYPE );
			Object->FadeTime = pData->Float;
			break;
		}

		// adjust max visible distance
		case CORONA_MAXVISIBLEDISTANCE_ID:
		{
			assert( DataType == PROPERTY_FLOAT_TYPE );
			Object->MaxVisibleDistance = pData->Float;
			break;
		}

		// adjust min radius
		case CORONA_MINRADIUS_ID:
		{
			assert( DataType == PROPERTY_FLOAT_TYPE );
			if ( pData->Float <= Object->MaxRadius )
			{
				Object->MinRadius = pData->Float;
			}
			break;
		}

		// adjust max radius
		case CORONA_MAXRADIUS_ID:
		{
			assert( DataType == PROPERTY_FLOAT_TYPE );
			if ( pData->Float >= Object->MinRadius )
			{
				Object->MaxRadius = pData->Float;
			}
			break;
		}

		// adjust min radius distance
		case CORONA_MINRADIUSDISTANCE_ID:
		{
			assert( DataType == PROPERTY_FLOAT_TYPE );
			if ( pData->Float <= Object->MaxRadiusDistance )
			{
				Object->MinRadiusDistance = pData->Float;
			}
			break;
		}

		// adjust max radius distance
		case CORONA_MAXRADIUSDISTANCE_ID:
		{
			assert( DataType == PROPERTY_FLOAT_TYPE );
			if ( pData->Float >= Object->MinRadiusDistance )
			{
				Object->MaxRadiusDistance = pData->Float;
			}
			break;
		}

		// process art size choice
		case CORONA_ARTSIZE_ID:
		{

			// ensure valid data
			assert( DataType == PROPERTY_COMBO_TYPE );
			assert( pData->String != NULL );

			// process new size choice
			ObjUtil_TextureGroupSetSize(	Object->Engine, Object->ResourceMgr,
											AvailableArt, pData->String,
											&( Object->BitmapName ), &( Object->AlphaName ),
											&( Object->Art ), &( Object->ArtName ) );

			// set dirty flag
			CoronaPropertyList.bDirty = JE_TRUE;
			break;
		}

		// process art choice
		case CORONA_ARTBITMAP_ID:
		case CORONA_ARTALPHA_ID:
		{

			// locals
			jeBoolean	Result;

			// ensure valid data
			assert( DataType == PROPERTY_COMBO_TYPE );
			assert( pData->String != NULL );

			// process new bitmap choice...
			if ( FieldID == CORONA_ARTBITMAP_ID )
			{
				Result = ObjUtil_TextureGroupSetArt(	Object->Engine, Object->ResourceMgr,
														AvailableArt, pData->String, NULL,
														&( Object->BitmapName ), &( Object->AlphaName ),
														&( Object->Art ), &( Object->ArtName ) );
			}
			// ...or alpha choice
			else
			{
				Result = ObjUtil_TextureGroupSetArt(	Object->Engine, Object->ResourceMgr,
														AvailableArt, NULL, pData->String,
														&( Object->BitmapName ), &( Object->AlphaName ),
														&( Object->Art ), &( Object->ArtName ) );
			}

			// log errors
			if ( Result == JE_FALSE )
			{
				ObjUtil_LogError( hClassInstance, JE_ERR_SUBSYSTEM_FAILURE, IDS_ERROR_SetProperty );
				return JE_FALSE;
			}
			break;
		}

		// if we got to here then its an unsupported field
		default:
		{
			return JE_FALSE;
			break;
		}
	}

	// all done
	return JE_TRUE;

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
	Corona	*Object;
	jeVec3d	Pos;

	// ensure valid data
	assert( Instance != NULL );
	assert( Xf != NULL );

	// get object data
	Object = (Corona *)Instance;

	// save xform
	Object->Xf = *Xf;
	jeVec3d_Copy( &( Object->Xf.Translation ), &Pos );
	jeXForm3d_Orthonormalize( &( Object->Xf ) );
	jeVec3d_Copy( &Pos, &( Object->Xf.Translation ) );

	// all done
	return JE_TRUE;

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
	Corona	*Object;

	// ensure valid data
	assert( Instance != NULL );
	assert( Xf != NULL );

	// get object data
	Object = (Corona *)Instance;

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
	return ( JE_OBJECT_XFORM_TRANSLATE | JE_OBJECT_XFORM_ROTATE );

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
	HWND	Parent )
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

	// locals
	Corona	*Object;

	// ensure valid data
	assert( Instance != NULL );

	// do nothing if no time has elapsed
	assert( TimeDelta >= 0.0f );
	if ( TimeDelta == 0.0f )
	{
		return JE_TRUE;
	}

	// get object
	Object = (Corona *)Instance;
	assert( Object != NULL );

	// set new radius
	if ( Object->Visible )
	{

		// locals
		float	DesiredRadius;

		// determine desired radius
		if ( Object->DistanceToCorona >= Object->MaxRadiusDistance )
		{
			DesiredRadius = Object->MaxRadius;
		}
		else if	( Object->DistanceToCorona <= Object->MinRadiusDistance )
		{
			DesiredRadius = Object->MinRadius;
		}
		else
		{

			// locals
			jeFloat	Slope;

			// determine radius
			Slope = ( Object->MaxRadius - Object->MinRadius ) / ( Object->MaxRadiusDistance - Object->MinRadiusDistance );
			DesiredRadius = Object->MinRadius + Slope * ( Object->DistanceToCorona - Object->MinRadiusDistance );
		}

		// scale radius upwards
		if ( Object->FadeTime > 0.0f )
		{
			Object->LastVisibleRadius += ( ( TimeDelta * Object->MaxRadius ) / Object->FadeTime );
			if ( Object->LastVisibleRadius > DesiredRadius )
			{
				Object->LastVisibleRadius = DesiredRadius;
			}
		}
		else
		{
			Object->LastVisibleRadius = DesiredRadius;
		}
	}
	else if ( Object->LastVisibleRadius > 0.0f )
	{

		// scale radius down
		if ( Object->FadeTime > 0.0f )
		{
			Object->LastVisibleRadius -= ( ( TimeDelta * Object->MaxRadius ) / Object->FadeTime );
			if ( Object->LastVisibleRadius < 0.0f )
			{
				Object->LastVisibleRadius = 0.0f;
			}
		}
		else
		{
			Object->LastVisibleRadius = 0.0f;
		}
	}

	// all done
	return JE_TRUE;

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

	// locals
	Corona	*Object;

	// ensure valid data
	assert( Instance != NULL );
	assert( Data != NULL );

	// get object
	Object = (Corona *)Instance;
	assert( Object != NULL );

	// process message
	switch ( Msg )
	{

		// unsupported message
		default:
		{
			//return JE_FALSE;
			break;
		}
	}

	// all done
	return JE_FALSE;

	// Eliminate warnings
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
	jeObject* newCorObj = NULL;
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
		JE_VFILE_TYPE_MEMORY|JE_VFILE_TYPE_VIRTUAL,
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

	newCorObj = CreateFromFile(ramfile, ptrMgr);
	if (!newCorObj) {
		jeErrorLog_AddString(JE_ERR_SUBSYSTEM_FAILURE, "Unable to reade the object back from a temp VFile Memory File", NULL);
		jeVFile_Close(ramfile);
		jeVFile_Close(ramdisk);
		jeRam_Free(vfsmemctx.Data);
		return NULL;
	}

	jeVFile_Close(ramfile);
	jeVFile_Close(ramdisk);

	jeRam_Free(vfsmemctx.Data);

	return( newCorObj );
}
//---

// Icestorm
jeBoolean	JETCC ChangeBoxCollision(const void *Instance,const jeVec3d *Pos, const jeExtBox *FrontBox, const jeExtBox *BackBox, jeExtBox *ImpactBox, jePlane *Plane)
{
	return( JE_FALSE );Plane;ImpactBox;BackBox;FrontBox;Pos;Instance;
}