/****************************************************************************************/
/*  PUPPET.C																			*/
/*                                                                                      */
/*  Author: Mike Sandige	                                                            */
/*  Description: Puppet implementation.									.				*/
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
//#define CIRCULAR_SHADOW
#define SHADOW_MAP
//#define PROJECTED_SHADOW

#include <math.h>  //fabs()
#include <assert.h>

#include "XFArray.h"
#include "Puppet.h"
#include "Pose.h"
#include "Errorlog.h"
#include "Ram.h"
#include "TClip.h"

#include "ExtBox.h"
#include "BodyInst.h"

#ifdef PROFILE
//#include "rdtsc.h"
#endif

#include "Bitmap.h"
#include "Bitmap._h"

#include "Engine._h" // for Engine->DebugInfo
#include "Camera._h"

#include "jeMaterial._h"

#define PUPPET_DEFAULT_MAX_DYNAMIC_LIGHTS 3
#define PUPPET_DEFAULT_MAX_STATIC_LIGHTS 3

typedef struct jePuppet_Color
{
	jeFloat				Red,Green,Blue;
} jePuppet_Color;

typedef struct jePuppet_Material
{
	jePuppet_Color		 Color;
	jeBoolean			 UseTexture;
	//jeBitmap			*Bitmap;
	jeMaterialSpec		*Material;
	jeUVMapper			Mapper;
	const char			*TextureName;
	const char			*AlphaName;
} jePuppet_Material;

#define MAX_DYNAMIC_LIGHTS			(32)
#define MAX_STATIC_LIGHTS				(32)

typedef struct
{
	jeVec3d			Normal;
	jePuppet_Color	Color;
	jeFloat			Distance;
	jeFloat			Radius;
} jePuppet_Light;

typedef struct
{
	jePuppet_Light DLights[MAX_DYNAMIC_LIGHTS];
	jePuppet_Light SLights[MAX_STATIC_LIGHTS];
	int DLightCount;
	int SLightCount;
} jePuppet_BoneLight;

typedef struct jePuppet
{
	jeVFile *			 TextureFileContext;
	//jeXFArray			*JointTransforms;	
	jeBodyInst			*BodyInstance;
	int					 MaterialCount;
	jePuppet_Material	*MaterialArray;
	int					 MaxDynamicLightsToUse;
	int						MaxStaticLightsToUse;
	int					 LightReferenceBoneIndex;
		
	jeVec3d				 FillLightNormal;
	jePuppet_Color		 FillLightColor;			// 0..255
	jeBoolean			 UseFillLight;				// use fill light normal
	
	jePuppet_Color		 AmbientLightIntensity;		// 0..1
	jeBoolean			 AmbientLightFromFloor;		// use local lighting from floor

	jeBoolean			 PerBoneLighting;

// @@
	// for the case of non- per-bone lighting
	jePuppet_Light	SLights[MAX_STATIC_LIGHTS]; // cached static lights
	int							SLightCount; // cached static light count

	// for the case of per-bone lighting
	jePuppet_BoneLight *BoneLightArray;
	int BoneLightArraySize;

	jeBoolean			 DoShadow;
	jeFloat				 ShadowScale;
	const jeMaterialSpec *ShadowMap;
	int					 ShadowBoneIndex;

	jeEngine*			pEngine;

//	[MacroArt::Begin]
//	Thanks Dee(cryscan@home.net)	
	float				 fOverallAlpha;
//	[MacroArt::End]

} jePuppet;

typedef struct
{
	jeBoolean		UseFillLight;
	jeVec3d			FillLightNormal;
	jePuppet_Color	MaterialColor;
	jePuppet_Color  FillLightColor;
	jePuppet_Color	Ambient;
	jeVec3d			SurfaceNormal;
	jePuppet_Light	DLights[MAX_DYNAMIC_LIGHTS];
	int				DLightCount;
	jeBoolean		PerBoneLighting;
} jePuppet_LightParamGroup;

//	[MacroArt::Begin]

float JETCF jePuppet_GetAlpha(const jePuppet *P)
{
	assert( P );
	return P->fOverallAlpha;
}

void JETCF jePuppet_SetAlpha(jePuppet *P, float Alpha)
{
	assert( P );
	P->fOverallAlpha = Alpha;
}

jeEngine* JETCF jePuppet_GetEngine(jePuppet *P)
{
	return P->pEngine;
}

//	[MacroArt::End]

// Local info stored across multiple puppets to avoid resource waste.

jePuppet_LightParamGroup jePuppet_StaticLightGrp;
/*
jePuppet_BoneLight		 *jePuppet_StaticBoneLightArray=NULL;
int						  jePuppet_StaticBoneLightArraySize=0;
*/
int						  jePuppet_StaticPuppetCount=0;
int						  jePuppet_StaticFlags[2]={1768710981,560296816};

static jeBoolean JETCF jePuppet_FetchTextures(jePuppet *P, const jeBody *B)
{
	int i;
	assert( P );
	
	P->MaterialCount = jeBody_GetMaterialCount(B);
	if (P->MaterialCount <= 0)
	{
		return JE_TRUE;
	}
	
	P->MaterialArray = JE_RAM_ALLOCATE_ARRAY_CLEAR(jePuppet_Material, P->MaterialCount);
	if (P->MaterialArray == NULL)
	{
		jeErrorLog_Add(JE_ERR_MEMORY_RESOURCE,"jePuppet_FetchTextures: Failed to allocate puppet material array");
		return JE_FALSE;
	}
	
	for (i=0; i<P->MaterialCount; i++)
	{
		const char *Name;
		jeMaterialSpec *Bitmap;
		jeUVMapper Mapper;
		jePuppet_Material *M;

		M = P->MaterialArray + i;

		jeBody_GetMaterial( B, i, &(Name), &(Bitmap),
						&(M->Color.Red),&(M->Color.Green),&(M->Color.Blue), 
						&Mapper);

		if (Bitmap == NULL )
		{
			M->Material     = NULL;
			M->UseTexture = JE_FALSE;
		}
		else
		{
			M->UseTexture = JE_TRUE;
			assert( P->pEngine );

			M->Material = Bitmap;
			jeMaterialSpec_CreateRef(Bitmap);

/*
			if ( ! jeEngine_AddBitmap(P->pEngine,Bitmap, JE_ENGINE_BITMAP_TYPE_3D) )
			{
				jeErrorLog_AddString(JE_ERR_SUBSYSTEM_FAILURE,"jePuppet_FetchTextures : Engine_AddBitmap", NULL);
				jeRam_Free(P->MaterialArray);
				P->MaterialArray = NULL;
				P->MaterialCount = 0;
				return JE_FALSE;
			}
*/
		}
	}

	return JE_TRUE;
}	

int JETCF jePuppet_GetMaterialCount( jePuppet *P )
{
	assert( P );
	return P->MaterialCount;
}

jeBoolean     jePuppet_GetMaterial( jePuppet *P, int MaterialIndex,
									jeMaterialSpec **Bitmap, 
									jeFloat *Red, jeFloat *Green, jeFloat *Blue, jeUVMapper * pMapper)
{
	assert( P      );
	assert( Red    );
	assert( Green  );
	assert( Blue   );
	assert(pMapper);
	assert( Bitmap );
	assert( MaterialIndex >= 0 );
	assert( MaterialIndex < P->MaterialCount );

	{
		jePuppet_Material *M = &(P->MaterialArray[MaterialIndex]);
		*Bitmap = M->Material;
		*Red    = M->Color.Red;
		*Green  = M->Color.Green;
		*Blue   = M->Color.Blue;
		*pMapper = M->Mapper;
#ifdef _DEBUG
		if ( M->Material )
			assert(M->UseTexture);
#endif
	}

	return JE_TRUE;
}


jeBoolean	jePuppet_SetMaterial(jePuppet *P, int MaterialIndex, jeMaterialSpec *Bitmap, 
								 jeFloat Red, jeFloat Green, jeFloat Blue, 
								 jeUVMapper Mapper)
{
	assert( P );
	assert( MaterialIndex >= 0 );
	assert( MaterialIndex < P->MaterialCount );

	{
		jeMaterialSpec * OldBitmap;
		jePuppet_Material *M = P->MaterialArray + MaterialIndex;

		OldBitmap = M->Material;

		M->Material		= Bitmap;
		M->Color.Red    = Red;
		M->Color.Green  = Green;
		M->Color.Blue   = Blue;
		M->Mapper = Mapper;

		if ( OldBitmap != Bitmap ) 
		{		
/*
			if ( OldBitmap )
			{
				assert( M->UseTexture );		
				jeEngine_RemoveBitmap( P->pEngine, OldBitmap );
				jeMaterialSpec_Destroy( &(OldBitmap) );
			}
*/			
			M->UseTexture = JE_FALSE;

			if ( Bitmap )
			{
				jeMaterialSpec_CreateRef(Bitmap);
						
				M->UseTexture = JE_TRUE;

/*
				if ( ! jeEngine_AddBitmap(P->pEngine,Bitmap, JE_ENGINE_BITMAP_TYPE_3D) )
				{
					jeErrorLog_AddString(JE_ERR_SUBSYSTEM_FAILURE,"jePuppet_SetMaterial : Engine_AddBitmap", NULL);
					return JE_FALSE;
				}
*/
			}
		}
	}

	return JE_TRUE;
}
	

