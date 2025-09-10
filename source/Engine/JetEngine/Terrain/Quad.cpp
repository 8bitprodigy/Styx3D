/****************************************************************************************/
/*  QUAD.C                                                                              */
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
	/*{********* BOF ******/

//#define TERRAIN_MAX_BRIGHT		(200.0f)
#define TERRAIN_MAX_BRIGHT		(254.0f)
#define TERRAIN_MAX_DLIGHT		(255.0f - TERRAIN_MAX_BRIGHT)
#define MAX_CLIP_VERTS			(40)	// 2K

/**********

startup time is rough! don't use heightmaps bigger than 257x257 !
memory use is harsh too : 14 Megs at 257

todos:

	1. use frame coherency? and geomorph to smooth pops?
			each quad has a morph status :
				none
				removing children
				adding children
			if either of the latter, it stores a morph time and an
			extra pair of lit verts containing the start & stop;
			(the QuadPoint is replaced with the current morph status)

	2. do a final backface on a per-poly basis right before the RenderMisc call (in screen space!)

	3. bug : when lights tesselate quads, the new quads pick the up attributes of their parents,
		which means some of the new quads get triangulated when they shouldn't
		(eg. when the quad hits the edge of the screen, but the new ones don't)

	4. see NextXY comment below ; maintain linked lists of the active
		points to immediately resolve t-joints

VTune results :
	Quad : all the time is in Edge-Neighbor related stuff
			(there's a mysterious MemPool spike; presumably this
			is related to the Link Push/Pop used in the Edge-Neighbor stuff !?)
			<> could use a Stack instead, which might be a bit faster
		*** could fix this by keeping NextX and NextY pointers in every point !
	Camera_ProjectArray && XFormArray_asm are taking lots of time

	all the clipping & rendering doesn't show up at all

	The Timer entry R_Driver takes a lot of time, as does Driver->EndFrame !

--------------------

counting Leaves, Nodes, and Points
	you start with
		L = 1, N = 0, P = 4  (Q = L + N)
	each refine step does *at most* N ++; L += 3; P += 5; (so Q += 4)
	thus in R steps :
		Q = 4*R + 1
		L = 3*R + 1
		N = R
		P = 5*R + 4	(or less)													

note : quads marked as ACTIVE_LEAF will be rendered

***********/

#define SIMPLIFY_ERROR_PIXELS	(0.90f)	//1.0f is a heightmap pixel value of height error

//#define DO_TIMER	// use project settings
//#define DO_REPORT

#define ERROR_MAX		(2048)	//sets the accuracy and radix size

#include <assert.h>
#include <stdlib.h>
#include <string.h>	// for memset
#include <math.h>

#include "BaseType.h"
#include "jeTypes.h"

#include "Dcommon.h"
#include "Engine.h"
#include "Engine._h"
#include "Bitmap._h" // for GetTH

#include "Camera.h"
#include "Errorlog.h"
#include "Ram.h"
#include "ExtBox.h"
#include "List.h"
#include "Log.h"
#include "Cpu.h"
#include "MemPool.h"

#include "jeFrustum.h"
#include "jePlane.h"

#include "Timer.h"
#include "Report.h"
#include "Tsc.h"

#include "Quad.h"
#include "Terrain.h"
#include "Terrain._h"

#ifndef max
#define min(a,b) (((a)<(b))?(a):(b))
#define max(a,b) (((a)>(b))?(a):(b))
#endif

/*}{*********** The QuadTree ***********/

typedef struct Quad		Quad;
typedef struct QuadTri  QuadTri;

typedef enum
{
	QUAD_NW = 0,
	QUAD_NE,
	QUAD_SE,
	QUAD_SW,
	NUM_QUADS
} EQuadPosition;

typedef enum
{
	EDGE_N = 0,
	EDGE_E,
	EDGE_S,
	EDGE_W,
	NUM_EDGES
} ENodeEdge;

typedef enum
{
	VIS_NONE=0,
	VIS_PARTIAL,
	VIS_FULL
} Vis;

typedef struct QuadPoint
{
	jeVec3d World;
	jeVec3d Normal;
	jeRGBA  Color;			// not used when tex-lighting and not dynamic lighting
							// $$ if we got rid of the cool tesselation for dynamic lighting,
							//	then we could get rid of this and just recompute the color on the fly
	uint32  DLightFlags;	// only used while in the process of dynamic lighting
							//	tracks which lights have touched us since when traversing by quad, 
							//	you can touch each point repeatedly
} QuadPoint;

struct Quad
{
	LinkNode LN;	// must be at head
	
	// all the stuff that doesn't need a full 32 bits:
	uint8 Active;
	uint8 Position;
	uint8 Pad;
	uint8 Vis;

	uint32 ClipFlags;	// necessary? can't put in on the stack when radixing...

	// all precomputed:

	// the points, children & parent could be computed from an x & y and a depth
	Quad * pParent;
	Quad * pChildren[4];
	QuadPoint * Points[4];	// QUAD_NW,QUAD_NE,QUAD_SE,QUAD_SW

	jeExtBox BBox;	// min&max of me and all my kids

	jeVec3d Normal;	//	could use * pNormal except at the lowest level

	// this is the hard-core computed stuff:
	float MaxSin2Normal; // cone of all childrens normals
	float ErrNormal,ErrIsotropic;

	/** morph info :

	int		MorphType;
	float	MorphProgress;
	jeLVertex MorphStart,MorphStop;

	**/
};

struct QuadTri
{
	LinkNode LN;	// must be at head
	QuadPoint * Points[3];
};

#define QUADTREE_SIGNATURE	((uint32)0xC0CAC01A)

struct QuadTree
{
	uint32 Signature1;

	MemPool *	QuadPool;
	MemPool *	VertexPool;
	Stack *		TheStack;
	RadixLN *	TheRadix;
	Stack *		QuadsDynamicDestroyStack;

	Quad * Root;

	jeBoolean IsTesselated;
	int NumQuads,NumPoints;

	// config info:
	int BaseDepth,MaxNumLeaves,MinError;

	uint32 Signature2;

	int TexDim;
	float XtoU,YtoV;

	const jeTerrain *Terrain; // this is super-naughty, but it's a pain in the ass to avoid
};

#define ACTIVE_LEAF (7)
#define ACTIVE_NODE (3)

/****** accessors for the Leaf/Node data ; stuff not in 'Quad' *************/

#define Quad_Point(pQuad,pos)	((pQuad)->Points[pos])
#define Quad_WorldPoint(pQuad,pos)	(&(((pQuad)->Points[pos])->World))
#define Quad_IsLeaf(pQuad)		((pQuad)->Active == ACTIVE_LEAF)
#define Quad_HasChildren(pQuad)	((pQuad)->pChildren[0] != NULL ) // quad either has 4 kids or none
#define Quad_GetPosition(pQuad)	((pQuad)->Position)
#define Quad_CenterPoint(pQuad)	(pQuad->pChildren[QUAD_SW]->Points[QUAD_NE])
#define Quad_CenterX(pQuad)		(((pQuad)->Points[QUAD_SW]->World.X + (pQuad)->Points[QUAD_SE]->World.X)*0.5f)
#define Quad_CenterY(pQuad)		(((pQuad)->Points[QUAD_SW]->World.Y + (pQuad)->Points[QUAD_NW]->World.Y)*0.5f)

/*}{*********** Macros ***********/

#define QuadPoint2LVertex(QPIn,LVertIn) do {\
				QuadPoint * QP;				\
				jeLVertex * LVert;			\
				QP = (QPIn);				\
				LVert = (LVertIn);			\
				LVert->X = QP->World.X;		\
				LVert->Y = QP->World.Y;		\
				LVert->Z = QP->World.Z;		\
				LVert->r = QP->Color.r;		\
				LVert->g = QP->Color.g;		\
				LVert->b = QP->Color.b;		\
				LVert->a = QP->Color.a;		\
				LVert->u = LVert->X * XtoU_g;\
				LVert->v = LVert->Y * YtoV_g; } while(0)

#define setmax(x,m)			x = max(x,m)
#define max3(a,b,c)			max(max(a,b),c)
#define max4(a,b,c,d)		max(d,max3(a,b,c))
#define min3(a,b,c)			min(min(a,b),c)
#define min4(a,b,c,d)		min(d,min3(a,b,c))
#define minmax(x,lo,hi)		( (x)<(lo)?(lo):( (x)>(hi)?(hi):(x)) )
#define putminmax(x,lo,hi)	x = minmax(x,lo,hi)
#define isinrange(x,lo,hi)	( (x)>=(lo) && (x)<=(hi) )

#define ispow2(X) ( ( (X) & ~(-(X)) ) == 0 )

//#ifdef WIN32
//#define swapints(a,b)	do { (a) ^= (b); (b) ^= (a); (a) ^= (b); } while(0)
//#endif
static void swapints(jeVec3d *v1, jeVec3d *v2)
{
	jeVec3d			temp;

	jeVec3d_Copy(v1, &temp);
	jeVec3d_Copy(v2, v1);
	jeVec3d_Copy(&temp, v2);
}

TIMER_VARS(Tesselate);
TIMER_VARS(Render);
TIMER_VARS(R_Driver);
TIMER_VARS(GetNeighbors);
TIMER_VARS(Triangulate);
TIMER_VARS(Vis);
TIMER_VARS(LightDynamic_Flat);
TIMER_VARS(LightDynamic_Clamp);
TIMER_VARS(LightDynamic_Sphere);

REPORT_VARS(NumDynamicQuads);
REPORT_VARS(SubdividedError);
REPORT_VARS(RenderedPolys);
REPORT_VARS(PolysPerLeaf);
REPORT_VARS(FrustumClips);
REPORT_VARS(ViewError_Max);
REPORT_VARS(ViewError_Min);
REPORT_VARS(ViewError_Clipped);
REPORT_VARS(ViewError_Backfaced);
REPORT_VARS(ViewError_Count);
REPORT_VARS(SubdividedMaxQuads);
REPORT_VARS(SubdividedMinError);
REPORT_VARS(SphereLight_QuadsLit);

REPORT(static int NumDynamicQuads);
REPORT(static int SubdividedError);
REPORT(static int RenderedPolys);
REPORT(static int FrustumClips);
REPORT(static int ViewError_Max);
REPORT(static int ViewError_Min);
REPORT(static int ViewError_Count);
REPORT(static int ViewError_Clipped);
REPORT(static int ViewError_Backfaced);
REPORT(static float PolysPerLeaf);
REPORT(static int NumLeaves);
REPORT(static int QuadsUnSimplified);

/*}{************ Protos **********/

jeBoolean QuadTree_IntersectRay(QuadTree *QT,jeVec3d *v0,jeVec3d *v1);

static void Quads_ArrayToPool(QuadTree * QT,Quad * RootQuad,Quad * QuadArray,int NumQuads,QuadPoint *PointArray,int NumPoints);
static void Quad_Simplify(Quad * pQuad,float ScaleZ,int MinDepth);
void Quad_FixBBoxes(Quad *pQuad);

static int intlog2(int x);
static void Quad_ComputeErrors(Quad * pQuad,float * HMPtr,int size,int stride);
void Quad_ComputeNormalDeviation(Quad *pQuad);

void Quad_TriangulateAndRender(Quad * pQuad);
void Quad_RenderQuadTri(Quad * pQuad,QuadPoint *v0,QuadPoint *v1,QuadPoint *v2);
void Quad_RenderQuad(Quad * pQuad);
void Quad_RenderTris(Quad * pQuad);
void Quad_RenderQuadTJ(Quad * pQuad);

static float Quad_Interplotate(Quad *pQuad,float fx,float fy);
void QuadTree_Destroy(QuadTree **pT);

void Quad_AddRadixLNChildren(RadixLN *pRadix,Quad * pQuad);
void Quad_AddRadixLNChildrenToDepth(RadixLN *pRadix,Quad * pQuad,int Depth);

void Quad_PushStackChildren(Stack * pStack,Quad * pQuad);

Vis Quad_Vis(Quad * pQuad);

void Quad_RenderNoVis(Quad * pQuad);
void Quad_RenderWithVis(Quad * pQuad);

jeBoolean TriIsCCW(QuadTri *pTri);
jeBoolean QuadTri_IsValid(QuadTri *pTri);

EQuadPosition Quad_ReflectPosAcrossEdge(EQuadPosition pos, ENodeEdge edge);
jeBoolean Quad_PosIsInEdgeDirection(EQuadPosition pos, ENodeEdge edge);
const Quad* Quad_GetEdgeNeighbor(const Quad* pQuad, ENodeEdge edge);

Link * Quad_GetEdgeNeighborPoints(const Quad *pQuad,ENodeEdge edge,int *pNumPoints);	// use Link_Destroy() when you're done
Link * Quad_GetEdgeNeighborQuads(const Quad *pQuad,ENodeEdge edge,int *pNumQuads);

Quad * QuadTree_GetQuadAtXY(const QuadTree *QT,jeFloat X,jeFloat Y);

/*}{************ Vec3d Inlines **********/
#ifdef WIN32
#define VEC_INLINE_CC __stdcall
#endif

#ifdef BUILD_BE
#define VEC_INLINE_CC
#endif

#include "Vec3d.h"

/*#if (defined(JETDLLVERSION) || defined(NDEBUG) || defined(BUILD_BE))
//{ **************************** DLL ****************************
// must use defines, not inline funcs!

#define jeVec3d_AddScaled(V1,V2,Scale,V1PlusV2Scaled)		\
do{ (V1PlusV2Scaled)->X = (V1)->X + (V2)->X*(Scale);		\
	(V1PlusV2Scaled)->Y = (V1)->Y + (V2)->Y*(Scale);		\
	(V1PlusV2Scaled)->Z = (V1)->Z + (V2)->Z*(Scale); } while(0)

#define jeVec3d_Add(V1,V2,V1PlusV2)		\
do{ (V1PlusV2)->X = (V1)->X + (V2)->X;		\
	(V1PlusV2)->Y = (V1)->Y + (V2)->Y;		\
	(V1PlusV2)->Z = (V1)->Z + (V2)->Z; } while(0)

#define jeVec3d_Subtract(V1,V2,V1MinusV2)	\
do{	(V1MinusV2)->X = (V1)->X - (V2)->X;			\
	(V1MinusV2)->Y = (V1)->Y - (V2)->Y;			\
	(V1MinusV2)->Z = (V1)->Z - (V2)->Z; } while(0)

#define jeVec3d_DotProduct(V1,V2)	\
	((V1)->X*(V2)->X + (V1)->Y*(V2)->Y + (V1)->Z*(V2)->Z)

#define jeVec3d_LengthSquared(V1)	\
	((V1)->X*(V1)->X + (V1)->Y*(V1)->Y + (V1)->Z*(V1)->Z )

#define jeVec3d_DistanceBetweenSquared(V1,V2)     \
	( ((V1)->X - (V2)->X) * ((V1)->X - (V2)->X) + \
	  ((V1)->Y - (V2)->Y) * ((V1)->Y - (V2)->Y) + \
	  ((V1)->Z - (V2)->Z) * ((V1)->Z - (V2)->Z) )

#else	//}{ ****************************



static __inline void VEC_INLINE_CC jeVec3d_AddScaled(const jeVec3d *V1, const jeVec3d *V2, jeFloat Scale, jeVec3d *V1PlusV2Scaled)
{
	V1PlusV2Scaled->X = V1->X + V2->X*Scale;
	V1PlusV2Scaled->Y = V1->Y + V2->Y*Scale;
	V1PlusV2Scaled->Z = V1->Z + V2->Z*Scale;
}

static __inline void VEC_INLINE_CC jeVec3d_Add(const jeVec3d *V1, const jeVec3d *V2, jeVec3d *V1PlusV2)
{
	V1PlusV2->X = V1->X + V2->X;
	V1PlusV2->Y = V1->Y + V2->Y;
	V1PlusV2->Z = V1->Z + V2->Z;
}

static __inline void VEC_INLINE_CC jeVec3d_Subtract(const jeVec3d *V1, const jeVec3d *V2, jeVec3d *V1MinusV2)
{
	V1MinusV2->X = V1->X - V2->X;
	V1MinusV2->Y = V1->Y - V2->Y;
	V1MinusV2->Z = V1->Z - V2->Z;
}

static __inline jeFloat VEC_INLINE_CC jeVec3d_DotProduct(const jeVec3d *V1, const jeVec3d *V2)
{
	return (V1->X*V2->X + V1->Y*V2->Y + V1->Z*V2->Z);
}

static __inline jeFloat VEC_INLINE_CC jeVec3d_LengthSquared(const jeVec3d *V1)
{
	return ( (V1)->X * (V1)->X + (V1)->Y * (V1)->Y + (V1)->Z * (V1)->Z );
}

static __inline jeFloat VEC_INLINE_CC jeVec3d_DistanceBetweenSquared(const jeVec3d *V1, const jeVec3d *V2)
{
float d,x;
	x = (V1->X - V2->X);
	d = x*x;
	x = (V1->Y - V2->Y);
	d+= x*x;
	x = (V1->Z - V2->Z);
	d+= x*x;
return d;
}

#endif	//} ****************************
*/
#define jeVec3d_AddTo(onto,val)			\
	do { (onto)->X += (val)->X; (onto)->Y += (val)->Y; (onto)->Z += (val)->Z; } while(0)

#define jeVec3d_SubtractFrom(from,val)	\
	do { (from)->X -= (val)->X; (from)->Y -= (val)->Y; (from)->Z -= (val)->Z; } while(0)

static __inline void VEC_INLINE_CC jeVec3d_Average(const jeVec3d *v1,const jeVec3d *v2,jeVec3d *v)
{
	v->X = (v1->X + v2->X) * 0.5f;
	v->Y = (v1->Y + v2->Y) * 0.5f;
	v->Z = (v1->Z + v2->Z) * 0.5f;
}


/*}{************ Create/Destroy **********/

