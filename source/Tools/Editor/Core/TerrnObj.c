/****************************************************************************************/
/*  TERRNOBJ.C                                                                          */
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

#pragma warning(disable : 4206)

#if 0 //{

/* Terrain.c */

#include <Assert.h>
#include <Memory.h>
#include <String.h>
#include <math.h>

#include "ErrorLog.h"
#include "jet.h"
#include "Ram.h"
#include "Transform.h"
#include "Util.h"
#include "resource.h"

#include "Terrain.h"
#include "ObjectDef.h"
#include "TerrnObj.h"
#include "Object.h"

#define SIGNATURE			'LITE'

#define TERRAIN_BOX_WIDTH			512.0f
#define TERRAIN_BOX_HEIGHT			20.0f
#define TERRAIN_BOX_DEPTH			512.0f
#define TERRAIN_FLAG_DIRTY			0x0001
#define TERRAIN_FLAG_WBOUNDSDIRTY		0x0002
#define TERRAIN_FLAG_DIRTYALL			TERRAIN_FLAG_DIRTY | TERRAIN_FLAG_WBOUNDSDIRTY
#define TERRAIN_MAXNAMELENGTH	(31)
#define TERRAIN_DEFAULT_WIDTH		257
#define TERRAIN_DEFAULT_HEIGHT		257

typedef struct tagTerrain
{
	Object				ObjectData ;
#ifdef _DEBUG
	int					nSignature ;
#endif
	jeWorld				*	pWorld ; //Not mine do not destroy
	int32				Flags;
	jeExtBox			WorldBounds ;
	jeTerrain		*	TerrainData ;
	jeObject		*	jeObjectData;
	jeBitmap		*	HeightMap ;
	jeBitmap		*	TerrainMap ;
	jeVec3d				Center ;
	jeVec3d				Size ;
} Terrain ;

//STATIC FUNCTIONS

static void Terrain_SetData( Terrain * pTerrain )
{
	pTerrain;
}

static void Terrain_SizeEdge( Terrain * pTerrain, const jeVec3d * pStillEdge, const jeFloat fScale, ORTHO_AXIS Axis )
{
	float	fTemp;


	fTemp = jeVec3d_GetElement( &pTerrain->Center, Axis ) - jeVec3d_GetElement( pStillEdge, Axis ) ;
	fTemp = fTemp * fScale ;
	fTemp = fTemp + jeVec3d_GetElement( pStillEdge, Axis ) ;
	jeVec3d_SetElement( &pTerrain->Center, Axis, fTemp ) ;
	Terrain_SetData( pTerrain );
}

static jeBitmap* Terrain_CreateDefaultMap( )
{
	jeBitmap* pHeightMap;
	jeBitmap * Lock;
	jeBoolean success;
	jeBitmap_Info Info;
	uint8 *bits,*bptr;
	int x,y;

	pHeightMap = jeBitmap_Create(TERRAIN_DEFAULT_WIDTH, TERRAIN_DEFAULT_HEIGHT, 1, JE_PIXELFORMAT_8BIT ); 
	success = jeBitmap_LockForWriteFormat(pHeightMap,&Lock,0,0,JE_PIXELFORMAT_8BIT_PAL);
	if ( ! success )
	{

		success = jeBitmap_SetFormat(pHeightMap,JE_PIXELFORMAT_8BIT,JE_TRUE,0,NULL);
		assert(success);
		success = jeBitmap_LockForWriteFormat(pHeightMap,&Lock,0,0,JE_PIXELFORMAT_8BIT);
		assert(success);
	}
	success = jeBitmap_GetInfo(Lock,&Info,NULL);
	assert(success);

	//seting the palette 0 to green so default terrain map will be all green
	{
	jeBitmap_Palette * Pal;


		Pal = jeBitmap_Palette_Create(JE_PIXELFORMAT_24BIT_RGB,256);
		assert(Pal);


		success = jeBitmap_Palette_SetEntryColor(Pal,0,0,255,0,255);
		assert(success);

		success = jeBitmap_SetPalette(pHeightMap,Pal);
		assert(success);
	}
	// you can only call _GetBits on a locked bitmap

	bits = jeBitmap_GetBits(Lock);
	assert( bits );

	bptr = bits;
	for(y=0; y < Info.Height; y++)
	{
		for(x=0; x < Info.Width; x++)
		{

			*bptr++ = (uint8)(sin( sqrt(x*x + y*y)*JE_PI/16.0 )*127 + 127);
		}

		bptr += Info.Stride -  Info.Width;
	}
	bits = bptr = NULL;

	// you call Unlock on all the mips you locked - not on the original bitmap!

	success = jeBitmap_UnLock(Lock);
	assert(success);
	return( pHeightMap );
}