jePuppet* JETCF jePuppet_Create(jeVFile *TextureFS, const jeBody *B, jeEngine *pEngine)
{
	jePuppet *P;

	assert( jeBody_IsValid(B)!=JE_FALSE );
	
	P = JE_RAM_ALLOCATE_STRUCT_CLEAR(jePuppet);
	if (P==NULL)
	{
		jeErrorLog_Add(JE_ERR_MEMORY_RESOURCE,"jePuppet_Create: Failed to allocate instance");
		return NULL;
	}

	//P->JointTransforms = NULL;
	P->BodyInstance = NULL;
	P->MaxDynamicLightsToUse = PUPPET_DEFAULT_MAX_DYNAMIC_LIGHTS;
	P->MaxStaticLightsToUse = PUPPET_DEFAULT_MAX_STATIC_LIGHTS;
	P->LightReferenceBoneIndex = JE_POSE_ROOT_JOINT;

	P->FillLightNormal.X = -0.2f;
	P->FillLightNormal.Y = 1.0f;
	P->FillLightNormal.Z = 0.4f;
	jeVec3d_Normalize(&(P->FillLightNormal));
	P->FillLightColor.Red    = 0.25f;
	P->FillLightColor.Green  = 0.25f;
	P->FillLightColor.Blue   = 0.25f;
	P->UseFillLight = JE_TRUE;

	P->AmbientLightIntensity.Red   = 0.1f;
	P->AmbientLightIntensity.Green = 0.1f;
	P->AmbientLightIntensity.Blue  = 0.1f;
	P->AmbientLightFromFloor = JE_TRUE;

	P->DoShadow = JE_FALSE;
	P->ShadowScale = 0.0f;
	P->ShadowBoneIndex =  JE_POSE_ROOT_JOINT;
	P->TextureFileContext = TextureFS;

	P->BoneLightArray = NULL;
	P->BoneLightArraySize = 0;
//	[MacroArt::Begin]
	P->fOverallAlpha=255.0f;
//	[MacroArt::End]

// @@
	P->pEngine = pEngine;
				
	if (jePuppet_FetchTextures(P,B)==JE_FALSE)
	{
		jeRam_Free(P);
		return NULL;
	}

	P->BodyInstance = jeBodyInst_Create(B);
	if (P->BodyInstance == NULL)
	{
		jeErrorLog_Add(JE_ERR_MEMORY_RESOURCE,"jePuppet_Create: Failed to allocate body");
		jePuppet_Destroy( &P );
		return NULL;
	}

	return P;
}


void JETCF jePuppet_Destroy(jePuppet **P)
{
	assert( P  );
	assert( *P );
	if ( (*P)->BodyInstance )
	{
		jeBodyInst_Destroy( &((*P)->BodyInstance) );
		(*P)->BodyInstance = NULL;
	}
	if ( (*P)->MaterialArray )
	{
		jePuppet_Material *M;
		int i;

		for (i=0; i<(*P)->MaterialCount; i++)
		{
			M = &((*P)->MaterialArray[i]);
			if (M->UseTexture )
			{					
				assert( M->Material );
				//jeEngine_RemoveBitmap( (*P)->pEngine, M->Bitmap );
				jeMaterialSpec_Destroy( &(M->Material) );
				M->UseTexture = JE_FALSE;
			}
		}


		jeRam_Free( (*P)->MaterialArray );
		(*P)->BodyInstance = NULL;
	}
	if ( (*P)->ShadowMap )
	{
		jeBitmap_Destroy((jeBitmap **)&((*P)->ShadowMap));
		(*P)->ShadowMap = NULL;
	}

	if ( (*P)->BoneLightArray!=NULL)
		{
			jeRam_Free((*P)->BoneLightArray);
		}

	jeRam_Free( (*P) );
	*P = NULL;

	// clean up any shared resources.
	jePuppet_StaticPuppetCount--;
	if (jePuppet_StaticPuppetCount==0)
	{
		/*
		if (jePuppet_StaticBoneLightArray!=NULL)
			jeRam_Free(jePuppet_StaticBoneLightArray);
		jePuppet_StaticBoneLightArray=NULL;
		jePuppet_StaticBoneLightArraySize = 0;
		*/
	}	
}


void jePuppet_GetLightingOptions(const jePuppet *P,
	jeBoolean *UseFillLight,
	jeVec3d *FillLightNormal,
	jeFloat *FillLightRed,				
	jeFloat *FillLightGreen,				
	jeFloat *FillLightBlue,				
	jeFloat *AmbientLightRed,			
	jeFloat *AmbientLightGreen,			
	jeFloat *AmbientLightBlue,			
	jeBoolean *UseAmbientLightFromFloor,
	int32 *MaximumDynamicLightsToUse,
	int32 *MaximumStaticLightsToUse,
	int32 *LightReferenceBoneIndex,
	jeBoolean *PerBoneLighting
	)
{
	jeFloat Scaler;
	assert( P != NULL);
	assert( UseFillLight );
	assert( FillLightNormal );
	assert( FillLightRed );	
	assert( FillLightGreen );	
	assert( FillLightBlue );	
	assert( AmbientLightRed );
	assert( AmbientLightGreen );			
	assert( AmbientLightBlue );			
	assert( UseAmbientLightFromFloor );
	assert( MaximumDynamicLightsToUse );	
	assert( LightReferenceBoneIndex );
		
	*UseFillLight = P->UseFillLight;

	*FillLightNormal = P->FillLightNormal;
	
	Scaler = 255.0f;
	*FillLightRed   = P->FillLightColor.Red * Scaler;
	*FillLightGreen = P->FillLightColor.Green * Scaler;
	*FillLightBlue  = P->FillLightColor.Blue * Scaler;
	
	*AmbientLightRed    = P->AmbientLightIntensity.Red * Scaler;
	*AmbientLightGreen  = P->AmbientLightIntensity.Green * Scaler;
	*AmbientLightBlue   = P->AmbientLightIntensity.Blue * Scaler;
	
	*UseAmbientLightFromFloor  = P->AmbientLightFromFloor;
	*MaximumDynamicLightsToUse = P->MaxDynamicLightsToUse;
	*MaximumStaticLightsToUse = P->MaxStaticLightsToUse;
	*LightReferenceBoneIndex   = P->LightReferenceBoneIndex;
	*PerBoneLighting		   = P->PerBoneLighting;
}	

void jePuppet_SetLightingOptions(jePuppet *P,
	jeBoolean UseFillLight,
	const jeVec3d *FillLightNormal,
	jeFloat FillLightRed,				// 0 .. 255
	jeFloat FillLightGreen,				// 0 .. 255
	jeFloat FillLightBlue,				// 0 .. 255
	jeFloat AmbientLightRed,			// 0 .. 255
	jeFloat AmbientLightGreen,			// 0 .. 255
	jeFloat AmbientLightBlue,			// 0 .. 255
	jeBoolean UseAmbientLightFromFloor,
	int MaximumDynamicLightsToUse,		// 0 for none
	int MaximumStaticLightsToUse, // 0 for none
	int LightReferenceBoneIndex,
	jeBoolean PerBoneLighting
	)
{
	jeFloat Scaler;
	assert( P!= NULL);
	assert( FillLightNormal );
	assert( jeVec3d_IsNormalized(FillLightNormal) );
	assert( MaximumDynamicLightsToUse >= 0 );
	assert( (LightReferenceBoneIndex >=0) || (LightReferenceBoneIndex==JE_POSE_ROOT_JOINT));
		
	P->UseFillLight = UseFillLight;

	P->FillLightNormal = *FillLightNormal;
	
	Scaler = 1.0f/255.0f;

	P->FillLightColor.Red   = FillLightRed   * Scaler;
	P->FillLightColor.Green = FillLightGreen * Scaler;
	P->FillLightColor.Blue  = FillLightBlue  * Scaler;
	
	P->AmbientLightIntensity.Red   = AmbientLightRed   * Scaler;
	P->AmbientLightIntensity.Green = AmbientLightGreen * Scaler;
	P->AmbientLightIntensity.Blue  = AmbientLightBlue  * Scaler;
	
	P->AmbientLightFromFloor =UseAmbientLightFromFloor;
	P->MaxDynamicLightsToUse = MaximumDynamicLightsToUse;
	P->MaxStaticLightsToUse = MaximumStaticLightsToUse;
	P->LightReferenceBoneIndex = LightReferenceBoneIndex;
	P->PerBoneLighting		 = 	PerBoneLighting;
}	

