/****************************************************************************************/
/*  PULSINGLIGHT.C                                                                      */
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
/*  Modified December 15, 2002 by Tom Morris                                                                                    */
/****************************************************************************************/
#ifdef WIN32
#pragma warning ( disable : 4115 )
#include <windows.h>
#pragma warning ( default : 4115 )
#endif

#ifdef BUILD_BE
#include <image.h>
#include <Resources.h>
#endif

#include <assert.h>
#include <float.h>
#include <stdio.h>
#include <math.h>
#include "VFile.h"
#include "jeProperty.h"
#include "Ram.h"
#include "jeResource.h"
#include "jeWorld.h"
#include "PulsingLight.h"
#include "Resource.h"
#include "Errorlog.h"
#include "jeMaterial.h"
#include "jeResource.h"




#define PULSINGLIGHTOBJECT_VERSION 1



//Royce
#define OBJ_PERSIST_SIZE 5000
//---


//	TODO - cleanup and document

////////////////////////////////////////////////////////////////////////////////////////
//	Property list stuff
////////////////////////////////////////////////////////////////////////////////////////
enum
{
	PULSINGLIGHT_RADIUS_ID = PROPERTY_LOCAL_DATATYPE_START,
	PULSINGLIGHT_BRIGHTNESS_ID,

	//	static or pulsing
	PULSINGLIGHT_PULSEGROUP_ID,
	PULSINGLIGHT_PULSING_ID,
	PULSINGLIGHT_PULSESPEED_ID,
	PULSINGLIGHT_PULSINGCOMBO_ID,
	PULSINGLIGHT_PULSEGROUPEND_ID,

	// color
	PULSINGLIGHT_COLORGROUP_ID,
	PULSINGLIGHT_COLORRED_ID,
	PULSINGLIGHT_COLORGREEN_ID,
	PULSINGLIGHT_COLORBLUE_ID,
	PULSINGLIGHT_COLOR_ID,
	PULSINGLIGHT_COLORGROUPEND_ID,

	//	shadow
	PULSINGLIGHT_CASTSHADOW_ID,

	//	world icon
	PULSINGLIGHT_DISPLAY_ICON_ID,
	PULSINGLIGHT_LAST_ID
};
enum
{
	PULSINGLIGHT_RADIUS_INDEX = 0,
	PULSINGLIGHT_BRIGHTNESS_INDEX,

	//	static or pulsing
	PULSINGLIGHT_PULSEGROUP_INDEX,
	PULSINGLIGHT_PULSING_INDEX,
	PULSINGLIGHT_PULSESPEED_INDEX,
	PULSINGLIGHT_PULSINGCOMBO_INDEX,
	PULSINGLIGHT_PULSEGROUPEND_INDEX,

	// color
	PULSINGLIGHT_COLORGROUP_INDEX,
	PULSINGLIGHT_COLORRED_INDEX,
	PULSINGLIGHT_COLORGREEN_INDEX,
	PULSINGLIGHT_COLORBLUE_INDEX,
	PULSINGLIGHT_COLOR_INDEX,
	PULSINGLIGHT_COLORGROUPEND_INDEX,

	//	shadow
	PULSINGLIGHT_CASTSHADOW_INDEX,

	//	world icon
	PULSINGLIGHT_DISPLAY_ICON_INDEX,
	PULSINGLIGHT_LAST_INDEX
};


////////////////////////////////////////////////////////////////////////////////////////
//	Globals
//
////////////////////////////////////////////////////////////////////////////////////////
#ifdef WIN32
static HINSTANCE		hClassInstance = NULL;
#endif

#ifdef BUILD_BE
static image_id			hClassInstance = NULL;
#endif

static jeProperty		PulsingLightProperties[PULSINGLIGHT_LAST_INDEX];
static jeProperty_List	PulsingLightPropertyList = { PULSINGLIGHT_LAST_INDEX, &( PulsingLightProperties[0] ) };

//	support for pulse patterns
char	charNoSelection[10] = "< none >";
static char		*pStaticNoSelection = charNoSelection;

//	support for world icon
static 	jeBitmap		*m_pBitmap = NULL;
static jeMaterialSpec *MatSpec;

////////////////////////////////////////////////////////////////////////////////////////
//	Defaults
//
////////////////////////////////////////////////////////////////////////////////////////
#define PULSINGLIGHT_DEFAULT_RADIUS			50.0f
#define PULSINGLIGHT_DEFAULT_BRIGHTNESS		20.0f
#define	PULSINGLIGHT_DEFAULT_PULSING		0
#define PULSINGLIGHT_DEFAULT_PATTERN		"aaaaaaaazzzzzzzz"
#define PULSINGLIGHT_DEFAULT_COLORRED		128.0f
#define PULSINGLIGHT_DEFAULT_COLORGREEN		128.0f
#define PULSINGLIGHT_DEFAULT_COLORBLUE		128.0f
#define PULSINGLIGHT_DEFAULT_CASTSHADOW		JE_FALSE
#define	PULSINGLIGHT_DEFAULT_PULSESPEED		1
#define	PULSINGLIGHT_DEFAULT_RADIUS_SPEED	2.0f

//	hack to establish the quantity of available pulse patterns
#define	PULSINGLIGHT_NO_OF_PATTERNS			16

#define	PULSINGLIGHT_DEFAULT_DISPLAY_ICON	JE_FALSE

//	pulse pattern strings here correspond 1:1 to pattern Names below.
//	if you add or subtract from these arrays, you must revise
//	the #define	PULSINGLIGHT_NO_OF_PATTERNS to reflect the new quantity

	char	*m_pcharPulsePattern[] = 
    {
		"aaaaaaaazzzzzzzz",
		"amzzma",
		"abcdefghijklmnopqrstuvwxyzyxwvutsrqponmlkjihgfedcba",
		"jklmnopqrstuvwxyzyxwvutsrqponmlkj",
		"abcdefghijklmnopqrrqponmlkjihgfedcba",
		"az",
		"aaazzz",
		"",
		"zzaaaa",
		"mmnmmommommnonmmonqnmmo",
		"mmamammmmammamamaaamammma",
		"mmmmmaaaaammmmmaaaaaabcdefgabcdefg",
		"mamamamamama",
		"nmonqnmomnmomomno",
		"mmmaaaabcdefgmmmmaaaammmaamm",
		"mmmaaammmaaammmabcdefaaaammmmabcdefmmmaaaa"
	};

	char **m_ppPulsePattern = m_pcharPulsePattern;

	//	pulse pattern names correspond 1:1 to pulse pattern strings above
	char	*m_pcharPulseName[] = 
    {
		"On/Off",
		"Grow/Decay1",
		"Grow/Decay2",
		"Grow/Decay3",
		"Grow/Decay4",
		"Grow/On",
		"Grow/On2",
		"Grow/Off",
		"On/Decay",
		"Flame",
		"Fire",
		"Blink Blink",
		"Drum Roll",
		"Neon",
		"Heartbeat",
		"Crash Scene"
	};

	char **m_ppPulseName = m_pcharPulseName;
	
////////////////////////////////////////////////////////////////////////////////////////
//	PulsingLight Object struct
//
////////////////////////////////////////////////////////////////////////////////////////
typedef struct PulsingLight
{
	jeWorld			*pWorld;
	jeResourceMgr	*pResourceMgr;
	jeEngine		*pEngine;
	int				RefCount;
	jeXForm3d		Xf;
	jeLight			*pLight;
	jeUserPoly		*pPoly;
	jeLVertex		Vertex;
	int				iPulsing;
	float			fRadiusSpeed;
	int				iPulseSpeed;
	float			fLastTime;
	char			*pcharPulsePattern;	
	char			charPulsePattern[MAX_PATH];
	jeVec3d			Color;
	float			Radius;
	float			Brightness;
	jeBoolean		CastShadow;
	jeBoolean		LoadedFromDisk;
	jeBoolean		bDisplayIcon;
} PulsingLight;