// CREATORS
Terrain *	Terrain_Create( jeWorld	* pWorld, Group * pGroup, const char * const pszName, int32 nNumber, jeBitmap *HeightMap, jeBitmap * TerrainMap )
{
	Terrain	*	pTerrain;
	assert( pszName );
	assert( HeightMap );
	assert( TerrainMap );

	pTerrain = JE_RAM_ALLOCATE_STRUCT( Terrain );
	if( pTerrain == NULL )
	{
		jeErrorLog_Add( JE_ERR_MEMORY_RESOURCE, "Unable to allocate Terrain" );
		return( NULL );
	}
	memset( pTerrain, 0, sizeof( Terrain ) );
	assert( (pTerrain->nSignature = SIGNATURE) == SIGNATURE ) ;	// ASSIGN
	if( !Object_Init( &pTerrain->ObjectData, pGroup, KIND_TERRAIN, pszName, nNumber ) )
	{
		jeErrorLog_Add( JE_ERR_INTERNAL_RESOURCE, "Trace" );
		jeRam_Free( pTerrain );
		return( NULL );
	}
	pTerrain->pWorld = pWorld;
	pTerrain->HeightMap = HeightMap;
	pTerrain->TerrainMap = TerrainMap;

	jeVec3d_Set( &pTerrain->Center, 0.0f, 0.0f, 0.0f );
	jeVec3d_Set( &pTerrain->Size,	TERRAIN_BOX_WIDTH, TERRAIN_BOX_HEIGHT, TERRAIN_BOX_DEPTH );
	pTerrain->TerrainData = jeTerrain_CreateFromBitmap(pTerrain->HeightMap, &pTerrain->Center, &pTerrain->Size);
	if( pTerrain->TerrainData == NULL )
	{
		jeErrorLog_Add( JE_ERR_INTERNAL_RESOURCE, "Trace" );
		jeRam_Free( pTerrain );
		return( NULL );
	}
	//jeTerrain_SetTextures( pTerrain->TerrainData,&TerrainMap, 1);
	Terrain_UpdateBounds( pTerrain );

	pTerrain->jeObjectData = jeObject_Create("Terrain");
	jeTerrain_InitObject( pTerrain->TerrainData,pTerrain->jeObjectData);
	jeWorld_AddObject( pTerrain->pWorld, pTerrain->jeObjectData);
	
	return( pTerrain );
}// Terrain_Create


Terrain *	Terrain_Copy( Terrain *	pTerrain, int32 nNumber )
{
	Terrain *pNewTerrain;

	
	assert( pTerrain );
	assert( SIGNATURE == pTerrain->nSignature ) ;

	pNewTerrain = JE_RAM_ALLOCATE_STRUCT( Terrain );
	if( pNewTerrain == NULL )
	{
		jeErrorLog_Add( JE_ERR_MEMORY_RESOURCE, "Unable to allocate Terrain" );
		return( NULL );
	}
	memset( pNewTerrain, 0, sizeof( Terrain ) );
	assert( (pNewTerrain->nSignature = SIGNATURE) == SIGNATURE ) ;	// ASSIGN
	if( !Object_Init( &pNewTerrain->ObjectData, pTerrain->ObjectData.pGroup, KIND_TERRAIN, pTerrain->ObjectData.pszName, nNumber ) )
	{
		jeErrorLog_Add( JE_ERR_INTERNAL_RESOURCE, "Trace" );
		jeRam_Free( pNewTerrain );
		return( NULL );
	}
	pNewTerrain->pWorld = pTerrain->pWorld;
	pNewTerrain->HeightMap = pTerrain->HeightMap;
	jeBitmap_CreateRef(pTerrain->HeightMap);
	pNewTerrain->TerrainMap = pTerrain->TerrainMap;
	jeBitmap_CreateRef(pTerrain->TerrainMap);
	pNewTerrain->Center = pTerrain->Center;
	pNewTerrain->Size = pTerrain->Size;
	pNewTerrain->WorldBounds = pTerrain->WorldBounds;
	pNewTerrain->TerrainData = jeTerrain_CreateFromBitmap(pTerrain->HeightMap, &pTerrain->Center, &pTerrain->Size);
	if( pNewTerrain->TerrainData == NULL )
	{
		jeErrorLog_Add( JE_ERR_INTERNAL_RESOURCE, "Trace" );
		jeRam_Free( pNewTerrain );
		return( NULL );
	}

	pNewTerrain->jeObjectData = jeObject_Create("Terrain");
	jeTerrain_InitObject( pNewTerrain->TerrainData,pNewTerrain->jeObjectData);
	jeWorld_AddObject( pNewTerrain->pWorld, pNewTerrain->jeObjectData);

	return( pNewTerrain );
}

