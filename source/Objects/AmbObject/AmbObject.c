/****************************************************************************************/
/*  AMBOBJECT.C                                                                         */
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
#include <windows.h>

#include <string.h>
#include <float.h>
#include "AmbObject.h"
#include "jeTypes.h"
#include "jeProperty.h"
#include "jeUserPoly.h"
#include "errorlog.h"
#include "jet.h"
#include "ram.h"
#include "memory.h"
#include "assert.h"
#include "dsound.h"
#include "resource.h"
#include "jeResource.h"

#include "snd.h"

////////////////////// IMPORTANT
// If you change the structure formats, data ids, then bump the version number
//////////////////////

#define AMBOBJ_VERSION 1


#define PROP1_NAME "FileName:"
#define PROP2_NAME "Radius:"
//Royce-2
#define PROP3_NAME "Display"
//---

//	Tom
#define PROP4_NAME "Mute"

// BEGIN - Add loop checkbox to editor - paradoxnj
#define PROP5_NAME "Loop"
// END - Add loop checkbox to editor - paradoxnj

//Royce
#define OBJ_PERSIST_SIZE 5000
//---

enum 
{ 
	AMBOBJ_NAMELIST = PROPERTY_LOCAL_DATATYPE_START,
	AMBOBJ_SIZE,
	//Royce-2
	AMBOBJ_DISPLAYTOGGLE,
	//---
	//	Tom
	AMBOBJ_PROPERTY_MUTE_BOX,

	// BEGIN - Add loop checkbox to editor - paradoxnj
	AMBOBJ_PROPERTY_LOOP_BOX
	// END - Add loop checkbox to editor - paradoxnj
};

enum {
	AMB_NAMELIST_INDEX,
	AMB_SIZE_INDEX,
	//Royce-2
	AMB_DISPLAYTOGGLE_INDEX,
	//---
	AMB_INDEX_MUTE_BOX,
	// BEGIN - Add loop checkbox to editor - paradoxnj
	AMB_INDEX_LOOP_BOX,
	// END - Add loop checkbox to editor - paradoxnj

	AMB_LAST_INDEX
};

#define DEFAULT_RADIUS 1000.0f

static 	jeBitmap	*pBitmap = NULL;
static jeMaterialSpec *MatSpec;
typedef struct AmbObj {
	EffectResource  Resource;		// Resources: Camera, Engine, World, SoundSystem
	Snd				SndData;		// All info for playing the sound
	char			Name[256];
	int				RefCnt;
	jeUserPoly		*Poly;
	jeLVertex		Vertex;
	//Royce-2
	int				DisplayToggle;
	//---
	//	Tom
	jeBoolean		bMute;
	// BEGIN - Add loop checkbox to editor - paradoxnj
	jeBoolean		bLoop;
	// END - Add loop checkbox to editor - paradoxnj
} AmbObj;

jeProperty AmbProperties[AMB_LAST_INDEX];
jeProperty_List AmbPropertyList = { AMB_LAST_INDEX, &AmbProperties[0] };

#define MAX_NAMES 1024
char *NameList[MAX_NAMES];
int NameListCount = 0;

#define UTIL_MAX_RESOURCE_LENGTH	(128)
static char stringbuffer[UTIL_MAX_RESOURCE_LENGTH + 1];
static char	*NoSelection = "< none >";

//////////////////////////////////////////////////////////////////////////////
//
//  LOCAL UTILITY
//
//////////////////////////////////////////////////////////////////////////////

static jeBoolean Util_StrDupManagePtr(char **dest, char *src, int min_size)
	{
	int len;

	assert(dest);
	assert(src);

	len = strlen(src)+1;

	if (*dest)
		{
		if ( len < min_size )
			{
			strcpy(*dest, src);
			return JE_TRUE;
			}

		jeRam_Free(*dest);
		*dest = NULL;
		}

	*dest = jeRam_Allocate(__max(min_size, len));
	if (*dest == NULL)
		{
		jeErrorLog_Add( JE_ERR_MEMORY_RESOURCE, NULL );
		return JE_FALSE;
		}

	strcpy(*dest, src);
	return JE_TRUE;
	}

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

//////////////////////////////////////////////////////////////////////////////
//
//  LOCAL BITMAP RELATED
//
//////////////////////////////////////////////////////////////////////////////

