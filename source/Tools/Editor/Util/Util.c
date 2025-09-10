/****************************************************************************************/
/*  UTIL.C                                                                              */
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
#pragma warning(disable : 4201 4214 4115)
#include <Windows.h>
#include <Windowsx.h>
#pragma warning(default : 4201 4214 4115; disable : 4514)

#include "BaseType.h"
#include "Ram.h"

#include "Util.h"

#define MAX(aa,bb)   ( ((aa)>(bb))?(aa):(bb) )
#define MIN(aa,bb)   ( ((aa)<(bb))?(aa):(bb) )

static HINSTANCE hResources = 0;
static char stringbuffer[UTIL_MAX_RESOURCE_LENGTH + 1];

void Util_Init( unsigned long hStringResources )
{
	hResources = (HINSTANCE)hStringResources ;
}// Util_Init

char * Util_GetRcString( char * psz, const unsigned int idResource )
{
	assert( psz != NULL ) ;
	
	if( LoadString( hResources, idResource, psz, UTIL_MAX_RESOURCE_LENGTH ) == 0)
		psz[0] = '\0' ;

	return psz ;
}// Util_GetRcString

//-----------------------------------------------------------------------------
//	  FUNCTION:   Util_LoadLibraryString() - Get a resource string from an
//												external library.
//
//	PARAMETERS:   HINSTANCE libhinst  - library handle (same as HMODULE)
//				  UINT resid		  - id of string resource
//
//	   RETURNS:   char * - allocated string with resource
//
//		 NOTES:   Free the string with ram_free() or Resource_FreeString()
//
//				  See notes with Resource_LoadLocalString()
//
//-----------------------------------------------------------------------------
static char *Util_LoadLibraryString(HINSTANCE libhinst, UINT resid)
{
	int result;
	char *rcbuffer;
 
	assert(libhinst);
	assert(resid);
 
	// Load the string
	result = LoadString(libhinst, resid, stringbuffer, UTIL_MAX_RESOURCE_LENGTH);
 
	assert(result);
	if (! result)
		return (NULL);
 
	//
	//	Note that if we did't allocate space and copy the string, then we
	//	would be limited to having one string loaded at a time. Or we would
	//	setup some kind of revolving buffer.  Either of these options is
	//	risky and could eventually cause a problem elsewhere... 	 LF
	//
 
	// Allocate memory for the string
	rcbuffer = (char*)jeRam_Allocate(strlen(stringbuffer) + 1);
	strcpy(rcbuffer, stringbuffer);
 
#ifndef NDEBUG
	memset(stringbuffer, 0xFF, UTIL_MAX_RESOURCE_LENGTH + 1);
#endif
 
	// return the allocated string
	return (rcbuffer);
}//Util_LoadLibraryString

char * Util_LoadText( unsigned int resid )
{
	HGLOBAL hTextRes;
	HRSRC  hTextInfo;
	char * pTextRes;
	char * pTextString;
	int		ResSize;

	hTextInfo = FindResource( hResources, MAKEINTRESOURCE( resid ), "TEXT" );
	if( hTextInfo == NULL )
		return( NULL );

	hTextRes = LoadResource( hResources, hTextInfo );
	if( hTextRes  == NULL )
		return( NULL );
	pTextRes = (char*)LockResource( hTextRes );
	if( pTextRes  == NULL )
		return( NULL );
	ResSize = SizeofResource( hResources, hTextInfo );
	pTextString = jeRam_Allocate( ResSize + 1 );
	if( pTextString == NULL )
		return( NULL );
	memcpy( pTextString, pTextRes, ResSize );
	pTextString[ResSize] = '\0';
	FreeResource( hTextRes );

	return( pTextString );
}

//------------------------------------------------------------------------
//	  FUNCTION:   Util_LoadLocalRcString() - Get a resource string from the
//											  system executable.
//
//	PARAMETERS:   UINT resid - id of string resource
//
//	   RETURNS:   char * - allocated string with resource
//
//		 NOTES:   Free the string with ram_free() or Resource_FreeString()
//
//				  Uses a large, very temporary, buffer to read in the string.
//				  This method was used because the combination of using
//				  FindResource() and SizeofResource() did not always work.
//				  Additionally, SizeofResource() returned a size based upon
//				  some kind of alignment, resulting in much larger sizes
//				  than the actual string required.
//
//				  Anyway, after the string is loaded, memory is allocated for
//				  the actual string size and the string is copied into this
//				  allocated memory.  Not very efficient, and it requires a
//				  predermined maximum size (MAX_STRING_RESOURCE).  I would
//				  have rather determined this dynamicly, but I could find no
//				  reliable methods to do so.
//
//				  The result is, we can read any string resource up to
//				  MAX_STRING_RESOURCE and place it in its own memory, retain
//				  the string as long as we like, and then free the memory
//				  when we are done. No limitations, other than memory, on the
//				  number of strings loaded or what we do with them. The caller
//				  is not required to know the size of the string or allocate
//				  any memory for the string.  LF
//
//-----------------------------------------------------------------------------
char *Util_LoadLocalRcString(unsigned int resid)
{
	return (Util_LoadLibraryString(hResources, resid));
}// Util_LoadLocalRcString

