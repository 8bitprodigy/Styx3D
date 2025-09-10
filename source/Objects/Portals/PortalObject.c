/****************************************************************************************/
/*  PORTALOBJECT.C                                                                      */
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
#include <Resources.h>
#include <image.h>
#endif

#include <string.h>
#include <memory.h>
#include <assert.h>

#include "PortalObject.h"
#include "jeTypes.h"
#include "jeProperty.h"
#include "jeUserPoly.h"
#include "Jet.h"
#include "Camera.h"
#include "Ram.h"
#include "Bitmap.h"
#include "VFile.h"
#include "Errorlog.h"
//#include "resource.h"
#include "EditMsg.h"


#define PORTALOBJECT_VERSION 1


typedef struct 
{
	const jePlane		*Plane;
	const jeXForm3d		*FaceXForm;
	jeWorld				*World;
	jeCamera			*Camera;
	jeFrustum			*Frustum;
} PortalMsgData;

enum {
	PORTAL_SKYBOX_CHECK_ID = PROPERTY_LOCAL_DATATYPE_START,
	PORTAL_SPEED_ID,
	PORTAL_RADIOX_ID,
	PORTAL_RADIOY_ID,
	PORTAL_RADIOZ_ID,
	PORTAL_FOV_ID,  // Jeff: For FOV property
	PORTAL_LAST_ID
};

enum {
	PORTAL_SKYBOX_CHECK_INDEX,
	PORTAL_SPEED_INDEX,
	PORTAL_RADIOX_INDEX,
	PORTAL_RADIOY_INDEX,
	PORTAL_RADIOZ_INDEX,
	PORTAL_FOV_INDEX,     // Jeff: For FOV property
	PORTAL_LAST_INDEX
};

jeBrush *	Brush;

typedef struct PortalObj {

	jeFloat				FOV;

	jePortal		*Portal;

	jeBoolean		SkyBox;
	jeFloat			RotateSpeed;
	jeFloat			Rotation;
	int32			RAxis;

	int					RefCnt;

	jeBoolean		RenderNextFlag;
} PortalObj;


jeProperty PortalProperties[PORTAL_LAST_INDEX];
jeProperty_List PortalPropertyList = { PORTAL_LAST_INDEX, &PortalProperties[0] };

char *NameList[3];

#define UTIL_MAX_RESOURCE_LENGTH	(128)
static char stringbuffer[UTIL_MAX_RESOURCE_LENGTH + 1];

#define DEFAULT_SIZE 16.0f

static jeBoolean BrushExtBox( jeBrush * pBrush, jeExtBox * pExtBox )
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
			if( jeExtBox_IsValid( pExtBox ) )
				jeExtBox_ExtendToEnclose( pExtBox, &Vert );
			else
				jeExtBox_SetToPoint ( pExtBox, &Vert );
		}
		pFace = jeBrush_GetNextFace( pBrush, pFace ) ;
	}  
	return( JE_TRUE );
}// BrushExtBox

static jeBoolean Portal_CreateFace( jeBrush * Brush, jeVec3d *Verts, int32 nVerts, jeFaceInfo * pFaceInfo)
{
	jeBrush_Face *Face;
	int i;

	assert( Brush );
	assert( Verts );

	Face = jeBrush_CreateFace(Brush, nVerts);
	if( Face == NULL )
	{
		jeErrorLog_Add( JE_ERR_INTERNAL_RESOURCE, "Unable to create brush face." );
		return(JE_FALSE );
	}
	for( i = 0; i < nVerts ; i++)
		jeBrush_FaceSetVertByIndex(Face, i, &Verts[i] );
	jeBrush_FaceSetFaceInfo(Face, pFaceInfo);
	return(JE_TRUE );
}

