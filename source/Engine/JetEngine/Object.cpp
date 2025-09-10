/****************************************************************************************/
/*  OBJECT.C                                                                            */
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
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "Object.h"
#include "Errorlog.h"
#include "Ram.h"
#include "crc32.h"
#include "jeFrustum.h"
#include "Util.h"
#include "Log.h"

#ifndef WINVER
#define HWND void *
#endif

#define MAX_DEFS	(100)
#define INVALID_INDEX	(-1)

//#define ObjectError(str,Obz)	jeErrorLog_AddString(-1,"jeObject " (((Obz) != nullptr) ? (((jeObject *)(Obz))->Name) : "Unknown") " Error: " str, nullptr))
__inline void ObjectError(const char* str, const jeObject* Obz)
{
	char msg[1024];
	sprintf_s(msg, "jeObject %s Error: %s", (((Obz) != nullptr) ? (((jeObject *)(Obz))->Name) : "Unknown"), str);
	jeErrorLog_AddString(-1, msg, (((Obz) != nullptr) ? (((jeObject *)(Obz))->Name) : "Unknown"));
}


/*}{********************** Manager Functions ******************/

static const uint32 jeObject_Tag = 0x424F4547; //GEOB

static jeObjectDef	RegisteredDefs[MAX_DEFS];
static uint32		RegisteredTag[MAX_DEFS];
static unsigned int			NumRegisteredDefs = 0;

uint32 __inline jeObject_DefTag(const jeObjectDef * Methods)
{
return CRC32_Array((const uint8 *)Methods->Name,strlen(Methods->Name));
}

//====================================================================================================
//====================================================================================================
JETAPI jeBoolean	JETCC jeObject_RegisterGlobalObjectDef(const jeObjectDef * Methods)
{
	uint32 Tag;
	int i;


	assert(Methods);

	if ( strlen(Methods->Name) == 0 )
	{
		jeErrorLog_AddString(-1,"Object Error: no Name!",Methods->Name);
		return JE_FALSE;
	}

	Tag = jeObject_DefTag(Methods);

	// <> this is not thread-safe

	for(i=0;i<NumRegisteredDefs;i++)
	{
		if ( RegisteredTag[i] == Tag )
		{
			if ( memcmp(&(RegisteredDefs[i]),Methods,sizeof(*Methods)) == 0 )
				return JE_TRUE;
			else
			{
				jeErrorLog_AddString(-1,"Object Error: Tag collision!",Methods->Name);
				return JE_FALSE;
			}
		}
	}

	if ( NumRegisteredDefs == MAX_DEFS )
	{
		jeErrorLog_AddString(-1,"Object Error: too many defs!",Methods->Name);
		return JE_FALSE;
	}

	RegisteredTag[ NumRegisteredDefs] = Tag;
	RegisteredDefs[NumRegisteredDefs] = *Methods;
	NumRegisteredDefs++;
	
	return JE_TRUE;
}
//====================================================================================================
//====================================================================================================
int32 jeObject_FindObjectDef( const char * TypeName  )
{
	int i;

	for( i = 0; i < NumRegisteredDefs; i++ )
	{
		if( strcmp( TypeName, RegisteredDefs[i].Name) == 0 )
			return( i );
	}
	return( INVALID_INDEX );
}
//====================================================================================================
//====================================================================================================
JETAPI int32		JETCC jeObject_GetRegisteredN()
{
	return( NumRegisteredDefs );
}

//====================================================================================================
//====================================================================================================

JETAPI const char*	JETCC jeObject_GetRegisteredDefName( int Index )
{
	assert( Index < NumRegisteredDefs );

	return( RegisteredDefs[Index].Name );
}

JETAPI uint32	JETCC jeObject_GetRegisteredFlags( int Index )
{
	assert( Index < NumRegisteredDefs );

	return( RegisteredDefs[Index].Flags);
}

JETAPI jeBoolean	JETCC jeObject_GetRegisteredPropertyList(const char * TypeName, jeProperty_List **List)
{
	int Index;

	Index =  jeObject_FindObjectDef( TypeName  );
	if( Index == INVALID_INDEX )
		return( JE_FALSE );
	if( RegisteredDefs[Index].GetGlobalPropertyList == nullptr )
	{
		*List = nullptr;
		return( JE_TRUE );
	}
	return( (*RegisteredDefs[Index].GetGlobalPropertyList)(List) );
}

JETAPI jeBoolean	JETCC jeObject_SetRegisteredProperty( const char * TypeName, int32 FieldID, PROPERTY_FIELD_TYPE DataType, jeProperty_Data * pData )
{
	int Index;

	Index =  jeObject_FindObjectDef( TypeName  );
	if( Index == INVALID_INDEX )
		return( JE_FALSE );
	if( RegisteredDefs[Index].SetGlobalProperty == nullptr )
	{
		return( JE_TRUE );
	}
	return( (*RegisteredDefs[Index].SetGlobalProperty)(FieldID, DataType, pData) );
}


