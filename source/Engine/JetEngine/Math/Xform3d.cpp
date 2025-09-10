/****************************************************************************************/
/*  XFORM3D.C                                                                           */
/*                                                                                      */
/*  Author:                                                                             */
/*  Description: 3D transform implementation                                            */
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
#include <math.h>

#include "XForm3d.h"
#include "asmXForm3d.h"
#include "cpu.h"

#include "memory.h"
#pragma intrinsic(memcpy)

// Krouer: file to monitor cycle perfs
//#include "iaperf.h"


#ifndef NDEBUG
	static jeBoolean jeXForm3d_MaximalAssertionMode = JE_TRUE;
	#define jeXForm3d_Assert if (jeXForm3d_MaximalAssertionMode) assert

JETAPI 	void JETCC jeXForm3d_SetMaximalAssertionMode( jeBoolean Enable )
	{
		assert( (Enable == JE_TRUE) || (Enable == JE_FALSE) );
		jeXForm3d_MaximalAssertionMode = Enable;
	}
#else
	#define jeXForm3d_Assert(x)
#endif


JETAPI jeBoolean JETCC jeXForm3d_IsValid(const jeXForm3d *M)
	// returns JE_TRUE if M is 'valid'  
	// 'valid' means that M is non NULL, and there are no NAN's in the matrix.
{

	if (M == NULL)
		return JE_FALSE;
	if (jeVec3d_IsValid(&(M->Translation)) == JE_FALSE)
		return JE_FALSE;

	if ((M->AX * M->AX) < 0.0f) 
		return JE_FALSE;
	if ((M->AY * M->AY) < 0.0f) 
		return JE_FALSE;
	if ((M->AZ * M->AZ) < 0.0f) 
		return JE_FALSE;

	if ((M->BX * M->BX) < 0.0f) 
		return JE_FALSE;
	if ((M->BY * M->BY) < 0.0f) 
		return JE_FALSE;
	if ((M->BZ * M->BZ) < 0.0f) 
		return JE_FALSE;
	
	if ((M->CX * M->CX) < 0.0f) 
		return JE_FALSE;
	if ((M->CY * M->CY) < 0.0f) 
		return JE_FALSE;
	if ((M->CZ * M->CZ) < 0.0f) 
		return JE_FALSE;

	return JE_TRUE;
}




JETAPI void JETCC jeXForm3d_SetIdentity(jeXForm3d *M)
	// sets M to an identity matrix (clears it)
{
	assert( M != NULL );			
	
	M->AX = M->BY = M->CZ = 1.0f;
	M->AY = M->AZ = M->BX = M->BZ = M->CX = M->CY = 0.0f;
	M->Translation.X = M->Translation.Y = M->Translation.Z = 0.0f;
	M->Flags = 0;

	jeXForm3d_Assert ( jeXForm3d_IsOrthonormal(M) == JE_TRUE );
}
	
JETAPI void JETCC jeXForm3d_SetXRotation(jeXForm3d *M,jeFloat RadianAngle)
	// sets up a transform that rotates RadianAngle about X axis
{
	jeFloat Cos,Sin;
	assert( M != NULL );
	assert( RadianAngle * RadianAngle >= 0.0f );

	Cos = (jeFloat)cos(RadianAngle);
	Sin = (jeFloat)sin(RadianAngle);
	M->BY =  Cos;
	M->BZ = -Sin;
	M->CY =  Sin;
	M->CZ =  Cos;
	M->AX = 1.0f;
	M->AY = M->AZ = M->BX = M->CX = 0.0f;
	M->Translation.X = M->Translation.Y = M->Translation.Z = 0.0f;
	M->Flags = 0;

	jeXForm3d_Assert ( jeXForm3d_IsOrthonormal(M) == JE_TRUE );
}  
	
JETAPI void JETCC jeXForm3d_SetYRotation(jeXForm3d *M,jeFloat RadianAngle)
	// sets up a transform that rotates RadianAngle about Y axis
{
	jeFloat Cos,Sin;
	assert( M != NULL );
	assert( RadianAngle * RadianAngle >= 0.0f );

	Cos = (jeFloat)cos(RadianAngle);
	Sin = (jeFloat)sin(RadianAngle);
	
	M->AX =  Cos;
	M->AZ =  Sin;
	M->CX = -Sin;
	M->CZ =  Cos;
	M->BY = 1.0f;
	M->AY = M->BX = M->BZ = M->CY = 0.0f;
	M->Translation.X = M->Translation.Y = M->Translation.Z = 0.0f;
	M->Flags = 0;

	jeXForm3d_Assert ( jeXForm3d_IsOrthonormal(M) == JE_TRUE );
}

JETAPI void JETCC jeXForm3d_SetZRotation(jeXForm3d *M,jeFloat RadianAngle)
	// sets up a transform that rotates RadianAngle about Z axis
{
	jeFloat Cos,Sin;
	assert( M != NULL );
	assert( RadianAngle * RadianAngle >= 0.0f );

	Cos = (jeFloat)cos(RadianAngle);
	Sin = (jeFloat)sin(RadianAngle);
	
	M->AX =  Cos;
	M->AY = -Sin;
	M->BX =  Sin;
	M->BY =  Cos;
	M->CZ = 1.0f;
	M->AZ = M->BZ = M->CX = M->CY = 0.0f;
	M->Translation.X = M->Translation.Y = M->Translation.Z = 0.0f;
	M->Flags = 0;

	jeXForm3d_Assert ( jeXForm3d_IsOrthonormal(M) == JE_TRUE );
}

JETAPI void JETCC jeXForm3d_SetTranslation(jeXForm3d *M,jeFloat x, jeFloat y, jeFloat z)
	// sets up a transform that translates x,y,z
{
	assert( M != NULL );

	M->Translation.X = x;
	M->Translation.Y = y;
	M->Translation.Z = z;
	assert( jeVec3d_IsValid(&M->Translation)!=JE_FALSE);

	M->AX = M->BY = M->CZ = 1.0f;
	M->AY = M->AZ = 0.0f;
	M->BX = M->BZ = 0.0f;
	M->CX = M->CY = 0.0f;
	M->Flags = 0;

	jeXForm3d_Assert ( jeXForm3d_IsOrthonormal(M) == JE_TRUE );
}

