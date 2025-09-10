/*!
	@file Array.h 
	
	@author 
	@brief Default array management set of functions

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

#ifndef JE_ARRAY_H
#define JE_ARRAY_H

#include "BaseType.h"
#include "VFile.h"
#include "jePtrMgr.h"

#ifdef __cplusplus
extern "C" {
#endif

//-----------------------------------------------------------------------
// types

/*! @typedef jeArray
    @brief A reference to an Array
*/
typedef struct jeArray jeArray;

/*! @typedef jeArray_Index
    @brief The jeArray Index type
*/
typedef uint32 jeArray_Index;

/*! @typedef jeArray_Defragmenter
    @brief A reference to an Array defragmenter

	@ref jearraydefrag
*/
typedef struct jeArray_Defragmenter jeArray_Defragmenter;

/*! @def jeArray_MaxIndex
    @brief The last jeArray Index value
*/
#define jeArray_MaxIndex        ((jeArray_Index)((~ ((jeArray_Index)0) )-1))

/*! @def JE_ARRAY_NULL_INDEX
    @brief The NULL jeArray Index value
*/
#define JE_ARRAY_NULL_INDEX     (jeArray_MaxIndex+1)

/*! @typedef jeArray_IOFunc
*   @brief A reference to the Read/Write function to load/save array elements.
*/
typedef jeBoolean (* jeArray_IOFunc) (jeVFile * File,void * Element,void *Context);

//-----------------------------------------------------------------------
// protos

/*! @fn jeArray* jeArray_Create(int32 HunkLength, int32 NumHunks, int32 AutoExtendNumItems)
    @brief Create a jeArray instance for NumHunks elements of HunkLength bytes.
	@param[in] HunkLength The size of the element
	@param[in] NumHunks	The count of elements
	@param[in] AutoExtendNumItems The number to use when need to increase Array size
	@return The jeArray instance or NULL if failed
*/
JETAPI jeArray*			JETCC jeArray_Create(int32 HunkLength, int32 NumHunks, int32 AutoExtendNumItems);

JETAPI void 			JETCC jeArray_CreateRef(		jeArray * Array);
JETAPI void 			JETCC jeArray_Destroy(		jeArray ** pArray);
JETAPI jeBoolean		JETCC jeArray_Extend(			jeArray * Array, int32 NumHunks);
JETAPI void				JETCC jeArray_Reset(			jeArray * Array);

JETAPI void *			JETCC jeArray_GetNewElement(	jeArray * Array);
JETAPI jeBoolean		JETCC jeArray_FreeElement(	jeArray * Array,void * Element);
	
JETAPI jeArray_Index	JETCC jeArray_GetElementIndex(void * Element);
							// == (ArrayPtr - Array)/sizeof(Item)
JETAPI void *			JETCC jeArray_GetElement(jeArray * Array,jeArray_Index Index);
							// == Array[Index]

JETAPI void *			JETCC jeArray_GetNextElement(jeArray * Array,void * H);
							// use NULL to get it started

JETAPI jeArray_Index JETCC jeArray_GetNextIndex(jeArray * Array, jeArray_Index Index);
							// use JE_ARRAY_NULL_INDEX to get it started
/*! @fn jeArray_Defragmenter* jeArray_DefragmentStart(jeArray * Array)
	@brief Create  drefragmenter for the array Array
	@param[in] Array The array to defragment
	@return The defragmenter instance or NULL if failed
	@todo Implement this function in order to return other thing than an empty structure.
*/
JETAPI jeArray_Defragmenter* JETCC jeArray_DefragmentStart(jeArray * Array);

/*! @fn jeArray_Index jeArray_Defragment(jeArray_Defragmenter * D, jeArray_Index I)
	@brief Return the defragmented index of the Ith element
	@param[in] D The array defragmenter to use
	@param[in] I The array index to defragment
	@return The new index of the element
	@todo Implement body of this function, currently only return I value.
*/
JETAPI jeArray_Index		JETCC jeArray_Defragment(jeArray_Defragmenter * D, jeArray_Index I);

/*! @fn void jeArray_DefragmentEnd(jeArray_Defragmenter * D)
	@brief End the defragmenter use and destroy its resources
	@param[in] D The array defragmenter to use
	@todo Implement body of this function, currently do nothing
*/
JETAPI void 				JETCC jeArray_DefragmentEnd(jeArray_Defragmenter * D);

/*! @fn jeArray* jeArray_CreateFromFile(jeVFile * File, uint16 lVersionSizeOffset, jeArray_IOFunc ElementReader,void *ReaderContext)
	@brief Create an array from data read from a file


	@param[in] File The file from where to read initialisating data.
	@param[in] lVersionSizeOffset The size offset to add in the element size allocation.
	@param[in] ElementReader The element reader callback function.
	@param[in] ReaderContext The data to pass to the callback.
	@return An array instance or NULL if failed.
*/
JETAPI jeArray *		JETCC jeArray_CreateFromFile(jeVFile * File, uint16 lVersionSizeOffset, jeArray_IOFunc ElementReader,void *ReaderContext);

/*! @fn jeBoolean jeArray_WriteToFile(const jeArray * Array, jeVFile * File, jeArray_IOFunc ElementWriter,void *WriterContext)
	@brief Write array data to the file.


	@param[in] Array The array to write.
	@param[in] File The file where to write data.
	@param[in] ElementWriter The element writer callback function.
	@param[in] WriterContext The data to pass to the callback.
	@return JE_TRUE if succeed, JE_FALSE otherwise.
*/
JETAPI jeBoolean		JETCC jeArray_WriteToFile(const jeArray * Array, jeVFile * File, jeArray_IOFunc ElementWriter,void *WriterContext);
							// you should _Defragment before you WriteToFile if you can !!!!

//-----------------------------------------------------------------------
// docs

/*! @page jearraydefrag jeArray Defrag Documentation
Degfrag sample code:<br>
<b>The defragmenter code is not implemented yet. No need to use it!</b>
@code
void DoMyDefrag(jeArray * A)
{
jeArray_Defragmenter * D;

	D = jeArray_DefragmentStart(A);
	assert(D);

	while( I have stuff )
	{
		// remap an Index
		MyIndex = jeArray_GetElementIndex(MyData->Pointer);
		jeArray_Defragment(D, & MyIndex );
		MyData->Index = MyIndex;
	}

	jeArray_DefragmentDone(D);

	while ( I have stuff )
	{
		MyData->Pointer = jeArray_GetElement(A,MyData->Index);
	}

}
@endcode

*/

#ifdef __cplusplus
}
#endif

#endif /* ARRAY_H */

