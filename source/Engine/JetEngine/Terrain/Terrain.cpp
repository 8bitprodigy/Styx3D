/****************************************************************************************/
/*  TERRAIN.C                                                                           */
/*                                                                                      */
/*  Author:  Charles Bloom                                                              */
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
/**********---------------------

(later) todos :

	1. good file IO
	2. vis
		with brushes for in-terrain vis !

@@ for urgent
<> for todos
{} for notes/peformance concerns

---------------------

todo :

EDITOR STUFF :

<> jeLight UI in editor

<> set heightmap from materials in editor

<> vec3d Size in property list isn't editable !?
	using separate floats is working

<> need a deselect message 

@@ take materials instead of bitmaps for textures;
	write out materials via the NameMgr
	support the UV mapper or whatever is in the material

CODE STUFF :

<> my dynamic lights are different than the world variety; must think
	-> do lighting in light maps?
	the original reasons why not to: 
		1. fill rate; hurts huge on voodoo 1's and whatnot; not an issue in the future
		2. memory usage; we can easily have 16 256x256 textures (2 megs) of terrain textures
			if we have per-pixel lightmaps, that doubles
		3. textures mip, lightmaps don't ; this is a big issue
	if we have mipping jeBitmap multi-texturing, then we could do one lightmap pel per 4 texels (or so)

<> make a jeWorld_CalculateLighting function

@@ take a two-sided flag

<> Collision is vs. "thick rays" (extruded spheres) not extruded extboxes right now
	should be easy to fix up right

<> file IO options :
	1. save lit textures
	2. save quadtree

<> Icestorm: Port the BoxCollision to ChangeBoxCollision and set it in the ObjectDef
				(currently set to NULL!!) Don't forget: Plane&Impact CAN be NULL!

---------------------***********/

////////////////////////////////////////////////////////////////////////////////
// 
////////// Jet3D Note: Important !!  ///////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////
// CJP(chrisjp@eudoramail.com) : 2.18.00
//
// The loading / saving code has been modified to save the heightmap and bitmaps directly to the file, not through the embedded VFS
// This change will break all levels which currently contain one or more terrain objects, but 
//		will allow multiple terrain objects to be saved
// To allow loading of old terrain objects, #define CJP_LOAD_OLDTERRAINOBJECTS
// To allow saving of old terrain objects ( can't think of a reason why..) #define CJP_SAVE_OLDTERRAINOBJECTS
//
// #define CJP_LOAD_OLDTERRAINOBJECTS 1
// #define CJP_SAVE_OLDTERRAINOBJECTS 1

// End of note.

#include "Object.h"
#include "Terrain.h"
#include "Terrain._h"
#include "Engine.h" 
#include "jeFrustum.h" 
#include "Ram.h"
#include "jeProperty.h"
#include "Util.h"
#include "jeChain.h"
#include "jeWorld.h"
#include "Errorlog.h"
#include "Camera._h"

#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h> // for sprintf

#ifndef max
#define min(a,b) (((a)<(b))?(a):(b))
#define max(a,b) (((a)>(b))?(a):(b))
#endif

#ifdef BUILD_BE
#define stricmp strcasecmp
#define strnicmp strncasecmp
#endif

/*}{******************************************************/
//	Protos & Macros

#define allocate(ptr)	ptr = jeRam_AllocateClear(sizeof(*ptr))
#define clear(ptr)		memset(ptr,0,sizeof(*ptr))
#define destroy(ptr)	if ( ptr ) { jeRam_Free(ptr); (ptr) = NULL; } else
#define max3(a,b,c) max(max(a,b),c)
#define min3(a,b,c) min(min(a,b),c)

static jeBoolean jeTerrain_IsValid(const jeTerrain *Terrain);
static jeBoolean JETCC jeTerrain_AttachEngine(void *T,jeEngine *Engine);
static jeBoolean JETCC jeTerrain_DetachEngine(void *T,jeEngine *Engine);
static jeBoolean jeTerrain_RefreshQT(jeTerrain * T);

static void jeTerrain_DeSelect(jeTerrain * T);
static void jeTerrain_Select(jeTerrain * T,jeVec3d *pWorldVec);

/*}{******************************************************/
//	Creators, Destroys, File IO

static void FillPalBmp(jeBitmap *Bmp,int r,int g,int b)
{
jeBitmap * Lock;
jeBitmap_Palette * Pal;
int y,s,w,h;
jeBitmap_Info Info;
uint8 * bptr;

	Pal = jeBitmap_Palette_Create(JE_PIXELFORMAT_24BIT_RGB,256);
	assert(Pal);
	jeBitmap_Palette_SetEntryColor(Pal,0,r,g,b,255);
	jeBitmap_SetPalette(Bmp,Pal);
	jeBitmap_Palette_Destroy(&Pal);

	jeBitmap_LockForWriteFormat(Bmp,&Lock,0,0,JE_PIXELFORMAT_8BIT_PAL);

	jeBitmap_GetInfo(Lock,&Info,NULL);

	s = Info.Stride;
	w = Info.Width;
	h = Info.Height;
	
	bptr = (uint8 *)jeBitmap_GetBits(Lock);
	assert(bptr);

	for(y=0;y<h;y++)
	{
		memset(bptr,0,w);
		bptr += s;
	}

	jeBitmap_UnLock(Lock);
}

JETAPI void * JETCC jeTerrain_Create(void)
{
jeTerrain * T = NULL;
jeBoolean ret;

	T = (jeTerrain *)jeRam_AllocateClear(sizeof(jeTerrain));
	if ( ! T )
		return NULL;

	T->Untouchable = JE_TRUE;

	T->SelfCheck = T;

	T->RefCount = 1;

	// Krouer: enable rendering
	T->RenderNextFlag = JE_TRUE;

	T->MaxQuads = 500;
	T->MinError = 0.015f;

	jeXForm3d_SetIdentity(&(T->XFTerrainToWorld));
	T->XFWorldToTerrain = T->XFTerrainToWorld;
		
	T->LastTesselatedCameraPos.X = 9999999999.9f;
	T->Changed = JE_FALSE;

	T->Size.X = T->Size.Y = T->Size.Z = 100.0f;

	T->NullTexture = jeBitmap_Create(8,8,1,JE_PIXELFORMAT_8BIT_PAL);
	FillPalBmp(T->NullTexture,0,255,0);

	T->HiliteTexture = jeBitmap_Create(8,8,1,JE_PIXELFORMAT_8BIT_PAL);
	FillPalBmp(T->HiliteTexture,0,0,255);

	T->TexDim = 1;
	ret = jeTerrain_SetATexture(T,T->NullTexture,0,0);
	assert(ret);

	{
	jeBitmap * Bmp = NULL;

		Bmp = jeBitmap_Create(8,8,1,JE_PIXELFORMAT_8BIT_PAL);
		if (Bmp)
		{
			FillPalBmp(Bmp,0,0,0);

			ret = jeTerrain_SetHeightmap(T,Bmp);
			assert(ret);

			jeBitmap_Destroy(&Bmp);
		}

		strcpy(T->HeightmapName,"null");
	}

	T->Untouchable = JE_FALSE;

return T;
}

JETAPI jeBoolean JETCC  jeTerrain_SetSize(jeTerrain * T,jeVec3d * pSize)
{
	assert( jeTerrain_IsValid(T) );
	assert( pSize );

	jeTerrain_DeSelect(T);

	if ( T->HM )
	{
	jeVec3d Scale;
		
		T->Untouchable = JE_TRUE;

		Scale.X = pSize->X / T->Size.X;
		Scale.Y = pSize->Y / T->Size.Y;
		Scale.Z = pSize->Z / T->Size.Z;

		if ( Scale.Z != 1.0f )
		{
		float * HMptr;
		int cnt;
			cnt = T->HMWidth * T->HMHeight;
			HMptr = T->HM;
			while(cnt--)
			{
				*HMptr++ *= Scale.Z;
			}
		}

		T->Size = *pSize;

		T->InvCubeSize.X = (float)(T->HMWidth  - 1) / T->Size.X;
		T->InvCubeSize.Y = (float)(T->HMHeight - 1) / T->Size.Y;
		T->InvCubeSize.Z = 255.0f   / T->Size.Z;

		T->CubeSize.X = 1.0f / T->InvCubeSize.X;
		T->CubeSize.Y = 1.0f / T->InvCubeSize.Y;
		T->CubeSize.Z = 1.0f / T->InvCubeSize.Z;

		if ( T->QT )
		{
			/* <> Madre de dios, this is slow, but it's actually quite tricky to scale,
			*	because you have things like Sin2Normal and ErrIsotropic and whatnot
			*	which are affected in some funny way by the scaling of coordinate systems
			*
			*	<> notez : we could do an isotropic scaling much more easily
			**/
			QuadTree_Destroy(&(T->QT));
			T->QT = QuadTree_Create(T);

			if ( ! T->QT )
				return JE_FALSE;

			if ( ! jeTerrain_RefreshQT(T) )
				return JE_FALSE;
		}
		
		T->Untouchable = JE_FALSE;
	}
	else
	{
		T->Size = *pSize;
	}

return JE_TRUE;
}

JETAPI jeBoolean	JETCC jeTerrain_GetHeightmap(jeTerrain * T,jeBitmap ** pBmp)
{
	assert( jeTerrain_IsValid(T) );
	*pBmp = T->Heightmap;
return JE_TRUE;
}

#define ispow2(X) ( ( (X) & ~(-(X)) ) == 0 )

JETAPI jeBoolean JETCC  jeTerrain_SetHeightmap(jeTerrain * T,jeBitmap * Bmp)
{
jeBitmap * Lock = NULL;
jeBitmap_Info Info;
int w,h;

	assert( jeTerrain_IsValid(T) );
	jeTerrain_DeSelect(T);

	if ( Bmp == T->Heightmap ) // do nothing {} Bmp could've changed
	{
		return JE_TRUE;
	}
	else
	{
	jeVFile *F1,*F2;
	char *N1,*N2;
		if ( jeBitmap_GetPersistableName(Bmp,&F1,&N1) &&
	 		jeBitmap_GetPersistableName(T->Heightmap,&F2,&N2) )
		{
			// both bitmaps persist as the same thing!
			if ( F1 == F2 && strcmp(N1,N2) == 0 )
			{
				return JE_TRUE;
			}
		}
	}

	T->Untouchable = JE_TRUE;

	if ( ! jeBitmap_GetInfo(Bmp,&Info,NULL) )
		goto fail;

	w = Info.Width;
	h = Info.Height;

	if ( ! ispow2(w) )
	{
		jeErrorLog_AddString(-1,"SetHeightmap : width not a power of 2", NULL);
		return JE_FALSE;
	}
	if ( ! ispow2(h) )
	{
		jeErrorLog_AddString(-1,"SetHeightmap : height not a power of 2", NULL);
		return JE_FALSE;
	}		
	if ( w < 8 || h < 8 )
	{
		jeErrorLog_AddString(-1,"SetHeightmap : width & height must be >= 8", NULL);
		return JE_FALSE;
	}	

	if ( T->QT )
		QuadTree_Destroy(&(T->QT));

	if ( T->HM )
		jeRam_Free(T->HM);

	if ( T->Heightmap )
		jeBitmap_Destroy(&(T->Heightmap));

	T->Heightmap = Bmp;
	jeBitmap_CreateRef(Bmp);

	T->Changed = JE_TRUE;

	{
	int x,y,w,h,s;
	uint8 * bptr;
	float * HMptr,ScaleZ;

		if ( ! jeBitmap_LockForRead(Bmp,&Lock,0,0,JE_PIXELFORMAT_8BIT_GRAY,JE_FALSE,0) )
			goto fail;

		if ( ! jeBitmap_GetInfo(Lock,&Info,NULL) )
			goto fail;

		w = Info.Width;
		h = Info.Height;
		s = Info.Stride;
		T->HMWidth  = w + 1;		
		T->HMHeight = h + 1;
			
		T->InvCubeSize.X = (float)(T->HMWidth  - 1) / T->Size.X;
		T->InvCubeSize.Y = (float)(T->HMHeight - 1) / T->Size.Y;
		T->InvCubeSize.Z = 255.0f   / T->Size.Z;

		T->CubeSize.X = 1.0f / T->InvCubeSize.X;
		T->CubeSize.Y = 1.0f / T->InvCubeSize.Y;
		T->CubeSize.Z = 1.0f / T->InvCubeSize.Z;

		if ( (T->HM = (float *)jeRam_Allocate(T->HMWidth*T->HMHeight*sizeof(float))) == NULL )
			goto fail;
		
		bptr = (uint8 *)jeBitmap_GetBits(Lock);
		assert(bptr);
		HMptr = T->HM;
		ScaleZ = T->CubeSize.Z;

		for(y=h;y--;)
		{
			for(x=w;x--;)
			{
			float f;
				f = *bptr++;
				*HMptr++ = f * ScaleZ;
			}
			*HMptr++ = HMptr[-1];
			bptr += s-w;
		}
		for(x=w+1;x--;)
		{
			*HMptr++ = HMptr[- T->HMWidth];
		}

		jeBitmap_UnLock(Lock); Lock = NULL;
	}

	T->QT = QuadTree_Create(T);
	if ( ! T->QT )
		goto fail;

	if ( ! jeTerrain_RefreshQT(T) )
		goto fail;

	{
	jeVFile *BmpF;
	char *BmpN;
		if ( jeBitmap_GetPersistableName(Bmp,&BmpF,&BmpN) )
		{
			strcpy(T->HeightmapName,BmpN);
		}
		else
		{
			strcpy(T->HeightmapName,"unknown");
		}
	}

	T->Untouchable = JE_FALSE;

return JE_TRUE;

	fail:

	if ( Lock )
		jeBitmap_UnLock(Lock);

return JE_FALSE;
}