static jeBoolean AmbObject_LoadBmp()
{
	// Jeff:  Ambient bitmap from resources - 8/18/2005
	
	jeVFile	* BmpFile;
	HRSRC hFRes; 
    HGLOBAL hRes; 
    jeVFile_MemoryContext Context; 
    HINSTANCE hInst;
    
	#ifdef _DEBUG 
	    hInst = LoadLibrary("AmbientObj.ddl");
	#else
        hInst = LoadLibrary("AmbientObj.dll");
    #endif
    hFRes = FindResource(hInst, MAKEINTRESOURCE(IDR_AMBIENT) ,"jeBitmap"); 
    hRes = LoadResource(hInst, hFRes) ;  
    
    Context.Data  = LockResource(hRes); 
    Context.DataLength = SizeofResource(hInst,hFRes); 

	BmpFile = jeVFile_OpenNewSystem(NULL, JE_VFILE_TYPE_MEMORY,	NULL,
		                            &Context,JE_VFILE_OPEN_READONLY  );
	if( BmpFile == NULL )
		return( JE_FALSE );
	pBitmap = jeBitmap_CreateFromFile( BmpFile );
	jeVFile_Close( BmpFile );
	if( pBitmap == NULL )
		return( JE_FALSE );
	jeBitmap_SetColorKey( pBitmap, JE_TRUE, 255, JE_TRUE );

	
	return( JE_TRUE );
	
}

static jeBoolean AmbObject_InitIcon( AmbObj * pAmbObj )
{
	assert(pAmbObj != NULL);

	//Royce-2
	assert(pBitmap);
	
	if( pBitmap == NULL )
		if( !AmbObject_LoadBmp() )
			return( JE_FALSE );
			
	//---

	pAmbObj->Vertex.r = 255.0f;
	pAmbObj->Vertex.g = 255.0f;
	pAmbObj->Vertex.b = 255.0f;
	pAmbObj->Vertex.a = 255.0f;
	pAmbObj->Vertex.u = 0.0f;
	pAmbObj->Vertex.v = 0.0f;
	pAmbObj->Vertex.sr = 255.0f;
	pAmbObj->Vertex.sg = 255.0f;
	pAmbObj->Vertex.sb = 255.0f;

	pAmbObj->Vertex.X = 0.0f;
	pAmbObj->Vertex.Y = 0.0f;
	pAmbObj->Vertex.Z = 0.0f;

	if (!MatSpec)
	{
	    MatSpec = jeMaterialSpec_Create(jeResourceMgr_GetEngine(jeResourceMgr_GetSingleton()), jeResourceMgr_GetSingleton());
#pragma message ("Krouer: change NULL to something better next time")
	    jeMaterialSpec_AddLayerFromBitmap(MatSpec, 0, pBitmap, NULL);
	}
    
	pAmbObj->Poly = jeUserPoly_CreateSprite(	&pAmbObj->Vertex,
									MatSpec,
									1.0f,
									JE_RENDER_FLAG_ALPHA | JE_RENDER_FLAG_NO_ZWRITE );

	return JE_TRUE;
}				

static jeBoolean AmbObject_UpdateIcon( AmbObj * pAmbObj )
{
	assert(pAmbObj != NULL);

	pAmbObj->Vertex.X = pAmbObj->SndData.Pos.X;
	pAmbObj->Vertex.Y = pAmbObj->SndData.Pos.Y;
	pAmbObj->Vertex.Z = pAmbObj->SndData.Pos.Z;

	//Royce-2
	if (pAmbObj->DisplayToggle)
	{
		if (!MatSpec)
	    {
	        MatSpec = jeMaterialSpec_Create(jeResourceMgr_GetEngine(jeResourceMgr_GetSingleton()), jeResourceMgr_GetSingleton());
#pragma message ("Krouer: change NULL to something better next time")
	        jeMaterialSpec_AddLayerFromBitmap(MatSpec, 0, pBitmap, NULL);
	    }
		jeUserPoly_UpdateSprite(pAmbObj->Poly, &pAmbObj->Vertex, MatSpec, 1.0f);
	}
	//---

	return JE_TRUE;
}				

//////////////////////////////////////////////////////////////////////////////
//
//  LOCAL SOUND RELATED
//
//////////////////////////////////////////////////////////////////////////////

