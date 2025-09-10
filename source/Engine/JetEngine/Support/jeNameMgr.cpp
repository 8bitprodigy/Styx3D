/****************************************************************************************/
/*  JENAMEMGR.C                                                                         */
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
#include <memory.h>
#include <assert.h>
#include <stdio.h>

#include "jeChain.h"
#include "Ram.h"
#include "VFile.h"

#ifdef NEWSAVE

typedef struct jeNameMgr
{
	int32					RefCount;
	jeVFile					*System;
	jeVFile                 *Dir;
    int32                   Flags;

    jeChain                 *List;
} jeNameMgr;

#define JE_NAME_MGR_NAME_SIZE 8 // without null terminator

typedef struct ReadWriteData
{
    char                                PointerText[JE_NAME_MGR_NAME_SIZE]; // no null terminator
    void                                *DataPtr;
    jeNameMgr_WriteToFileCallback       WriteToFile;
	 jeBoolean							Flushed;
} ReadWriteData;

///////////////////////////////////////
// Local fucntions
///////////////////////////////////////

jeBoolean SaveCallbackData(jeNameMgr *NM,
                           char *PointerText,
                           void *DataPtr,
                           jeNameMgr_WriteToFileCallback CB_Write)
    {
    ReadWriteData *RWData;

	assert(NM);
	assert(PointerText);
	assert(DataPtr);

    RWData = (ReadWriteData *)jeRam_Allocate(sizeof(ReadWriteData));

	if (!RWData)
		return JE_FALSE;

    memcpy(RWData->PointerText, PointerText, JE_NAME_MGR_NAME_SIZE);
    RWData->WriteToFile = CB_Write;
    RWData->DataPtr = DataPtr;
	RWData->Flushed = JE_FALSE;

    if (!jeChain_AddLinkData(NM->List, RWData))
		{
		jeRam_Free(RWData);
		return JE_FALSE;
		}

	return JE_TRUE;
    }

ReadWriteData *FindCallbackData(jeNameMgr *NM, char *PointerText)
{
    jeChain_Link *Link;

	assert(NM);
	assert(PointerText);

    // get rid of data from the list
	for (Link = jeChain_GetFirstLink(NM->List); Link; Link = jeChain_LinkGetNext(Link))
	{
		// locals
		ReadWriteData	*RWData;

		// get data pointer
		RWData = (ReadWriteData *)jeChain_LinkGetLinkData( Link );

        if (memcmp(RWData->PointerText, PointerText, JE_NAME_MGR_NAME_SIZE) == 0)
            {
            return RWData;
            }
	}

    return NULL;
}

ReadWriteData *FindCallbackDataByPtr(jeNameMgr *NM, void *DataPtr)
{
    jeChain_Link *Link;

	assert(NM);
	assert(DataPtr);

    // get rid of data from the list
	for (Link = jeChain_GetFirstLink(NM->List); Link; Link = jeChain_LinkGetNext(Link))
	{
		// locals
		ReadWriteData	*RWData;

		// get data pointer
		RWData = (ReadWriteData *)jeChain_LinkGetLinkData( Link );

        if (RWData->DataPtr == DataPtr)
            {
            return RWData;
            }
	}

    return NULL;
}

jeBoolean PtrToText(void *Ptr, char *TextBuff)
    {
	int32 count;

	assert(Ptr);
	assert(TextBuff);

    count = sprintf(TextBuff,"%08x",Ptr);

    if (count > JE_NAME_MGR_NAME_SIZE || count < JE_NAME_MGR_NAME_SIZE)
		return JE_FALSE;

    return JE_TRUE;
    }

///////////////////////////////////////
// Public functions
///////////////////////////////////////

JETAPI jeNameMgr * JETCC jeNameMgr_Create(jeVFile *System, int32 CreateFlags)
{
	jeNameMgr		*NM;

	assert (System);
    assert (CreateFlags == JE_NAME_MGR_CREATE_FOR_WRITE || CreateFlags == JE_NAME_MGR_CREATE_FOR_READ);

	NM = JE_RAM_ALLOCATE_STRUCT(jeNameMgr);

	if (!NM)
		return NULL;

	memset(NM, 0, sizeof(*NM));

	NM->RefCount = 1;
    NM->Flags = CreateFlags;

    NM->List = jeChain_Create();

	if (!NM->List)
		{
		jeRam_Free(NM);
		return NULL;
		}

	NM->System = System;

	return NM;
}

