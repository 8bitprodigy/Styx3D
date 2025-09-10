/****************************************************************************************/
/*  MODELOBJECT.C                                                                       */
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
#ifdef WIN32
#include "windows.h"
#endif

#ifdef BUILD_BE
#include <OS.h>
#endif

#include <memory.h>
#include <assert.h>
#include <string.h>

#include "ModelObject.h"

#include "jeTypes.h"
#include "jeProperty.h"
#include "jeModel.h"
#include "jeBrush.h"
#include "Jet.h"
#include "Ram.h"
#include "Bitmap.h"
#include "VFile.h"
#include "ModelInstance.h"
#include "Resource.h"
#include "Errorlog.h"

char *NameList[3];
#ifdef WIN32
HINSTANCE ghInstance;
#endif

#ifdef BUILD_BE
#include <Resources.h>
image_id ghInstance;
#endif

#define MODELOBJECT_VERSION 1


#define UTIL_MAX_RESOURCE_LENGTH	(128)
static char stringbuffer[UTIL_MAX_RESOURCE_LENGTH + 1];

#define DEFAULT_SIZE 16.0f

enum {
	MODEL_STATS_ID = PROPERTY_LOCAL_DATATYPE_START,
	MODEL_AREAS_ID,
	MODEL_VIS_PORTALS_ID,
	MODEL_PORTALS_ID,
	MODEL_SUB_FACES_ID,
	MODEL_DRAW_FACES_ID,
	MODEL_SPLITS_ID,
	MODEL_LEAFS_ID,
	MODEL_NODES_ID,
	MODEL_BRUSH_FACES_ID,
	MODEL_BRUSHES_ID,
	MODEL_VISABLE_FACES_ID,
	MODEL_STATS_END_ID
};



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
 
	// return the allocated string
	return (rcbuffer);
}//Util_LoadLibraryString

#endif

#ifdef WIN32
int Util_GetAppPath(
	char	*Buf,		// where to store path name
	int		BufSize )	// size of buf
{

	// locals
	int	Count;

	// get exe full path name
	Count = GetModuleFileName( NULL, Buf, BufSize );
	if ( Count == 0 )
	{
		return 0;
	}

	// eliminate the exe from the path name
	while ( Count >= 0 )
	{
		if ( Buf[Count] == '\\' )
		{
			break;
		}
		Buf[Count] = '\0';
		Count--;
	}

	// all done
	return Count;

} // Util_GetAppPath()
#endif

static jeBoolean BrushExtBox( jeXForm3d * pModelXF, jeBrush * pBrush, jeExtBox * pExtBox )
{
	jeBrush_Face *	pFace ;
	int				nVerts ;
	int				i ;
	jeXForm3d		XForm ;
	jeVec3d			Vert;
	const jeVec3d	*pVert;

	assert( pBrush != NULL ) ;
	assert( pExtBox != NULL ) ;

	jeXForm3d_Copy( jeBrush_GetXForm( pBrush ), &XForm ) ;
	pFace = jeBrush_GetNextFace( pBrush, NULL ) ;
	//Set pExtBox Invalid
	pExtBox->Max.X  = -1.0f;
	pExtBox->Max.Y  = -1.0f;
	pExtBox->Max.Z  = -1.0f;
	pExtBox->Min.X  = 1.0f;
	pExtBox->Min.Y  = 1.0f;
	pExtBox->Min.Z  = 1.0f;

	while( pFace != NULL )
	{
		nVerts = jeBrush_FaceGetVertCount( pFace );
		for( i=0; i<nVerts; i++ )
		{
			pVert = jeBrush_FaceGetVertByIndex( pFace, i) ;
			jeXForm3d_Transform( &XForm, pVert, &Vert ) ;
			jeXForm3d_Transform( pModelXF, &Vert, &Vert ) ;
			if( jeExtBox_IsValid( pExtBox ) )
				jeExtBox_ExtendToEnclose( pExtBox, &Vert );
			else
				jeExtBox_SetToPoint ( pExtBox, &Vert );
		}
		pFace = jeBrush_GetNextFace( pBrush, pFace ) ;
	}  
	return( JE_TRUE );
}// BrushExtBox


