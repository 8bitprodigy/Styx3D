/****************************************************************************************/
/*  BOXOBJECT.C                                                                         */
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
#include <image.h>
#include <Resources.h>
#endif

#include <string.h>
#include <memory.h>
#include <assert.h>

#include "BoxObject.h"
#include "jeTypes.h"
#include "jeProperty.h"
#include "jeUserPoly.h"
#include "Jet.h"
#include "Ram.h"
#include "Bitmap.h"
#include "VFile.h"
#include "Resource.h"
#include "EditMsg.h"
#include "Errorlog.h"


#define BOXOBJ_VERSION 1 


enum {
	BOXOBJ_SIZE_ID = PROPERTY_LOCAL_DATATYPE_START,
	BOXOBJ_NAMLIST_ID
};

jeBrush *	pBrush;

typedef struct BoxObj {
	jeUserPoly	*Faces[6];
	jeLVertex	Vertex[8];
	jeXForm3d	XForm;
	float		Size;
	int			RefCnt;
} BoxObj;

enum {
	BOX_SIZE_INDEX,
	BOX_LAST_INDEX
};

jeProperty BoxProperties[BOX_LAST_INDEX];
jeProperty_List BoxPropertyList = { 1, &BoxProperties[0] };

char *NameList[3];

#define UTIL_MAX_RESOURCE_LENGTH	(128)
static char stringbuffer[UTIL_MAX_RESOURCE_LENGTH + 1];

#define DEFAULT_SIZE 16.0f


static jeBoolean CreateGlobalBrush()
{
	pBrush = jeBrush_Create( 8 );
	if( pBrush )
	{
		if( !jeBrush_CreateFace( pBrush, 4 ) )
			goto CGB_ERROR;
		if( !jeBrush_CreateFace( pBrush, 4 ) )
			goto CGB_ERROR;
		if( !jeBrush_CreateFace( pBrush, 4 ) )
			goto CGB_ERROR;
		if( !jeBrush_CreateFace( pBrush, 4 ) )
			goto CGB_ERROR;
		if( !jeBrush_CreateFace( pBrush, 4 ) )
			goto CGB_ERROR;
		if( !jeBrush_CreateFace( pBrush, 4 ) )
			goto CGB_ERROR;
	}
	return( JE_TRUE );
CGB_ERROR:
	jeBrush_Destroy( &pBrush );
	pBrush = NULL;
	return( JE_FALSE );
}

