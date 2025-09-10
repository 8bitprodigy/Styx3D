/****************************************************************************************/
/*  UTIL.H																				*/
/*                                                                                      */
/*  Author: Jim Mischel		                                                            */
/*  Description: Various useful utility functions.										*/
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
#ifndef UTIL_H
#define UTIL_H

#include "basetype.h"
#ifdef __cplusplus
	extern "C" {
#endif

char *Util_Strdup (const char *s);
jeBoolean Util_SetString (char **ppString, const char *NewValue);

jeBoolean Util_IsValidInt (char const *Text, int *TheVal);
jeBoolean Util_IsValidFloat (const char *Text, float *TheFloat);
unsigned int Util_htoi (const char *s);
void Util_QuoteString (const char *s, char *d);

// Obtain the integer at the end of a string.
// For example, foo123 will return the value 123 in *pVal
// pLastChar points to the last non-numeric character (possibly end of string)
jeBoolean Util_GetEndStringValue( const char *psz, int32 *pVal, int32 *pLastChar) ;

jeBoolean Util_FileExists (const char *Filename);

#ifdef __cplusplus
	}
#endif

#endif
