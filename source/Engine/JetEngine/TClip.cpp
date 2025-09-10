/****************************************************************************************/
/*  TCLIP.C                                                                             */
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

#ifdef __ICL
#pragma message("ICL")
#pragma warning(disable : 344 266) // seems to do nothing to intel
#endif

//#define DO_TIMER
//#define USE_OLD

/*********

Cbloom Jan 18
TClip gained 2-3 fps
(not counting the gains from _SetTexture)

I reorganized the TClip_Triangle function flow to early-out 
for triangles all-in or all-out.  The old code considered this
case, but was not as lean as possible for these most-common cases.

To solve these problems, flow was changed to :
	1. do all compares and accumulated the 3 out bits for each of the five faces
		(so we have 15 bit-flags)
	2. then do clips while more clips remain.
	3. as a free benefit, the new structure means that Rasterize is only called once
			in TClip_Triangle, so it was inlined.

Step two results in very fast exiting when no clipping remains.

If/When we get the Intel compiler that can optimize ?: to CMOV, speed will improve
even more!

*** Tested : with /Qxi /Qipo /G6 on the Intel compiler, we cut another 8% off the time!
	The result is a net 63% gain in TClip time !  From 1 ms/frame to 0.37 ms/frame !!!

-----------------------------------

Timer profiling shows:
(with D3DDrv, in ActView, viewing dema.act)
times are seconds per frame

default pose : 58.6 fps
TClip_New            : 0.006749
TClip_Rasterize      : 0.006149

default pose : 52.9 fps
TClip_Old            : 0.007230 : 1.$ %
TClip_Rasterize      : 0.006183 : 1.$ %

***********/

// TClip.c
//  Fast Triangle Clipping
/*}{***********************/

#include <assert.h>
#include <string.h>

#include "Dcommon.h"
#include "Engine.h"

#include "TClip.h"
#include "Bitmap._h"

#include "List.h"
#include "Ram.h"  
#include "Errorlog.h"

#include "Timer.h"

#include "jeMaterial.h"

TIMER_VARS(TClip_Triangle);

//#define ONE_OVER_Z_PIPELINE	// this has more accuracy, but the slowness of doing 1/ divides

typedef enum 
{	
	BACK_CLIPPING_PLANE = 0,
	LEFT_CLIPPING_PLANE,
	RIGHT_CLIPPING_PLANE,
	TOP_CLIPPING_PLANE,
	BOTTOM_CLIPPING_PLANE,
	NUM_CLIPPING_PLANES
} jeTClip_ClippingPlane;

// 3 bits for V_IN/OUT flags
#define V_ALL_IN (0)
#define V0_OUT	(1)
#define V1_OUT	(2)
#define V2_OUT	(4)

	// at a=0, result is l;  at a=1, result is h
#define LINEAR_INTERPOLATE(a,l,h)     ((l)+(((h)-(l))*(a)))

typedef struct jeTClip_StaticsType
{
	jeFloat LeftEdge;
	jeFloat RightEdge;
	jeFloat TopEdge;
	jeFloat BottomEdge;
	jeFloat BackEdge;

	DRV_Driver * Driver;
	jeEngine	*Engine;
	const jeMaterialSpec *Material;
	jeTexture * THandle;

	int32 RenderFlags;
	uint32 DefaultRenderFlags;
} jeTClip_StaticsType;

/*}{************ Protos ***********/

static void JETCF jeTClip_Split(JE_LVertex *NewVertex,const JE_LVertex *V1,const JE_LVertex *V2,int ClippingPlane);
static void JETCF jeTClip_TrianglePlane(const JE_LVertex * zTriVertex,int ClippingPlane);

/*}{************ The State Statics ***********/

static Link * jeTClip_Link = NULL;
static jeTClip_StaticsType jeTClip_Statics;

/*}{************ Functions ***********/

JETAPI jeBoolean JETCC jeTClip_Push(void)
{
jeTClip_StaticsType * TCI;

	if ( ! jeTClip_Link )
	{
		List_Start();
		jeTClip_Link = Link_Create();
		if ( ! jeTClip_Link ) 
			return JE_FALSE;
	}

	TCI = (jeTClip_StaticsType *)jeRam_Allocate(sizeof(jeTClip_StaticsType));
	if ( ! TCI )
		return JE_FALSE;
	memcpy(TCI,&jeTClip_Statics,sizeof(jeTClip_StaticsType));

	Link_Push( jeTClip_Link , TCI );

	return JE_TRUE;
}