////////////////////////////////////////////////////////////////////////////////////
//	Util_GetAppPath
//	
////////////////////////////////////////////////////////////////////////////////////
static int Util_GetAppPath(
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

} 



#ifdef WIN32

////////////////////////////////////////////////////////////////////////////////////////
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


////////////////////////////////////////////////////////////////////////////////////
//	Util_LoadLibraryString
//	
////////////////////////////////////////////////////////////////////////////////////
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
 
	printf("Read %s\n" , rcbuffer);
	// return the allocated string
	return (rcbuffer);
}//Util_LoadLibraryString

#endif


////////////////////////////////////////////////////////////////////////////////////////
//	PulsingLight_Destroy()
//
////////////////////////////////////////////////////////////////////////////////////////
static jeBoolean PulsingLight_Destroy(
	PulsingLight	*pObject )	// object from which light will be created
{

	// ensure valid data
	assert( pObject != NULL );

	// remove light from world
	assert( pObject->pLight != NULL );
	assert( pObject->pWorld != NULL );
	if ( jeWorld_RemoveDLight( pObject->pWorld, pObject->pLight ) == JE_FALSE )
	{
		jeErrorLog_Add( JE_ERR_INTERNAL_RESOURCE, NULL );
		return JE_FALSE;
	}

	// destroy light
	jeLight_Destroy( &pObject->pLight );

	if (pObject->pPoly)
	{
		jeUserPoly_Destroy(&pObject->pPoly);
	}

	// all done
	return JE_TRUE;

} // PulsingLight_Destroy()



////////////////////////////////////////////////////////////////////////////////////////
//	PulsingLight_Create()
//
////////////////////////////////////////////////////////////////////////////////////////
static jeBoolean PulsingLight_Create(
									 PulsingLight	*pObject )	// object from which light will be created
{

	// locals
	jeBoolean	Result = JE_FALSE;

	// ensure valid data
	assert( pObject != NULL );

	if (pObject)
	{
		// create light
		pObject->pLight = NULL;
		pObject->pLight = jeLight_Create();
		if ( pObject->pLight == NULL )
		{
			jeErrorLog_Add( JE_ERR_INTERNAL_RESOURCE, NULL );
			return JE_FALSE;
		}

		// set light attributes
		Result = jeLight_SetAttributes(	pObject->pLight,
			&( pObject->Xf.Translation ),
			&( pObject->Color ),
			pObject->Radius, 
			pObject->Brightness, 
			JE_LIGHT_FLAG_FAST_LIGHTING_MODEL );	//undone, dont know flags for cast shadow
#pragma message ("shadow flags")	

		if ( Result == JE_FALSE )
		{
			jeErrorLog_Add( JE_ERR_INTERNAL_RESOURCE, NULL );
			jeLight_Destroy( &( pObject->pLight ) );
			return JE_FALSE;
		}

		// add it to the world
		Result = jeWorld_AddDLight( pObject->pWorld, pObject->pLight );
		if ( Result == JE_FALSE )
		{
			jeErrorLog_Add( JE_ERR_INTERNAL_RESOURCE, NULL );
			jeLight_Destroy( &( pObject->pLight ) );
			return JE_FALSE;
		}

		// all done
		return JE_TRUE;

	}	//	if (pObject)...
	return JE_TRUE;

} // PulsingLight_Create()




//////////////////////////////////////////////////////////////////////////////
//  LOCAL BITMAP RELATED


////////////////////////////////////////////////////////////////////////////////////
//	PulsingLight_LoadBmp
//	
////////////////////////////////////////////////////////////////////////////////////
static jeBoolean PulsingLight_LoadBmp()
{
	// Jeff:  Load PulsingLight bitmap from resources - 8/18/2005
	jeVFile	*pBmpFile;
	HRSRC hFRes; 
    HGLOBAL hRes; 
    jeVFile_MemoryContext Context; 
    HINSTANCE hInst;
    
	#ifdef _DEBUG 
	    hInst = LoadLibrary("PulsingLightObj.ddl");
	#else
        hInst = LoadLibrary("PulsingLightObj.dll");
    #endif
    hFRes = FindResource(hInst, MAKEINTRESOURCE(IDR_PULSINGLIGHT) ,"jeBitmap"); 
    hRes = LoadResource(hInst, hFRes) ;  
    
    Context.Data  = LockResource(hRes); 
    Context.DataLength = SizeofResource(hInst,hFRes); 

	pBmpFile = jeVFile_OpenNewSystem(NULL, JE_VFILE_TYPE_MEMORY,	NULL,
		                            &Context,JE_VFILE_OPEN_READONLY  );

	if( pBmpFile == NULL )
		return( JE_FALSE );

	m_pBitmap = jeBitmap_CreateFromFile( pBmpFile );
	
	jeVFile_Close( pBmpFile );

	if( m_pBitmap == NULL )
		return( JE_FALSE );

	jeBitmap_SetColorKey( m_pBitmap, JE_TRUE, 255, JE_TRUE );
	return( JE_TRUE );
	
}


////////////////////////////////////////////////////////////////////////////////////
//	PulsingLight_InitIcon
//	
////////////////////////////////////////////////////////////////////////////////////
static jeBoolean PulsingLight_InitIcon( PulsingLight * pObject )
{
	assert(pObject != NULL);

	//Royce-2
	assert(m_pBitmap);
	/*
	if( pBitmap == NULL )
	if( !AmbObject_LoadBmp() )
	return( JE_FALSE );*/
	//---

	if (pObject)
	{
		pObject->pPoly = NULL;

		if (m_pBitmap)
		{
			pObject->Vertex.r = 255.0f;
			pObject->Vertex.g = 255.0f;
			pObject->Vertex.b = 255.0f;
			pObject->Vertex.a = 255.0f;
			pObject->Vertex.u = 0.0f;
			pObject->Vertex.v = 0.0f;
			pObject->Vertex.sr = 255.0f;
			pObject->Vertex.sg = 255.0f;
			pObject->Vertex.sb = 255.0f;

			pObject->Vertex.X = 0.0f;
			pObject->Vertex.Y = 0.0f;
			pObject->Vertex.Z = 0.0f;

			if (!MatSpec)
	        {
	            MatSpec = jeMaterialSpec_Create(jeResourceMgr_GetEngine(jeResourceMgr_GetSingleton()), jeResourceMgr_GetSingleton());
#pragma message ("Krouer: change NULL to something better next time")
	            jeMaterialSpec_AddLayerFromBitmap(MatSpec, 0, m_pBitmap, NULL);
	        }
			pObject->pPoly = jeUserPoly_CreateSprite(&pObject->Vertex,
				MatSpec,
				1.0f,
				JE_RENDER_FLAG_ALPHA | JE_RENDER_FLAG_NO_ZWRITE );

			return JE_TRUE;
		}	//	if (m_pBitmap)...
		return JE_FALSE;
	}	//	if (pObject)...
	return JE_FALSE;
}				


