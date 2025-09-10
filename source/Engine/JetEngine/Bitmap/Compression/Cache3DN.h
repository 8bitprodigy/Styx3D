/****************************************************************************************/
/*  CACHE3DN.H                                                                          */
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
#ifndef __CACHE3DN_H
#define __CACHE3DN_H

#ifdef __cplusplus
extern "C" {
#endif

void cachetouch_w_3dnow(const void * data,int num32s);

void cachetouch_r_3dnow(const void * data,int num32s);

void memclear_3dnow(void *data,int len);

void copy32_3dnow(char *to,const char *from);

void copy32_3dnow_fastcall(void);

void copy32_8_3dnow(char *to,char ** froms);

void memcpy32s_3dnow(char *to,const char * from,int num32s);

#ifdef __cplusplus
}
#endif

#endif