Terrain *	Terrain_FromTemplate( char * pszName, Group * pGroup, Terrain *	pTerrain, int32 nNumber )
{
	Terrain *pNewTerrain;

	
	assert( pszName );
	assert( pTerrain );

	pNewTerrain = Terrain_Copy( pTerrain, nNumber );
	if( pNewTerrain == NULL )
	{
		jeErrorLog_Add( JE_ERR_INTERNAL_RESOURCE, "Trace" );
		return( NULL );
	}
	if( pNewTerrain->ObjectData.pszName != NULL )
	{
		jeRam_Free( pNewTerrain->ObjectData.pszName );
	}
	pNewTerrain->ObjectData.pszName = pszName;
	pNewTerrain->ObjectData.pGroup = pGroup ;
	Terrain_UpdateBounds( pNewTerrain );
	return( pNewTerrain );
}

Terrain * Terrain_CreateTemplate( jeWorld * pWorld )
{
	Terrain	*	pTerrain;

	pTerrain = JE_RAM_ALLOCATE_STRUCT( Terrain );
	if( pTerrain == NULL )
	{
		jeErrorLog_Add( JE_ERR_MEMORY_RESOURCE, "Unable to allocate Terrain" );
		return( NULL );
	}
	memset( pTerrain, 0, sizeof( Terrain ) );
	assert( (pTerrain->nSignature = SIGNATURE) == SIGNATURE ) ;	// ASSIGN
	if( !Object_Init( &pTerrain->ObjectData, NULL, KIND_TERRAIN, "Terrain", 0 ) )
	{
		jeErrorLog_Add( JE_ERR_INTERNAL_RESOURCE, "Terrain_CreateTemplate:Object_Init" );
		jeRam_Free( pTerrain );
		return( NULL );
	}
	pTerrain->pWorld = pWorld;
	pTerrain->HeightMap = Terrain_CreateDefaultMap();
	if( pTerrain->HeightMap == NULL )
	{
		jeErrorLog_Add( JE_ERR_INTERNAL_RESOURCE, "Terrain_CreateTemplate:Terrain_CreateDefaultMap" );
		return( NULL );
	}
	pTerrain->TerrainMap = Terrain_CreateDefaultMap();
	if( pTerrain->TerrainMap == NULL )
	{
		jeErrorLog_Add( JE_ERR_INTERNAL_RESOURCE, "Terrain_CreateTemplate:Terrain_CreateDefaultMap" );
		return( NULL );
	}

	jeVec3d_Set( &pTerrain->Center, 0.0f, 0.0f, 0.0f );
	jeVec3d_Set( &pTerrain->Size,	TERRAIN_BOX_WIDTH, TERRAIN_BOX_HEIGHT, TERRAIN_BOX_DEPTH );
	pTerrain->TerrainData = NULL;
	pTerrain->jeObjectData = NULL;
	Terrain_UpdateBounds( pTerrain );

	return( pTerrain );
}

char *	Terrain_CreateDefaultName(  )
{
	return( Util_LoadLocalRcString( IDS_TERRAIN ) );
}

void Terrain_Destroy( Terrain ** ppTerrain ) 
{
	if( (*ppTerrain)->TerrainMap )
		jeBitmap_Destroy( &(*ppTerrain)->TerrainMap );
	if( (*ppTerrain)->HeightMap )
		jeBitmap_Destroy( &(*ppTerrain)->HeightMap );
	if( (*ppTerrain)->TerrainData  != NULL )
		jeTerrain_Destroy(&(*ppTerrain)->TerrainData);
	jeRam_Free( (*ppTerrain) );
}// Terrain_Destroy


