/****************************************************************************************/
/*  DYNAMICLIGHT.H                                                                      */
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
#include "Engine.h"
#include "Camera.h"
#include "jeFrustum.h"
#include "jeProperty.h"
#include "Object.h"
#include "jeWorld.h"
#include "jePtrMgr.h"

////////////////////////////////////////////////////////////////////////////////////////
//	Prototypes
////////////////////////////////////////////////////////////////////////////////////////
#ifdef WIN32
void		Init_Class( HINSTANCE hInstance );
#endif

#ifdef BUILD_BE
void		Init_Class( image_id hInstance );
#endif

void		DeInit_Class( void );
void *		JETCC CreateInstance( void );
void		JETCC CreateRef( void *Instance );
jeBoolean	JETCC Destroy( void **Instance );

jeBoolean JETCC Render(
	const void				*Instance,	// object instance data
	const jeWorld			*World,		// world
	const jeEngine			*Engine,	// engine
	const jeCamera			*Camera,				// camera
	const jeFrustum			*CameraSpaceFrustum, 	// frustum
	jeObject_RenderFlags	RenderFlags);

jeBoolean	JETCC AttachWorld( void *Instance, jeWorld *pWorld );
jeBoolean	JETCC DettachWorld( void *Instance, jeWorld *pWorld );
jeBoolean	JETCC AttachEngine( void *Instance, jeEngine *Engine );
jeBoolean	JETCC DettachEngine( void *Instance, jeEngine *Engine );
jeBoolean	JETCC AttachSoundSystem( void *Instance, jeSound_System *SoundSystem );
jeBoolean	JETCC DettachSoundSystem( void *Instance, jeSound_System *SoundSystem );
jeBoolean	JETCC Collision( const void *Object, const jeExtBox *Box, const jeVec3d *Front, const jeVec3d *Back, jeVec3d *Impact, jePlane *Plane );
jeBoolean	JETCC GetExtBox( const void *Instance, jeExtBox *BBox );

void *	JETCC CreateFromFile(jeVFile * File, jePtrMgr *);
jeBoolean	JETCC WriteToFile(const void * Instance,jeVFile * File, jePtrMgr *);

jeBoolean	JETCC GetPropertyList( void *Instance, jeProperty_List **List );
jeBoolean	JETCC SetProperty( void *Instance, int32 FieldID, PROPERTY_FIELD_TYPE DataType, jeProperty_Data *pData );
jeBoolean	JETCC SetXForm( void *Instance, const jeXForm3d *XF );
jeBoolean	JETCC GetXForm( const void *Instance, jeXForm3d *XF );
int			JETCC GetXFormModFlags( const void *Instance );
jeBoolean	JETCC GetChildren( const void *Instance, jeObject *Children, int MaxNumChildren );
jeBoolean	JETCC AddChild( void * Instance, const jeObject *Child );
jeBoolean	JETCC RemoveChild( void *Instance, const jeObject *Child );

#ifdef WIN32
jeBoolean	JETCC EditDialog( void *Instance,HWND Parent );
#endif

#ifdef BUILD_BE
jeBoolean	JETCC EditDialog( void *Instance,class G3DView* Parent );
#endif

jeBoolean	JETCC Frame( void *Instance, float TimeDelta );
jeBoolean	JETCC SendAMessage( void *Instance, int32 Msg, void *Data );
//Royce
void * JETCC DuplicateInstance(void * Instance);
//---
// Icestorm
jeBoolean	JETCC ChangeBoxCollision(const void *Instance,const jeVec3d *Pos, const jeExtBox *FrontBox, const jeExtBox *BackBox, jeExtBox *ImpactBox, jePlane *Plane);