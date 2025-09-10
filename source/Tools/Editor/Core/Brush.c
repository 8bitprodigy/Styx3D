/****************************************************************************************/
/*  BRUSH.C                                                                             */
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
#include <Float.h>
#include <Memory.h>
#include <String.h>
#include "../Resource.h"

#include "ErrorLog.h"
#include "jet.h"
#include "Ram.h"
#include "Transform.h"
#include "Util.h"
#include "FaceList.h"
#include "Units.h"

#include "Brush.h"
#include "ObjectDef.h"
#include "AppData.h"

#define SIGNATURE			(0x36925814)
#define BRUSH_MAXNAMELENGTH	(31)

#define BRUSH_FLAG_BOUNDSDIRTY	(2)
#define BRUSH_DRAW_MASK			(JE_BSP_CONTENTS_AIR | JE_BSP_CONTENTS_EMPTY | JE_BSP_CONTENTS_SOLID)

typedef enum
{
	BRUSH_GLOBAL_UPDATEGROUP_ID = PROPERTY_LOCAL_DATATYPE_START,
	BRUSH_GLOBAL_UPDATE_MANUEL_ID,
	BRUSH_GLOBAL_UPDATE_CHANGE_ID,
	BRUSH_GLOBAL_UPDATE_REALTIME_ID,
	BRUSH_GLOBAL_UPDATEGROUP_END_ID,
	BRUSH_GLOBAL_MAINTAIN_LIGHING_ID
};

static int gBrush_Update = OBJECT_UPDATE_MANUEL;
static int gBrush_Lighting = JE_TRUE;

char  **VisPortalArray = NULL;
int32 VisPortalN;
int32 VisPortalAllocateN = 0;
char  VisPortalNone[] = "None";
#define	VISPORTALBLOCK 10
#define VIS_PORTAL_TYPE "Portal"

typedef struct tagBrush
{
	Object		ObjectData ;
	uint32		Flags ;
#ifdef _DEBUG
	int			nSignature ;
#endif
	BRUSH_KIND	Kind ;
	FaceList *  pSelFaces ;		//List of selected faces of this brush
	VertList *  pSelVert ;		//List of selected verts of this brush
	jeExtBox	WorldBounds ;
	jeBrush	*	pBrush ;
	Model	*	pModel ;
	jeWorld *	pWorld;
	jeBoolean	bLockTextures;
	BrushTemplate *pTemplate;
	jeBoolean	bShow;
//  BRUSH ID
} Brush ;

typedef struct tagMaterialUndoContext
{
	jeBrush_Face	* pFace;
	jeModel * pgeModel;
	jeMaterial_ArrayIndex MaterialIndex;
} MaterialUndoContext;

jeBrush * Brush_CopygeBrush( const Brush * pBrush )
{
	int32				i ;
	jeBrush			*	pgeBrush ;
	int32				nFaces ;
	int32				nVerts ;
	jeBrush_Face	*	pFace = NULL ;
	jeBrush_Face	*	pOrgFace ;
	jeXForm3d			XForm ;
	const jeVec3d	*	pVert ;
	jeFaceInfo			FaceInfo;

	assert( pBrush != NULL ) ;
	assert( SIGNATURE == pBrush->nSignature ) ;
	assert( pBrush->pBrush != NULL ) ;
	
	nFaces = Brush_GetFaceCount( pBrush ) ;
	pgeBrush = jeBrush_Create( nFaces ) ;
	if( pgeBrush == NULL )
	{
		jeErrorLog_Add( JE_ERR_SUBSYSTEM_FAILURE, "Brush_CopygeBrush:jeBrush_Create");
		goto BCB_FAILURE ;
	}

	pOrgFace = Brush_GetNextFace( pBrush, NULL ) ;
	while( pOrgFace != NULL )
	{
		pFace = jeBrush_CreateFace( pgeBrush, Brush_FaceGetVertCount( pOrgFace ) ) ;
		if( pFace == NULL )
			goto BCB_FAILURE ;
		
		nVerts = Brush_FaceGetVertCount( pOrgFace ) ;
		for( i=0; i<nVerts; i++ )
		{
			pVert = jeBrush_FaceGetVertByIndex( pOrgFace, i ) ;
			jeBrush_FaceSetVertByIndex( pFace, i, pVert ) ;
		}
		if( !jeBrush_FaceGetFaceInfo( pOrgFace, &FaceInfo ) )
		{
			jeErrorLog_Add( JE_ERR_SUBSYSTEM_FAILURE, "Brush_CopygeBrush:jeBrush_FaceGetFaceInfo");
			goto BCB_FAILURE ;
		}
		if( !jeBrush_FaceSetFaceInfo( pFace, &FaceInfo ) )
		{
			jeErrorLog_Add( JE_ERR_SUBSYSTEM_FAILURE, "Brush_CopygeBrush:jeBrush_FaceGetFaceInfo");
			goto BCB_FAILURE ;
		}

		pOrgFace = Brush_GetNextFace( pBrush, pOrgFace ) ;
	} 

	Brush_GetXForm( pBrush, &XForm );
	jeBrush_SetXForm( pgeBrush, &XForm, pBrush->bLockTextures ) ;

	return pgeBrush ;

BCB_FAILURE :
	if( pgeBrush != NULL )
		jeBrush_Destroy( &pgeBrush ) ;

	return NULL ;

}// Brush_CopygeBrush


static void Brush_RebuildBounds( FaceVertInfo * pfvi, void * lParam )
{
	const jeVec3d * pVert ;
	jeVec3d			Vert ;
	jeXForm3d		XForm ;
	Brush	*	pBrush = (Brush*) lParam ;

	assert( pfvi );
	assert( pfvi->pFace );
	assert( pBrush );
	assert( SIGNATURE == pBrush->nSignature ) ;

	pVert = Brush_FaceGetVertByIndex( pfvi->pFace, pfvi->nVert ) ;
	Brush_GetXForm( pBrush, &XForm ) ;
	jeXForm3d_Transform( &XForm, pVert, &Vert ) ;
	Util_geExtBox_ExtendToEnclose( &pBrush->WorldBounds, &Vert ) ;
}// Brush_TransformFaceVerts



static void Brush_SetContents( Brush * pBrush, BRUSH_TYPE eAddType )
{
	jeBrush_Contents	Contents ;
	assert( pBrush != NULL ) ;
	assert( SIGNATURE == pBrush->nSignature ) ;

	Contents = jeBrush_GetContents( Brush_GetjeBrush( pBrush ) ) ;

	switch( eAddType )
	{
	case BRUSH_ADD :
		Contents &= ~JE_BSP_CONTENTS_AIR;
		Contents |= JE_BSP_CONTENTS_SOLID;
		break ;
	case BRUSH_SUBTRACT :
		Contents &= ~JE_BSP_CONTENTS_SOLID;
		Contents |= JE_BSP_CONTENTS_AIR;
		break ;

	default:
		assert( 0 );
	}
	jeBrush_SetContents( Brush_GetjeBrush( pBrush ), Contents ) ;

}// Brush_SetContents 

static void Brush_SizeEdge( Brush * pBrush, const jeVec3d * pStillEdge, const jeFloat fScale, ORTHO_AXIS Axis )
{
	jeVec3d		Scale ;
	jeVec3d		Temp ;
	jeVec3d		BrushWorldCenter ;
	jeXForm3d	XForm ;
	jeFloat		fTemp ;

	assert( pBrush );
	assert( pStillEdge );
	assert( SIGNATURE == pBrush->nSignature ) ;

	if( fScale == 1.0f )
		return ;

	Brush_GetXForm( pBrush, &XForm ) ;

	jeVec3d_Set( &Scale, 1.0f, 1.0f, 1.0f ) ;
	jeVec3d_SetElement( &Scale, Axis, fScale ) ;
	Temp = XForm.Translation ;	
	jeVec3d_Clear( &XForm.Translation ) ;
	jeXForm3d_Scale( &XForm, Scale.X, Scale.Y, Scale.Z ) ;
	XForm.Translation = Temp ;

	Brush_GetWorldCenter( pBrush, &BrushWorldCenter ) ;

	fTemp = jeVec3d_GetElement( &BrushWorldCenter, Axis ) - jeVec3d_GetElement( pStillEdge, Axis ) ;
	fTemp = fTemp * fScale ;
	fTemp = fTemp + jeVec3d_GetElement( pStillEdge, Axis ) ;
	jeVec3d_SetElement( &BrushWorldCenter, Axis, fTemp ) ;
	XForm.Translation = BrushWorldCenter ;

	Brush_SetXForm( pBrush, &XForm) ;

	Brush_SetModified( pBrush ) ;

	assert( Brush_IsBoundsValid( pBrush ) ) ;

}// Brush_SizeEdge


//
// END STATIC
//


Brush * Brush_Create( const char * const pszName, Group * pGroup,  int32 nNumber )
{
	Brush * pBrush ;
	assert( pszName != NULL ) ;
	assert( strlen( pszName ) < BRUSH_MAXNAMELENGTH ) ;

	pBrush = JE_RAM_ALLOCATE_STRUCT_CLEAR( Brush ) ;
	if( pBrush == NULL )
	{
		jeErrorLog_Add( JE_ERR_MEMORY_RESOURCE, "Brush_Create" );
		goto BC_FAILURE ;
	}

	assert( (pBrush->nSignature = SIGNATURE) == SIGNATURE ) ;	// ASSIGN
	
	if( !Object_Init( &pBrush->ObjectData, pGroup, KIND_BRUSH, pszName, nNumber )  )
	{
		jeErrorLog_Add( JE_ERR_INTERNAL_RESOURCE, "Brush_Create:Object_Init" );
		goto BC_FAILURE ;
	}
	pBrush->pTemplate = NULL;
	pBrush->Kind = BRUSH_INVALID ;
	pBrush->Flags = BRUSH_FLAG_BOUNDSDIRTY ;
	// Krouer: change default lock behavior -- cannot be changed
	pBrush->bLockTextures = JE_TRUE; //JE_FALSE;
	pBrush->pWorld = NULL;
	pBrush->bShow = JE_TRUE;
	pBrush->pSelFaces = FaceList_Create();
	if( pBrush->pSelFaces == NULL )
	{
		jeErrorLog_Add( JE_ERR_INTERNAL_RESOURCE, "Brush_Create:FaceList_Create" );
		goto BC_FAILURE ;
	}

	pBrush->pSelVert = VertList_Create();
	if( pBrush->pSelVert == NULL )
	{
		jeErrorLog_Add( JE_ERR_INTERNAL_RESOURCE, "Brush_Create:VertList_Create" );
		goto BC_FAILURE ;
	}

	return pBrush ;
	
BC_FAILURE :
	if( pBrush != NULL )
		Object_Free( (Object**)&pBrush ) ;
	return NULL ;
}// Brush_Create



void Brush_Destroy( Brush ** ppBrush )
{
	Brush * pBrush ;
	assert( ppBrush != NULL ) ;
	pBrush = *ppBrush ;
	assert( SIGNATURE == pBrush->nSignature ) ;

	if( pBrush->pModel != NULL )
		Model_RemoveBrush( pBrush->pModel, pBrush );
	
	if( pBrush->pBrush != NULL )
		jeBrush_Destroy( &pBrush->pBrush ) ;
	if( pBrush->pSelFaces )
		FaceList_Destroy( &pBrush->pSelFaces, NULL );

	if( pBrush->pSelVert )
		VertList_Destroy( &pBrush->pSelVert, NULL );

	// Added by Icestorm
	// [MLB-ICE]
	if( pBrush->pTemplate )
		BrushTemplate_Destroy( &(pBrush->pTemplate) );
	if( pBrush->ObjectData.pszName )
		jeRam_Free( pBrush->ObjectData.pszName );
	// [MLB-ICE] EOB

	assert( ((*ppBrush)->nSignature = 0) == 0 ) ;	// CLEAR
	pBrush->ObjectData.ObjectKind = KIND_INVALID ;
	jeRam_Free( *ppBrush ) ;

}// Brush_Destroy


Brush * Brush_Copy( const Brush * pBrush, const int32 nNumber )
{
	Brush			*	pNewBrush ;
	jeBrush_Contents	Contents ;
	jeBrush			*	pgeBrush ;

	// Brush_Copy does not add the brush to the model

	assert( pBrush != NULL ) ;
	assert( SIGNATURE == pBrush->nSignature ) ;

	pgeBrush = Brush_CopygeBrush( pBrush ) ;
	if( pgeBrush == NULL )
		return NULL ;

	pNewBrush = Brush_Create( Object_GetName( &pBrush->ObjectData ), pBrush->ObjectData.pGroup, nNumber ) ;
	if( pNewBrush == NULL )
	{
		jeBrush_Destroy( &pgeBrush ) ;
		return NULL ;
	}

	pNewBrush->pBrush = pgeBrush ;
	pNewBrush->Flags = pBrush->Flags ;
	Contents = jeBrush_GetContents( Brush_GetjeBrush( pBrush ) ) ;
	jeBrush_SetContents( Brush_GetjeBrush( pNewBrush ), Contents ) ;
	pNewBrush->pTemplate = BrushTemplate_Copy( pBrush->pTemplate );
	
	pNewBrush->pModel		= NULL ;
	pNewBrush->Flags		= pBrush->Flags ;	
	pNewBrush->Kind			= pBrush->Kind ;
	pNewBrush->WorldBounds	= pBrush->WorldBounds ;
	pNewBrush->pWorld		= pBrush->pWorld;
	pNewBrush->bShow		= pBrush->bShow;

	return pNewBrush ;
}// Brush_Copy

char *	Brush_CreateDefaultName( BRUSH_TYPE Type )
{
	switch( Type )
	{
		case BRUSH_BOX:
		return( Util_LoadLocalRcString( IDS_BRUSH_BOX ) );

		case BRUSH_SPHERE:
		return( Util_LoadLocalRcString( IDS_DEFAULTSPHERETEMPLATENAME ) );
		
		case BRUSH_CYLINDER:
		return( Util_LoadLocalRcString( IDS_BRUSH_CYLINDER ) );

		case BRUSH_SHEET:
			return( Util_LoadLocalRcString( IDS_DEFAULTSHEETTEMPLATENAME ) );

		case BRUSH_ARCH:
			return (Util_LoadLocalRcString(IDS_BRUSH_ARCH));

		default:
			assert( 0 );
	}
	return( NULL );
}

char * Brush_CreateKindName( )
{
	return( Util_LoadLocalRcString( IDS_BRUSH ) );
}