jeBoolean CreateGlobalBrush (int BoxSize  )
{
	//revisit for error handling when merged
	jeVec3d		Verts[16];
	jeVec3d		FaceVerts[4];
	jeFaceInfo  FaceInfo;


	jeFaceInfo_SetDefaults( &FaceInfo );
	Brush = jeBrush_Create(11);
	if(Brush == NULL )
	{
		jeErrorLog_Add( JE_ERR_INTERNAL_RESOURCE, "Unable to create jeBrush" );
		return( JE_FALSE );
	}

	// Vertices 0 to 3 are the 4 corners of the top face
	jeVec3d_Set (&Verts[0], (float)-(BoxSize/2), (float)(BoxSize/2), (float)-(BoxSize*0.38f));
	jeVec3d_Set (&Verts[1], (float)-(BoxSize/2), (float)(BoxSize/2), (float)(BoxSize*0.75f));
	jeVec3d_Set (&Verts[2], (float)(BoxSize/2), (float)(BoxSize/2), (float)(BoxSize*0.75f));
	jeVec3d_Set (&Verts[3], (float)(BoxSize/2), (float)(BoxSize/2), (float)-(BoxSize*0.38f));

	// Vertices 4 to 7 are the 4 corners of the bottom face
	jeVec3d_Set (&Verts[4], (float)-(BoxSize/2), (float)-(BoxSize/2), (float)-(BoxSize*0.38f));
	jeVec3d_Set (&Verts[5], (float)(BoxSize/2), (float)-(BoxSize/2), (float)-(BoxSize*0.38f));
	jeVec3d_Set (&Verts[6], (float)(BoxSize/2), (float)-(BoxSize/2), (float)(BoxSize*0.75f));
	jeVec3d_Set (&Verts[7], (float)-(BoxSize/2), (float)-(BoxSize/2), (float)(BoxSize*0.75f));

	// Vertices 8 to 11 are the 4 corners of the Lens bottom
	jeVec3d_Set (&Verts[8], (float)-(BoxSize/4), (float)-(BoxSize/4), (float)-(BoxSize*0.38f));
	jeVec3d_Set (&Verts[9], (float)-(BoxSize/4), (float)(BoxSize/4) , (float)-(BoxSize*0.38f));
	jeVec3d_Set (&Verts[10], (float)(BoxSize/4), (float)(BoxSize/4) , (float)-(BoxSize*0.38f));
	jeVec3d_Set (&Verts[11], (float)(BoxSize/4), (float)-(BoxSize/4), (float)-(BoxSize*0.38f));

	// Vertices 12 to 11 are the 4 corners of the Lens top
	jeVec3d_Set (&Verts[12], (float)-(BoxSize/3), (float)-(BoxSize/3), (float)-(BoxSize*0.75f));
	jeVec3d_Set (&Verts[13], (float)-(BoxSize/3), (float)(BoxSize/3) , (float)-(BoxSize*0.75f));
	jeVec3d_Set (&Verts[14], (float)(BoxSize/3) , (float)(BoxSize/3) , (float)-(BoxSize*0.75f) );
	jeVec3d_Set (&Verts[15], (float)(BoxSize/3) , (float)-(BoxSize/3), (float)-(BoxSize*0.75f) );

	FaceVerts[3]	=Verts[0];
	FaceVerts[2]	=Verts[1];
	FaceVerts[1]	=Verts[2];
	FaceVerts[0]	=Verts[3];

	if( !Portal_CreateFace( Brush, FaceVerts, 4, &FaceInfo ) )
	{
		jeErrorLog_Add( JE_ERR_INTERNAL_RESOURCE, "Trace" );
		jeBrush_Destroy( &Brush);
	}

	FaceVerts[3]	=Verts[4];
	FaceVerts[2]	=Verts[5];
	FaceVerts[1]	=Verts[6];
	FaceVerts[0]	=Verts[7];

	if( !Portal_CreateFace( Brush, FaceVerts, 4, &FaceInfo ) )
	{
		jeErrorLog_Add( JE_ERR_INTERNAL_RESOURCE, "Trace" );
		jeBrush_Destroy( &Brush);
	}

	FaceVerts[3]	=Verts[1];
	FaceVerts[2]	=Verts[7];
	FaceVerts[1]	=Verts[6];
	FaceVerts[0]	=Verts[2];

	if( !Portal_CreateFace( Brush, FaceVerts, 4, &FaceInfo ) )
	{
		jeErrorLog_Add( JE_ERR_INTERNAL_RESOURCE, "Trace" );
		jeBrush_Destroy( &Brush);
	}

	FaceVerts[3]	=Verts[0];
	FaceVerts[2]	=Verts[3];
	FaceVerts[1]	=Verts[5];
	FaceVerts[0]	=Verts[4];

	if( !Portal_CreateFace( Brush, FaceVerts, 4, &FaceInfo ) )
	{
		jeErrorLog_Add( JE_ERR_INTERNAL_RESOURCE, "Trace" );
		jeBrush_Destroy( &Brush);
	}

	FaceVerts[3]	=Verts[0];
	FaceVerts[2]	=Verts[4];
	FaceVerts[1]	=Verts[7];
	FaceVerts[0]	=Verts[1];

	if( !Portal_CreateFace( Brush, FaceVerts, 4, &FaceInfo ) )
	{
		jeErrorLog_Add( JE_ERR_INTERNAL_RESOURCE, "Trace" );
		jeBrush_Destroy( &Brush);
	}

	FaceVerts[3]	=Verts[3];
	FaceVerts[2]	=Verts[2];
	FaceVerts[1]	=Verts[6];
	FaceVerts[0]	=Verts[5];

	if( !Portal_CreateFace( Brush, FaceVerts, 4, &FaceInfo ) )
	{
		jeErrorLog_Add( JE_ERR_INTERNAL_RESOURCE, "Trace" );
		jeBrush_Destroy( &Brush);
	}



	FaceVerts[3]	=Verts[8];
	FaceVerts[2]	=Verts[9];
	FaceVerts[1]	=Verts[10];
	FaceVerts[0]	=Verts[11];

	if( !Portal_CreateFace( Brush, FaceVerts, 4, &FaceInfo ) )
	{
		jeErrorLog_Add( JE_ERR_INTERNAL_RESOURCE, "Trace" );
		jeBrush_Destroy( &Brush);
	}

	FaceVerts[3]	=Verts[12];
	FaceVerts[2]	=Verts[13];
	FaceVerts[1]	=Verts[14];
	FaceVerts[0]	=Verts[15];

	if( !Portal_CreateFace( Brush, FaceVerts, 4, &FaceInfo ) )
	{
		jeErrorLog_Add( JE_ERR_INTERNAL_RESOURCE, "Trace" );
		jeBrush_Destroy( &Brush);
	}

	FaceVerts[3]	=Verts[9];
	FaceVerts[2]	=Verts[15];
	FaceVerts[1]	=Verts[14];
	FaceVerts[0]	=Verts[13];

	if( !Portal_CreateFace( Brush, FaceVerts, 4, &FaceInfo ) )
	{
		jeErrorLog_Add( JE_ERR_INTERNAL_RESOURCE, "Trace" );
		jeBrush_Destroy( &Brush);
	}

	FaceVerts[3]	=Verts[8];
	FaceVerts[2]	=Verts[11];
	FaceVerts[1]	=Verts[13];
	FaceVerts[0]	=Verts[12];

	if( !Portal_CreateFace( Brush, FaceVerts, 4, &FaceInfo ) )
	{
		jeErrorLog_Add( JE_ERR_INTERNAL_RESOURCE, "Trace" );
		jeBrush_Destroy( &Brush);
	}

	FaceVerts[3]	=Verts[8];
	FaceVerts[2]	=Verts[12];
	FaceVerts[1]	=Verts[15];
	FaceVerts[0]	=Verts[9];

	if( !Portal_CreateFace( Brush, FaceVerts, 4, &FaceInfo ) )
	{
		jeErrorLog_Add( JE_ERR_INTERNAL_RESOURCE, "Trace" );
		jeBrush_Destroy( &Brush);
	}

	FaceVerts[3]	=Verts[11];
	FaceVerts[2]	=Verts[10];
	FaceVerts[1]	=Verts[14];
	FaceVerts[0]	=Verts[13];

	if( !Portal_CreateFace( Brush, FaceVerts, 4, &FaceInfo ) )
	{
		jeErrorLog_Add( JE_ERR_INTERNAL_RESOURCE, "Trace" );
		jeBrush_Destroy( &Brush);
	}

	return	JE_TRUE;
}

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
void Init_Class( HINSTANCE hInstance )
#endif
#ifdef BUILD_BE
void Init_Class( image_id hInstance )
#endif
{
	jeProperty *Property;

	Property = &PortalProperties[PORTAL_SKYBOX_CHECK_INDEX];
	jeProperty_FillCheck(Property, "SkyBox", 1, PORTAL_SKYBOX_CHECK_ID);

	Property = &PortalProperties[PORTAL_SPEED_INDEX];
	jeProperty_FillFloat(Property, "Speed", 1.0f, PORTAL_SPEED_ID, 0.0f, 100.0f, 1.0f);

	Property = &PortalProperties[PORTAL_RADIOX_INDEX];
	jeProperty_FillRadio(Property, "X Axis", JE_TRUE, PORTAL_RADIOX_ID);

	Property = &PortalProperties[PORTAL_RADIOY_INDEX];
	jeProperty_FillRadio(Property, "Y Axis", JE_FALSE, PORTAL_RADIOY_ID);

	Property = &PortalProperties[PORTAL_RADIOZ_INDEX];
	jeProperty_FillRadio(Property, "Z Axis", JE_FALSE, PORTAL_RADIOZ_ID);

	// Jeff:  Init FOV property
	Property = &PortalProperties[PORTAL_FOV_INDEX];
	jeProperty_FillFloat(Property, "FOV", 2.0f, PORTAL_FOV_ID, -JE_PI, JE_PI, 0.1f);

}