static jeBoolean AmbObject_LoadSound(AmbObj * pAmbObj, char *Name)
{
	jeResourceMgr *ResourceMgr;
	jeVFile *SoundDir,*SndFile = NULL;
	jeSound_Def *NewSoundDef;

	if (!pAmbObj->Resource.Sound || !pAmbObj->Resource.World)
		return JE_TRUE;

	//Royce-2
	if (!Name || !Name[0] || !strcmp(NoSelection, Name))
	//---
	{
		return JE_TRUE;
	}

	// clear any old sound
	if (pAmbObj->SndData.SoundDef != NULL)
	{
		Snd_Remove(&pAmbObj->Resource, &pAmbObj->SndData);
		jeSound_FreeSoundDef(pAmbObj->Resource.Sound, pAmbObj->SndData.SoundDef);
		pAmbObj->SndData.SoundDef = NULL;
	}

	assert(pAmbObj->Resource.World);
	ResourceMgr = jeWorld_GetResourceMgr(pAmbObj->Resource.World);

	if (ResourceMgr == NULL)
		{
		jeErrorLog_AddString(JE_ERR_SUBSYSTEM_FAILURE,"AmbObject_LoadSound: jeWorld_GetResourceMgr() failed", Name);
		return JE_FALSE;
		}

	SoundDir = jeResource_GetVFile(ResourceMgr, "Sounds");

	//SndFile = jeVFile_OpenNewSystem( SoundDir, JE_VFILE_TYPE_DOS, Name, NULL, JE_VFILE_OPEN_READONLY );
	SndFile = jeVFile_Open( SoundDir, Name, JE_VFILE_OPEN_READONLY);

	if (SndFile == NULL)
	{
		jeErrorLog_AddString(JE_ERR_FILEIO_OPEN,"AmbObject_LoadSound: jeVFile_Open() failed", Name);
		goto LOAD_CLEAN;
	}

	// create the new sound def
	NewSoundDef = jeSound_LoadSoundDef( pAmbObj->Resource.Sound, SndFile );
	if (NewSoundDef == NULL)
	{
		jeErrorLog_AddString(JE_ERR_FILEIO_READ,"AmbObject_LoadSound: jeSound_LoadSoundDef() failed", Name);
		goto LOAD_CLEAN;
	}

	pAmbObj->SndData.SoundDef = NewSoundDef;
	
	jeVFile_Close( SndFile );

	// [MLB-ICE]
	jeResource_MgrDestroy(&ResourceMgr);	// Icestorm: We should clear this Instance up, it was referenced!
	// [MLB-ICE] EOB

	return( JE_TRUE );

LOAD_CLEAN:

	if (SndFile != NULL)
		jeVFile_Close( SndFile );

	strcpy(pAmbObj->Name, NoSelection);

	// [MLB-ICE]
	jeResource_MgrDestroy(&ResourceMgr);	// Icestorm: We should clear this Instance up, it was referenced!
	// [MLB-ICE] EOB

	return JE_TRUE;
}

static jeBoolean AmbObj_ReadSoundNames(jeVFile *FileBase, int *FileCount)
	{
	jeVFile_Finder * Finder;

	assert(FileBase);

	Finder = jeVFile_CreateFinder(FileBase,"*.wav");
	if ( ! Finder )
		{
		jeVFile_Close(FileBase);
		return JE_FALSE;
		}

	while( jeVFile_FinderGetNextFile(Finder) )
		{
		jeVFile_Properties Properties;
		jeVFile_FinderGetProperties(Finder,&Properties);

		strlwr(Properties.Name);

		if (Util_StrDupManagePtr(&NameList[(*FileCount)++], Properties.Name, 32) == JE_FALSE)
			{
			jeVFile_DestroyFinder(Finder);
			return JE_FALSE;
			}
		}

	jeVFile_DestroyFinder(Finder);

	return JE_TRUE;
	}

static jeBoolean JETCC AmbObj_GetSoundNames( void * Instance, jeWorld * pWorld )
{
	AmbObj *pAmbObj = (AmbObj*)Instance;
	jeResourceMgr *ResourceMgr;
	jeVFile *SoundDir;

	assert( Instance );
	assert( pWorld );

	Util_StrDupManagePtr(&NameList[0], NoSelection, 32);
	NameListCount = 1;

	ResourceMgr = jeWorld_GetResourceMgr(pWorld);

	if (ResourceMgr == NULL)
		return JE_FALSE;

	SoundDir = jeResource_GetVFile(ResourceMgr, "Sounds");

	if (!SoundDir)
		return JE_FALSE;

	if (!AmbObj_ReadSoundNames(SoundDir, &NameListCount))
		return JE_FALSE;

	// [MLB-ICE]
	jeResource_MgrDestroy(&ResourceMgr);	// Icestorm: We should clear this Instance up, it was referenced!
	// [MLB-ICE] EOB

	return( JE_TRUE );
}

