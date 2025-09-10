/****************************************************************************************/
/*  STRUTIL.H                                                                           */
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

#ifndef CRB_STRUTIL_H
#define CRB_STRUTIL_H

#include <string.h>

#ifdef __WATCOMC__
#define IN_COMPILER
#endif

#ifdef __BORLANDC__
#define IN_COMPILER
#endif

#ifdef _AMIGA
#define IN_COMPILER
#endif

#ifdef _MSC_VER
#define IN_COMPILER
#endif

#include "Utility.h"

char * getParam(char *arg,char *argv[],int *iPtr);

char * nexttok(char *str); // modifies str!
char * find_numeric(char *str);
char * skipwhitespace(char * str);

void strcommas(char *str,uint32 num);

char * strichr(const char *str,short c);
char * stristr(const char *bigstr,const char *substr);
char * strnchr(const char *str,short c,int len);
void strtolower(char *str);
void strins(char *to,const char *fm);
	 /* strins drops fm at head of to */
void strrep(char *str,char fm,char to);

#ifndef IN_COMPILER
#ifndef stricmp
int stricmp(const char *str,const char *vs);
#endif
#ifndef strupr
char *strupr(char *str);
#endif
#ifndef strrev
char *strrev(char *str);
#endif
#endif

#define strlwr strtolower
#define strlower strtolower
#define strupper strupr

#endif

