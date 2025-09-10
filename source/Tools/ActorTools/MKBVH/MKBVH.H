/****************************************************************************************/
/*  MKBVH.H	                                                                            */
/*                                                                                      */
/*  Author:             	                                                            */
/*  Description:                                                						*/
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
#ifndef MKBVH_H
#define MKBVH_H

#ifdef __cplusplus
extern "C" {
#endif

#include "Mkutil.h"			//ReturnCode

typedef struct MkBVH_Options MkBVH_Options;

MkBVH_Options*		MkBVH_OptionsCreate    (void);
void				MkBVH_OptionsDestroy   (MkBVH_Options** ppOptions);
ReturnCode			MkBVH_ParseOptionString(MkBVH_Options* options, const char* string,
											 MK_Boolean InScript,MkUtil_Printf PrintfCallback);
void				MkBVH_OutputUsage	   (MkUtil_Printf PrintfCallback);
ReturnCode			MkBVH_DoMake		   (MkBVH_Options* options,MkUtil_Printf PrintfCallback);


#ifdef __cplusplus
}
#endif

#endif