////////////////////////////////////////////////////////////////////////////////////
//	PulsingLight_UpdateIcon
//	
////////////////////////////////////////////////////////////////////////////////////
static jeBoolean PulsingLight_UpdateIcon( PulsingLight * pObject )
{
	assert(pObject != NULL);

	pObject->Vertex.X = pObject->Xf.Translation.X;
	pObject->Vertex.Y = pObject->Xf.Translation.Y;
	pObject->Vertex.Z = pObject->Xf.Translation.Z;

	if (pObject)
	{
		if (m_pBitmap)
		{
			if (pObject->pPoly)
			{
				if (pObject->bDisplayIcon)
				{
					if (!MatSpec)
	                {
	                    MatSpec = jeMaterialSpec_Create(jeResourceMgr_GetEngine(jeResourceMgr_GetSingleton()), jeResourceMgr_GetSingleton());
#pragma message ("Krouer: change NULL to something better next time")
	                    jeMaterialSpec_AddLayerFromBitmap(MatSpec, 0, m_pBitmap, NULL);
	                }
					jeUserPoly_UpdateSprite(pObject->pPoly, &pObject->Vertex, MatSpec, 1.0f);
				}
			}
		}	//	if (m_pBitmap)...
		return JE_TRUE;
	}	//	if (pObject)...
	return JE_TRUE;
}				


////////////////////////////////////////////////////////////////////////////////////////
//	Init_Class()
//
////////////////////////////////////////////////////////////////////////////////////////
void Init_Class(
#ifdef WIN32
	HINSTANCE	hInstance )	// dll instance handle
#endif
#ifdef BUILD_BE
	image_id	hInstance )	// dll instance handle
#endif
{
#ifdef WIN32
	// ensure valid data
	assert( hInstance != NULL );
#endif
#ifdef BUILD_BE
	// ensure valid data
	assert( hInstance > 0 );
#endif
	// save hinstance
	hClassInstance = hInstance;

	// setup radius property
	jeProperty_FillFloat(	&( PulsingLightProperties[PULSINGLIGHT_RADIUS_INDEX] ),
							Util_LoadLibraryString( hClassInstance, IDS_RADIUS ),
							PULSINGLIGHT_DEFAULT_RADIUS,
							PULSINGLIGHT_RADIUS_ID,
							0.1f, FLT_MAX, 5.0f );

	// setup brightness property
	jeProperty_FillFloat(	&( PulsingLightProperties[PULSINGLIGHT_BRIGHTNESS_INDEX] ),
							Util_LoadLibraryString( hClassInstance, IDS_BRIGHTNESS ),
							PULSINGLIGHT_DEFAULT_BRIGHTNESS,
							PULSINGLIGHT_BRIGHTNESS_ID,
							0.1f, FLT_MAX, 5.0f );


	// start color group
	jeProperty_FillGroup(	&( PulsingLightProperties[PULSINGLIGHT_PULSEGROUP_INDEX] ),
							Util_LoadLibraryString( hClassInstance, IDS_PULSEGROUP ),
							PULSINGLIGHT_PULSEGROUP_INDEX );

	jeProperty_FillCheck(&( PulsingLightProperties[PULSINGLIGHT_PULSING_INDEX] ),
							Util_LoadLibraryString( hClassInstance, IDS_PULSING_LIGHT ),
							PULSINGLIGHT_DEFAULT_PULSING,
							PULSINGLIGHT_PULSING_ID);

	jeProperty_FillInt(	&( PulsingLightProperties[PULSINGLIGHT_PULSESPEED_INDEX] ),
							Util_LoadLibraryString( hClassInstance, IDS_PULSE_SPEED ),
							PULSINGLIGHT_DEFAULT_PULSESPEED,
							PULSINGLIGHT_PULSESPEED_ID,
							1.0f, 100.0f, 1.0f );

	jeProperty_FillCombo( &( PulsingLightProperties[PULSINGLIGHT_PULSINGCOMBO_INDEX] ),
							Util_LoadLibraryString( hClassInstance, IDS_PULSE_PATTERN),
							pStaticNoSelection, PULSINGLIGHT_PULSINGCOMBO_ID, 0, &pStaticNoSelection);
	// end pulse group
	jeProperty_FillGroupEnd( &( PulsingLightProperties[PULSINGLIGHT_PULSEGROUPEND_INDEX] ), PULSINGLIGHT_COLORGROUPEND_ID );



	// start color group
	jeProperty_FillGroup(	&( PulsingLightProperties[PULSINGLIGHT_COLORGROUP_INDEX] ),
							Util_LoadLibraryString( hClassInstance, IDS_COLORGROUP ),
							PULSINGLIGHT_COLORGROUP_INDEX );

	//	Color properties
	////////////////////////////////////////////////////////////////////////////////////////

	// start color group
	jeProperty_FillGroup(	&( PulsingLightProperties[PULSINGLIGHT_COLORGROUP_INDEX] ),
							Util_LoadLibraryString( hClassInstance, IDS_COLORGROUP ),
							PULSINGLIGHT_COLORGROUP_INDEX );
	
	// setup color red property
	jeProperty_FillFloat(	&( PulsingLightProperties[PULSINGLIGHT_COLORRED_INDEX] ),
							Util_LoadLibraryString( hClassInstance, IDS_COLORRED ),
							PULSINGLIGHT_DEFAULT_COLORRED,
							PULSINGLIGHT_COLORRED_ID,
							0.0f, 255.0f, 1.0f );

	// setup ambient light green property
	jeProperty_FillFloat(	&( PulsingLightProperties[PULSINGLIGHT_COLORGREEN_INDEX] ),
							Util_LoadLibraryString( hClassInstance, IDS_COLORGREEN ),
							PULSINGLIGHT_DEFAULT_COLORGREEN,
							PULSINGLIGHT_COLORGREEN_ID,
							0.0f, 255.0f, 1.0f );

	// setup ambient light blue property
	jeProperty_FillFloat(	&( PulsingLightProperties[PULSINGLIGHT_COLORBLUE_INDEX] ),
							Util_LoadLibraryString( hClassInstance, IDS_COLORBLUE ),
							PULSINGLIGHT_DEFAULT_COLORBLUE,
							PULSINGLIGHT_COLORBLUE_ID,
							0.0f, 255.0f, 1.0f );

	// setup color property
	{
		jeVec3d	Color = { PULSINGLIGHT_DEFAULT_COLORRED, PULSINGLIGHT_DEFAULT_COLORGREEN, PULSINGLIGHT_DEFAULT_COLORBLUE };
		jeProperty_FillColorPicker(	&( PulsingLightProperties[PULSINGLIGHT_COLOR_INDEX] ),
									Util_LoadLibraryString( hClassInstance, IDS_COLOR ),
									&Color,
									PULSINGLIGHT_COLOR_ID );
	}


	// end color group
	jeProperty_FillGroupEnd( &( PulsingLightProperties[PULSINGLIGHT_COLORGROUPEND_INDEX] ), PULSINGLIGHT_COLORGROUPEND_ID );


	//	Misc properties
	////////////////////////////////////////////////////////////////////////////////////////

	// setup cast shadow flag
	jeProperty_FillCheck(	&( PulsingLightProperties[PULSINGLIGHT_CASTSHADOW_INDEX] ),
							Util_LoadLibraryString( hClassInstance, IDS_CASTSHADOW ),
							PULSINGLIGHT_DEFAULT_CASTSHADOW,
							PULSINGLIGHT_CASTSHADOW_ID );

	// setup display icon flag
	jeProperty_FillCheck(	&( PulsingLightProperties[PULSINGLIGHT_DISPLAY_ICON_INDEX] ),
							Util_LoadLibraryString( hClassInstance, IDS_DISPLAY_ICON ),
							PULSINGLIGHT_DEFAULT_DISPLAY_ICON,
							PULSINGLIGHT_DISPLAY_ICON_ID );


	// final init
	PulsingLightPropertyList.jePropertyN = PULSINGLIGHT_LAST_INDEX;

} // Init_Class()


////////////////////////////////////////////////////////////////////////////////////////
//	DeInit_Class()
//
////////////////////////////////////////////////////////////////////////////////////////
void DeInit_Class(
	void )	// no parameters
{
	if (m_pBitmap)
	{
		jeBitmap_Destroy(&m_pBitmap);
		m_pBitmap = NULL;
	}
	// zap instance pointer
	hClassInstance = NULL;

} // DeInit_Class()