//////////////////////////////////////////////////////////////////////////////
//
//  PROCESS ATTACH/DETACH
//
//////////////////////////////////////////////////////////////////////////////

void Init_Class( HINSTANCE hInstance )
{
	//Royce-2
	AmbObject_LoadBmp(); //failure is not fatal, don't bother checking
	//pBitmap is now our flag to tell us whether "ambient.bmp" is in the
	//host exe's dir. If it is new instances will default DisplayToggle ON
	//(i.e. we assume we're in the editor instead of some game)
	//---
}

void Destroy_Class( void )
{
	int i;

	for (i = 0; i < MAX_NAMES; i++)
		{
		if (NameList[i])
			{
			jeRam_Free(NameList[i]);
			NameList[i] = NULL;
			}
		}

	NameListCount = 0;
	//Royce-2
	jeBitmap_Destroy(&pBitmap);
	//---
}

//////////////////////////////////////////////////////////////////////////////
//
//  DLL INTERFACE
//
//////////////////////////////////////////////////////////////////////////////

void * JETCC CreateInstance( void )
{
	AmbObj *pAmbObj;

	pAmbObj = JE_RAM_ALLOCATE_STRUCT( AmbObj );
	if( pAmbObj == NULL )
		return( NULL );
	memset(pAmbObj, 0, sizeof(*pAmbObj));
	pAmbObj->SndData.Min = DEFAULT_RADIUS;
	//	tom morris feb 2005 -- changed default loop to TRUE
	//	set default mute to FALSE;
	pAmbObj->bLoop= /*JE_FALSE*/JE_TRUE; // 
	pAmbObj->SndData.Loop = JE_TRUE;
	pAmbObj->bMute = JE_FALSE;
	//	end tom morris feb 2005
	//Royce-2
	pAmbObj->DisplayToggle = pBitmap ? JE_TRUE : JE_FALSE;
	//---
	strcpy(pAmbObj->Name, NoSelection);
	pAmbObj->RefCnt = 1;
	//Royce-2

	return( pAmbObj );
	/*
	if( !AmbObject_InitIcon( pAmbObj ) )
		goto CI_ERROR;

	

CI_ERROR:
	jeRam_Free( pAmbObj );
	return( NULL );
	*/
	//---
}


void JETCC CreateRef(void * Instance)
{
	AmbObj *pAmbObj = (AmbObj*)Instance;

	assert( Instance );

	pAmbObj->RefCnt++;
}

jeBoolean JETCC Destroy(void **pInstance)
{
	AmbObj **hAmbObj = (AmbObj**)pInstance;
	AmbObj *pAmbObj = *hAmbObj;

	assert( pInstance );
	assert( pAmbObj->RefCnt > 0 );

	pAmbObj->RefCnt--;
	if( pAmbObj->RefCnt == 0 )
	{
		if (pAmbObj->SndData.SoundDef != NULL)
		{
			if (pAmbObj->SndData.Sound)
			{
			//	tom morris June 2005
			int iResult = 0;
			iResult = jeSound_GetStatus(pAmbObj->Resource.Sound, pAmbObj->SndData.Sound);
			if (iResult & DSBSTATUS_PLAYING)
			//	commented out by tom 
			//	if (jeSound_SoundIsPlaying(pAmbObj->Resource.Sound, pAmbObj->SndData.Sound))
			//
			jeSound_StopSound(pAmbObj->Resource.Sound, pAmbObj->SndData.Sound);

				pAmbObj->SndData.Sound = NULL;
			}

			Snd_Remove(&pAmbObj->Resource, &pAmbObj->SndData);
			jeSound_FreeSoundDef(pAmbObj->Resource.Sound, pAmbObj->SndData.SoundDef);
			pAmbObj->SndData.SoundDef = NULL;
		}

		if (pAmbObj->Poly)
			{
			jeUserPoly_Destroy(&pAmbObj->Poly);
			}

		jeRam_Free( pAmbObj );
	}

	return JE_TRUE;
}


jeBoolean JETCC Render(const void * Instance, const jeWorld * pWorld, const jeEngine *Engine, const jeCamera *Camera, const jeFrustum *CameraSpaceFrustum, jeObject_RenderFlags RenderFlags)
{
	AmbObj *pAmbObj = (AmbObj*)Instance;

	assert( Instance );
	assert( pWorld );
	assert( Engine );
	assert( Camera );

	if( jeWorld_GetRenderRecursion( pWorld ) > 1 )
		return JE_TRUE;

	if (pAmbObj->Resource.Sound != NULL && pAmbObj->SndData.SoundDef != NULL)
	{
		pAmbObj->SndData.Loop = pAmbObj->bLoop;

		pAmbObj->Resource.Camera = (jeCamera *)Camera;
		Snd_Process( &pAmbObj->Resource, 0.0f, pAmbObj->bMute, &pAmbObj->SndData );
	}
	
	return( JE_TRUE );
}