JETAPI jeBoolean JETCC jeTClip_Pop(void)
{
jeTClip_StaticsType * TCI;
	if ( ! jeTClip_Link )
		return JE_FALSE;
	TCI = (jeTClip_StaticsType *)Link_Pop( jeTClip_Link );
	if ( ! TCI )
		return JE_FALSE;
	memcpy(&jeTClip_Statics,TCI,sizeof(jeTClip_StaticsType));
	jeRam_Free(TCI);

	if ( ! Link_Peek(jeTClip_Link) )
	{
		Link_Destroy(jeTClip_Link);
		jeTClip_Link = NULL;
		List_Stop();
	}
	return JE_TRUE;
}

JETAPI jeBoolean JETCC jeTClip_SetTexture(const jeMaterialSpec * Material, int32 RenderFlags)
{
	jeTexture* Texture = NULL;
	jeBitmap* Bitmap = NULL;
	jeTClip_Statics.Material = Material;
	jeTClip_Statics.RenderFlags = RenderFlags;
	
    if (Material != NULL) {
	    Texture = jeMaterialSpec_GetLayerTexture(Material, 0);
	    jeTClip_Statics.THandle = Texture;
	    if ( Texture == NULL) {
		    Bitmap = jeMaterialSpec_GetLayerBitmap(Material, 0);
	    }
    }

	if ( Bitmap )
	{
		jeTClip_Statics.THandle = jeBitmap_GetTHandle(Bitmap);
		assert(jeTClip_Statics.THandle);
	}
	else
	{
		jeTClip_Statics.THandle = NULL;
	}
	return JE_TRUE;
}

JETAPI void JETCC jeTClip_SetupEdges(
	jeEngine *Engine,
	jeFloat LeftEdge, 
	jeFloat RightEdge,
	jeFloat TopEdge ,
	jeFloat BottomEdge,
	jeFloat BackEdge)
{ 
	assert(Engine);
	memset(&jeTClip_Statics,0,sizeof(jeTClip_Statics));
	jeTClip_Statics.Engine		= Engine;
	jeTClip_Statics.Driver		= jeEngine_GetDriver(Engine);
	jeTClip_Statics.LeftEdge	= LeftEdge;
	jeTClip_Statics.RightEdge	= RightEdge;
	jeTClip_Statics.TopEdge		= TopEdge;
	jeTClip_Statics.BottomEdge	= BottomEdge;
	jeTClip_Statics.BackEdge	= BackEdge;
	if (jeEngine_GetDefaultRenderFlags(Engine, &jeTClip_Statics.DefaultRenderFlags)==JE_FALSE)
		{
			jeErrorLog_Add(JE_ERR_SUBSYSTEM_FAILURE,"jeTClip_SetupEdges");
			jeErrorLog_Clear();
			jeTClip_Statics.DefaultRenderFlags = 0;
		}
}

#ifdef DO_TIMER
void jeTClip_Done(void)
{
	TIMER_REPORT(TClip_Triangle);
}
#endif

JETAPI void JETCC jeTClip_Triangle(const JE_LVertex TriVertex[3])
{

	TIMER_P(TClip_Triangle);

	jeTClip_TrianglePlane(TriVertex,BACK_CLIPPING_PLANE);

	TIMER_Q(TClip_Triangle);
}



/*}{************ TClip_Split ***********/

