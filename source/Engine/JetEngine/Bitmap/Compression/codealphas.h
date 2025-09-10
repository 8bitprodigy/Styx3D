/****************************************************************************************/
/*  CODEALPHAS.H                                                                        */
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
#ifndef __CODEIMAGE_ALPHAS_H
#define __CODEIMAGE_ALPHAS_H

// to be included only by codeimage.c !

#include "BaseType.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct image image;
typedef struct coder coder;

extern void encodeImageAlpha(coder * encoder,image *im);
extern void decodeImageAlpha(coder * decoder,image *im);

#ifdef __cplusplus
}
#endif

#endif // CODE_IMAGE_ALPHAS_H
