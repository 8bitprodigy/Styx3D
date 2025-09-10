/****************************************************************************************/
/*  PCache.h                                                                            */
/*                                                                                      */
/*  Author: John Pollard                                                                */
/*  Description: D3D poly cache                                                         */
/*                                                                                      */
/*  The contents of this file are subject to the Jet3D Public License                   */
/*  Version 1.01 (the "License"); you may not use this file except in                   */
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
#ifndef D3D_PCACHE_H
#define D3D_PCACHE_H

extern DRV_CacheInfo						PCache_CacheInfo;

typedef struct PCache_PolyList				PCache_PolyList;

//====================================================================================
//	API Prototypes
//====================================================================================
void PCache_BeginScene(void);
void PCache_InitStaticsAndGlobals(void);
jeBoolean PCache_InsertGouraudPoly(jeTLVertex *Verts, int32 NumVerts, uint32 Flags);
jeBoolean PCache_InsertWorldPoly(jeTLVertex *Verts, int32 NumVerts, jeRDriver_Layer *Layers, int32 NumLayers, void *LMapCBContext, uint32 Flags);
jeBoolean PCache_InsertMiscPoly(jeTLVertex *Verts, int32 NumVerts, jeRDriver_Layer *Layers, int32 NumLayers, uint32 Flags);
jeBoolean DRIVERCC PCache_BeginBatch(void);
jeBoolean DRIVERCC PCache_EndBatch(void);
jeBoolean PCache_FlushALLBatches(void);
jeBoolean PCache_FlushBatch(PCache_PolyList *PolyList);
void PCache_Reset(void);


#endif
