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
static BitmapList		*Bitmaps;
static char				*NoSelection = "< none >";

// Added by Incarnadine
// This function gets the correct translation for a collision/render box.  The problem was,
// boxes are positioned by their center, while ActorObj may be positioned anywhere.  This
// takes that into account and returns where the box should be positioned.
void GetBoxTranslation(const jeExtBox *Box, const jeActor *Actor, jeVec3d *Translation)
{
	jeVec3d BoxCenterToActor;
	jeVec3d BoxCenter;	
	jeVec3d TranslationMod;
	jeVec3d ActorPos, Zero;
	ActorObj *Object;

	assert(Box != NULL);
	assert(Translation != NULL);
	assert(Actor != NULL);
	assert(Actor->Object != NULL);	
	
	Object = Actor->Object;
	jeVec3d_Set(&Zero,0,0,0);
	ActorPos = Actor->Xf.Translation;	
	jeExtBox_GetTranslation(Box,&BoxCenter);
	jeVec3d_Subtract(&BoxCenter, &Zero,&BoxCenterToActor);
	BoxCenterToActor.X = 0.0f;//*= Object->ScaleX;
	BoxCenterToActor.Y *= Object->ScaleX;
	BoxCenterToActor.Z = 0.0f;//*= Object->ScaleZ;	
	jeVec3d_Subtract(&BoxCenterToActor,&BoxCenter,&TranslationMod);
	jeVec3d_Add(&TranslationMod,&ActorPos,Translation);
}

////////////////////////////////////////////////////////////////////////////////////////
//
//	Util_StrDup()
//
////////////////////////////////////////////////////////////////////////////////////////
static char * Util_StrDup(
	const char	*const String )	// string to copy
{

	// locals
	char	*NewString;

	// ensure valid data
	assert( String != NULL );

	// copy string
	NewString = (char *)jeRam_Allocate( strlen( String ) + 1 );
	if ( NewString ) 
	{
		strcpy( NewString, String );
	}
	else
	{
		jeErrorLog_Add( JE_ERR_MEMORY_RESOURCE, NULL );
	}

	// return string
	return NewString;

} // Util_StrDup()



////////////////////////////////////////////////////////////////////////////////////////
//
//	Util_ResetActorMaterialToDefault()
//
//	Resets an actors material to its default one.
//
////////////////////////////////////////////////////////////////////////////////////////
static jeBoolean Util_ResetActorMaterialToDefault(
	jeActor		*Actor,				// actor to reset
	jeActor_Def	*ActorDef,			// actor def from which to get default info from
	int			MaterialIndex )		// material index
{

	// locals
	jeBody		*Body;
	jeBoolean	Result;
	const char	*MaterialName;
	jeMaterialSpec	*Bitmap;
	jeFloat		Red, Green, Blue;
	jeUVMapper	Mapper;

	// ensure valid data
	assert( Actor != NULL );
	assert( ActorDef != NULL );
	assert( MaterialIndex >= 0 );

	// get actor def body
	Body = jeActor_GetBody( ActorDef );
	assert ( Body != NULL );

	// get default material
	Result = jeBody_GetMaterial(	Body, MaterialIndex, &MaterialName,
									&Bitmap, &Red, &Green, &Blue,
									&Mapper );
	if ( Result == JE_FALSE )
	{
		jeErrorLog_Add( JE_ERR_SUBSYSTEM_FAILURE, "Failed to reset actor material to default" );
		return JE_FALSE;
	}

	// reset actor matet
	return jeActor_SetMaterial( Actor, MaterialIndex, Bitmap, Red, Green, Blue, Mapper );

} // Util_ResetActorMaterialToDefault()