//====================================================================================================
//====================================================================================================
JETAPI jeObject *	JETCC jeObject_Create( const char * TypeName )
{
	jeObject* Object{};
	int32 Index = 0;

	Index = jeObject_FindObjectDef( TypeName );

	if( Index == INVALID_INDEX )
		return nullptr;

	Object = (jeObject *)jeRam_AllocateClear(sizeof(jeObject)); // <> MemPool

	if( Object == nullptr )
		return( nullptr );

	Object->Name = nullptr;
	Object->Methods = &RegisteredDefs[Index];
	Object->RefCnt = 1;
	Object->pWorld = nullptr;
	Object->pEngine = nullptr;
	Object->pSoundSystem = nullptr;
	Object->Contents = CONTENTS_SOLID;
	Object->Parent = nullptr;
	Object->Self = Object;
	Object->Children = jeChain_Create();

	if (!Object->Children)
		goto ExitWithError;

	Object->Instance = Object->Methods->CreateInstance();

	if( Object->Instance == nullptr )
		goto ExitWithError;

	return Object;

	ExitWithError:
	{
		if (Object)
		{
			if (Object->Instance)
			{
				assert(Object->Methods);
				assert(Object->Children);

				Object->Methods->Destroy(&Object->Instance);
			}

			if (Object->Children)
				jeChain_Destroy(&Object->Children);

			jeRam_Free(Object);
		}
		return nullptr;
	}
}
//====================================================================================================
//====================================================================================================
JETAPI jeObject *	JETCC jeObject_Duplicate( jeObject *pObject )
{
	jeObject * pObjectCopy{};
	int32 Index{};

	if( pObject->Methods->DuplicateInstance == nullptr )
		return( nullptr );

	Index = jeObject_FindObjectDef( jeObject_GetTypeName(pObject) );

	if( Index == INVALID_INDEX )
		return nullptr;

	pObjectCopy = (jeObject *)jeRam_AllocateClear(sizeof(jeObject)); // <> MemPool
	if( pObjectCopy == nullptr )
		return( nullptr );

	pObjectCopy->Name = nullptr;
	pObjectCopy->Methods = &RegisteredDefs[Index];
	pObjectCopy->RefCnt = 1;
	//Royce
	pObjectCopy->Children = jeChain_Create();
	//---

	pObjectCopy->Instance = pObject->Methods->DuplicateInstance(pObject->Instance);
	if( pObjectCopy->Instance == nullptr )
	{
		jeRam_Free( pObjectCopy );
		return( nullptr );
	}
	return pObjectCopy;
}

//====================================================================================================
//====================================================================================================
JETAPI void		JETCC jeObject_Destroy(jeObject ** pObject)
{
	jeObject * Object;
	assert(pObject);
	Object = *pObject;
	if ( ! Object )
		return;
	if (Object != Object->Self) {
		return;
	}
	assert( Object->RefCnt > 0 );
#ifdef _DEBUG
	Log_Printf("jeObject_Destroy %s\n", Object->Name);
#endif

	Object->RefCnt--;

	if( Object->RefCnt == 0 )
	{
		if (Object->Children)
			jeChain_Destroy(&Object->Children);

		if( Object->Name != nullptr )
			jeRam_Free(Object->Name); // <> MemPool

		// Free the instance
		if( Object->Instance != nullptr ) // Krouer: do not crash if object has failed to load
			jeObject_Free(Object); 

		jeRam_Free(Object); // <> MemPool
	}

	*pObject = nullptr;
}

//====================================================================================================
//====================================================================================================
JETAPI void			JETCC jeObject_SetName( jeObject * pObject, const char * Name )
{
	assert( pObject );
	assert( Name );

	if( pObject->Name != nullptr )
		jeRam_Free( pObject->Name );
	pObject->Name = Util_StrDup( Name );
}

JETAPI const char  *JETCC jeObject_GetName( const jeObject * pObject )
{
	assert( pObject );
	return( pObject->Name );
}

/*}{********************** Object Functions ******************/


