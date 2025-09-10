/****************************************************************************************/
/*  MEMPOOL.H                                                                           */
/*                                                                                      */
/*  Author: Charles Bloom                                                               */
/*  Description: Fixed size block memory allocator interface                            */
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
#ifndef MEMPOOL_H
#define MEMPOOL_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct MemPool MemPool;

extern MemPool * 	MemPool_Create(int HunkLength,int NumHunks,int AutoExtendNumItems);
extern int 			MemPool_Extend(		MemPool * Pool,int NumHunks);
extern void 		MemPool_Destroy(	MemPool ** pPool);
extern void 		MemPool_Reset(		MemPool * Pool);
extern void * 		MemPool_GetHunk(	MemPool * Pool);
extern int 			MemPool_FreeHunk(	MemPool * Pool,void *Hunk);
extern int 			MemPool_HasHunk(	MemPool * Pool,void * Hunk);

extern int	 		MemPool_MemoryUsed(	MemPool * Pool);

	// int returns are boolean.  note the lack of any includes!
	// NOTEZ : memory returned by GetHunk is gauranteed to be zero'ed !

#ifdef _DEBUG
extern int			MemPool_IsValid(MemPool * Pool);
#else
#define MemPool_IsValid(P)	(1)
#endif

#ifdef __cplusplus
}
#endif

#endif /*CRB_MEMPOOL_H*/