// LP = array of lights
// ReferencePoint = world space location of attachment point
static int JETCC jePuppet_PrepDynamicLights(const jePuppet *P, 
	const jeWorld *World,
	jePuppet_Light *LP,
	const jeVec3d *ReferencePoint)
{
	int				i,j,cnt;
	jeChain			*DLightChain;
	jeChain_Link	*Link;


	assert( P );
	assert( LP );

	DLightChain = jeWorld_GetDLightChain(World);
	
	cnt=0;

	for (Link = jeChain_GetFirstLink(DLightChain); Link; Link = jeChain_LinkGetNext(Link))
	{
		jeLight		*L;
		jeVec3d		Position; 
		jeVec3d		Color;
		jeVec3d		Normal;
		jeFloat		Radius; 
		jeFloat		Brightness;
		uint32		Flags;

		L = (jeLight*)jeChain_LinkGetLinkData(Link);

		if (!jeLight_GetAttributes(	L, &Position,&Color,&Radius,&Brightness, &Flags))
		{
			jeErrorLog_AddString(JE_ERR_SUBSYSTEM_FAILURE,"jePuppet_PrepDynamicLights: failed to get light attributes",NULL);
			continue;
		}

		if (!(Flags & JE_LIGHT_FLAG_FAST_LIGHTING_MODEL))
			continue;

		jeVec3d_Subtract(&Position,ReferencePoint,&Normal);

		LP[cnt].Distance =	Normal.X * Normal.X + 
							Normal.Y * Normal.Y +
							Normal.Z * Normal.Z;

		if (LP[cnt].Distance < Radius*Radius)
		{
			LP[cnt].Color.Red   = Color.X;
			LP[cnt].Color.Green = Color.Y;
			LP[cnt].Color.Blue  = Color.Z;
			LP[cnt].Radius = Radius;
			LP[cnt].Normal = Normal;
			cnt++;
		}
	}

	// sort dynamic lights by distance (squared)
	for (i=0; i<cnt; i++)
		for (j=0; j<cnt-1; j++)
			{
				if (LP[j].Distance > LP[j+1].Distance)
					{
						jePuppet_Light Swap = LP[j];
						LP[j] = LP[j+1];
						LP[j+1] = Swap;
					}
			}

	if (cnt > P->MaxDynamicLightsToUse)
		cnt = P->MaxDynamicLightsToUse;

	// go back and finish setting up closest lights
	for (i=0; i<cnt; i++)
		{
			jeFloat Distance = (jeFloat)sqrt(LP[i].Distance);
			jeFloat OneOverDistance;
			jeFloat Scale;
			if (Distance < 1.0f)
				Distance = 1.0f;
			OneOverDistance = 1.0f / Distance;
			LP[i].Normal.X *= OneOverDistance;
			LP[i].Normal.Y *= OneOverDistance;
			LP[i].Normal.Z *= OneOverDistance;

			LP[i].Distance = Distance;

			//assert( Distance  < LP[i].Radius );

			Scale = 1.0f - Distance / LP[i].Radius ;
			Scale *= (1.0f/255.0f);
			LP[i].Color.Red   *= Scale;
			LP[i].Color.Green *= Scale;
			LP[i].Color.Blue  *= Scale;
		}
			
	return cnt;			
}

// @@
// LP = array of lights
// ReferencePoint = world space location of attachment point
// recache = indicate whether to scan thru all static lights to see which are
//						closest to actor

static int JETCC jePuppet_PrepStaticLights(const jePuppet *P, 
	const jeWorld *World,
	jePuppet_Light *LP,
	const jeVec3d *ReferencePoint)
{
	int				i,j, cnt;
	jeChain			*SLightChain;
	jeChain_Link	*Link;


	assert( P );
	assert( LP );

	
#pragma message("**************************************************************************")
#pragma message("puppet.c: jePuppet_PrepStaticLights()")
#pragma message("Instead of cycling thru all the static lights in the world, find out what area the")
#pragma message("ReferencePoint is in and use the lights that are in that (and possibly)")
#pragma message("neighboring areas!")
#pragma message("**************************************************************************")

	SLightChain = jeWorld_GetLightChain(World);
	
	cnt=0;

	for (Link = jeChain_GetFirstLink(SLightChain); Link; Link = jeChain_LinkGetNext(Link))
	{
		jeLight		*L;
		jeVec3d		Position; 
		jeVec3d		Color;
		jeVec3d		Normal;
		jeFloat		Radius; 
		jeFloat		Brightness;
		uint32		Flags;

		L = (jeLight*)jeChain_LinkGetLinkData(Link);

		if (!jeLight_GetAttributes(	L, &Position,&Color,&Radius,&Brightness, &Flags))
		{
			jeErrorLog_AddString(JE_ERR_SUBSYSTEM_FAILURE,"jePuppet_PrepStaticLights: failed to get light attributes",NULL);
			continue;
		}

		/*
		if (!(Flags & JE_LIGHT_FLAG_FAST_LIGHTING_MODEL))
			continue;
		*/

		Normal.X = Position.X - ReferencePoint->X;
		Normal.Y = Position.Y - ReferencePoint->Y;
		Normal.Z = Position.Z - ReferencePoint->Z;

		LP[cnt].Distance =	Normal.X * Normal.X + 
							Normal.Y * Normal.Y +
							Normal.Z * Normal.Z;

		if (LP[cnt].Distance < Radius*Radius)
			{
				LP[cnt].Color.Red   = Color.X;
				LP[cnt].Color.Green = Color.Y;
				LP[cnt].Color.Blue  = Color.Z;
				LP[cnt].Radius = Radius;
				LP[cnt].Normal = Normal;
				cnt++;
			}
	}

	// sort static lights by distance (squared)
	for (i = 0; i < cnt; i++)
		for (j = 0; j < (cnt - 1); j++)
			{
				if (LP[j].Distance > LP[j+1].Distance)
					{
						jePuppet_Light Swap = LP[j];
						LP[j] = LP[j+1];
						LP[j+1] = Swap;
					}
			}

	if (cnt > P->MaxStaticLightsToUse)
		cnt = P->MaxStaticLightsToUse;

	// go back and finish setting up closest static lights
	for (i = 0; i < cnt; i ++)
		{
			jeFloat Distance = (jeFloat)sqrt(LP[i].Distance);
			jeFloat OneOverDistance;
			jeFloat Scale;

			if (Distance < 1.0f)
				Distance = 1.0f;
			OneOverDistance = 1.0f / Distance;
			LP[i].Normal.X *= OneOverDistance;
			LP[i].Normal.Y *= OneOverDistance;
			LP[i].Normal.Z *= OneOverDistance;

			LP[i].Distance = Distance;

			//assert( Distance  < LP[i].Radius );

			Scale = 1.0f - Distance / LP[i].Radius ;
			Scale *= (1.0f/255.0f);
			LP[i].Color.Red   *= Scale;
			LP[i].Color.Green *= Scale;
			LP[i].Color.Blue  *= Scale;
		}

	return cnt;
}

	
static void JETCC jePuppet_ComputeAmbientLight(
		const jePuppet *P, 
		jePuppet_Color *Ambient,
		const jeVec3d *ReferencePoint)
{
	assert( P );
	assert( Ambient );

#if 0
	if (P->AmbientLightFromFloor != JE_FALSE)
		{
			#define JE_PUPPET_MAX_AMBIENT (0.3f)
			int32			Node, Plane, i;
			jeVec3d			Pos1, Pos2, Impact;
			GFX_Node		*GFXNodes;
			Surf_SurfInfo	*Surf;
			JE_RGBA			RGBA;
			jeBoolean		Col1, Col2;
			
			GFXNodes = World->CurrentBSP->BSPData.GFXNodes;
			
			Pos1 = *ReferencePoint;
			
			Pos2 = Pos1;

			Pos2.Y -= 30000.0f;

			// Get shadow hit plane impact point
			Col1 = Trace_WorldCollisionExact2((jeWorld*)World, &Pos1, &Pos1, &Impact, &Node, &Plane, NULL);
			Col2 = Trace_WorldCollisionExact2((jeWorld*)World, &Pos1, &Pos2, &Impact, &Node, &Plane, NULL);

			// Now find the color of the mesh by getting the lightmap point he is standing on...
			if (!Col1 && Col2)
				{
					Surf = &(World)->CurrentBSP->SurfInfo[GFXNodes[Node].FirstFace];
					if (Surf->LInfo.Face<0)
						{	// FIXME?  surface has no light...
							Ambient->Red = Ambient->Green = Ambient->Blue = 0.0f;
							return;
						}

					for (i=0; i< GFXNodes[Node].NumFaces; i++)
						{
							if (Surf_InSurfBoundingBox(Surf, &Impact, 20.0f))
								{
									Light_SetupLightmap(&Surf->LInfo, NULL);			

									if (Light_GetLightmapRGB(Surf, &Impact, &RGBA))
										{
											jeFloat Scale = 1.0f / 255.0f;
											Ambient->Red   = RGBA.r * Scale;
											Ambient->Green = RGBA.g * Scale;
											Ambient->Blue  = RGBA.b * Scale;
											if (Ambient->Red > JE_PUPPET_MAX_AMBIENT) 
												{
													Ambient->Red = JE_PUPPET_MAX_AMBIENT;
												}
											if (Ambient->Green > JE_PUPPET_MAX_AMBIENT) 
												{
													Ambient->Green = JE_PUPPET_MAX_AMBIENT;
												}
											if (Ambient->Blue > JE_PUPPET_MAX_AMBIENT) 
												{
													Ambient->Blue = JE_PUPPET_MAX_AMBIENT;
												}
											break;
										}
								}
							Surf++;
						}
				}
			else
				{
					*Ambient = P->AmbientLightIntensity;
				}
		}
	else
#endif
	{
		*Ambient = P->AmbientLightIntensity;
	}
}


