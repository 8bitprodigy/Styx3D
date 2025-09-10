/****************************************************************************************/
/*  JEUSERPOLY.C                                                                        */
/*                                                                                      */
/*  Author:  John Pollard                                                               */
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
#include <assert.h>
#include <string.h>

// Public Dependents
#include "jeUserPoly.h"

// Private dependents
#include "Ram.h"

#include "Dcommon.h"
#include "Camera._h"

#include "jeMaterial.h"

#pragma message ("All the mallocs should eventually use jeMemPool...")

//========================================================================================
//========================================================================================
#define ZeroMem(a) memset(a, 0, sizeof(*a))
#define ZeroMemArray(a, s) memset(a, 0, sizeof(*a)*s)

#define JE_USERPOLY_MAX_REFCOUNTS			(0xffffffff>>1)

// We should eventually make structures for each kind, and use jeUserPoly as an interface to it
typedef struct jeUserPoly
{
	// Info about the poly
	jeUserPoly_Type		Type;
	uint32				Flags;
	const jeMaterialSpec* Material;

	// The verts
	jeLVertex			Verts[4];
	jeFloat				Scale;				// Scale of sprite

	int32				RefCount;
} jeUserPoly;

//========================================================================================
//	Local statics
//========================================================================================
static jeBoolean RenderQuad(const jeUserPoly *Poly, const jeEngine *Engine, const jeCamera *Camera, const jeFrustum *Frustum);
static jeBoolean RenderTri(const jeUserPoly *Poly, const jeEngine *Engine, const jeCamera *Camera, const jeFrustum *Frustum);
static jeBoolean RenderSprite(const jeUserPoly *Poly, const jeEngine *Engine, const jeCamera *Camera, const jeFrustum *Frustum);
static jeBoolean RenderLine(const jeUserPoly *Poly, const jeEngine *Engine, const jeCamera *Camera, const jeFrustum *Frustum);
//========================================================================================
//	jeUserPoly_CreateTri
//	Create a user poly of type tri
//  Vizard/Void/Jeff Muizelaar
//========================================================================================
JETAPI jeUserPoly * JETCC jeUserPoly_CreateTri(	const jeLVertex		*v1, 
												const jeLVertex		*v2, 
												const jeLVertex		*v3, 
												const jeMaterialSpec	*Material,
												uint32				Flags)
{
	jeUserPoly		*Tri;

	assert(v1);
	assert(v2);
	assert(v3);

	Tri = JE_RAM_ALLOCATE_STRUCT(jeUserPoly);
	//We are allocating enough space for 4 vertexes instead of the 3 we need for a tri
	//Don't know how this would be remedied
	if (!Tri)
		return NULL;

	// Clear the memory
	ZeroMem(Tri);

	Tri->RefCount = 1;
	
	// Set the type
	Tri->Type = Type_Tri;

	Tri->Material = Material;

	Tri->Flags = Flags;

	// Copy the verts
	Tri->Verts[0] = *v1;
	Tri->Verts[1] = *v2;
	Tri->Verts[2] = *v3;

	return Tri;
}

//========================================================================================
//	jeUserPoly_CreateQuad
//	Create a user poly of type quad
//========================================================================================
JETAPI jeUserPoly * JETCC jeUserPoly_CreateQuad(	const jeLVertex		*v1, 
												const jeLVertex		*v2, 
												const jeLVertex		*v3, 
												const jeLVertex		*v4, 
												const jeMaterialSpec *Material,
												uint32				Flags)
{
	jeUserPoly		*Quad;

	assert(v1);
	assert(v2);
	assert(v3);
	assert(v4);

	Quad = JE_RAM_ALLOCATE_STRUCT(jeUserPoly);

	if (!Quad)
		return NULL;

	// Clear the memory
	ZeroMem(Quad);

	Quad->RefCount = 1;
	
	// Set the type
	Quad->Type = Type_Quad;

	Quad->Material = Material;

	Quad->Flags = Flags;

	// Copy the verts
	Quad->Verts[0] = *v1;
	Quad->Verts[1] = *v2;
	Quad->Verts[2] = *v3;
	Quad->Verts[3] = *v4;

	return Quad;
}