JETAPI jeBoolean JETCC jeNameMgr_CreateRef(jeNameMgr *NameMgr)
{
	assert(NameMgr);

	NameMgr->RefCount++;

	return JE_TRUE;
}

JETAPI void JETCC jeNameMgr_Destroy(jeNameMgr **NameMgr)
{
    jeChain_Link *Link;

	assert(NameMgr);

	(*NameMgr)->RefCount --;

	if ((*NameMgr)->RefCount == 0)
	{
        // close open name manager directory
        if ((*NameMgr)->Dir)
            {
		    jeVFile_Close((*NameMgr)->Dir);
            }

        // get rid of data from the list
		for (Link = jeChain_GetFirstLink((*NameMgr)->List); Link; Link = jeChain_LinkGetNext(Link))
		{
			// locals
			ReadWriteData	*RWData;

			// get data pointer
			RWData = (ReadWriteData *)jeChain_LinkGetLinkData( Link );
            jeRam_Free(RWData);
		}

        // destroy the list
        jeChain_Destroy(&(*NameMgr)->List);

		jeRam_Free(*NameMgr);
	}

	*NameMgr = NULL;
}

JETAPI jeBoolean JETCC jeNameMgr_Write(jeNameMgr *NM, jeVFile *VFile, void *PtrToData, jeNameMgr_WriteToFileCallback CB_Write)
	{
	void *Data;
    char NameString[JE_NAME_MGR_NAME_SIZE+1];

	assert(NM);
	assert(VFile);
	assert(PtrToData);
	assert(CB_Write);
    assert(NM->Flags == JE_NAME_MGR_CREATE_FOR_WRITE);

    if (!PtrToText(PtrToData, NameString))
		return JE_FALSE;

	jeVFile_Write(VFile, NameString, JE_NAME_MGR_NAME_SIZE);

    Data = FindCallbackDataByPtr(NM, PtrToData);
    if (!Data)
        {
	    SaveCallbackData(NM, NameString, PtrToData, CB_Write);
        }

	return JE_TRUE;
	}

JETAPI jeBoolean JETCC jeNameMgr_Read(jeNameMgr *NM, jeVFile *VFile, jeNameMgr_CreateFromFileCallback CB_Read, void **ReturnPointer)
	{
	ReadWriteData *Data;
	char NameString[JE_NAME_MGR_NAME_SIZE+1];
	jeVFile *File = NULL;

	assert(NM);
	assert(VFile);
	assert(ReturnPointer);
	assert(CB_Read);

    assert(NM->Flags == JE_NAME_MGR_CREATE_FOR_READ);

	jeVFile_Read(VFile, NameString, JE_NAME_MGR_NAME_SIZE);
    NameString[JE_NAME_MGR_NAME_SIZE] = '\0';

    Data = FindCallbackData(NM, NameString);
	if (Data && Data->DataPtr)
		{
        *ReturnPointer = Data->DataPtr;
		return JE_TRUE;
		}
	else
		{
		void *DataPtr;

        if (!NM->Dir)
            {
	        NM->Dir = jeVFile_Open(NM->System, "NameMgr", JE_VFILE_OPEN_READONLY|JE_VFILE_OPEN_DIRECTORY);
            }

		File = jeVFile_Open(NM->Dir, NameString, JE_VFILE_OPEN_READONLY);

		if (!File)
			goto Error;

		DataPtr = CB_Read(File, NM);

		if (!DataPtr)
			goto Error;

		jeVFile_Close(File);
		File = NULL;

		*ReturnPointer = DataPtr;

	    if (!SaveCallbackData(NM, NameString, DataPtr, NULL))
			goto Error;

		return JE_TRUE;
		}

	return JE_TRUE;

	Error:

	if (File)
		jeVFile_Close(File);

	return JE_FALSE;
	}