JETAPI jeObject *	JETCC jeObject_CreateFromFile(jeVFile * File, jePtrMgr *PtrMgr)
{
	jeObject * Object = nullptr;
	jeVFile * HintsFile = nullptr;
	uint32 Tag;
	long StartPos = -1;
	uint32	NameLng{};

	if (PtrMgr)
	{
		if (!jePtrMgr_ReadPtr(PtrMgr, File, (void **)&Object))
			return nullptr;

		if (Object)
		{
			jeObject_CreateRef(Object);

			return Object;		// Ptr found in stack, return it
		}
	}

	HintsFile = jeVFile_CreateHintsFile(File);
	if ( ! HintsFile )
		return nullptr;

	if ( ! jeVFile_Tell(HintsFile,&StartPos))
		goto fail;

	Object = (jeObject *)jeRam_AllocateClear(sizeof(jeObject)); // <> MemPool
	if ( ! Object )
		goto fail;

	Object->Contents = CONTENTS_SOLID;
	Object->Children = jeChain_Create();
	Object->RefCnt = 1;
	Object->Self = Object;

	if (!Object->Children)
		goto fail;

	if (PtrMgr)
	{
#pragma message( "Should we recover the pushed pointer on failure?")
		// Push the ptr on the stack
		if (!jePtrMgr_PushPtr(PtrMgr, Object))
			goto fail;
	}

	if ( ! jeVFile_Read(HintsFile,&Tag,sizeof(Tag)) )
		goto fail;

	if ( Tag != jeObject_Tag )
	{
		ObjectError("didn't get GEOB tag!",nullptr);
		goto fail;
	}
	
	if ( ! jeVFile_Read(HintsFile,&Tag,sizeof(Tag)) )
		goto fail;

	{
	int i;
		for(i=0;i<NumRegisteredDefs;i++)
		{
			if ( RegisteredTag[i] == Tag )
			{
				Object->Methods = &RegisteredDefs[i];
				break;
			}
		}
	}
	
	if ( ! jeVFile_Read(File, &NameLng,sizeof(NameLng)) )
		goto fail;

	if( NameLng )
	{
		Object->Name = (char *)jeRam_Allocate( NameLng );
		if ( ! jeVFile_Read(File, Object->Name, NameLng) )
			goto fail;
	}

	if ( ! Object->Methods )
	{
		ObjectError("Couldn't find registered def to create!",Object);
		goto fail;
	}
	else if ( ! Object->Methods->CreateFromFile )
	{
		ObjectError("Found registered with no create!",Object);
		goto fail;
	}

	jeVFile_Close(HintsFile);
	HintsFile = nullptr;

	Object->Instance = Object->Methods->CreateFromFile(File, PtrMgr);
	
	if ( ! 	Object->Instance )
		goto fail;

	return Object;

fail:

	ObjectError("CreateFromFile failed",Object);
	jeObject_Destroy(&Object);
	if ( HintsFile )
	{
		if ( StartPos != -1 )
			jeVFile_Seek(HintsFile,StartPos,JE_VFILE_SEEKSET);
		jeVFile_Close(HintsFile);
	}

	return nullptr;
}


//====================================================================================================
//====================================================================================================

JETAPI jeBoolean	JETCC jeObject_WriteToFile(const jeObject * Object,jeVFile * File, jePtrMgr *PtrMgr)

{
	jeVFile * HintsFile = nullptr;
	uint32 Tag;
	long StartPos = -1;
	uint32	NameLng{};

	assert(Object && Object->Instance && Object->Methods);

	if (PtrMgr)
	{
		uint32		Count;

		// writes the pointer header
		if (!jePtrMgr_WritePtr(PtrMgr, File, (void*)Object, &Count))
			return JE_FALSE;

		if (Count)
			return JE_TRUE;		// Ptr was on stack, so return

		assert ( Object->Methods->WriteToFile );

		// For object reentrance this need to be done here - consulted John
		// if an error occurs after this then a pop need to be done
		if (PtrMgr)
		{
			// Push the ptr on the stack
			if (!jePtrMgr_PushPtr(PtrMgr, (void*)Object))
				return JE_FALSE;
		}

	}

	if ( ! Object->Methods->WriteToFile )
		return JE_FALSE;

	HintsFile = jeVFile_CreateHintsFile(File);
	if ( ! HintsFile )
		goto fail;

	if ( ! jeVFile_Tell(HintsFile,&StartPos))
		goto fail;

	if ( ! jeVFile_Write(HintsFile,&jeObject_Tag,sizeof(jeObject_Tag)) )
		goto fail;

	Tag = jeObject_DefTag(Object->Methods);

	if ( ! jeVFile_Write(HintsFile,&Tag,sizeof(Tag)) )
		goto fail;

	if( Object->Name  )
	{
		NameLng = strlen( Object->Name ) + 1;
	}
	else
		NameLng = 0;
	if ( ! jeVFile_Write(File, &NameLng,sizeof(NameLng)) )
		goto fail;

	if( NameLng )
		if ( ! jeVFile_Write(File, Object->Name, NameLng) )
			goto fail;

	jeVFile_Close(HintsFile);
	HintsFile = nullptr;

	if ( ! Object->Methods->WriteToFile(Object->Instance,File, PtrMgr) )
		goto fail;

	return JE_TRUE;

fail:

	if (PtrMgr)
		{
		jePtrMgr_PopPtr(PtrMgr, (void*)Object);
		}

	ObjectError("WriteToFile failed",Object);
	if (HintsFile)
	{
		if ( StartPos != -1 )
			jeVFile_Seek(HintsFile,StartPos,JE_VFILE_SEEKSET);
		jeVFile_Close(HintsFile);
	}
	return JE_FALSE;
}



//====================================================================================================
//====================================================================================================
JETAPI jeObject_Type	JETCC jeObject_GetType(const jeObject * Object)
{
	assert(Object && Object->Instance && Object->Methods);
	return Object->Methods->Type;
}

