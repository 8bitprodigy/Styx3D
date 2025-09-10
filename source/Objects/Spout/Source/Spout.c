/****************************************************************************************/
/*  SPOUT.C                                                                             */
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
#include <stdlib.h> // rand
#include <stdio.h>
#define stricmp strcasecmp
#define itoa( number, string, radix) sprintf(string, "%i" , number )
#endif

#include <assert.h>
#include <string.h>
#include <float.h>
#include "VFile.h"
#include "jeProperty.h"
#include "Ram.h"
#include "jeResource.h"
#include "jeWorld.h"
#include "Spout.h"
#include "Resource.h"
#include "jeParticle.h"
#include "jeVersion.h"
#include "Errorlog.h"
#include "jeMaterial.h"
#include "jeResource.h"


#define SPOUTOBJECT_VERSION 1


////////////////////////////////////////////////////////////////////////////////////////
//	Property list stuff
////////////////////////////////////////////////////////////////////////////////////////
enum
{
	SPOUT_RATE_ID = PROPERTY_LOCAL_DATATYPE_START,
	SPOUT_ANGLE_ID,
	SPOUT_MINSPEED_ID,
	SPOUT_MAXSPEED_ID,
	SPOUT_MINSCALE_ID,
	SPOUT_MAXSCALE_ID,
	SPOUT_MINUNITLIFE_ID,
	SPOUT_MAXUNITLIFE_ID,
	SPOUT_COLORMINGROUP_ID,
	SPOUT_COLORMINRED_ID,
	SPOUT_COLORMINGREEN_ID,
	SPOUT_COLORMINBLUE_ID,
	SPOUT_COLORMIN_ID,
	SPOUT_COLORMINGROUPEND_ID,
	SPOUT_COLORMAXGROUP_ID,
	SPOUT_COLORMAXRED_ID,
	SPOUT_COLORMAXGREEN_ID,
	SPOUT_COLORMAXBLUE_ID,
	SPOUT_COLORMAX_ID,
	SPOUT_COLORMAXGROUPEND_ID,
	SPOUT_DRAWEXTBOXGROUP_ID,
	SPOUT_DRAWEXTBOXDISPLAY_ID,
	SPOUT_DRAWEXTBOXMINX_ID,
	SPOUT_DRAWEXTBOXMINY_ID,
	SPOUT_DRAWEXTBOXMINZ_ID,
	SPOUT_DRAWEXTBOXMAXX_ID,
	SPOUT_DRAWEXTBOXMAXY_ID,
	SPOUT_DRAWEXTBOXMAXZ_ID,
	SPOUT_DRAWEXTBOXGROUPEND_ID,
	SPOUT_ARTGROUP_ID,
	SPOUT_ARTSIZE_ID,
	SPOUT_BITMAPLIST_ID,
	SPOUT_ALPHALIST_ID,
	SPOUT_ARTGROUPEND_ID,
	SPOUT_LAST_ID
};
enum
{
	SPOUT_RATE_INDEX = 0,
	SPOUT_ANGLE_INDEX,
	SPOUT_MINSPEED_INDEX,
	SPOUT_MAXSPEED_INDEX,
	SPOUT_MINSCALE_INDEX,
	SPOUT_MAXSCALE_INDEX,
	SPOUT_MINUNITLIFE_INDEX,
	SPOUT_MAXUNITLIFE_INDEX,
	SPOUT_COLORMINGROUP_INDEX,
	SPOUT_COLORMINRED_INDEX,
	SPOUT_COLORMINGREEN_INDEX,
	SPOUT_COLORMINBLUE_INDEX,
	SPOUT_COLORMIN_INDEX,
	SPOUT_COLORMINGROUPEND_INDEX,
	SPOUT_COLORMAXGROUP_INDEX,
	SPOUT_COLORMAXRED_INDEX,
	SPOUT_COLORMAXGREEN_INDEX,
	SPOUT_COLORMAXBLUE_INDEX,
	SPOUT_COLORMAX_INDEX,
	SPOUT_COLORMAXGROUPEND_INDEX,
	SPOUT_DRAWEXTBOXGROUP_INDEX,
	SPOUT_DRAWEXTBOXDISPLAY_INDEX,
	SPOUT_DRAWEXTBOXMINX_INDEX,
	SPOUT_DRAWEXTBOXMINY_INDEX,
	SPOUT_DRAWEXTBOXMINZ_INDEX,
	SPOUT_DRAWEXTBOXMAXX_INDEX,
	SPOUT_DRAWEXTBOXMAXY_INDEX,
	SPOUT_DRAWEXTBOXMAXZ_INDEX,
	SPOUT_DRAWEXTBOXGROUPEND_INDEX,
	SPOUT_ARTGROUP_INDEX,
	SPOUT_ARTSIZE_INDEX,
	SPOUT_BITMAPLIST_INDEX,
	SPOUT_ALPHALIST_INDEX,
	SPOUT_ARTGROUPEND_INDEX,
	SPOUT_LAST_INDEX
};


////////////////////////////////////////////////////////////////////////////////////////
//	Bitmaplist struct
////////////////////////////////////////////////////////////////////////////////////////
typedef struct BitmapList
{
	int		Total;
	char	**Name;
	int		*Width;
	int		*Height;
	int		*NumericSizes;
	char	**StringSizes;
	int		SizesListSize;

} BitmapList;


////////////////////////////////////////////////////////////////////////////////////////
//	Globals
////////////////////////////////////////////////////////////////////////////////////////
#ifdef WIN32
static HINSTANCE		hClassInstance = NULL;
#endif

#ifdef BUILD_BE
static image_id			hClassInstance = NULL;
#endif

static BitmapList		*Bitmaps, CurBitmaps;
static int				*BitmapWidth, *BitmapHeight;
static jeProperty		SpoutProperties[SPOUT_LAST_INDEX];
static jeProperty_List	SpoutPropertyList = { SPOUT_LAST_INDEX, &( SpoutProperties[0] ) };
static char				*NoSelection = "< none >";


////////////////////////////////////////////////////////////////////////////////////////
//	Defaults
////////////////////////////////////////////////////////////////////////////////////////
#define SPOUT_DEFAULT_RATE					0.25f
#define SPOUT_DEFAULT_ANGLE					0.5f
#define SPOUT_DEFAULT_MINSPEED				10.0f
#define SPOUT_DEFAULT_MAXSPEED				20.0f
#define SPOUT_DEFAULT_MINSCALE				1.0f
#define SPOUT_DEFAULT_MAXSCALE				1.0f
#define SPOUT_DEFAULT_MINUNITLIFE			2.0f
#define SPOUT_DEFAULT_MAXUNITLIFE			4.0f
#define SPOUT_DEFAULT_COLORRED				255.0f
#define SPOUT_DEFAULT_COLORGREEN			255.0f
#define SPOUT_DEFAULT_COLORBLUE				255.0f
#define SPOUT_DEFAULT_DRAWEXTBOXDISPLAY		JE_FALSE
#define SPOUT_DEFAULT_DRAWEXTBOXMINX		-16.0f
#define SPOUT_DEFAULT_DRAWEXTBOXMINY		-16.0f
#define SPOUT_DEFAULT_DRAWEXTBOXMINZ		-16.0f
#define SPOUT_DEFAULT_DRAWEXTBOXMAXX		16.0f
#define SPOUT_DEFAULT_DRAWEXTBOXMAXY		16.0f
#define SPOUT_DEFAULT_DRAWEXTBOXMAXZ		16.0f


////////////////////////////////////////////////////////////////////////////////////////
//	Object data
////////////////////////////////////////////////////////////////////////////////////////
typedef struct Spout
{
	jeParticle_System	*Ps;
	jeWorld				*World;
	jeResourceMgr		*ResourceMgr;
	jeEngine			*Engine;
	jeXForm3d			Xf;
	int					RefCount;
	int					CurWidth;
    jeMaterialSpec      *Art;
	//jeBitmap			*Art;
	char				*ArtName;
	char				*BitmapName;
	char				*AlphaName;
	float				TimeElapsed;
	float				Rate;
	JE_RGBA				MinColor, MaxColor;
	float				MinSpeed, MaxSpeed;
	float				MinScale, MaxScale;
	float				MinUnitLife, MaxUnitLife;
	float				Angle;
	jeVec3d				Gravity;
	jeBoolean			DrawExtBoxDisplay;
	jeExtBox			DrawExtBox;
	jeBoolean			LoadedFromDisk;

} Spout;



////////////////////////////////////////////////////////////////////////////////////////
//
//	Util_StrDup()
//
////////////////////////////////////////////////////////////////////////////////////////
static char * Util_StrDup(
	const char	*const psz )	// string to copy
{

	// copy string
	char * p = (char *)jeRam_Allocate( strlen( psz ) + 1 );
	if ( p ) 
	{
		strcpy( p, psz );
	}
	else
	{
		jeErrorLog_Add( JE_ERR_MEMORY_RESOURCE, NULL );
	}

	// return string
	return p;

} // Util_StrDup()



////////////////////////////////////////////////////////////////////////////////////////
//
//	Util_RecreateCurBitmapsList()
//
///////////////////////////////////////////////////////////////////////////////////////
static void Util_RecreateCurBitmapsList(
	Spout	*Object )
{

	// locals
	int	i;

	// ensure valid data
	assert( Object != NULL );

	// free current names			
	for ( i = 0; i < CurBitmaps.Total; i++ )
	{
		if( CurBitmaps.Name[i] != NULL )
		{
			jeRam_Free( CurBitmaps.Name[i] );
			CurBitmaps.Name[i] = NULL;
		}
	}
	CurBitmaps.Total = 0;

	// add new names
	for ( i = 0; i < Bitmaps->Total; i++ )
	{
		if ( ( Bitmaps->Width[i] == Bitmaps->NumericSizes[Object->CurWidth] ) || ( i == 0 ) )
		{
			CurBitmaps.Name[CurBitmaps.Total] = Util_StrDup( Bitmaps->Name[i] );
			CurBitmaps.Total++;
		}
	}

} // Util_RecreateCurBitmapsList()



////////////////////////////////////////////////////////////////////////////////////////
//
//	Util_DrawPoly()
//
///////////////////////////////////////////////////////////////////////////////////////
static void Util_DrawPoly(
	jeWorld		*World,	// world in which to draw poly
	jeLVertex	*V1,	// top left
	jeLVertex	*V2,	// top right
	jeLVertex	*V3,	// bottom right
	jeLVertex	*V4 )	// bottom left
{

	// locals
	jeUserPoly	*Poly;

	// ensure valid data
	assert( World != NULL );
	assert( V1 != NULL );
	assert( V2 != NULL );
	assert( V3 != NULL );
	assert( V4 != NULL );

	// draw poly
	Poly = jeUserPoly_CreateQuad( V1, V2, V3, V4, NULL, JE_RENDER_FLAG_ALPHA );
	jeWorld_AddUserPoly( World, Poly, JE_TRUE );
	jeUserPoly_Destroy( &Poly );

} // Util_DrawPoly()