static jeBoolean jeTerrain_RefreshQT(jeTerrain * T)
{
	if ( ! T->QT )
		return JE_FALSE;
	jeTerrain_SetParameters(T,T->MaxQuads,T->MinError);
	QuadTree_SetTexDim(T->QT,T->TexDim);
return JE_TRUE;
}

JETAPI jeBoolean JETCC jeTerrain_Destroy(void ** pT)
{
jeTerrain * T;
int i;

	assert(pT);
	T = (jeTerrain *)*pT;
	if ( ! T )
		return( JE_TRUE );

	T->RefCount --;
	if ( T->RefCount > 0 )
		return( JE_FALSE );

	if ( T->Engine )
		jeTerrain_DetachEngine(T,T->Engine);

	if ( T->NullTexture )
		jeBitmap_Destroy(&(T->NullTexture));
	if ( T->HiliteTexture )
		jeBitmap_Destroy(&(T->HiliteTexture));

	for(i=0;i<MAX_TEXTURES;i++)
	{
	jeBitmap * Bmp;

		if ( T->Textures[i] && T->RegisteredTexture[i] && T->Engine )
			jeEngine_RemoveBitmap(T->Engine,T->Textures[i]);

		Bmp = T->Textures[i];
		if ( Bmp )
			jeBitmap_Destroy(&Bmp);
		Bmp = T->PreLightTextures[i];
		if ( Bmp )
			jeBitmap_Destroy(&Bmp);
	}

	if ( T->QT )
	{
		QuadTree_ShowStats(T->QT);
		QuadTree_Destroy(&(T->QT));
	}

	if ( T->HM )
		jeRam_Free(T->HM);

	if ( T->Heightmap )
		jeBitmap_Destroy(&(T->Heightmap));

	if ( T->PropertyList )
		jeProperty_ListDestroy(&(T->PropertyList));

	destroy(T);

	*pT = NULL;
	return( JE_TRUE );
}

JETAPI void JETCC jeTerrain_CreateRef(void * T)
{
	assert( jeTerrain_IsValid((jeTerrain*)T) );
	((jeTerrain*)T)->RefCount ++;
	return;
}

/*}{****************** File IO ************************************/

/*****

<>

this CreateFromFile is awfully slow
it'd be better to write the quadtree & recreate it;
of course then we have to recreate the heightmap plane from the QT
 which is a hassle

*****/

#define jeVFile_ReadEntity(VF,ptr)	jeVFile_Read( VF,(ptr),sizeof(*(ptr)))
#define jeVFile_WriteEntity(VF,ptr)	jeVFile_Write(VF,(ptr),sizeof(*(ptr)))

#define DISABLE_PTRMGR

static const uint32 jeTerrain_Tag = 0x6E725447; // GTrn

JETAPI jeBoolean JETCC jeTerrain_WriteToFile(const void *Terrain, jeVFile * File, jePtrMgr *PtrMgr)
{
	jeTerrain* T = (jeTerrain *)Terrain;
#if 0 //{

	{
	jeXForm3d XF;
	jeVec3d Size;
	uint32 MaxQuads,TexDim;
	float MinError;

		jeTerrain_GetXForm(T,&XF);
		Size = T->Size;
		MaxQuads = T->MaxQuads;
		MinError = T->MinError;
		TexDim = T->TexDim;
		
		jeVFile_WriteEntity(File, &XF );
		jeVFile_WriteEntity(File, &Size );
		jeVFile_WriteEntity(File, &MaxQuads );
		jeVFile_WriteEntity(File, &MinError );
		jeVFile_WriteEntity(File, &TexDim );
	}

return JE_TRUE;

#else //}{

// Added by cjp
#ifdef CJP_SAVE_OLDTERRAINOBJECTS

jeVFile *VFS,*SubFile;
jeBoolean suc;
int texN;

	assert( jeTerrain_IsValid(T) );

	#ifdef DISABLE_PTRMGR
	PtrMgr = NULL;
	#endif

	VFS = jeVFile_OpenNewSystem( File, JE_VFILE_TYPE_VIRTUAL, NULL, NULL, 
									JE_VFILE_OPEN_CREATE | JE_VFILE_OPEN_DIRECTORY );
	if ( ! VFS )
		return JE_FALSE;

	SubFile = jeVFile_Open( VFS, "Terrain_Info", JE_VFILE_OPEN_CREATE );
	if ( ! SubFile )
	{
		jeVFile_Close(VFS);
		return JE_FALSE;
	}

	{
	jeXForm3d XF;
	jeVec3d Size;
	uint32 MaxQuads,TexDim;
	float MinError;

		jeTerrain_GetXForm(T,&XF);
		Size = T->Size;
		MaxQuads = T->MaxQuads;
		MinError = T->MinError;
		TexDim = T->TexDim;
		
		jeVFile_WriteEntity(SubFile, &jeTerrain_Tag );
		jeVFile_WriteEntity(SubFile, &XF );
		jeVFile_WriteEntity(SubFile, &Size );
		jeVFile_WriteEntity(SubFile, &MaxQuads );
		jeVFile_WriteEntity(SubFile, &MinError );
		jeVFile_WriteEntity(SubFile, &TexDim );
	}

	suc = jeVFile_Close(SubFile); SubFile = NULL;
	assert(suc);

//	if ( ! jeBitmap_WriteToFileName2(T->Heightmap, VFS, "Terrain_Heightmap", PtrMgr) ) 
	if ( ! jeBitmap_WriteToFileName(T->Heightmap, VFS, "Terrain_Heightmap") ) 
	{
		jeVFile_Close(SubFile);
		jeVFile_Close(VFS);
		return JE_FALSE;
	}

	for(texN=0;texN<(T->TexDim * T->TexDim);texN++)
	{
	char Name[1024];
	jeBitmap * Bmp;

		sprintf(Name,"Terrain_Texture%d",texN);

		if ( T->PreLightTextures[texN] ) 
			Bmp = T->PreLightTextures[texN];
		else
			Bmp = T->Textures[texN];

	//	if ( ! jeBitmap_WriteToFileName2(Bmp,VFS,Name,PtrMgr) )
		if ( ! jeBitmap_WriteToFileName(Bmp,VFS,Name) )
		{
			jeVFile_Close(VFS);
			return JE_FALSE;
		}
	}

	suc = jeVFile_Close(VFS);
	assert(suc);

return JE_TRUE;

#else

	// The new code to save terrain objects..
	int texN;
	uint8 Version = 1;
	uint32 Tag = FILE_UNIQUE_ID;

	assert( jeTerrain_IsValid(T) );

	{
		jeXForm3d XF;
		jeVec3d Size;
		uint32 MaxQuads,TexDim;
		float MinError;

		jeTerrain_GetXForm(T,&XF);
		Size = T->Size;
		MaxQuads = T->MaxQuads;
		MinError = T->MinError;
		TexDim = T->TexDim;
		
		jeVFile_WriteEntity(File, &jeTerrain_Tag );
		jeVFile_WriteEntity(File, &XF );
		jeVFile_WriteEntity(File, &Size );
		jeVFile_WriteEntity(File, &MaxQuads );
		jeVFile_WriteEntity(File, &MinError );
		jeVFile_WriteEntity(File, &TexDim );
	}

//	if ( ! jeBitmap_WriteToFileName2(T->Heightmap, VFS, "Terrain_Heightmap", PtrMgr) ) 
	if ( ! jeBitmap_WriteToFile(T->Heightmap, File) )
	{
		return JE_FALSE;
	}

	jeVFile_Write(File, &Tag, sizeof(uint32));
	jeVFile_Write(File, &Version, sizeof(uint8));

	for(texN=0;texN<(T->TexDim * T->TexDim);texN++)
	{
		char* BmpName;
		jeVFile* fs;
		char Name[256];
		jeBitmap * Bmp;

		if ( T->PreLightTextures[texN] ) 
			Bmp = T->PreLightTextures[texN];
		else
			Bmp = T->Textures[texN];

		jeBitmap_GetPersistableName(Bmp, &fs, &BmpName);
		if (BmpName == NULL) {
			strcpy(Name, "Jet3D");
		} else {
			char* Global = strstr(BmpName, "GlobalMaterials");
			if (Global == NULL) {
				strcpy(Name, BmpName);
			} else {
				sprintf(Name,"%s",Global+strlen("GlobalMaterials")+1);
			}
			BmpName = strrchr(Name, '.');
			BmpName[0] = 0;
		}

		jeVFile_Write(File, Name, 256);
	}

return JE_TRUE;


#endif // End of new save code. (CJP)

#endif // }

}

