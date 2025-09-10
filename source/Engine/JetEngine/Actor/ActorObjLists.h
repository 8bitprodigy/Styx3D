////////////////////////////////////////////////////////////////////////////////////////
//
//	ActorObj_DestroyMotionList()
//
////////////////////////////////////////////////////////////////////////////////////////
static void ActorObj_DestroyMotionList(
	jeActor* Actor )	// object whose motion list will be destroyed
{

	// locals
	int	i;
	ActorObj* Object;

	// ensure valid data
	assert( Actor != NULL );
	Object = Actor->Object;
	assert( Object != NULL );
	assert( Object->MotionList != NULL );
	assert( Object->MotionListSize > 0 );

	// free strings
	for ( i = 0; i < Object->MotionListSize; i++ )
	{
		assert( Object->MotionList[i] != NULL );
		jeRam_Free( Object->MotionList[i] );
	}

	// free list
	jeRam_Free( Object->MotionList );

	// reset related data fields
	Object->MotionList = NULL;
	Object->MotionListSize = 0;
	Object->Motion = NULL;
	Object->MotionTime = 0.0f;
	if ( Object->MotionName != NULL )
	{
		jeRam_Free( Object->MotionName );
		Object->MotionName = NULL;
	}

} // ActorObj_DestroyMotionList()



////////////////////////////////////////////////////////////////////////////////////////
//
//	ActorObj_DestroyBoneList()
//
////////////////////////////////////////////////////////////////////////////////////////
static void ActorObj_DestroyBoneList(
	jeActor* Actor)	// object whose bone list will be destroyed
{

	// locals
	int	i;
	ActorObj* Object;

	// ensure valid data
	assert( Actor != NULL );
	Object = Actor->Object;
	assert( Object != NULL );

	// zap string pointers
	assert( Object->BoneList != NULL );
	assert( Object->BoneListSize > 0 );
	for ( i = 0; i < Object->BoneListSize; i++ )
	{
		Object->BoneList[i] = NULL;
	}

	// free list
	jeRam_Free( Object->BoneList );

	// free current bone name
	assert( Object->LightReferenceBoneName != NULL );
	jeRam_Free( Object->LightReferenceBoneName );
	Object->LightReferenceBoneName = NULL;

	// reset related data fields
	Object->BoneList = NULL;
	Object->BoneListSize = 0;

} // ActorObj_DestroyBoneList()



////////////////////////////////////////////////////////////////////////////////////////
//
//	ActorObj_DestroyMaterialList()
//
////////////////////////////////////////////////////////////////////////////////////////
static void ActorObj_DestroyMaterialList(
	jeActor	*Actor )	// object whose material list will be destroyed
{

	// locals
	int	i;
	ActorObj* Object;

	// ensure valid data
	assert( Actor != NULL );
	
	Object = Actor->Object;


	// zap data
	for ( i = 0; i < Object->MaterialListSize; i++ )
	{

		// zap material string pointers
		if ( Object->MaterialList != NULL )
		{
			Object->MaterialList[i] = NULL;
		}

		// zap overide material bitmaps
		if ( Object->MaterialOverideBitmap != NULL )
		{
			if ( Object->MaterialOverideBitmap[i] != NULL )
			{
				//undone
				Util_ResetActorMaterialToDefault( Actor, Actor->ActorDefinition, i );
				/*jeEngine_RemoveBitmap( Object->Engine, Object->MaterialOverideBitmap[i] );
				if ( jeResource_Delete( Object->ResourceMgr, Object->MaterialOverideList[i] ) == 0 )
				{
					jeBitmap_Destroy( &( Object->MaterialOverideBitmap[i] ) );
				}*/
				Object->MaterialOverideBitmap[i] = NULL;
			}
		}

		// zap overide material string pointers
		if ( Object->MaterialOverideList != NULL )
		{
			Object->MaterialOverideList[i] = NULL;
		}

		// zap material mapper strings
		if ( Object->MaterialMapperList != NULL )
		{
			Object->MaterialMapperList[i] = NULL;
		}
	}

	// free lists
	if ( Object->MaterialList != NULL )
	{
		jeRam_Free( Object->MaterialList );
		Object->MaterialList = NULL;
	}
	if ( Object->MaterialOverideList != NULL )
	{
		jeRam_Free( Object->MaterialOverideList );
		Object->MaterialOverideList = NULL;
	}
	if ( Object->MaterialOverideBitmap != NULL )
	{
		jeRam_Free( Object->MaterialOverideBitmap );
		Object->MaterialOverideBitmap = NULL;
	}
	if ( Object->MaterialMapperList != NULL )
	{
		jeRam_Free( Object->MaterialMapperList );
		Object->MaterialMapperList = NULL;
	}

	// reset related data fields
	Object->MaterialCurrent = 0;
	Object->MaterialListSize = 0;

} // ActorObj_DestroyMaterialList()