//====================================================================================================
//====================================================================================================
JETAPI const char *JETCC jeObject_GetTypeName	(const jeObject * Object)
{
	assert(Object && Object->Instance && Object->Methods);
	return Object->Methods->Name;
}

//====================================================================================================
//====================================================================================================
JETAPI jeBoolean	JETCC jeObject_GetPropertyList(const jeObject *Object, jeProperty_List **List)
{
	assert(Object);
	assert(Object->Instance);
	assert(Object->Methods);

	if (!Object->Methods->GetPropertyList)
		return JE_FALSE;

	return Object->Methods->GetPropertyList(Object->Instance, List);
}

//====================================================================================================
//====================================================================================================

JETAPI jeBoolean	JETCC jeObject_SetProperty(jeObject *Object, int32 FieldID, PROPERTY_FIELD_TYPE DataType, jeProperty_Data * pData )
{
	assert(Object);
	assert(Object->Instance);
	assert(Object->Methods);

	if (!Object->Methods->SetProperty)
		return JE_FALSE;

	return Object->Methods->SetProperty(Object->Instance, FieldID, DataType, pData );
}

//====================================================================================================
//====================================================================================================

JETAPI jeBoolean	JETCC jeObject_GetProperty(const jeObject *Object, int32 FieldID, PROPERTY_FIELD_TYPE DataType, jeProperty_Data * pData )
{
	assert(Object);
	assert(Object->Instance);
	assert(Object->Methods);

	if (!Object->Methods->GetProperty)
		return JE_FALSE;

	return Object->Methods->GetProperty(Object->Instance, FieldID, DataType, pData );
}

//====================================================================================================
//====================================================================================================
JETAPI void   *		JETCC jeObject_GetInstance( const jeObject *Object )
{
	assert(Object);

	return( Object->Instance );
}

//====================================================================================================
//====================================================================================================
JETAPI void			JETCC jeObject_CreateInstanceRef(jeObject * Object)
{
	assert(Object);
	assert(Object->Instance);
	assert(Object->Methods);
	assert(Object->Methods->CreateRef);
	Object->Methods->CreateRef(Object->Instance);
}

//====================================================================================================
//====================================================================================================
JETAPI jeBoolean JETCC jeObject_Free		(jeObject * Object)
{
	assert(Object && Object->Instance && Object->Methods);
	assert(Object->Methods->Destroy);
	return( Object->Methods->Destroy(&(Object->Instance)));
}

//====================================================================================================
//====================================================================================================
JETAPI void		JETCC jeObject_CreateRef	(jeObject * Object)
{
	assert(Object );
	 Object->RefCnt++;
}

//====================================================================================================
//====================================================================================================
JETAPI jeBoolean JETCC jeObject_Render(	const jeObject			*Object,
												const jeWorld			*World, 
												const jeEngine			*Engine, 
												const jeCamera			*Camera, 
												const jeFrustum			*CameraSpaceFrustum, 
												jeObject_RenderFlags	RenderFlags)
{
	assert(Object && Object->Instance && Object->Methods);

	if (!Object->Methods->Render )
		return JE_FALSE;

	return Object->Methods->Render(Object->Instance, World, Engine, Camera, CameraSpaceFrustum, RenderFlags);
}

//====================================================================================================
//====================================================================================================
JETAPI jeBoolean JETCC jeObject_AttachWorld( jeObject *Object, jeWorld * pWorld )
{
	//jeChain_Link	*Link;
	jeBoolean		Ret{};
	jeObject	*Object2{};

	assert(Object && Object->Instance && Object->Methods);

	Ret = JE_TRUE;
	
	//Royce
	if (Object->Children) {
	//---
		for (Object2 = (jeObject *)jeChain_GetNextLinkData(Object->Children, nullptr); Object2; Object2 = (jeObject *)jeChain_GetNextLinkData(Object->Children, Object2))
		{
			assert(Object2);

			if (!Object2->Methods->AttachWorld )
				continue;

			Ret &= Object2->Methods->AttachWorld( Object2->Instance, pWorld);
		}
	//Royce
	}
	//---

	if (!Object->Methods->AttachWorld )
		return Ret;

	Ret &= Object->Methods->AttachWorld(Object->Instance, pWorld );
	Object->pWorld = pWorld;

	return Ret;
}

//====================================================================================================
//====================================================================================================
JETAPI jeBoolean JETCC jeObject_DettachWorld( jeObject *Object, jeWorld * pWorld )
{
	//jeChain_Link	*Link;
	jeBoolean		Ret{};
	jeObject	*Object2{};

	assert(Object && Object->Instance && Object->Methods);
	if (Object != Object->Self) {
		return JE_FALSE;
	}

	Ret = JE_TRUE;

	for (Object2 = (jeObject *)jeChain_GetNextLinkData(Object->Children, nullptr); Object2; Object2 = (jeObject *)jeChain_GetNextLinkData(Object->Children, Object2))
	{
		assert(Object2);

		if (Object2 != Object2->Self) {
			continue;
		}

		if (!Object2->Methods->DettachWorld )
			continue;

		Ret &= Object2->Methods->DettachWorld( Object2->Instance, pWorld);
	}

	if (!Object->Methods->DettachWorld)
		return Ret;

	Ret &= Object->Methods->DettachWorld(Object->Instance, pWorld );
	Object->pWorld = nullptr;

	return Ret;
}

