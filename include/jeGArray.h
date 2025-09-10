/****************************************************************************************/
/*  JEGARRAY.H                                                                          */
/*                                                                                      */
/*  Author:  John Pollard                                                               */
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

#ifndef JE_GARRAY_H
#define JE_GARRAY_H

#include "BaseType.h"
#include "VFile.h"
#include "jePtrMgr.h"

#ifdef __cplusplus
extern "C" {
#endif

//========================================================================================
//	Typedefs/#defines
//========================================================================================
typedef struct	jeGArray			jeGArray;
typedef			uint16				jeGArray_Index;
									
typedef			void				jeGArray_Element;
typedef			uint16				jeGArray_RefType;

typedef			jeBoolean			jeGArray_IOFunc(jeVFile *File, jeGArray_Element *Element, void *Context);


#define JE_GARRAY_MAX_ELEMENTS		(0xffff-1)
#define	JE_GARRAY_NULL_INDEX		(JE_GARRAY_MAX_ELEMENTS+1)
#define JE_GARRAY_MAX_ELEMENT_SIZE	0xffff

#define	JE_GARRAY_MAX_ELEMENT_REFCOUNT	0xffff
//========================================================================================
//	Structure defs
//========================================================================================

//========================================================================================
//	Function prototypes
//========================================================================================
jeGArray	*jeGArray_Create(int32 StartElements, int32 ElementSize);


jeBoolean	jeGArray_WriteToFile(const jeGArray *Array, jeVFile *VFile, jeGArray_IOFunc *IOFunc, void *IOFuncContext, jePtrMgr *PtrMgr);
jeGArray	*jeGArray_CreateFromFile(jeVFile *VFile, jeGArray_IOFunc *IOFunc, void *IOFuncContext, jePtrMgr *PtrMgr);

jeBoolean	jeGArray_CreateRef(jeGArray *Array);
void		jeGArray_Destroy(jeGArray **Array);
jeBoolean	jeGArray_IsValid(const jeGArray *Array);
jeGArray_Index jeGArray_AddElement(jeGArray *Array, const jeGArray_Element *Element);
jeBoolean	jeGArray_RefElement(jeGArray *Array, jeGArray_Index Index);
void		jeGArray_RemoveElement(jeGArray *Array, jeGArray_Index *Index);
int32		jeGArray_GetSize(const jeGArray *Array);
jeGArray_Element *jeGArray_GetElements(const jeGArray *Array);
jeGArray_RefType *jeGArray_GetRefCounts(const jeGArray *Array);
const		jeGArray_RefType jeGArray_GetElementRefCountByIndex(const jeGArray *Array, jeGArray_Index Index);
void		jeGArray_SetElementByIndex(jeGArray *Array, jeGArray_Index Index, const jeGArray_Element *Element);
const		jeGArray_Element *jeGArray_GetElementByIndex(const jeGArray *Array, jeGArray_Index Index);

#ifdef __cplusplus
}
#endif

#endif