JETAPI jeBoolean JETCC jeNameMgr_WriteFlush(jeNameMgr *NM)
	{
	jeVFile *File = NULL;
	jeChain_Link *Link;//, *Next;
	char NameString[JE_NAME_MGR_NAME_SIZE+1];
	ReadWriteData *RWData;
	int Count, i, ret;

	assert(NM);
    assert(NM->Flags == JE_NAME_MGR_CREATE_FOR_WRITE);
	assert(NM->System);

	assert(NM->Dir == NULL);

	NM->Dir = jeVFile_Open(NM->System, "NameMgr", JE_VFILE_OPEN_CREATE | JE_VFILE_OPEN_DIRECTORY);

	if (!NM->Dir)
		goto Error;

	assert(NM->List);

	Count = jeChain_GetLinkCount(NM->List);
	for (i = 0; i < Count; i++)
		{
		Link = jeChain_GetLinkByIndex(NM->List, i);
   		// get data pointer
		RWData = (ReadWriteData *)jeChain_LinkGetLinkData( Link );

		if (RWData->Flushed)
			continue;

        memcpy(NameString, RWData->PointerText, JE_NAME_MGR_NAME_SIZE);
        NameString[JE_NAME_MGR_NAME_SIZE] = '\0';

		File = jeVFile_Open(NM->Dir, NameString, JE_VFILE_OPEN_CREATE);

		if (!File)
			goto Error;

		ret = RWData->WriteToFile(RWData->DataPtr, File, NM);
		Count = jeChain_GetLinkCount(NM->List); // get a new count

		if (!ret)
			goto Error;

		jeVFile_Close(File);
		File = NULL;

		RWData->Flushed = JE_TRUE;
		}

	jeVFile_Close(NM->Dir);
	NM->Dir = NULL;

	return JE_TRUE;

	Error:

	if (File)
		jeVFile_Close(File);

	if (NM->Dir)
		jeVFile_Close(NM->Dir);

	return JE_FALSE;
}

#endif

