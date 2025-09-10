/****************************************************************************************/
/*  JEFRUSTUM.C                                                                         */
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
#include <stdio.h>
#include <assert.h>
#include <memory.h>		// memcpy

#include "jeFrustum.h"
#include "Camera._h"

static void SetUpFrustumBBox(jeFrustum *Info);


//================================================================================
//	jeFrustum_SetFromCamera
//================================================================================
JETAPI void JETCC jeFrustum_SetFromCamera(jeFrustum *Frustum, const jeCamera *Camera)
{
    jeFloat	s, c;
    jeVec3d	Normal;
	int32	i;

	// BEGIN - Far clip plane - paradoxnj 2/9/2005
	jeBoolean		ZFarEnable;
	jeFloat			ZFar;
	// END - Far clip plane - paradoxnj 2/9/2005

    jeCamera_GetViewAngleXSinCos(Camera,&s,&c);

    // Left clip plane
    Normal.X = s;
    Normal.Y = 0.0f;
    Normal.Z = -c;
	jeVec3d_Normalize(&Normal);
	Frustum->Planes[0].Normal = Normal;

    // Right clip plane
    Normal.X = -s;
	jeVec3d_Normalize(&Normal);
	Frustum->Planes[1].Normal = Normal;

    jeCamera_GetViewAngleYSinCos(Camera,&s,&c);

    // Bottom clip plane
    Normal.X = 0.0f;
    Normal.Y = s;
    Normal.Z = -c;
	jeVec3d_Normalize(&Normal);
	Frustum->Planes[2].Normal = Normal;

    // Top clip plane
    Normal.Y = -s;
	jeVec3d_Normalize(&Normal);
	Frustum->Planes[3].Normal = Normal;

	Frustum->FrontPlane = NULL;
	Frustum->NumPlanes = 4;

	// Clear all distances
	for (i=0; i<Frustum->NumPlanes; i++)
	{
		Frustum->Planes[i].Dist = 0.0f;
		Frustum->Planes[i].Type = Type_Any;
	}

	// BEGIN - Far clip plane - paradoxnj MODIFIED 3/9/2005
	jeCamera_GetFarClipPlane(Camera, &ZFarEnable, &ZFar);

	if (ZFarEnable)
	{
		// Farclip plane
		Normal.X = 0.0f;
		Normal.Y = 0.0f;
		Normal.Z = 1.0f;
		jeVec3d_Normalize(&Normal);
		Frustum->Planes[4].Normal = Normal;

		Frustum->Planes[4].Dist = -(ZFar/jeCamera_GetZScale(Camera));
//		Frustum->Planes[4].Dist = -jeCamera_GetZScale(Camera);
		Frustum->Planes[4].Type = Type_Any;

		Frustum->NumPlanes = 5;
	}
	// END - Far clip plane - paradoxnj MODIFIED 3/9/2005

	// Get BBox info for fast BBox rejection against frustum...
	SetUpFrustumBBox(Frustum);
}

//================================================================================
//	jeFrustum_SetFromCamera
//================================================================================
JETAPI void JETCC jeFrustum_SetWorldSpaceFromCamera(jeFrustum *pFrustum, const jeCamera *Camera)
{
	jeFrustum CamFrustum;

	jeFrustum_SetFromCamera(&CamFrustum,Camera);
	jeFrustum_TransformToWorldSpace(&CamFrustum,Camera,pFrustum);
	pFrustum->FrontPlane = NULL;
}

//================================================================================
//	jeFrustum_SetFromVerts
//	Create a frustum looking through a poly (from the POV)
//================================================================================
JETAPI jeBoolean JETCC jeFrustum_SetFromVerts(jeFrustum *Frustum, const jeVec3d *POV, const jeVec3d *Verts, int32 NumVerts)
{
	int32			NextVert;
	const jeVec3d	*Vert1, *Vert2;
	jeVec3d			Vect1, Vect2;
	jePlane			*Planes;
	int32			i;

	if (NumVerts >= JE_FRUSTUM_MAX_PLANES)
		return JE_FALSE;		// Too many planes!!!
	
	Planes = Frustum->Planes;

	Frustum->NumPlanes = 0;

	for (i=0; i< NumVerts; i++)
	{
		NextVert = ((i+1) < NumVerts) ? (i+1) : 0;

		Vert1 = &Verts[i];
		Vert2 = &Verts[NextVert];

		// FIXME:  Check for coplanar edges???
		if (jeVec3d_Compare(Vert1, Vert2, 0.1f))	// Degenerate edge...
			continue;	

		jeVec3d_Subtract(Vert2, Vert1, &Vect1);
		jeVec3d_Subtract(POV, Vert1, &Vect2);

		jeVec3d_CrossProduct(&Vect2, &Vect1, &Planes->Normal);
		jeVec3d_Normalize(&Planes->Normal);

		Planes->Dist = jeVec3d_DotProduct(POV, &Planes->Normal);
		Planes->Type = Type_Any;

		Planes++;
		Frustum->NumPlanes++;
	}

	// Get BBox info for fast BBox rejection against frustum...
	SetUpFrustumBBox(Frustum);
	
	return JE_TRUE;
}

