/*!
	@file jePtrMgr.h
	
	@author John Pollard
	@brief Helper for resource load and save

	@par Licence
	The contents of this file are subject to the Jet3D Public License       
	Version 1.02 (the "License"); you may not use this file except in         
	compliance with the License. You may obtain a copy of the License at       
	http://www.jet3d.com                                                        
                                                                             
	@par
	Software distributed under the License is distributed on an "AS IS"           
	basis, WITHOUT WARRANTY OF ANY KIND, either express or implied.  See           
	the License for the specific language governing rights and limitations          
	under the License.                                                               
                                                                                  
	@par
	The Original Code is Jet3D, released December 12, 1999.                            
	Copyright (C) 1996-1999 Eclipse Entertainment, L.L.C. All Rights Reserved           
*/

#ifndef JE_PTRMGR_H
#define JE_PTRMGR_H

#include "BaseType.h"
#include "VFile.h"

#ifdef __cplusplus
extern "C" {
#endif

/*! @typedef jePtrMgr
	@brief A instance of a resource pointer helper
*/
typedef struct jePtrMgr		jePtrMgr;

typedef struct jeResourceMgr jeResourceMgr;

//=======================================================================================
//	Function prototypes
//=======================================================================================
/*! @fn jePtrMgr* jePtrMgr_Create(void)
	@brief Create a default jePtrMgr instance.
	@return The instance of jePtrMgr or NULL if failed
*/
JETAPI jePtrMgr*	JETCC jePtrMgr_Create(void);

/*! @fn jeBoolean jePtrMgr_IsValid(const jePtrMgr *PtrMgr)
	@brief Test the validity of the jePtrMr.
	
	@param[in] PtrMgr The instance subject of the validity test
	@return JE_TRUE if PtrMgr is valid, JE_FALSE otherwise
*/
JETAPI jeBoolean	JETCC jePtrMgr_IsValid(const jePtrMgr *PtrMgr);

/*! @fn jeBoolean jePtrMgr_CreateRef(jePtrMgr *PtrMgr)
	@brief Create a reference of the PtrMgr instance parameter.
	
	@param[in] PtrMgr The instance to reference
	@return JE_TRUE if succeed, JE_FALSE otherwise
*/
JETAPI jeBoolean	JETCC jePtrMgr_CreateRef(jePtrMgr *PtrMgr);

/*! @fn void jePtrMgr_Destroy(jePtrMgr **PtrMgr);
	@brief Decrement the PtrMgr reference counter and destroy it if counter reaches 0.
	
	@param[in] PtrMgr The PtrMgr to dereference and destroy if needed
*/
JETAPI void			JETCC jePtrMgr_Destroy(jePtrMgr **PtrMgr);

/*! @fn jeBoolean jePtrMgr_ReadPtr(jePtrMgr *PtrMgr, jeVFile *VFile, void **Ptr)
	@brief Reads the ptr header, and determines if the ptr is in the ptr stack.  If in stack, it refs it by 1
	
	@param[in] PtrMgr The jePtrMgr instance used to parse
	@param[in] VFile The jeVFile from where #jePtrMgr read
	@param[in] Ptr The object/item read from the file #VFile
	@return JE_TRUE if succeed, JE_FALSE otherwise
*/
JETAPI jeBoolean	JETCC jePtrMgr_ReadPtr(jePtrMgr *PtrMgr, jeVFile *VFile, void **Ptr);

/*! @fn jeBoolean jePtrMgr_PushPtr(jePtrMgr *PtrMgr, void *Ptr)
	@brief Pushes a pointer onto the ptr stack.
	
	@param[in] PtrMgr The jePtrMgr instance used to parse
	@param[in] Ptr The object/item to keep information we have seen it
	@return JE_TRUE if succeed, JE_FALSE otherwise
*/
JETAPI jeBoolean	JETCC jePtrMgr_PushPtr(jePtrMgr *PtrMgr, void *Ptr);

/*! @fn jeBoolean jePtrMgr_WritePtr(jePtrMgr *PtrMgr, jeVFile *VFile, void *Ptr, uint32 *Count)
	@brief Write the ptr header and returns the current ref count of the ptr in the stack (0 == not in stack yet).
	
	@param[in] PtrMgr The jePtrMgr instance used to parse
	@param[in] VFile The file where to write Ptr header
	@param[in] Ptr The object/item to write indexes
	@param[out] Count The number of time we have seen this pointer
	@return JE_TRUE if succeed, JE_FALSE otherwise
*/
JETAPI jeBoolean	JETCC jePtrMgr_WritePtr(jePtrMgr *PtrMgr, jeVFile *VFile, void *Ptr, uint32 *Count);

/*! @fn void jePtrMgr_PopPtr(jePtrMgr *PtrMgr, void *Ptr)
	@brief Pops the last pushed ptr off the stack (Must specify the pointer for internal error checking).
	
	@param[in] PtrMgr The jePtrMgr instance used to parse
	@param[in] Ptr The pointer to pop
	@return JE_TRUE if succeed, JE_FALSE otherwise
*/
JETAPI void			JETCC jePtrMgr_PopPtr(jePtrMgr *PtrMgr, void *Ptr);

/*! @fn jeBoolean jePtrMgr_GetPtrCount(const jePtrMgr *PtrMgr, int32 *PtrCount)
	@brief Pops the last pushed ptr off the stack (Must specify the pointer for internal error checking).
	
	@param[in] PtrMgr The jePtrMgr instance used to parse
	@param[out] PtrCount The number of pointer in the ptr stack
	@return JE_TRUE if succeed, JE_FALSE otherwise
*/
JETAPI jeBoolean	JETCC jePtrMgr_GetPtrCount(const jePtrMgr *PtrMgr, int32 *PtrCount);

/*! @fn jeBoolean jePtrMgr_GetPtrRefs(const jePtrMgr *PtrMgr, int32 *PtrRefs)
	@brief Pops the last pushed ptr off the stack (Must specify the pointer for internal error checking).
	
	@param[in] PtrMgr The jePtrMgr instance used to parse
	@param[out] PtrRefs The total number of pointer references (count of each pointer) in the ptr stack
	@return JE_TRUE if succeed, JE_FALSE otherwise
*/
JETAPI jeBoolean	JETCC jePtrMgr_GetPtrRefs(const jePtrMgr *PtrMgr, int32 *PtrRefs);

/*! @fn jeResourceMgr* jePtrMgr_GetResourceMgr(const jePtrMgr *PtrMgr)
	@brief Current jeResourceMgr accessor

	@param[in] PtrMgr The jePtrMgr instance
	@return The current jeResourceMgr or NULL if no Resource Manager set
*/
JETAPI jeResourceMgr* JETCC jePtrMgr_GetResourceMgr(const jePtrMgr *PtrMgr);

/*! 
@page ptrmgr The Pointer Manager
@section goal Description
@par
#jePtrMgr keep a stack of all resources pointers opened. It is used to avoid duplication of resources sharing the same pointer.<br>
When writing, engine adds to the #jePtrMgr stack all addresses of items it parses. Instead of writing the complete object, it only write the index in the stack.<br>
When loading, engine recreate the stack of the jePtrMgr when recreating all items. Engine queries the #jePtrMgr stack for each item it tries to create. If the object is known in
the stack, the #jePtrMgr instance will return its indexes. If not, the indexes returned is <b>-1</b>.
@section samples Examples
@par Example of read code
@code
jeActor *jeActor_CreateFromFile(jeVFile *VFile, jePtrMgr *PtrMgr)
{
	if (PtrMgr)
	{
		if (!jePtrMgr_ReadPtr(PtrMgr, VFile, &Actor))
			return NULL;

		if (Actor)
		{
			if (!jeActor_CreateRef(Actor))
				return NULL:

			return Actor;		// Ptr found in stack, return it
		}
	}

	// Create a new actor
	Actor = JE_RAM_ALLOCATE_STRUCT(jeActor);

	if (!Actor)
		return NULL;
	
	if (!jeVFile_Read(VFile, &Actor->Number, sizeof(Actor->Number))
		goto ExitWithError;

	if (PtrMgr)
	{
		// Push the ptr on the stack
		if (!jePtrMgr_PushPtr(PtrMgr, Actor))
			goto ExitWithError;
	}
	
	return Actor;

	ExitWithError:
	{
		if (Actor)
			jeRam_Free(Actor);

		return NULL;
	}
}
@endcode

@par Example of write code
@code
jeBoolean jeActor_WriteToFile(const jeActor *Actor, jeVFile *VFile, jePtrMgr *PtrMgr)
{
	uint32		Count;

	if (PtrMgr)
	{
		if (!jePtrMgr_WritePtr(PtrMgr, VFile, Actor, &Count))
			return JE_FALSE;

		if (Count)		// Already loaded
			return JE_TRUE;
	}

	if (!jeVFile_Write(VFile, &Actor->Number, sizeof(Actor->Number))
		return JE_FALSE:
	
	if (PtrMgr)
	{
		// Push the ptr on the stack
		if (!jePtrMgr_PushPtr(PtrMgr, Actor))
			return JE_FALSE;
	}

	return JE_TRUE;
}
@endcode
*/


#ifdef __cplusplus
}
#endif

#endif


