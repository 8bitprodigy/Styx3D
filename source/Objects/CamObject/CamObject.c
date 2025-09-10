/****************************************************************************************/
/*  CAMOBJECT.C                                                                         */
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

#include "CamObject.h"
#include "jeTypes.h"
#include "jeProperty.h"
#include "jeUserPoly.h"
#include "Jet.h"
#include "Camera.h"
#include "Ram.h"

#include "Bitmap.h"
#include "VFile.h"

#include "Errorlog.h"
#include "Resource.h"
#include "EditMsg.h"
#include "CamFieldID.h"


#define CAMOBJECT_VERSION 2


enum {
	CAMREA_FOV_INDEX,

	// BEGIN - Far clip plane editor box - paradoxnj 3/9/2005
	CAMREA_FARCLIP_INDEX,
	CAMREA_FARCLIPENABLED_INDEX,
	// END - Far clip plane editor box - paradoxnj 3/9/2005

	CAMREA_LAST_INDEX
};

jeBrush *	Brush;

typedef struct CamObj {

	jeFloat				FOV;
	jeXForm3d			XForm;

	// BEGIN - Far clip plane box - paradoxnj 3/9/2005
	jeBoolean			FarClipEnabled;
	jeFloat				FarClip;
	// END - Far clip plane box - paradoxnj 3/9/2005

	int					RefCnt;
} CamObj;


jeProperty CamProperties[CAMREA_LAST_INDEX];
jeProperty_List CamPropertyList = { CAMREA_LAST_INDEX, &CamProperties[0], JE_FALSE };

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

static jeBoolean CamObj_CreateFace( jeBrush * Brush, jeVec3d *Verts, int32 nVerts, jeFaceInfo * pFaceInfo)
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

	if( !CamObj_CreateFace( Brush, FaceVerts, 4, &FaceInfo ) )
	{
		jeErrorLog_Add( JE_ERR_INTERNAL_RESOURCE, "Trace" );
		jeBrush_Destroy( &Brush);
	}

	FaceVerts[3]	=Verts[4];
	FaceVerts[2]	=Verts[5];
	FaceVerts[1]	=Verts[6];
	FaceVerts[0]	=Verts[7];

	if( !CamObj_CreateFace( Brush, FaceVerts, 4, &FaceInfo ) )
	{
		jeErrorLog_Add( JE_ERR_INTERNAL_RESOURCE, "Trace" );
		jeBrush_Destroy( &Brush);
	}

	FaceVerts[3]	=Verts[1];
	FaceVerts[2]	=Verts[7];
	FaceVerts[1]	=Verts[6];
	FaceVerts[0]	=Verts[2];

	if( !CamObj_CreateFace( Brush, FaceVerts, 4, &FaceInfo ) )
	{
		jeErrorLog_Add( JE_ERR_INTERNAL_RESOURCE, "Trace" );
		jeBrush_Destroy( &Brush);
	}

	FaceVerts[3]	=Verts[0];
	FaceVerts[2]	=Verts[3];
	FaceVerts[1]	=Verts[5];
	FaceVerts[0]	=Verts[4];

	if( !CamObj_CreateFace( Brush, FaceVerts, 4, &FaceInfo ) )
	{
		jeErrorLog_Add( JE_ERR_INTERNAL_RESOURCE, "Trace" );
		jeBrush_Destroy( &Brush);
	}

	FaceVerts[3]	=Verts[0];
	FaceVerts[2]	=Verts[4];
	FaceVerts[1]	=Verts[7];
	FaceVerts[0]	=Verts[1];

	if( !CamObj_CreateFace( Brush, FaceVerts, 4, &FaceInfo ) )
	{
		jeErrorLog_Add( JE_ERR_INTERNAL_RESOURCE, "Trace" );
		jeBrush_Destroy( &Brush);
	}

	FaceVerts[3]	=Verts[3];
	FaceVerts[2]	=Verts[2];
	FaceVerts[1]	=Verts[6];
	FaceVerts[0]	=Verts[5];

	if( !CamObj_CreateFace( Brush, FaceVerts, 4, &FaceInfo ) )
	{
		jeErrorLog_Add( JE_ERR_INTERNAL_RESOURCE, "Trace" );
		jeBrush_Destroy( &Brush);
	}



	FaceVerts[3]	=Verts[8];
	FaceVerts[2]	=Verts[9];
	FaceVerts[1]	=Verts[10];
	FaceVerts[0]	=Verts[11];

	if( !CamObj_CreateFace( Brush, FaceVerts, 4, &FaceInfo ) )
	{
		jeErrorLog_Add( JE_ERR_INTERNAL_RESOURCE, "Trace" );
		jeBrush_Destroy( &Brush);
	}

	FaceVerts[3]	=Verts[12];
	FaceVerts[2]	=Verts[13];
	FaceVerts[1]	=Verts[14];
	FaceVerts[0]	=Verts[15];

	if( !CamObj_CreateFace( Brush, FaceVerts, 4, &FaceInfo ) )
	{
		jeErrorLog_Add( JE_ERR_INTERNAL_RESOURCE, "Trace" );
		jeBrush_Destroy( &Brush);
	}

	FaceVerts[3]	=Verts[9];
	FaceVerts[2]	=Verts[15];
	FaceVerts[1]	=Verts[14];
	FaceVerts[0]	=Verts[13];

	if( !CamObj_CreateFace( Brush, FaceVerts, 4, &FaceInfo ) )
	{
		jeErrorLog_Add( JE_ERR_INTERNAL_RESOURCE, "Trace" );
		jeBrush_Destroy( &Brush);
	}

	FaceVerts[3]	=Verts[8];
	FaceVerts[2]	=Verts[11];
	FaceVerts[1]	=Verts[13];
	FaceVerts[0]	=Verts[12];

	if( !CamObj_CreateFace( Brush, FaceVerts, 4, &FaceInfo ) )
	{
		jeErrorLog_Add( JE_ERR_INTERNAL_RESOURCE, "Trace" );
		jeBrush_Destroy( &Brush);
	}

	FaceVerts[3]	=Verts[8];
	FaceVerts[2]	=Verts[12];
	FaceVerts[1]	=Verts[15];
	FaceVerts[0]	=Verts[9];

	if( !CamObj_CreateFace( Brush, FaceVerts, 4, &FaceInfo ) )
	{
		jeErrorLog_Add( JE_ERR_INTERNAL_RESOURCE, "Trace" );
		jeBrush_Destroy( &Brush);
	}

	FaceVerts[3]	=Verts[11];
	FaceVerts[2]	=Verts[10];
	FaceVerts[1]	=Verts[14];
	FaceVerts[0]	=Verts[13];

	if( !CamObj_CreateFace( Brush, FaceVerts, 4, &FaceInfo ) )
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