void * JETCC CreateInstance( void )
{
	PortalObj *pPortalObj;
	jePortal *Portal;

	Portal = jePortal_Create();

	if (!Portal)
	{
		return NULL;
	}

	pPortalObj = JE_RAM_ALLOCATE_STRUCT_CLEAR( PortalObj );
	if( pPortalObj == NULL )
	{
		jeErrorLog_Add( JE_ERR_MEMORY_RESOURCE, "CreateInstance");
		return( NULL );
	}
	pPortalObj->FOV = 2.0f;

	Portal->Recursion = 0;

	pPortalObj->Portal = Portal;
	pPortalObj->RAxis = 1;

	pPortalObj->RenderNextFlag = JE_TRUE;
	
	pPortalObj->RefCnt = 1;

	if( Brush == NULL )
	{
		if( !CreateGlobalBrush(16) )
			return(NULL);
	}

	return( pPortalObj );

}

void * JETCC DuplicateInstance(void * Instance)
{
	PortalObj *pPortalObj = (PortalObj*)Instance;
	PortalObj *pNewPortalObj;

	pNewPortalObj = (PortalObj *)CreateInstance( );
	if( pNewPortalObj == NULL )
	{
		jeErrorLog_Add( JE_ERR_SUBSYSTEM_FAILURE, "DuplicateInstance:CreateInstance");
		return( NULL );
	}
	jeXForm3d_Copy( &pPortalObj->Portal->XForm, &pNewPortalObj->Portal->XForm );
	pNewPortalObj->FOV			= pPortalObj->FOV;		
	pNewPortalObj->RefCnt		= pPortalObj->RefCnt;		

	pNewPortalObj->SkyBox		= pPortalObj->SkyBox;
	pNewPortalObj->RotateSpeed	= pPortalObj->RotateSpeed;
	pNewPortalObj->Rotation		= pPortalObj->Rotation;
	pNewPortalObj->RAxis		= pPortalObj->RAxis;

	return( pNewPortalObj );
}

