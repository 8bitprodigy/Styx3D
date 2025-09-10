/****************************************************************************************/
/*  JECHAIN.H                                                                           */
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

#ifndef JE_CHAIN_H
#define JE_CHAIN_H

#include "BaseType.h"
#include "VFile.h"
#include "jePtrMgr.h"

#ifdef __cplusplus
extern "C" {
#endif

//========================================================================================
//	Typedefs/#defines
//========================================================================================

//========================================================================================
//	Structure defs
//========================================================================================
typedef struct jeChain			jeChain;
typedef struct jeChain_Link		jeChain_Link;

typedef jeBoolean jeChain_IOFunc(jeVFile *VFile, void **LinkData, void *Context, jePtrMgr *PtrMgr); // Write
typedef jeBoolean jeChain_ReadIOFunc(jeVFile *VFile, void **LinkData, void *Context, jePtrMgr *PtrMgr); //Read

//========================================================================================
//	Function prototypes
//========================================================================================
jeChain		*jeChain_Create(void);
jeBoolean	jeChain_CreateRef(jeChain *Chain);

jeBoolean	jeChain_WriteToFile(const jeChain *Chain, jeVFile *VFile, jeChain_IOFunc *IOFunc, void *Context, jePtrMgr *PtrMgr);
jeChain		*jeChain_CreateFromFile(jeVFile *VFile, jeChain_ReadIOFunc *IOFunc, void *Context, jePtrMgr *PtrMgr);

void		jeChain_Destroy(jeChain **Chain);
jeBoolean	jeChain_IsValid(const jeChain *Chain);
jeChain_Link *jeChain_FindLink(const jeChain *Chain, void *LinkData);
jeBoolean	jeChain_AddLink(jeChain *Chain, jeChain_Link *Link);
jeBoolean	jeChain_InsertLinkAfter(jeChain *Chain, jeChain_Link *InsertAfter, jeChain_Link *Link);
jeBoolean	jeChain_InsertLinkBefore(jeChain *Chain, jeChain_Link *InsertBefore, jeChain_Link *Link);
jeBoolean	jeChain_AddLinkData(jeChain *Chain, void *LinkData);
jeBoolean	jeChain_InsertLinkData(jeChain *Chain, jeChain_Link *InsertAfter, void *LinkData);
jeBoolean	jeChain_RemoveLink(jeChain *Chain, jeChain_Link *Link);
jeBoolean	jeChain_RemoveLinkData(jeChain *Chain, void *LinkData);
uint32		jeChain_GetLinkCount(const jeChain *Chain);
jeChain_Link *jeChain_GetFirstLink(const jeChain *Chain);
jeChain_Link *jeChain_GetLinkByIndex(const jeChain *Chain, uint32 Index);
void		*jeChain_GetLinkDataByIndex(const jeChain *Chain, uint32 Index);
void		*jeChain_GetNextLinkData(jeChain *Chain, void *Start);
jeChain_Link *jeChain_LinkCreate(void *LinkData);
void		jeChain_LinkDestroy(jeChain_Link **Link);
jeBoolean	jeChain_LinkIsValid(const jeChain_Link *Link);
void		*jeChain_LinkGetLinkData(const jeChain_Link *Link);
jeChain_Link *jeChain_LinkGetNext(const jeChain_Link *Link);
jeChain_Link *jeChain_LinkGetPrev(const jeChain_Link *Link);

uint32		jeChain_LinkDataGetIndex(const jeChain *Chain, void *LinkData);

#ifdef __cplusplus
}
#endif

#endif