QuadTree * QuadTree_Create(const jeTerrain *T)
{
QuadTree * QT;
int QWidth,QHeight,PWidth;
int w,h,x,y,boxsize;
Quad *pQuad,*QuadBase,*PrevQuadBase,*NextQuadBase,*RootQuad=NULL;
QuadPoint * pP;
float * HMPtr;
int NumQuads,NumPoints;
// static arrays
Quad *		Quads = NULL;
QuadPoint *	Points = NULL;

	assert(T);

	QT = (QuadTree *)jeRam_AllocateClear(sizeof(QuadTree));
	if ( ! QT )
		return NULL;

	QT->Terrain = T;

	QT->BaseDepth = 3;
	QT->MaxNumLeaves = 500;
	QT->MinError = 30;

	assert( T->HMWidth == T->HMHeight ); // pure laziness

	PWidth = T->HMWidth;		// point array dimensions
	QWidth = (T->HMWidth - 1);	// quad pyramid base dimensions
	QHeight= (T->HMHeight- 1);

	// assert W & H are a power of two + 1
	assert( ispow2(QWidth) );
	assert( ispow2(QHeight) );
	assert( QWidth >= (1<<QT->BaseDepth) ); // must have Base kids!

	QT->Signature1 = QUADTREE_SIGNATURE;
	QT->Signature2 = QUADTREE_SIGNATURE;

	NumQuads  = ( QWidth * QHeight * 4 - 1) / 3;
	NumPoints = T->HMWidth * T->HMHeight;

	pushTSC();

	QT->QuadPool = MemPool_Create(sizeof(Quad),1024,1024);
	if ( ! QT->QuadPool )
	{
		QuadTree_Destroy(&QT);
		return NULL;
	}

	QT->VertexPool = MemPool_Create(sizeof(QuadPoint),1024,1024);
	if ( ! QT->VertexPool )
	{
		QuadTree_Destroy(&QT);
		return NULL;
	}

	QT->TheStack = Stack_Create();
	if ( ! QT->TheStack )
	{
		QuadTree_Destroy(&QT);
		return NULL;
	}

	QT->QuadsDynamicDestroyStack = Stack_Create();
	if ( ! QT->QuadsDynamicDestroyStack )
	{
		QuadTree_Destroy(&QT);
		return NULL;
	}


	QT->TheRadix = RadixLN_Create(ERROR_MAX);
	if ( ! QT->TheRadix )
	{
		QuadTree_Destroy(&QT);
		return NULL;
	}

	Quads = (Quad *)jeRam_AllocateClear( NumQuads * sizeof(Quad) );
	if ( ! Quads )
	{
		QuadTree_Destroy(&QT);
		return NULL;
	}

	Points = (QuadPoint *)jeRam_AllocateClear( NumPoints * sizeof(QuadPoint) );
	if ( ! Points )
	{
		QuadTree_Destroy(&QT);
		return NULL;
	}

	#ifdef _LOG
	{
	int QuadMB,PointMB;
		QuadMB  = (NumQuads * sizeof(Quad))>>20;
		PointMB = (NumPoints * sizeof(QuadPoint))>>20;
		Log_Printf("QuadTree : Memory used: total = %d MB, quads = %d MB, points = %d MB\n",
			QuadMB + PointMB,QuadMB,PointMB);
	}
	#endif

	// set up the points
	{
	jeRGBA Color;
		pP = Points;
		w = T->HMWidth ;
		h = T->HMHeight;
		HMPtr = T->HM;
		Color.r = Color.g = Color.b = TERRAIN_MAX_BRIGHT;
		Color.a = 255.0f;
		for(y=0;y<h;y++)
		{
			for(x=0;x<w;x++)
			{
				pP->World.X = x * T->CubeSize.X;
				pP->World.Y = y * T->CubeSize.Y;
				pP->World.Z = *HMPtr++;

				pP->Color = Color;
				pP++;
			}
		}
	}

	// link up the quadtree:

	QuadBase = NULL;
	pQuad = Quads;

	w = QWidth; h = QHeight;
	while( w > 0 )
	{
		PrevQuadBase = QuadBase;
		QuadBase = pQuad;
		NextQuadBase = pQuad + w*h;
		for(y=0;y<h;y++)
		{
			for(x=0;x<w;x++)
			{
				if ( w == 1 )
				{
					assert( ! RootQuad );
					RootQuad = pQuad;
				}
				else
				{
					pQuad->pParent = NextQuadBase + (x>>1) + (y>>1)*(w>>1);
				}
				
				if ( PrevQuadBase )
				{
				Quad * ChildPtr;
					ChildPtr = PrevQuadBase + (x<<1) + (y<<1)*(w<<1);
					pQuad->pChildren[QUAD_SW] = ChildPtr;
					pQuad->pChildren[QUAD_SE] = ChildPtr+1;
					pQuad->pChildren[QUAD_NW] = ChildPtr + (w<<1);
					pQuad->pChildren[QUAD_NE] = ChildPtr+1+(w<<1);
					pQuad->Points[0] = pQuad->pChildren[0]->Points[0];
					pQuad->Points[1] = pQuad->pChildren[1]->Points[1];
					pQuad->Points[2] = pQuad->pChildren[2]->Points[2];
					pQuad->Points[3] = pQuad->pChildren[3]->Points[3];
					pQuad->pChildren[0]->Position = 0;
					pQuad->pChildren[1]->Position = 1;
					pQuad->pChildren[2]->Position = 2;
					pQuad->pChildren[3]->Position = 3;

					pQuad->BBox.Min.Z = min4(
						pQuad->pChildren[0]->BBox.Min.Z,
						pQuad->pChildren[1]->BBox.Min.Z,
						pQuad->pChildren[2]->BBox.Min.Z,
						pQuad->pChildren[3]->BBox.Min.Z);
					pQuad->BBox.Max.Z = max4( 
						pQuad->pChildren[0]->BBox.Max.Z,
						pQuad->pChildren[1]->BBox.Max.Z,
						pQuad->pChildren[2]->BBox.Max.Z,
						pQuad->pChildren[3]->BBox.Max.Z);

					assert(pQuad->pChildren[0]->pParent == pQuad);
					assert(pQuad->pChildren[1]->pParent == pQuad);
					assert(pQuad->pChildren[2]->pParent == pQuad);
					assert(pQuad->pChildren[3]->pParent == pQuad);
				}
				else
				{
					pQuad->Points[QUAD_SW] = Points + x + y*PWidth;
					pQuad->Points[QUAD_SE] = Points + x + y*PWidth + 1;
					pQuad->Points[QUAD_NW] = Points + x + (y+1)*PWidth;
					pQuad->Points[QUAD_NE] = Points + x + (y+1)*PWidth + 1;
					
					pQuad->BBox.Min.Z = min4(
											pQuad->Points[0]->World.Z,
											pQuad->Points[1]->World.Z,
											pQuad->Points[2]->World.Z,
											pQuad->Points[3]->World.Z);
					pQuad->BBox.Max.Z = max4(
											pQuad->Points[0]->World.Z,
											pQuad->Points[1]->World.Z,
											pQuad->Points[2]->World.Z,
											pQuad->Points[3]->World.Z);
				}
				
				pQuad->BBox.Min.X = pQuad->Points[QUAD_SW]->World.X;
				pQuad->BBox.Max.X = pQuad->Points[QUAD_SE]->World.X;
				pQuad->BBox.Min.Y = pQuad->Points[QUAD_SW]->World.Y;
				pQuad->BBox.Max.Y = pQuad->Points[QUAD_NW]->World.Y;

				assert( jeExtBox_IsValid(&(pQuad->BBox)) );

				pQuad++;
			}
		}

		w >>= 1; h >>= 1;
	}

	showPopTSC("QuadTree_Create : Startup");
	pushTSC();
	
	// compute the normals, extbox & gouraud

	pQuad = Quads;
	for(y=0;y<QHeight;y++)
	{
		for(x=0;x<QWidth;x++)
		{
		jeVec3d v1,v2;
			jeVec3d_Subtract(&(pQuad->Points[QUAD_SE]->World),&(pQuad->Points[QUAD_SW]->World),&v1);
			jeVec3d_Subtract(&(pQuad->Points[QUAD_NW]->World),&(pQuad->Points[QUAD_SW]->World),&v2);
			jeVec3d_CrossProduct(&v1, &v2, &(pQuad->Normal));
			jeVec3d_Normalize(&(pQuad->Normal));
			assert(pQuad->Normal.Z > 0.0f );
			
			jeVec3d_AddTo(&(pQuad->Points[0]->Normal),&(pQuad->Normal));
			jeVec3d_AddTo(&(pQuad->Points[1]->Normal),&(pQuad->Normal));
			jeVec3d_AddTo(&(pQuad->Points[2]->Normal),&(pQuad->Normal));
			jeVec3d_AddTo(&(pQuad->Points[3]->Normal),&(pQuad->Normal));

			pQuad++;
		}
	}

	pP = Points;
	for(x=NumPoints;x--;)
	{
		jeVec3d_Normalize(&(pP->Normal));
		pP++;
	}

	// propagate up the normals to the higher levels

	pQuad = Quads + QWidth * QHeight;
	w = QWidth >>1;
	h = QHeight>>1;
	while( w > 0 )
	{
		for(y=0;y<h;y++)
		{
			for(x=0;x<w;x++)
			{
				// use the gouraud normal
				assert( jeExtBox_ContainsPoint(&(pQuad->BBox),&(Quad_CenterPoint(pQuad)->World)) );
				pQuad->Normal = Quad_CenterPoint(pQuad)->Normal;
				pQuad++;
			}
		}
		w >>= 1; h >>= 1;
	}

	showPopTSC("QuadTree_Create : Make Normals");
	pushTSC();

	{
	Stack * pStack;
		pStack = QT->TheStack;
		Stack_Push(pStack,RootQuad);
		while( (pQuad = (Quad *)Stack_Pop(pStack)) != NULL  )
		{
			Quad_ComputeNormalDeviation(pQuad);
			if ( Quad_HasChildren(pQuad) )
			{
				Stack_Push(pStack,pQuad->pChildren[0]);
				Stack_Push(pStack,pQuad->pChildren[1]);
				Stack_Push(pStack,pQuad->pChildren[2]);
				Stack_Push(pStack,pQuad->pChildren[3]);
			}
		}
	}

	showPopTSC("QuadTree_Create : Compute Normal Deviations");
	pushTSC();

	// compute errors

	pQuad = Quads + QWidth * QHeight;
		// skip the lowest level, cuz they have error = 0 anyway
	w = QWidth >>1; boxsize = 2;
	h = QHeight>>1;
	while( w > 0 )
	{
		for(y=0;y<h;y++)
		{
			HMPtr = T->HM + y*boxsize*T->HMWidth;
			for(x=0;x<w;x++)
			{
				Quad_ComputeErrors(pQuad,HMPtr,boxsize+1,T->HMWidth); 
				pQuad++;
				HMPtr += boxsize;
			}
		}

		w >>= 1; h >>= 1;
		boxsize <<= 1;
	}

	showPopTSC("QuadTree_Create : Compute Errors");

	pushTSC();

	REPORT(QuadsUnSimplified=0);

	{
	float scale;
		scale = 255.0f / ( RootQuad->BBox.Max.Z - RootQuad->BBox.Min.Z + 1.0f );
		Quad_Simplify(RootQuad,scale,QT->BaseDepth);
	}

	REPORT( Log_Printf("Quads Simplified = %d = %f %%\n",(NumQuads - QuadsUnSimplified),
		(NumQuads - QuadsUnSimplified)*100.0f/NumQuads) );
		
	showPopTSC("QuadTree_Create : View-Independent Simplify");

	Quads_ArrayToPool(QT,RootQuad,Quads,NumQuads,Points,NumPoints);

	#ifdef _LOG
	{
	int QuadMB,PointMB;
		QuadMB  = MemPool_MemoryUsed(QT->QuadPool) >> 20;
		PointMB  = MemPool_MemoryUsed(QT->VertexPool) >> 20;
		Log_Printf("QuadTree : Memory used: total = %d MB, quads = %d MB, points = %d MB\n",
			QuadMB + PointMB,QuadMB,PointMB);
	}
	#endif

	// re-percolate up the BBoxes ; Simplify may have changed them

	Quad_FixBBoxes(QT->Root);

	jeRam_Free(Points);
	jeRam_Free(Quads);

return QT;
}

void QuadTree_SetParameters(QuadTree * QT,uint32 BaseDepth,uint32 MaxLeaves,float MinError)
{
	if ( BaseDepth < 3 ) BaseDepth = 3;
	QT->BaseDepth = BaseDepth;
	QT->MaxNumLeaves = MaxLeaves; // based on PolysPerLeaf = 2
	QT->MinError = jeFloat_ToInt(MinError * ERROR_MAX);
}

/*}{********* Setup Stuff ************/

void Quad_FixBBoxes(Quad *pQuad)
{
	if ( Quad_HasChildren(pQuad) )
	{
		Quad_FixBBoxes(pQuad->pChildren[0]);
		Quad_FixBBoxes(pQuad->pChildren[1]);
		Quad_FixBBoxes(pQuad->pChildren[2]);
		Quad_FixBBoxes(pQuad->pChildren[3]);

		pQuad->BBox.Min.Z = min4(
			pQuad->pChildren[0]->BBox.Min.Z,
			pQuad->pChildren[1]->BBox.Min.Z,
			pQuad->pChildren[2]->BBox.Min.Z,
			pQuad->pChildren[3]->BBox.Min.Z);
		pQuad->BBox.Max.Z = max4( 
			pQuad->pChildren[0]->BBox.Max.Z,
			pQuad->pChildren[1]->BBox.Max.Z,
			pQuad->pChildren[2]->BBox.Max.Z,
			pQuad->pChildren[3]->BBox.Max.Z);
	}
	else
	{
		pQuad->BBox.Min.Z = min4(
			pQuad->Points[0]->World.Z,
			pQuad->Points[1]->World.Z,
			pQuad->Points[2]->World.Z,
			pQuad->Points[3]->World.Z);
		pQuad->BBox.Max.Z = max4( 
			pQuad->Points[0]->World.Z,
			pQuad->Points[1]->World.Z,
			pQuad->Points[2]->World.Z,
			pQuad->Points[3]->World.Z);
	}
}

Quad * Quads_Copy(MemPool * QuadPool,Quad * OldQuad,QuadPoint *PointArray,QuadPoint ** NewPoints,int * NewPointsRefCounts)
{
Quad * NewQuad;
int i,a;

	NewQuad = (Quad *)MemPool_GetHunk(QuadPool);
	assert(NewQuad);
	*NewQuad = *OldQuad;

	for(i=0;i<4;i++)
	{
		a = ((int)(OldQuad->Points[i]) - (int)PointArray)/sizeof(QuadPoint);
		NewQuad->Points[i] = NewPoints[a];
		NewPointsRefCounts[a] ++; 
	}

	for(i=0;i<4;i++)
	{
		if ( OldQuad->pChildren[i] )
		{
			NewQuad->pChildren[i] = Quads_Copy(QuadPool,OldQuad->pChildren[i],PointArray,NewPoints,NewPointsRefCounts);
			NewQuad->pChildren[i]->pParent = NewQuad;
		}
	}

return NewQuad;
}

static void Quads_ArrayToPool(QuadTree * QT,Quad * RootQuad,Quad * QuadArray,int NumQuads,QuadPoint *PointArray,int NumPoints)
{
int i;
int * NewPointsRefCounts;
QuadPoint ** NewPoints;

	NewPoints = (QuadPoint **)malloc(4*NumPoints); assert(NewPoints);
	NewPointsRefCounts = (int *)malloc(4*NumPoints);assert(NewPointsRefCounts);

	for(i=0;i<NumPoints;i++)
	{
		NewPointsRefCounts[i] = 0;
		NewPoints[i] = (QuadPoint *)MemPool_GetHunk(QT->VertexPool);
		*(NewPoints[i]) = PointArray[i];
	}

	QT->Root = Quads_Copy(QT->QuadPool,RootQuad,PointArray,NewPoints,NewPointsRefCounts);

	for(i=0;i<NumPoints;i++)
	{
		if ( ! NewPointsRefCounts[i] )
			MemPool_FreeHunk(QT->VertexPool,NewPoints[i]);
	}

	free(NewPoints);
	free(NewPointsRefCounts);
}

static void Quad_Simplify(Quad * pQuad,float ScaleZ,int MinDepth)
{
	if ( MinDepth > 0 )
	{
		MinDepth --;
		assert( Quad_HasChildren(pQuad) );
		Quad_Simplify(pQuad->pChildren[0],ScaleZ,MinDepth);
		Quad_Simplify(pQuad->pChildren[1],ScaleZ,MinDepth);
		Quad_Simplify(pQuad->pChildren[2],ScaleZ,MinDepth);
		Quad_Simplify(pQuad->pChildren[3],ScaleZ,MinDepth);
	}
	else
	{
	float Error;
		REPORT(QuadsUnSimplified++);
		Error = max(pQuad->ErrIsotropic,pQuad->ErrNormal)*ScaleZ; // eh ?
		if ( Error < SIMPLIFY_ERROR_PIXELS )
		{
			// they'll get freed in the ArrayToPool step
			pQuad->pChildren[0] = pQuad->pChildren[1] =
				pQuad->pChildren[2] = pQuad->pChildren[3] = NULL;
		}
		else
		{
			assert( Quad_HasChildren(pQuad) );
			Quad_Simplify(pQuad->pChildren[0],ScaleZ,MinDepth);
			Quad_Simplify(pQuad->pChildren[1],ScaleZ,MinDepth);
			Quad_Simplify(pQuad->pChildren[2],ScaleZ,MinDepth);
			Quad_Simplify(pQuad->pChildren[3],ScaleZ,MinDepth);
		}
	}
}

static void Quad_ComputeErrors(Quad * pQuad,float * HMPtr,int size,int stride)
{
float MaxErrZ2;
int x,y;
float invsize = 1.0f / (size-1);

	// BTW the top level errors are very expensive to compute

	MaxErrZ2 = 0.0f;
	for(y=0;y<size;y++)
	{
		for(x=0;x<size;x++)
		{
		float z;
			z = Quad_Interplotate(pQuad,x*invsize,y*invsize);
			z -= *HMPtr++;
			z *= z;
			if ( z > MaxErrZ2 )
				MaxErrZ2 = z;
		}
		HMPtr += stride - size;
	}
	MaxErrZ2 = jeFloat_Sqrt(MaxErrZ2);
	pQuad->ErrNormal = MaxErrZ2 * pQuad->Normal.Z;
	pQuad->ErrNormal = JE_ABS(pQuad->ErrNormal);

	// this isn't really right for the isotropic error..
	
	pQuad->ErrIsotropic = MaxErrZ2 * jeFloat_Sqrt( 1.0f - (pQuad->Normal.Z * pQuad->Normal.Z));

/**
	{
	float ErrIsotropic1,ErrIsotropic2;
	ErrIsotropic1 = MaxErrZ2 * jeFloat_Sqrt( 1.0f - (pQuad->Normal.Z * pQuad->Normal.Z));
	ErrIsotropic2 = MaxErrZ2 * pQuad->MaxSin2Normal;
	pQuad->ErrIsotropic = max(ErrIsotropic1,ErrIsotropic2);
	}
**/
}

