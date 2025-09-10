/****************************************************************************************/
/*  JEPORTAL.C                                                                          */
/*                                                                                      */
/*  Author:  John Pollard                                                               */
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
#include <assert.h>
#include <string.h>

#include "jePortal.h"

#include "Ram.h"

//========================================================================================
//========================================================================================
#define ZeroMem(a) memset(a, 0, sizeof(*a))
#define ZeroMemArray(a, s) memset(a, 0, sizeof(*a)*s)

//========================================================================================
//	jePortal_Create
//========================================================================================
JETAPI jePortal * JETCC jePortal_Create(void)
{
	jePortal *Portal;

	Portal = JE_RAM_ALLOCATE_STRUCT(jePortal);

	if (!Portal)
		return NULL;

	ZeroMem(Portal);

	Portal->RefCount = 1;

	jeXForm3d_SetIdentity(&Portal->XForm);

	return Portal;
}

//========================================================================================
//	jePortal_CreateRef
//========================================================================================
JETAPI jeBoolean JETCC jePortal_CreateRef(jePortal *Portal)
{
	assert (jePortal_IsValid(Portal));

	Portal->RefCount++;

	return JE_TRUE;
}

//========================================================================================
//	jePortal_Destroy
//========================================================================================
JETAPI void JETCC jePortal_Destroy(jePortal **Portal)
{
	assert(Portal);
	assert(jePortal_IsValid(*Portal));

	(*Portal)->RefCount--;

	if ((*Portal)->RefCount > 0)
		return;

	jeRam_Free(*Portal);
	*Portal = NULL;
}

//========================================================================================
//	jePortal_IsValid
//========================================================================================
JETAPI jeBoolean JETCC jePortal_IsValid(const jePortal *Portal)
{
	if (!Portal)
		return JE_FALSE;

	if (Portal->RefCount <= 0)
		return JE_FALSE;

	return JE_TRUE;
}
