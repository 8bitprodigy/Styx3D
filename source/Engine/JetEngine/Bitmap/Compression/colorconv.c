/****************************************************************************************/
/*  COLORCONV.C                                                                         */
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
#include "Utility.h"
#include "colorconv.h"
#include <math.h>

#pragma warning(disable : 4244 )

// ======================================================================

/*****
	supposedly better YUV :

    RGB -> YCbCr (with Rec 601-1 specs)         | YCbCr (with Rec 601-1 specs) -> RGB
    Y=  0.2989*Red+0.5867*Green+0.1144*Blue     | Red=  Y+0.0000*Cb+1.4022*Cr
    Cb=-0.1687*Red-0.3312*Green+0.5000*Blue     | Green=Y-0.3456*Cb-0.7145*Cr
    Cr= 0.5000*Red-0.4183*Green-0.0816*Blue     | Blue= Y+1.7710*Cb+0.0000*Cr
*****/

/**********

our YUV :

    Y  =  0.29900 * R + 0.58700 * G + 0.11400 * B
    Cb = -0.16874 * R - 0.33126 * G + 0.50000 * B  
    Cr =  0.50000 * R - 0.41869 * G - 0.08131 * B  

    R = Y                + 1.40200 * Cr
    G = Y - 0.34414 * Cb - 0.71414 * Cr
    B = Y + 1.77200 * Cb

***********/

#define YUV_SHIFT 	14
#define YUV_EXTRA 	0
#define YUV_SHIFT_MORE 	(YUV_SHIFT + YUV_EXTRA)
#define YUV_SHIFT_LESS 	(YUV_SHIFT - YUV_EXTRA)
#define YUV_HALF_MORE	(1<<(YUV_SHIFT_MORE-1))
#define YUV_HALF_LESS	(1<<(YUV_SHIFT_LESS-1))
#define YUV_ONE		(1<<YUV_SHIFT)
#define Y_R   ((int)( 0.29900 * YUV_ONE ))
#define Y_G   ((int)( 0.58700 * YUV_ONE ))
#define Y_B   ((int)( 0.11400 * YUV_ONE ))
#define U_R   ((int)(-0.16874 * YUV_ONE ))
#define U_G   ((int)(-0.33126 * YUV_ONE ))
#define U_B   ((int)( 0.50000 * YUV_ONE ))
#define V_R   ((int)(-0.50000 * YUV_ONE ))	// ** important sign change of 'V' from jpeg default
#define V_G   ((int)( 0.41869 * YUV_ONE ))
#define V_B   ((int)( 0.08131 * YUV_ONE ))
#define R_Y   (     YUV_ONE )       
#define R_U   (           0 )       
#define R_V   ((int)(-1.40200 * YUV_ONE ))
#define G_Y   (     YUV_ONE )       
#define G_U   ((int)(-0.34414 * YUV_ONE ))
#define G_V   ((int)( 0.71414 * YUV_ONE ))
#define B_Y   (     YUV_ONE )       
#define B_U   ((int)( 1.77200 * YUV_ONE ))
#define B_V   (           0 )       

#define Y_RGB(R,G,B) (( Y_R * (R) + Y_G * (G) + Y_B * (B) + YUV_HALF_LESS ) >> YUV_SHIFT_LESS)
#define U_RGB(R,G,B) (( U_R * (R) + U_G * (G) + U_B * (B) + YUV_HALF_LESS ) >> YUV_SHIFT_LESS)
#define V_RGB(R,G,B) (( V_R * (R) + V_G * (G) + V_B * (B) + YUV_HALF_LESS ) >> YUV_SHIFT_LESS)
#define R_YUV(Y,U,V) (( R_Y * (Y) + R_U * (U) + R_V * (V) + YUV_HALF_MORE ) >> YUV_SHIFT_MORE)
#define G_YUV(Y,U,V) (( G_Y * (Y) + G_U * (U) + G_V * (V) + YUV_HALF_MORE ) >> YUV_SHIFT_MORE)
#define B_YUV(Y,U,V) (( B_Y * (Y) + B_U * (U) + B_V * (V) + YUV_HALF_MORE ) >> YUV_SHIFT_MORE)

void conv_RGB_to_YUV(int R,int G,int B,int *Y,int *U,int *V)
{
*Y = Y_RGB(R,G,B);
*U = U_RGB(R,G,B) + 127;
*V = V_RGB(R,G,B) + 127;
	// U & V nearly identical
}
void conv_YUV_to_RGB(int Y,int U,int V,int *R,int *G,int *B)
{
U -= 127; V -= 127;
*R = R_YUV(Y,U,V);
*G = G_YUV(Y,U,V);
*B = B_YUV(Y,U,V);
}

