
#ifdef WIN32
#include <windows.h>
#endif

#include <math.h>

#include <assert.h>
#include <stdlib.h>

#include "Ram.h"
#include "Puppet.h"
#include "Body.h"
#include "Motion.h"
#include "Errorlog.h"
#include "StrBlock.h"
#include "Log.h"

#include "Actor.h"
#include "Actor._h"
#include "ActorObj.h"

#include "ActorUtil.h"
#include "ActorPropertyList.h"


#define ACTOROBJECT_VERSION 1


JETAPI jeActor *JETCC jeActor_Create();

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
	jeActor	*Actor;
	ActorObj *Object;

	// ensure valid data
	assert( Instance != NULL );
	assert( World != NULL );

	// get object
	Actor = (jeActor *)Instance;
	Object = Actor->Object;

	if (Object->World == World)
	{
		return JE_TRUE;
	}
	// save world pointer
	Object->World = World;

	// save an instance of the resource manager
	Object->ResourceMgr = jeWorld_GetResourceMgr( World );
	assert( Object->ResourceMgr != NULL );

	// build mapper name list
	if ( MaterialMapperNameList == NULL )
	{

		// locals
		int	i;

		// allocate list
		MaterialMapperNameList = (char **)jeRam_AllocateClear( sizeof ( char * ) * MaterialMapperTableSize );
		if ( MaterialMapperNameList == NULL )
		{
			jeErrorLog_Add( JE_ERR_SUBSYSTEM_FAILURE, NULL );
			return JE_FALSE;
		}

		// init list
		for ( i = 0; i < MaterialMapperTableSize; i++ )
		{
			MaterialMapperNameList[i] = MaterialMapperTable[i].Name;
		}
	}


	// build bitmap list if required
	if ( Bitmaps == NULL )
	{

		// create bitmap list
		Bitmaps = Util_CreateBitmapList( Object->ResourceMgr, "GlobalMaterials", "*.bmp" );
		if ( Bitmaps == NULL )
		{
			jeErrorLog_Add( JE_ERR_SUBSYSTEM_FAILURE, NULL );
			return JE_FALSE;
		}
	}

	// prepare actor list combo box
	if ( ActorDefList != NULL )
	{
		Util_DestroyFileList( &ActorDefList, &ActorDefListSize );
	}
	ActorDefList = Util_BuildFileList( Object->ResourceMgr, "Actors", "*.act", &ActorDefListSize );
	if ( ActorDefList == NULL )
	{
		Object->World = NULL;
		Object->ResourceMgr = NULL;
		jeErrorLog_Add( JE_ERR_SUBSYSTEM_FAILURE, NULL );
		return JE_FALSE;
	}
	jeProperty_FillCombo(	&( ActorObjPropertyList.pjeProperty[ACTOROBJ_LIST_INDEX] ),
							IDS_ACTORLIST,
							ActorDefList[0],
							ACTOROBJ_LIST_ID,
							ActorDefListSize,
							ActorDefList );

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
	jeActor *Actor;
	ActorObj *Object;

	// ensure valid data
	assert( Instance != NULL );
	assert( World != NULL );

	// get object
	Actor = (jeActor *)Instance;
	Object = Actor->Object;

	//assert( Object->World == World );

	if (Object->ResourceMgr) { // destroy our instance of the resource manager
		jeResource_MgrDestroy( &( Object->ResourceMgr ) );
	}

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
	jeActor	*Actor;
	ActorObj *Object;

	// ensure valid data
	assert( Instance != NULL );
	assert( Engine != NULL );

	// get object data
	Actor = (jeActor *)Instance;
	Object = Actor->Object;

	// save engine pointer
	Actor->Object->Engine = Engine;

	// set properties
	if ( Object->LoadedFromDisk == JE_TRUE )
	{

		// locals
		jeProperty_Data	Data;
		char *RememberMotionName;
		jeBoolean ret;

		// reset loaded from disk flag
		//Object->LoadedFromDisk = JE_FALSE;  // Krouer: move this line later

		// compensate for hack rotation that will occur
		{
			jeVec3d	Pos;
			jeVec3d_Copy( &( Actor->Xf.Translation ), &Pos );
			jeVec3d_Set( &( Actor->Xf.Translation ), 0.0f, 0.0f, 0.0f );
			jeXForm3d_RotateX( &( Actor->Xf ), JE_HALFPI );
			jeVec3d_Copy( &Pos, &( Actor->Xf.Translation ) );
		}

		RememberMotionName = Util_StrDup( Object->MotionName );

		// set actor def
		Data.String = Util_StrDup( Object->ActorDefName );
		ret = SetProperty( Actor, ACTOROBJ_LIST_ID, PROPERTY_COMBO_TYPE, &Data );
		jeRam_Free( Data.String );

		if (!ret)
		{
            // Krouer: restore the previous commented line
    	    Object->LoadedFromDisk = JE_FALSE;
			jeRam_Free(Object->ActorDefName);
			jeRam_Free(Object->MotionName);
			jeRam_Free(Object->LightReferenceBoneName);
			Object->ActorDefName = Util_StrDup( NoSelection );
			Object->MotionName = Util_StrDup( NoSelection );
			Object->LightReferenceBoneName = Util_StrDup( NoSelection );
			return JE_TRUE;
		}

		// set motion name
		Data.String = Util_StrDup( RememberMotionName );
		ret = SetProperty( Actor, ACTOROBJ_MOTIONLIST_ID, PROPERTY_COMBO_TYPE, &Data );
		jeRam_Free( Data.String );
		jeRam_Free( RememberMotionName );

		if (!ret)
		{
            // Krouer: restore the previous commented line
    	    Object->LoadedFromDisk = JE_FALSE;
			jeRam_Free(Object->MotionName);
			Object->MotionName = Util_StrDup( NoSelection );
		}

		// set light reference bone name
		Data.String = Util_StrDup( Object->LightReferenceBoneName );
		ret = SetProperty( Actor, ACTOROBJ_LIGHTREFERENCEBONENAMELIST_ID, PROPERTY_COMBO_TYPE, &Data );
		jeRam_Free( Data.String );

		if (!ret)
		{
			jeRam_Free(Object->LightReferenceBoneName);
			Object->LightReferenceBoneName = Util_StrDup( NoSelection );
		}

        // Krouer: restore the previous commented line
    	Object->LoadedFromDisk = JE_FALSE;
	}
	else
	{
		if(Actor->ActorDefinition != NULL)
			jeActor_AttachEngine( Actor, Object->Engine );		
	}


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
	jeActor *Actor;
	ActorObj* Object;

	// ensure valid data
	assert( Instance != NULL );
	assert( Engine != NULL );

	// get object data
	Actor = (jeActor *)Instance;
	Object = Actor->Object;
