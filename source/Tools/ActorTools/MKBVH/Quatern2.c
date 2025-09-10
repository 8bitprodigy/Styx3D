/****************************************************************************************/
/*  QUATERN2.C	                                                                            */
/*                                                                                      */
/*  Author:             	                                                            */
/*  Description:                                                						*/
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
//---------------------------------------------------------------------------
// This source is lifted straight from Ken Shoemake's article in 
// Graphics Gems IV, "Euler Angle Conversion."
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
// QUATTYPE.H
//---------------------------------------------------------------------------


/**** QuatType.h - Basic type declarations ****/
#ifndef _H_QuatTypes
#define _H_QuatTypes
/*** Definitions ***/
typedef struct {float x, y, z, w;} Quat; /* Quaternion */
enum QuatPart {X, Y, Z, W};
typedef float HMatrix[4][4]; /* Right-handed, for column vectors */
typedef Quat EulerAngles;    /* (x,y,z)=ang 1,2,3, w=order code  */
#endif

//---------------------------------------------------------------------------
// EULERANGLE.H
//---------------------------------------------------------------------------

/**** EulerAngle.h - Support for 24 angle schemes ****/
/* Ken Shoemake, 1993 */
#ifndef _H_EulerAngles
#define _H_EulerAngles
//#include "QuatType.h"
/*** Order type constants, constructors, extractors ***/
    /* There are 24 possible conventions, designated by:    */
    /*	  o EulAxI = axis used initially		    */
    /*	  o EulPar = parity of axis permutation		    */
    /*	  o EulRep = repetition of initial axis as last	    */
    /*	  o EulFrm = frame from which axes are taken	    */
    /* Axes I,J,K will be a permutation of X,Y,Z.	    */
    /* Axis H will be either I or K, depending on EulRep.   */
    /* Frame S takes axes from initial static frame.	    */
    /* If ord = (AxI=X, Par=Even, Rep=No, Frm=S), then	    */
    /* {a,b,c,ord} means Rz(c)Ry(b)Rx(a), where Rz(c)v	    */
    /* rotates v around Z by c radians.			    */
#define EulFrmS	     0
#define EulFrmR	     1
#define EulFrm(ord)  ((unsigned)(ord)&1)
#define EulRepNo     0
#define EulRepYes    1
#define EulRep(ord)  (((unsigned)(ord)>>1)&1)
#define EulParEven   0
#define EulParOdd    1
#define EulPar(ord)  (((unsigned)(ord)>>2)&1)
#define EulSafe	     "\000\001\002\000"
#define EulNext	     "\001\002\000\001"
#define EulAxI(ord)  ((int)(EulSafe[(((unsigned)(ord)>>3)&3)]))
#define EulAxJ(ord)  ((int)(EulNext[EulAxI(ord)+(EulPar(ord)==EulParOdd)]))
#define EulAxK(ord)  ((int)(EulNext[EulAxI(ord)+(EulPar(ord)!=EulParOdd)]))
#define EulAxH(ord)  ((EulRep(ord)==EulRepNo)?EulAxK(ord):EulAxI(ord))
    /* EulGetOrd unpacks all useful information about order simultaneously. */
#define EulGetOrd(ord,i,j,k,h,n,s,f) {unsigned o=ord;f=o&1;o>>=1;s=o&1;o>>=1;\
    n=o&1;o>>=1;i=EulSafe[o&3];j=EulNext[i+n];k=EulNext[i+1-n];h=s?k:i;}
    /* EulOrd creates an order value between 0 and 23 from 4-tuple choices. */
#define EulOrd(i,p,r,f)	   (((((((i)<<1)+(p))<<1)+(r))<<1)+(f))
    /* Static axes */
#define EulOrdXYZs    EulOrd(X,EulParEven,EulRepNo,EulFrmS)
#define EulOrdXYXs    EulOrd(X,EulParEven,EulRepYes,EulFrmS)
#define EulOrdXZYs    EulOrd(X,EulParOdd,EulRepNo,EulFrmS)
#define EulOrdXZXs    EulOrd(X,EulParOdd,EulRepYes,EulFrmS)
#define EulOrdYZXs    EulOrd(Y,EulParEven,EulRepNo,EulFrmS)
#define EulOrdYZYs    EulOrd(Y,EulParEven,EulRepYes,EulFrmS)
#define EulOrdYXZs    EulOrd(Y,EulParOdd,EulRepNo,EulFrmS)
#define EulOrdYXYs    EulOrd(Y,EulParOdd,EulRepYes,EulFrmS)
#define EulOrdZXYs    EulOrd(Z,EulParEven,EulRepNo,EulFrmS)
#define EulOrdZXZs    EulOrd(Z,EulParEven,EulRepYes,EulFrmS)
#define EulOrdZYXs    EulOrd(Z,EulParOdd,EulRepNo,EulFrmS)
#define EulOrdZYZs    EulOrd(Z,EulParOdd,EulRepYes,EulFrmS)
    /* Rotating axes */
