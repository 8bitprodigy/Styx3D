/****************************************************************************************/
/*  TRANSFORM.H                                                                         */
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
#ifndef __TRANSFORM_H
#define __TRANSFORM_H

#include "BaseType.h"
#include "Wavelet.h"
#include "Utility.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct image image;

extern int nTransforms;
extern jeBoolean transformMips[];
extern char * transformNames[];

extern void transformImageInt(image *im,int levels,jeBoolean inverse,int transformN,
									jeBoolean doLHtranspose,jeBoolean doBlock);

typedef void (*pyramidHook) (void *passback, image *im,int level, int width, int height);

extern void unTransformImageIntToPyramid(image *im,int levels,int transformN,
				int fullLevel,int lowLevel,pyramidHook hook,void *passback,jeBoolean doLHtranspose);

extern void pyramidHook_patchToRaw (void *passback, image *im,int level, int width, int height);

void transformPyramid(int ** pyramid,int levels,int fulllen,int transformN,int inverse);

void transposeLH(image *im,int plane,int level);
void transposeHL(image *im,int plane,int level);

#ifdef __cplusplus
}
#endif

#endif // TRANSFORM