////////////////////////////////////////////////////////////////////////////////////////
//
//	Util_DrawExtBox()
//
///////////////////////////////////////////////////////////////////////////////////////
static void Util_DrawExtBox(
	jeWorld		*World,		// world to draw it in
	JE_RGBA		*Color,		// color to draw it in
	jeExtBox	*ExtBox )	// extent box to draw
{

	// locals
	jeLVertex	Vertex[4];
	int			i;

	// ensure valid data
	assert( World != NULL );
	assert( Color != NULL );
	assert( ExtBox != NULL );

	// init vert struct
	for ( i = 0; i < 4; i++ )
	{
		Vertex[i].a = Color->a;
		Vertex[i].r = Color->r;
		Vertex[i].g = Color->g;
		Vertex[i].b = Color->b;
	}
	Vertex[0].u = 0.0f;
	Vertex[0].v = 0.0f;
	Vertex[1].u = 1.0f;
	Vertex[1].v = 0.0f;
	Vertex[2].u = 1.0f;
	Vertex[2].v = 1.0f;
	Vertex[3].u = 0.0f;
	Vertex[3].v = 1.0f;

	// side 1
	Vertex[0].X = ExtBox->Min.X;
	Vertex[0].Y = ExtBox->Max.Y;
	Vertex[0].Z = ExtBox->Min.Z;
	Vertex[1].X = ExtBox->Max.X;
	Vertex[1].Y = ExtBox->Max.Y;
	Vertex[1].Z = ExtBox->Min.Z;
	Vertex[2].X = ExtBox->Max.X;
	Vertex[2].Y = ExtBox->Min.Y;
	Vertex[2].Z = ExtBox->Min.Z;
	Vertex[3].X = ExtBox->Min.X;
	Vertex[3].Y = ExtBox->Min.Y;
	Vertex[3].Z = ExtBox->Min.Z;
	Util_DrawPoly( World, &( Vertex[0] ), &( Vertex[1] ), &( Vertex[2] ), &( Vertex[3] ) );

	// side 2
	Vertex[0].X = ExtBox->Min.X;
	Vertex[0].Y = ExtBox->Max.Y;
	Vertex[0].Z = ExtBox->Max.Z;
	Vertex[1].X = ExtBox->Max.X;
	Vertex[1].Y = ExtBox->Max.Y;
	Vertex[1].Z = ExtBox->Max.Z;
	Vertex[2].X = ExtBox->Max.X;
	Vertex[2].Y = ExtBox->Min.Y;
	Vertex[2].Z = ExtBox->Max.Z;
	Vertex[3].X = ExtBox->Min.X;
	Vertex[3].Y = ExtBox->Min.Y;
	Vertex[3].Z = ExtBox->Max.Z;
	Util_DrawPoly( World, &( Vertex[0] ), &( Vertex[1] ), &( Vertex[2] ), &( Vertex[3] ) );

	// side 3
	Vertex[0].X = ExtBox->Min.X;
	Vertex[0].Y = ExtBox->Max.Y;
	Vertex[0].Z = ExtBox->Min.Z;
	Vertex[1].X = ExtBox->Min.X;
	Vertex[1].Y = ExtBox->Max.Y;
	Vertex[1].Z = ExtBox->Max.Z;
	Vertex[2].X = ExtBox->Max.X;
	Vertex[2].Y = ExtBox->Max.Y;
	Vertex[2].Z = ExtBox->Max.Z;
	Vertex[3].X = ExtBox->Max.X;
	Vertex[3].Y = ExtBox->Max.Y;
	Vertex[3].Z = ExtBox->Min.Z;
	Util_DrawPoly( World, &( Vertex[0] ), &( Vertex[1] ), &( Vertex[2] ), &( Vertex[3] ) );

	// side 4
	Vertex[0].X = ExtBox->Max.X;
	Vertex[0].Y = ExtBox->Max.Y;
	Vertex[0].Z = ExtBox->Min.Z;
	Vertex[1].X = ExtBox->Max.X;
	Vertex[1].Y = ExtBox->Max.Y;
	Vertex[1].Z = ExtBox->Max.Z;
	Vertex[2].X = ExtBox->Max.X;
	Vertex[2].Y = ExtBox->Min.Y;
	Vertex[2].Z = ExtBox->Max.Z;
	Vertex[3].X = ExtBox->Max.X;
	Vertex[3].Y = ExtBox->Min.Y;
	Vertex[3].Z = ExtBox->Min.Z;
	Util_DrawPoly( World, &( Vertex[0] ), &( Vertex[1] ), &( Vertex[2] ), &( Vertex[3] ) );

	// side 5
	Vertex[0].X = ExtBox->Max.X;
	Vertex[0].Y = ExtBox->Min.Y;
	Vertex[0].Z = ExtBox->Min.Z;
	Vertex[1].X = ExtBox->Max.X;
	Vertex[1].Y = ExtBox->Min.Y;
	Vertex[1].Z = ExtBox->Max.Z;
	Vertex[2].X = ExtBox->Min.X;
	Vertex[2].Y = ExtBox->Min.Y;
	Vertex[2].Z = ExtBox->Max.Z;
	Vertex[3].X = ExtBox->Min.X;
	Vertex[3].Y = ExtBox->Min.Y;
	Vertex[3].Z = ExtBox->Min.Z;
	Util_DrawPoly( World, &( Vertex[0] ), &( Vertex[1] ), &( Vertex[2] ), &( Vertex[3] ) );

	// side 6
	Vertex[0].X = ExtBox->Min.X;
	Vertex[0].Y = ExtBox->Min.Y;
	Vertex[0].Z = ExtBox->Min.Z;
	Vertex[1].X = ExtBox->Min.X;
	Vertex[1].Y = ExtBox->Min.Y;
	Vertex[1].Z = ExtBox->Max.Z;
	Vertex[2].X = ExtBox->Min.X;
	Vertex[2].Y = ExtBox->Max.Y;
	Vertex[2].Z = ExtBox->Max.Z;
	Vertex[3].X = ExtBox->Min.X;
	Vertex[3].Y = ExtBox->Max.Y;
	Vertex[3].Z = ExtBox->Min.Z;
	Util_DrawPoly( World, &( Vertex[0] ), &( Vertex[1] ), &( Vertex[2] ), &( Vertex[3] ) );

} // Util_DrawExtBox()



////////////////////////////////////////////////////////////////////////////////////////
//
//	Spout_DestroyArt()
//
////////////////////////////////////////////////////////////////////////////////////////
static void Spout_DestroyArt(
	Spout	*Object )	// object whose bitmap will be destroyed
{
	// ensure valid data
	assert( Object != NULL );

	// destroy art
	if ( Object->Art != NULL )
	{
        jeBitmap* bmpArt;
		assert( Object->Engine != NULL );
		assert( Object->ResourceMgr != NULL );

        bmpArt = jeMaterialSpec_GetLayerBitmap(Object->Art, 0);
        if (bmpArt)
            jeEngine_RemoveBitmap( Object->Engine, bmpArt );

        jeMaterialSpec_Destroy(&Object->Art);
	}

} // Spout_DestroyArt()



////////////////////////////////////////////////////////////////////////////////////////
//
//	Util_WriteString()
//
////////////////////////////////////////////////////////////////////////////////////////
static jeBoolean Util_WriteString(
	jeVFile	*File,		// file to write to
	char	*String )	// string to write out
{

	// locals
	int			Size;
	jeBoolean	Result = JE_TRUE;

	// ensure valid data
	assert( File != NULL );
	assert( String != NULL );

	// write out complete
	Size = strlen( String ) + 1;
	assert( Size > 0 );
	Result &= jeVFile_Write( File, &Size, sizeof( Size ) );
	Result &= jeVFile_Write( File, String, Size );

	// all done
	return Result;

} // Util_WriteString()



////////////////////////////////////////////////////////////////////////////////////////
//
//	Util_CreateBitmapFromFileName()
//
//	Create a bitmap from a file.
//
////////////////////////////////////////////////////////////////////////////////////////
static jeBitmap * Util_CreateBitmapFromFileName(
	jeVFile		*File,			// file system to use
	const char	*Name,			// name of the file
	const char	*AlphaName )	// name of the alpha file
{

	// locals
	jeVFile		*BmpFile;
	jeBitmap	*Bmp;
	jeBoolean	Result;

	// ensure valid data
	assert( Name != NULL );

	// open the bitmap
	if ( File == NULL )
	{
		BmpFile = jeVFile_OpenNewSystem( NULL, JE_VFILE_TYPE_DOS, Name, NULL, JE_VFILE_OPEN_READONLY );
	}
	else
	{
		BmpFile = jeVFile_Open( File, Name, JE_VFILE_OPEN_READONLY );
	}
	if ( BmpFile == NULL )
	{
		jeErrorLog_Add( JE_ERR_FILEIO_OPEN, NULL );
		return NULL;
	}

	// create the bitmap
	Bmp = jeBitmap_CreateFromFile( BmpFile );
	jeVFile_Close( BmpFile );
	if ( Bmp == NULL )
	{
		jeErrorLog_Add( JE_ERR_SUBSYSTEM_FAILURE, NULL );
		return NULL;
	}

	// add alpha if required...
	if ( AlphaName != NULL )
	{

		// locals
		jeBitmap	*AlphaBmp;
		jeVFile		*AlphaFile;

		// open alpha file
		if ( File == NULL )
		{
			AlphaFile = jeVFile_OpenNewSystem( NULL, JE_VFILE_TYPE_DOS, AlphaName, NULL, JE_VFILE_OPEN_READONLY );
		}
		else
		{
			AlphaFile = jeVFile_Open( File, AlphaName, JE_VFILE_OPEN_READONLY );
		}
		if( AlphaFile == NULL )
		{
			jeErrorLog_Add( JE_ERR_FILEIO_OPEN, NULL );
			jeBitmap_Destroy( &Bmp );
			return NULL;
		}

		// create alpha bitmap
		AlphaBmp = jeBitmap_CreateFromFile( AlphaFile );
		jeVFile_Close( AlphaFile );
		if ( AlphaBmp == NULL )
		{
			jeErrorLog_Add( JE_ERR_SUBSYSTEM_FAILURE, NULL );
			jeBitmap_Destroy( &Bmp );
			return NULL;
		}

		// fail if alpha isn't same size as main bitmap
		if (	( jeBitmap_Width( Bmp ) != jeBitmap_Width( AlphaBmp ) ) ||
				( jeBitmap_Height( Bmp ) != jeBitmap_Height( AlphaBmp ) ) )
		{
			jeErrorLog_Add( JE_ERR_BAD_PARAMETER, NULL );
			jeBitmap_Destroy( &AlphaBmp );
			jeBitmap_Destroy( &Bmp );
			return NULL;
		}

		// set its alpha
		Result = jeBitmap_SetAlpha( Bmp, AlphaBmp );
		if ( Result == JE_FALSE )
		{
			jeErrorLog_Add( JE_ERR_SUBSYSTEM_FAILURE, NULL );
			jeBitmap_Destroy( &AlphaBmp );
			jeBitmap_Destroy( &Bmp );
			return NULL;
		}

		// don't need the alpha anymore
		jeBitmap_Destroy( &AlphaBmp );
	}
	// ...or just set the color key
	else
	{
		Result = jeBitmap_SetColorKey( Bmp, JE_TRUE, 255, JE_FALSE );
		assert( Result );
	}

	// all done
	return Bmp;

} // Util_CreateBitmapFromFileName()



////////////////////////////////////////////////////////////////////////////////////////
//
//	Util_DestroyBitmapList()
//
////////////////////////////////////////////////////////////////////////////////////////
static void Util_DestroyBitmapList(
	BitmapList	**DeadList )	// list to destroy
{

	// locals
	BitmapList	*List;
	int			i;

	// ensure valid data
	assert( DeadList != NULL );
	assert( *DeadList != NULL );

	// get list pointer
	List = *DeadList;

	// destroy file list
	if ( List->Name != NULL )
	{
		assert( List->Total > 0 );
		for ( i = 0; i < List->Total; i++ )
		{
			if ( List->Name[i] != NULL )
			{
				jeRam_Free( List->Name[i] );
			}
		}
		jeRam_Free( List->Name );
	}

	// destroy width and height lists
	if ( List->Width != NULL )
	{
		jeRam_Free( List->Width );
	}
	if ( List->Height != NULL )
	{
		jeRam_Free( List->Height );
	}

	// destroy numeric sizes list
	if ( List->NumericSizes != NULL )
	{
		jeRam_Free( List->NumericSizes );
	}

	// destroy string sizes list
	if ( List->StringSizes != NULL )
	{
		for ( i = 0; i < List->SizesListSize; i++ )
		{
			assert( List->StringSizes[i] != NULL );
			jeRam_Free( List->StringSizes[i] );
		}
	}

	// free bitmaplist struct
	jeRam_Free( List );

	// zap pointer
	*DeadList = NULL;

} // Util_DestroyBitmapList()