JETAPI void JETCC jeXForm3d_SetScaling(jeXForm3d *M,jeFloat x, jeFloat y, jeFloat z)
	// sets up a transform that scales by x,y,z
{
	assert( M != NULL );
	assert( x * x >= 0.0f);
	assert( y * y >= 0.0f);
	assert( z * z >= 0.0f);
	assert( x > GEXFORM3D_MINIMUM_SCALE );
	assert( y > GEXFORM3D_MINIMUM_SCALE );
	assert( z > GEXFORM3D_MINIMUM_SCALE );


	M->AX = x;
	M->BY = y;
	M->CZ = z;

	M->AY = M->AZ = 0.0f;
	M->BX = M->BZ = 0.0f;
	M->CX = M->CY = 0.0f;
	M->Translation.X = M->Translation.Y = M->Translation.Z = 0.0f;

	//If any of the scale values are non-equal than the transform is nonorthogonal
	if( x != y  )
		M->Flags = XFORM3D_NONORTHOGONALISOK;	
	else
	if( x != z )
		M->Flags = XFORM3D_NONORTHOGONALISOK;
	else
	{
		M->Flags = 0;
		jeXForm3d_Assert ( jeXForm3d_IsOrthogonal(M) == JE_TRUE );
	}
}

JETAPI void JETCC jeXForm3d_RotateX(jeXForm3d *M,jeFloat RadianAngle)
	// Rotates M by RadianAngle about X axis
{
	jeXForm3d R;
	assert( M != NULL );
	assert( RadianAngle * RadianAngle >= 0.0f );
	jeXForm3d_Assert ( jeXForm3d_IsOrthogonal(M) == JE_TRUE );

	jeXForm3d_SetXRotation(&R,RadianAngle);
	jeXForm3d_Multiply(&R, M, M);
	jeXForm3d_Assert ( jeXForm3d_IsOrthogonal(M) == JE_TRUE );
}

JETAPI void JETCC jeXForm3d_RotateY(jeXForm3d *M,jeFloat RadianAngle)
	// Rotates M by RadianAngle about Y axis
{
	jeXForm3d R;
	assert( M != NULL );
	assert( RadianAngle * RadianAngle >= 0.0f );
	jeXForm3d_Assert ( jeXForm3d_IsOrthogonal(M) == JE_TRUE );

	jeXForm3d_SetYRotation(&R,RadianAngle);
	jeXForm3d_Multiply(&R, M, M);

	jeXForm3d_Assert ( jeXForm3d_IsOrthogonal(M) == JE_TRUE );
}

JETAPI void JETCC jeXForm3d_RotateZ(jeXForm3d *M,jeFloat RadianAngle)
	// Rotates M by RadianAngle about Z axis
{
	jeXForm3d R;
	assert( M != NULL );
	assert( RadianAngle * RadianAngle >= 0.0f );
	jeXForm3d_Assert ( jeXForm3d_IsOrthogonal(M) == JE_TRUE );

	jeXForm3d_SetZRotation(&R,RadianAngle);
	jeXForm3d_Multiply(&R,M,M);

	jeXForm3d_Assert ( jeXForm3d_IsOrthogonal(M) == JE_TRUE );
}

JETAPI void JETCC jeXForm3d_PostRotateX(jeXForm3d *M,jeFloat RadianAngle)
	// Rotates M by RadianAngle about X axis
{
	jeXForm3d R;
	assert( M != NULL );
	assert( RadianAngle * RadianAngle >= 0.0f );
	jeXForm3d_Assert ( jeXForm3d_IsOrthogonal(M) == JE_TRUE );

	jeXForm3d_SetXRotation(&R,RadianAngle);
	jeXForm3d_Multiply(M,&R,M);
	jeXForm3d_Assert ( jeXForm3d_IsOrthogonal(M) == JE_TRUE );
}

JETAPI void JETCC jeXForm3d_PostRotateY(jeXForm3d *M,jeFloat RadianAngle)
	// Rotates M by RadianAngle about Y axis
{
	jeXForm3d R;
	assert( M != NULL );
	assert( RadianAngle * RadianAngle >= 0.0f );
	jeXForm3d_Assert ( jeXForm3d_IsOrthogonal(M) == JE_TRUE );

	jeXForm3d_SetYRotation(&R,RadianAngle);
	jeXForm3d_Multiply(M,&R,M);

	jeXForm3d_Assert ( jeXForm3d_IsOrthogonal(M) == JE_TRUE );
}

JETAPI void JETCC jeXForm3d_PostRotateZ(jeXForm3d *M,jeFloat RadianAngle)
	// Rotates M by RadianAngle about Z axis
{
	jeXForm3d R;
	assert( M != NULL );
	assert( RadianAngle * RadianAngle >= 0.0f );
	jeXForm3d_Assert ( jeXForm3d_IsOrthogonal(M) == JE_TRUE );

	jeXForm3d_SetZRotation(&R,RadianAngle);
	jeXForm3d_Multiply(M,&R,M);

	jeXForm3d_Assert ( jeXForm3d_IsOrthogonal(M) == JE_TRUE );
}

JETAPI void JETCC jeXForm3d_Translate(jeXForm3d *M,jeFloat x, jeFloat y, jeFloat z)
	// Translates M by x,y,z
{
	jeXForm3d T;
	assert( M != NULL );
	jeXForm3d_Assert ( jeXForm3d_IsOrthogonal(M) == JE_TRUE );

	jeXForm3d_SetTranslation(&T,x,y,z);
	jeXForm3d_Multiply(&T, M, M);

	jeXForm3d_Assert ( jeXForm3d_IsOrthogonal(M) == JE_TRUE );
}

