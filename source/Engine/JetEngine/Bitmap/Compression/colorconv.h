/****************************************************************************************/
/*  COLORCONV.H                                                                         */
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
#ifndef BRANDO_COLORCONV_H
#define BRANDO_COLORCONV_H

	// YUV here is actually Y-Cb-Cr
extern void conv_RGB_to_YUV(int R,int G,int B,int *Y,int *U,int *V);
extern void conv_YUV_to_RGB(int Y,int U,int V,int *R,int *G,int *B);

extern void conv_RGB_to_HSV(int R,int G,int B,int *H,int *S,int *V);
extern void conv_HSV_to_RGB(int H,int S,int V,int *R,int *G,int *B);

extern void conv_RGB_to_LAB(int R,int G,int B,int *L,int *a,int *b);
extern void conv_LAB_to_RGB(int L,int a,int b,int *R,int *G,int *B);

#endif
