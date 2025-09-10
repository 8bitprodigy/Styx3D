/****************************************************************************************/
/*  MATRIDX.C                                                                           */
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
#include "bitmap.h"
#include "jeMaterial.h"
#include <string.h>
#include "assert.h"
#include "errorlog.h"
#include "ram.h"
#include "util.h"

/* This structure contains the binding of the jeBitmaps to the editable bmps */
typedef struct MatrIdx_Struct {
	char					*	Name;
	jeMaterial_ArrayIndex		MaterialIndex;
	int32						RefCnt;
} MatrIdx_Struct;


// Creates a new MatrIdx structure, addes pBitmap to array and initializes structure
MatrIdx_Struct *MatrIdx_Create( jeMaterial_Array * pMatlArray, jeBitmap * pBitmap, const char * Name )
{
	MatrIdx_Struct *pMatrIdx;

	pMatrIdx = JE_RAM_ALLOCATE_STRUCT( MatrIdx_Struct );
	if( pMatrIdx == NULL )
	{
		jeErrorLog_Add( JE_ERR_MEMORY_RESOURCE, "Failed allocate MatrIdx" );
		return( NULL );
	}
	pMatrIdx->Name = Util_StrDup( Name );
	if( pMatrIdx->Name == NULL )
	{
		jeRam_Free( pMatrIdx );
		jeErrorLog_Add( JE_ERR_MEMORY_RESOURCE, "Failed allocate MatrIdx->Name");
		return( NULL );
	}


	pMatrIdx->MaterialIndex =  jeMaterial_ArrayCreateMaterial(pMatlArray, Name );
	if( pMatrIdx->MaterialIndex  == JE_MATERIAL_ARRAY_NULL_INDEX )
	{
		jeRam_Free( pMatrIdx->Name );
		jeRam_Free( pMatrIdx );
		jeErrorLog_Add( JE_ERR_INTERNAL_RESOURCE, "Failed to create Material to array.");
		return( NULL );
	}

	if( !jeMaterial_ArraySetMaterialBitmap( pMatlArray, pMatrIdx->MaterialIndex, pBitmap, Name ) )
	{
		jeRam_Free( pMatrIdx->Name );
		jeRam_Free( pMatrIdx );
		jeErrorLog_Add( JE_ERR_INTERNAL_RESOURCE, "Failed to add Material to array.");
		return( NULL );
	}
	pMatrIdx->RefCnt = 1;
	return( pMatrIdx );
}

const char* MatrIdx_GetName( MatrIdx_Struct* MatrIdx )
{
	assert( MatrIdx );
	assert( MatrIdx->Name );
	return( MatrIdx->Name );
}

const jeBitmap	*	MatrIdx_GetBitmap( jeMaterial_Array * pMatlArray, MatrIdx_Struct* MatrIdx )
{
	const jeMaterial * Material;

	assert( MatrIdx );

	Material = jeMaterial_ArrayGetMaterialByIndex( pMatlArray, MatrIdx->MaterialIndex );
	if( Material ==  NULL )
	{
		jeErrorLog_Add( JE_ERR_INTERNAL_RESOURCE, "Failed to add Material to array.");
		return( NULL );
	}
	return( jeMaterial_GetBitmap( Material ) );

}

void MatrIdx_AddRef( MatrIdx_Struct* MatrIdx )
{
	assert( MatrIdx );

	MatrIdx->RefCnt++;
}

jeMaterial_ArrayIndex	MatrIdx_GetIndex( MatrIdx_Struct* pMatrIdx )
{
	return( pMatrIdx->MaterialIndex );
}

//returns JE_TRUE if object was truly destroyed
//returns JE_FALSE if only RefCnt was decremented
jeBoolean MatrIdx_Destroy( MatrIdx_Struct** hMatrIdx )
{
	assert( hMatrIdx );

	
	assert( (*hMatrIdx )->RefCnt > 0 );

	(*hMatrIdx )->RefCnt--;
	if( (*hMatrIdx )->RefCnt > 0 )
		return( JE_FALSE );

	if( (*hMatrIdx )->Name != NULL )
		jeRam_Free( (*hMatrIdx )->Name );


	jeRam_Free( (*hMatrIdx ) );
	return( JE_TRUE );
}
	

/* EOF: MatrIdx.c */