#ifdef _DEBUG
	if (Object->Engine) assert( Object->Engine == Engine );
#endif

	// destroy motion list
	if ( Object->MotionListSize > 0 )
	{
		ActorObj_DestroyMotionList( Actor );
	}

	// free actor def name
	if ( Object->ActorDefName != NULL )
	{
		jeRam_Free( Object->ActorDefName );
		Object->ActorDefName = NULL;
	}
		
	// zap engine pointer	
	if(Object->Engine && Actor->ActorDefinition)
	{
		jeActor_DetachEngine(Actor,Engine);
		Object->Engine = NULL;
	}


	// all done
	return JE_TRUE;
} // DettachEngine()

void JETCC CreateRef(void *Actor)
{
	jeActor_CreateRef((jeActor*)Actor);
}

////////////////////////////////////////////////////////////////////////////////////////
//
//	Render()
//
////////////////////////////////////////////////////////////////////////////////////////
jeBoolean JETCC Render(
	const void				*ActorPtr,				// object instance data
	const jeWorld			*World,					// world
	const jeEngine			*Engine,				// engine
	const jeCamera			*Camera,				// camera
	const jeFrustum			*CameraSpaceFrustum,	// frustum 	
	jeObject_RenderFlags	RenderFlags )			// render flags
{

	// locals
	//ActorObj	*Object;
	jeActor *Actor = (jeActor *)ActorPtr;
	
	jeBody		*Body;
	jeFloat		floate;
	jeCamera	*Cam2;
	ActorObj *Object;

	// ensure valid data
	assert( Actor != NULL );
	assert( World != NULL );
	assert( Engine != NULL );
	assert( Camera != NULL );
	assert( CameraSpaceFrustum != NULL );

	Object = Actor->Object;

	// peform rendering if an actor exists
	if ( jeActor_IsValid(Actor) == JE_TRUE && jeActor_DefIsValid(Actor->ActorDefinition) == JE_TRUE)
	{
		jeVec3d	Pos;
		jeVec3d Vector;
		const jeVec3d * POV;
		int lod=0;
		jeXForm3d Xf;
		jeActor_GetXForm(Actor,&Xf);
				
		//Calculate LOD
		// get actor def body
		Body = jeActor_GetBody( Actor->ActorDefinition );
		jeVec3d_Copy( &( Xf.Translation ), &Pos );
		Cam2 = (jeCamera*)Camera;
		POV = jeCamera_GetPov2(Cam2);
		jeVec3d_Copy(POV, &Vector);
		floate = jeVec3d_Length(&Vector);
		
		if(floate >=0.0f)
			lod = 0;
#if 0
		if(floate >=200.0f)
			lod = 1;
		if(floate >= 400.0f)
			lod = 2;
		if(floate >=800.0f)
			lod = 3;
		if(floate >=1600.0f)
			lod = 4;
		if(floate >=3200.0f)
			lod = 5;
		if(floate >=6400.0f)
			lod = 6;
#endif

		jeBody_ComputeLevelsOfDetail(Body, lod);
		
		
		// render the actor
		if ( RenderFlags & JE_OBJECT_RENDER_FLAG_CAMERA_FRUSTUM )
		{
			jeActor_Render( Actor, (jeEngine *)Engine, (jeWorld *)World, Camera );
		}
		else
		{
			jeActor_RenderThroughFrustum( Actor, (jeEngine *)Engine, (jeWorld *)World, (jeCamera*)Camera , CameraSpaceFrustum );
		}

		// display collision ext box
		if ( Object->CollisionExtBoxDisplay == JE_TRUE )
		{

			// locals
			JE_RGBA		Color = { 0.0f, 255.0f, 0.0f, 64.0f };
			jeExtBox	ExtBox;
			jeVec3d Translation;

			// copy ext box
			ExtBox = Object->CollisionExtBox;

			// adjust extent box
			GetBoxTranslation(&ExtBox, Actor, &Translation);
			jeExtBox_Translate( &ExtBox, Translation.X, Translation.Y, Translation.Z );
			jeExtBox_Scale( &ExtBox, Object->ScaleX, Object->ScaleZ, Object->ScaleY );

			// draw it
			Util_DrawExtBox( (jeWorld *)World, &Color, &ExtBox );
		}

		// display render ext box
		if ( Object->RenderExtBoxDisplay == JE_TRUE )
		{

			// locals
			JE_RGBA		Color = { 255.0f, 0.0f, 0.0f, 64.0f };
			jeExtBox	ExtBox;
			jeVec3d Translation;

			// copy ext box
			ExtBox = Object->RenderHintExtBox;

			// adjust extent box
			GetBoxTranslation(&ExtBox, Actor, &Translation);
			jeExtBox_Translate( &ExtBox, Translation.X, Translation.Y, Translation.Z );
			jeExtBox_Scale( &ExtBox, Object->ScaleX, Object->ScaleZ, Object->ScaleY );

			// draw it
			Util_DrawExtBox( (jeWorld *)World, &Color, &ExtBox );
		}
	}


	// all done
	return JE_TRUE;

} // Render()