JETAPI jeTerrain * JETCC jeTerrain_CreateFromFileExt(jeVFile * File,jeVFile *ResourceBaseFS,jePtrMgr *PtrMgr)
{

#if 0 //{
jeTerrain *T;

	T = jeTerrain_Create();
	if ( ! T )
	{
		jeErrorLog_AddString(-1,"CreateFromFile : failure", NULL);
		return NULL;
	}

	{
	jeXForm3d XF;
	jeVec3d Size;
	uint32 MaxQuads,TexDim;
	float MinError;

		jeVFile_ReadEntity(File, &XF );
		jeVFile_ReadEntity(File, &Size );
		jeVFile_ReadEntity(File, &MaxQuads );
		jeVFile_ReadEntity(File, &MinError );
		jeVFile_ReadEntity(File, &TexDim );

		if ( ! jeTerrain_SetXForm(T,&XF) )
		{
			jeErrorLog_AddString(-1,"CreateFromFile : failure", NULL);
			return NULL;
		}
		if ( ! jeTerrain_SetSize(T,&Size) )
		{
			jeErrorLog_AddString(-1,"CreateFromFile : failure", NULL);
			return NULL;
		}
		if ( ! jeTerrain_SetParameters(T,MaxQuads,MinError) )
		{
			jeErrorLog_AddString(-1,"CreateFromFile : failure", NULL);
			return NULL;
		}
		if ( ! jeTerrain_SetTexDim(T,TexDim) )
		{
			jeErrorLog_AddString(-1,"CreateFromFile : failure", NULL);
			return NULL;
		}
	}

	return T;

#else //}{

// Added by cjp 2.18.00
// Code to load files with the old VFS style terrain objects.

#ifdef CJP_LOAD_OLDTERRAINOBJECTS

jeTerrain *T;
jeVFile *VFS,*SubFile;
jeBitmap * Heightmap;
int tx,ty;

	#ifdef DISABLE_PTRMGR
	PtrMgr = NULL;
	#endif

	T = jeTerrain_Create();
	if ( ! T )
		return NULL;

	VFS = jeVFile_OpenNewSystem( File, JE_VFILE_TYPE_VIRTUAL, NULL, NULL, 
									JE_VFILE_OPEN_READONLY | JE_VFILE_OPEN_DIRECTORY );
	if ( ! VFS )
		return NULL;

	SubFile = jeVFile_Open( VFS, "Terrain_Info", JE_VFILE_OPEN_READONLY );
	if ( ! SubFile )
	{
		jeVFile_Close(VFS);
		return NULL;
	}

	/*****

	XForm
	Size
	MaxQuads & MinError
	TexDim

	******/

	{
	jeXForm3d XF;
	jeVec3d Size;
	uint32 MaxQuads,TexDim;
	float MinError;
	uint32 Tag;

		jeVFile_ReadEntity(SubFile, &Tag);

		if ( Tag != jeTerrain_Tag )
		{
			jeErrorLog_AddString(-1,"CreateFromFile : didn't get terrain tag!", NULL);
			return NULL;
		}

		jeVFile_ReadEntity(SubFile, &XF );
		jeVFile_ReadEntity(SubFile, &Size );
		jeVFile_ReadEntity(SubFile, &MaxQuads );
		jeVFile_ReadEntity(SubFile, &MinError );
		jeVFile_ReadEntity(SubFile, &TexDim );

		if ( ! jeTerrain_SetXForm(T,&XF) )
		{
			jeErrorLog_AddString(-1,"CreateFromFile : failure", NULL);
			return NULL;
		}
		if ( ! jeTerrain_SetSize(T,&Size) )
		{
			jeErrorLog_AddString(-1,"CreateFromFile : failure", NULL);
			return NULL;
		}
		if ( ! jeTerrain_SetParameters(T,MaxQuads,MinError) )
		{
			jeErrorLog_AddString(-1,"CreateFromFile : failure", NULL);
			return NULL;
		}
		if ( ! jeTerrain_SetTexDim(T,TexDim) )
		{
			jeErrorLog_AddString(-1,"CreateFromFile : failure", NULL);
			return NULL;
		}
	}

	jeVFile_Close(SubFile); SubFile = NULL;

//	if ( ! (Heightmap = jeBitmap_CreateFromFile2(SubFile,ResourceBaseFS,PtrMgr)) )
	if ( ! (Heightmap = jeBitmap_CreateFromFileName(VFS,"Terrain_Heightmap")) )
	{
		jeVFile_Close(SubFile);
		jeVFile_Close(VFS);
		destroy(T);
		return NULL;
	}

	if ( ! jeTerrain_SetHeightmap(T,Heightmap) )
	{
		jeTerrain_Destroy(&T);
		jeVFile_Close(VFS);
		return NULL;
	}

	for(tx=0;tx<(T->TexDim);tx++)
	{
		for(ty=0;ty<(T->TexDim);ty++)
		{
		char Name[1024];
		jeBitmap * Tex;

			sprintf(Name,"Terrain_Texture%d",tx + ty*(T->TexDim));

		//	if ( ! (Tex = jeBitmap_CreateFromFileName2(VFS,Name,PtrMgr)) )
			if ( ! (Tex = jeBitmap_CreateFromFileName(VFS,Name)) )
			{
				jeVFile_Close(VFS);
				destroy(T);
				return NULL;
			}

			jeTerrain_SetATexture(T,Tex,tx,ty);
		}
	}

	jeVFile_Close(VFS);

return T;

#else
// Load the new direct style of terrain objects.

	jeTerrain *T;

	jeBitmap * Heightmap;
	int tx,ty;
	uint8 Version;
	uint32 Tag;

	T = (jeTerrain *)jeTerrain_Create();
	if ( ! T )
		return NULL;

	/*****

	XForm
	Size
	MaxQuads & MinError
	TexDim

	******/

	{
		jeXForm3d XF;
		jeVec3d Size;
		uint32 MaxQuads,TexDim;
		float MinError;

		jeVFile_ReadEntity(File, &Tag);

		if ( Tag != jeTerrain_Tag )
		{
			jeErrorLog_AddString(-1,"CreateFromFile : didn't get terrain tag!", NULL);
			return NULL;
		}

		jeVFile_ReadEntity(File, &XF );
		jeVFile_ReadEntity(File, &Size );
		jeVFile_ReadEntity(File, &MaxQuads );
		jeVFile_ReadEntity(File, &MinError );
		jeVFile_ReadEntity(File, &TexDim );

		if ( ! jeTerrain_SetXForm(T,&XF) )
		{
			jeErrorLog_AddString(-1,"CreateFromFile : failure", NULL);
			return NULL;
		}
		if ( ! jeTerrain_SetSize(T,&Size) )
		{
			jeErrorLog_AddString(-1,"CreateFromFile : failure", NULL);
			return NULL;
		}
		if ( ! jeTerrain_SetParameters(T,MaxQuads,MinError) )
		{
			jeErrorLog_AddString(-1,"CreateFromFile : failure", NULL);
			return NULL;
		}
		if ( ! jeTerrain_SetTexDim(T,TexDim) )
		{
			jeErrorLog_AddString(-1,"CreateFromFile : failure", NULL);
			return NULL;
		}
	}

//	if ( ! (Heightmap = jeBitmap_CreateFromFile2(SubFile,ResourceBaseFS,PtrMgr)) )
	if ( ! (Heightmap = jeBitmap_CreateFromFile(File)) )
	{
		destroy(T);
		return NULL;
	}

	if ( ! jeTerrain_SetHeightmap(T,Heightmap) )
	{
		jeTerrain_Destroy((void **)&T);
		return NULL;
	}

	if(!jeVFile_Read(File, &Tag, sizeof(Tag)))
	{
		jeErrorLog_Add( JE_ERR_SYSTEM_RESOURCE, NULL );
		jeTerrain_Destroy((void **)&T);
		return NULL;
	}

	if (Tag == FILE_UNIQUE_ID)
	{
		if (!jeVFile_Read(File, &Version, sizeof(Version)))
		{
    		jeErrorLog_Add( JE_ERR_SYSTEM_RESOURCE, NULL );
			jeTerrain_Destroy((void **)&T);
		    return NULL;
		}
	}
	else
	{
		Version = 0;
		jeVFile_Seek(File,-((int)sizeof(Tag)),JE_VFILE_SEEKCUR);
	}
	
	// I think it should be for(y values) then for(x values) 
	for(ty=0;ty<(T->TexDim);ty++)
	{
		for(tx=0;tx<(T->TexDim);tx++)
		{
			jeBitmap * Tex;

			if (Version) {
				char Name[256];
				jeVFile_Read(File, Name, 256);
				Tex = (jeBitmap*) jeResource_GetResource(jePtrMgr_GetResourceMgr(PtrMgr), JE_RESOURCE_BITMAP, Name);
			} else {
				Tex = jeBitmap_CreateFromFile(File);
			}

//			sprintf(Name,"Terrain_Texture%d",tx + ty*(T->TexDim));
//			if ( ! (Tex = jeBitmap_CreateFromFileName2(VFS,Name,PtrMgr)) )

			if (Tex==NULL)
			{
				destroy(T);
				return NULL;
			}

			jeTerrain_SetATexture(T,Tex,tx,ty);
		}
	}

return T;

// End of new loader code
#endif // OLD_TERRAINOBJECTS

#endif //}

}

JETAPI void * JETCC jeTerrain_CreateFromFile(jeVFile * File, jePtrMgr *PtrMgr)
{
return jeTerrain_CreateFromFileExt(File,NULL,PtrMgr);
}

/*}{******************************************************/

static void jeTerrain_XFormCameraToTerrainSpace(jeTerrain * T,jeCamera *pC)
{
jeXForm3d CXF;

	/**
		Camera in takes World space to Screen space
		we want to make a camera that takes terrain space to screen space

		the old camera xform is WtoS
		we want to append TtoW
			C = (WtoS) (TtoW)

		notez : the "Transpose" XForm in the camera is the one that
			actually does the WtoS projection
	***/

	jeCamera_GetTransposeXForm(pC,&CXF);  // CXF = W ^ -1

	jeXForm3d_Multiply(&CXF,&(T->XFTerrainToWorld),&CXF); // CXF = W^-1 T

	jeCamera_SetTransposeXForm(pC,&CXF); // C = CXF^-1 = (W^-1 T)^-1 = T^-1 W
}

static void jeTerrain_XFormFrustumToTerrainSpace(jeTerrain * T,const jeFrustum *worldF,jeFrustum *pF)
{
	jeFrustum_Transform(worldF,&(T->XFWorldToTerrain),pF);
}

extern const jeXForm3d *	JETCF jeCamera_XForm( const jeCamera *Camera);
extern const jeXForm3d *	JETCF jeCamera_WorldXForm( const jeCamera *Camera);
//extern const jeVec3d *		JETCF jeCamera_GetPov(const jeCamera *Camera);

static void jeTerrain_MakeTerrainSpaceFrustum(jeTerrain * T,const jeCamera * Camera,jeFrustum *pF)
{
	jeCamera_PushXForm((jeCamera *)Camera);
	jeTerrain_XFormCameraToTerrainSpace(T,(jeCamera *)Camera);
	
	jeFrustum_SetFromCamera(pF,Camera); // pF in camera space

	jeFrustum_TransformAnchored(pF, jeCamera_WorldXForm(Camera), jeCamera_GetPov(Camera) );

	jeCamera_PopXForm((jeCamera *)Camera);
}

/*}{****************** Z at XY stuff ************************************/
// !!! ALL OF THIS _Get stuff is in terrain space !!

static void jeTerrain_GetZBox(const jeTerrain *T,jeFloat X,jeFloat Y,jeFloat * CornerZs, jeFloat * pfx,jeFloat * pfy)
{
float baseX,baseY;
int sx,sy;
float * HMptr;

	sx = (int)(X * T->InvCubeSize.X);
	sy = (int)(Y * T->InvCubeSize.Y);
	
	sx = JE_CLAMP(sx,0,T->HMWidth -2);
	sy = JE_CLAMP(sy,0,T->HMHeight-2);

	baseX = sx * T->CubeSize.X;
	baseY = sy * T->CubeSize.Y;

	*pfx = (X - baseX) * T->InvCubeSize.X;
	*pfy = (Y - baseY) * T->InvCubeSize.Y;

	HMptr = T->HM + sx + sy * T->HMWidth;
	CornerZs[0] = HMptr[0];
	CornerZs[1] = HMptr[1];
	HMptr += T->HMWidth;
	CornerZs[2] = HMptr[1];
	CornerZs[3] = HMptr[0];

return;
}

