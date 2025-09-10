/*!
	@file Object.h 
	
	@author
	@brief Object definition

	@par Licence
	The contents of this file are subject to the Jet3D Public License       
	Version 1.02 (the "License"); you may not use this file except in         
	compliance with the License. You may obtain a copy of the License at       
	http://www.jet3d.com                                                        
                                                                             
	@par
	Software distributed under the License is distributed on an "AS IS"           
	basis, WITHOUT WARRANTY OF ANY KIND, either express or implied.  See           
	the License for the specific language governing rights and limitations          
	under the License.                                                               
                                                                                  
	@par
	The Original Code is Jet3D, released December 12, 1999.                            
	Copyright (C) 1996-1999 Eclipse Entertainment, L.L.C. All Rights Reserved           
*/
#ifndef JE_OBJECT_H
#define JE_OBJECT_H

#include "jeTypes.h"
#include "VFile.h"
#include "ExtBox.h"
#include "Bitmap.h"
#include "Camera.h"
#include "jeFrustum.h"
#include "jeProperty.h"
#include "jeChain.h"
#include "jePtrMgr.h"


#ifdef __cplusplus
extern "C" {
#endif

#ifndef WINVER
#define HWND void *
#endif

#include "Sound.h"

/*}{********* jeObject ********/

#include "jePtrMgr.h"
typedef struct jeEngine		jeEngine;
typedef struct jeWorld		jeWorld;

typedef struct jeObject		jeObject;
typedef struct jeObjectDef	jeObjectDef;

#define FILE_UNIQUE_ID 3132205802


#define CONTENTS_EMPTY 0
#define CONTENTS_SOLID 1
#define CONTENTS_INVISIBLE 2

/*! struct jeObject
	@brief The jeObject data structure
*/
struct jeObject
{
	char *				Name;		//!< Name of the object in clear
	const jeObjectDef	*Methods;	//!< Array of functions any object of this kind can use
	void				*Instance;	//!< Value of one object of this jeObject kind
	jeObject			*Parent;	//!< Parent object of the object Instance
	jeChain				*Children;	//!< List of all childrens of this object Instance
	jeWorld	*			pWorld;		//!< jeWorld attached to this object Instance
	jeEngine *			pEngine;	//!< jeEngine attached to this object Instance
	jeSound_System *	pSoundSystem;	//!< jeSound_System attached to this object Instance
	/*! Collision contents 
		@author Incarnadine
	*/
	int32				Contents;					
	int32				RefCnt;		//!< The jeObject reference counter
	jeObject*			Self;
};

/*! @name jeObject XForm mod flags */
/*@{ */
/*! @def JE_OBJECT_XFORM_TRANSLATE
	@brief The translation of an XForm can be modifyed
*/
#define JE_OBJECT_XFORM_TRANSLATE		(1<<0)
/*! @def JE_OBJECT_XFORM_SCALE
	@brief The XForm can be scaled
*/
#define JE_OBJECT_XFORM_SCALE			(1<<1)
/*! @def JE_OBJECT_XFORM_ROTATE
	@brief The XForm can be rotated
*/
#define JE_OBJECT_XFORM_ROTATE			(1<<2)
/*! @def JE_OBJECT_XFORM_SHEAR
	@brief The XForm can be sheared
*/
#define JE_OBJECT_XFORM_SHEAR			(1<<3)
/*! @def JE_OBJECT_XFORM_ALL
	@brief All flags altogether
*/
#define JE_OBJECT_XFORM_ALL				( JE_OBJECT_XFORM_TRANSLATE | JE_OBJECT_XFORM_SCALE |JE_OBJECT_XFORM_ROTATE|JE_OBJECT_XFORM_SHEAR)
/*@} */

/*! @name jeObject flag possible values */
/*@{ */
#define JE_OBJECT_HIDDEN				0x0001		//!< This object is can not be created by user
#define JE_OBJECT_VISRENDER				0x0002		//!< This object must be rendered only if visible
/*@} */

/*! @enum jeObject_Type
	@brief jeObject kind possible values */
typedef enum
{
	JE_OBJECT_TYPE_OBJECTLIST,
	JE_OBJECT_TYPE_ACTOR,
	JE_OBJECT_TYPE_MODEL,
	JE_OBJECT_TYPE_TERRAIN,
	JE_OBJECT_TYPE_CURVE,
	JE_OBJECT_TYPE_PORTAL,
	JE_OBJECT_TYPE_UNKNOWN
} jeObject_Type;

	// the _UNKNOWN type cannot be cast & must be accessed through its ->Methods virtual methods
	// note : we (or the user) can add types after _UNKNOWN
	// you should always check >= _UNKNOWN to see if you can't cast it; (never use == _UNKNOWN)

// Render Flags
typedef uint32											jeObject_RenderFlags;

#define JE_OBJECT_RENDER_FLAG_CAMERA_FRUSTUM			(1<<0)

// Built in system messages 
//	(Should these be in jeWorld.h? World is really the object mgr, and should define these types of msg's...)
#define JE_OBJECT_MSG_WORLD_ADD_SLIGHT_UPDATE			(-1)		// SLIGHT = Static Light
#define JE_OBJECT_MSG_WORLD_REMOVE_SLIGHT_UPDATE		(-2)
#define JE_OBJECT_MSG_WORLD_REBUILDBSP		(-3)  // Incarnadine
#define JE_OBJECT_MSG_WORLD_REBUILDLIGHTS (-4) // Incarnadine

/*}{********************** Object-Manager Functions ******************/

JETAPI jeBoolean	JETCC jeObject_RegisterGlobalObjectDef(const jeObjectDef * Methods);
							// you must Register your Def if you want _CreateFromFile
							//	to be able to make objects of your type.
							// this means Registering user objects *before* calling jeWorld_Create()

JETAPI int32		JETCC jeObject_GetRegisteredN();
							// Returns the number of Registered Defs

JETAPI const char*	JETCC jeObject_GetRegisteredDefName( int Index );
JETAPI uint32		JETCC jeObject_GetRegisteredFlags( int Index );
JETAPI jeBoolean	JETCC jeObject_GetRegisteredPropertyList(const char * TypeName, jeProperty_List **List);
JETAPI jeBoolean	JETCC jeObject_SetRegisteredProperty( const char * TypeName, int32 FieldID, PROPERTY_FIELD_TYPE DataType, jeProperty_Data * pData );

							// Index must be less than the number of Registered Defs 
/*}{********************** Object Functions ******************/

JETAPI jeObject *	JETCC jeObject_Create		( const char * TypeName );					// mallocs a blank object
JETAPI jeObject *	JETCC jeObject_Duplicate( jeObject *pObject );

JETAPI void			JETCC jeObject_Destroy	(jeObject ** pObject);	// frees & destroys an object
JETAPI void			JETCC jeObject_SetName( jeObject * pObject, const char * Name );
JETAPI const char  *JETCC jeObject_GetName( const jeObject * pObject );

JETAPI jeBoolean	JETCC jeObject_WriteToFile(const jeObject * Object,jeVFile * File, jePtrMgr *PtrMgr);
JETAPI jeObject *	JETCC jeObject_CreateFromFile(jeVFile * File, jePtrMgr *PtrMgr);

JETAPI jeObject_Type	JETCC jeObject_GetType(const jeObject * Object);
JETAPI const char	*JETCC jeObject_GetTypeName	(const jeObject * Object);

JETAPI jeBoolean	JETCC jeObject_GetPropertyList(const jeObject *Object, jeProperty_List **List);
JETAPI jeBoolean	JETCC jeObject_SetProperty(jeObject *Object, int32 FieldID, PROPERTY_FIELD_TYPE DataType, jeProperty_Data * pData );
JETAPI jeBoolean	JETCC jeObject_GetProperty(const jeObject *Object, int32 FieldID, PROPERTY_FIELD_TYPE DataType, jeProperty_Data * pData );

JETAPI void   *		JETCC jeObject_GetInstance( const jeObject *Object );
JETAPI void			JETCC jeObject_CreateInstanceRef(jeObject * Object);
JETAPI jeBoolean	JETCC jeObject_Free(jeObject * Object);	
						// destroys the *Instance*, not the object's memory
						// returns JE_FALSE if RefCnt is not 0
JETAPI void			JETCC jeObject_CreateRef(jeObject * Object);

JETAPI jeBoolean	JETCC jeObject_Render(	const jeObject			*Object,
													const jeWorld			*World, 
													const jeEngine			*Engine,
													const jeCamera			*Camera, 
													const jeFrustum			*CameraSpaceFrustum, 
													jeObject_RenderFlags	RenderFlags);

JETAPI jeBoolean	JETCC jeObject_AttachWorld( jeObject *Object, jeWorld * pWorld );
JETAPI jeBoolean	JETCC jeObject_DettachWorld( jeObject *Object, jeWorld * pWorld );
JETAPI jeBoolean	JETCC jeObject_AttachEngine( jeObject *Object, jeEngine *Engine );
JETAPI jeBoolean	JETCC jeObject_DettachEngine(jeObject *Object, jeEngine *Engine );
JETAPI jeBoolean	JETCC jeObject_AttachSoundSystem( jeObject *Object, jeSound_System *pSoundSystem );
JETAPI jeBoolean	JETCC jeObject_DettachSoundSystem(jeObject *Object, jeSound_System *pSoundSystem );

JETAPI jeBoolean	JETCC jeObject_Collision(const jeObject *Object, const jeExtBox *Box, const jeVec3d *Front, const jeVec3d *Back, jeVec3d *Impact, jePlane *Plane, jeObject ** SubObject);

JETAPI jeBoolean	JETCC jeObject_GetExtBox	(const jeObject * Object,jeExtBox *BBox);

JETAPI jeBoolean	JETCC jeObject_SetXForm	(jeObject * Object,const jeXForm3d *XF);
JETAPI jeBoolean	JETCC jeObject_GetXForm	(const jeObject * Object,jeXForm3d *XF);
JETAPI int			JETCC jeObject_GetXFormModFlags( const jeObject * Object );

JETAPI jeBoolean	JETCC jeObject_GetChildren(const jeObject * Object,jeObject * Children,int MaxNumChildren);
JETAPI jeObject		*JETCC jeObject_GetNextChild(const jeObject *Object, jeObject *Start);
JETAPI jeObject		*JETCC jeObject_GetParent( const jeObject *Object );
JETAPI jeBoolean	JETCC jeObject_AddChild	(jeObject * Object,   jeObject * Child);
JETAPI jeBoolean	JETCC jeObject_RemoveChild(jeObject * Object, jeObject * Child);

#ifdef WIN32
JETAPI jeBoolean	JETCC jeObject_EditDialog (jeObject * Object,HWND Parent);
#endif

#ifdef BUILD_BE
JETAPI jeBoolean	JETCC jeObject_EditDialog (jeObject * Object,class G3DView * Parent);
#endif

JETAPI jeBoolean	JETCC jeObject_SendMessage (jeObject * Object,int32 Msg, void * Data);

JETAPI jeBoolean	JETCC jeObject_Frame (jeObject * Object,float TimeDelta );

JETAPI int32 JETCC jeObject_GetContents(const jeObject * Object);
JETAPI void JETCC jeObject_SetContents(jeObject * Object, int32 Contents);

typedef struct jeCollisionInfo jeCollisionInfo;
JETAPI jeBoolean JETCC jeObject_MoveCollision(jeObject * Object, 											
											const jeVec3d *TargetPos,
											jeCollisionInfo *CollisionInfo);

// Added by Icestorm
JETAPI jeBoolean	JETCC jeObject_ChangeBoxCollision(const jeObject	*Object,
													  const jeVec3d		*Pos,
													  const jeExtBox	*FrontBox,
													  const jeExtBox	*BackBox,
													  jeExtBox			*ImpactBox,
													  jePlane			*Plane,
													  jeObject			** SubObject);

// KROUER: Actor/Object visibility
/*! @fn void jeObject_SetRenderNextPass(const jeObject	*Object, jeBoolean RenderNext)
	@brief Toggle the render of the object in the next rendering pass
    
	This function is used mainly for visibility definition.<br>To render the objects in 
	the next objects rendering loop, you must set the RenderNextTime flag to JE_TRUE.
	To make the object rendering loop ignoring it, you must set the RenderingNextTime 
	flag to JE_FALSE.
	JetEngine rendering is done in 2 passes:<br>
	1. Define which part of the BSP to render and render them<br>
	2. Render all objects<br>
	<br>
	To be more accurate, default BSP is embed in the head object of the object list. By this way,
	default BSP object is always rendered first.
	@author Bruno Pettorelli (krouer@jet3d.com)

	@param[in] Object The object on which apply the RenderNext value
	@param[in] RenderNext A boolean value to set or reset the Render nex pass value
*/
JETAPI void			JETCC jeObject_SetRenderNextPass(const jeObject	*Object, jeBoolean RenderNext);

/*! @fn uint32 jeObject_GetFlags(const jeObject * Object);
	@brief Read the Flag member of the Object

    @param[in] Object The object from which to read the flag
	@return The reading flag
*/
JETAPI uint32		JETCC jeObject_GetFlags(const jeObject * Object);

/***

todos :

	1. make a _TYPE_LIST which is just a container for Add & Remove calls;
		Render & such call the function on all children

***/

/*}{******** The Object Def *********/

struct jeObjectDef
{
	jeObject_Type	Type;
	const char *	Name;		// Defs MUST have unique names!
	uint32			Flags;
	
	void	*	( JETCC * CreateInstance)(void);
	void		( JETCC * CreateRef)	(void * Instance);
	jeBoolean	( JETCC * Destroy)		(void **pInstance);
				//Returns JE_TRUE if refcnt is 0 and instance is destroyed

	jeBoolean	( JETCC * AttachWorld ) ( void * Instance, jeWorld * pWorld );
	jeBoolean	( JETCC * DettachWorld ) ( void * Instance, jeWorld * pWorld );
				
	jeBoolean	( JETCC * AttachEngine ) ( void * Instance, jeEngine *Engine );
	jeBoolean	( JETCC * DettachEngine ) ( void * Instance, jeEngine *Engine );

	jeBoolean	( JETCC * AttachSoundSystem ) ( void * Instance, jeSound_System *SoundSystem );
	jeBoolean	( JETCC * DettachSoundSystem ) ( void * Instance, jeSound_System *SoundSystem );

	jeBoolean	( JETCC * Render) (const void * Instance, const jeWorld *World, const jeEngine *Engine, const jeCamera *Camera, const jeFrustum *CameraSpaceFrustum, jeObject_RenderFlags RenderFlags);

	jeBoolean	( JETCC * Collision)	(const void * Instance, const jeExtBox *Box, const jeVec3d *Front, const jeVec3d *Back, jeVec3d *Impact, jePlane *Plane);

	jeBoolean	( JETCC * GetExtBox)	(const void * Instance,jeExtBox *BBox);

	//--- the stuff above this point is pretty secure

	void *		( JETCC * CreateFromFile)(jeVFile * File, jePtrMgr *PtrMgr);
	jeBoolean	( JETCC * WriteToFile)	(const void * Instance,jeVFile * File, jePtrMgr *PtrMgr);

	jeBoolean	( JETCC * GetPropertyList)	(void * Instance, jeProperty_List **List);
	jeBoolean	( JETCC * SetProperty)      (void * Instance, int32 FieldID, PROPERTY_FIELD_TYPE DataType, jeProperty_Data * pData );
	jeBoolean	( JETCC * GetProperty )( void * Instance, int32 FieldID, PROPERTY_FIELD_TYPE DataType, jeProperty_Data * pData );

	jeBoolean	( JETCC * SetXForm)	(void * Instance,const jeXForm3d *XF);
	jeBoolean	( JETCC * GetXForm)	(const void * Instance,jeXForm3d *XF);
								// if your object is added to a world, you *MUST* call World_UpdateObject if
								//	you call Object_SetXForm
	int			( JETCC * GetXFormModFlags )( const void * Instace );
								//The flags determine in what way is it valid to modify the transform

	jeBoolean	( JETCC * GetChildren)	(const void * Instance,jeObject * Children,int MaxNumChildren);
	jeBoolean	( JETCC * AddChild)	(void * Instance,const jeObject * Child);
	jeBoolean	( JETCC * RemoveChild)	(void * Instance,const jeObject * Child);

	jeBoolean	( JETCC * EditDialog )	(void * Instance, HWND Parent);
	jeBoolean	( JETCC * SendMessage )	(void * Instance, int32 Msg, void * Data);
	jeBoolean	( JETCC * Frame ) (void * Instance, float Time );
	void *		( JETCC * DuplicateInstance )( void * Instance );

	// Added by Icestorm
	jeBoolean	( JETCC * ChangeBoxCollision )(const void * Instance, const jeVec3d *Pos, const jeExtBox *FrontBox, const jeExtBox *BackBox, jeExtBox *ImpactBox, jePlane *Plane);

	jeBoolean	( JETCC * GetGlobalPropertyList)	(jeProperty_List **List);
	jeBoolean	( JETCC * SetGlobalProperty)      (int32 FieldID, PROPERTY_FIELD_TYPE DataType, jeProperty_Data * pData );

	// Added by KROUER
	/*! @brief Enable the draw for the next render pass
		@see jeObject_SetRenderNextPass
	*/
	void        ( JETCC * SetRenderNextPass) (void * Instance, jeBoolean RenderNext);
};

#ifndef WINVER
#undef HWND
#endif


#ifdef __cplusplus
}
#endif

#endif

