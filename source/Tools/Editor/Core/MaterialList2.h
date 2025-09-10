/****************************************************************************************/
/*  MATERIALLIST2.H                                                                     */
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
#ifndef MATERIALLIST2_H
#define MATERIALLIST2_H

#include "Jet.h"


#ifdef __cplusplus
	extern "C" {
#endif


////////////////////////////////////////////////////////////////////////////////////////
//	
////////////////////////////////////////////////////////////////////////////////////////
typedef struct MaterialDirectory	MaterialDirectory;


////////////////////////////////////////////////////////////////////////////////////////
//	Prototypes
////////////////////////////////////////////////////////////////////////////////////////

//	MaterialList_GetDirectory()
//
////////////////////////////////////////////////////////////////////////////////////////
MaterialDirectory * MaterialList_GetDirectory(
	char				*DirectoryName,		// name of directory that we want
	MaterialDirectory	*CurDirectory );	// from where to start looking

//	MaterialList_Create()
//
////////////////////////////////////////////////////////////////////////////////////////
MaterialDirectory * MaterialList2_Create(
	char	*MaterialsPath );	// path where materials are found


#ifdef __cplusplus
	}
#endif

#endif
