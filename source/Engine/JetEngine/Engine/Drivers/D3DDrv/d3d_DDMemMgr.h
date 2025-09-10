/****************************************************************************************/
/*  DDMemMgr.h                                                                          */
/*                                                                                      */
/*  Author: John Pollard                                                                */
/*  Description: Mini D3D memory manager                                                */
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
#ifndef DDMEMMGR_H
#define DDMEMMGR_H

#include <Windows.h>
#include <Assert.h>

#include "BaseType.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct	DDMemMgr				DDMemMgr;
typedef struct	DDMemMgr_Partition		DDMemMgr_Partition;

DDMemMgr	*DDMemMgr_Create(uint32 Size);
void		DDMemMgr_Destroy(DDMemMgr *MemMgr);
void		DDMemMgr_Reset(DDMemMgr *MemMgr);
uint32		DDMemMgr_GetFreeMem(DDMemMgr *MemMgr);
DDMemMgr_Partition *DDMemMgr_PartitionCreate(DDMemMgr *MemMgr, uint32 Size);
void		DDMemMgr_PartitionDestroy(DDMemMgr_Partition *Partition);
void		DDMemMgr_PartitionReset(DDMemMgr_Partition *Partition);
uint32		DDMemMgr_PartitionGetTotalMem(DDMemMgr_Partition *Partition);
uint32		DDMemMgr_PartitionGetFreeMem(DDMemMgr_Partition *Partition);
jeBoolean	DDMemMgr_PartitionAllocMem(DDMemMgr_Partition *Partition, uint32 Size);
void		DDMemMgr_PartitionFreeMem(DDMemMgr_Partition *Partition, uint32 Size);

#ifdef __cplusplus
}
#endif
#endif
