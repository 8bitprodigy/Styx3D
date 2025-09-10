#include <float.h>

#ifdef BUILD_BE
#define stricmp strcasecmp
#define HINSTANCE image_id
#endif

////////////////////////////////////////////////////////////////////////////////////////
//	Property list stuff
////////////////////////////////////////////////////////////////////////////////////////
#define    IDS_USEFILLLIGHT        "Use fill light:"
#define    IDS_FILLLIGHTNORMAL     "FN:"
#define    IDS_USEAMBIENTLIGHTFROMFLOOR "Use ambient light from floor:"
#define    IDS_MAXIMUMDYNAMICLIGHTSTOUSE "D Lights:"
#define    IDS_MAXIMUMSTATICLIGHTSTOUSE "S Lights:"
#define    IDS_LIGHTREFERENCEBONENAME "L Bone:"
#define    IDS_PERBONELIGHTING     "Per bone lighting:"
#define    IDS_FILLLIGHTNORMALX    "X Deg:"
#define    IDS_FILLLIGHTNORMALY    "Y Deg:"

#define    IDS_FILLLIGHTNORMALZ				"Z Deg:"
#define    IDS_AMBIENTLIGHT					"Color Picker"
#define    IDS_FILLLIGHT						"Color Picker"
#define    IDS_SCALEX							"Scale:"
#define    IDS_SCALEY							"Y:"
#define IDS_SCALEZ							"Z:"
#define    IDS_ACTORLIST						"Actor:"
#define    IDS_MOTIONLIST						"Motion:"
#define    IDS_BONELIST						"LBone:"
#define	IDS_COLLISIONEXTBOXGROUP			"Collision Box"
#define	IDS_COLLISIONEXTBOXDISPLAY			"Display Collision Box:"
#define	IDS_COLLISIONEXTBOXMINX				"XMin:"
#define	IDS_COLLISIONEXTBOXMINY				"YMin:"
#define	IDS_COLLISIONEXTBOXMINZ				"ZMin:"
#define	IDS_COLLISIONEXTBOXMAXX				"XMax:"
#define	IDS_COLLISIONEXTBOXMAXY				"YMax:"
#define	IDS_COLLISIONEXTBOXMAXZ				"ZMax:"
#define	IDS_RENDEREXTBOXGROUP				"Render Box"
#define	IDS_RENDEREXTBOXDISPLAY				"Display Render Box:"
#define	IDS_RENDEREXTBOXMINX				"XMin:"
#define	IDS_RENDEREXTBOXMINY				"YMin:"
#define	IDS_RENDEREXTBOXMINZ				"ZMin:"
#define	IDS_RENDEREXTBOXMAXX				"XMax:"
#define	IDS_RENDEREXTBOXMAXY				"YMax:"
#define	IDS_RENDEREXTBOXMAXZ				"ZMax:"
#define	IDS_SCALEGROUP						"Scale"
#define	IDS_FILLLIGHTGROUP					"Fill"
#define	IDS_AMBIENTLIGHTGROUP				"Ambient"
#define	IDS_AMBIENTLIGHTRED					"Red:"
#define	IDS_AMBIENTLIGHTGREEN				"Green:"
#define	IDS_AMBIENTLIGHTBLUE				"Blue:"
#define	IDS_FILLLIGHTRED					"Red:"
#define	IDS_FILLLIGHTGREEN					"Green:"
#define	IDS_FILLLIGHTBLUE					"Blue:"
#define	IDS_FILLLIGHTNORNALACTORRELATIVE	"FN Actor Relative:"
#define	IDS_MOTIONTIMESCALE					"MT Scale:"
#define	IDS_MATERIALGROUP					"Materials"
#define	IDS_MATERIALLIST					"Name:"
#define	IDS_MATERIALOVERIDE					"Override:"
#define	IDS_MATERIALRED						"Red:"
#define	IDS_MATERIALGREEN					"Green:"
#define	IDS_MATERIALBLUE					"Blue:"
#define	IDS_MATERIALMAPPER					"Mapper:"


////////////////////////////////////////////////////////////////////////////////////////
//	Globals
////////////////////////////////////////////////////////////////////////////////////////
static HINSTANCE		hClassInstance = NULL;
static char				**ActorDefList = NULL;
static int				ActorDefListSize = 0;
static jeProperty		ActorObjProperties[ACTOROBJ_LAST_INDEX];
static jeProperty_List	ActorObjPropertyList = { ACTOROBJ_LAST_INDEX, &( ActorObjProperties[0] ) };

////////////////////////////////////////////////////////////////////////////////////////
//	UV material mapper setup
////////////////////////////////////////////////////////////////////////////////////////
typedef jeBoolean JETCC UVMAP( const jeXForm3d* pXForm, jeLVertex* pVerts, const jeVec3d* pNormals, int nverts );
typedef struct
{
	int		Index;
	char	*Name;
	UVMAP	*Mapper;

} Mapper_Table;
static Mapper_Table	MaterialMapperTable[] = 
{
	{ 0, "< none >", NULL },
	{ 1, "Reflection", jeUVMap_Reflection },
	{ 2, "Refraction", jeUVMap_Refraction },
	{ 3, "Projection", jeUVMap_Projection }
};
static char ** MaterialMapperNameList = NULL;
#define MaterialMapperTableSize	( sizeof( MaterialMapperTable ) / sizeof( MaterialMapperTable[0] ) )

////////////////////////////////////////////////////////////////////////////////////////
//	Defaults
////////////////////////////////////////////////////////////////////////////////////////
#define ACTOROBJ_VERSION_NUMBER							1.0
#define ACTOROBJ_DEFAULT_AMBIENTLIGHTRED				128.0f
#define ACTOROBJ_DEFAULT_AMBIENTLIGHTGREEN				128.0f
#define ACTOROBJ_DEFAULT_AMBIENTLIGHTBLUE				128.0f
#define ACTOROBJ_DEFAULT_FILLLIGHTRED					255.0f
#define ACTOROBJ_DEFAULT_FILLLIGHTGREEN					255.0f
#define ACTOROBJ_DEFAULT_FILLLIGHTBLUE					255.0f
#define ACTOROBJ_DEFAULT_MAXDYNAMICLIGHTS				3
#define ACTOROBJ_DEFAULT_MAXSTATICLIGHTS				0
#define ACTOROBJ_DEFAULT_SCALEX							1.0f
#define ACTOROBJ_DEFAULT_SCALEY							1.0f
#define ACTOROBJ_DEFAULT_SCALEZ							1.0f
#define ACTOROBJ_DEFAULT_PERBONELIGHTING				JE_FALSE
#define ACTOROBJ_DEFAULT_USEFILLLIGHT					JE_TRUE
#define ACTOROBJ_DEFAULT_FILLLIGHTNORMALACTORRELATIVE	JE_TRUE
#define ACTOROBJ_DEFAULT_USEAMBFROMFLOOR				JE_FALSE
#define ACTOROBJ_DEFAULT_COLLISIONEXTBOXDISPLAY			JE_FALSE
#define ACTOROBJ_DEFAULT_RENDEREXTBOXDISPLAY			JE_FALSE
#define ACTOROBJ_DEFAULT_FILLNORMALX					30.0f
#define ACTOROBJ_DEFAULT_FILLNORMALY					45.0f
#define ACTOROBJ_DEFAULT_FILLNORMALZ					0.0f
#define ACTOROBJ_DEFAULT_MOTIONTIMESCALE				1.0f
#define ACTOROBJ_DEFAULT_EXTBOXMINX						-40.0f
#define ACTOROBJ_DEFAULT_EXTBOXMINY						-40.0f
#define ACTOROBJ_DEFAULT_EXTBOXMINZ						-40.0f
#define ACTOROBJ_DEFAULT_EXTBOXMAXX						40.0f
#define ACTOROBJ_DEFAULT_EXTBOXMAXY						40.0f
#define ACTOROBJ_DEFAULT_EXTBOXMAXZ						40.0f

//Royce
#define OBJ_PERSIST_SIZE 10000
//----

#include "ActorObjLists.h"