Brush * Brush_FromTemplate( BrushTemplate *pTemplate,  Group * pGroup, char * Name, int32 nNumber, jeFaceInfo * pFaceInfo, BRUSH_TYPE eAddType )
{
	Brush			*	pNewBrush ;

	pNewBrush = Brush_Create( Name, pGroup, nNumber );
	if( pNewBrush == NULL )
		goto BFT_FAILURE ;
	pNewBrush->pTemplate = pTemplate;
	pNewBrush->ObjectData.pGroup = pGroup ;
	pNewBrush->pBrush = BrushTemplate_CreateBrush( pTemplate,  pFaceInfo ); 
	if( pNewBrush->pBrush == NULL )
		goto BFT_FAILURE ;
	
	Brush_SetContents( pNewBrush, eAddType ) ;
	return pNewBrush ;

BFT_FAILURE :
	if( pNewBrush != NULL )
		Object_Free( (Object**)pNewBrush ) ;

	return NULL ;
}// Brush_FromTemplate

// ACCESSORS
int32 Brush_GetFaceCount( const Brush * pBrush )
{
	assert( pBrush != NULL ) ;
	assert( SIGNATURE == pBrush->nSignature ) ;
	assert( pBrush->pBrush != NULL ) ;

	return jeBrush_GetFaceCount( pBrush->pBrush ) ;
	
}// Brush_GetFaceCount

jeBrush * Brush_GetjeBrush( const Brush * pBrush )
{
	assert( pBrush != NULL ) ;
	assert( SIGNATURE == pBrush->nSignature ) ;
	
	return pBrush->pBrush ;

}// Brush_GetjeBrush

jeBrush_Face * Brush_GetFaceByIndex( const Brush * pBrush, int32 Index )
{
	assert( pBrush != NULL ) ;
	assert( SIGNATURE == pBrush->nSignature ) ;
	
	return jeBrush_GetFaceByIndex( pBrush->pBrush, Index ) ;

}// Brush_GetFaceByIndex


int32 Brush_GetLargestFaceVertexCount( const Brush * pBrush )
{
	int32			nCount ;
	int32			nVerts ;
	jeBrush_Face *	pFace ;
	assert( pBrush != NULL ) ;
	assert( SIGNATURE == pBrush->nSignature ) ;

	pFace = Brush_GetNextFace( pBrush, NULL ) ;
	nCount = 0 ;
	do
	{
		nVerts = jeBrush_FaceGetVertCount( pFace ) ;
		if( nVerts > nCount )
			nCount = nVerts ;
		pFace = Brush_GetNextFace( pBrush, pFace ) ;
	} while( pFace != NULL ) ;

	return nCount ;
}// Brush_GetLargestFaceVertexCount

//	Brush_GetNextSelFace
//	When no face is selected ruturns first face. 
//	When more then one face is selected on a brush the first in selection.
//	When one face is slected it returns next face.
//	Returns curently selected face is last in list.
jeBrush_Face *	Brush_GetNextSelFace( Brush * pBrush )
{
	int32 FaceCnt;
	FaceIterator  pMI;
	jeBrush_Face *pSelFace = NULL;
	assert( pBrush );
	assert( pBrush->pSelFaces );

	FaceCnt = FaceList_GetNumFace( pBrush->pSelFaces );

	if( FaceCnt == 0 )
	{
		return( jeBrush_GetNextFace( pBrush->pBrush, NULL ) );
	}
	if( FaceCnt > 1 )
	{
		return( FaceList_GetFirstFace( pBrush->pSelFaces, &pMI ) );
	}
	pSelFace = FaceList_GetFirstFace( pBrush->pSelFaces, &pMI );
	return( jeBrush_GetNextFace( pBrush->pBrush, pSelFace )  );
}// Brush_GetNextSelFace

//	Brush_GetPrevSelFace
//	When no face is selected ruturns last face. 
//	When more then one face is selected on a brush the last in selection.
//	When one face is slected it returns prev face.
//	Returns curently selected face is firt in list.
jeBrush_Face *	Brush_GetPrevSelFace( Brush * pBrush )
{
	int32 FaceCnt;
	jeBrush_Face *pLastFound = NULL;
	jeBrush_Face *Cur = NULL;
	FaceIterator  pMI = NULL;
	assert( pBrush );
	assert( pBrush->pSelFaces );

	FaceCnt = FaceList_GetNumFace( pBrush->pSelFaces );

	if( FaceCnt == 0 )
	{
		Cur = jeBrush_GetNextFace( pBrush->pBrush, pLastFound );
		while( Cur != NULL )
		{
			pLastFound = Cur;
			Cur = jeBrush_GetNextFace( pBrush->pBrush, pLastFound );
		}
		return( pLastFound );
	}
	if( FaceCnt > 1 )
	{
		Cur = FaceList_GetFirstFace( pBrush->pSelFaces, &pMI );
		while(  Cur != NULL )
		{
			pLastFound = Cur;
			Cur = FaceList_GetNextFace( pBrush->pSelFaces, &pMI );
		}
		return( pLastFound );
	}
	Cur = FaceList_GetFirstFace( pBrush->pSelFaces, &pMI );
	return( jeBrush_GetPrevFace( pBrush->pBrush, Cur) );
}// Brush_GetPrevSelFace

Model * Brush_GetModel( Brush * pBrush )
{
	assert( pBrush != NULL ) ;
	assert( SIGNATURE == pBrush->nSignature ) ;

	return pBrush->pModel ;
}// Brush_GetModel


void Brush_GetVertexPoint( Brush * pBrush, jeVertArray_Index Index, jeVec3d * pVert )
{
	jeXForm3d			XForm ;

	assert( pBrush != NULL ) ;
	assert( SIGNATURE == pBrush->nSignature ) ;
	assert( pVert != NULL ) ;

	*pVert = *jeVertArray_GetVertByIndex( jeBrush_GetVertArray( pBrush->pBrush ), Index ) ;
	Brush_GetXForm( pBrush, &XForm ) ;
	jeXForm3d_Transform( &XForm, pVert, pVert ) ;
}// Brush_GetVertexPoint


void Brush_Update( Brush * pBrush, int Update_Type )
{
	Model *	pModel;	
	jeBrush_Face * pFace;
	FaceIterator MI;
	assert( pBrush != NULL );
	assert( pBrush->pBrush );

	if( Update_Type >= OBJECT_UPDATE_CHANGE )
		Object_Dirty( (Object*)pBrush );

	if( Update_Type > gBrush_Update )
		return;
	if( !(pBrush->ObjectData.miscFlags & OBJECT_DIRTY  ) )
		return;

	pModel = Brush_GetModel( pBrush ) ;
	if( pModel != NULL )
	{
		jeModel_UpdateBrush(Model_GetguModel(pModel ), pBrush->pBrush, gBrush_Lighting);
	}
	pFace = FaceList_GetFirstFace( pBrush->pSelFaces,  &MI) ;
	while( pFace != NULL )
	{
		jeModel_SetBrushFaceCBOnOff( Model_GetguModel(pModel ), pFace, JE_TRUE );
		pFace = FaceList_GetNextFace( pBrush->pSelFaces,  &MI) ;
	}
	pBrush->ObjectData.miscFlags &= ~OBJECT_DIRTY;

	
}

const jeExtBox * Brush_GetWorldAxialBounds( const Brush * pBrush )
{
	assert( pBrush != NULL ) ;
	assert( SIGNATURE == pBrush->nSignature ) ;
	
	//if( pBrush->Flags & BRUSH_FLAG_BOUNDSDIRTY )
	{	
		Brush * pEvalBrush = (Brush*)pBrush ;			// Lazy Evaluation requires removing the const
		Brush_UpdateBounds( pEvalBrush ) ;
	}

	return &pBrush->WorldBounds ;

}// Brush_GetWorldAxialBounds

void Brush_GetWorldCenter( const Brush * pBrush, jeVec3d * pCenter )
{
	jeXForm3d	XForm ;
	assert( pBrush != NULL ) ;
	assert( SIGNATURE == pBrush->nSignature ) ;
	assert( pCenter != NULL ) ;
	
	jeVec3d_Clear( pCenter ) ;
	Brush_GetXForm( pBrush , &XForm ) ;
	jeXForm3d_Transform( &XForm, pCenter, pCenter ) ;

}// Brush_GetWorldCenter

void	Brush_GetXForm( const Brush * pBrush, jeXForm3d *pXF )
{
	jeXForm3d ModelXForm;

	assert( pBrush != NULL ) ;
	assert( SIGNATURE == pBrush->nSignature ) ;

	jeXForm3d_Copy( jeBrush_GetXForm( pBrush->pBrush ), pXF ) ;
	if( pBrush->pModel )
	{
		Model_GetXForm(pBrush->pModel,&ModelXForm );
		jeXForm3d_Multiply( &ModelXForm, pXF, pXF );
	}

}// Brush_GetXForm

BRUSH_KIND Brush_GetKind( const Brush * pBrush )
{
	assert( pBrush != NULL ) ;
	assert( SIGNATURE == pBrush->nSignature ) ;

	return( pBrush->Kind  );

}// Brush_GetKind

VertList *	Brush_GetSelVert( const Brush * pBrush )
{
	return( pBrush->pSelVert );
} //Brush_GetSelVert


// IS
jeBoolean Brush_IsInModel( const Brush * pBrush )
{
	assert( pBrush != NULL ) ;
	assert( SIGNATURE == pBrush->nSignature ) ;

	return ( pBrush->pModel != NULL ) ? JE_TRUE : JE_FALSE ;

}// Brush_IsInModel

jeBoolean Brush_IsPointOverVertex( const Brush * pBrush, const jeVec3d * pWorldPt, ORTHO_AXIS OAxis, const jeFloat fThreshold, uint32 * pnVertex )
{
	jeExtBox	WorldBounds ;
	jeVec3d		Threshold ;

	assert( pBrush != NULL ) ;
	assert( SIGNATURE == pBrush->nSignature ) ;
	assert( pBrush->pBrush != NULL ) ;
	assert( pWorldPt != NULL ) ;
	assert( pnVertex != NULL ) ;

	jeVec3d_Set( &Threshold, fThreshold, fThreshold, fThreshold ) ;
	WorldBounds = *Brush_GetWorldAxialBounds( pBrush ) ;
	jeVec3d_Subtract( &WorldBounds.Min, &Threshold, &WorldBounds.Min ) ;
	jeVec3d_Add( &WorldBounds.Max, &Threshold, &WorldBounds.Max ) ;
	jeVec3d_SetElement( &WorldBounds.Min, OAxis, 0.0f ) ;
	jeVec3d_SetElement( &WorldBounds.Max, OAxis, 0.0f ) ;

	// Quick test of bounding box
	if( jeExtBox_ContainsPoint( &WorldBounds, pWorldPt ) )
	{
		jeVertArray		*	pVerts ;
		int32				nVerts ;
		jeVec3d				Vert ;
		jeXForm3d			XForm ;
		jeVertArray_Index	i ;

		Brush_GetXForm( pBrush, &XForm ) ;
		pVerts = jeBrush_GetVertArray( pBrush->pBrush ) ;
		nVerts = jeVertArray_GetMaxIndex( pVerts );

		for( i=0; i<nVerts; i++ )
		{
			Vert = *jeVertArray_GetVertByIndex( pVerts, i ) ;
			jeXForm3d_Transform( &XForm, &Vert, &Vert ) ;
			jeVec3d_SetElement( &Vert, OAxis, 0.0f ) ;
			if( jeVec3d_Compare( pWorldPt, &Vert, fThreshold ) )
			{
				*pnVertex = i ;
				return JE_TRUE ;
			}
		}
	}
	return JE_FALSE ;
}// Brush_IsPointOverVertex


jeBoolean Brush_IsPointOverNearVertex( const Brush * pBrush, const jeVec3d * pWorldPt, ORTHO_AXIS OAxis, const jeFloat fThreshold, uint32 * pnVertex )
{
	jeExtBox	WorldBounds ;
	jeVec3d		Threshold ;
	jeBoolean	bFoundVert = JE_FALSE ;

	assert( pBrush != NULL ) ;
	assert( SIGNATURE == pBrush->nSignature ) ;
	assert( pBrush->pBrush != NULL ) ;
	assert( pWorldPt != NULL ) ;
	assert( pnVertex != NULL ) ;

	// Tests for point closest in ortho view
	jeVec3d_Set( &Threshold, fThreshold, fThreshold, fThreshold ) ;
	WorldBounds = *Brush_GetWorldAxialBounds( pBrush ) ;
	jeVec3d_Subtract( &WorldBounds.Min, &Threshold, &WorldBounds.Min ) ;
	jeVec3d_Add( &WorldBounds.Max, &Threshold, &WorldBounds.Max ) ;
	jeVec3d_SetElement( &WorldBounds.Min, OAxis, 0.0f ) ;
	jeVec3d_SetElement( &WorldBounds.Max, OAxis, 0.0f ) ;

	// Quick test of bounding box
	if( jeExtBox_ContainsPoint( &WorldBounds, pWorldPt ) )
	{
		jeVertArray		*	pVerts ;
		int32				nVerts ;
		jeVec3d				Vert ;
		jeVec3d				Dif ;
		jeXForm3d			XForm ;
		jeVertArray_Index	i ;
		jeFloat				fNear = -FLT_MAX ;
		jeFloat				fNearAxis ;
		jeFloat				fDistSqared;
		jeFloat				fFoundDist;

		Brush_GetXForm( pBrush, &XForm );
		pVerts = jeBrush_GetVertArray( pBrush->pBrush ) ;
		nVerts = jeVertArray_GetMaxIndex( pVerts );

		fFoundDist = FLT_MAX;
		for( i=0; i<nVerts; i++ )
		{
			Vert = *jeVertArray_GetVertByIndex( pVerts, i ) ;
			jeXForm3d_Transform( &XForm, &Vert, &Vert ) ;
			jeVec3d_SetElement( &Vert, OAxis, 0.0f ) ;
			jeVec3d_Subtract( pWorldPt, &Vert, &Dif);
			fDistSqared = jeVec3d_LengthSquared( &Dif );
			if( fDistSqared < fThreshold && fDistSqared <= fFoundDist )
			{
				Vert = *jeVertArray_GetVertByIndex( pVerts, i ) ;
				fNearAxis = jeVec3d_GetElement( &Vert, OAxis ) ;
				if( fNearAxis > fNear )
				{
					*pnVertex = i ;
					bFoundVert = JE_TRUE ;
					fNear = fNearAxis ;
					fFoundDist = fDistSqared;
				}
			}
		}
	}
	return bFoundVert ;
}// Brush_IsPointOverNearVertex


jeBoolean Brush_HasSelectedVert( const Brush * pBrush )
{
	assert( pBrush );
	assert( pBrush->pSelVert );

	return( VertList_GetNumVert( pBrush->pSelVert ) != 0 );
}// Brush_HasSelectedVert

jeBoolean Brush_HasSelectedFace( const Brush * pBrush )
{
	assert( pBrush );
	assert( pBrush->pSelFaces );

	return( FaceList_GetNumFace( pBrush->pSelFaces ) != 0 );
}// Brush_HasSelectedFace


