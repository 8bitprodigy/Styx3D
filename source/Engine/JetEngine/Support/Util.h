/****************************************************************************************/
/*  UTIL.H                                                                              */
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

#ifndef UTIL_H
#define UTIL_H

#ifdef __cplusplus
extern "C" {
#endif

#define UTIL_MAX_RESOURCE_LENGTH	(128)
#define FLOAT_NEAR_ZERO				(0.00001f)

void		Util_Init( unsigned long hStringResources ) ;

char *		Util_GetRcString( char * psz, const unsigned int idResource ) ;
char *		Util_LoadLocalRcString(unsigned int resid);  //allocates memory and fills string
char *		Util_LoadText( unsigned int resid );
char *		Util_StrDup( const char * const psz ) ;
int			Util_GetAppPath( char *Buf, int BufSize );

#ifdef __cplusplus
}
#endif

#endif // Prevent multiple inclusion
/* EOF: Util.h */