// MODIFIERS
void Terrain_Move( Terrain * pTerrain, const jeVec3d * pWorldDistance )
{
	assert( pTerrain != NULL ) ;
	assert( SIGNATURE == pTerrain->nSignature ) ;

	Terrain_SetModified( pTerrain );
	jeVec3d_Add( &pTerrain->Center, pWorldDistance, &pTerrain->Center );
	Terrain_SetData( pTerrain );

}// Terrain_Move

void Terrain_Size( Terrain * pTerrain, const jeExtBox * pSelectedBounds, const jeFloat hScale, const jeFloat vScale, SELECT_HANDLE eSizeType, ORTHO_AXIS HAxis, ORTHO_AXIS VAxis )
{
	assert( pTerrain != NULL ) ;
	assert( SIGNATURE == pTerrain->nSignature ) ;

	// The Z axis is flipped (down is positive)
	// We just determine the edge and call _SizeEdge once or twice to keep this
	// as simple as possible
	switch( eSizeType )
	{
	case Select_Top :
		if( Ortho_Axis_Z == VAxis )
			Terrain_SizeEdge( pTerrain, &pSelectedBounds->Max, 1.0f/vScale, VAxis ) ;
		else
			Terrain_SizeEdge( pTerrain, &pSelectedBounds->Min, vScale, VAxis ) ;
		break ;

	case Select_Bottom :
		if( Ortho_Axis_Z == VAxis )
			Terrain_SizeEdge( pTerrain, &pSelectedBounds->Min, vScale, VAxis ) ;
		else
			Terrain_SizeEdge( pTerrain, &pSelectedBounds->Max, 1.0f/vScale, VAxis ) ;
		break ;

	case Select_Left :
		Terrain_SizeEdge( pTerrain, &pSelectedBounds->Max, 1.0f/hScale, HAxis ) ;
		break ;

	case Select_Right :
		Terrain_SizeEdge( pTerrain, &pSelectedBounds->Min, hScale, HAxis ) ;
		break ;

	case Select_TopLeft :
		if( Ortho_Axis_Z == VAxis )
			Terrain_SizeEdge( pTerrain, &pSelectedBounds->Max, 1.0f/vScale, VAxis ) ;
		else
			Terrain_SizeEdge( pTerrain, &pSelectedBounds->Min, vScale, VAxis ) ;
		Terrain_SizeEdge( pTerrain, &pSelectedBounds->Max, 1.0f/hScale, HAxis ) ;
		break ;		

	case Select_TopRight :
		if( Ortho_Axis_Z == VAxis )
			Terrain_SizeEdge( pTerrain, &pSelectedBounds->Max, 1.0f/vScale, VAxis ) ;
		else
			Terrain_SizeEdge( pTerrain, &pSelectedBounds->Min, vScale, VAxis ) ;
		Terrain_SizeEdge( pTerrain, &pSelectedBounds->Min, hScale, HAxis ) ;
		break ;

	case Select_BottomLeft :
		if( Ortho_Axis_Z == VAxis )
			Terrain_SizeEdge( pTerrain, &pSelectedBounds->Min, vScale, VAxis ) ;
		else
			Terrain_SizeEdge( pTerrain, &pSelectedBounds->Max, 1.0f/vScale, VAxis ) ;
		Terrain_SizeEdge( pTerrain, &pSelectedBounds->Max, 1.0f/hScale, HAxis ) ;
		break ;
	
	case Select_BottomRight :
		if( Ortho_Axis_Z == VAxis )
			Terrain_SizeEdge( pTerrain, &pSelectedBounds->Min, vScale, VAxis ) ;
		else
			Terrain_SizeEdge( pTerrain, &pSelectedBounds->Max, 1.0f/vScale, VAxis ) ;
		Terrain_SizeEdge( pTerrain, &pSelectedBounds->Min, hScale, HAxis ) ;
		break ;
	}
	Terrain_SetModified( pTerrain ) ;

}// Terrain_Size

