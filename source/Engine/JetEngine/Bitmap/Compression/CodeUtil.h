/****************************************************************************************/
/*  CODEUTIL.H                                                                          */
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
#ifndef __COMPUTIL_CODEUTIL_H
#define __COMPUTIL_CODEUTIL_H

/** some common coding routines for variable-length uint32ers ;

	all routines suffixed in three flavors :

		_byte : writes to an array
		_ari  : writes to an arithInfo

	all routines prefixed by cu_

***/

#include "Utility.h"
#include "arithc.h"
#include "lbitio.h"

#ifdef __cplusplus
extern "C" {
#endif

void cu_putEscaping_byte(uint32 val,uint8 **stream);	/** escape of 0xFF **/
uint32  cu_getEscaping_byte(uint8 **stream);	/** stream is moved ahead **/

void cu_putEscaping_ari(uint32 val,arithInfo *stream,uint32 escape);	/** escape of (1<<escape_bits) **/
uint32  cu_getEscaping_ari(arithInfo *stream,uint32 escape);

void cu_putExpanding_ari(uint32 val,arithInfo *stream,uint32 init_max,uint32 step_max);
uint32  cu_getExpanding_ari(arithInfo *stream,uint32 init_max,uint32 step_max);

void cu_putMulting_ari(uint32 val,arithInfo *stream,uint32 init_max,uint32 step_mult);
uint32  cu_getMulting_ari(arithInfo *stream,uint32 init_max,uint32 step_mult);

void cu_putExpandingSigned_ari(int val,arithInfo *stream,int init_max,int step_max);
int  cu_getExpandingSigned_ari(arithInfo *stream,int init_max,int step_max);

void cu_putMultingSigned_ari(int val,arithInfo *stream,int init_max,int step_mult);
int  cu_getMultingSigned_ari(arithInfo *stream,int init_max,int step_mult);

void cu_putEscaping_bii(int val,struct LBitIOInfo *stream,int escape_bits);	/** escape of (1<<escape_bits) **/
int  cu_getEscaping_bii(struct LBitIOInfo *stream,int escape_bits);

void cu_putExpanding_bii(int val,struct LBitIOInfo *stream,int init_bits,int step_bits);	/** escape of (1<<escape_bits) **/
int  cu_getExpanding_bii(struct LBitIOInfo *stream,int init_bits,int step_bits);

#ifdef __cplusplus
}
#endif


#endif // CODEUTIL_H


