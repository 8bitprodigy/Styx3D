/*!
  @file jeRay.cpp

  @author Anthony Rufrano (paradoxnj)
  @brief Ray casting code
                                                                   
  The contents of this file are subject to the Jet3D Public License
  Version 1.02 (the "License"); you may not use this file except in
  compliance with the License. You may obtain a copy of the License at
  http://www.jet3d.com
                                                                     
  Software distributed under the License is distributed on an "AS IS"
  basis, WITHOUT WARRANTY OF ANY KIND, either express or implied.  See
  the License for the specific language governing rights and limitations
  under the License.
                                 
  The Original Code is Jet3D, released December 12, 1999.
  Copyright (C) 1996-1999 Eclipse Entertainment, L.L.C. All Rights Reserved
*/

#include <assert.h>
#include <math.h>
#include "jeRay.h"

JETAPI void JETCC jeRay_Set(jeRay *Ray, jeVec3d *Origin, jeVec3d *Dir)
{
	assert(Ray != NULL);
	assert(Origin != NULL);
	assert(Dir != NULL);

	jeVec3d_Copy(Origin, &Ray->Origin);
	jeVec3d_Copy(Dir, &Ray->Direction);
}

JETAPI void JETCC jeRay_Get(const jeRay *Ray, jeVec3d *Origin, jeVec3d *Dir)
{
	assert(Ray != NULL);
	assert(Origin != NULL);
	assert(Dir != NULL);

	jeVec3d_Copy(&Ray->Origin, Origin);
	jeVec3d_Copy(&Ray->Direction, Dir);
}

JETAPI jeBoolean JETCC jeRay_IntersectsWithTriangle(const jeRay *Ray, const jeVec3d *V1, const jeVec3d *V2, const jeVec3d *V3, jeBoolean Cull, float *T, jeVec3d *Impact)
{
	jeVec3d					pvec, tvec, qvec;
	jeVec3d					edge1, edge2;
	float					det, u, v;

	jeVec3d_Subtract(V2, V1, &edge1);
	jeVec3d_Subtract(V3, V1, &edge2);

	jeVec3d_CrossProduct(&Ray->Direction, &edge2, &pvec);

	det = jeVec3d_DotProduct(&edge1, &pvec);
	if (Cull && (det < 0.0001f))
		return JE_FALSE;
	else if ((det < 0.0001f) && (det > -0.0001f))
		return JE_FALSE;

	//Calc distance to plane.  Less than 0 means the ray is behind the plane.
	jeVec3d_Subtract(&Ray->Origin, V1, &tvec);
	u = jeVec3d_DotProduct(&tvec, &pvec);
	if (u < 0.0f || u > det)
		return JE_FALSE;

	jeVec3d_CrossProduct(&tvec, &edge1, &qvec);
	v = jeVec3d_DotProduct(&Ray->Origin, &qvec);
	if (v < 0.0f || (u + v) > det)
		return JE_FALSE;

	if (T != NULL)
	{
		float				invDet;

		*T = jeVec3d_DotProduct(&edge2, &qvec);
		invDet = 1.0f / det;
		*T *= invDet;
	}

	return JE_TRUE;
}

JETAPI jeBoolean JETCC jeRay_IntersectsWithPlane(const jeRay *Ray, const jePlane *Plane, jeBoolean Cull, float *T, jeVec3d *Impact)
{
	float						Vd, Vo, _t;

	Vd = jeVec3d_DotProduct(&Plane->Normal, &Ray->Direction);
	if (fabs(Vd) < 0.00001f)
		return JE_FALSE;

	if (Cull && (Vd > 0.0f))
		return JE_FALSE;

	Vo = -((jeVec3d_DotProduct(&Plane->Normal, &Ray->Origin) + Plane->Dist));
	_t = Vo / Vd;
	if (_t < 0.0f)
		return JE_FALSE;

	if (Impact)
		jeVec3d_AddScaled(&Ray->Origin, &Ray->Direction, _t, Impact);

	if (T)
		*T = _t;

	return JE_TRUE;
}