static void JETCF jeTClip_Split(JE_LVertex *NewVertex,const JE_LVertex *V1,const JE_LVertex *V2,int ClippingPlane)
{
	jeFloat Ratio=0.0f;
	jeFloat OneOverZ1,OneOverZ2;
	
	#ifdef ONE_OVER_Z_PIPELINE
		// in here ->Z is really (one over z)
		OneOverZ1 = V1->Z;
		OneOverZ2 = V2->Z;
	#else
		OneOverZ1 = 1.0f/V1->Z;
		OneOverZ2 = 1.0f/V2->Z;
	#endif

	switch (ClippingPlane)
		{
			case (BACK_CLIPPING_PLANE):
				assert((V2->Z - V1->Z)!=0.0f);
				Ratio = ((1.0f/jeTClip_Statics.BackEdge) - OneOverZ2)/( OneOverZ1 - OneOverZ2 );

				NewVertex->X = LINEAR_INTERPOLATE(Ratio,(V2->X),(V1->X));
				NewVertex->Y = LINEAR_INTERPOLATE(Ratio,(V2->Y),(V1->Y));
				#ifdef ONE_OVER_Z_PIPELINE
				NewVertex->Z = 1.0f/ jeTClip_Statics.BackEdge;
				#else
				NewVertex->Z = jeTClip_Statics.BackEdge;
				#endif
			
				break;
			case (LEFT_CLIPPING_PLANE):
				assert((V2->X - V1->X)!=0.0f);
				Ratio = (jeTClip_Statics.LeftEdge - V2->X)/( V1->X - V2->X);

				NewVertex->X = jeTClip_Statics.LeftEdge;
				NewVertex->Y = LINEAR_INTERPOLATE(Ratio,(V2->Y),(V1->Y));
				#ifdef ONE_OVER_Z_PIPELINE
				NewVertex->Z = LINEAR_INTERPOLATE(Ratio,OneOverZ2,OneOverZ1);
				#else
				NewVertex->Z = 1.0f/LINEAR_INTERPOLATE(Ratio,OneOverZ2,OneOverZ1);
				#endif
		
				break;
			case (RIGHT_CLIPPING_PLANE):
				assert((V2->X - V1->X)!=0.0f);
				Ratio = (jeTClip_Statics.RightEdge - V2->X)/( V1->X - V2->X);

				NewVertex->X = jeTClip_Statics.RightEdge;
				NewVertex->Y = LINEAR_INTERPOLATE(Ratio,(V2->Y),(V1->Y));
				#ifdef ONE_OVER_Z_PIPELINE
				NewVertex->Z = LINEAR_INTERPOLATE(Ratio,OneOverZ2,OneOverZ1);
				#else
				NewVertex->Z = 1.0f/LINEAR_INTERPOLATE(Ratio,OneOverZ2,OneOverZ1);
				#endif

				break;
			case (TOP_CLIPPING_PLANE):
				assert((V2->Y - V1->Y)!=0.0f);
				Ratio = (jeTClip_Statics.TopEdge - V2->Y)/( V1->Y - V2->Y);

				NewVertex->X = LINEAR_INTERPOLATE(Ratio,(V2->X),(V1->X));
				NewVertex->Y = jeTClip_Statics.TopEdge;
				#ifdef ONE_OVER_Z_PIPELINE
				NewVertex->Z = LINEAR_INTERPOLATE(Ratio,OneOverZ2,OneOverZ1);
				#else
				NewVertex->Z = 1.0f/LINEAR_INTERPOLATE(Ratio,OneOverZ2,OneOverZ1);
				#endif
				
				break;
			case (BOTTOM_CLIPPING_PLANE):
				assert((V2->Y - V1->Y)!=0.0f);
				Ratio = (jeTClip_Statics.BottomEdge - V2->Y)/( V1->Y - V2->Y);

				NewVertex->X = LINEAR_INTERPOLATE(Ratio,(V2->X),(V1->X));
				NewVertex->Y = jeTClip_Statics.BottomEdge;
				#ifdef ONE_OVER_Z_PIPELINE
				NewVertex->Z = LINEAR_INTERPOLATE(Ratio,OneOverZ2,OneOverZ1);
				#else
				NewVertex->Z = 1.0f/LINEAR_INTERPOLATE(Ratio,OneOverZ2,OneOverZ1);
				#endif

				break;
		}

	
	{
		jeFloat OneOverZ1_Ratio;
		jeFloat OneOverZ2_Ratio;
		#ifdef ONE_OVER_Z_PIPELINE
		OneOverZ1 *= 1.0f / NewVertex->Z;
		OneOverZ2 *= 1.0f / NewVertex->Z;
		#else
		OneOverZ1 *= NewVertex->Z;
		OneOverZ2 *= NewVertex->Z;
		#endif
		OneOverZ1_Ratio = OneOverZ1 * Ratio;
		OneOverZ2_Ratio = OneOverZ2 * Ratio;

		//  the following is optimized to get rid of a handfull of multiplies. Read:
		//	NewVertex->r = LINEAR_INTERPOLATE(Ratio,(V2->r * OneOverZ2),(V1->r * OneOverZ1));

		NewVertex->r =(V2->r * OneOverZ2) + (V1->r * OneOverZ1_Ratio) - (V2->r * OneOverZ2_Ratio);
		NewVertex->g =(V2->g * OneOverZ2) + (V1->g * OneOverZ1_Ratio) - (V2->g * OneOverZ2_Ratio);
		NewVertex->b =(V2->b * OneOverZ2) + (V1->b * OneOverZ1_Ratio) - (V2->b * OneOverZ2_Ratio);
		NewVertex->a =(V2->a * OneOverZ2) + (V1->a * OneOverZ1_Ratio) - (V2->a * OneOverZ2_Ratio);
		NewVertex->u =(V2->u * OneOverZ2) + (V1->u * OneOverZ1_Ratio) - (V2->u * OneOverZ2_Ratio);
		NewVertex->v =(V2->v * OneOverZ2) + (V1->v * OneOverZ1_Ratio) - (V2->v * OneOverZ2_Ratio);
	}

}


