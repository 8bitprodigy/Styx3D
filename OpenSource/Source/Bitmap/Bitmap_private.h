#ifndef BITMAP_PRIVATE_H
#define BITMAP_PRIVATE_H

/****************************************************************************************/
/*  Bitmap._h                                                                           */
/*                                                                                      */
/*  Author: Charles Bloom                                                               */
/*  Description:  Engine-Internal Bitmap Functions                                      */
/*                                                                                      */
/*  The contents of this file are subject to the Genesis3D Public License               */
/*  Version 1.01 (the "License"); you may not use this file except in                   */
/*  compliance with the License. You may obtain a copy of the License at                */
/*  http://www.genesis3d.com                                                            */
/*                                                                                      */
/*  Software distributed under the License is distributed on an "AS IS"                 */
/*  basis, WITHOUT WARRANTY OF ANY KIND, either express or implied.  See                */
/*  the License for the specific language governing rights and limitations              */
/*  under the License.                                                                  */
/*                                                                                      */
/*  The Original Code is Genesis3D, released March 25, 1999.                            */
/*  Genesis3D Version 1.1 released November 15, 1999                                 */
/*  Copyright (C) 1999 WildTangent, Inc. All Rights Reserved           */
/*                                                                                      */
/****************************************************************************************/


#include	"Bitmap.h"
#include	"DCommon.h"

#ifdef __cplusplus
extern "C" {
#endif

/***
**
*
* These functions are intended for internal Genesis use only.
*
* This can become public once Driver is exposed/formalized. !@@!
*
* Only engine.c , and maybe world & bitmaplist should include this !
*
* You MUST DetachDriver from a bitmap, before you change the Driver
*
**
 **/

#define BITMAP_GENESIS_INTERNAL	GENESISCC

geBoolean			GENESISCC geBitmap_AttachToDriver(geBitmap *Bmp, DRV_Driver * Driver, uint32 DriverFlags);
	// use Driverflags == 0 to use the flags from _SetDriverFlags

geBoolean			GENESISCC geBitmap_DetachDriver(geBitmap *Bmp, geBoolean DoUpdate);
	// You MUST Detach the Driver while it is identical to the way it was when attached!

geBitmap_Palette *	GENESISCC geBitmap_Palette_CreateFromDriver(DRV_Driver * Driver,gePixelFormat Format,int Size); /*<>*/

geRDriver_THandle * GENESISCC geBitmap_GetTHandle(const geBitmap *Bmp);

geBoolean			GENESISCC geBitmap_SetDriverFlags(geBitmap *Bmp,uint32 flags);

geBoolean			GENESISCC geBitmap_SetGammaCorrection_DontChange(geBitmap *Bmp,geFloat Gamma);

#ifdef __cplusplus
}
#endif


#endif // BITMAP_PRIVATE_H


