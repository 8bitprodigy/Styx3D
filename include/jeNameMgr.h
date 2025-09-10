/****************************************************************************************/
/*  JENAMEMGR.H                                                                         */
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
#ifndef __JE_NAMEMGR_H__
#define __JE_NAMEMGR_H__

#include "BaseType.h"
#include "VFile.h"

#ifdef NEWSAVE

#ifdef __cplusplus
extern "C" {
#endif

typedef struct jeChain			jeChain;
typedef struct jeChain_Link		jeChain_Link;

typedef struct jeNameMgr jeNameMgr;
typedef void *  (JETCC *jeNameMgr_CreateFromFileCallback)(jeVFile *VFile, jeNameMgr *NM);
typedef jeBoolean (JETCC *jeNameMgr_WriteToFileCallback)(void *DataPtr, jeVFile *VFile, jeNameMgr *NM);

#define JE_NAME_MGR_CREATE_FOR_READ (1<<0)
#define JE_NAME_MGR_CREATE_FOR_WRITE (1<<1)

JETAPI jeNameMgr * JETCC jeNameMgr_Create(jeVFile *System, int32 CreateFlags);
JETAPI jeBoolean JETCC jeNameMgr_CreateRef(jeNameMgr *NameMgr);
JETAPI void JETCC jeNameMgr_Destroy(jeNameMgr **NameMgr);
JETAPI jeBoolean JETCC jeNameMgr_Write(jeNameMgr *NM, jeVFile *VFile, void *PtrToData, jeNameMgr_WriteToFileCallback CB_Write);
JETAPI jeBoolean JETCC jeNameMgr_Read(jeNameMgr *NM, jeVFile *VFile, jeNameMgr_CreateFromFileCallback CB_Read, void **ReturnPointer);
JETAPI jeBoolean JETCC jeNameMgr_WriteFlush(jeNameMgr *NM);

#ifdef __cplusplus
}
#endif

#endif

#endif
