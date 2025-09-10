/****************************************************************************************/
/*  ECLIPSENAMES.H                                                                      */
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
#ifndef	ECLIPSENAMES_H
#define	ECLIPSENAMES_H

#include	"Symbol.h"

#ifdef	__cplusplus
extern "C" {
#endif

typedef	enum
{
	JE_ECLIPSENAMES_STRUCTUREFIELDS,
	JE_ECLIPSENAMES_FIELDDEFAULTVALUE,
	JE_ECLIPSENAMES_TYPES,
	JE_ECLIPSENAMES_TYPEDEFINITIONS,
}	jeEclipseNames_Id;

jeSymbol *jeEclipseNames(jeSymbol_Table *ST, jeEclipseNames_Id Id);

#ifdef	__cplusplus
}
#endif

#endif