JETAPI void JETCC jeXForm3d_Scale(jeXForm3d *M,jeFloat x, jeFloat y, jeFloat z)
	// Scales M by x,y,z
{
	jeXForm3d S;
	assert( M != NULL );
	jeXForm3d_Assert ( jeXForm3d_IsOrthogonal(M) == JE_TRUE );

	jeXForm3d_SetScaling(&S,x,y,z);
	jeXForm3d_Multiply(&S, M, M);
	//If any of the scale values are non-equal than the transform is nonorthogonal
	if( x != y  )
		M->Flags = XFORM3D_NONORTHOGONALISOK;	
	else
	if( x != z )
		M->Flags = XFORM3D_NONORTHOGONALISOK;
	else
	{
		jeXForm3d_Assert ( jeXForm3d_IsOrthogonal(M) == JE_TRUE );
	}

}

JETAPI void JETCC jeXForm3d_Multiply(
	const jeXForm3d *M1, 
	const jeXForm3d *M2, 
	jeXForm3d *MProduct)
	// MProduct = matrix multiply of M1*M2
{
jeXForm3d MProductL;
	assert( M1       != NULL );
	assert( M2       != NULL );
	assert( MProduct != NULL );
	jeXForm3d_Assert ( jeXForm3d_IsOrthogonal(M1) == JE_TRUE );
	jeXForm3d_Assert ( jeXForm3d_IsOrthogonal(M2) == JE_TRUE );

	MProductL.AX = M1->AX * M2->AX + M1->AY * M2->BX + M1->AZ * M2->CX;
	MProductL.AY = M1->AX * M2->AY + M1->AY * M2->BY + M1->AZ * M2->CY;
	MProductL.AZ = M1->AX * M2->AZ + M1->AY * M2->BZ + M1->AZ * M2->CZ;

	MProductL.BX = M1->BX * M2->AX + M1->BY * M2->BX + M1->BZ * M2->CX;
	MProductL.BY = M1->BX * M2->AY + M1->BY * M2->BY + M1->BZ * M2->CY;
	MProductL.BZ = M1->BX * M2->AZ + M1->BY * M2->BZ + M1->BZ * M2->CZ;

	MProductL.CX = M1->CX * M2->AX + M1->CY * M2->BX + M1->CZ * M2->CX;
	MProductL.CY = M1->CX * M2->AY + M1->CY * M2->BY + M1->CZ * M2->CY;
	MProductL.CZ = M1->CX * M2->AZ + M1->CY * M2->BZ + M1->CZ * M2->CZ;

	MProductL.Translation.X =  M1->AX * M2->Translation.X
							 + M1->AY * M2->Translation.Y
							 + M1->AZ * M2->Translation.Z
							 + M1->Translation.X;

	MProductL.Translation.Y =  M1->BX * M2->Translation.X
							 + M1->BY * M2->Translation.Y
							 + M1->BZ * M2->Translation.Z
							 + M1->Translation.Y;

	MProductL.Translation.Z =  M1->CX * M2->Translation.X
							 + M1->CY * M2->Translation.Y
							 + M1->CZ * M2->Translation.Z
							 + M1->Translation.Z;

	MProductL.Flags = ( ( M1->Flags | M2->Flags ) & XFORM3D_NONORTHOGONALISOK );
	
	*MProduct = MProductL;

	jeXForm3d_Assert ( jeXForm3d_IsOrthogonal(MProduct) == JE_TRUE );
}

JETAPI void JETCC jeXForm3d_Transform(
	const jeXForm3d *M,
	const jeVec3d *V, 
	jeVec3d *Result)
	// Result is Matrix M * Vector V:  V Tranformed by M 
{
	jeVec3d VL;
	assert( M != NULL );
	assert( jeVec3d_IsValid(V)!=JE_FALSE);

	assert( Result != NULL );
	jeXForm3d_Assert ( jeXForm3d_IsOrthogonal(M) == JE_TRUE );

	VL = *V;

	Result->X = (VL.X * M->AX) + (VL.Y * M->AY) + (VL.Z * M->AZ) + M->Translation.X;
	Result->Y = (VL.X * M->BX) + (VL.Y * M->BY) + (VL.Z * M->BZ) + M->Translation.Y;
	Result->Z = (VL.X * M->CX) + (VL.Y * M->CY) + (VL.Z * M->CZ) + M->Translation.Z;
	jeXForm3d_Assert( jeVec3d_IsValid(Result)!=JE_FALSE);

}


typedef void (JETCC *XFMVECARRAY)(const jeXForm3d *XForm, const jeVec3d *Source, jeVec3d *Dest, int32 Count);
static	XFMVECARRAY	XFormVecArray	=NULL;	//does this guarantee first time null?

typedef void (JETCC *XFMARRAY)(const jeXForm3d *XForm, const jeVec3d *Source, jeVec3d *Dest, int32 SourceStride, int32 DestStride, int32 Count);
static	XFMARRAY	XFormArray	=NULL;	//does this guarantee first time null?

JETAPI	jeBoolean	JETCC	jeXForm3d_UsingKatmai(void) 
{
	return (XFormVecArray==jeXForm3d_TransformVecArrayKatmai);
}

JETAPI	jeBoolean	JETCC	jeXForm3d_EnableKatmai(jeBoolean useit) 
{
	if(jeCPU_Features & JE_CPU_HAS_KATMAI)
	{
		if(useit)
		{
			XFormVecArray	=jeXForm3d_TransformVecArrayKatmai;
		}
		else
		{
			XFormVecArray	=jeXForm3d_TransformVecArrayX86;
		}
		return	JE_TRUE;
	}

	return	JE_FALSE;
}

