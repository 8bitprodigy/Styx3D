/****************************************************************************************/
/*  MAXMATH.C                                                                           */
/*                                                                                      */
/*  Author: Stephen Balkum	                                                            */
/*  Description:  3DS MAX compatible transformation functions.                          */
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
/*-------------------------------------------------------------------------*\

  It seems that the authors of 3DSMax have decided to use their own flavor
  of matrix math.  Essentially, the translation component of the 4x4
  homogeneous matrix has moved from the 4th column to the 4th row.  This
  affects transforms, muliplies, and inverses.  Appropriate versions are
  given here.

\*-------------------------------------------------------------------------*/

#include <assert.h>
#include <math.h>

#include "xform3d.h"

#define SWAP(a, b) { temp = (a); (a) = (b); (b) = temp; }
#define MATRIX_DIM 4

jeBoolean gaussj(float **a);

// jeXForm3d_Multiply causes the MSDEV50 compiler to choke when "global
// optimizations" is turned on.  The result is that the compiler hangs
// and must be aborted.  Simplifying the expressions did not help.
#pragma optimize("g", off)

void MaxMath_Multiply(const jeXForm3d* M1, const jeXForm3d* M2, jeXForm3d* MProduct)
{
	jeXForm3d M1L;
	jeXForm3d M2L;

	assert( M1       != NULL );
	assert( M2       != NULL );
	assert( MProduct != NULL );
	assert( jeXForm3d_IsOrthogonal(M1) == JE_TRUE );
	assert( jeXForm3d_IsOrthogonal(M2) == JE_TRUE );

	M1L = *M1;
	M2L = *M2;

	MProduct->AX =  M1L.AX * M2L.AX;
	MProduct->AX += M1L.AY * M2L.BX;
	MProduct->AX += M1L.AZ * M2L.CX;

	MProduct->AY =  M1L.AX * M2L.AY;
	MProduct->AY += M1L.AY * M2L.BY;
	MProduct->AY += M1L.AZ * M2L.CY;

	MProduct->AZ =  M1L.AX * M2L.AZ;
	MProduct->AZ += M1L.AY * M2L.BZ;
	MProduct->AZ += M1L.AZ * M2L.CZ;

	MProduct->BX =  M1L.BX * M2L.AX;
	MProduct->BX += M1L.BY * M2L.BX;
	MProduct->BX += M1L.BZ * M2L.CX;

	MProduct->BY =  M1L.BX * M2L.AY;
	MProduct->BY += M1L.BY * M2L.BY;
	MProduct->BY += M1L.BZ * M2L.CY;

	MProduct->BZ =  M1L.BX * M2L.AZ;
	MProduct->BZ += M1L.BY * M2L.BZ;
	MProduct->BZ += M1L.BZ * M2L.CZ;

	MProduct->CX =  M1L.CX * M2L.AX;
	MProduct->CX += M1L.CY * M2L.BX;
	MProduct->CX += M1L.CZ * M2L.CX;

	MProduct->CY =  M1L.CX * M2L.AY;
	MProduct->CY += M1L.CY * M2L.BY;
	MProduct->CY += M1L.CZ * M2L.CY;

	MProduct->CZ =  M1L.CX * M2L.AZ;
	MProduct->CZ += M1L.CY * M2L.BZ;
	MProduct->CZ += M1L.CZ * M2L.CZ;

	MProduct->Translation.X =  M2L.AX * M1L.Translation.X;
	MProduct->Translation.X += M2L.BX * M1L.Translation.Y;
	MProduct->Translation.X += M2L.CX * M1L.Translation.Z;
	MProduct->Translation.X += M2L.Translation.X;

	MProduct->Translation.Y =  M2L.AY * M1L.Translation.X;
	MProduct->Translation.Y += M2L.BY * M1L.Translation.Y;
	MProduct->Translation.Y += M2L.CY * M1L.Translation.Z;
	MProduct->Translation.Y += M2L.Translation.Y;

	MProduct->Translation.Z =  M2L.AZ * M1L.Translation.X;
	MProduct->Translation.Z += M2L.BZ * M1L.Translation.Y;
	MProduct->Translation.Z += M2L.CZ * M1L.Translation.Z;
	MProduct->Translation.Z += M2L.Translation.Z;
	
	assert ( jeXForm3d_IsOrthogonal(MProduct) == JE_TRUE );
}
#pragma optimize("", on)