////////////////////////////////////////////////////////////////////////////////////////
//
//	Collision()
//
///////////////////////////////////////////////////////////////////////////////////////
jeBoolean JETCC Collision(
	const void* ActorPtr,
	const jeExtBox	*Box,
	const jeVec3d	*Front,
	const jeVec3d	*Back,	
	jeVec3d			*Impact,
	jePlane			*Plane )
{
	jeActor* Actor = (jeActor *)ActorPtr;
	jeCollisionInfo CollisionInfo;
	ActorObj	*Object;

	// ensure valid data
	assert( Actor != NULL );
	// Box CAN be null
	assert( Front != NULL );
	assert( Back != NULL );
	//assert( Impact != NULL );  Removed by Incarnadine. Impact&Plane CAN be NULL.
	//assert( Plane != NULL );
	
	Object = Actor->Object;
	if(!Actor->ActorDefinition) return JE_FALSE;

	if(jeActor_Collision((jeActor*)Actor,Object->World,Box,Front,Back,&CollisionInfo))
	{
		if(Impact != NULL)	*Impact = CollisionInfo.Impact;
		if(Plane != NULL) *Plane = CollisionInfo.Plane;
		return JE_TRUE;
	}
	return JE_FALSE;
} // Collision()

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
	jeActor	*Actor;
	ActorObj *Object;
		
	// ensure valid data
	assert( Instance != NULL );

	// dp nothing if no time has elapsed
	if ( TimeDelta == 0.0f )
	{
		return JE_TRUE;
	}

	// get object
	Actor = (jeActor *)Instance;
	assert( Actor != NULL );
	Object = Actor->Object;

	// get actor def body
	/* MOVED TO RENDER()
	Body = jeActor_GetBody( Object->ActorDef );
	jeBody_ComputeLevelsOfDetail(Body, 1); //TODO replace 1*/

	// adjust actors motion
	if ( Object->Motion != NULL )
	{

		// locals
		jeBoolean	Result;
		float		TimeScale;
		float		MotionTime;
		float		Start, End, Total;

		// get time scale
		TimeScale = (float)fabs( Object->MotionTimeScale );

		// adjust motion time
		Object->MotionTime += ( TimeDelta * TimeScale );

		// get total motion time
		Result = jeMotion_GetTimeExtents( Object->Motion, &Start, &End );
		assert( Result == JE_TRUE );
		Total = End - Start;

		// adjust motion time if required
		if( Object->MotionTime > Total )
		{
			Object->MotionTime = (float)fmod( Object->MotionTime, Total );
		}

		// set motion time
		if ( Object->MotionTimeScale >= 0.0f )
		{
			MotionTime = Object->MotionTime;
		}
		else
		{
			MotionTime = Total - Object->MotionTime;
		}

		// set new pose
		jeActor_SetPose( Actor, Object->Motion, Object->MotionTime, &Actor->Xf );
	}

	// all done
	return JE_TRUE;

} // Frame()