//========================================================================================
//	jeXForm3d_TransformVecArray
//	Calls the correct version (if no flags are set goes to x86)
//========================================================================================
JETAPI void JETCC jeXForm3d_TransformVecArray(const jeXForm3d *XForm, 
	const jeVec3d *Source, jeVec3d *Dest, int32 Count)
{

#if 0 // @@
	jeXForm3d_TransformArray(XForm,Source,sizeof(jeVec3d),Dest,sizeof(jeVec3d),Count);
#endif

	if(XFormVecArray)
	{
		XFormVecArray(XForm, Source, Dest, Count);	
	}
	else
	{
		if(jeCPU_Features & JE_CPU_HAS_KATMAI)
		{
			XFormVecArray	=jeXForm3d_TransformVecArrayKatmai;
		}
#if 0	//darn no native 3dnow 
		else if(jeCPU_Features & JE_CPU_HAS_3DNOW)
		{
			XFormVecArray	=jeXForm3d_TransformVecArray3DNow;
		}
#endif
		else
		{
			XFormVecArray	=jeXForm3d_TransformVecArrayX86;
		}
		
		XFormVecArray(XForm, Source, Dest, Count);
	}
}

//========================================================================================
//	jeXForm3d_TransformArray (allows strides)
//	Calls the correct version (if no flags are set goes to x86)
//========================================================================================
JETAPI void JETCC jeXForm3d_TransformArray(const jeXForm3d *XForm,
												   const jeVec3d *Source,
													   int32 SourceStride,
												   jeVec3d *Dest,
													   int32 DestStride,
												   int32 Count)
{
	if(XFormArray)
	{
		XFormArray(XForm, Source, Dest, SourceStride, DestStride, Count);	
	}
	else
	{
		if(jeCPU_Features & JE_CPU_HAS_KATMAI)
		{
			XFormArray	=jeXForm3d_TransformArrayKatmai;
		}
		#pragma message("XForm3d : Get the 3dnow XFormArray integrated!")
#if 0	//darn no native 3dnow 
		else if(jeCPU_Features & JE_CPU_HAS_3DNOW)
		{
			XFormArray	=jeXForm3d_TransformArray3DNow;
		}
#endif
		else
		{
			XFormArray	=jeXForm3d_TransformArrayX86;
		}
		
		XFormArray(XForm, Source, Dest, SourceStride, DestStride, Count);
	}
}

JETAPI void JETCC jeXForm3d_Rotate(
	const jeXForm3d *M,
	const jeVec3d *V, 
	jeVec3d *Result)
	// Result is Matrix M * Vector V:  V Rotated by M (no translation)
{
	jeVec3d VL;
	assert( M != NULL );
	assert( Result != NULL );
	jeXForm3d_Assert ( jeXForm3d_IsOrthogonal(M) == JE_TRUE );
	assert( jeVec3d_IsValid(V)!=JE_FALSE);

	VL = *V;

	Result->X = (VL.X * M->AX) + (VL.Y * M->AY) + (VL.Z * M->AZ);
	Result->Y = (VL.X * M->BX) + (VL.Y * M->BY) + (VL.Z * M->BZ);
	Result->Z = (VL.X * M->CX) + (VL.Y * M->CY) + (VL.Z * M->CZ);
	jeXForm3d_Assert( jeVec3d_IsValid(Result)!=JE_FALSE);
}


JETAPI void JETCC jeXForm3d_GetLeft(const jeXForm3d *M, jeVec3d *Left)
	// Gets a vector that is 'left' in the frame of reference of M (facing -Z)
{
	assert( M     != NULL );
	assert( Left != NULL );
	jeXForm3d_Assert ( jeXForm3d_IsOrthogonal(M) == JE_TRUE );
	
	Left->X = -M->AX;
	Left->Y = -M->BX;
	Left->Z = -M->CX;
	jeXForm3d_Assert( jeVec3d_IsValid(Left)!=JE_FALSE);
}

JETAPI void JETCC jeXForm3d_GetUp(const jeXForm3d *M,    jeVec3d *Up)
	// Gets a vector that is 'up' in the frame of reference of M (facing -Z)
{
	assert( M  != NULL );
	assert( Up != NULL );
	jeXForm3d_Assert ( jeXForm3d_IsOrthogonal(M) == JE_TRUE );
	
	Up->X = M->AY;
	Up->Y = M->BY;
	Up->Z = M->CY;
	jeXForm3d_Assert( jeVec3d_IsValid(Up)!=JE_FALSE);
}

JETAPI void JETCC jeXForm3d_GetIn(const jeXForm3d *M,  jeVec3d *In)
	// Gets a vector that is 'in' in the frame of reference of M (facing -Z)
{
	assert( M    != NULL );
	assert( In != NULL );
	jeXForm3d_Assert ( jeXForm3d_IsOrthogonal(M) == JE_TRUE );

	In->X = -M->AZ;
	In->Y = -M->BZ;
	In->Z = -M->CZ;
	jeXForm3d_Assert( jeVec3d_IsValid(In)!=JE_FALSE);
}

// KROUER : start of inverse matrix replace
#define _USE_OLD_INVERSE 0	// set it to 1 to use Gaussian method
#if _USE_OLD_INVERSE		// old inverse matrice using Gaussian and 3x3 matrice
typedef struct
{
	float x[3][3];
}	Matrix33;

void Matrix33_Copy(const Matrix33* m, Matrix33* c)
{
//	int i, j;

	assert(m != NULL);
	assert(c != NULL);

/*  // Krouer: optimisation
	for (i = 0; i < 3; i++)
		for (j = 0; j < 3; j++)
			c->x[i][j] = m->x[i][j];
*/
	memcpy(c->x, m->x, 9 * sizeof(float));
}

void Matrix33_SetIdentity(Matrix33* m)
{
//	int i, j;

	assert(m != NULL);

/*  // Krouer: optimisation
	for (i = 0; i < 3; i++)
		for (j = 0; j < 3; j++)
		{
			if (i == j) 
				m->x[i][j] = 1.f;
			else m->x[i][j] = 0.f;
		}
*/
	m->x[0][0] = m->x[1][1] = m->x[2][2] = 1.f;
	m->x[0][1] = m->x[0][2] = 0.f;
	m->x[1][0] = m->x[1][2] = 0.f;
	m->x[2][0] = m->x[2][1] = 0.f;
}