float Quad_ChildrenMinCos(Quad * pQuad,jeVec3d * pNormal)
{
float Cos;

	// this take a huge amount of time, but there's
	//	essentially no way around it!

	Cos = jeVec3d_DotProduct(pNormal,&(pQuad->Normal));

	if ( Quad_HasChildren(pQuad) )
	{
	float dot;
	int i;
		for(i=0;i<4;i++)
		{
			dot = Quad_ChildrenMinCos(pQuad->pChildren[i],pNormal);
			Cos = min(dot,Cos);
		}
	}

return Cos;
}

void Quad_ComputeNormalDeviation(Quad *pQuad)
{
float MinCos;

	/*{

	this is NlogN (where N = # of quads).
	for each quad, we must walk over all its children and do a dot product
	so we have :

	1*(N) + 4*(N/4) + 16*(N/16) ... (N/4)*4

		= N * log4(N)

	(that's a log-base-4)

	}*/

	MinCos = Quad_ChildrenMinCos(pQuad,&(pQuad->Normal));
	
	if ( MinCos < 0.0f )
	{
		pQuad->MaxSin2Normal = 1.0f;
		return;
	}

	assert(MinCos < 1.0001f);

	pQuad->MaxSin2Normal = 1.0f - MinCos*MinCos;
	if ( pQuad->MaxSin2Normal < 0.0f )
		pQuad->MaxSin2Normal = 0.0f;
}

jeBoolean QuadTree_SetTexDim(QuadTree *QT,int Dim)
{
int i,cnt;

	cnt = Dim*Dim;

	assert( QuadTree_IsValid(QT) );
	
	if ( cnt > MAX_TEXTURES )
		return JE_FALSE;

	i = 1<<(QT->BaseDepth);
	if ( (i/Dim)*Dim != i )
		return JE_FALSE;

	QT->TexDim = Dim;

	QT->XtoU = Dim / QT->Root->BBox.Max.X;
	QT->YtoV = Dim / QT->Root->BBox.Max.Y;

return JE_TRUE;
}

void QuadTree_Destroy(QuadTree **pQT)
{
QuadTree * QT;

	assert(pQT);
	QT = *pQT;
	if ( ! QT ) return;

	if ( QT->TheStack )
		Stack_Destroy(QT->TheStack);

	if ( QT->QuadsDynamicDestroyStack )
		Stack_Destroy(QT->QuadsDynamicDestroyStack);

	if ( QT->TheRadix )
		RadixLN_Destroy(QT->TheRadix);

	if ( QT->VertexPool )
		MemPool_Destroy(&(QT->VertexPool));

	if ( QT->QuadPool )
		MemPool_Destroy(&(QT->QuadPool));

	jeRam_Free(QT);
}

/*}{********* Tesselate & Render ************/

static jeFrustum Frustum_g;
static jeCamera * pCamera_g;
static jeVec3d * pCameraPos_g;
static DRV_Driver * pRDriver_g;
static Stack * Stack_g;
static jeTexture * pTHandles_g[MAX_TEXTURES];
static jeRDriver_Layer Layer_g;
static int TexDim_g;
static float XtoU_g,YtoV_g;

void Quad_ActivateNode(Quad *pQuad)
{
Quad **ppLeaves;
int i;

	assert(pQuad->Vis);
	assert(pQuad->Active == ACTIVE_LEAF);

	ppLeaves = pQuad->pChildren;
	pQuad->Active = ACTIVE_NODE;
	for(i=0;i<4;i++)
	{
		assert( ppLeaves[i] );
		ppLeaves[i]->Vis = Quad_Vis(ppLeaves[i]);
		ppLeaves[i]->Active = ACTIVE_LEAF;
	}
}

void Quad_ActivateRoot(Quad *Q)
{
	Q->ClipFlags = (1UL<<(Frustum_g.NumPlanes)) - 1UL;
	Q->Vis		= VIS_FULL;
	Q->Active	= ACTIVE_LEAF;
	Quad_ActivateNode(Q);
}

void QuadTree_DestroyDynamic(const QuadTree *QT)
{
Quad *Q,*C;

	while( ( Q = (Quad *)Stack_Pop(QT->QuadsDynamicDestroyStack) ) != NULL )
	{
	int i;
	
		// free the 5 new points in a plus shape
		C = Q->pChildren[QUAD_NW];
		MemPool_FreeHunk(QT->VertexPool,C->Points[QUAD_NE]);
		MemPool_FreeHunk(QT->VertexPool,C->Points[QUAD_SW]);
		MemPool_FreeHunk(QT->VertexPool,C->Points[QUAD_SE]);
		C = Q->pChildren[QUAD_SE];
		MemPool_FreeHunk(QT->VertexPool,C->Points[QUAD_NE]);
		MemPool_FreeHunk(QT->VertexPool,C->Points[QUAD_SW]);

		REPORT(NumDynamicQuads -= 4);

		for(i=0;i<4;i++)
		{
			C = Q->pChildren[i];
			Q->pChildren[i] = NULL;
			assert(C);
			assert( ! Quad_HasChildren(C) ); // must be a leaf
			MemPool_FreeHunk(QT->QuadPool,C);
		}

		Q->Active = ACTIVE_LEAF; // no longer a node
		// we might render again without re-tesselating!
	}
	
	REPORT(assert(NumDynamicQuads == 0 ));
}

jeBoolean QuadTree_Tesselate(QuadTree *QT,jeVec3d * pPos,jeFrustum *pFrustum)
{
RadixLN * pRadix;
Quad * pQuad;
int Error;
int RefineSteps,MaxRefineSteps,MinError;
int i;

	assert( QuadTree_IsValid(QT) );

	// this Frustum was just made from the Camera by Terrain_Tesselate

	Frustum_g = *pFrustum;
	pCameraPos_g = pPos;
	pCamera_g = NULL;
	pRDriver_g = NULL;

	REPORT(ViewError_Max=ViewError_Clipped=ViewError_Backfaced=ViewError_Count=0);
	REPORT(ViewError_Min=ERROR_MAX);

	// push nodes on a radix, and refine in order of max error

	RefineSteps = 0;
	MaxRefineSteps = QT->MaxNumLeaves/3;
	MinError = QT->MinError;

	assert(QT->BaseDepth >= 1);

	TIMER_P(Tesselate);

	// kill the old render-dynamic quads
	QuadTree_DestroyDynamic(QT);
	REPORT(NumDynamicQuads = 0);

	pRadix = QT->TheRadix;
	assert(pRadix);

	RadixLN_Reset(pRadix);
	
	Quad_ActivateRoot(QT->Root);

	assert(Quad_HasChildren(QT->Root));

	for(i=0;i<4;i++)
	{
		Quad_AddRadixLNChildrenToDepth(pRadix,QT->Root->pChildren[i],QT->BaseDepth-1);
	}

	jeCPU_FloatControl_Push();
	jeCPU_FloatControl_SinglePrecision();
//	jeCPU_FloatControl_RoundDown(); //{} ?

	while( (pQuad = (Quad *)RadixLN_CutMax(pRadix,&Error)) && Error )
	{
		RefineSteps ++;
		assert(pQuad->Active == ACTIVE_LEAF);

		if ( RefineSteps >= MaxRefineSteps || Error < MinError )
			break;
			
		Quad_AddRadixLNChildren(pRadix,pQuad);
	}

	jeCPU_FloatControl_Pop();

	TIMER_Q(Tesselate);

	#ifdef DO_REPORT //{
	{
	int SubdividedMaxQuads = 0, SubdividedMinError = 0;
		NumLeaves = RefineSteps*3;
		if ( RefineSteps >= MaxRefineSteps )
			SubdividedMaxQuads = 1;
		else
			SubdividedMinError = 1;
		REPORT_ADD(SubdividedMaxQuads);
		REPORT_ADD(SubdividedMinError);
	}
	#endif //}

	QT->IsTesselated = JE_TRUE;

	REPORT(SubdividedError = Error);
	REPORT_ADD(SubdividedError);
	REPORT_ADD(ViewError_Clipped);
	REPORT_ADD(ViewError_Backfaced);
	REPORT_ADD(ViewError_Min);
	REPORT_ADD(ViewError_Max);
	REPORT_ADD(ViewError_Count);

	return JE_TRUE;
}

jeBoolean QuadTree_Render(const QuadTree *QT,jeEngine *E,jeCamera *Cam,jeFrustum *F)
{
int i;

	assert( QuadTree_IsValid(QT) );

	if ( ! QT->IsTesselated )
		return JE_FALSE;

	REPORT(RenderedPolys=FrustumClips=0);

	Frustum_g = *F;
	pRDriver_g = jeEngine_GetDriver(E);
	pCamera_g = Cam;

	TexDim_g = QT->TexDim;
	XtoU_g = QT->XtoU;
	YtoV_g = QT->YtoV;

	for(i=0;i<MAX_TEXTURES;i++)
	{
	jeBitmap ** Textures;
		Textures = (jeBitmap **) QT->Terrain->Textures;
		if ( Textures[i] )
		{
			pTHandles_g[i] = jeBitmap_GetTHandle(Textures[i]);
			assert(pTHandles_g[i]);
		}
		else
		{
			pTHandles_g[i] = NULL;
		}
	}

	if ( QT->Terrain->HasSelection )
	{
	int sel;
	const jeTerrain * T = QT->Terrain;

		sel = T->SelectionTexX + T->SelectionTexY * T->TexDim;
		
		pTHandles_g[sel] = jeBitmap_GetTHandle( T->HiliteTexture );
		assert(pTHandles_g[sel]);
	}

	// <> could use a manual stack for the renderer

	TIMER_P(Render);

	jeCPU_FloatControl_Push();
	jeCPU_FloatControl_SinglePrecision();
	jeCPU_FloatControl_RoundDown();

	//	ClipFlags already set in Tesselate; 
	//		unfortunately that doesn't help much
	//	the rendering frustum must be smaller than the tesselating frustum, but
	//		the planes may be totally different!
	QT->Root->ClipFlags = (1UL<<(Frustum_g.NumPlanes)) - 1UL;
	Quad_RenderWithVis(QT->Root->pChildren[0]);
	Quad_RenderWithVis(QT->Root->pChildren[1]);
	Quad_RenderWithVis(QT->Root->pChildren[2]);
	Quad_RenderWithVis(QT->Root->pChildren[3]);

	jeCPU_FloatControl_Pop();

	TIMER_Q(Render);

	REPORT( if ( NumLeaves == 0 ) PolysPerLeaf = 0.0; else PolysPerLeaf = ((float)RenderedPolys / NumLeaves));
	REPORT_ADD(PolysPerLeaf);
	REPORT_ADD(FrustumClips);
	REPORT_ADD(RenderedPolys);

	REPORT( jeEngine_DebugPrintf(E, JE_COLOR_XRGB(255,255,255), "Terrain: Leaves : %d, Polys : %d, Error : %d",NumLeaves,RenderedPolys,SubdividedError) );

	return JE_TRUE;
}

/*}{********* Render Sub-Funcs ************/

// recrusively render & frustum-clip :

void Quad_RenderNoVis(Quad * pQuad)
{
	assert( pQuad->Vis || pQuad->Active == ACTIVE_LEAF );

	if ( ! pQuad->Vis )
		return;

	if ( pQuad->Active == ACTIVE_LEAF )
	{
		pQuad->ClipFlags = 0;
		Quad_RenderTris(pQuad);
	}
	else
	{
		assert(Quad_HasChildren(pQuad));
		Quad_RenderNoVis(pQuad->pChildren[0]);
		Quad_RenderNoVis(pQuad->pChildren[1]);
		Quad_RenderNoVis(pQuad->pChildren[2]);	
		Quad_RenderNoVis(pQuad->pChildren[3]);
	}
}

void Quad_RenderWithVis(Quad * pQuad)
{
Vis v;
	assert( pQuad->Vis || pQuad->Active == ACTIVE_LEAF );
	
	if ( ! pQuad->Vis ) // backfaced or outside the cam frustum
		return;

#if 1 // <>	if not, just re-use the vis from tesselation !
	// can't set it into pquad->vis cuz the next render will ref that
	v = Quad_Vis(pQuad);
#endif

	if ( v == VIS_FULL )
	{
		Quad_RenderNoVis(pQuad);
	}
	else if ( v == VIS_PARTIAL )
	{
		if ( pQuad->Active == ACTIVE_LEAF )
		{
			Quad_RenderTris(pQuad);
		}
		else
		{
			assert(Quad_HasChildren(pQuad));
			Quad_RenderWithVis(pQuad->pChildren[0]);
			Quad_RenderWithVis(pQuad->pChildren[1]);
			Quad_RenderWithVis(pQuad->pChildren[2]);
			Quad_RenderWithVis(pQuad->pChildren[3]);
		}
	}
}

void Quad_RenderTris(Quad * pQuad)
{
	if ( ! pQuad->ClipFlags )
	{
		Quad_RenderQuadTJ(pQuad);
	}
	else
	{
		Quad_TriangulateAndRender(pQuad);
	}
}

static void UnitizeVerts(jeLVertex * Verts,int nVerts)
{
int i;
float lowu,lowv;

	// <> crappy and slow!
	// necessary because the drivers CLAMP to (0,1)
	// and we want to share vertices across textures...

	lowu = Verts[0].u;
	lowv = Verts[0].v;
	for(i=1;i<nVerts;i++)
	{
		lowu = min(lowu,Verts[i].u);
		lowv = min(lowv,Verts[i].v);
	}

	lowu = jeFloat_RoundToInt(lowu + JE_EPSILON);
	lowv = jeFloat_RoundToInt(lowv + JE_EPSILON);

	while(nVerts--)
	{
		Verts->a = 255.0f;	// Frustum clip does *NOT* do alpha , just RGB
		Verts->u -= lowu;
		Verts->v -= lowv;
		assert( JE_CLAMP(Verts->u, - JE_EPSILON,1.0f + JE_EPSILON) == Verts->u );
		assert( JE_CLAMP(Verts->v, - JE_EPSILON,1.0f + JE_EPSILON) == Verts->v );
		Verts++;
	}
}

int Quad_TexNum(Quad *pQuad)
{
int u,v;
	u = jeFloat_ToInt((pQuad->Points[QUAD_SW]->World.X + pQuad->Points[QUAD_SE]->World.X)*0.5f*XtoU_g);
	v = jeFloat_ToInt((pQuad->Points[QUAD_SW]->World.Y + pQuad->Points[QUAD_NW]->World.Y)*0.5f*YtoV_g);
return (u + v * TexDim_g);
}

void Quad_RenderQuad(Quad *pQuad)
{
int32 nVerts;
uint32 ClipFlags;
jeLVertex Verts1[MAX_CLIP_VERTS],Verts2[MAX_CLIP_VERTS],*pVerts1,*pVerts2;
uint32 mask;
jePlane * pPlane;

	ClipFlags = pQuad->ClipFlags;
	assert(ClipFlags);

	QuadPoint2LVertex(pQuad->Points[QUAD_SW],Verts1+0);
	QuadPoint2LVertex(pQuad->Points[QUAD_SE],Verts1+1);
	QuadPoint2LVertex(pQuad->Points[QUAD_NW],Verts1+2);

	nVerts = 3;	
	pVerts1 = Verts1;
	pVerts2 = Verts2;
	
	for(mask=1,pPlane = Frustum_g.Planes;mask <= ClipFlags;mask += mask, pPlane++)
	{
	void * z;
		if ( ! (ClipFlags & mask) )
			continue;

		REPORT(FrustumClips++);

		if ( ! jeFrustum_ClipLVertsToPlaneXYZUVRGB(pPlane,pVerts1,pVerts2,nVerts,&nVerts) )
			goto RenderNextQuadTri;

		z		= pVerts1;
		pVerts1	= pVerts2;
		pVerts2	= (jeLVertex *)z;
	}

	// pVerts1 is the good stuff
	jeCamera_TransformAndProjectAndClampArray(pCamera_g ,
									(jeVec3d *)pVerts1, sizeof(jeLVertex),
									(jeVec3d *)pVerts1, sizeof(jeLVertex), nVerts);

/****
	<> apparently the softdrv uses the poly vert winding to do backfacing
	I just happen to send my verts clockwise *IN WORLD SPACE* which is counter clockwise in screen space
	so I have to send the reverse render flag;

	OF NOTE : in the TJoint version, I send it the opposite way!

****/
#define QUAD_RENDER_FLAGS	(JE_RENDER_FLAG_COUNTER_CLOCKWISE|JE_RENDER_FLAG_CLAMP_UV)

	TIMER_P(R_Driver);
	assert(pTHandles_g[Quad_TexNum(pQuad)]);
	UnitizeVerts(pVerts1,nVerts);
	Layer_g.THandle = pTHandles_g[Quad_TexNum(pQuad)];
	pRDriver_g->RenderMiscTexturePoly((jeTLVertex *)pVerts1,nVerts,&Layer_g,1,QUAD_RENDER_FLAGS);
	TIMER_Q(R_Driver);

RenderNextQuadTri:
	REPORT(RenderedPolys++);
	
	QuadPoint2LVertex(pQuad->Points[QUAD_SE],Verts1+0);
	QuadPoint2LVertex(pQuad->Points[QUAD_NE],Verts1+1);
	QuadPoint2LVertex(pQuad->Points[QUAD_NW],Verts1+2);
	
	nVerts = 3;	
	pVerts1 = Verts1;
	pVerts2 = Verts2;
	
	for(mask=1,pPlane = Frustum_g.Planes;mask <= ClipFlags;mask += mask, pPlane++)
	{
	void * z;
		if ( ! (ClipFlags & mask) )
			continue;

		REPORT(FrustumClips++);

		assert(nVerts <= MAX_CLIP_VERTS);

		if ( ! jeFrustum_ClipLVertsToPlaneXYZUVRGB(pPlane,pVerts1,pVerts2,nVerts,&nVerts) )
			return;

		assert(nVerts <= MAX_CLIP_VERTS);

		z		= pVerts1;
		pVerts1	= pVerts2;
		pVerts2	= (jeLVertex *)z;
	}

	// pVerts1 is the good stuff
	jeCamera_TransformAndProjectAndClampArray(pCamera_g ,
									(jeVec3d *)pVerts1, sizeof(jeLVertex),
									(jeVec3d *)pVerts1, sizeof(jeLVertex), nVerts);

	TIMER_P(R_Driver);
	assert(pTHandles_g[Quad_TexNum(pQuad)]);
	UnitizeVerts(pVerts1,nVerts);
	Layer_g.THandle = pTHandles_g[Quad_TexNum(pQuad)];
	pRDriver_g->RenderMiscTexturePoly((jeTLVertex *)pVerts1,nVerts,&Layer_g,1,QUAD_RENDER_FLAGS);
	TIMER_Q(R_Driver);

	REPORT(RenderedPolys++);
}

