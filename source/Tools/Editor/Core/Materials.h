/****************************************************************************************/
/*  MATERIALS.H                                                                         */
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

#ifndef MATERIALS_H
#define MATERIALS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "bitmap.h"
#include "jeWorld.h"

typedef struct		Material_Struct			Material_Struct;

// Krouer: old function to generate bmps
Material_Struct *	Materials_Load( jeEngine* pEngine, jeResourceMgr* pResMgr, char* DirPath, char* Name );
// Krouer: this function create JMAT material from BMP files
Material_Struct *	Materials_ConvertToJMAT( jeEngine* pEngine, jeResourceMgr* pResMgr, char* DirPath, char* Name );
// Krouer: this function load JMAT material
Material_Struct *	Materials_LoadEx( jeEngine* pEngine, jeResourceMgr* pResMgr, char* DirPath, char* Name );

void				Materials_Destroy( Material_Struct* Material );
const char		*	Materials_GetName( Material_Struct* Material );

#ifdef _USE_BITMAPS
const jeBitmap		*	Materials_GetBitmap( Material_Struct* Material );
#endif
const jeMaterialSpec*	Materials_GetMaterialSpec( Material_Struct* Material );


#ifdef __cplusplus
}
#endif

#endif //Prevent multiple inclusion
/* EOF: Materials.h */