//================================================================================
//	jeFrustum_SetFromVerts2
//	Create a frustum looking through a poly (from the origin)
//================================================================================
JETAPI jeBoolean JETCC jeFrustum_SetFromVerts2(jeFrustum *Frustum, const jeVec3d *Verts, int32 NumVerts)
{
	int32			NextVert;
	const jeVec3d	*pVert1, *pVert2;
	jeVec3d			Vect;
	jePlane			*Planes;
	int32			i;

	if (NumVerts >= JE_FRUSTUM_MAX_PLANES)
		return JE_FALSE;		// Too many planes!!!
	
	Planes = Frustum->Planes;

	Frustum->FrontPlane = NULL;
	Frustum->NumPlanes = 0;

	for (i=0; i< NumVerts; i++)
	{
		NextVert = ((i+1) < NumVerts) ? (i+1) : 0;

		pVert1 = &Verts[i];
		pVert2 = &Verts[NextVert];

		// FIXME:  Check for coplanar edges???
		if (jeVec3d_Compare(pVert1, pVert2, 0.1f))	// Degenerate edge...
			continue;	

		jeVec3d_Subtract(pVert1, pVert2, &Vect);
		
		jeVec3d_CrossProduct(&Vect, pVert2, &Planes->Normal);
		jeVec3d_Normalize(&Planes->Normal);

		Planes->Dist = 0.0f;
		Planes->Type = Type_Any;

		Planes++;
		Frustum->NumPlanes++;
	}

	// Get BBox info for fast BBox rejection against frustum...
	SetUpFrustumBBox(Frustum);
	
	return JE_TRUE;
}

//================================================================================
//	jeFrustum_SetFromLVerts
//	Create a frustum looking through a poly (from the POV)
//================================================================================
JETAPI jeBoolean JETCC jeFrustum_SetFromLVerts(jeFrustum *Frustum, const jeVec3d *POV, const jeLVertex *Verts, int32 NumVerts)
{
	int32			NextVert;
	jeVec3d			*Vert1, *Vert2;
	jeVec3d			Vect1, Vect2;
	jePlane			*Planes;
	int32			i;

	if (NumVerts >= JE_FRUSTUM_MAX_PLANES)
		return JE_FALSE;		// Too many planes!!!
	
	Planes = Frustum->Planes;

	Frustum->FrontPlane = NULL;
	Frustum->NumPlanes = 0;

	for (i=0; i< NumVerts; i++)
	{
		NextVert = ((i+1) < NumVerts) ? (i+1) : 0;

		Vert1 = (jeVec3d*)&Verts[i];
		Vert2 = (jeVec3d*)&Verts[NextVert];

		// FIXME:  Check for coplanar edges???
		if (jeVec3d_Compare(Vert1, Vert2, 0.1f))	// Degenerate edge...
			continue;	

		jeVec3d_Subtract(Vert2, Vert1, &Vect1);
		jeVec3d_Subtract(POV, Vert1, &Vect2);

		jeVec3d_CrossProduct(&Vect2, &Vect1, &Planes->Normal);
		//jeVec3d_CrossProduct(&Vect1, &Vect2, &Planes->Normal);
		jeVec3d_Normalize(&Planes->Normal);

		Planes->Dist = jeVec3d_DotProduct(POV, &Planes->Normal);
		Planes->Type = Type_Any;

		Planes++;
		Frustum->NumPlanes++;
	}

	// Get BBox info for fast BBox rejection against frustum...
	SetUpFrustumBBox(Frustum);
	
	return JE_TRUE;
}

//================================================================================
//	jeFrustum_SetFromLVerts2
//	Create a frustum looking through a poly (from the origin)
//================================================================================
JETAPI jeBoolean JETCC jeFrustum_SetFromLVerts2(jeFrustum *Frustum, const jeLVertex *Verts, int32 NumVerts, jeBoolean Flip)
{
	int32			NextVert;
	const jeVec3d	*pVert1, *pVert2;
	jeVec3d			Vect;
	jePlane			*Planes;
	int32			i;

	if (NumVerts >= JE_FRUSTUM_MAX_PLANES)
		return JE_FALSE;		// Too many planes!!!
	
	Planes = Frustum->Planes;

	Frustum->FrontPlane = NULL;
	Frustum->NumPlanes = 0;

	for (i=0; i< NumVerts; i++)
	{
		NextVert = ((i+1) < NumVerts) ? (i+1) : 0;

		pVert1 = (jeVec3d*)&Verts[i];
		pVert2 = (jeVec3d*)&Verts[NextVert];

		// FIXME:  Check for coplanar edges???
		if (jeVec3d_Compare(pVert1, pVert2, 0.1f))	// Degenerate edge...
			continue;	

		jeVec3d_Subtract(pVert1, pVert2, &Vect);
		
		if (Flip)
			jeVec3d_CrossProduct(pVert2, &Vect, &Planes->Normal);
		else
			jeVec3d_CrossProduct(&Vect, pVert2, &Planes->Normal);

		jeVec3d_Normalize(&Planes->Normal);

		Planes->Dist = 0.0f;
		Planes->Type = Type_Any;

		Planes++;
		Frustum->NumPlanes++;
	}

	// Get BBox info for fast BBox rejection against frustum...
	SetUpFrustumBBox(Frustum);
	
	return JE_TRUE;
}

//================================================================================
//	jeFrustum_AddPlane
//================================================================================
JETAPI jeBoolean JETCC jeFrustum_AddPlane(jeFrustum *Frustum, const jePlane *SrcPlane, jeBoolean FrontPlane)
{
	jePlane			*pDstPlane;

	if (Frustum->NumPlanes >= JE_FRUSTUM_MAX_PLANES)
		return JE_FALSE;		// Out of space!

	pDstPlane = &Frustum->Planes[Frustum->NumPlanes++];

	*pDstPlane = *SrcPlane;

	if (FrontPlane)
		Frustum->FrontPlane = pDstPlane;

	return JE_TRUE;
}