////////////////////////////////////////////////////////////////////////////////////////
//
//	Util_CreateBitmapFromFileName()
//
//	Create a bitmap from a file.
//
////////////////////////////////////////////////////////////////////////////////////////
static jeMaterialSpec * Util_CreateBitmapFromFileName(
	jeVFile		*File,			// file system to use
	const char	*Name,			// name of the file
	const char	*AlphaName,		// name of the alpha file
	const jeResourceMgr* ResourceMgr)	
{

	// locals
	jeVFile		*BmpFile;
	jeBitmap	*Bmp;
	jeMaterialSpec *MatSpec;
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
	
	MatSpec = jeMaterialSpec_Create(jeResourceMgr_GetEngine(ResourceMgr), (jeResourceMgr*) ResourceMgr);
	
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

#pragma message ("Krouer: change NULL to something better next time")
	jeMaterialSpec_AddLayerFromBitmap(MatSpec, 0, Bmp, NULL);

	// all done
	return MatSpec;

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
	BitmapList		*Bmps;
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
		//jeBitmap			*Bitmap;
		jeMaterialSpec		*MatSpec;

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
		MatSpec = Util_CreateBitmapFromFileName( FileDir, Bmps->Name[CurFile], NULL, ResourceMgr );
		if ( MatSpec == NULL )
		{
			jeErrorLog_Add( JE_ERR_SUBSYSTEM_FAILURE, NULL );
			goto ERROR_Util_BuildBitmapList;
		}
		Bmps->Width[CurFile] = jeMaterialSpec_Width( MatSpec );
		Bmps->Height[CurFile] = jeMaterialSpec_Height( MatSpec );
		jeMaterialSpec_Destroy( &MatSpec );

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
			sprintf(Buf, "%i" , Bmps->NumericSizes[i]); //itoa( Bmps->NumericSizes[i], Buf, 10 );
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
//	Util_CreateEmptyList()
//
////////////////////////////////////////////////////////////////////////////////////////
static jeBoolean Util_CreateEmptyList(
	char	***List,		// where to save list pointer
	int		*ListSize )		// where to save list size
{

	// locals
	char	**NewList;

	// ensure valid data
	assert( List != NULL );
	assert( ListSize != NULL );

	// reset passed in data
	*List = NULL;
	*ListSize = 0;

	// allocate list
	NewList = (char **)jeRam_Allocate( sizeof( char * ) );
	if ( NewList == NULL )
	{
		jeErrorLog_Add( JE_ERR_MEMORY_RESOURCE, NULL );
		return JE_FALSE;
	}

	// init data
	NewList[0] = Util_StrDup( NoSelection );
	*ListSize = 1;

	// all done
	*List = NewList;
	return JE_TRUE;

} // Util_CreateEmptyList()



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

	// log errors
	if ( Result != JE_TRUE )
	{
		jeErrorLog_Add( JE_ERR_FILEIO_WRITE, NULL );
	}

	// all done
	return Result;

} // Util_WriteString()



////////////////////////////////////////////////////////////////////////////////////////
//
//	Util_DestroyFileList()
//
////////////////////////////////////////////////////////////////////////////////////////
static void Util_DestroyFileList(
	char	***DeadList,		// list to destroy
	int		*DeadListSize )		// size of list
{

	// locals
	char	**List;
	int		ListSize;
	int		i;

	// save pointers
	assert( DeadList != NULL );
	List = *DeadList;
	assert( List != NULL );
	assert( DeadListSize != NULL );
	ListSize = *DeadListSize;
	assert( ListSize > 0 );

	// free all strings
	for ( i = 0; i < ListSize; i++ )
	{
		if ( List[i] != NULL )
		{
			jeRam_Free( List[i] );
			List[i] = NULL;
		}
	}

	// free the list itself
	jeRam_Free( List );

	// zap final data
	List = NULL;
	ListSize = 0;

} // Util_DestroyFileList()