jeBoolean	Brush_IsFaceSelected( const Brush * pBrush, jeBrush_Face * pFace )
{
	FaceIterator  fI;

	assert( pBrush );
	assert( pFace );

	return( FaceList_Search( pBrush->pSelFaces,pFace, &fI ) );
}// Brush_IsFaceSelected

jeBoolean	Brush_IsInRect( const Brush * pBrush, jeExtBox *pSelRect, jeBoolean bSelEncompeses )
{
	const jeExtBox *pWorldBounds;
	jeExtBox		Result;

	assert( pBrush );
	assert( pSelRect );
	
	pWorldBounds = Brush_GetWorldAxialBounds(pBrush ) ;
	if( bSelEncompeses )
	{
		if( pSelRect->Max.X >= pWorldBounds->Max.X &&
			pSelRect->Max.Y >= pWorldBounds->Max.Y &&
			pSelRect->Max.Z >= pWorldBounds->Max.Z &&
			pSelRect->Min.X <= pWorldBounds->Min.X &&
			pSelRect->Min.Y <= pWorldBounds->Min.Y &&
			pSelRect->Min.Z <= pWorldBounds->Min.Z )
			 return( JE_TRUE );
	}
	else
	{
		return( Util_geExtBox_Intersection ( pSelRect, pWorldBounds, &Result	) );
	}
	return( JE_FALSE );
}//Brush_IsInRect

jeBoolean Brush_IsCutBrush( const Brush * pBrush )
{
	jeBrush_Contents Contents;

	assert( pBrush );
	assert( SIGNATURE == pBrush->nSignature ) ;
	assert( pBrush->pBrush );

	Contents = jeBrush_GetContents( pBrush->pBrush );

	return( Contents & JE_BSP_CONTENTS_AIR );
}

//
// MODIFIERS
//
void Brush_Move( Brush * pBrush, const jeVec3d * pWorldDistance )
{
	jeXForm3d	XForm ;
	assert( pBrush != NULL ) ;
	assert( SIGNATURE == pBrush->nSignature ) ;
	assert( pWorldDistance );

	Brush_GetXForm( pBrush, &XForm ) ;
	jeXForm3d_Translate( &XForm, pWorldDistance->X, pWorldDistance->Y, pWorldDistance->Z ) ;

	Brush_SetXForm( pBrush, &XForm ) ;
	Brush_SetModified( pBrush ) ;

	assert( Brush_IsBoundsValid( pBrush ) ) ;

}// Brush_Move

void Brush_Rotate( Brush * pBrush, ORTHO_AXIS RAxis, jeFloat RadianAngle, const jeVec3d * pRotationCenter )
{
	jeXForm3d	XForm ;
	assert( pBrush != NULL ) ;
	assert( SIGNATURE == pBrush->nSignature ) ;
	
	Brush_GetXForm( pBrush, &XForm ) ;
	jeXForm3d_Translate( &XForm, -pRotationCenter->X, -pRotationCenter->Y, -pRotationCenter->Z ) ;
	switch( RAxis )
	{
	case Ortho_Axis_X :
		jeXForm3d_RotateX( &XForm, RadianAngle ) ;	break ;
	case Ortho_Axis_Y :
		jeXForm3d_RotateY( &XForm, RadianAngle ) ;	break ;
	case Ortho_Axis_Z :
		jeXForm3d_RotateZ( &XForm, RadianAngle ) ;	break ;
	}
	jeXForm3d_Translate( &XForm, pRotationCenter->X, pRotationCenter->Y, pRotationCenter->Z ) ; 

	Brush_SetXForm( pBrush, &XForm) ;
	Brush_SetModified( pBrush ) ;

	assert( Brush_IsBoundsValid( pBrush ) ) ;

}// Brush_Rotate



void Brush_SetModel( Brush * pBrush, Model * pModel )
{
	assert( pBrush != NULL ) ;
	assert( SIGNATURE == pBrush->nSignature ) ;

	pBrush->pModel = pModel ;
}// Brush_SetModel

void Brush_SetModified( Brush * pBrush )
{
	assert( pBrush != NULL ) ;
	assert( SIGNATURE == pBrush->nSignature ) ;
	
	pBrush->Flags |= BRUSH_FLAG_BOUNDSDIRTY ;	// Both World and Brush bounds
	Object_Dirty( &pBrush->ObjectData );

}// Brush_SetModified


void Brush_Size( Brush * pBrush, const jeExtBox * pSelectedBounds, const jeFloat hScale, const jeFloat vScale, SELECT_HANDLE eSizeType, ORTHO_AXIS HAxis, ORTHO_AXIS VAxis )
{
	assert( pBrush != NULL ) ;
	assert( SIGNATURE == pBrush->nSignature ) ;

	// The Z axis is flipped (down is positive)
	// We just determine the edge and call _SizeEdge once or twice to keep this
	// as simple as possible
	switch( eSizeType )
	{
	case Select_Top :
		if( Ortho_Axis_Z == VAxis )
			Brush_SizeEdge( pBrush, &pSelectedBounds->Max, vScale, VAxis ) ;
		else
			Brush_SizeEdge( pBrush, &pSelectedBounds->Min, vScale, VAxis ) ;
		break ;

	case Select_Bottom :
		if( Ortho_Axis_Z == VAxis )
			Brush_SizeEdge( pBrush, &pSelectedBounds->Min, vScale, VAxis ) ;
		else
			Brush_SizeEdge( pBrush, &pSelectedBounds->Max, vScale, VAxis ) ;
		break ;

	case Select_Left :
		Brush_SizeEdge( pBrush, &pSelectedBounds->Max, hScale, HAxis ) ;
		break ;

	case Select_Right :
		Brush_SizeEdge( pBrush, &pSelectedBounds->Min, hScale, HAxis ) ;
		break ;

	case Select_TopLeft :
		if( Ortho_Axis_Z == VAxis )
			Brush_SizeEdge( pBrush, &pSelectedBounds->Max, vScale, VAxis ) ;
		else
			Brush_SizeEdge( pBrush, &pSelectedBounds->Min, vScale, VAxis ) ;
		Brush_SizeEdge( pBrush, &pSelectedBounds->Max, hScale, HAxis ) ;
		break ;		

	case Select_TopRight :
		if( Ortho_Axis_Z == VAxis )
			Brush_SizeEdge( pBrush, &pSelectedBounds->Max, vScale, VAxis ) ;
		else
			Brush_SizeEdge( pBrush, &pSelectedBounds->Min, vScale, VAxis ) ;
		Brush_SizeEdge( pBrush, &pSelectedBounds->Min, hScale, HAxis ) ;
		break ;

	case Select_BottomLeft :
		if( Ortho_Axis_Z == VAxis )
			Brush_SizeEdge( pBrush, &pSelectedBounds->Min, vScale, VAxis ) ;
		else
			Brush_SizeEdge( pBrush, &pSelectedBounds->Max, vScale, VAxis ) ;
		Brush_SizeEdge( pBrush, &pSelectedBounds->Max, hScale, HAxis ) ;
		break ;
	
	case Select_BottomRight :
		if( Ortho_Axis_Z == VAxis )
			Brush_SizeEdge( pBrush, &pSelectedBounds->Min, vScale, VAxis ) ;
		else
			Brush_SizeEdge( pBrush, &pSelectedBounds->Max, vScale, VAxis ) ;
		Brush_SizeEdge( pBrush, &pSelectedBounds->Min, hScale, HAxis ) ;
		break ;
	}
	Brush_SetModified( pBrush ) ;

}// Brush_Size


void Brush_SetXForm( Brush * pBrush, const jeXForm3d * XForm )
{
	jeXForm3d TempXForm;
	jeXForm3d ModelXForm;
	jeXForm3d IModelXForm;
	assert( pBrush != NULL ) ;
	assert( XForm != NULL ) ;
	assert( SIGNATURE == pBrush->nSignature ) ;

	TempXForm = *XForm;
	TempXForm.Flags = XFORM3D_NONORTHOGONALISOK;
	if( pBrush->pModel )
	{
		Model_GetXForm( pBrush->pModel, &ModelXForm );
		jeXForm3d_GetTranspose( &ModelXForm, &IModelXForm);
		jeXForm3d_Multiply( &IModelXForm, &TempXForm, &TempXForm );
	}
	jeBrush_SetXForm( pBrush->pBrush, &TempXForm, pBrush->bLockTextures) ;
	Brush_SetModified( pBrush ) ;
	assert( Brush_IsBoundsValid( pBrush ) ) ;

}// Brush_SetXForm

void Brush_UpdateBounds( const Brush * pBrush )
{
	if( pBrush->Flags & BRUSH_FLAG_BOUNDSDIRTY )
	{
		Brush * pModBrush = (Brush*)pBrush ;	// Lazy Eval requires cast
		
		Util_ExtBox_SetInvalid( &pModBrush->WorldBounds ) ;
		Brush_EnumFaceVerts( pModBrush, pModBrush, Brush_RebuildBounds ) ;
		pModBrush->Flags &= ~BRUSH_FLAG_BOUNDSDIRTY ;
	}
}// Brush_UpdateBounds

void Brush_SetGeBrush( Brush* pBrush, BRUSH_KIND Kind, jeBrush * pgeBrush )
{

	pBrush->pBrush = pgeBrush;
	pBrush->Flags |= BRUSH_FLAG_BOUNDSDIRTY;
	pBrush->Kind = Kind;
}//Brush_SetGeBrush

//When shearing there is a fixed side which is opposite the dragging handle
//To shear, each vert needs shift in the direction of the shear equal too
//the ratio of its distance from the fixed handle devided by the distance of the
//dragging handle.
//Since shiffting just the verts could cause them to shift beyond the brushs transform,
//it is best to shift the brush transform first and then shift the verts basded on the 
//distance between the transform and the vert.
void Brush_Shear( Brush* pBrush, const jeVec3d * pWorldDistance, SELECT_HANDLE eSizeType, ORTHO_AXIS HAxis, ORTHO_AXIS VAxis, const jeExtBox * pSelectedBounds)
{
	short		i;
	jeVertArray		*	pVerts ;
	int32				nVerts ;
	jeVec3d				Vert ;
	jeVec3d				VertTransformed ;
	jeVec3d				Translation;
	ORTHO_AXIS			ScaleAxis = 0;
	jeFloat				ScaleLength = 0.0f;
	jeFloat				FixedHandle = 0.0f;
	jeXForm3d			BrushXForm;
	jeXForm3d			InverseBrushXForm;
	jeFloat				Scale;

	assert(pBrush);
	assert(pWorldDistance);
	assert(pSelectedBounds);

	jeVec3d_Set( &Translation, 0.0f, 0.0f, 0.0f );
	switch( eSizeType )
	{
	case Select_Left:
		ScaleAxis = HAxis;
		FixedHandle = jeVec3d_GetElement( &pSelectedBounds->Max, HAxis );
		jeVec3d_SetElement( &Translation, VAxis, -jeVec3d_GetElement( pWorldDistance, VAxis ) );
		ScaleLength = jeVec3d_GetElement( &pSelectedBounds->Max, HAxis ) - jeVec3d_GetElement( &pSelectedBounds->Min, HAxis );
		break;

	case Select_Right:
		ScaleAxis = HAxis;
		FixedHandle = jeVec3d_GetElement( &pSelectedBounds->Min, HAxis );
		jeVec3d_SetElement( &Translation, VAxis, jeVec3d_GetElement( pWorldDistance, VAxis ) );
		ScaleLength = jeVec3d_GetElement( &pSelectedBounds->Max, HAxis ) - jeVec3d_GetElement( &pSelectedBounds->Min, HAxis );
		break;

	case Select_Top:
		ScaleAxis = VAxis;
		if( VAxis == Ortho_Axis_Z ) //I'm told that this is because in is negative Z
		{
			FixedHandle = jeVec3d_GetElement( &pSelectedBounds->Max, VAxis );
			jeVec3d_SetElement( &Translation, HAxis, -jeVec3d_GetElement( pWorldDistance, HAxis ) );
		}
		else
		{
			FixedHandle = jeVec3d_GetElement( &pSelectedBounds->Min, VAxis );
			jeVec3d_SetElement( &Translation, HAxis, jeVec3d_GetElement( pWorldDistance, HAxis ) );
		}
		ScaleLength = jeVec3d_GetElement( &pSelectedBounds->Max, VAxis ) - jeVec3d_GetElement( &pSelectedBounds->Min, VAxis );
		break;

	case Select_Bottom:
		ScaleAxis = VAxis;
		if( VAxis == Ortho_Axis_Z ) //I'm told that this is because in is negative Z
		{
			FixedHandle = jeVec3d_GetElement( &pSelectedBounds->Min, VAxis );
			jeVec3d_SetElement( &Translation, HAxis, jeVec3d_GetElement( pWorldDistance, HAxis ) );
		}
		else
		{
			FixedHandle = jeVec3d_GetElement( &pSelectedBounds->Max, VAxis );
			jeVec3d_SetElement( &Translation, HAxis, -jeVec3d_GetElement( pWorldDistance, HAxis ) );
		}
		ScaleLength = jeVec3d_GetElement( &pSelectedBounds->Max, VAxis ) - jeVec3d_GetElement( &pSelectedBounds->Min, VAxis );
		break;

	default:
		assert(0);
	}

	//First transform the brush
	Brush_GetXForm( pBrush, &BrushXForm ) ;
	Scale = (jeVec3d_GetElement( &BrushXForm.Translation, ScaleAxis ) - FixedHandle )/ScaleLength;
	jeVec3d_MA(&BrushXForm.Translation,  Scale, &Translation,  &BrushXForm.Translation );
	Brush_SetXForm(pBrush, &BrushXForm);

	pVerts = jeBrush_GetVertArray( pBrush->pBrush ) ;
	nVerts = jeVertArray_GetMaxIndex( pVerts );

	for( i=0; i<nVerts; i++ )
	{

		Vert = *jeVertArray_GetVertByIndex( pVerts, i ) ;
		jeXForm3d_Transform( &BrushXForm, &Vert, &VertTransformed );
		Scale = (jeVec3d_GetElement( &VertTransformed, ScaleAxis ) - jeVec3d_GetElement( &BrushXForm.Translation, ScaleAxis ) )/ScaleLength;

		jeVec3d_MA(&VertTransformed,  Scale, &Translation,  &VertTransformed );
		Brush_GetXForm( pBrush, &BrushXForm ) ;
		jeXForm3d_GetInverse(&BrushXForm, &InverseBrushXForm);
		InverseBrushXForm.Flags = XFORM3D_NONORTHOGONALISOK;
		jeXForm3d_Transform( &InverseBrushXForm, &VertTransformed, &Vert );
		jeVertArray_SetVertByIndex( pVerts, i, &Vert );
	}
	pBrush->Flags |= BRUSH_FLAG_BOUNDSDIRTY;
}

