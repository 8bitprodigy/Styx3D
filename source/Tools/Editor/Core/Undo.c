/****************************************************************************************/
/*  UNDO.C                                                                              */
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
#include <Assert.h>
#include <Memory.h>

#include "Ram.h"
#include "errorlog.h"
#include "Undo.h"
#include "jeList.h"

#define SIGNATURE (0x01233210)
#define INVALID_TRANSACTION -1
#define UNDO_STRING_START 1024

typedef struct tagUndoSubTransaction
{
	UNDO_FUNCTIONS	Function ;
	Object	*	pObject;
	void	*	Context;
} UndoSubTransaction;

typedef struct tagUndoTransaction
{
	UNDO_TYPES	Type ;
	List	*	SubTransactions;
} UndoTransaction ;

typedef struct tagUndoCallbacks
{
	UNDO_FUNCTIONS			Function;
	Undo_RestoreCB			RestoreCB;
	Undo_DestroyContextCB	DestroyContextCB;
} UndoCallbacks;

typedef struct tagUndo
{
#ifdef _DEBUG
	int						nSignature ;
#endif
	UndoTransaction **pUndoStack;
	int32 nLevels;
	int32 StackTop;
	int32 StackBottom;
	UndoCallbacks CallbackArray[ UNDO_LAST_CALLBACK ];
	jeBoolean bBrushLighting;  //Temporary holder for callback
} Undo ;

// Undo transactions:
//	Add - Have pointer to new thing
//	Modify	- Own Pointer to original things and have pointer to new thing
//	Delete	- Own Pointer to deleted things



Undo * Undo_Create( const int32 nLevels )
{
	Undo * pUndo ;
	
	pUndo = JE_RAM_ALLOCATE_STRUCT( Undo ) ;
	if( pUndo == NULL )
	{
		jeErrorLog_Add( JE_ERR_MEMORY_RESOURCE, "Uanble to create undo" );
		goto UC_FAILURE ;
	}
	memset( pUndo, 0, sizeof *pUndo ) ;
	assert( (pUndo->nSignature = SIGNATURE) == SIGNATURE ) ;	// ASSIGN
	pUndo->pUndoStack = JE_RAM_ALLOCATE_ARRAY( UndoTransaction*, nLevels );
	if( pUndo == NULL )
	{
		jeErrorLog_Add( JE_ERR_MEMORY_RESOURCE, "Uanble to create undo stac" );
		goto UC_FAILURE ;
	}
	memset( pUndo->pUndoStack, 0, nLevels * sizeof(UndoTransaction*) ) ;

	pUndo->nLevels = nLevels;
	pUndo->StackTop = INVALID_TRANSACTION;
	pUndo->StackBottom = 0;

	return pUndo ;

UC_FAILURE :
	if( pUndo != NULL )
		Undo_Destroy( &pUndo ) ;
	return NULL ;
}//Undo_Create

static void Undo_DeleteSubTransactionCB( void* Data )
{
	UndoSubTransaction *pSubTransaction = (UndoSubTransaction*)Data;

	Object_Free( &pSubTransaction->pObject );
	jeRam_Free( pSubTransaction );
}

static jeBoolean Undo_DestroyContext(void *pData, void *lParam)
{
	UndoSubTransaction	*	pSubTransaction = (UndoSubTransaction*)pData;
	Undo				*	pUndo			= (Undo*)lParam;
	Undo_DestroyContextCB	DestroyContextCB;

	assert( pUndo );
	assert( pSubTransaction );
	assert( pUndo->CallbackArray[ pSubTransaction->Function ].Function == pSubTransaction->Function );
	assert( pUndo->CallbackArray[ pSubTransaction->Function ].DestroyContextCB );
	DestroyContextCB = pUndo->CallbackArray[ pSubTransaction->Function ].DestroyContextCB;
	(*DestroyContextCB)( pSubTransaction->Context );
	pSubTransaction->Context = NULL;
	return( JE_TRUE );
}

