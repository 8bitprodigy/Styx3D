/****************************************************************************************/
/*  MATRARRAY.C                                                                         */
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
#include "vfile.h"
#include "jelist.h"
#include <string.h>
#include "assert.h"
#include "errorlog.h"
#include "ram.h"
#include "util.h"
#include "MatrArray.h"

typedef struct MatrArray_Struct {
	List* pList;
	jeMaterial_Array	*	pMatlArray;
	MatrIdx_Struct		*	CurMatrIdx;
}	MatrArray_Struct;



//Creates a material list.  
MatrArray_Struct* MatrArray_Create( jeMaterial_Array * pMatlArray )
{
	MatrArray_Struct* MatrArray;

	MatrArray = JE_RAM_ALLOCATE_STRUCT( MatrArray_Struct );
	if( MatrArray == NULL )
	{
		jeErrorLog_Add( JE_ERR_MEMORY_RESOURCE, NULL );
		return( NULL );
	}
	MatrArray->pList = List_Create ();
	if( MatrArray->pList == NULL )
		return( NULL );
	MatrArray->CurMatrIdx = NULL;
	MatrArray->pMatlArray = pMatlArray;

	return( MatrArray );
}

void MatrArray_Destroy( MatrArray_Struct **hMatrArray )
{
	if( (*hMatrArray)->pList != NULL )
		List_Destroy ( &(*hMatrArray)->pList, (List_DestroyCallback) MatrIdx_Destroy );

	jeRam_Free( (*hMatrArray) );

}

MatrIdx_Struct *	MatrArray_GetMatrIdx( MatrArray_Struct * MatrArray, MatrIdxIterator  pMI )
{
	assert( pMI );
	assert( MatrArray );
	assert( MatrArray->pList );

	return( (MatrIdx_Struct *)List_GetData( pMI ) );
}

MatrIdx_Struct *	MatrArray_GetFirstMatrIdx( MatrArray_Struct * MatrArray, MatrIdxIterator * pMI )
{
	assert( pMI );
	assert( MatrArray );
	assert( MatrArray->pList );

	return( (MatrIdx_Struct *)List_GetFirst( MatrArray->pList, pMI ) );
}

MatrIdx_Struct *	MatrArray_GetNextMatrIdx( MatrArray_Struct * MatrArray, MatrIdxIterator * pMI )
{
	assert( pMI );
	assert( MatrArray );
	assert( MatrArray->pList );

	return( (MatrIdx_Struct *)List_GetNext( MatrArray->pList, pMI ) );
}


MatrIdx_Struct*	MatrArray_GetCurMatrIdx( MatrArray_Struct* MatrArray )
{
	assert( MatrArray );

	return( MatrArray->CurMatrIdx );
}

void MatrArray_SetCurMatrIdx( MatrArray_Struct* MatrArray, MatrIdx_Struct* MatrIdx )
{
	assert( MatrArray );
	assert( MatrIdx );

	MatrArray->CurMatrIdx = MatrIdx;
}

MatrIdx_Struct * MatrArray_Add( MatrArray_Struct * pMatrArray, jeBitmap * pBitmap, const char * Name )
{
	MatrIdx_Struct* pMatrIdx;


	pMatrIdx = MatrIdx_Create( pMatrArray->pMatlArray, pBitmap, Name );
	if( pMatrIdx == NULL )
	{
		jeErrorLog_Add( JE_ERR_INTERNAL_RESOURCE, "Trace" );
		return( NULL );
	}
	List_Append ( pMatrArray->pList, pMatrIdx );
	
	return( pMatrIdx );
}

static jeBoolean MatrArray_SearchNameCB(void *pData, void *lParam)
{
	MatrIdx_Struct* MatrIdx = (MatrIdx_Struct*)pData;
	char* Name = lParam;

	assert( pData );
	assert( lParam );

	return( !strcmp( MatrIdx_GetName( MatrIdx ) , Name ) );
} 

MatrIdx_Struct*	MatrArray_SearchByName( MatrArray_Struct* MatrArray, MatrIdxIterator * pMI, const char* Name )
{
	MatrIdx_Struct* MatrIdx;
	
	assert( MatrArray );
	assert( pMI );
	assert( Name );

	if( !List_Search ( MatrArray->pList, 
						MatrArray_SearchNameCB, 
						(void*)Name, &MatrIdx, pMI ) )
		return( NULL );
	return( MatrIdx );
}
/* EOF: MatrArray.c */