void Terrain_SetXForm( Terrain * pTerrain, const jeXForm3d * XForm )
{

	assert( pTerrain );
	assert( SIGNATURE == pTerrain->nSignature ) ;
	assert( XForm );

	pTerrain->Center = XForm->Translation;
	Terrain_SetData( pTerrain );
	Terrain_SetModified( pTerrain );

}// Terrain_SetXForm


void Terrain_UpdateBounds( Terrain * pTerrain )
{

	assert( pTerrain );

	jeExtBox_SetTranslation ( &pTerrain->WorldBounds, &pTerrain->Center );
	jeExtBox_Set (  &pTerrain->WorldBounds,
					pTerrain->Center.X - pTerrain->Size.X/2.0f,
					pTerrain->Center.Y - pTerrain->Size.Y/2.0f,
					pTerrain->Center.Z - pTerrain->Size.Z/2.0f,
					pTerrain->Center.X + pTerrain->Size.X/2.0f,
					pTerrain->Center.Y + pTerrain->Size.Y/2.0f,
					pTerrain->Center.Z + pTerrain->Size.Z/2.0f
	);

}

void Terrain_SetModified( Terrain * pTerrain )
{
	assert( pTerrain != NULL ) ;
	assert( SIGNATURE == pTerrain->nSignature ) ;
	
	pTerrain->Flags |= TERRAIN_FLAG_DIRTYALL ;	
}// Terrain_SetModified


// ACCESSORS
void Terrain_GetXForm( const Terrain * pTerrain, jeXForm3d * XForm )
{
	jeXForm3d_SetIdentity( XForm );
	XForm->Translation = pTerrain->Center;
}

const jeExtBox * Terrain_GetWorldAxialBounds( const Terrain * pTerrain )
{
	assert( pTerrain != NULL ) ;
	assert( SIGNATURE == pTerrain->nSignature ) ;
	
	if( pTerrain->Flags & TERRAIN_FLAG_WBOUNDSDIRTY )
	{	
		Terrain * pEvalTerrain = (Terrain*)pTerrain ;			// Lazy Evaluation requires removing the const
		Terrain_UpdateBounds( pEvalTerrain ) ;
	}

	return &pTerrain->WorldBounds ;

}// Terrain_GetWorldAxialBounds

jeTerrain *			Terrain_GetTerrain( const Terrain * pTerrain )
{
	assert( pTerrain );

	return( pTerrain->TerrainData );
}

jeBoolean Terrain_SelectClosest( Terrain * pTerrain, FindInfo	*	pFindInfo )
{
	int					i ; 
	Point				points[5] ;
	jeFloat				DistSq ;
	const jeExtBox *	Bounds ;


	assert( pTerrain != NULL );
	assert( pFindInfo != NULL );
	assert( pFindInfo->pOrtho  != NULL ) ;
	
	Bounds = Terrain_GetWorldAxialBounds( pTerrain ) ;
	Ortho_WorldToView( pFindInfo->pOrtho, &Bounds->Min, &points[0] ) ;
	Ortho_WorldToView( pFindInfo->pOrtho, &Bounds->Max, &points[2] ) ;

	points[1] = points[0];
	points[1].Y = points[2].Y;

	points[3] = points[2];
	points[3].Y = points[0].Y;

	points[4] = points[0];

	for( i=0; i<4; i++ )
	{		
		DistSq = Util_PointToLineDistanceSquared( &points[i], &points[i+1], pFindInfo->pViewPt ) ;
		if( DistSq < pFindInfo->fMinDistance )
		{
			pFindInfo->fMinDistance = DistSq ;
			pFindInfo->pObject = (Object*)pTerrain ;
			pFindInfo->nFace = i ;
			pFindInfo->nFaceEdge = 0;
		}
	}
	return( JE_TRUE );
}

//IS
jeBoolean	Terrain_IsInRect( const Terrain * pTerrain, jeExtBox *pSelRect, jeBoolean bSelEncompeses )
{
	const jeExtBox *pWorldBounds;
	jeExtBox		Result;

	assert( pTerrain );
	assert( pSelRect );
	
	pWorldBounds = Terrain_GetWorldAxialBounds( pTerrain ) ;
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
}//Terrain_IsInRect