JETAPI jeBoolean JETCC jeRay_InterestsWithExtBox(const jeRay *Ray, const jeExtBox *ExtBox, jeBoolean Cull, float *T, jeVec3d *Impact)
{
	jeVec3d					MaxT;
	jeBoolean				Inside = JE_TRUE;
	float					plane;

	jeVec3d_Set(&MaxT, -1.0f, -1.0f, -1.0f);

	if (Ray->Origin.X < ExtBox->Min.X)
	{
		Impact->X = ExtBox->Min.X;
		Inside = JE_FALSE;
		if (Ray->Direction.X != 0.0f)
			MaxT.X = (ExtBox->Min.X - Ray->Origin.X) / Ray->Direction.X;
	}
	else if (Ray->Origin.X > ExtBox->Max.X)
	{
		Impact->X = ExtBox->Max.X;
		Inside = JE_FALSE;
		if (Ray->Direction.X != 0.0f)
			MaxT.X = (ExtBox->Max.X - Ray->Origin.X) / Ray->Direction.X;
	}
	
	if (Ray->Origin.Y < ExtBox->Min.Y)
	{
		Impact->Y = ExtBox->Min.Y;
		Inside = JE_FALSE;
		if (Ray->Direction.Y != 0.0f)
			MaxT.Y = (ExtBox->Min.Y - Ray->Origin.Y) / Ray->Direction.Y;
	}
	else if (Ray->Origin.Y > ExtBox->Max.Y)
	{
		Impact->Y = ExtBox->Max.Y;
		Inside = JE_FALSE;
		if (Ray->Direction.Y != 0.0f)
			MaxT.Y = (ExtBox->Max.Y - Ray->Origin.Y) / Ray->Direction.Y;
	}

	if (Ray->Origin.Z < ExtBox->Min.Z)
	{
		Impact->Z = ExtBox->Min.Z;
		Inside = JE_FALSE;
		if (Ray->Direction.Z != 0.0f)
			MaxT.Z = (ExtBox->Min.Z - Ray->Origin.Z) / Ray->Direction.Z;
	}
	else if (Ray->Origin.Z > ExtBox->Max.Z)
	{
		Impact->Z = ExtBox->Max.Z;
		Inside = JE_FALSE;
		if (Ray->Direction.Z != 0.0f)
			MaxT.Z = (ExtBox->Max.Z - Ray->Origin.Z) / Ray->Direction.Z;
	}

	if (Inside)
	{
		jeVec3d_Copy(&Ray->Origin, Impact);
		return JE_TRUE;
	}

	plane = MaxT.X;

	if (MaxT.Y > MaxT.X)
		plane = MaxT.Y;
	
	if (MaxT.Z > MaxT.X)
		plane = MaxT.Z;

	if (plane < 0.0f)
		return JE_FALSE;

	if (plane == MaxT.X)
	{
		Impact->X = Ray->Origin.X + MaxT.X * Ray->Direction.X;
		if ((Impact->X < ExtBox->Min.X - 0.00001f) || (Impact->X < ExtBox->Max.X + 0.00001f))
			return JE_FALSE;
	}
	else if (plane == MaxT.Y)
	{
		Impact->Y = Ray->Origin.Y + MaxT.Y * Ray->Direction.Y;
		if ((Impact->Y < ExtBox->Min.Y - 0.00001f) || (Impact->Y < ExtBox->Max.Y + 0.00001f))
			return JE_FALSE;
	}
	else if (plane == MaxT.Z)
	{
		Impact->Z = Ray->Origin.Z + MaxT.Z * Ray->Direction.Z;
		if ((Impact->Z < ExtBox->Min.Z - 0.00001f) || (Impact->Z < ExtBox->Max.Z + 0.00001f))
			return JE_FALSE;
	}

	return JE_TRUE;
}