jeBoolean JETCC AttachWorld( void * Instance, jeWorld * pWorld )
{
	AmbObj *pAmbObj = (AmbObj*)Instance;

	assert( Instance );
	assert( pWorld );

	pAmbObj->Resource.World = pWorld;

	if (!AmbObj_GetSoundNames(Instance, pAmbObj->Resource.World))
		return JE_FALSE;

	jeProperty_FillCombo( &AmbPropertyList.pjeProperty[AMB_NAMELIST_INDEX], 
		PROP1_NAME, pAmbObj->Name, AMBOBJ_NAMELIST, NameListCount, NameList );

	if (pAmbObj->Name[0] && !pAmbObj->SndData.SoundDef)
		{
		AmbObject_LoadSound(pAmbObj, pAmbObj->Name);
		}

	//Royce-2
	
	// add the pAmbObj->Poly to the world
	if (pAmbObj->DisplayToggle && !pAmbObj->Poly ) {
		if (AmbObject_InitIcon(pAmbObj)) {
			if ( jeWorld_AddUserPoly( pWorld, pAmbObj->Poly, JE_FALSE ) == JE_FALSE )
			{
				jeUserPoly_Destroy( &( pAmbObj->Poly ) );
				return JE_FALSE;
			}
		}
		else return JE_FALSE;
		AmbObject_UpdateIcon(pAmbObj);
	}
	//---
	


	return( JE_TRUE );
}

jeBoolean	JETCC DettachWorld( void * Instance, jeWorld * pWorld )
{
	AmbObj *pAmbObj = (AmbObj*)Instance;

	assert( Instance );

	//Royce-2
	if (pAmbObj->Poly) {
		if (pAmbObj->DisplayToggle) 
			if ( jeWorld_RemoveUserPoly( pWorld, pAmbObj->Poly) == JE_FALSE ) 
				return JE_FALSE;
		
		
		jeUserPoly_Destroy(&(pAmbObj->Poly));
		//---
	}

	// clear any old sound
	if (pAmbObj->SndData.SoundDef != NULL)
	{
		Snd_Remove(&pAmbObj->Resource, &pAmbObj->SndData);
	}

	pAmbObj->Resource.World = NULL;

	return( JE_TRUE );
}
				
jeBoolean	JETCC AttachEngine ( void * Instance, jeEngine *Engine )
{
	AmbObj *pAmbObj = (AmbObj*)Instance;


	assert( Instance );
	assert( Engine );

	//Royce-2
	if( pBitmap )
		return( jeEngine_AddBitmap( (jeEngine*)Engine, pBitmap, JE_ENGINE_BITMAP_TYPE_3D ) );	
	//---
	return JE_TRUE;
	Instance;
}

jeBoolean	JETCC DettachEngine( void * Instance, jeEngine *Engine )
{
	assert( Instance );

	//Royce-2
	
	if( pBitmap )
		jeEngine_RemoveBitmap(	Engine, pBitmap );
	//---
	return( JE_TRUE );
	Instance;
}

jeBoolean	JETCC AttachSoundSystem( void * Instance, jeSound_System *SoundSystem )
{
	AmbObj *pAmbObj = (AmbObj*)Instance;


	assert( Instance );

	pAmbObj->Resource.Sound = SoundSystem;

	if (pAmbObj->Name[0] && !pAmbObj->SndData.SoundDef)
		{
		AmbObject_LoadSound(pAmbObj, pAmbObj->Name);
		}

	return( JE_TRUE );
}

jeBoolean	JETCC DettachSoundSystem( void * Instance, jeSound_System *SoundSystem )
{
	AmbObj *pAmbObj = (AmbObj*)Instance;

	assert( Instance );

	pAmbObj->Resource.Sound = NULL;

	return( JE_TRUE );
	SoundSystem;
}

jeBoolean	JETCC Collision(const jeObject *Object, const jeExtBox *Box, const jeVec3d *Front, const jeVec3d *Back, jeVec3d *Impact, jePlane *Plane)
{
	return( JE_FALSE );
}