void Matrix33_SwapRows(Matrix33* m,int r1,int r2)
{
int i;
	for(i=0;i<3;i++)
	{
	float temp;
		temp		= m->x[r1][i];
		m->x[r1][i] = m->x[r2][i];
		m->x[r2][i] = temp;
	}
}

void Matrix33_GetInverse(const Matrix33* m, Matrix33* inv)
{
	int i, j, k;
	Matrix33 copy;

	assert(m != NULL);
	assert(inv != NULL);

	Matrix33_Copy(m, &copy);
	Matrix33_SetIdentity(inv);

	for (i = 0; i < 3; i++)
	{
	float bigv;
		// first find the row with the largest coefficient
		k = i;
		bigv = copy.x[i][i];
		bigv = JE_ABS(bigv);
		for(j = i+1;j<3;j++)
		{
		float v;
			v = copy.x[j][i];
			if ( JE_ABS(v) > JE_ABS(bigv) )
			{
				k = j;
				bigv = v;
			}
		}

		// now row k has the largest value (bigv) in column i
		if ( k != i )
		{
			Matrix33_SwapRows(&copy,i,k);
			Matrix33_SwapRows(inv,i,k);
		}

		assert(JE_ABS(bigv) >= 1e-5);

		for (j = 0; j < 3; j++)
		{
			inv->x[i][j] /= bigv;
			copy.x[i][j] /= bigv;
		}

		for (j = 0; j < 3; j++)
		{
			if (j != i)
			{
				float mulby = copy.x[j][i];
				if ( mulby != 0.0f)
				{	
					for (k = 0; k < 3; k++)
					{
						copy.x[j][k] -= mulby * copy.x[i][k];
						inv->x[j][k] -= mulby * inv->x[i][k];
					}
				}
			}
		}
	}
}

void Matrix33_ExtractFromXForm3d(Matrix33* m, const jeXForm3d* xform)
{
	assert(xform != NULL);
	assert(m != NULL);

	m->x[0][0] = xform->AX; m->x[0][1] = xform->AY; m->x[0][2] = xform->AZ;
	m->x[1][0] = xform->BX; m->x[1][1] = xform->BY; m->x[1][2] = xform->BZ;
	m->x[2][0] = xform->CX; m->x[2][1] = xform->CY; m->x[2][2] = xform->CZ;
}

void jeXForm3d_ExtractFromMatrix33(jeXForm3d* xform, const Matrix33* m)
{
	assert(xform != NULL);
	assert(m != NULL);

	jeVec3d_Clear(&xform->Translation);

	xform->AX = m->x[0][0]; xform->AY = m->x[0][1]; xform->AZ = m->x[0][2];
	xform->BX = m->x[1][0]; xform->BY = m->x[1][1]; xform->BZ = m->x[1][2];
	xform->CX = m->x[2][0]; xform->CY = m->x[2][1]; xform->CZ = m->x[2][2];
}

JETAPI void JETCC jeXForm3d_GetInverse(const jeXForm3d *M, jeXForm3d *MInv)
{
	Matrix33	Matrix, InvMatrix;

	MInv->Flags = M->Flags;
	jeXForm3d_Assert ( jeXForm3d_IsOrthogonal(M) == JE_TRUE );

	Matrix33_ExtractFromXForm3d(&Matrix, M);
	Matrix33_GetInverse(&Matrix, &InvMatrix);

	jeXForm3d_ExtractFromMatrix33(MInv, &InvMatrix);

	{
		jeXForm3d T;
		jeXForm3d_SetTranslation(&T,-M->Translation.X,-M->Translation.Y,-M->Translation.Z);
		jeXForm3d_Multiply(MInv,&T,MInv);
	}
}
#else // New inverse matrice algo using Cramer method

JETAPI void JETCC jeXForm3d_GetInverse(const jeXForm3d *M, jeXForm3d *MInv)
{
	float det;
	float cofactors[9];

	// initialize the cofactors
	cofactors[0] = M->BY*M->CZ - M->BZ*M->CY;
	cofactors[1] = M->AZ*M->CY - M->AY*M->CZ;
	cofactors[2] = M->AY*M->BZ - M->AZ*M->BY;
	cofactors[3] = M->BZ*M->CX - M->BX*M->CZ;
	cofactors[4] = M->AX*M->CZ - M->AZ*M->CX;
	cofactors[5] = M->AZ*M->BX - M->AX*M->BZ;
	cofactors[6] = M->BX*M->CY - M->BY*M->CX;
	cofactors[7] = M->AY*M->CX - M->AX*M->CY;
	cofactors[8] = M->AX*M->BY - M->AY*M->BX;

	det = M->AX*cofactors[0] + M->BX*cofactors[1] + M->CX*cofactors[2];
	det = 1.f / det;

	MInv->AX = cofactors[0]*det;
	MInv->AY = cofactors[1]*det;
	MInv->AZ = cofactors[2]*det;

	MInv->BX = cofactors[3]*det;
	MInv->BY = cofactors[4]*det;
	MInv->BZ = cofactors[5]*det;

	MInv->CX = cofactors[6]*det;
	MInv->CY = cofactors[7]*det;
	MInv->CZ = cofactors[8]*det;

	MInv->Translation.X = 0;//-M->Translation.X;
	MInv->Translation.Y = 0;//-M->Translation.Y;
	MInv->Translation.Z = 0;//-M->Translation.Z;

	{
		jeXForm3d T;
		jeXForm3d_SetTranslation(&T,-M->Translation.X,-M->Translation.Y,-M->Translation.Z);
		jeXForm3d_Multiply(MInv,&T,MInv);
	}
}
#endif // End Inverse matrix by KROUER