//  FACE MODIFIERS
void Brush_SelectFace( Brush * pBrush, jeBrush_Face * pFace )
{
	FaceIterator	fI;

	assert( pBrush );
	assert( VertList_GetNumVert( pBrush->pSelVert ) == 0 );

	if( FaceList_Search( pBrush->pSelFaces, pFace, &fI ) )
		return;
	FaceList_Append( pBrush->pSelFaces, pFace );
	if( pBrush->pModel )
		jeModel_SetBrushFaceCBOnOff(Model_GetguModel( pBrush->pModel), pFace, JE_TRUE );
}

void Brush_DeselectFace( Brush * pBrush, jeBrush_Face * pFace, jeBoolean *FaceListEmpty )
{

	assert( pBrush );
	assert( VertList_GetNumVert( pBrush->pSelVert ) == 0 );

	FaceList_Remove( pBrush->pSelFaces, pFace );
	*FaceListEmpty = ( FaceList_GetNumFace( pBrush->pSelFaces ) == 0 );
	if( pBrush->pModel )
		jeModel_SetBrushFaceCBOnOff(Model_GetguModel( pBrush->pModel), pFace, JE_FALSE );
}

void Brush_DeselectAllFaces( Brush * pBrush )
{
	FaceIterator	fI;
	jeBrush_Face	*	pFace;

	assert( pBrush );

	while( FaceList_GetNumFace(pBrush->pSelFaces)  )
	{
			pFace = FaceList_GetFirstFace(pBrush->pSelFaces, &fI );
			if( pBrush->pModel )
				jeModel_SetBrushFaceCBOnOff(Model_GetguModel( pBrush->pModel), pFace, JE_FALSE );
			FaceList_Remove( pBrush->pSelFaces, pFace );
	}
}

void Brush_SelectAllFaces( Brush * pBrush )
{
	jeBrush_Face	*	pFace = NULL;

	assert( pBrush );
	assert( pBrush->pSelFaces );

	Brush_DeselectAllFaces( pBrush );
	
	pFace = jeBrush_GetNextFace( pBrush->pBrush, pFace);
	while( pFace != NULL )
	{
		FaceList_Append( pBrush->pSelFaces, pFace );
		if( pBrush->pModel )
			jeModel_SetBrushFaceCBOnOff(Model_GetguModel( pBrush->pModel), pFace, JE_TRUE );
		pFace = jeBrush_GetNextFace( pBrush->pBrush, pFace);
	}
}

void Brush_SelectFirstFace( Brush * pBrush )
{
	jeBrush_Face	*	pFace = NULL;

	pFace = jeBrush_GetNextFace( pBrush->pBrush, pFace);
	FaceList_Append( pBrush->pSelFaces, pFace );
	if( pBrush->pModel )
		jeModel_SetBrushFaceCBOnOff(Model_GetguModel( pBrush->pModel), pFace, JE_TRUE );
}

void Brush_SelectLastFace( Brush * pBrush )
{
	jeBrush_Face	*	pFace = NULL;
	jeBrush_Face	*	pPrevFace = NULL;

	do 
	{
		pPrevFace = pFace;
		pFace = jeBrush_GetNextFace( pBrush->pBrush, pPrevFace);
	}while( pFace );
	if( pPrevFace !=NULL )
	{
		FaceList_Append( pBrush->pSelFaces, pPrevFace );
		if( pBrush->pModel )
			jeModel_SetBrushFaceCBOnOff(Model_GetguModel( pBrush->pModel), pFace, JE_TRUE );
	}
}// Brush_SelectLastFace

static void * Brush_BuildMatrUndoContext( jeBrush_Face *pFace, jeModel * pModel, jeMaterial_ArrayIndex MaterialIndex )
{
	MaterialUndoContext *pUndoContext;

	pUndoContext = JE_RAM_ALLOCATE_STRUCT( MaterialUndoContext );
	if( pUndoContext == NULL )
		return( NULL );
	pUndoContext->MaterialIndex = MaterialIndex;
	pUndoContext->pgeModel = pModel;
	pUndoContext->pFace = pFace;

	return( pUndoContext );
}

static jeBoolean Brush_ApplyMatrCB( jeBrush_Face *pFace, void * pVoid )
{
	FaceInfoCB_Struct * pFaceInfoData = (FaceInfoCB_Struct*)pVoid;
	jeFaceInfo FaceInfo;
	void * UndoContext;

	assert( pFace );
	assert( pVoid );

	jeBrush_FaceGetFaceInfo( pFace, &FaceInfo );
	UndoContext = Brush_BuildMatrUndoContext( pFace, pFaceInfoData->pgeModel, FaceInfo.MaterialIndex );
	if( UndoContext != NULL )
		Undo_AddSubTransaction( pFaceInfoData->pUndo, UNDO_APPLYMATERIAL, (Object*)pFaceInfoData->pBrush, UndoContext );

	FaceInfo.MaterialIndex = pFaceInfoData->pFaceInfo->MaterialIndex;
	jeBrush_FaceSetFaceInfo( pFace, &FaceInfo);
	if( pFaceInfoData->pgeModel != NULL )
		jeModel_UpdateBrushFace( pFaceInfoData->pgeModel, pFace, JE_TRUE );
	return( JE_TRUE );
}

void Brush_ApplyMatrToFaces( Brush * pBrush, const jeFaceInfo *pFaceInfo, Undo *pUndo )
{
	FaceInfoCB_Struct FaceInfoData;

	assert( pBrush );
	assert( pFaceInfo );

	FaceInfoData.pFaceInfo = (jeFaceInfo *)pFaceInfo;
	FaceInfoData.pBrush = pBrush;
	FaceInfoData.pUndo = pUndo;
	if(  pBrush->pModel != NULL )
		FaceInfoData.pgeModel = Model_GetguModel( pBrush->pModel );
	else
		FaceInfoData.pgeModel = NULL;
	FaceList_Enum(pBrush->pSelFaces, Brush_ApplyMatrCB, (void*)&FaceInfoData);
}//Brush_ApplyMatrToFaces

static jeBoolean Brush_SetFaceInfoCB( jeBrush_Face *pFace, void * pVoid )
{
	FaceInfoCB_Struct* pFaceInfoData = (FaceInfoCB_Struct*)pVoid;
	jeFaceInfo FaceInfo;
	int32		Flags;

	assert( pFace );
	assert( pVoid );

	assert( pFace );
	assert( pVoid );

	jeBrush_FaceGetFaceInfo(pFace, &FaceInfo);

	if( pFaceInfoData->FieldFlag & FACE_FIELD_DRAWSCALEU )
	{
		if( pFaceInfoData->pFaceInfo->DrawScaleU == 0.0f )
		{
			if( FaceInfo.DrawScaleU > 0.0f )
				FaceInfo.DrawScaleU = -0.1f;
			else
				FaceInfo.DrawScaleU = 0.1f;
		}
		else
			FaceInfo.DrawScaleU = pFaceInfoData->pFaceInfo->DrawScaleU;
	}

	if( pFaceInfoData->FieldFlag & FACE_FIELD_DRAWSCALEV )
	{
		if( pFaceInfoData->pFaceInfo->DrawScaleV == 0.0f )
		{
			if( FaceInfo.DrawScaleV > 0.0f )
				FaceInfo.DrawScaleV = -0.1f;
			else
				FaceInfo.DrawScaleV = 0.1f;
		}
		else
			FaceInfo.DrawScaleV = pFaceInfoData->pFaceInfo->DrawScaleV;
	}

	Flags = pFaceInfoData->pFaceInfo->Flags & FACEINFO_GOURAUD;
	if( pFaceInfoData->FieldFlag & FACE_FIELD_GOURAUD )
	{
		Flags = FACE_FIELD_GOURAUD & pFaceInfoData->pFaceInfo->Flags;
		FaceInfo.Flags &= ~FACE_FIELD_GOURAUD;
		FaceInfo.Flags |= Flags;
	}

	Flags = pFaceInfoData->pFaceInfo->Flags & FACEINFO_VIS_PORTAL;
	if( pFaceInfoData->FieldFlag & FACE_FIELD_VIS_PORTAL )
	{
		Flags = FACEINFO_VIS_PORTAL & pFaceInfoData->pFaceInfo->Flags;
		FaceInfo.Flags &= ~FACEINFO_VIS_PORTAL;
		FaceInfo.Flags |= Flags;
	}

	Flags = pFaceInfoData->pFaceInfo->Flags & FACEINFO_TRANSPARENT;
	if( pFaceInfoData->FieldFlag & FACEINFO_TRANSPARENT )
	{
		Flags = FACEINFO_TRANSPARENT & pFaceInfoData->pFaceInfo->Flags;
		FaceInfo.Flags &= ~FACEINFO_TRANSPARENT;
		FaceInfo.Flags |= Flags;
	}

	Flags = pFaceInfoData->pFaceInfo->Flags & FACEINFO_FLAT;
	if( pFaceInfoData->FieldFlag & FACE_FIELD_FLAT )
	{
		Flags = FACEINFO_FLAT & pFaceInfoData->pFaceInfo->Flags;
		FaceInfo.Flags &= ~FACEINFO_FLAT;
		FaceInfo.Flags |= Flags;
	}

	Flags = pFaceInfoData->pFaceInfo->Flags & FACEINFO_FULLBRIGHT;
	if( pFaceInfoData->FieldFlag & FACE_FIELD_FULLBRIGHT )
	{
		Flags = FACEINFO_FULLBRIGHT & pFaceInfoData->pFaceInfo->Flags;
		FaceInfo.Flags &= ~FACEINFO_FULLBRIGHT;
		FaceInfo.Flags |= Flags;
	}

	Flags = pFaceInfoData->pFaceInfo->Flags & FACEINFO_RENDER_PORTAL_ONLY;
	if( pFaceInfoData->FieldFlag & FACE_FIELD_ONLYPORTAL )
	{
		Flags = FACEINFO_RENDER_PORTAL_ONLY & pFaceInfoData->pFaceInfo->Flags;
		FaceInfo.Flags &= ~FACEINFO_RENDER_PORTAL_ONLY;;
		FaceInfo.Flags |= Flags;
	}

	if( pFaceInfoData->FieldFlag & FACE_FIELD_PORTALCAMERA )
	{
		FaceInfo.PortalCamera = pFaceInfoData->pFaceInfo->PortalCamera;
	}

	if( pFaceInfoData->FieldFlag & FACE_FIELD_LMAPSCALEU )
	{
		if( pFaceInfoData->pFaceInfo->LMapScaleU == 0.0f )
		{
			if( FaceInfo.LMapScaleU > 0.0f )
				FaceInfo.LMapScaleU = -0.5f;
			else
				FaceInfo.LMapScaleU = 0.5f;
		}
		else
			FaceInfo.LMapScaleU = pFaceInfoData->pFaceInfo->LMapScaleU;
	}

	if( pFaceInfoData->FieldFlag & FACE_FIELD_LMAPSCALEV )
	{
		if( pFaceInfoData->pFaceInfo->LMapScaleV == 0.0f )
		{
			if( FaceInfo.LMapScaleV > 0.0f )
				FaceInfo.LMapScaleV = -0.5f;
			else
				FaceInfo.LMapScaleV = 0.5f;
		}
		else
			FaceInfo.LMapScaleV = pFaceInfoData->pFaceInfo->LMapScaleV;
	}

	if( pFaceInfoData->FieldFlag & FACE_FIELD_ROTATE )
	{
		if( pFaceInfoData->pFaceInfo->Rotate >= 360.0f ) //The limits function wont let it greater than 360
			pFaceInfoData->pFaceInfo->Rotate -= 360.0f;
		else
		if(  pFaceInfoData->pFaceInfo->Rotate <= 0.0f ) //The limits function wont let it less than -1
			pFaceInfoData->pFaceInfo->Rotate += 360.0f;
		FaceInfo.Rotate = pFaceInfoData->pFaceInfo->Rotate;
	}

	if( pFaceInfoData->FieldFlag & FACE_FIELD_SHIFTU )
		FaceInfo.ShiftU = pFaceInfoData->pFaceInfo->ShiftU;

	if( pFaceInfoData->FieldFlag & FACE_FIELD_SHIFTV )
		FaceInfo.ShiftV = pFaceInfoData->pFaceInfo->ShiftV;

	if( pFaceInfoData->FieldFlag & FACE_FIELD_ALPHA )
		FaceInfo.Alpha = pFaceInfoData->pFaceInfo->Alpha;

	jeBrush_FaceSetFaceInfo( pFace, &FaceInfo);
	if( pFaceInfoData->pgeModel != NULL )
		jeModel_UpdateBrushFace( pFaceInfoData->pgeModel, pFace, JE_TRUE );
	return( JE_TRUE );
}

void Brush_SetFaceInfo( Brush * pBrush, FaceInfoCB_Struct* FaceInfoData)
{

	if( pBrush->pModel != NULL )
		FaceInfoData->pgeModel = Model_GetguModel( pBrush->pModel );
	else
		FaceInfoData->pgeModel = NULL;
	FaceList_Enum(pBrush->pSelFaces, Brush_SetFaceInfoCB, FaceInfoData);
}//Brush_SetFaceInfo



jeBoolean Brush_FillPositionDescriptor( Brush *pBrush, jeProperty_List * pArray )
{
	jeXForm3d XForm;
	char * Name;

	jeProperty Property;
	Brush_GetXForm( pBrush, &XForm );

	Name = Util_LoadLocalRcString( IDS_POSITION_FIELD );
	if( Name == NULL )
		return( JE_FALSE );
	jeProperty_FillVec3dGroup( &Property, Name, &XForm.Translation,	OBJECT_POSITION_FIELD  );
	if( !jeProperty_Append( pArray,  &Property ) )
	{
		jeRam_Free( Name );
		return( JE_FALSE );
	}
	jeRam_Free( Name );

	Name = Util_LoadLocalRcString( IDS_POSITIONX_FIELD );
	if( Name == NULL )
		return( JE_FALSE );
	jeProperty_FillFloat(  &Property, Name, XForm.Translation.X, OBJECT_POSITION_FIELDX, -FLT_MAX, FLT_MAX, 1.0f );
	if( !jeProperty_Append( pArray,  &Property ) )
	{
		jeRam_Free( Name );
		return( JE_FALSE );
	}
	jeRam_Free( Name );

	Name = Util_LoadLocalRcString( IDS_POSITIONY_FIELD );
	if( Name == NULL )
		return( JE_FALSE );
	jeProperty_FillFloat(  &Property, Name, XForm.Translation.Y,	OBJECT_POSITION_FIELDY, -FLT_MAX, FLT_MAX, 1.0f );
	if( !jeProperty_Append( pArray, &Property ) )
	{
		jeRam_Free( Name );
		return( JE_FALSE );
	}
	jeRam_Free( Name );

	Name = Util_LoadLocalRcString( IDS_POSITIONZ_FIELD );
	if( Name == NULL )
		return( JE_FALSE );
	jeProperty_FillFloat( &Property, Name, XForm.Translation.Z, OBJECT_POSITION_FIELDZ, -FLT_MAX, FLT_MAX, 1.0f );
	if( !jeProperty_Append( pArray, &Property ) )
	{
		jeRam_Free( Name );
		return( JE_FALSE );
	}
	jeRam_Free( Name );

	jeProperty_FillGroupEnd( &Property, OBJECT_POSITION_FIELD_END );
	if( !jeProperty_Append( pArray, &Property ) )
	{
		return( JE_FALSE );
	}
	return( JE_TRUE );
}