////////////////////////////////////////////////////////////////////////////////////////
//
//	ActorObj_CreateMotionList()
//
////////////////////////////////////////////////////////////////////////////////////////
static jeBoolean ActorObj_CreateMotionList(
	jeActor	*Actor )	// object whose motion list will be created
{

	// locals
	const char	*MotionName;
	int			i;
	ActorObj* Object;

	// ensure valid data
	assert( Actor != NULL );
	Object = Actor->Object;

	// get motion count
	Object->MotionListSize = 1;
	if ( Actor->ActorDefinition != NULL )
	{
		Object->MotionListSize += jeActor_GetMotionCount( Actor->ActorDefinition );
	}

	// allocate motion list
	Object->MotionList = (char **)jeRam_Allocate( sizeof( char * ) * Object->MotionListSize );
	if ( Object->MotionList == NULL )
	{
		jeErrorLog_Add( JE_ERR_MEMORY_RESOURCE, NULL );
		Object->MotionListSize = 0;
		return JE_FALSE;
	}

	// first entry is always the "no selection" slot
	Object->MotionList[0] = Util_StrDup( NoSelection );

	// init motion list
	if ( Actor->ActorDefinition != NULL )
	{
		for ( i = 1; i < Object->MotionListSize; i++ )
		{
			MotionName = jeActor_GetMotionName( Actor->ActorDefinition, i - 1 );
			assert( MotionName != NULL );
			Object->MotionList[i] = Util_StrDup( MotionName );
		}
	}

	// set default motion
	assert( Object->MotionName == NULL );
	Object->MotionName = Util_StrDup( Object->MotionList[0] );
	jeProperty_FillCombo(	&( ActorObjPropertyList.pjeProperty[ACTOROBJ_MOTIONLIST_INDEX] ),
							IDS_MOTIONLIST,
							Object->MotionList[0],
							ACTOROBJ_MOTIONLIST_ID,
							Object->MotionListSize,
							Object->MotionList );

	// all done
	return JE_TRUE;

} // ActorObj_CreateMotionList()



////////////////////////////////////////////////////////////////////////////////////////
//
//	ActorObj_CreateBoneList()
//
////////////////////////////////////////////////////////////////////////////////////////
static jeBoolean ActorObj_CreateBoneList(
	jeActor* Actor )	// object whose bone list will be created
{

	// locals
	jeBody		*Body = NULL;
	int			i;
	jeXForm3d	Xf;
	int			ParentBoneIndex;
	ActorObj* Object;

	// ensure valid data
	assert( Actor != NULL );
	Object = Actor->Object;

	// get actor body
	if ( Actor->ActorDefinition != NULL )
	{
		Body = jeActor_GetBody( Actor->ActorDefinition );
	}

	// get bone count
	Object->BoneListSize = 1;
	if ( Body != NULL )
	{
		Object->BoneListSize += jeBody_GetBoneCount( Body );
	}

	// allocate bone list
	assert( Object->BoneList == NULL );
	Object->BoneList = (char **)jeRam_Allocate( sizeof( char * ) * Object->BoneListSize );
	if ( Object->BoneList == NULL )
	{
		jeErrorLog_Add( JE_ERR_MEMORY_RESOURCE, NULL );
		return JE_FALSE;
	}

	// first entry is always the "no selection" slot
	Object->BoneList[0] = Util_StrDup( NoSelection );

	// save bone names
	if ( Body != NULL )
	{
		for ( i = 1; i < Object->BoneListSize; i++ )
		{
			jeBody_GetBone( Body, i - 1, (const char **)&( Object->BoneList[i] ), &Xf, &ParentBoneIndex );
			assert( Object->BoneList[i] != NULL );
		}
	}

	// set default bone
	assert( Object->LightReferenceBoneName == NULL );
	Object->LightReferenceBoneName = Util_StrDup( Object->BoneList[0] );
	jeProperty_FillCombo(	&( ActorObjPropertyList.pjeProperty[ACTOROBJ_LIGHTREFERENCEBONENAMELIST_INDEX] ),
							IDS_BONELIST,
							Object->LightReferenceBoneName,
							ACTOROBJ_LIGHTREFERENCEBONENAMELIST_ID,
							Object->BoneListSize,
							Object->BoneList );

	// all done
	return JE_TRUE;

} // ActorObj_CreateBoneList()