jeBoolean Util_IsKeyDown( int vKey )
{
	short KeyState;

	KeyState = GetAsyncKeyState( vKey ) ;
	return ( KeyState & 0x8000 ) ? JE_TRUE : JE_FALSE ;
}// Util_IsKeyDown

char * Util_StrDup( const char * const psz )
{
	char * p = (char *)jeRam_Allocate( strlen( psz ) + 1 ) ;
	if( p ) 
	{
		strcpy( p, psz ) ;
	}
	return  p ;

}// Util_StrDup

jeBoolean Util_Polyline( int32 hDC, Point * pPoints, int32 nPoints )
{
	return Polyline( (HDC)hDC, (POINT*)pPoints, nPoints ) ;
}// Util_Polyline

jeFloat Util_PointToLineDistanceSquared( const Point * pL1, const Point * pL2, const Point * pPoint )
{
	int32	xkj, ykj ;
	jeFloat	t ;
	jeFloat	xfac, yfac ;
	int32	denom ;
	int32	dx, dy ;

	assert(pL1 != NULL);
	assert(pL2 != NULL);
	assert(pPoint != NULL);
	
	// Code from "A Programmers Geometry, P. 47

	xkj = pL1->X - pPoint->X ;
	ykj = pL1->Y - pPoint->Y ;

	dx = pL2->X - pL1->X ;
	dy = pL2->Y - pL1->Y ;

	denom = (dx * dx) + (dy * dy) ;

	if( IsFloatZero( (jeFloat)denom ) )
	{
		return (jeFloat)(xkj*xkj) + (ykj*ykj) ;
	}

	t = (- ((jeFloat)((xkj * dx) + (ykj * dy))) / (jeFloat)denom) ;
	t = min( max(t, 0.0f), 1.0f ) ;

	xfac = xkj + ( t * dx ) ;
	yfac = ykj + ( t * dy ) ;

	return (xfac*xfac) + (yfac*yfac) ;
}/* Util_PointToLineDistanceSquared */

jeFloat	Util_PointDistanceSquared( const Point * pL1, const Point * pL2 )
{
	int32 dX, dY;

	dX = pL1->X - pL2->X;
	dY = pL1->Y - pL2->Y;
	return( (jeFloat)(dX*dX+dY*dY) );
}

// This routine exists because insane "validity" checking in Jet3D ruins ExtBox
void Util_ExtBox_Union( const jeExtBox *B1, const jeExtBox *B2, jeExtBox *Result )
{
	assert( B1 != NULL ) ;
	assert( B2 != NULL ) ;
	assert( Result != NULL );

	jeExtBox_Set (	Result,
				MIN (B1->Min.X, B2->Min.X),
				MIN (B1->Min.Y, B2->Min.Y),
				MIN (B1->Min.Z, B2->Min.Z),
				MAX (B1->Max.X, B2->Max.X),
				MAX (B1->Max.Y, B2->Max.Y),
				MAX (B1->Max.Z, B2->Max.Z) );
}// Util_ExtBox_Union

// This routine exists because insane "validity" checking in Jet3D ruins ExtBox 
// Extend a box to encompass the passed point
void Util_geExtBox_ExtendToEnclose( jeExtBox *B, const jeVec3d *Point )
{
	assert( B != NULL ) ;
	assert( Point != NULL );
	assert( jeVec3d_IsValid(Point) != JE_FALSE );

	if (Point->X > B->Max.X ) B->Max.X = Point->X;
	if (Point->Y > B->Max.Y ) B->Max.Y = Point->Y;
	if (Point->Z > B->Max.Z ) B->Max.Z = Point->Z;

	if (Point->X < B->Min.X ) B->Min.X = Point->X;
	if (Point->Y < B->Min.Y ) B->Min.Y = Point->Y;
	if (Point->Z < B->Min.Z ) B->Min.Z = Point->Z;

}