static jeBoolean Brush_GetFaceInfoCB( jeBrush_Face *pFace, void * pVoid )
{
	FaceInfoCB_Struct* pFaceInfoData = (FaceInfoCB_Struct*)pVoid;
	jeBoolean bInitAll;
	jeFaceInfo FaceInfo;
	int32		Flags;
	int32		OldFlags;
	assert( pFace );
	assert( pVoid );
	bInitAll = (pFaceInfoData->FieldFlag == FACE_INIT_ALL );

	jeBrush_FaceGetFaceInfo( pFace, &FaceInfo );

	if( bInitAll )
	{
		(*pFaceInfoData->pFaceInfo) = FaceInfo;
		pFaceInfoData->FieldFlag = 0;
		return( JE_TRUE );
	}

	if(FaceInfo.DrawScaleU != pFaceInfoData->pFaceInfo->DrawScaleU )
		pFaceInfoData->FieldFlag |= FACE_FIELD_DRAWSCALEU;

	if(FaceInfo.DrawScaleV != pFaceInfoData->pFaceInfo->DrawScaleV )
		pFaceInfoData->FieldFlag |= FACE_FIELD_DRAWSCALEV;

	Flags = FaceInfo.Flags & FACEINFO_GOURAUD;
	OldFlags = pFaceInfoData->pFaceInfo->Flags & FACEINFO_GOURAUD;
	if( Flags != OldFlags )
		pFaceInfoData->FieldFlag |= FACE_FIELD_GOURAUD;

	Flags = FaceInfo.Flags & FACEINFO_VIS_PORTAL;
	OldFlags = pFaceInfoData->pFaceInfo->Flags & FACEINFO_VIS_PORTAL;
	if( Flags != OldFlags )
		pFaceInfoData->FieldFlag |= FACE_FIELD_VIS_PORTAL;

	Flags = FaceInfo.Flags & FACEINFO_TRANSPARENT;
	OldFlags = pFaceInfoData->pFaceInfo->Flags & FACEINFO_TRANSPARENT;
	if( Flags != OldFlags )
		pFaceInfoData->FieldFlag |= FACE_FIELD_INVISIBLE;

	Flags = FaceInfo.Flags & FACEINFO_FULLBRIGHT;
	OldFlags = pFaceInfoData->pFaceInfo->Flags & FACEINFO_FULLBRIGHT;
	if( Flags != OldFlags )
		pFaceInfoData->FieldFlag |= FACE_FIELD_FULLBRIGHT;

	Flags = FaceInfo.Flags & FACEINFO_FLAT;
	OldFlags = pFaceInfoData->pFaceInfo->Flags & FACEINFO_FLAT;
	if( Flags != OldFlags )
		pFaceInfoData->FieldFlag |= FACE_FIELD_FULLBRIGHT;

	if(FaceInfo.LMapScaleU != pFaceInfoData->pFaceInfo->LMapScaleU )
		pFaceInfoData->FieldFlag |= FACE_FIELD_LMAPSCALEU;

	if(FaceInfo.LMapScaleV != pFaceInfoData->pFaceInfo->LMapScaleV )
		pFaceInfoData->FieldFlag |= FACE_FIELD_LMAPSCALEV;

	if(FaceInfo.Rotate != pFaceInfoData->pFaceInfo->Rotate )
		pFaceInfoData->FieldFlag |= FACE_FIELD_ROTATE;

	if(FaceInfo.ShiftU != pFaceInfoData->pFaceInfo->ShiftU )
		pFaceInfoData->FieldFlag |= FACE_FIELD_SHIFTU;

	if(FaceInfo.ShiftV != pFaceInfoData->pFaceInfo->ShiftV )
		pFaceInfoData->FieldFlag |= FACE_FIELD_SHIFTV;

	if(FaceInfo.Alpha != pFaceInfoData->pFaceInfo->Alpha )
		pFaceInfoData->FieldFlag |= FACE_FIELD_ALPHA;

	return( JE_TRUE );
}

void Brush_GetFaceInfo( Brush * pBrush, FaceInfoCB_Struct* pFaceInfoData )
{
	assert( pBrush );
	assert( pFaceInfoData );
	FaceList_Enum( pBrush->pSelFaces, Brush_GetFaceInfoCB, pFaceInfoData);
}


jeBoolean Brush_FillFaceInfoDescriptor( Brush *pBrush, jeProperty_List * pArray )
{
	jeProperty Property;
	FaceInfoCB_Struct FaceInfoData;
	jeFaceInfo  FaceInfo;
	int	Bool;
	char * Name;
	const char * PortalName = NULL;

	FaceInfoData.FieldFlag = FACE_INIT_ALL;
	FaceInfoData.pFaceInfo = &FaceInfo;

	Brush_GetFaceInfo( pBrush, &FaceInfoData );
	

	//Face Info Group Begin
	Name = Util_LoadLocalRcString( IDS_FACE_FIELD );
	if( Name == NULL )
		return( JE_FALSE );
	jeProperty_FillGroup( &Property, Name,	BRUSH_FACEINFO_FIELD  );
	jeRam_Free( Name );
	if( !jeProperty_Append( pArray, &Property  ) )
	{
		return( JE_FALSE );
	}

	//Gouraud check box
	Bool = (( FaceInfo.Flags & FACEINFO_GOURAUD ) != 0 );
	Name = Util_LoadLocalRcString( IDS_GOURAND_FIELD );
	if( Name == NULL )
		return( JE_FALSE );
	jeProperty_FillCheck( &Property, Name, Bool, BRUSH_GOURAND_FIELD );
	jeRam_Free( Name );
	if( FaceInfoData.FieldFlag & FACE_FIELD_GOURAUD )
		jeProperty_SetDataInvalid( &Property  );
	jeProperty_SetDisabled( &Property, JE_TRUE );
	if( !jeProperty_Append( pArray, &Property  ) )
	{
		return( JE_FALSE );
	}

	//Flat shade check box
	Bool = (( FaceInfo.Flags & FACEINFO_FLAT ) != 0 );
	Name = Util_LoadLocalRcString( IDS_FLAT_FIELD );
	if( Name == NULL )
		return( JE_FALSE );
	jeProperty_FillCheck( &Property, Name, Bool, BRUSH_FLAT_FIELD );
	jeRam_Free( Name );
	if( FaceInfoData.FieldFlag & FACE_FIELD_FLAT )
		jeProperty_SetDataInvalid( &Property );
	jeProperty_SetDisabled(&Property, JE_TRUE );
	if( !jeProperty_Append( pArray, &Property ) )
	{
		return( JE_FALSE );
	}


	//Transparent check box
	Bool = (( FaceInfo.Flags & FACEINFO_TRANSPARENT ) != 0 );
	Name = Util_LoadLocalRcString( IDS_TRANSPARENT_FIELD );
	if( Name == NULL )
		return( JE_FALSE );
	jeProperty_FillCheck( &Property, Name, Bool, BRUSH_TRANSPARENT_FIELD );
	jeRam_Free( Name );
	if( FaceInfoData.FieldFlag & FACE_FIELD_INVISIBLE )
		jeProperty_SetDataInvalid( &Property );
	if( !jeProperty_Append( pArray, &Property ) )
	{
		return( JE_FALSE );
	}


	//Aplha blending float field
	Name = Util_LoadLocalRcString( IDS_ALPHA_FIELD );
	if( Name == NULL )
		return( JE_FALSE );
	jeProperty_FillFloat( &Property, Name, FaceInfo.Alpha, BRUSH_ALPHA_FIELD, 0, 255, 1.0f );
	jeRam_Free( Name );
	if( FaceInfoData.FieldFlag & FACE_FIELD_ALPHA )
		jeProperty_SetDataInvalid( &Property );
	if( !jeProperty_Append( pArray, &Property ) )
	{
		return( JE_FALSE );
	}


	//Fullbright check box
	Bool = (( FaceInfo.Flags & FACEINFO_FULLBRIGHT ) != 0 );
	Name = Util_LoadLocalRcString( IDS_FULLBRIGHT_FIELD );
	if( Name == NULL )
		return( JE_FALSE );
	jeProperty_FillCheck( &Property, Name, Bool, BRUSH_FULLBRIGHT_FIELD );
	jeRam_Free( Name );
	if( FaceInfoData.FieldFlag &  FACE_FIELD_FULLBRIGHT )
		jeProperty_SetDataInvalid( &Property );
	if( !jeProperty_Append( pArray, &Property ) )
	{
		return( JE_FALSE );
	}


	//Vis Portal check box
	Bool = (( FaceInfo.Flags & FACEINFO_VIS_PORTAL ) != 0 );
	Name = Util_LoadLocalRcString( IDS_VIS_PORTAL_FIELD );
	if( Name == NULL )
		return( JE_FALSE );
	jeProperty_FillCheck( &Property, Name, Bool, BRUSH_VIS_PORTAL_FIELD );
	jeRam_Free( Name );
	if( FaceInfoData.FieldFlag & FACE_FIELD_VIS_PORTAL )
		jeProperty_SetDataInvalid( &Property );
	if( !jeProperty_Append( pArray, &Property ) )
	{
		return( JE_FALSE );
	}


	//Render Portal check box
	Bool = (( FaceInfo.Flags & FACEINFO_RENDER_PORTAL_ONLY ) != 0 );
	Name = Util_LoadLocalRcString( IDS_PORTAL_ONLY_FIELD );
	if( Name == NULL )
		return( JE_FALSE );
	jeProperty_FillCheck( &Property, Name, Bool, BRUSH_RENDER_PORTAL_ONLY );
	jeRam_Free( Name );
	if( FaceInfoData.FieldFlag & FACE_FIELD_ONLYPORTAL )
		jeProperty_SetDataInvalid( &Property );
	if( !jeProperty_Append( pArray, &Property ) )
	{
		return( JE_FALSE );
	}

	//Vis Portal combo box
	Name = Util_LoadLocalRcString( IDS_VIS_PORTAL_COMBO );
	if( Name == NULL )
		return( JE_FALSE );
	if( !(FaceInfoData.FieldFlag & FACE_FIELD_PORTALCAMERA) && FaceInfo.PortalCamera )
		PortalName = jeObject_GetName( FaceInfo.PortalCamera );
	else
		PortalName = VisPortalNone;
	jeProperty_FillCombo( &Property, Name, (char*)PortalName, BRUSH_VIS_PORTAL_COMBO, VisPortalN, VisPortalArray  );
	jeRam_Free( Name );
	if( !jeProperty_Append( pArray, &Property ) )
	{
		return( JE_FALSE );
	}

	Name = Util_LoadLocalRcString( IDS_ROTATE_FIELD );
	if( Name == NULL )
		return( JE_FALSE );
	jeProperty_FillFloat( &Property, Name, FaceInfo.Rotate,	BRUSH_ROTATE_FIELD, -1, 360, 1.0f );
	jeRam_Free( Name );
	if( FaceInfoData.FieldFlag & FACE_FIELD_ROTATE )
		jeProperty_SetDataInvalid( &Property );
	//jeProperty_SetDisabled( &Property, JE_TRUE );
	if( !jeProperty_Append( pArray, &Property ) )
	{
		return( JE_FALSE );
	}


	Name = Util_LoadLocalRcString( IDS_SHIFT_FIELD );
	if( Name == NULL )
		return( JE_FALSE );
	jeProperty_FillGroup( &Property, Name,	BRUSH_SHIFT_GROUP  );
	jeRam_Free( Name );
	if( !jeProperty_Append( pArray, &Property ) )
	{
		return( JE_FALSE );
	}

	Name = Util_LoadLocalRcString( IDS_SHIFTU_FIELD );
	if( Name == NULL )
		return( JE_FALSE );
	jeProperty_FillFloat( &Property, Name, FaceInfo.ShiftU,	BRUSH_SHIFTU_FIELD, -FLT_MAX, FLT_MAX, 1.0f );
	jeRam_Free( Name );

	if( FaceInfoData.FieldFlag & FACE_FIELD_SHIFTU )
		jeProperty_SetDataInvalid( &Property );
	if( !jeProperty_Append( pArray, &Property ) )
	{
		return( JE_FALSE );
	}


	Name = Util_LoadLocalRcString( IDS_SHIFTV_FIELD );
	if( Name == NULL )
		return( JE_FALSE );
	jeProperty_FillFloat( &Property, Name, FaceInfo.ShiftV, BRUSH_SHIFTV_FIELD, -FLT_MAX, FLT_MAX, 1.0f );
	jeRam_Free( Name );
	if( FaceInfoData.FieldFlag & FACE_FIELD_SHIFTV )
		jeProperty_SetDataInvalid( &Property );
	if( !jeProperty_Append( pArray, &Property ) )
	{
		return( JE_FALSE );
	}


	jeProperty_FillGroupEnd( &Property, BRUSH_SHIFT_GROUP_END );
	if( !jeProperty_Append( pArray, &Property ) )
	{
		return( JE_FALSE );
	}



	Name = Util_LoadLocalRcString( IDS_DRAWSCALE_FIELD );
	if( Name == NULL )
		return( JE_FALSE );
	jeProperty_FillGroup( &Property, Name,	BRUSH_DRAWSCALE_GROUP  );
	jeRam_Free( Name );
	if( !jeProperty_Append( pArray, &Property ) )
	{
		return( JE_FALSE );
	}


	Name = Util_LoadLocalRcString( IDS_DRAWSCALEU_FIELD );
	if( Name == NULL )
		return( JE_FALSE );
	jeProperty_FillFloat( &Property, Name, FaceInfo.DrawScaleU,	BRUSH_DRAWSCALEU_FIELD, -64.0f, 64.0f, 0.1f );
	jeRam_Free( Name );
	if( FaceInfoData.FieldFlag & FACE_FIELD_DRAWSCALEU )
		jeProperty_SetDataInvalid( &Property );
	if( !jeProperty_Append( pArray, &Property ) )
	{
		return( JE_FALSE );
	}


	Name = Util_LoadLocalRcString( IDS_DRAWSCALEV_FIELD );
	if( Name == NULL )
		return( JE_FALSE );
	jeProperty_FillFloat( &Property, Name, FaceInfo.DrawScaleV,	BRUSH_DRAWSCALEV_FIELD, -64.0f, 64.0f, 0.1f );
	jeRam_Free( Name );
	if( FaceInfoData.FieldFlag & FACE_FIELD_DRAWSCALEV )
		jeProperty_SetDataInvalid( &Property );
	if( !jeProperty_Append( pArray, &Property ) )
	{
		return( JE_FALSE );
	}


	jeProperty_FillGroupEnd( &Property, BRUSH_DRAWSCALE_GROUP_END );
	if( !jeProperty_Append( pArray, &Property ) )
	{
		return( JE_FALSE );
	}


	Name = Util_LoadLocalRcString( IDS_LIGHTMAP_FIELD );
	if( Name == NULL )
		return( JE_FALSE );
	jeProperty_FillGroup( &Property, Name,	BRUSH_LIGHTMAP_GROUP  );
	jeRam_Free( Name );
	if( !jeProperty_Append( pArray, &Property ) )
	{
		return( JE_FALSE );
	}


	Name = Util_LoadLocalRcString( IDS_LMAPSCALEU_FIELD );
	if( Name == NULL )
		return( JE_FALSE );
	jeProperty_FillFloat( &Property, Name, FaceInfo.LMapScaleU,	BRUSH_LMAPSCALEU_FIELD, -FLT_MAX, FLT_MAX, 0.5f );
	jeRam_Free( Name );
	if( FaceInfoData.FieldFlag & FACE_FIELD_LMAPSCALEU )
		jeProperty_SetDataInvalid( &Property );
	if( !jeProperty_Append( pArray, &Property ) )
	{
		return( JE_FALSE );
	}


	Name = Util_LoadLocalRcString( IDS_LMAPSCALEV_FIELD );
	if( Name == NULL )
		return( JE_FALSE );
	jeProperty_FillFloat( &Property, Name, FaceInfo.LMapScaleV,	BRUSH_LMAPSCALEV_FIELD, -FLT_MAX, FLT_MAX, 0.5f );
	jeRam_Free( Name );
	if( FaceInfoData.FieldFlag & FACE_FIELD_LMAPSCALEV )
		jeProperty_SetDataInvalid( &Property );
	if( !jeProperty_Append( pArray, &Property ) )
	{
		return( JE_FALSE );
	}


	jeProperty_FillGroupEnd( &Property, BRUSH_LIGHTMAP_GROUP_END );
	if( !jeProperty_Append( pArray, &Property ) )
	{
		return( JE_FALSE );
	}


	jeProperty_FillGroupEnd( &Property, BRUSH_FACEINFO_FIELD_END );
	if( !jeProperty_Append( pArray, &Property ) )
	{
		return( JE_FALSE );
	}
	return( JE_TRUE );
}