//========================================================================================
//	jeUserPoly_CreateSprite
//	Create a user poly of type Sprite
//========================================================================================
JETAPI jeUserPoly * JETCC jeUserPoly_CreateSprite(	const jeLVertex		*v1, 
												const jeMaterialSpec *Material,
												jeFloat				Scale,
												uint32				Flags)
{
	jeUserPoly		*Sprite;

	assert(v1);
	assert(Material);		// For now, you need a bitmap

	Sprite = JE_RAM_ALLOCATE_STRUCT(jeUserPoly);

	if (!Sprite)
		return NULL;

	// Clear the memory
	ZeroMem(Sprite);
	
	Sprite->RefCount = 1;

	// Set the type
	Sprite->Type = Type_Sprite;

	Sprite->Material = Material;

	Sprite->Scale = Scale;

	Sprite->Flags = Flags;

	// Copy the vert that will be the center of the sprite
	Sprite->Verts[0] = *v1;

	return Sprite;
}

//========================================================================================
//	jeUserPoly_CreateLine
//========================================================================================
JETAPI jeUserPoly * JETCC jeUserPoly_CreateLine(const jeLVertex *v1, const jeLVertex *v2, jeFloat Scale, uint32 Flags)
{
	jeUserPoly		*Line;

	assert(v1);
	assert(v2);

	Line = JE_RAM_ALLOCATE_STRUCT(jeUserPoly);

	if (!Line)
		return NULL;

	// Clear the memory
	ZeroMem(Line);
	
	Line->RefCount = 1;

	// Set the type
	Line->Type = Type_Line;

	Line->Scale = Scale;

	Line->Flags = Flags;

	Line->Verts[0] = *v1;
	Line->Verts[1] = *v2;

	return Line;
}

//========================================================================================
//	jeUserPoly_IsValid
//========================================================================================
JETAPI jeBoolean JETCC jeUserPoly_IsValid(const jeUserPoly *Poly)
{
	if (!Poly)
		return JE_FALSE;

	if (Poly->RefCount <= 0)		// At least 1 person must be referencing the object
		return JE_FALSE;

	return JE_TRUE;
}

//========================================================================================
//	jeUserPoly_CreateRef
//========================================================================================
JETAPI jeBoolean JETCC jeUserPoly_CreateRef(jeUserPoly *Poly)
{
	assert(jeUserPoly_IsValid(Poly) == JE_TRUE);

	if (Poly->RefCount >= JE_USERPOLY_MAX_REFCOUNTS)
	{
		assert(0);	// I know this is bad, but it is a common mistake to ignore the return values of jeObject_CreateRef!!!!
		return JE_FALSE;
	}

	Poly->RefCount++;

	return JE_TRUE;
}

//========================================================================================
//	jeUserPoly_Destroy
//========================================================================================
JETAPI void JETCC jeUserPoly_Destroy(jeUserPoly **Poly)
{
	assert(Poly);
	assert(jeUserPoly_IsValid(*Poly) == JE_TRUE);

	(*Poly)->RefCount--;

	if ((*Poly)->RefCount == 0)
		jeRam_Free(*Poly);

	*Poly = NULL;
}

//========================================================================================
//	jeUserPoly_UpdateTri
//  Vizard/Void/Jeff Muizelaar
//========================================================================================
JETAPI jeBoolean JETCC jeUserPoly_UpdateTri(	jeUserPoly *Poly, 
											const jeLVertex *v1, 
											const jeLVertex *v2, 
											const jeLVertex *v3, 
											const jeMaterialSpec *Material)
{
	assert(jeUserPoly_IsValid(Poly) == JE_TRUE);
	assert(Poly->Type == Type_Tri);
	assert(v1);
	assert(v2);
	assert(v3);

	Poly->Verts[0] = *v1;
	Poly->Verts[1] = *v2;
	Poly->Verts[2] = *v3;

	Poly->Material = Material;

	return JE_TRUE;
}

//========================================================================================
//	jeUserPoly_UpdateQuad
//========================================================================================
JETAPI jeBoolean JETCC jeUserPoly_UpdateQuad(	jeUserPoly *Poly, 
											const jeLVertex *v1, 
											const jeLVertex *v2, 
											const jeLVertex *v3, 
											const jeLVertex *v4, 
											const jeMaterialSpec *Material)
{
	assert(jeUserPoly_IsValid(Poly) == JE_TRUE);
	assert(Poly->Type == Type_Quad);
	assert(v1);
	assert(v2);
	assert(v3);
	assert(v4);

	Poly->Verts[0] = *v1;
	Poly->Verts[1] = *v2;
	Poly->Verts[2] = *v3;
	Poly->Verts[3] = *v4;

	Poly->Material = Material;

	return JE_TRUE;
}