//====================================================================================================
//====================================================================================================
JETAPI jeBoolean JETCC jeObject_AttachEngine( jeObject *Object, jeEngine *Engine )
{
	//jeChain_Link	*Link;
	jeBoolean		Ret{};
	jeObject	*Object2{};

	assert(Object && Object->Instance && Object->Methods);

	Ret = JE_TRUE;

	//Royce
	if (Object->Children) {
	//---
		for (Object2 = (jeObject*)jeChain_GetNextLinkData(Object->Children, nullptr); Object2; Object2 = (jeObject *)jeChain_GetNextLinkData(Object->Children, Object2))
		{
		
			assert(Object2);

			if (!Object2->Methods->AttachEngine )
				continue;

			Ret &= Object2->Methods->AttachEngine( Object2->Instance, Engine);
		}
	//Royce
	}
	//----

	if (!Object->Methods->AttachEngine )
		return Ret;
	
	Ret &= Object->Methods->AttachEngine(Object->Instance, Engine );
	Object->pEngine = Engine;

	return Ret;
}

//====================================================================================================
//====================================================================================================
JETAPI jeBoolean JETCC jeObject_DettachEngine( jeObject *Object, jeEngine *Engine )
{
	//jeChain_Link	*Link;
	jeBoolean		Ret{};
	jeObject	*Object2{};

	assert(Object && Object->Instance && Object->Methods);
	if (Object != Object->Self) {
		return JE_TRUE;
	}

	Ret = JE_TRUE;

	for (Object2 = (jeObject *)jeChain_GetNextLinkData(Object->Children, nullptr); Object2; Object2 = (jeObject *)jeChain_GetNextLinkData(Object->Children, Object2))
	{
		assert(Object2);

		if (Object2 != Object2->Self) {
			continue;
		}

		if (!Object2->Methods->DettachEngine )
			continue;

		Ret &= Object2->Methods->DettachEngine( Object2->Instance, Engine);
	}

	if (!Object->Methods->DettachEngine )
		return Ret;

	Ret &= Object->Methods->DettachEngine(Object->Instance, Engine );
	Object->pEngine = nullptr;

	return Ret;
}

//====================================================================================================
//====================================================================================================
JETAPI jeBoolean JETCC jeObject_AttachSoundSystem( jeObject *Object, jeSound_System *SoundSystem )
{
	//jeChain_Link	*Link;
	jeBoolean		Ret{};
	jeObject* Object2{};

	assert(Object && Object->Instance && Object->Methods);
	Object->pSoundSystem = SoundSystem;

	Ret = JE_TRUE;

	//Royce
	if (Object->Children) {
	//---
		for (Object2 = (jeObject *)jeChain_GetNextLinkData(Object->Children, nullptr); Object2; Object2 = (jeObject *)jeChain_GetNextLinkData(Object->Children, Object2))
		{
		
			assert(Object2);

			if (!Object2->Methods->AttachSoundSystem )
				continue;

			Ret &= Object2->Methods->AttachSoundSystem( Object2->Instance, SoundSystem);
		}
	//Royce
	}
	//---

	if (!Object->Methods->AttachSoundSystem )
		return Ret;


	Ret &= Object->Methods->AttachSoundSystem(Object->Instance, SoundSystem );

	return Ret;
}

//====================================================================================================
//====================================================================================================
JETAPI jeBoolean JETCC jeObject_DettachSoundSystem( jeObject *Object, jeSound_System *SoundSystem )
{
	jeChain_Link	*Link{};
	jeBoolean		Ret{};

	assert(Object && Object->Instance && Object->Methods);
	if (Object != Object->Self) {
		return JE_FALSE;
	}

	Object->pSoundSystem = nullptr;

	Ret = JE_TRUE;

	for (Link = jeChain_GetFirstLink(Object->Children); Link; Link = jeChain_LinkGetNext(Link))
	{
		jeObject	*Object2 = (jeObject*)Link;
		
		assert(Object2);
		if (Object2 != Object2->Self) {
			continue;
		}
	
		if (!Object2->Methods || !Object2->Methods->DettachSoundSystem )
			continue;

		Ret &= Object2->Methods->DettachSoundSystem(Object2->Instance, SoundSystem);
	}

	if (!Object->Methods->DettachSoundSystem )
		return Ret;

	Ret &= Object->Methods->DettachSoundSystem(Object->Instance, SoundSystem);


	return Ret;
}