static void Brush_AddToPortalList( char * Name )
{
	if( VisPortalN == VisPortalAllocateN )
	{
		VisPortalAllocateN += VISPORTALBLOCK;
		if( VisPortalArray == NULL )
			VisPortalArray = jeRam_Allocate( sizeof( char*) * VisPortalAllocateN );
		else
			VisPortalArray = jeRam_Realloc( VisPortalArray, VisPortalAllocateN );
		if( VisPortalArray == NULL )
		{
			jeErrorLog_Add( JE_ERR_MEMORY_RESOURCE,"Level_AddToPortalList:jeRam_Realloc");
			return;
		}

	}
	assert( VisPortalN <= VisPortalAllocateN );
	VisPortalArray[VisPortalN] = Name;
	VisPortalN++;
}

static void Brush_UpdateVisPortalListChild( jeObject * pObject )
{
	jeObject * pChildObject;

	pChildObject = jeObject_GetNextChild( pObject, NULL );
	while( pChildObject )
	{
		if( jeObject_GetType(pChildObject) == JE_OBJECT_TYPE_PORTAL )
		{
			Brush_AddToPortalList( (char*)jeObject_GetName( pChildObject ) );
		}
		Brush_UpdateVisPortalListChild( pChildObject );
		pChildObject = jeObject_GetNextChild( pObject, pChildObject );
	}
}

static void Brush_UpdateVisPortalList(Brush * pBrush)
{
	jeObject *pObject;
	
	VisPortalN = 0;
	Brush_AddToPortalList( VisPortalNone );
	pObject = jeWorld_GetNextObject(pBrush->pWorld,NULL );
	while( pObject )
	{
		if( jeObject_GetType(pObject) == JE_OBJECT_TYPE_PORTAL )
		{
			Brush_AddToPortalList( (char*)jeObject_GetName( pObject ) );
		}
		Brush_UpdateVisPortalListChild( pObject );
		pObject = jeWorld_GetNextObject(pBrush->pWorld,pObject );
	}
}

jeProperty_List *	Brush_BuildDescriptor( Brush * pBrush )
{
	jeProperty_List * pArray = NULL;
	jeProperty Property;
	int	Bool;
	char * Name;

	jeBrush_Contents Contents;
#define BRUSH_BASE_FIELDN 33

	assert( pBrush );

	Brush_UpdateVisPortalList( pBrush);
	Contents = jeBrush_GetContents( pBrush->pBrush);
	pArray = jeProperty_ListCreateEmpty();
	if( pArray == NULL )
	{
		jeErrorLog_Add( JE_ERR_MEMORY_RESOURCE, "DescriptorArray" );
		return NULL;
	}

	Name = Util_LoadLocalRcString( IDS_NAME_FIELD );
	if( Name == NULL )
		goto BBD_ERROR;
	jeProperty_FillString( &Property, Name, pBrush->ObjectData.pszName, OBJECT_NAME_FIELD );
	jeRam_Free( Name );
	if( !jeProperty_Append( pArray, &Property ) )
	{
		goto BBD_ERROR;
	}


	Brush_FillPositionDescriptor( pBrush, pArray );


	
	Bool = (( Contents & JE_BSP_CONTENTS_SOLID ) != 0 );
	Name = Util_LoadLocalRcString( IDS_SOLID_FIELD );
	if( Name == NULL )
		goto BBD_ERROR;
	jeProperty_FillRadio( &Property, Name, Bool, BRUSH_SOLID_FIELD );
	jeRam_Free( Name );
	if( !jeProperty_Append( pArray, &Property ) )
	{
		goto BBD_ERROR;
	}


	Bool = (( Contents & JE_BSP_CONTENTS_EMPTY ) != 0 );
	Name = Util_LoadLocalRcString( IDS_EMPTY_FIELD );
	if( Name == NULL )
		goto BBD_ERROR;
	jeProperty_FillRadio( &Property, Name, Bool, BRUSH_EMPTY_FIELD );
	jeRam_Free( Name );
	if( !jeProperty_Append( pArray, &Property ) )
	{
		goto BBD_ERROR;
	}


	Bool = (( Contents & JE_BSP_CONTENTS_AIR ) != 0 );
	Name = Util_LoadLocalRcString( IDS_AIR_FIELD );
	if( Name == NULL )
		goto BBD_ERROR;
	jeProperty_FillRadio( &Property, Name, Bool, BRUSH_AIR_FIELD );
	jeRam_Free( Name );
	if( !jeProperty_Append( pArray, &Property ) )
	{
		goto BBD_ERROR;
	}


	jeProperty_FillGroupEnd( &Property, BRUSH_CONTENT_FIELD_END );
	if( !jeProperty_Append( pArray, &Property ) )
	{
		goto BBD_ERROR;
	}


	Brush_FillFaceInfoDescriptor( pBrush, pArray );
	BrushTemplate_FillTemplateDescriptor( pBrush->pTemplate, pArray );
	return( pArray );

BBD_ERROR:
	jeProperty_ListDestroy( &pArray );
	return( NULL );
}

void Brush_ModifyTemplate( 	Brush * pBrush, int DataId, int DataType, jeProperty_Data * pData, jeBoolean bBrushUpdate, jeBoolean bLightUpdate  )
{
	jeXForm3d XForm;
	jeBrush_Face *pFace;
	jeFaceInfo  FaceInfo;
	jeBrush_Contents Contents;
	Model * pModel;

	Brush_DeselectAllFaces( pBrush  );
	BrushTemplate_SetProperty( pBrush->pTemplate,  DataId, DataType, pData );
	Contents = jeBrush_GetContents( pBrush->pBrush );
	Brush_GetXForm( pBrush, &XForm);
	pFace = jeBrush_GetNextFace(pBrush->pBrush, NULL);
	if( pFace == NULL )
		return;
	if( !jeBrush_FaceGetFaceInfo(pFace, &FaceInfo) )
		return;


	pModel = pBrush->pModel;
	if( pBrush->pBrush != NULL )
	{
		Model_RemoveBrushWorld( pModel, pBrush );
		pBrush->pModel = NULL;
		jeBrush_Destroy( &pBrush->pBrush ) ;
	}

	pBrush->pBrush = BrushTemplate_CreateBrush( pBrush->pTemplate,&FaceInfo);
	jeBrush_SetContents(pBrush->pBrush, Contents);
	Brush_SetXForm( pBrush, &XForm );
	Brush_SetModified( pBrush );
	Model_AddBrushWorld( pModel, pBrush, bBrushUpdate, bLightUpdate );
	Brush_SelectAllFaces( pBrush );
}

static jeObject *Brush_FindPortalChild( jeObject * pObject, const char *PortalName )
{
	jeObject * pChildObject;
	jeObject * pChildChildObject;
	const char *ObjectName;

	pChildObject = jeObject_GetNextChild( pObject, NULL );
	while( pChildObject )
	{
		ObjectName = jeObject_GetName(pChildObject );
		if( ObjectName != NULL )
			if( strcmp( ObjectName, PortalName ) == 0 )
				return( pChildObject );
		pChildChildObject = Brush_FindPortalChild( pChildObject, PortalName );
		if( pChildChildObject != NULL )
			return( pChildChildObject );
		pChildObject = jeObject_GetNextChild( pObject, pChildObject );
	}
	return( NULL );
}

static jeObject *Brush_FindPortal( jeWorld * pWorld, const char *PortalName )
{
	jeObject * pObject;
	jeObject * pChildObject;
	const char *ObjectName;

	pObject = jeWorld_GetNextObject( pWorld, NULL );
	while( pObject )
	{
		ObjectName = jeObject_GetName(pObject );
		if( ObjectName != NULL )
			if( strcmp( ObjectName, PortalName ) == 0 )
				return( pObject );
		pChildObject = Brush_FindPortalChild( pObject, PortalName );
		if( pChildObject != NULL )
			return( pChildObject );
		pObject = jeWorld_GetNextObject( pWorld, pObject );
	}
	return( NULL );
}

void Brush_SetProperty( Brush* pBrush, int DataId, int DataType, jeProperty_Data * pData, jeBoolean bBrushUpdate, jeBoolean bLightUpdate  )
{
	int Contents = jeBrush_GetContents( pBrush->pBrush);
	FaceInfoCB_Struct FaceInfoData;
	jeFaceInfo  FaceInfo;
	jeBoolean	bFaceData = JE_FALSE;

	if( DataId >= TEMPLATE_FIELD_START )
	{
		Brush_ModifyTemplate(pBrush,  DataId, DataType, pData, bBrushUpdate, bLightUpdate  );
		return;
	}
	FaceInfoData.FieldFlag = 0;
	FaceInfoData.pFaceInfo = &FaceInfo;
	DataType;
	switch( DataId )
	{		

	case BRUSH_SOLID_FIELD:
		if( pData->Bool )
		{
			Contents |= JE_BSP_CONTENTS_SOLID;
			Contents &= ~JE_BSP_CONTENTS_EMPTY;
			Contents &= ~JE_BSP_CONTENTS_AIR;
		}
		break;

	case BRUSH_EMPTY_FIELD:
		if( pData->Bool )
		{
			Contents &= ~JE_BSP_CONTENTS_SOLID;
			Contents |= JE_BSP_CONTENTS_EMPTY;
			Contents &= ~JE_BSP_CONTENTS_AIR;
		}
		break;

	case BRUSH_AIR_FIELD:
		if( pData->Bool )
		{
			Contents &= ~JE_BSP_CONTENTS_SOLID;
			Contents &= ~JE_BSP_CONTENTS_EMPTY;
			Contents |= JE_BSP_CONTENTS_AIR;
		}
		break;
		
	case BRUSH_GOURAND_FIELD:
	{
		FaceInfo.Flags = pData->Bool ? FACEINFO_GOURAUD:0;
		FaceInfoData.FieldFlag = FACE_FIELD_GOURAUD;
		bFaceData = JE_TRUE;
	}
	break;
		
	case BRUSH_FLAT_FIELD:
	{
		FaceInfo.Flags = pData->Bool ? FACEINFO_FLAT:0;
		FaceInfoData.FieldFlag = FACE_FIELD_FLAT;
		bFaceData = JE_TRUE;
	}
	break;
		
	case BRUSH_TRANSPARENT_FIELD:
	{
		FaceInfo.Flags = pData->Bool ? FACEINFO_TRANSPARENT:0;
		FaceInfoData.FieldFlag = FACE_FIELD_INVISIBLE;
		bFaceData = JE_TRUE;
	}
	break;

				
	case BRUSH_FULLBRIGHT_FIELD:
	{
		FaceInfo.Flags = pData->Bool ? FACEINFO_FULLBRIGHT:0;
		FaceInfoData.FieldFlag = FACE_FIELD_FULLBRIGHT;
		bFaceData = JE_TRUE;
	}
	break;

		
	case BRUSH_VIS_PORTAL_FIELD:
	{
		FaceInfo.Flags = pData->Bool ? FACEINFO_VIS_PORTAL:0;
		FaceInfoData.FieldFlag = FACE_FIELD_VIS_PORTAL;
		bFaceData = JE_TRUE;
	}
	break;
	
	case BRUSH_RENDER_PORTAL_ONLY:
	{
		FaceInfo.Flags = pData->Bool ? FACEINFO_RENDER_PORTAL_ONLY:0;
		FaceInfoData.FieldFlag = FACE_FIELD_ONLYPORTAL;
		bFaceData = JE_TRUE;
	}
	break;

	case BRUSH_VIS_PORTAL_COMBO:
	{
		jeObject * pPortalCamera;

		assert( pBrush->pWorld );
		pPortalCamera = Brush_FindPortal( pBrush->pWorld, pData->String );
		FaceInfo.PortalCamera = pPortalCamera;
		FaceInfoData.FieldFlag = FACE_FIELD_PORTALCAMERA;
		if( pPortalCamera != NULL )
		{
			FaceInfo.Flags = FACEINFO_TRANSPARENT;
			FaceInfoData.FieldFlag |= FACE_FIELD_INVISIBLE;
		}
		bFaceData = JE_TRUE;
	}
	break;

	case BRUSH_ALPHA_FIELD:
	{
		FaceInfo.Alpha = pData->Float;
		FaceInfoData.FieldFlag = FACE_FIELD_ALPHA;
		bFaceData = JE_TRUE;
	}
	break;

	case BRUSH_ROTATE_FIELD:
	{
		FaceInfo.Rotate = pData->Float;
		FaceInfoData.FieldFlag = FACE_FIELD_ROTATE;
		bFaceData = JE_TRUE;
	}
	break;

	case BRUSH_SHIFTU_FIELD:
	{
		FaceInfo.ShiftU = pData->Float;
		FaceInfoData.FieldFlag = FACE_FIELD_SHIFTU;
		bFaceData = JE_TRUE;
	}
	break;

	case BRUSH_SHIFTV_FIELD:
	{
		FaceInfo.ShiftV = pData->Float;
		FaceInfoData.FieldFlag = FACE_FIELD_SHIFTV;
		bFaceData = JE_TRUE;
	}
	break;

	case BRUSH_DRAWSCALEU_FIELD:
	{
		FaceInfo.DrawScaleU = pData->Float;
		FaceInfoData.FieldFlag = FACE_FIELD_DRAWSCALEU;
		bFaceData = JE_TRUE;
	}
	break;

	case BRUSH_DRAWSCALEV_FIELD:
	{
		FaceInfo.DrawScaleV = pData->Float;
		FaceInfoData.FieldFlag = FACE_FIELD_DRAWSCALEV;
		bFaceData = JE_TRUE;
	}
	break;

	case BRUSH_LMAPSCALEU_FIELD:
	{
		FaceInfo.LMapScaleU = pData->Float;
		FaceInfoData.FieldFlag = FACE_FIELD_LMAPSCALEU;
		bFaceData = JE_TRUE;
	}
	break;

	case BRUSH_LMAPSCALEV_FIELD:
	{
		FaceInfo.LMapScaleV = pData->Float;
		FaceInfoData.FieldFlag = FACE_FIELD_LMAPSCALEV;
		bFaceData = JE_TRUE;
	}
	break;

	}
	if( bFaceData )
	{
		Brush_SetFaceInfo( pBrush, &FaceInfoData );
	}
	else
	{
		Model* pModel;
		jeBrush_SetContents( pBrush->pBrush, Contents );
		pModel = pBrush->pModel;
		Brush_SetModified( pBrush );
		if( bBrushUpdate )
			Brush_Update( pBrush, bLightUpdate );
	}
}

