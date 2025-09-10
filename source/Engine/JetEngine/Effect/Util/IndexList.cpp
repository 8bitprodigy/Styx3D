/****************************************************************************************/
/*  INDEXLIST.C                                                                         */
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
//
//	Used to manage an indexed list of items.
//
#include <memory.h>
#include <assert.h>
#include "Jet.h"
#include "ram.h"


////////////////////////////////////////////////////////////////////////////////////////
//	The indexed list struct.
////////////////////////////////////////////////////////////////////////////////////////
typedef struct
{
	int		Size;			// how many elements in the list
	int		IncSize;		// how many elements to add to the list when it gets full
	void	**ElementList;	// list of all the elements

} IndexList;



////////////////////////////////////////////////////////////////////////////////////////
//
//	IndexList_GetListSize()
//
//	Get the size of a list.
//
////////////////////////////////////////////////////////////////////////////////////////
int IndexList_GetListSize(
	IndexList	*List )		// list whose size we want
{

	// ensure valid data
	assert( List != NULL );
	assert( List->Size > 0 );

	// return list size
	return List->Size;

} // IndexList_GetListSize()



////////////////////////////////////////////////////////////////////////////////////////
//
//	IndexList_GetElement()
//
//	Get an elements data.
//
////////////////////////////////////////////////////////////////////////////////////////
void * IndexList_GetElement(
	IndexList	*List,		// list in which the element exists
	int			Number )	// element number
{

	// ensure valid data
	assert( List != NULL );
	assert( Number >= 0 );
	assert( Number < List->Size );

	// return the element pointer
	return List->ElementList[Number];

} // IndexList_GetElement()



////////////////////////////////////////////////////////////////////////////////////////
//
//	IndexList_DeleteElement()
//
//	Remove an elements data from the list.
//
////////////////////////////////////////////////////////////////////////////////////////
void IndexList_DeleteElement(
	IndexList	*List,		// list in which the element exists
	int			Number )	// element number
{

	// ensure valid data
	assert( List != NULL );
	assert( Number >= 0 );
	assert( Number < List->Size );

	// zap the element pointer
	List->ElementList[Number] = NULL;

} // IndexList_DeleteElement()



////////////////////////////////////////////////////////////////////////////////////////
//
//	IndexList_AddElement()
//
//	Add an elements data to the list.
//
////////////////////////////////////////////////////////////////////////////////////////
void IndexList_AddElement(
	IndexList	*List,		// list in which to add the elements data
	int			Number,		// element number
	void		*Data )		// pointer to add
{

	// ensure valid data
	assert( List != NULL );
	assert( Number >= 0 );
	assert( Number < List->Size );
	assert( Data != NULL );

	// add the elements data
	List->ElementList[Number] = Data;

} // IndexList_AddElement()



////////////////////////////////////////////////////////////////////////////////////////
//
//	IndexList_IncreaseSize()
//
//	Increase the size of the list.
//
////////////////////////////////////////////////////////////////////////////////////////
static jeBoolean IndexList_IncreaseSize(
	IndexList	*List )		// list whose size will be increased
{

	// locals
	void	**NewElementList;
	int		OldSize, NewSize;
	int		i;

	// ensure valid data
   	assert( List != NULL );
	assert( List->IncSize > 0 );

	// create new element list
	OldSize = List->Size;
	NewSize = List->Size + List->IncSize;
	NewElementList = jeRam_Allocate( sizeof( *NewElementList ) * NewSize );
	if ( NewElementList == NULL )
	{
		return JE_FALSE;
	}
	memset( NewElementList, 0, sizeof( *NewElementList ) * NewSize );

	// copy old list into new one
	for ( i = 0; i < OldSize; i++ )
	{
		NewElementList[i] = List->ElementList[i];
	}
	jeRam_Free( List->ElementList );
	List->ElementList = NewElementList;
	List->Size = NewSize;

	// all done
	return JE_TRUE;

} // IndexList_IncreaseSize()



////////////////////////////////////////////////////////////////////////////////////////
//
//	IndexList_GetEmptySlot()
//
//	Get an empty slot index in the list.
//
////////////////////////////////////////////////////////////////////////////////////////
int IndexList_GetEmptySlot(
	IndexList	*List )		// list in which we want an empyt slot
{

	// locals
	int	Slot;

	// ensure valid data
	assert( List != NULL );
	assert( List->Size > 0 );

	// search for an empty slot
	for ( Slot = 0; Slot < List->Size; Slot++ )
	{
		if ( List->ElementList[Slot] == NULL )
		{
			break;
		}
	}

	// if there was no empty slot then increase the list
	if ( Slot == List->Size )
	{

		// locals
		jeBoolean	Result;

		// fail if its a non increasing list
		assert( List->IncSize >= 0 );
		if ( List->IncSize == 0 )
		{
			return -1;
		}

		// increase the list size
		Slot = List->Size;
		Result = IndexList_IncreaseSize( List );
		if ( Result == JE_FALSE )
		{
			return -1;
		}
	}

	// return slot number
	return Slot;

} // IndexList_GetEmptySlot()



////////////////////////////////////////////////////////////////////////////////////////
//
//	IndexList_Destroy()
//
//	Destroy a list.
//
////////////////////////////////////////////////////////////////////////////////////////
void IndexList_Destroy(
	IndexList	**List )	// list to destroy
{

	// locals
	IndexList	*DeadList;

	// ensure valid data
	assert( List != NULL );
	assert( *List != NULL );

	// setup list pointer
	DeadList = *List;

	// free the element list
	if ( DeadList->ElementList != NULL )
	{
		jeRam_Free( DeadList->ElementList );
		DeadList->ElementList = NULL;
	}

	// free the struct
	jeRam_Free( DeadList );

	// zap pointer
	*List = NULL;

} // IndexList_Destroy()



////////////////////////////////////////////////////////////////////////////////////////
//
//	IndexList_Create()
//
//	Create a new list.
//
////////////////////////////////////////////////////////////////////////////////////////
IndexList * IndexList_Create(
	int	StartSize,	// starting size of the list
	int	IncSize )	// how much to enlarge the list when it gets full
{

	// locals
	IndexList	*List;

	// ensure valid data
	assert( StartSize > 0 );
	assert( IncSize >= 0 );

	// create main struct
	List = jeRam_Allocate( sizeof( *List ) );
	if ( List == NULL )
	{
		return NULL;
	}
	memset( List, 0, sizeof( *List ) );

	// create element list
	List->Size = StartSize;
	List->IncSize = IncSize;
	List->ElementList = jeRam_Allocate( sizeof( *( List->ElementList ) ) * List->Size );
	if ( List->ElementList == NULL )
	{
		jeRam_Free( List );
		return NULL;
	}
	memset( List->ElementList, 0, sizeof( *( List->ElementList ) ) * List->Size );

	// all done
	return List;

} // IndexList_Create()