void MaxMath_Transform(const jeXForm3d* M, const jeVec3d* v, jeVec3d* r)
{
	jeVec3d vv;

	vv = *v;

	r->X = M->AX * vv.X + M->BX * vv.Y + M->CX * vv.Z + M->Translation.X;
	r->Y = M->AY * vv.X + M->BY * vv.Y + M->CY * vv.Z + M->Translation.Y;
	r->Z = M->AZ * vv.X + M->BZ * vv.Y + M->CZ * vv.Z + M->Translation.Z;
}

void MaxMath_GetInverse(const jeXForm3d* A, jeXForm3d* Inv)
{
	float AInv[MATRIX_DIM+1][MATRIX_DIM+1]; // 1 based
	float* pARows[MATRIX_DIM+1]; // 1 based

	AInv[1][1] = A->AX;
	AInv[1][2] = A->AY;
	AInv[1][3] = A->AZ;
	AInv[1][4] = 0.0f;
	AInv[2][1] = A->BX;
	AInv[2][2] = A->BY;
	AInv[2][3] = A->BZ;
	AInv[2][4] = 0.0f;
	AInv[3][1] = A->CX;
	AInv[3][2] = A->CY;
	AInv[3][3] = A->CZ;
	AInv[3][4] = 0.0f;
	AInv[4][1] = A->Translation.X;
	AInv[4][2] = A->Translation.Y;
	AInv[4][3] = A->Translation.Z;
	AInv[4][4] = 1.0f;

	pARows[1] = AInv[1];
	pARows[2] = AInv[2];
	pARows[3] = AInv[3];
	pARows[4] = AInv[4];

	gaussj(pARows);

	Inv->AX = AInv[1][1];
	Inv->AY = AInv[1][2];
	Inv->AZ = AInv[1][3];

	Inv->BX = AInv[2][1];
	Inv->BY = AInv[2][2];
	Inv->BZ = AInv[2][3];

	Inv->CX = AInv[3][1];
	Inv->CY = AInv[3][2];
	Inv->CZ = AInv[3][3];

	Inv->Translation.X = AInv[4][1];
	Inv->Translation.Y = AInv[4][2];
	Inv->Translation.Z = AInv[4][3];
}