/*}{************ TClip_TrianglePlane (New) ***********/

static void JETCF jeTClip_TrianglePlane(const JE_LVertex * TriVertex,
											int ClippingPlane)
{
uint32 OutBits = 0;

	switch(ClippingPlane)
	{
	case BACK_CLIPPING_PLANE:

		OutBits |= (TriVertex[0].Z < jeTClip_Statics.BackEdge) ? V0_OUT : 0;
		OutBits |= (TriVertex[1].Z < jeTClip_Statics.BackEdge) ? V1_OUT : 0;
		OutBits |= (TriVertex[2].Z < jeTClip_Statics.BackEdge) ? V2_OUT : 0;

	case LEFT_CLIPPING_PLANE:

		OutBits |= (TriVertex[0].X < jeTClip_Statics.LeftEdge)  ? (V0_OUT<<3) : 0;
		OutBits |= (TriVertex[1].X < jeTClip_Statics.LeftEdge)  ? (V1_OUT<<3) : 0;
		OutBits |= (TriVertex[2].X < jeTClip_Statics.LeftEdge)  ? (V2_OUT<<3) : 0;

	case RIGHT_CLIPPING_PLANE:

		OutBits |= (TriVertex[0].X > jeTClip_Statics.RightEdge) ? (V0_OUT<<6) : 0;
		OutBits |= (TriVertex[1].X > jeTClip_Statics.RightEdge) ? (V1_OUT<<6) : 0;
		OutBits |= (TriVertex[2].X > jeTClip_Statics.RightEdge) ? (V2_OUT<<6) : 0;

	case TOP_CLIPPING_PLANE:

		OutBits |= (TriVertex[0].Y < jeTClip_Statics.TopEdge) ? (V0_OUT<<9) : 0;
		OutBits |= (TriVertex[1].Y < jeTClip_Statics.TopEdge) ? (V1_OUT<<9) : 0;
		OutBits |= (TriVertex[2].Y < jeTClip_Statics.TopEdge) ? (V2_OUT<<9) : 0;

	case BOTTOM_CLIPPING_PLANE:

		OutBits |= (TriVertex[0].Y > jeTClip_Statics.BottomEdge) ?  (V0_OUT<<12) : 0;
		OutBits |= (TriVertex[1].Y > jeTClip_Statics.BottomEdge) ?  (V1_OUT<<12) : 0;
		OutBits |= (TriVertex[2].Y > jeTClip_Statics.BottomEdge) ?  (V2_OUT<<12) : 0;

	case NUM_CLIPPING_PLANES:
		break;
	}

	if ( OutBits )
	{
	JE_LVertex NewTriVertex[3];
		ClippingPlane = 0;
		for(;;)
		{
			assert(ClippingPlane < NUM_CLIPPING_PLANES);

			switch ( OutBits & 7 )
			{
				case (V_ALL_IN):  //NOT CLIPPED
					OutBits >>= 3;
					ClippingPlane ++;
					continue;

				// these all return:

				case (V0_OUT):
					NewTriVertex[0] = TriVertex[2];
					jeTClip_Split(&(NewTriVertex[1]),TriVertex+0,TriVertex+2,ClippingPlane);
					NewTriVertex[2] = TriVertex[1];

					jeTClip_TrianglePlane(NewTriVertex,ClippingPlane+1);

					NewTriVertex[0] = NewTriVertex[1];
					jeTClip_Split(&(NewTriVertex[1]),TriVertex+0,TriVertex+1,ClippingPlane);

					//<> could gain a little speed like this, but who cares?
					//	if ( ! (OutBits>>3) )
					//		goto Rasterize
					//	else
					jeTClip_TrianglePlane(NewTriVertex,ClippingPlane+1); 
					return;

				case (V1_OUT):
					NewTriVertex[0] = TriVertex[0];
					jeTClip_Split(&(NewTriVertex[1]),TriVertex+0,TriVertex+1,ClippingPlane);
					NewTriVertex[2] = TriVertex[2];

					jeTClip_TrianglePlane(NewTriVertex,ClippingPlane+1);

					NewTriVertex[0] = NewTriVertex[1];
					jeTClip_Split(&(NewTriVertex[1]),TriVertex+1,TriVertex+2,ClippingPlane);
					
					jeTClip_TrianglePlane(NewTriVertex,ClippingPlane+1); 
					return;

				case (V0_OUT + V1_OUT):
					NewTriVertex[0] = TriVertex[2];
					jeTClip_Split(&(NewTriVertex[1]),TriVertex+0,TriVertex+2,ClippingPlane);
					jeTClip_Split(&(NewTriVertex[2]),TriVertex+1,TriVertex+2,ClippingPlane);
				
					jeTClip_TrianglePlane(NewTriVertex,ClippingPlane+1); 
					return;

				case (V2_OUT):
					NewTriVertex[0] = TriVertex[1];
					jeTClip_Split(&(NewTriVertex[1]),TriVertex+1,TriVertex+2,ClippingPlane);
					NewTriVertex[2] = TriVertex[0];

					jeTClip_TrianglePlane(NewTriVertex,ClippingPlane+1);

					NewTriVertex[0] = NewTriVertex[1];
					jeTClip_Split(&(NewTriVertex[1]),TriVertex+0,TriVertex+2,ClippingPlane);

					jeTClip_TrianglePlane(NewTriVertex,ClippingPlane+1);
					return;

				case (V2_OUT + V0_OUT):
					NewTriVertex[0] = TriVertex[1];
					jeTClip_Split(&(NewTriVertex[1]),TriVertex+1,TriVertex+2,ClippingPlane);
					jeTClip_Split(&(NewTriVertex[2]),TriVertex+0,TriVertex+1,ClippingPlane);

					jeTClip_TrianglePlane(NewTriVertex,ClippingPlane+1);
					return;

				case (V2_OUT + V1_OUT):
					NewTriVertex[0] = TriVertex[0];
					jeTClip_Split(&(NewTriVertex[1]),TriVertex+0,TriVertex+1,ClippingPlane);
					jeTClip_Split(&(NewTriVertex[2]),TriVertex+0,TriVertex+2,ClippingPlane);

					jeTClip_TrianglePlane(NewTriVertex,ClippingPlane+1);
					return;

				case (V2_OUT + V1_OUT + V0_OUT):
					/* TOTALLY CLIPPED */
					return;
			}
		}
	}


	if ( jeTClip_Statics.THandle )
	{
		jeRDriver_Layer		Layer;

		Layer.THandle = jeTClip_Statics.THandle;

		assert(jeTClip_Statics.Driver);
		jeTClip_Statics.Driver->RenderMiscTexturePoly((jeTLVertex *)TriVertex,
			3,&Layer, 1, 
			jeTClip_Statics.RenderFlags | jeTClip_Statics.DefaultRenderFlags | JE_RENDER_FLAG_COUNTER_CLOCKWISE );
	}
	else
	{
		assert(jeTClip_Statics.Driver);
		jeTClip_Statics.Driver->RenderGouraudPoly((jeTLVertex *)TriVertex,3,
			jeTClip_Statics.RenderFlags | jeTClip_Statics.DefaultRenderFlags | JE_RENDER_FLAG_COUNTER_CLOCKWISE );
	}


}

/*}{*********** EOF ************/