jeBoolean JETCC SetMaterial(void * Instance,const jeBitmap *Bmp,const jeRGBA * Color)
{
	return( JE_TRUE );
}

jeBoolean JETCC GetMaterial(const void * Instance,jeBitmap **pBmp,jeRGBA * Color)
{
	return( JE_TRUE );
}

jeBoolean JETCC GetExtBox(const void * Instance,jeExtBox *BBox)
{
	AmbObj *pAmbObj = (AmbObj*)Instance;
	jeVec3d Point;

	assert( Instance );
	assert( BBox );

	Point = pAmbObj->SndData.Pos;

	jeExtBox_Set (  BBox, 
					Point.X-5.0f, Point.Y-5.0f, Point.Z-5.0f,
					Point.X+5.0f, Point.Y+5.0f, Point.Z+5.0f);

	return( JE_TRUE );
}


void *	JETCC CreateFromFile(jeVFile * File, jePtrMgr *PtrMgr)
{
	AmbObj * pAmbObj;
	BYTE Version;
	uint32 Tag;

	pAmbObj = JE_RAM_ALLOCATE_STRUCT( AmbObj );
	memset(pAmbObj, 0, sizeof(*pAmbObj));
	
	if( pAmbObj == NULL )
		return( NULL );

 	if(!jeVFile_Read(File, &Tag, sizeof(Tag)))
	{
		jeErrorLog_Add( JE_ERR_FILEIO_READ, "AmbObject_CreateFromFile:Tag" );
		goto CFF_ERROR;
	}

	if (Tag == FILE_UNIQUE_ID)
	{
		if (!jeVFile_Read(File, &Version, sizeof(Version)))
		{
    		jeErrorLog_Add( JE_ERR_FILEIO_READ, "AmbObject_CreateFromFile:Version" );
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
	
	    if( !jeVFile_Read(	File, pAmbObj->Name, sizeof( pAmbObj->Name) ) )
		{
    	    jeErrorLog_Add(JE_ERR_FILEIO_READ, "AmbObject_CreateFromFile:Name");
		    goto CFF_ERROR;
		}

	    if( !jeVFile_Read(	File, &pAmbObj->SndData.Pos, sizeof( pAmbObj->SndData.Pos) ) )
		{
    	    jeErrorLog_Add(JE_ERR_FILEIO_READ, "AmbObject_CreateFromFile:SndData.Pos");
		    goto CFF_ERROR;
		}
	
	    if( !jeVFile_Read(	File, &pAmbObj->SndData.Min, sizeof( pAmbObj->SndData.Min) ) )
		{
    	    jeErrorLog_Add(JE_ERR_FILEIO_READ, "AmbObject_CreateFromFile:SndData.Min");
		    goto CFF_ERROR;
		}

	    if( !jeVFile_Read(	File, &pAmbObj->SndData.Loop, sizeof( pAmbObj->SndData.Loop) ) )
		{
    	    jeErrorLog_Add(JE_ERR_FILEIO_READ, "AmbObject_CreateFromFile:SndData.Loop");
		    goto CFF_ERROR;
		}
	}
	

	//Royce-2
	//this property is detected based on the existance of ambient.bmp
	pAmbObj->DisplayToggle = pBitmap ? JE_TRUE : JE_FALSE; //not a fatal error
	//---
	pAmbObj->RefCnt = 1;

	//	tom morris	feb 2005 -- setting loop checkbox to match saved state
	pAmbObj->bLoop = pAmbObj->SndData.Loop;
	//	end tom morris feb 2005

	AmbObject_LoadSound(pAmbObj, pAmbObj->Name);

	return( pAmbObj );

CFF_ERROR:

	jeRam_Free( pAmbObj );
	return( NULL );
	PtrMgr;
}



jeBoolean	JETCC WriteToFile(const void * Instance,jeVFile * File, jePtrMgr *PtrMgr)
{
	AmbObj *pAmbObj = (AmbObj*)Instance;
	BYTE Version = AMBOBJ_VERSION;
	uint32 Tag = FILE_UNIQUE_ID;

	assert( Instance );


	if( !jeVFile_Write(	File, &Tag, sizeof(Tag)))
	{
    	jeErrorLog_Add(JE_ERR_FILEIO_WRITE, "AmbObject_WriteToFile:Tag");
	    return( JE_FALSE );
	}
	
	if( !jeVFile_Write(	File, &Version, sizeof(Version) ) )
	{
    	jeErrorLog_Add(JE_ERR_FILEIO_WRITE, "AmbObject_WriteToFile:Version");
	    return( JE_FALSE );
	}

	if( !jeVFile_Write(	File, pAmbObj->Name, sizeof( pAmbObj->Name) ) )
	{
    	jeErrorLog_Add(JE_ERR_FILEIO_WRITE, "AmbObject_WriteToFile:Name");
	    return( JE_FALSE );
	}

	if( !jeVFile_Write(	File, &pAmbObj->SndData.Pos, sizeof( pAmbObj->SndData.Pos) ) )
	{
    	jeErrorLog_Add(JE_ERR_FILEIO_WRITE, "AmbObject_WriteToFile:SndData.Pos");
		return( JE_FALSE );
	}

	if( !jeVFile_Write(	File, &pAmbObj->SndData.Min, sizeof( pAmbObj->SndData.Min) ) )
	{
    	jeErrorLog_Add(JE_ERR_FILEIO_WRITE, "AmbObject_WriteToFile:SndData.Min");
		return( JE_FALSE );
	}

	if( !jeVFile_Write(	File, &pAmbObj->SndData.Loop, sizeof( pAmbObj->SndData.Loop) ) )
	{
    	jeErrorLog_Add(JE_ERR_FILEIO_WRITE, "AmbObject_WriteToFile:SndData.Loop");
		return( JE_FALSE );
	}

	return( JE_TRUE );
	PtrMgr;
}


jeBoolean	JETCC GetPropertyList(void * Instance, jeProperty_List **List)
{
	AmbObj *pAmbObj = (AmbObj*)Instance;

	assert( Instance );

	//assert(pAmbObj->Resource.World);
	if (pAmbObj->Resource.World)
		{
		if (!AmbObj_GetSoundNames(Instance, pAmbObj->Resource.World))
			return JE_FALSE;

		jeProperty_FillCombo( &AmbPropertyList.pjeProperty[AMB_NAMELIST_INDEX], 
			PROP1_NAME, pAmbObj->Name, AMBOBJ_NAMELIST, NameListCount, NameList );
		}
	else
		{
		jeProperty_FillCombo( &AmbPropertyList.pjeProperty[AMB_NAMELIST_INDEX], 
			PROP1_NAME, NoSelection, AMBOBJ_NAMELIST, 1, &NoSelection );
		}


	jeProperty_FillFloat( &AmbPropertyList.pjeProperty[AMB_SIZE_INDEX], 
		PROP2_NAME, pAmbObj->SndData.Min, AMBOBJ_SIZE, 0, FLT_MAX, 10.0f );

	//Royce-2
	jeProperty_FillCheck( &AmbPropertyList.pjeProperty[AMB_DISPLAYTOGGLE_INDEX],
		PROP3_NAME, pAmbObj->DisplayToggle, AMBOBJ_DISPLAYTOGGLE);
	//---

	//	Tom
	jeProperty_FillCheck( &AmbPropertyList.pjeProperty[AMB_INDEX_MUTE_BOX],
		PROP4_NAME, pAmbObj->bMute, AMBOBJ_PROPERTY_MUTE_BOX);

	// BEGIN - Add loop checkbox to editor - paradoxnj
	jeProperty_FillCheck( &AmbPropertyList.pjeProperty[AMB_INDEX_LOOP_BOX],
		PROP5_NAME, pAmbObj->bLoop, AMBOBJ_PROPERTY_LOOP_BOX);

	*List = jeProperty_ListCopy( &AmbPropertyList);
	AmbPropertyList.bDirty = JE_FALSE;

	if( *List == NULL )
		return( JE_FALSE );

	return( JE_TRUE );
}


jeBoolean	JETCC SetProperty( void * Instance, int32 FieldID, PROPERTY_FIELD_TYPE DataType, jeProperty_Data * pData )
{
	AmbObj *pAmbObj = (AmbObj*)Instance;

	assert( Instance );
	assert( pData );

	//Royce-2
	
	switch (FieldID) 
	{ 
	case AMBOBJ_SIZE :
	
		if (pData->Float < 0.0f)
			pData->Float = 0.0f;

		pAmbObj->SndData.Min = pData->Float;
		break;
	case AMBOBJ_NAMELIST:
		
		if (strcmp(pData->String, NoSelection) == 0)
			return JE_TRUE;

		strcpy(pAmbObj->Name, pData->String);
		AmbObject_LoadSound(pAmbObj, pData->String);
		AmbPropertyList.bDirty = JE_TRUE;
		break;

	//	Tom
	case AMBOBJ_PROPERTY_MUTE_BOX:
	{
		pAmbObj->bMute = (jeBoolean)pData->Bool;
		break;
	}

	case AMBOBJ_PROPERTY_LOOP_BOX:
		{
			pAmbObj->bLoop = (jeBoolean)pData->Bool;
			break;
		}
	case AMBOBJ_DISPLAYTOGGLE:
		pAmbObj->DisplayToggle = pData->Bool;
		if (pAmbObj->DisplayToggle ) 
		{
			if ( pAmbObj->Resource.World) 
			{
				if (pBitmap) {
					if (!pAmbObj->Poly) 
						AmbObject_InitIcon(pAmbObj);

					//turn on the sprite
					if ( jeWorld_AddUserPoly( pAmbObj->Resource.World, pAmbObj->Poly, JE_FALSE ) == JE_FALSE ) {
						//if this busts we should probably hear about it
						//but it is not fatal
						jeUserPoly_Destroy( &( pAmbObj->Poly ) );
						jeErrorLog_AddString(JE_ERR_SUBSYSTEM_FAILURE, "Unable to add the Ambient UserPoly to the World.", NULL);
						//note that the bitmap may still be outstanding
					}
					AmbObject_UpdateIcon(pAmbObj);
				}
			}
			
		}
		else {
			//turn off the sprite
			if (pAmbObj->Poly) {
				if ( jeWorld_RemoveUserPoly( pAmbObj->Resource.World, pAmbObj->Poly) == JE_FALSE ) {
					//something bad has probably gone wrong here
					//but I still don't think it should be fatal
					jeErrorLog_AddString(JE_ERR_SUBSYSTEM_FAILURE, "Unable to remove the Ambient UserPoly from the World.", NULL);
					break; //don't orphan the poly 
				}
			}
			
			
		}
		break;
	}
	//---

	


	return( JE_TRUE );
}

jeBoolean	JETCC SetXForm(void * Instance,const jeXForm3d *XF)
{
	AmbObj *pAmbObj = (AmbObj*)Instance;

	assert( Instance );
	assert( XF );

	pAmbObj->SndData.Pos = XF->Translation;
	AmbObject_UpdateIcon(pAmbObj);

	return( JE_TRUE );
}

jeBoolean JETCC GetXForm(const void * Instance,jeXForm3d *XF)
{
	AmbObj *pAmbObj = (AmbObj*)Instance;

	assert( Instance );
	assert( XF );

	jeXForm3d_SetIdentity(XF);
	XF->Translation = pAmbObj->SndData.Pos;
	return( JE_TRUE );
}

int	JETCC GetXFormModFlags( const void * Instance )
{
	Instance;
	return( JE_OBJECT_XFORM_TRANSLATE);
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

jeBoolean JETCC EditDialog (void * Instance,HWND Parent)
{
	return( JE_TRUE );
}


jeBoolean JETCC MessageFunction (void * Instance, int32 Msg, void * Data)
{
	AmbObj *pAmbObj = (AmbObj*)Instance;

	assert( Instance );

	switch (Msg)
		{
		default:
			return JE_FALSE;
			break;
		}// switch

	return( JE_TRUE );
}


jeBoolean	JETCC UpdateTimeDelta(void * Instance, float TimeDelta )
{
	// locals
	//AmbObj	*pAmbObj;

	// ensure valid data
	assert( Instance != NULL );

	if ( TimeDelta == 0.0f )
	{
		return JE_TRUE;
	}

	return JE_TRUE;
}


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
	jeObject* newAmbObj = NULL;
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
		JE_VFILE_TYPE_MEMORY|JE_VFILE_TYPE_VIRTUAL,
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

	newAmbObj = CreateFromFile(ramfile, ptrMgr);
	if (!newAmbObj) {
		jeErrorLog_AddString(JE_ERR_SUBSYSTEM_FAILURE, "Unable to reade the object back from a temp VFile Memory File", NULL);
		jeVFile_Close(ramfile);
		jeVFile_Close(ramdisk);
		jeRam_Free(vfsmemctx.Data);
		return NULL;
	}

	jeVFile_Close(ramfile);
	jeVFile_Close(ramdisk);

	jeRam_Free(vfsmemctx.Data);

	return( newAmbObj );
}
//---

// Icestorm
jeBoolean	JETCC ChangeBoxCollision(const void *Instance,const jeVec3d *Pos, const jeExtBox *FrontBox, const jeExtBox *BackBox, jeExtBox *ImpactBox, jePlane *Plane)
{
	return( JE_FALSE );
}