JETAPI void JETCC jeXForm3d_GetTranspose(const jeXForm3d *M, jeXForm3d *MInv)
{
	jeXForm3d M1;
	jeXForm3d_Assert ( jeXForm3d_IsOrthogonal(M) == JE_TRUE );

	M1 = *M;

	MInv->AX = M1.AX;
	MInv->AY = M1.BX;
	MInv->AZ = M1.CX;

	MInv->BX = M1.AY;
	MInv->BY = M1.BY;
	MInv->BZ = M1.CY;

	MInv->CX = M1.AZ;
	MInv->CY = M1.BZ;
	MInv->CZ = M1.CZ;

	MInv->Translation.X = 0.0f;
	MInv->Translation.Y = 0.0f;
	MInv->Translation.Z = 0.0f;

	MInv->Flags = M->Flags;

/*****

this is the same as:

	CXForm->Translation = MXForm->Translation;

	jeVec3d_Inverse(&CXForm->Translation);

	// Rotate the translation in the new camera matrix
	jeXForm3d_Rotate(CXForm, &CXForm->Translation, &CXForm->Translation);


******/
	{
		jeXForm3d T;
		jeXForm3d_SetTranslation(&T,-M1.Translation.X,-M1.Translation.Y,-M1.Translation.Z);
		jeXForm3d_Multiply(MInv,&T,MInv);
	}

	jeXForm3d_Assert ( jeXForm3d_IsOrthogonal(MInv) == JE_TRUE );
}

JETAPI void JETCC jeXForm3d_TransposeTransform(
	const jeXForm3d *M, 
	const jeVec3d *V, 
	jeVec3d *Result)
	// applies the Transpose transform of M to V.  Result = (M^T) * V
{
	jeVec3d V1;

	assert( M      != NULL );
	assert( jeVec3d_IsValid(V)!=JE_FALSE);

	assert( Result != NULL );
	jeXForm3d_Assert ( jeXForm3d_IsOrthogonal(M) == JE_TRUE );

	V1.X = V->X - M->Translation.X;
	V1.Y = V->Y - M->Translation.Y;
	V1.Z = V->Z - M->Translation.Z;

	Result->X = (V1.X * M->AX) + (V1.Y * M->BX) + (V1.Z * M->CX);
	Result->Y = (V1.X * M->AY) + (V1.Y * M->BY) + (V1.Z * M->CY);
	Result->Z = (V1.X * M->AZ) + (V1.Y * M->BZ) + (V1.Z * M->CZ);
	jeXForm3d_Assert( jeVec3d_IsValid(Result)!=JE_FALSE);
}


JETAPI void JETCC jeXForm3d_Copy(
	const jeXForm3d *Src, 
	jeXForm3d *Dst)
{	
	assert( Src != NULL );
	assert( Dst != NULL );
	jeXForm3d_Assert ( jeXForm3d_IsOrthogonal(Src) == JE_TRUE );

	*Dst = *Src;
}    

JETAPI void JETCC jeXForm3d_GetEulerAngles(const jeXForm3d *M, jeVec3d *Angles)
	// order of angles z,y,x
{
	jeFloat AZ;
	assert( M      != NULL );
	assert( Angles != NULL );

	jeXForm3d_Assert ( jeXForm3d_IsOrthonormal(M) == JE_TRUE );
	
	//ack.  due to floating point error, the value can drift away from 1.0 a bit
	//      this will clamp it.  The _IsOrthonormal test will pass because it allows
	//      for a tolerance.

	AZ = M->AZ;
	if (AZ > 1.0f) 
		AZ = 1.0f;
	if (AZ < -1.0f) 
		AZ = -1.0f;

	Angles->Y = -(jeFloat)asin(-AZ);

	if ( cos(Angles->Y) != 0 )
	{
		Angles->X = -(jeFloat)atan2(M->BZ, M->CZ);
		Angles->Z = -(jeFloat)atan2(M->AY, M->AX);
	}
	else
	{
		Angles->X = -(jeFloat)atan2(M->BX, M->BY);
		Angles->Z = 0.0f;
	}
	assert( jeVec3d_IsValid(Angles)!=JE_FALSE);
}


JETAPI void JETCC jeXForm3d_SetEulerAngles(jeXForm3d *M, const jeVec3d *Angles)
	// order of angles z,y,x
{
	jeXForm3d XM, YM, ZM;							            

	assert( M      != NULL );
	assert( jeVec3d_IsValid(Angles)!=JE_FALSE);
	
	jeXForm3d_SetXRotation(&XM,Angles->X);
	jeXForm3d_SetYRotation(&YM,Angles->Y);
	jeXForm3d_SetZRotation(&ZM,Angles->Z);
	
	jeXForm3d_Multiply(&XM, &YM, M);
	jeXForm3d_Multiply(M, &ZM, M);
	

	jeXForm3d_Assert ( jeXForm3d_IsOrthonormal(M) == JE_TRUE );

}

JETAPI jeBoolean JETCC jeXForm3d_IsOrthonormal(const jeXForm3d *M)
	// returns JE_TRUE if M is orthonormal 
	// (if the rows and columns are all normalized (transform has no scaling or shearing)
	// and is orthogonal (row1 cross row2 = row3 & col1 cross col2 = col3)
{
#define ORTHONORMAL_TOLERANCE ((jeFloat)(0.001f))
	jeVec3d Col1,Col2,Col3;
	jeVec3d Col1CrossCol2;
	jeBoolean IsOrthonormal;
	assert( M != NULL );

#pragma message ("This test for non-orthogonal is not quite correct")
	if	(M->Flags & XFORM3D_NONORTHOGONALISOK)
		return JE_TRUE;

	jeXForm3d_Assert ( jeXForm3d_IsValid(M) == JE_TRUE );

	Col1.X = M->AX;
	Col1.Y = M->BX;
	Col1.Z = M->CX;
	
	Col2.X = M->AY;
	Col2.Y = M->BY;
	Col2.Z = M->CY;

	Col3.X = M->AZ;
	Col3.Y = M->BZ;
	Col3.Z = M->CZ;

	jeVec3d_CrossProduct(&Col1,&Col2,&Col1CrossCol2);

	IsOrthonormal = jeVec3d_Compare(&Col1CrossCol2,&Col3,ORTHONORMAL_TOLERANCE);
	if (IsOrthonormal == JE_FALSE)
		{
			jeVec3d_Inverse(&Col3);
			IsOrthonormal = jeVec3d_Compare(&Col1CrossCol2,&Col3,ORTHONORMAL_TOLERANCE);
		}

	if ( jeVec3d_IsValid(&(M->Translation)) ==JE_FALSE)
		return JE_FALSE;

	return IsOrthonormal;
}