////////////////////////////////////////////////////////////////////////////////////////
//
//	ActorObj_CreateMaterialList()
//
////////////////////////////////////////////////////////////////////////////////////////
static jeBoolean ActorObj_CreateMaterialList(
	jeActor* Actor )	// object whose material list will be created
{

	// locals
	jeBody	*Body;
	int		i;
	ActorObj* Object;

	// ensure valid data
	assert( Actor != NULL );
	assert( Actor->Object != NULL );
	Object = Actor->Object;

	// reset current material number
	Object->MaterialCurrent = 0;

	// prepare material list if required
	Object->MaterialListSize = jeActor_GetMaterialCount( Actor );
	if ( Object->MaterialListSize <= 0 )
	{
		goto ERROR_ActorObj_CreateMaterialList;
	}

	// allocate lists
	Object->MaterialList = (char **)jeRam_AllocateClear( sizeof( char * ) * Object->MaterialListSize );
	Object->MaterialOverideList = (char **)jeRam_AllocateClear( sizeof( char * ) * Object->MaterialListSize );
	Object->MaterialMapperList = (char **)jeRam_AllocateClear( sizeof( char * ) * Object->MaterialListSize );
	Object->MaterialOverideBitmap = (jeMaterialSpec **)jeRam_AllocateClear( sizeof ( jeMaterialSpec * ) * Object->MaterialListSize );
	if (	( Object->MaterialList == NULL ) ||
			( Object->MaterialOverideList == NULL ) ||
			( Object->MaterialOverideBitmap == NULL ) ||
			( Object->MaterialMapperList == NULL ) )

	{
		goto ERROR_ActorObj_CreateMaterialList;
	}

	// get actor body
	Body = jeActor_GetBody( Actor->ActorDefinition );
	if ( Body == NULL )
	{
		goto ERROR_ActorObj_CreateMaterialList;
	}

	// prepare lists
	for ( i = 0; i < Object->MaterialListSize; i++ )
	{

		// locals
		jeBoolean	GotMaterial;
		jeMaterialSpec	*Bitmap;
		jeFloat		Red, Green, Blue;
		jeUVMapper	Mapper;

		// setup material name list
		GotMaterial = jeBody_GetMaterial(	Body, i, (const char **)&( Object->MaterialList[i] ), 
											&Bitmap, &Red, &Green, &Blue, &Mapper );
		if ( GotMaterial == JE_FALSE )
		{
			goto ERROR_ActorObj_CreateMaterialList;
		}

		// setup overide name list
		Object->MaterialOverideList[i] = Bitmaps->Name[0];

		// setup mapper name list
		Object->MaterialMapperList[i] = MaterialMapperTable[0].Name;
	}

	// setup defaults
	jeProperty_FillCombo(	&( ActorObjPropertyList.pjeProperty[ACTOROBJ_MATERIALLIST_INDEX] ),
							IDS_MATERIALLIST,
							Object->MaterialList[Object->MaterialCurrent],
							ACTOROBJ_MATERIALLIST_ID,	
							Object->MaterialListSize,
							Object->MaterialList );
	jeProperty_FillCombo(	&( ActorObjPropertyList.pjeProperty[ACTOROBJ_MATERIALOVERIDE_INDEX] ),
							IDS_MATERIALOVERIDE,
							Object->MaterialOverideList[Object->MaterialCurrent],
							ACTOROBJ_MATERIALOVERIDE_ID,	
							Bitmaps->Total,
							Bitmaps->Name );
	jeProperty_FillCombo(	&( ActorObjPropertyList.pjeProperty[ACTOROBJ_MATERIALMAPPER_INDEX] ),
							IDS_MATERIALMAPPER,
							Object->MaterialMapperList[0],
							ACTOROBJ_MATERIALMAPPER_ID,
							MaterialMapperTableSize,
							MaterialMapperNameList );

	// all done
	return JE_TRUE;

	// handle errors
	ERROR_ActorObj_CreateMaterialList:

	// destroy all created stuff
	ActorObj_DestroyMaterialList( Actor );

	// return failure
	return JE_FALSE;

} // ActorObj_CreateMaterialList()