//FILE
Terrain * Terrain_CreateFromFile( jeVFile * pF )
{
	Terrain	*	pTerrain = NULL ;
	assert( jeVFile_IsValid( pF ) ) ;


	pTerrain = JE_RAM_ALLOCATE_STRUCT( Terrain );
	if( pTerrain == NULL )
	{
		jeErrorLog_Add( JE_ERR_MEMORY_RESOURCE, "Unable to allocate Terrain" );
		return( NULL );
	}
	memset( pTerrain, 0, sizeof( Terrain ) );
	assert( (pTerrain->nSignature = SIGNATURE) == SIGNATURE ) ;	// ASSIGN

	if( !Object_InitFromFile( pF , &pTerrain->ObjectData ) )
	{
		jeErrorLog_AddString( JE_ERR_FILEIO_READ, "Object_InitFromFile.\n", NULL);
		return JE_FALSE;
	}
	if( !jeVFile_Read( pF, &pTerrain->Center, sizeof pTerrain->Center ) )
	{
		jeErrorLog_AddString( JE_ERR_FILEIO_READ, "Terrain_ReadFromFile.\n", NULL);
		return JE_FALSE;
	}
	if( !jeVFile_Read( pF, &pTerrain->Size, sizeof pTerrain->Size ) )
	{
		jeErrorLog_AddString( JE_ERR_FILEIO_READ, "Terrain_ReadFromFile.\n", NULL);
		return JE_FALSE;
	}
	pTerrain->HeightMap = jeBitmap_CreateFromFile( pF );
	if( pTerrain->HeightMap == NULL )
	{
		jeErrorLog_AddString( JE_ERR_FILEIO_READ, "Terrain_ReadFromFile.\n", NULL);
		return JE_FALSE;
	}

	pTerrain->TerrainMap = jeBitmap_CreateFromFile( pF );
	if( pTerrain->TerrainMap == NULL )
	{
		jeErrorLog_AddString( JE_ERR_FILEIO_READ, "Terrain_ReadFromFile.\n", NULL);
		return JE_FALSE;
	}

/*	pTerrain->TerrainData = jeTerrain_CreateFromFile(pF);
	if( pTerrain->TerrainData == NULL )
	{
		jeErrorLog_AddString( JE_ERR_FILEIO_READ, "Terrain_ReadFromFile.\n", NULL);
		return JE_FALSE;
	}
*/
	Terrain_UpdateBounds( pTerrain );
	return( pTerrain );
}


jeBoolean Terrain_WriteToFile( Terrain * pTerrain, jeVFile * pF )
{
	assert( pTerrain != NULL ) ;
	assert( SIGNATURE == pTerrain->nSignature ) ;
	assert( jeVFile_IsValid( pF ) ) ;

	if( !Object_WriteToFile( &pTerrain->ObjectData, pF ) )
	{
		jeErrorLog_AddString(JE_ERR_FILEIO_WRITE, "Object_WriteToFile.\n", NULL);
		return JE_FALSE;
	}
	if( jeVFile_Write( pF, &pTerrain->Center, sizeof pTerrain->Center ) == JE_FALSE )
	{
		jeErrorLog_AddString(JE_ERR_FILEIO_WRITE, "Terrain_WriteToFile.\n", NULL);
		return JE_FALSE;
	}
	if( jeVFile_Write( pF, &pTerrain->Size, sizeof pTerrain->Size ) == JE_FALSE )
	{
		jeErrorLog_AddString(JE_ERR_FILEIO_WRITE, "Terrain_WriteToFile.\n", NULL);
		return JE_FALSE;
	}
	if( jeBitmap_WriteToFile( pTerrain->HeightMap, pF ) == JE_FALSE )
	{
		jeErrorLog_AddString(JE_ERR_FILEIO_WRITE, "Terrain_WriteToFile.\n", NULL);
		return JE_FALSE;
	}
	if( jeBitmap_WriteToFile( pTerrain->TerrainMap, pF ) == JE_FALSE )
	{
		jeErrorLog_AddString(JE_ERR_FILEIO_WRITE, "Terrain_WriteToFile.\n", NULL);
		return JE_FALSE;
	}
/*
	if( jeTerrain_WriteToFile(pTerrain->TerrainData, pF ) == JE_FALSE )
	{
		jeErrorLog_AddString(JE_ERR_FILEIO_WRITE, "Terrain_WriteToFile.\n", NULL);
		return JE_FALSE;
	}
*/	
	return JE_TRUE ;

}// Terrain_WriteToFile

#endif // }
