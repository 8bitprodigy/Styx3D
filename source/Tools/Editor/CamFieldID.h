/****************************************************************************************/
/*  CAMFIELDID.H                                                                        */
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
/*  The Original Code is Jet3D, released Dec 4, 1999.                                   */
/*  Copyright (C) 1996-1999 Eclipse Entertainment, L.L.C. All Rights Reserved           */
/*                                                                                      */
/****************************************************************************************/
enum {
	CAMREA_FOV_ID = PROPERTY_LOCAL_DATATYPE_START,
	PORTAL_SKYBOX_CHECK_ID,
	PORTAL_SPEED_ID,
	PORTAL_RADIOX_ID,
	PORTAL_RADIOY_ID,
	PORTAL_RADIOZ_ID,

	// BEGIN - Far Clip plane box - paradoxnj 3/9/2005
	CAMREA_FARCLIP_ID,
	CAMREA_FARCLIPENABLE_ID,
	// END - Far clip plane box - paradoxnj 3/9/2005

	CAMREA_LAST_ID
};