void Undo_DeleteTransaction( Undo* pUndo, int32 TransIdx )
{
	UndoTransaction *pTransaction;
	
	pTransaction = pUndo->pUndoStack[TransIdx ];
	List_ForEach( pTransaction->SubTransactions, Undo_DestroyContext, pUndo );
	List_Destroy (&pTransaction->SubTransactions, Undo_DeleteSubTransactionCB );
	jeRam_Free( pTransaction );
	pUndo->pUndoStack[TransIdx ] = NULL;

}

// Creates a new Transaction and puts it on the top of the stack
// If list is full the  bottom transaction is deleted to make room.
jeBoolean Undo_Push( Undo *pUndo, UNDO_TYPES Type )
{

	UndoTransaction *pTransaction;

	assert( pUndo );


	pTransaction = JE_RAM_ALLOCATE_STRUCT( UndoTransaction ) ;
	if( pUndo == NULL )
	{
		jeErrorLog_Add( JE_ERR_MEMORY_RESOURCE, "Uanble to create UndoTransaction" );
		return( JE_FALSE ) ;
	}
	pTransaction->Type = Type;
	pTransaction->SubTransactions = List_Create();
	if( pTransaction->SubTransactions == NULL )
	{
		jeRam_Free( pTransaction );
		return( JE_FALSE ) ;
	}
	if( pUndo->StackTop == INVALID_TRANSACTION )
		pUndo->StackTop = pUndo->StackBottom;
	else
		pUndo->StackTop++;
	if( pUndo->StackTop == pUndo->nLevels )
		pUndo->StackTop = 0;
	if( pUndo->StackTop == pUndo->StackBottom )
	{
		if( pUndo->pUndoStack[pUndo->StackBottom] != NULL )
		{
			Undo_DeleteTransaction( pUndo, pUndo->StackBottom );
			pUndo->StackBottom++;
			if( pUndo->StackBottom == pUndo->nLevels )
				pUndo->StackBottom = 0;
		}
	}

	pUndo->pUndoStack[pUndo->StackTop] = pTransaction;
	return( JE_TRUE );
}

static jeBoolean Undo_RestoreListCB(void *pData, void *lParam)
{
	UndoSubTransaction* pSubTransaction;
	Undo	* pUndo;
	Undo_RestoreCB	RestoreCB;

	pUndo = (Undo*)lParam;
	pSubTransaction = (UndoSubTransaction*)pData;

	assert( pUndo->CallbackArray[pSubTransaction->Function].Function == pSubTransaction->Function );
	RestoreCB = pUndo->CallbackArray[pSubTransaction->Function].RestoreCB;
	if( !(*RestoreCB)( pSubTransaction->pObject, pSubTransaction->Context ) )
		return( JE_FALSE );
	Object_Update( pSubTransaction->pObject, pUndo->bBrushLighting, JE_FALSE );
	return( JE_TRUE );
}

// Undo_Pop
//Removes top Transaction and calls restore routine for all of its sub-transactions
jeBoolean	Undo_Pop( Undo *pUndo, jeBoolean bBrushLighting )
{
	UndoTransaction *pTransaction;

	assert( pUndo );
	assert( pUndo->pUndoStack );
	assert( pUndo->nLevels > 0 );


	if( pUndo->StackTop == INVALID_TRANSACTION )
		return( JE_FALSE );
	pTransaction = pUndo->pUndoStack[pUndo->StackTop];
	assert( pTransaction ); 

	pUndo->bBrushLighting = bBrushLighting;
	List_ForEach( pTransaction->SubTransactions, Undo_RestoreListCB, pUndo );
	Undo_DeleteTransaction( pUndo, pUndo->StackTop );
	if( pUndo->StackTop == pUndo->StackBottom )
	{
		pUndo->StackTop = INVALID_TRANSACTION;
	}
	else
	{
		pUndo->StackTop--;
		if( pUndo->StackTop < 0 )
			pUndo->StackTop = pUndo->nLevels - 1;
	}


	return( JE_TRUE );
}