//========================================================================================
//	jeUserPoly_UpdateSprite
//========================================================================================
JETAPI jeBoolean JETCC jeUserPoly_UpdateSprite(jeUserPoly *Poly, const jeLVertex *v1, const jeMaterialSpec *Material, jeFloat Scale)
{

	assert(jeUserPoly_IsValid(Poly) == JE_TRUE);
	assert(Poly->Type == Type_Sprite);
	assert(v1);

	Poly->Verts[0] = *v1;
	Poly->Material = Material;
	Poly->Scale = Scale;

	return JE_TRUE;
}

//========================================================================================
//	jeUserPoly_UpdateLine
//========================================================================================
JETAPI jeBoolean JETCC jeUserPoly_UpdateLine(jeUserPoly *Poly, const jeLVertex *v1, const jeLVertex *v2, jeFloat Scale)
{

	assert(jeUserPoly_IsValid(Poly) == JE_TRUE);
	assert(Poly->Type == Type_Line);
	assert(v1);

	Poly->Verts[0] = *v1;
	Poly->Verts[1] = *v2;
	Poly->Scale = Scale;

	return JE_TRUE;
}

//========================================================================================
//	jeUserPoly_Render
//========================================================================================
JETAPI jeBoolean JETCC jeUserPoly_Render(const jeUserPoly *Poly, const jeEngine *Engine, const jeCamera *Camera, const jeFrustum *Frustum)
{
	assert(jeUserPoly_IsValid(Poly) == JE_TRUE);
	assert(Engine);
	assert(Camera);
	assert(Frustum);

	switch (Poly->Type)
	{
		//Vizard/Void/Jeff Muizelaar
		case Type_Tri:
			return RenderTri(Poly, Engine, Camera, Frustum);

		case Type_Quad:
			return RenderQuad(Poly, Engine, Camera, Frustum);

		case Type_Sprite:
			return RenderSprite(Poly, Engine, Camera, Frustum);

		case Type_Line:
			return RenderLine(Poly, Engine, Camera, Frustum);

		default:
			assert(0);		// Illegal!!!
	}

	return JE_TRUE;
}

// As long as the poly coming in does not have more planes than the frustum, then the buffer only needs
//	to be as big as the number of clip planes*2
#define JE_USERPOLY_MAX_CLIPPLANES		(JE_FRUSTUM_MAX_PLANES) 
#define JE_USERPOLY_MAX_CLIPVERTS		(JE_USERPOLY_MAX_CLIPPLANES*2)

//========================================================================================
//	RenderTri
//  Vizard/Void/Jeff Muizelaar
//========================================================================================
static jeBoolean RenderTri(const jeUserPoly *Poly, const jeEngine *Engine, const jeCamera *Camera, const jeFrustum *Frustum)
{
	jeFrustum_LClipInfo		ClipInfo;
	jeLVertex				Work1[JE_USERPOLY_MAX_CLIPVERTS], Work2[JE_USERPOLY_MAX_CLIPVERTS];
	jeTLVertex				TLVerts[JE_USERPOLY_MAX_CLIPVERTS];

	assert(jeUserPoly_IsValid(Poly) == JE_TRUE);
	assert(Engine);
	assert(Camera);
	assert(Frustum);
	assert(Poly->Type == Type_Tri);
	assert(Frustum->NumPlanes <= JE_USERPOLY_MAX_CLIPPLANES);

	ClipInfo.SrcVerts = Poly->Verts;
	ClipInfo.NumSrcVerts = 3;
	ClipInfo.Work1 = Work1;
	ClipInfo.Work2 = Work2;
	ClipInfo.ClipFlags = 0xffff;

	// Clip the verts against the frustum
	// Clip UV, and RGB
	if (!jeFrustum_ClipLVertsXYZUVRGBA(Frustum, &ClipInfo))
		return JE_TRUE;		// Poly was clipped away
	
	// Transform and project the poly
	jeCamera_TransformAndProjectAndClampLArray(Camera, ClipInfo.DstVerts, TLVerts, ClipInfo.NumDstVerts);

	// Render it
	jeEngine_RenderPoly(Engine, TLVerts, ClipInfo.NumDstVerts, Poly->Material, Poly->Flags);

	return JE_TRUE;
}