JETAPI void JETCC jeXForm3d_Orthonormalize(jeXForm3d *M)
	// essentially removes scaling (or other distortions) from 
	// an orthogonal (or nearly orthogonal) matrix 
{
	jeVec3d Col1,Col2,Col3;
	assert( M != NULL );
	jeXForm3d_Assert ( jeXForm3d_IsValid(M) == JE_TRUE );

#pragma message ("This test for non-orthogonal is not quite correct")
	if	(M->Flags & XFORM3D_NONORTHOGONALISOK)
		return;

	// Normalize Col 1 & 2
	Col1.X = M->AX;
	Col1.Y = M->BX;
	Col1.Z = M->CX;
	jeVec3d_Normalize(&Col1);
	M->AX = Col1.X;
	M->BX = Col1.Y;
	M->CX = Col1.Z;
	
	Col2.X = M->AY;
	Col2.Y = M->BY;
	Col2.Z = M->CY;
	jeVec3d_Normalize(&Col2);
	M->AY = Col2.X;
	M->BY = Col2.Y;
	M->CY = Col2.Z;

	// Cross Col 1 & 2 to get 3
	jeVec3d_CrossProduct(&Col1,&Col2,&Col3);

	M->AZ = Col3.X;
	M->BZ = Col3.Y;
	M->CZ = Col3.Z;

	// Cross Col 3 and 1 to get 2
	jeVec3d_CrossProduct(&Col3,&Col1,&Col2);

	M->AY = Col2.X;
	M->BY = Col2.Y;
	M->CY = Col2.Z;

	jeXForm3d_Assert ( jeXForm3d_IsOrthonormal(M) == JE_TRUE );
}




JETAPI jeBoolean JETCC jeXForm3d_IsOrthogonal(const jeXForm3d *M)
	// returns JE_TRUE if M is orthogonal
	// (row1 cross row2 = row3 & col1 cross col2 = col3)
{
#define ORTHOGONAL_TOLERANCE ((jeFloat)(0.001f))
	jeVec3d Col1,Col2,Col3;
	jeVec3d Col1CrossCol2;
	jeBoolean IsOrthogonal;
	assert( M != NULL );
	jeXForm3d_Assert ( jeXForm3d_IsValid(M) == JE_TRUE );

#pragma message ("This test for non-orthogonal is not quite correct")
	if	(M->Flags & XFORM3D_NONORTHOGONALISOK)
		return JE_TRUE;

	//return JE_TRUE;

	Col1.X = M->AX;
	Col1.Y = M->BX;
	Col1.Z = M->CX;
	//jeVec3d_Normalize(&Col1);
	
	Col2.X = M->AY;
	Col2.Y = M->BY;
	Col2.Z = M->CY;
	//jeVec3d_Normalize(&Col2);
	
	Col3.X = M->AZ;
	Col3.Y = M->BZ;
	Col3.Z = M->CZ;
	jeVec3d_Normalize(&Col3);
	
	jeVec3d_CrossProduct(&Col1,&Col2,&Col1CrossCol2);
		
	jeVec3d_Normalize(&Col1CrossCol2);
	
	IsOrthogonal = jeVec3d_Compare(&Col1CrossCol2,&Col3,ORTHOGONAL_TOLERANCE);
	if (IsOrthogonal == JE_FALSE)
		{
			jeVec3d_Inverse(&Col3);
			IsOrthogonal = jeVec3d_Compare(&Col1CrossCol2,&Col3,ORTHOGONAL_TOLERANCE);
		}

	if ( jeVec3d_IsValid(&(M->Translation)) ==JE_FALSE)
		return JE_FALSE;

	return IsOrthogonal;
}

JETAPI void JETCC jeXForm3d_LookAt (jeXForm3d* M, const jeVec3d *Target)
{
	/*
	jeVec3d Pos, InVect;
	jeVec3d			LVect = {1.0f,0.0f,0.0f}, UpVect = {0.0f,1.0f,0.0f};

	jeVec3d_Subtract(Target, &M->Translation, &InVect);

	Pos = M->Translation;

	if (jeVec3d_Length(&InVect) == 0.0f)
	{
		jeXForm3d_SetIdentity(M);
	}
	else
	{
		jeVec3d_Normalize(&InVect);
		if ((1.0f - fabs(jeVec3d_DotProduct(&InVect, &UpVect))) < 0.01f)
		{
			jeVec3d_CrossProduct(&LVect, &InVect, &UpVect);
			jeVec3d_Normalize(&UpVect);
			jeVec3d_CrossProduct(&UpVect, &InVect, &LVect);
			jeXForm3d_SetFromLeftUpIn(M, &LVect, &UpVect, &InVect);
		}
		else
		{
			jeVec3d_CrossProduct(&UpVect, &InVect, &LVect);
			jeVec3d_Normalize(&LVect);
			jeVec3d_CrossProduct(&InVect, &LVect, &UpVect);
			jeXForm3d_SetFromLeftUpIn(M, &LVect, &UpVect, &InVect);
		}
	}

	M->Translation = Pos;
*/
	
	jeVec3d		vecNewIn, vecNewUp, vecNewLeft;
	jeVec3d_Subtract(&M->Translation, Target, &vecNewIn);
	jeVec3d_Normalize(&vecNewIn);

	vecNewUp.X = 0.0f;
	vecNewUp.Y = 1.0f;
	vecNewUp.Z = 0.0f;

	jeVec3d_CrossProduct(&vecNewUp, &vecNewIn, &vecNewLeft);
	jeVec3d_Normalize(&vecNewLeft);

	M->AX = vecNewLeft.X;
	M->BX = vecNewLeft.Y;
	M->CX = vecNewLeft.Z;

	M->AY = vecNewUp.X;
	M->BY = vecNewUp.Y;
	M->CY = vecNewUp.Z;

	M->AZ = vecNewIn.X;
	M->BZ = vecNewIn.Y;
	M->CZ = vecNewIn.Z;

	if (!jeXForm3d_IsOrthonormal(M))
		jeXForm3d_Orthonormalize(M);

	jeVec3d_Subtract(&M->Translation, Target, &vecNewIn);
	jeVec3d_Normalize(&vecNewIn);

	jeVec3d_CrossProduct(&vecNewIn, &vecNewLeft,&vecNewUp);
	jeVec3d_Normalize(&vecNewUp);

	M->AX = vecNewLeft.X;
	M->BX = vecNewLeft.Y;
	M->CX = vecNewLeft.Z;

	M->AY = vecNewUp.X;
	M->BY = vecNewUp.Y;
	M->CY = vecNewUp.Z;

	M->AZ = vecNewIn.X;
	M->BZ = vecNewIn.Y;
	M->CZ = vecNewIn.Z;

	if (!jeXForm3d_IsOrthonormal(M))
		jeXForm3d_Orthonormalize(M);

}