void JETCC CreateRef(void * Instance)
{
	PortalObj *pPortalObj = (PortalObj*)Instance;

	assert( Instance );

	pPortalObj->RefCnt++;
}

jeBoolean JETCC Destroy(void **pInstance)
{
	PortalObj **hPortalObj = (PortalObj**)pInstance;
	PortalObj *pPortalObj = *hPortalObj;

	assert( pInstance );
	assert( pPortalObj->RefCnt > 0 );

	pPortalObj->RefCnt--;
	if( pPortalObj->RefCnt == 0 )
	{
		jePortal_Destroy(&pPortalObj->Portal);
		jeRam_Free( pPortalObj );
	}
	else
		return( JE_FALSE );
	return( JE_TRUE );
}


jeBoolean JETCC Render(const void * Instance, const jeWorld * pWorld, const jeEngine *Engine, const jeCamera *Camera, const jeFrustum *CameraSpaceFrustum, jeObject_RenderFlags RenderFlags)
{
	// TODO : implement it with Vertex/Index Section data call
	return( JE_TRUE );
}

jeBoolean	JETCC AttachWorld( void * Instance, jeWorld * pWorld )
{
	PortalObj *pPortalObj = (PortalObj*)Instance;

	assert( Instance );
	return( JE_TRUE );
}