////////////////////////////////////////////////////////////////////////////////////////
//
//	ActorObj_ResetMotionList()
//
////////////////////////////////////////////////////////////////////////////////////////
static void ActorObj_ResetMotionList(
	jeActor *Actor )	// object whose motion list will be reset
{
	ActorObj* Object;

	// ensure valid data
	assert( Actor != NULL );
	Object = Actor->Object;

	// destroy any existing motion list 
	if ( Object->MotionListSize > 0 )
	{
		ActorObj_DestroyMotionList( Actor );
	}

	// create empty list
	Util_CreateEmptyList( &( Object->MotionList ), &( Object->MotionListSize ) );

	// set combo box choice
	jeProperty_FillCombo(	&( ActorObjPropertyList.pjeProperty[ACTOROBJ_MOTIONLIST_INDEX] ),
							IDS_MOTIONLIST,
							Object->MotionList[0],
							ACTOROBJ_MOTIONLIST_ID,
							Object->MotionListSize,
							Object->MotionList );

	// set default choice
	if ( Object->MotionName != NULL )
	{
		jeRam_Free( Object->MotionName );
	}
	Object->MotionName = Util_StrDup( Object->MotionList[0] );


} // ActorObj_ResetMotionList()



////////////////////////////////////////////////////////////////////////////////////////
//
//	ActorObj_ResetBoneList()
//
////////////////////////////////////////////////////////////////////////////////////////
static void ActorObj_ResetBoneList(
	jeActor* Actor)	// object whose bone list will be reset
{
	ActorObj* Object;

	// ensure valid data
	assert( Actor != NULL );
	Object = Actor->Object;

	// destroy any existing bone list 
	if ( Object->BoneListSize > 0 )
	{
		ActorObj_DestroyBoneList( Actor );
	}

	// create empty list
	Util_CreateEmptyList( &( Object->BoneList ), &( Object->BoneListSize ) );

	// set combo box choice
	jeProperty_FillCombo(	&( ActorObjPropertyList.pjeProperty[ACTOROBJ_LIGHTREFERENCEBONENAMELIST_INDEX] ),
							IDS_BONELIST,
							Object->LightReferenceBoneName,
							ACTOROBJ_LIGHTREFERENCEBONENAMELIST_ID,
							Object->BoneListSize,
							Object->BoneList );

	// set default choice
	if ( Object->LightReferenceBoneName != NULL )
	{
		jeRam_Free( Object->LightReferenceBoneName );
	}
	Object->LightReferenceBoneName = Util_StrDup( Object->BoneList[0] );

} // ActorObj_ResetBoneList()



////////////////////////////////////////////////////////////////////////////////////////
//
//	ActorObj_ResetMaterialList()
//
////////////////////////////////////////////////////////////////////////////////////////
static void ActorObj_ResetMaterialList(
	jeActor *Actor )	// object whose material list will be reset
{
	ActorObj* Object;

	// ensure valid data
	assert( Actor != NULL );
	Object = Actor->Object;

	// destroy any existing material list 
	ActorObj_DestroyMaterialList( Actor );

	// create empty lists
	Util_CreateEmptyList( &( Object->MaterialList ), &( Object->MaterialListSize ) );
	Util_CreateEmptyList( &( Object->MaterialOverideList ), &( Object->MaterialListSize ) );
	Util_CreateEmptyList( &( Object->MaterialMapperList ), &( Object->MaterialListSize ) );

	// set combo box defaults
	jeProperty_FillCombo(	&( ActorObjPropertyList.pjeProperty[ACTOROBJ_MATERIALLIST_INDEX] ),
							IDS_MATERIALLIST,
							Object->MaterialList[0],
							ACTOROBJ_MATERIALLIST_ID,	
							Object->MaterialListSize,
							Object->MaterialList );
	jeProperty_FillCombo(	&( ActorObjPropertyList.pjeProperty[ACTOROBJ_MATERIALOVERIDE_INDEX] ),
							IDS_MATERIALOVERIDE,
							Object->MaterialOverideList[0],
							ACTOROBJ_MATERIALOVERIDE_ID,	
							Object->MaterialListSize,
							Object->MaterialOverideList );
	jeProperty_FillCombo(	&( ActorObjPropertyList.pjeProperty[ACTOROBJ_MATERIALMAPPER_INDEX] ),
							IDS_MATERIALMAPPER,
							Object->MaterialMapperList[0],
							ACTOROBJ_MATERIALMAPPER_ID,
							MaterialMapperTableSize,
							MaterialMapperNameList );

} // ActorObj_ResetMaterialList()