//================================================================================
//	jeFrustum_RotateToWorldSpace
//================================================================================
JETAPI void JETCC jeFrustum_RotateToWorldSpace(const jeFrustum *In, const jeCamera *Camera, jeFrustum *Out)
{
    int32			i;
	const jePlane	*InPlane;
	jePlane			*OutPlane;
	const jeXForm3d	*InvXForm;

	InvXForm = jeCamera_WorldXForm(Camera);			// Get CameraToWorldXForm

	InPlane = In->Planes;
	OutPlane = Out->Planes;

	// Rotate all the planes
	for (i=0; i<In->NumPlanes; i++, InPlane++, OutPlane++)
	{
		OutPlane->Type = InPlane->Type;

		jePlane_Rotate(InPlane, InvXForm, OutPlane);

		// If this InPlane is the front plane in the In frustum, then assign the 
		// corresponding OutPlane as the frontplane in the Out Frustum
		if (InPlane == In->FrontPlane)
			Out->FrontPlane = OutPlane;
	}

	Out->NumPlanes = In->NumPlanes;

	// Get BBox info for fast BBox rejection against frustum...
	SetUpFrustumBBox(Out);
}

//================================================================================
//	jeFrustum_TransformToWorldSpace
//================================================================================
JETAPI void JETCC jeFrustum_TransformToWorldSpace(const jeFrustum *In, const jeCamera *Camera, jeFrustum *Out)
{
    int32			i;
	const jePlane	*InPlane;
	jePlane			*OutPlane;
	const jeXForm3d	*InvXForm;

	InvXForm = jeCamera_WorldXForm(Camera);			// Get CameraToWorldXForm

	InPlane = In->Planes;
	OutPlane = Out->Planes;

	// Transform all the planes
	for (i=0; i<In->NumPlanes; i++, InPlane++, OutPlane++)
	{
		if (InPlane->Dist)
			jePlane_Transform(InPlane, InvXForm, OutPlane);
		else
		{
			// Transformation for plane is easy if plane is at the origin
			jePlane_Rotate(InPlane, InvXForm, OutPlane);		
			OutPlane->Dist = jeVec3d_DotProduct(jeCamera_GetPov(Camera), &OutPlane->Normal) - CLIP_PLANE_EPSILON;
		}
			
		// If this InPlane is the front plane in the In frustum, then assign the 
		// corresponding OutPlane as the frontplane in the Out Frustum
		if (InPlane == In->FrontPlane)
			Out->FrontPlane = OutPlane;
	}

	Out->NumPlanes = In->NumPlanes;

	// Get BBox info for fast BBox rejection against frustum...
	SetUpFrustumBBox(Out);
}

//================================================================================
//	jeFrustum_Rotate
//================================================================================
JETAPI void JETCC jeFrustum_Rotate(const jeFrustum *In, const jeXForm3d *XForm, jeFrustum *Out)
{
    int32			i;
	const jePlane	*InPlane;
	jePlane			*OutPlane;

	InPlane = In->Planes;
	OutPlane = Out->Planes;

	// Transform all the planes
	for (i=0; i<In->NumPlanes; i++, InPlane++, OutPlane++)
	{
		jePlane_Rotate(InPlane, XForm, OutPlane);		

		// If this InPlane is the front plane in the In frustum, then assign the 
		// corresponding OutPlane as the frontplane in the Out Frustum
		if (InPlane == In->FrontPlane)
			Out->FrontPlane = OutPlane;
	}

	Out->NumPlanes = In->NumPlanes;

	// Get BBox info for fast BBox rejection against frustum...
	SetUpFrustumBBox(Out);
}

//================================================================================
//	jeFrustum_Transform
//================================================================================
JETAPI void JETCC jeFrustum_Transform(const jeFrustum *In, const jeXForm3d *XForm, jeFrustum *Out)
{
    int32			i;
	const jePlane	*InPlane;
	jePlane			*OutPlane;

	InPlane = In->Planes;
	OutPlane = Out->Planes;

	// Transform all the planes
	for (i=0; i<In->NumPlanes; i++, InPlane++, OutPlane++)
	{
		if (InPlane->Dist)
			jePlane_Transform(InPlane, XForm, OutPlane);
		else
		{
			// Transformation for plane is easy if plane is at the origin
			jePlane_Rotate(InPlane, XForm, OutPlane);		
			OutPlane->Dist = jeVec3d_DotProduct(&XForm->Translation, &OutPlane->Normal) - CLIP_PLANE_EPSILON;
		}
			
		// If this InPlane is the front plane in the In frustum, then assign the 
		// corresponding OutPlane as the frontplane in the Out Frustum
		if (InPlane == In->FrontPlane)
			Out->FrontPlane = OutPlane;
	}

	Out->NumPlanes = In->NumPlanes;

	// Get BBox info for fast BBox rejection against frustum...
	SetUpFrustumBBox(Out);
}

