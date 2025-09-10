/****************************************************************************************/
/*  OBJECTDEF.H                                                                         */
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
#pragma once

// This header is to be inlcuded in any module which is going to define a Object submodule

#ifndef OBJECTDEF_H
#define OBJECTDEF_H

#ifdef __cplusplus
extern "C" {
#endif
#include "jwObject.h"
#include "Group.h"
#include "Drawtool.h"

typedef struct tagObject
{
	OBJECT_KIND	ObjectKind ;
	Group	*	pGroup ;
	uint32		GroupTag ; //Used to reattach to group
	int32		RefCnt;
	char	*	pszName ;
	int32		nNumber ; //Unique numeber for objects of same name and kind
	uint32		miscFlags ; // Temporary flags set to mark an object
} Object ;

jeBoolean		Object_Init( Object * pObject, Group * pGroup, OBJECT_KIND ObjectKind, const char * const pszName, int32 nNumber );
#define OBJECT_DIRTY	0x10000000
#define OBJECT_INLEVEL  0x20000000

#ifdef __cplusplus
}
#endif

#endif // Prevent multiple inclusion
/* EOF: ObjectDef.h */
