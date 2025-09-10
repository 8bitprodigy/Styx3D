/****************************************************************************************/
/*  CRC32.H                                                                             */
/*                                                                                      */
/*  Author: Charles Bloom                                                               */
/*  Description: CRC construction module                                                */
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
#ifndef JE_CRC32_H
#define JE_CRC32_H

#include "BaseType.h"

#ifdef __cplusplus
extern "C" {
#endif

extern uint32 CRC32_Array(const uint8 * buf,uint32 buflen);

extern uint32 CRC32_Start(void);
extern uint32 CRC32_Finish(uint32 crc);

extern uint32 CRC32_AddByte(uint32 crc,uint8 b);
extern uint32 CRC32_AddWord(uint32 crc,uint16 w);
extern uint32 CRC32_AddLong(uint32 crc,uint32 w);

#ifdef __cplusplus
}
#endif

#endif /* CRB_CRC32 */