//================================================================================
//	jeFrustum_Transform
//================================================================================
JETAPI void JETCC jeFrustum_TransformRenorm(const jeFrustum *In, const jeXForm3d *XForm, jeFrustum *Out)
{
    int32			i;
	const jePlane	*InPlane;
	jePlane			*OutPlane;

	InPlane = In->Planes;
	OutPlane = Out->Planes;

	// Transform all the planes
	for (i=0; i<In->NumPlanes; i++, InPlane++, OutPlane++)
	{
		jePlane_TransformRenorm(InPlane, XForm, OutPlane);
			
		// If this InPlane is the front plane in the In frustum, then assign the 
		// corresponding OutPlane as the frontplane in the Out Frustum
		if (InPlane == In->FrontPlane)
			Out->FrontPlane = OutPlane;
	}

	Out->NumPlanes = In->NumPlanes;

	// Get BBox info for fast BBox rejection against frustum...
	SetUpFrustumBBox(Out);
}

//================================================================================
//	jeFrustum_Transform
//================================================================================
JETAPI void JETCC jeFrustum_TransformAnchored(jeFrustum *F, const jeXForm3d *XForm, const jeVec3d * Anchor)
{
int32			i;
jePlane			*Plane;

	Plane = F->Planes;

	// Transform all the planes
	for (i=0; i<F->NumPlanes; i++, Plane++)
	{
		jeXForm3d_Rotate(XForm, &Plane->Normal, &Plane->Normal);
		jeVec3d_Normalize(&Plane->Normal); // if XForm isn't normalized, need to renorm here
		// Find the Dist of the new plane by projecting the transformed point on the new plane
		Plane->Dist = jeVec3d_DotProduct(&Plane->Normal, Anchor);
		Plane->Type = Type_Any;
	}

	// Get BBox info for fast BBox rejection against frustum...
	SetUpFrustumBBox(F);
}

//================================================================================
//	SetUpFrustumBBox
//	Setup bbox min/max test for the quadrant the frustum planes are in...
//================================================================================
static void SetUpFrustumBBox(jeFrustum *Info)
{
	int32		i, *Index;

	Index = Info->FrustumBBoxIndexes;

	for (i=0 ; i<Info->NumPlanes ; i++)
	{
		if (Info->Planes[i].Normal.X < 0)			// Plane is facing left
		{
			Index[0] = 0;							// Take LEFT side of box for complete rejection
			Index[3] = 4;							// Take RIGHT side of box for totally accept
		}
		else										// Plane is facing right
		{
			Index[0] = 4;							// Take RIGHT side of box for complete rejection
			Index[3] = 0;							// Take LEFT side of box for totally accept
		}
		if (Info->Planes[i].Normal.Y < 0)			// Same rules apply for below...
		{
			Index[1] = 1;
			Index[4] = 5;
		}
		else
		{
			Index[1] = 5;
			Index[4] = 1;
		}
		if (Info->Planes[i].Normal.Z < 0)
		{
			Index[2] = 2;
			Index[5] = 6;
		}
		else
		{
			Index[2] = 6;
			Index[5] = 2;
		}

		Info->pFrustumBBoxIndexes[i] = Index;
		Index += 6;
	}
}

//================================================================================
//	jeFrustum_SetClipFlagsFromBBox
//================================================================================
JETAPI jeBoolean JETCC jeFrustum_SetClipFlagsFromExtBox(const jeFrustum *Frustum,const jeExtBox *BBox,uint32 ClipFlags,uint32 *pClipFlags)
{
const jeFloat *MinMaxs;
int32		*Index, p;
uint32		mask;
jeFloat		Dist2;
jeVec3d		Pnt;
const jePlane *pPlane;

	assert(Frustum);
	assert(BBox);

	MinMaxs = (const jeFloat *)&(BBox->Min);

	for (pPlane = Frustum->Planes, p=0; ; p++, pPlane++)
	{
		mask = 1UL<<p;
		if ( mask > ClipFlags )
			break;
		if (!(ClipFlags & mask))
			continue;

		Index = Frustum->pFrustumBBoxIndexes[p];

		Pnt.X = MinMaxs[Index[0]];
		Pnt.Y = MinMaxs[Index[1]];
		Pnt.Z = MinMaxs[Index[2]];
		
		Dist2 = jeVec3d_DotProduct(&Pnt, &pPlane->Normal);
		Dist2 -= pPlane->Dist;

		if (Dist2 <= 0)
		{
			// box is totally outside
			return JE_FALSE;
		}

		Pnt.X = MinMaxs[Index[3]];
		Pnt.Y = MinMaxs[Index[4]];
		Pnt.Z = MinMaxs[Index[5]];

		Dist2 = jeVec3d_DotProduct(&Pnt, &pPlane->Normal);
		Dist2 -= pPlane->Dist;

		if (Dist2 >= 0)		
			ClipFlags ^= mask;		// Don't need to clip to this plane anymore
	}

	if ( pClipFlags )
		*pClipFlags = ClipFlags;

   return JE_TRUE;
}

