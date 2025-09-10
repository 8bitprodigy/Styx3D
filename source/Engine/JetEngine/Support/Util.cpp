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
	pTextString = (char*)jeRam_Allocate( ResSize + 1 );
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

char * Util_StrDup( const char * const psz )
{
	char * p = (char *)jeRam_Allocate( strlen( psz ) + 1 ) ;
	if( p ) 
	{
		strcpy( p, psz ) ;
	}
	return  p ;

}// Util_StrDup

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