JETAPI jeFloat JETCC jeTerrain_GetHeightAtWorldSpaceVec(const jeTerrain *Terrain,const jeVec3d *pV)
{
jeVec3d V;
	jeXForm3d_Transform(&(Terrain->XFWorldToTerrain),pV,&V);
	V.Z = jeTerrain_GetHeightAtXY(Terrain,V.X,V.Y);
	jeXForm3d_Transform(&(Terrain->XFTerrainToWorld),&V,&V);
return V.Z;
}

JETAPI void JETCC jeTerrain_GetNormalAtWorldSpaceVec(const jeTerrain *Terrain,const jeVec3d *pV,jeVec3d *pN)
{
jeVec3d V;
	jeXForm3d_Transform(&(Terrain->XFWorldToTerrain),pV,&V);
	jeTerrain_GetNormalAtXY(Terrain,V.X,V.Y,pN);
	jeXForm3d_Rotate(&(Terrain->XFTerrainToWorld),pN,pN);
}

JETAPI jeFloat JETCC jeTerrain_GetHeightAtXY(const jeTerrain *Terrain,jeFloat X,jeFloat Y)
{
jeFloat fx,fy;
jeFloat CornerZs[4]; //SW,SE,NE,NW
jeFloat z;

	assert( jeTerrain_IsValid(Terrain) );

	jeTerrain_GetZBox(Terrain,X,Y, CornerZs, &fx,&fy);
	
	z =			fy  * (CornerZs[2] * fx + CornerZs[3] * (1.0f - fx)) + 
		(1.0f - fy) * (CornerZs[1] * fx + CornerZs[0] * (1.0f - fx));

return z;
}

static jeBoolean 	jeTerrain_GetNormalAtXY_Raw(const jeTerrain *Terrain,jeFloat X,jeFloat Y,
										jeVec3d *pNormal,jeFloat *pfx,jeFloat *pfy)
{
jeFloat CornerZs[4]; //SW,SE,NE,NW
jeVec3d Seg1,Seg2;

	jeTerrain_GetZBox(Terrain,X,Y, CornerZs, pfx,pfy);
	
	// {} could write a custom cross product that takes advantage of our known zeros

	Seg1.X = Terrain->CubeSize.X;
	Seg1.Y = 0.0f;
	Seg1.Z = CornerZs[1] - CornerZs[0]; //SE - SW

	Seg2.X = 0.0f;
	Seg2.Y = Terrain->CubeSize.Y;
	Seg2.Z = CornerZs[3] - CornerZs[0]; //NW - SW

	jeVec3d_CrossProduct(&Seg1,&Seg2,pNormal); // pNormal = Seg1 x Seg2 , points up
	assert( pNormal->Z >= 0.0f );
	jeVec3d_Normalize(pNormal);

	return JE_TRUE;
}

JETAPI void JETCC jeTerrain_GetNormalAtXY_Rough(const jeTerrain *Terrain,jeFloat X,jeFloat Y,jeVec3d *pNormal)
{
jeFloat fx,fy;
	jeTerrain_GetNormalAtXY_Raw(Terrain,X,Y,pNormal,&fx,&fy);
}

JETAPI void JETCC jeTerrain_GetNormalAtXY(const jeTerrain *Terrain,jeFloat X,jeFloat Y,jeVec3d *pNormal)
{
jeFloat fx,fy,mulx,muly,stepx,stepy;
jeVec3d NormalX,NormalY;

	assert( jeTerrain_IsValid(Terrain) );

	jeTerrain_GetNormalAtXY_Raw(Terrain,X,Y,pNormal,&fx,&fy);
	if ( fx < 0.5f )
	{
		mulx = 1.0f - 2.0f * fx;
		stepx = - Terrain->CubeSize.X;
	}
	else
	{
		mulx = 2.0f * fx - 1.0f;
		stepx = + Terrain->CubeSize.X;
	}

	if ( fy < 0.5f )
	{
		muly = 1.0f - 2.0f * fy;
		stepy = - Terrain->CubeSize.Y;
	}
	else
	{
		muly = 2.0f * fy - 1.0f;
		stepy = + Terrain->CubeSize.Y;
	}

	jeTerrain_GetNormalAtXY_Raw(Terrain,X+stepx,Y,&NormalX,&fx,&fy);
	jeTerrain_GetNormalAtXY_Raw(Terrain,X,Y+stepy,&NormalY,&fx,&fy);

	jeVec3d_AddScaled(pNormal,&NormalX,mulx,pNormal);
	jeVec3d_AddScaled(pNormal,&NormalY,muly,pNormal);

	jeVec3d_Normalize(pNormal);
}

/*}{******************************************************/

jeBoolean jeExtBox_SphereCollision(jeExtBox *pBox,jeVec3d *pPos,jeFloat Radius)
{
	if(( pPos->X >= (pBox->Min.X - Radius) && pPos->X <= (pBox->Max.X + Radius) )
	&& ( pPos->Y >= (pBox->Min.Y - Radius) && pPos->Y <= (pBox->Max.Y + Radius) )
	&& ( pPos->Z >= (pBox->Min.Z - Radius) && pPos->Z <= (pBox->Max.Z + Radius) ))
		return JE_TRUE;
return JE_FALSE;
}

static jeBoolean jeTerrain_SetDynamicLightsFromWorld(jeTerrain *T,jeWorld *World)
{
jeChain * LightChain;
jeChain_Link * Link;
jeExtBox TerrainBox;

	assert( jeTerrain_IsValid(T) );

	LightChain = jeWorld_GetDLightChain(World);
	if ( ! LightChain )
		return JE_FALSE;

	T->NumDynamicLights = 0;

	jeTerrain_GetExtBox(T,&TerrainBox);

	for( Link = jeChain_GetFirstLink(LightChain); Link; Link = jeChain_LinkGetNext(Link) )
	{
	jeLight * Light;
	jeVec3d Pos,Color;
	jeFloat Radius,Brightness;
	uint32 Flags;
	jeTerrain_Light * TLight;

		if ( T->NumDynamicLights >= TERRAIN_MAX_NUM_LIGHTS )
			break; 

		Light = (jeLight *)jeChain_LinkGetLinkData(Link);

		if ( ! jeLight_GetAttributes(Light,&Pos,&Color,&Radius,&Brightness,&Flags) )
			return JE_FALSE;

		// if the light is way out of the extbox of the terrain, don't even add it to the list!
		if ( ! (Flags & JE_LIGHT_FLAG_PARALLEL) )
			if ( ! jeExtBox_SphereCollision(&TerrainBox,&Pos,Radius) )
				continue; 

		TLight = T->DynamicLights + T->NumDynamicLights;
		T->NumDynamicLights ++;

		TLight->Type = TERRAIN_LIGHT_SPHERE;

		// must store the position in terrain space!		
		jeXForm3d_Transform(&(T->XFWorldToTerrain),&Pos,&(TLight->Vector));

		jeVec3d_Scale(&Color, Brightness, &Color); // <> or something

		TLight->Color.r = Color.X;
		TLight->Color.g = Color.Y;
		TLight->Color.b = Color.Z;

		TLight->MaxColor = max3(JE_ABS(TLight->Color.r),JE_ABS(TLight->Color.g),JE_ABS(TLight->Color.b));
	}

return JE_TRUE;
}

static jeBoolean jeTerrain_RestoreUnLitTextures(jeTerrain *T)
{
int i;
jeBitmap *OldBmp;

	if ( ! T->TexturesAreLit )
		return JE_TRUE;

	for(i=0;i<(T->TexDim * T->TexDim);i++)
	{
		OldBmp = T->PreLightTextures[i];
		if ( ! OldBmp )
		{
			T->PreLightTextures[i] = T->Textures[i];
			T->Textures[i] = jeBitmap_CreateCopy(T->PreLightTextures[i]);

			if ( T->Engine )
			{
				if ( T->RegisteredTexture[i] )
				{
					jeEngine_RemoveBitmap(T->Engine,T->PreLightTextures[i]);
				}
				jeEngine_AddBitmap(T->Engine,T->Textures[i],JE_ENGINE_BITMAP_TYPE_3D);
				T->RegisteredTexture[i] = JE_TRUE;
			}
		}
		else
		{
			jeBitmap_BlitBitmap( OldBmp, T->Textures[i] );
		}
	}

	T->TexturesAreLit = JE_FALSE;
return JE_TRUE;
}

static jeBoolean jeTerrain_SaveUnLitTextures(jeTerrain *T)
{
int i;

	for(i=0;i<(T->TexDim * T->TexDim);i++)
	{
		if ( ! T->PreLightTextures[i] )
		{
			T->PreLightTextures[i] = T->Textures[i];
			T->Textures[i] = jeBitmap_CreateCopy(T->PreLightTextures[i]);
			
			if ( T->Engine )
			{
				if ( T->RegisteredTexture[i] )
				{
					jeEngine_RemoveBitmap(T->Engine,T->PreLightTextures[i]);
				}
				jeEngine_AddBitmap(T->Engine,T->Textures[i],JE_ENGINE_BITMAP_TYPE_3D);
				T->RegisteredTexture[i] = JE_TRUE;
			}
		}
	}

return JE_TRUE;
}

static jeBoolean jeTerrain_SetDefaultLighting(jeTerrain *T)
{
	assert( jeTerrain_IsValid(T) );

	if ( T->TexturesAreLit )
	{
		if ( ! jeTerrain_RestoreUnLitTextures(T) )
			return JE_FALSE;
	}
	
	if ( T->QT )
	{
		QuadTree_ResetAllVertexLighting(T->QT);
	}

return JE_TRUE;
}

JETAPI jeBoolean JETCC jeTerrain_SetLightsInTextureFromWorld(jeTerrain *T,jeWorld *World,jeBoolean SelfShadow,jeBoolean GetWorldShadows)
{
jeLight * Lights[TERRAIN_MAX_NUM_LIGHTS];
int l,NumLights;
jeChain * LightChain;
jeChain_Link * Link;
jeExtBox TerrainBox;

	assert( jeTerrain_IsValid(T) );
	if ( ! T->QT )
		return JE_FALSE;

	if ( ! World )
		return JE_FALSE;
				
	if ( T->TexturesAreLit )
	{
		if ( ! jeTerrain_RestoreUnLitTextures(T) )
			return JE_FALSE;
	}
	else
	{
		jeTerrain_SaveUnLitTextures(T);
	}

	LightChain = jeWorld_GetLightChain(World);
	if ( ! LightChain )
		return JE_FALSE;

	jeTerrain_GetExtBox(T,&TerrainBox);

	for(NumLights = 0,Link = jeChain_GetFirstLink(LightChain); Link; Link = jeChain_LinkGetNext(Link) )
	{
	jeLight * Light;
	jeVec3d Pos,Color;
	jeFloat Radius,Brightness;
	uint32 Flags;

		if ( NumLights >= TERRAIN_MAX_NUM_LIGHTS )
			break; 

		Light = (jeLight *)jeChain_LinkGetLinkData(Link);

		if ( ! jeLight_GetAttributes(Light,&Pos,&Color,&Radius,&Brightness,&Flags) )
			return JE_FALSE;

		// if the light is way out of the extbox of the terrain, don't even add it to the list!	
		if ( ! (Flags & JE_LIGHT_FLAG_PARALLEL) )
			if ( ! jeExtBox_SphereCollision(&TerrainBox,&Pos,Radius) )
				continue; 

		Lights[NumLights] = jeLight_CreateFromLight(Light);

		// must store the position in terrain space!		
		jeXForm3d_Transform(&(T->XFWorldToTerrain),&Pos,&Pos);

		if ( ! jeLight_SetAttributes(Lights[NumLights],&Pos,&Color,Radius,Brightness,Flags) )
			return JE_FALSE;

		NumLights++;
	}


	QuadTree_ResetAllVertexLighting(T->QT);

	QuadTree_LightTexture(T->QT,Lights,NumLights,SelfShadow,GetWorldShadows);

	for(l=0;l<NumLights;l++)
	{
		jeLight_Destroy(&(Lights[l]));
	}

	T->TexturesAreLit = JE_TRUE;

return JE_TRUE;
}