//================================================================================
//	jeFrustum_ClipLVertsToPlaneXYZUV
//	Clips X, Y, Z, u, v
//================================================================================
JETAPI jeBoolean JETCC jeFrustum_ClipLVertsToPlaneXYZUV(	const jePlane *pPlane, 
											const jeLVertex *pIn, jeLVertex *pOut,
											int32 NumVerts, int32 *OutVerts)
{
    int32			i, CurIn, NextIn;
    float			CurDot, NextDot, Scale;
    const jeLVertex	*pIn2, *pNext;
	jeLVertex		*pOut2;
	const jeVec3d	*pNormal;

    pIn2 = pIn;
    pOut2= pOut;
	pNormal = &pPlane->Normal;

	CurDot = (pIn->X * pNormal->X) + (pIn->Y * pNormal->Y) + (pIn->Z * pNormal->Z);
    CurIn = (CurDot >= pPlane->Dist);

    for (i=0 ; i<NumVerts ; i++)
    {
		pNext = ((i+1) < NumVerts) ? (pIn2+1): pIn;

        // Keep the current vertex if it's inside the plane
        if (CurIn) 
		{
            pOut2->X = pIn2->X;
            pOut2->Y = pIn2->Y;
            pOut2->Z = pIn2->Z;
            pOut2->u = pIn2->u;
            pOut2->v = pIn2->v;
			pOut2++;
		}

		NextDot = (pNext->X * pNormal->X) + (pNext->Y * pNormal->Y) + (pNext->Z * pNormal->Z);
		NextIn = (NextDot >= pPlane->Dist);

        // Add a clipped vertex if one end of the current edge is
        // inside the plane and the other is outside
        if (CurIn != NextIn)
        {
			Scale = (pPlane->Dist - CurDot) / (NextDot - CurDot);

            pOut2->X = pIn2->X + (pNext->X - pIn2->X) * Scale;
            pOut2->Y = pIn2->Y + (pNext->Y - pIn2->Y) * Scale;
            pOut2->Z = pIn2->Z + (pNext->Z - pIn2->Z) * Scale;

            pOut2->u = pIn2->u + (pNext->u - pIn2->u) * Scale;
            pOut2->v = pIn2->v + (pNext->v - pIn2->v) * Scale;

            pOut2++;
        }

        CurDot = NextDot;
        CurIn = NextIn;
        pIn2++;
    }

    *OutVerts = pOut2 - pOut;

    if (*OutVerts < 3)
        return JE_FALSE;

    return JE_TRUE;
}

//================================================================================
//	jeFrustum_ClipLVertsToPlaneXYZUVRGB
//	Clips X, Y, Z, u, v
//================================================================================
JETAPI jeBoolean JETCC jeFrustum_ClipLVertsToPlaneXYZUVRGB(	const jePlane *pPlane, 
												const jeLVertex *pIn, jeLVertex *pOut,
												int32 NumVerts, int32 *OutVerts)
{
    int32			i, CurIn, NextIn;
    float			CurDot, NextDot, Scale;
    const jeLVertex	*pIn2, *pNext;
	jeLVertex		*pOut2;
	const jeVec3d	*pNormal;

    pIn2 = pIn;
    pOut2= pOut;
	pNormal = &pPlane->Normal;

	CurDot = (pIn->X * pNormal->X) + (pIn->Y * pNormal->Y) + (pIn->Z * pNormal->Z);
    CurIn = (CurDot >= pPlane->Dist);

    for (i=0 ; i<NumVerts ; i++)
    {
		pNext = ((i+1) < NumVerts) ? (pIn2+1): pIn;

        // Keep the current vertex if it's inside the plane
        if (CurIn) 
		{
            memcpy(pOut2, pIn2, sizeof(jeLVertex));
			pOut2++;
		}

		NextDot = (pNext->X * pNormal->X) + (pNext->Y * pNormal->Y) + (pNext->Z * pNormal->Z);
		NextIn = (NextDot >= pPlane->Dist);

        // Add a clipped vertex if one end of the current edge is
        // inside the plane and the other is outside
        if (CurIn != NextIn)
        {
			Scale = (pPlane->Dist - CurDot) / (NextDot - CurDot);

            pOut2->X = pIn2->X + (pNext->X - pIn2->X) * Scale;
            pOut2->Y = pIn2->Y + (pNext->Y - pIn2->Y) * Scale;
            pOut2->Z = pIn2->Z + (pNext->Z - pIn2->Z) * Scale;

            pOut2->u = pIn2->u + (pNext->u - pIn2->u) * Scale;
            pOut2->v = pIn2->v + (pNext->v - pIn2->v) * Scale;

            pOut2->r = pIn2->r + (pNext->r - pIn2->r) * Scale;
            pOut2->g = pIn2->g + (pNext->g - pIn2->g) * Scale;
            pOut2->b = pIn2->b + (pNext->b - pIn2->b) * Scale;

            pOut2++;
        }

        CurDot = NextDot;
        CurIn = NextIn;
        pIn2++;
    }

    *OutVerts = pOut2 - pOut;

    if (*OutVerts < 3)
        return JE_FALSE;

    return JE_TRUE;
}