void FillProperties(void)
{
	// setup X scale property
	jeProperty_FillFloat(	&( ActorObjProperties[ACTOROBJ_SCALEX_INDEX] ),
							( IDS_SCALEX ),
							ACTOROBJ_DEFAULT_SCALEX,
							ACTOROBJ_SCALEX_ID,
							0.00005f, FLT_MAX, 0.1f );

	// setup Y scale property
	/*jeProperty_FillFloat(	&( ActorObjProperties[ACTOROBJ_SCALEY_INDEX] ),
							( IDS_SCALEY ),
							ACTOROBJ_DEFAULT_SCALEY,
							ACTOROBJ_SCALEY_ID,
							0.00005f, FLT_MAX, 0.1f );

	// setup Z scale property
	jeProperty_FillFloat(	&( ActorObjProperties[ACTOROBJ_SCALEZ_INDEX] ),
							( IDS_SCALEZ ),
							ACTOROBJ_DEFAULT_SCALEZ,
							ACTOROBJ_SCALEZ_ID,
							0.00005f, FLT_MAX, 0.1f );*/

	// end scale box group
	//jeProperty_FillGroupEnd( &( ActorObjProperties[ACTOROBJ_SCALEGROUPEND_INDEX] ), ACTOROBJ_SCALEGROUPEND_ID );


	////////////////////////////////////////////////////////////////////////////////////////
	//	Fill light properties
	////////////////////////////////////////////////////////////////////////////////////////

	// start fill light group
	{
		jeVec3d	Color = { ACTOROBJ_DEFAULT_FILLLIGHTRED, ACTOROBJ_DEFAULT_FILLLIGHTGREEN, ACTOROBJ_DEFAULT_FILLLIGHTBLUE };
		jeProperty_FillColorGroup(	&( ActorObjProperties[ACTOROBJ_FILLIGHTGROUP_INDEX] ),
									( IDS_FILLLIGHTGROUP ),
									&Color,
									ACTOROBJ_FILLIGHTGROUP_ID );
	}

	// setup fill light property
	{
		jeVec3d	Color = { ACTOROBJ_DEFAULT_FILLLIGHTRED, ACTOROBJ_DEFAULT_FILLLIGHTGREEN, ACTOROBJ_DEFAULT_FILLLIGHTBLUE };
		jeProperty_FillColorPicker(	&( ActorObjProperties[ACTOROBJ_FILLLIGHT_INDEX] ),
									( IDS_FILLLIGHT ),
									&Color,
									ACTOROBJ_FILLLIGHT_ID );
	}

	// setup use fill light property
	jeProperty_FillCheck(	&( ActorObjProperties[ACTOROBJ_FILLLIGHTUSE_INDEX] ),
							( IDS_USEFILLLIGHT ),
							ACTOROBJ_DEFAULT_USEFILLLIGHT,
							ACTOROBJ_FILLLIGHTUSE_ID );

	// setup fill normal actor relative property
	jeProperty_FillCheck(	&( ActorObjProperties[ACTOROBJ_FILLLIGHTNORMALACTORRELATIVE_INDEX] ),
							( IDS_FILLLIGHTNORNALACTORRELATIVE ),
							ACTOROBJ_DEFAULT_FILLLIGHTNORMALACTORRELATIVE,
							ACTOROBJ_FILLLIGHTNORMALACTORRELATIVE_ID );

	// setup fill light red property
	jeProperty_FillFloat(	&( ActorObjProperties[ACTOROBJ_FILLLIGHTRED_INDEX] ),
							( IDS_FILLLIGHTRED ),
							ACTOROBJ_DEFAULT_FILLLIGHTRED,
							ACTOROBJ_FILLLIGHTRED_ID,
							0.0f, 255.0f, 1.0f );

	// setup fill light green property
	jeProperty_FillFloat(	&( ActorObjProperties[ACTOROBJ_FILLLIGHTGREEN_INDEX] ),
							( IDS_FILLLIGHTGREEN ),
							ACTOROBJ_DEFAULT_FILLLIGHTGREEN,
							ACTOROBJ_FILLLIGHTGREEN_ID,
							0.0f, 255.0f, 1.0f );

	// setup fill light blue property
	jeProperty_FillFloat(	&( ActorObjProperties[ACTOROBJ_FILLLIGHTBLUE_INDEX] ),
							( IDS_FILLLIGHTBLUE ),
							ACTOROBJ_DEFAULT_FILLLIGHTBLUE,
							ACTOROBJ_FILLLIGHTBLUE_ID,
							0.0f, 255.0f, 1.0f );

	// setup fill light normal property
	{
		jeVec3d	Vect = { ACTOROBJ_DEFAULT_FILLNORMALX, ACTOROBJ_DEFAULT_FILLNORMALY, ACTOROBJ_DEFAULT_FILLNORMALZ };
		jeProperty_FillVec3dGroup(	&( ActorObjProperties[ACTOROBJ_FILLLIGHTNORMAL_INDEX] ),
									( IDS_FILLLIGHTNORMAL ),
									&Vect,
									ACTOROBJ_FILLLIGHTNORMAL_ID );
	}

	// setup fill light normal X property
	jeProperty_FillFloat(	&( ActorObjProperties[ACTOROBJ_FILLLIGHTNORMALX_INDEX] ),
							( IDS_FILLLIGHTNORMALX ),
							ACTOROBJ_DEFAULT_FILLNORMALX,
							ACTOROBJ_FILLLIGHTNORMALX_ID,
							0.0f, 359.0f, 1.0f );

	// setup fill light normal Y property
	jeProperty_FillFloat(	&( ActorObjProperties[ACTOROBJ_FILLLIGHTNORMALY_INDEX] ),
							( IDS_FILLLIGHTNORMALY ),
							ACTOROBJ_DEFAULT_FILLNORMALY,
							ACTOROBJ_FILLLIGHTNORMALY_ID,
							0.0f, 359.0f, 1.0f );

	// setup fill light normal Z property
	jeProperty_FillFloat(	&( ActorObjProperties[ACTOROBJ_FILLLIGHTNORMALZ_INDEX] ),
							( IDS_FILLLIGHTNORMALZ ),
							ACTOROBJ_DEFAULT_FILLNORMALZ,
							ACTOROBJ_FILLLIGHTNORMALZ_ID,
							0.0f, 359.0f, 1.0f );

	// end fill light normal property
	jeProperty_FillGroupEnd( &( ActorObjProperties[ACTOROBJ_FILLLIGHTNORMALEND_INDEX] ), ACTOROBJ_FILLLIGHTNORMALEND_ID );

	// end fill light group
	jeProperty_FillGroupEnd( &( ActorObjProperties[ACTOROBJ_FILLIGHTGROUPEND_INDEX] ), ACTOROBJ_FILLIGHTGROUPEND_ID );


	////////////////////////////////////////////////////////////////////////////////////////
	//	Ambient light properties
	////////////////////////////////////////////////////////////////////////////////////////

	// start ambient group
	{
		jeVec3d	Color = { ACTOROBJ_DEFAULT_AMBIENTLIGHTRED, ACTOROBJ_DEFAULT_AMBIENTLIGHTGREEN, ACTOROBJ_DEFAULT_AMBIENTLIGHTBLUE };
		jeProperty_FillColorGroup(	&( ActorObjProperties[ACTOROBJ_AMBIENTLIGHTGROUP_INDEX] ),
								( IDS_AMBIENTLIGHTGROUP ),
								&Color,
								ACTOROBJ_AMBIENTLIGHTGROUP_ID );
	}

	// setup ambient light property
	{
		jeVec3d	Color = { ACTOROBJ_DEFAULT_AMBIENTLIGHTRED, ACTOROBJ_DEFAULT_AMBIENTLIGHTGREEN, ACTOROBJ_DEFAULT_AMBIENTLIGHTBLUE };
		jeProperty_FillColorPicker(	&( ActorObjProperties[ACTOROBJ_AMBIENTLIGHT_INDEX] ),
									( IDS_AMBIENTLIGHT ),
									&Color,
									ACTOROBJ_AMBIENTLIGHT_ID );
	}

	// setup ambient light red property
	jeProperty_FillFloat(	&( ActorObjProperties[ACTOROBJ_AMBIENTLIGHTRED_INDEX] ),
							( IDS_AMBIENTLIGHTRED ),
							ACTOROBJ_DEFAULT_AMBIENTLIGHTRED,
							ACTOROBJ_AMBIENTLIGHTRED_ID,
							0.0f, 255.0f, 1.0f );

	// setup ambient light green property
	jeProperty_FillFloat(	&( ActorObjProperties[ACTOROBJ_AMBIENTLIGHTGREEN_INDEX] ),
							( IDS_AMBIENTLIGHTGREEN ),
							ACTOROBJ_DEFAULT_AMBIENTLIGHTGREEN,
							ACTOROBJ_AMBIENTLIGHTGREEN_ID,
							0.0f, 255.0f, 1.0f );

	// setup ambient light blue property
	jeProperty_FillFloat(	&( ActorObjProperties[ACTOROBJ_AMBIENTLIGHTBLUE_INDEX] ),
							( IDS_AMBIENTLIGHTBLUE ),
							ACTOROBJ_DEFAULT_AMBIENTLIGHTBLUE,
							ACTOROBJ_AMBIENTLIGHTBLUE_ID,
							0.0f, 255.0f, 1.0f );

	// end ambient light group
	jeProperty_FillGroupEnd( &( ActorObjProperties[ACTOROBJ_AMBIENTLIGHTGROUPEND_INDEX] ), ACTOROBJ_AMBIENTLIGHTGROUPEND_ID );


	////////////////////////////////////////////////////////////////////////////////////////
	//	Collision box properties
	////////////////////////////////////////////////////////////////////////////////////////

	// start collision box group
	jeProperty_FillGroup( &( ActorObjProperties[ACTOROBJ_COLLISIONEXTBOXGROUP_INDEX] ),
						( IDS_COLLISIONEXTBOXGROUP ),
						ACTOROBJ_COLLISIONEXTBOXGROUP_ID );

	// setup collision box display property
	jeProperty_FillCheck(	&( ActorObjProperties[ACTOROBJ_COLLISIONEXTBOXDISPLAY_INDEX] ),
							( IDS_COLLISIONEXTBOXDISPLAY ),
							ACTOROBJ_DEFAULT_COLLISIONEXTBOXDISPLAY,
							ACTOROBJ_COLLISIONEXTBOXDISPLAY_ID );

	// setup collision box properties
	jeProperty_FillFloat(	&( ActorObjProperties[ACTOROBJ_COLLISIONEXTBOXMINX_INDEX] ),
							( IDS_COLLISIONEXTBOXMINX ),
							ACTOROBJ_DEFAULT_EXTBOXMINX,
							ACTOROBJ_COLLISIONEXTBOXMINX_ID,
							-FLT_MAX, FLT_MAX, 5.0f );
	jeProperty_FillFloat(	&( ActorObjProperties[ACTOROBJ_COLLISIONEXTBOXMINY_INDEX] ),
							( IDS_COLLISIONEXTBOXMINY ),
							ACTOROBJ_DEFAULT_EXTBOXMINY,
							ACTOROBJ_COLLISIONEXTBOXMINY_ID,
							-FLT_MAX, FLT_MAX, 5.0f );
	jeProperty_FillFloat(	&( ActorObjProperties[ACTOROBJ_COLLISIONEXTBOXMINZ_INDEX] ),
							( IDS_COLLISIONEXTBOXMINZ ),
							ACTOROBJ_DEFAULT_EXTBOXMINZ,
							ACTOROBJ_COLLISIONEXTBOXMINZ_ID,
							-FLT_MAX, FLT_MAX, 5.0f );
	jeProperty_FillFloat(	&( ActorObjProperties[ACTOROBJ_COLLISIONEXTBOXMAXX_INDEX] ),
							( IDS_COLLISIONEXTBOXMAXX ),
							ACTOROBJ_DEFAULT_EXTBOXMAXX,
							ACTOROBJ_COLLISIONEXTBOXMAXX_ID,
							-FLT_MAX, FLT_MAX, 5.0f );
	jeProperty_FillFloat(	&( ActorObjProperties[ACTOROBJ_COLLISIONEXTBOXMAXY_INDEX] ),
							( IDS_COLLISIONEXTBOXMAXY ),
							ACTOROBJ_DEFAULT_EXTBOXMAXY,
							ACTOROBJ_COLLISIONEXTBOXMAXY_ID,
							-FLT_MAX, FLT_MAX, 5.0f );
	jeProperty_FillFloat(	&( ActorObjProperties[ACTOROBJ_COLLISIONEXTBOXMAXZ_INDEX] ),
							( IDS_COLLISIONEXTBOXMAXZ ),
							ACTOROBJ_DEFAULT_EXTBOXMAXZ,
							ACTOROBJ_COLLISIONEXTBOXMAXZ_ID,
							-FLT_MAX, FLT_MAX, 5.0f );

	// end collision box group
	jeProperty_FillGroupEnd( &( ActorObjProperties[ACTOROBJ_COLLISIONEXTBOXGROUPEND_INDEX] ), ACTOROBJ_COLLISIONEXTBOXGROUPEND_ID );


	////////////////////////////////////////////////////////////////////////////////////////
	//	Render box properties
	////////////////////////////////////////////////////////////////////////////////////////

	// start render box group
	jeProperty_FillGroup( &( ActorObjProperties[ACTOROBJ_RENDEREXTBOXGROUP_INDEX] ),
						( IDS_RENDEREXTBOXGROUP ),
						ACTOROBJ_RENDEREXTBOXGROUP_ID );

	// setup render box display property
	jeProperty_FillCheck(	&( ActorObjProperties[ACTOROBJ_RENDEREXTBOXDISPLAY_INDEX] ),
							( IDS_RENDEREXTBOXDISPLAY ),
							ACTOROBJ_DEFAULT_RENDEREXTBOXDISPLAY,
							ACTOROBJ_RENDEREXTBOXDISPLAY_ID );

	// setup render box properties
	jeProperty_FillFloat(	&( ActorObjProperties[ACTOROBJ_RENDEREXTBOXMINX_INDEX] ),
							( IDS_RENDEREXTBOXMINX ),
							ACTOROBJ_DEFAULT_EXTBOXMINX,
							ACTOROBJ_RENDEREXTBOXMINX_ID,
							-FLT_MAX, FLT_MAX, 5.0f );
	jeProperty_FillFloat(	&( ActorObjProperties[ACTOROBJ_RENDEREXTBOXMINY_INDEX] ),
							( IDS_RENDEREXTBOXMINY ),
							ACTOROBJ_DEFAULT_EXTBOXMINY,
							ACTOROBJ_RENDEREXTBOXMINY_ID,
							-FLT_MAX, FLT_MAX, 5.0f );
	jeProperty_FillFloat(	&( ActorObjProperties[ACTOROBJ_RENDEREXTBOXMINZ_INDEX] ),
							( IDS_RENDEREXTBOXMINZ ),
							ACTOROBJ_DEFAULT_EXTBOXMINZ,
							ACTOROBJ_RENDEREXTBOXMINZ_ID,
							-FLT_MAX, FLT_MAX, 5.0f );
	jeProperty_FillFloat(	&( ActorObjProperties[ACTOROBJ_RENDEREXTBOXMAXX_INDEX] ),
							( IDS_RENDEREXTBOXMAXX ),
							ACTOROBJ_DEFAULT_EXTBOXMAXX,
							ACTOROBJ_RENDEREXTBOXMAXX_ID,
							-FLT_MAX, FLT_MAX, 5.0f );
	jeProperty_FillFloat(	&( ActorObjProperties[ACTOROBJ_RENDEREXTBOXMAXY_INDEX] ),
							( IDS_RENDEREXTBOXMAXY ),
							ACTOROBJ_DEFAULT_EXTBOXMAXY,
							ACTOROBJ_RENDEREXTBOXMAXY_ID,
							-FLT_MAX, FLT_MAX, 5.0f );
	jeProperty_FillFloat(	&( ActorObjProperties[ACTOROBJ_RENDEREXTBOXMAXZ_INDEX] ),
							( IDS_RENDEREXTBOXMAXZ ),
							ACTOROBJ_DEFAULT_EXTBOXMAXZ,
							ACTOROBJ_RENDEREXTBOXMAXZ_ID,
							-FLT_MAX, FLT_MAX, 5.0f );

	// end render box group
	jeProperty_FillGroupEnd( &( ActorObjProperties[ACTOROBJ_RENDEREXTBOXGROUPEND_INDEX] ), ACTOROBJ_RENDEREXTBOXGROUPEND_ID );


	////////////////////////////////////////////////////////////////////////////////////////
	//	Material box properties
	////////////////////////////////////////////////////////////////////////////////////////

	// start material group
	jeProperty_FillGroup(	&( ActorObjProperties[ACTOROBJ_MATERIALGROUP_INDEX] ),
							( IDS_MATERIALGROUP ),
							ACTOROBJ_MATERIALGROUP_ID );

	// setup material color
	/*jeProperty_FillFloat(	&( ActorObjProperties[ACTOROBJ_MATERIALRED_INDEX] ),
							( IDS_MATERIALRED ),
							255.0f,
							ACTOROBJ_MATERIALRED_ID,
							0.0f, 255.0f, 1.0f );
	jeProperty_FillFloat(	&( ActorObjProperties[ACTOROBJ_MATERIALGREEN_INDEX] ),
							( IDS_MATERIALGREEN ),
							255.0f,
							ACTOROBJ_MATERIALGREEN_ID,
							0.0f, 255.0f, 1.0f );
	jeProperty_FillFloat(	&( ActorObjProperties[ACTOROBJ_MATERIALBLUE_INDEX] ),
							( IDS_MATERIALBLUE ),
							255.0f,
							ACTOROBJ_MATERIALBLUE_ID,
							0.0f, 255.0f, 1.0f );*/

	// end material group
	jeProperty_FillGroupEnd( &( ActorObjProperties[ACTOROBJ_MATERIALGROUPEND_INDEX] ), ACTOROBJ_MATERIALGROUPEND_ID );


	////////////////////////////////////////////////////////////////////////////////////////
	//	Misc properties
	////////////////////////////////////////////////////////////////////////////////////////

	// setup use ambient light from floor property
	jeProperty_FillCheck(	&( ActorObjProperties[ACTOROBJ_USEAMBIENTLIGHTFROMFLOOR_INDEX] ),
							( IDS_USEAMBIENTLIGHTFROMFLOOR ),
							ACTOROBJ_DEFAULT_USEAMBFROMFLOOR,
							ACTOROBJ_USEAMBIENTLIGHTFROMFLOOR_ID );
	jeProperty_SetDisabled( &( ActorObjProperties[ACTOROBJ_USEAMBIENTLIGHTFROMFLOOR_INDEX] ), JE_TRUE );


	// setup maximum dynamic lights to use property
	jeProperty_FillInt(	&( ActorObjProperties[ACTOROBJ_MAXIMUMDYNAMICLIGHTSTOUSE_INDEX] ),
						( IDS_MAXIMUMDYNAMICLIGHTSTOUSE ),
						ACTOROBJ_DEFAULT_MAXDYNAMICLIGHTS,
						ACTOROBJ_MAXIMUMDYNAMICLIGHTSTOUSE_ID,
						1.0f, 32.0f, 1.0f );

	// setup maximum static lights to use property
	jeProperty_FillInt(	&( ActorObjProperties[ACTOROBJ_MAXIMUMSTATICLIGHTSTOUSE_INDEX] ),
						( IDS_MAXIMUMSTATICLIGHTSTOUSE ),
						ACTOROBJ_DEFAULT_MAXSTATICLIGHTS,
						ACTOROBJ_MAXIMUMSTATICLIGHTSTOUSE_ID,
						1.0f, 32.0f, 1.0f );

	// setup per bone lighting property
	jeProperty_FillCheck(	&( ActorObjProperties[ACTOROBJ_PERBONELIGHTING_INDEX] ),
							( IDS_PERBONELIGHTING ),
							ACTOROBJ_DEFAULT_PERBONELIGHTING,
							ACTOROBJ_PERBONELIGHTING_ID );

	// setup motion time scale property
	jeProperty_FillFloat(	&( ActorObjProperties[ACTOROBJ_MOTIONTIMESCALE_INDEX] ),
							( IDS_MOTIONTIMESCALE ),
							ACTOROBJ_DEFAULT_MOTIONTIMESCALE,
							ACTOROBJ_MOTIONTIMESCALE_ID,
							-FLT_MAX, FLT_MAX, 0.1f );


	// final init
	ActorObjPropertyList.jePropertyN = ACTOROBJ_LAST_INDEX;
}