//========================================================================================
//	RenderQuad
//========================================================================================
static jeBoolean RenderQuad(const jeUserPoly *Poly, const jeEngine *Engine, const jeCamera *Camera, const jeFrustum *Frustum)
{
	jeFrustum_LClipInfo		ClipInfo;
	jeLVertex				Work1[JE_USERPOLY_MAX_CLIPVERTS], Work2[JE_USERPOLY_MAX_CLIPVERTS];
	jeTLVertex				TLVerts[JE_USERPOLY_MAX_CLIPVERTS];

	assert(jeUserPoly_IsValid(Poly) == JE_TRUE);
	assert(Engine);
	assert(Camera);
	assert(Frustum);
	assert(Poly->Type == Type_Quad);
	assert(Frustum->NumPlanes <= JE_USERPOLY_MAX_CLIPPLANES);

	ClipInfo.SrcVerts = Poly->Verts;
	ClipInfo.NumSrcVerts = 4;
	ClipInfo.Work1 = Work1;
	ClipInfo.Work2 = Work2;
	ClipInfo.ClipFlags = 0xffff;

	// Clip the verts against the frustum
	// Clip UV, and RGB
	if (!jeFrustum_ClipLVertsXYZUVRGBA(Frustum, &ClipInfo))
		return JE_TRUE;		// Poly was clipped away
	
	// Transform and project the poly
	jeCamera_TransformAndProjectAndClampLArray(Camera, ClipInfo.DstVerts, TLVerts, ClipInfo.NumDstVerts);

	// Render it
	jeEngine_RenderPoly(Engine, TLVerts, ClipInfo.NumDstVerts, Poly->Material, Poly->Flags);

	return JE_TRUE;
}

//========================================================================================
//	RenderSprite
//========================================================================================
static jeBoolean RenderSprite(const jeUserPoly *Poly, const jeEngine *Engine, const jeCamera *Camera, const jeFrustum *Frustum)
{
	jeFrustum_LClipInfo		ClipInfo;
	jeLVertex				Work1[JE_USERPOLY_MAX_CLIPVERTS], Work2[JE_USERPOLY_MAX_CLIPVERTS];
	jeTLVertex				TLVerts[JE_USERPOLY_MAX_CLIPVERTS];
	jeLVertex				*pVerts;
	jeVec3d					Up, Left, Start;
	jeFloat					XScale, YScale, UShift, VShift;
	const jeXForm3d			*MXForm;

	assert(jeUserPoly_IsValid(Poly) == JE_TRUE);
	assert(Engine);
	assert(Camera);
	assert(Frustum);
	assert(Poly->Type == Type_Sprite);
	assert(Frustum->NumPlanes <= JE_USERPOLY_MAX_CLIPPLANES);

	pVerts = Work1;

	pVerts[0] = pVerts[1] = pVerts[2] = pVerts[3] = Poly->Verts[0];

	UShift = pVerts[0].u;
	VShift = pVerts[0].v;

	Start.X = pVerts[0].X;
	Start.Y = pVerts[0].Y;
	Start.Z = pVerts[0].Z;

	MXForm = jeCamera_WorldXForm(Camera);

	jeXForm3d_GetLeft(MXForm, &Left);
	jeXForm3d_GetUp(MXForm, &Up);

	XScale = (float)jeMaterialSpec_Width(Poly->Material) * Poly->Scale;
	YScale = (float)jeMaterialSpec_Height(Poly->Material) * Poly->Scale;

	jeVec3d_Scale(&Left, XScale*0.2f, &Left);
	jeVec3d_Scale(&Up, YScale*0.2f, &Up);

	pVerts->X = Start.X + Left.X + Up.X;
	pVerts->Y = Start.Y + Left.Y + Up.Y;
	pVerts->Z = Start.Z + Left.Z + Up.Z;
	pVerts->u = 0.0f + UShift;
	pVerts->v = 0.0f + VShift;

	pVerts++;
	
	pVerts->X = Start.X - Left.X + Up.X;
	pVerts->Y = Start.Y - Left.Y + Up.Y;
	pVerts->Z = Start.Z - Left.Z + Up.Z;
	pVerts->u = 1.0f + UShift;
	pVerts->v = 0.0f + VShift;
	
	pVerts++;
	
	pVerts->X = Start.X - Left.X - Up.X;
	pVerts->Y = Start.Y - Left.Y - Up.Y;
	pVerts->Z = Start.Z - Left.Z - Up.Z;
	pVerts->u = 1.0f + UShift;
	pVerts->v = 1.0f + VShift;

	pVerts++;
	
	pVerts->X = Start.X + Left.X - Up.X;
	pVerts->Y = Start.Y + Left.Y - Up.Y;
	pVerts->Z = Start.Z + Left.Z - Up.Z;
	pVerts->u = 0.0f + UShift;
	pVerts->v = 1.0f + VShift;

	//  Setup ClipInfo
	ClipInfo.SrcVerts = Work1;
	ClipInfo.NumSrcVerts = 4;
	ClipInfo.Work1 = Work1;
	ClipInfo.Work2 = Work2;
	ClipInfo.ClipFlags = 0xffff;

	// Clip the verts against the frustum
	// Clip UV, and RGBA
	if (!jeFrustum_ClipLVertsXYZUVRGBA(Frustum, &ClipInfo))
		return JE_TRUE;		// Poly was clipped away
	
	// Transform and project the poly
	jeCamera_TransformAndProjectAndClampLArray(Camera, ClipInfo.DstVerts, TLVerts, ClipInfo.NumDstVerts);

	// Render it
	jeEngine_RenderPoly(Engine, TLVerts, ClipInfo.NumDstVerts, Poly->Material, Poly->Flags);

	return JE_TRUE;
}