#define EulOrdZYXr    EulOrd(X,EulParEven,EulRepNo,EulFrmR)
#define EulOrdXYXr    EulOrd(X,EulParEven,EulRepYes,EulFrmR)
#define EulOrdYZXr    EulOrd(X,EulParOdd,EulRepNo,EulFrmR)
#define EulOrdXZXr    EulOrd(X,EulParOdd,EulRepYes,EulFrmR)
#define EulOrdXZYr    EulOrd(Y,EulParEven,EulRepNo,EulFrmR)
#define EulOrdYZYr    EulOrd(Y,EulParEven,EulRepYes,EulFrmR)
#define EulOrdZXYr    EulOrd(Y,EulParOdd,EulRepNo,EulFrmR)
#define EulOrdYXYr    EulOrd(Y,EulParOdd,EulRepYes,EulFrmR)
#define EulOrdYXZr    EulOrd(Z,EulParEven,EulRepNo,EulFrmR)
#define EulOrdZXZr    EulOrd(Z,EulParEven,EulRepYes,EulFrmR)
#define EulOrdXYZr    EulOrd(Z,EulParOdd,EulRepNo,EulFrmR)
#define EulOrdZYZr    EulOrd(Z,EulParOdd,EulRepYes,EulFrmR)

EulerAngles Eul_(float ai, float aj, float ah, int order);
Quat Eul_ToQuat(EulerAngles ea);
void Eul_ToHMatrix(EulerAngles ea, HMatrix M);
EulerAngles Eul_FromHMatrix(HMatrix M, int order);
EulerAngles Eul_FromQuat(Quat q, int order);
#endif

//---------------------------------------------------------------------------
// EULERANGLE.c
//---------------------------------------------------------------------------

/**** EulerAngles.c - Convert Euler angles to/from matrix or quat ****/
/* Ken Shoemake, 1993 */
#include <math.h>
#include <float.h>
//#include "EulerAngle.h"

EulerAngles Eul_(float ai, float aj, float ah, int order)
{
    EulerAngles ea;
    ea.x = ai; ea.y = aj; ea.z = ah;
    ea.w = (float)order;
    return (ea);
}
/* Construct quaternion from Euler angles (in radians). */
Quat Eul_ToQuat(EulerAngles ea)
{
    Quat qu;
    double a[3], ti, tj, th, ci, cj, ch, si, sj, sh, cc, cs, sc, ss;
    int i,j,k,h,n,s,f;
    EulGetOrd((unsigned int)ea.w,i,j,k,h,n,s,f);
    if (f==EulFrmR) {float t = ea.x; ea.x = ea.z; ea.z = t;}
    if (n==EulParOdd) ea.y = -ea.y;
    ti = ea.x*0.5; tj = ea.y*0.5; th = ea.z*0.5;
    ci = cos(ti);  cj = cos(tj);  ch = cos(th);
    si = sin(ti);  sj = sin(tj);  sh = sin(th);
    cc = ci*ch; cs = ci*sh; sc = si*ch; ss = si*sh;
    if (s==EulRepYes) {
	a[i] = cj*(cs + sc);	/* Could speed up with */
	a[j] = sj*(cc + ss);	/* trig identities. */
	a[k] = sj*(cs - sc);
	qu.w = (float)(cj*(cc - ss));
    } else {
	a[i] = cj*sc - sj*cs;
	a[j] = cj*ss + sj*cc;
	a[k] = cj*cs - sj*sc;
	qu.w = (float)(cj*cc + sj*ss);
    }
    if (n==EulParOdd) a[j] = -a[j];
    qu.x = (float)a[X]; qu.y = (float)a[Y]; qu.z = (float)a[Z];
    return (qu);
}

/* Construct matrix from Euler angles (in radians). */
void Eul_ToHMatrix(EulerAngles ea, HMatrix M)
{
    double ti, tj, th, ci, cj, ch, si, sj, sh, cc, cs, sc, ss;
    int i,j,k,h,n,s,f;
    EulGetOrd((unsigned int)ea.w,i,j,k,h,n,s,f);
    if (f==EulFrmR) {float t = ea.x; ea.x = ea.z; ea.z = t;}
    if (n==EulParOdd) {ea.x = -ea.x; ea.y = -ea.y; ea.z = -ea.z;}
    ti = ea.x;	  tj = ea.y;	th = ea.z;
    ci = cos(ti); cj = cos(tj); ch = cos(th);
    si = sin(ti); sj = sin(tj); sh = sin(th);
    cc = ci*ch; cs = ci*sh; sc = si*ch; ss = si*sh;
    if (s==EulRepYes) {
	M[i][i] = (float)cj;	  M[i][j] =  (float)(sj*si);    M[i][k] =  (float)(sj*ci);
	M[j][i] = (float)(sj*sh);  M[j][j] = (float)(-cj*ss+cc); M[j][k] = (float)(-cj*cs-sc);
	M[k][i] = (float)(-sj*ch); M[k][j] =  (float)(cj*sc+cs); M[k][k] =  (float)(cj*cc-ss);
    } else {
	M[i][i] = (float)(cj*ch); M[i][j] = (float)(sj*sc-cs); M[i][k] = (float)(sj*cc+ss);
	M[j][i] = (float)(cj*sh); M[j][j] = (float)(sj*ss+cc); M[j][k] = (float)(sj*cs-sc);
	M[k][i] = (float)(-sj);	 M[k][j] = (float)(cj*si);    M[k][k] = (float)(cj*ci);
    }
    M[W][X]=M[W][Y]=M[W][Z]=M[X][W]=M[Y][W]=M[Z][W]=0.0; M[W][W]=1.0;
}