jeBoolean	JETCC DettachWorld( void * Instance, jeWorld * pWorld )
{
	PortalObj *pPortalObj = (PortalObj*)Instance;

	assert( Instance );
	return( JE_TRUE );
}
				
jeBoolean	JETCC AttachEngine ( void * Instance, jeEngine *Engine )
{
 return( JE_TRUE );
 Engine;
 Instance;
}

jeBoolean	JETCC DettachEngine( void * Instance, jeEngine *Engine )
{
	return( JE_TRUE );
	Instance;
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

jeBoolean	JETCC Collision(const jeObject *Object, const jeExtBox *Box, const jeVec3d *Front, const jeVec3d *Back, jeVec3d *Impact, jePlane *Plane)
{
	return( JE_FALSE );
}


jeBoolean JETCC GetExtBox(const void * Instance,jeExtBox *BBox)
{
	PortalObj *pPortalObj = (PortalObj*)Instance;

	assert( Instance );

	jeBrush_SetXForm( Brush,  &pPortalObj->Portal->XForm, JE_FALSE );
	BrushExtBox( Brush, BBox );
	return( JE_TRUE );
}


void *	JETCC CreateFromFile(jeVFile * File, jePtrMgr *PtrMgr)
{
	PortalObj * pPortalObj;
	jePortal *Portal;
	BYTE Version;
	uint32 Tag;

	Portal = jePortal_Create();

	if (!Portal)
	{
		return NULL;
	}

	pPortalObj = JE_RAM_ALLOCATE_STRUCT( PortalObj );
	if( pPortalObj == NULL )
		return( NULL );

	pPortalObj->Portal = Portal;


	if(!jeVFile_Read(File, &Tag, sizeof(Tag)))
	{
		jeErrorLog_Add( JE_ERR_FILEIO_READ, "PortalObject_CreateFromFile:VersionString" );
		goto CFF_ERROR;
	}

	if (Tag == FILE_UNIQUE_ID)
	{
		if (!jeVFile_Read(File, &Version, sizeof(Version)))
		{
    		jeErrorLog_Add( JE_ERR_FILEIO_READ, "PortalObject_CreateFromFile:Version" );
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
	
	    if( !jeVFile_Read(	File, &pPortalObj->FOV, sizeof( pPortalObj->FOV) ) )
		{
		    jeErrorLog_Add( JE_ERR_FILEIO_READ, "CreateFromFile:FOV" );
		    goto CFF_ERROR;
		}

	    if (!jeVFile_Read(File, &pPortalObj->Portal->XForm, sizeof(pPortalObj->Portal->XForm)))
		{
		    jeErrorLog_Add( JE_ERR_FILEIO_READ, "CreateFromFile:XForm" );
		    goto CFF_ERROR;
		}

	    if (!jeVFile_Read(File, &pPortalObj->SkyBox, sizeof(pPortalObj->SkyBox)))
		{
		    jeErrorLog_Add( JE_ERR_FILEIO_READ, "CreateFromFile:SkyBox" );
		    goto CFF_ERROR;
		}

	}

	pPortalObj->RenderNextFlag = JE_TRUE;

	pPortalObj->RefCnt = 1;

	if( Brush == NULL )
	{
		if( !CreateGlobalBrush(16) )
			return(NULL);
	}

	return( pPortalObj );

CFF_ERROR:
	jeRam_Free( pPortalObj );
	return( NULL );
	PtrMgr;
}

jeBoolean	JETCC WriteToFile(const void * Instance,jeVFile * File, jePtrMgr *PtrMgr)
{
	PortalObj *pPortalObj = (PortalObj*)Instance;
	BYTE Version = PORTALOBJECT_VERSION;
	uint32 Tag = FILE_UNIQUE_ID;

	assert( Instance );

	if( !jeVFile_Write(	File, &Tag, sizeof(Tag)))
	{
    	jeErrorLog_Add(JE_ERR_FILEIO_WRITE, "PortalObject_WriteToFile:Tag");
	    return( JE_FALSE );
	}
	
	if( !jeVFile_Write(	File, &Version, sizeof(Version) ) )
	{
    	jeErrorLog_Add(JE_ERR_FILEIO_WRITE, "PortalObject_WriteToFile:Version");
	    return( JE_FALSE );
	}

	if( !jeVFile_Write(	File, &pPortalObj->FOV, sizeof( pPortalObj->FOV) ) )
	{
		jeErrorLog_Add( JE_ERR_FILEIO_WRITE, "CreateFromFile:FOV" );
		return( JE_FALSE );
	}

	if (!jeVFile_Write(File, &pPortalObj->Portal->XForm, sizeof(pPortalObj->Portal->XForm)))
	{
		jeErrorLog_Add( JE_ERR_FILEIO_WRITE, "CreateFromFile:XForm" );
		return( JE_FALSE );
	}

	if (!jeVFile_Write(File, &pPortalObj->SkyBox, sizeof(pPortalObj->SkyBox)))
	{
		jeErrorLog_Add( JE_ERR_FILEIO_WRITE, "CreateFromFile:SkyBox" );
		return( JE_FALSE );
	}

	return( JE_TRUE );
	PtrMgr;
}


// Jeff:  Rewrote function to update all properties - 02/09/05
jeBoolean	JETCC GetPropertyList(void * Instance, jeProperty_List **List)
{
	PortalObj		*pPortalObj;
	
	assert( Instance );
	pPortalObj = (PortalObj*)Instance;
	
	// setup property list
	PortalProperties[PORTAL_SKYBOX_CHECK_INDEX].Data.Bool = pPortalObj->SkyBox;
	PortalProperties[PORTAL_SPEED_INDEX].Data.Float = pPortalObj->RotateSpeed;
	PortalProperties[PORTAL_RADIOX_INDEX].Data.Bool = (pPortalObj->RAxis == 0);
	PortalProperties[PORTAL_RADIOY_INDEX].Data.Bool = (pPortalObj->RAxis == 1);
	PortalProperties[PORTAL_RADIOZ_INDEX].Data.Bool = (pPortalObj->RAxis == 2);
	PortalProperties[PORTAL_FOV_INDEX].Data.Float   = pPortalObj->FOV;


	// copy property list
	*List = jeProperty_ListCopy( &PortalPropertyList );
	if ( *List == NULL )
	{
		jeErrorLog_Add( JE_ERR_INTERNAL_RESOURCE, "Unable to create portal property list");
		return JE_FALSE;
	}

	return( JE_TRUE );
}

jeBoolean	JETCC SetProperty( void * Instance, int32 FieldID, PROPERTY_FIELD_TYPE DataType, jeProperty_Data * pData )
{
	PortalObj *pPortalObj = (PortalObj*)Instance;

	assert( Instance );
	switch( FieldID  )
	{
		case PORTAL_SKYBOX_CHECK_ID:
		{
			assert(DataType == PROPERTY_CHECK_TYPE);
			pPortalObj->SkyBox = pData->Bool;
			break;
		}

		case PORTAL_SPEED_ID:
		{
			assert(DataType == PROPERTY_FLOAT_TYPE);
			pPortalObj->RotateSpeed = *(float*)pData;
			break;
		}

		case PORTAL_RADIOX_ID:
		{
			assert(DataType == PROPERTY_RADIO_TYPE);
			pPortalObj->RAxis = 0;
			break;
		}

		case PORTAL_RADIOY_ID:
		{
			assert(DataType == PROPERTY_RADIO_TYPE);
			pPortalObj->RAxis = 1;
			break;
		}

		case PORTAL_RADIOZ_ID:
		{
			assert(DataType == PROPERTY_RADIO_TYPE);
			pPortalObj->RAxis = 2;
			break;
		}

		// Jeff:  Update FOV 
		case PORTAL_FOV_ID:
		{
			assert(DataType == PROPERTY_FLOAT_TYPE);
			pPortalObj->FOV = *(float*)pData;
			break;
		}
	}
	return( JE_TRUE );
}

jeBoolean	JETCC GetProperty( void * Instance, int32 FieldID, PROPERTY_FIELD_TYPE DataType, jeProperty_Data * pData )
{
	PortalObj *pPortalObj = (PortalObj*)Instance;

	assert( Instance );
	switch( FieldID  )
	{
	default:
		return( JE_FALSE );
	}
	return( JE_TRUE );
}

jeBoolean	JETCC SetXForm(void * Instance,const jeXForm3d *XF)
{
	PortalObj *pPortalObj = (PortalObj*)Instance;

	assert( Instance );
	jeXForm3d_Copy( XF, &pPortalObj->Portal->XForm );

	pPortalObj->Portal->XForm.Flags = XFORM3D_NONORTHOGONALISOK;
	jeXForm3d_Orthonormalize(&pPortalObj->Portal->XForm);
	return( JE_TRUE);
}

jeBoolean JETCC GetXForm(const void * Instance,jeXForm3d *XF)
{
	PortalObj *pPortalObj = (PortalObj*)Instance;

	assert( Instance );
	jeXForm3d_Copy( &pPortalObj->Portal->XForm, XF  );
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
	return( JE_TRUE );
}

jeBoolean JETCC RemoveChild(void * Instance,const jeObject * Child)
{
	return( JE_TRUE );
}

#ifdef WIN32
jeBoolean JETCC EditDialog (void * Instance,HWND Parent)
#endif
#ifdef BUILD_BE
jeBoolean JETCC EditDialog (void * Instance, class G3DView* Parent)
#endif
{
	return( JE_TRUE );
}
static jeBoolean RenderPortalObjectInstance2(PortalObj *pPortalObj, jePortal *Portal, const jePlane *Plane, const jeXForm3d *FaceXForm, jeWorld *World, jeCamera *Camera, jeFrustum *Frustum)
{
	jeXForm3d		XForm, InvXForm, NewXForm;
	jeBoolean		Ret;
	jeFloat			ZScale;
	jeFloat         FOV;    // Jeff: used to save and restore current camera's FOV
	jeRect          Rect;   // Jeff: used to store current camera's rect       
	

	if (Portal->Recursion > 0)
		return JE_TRUE;

	// Get Camera XForm
	jeCamera_GetXForm(Camera, &XForm);

	if (pPortalObj->SkyBox)
	{
		// Skymode
		NewXForm = XForm;

		jeVec3d_Clear(&NewXForm.Translation);
		jeXForm3d_Multiply(&Portal->XForm, &NewXForm, &NewXForm);
		
	// Jeff:  Active Skybox rotation code - 02/09/05	
	#if 1
		if (pPortalObj->Rotation)
		{
			switch (pPortalObj->RAxis)
			{
				case 0:
					jeXForm3d_RotateX(&NewXForm, pPortalObj->Rotation);
					break;

				case 1:
					jeXForm3d_RotateY(&NewXForm, pPortalObj->Rotation);
					break;

				case 2:
					jeXForm3d_RotateZ(&NewXForm, pPortalObj->Rotation);
					break;
			}
		}
	#endif

		NewXForm.Translation = Portal->XForm.Translation;

		#pragma message ("Find the correct amount to scale by to put the camera on the new frustum front plane")
		ZScale = jeCamera_GetZScale(Camera);
		jeCamera_SetZScale(Camera, ZScale*20.0f);
	}
	else
	{
		jePlane		FrontPlane;
		jeXForm3d	WorldToCameraXForm;

		//	XForm the Camera to the Dest Portal location
		// This is the equation we need to XForm the camera from the FaceXForm to the PortalXForm
		//	P' = PortalXForm*InvFaceXForm*CameraXForm
		//	What this does, is take the point, and XForm against the camera like normal, then XForm by the amount
		//	it would take to get the FaceXForm to line up with the origin, then XForm into the Portal XForm...
		jeXForm3d_GetTranspose(FaceXForm, &InvXForm);

		jeXForm3d_Multiply(&Portal->XForm, &InvXForm, &NewXForm);
		jeXForm3d_Multiply(&NewXForm, &XForm, &NewXForm);
		
		// Get the WorldToCameraXForm
		jeCamera_GetTransposeXForm(Camera, &WorldToCameraXForm);

		// Transform the FacePlane to camera space
		jePlane_Transform(Plane, &WorldToCameraXForm, &FrontPlane);

		// Add the Plane to the Frustum
		if (!jeFrustum_AddPlane(Frustum, &FrontPlane, JE_TRUE))
			return JE_FALSE;
	}

	// Put the new XForm into the camera
	jeCamera_SetXForm((jeCamera*)Camera, &NewXForm);

	// Jeff: Save current camera's FOV and Rect
	jeCamera_GetAttributes(Camera,&FOV,&Rect);

	// Jeff: Set camera's FOV 
	jeCamera_SetAttributes(Camera,pPortalObj->FOV,&Rect);


	Portal->Recursion++;

	// Render the scene from this camera
	Ret = jeWorld_Render(World, Camera, Frustum);

	Portal->Recursion--;

	// Restore Camera XForm
	jeCamera_SetXForm((jeCamera*)Camera, &XForm);

	// Jeff:  restore Camera's FOV and Rect
	jeCamera_SetAttributes(Camera,FOV,&Rect);

	if (pPortalObj->SkyBox)
		jeCamera_SetZScale(Camera, ZScale);

	return Ret;
}

jeBoolean JETCC SendMsg(void * Instance, int32 Msg, void * Data)
{
	PortalObj *pPortalObj = (PortalObj*)Instance;

	switch( Msg)
	{
		case JETEDITOR_GET_JEBRUSH:
		{
			jeBrush **hBrush = (jeBrush**)Data;

			assert(Brush);
			jeBrush_SetXForm( Brush, &pPortalObj->Portal->XForm, JE_FALSE);
			*hBrush = Brush;
			return( JE_TRUE );
		}

		case 0:
		{
			PortalMsgData		*MData;
			
			MData = (PortalMsgData*)Data;

			if (MData->Frustum) {
				return RenderPortalObjectInstance2(	pPortalObj, 
													pPortalObj->Portal, 
													MData->Plane, 
													MData->FaceXForm, 
													MData->World, 
													MData->Camera, 
													MData->Frustum);
			} else {
				// could be change to a better suited function in next future
				return RenderPortalObjectInstance2(	pPortalObj, 
													pPortalObj->Portal, 
													MData->Plane, 
													MData->FaceXForm, 
													MData->World, 
													MData->Camera, 
													MData->Frustum);
			}
		}

	}
	return( JE_FALSE );
}

jeBoolean JETCC PortalFrame( void *Instance, jeFloat Time)
{
	PortalObj *pPortalObj = (PortalObj *)Instance;

	pPortalObj->Rotation += pPortalObj->RotateSpeed*0.01f;

	return JE_TRUE;
}

// Icestorm
jeBoolean	JETCC ChangeBoxCollision(const void *Instance,const jeVec3d *Pos, const jeExtBox *FrontBox, const jeExtBox *BackBox, jeExtBox *ImpactBox, jePlane *Plane)
{
	return( JE_FALSE );
}

// Krouer
void JETCC SetRenderNextFlag(void *Instance, jeBoolean NextFlag)
{
	PortalObj *pPortalObj = (PortalObj *)Instance;
	pPortalObj->RenderNextFlag = NextFlag;
}