jeBoolean JETCC GetExtBox(
	const void	*Instance,	// object instance data
	jeExtBox	*BBox )		// where to store extent box
{
		// locals
		jeVec3d Pos;
		ActorObj *Object;
		jeExtBox	ExtBox;
		jeActor *Actor = (jeActor*)Instance;
		Object = Actor->Object;
		

		// now update the collision boxes
		// copy ext box
		ExtBox = Object->CollisionExtBox;
		if(!jeExtBox_IsValid(&ExtBox))
		{
			// save extent box
			Pos = Actor->Xf.Translation;
			jeExtBox_Set (  BBox, 
						Pos.X - 5.0f, Pos.Y - 5.0f, Pos.Z - 5.0f,
						Pos.X + 5.0f, Pos.Y + 5.0f, Pos.Z + 5.0f );
		}
		else
		{
			jeVec3d Translation;

			// Dist from box actor pos to box center
			GetBoxTranslation(&ExtBox, Actor, &Translation);
			jeExtBox_Translate( &ExtBox, Translation.X, Translation.Y, Translation.Z );
			jeExtBox_Scale( &ExtBox, Object->ScaleX, Object->ScaleZ, Object->ScaleY );
			*BBox = ExtBox;
		}

		return JE_TRUE;
}

int	JETCC GetXFormModFlags( const void * Instance )
{
	Instance;
	return( JE_OBJECT_XFORM_TRANSLATE | JE_OBJECT_XFORM_ROTATE);
}

