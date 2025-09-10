/****************************************************************************************/
/*  STRBLOCK.H																			*/
/*                                                                                      */
/*  Author: Mike Sandige	                                                            */
/*  Description: String block interface.												*/
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
#ifndef JE_STRBLOCK_H
#define JE_STRBLOCK_H

#include "BaseType.h"	// jeBoolean
#include "VFile.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct jeStrBlock jeStrBlock;

JETAPI jeStrBlock *JETCC jeStrBlock_Create(void);
JETAPI void JETCC jeStrBlock_Destroy(jeStrBlock **SB);

JETAPI jeBoolean JETCC jeStrBlock_Append(jeStrBlock **ppSB,const char *String);

JETAPI void JETCC jeStrBlock_Delete(jeStrBlock **ppSB,int Nth);

JETAPI const char *JETCC jeStrBlock_GetString(const jeStrBlock *SB, int Index);

JETAPI jeBoolean JETCC jeStrBlock_FindString(const jeStrBlock* pSB, const char* String, int* pIndex);

JETAPI int JETCC jeStrBlock_GetCount(const jeStrBlock *SB);
JETAPI int JETCC jeStrBlock_GetChecksum(const jeStrBlock *SB);

JETAPI jeStrBlock* JETCC jeStrBlock_CreateFromFile(jeVFile* pFile);
JETAPI jeBoolean JETCC jeStrBlock_WriteToFile(const jeStrBlock *SB,jeVFile *pFile);

#ifdef __cplusplus
}
#endif

#endif