jeBoolean Util_geExtBox_Intersection ( const jeExtBox *B1, const jeExtBox *B2, jeExtBox *Result	)
{
	if( !jeExtBox_IsValid( B1 ) )
		return( JE_FALSE );
	if( !jeExtBox_IsValid( B2 ) )
		return( JE_FALSE );
	return( jeExtBox_Intersection( B1, B2, Result ) );
}


void Util_geExtBox_InitFromTwoPoints( jeExtBox * B, const jeVec3d * p1, const jeVec3d * p2 )
{
	assert( B != NULL ) ;
	assert( p1 != NULL ) ;
	assert( p2 != NULL ) ;

	if( p1->X < p2->X )
	{
		B->Min.X = p1->X ;
		B->Max.X = p2->X ;
	}
	else
	{
		B->Min.X = p2->X ;
		B->Max.X = p1->X ;
	}

	if( p1->Y < p2->Y )
	{
		B->Min.Y = p1->Y ;
		B->Max.Y = p2->Y ;
	}
	else
	{
		B->Min.Y = p2->Y ;
		B->Max.Y = p1->Y ;
	}

	if( p1->Z < p2->Z )
	{
		B->Min.Z = p1->Z ;
		B->Max.Z = p2->Z ;
	}
	else
	{
		B->Min.Z = p2->Z ;
		B->Max.Z = p1->Z ;
	}
}// Util_geExtBox_InitFromTwoPoints

jeFloat Util_geExtBox_GetExtent( const jeExtBox *B, int32 nElement )
{
	assert( B != NULL ) ;
	assert( nElement < 3 ) ;
	return jeVec3d_GetElement( &B->Max, nElement) - jeVec3d_GetElement( &B->Min, nElement) ;
}// Util_geExtBox_GetExtent

void Util_ExtBox_SetInvalid( jeExtBox *B )
{
	B->Min.X = B->Min.Y = B->Min.Z = FLT_MAX ;
	B->Max.X = B->Max.Y = B->Max.Z = -FLT_MAX ;
}// Util_ExtBox_SetInvalid

void Util_ExtBox_Transform( const jeExtBox *B, const jeXForm3d *XForm, jeExtBox *Result )
{
	static	jeVec3d 	UnitBox[8] = { { -0.5f, -0.5f, -0.5f },
									   { -0.5f, -0.5f,  0.5f },
									   { -0.5f,  0.5f, -0.5f },
									   { -0.5f,  0.5f,  0.5f },
									   {  0.5f, -0.5f, -0.5f },
									   {  0.5f, -0.5f,  0.5f },
									   {  0.5f,  0.5f, -0.5f },
									   {  0.5f,  0.5f,  0.5f } };
	jeVec3d	Box[8];
	jeVec3d	Diff;
	jeVec3d	ExtBoxCenter;
	int		i;

	assert( jeExtBox_IsValid(B) != JE_FALSE );
	assert( jeXForm3d_IsValid(XForm) != JE_FALSE );
	assert( Result != NULL );

	jeVec3d_Subtract(&B->Max, &B->Min, &Diff);
	jeVec3d_Set(&ExtBoxCenter, B->Min.X + Diff.X / 2.0f, 
							   B->Min.Y + Diff.Y / 2.0f, 
							   B->Min.Z + Diff.Z / 2.0f);
	for	(i = 0; i < 8; i++)
	{
		Box[i].X = UnitBox[i].X * Diff.X + ExtBoxCenter.X;
		Box[i].Y = UnitBox[i].Y * Diff.Y + ExtBoxCenter.Y;
		Box[i].Z = UnitBox[i].Z * Diff.Z + ExtBoxCenter.Z;
	}

	jeXForm3d_TransformVecArray(XForm, Box, Box, 8);
	jeExtBox_SetToPoint(Result, &Box[0]);
	for	(i = 1; i < 8; i++)
		jeExtBox_ExtendToEnclose(Result, &Box[i]);
}

