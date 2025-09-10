/****************************************************************************************/
/*  JEPOLYMGR.H                                                                         */
/*                                                                                      */
/*  Author:  John Pollard                                                               */
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

#ifndef JE_POLYMGR_H
#define JE_POLYMGR_H

#include "Dcommon.h"
#include "Engine.h"
#include "BaseType.h"
#include "jeTypes.h"

#ifdef __cplusplus
extern "C" {
#endif

//========================================================================================
//	Typedefs/#defines
//========================================================================================
typedef struct jePolyMgr				jePolyMgr;

//========================================================================================
//	Structure defs
//========================================================================================

//========================================================================================
//	Function prototypes
//========================================================================================

jePolyMgr *jePolyMgr_Create(void);
jeBoolean jePolyMgr_IsValid(const jePolyMgr *Mgr);
jeBoolean jePolyMgr_CreateRef(jePolyMgr *Mgr);
void jePolyMgr_Destroy(jePolyMgr **Mgr);
void jePolyMgr_SetDriver(jePolyMgr *Mgr, DRV_Driver *Driver);
void jePolyMgr_RenderGouraudPoly(	jePolyMgr			*Mgr, 
									const jeTLVertex	*Verts, 
									int32				NumVerts, 
									uint32				Flags);
void jePolyMgr_RenderMiscPoly(	jePolyMgr				*Mgr, 
								const jeTLVertex		*Verts, 
								int32					NumVerts, 
								jeRDriver_Layer			*Layers,
								int32					NumLayers,
								uint32					Flags);

void jePolyMgr_RenderWorldPoly(	jePolyMgr				*Mgr, 
								const jeTLVertex		*Verts, 
								int32					NumVerts, 
								jeRDriver_Layer			*Layers,
								int32					NumLayers,
								void					*LMapCBContext,
								uint32					Flags);
void jePolyMgr_FlushBatch(jePolyMgr *Mgr);

#ifdef __cplusplus
}
#endif

#endif