#ifdef WIN32
void Init_Class( HINSTANCE hInstance )
#endif
#ifdef BUILD_BE
void Init_Class( image_id hInstance )
#endif
{
	char * FieldName;

	FieldName = Util_LoadLibraryString(hInstance, IDS_FOV );
	if( FieldName )
	{
		jeProperty_FillFloat( &CamProperties[CAMREA_FOV_INDEX],FieldName,  1.0f, CAMREA_FOV_ID, 0.1f, 4.0f, 0.1f );
		jeRam_Free( FieldName );
	}

	// BEGIN - Far clip plane box - paradoxnj 3/9/2005
	FieldName = Util_LoadLibraryString(hInstance, IDS_FARCLIP);
	if (FieldName)
	{
		jeProperty_FillFloat( &CamProperties[CAMREA_FARCLIP_INDEX], FieldName, 10000.0f, CAMREA_FARCLIP_ID, 1.0f, 99999.0f, 1.0f);
		jeRam_Free(FieldName);
	}

	FieldName = Util_LoadLibraryString(hInstance, IDS_FARCLIPENABLED);
	if (FieldName)
	{
		jeProperty_FillCheck(&CamProperties[CAMREA_FARCLIPENABLED_INDEX], FieldName, JE_TRUE, CAMREA_FARCLIPENABLE_ID);
		jeRam_Free(FieldName);
	}
	// END - Far clip plane box - paradoxnj 3/9/2005
}