// @@
static void JETCC jePuppet_SetVertexColor(jePuppet* P,
	jeLVertex *v,int BoneIndex)
{
	jeFloat RedIntensity,GreenIntensity,BlueIntensity;
	jeFloat Color;						
	int l;
	jeVec3d surfaceNormal;

	assert(v != NULL);
	
	RedIntensity   = jePuppet_StaticLightGrp.Ambient.Red;
	GreenIntensity = jePuppet_StaticLightGrp.Ambient.Green;
	BlueIntensity  = jePuppet_StaticLightGrp.Ambient.Blue;

	surfaceNormal = jePuppet_StaticLightGrp.SurfaceNormal;

	if (jePuppet_StaticLightGrp.UseFillLight)
	{
		jeFloat Intensity;
		Intensity = jePuppet_StaticLightGrp.FillLightNormal.X * surfaceNormal.X + 
					jePuppet_StaticLightGrp.FillLightNormal.Y * surfaceNormal.Y + 
					jePuppet_StaticLightGrp.FillLightNormal.Z * surfaceNormal.Z;
		if (Intensity > 0.0)
		{
			RedIntensity   += Intensity * jePuppet_StaticLightGrp.FillLightColor.Red;
			GreenIntensity += Intensity * jePuppet_StaticLightGrp.FillLightColor.Green;
			BlueIntensity  += Intensity * jePuppet_StaticLightGrp.FillLightColor.Blue;
		}
	}

	if (jePuppet_StaticLightGrp.PerBoneLighting)
	{
		jePuppet_BoneLight *L;

		L = &P->BoneLightArray[BoneIndex];

		// accumulate dynamic lighting
		for (l = 0; l < L->DLightCount; l ++)
		{
			jeVec3d *LightNormal;
			float Intensity;
		
			LightNormal = &(L->DLights[l].Normal);

			Intensity=	LightNormal->X * surfaceNormal.X + 
						LightNormal->Y * surfaceNormal.Y + 
						LightNormal->Z * surfaceNormal.Z;
			if (Intensity > 0.0f)
			{
				RedIntensity   += Intensity * L->DLights[l].Color.Red;
				GreenIntensity += Intensity * L->DLights[l].Color.Green;
				BlueIntensity  += Intensity * L->DLights[l].Color.Blue;
			}
		}

		// accumulate static lighting
		for (l = 0; l < L->SLightCount; l ++)
		{
			jeVec3d *LightNormal;
			float Intensity;
		
			LightNormal = &(L->SLights[l].Normal);

			Intensity=	LightNormal->X * surfaceNormal.X + 
						LightNormal->Y * surfaceNormal.Y + 
						LightNormal->Z * surfaceNormal.Z;
			if (Intensity > 0.0f)
			{
				RedIntensity   += Intensity * L->SLights[l].Color.Red;
				GreenIntensity += Intensity * L->SLights[l].Color.Green;
				BlueIntensity  += Intensity * L->SLights[l].Color.Blue;
			}
		}
	}
	else // not doing per-bone lighting
	{
		// accumulate dynamic lighting
		for (l = 0; l < jePuppet_StaticLightGrp.DLightCount; l ++)
		{
			jeVec3d *LightNormal;
			float Intensity;
		
			LightNormal = &(jePuppet_StaticLightGrp.DLights[l].Normal);

			Intensity=	LightNormal->X * surfaceNormal.X + 
						LightNormal->Y * surfaceNormal.Y + 
						LightNormal->Z * surfaceNormal.Z;
			if (Intensity > 0.0f)
			{
				RedIntensity   += Intensity * jePuppet_StaticLightGrp.DLights[l].Color.Red;
				GreenIntensity += Intensity * jePuppet_StaticLightGrp.DLights[l].Color.Green;
				BlueIntensity  += Intensity * jePuppet_StaticLightGrp.DLights[l].Color.Blue;
			}
		}

		// accumulate static lighting
		for (l = 0; l < P->SLightCount; l ++)
		{
			jeVec3d *LightNormal;
			float Intensity;
		
			LightNormal = &P->SLights[l].Normal;

			Intensity=	LightNormal->X * surfaceNormal.X + 
						LightNormal->Y * surfaceNormal.Y + 
						LightNormal->Z * surfaceNormal.Z;
			if (Intensity > 0.0f)
			{
				RedIntensity   += Intensity * P->SLights[l].Color.Red;
				GreenIntensity += Intensity * P->SLights[l].Color.Green;
				BlueIntensity  += Intensity * P->SLights[l].Color.Blue;
			}
		}
	}

	Color = jePuppet_StaticLightGrp.MaterialColor.Red * RedIntensity;
	v->r = JE_CLAMP(Color, 0.0f, 255.0f);

	Color = jePuppet_StaticLightGrp.MaterialColor.Green * GreenIntensity;
	v->g = JE_CLAMP(Color, 0.0f, 255.0f);

	Color = jePuppet_StaticLightGrp.MaterialColor.Blue * BlueIntensity;
	v->b = JE_CLAMP(Color, 0.0f, 255.0f);
}

#pragma message ("Make a jePuppet_SetShadowPosition(...) ")