void Quad_RenderQuadTri(Quad *pQuad,QuadPoint *v0,QuadPoint *v1,QuadPoint *v2)
{
int32 nVerts;
jeLVertex Verts1[MAX_CLIP_VERTS],Verts2[MAX_CLIP_VERTS],*pVerts1,*pVerts2;
uint32 mask;
jePlane * pPlane;
uint32 ClipFlags;

	ClipFlags = pQuad->ClipFlags;

	nVerts = 3;
	QuadPoint2LVertex(v0,Verts1+0);
	QuadPoint2LVertex(v1,Verts1+1);
	QuadPoint2LVertex(v2,Verts1+2);
	// clip against Frustum_g ; make any number of points, or totally occlude the poly
	
	pVerts1 = Verts1;
	pVerts2 = Verts2;

	for(mask=1,pPlane = Frustum_g.Planes;mask <= ClipFlags;mask += mask, pPlane++)
	{
	void * z;
		if ( ! (ClipFlags & mask ) )
			continue;

		REPORT(FrustumClips++);

		assert(nVerts <= MAX_CLIP_VERTS);

		if ( ! jeFrustum_ClipLVertsToPlaneXYZUVRGB(pPlane,pVerts1,pVerts2,nVerts,&nVerts) )
			return;

		assert(nVerts <= MAX_CLIP_VERTS);

		z		= pVerts1;
		pVerts1	= pVerts2;
		pVerts2	= (jeLVertex *)z;
	}

	// pVerts1 is the good stuff
	jeCamera_TransformAndProjectAndClampArray(pCamera_g ,
									(jeVec3d *)pVerts1, sizeof(jeLVertex),
									(jeVec3d *)pVerts1, sizeof(jeLVertex), nVerts);

	TIMER_P(R_Driver);
	assert(pTHandles_g[Quad_TexNum(pQuad)]);
	UnitizeVerts(pVerts1,nVerts);
	Layer_g.THandle = pTHandles_g[Quad_TexNum(pQuad)];
	pRDriver_g->RenderMiscTexturePoly((jeTLVertex *)pVerts1,nVerts,&Layer_g,1,QUAD_RENDER_FLAGS);
	TIMER_Q(R_Driver);

	REPORT(RenderedPolys++);
}

	// the TJ is for T-Junction-fixing
void Quad_RenderQuadTJ(Quad * pQuad)
{
int edge;
jeLVertex LVerts[MAX_CLIP_VERTS];	// 21 LVerts is < 1K
jeLVertex *pLVert;
int NumLVerts;

	/*	
		render the whole quad

		render without tesselating, even when there are T-joints !
	
		{} this cuts *WAY* down on the poly count, down to like 1.2 per leaf!

		this is actually a kind of wierd thing to do, since these 4 points
			are probably not coplanar (in world space) ! :^)

	*/

	pLVert = LVerts;
	NumLVerts = 0;

	TIMER_P(GetNeighbors);

	for(edge=0;edge<4;edge++)
	{
	QuadPoint *CurPoint;
	int cnt;
	Link *edgePoints;

		edgePoints = (Link *)Quad_GetEdgeNeighborPoints(pQuad,(ENodeEdge)edge,&cnt);
		assert(edgePoints);
		assert(cnt >= 1);

		while( CurPoint =(QuadPoint *)Link_Pop( edgePoints ) ) 
		{
			QuadPoint2LVertex(CurPoint,pLVert); pLVert++;
			NumLVerts ++;
		}
		
		Link_Destroy(edgePoints);
	}

	TIMER_Q(GetNeighbors);

	assert(NumLVerts >= 4);
	assert(NumLVerts <= MAX_CLIP_VERTS);

	jeCamera_TransformAndProjectAndClampArray(pCamera_g ,
									(jeVec3d *)LVerts, sizeof(jeLVertex),
									(jeVec3d *)LVerts, sizeof(jeLVertex), NumLVerts);

	TIMER_P(R_Driver);
	assert(pTHandles_g[Quad_TexNum(pQuad)]);
	UnitizeVerts(LVerts,NumLVerts);
	Layer_g.THandle = pTHandles_g[Quad_TexNum(pQuad)];
	pRDriver_g->RenderMiscTexturePoly((jeTLVertex *)LVerts,NumLVerts,&Layer_g,1,JE_RENDER_FLAG_CLAMP_UV);
	TIMER_Q(R_Driver);

	REPORT(RenderedPolys++);	
}

/*}{********* View-Dependent Error ************/

int Quad_ViewError(Quad *pQuad)
{
jeVec3d CamToMe;
float dot,len;
int err;

	// computing the error has three steps :
	//	1. frustum clip versus BBox
	//	2. backface vs. normal-cone
	//	3. error measure from camera to normal & isotropic error

	assert( pQuad->Vis == Quad_Vis(pQuad) );

	if ( ! pQuad->Vis )
	{
		REPORT( ViewError_Clipped ++ );
		return 0; // out of frustum
	}

	// this null check gaurantees no nodes have null children in the future
	if ( ! Quad_HasChildren(pQuad) ) // it's a bottom leaf!
	{
		assert(pQuad->pChildren[1] == NULL);
		assert(pQuad->pChildren[2] == NULL);
		assert(pQuad->pChildren[3] == NULL);
		return 0;
	}
	assert(pQuad->pChildren[1]);
	assert(pQuad->pChildren[2]);
	assert(pQuad->pChildren[3]);

	jeVec3d_Subtract(&(Quad_CenterPoint(pQuad)->World),pCameraPos_g,&CamToMe);

	len = jeVec3d_Normalize(&CamToMe);

	dot = jeVec3d_DotProduct(&CamToMe,&(pQuad->Normal));
	// we put a little tolerance in; if we wanted to avoid this tolerance,
	//  we would have to check the dot against the four corners of the quad,
	//  intead of just the center
	// <> this makes some things not get backfaced that could be, and everyone complains about it
	if ( dot > 0.05f )
	{
		dot -= 0.05f;
		if ( (dot*dot) > pQuad->MaxSin2Normal )
		{
			REPORT( ViewError_Backfaced ++ );
			pQuad->Vis = VIS_NONE;
			return 0; // backfacing
		}
	}

	// there are two sqrts in here, but hard to avoid !?

	jeVec3d_CrossProduct(&CamToMe,&(pQuad->Normal),&CamToMe);
	dot = jeVec3d_Length(&CamToMe); // the magnitude of the crossproduct = sin of angle

	dot *= pQuad->ErrNormal;		// do is now the normal error

	dot = max(dot,pQuad->ErrIsotropic); // take larger of normal & isotropic errors

	if ( len < 0.00000001f )
		return ERROR_MAX;

	dot *= (2.0f * ERROR_MAX / len); // divide by screen-space Z

	err = jeFloat_ToInt(dot);
	err = min(err,ERROR_MAX);

	REPORT( if ( err > ViewError_Max ) ViewError_Max = err );
	REPORT( if ( err < ViewError_Min ) ViewError_Min = err );

	REPORT( ViewError_Count ++ );

	return err;
}

/*}{********* Misc ************/

void QuadTree_GetExtBox(const QuadTree * QT,jeExtBox * pBox)
{
	assert(QuadTree_IsValid(QT));
	assert(pBox);
	*pBox = QT->Root->BBox;
}

static float Quad_Interplotate(Quad *pQuad,float fx,float fy)
{
float Z;
	assert( isinrange(fx,0,1) );
	Z = (1.0f - fy) * ( (1.0f - fx) * pQuad->Points[QUAD_SW]->World.Z + fx * pQuad->Points[QUAD_SE]->World.Z )
			+ fy * ( (1.0f - fx) * pQuad->Points[QUAD_NW]->World.Z + fx * pQuad->Points[QUAD_NE]->World.Z );
	assert( Z >= (pQuad->BBox.Min.Z - 0.00001f) && Z <= (pQuad->BBox.Max.Z + 0.00001f) );
return Z;
}

void Quad_PushStackChildren(Stack * pStack,Quad * pQuad)
{
Quad **ppLeaves;

	ppLeaves = pQuad->pChildren;

	assert( *ppLeaves );
	Stack_Push(pStack,*ppLeaves); ppLeaves++;
	assert( *ppLeaves );
	Stack_Push(pStack,*ppLeaves); ppLeaves++;
	assert( *ppLeaves );
	Stack_Push(pStack,*ppLeaves); ppLeaves++;
	assert( *ppLeaves );
	Stack_Push(pStack,*ppLeaves);
}

void Quad_AddRadixLNChildrenToDepth(RadixLN *pRadix,Quad * pQuad,int Depth)
{
	assert(pQuad->Active == ACTIVE_LEAF);
	assert( pQuad->Vis == Quad_Vis(pQuad) );

	if ( ! pQuad->Vis )
		return;

	if ( Depth )
	{
	Quad **ppLeaves;
	int i;
		assert( Quad_HasChildren(pQuad) );

		ppLeaves = pQuad->pChildren;
		Depth--;
		
		Quad_ActivateNode(pQuad);

		for(i=0;i<4;i++)
		{
			assert( ppLeaves[i] );
			Quad_AddRadixLNChildrenToDepth(pRadix,ppLeaves[i],Depth);
		}				
	}
	else
	{
		if ( Quad_HasChildren(pQuad) )
		{
			Quad_AddRadixLNChildren(pRadix,pQuad);
		}
	}
}

void Quad_AddRadixLNChildren(RadixLN *pRadix,Quad * pQuad)
{
Quad **ppLeaves;
	
	Quad_ActivateNode(pQuad);

	ppLeaves = pQuad->pChildren;

	assert( *ppLeaves );
	RadixLN_AddTail(pRadix,(LinkNode *)*ppLeaves, Quad_ViewError(*ppLeaves) ); 
	ppLeaves++;
			
	assert( *ppLeaves );
	RadixLN_AddTail(pRadix,(LinkNode *)*ppLeaves, Quad_ViewError(*ppLeaves) ); 
	ppLeaves++;
			
	assert( *ppLeaves );
	RadixLN_AddTail(pRadix,(LinkNode *)*ppLeaves, Quad_ViewError(*ppLeaves) ); 
	ppLeaves++;
			
	assert( *ppLeaves );
	RadixLN_AddTail(pRadix,(LinkNode *)*ppLeaves, Quad_ViewError(*ppLeaves) ); 
}

Vis Quad_VisFrustumBBox(jeFrustum *pFrustum,uint32 * pClipFlags, jeVec3d *pMin,jeVec3d *pMax)
{
Vis vis;
jeFloat Dist;
jePlane * Plane;
uint32 ClipFlags,mask;
jeVec3d InclusionPoint,ExclusionPoint,*pNormal;

	vis = VIS_FULL;
	ClipFlags = *pClipFlags;

	assert(pMin && pMax);
	assert(pMax->X > pMin->X);
	assert(pMax->Y > pMin->Y);
	assert(pMax->Z >= pMin->Z);

	for(mask=1,Plane = pFrustum->Planes; 
		mask <= ClipFlags;
		mask += mask, Plane++)
	{
		if ( ! (ClipFlags & mask) ) continue;

		REPORT(FrustumClips++);

		pNormal = &(Plane->Normal);

		/***
	
			imagine a 2-d rectangle straddling a line, with a normal			
			the normal points towards the 'exclusion point'
			in the opposite direction is the 'inclusion point'
			if the 'exclusion' point is outside (behind the line) then the whole rect is
			if the 'inclusion' point is inside (in front of the normal) then the whole rect is
			otherwise, it's partial

		***/

#define ConditionalAssign(cond,target1,target2,src1,src2)	\
	do { if ( cond ) { target1 = src1; target2 = src2; } else { target1 = src2; target2 = src1; } } while(0)

		ConditionalAssign( (pNormal->X > 0) , ExclusionPoint.X, InclusionPoint.X, pMax->X, pMin->X );
		ConditionalAssign( (pNormal->Y > 0) , ExclusionPoint.Y, InclusionPoint.Y, pMax->Y, pMin->Y );
		ConditionalAssign( (pNormal->Z > 0) , ExclusionPoint.Z, InclusionPoint.Z, pMax->Z, pMin->Z );

		Dist = jeVec3d_DotProduct(&ExclusionPoint,pNormal) - Plane->Dist;
		if ( Dist < 0 ) // outside
		{
			vis = VIS_NONE;
			break;	// we're done
		}

		Dist = jeVec3d_DotProduct(&InclusionPoint,pNormal) - Plane->Dist;
		if ( Dist < 0 ) // outside
		{
			vis = VIS_PARTIAL;
		}
		else
		{
			// fully included, never clip here again
			ClipFlags -= mask;
		}
	}

	*pClipFlags = ClipFlags;

return vis;
}

Vis Quad_Vis(Quad * pQuad)
{
Vis v;
	TIMER_P(Vis);
	assert(pQuad->pParent);
	assert(pQuad->pParent->Vis);
	pQuad->ClipFlags = pQuad->pParent->ClipFlags;
	if ( ! pQuad->ClipFlags )
		v = VIS_FULL;
	else
		v = Quad_VisFrustumBBox(&Frustum_g,&(pQuad->ClipFlags),&(pQuad->BBox.Min),&(pQuad->BBox.Max));
	TIMER_Q(Vis);
return v;
}

static int intlog2(int x)
{
	float xf = (float)x;
	return ((*(int*)&xf) >> 23) - 127;
}

/*}{***** neighbor-finding algorithms : **************/

__inline jeBoolean Quad_PosIsInEdgeDirection(EQuadPosition pos, ENodeEdge edge)
{

	// so for Direction = W, returns true for SW and NW
	// edges are N,E,S,W		(both clockwise cycles)
	// nodes are NW,NE,SE,SW

	static const jeBoolean LogicTable[NUM_EDGES][NUM_QUADS] = {
		/*N*/	{ 1,1,0,0 },
		/*E*/	{ 0,1,1,0 },
		/*S*/	{ 0,0,1,1 },
		/*W*/	{ 1,0,0,1 }
	};

	// if ( pos == edge || pos == ((edge+1)&3) ) return true; else return false;

	return(LogicTable[edge][pos]);
}

__inline jeBoolean Quad_NodeIsInEdgeDirection(const Quad *pQuad, ENodeEdge edge)
{
	if ( ! pQuad->pParent ) return JE_FALSE;
	else
	{
		return Quad_PosIsInEdgeDirection((EQuadPosition)(Quad_GetPosition(pQuad)),edge);
	}
}

__inline EQuadPosition Quad_ReflectPosAcrossEdge(EQuadPosition pos, ENodeEdge edge)
{

	// for edge = W , exchanges SE and SW, and such
	// reflecting across E or W is the same thing, as is N or S

	static const EQuadPosition LogicTable[NUM_EDGES][NUM_QUADS] = {
	// 			  QUAD_NW, QUAD_NE, QUAD_SE, QUAD_SW,
		/*N*/	{ QUAD_SW, QUAD_SE, QUAD_NE, QUAD_NW },
		/*E*/	{ QUAD_NE, QUAD_NW, QUAD_SW, QUAD_SE },
		/*S*/	{ QUAD_SW, QUAD_SE, QUAD_NE, QUAD_NW },
		/*W*/	{ QUAD_NE, QUAD_NW, QUAD_SW, QUAD_SE }
	};

	return(LogicTable[edge][pos]);
}

__inline EQuadPosition Quad_ReflectNodeAcrossEdge(const Quad *pQuad, ENodeEdge edge)
{
	return Quad_ReflectPosAcrossEdge((EQuadPosition)(Quad_GetPosition(pQuad)),edge);
}

const Quad* Quad_GetEdgeNeighbor(const Quad* pQuad, ENodeEdge edge)
{
EQuadPosition stepDirection;

	assert((edge >= 0) && (edge < NUM_EDGES));

	if ( ! pQuad->pParent ) return NULL;

	stepDirection = (EQuadPosition)Quad_GetPosition(pQuad);

	// keep stepping to your parent, as long that step is
	//	in the direction of the desired edge

	pQuad = (const Quad *)pQuad->pParent;

	if ( Quad_PosIsInEdgeDirection(stepDirection, edge) )
		pQuad = Quad_GetEdgeNeighbor(pQuad,edge);

	if ( ! pQuad ) return NULL;

	if ( Quad_IsLeaf(pQuad) ) return pQuad;

	stepDirection = Quad_ReflectPosAcrossEdge(stepDirection, edge);

	return pQuad->pChildren[stepDirection];

return pQuad;
}

	/* before & after the edge in the clockwise cyclic order */
static const EQuadPosition EdgePointPre[NUM_EDGES] = {
	/*N*/	QUAD_NW,
	/*E*/	QUAD_NE,
	/*S*/	QUAD_SE,
	/*W*/	QUAD_SW
};
static const EQuadPosition EdgePointPost[NUM_EDGES] = {
	/*N*/	QUAD_NE,
	/*E*/	QUAD_SE,
	/*S*/	QUAD_SW,
	/*W*/	QUAD_NW
};

static const ENodeEdge EdgeOpposite[NUM_EDGES] = { 
	EDGE_S , EDGE_W, EDGE_N, EDGE_E 
};

void Quad_AddEdgePoints(const Quad * pQuad, ENodeEdge edge,Link * Points,int *pNumPoints, jeFloat low,jeFloat high)
{
	if ( ! Quad_IsLeaf(pQuad) ) 
	{
		Quad_AddEdgePoints( pQuad->pChildren[EdgePointPre[edge]] , edge,Points,pNumPoints,low,high);
		Quad_AddEdgePoints( pQuad->pChildren[EdgePointPost[edge]], edge,Points,pNumPoints,low,high);
	}
	else 
	{
	QuadPoint *pPoint;
	jeFloat coord;
		pPoint = pQuad->Points[EdgePointPre[edge]];
		switch(edge)
		{
		case EDGE_N:
		case EDGE_S:
			coord = pPoint->World.X;
			break;
		case EDGE_E:
		case EDGE_W:
			coord = pPoint->World.Y;
			break;
		}
		if ( coord > low && coord < high )
		{
			(*pNumPoints) += 1;
			Link_Push(Points, (void *)pPoint );
		}
	}
}