#ifdef WIN32
jeBoolean JETCC EditDialog (void * Instance,HWND Parent)
#endif
#ifdef BUILD_BE
jeBoolean JETCC EditDialog (void * Instance,class G3DView* Parent)
#endif
{
	return( JE_TRUE );
}

jeBoolean JETCC SendObjMessage(void * Instance, int32 Msg, void * Data)
{
	// locals
	jeActor *Actor;
	ActorObj	*Object;

	// ensure valid data
	assert( Instance != NULL );
	assert( Data != NULL );

	// get object
	Actor = (jeActor *)Instance;
	Object = Actor->Object;
	assert( Object != NULL );

	// process message
	switch ( Msg )
	{
	case 1:
/*		case JETEDITOR_GET_JEBRUSH:
		{
			jeBrush **hBrush = (jeBrush**)Data;
			if( pBrush == NULL )
			{
				if( !CreateGlobalBrush() )
					return(JE_FALSE);
			}
			UpdateGlobalBrush( Object );
			*hBrush = pBrush;
			return( JE_TRUE );
		}
		break;

		// event mesage
		case OBJECT_EVENT_MSG:
		{

			// get event data struct
			Object_EventData *ed = Data;

			// process event type
			switch ( ed->EventType )
			{

				// change motion
				case 0:
				{

					// locals
					jeProperty_Data	Data;

					// get motion name
					assert( ed->Args != NULL );
					Data.String = Util_StrDup( ed->Args );

					// apply motion change
					SetProperty( Actor, ACTOROBJ_MOTIONLIST_ID, PROPERTY_COMBO_TYPE, &Data );

					// free temporary string
					jeRam_Free( Data.String );
					break;
				}
			}
			break;
		}*/

		// unsupported message
		default:
		{
			return JE_FALSE;
			break;
		}
	}

	// all done
	return JE_FALSE;
}

void* JETCC CreateInstance(void)
{
	jeActor* A;
	A = jeActor_Create();
	A->Object = JE_RAM_ALLOCATE_STRUCT_CLEAR( ActorObj );	
	A->Object->Engine = NULL;
	InitObjectProperties(A);
	FillProperties();	
	return A;
}

