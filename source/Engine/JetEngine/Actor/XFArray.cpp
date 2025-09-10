/****************************************************************************************/
/*  XFARRAY.C																			*/
/*                                                                                      */
/*  Author: Mike Sandige	                                                            */
/*  Description: Array of transforms implementation.									*/
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
#include <assert.h>
#include "XFArray.h"
#include "Ram.h"
#include "Errorlog.h"


typedef struct jeXFArray
{
	int		 TransformCount;
	jeXForm3d *TransformArray;
} jeXFArray;

jeXFArray *JETCC jeXFArray_Create(int Size)
{
	jeXFArray *XFA;

	assert( Size > 0 );

	XFA = JE_RAM_ALLOCATE_STRUCT_CLEAR( jeXFArray );
	if (XFA == NULL)
		{
			jeErrorLog_Add( JE_ERR_MEMORY_RESOURCE , "jeXFArray_Create.");
			return NULL;
		}
	XFA->TransformArray = JE_RAM_ALLOCATE_ARRAY_CLEAR(jeXForm3d,Size);
	if (XFA->TransformArray == NULL)
		{
			jeErrorLog_Add( JE_ERR_MEMORY_RESOURCE , "jeXFArray_Create.");
			jeRam_Free( XFA );
			return NULL;
		}
	XFA->TransformCount = Size;
	{
		jeXForm3d X;
		jeXForm3d_SetIdentity(&X);
		jeXFArray_SetAll(XFA,&X);
	}
	return XFA;
}

void JETCC jeXFArray_Destroy( jeXFArray **XFA )
{
	assert( XFA != NULL );
	assert( *XFA != NULL );
	assert( (*XFA)->TransformCount > 0 );
	assert( (*XFA)->TransformArray != NULL );
	
	(*XFA)->TransformCount = -1;
	jeRam_Free( (*XFA)->TransformArray);
	(*XFA)->TransformArray = NULL;
	jeRam_Free( (*XFA) );
	(*XFA) = NULL;
}

jeXForm3d *JETCC jeXFArray_GetElements(const jeXFArray *XFA, int *Size)
{
	assert( XFA != NULL );
	assert( Size != NULL );
	assert( XFA->TransformCount > 0 );
	assert( XFA->TransformArray != NULL );

	*Size = XFA->TransformCount;
	return XFA->TransformArray;
}

void JETCC jeXFArray_SetAll(jeXFArray *XFA, const jeXForm3d *Matrix)
{
	assert( XFA != NULL );
	assert( Matrix != NULL );
	assert( XFA->TransformCount > 0 );
	assert( XFA->TransformArray != NULL );
	{
		int i;
		jeXForm3d *X;
		for (i=0,X=XFA->TransformArray; i<XFA->TransformCount; i++,X++)
			{
				*X = *Matrix;
			}
	}
}