//================================================================================
//	jeFrustum_ClipLVertsToPlaneXYZUVRGBA
//================================================================================
JETAPI jeBoolean JETCC jeFrustum_ClipLVertsToPlaneXYZUVRGBA(	const jePlane *pPlane, 
												const jeLVertex *pIn, jeLVertex *pOut,
												int32 NumVerts, int32 *OutVerts)
{
    int32			i, CurIn, NextIn;
    float			CurDot, NextDot, Scale;
    const jeLVertex	*pIn2, *pNext;
	jeLVertex		*pOut2;
	const jeVec3d	*pNormal;

    pIn2 = pIn;
    pOut2= pOut;
	pNormal = &pPlane->Normal;

	CurDot = (pIn->X * pNormal->X) + (pIn->Y * pNormal->Y) + (pIn->Z * pNormal->Z);
    CurIn = (CurDot >= pPlane->Dist);

    for (i=0 ; i<NumVerts ; i++)
    {
		pNext = ((i+1) < NumVerts) ? (pIn2+1): pIn;

        // Keep the current vertex if it's inside the plane
        if (CurIn) 
		{
            *pOut2 = *pIn2;
			pOut2++;
		}

		NextDot = (pNext->X * pNormal->X) + (pNext->Y * pNormal->Y) + (pNext->Z * pNormal->Z);
		NextIn = (NextDot >= pPlane->Dist);

        // Add a clipped vertex if one end of the current edge is
        // inside the plane and the other is outside
        if (CurIn != NextIn)
        {
			Scale = (pPlane->Dist - CurDot) / (NextDot - CurDot);

            pOut2->X = pIn2->X + (pNext->X - pIn2->X) * Scale;
            pOut2->Y = pIn2->Y + (pNext->Y - pIn2->Y) * Scale;
            pOut2->Z = pIn2->Z + (pNext->Z - pIn2->Z) * Scale;

            pOut2->u = pIn2->u + (pNext->u - pIn2->u) * Scale;
            pOut2->v = pIn2->v + (pNext->v - pIn2->v) * Scale;

            pOut2->r = pIn2->r + (pNext->r - pIn2->r) * Scale;
            pOut2->g = pIn2->g + (pNext->g - pIn2->g) * Scale;
            pOut2->b = pIn2->b + (pNext->b - pIn2->b) * Scale;
            pOut2->a = pIn2->a + (pNext->a - pIn2->a) * Scale;

            pOut2++;
        }

        CurDot = NextDot;
        CurIn = NextIn;
        pIn2++;
    }

    *OutVerts = pOut2 - pOut;

    if (*OutVerts < 3)
        return JE_FALSE;

    return JE_TRUE;
}

//================================================================================
//	jeFrustum_ClipLVertsToPlaneXYZUVRGBAS
//================================================================================
JETAPI jeBoolean JETCC jeFrustum_ClipLVertsToPlaneXYZUVRGBAS(const jePlane *pPlane, 
												const jeLVertex *pIn, jeLVertex *pOut,
												int32 NumVerts, int32 *OutVerts)
{
    int32			i, CurIn, NextIn;
    float			CurDot, NextDot, Scale;
    const jeLVertex	*pIn2, *pNext;
	jeLVertex		*pOut2;
	const jeVec3d	*pNormal;

    pIn2 = pIn;
    pOut2= pOut;
	pNormal = &pPlane->Normal;

	CurDot = (pIn->X * pNormal->X) + (pIn->Y * pNormal->Y) + (pIn->Z * pNormal->Z);
    CurIn = (CurDot >= pPlane->Dist);

    for (i=0 ; i<NumVerts ; i++)
    {
		pNext = ((i+1) < NumVerts) ? (pIn2+1): pIn;

        // Keep the current vertex if it's inside the plane
        if (CurIn) 
		{
            *pOut2 = *pIn2;
			pOut2++;
		}

		NextDot = (pNext->X * pNormal->X) + (pNext->Y * pNormal->Y) + (pNext->Z * pNormal->Z);
		NextIn = (NextDot >= pPlane->Dist);

        // Add a clipped vertex if one end of the current edge is
        // inside the plane and the other is outside
        if (CurIn != NextIn)
        {
			Scale = (pPlane->Dist - CurDot) / (NextDot - CurDot);

            pOut2->X = pIn2->X + (pNext->X - pIn2->X) * Scale;
            pOut2->Y = pIn2->Y + (pNext->Y - pIn2->Y) * Scale;
            pOut2->Z = pIn2->Z + (pNext->Z - pIn2->Z) * Scale;

            pOut2->u = pIn2->u + (pNext->u - pIn2->u) * Scale;
            pOut2->v = pIn2->v + (pNext->v - pIn2->v) * Scale;

            pOut2->r = pIn2->r + (pNext->r - pIn2->r) * Scale;
            pOut2->g = pIn2->g + (pNext->g - pIn2->g) * Scale;
            pOut2->b = pIn2->b + (pNext->b - pIn2->b) * Scale;
            pOut2->a = pIn2->a + (pNext->a - pIn2->a) * Scale;

            pOut2->sr = pIn2->sr + (pNext->sr - pIn2->sr) * Scale;
            pOut2->sg = pIn2->sg + (pNext->sg - pIn2->sg) * Scale;
            pOut2->sb = pIn2->sb + (pNext->sb - pIn2->sb) * Scale;

            pOut2++;
        }

        CurDot = NextDot;
        CurIn = NextIn;
        pIn2++;
    }

    *OutVerts = pOut2 - pOut;

    if (*OutVerts < 3)
        return JE_FALSE;

    return JE_TRUE;
}

//================================================================================
//	jeFrustum_ClipLVertsXYZUV
//================================================================================
JETAPI jeBoolean JETCC jeFrustum_ClipLVertsXYZUV(const jeFrustum *Frustum, jeFrustum_LClipInfo *ClipInfo)
{
	jeLVertex		*pSrc, *pDst;
	int32			NumVerts, i;
	const jePlane	*pPlane;
	uint32			ClipFlags;

	assert(Frustum);
	assert(ClipInfo);
	assert(ClipInfo->Work2 != ClipInfo->SrcVerts);

	ClipFlags = ClipInfo->ClipFlags;

	if (!ClipFlags)		// Early out if possible
	{
		ClipInfo->NumDstVerts = ClipInfo->NumSrcVerts;
		ClipInfo->DstVerts = (jeLVertex*)ClipInfo->SrcVerts;
		return JE_TRUE;
	}

	pSrc = (jeLVertex*)ClipInfo->SrcVerts;
	pDst = ClipInfo->Work2;

	NumVerts = ClipInfo->NumSrcVerts;

	for (pPlane = Frustum->Planes, i=0; i< Frustum->NumPlanes; i++, pPlane++)
	{
		if (!(ClipFlags & (1<<i)))
			continue;

		if (!jeFrustum_ClipLVertsToPlaneXYZUV(pPlane, pSrc, pDst, NumVerts, &NumVerts))
			return JE_FALSE;

		if (pDst == ClipInfo->Work2)
		{
			pSrc = ClipInfo->Work2;
			pDst = ClipInfo->Work1;
		}
		else
		{
			pSrc = ClipInfo->Work1;
			pDst = ClipInfo->Work2;
		}
	}

	ClipInfo->NumDstVerts = NumVerts;
	ClipInfo->DstVerts = pSrc;

	return JE_TRUE;
}