////////////////////////////////////////////////////////////////////////////////////////
//
//	Util_BuildFileList()
//
////////////////////////////////////////////////////////////////////////////////////////
static char ** Util_BuildFileList(
	jeResourceMgr	*ResourceMgr,
	char			*ResourceName,
	char			*FileFilter,
	int				*FileListSize )
{

	// locals
	jeVFile			*FileDir = NULL;
	jeVFile_Finder	*Finder = NULL;
	int				TotalFiles = 0;
	int				CurFile;
	char			**FileList = NULL;

	// ensure valid data
	assert( ResourceMgr != NULL );
	assert( ResourceName != NULL );
	assert( FileFilter != NULL );
	assert( FileListSize );

	// get vfile dir
	FileDir = jeResource_GetVFile( ResourceMgr, ResourceName );
	if ( FileDir == NULL )
	{
		jeErrorLog_Add( JE_ERR_SUBSYSTEM_FAILURE, NULL );
		return NULL;
	}

	// create directory finder
	Finder = jeVFile_CreateFinder( FileDir, FileFilter );
	if ( Finder == NULL )
	{
		goto ERROR_Util_BuildFileList;
	}

	// determine how many files there are
	TotalFiles = 1;
	while ( jeVFile_FinderGetNextFile( Finder ) == JE_TRUE )
	{
		TotalFiles++;
	}

	// destroy finder
	jeVFile_DestroyFinder( Finder );
	Finder = NULL;

	// allocate file list
	FileList = (char **)jeRam_AllocateClear( sizeof( char * ) * TotalFiles );
	if ( FileList == NULL )
	{
		jeErrorLog_Add( JE_ERR_MEMORY_RESOURCE, NULL );
		goto ERROR_Util_BuildFileList;
	}

	// create directory finder
	Finder = jeVFile_CreateFinder( FileDir, FileFilter );
	if ( Finder == NULL )
	{
		jeErrorLog_Add( JE_ERR_SUBSYSTEM_FAILURE, NULL );
		goto ERROR_Util_BuildFileList;
	}

	// first entry is always the "no selection" slot
	CurFile = 0;
	FileList[CurFile++] = Util_StrDup( NoSelection );

	// build file list
	while ( jeVFile_FinderGetNextFile( Finder ) == JE_TRUE )
	{

		// locals
		jeVFile_Properties	Properties;

		// get properties of current file
		if( jeVFile_FinderGetProperties( Finder, &Properties ) == JE_FALSE )
		{
			jeErrorLog_Add( JE_ERR_SUBSYSTEM_FAILURE, NULL );
			continue;
		}

		// save file name
		assert( CurFile < TotalFiles );
		FileList[CurFile] = Util_StrDup( Properties.Name );
		if ( FileList[CurFile] != NULL )
		{
			CurFile++;
		}
	}

	// destroy finder
	jeVFile_DestroyFinder( Finder );

	// close vfile dir
	if ( jeResource_DeleteVFile( ResourceMgr, ResourceName ) == 0 )
	{
		jeVFile_Close( FileDir );
	}

	// return file list
	*FileListSize = CurFile;
	return FileList;

	// error handling
	ERROR_Util_BuildFileList:

	// destroy file list
	if ( FileList != NULL )
	{
		CurFile = 0;
		while ( CurFile < TotalFiles )
		{
			if ( FileList[CurFile] != NULL )
			{
				jeRam_Free( FileList[CurFile] );
			}
			CurFile++;
		}
		jeRam_Free( FileList );
	}

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

	// zap file list size
	*FileListSize = 0;

	// return failure
	return NULL;

} // Util_BuildFileList()



////////////////////////////////////////////////////////////////////////////////////////
//
//	Util_LoadLibraryString()
//
////////////////////////////////////////////////////////////////////////////////////////

#ifdef BUILD_BE

#include <Resources.h>
#include <image.h>

static char *Util_LoadLibraryString(image_id libhinst, int32 resid)
{
	BResources resourcefile;
	int result;
	char *rcbuffer;
 	image_info info;
	size_t outSize;
	
	assert(libhinst > 0);
	assert(resid);

///	hResources = (image_id)hStringResources ;
	
	if(get_image_info(libhinst,&info) != B_OK)
		return NULL;
		
	BFile* resFile = new BFile(info.name , B_READ_ONLY);
	
	resourcefile.SetTo(resFile,false);

	char* loadedString = (char *)resourcefile.FindResource('DATA', resid, &outSize);
	
	//
	//	Note that if we did't allocate space and copy the string, then we
	//	would be limited to having one string loaded at a time. Or we would
	//	setup some kind of revolving buffer.  Either of these options is
	//	risky and could eventually cause a problem elsewhere... 	 LF
	//
 
	// Allocate memory for the string
	rcbuffer = (char *)malloc(strlen(loadedString) + 1); //(char*)jeRam_Allocate(strlen(loadedString) + 1);
	strcpy(rcbuffer, loadedString);
 
//#ifndef NDEBUG
//	memset(stringbuffer, 0xFF, UTIL_MAX_RESOURCE_LENGTH + 1);
//#endif
 
	// return the allocated string
	return (rcbuffer);
}//Util_LoadLibraryString

#endif

#ifdef WIN32

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
		jeErrorLog_Add( JE_ERR_WINDOWS_API_FAILURE, NULL );
		return NULL;
	}

	// copy resource string
	NewString = (char *)jeRam_Allocate( Size + 1 );
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