////////////////////////////////////////////////////////////////////////////////////////
//
//	Util_CreateBitmapList()
//
////////////////////////////////////////////////////////////////////////////////////////
static BitmapList * Util_CreateBitmapList(
	jeResourceMgr	*ResourceMgr,	// resource manager to use
	char			*ResourceName,	// name of resource
	char			*FileFilter )	// file filter
{

	// locals
	BitmapList		*Bmps = NULL;
	jeVFile			*FileDir = NULL;
	jeVFile_Finder	*Finder = NULL;
	int				CurFile;

	// ensure valid data
	assert( ResourceMgr != NULL );
	assert( ResourceName != NULL );
	assert( FileFilter != NULL );

	// allocate bitmaplist struct
	Bmps = (BitmapList *)jeRam_AllocateClear( sizeof( *Bmps ) );
	if ( Bmps == NULL )
	{
		jeErrorLog_Add( JE_ERR_MEMORY_RESOURCE, NULL );
		return NULL;
	}

	// get vfile dir
	FileDir = jeResource_GetVFile( ResourceMgr, ResourceName );
	if ( FileDir == NULL )
	{
		jeErrorLog_Add( JE_ERR_SUBSYSTEM_FAILURE, NULL );
		goto ERROR_Util_BuildBitmapList;
	}

	// create directory finder
	Finder = jeVFile_CreateFinder( FileDir, FileFilter );
	if ( Finder == NULL )
	{
		jeErrorLog_Add( JE_ERR_SUBSYSTEM_FAILURE, NULL );
		goto ERROR_Util_BuildBitmapList;
	}

	// determine how many files there are
	Bmps->Total = 1;
	while ( jeVFile_FinderGetNextFile( Finder ) == JE_TRUE )
	{
		Bmps->Total++;
	}

	// destroy finder
	jeVFile_DestroyFinder( Finder );
	Finder = NULL;

	// allocate name list
	Bmps->Name = (char **)jeRam_AllocateClear( sizeof( char * ) * Bmps->Total );
	if ( Bmps->Name == NULL )
	{
		jeErrorLog_Add( JE_ERR_MEMORY_RESOURCE, NULL );
		goto ERROR_Util_BuildBitmapList;
	}

	// allocate width list
	Bmps->Width = (int *)jeRam_AllocateClear( sizeof( int * ) * Bmps->Total );
	if ( Bmps->Width == NULL )
	{
		jeErrorLog_Add( JE_ERR_MEMORY_RESOURCE, NULL );
		goto ERROR_Util_BuildBitmapList;
	}

	// allocate height list
	Bmps->Height = (int *)jeRam_AllocateClear( sizeof( int * ) * Bmps->Total );
	if ( Bmps->Height == NULL )
	{
		jeErrorLog_Add( JE_ERR_MEMORY_RESOURCE, NULL );
		goto ERROR_Util_BuildBitmapList;
	}

	// allocate numeric sizes list
	Bmps->NumericSizes = (int *)jeRam_AllocateClear( sizeof( int * ) * Bmps->Total );
	if ( Bmps->NumericSizes == NULL )
	{
		jeErrorLog_Add( JE_ERR_MEMORY_RESOURCE, NULL );
		goto ERROR_Util_BuildBitmapList;
	}

	// allocate string sizes list
	Bmps->StringSizes = (char **)jeRam_AllocateClear( sizeof( char * ) * Bmps->Total );
	if ( Bmps->StringSizes == NULL )
	{
		jeErrorLog_Add( JE_ERR_MEMORY_RESOURCE, NULL );
		goto ERROR_Util_BuildBitmapList;
	}

	// create directory finder
	Finder = jeVFile_CreateFinder( FileDir, FileFilter );
	if ( Finder == NULL )
	{
		jeErrorLog_Add( JE_ERR_SUBSYSTEM_FAILURE, NULL );
		goto ERROR_Util_BuildBitmapList;
	}

	// first entry is always the "no selection" slot
	CurFile = 0;
	Bmps->Name[CurFile++] = Util_StrDup( NoSelection );

	// build file list
	while ( jeVFile_FinderGetNextFile( Finder ) == JE_TRUE )
	{

		// locals
		jeVFile_Properties	Properties;
		jeBitmap			*Bitmap;

		// get properties of current file
		if( jeVFile_FinderGetProperties( Finder, &Properties ) == JE_FALSE )
		{
			jeErrorLog_Add( JE_ERR_SUBSYSTEM_FAILURE, NULL );
			goto ERROR_Util_BuildBitmapList;
		}

		// save file name
		assert( CurFile < Bmps->Total );
		Bmps->Name[CurFile] = Util_StrDup( Properties.Name );
		if ( Bmps->Name[CurFile] == NULL )
		{
			jeErrorLog_Add( JE_ERR_SUBSYSTEM_FAILURE, NULL );
			goto ERROR_Util_BuildBitmapList;
		}

		// save width and height
		Bitmap = Util_CreateBitmapFromFileName( FileDir, Bmps->Name[CurFile], NULL );
		if ( Bitmap == NULL )
		{
			jeErrorLog_Add( JE_ERR_SUBSYSTEM_FAILURE, NULL );
			goto ERROR_Util_BuildBitmapList;
		}
		Bmps->Width[CurFile] = jeBitmap_Width( Bitmap );
		Bmps->Height[CurFile] = jeBitmap_Height( Bitmap );
		jeBitmap_Destroy( &Bitmap );

		// add sise to numeric sizes list
		{

			// locals
			jeBoolean	AddIt;
			int			i;

			// check if it needs to be added to numeric sizes list
			AddIt = JE_TRUE;
			for ( i = 0; i < Bmps->Total; i++ )
			{
				if ( Bmps->Width[CurFile] == Bmps->NumericSizes[i] )
				{
					AddIt = JE_FALSE;
					break;
				}
			}

			// add it if required
			if ( AddIt == JE_TRUE )
			{
				for ( i = 0; i < Bmps->Total; i++ )
				{
					if (	( Bmps->Width[CurFile] < Bmps->NumericSizes[i] ) ||
							( Bmps->NumericSizes[i] == 0 ) )
					{
						int	Hold1, Hold2;
						Hold1 = Bmps->Width[CurFile];
						for ( ; i < Bmps->Total; i++ )
						{
							Hold2 = Bmps->NumericSizes[i];
							Bmps->NumericSizes[i] = Hold1;
							Hold1 = Hold2;
						}
						AddIt = JE_FALSE;
						break;
					}
				}
			}
			assert( AddIt == JE_FALSE );
		}

		// adjust file counter
		CurFile++;
	}

	// destroy finder
	jeVFile_DestroyFinder( Finder );

	// close vfile dir
	if ( jeResource_DeleteVFile( ResourceMgr, ResourceName ) == 0 )
	{
		jeVFile_Close( FileDir );
	}

	// create string sizes list
	{

		// locals
		int		i;
		char	Buf[256];

		// build list
		Bmps->SizesListSize = Bmps->Total;
		for ( i = 0; i < Bmps->Total; i++ )
		{
			if ( Bmps->NumericSizes[i] == 0 )
			{
				Bmps->SizesListSize = i;
				break;
			}
			itoa( Bmps->NumericSizes[i], Buf, 10 );
			Bmps->StringSizes[i] = Util_StrDup( Buf );
		}
	}

	// return bitmaplist struct
	return Bmps;

	// error handling
	ERROR_Util_BuildBitmapList:

	// destroy bitmap list
	assert( Bmps != NULL );
	Util_DestroyBitmapList( &Bmps );

	// destroy finder
	if ( Finder != NULL )
	{
		jeVFile_DestroyFinder( Finder );
	}

	// close vfile dir
	if ( FileDir != NULL )
	{
		if ( jeResource_DeleteVFile( ResourceMgr, ResourceName ) == 0 )
		{
			jeVFile_Close( FileDir );
		}
	}

	// return failure
	return NULL;

} // Util_CreateBitmapList()



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


////////////////////////////////////////////////////////////////////////////////////////
//
//	Util_SetArtSize()
//
//	Sets up the art size combo box bases on a bitmap and alpha.
//
////////////////////////////////////////////////////////////////////////////////////////
void Util_SetArtSize(
  	Spout	*Object )	// object to base combo box off
{

	// locals
	int	i;
	int	BitmapSlot, AlphaSlot;

	// ensure valid data
	assert( Object != NULL );

	// determine bitmap and alpha slots
	BitmapSlot = - 1;
	AlphaSlot = -1;
	for ( i = 1; i < Bitmaps->Total; i++ )
	{

		// get bitmap slot
		if ( ( Object->BitmapName != NULL ) && ( BitmapSlot == -1 ) )
		{
			if ( stricmp( Object->BitmapName, Bitmaps->Name[i] ) == 0 )
			{
				BitmapSlot = i;
			}
		}

		// get alpha slot
		if ( ( Object->AlphaName != NULL ) && ( AlphaSlot == -1 ) )
		{
			if ( stricmp( Object->AlphaName, Bitmaps->Name[i] ) == 0 )
			{
				AlphaSlot = i;
			}
		}
	}

	// ensure non bogosity
	if ( ( BitmapSlot != -1 ) && ( AlphaSlot != -1 ) )
	{
		assert( Bitmaps->Width[BitmapSlot] == Bitmaps->Width[AlphaSlot] );
	}

	// adjust curwidth value
	if ( BitmapSlot != -1 )
	{
		for ( i = 0; i < Bitmaps->SizesListSize; i++ )
		{
			if ( Bitmaps->NumericSizes[i] == Bitmaps->Width[BitmapSlot] )
			{
				Object->CurWidth = i;
				break;
			}
		}
	}

	// recreate CurBitmaps list
	Util_RecreateCurBitmapsList( Object );

} // Util_SetArtSize()



////////////////////////////////////////////////////////////////////////////////////////
//
//	Util_Frand()
//
//	Picks a random float within the supplied range.
//
////////////////////////////////////////////////////////////////////////////////////////
static float Util_Frand(
	float Low,		// minimum value
	float High )	// maximum value
{

	// locals
	float	Range;

	// ensure valid data
	assert( High >= Low );

	// if they are the same then just return one of them
	if ( High == Low )
	{
		return Low;
	}

	// pick a random float from whithin the range
	Range = High - Low;
	return ( (float)( ( ( rand() % 1000 ) + 1 ) ) ) / 1000.0f * Range + Low;

} // Util_Frand()



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
	image_id 	hInstance ) // dll instance handle