JETAPI void JETCC jeXForm3d_RotateAboutLeft (jeXForm3d* M, jeFloat RadianAngle)
{
     jeVec3d Angles, Pos;

     Pos = M->Translation;
     jeXForm3d_Translate (M, -Pos.X, -Pos.Y, -Pos.Z); 
     jeXForm3d_GetEulerAngles (M, &Angles); 
     jeXForm3d_SetIdentity (M); 
     jeXForm3d_RotateX (M, -RadianAngle); 
     jeXForm3d_RotateZ (M, Angles.Z); 
     jeXForm3d_RotateY (M, Angles.Y);
     jeXForm3d_RotateX (M, Angles.X);

     jeXForm3d_Translate (M, Pos.X, Pos.Y, Pos.Z);
}



JETAPI void JETCC jeXForm3d_SetFromLeftUpIn(
	jeXForm3d *M,
	const jeVec3d *Left, 
	const jeVec3d *Up, 
	const jeVec3d *In)
{
	assert(M);
	assert(Left);
	assert(Up);
	assert(In);
	jeXForm3d_Assert(jeVec3d_IsNormalized(Left));
	jeXForm3d_Assert(jeVec3d_IsNormalized(Up));
	jeXForm3d_Assert(jeVec3d_IsNormalized(In));

	M->AX = -Left->X;
	M->BX = -Left->Y;
	M->CX = -Left->Z;
	M->AY =  Up->X;
	M->BY =  Up->Y;
	M->CY =  Up->Z;
	M->AZ = -In->X;
	M->BZ = -In->Y;
	M->CZ = -In->Z;

	jeVec3d_Clear(&M->Translation);
	M->Flags = 0;

	jeXForm3d_Assert ( jeXForm3d_IsOrthonormal(M) == JE_TRUE );
}

JETAPI void JETCC jeXForm3d_Mirror(
	const		jeXForm3d *Source, 
	const		jeVec3d *PlaneNormal, 
	float		PlaneDist, 
	jeXForm3d	*Dest)
{
	float			Dist;
	jeVec3d			In, Left, Up;
	jeXForm3d		Original;
	jeVec3d			MirrorTranslation;

	jeXForm3d_Assert ( jeXForm3d_IsOrthogonal(Source) == JE_TRUE );
	assert( PlaneNormal != NULL );
	assert( Dest        != NULL );

	jeXForm3d_Copy(Source, &Original);

	// Mirror the translation portion of the matrix
	Dist = jeVec3d_DotProduct(&Original.Translation, PlaneNormal) - PlaneDist;
	jeVec3d_AddScaled(&Original.Translation, PlaneNormal, -Dist*2.0f, &MirrorTranslation);

	// Mirror the Rotational portion of the xform first
	jeXForm3d_GetIn(&Original, &In);
	jeVec3d_Add(&Original.Translation, &In, &In);
	Dist = jeVec3d_DotProduct(&In, PlaneNormal) - PlaneDist;
	jeVec3d_AddScaled(&In, PlaneNormal, -Dist*2.0f, &In);
	jeVec3d_Subtract(&In, &MirrorTranslation, &In);
	jeVec3d_Normalize(&In);

	jeXForm3d_GetLeft(&Original, &Left);
	jeVec3d_Add(&Original.Translation, &Left, &Left);
	Dist = jeVec3d_DotProduct(&Left, PlaneNormal) - PlaneDist;
	jeVec3d_AddScaled(&Left, PlaneNormal, -Dist*2.0f, &Left);
	jeVec3d_Subtract(&Left, &MirrorTranslation, &Left);
	jeVec3d_Normalize(&Left);

	jeXForm3d_GetUp(&Original, &Up);
	jeVec3d_Add(&Original.Translation, &Up, &Up);
	Dist = jeVec3d_DotProduct(&Up, PlaneNormal) - PlaneDist;
	jeVec3d_AddScaled(&Up, PlaneNormal, -Dist*2.0f, &Up);
	jeVec3d_Subtract(&Up, &MirrorTranslation, &Up);
	jeVec3d_Normalize(&Up);

	jeXForm3d_SetFromLeftUpIn(Dest, &Left, &Up, &In);

	// Must set the mirror translation here since jeXForm3d_SetFromLeftUpIn cleared the translation portion
	jeVec3d_Set(&Dest->Translation, MirrorTranslation.X, MirrorTranslation.Y, MirrorTranslation.Z);

	jeXForm3d_Assert ( jeXForm3d_IsOrthogonal(Dest) == JE_TRUE );
}