Link * Quad_GetEdgeNeighborPoints(const Quad *pQuad,ENodeEdge edge,int *pNumPoints)
{
Link * Points;
jeFloat low,high;

	*pNumPoints = 0;

	Points = Link_Create();
	if (! Points ) return NULL;

	Link_Push(Points, (void *)Quad_Point(pQuad, EdgePointPost[edge] ));
	(*pNumPoints) += 1;

	pQuad = Quad_GetEdgeNeighbor(pQuad,edge);
	if ( pQuad )
	{

		switch(edge)
		{
			case EDGE_N:
				high = pQuad->Points[QUAD_NE]->World.X;
				low  = pQuad->Points[QUAD_NW]->World.X;
				break;
			case EDGE_S:
				high = pQuad->Points[QUAD_SE]->World.X;
				low  = pQuad->Points[QUAD_SW]->World.X;
				break;
			case EDGE_W:
				high = pQuad->Points[QUAD_NW]->World.Y;
				low  = pQuad->Points[QUAD_SW]->World.Y;
				break;
			case EDGE_E:
				high = pQuad->Points[QUAD_NE]->World.Y;
				low  = pQuad->Points[QUAD_SE]->World.Y;
				break;
		}
		assert( high > low );

		edge = EdgeOpposite[edge];
		// goes to the *opposite* edge and walks clockwise,
		// which *counter*clockwise along our edge
		// but then we *push* on the link which makes the link come out in
		//	clockwise order on our edge !!

		Quad_AddEdgePoints(pQuad, edge, Points,pNumPoints, low,high);

	//	Link_Push(Points, (void *)Quad_Point(pQuad, EdgePointPost[edge] ));
		// we don't include the Post point of the opposite edge, 
		//	which means we're missing the *Pre* point on our edge

		// the points are pushed in order from the Pre corner to the Post corner
	}

return Points;
}

void Quad_AddEdgeQuads(const Quad * pQuad, ENodeEdge edge,Link * Quads,int *pNumQuads, jeFloat low,jeFloat high)
{
	if ( ! Quad_IsLeaf(pQuad) ) 
	{
		Quad_AddEdgeQuads( pQuad->pChildren[EdgePointPre[edge]] , edge,Quads,pNumQuads,low,high);
		Quad_AddEdgeQuads( pQuad->pChildren[EdgePointPost[edge]], edge,Quads,pNumQuads,low,high);
	}
	else 
	{
	jeFloat coord1,coord2;
	jeVec3d *pt1,*pt2;
		pt1 = &(pQuad->Points[EdgePointPre[edge]]->World);
		pt2 = &(pQuad->Points[EdgePointPost[edge]]->World);
		switch(edge)
		{
		case EDGE_N:
		case EDGE_S:
			coord1 = pt1->X;
			coord2 = pt2->X;
			break;
		case EDGE_E:
		case EDGE_W:
			coord1 = pt1->Y;
			coord2 = pt2->Y;
			break;
		}
		if ( ! isinrange(coord1,low,high) && ! isinrange(coord2,low,high) )
			return;

		(*pNumQuads) += 1;
		Link_Push(Quads, (void *)pQuad );
	}
}

Link * Quad_GetEdgeNeighborQuads(const Quad *pQuad,ENodeEdge edge,int *pNumQuads)
{
Link * Quads;
jeFloat low,high;

	*pNumQuads = 0;

	Quads = Link_Create();
	if (! Quads ) return NULL;

	pQuad = Quad_GetEdgeNeighbor(pQuad,edge);
	if ( pQuad )
	{

		switch(edge)
		{
			case EDGE_N:
				high = pQuad->Points[QUAD_NE]->World.X;
				low  = pQuad->Points[QUAD_NW]->World.X;
				break;
			case EDGE_S:
				high = pQuad->Points[QUAD_SE]->World.X;
				low  = pQuad->Points[QUAD_SW]->World.X;
				break;
			case EDGE_W:
				high = pQuad->Points[QUAD_NW]->World.Y;
				low  = pQuad->Points[QUAD_SW]->World.Y;
				break;
			case EDGE_E:
				high = pQuad->Points[QUAD_NE]->World.Y;
				low  = pQuad->Points[QUAD_SE]->World.Y;
				break;
		}
		assert( high > low );

		edge = EdgeOpposite[edge];
		Quad_AddEdgeQuads(pQuad, edge, Quads,pNumQuads, low,high);
	}

return Quads;
}

const Quad* Quad_GetEdgeNeighbor_ManualStack(const Quad* pQuad, ENodeEdge edge)
{
EQuadPosition stepDirection;
Stack * pGetEdgeNeighborStack = NULL;

	assert((edge >= 0) && (edge < NUM_EDGES));

	if ( ! pGetEdgeNeighborStack )
		pGetEdgeNeighborStack = Stack_Create();

	// keep stepping to your parent, as long that step is
	//	in the direction of the desired edge

	for(;;)
	{
		if ( ! pQuad->pParent ) break;

		stepDirection = (EQuadPosition)Quad_GetPosition(pQuad);

		if ( ! Quad_PosIsInEdgeDirection(stepDirection, edge) ) break;

		Stack_Push(pGetEdgeNeighborStack, (void *)(Quad_ReflectPosAcrossEdge(stepDirection, edge) + 1) );
		pQuad = (const Quad *)pQuad->pParent;
	}

	if ( ! pQuad->pParent ) return NULL;

	// at the top you always step to a gauranteed sibling
	pQuad = pQuad->pParent->pChildren[ Quad_ReflectNodeAcrossEdge(pQuad, edge) ];

	// now step down

	// Port ? Does this work properly?
	
	while( (stepDirection = (EQuadPosition)((int)Stack_Pop(pGetEdgeNeighborStack) - 1)) >= 0 )
	{
		if ( Quad_IsLeaf(pQuad) ) break;

		pQuad = pQuad->pChildren[stepDirection];
	}

	Stack_Destroy(pGetEdgeNeighborStack);	pGetEdgeNeighborStack = NULL;

return pQuad;
}

/*}{************ Triangulate **********/

void Quad_TriangulateAndRender(Quad * pQuad)
{
Link *edges[4];
int edge,edgePoints[4],totPoints,PointsLeft;
QuadPoint *CurPoint,*LastPoint;
QuadPoint *CornerSE,*FirstWest,*LastNorth;

	TIMER_P(Triangulate);

	TIMER_P(GetNeighbors);

	totPoints = 0;
	for(edge=0;edge<4;edge++)
	{
		edges[edge] = Quad_GetEdgeNeighborPoints(pQuad,(ENodeEdge)edge,&(edgePoints[edge]));
		assert(edges[edge]);
		if ( edgePoints[edge] ) edgePoints[edge]--; // don't count the corner
		totPoints += edgePoints[edge];
	}
	assert(edge == 4);

	TIMER_Q(GetNeighbors);

	// && could detect this early-out case more elegantly (avoid Link_ creates & whatnot)

	if ( totPoints == 0 ) // no intersecting points
	{
		for(edge=0;edge<4;edge++)
		{
			if ( edges[edge] ) 
				Link_Destroy(edges[edge]);
		}
		
		Quad_RenderQuad(pQuad);

		TIMER_Q(Triangulate);	

		return;
	}

	CornerSE = pQuad->Points[QUAD_SE];
	// we make a fan around the SE corner

	// walk from SE to SW

	FirstWest = (QuadPoint *)Link_Pop( edges[EDGE_W] );
	assert(FirstWest !=  pQuad->Points[QUAD_SW]);
	LastPoint = CornerSE;
	while( CurPoint = (QuadPoint *)Link_Pop( edges[EDGE_S] ) ) 
	{
		assert( JE_FLOATS_EQUAL( CurPoint->World.Y, LastPoint->World.Y ) );
		assert(LastPoint != pQuad->Points[QUAD_SW] );
		Quad_RenderQuadTri(pQuad,FirstWest,CurPoint,LastPoint);
		LastPoint = CurPoint;
	}
	assert(LastPoint == pQuad->Points[QUAD_SW] );

	// walk from SW to NW

	LastPoint = FirstWest;
	while( CurPoint = (QuadPoint *)Link_Pop( edges[EDGE_W] ) )
	{
		assert( JE_FLOATS_EQUAL( CurPoint->World.X, LastPoint->World.X ) );
		assert(LastPoint != pQuad->Points[QUAD_NW] );
		Quad_RenderQuadTri(pQuad,CurPoint,LastPoint,CornerSE);
		LastPoint = CurPoint;
	}
	assert(LastPoint == pQuad->Points[QUAD_NW] );

	// walk from NW to NE

	PointsLeft = edgePoints[EDGE_N];
	while( PointsLeft && (CurPoint = (QuadPoint *)Link_Pop( edges[EDGE_N] )) )
	{
		assert( JE_FLOATS_EQUAL( CurPoint->World.Y, LastPoint->World.Y ) );
		assert(LastPoint != pQuad->Points[QUAD_NE] );
		Quad_RenderQuadTri(pQuad,CurPoint,LastPoint,CornerSE);
		LastPoint = CurPoint;
		PointsLeft--;
	}
	assert(PointsLeft == 0);
	assert(LastPoint != pQuad->Points[QUAD_NE] );
	LastNorth = LastPoint; // didn't go all the way to the end

	LastPoint = pQuad->Points[QUAD_NE];
	// walk from NE to SE
	while( CurPoint = (QuadPoint *)Link_Pop( edges[EDGE_E] ) )
	{
		assert( JE_FLOATS_EQUAL( CurPoint->World.X, LastPoint->World.X ) );
		assert(LastPoint != pQuad->Points[QUAD_SE] );
		Quad_RenderQuadTri(pQuad,LastNorth,CurPoint,LastPoint);
		LastPoint = CurPoint;
	}
	assert(LastPoint == pQuad->Points[QUAD_SE] );

	for(edge=0;edge<4;edge++)
	{
		if ( edges[edge] ) 
			Link_Destroy(edges[edge]);
	}
	
	TIMER_Q(Triangulate);	
}

/*}{************ Debug **********/

jeBoolean QuadTri_IsValid(QuadTri *pTri)
{
	if ( pTri->Points[0]->World.X == pTri->Points[1]->World.X &&
		 pTri->Points[0]->World.X == pTri->Points[2]->World.X )
		 return  JE_FALSE;
	if ( pTri->Points[0]->World.Y == pTri->Points[1]->World.Y &&
		 pTri->Points[0]->World.Y == pTri->Points[2]->World.Y )
		 return  JE_FALSE;
return JE_TRUE;
}

jeBoolean QuadTree_IsValid(const QuadTree *QT)
{
	assert(QT);
	assert(QT->Signature1 == QUADTREE_SIGNATURE);
	assert(QT->Signature2 == QUADTREE_SIGNATURE);
return JE_TRUE;
}

#if 1 //{
#include <stdio.h>

void QuadTree_ShowStats(const QuadTree *QT)
{
#ifdef DO_TIMER
	timerFP = fopen("quad.log","wt");
	assert(timerFP);
	TIMER_REPORT(Tesselate);
	TIMER_REPORT(Vis);
	TIMER_REPORT(Render);
	TIMER_REPORT(R_Driver);
	TIMER_REPORT(Triangulate);
	TIMER_REPORT(GetNeighbors);
	TIMER_REPORT(LightDynamic_Flat);
	TIMER_REPORT(LightDynamic_Clamp);
	TIMER_REPORT(LightDynamic_Sphere);

	REPORT(reportFP = timerFP);
	REPORT_REPORT(NumDynamicQuads);
	REPORT_REPORT(SphereLight_QuadsLit);
	REPORT_REPORT(RenderedPolys);
	REPORT_REPORT(FrustumClips);
	REPORT_REPORT(ViewError_Max);
	REPORT_REPORT(ViewError_Min);
	REPORT_REPORT(ViewError_Clipped);
	REPORT_REPORT(ViewError_Backfaced);
	REPORT_REPORT(ViewError_Count);
	REPORT_REPORT(SubdividedMaxQuads);
	REPORT_REPORT(SubdividedMinError);
	REPORT_REPORT(PolysPerLeaf);
	REPORT_REPORT(SubdividedError);

#ifdef _DEBUG
	jeRam_ShowStats(timerFP);
#endif

	fclose(timerFP);
	timerFP = stdin;
#endif
}
#endif //}

/*}{************ Lighting **********/

void QuadTree_ResetAllVertexLighting(QuadTree * QT)
{
Stack *pStack;
Quad * pQuad;
jeRGBA Color;

	// just Flat light !
	// this is really just to clear out the old dynamic lights

	Color.r = Color.g = Color.b = TERRAIN_MAX_BRIGHT;
	Color.a = 255.0f;

	pStack = QT->TheStack;
	Stack_Push(pStack,QT->Root);

	while( (pQuad = (Quad *)Stack_Pop(pStack)) != NULL )
	{
		if ( Quad_HasChildren(pQuad) )
		{
			Stack_Push(pStack,pQuad->pChildren[0]);
			Stack_Push(pStack,pQuad->pChildren[1]);
			Stack_Push(pStack,pQuad->pChildren[2]);
			Stack_Push(pStack,pQuad->pChildren[3]);
		}
		else
		{
			pQuad->Points[0]->Color = Color;
			pQuad->Points[1]->Color = Color;
			pQuad->Points[2]->Color = Color;
			pQuad->Points[3]->Color = Color;
		}
	}
}

void QuadTree_ApplyDynamicLights_Flat(QuadTree * QT,jeTerrain_Light * LightsArray,int LightArrayLen)
{
Stack *pStack;
Quad * pQuad;
jeRGBA Color;

	// just Flat light !
	// this is really just to clear out the old dynamic lights

	TIMER_P(LightDynamic_Flat);

	Color.r = Color.g = Color.b = TERRAIN_MAX_BRIGHT;
	Color.a = 255.0f;

	pStack = QT->TheStack;
	Stack_Push(pStack,QT->Root);

	while( (pQuad = (Quad *)Stack_Pop(pStack)) != NULL )
	{
		if ( pQuad->Active == ACTIVE_NODE )
		{
			Stack_Push(pStack,pQuad->pChildren[0]);
			Stack_Push(pStack,pQuad->pChildren[1]);
			Stack_Push(pStack,pQuad->pChildren[2]);
			Stack_Push(pStack,pQuad->pChildren[3]);
		}
		else if ( pQuad->Vis )
		{
			assert(pQuad->Active == ACTIVE_LEAF);
			pQuad->Points[0]->Color = Color;
			pQuad->Points[1]->Color = Color;
			pQuad->Points[2]->Color = Color;
			pQuad->Points[3]->Color = Color;
			pQuad->Points[0]->DLightFlags = 
			pQuad->Points[1]->DLightFlags = 
			pQuad->Points[2]->DLightFlags = 
			pQuad->Points[3]->DLightFlags = 0xFFFFFFFF;
		}
	}
	
	TIMER_Q(LightDynamic_Flat);
}

void QuadTree_ApplyDynamicLights_Clamp(QuadTree * QT)
{
Stack *pStack;
Quad * pQuad;

	TIMER_P(LightDynamic_Clamp);

	pStack = QT->TheStack;
	Stack_Push(pStack,QT->Root);

	while( (pQuad = (Quad *)Stack_Pop(pStack)) != NULL )
	{
		if ( pQuad->Active == ACTIVE_NODE )
		{
			Stack_Push(pStack,pQuad->pChildren[0]);
			Stack_Push(pStack,pQuad->pChildren[1]);
			Stack_Push(pStack,pQuad->pChildren[2]);
			Stack_Push(pStack,pQuad->pChildren[3]);
		}
		else if ( pQuad->Vis )
		{
		int p;
			assert(pQuad->Active == ACTIVE_LEAF);
			for(p=0;p<4;p++)
			{
			jeRGBA *pColor;
				pColor = &(pQuad->Points[p]->Color);
				pColor->r = JE_CLAMP(pColor->r,0.0f,255.0f);
				pColor->g = JE_CLAMP(pColor->g,0.0f,255.0f);
				pColor->b = JE_CLAMP(pColor->b,0.0f,255.0f);
			}
		}
	}
	
	TIMER_Q(LightDynamic_Clamp);
}

__inline jeBoolean JETCC PointInRectXY(const jeExtBox * BBox,const jeVec3d * V,jeFloat Radius)
{
jeFloat D;

	// <> all these ifs might be slower than some
	//		more mathematical method

	if ( V->X > BBox->Max.X )
	{
		D = V->X - BBox->Max.X;
		if ( D > Radius )
			return JE_FALSE;
	}
	else if ( V->X < BBox->Min.X )
	{
		D = BBox->Min.X - V->X; 
		if ( D > Radius )
			return JE_FALSE;
	}
	// else it's in the middle

	if ( V->Y > BBox->Max.Y )
	{
		D = V->Y - BBox->Max.Y;
		if ( D > Radius )
			return JE_FALSE;
	}
	else if ( V->Y < BBox->Min.Y )
	{
		D = BBox->Min.Y - V->Y; 
		if ( D > Radius )
			return JE_FALSE;
	}

	return JE_TRUE;
}

float Interp4(float fx,float fy,float NW,float NE,float SE,float SW)
{
float E,W;
	E = ( SE + fy * (NE - SE) );
	W = ( SW + fy * (NW - SW) );
return W + fx * (E - W);
}

void Vertex_QuadInterpColors(const jeExtBox * BBox,const jeVec3d * V,jeRGBA * CornerColors,jeRGBA *InterpColor)
{
float fx,fy;

	fx = (V->X - BBox->Min.X) / (BBox->Max.X - BBox->Min.X);
	fy = (V->Y - BBox->Min.Y) / (BBox->Max.Y - BBox->Min.Y);

	assert( fx >= 0.0f && fx <= 1.0f );
	assert( fy >= 0.0f && fy <= 1.0f );

	InterpColor->r = Interp4(fx,fy,CornerColors[0].r,CornerColors[1].r,CornerColors[2].r,CornerColors[3].r);
	InterpColor->g = Interp4(fx,fy,CornerColors[0].g,CornerColors[1].g,CornerColors[2].g,CornerColors[3].g);
	InterpColor->b = Interp4(fx,fy,CornerColors[0].b,CornerColors[1].b,CornerColors[2].b,CornerColors[3].b);

}

void GetPointClosestToRectXY(const jeExtBox * BBox,const jeVec3d * V,jeVec3d *pGot)
{
	// this is really trival :
	assert( BBox->Max.X > BBox->Min.X );
	assert( BBox->Max.Y > BBox->Min.Y );
	pGot->X = JE_CLAMP(V->X,BBox->Min.X,BBox->Max.X);
	pGot->Y = JE_CLAMP(V->Y,BBox->Min.Y,BBox->Max.Y);
}