#endif
{

	// ensure valid data
	assert( hInstance != NULL );

	// save hinstance
	hClassInstance = hInstance;

	// setup rate property
	jeProperty_FillFloat(	&( SpoutProperties[SPOUT_RATE_INDEX] ),
							Util_LoadLibraryString( hClassInstance, IDS_RATE ),
							SPOUT_DEFAULT_RATE,
							SPOUT_RATE_ID,
							0.01f, 1.0f, 0.01f );

	// setup angle property
	jeProperty_FillFloat(	&( SpoutProperties[SPOUT_ANGLE_INDEX] ),
							Util_LoadLibraryString( hClassInstance, IDS_ANGLE ),
							SPOUT_DEFAULT_ANGLE,
							SPOUT_ANGLE_ID,
							0.0f, 3.14f, 0.1f );

	// setup min speed property
	jeProperty_FillFloat(	&( SpoutProperties[SPOUT_MINSPEED_INDEX] ),
							Util_LoadLibraryString( hClassInstance, IDS_MINSPEED ),
							SPOUT_DEFAULT_MINSPEED,
							SPOUT_MINSPEED_ID,
							0.0f, FLT_MAX, 2.0f );

	// setup max speed property
	jeProperty_FillFloat(	&( SpoutProperties[SPOUT_MAXSPEED_INDEX] ),
							Util_LoadLibraryString( hClassInstance, IDS_MAXSPEED ),
							SPOUT_DEFAULT_MAXSPEED,
							SPOUT_MAXSPEED_ID,
							0.0f, FLT_MAX, 2.0f );

	// setup min scale property
	jeProperty_FillFloat(	&( SpoutProperties[SPOUT_MINSCALE_INDEX] ),
							Util_LoadLibraryString( hClassInstance, IDS_MINSCALE ),
							SPOUT_DEFAULT_MINSCALE,
							SPOUT_MINSCALE_ID,
							0.01f, FLT_MAX, 0.1f );

	// setup max scale property
	jeProperty_FillFloat(	&( SpoutProperties[SPOUT_MAXSCALE_INDEX] ),
							Util_LoadLibraryString( hClassInstance, IDS_MAXSCALE ),
							SPOUT_DEFAULT_MAXSCALE,
							SPOUT_MAXSCALE_ID,
							0.01f, FLT_MAX, 0.1f );

	// setup min unit life property
	jeProperty_FillFloat(	&( SpoutProperties[SPOUT_MINUNITLIFE_INDEX] ),
							Util_LoadLibraryString( hClassInstance, IDS_MINUNITLIFE ),
							SPOUT_DEFAULT_MINUNITLIFE,
							SPOUT_MINUNITLIFE_ID,
							0.1f, FLT_MAX, 0.1f );

	// setup max unit life property
	jeProperty_FillFloat(	&( SpoutProperties[SPOUT_MAXUNITLIFE_INDEX] ),
							Util_LoadLibraryString( hClassInstance, IDS_MAXUNITLIFE ),
							SPOUT_DEFAULT_MAXUNITLIFE,
							SPOUT_MAXUNITLIFE_ID,
							0.1f, FLT_MAX, 0.1f );


	////////////////////////////////////////////////////////////////////////////////////////
	//	Min color properties
	////////////////////////////////////////////////////////////////////////////////////////

	// start min color group
	{
		jeVec3d	Color = { SPOUT_DEFAULT_COLORRED, SPOUT_DEFAULT_COLORGREEN, SPOUT_DEFAULT_COLORBLUE };
		jeProperty_FillColorGroup(	&( SpoutProperties[SPOUT_COLORMINGROUP_INDEX] ),
									Util_LoadLibraryString( hClassInstance, IDS_COLORMINGROUP ),
									&Color,
									SPOUT_COLORMINGROUP_INDEX );
	}

	// setup min color picker property
	{
		jeVec3d	Color = { SPOUT_DEFAULT_COLORRED, SPOUT_DEFAULT_COLORGREEN, SPOUT_DEFAULT_COLORBLUE };
		jeProperty_FillColorPicker(	&( SpoutProperties[SPOUT_COLORMIN_INDEX] ),
									Util_LoadLibraryString( hClassInstance, IDS_COLORMIN ),
									&Color,
									SPOUT_COLORMIN_ID );
	}

	// setup min color red property
	jeProperty_FillFloat(	&( SpoutProperties[SPOUT_COLORMINRED_INDEX] ),
							Util_LoadLibraryString( hClassInstance, IDS_COLORMINRED ),
							SPOUT_DEFAULT_COLORRED,
							SPOUT_COLORMINRED_ID,
							0.0f, 255.0f, 1.0f );

	// setup min color green property
	jeProperty_FillFloat(	&( SpoutProperties[SPOUT_COLORMINGREEN_INDEX] ),
							Util_LoadLibraryString( hClassInstance, IDS_COLORMINGREEN ),
							SPOUT_DEFAULT_COLORGREEN,
							SPOUT_COLORMINGREEN_ID,
							0.0f, 255.0f, 1.0f );

	// setup min color blue property
	jeProperty_FillFloat(	&( SpoutProperties[SPOUT_COLORMINBLUE_INDEX] ),
							Util_LoadLibraryString( hClassInstance, IDS_COLORMINBLUE ),
							SPOUT_DEFAULT_COLORBLUE,
							SPOUT_COLORMINBLUE_ID,
							0.0f, 255.0f, 1.0f );

	// end min color group
	jeProperty_FillGroupEnd( &( SpoutProperties[SPOUT_COLORMINGROUPEND_INDEX] ), SPOUT_COLORMINGROUPEND_INDEX );


	////////////////////////////////////////////////////////////////////////////////////////
	//	Max color properties
	////////////////////////////////////////////////////////////////////////////////////////

	// start max color group
	{
		jeVec3d	Color = { SPOUT_DEFAULT_COLORRED, SPOUT_DEFAULT_COLORGREEN, SPOUT_DEFAULT_COLORBLUE };
		jeProperty_FillColorGroup(	&( SpoutProperties[SPOUT_COLORMAXGROUP_INDEX] ),
									Util_LoadLibraryString( hClassInstance, IDS_COLORMAXGROUP ),
									&Color,
									SPOUT_COLORMAXGROUP_INDEX );
	}

	// setup max color picker property
	{
		jeVec3d	Color = { SPOUT_DEFAULT_COLORRED, SPOUT_DEFAULT_COLORGREEN, SPOUT_DEFAULT_COLORBLUE };
		jeProperty_FillColorPicker(	&( SpoutProperties[SPOUT_COLORMAX_INDEX] ),
									Util_LoadLibraryString( hClassInstance, IDS_COLORMAX ),
									&Color,
									SPOUT_COLORMAX_ID );
	}

	// setup max color red property
	jeProperty_FillFloat(	&( SpoutProperties[SPOUT_COLORMAXRED_INDEX] ),
							Util_LoadLibraryString( hClassInstance, IDS_COLORMAXRED ),
							SPOUT_DEFAULT_COLORRED,
							SPOUT_COLORMAXRED_ID,
							0.0f, 255.0f, 1.0f );

	// setup max color green property
	jeProperty_FillFloat(	&( SpoutProperties[SPOUT_COLORMAXGREEN_INDEX] ),
							Util_LoadLibraryString( hClassInstance, IDS_COLORMAXGREEN ),
							SPOUT_DEFAULT_COLORGREEN,
							SPOUT_COLORMAXGREEN_ID,
							0.0f, 255.0f, 1.0f );

	// setup max color blue property
	jeProperty_FillFloat(	&( SpoutProperties[SPOUT_COLORMAXBLUE_INDEX] ),
							Util_LoadLibraryString( hClassInstance, IDS_COLORMAXBLUE ),
							SPOUT_DEFAULT_COLORBLUE,
							SPOUT_COLORMAXBLUE_ID,
							0.0f, 255.0f, 1.0f );

	// end max color group
	jeProperty_FillGroupEnd( &( SpoutProperties[SPOUT_COLORMAXGROUPEND_INDEX] ), SPOUT_COLORMAXGROUPEND_INDEX );


	////////////////////////////////////////////////////////////////////////////////////////
	//	Draw box properties
	////////////////////////////////////////////////////////////////////////////////////////

	// start draw box group
	jeProperty_FillGroup(	&( SpoutProperties[SPOUT_DRAWEXTBOXGROUP_INDEX] ),
							Util_LoadLibraryString( hClassInstance, IDS_DRAWEXTBOXGROUP ),
							SPOUT_DRAWEXTBOXGROUP_ID );

	// setup draw box display property
	jeProperty_FillCheck(	&( SpoutProperties[SPOUT_DRAWEXTBOXDISPLAY_INDEX] ),
							Util_LoadLibraryString( hClassInstance, IDS_DRAWEXTBOXDISPLAY ),
							SPOUT_DEFAULT_DRAWEXTBOXDISPLAY,
							SPOUT_DRAWEXTBOXDISPLAY_ID );

	// setup draw box adjustment properties
	jeProperty_FillFloat(	&( SpoutProperties[SPOUT_DRAWEXTBOXMINX_INDEX] ),
							Util_LoadLibraryString( hClassInstance, IDS_DRAWEXTBOXMINX ),
							SPOUT_DEFAULT_DRAWEXTBOXMINX,
							SPOUT_DRAWEXTBOXMINX_ID,
							-FLT_MAX, FLT_MAX, 8.0f );
	jeProperty_FillFloat(	&( SpoutProperties[SPOUT_DRAWEXTBOXMINY_INDEX] ),
							Util_LoadLibraryString( hClassInstance, IDS_DRAWEXTBOXMINY ),
							SPOUT_DEFAULT_DRAWEXTBOXMINY,
							SPOUT_DRAWEXTBOXMINY_ID,
							-FLT_MAX, FLT_MAX, 8.0f );
	jeProperty_FillFloat(	&( SpoutProperties[SPOUT_DRAWEXTBOXMINZ_INDEX] ),
							Util_LoadLibraryString( hClassInstance, IDS_DRAWEXTBOXMINZ ),
							SPOUT_DEFAULT_DRAWEXTBOXMINZ,
							SPOUT_DRAWEXTBOXMINZ_ID,
							-FLT_MAX, FLT_MAX, 8.0f );
	jeProperty_FillFloat(	&( SpoutProperties[SPOUT_DRAWEXTBOXMAXX_INDEX] ),
							Util_LoadLibraryString( hClassInstance, IDS_DRAWEXTBOXMAXX ),
							SPOUT_DEFAULT_DRAWEXTBOXMAXX,
							SPOUT_DRAWEXTBOXMAXX_ID,
							-FLT_MAX, FLT_MAX, 8.0f );
	jeProperty_FillFloat(	&( SpoutProperties[SPOUT_DRAWEXTBOXMAXY_INDEX] ),
							Util_LoadLibraryString( hClassInstance, IDS_DRAWEXTBOXMAXY ),
							SPOUT_DEFAULT_DRAWEXTBOXMAXY,
							SPOUT_DRAWEXTBOXMAXY_ID,
							-FLT_MAX, FLT_MAX, 8.0f );
	jeProperty_FillFloat(	&( SpoutProperties[SPOUT_DRAWEXTBOXMAXZ_INDEX] ),
							Util_LoadLibraryString( hClassInstance, IDS_DRAWEXTBOXMAXZ ),
							SPOUT_DEFAULT_DRAWEXTBOXMAXZ,
							SPOUT_DRAWEXTBOXMAXZ_ID,
							-FLT_MAX, FLT_MAX, 8.0f );

	// end draw box group
	jeProperty_FillGroupEnd( &( SpoutProperties[SPOUT_DRAWEXTBOXGROUPEND_INDEX] ), SPOUT_DRAWEXTBOXGROUPEND_ID );


	////////////////////////////////////////////////////////////////////////////////////////
	//	Art properties
	////////////////////////////////////////////////////////////////////////////////////////

	// start draw box group
	jeProperty_FillGroup(	&( SpoutProperties[SPOUT_ARTGROUP_INDEX] ),
							Util_LoadLibraryString( hClassInstance, IDS_ARTGROUP ),
							SPOUT_ARTGROUP_ID );

	// end draw box group
	jeProperty_FillGroupEnd( &( SpoutProperties[SPOUT_ARTGROUPEND_INDEX] ), SPOUT_ARTGROUPEND_ID );


	////////////////////////////////////////////////////////////////////////////////////////
	//	Misc properties
	////////////////////////////////////////////////////////////////////////////////////////

	// final init
	SpoutPropertyList.jePropertyN = SPOUT_LAST_INDEX;

} // Init_Class()