static jeBoolean Brush_ResetSelFacerCB( jeBrush_Face *pFace, void * Context )
{
	Model * pModel = (Model*)Context;

	if( pModel )
		jeModel_SetBrushFaceCBOnOff(Model_GetguModel( pModel ), pFace, JE_TRUE );
	return( JE_TRUE );
}

void Brush_ResetSelFace( Brush * pBrush )
{
	FaceList_Enum(pBrush->pSelFaces, Brush_ResetSelFacerCB, pBrush->pModel);
}

jeBoolean Brush_AttachWorld( Brush * pBrush, jeWorld * pWorld )
{
	pBrush->pWorld = pWorld;
	return( JE_TRUE );
}

jeBoolean Brush_DettachWorld( Brush * pBrush, jeWorld * pWorld )
{
	pBrush->pWorld = NULL;
	pWorld;
	return( JE_TRUE );
}

jeBoolean Brush_SelectVert( Brush * pBrush, jeVertArray_Index  Index )
{
	VertIterator	vI;
	Vert_Struct	*	pVert;
	jeVertArray *	pArray;

	assert( pBrush );

	if( VertList_SearchByIndex( pBrush->pSelVert, &Index, &vI ) )
		return( JE_TRUE );
	pVert = JE_RAM_ALLOCATE_STRUCT( Vert_Struct );
	if( pVert == NULL )
	{
		jeErrorLog_Add( JE_ERR_MEMORY_RESOURCE, "Brush_SelectVert" );
		return( JE_FALSE );
	}
	pVert->Index = Index;
	pArray = jeBrush_GetVertArray( pBrush->pBrush );
	pVert->LastPos = *jeVertArray_GetVertByIndex( pArray, Index );
	VertList_Append( pBrush->pSelVert, pVert );
	return( JE_TRUE );
}// Brush_SelectVert

void Brush_DestroyVertCB(void *pData)
{
	jeRam_Free( pData );
}

void Brush_DeselectVert( Brush * pBrush, jeVertArray_Index  Index )
{

	assert( pBrush );
	assert( FaceList_GetNumFace( pBrush->pSelFaces ) == 0 );

	VertList_Remove( pBrush->pSelVert, Index, Brush_DestroyVertCB );
}// Brush_DeselectVert

void Brush_ToggleVert( Brush * pBrush, jeVertArray_Index Index)
{
	VertIterator	vI;

	assert( pBrush );
	if( VertList_SearchByIndex( pBrush->pSelVert, &Index, &vI ) )
	{
		Brush_DeselectVert( pBrush, Index);
	}
	else
		Brush_SelectVert( pBrush, Index );
}// Brush_ToggleVert

void Brush_DeselectAllVert( Brush * pBrush )
{
	VertIterator	vI;
	Vert_Struct	*	pVert;

	assert( pBrush );

	while( VertList_GetNumVert(pBrush->pSelVert)  )
	{
			pVert = VertList_GetFirstVert(pBrush->pSelVert, &vI );
			VertList_Remove( pBrush->pSelVert, pVert->Index, Brush_DestroyVertCB );
	}
}

typedef struct BrushMoveInfo {
	Brush * pBrush;
	jeVec3d * dWorldDist;
} BrushMoveInfo;

// Modified by CJP : 1/9/00

static jeBoolean Brush_MoveVertCB(void *pData, void *lParam)
{
	Vert_Struct	*	pVert =  (Vert_Struct*)pData;
	BrushMoveInfo	*bmI  = (BrushMoveInfo*)lParam;
	jeVec3d			NewPos;
	jeVec3d			dBrushDist;
	jeVertArray *	pArray;
	jeXForm3d 		BrushXForm;
	jeXForm3d		IBrushXForm; //Inverse of BrushXForm

	assert( pVert );
	assert( lParam );

	pArray = jeBrush_GetVertArray( bmI->pBrush->pBrush );
	pVert->LastPos = *jeVertArray_GetVertByIndex( pArray, pVert->Index );
	// CJP : shouldn't be neccesary.. 
	// NewPos = pVert->LastPos;

	Brush_GetXForm(bmI->pBrush, &BrushXForm);

	//Apply the inverse Transform to the vector to get it into brush space
	// CJP : I believe the inverse here is correct, not the transpose
	// jeXForm3d_GetTranspose( &BrushXForm, &IBrushXForm );
	jeXForm3d_GetInverse(&BrushXForm, &IBrushXForm);

	//The vector does not need to be translated only rotated and scaled
	//jeVec3d_Set( &IBrushXForm.Translation, pVert->LastPos.X, pVert->LastPos.Y, pVert->LastPos.Z );
	jeVec3d_Set( &IBrushXForm.Translation, 0.0f, 0.0f , 0.0f );
	jeXForm3d_Transform( &IBrushXForm, bmI->dWorldDist, &dBrushDist );

	jeVec3d_Add( &pVert->LastPos, &dBrushDist, &NewPos );

	jeVertArray_SetVertByIndex( pArray, pVert->Index, &NewPos );
	return( JE_TRUE );
}// Brush_MoveVertCB

// End modification.

void Brush_MoveSelectedVert( Brush * pBrush, jeVec3d *dWorldDist)
{
	BrushMoveInfo bmI;

	assert( pBrush );


	bmI.dWorldDist = dWorldDist;
	bmI.pBrush = pBrush;

	VertList_Enum( pBrush->pSelVert, &bmI, Brush_MoveVertCB );

	Brush_SetModified(pBrush); 
	pBrush->Flags |= BRUSH_FLAG_BOUNDSDIRTY;
} // Brush_MoveSelectedVert

// end cjp modification

static jeBoolean Brush_RestoreVertCB(void *pData, void *lParam)
{
	Vert_Struct	*	pVert =  (Vert_Struct*)pData;
	Brush		*	pBrush = ( Brush* )lParam;
	jeVertArray *	pArray;

	assert( pData );
	assert( lParam );
	pArray = jeBrush_GetVertArray( pBrush->pBrush );
	jeVertArray_SetVertByIndex( pArray, pVert->Index, &pVert->LastPos );
	return( JE_TRUE );
}// Brush_RestoreVertCB

void Brush_RestoreSelVert( Brush * pBrush )
{
	assert( pBrush );
	VertList_Enum( pBrush->pSelVert, pBrush, Brush_RestoreVertCB );
} // Brush_RestoreSelVert

jeBoolean Brush_SelectVertInRect( Brush * pBrush, jeExtBox *pSelBox )
{
	jeVertArray		*	pVerts ;
	int32				nVerts ;
	jeVec3d				Vert ;
	jeXForm3d			XForm ;
	jeVertArray_Index	i ;
	jeBoolean			bSelChanged = JE_FALSE;

	Brush_GetXForm( pBrush, &XForm ) ;
	pVerts = jeBrush_GetVertArray( pBrush->pBrush ) ;
		nVerts = jeVertArray_GetMaxIndex( pVerts );

	for( i=0; i<nVerts; i++ )
	{
		Vert = *jeVertArray_GetVertByIndex( pVerts, i ) ;
		jeXForm3d_Transform( &XForm, &Vert, &Vert ) ;
		if( jeExtBox_ContainsPoint ( pSelBox, &Vert ) )
		{
			Brush_SelectVert(  pBrush, i );
			bSelChanged =  JE_TRUE ;
		}
	}
	return( bSelChanged );
}
//
// TODO: Move to face module?
//
int32 Brush_FaceGetVertCount( jeBrush_Face * pFace )
{
	assert( pFace != NULL ) ;

	return jeBrush_FaceGetVertCount( pFace ) ;
}// Brush_FaceGetVertCount

const jeVec3d * Brush_FaceGetVertByIndex( jeBrush_Face * pFace, int32 Index )
{
	assert( pFace != NULL ) ;
	
	return jeBrush_FaceGetVertByIndex( pFace, Index ) ;
}// Brush_FaceGetVertByIndex

jeVec3d Brush_FaceGetWorldSpaceVertByIndex(const Brush * pBrush, const jeBrush_Face * pFace, int32 Index )
{
	jeVec3d Vert;
	jeXForm3d ModelXF;
	assert( pFace != NULL ) ;

	Vert = jeBrush_FaceGetWorldSpaceVertByIndex( pFace, Index );
	if( pBrush->pModel )
	{
		Model_GetXForm( pBrush->pModel, &ModelXF );
		jeXForm3d_Transform( &ModelXF, &Vert, &Vert );
	}
	return  Vert;
}// Brush_FaceGetWorldSpaceVertByIndex

jeBrush_Face * Brush_GetNextFace( const Brush * pBrush, jeBrush_Face * pStart )
{
	assert( pBrush != NULL ) ;
	assert( SIGNATURE == pBrush->nSignature ) ;
	assert( pBrush->pBrush != NULL ) ;

	return jeBrush_GetNextFace( pBrush->pBrush, pStart ) ;
}// Brush_GetNextFace

//
// ENUMERATION
//
void Brush_EnumFaceVerts( Brush * pBrush, void * pParam, BrushFaceVertCB Callback )
{
	jeBrush_Face *	pFace ;
	int				nVerts ;
	int				i ;
	FaceVertInfo	fvi ;
	assert( pBrush != NULL ) ;
	assert( SIGNATURE == pBrush->nSignature ) ;
	assert( Callback != NULL ) ;

	pFace = Brush_GetNextFace( pBrush, NULL ) ;
	fvi.pFace = pFace ;
	do
	{
		nVerts = Brush_FaceGetVertCount( pFace ) ;
		for( i=0; i<nVerts; i++ )
		{
			fvi.nVert = i ;
			Callback( &fvi, pParam ) ;
		}
		pFace = Brush_GetNextFace( pBrush, pFace ) ;
		fvi.pFace = pFace ;
	} while( pFace != NULL ) ;
}// Brush_EnumFaceVerts

// CALLBACK

jeBoolean Brush_ReattachCB( Brush *pBrush, void* lParam )
{
	BrushReattachInfo	* pbri = (BrushReattachInfo*)lParam ;
	
	//assert( pBrush->pBrush == NULL ) ;
	pBrush->pModel =  pbri->pModel;
	pBrush->pWorld =  pbri->pWorld;
	//pBrush->pBrush	= pbri->pgeBrush ;
	pbri->nIndexTag	= BRUSH_REATTACH_GOOD ;
	return JE_TRUE ;

}// Brush_ReattachCB


// FILE HANDLING
Brush * Brush_CreateFromFile( jeVFile * pF, const int32 nVersion, jePtrMgr * pPtrMgr )
{
	Brush	*	pBrush = NULL ;
	assert( jeVFile_IsValid( pF ) ) ;
	assert( nVersion <= BRUSH_VERSION ) ;
	
	if( BRUSH_VERSION != nVersion )
	{
		jeErrorLog_AddString(JE_ERR_FILEIO_READ, "Brush_CreateFromFile Version.\n", NULL);
		return NULL ;
	}
	
	pBrush = Brush_Create( "", NULL, 0 ) ;
	if( pBrush == NULL )
		goto BCFF_FAILURE ;

	if( !Object_InitFromFile( pF, &pBrush->ObjectData ) )
		goto BCFF_FAILURE ;

	if( !jeVFile_Read( pF, &pBrush->Kind, sizeof pBrush->Kind ) )
		goto BCFF_FAILURE ;

	pBrush->pBrush = jeBrush_CreateFromFile( pF, pPtrMgr );
	pBrush->pTemplate = BrushTemplate_CreateFromFile( pF );
	return pBrush ;

BCFF_FAILURE :
	if( pBrush != NULL )
		Object_Free( (Object**)pBrush ) ;

	jeErrorLog_AddString(JE_ERR_FILEIO_READ, "Brush_CreateFromFile.\n", NULL);
	return NULL ;

}// Brush_CreateFromFile


jeBoolean Brush_WriteToFile( Brush * pBrush, Brush_WriteInfo * pWriteInfo)
{
	jeVFile * pF = pWriteInfo->pF;

	assert( pBrush != NULL ) ;
	assert( SIGNATURE == pBrush->nSignature ) ;
	assert( jeVFile_IsValid( pF ) ) ;

	if( !Object_WriteToFile( &pBrush->ObjectData, pF  ) )
	{
		jeErrorLog_AddString(JE_ERR_FILEIO_WRITE, "Object_InitFromFile.\n", NULL);
		return JE_FALSE;
	}

	if( jeVFile_Write( pF, &pBrush->Kind, sizeof pBrush->Kind ) == JE_FALSE )
	{
		jeErrorLog_AddString(JE_ERR_FILEIO_WRITE, "Brush_WriteToFile.\n", NULL);
		return JE_FALSE;
	}


	if( jeBrush_WriteToFile( pBrush->pBrush, pF, pWriteInfo->pPtrMgr ) == JE_FALSE )
	{
		jeErrorLog_AddString(JE_ERR_FILEIO_WRITE, "Brush_WriteToFile.\n", NULL);
		return JE_FALSE;
	}

	BrushTemplate_WriteToFile( pBrush->pTemplate, pF );
	return JE_TRUE ;

}// Brush_WriteToFile


