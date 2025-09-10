/****************************************************************************************/
/*  GEASSERT.H                                                                          */
/*                                                                                      */
/*  Author:                                                                             */
/*  Description: Replacement for assert interface                                       */
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
#ifndef JE_ASSERT_H
#define JE_ASSERT_H

#include <assert.h>
#include "BaseType.h"

#ifdef __cplusplus
extern "C" {
#endif

// You should use jeAssert() anywhere in the Jet3D engine that
// you would normally use assert().
//
// If you wish to be called back when asserts happen, use the
// routine jeAssertSetCallback().  It returns the address of
// the callback routine that you're replacing.


#ifdef NDEBUG

	#define jeAssert(exp)

#else

	extern void jeAssertEntryPoint( void *, void *, unsigned );

	#define jeAssert(exp) (void)( (exp) || (jeAssertEntryPoint(#exp, __FILE__, __LINE__), 0) )

#endif

void jeAssertDefault( void *exp, void *file, unsigned long line );

/************************************************************/

typedef void jeAssertCallbackFn( void *exp, void *file, unsigned long line );

jeAssertCallbackFn *jeAssertSetCallback( jeAssertCallbackFn *newAssertCallback );

typedef void (*jeAssert_CriticalShutdownCallback) (uint32 Context);

extern void jeAssert_SetCriticalShutdownCallback( jeAssert_CriticalShutdownCallback CB , uint32 Context,
												jeAssert_CriticalShutdownCallback * pOldCB , uint32 * pOldContext);

/************************************************************/

#ifdef __cplusplus
}
#endif

#endif
