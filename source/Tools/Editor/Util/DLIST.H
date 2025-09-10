/****************************************************************************************/
/*  DLIST.H                                                                             */
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
#ifndef DLINKLST3
#define DLINKLST3
#define xxSELF_TEST

#include "BaseType.h"

typedef struct DList_NodeS	
{
	struct DList_NodeS* Prev;
	struct DList_NodeS* Next;
	char Data[1];
} DList_NodeS, *DList_NodeP;

typedef struct 
{
	DList_NodeP Head;
	DList_NodeP Tail;
	DList_NodeP NodeArray;
	DList_NodeP FreeNode;
	int32 DataSize;
	int32 NodeN;
}	DListS, *DListP;

typedef int32 DList_NodeHandle;

#define DLIST_HEAD 1
#define DLIST_TAIL 2

//NodeData is the data in the node being searched
//CompData is the data that is being compared
//the routine should return 0 if the data is equal
//the routine should return no zero for not equal
//the return is done this way so that the same routine could be
//used for a sort.
typedef int32 (*DList_SearchCB)( void* NodeData, void* CompData );

//NodeData is the data to be destroyed.
typedef void (*DList_DestroyCB)( void* NodeData, void* Context );

// itemN is the max number of nodes that can be in a list.
// if itemN is specified the nodes are preallocated.
// if itemN is 0 nodes are created from the heap as needed.
// data size is size of the data stored in a node.  If you
// plan just to store pointer to other data pass sizeof( void* )
DListP DList_Create( int32 itemN, int32 dataSize );

//Returns if any nodes have  been added to the list
int32 DList_NodeN( DListP List );

//Inerts a node before the given NodeHandle.
// NodeHandle DLIST_HEAD will make the new node the head of the list
jeBoolean DList_PreInsert( DListP List, DList_NodeHandle Node, void* Data );

//Inerts a node after the given NodeHandle.
// NodeHandle DLIST_TAIL will make the new node tail of the list
jeBoolean DList_Insert( DListP List, DList_NodeHandle Node, void* Data );

//	Gets the data of the specified NodeHandle
// NodeHandle will be filled with a specific NodeHandle when
// DLIST_HEAD and DLIST_TAIL are specified.  NULL can be passed
// for the Data when not interseted in the Data and just want 
// the NodeHandle
jeBoolean DList_Get( DListP List, DList_NodeHandle *Node, void* Data );

//	Sets the data of the specified NodeHandle
// NodeHandle will be filled with a specific NodeHandle when
// DLIST_HEAD and DLIST_TAIL are specified.  NULL can be passed
// for the Data when not interseted in the Data and just want 
// the NodeHandle
jeBoolean DList_Set( DListP List, DList_NodeHandle *Node, void* Data );

//	Gets the data of the Node following the specified NodeHandle.
//  If *Node is NULL the List Head will be returned.
// NodeHandle will be filled with a next NodeHandle.  NULL can be 
// passed for the Data when not interseted in the Data and just want 
// the NodeHandle.  The function will return FALSE if the Tail Node is passed in.
jeBoolean DList_GetNext( DListP List, DList_NodeHandle* Node, void* Data );

//	Gets the data of the Node before the specified NodeHandle
//  If *Node is NULL the List Tail will be returned.
// NodeHandle will be filled with a next NodeHandle when  NULL can be 
// passed for the Data when not interseted in the Data and just want 
// the NodeHandle. The function will return FALSE if the Head Node is passed in.
jeBoolean DList_GetPrev( DListP List, DList_NodeHandle* Node, void* Data );

//  Unlinks and destroys the Node specified by NodeHandle.
//	The Node Data is copied into Data before being destroying its copy.
//  Pass a callback function if Node version of the Data needs to 
//	be processed before being destroyed.  
//	Pass NULL for call back if no added processing is needed.
jeBoolean DList_Unlink( DListP List, DList_NodeHandle Node, void* Data, DList_DestroyCB CallBack, void* Context );

// Parses through the List Calling the CallBack until it returns 0 meaning data is 
// equal to. Returns TRUE and sets the NodeHandle.  The value of data is not changed
// since if it returns TRUE it assumed the contents of Data are the same as the Node.
jeBoolean DList_Search( DListP List, DList_NodeHandle* Node, void* Data, DList_SearchCB CallBack );

// Destroys entire list.  The Destroy Callback will be called for each 
// Nodes Data.  Pass NULL for CallBack if no addtional processing is needed.
void DList_Destroy( DListP *List, DList_DestroyCB CallBack, void* Context );

#endif //DLINKLST3