////////////////////////////////////////////////////////////////////////////////////////
//
//	DeInit_Class()
//
////////////////////////////////////////////////////////////////////////////////////////
void DeInit_Class(
	void )	// no parameters
{

	// free bitmap list
	if ( Bitmaps != NULL )
	{
		Util_DestroyBitmapList( &Bitmaps );
	}

	// free current bitmaps list
	if ( CurBitmaps.Name != NULL )
	{

		// locals
		int	i;

		// free list
		for ( i = 0; i < CurBitmaps.Total; i++ )
		{
			if ( CurBitmaps.Name[i] != NULL )
			{
				jeRam_Free( CurBitmaps.Name[i] );
			}
		}
		jeRam_Free( CurBitmaps.Name );
	}

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
	Spout	*Object;

	// allocate struct
	Object = (Spout *)jeRam_AllocateClear( sizeof( *Object ) );
	if ( Object == NULL )
	{
		jeErrorLog_Add( JE_ERR_MEMORY_RESOURCE, NULL );
		return NULL;
	}

	// create particle system
	Object->Ps = jeParticle_SystemCreate( JET_MAJOR_VERSION, JET_MINOR_VERSION );
	if ( Object->Ps == NULL )
	{
		jeErrorLog_Add( JE_ERR_SUBSYSTEM_FAILURE, NULL );
		jeRam_Free( Object );
		return NULL;
	}

	// init remaining fields
	jeXForm3d_SetIdentity( &Object->Xf );
	Object->RefCount = 1;
	Object->Rate = SPOUT_DEFAULT_RATE;
	Object->Angle = SPOUT_DEFAULT_ANGLE;
	Object->MinScale = SPOUT_DEFAULT_MINSCALE;
	Object->MaxScale = SPOUT_DEFAULT_MAXSCALE;
	Object->MinSpeed = SPOUT_DEFAULT_MINSPEED;
	Object->MaxSpeed = SPOUT_DEFAULT_MAXSPEED;
	Object->MinUnitLife = SPOUT_DEFAULT_MINUNITLIFE;
	Object->MaxUnitLife = SPOUT_DEFAULT_MAXUNITLIFE;
	Object->MinColor.r = Object->MaxColor.r = SPOUT_DEFAULT_COLORRED;
	Object->MinColor.g = Object->MaxColor.g = SPOUT_DEFAULT_COLORGREEN;
	Object->MinColor.b = Object->MaxColor.b = SPOUT_DEFAULT_COLORBLUE;
	Object->MinColor.a = Object->MaxColor.a = 255.0f;
	Object->DrawExtBoxDisplay = SPOUT_DEFAULT_DRAWEXTBOXDISPLAY;
	Object->DrawExtBox.Min.X = SPOUT_DEFAULT_DRAWEXTBOXMINX;
	Object->DrawExtBox.Min.Y = SPOUT_DEFAULT_DRAWEXTBOXMINY;
	Object->DrawExtBox.Min.Z = SPOUT_DEFAULT_DRAWEXTBOXMINZ;
	Object->DrawExtBox.Max.X = SPOUT_DEFAULT_DRAWEXTBOXMAXX;
	Object->DrawExtBox.Max.Y = SPOUT_DEFAULT_DRAWEXTBOXMAXY;
	Object->DrawExtBox.Max.Z = SPOUT_DEFAULT_DRAWEXTBOXMAXZ;

	// setup art size property
	jeProperty_FillCombo(	&( SpoutPropertyList.pjeProperty[SPOUT_ARTSIZE_INDEX] ),
							Util_LoadLibraryString( hClassInstance, IDS_ARTSIZE ),
							0,
							SPOUT_ARTSIZE_ID,
							0,
							NULL );

	// setup bitmap list property
	Object->BitmapName = Util_StrDup( NoSelection );
	jeProperty_FillCombo(	&( SpoutPropertyList.pjeProperty[SPOUT_BITMAPLIST_INDEX] ),
							Util_LoadLibraryString( hClassInstance, IDS_BITMAPLIST ),
							NoSelection,
							SPOUT_BITMAPLIST_ID,
							1,
							&NoSelection );

	// setup alpha list property
	Object->AlphaName = Util_StrDup( NoSelection );
	jeProperty_FillCombo(	&( SpoutPropertyList.pjeProperty[SPOUT_ALPHALIST_INDEX] ),
							Util_LoadLibraryString( hClassInstance, IDS_ALPHALIST ),
							NoSelection,
							SPOUT_ALPHALIST_ID,
							1,
							&NoSelection );

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
	Spout	*Object;
	
	// get object
	Object = (Spout *)Instance;
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
	Spout	*Object;

	// ensure valid data
	assert( Instance != NULL );

	// get object
	Object = (Spout *)*Instance;
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
	assert( Object->Ps == NULL );
	assert( Object->Art == NULL );
	assert( Object->ArtName == NULL );
	//assert( Object->SizeName == NULL );
	assert( Object->BitmapName == NULL );
	assert( Object->AlphaName == NULL );

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
	jeObject_RenderFlags	RenderFlags)	
{

	// locals
	Spout	*Object;

	// ensure valid data
	assert( Instance != NULL );

	// get object
	Object = (Spout *)Instance;

	// display draw ext box
	if ( Object->DrawExtBoxDisplay == JE_TRUE )
	{

		// locals
		JE_RGBA		Color = { 255.0f, 0.0f, 0.0f, 64.0f };
		jeExtBox	ExtBox;

		// copy ext box and translate it
		ExtBox = Object->DrawExtBox;
		jeExtBox_Translate( &ExtBox, Object->Xf.Translation.X, Object->Xf.Translation.Y, Object->Xf.Translation.Z );

		// draw it
		Util_DrawExtBox( (jeWorld *)World, &Color, &ExtBox );
	}

	// all done
	return JE_TRUE;

	// eliminate warnings
	Engine;
	Camera;
	CameraSpaceFrustum;

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
	Spout	*Object;

	// ensure valid data
	assert( Instance != NULL );
	assert( World != NULL );

	// get object data
	Object = (Spout *)Instance;

	// save world pointer
	Object->World = World;

	// save an instance of the resource manager
	Object->ResourceMgr = jeWorld_GetResourceMgr( World );
	assert( Object->ResourceMgr != NULL );

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

		// init current list
		CurBitmaps.Name = (char **)jeRam_AllocateClear( sizeof( char * ) * Bitmaps->Total );
		CurBitmaps.Total = 0;
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
	Spout	*Object;

	// ensure valid data
	assert( Instance != NULL );
	assert( World != NULL );

	// get object data
	Object = (Spout *)Instance;
	assert( Object->World == World );

	// destroy its particle system
	if ( Object->Ps != NULL )
	{
		jeParticle_SystemDestroy( Object->Ps );
		Object->Ps = NULL;
	}

	// destroy our instance of the resource manager
	jeResource_MgrDestroy( &( Object->ResourceMgr ) );

	// zap world pointer
	Object->World = NULL;

	// all done
	return JE_TRUE;

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
	Spout	*Object;

	// ensure valid data
	assert( Instance != NULL );
	assert( Engine != NULL );

	// get object data
	Object = (Spout *)Instance;

	// save engine pointer
	Object->Engine = Engine;

	// setup art size combo box based on bitmap and alpha names
	Util_SetArtSize( Object );

	// set properties if object was loaded from disk
	if ( Object->LoadedFromDisk == JE_TRUE )
	{

		// reset loaded from disk flag
		Object->LoadedFromDisk = JE_FALSE;

		// set bitmap name property
		if ( Object->BitmapName != NULL )
		{

			// locals
			jeProperty_Data	Data;
			jeBoolean		Result;

			// set property
			Data.String = Util_StrDup( Object->BitmapName );
			Result = SetProperty( Object, SPOUT_BITMAPLIST_ID, PROPERTY_COMBO_TYPE, &Data );
			jeRam_Free( Data.String );
		}

		// set alpha name property
		if ( Object->AlphaName != NULL )
		{

			// locals
			jeProperty_Data	Data;
			jeBoolean		Result;

			// set property
			Data.String = Util_StrDup( Object->AlphaName );
			Result = SetProperty( Object, SPOUT_ALPHALIST_ID, PROPERTY_COMBO_TYPE, &Data );
			jeRam_Free( Data.String );
		}
	}

	// flag property list as dirty
	SpoutPropertyList.bDirty = JE_TRUE;

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
	Spout	*Object;

	// ensure valid data
	assert( Instance != NULL );
	assert( Engine != NULL );

	// get object data
	Object = (Spout *)Instance;
	assert( Object->Engine == Engine );

	// destroy object bitmap
	Spout_DestroyArt( Object );

	// destroy bitmap names
	if ( Object->AlphaName != NULL )
	{
		jeRam_Free( Object->AlphaName );
		Object->AlphaName = NULL;
	}
	if ( Object->BitmapName != NULL )
	{
		jeRam_Free( Object->BitmapName );
		Object->BitmapName = NULL;
	}
	if ( Object->ArtName != NULL )
	{
		jeRam_Free( Object->ArtName );
		Object->ArtName = NULL;
	}

	// zap engine pointer
	Object->Engine = NULL;

	// all done
	return JE_TRUE;

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
	//assert( Impact != NULL ); Removed by Icestorm. They CAN be NULL.
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
	Spout		*Object;

	// ensure valid data
	assert( Instance != NULL );
	assert( BBox != NULL );

	// get object data
	Object = (Spout *)Instance;

	// save extent box
	*BBox = Object->DrawExtBox;
	jeExtBox_Translate( BBox, Object->Xf.Translation.X, Object->Xf.Translation.Y, Object->Xf.Translation.Z );

	// all done
	return JE_TRUE;

} // GetExtBox()