UNDO_TYPES	Undo_GetTopType( Undo *pUndo )
{
	UndoTransaction *pTransaction;

	assert( pUndo );
	assert( pUndo->pUndoStack );
	assert( pUndo->nLevels > 0 );


	if( pUndo->StackTop == INVALID_TRANSACTION )
		return( JE_FALSE );
	pTransaction = pUndo->pUndoStack[pUndo->StackTop];
	assert( pTransaction ); 
	return( pTransaction->Type );
}

//Adds a sub-transaction to top Transaction
//Creates a Ref to the Object and frees it when sub-transaction is deleted
//Context is assumed to be an allocated block of memory owned by the sub-transaction
//this block will be freed when the sub-transaction is deleted.  Context may be NULL
//in which case it will be ignored.
jeBoolean Undo_AddSubTransaction( Undo *pUndo, UNDO_FUNCTIONS Function, Object * pObject, void *Context )
{
	UndoTransaction *pTransaction;
	UndoSubTransaction* pSubTransaction;

	assert( pUndo );
	assert( pObject );

	if( pUndo->StackTop == INVALID_TRANSACTION )
		return( JE_FALSE );
	pTransaction = pUndo->pUndoStack[pUndo->StackTop];
	assert( pTransaction ); 

	pSubTransaction = JE_RAM_ALLOCATE_STRUCT( UndoSubTransaction ) ;
	if( pSubTransaction == NULL )
		return( JE_FALSE );
	Object_AddRef( pObject );
	pSubTransaction->Function = Function;
	pSubTransaction->pObject = pObject;
	pSubTransaction->Context = Context;
	List_Append (pTransaction->SubTransactions, pSubTransaction);
	return( JE_TRUE );
	
}

jeBoolean	Undo_CanUndo( Undo *pUndo, int32* UndoStringID )
{
	UndoTransaction *pTransaction;

	assert( pUndo );
	assert( UndoStringID );
	if( pUndo->StackTop == INVALID_TRANSACTION )
	{
			*UndoStringID = UNDO_NONE + UNDO_STRING_START;
			return(JE_FALSE );
	}
	
	assert( pUndo->pUndoStack[pUndo->StackTop] );
	pTransaction = pUndo->pUndoStack[pUndo->StackTop];
	*UndoStringID = pTransaction->Type + UNDO_STRING_START;
	return( JE_TRUE );
}

// Undo_RegisterCallBack
// Fills the Callback array for the corrisponding function
void	Undo_RegisterCallBack( Undo *pUndo, UNDO_FUNCTIONS Function, Undo_RestoreCB RestoreCB, Undo_DestroyContextCB DestoyContextCB )
{
	pUndo->CallbackArray[Function].Function = Function;
	pUndo->CallbackArray[Function].RestoreCB = RestoreCB;
	pUndo->CallbackArray[Function].DestroyContextCB = DestoyContextCB;
}

void Undo_Reset( Undo *pUndo )
{
	int i;
	assert( pUndo != NULL ) ;
	assert( pUndo->nSignature == SIGNATURE ) ;
	if( pUndo->pUndoStack != NULL )
	{
		for( i = 0 ; i < pUndo->nLevels; i++ )
		{
			if( pUndo->pUndoStack[i] != NULL )
				Undo_DeleteTransaction( pUndo, i );
		}
		pUndo->StackBottom = 0;
		pUndo->StackTop = INVALID_TRANSACTION;
	}
}

void Undo_Destroy( Undo ** ppUndo )
{
	Undo * pUndo ;
	int i;

	assert( ppUndo != NULL ) ;
	pUndo = *ppUndo ;
	assert( pUndo->nSignature == SIGNATURE ) ;

	assert( (pUndo->nSignature = 0) == 0 ) ;	// CLEAR
	if( pUndo->pUndoStack != NULL )
	{
		for( i = 0 ; i < pUndo->nLevels; i++ )
		{
			if( pUndo->pUndoStack[i] != NULL )
				Undo_DeleteTransaction( pUndo, i );
		}
		jeRam_Free( pUndo->pUndoStack );
	}

	jeRam_Free( pUndo ) ;
}// Undo_Destroy