/****************************************************************************************/
/*  CLASS.H                                                                             */
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

/****************
Class is an editor level object which is a selection place holder for
Object class information.  This object knows how to build class property lists
for both internal editor objects and user objects.
****************/
#pragma once

#ifndef CLASS_H
#define CLASS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "defs.h"
#include "jeProperty.h"

typedef struct tagClass Class ;

Class *			Class_Create( const char * const pszName, int Kind) ;
void			Class_Destroy( Class ** ppClass ) ;

int				Class_GetClassKind( Class * pClass );

jeProperty_List *	Class_BuildDescriptor( Class * pClass );
void
				Class_SetProperty( Class * pClass, int DataId, int DataType, jeProperty_Data * pData, jeBoolean bUpdate );
#ifdef __cplusplus
}
#endif


#endif