////////////////////////////////////////////////////////////////////////////////////////
//
//	GetPropertyList()
//
///////////////////////////////////////////////////////////////////////////////////////
jeBoolean JETCC GetPropertyList(
	void* Instance,	// object instance data
	jeProperty_List	**List)		// where to save property list pointer
{

	// locals	
	jeActor* Actor;
	ActorObj *Object;

	// ensure valid data
	assert( Instance != NULL );
	assert( List != NULL );

	// get object data
	Actor = (jeActor*)Instance;
	Object = Actor->Object;

	// setup property list
	//ActorObjProperties[ACTOROBJ_SCALEGROUP_INDEX].Data.Vector.X = Object->ScaleX;
	//ActorObjProperties[ACTOROBJ_SCALEGROUP_INDEX].Data.Vector.Y = Object->ScaleY;
	//ActorObjProperties[ACTOROBJ_SCALEGROUP_INDEX].Data.Vector.Z = Object->ScaleZ;
	ActorObjProperties[ACTOROBJ_SCALEX_INDEX].Data.Float = Object->ScaleX;
	//ActorObjProperties[ACTOROBJ_SCALEY_INDEX].Data.Float = Object->ScaleY;
	//ActorObjProperties[ACTOROBJ_SCALEZ_INDEX].Data.Float = Object->ScaleZ;
	ActorObjProperties[ACTOROBJ_PERBONELIGHTING_INDEX].Data.Bool = Object->PerBoneLighting;
	ActorObjProperties[ACTOROBJ_MOTIONTIMESCALE_INDEX].Data.Float = Object->MotionTimeScale;
	ActorObjProperties[ACTOROBJ_MAXIMUMDYNAMICLIGHTSTOUSE_INDEX].Data.Int = Object->MaximumDynamicLightsToUse;
	ActorObjProperties[ACTOROBJ_MAXIMUMSTATICLIGHTSTOUSE_INDEX].Data.Int = Object->MaximumStaticLightsToUse;
	ActorObjProperties[ACTOROBJ_FILLLIGHTUSE_INDEX].Data.Bool = Object->UseFillLight;
	ActorObjProperties[ACTOROBJ_FILLLIGHTNORMALACTORRELATIVE_INDEX].Data.Bool = Object->FillNormalActorRelative;
	ActorObjProperties[ACTOROBJ_USEAMBIENTLIGHTFROMFLOOR_INDEX].Data.Bool = Object->UseAmbientLightFromFloor;
	ActorObjProperties[ACTOROBJ_FILLIGHTGROUP_INDEX].Data.Vector.X = Object->FillLightRed;
	ActorObjProperties[ACTOROBJ_FILLIGHTGROUP_INDEX].Data.Vector.Y = Object->FillLightGreen;
	ActorObjProperties[ACTOROBJ_FILLIGHTGROUP_INDEX].Data.Vector.Z = Object->FillLightBlue;
	ActorObjProperties[ACTOROBJ_FILLLIGHT_INDEX].Data.Vector.X = Object->FillLightRed;
	ActorObjProperties[ACTOROBJ_FILLLIGHT_INDEX].Data.Vector.Y = Object->FillLightGreen;
	ActorObjProperties[ACTOROBJ_FILLLIGHT_INDEX].Data.Vector.Z = Object->FillLightBlue;
	ActorObjProperties[ACTOROBJ_FILLLIGHTRED_INDEX].Data.Float = Object->FillLightRed;
	ActorObjProperties[ACTOROBJ_FILLLIGHTGREEN_INDEX].Data.Float = Object->FillLightGreen;
	ActorObjProperties[ACTOROBJ_FILLLIGHTBLUE_INDEX].Data.Float = Object->FillLightBlue;
	ActorObjProperties[ACTOROBJ_AMBIENTLIGHT_INDEX].Data.Vector.X = Object->AmbientLightRed;
	ActorObjProperties[ACTOROBJ_AMBIENTLIGHT_INDEX].Data.Vector.Y = Object->AmbientLightGreen;
	ActorObjProperties[ACTOROBJ_AMBIENTLIGHT_INDEX].Data.Vector.Z = Object->AmbientLightBlue;
	ActorObjProperties[ACTOROBJ_AMBIENTLIGHTGROUP_INDEX].Data.Vector.X = Object->AmbientLightRed;
	ActorObjProperties[ACTOROBJ_AMBIENTLIGHTGROUP_INDEX].Data.Vector.Y = Object->AmbientLightGreen;
	ActorObjProperties[ACTOROBJ_AMBIENTLIGHTGROUP_INDEX].Data.Vector.Z = Object->AmbientLightBlue;
	ActorObjProperties[ACTOROBJ_AMBIENTLIGHTRED_INDEX].Data.Float = Object->AmbientLightRed;
	ActorObjProperties[ACTOROBJ_AMBIENTLIGHTGREEN_INDEX].Data.Float = Object->AmbientLightGreen;
	ActorObjProperties[ACTOROBJ_AMBIENTLIGHTBLUE_INDEX].Data.Float = Object->AmbientLightBlue;
	ActorObjProperties[ACTOROBJ_FILLLIGHTNORMAL_INDEX].Data.Vector.X = Object->FillLightNormal.X;
	ActorObjProperties[ACTOROBJ_FILLLIGHTNORMAL_INDEX].Data.Vector.Y = Object->FillLightNormal.Y;
	ActorObjProperties[ACTOROBJ_FILLLIGHTNORMAL_INDEX].Data.Vector.Z = Object->FillLightNormal.Z;
	ActorObjProperties[ACTOROBJ_FILLLIGHTNORMALX_INDEX].Data.Float = Object->FillLightNormal.X;
	ActorObjProperties[ACTOROBJ_FILLLIGHTNORMALY_INDEX].Data.Float = Object->FillLightNormal.Y;
	ActorObjProperties[ACTOROBJ_FILLLIGHTNORMALZ_INDEX].Data.Float = Object->FillLightNormal.Z;

	// setup collision ext box def list
	ActorObjProperties[ACTOROBJ_COLLISIONEXTBOXDISPLAY_INDEX].Data.Bool = Object->CollisionExtBoxDisplay;
	ActorObjProperties[ACTOROBJ_COLLISIONEXTBOXMINX_INDEX].Data.Float = Object->CollisionExtBox.Min.X;
	ActorObjProperties[ACTOROBJ_COLLISIONEXTBOXMINY_INDEX].Data.Float = Object->CollisionExtBox.Min.Y;
	ActorObjProperties[ACTOROBJ_COLLISIONEXTBOXMINZ_INDEX].Data.Float = Object->CollisionExtBox.Min.Z;
	ActorObjProperties[ACTOROBJ_COLLISIONEXTBOXMAXX_INDEX].Data.Float = Object->CollisionExtBox.Max.X;
	ActorObjProperties[ACTOROBJ_COLLISIONEXTBOXMAXY_INDEX].Data.Float = Object->CollisionExtBox.Max.Y;
	ActorObjProperties[ACTOROBJ_COLLISIONEXTBOXMAXZ_INDEX].Data.Float = Object->CollisionExtBox.Max.Z;

	// setup render ext box def list
	ActorObjProperties[ACTOROBJ_RENDEREXTBOXDISPLAY_INDEX].Data.Bool = Object->RenderExtBoxDisplay;
	ActorObjProperties[ACTOROBJ_RENDEREXTBOXMINX_INDEX].Data.Float = Object->RenderHintExtBox.Min.X;
	ActorObjProperties[ACTOROBJ_RENDEREXTBOXMINY_INDEX].Data.Float = Object->RenderHintExtBox.Min.Y;
	ActorObjProperties[ACTOROBJ_RENDEREXTBOXMINZ_INDEX].Data.Float = Object->RenderHintExtBox.Min.Z;
	ActorObjProperties[ACTOROBJ_RENDEREXTBOXMAXX_INDEX].Data.Float = Object->RenderHintExtBox.Max.X;
	ActorObjProperties[ACTOROBJ_RENDEREXTBOXMAXY_INDEX].Data.Float = Object->RenderHintExtBox.Max.Y;
	ActorObjProperties[ACTOROBJ_RENDEREXTBOXMAXZ_INDEX].Data.Float = Object->RenderHintExtBox.Max.Z;

	// setup actor def list
	assert( ActorDefList != NULL );
	assert( ActorDefListSize > 0 );
	jeProperty_FillCombo(	&( ActorObjPropertyList.pjeProperty[ACTOROBJ_LIST_INDEX] ),
							IDS_ACTORLIST,
							Object->ActorDefName,
							ACTOROBJ_LIST_ID,
							ActorDefListSize,
							ActorDefList );

	// setup motion list
	jeProperty_FillCombo(	&( ActorObjPropertyList.pjeProperty[ACTOROBJ_MOTIONLIST_INDEX] ),
							IDS_MOTIONLIST,
							Object->MotionName,
							ACTOROBJ_MOTIONLIST_ID,
							Object->MotionListSize,
							Object->MotionList );

	// setup bone list
	jeProperty_FillCombo(	&( ActorObjPropertyList.pjeProperty[ACTOROBJ_LIGHTREFERENCEBONENAMELIST_INDEX] ),
							IDS_BONELIST,
							Object->LightReferenceBoneName,
							ACTOROBJ_LIGHTREFERENCEBONENAMELIST_ID,
							Object->BoneListSize,
							Object->BoneList );

	// setup material list
	jeProperty_FillCombo(	&( ActorObjPropertyList.pjeProperty[ACTOROBJ_MATERIALLIST_INDEX] ),
							IDS_MATERIALLIST,
							Object->MaterialList[Object->MaterialCurrent],
							ACTOROBJ_MATERIALLIST_ID,	
							Object->MaterialListSize,
							Object->MaterialList );

	// setup material overide list
	jeProperty_FillCombo(	&( ActorObjPropertyList.pjeProperty[ACTOROBJ_MATERIALOVERIDE_INDEX] ),
							IDS_MATERIALOVERIDE,
							Object->MaterialOverideList[Object->MaterialCurrent],
							ACTOROBJ_MATERIALOVERIDE_ID,	
							Bitmaps->Total,
							Bitmaps->Name );

	// setup material mapper list
	jeProperty_FillCombo(	&( ActorObjPropertyList.pjeProperty[ACTOROBJ_MATERIALMAPPER_INDEX] ),
							IDS_MATERIALMAPPER,
							Object->MaterialMapperList[Object->MaterialCurrent],
							ACTOROBJ_MATERIALMAPPER_ID,
							MaterialMapperTableSize,
							MaterialMapperNameList );

	// copy property list
	*List = jeProperty_ListCopy( &ActorObjPropertyList );
	if ( *List == NULL )
	{
		jeErrorLog_Add( JE_ERR_SUBSYSTEM_FAILURE, NULL );
		return JE_FALSE;
	}

	// reset dirty flag
	ActorObjPropertyList.bDirty = JE_FALSE;

	// all done
	return JE_TRUE;

} // GetPropertyList()