//====================================================================================================
//	jeObject_Collision
//====================================================================================================
JETAPI jeBoolean JETCC jeObject_Collision(const jeObject *Object, const jeExtBox *Box, const jeVec3d *Front, const jeVec3d *Back, jeVec3d *Impact, jePlane *Plane, jeObject ** pSubObject)
{
jeChain_Link * Link{};
jeFloat Distance{},ClosestDistance{};
jeVec3d ClosestImpact{};
jePlane ClosestPlane{};
const jeObject * ClosestObject{};
jeBoolean GotHit{};

	assert(Object);
	assert(Object->Instance);
	assert(Object->Methods);

	GotHit = JE_FALSE;
	if(jeObject_GetContents(Object) != CONTENTS_SOLID) return GotHit; // Incarnadine

	if ( pSubObject )
	{
		assert( Object->Children );
		for( Link = jeChain_GetFirstLink(Object->Children); Link; Link = jeChain_LinkGetNext(Link) )
		{
		jeObject *Child,*ChildSubO;
			Child = (jeObject *)jeChain_LinkGetLinkData(Link);
			assert(Child);

			if (Impact && Plane)
			{
				if ( jeObject_Collision(Child, Box, Front, Back, Impact, Plane, &ChildSubO) )
				{
					Distance = jeVec3d_DistanceBetweenSquared(Front,Impact);
					if ( ! GotHit || Distance < ClosestDistance )
					{
						GotHit = JE_TRUE;
						ClosestDistance = Distance;
						ClosestImpact = *Impact;
						ClosestPlane = *Plane;
						ClosestObject = ChildSubO;
					}
				}
			} else
				if ( jeObject_Collision(Child, Box, Front, Back, nullptr, nullptr, &ChildSubO) )
					return JE_TRUE;
		}
	}

	if ( Object->Methods->Collision)
	{
		if (Impact && Plane)
		{
			if ( Object->Methods->Collision(Object->Instance, Box, Front, Back, Impact, Plane) )
			{
				Distance = jeVec3d_DistanceBetweenSquared(Front,Impact);
				if ( ! GotHit || Distance < ClosestDistance )
				{
					GotHit = JE_TRUE;
					ClosestDistance = Distance;
					ClosestImpact = *Impact;
					ClosestPlane = *Plane;
					ClosestObject = Object;
				}
			}
		} else
			if ( Object->Methods->Collision(Object->Instance, Box, Front, Back, nullptr, nullptr) )
				return JE_TRUE;
	}

	if ( GotHit )
	{
		*Impact = ClosestImpact;
		*Plane = ClosestPlane;
		if ( pSubObject ) *pSubObject = (jeObject *)ClosestObject;
		return JE_TRUE;
	}

return JE_FALSE;
}

// Added by Icestorm
//====================================================================================================
//	jeObject_ChangeBoxCollision
//====================================================================================================
JETAPI jeBoolean JETCC jeObject_ChangeBoxCollision(const jeObject *Object, const jeVec3d *Pos, const jeExtBox *FrontBox, const jeExtBox *BackBox, jeExtBox *ImpactBox, jePlane *Plane, jeObject ** SubObject)
{
	jeChain_Link* Link{};
	jeFloat Distance{}, ClosestDistance{};
	jeExtBox ClosestImpactBox{};
	jePlane ClosestPlane{};
	const jeObject* ClosestObject{};
	jeBoolean GotHit{};

	assert(Object);
	assert(Object->Instance);
	assert(Object->Methods);

	GotHit = JE_FALSE;

	if ( SubObject )
	{
		assert( Object->Children );
		for( Link = jeChain_GetFirstLink(Object->Children); Link; Link = jeChain_LinkGetNext(Link) )
		{
			jeObject *Child,*ChildSubO{};
			Child = (jeObject *)jeChain_LinkGetLinkData(Link);
			assert(Child);

			if (ImpactBox && Plane)
			{
				if ( jeObject_ChangeBoxCollision(Child, Pos, FrontBox, BackBox, ImpactBox, Plane, &ChildSubO) )
				{
					Distance = jeVec3d_DistanceBetweenSquared(&FrontBox->Min, &ImpactBox->Min);
					if ( ! GotHit || Distance < ClosestDistance )
					{
						GotHit = JE_TRUE;
						ClosestDistance = Distance;
						ClosestImpactBox = *ImpactBox;
						ClosestPlane = *Plane;
						ClosestObject = ChildSubO;
					}
				}
			} else
				if ( jeObject_ChangeBoxCollision(Child, Pos, FrontBox, BackBox, nullptr, nullptr, &ChildSubO) )
					return JE_TRUE;
		}
	}

	if ( Object->Methods->ChangeBoxCollision)
	{
		if (ImpactBox && Plane)
		{
			if ( Object->Methods->ChangeBoxCollision(Object->Instance, Pos, FrontBox, BackBox, ImpactBox, Plane) )
			{
				Distance = jeVec3d_DistanceBetweenSquared(&FrontBox->Min, &ImpactBox->Min);
				if ( ! GotHit || Distance < ClosestDistance )
				{
					GotHit = JE_TRUE;
					ClosestDistance = Distance;
					ClosestImpactBox = *ImpactBox;
					ClosestPlane = *Plane;
					ClosestObject = Object;
				}
			}
		} else
			if ( Object->Methods->ChangeBoxCollision(Object->Instance, Pos, FrontBox, BackBox, nullptr, nullptr) )
				return JE_TRUE;
	}

	if ( GotHit )
	{
		*ImpactBox = ClosestImpactBox;
		*Plane = ClosestPlane;
		if ( SubObject ) *SubObject = (jeObject *)ClosestObject;
		return JE_TRUE;
	}

return JE_FALSE;
}

