/****************************************************************************************/
/*  IMAGE.H                                                                             */
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
#ifndef __IMAGE_H
#define __IMAGE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "BaseType.h"
#include "Utility.h"

typedef struct image 
{
	int width,height,stride,planes;
	int plane_size,tot_size,plane_bytes,tot_bytes;
	int ***data;
		/** data[plane][y][x] **/
		/** data[plane][0] is a pointer to the whole plane **/

	jeBoolean alphaIsBoolean;	// otherwise a full uint8 alpha channel
	uint8 * alpha;		// plane_size bytes of alpha data , NULL for none
} image;

extern image * newImage(int width, int height,int planes);
extern image * copyImage(image *im);
extern image * newImageFrom(image *im);
extern void freeImage(image *im);
extern void zeroImage(image *im);

extern image * newImageAlpha(int width, int height,int planes,jeBoolean alphaIsBoolean);
extern void insertImageAlpha(image *im,uint8 *alpha);

extern void patchImage(image *fm,image *to,int fmx,int fmy,int w,int h,int tox,int toy);

extern jeBoolean extendImage(image *im,int fmw,int fmh);
extern jeBoolean zeroExtendedImage(image *im,int fmw,int fmh,int levels);
extern jeBoolean zeroAlphaImage(image *im,int levels);

extern void transposeImage(image *im);

double PSNR(double rmse);
double imageRMSE(image *src,image *comp);

#ifdef __cplusplus
}
#endif
#endif

