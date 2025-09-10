/****************************************************************************************/
/*  EDITMSG.H                                                                           */
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
#ifndef EDITMSG_H
#define EDITMSG_H

typedef struct Select3dContextDef
{
	jeVec3d Front;
	jeVec3d Back;
	jeVec3d Impact;
} Select3dContextDef;



typedef enum {
	JETEDITOR_GET_JEBRUSH = 4000,	// Context is pointer to jeBrush pointer (jeBrush**)
	JETEDITOR_SELECT3D,				// Context is Select3dContextDef
	JETEDITOR_APPLYMATERIAL,		// Context is jeBitmap
	JETEDITOR_APPLYMATERIALSPEC,	// Context is jeMaterialSpec
};

#endif // EDITMSG_H