////////////////////////////////////////////////////////////////////////////////////////
//	CreateInstance()
//
////////////////////////////////////////////////////////////////////////////////////////
void * JETCC CreateInstance(
	void )	// no parameters
{

	// locals
	PulsingLight	*pObject = NULL;

	// allocate struct
	pObject = (PulsingLight *)jeRam_AllocateClear( sizeof( *pObject ) );
	if ( pObject == NULL )
	{
		jeErrorLog_Add( JE_ERR_MEMORY_RESOURCE, NULL );
		return NULL;
	}

	//Royce-2
	PulsingLight_LoadBmp(); //failure is not fatal, don't bother checking
	//m_pBitmap is now our flag to tell us whether "pulsing_light.bmp" is in the
	//host exe's dir. If it is new instances will default bDisplayToggle ON
	//(i.e. we assume we're in the editor instead of some game)
	//---

	// get default settings
	pObject->Radius = PULSINGLIGHT_DEFAULT_RADIUS;
	pObject->Brightness = PULSINGLIGHT_DEFAULT_BRIGHTNESS;
	pObject->iPulsing = PULSINGLIGHT_DEFAULT_PULSING;
	pObject->iPulseSpeed = PULSINGLIGHT_DEFAULT_PULSESPEED;
	pObject->fRadiusSpeed = PULSINGLIGHT_DEFAULT_RADIUS_SPEED;
	pObject->fLastTime = 0.0f;
	pObject->pcharPulsePattern = PULSINGLIGHT_DEFAULT_PATTERN;
	pObject->Color.X = PULSINGLIGHT_DEFAULT_COLORRED;
	pObject->Color.Y = PULSINGLIGHT_DEFAULT_COLORGREEN;
	pObject->Color.Z = PULSINGLIGHT_DEFAULT_COLORBLUE;
	pObject->CastShadow = PULSINGLIGHT_DEFAULT_CASTSHADOW;

	// init remaining fields
	jeXForm3d_SetIdentity( &pObject->Xf );
	pObject->RefCount = 1;

	// all done
	return pObject;

} // CreateInstance()



////////////////////////////////////////////////////////////////////////////////////////
//	CreateRef()
//
////////////////////////////////////////////////////////////////////////////////////////
void JETCC CreateRef(
	void	*Instance )	// instance data
{

	// locals
	PulsingLight	*pObject = NULL;
	
	// get object
	pObject = (PulsingLight *)Instance;
	assert( pObject != NULL );

	if (pObject)
	{
		// adjust object ref count
		pObject->RefCount++;
	}
} // CreateRef()



////////////////////////////////////////////////////////////////////////////////////////
//	Destroy()
//
////////////////////////////////////////////////////////////////////////////////////////
jeBoolean JETCC Destroy(
						void	**Instance )	// pointer to instance data
{

	// locals
	PulsingLight	*pObject = NULL;

	// ensure valid data
	assert( Instance != NULL );

	// get object
	pObject = (PulsingLight *)*Instance;
	assert( pObject != NULL );
	assert( pObject->RefCount > 0 );

	if (pObject)
	{
		// do nothing if ref count is not at zero
		pObject->RefCount--;
		if ( pObject->RefCount > 0 )
		{
			return JE_FALSE;
		}

		if (pObject->pPoly)
		{
			jeUserPoly_Destroy(&pObject->pPoly);
			pObject->pPoly = NULL;
		}

		// make sure everything has been properly destroyed
		assert( pObject->pWorld == NULL );
		assert( pObject->pEngine == NULL );
		assert( pObject->pResourceMgr == NULL );
		assert( pObject->pLight == NULL );

		// free struct
		jeRam_Free( pObject );

		// zap pointer
		*Instance = NULL;

		return JE_TRUE;
	}	//	if (pObject)...

	// all done
	return JE_TRUE;

} // Destroy()



////////////////////////////////////////////////////////////////////////////////////////
//	Render()
//
////////////////////////////////////////////////////////////////////////////////////////
jeBoolean JETCC Render(
					   const void				*Instance,	// object instance data
					   const jeWorld			*World,		// world
					   const jeEngine			*Engine,	// engine
					   const jeCamera			*Camera,				// camera
					   const jeFrustum			*CameraSpaceFrustum, 	// frustum
					   jeObject_RenderFlags	RenderFlags)
{

	PulsingLight	*pObject = NULL;
	float	fPercentage = 0.0f;
	int		iIndex = 0;
	int		iNumFunctionValues = 0;
	float	fRadius = 0.0f;
	float	fPulseSpeed = 0.0f;
	float	fDummyFloat = 0.0f;
	float	Remainder = 0.0f;
	float	InterpolationPercentage = 0.0f;
	int		DeltaValue = 0;
	float	Value = 0.0f;
	float	fIntervalWidth = 0.0f;
	float	fMinRadius = 20.0f;

	// ensure valid data
	assert( Instance != NULL );

	// get object data
	pObject = (PulsingLight *)Instance;
	if (pObject)
	{
		fDummyFloat = (float)pObject->iPulseSpeed;
		fPulseSpeed = fDummyFloat/100.0f;

		if (pObject->pLight)
		{
			if (!pObject->iPulsing)
			{
				jeLight_SetAttributes(	pObject->pLight,
					&( pObject->Xf.Translation ),
					&( pObject->Color ),
					pObject->Radius, 
					pObject->Brightness, 
					JE_LIGHT_FLAG_FAST_LIGHTING_MODEL );	//undone dont know flags for cast shadow
			}	//	if (!pObject->iPulsing)...
			else
			{
				//	reset the interval length to reflect the current pattern
				iNumFunctionValues = strlen(pObject->pcharPulsePattern);
				if (iNumFunctionValues > 0)
				{
					fIntervalWidth = pObject->fRadiusSpeed / (float)iNumFunctionValues;
				}

				if (pObject->fRadiusSpeed > pObject->fLastTime)
				{
					fPercentage = pObject->fLastTime / pObject->fRadiusSpeed;

					iIndex = (int)(fPercentage * iNumFunctionValues);
					if (iIndex < iNumFunctionValues)
					{
						if	((iIndex < (iNumFunctionValues - 1)))
						{
							Remainder = (float)fmod(pObject->fLastTime, fIntervalWidth);
							InterpolationPercentage = Remainder / fIntervalWidth;
							DeltaValue = pObject->pcharPulsePattern[iIndex + 1] - pObject->pcharPulsePattern[iIndex];
							Value = pObject->pcharPulsePattern[iIndex] + DeltaValue * InterpolationPercentage;
							fPercentage = ((float)(Value - 'a')) / ((float)('z' - 'a'));
						}	//	if	((iIndex < (iNumFunctionValues - 1))) ...
						else
						{
							fPercentage = ((float)(pObject->pcharPulsePattern[iIndex] - 'a')) / ((float)('z' - 'a'));
						}	//	else...
					}	//	if (iIndex < iNumFunctionValues)...
					fRadius = fPercentage * (pObject->Radius - fMinRadius) + fMinRadius;

					jeLight_SetAttributes(	pObject->pLight,
						&( pObject->Xf.Translation ),
						&( pObject->Color ),
						fRadius, 
						pObject->Brightness, 
						JE_LIGHT_FLAG_FAST_LIGHTING_MODEL );

				}	//	if (pObject->fRadiusSpeed >...
				pObject->fLastTime = (float)fmod(pObject->fLastTime + fPulseSpeed, pObject->fRadiusSpeed);
			}	//	else...
		
			PulsingLight_UpdateIcon(pObject);

			// all done
			return JE_TRUE;

			// eliminate warnings
			Instance;
			World;
			Engine;
			Camera;
			CameraSpaceFrustum;
			RenderFlags;
		}	//	if (pObject->pLight)...
		return JE_TRUE;
	}	//	if (pObject)
	return JE_FALSE;
} // Render()