jeFloat DistanceSquaredToClosestXY(const jeExtBox * BBox,const jeVec3d * V)
{
float X,Y;
	// this is really trival :
	assert( BBox->Max.X > BBox->Min.X );
	assert( BBox->Max.Y > BBox->Min.Y );
	X = JE_CLAMP(V->X,BBox->Min.X,BBox->Max.X);
	Y = JE_CLAMP(V->Y,BBox->Min.Y,BBox->Max.Y);
	X -= V->X;
	Y -= V->Y;
return X*X + Y*Y;
}

QuadPoint *Vert_CreateAverage(QuadTree *QT,const QuadPoint *v1,const QuadPoint *v2)
{
QuadPoint * v;
	v = (QuadPoint *)MemPool_GetHunk(QT->VertexPool);
//	jeVec3d_Average(&(v1->Normal),&(v2->Normal),&(v->Normal));
//	jeVec3d_Normalize(&(v->Normal)); 
//{} normal never used ?
#ifdef _DEBUG
	jeVec3d_Clear(&(v->Normal));
#endif
	jeVec3d_Average(&(v1->World ),&(v2->World ),&(v->World ));
	v->World.Z  = jeTerrain_GetHeightAtXY(QT->Terrain,v->World.X,v->World.Y);
	v->Color.r	= v->Color.g = v->Color.b = TERRAIN_MAX_BRIGHT;
	v->Color.a	= 255.0f;
	v->DLightFlags = 0xFFFFFFFF;
return v;
}

void Quad_CreateDynamicChildren(QuadTree * QT,Quad *pQuad)
{
QuadPoint *vN,*vS,*vW,*vE,*vC;
QuadPoint *EdgePoints[4];
int i;

	// create the children
	//	they will not have subdivide info
	//	add their parents to a stack of nodes whose kids should be deleted
	//	allocate the vertices on the pool & just don't bother sharing them

	assert( pQuad->Active == ACTIVE_LEAF );
	assert( pQuad->Vis );
	assert( pQuad->Vis == Quad_Vis(pQuad) );

	vN = Vert_CreateAverage(QT,pQuad->Points[QUAD_NW],pQuad->Points[QUAD_NE]);
	vS = Vert_CreateAverage(QT,pQuad->Points[QUAD_SW],pQuad->Points[QUAD_SE]);
	vE = Vert_CreateAverage(QT,pQuad->Points[QUAD_NE],pQuad->Points[QUAD_SE]);
	vW = Vert_CreateAverage(QT,pQuad->Points[QUAD_NW],pQuad->Points[QUAD_SW]);
	vC = Vert_CreateAverage(QT,vN,vS);

	EdgePoints[QUAD_NW] = vN;
	EdgePoints[QUAD_NE] = vE;
	EdgePoints[QUAD_SE] = vS;
	EdgePoints[QUAD_SW] = vW;

	for(i=0;i<4;i++)
	{
	Quad * Q;

		assert(pQuad->pChildren[i] == NULL);
		Q = (Quad *)MemPool_GetHunk(QT->QuadPool);
		assert(Q);
		pQuad->pChildren[i] = Q;
		*Q = *pQuad;

		Q->pParent = pQuad;
		Q->pChildren[0] = Q->pChildren[1] = Q->pChildren[2] = Q->pChildren[3] = NULL;

		Q->Position = i;

		// Q->Points[i] stays the same
		Q->Points[(i + 2)&3] = vC;
		Q->Points[(i + 1)&3] = EdgePoints[i];
		Q->Points[(i + 3)&3] = EdgePoints[(i+3)&3];

		// fix the bbox
		Q->BBox.Min.X = Q->Points[QUAD_SW]->World.X;
		Q->BBox.Max.X = Q->Points[QUAD_SE]->World.X;
		Q->BBox.Min.Y = Q->Points[QUAD_SW]->World.Y;
		Q->BBox.Max.Y = Q->Points[QUAD_NW]->World.Y;

		//Q->Vis set in ActivateNode
	}

	REPORT(NumDynamicQuads += 4);	
	Stack_Push(QT->QuadsDynamicDestroyStack,pQuad);
}

jeBoolean ThreePointsAreColinearXY(jeVec3d *p0,jeVec3d *p1,jeVec3d *p2)
{
jeVec3d v0,v1;
jeFloat Len;
	jeVec3d_Subtract(p1,p0,&v0);
	jeVec3d_Subtract(p2,p1,&v1);
	v0.Z = v1.Z = 0.0f; // kill the Z's
	jeVec3d_CrossProduct(&v0,&v1,&v0);
	Len = jeVec3d_LengthSquared(&v0);
return JE_FLOAT_ISZERO(Len);
}

void QuadTree_ApplyDynamicLight_Sphere_Subdividing(QuadTree * QT,jeTerrain_Light * pLight,uint32 Mask)
{
Stack *pStack;
Quad * pQuad;
jeVec3d LightPos;
jeRGBA LightColor;
jeFloat LightRadius,LightRadiusSquared;
jeFloat ErrorToleranceSquared;
REPORT(int SphereLight_QuadsLit = 0);


	TIMER_P(LightDynamic_Sphere);

	LightPos = pLight->Vector;
	LightColor = pLight->Color;
	LightRadiusSquared = pLight->MaxColor;
	LightRadius = jeFloat_Sqrt(LightRadiusSquared);

	ErrorToleranceSquared = jeFloat_Sqr( QT->MinError * 0.3f );
	ErrorToleranceSquared = max(ErrorToleranceSquared, 2.0f );

	pStack = QT->TheStack;
	Stack_Push(pStack,QT->Root);

	while( (pQuad = (Quad *)Stack_Pop(pStack)) != NULL )
	{
		if ( ! pQuad->Vis )
			continue;

		if ( ! PointInRectXY(&(pQuad->BBox),&LightPos,LightRadius) )
			continue;

		assert( pQuad->pParent == NULL || pQuad->Vis == Quad_Vis(pQuad) );

		if ( pQuad->Active == ACTIVE_NODE )
		{
			assert( pQuad->Vis );
			Stack_Push(pStack,pQuad->pChildren[0]);
			Stack_Push(pStack,pQuad->pChildren[1]);
			Stack_Push(pStack,pQuad->pChildren[2]);
			Stack_Push(pStack,pQuad->pChildren[3]);
		}
		else	// vis done in CreateDynamic now
		{
		jeRGBA CornerColors[4];
		int p;

			assert( pQuad->Active == ACTIVE_LEAF );

			REPORT(SphereLight_QuadsLit++);

			for(p=0;p<4;p++)
			{
			float d;
				d = jeVec3d_DistanceBetweenSquared(&(pQuad->Points[p]->World),&LightPos);
				d = 1.0f / d;
				CornerColors[p].r = d * LightColor.r;
				CornerColors[p].g = d * LightColor.g;
				CornerColors[p].b = d * LightColor.b;
				
				CornerColors[p].r = JE_CLAMP(CornerColors[p].r,-TERRAIN_MAX_DLIGHT,TERRAIN_MAX_DLIGHT);
				CornerColors[p].g = JE_CLAMP(CornerColors[p].g,-TERRAIN_MAX_DLIGHT,TERRAIN_MAX_DLIGHT);
				CornerColors[p].b = JE_CLAMP(CornerColors[p].b,-TERRAIN_MAX_DLIGHT,TERRAIN_MAX_DLIGHT);
			}

			// should we subdivide ?

			{
			float d;
			jeVec3d EdgePos,*PrevPos,*NextPos;
			jeRGBA TrueColor,InterpColor,*PrevColor,*NextColor;
				
				// check lighting at edge midpoints vs. interpolated lighting
				// this catches the case where one point is being lit right, but the
				//  linear interp of gouraud isnt right	
				for(p=0;p<4;p++)
				{
					PrevPos = &(pQuad->Points[ p       ]->World);	// plus 3 is the same as minus 1 in mod-4 :^)
					NextPos = &(pQuad->Points[ (p+1)&3 ]->World);
					PrevColor = CornerColors + p;
					NextColor = CornerColors + ((p+1)&3);
					
					EdgePos.X = (NextPos->X + PrevPos->X)*0.5f;
					EdgePos.Y = (NextPos->Y + PrevPos->Y)*0.5f;
					EdgePos.Z = (NextPos->Z + PrevPos->Z)*0.5f;
					
					assert( ThreePointsAreColinearXY( PrevPos,&EdgePos,NextPos ) );

					d = jeVec3d_DistanceBetweenSquared(&EdgePos,&LightPos);
					d = 1.0f / d;
					TrueColor.r = d * LightColor.r;
					TrueColor.g = d * LightColor.g;
					TrueColor.b = d * LightColor.b;

					TrueColor.r = JE_CLAMP(TrueColor.r,-TERRAIN_MAX_DLIGHT,TERRAIN_MAX_DLIGHT);
					TrueColor.g = JE_CLAMP(TrueColor.g,-TERRAIN_MAX_DLIGHT,TERRAIN_MAX_DLIGHT);
					TrueColor.b = JE_CLAMP(TrueColor.b,-TERRAIN_MAX_DLIGHT,TERRAIN_MAX_DLIGHT);

					InterpColor.r = (PrevColor->r + NextColor->r)*0.5f;
					InterpColor.g = (PrevColor->g + NextColor->g)*0.5f;
					InterpColor.b = (PrevColor->b + NextColor->b)*0.5f;

					d = jeVec3d_DistanceBetweenSquared((jeVec3d *)&TrueColor,(jeVec3d *)&InterpColor); // naughty !

					if ( d > ErrorToleranceSquared )
					{
						goto DoSubdivide;
					}
				}
				
				// check the closest point's lighting versus its true lighting
				if ( 1 )
				{
					GetPointClosestToRectXY(&(pQuad->BBox),&LightPos,&EdgePos);

					EdgePos.Z = jeTerrain_GetHeightAtXY(QT->Terrain,EdgePos.X,EdgePos.Y);

					d = jeVec3d_DistanceBetweenSquared(&EdgePos,&LightPos);
					d = 1.0f / d;
					TrueColor.r = d * LightColor.r;
					TrueColor.g = d * LightColor.g;
					TrueColor.b = d * LightColor.b;
					
					TrueColor.r = JE_CLAMP(TrueColor.r,-TERRAIN_MAX_DLIGHT,TERRAIN_MAX_DLIGHT);
					TrueColor.g = JE_CLAMP(TrueColor.g,-TERRAIN_MAX_DLIGHT,TERRAIN_MAX_DLIGHT);
					TrueColor.b = JE_CLAMP(TrueColor.b,-TERRAIN_MAX_DLIGHT,TERRAIN_MAX_DLIGHT);

					Vertex_QuadInterpColors(&(pQuad->BBox),&EdgePos,CornerColors,&InterpColor);

					d = jeVec3d_DistanceBetweenSquared((jeVec3d *)&TrueColor,(jeVec3d *)&InterpColor); // naughty !

					if ( d > ErrorToleranceSquared )
					{
						goto DoSubdivide;
					}
				}

				if ( 0 )
				{
				// @@ when lights tesselate, they pick up attributes from parents,
				//	   which means some get triangulated when they shouldn't

				DoSubdivide:
					assert(pQuad->Active == ACTIVE_LEAF);
					if ( ! Quad_HasChildren(pQuad) )
					{
						Quad_CreateDynamicChildren(QT,pQuad);
					}
					assert(pQuad->Active == ACTIVE_LEAF);
					Quad_ActivateNode(pQuad);
					Stack_Push(pStack,pQuad->pChildren[0]);
					Stack_Push(pStack,pQuad->pChildren[1]);
					Stack_Push(pStack,pQuad->pChildren[2]);
					Stack_Push(pStack,pQuad->pChildren[3]);
					continue;
				}
			}

			for(p=0;p<4;p++)
			{
			QuadPoint * QP;
				QP = pQuad->Points[p];
				if ( QP->DLightFlags & Mask )
				{
				jeRGBA *pColor;
					QP->DLightFlags ^= Mask;
					assert ( ! (QP->DLightFlags & Mask) );
					pColor = &(QP->Color);
					pColor->r += CornerColors[p].r;
					pColor->g += CornerColors[p].g;
					pColor->b += CornerColors[p].b;
				}
			}
		}
	}
	
	REPORT_ADD(SphereLight_QuadsLit);

	TIMER_Q(LightDynamic_Sphere);
}

void QuadTree_ApplyDynamicLight_Sphere(QuadTree * QT,jeTerrain_Light * pLight,uint32 Mask)
{
Stack *pStack;
Quad * pQuad;
jeVec3d LightPos;
jeRGBA LightColor;
jeFloat LightRadius,LightRadiusSquared;
REPORT(int SphereLight_QuadsLit = 0);

	TIMER_P(LightDynamic_Sphere);

	LightPos = pLight->Vector;
	LightColor = pLight->Color;
	LightRadiusSquared = pLight->MaxColor;
	LightRadius = jeFloat_Sqrt(LightRadiusSquared);

	pStack = QT->TheStack;
	Stack_Push(pStack,QT->Root);

	while( (pQuad = (Quad *)Stack_Pop(pStack)) != NULL )
	{
		if ( ! pQuad->Vis )
			continue;

		if ( ! PointInRectXY(&(pQuad->BBox),&LightPos,LightRadius) )
			continue;

		if ( pQuad->Active == ACTIVE_NODE )
		{
			Stack_Push(pStack,pQuad->pChildren[0]);
			Stack_Push(pStack,pQuad->pChildren[1]);
			Stack_Push(pStack,pQuad->pChildren[2]);
			Stack_Push(pStack,pQuad->pChildren[3]);
		}
		else
		{
		int p;

			assert( pQuad->Active == ACTIVE_LEAF );

			for(p=0;p<4;p++)
			{
			QuadPoint * QP;
				QP = pQuad->Points[p];
				if ( QP->DLightFlags & Mask )
				{
				jeRGBA *pColor,TrueColor;
				float d;
					QP->DLightFlags ^= Mask;
					
					d = jeVec3d_DistanceBetweenSquared(&(QP->World),&LightPos);
					d = 1.0f / d;

					pColor = &(QP->Color);

					TrueColor.r = d * LightColor.r;
					TrueColor.g = d * LightColor.g;
					TrueColor.b = d * LightColor.b;

					TrueColor.r = JE_CLAMP(TrueColor.r,-TERRAIN_MAX_DLIGHT,TERRAIN_MAX_DLIGHT);
					TrueColor.g = JE_CLAMP(TrueColor.g,-TERRAIN_MAX_DLIGHT,TERRAIN_MAX_DLIGHT);
					TrueColor.b = JE_CLAMP(TrueColor.b,-TERRAIN_MAX_DLIGHT,TERRAIN_MAX_DLIGHT);

					pColor->r += TrueColor.r;
					pColor->g += TrueColor.g;
					pColor->b += TrueColor.b;
				}
			}
		}
	}
	
	TIMER_Q(LightDynamic_Sphere);
}

void QuadTree_ApplyDynamicLights_Spheres(QuadTree * QT,jeTerrain_Light * LightsArray,int LightArrayLen)
{
jeTerrain_Light * Lights;
int n;
	for(Lights = LightsArray,n=0;n<LightArrayLen;n++, Lights++)
	{
		if ( Lights->Type == TERRAIN_LIGHT_SPHERE )
		{
			assert(n < TERRAIN_MAX_NUM_LIGHTS); //CyRiuS
			QuadTree_ApplyDynamicLight_Sphere_Subdividing(QT,Lights,(1UL<<n));
		}
	}

	// if the 2nd light made new points, then they haven't been lit by the 1st
	//	so relight
	// <> this is not very elegant!

	for(Lights = LightsArray,n=0;n<(LightArrayLen-1);n++, Lights++)
	{
		if ( Lights->Type == TERRAIN_LIGHT_SPHERE )
		{
			assert(n < TERRAIN_MAX_NUM_LIGHTS); //CyRiuS
			QuadTree_ApplyDynamicLight_Sphere(QT,Lights,(1UL<<n));
		}
	}
}

jeBoolean QuadTree_LightTesselatedPoints(QuadTree *QT,jeTerrain_Light * Lights,int NumLights)
{
	QuadTree_ApplyDynamicLights_Flat(QT,Lights,NumLights);
	QuadTree_ApplyDynamicLights_Spheres(QT,Lights,NumLights);
	QuadTree_ApplyDynamicLights_Clamp(QT);

	REPORT_ADD(NumDynamicQuads);

return JE_TRUE;
}

#include "jeWorld.h"