void * JETCC CreateInstance( void )
{
	CamObj *pCamObj;

	pCamObj = JE_RAM_ALLOCATE_STRUCT_CLEAR( CamObj );
	if( pCamObj == NULL )
	{
		jeErrorLog_Add( JE_ERR_MEMORY_RESOURCE, "CreateInstance");
		return( NULL );
	}
	pCamObj->FOV = 2.0f;
	jeXForm3d_SetIdentity( &pCamObj->XForm );

	
	pCamObj->RefCnt = 1;
	return( pCamObj );

}

void * JETCC DuplicateInstance(void * Instance)
{
	CamObj *pCamObj = (CamObj*)Instance;
	CamObj *pNewCamObj;

	pNewCamObj = (CamObj *)CreateInstance( );
	if( pNewCamObj == NULL )
	{
		jeErrorLog_Add( JE_ERR_SUBSYSTEM_FAILURE, "DuplicateInstance:CreateInstance");
		return( NULL );
	}
	jeXForm3d_Copy( &pCamObj->XForm, &pNewCamObj->XForm );
	pNewCamObj->FOV			= pCamObj->FOV;		
	pNewCamObj->RefCnt		= pCamObj->RefCnt;		

	return( pNewCamObj );
}

void JETCC CreateRef(void * Instance)
{
	CamObj *pCamObj = (CamObj*)Instance;

	assert( Instance );

	pCamObj->RefCnt++;
}

jeBoolean JETCC Destroy(void **pInstance)
{
	CamObj **hCamObj = (CamObj**)pInstance;
	CamObj *pCamObj = *hCamObj;

	assert( pInstance );
	assert( pCamObj->RefCnt > 0 );

	pCamObj->RefCnt--;
	if( pCamObj->RefCnt == 0 )
	{
		jeRam_Free( pCamObj );
	}
	else
		return( JE_FALSE );
	return( JE_TRUE );
}


jeBoolean JETCC Render(const void * Instance, const jeWorld * pWorld, const jeEngine *Engine, const jeCamera *Camera, const jeFrustum *CameraSpaceFrustum, jeObject_RenderFlags RenderFlags)
{

	// BEGIN - Far clip plane box - paradoxnj 3/9/2005
	CamObj					*pCamObj = (CamObj*)Instance;
	jeRect					Rect;
	jeFloat					FOV;

	jeCamera_GetAttributes((jeCamera*)Camera, &FOV, &Rect);
	jeCamera_SetFarClipPlane((jeCamera*)Camera, pCamObj->FarClipEnabled, pCamObj->FarClip);
	jeCamera_SetAttributes((jeCamera*)Camera, pCamObj->FOV, &Rect);
	//END - Far clip plane box - paradoxnj 3/9/2005

	return( JE_TRUE );

}

jeBoolean	JETCC AttachWorld( void * Instance, jeWorld * pWorld )
{
	CamObj *pCamObj = (CamObj*)Instance;

	assert( Instance );
	return( JE_TRUE );
}

jeBoolean	JETCC DettachWorld( void * Instance, jeWorld * pWorld )
{
	CamObj *pCamObj = (CamObj*)Instance;

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
	CamObj *pCamObj = (CamObj*)Instance;

	assert( Instance );

	jeBrush_SetXForm( Brush,  &pCamObj->XForm, JE_FALSE );
	BrushExtBox( Brush, BBox );
	return( JE_TRUE );
}