#ifdef WIN32
void Init_Class( HINSTANCE hInstance )
#endif
#ifdef BUILD_BE
void Init_Class( image_id hInstance )
#endif
{
	ghInstance = hInstance;
}



void * JETCC CreateInstance(void)
{
	ModelInstance * pModelInstance;
	
	pModelInstance = JE_RAM_ALLOCATE_STRUCT( ModelInstance );
	if( pModelInstance == NULL )
		return( NULL );

	pModelInstance->pModel = jeModel_Create();
	pModelInstance->RefCnt = 1;
	if( pModelInstance->pModel == NULL )
	{
		jeRam_Free( pModelInstance );
		return( NULL );
	}
		
	return( pModelInstance );

}


void JETCC CreateRef(void * Instance)
{
	ModelInstance *pModelInstance = (ModelInstance*)Instance;

	assert( Instance );

	pModelInstance->RefCnt++;
}

jeBoolean JETCC Destroy(void **pInstance)
{
	ModelInstance **hModelInstance = (ModelInstance**)pInstance;
	ModelInstance *pModelInstance = *hModelInstance;

	assert( pInstance );
	assert( pModelInstance->RefCnt > 0 );

	pModelInstance->RefCnt--;
	if( pModelInstance->RefCnt == 0 )
	{
		jeModel_Destroy( &pModelInstance->pModel );
		jeRam_Free( pModelInstance );
	}
	else
		return( JE_FALSE );
	return( JE_TRUE );
}


jeBoolean JETCC Render(const void * Instance, const jeWorld * pWorld, const jeEngine *Engine, const jeCamera *Camera, const jeFrustum *CameraSpaceFrustum, jeObject_RenderFlags RenderFlags)
{
	ModelInstance		*pModelInstance = (ModelInstance*)Instance;
	
	if (!jeModel_Render(pModelInstance->pModel, (jeCamera*)Camera, (jeFrustum*)CameraSpaceFrustum))
		return JE_FALSE;

	return JE_TRUE;
}

jeBoolean	JETCC AttachWorld( void * Instance, jeWorld * pWorld )
{
	ModelInstance		*pModelInstance = (ModelInstance*)Instance;
	jeFaceInfo_Array	*FArray;
	jeMaterial_Array	*MArray;
	jeChain				*LChain;
	jeChain				*DLChain;

	assert( Instance );
	assert( pWorld );
	
	FArray = jeWorld_GetFaceInfoArray(pWorld);
	assert(FArray);

	MArray = jeWorld_GetMaterialArray(pWorld);
	assert(MArray);

	LChain = jeWorld_GetLightChain(pWorld);
	assert(LChain);

	DLChain = jeWorld_GetDLightChain(pWorld);
	assert(DLChain);

   	// Krouer - distribute the World pointer to the model
   	jeModel_SetWorld(pModelInstance->pModel, pWorld);
	jeModel_SetArrays(pModelInstance->pModel, FArray, MArray, LChain, DLChain);

	return JE_TRUE;
}

jeBoolean	JETCC DettachWorld( void * Instance, jeWorld * pWorld )
{
	ModelInstance *pModelInstance = (ModelInstance*)Instance;

	assert( Instance );
	assert( pWorld );

   	// Krouer - distribute the World NULL pointer to the model
   	jeModel_SetWorld(pModelInstance->pModel, NULL);
	return JE_TRUE;
}
				
jeBoolean	JETCC AttachEngine ( void * Instance, jeEngine *Engine )
{
	ModelInstance *pModelInstance = (ModelInstance*)Instance;

	assert( Instance );
	assert( Engine);

	return jeModel_SetEngine(pModelInstance->pModel, Engine);
}

