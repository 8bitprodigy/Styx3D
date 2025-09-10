/****************************************************************************************/
/*  UNDO.H                                                                              */
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

/*************
  Undo is a Stack of Transactions.  The stack will hold only as many transactions
  as speciefied by nLevels.  If more nLevels transactions are added the bottom of
  the stack is deleted so the new transaction can be added.

  Pushing a new Transactions.
  AddSubTransaction adds a sub-transaction the Transaction at the top of the stack
  Poping removes the top Transactions and calls the restore function for each of the 
  sub-transaction in the reverse order the  sub-transaction were added.

**************/
#pragma once

#ifndef UNDO_H
#define UNDO_H

#include "BaseType.h"
#include "jwObject.h"

#ifdef __cplusplus
extern "C" {
#endif

#define UNDO_MAX_STRING 32
typedef enum
{
	UNDO_UNDEFINED,
	UNDO_TRANSFORM,
	UNDO_DELETEOBJECT,
	UNDO_CREATEOBJECT,
	UNDO_APPLYTEXTURE,
	UNDO_BRUSHSHEAR,
	UNDO_LAST_CALLBACK
} UNDO_FUNCTIONS;

typedef enum 
{
	UNDO_MOVE,
	UNDO_ROTATE,
	UNDO_RESIZE,
	UNDO_DELETE,
	UNDO_APPLYMATERIAL,
	UNDO_NONE,
	UNDO_CREATE,
	UNDO_SHEAR
}	UNDO_TYPES;


typedef jeBoolean	(*Undo_RestoreCB)(Object *pObject, void *Context );
typedef void		(*Undo_DestroyContextCB)( void *Context);

typedef struct tagUndo	Undo ;

Undo *				Undo_Create( const int32 nLevels ) ;

// Creates a new Transaction and puts it on the top of the stack
// If list is full the  bottom transaction is deleted to make room.
jeBoolean			Undo_Push( Undo *pUndo, UNDO_TYPES Function );

//Removes top Transaction and calls restore routine for all of its sub-transactions
jeBoolean			Undo_Pop( Undo *pUndo, jeBoolean bBrushLighting );

//Returns the type of top Transaction
UNDO_TYPES			Undo_GetTopType( Undo *pUndo );

//Returns TRUE if there is a valid Undo in the stack
//UndoStringID is an ID of res string describing the undo
//UndoStringID will be valid even on a FALSE ( Containing the id for string "None" )
jeBoolean			Undo_CanUndo( Undo *pUndo, int32* UndoStringID );

//Adds a sub-transaction to top Transaction
//Creates a Ref to the Object and frees it when sub-transaction is deleted
//Context is assumed to be an allocated block of memory owned by the sub-transaction
//this block will be freed when the sub-transaction is deleted.
jeBoolean			Undo_AddSubTransaction( Undo *pUndo, UNDO_FUNCTIONS Function, Object * pObject, void *Context );

void				Undo_RegisterCallBack( Undo *pUndo, 
										   UNDO_FUNCTIONS Function, 
										   Undo_RestoreCB RestoreCB, 
										   Undo_DestroyContextCB DestoyContextCB );

void			Undo_Reset( Undo *pUndo );
void			Undo_Destroy( Undo ** ppUndo ) ;


#ifdef __cplusplus
}
#endif

#endif // Prevent multiple inclusion

/* EOF: Undo.h */