static jeBoolean UpdateGlobalBrush( BoxObj *pBoxObject )
{
	jeBrush_Face *pFace;
	jeVec3d		  Vertex;

	pFace = jeBrush_GetNextFace( pBrush, NULL );
	if( pFace == NULL )
		return(JE_FALSE );

	jeVec3d_Set( &Vertex, pBoxObject->Vertex[3].X, pBoxObject->Vertex[3].Y, pBoxObject->Vertex[3].Z );
	jeVec3d_Scale( &Vertex, pBoxObject->Size, &Vertex );
	jeBrush_FaceSetVertByIndex( pFace, 0, &Vertex);
	jeVec3d_Set( &Vertex, pBoxObject->Vertex[2].X, pBoxObject->Vertex[2].Y, pBoxObject->Vertex[2].Z );
	jeVec3d_Scale( &Vertex, pBoxObject->Size, &Vertex );
	jeBrush_FaceSetVertByIndex( pFace, 1, &Vertex);
	jeVec3d_Set( &Vertex, pBoxObject->Vertex[1].X, pBoxObject->Vertex[1].Y, pBoxObject->Vertex[1].Z );
	jeVec3d_Scale( &Vertex, pBoxObject->Size, &Vertex );
	jeBrush_FaceSetVertByIndex( pFace, 2, &Vertex);
	jeVec3d_Set( &Vertex, pBoxObject->Vertex[0].X, pBoxObject->Vertex[0].Y, pBoxObject->Vertex[0].Z );
	jeVec3d_Scale( &Vertex, pBoxObject->Size, &Vertex );
	jeBrush_FaceSetVertByIndex( pFace, 3, &Vertex);

	pFace = jeBrush_GetNextFace( pBrush, pFace );
	if( pFace == NULL )
		return(JE_FALSE );

	jeVec3d_Set( &Vertex, pBoxObject->Vertex[0].X, pBoxObject->Vertex[0].Y, pBoxObject->Vertex[0].Z );
	jeVec3d_Scale( &Vertex, pBoxObject->Size, &Vertex );
	jeBrush_FaceSetVertByIndex( pFace, 0, &Vertex);
	jeVec3d_Set( &Vertex, pBoxObject->Vertex[1].X, pBoxObject->Vertex[1].Y, pBoxObject->Vertex[1].Z );
	jeVec3d_Scale( &Vertex, pBoxObject->Size, &Vertex );
	jeBrush_FaceSetVertByIndex( pFace, 1, &Vertex);
	jeVec3d_Set( &Vertex, pBoxObject->Vertex[5].X, pBoxObject->Vertex[5].Y, pBoxObject->Vertex[5].Z );
	jeVec3d_Scale( &Vertex, pBoxObject->Size, &Vertex );
	jeBrush_FaceSetVertByIndex( pFace, 2, &Vertex);
	jeVec3d_Set( &Vertex, pBoxObject->Vertex[4].X, pBoxObject->Vertex[4].Y, pBoxObject->Vertex[4].Z );
	jeVec3d_Scale( &Vertex, pBoxObject->Size, &Vertex );
	jeBrush_FaceSetVertByIndex( pFace, 3, &Vertex);

	pFace = jeBrush_GetNextFace( pBrush, pFace );
	if( pFace == NULL )
		return(JE_FALSE );

	jeVec3d_Set( &Vertex, pBoxObject->Vertex[1].X, pBoxObject->Vertex[1].Y, pBoxObject->Vertex[1].Z );
	jeVec3d_Scale( &Vertex, pBoxObject->Size, &Vertex );
	jeBrush_FaceSetVertByIndex( pFace, 0, &Vertex);
	jeVec3d_Set( &Vertex, pBoxObject->Vertex[2].X, pBoxObject->Vertex[2].Y, pBoxObject->Vertex[2].Z );
	jeVec3d_Scale( &Vertex, pBoxObject->Size, &Vertex );
	jeBrush_FaceSetVertByIndex( pFace, 1, &Vertex);
	jeVec3d_Set( &Vertex, pBoxObject->Vertex[6].X, pBoxObject->Vertex[6].Y, pBoxObject->Vertex[6].Z );
	jeVec3d_Scale( &Vertex, pBoxObject->Size, &Vertex );
	jeBrush_FaceSetVertByIndex( pFace, 2, &Vertex);
	jeVec3d_Set( &Vertex, pBoxObject->Vertex[5].X, pBoxObject->Vertex[5].Y, pBoxObject->Vertex[5].Z );
	jeVec3d_Scale( &Vertex, pBoxObject->Size, &Vertex );
	jeBrush_FaceSetVertByIndex( pFace, 3, &Vertex);

	pFace = jeBrush_GetNextFace( pBrush, pFace );
	if( pFace == NULL )
		return(JE_FALSE );

	jeVec3d_Set( &Vertex, pBoxObject->Vertex[2].X, pBoxObject->Vertex[2].Y, pBoxObject->Vertex[2].Z );
	jeVec3d_Scale( &Vertex, pBoxObject->Size, &Vertex );
	jeBrush_FaceSetVertByIndex( pFace, 0, &Vertex);
	jeVec3d_Set( &Vertex, pBoxObject->Vertex[3].X, pBoxObject->Vertex[3].Y, pBoxObject->Vertex[3].Z );
	jeVec3d_Scale( &Vertex, pBoxObject->Size, &Vertex );
	jeBrush_FaceSetVertByIndex( pFace, 1, &Vertex);
	jeVec3d_Set( &Vertex, pBoxObject->Vertex[7].X, pBoxObject->Vertex[7].Y, pBoxObject->Vertex[7].Z );
	jeVec3d_Scale( &Vertex, pBoxObject->Size, &Vertex );
	jeBrush_FaceSetVertByIndex( pFace, 2, &Vertex);
	jeVec3d_Set( &Vertex, pBoxObject->Vertex[6].X, pBoxObject->Vertex[6].Y, pBoxObject->Vertex[6].Z );
	jeVec3d_Scale( &Vertex, pBoxObject->Size, &Vertex );
	jeBrush_FaceSetVertByIndex( pFace, 3, &Vertex);

	pFace = jeBrush_GetNextFace( pBrush, pFace );
	if( pFace == NULL )
		return(JE_FALSE );

	jeVec3d_Set( &Vertex, pBoxObject->Vertex[3].X, pBoxObject->Vertex[3].Y, pBoxObject->Vertex[3].Z );
	jeVec3d_Scale( &Vertex, pBoxObject->Size, &Vertex );
	jeBrush_FaceSetVertByIndex( pFace, 0, &Vertex);
	jeVec3d_Set( &Vertex, pBoxObject->Vertex[0].X, pBoxObject->Vertex[0].Y, pBoxObject->Vertex[0].Z );
	jeVec3d_Scale( &Vertex, pBoxObject->Size, &Vertex );
	jeBrush_FaceSetVertByIndex( pFace, 1, &Vertex);
	jeVec3d_Set( &Vertex, pBoxObject->Vertex[4].X, pBoxObject->Vertex[4].Y, pBoxObject->Vertex[4].Z );
	jeVec3d_Scale( &Vertex, pBoxObject->Size, &Vertex );
	jeBrush_FaceSetVertByIndex( pFace, 2, &Vertex);
	jeVec3d_Set( &Vertex, pBoxObject->Vertex[7].X, pBoxObject->Vertex[7].Y, pBoxObject->Vertex[7].Z );
	jeVec3d_Scale( &Vertex, pBoxObject->Size, &Vertex );
	jeBrush_FaceSetVertByIndex( pFace, 3, &Vertex);

	pFace = jeBrush_GetNextFace( pBrush, pFace );
	if( pFace == NULL )
		return(JE_FALSE );

	jeVec3d_Set( &Vertex, pBoxObject->Vertex[4].X, pBoxObject->Vertex[4].Y, pBoxObject->Vertex[4].Z );
	jeVec3d_Scale( &Vertex, pBoxObject->Size, &Vertex );
	jeBrush_FaceSetVertByIndex( pFace, 0, &Vertex);
	jeVec3d_Set( &Vertex, pBoxObject->Vertex[5].X, pBoxObject->Vertex[5].Y, pBoxObject->Vertex[5].Z );
	jeVec3d_Scale( &Vertex, pBoxObject->Size, &Vertex );
	jeBrush_FaceSetVertByIndex( pFace, 1, &Vertex);
	jeVec3d_Set( &Vertex, pBoxObject->Vertex[6].X, pBoxObject->Vertex[6].Y, pBoxObject->Vertex[6].Z );
	jeVec3d_Scale( &Vertex, pBoxObject->Size, &Vertex );
	jeBrush_FaceSetVertByIndex( pFace, 2, &Vertex);
	jeVec3d_Set( &Vertex, pBoxObject->Vertex[7].X, pBoxObject->Vertex[7].Y, pBoxObject->Vertex[7].Z );
	jeVec3d_Scale( &Vertex, pBoxObject->Size, &Vertex );
	jeBrush_FaceSetVertByIndex( pFace, 3, &Vertex);

	jeBrush_SetXForm( pBrush, &pBoxObject->XForm, JE_FALSE );
	return( JE_TRUE );
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

static jeBoolean BoxObject_InitBox( BoxObj * pBoxObj )
{
	int i;
	assert( pBoxObj );
	assert( pBoxObj->Size > 0.0f );
	for( i = 0; i < 8; i++ )
	{
		pBoxObj->Vertex[i].r = 255.0f;
		pBoxObj->Vertex[i].g = 255.0f;
		pBoxObj->Vertex[i].b = 255.0f;
		pBoxObj->Vertex[i].a = 255.0f;
		pBoxObj->Vertex[i].u = 0.0f;
		pBoxObj->Vertex[i].v = 0.0f;
		pBoxObj->Vertex[i].sr = 255.0f;
		pBoxObj->Vertex[i].sg = 255.0f;
		pBoxObj->Vertex[i].sb = 255.0f;
	}

	pBoxObj->Vertex[0].X = 0.5f;
	pBoxObj->Vertex[0].Y = 0.5f;
	pBoxObj->Vertex[0].Z = 0.5f;

	pBoxObj->Vertex[1].X = 0.5f;
	pBoxObj->Vertex[1].Y = 0.5f;
	pBoxObj->Vertex[1].Z = -0.5f;

	pBoxObj->Vertex[2].X = -0.5f;
	pBoxObj->Vertex[2].Y = 0.5f;
	pBoxObj->Vertex[2].Z = -0.5f;

	pBoxObj->Vertex[3].X = -0.5f;
	pBoxObj->Vertex[3].Y = 0.5f;
	pBoxObj->Vertex[3].Z = 0.5f;


	pBoxObj->Vertex[4].X = 0.5f;
	pBoxObj->Vertex[4].Y = -0.5f;
	pBoxObj->Vertex[4].Z = 0.5f;

	pBoxObj->Vertex[5].X = 0.5f;
	pBoxObj->Vertex[5].Y = -0.5f;
	pBoxObj->Vertex[5].Z = -0.5f;

	pBoxObj->Vertex[6].X = -0.5f;
	pBoxObj->Vertex[6].Y = -0.5f;
	pBoxObj->Vertex[6].Z = -0.5f;

	pBoxObj->Vertex[7].X = -0.5f;
	pBoxObj->Vertex[7].Y = -0.5f;
	pBoxObj->Vertex[7].Z = 0.5f;


	memset( pBoxObj->Faces, sizeof( jeUserPoly	*) * 6, 0 );
	pBoxObj->Faces[0] = jeUserPoly_CreateQuad(	&pBoxObj->Vertex[3], 
												&pBoxObj->Vertex[2],
												&pBoxObj->Vertex[1],
												&pBoxObj->Vertex[0] ,
												NULL,
												0 );
	if( pBoxObj->Faces[0] == NULL )
		goto INITBOX_ERR;

	pBoxObj->Faces[1] = jeUserPoly_CreateQuad(	&pBoxObj->Vertex[0], 
												&pBoxObj->Vertex[1],
												&pBoxObj->Vertex[5],
												&pBoxObj->Vertex[4] ,
												NULL,
												0 );
	if( pBoxObj->Faces[1] == NULL )
		goto INITBOX_ERR;

	pBoxObj->Faces[2] = jeUserPoly_CreateQuad(	&pBoxObj->Vertex[1], 
												&pBoxObj->Vertex[2],
												&pBoxObj->Vertex[6],
												&pBoxObj->Vertex[5] ,
												NULL,
												0 );
	if( pBoxObj->Faces[2] == NULL )
		goto INITBOX_ERR;

	pBoxObj->Faces[3] = jeUserPoly_CreateQuad(	&pBoxObj->Vertex[2], 
												&pBoxObj->Vertex[3],
												&pBoxObj->Vertex[7],
												&pBoxObj->Vertex[6] ,
												NULL,
												0 );
	if( pBoxObj->Faces[3] == NULL )
		goto INITBOX_ERR;

	pBoxObj->Faces[4] = jeUserPoly_CreateQuad(	&pBoxObj->Vertex[3], 
												&pBoxObj->Vertex[0],
												&pBoxObj->Vertex[4],
												&pBoxObj->Vertex[7] ,
												NULL,
												0 );
	if( pBoxObj->Faces[4] == NULL )
		goto INITBOX_ERR;

	pBoxObj->Faces[5] = jeUserPoly_CreateQuad(	&pBoxObj->Vertex[4], 
												&pBoxObj->Vertex[5],
												&pBoxObj->Vertex[6],
												&pBoxObj->Vertex[7] ,
												NULL,
												0 );
	if( pBoxObj->Faces[5] == NULL )
		goto INITBOX_ERR;

	return( JE_TRUE );

INITBOX_ERR:
	for( i = 0; i < 6; i++ )
		if( pBoxObj->Faces[i] != NULL )
			jeUserPoly_Destroy(&pBoxObj->Faces[i]);
	return( JE_FALSE );
}				

static void BoxObject_TransformVert( jeLVertex * pVertex, jeXForm3d * pXForm )
{
	jeVec3d Point;
	assert( pVertex );
	assert( pXForm );

	//I guess for speed I could cheat and recognize that the structure from
	//pVertex->X matches Vec3d Struct.
	Point.X = pVertex->X;
	Point.Y = pVertex->Y;
	Point.Z = pVertex->Z;

	jeXForm3d_Transform( pXForm, &Point, &Point );

	pVertex->X = Point.X;
	pVertex->Y = Point.Y;
	pVertex->Z = Point.Z;
}

static jeBoolean BoxObject_UpdateFaces( BoxObj * pBoxObj )
{

	jeLVertex	Vertex[4];
	jeXForm3d	ScaleXForm;

	jeXForm3d_SetIdentity(&ScaleXForm);
	jeXForm3d_Scale(&ScaleXForm, pBoxObj->Size, pBoxObj->Size, pBoxObj->Size);

	Vertex[0] = pBoxObj->Vertex[3];
	BoxObject_TransformVert( &Vertex[0], &ScaleXForm );
	BoxObject_TransformVert( &Vertex[0], &pBoxObj->XForm );
	Vertex[1] = pBoxObj->Vertex[2];
	BoxObject_TransformVert( &Vertex[1], &ScaleXForm );
	BoxObject_TransformVert( &Vertex[1], &pBoxObj->XForm );
	Vertex[2] = pBoxObj->Vertex[1];
	BoxObject_TransformVert( &Vertex[2], &ScaleXForm );
	BoxObject_TransformVert( &Vertex[2], &pBoxObj->XForm );
	Vertex[3] = pBoxObj->Vertex[0];
	BoxObject_TransformVert( &Vertex[3], &ScaleXForm );
	BoxObject_TransformVert( &Vertex[3], &pBoxObj->XForm );

	 if( !jeUserPoly_UpdateQuad(				pBoxObj->Faces[0],	
												&Vertex[0], 
												&Vertex[1],
												&Vertex[2],
												&Vertex[3] ,
												NULL ) )
	 {
		 return( JE_FALSE );
	 }

	Vertex[0] = pBoxObj->Vertex[0];
	BoxObject_TransformVert( &Vertex[0], &ScaleXForm );
	BoxObject_TransformVert( &Vertex[0], &pBoxObj->XForm );
	Vertex[1] = pBoxObj->Vertex[1];
	BoxObject_TransformVert( &Vertex[1], &ScaleXForm );
	BoxObject_TransformVert( &Vertex[1], &pBoxObj->XForm );
	Vertex[2] = pBoxObj->Vertex[5];
	BoxObject_TransformVert( &Vertex[2], &ScaleXForm );
	BoxObject_TransformVert( &Vertex[2], &pBoxObj->XForm );
	Vertex[3] = pBoxObj->Vertex[4];
	BoxObject_TransformVert( &Vertex[3], &ScaleXForm );
	BoxObject_TransformVert( &Vertex[3], &pBoxObj->XForm );

	 if( !jeUserPoly_UpdateQuad(				pBoxObj->Faces[1],	
												&Vertex[0], 
												&Vertex[1],
												&Vertex[2],
												&Vertex[3] ,
												NULL ) )
	 {
		 return( JE_FALSE );
	 }

	Vertex[0] = pBoxObj->Vertex[1];
	BoxObject_TransformVert( &Vertex[0], &ScaleXForm );
	BoxObject_TransformVert( &Vertex[0], &pBoxObj->XForm );
	Vertex[1] = pBoxObj->Vertex[2];
	BoxObject_TransformVert( &Vertex[1], &ScaleXForm );
	BoxObject_TransformVert( &Vertex[1], &pBoxObj->XForm );
	Vertex[2] = pBoxObj->Vertex[6];
	BoxObject_TransformVert( &Vertex[2], &ScaleXForm );
	BoxObject_TransformVert( &Vertex[2], &pBoxObj->XForm );
	Vertex[3] = pBoxObj->Vertex[5];
	BoxObject_TransformVert( &Vertex[3], &ScaleXForm );
	BoxObject_TransformVert( &Vertex[3], &pBoxObj->XForm );

	 if( !jeUserPoly_UpdateQuad(				pBoxObj->Faces[2],	
												&Vertex[0], 
												&Vertex[1],
												&Vertex[2],
												&Vertex[3] ,
												NULL ) )
	 {
		 return( JE_FALSE );
	 }

	Vertex[0] = pBoxObj->Vertex[2];
	BoxObject_TransformVert( &Vertex[0], &ScaleXForm );
	BoxObject_TransformVert( &Vertex[0], &pBoxObj->XForm );
	Vertex[1] = pBoxObj->Vertex[3];
	BoxObject_TransformVert( &Vertex[1], &ScaleXForm );
	BoxObject_TransformVert( &Vertex[1], &pBoxObj->XForm );
	Vertex[2] = pBoxObj->Vertex[7];
	BoxObject_TransformVert( &Vertex[2], &ScaleXForm );
	BoxObject_TransformVert( &Vertex[2], &pBoxObj->XForm );
	Vertex[3] = pBoxObj->Vertex[6];
	BoxObject_TransformVert( &Vertex[3], &ScaleXForm );
	BoxObject_TransformVert( &Vertex[3], &pBoxObj->XForm );

	 if( !jeUserPoly_UpdateQuad(				pBoxObj->Faces[3],	
												&Vertex[0], 
												&Vertex[1],
												&Vertex[2],
												&Vertex[3] ,
												NULL ) )
	 {
		 return( JE_FALSE );
	 }

	Vertex[0] = pBoxObj->Vertex[3];
	BoxObject_TransformVert( &Vertex[0], &ScaleXForm );
	BoxObject_TransformVert( &Vertex[0], &pBoxObj->XForm );
	Vertex[1] = pBoxObj->Vertex[0];
	BoxObject_TransformVert( &Vertex[1], &ScaleXForm );
	BoxObject_TransformVert( &Vertex[1], &pBoxObj->XForm );
	Vertex[2] = pBoxObj->Vertex[4];
	BoxObject_TransformVert( &Vertex[2], &ScaleXForm );
	BoxObject_TransformVert( &Vertex[2], &pBoxObj->XForm );
	Vertex[3] = pBoxObj->Vertex[7];
	BoxObject_TransformVert( &Vertex[3], &ScaleXForm );
	BoxObject_TransformVert( &Vertex[3], &pBoxObj->XForm );

	 if( !jeUserPoly_UpdateQuad(				pBoxObj->Faces[4],
												&Vertex[0], 
												&Vertex[1],
												&Vertex[2],
												&Vertex[3] ,
												NULL ) )
	 {
		 return( JE_FALSE );
	 }

	Vertex[0] = pBoxObj->Vertex[4];
	BoxObject_TransformVert( &Vertex[0], &ScaleXForm );
	BoxObject_TransformVert( &Vertex[0], &pBoxObj->XForm );
	Vertex[1] = pBoxObj->Vertex[5];
	BoxObject_TransformVert( &Vertex[1], &ScaleXForm );
	BoxObject_TransformVert( &Vertex[1], &pBoxObj->XForm );
	Vertex[2] = pBoxObj->Vertex[6];
	BoxObject_TransformVert( &Vertex[2], &ScaleXForm );
	BoxObject_TransformVert( &Vertex[2], &pBoxObj->XForm );
	Vertex[3] = pBoxObj->Vertex[7];
	BoxObject_TransformVert( &Vertex[3], &ScaleXForm );
	BoxObject_TransformVert( &Vertex[3], &pBoxObj->XForm );

	 if( !jeUserPoly_UpdateQuad(				pBoxObj->Faces[5],	
												&Vertex[0], 
												&Vertex[1],
												&Vertex[2],
												&Vertex[3] ,
												NULL ) )
	 {
		 return( JE_FALSE );
	 }

	return( JE_TRUE );

}				

#ifdef WIN32
void Init_Class( HINSTANCE hInstance )
#endif
#ifdef BUILD_BE
void Init_Class( image_id hInstance )
#endif
{
	BoxProperties[BOX_SIZE_INDEX].Type = PROPERTY_FLOAT_TYPE;
	BoxProperties[BOX_SIZE_INDEX].bDisabled = JE_FALSE;
	BoxProperties[BOX_SIZE_INDEX].Data.Float = 16.0f;
	BoxProperties[BOX_SIZE_INDEX].DataId = BOXOBJ_SIZE_ID;
	BoxProperties[BOX_SIZE_INDEX].DataSize = sizeof( float );
	BoxProperties[BOX_SIZE_INDEX].FieldName = Util_LoadLibraryString(hInstance, IDS_SIZE );
    BoxProperties[BOX_SIZE_INDEX].TypeInfo.NumInfo.Min = 1.0f;
	BoxProperties[BOX_SIZE_INDEX].TypeInfo.NumInfo.Max = 64.0f;
	BoxProperties[BOX_SIZE_INDEX].TypeInfo.NumInfo.Increment = 1.0f;

}



void * JETCC CreateInstance( void )
{
	BoxObj *pBoxObj;

	pBoxObj = JE_RAM_ALLOCATE_STRUCT( BoxObj );
	if( pBoxObj == NULL )
	{
		jeErrorLog_Add( JE_ERR_MEMORY_RESOURCE, "CreateInstance");
		return( NULL );
	}
	jeXForm3d_SetIdentity( &pBoxObj->XForm );
	pBoxObj->Size = DEFAULT_SIZE;
	pBoxObj->RefCnt = 1;
	if( !BoxObject_InitBox( pBoxObj ) )
	{
		jeRam_Free( pBoxObj );
		return( NULL );
	}
	return( pBoxObj );

}

void * JETCC DuplicateInstance(void * Instance)
{
	BoxObj *pBoxObj = (BoxObj*)Instance;
	BoxObj *pNewBoxObj;

	pNewBoxObj = (BoxObj *)CreateInstance( );
	if( pNewBoxObj == NULL )
	{
		jeErrorLog_Add( JE_ERR_SUBSYSTEM_FAILURE, "DuplicateInstance:CreateInstance");
		return( NULL );
	}
	SetXForm( pNewBoxObj, &pBoxObj->XForm );
	pNewBoxObj->Size = pBoxObj->Size;
	if( !BoxObject_UpdateFaces( pNewBoxObj ) )
	{
		Destroy( (void **)&pNewBoxObj );
		jeErrorLog_Add( JE_ERR_SUBSYSTEM_FAILURE, "DuplicateInstance:BoxObject_UpdateFaces");
		return( NULL );
	}
	return( pNewBoxObj );
}

void JETCC CreateRef(void * Instance)
{
	BoxObj *pBoxObj = (BoxObj*)Instance;

	assert( Instance );

	pBoxObj->RefCnt++;
}

jeBoolean JETCC Destroy(void **pInstance)
{
	int i;
	BoxObj **hBoxObj = (BoxObj**)pInstance;
	BoxObj *pBoxObj = *hBoxObj;

	assert( pInstance );
	assert( pBoxObj->RefCnt > 0 );

	pBoxObj->RefCnt--;
	if( pBoxObj->RefCnt == 0 )
	{
		for( i = 0; i < 6; i++ )
			if( pBoxObj->Faces[i] != NULL )
				jeUserPoly_Destroy(&pBoxObj->Faces[i]);
		jeRam_Free( pBoxObj );
	}
	else
		return( JE_FALSE );
	return( JE_TRUE );
}


jeBoolean JETCC Render(const void * Instance, const jeWorld * pWorld, const jeEngine *Engine, const jeCamera *Camera, const jeFrustum *CameraSpaceFrustum, jeObject_RenderFlags RenderFlags)
{

	return( JE_TRUE );

}

jeBoolean	JETCC AttachWorld( void * Instance, jeWorld * pWorld )
{
	int i;
	BoxObj *pBoxObj = (BoxObj*)Instance;

	assert( Instance );
	for( i = 0; i < 6; i++ )
		if( !jeWorld_AddUserPoly(pWorld ,pBoxObj->Faces[i], JE_FALSE) )
			return( JE_FALSE );
	return( JE_TRUE );
}

jeBoolean	JETCC DettachWorld( void * Instance, jeWorld * pWorld )
{
	int i;
	BoxObj *pBoxObj = (BoxObj*)Instance;

	assert( Instance );
	for( i = 0; i < 6; i++ )
		if( !jeWorld_RemoveUserPoly(pWorld ,pBoxObj->Faces[i]) )
			return( JE_FALSE );
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

jeBoolean	JETCC Collision(const void * Instance, const jeExtBox *Box, const jeVec3d *Front, const jeVec3d *Back, jeVec3d *Impact, jePlane *Plane)
{
	jeExtBox BBox;
	jeVec3d  Normal;
	jeFloat		T;
	
	GetExtBox(Instance,&BBox);
	if (Impact)
	{
		if( jeExtBox_RayCollision( &BBox, Front, Back, &T, &Normal ) )
		{
			jeVec3d_Subtract( Back, Front, Impact );
			jeVec3d_Scale( Impact, T, Impact );
			jeVec3d_Add( Back, Impact, Impact );
			return( JE_TRUE );
		}
		return( JE_FALSE );
	} else
		return jeExtBox_RayCollision( &BBox, Front, Back, NULL, NULL );
}


jeBoolean JETCC GetExtBox(const void * Instance,jeExtBox *BBox)
{
	jeVec3d Point;
	BoxObj *pBoxObj = (BoxObj*)Instance;
	int i;
	jeXForm3d	ScaleXForm;


	assert( Instance );
	assert( BBox );

	jeXForm3d_SetIdentity(&ScaleXForm);
	jeXForm3d_Scale(&ScaleXForm, pBoxObj->Size, pBoxObj->Size, pBoxObj->Size);

	Point.X = pBoxObj->Vertex[0].X;
	Point.Y = pBoxObj->Vertex[0].Y;
	Point.Z = pBoxObj->Vertex[0].Z;
	jeXForm3d_Transform( &ScaleXForm, &Point, &Point );
	jeXForm3d_Transform( &pBoxObj->XForm, &Point, &Point );

	jeExtBox_SetToPoint ( BBox, &Point );
	for( i = 1; i < 8 ; i ++ )
	{
		Point.X = pBoxObj->Vertex[i].X;
		Point.Y = pBoxObj->Vertex[i].Y;
		Point.Z = pBoxObj->Vertex[i].Z;
		jeXForm3d_Transform( &ScaleXForm, &Point, &Point );
		jeXForm3d_Transform( &pBoxObj->XForm, &Point, &Point );
		jeExtBox_ExtendToEnclose( BBox, &Point );
	}
	return( JE_TRUE );
}


void *	JETCC CreateFromFile(jeVFile * File, jePtrMgr *PtrMgr)
{
	BoxObj * pBoxObj;
	BYTE Version;
    uint32 Tag;

	pBoxObj = JE_RAM_ALLOCATE_STRUCT( BoxObj );
	if( pBoxObj == NULL )
		return( NULL );

 	if(!jeVFile_Read(File, &Tag, sizeof(Tag)))
	{
		jeErrorLog_Add( JE_ERR_FILEIO_READ, "BoxObject_CreateFromFile:Tag" );
		goto CFF_ERROR;
	}

	if (Tag == FILE_UNIQUE_ID)
	{
		if (!jeVFile_Read(File, &Version, sizeof(Version)))
		{
    		jeErrorLog_Add( JE_ERR_FILEIO_READ, "BoxObject_CreateFromFile:Version" );
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
		if( !jeVFile_Read(	File, &pBoxObj->XForm, sizeof( pBoxObj->XForm) ) )
		{
	   	    jeErrorLog_Add(JE_ERR_FILEIO_READ, "BoxObject_CreateFromFile:XForm");
        	goto CFF_ERROR;
		}

	    if( !jeVFile_Read(	File, &pBoxObj->Size, sizeof( pBoxObj->Size) ) )
		{
	   	    jeErrorLog_Add(JE_ERR_FILEIO_READ, "BoxObject_CreateFromFile:Size");
		    goto CFF_ERROR;
		}
	}

	pBoxObj->RefCnt = 1;
	if( !BoxObject_InitBox( pBoxObj ) )
		goto CFF_ERROR;
	if( !BoxObject_UpdateFaces( pBoxObj ) )
		goto CFF_ERROR;

	return( pBoxObj );

CFF_ERROR:

	jeRam_Free( pBoxObj );
	return( NULL );
}


jeBoolean	JETCC WriteToFile(const void * Instance,jeVFile * File, jePtrMgr *PtrMgr)
{
	BoxObj *pBoxObj = (BoxObj*)Instance;
	BYTE Version = BOXOBJ_VERSION;
	uint32 Tag = FILE_UNIQUE_ID;

	assert( Instance );

	if(!jeVFile_Write(File,&Tag, sizeof(Tag)))
	{
	   	jeErrorLog_Add(JE_ERR_FILEIO_WRITE, "BoxObject_WriteToFile:Tag");
		return( JE_FALSE );
	}

	if( !jeVFile_Write(	File, &Version, sizeof(Version) ) )
	{
	   	jeErrorLog_Add(JE_ERR_FILEIO_WRITE, "BoxObject_WriteToFile:Version");
		return( JE_FALSE );
	}

	if( !jeVFile_Write(	File, &pBoxObj->XForm, sizeof( pBoxObj->XForm) ) )
	{
		jeErrorLog_Add(JE_ERR_FILEIO_WRITE, "BoxObject_WriteToFile:XForm");
		return( JE_FALSE );
	}

	if( !jeVFile_Write(	File, &pBoxObj->Size, sizeof( pBoxObj->Size) ) )
	{
		jeErrorLog_Add(JE_ERR_FILEIO_WRITE, "BoxObject_WriteToFile:Size");
		return( JE_FALSE );
	}


	return( JE_TRUE );
}


jeBoolean	JETCC GetPropertyList(void * Instance, jeProperty_List **List)
{
	BoxObj *pBoxObj = (BoxObj*)Instance;

	assert( Instance );

	BoxProperties[BOX_SIZE_INDEX].Data.Float = pBoxObj->Size;

	*List = jeProperty_ListCopy( &BoxPropertyList);
	if( *List == NULL )
		return( JE_FALSE );
	return( JE_TRUE );
}

jeBoolean	JETCC SetProperty( void * Instance, int32 FieldID, PROPERTY_FIELD_TYPE DataType, jeProperty_Data * pData )
{
	BoxObj *pBoxObj = (BoxObj*)Instance;

	assert( Instance );
	if( FieldID == BOXOBJ_SIZE_ID )
	{
		pBoxObj->Size = pData->Float;
		BoxObject_UpdateFaces( pBoxObj );
	}
	return( JE_TRUE );
}

jeBoolean	JETCC SetXForm(void * Instance,const jeXForm3d *XF)
{
	BoxObj *pBoxObj = (BoxObj*)Instance;

	assert( Instance );

	pBoxObj->XForm = *XF;
	if( !BoxObject_UpdateFaces( pBoxObj ) )
		return JE_FALSE;
	return( JE_TRUE );
}

jeBoolean JETCC GetXForm(const void * Instance,jeXForm3d *XF)
{
	BoxObj *pBoxObj = (BoxObj*)Instance;

	assert( Instance );
	*XF = pBoxObj->XForm;
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
jeBoolean JETCC EditDialog (void * Instance, class G3DView *Parent)
#endif
{
	return( JE_TRUE );
}

jeBoolean JETCC SendMsg(void * Instance, int32 Msg, void * Data)
{
	BoxObj *pBoxObj = (BoxObj*)Instance;

	if( Msg == JETEDITOR_GET_JEBRUSH )
	{
		jeBrush **hBrush = (jeBrush**)Data;
		if( pBrush == NULL )
			if( !CreateGlobalBrush() )
				return(JE_FALSE);
		if( !UpdateGlobalBrush( pBoxObj ) )
			return(JE_FALSE );
		*hBrush = pBrush;
		return( JE_TRUE );
	}
	return( JE_FALSE );
}

// Icestorm: Collision ignores Box=>ChangeBoxCollision ignores all
jeBoolean	JETCC ChangeBoxCollision(const void *Instance,const jeVec3d *Pos, const jeExtBox *FrontBox, const jeExtBox *BackBox, jeExtBox *ImpactBox, jePlane *Plane)
{
	return( JE_FALSE );
}