#if 0
//	Example read code
jeActor *jeActor_CreateFromFile(jeVFile *VFile)
{
	// Create a new actor
	Actor = JE_RAM_ALLOCATE_STRUCT(jeActor);

	if (!Actor)
		return NULL;
	
	if (!jeVFile_Read(VFile, &Actor->Number, sizeof(Actor->Number))
		goto ExitWithError;

	return Actor;

	ExitWithError:
	{
		if (Actor)
			jeRam_Free(Actor);

		return NULL;
	}
}

//	Example write code
jeBoolean jeActor_WriteToFile(const jeActor *Actor, jeVFile *VFile)
{
	uint32		Count;

	if (!jeVFile_Write(VFile, &Actor->Number, sizeof(Actor->Number))
		return JE_FALSE:

	return JE_TRUE;
}


jeBoolean jeWorld_Load(World,NameMgr,VFile)
{
	read ..  ..

	vfile_read(&NumActors);
	for (i=0;NumActors; i++)
		if (!jeNameMgr_Read(NameMgr, VFile, jeActor_ReadFromFile, &(Actor[i])))
			{
				log a failed load;
			}

	read .. . . .

}

jeBoolean jeWorld_Save(World,NameMgr,VFile)
{
	jeNameMgr_Create();

	write .. .. .

	vfile_write(NumActors);
	for (i=0;NumActors; i++)
		if (!jeNameMgr_Write(NameMgr, VFile, jeActor_WriteFromFile, &(Actor[i])))
			{
				ErrorLog;
			}

	write  .. . ..

	jeNameMgr_Flush(NameMgr);
	jeNameMgr_Destroy(NameMgr);
}


jeNameMgr_Write(NameMgr, VFile, Name, Obj, CallbackWrite);
jeNameMgr_Read(NameMgr, VFile, CallbackRead, ReturnPointer);



// search for CreateFromFile		
actor.h(106):JETAPI jeActor_Def *JETCC jeActor_DefCreateFromFile(jeVFile *pFile);
array.h(51):extern jeArray *		jeArray_CreateFromFile(jeVFile * File,jeArray_IOFunc ElementReader,void *ReaderContext);
bitmap.h(44):JETAPI jeBitmap *	JETCC	jeBitmap_CreateFromFile( jeVFile *F );
bitmap.h(45):JETAPI jeBitmap *	JETCC	jeBitmap_CreateFromFileName(const jeVFile *BaseFS,const char *Name);
bitmap.h(243):JETAPI jeBitmap_Palette *	JETCC	jeBitmap_Palette_CreateFromFile(jeVFile *F);
body.h(133):JETAPI jeBody  *JETCC  jeBody_CreateFromFile(jeVFile *pFile);
jeBrush.h(64):JETAPI jeBrush		*jeBrush_CreateFromFile(jeVFile *VFile, jePtrMgr *PtrMGr);
jeChain.h(39):jeChain		*jeChain_CreateFromFile(jeVFile *VFile, jeChain_IOFunc *IOFunc, void *Context, jePtrMgr *PtrMgr);
jeFaceInfo.h(73):JETAPI jeFaceInfo_Array *jeFaceInfo_ArrayCreateFromFile(jeVFile *VFile, jeGArray_IOFunc *IOFunc, void *IOFuncContext, jePtrMgr *PtrMgr);
jeGArray.h(47):jeGArray	*jeGArray_CreateFromFile(jeVFile *VFile, jeGArray_IOFunc *IOFunc, void *IOFuncContext, jePtrMgr *PtrMgr);
jeIndexPoly.h(41):jeIndexPoly *jeIndexPoly_CreateFromFile(jeVFile *VFile);
jeLight.h(31):JETAPI jeLight		*jeLight_CreateFromFile(jeVFile *VFile, jePtrMgr *PtrMgr);
jeMaterial.h(47):JETAPI jeMaterial_Array		*jeMaterial_ArrayCreateFromFile(jeVFile *VFile, jePtrMgr *PtrMgr);
jePtrMgr.h(47):jeActor *jeActor_CreateFromFile(jeVFile *VFile, jePtrMgr *PtrMgr)
jeVertArray.h(39):JETAPI jeVertArray		*jeVertArray_CreateFromFile(jeVFile *VFile);
jeModel.h(35):JETAPI jeModel		*jeModel_CreateFromFile(jeVFile *VFile, jePtrMgr *PtrMgr);
jeWorld.h(71):JETAPI jeWorld	*	jeWorld_CreateFromFile(jeVFile *VFile, jePtrMgr *PtrMgr, jeResourceMgr * pResourceMgr );
motion.h(150):JETAPI jeMotion *JETCC jeMotion_CreateFromFile(jeVFile *f);
object.h(99):JETAPI jeObject *	JETCC jeObject_CreateFromFile(jeVFile * File, jeObjectIO *ObjIO);
object.h(186):	void *		( JETCC * CreateFromFile)(jeVFile * File, jeObjectIO *ObjIO);
path.h(128):JETAPI jePath* JETCC jePath_CreateFromFile(jeVFile *F);
strblock.h(28):JETAPI jeStrBlock* JETCC jeStrBlock_CreateFromFile(jeVFile* pFile);
terrain.h(51):JETAPI jeTerrain *	JETCC jeTerrain_CreateFromFile(jeVFile * File, jePtrMgr *PtrMgr);

// search for WriteToFile
array.h(52):extern jeBoolean		jeArray_WriteToFile(const jeArray * Array,jeVFile * File,jeArray_IOFunc ElementWriter,void *WriterContext);
bitmap.h(46):JETAPI jeBoolean 	JETCC	jeBitmap_WriteToFile( const jeBitmap *Bmp, jeVFile *F );
bitmap.h(47):JETAPI jeBoolean	JETCC	jeBitmap_WriteToFileName(const jeBitmap * Bmp,const jeVFile *BaseFS,const char *Name);
bitmap.h(258):JETAPI jeBoolean		JETCC	jeBitmap_Palette_WriteToFile(const jeBitmap_Palette *Palette,jeVFile *F);
body.h(132):JETAPI jeBoolean JETCC jeBody_WriteToFile(const jeBody *B, jeVFile *pFile);
jeBrush.h(66):JETAPI jeBoolean	jeBrush_WriteToFile(const jeBrush *Brush, jeVFile *VFile, jePtrMgr *PtrMGr);
jeChain.h(40):jeBoolean	jeChain_WriteToFile(const jeChain *Chain, jeVFile *VFile, jeChain_IOFunc *IOFunc, void *Context, jePtrMgr *PtrMgr);
jeGArray.h(48):jeBoolean	jeGArray_WriteToFile(const jeGArray *Array, jeVFile *VFile, jeGArray_IOFunc *IOFunc, void *IOFuncContext, jePtrMgr *PtrMgr);
jeIndexPoly.h(42):jeBoolean	jeIndexPoly_WriteToFile(const jeIndexPoly *Poly, jeVFile *VFile);
jeLight.h(33):JETAPI jeBoolean	jeLight_WriteToFile(const jeLight *Light, jeVFile *VFile, jePtrMgr *PtrMgr);
jePtrMgr.h(91):jeBoolean jeActor_WriteToFile(const jeActor *Actor, jeVFile *VFile, jePtrMgr *PtrMgr)
jeVertArray.h(40):JETAPI jeBoolean		jeVertArray_WriteToFile(const jeVertArray *Array, jeVFile *VFile);
jeModel.h(36):JETAPI jeBoolean	jeModel_WriteToFile(const jeModel *Model, jeVFile *VFile, jePtrMgr *PtrMgr);
jeWorld.h(72):JETAPI jeBoolean	jeWorld_WriteToFile(const jeWorld *World, jeVFile *VFile, jePtrMgr *PtrMgr);
motion.h(151):JETAPI jeBoolean JETCC jeMotion_WriteToFile(const jeMotion *M,jeVFile *pFile);
object.h(100):JETAPI jeBoolean	JETCC jeObject_WriteToFile(const jeObject * Object,jeVFile * File, jeObjectIO *ObjIO);
path.h(131):JETAPI jeBoolean JETCC jePath_WriteToFile(const jePath *P, jeVFile *F);
strblock.h(29):JETAPI jeBoolean JETCC jeStrBlock_WriteToFile(const jeStrBlock *SB,jeVFile *pFile);
terrain.h(55):JETAPI jeBoolean 	JETCC jeTerrain_WriteToFile(const jeTerrain *pTerrain,jeVFile * File, jePtrMgr *PtrMgr);


// search for jePtrMgr
jeBrush.h(64):JETAPI jeBrush		*jeBrush_CreateFromFile(jeVFile *VFile, jePtrMgr *PtrMGr);
jeBrush.h(66):JETAPI jeBoolean	jeBrush_WriteToFile(const jeBrush *Brush, jeVFile *VFile, jePtrMgr *PtrMGr);
jeChain.h(39):jeChain		*jeChain_CreateFromFile(jeVFile *VFile, jeChain_IOFunc *IOFunc, void *Context, jePtrMgr *PtrMgr);
jeChain.h(40):jeBoolean	jeChain_WriteToFile(const jeChain *Chain, jeVFile *VFile, jeChain_IOFunc *IOFunc, void *Context, jePtrMgr *PtrMgr);
jeFaceInfo.h(73):JETAPI jeFaceInfo_Array *jeFaceInfo_ArrayCreateFromFile(jeVFile *VFile, jeGArray_IOFunc *IOFunc, void *IOFuncContext, jePtrMgr *PtrMgr);
jeFaceInfo.h(74):JETAPI jeBoolean	jeFaceInfo_ArrayWriteToFile(const jeFaceInfo_Array *Array, jeVFile *VFile, jeGArray_IOFunc *IOFunc, void *IOFuncContext, jePtrMgr *PtrMgr);
jeGArray.h(47):jeGArray	*jeGArray_CreateFromFile(jeVFile *VFile, jeGArray_IOFunc *IOFunc, void *IOFuncContext, jePtrMgr *PtrMgr);
jeGArray.h(48):jeBoolean	jeGArray_WriteToFile(const jeGArray *Array, jeVFile *VFile, jeGArray_IOFunc *IOFunc, void *IOFuncContext, jePtrMgr *PtrMgr);
jeLight.h(31):JETAPI jeLight		*jeLight_CreateFromFile(jeVFile *VFile, jePtrMgr *PtrMgr);
jeLight.h(33):JETAPI jeBoolean	jeLight_WriteToFile(const jeLight *Light, jeVFile *VFile, jePtrMgr *PtrMgr);
jeMaterial.h(47):JETAPI jeMaterial_Array		*jeMaterial_ArrayCreateFromFile(jeVFile *VFile, jePtrMgr *PtrMgr);
jeMaterial.h(48):JETAPI jeBoolean			jeMaterial_ArrayWriteToFile(jeMaterial_Array *MatArray, jeVFile *VFile, jePtrMgr *PtrMgr);
jeModel.h(35):JETAPI jeModel		*jeModel_CreateFromFile(jeVFile *VFile, jePtrMgr *PtrMgr);
jeModel.h(36):JETAPI jeBoolean	jeModel_WriteToFile(const jeModel *Model, jeVFile *VFile, jePtrMgr *PtrMgr);
jeWorld.h(71):JETAPI jeWorld	*	jeWorld_CreateFromFile(jeVFile *VFile, jePtrMgr *PtrMgr, jeResourceMgr * pResourceMgr );
jeWorld.h(72):JETAPI jeBoolean	jeWorld_WriteToFile(const jeWorld *World, jeVFile *VFile, jePtrMgr *PtrMgr);
jeWorld.h(154):JETAPI jeObjectIO *jeWorld_CreateObjectIO(jeWorld *pWorld, jePtrMgr *PtrMgr);
jeWorld.h(157):JETAPI jePtrMgr *jeWorld_GetObjectIOPtrMgr(jeObjectIO *ObjIO);
terrain.h(51):JETAPI jeTerrain *	JETCC jeTerrain_CreateFromFile(jeVFile * File, jePtrMgr *PtrMgr);
terrain.h(55):JETAPI jeBoolean 	JETCC jeTerrain_WriteToFile(const jeTerrain *pTerrain,jeVFile * File, jePtrMgr *PtrMgr);

Brush.h(230):Brush *				Brush_CreateFromFile( jeVFile * pF, const int32 nVersion, jePtrMgr * pPtrMgr ) ;
BrushList.h(49):BrushList *			BrushList_CreateFromFile( jeVFile * pF, jePtrMgr * pPtrMgr ) ;
CamObj.h(64):Camera *		Camera_CreateFromFile( jeVFile * pF, jeWorld *pWorld, jePtrMgr *PtrMgr );
CamObj.h(65):jeBoolean		Camera_WriteToFile( Camera * pCamera, jeVFile * pF, jeWorld *pWorld, jePtrMgr *PtrMgr );
CameraList.h(43):CameraList *			CameraList_CreateFromFile( jeVFile * pF, jeWorld *pWorld, jePtrMgr *pPtrMgr  ) ;
CameraList.h(44):jeBoolean			CameraList_WriteToFile( CameraList * pList, jeVFile * pF, jeWorld *pWorld, jePtrMgr *pPtrMgr ) ;
Level.h(170):Level *				Level_CreateFromFile( jeVFile * pF, jeWorld * pWorld, MaterialList_Struct * pGlobalMaterials, jePtrMgr * pPtrMgr, float Version ) ;
Level.h(171):jeBoolean			Level_WriteToFile( Level * pLevel, jeVFile * pF, jePtrMgr * pPtrMgr ) ;
Light.h(91):Light * Light_CreateFromFile( jeVFile * pF, jeWorld * pWorld, jePtrMgr * pPtrMgr );
Light.h(92):jeBoolean Light_WriteToFile( Light * pLight, jeVFile * pF, jePtrMgr * pPtrMgr );
LightList.h(43):LightList *			LightList_CreateFromFile( jeVFile * pF, jeWorld  * pWorld, jePtrMgr * pPtrMgr ) ;
LightList.h(44):jeBoolean			LightList_WriteToFile( LightList * pList, jeVFile * pF, jePtrMgr * pPtrMgr ) ;
UserObj.h(53):UserObj * UserObj_CreateFromFile( jeVFile * pF, jeWorld *pWorld, jePtrMgr * pPtrMgr );
UserObj.h(54):jeBoolean UserObj_WriteToFile( UserObj * pUserObj, jeVFile * pF, jeWorld *pWorld, jePtrMgr * pPtrMgr );
model.h(72):Model *			Model_CreateFromFile( jeVFile * pF, const int32 nVersion, jeWorld *pWorld, jePtrMgr * pPtrMgr ) ;
modellist.h(45):ModelList *		ModelList_CreateFromFile( jeVFile * pF, jeWorld *pWorld, jePtrMgr * pPtrMgr ) ;
modellist.h(46):jeBoolean		ModelList_WriteToFile( ModelList * pList, jeVFile * pF, jeWorld *pWorld, jePtrMgr * pPtrMgr ) ;

#endif