JETAPI jeBoolean JETCC jeTerrain_SetLightsOnVertsFromWorld(jeTerrain *T,jeWorld *World)
{
jeLight * Lights[TERRAIN_MAX_NUM_LIGHTS];
int l,NumLights;
jeChain * LightChain;
jeChain_Link * Link;
jeExtBox TerrainBox;

	assert( jeTerrain_IsValid(T) );
	if ( ! T->QT )
		return JE_FALSE;

	if ( ! World )
		return JE_FALSE;

	LightChain = jeWorld_GetLightChain(World);
	if ( ! LightChain )
		return JE_FALSE;

	jeTerrain_GetExtBox(T,&TerrainBox);

	for(NumLights = 0,Link = jeChain_GetFirstLink(LightChain); Link; Link = jeChain_LinkGetNext(Link) )
	{
	jeLight * Light;
	jeVec3d Pos,Color;
	jeFloat Radius,Brightness;
	uint32 Flags;

		if ( NumLights >= TERRAIN_MAX_NUM_LIGHTS )
			break; 

		Light = (jeLight *)jeChain_LinkGetLinkData(Link);

		if ( ! jeLight_GetAttributes(Light,&Pos,&Color,&Radius,&Brightness,&Flags) )
			return JE_FALSE;

		// if the light is way out of the extbox of the terrain, don't even add it to the list!
		if ( ! (Flags & JE_LIGHT_FLAG_PARALLEL) )
			if ( ! jeExtBox_SphereCollision(&TerrainBox,&Pos,Radius) )
				continue; 

		Lights[NumLights] = jeLight_CreateFromLight(Light);

		// must store the position in terrain space!		
		jeXForm3d_Transform(&(T->XFWorldToTerrain),&Pos,&Pos);

		if ( ! jeLight_SetAttributes(Lights[NumLights],&Pos,&Color,Radius,Brightness,Flags) )
			return JE_FALSE;

		NumLights++;
	}

	jeTerrain_RestoreUnLitTextures(T);

	QuadTree_LightAllPoints(T->QT,Lights,NumLights);

	for(l=0;l<NumLights;l++)
	{
		jeLight_Destroy(&(Lights[l]));
	}

return JE_TRUE;
}

JETAPI jeBoolean JETCC jeTerrain_SetLightsOnVerts(jeTerrain *T, jeLight **SrcLights, int Count)
{
	jeLight * Lights[TERRAIN_MAX_NUM_LIGHTS];
	int l,NumLights;
	int	i;
	//jeChain * LightChain;
	//jeChain_Link * Link;
	jeExtBox TerrainBox;

	if	(Count > TERRAIN_MAX_NUM_LIGHTS)
		return JE_FALSE;

	assert( jeTerrain_IsValid(T) );
	if ( ! T->QT )
		return JE_FALSE;

	jeTerrain_GetExtBox(T,&TerrainBox);

	for(NumLights = 0, i = 0; i < Count; i++ )
	{
		jeLight * Light;
		jeVec3d Pos,Color;
		jeFloat Radius,Brightness;
		uint32 Flags;

		Light = SrcLights[i];

		if ( ! jeLight_GetAttributes(Light,&Pos,&Color,&Radius,&Brightness,&Flags) )
			return JE_FALSE;

		// if the light is way out of the extbox of the terrain, don't even add it to the list!
		if ( ! (Flags & JE_LIGHT_FLAG_PARALLEL) )
			if ( ! jeExtBox_SphereCollision(&TerrainBox,&Pos,Radius) )
				continue; 

		Lights[NumLights] = jeLight_CreateFromLight(Light);

		// must store the position in terrain space!		
		jeXForm3d_Transform(&(T->XFWorldToTerrain),&Pos,&Pos);

		if ( ! jeLight_SetAttributes(Lights[NumLights],&Pos,&Color,Radius,Brightness,Flags) )
			return JE_FALSE;

		NumLights++;
	}

	jeTerrain_RestoreUnLitTextures(T);

	QuadTree_LightAllPoints(T->QT,Lights,NumLights);

	for(l=0;l<NumLights;l++)
	{
		jeLight_Destroy(&(Lights[l]));
	}

	return JE_TRUE;
}

JETAPI jeBoolean JETCC jeTerrain_GetTextureAtXY(const jeTerrain *T,jeFloat X,jeFloat Y,const jeBitmap ** pBmp,int *pTX,int *pTY)
{
	int tx,ty;

	assert( jeTerrain_IsValid(T) );
	
	tx = (int)(X * T->TexDim / T->Size.X);
	ty = (int)(Y * T->TexDim / T->Size.Y);

	if ( tx < 0 || tx >= T->TexDim || ty < 0 || ty >= T->TexDim )
		return JE_FALSE;

	if ( pBmp )	*pBmp = T->Textures[ tx + ty * T->TexDim ];
	if ( pTX  ) *pTX = tx;
	if ( pTY  ) *pTY = ty;

	return JE_TRUE;
}

JETAPI jeBoolean JETCC jeTerrain_GetTextureAtWorldVec(const jeTerrain *T,const jeVec3d *pVec,const jeBitmap ** pBmp,int *pTX,int *pTY)
{
	jeVec3d V;
	jeXForm3d_Transform(&(T->XFWorldToTerrain),pVec,&V);
	return jeTerrain_GetTextureAtXY(T,V.X,V.Y,pBmp,pTX,pTY);
}

JETAPI jeBoolean JETCC jeTerrain_SetATexture(jeTerrain *T,const jeBitmap * Bmp,int x,int y)
{
	int i;

	assert( jeTerrain_IsValid(T) );

	jeTerrain_DeSelect(T);

	if ( ! Bmp ) Bmp = T->NullTexture;

	if ( x >= T->TexDim || y >= T->TexDim )
		return JE_FALSE;

	i = x + y * T->TexDim;

	jeBitmap_CreateRef((jeBitmap *)Bmp);

	if ( T->Textures[i] )
	{
		if ( T->RegisteredTexture[i] && T->Engine )
		{
			jeEngine_RemoveBitmap(T->Engine,T->Textures[i]);
			T->RegisteredTexture[i] = JE_FALSE;
		}
		jeBitmap_Destroy( & (T->Textures[i]) );
	}

	if ( T->PreLightTextures[i] )
		jeBitmap_Destroy( & (T->PreLightTextures[i]) );

	T->Textures[i] = (jeBitmap *)Bmp;
	T->PreLightTextures[i] = NULL;
	T->RegisteredTexture[i] = JE_FALSE;

	if ( T->Engine )
	{
		jeEngine_AddBitmap(T->Engine,T->Textures[i],JE_ENGINE_BITMAP_TYPE_3D);
		T->RegisteredTexture[i] = JE_TRUE;
	}

	return JE_TRUE;
}

static int intlog2(int x)
{
	float xf = (float)x;
	return ((*(int*)&xf) >> 23) - 127;
}

JETAPI jeBoolean JETCC jeTerrain_SetTexDim(jeTerrain *T,int TexDim)
{
	int OldTexDim;
	jeBitmap * OldTextures[MAX_TEXTURES];
	jeBitmap * OldPLTextures[MAX_TEXTURES];
	int x,y,i;

	assert( jeTerrain_IsValid(T) );
	
	jeTerrain_DeSelect(T);

	if ( TexDim < 1 || TexDim > MAX_TEXDIM )
	{
		jeErrorLog_AddString(-1,"SetTexDim : out of bounds!", NULL);
		return JE_FALSE;
	}

	x = intlog2(TexDim);
	if ( (1<<x) != TexDim ) // !! TexDim must be a power of 2
	{
		jeErrorLog_AddString(-1,"SetTexDim : not a power of 2", NULL);
		return JE_FALSE;
	}

	OldTexDim = T->TexDim;
	for(i=0;i<OldTexDim * OldTexDim;i++)
	{
		if ( T->Textures[i] && T->PreLightTextures[i] && T->RegisteredTexture[i] && T->Engine )
		{
			jeEngine_RemoveBitmap(T->Engine,T->Textures[i]);
			T->RegisteredTexture[i] = JE_FALSE;
		}
	}

	memcpy(OldTextures,T->Textures,sizeof(jeBitmap *)*MAX_TEXTURES);
	memcpy(OldPLTextures,T->PreLightTextures,sizeof(jeBitmap *)*MAX_TEXTURES);

	T->TexDim = TexDim;
	memset(T->Textures,0,sizeof(jeBitmap *)*MAX_TEXTURES);

	for(x=0;x<TexDim;x++)
	{
		for(y=0;y<TexDim;y++)
		{
			if ( x < OldTexDim && y < OldTexDim )
			{
			int i;
				i = x + y * OldTexDim;
				if ( OldPLTextures[i] )
					jeTerrain_SetATexture(T,OldPLTextures[i],x,y);
				else
					jeTerrain_SetATexture(T,OldTextures[i],x,y);
			}
			else
				jeTerrain_SetATexture(T,NULL,x,y);
		}
	}

	for(x=0;x<OldTexDim * OldTexDim;x++)
	{
		assert( OldTextures[x] );
		if ( OldPLTextures[x] )
		{
			jeBitmap_Destroy( OldPLTextures + x );
		}
		jeBitmap_Destroy( OldTextures + x );
	}

	if ( T->QT )
	{
		QuadTree_SetTexDim(T->QT,TexDim);
	}

	T->Changed = JE_TRUE;

return JE_TRUE;
}

/*}{******************************************************/

JETAPI jeBoolean JETCC jeTerrain_RenderPrep(jeTerrain * T,jeEngine *Engine,jeCamera *worldC)
{
jeVec3d Pos,Vec;

	assert( jeTerrain_IsValid(T) );
	assert( worldC);

	if ( T->Untouchable )
		return JE_TRUE;

	if ( ! T->QT )
		return JE_FALSE;

	jeTerrain_AttachEngine(T,Engine);

	{
	jeXForm3d XF;
		jeCamera_GetXForm(worldC,&XF);

		Pos = XF.Translation;
		jeXForm3d_GetIn(&XF,&Vec);
		jeXForm3d_Rotate(&(T->XFWorldToTerrain),&Vec,&Vec);

		jeXForm3d_Transform(&(T->XFWorldToTerrain),&Pos,&Pos);

		 // camera Pos & Vec now in terrain space
	}

	if ( ! T->Changed )
	{
	float dPos,dVec;
	jeVec3d Temp;

		jeVec3d_Subtract(&Pos,&(T->LastTesselatedCameraPos),&Temp);
		dPos = jeVec3d_Length(&Temp) * T->InvCubeSize.X;

			// difference in position *in heightmap pixels*

		dVec = jeVec3d_DotProduct(&Vec,&(T->LastTesselatedCameraVec));
		dVec = 1.0f - JE_ABS(dVec);

		// these numbers are pretty good
		// {} if you put the dPos tolerance too high, you'll get black slivers 
		//		when you make small moves (because of the backfacing)
		if ( dPos >= 0.05f || dVec >= 0.0001f )
		{
			T->Changed = JE_TRUE;
		}
	}

	if ( T->Changed )
	{
	jeFrustum F;

		jeTerrain_MakeTerrainSpaceFrustum(T,worldC,&F);

		if ( ! QuadTree_Tesselate(T->QT,&Pos,&F) )
		{
			return JE_FALSE;
		}

		if ( T->World )
		{
			jeTerrain_SetDynamicLightsFromWorld(T,T->World);
		}

		if ( T->NumDynamicLights > 0 )
		{
			if ( ! QuadTree_LightTesselatedPoints(T->QT,T->DynamicLights,T->NumDynamicLights) )
			{
				return JE_FALSE;
			}
		}

		T->LastTesselatedCameraPos = Pos;
		T->LastTesselatedCameraVec = Vec;

		T->Changed = JE_FALSE;
	}

	return JE_TRUE;
}