//================================================================================
//	jeFrustum_ClipLVertsXYZUVRGB
//================================================================================
JETAPI jeBoolean JETCC jeFrustum_ClipLVertsXYZUVRGB(const jeFrustum *Frustum, jeFrustum_LClipInfo *ClipInfo)
{
	jeLVertex		*pSrc, *pDst;
	int32			NumVerts, i;
	const jePlane	*pPlane;
	uint32			ClipFlags;

	assert(Frustum);
	assert(ClipInfo);
	assert(ClipInfo->Work2 != ClipInfo->SrcVerts);

	ClipFlags = ClipInfo->ClipFlags;

	if (!ClipFlags)		// Early out if possible
	{
		ClipInfo->NumDstVerts = ClipInfo->NumSrcVerts;
		ClipInfo->DstVerts = (jeLVertex*)ClipInfo->SrcVerts;
		return JE_TRUE;
	}

	pSrc = (jeLVertex*)ClipInfo->SrcVerts;
	pDst = ClipInfo->Work2;

	NumVerts = ClipInfo->NumSrcVerts;

	for (pPlane = Frustum->Planes, i=0; i< Frustum->NumPlanes; i++, pPlane++)
	{
		if (!(ClipFlags & (1<<i)))
			continue;

		if (!jeFrustum_ClipLVertsToPlaneXYZUVRGB(pPlane, pSrc, pDst, NumVerts, &NumVerts))
			return JE_FALSE;

		if (pDst == ClipInfo->Work2)
		{
			pSrc = ClipInfo->Work2;
			pDst = ClipInfo->Work1;
		}
		else
		{
			pSrc = ClipInfo->Work1;
			pDst = ClipInfo->Work2;
		}
	}

	ClipInfo->NumDstVerts = NumVerts;
	ClipInfo->DstVerts = pSrc;

	return JE_TRUE;
}

//================================================================================
//	jeFrustum_ClipLVertsXYZUVRGBA
//================================================================================
JETAPI jeBoolean JETCC jeFrustum_ClipLVertsXYZUVRGBA(const jeFrustum *Frustum, jeFrustum_LClipInfo *ClipInfo)
{
	jeLVertex		*pSrc, *pDst;
	int32			NumVerts, i;
	const jePlane	*pPlane;
	uint32			ClipFlags;

	assert(Frustum);
	assert(ClipInfo);
	assert(ClipInfo->Work2 != ClipInfo->SrcVerts);

	ClipFlags = ClipInfo->ClipFlags;

	if (!ClipFlags)		// Early out if possible
	{
		ClipInfo->NumDstVerts = ClipInfo->NumSrcVerts;
		ClipInfo->DstVerts = (jeLVertex*)ClipInfo->SrcVerts;
		return JE_TRUE;
	}

	pSrc = (jeLVertex*)ClipInfo->SrcVerts;
	pDst = ClipInfo->Work2;

	NumVerts = ClipInfo->NumSrcVerts;

	for (pPlane = Frustum->Planes, i=0; i< Frustum->NumPlanes; i++, pPlane++)
	{
		if (!(ClipFlags & (1<<i)))
			continue;

		if (!jeFrustum_ClipLVertsToPlaneXYZUVRGBA(pPlane, pSrc, pDst, NumVerts, &NumVerts))
			return JE_FALSE;

		if (pDst == ClipInfo->Work2)
		{
			pSrc = ClipInfo->Work2;
			pDst = ClipInfo->Work1;
		}
		else
		{
			pSrc = ClipInfo->Work1;
			pDst = ClipInfo->Work2;
		}
	}

	ClipInfo->NumDstVerts = NumVerts;
	ClipInfo->DstVerts = pSrc;

	return JE_TRUE;
}