void InitObjectProperties(jeActor *Actor)
{	
	ActorObj* Object;
	
	Object = Actor->Object;

	jeExtBox_Set(&(Object->CollisionExtBox), -5.0f,-5.0f,-5.0f,5.0f,5.0f,5.0f);
	jeExtBox_Set(&(Object->RenderHintExtBox), -5.0f,-5.0f,-5.0f,5.0f,5.0f,5.0f);

	// Init strings
	Object->MotionName = NULL;
	Object->ActorDefName = NULL;

	// get default settings
	Object->UseFillLight = ACTOROBJ_DEFAULT_USEFILLLIGHT;
	Object->FillNormalActorRelative = ACTOROBJ_DEFAULT_FILLLIGHTNORMALACTORRELATIVE;
	jeVec3d_Set( &( Object->FillLightNormal ), ACTOROBJ_DEFAULT_FILLNORMALX, ACTOROBJ_DEFAULT_FILLNORMALY, ACTOROBJ_DEFAULT_FILLNORMALZ );
	Object->FillLightRed = ACTOROBJ_DEFAULT_FILLLIGHTRED;
	Object->FillLightGreen = ACTOROBJ_DEFAULT_FILLLIGHTGREEN;
	Object->FillLightBlue = ACTOROBJ_DEFAULT_FILLLIGHTBLUE;
	Object->AmbientLightRed = ACTOROBJ_DEFAULT_AMBIENTLIGHTRED;
	Object->AmbientLightGreen = ACTOROBJ_DEFAULT_AMBIENTLIGHTGREEN;
	Object->AmbientLightBlue = ACTOROBJ_DEFAULT_AMBIENTLIGHTBLUE;
	Object->UseAmbientLightFromFloor = ACTOROBJ_DEFAULT_USEAMBFROMFLOOR;
	Object->CollisionExtBoxDisplay = ACTOROBJ_DEFAULT_COLLISIONEXTBOXDISPLAY;
	Object->RenderExtBoxDisplay = ACTOROBJ_DEFAULT_RENDEREXTBOXDISPLAY;
	Object->MaximumDynamicLightsToUse = ACTOROBJ_DEFAULT_MAXDYNAMICLIGHTS;
	Object->MaximumStaticLightsToUse = ACTOROBJ_DEFAULT_MAXSTATICLIGHTS;
	Object->PerBoneLighting = ACTOROBJ_DEFAULT_PERBONELIGHTING;
	Object->ScaleX = ACTOROBJ_DEFAULT_SCALEX;
	Object->ScaleY = ACTOROBJ_DEFAULT_SCALEY;
	Object->ScaleZ = ACTOROBJ_DEFAULT_SCALEZ;
	Object->MotionTimeScale = ACTOROBJ_DEFAULT_MOTIONTIMESCALE;

	// init remaining fields
	jeXForm3d_SetIdentity( &( Actor->Xf ) );
	Object->RefCount = 1;

	// create empty actor def list
	Object->ActorDefName = Util_StrDup( NoSelection );
	jeProperty_FillCombo(	&( ActorObjPropertyList.pjeProperty[ACTOROBJ_LIST_INDEX] ),
							IDS_ACTORLIST,
							NoSelection,
							ACTOROBJ_LIST_ID,
							1,
							&NoSelection );

	// create empty motion list
	ActorObj_ResetMotionList( Actor );

	// create empty bone list
	ActorObj_ResetBoneList( Actor );

	// create empty matarial list
	ActorObj_ResetMaterialList( Actor );
}