JETAPI jeBoolean JETCC jeTerrain_RenderThroughCamera(jeTerrain *T,const jeWorld *World, const jeEngine *E,
																	jeCamera *Camera)
{
jeFrustum F;
jeBoolean ret;

	assert( jeTerrain_IsValid(T) );	

	if ( ! T->World  ) ((jeTerrain *)T)->World = (jeWorld *)World;

	if (!jeTerrain_RenderPrep((jeTerrain*)T,(jeEngine *)E,Camera))
		return JE_FALSE;

	jeTerrain_MakeTerrainSpaceFrustum(T,Camera,&F);

	jeCamera_PushXForm(Camera);
	jeTerrain_XFormCameraToTerrainSpace(T,Camera);
	
	ret = QuadTree_Render(T->QT,(jeEngine *)E,Camera,&F);
	
	jeCamera_PopXForm(Camera);

return ret;
}

JETAPI jeBoolean JETCC jeTerrain_RenderThroughFrustum(jeTerrain *T,const jeWorld *World, const jeEngine *E,
																jeCamera *Camera, const jeFrustum *worldF)
{
jeFrustum F;
jeBoolean ret;

	assert( jeTerrain_IsValid(T) );	

	if ( ! T->World  ) ((jeTerrain *)T)->World = (jeWorld *)World;

	if (!jeTerrain_RenderPrep((jeTerrain*)T,(jeEngine *)E,Camera))
		return JE_FALSE;

	jeTerrain_XFormFrustumToTerrainSpace(T,worldF,&F);

	jeCamera_PushXForm(Camera);
	jeTerrain_XFormCameraToTerrainSpace(T,Camera);

	ret = QuadTree_Render(T->QT,(jeEngine *)E,Camera,&F);
	
	jeCamera_PopXForm(Camera);

return ret;
}

JETAPI jeBoolean JETCC jeTerrain_ObjectRender(const void *Terrain,const jeWorld *World, const jeEngine *E,
											const jeCamera *Camera, const jeFrustum *F,jeObject_RenderFlags RenderFlags)
{
	jeTerrain* T = (jeTerrain *)Terrain;
	assert( jeTerrain_IsValid(T) );	
	assert( World && E && Camera );

	if ( T->Untouchable )
		return JE_TRUE;

	if ( !T->RenderNextFlag )
		return JE_TRUE;

	if ( RenderFlags & JE_OBJECT_RENDER_FLAG_CAMERA_FRUSTUM )
	{
		return jeTerrain_RenderThroughCamera((jeTerrain*)T,World,E,(jeCamera *)Camera);
	}
	else
	{
	jeFrustum WF;
		jeFrustum_TransformToWorldSpace(F,Camera,&WF);
		return jeTerrain_RenderThroughFrustum((jeTerrain*)T,World,E,(jeCamera *)Camera,&WF);
	}
}

/*}{******************************************************/

JETAPI jeBoolean JETCC jeTerrain_SetParameters(jeTerrain *T,uint32 MaxQuads,float MinError)
{
	assert( jeTerrain_IsValid(T) );
	T->MaxQuads = MaxQuads;
	T->MinError = MinError;

	if ( T->QT )
	{
		// if QT is made later, we'll set the parameters then
		QuadTree_SetParameters(T->QT,2,MaxQuads,MinError);
	}

	// force a re-tesselate
	T->Changed = JE_TRUE;

return JE_TRUE;
}

JETAPI void JETCC jeExtBox_Transform(const jeExtBox *pIn,const jeXForm3d * pXF,jeExtBox *pOut)
{
jeVec3d Corners[8];
int i;

	for(i=0;i<8;i++)
	{
	jeVec3d *pV;
		pV = Corners + i;
		if ( i & 1 ) pV->X = pIn->Min.X; else pV->X = pIn->Max.X;
		if ( i & 2 ) pV->Y = pIn->Min.Y; else pV->Y = pIn->Max.Y;
		if ( i & 4 ) pV->Z = pIn->Min.Z; else pV->Z = pIn->Max.Z;
		
		jeXForm3d_Transform(pXF,pV,pV);
	}

	pOut->Min = pOut->Max = Corners[0];
	for(i=1;i<8;i++)
	{
	jeVec3d *pV;
		pV = Corners + i;
		pOut->Min.X = min(pOut->Min.X,pV->X);
		pOut->Min.Y = min(pOut->Min.Y,pV->Y);
		pOut->Min.Z = min(pOut->Min.Z,pV->Z);
		pOut->Max.X = max(pOut->Max.X,pV->X);
		pOut->Max.Y = max(pOut->Max.Y,pV->Y);
		pOut->Max.Z = max(pOut->Max.Z,pV->Z);
	}
}

JETAPI jeBoolean JETCC jeTerrain_GetExtBox(const void *Terrain,jeExtBox * pBox)
{
	const jeTerrain* T = (jeTerrain *)Terrain;
	assert( jeTerrain_IsValid(T) );
	
	if ( T->QT && ! T->Untouchable )
	{
		QuadTree_GetExtBox(T->QT,pBox);

		// pbox is in terrain space; must xform it back :

		jeExtBox_Transform(pBox,&(T->XFTerrainToWorld),pBox);
	}
	else
	{
		// make a little fake box
		pBox->Min = T->XFTerrainToWorld.Translation;
		pBox->Max = pBox->Min;
		pBox->Max.X += T->Size.X;
		pBox->Max.Y += T->Size.Y;
		pBox->Max.Z += T->Size.Z;
	}
 
   return JE_TRUE;
}

static jeBoolean jeTerrain_IsValid(const jeTerrain *T)
{
	assert( T );	
	assert( ! T->QT || QuadTree_IsValid(T->QT) );
	assert( T->SelfCheck == T );
	return JE_TRUE;
}

JETAPI jeBoolean JETCC jeTerrain_IntersectsRay(const jeTerrain *T,jeVec3d *pStart,jeVec3d *pDirection)
{
   jeExtBox Box;
   jeVec3d Start,Direction;

	assert( jeTerrain_IsValid(T) );
	assert(pStart && pDirection );

	if ( ! T->QT )
		return JE_FALSE;

	// xform the ray into terrain space :
	jeXForm3d_Transform(&(T->XFWorldToTerrain),pStart,&Start);
	jeXForm3d_Rotate(&(T->XFWorldToTerrain),pDirection,&Direction);

	QuadTree_GetExtBox(T->QT,&Box);

	// this isn't reall right, if start is outside & direction points towards the QT
	pStart->X = JE_CLAMP(pStart->X,Box.Min.X,Box.Max.X);
	pStart->Y = JE_CLAMP(pStart->Y,Box.Min.Y,Box.Max.Y);
	pStart->Z = JE_CLAMP(pStart->Z,Box.Min.Z,Box.Max.Z);

	#if 1
	/*****
	This isn't actually right, cuz pStart->Z could be above the terrain, but 
	we need to avoid the asserts in QuadTree_ for now
	*****/
	{
	jeVec3d Normal;
		jeTerrain_GetNormalAtXY(T,Start.X,Start.Y,&Normal);
		if ( jeVec3d_DotProduct(&Normal,&Direction) <= 0.0f )
			return JE_TRUE;
	}
	#endif

	return QuadTree_IntersectRay(T->QT,&Start,&Direction);
}

JETAPI jeBoolean JETCC jeTerrain_SphereCollision(const jeTerrain *T,
													const jeVec3d *pFrom, const jeVec3d *pTo, jeFloat Radius,
													jeVec3d *Impact, jePlane *Plane)
{
   jeVec3d From,To;
   jeFloat FromZ,ToZ,MinRadius;
   jeExtBox QTBox;

	assert( jeTerrain_IsValid(T) );
	if ( ! T->QT )
		return JE_FALSE;

	// xform the ray into terrain space :
	jeXForm3d_Transform(&(T->XFWorldToTerrain),pFrom,&From);
	jeXForm3d_Transform(&(T->XFWorldToTerrain),pTo  ,&To  );

	MinRadius = min3(T->CubeSize.X,T->CubeSize.Y,T->CubeSize.Z);
	MinRadius *= 0.5f;
	if ( Radius < MinRadius ) Radius = MinRadius;

	QuadTree_GetExtBox(T->QT,&QTBox);

	// early out:
	if ( ! jeExtBox_RayCollision(&QTBox,&From,&To,NULL,NULL) &&
		! jeExtBox_ContainsPoint(&QTBox,&From) && ! jeExtBox_ContainsPoint(&QTBox,&To)  )
		return JE_FALSE;

	FromZ = jeTerrain_GetHeightAtXY(T,From.X,From.Y);
	ToZ   = jeTerrain_GetHeightAtXY(T,To.X,To.Y);

	if ( From.Z <= (FromZ + Radius) )
	{
		// collided at start!
		*Impact = From;
		Impact->Z = FromZ;
		goto GotImpact;
	}
	
	if ( ! QuadTree_IntersectThickRay(T->QT,&From,&To,Radius,Impact) )
		return JE_FALSE; // no collision

GotImpact :

	// set up the plane from the Impact
	jeTerrain_GetNormalAtXY(T,Impact->X,Impact->Y,&(Plane->Normal));

	// fix Impact and Plane back to world space
	jeXForm3d_Transform(&(T->XFTerrainToWorld),Impact,Impact);
	jeXForm3d_Rotate(&(T->XFTerrainToWorld),&(Plane->Normal),&(Plane->Normal));

	Plane->Dist = jeVec3d_DotProduct( Impact, &(Plane->Normal) );
	Plane->Type = Type_Any;

   return JE_TRUE;
}

JETAPI jeBoolean JETCC jeTerrain_BoxCollision(const void *Terrain, const jeExtBox *Box, 
													const jeVec3d *Front, const jeVec3d *Back, 
													jeVec3d *Impact, jePlane *Plane)
{
   jeFloat Radius;
   jeVec3d LocalImpact;	// Added by Icestorm: Impact&Plane CAN be NULL
   jePlane LocalPlane;
   const jeTerrain* T = (jeTerrain *)Terrain;

	if ( ! Impact )
		Impact=&LocalImpact;
	if ( ! Plane )
		Plane=&LocalPlane;
	if ( ! Box )
		Radius = 0.0f;
	else
		Radius = jeVec3d_DistanceBetween( &(Box->Max), &(Box->Min) ) * 0.5f;

   return jeTerrain_SphereCollision(T,Front,Back,Radius,Impact,Plane);
}