////////////////////////////////////////////////////////////////////////////////////////
//	AttachWorld()
//
////////////////////////////////////////////////////////////////////////////////////////
jeBoolean JETCC AttachWorld(
							void	*Instance,	// object instance data
							jeWorld	*World )	// world
{

	// locals
	PulsingLight	*pObject = NULL;

	// ensure valid data
	assert( Instance != NULL );
	assert( World != NULL );

	if (World)
	{
		// get object
		pObject = (PulsingLight *)Instance;

		if (pObject)
		{
			// save world pointer
			pObject->pWorld = World;

			// save an instance of the resource manager
			pObject->pResourceMgr = jeWorld_GetResourceMgr( World );
			assert( pObject->pResourceMgr != NULL );

			// create light
			if ( PulsingLight_Create( pObject ) == JE_FALSE )
			{
				jeErrorLog_Add( JE_ERR_INTERNAL_RESOURCE, NULL );
				jeResource_MgrDestroy( &( pObject->pResourceMgr ) );
				pObject->pWorld = NULL;
				return JE_FALSE;
			}

			// add the pAmbObj->Poly to the world
			if (pObject->bDisplayIcon && !pObject->pPoly ) 
			{
				if (PulsingLight_InitIcon(pObject)) 
				{
					if ( jeWorld_AddUserPoly( World, pObject->pPoly, JE_FALSE ) == JE_FALSE )
					{
						jeUserPoly_Destroy( &( pObject->pPoly ) );
						return JE_FALSE;
					}
				}
				else return JE_FALSE;

				PulsingLight_UpdateIcon(pObject);
			}	//	if (pObject->bDisplayIcon && ...

			// all done
			return JE_TRUE;
		}	//	if (pObject)...

		return JE_FALSE;
	}	//	if (World)...
	return JE_FALSE;


} // AttachWorld()



////////////////////////////////////////////////////////////////////////////////////////
//	DettachWorld()
//
////////////////////////////////////////////////////////////////////////////////////////
jeBoolean JETCC DettachWorld(
							 void	*Instance,	// object instance data
							 jeWorld	*pWorld )	// world
{

	// locals
	PulsingLight	*pObject = NULL;

	// ensure valid data
	assert( Instance != NULL );
	assert( pWorld != NULL );

	// get object
	pObject = (PulsingLight *)Instance;
	assert( pObject->pWorld == pWorld );

	if (pObject)
	{
		if (pObject->pPoly) 
		{
			if (pObject->bDisplayIcon) 
				if ( jeWorld_RemoveUserPoly( pWorld, pObject->pPoly) == JE_FALSE ) 
					return JE_FALSE;

			jeUserPoly_Destroy(&(pObject->pPoly));
		}


		// destroy light
		PulsingLight_Destroy( pObject );

		// destroy our instance of the resource manager
		jeResource_MgrDestroy( &( pObject->pResourceMgr ) );

		// zap world pointer
		pObject->pWorld = NULL;

		// all done
		return JE_TRUE;

	}	//	if (pObject)...
	return JE_FALSE;
	// eliminate warnings
	pWorld;

} // DettachWorld()



////////////////////////////////////////////////////////////////////////////////////////
//	AttachEngine()
//
////////////////////////////////////////////////////////////////////////////////////////
jeBoolean JETCC AttachEngine(
							 void		*Instance,	// object instance data
							 jeEngine	*Engine )	// engine
{
	// locals
	PulsingLight	*pObject = NULL;

	// ensure valid data
	assert( Instance != NULL );
	assert( Engine != NULL );

	// get object data
	pObject = (PulsingLight *)Instance;

	if (pObject && Engine)
	{
		// save engine pointer
		pObject->pEngine = Engine;

		if( m_pBitmap )
			return( jeEngine_AddBitmap( (jeEngine*)Engine, m_pBitmap, JE_ENGINE_BITMAP_TYPE_3D ) );	

		return JE_TRUE;
	}	//	if (pObject)...
	return JE_FALSE;

} // AttachEngine()



////////////////////////////////////////////////////////////////////////////////////////
//	DettachEngine()
//
////////////////////////////////////////////////////////////////////////////////////////
jeBoolean JETCC DettachEngine(
							  void		*Instance,	// object instance data
							  jeEngine	*Engine )	// engine
{

	// locals
	PulsingLight	*pObject = NULL;

	// ensure valid data
	assert( Instance != NULL );
	assert( Engine != NULL );

	// get object data
	pObject = (PulsingLight *)Instance;
	if (pObject)
	{
		assert( pObject->pEngine == Engine );

		// zap engine pointer
		pObject->pEngine = NULL;

		// all done
		return JE_TRUE;

	}	//	if (pObject)...
	// eliminate warnings
	Engine;

	return JE_FALSE;
} // DettachEngine()



////////////////////////////////////////////////////////////////////////////////////////
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
	//assert( Impact != NULL ); Removed by Icestorm. Impact&Plane CAN be NULL.
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
//	GetExtBox()
//
///////////////////////////////////////////////////////////////////////////////////////
jeBoolean JETCC GetExtBox(
	const void	*Instance,	// object instance data
	jeExtBox	*BBox )		// where to store extent box
{

	// locals
	PulsingLight	*Object;
	jeVec3d			Pos;

	// ensure valid data
	assert( Instance != NULL );
	assert( BBox != NULL );

	// get object data
	Object = (PulsingLight *)Instance;

	// save extent box
	Pos = Object->Xf.Translation;
	jeExtBox_Set (  BBox, 
					Pos.X - 5.0f, Pos.Y - 5.0f, Pos.Z - 5.0f,
					Pos.X + 5.0f, Pos.Y + 5.0f, Pos.Z + 5.0f );

	// all done
	return JE_TRUE;

} // GetExtBox()