void QuadTree_LightTexture(QuadTree *QT,jeLight ** Lights,int NumLights,jeBoolean SelfShadow,jeBoolean WorldShadow)
{
jeExtBox QT_BBox,TexBBox;
float TexBBox_StepX,TexBBox_StepY;
float Bitmap_StepX,Bitmap_StepY;
int tx,ty,bx,by,TexDim;
jeBitmap *Tex,*Lock;
jeBitmap_Info TexInfo;
jeBoolean suc;
jeVec3d Normal;
const jeTerrain * Terrain;

	if ( WorldShadow )
		SelfShadow = JE_TRUE;

	Terrain = QT->Terrain;
	QT_BBox = QT->Root->BBox;

	TexDim = Terrain->TexDim;

	TexBBox_StepX = (QT_BBox.Max.X - QT_BBox.Min.X) / TexDim;
	TexBBox_StepY = (QT_BBox.Max.Y - QT_BBox.Min.Y) / TexDim;

	Log_Printf("Quad : Light Texture begins!\n");
	pushTSC();

	for(ty=0;ty<TexDim;ty++)
	{
		TexBBox.Min.Y = QT_BBox.Min.Y + ty * TexBBox_StepY;
		TexBBox.Max.Y = TexBBox.Min.Y + TexBBox_StepY;
		for(tx=0;tx<TexDim;tx++)
		{
		uint8 *TexBits,*ptr;
		jePixelFormat_ColorGetter pfGetColor;
		jePixelFormat_ColorPutter pfPutColor;
		const jePixelFormat_Operations * pfOps;

			Log_Printf("Quad : Light Texture %d,%d\n",tx,ty);

			TexBBox.Min.X = QT_BBox.Min.X + tx * TexBBox_StepX;
			TexBBox.Max.X = TexBBox.Min.X + TexBBox_StepX;
	
			Tex = Terrain->Textures[ ty * TexDim + tx ];
			assert(Tex);

			jeBitmap_SetMipCount(Tex,1);

			suc = jeBitmap_LockForWrite(Tex,&Lock,0,0);
			assert(suc);

			suc = jeBitmap_GetInfo(Lock,&TexInfo,NULL);
			assert(suc);

			pfOps = jePixelFormat_GetOperations(TexInfo.Format);
			assert(pfOps);
			pfGetColor = pfOps->GetColor;
			pfPutColor = pfOps->PutColor;

			if ( ! pfGetColor || ! pfPutColor )
			{
				jeBitmap_UnLock(Lock);

				suc = jeBitmap_SetFormat(Tex,JE_PIXELFORMAT_32BIT_ARGB,JE_FALSE,0,NULL);
				assert(suc);

				suc = jeBitmap_LockForWriteFormat(Tex,&Lock,0,0,JE_PIXELFORMAT_32BIT_ARGB);
				assert(suc);
				
				suc = jeBitmap_GetInfo(Lock,&TexInfo,NULL);
				assert(suc);
				
				pfOps = jePixelFormat_GetOperations(TexInfo.Format);
				assert(pfOps);
				pfGetColor = pfOps->GetColor;
				pfPutColor = pfOps->PutColor;
			}

			Bitmap_StepX = (TexBBox.Max.X - TexBBox.Min.X) / TexInfo.Width;
			Bitmap_StepY = (TexBBox.Max.Y - TexBBox.Min.Y) / TexInfo.Height;

			TexBits = (uint8 *)jeBitmap_GetBits(Lock);
			assert(TexBits);
			ptr = TexBits;

			/****

			@@ two todos :

			1. for all sphere lights, check for distance > radius before doing collision
			2. for all sphere lights, check distance to entire texture vs. radius
					and possibly disable the light for the whole texture

			*****/

			for(by=0;by<(TexInfo.Height);by++)
			{
			float X,Y;
				X = TexBBox.Min.X + (Bitmap_StepX * 0.5f);
				Y = TexBBox.Min.Y + Bitmap_StepY * (by + 0.5f);
				for(bx=0;bx<(TexInfo.Width);bx++,X += Bitmap_StepX)
				{
				uint8 * saveptr;
				int R,G,B,A,l;
				jeRGBA LightColor;
				jeVec3d World;

					jeTerrain_GetNormalAtXY_Rough(Terrain,X,Y,&Normal);

					assert( X >= TexBBox.Min.X && X <= TexBBox.Max.X );
					assert( Y >= TexBBox.Min.Y && Y <= TexBBox.Max.Y );

					saveptr = ptr;
					pfGetColor(&ptr,&R,&G,&B,&A);

					World.X = X;
					World.Y = Y;
					World.Z = jeTerrain_GetHeightAtXY(Terrain,X,Y);

					LightColor.r = LightColor.g = LightColor.b = 0.0f;

					if ( WorldShadow )
					{
					uint32 Flags;
					jeFloat Radius,Brightness;
					jeVec3d LightVec,Color;
					jeVec3d RaisedWorld;
					Quad * MyQ;
					jeCollisionInfo CollisionInfo;
					jeVec3d RaisedWorldWorld,LightVecWorld;

						// try to make sure we don't hit ourselves accidentally:
						MyQ = QuadTree_GetQuadAtXY(QT,World.X,World.Y);
						RaisedWorld = World;
						RaisedWorld.Z = MyQ->BBox.Max.Z + QT->Terrain->CubeSize.Z + JE_EPSILON;

						jeXForm3d_Transform(&(QT->Terrain->XFTerrainToWorld),&RaisedWorld,&RaisedWorldWorld);

						for(l=0;l<NumLights;l++)
						{
							jeLight_GetAttributes(Lights[l],&LightVec,&Color,&Radius,&Brightness,&Flags);
						
							if ( Flags & JE_LIGHT_FLAG_SUN )
							{
								jeVec3d_AddScaled(&RaisedWorld,&LightVec,4000.0f,&LightVec);
							}
							else if ( jeVec3d_DistanceBetweenSquared(&RaisedWorld,&LightVec) >= (Radius * Radius) )
							{
								continue;
							}
							jeXForm3d_Transform(&(QT->Terrain->XFTerrainToWorld),&LightVec,&LightVecWorld);

							if ( ! jeWorld_Collision(QT->Terrain->World,NULL,&RaisedWorldWorld,&LightVecWorld,&CollisionInfo) )
							{
							jeRGBA CurColor;
								jeLight_CalculateLighting(Lights[l],&World,&Normal,&CurColor);
								LightColor.r += CurColor.r;
								LightColor.g += CurColor.g;
								LightColor.b += CurColor.b;
							}
						}
					}
					else if ( SelfShadow )
					{
					jeBoolean Blocked;
					uint32 Flags;
					jeFloat Radius,Brightness;
					jeVec3d LightVec,Color;
					jeVec3d RaisedWorld;
					Quad * MyQ;

						// try to make sure we don't hit ourselves accidentally:
						MyQ = QuadTree_GetQuadAtXY(QT,World.X,World.Y);
						RaisedWorld = World;
						RaisedWorld.Z = MyQ->BBox.Max.Z + QT->Terrain->CubeSize.Z + JE_EPSILON;

						for(l=0;l<NumLights;l++)
						{
							jeLight_GetAttributes(Lights[l],&LightVec,&Color,&Radius,&Brightness,&Flags);
						
							if ( Flags & JE_LIGHT_FLAG_SUN )
							{
								Blocked = QuadTree_IntersectRay(QT,&RaisedWorld,&LightVec);
							}
							else
							{
								if ( jeVec3d_DistanceBetweenSquared(&RaisedWorld,&LightVec) >= (Radius * Radius) )
								{
									continue;
								}
								else
								{
								jeVec3d Hit;
									Blocked = QuadTree_IntersectThickRay(QT,&RaisedWorld,&LightVec,0.0f,&Hit);
								}
							}
							
							if ( ! Blocked )
							{
							jeRGBA CurColor;
								jeLight_CalculateLighting(Lights[l],&World,&Normal,&CurColor);
								LightColor.r += CurColor.r;
								LightColor.g += CurColor.g;
								LightColor.b += CurColor.b;
							}
						}
					}
					else // no shadows
					{
						for(l=0;l<NumLights;l++)
						{
						jeRGBA CurColor;
							jeLight_CalculateLighting(Lights[l],&World,&Normal,&CurColor);
							LightColor.r += CurColor.r;
							LightColor.g += CurColor.g;
							LightColor.b += CurColor.b;
						}
					}

					#define ONE_OVER_256	(0.00390625f)

					#if 1 //{ @@ else does overbrighting
					LightColor.r = JE_CLAMP(LightColor.r,0,255);
					LightColor.g = JE_CLAMP(LightColor.g,0,255);
					LightColor.b = JE_CLAMP(LightColor.b,0,255);
					#endif //}

					R = (int)(R * LightColor.r * ONE_OVER_256);
					G = (int)(G * LightColor.g * ONE_OVER_256);
					B = (int)(B * LightColor.b * ONE_OVER_256);

					R = JE_CLAMP(R,0,255);
					G = JE_CLAMP(G,0,255);
					B = JE_CLAMP(B,0,255);

					pfPutColor(&saveptr,R,G,B,A);
				}
			}

			jeBitmap_UnLock(Lock);
		}
	}
	
	showPopTSC("Quad : LightTexture");
}

void QuadTree_LightAllPoints(QuadTree *QT,jeLight ** Lights,int NumLights)
{
Stack *pStack;
Quad * pQuad;

	pStack = QT->TheStack;
	Stack_Push(pStack,QT->Root);

	while( (pQuad = (Quad *)Stack_Pop(pStack)) != NULL )
	{
		if ( Quad_HasChildren(pQuad) )
		{
			Stack_Push(pStack,pQuad->pChildren[0]);
			Stack_Push(pStack,pQuad->pChildren[1]);
			Stack_Push(pStack,pQuad->pChildren[2]);
			Stack_Push(pStack,pQuad->pChildren[3]);
		}
		else
		{
		int p;
		QuadPoint *v;

			for(p=0;p<4;p++)
			{
			int l;
				v = pQuad->Points[p];
				v->Color.r = v->Color.g = v->Color.b = 0.0f;
				for(l=0;l<NumLights;l++)
				{
				jeRGBA CurColor;
					jeLight_CalculateLighting(Lights[l],&(v->World),&(v->Normal),&CurColor);
					v->Color.r += CurColor.r;
					v->Color.g += CurColor.g;
					v->Color.b += CurColor.b;
				}
				v->Color.r = JE_CLAMP(v->Color.r,0.0f,255.0f);
				v->Color.g = JE_CLAMP(v->Color.g,0.0f,255.0f);
				v->Color.b = JE_CLAMP(v->Color.b,0.0f,255.0f);
				v->Color.a = 255.0f;
				v++;
			}
		}
	}
}

/*}{************ IntersectRay **********/

jeBoolean jeExtBox_HitsSegment(const jeExtBox * pBox,jeVec3d * pA,jeVec3d *pB)
{

	// look for quick inclusion : is one of the points in the box ?

	if ( (pA->X > pBox->Min.X) && (pA->X < pBox->Max.X) &&
		 (pA->Y > pBox->Min.Y) && (pA->Y < pBox->Max.Y) &&
		 (pA->Z > pBox->Min.Z) && (pA->Z < pBox->Max.Z) )
		return JE_TRUE;

	if ( (pB->X > pBox->Min.X) && (pB->X < pBox->Max.X) &&
		 (pB->Y > pBox->Min.Y) && (pB->Y < pBox->Max.Y) &&
		 (pB->Z > pBox->Min.Z) && (pB->Z < pBox->Max.Z) )
		return JE_TRUE;

	// both points are out, straddling some edges; must do intersects

	{
	jeVec3d VA,VB;
	jeFloat Mult;

		// pA & pB get modified, so put 'em in temp stores
		VA = *pA; pA = &VA;
		VB = *pB; pB = &VB;
	
		// first X

		// sort so A is 'lower'
		if ( pA->X > pB->X ) 
			swapints(pA,pB);
		
		assert( pA->X <= pB->X );

		// it's all out one side or the other :
		if ( pB->X <= pBox->Min.X || pA->X >= pBox->Max.X )
			return JE_FALSE;

		if ( pA->X <= pBox->Min.X && pB->X >= pBox->Min.X )
		{
			// the fraction of the distance from A to B
			Mult = (pBox->Min.X - pA->X) / ( pB->X - pA->X );
			//pA->X = pA->X + (pB->X - pA->X) * Mult;
			pA->X = pBox->Min.X;
			pA->Y = pA->Y + (pB->Y - pA->Y) * Mult;
			pA->Z = pA->Z + (pB->Z - pA->Z) * Mult;
		}
		if ( pA->X <= pBox->Max.X && pB->X >= pBox->Max.X )
		{
			// the fraction of the distance from B to A
			Mult = (pBox->Max.X - pB->X) / ( pA->X - pB->X );
			pB->X = pBox->Max.X;
			pB->Y = pB->Y + (pA->Y - pB->Y) * Mult;
			pB->Z = pB->Z + (pA->Z - pB->Z) * Mult;
		}
		
		assert( pA->X >= pBox->Min.X );
		assert( pB->X <= pBox->Max.X );
		assert( pA->X <= pB->X );

		// then Y

		if ( pA->Y > pB->Y ) 
		{
			jeVec3d			temp;

			jeVec3d_Copy(pA, &temp);
			jeVec3d_Copy(pB, pA);
			jeVec3d_Copy(&temp, pB);
			//swapints(pA,pB);
		}

		assert( pA->Y <= pB->Y );

		if ( pB->Y <= pBox->Min.Y || pA->Y >= pBox->Max.Y )
			return JE_FALSE;

		if ( pA->Y <= pBox->Min.Y && pB->Y >= pBox->Min.Y )
		{
			Mult = (pBox->Min.Y - pA->Y) / ( pB->Y - pA->Y );
			pA->Y = pBox->Min.Y;
			pA->X = pA->X + (pB->X - pA->X) * Mult;
			pA->Z = pA->Z + (pB->Z - pA->Z) * Mult;
		}
		if ( pA->Y <= pBox->Max.Y && pB->Y >= pBox->Max.Y )
		{
			Mult = (pBox->Max.Y - pB->Y) / ( pA->Y - pB->Y );
			pB->Y = pBox->Max.Y;
			pB->X = pB->X + (pA->X - pB->X) * Mult;
			pB->Z = pB->Z + (pA->Z - pB->Z) * Mult;
		}
		
		assert( pA->Y >= pBox->Min.Y );
		assert( pB->Y <= pBox->Max.Y );
		assert( pA->Y <= pB->Y );

		// then Z

		if ( pA->Z > pB->Z ) swapints(pA,pB);

		assert( pA->Z <= pB->Z );

		if ( pB->Z <= pBox->Min.Z || pA->Z >= pBox->Max.Z )
			return JE_FALSE;

	}

return JE_TRUE;
}

Quad * QuadTree_GetQuadAtXY(const QuadTree *QT,jeFloat X,jeFloat Y)
{
Quad * Q;
	Q = QT->Root;
	while( Quad_HasChildren(Q) )
	{
	QuadPoint * P;

		assert( X >= Q->BBox.Min.X && X <= Q->BBox.Max.X );
		assert( Y >= Q->BBox.Min.Y && Y <= Q->BBox.Max.Y );

		P = Quad_CenterPoint(Q);

		if ( X >= P->World.X ) // E
		{
			if ( Y >= P->World.Y ) // N
			{
				Q = Q->pChildren[QUAD_NE];
			}
			else // S
			{
				Q = Q->pChildren[QUAD_SE];
			}
		}
		else // W
		{
			if ( Y >= P->World.Y ) // N
			{
				Q = Q->pChildren[QUAD_NW];
			}
			else // S
			{
				Q = Q->pChildren[QUAD_SW];
			}
		}
	}
return Q;
}

jeBoolean CollideExtBoxXY2(const jeVec3d *pVec,const jeVec3d *pDirection,const jeExtBox *pBox,
								jeVec3d * pHits)
{
jeFloat Scale;
jeVec3d Hit;
int HitI = 0;

	if ( (pDirection->X > 0.0f && pVec->X >= pBox->Max.X) ||
		 (pDirection->X < 0.0f && pVec->X <= pBox->Min.X) ||
		 (pDirection->Y > 0.0f && pVec->Y >= pBox->Max.Y) ||
		 (pDirection->Y < 0.0f && pVec->Y <= pBox->Min.Y) )
		return JE_FALSE;

	if ( pDirection->X > 0.0f )
	{
		Scale = (pBox->Min.X - pVec->X)/pDirection->X;
		Hit.X = pBox->Min.X;
		Hit.Y = pVec->Y + pDirection->Y * Scale;
		Hit.Z = pVec->Z + pDirection->Z * Scale;

		if ( Hit.Y >= pBox->Min.Y && Hit.Y <= pBox->Max.Y )
			pHits[HitI++] = Hit;

		Scale = (pBox->Max.X - pVec->X)/pDirection->X;
		Hit.X = pBox->Max.X;
		Hit.Y = pVec->Y + pDirection->Y * Scale;
		Hit.Z = pVec->Z + pDirection->Z * Scale;

		if ( Hit.Y >= pBox->Min.Y && Hit.Y <= pBox->Max.Y )
			pHits[HitI++] = Hit;
	}

	if ( pDirection->Y > 0.0f )
	{
		Scale = (pBox->Min.Y - pVec->Y)/pDirection->Y;
		Hit.Y = pBox->Min.Y;
		Hit.X = pVec->X + pDirection->X * Scale;
		Hit.Z = pVec->Z + pDirection->Z * Scale;

		if ( Hit.X >= pBox->Min.X && Hit.X <= pBox->Max.X )
			pHits[HitI++] = Hit;

		Scale = (pBox->Max.Y - pVec->Y)/pDirection->Y;
		Hit.Y = pBox->Max.Y;
		Hit.X = pVec->X + pDirection->X * Scale;
		Hit.Z = pVec->Z + pDirection->Z * Scale;

		if ( Hit.X >= pBox->Min.X && Hit.X <= pBox->Max.X )
			pHits[HitI++] = Hit;
	}

	if ( HitI > 0 )
	{
		if ( HitI == 1 )
		{
			// this can happen when we just nick a corner...
			pHits[1] = pHits[0];
			HitI = 2;
		}
		else if ( HitI > 2 )
		{
			while(HitI > 2)
			{
			jeFloat d1,d2,d3;
				d1 = jeVec3d_DistanceBetweenSquared(pHits+0,pHits+1);
				d2 = jeVec3d_DistanceBetweenSquared(pHits+HitI-1,pHits+HitI-2);
				d3 = jeVec3d_DistanceBetweenSquared(pHits+HitI-1,pHits+0);
				if ( d2 < d1 || d3 < d1 )
				{
					// clones are at end of list
					HitI--;
				}
				else
				{
				int i;
					// clones are at head
					for(i=1;i<(HitI-1);i++)
					{
						pHits[i] = pHits[i+1];
					}
					HitI--;
				}
			}
		}

		return JE_TRUE;
	}

return JE_FALSE;
}

jeBoolean CollideExtBoxXY(jeVec3d *pVec,const jeVec3d *pDirection,const jeExtBox *pBox)
{
jeFloat Scale;

	// left or right edge plane

	if ( pDirection->X > 0.0f )
	{
		if ( pBox->Min.X >= pVec->X )
		{
			Scale = (pBox->Min.X - pVec->X)/pDirection->X;
			//pVec->X = pVec->X + pDirection->X * Scale;
			pVec->X = pBox->Min.X;
			pVec->Y = pVec->Y + pDirection->Y * Scale;
			pVec->Z = pVec->Z + pDirection->Z * Scale;

			if ( pVec->Y >= pBox->Min.Y && pVec->Y <= pBox->Max.Y )
				return JE_TRUE;
		}
	}
	else
	{
		if ( pBox->Max.X <= pVec->X )
		{
			Scale = (pBox->Max.X - pVec->X)/pDirection->X;
			pVec->X = pBox->Max.X;
			pVec->Y = pVec->Y + pDirection->Y * Scale;
			pVec->Z = pVec->Z + pDirection->Z * Scale;

			if ( pVec->Y >= pBox->Min.Y && pVec->Y <= pBox->Max.Y )
				return JE_TRUE;
		}
	}

	if ( pDirection->Y > 0.0f )
	{
		if ( pBox->Min.Y >= pVec->Y )
		{
			Scale = (pBox->Min.Y - pVec->Y)/pDirection->Y;
			//pVec->Y = pVec->Y + pDirection->Y * Scale;
			pVec->Y = pBox->Min.Y;
			pVec->X = pVec->X + pDirection->X * Scale;
			pVec->Z = pVec->Z + pDirection->Z * Scale;

			if ( pVec->X >= pBox->Min.X && pVec->X <= pBox->Max.X )
				return JE_TRUE;
		}
	}
	else
	{
		if ( pBox->Max.Y <= pVec->Y )
		{
			Scale = (pBox->Max.Y - pVec->Y)/pDirection->Y;
			pVec->Y = pBox->Max.Y;
			pVec->X = pVec->X + pDirection->X * Scale;
			pVec->Z = pVec->Z + pDirection->Z * Scale;

			if ( pVec->X >= pBox->Min.X && pVec->X <= pBox->Max.X )
				return JE_TRUE;
		}
	}

return JE_FALSE;
}