#pragma warning (disable:4100)
static void JETCC jePuppet_DrawShadow(const jePuppet *P, 
						const jePose *Joints, 
						jeEngine *Engine, 
						const jeCamera *Camera)
{
#if 0
	jeLVertex v[3];
	
	jeVec3d		Impact;
	jeXForm3d	RootTransform;
	
	assert( P );
	assert( Camera );
	assert( Joints );

	assert( (P->ShadowBoneIndex < jePose_GetJointCount(Joints)) || (P->ShadowBoneIndex ==JE_POSE_ROOT_JOINT));
	assert( (P->ShadowBoneIndex >=0)					    	|| (P->ShadowBoneIndex ==JE_POSE_ROOT_JOINT));

	jePose_GetJointTransform(Joints,P->ShadowBoneIndex,&RootTransform);
	
	{
		GFX_Plane		Plane;
		jeVec3d			Pos1, Pos2;
		GFX_Node		*GFXNodes;
		jeWorld_Model	*Model;
		Mesh_RenderQ	*Mesh;
		jeActor         *Actor;
		jeBoolean		GoodImpact;

			
		GFXNodes = (World)->CurrentBSP->BSPData.GFXNodes;
		
		Pos1 = RootTransform.Translation;
			
		Pos2 = Pos1;

		Pos2.Y -= 30000.0f;

		// Get shadow hit plane impact point
		GoodImpact = Trace_WorldCollisionExact(World, 
									&Pos1,&Pos2,JE_COLLIDE_MODELS,&Impact,&Plane,&Model,&Mesh,&Actor,0, NULL, NULL);

	}
	Impact.Y += 1.0f;

	v[0].r = v[0].b = v[0].g = 0.0f;
	v[1].r = v[1].b = v[1].g = 0.0f;
	v[2].r = v[2].b = v[2].g = 0.0f;
	
#ifdef SHADOW_MAP
	{
		int i;
		jeLVertex s[4];
		jeVec3d ws[4];
		jeVec3d In,Left;
		jeVec3d Up;
		jeVec3d Zero = {0.0f,0.0f,0.0f};
		
		jeVec3d_Subtract(&Impact,&(RootTransform.Translation),&Up);
		jeVec3d_Normalize(&Up);
		jeVec3d_CrossProduct(&(Plane.Normal),&Up,&Left);
		if (jeVec3d_Compare(&Left,&Zero,0.001f)!=JE_FALSE)
			{
				jeXForm3d_GetLeft(&(RootTransform),&Left);
			}
		jeVec3d_CrossProduct(&Left,&(Plane.Normal),&In);

		jeVec3d_Normalize(&Left);
		jeVec3d_Normalize(&In);

		s[0].r = s[0].b = s[0].g = 0.0f;
		s[1].r = s[1].b = s[1].g = 0.0f;
		s[2].r = s[2].b = s[2].g = 0.0f;
		s[3].r = s[3].b = s[3].g = 0.0f;

		jeVec3d_Scale(&In  ,P->ShadowScale,&In);
		jeVec3d_Scale(&Left,P->ShadowScale,&Left);

		s[0].a = s[1].a = s[2].a = s[3].a  = 160.0f;

		s[0].u = 0.0f; s[0].v = 0.0f;
		s[1].u = 1.0f; s[1].v = 0.0f;
		s[2].u = 1.0f; s[2].v = 1.0f;
		s[3].u = 0.0f; s[3].v = 1.0f;
		ws[0].Y = ws[1].Y = ws[2].Y = ws[3].Y = Impact.Y;

		ws[0].X = RootTransform.Translation.X + Left.X - In.X;
		ws[0].Z = RootTransform.Translation.Z + Left.Z - In.Z;

		ws[1].X = RootTransform.Translation.X - Left.X - In.X;
		ws[1].Z = RootTransform.Translation.Z - Left.Z - In.Z;

		ws[2].X = RootTransform.Translation.X - Left.X + In.X;
		ws[2].Z = RootTransform.Translation.Z - Left.Z + In.Z;
		
		ws[3].X = RootTransform.Translation.X + Left.X + In.X;
		ws[3].Z = RootTransform.Translation.Z + Left.Z + In.Z;

		for (i=0; i<4; i++)
			{
				jeCamera_Transform(Camera,&ws[i],&ws[i]);
				jeCamera_Project(Camera,&ws[i],&ws[i]);
			}

		
		s[0].X = ws[0].X; s[0].Y = ws[0].Y; s[0].Z = ws[0].Z;
		s[1].X = ws[1].X; s[1].Y = ws[1].Y; s[1].Z = ws[1].Z;
		s[2].X = ws[2].X; s[2].Y = ws[2].Y; s[2].Z = ws[2].Z;
		s[3].X = ws[3].X; s[3].Y = ws[3].Y; s[3].Z = ws[3].Z;
		
		jeTClip_SetTexture(P->ShadowMap);

		jeTClip_Triangle(s);
		s[1] = s[2];
		s[2] = s[3];

		jeTClip_Triangle(s);
		
	}
#endif
	
#ifdef CIRCULAR_SHADOW
	v[0].a = v[1].a = v[2].a = 160.0f;
	v[0].u = v[1].u = v[2].u = 0.5f;
	v[0].v = v[1].v = v[2].v = 0.5f;
	
	v[0].X = Impact.X;
	v[0].Y = v[1].Y = v[2].Y = Impact.Y;
	v[0].Z = Impact.Z;
	
	{
		int steps = 30;
		int i;
		jeVec3d V;
		jeFloat Angle = 0.0f;
		jeFloat DAngleDStep = -(2.0f * 3.14159f / (jeFloat)steps);
		jeFloat Radius = P->ShadowScale;

		V = Impact;
		jeCamera_Transform(Camera,&V,&V);
		jeCamera_Project(Camera,&V,&V);
		v[0].X = V.X;
		v[0].Y = V.Y;
		v[0].Z = V.Z;

		jeTClip_SetTexture(NULL);

		V = Impact;
		V.Z += Radius;
		jeCamera_Transform(Camera,&V,&V);
		jeCamera_Project(Camera,&V,&V);
		v[1].X = V.X;
		v[1].Y = V.Y;
		v[1].Z = V.Z;
		for (i=0; i<steps+1; i++)
			{
				v[2] = v[1];

				V = Impact;
				V.X += (jeFloat)(sin( Angle ) * Radius);
				V.Z += (jeFloat)(cos( Angle ) * Radius);
				jeCamera_Transform(Camera,&V,&V);
				jeCamera_Project(Camera,&V,&V);
				v[1].X = V.X;
				v[1].Y = V.Y;
				v[1].Z = V.Z;

				Angle = Angle + DAngleDStep;
				jeTClip_Triangle(v);
			}
	}
#endif

#ifdef PROJECTED_SHADOW			
	{
		int i,j,Count;
		jeBodyInst_Index *List;
		jeBodyInst_Index Command;
		jeBody_SkinVertex *SV;
		
		G = jeBodyInst_GetShadowGeometry(P->BodyInstance,
						jePose_GetAllJointTransforms(Joints),0,Camera,&Impact);

		if ( G == NULL )
			{
				jeErrorLog_AddString(JE_ERR_SUBSYSTEM_FAILURE,"jePuppet_DrawShadow:  Failed to get shadow geometry",NULL);
				return JE_FALSE;
			}

		jeTClip_SetTexture(NULL);

		Count = G->FaceCount;
		List  = G->FaceList;

		for (i=0; i<Count; i++)
			{	
				Command = *List;
				List ++;
				//Material = *List;
				List ++;

				assert( Command == JE_BODY_FACE_TRIANGLE );

				{
					float AX,AY,BXMinusAX,BYMinusAY,CYMinusAY,CXMinusAX;
					jeBodyInst_Index *List2;
					
					List2 = List;
					SV = &(G->SkinVertexArray[ *List2 ]);
					AX = SV->SVPoint.X;
					AY = SV->SVPoint.Y;
					List2++;
					List2++;
					
					SV = &(G->SkinVertexArray[ *List2 ]);
					BXMinusAX = SV->SVPoint.X - AX;
					BYMinusAY = SV->SVPoint.Y - AY;
					List2++;
					List2++;

					SV = &(G->SkinVertexArray[ *List2 ]);
					CXMinusAX = SV->SVPoint.X - AX;
					CYMinusAY = SV->SVPoint.Y - AY;
					List2++;
					List2++;

					// ZCROSS is z the component of a 2d vector cross product of ABxAC
					//#define ZCROSS(Ax,Ay,Bx,By,Cx,Cy)  ((((Bx)-(Ax))*((Cy)-(Ay))) - (((By)-(Ay))*((Cx)-(Ax))))

					// 2d cross product of AB cross AC   (A is vtx[0], B is vtx[1], C is vtx[2]
					if ( ((BXMinusAX * CYMinusAY) - (BYMinusAY * CXMinusAX)) > 0.0f )
						{
							List = List2;
							continue;
						}
				}						

			#define SOME_SCALE (  255.0f / 40.0f )
				for (j=0; j<3; j++)
					{
						SV = &(G->SkinVertexArray[ *List ]);
						List++;

						v[j].X = SV->SVPoint.X;
						v[j].Y = SV->SVPoint.Y;

						v[j].Z = SV->SVPoint.Z;
						v[j].u = SV->SVU;
						v[j].v = SV->SVV;
						
						List++;
						v[j].a = (255.0f- (SV->SVU * SOME_SCALE));
					}
			
				if ((v[0].a > 0) && (v[1].a > 0) && (v[2].a > 0))
					{
						jeTClip_Triangle(v);
					}
			}
		assert( ((uint32)List) - ((uint32)G->FaceList) == (uint32)(G->FaceListSize) );
	}
#endif

#endif

}

#pragma warning (default:4100)

extern jeBoolean	h_LeftHanded;		// Hack of all mothers, need to check camera to see if left/right handed...

#define	DO_UV_MAPPING

extern jeWorld_DebugInfo g_WorldDebugInfo;

