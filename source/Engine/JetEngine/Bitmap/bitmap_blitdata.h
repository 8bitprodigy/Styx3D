/****************************************************************************************/
/*  Bitmap_BlitData.h                                                                   */
/*                                                                                      */
/*  Author: Charles Bloom                                                               */
/*  Description:  The Bitmap_BlitData function                                          */
/*					Does all format conversions											*/
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
#ifndef BITMAP_BLITDATA_H
#define BITMAP_BLITDATA_H

#ifndef BITMAP_PRIVATE_H
Intentional Error : bitmap_blidata only allowed in bitmap internals!
#endif

extern jeBoolean jeBitmap_BlitData(
								const jeBitmap_Info * SrcInfo,const void *SrcData,const jeBitmap *SrcBmp,
									  jeBitmap_Info * DstInfo,		void *DstData,const jeBitmap *DstBmp,
								int SizeX,
								int SizeY);

#endif //BITMAP_BLITDATA_H