jeBoolean	JETCC DettachEngine( void * Instance, jeEngine *Engine )
{
	ModelInstance *pModelInstance = (ModelInstance*)Instance;

	assert( Instance );
	assert( Engine);

	jeModel_SetEngine(pModelInstance->pModel, NULL);

	return( JE_TRUE );
}

jeBoolean	JETCC AttachSoundSystem( void * Instance, jeSound_System *SoundSystem )
{
	return( JE_TRUE );
	Instance;
	SoundSystem;
}

jeBoolean	JETCC DettachSoundSystem( void * Instance, jeSound_System *SoundSystem )
{
	return( JE_TRUE );
	Instance;
	SoundSystem;
}

jeBoolean	JETCC Collision(const void * Instance, const jeExtBox *Box, const jeVec3d *Front, const jeVec3d *Back, jeVec3d *Impact, jePlane *Plane)
{	
	ModelInstance		*pModelInstance = (ModelInstance*)Instance;

	assert( Instance );
		
	// Incarnadine
	return( jeModel_Collision( 	pModelInstance->pModel, Box, Front, Back, Impact, Plane) );		
}

jeBoolean JETCC GetExtBox(const void * Instance, jeExtBox *BBox)
{
	ModelInstance *pModelInstance = (ModelInstance*)Instance;
	jeBrush		*pBrush;
	jeExtBox	ExtBox;
	jeXForm3d	ModelXF;

	
	GetXForm( Instance, &ModelXF );
	pBrush = jeModel_GetNextBrush( pModelInstance->pModel, NULL );
	if( pBrush == NULL )
		return( JE_FALSE );
	BrushExtBox( &ModelXF, pBrush, BBox );
	while( pBrush )
	{
		BrushExtBox( &ModelXF, pBrush, &ExtBox );
		if( jeExtBox_IsValid( &ExtBox ) )
		{
			if( jeExtBox_IsValid( BBox ) )
				jeExtBox_Union ( BBox, &ExtBox, BBox );
			else
				*BBox = ExtBox;
		}
		pBrush = jeModel_GetNextBrush( pModelInstance->pModel, pBrush );
	}
	return( JE_TRUE );
}