//================================================================================
//	jeFrustum_ClipLVertsXYZUVRGBAS
//================================================================================
JETAPI jeBoolean JETCC jeFrustum_ClipLVertsXYZUVRGBAS(const jeFrustum *Frustum, jeFrustum_LClipInfo *ClipInfo)
{
	jeLVertex		*pSrc, *pDst;
	int32			NumVerts, i;
	const jePlane	*pPlane;
	uint32			ClipFlags;

	assert(Frustum);
	assert(ClipInfo);
	assert(ClipInfo->Work2 != ClipInfo->SrcVerts);

	ClipFlags = ClipInfo->ClipFlags;

	if (!ClipFlags)		// Early out if possible
	{
		ClipInfo->NumDstVerts = ClipInfo->NumSrcVerts;
		ClipInfo->DstVerts = (jeLVertex*)ClipInfo->SrcVerts;
		return JE_TRUE;
	}

	pSrc = (jeLVertex*)ClipInfo->SrcVerts;
	pDst = ClipInfo->Work2;

	NumVerts = ClipInfo->NumSrcVerts;

	for (pPlane = Frustum->Planes, i=0; i< Frustum->NumPlanes; i++, pPlane++)
	{
		if (!(ClipFlags & (1<<i)))
			continue;

		if (!jeFrustum_ClipLVertsToPlaneXYZUVRGBAS(pPlane, pSrc, pDst, NumVerts, &NumVerts))
			return JE_FALSE;

		if (pDst == ClipInfo->Work2)
		{
			pSrc = ClipInfo->Work2;
			pDst = ClipInfo->Work1;
		}
		else
		{
			pSrc = ClipInfo->Work1;
			pDst = ClipInfo->Work2;
		}
	}

	ClipInfo->NumDstVerts = NumVerts;
	ClipInfo->DstVerts = pSrc;

	return JE_TRUE;
}

//================================================================================
//	jeFrustum_ClipVertsToPlane
//================================================================================
JETAPI jeBoolean JETCC jeFrustum_ClipVertsToPlane(	const jePlane *pPlane, 
										const jeVec3d *pIn, jeVec3d *pOut,
										int32 NumVerts, int32 *OutVerts)
{
    int32			i, CurIn, NextIn;
    float			CurDot, NextDot, Scale;
    const jeVec3d	*pIn2, *pNext;
	jeVec3d			*pOut2;
	const jeVec3d	*pNormal;

    pIn2 = pIn;
    pOut2= pOut;
	pNormal = &pPlane->Normal;

	CurDot = (pIn->X * pNormal->X) + (pIn->Y * pNormal->Y) + (pIn->Z * pNormal->Z);
    CurIn = (CurDot >= pPlane->Dist);

    for (i=0 ; i<NumVerts ; i++)
    {
		pNext = ((i+1) < NumVerts) ? (pIn2+1): pIn;

        // Keep the current vertex if it's inside the plane
        if (CurIn) 
		{
            pOut2->X = pIn2->X;
            pOut2->Y = pIn2->Y;
            pOut2->Z = pIn2->Z;
			pOut2++;
		}

		NextDot = (pNext->X * pNormal->X) + (pNext->Y * pNormal->Y) + (pNext->Z * pNormal->Z);
		NextIn = (NextDot >= pPlane->Dist);

        // Add a clipped vertex if one end of the current edge is
        // inside the plane and the other is outside
        if (CurIn != NextIn)
        {
			Scale = (pPlane->Dist - CurDot) / (NextDot - CurDot);

            pOut2->X = pIn2->X + (pNext->X - pIn2->X) * Scale;
            pOut2->Y = pIn2->Y + (pNext->Y - pIn2->Y) * Scale;
            pOut2->Z = pIn2->Z + (pNext->Z - pIn2->Z) * Scale;

            pOut2++;
        }

        CurDot = NextDot;
        CurIn = NextIn;
        pIn2++;
    }

    *OutVerts = pOut2 - pOut;

    if (*OutVerts < 3)
        return JE_FALSE;

    return JE_TRUE;
}

//================================================================================
//	jeFrustum_ClipVerts
//================================================================================
JETAPI jeBoolean JETCC jeFrustum_ClipVerts(const jeFrustum *Frustum, jeFrustum_ClipInfo *ClipInfo)
{
	jeVec3d			*pSrc, *pDst;
	int32			i, NumVerts;
	const jePlane	*pPlane;

	assert(Frustum);
	assert(ClipInfo);
	assert(ClipInfo->Work2 != ClipInfo->SrcVerts);

	if (!ClipInfo->ClipFlags)		// Early out if possible
	{
		ClipInfo->NumDstVerts = ClipInfo->NumSrcVerts;
		ClipInfo->DstVerts = (jeVec3d*)ClipInfo->SrcVerts;
		return JE_TRUE;
	}

	pSrc = (jeVec3d*)ClipInfo->SrcVerts;
	pDst = ClipInfo->Work2;

	NumVerts = ClipInfo->NumSrcVerts;

	for (pPlane = Frustum->Planes, i=0; i< Frustum->NumPlanes; i++, pPlane++)
	{
		if (!(ClipInfo->ClipFlags & (1<<i)))
			continue;

		if (!jeFrustum_ClipVertsToPlane(pPlane, pSrc, pDst, NumVerts, &NumVerts))
			return JE_FALSE;

		if (pDst == ClipInfo->Work2)
		{
			pSrc = ClipInfo->Work2;
			pDst = ClipInfo->Work1;
		}
		else
		{
			pSrc = ClipInfo->Work1;
			pDst = ClipInfo->Work2;
		}
	}

	ClipInfo->NumDstVerts = NumVerts;
	ClipInfo->DstVerts = pSrc;

	return JE_TRUE;
}

//================================================================================
//	jeFrustum_PointCollision
//================================================================================
JETAPI jeBoolean JETCC jeFrustum_PointCollision(const jeFrustum *Frustum, const jeVec3d *Point, jeFloat Radius)
{
	int32			i;
	const jePlane	*pPlane;

	for (pPlane = Frustum->Planes, i=0; i< Frustum->NumPlanes; i++, pPlane++)
	{
		if (jePlane_PointDistance(pPlane, Point) < -Radius)
			return JE_FALSE;		// Point behind plane (outside of frustum)
	}	

	return JE_TRUE;
}