//========================================================================================
//	RenderLine
//========================================================================================
static jeBoolean RenderLine(const jeUserPoly *Poly, const jeEngine *Engine, const jeCamera *Camera, const jeFrustum *Frustum)
{
	jeFrustum_LClipInfo		ClipInfo;
	jeLVertex				Work1[JE_USERPOLY_MAX_CLIPVERTS], Work2[JE_USERPOLY_MAX_CLIPVERTS];
	const jeLVertex			*v1, *v2;
	jeTLVertex				TLVerts[JE_USERPOLY_MAX_CLIPVERTS];
	jeLVertex				*pVerts;
	jeVec3d					Left;
	const jeXForm3d			*MXForm;

	assert(jeUserPoly_IsValid(Poly) == JE_TRUE);
	assert(Engine);
	assert(Camera);
	assert(Frustum);
	assert(Poly->Type == Type_Line);
	assert(Frustum->NumPlanes <= JE_USERPOLY_MAX_CLIPPLANES);

	// Get camera info
	MXForm = jeCamera_WorldXForm(Camera);
	jeXForm3d_GetLeft(MXForm, &Left);

	v1 = &Poly->Verts[0];
	v2 = &Poly->Verts[1];

	pVerts = Work1;
	pVerts[0] = pVerts[1] = *v1;
	pVerts[2] = pVerts[3] = *v2;

	jeVec3d_Scale(&Left, Poly->Scale, &Left);

	if (v2->Y < v1->Y)
		jeVec3d_Inverse(&Left);

	pVerts->X = v1->X + Left.X;
	pVerts->Y = v1->Y + Left.Y;
	pVerts->Z = v1->Z + Left.Z;

	pVerts++;
	
	pVerts->X = v1->X - Left.X;
	pVerts->Y = v1->Y - Left.Y;
	pVerts->Z = v1->Z - Left.Z;
	
	pVerts++;

	pVerts->X = v2->X - Left.X;
	pVerts->Y = v2->Y - Left.Y;
	pVerts->Z = v2->Z - Left.Z;

	pVerts++;
	
	pVerts->X = v2->X + Left.X;
	pVerts->Y = v2->Y + Left.Y;
	pVerts->Z = v2->Z + Left.Z;

	//  Setup ClipInfo
	ClipInfo.SrcVerts = Work1;
	ClipInfo.NumSrcVerts = 4;
	ClipInfo.Work1 = Work1;
	ClipInfo.Work2 = Work2;
	ClipInfo.ClipFlags = 0xffff;

	// Clip the verts against the frustum
	// Clip UV, and RGB
	if (!jeFrustum_ClipLVertsXYZUVRGB(Frustum, &ClipInfo))
		return JE_TRUE;		// Poly was clipped away
	
	// Transform and project the poly
	jeCamera_TransformAndProjectAndClampLArray(Camera, ClipInfo.DstVerts, TLVerts, ClipInfo.NumDstVerts);

	// Render it
	jeEngine_RenderPoly(Engine, TLVerts, ClipInfo.NumDstVerts, NULL, Poly->Flags);

	return JE_TRUE;
}