//====================================================================================================
//====================================================================================================
JETAPI jeBoolean	JETCC jeObject_GetExtBox	(const jeObject * Object,jeExtBox *BBox)
{
	assert(Object && Object->Instance && Object->Methods);

	if (!Object->Methods->GetExtBox )
		return JE_FALSE;

	return Object->Methods->GetExtBox(Object->Instance,BBox);
}


//====================================================================================================
//====================================================================================================
JETAPI jeBoolean	JETCC jeObject_SetXForm	(jeObject * Object,const jeXForm3d *XF)
{
	assert(Object && Object->Instance && Object->Methods);

	if (!Object->Methods->SetXForm)
		return JE_FALSE;

	return Object->Methods->SetXForm(Object->Instance, XF);
}

//====================================================================================================
//====================================================================================================
JETAPI jeBoolean JETCC jeObject_GetXForm	(const jeObject * Object,jeXForm3d *XF)
{
	assert(Object && Object->Instance && Object->Methods);

	if (!Object->Methods->GetXForm)
		return JE_FALSE;

	return Object->Methods->GetXForm(Object->Instance,XF);
}

//====================================================================================================
//====================================================================================================
JETAPI int JETCC jeObject_GetXFormModFlags( const jeObject * Object )
{
	assert(Object && Object->Instance && Object->Methods);

	if (!Object->Methods->GetXFormModFlags)
		return JE_FALSE;

	return Object->Methods->GetXFormModFlags(Object->Instance);
}

//====================================================================================================
//====================================================================================================
JETAPI jeBoolean	JETCC jeObject_GetChildren(const jeObject * Object,jeObject * Children,int MaxNumChildren)
{
	assert(Object && Object->Instance && Object->Methods);

	if (!Object->Methods->GetChildren )
		return JE_FALSE;

	return Object->Methods->GetChildren(Object->Instance,Children,MaxNumChildren);
}

//====================================================================================================
//	jeObject_GetNextChild
//====================================================================================================
JETAPI jeObject *JETCC jeObject_GetNextChild(const jeObject *Object, jeObject *Start)
{
	assert(Object && Object->Instance && Object->Methods);

	return (jeObject *)jeChain_GetNextLinkData(Object->Children, Start);
}

//====================================================================================================
//====================================================================================================
JETAPI jeObject *JETCC jeObject_GetParent( const jeObject *Object )
{
	assert(Object && Object->Instance && Object->Methods);

	return( Object->Parent );
}

//====================================================================================================
//====================================================================================================
JETAPI jeBoolean	JETCC jeObject_AddChild	(jeObject * Object, jeObject * Child)
{
/*	if (Object != nullptr)
	{
		if (Object->Instance != nullptr)
		{
			if (Object->Methods != nullptr)
			{
				if (!jeChain_AddLinkData(Object->Children, (void*)Child))
					return JE_FALSE;

				if (!Object->Methods->AddChild)
					return JE_TRUE;

				//if (Object->Methods->Type != JE_OBJECT_TYPE_MODEL || Child->Methods->Type != JE_OBJECT_TYPE_ACTOR)
				//{
				if (Object != nullptr)
				{ 
				if (Object->pWorld != nullptr && Child != nullptr)
					jeObject_AttachWorld(Child, Object->pWorld);
				if (Object->pEngine != nullptr && Child != nullptr)
					jeObject_AttachEngine(Child, Object->pEngine);
				if (Object->pSoundSystem != nullptr && Child != nullptr)
					jeObject_AttachSoundSystem(Child, Object->pSoundSystem);
				//}
				Child->Parent = Object;
				//jeObject_CreateRef(Child);
				return Object->Methods->AddChild(Object->Instance, Child);
				}
			}
			return JE_FALSE;
		}
		return JE_FALSE;
	}
	return JE_FALSE;
	*/

//by trilobite

	assert(Object && Object->Instance && Object->Methods);

	assert(!jeChain_FindLink(Object->Children, (void*)Child));

	if (!jeChain_AddLinkData(Object->Children, (void*)Child))
		return JE_FALSE;
	
	if (!Object->Methods->AddChild)
		return JE_TRUE;

	//if (Object->Methods->Type != JE_OBJECT_TYPE_MODEL || Child->Methods->Type != JE_OBJECT_TYPE_ACTOR)
	//{
		if( Object->pWorld )
			jeObject_AttachWorld( Child, Object->pWorld );
		if( Object->pEngine )
			jeObject_AttachEngine( Child, Object->pEngine );
		if( Object->pSoundSystem )
			jeObject_AttachSoundSystem( Child, Object->pSoundSystem );
	//}
	Child->Parent = Object;
	//jeObject_CreateRef(Child);
	return Object->Methods->AddChild(Object->Instance,Child);

	
}