//==============================================
/*****

	HSV :

******/

#define max3(a,b,c) max(max(a,b),c)
#define min3(a,b,c) min(min(a,b),c)

#define H_RANGE	(256)
#define H_SCALE	(H_RANGE/6)

#define SAT_SHIFT	8
#define SAT_RANGE	(1<<SAT_SHIFT)

void conv_RGB_to_HSV(int R,int G,int B,int *H,int *S,int *V)
{
int max,min,diff;

	max = max3(R,G,B);
	min = min3(R,G,B);
	diff = max - min;

	if ( max == 0 || diff == 0 ) {
		*V = *S = *H = 0;
	} else {
		int r,g,b;

		*V = max;	// value automatically has the range of the inputs

		*S = ((diff)<<SAT_SHIFT)/max;	// saturation in one byte

		r = ((max - R)*H_SCALE)/diff;
		g = ((max - G)*H_SCALE)/diff;
		b = ((max - B)*H_SCALE)/diff;

		if ( R == max ) {
			*H = b - g;
		} else if ( G == max ) {
			*H = r - b + 2*H_SCALE;
		} else { /** B == max **/
			*H = g - r + 4*H_SCALE;
		}
		if ( *H < 0 ) *H += H_RANGE;
	}
}

void conv_HSV_to_RGB(int H,int S,int V,int *R,int *G,int *B)
{
double f;
int p,q,t;

	while ( H < 0 ) H += H_RANGE; while( H >= H_RANGE ) H -= H_RANGE;
	f = H * (1.0/H_SCALE);
	H = (int)f;
	f -= H;		// f now in [0,1]

	t = (V*S)>>SAT_SHIFT;
	p = V - t;
	q = V - (int)(f*(double)t);
	t = V + p - q;

	switch(H) {
		case 0: *R=V; *G=t; *B=p; break;

		case 1: *R=q; *G=V; *B=p; break;
		case 2: *R=p; *G=V; *B=t; break;

		case 3: *R=p; *G=q; *B=V; break;
		case 4: *R=t; *G=p; *B=V; break;

		case 5: *R=V; *G=p; *B=q; break;
	}
}


//==============================================
/*****

	Lab :

******/

// #define L_SCALE_100	// turning off improves error mucho , 33 -> 12

void conv_RGB_to_LAB(int R,int G,int B,int *L,int *a,int *b)
{
double x,y,z;

    x = 0.001777246 *R+ 0.001410251*G + 0.0007339904*B ;    
    y = 0.0008705882*R+ 0.002772549*G + 0.0002784314*B ;
    z = 0.0000720215*R+ 0.000468140*G + 0.003381408 *B ;

	if ( y > 0.008856 ) {
		x = pow(x,1.0/3.0);
		y = pow(y,1.0/3.0);
		z = pow(z,1.0/3.0);
	} else {
		x = (7.787 * x + 0.137931);
		y = (7.787 * y + 0.137931);
		z = (7.787 * z + 0.137931);
	}

#ifdef L_SCALE_100
	*L = (116 * y) - 15;		// 0 -> 100
#else
	*L = (295 * y) - 40.69;		// 0 -> 255
#endif

	*a = 500 * ( x - y );		//  -89-98
	*b = 200 * ( y - z );		//  -144-91
}

void conv_LAB_to_RGB(int L,int a,int b,int *R,int *G,int *B)
{
double x,y,z;

#ifdef L_SCALE_100
	y = (double)(L+15)*0.00862069;
#else
	y = (double)(L+40.69)*0.003389831;
#endif

	x = a*0.002 + y;
	z = y - b*0.005;

#ifdef L_SCALE_100
	if ( L >= 9 ) {
#else
	if ( L >= 20 ) {
#endif
		x *= x*x;
		y *= y*y;
		z *= z*z;
	} else {
		x = 0.12842*(x - 0.137931);
		y = 0.12842*(y - 0.137931);
		z = 0.12842*(z - 0.137931);
	}

	// from the color space faq:
	// an avg. err of 33
	*R  =  742.808130*x - 355.215*y - 132.182820*z ; 
	*G  = -234.992190*x + 478.38 *y + 11.663190 *z ;
	*B  =  16.490680 *x -  58.395*y + 296.855955*z ;

/***
	// the true matrix inverse:
	// err = 2288 !
	*R  = 741.981259  *x -355.150331*y - 31.815540 *z ;
	*G  = -234.659674 *x +478.084430*y + 11.570397 *z ;
	*B  = 16.683870   *x -58.624094 *y + 296.940501*z ;
*******/

	putminmax(*R,0,0xFF);
	putminmax(*G,0,0xFF);
	putminmax(*B,0,0xFF);
}