void Util_ExtBox_TransformJ( const jeExtBox *B, const jeXForm3d *XForm, jeExtBox *Result )
{
	jeVec3d				Verts[8] ;
	int					i ;

	assert( jeExtBox_IsValid(B) != JE_FALSE );
	assert( jeXForm3d_IsValid(XForm) != JE_FALSE );
	assert( Result != NULL );
	
	// Setup the 8 corners of the box into an array
	Verts[0] = B->Min ;
	Verts[7] = B->Max ;
	Verts[1].X = Verts[2].X = Verts[3].X = Verts[0].X ;
	Verts[1].Y = Verts[0].Y ;
	Verts[3].Z = Verts[4].Z = Verts[6].Z = Verts[0].Z ;
	Verts[4].Y = Verts[5].Y = Verts[0].Y ;

	Verts[1].Z = Verts[2].Z = Verts[5].Z = Verts[7].Z ;
	Verts[2].Y = Verts[3].Y = Verts[6].Y = Verts[7].Y ;
	Verts[4].X = Verts[5].X = Verts[6].X = Verts[7].X ;

	jeXForm3d_TransformVecArray(XForm, Verts, Verts, 8);
	jeExtBox_SetToPoint(Result, &Verts[0]);
	for	(i = 1; i < 8; i++)
		jeExtBox_ExtendToEnclose(Result, &Verts[i]);
}

int32 Util_Time()
{
	return( timeGetTime() );
}

jeFloat Util_log2( jeFloat f )
{
	return (jeFloat)(log (f)/log (2.0f));
}// log2

jeFloat Util_NearestLowerPowerOf2( const jeFloat fVal )
{
	// This would likely be faster if you count the bits
	// and then loop multiplying by 2 for each bit
	return (jeFloat)pow (2, (int)(Util_log2(fVal) ));
}// Util_NearestLowerPowerOf2


void Util_DriveAndPathOnly( char * pszPath ) 
{
	char	szDrive[_MAX_DRIVE] ;
	char	szDir[_MAX_DIR] ;

	_splitpath( pszPath, szDrive, szDir, NULL, NULL ) ;
	strcpy( pszPath, szDrive ) ;
	strcat( pszPath, szDir ) ;
}// Util_DriveAndPathOnly

void Util_NameOnly( char * pszPath )
{
	char	szName[_MAX_FNAME] ;
	char	szExt[_MAX_EXT] ;

	_splitpath( pszPath, NULL, NULL, szName, szExt ) ;
	strcpy( pszPath, szName ) ;
	strcat( pszPath, szExt ) ;
}// Util_NameOnly

void Util_StripTrailingBackslash(char * pszString )
{
	int len;

	assert(pszString != NULL);

	len = strlen(pszString);
	if( (len > 0) && (pszString[len - 1] == '\\') )
		pszString[len - 1] = 0;
}// Util_StripTrailingBackslash

void Util_NewExtension(char * pszString, const char * pszNewExt)
{
	char	szPath[_MAX_PATH] ;
	char	szName[_MAX_FNAME] ;

	_splitpath( pszString, NULL, szPath, szName, NULL ) ;
	strcpy( pszString, szPath ) ;
	strcat( pszString, szName ) ;
	strcat( pszString, pszNewExt ) ;
}// Util_NewExtension

jeBoolean Util_geVFile_ReadString( jeVFile * pFile, char * pszBuffer, const int32 nMaxChars )
{
	int32		i ;
	assert( jeVFile_IsValid( pFile ) ) ;
	assert( pszBuffer != NULL ) ;
	assert( nMaxChars >= 1 ) ;

	for( i=0; i<nMaxChars; i++ )
	{
		if( !jeVFile_Read( pFile, pszBuffer, 1 ) )
			return JE_FALSE ;

		if( *pszBuffer == 0 )
			return JE_TRUE ;
		pszBuffer++ ;
	}
	return JE_FALSE ;

}// Util_geVFile_ReadString

float Util_GetTime()
{
	float time;

	time = timeGetTime() * 0.001f;

	return( time );
}


////////////////////////////////////////////////////////////////////////////////////////
//
//	Util_GetAppPath()
//
//	Get path of calling exe
//
////////////////////////////////////////////////////////////////////////////////////////
int Util_GetAppPath(
	char	*Buf,		// where to store path name
	int		BufSize )	// size of buf
{

	// locals
	int	Count;

	// get exe full path name
	Count = GetModuleFileName( NULL, Buf, BufSize );
	if ( Count == 0 )
	{
		return 0;
	}

	// eliminate the exe from the path name
	while ( Count >= 0 )
	{
		if ( Buf[Count] == '\\' )
		{
			break;
		}
		Buf[Count] = '\0';
		Count--;
	}

	// all done
	return Count;

} // Util_GetAppPath()

/* EOF: Util.c */