void *	JETCC CreateFromFile(jeVFile * File, jePtrMgr *PtrMgr)
{
	ModelInstance * pModelInstance;
	BYTE Version;
	uint32 Tag;
	

	pModelInstance = JE_RAM_ALLOCATE_STRUCT( ModelInstance );
	if( pModelInstance == NULL )
		return( NULL );

	if(!jeVFile_Read(File, &Tag, sizeof(Tag)))
	{
		jeErrorLog_Add( JE_ERR_FILEIO_READ, "ModelObject_CreateFromFile:Tag" );
		goto CFF_ERROR;
	}

	if (Tag == FILE_UNIQUE_ID)
	{
		if (!jeVFile_Read(File, &Version, sizeof(Version)))
		{
    		jeErrorLog_Add( JE_ERR_FILEIO_READ, "ModelObject_CreateFromFile:Version" );
	       	goto CFF_ERROR;
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
    	pModelInstance->pModel = jeModel_CreateFromFile( File, PtrMgr);
	    if( pModelInstance->pModel == NULL )
		{
			jeErrorLog_Add( JE_ERR_FILEIO_READ, "jeModel_CreateFromFile" );
		    goto CFF_ERROR;
		}
	}
    
	pModelInstance->RefCnt = 1;
	OutputDebugString("END: ModelObject_CreateFromFile()\n");

	return( pModelInstance );

CFF_ERROR:
	OutputDebugString("ERROR: ModelObject_CreateFromFile()\n");

	jeRam_Free( pModelInstance );
	return( NULL );
}


jeBoolean	JETCC WriteToFile(const void * Instance,jeVFile * File, jePtrMgr *PtrMgr)
{
	ModelInstance *pModelInstance = (ModelInstance*)Instance;
	BYTE Version = MODELOBJECT_VERSION;
	uint32 Tag = FILE_UNIQUE_ID;

	assert( Instance );


	if (!jeVFile_Write( File, &Tag, sizeof(Tag)))
	{
		jeErrorLog_Add( JE_ERR_FILEIO_WRITE, "ModelObject_WriteToFile:Tag" );
	    return( JE_FALSE );
	}

	if (!jeVFile_Write( File, &Version, sizeof(Version)))
	{
    	jeErrorLog_Add( JE_ERR_FILEIO_WRITE, "ModelObject_WriteToFile:VersionString" );
	    return( JE_FALSE );
	}

	if( !jeModel_WriteToFile(pModelInstance->pModel, File, PtrMgr ) )
	{
		jeErrorLog_Add( JE_ERR_FILEIO_WRITE, "jeModel_WriteToFile" );
		return( JE_FALSE );
	}

	return( JE_TRUE );
	PtrMgr;
}


jeBoolean	JETCC GetPropertyList(void * Instance, jeProperty_List **List)
{
	ModelInstance *pModelInstance = (ModelInstance*)Instance;
	char * Name = NULL;
	jeProperty_List *PropertyList;
	jeProperty		 Property;
	const		jeBSP_DebugInfo * DebugInfo;

	assert( Instance );
	*List = NULL;

	DebugInfo = jeModel_GetBSPDebugInfo( pModelInstance->pModel );
	PropertyList = jeProperty_ListCreate( 0 );
	if( PropertyList == NULL )
	{
		jeErrorLog_Add( JE_ERR_SUBSYSTEM_FAILURE, "GetPropertyList:jeProperty_FillGroup");
		return( JE_FALSE );
	}

	Name = Util_LoadLibraryString( ghInstance, IDS_STATS );
	if( Name == NULL )
	{
		jeErrorLog_Add( JE_ERR_MEMORY_RESOURCE, "GetPropertyList:Util_LoadLibraryString");
		goto fail;
	}
	if( !jeProperty_FillGroup( &Property, Name, MODEL_STATS_ID ) )
	{
		jeErrorLog_Add( JE_ERR_SUBSYSTEM_FAILURE, "GetPropertyList:jeProperty_FillGroup");
		goto fail;
	}
	jeProperty_Append( PropertyList, &Property );
	jeRam_Free( Name );


	Name = Util_LoadLibraryString( ghInstance, IDS_AREAS );
	if( Name == NULL )
	{
		jeErrorLog_Add( JE_ERR_MEMORY_RESOURCE, "GetPropertyList:Util_LoadLibraryString");
		goto fail;
	}
	if( !jeProperty_FillStaticInt( &Property, Name, DebugInfo->NumAreas, MODEL_AREAS_ID ) )
	{
		jeErrorLog_Add( JE_ERR_SUBSYSTEM_FAILURE, "GetPropertyList:jeProperty_FillStaticInt");
		goto fail;
	}
	jeProperty_SetDisabled( &Property, JE_TRUE );
	jeProperty_Append( PropertyList, &Property );
	jeRam_Free( Name );

	Name = Util_LoadLibraryString( ghInstance, IDS_VIS_PORTALS );
	if( Name == NULL )
	{
		jeErrorLog_Add( JE_ERR_MEMORY_RESOURCE, "GetPropertyList:Util_LoadLibraryString");
		goto fail;
	}
	if( !jeProperty_FillStaticInt( &Property, Name, DebugInfo->NumVisPortals, MODEL_VIS_PORTALS_ID  ) )
	{
		jeErrorLog_Add( JE_ERR_SUBSYSTEM_FAILURE, "GetPropertyList:jeProperty_FillStaticInt");
		goto fail;
	}
	jeProperty_SetDisabled( &Property, JE_TRUE );
	jeProperty_Append( PropertyList, &Property );
	jeRam_Free( Name );

	Name = Util_LoadLibraryString( ghInstance, IDS_PORTALS );
	if( Name == NULL )
	{
		jeErrorLog_Add( JE_ERR_MEMORY_RESOURCE, "GetPropertyList:Util_LoadLibraryString");
		goto fail;
	}
	if( !jeProperty_FillStaticInt( &Property, Name, DebugInfo->NumPortals, MODEL_PORTALS_ID  ) )
	{
		jeErrorLog_Add( JE_ERR_SUBSYSTEM_FAILURE, "GetPropertyList:jeProperty_FillStaticInt");
		goto fail;
	}
	jeProperty_SetDisabled( &Property, JE_TRUE );
	jeProperty_Append( PropertyList, &Property );
	jeRam_Free( Name );

	Name = Util_LoadLibraryString( ghInstance, IDS_SUB_FACES );
	if( Name == NULL )
	{
		jeErrorLog_Add( JE_ERR_MEMORY_RESOURCE, "GetPropertyList:Util_LoadLibraryString");
		goto fail;
	}
	if( !jeProperty_FillStaticInt( &Property, Name, DebugInfo->NumSubdividedDrawFaces, MODEL_SUB_FACES_ID  ) )
	{
		jeErrorLog_Add( JE_ERR_SUBSYSTEM_FAILURE, "GetPropertyList:jeProperty_FillStaticInt");
		goto fail;
	}
	jeProperty_SetDisabled( &Property, JE_TRUE );
	jeProperty_Append( PropertyList, &Property );
	jeRam_Free( Name );

	Name = Util_LoadLibraryString( ghInstance, IDS_DRAW_FACES );
	if( Name == NULL )
	{
		jeErrorLog_Add( JE_ERR_MEMORY_RESOURCE, "GetPropertyList:Util_LoadLibraryString");
		goto fail;
	}
	if( !jeProperty_FillStaticInt( &Property, Name, DebugInfo->NumDrawFaces, MODEL_DRAW_FACES_ID  ) )
	{
		jeErrorLog_Add( JE_ERR_SUBSYSTEM_FAILURE, "GetPropertyList:jeProperty_FillStaticInt");
		goto fail;
	}
	jeProperty_SetDisabled( &Property, JE_TRUE );
	jeProperty_Append( PropertyList, &Property );
	jeRam_Free( Name );

	Name = Util_LoadLibraryString( ghInstance, IDS_SPLITS );
	if( Name == NULL )
	{
		jeErrorLog_Add( JE_ERR_MEMORY_RESOURCE, "GetPropertyList:Util_LoadLibraryString");
		goto fail;
	}
	if( !jeProperty_FillStaticInt( &Property, Name, DebugInfo->NumSplits, MODEL_SPLITS_ID  ) )
	{
		jeErrorLog_Add( JE_ERR_SUBSYSTEM_FAILURE, "GetPropertyList:jeProperty_FillStaticInt");
		goto fail;
	}
	jeProperty_SetDisabled( &Property, JE_TRUE );
	jeProperty_Append( PropertyList, &Property );
	jeRam_Free( Name );

	Name = Util_LoadLibraryString( ghInstance, IDS_LEAFS );
	if( Name == NULL )
	{
		jeErrorLog_Add( JE_ERR_MEMORY_RESOURCE, "GetPropertyList:Util_LoadLibraryString");
		goto fail;
	}
	if( !jeProperty_FillStaticInt( &Property, Name, DebugInfo->NumLeafs, MODEL_LEAFS_ID  ) )
	{
		jeErrorLog_Add( JE_ERR_SUBSYSTEM_FAILURE, "GetPropertyList:jeProperty_FillStaticInt");
		goto fail;
	}
	jeProperty_SetDisabled( &Property, JE_TRUE );
	jeProperty_Append( PropertyList, &Property );
	jeRam_Free( Name );

	Name = Util_LoadLibraryString( ghInstance, IDS_NODES );
	if( Name == NULL )
	{
		jeErrorLog_Add( JE_ERR_MEMORY_RESOURCE, "GetPropertyList:Util_LoadLibraryString");
		goto fail;
	}
	if( !jeProperty_FillStaticInt( &Property, Name, DebugInfo->NumNodes, MODEL_NODES_ID  ) )
	{
		jeErrorLog_Add( JE_ERR_SUBSYSTEM_FAILURE, "GetPropertyList:jeProperty_FillStaticInt");
		goto fail;
	}
	jeProperty_SetDisabled( &Property, JE_TRUE );
	jeProperty_Append( PropertyList, &Property );
	jeRam_Free( Name );

	Name = Util_LoadLibraryString( ghInstance, IDS_BRUSH_FACES );
	if( Name == NULL )
	{
		jeErrorLog_Add( JE_ERR_MEMORY_RESOURCE, "GetPropertyList:Util_LoadLibraryString");
		goto fail;
	}
	if( !jeProperty_FillStaticInt( &Property, Name, DebugInfo->NumTotalBrushFaces, MODEL_BRUSH_FACES_ID  ) )
	{
		jeErrorLog_Add( JE_ERR_SUBSYSTEM_FAILURE, "GetPropertyList:jeProperty_FillStaticInt");
		goto fail;
	}
	jeProperty_SetDisabled( &Property, JE_TRUE );
	jeProperty_Append( PropertyList, &Property );
	jeRam_Free( Name );

	Name = Util_LoadLibraryString( ghInstance, IDS_BRUSHES );
	if( Name == NULL )
	{
		jeErrorLog_Add( JE_ERR_MEMORY_RESOURCE, "GetPropertyList:Util_LoadLibraryString");
		goto fail;
	}
	if( !jeProperty_FillStaticInt( &Property, Name, DebugInfo->NumBrushes, MODEL_BRUSHES_ID  ) )
	{
		jeErrorLog_Add( JE_ERR_SUBSYSTEM_FAILURE, "GetPropertyList:jeProperty_FillStaticInt");
		goto fail;
	}
	jeProperty_SetDisabled( &Property, JE_TRUE );
	jeProperty_Append( PropertyList, &Property );
	jeRam_Free( Name );

	Name = Util_LoadLibraryString( ghInstance, IDS_VISABLE_FACES );
	if( Name == NULL )
	{
		jeErrorLog_Add( JE_ERR_MEMORY_RESOURCE, "GetPropertyList:Util_LoadLibraryString");
		goto fail;
	}
	if( !jeProperty_FillStaticInt( &Property, Name, DebugInfo->NumVisibleBrushFaces, MODEL_VISABLE_FACES_ID  ) )
	{
		jeErrorLog_Add( JE_ERR_SUBSYSTEM_FAILURE, "GetPropertyList:jeProperty_FillStaticInt");
		goto fail;
	}
	jeProperty_SetDisabled( &Property, JE_TRUE );
	jeProperty_Append( PropertyList, &Property );
	jeRam_Free( Name );

	if( !jeProperty_FillGroupEnd( &Property, MODEL_STATS_END_ID  ) )
	{
		jeErrorLog_Add( JE_ERR_SUBSYSTEM_FAILURE, "GetPropertyList:jeProperty_FillGroupEnd");
		goto fail;
	}
	jeProperty_Append( PropertyList, &Property );

	*List = PropertyList;
	return( JE_TRUE );
fail:
	if( Name != NULL )
		jeRam_Free( Name );
	jeProperty_ListDestroy( &PropertyList );
	*List = NULL;
	return( JE_FALSE );
}

jeBoolean	JETCC SetProperty( void * Instance, int32 FieldID, PROPERTY_FIELD_TYPE DataType, jeProperty_Data * pData )
{

	assert( Instance );
	return( JE_TRUE );
}

jeBoolean	JETCC SetXForm(void * Instance,const jeXForm3d *XF)
{
	ModelInstance *pModelInstance = (ModelInstance*)Instance;

	assert( Instance );

	return( jeModel_SetXForm( pModelInstance->pModel, XF ) );
}

jeBoolean JETCC GetXForm(const void * Instance,jeXForm3d *XF)
{
	ModelInstance *pModelInstance = (ModelInstance*)Instance;

	assert( Instance );
	jeXForm3d_Copy( jeModel_GetXForm(pModelInstance->pModel ), XF  );
	return( JE_TRUE );
}

int	JETCC GetXFormModFlags( const void * Instance )
{
	Instance;
	return( JE_OBJECT_XFORM_TRANSLATE | JE_OBJECT_XFORM_ROTATE );
}

jeBoolean JETCC GetChildren(const void * Instance,jeObject * Children,int MaxNumChildren)
{
	return( JE_TRUE );
}

jeBoolean JETCC AddChild(void * Instance,const jeObject * Child)
{
	ModelInstance *pModelInstance = (ModelInstance*)Instance;

	assert( Instance );

	return jeModel_AddObject(pModelInstance->pModel,(jeObject*) Child);
}

jeBoolean JETCC RemoveChild(void * Instance,const jeObject * Child)
{
	ModelInstance *pModelInstance = (ModelInstance*)Instance;

	assert( Instance );

	return jeModel_RemoveObject(pModelInstance->pModel,(jeObject*) Child);
}

#ifdef WIN32
jeBoolean JETCC EditDialog (void * Instance,HWND Parent)
#endif
#ifdef BUILD_BE
jeBoolean JETCC EditDialog (void * Instance, class G3DView *Parent)
#endif
{
	return( JE_TRUE );
}

jeBoolean JETCC SendModelMessage(void *Instance, int32 Msg, void *Data)
{
	ModelInstance *pModelInstance = (ModelInstance*)Instance;

	assert(Instance);
	assert(pModelInstance->pModel);

	switch (Msg)
	{
		// Incarnadine Begin	
	    case JE_OBJECT_MSG_WORLD_REBUILDBSP:  
		{
			jeBSPSetup *BSPSetup;

			assert(Data);
	
			BSPSetup = (jeBSPSetup*)Data;
			return jeModel_RebuildBSP(pModelInstance->pModel, BSPSetup->Options, BSPSetup->Logic, BSPSetup->LogicBalance);					
		} 
		case JE_OBJECT_MSG_WORLD_REBUILDLIGHTS:
		{
			return jeModel_RebuildLights(pModelInstance->pModel);			
		}
		// Incarnadine End
		case JE_OBJECT_MSG_WORLD_ADD_SLIGHT_UPDATE:
		case JE_OBJECT_MSG_WORLD_REMOVE_SLIGHT_UPDATE:
		{
			jeVec3d		Pos;
			jeFloat		Radius;

			assert(Data);

			if (!jeLight_GetAttributes((jeLight*)Data, &Pos, NULL, &Radius, NULL, NULL))
				return JE_FALSE;

			if (!jeModel_RebuildLightsFromPoint(pModelInstance->pModel, &Pos, Radius))
				return JE_FALSE;

			break;
		}
	}

	return JE_TRUE;
}

//Icestorm
jeBoolean	JETCC ChangeBoxCollision(const void *Instance,const jeVec3d *Pos, const jeExtBox *FrontBox, const jeExtBox *BackBox, jeExtBox *ImpactBox, jePlane *Plane)
{	
	ModelInstance		*pModelInstance = (ModelInstance*)Instance;

	assert( Instance );
		
	return( jeModel_ChangeBoxCollision(pModelInstance->pModel, Pos, FrontBox, BackBox, ImpactBox, Plane) );		
}