////////////////////////////////////////////////////////////////////////////////////////
//	CreateFromFile()
//
///////////////////////////////////////////////////////////////////////////////////////
#if NEWLOAD_DLT
void * JETCC CreateFromFile(
	jeVFile		*File,		// vfile to use
	jeNameMgr *NM )	// pointer manager
{

	// locals
	PulsingLight	*Object;
	jeBoolean		Result = JE_TRUE;
	int				i,	iNum_of_patterns = PULSINGLIGHT_NO_OF_PATTERNS;

	// ensure valid data
	assert( File != NULL );

	// allocate struct
	Object = jeRam_AllocateClear( sizeof( *Object ) );
	if ( Object == NULL )
	{
		jeErrorLog_Add( JE_ERR_MEMORY_RESOURCE, NULL );
		return NULL;
	}

	// init struct
	Object->RefCount = 1;
	Object->LoadedFromDisk = JE_TRUE;

	// read xform
	Result &= jeVFile_Read( File, &( Object->Xf ), sizeof( Object->Xf ) );

	// read color
	Result &= jeVFile_Read( File, &( Object->Color.X ), sizeof( Object->Color.X ) );
	Result &= jeVFile_Read( File, &( Object->Color.Y ), sizeof( Object->Color.Y ) );
	Result &= jeVFile_Read( File, &( Object->Color.Z ), sizeof( Object->Color.Z ) );

	// read radius
	Result &= jeVFile_Read( File, &( Object->Radius ), sizeof( Object->Radius ) );

	// read brightness
	Result &= jeVFile_Read( File, &( Object->Brightness ), sizeof( Object->Brightness ) );

	// read pulsing flag
	Result &= jeVFile_Read( File, &( Object->iPulsing ), sizeof( Object->iPulsing ) );

	// read pulse speed
	Result &= jeVFile_Read( File, &( Object->iPulseSpeed ), sizeof( Object->iPulseSpeed ) );

	// read pulse pattern
	Result &= jeVFile_Read( File, &( Object->charPulsePattern), sizeof(Object->charPulsePattern) );
	Object->pcharPulsePattern = &Object->charPulsePattern;
	Object->fLastTime = 0.0f;
	Object->fRadiusSpeed = PULSINGLIGHT_DEFAULT_RADIUS_SPEED;

//	let's find out the sequential location of the current pattern
	for (i = 0; i < iNum_of_patterns; i++)
	{
		if (!strcmp(Object->pcharPulsePattern, m_ppPulsePattern[i]))
		{
			break;
		}
	}
	
	//	we use [i] to set our combobox on the NAME for the current pattern
	jeProperty_FillCombo(&( PulsingLightPropertyList.pjeProperty[PULSINGLIGHT_PULSINGCOMBO_INDEX] ),
							Util_LoadLibraryString( hClassInstance, IDS_PULSE_PATTERN ),
							m_ppPulseName[i],
							PULSINGLIGHT_PULSINGCOMBO_ID,
							iNum_of_patterns,
							m_ppPulseName );

	//	load the icon bitmap, if necessary
	if (!m_pBitmap)
	{
		PulsingLight_LoadBmp();
	}


	// fail if there was an error
	if ( Result == JE_FALSE )
	{
		jeErrorLog_Add( JE_ERR_SYSTEM_RESOURCE, NULL );
		goto ERROR_CreateFromFile;
	}

	// all done
	return Object;

	// handle errors
	ERROR_CreateFromFile:

	// free object
	jeRam_Free( Object );

	// return error
	return NULL;

	// eliminate warnings
	NM;

} // CreateFromFile()
#else
////////////////////////////////////////////////////////////////////////////////////////
//	CreateFromFile()
//
///////////////////////////////////////////////////////////////////////////////////////
void * JETCC CreateFromFile(
	jeVFile		*File,		// vfile to use
	jePtrMgr *PtrMgr )	// pointer manager
{

	// locals
	PulsingLight	*Object = NULL;
	jeBoolean		Result = JE_TRUE;
	int				i,	iNum_of_patterns = PULSINGLIGHT_NO_OF_PATTERNS;
	BYTE Version;
	uint32 Tag;
	
	// ensure valid data
	assert( File != NULL );

	// allocate struct
	Object = (PulsingLight *)jeRam_AllocateClear( sizeof( *Object ) );
	if ( Object == NULL )
	{
		jeErrorLog_Add( JE_ERR_MEMORY_RESOURCE, NULL );
		return NULL;
	}

	// init struct
	Object->RefCount = 1;
	Object->LoadedFromDisk = JE_TRUE;


	if(!jeVFile_Read(File, &Tag, sizeof(Tag)))
	{
		jeErrorLog_Add( JE_ERR_FILEIO_READ, "PulsingLight_CreateFromFile:Tag" );
		goto ERROR_CreateFromFile;
	}

	if (Tag == FILE_UNIQUE_ID)
	{
		if (!jeVFile_Read(File, &Version, sizeof(Version)))
		{
    		jeErrorLog_Add( JE_ERR_FILEIO_READ, "PulsingLight_CreateFromFile:Version" );
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

	    // read xform
	    Result &= jeVFile_Read( File, &( Object->Xf ), sizeof( Object->Xf ) );

	    // read color
	    Result &= jeVFile_Read( File, &( Object->Color.X ), sizeof( Object->Color.X ) );
	    Result &= jeVFile_Read( File, &( Object->Color.Y ), sizeof( Object->Color.Y ) );
	    Result &= jeVFile_Read( File, &( Object->Color.Z ), sizeof( Object->Color.Z ) );

	    // read radius
	    Result &= jeVFile_Read( File, &( Object->Radius ), sizeof( Object->Radius ) );

	    // read brightness
	    Result &= jeVFile_Read( File, &( Object->Brightness ), sizeof( Object->Brightness ) );

	    // read pulsing flag
	    Result &= jeVFile_Read( File, &( Object->iPulsing ), sizeof( Object->iPulsing ) );

	    // read pulse speed
	    Result &= jeVFile_Read( File, &( Object->iPulseSpeed ), sizeof( Object->iPulseSpeed ) );

	    // read pulse pattern
	    Result &= jeVFile_Read( File, &( Object->charPulsePattern), sizeof(Object->charPulsePattern) );
	}

	Object->pcharPulsePattern = Object->charPulsePattern;
	Object->fLastTime = 0.0f;
	Object->fRadiusSpeed = PULSINGLIGHT_DEFAULT_RADIUS_SPEED;

	//	let's find out the sequential location of the current pattern
	for (i = 0; i < iNum_of_patterns; i++)
	{
		if (!strcmp(Object->pcharPulsePattern, m_ppPulsePattern[i]))
		{
			break;
		}
	}
	
	//	we use [i] to set our combobox on the NAME for the current pattern
	jeProperty_FillCombo(&( PulsingLightPropertyList.pjeProperty[PULSINGLIGHT_PULSINGCOMBO_INDEX] ),
							Util_LoadLibraryString( hClassInstance, IDS_PULSE_PATTERN ),
							m_ppPulseName[i],
							PULSINGLIGHT_PULSINGCOMBO_ID,
							iNum_of_patterns,
							m_ppPulseName );

	//	load the icon bitmap, if necessary
	if (!m_pBitmap)
	{
		PulsingLight_LoadBmp();
	}

	// fail if there was an error
	if ( Result == JE_FALSE )
	{
		jeErrorLog_Add( JE_ERR_SYSTEM_RESOURCE, NULL );
		goto ERROR_CreateFromFile;
	}

	// all done
	return Object;

	// handle errors
	ERROR_CreateFromFile:

	// free object
	jeRam_Free( Object );

	// return error
	return NULL;
	PtrMgr;

} // CreateFromFile()
#endif

////////////////////////////////////////////////////////////////////////////////////////
//	WriteToFile()
//
///////////////////////////////////////////////////////////////////////////////////////
#if NEWSAVE_DLT
jeBoolean JETCC WriteToFile(
	const void	*Instance,
	jeVFile		*File,
	jeNameMgr *NM )
{

	// locals
	PulsingLight	*Object;
	jeBoolean		Result = JE_TRUE;
	BYTE Version =  PULSINLIGHT_VERSION;
	uint32 Tag = FILE_UNIQUE_ID;

	// ensure valid data
	assert( Instance != NULL );
	assert( File != NULL );

	// get object data
	Object = (PulsingLight *)Instance;

	//Write Version
	Result &= jeVFile_Write(File, &Tag, sizeof(Tag));
	Result &= jeVFile_Write(File,&Version,sizeof(Version));


	// write xform
	Result &= jeVFile_Write( File, &( Object->Xf ), sizeof( Object->Xf ) );

	// write color
	Result &= jeVFile_Write( File, &( Object->Color.X ), sizeof( Object->Color.X ) );
	Result &= jeVFile_Write( File, &( Object->Color.Y ), sizeof( Object->Color.Y ) );
	Result &= jeVFile_Write( File, &( Object->Color.Z ), sizeof( Object->Color.Z ) );

	// write radius
	Result &= jeVFile_Write( File, &( Object->Radius ), sizeof( Object->Radius ) );

	// write brightness
	Result &= jeVFile_Write( File, &( Object->Brightness ), sizeof( Object->Brightness ) );

	// write pulsing flag
	Result &= jeVFile_Write( File, &( Object->iPulsing ), sizeof( Object->iPulsing ) );

	// write pulse speed
	Result &= jeVFile_Write( File, &( Object->iPulseSpeed ), sizeof( Object->iPulseSpeed ) );

	//	write pulse pattern
	//	first zero out our char string var
	strcpy(Object->charPulsePattern, Object->pcharPulsePattern);
	Result &= jeVFile_Write( File, &( Object->charPulsePattern ), sizeof(Object->charPulsePattern ) );

	// log errors
	if ( Result != JE_TRUE )
	{
		jeErrorLog_Add( JE_ERR_SYSTEM_RESOURCE, NULL );
	}

	// all done
	return Result;

	// eliminate warnings
	NM;

} // WriteToFile()
#else
jeBoolean JETCC WriteToFile(
	const void	*Instance,
	jeVFile		*File,
	jePtrMgr *PtrMgr )
{

	// locals
	PulsingLight	*Object;
	jeBoolean		Result = JE_TRUE;

	// ensure valid data
	assert( Instance != NULL );
	assert( File != NULL );

	// get object data
	Object = (PulsingLight *)Instance;

	// write xform
	Result &= jeVFile_Write( File, &( Object->Xf ), sizeof( Object->Xf ) );

	// write color
	Result &= jeVFile_Write( File, &( Object->Color.X ), sizeof( Object->Color.X ) );
	Result &= jeVFile_Write( File, &( Object->Color.Y ), sizeof( Object->Color.Y ) );
	Result &= jeVFile_Write( File, &( Object->Color.Z ), sizeof( Object->Color.Z ) );

	// write radius
	Result &= jeVFile_Write( File, &( Object->Radius ), sizeof( Object->Radius ) );

	// write brightness
	Result &= jeVFile_Write( File, &( Object->Brightness ), sizeof( Object->Brightness ) );

	// write pulsing flag
	Result &= jeVFile_Write( File, &( Object->iPulsing ), sizeof( Object->iPulsing ) );

	// write pulse speed
	Result &= jeVFile_Write( File, &( Object->iPulseSpeed ), sizeof( Object->iPulseSpeed ) );

	//	write pulse pattern
	strcpy(Object->charPulsePattern, Object->pcharPulsePattern);
	Result &= jeVFile_Write( File, &( Object->charPulsePattern ), sizeof(Object->charPulsePattern ) );

	// log errors
	if ( Result != JE_TRUE )
	{
		jeErrorLog_Add( JE_ERR_SYSTEM_RESOURCE, NULL );
	}

	// all done
	return Result;
	PtrMgr;

	// eliminate warnings
} // WriteToFile()
#endif



////////////////////////////////////////////////////////////////////////////////////////
//	GetPropertyList()
//
///////////////////////////////////////////////////////////////////////////////////////
jeBoolean JETCC GetPropertyList(
								void			*Instance,	// object instance data
								jeProperty_List	**List)		// where to save property list pointer
{

	// locals
	PulsingLight	*pObject = NULL;
	int iNum_of_patterns = PULSINGLIGHT_NO_OF_PATTERNS;
	int i = 0;

	// ensure valid data
	assert( Instance != NULL );
	assert( List != NULL );

	// get object data
	pObject = (PulsingLight *)Instance;

	if (pObject)
	{
		// setup property list
		PulsingLightProperties[PULSINGLIGHT_RADIUS_INDEX].Data.Float = pObject->Radius;
		PulsingLightProperties[PULSINGLIGHT_BRIGHTNESS_INDEX].Data.Float = pObject->Brightness;
		PulsingLightProperties[PULSINGLIGHT_PULSING_INDEX].Data.Int = pObject->iPulsing;
		PulsingLightProperties[PULSINGLIGHT_PULSESPEED_INDEX].Data.Int = pObject->iPulseSpeed;
		PulsingLightProperties[PULSINGLIGHT_COLOR_INDEX].Data.Vector.X = pObject->Color.X;
		PulsingLightProperties[PULSINGLIGHT_COLOR_INDEX].Data.Vector.Y = pObject->Color.Y;
		PulsingLightProperties[PULSINGLIGHT_COLOR_INDEX].Data.Vector.Z = pObject->Color.Z;
		PulsingLightProperties[PULSINGLIGHT_COLORRED_INDEX].Data.Float = pObject->Color.X;
		PulsingLightProperties[PULSINGLIGHT_COLORGREEN_INDEX].Data.Float = pObject->Color.Y;
		PulsingLightProperties[PULSINGLIGHT_COLORBLUE_INDEX].Data.Float = pObject->Color.Z;
		PulsingLightProperties[PULSINGLIGHT_CASTSHADOW_INDEX].Data.Bool = pObject->CastShadow;
		PulsingLightProperties[PULSINGLIGHT_DISPLAY_ICON_INDEX].Data.Bool = pObject->bDisplayIcon;

		if (pObject->pcharPulsePattern)
		{
			//	let's find out the sequential location of the current pattern
			for (i = 0; i < iNum_of_patterns; i++)
			{
				if (!strcmp(pObject->pcharPulsePattern, m_ppPulsePattern[i]))
				{
					break;
				}
			}
		}

		//	we use [i] to set our combobox on the NAME for the current pattern
		jeProperty_FillCombo(&( PulsingLightPropertyList.pjeProperty[PULSINGLIGHT_PULSINGCOMBO_INDEX] ),
			Util_LoadLibraryString( hClassInstance, IDS_PULSE_PATTERN ),
			m_ppPulseName[i],
			PULSINGLIGHT_PULSINGCOMBO_ID,
			iNum_of_patterns,
			m_ppPulseName );

		// copy property list
		*List = jeProperty_ListCopy( &PulsingLightPropertyList );
		if ( *List == NULL )
		{
			jeErrorLog_Add( JE_ERR_INTERNAL_RESOURCE, NULL );
			return JE_FALSE;
		}

		// all done
		return JE_TRUE;
	}
	return JE_FALSE;

} // GetPropertyList()



////////////////////////////////////////////////////////////////////////////////////////
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
	PulsingLight	*pObject = NULL;
	jeBoolean		AdjustPulsingLightProperties = JE_FALSE;
	jeBoolean		bPolyAdded;
	int				i;

	// ensure valid data
	assert( Instance != NULL );
	assert( pData != NULL );

	// get object data
	pObject = (PulsingLight *)Instance;

	// process field id
	switch ( FieldID )
	{

		// adjust radius
	case PULSINGLIGHT_RADIUS_ID:
		{
			assert( DataType == PROPERTY_FLOAT_TYPE );
			pObject->Radius = pData->Float;
			AdjustPulsingLightProperties = JE_TRUE;
			break;
		}

		// adjust brightness
	case PULSINGLIGHT_BRIGHTNESS_ID:
		{
			assert( DataType == PROPERTY_FLOAT_TYPE );
			pObject->Brightness = pData->Float;
			AdjustPulsingLightProperties = JE_TRUE;
			break;
		}

	case PULSINGLIGHT_PULSING_ID:
		{
			assert( DataType == PROPERTY_CHECK_TYPE );
			pObject->iPulsing = pData->Int;
			//			if (Object->iPulsing)
			//				Object->iStatic = 0;
			AdjustPulsingLightProperties = JE_TRUE;
			break;
		}

	case PULSINGLIGHT_PULSESPEED_ID:
		{
			assert( DataType == PROPERTY_INT_TYPE );
			pObject->iPulseSpeed = pData->Int;
			AdjustPulsingLightProperties = JE_TRUE;
			break;
		}


	case PULSINGLIGHT_PULSINGCOMBO_ID:
		{
			if (!strcmp(pData->String, pStaticNoSelection))
				break;

			assert( DataType == PROPERTY_COMBO_TYPE );
			for (i=0; i < PULSINGLIGHT_NO_OF_PATTERNS; i++)
			{
				if (!strcmp(pData->String, m_ppPulseName[i]))
				{
					pObject->pcharPulsePattern = m_ppPulsePattern[i];
					break;
				}
			}
			break;
		}

		// adjust color
	case PULSINGLIGHT_COLOR_ID:
		{
			assert( DataType == PROPERTY_COLOR_PICKER_TYPE );
			pObject->Color.X = pData->Vector.X;
			pObject->Color.Y = pData->Vector.Y;
			pObject->Color.Z = pData->Vector.Z;
			AdjustPulsingLightProperties = JE_TRUE;
			break;
		}
	case PULSINGLIGHT_COLORRED_ID:
		{
			assert( DataType == PROPERTY_FLOAT_TYPE );
			pObject->Color.X = pData->Float;
			AdjustPulsingLightProperties = JE_TRUE;
			break;
		}
	case PULSINGLIGHT_COLORGREEN_ID:
		{
			assert( DataType == PROPERTY_FLOAT_TYPE );
			pObject->Color.Y = pData->Float;
			AdjustPulsingLightProperties = JE_TRUE;
			break;
		}
	case PULSINGLIGHT_COLORBLUE_ID:
		{
			assert( DataType == PROPERTY_FLOAT_TYPE );
			pObject->Color.Z = pData->Float;
			AdjustPulsingLightProperties = JE_TRUE;
			break;
		}

	// adjust cast shadow flag
	case PULSINGLIGHT_CASTSHADOW_ID:
		{
			assert( DataType == PROPERTY_CHECK_TYPE );
			pObject->CastShadow = pData->Bool;
			AdjustPulsingLightProperties = JE_TRUE;
			break;
		}

	// show  hide icon poly
	case PULSINGLIGHT_DISPLAY_ICON_ID:
		{
			assert( DataType == PROPERTY_CHECK_TYPE );
			pObject->bDisplayIcon = pData->Bool;

			if (!pObject->bDisplayIcon)
			{
				if (pObject->pPoly)
				{
					jeWorld_RemoveUserPoly(pObject->pWorld, pObject->pPoly);
				}
			}
			
			if (pObject->bDisplayIcon ) 
			{
				if ( pObject->pWorld) 
				{
					if (m_pBitmap) 
					{
						if (!pObject->pPoly) 
						{
							if (!PulsingLight_InitIcon(pObject))
							{
								break;
							}
						}
						if (pObject->pPoly) 
						{
					
						//turn on the sprite
						bPolyAdded = jeWorld_AddUserPoly( pObject->pWorld, pObject->pPoly, JE_FALSE); 
						if (!bPolyAdded) 
						{
							jeUserPoly_Destroy( &( pObject->pPoly ) );
							jeErrorLog_AddString(JE_ERR_SUBSYSTEM_FAILURE, "Unable to add the Pulsing Light UserPoly to the World.", NULL);
							//note that the bitmap may still be outstanding
						}

						PulsingLight_UpdateIcon(pObject);
						}	//	if (pObject->pPoly) ...

					}	//	if(m_pBitmap)...
				}	//	if ( pObject->pWorld) ...
			}	//	if (pObject->bDisplayIcon ) ...

			break;
		}

		// if we got to here then its an unsupported field
	default:
		{
			assert( 0 );
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
//	SetXForm()
//
///////////////////////////////////////////////////////////////////////////////////////
jeBoolean JETCC SetXForm(
						 void			*Instance,	// object instance data
						 const jeXForm3d	*Xf )		// new xform
{

	// locals
	PulsingLight	*pObject = NULL;

	// ensure valid data
	assert( Instance != NULL );
	assert( Xf != NULL );

	// get object data
	pObject = (PulsingLight *)Instance;

	if (pObject)
	{
		// save xform
		pObject->Xf = *Xf;

		// adjust light
		if (pObject->pLight)
		{
			return jeLight_SetAttributes( pObject->pLight,
				&( pObject->Xf.Translation ),
				&( pObject->Color ),
				pObject->Radius, 
				pObject->Brightness, 
				JE_LIGHT_FLAG_FAST_LIGHTING_MODEL );	//undone dont know flags
		}

		return JE_TRUE;
	}
#pragma message ("shadow flags")	

	return JE_TRUE;


} // SetXForm()



////////////////////////////////////////////////////////////////////////////////////////
//	GetXForm()
//
///////////////////////////////////////////////////////////////////////////////////////
jeBoolean JETCC GetXForm(
	const void	*Instance,	// object instance data
	jeXForm3d	*Xf )		// where to store xform
{

	// locals
	PulsingLight	*Object;

	// ensure valid data
	assert( Instance != NULL );
	assert( Xf != NULL );

	// get object data
	Object = (PulsingLight *)Instance;

	// save xform
	*Xf = Object->Xf;

	// all done
	return JE_TRUE;

} // GetXForm()



////////////////////////////////////////////////////////////////////////////////////////
//	GetXFormModFlags()
//
///////////////////////////////////////////////////////////////////////////////////////
int	JETCC GetXFormModFlags(
	const void	*Instance )	// object instance data
{

	// return xform mod flags
	return JE_OBJECT_XFORM_TRANSLATE;

	// eliminate warnings
	Instance;

} // GetXFormModFlags()



////////////////////////////////////////////////////////////////////////////////////////
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
//	EditDialog()
//
///////////////////////////////////////////////////////////////////////////////////////
jeBoolean JETCC EditDialog(
	void	*Instance,
#ifdef WIN32
	HWND	Parent )
#endif
#ifdef BUILD_BE
	class G3DView* Parent)
#endif
{
	// all done
	return JE_TRUE;

	// eliminate warnings
	Instance;
	Parent;

} // EditDialog()



////////////////////////////////////////////////////////////////////////////////////////
//	Frame()
//
///////////////////////////////////////////////////////////////////////////////////////
jeBoolean JETCC Frame(
	void	*Instance,
	float	TimeDelta )
{
	
	// ensure valid data
	assert( Instance != NULL );

	// all done
	return JE_TRUE;

	// eliminate warnings
	Instance;
	TimeDelta;

} // Frame()



////////////////////////////////////////////////////////////////////////////////////////
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

//Royce
////////////////////////////////////////////////////////////////////////////////////////
//	DuplicateInstance()
//
///////////////////////////////////////////////////////////////////////////////////////
void * JETCC DuplicateInstance(void * Instance)
{
	jeVFile *ramdisk, *ramfile;
	jeVFile_MemoryContext vfsmemctx;
	jeObject* newDLight = NULL;
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
		(jeVFile_TypeIdentifier) (JE_VFILE_TYPE_MEMORY|JE_VFILE_TYPE_VIRTUAL),
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

	newDLight = (jeObject *)CreateFromFile(ramfile, ptrMgr);
	if (!newDLight) {
		jeErrorLog_AddString(JE_ERR_SUBSYSTEM_FAILURE, "Unable to reade the object back from a temp VFile Memory File", NULL);
		jeVFile_Close(ramfile);
		jeVFile_Close(ramdisk);
		jeRam_Free(vfsmemctx.Data);
		return NULL;
	}

	jeVFile_Close(ramfile);
	jeVFile_Close(ramdisk);

	jeRam_Free(vfsmemctx.Data);

	return( newDLight );
}
//---

// Icestorm
////////////////////////////////////////////////////////////////////////////////////
//	ChangeBoxCollision
//	
////////////////////////////////////////////////////////////////////////////////////
jeBoolean	JETCC ChangeBoxCollision(const void *Instance,const jeVec3d *Pos, const jeExtBox *FrontBox, const jeExtBox *BackBox, jeExtBox *ImpactBox, jePlane *Plane)
{
	return( JE_FALSE );Plane;ImpactBox;BackBox;FrontBox;Pos;Instance;
}