// DEBUGGING
#ifdef _DEBUG
jeBoolean Brush_IsValid( const Brush * pBrush )
{
	assert( pBrush != NULL ) ;
	return SIGNATURE == pBrush->nSignature ;
}
jeBoolean Brush_IsBoundsValid( Brush * pBrush )
{
	assert( pBrush != NULL ) ;
	assert( SIGNATURE == pBrush->nSignature ) ;
	return( JE_TRUE );
}//Brush_IsBoundsValid

#endif //_DEBUG

//PRESENTATION
typedef struct tagVertDrawInfo
{
	int32			hDC ;
	Brush		*   pBrush;
	const Ortho	*	pOrtho ;
} VertDrawInfo ;


static jeBoolean Brush_DrawSelectedVertsCB( Vert_Struct *pVert, void * pVoid )
{
	VertDrawInfo *	pvdi = (VertDrawInfo*)pVoid ;
	jeVec3d			Vert ;
	Point			point ;
	HDC     		hVertexBitmap;


	// Jeff:  Load select vertex bitmap from resources - 8/18/2005
	hVertexBitmap = CreateCompatibleDC((HDC)pvdi->hDC);
	SelectObject(hVertexBitmap, (HBITMAP)AppData_GetSelectedVertex());
	Brush_GetVertexPoint( pvdi->pBrush, pVert->Index, &Vert ) ;
	Ortho_WorldToView( pvdi->pOrtho, &Vert, &point ) ;	
	BitBlt((HDC)pvdi->hDC, point.X-3, point.Y-3, 6, 6,hVertexBitmap, 0, 0, SRCCOPY) ;
	DeleteDC(hVertexBitmap);
	
	return JE_TRUE ;
}// Ortho_DrawSelectedVertsCB

#define BRUSH_MAXPOINTSPERFACE			(64)

void Brush_RenderOrthoFaces( Brush *pBrush, const Ortho * pOrtho,  int32 hDC, jeBoolean bDrawVertex, jeBoolean bDrawSelFaces, jeBoolean bColorOveride )
{
	int				i, j ;
	jeBrush_Face *	pFace ;
	Point			points[BRUSH_MAXPOINTSPERFACE];
	jeVec3d			Vert ;
	int				nVertices ;
	jeBoolean		bFaceSelected ;
	HDC         	hVertexBitmap;
	jwePen	*		pPen = NULL;

	assert( pOrtho != NULL ) ;
	assert( SIGNATURE == pBrush->nSignature ) ;
	assert( pBrush != NULL ) ;

    if (!pBrush->bShow) return;
	
	// Jeff:  Load vertex bitmap from resources - 8/18/2005
	if( bDrawVertex )
	{
		hVertexBitmap = CreateCompatibleDC((HDC)hDC);
		SelectObject(hVertexBitmap, (HBITMAP)AppData_GetVertex());
	}

	if( ! bColorOveride )
	{
		if( Brush_IsCutBrush( pBrush ) )
		{
			pPen =	Pen_SelectSubtractBrushColor( hDC ) ;
		}
		else
		{
			pPen =	Pen_SelectAddBrushColor( hDC ) ;
		}
	}

	i = 0;
	pFace = Brush_GetNextFace( pBrush, NULL ) ;
	do
	{
		bFaceSelected = Brush_IsFaceSelected( pBrush, pFace ) ;
		if( !(bDrawSelFaces && bFaceSelected) )
		{
			nVertices = Brush_FaceGetVertCount( pFace ) ;
			assert( nVertices < BRUSH_MAXPOINTSPERFACE ) ;
			for( i=0; i<nVertices; i++ )
			{
				Vert = Brush_FaceGetWorldSpaceVertByIndex( pBrush, pFace, i ) ;
				Ortho_WorldToView( pOrtho, &Vert, &points[i] ) ;
			}
			points[i++] = points[0] ;

			// It would appear we just let GDI clip...
			Pen_Polyline( hDC, points, i ) ;	// hDC==hDrawDC
		}

		if( bDrawVertex )
		{
			i = Brush_FaceGetVertCount( pFace ) ;
			for( j=0; j<i; j++ )
				BitBlt((HDC) hDC, points[j].X-3, points[j].Y-3, 6, 6, hVertexBitmap, 0, 0, SRCCOPY );
		}
		pFace = Brush_GetNextFace( pBrush, pFace ) ;
	} while( pFace != NULL ) ;

	if( ! bColorOveride && pPen )
		Pen_Release( pPen, hDC );
	if( bDrawVertex  )
		DeleteDC(hVertexBitmap );

	// 2nd pass for selected faces?
	if( bDrawSelFaces )
	{
        FaceIterator fI;
		pPen = Pen_SelectSelectedFaceColor( hDC );

        pFace = FaceList_GetFirstFace(pBrush->pSelFaces, &fI ) ;
		while (pFace)
		{
			if( Brush_IsFaceSelected( pBrush, pFace ) )
			{
				nVertices = Brush_FaceGetVertCount( pFace ) ;
				assert( nVertices < BRUSH_MAXPOINTSPERFACE ) ;
				for( i=0; i<nVertices; i++ )
				{
					Vert = Brush_FaceGetWorldSpaceVertByIndex( pBrush, pFace, i ) ;
					Ortho_WorldToView( pOrtho, &Vert, &points[i] ) ;
				}
				points[i++] = points[0] ;

				// It would appear we just let GDI clip...
				Pen_Polyline( hDC, points, i ) ;	// hDC==hDrawDC
			}
            pFace = FaceList_GetNextFace( pBrush->pSelFaces, &fI ) ;
		};

		Pen_Release( pPen, hDC );
	}// Draw Sel Faces
	
	if( bDrawVertex )
	{
		VertDrawInfo	vdi ;
		VertList	*	pVerts ;
	
		// If any verts are selected, draw the verts
		pVerts = Brush_GetSelVert( pBrush ) ;
		if( VertList_GetNumVert( pVerts ) )
		{
			vdi.hDC = hDC ;
			vdi.pBrush = pBrush ;
			vdi.pOrtho = pOrtho ;
			VertList_Enum( pVerts, &vdi, Brush_DrawSelectedVertsCB ) ;
		}

	}

}// Brush_RenderOrthoFaces

//UNDO
jeBoolean Brush_RestoreMaterialCB( Object *pObject, void *Context )
{
	MaterialUndoContext *pUndoContext = (MaterialUndoContext*)Context;
	jeFaceInfo FaceInfo;

	jeBrush_FaceGetFaceInfo( pUndoContext->pFace, &FaceInfo );

	FaceInfo.MaterialIndex = pUndoContext->MaterialIndex;
	jeBrush_FaceSetFaceInfo( pUndoContext->pFace, &FaceInfo);
	if( pUndoContext->pgeModel != NULL )
		jeModel_UpdateBrushFace( pUndoContext->pgeModel,  pUndoContext->pFace, JE_TRUE );
	return( JE_TRUE );
	pObject;
}

void Brush_DestroyMaterialContextCB( void *Context )
{
	jeRam_Free( Context );
}

jeBoolean Brush_SelectClosest( Brush * pBrush, FindInfo	*	pFindInfo )
{
	int					i, j ; 
	int					nFaces ;
	int					nVerts ;
	jeBrush_Face	*	pFace ; 
	Point				pt1, pt2 ;
	jeVec3d				wpt1, wpt2 ;
	jeFloat				DistSq ;
	Model			*	pModel;

	assert( pBrush != NULL );
	assert( pFindInfo != NULL );
	assert( pFindInfo->pOrtho  != NULL ) ;
	nFaces = Brush_GetFaceCount( pBrush ) ;
	
	for( i=0; i<nFaces; i++ )
	{
		pFace = Brush_GetFaceByIndex( pBrush, i ) ;
		
		nVerts = Brush_FaceGetVertCount( pFace ) ;
		wpt1 = Brush_FaceGetWorldSpaceVertByIndex( pBrush, pFace, 0 ) ;
		Ortho_WorldToView( pFindInfo->pOrtho, &wpt1, &pt1 ) ;
		for( j=1; j<nVerts; j++ )
		{
			wpt2 = Brush_FaceGetWorldSpaceVertByIndex( pBrush,  pFace, j ) ;
			Ortho_WorldToView( pFindInfo->pOrtho, &wpt2, &pt2 ) ;

			DistSq = Util_PointToLineDistanceSquared( &pt1, &pt2, pFindInfo->pViewPt ) ;
			if( DistSq < pFindInfo->fMinDistance )
			{
				pFindInfo->fMinDistance = DistSq ;
				pModel = Brush_GetModel( pBrush );
				if( pBrush->pModel && Model_IsLocked( pModel ) )
					pFindInfo->pObject = (Object*)pModel ;
				else
					pFindInfo->pObject = (Object*)pBrush ;
				pFindInfo->nFace = i ;
				pFindInfo->nFaceEdge = j-1;
			}
			wpt1 = wpt2 ;
			pt1 = pt2 ;
		}
		wpt2 = Brush_FaceGetWorldSpaceVertByIndex( pBrush, pFace, 0 ) ;
		Ortho_WorldToView( pFindInfo->pOrtho, &wpt2, &pt2 ) ;

		DistSq = Util_PointToLineDistanceSquared( &pt1, &pt2, pFindInfo->pViewPt ) ;
		if( DistSq < pFindInfo->fMinDistance )
		{
			pFindInfo->fMinDistance = DistSq ;
			pFindInfo->pObject = (Object*)pBrush ;
			pFindInfo->nFace = i ;
			pFindInfo->nFaceEdge = j-1;
		}
	}
	return( JE_TRUE );
}

jeProperty_List *	Brush_GlobalPropertyList()
{
	jeProperty_List * pList;
	jeProperty	Property;
	char *	Name;
	jeBoolean bCheck;

	pList  =  jeProperty_ListCreate(0);
	if( pList == NULL )
		return( NULL );
	
	Name = Util_LoadLocalRcString( IDS_UPDATE ) ;
	jeProperty_FillGroup( &Property, Name, BRUSH_GLOBAL_UPDATEGROUP_ID );
	if( !jeProperty_Append( pList, &Property ) )
	{
		jeErrorLog_Add( JE_ERR_SUBSYSTEM_FAILURE, "Brush_GlobalPropertyList:jeProperty_Append");
		jeProperty_ListDestroy( &pList );
		return( NULL );
	}

	Name = Util_LoadLocalRcString( IDS_UPDATE_MANUEL ) ;
	bCheck = (gBrush_Update == OBJECT_UPDATE_MANUEL );
	jeProperty_FillRadio( &Property, Name, bCheck, BRUSH_GLOBAL_UPDATE_MANUEL_ID );
	if( !jeProperty_Append( pList, &Property ) )
	{
		jeErrorLog_Add( JE_ERR_SUBSYSTEM_FAILURE, "Brush_GlobalPropertyList:jeProperty_Append");
		jeProperty_ListDestroy( &pList );
		return( NULL );
	}
	
	Name = Util_LoadLocalRcString( IDS_UPDATE_CHANGE ) ;
	bCheck = (gBrush_Update == OBJECT_UPDATE_CHANGE );
	jeProperty_FillRadio( &Property, Name, bCheck, BRUSH_GLOBAL_UPDATE_CHANGE_ID );
	if( !jeProperty_Append( pList, &Property ) )
	{
		jeErrorLog_Add( JE_ERR_SUBSYSTEM_FAILURE, "Brush_GlobalPropertyList:jeProperty_Append");
		jeProperty_ListDestroy( &pList );
		return( NULL );
	}

	Name = Util_LoadLocalRcString( IDS_UPDATE_REALTIME ) ;
	bCheck = (gBrush_Update == OBJECT_UPDATE_REALTIME );
	jeProperty_FillRadio( &Property, Name, bCheck, BRUSH_GLOBAL_UPDATE_REALTIME_ID );
	jeProperty_SetDisabled( &Property, JE_TRUE );
	if( !jeProperty_Append( pList, &Property ) )
	{
		jeErrorLog_Add( JE_ERR_SUBSYSTEM_FAILURE, "Brush_GlobalPropertyList:jeProperty_Append");
		jeProperty_ListDestroy( &pList );
		return( NULL );
	}

	jeProperty_FillGroupEnd( &Property, BRUSH_GLOBAL_UPDATEGROUP_END_ID );
	if( !jeProperty_Append( pList, &Property ) )
	{
		jeErrorLog_Add( JE_ERR_SUBSYSTEM_FAILURE, "Brush_GlobalPropertyList:jeProperty_Append");
		jeProperty_ListDestroy( &pList );
		return( NULL );
	}

	Name = Util_LoadLocalRcString( IDS_MAINTAIN_LIGHTING ) ;
	jeProperty_FillCheck( &Property, Name, gBrush_Lighting, BRUSH_GLOBAL_MAINTAIN_LIGHING_ID );
	if( !jeProperty_Append( pList, &Property ) )
	{
		jeErrorLog_Add( JE_ERR_SUBSYSTEM_FAILURE, "Brush_GlobalPropertyList:jeProperty_Append");
		jeProperty_ListDestroy( &pList );
		return( NULL );
	}
	
	return( pList );
}

void Brush_SetGlobalProperty( int DataId, int DataType, jeProperty_Data * pData )
{
	switch( DataId )
	{
		case BRUSH_GLOBAL_UPDATE_MANUEL_ID:
			if( pData->Bool )
			{
				gBrush_Update = OBJECT_UPDATE_MANUEL;
			}
			break;

		case BRUSH_GLOBAL_UPDATE_CHANGE_ID:
			if( pData->Bool )
			{
				gBrush_Update = OBJECT_UPDATE_CHANGE;
			}
			break;

		case BRUSH_GLOBAL_UPDATE_REALTIME_ID:
			if( pData->Bool )
			{
				gBrush_Update = OBJECT_UPDATE_REALTIME;
			}
			break;

		case BRUSH_GLOBAL_MAINTAIN_LIGHING_ID:
			gBrush_Lighting =  pData->Bool;
			break;
	}
	DataType;
}

jeBoolean Brush_IsVisible( const Brush* pBrush )
{
    assert(pBrush);
    return pBrush->bShow;
}

void Brush_Show( Brush* pBrush, jeBoolean Visible )
{
    assert(pBrush);
    pBrush->bShow = Visible;
}

/* EOF: Brush.c */