void *	JETCC CreateFromFile(jeVFile * File, jePtrMgr *PtrMgr)
{
	CamObj	*pCamObj;
	BYTE Version;
	uint32 Tag;
	OutputDebugString("CamObject\n");
	pCamObj = JE_RAM_ALLOCATE_STRUCT( CamObj );
	
	if( pCamObj == NULL )
		return( NULL );

	if(!jeVFile_Read(File, &Tag, sizeof(Tag)))
	{
		jeErrorLog_Add( JE_ERR_FILEIO_READ, "CamObject_CreateFromFile:Tag" );
		goto CFF_ERROR;
	}

	if (Tag == FILE_UNIQUE_ID)
	{
		if (!jeVFile_Read(File, &Version, sizeof(Version)))
		{
    		jeErrorLog_Add( JE_ERR_FILEIO_READ, "CamObject_CreateFromFile:Version" );
	       	goto CFF_ERROR;
		}
	}
	else
	{
		Version = 1;
		jeVFile_Seek(File,-((int)sizeof(Tag)),JE_VFILE_SEEKCUR);
	}
	
	if (Version >= 1)
	{
    	if( !jeVFile_Read(	File, &pCamObj->FOV, sizeof( pCamObj->FOV) ) )
		{
		    jeErrorLog_Add( JE_ERR_FILEIO_READ, "CamObject_CreateFromFile:FOV" );
		    goto CFF_ERROR;
		}

		if (!jeVFile_Read(File, &pCamObj->XForm, sizeof(pCamObj->XForm)))
		{
		    jeErrorLog_Add( JE_ERR_FILEIO_READ, "CamObject_CreateFromFile:XForm" );
		    goto CFF_ERROR;
		}

	}

	if (Version >= 2)
	{
    	// BEGIN - Far clip plane box - paradoxnj 3/9/2005
	    if (!jeVFile_Read(File, &pCamObj->FarClipEnabled, sizeof(jeBoolean)))
		{
		    jeErrorLog_Add( JE_ERR_FILEIO_READ, "CamObject_CreateFromFile:FarClipEnable");
		    goto CFF_ERROR;
		}

	    if (!jeVFile_Read(File, &pCamObj->FarClip, sizeof(jeFloat)))
		{
		    jeErrorLog_Add( JE_ERR_FILEIO_READ, "CamObject_CreateFromFile:FarClip");
		    goto CFF_ERROR;
		}
	    // END - Far clip plane box - paradoxnj 3/9/2005
	}
	else
	{
		// Defualt Values
		pCamObj->FarClipEnabled = JE_FALSE;
        pCamObj->FarClip = 10000.00f;
	}


	pCamObj->RefCnt = 1;
	return( pCamObj );

CFF_ERROR:

	jeRam_Free( pCamObj );
	return( NULL );
}



jeBoolean	JETCC WriteToFile(const void * Instance,jeVFile * File, jePtrMgr *PtrMgr)
{
	BYTE Version = CAMOBJECT_VERSION;
	uint32 Tag = FILE_UNIQUE_ID;

	CamObj *pCamObj = (CamObj*)Instance;
	
	
	assert( Instance );

	if( !jeVFile_Write(	File, &Tag,sizeof(Tag)))
	{
    	jeErrorLog_Add(JE_ERR_FILEIO_WRITE, "CamObject_WriteToFile:Tag");
	    return( JE_FALSE );
	}
	
	if( !jeVFile_Write(	File, &Version, sizeof(Version) ) )
	{
    	jeErrorLog_Add(JE_ERR_FILEIO_WRITE, "CamObject_WriteToFile:Version");
	    return( JE_FALSE );
	}

	if( !jeVFile_Write(	File, &pCamObj->FOV, sizeof( pCamObj->FOV) ) )
	{
		jeErrorLog_Add( JE_ERR_FILEIO_WRITE, "CreateFromFile:FOV" );
		return( JE_FALSE );
	}

	if (!jeVFile_Write(File, &pCamObj->XForm, sizeof(pCamObj->XForm)))
	{
		jeErrorLog_Add( JE_ERR_FILEIO_WRITE, "CreateFromFile:XForm" );
		return( JE_FALSE );
	}

	// BEGIN - Far clip plane box - paradoxnj 3/9/2005
	if (!jeVFile_Write(File, &pCamObj->FarClipEnabled, sizeof(jeBoolean)))
	{
		jeErrorLog_Add(JE_ERR_FILEIO_WRITE, "WriteToFile:FarClipEnabled");
		return JE_FALSE;
	}

	if (!jeVFile_Write(File, &pCamObj->FarClip, sizeof(jeFloat)))
	{
		jeErrorLog_Add(JE_ERR_FILEIO_WRITE, "WriteToFile:FarClip");
		return JE_FALSE;
	}
	// END - Far clip plane box - paradoxnj 3/9/2005

	return( JE_TRUE );
}