////////////////////////////////////////////////////////////////////////////////////////
//
//	SetProperty()
//
///////////////////////////////////////////////////////////////////////////////////////
jeBoolean JETCC SetProperty(
	void *Instance,	// object instance data
	int32				FieldID,	// id of field to be changed
	PROPERTY_FIELD_TYPE	DataType,	// type of data
	jeProperty_Data		*pData )	// new data
{

	// locals	
	jeActor* Actor;
	ActorObj* Object;
	jeBoolean	AdjustActorProperties = JE_FALSE;

	// ensure valid data
	assert( Instance != NULL );
	assert( pData != NULL );

	// get object data			
	Actor = (jeActor*)Instance;
	Object = Actor->Object;
	if(Object == NULL) return JE_FALSE;

	// process field id
	switch ( FieldID )
	{

		// adjust scale
		case ACTOROBJ_SCALEX_ID:
		{
			assert( DataType == PROPERTY_FLOAT_TYPE );
			Object->ScaleX = pData->Float;
			Object->ScaleY = pData->Float;
			Object->ScaleZ = pData->Float;

			AdjustActorProperties = JE_TRUE;
			break;
		}
		// adjust collision extent box size
		case ACTOROBJ_COLLISIONEXTBOXMINX_ID:
		{
			float	OldVal;
			assert( DataType == PROPERTY_FLOAT_TYPE );
			OldVal = Object->CollisionExtBox.Min.X;
			Object->CollisionExtBox.Min.X = pData->Float;
			if ( jeExtBox_IsValid( &( Object->CollisionExtBox ) ) == JE_FALSE )
			{
				Object->CollisionExtBox.Min.X = OldVal;
			}
			break;
		}
		case ACTOROBJ_COLLISIONEXTBOXMINY_ID:
		{
			float	OldVal;
			assert( DataType == PROPERTY_FLOAT_TYPE );
			OldVal = Object->CollisionExtBox.Min.Y;
			Object->CollisionExtBox.Min.Y = pData->Float;
			if ( jeExtBox_IsValid( &( Object->CollisionExtBox ) ) == JE_FALSE )
			{
				Object->CollisionExtBox.Min.Y = OldVal;
			}
			break;
		}
		case ACTOROBJ_COLLISIONEXTBOXMINZ_ID:
		{
			float	OldVal;
			assert( DataType == PROPERTY_FLOAT_TYPE );
			OldVal = Object->CollisionExtBox.Min.Z;
			Object->CollisionExtBox.Min.Z = pData->Float;
			if ( jeExtBox_IsValid( &( Object->CollisionExtBox ) ) == JE_FALSE )
			{
				Object->CollisionExtBox.Min.Z = OldVal;
			}
			break;
		}
		case ACTOROBJ_COLLISIONEXTBOXMAXX_ID:
		{
			float	OldVal;
			assert( DataType == PROPERTY_FLOAT_TYPE );
			OldVal = Object->CollisionExtBox.Max.X;
			Object->CollisionExtBox.Max.X = pData->Float;
			if ( jeExtBox_IsValid( &( Object->CollisionExtBox ) ) == JE_FALSE )
			{
				Object->CollisionExtBox.Max.X = OldVal;
			}
			break;
		}
		case ACTOROBJ_COLLISIONEXTBOXMAXY_ID:
		{
			float	OldVal;
			assert( DataType == PROPERTY_FLOAT_TYPE );
			OldVal = Object->CollisionExtBox.Max.Y;
			Object->CollisionExtBox.Max.Y = pData->Float;
			if ( jeExtBox_IsValid( &( Object->CollisionExtBox ) ) == JE_FALSE )
			{
				Object->CollisionExtBox.Max.Y = OldVal;
			}
			break;
		}
		case ACTOROBJ_COLLISIONEXTBOXMAXZ_ID:
		{
			float	OldVal;
			assert( DataType == PROPERTY_FLOAT_TYPE );
			OldVal = Object->CollisionExtBox.Max.Z;
			Object->CollisionExtBox.Max.Z = pData->Float;
			if ( jeExtBox_IsValid( &( Object->CollisionExtBox ) ) == JE_FALSE )
			{
				Object->CollisionExtBox.Max.Z = OldVal;
			}
			break;
		}

		// adjust render extent box size
		case ACTOROBJ_RENDEREXTBOXMINX_ID:
		{
			float	OldVal;
			assert( DataType == PROPERTY_FLOAT_TYPE );
			OldVal = Object->RenderHintExtBox.Min.X;
			Object->RenderHintExtBox.Min.X = pData->Float;
			if ( jeExtBox_IsValid( &( Object->RenderHintExtBox ) ) == JE_FALSE )
			{
				Object->RenderHintExtBox.Min.X = OldVal;
			}
			break;
		}
		case ACTOROBJ_RENDEREXTBOXMINY_ID:
		{
			float	OldVal;
			assert( DataType == PROPERTY_FLOAT_TYPE );
			OldVal = Object->RenderHintExtBox.Min.Y;
			Object->RenderHintExtBox.Min.Y = pData->Float;
			if ( jeExtBox_IsValid( &( Object->RenderHintExtBox ) ) == JE_FALSE )
			{
				Object->RenderHintExtBox.Min.Y = OldVal;
			}
			break;
		}
		case ACTOROBJ_RENDEREXTBOXMINZ_ID:
		{
			float	OldVal;
			assert( DataType == PROPERTY_FLOAT_TYPE );
			OldVal = Object->RenderHintExtBox.Min.Z;
			Object->RenderHintExtBox.Min.Z = pData->Float;
			if ( jeExtBox_IsValid( &( Object->RenderHintExtBox ) ) == JE_FALSE )
			{
				Object->RenderHintExtBox.Min.Z = OldVal;
			}
			break;
		}
		case ACTOROBJ_RENDEREXTBOXMAXX_ID:
		{
			float	OldVal;
			assert( DataType == PROPERTY_FLOAT_TYPE );
			OldVal = Object->RenderHintExtBox.Max.X;
			Object->RenderHintExtBox.Max.X = pData->Float;
			if ( jeExtBox_IsValid( &( Object->RenderHintExtBox ) ) == JE_FALSE )
			{
				Object->RenderHintExtBox.Max.X = OldVal;
			}
			break;
		}
		case ACTOROBJ_RENDEREXTBOXMAXY_ID:
		{
			float	OldVal;
			assert( DataType == PROPERTY_FLOAT_TYPE );
			OldVal = Object->RenderHintExtBox.Max.Y;
			Object->RenderHintExtBox.Max.Y = pData->Float;
			if ( jeExtBox_IsValid( &( Object->RenderHintExtBox ) ) == JE_FALSE )
			{
				Object->RenderHintExtBox.Max.Y = OldVal;
			}
			break;
		}
		case ACTOROBJ_RENDEREXTBOXMAXZ_ID:
		{
			float	OldVal;
			assert( DataType == PROPERTY_FLOAT_TYPE );
			OldVal = Object->RenderHintExtBox.Max.Z;
			Object->RenderHintExtBox.Max.Z = pData->Float;
			if ( jeExtBox_IsValid( &( Object->RenderHintExtBox ) ) == JE_FALSE )
			{
				Object->RenderHintExtBox.Max.Z = OldVal;
			}
			break;
		}

		// adjust perbone lighting
		case ACTOROBJ_PERBONELIGHTING_ID:
		{

			// save new perbone lightint setting
			assert( DataType == PROPERTY_CHECK_TYPE );
			Object->PerBoneLighting = pData->Bool;
			AdjustActorProperties = JE_TRUE;

			// if per bone lighting is turned on then reset bone lighting combo box
			if ( pData->Bool == JE_TRUE )
			{

				// free old bone name
				assert( Object->LightReferenceBoneName != NULL );
				jeRam_Free( Object->LightReferenceBoneName );

				// save new bone name
				Object->LightReferenceBoneName = Util_StrDup( Object->BoneList[0] );

				// set combo box choice
				jeProperty_FillCombo(	&( ActorObjPropertyList.pjeProperty[ACTOROBJ_LIGHTREFERENCEBONENAMELIST_INDEX] ),
										IDS_BONELIST,
										Object->LightReferenceBoneName,
										ACTOROBJ_LIGHTREFERENCEBONENAMELIST_ID,
										Object->BoneListSize,
										Object->BoneList );
			}
			break;
		}

		// adjust motion time scale
		case ACTOROBJ_MOTIONTIMESCALE_ID:
		{
			assert( DataType == PROPERTY_FLOAT_TYPE );
			Object->MotionTimeScale = pData->Float;
			break;
		}

		// adjust use of fill light
		case ACTOROBJ_FILLLIGHTUSE_ID:
		{
			assert( DataType == PROPERTY_CHECK_TYPE );
			Object->UseFillLight = pData->Bool;
			AdjustActorProperties = JE_TRUE;
			break;
		}

		// adjust fill light normal actor relative
		case ACTOROBJ_FILLLIGHTNORMALACTORRELATIVE_ID:
		{
			assert( DataType == PROPERTY_CHECK_TYPE );
			Object->FillNormalActorRelative = pData->Bool;
			AdjustActorProperties = JE_TRUE;
			break;
		}

		// adjust fill light normal
		case ACTOROBJ_FILLLIGHTNORMALX_ID:
		{
			assert( DataType == PROPERTY_FLOAT_TYPE );
			Object->FillLightNormal.X = pData->Float;
			AdjustActorProperties = JE_TRUE;
			break;
		}
		case ACTOROBJ_FILLLIGHTNORMALY_ID:
		{
			assert( DataType == PROPERTY_FLOAT_TYPE );
			Object->FillLightNormal.Y = pData->Float;
			AdjustActorProperties = JE_TRUE;
			break;
		}
		case ACTOROBJ_FILLLIGHTNORMALZ_ID:
		{
			assert( DataType == PROPERTY_FLOAT_TYPE );
			Object->FillLightNormal.Z = pData->Float;
			AdjustActorProperties = JE_TRUE;
			break;
		}

		// adjust fill light color
		case ACTOROBJ_FILLLIGHT_ID:
		{
			assert( DataType == PROPERTY_COLOR_PICKER_TYPE );
			Object->FillLightRed = pData->Vector.X;
			Object->FillLightGreen = pData->Vector.Y;
			Object->FillLightBlue = pData->Vector.Z;
			AdjustActorProperties = JE_TRUE;
			break;
		}
		case ACTOROBJ_FILLLIGHTRED_ID:
		{
			assert( DataType == PROPERTY_FLOAT_TYPE );
			Object->FillLightRed = pData->Float;
			AdjustActorProperties = JE_TRUE;
			break;
		}
		case ACTOROBJ_FILLLIGHTGREEN_ID:
		{
			assert( DataType == PROPERTY_FLOAT_TYPE );
			Object->FillLightGreen = pData->Float;
			AdjustActorProperties = JE_TRUE;
			break;
		}
		case ACTOROBJ_FILLLIGHTBLUE_ID:
		{
			assert( DataType == PROPERTY_FLOAT_TYPE );
			Object->FillLightBlue = pData->Float;
			AdjustActorProperties = JE_TRUE;
			break;
		}

		// adjust ambient light color
		case ACTOROBJ_AMBIENTLIGHT_ID:
		{
			assert( DataType == PROPERTY_COLOR_PICKER_TYPE );
			Object->AmbientLightRed = pData->Vector.X;
			Object->AmbientLightGreen = pData->Vector.Y;
			Object->AmbientLightBlue = pData->Vector.Z;
			AdjustActorProperties = JE_TRUE;
			break;
		}
		case ACTOROBJ_AMBIENTLIGHTRED_ID:
		{
			assert( DataType == PROPERTY_FLOAT_TYPE );
			Object->AmbientLightRed = pData->Float;
			AdjustActorProperties = JE_TRUE;
			break;
		}
		case ACTOROBJ_AMBIENTLIGHTGREEN_ID:
		{
			assert( DataType == PROPERTY_FLOAT_TYPE );
			Object->AmbientLightGreen = pData->Float;
			AdjustActorProperties = JE_TRUE;
			break;
		}
		case ACTOROBJ_AMBIENTLIGHTBLUE_ID:
		{
			assert( DataType == PROPERTY_FLOAT_TYPE );
			Object->AmbientLightBlue = pData->Float;
			AdjustActorProperties = JE_TRUE;
			break;
		}

		// adjust whether or not to use fill light from floor
		case ACTOROBJ_USEAMBIENTLIGHTFROMFLOOR_ID:
		{
			assert( DataType == PROPERTY_CHECK_TYPE );
			Object->UseAmbientLightFromFloor = pData->Bool;
			AdjustActorProperties = JE_TRUE;
			break;
		}

		// adjust whether or not to display collision extent box
		case ACTOROBJ_COLLISIONEXTBOXDISPLAY_ID:
		{
			assert( DataType == PROPERTY_CHECK_TYPE );
			Object->CollisionExtBoxDisplay = pData->Bool;
			break;
		}

		// adjust whether or not to display draw extent box
		case ACTOROBJ_RENDEREXTBOXDISPLAY_ID:
		{
			assert( DataType == PROPERTY_CHECK_TYPE );
			Object->RenderExtBoxDisplay = pData->Bool;
			break;
		}

		// adjust maximum dynamic lights to use
		case ACTOROBJ_MAXIMUMDYNAMICLIGHTSTOUSE_ID:
		{
			assert( DataType == PROPERTY_INT_TYPE );
			Object->MaximumDynamicLightsToUse = pData->Int;
			AdjustActorProperties = JE_TRUE;
			break;
		}

		// adjust maximum dynamic lights to use
		case ACTOROBJ_MAXIMUMSTATICLIGHTSTOUSE_ID:
		{
			assert( DataType == PROPERTY_INT_TYPE );
			Object->MaximumStaticLightsToUse = pData->Int;
			AdjustActorProperties = JE_TRUE;
			break;
		}


		// adjust actor
		case ACTOROBJ_LIST_ID:
		{
			jeActor_Def* ActorDefinition = NULL;
			jeVec3d	Pos;
			jeXForm3d Xf;
			//ActorObj *pObj = NULL;

			// ensure valid data
			assert( DataType == PROPERTY_COMBO_TYPE );
			assert( pData->String != NULL );

			// reset motion list
			ActorObj_ResetMotionList( Actor );

			// reset bone list
			ActorObj_ResetBoneList( Actor );

			// reset material list
			ActorObj_ResetMaterialList( Actor );

			// save new actor def name
			if ( Object->ActorDefName != NULL )
			{
				jeRam_Free( Object->ActorDefName );
			}

			// Save old parameters
			//jeVec3d_Copy( &( Actor->Xf.Translation ), &Pos );
			//pObj = Actor->Object;
			Xf = Actor->Xf;
			Pos = Xf.Translation;
			//jeXForm3d_SetIdentity(&Xf);
			jeXForm3d_RotateX( &Xf, -JE_HALFPI );
			Xf.Translation = Pos;

			// destroy current actor			
			if(Actor->ActorDefinition != NULL)
				jeActor_DetachEngine( Actor, Object->Engine );

			Actor->CanFree = JE_FALSE;
			Actor->Object = NULL;							
			jeActor_Destroy( &( Actor ) );

			// Restore parameters
			Actor->CanFree = JE_TRUE;
			//Actor->Object = pObj;
			Actor->Object = Object;
			//Actor->Xf = Xf;			
			//jeXForm3d_SetIdentity( &( Actor->Xf ) );
			//jeVec3d_Copy( &Pos, &( Actor->Xf.Translation ) );

			Object->ActorDefName = Util_StrDup( pData->String );

			// do nothing more if no selection is made
			if ( stricmp( pData->String, NoSelection ) == 0 )
			{
				break;
			}

			// get new actor def			
			ActorDefinition = (jeActor_Def *)jeResource_Get( Object->ResourceMgr, Object->ActorDefName );

			// if it doesn't exist then create it
			if ( ActorDefinition == NULL )
			{
				// locals
				jeVFile	*FileDir = NULL;
				jeVFile	*ActorDefFile;

				// get vfile dir
				FileDir = jeResource_GetVFile( Object->ResourceMgr, "Actors" );
				if ( FileDir == NULL )
				{
					jeErrorLog_Add( JE_ERR_SUBSYSTEM_FAILURE, NULL );
					return JE_FALSE;
				}

				// open actor def file
				ActorDefFile = jeVFile_Open( FileDir, Object->ActorDefName, JE_VFILE_OPEN_READONLY );
				if ( ActorDefFile == NULL )
				{
					jeErrorLog_AddString( JE_ERR_FILEIO_OPEN, "SetProperty(): jeVFile_Open() failed.", Object->ActorDefName);
					return JE_FALSE;
				}

				// create actor def
				ActorDefinition = jeActor_DefCreateFromFile( ActorDefFile );
				jeVFile_Close( ActorDefFile );

				// close vfile dir
				if ( jeResource_DeleteVFile( Object->ResourceMgr, "Actors" ) == 0 )
				{
					jeErrorLog_Add( JE_ERR_SUBSYSTEM_FAILURE, NULL );
					jeVFile_Close( FileDir );
				}

				// fail if actor def wasnt created
				if ( ActorDefinition == NULL )
				{
					jeErrorLog_Add( JE_ERR_SUBSYSTEM_FAILURE, NULL );
					return JE_FALSE;
				}

				// add actor def to the resource manager
				assert( Object->ActorDefName != NULL );
				jeResource_Add( Object->ResourceMgr, Object->ActorDefName, JE_RESOURCE_ACTOR, ActorDefinition );
			}

			// create actor
			assert( ActorDefinition != NULL );
			jeActor_SetActorDef(Actor,ActorDefinition);

			// add actor to the engine
			jeActor_AttachEngine( Actor, Object->Engine );

			//FillProperties();			
			//InitObjectProperties(Object->);

			// set initial pose
            if (!Object->LoadedFromDisk)
			{
				// locals
				jeVec3d		tmpPos;

				//SetupGlobalBrush( Object );
			    tmpPos = Xf.Translation;
			    jeXForm3d_SetIdentity(&Xf);
			    jeXForm3d_RotateX( &Xf, -JE_HALFPI );
			    Xf.Translation = tmpPos;

				// save original position and apply hack rotation
				jeVec3d_Copy( &( Actor->Xf.Translation ), &tmpPos );
				jeVec3d_Set( &( Actor->Xf.Translation ), 0.0f, 0.0f, 0.0f );
				jeXForm3d_RotateX( &( Actor->Xf ), -JE_HALFPI );
				jeActor_ClearPose( Actor, &( Actor->Xf ) );

				// setup collision ext box
				jeActor_GetDynamicExtBox( Actor, &( Object->CollisionExtBox ) );
				jeActor_SetExtBox( Actor, &( Object->CollisionExtBox ), NULL );

				// setup draw box
				jeActor_GetDynamicExtBox( Actor, &( Object->RenderHintExtBox ) );
				jeActor_SetRenderHintExtBox( Actor, &( Object->RenderHintExtBox ), NULL );

				// restore original position				
				//jeVec3d_Copy( &Pos, &( Actor->Xf.Translation ) );				
				jeVec3d_Copy( &tmpPos, &( Xf.Translation ) );				
				jeActor_SetXForm(Actor,&Xf);
				jeActor_ClearPose( Actor, &( Actor->Xf ) );
            } else {
                // reset the position to compute the Collision Bounding and Draw Box
				jeVec3d_Set( &( Actor->Xf.Translation ), 0.0f, 0.0f, 0.0f );
			    jeXForm3d_RotateX( &Actor->Xf, -JE_HALFPI );
				jeActor_ClearPose( Actor, &( Actor->Xf ) );

                // setup collision ext box
				jeActor_GetDynamicExtBox( Actor, &( Object->CollisionExtBox ) );
				jeActor_SetExtBox( Actor, &( Object->CollisionExtBox ), NULL );

				// setup draw box
				jeActor_GetDynamicExtBox( Actor, &( Object->RenderHintExtBox ) );
				jeActor_SetRenderHintExtBox( Actor, &( Object->RenderHintExtBox ), NULL );

                // Reset save XForm to the actor
				jeActor_SetXForm(Actor,&Xf);
				jeActor_ClearPose( Actor, &( Actor->Xf ) );
            }

			// create new motion list
			if ( Object->MotionListSize > 0 )
			{
				ActorObj_DestroyMotionList( Actor );
			}
			ActorObj_CreateMotionList( Actor );

			// create new bone list
			if ( Object->BoneListSize > 0 )
			{
				ActorObj_DestroyBoneList( Actor );
			}
			ActorObj_CreateBoneList( Actor );

			// create new material list
			ActorObj_DestroyMaterialList( Actor );
			ActorObj_CreateMaterialList( Actor );

			// set adjust flag
			AdjustActorProperties = JE_TRUE;

			// make sure the property list gets rebuilt on screen			
			ActorObjPropertyList.bDirty = JE_TRUE;
			break;
		}

		// adjust motion
		case ACTOROBJ_MOTIONLIST_ID:
		{

			// ensure valid data
			assert( DataType == PROPERTY_COMBO_TYPE );
			assert( pData->String != NULL );
			assert( Object->MotionListSize > 0 );

			// save new motion name
			if ( Object->MotionName != NULL )
			{
				jeRam_Free( Object->MotionName );
				Object->MotionName = NULL;
			}
			Object->MotionName = Util_StrDup( pData->String );

			// do nothing more if no selection is made...
			if ( stricmp( pData->String, NoSelection ) == 0 )
			{
				Object->Motion = NULL;
				jeActor_ClearPose( Actor, &( Actor->Xf ) );
			}
			// ...or setup motion
			else
			{
				assert( Actor->ActorDefinition != NULL );
				Object->Motion = jeActor_GetMotionByName( Actor->ActorDefinition, pData->String );
				if ( Object->Motion != NULL )
				{
					Object->MotionTime = 0.0f;
                    jeActor_AnimationTestStep(Actor, Object->MotionTime);
			        jeActor_SetPose( Actor, Object->Motion, Object->MotionTime, &Actor->Xf );
				}
			}
			break;
		}

		// adjust light reference bone
		case ACTOROBJ_LIGHTREFERENCEBONENAMELIST_ID:
		{

			// ensure valid data
			assert( DataType == PROPERTY_COMBO_TYPE );
			assert( pData->String != NULL );
			assert( Object->BoneListSize > 0 );

			// save new bone name
			assert( Object->LightReferenceBoneName != NULL );
			jeRam_Free( Object->LightReferenceBoneName );
			Object->LightReferenceBoneName = Util_StrDup( pData->String );

			// do nothing more if no selection is made
			if ( stricmp( pData->String, NoSelection ) == 0 )
			{
				break;
			}

			// make sure bone exists
			assert( jeActor_DefHasBoneNamed( Actor->ActorDefinition, pData->String ) == JE_TRUE );

			// uncheck per bone lighting flag
			Object->PerBoneLighting = JE_FALSE;

			// set adjust flag
			AdjustActorProperties = JE_TRUE;
			break;
		}

		// adjust material number choice
		case ACTOROBJ_MATERIALLIST_ID:
		{

			// locals
			int i;

			// ensure valid data
			assert( DataType == PROPERTY_COMBO_TYPE );
			assert( pData->String != NULL );
			assert( Object->MaterialList != NULL );
			assert( Object->MaterialListSize > 0 );

			// do nothing if no actor has been loaded
			if ( Actor == NULL )
			{
				break;
			}

			// determine new desired material number choice
			Object->MaterialCurrent = 0;
			for ( i = 0; i < Object->MaterialListSize; i++ )
			{
				if ( stricmp( pData->String, Object->MaterialList[i] ) == 0 )
				{
					Object->MaterialCurrent = i;
				}
			}

			// make sure the property list gets rebuilt on screen
			ActorObjPropertyList.bDirty = JE_TRUE;
			break;
		}

		// adjust material overide choice
		case ACTOROBJ_MATERIALOVERIDE_ID:
		{

			// locals
			int			i;
			jeBoolean	MaterialSet = JE_FALSE;

			// ensure valid data
			assert( DataType == PROPERTY_COMBO_TYPE );
			assert( pData->String != NULL );
			assert( Object->MaterialOverideList != NULL );
			assert( Object->MaterialListSize > 0 );

			// do nothing if no actor has been loaded
			if ( Actor == NULL )
			{
				break;
			}

			// determine which overide bitmap was picked
			for ( i = 0; i < Bitmaps->Total; i++ )
			{
				if ( stricmp( Bitmaps->Name[i], pData->String ) == 0 )
				{

					// locals
					jeMaterialSpec	*Bitmap;
					jeFloat		Red, Green, Blue;
					jeUVMapper	Mapper;

					// free old overide bitmap
					if ( Object->MaterialOverideBitmap[Object->MaterialCurrent] != NULL )
					{
						//undone
						/*jeEngine_RemoveBitmap( Object->Engine, Object->MaterialOverideBitmap[Object->MaterialCurrent] );
						if ( jeResource_Delete( Object->ResourceMgr, Object->MaterialOverideList[Object->MaterialCurrent] ) == 0 )
						{
							jeBitmap_Destroy( &( Object->MaterialOverideBitmap[Object->MaterialCurrent] ) );
						}*/
						Object->MaterialOverideBitmap[Object->MaterialCurrent] = NULL;
					}

					// get overide bitmap name
					Object->MaterialOverideList[Object->MaterialCurrent] = Bitmaps->Name[i];

					// get default material if no overide was selected...
					if ( stricmp( Object->MaterialOverideList[Object->MaterialCurrent],  NoSelection ) == 0 )
					{

						// locals
						jeBody		*Body;
						jeBoolean	Result;
						const char	*DefaultMaterialName;

						// get actor def body
						Body = jeActor_GetBody( Actor->ActorDefinition );
						assert ( Body != NULL );

						// get default material
						Result = jeBody_GetMaterial(	Body, Object->MaterialCurrent, &DefaultMaterialName,
														&Bitmap, &Red, &Green, &Blue,
														&Mapper );
						if ( Result == JE_FALSE )
						{
							jeErrorLog_Add( JE_ERR_SUBSYSTEM_FAILURE, "Failed to get default actor material" );
							return JE_FALSE;
						}

						// zap overide bitmap pointer
						Object->MaterialOverideBitmap[Object->MaterialCurrent] = NULL;
					}
					// ...otherwise replace the material with the selected one
					else
					{

						// set defaults
						Red = Green = Blue = 255.0f;
						Mapper = NULL;

						// get bitmap
						//undone
						//Bitmap = jeResource_Get( Object->ResourceMgr, Object->MaterialOverideList[Object->MaterialCurrent] );
						Bitmap = NULL;

						// if it doesn't exist then create it
						if ( Bitmap == NULL )
						{

							// locals
							jeVFile	*FileDir;

							// get vfile dir
							FileDir = jeResource_GetVFile( Object->ResourceMgr, "GlobalMaterials" );
							if ( FileDir == NULL )
							{
								jeErrorLog_Add( JE_ERR_SUBSYSTEM_FAILURE, NULL );
								return JE_FALSE;
							}

							// create new art
							Bitmap = Util_CreateBitmapFromFileName( FileDir, Object->MaterialOverideList[Object->MaterialCurrent], NULL, Object->ResourceMgr );
		
							// close vfile dir
							if ( jeResource_DeleteVFile( Object->ResourceMgr, "GlobalMaterials" ) == 0 )
							{
								jeVFile_Close( FileDir );
							}

							// fail if art wasn't created
							if ( Bitmap == NULL )
							{
								jeErrorLog_Add( JE_ERR_SUBSYSTEM_FAILURE, "Failed to create bitmap" );
								return JE_FALSE;
							}

							// add it to the resource manager and engine
							//undone
							//jeResource_Add( Object->ResourceMgr, Object->MaterialOverideList[Object->MaterialCurrent], Bitmap );
							//jeEngine_AddBitmap( Object->Engine, Bitmap, JE_ENGINE_BITMAP_TYPE_3D );
						}

						// save overide bitmap pointer
						Object->MaterialOverideBitmap[Object->MaterialCurrent] = Bitmap;
					}

					// apply new material
					if ( Bitmap != NULL )
					{

						// locals
						int	MapperIndex;

						// overide mapper choice if required
						for ( MapperIndex = 0; MapperIndex < MaterialMapperTableSize; MapperIndex++ )
						{
							if ( stricmp( Object->MaterialMapperList[Object->MaterialCurrent], MaterialMapperTable[MapperIndex].Name ) == 0 )
							{
								if ( MaterialMapperTable[MapperIndex].Mapper != NULL )
								{
									Mapper = MaterialMapperTable[MapperIndex].Mapper;
								}
							}
						}

						// apply changes
						MaterialSet = jeActor_SetMaterial(	Actor, Object->MaterialCurrent,
															Bitmap,
															Red, Green, Blue,
															Mapper );
					}
					else
					{
						MaterialSet = JE_TRUE;
					}
				}
			}

			// log errors
			if ( MaterialSet == JE_FALSE )
			{
				jeErrorLog_Add( JE_ERR_SUBSYSTEM_FAILURE, "Failed to change actor material" );
			}
			break;
		}

		// adjust material mapper choice
		case ACTOROBJ_MATERIALMAPPER_ID:
		{

			// ensure valid data
			assert( DataType == PROPERTY_COMBO_TYPE );
			assert( pData->String != NULL );
			assert( Object->MaterialMapperList != NULL );
			assert( Object->MaterialListSize > 0 );

			// do nothing if no actor has been loaded
			if ( Actor == NULL )
			{
				break;
			}

			// assign mapper
			{

				// locals
				jeBody		*Body;
				jeMaterialSpec	*Material;
				char		*MaterialName;
				jeFloat		Red, Green, Blue;
				jeUVMapper	Mapper;
				jeBoolean	Result;
				int			MapperIndex;

				// get actor def body
				Body = jeActor_GetBody( Actor->ActorDefinition );
				assert ( Body != NULL );

				// get default material info
				Result = jeBody_GetMaterial(	Body, Object->MaterialCurrent, (const char **)&MaterialName,
												&Material, &Red, &Green, &Blue,
												&Mapper );

				// determine which mapper was picked
				for ( MapperIndex = 0; MapperIndex < MaterialMapperTableSize; MapperIndex++ )
				{
					if ( stricmp( MaterialMapperTable[MapperIndex].Name, pData->String ) == 0 )
					{
						if ( Object->MaterialOverideBitmap[Object->MaterialCurrent] != NULL )
						{
							Material = Object->MaterialOverideBitmap[Object->MaterialCurrent];
						}
						Result = jeActor_SetMaterial(	Actor, Object->MaterialCurrent,
														Material,
														Red, Green, Blue,
														MaterialMapperTable[MapperIndex].Mapper );
						Object->MaterialMapperList[Object->MaterialCurrent] = MaterialMapperTable[MapperIndex].Name;
					}
				}
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

	// adjust actor properties if required
	if ( ( Object != NULL ) && ( AdjustActorProperties == JE_TRUE ) )
	{				
		// scale to current settings
		if(Actor->ActorDefinition)
		{
		// locals
		char	*LightBone;
		jeVec3d	FillLightNormal;
			jeExtBox ExtBox;
			jeVec3d Translation;

		// if bone name is noselection string then null it
		LightBone = Object->LightReferenceBoneName;
		if ( stricmp( Object->LightReferenceBoneName, NoSelection ) == 0 )
		{
			Object->LightReferenceBoneName = NULL;
		}

		// setup fill light normal
		{

			// locals
			jeVec3d		Angles;
			jeVec3d		TempLightNormal;
			jeXForm3d	Xf;

			// convert each degree to radians
			Angles.X = jeFloat_DegToRad( Object->FillLightNormal.X );
			Angles.Y = jeFloat_DegToRad( Object->FillLightNormal.Y );
			Angles.Z = jeFloat_DegToRad( Object->FillLightNormal.Z );

			// build matrix from radian angles
			jeXForm3d_SetEulerAngles( &Xf, &Angles );

			// get temporary actor relative normal
			jeXForm3d_GetIn( &Xf, &TempLightNormal );

			// keep it as actor relative normal
			if ( Object->FillNormalActorRelative == JE_TRUE )
			{
				jeVec3d_Copy( &TempLightNormal, &FillLightNormal );
			}
			// ...or convert it to world relative normal
			else
			{

				// locals
				jeBoolean	Result;
				jeXForm3d	XfT;

				// get actors current xf
				Result = jeActor_GetBoneTransform( Actor, NULL, &Xf );
				assert( Result == JE_TRUE );

				// build new normal
				jeXForm3d_GetTranspose( &Xf, &XfT );
				jeXForm3d_Rotate( &XfT, &TempLightNormal, &FillLightNormal );
			}

			// make sure its normalize
			jeVec3d_Normalize( &FillLightNormal );
		}

		// make lighting adjustments
		jeActor_SetLightingOptions(	Actor,
									Object->UseFillLight,
									&FillLightNormal,
									Object->FillLightRed,
									Object->FillLightGreen,
									Object->FillLightBlue,
									Object->AmbientLightRed,
									Object->AmbientLightGreen,
									Object->AmbientLightBlue,
									Object->UseAmbientLightFromFloor,
									Object->MaximumDynamicLightsToUse,
									Object->MaximumStaticLightsToUse,
									Object->LightReferenceBoneName,
									Object->PerBoneLighting );

		// Scale
		jeActor_SetScale( Actor, Object->ScaleX, Object->ScaleY, Object->ScaleZ );

		// Update boxes
		ExtBox = Object->CollisionExtBox;
		GetBoxTranslation(&ExtBox, Actor, &Translation);
		jeVec3d_Subtract(&Translation,&Actor->Xf.Translation,&Translation); // Take out actor pos (make relative)
		jeExtBox_Translate( &ExtBox, Translation.X, Translation.Y, Translation.Z );
		jeExtBox_Scale( &ExtBox, Object->ScaleX, Object->ScaleZ, Object->ScaleY );			
		jeActor_SetExtBox( Actor, &ExtBox, NULL );

		ExtBox = Object->RenderHintExtBox;
		GetBoxTranslation(&ExtBox, Actor, &Translation);
		jeVec3d_Subtract(&Translation,&Actor->Xf.Translation,&Translation); // Take out actor pos (make relative)
		jeExtBox_Translate( &ExtBox, Translation.X, Translation.Y, Translation.Z );
		jeExtBox_Scale( &ExtBox, Object->ScaleX, Object->ScaleZ, Object->ScaleY );
		jeActor_SetRenderHintExtBox( Actor, &ExtBox, NULL );

		// restore bone name
		Object->LightReferenceBoneName = LightBone;
		//SetupGlobalBrush(Object );

		}
	}

	// all done
	return JE_TRUE;

	// eliminate warnings
	DataType;

} // SetProperty()