/*}{******************************************************/
/**********
#if 0 //{

static void MakeExtBoxSilhouette(jeVec3d * Silhouette,const jeVec3d *POV,const jeExtBox *pBox)
{
jeVec3d Close,Far;

	// the silhoutte is all 6 points which are neither the nearest nor the farthest

	if ( JE_ABS(POV->X - pBox->Min.X) < JE_ABS(POV->X - pBox->Max.X) )
		{	Close.X = pBox->Min.X; Far.X = pBox->Max.X;	} 	
	else
		{	Close.X = pBox->Max.X; Far.X = pBox->Min.X;	} 	
		
	if ( JE_ABS(POV->Y - pBox->Min.Y) < JE_ABS(POV->Y - pBox->Max.Y) )
		{	Close.Y = pBox->Min.Y; Far.Y = pBox->Max.Y;	} 	
	else
		{	Close.Y = pBox->Max.Y; Far.Y = pBox->Min.Y;	} 	

	if ( JE_ABS(POV->Z - pBox->Min.Z) < JE_ABS(POV->Z - pBox->Max.Z) )
		{	Close.Z = pBox->Min.Z; Far.Z = pBox->Max.Z;	} 	
	else
		{	Close.Z = pBox->Max.Z; Far.Z = pBox->Min.Z;	}

	*Silhouette = Close;
	Silhouette->X = Far.X;
	Silhouette++;

	*Silhouette = Close;
	Silhouette->X = Far.X;
	Silhouette->Y = Far.Y;
	Silhouette++;

	*Silhouette = Close;
	Silhouette->Y = Far.Y;
	Silhouette++;
	
	*Silhouette = Close;
	Silhouette->Y = Far.Y;
	Silhouette->Z = Far.Z;
	Silhouette++;

	*Silhouette = Close;
	Silhouette->Z = Far.Z;
	Silhouette++;

	*Silhouette = Close;
	Silhouette->X = Far.X;
	Silhouette->Z = Far.Z;
	Silhouette++;
}

JETAPI jeBoolean JETCC jeTerrain_ExtBoxIsVis(const jeTerrain *T,const jeExtBox *pBox,const jeCamera *pCamera)
{
const jeVec3d * POV;
jeVec3d Silhouette[6];
jeFrustum F;
int i;

	assert( jeTerrain_IsValid(T) );
	assert( pBox && pCamera );

	POV = jeCamera_GetPov(pCamera);

	if ( jeExtBox_ContainsPoint(pBox,POV) )
		return JE_TRUE;

	MakeExtBoxSilhouette(Silhouette,POV,pBox);

	jeFrustum_SetWorldSpaceFromCamera(&F,pCamera);

	// quick test the bbox vs. frustum

	if ( ! jeFrustum_SetClipFlagsFromExtBox(&F,pBox,(1UL<<F.NumPlanes)-1,NULL) )
		return JE_FALSE;

	// could just shoot IntersectRays to the 6 sillhouette verts ?
	//	NO ! in fact the frustum has the same problem :
	//		you'll hit quads *behind* the desired object !

#if 0
	{
	jeFrustum_ClipInfo ClipInfo;
	jeVec3d Work1[64],Work2[64]

		ClipInfo.ClipFlags = (1UL<<F.NumPlanes)-1;
		ClipInfo.NumSrcVerts = 6;
		ClipInfo.SrcVerts = Silhouette;
		ClipInfo.Work1 = Work1;
		ClipInfo.Work2 = Work2;

		if ( ! jeFrustum_ClipVerts(&F,&ClipInfo) )
			return JE_FALSE;
		if ( ClipInfo.NumDstVerts < 3 )
			return JE_FALSE;

		if ( ! jeFrustum_SetFromVerts(&F,POV,ClipInfo.DstVerts,ClipInfo.NumDstVerts) )
			return JE_TRUE;

	// this is all very nice, but the QuadTree_InterectFrustum function is a nightmare!
	//	what we really want to know is : is the frustum totally covered ?
	//	but to know that we must build a beamtree or something to add up the partial occlusions !!
	//return QuadTree_IntersectFrustum(T->QT,&F);
	}
#endif

	return JE_TRUE;
}

#endif //}
***************/

/*}{******************************************************/

static jeProperty * jeProperty_GetOrCreate(jeProperty_List * pList,int FieldID)
{
   jeProperty * pP;

	pP = jeProperty_ListFindByDataId(pList,FieldID);

	if ( ! pP )
	{
	jeProperty Dummy;
		Dummy.FieldName = NULL;

		if ( ! jeProperty_Append(pList,&Dummy) )
			return NULL;

		pP = pList->pjeProperty + (pList->jePropertyN - 1);
	}

   return pP;
}

jeBoolean	JETCC jeTerrain_GetPropertyListPtr(jeTerrain * T, jeProperty_List **ppList)
{
   jeProperty * P;

	assert( jeTerrain_IsValid(T) );
	assert( ppList );

	if ( ! T->PropertyList )
	{
		T->PropertyList = jeProperty_ListCreateEmpty();
		if ( ! T->PropertyList )
		{
			*ppList = NULL;
			return JE_FALSE;
		}
	}

	// set up the properties

	P = jeProperty_GetOrCreate(T->PropertyList,TERRAIN_PROPERTY_HEIGHTMAP);
	if ( ! P )
		return JE_FALSE;
	if ( ! P->FieldName )
	{
		jeProperty_FillString(P,"Heightmap",T->HeightmapName,TERRAIN_PROPERTY_HEIGHTMAP);
	}
	P->Data.Ptr = T->HeightmapName;

/*
	P = jeProperty_GetOrCreate(T->PropertyList,TERRAIN_PROPERTY_VERTEXLIGHTING);
	if ( ! P )
		return JE_FALSE;
	if ( ! P->FieldName )
	{
		jeProperty_FillButton(P,"LightVerts",TERRAIN_PROPERTY_VERTEXLIGHTING);
	}

	P = jeProperty_GetOrCreate(T->PropertyList,TERRAIN_PROPERTY_TEXTURELIGHTING);
	if ( ! P )
		return JE_FALSE;
	if ( ! P->FieldName )
	{
		jeProperty_FillButton(P,"LightTex",TERRAIN_PROPERTY_TEXTURELIGHTING);
	}

	P = jeProperty_GetOrCreate(T->PropertyList,TERRAIN_PROPERTY_NOLIGHTING);
	if ( ! P )
		return JE_FALSE;
	if ( ! P->FieldName )
	{
		jeProperty_FillButton(P,"FullBright",TERRAIN_PROPERTY_NOLIGHTING);
	}
*/

	{
	static const char * LightListStrings[] = { "FullBright", "Vertex", "Texture", "WorldShadowed", NULL };
		P = jeProperty_GetOrCreate(T->PropertyList,TERRAIN_PROPERTY_LIGHTING_LIST);
		if ( ! P )
			return JE_FALSE;
		if ( ! P->FieldName )
		{
			jeProperty_FillCombo(P,"Lighting",(char *)LightListStrings[T->LightListSel],TERRAIN_PROPERTY_LIGHTING_LIST,4,(char **)LightListStrings);
		}
		P->Data.String = (char *)LightListStrings[T->LightListSel];
	}

	P = jeProperty_GetOrCreate(T->PropertyList,TERRAIN_PROPERTY_MAXQUADS);
	if ( ! P )
		return JE_FALSE;
	if ( ! P->FieldName )
	{
		jeProperty_FillInt(P, "MaxQuads", T->MaxQuads, TERRAIN_PROPERTY_MAXQUADS, 50, 4000, 25);
	}
	P->Data.Int = T->MaxQuads;
		
	P = jeProperty_GetOrCreate(T->PropertyList,TERRAIN_PROPERTY_MINERROR);
	if ( ! P )
		return JE_FALSE;
	if ( ! P->FieldName )
	{
		jeProperty_FillFloat(P, "MinError", T->MinError, TERRAIN_PROPERTY_MINERROR, 0.001f, 0.01f, 0.001f);
	}
	P->Data.Float = T->MinError;

	P = jeProperty_GetOrCreate(T->PropertyList,TERRAIN_PROPERTY_TEXDIMLOG2);
	if ( ! P )
		return JE_FALSE;
	if ( ! P->FieldName )
	{
		jeProperty_FillInt(P, "Texlog2",intlog2(T->TexDim), TERRAIN_PROPERTY_TEXDIMLOG2, 0, MAX_TEXDIM_LOG2, 1);
	}
	P->Data.Int = intlog2(T->TexDim);

#if 0 //{ @@ busted

	P = jeProperty_GetOrCreate(T->PropertyList,TERRAIN_PROPERTY_SIZE);
	if ( ! P )
		return JE_FALSE;
	if ( ! P->FieldName )
	{
		jeProperty_FillVec3dGroup(P,"Size",&(T->Size),TERRAIN_PROPERTY_SIZE);
	}
	P->Data.Vector = T->Size;

#else //}{

	P = jeProperty_GetOrCreate(T->PropertyList,TERRAIN_PROPERTY_SIZE_X);
	if ( ! P )
		return JE_FALSE;
	if ( ! P->FieldName )
	{
		//jeProperty_FillFloat(P, "SizeX", T->Size.X, TERRAIN_PROPERTY_SIZE_X, 50.0f, 4000.0f, 10.0f);
		//CyRiuS
		jeProperty_FillFloat(P, "SizeX", T->Size.X, TERRAIN_PROPERTY_SIZE_X, 50.0f, 16384.0f, 10.0f);
	}
	P->Data.Float = T->Size.X;

	P = jeProperty_GetOrCreate(T->PropertyList,TERRAIN_PROPERTY_SIZE_Y);
	if ( ! P )
		return JE_FALSE;
	if ( ! P->FieldName )
	{
		//CyRiuS
		jeProperty_FillFloat(P, "SizeY", T->Size.Y, TERRAIN_PROPERTY_SIZE_Y, 50.0f, 16384.0f, 10.0f);
	}
	P->Data.Float = T->Size.Y;
	
	P = jeProperty_GetOrCreate(T->PropertyList,TERRAIN_PROPERTY_SIZE_Z);
	if ( ! P )
		return JE_FALSE;
	if ( ! P->FieldName )
	{
		//CyRiuS
		jeProperty_FillFloat(P, "SizeZ", T->Size.Z, TERRAIN_PROPERTY_SIZE_Z, 50.0f, 16384.0f, 1.0f);
	}
	P->Data.Float = T->Size.Z;
#endif //}

	*ppList = T->PropertyList;

   return JE_TRUE;
}

jeBoolean	JETCC jeTerrain_GetPropertyListCopy(void * T, jeProperty_List **ppList)
{
	jeTerrain						*Ter = (jeTerrain*)T;

	if ( ! jeTerrain_GetPropertyListPtr(Ter,ppList) )
		return JE_FALSE;

	*ppList = jeProperty_ListCopy(*ppList);
	if ( ! *ppList )
		return JE_FALSE;

   return JE_TRUE;
}

jeBoolean	JETCC jeTerrain_GetProperty(void * T, int32 FieldID, PROPERTY_FIELD_TYPE DataType, jeProperty_Data * pData )
{
   jeProperty_List *pList;
   jeProperty * pP;
	
	if ( ! jeTerrain_GetPropertyListPtr((jeTerrain*)T,&pList) )
		return JE_FALSE;

	pP = jeProperty_ListFindByDataId(pList,FieldID);

	if ( ! pP )
		return JE_FALSE;
	
	*pData = pP->Data;

   return JE_TRUE;
}