//====================================================================================================
//====================================================================================================
JETAPI jeBoolean	JETCC jeObject_RemoveChild(jeObject* Object, jeObject* Child)
{

/*
	if (Object != nullptr)
	{
		if (!jeChain_FindLink(Object->Children, (void*)Child))
			return JE_FALSE;

		if (!jeChain_RemoveLinkData(Object->Children, (void*)Child))
			return JE_FALSE;
	
		if (Object != nullptr)
		{
			if (Object->Methods != nullptr)
				if (!Object->Methods->RemoveChild)
					return JE_TRUE;
		}
	}

	if (Object != nullptr && Child != nullptr)
	{
		if (Object->pWorld != nullptr)
			jeObject_DettachWorld(Child, Object->pWorld);
		if (Object->pEngine != nullptr)
			jeObject_DettachEngine(Child, Object->pEngine);
		if (Object->pSoundSystem != nullptr)
			jeObject_DettachSoundSystem(Child, Object->pSoundSystem);
		Child->Parent = nullptr;

		return Object->Methods->RemoveChild(Object->Instance, Child);
	}

	return JE_TRUE;

	*/

	
	assert(Object && Object->Instance && Object->Methods);

	if (!jeChain_FindLink(Object->Children, (void*)Child))
		return JE_FALSE;

	if (!jeChain_RemoveLinkData(Object->Children, (void*)Child))
		return JE_FALSE;

	if (!Object->Methods->RemoveChild )
		return JE_TRUE;
	if( Object->pWorld )
		jeObject_DettachWorld( Child, Object->pWorld );
	if( Object->pEngine )
		jeObject_DettachEngine( Child, Object->pEngine );
	if( Object->pSoundSystem )
		jeObject_DettachSoundSystem( Child, Object->pSoundSystem );
	Child->Parent = nullptr;

	return Object->Methods->RemoveChild(Object->Instance,Child);

	
}

//====================================================================================================
//====================================================================================================
JETAPI jeBoolean	JETCC jeObject_EditDialog (jeObject * Object,HWND Parent)
{
	assert(Object && Object->Instance && Object->Methods);

	if (!Object->Methods->EditDialog )
		return JE_FALSE;
		
	return Object->Methods->EditDialog(Object->Instance,Parent);
}

//====================================================================================================
//====================================================================================================
JETAPI jeBoolean	JETCC jeObject_SendMessage (jeObject * Object,int32 Msg, void * Data)
{
	assert(Object && Object->Instance && Object->Methods);
	
	if (!Object->Methods->SendMessage )
		return JE_FALSE;

	return Object->Methods->SendMessage(Object->Instance, Msg, Data );
}

//====================================================================================================
//====================================================================================================
JETAPI jeBoolean	JETCC jeObject_Frame (jeObject * Object,float TimeDelta )
{
	jeBoolean		Ret = JE_TRUE;
	jeObject	*Object2{};
	assert(Object && Object->Instance && Object->Methods);
	

	for (Object2 = (jeObject *)jeChain_GetNextLinkData(Object->Children, nullptr); Object2; Object2 = (jeObject *)jeChain_GetNextLinkData(Object->Children, Object2))
	{
		assert(Object2);

		if (!Object2->Methods->Frame )
			continue;

		Ret &= Object2->Methods->Frame( Object2->Instance, TimeDelta);
	}

	if (!Object->Methods->Frame )
		return Ret;
	Ret &= Object->Methods->Frame(Object->Instance, TimeDelta );
	return Ret;
}

/*}{********************** Crap ******************/

static jeObject TestO = { nullptr, nullptr };

//====================================================================================================
//====================================================================================================
static void TestFunc(jeObject * O)
{
	jeObject_Free(O);
}

JETAPI int32 JETCC jeObject_GetContents(const jeObject * Object)
{
	assert(Object != nullptr);
	return Object->Contents;
}

JETAPI void JETCC jeObject_SetContents(jeObject * Object, int32 Contents)
{
	assert(Object != nullptr);
	Object->Contents = Contents;
}

JETAPI void JETCC jeObject_SetRenderNextPass(const jeObject* Object, jeBoolean RenderNext)
{
	assert(Object != nullptr);

	if (Object->Methods->SetRenderNextPass)
		Object->Methods->SetRenderNextPass(Object->Instance, RenderNext);
}

JETAPI uint32 JETCC jeObject_GetFlags(const jeObject * Object)
{
	assert(Object != nullptr);

	return Object->Methods->Flags;
}