jeBoolean jePuppet_RenderThroughFrustum(const jePuppet		*P, 
										const jePose		*Joints, 
										const jeExtBox		*Box, 
										jeEngine			*Engine, 
										const jeWorld		*World,
										const jeCamera		*Camera, 
										const jeFrustum		*Frustum,
										jeBoolean updateStaticLightingFlag)
{
	//	TOM 05-24-03 This function needs to be rewritten to make it easier to handle
	//	pointer assignment errors -- especially for local var PM.

	uint32						ClipFlags;
	jeVec3d						Scale;
	const jeXFArray				*JointTransforms = NULL;
	const jeBodyInst_Geometry	*G = NULL;
	jeFrustum					WorldSpaceFrustum;
	jePuppet					*LP = NULL;

	assert( P      );
	assert( Engine );
	assert( Camera );
	assert( Joints );

	LP = (jePuppet *)P;

	JointTransforms = jePose_GetAllJointTransforms(Joints);

#pragma message ("Level of detail hacked:")

	jePose_GetScale(Joints,&Scale);
	G = jeBodyInst_GetGeometry(P->BodyInstance, &Scale, JointTransforms, 0, NULL);

	jeFrustum_TransformToWorldSpace(Frustum, Camera, &WorldSpaceFrustum);
	Frustum = &WorldSpaceFrustum;

	// Setup clip flags to clip to all frustum planes...
	ClipFlags = 0xffff;

	// Now either totally reject actor against frustum, or remove planes that don't need to be clipped against, etc...
//#if 1
	{
		jePlane		*pPlane = NULL;
		int32		k;

		pPlane = ((jeFrustum*)Frustum)->Planes;

		for (k=0; k< Frustum->NumPlanes; k++, pPlane++)
		{
			jePlane_Side	Side;

			pPlane->Type = Type_Any;

			Side = jePlane_BoxSide(pPlane, Box, 0.01f);

			if (Side == PSIDE_BACK)
				return JE_TRUE;			// Actor not in view frustum

			if (Side == PSIDE_FRONT)
				ClipFlags ^= (1<<k);	// Don't need to clip to this plane
		}
	}
//#endif

	if (G == NULL)
	{
		jeErrorLog_Add(JE_ERR_SUBSYSTEM_FAILURE,"jePuppet_RenderThroughFrustum: Failed to get draw geometry");
		return JE_FALSE;
	}

	{
		int32				NumFaces;
		int32				i;
		jeBodyInst_Index	*List = NULL;
		jeXForm3d			RootTransform;
		const jeXForm3d		*pActorToWorldXForm = NULL;
		jeXForm3d			MapperXForm, *pMapperXForm;
		uint32				RenderFlags;
		jeBodyInst_Index	LastMaterial;

		if (h_LeftHanded)
			RenderFlags = 0;
		else
			RenderFlags = JE_RENDER_FLAG_COUNTER_CLOCKWISE;

		jePuppet_StaticLightGrp.UseFillLight		 = P->UseFillLight;
		jePuppet_StaticLightGrp.FillLightNormal		 = P->FillLightNormal;
		jePuppet_StaticLightGrp.FillLightColor.Red	 = P->FillLightColor.Red;
		jePuppet_StaticLightGrp.FillLightColor.Green = P->FillLightColor.Green;
		jePuppet_StaticLightGrp.FillLightColor.Blue  = P->FillLightColor.Blue;
		jePuppet_StaticLightGrp.PerBoneLighting      = P->PerBoneLighting;

		jePose_GetJointTransform(Joints,P->LightReferenceBoneIndex,&(RootTransform));

		pActorToWorldXForm = jeCamera_XForm(Camera);

		// do dynamic lighting pass

		if (P->MaxDynamicLightsToUse > 0)
		{
			if (P->PerBoneLighting)
			{
				int BoneCount;
				const jeXForm3d *XFA = jeXFArray_GetElements(JointTransforms, &BoneCount);
				if (BoneCount>0)
				{
					if (P->BoneLightArraySize < BoneCount)
					{
						// realloc light array to correct size
						jePuppet_BoneLight *LG;

						LG = (jePuppet_BoneLight *)jeRam_Realloc(P->BoneLightArray, sizeof(jePuppet_BoneLight) * BoneCount);
						if (LG==NULL)
						{
							jeErrorLog_Add(JE_ERR_SUBSYSTEM_FAILURE,"jePuppet_Render: Failed to allocate space for bone lighting info cache");
							return JE_FALSE;
						}
						LP->BoneLightArray = LG;
						LP->BoneLightArraySize = BoneCount;
					}
					for (i=0; i<BoneCount; i++) // loop thru the bones
					{
						// for all dynamic lights, accumulate onto bone i
						P->BoneLightArray[i].DLightCount = jePuppet_PrepDynamicLights(P,World,
							P->BoneLightArray[i].DLights,&(XFA[i].Translation));
					}
				}
			}
			else
			{
				jePuppet_StaticLightGrp.DLightCount = jePuppet_PrepDynamicLights(P,World,
					jePuppet_StaticLightGrp.DLights,&(RootTransform.Translation));
			}
		}

		else
		{
			jePuppet_StaticLightGrp.DLightCount = 0;
		}

		// do static lighting pass

		if (P->MaxStaticLightsToUse > 0)
		{
			if (updateStaticLightingFlag) // need to re-cache static lighting for this puppet
			{
				if (P->PerBoneLighting)
				{
					int BoneCount;
					const jeXForm3d *XFA = jeXFArray_GetElements(JointTransforms, &BoneCount);
					if (BoneCount>0)
					{
						if (P->BoneLightArraySize < BoneCount)
						{
							// realloc light array to correct size
							jePuppet_BoneLight *LG = NULL;

							LG = (jePuppet_BoneLight *)jeRam_Realloc(P->BoneLightArray, sizeof(jePuppet_BoneLight) * BoneCount);
							if (LG==NULL)
							{
								jeErrorLog_Add(JE_ERR_SUBSYSTEM_FAILURE,"jePuppet_Render: Failed to allocate space for bone lighting info cache");
								return JE_FALSE;
							}
							LP->BoneLightArray = LG;
							LP->BoneLightArraySize = BoneCount;
						}
						for (i=0; i<BoneCount; i++) // loop thru the bones
						{
							// for all static lights, accumulate onto bone i
							P->BoneLightArray[i].SLightCount = jePuppet_PrepStaticLights(P,
								World,
								P->BoneLightArray[i].SLights,
								&(XFA[i].Translation));
						}
					}
				}
				else // not doing per-bone lighting
				{
					LP->SLightCount = jePuppet_PrepStaticLights(P,
						World,
						LP->SLights,
						&(RootTransform.Translation));
				}
			}
		}

		else
		{
			LP->SLightCount = 0;
		}

		// @@
		jePuppet_ComputeAmbientLight(P, &(jePuppet_StaticLightGrp.Ambient), &(RootTransform.Translation));

		NumFaces	= G->FaceCount;
		List		= G->FaceList;

		LastMaterial = -1;

		// For each face, clip it to the view frustum 
		jePuppet_Material	*PM = NULL;
		for (i=0; i<NumFaces; i++)
		{
#define MAX_TEMP_VERTS		64		

			int32				v;
			jeBodyInst_Index	Command, Material;
			float				Dist;
			jeLVertex			LVerts1[MAX_TEMP_VERTS], LVerts2[MAX_TEMP_VERTS], *pLVert;
			jeTLVertex			TLVerts[MAX_TEMP_VERTS];
			jeVec3d				v1, v2, v3;
			jeFrustum_LClipInfo	ClipInfo;

			Command	= *List;
			List++;
			Material = *List;
			List ++;

			assert( Command == JE_BODYINST_FACE_TRIANGLE );
			assert( Material>=0 );
			assert( Material<P->MaterialCount);

#ifdef DO_UV_MAPPING
			if (Material != LastMaterial)
			{
				PM = &(P->MaterialArray[Material]);
				if (PM)
				{
					jePuppet_StaticLightGrp.MaterialColor = PM->Color;

					if (PM->Mapper != jeUVMap_Projection)
					{
						pMapperXForm = (jeXForm3d*)pActorToWorldXForm;
					}
					else
					{
#pragma message("Puppet.c: hard-coded default projection matrix vals for case of jeUVMap_Projection")
						jeXForm3d		ProjXForm;

						ProjXForm.AX = 0.03f; ProjXForm.AY = 0.02f; ProjXForm.AZ = 0.0f;
						ProjXForm.BX = 0.01f; ProjXForm.BY = 0.09f; ProjXForm.BZ = 0.0f;
						ProjXForm.CX = 0.06f; ProjXForm.CY = 0.08f; ProjXForm.CZ = 0.0f;

						jeVec3d_Clear(&ProjXForm.Translation);

						pMapperXForm = &MapperXForm;

						jeXForm3d_Multiply(&ProjXForm, pActorToWorldXForm, pMapperXForm);
					}	//	else...

					LastMaterial = Material;		// Make LastMaterial current
				}	//	if (PM)...
//				else
//				{
//					return JE_FALSE;
//				}
			}	//	if (Material != LastMaterial)...
#else
			PM = &(P->MaterialArray[Material]);
			if (PM)
			{
				jePuppet_StaticLightGrp.MaterialColor = PM->Color;
			}
			else
			{
				return JE_FALSE;
			}
#endif

				// Fill in the LVert array
				pLVert = LVerts1;

				for (v=0; v< 3; v++, pLVert++)
				{
					jeBodyInst_SkinVertex	*SVert;

					SVert = &G->SkinVertexArray[*List];
					List++;

					// Get XYZ
					*((jeVec3d*)pLVert) = SVert->SVPoint;

					// Get UV
#ifdef DO_UV_MAPPING
#pragma message("Puppet : UVMapper should act on an array of verts!")
					if (PM)
					{
						if (PM->Mapper != NULL)
						{
							jeLVertex		MapVert;

							*((jeVec3d*)&MapVert) = SVert->SVW;

							PM->Mapper(pMapperXForm, &MapVert, &G->NormalArray[*List], 1);

							pLVert->u = MapVert.u;
							pLVert->v = MapVert.v;
						}
						else
	#endif
						{
							pLVert->u = SVert->SVU;
							pLVert->v = SVert->SVV;
						}
					}	//	if (PM)...

					assert( ((float)fabs(1.0-jeVec3d_Length( &(G->NormalArray[ *List ] ))))< 0.001f );

					jePuppet_StaticLightGrp.SurfaceNormal = (G->NormalArray[ *List ]);
					List++;

					// Get RGB
					jePuppet_SetVertexColor(LP, pLVert,SVert->ReferenceBoneIndex);
				}	//	for...

#pragma message ("This backface rejection code should go above uv/lighting computations...")
				jeVec3d_Subtract((jeVec3d*)&LVerts1[2], (jeVec3d*)&LVerts1[1], &v1);
				jeVec3d_Subtract((jeVec3d*)&LVerts1[0], (jeVec3d*)&LVerts1[1], &v2);
				jeVec3d_CrossProduct(&v1, &v2, &v3);
				jeVec3d_Normalize(&v3);

				Dist = jeVec3d_DotProduct(&v3, (jeVec3d*)&LVerts1[0]);

				Dist = jeVec3d_DotProduct(&v3, jeCamera_GetPov(Camera)) - Dist;

				if (Dist <= 0)
					continue;		// Backfaced to camera

				ClipInfo.NumSrcVerts = 3;
				ClipInfo.SrcVerts = LVerts1;

				ClipInfo.Work1 = LVerts1;
				ClipInfo.Work2 = LVerts2;

				ClipInfo.ClipFlags = ClipFlags;

				// Clip UVRGB
				if (!jeFrustum_ClipLVertsXYZUVRGB(Frustum, &ClipInfo))
					continue;		// Poly was clipped away

				// Transform to world space...
				for (pLVert = ClipInfo.DstVerts, v=0; v< ClipInfo.NumDstVerts; v++, pLVert++)
					jeXForm3d_Transform(pActorToWorldXForm, (jeVec3d*)pLVert, (jeVec3d*)pLVert);

				// Project to screenspace
				jeCamera_ProjectAndClampLArray(Camera, ClipInfo.DstVerts, TLVerts, ClipInfo.NumDstVerts);

				//TLVerts[0].a = 255.0f;
				//	[MacroArt::Begin]
				TLVerts[0].a = P->fOverallAlpha;
				//			if(P->fOverallAlpha<255.0f) RenderFlags=RenderFlags|JE_RENDER_FLAG_ALPHA;
				//	[MacroArt::End]

				g_WorldDebugInfo.NumActorPolys++;
			if (PM)
			{
				jeEngine_RenderPoly(Engine, TLVerts, ClipInfo.NumDstVerts, PM->Material, RenderFlags);
			}	//	if (PM)...
//			else
//			{
//				return JE_FALSE;
//			}

		}
	}

	//"Need to write a RenderShadowThroughFrustum...")
	/*
	if (P->DoShadow)
	{
	jePuppet_DrawShadow(P,Engine,World,Camera);
	}
	*/
	return JE_TRUE;
}

#ifdef PROFILE
#define PUPPET_AVERAGE_ACROSS 60
double Puppet_AverageCount[PUPPET_AVERAGE_ACROSS]={
	0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,
	0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,
	0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,
	0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,
	0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,
	0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0};