jeBoolean	JETCC jeTerrain_SetProperty(void * T, int32 FieldID, PROPERTY_FIELD_TYPE DataType, jeProperty_Data * pData )
{
	jeTerrain							*Ter = (jeTerrain*)T;

	assert( jeTerrain_IsValid(Ter) );

	jeTerrain_DeSelect(Ter);

	switch( FieldID )
	{
		case TERRAIN_PROPERTY_HEIGHTMAP:
		{
			
		jeBitmap * Bmp = NULL;
		jeBoolean Ret;
			Bmp = jeBitmap_CreateFromFileName(NULL,(char *)pData->Ptr);
			if ( ! Bmp )
				return JE_FALSE;
			Ret = jeTerrain_SetHeightmap(Ter,Bmp);
			jeBitmap_Destroy(&Bmp);
			strcpy(Ter->HeightmapName,(char *)pData->Ptr);
		return Ret;
		}

/*
		case TERRAIN_PROPERTY_VERTEXLIGHTING:
			return jeTerrain_SetLightsOnVertsFromWorld(T,T->World);

		case TERRAIN_PROPERTY_TEXTURELIGHTING:
			return jeTerrain_SetLightsInTextureFromWorld(T,T->World);

		case TERRAIN_PROPERTY_NOLIGHTING:
			return jeTerrain_SetDefaultLighting(T);
*/

		case TERRAIN_PROPERTY_LIGHTING_LIST:
			
			#define strmatch(str,vs)	( strnicmp(str,vs,strlen(vs)) == 0 )

			if ( strmatch(pData->String,"Full") )
			{
				Ter->LightListSel = 0;

				return jeTerrain_SetDefaultLighting(Ter);
			}
			else if ( strmatch(pData->String,"vert") )
			{
				Ter->LightListSel = 1;
				return jeTerrain_SetLightsOnVertsFromWorld(Ter,Ter->World);
			}
			else if ( strmatch(pData->String,"tex") )
			{
				Ter->LightListSel = 2;
				return jeTerrain_SetLightsInTextureFromWorld(Ter,Ter->World,JE_TRUE,JE_FALSE);
			}
			else if ( strmatch(pData->String,"world") )
			{
				Ter->LightListSel = 3;
				return jeTerrain_SetLightsInTextureFromWorld(Ter,Ter->World,JE_TRUE,JE_TRUE);
			}
			else
				return JE_FALSE;

		case TERRAIN_PROPERTY_MAXQUADS:
			if ( DataType != PROPERTY_INT_TYPE )
				return JE_FALSE;
			return jeTerrain_SetParameters(Ter, pData->Int, Ter->MinError);

		case TERRAIN_PROPERTY_MINERROR:
			if ( DataType != PROPERTY_FLOAT_TYPE )
				return JE_FALSE;
			return jeTerrain_SetParameters(Ter, Ter->MaxQuads, pData->Float);

		case TERRAIN_PROPERTY_TEXDIMLOG2:
			if ( DataType != PROPERTY_INT_TYPE )
				return JE_FALSE;
			if ( pData->Int < 0 || pData->Int > MAX_TEXDIM_LOG2 )
				return JE_FALSE;

			return jeTerrain_SetTexDim(Ter, 1 << (pData->Int) );

		case TERRAIN_PROPERTY_SIZE:
			return jeTerrain_SetSize(Ter,&(pData->Vector));
		
		case TERRAIN_PROPERTY_SIZE_X:
		{
		jeVec3d NewSize;
			NewSize = Ter->Size;
			NewSize.X = pData->Float;
			return jeTerrain_SetSize(Ter,&NewSize);
		}
		case TERRAIN_PROPERTY_SIZE_Y:
		{
		jeVec3d NewSize;
			NewSize = Ter->Size;
			NewSize.Y = pData->Float;
			return jeTerrain_SetSize(Ter,&NewSize);
		}

		case TERRAIN_PROPERTY_SIZE_Z:
		{
		jeVec3d NewSize;
			NewSize = Ter->Size;
			NewSize.Z = pData->Float;
			return jeTerrain_SetSize(Ter,&NewSize);
		}



		default:
			return JE_FALSE;
	}

   return JE_TRUE;
}

JETAPI jeBoolean JETCC jeTerrain_SetXForm(void * Terrain,const jeXForm3d *pXF)
{
	jeTerrain* T = (jeTerrain *)Terrain;
	
	assert( jeTerrain_IsValid(T) );

	if ( ! jeXForm3d_IsOrthonormal(pXF) )
		return JE_FALSE;

	T->XFTerrainToWorld = *pXF;
	T->Changed = JE_TRUE;

	jeXForm3d_GetTranspose(pXF,&(T->XFWorldToTerrain));

   return JE_TRUE;
}

JETAPI jeBoolean JETCC jeTerrain_GetXForm(const void * Terrain,jeXForm3d *pXF)
{
	jeTerrain *T = (jeTerrain *)Terrain;
	assert( jeTerrain_IsValid(T) );
	*pXF = T->XFTerrainToWorld;
   return JE_TRUE;
}

static int JETCC jeTerrain_GetXFormModFlags ( const void * Instace )
{
   return	JE_OBJECT_XFORM_TRANSLATE | JE_OBJECT_XFORM_ROTATE;
}

#pragma message ("Cross link between Terrain and editor sources")
#include "EditMsg.h"
//#include "..\..\tools\Editor\editorsdk\include\EditMsg.h"

static void jeTerrain_DeSelect(jeTerrain * T)
{
	if ( T->HasSelection )
	{
		T->HasSelection = JE_FALSE;
	}
}

static void jeTerrain_Select(jeTerrain * T,jeVec3d *pWorldVec)
{
   jeVec3d V;
	assert( jeTerrain_IsValid(T) );
	
	jeTerrain_DeSelect(T);

	jeXForm3d_Transform(&(T->XFWorldToTerrain),pWorldVec,&V);

	if ( V.X < 0.0f || V.Y < 0.0f || V.X > T->Size.X || V.Y > T->Size.Y )
		return;

	T->SelectionX = V.X;
	T->SelectionY = V.Y;

	jeTerrain_GetTextureAtXY(T,V.X,V.Y,NULL,&(T->SelectionTexX),&(T->SelectionTexY));

	T->HasSelection = JE_TRUE;
}

static jeBoolean	JETCC jeTerrain_SendMessage	(void * T, int32 Msg, void * Data)
{
	jeTerrain							*Ter = (jeTerrain*)T;

	switch(Msg)
	{

	case JETEDITOR_SELECT3D:
	{
	Select3dContextDef *pContext;
	int OldSelX,OldSelY;

			pContext = (Select3dContextDef *)Data;

			if ( Ter->HasSelection )
			{
				OldSelX = Ter->SelectionTexX;
				OldSelY = Ter->SelectionTexY;
			}

			else
				OldSelX = OldSelY = -1;
			
			jeTerrain_Select(Ter,&(pContext->Impact));

			if ( OldSelX == Ter->SelectionTexX && OldSelY == Ter->SelectionTexY )
				jeTerrain_DeSelect(Ter);	

		return JE_TRUE;
	}

	case JETEDITOR_APPLYMATERIAL:
	{
		if ( Ter->HasSelection )
		{
			jeTerrain_SetATexture(Ter,(jeBitmap *)Data,Ter->SelectionTexX,Ter->SelectionTexY);
			return JE_TRUE;
		}

		return JE_FALSE;
	}
	case JETEDITOR_APPLYMATERIALSPEC:
	{
		if ( Ter->HasSelection )
		{
			jeBitmap* pBitmap;

			pBitmap = jeMaterialSpec_GetLayerBitmap((jeMaterialSpec *)Data, 0);
			jeTerrain_SetATexture(Ter,pBitmap,Ter->SelectionTexX,Ter->SelectionTexY);
			return JE_TRUE;
		}

		return JE_FALSE;
	}

	default:
		return JE_FALSE; // false means did not do anything with the message!!!!
	}
}

JETAPI jeBoolean JETCC jeTerrain_AttachEngine(void *T,jeEngine *Engine)
{
	jeTerrain					*Ter = (jeTerrain*)T;

	assert( Engine );
	assert( jeTerrain_IsValid((jeTerrain*)T) );

	// already the one attached, just ignore the attach request
	if ( Ter->Engine == Engine )
		return JE_TRUE;

	jeTerrain_DetachEngine(Ter,Ter->Engine);

	Ter->Engine = Engine;

	if ( Ter->Engine )
	{
	int i;

		jeEngine_CreateRef(Ter->Engine);

		jeEngine_AddBitmap(Ter->Engine,Ter->NullTexture,JE_ENGINE_BITMAP_TYPE_3D);
		jeEngine_AddBitmap(Ter->Engine,Ter->HiliteTexture,JE_ENGINE_BITMAP_TYPE_3D);

		for(i=0;i<(Ter->TexDim * Ter->TexDim);i++)
		{
			if ( Ter->Textures[i] && ! Ter->RegisteredTexture[i] )
			{
				jeEngine_AddBitmap(Ter->Engine,Ter->Textures[i],JE_ENGINE_BITMAP_TYPE_3D);
				Ter->RegisteredTexture[i] = JE_TRUE;
			}
		}
	}

   return JE_TRUE;
} 

JETAPI jeBoolean JETCC jeTerrain_DetachEngine(void *T,jeEngine *Engine)
{
	jeTerrain						*Ter = (jeTerrain*)T;

	assert( jeTerrain_IsValid(Ter) );
	//assert( T->Engine == Engine );
	
	if ( Ter->Engine )
	{
		int i;
		jeEngine_RemoveBitmap(Ter->Engine,Ter->NullTexture);
		jeEngine_RemoveBitmap(Ter->Engine,Ter->HiliteTexture);
		
		for(i=0;i<(Ter->TexDim * Ter->TexDim);i++)
		{
			if ( Ter->RegisteredTexture[i] )
			{
				jeEngine_RemoveBitmap(Ter->Engine,Ter->Textures[i]);
				Ter->RegisteredTexture[i] = JE_FALSE;
			}
		}
		
		jeEngine_Destroy(&(Ter->Engine));
	}
	
	Ter->Engine = NULL;

   return JE_TRUE;
} 

static jeBoolean JETCC jeTerrain_AttachWorld(void *T,jeWorld *World)
{
	assert( World );
	assert( jeTerrain_IsValid((jeTerrain*)T) );

	((jeTerrain*)T)->World = World;

   return JE_TRUE;
} 

static jeBoolean JETCC jeTerrain_DetachWorld(void *T,jeWorld *World)
{
	assert( World );
	assert( jeTerrain_IsValid((jeTerrain*)T) );

	((jeTerrain*)T)->World = NULL;

   return JE_TRUE;
} 

/*}{******************************************************/

#pragma warning(disable : 4028 4090)
jeObjectDef jeTerrain_ObjectDef =
{
	JE_OBJECT_TYPE_TERRAIN,
	"Terrain",
	JE_OBJECT_VISRENDER, //flags

	jeTerrain_Create,
	jeTerrain_CreateRef,
	jeTerrain_Destroy,

	jeTerrain_AttachWorld,
	jeTerrain_DetachWorld,
	jeTerrain_AttachEngine,
	jeTerrain_DetachEngine,
	NULL, //Attach SoundSystem
	NULL, //Detach SoundSystem

	jeTerrain_ObjectRender,
	jeTerrain_BoxCollision,
	jeTerrain_GetExtBox,

	jeTerrain_CreateFromFile,
	jeTerrain_WriteToFile,

	jeTerrain_GetPropertyListCopy,
	jeTerrain_SetProperty,
	jeTerrain_GetProperty,

	jeTerrain_SetXForm,
	jeTerrain_GetXForm,
	jeTerrain_GetXFormModFlags,

	NULL,NULL,NULL, // children stuff

	NULL, // editdialog
	jeTerrain_SendMessage,
	NULL, // updatetime
	NULL, // duplicate
	NULL,	// ChangeBoxCollision
	NULL,	// GetGlobalPropertyList
	NULL,	// SetGlobalProperty
	jeTerrain_SetRenderNextTime,
};
#pragma warning(default : 4028 4090)

JETAPI void		JETCC jeTerrain_InitObject(const jeTerrain *T,jeObject *O)
{
	assert( jeTerrain_IsValid(T) );	
	assert( O );
	O->Name = NULL;
	O->Methods = &jeTerrain_ObjectDef;
	O->Instance = (void *)T;
	O->RefCnt = 0;
}

JETAPI jeBoolean JETCC jeTerrain_RegisterObjectDef(void)
{
   return jeObject_RegisterGlobalObjectDef( &jeTerrain_ObjectDef );
}

/*******************************************************/


// Krouer: interact from the BSP
JETAPI void	JETCC jeTerrain_SetRenderNextTime(void* T, jeBoolean RenderNext)
{
	((jeTerrain*)T)->RenderNextFlag = RenderNext;
}