JETAPI jeBoolean JETCC Destroy(void **pActor) //jeActor **pA)
{
	jeActor** pA = (jeActor **)pActor;
	jeRam_Free((*pA)->Object);
	(*pA)->Object = NULL;
	jeActor_Destroy(pA);	
	return JE_TRUE;
}

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
	jeActor *Actor;
	ActorObj		*Object;
	int				Size;
	jeBoolean		Result = JE_TRUE;
	BYTE Version;
	uint32 Tag;

	OutputDebugString("ActorObject\n");

	// ensure valid data
	assert( File != NULL );
	assert( PtrMgr != NULL );

	// create new object
	Actor = (jeActor *)CreateInstance();
	if( Actor == NULL ) return NULL;

	Object = Actor->Object;
	if ( Object == NULL )
	{
		return NULL;
	}

	//read version 
 	if(!jeVFile_Read(File, &Tag, sizeof(Tag)))
	{
		jeErrorLog_Add( JE_ERR_FILEIO_READ, "ActorObject_CreateFromFile:Tag" );
		goto ERROR_CreateFromFile;
	}

	if (Tag == FILE_UNIQUE_ID)
	{
		if (!jeVFile_Read(File, &Version, sizeof(Version)))
		{
    		jeErrorLog_Add( JE_ERR_FILEIO_READ, "ActorObject_CreateFromFile:Version" );
	       	goto ERROR_CreateFromFile;
		}
	}
	else
	{
		//for backwards compatibility with old object format
		Version = 1;
		jeVFile_Seek(File,-((int)sizeof(Tag)),JE_VFILE_SEEKCUR);
	}


	if (Version >= 1)
	{
	    // read actor def name
	    Result &= jeVFile_Read( File, &( Size ), sizeof( Size ) );
	    if ( ( Size > 0 ) && ( Result == JE_TRUE ) )
		{
		    Object->ActorDefName = (char *)jeRam_Allocate( Size );
		    if ( Object->ActorDefName == NULL )
			{
			    jeErrorLog_Add( JE_ERR_MEMORY_RESOURCE, NULL );
			    goto ERROR_CreateFromFile;
			}
		    Result &= jeVFile_Read( File, Object->ActorDefName, Size );
		}

	    // read motion name
	    Result &= jeVFile_Read( File, &( Size ), sizeof( Size ) );
	    if ( ( Size > 0 ) && ( Result == JE_TRUE ) )
		{
		    Object->MotionName = (char *)jeRam_Allocate( Size );
		    if ( Object->MotionName == NULL )
			{
			    jeErrorLog_Add( JE_ERR_MEMORY_RESOURCE, NULL );
			    goto ERROR_CreateFromFile;
			}
		    Result &= jeVFile_Read( File, Object->MotionName, Size );
		}

	    // read light reference bone name
	    Result &= jeVFile_Read( File, &( Size ), sizeof( Size ) );
	    if ( ( Size > 0 ) && ( Result == JE_TRUE ) )
		{
		    Object->LightReferenceBoneName = (char *)jeRam_Allocate( Size );
		    if ( Object->LightReferenceBoneName == NULL )
			{
			    jeErrorLog_Add( JE_ERR_MEMORY_RESOURCE, NULL );
			    goto ERROR_CreateFromFile;
			}
		    Result &= jeVFile_Read( File, Object->LightReferenceBoneName, Size );
		}

	    // read xform
	    Result &= jeVFile_Read( File, &( Actor->Xf ), sizeof( Actor->Xf ) );

	    // read scales
	    Result &= jeVFile_Read( File, &( Object->ScaleX ), sizeof( Object->ScaleX ) );
	    Result &= jeVFile_Read( File, &( Object->ScaleY ), sizeof( Object->ScaleY ) );
	    Result &= jeVFile_Read( File, &( Object->ScaleZ ), sizeof( Object->ScaleZ ) );

	    // read fill light color
	    Result &= jeVFile_Read( File, &( Object->FillLightRed ), sizeof( Object->FillLightRed ) );
	    Result &= jeVFile_Read( File, &( Object->FillLightGreen ), sizeof( Object->FillLightGreen ) );
	    Result &= jeVFile_Read( File, &( Object->FillLightBlue ), sizeof( Object->FillLightBlue ) );

	    // read ambient light color
	    Result &= jeVFile_Read( File, &( Object->AmbientLightRed ), sizeof( Object->AmbientLightRed ) );
	    Result &= jeVFile_Read( File, &( Object->AmbientLightGreen ), sizeof( Object->AmbientLightGreen ) );
	    Result &= jeVFile_Read( File, &( Object->AmbientLightBlue ), sizeof( Object->AmbientLightBlue ) );

	    // read per bone lighting flag
	    Result &= jeVFile_Read( File, &( Object->PerBoneLighting ), sizeof( Object->PerBoneLighting ) );

	    // read fill light normal
	    Result &= jeVFile_Read( File, &( Object->FillLightNormal ), sizeof( Object->FillLightNormal ) );

	    // read use fill light flag
	    Result &= jeVFile_Read( File, &( Object->UseFillLight ), sizeof( Object->UseFillLight ) );

	    // read use ambient light from floor flag
	    Result &= jeVFile_Read( File, &( Object->UseAmbientLightFromFloor ), sizeof( Object->UseAmbientLightFromFloor ) );

	    // read max dynamic lights amount
	    Result &= jeVFile_Read( File, &( Object->MaximumDynamicLightsToUse ), sizeof( Object->MaximumDynamicLightsToUse ) );

	    // read collision ext box info
	    Result &= jeVFile_Read( File, &( Object->CollisionExtBox ), sizeof( Object->CollisionExtBox ) );

	    // read draw ext box adjust info
	    Result &= jeVFile_Read( File, &( Object->RenderHintExtBox ), sizeof( Object->RenderHintExtBox ) );

	    // read fill normal actor relative flag
	    Result &= jeVFile_Read( File, &( Object->FillNormalActorRelative ), sizeof( Object->FillNormalActorRelative ) );

	    // read motion time scale
	    Result &= jeVFile_Read( File, &( Object->MotionTimeScale ), sizeof( Object->MotionTimeScale ) );
	}

	// fail if there was an error
	if ( Result == JE_FALSE )
	{
		jeErrorLog_Add( JE_ERR_FILEIO_READ, NULL );
		goto ERROR_CreateFromFile;
	}

	// all done
	Object->LoadedFromDisk = JE_TRUE;
	return Actor;

	// handle errors
	ERROR_CreateFromFile:

	// free all strings
	if ( Object->ActorDefName != NULL )
	{
		jeRam_Free( Object->ActorDefName );
	}
	if ( Object->MotionName != NULL )
	{
		jeRam_Free( Object->MotionName );
	}
	if ( Object->LightReferenceBoneName != NULL )
	{
		jeRam_Free( Object->LightReferenceBoneName );
	}

	// free object
	jeRam_Free( Object );

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
	jeActor *Actor;
	BYTE Version = ACTOROBJECT_VERSION;
	ActorObj	*Object;
	jeBoolean	Result = JE_TRUE;
	int			Size;
	uint32 Tag = FILE_UNIQUE_ID;

	// ensure valid data
	assert( Instance != NULL );
	assert( File != NULL );
	assert( PtrMgr != NULL );

	// get object data
	Actor = (jeActor *)Instance;
	Object = Actor->Object;

	if( !jeVFile_Write(	File, &Tag, sizeof(Tag)))
	{
    	jeErrorLog_Add(JE_ERR_FILEIO_WRITE, "ActorObject_WriteToFile:Tag");
	    return( JE_FALSE );
	}
	
	if( !jeVFile_Write(	File, &Version, sizeof(Version) ) )
	{
    	jeErrorLog_Add(JE_ERR_FILEIO_WRITE, "ActorObject_WriteToFile:Version");
	    return( JE_FALSE );
	}

	// write actor def name
	if ( Object->ActorDefName != NULL )
	{
		Util_WriteString( File, Object->ActorDefName );
	}
	else
	{
		Size = 0;
		Result &= jeVFile_Write( File, &Size, sizeof( Size ) );
	}

	// write motion name
	if ( Object->MotionName != NULL )
	{
		Util_WriteString( File, Object->MotionName );
	}
	else
	{
		Size = 0;
		Result &= jeVFile_Write( File, &Size, sizeof( Size ) );
	}

	// write light reference bone name
	if ( Object->LightReferenceBoneName != NULL )
	{
		Util_WriteString( File, Object->LightReferenceBoneName );
	}
	else
	{
		Size = 0;
		Result &= jeVFile_Write( File, &Size, sizeof( Size ) );
	}

	// write xform
	Result &= jeVFile_Write( File, &( Actor->Xf ), sizeof( Actor->Xf ) );

	// write scales
	Result &= jeVFile_Write( File, &( Object->ScaleX ), sizeof( Object->ScaleX ) );
	Result &= jeVFile_Write( File, &( Object->ScaleY ), sizeof( Object->ScaleY ) );
	Result &= jeVFile_Write( File, &( Object->ScaleZ ), sizeof( Object->ScaleZ ) );

	// write fill light color
	Result &= jeVFile_Write( File, &( Object->FillLightRed ), sizeof( Object->FillLightRed ) );
	Result &= jeVFile_Write( File, &( Object->FillLightGreen ), sizeof( Object->FillLightGreen ) );
	Result &= jeVFile_Write( File, &( Object->FillLightBlue ), sizeof( Object->FillLightBlue ) );

	// write ambient light color
	Result &= jeVFile_Write( File, &( Object->AmbientLightRed ), sizeof( Object->AmbientLightRed ) );
	Result &= jeVFile_Write( File, &( Object->AmbientLightGreen ), sizeof( Object->AmbientLightGreen ) );
	Result &= jeVFile_Write( File, &( Object->AmbientLightBlue ), sizeof( Object->AmbientLightBlue ) );

	// write per bone lighting flag
	Result &= jeVFile_Write( File, &( Object->PerBoneLighting ), sizeof( Object->PerBoneLighting ) );

	// write fill light normal
	Result &= jeVFile_Write( File, &( Object->FillLightNormal ), sizeof( Object->FillLightNormal ) );

	// write use fill light flag
	Result &= jeVFile_Write( File, &( Object->UseFillLight ), sizeof( Object->UseFillLight ) );

	// write use ambient light from floor flag
	Result &= jeVFile_Write( File, &( Object->UseAmbientLightFromFloor ), sizeof( Object->UseAmbientLightFromFloor ) );

	// write max dynamic lights amount
	Result &= jeVFile_Write( File, &( Object->MaximumDynamicLightsToUse ), sizeof( Object->MaximumDynamicLightsToUse ) );

	// write collision ext box adjust info
	Result &= jeVFile_Write( File, &( Object->CollisionExtBox ), sizeof( Object->CollisionExtBox ) );

	// write draw ext box adjust info
	Result &= jeVFile_Write( File, &( Object->RenderHintExtBox ), sizeof( Object->RenderHintExtBox ) );

	// write fill normal actor relative flag
	Result &= jeVFile_Write( File, &( Object->FillNormalActorRelative ), sizeof( Object->FillNormalActorRelative ) );

	// write motion time scale
	Result &= jeVFile_Write( File, &( Object->MotionTimeScale ), sizeof( Object->MotionTimeScale ) );

	// log errors
	if ( Result != JE_TRUE )
	{
		jeErrorLog_Add( JE_ERR_FILEIO_WRITE, NULL );
	}

	// all done
	return Result;

	// eliminate warnings
	PtrMgr;

} // WriteToFile()

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
	jeObject* newActor = NULL;
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
		(jeVFile_TypeIdentifier)(JE_VFILE_TYPE_MEMORY|JE_VFILE_TYPE_VIRTUAL),
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

	newActor = (jeObject *)CreateFromFile(ramfile, ptrMgr);
	if (!newActor) {
		jeErrorLog_AddString(JE_ERR_SUBSYSTEM_FAILURE, "Unable to reade the object back from a temp VFile Memory File", NULL);
		jeVFile_Close(ramfile);
		jeVFile_Close(ramdisk);
		jeRam_Free(vfsmemctx.Data);
		return NULL;
	}

	jeVFile_Close(ramfile);
	jeVFile_Close(ramdisk);

	jeRam_Free(vfsmemctx.Data);

	return( newActor );
}