jeBoolean QuadTree_RayCollision(const QuadTree * QT,const Quad *pQuad,const jeVec3d * pStart,const jeVec3d *pDirection)
{
jeVec3d Hits[4];
const jeExtBox *pBox;

	// find the X & Y's of the two intersection points
	pBox = &(pQuad->BBox);
	
	#if 0
	if (pStart->X >= pBox->Min.X && pStart->X <= pBox->Max.X &&
		pStart->Y >= pBox->Min.Y && pStart->Y <= pBox->Max.Y )
		return JE_FALSE; // point is on *my* quad !
	#endif

	if ( ! CollideExtBoxXY2(pStart,pDirection,pBox,Hits) )
		return JE_FALSE; // missed it completely

	if ( Hits[0].Z <= jeTerrain_GetHeightAtXY(QT->Terrain,Hits[0].X,Hits[0].Y) )
		return JE_TRUE;
	if ( Hits[1].Z <= jeTerrain_GetHeightAtXY(QT->Terrain,Hits[1].X,Hits[1].Y) )
		return JE_TRUE;

return JE_FALSE;
}

jeBoolean Quad_RayCollision(const Quad *pQuad,const jeVec3d * pStart,const jeVec3d *pDirection)
{
jeFloat DNormal,Dot;
jeVec3d P;

	// <> this is kinda foogly cuz a quad is not really planar; it is four points which may not be coplanar!

	// find the intersection with the plane of the quad :

#define PLANE_TOLERANCE	(0.001f)

	// fraction of the ray that points towards the plane
	// dot should be negative
	Dot = jeVec3d_DotProduct(&(pQuad->Normal),pDirection);

	// distance from start perp to the plane :
	// distance from start perp to the plane :
	jeVec3d_Average(&(pQuad->BBox.Min),&(pQuad->BBox.Max),&P);
	jeVec3d_Subtract(&P,pStart,&P);
	DNormal = jeVec3d_DotProduct(&(pQuad->Normal),&P);

	if ( JE_ABS(Dot) < PLANE_TOLERANCE )
	{
		if ( Dot <= 0.0f )
			Dot = - PLANE_TOLERANCE;
		else
			Dot = PLANE_TOLERANCE;
	}

	DNormal /= Dot;	// neg / neg = pos

	if ( DNormal < 0.0f ) // collision point is behind the start point
		return JE_FALSE;

	jeVec3d_AddScaled(pStart,pDirection,DNormal,&P);


	if (P.X <= (pQuad->BBox.Min.X - PLANE_TOLERANCE) ||
		P.X >= (pQuad->BBox.Max.X + PLANE_TOLERANCE) ||
		P.Y <= (pQuad->BBox.Min.Y - PLANE_TOLERANCE) ||
		P.Y >= (pQuad->BBox.Max.Y + PLANE_TOLERANCE) ) return JE_FALSE;

return JE_TRUE;
}

jeBoolean Quad_RayCollision2(const Quad *pQuad,const jeVec3d * pStart,const jeVec3d *pDirection,jeFloat RayLength,jeVec3d *pHit)
{
jeFloat DNormal,Dot;
jeVec3d P;

	// find the intersection with the plane of the quad :

	// fraction of the ray that points towards the plane
	// dot should be negative
	Dot = jeVec3d_DotProduct(&(pQuad->Normal),pDirection);

	// distance from start perp to the plane :
	jeVec3d_Average(&(pQuad->BBox.Min),&(pQuad->BBox.Max),&P);
	jeVec3d_Subtract(&P,pStart,&P);
	DNormal = jeVec3d_DotProduct(&(pQuad->Normal),&P);

	if ( JE_ABS(Dot) < PLANE_TOLERANCE )
	{
		if ( Dot <= 0.0f )
			Dot = - PLANE_TOLERANCE;
		else
			Dot = PLANE_TOLERANCE;
	}

	DNormal /= Dot;	// neg / neg = pos

	if ( DNormal < 0.0f ) // collision point is behind the start point
		return JE_FALSE;

	jeVec3d_AddScaled(pStart,pDirection,DNormal,&P);

	if (P.X <= (pQuad->BBox.Min.X - PLANE_TOLERANCE) ||
		P.X >= (pQuad->BBox.Max.X + PLANE_TOLERANCE) ||
		P.Y <= (pQuad->BBox.Min.Y - PLANE_TOLERANCE) ||
		P.Y >= (pQuad->BBox.Max.Y + PLANE_TOLERANCE) ) return JE_FALSE;

	*pHit = P;

return JE_TRUE;
}

jeBoolean QuadTree_ThickRayCollision(const QuadTree * QT,const Quad *pQuad,const jeVec3d * pStart,const jeVec3d *pDirection,
										jeFloat Radius,jeFloat RayLength,jeVec3d *pHit)
{
jeVec3d Hits[4];
const jeExtBox *pBox;

	// find the X & Y's of the two intersection points
	pBox = &(pQuad->BBox);
	
	if ( ! CollideExtBoxXY2(pStart,pDirection,pBox,Hits) )
		return JE_FALSE; // missed it completely

	if ( Hits[0].Z <= (jeTerrain_GetHeightAtXY(QT->Terrain,Hits[0].X,Hits[0].Y) + Radius) )
	{
		*pHit = Hits[0];
		if ( jeVec3d_DistanceBetween(pStart,pHit) < RayLength )
			return JE_TRUE;
	}
	if ( Hits[1].Z <= (jeTerrain_GetHeightAtXY(QT->Terrain,Hits[1].X,Hits[1].Y) + Radius) )
	{
		*pHit = Hits[1];
		if ( jeVec3d_DistanceBetween(pStart,pHit) < RayLength )
			return JE_TRUE;
	}

return JE_FALSE;
}

jeBoolean Quad_ThickRayCollision(const Quad *pQuad,const jeVec3d * pStart,const jeVec3d *pDirection,
										jeFloat Radius,jeFloat RayLength,jeVec3d *pHit)
{
jeFloat DNormal,Dot;
jeVec3d P;

	// find the intersection with the plane of the quad :

	// fraction of the ray that points towards the plane
	// dot should be negative
	Dot = jeVec3d_DotProduct(&(pQuad->Normal),pDirection);

	// distance from start perp to the plane :
	jeVec3d_Average(&(pQuad->BBox.Min),&(pQuad->BBox.Max),&P);
	jeVec3d_Subtract(&P,pStart,&P);
	DNormal = jeVec3d_DotProduct(&(pQuad->Normal),&P);

	if ( JE_ABS(Dot) < PLANE_TOLERANCE )
	{
		if ( Dot <= 0.0f )
			Dot = - PLANE_TOLERANCE;
		else
			Dot = PLANE_TOLERANCE;
	}

	DNormal /= Dot;	// neg / neg = pos

	// DNormal is now the length along Direction where we hit :

	if ( DNormal < 0.0f || DNormal > (RayLength + Radius) )
		return JE_FALSE; // off the end of the segment

	jeVec3d_AddScaled(pStart,pDirection,DNormal,&P);

	// @@ could pretty easily make this a proper extbox instead of a sphere

	Radius += PLANE_TOLERANCE;

	if ((P.X + Radius) <= pQuad->BBox.Min.X ||
		(P.X - Radius) >= pQuad->BBox.Max.X ||
		(P.Y + Radius) <= pQuad->BBox.Min.Y ||
		(P.Y - Radius) >= pQuad->BBox.Max.Y ) return JE_FALSE;

	*pHit = P;

return JE_TRUE;
}

/** QuadTree_IntersectRay ; the parent intersection functions **************/

jeBoolean QuadTree_IntersectRay(QuadTree *QT,jeVec3d *pStart,jeVec3d *pDirection)
{
jeVec3d StartP,EndP,Direction;// B = A + AB
//jeExtBox EndPox;
Stack * S = NULL;
Quad * Q = NULL;

/*****

	there's a funny thing here here :

	for these purposes, we don't actually want the quad's bbox ;
	what we want is to treat the Min.Z as - infinity

	When the starting point of the ray is not on the surface, the
	methods here are fine, but for a ray on the surface which can
	go straight through solid, it might never hit a quads bbox, and
	just be embedded in solid the whole time.

	Actually, the normal-check should solve this.

*****/

	StartP = *pStart;
	Direction = *pDirection;
	jeVec3d_AddScaled(pStart,&Direction,
		(QT->Root->BBox.Max.X - QT->Root->BBox.Min.X + QT->Root->BBox.Max.Y - QT->Root->BBox.Min.Y),
		&EndP);

	assert( jeVec3d_IsNormalized(&Direction) );
	
	#if 0
	{
	jeVec3d Down;
		Down.Z = - 1.0f;
		Down.X = Down.Y = 0.0f;
		assert( Quad_RayCollision(MyQ,&StartP,&Down) );
	}
	#endif

	S = QT->TheStack;
	Stack_Push(S,QT->Root);

	while( (Q = (Quad *)Stack_Pop(S)) )
	{
	jeExtBox *pBox;
	jeVec3d AtoQ;
	jeFloat Dot,DistSqr,BoxRadiusSqr;

		pBox = &(Q->BBox);

		// radius of bounding sphere :
		// we-over count the quads that could hit me

		BoxRadiusSqr = jeVec3d_DistanceBetweenSquared(&(pBox->Max),&(pBox->Min)) * 0.25f + JE_EPSILON;

		AtoQ.X = (pBox->Min.X + pBox->Max.X)*0.5f - StartP.X;
		AtoQ.Y = (pBox->Min.Y + pBox->Max.Y)*0.5f - StartP.Y;
		AtoQ.Z = (pBox->Min.Z + pBox->Max.Z)*0.5f - StartP.Z;

		Dot = jeVec3d_DotProduct(&AtoQ,&Direction);

		DistSqr = jeVec3d_LengthSquared(&AtoQ) - Dot * Dot;

		if ( DistSqr > BoxRadiusSqr )
			continue;

		if ( Quad_HasChildren(Q) )
		{
			Stack_Push(S,Q->pChildren[0]);
			Stack_Push(S,Q->pChildren[1]);
			Stack_Push(S,Q->pChildren[2]);
			Stack_Push(S,Q->pChildren[3]);
		}
		else
		{
			// we hit a leaf

			// do a more-accurate check now

//			if ( Quad_RayCollision(Q,&StartP,&Direction) )
			if ( QuadTree_RayCollision(QT,Q,&StartP,&Direction) )
			{
				Stack_Reset(S);
				return JE_TRUE;
			}
		}
	
	}

return JE_FALSE;
}

jeBoolean	QuadTree_IntersectThickRay(const QuadTree * QT,
				const jeVec3d * pFrom,const jeVec3d * pTo,
				jeFloat Radius,jeVec3d * pImpact)
{
jeVec3d StartP,EndP,Direction;
jeFloat RayLength;
Stack * S;
Quad * Q;

	// @@ could pretty easily make a proper extbox colliding version

	StartP = *pFrom;
	EndP   = *pTo;
	jeVec3d_Subtract( &EndP, &StartP, &Direction );
	RayLength = jeVec3d_Normalize( &Direction );


	S = QT->TheStack;
	Stack_Push(S,QT->Root);

	while( (Q = (Quad *)Stack_Pop(S)) )
	{
	jeExtBox *pBox;
	jeVec3d AtoQ,Qcenter;
	jeFloat Dot,DistSqr,BoxRadius,BoxRadiusSqr;

		pBox = &(Q->BBox);

		// radius of bounding sphere :
		// we-over count the quads that could hit me

		BoxRadius = jeVec3d_DistanceBetween(&(pBox->Max),&(pBox->Min)) * 0.5f + JE_EPSILON + Radius;
		BoxRadiusSqr = BoxRadius * BoxRadius;


		Qcenter.X = (pBox->Min.X + pBox->Max.X)*0.5f;
		Qcenter.Y = (pBox->Min.Y + pBox->Max.Y)*0.5f;
		Qcenter.Z = (pBox->Min.Z + pBox->Max.Z)*0.5f;

		AtoQ.X = Qcenter.X - StartP.X;
		AtoQ.Y = Qcenter.Y - StartP.Y;
		AtoQ.Z = Qcenter.Z - StartP.Z;

		Dot = jeVec3d_DotProduct(&AtoQ,&Direction);

		DistSqr = jeVec3d_LengthSquared(&AtoQ) - Dot * Dot;

		if ( DistSqr > BoxRadiusSqr )
			continue;

		// we're within the bounding sphere of the quad; get more accurate

		if ( Quad_HasChildren(Q) )
		{
		EQuadPosition Qstart;
			// push in order of closest to farthest
			if ( StartP.X >= Qcenter.X )
				if ( StartP.Y >= Qcenter.Y )
					Qstart = QUAD_NE;
				else
					Qstart = QUAD_SE;
			else
				if ( StartP.Y >= Qcenter.Y )
					Qstart = QUAD_NW;
				else
					Qstart = QUAD_SW;

			Stack_Push(S,Q->pChildren[Qstart]);
			Stack_Push(S,Q->pChildren[(Qstart+1)&3]);
			Stack_Push(S,Q->pChildren[(Qstart+3)&3]);
			Stack_Push(S,Q->pChildren[(Qstart+2)&3]);
		}
		else
		{
			// we hit a leaf
			// do a more-accurate check now

			if ( Quad_ThickRayCollision(Q,&StartP,&Direction,Radius,RayLength,pImpact) )
//			if ( QuadTree_ThickRayCollision(QT,Q,&StartP,&Direction,Radius,RayLength,pImpact) )
			{
				Stack_Reset(S);
				return JE_TRUE;
			}
		}
	}

return JE_FALSE;
}

void jeXForm3d_SetInverseRay(jeXForm3d * pXF,const jeVec3d * pBase,const jeVec3d *prayZ)
{
jeVec3d rayX,rayY;

	rayX.X = prayZ->Y;
	rayX.Y = prayZ->Z;
	rayX.Z = prayZ->X;

	jeVec3d_CrossProduct(&rayX,prayZ,&rayY);
	jeVec3d_Normalize(&rayY);
	jeVec3d_CrossProduct(&rayY,prayZ,&rayX);
	jeVec3d_Normalize(&rayX);
	
	jeXForm3d_SetFromLeftUpIn(pXF,&rayX,&rayY,prayZ);

	pXF->Translation = *pBase;

	jeXForm3d_GetTranspose(pXF,pXF);
}

jeBoolean Quad_ThickRayCollisionProjecting(const Quad * Q,const jeXForm3d * pXF,
	jeFloat Radius,jeFloat RayLength,jeVec3d * pImpact)
{
jeVec3d Points[4];
jeExtBox EB;
	jeXForm3d_Transform(pXF,Quad_WorldPoint(Q,0),Points+0);
	jeXForm3d_Transform(pXF,Quad_WorldPoint(Q,1),Points+1);
	jeXForm3d_Transform(pXF,Quad_WorldPoint(Q,2),Points+2);
	jeXForm3d_Transform(pXF,Quad_WorldPoint(Q,3),Points+3);

	// <> could do a true point in poly; just be lazy for now :

	jeExtBox_SetToPoint(&EB,Points+0);
	jeExtBox_ExtendToEnclose(&EB,Points+1);
	jeExtBox_ExtendToEnclose(&EB,Points+2);
	jeExtBox_ExtendToEnclose(&EB,Points+3);

	if (EB.Min.X <= Radius && EB.Max.X >= -Radius &&
		EB.Min.Y <= Radius && EB.Max.Y >= -Radius &&
		EB.Min.Z <= (RayLength + Radius) )
	{
		jeVec3d_Average( Quad_WorldPoint(Q,0) ,Quad_WorldPoint(Q,2), pImpact );
		return JE_TRUE;
	}

return JE_FALSE;
}

static void jeExtBox_Transform(const jeExtBox *pIn,const jeXForm3d * pXF,jeExtBox *pOut)
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

jeBoolean Quad_ThickRayCollisionProjectingBBox(const Quad * Q,const jeXForm3d * pXF,
	jeFloat Radius,jeFloat RayLength)
{
jeExtBox EB;

	jeExtBox_Transform(&(Q->BBox),pXF,&EB);

	if (EB.Min.X <= Radius && EB.Max.X >= -Radius &&
		EB.Min.Y <= Radius && EB.Max.Y >= -Radius &&
		EB.Min.Z <= (RayLength + Radius) )
	{
		return JE_TRUE;
	}

return JE_FALSE;
}

/*****


here's another way to do collisions :

	make an XForm that transforms you into the space of the ray;
		(eg. pStart is at {0,0,0} and pDirection is {0,0,RayLength} in "ray space")
	now just xform the 4 corners of the quad into ray space.  If it straddles 0,0 in ray space, it's a hit!

<> has bugs

*******/

jeBoolean	QuadTree_IntersectThickRayProjecting(const QuadTree * QT,
				const jeVec3d * pFrom,const jeVec3d * pTo,
				jeFloat Radius,jeVec3d * pImpact)
{
jeVec3d StartP,EndP,Direction;
jeFloat RayLength;
Stack * S;
Quad * Q;
jeXForm3d XF;

	StartP = *pFrom;
	EndP   = *pTo;
	jeVec3d_Subtract( &EndP, &StartP, &Direction );
	RayLength = jeVec3d_Normalize( &Direction );

	jeXForm3d_SetInverseRay(&XF,&StartP,&Direction);

	S = QT->TheStack;
	Stack_Push(S,QT->Root);

	while( (Q = (Quad *)Stack_Pop(S)) )
	{
	
		if ( ! Quad_ThickRayCollisionProjectingBBox(Q,&XF,Radius,RayLength) )
			continue;

		// we're within the bounding sphere of the quad; get more accurate

		if ( Quad_HasChildren(Q) )
		{
			Stack_Push(S,Q->pChildren[0]);
			Stack_Push(S,Q->pChildren[1]);
			Stack_Push(S,Q->pChildren[2]);
			Stack_Push(S,Q->pChildren[3]);
		}
		else
		{
			if ( ! Quad_ThickRayCollisionProjecting(Q,&XF,Radius,RayLength,pImpact) )
				continue;

			Stack_Reset(S);
			return JE_TRUE;
		}
	}

return JE_FALSE;
}

/*}************ EOF **********/
