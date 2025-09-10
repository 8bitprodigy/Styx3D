/****************************************************************************************/
/*  XFARRAY.H																			*/
/*                                                                                      */
/*  Author: Mike Sandige	                                                            */
/*  Description: Array of transforms interface.											*/
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
#ifndef JE_XFARRAY_H
#define JE_XFARRAY_H

/* This is a simple object to formalize an array of transforms (jeXForm3d)

   Unfortunately, it's not a very safe object.

   This object exports data (allows external access to one of it's data members)
   This is dangerous - no checking can be done on the use of that data, and no
   checking can be done on array boundry conditions.  This is on purpose.
   

 */

#include "Xform3d.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct jeXFArray jeXFArray;

	// Create the object.  Creates an array of Size elements.  
	// All elements are initialized to the identity transform
jeXFArray *JETCC jeXFArray_Create(int Size);

	// Destroy the object.  Don't use the pointer returned by _GetElements
	// after destroying the ojbect!
void JETCC jeXFArray_Destroy( jeXFArray **XFA );

	// Get a pointer to the array.  For external iteration.  The size of the 
	// array is returned in Size.  Valid array indicies are (0..Size-1)
jeXForm3d *JETCC jeXFArray_GetElements(const jeXFArray *XFA, int *Size);

	// Sets every transform in the array to the given transform.
void JETCC jeXFArray_SetAll(jeXFArray *XFA, const jeXForm3d *Matrix);

#ifdef __cplusplus
}
#endif


#endif
