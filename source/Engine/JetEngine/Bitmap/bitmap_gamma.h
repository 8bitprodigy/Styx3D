/****************************************************************************************/
/*  Bitmap_Gamma.h                                                                      */
/*                                                                                      */
/*  Author: Charles Bloom                                                               */
/*  Description:  The Bitmap_Gamma_Apply function                                       */
/*					Fast Gamma correction routines for various pixel formats			*/
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
#ifndef BITMAP_GAMMA_H
#define BITMAP_GAMMA_H

#ifndef BITMAP_PRIVATE_H
Intentional Error : bitmap_blidata only allowed in bitmap internals!
#endif

extern jeBoolean jeBitmap_Gamma_Apply(jeBitmap * Bitmap,jeBoolean Invert);

#endif //BITMAP_GAMMA_H