jeBoolean	JETCC GetPropertyList(void * Instance, jeProperty_List **List)
{
	CamObj *pCamObj = (CamObj*)Instance;

	assert( Instance );

	CamProperties[CAMREA_FOV_INDEX].Data.Float = pCamObj->FOV;
	// BEGIN - Far clip plane box - paradoxnj 3/9/2005
	CamProperties[CAMREA_FARCLIP_INDEX].Data.Float = pCamObj->FarClip;
	CamProperties[CAMREA_FARCLIPENABLED_INDEX].Data.Bool = pCamObj->FarClipEnabled;
	// END - Far clip plane box - paradoxnj 3/9/2005

	*List = jeProperty_ListCopy( &CamPropertyList );
	if( *List == NULL )
		return( JE_FALSE );
	return( JE_TRUE );
}

jeBoolean	JETCC SetProperty( void * Instance, int32 FieldID, PROPERTY_FIELD_TYPE DataType, jeProperty_Data * pData )
{
	CamObj *pCamObj = (CamObj*)Instance;

	assert( Instance );
	switch( FieldID  )
	{
	case CAMREA_FOV_ID:
		assert( DataType == PROPERTY_FLOAT_TYPE );
		pCamObj->FOV = pData->Float;
		break;
		
	// BEGIN - Far clip plane box - paradoxnj 3/9/2005
	case CAMREA_FARCLIPENABLE_ID:
		assert( DataType == PROPERTY_CHECK_TYPE );
		pCamObj->FarClipEnabled = pData->Bool;
		break;

	case CAMREA_FARCLIP_ID:
		assert(DataType == PROPERTY_FLOAT_TYPE);
		pCamObj->FarClip = pData->Float;
		break;
	// END - Far clip plane box - paradoxnj 3/9/2005

	}
	return( JE_TRUE );
}

jeBoolean	JETCC GetProperty( void * Instance, int32 FieldID, PROPERTY_FIELD_TYPE DataType, jeProperty_Data * pData )
{
	CamObj *pCamObj = (CamObj*)Instance;

	assert( Instance );
	switch( FieldID  )
	{
	case CAMREA_FOV_ID:
		assert( DataType == PROPERTY_FLOAT_TYPE );
		pData->Float = pCamObj->FOV;
		break;

	// BEGIN - Far clip plane box - paradoxnj 3/9/2005
	case CAMREA_FARCLIPENABLE_ID:
		assert(DataType == PROPERTY_CHECK_TYPE);
		pData->Bool = pCamObj->FarClipEnabled;
		break;

	case CAMREA_FARCLIP_ID:
		assert(DataType == PROPERTY_FLOAT_TYPE);
		pData->Float = pCamObj->FarClip;
		break;
	// END - Far clip plane box - paradoxnj 3/9/2005
	}
	return( JE_TRUE );
}

jeBoolean	JETCC SetXForm(void * Instance,const jeXForm3d *XF)
{
	CamObj *pCamObj = (CamObj*)Instance;

	assert( Instance );
	jeXForm3d_Copy( XF, &pCamObj->XForm );

	pCamObj->XForm.Flags = XFORM3D_NONORTHOGONALISOK;
	jeXForm3d_Orthonormalize(&pCamObj->XForm);
	return( JE_TRUE);
}

jeBoolean JETCC GetXForm(const void * Instance,jeXForm3d *XF)
{
	CamObj *pCamObj = (CamObj*)Instance;

	assert( Instance );
	jeXForm3d_Copy( &pCamObj->XForm, XF  );
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

jeBoolean JETCC SendMsg(void * Instance, int32 Msg, void * Data)
{
	CamObj *pCamObj = (CamObj*)Instance;

	switch( Msg)
	{
		case JETEDITOR_GET_JEBRUSH:
		{
			jeBrush **hBrush = (jeBrush**)Data;
			if( Brush == NULL )
				if( !CreateGlobalBrush(16) )
					return(JE_FALSE);
			jeBrush_SetXForm( Brush, &pCamObj->XForm, JE_FALSE);
			*hBrush = Brush;
			return( JE_TRUE );
		}


	}
	return( JE_FALSE );
}

jeBoolean JETCC PortalFrame( void *Instance, jeFloat Time)
{
	Instance;
	Time;
	return JE_TRUE;
}

// Icestorm
jeBoolean	JETCC ChangeBoxCollision(const void *Instance,const jeVec3d *Pos, const jeExtBox *FrontBox, const jeExtBox *BackBox, jeExtBox *ImpactBox, jePlane *Plane)
{
	return( JE_FALSE );
}