void JETCC SetRenderNextTime(void * Instance, jeBoolean RenderNextTime)
{
	jeActor_SetRenderNextTime((jeActor*)Instance, RenderNextTime);
}

jeBoolean JETCC SetXForm(void *Actor, const jeXForm3d *XF)
{
	jeActor_SetXForm((jeActor*)Actor, XF);
	return JE_TRUE;
}

jeBoolean JETCC GetXForm(const void *Actor, jeXForm3d *XF)
{
	jeActor_GetXForm((jeActor*)Actor, XF);
	return JE_TRUE;
}

//#pragma warning(disable : 4028 4090)
jeObjectDef jeActor_ObjectDef =
{
	JE_OBJECT_TYPE_ACTOR,
	"Actor",
	JE_OBJECT_VISRENDER,
	CreateInstance,
	CreateRef,
	Destroy,

	AttachWorld,
	DettachWorld,
	AttachEngine,
	DettachEngine,

	NULL, // Soundsystem
	NULL, // Soundsystem
	Render,//jeActor_Render,
	Collision,//jeActor_Collision,
	GetExtBox,
	CreateFromFile,
	WriteToFile,
	GetPropertyList,
	SetProperty,
	NULL,// GetProperty
	
	SetXForm,
	GetXForm,

	GetXFormModFlags,
	NULL,//jeActor_GetChildren,
	NULL,//jeActor_AddChild,
	NULL,//jeActor_RemoveChild,
	EditDialog,
	SendObjMessage,
	Frame,
	//Royce
	DuplicateInstance,
	//---
	NULL,	// ChangeBoxCollision
	NULL,	// GetGlobalPropertyList
	NULL,	// SetGlobalProperty
	SetRenderNextTime,
};
