/****************************************************************************************/
/*  THREADLOG.H                                                                         */
/*                                                                                      */
/*  Author:  Eli Boling                                                                 */
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
#ifndef	THREADLOG_H
#define	THREADLOG_H

#include	"BaseType.h"

#ifdef	__cplusplus
extern "C" {
#endif

jeBoolean ThreadLog_Initialize(void);
void		 ThreadLog_Printf(const char *Msg, ...);
jeBoolean ThreadLog_Report(const char *FileName);

#ifdef	__cplusplus
}
#endif

#endif