////////////////////////////////////////////////////////////////////////////////////////
//
//	CreateFromFile()
//
///////////////////////////////////////////////////////////////////////////////////////
#if NEWLOAD_SPT
void * JETCC CreateFromFile(
	jeVFile		*File,		// vfile to use
	jeNameMgr *NM )	// pointer manager
{

	// locals
	Spout		*Object;
	int			Size;
	jeBoolean	Result = JE_TRUE;

	// ensure valid data
	assert( File != NULL );

	// create new object
	Object = CreateInstance();
	if ( Object == NULL )
	{
		return NULL;
	}

	// read art name
	Result &= jeVFile_Read( File, &( Size ), sizeof( Size ) );
	if ( ( Size > 0 ) && ( Result == JE_TRUE ) )
	{
		Object->ArtName = jeRam_Allocate( Size );
		if ( Object->ArtName == NULL )
		{
			jeErrorLog_Add( JE_ERR_MEMORY_RESOURCE, NULL );
			goto ERROR_CreateFromFile;
		}
		Result &= jeVFile_Read( File, Object->ArtName, Size );
	}

	// read bitmap name
	Result &= jeVFile_Read( File, &( Size ), sizeof( Size ) );
	if ( ( Size > 0 ) && ( Result == JE_TRUE ) )
	{
		Object->BitmapName = jeRam_Allocate( Size );
		if ( Object->BitmapName == NULL )
		{
			jeErrorLog_Add( JE_ERR_MEMORY_RESOURCE, NULL );
			goto ERROR_CreateFromFile;
		}
		Result &= jeVFile_Read( File, Object->BitmapName, Size );
	}

	// read alpha name
	Result &= jeVFile_Read( File, &( Size ), sizeof( Size ) );
	if ( ( Size > 0 ) && ( Result == JE_TRUE ) )
	{
		Object->AlphaName = jeRam_Allocate( Size );
		if ( Object->AlphaName == NULL )
		{
			jeErrorLog_Add( JE_ERR_MEMORY_RESOURCE, NULL );
			goto ERROR_CreateFromFile;
		}
		Result &= jeVFile_Read( File, Object->AlphaName, Size );
	}

	// read xform
	Result &= jeVFile_Read( File, &( Object->Xf ), sizeof( Object->Xf ) );

	// read gravity
	Result &= jeVFile_Read( File, &( Object->Gravity ), sizeof( Object->Gravity ) );

	// read angle
	Result &= jeVFile_Read( File, &( Object->Angle ), sizeof( Object->Angle ) );

	// read rate
	Result &= jeVFile_Read( File, &( Object->Rate ), sizeof( Object->Rate ) );

	// read min color
	Result &= jeVFile_Read( File, &( Object->MinColor ), sizeof( Object->MinColor ) );

	// read max color
	Result &= jeVFile_Read( File, &( Object->MaxColor ), sizeof( Object->MaxColor ) );

	// read min speed
	Result &= jeVFile_Read( File, &( Object->MinSpeed ), sizeof( Object->MinSpeed ) );

	// read max speed
	Result &= jeVFile_Read( File, &( Object->MaxSpeed ), sizeof( Object->MaxSpeed ) );

	// read min scale
	Result &= jeVFile_Read( File, &( Object->MinScale ), sizeof( Object->MinScale ) );

	// read max scale
	Result &= jeVFile_Read( File, &( Object->MaxScale ), sizeof( Object->MaxScale ) );

	// read min unit life
	Result &= jeVFile_Read( File, &( Object->MinUnitLife ), sizeof( Object->MinUnitLife ) );

	// read max unit life
	Result &= jeVFile_Read( File, &( Object->MaxUnitLife ), sizeof( Object->MaxUnitLife ) );

	// read out draw box
	Result &= jeVFile_Read( File, &( Object->DrawExtBox ), sizeof( Object->DrawExtBox ) );

	// fail if there was an error
	if ( Result == JE_FALSE )
	{
		jeErrorLog_Add( JE_ERR_FILEIO_READ, NULL );
		goto ERROR_CreateFromFile;
	}

	// all done
	Object->LoadedFromDisk = JE_TRUE;
	return Object;

	// handle errors
	ERROR_CreateFromFile:

	// free all strings
	if ( Object->ArtName != NULL )
	{
		jeRam_Free( Object->ArtName );
	}
	if ( Object->BitmapName != NULL )
	{
		jeRam_Free( Object->BitmapName );
	}
	if ( Object->AlphaName != NULL )
	{
		jeRam_Free( Object->AlphaName );
	}

	// free object
	jeRam_Free( Object );

	// return error
	return NULL;
	NM;

} // CreateFromFile()
#else
void * JETCC CreateFromFile(
	jeVFile		*File,		// vfile to use
	jePtrMgr *PtrMgr )	// pointer manager
{

	// locals
	Spout		*Object;
	int			Size;
	jeBoolean	Result = JE_TRUE;
	BYTE Version;
	uint32 Tag;
 
	OutputDebugString("SpoutObject\n");

	// ensure valid data
	assert( File != NULL );

	// create new object
	Object = (Spout *)CreateInstance();
	if ( Object == NULL )
	{
		return NULL;
	}

	//Read Version
	if(!jeVFile_Read(File, &Tag, sizeof(Tag)))
	{
		jeErrorLog_Add( JE_ERR_FILEIO_READ, "SpoutObject_CreateFromFile:Tag" );
		goto ERROR_CreateFromFile;
	}

	if (Tag == FILE_UNIQUE_ID)
	{
		if (!jeVFile_Read(File, &Version, sizeof(Version)))
		{
    		jeErrorLog_Add( JE_ERR_FILEIO_READ, "SpoutObject_CreateFromFile:Version" );
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
	    // read art name
	    Result &= jeVFile_Read( File, &( Size ), sizeof( Size ) );
	    if ( ( Size > 0 ) && ( Result == JE_TRUE ) )
		{
		    Object->ArtName = (char *)jeRam_Allocate( Size );
		    if ( Object->ArtName == NULL )
			{
			    jeErrorLog_Add( JE_ERR_MEMORY_RESOURCE, NULL );
			    goto ERROR_CreateFromFile;
			}
		    Result &= jeVFile_Read( File, Object->ArtName, Size );
		}

	    // read bitmap name
	    Result &= jeVFile_Read( File, &( Size ), sizeof( Size ) );
	    if ( ( Size > 0 ) && ( Result == JE_TRUE ) )
		{
		    Object->BitmapName = (char *)jeRam_Allocate( Size );
		    if ( Object->BitmapName == NULL )
			{
			    jeErrorLog_Add( JE_ERR_MEMORY_RESOURCE, NULL );
			    goto ERROR_CreateFromFile;
			}
		    Result &= jeVFile_Read( File, Object->BitmapName, Size );
		}

	    // read alpha name
	    Result &= jeVFile_Read( File, &( Size ), sizeof( Size ) );
	    if ( ( Size > 0 ) && ( Result == JE_TRUE ) )
		{
		    Object->AlphaName = (char *)jeRam_Allocate( Size );
		    if ( Object->AlphaName == NULL )
			{
			    jeErrorLog_Add( JE_ERR_MEMORY_RESOURCE, NULL );
			    goto ERROR_CreateFromFile;
			}
		    Result &= jeVFile_Read( File, Object->AlphaName, Size );
		}

	    // read xform
	    Result &= jeVFile_Read( File, &( Object->Xf ), sizeof( Object->Xf ) );

	    // read gravity
	    Result &= jeVFile_Read( File, &( Object->Gravity ), sizeof( Object->Gravity ) );

	    // read angle
	    Result &= jeVFile_Read( File, &( Object->Angle ), sizeof( Object->Angle ) );

	    // read rate
	    Result &= jeVFile_Read( File, &( Object->Rate ), sizeof( Object->Rate ) );

	    // read min color
	    Result &= jeVFile_Read( File, &( Object->MinColor ), sizeof( Object->MinColor ) );

	    // read max color
	    Result &= jeVFile_Read( File, &( Object->MaxColor ), sizeof( Object->MaxColor ) );

	    // read min speed
	    Result &= jeVFile_Read( File, &( Object->MinSpeed ), sizeof( Object->MinSpeed ) );

    	// read max speed
   	    Result &= jeVFile_Read( File, &( Object->MaxSpeed ), sizeof( Object->MaxSpeed ) );
 
	    // read min scale
	    Result &= jeVFile_Read( File, &( Object->MinScale ), sizeof( Object->MinScale ) );

	    // read max scale
	    Result &= jeVFile_Read( File, &( Object->MaxScale ), sizeof( Object->MaxScale ) );

	    // read min unit life
	    Result &= jeVFile_Read( File, &( Object->MinUnitLife ), sizeof( Object->MinUnitLife ) );

	    // read max unit life
	    Result &= jeVFile_Read( File, &( Object->MaxUnitLife ), sizeof( Object->MaxUnitLife ) );

	    // read out draw box
	    Result &= jeVFile_Read( File, &( Object->DrawExtBox ), sizeof( Object->DrawExtBox ) );
	}


	// fail if there was an error
	if ( Result == JE_FALSE )
	{
	    jeErrorLog_Add( JE_ERR_FILEIO_READ, NULL );
	    goto ERROR_CreateFromFile;
	}

	// all done
	Object->LoadedFromDisk = JE_TRUE;
	return Object;

	// handle errors
	ERROR_CreateFromFile:

	// free all strings
	if ( Object->ArtName != NULL )
	{
		jeRam_Free( Object->ArtName );
	}
	if ( Object->BitmapName != NULL )
	{
		jeRam_Free( Object->BitmapName );
	}
	if ( Object->AlphaName != NULL )
	{
		jeRam_Free( Object->AlphaName );
	}

	// free object
	jeRam_Free( Object );

	// return error
	return NULL;

} // CreateFromFile()
#endif


////////////////////////////////////////////////////////////////////////////////////////
//
//	WriteToFile()
//
///////////////////////////////////////////////////////////////////////////////////////
#if NEWSAVE_SPT
jeBoolean JETCC WriteToFile(
	const void	*Instance,
	jeVFile		*File,
	jeNameMgr *NM )
{

	// locals
	Spout		*Object;
	jeBoolean	Result = JE_TRUE;
	int			Size;
	BYTE Version = SPOUTOBJECT_VERSION;
	uint32 Tag = FILE_UNIQUE_ID;

	// ensure valid data
	assert( Instance != NULL );
	assert( File != NULL );

	// get object data
	Object = (Spout *)Instance;

	//write version
	if( !jeVFile_Write(	File, &Tag, sizeof(Tag)))
	{
    	jeErrorLog_Add(JE_ERR_FILEIO_WRITE, "SpoutObject_WriteToFile:Tag");
	    return( JE_FALSE );
	}
	
	if( !jeVFile_Write(	File, &Version, sizeof(Version) ) )
	{
    	jeErrorLog_Add(JE_ERR_FILEIO_WRITE, "SpoutObject_WriteToFile:Version");
	    return( JE_FALSE );
	}

	// write art name
	if ( Object->ArtName != NULL )
	{
		Util_WriteString( File, Object->ArtName );
	}
	else
	{
		Size = 0;
		Result &= jeVFile_Write( File, &Size, sizeof( Size ) );
	}

	// write bitmap name
	if ( Object->BitmapName != NULL )
	{
		Util_WriteString( File, Object->BitmapName );
	}
	else
	{
		Size = 0;
		Result &= jeVFile_Write( File, &Size, sizeof( Size ) );
	}

	// write alpha name
	if ( Object->AlphaName != NULL )
	{
		Util_WriteString( File, Object->AlphaName );
	}
	else
	{
		Size = 0;
		Result &= jeVFile_Write( File, &Size, sizeof( Size ) );
	}

	// write xform
	Result &= jeVFile_Write( File, &( Object->Xf ), sizeof( Object->Xf ) );

	// write gravity
	Result &= jeVFile_Write( File, &( Object->Gravity ), sizeof( Object->Gravity ) );

	// write angle
	Result &= jeVFile_Write( File, &( Object->Angle ), sizeof( Object->Angle ) );

	// write rate
	Result &= jeVFile_Write( File, &( Object->Rate ), sizeof( Object->Rate ) );

	// write min color
	Result &= jeVFile_Write( File, &( Object->MinColor ), sizeof( Object->MinColor ) );

	// write max color
	Result &= jeVFile_Write( File, &( Object->MaxColor ), sizeof( Object->MaxColor ) );

	// write min speed
	Result &= jeVFile_Write( File, &( Object->MinSpeed ), sizeof( Object->MinSpeed ) );

	// write max speed
	Result &= jeVFile_Write( File, &( Object->MaxSpeed ), sizeof( Object->MaxSpeed ) );

	// write min scale
	Result &= jeVFile_Write( File, &( Object->MinScale ), sizeof( Object->MinScale ) );

	// write max scale
	Result &= jeVFile_Write( File, &( Object->MaxScale ), sizeof( Object->MaxScale ) );

	// write min unit life
	Result &= jeVFile_Write( File, &( Object->MinUnitLife ), sizeof( Object->MinUnitLife ) );

	// write max unit life
	Result &= jeVFile_Write( File, &( Object->MaxUnitLife ), sizeof( Object->MaxUnitLife ) );

	// write out draw box
	Result &= jeVFile_Write( File, &( Object->DrawExtBox ), sizeof( Object->DrawExtBox ) );

	// log errors
	if ( Result != JE_TRUE )
	{
		jeErrorLog_Add( JE_ERR_FILEIO_WRITE, NULL );
	}

	// all done
	return Result;
	NM;

} // WriteToFile()
#else
	jeBoolean JETCC WriteToFile(
	const void	*Instance,
	jeVFile		*File,
	jePtrMgr *PtrMgr )
{

	// locals
	Spout		*Object;
	jeBoolean	Result = JE_TRUE;
	int			Size;

	// ensure valid data
	assert( Instance != NULL );
	assert( File != NULL );

	// get object data
	Object = (Spout *)Instance;

	// write art name
	if ( Object->ArtName != NULL )
	{
		Util_WriteString( File, Object->ArtName );
	}
	else
	{
		Size = 0;
		Result &= jeVFile_Write( File, &Size, sizeof( Size ) );
	}

	// write bitmap name
	if ( Object->BitmapName != NULL )
	{
		Util_WriteString( File, Object->BitmapName );
	}
	else
	{
		Size = 0;
		Result &= jeVFile_Write( File, &Size, sizeof( Size ) );
	}

	// write alpha name
	if ( Object->AlphaName != NULL )
	{
		Util_WriteString( File, Object->AlphaName );
	}
	else
	{
		Size = 0;
		Result &= jeVFile_Write( File, &Size, sizeof( Size ) );
	}

	// write xform
	Result &= jeVFile_Write( File, &( Object->Xf ), sizeof( Object->Xf ) );

	// write gravity
	Result &= jeVFile_Write( File, &( Object->Gravity ), sizeof( Object->Gravity ) );

	// write angle
	Result &= jeVFile_Write( File, &( Object->Angle ), sizeof( Object->Angle ) );

	// write rate
	Result &= jeVFile_Write( File, &( Object->Rate ), sizeof( Object->Rate ) );

	// write min color
	Result &= jeVFile_Write( File, &( Object->MinColor ), sizeof( Object->MinColor ) );

	// write max color
	Result &= jeVFile_Write( File, &( Object->MaxColor ), sizeof( Object->MaxColor ) );

	// write min speed
	Result &= jeVFile_Write( File, &( Object->MinSpeed ), sizeof( Object->MinSpeed ) );

	// write max speed
	Result &= jeVFile_Write( File, &( Object->MaxSpeed ), sizeof( Object->MaxSpeed ) );

	// write min scale
	Result &= jeVFile_Write( File, &( Object->MinScale ), sizeof( Object->MinScale ) );

	// write max scale
	Result &= jeVFile_Write( File, &( Object->MaxScale ), sizeof( Object->MaxScale ) );

	// write min unit life
	Result &= jeVFile_Write( File, &( Object->MinUnitLife ), sizeof( Object->MinUnitLife ) );

	// write max unit life
	Result &= jeVFile_Write( File, &( Object->MaxUnitLife ), sizeof( Object->MaxUnitLife ) );

	// write out draw box
	Result &= jeVFile_Write( File, &( Object->DrawExtBox ), sizeof( Object->DrawExtBox ) );

	// log errors
	if ( Result != JE_TRUE )
	{
		jeErrorLog_Add( JE_ERR_FILEIO_WRITE, NULL );
	}

	// all done
	return Result;

} // WriteToFile()

#endif


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
	Spout	*Object;

	// ensure valid data
	assert( Instance != NULL );
	assert( List != NULL );

	// get object data
	Object = (Spout *)Instance;

	// setup property list
	SpoutProperties[SPOUT_RATE_INDEX].Data.Float = Object->Rate;
	SpoutProperties[SPOUT_ANGLE_INDEX].Data.Float = Object->Angle;
	SpoutProperties[SPOUT_MINSPEED_INDEX].Data.Float = Object->MinSpeed;
	SpoutProperties[SPOUT_MAXSPEED_INDEX].Data.Float = Object->MaxSpeed;
	SpoutProperties[SPOUT_MINSCALE_INDEX].Data.Float = Object->MinScale;
	SpoutProperties[SPOUT_MAXSCALE_INDEX].Data.Float = Object->MaxScale;
	SpoutProperties[SPOUT_MINUNITLIFE_INDEX].Data.Float = Object->MinUnitLife;
	SpoutProperties[SPOUT_MAXUNITLIFE_INDEX].Data.Float = Object->MaxUnitLife;
	SpoutProperties[SPOUT_COLORMINGROUP_INDEX].Data.Vector.X = Object->MinColor.r;
	SpoutProperties[SPOUT_COLORMINGROUP_INDEX].Data.Vector.Y = Object->MinColor.g;
	SpoutProperties[SPOUT_COLORMINGROUP_INDEX].Data.Vector.Z = Object->MinColor.b;
	SpoutProperties[SPOUT_COLORMIN_INDEX].Data.Vector.X = Object->MinColor.r;
	SpoutProperties[SPOUT_COLORMIN_INDEX].Data.Vector.Y = Object->MinColor.g;
	SpoutProperties[SPOUT_COLORMIN_INDEX].Data.Vector.Z = Object->MinColor.b;
	SpoutProperties[SPOUT_COLORMAXGROUP_INDEX].Data.Vector.X = Object->MaxColor.r;
	SpoutProperties[SPOUT_COLORMAXGROUP_INDEX].Data.Vector.Y = Object->MaxColor.g;
	SpoutProperties[SPOUT_COLORMAXGROUP_INDEX].Data.Vector.Z = Object->MaxColor.b;
	SpoutProperties[SPOUT_COLORMAX_INDEX].Data.Vector.X = Object->MaxColor.r;
	SpoutProperties[SPOUT_COLORMAX_INDEX].Data.Vector.Y = Object->MaxColor.g;
	SpoutProperties[SPOUT_COLORMAX_INDEX].Data.Vector.Z = Object->MaxColor.b;
	SpoutProperties[SPOUT_COLORMINRED_INDEX].Data.Float = Object->MinColor.r;
	SpoutProperties[SPOUT_COLORMINGREEN_INDEX].Data.Float = Object->MinColor.g;
	SpoutProperties[SPOUT_COLORMINBLUE_INDEX].Data.Float = Object->MinColor.b;
	SpoutProperties[SPOUT_COLORMAXRED_INDEX].Data.Float = Object->MaxColor.r;
	SpoutProperties[SPOUT_COLORMAXGREEN_INDEX].Data.Float = Object->MaxColor.g;
	SpoutProperties[SPOUT_COLORMAXBLUE_INDEX].Data.Float = Object->MaxColor.b;
	SpoutProperties[SPOUT_DRAWEXTBOXDISPLAY_INDEX].Data.Bool = Object->DrawExtBoxDisplay;
	SpoutProperties[SPOUT_DRAWEXTBOXMINX_INDEX].Data.Float = Object->DrawExtBox.Min.X;
	SpoutProperties[SPOUT_DRAWEXTBOXMINY_INDEX].Data.Float = Object->DrawExtBox.Min.Y;
	SpoutProperties[SPOUT_DRAWEXTBOXMINZ_INDEX].Data.Float = Object->DrawExtBox.Min.Z;
	SpoutProperties[SPOUT_DRAWEXTBOXMAXX_INDEX].Data.Float = Object->DrawExtBox.Max.X;
	SpoutProperties[SPOUT_DRAWEXTBOXMAXY_INDEX].Data.Float = Object->DrawExtBox.Max.Y;
	SpoutProperties[SPOUT_DRAWEXTBOXMAXZ_INDEX].Data.Float = Object->DrawExtBox.Max.Z;

	// set art size property
	jeProperty_FillCombo(	&( SpoutPropertyList.pjeProperty[SPOUT_ARTSIZE_INDEX] ),
							Util_LoadLibraryString( hClassInstance, IDS_ARTSIZE ),
							Bitmaps->StringSizes[Object->CurWidth],
							SPOUT_ARTSIZE_ID,
							Bitmaps->SizesListSize,
							Bitmaps->StringSizes );

	// set bitmap and alpha lists
	jeProperty_FillCombo(	&( SpoutPropertyList.pjeProperty[SPOUT_BITMAPLIST_INDEX] ),
							Util_LoadLibraryString( hClassInstance, IDS_BITMAPLIST ),
							Object->BitmapName,
							SPOUT_BITMAPLIST_ID,
							CurBitmaps.Total,
							CurBitmaps.Name );
	jeProperty_FillCombo(	&( SpoutPropertyList.pjeProperty[SPOUT_ALPHALIST_INDEX] ),
							Util_LoadLibraryString( hClassInstance, IDS_ALPHALIST ),
							Object->AlphaName,
							SPOUT_ALPHALIST_ID,
							CurBitmaps.Total,
							CurBitmaps.Name );

	// get property list
	*List = jeProperty_ListCopy( &SpoutPropertyList );
	if ( *List == NULL )
	{
		jeErrorLog_Add( JE_ERR_SUBSYSTEM_FAILURE, NULL );
		return JE_FALSE;
	}

	// reset dirty flag
	SpoutPropertyList.bDirty = JE_FALSE;

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
	Spout		*Object = NULL;
	jeBoolean	Result = JE_TRUE;

	// ensure valid data
	assert( Instance != NULL );
	assert( pData != NULL );

	// get object data
	Object = (Spout *)Instance;

	// process field id
	switch ( FieldID )
	{

		// adjust rate
		case SPOUT_RATE_ID:
		{
			assert( DataType == PROPERTY_FLOAT_TYPE );
			Object->Rate = pData->Float;
			break;
		}

		// adjust angle
		case SPOUT_ANGLE_ID:
		{
			assert( DataType == PROPERTY_FLOAT_TYPE );
			Object->Angle = pData->Float;
			break;
		}

		// adjust min speed
		case SPOUT_MINSPEED_ID:
		{
			assert( DataType == PROPERTY_FLOAT_TYPE );
			if ( pData->Float <= Object->MaxSpeed )
			{
				Object->MinSpeed = pData->Float;
			}
			break;
		}

		// adjust max speed
		case SPOUT_MAXSPEED_ID:
		{
			assert( DataType == PROPERTY_FLOAT_TYPE );
			if ( pData->Float >= Object->MinSpeed )
			{
				Object->MaxSpeed = pData->Float;
			}
			break;
		}

		// adjust min scale
		case SPOUT_MINSCALE_ID:
		{
			assert( DataType == PROPERTY_FLOAT_TYPE );
			if ( pData->Float <= Object->MaxScale )
			{
				Object->MinScale = pData->Float;
			}
			break;
		}

		// adjust max scale
		case SPOUT_MAXSCALE_ID:
		{
			assert( DataType == PROPERTY_FLOAT_TYPE );
			if ( pData->Float >= Object->MinScale )
			{
				Object->MaxScale = pData->Float;
			}
			break;
		}

		// adjust min unit life
		case SPOUT_MINUNITLIFE_ID:
		{
			assert( DataType == PROPERTY_FLOAT_TYPE );
			if ( pData->Float <= Object->MaxUnitLife )
			{
				Object->MinUnitLife = pData->Float;
			}
			break;
		}

		// adjust max unit life
		case SPOUT_MAXUNITLIFE_ID:
		{
			assert( DataType == PROPERTY_FLOAT_TYPE );
			if ( pData->Float >= Object->MinUnitLife )
			{
				Object->MaxUnitLife = pData->Float;
			}
			break;
		}

		// adjust min color
		case SPOUT_COLORMIN_ID:
		{
			assert( DataType == PROPERTY_COLOR_PICKER_TYPE );
			Object->MinColor.r = pData->Vector.X;
			Object->MinColor.g = pData->Vector.Y;
			Object->MinColor.b = pData->Vector.Z;
//	1.16.05 by tom morris - better support for new color button
//			Object->MaxColor.r = ( Object->MinColor.r > Object->MaxColor.r ) ? Object->MinColor.r : Object->MaxColor.r;
//			Object->MaxColor.g = ( Object->MinColor.g > Object->MaxColor.g ) ? Object->MinColor.g : Object->MaxColor.g;
//			Object->MaxColor.b = ( Object->MinColor.b > Object->MaxColor.b ) ? Object->MinColor.b : Object->MaxColor.b;
			break;
		}
		case SPOUT_COLORMINRED_ID:
		{
			assert( DataType == PROPERTY_FLOAT_TYPE );
			Object->MinColor.r = pData->Float;
//	1.16.05 by tom morris - better support for new color button
//			Object->MaxColor.r = ( Object->MinColor.r > Object->MaxColor.r ) ? Object->MinColor.r : Object->MaxColor.r;
			break;
		}
		case SPOUT_COLORMINGREEN_ID:
		{
			assert( DataType == PROPERTY_FLOAT_TYPE );
			Object->MinColor.g = pData->Float;
//	1.16.05 by tom morris - better support for new color button
//			Object->MaxColor.g = ( Object->MinColor.g > Object->MaxColor.g ) ? Object->MinColor.g : Object->MaxColor.g;
			break;
		}
		case SPOUT_COLORMINBLUE_ID:
		{
			assert( DataType == PROPERTY_FLOAT_TYPE );
			Object->MinColor.b = pData->Float;
//	1.16.05 by tom morris - better support for new color button
//			Object->MaxColor.b = ( Object->MinColor.b > Object->MaxColor.b ) ? Object->MinColor.b : Object->MaxColor.b;
			break;
		}

		// adjust max color
		case SPOUT_COLORMAX_ID:
		{
			assert( DataType == PROPERTY_COLOR_PICKER_TYPE );
			Object->MaxColor.r = pData->Vector.X;
			Object->MaxColor.g = pData->Vector.Y;
			Object->MaxColor.b = pData->Vector.Z;
//	1.16.05 by tom morris - better support for new color button
//			Object->MinColor.r = ( Object->MinColor.r > Object->MaxColor.r ) ? Object->MaxColor.r : Object->MinColor.r;
//			Object->MinColor.g = ( Object->MinColor.g > Object->MaxColor.g ) ? Object->MaxColor.g : Object->MinColor.g;
//			Object->MinColor.b = ( Object->MinColor.b > Object->MaxColor.b ) ? Object->MaxColor.b : Object->MinColor.b;
			break;
		}
		case SPOUT_COLORMAXRED_ID:
		{
			assert( DataType == PROPERTY_FLOAT_TYPE );
			Object->MaxColor.r = pData->Float;
//	1.16.05 by tom morris - better support for new color button
//			Object->MinColor.r = ( Object->MinColor.r > Object->MaxColor.r ) ? Object->MaxColor.r : Object->MinColor.r;
			break;
		}
		case SPOUT_COLORMAXGREEN_ID:
		{
			assert( DataType == PROPERTY_FLOAT_TYPE );
			Object->MaxColor.g = pData->Float;
//	1.16.05 by tom morris - better support for new color button
//			Object->MinColor.g = ( Object->MinColor.g > Object->MaxColor.g ) ? Object->MaxColor.g : Object->MinColor.g;
			break;
		}
		case SPOUT_COLORMAXBLUE_ID:
		{
			assert( DataType == PROPERTY_FLOAT_TYPE );
			Object->MaxColor.b = pData->Float;
//	1.16.05 by tom morris - better support for new color button
//			Object->MinColor.b = ( Object->MinColor.b > Object->MaxColor.b ) ? Object->MaxColor.b : Object->MinColor.b;
			break;
		}

		// toggle draw box
		case SPOUT_DRAWEXTBOXDISPLAY_ID:
		{
			assert( DataType == PROPERTY_CHECK_TYPE );
			Object->DrawExtBoxDisplay = pData->Bool;
			break;
		}

		// adjust draw extent box size
		case SPOUT_DRAWEXTBOXMINX_ID:
		{
			float	OldVal;
			assert( DataType == PROPERTY_FLOAT_TYPE );
			OldVal = Object->DrawExtBox.Min.X;
			Object->DrawExtBox.Min.X = pData->Float;
			if ( jeExtBox_IsValid( &( Object->DrawExtBox ) ) == JE_FALSE )
			{
				Object->DrawExtBox.Min.X = OldVal;
			}
			break;
		}
		case SPOUT_DRAWEXTBOXMINY_ID:
		{
			float	OldVal;
			assert( DataType == PROPERTY_FLOAT_TYPE );
			OldVal = Object->DrawExtBox.Min.Y;
			Object->DrawExtBox.Min.Y = pData->Float;
			if ( jeExtBox_IsValid( &( Object->DrawExtBox ) ) == JE_FALSE )
			{
				Object->DrawExtBox.Min.Y = OldVal;
			}
			break;
		}
		case SPOUT_DRAWEXTBOXMINZ_ID:
		{
			float	OldVal;
			assert( DataType == PROPERTY_FLOAT_TYPE );
			OldVal = Object->DrawExtBox.Min.Z;
			Object->DrawExtBox.Min.Z = pData->Float;
			if ( jeExtBox_IsValid( &( Object->DrawExtBox ) ) == JE_FALSE )
			{
				Object->DrawExtBox.Min.Z = OldVal;
			}
			break;
		}
		case SPOUT_DRAWEXTBOXMAXX_ID:
		{
			float	OldVal;
			assert( DataType == PROPERTY_FLOAT_TYPE );
			OldVal = Object->DrawExtBox.Max.X;
			Object->DrawExtBox.Max.X = pData->Float;
			if ( jeExtBox_IsValid( &( Object->DrawExtBox ) ) == JE_FALSE )
			{
				Object->DrawExtBox.Max.X = OldVal;
			}
			break;
		}
		case SPOUT_DRAWEXTBOXMAXY_ID:
		{
			float	OldVal;
			assert( DataType == PROPERTY_FLOAT_TYPE );
			OldVal = Object->DrawExtBox.Max.Y;
			Object->DrawExtBox.Max.Y = pData->Float;
			if ( jeExtBox_IsValid( &( Object->DrawExtBox ) ) == JE_FALSE )
			{
				Object->DrawExtBox.Max.Y = OldVal;
			}
			break;
		}
		case SPOUT_DRAWEXTBOXMAXZ_ID:
		{
			float	OldVal;
			assert( DataType == PROPERTY_FLOAT_TYPE );
			OldVal = Object->DrawExtBox.Max.Z;
			Object->DrawExtBox.Max.Z = pData->Float;
			if ( jeExtBox_IsValid( &( Object->DrawExtBox ) ) == JE_FALSE )
			{
				Object->DrawExtBox.Max.Z = OldVal;
			}
			break;
		}

		// adjust size choice
		case SPOUT_ARTSIZE_ID:
		{

			// locals
			int	i;

			// ensure valid data
			assert( DataType == PROPERTY_COMBO_TYPE );

			// determine which size was selected
			Object->CurWidth = -1;
			for ( i = 0; i < Bitmaps->Total; i++ )
			{
				if ( stricmp( Bitmaps->StringSizes[i], pData->String ) == 0 )
				{
					Object->CurWidth = i;
					break;
				}
			}
			assert( Object->CurWidth != -1 );

			// recreate CurBitmaps list
			Util_RecreateCurBitmapsList( Object );

			// destroy all current art
			{
			
				// remove all particles
				jeParticle_SystemRemoveAll( Object->Ps );

				// destroy objects current bitmap
				Spout_DestroyArt( Object );

				// zap all art names
				if ( Object->BitmapName != NULL )
				{
					jeRam_Free( Object->BitmapName );
				}
				Object->BitmapName = Util_StrDup( NoSelection );
				if ( Object->AlphaName != NULL )
				{
					jeRam_Free( Object->AlphaName );
				}
				Object->AlphaName = Util_StrDup( NoSelection );
				if ( Object->ArtName != NULL )
				{
					jeRam_Free( Object->ArtName );
					Object->ArtName = NULL;
				}
			}

			// force rebuild of properties bar
			SpoutPropertyList.bDirty = JE_TRUE;
			break;
		}

		// adjust bitmap
		case SPOUT_BITMAPLIST_ID:
		case SPOUT_ALPHALIST_ID:
		{
			// locals
			int	Size;

			// ensure valid data
			assert( DataType == PROPERTY_COMBO_TYPE );
			assert( pData->String != NULL );

			// remove all particles
			jeParticle_SystemRemoveAll( Object->Ps );

			// destroy objects current bitmap
			Spout_DestroyArt( Object );

			// recreate bitmap name...
			if ( FieldID == SPOUT_BITMAPLIST_ID )
			{
				if ( Object->BitmapName != NULL )
				{
					jeRam_Free( Object->BitmapName );
				}
				Object->BitmapName = Util_StrDup( pData->String );
			}
			// ...or alpha name
			else if ( FieldID == SPOUT_ALPHALIST_ID )
			{
				if ( Object->AlphaName != NULL )
				{
					jeRam_Free( Object->AlphaName );
				}
				Object->AlphaName = Util_StrDup( pData->String );
			}

			// zap art name
			if ( Object->ArtName != NULL )
			{
				jeRam_Free( Object->ArtName );
			}

			// do nothing further if no main bitmap is provided
			if ( stricmp( Object->BitmapName, NoSelection ) == 0 )
			{
				break;
			}

			// build art name
			Size = strlen( Object->BitmapName ) + 1;
			if ( stricmp( Object->AlphaName, NoSelection ) != 0 )
			{
				Size += strlen( Object->AlphaName );
			}
			Object->ArtName = (char *)jeRam_Allocate( Size );
			if ( Object->ArtName == NULL )
			{
				jeErrorLog_Add( JE_ERR_MEMORY_RESOURCE, NULL );
				break;
			}
			strcpy( Object->ArtName, Object->BitmapName );
			if ( stricmp( Object->AlphaName, NoSelection ) != 0 )
			{
				strcat( Object->ArtName, Object->AlphaName );
			}

            // if it doesn't exist then create it
			if ( Object->Art == NULL )
			{
    			// locals
                jeBitmap* artbmp;
				jeVFile	*FileDir;

    			// create new art
                Object->Art = jeMaterialSpec_Create(Object->Engine, Object->ResourceMgr);
	
                // get vfile dir
				FileDir = jeResource_GetVFile( Object->ResourceMgr, "GlobalMaterials" );
				if ( FileDir == NULL )
				{
					jeErrorLog_Add( JE_ERR_SUBSYSTEM_FAILURE, NULL );
					return JE_FALSE;
				}

				// create new art
				if ( stricmp( Object->AlphaName, NoSelection ) != 0 )
				{
					artbmp = Util_CreateBitmapFromFileName( FileDir, Object->BitmapName, Object->AlphaName );
				}
				else
				{
					artbmp = Util_CreateBitmapFromFileName( FileDir, Object->BitmapName, NULL );
				}

				// close vfile dir
				if ( jeResource_DeleteVFile( Object->ResourceMgr, "GlobalMaterials" ) == 0 )
				{
					jeVFile_Close( FileDir );
				}

				// fail if art wasnt created
				if ( artbmp == NULL )
				{
					jeErrorLog_Add( JE_ERR_SUBSYSTEM_FAILURE, NULL );
					return JE_FALSE;
				}

				// add it to the resource manager
#pragma message ("Krouer: change NULL to something better next time")
                jeMaterialSpec_AddLayerFromBitmap(Object->Art, 0, artbmp, NULL);
    			jeEngine_AddBitmap( Object->Engine, artbmp, JE_ENGINE_BITMAP_TYPE_3D );
			}
/*
            // add it to the engine
			jeEngine_AddBitmap( Object->Engine, Object->Art, JE_ENGINE_BITMAP_TYPE_3D );
*/
			break;
		}

		// if we got to here then its an unsupported field
		default:
		{
			assert( 0 );
			Result = JE_FALSE;
			break;
		}
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
	Spout	*Object;

	// ensure valid data
	assert( Instance != NULL );
	assert( Xf != NULL );

	// get object data
	Object = (Spout *)Instance;

	// save xform
	Object->Xf = *Xf;

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
	Spout	*Object;

	// ensure valid data
	assert( Instance != NULL );
	assert( Xf != NULL );

	// get object data
	Object = (Spout *)Instance;

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
#ifdef WIN32
	HWND	Parent )
#endif
#ifdef BUILD_BE
	class G3DView *Parent )
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

	// locals
	Spout	*Object;

	// ensure valid data
	assert( Instance != NULL );

	// do nothing if no time has elapsed
	if ( TimeDelta == 0.0f )
	{
		return JE_TRUE;
	}

	// get object
	Object = (Spout *)Instance;

	// do nothing if no bitmap has been selected
	if ( Object->Art == NULL )
	{
		return JE_TRUE;
	}

	// add to elapsed time
	Object->TimeElapsed += TimeDelta;

	// add new particles
	while ( Object->TimeElapsed > Object->Rate )
	{

		// locals
		JE_LVertex	Vertex;
		jeVec3d		Velocity;

		// adjust elapsed time
		Object->TimeElapsed -= Object->Rate;

		// setup source
		Vertex.X = Object->Xf.Translation.X;
		Vertex.Y = Object->Xf.Translation.Y;
		Vertex.Z = Object->Xf.Translation.Z;

		// setup velocity
		if ( Object->MinSpeed > 0.0f )
		{

			// locals
			jeXForm3d	Xf;

			// randomly rotate the xform
			jeXForm3d_Copy( &( Object->Xf ), &Xf );
			jeXForm3d_RotateX( &Xf, Util_Frand( -Object->Angle, Object->Angle ) );
			jeXForm3d_RotateZ( &Xf, Util_Frand( -Object->Angle, Object->Angle ) );

			// setup velocity vector
			jeXForm3d_GetUp( &Xf, &Velocity );
			jeVec3d_Normalize( &Velocity );
			jeVec3d_Scale( &Velocity, Util_Frand( Object->MinSpeed, Object->MaxSpeed ), &Velocity );
		}
		else
		{
			jeVec3d_Set( &Velocity, 0.0f, 0.0f, 0.0f );
		}

		// setup color
		Vertex.r = Util_Frand( Object->MinColor.r, Object->MaxColor.r );
		Vertex.g = Util_Frand( Object->MinColor.g, Object->MaxColor.g );
		Vertex.b = Util_Frand( Object->MinColor.b, Object->MaxColor.b );
		Vertex.a = Util_Frand( Object->MinColor.a, Object->MaxColor.a );

		// clear uv's
		Vertex.u = 0.0f;
		Vertex.v = 0.0f;

		// add the new particle
		assert( Object->World != NULL );

		jeParticle_SystemAddParticle(	Object->Ps,
										Object->World,
                                        Object->Art,
										&Vertex,
										NULL,
										Util_Frand( Object->MinUnitLife, Object->MaxUnitLife ),
										&Velocity,
										Util_Frand( Object->MinScale, Object->MaxScale ),
										&( Object->Gravity ) );
	}

	// update the particle system
	assert( Object->Ps != NULL );
	jeParticle_SystemFrame( Object->Ps, TimeDelta );

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

	// all done
	return JE_FALSE;

	// eliminate warnings
	Instance;
	Msg;
	Data;

} // SendAMessage()

// Icestorm
jeBoolean	JETCC ChangeBoxCollision(const void *Instance,const jeVec3d *Pos, const jeExtBox *FrontBox, const jeExtBox *BackBox, jeExtBox *ImpactBox, jePlane *Plane)
{
	return( JE_FALSE );Plane;ImpactBox;BackBox;FrontBox;Pos;Instance;
}