/* Convert matrix to Euler angles (in radians). */
EulerAngles Eul_FromHMatrix(HMatrix M, int order)
{
    EulerAngles ea;
    int i,j,k,h,n,s,f;
    EulGetOrd(order,i,j,k,h,n,s,f);
    if (s==EulRepYes) {
	double sy = sqrt(M[i][j]*M[i][j] + M[i][k]*M[i][k]);
	if (sy > 16*FLT_EPSILON) {
	    ea.x = (float)atan2(M[i][j], M[i][k]);
	    ea.y = (float)atan2(sy, M[i][i]);
	    ea.z = (float)atan2(M[j][i], -M[k][i]);
	} else {
	    ea.x = (float)atan2(-M[j][k], M[j][j]);
	    ea.y = (float)atan2(sy, M[i][i]);
	    ea.z = 0;
	}
    } else {
	double cy = sqrt(M[i][i]*M[i][i] + M[j][i]*M[j][i]);
	if (cy > 16*FLT_EPSILON) {
	    ea.x = (float)atan2(M[k][j], M[k][k]);
	    ea.y = (float)atan2(-M[k][i], cy);
	    ea.z = (float)atan2(M[j][i], M[i][i]);
	} else {
	    ea.x = (float)atan2(-M[j][k], M[j][j]);
	    ea.y = (float)atan2(-M[k][i], cy);
	    ea.z = 0;
	}
    }
    if (n==EulParOdd) {ea.x = -ea.x; ea.y = - ea.y; ea.z = -ea.z;}
    if (f==EulFrmR) {float t = ea.x; ea.x = ea.z; ea.z = t;}
    ea.w = (float)order;
    return (ea);
}

/* Convert quaternion to Euler angles (in radians). */
EulerAngles Eul_FromQuat(Quat q, int order)
{
    HMatrix M;
    double Nq = q.x*q.x+q.y*q.y+q.z*q.z+q.w*q.w;
    double s = (Nq > 0.0) ? (2.0 / Nq) : 0.0;
    double xs = q.x*s,	  ys = q.y*s,	 zs = q.z*s;
    double wx = q.w*xs,	  wy = q.w*ys,	 wz = q.w*zs;
    double xx = q.x*xs,	  xy = q.x*ys,	 xz = q.x*zs;
    double yy = q.y*ys,	  yz = q.y*zs,	 zz = q.z*zs;
    M[X][X] = (float)(1.0 - (yy + zz)); M[X][Y] = (float)(xy - wz); M[X][Z] = (float)(xz + wy);
    M[Y][X] = (float)(xy + wz); M[Y][Y] = (float)(1.0 - (xx + zz)); M[Y][Z] = (float)(yz - wx);
    M[Z][X] = (float)(xz - wy); M[Z][Y] = (float)(yz + wx); M[Z][Z] = (float)(1.0 - (xx + yy));
    M[W][X]=M[W][Y]=M[W][Z]=M[X][W]=M[Y][W]=M[Z][W]=0.0; M[W][W]=1.0;
    return (Eul_FromHMatrix(M, order));
}

//---------------------------------------------------------------------------
// Quatern2.c
//---------------------------------------------------------------------------

#include "quatern2.h"

void Quaternion_GetEulerZXY(jeQuaternion* Q, float* pZ, float* pX, float* pY)
{
	jeXForm3d Matrix;
	EulerAngles ea;
	HMatrix M;

	jeQuaternion_ToMatrix(Q, &Matrix);

	// convert to HMatrix type
	M[X][X] = Matrix.AX;
	M[X][Y] = Matrix.AY;
	M[X][Z] = Matrix.AZ;
	M[Y][X] = Matrix.BX;
	M[Y][Y] = Matrix.BY;
	M[Y][Z] = Matrix.BZ;
	M[Z][X] = Matrix.CX;
	M[Z][Y] = Matrix.CY;
	M[Z][Z] = Matrix.CZ;
    M[W][X] = M[W][Y] = M[W][Z] = M[X][W] = M[Y][W] = M[Z][W] = 0.0;
	M[W][W] = 1.0;

	ea = Eul_FromHMatrix(M, EulOrdZXYs);

	*pZ = ea.z;
	*pX = ea.x;
	*pY = ea.y;
}