void MaxMath_InverseMultiply(const jeXForm3d* A, const jeXForm3d* B, jeXForm3d* M)
{
	float BInv[MATRIX_DIM+1][MATRIX_DIM+1]; // 1 based
	float* pBRows[MATRIX_DIM+1]; // 1 based
	jeXForm3d Product;

	BInv[1][1] = B->AX;
	BInv[1][2] = B->AY;
	BInv[1][3] = B->AZ;
	BInv[1][4] = 0.0f;
	BInv[2][1] = B->BX;
	BInv[2][2] = B->BY;
	BInv[2][3] = B->BZ;
	BInv[2][4] = 0.0f;
	BInv[3][1] = B->CX;
	BInv[3][2] = B->CY;
	BInv[3][3] = B->CZ;
	BInv[3][4] = 0.0f;
	BInv[4][1] = B->Translation.X;
	BInv[4][2] = B->Translation.Y;
	BInv[4][3] = B->Translation.Z;
	BInv[4][4] = 1.0f;

	pBRows[1] = BInv[1];
	pBRows[2] = BInv[2];
	pBRows[3] = BInv[3];
	pBRows[4] = BInv[4];

	gaussj(pBRows);

	Product.AX = A->AX * BInv[1][1];
	Product.AX += A->AY * BInv[2][1];
	Product.AX += A->AZ * BInv[3][1];
	//Product.AX += 0.0f * BInv[4][1];

	Product.AY = A->AX * BInv[1][2];
	Product.AY += A->AY * BInv[2][2];
	Product.AY += A->AZ * BInv[3][2];
	//Product.AY += 0.0f * BInv[4][2];

	Product.AZ = A->AX * BInv[1][3];
	Product.AZ += A->AY * BInv[2][3];
	Product.AZ += A->AZ * BInv[3][3];
	//Product.AZ += 0.0f * BInv[4][3];

	Product.BX = A->BX * BInv[1][1];
	Product.BX += A->BY * BInv[2][1];
	Product.BX += A->BZ * BInv[3][1];
	//Product.BX += 0.0f * BInv[4][1];

	Product.BY = A->BX * BInv[1][2];
	Product.BY += A->BY * BInv[2][2];
	Product.BY += A->BZ * BInv[3][2];
	//Product.BY += 0.0f * BInv[4][2];

	Product.BZ = A->BX * BInv[1][3];
	Product.BZ += A->BY * BInv[2][3];
	Product.BZ += A->BZ * BInv[3][3];
	//Product.BZ += 0.0f * BInv[4][3];

	Product.CX = A->CX * BInv[1][1];
	Product.CX += A->CY * BInv[2][1];
	Product.CX += A->CZ * BInv[3][1];
	//Product.CX += 0.0f * BInv[4][1];

	Product.CY = A->CX * BInv[1][2];
	Product.CY += A->CY * BInv[2][2];
	Product.CY += A->CZ * BInv[3][2];
	//Product.CY += 0.0f * BInv[4][2];

	Product.CZ = A->CX * BInv[1][3];
	Product.CZ += A->CY * BInv[2][3];
	Product.CZ += A->CZ * BInv[3][3];
	//Product.CZ += 0.0f * BInv[4][3];

	Product.Translation.X = A->Translation.X * BInv[1][1];
	Product.Translation.X += A->Translation.Y * BInv[2][1];
	Product.Translation.X += A->Translation.Z * BInv[3][1];
	Product.Translation.X += 1.0f * BInv[4][1];

	Product.Translation.Y = A->Translation.X * BInv[1][2];
	Product.Translation.Y += A->Translation.Y * BInv[2][2];
	Product.Translation.Y += A->Translation.Z * BInv[3][2];
	Product.Translation.Y += 1.0f * BInv[4][2];

	Product.Translation.Z = A->Translation.X * BInv[1][3];
	Product.Translation.Z += A->Translation.Y * BInv[2][3];
	Product.Translation.Z += A->Translation.Z * BInv[3][3];
	Product.Translation.Z += 1.0f * BInv[4][3];

	*M = Product;
}

jeBoolean gaussj(float **a)
{
	int i, j, k, l, ll;
	int icol = 0;
	int irow = 0;
	float big, dum, pivinv, temp;
	int indxc[MATRIX_DIM+1]; // 1 based
	int indxr[MATRIX_DIM+1];
	int ipiv[MATRIX_DIM+1];

	for (j=1;j<=MATRIX_DIM;j++)
		ipiv[j] = 0;

	for (i=1;i<=MATRIX_DIM;i++)
	{
		big = 0.0;
		for (j=1;j<=MATRIX_DIM;j++)
		{
			if (ipiv[j] != 1)
			{
				for (k=1;k<=MATRIX_DIM;k++)
				{
					if (ipiv[k] == 0)
					{
						if (fabs(a[j][k]) >= big)
						{
							big = (float)fabs(a[j][k]);
							irow = j;
							icol = k;
						}
					} 
					else 
					{
						if (ipiv[k] > 1) 
							return(JE_FALSE);
					}
				}
			}
		}
		++(ipiv[icol]);
		if (irow != icol)
		{
			for (l=1;l<=MATRIX_DIM;l++) 
				SWAP(a[irow][l], a[icol][l])
		}
		indxr[i] = irow;
		indxc[i] = icol;
		if (a[icol][icol] == 0.0) 
			return(JE_FALSE);
		pivinv = 1.0f / a[icol][icol];
		a[icol][icol] = 1.0;
		for (l=1;l<=MATRIX_DIM;l++) 
			a[icol][l] *= pivinv;
		for (ll=1;ll<=MATRIX_DIM;ll++)
		{
			if (ll != icol)
			{
				dum = a[ll][icol];
				a[ll][icol] = 0.0;
				for (l=1;l<=MATRIX_DIM;l++) 
					a[ll][l] -= a[icol][l] * dum;
			}
		}
	}
	for (l=MATRIX_DIM;l>=1;l--)
	{
		if (indxr[l] != indxc[l])
			for (k=1;k<=MATRIX_DIM;k++)
				SWAP(a[k][indxr[l]], a[k][indxc[l]]);
	}

	return(JE_TRUE);
}
/* (C) Copr. 1986-92 Numerical Recipes Software ^. */
