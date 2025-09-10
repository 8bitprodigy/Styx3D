/****************************************************************************************/
/*  VISOBJECT.H                                                                         */
/*                                                                                      */
/*  Author:  Charles Bloom                                                              */
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
#ifndef JE_VISOBJECT_H
#define JE_VISOBJECT_H

#include "Engine.h"
#include "jeFrustum.h"
#include "Object.h"
#include "List.h"

#ifdef __cplusplus
extern "C" {
#endif

//-------------------------

typedef struct jeVisObject		jeVisObject;
typedef struct jeVisObjectList	jeVisObjectList;

//-------------------------

jeVisObjectList *	jeVisObjectList_Create(void);
void				jeVisObjectList_Destroy(jeVisObjectList ** pList);
jeBoolean			jeVisObjectList_IsValid(const jeVisObjectList * List);

//-------------------------

jeVisObject *		jeVisObjectList_CreateObject(	jeVisObjectList * List,jeObject *Obj);
jeVisObject *		jeVisObjectList_FindObject(	const jeVisObjectList * List,const jeObject *Obj);
void				jeVisObjectList_DestroyObject(jeVisObjectList * List,jeVisObject *VO);

jeVisObject *		jeVisObjectList_GetNext(const jeVisObjectList * List,jeVisObject *VO); // use NULL to start the walk

//-------------------------

void				jeVisObjectList_RenderStart(jeVisObjectList * List, const jeEngine *Engine, 
							const jeCamera *Camera, uint32 VisFrame);

void				jeVisObjectList_RenderAll(const jeVisObjectList * List,uint32 VisFrame);


//-------------------------

void				jeVisObject_MarkVis(jeVisObject *VO,uint32 VisFrame);

void				jeVisObject_Render(jeVisObject *VO,const jeFrustum *Frustum,uint32 VisFrame);
								// not const, cuz it marks visframe

const jeObject *	jeVisObject_Object(const jeVisObject *VO);

void				jeVisObject_AddArea(jeVisObject *VO,uint32 AreaUID); // use 0 to clear the list

//-------------------------

#ifdef __cplusplus
}
#endif

#endif

