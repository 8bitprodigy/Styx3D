/****************************************************************************************/
/*  REFPOOL.H                                                                           */
/*                                                                                      */
/*  Author: Eli Boling                                                                  */
/*  Description: Handle based reference pool interface                                  */
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
#ifndef	REFPOOL_H
#define	REFPOOL_H

#include	"basetype.h"

#ifdef	__cplusplus
extern "C" {
#endif

typedef	struct	RefPool		RefPool;

RefPool *	JETCC RefPool_Create(int Increment);

void	JETCC RefPool_Destroy(RefPool **Pool);

void ** JETCC RefPool_RefCreate(RefPool *Pool);

void JETCC RefPool_RefDestroy(RefPool *Pool, void ***Ref);

int JETCC RefPool_GetRefCount(const RefPool *Pool);

void ** JETCC RefPool_GetNextRef(const RefPool *Pool, const void **Ref);

#ifdef	__cplusplus
}
#endif

#endif