int Puppet_AverageIndex = 0;
#endif


jeBoolean	jePuppet_Render(const jePuppet	*P, 
							const jePose	*Joints,
							jeEngine		*Engine, 
							const jeWorld	*World,
							const jeCamera	*Camera, 
							jeExtBox		*TestBox,
							jeBoolean		updateStaticLightingFlag)
{
	jePuppet *LP;
	const jeXFArray *JointTransforms;
	jeVec3d Scale;
	#ifdef PROFILE
	rdtsc_timer_type RDTSCStart,RDTSCEnd;
	#endif
	jeRect ClippingRect;
	jeBoolean Clipping = JE_TRUE;

	// BEGIN - Fixed far clip plane for actors - paradoxnj 4/21/2005
	jeBoolean Enable;
	float ZFar;
	// END - Fixed far clip plane for actors - paradoxnj 4/21/2005

	#define BACK_EDGE (1.0f)

	const jeBodyInst_Geometry *G;
//	[MacroArt::Begin]
	uint32	RenderFlags;
//	[MacroArt::End]

	assert( P      );
	assert( Engine );
	assert( Camera );

	#ifdef PROFILE
	rdtsc_read(&RDTSCStart);
    rdtsc_zero(&RDTSCEnd);
	#endif


	LP = (jePuppet*)P;
	jeCamera_GetClippingRect(Camera,&ClippingRect);
	
	// BEGIN - Fixed far clip plane for actors - paradoxnj 4/21/2005
	jeCamera_GetFarClipPlane(Camera, &Enable, &ZFar);
	if (!Enable)
		ZFar = BACK_EDGE;
	// END - Fixed far clip plane for actor - paradoxnj 4/21/2005

	if (TestBox != NULL)
	{
		// see if the test box is visible on the screen.  If not: don't draw actor.
		// (transform and project it to the screen, then check extents of that projection
		//  against the clipping rect)
		jeVec3d BoxCorners[8];
		const jeXForm3d *ObjectToCamera;
		jeVec3d Maxs,Mins;
		#define BIG_NUMBER (99e9f)  
		int i;

		BoxCorners[0] = TestBox->Min;
		BoxCorners[1] = BoxCorners[0];  BoxCorners[1].X = TestBox->Max.X;
		BoxCorners[2] = BoxCorners[0];  BoxCorners[2].Y = TestBox->Max.Y;
		BoxCorners[3] = BoxCorners[0];  BoxCorners[3].Z = TestBox->Max.Z;
		BoxCorners[4] = TestBox->Max;
		BoxCorners[5] = BoxCorners[4];  BoxCorners[5].X = TestBox->Min.X;
		BoxCorners[6] = BoxCorners[4];  BoxCorners[6].Y = TestBox->Min.Y;
		BoxCorners[7] = BoxCorners[4];  BoxCorners[7].Z = TestBox->Min.Z;

		ObjectToCamera = jeCamera_XForm(Camera);
		assert( ObjectToCamera );

		jeVec3d_Set(&Maxs,-BIG_NUMBER,-BIG_NUMBER,-BIG_NUMBER);
		jeVec3d_Set(&Mins, BIG_NUMBER, BIG_NUMBER, BIG_NUMBER);
		for (i=0; i<8; i++)
		{
			jeVec3d V;
			jeXForm3d_Transform(  ObjectToCamera,&(BoxCorners[i]),&(BoxCorners[i]));
			jeCamera_Project(  Camera,&(BoxCorners[i]),&V);
			if (V.X > Maxs.X ) Maxs.X = V.X;
			if (V.X < Mins.X ) Mins.X = V.X;
			if (V.Y > Maxs.Y ) Maxs.Y = V.Y;
			if (V.Y < Mins.Y ) Mins.Y = V.Y;
			if (V.Z > Maxs.Z ) Maxs.Z = V.Z;
			if (V.Z < Mins.Z ) Mins.Z = V.Z;
		}

		if (   (Maxs.X < ClippingRect.Left) 
			|| (Mins.X > ClippingRect.Right)
			|| (Maxs.Y < ClippingRect.Top) 
			|| (Mins.Y > ClippingRect.Bottom)
			|| (Maxs.Z < BACK_EDGE) )
		{
			// not gonna draw: box is not visible.
			return JE_TRUE;
		}

		// BEGIN - Fixed far clip plane for actors - paradoxnj 4/21/2005
		if (Enable)
		{
			if (Mins.Z > ZFar)
				return JE_TRUE;				// Beyond ZFar ClipPlane
		}
		// END - Fixed far clip plane for actors - paradoxnj 4/21/2005
	}

	// Now actor is in the camera field - test it against the BSP area
	//extern jeBSPNode_Area *jeBSP_FindArea(jeBSP *BSP, const jeVec3d *Pos);


	Engine->DebugInfo.NumActors++;
	jeTClip_SetupEdges(Engine,
						(jeFloat)ClippingRect.Left,
						(jeFloat)ClippingRect.Right,
						(jeFloat)ClippingRect.Top,
						(jeFloat)ClippingRect.Bottom,
						BACK_EDGE);
		
	JointTransforms = jePose_GetAllJointTransforms(Joints);

//#pragma message ("Level of detail hacked:")
	jePose_GetScale(Joints,&Scale);

	G = jeBodyInst_GetGeometry(P->BodyInstance, &Scale, JointTransforms, 0,Camera);

	if ( G == NULL )
	{
		jeErrorLog_Add(JE_ERR_SUBSYSTEM_FAILURE,"jePuppet_Render: Failed to get draw geometry");
		return JE_FALSE;
	}

#ifdef ONE_OVER_Z_PIPELINE
#define TEST_Z_OUT(zzz, edge) 		((zzz) > (edge)) 
#define TEST_Z_IN(zzz, edge) 		((zzz) < (edge)) 
#pragma message ("test this! this is untested")
#else
#define TEST_Z_OUT(zzz, edge) 		((zzz) < (edge)) 
#define TEST_Z_IN(zzz, edge) 		((zzz) > (edge)) 
#endif


	// check for trivial rejection:
	{
		if (   (G->Maxs.X < ClippingRect.Left) 
			|| (G->Mins.X > ClippingRect.Right)
			|| (G->Maxs.Y < ClippingRect.Top) 
			|| (G->Mins.Y > ClippingRect.Bottom)
			|| ( TEST_Z_OUT( G->Maxs.Z, BACK_EDGE) ) )
		{
			// not gonna draw
			return JE_TRUE;
		}

		if (   (G->Maxs.X < ClippingRect.Right) 
			&& (G->Mins.X > ClippingRect.Left)
			&& (G->Maxs.Y < ClippingRect.Bottom) 
			&& (G->Mins.Y > ClippingRect.Top)
			&& ( TEST_Z_IN( G->Mins.Z, BACK_EDGE) ) )
		{
			// not gonna clip
			Clipping = JE_FALSE;
		}
		else
		{
			Clipping = JE_TRUE;
		}
	}

	{
		jeLVertex v[3], mapVert;
		int i,j,Count;
		jeBodyInst_Index *List;
		jeBodyInst_Index Command;
		jeBodyInst_SkinVertex *SV;
		jeXForm3d RootTransform;
		jePuppet_Material *PM;
		jeBodyInst_Index Material,LastMaterial;
		jeXForm3d CamXForm, projXForm, mapperXForm;

		jeCamera_GetTransposeXForm(Camera, &CamXForm);
		PM = NULL;

		jePuppet_StaticLightGrp.UseFillLight		 = P->UseFillLight;
		jePuppet_StaticLightGrp.FillLightNormal		 = P->FillLightNormal;
		jePuppet_StaticLightGrp.FillLightColor.Red	 = P->FillLightColor.Red;
		jePuppet_StaticLightGrp.FillLightColor.Green = P->FillLightColor.Green;
		jePuppet_StaticLightGrp.FillLightColor.Blue  = P->FillLightColor.Blue;
		jePuppet_StaticLightGrp.PerBoneLighting		 = P->PerBoneLighting;

		jePose_GetJointTransform(Joints,P->LightReferenceBoneIndex,&(RootTransform));

		// do dynamic lighting pass

		if (P->MaxDynamicLightsToUse > 0)
		{
			if (P->PerBoneLighting)
			{
				int BoneCount;
				const jeXForm3d *XFA = jeXFArray_GetElements(JointTransforms, &BoneCount);
				if (BoneCount>0)
				{
					if (P->BoneLightArraySize < BoneCount)
					{
						// realloc light array to correct size
						jePuppet_BoneLight *LG;
				
						LG = (jePuppet_BoneLight *)jeRam_Realloc(P->BoneLightArray, sizeof(jePuppet_BoneLight) * BoneCount);
						if (LG==NULL)
						{
							jeErrorLog_Add(JE_ERR_SUBSYSTEM_FAILURE,"jePuppet_Render: Failed to allocate space for bone lighting info cache");
							return JE_FALSE;
						}
						LP->BoneLightArray = LG;
						LP->BoneLightArraySize = BoneCount;
					}
					for (i=0; i<BoneCount; i++) // loop thru the bones
					{
						// for all dynamic lights, accumulate onto bone i
						LP->BoneLightArray[i].DLightCount = jePuppet_PrepDynamicLights(P,World,
							P->BoneLightArray[i].DLights,&(XFA[i].Translation));
					}
				}
			}
			else
			{
				jePuppet_StaticLightGrp.DLightCount = jePuppet_PrepDynamicLights(P,World,
							jePuppet_StaticLightGrp.DLights,&(RootTransform.Translation));
			}
		}

		else
		{
			jePuppet_StaticLightGrp.DLightCount = 0;
		}

		// do static lighting pass

		if (P->MaxStaticLightsToUse > 0)
		{
			if (updateStaticLightingFlag) // need to re-cache static lighting for this puppet
			{
				if (P->PerBoneLighting)
				{
					int BoneCount;
					const jeXForm3d *XFA = jeXFArray_GetElements(JointTransforms, &BoneCount);
					if (BoneCount>0)
					{
						if (P->BoneLightArraySize < BoneCount)
						{
							// realloc light array to correct size
							jePuppet_BoneLight *LG;
					
							LG = (jePuppet_BoneLight *)jeRam_Realloc(P->BoneLightArray, sizeof(jePuppet_BoneLight) * BoneCount);
							if (LG==NULL)
							{
								jeErrorLog_Add(JE_ERR_SUBSYSTEM_FAILURE,"jePuppet_Render: Failed to allocate space for bone lighting info cache");
								return JE_FALSE;
							}
							LP->BoneLightArray = LG;
							LP->BoneLightArraySize = BoneCount;
						}
						for (i=0; i<BoneCount; i++) // loop thru the bones
						{
							// for all static lights, accumulate onto bone i
							LP->BoneLightArray[i].SLightCount = jePuppet_PrepStaticLights(P,
								World,
								P->BoneLightArray[i].SLights,
								&(XFA[i].Translation));
						}
					}
				}
				else // not doing per-bone lighting
				{
					LP->SLightCount = jePuppet_PrepStaticLights(P,
						World,
						LP->SLights,
						&(RootTransform.Translation));
				}
			}
		}

		else
		{
			LP->SLightCount = 0;
		}

// @@
		jePuppet_ComputeAmbientLight(P, &(jePuppet_StaticLightGrp.Ambient),&(RootTransform.Translation));
		
		Count = G->FaceCount;
		List  = G->FaceList;
		//v[0].a = v[1].a= v[2].a = 255.0f;

//	[MacroArt::Begin]
	v[0].a = v[1].a= v[2].a =P->fOverallAlpha;
	RenderFlags=JE_RENDER_FLAG_COUNTER_CLOCKWISE;
//	if(P->fOverallAlpha<255.0f) RenderFlags=RenderFlags|JE_RENDER_FLAG_ALPHA;
//	[MacroArt::End]


		LastMaterial = -1;

		for (i=0; i<Count; i++)
		{	

			Command = *List;
			List ++;
			Material = *List;
			List ++;

			assert( Command == JE_BODYINST_FACE_TRIANGLE );
			assert( Material>=0 );
			assert( Material<P->MaterialCount);

			{
				float AX,AY,BXMinusAX,BYMinusAY,CYMinusAY,CXMinusAX;
				jeBodyInst_Index *List2;
				
				List2 = List;
				SV = &(G->SkinVertexArray[ *List2 ]);
				AX = SV->SVPoint.X;
				AY = SV->SVPoint.Y;
				List2++;
				List2++;
				
				SV = &(G->SkinVertexArray[ *List2 ]);
				BXMinusAX = SV->SVPoint.X - AX;
				BYMinusAY = SV->SVPoint.Y - AY;
				List2++;
				List2++;

				SV = &(G->SkinVertexArray[ *List2 ]);
				CXMinusAX = SV->SVPoint.X - AX;
				CYMinusAY = SV->SVPoint.Y - AY;
				List2++;
				List2++;

				// ZCROSS is z the component of a 2d vector cross product of ABxAC
				//#define ZCROSS(Ax,Ay,Bx,By,Cx,Cy)  ((((Bx)-(Ax))*((Cy)-(Ay))) - (((By)-(Ay))*((Cx)-(Ax))))
				// 2d cross product of AB cross AC   (A is vtx[0], B is vtx[1], C is vtx[2]
				
				if ( ((BXMinusAX * CYMinusAY) - (BYMinusAY * CXMinusAX)) > 0.0f )
				{
					List = List2;
					continue;
				}
				
			}

			if (Material != LastMaterial)
			{
				PM = &(P->MaterialArray[Material]);
				jeTClip_SetTexture(PM->Material,0);
				jePuppet_StaticLightGrp.MaterialColor = PM->Color;

				if (PM->Mapper != jeUVMap_Projection)
				{
					mapperXForm = CamXForm;
				}
				else
				{
#pragma message("Puppet.c: hard-coded default projection matrix vals for case of jeUVMap_Projection")
					projXForm.AX = 0.03f; projXForm.AY = 0.02f; projXForm.AZ = 0.0f;
					projXForm.BX = 0.01f; projXForm.BY = 0.09f; projXForm.BZ = 0.0f;
					projXForm.CX = 0.06f; projXForm.CY = 0.08f; projXForm.CZ = 0.0f;
					jeVec3d_Clear(&projXForm.Translation);

					jeXForm3d_Multiply(&projXForm, &CamXForm, &mapperXForm);
				}

				LastMaterial = Material;		// Make LastMaterial current
			}

			for (j=0; j<3; j++)
			{
				SV = &(G->SkinVertexArray[ *List ]);
				List++;

				v[j].X = SV->SVPoint.X;
				v[j].Y = SV->SVPoint.Y;
				v[j].Z = SV->SVPoint.Z;

#pragma message("Puppet : UVMapper should act on an array of verts!")
				if (PM->Mapper != NULL)
				{
					*((jeVec3d *)&mapVert) = SV->SVW;

					PM->Mapper(&mapperXForm, &mapVert, &G->NormalArray[*List], 1);

					v[j].u = mapVert.u;
					v[j].v = mapVert.v;
				}
				else
				{
					v[j].u = SV->SVU;
					v[j].v = SV->SVV;
				}
				assert( ((float)fabs(1.0-jeVec3d_Length( &(G->NormalArray[ *List ] ))))< 0.001f );
				
				jePuppet_StaticLightGrp.SurfaceNormal = (G->NormalArray[ *List ]);
				List++;

				jePuppet_SetVertexColor(LP, &v[j], SV->ReferenceBoneIndex);

			}
		
			g_WorldDebugInfo.NumActorPolys++;

			if (Clipping)
			{
				jeTClip_Triangle(v);
			}
			else
			{
				assert (PM != NULL);

//	[MacroArt::Begin]
				// BEGIN - Get rid of JE_ crap - paradoxnj 4/21/2005
				jeEngine_RenderPoly(Engine, (jeTLVertex *)v, 3, PM->Material,RenderFlags);
				// END - Get rid of JE_ crap - paradoxnj 4/21/2005
//				jeEngine_RenderPoly(Engine, (JE_TLVertex *)v, 3, PM->Bitmap,JE_RENDER_FLAG_COUNTER_CLOCKWISE );
//	[MacroArt::End]

			}

		}
		assert( ((uint32)List) - ((uint32)G->FaceList) == (uint32)(G->FaceListSize) );
	}

// @@
#pragma message("Puppet.c Line 2057:  Why are shadows not implemented - paradoxnj 4/21/2005")
/*
	if (P->DoShadow)
	{
		jePuppet_DrawShadow(P,Joints,Engine, Camera);
	}
*/

	#ifdef PROFILE
	{
		double Count=0.0;
		int i;

		rdtsc_read(&RDTSCEnd);
		rdtsc_delta(&RDTSCStart,&RDTSCEnd,&RDTSCEnd);
		//jeEngine_Printf(Engine, 320,10,"Puppet Render Time=%f",(double)(rdtsc_cycles(&RDTSCEnd)/200000000.0));
		//jeEngine_Printf(Engine, 320,30,"Puppet Render Cycles=%f",(double)(rdtsc_cycles(&RDTSCEnd)));
		Puppet_AverageCount[(Puppet_AverageIndex++)%PUPPET_AVERAGE_ACROSS] = rdtsc_cycles(&RDTSCEnd);
		for (i=0; i<PUPPET_AVERAGE_ACROSS; i++)
			{	
				Count+=Puppet_AverageCount[i];
			}
		Count /= (double)PUPPET_AVERAGE_ACROSS;

		//jeEngine_Printf(Engine, 320,60,"Puppet AVG Render Time=%f",(double)(Count/200000000.0));
		//jeEngine_Printf(Engine, 320,90,"Puppet AVG Render Cycles=%f",(double)(Count));
				
	}
	#endif

	return JE_TRUE;
}

void jePuppet_SetShadow(jePuppet *P, jeBoolean DoShadow, 
		jeFloat Scale, const jeMaterialSpec *ShadowMap,
		int BoneIndex)
{
	assert( P );
	assert( (DoShadow==JE_FALSE) || (DoShadow==JE_TRUE));

	if ( P->ShadowMap )
//		jeBitmap_Destroy((jeBitmap **)&(P->ShadowMap));
		jeMaterialSpec_Destroy((jeMaterialSpec **)&(P->ShadowMap));

	P->DoShadow = DoShadow;
	P->ShadowScale = Scale;
	P->ShadowMap = ShadowMap;
	P->ShadowBoneIndex = BoneIndex;

	if ( P->ShadowMap )
		jeMaterialSpec_CreateRef((jeMaterialSpec *)P->ShadowMap);
}
