/****************************************************************************************/
/*  PATHOBJ.C                                                                           */
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

//#include "forcelib.h"
#ifdef WIN32
#include <windows.h>
#endif

#include <memory.h>
#include <assert.h>
#include <math.h>
#include <stdlib.h>

#ifdef BUILD_BE
#include <image.h>
#include <sys/param.h>
#define min(a,b) (((a)<(b))?(a):(b))
#define max(a,b) (((a)>(b))?(a):(b))
#define stricmp strcasecmp
#endif

#include <string.h>
#include <float.h>
#include <limits.h>
#include "Errorlog.h"
#include "PathObj.h"
#include "jeTypes.h"
#include "jeProperty.h"
#include "Jet.h"
#include "Ram.h"
#include "Channel.h"
#include "Trigger.h"
#include "ObjectMsg.h"

////////////////////// IMPORTANT
// If you change the structure formats, data ids, then bump the MAJOR version number
// Minor version number changes will not change the format
//////////////////////


#define PATHOBJ_VERSION 1

typedef struct PathObj PathObj;

#ifdef WIN32
static HINSTANCE hInstance;
#endif

#ifdef BUILD_BE
static image_id hInstance;
#endif

static jeObject * PathObject_FindObjectFromName(jeWorld *World, char *NameToFind);
static jeBoolean PathObject_GetPropertyDataIdFromName(jeWorld *World, jeObject *Obj, char *NameToFind, int32 *PropDataId);
static jeBoolean PathObject_BuildObjectNameList(PathObj *pPathObj, int TimeLineNdx, int32 *NameCount);
static jeBoolean PathObject_BuildPropertyNameList(PathObj *pPathObj, int TimeLineNdx, int32 *NameCount);
static jeBoolean PathObject_GetPropertyInfoFromDataId(jeWorld *World, jeObject *Obj, int32 DataId, int32 *Type, jeProperty_Data *Data);
static jeBoolean PathObject_PutDataIntoChannelObject(PathObj *pPathObj, int TimeLineNdx, jeXForm3d *XF);
static jeBoolean PathObject_SampleMotion(PathObj *pPathObj, int TimeLineNdx);
static jeBoolean PathObject_ProcessEvents(PathObj *pPathObj, int TimeLineNdx, float StartTime, float EndTime);
static jeBoolean PathObject_ValidateObject(PathObj *pPathObj, int ChannelType, jeObject *Obj, int PropID);
static jeBoolean PathObject_BuildAllMotions(PathObj *pPathObj);
static jeBoolean PathObject_SampleXFormAtTime(PathObj *pPathObj, int TimeLineNdx, float Time, jeBoolean *XFormSet, jeXForm3d *XF);
static char	*NoSelection = "< none >";

#define ID_TO_INDEX_MOD_VALUE 100

#define PATHOBJ_ADD_POSITION_TIMELINE_BUTTON 20000
#define PATHOBJ_ADD_PROPERTY_TIMELINE_BUTTON 20001
#define PATHOBJ_SET_SELECTED_KEY			 20002

enum PathCommonID
{ 
	PATHOBJ_DETAILS_GROUP = PROPERTY_LOCAL_DATATYPE_START,
	PATHOBJ_DETAILS_LOOPTIME,
	PATHOBJ_DETAILS_POS_INTERP_LINEAR,
	PATHOBJ_DETAILS_POS_INTERP_HERMITE,
	PATHOBJ_DETAILS_POS_INTERP_HERMITE_ZERO,
	PATHOBJ_DETAILS_POS_END,
	PATHOBJ_DETAILS_ROT_INTERP_LINEAR,
	PATHOBJ_DETAILS_ROT_INTERP_SLERP,
	PATHOBJ_DETAILS_ROT_INTERP_SQUAD,
	PATHOBJ_DETAILS_ROT_END,
	PATHOBJ_DETAILS_END,
	PATHOBJ_NAMELIST,
	PATHOBJ_TIMELINE_START,
	PATHOBJ_TIMELINE_POS_CHANNEL,
	PATHOBJ_TIMELINE_ROT_CHANNEL,
	PATHOBJ_TIMELINE_EVENT_CHANNEL,
	PATHOBJ_TIMELINE_CUR_TIME,
	PATHOBJ_TIMELINE_END,
	PATHOBJ_COMMON_END
};

// Property id's for positional channels
enum PathPostionID
{ 
	PATHOBJ_POS = PATHOBJ_COMMON_END,
	PATHOBJ_POSX,
	PATHOBJ_POSY,
	PATHOBJ_POSZ,
	PATHOBJ_POS_END,
	PATHOBJ_POS_DEL_BUTTON,
};

// Property id's for property channels
enum PathPropertyBaseID
{ 
	PATHOBJ_PROP_PROPNAMELIST = PATHOBJ_COMMON_END,
	PATHOBJ_PROP_LAST
};

enum PathPropertyVecID
{ 
	PATHOBJ_VEC = PATHOBJ_PROP_LAST,
	PATHOBJ_VECX,
	PATHOBJ_VECY,
	PATHOBJ_VECZ,
	PATHOBJ_VEC_END,
	PATHOBJ_VEC_DEL_BUTTON,
};

enum PathPropertyColorID
{ 
	PATHOBJ_COLOR = PATHOBJ_PROP_LAST,
	PATHOBJ_COLOR_R,
	PATHOBJ_COLOR_G,
	PATHOBJ_COLOR_B,
	PATHOBJ_COLOR_END,
	PATHOBJ_COLOR_DEL_BUTTON,
};

enum PathPropertyFloatID
{ 
	PATHOBJ_FLOAT = PATHOBJ_PROP_LAST,
	PATHOBJ_FLOAT_DEL_BUTTON,
};

enum PathPropertyIntID
{ 
	PATHOBJ_INT = PATHOBJ_PROP_LAST,
	PATHOBJ_INT_DEL_BUTTON,
};

enum PathPropertyNoneID
{ 
	PATHOBJ_NONE_DEL_BUTTON = PATHOBJ_PROP_LAST,
};

// END: Property id's for property channels

#define DEFAULT_RADIUS 1024.0f
#define MIN_EVENT_STRING_SIZE 64

static 	jeBitmap	*pBitmap = NULL;

#define MAX_OBJ_NAME_LIST 2048
#define MAX_PROP_NAME_LIST 128

typedef struct TimeLineData
{
	jeObject				*Obj;			// Object to modify
	int						ChannelType;	// 0 for positional, 1 for property
	int						PropDataId;		// Primary id for properties 
	PROPERTY_FIELD_TYPE		PropFieldType;	// Not necessary if you have the DataId - but it comes in handy

	Channel					Channels[MAX_CHANNELS]; // cjp modified not to be channel channel..
	float					SampleTime;		// Takes into account looping time
	float					CurrTime;		// Where the time bar actually is. Does not take into account looping time.
	float					LastTime;		// Last position where the time bar actually was.

	jeBoolean				LoopTime;
	int						PosInterpType;
	int						RotInterpType;
	char					ObjectName[256];
	char					ObjectPropertyName[256];
	char					*ObjectNameList[MAX_OBJ_NAME_LIST];
	char					*PropertyNameList[MAX_PROP_NAME_LIST];
}TimeLineData;


typedef struct PathObj {
	jeMotion		*Motion[64];
	TimeLineData	TimeLineList[64];
	int				TimeLineCount;
	int				CurTimeLine;
	int				LastTimeLineModified;

	float			Time;
	jeBoolean		PlayMotions;
	int				RefCnt;
	jeBoolean		Dirty;
	jeWorld			*World;
} PathObj;

#define UTIL_MAX_RESOURCE_LENGTH	(128)
static char stringbuffer[UTIL_MAX_RESOURCE_LENGTH + 1];

enum PathControlID
{
	PATH_CONTROL_POSITION,
	PATH_CONTROL_PROPERTY,
};

void Util_StripChar(char *str, char *strip)
{
	char *ptr;
	
	assert(str);
	assert(strip);

	ptr = strstr(str, strip);

	if (!ptr)
		return;

	memmove(ptr, ptr + strlen(strip), strlen(ptr)+1);
}

jeBoolean Util_StrDupManagePtr(char **dest, char *src, int min_size)
{
	int len;

	assert(dest);
	assert(src);

	len = strlen(src)+1;

	if (*dest)
	{
		if ( len < min_size )
		{
			strcpy(*dest, src);
			return JE_TRUE;
		}

		jeRam_Free(*dest);
		*dest = NULL;
	}

	*dest = (char *)jeRam_Allocate(max(min_size, len));
	if (*dest == NULL)
	{
		jeErrorLog_Add( JE_ERR_MEMORY_RESOURCE, NULL );
		return JE_FALSE;
	}

	strcpy(*dest, src);
	return JE_TRUE;
}


void TimeEdit_FillTimeLineProperties(PathObj *pPathObj, char *Descr, jeProperty_List *List, int *ListNdx, int TimeLineNdx)
{
	int BaseID;
	TimeLineData *Element;
	char NewDescr[256];

	assert(pPathObj);
	assert(List);
	assert(ListNdx);

	BaseID = (TimeLineNdx * ID_TO_INDEX_MOD_VALUE);
	Element = &pPathObj->TimeLineList[TimeLineNdx];

	sprintf(NewDescr, "Controlling %s", Descr);
		
	jeProperty_FillTimeGroup( &List->pjeProperty[(*ListNdx)], NewDescr, BaseID + PATHOBJ_TIMELINE_START);
	(*ListNdx)++;

	List->pjeProperty[(*ListNdx)].Type = PROPERTY_CHANNEL_POS_TYPE;
	List->pjeProperty[(*ListNdx)].DataId = BaseID + PATHOBJ_TIMELINE_POS_CHANNEL;
	List->pjeProperty[(*ListNdx)].DataSize = sizeof(void*);
	List->pjeProperty[(*ListNdx)].Data.Ptr = &Element->Channels[CHANNEL_POS];
	List->pjeProperty[(*ListNdx)].FieldName = NULL;
	(*ListNdx)++;

	List->pjeProperty[(*ListNdx)].Type = PROPERTY_CHANNEL_ROT_TYPE;
	List->pjeProperty[(*ListNdx)].DataId = BaseID + PATHOBJ_TIMELINE_ROT_CHANNEL;
	List->pjeProperty[(*ListNdx)].DataSize = sizeof(void*);
	List->pjeProperty[(*ListNdx)].Data.Ptr = &Element->Channels[CHANNEL_ROT];
	List->pjeProperty[(*ListNdx)].FieldName = NULL;
	(*ListNdx)++;

	List->pjeProperty[(*ListNdx)].Type = PROPERTY_CHANNEL_EVENT_TYPE;
	List->pjeProperty[(*ListNdx)].DataId = BaseID + PATHOBJ_TIMELINE_EVENT_CHANNEL;
	List->pjeProperty[(*ListNdx)].DataSize = sizeof(void*);
	List->pjeProperty[(*ListNdx)].Data.Ptr = &Element->Channels[CHANNEL_EVENT];
	List->pjeProperty[(*ListNdx)].FieldName = NULL;
	(*ListNdx)++;

	jeProperty_FillFloat( &List->pjeProperty[(*ListNdx)], 
		"", Element->CurrTime, BaseID + PATHOBJ_TIMELINE_CUR_TIME, -FLT_MAX, FLT_MAX, 1.0f );

    List->pjeProperty[(*ListNdx)].Type = PROPERTY_CURTIME_TYPE;
	(*ListNdx)++;

	jeProperty_FillGroupEnd( &List->pjeProperty[(*ListNdx)], BaseID + PATHOBJ_TIMELINE_END);
	(*ListNdx)++;

}

void TimeEdit_FillDetails(PathObj *pPathObj, jeProperty_List *List, int *ListNdx, int TimeLineNdx)
{
	int BaseID;
	TimeLineData *Element;
	char *InterpLinearStr;
	char *InterpHermiteStr;
	char *InterpZeroStr;

	assert(pPathObj);
	assert(List);
	assert(ListNdx);

	BaseID = (TimeLineNdx * ID_TO_INDEX_MOD_VALUE);
	Element = &pPathObj->TimeLineList[TimeLineNdx];

	jeProperty_FillGroup( &List->pjeProperty[(*ListNdx)++], 
		"Details", BaseID + PATHOBJ_DETAILS_GROUP);

	jeProperty_FillCheck( &List->pjeProperty[(*ListNdx)++], 
		"Loop Time:", Element->LoopTime, BaseID + PATHOBJ_DETAILS_LOOPTIME );

    if (Element->ChannelType == PATH_CONTROL_PROPERTY)
    {
        InterpLinearStr = "Prop Interp Linear:";
        InterpHermiteStr = "Prop Interp Hermite:";
        InterpZeroStr = "Prop Interp Hermite Zero Div:";
    }
    else
    {
        InterpLinearStr = "Pos Interp Linear:";
        InterpHermiteStr = "Pos Interp Hermite:";
        InterpZeroStr = "Pos Interp Hermite Zero Div:";
    }

    jeProperty_FillRadio( &List->pjeProperty[(*ListNdx)++], 
        InterpLinearStr, Element->PosInterpType == JE_PATH_INTERPOLATE_LINEAR, 
        BaseID + PATHOBJ_DETAILS_POS_INTERP_LINEAR);
    jeProperty_FillRadio( &List->pjeProperty[(*ListNdx)++], 
        InterpHermiteStr, Element->PosInterpType == JE_PATH_INTERPOLATE_HERMITE, 
        BaseID + PATHOBJ_DETAILS_POS_INTERP_HERMITE);
    jeProperty_FillRadio( &List->pjeProperty[(*ListNdx)++], 
        InterpZeroStr, Element->PosInterpType == JE_PATH_INTERPOLATE_HERMITE_ZERO_DERIV, 
        BaseID + PATHOBJ_DETAILS_POS_INTERP_HERMITE_ZERO);
    jeProperty_FillGroupEnd( &List->pjeProperty[(*ListNdx)++], 
        BaseID + PATHOBJ_DETAILS_POS_END);

    if (Element->ChannelType == PATH_CONTROL_POSITION && !Element->Channels[CHANNEL_ROT].Disabled)
    {
        jeProperty_FillRadio( &List->pjeProperty[(*ListNdx)++], 
            "Rot Interp Linear:", Element->RotInterpType == JE_PATH_INTERPOLATE_LINEAR, 
            BaseID + PATHOBJ_DETAILS_ROT_INTERP_LINEAR);
        jeProperty_FillRadio( &List->pjeProperty[(*ListNdx)++], 
            "Rot Interp Slerp:", Element->RotInterpType == JE_PATH_INTERPOLATE_SLERP, 
            BaseID + PATHOBJ_DETAILS_ROT_INTERP_SLERP);
        jeProperty_FillRadio( &List->pjeProperty[(*ListNdx)++], 
            "Rot Interp Squad:", Element->RotInterpType == JE_PATH_INTERPOLATE_SQUAD, 
            BaseID + PATHOBJ_DETAILS_ROT_INTERP_SQUAD);
        jeProperty_FillGroupEnd( &List->pjeProperty[(*ListNdx)++], 
            BaseID + PATHOBJ_DETAILS_ROT_END);
    }

	jeProperty_FillGroupEnd( &List->pjeProperty[(*ListNdx)++], 
		BaseID + PATHOBJ_DETAILS_GROUP);
}

static jeBoolean PathObject_BuildPropertyList(PathObj * pPathObj, jeProperty_List **List, int *PropertyCount)
{
	int i;
	int32 NameCount;
	int PropertyListIndex;
	int BaseID;
	jeVec3d Pos = {0,0,0};
	TimeLineData *Element;

	assert(pPathObj);
	assert(List);
	assert(*List);
	assert(PropertyCount);

	PropertyListIndex = 0;

	jeProperty_FillButton( &(*List)->pjeProperty[PropertyListIndex++], 
		"Add Position Time Line", PATHOBJ_ADD_POSITION_TIMELINE_BUTTON);
	jeProperty_FillButton( &(*List)->pjeProperty[PropertyListIndex++], 
		"Add Property Time Line", PATHOBJ_ADD_PROPERTY_TIMELINE_BUTTON);

	for (i = 0; i < pPathObj->TimeLineCount; i++)
	{
		jeProperty_Data Data;

		BaseID = (i * ID_TO_INDEX_MOD_VALUE);

		Element = &pPathObj->TimeLineList[i];

		memset( &Data, 0, sizeof(Data)); // clear data in case its not set 

		switch (Element->ChannelType)
		{
        case PATH_CONTROL_POSITION:
            {
                const char *CurrName;

                NameCount = 0;
                PathObject_BuildObjectNameList(pPathObj, i, &NameCount);

                CurrName = NULL;
                if (Element->Obj)
                {
                    CurrName = jeObject_GetName(Element->Obj);
                    if (CurrName)
                    {
                        strcpy(Element->ObjectName, CurrName);
                        CurrName = Element->ObjectName;
                    }
                }

                if (CurrName == NULL)
                    CurrName = NoSelection;

                jeProperty_FillCombo( &(*List)->pjeProperty[PropertyListIndex++], 
                    "Object:", (char*)CurrName, BaseID + PATHOBJ_NAMELIST, NameCount, Element->ObjectNameList );

                TimeEdit_FillDetails(pPathObj, *List, &PropertyListIndex, i);

                TimeEdit_FillTimeLineProperties(pPathObj, (char*)CurrName, *List, &PropertyListIndex, i);

                if (Element->Obj)
                {
                    jeXForm3d XF;
                    jeObject_GetXForm(Element->Obj, &XF);
                    Data.Vector = XF.Translation;

                    if (Element->Channels[CHANNEL_POS].KeysSelected[0] >= 0 && Element->Channels[CHANNEL_ROT].KeysSelected[0] >= 0)
                    {
                        int PosSelNdx,RotSelNdx;
                        jeXForm3d XF;

                        // get rotation
                        RotSelNdx = Element->Channels[CHANNEL_ROT].KeysSelected[0];
                        XF = Element->Channels[CHANNEL_ROT].KeyData[RotSelNdx].XForm;

                        // get position
                        PosSelNdx = Element->Channels[CHANNEL_POS].KeysSelected[0];
                        Data.Vector = Element->Channels[CHANNEL_POS].KeyData[PosSelNdx].XForm.Translation;
                        XF.Translation = Element->Channels[CHANNEL_POS].KeyData[PosSelNdx].XForm.Translation;

                        // set xform
                        jeObject_SetXForm(Element->Obj, &XF);
                    }
                    else
                    if (Element->Channels[CHANNEL_ROT].KeysSelected[0] >= 0)
                    {
                        int RotSelNdx;
                        jeXForm3d XF, PosXF;
                        float Time;
                        jeBoolean Set;
                        jeVec3d Pos;

                        // start XF with current position/rotation
                        jeObject_GetXForm(Element->Obj, &XF);
                        Pos = XF.Translation;

                        // get rot keydata
                        RotSelNdx = Element->Channels[CHANNEL_ROT].KeysSelected[0];
                        XF = Element->Channels[CHANNEL_ROT].KeyData[RotSelNdx].XForm;

                        // sample for translation
                        Time = Element->Channels[CHANNEL_ROT].KeyList[RotSelNdx];
                        PathObject_SampleXFormAtTime(pPathObj, i, Time, &Set, &PosXF);
                        if (Set)
                        {
                            XF.Translation = PosXF.Translation;
                        }
                        else
                        {
                            XF.Translation = Pos;
                        }

                        Data.Vector = XF.Translation;

                        // set xform
                        jeObject_SetXForm(Element->Obj, &XF);
                    }
                    else
                    if (Element->Channels[CHANNEL_POS].KeysSelected[0] >= 0)
                    {
                        int PosSelNdx;
                        jeXForm3d XF, RotXF;
                        float Time;
                        jeBoolean Set;

                        // start XF with current position/rotation
                        jeObject_GetXForm(Element->Obj, &XF);

                        PosSelNdx = Element->Channels[CHANNEL_POS].KeysSelected[0];

                        // sample for rotation
                        Time = Element->Channels[CHANNEL_POS].KeyList[PosSelNdx];
                        PathObject_SampleXFormAtTime(pPathObj, i, Time, &Set, &RotXF);
                        if (Set)
                        {
                            XF = RotXF;
                        }

                        // set pos
                        XF.Translation = Element->Channels[CHANNEL_POS].KeyData[PosSelNdx].XForm.Translation;
                        Data.Vector = XF.Translation;

                        // set xform
                        jeObject_SetXForm(Element->Obj, &XF);
                    }
                }

                jeProperty_FillVec3dGroup( &(*List)->pjeProperty[PropertyListIndex++], 
                    "Position:", &Data.Vector, BaseID + PATHOBJ_POS);

                jeProperty_FillFloat( &(*List)->pjeProperty[PropertyListIndex++], 
                    "X:", Data.Vector.X, BaseID + PATHOBJ_POSX, -FLT_MAX, FLT_MAX, 1.0f );

                jeProperty_FillFloat( &(*List)->pjeProperty[PropertyListIndex++], 
                    "Y:", Data.Vector.Y, BaseID + PATHOBJ_POSY, -FLT_MAX, FLT_MAX, 1.0f );

                jeProperty_FillFloat( &(*List)->pjeProperty[PropertyListIndex++], 
                    "Z:", Data.Vector.Z, BaseID + PATHOBJ_POSZ, -FLT_MAX, FLT_MAX, 1.0f );

                jeProperty_FillGroupEnd( &(*List)->pjeProperty[PropertyListIndex++], 
                    BaseID + PATHOBJ_POS);

                jeProperty_FillButton( &(*List)->pjeProperty[PropertyListIndex++], 
                    "Delete This Time Line", BaseID + PATHOBJ_POS_DEL_BUTTON);
                break;
            }

        case PATH_CONTROL_PROPERTY:
            {
                char DescrBuff[256];
                const char *CurrObjName;
                const char *CurrPropName;
                int32 PropertyNameCount, ObjectNameCount;

                ObjectNameCount = 0;
                PathObject_BuildObjectNameList(pPathObj, i, &ObjectNameCount);

                CurrObjName = NULL;
                if (Element->Obj)
                {
                    CurrObjName = jeObject_GetName(Element->Obj);
                    if (CurrObjName)
                    {
                        strcpy(Element->ObjectName, CurrObjName);
                        CurrObjName = Element->ObjectName;
                    }
                }

                if (CurrObjName == NULL)
                    CurrObjName = NoSelection;

                jeProperty_FillCombo( &(*List)->pjeProperty[PropertyListIndex++], 
                    "Object:", (char*)CurrObjName, BaseID + PATHOBJ_NAMELIST, ObjectNameCount, Element->ObjectNameList );

                TimeEdit_FillDetails(pPathObj, *List, &PropertyListIndex, i);

                PropertyNameCount = 0;
                PathObject_BuildPropertyNameList(pPathObj, i, &PropertyNameCount);

                CurrPropName = NoSelection;
                if (Element->Obj && Element->PropDataId >= 0)
                {
                    CurrPropName = Element->ObjectPropertyName;
                }

                if (strcmp(CurrPropName, NoSelection) == 0)
                    sprintf(DescrBuff, "%s %s",CurrObjName, CurrPropName);
                else
                    sprintf(DescrBuff, "%s (%s)",CurrObjName, CurrPropName);

                TimeEdit_FillTimeLineProperties(pPathObj, DescrBuff, *List, &PropertyListIndex, i);

                jeProperty_FillCombo( &(*List)->pjeProperty[PropertyListIndex++], 
                    "Property:", (char*)CurrPropName, BaseID + PATHOBJ_PROP_PROPNAMELIST, PropertyNameCount, Element->PropertyNameList );

                if (Element->Obj)
                {
                    // load data from the object
                    PathObject_GetPropertyInfoFromDataId(pPathObj->World, Element->Obj, Element->PropDataId, NULL, &Data);
                }

                switch (Element->PropFieldType)
                {
                default:

                    jeProperty_FillButton( &(*List)->pjeProperty[PropertyListIndex++], 
                        "Delete This Time Line", BaseID + PATHOBJ_NONE_DEL_BUTTON);

                    break;
                case PROPERTY_INT_TYPE:

                    if (Element->Obj && Element->Channels[CHANNEL_POS].KeysSelected[0] >= 0)
                    {
                        int SelNdx = Element->Channels[CHANNEL_POS].KeysSelected[0];
                        Data.Int = (int)Element->Channels[CHANNEL_POS].KeyData[SelNdx].XForm.Translation.X;
                        jeObject_SetProperty(Element->Obj, Element->PropDataId, Element->PropFieldType, &Data );
                    }

                    jeProperty_FillInt( &(*List)->pjeProperty[PropertyListIndex++], 
                        "Value:", Data.Int, BaseID + PATHOBJ_INT, (float)-INT_MAX, (float)INT_MAX, (float)1 );

                    jeProperty_FillButton( &(*List)->pjeProperty[PropertyListIndex++], 
                        "Delete This Time Line", BaseID + PATHOBJ_INT_DEL_BUTTON);

                    break;

                case PROPERTY_FLOAT_TYPE:

                    if (Element->Obj && Element->Channels[CHANNEL_POS].KeysSelected[0] >= 0)
                    {
                        int SelNdx = Element->Channels[CHANNEL_POS].KeysSelected[0];
                        Data.Float = Element->Channels[CHANNEL_POS].KeyData[SelNdx].XForm.Translation.X;
                        jeObject_SetProperty(Element->Obj, Element->PropDataId, Element->PropFieldType, &Data );
                    }

                    jeProperty_FillFloat( &(*List)->pjeProperty[PropertyListIndex++], 
                        "Value:", Data.Float, BaseID + PATHOBJ_FLOAT, -FLT_MAX, FLT_MAX, 1.0f );

                    jeProperty_FillButton( &(*List)->pjeProperty[PropertyListIndex++], 
                        "Delete This Time Line", BaseID + PATHOBJ_FLOAT_DEL_BUTTON);
                    break;

                case PROPERTY_VEC3D_GROUP_TYPE:

                    if (Element->Obj && Element->Channels[CHANNEL_POS].KeysSelected[0] >= 0)
                    {
                        int SelNdx = Element->Channels[CHANNEL_POS].KeysSelected[0];
                        Data.Vector = Element->Channels[CHANNEL_POS].KeyData[SelNdx].XForm.Translation;
                        jeObject_SetProperty(Element->Obj, Element->PropDataId, Element->PropFieldType, &Data );
                    }

                    jeProperty_FillVec3dGroup( &(*List)->pjeProperty[PropertyListIndex++], 
                        "Vec:", &Data.Vector, BaseID + PATHOBJ_VEC);

                    jeProperty_FillFloat( &(*List)->pjeProperty[PropertyListIndex++], 
                        "X:", Data.Vector.X, BaseID + PATHOBJ_VECX, -FLT_MAX, FLT_MAX, 1.0f );

                    jeProperty_FillFloat( &(*List)->pjeProperty[PropertyListIndex++], 
                        "Y:", Data.Vector.Y, BaseID + PATHOBJ_VECY, -FLT_MAX, FLT_MAX, 1.0f );

                    jeProperty_FillFloat( &(*List)->pjeProperty[PropertyListIndex++], 
                        "Z:", Data.Vector.Z, BaseID + PATHOBJ_VECZ, -FLT_MAX, FLT_MAX, 1.0f );

                    jeProperty_FillGroupEnd( &(*List)->pjeProperty[PropertyListIndex++], 
                        BaseID + PATHOBJ_VEC);

                    jeProperty_FillButton( &(*List)->pjeProperty[PropertyListIndex++], 
                        "Delete This Time Line", BaseID + PATHOBJ_VEC_DEL_BUTTON);

                    break;

                case PROPERTY_COLOR_GROUP_TYPE:

                    if (Element->Obj && Element->Channels[CHANNEL_POS].KeysSelected[0] >= 0)
                    {
                        int SelNdx = Element->Channels[CHANNEL_POS].KeysSelected[0];
                        Data.Vector = Element->Channels[CHANNEL_POS].KeyData[SelNdx].XForm.Translation;
                        jeObject_SetProperty(Element->Obj, Element->PropDataId, Element->PropFieldType, &Data );
                    }

                    jeProperty_FillVec3dGroup( &(*List)->pjeProperty[PropertyListIndex++], 
                        "Color:", &Data.Vector, BaseID + PATHOBJ_COLOR);

                    jeProperty_FillFloat( &(*List)->pjeProperty[PropertyListIndex++], 
                        "Red:", Data.Vector.X, BaseID + PATHOBJ_COLOR_R, -FLT_MAX, FLT_MAX, 1.0f );

                    jeProperty_FillFloat( &(*List)->pjeProperty[PropertyListIndex++], 
                        "Green:", Data.Vector.Y, BaseID + PATHOBJ_COLOR_G, -FLT_MAX, FLT_MAX, 1.0f );

                    jeProperty_FillFloat( &(*List)->pjeProperty[PropertyListIndex++], 
                        "Blue:", Data.Vector.Z, BaseID + PATHOBJ_COLOR_B, -FLT_MAX, FLT_MAX, 1.0f );

                    jeProperty_FillGroupEnd( &(*List)->pjeProperty[PropertyListIndex++], 
                        BaseID + PATHOBJ_COLOR);

                    jeProperty_FillButton( &(*List)->pjeProperty[PropertyListIndex++], 
                        "Delete This Time Line", BaseID + PATHOBJ_COLOR_DEL_BUTTON);
                    break;

                }
            }
        }
    }

	*PropertyCount = PropertyListIndex;

	return JE_TRUE;
}

static jeObject * PathObject_FindObjectFromName(jeWorld *World, char *NameToFind)
{
	jeObject *CurrObject;
	const char *Name;

	assert (World);
	assert (NameToFind);

	// loop initializers
	CurrObject = NULL;

	while (TRUE)
	{
		CurrObject = jeWorld_GetNextObject(World, CurrObject);

		if (CurrObject == NULL)
			break;

		Name = jeObject_GetName( CurrObject );

		if (!Name) continue;

		if (stricmp(NameToFind, Name) == 0)
			return (CurrObject);
	}

	return NULL;
}

static jeBoolean PathObject_GetPropertyDataIdFromName(jeWorld *World, jeObject *Obj, char *NameToFind, int32 *PropDataId)
{
	jeProperty_List *List;
	int i;
	char str[1024];
	char *CurrGroupName = NULL;
	jeBoolean SuppressGroupName;

	assert (World);
	assert (Obj);
	assert (NameToFind);
	assert (PropDataId);

	if (jeObject_GetPropertyList(Obj, &List) == JE_FALSE)
	{
		return JE_FALSE;
	}

	// loop initializers
	*PropDataId = -1;

	for (i = 0; i < List->jePropertyN; i++)
	{
		SuppressGroupName = JE_FALSE;

		// only list properties that are valid for path-ing
		switch (List->pjeProperty[i].Type)
		{
		case PROPERTY_GROUP_END_TYPE:
			CurrGroupName = NULL;
			break;

		case PROPERTY_GROUP_TYPE:
			CurrGroupName = List->pjeProperty[i].FieldName;
			break;

		case PROPERTY_VEC3D_GROUP_TYPE:
		case PROPERTY_COLOR_GROUP_TYPE:
			SuppressGroupName = JE_TRUE;
			CurrGroupName = List->pjeProperty[i].FieldName;
			// no break! falls through on purpose!
		case PROPERTY_INT_TYPE:
		case PROPERTY_FLOAT_TYPE:

			if (!SuppressGroupName && CurrGroupName)
				sprintf(str, "%s->%s", CurrGroupName, List->pjeProperty[i].FieldName);
			else
				strcpy(str, List->pjeProperty[i].FieldName);

			if (strcmp(NameToFind, str) == 0)
			{
				*PropDataId = List->pjeProperty[i].DataId;
				jeProperty_ListDestroy(&List);
				return JE_TRUE;
			}

			break;
		}
	}

	jeProperty_ListDestroy(&List);
	return JE_FALSE;
}


static jeBoolean PathObject_BuildPropertyNameList(PathObj *pPathObj, int TimeLineNdx, int32 *NameCount)
{
	jeProperty_List *List;
	int i;
	TimeLineData *td;
	char *CurrGroupName = NULL;
	char str[1024];
	jeBoolean SuppressGroupName;

	assert (pPathObj);
	assert (NameCount);

	td = &pPathObj->TimeLineList[TimeLineNdx];

	// loop initializers
	*NameCount = 0;

	Util_StrDupManagePtr(&td->PropertyNameList[*NameCount], NoSelection, 32);
	(*NameCount)++;

	if (!td->Obj)
	{
		return JE_TRUE;
	}

	if (jeObject_GetPropertyList(td->Obj, &List) == JE_FALSE)
	{
		return JE_FALSE;
	}


	for (i = 0; i < List->jePropertyN; i++)
	{
		SuppressGroupName = JE_FALSE;

		// only list properties that are valid for path-ing
		switch (List->pjeProperty[i].Type)
		{
		case PROPERTY_GROUP_END_TYPE:
			CurrGroupName = NULL;
			break;

		case PROPERTY_GROUP_TYPE:
			CurrGroupName = List->pjeProperty[i].FieldName;
			break;

		case PROPERTY_VEC3D_GROUP_TYPE:
		case PROPERTY_COLOR_GROUP_TYPE:
			SuppressGroupName = JE_TRUE;
			CurrGroupName = List->pjeProperty[i].FieldName;
			// no break! falls through on purpose!
		case PROPERTY_INT_TYPE:
		case PROPERTY_FLOAT_TYPE:

			if (td->PropDataId != List->pjeProperty[i].DataId)
			{
				if (PathObject_ValidateObject(pPathObj, td->ChannelType, td->Obj, List->pjeProperty[i].DataId) == JE_FALSE)
					break;
			}

			if (!SuppressGroupName && CurrGroupName)
				sprintf(str, "%s->%s", CurrGroupName, List->pjeProperty[i].FieldName);
			else
				strcpy(str, List->pjeProperty[i].FieldName);

			Util_StrDupManagePtr(&td->PropertyNameList[*NameCount], str, 32);
			assert(td->PropertyNameList[*NameCount]);
			(*NameCount)++;

			break;
		}
	}

	jeProperty_ListDestroy(&List);
	return JE_TRUE;
}

static jeBoolean PathObject_ObjectHasProperties(PathObj *pPathObj, jeObject *Obj, jeBoolean *HasProperties)
{
	jeProperty_List *List;
	int i;

	assert (pPathObj);
	assert (HasProperties);

	*HasProperties = JE_FALSE;

	if (jeObject_GetPropertyList(Obj, &List) == JE_FALSE)
	{
		return JE_FALSE;
	}

	for (i = 0; i < List->jePropertyN; i++)
	{
		switch (List->pjeProperty[i].Type)
		{
		case PROPERTY_VEC3D_GROUP_TYPE:
		case PROPERTY_COLOR_GROUP_TYPE:
		case PROPERTY_INT_TYPE:
		case PROPERTY_FLOAT_TYPE:
			*HasProperties = JE_TRUE;
			break;
		}

		if (*HasProperties)
			break;
	}

	jeProperty_ListDestroy(&List);
	return JE_TRUE;
}

static jeBoolean PathObject_GetPropertyInfoFromDataId(jeWorld *World, jeObject *Obj, int32 DataId, int32 *Type, jeProperty_Data *Data)
{
	jeProperty_List *List;
	int i;

	assert (World);
	assert (Obj);

	if (jeObject_GetPropertyList(Obj, &List) == JE_FALSE)
	{
		return JE_FALSE;
	}

	for (i = 0; i < List->jePropertyN; i++)
	{
		if (List->pjeProperty[i].DataId == DataId)
		{
			if (Type)
				*Type = List->pjeProperty[i].Type;
			if (Data)
				*Data = List->pjeProperty[i].Data;

			jeProperty_ListDestroy(&List);
			return JE_TRUE;
		}
	}

	jeProperty_ListDestroy(&List);

	return (JE_FALSE);
}

static jeBoolean PathObject_ValidateObject(PathObj *pPathObj, int ChannelType, jeObject *Obj, int PropID)
{
	int i;

	assert (pPathObj);
	assert (Obj);

	// make sure an object/property is selected that has not already been selected for this PathObject

	for (i = 0; i < pPathObj->TimeLineCount; i++)
	{
		if (pPathObj->TimeLineList[i].ChannelType == ChannelType)
		{
			if (pPathObj->TimeLineList[i].ChannelType == PATH_CONTROL_POSITION)
			{
				if (pPathObj->TimeLineList[i].Obj == Obj)
				{
					return JE_FALSE;
				}
			}
			else
			if (pPathObj->TimeLineList[i].ChannelType == PATH_CONTROL_PROPERTY)
			{
				if (pPathObj->TimeLineList[i].Obj == Obj && pPathObj->TimeLineList[i].PropDataId == PropID)
				{
    				return JE_FALSE;
				}
			}
		}
	}

	return JE_TRUE;
}

static jeBoolean PathObject_BuildObjectNameList(PathObj *pPathObj, int TimeLineNdx, int32 *NameCount)
{
	jeObject *CurrObject;
	char *Name,*TypeName;
	TimeLineData *td;

	assert (pPathObj);
	assert (NameCount);

	td = &pPathObj->TimeLineList[TimeLineNdx];

	// loop initializers
	CurrObject = NULL;
	*NameCount = 0;

	Util_StrDupManagePtr(&td->ObjectNameList[*NameCount], NoSelection, 32);
	(*NameCount)++;

	while (TRUE)
	{
		CurrObject = jeWorld_GetNextObject(pPathObj->World, CurrObject);

		if (CurrObject == NULL)
			break;

		Name = (char *)jeObject_GetName( CurrObject );

		if (Name == NULL)
			continue;

		TypeName = (char*)jeObject_GetTypeName(CurrObject);

		if (TypeName)
		{
			// don't look at PathObjects
			if ( stricmp(TypeName, "PathObject") == 0)
			{
				continue;
			}
		}

		if (td->Obj != CurrObject)
		{
			if (td->ChannelType == PATH_CONTROL_PROPERTY)
			{
				jeBoolean HasProperties;
				PathObject_ObjectHasProperties(pPathObj, CurrObject, &HasProperties);

				if (!HasProperties)
					continue;
			}
			else
			if (td->ChannelType == PATH_CONTROL_POSITION)
			{
				int obj_flags = jeObject_GetXFormModFlags(CurrObject);

				if (obj_flags == 0)
				{
					continue;
				}

				if (PathObject_ValidateObject(pPathObj, td->ChannelType, CurrObject, -1) == JE_FALSE)
					continue;
			}
		}

		Util_StrDupManagePtr(&td->ObjectNameList[*NameCount], Name, 32);
		assert(td->ObjectNameList[*NameCount]);
		(*NameCount)++;
	}

	return JE_TRUE;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Object ATTACH, DETACH Routines
//
///////////////////////////////////////////////////////////////////////////////

void Destroy_Class( void )
{
}

#ifdef WIN32
void Init_Class( HINSTANCE hInst)
#endif
#ifdef BUILD_BE
void Init_Class( image_id hInst)
#endif
{
	//ParentWnd = hWnd;
	hInstance = hInst;
}

///////////////////////////////////////////////////////////////////////////////
//
//  Object INTERFACE Routines
//
///////////////////////////////////////////////////////////////////////////////

void * JETCC CreateInstance()
{
	PathObj *pPathObj;

	pPathObj = JE_RAM_ALLOCATE_STRUCT( PathObj );
	if( pPathObj == NULL )
		goto CI_ERROR;

	memset(pPathObj, 0, sizeof(*pPathObj));

	pPathObj->RefCnt = 1;

	return( pPathObj );

CI_ERROR:
	if (pPathObj)
		jeRam_Free( pPathObj );

	return( NULL );
}


void JETCC CreateRef(void * Instance)
{
	PathObj *pPathObj = (PathObj*)Instance;

	assert( Instance );

	pPathObj->RefCnt++;
}

static jeBoolean PathObject_ProcessEvents(PathObj *pPathObj, int TimeLineNdx, float StartTime, float EndTime)
{
	float Time;
	char *EventString;
	TimeLineData *td;
	jeMotion *Motion;
	jeObject *Obj;
	int MessageID;
	char *StrPtr;
	char *ptr;
	Object_EventData ed;

	//assert(StartTime <= EndTime);

	td = &pPathObj->TimeLineList[TimeLineNdx];
	Motion = pPathObj->Motion[TimeLineNdx];

	jeMotion_SetupEventIterator(Motion, StartTime, EndTime);

	while( jeMotion_GetNextEvent( Motion, &Time, (const char **)&EventString ) )
	{
		Obj = td->Obj;
		MessageID = 0;
		StrPtr = EventString;

		if (ptr = strstr(EventString,";")) // ptr now points to the semi-colon
	{
			char ObjName[256];

			sscanf(EventString,"%s %d", ObjName, &MessageID);
			Obj = PathObject_FindObjectFromName(pPathObj->World, ObjName);
			if (!Obj)
				return JE_FALSE;

			StrPtr = &ptr[1];		// move one past the semi-colon
			while(*StrPtr == ' ') // get rid of leading blanks
				StrPtr++;
		}

		ed.EventType = MessageID;
		ed.FromObj = td->Obj;
		ed.Args = StrPtr;

		jeObject_SendMessage(Obj, 0, &ed);
	}

	return JE_TRUE;
}

void JETCC PathObject_FreeTimeLine(PathObj *pPathObj, int TimeLineNdx)
{
	int k;
	int i = TimeLineNdx;
	TimeLineData *td;

	assert(pPathObj);
	assert(TimeLineNdx >= 0);

	td = &pPathObj->TimeLineList[TimeLineNdx];

	if (pPathObj->Motion[i])
		jeMotion_Destroy(&pPathObj->Motion[i]);

	if (td->Obj)
		jeObject_Destroy(&td->Obj);

	for (k = 0; k < td->Channels[CHANNEL_EVENT].KeyCount; k++)
	{
		if (td->Channels[CHANNEL_EVENT].KeyData[k].String)
		{
			jeRam_Free(td->Channels[CHANNEL_EVENT].KeyData[k].String);
			td->Channels[CHANNEL_EVENT].KeyData[k].String = NULL;
		}
	}

	for (i = 0; i < MAX_OBJ_NAME_LIST; i++)
	{
		if (td->ObjectNameList[i])
		{
			jeRam_Free(td->ObjectNameList[i]);
			td->ObjectNameList[i] = NULL;
		}
	}

	for (i = 0; i < MAX_PROP_NAME_LIST; i++)
	{
		if (td->PropertyNameList[i])
		{
			jeRam_Free(td->PropertyNameList[i]);
			td->PropertyNameList[i] = NULL;
		}
	}

}

jeBoolean JETCC Destroy(void **pInstance)
{
	PathObj **hPathObj = (PathObj**)pInstance;
	PathObj *pPathObj = *hPathObj;

	assert( pInstance );
	assert( pPathObj->RefCnt > 0 );

	pPathObj->RefCnt--;

	if( pPathObj->RefCnt == 0 )
	{
		int i;

		for (i = 0; i < pPathObj->TimeLineCount; i++)
		{
			PathObject_FreeTimeLine(pPathObj, i);
		}

		jeRam_Free( pPathObj );
	}

	return JE_TRUE;
}


jeBoolean JETCC Render(const void * Instance, const jeWorld * pWorld, const jeEngine *Engine, const jeCamera *Camera, const jeFrustum *CameraSpaceFrustum, jeObject_RenderFlags RenderFlags)
{
	PathObj *pPathObj = (PathObj*)Instance;

	assert( Instance );
	assert( pWorld );
	assert( Engine );
	assert( Camera );

	return( JE_TRUE );
}


jeBoolean	JETCC AttachWorld( void * Instance, jeWorld * pWorld )
{
	PathObj *pPathObj = (PathObj*)Instance;

	assert( Instance );

	pPathObj->World = pWorld;

	return( JE_TRUE );
}

jeBoolean	JETCC DettachWorld( void * Instance, jeWorld * pWorld )
{
	PathObj *pPathObj = (PathObj*)Instance;

	assert( Instance );

	pPathObj->World = NULL;

	return( JE_TRUE );
}
				
jeBoolean	JETCC AttachEngine ( void * Instance, jeEngine *Engine )
{
	PathObj *pPathObj = (PathObj*)Instance;

	assert( Instance );
	assert( Engine );

	return JE_TRUE;
	Instance;
}

jeBoolean	JETCC DettachEngine( void * Instance, jeEngine *Engine )
{
	assert( Instance );

	return( JE_TRUE );
	Instance;
}

jeBoolean	JETCC AttachSoundSystem( void * Instance, jeSound_System *SoundSystem )
{
	PathObj *pPathObj = (PathObj*)Instance;

	assert( Instance );

	return( JE_TRUE );
}

jeBoolean	JETCC DettachSoundSystem( void * Instance, jeSound_System *SoundSystem )
{
	PathObj *pPathObj = (PathObj*)Instance;

	assert( Instance );

	return( JE_TRUE );
	SoundSystem;
}

jeBoolean	JETCC Collision(const void *Object, const jeExtBox *Box, const jeVec3d *Front, const jeVec3d *Back, jeVec3d *Impact, jePlane *Plane)
{
	return( JE_FALSE );
}

jeBoolean JETCC SetMaterial(void * Instance,const jeBitmap *Bmp,const jeRGBA * Color)
{
	return( JE_TRUE );
}

jeBoolean JETCC GetMaterial(const void * Instance,jeBitmap **pBmp,jeRGBA * Color)
{
	return( JE_TRUE );
}

jeBoolean JETCC GetExtBox(const void * Instance,jeExtBox *BBox)
{
	/*
	jeVec3d Point = {0,0,0};
	assert( Instance );
	assert( BBox );

	BBox->Min.X = 0;
	BBox->Min.Y = 0;
	BBox->Min.Z = 0;
	BBox->Max.X = 0;
	BBox->Max.Y = 0;
	BBox->Max.Z = 0;

	jeExtBox_Set (  BBox, 
					Point.X-5.0f, Point.Y-5.0f, Point.Z-5.0f,
					Point.X+5.0f, Point.Y+5.0f, Point.Z+5.0f);
	*/

	return JE_FALSE;
	//return JE_TRUE;
	Instance, BBox;
}

////////////////////////////////////////////////////////////////////////////////////////
//
//	ObjUtil_ReadString()
//
////////////////////////////////////////////////////////////////////////////////////////
jeBoolean ObjUtil_ReadString(
	jeVFile	*File,		// file to read from
	char	**String )	// where to save string pointer
{

	// locals
	int			Size;
	jeBoolean	Result = JE_TRUE;

	// ensure valid data
	assert( File != NULL );
	assert( String != NULL );

	// read string
	Result &= jeVFile_Read( File, &( Size ), sizeof( Size ) );
	if ( ( Size > 0 ) && ( Result == JE_TRUE ) )
	{
		*String = (char *)jeRam_Allocate( Size );
		if ( *String == NULL )
		{
			return JE_FALSE;
		}
		Result &= jeVFile_Read( File, *String, Size );
	}

	// all done
	return Result;

} // ObjUtil_ReadString()


////////////////////////////////////////////////////////////////////////////////////////
//
//	ObjUtil_WriteString()
//
////////////////////////////////////////////////////////////////////////////////////////
jeBoolean ObjUtil_WriteString(
	jeVFile	*File,		// file to write to
	char	*String )	// string to write out
{

	// locals
	int			Size;
	jeBoolean	Result = JE_TRUE;

	// ensure valid data
	assert( File != NULL );
	assert( String != NULL );

	// write out complete
	Size = strlen( String ) + 1;
	assert( Size > 0 );
	Result &= jeVFile_Write( File, &Size, sizeof( Size ) );
	Result &= jeVFile_Write( File, String, Size );

	// all done
	return Result;

} // ObjUtil_WriteString()



void *	JETCC CreateFromFile(jeVFile * File, jePtrMgr *PtrMgr)

{
	PathObj	*Object;
	jeBoolean	Result = JE_TRUE;
	int			i,k;
	int			ProcCount;
	int			Ver;
	BYTE        Version;
	uint32        Tag;
	
	OutputDebugString("PathObject\n");

	// ensure valid data
	assert( File != NULL );
	assert( PtrMgr != NULL );

	Object = (PathObj *)CreateInstance();

	if (!Object) goto ExitErr;

	if(!jeVFile_Read(File, &Tag, sizeof(Tag)))
	{
		jeErrorLog_Add( JE_ERR_FILEIO_READ, "PathObject_CreateFromFile:Tag" );
		goto ExitErr;
	}

	if (Tag == FILE_UNIQUE_ID)
	{
		if (!jeVFile_Read(File, &Version, sizeof(Version)))
		{
    		jeErrorLog_Add( JE_ERR_FILEIO_READ, "PathObject_CreateFromFile:Version" );
	       	goto ExitErr;
		}
	}
	else
	{
		//for backwards compatibility with old object format
		jeVFile_Seek(File,-((int)sizeof(Tag)),JE_VFILE_SEEKCUR);
		if (!jeVFile_Read(File,  &Ver, sizeof(Ver)))
		    goto ExitErr;
		if (!jeVFile_Read(File,  &Ver, sizeof(Ver)))
            goto ExitErr;
		Version = 1;
	}


	if (Version >= 1)
	{
	
	    if (!jeVFile_Read(File,  &Object->TimeLineCount, sizeof(Object->TimeLineCount)))
		    goto ExitErr;

	    if (!jeVFile_Read(File,  &Object->CurTimeLine, sizeof(Object->CurTimeLine)))
		    goto ExitErr;

	    if (!jeVFile_Read(File,  &Object->LastTimeLineModified, sizeof(Object->LastTimeLineModified)))
		    goto ExitErr;

	    if (!jeVFile_Read(File,  &Object->Time, sizeof(Object->Time)))
		    goto ExitErr;

	    if (!jeVFile_Read(File,  &Object->PlayMotions, sizeof(Object->PlayMotions)))
		    goto ExitErr;
	}


	for (i = 0; i < Object->TimeLineCount; i++)
	{
		// read an entire time line
		if (!jeVFile_Read(File,  &Object->TimeLineList[i], sizeof(Object->TimeLineList[0])))
			goto ExitErr;

		// clear keydata for EVENTS
		memset(Object->TimeLineList[i].Channels[CHANNEL_EVENT].KeyData, 0, sizeof(Object->TimeLineList[0].Channels[0].KeyData));


		// clear allocated pointer values
		memset(Object->TimeLineList[i].ObjectNameList, 0, sizeof(Object->TimeLineList[0].ObjectNameList));
		memset(Object->TimeLineList[i].PropertyNameList, 0, sizeof(Object->TimeLineList[0].PropertyNameList));

		ProcCount = i;


		// read individual event keys
		for (k = 0; k < Object->TimeLineList[i].Channels[CHANNEL_EVENT].KeyCount; k++)
		{
			char *Buff;

			if (!ObjUtil_ReadString(File, &Buff))
				goto ExitErr;

			// KeyData.String will be null
			if (Util_StrDupManagePtr(&Object->TimeLineList[i].Channels[CHANNEL_EVENT].KeyData[k].String, Buff, MIN_EVENT_STRING_SIZE) == JE_FALSE)
				goto ExitErr;
			jeRam_Free (Buff);

		}
		Object->TimeLineList[i].Obj = jeObject_CreateFromFile(File, PtrMgr);
	}
	
	PathObject_BuildAllMotions(Object);

	return (Object);

ExitErr:

	printf("Error occured in CreateFromFile (PathObject)\n");
	if (Object)
	{
		Destroy((void **)&Object);
	}

	for (i = 0; i < ProcCount; i++)
	{
		for (k = 0; k < Object->TimeLineList[i].Channels[CHANNEL_EVENT].KeyCount; k++)
		{
			if (Object->TimeLineList[i].Channels[CHANNEL_EVENT].KeyData[k].String)
			{
				jeRam_Free(Object->TimeLineList[i].Channels[CHANNEL_EVENT].KeyData[k].String);
				Object->TimeLineList[i].Channels[CHANNEL_EVENT].KeyData[k].String = NULL;
			}
		}
	}

	return (NULL);
}


jeBoolean JETCC WriteToFile(
	const void	*Instance,
	jeVFile		*File,
	jePtrMgr	*PtrMgr )

{
	// locals
	PathObj	*Object;
	int			i,k;
	BYTE	Version = PATHOBJ_VERSION;
	uint32 Tag = FILE_UNIQUE_ID;
	int     iObjects=0;		// must be same type as Object->TimeLineCount JH

	// ensure valid data
	assert( Instance != NULL );
	assert( File != NULL );
	assert( PtrMgr != NULL );

	// get object data
	Object = (PathObj *)Instance;

	// count valid objects (added JH 2.5.2000)
	for (i = 0; i < Object->TimeLineCount; i++)
	{
		if (Object->TimeLineList[i].Obj!=NULL) iObjects++;
	}

	
	if (!jeVFile_Write(File, &Tag,sizeof(Tag)))
		goto ExitErr;
	
	if (!jeVFile_Write(File,  &Version, sizeof(Version)))
		goto ExitErr;
    
	if (!jeVFile_Write(File,  &iObjects, sizeof(iObjects)))
		goto ExitErr;

	if (!jeVFile_Write(File,  &Object->CurTimeLine, sizeof(Object->CurTimeLine)))
		goto ExitErr;

	if (!jeVFile_Write(File,  &Object->LastTimeLineModified, sizeof(Object->LastTimeLineModified)))
		goto ExitErr;

	if (!jeVFile_Write(File,  &Object->Time, sizeof(Object->Time)))
		goto ExitErr;

	if (!jeVFile_Write(File,  &Object->PlayMotions, sizeof(Object->PlayMotions)))
		goto ExitErr;

	for (i = 0; i < iObjects; i++)
	{
		// write a whole time line
	    if (Object->TimeLineList[i].Obj!=NULL)
		{
			if (!jeVFile_Write(File,  &Object->TimeLineList[i], sizeof(Object->TimeLineList[0])))
				goto ExitErr;

			// MAKE SURE AND SAVE THE CHANNEL STRINGS SEPERATELY!!!!!!!!

			// save the individual key strings
			for (k = 0; k < Object->TimeLineList[i].Channels[CHANNEL_EVENT].KeyCount; k++)
			{
				if (Object->TimeLineList[i].Channels[CHANNEL_EVENT].KeyData[k].String)
				{
					char *cp;
					int len;

					cp = Object->TimeLineList[i].Channels[CHANNEL_EVENT].KeyData[k].String;
					len = strlen(cp)+1;

					if (!ObjUtil_WriteString(File, cp))
						goto ExitErr;
				}
				else // No Text then write nothing
				{ 
                    if (!ObjUtil_WriteString(File, ""))
						goto ExitErr;
				}
			}
			
			if (!jeObject_WriteToFile(Object->TimeLineList[i].Obj, File, PtrMgr))
					goto ExitErr;
		}
	}

	// all done
	return TRUE;

	ExitErr:

	jeErrorLog_Add( JE_ERR_SYSTEM_RESOURCE, NULL );

	return JE_FALSE;
} // WriteToFile()


jeBoolean PathObject_InitAddTimeLine(PathObj *pPathObj, int ChannelType, int ChannelFlags)
{
	int ThisTimeLine;
	int i;

	assert(pPathObj);
	assert(ChannelType == PATH_CONTROL_POSITION || ChannelType == PATH_CONTROL_PROPERTY);

	ThisTimeLine = pPathObj->TimeLineCount;

	memset(&pPathObj->TimeLineList[ThisTimeLine], 0, sizeof(pPathObj->TimeLineList[0]));

	pPathObj->TimeLineList[ThisTimeLine].ChannelType = ChannelType;
	pPathObj->TimeLineList[ThisTimeLine].PropDataId = -1;
	pPathObj->TimeLineList[ThisTimeLine].PosInterpType = JE_PATH_INTERPOLATE_HERMITE;
	pPathObj->TimeLineList[ThisTimeLine].RotInterpType = JE_PATH_INTERPOLATE_SQUAD;

	for (i = 0; i < MAX_CHANNELS; i++)
	{
		pPathObj->TimeLineList[ThisTimeLine].Channels[i].Disabled = JE_TRUE;

		if (ChannelFlags & 1<<i)
		{
			memset(pPathObj->TimeLineList[ThisTimeLine].Channels[i].KeyData, 0, sizeof(pPathObj->TimeLineList[ThisTimeLine].Channels[i].KeyData));
			memset(pPathObj->TimeLineList[ThisTimeLine].Channels[i].KeyList, 0, sizeof(pPathObj->TimeLineList[ThisTimeLine].Channels[i].KeyList));
			memset(pPathObj->TimeLineList[ThisTimeLine].Channels[i].KeysSelected, -1, sizeof(pPathObj->TimeLineList[ThisTimeLine].Channels[i].KeysSelected));
			pPathObj->TimeLineList[ThisTimeLine].Channels[i].KeyCount = 0;
		}
	}

	pPathObj->TimeLineCount++;

	return JE_TRUE;
}

jeBoolean PathObject_EnableTimeLine(PathObj *pPathObj, int TimeLineNdx, int ChannelFlags)
{
	int i;

	assert(pPathObj);
	assert(TimeLineNdx >= 0 && TimeLineNdx <= 64);

	for (i = 0; i < MAX_CHANNELS; i++)
	{
		if (ChannelFlags & 1<<i)
		{
			pPathObj->TimeLineList[TimeLineNdx].Channels[i].Disabled = JE_FALSE;
		}
	}

	return JE_TRUE;
}

jeBoolean	JETCC GetPropertyList(void * Instance, jeProperty_List **List)
{
	PathObj *pPathObj = (PathObj*)Instance;
	int PropertyCount;
	jeProperty_List *LocalList;

	assert(Instance);
	assert(List);

	// Create a big list and build into that
	LocalList = jeProperty_ListCreate(2048);
	PathObject_BuildPropertyList(pPathObj, &LocalList, &PropertyCount);

	LocalList->pjeProperty = JE_RAM_REALLOC_ARRAY( LocalList->pjeProperty, jeProperty, PropertyCount);
	LocalList->jePropertyN = PropertyCount;

	LocalList->bDirty = pPathObj->Dirty;
	pPathObj->Dirty = JE_FALSE;

	*List = LocalList;

	if( *List == NULL )
		return( JE_FALSE );

	return( JE_TRUE );
}

jeBoolean PathObject_IsLooped(PathObj *pPathObj, int TimeLineNdx)
	{
	int i,k;

	float FirstPosTime = 9999999.9f, LastPosTime = -1.0f;
	float FirstRotTime = 9999999.9f, LastRotTime = -1.0f;
	int FirstPosNdx = -1, LastPosNdx = -1;
	int FirstRotNdx = -1, LastRotNdx = -1;
	TimeLineData *td;

	jeBoolean LoopValid = JE_TRUE;

	// this routine determines if the last and first key data is the same
	// does a seperate test for pos and rot

	td = &pPathObj->TimeLineList[TimeLineNdx];

	// find first and last pos/rot
	if (!td->Channels[CHANNEL_POS].Disabled)
	{
		i = CHANNEL_POS;

		for (k = 0; k < td->Channels[i].KeyCount; k++)
		{
			if (td->Channels[i].KeyList[k] < FirstPosTime)
			{
				FirstPosTime = td->Channels[i].KeyList[k];
				FirstPosNdx = k;
			}

			if (td->Channels[i].KeyList[k] > LastPosTime)
			{
				LastPosTime = td->Channels[i].KeyList[k];
				LastPosNdx = k;
			}
		}

		if (FirstPosNdx != LastPosNdx && FirstPosNdx >= 0)
		{
			if (memcmp(&td->Channels[i].KeyData[FirstPosNdx].XForm, &td->Channels[i].KeyData[LastPosNdx].XForm, sizeof(jeXForm3d)))
			{
				return JE_FALSE;
			}
		}
	}

	// find first and last pos/rot
	if (!td->Channels[CHANNEL_ROT].Disabled)
	{
		i = CHANNEL_ROT;

		for (k = 0; k < td->Channels[i].KeyCount; k++)
		{
			if (td->Channels[i].KeyList[k] < FirstRotTime)
			{
				FirstRotTime = td->Channels[i].KeyList[k];
				FirstRotNdx = k;
			}

			if (td->Channels[i].KeyList[k] > LastRotTime)
			{
				LastRotTime = td->Channels[i].KeyList[k];
				LastRotNdx = k;
			}
		}

		if (FirstRotNdx != LastRotNdx && FirstRotNdx >= 0)
		{
			if (memcmp(&td->Channels[i].KeyData[FirstRotNdx].XForm, &td->Channels[i].KeyData[LastRotNdx].XForm, sizeof(jeXForm3d)))
			{
				return JE_FALSE;
			}
		}
	}

	return (JE_TRUE);
}


jeBoolean PathObject_BuildMotion(PathObj *pPathObj, int TimeLineNdx)
{
	float Time;
	char String[256];
	jeXForm3d XForm;
	int i,k;
	int PathChannel[2] = {JE_PATH_TRANSLATION_CHANNEL, JE_PATH_ROTATION_CHANNEL};
	jePath *Path;
	int InsertIndex;
	TimeLineData *td;

	assert(pPathObj);
	assert(TimeLineNdx >= 0 && TimeLineNdx <= 64);

	if (pPathObj->Motion[TimeLineNdx] != NULL)
		jeMotion_Destroy(&pPathObj->Motion[TimeLineNdx]);

	pPathObj->Motion[TimeLineNdx] = jeMotion_Create(JE_TRUE);

	td = &pPathObj->TimeLineList[TimeLineNdx];

	Path = jePath_Create((jePath_Interpolator)td->PosInterpType, (jePath_Interpolator)td->RotInterpType, PathObject_IsLooped(pPathObj, TimeLineNdx));
	jePath_SetCutMode(Path, JE_TRUE);

	jeMotion_AddPath(pPathObj->Motion[TimeLineNdx], Path, "", &InsertIndex);

	for (i = 0; i < MAX_CHANNELS-1; i++)
	{
		if (!td->Channels[i].Disabled)
		{
			for (k = 0; k < td->Channels[i].KeyCount; k++)
			{
				Time = td->Channels[i].KeyList[k];
				XForm = td->Channels[i].KeyData[k].XForm;
				jePath_InsertKeyframe(Path, PathChannel[i], Time, &XForm); 
			}
		}
	}

	if (Path)
		jePath_Destroy(&Path); // will only decrement ref counter because the path was ref-ed by InsertKeyFrame

	i = CHANNEL_EVENT;
	if (!td->Channels[i].Disabled)
	{
		for (k = 0; k < td->Channels[i].KeyCount; k++)
		{
			if (td->Channels[i].KeyData[k].String)
			{
				Time = td->Channels[i].KeyList[k];
				strcpy(String, td->Channels[i].KeyData[k].String);
				jeMotion_InsertEvent(pPathObj->Motion[TimeLineNdx], Time, String);
			}
		}
	}

	return JE_TRUE;
}

jeBoolean PathObject_BuildAllMotions(PathObj *pPathObj)
{
	int i;

	for (i = 0; i < pPathObj->TimeLineCount; i++)
	{
		PathObject_BuildMotion(pPathObj, i);
	}

	return JE_TRUE;
}

static void PathObject_SafeGetTimeExtents(jePath *Path, float *StartTime, float *EndTime)
{
	int RotCount;
	int PosCount;
	float FirstPosTime, FirstRotTime;
	float LastPosTime, LastRotTime;
	jeXForm3d FirstXForm, LastXForm;
	#define MIN_PATH_INVALID_TIME -1.0f
	#define MAX_PATH_INVALID_TIME 9999999.0f

	assert(Path);
	assert(StartTime);
	assert(EndTime);

	PosCount = jePath_GetKeyframeCount(Path, JE_PATH_TRANSLATION_CHANNEL);
	RotCount = jePath_GetKeyframeCount(Path, JE_PATH_ROTATION_CHANNEL);

	if (PosCount == 0 && RotCount == 0)
	{
		*StartTime = MIN_PATH_INVALID_TIME;
		*EndTime = MIN_PATH_INVALID_TIME;
		return;
	}

	if (PosCount == 0)
	{
		FirstPosTime = MIN_PATH_INVALID_TIME;
		LastPosTime = MAX_PATH_INVALID_TIME;
	}
	else
	{
		jePath_GetKeyframe(Path, 0, JE_PATH_TRANSLATION_CHANNEL, &FirstPosTime, &FirstXForm);		// returns the matrix of the keyframe
		jePath_GetKeyframe(Path, PosCount-1, JE_PATH_TRANSLATION_CHANNEL, &LastPosTime, &LastXForm);		// returns the matrix of the keyframe
	}

	if (RotCount == 0)
	{
		FirstRotTime = MIN_PATH_INVALID_TIME;
		LastRotTime = MAX_PATH_INVALID_TIME;
	}
	else
	{
		jePath_GetKeyframe(Path, 0, JE_PATH_ROTATION_CHANNEL, &FirstRotTime, &FirstXForm);		// returns the matrix of the keyframe
		jePath_GetKeyframe(Path, RotCount-1, JE_PATH_ROTATION_CHANNEL, &LastRotTime, &LastXForm);		// returns the matrix of the keyframe
	}

	*StartTime = max(FirstPosTime, FirstRotTime);
	*EndTime = min(LastPosTime, LastRotTime);

	if (*EndTime == MAX_PATH_INVALID_TIME)
	{
		*EndTime = MIN_PATH_INVALID_TIME;
	}

	return;
}

static jeBoolean PathObject_SampleMotion(PathObj *pPathObj, int TimeLineNdx)
{
	jeXForm3d XForm;
	float Start, End;
	jePath *Path;
	TimeLineData *td;

	assert(pPathObj);
	assert(TimeLineNdx >= 0 && TimeLineNdx < 64);

	td = &pPathObj->TimeLineList[TimeLineNdx];

	// update sample time whether it will actually be sampled or not - it will get over written below if needed
	td->SampleTime = td->CurrTime;

	if (!pPathObj->Motion[TimeLineNdx])
		return JE_TRUE;

	Path = jeMotion_GetPath(pPathObj->Motion[TimeLineNdx], 0);

	assert(Path);

	if (Path)
	{
		PathObject_SafeGetTimeExtents(Path, &Start, &End);

		if (Start == End || Start < 0.0f || End < 0.0f)
		{
			return JE_TRUE;
		}

		if (td->LoopTime)
		{
			float Diff, NormStart, NormEnd, LastTime, CurrTime;

			if (td->CurrTime < Start)
			{
				return JE_TRUE;
			}

			Diff = Start;

			NormStart = Start - Diff;
			NormEnd = End - Diff;

			LastTime = (float)fmod((double)td->LastTime-Diff, (double)(NormEnd-NormStart));
			CurrTime = (float)fmod((double)td->CurrTime-Diff, (double)(NormEnd-NormStart));

			CurrTime += Diff;

			td->SampleTime = CurrTime;

			if (CurrTime < Start)
				return JE_TRUE;
			
			jePath_Sample(Path, CurrTime, &XForm);
			if (PathObject_ProcessEvents(pPathObj, TimeLineNdx, LastTime, CurrTime) == JE_FALSE)
			{
				return JE_FALSE;
			}
			PathObject_PutDataIntoChannelObject(pPathObj, TimeLineNdx, &XForm);
		}
		else
		{
			td->SampleTime = td->CurrTime;

			if (td->CurrTime >= Start && td->CurrTime <= End)
			{
				jePath_Sample(Path, td->CurrTime, &XForm);
				if (PathObject_ProcessEvents(pPathObj, TimeLineNdx, td->LastTime, td->CurrTime) == JE_FALSE)
				{
					return JE_FALSE;
				}
				PathObject_PutDataIntoChannelObject(pPathObj, TimeLineNdx, &XForm);
			}
		}
	}

	return JE_TRUE;
}

static jeBoolean PathObject_SampleXFormAtTime(PathObj *pPathObj, int TimeLineNdx, float Time, jeBoolean *XFormSet, jeXForm3d *XF)
{
	float Start, End;
	jePath *Path;
	TimeLineData *td;

	assert(pPathObj);
	assert(TimeLineNdx >= 0 && TimeLineNdx < 64);

	td = &pPathObj->TimeLineList[TimeLineNdx];

	if (!pPathObj->Motion[TimeLineNdx])
	{
		*XFormSet = JE_FALSE;
		return JE_TRUE;
	}

	Path = jeMotion_GetPath(pPathObj->Motion[TimeLineNdx], 0);

	assert(Path);

	if (Path)
	{
		PathObject_SafeGetTimeExtents(Path, &Start, &End);

		if (Start == End || Start < 0.0f || End < 0.0f)
		{
			*XFormSet = JE_FALSE;
			return JE_TRUE;
		}

		jePath_Sample(Path, Time, XF);
	}

	*XFormSet = JE_TRUE;
	return JE_TRUE;
}

static jeBoolean PathObject_DeleteTimeLine(PathObj *pPathObj, int TimeLineNdx)
{
	PathObject_FreeTimeLine(pPathObj, TimeLineNdx);

	assert(pPathObj);
	assert(TimeLineNdx >= 0 && TimeLineNdx <= 64);

	if (TimeLineNdx < pPathObj->TimeLineCount-1)
	{
		//Delete This Time Line
		memmove(&pPathObj->TimeLineList[TimeLineNdx], 
			&pPathObj->TimeLineList[TimeLineNdx+1], 
			sizeof(pPathObj->TimeLineList[0]) * (pPathObj->TimeLineCount - TimeLineNdx - 1));

		memmove(&pPathObj->Motion[TimeLineNdx], 
			&pPathObj->Motion[TimeLineNdx+1], 
			sizeof(pPathObj->Motion[0]) * (pPathObj->TimeLineCount - TimeLineNdx - 1));
	}

	pPathObj->TimeLineCount--;

	return (JE_TRUE);
}

jeBoolean	JETCC SetProperty( void * Instance, int32 FieldID, PROPERTY_FIELD_TYPE DataType, jeProperty_Data * pData )
{
	PathObj *pPathObj = (PathObj*)Instance;
	int TimeLineNdx, ID;
	TimeLineData *Element;

	assert( Instance );
	assert( DataType >= 0 && DataType < PROPERTY_LAST );
	assert( pData );

	// ignore anything that i'm not supposed to deal with
	if (FieldID < PROPERTY_LOCAL_DATATYPE_START)
		return JE_TRUE;

	// Get an array index - (Range 0 to MAX_CHANNELS)
	TimeLineNdx = (FieldID - PROPERTY_LOCAL_DATATYPE_START) / ID_TO_INDEX_MOD_VALUE;

	// Convert FieldID into correct ID - (Range 10000 to 10006 or so)
	ID = PROPERTY_LOCAL_DATATYPE_START + (FieldID % ID_TO_INDEX_MOD_VALUE);

	// shortcut pointer to the element
	Element = &pPathObj->TimeLineList[TimeLineNdx];

	switch (FieldID)
	{
		case PATHOBJ_ADD_POSITION_TIMELINE_BUTTON:
		{
			PathObject_InitAddTimeLine(pPathObj, PATH_CONTROL_POSITION, 1<<CHANNEL_POS|1<<CHANNEL_ROT|1<<CHANNEL_EVENT);
			pPathObj->Dirty = JE_TRUE;
			return JE_TRUE;
		}
		case PATHOBJ_ADD_PROPERTY_TIMELINE_BUTTON:
		{
			PathObject_InitAddTimeLine(pPathObj, PATH_CONTROL_PROPERTY, 1<<CHANNEL_POS|1<<CHANNEL_EVENT);
			pPathObj->Dirty = JE_TRUE;
			return JE_TRUE;
		}
		case PATHOBJ_SET_SELECTED_KEY:
		{
			jeXForm3d XF;
			int SelNdx;
			TimeLineData *Element = &pPathObj->TimeLineList[0];

			SelNdx = Element->Channels[CHANNEL_POS].KeysSelected[0];
			if (SelNdx >= 0)
			{
				jeObject_GetXForm(Element->Obj, &XF);
				Element->Channels[CHANNEL_POS].KeyData[SelNdx].XForm.Translation = XF.Translation;
			}

			SelNdx = Element->Channels[CHANNEL_ROT].KeysSelected[0];
			if (SelNdx >= 0)
			{
				jeVec3d Pos;
				jeObject_GetXForm(Element->Obj, &XF);
				Pos = Element->Channels[CHANNEL_ROT].KeyData[SelNdx].XForm.Translation;
				Element->Channels[CHANNEL_ROT].KeyData[SelNdx].XForm = XF;
				Element->Channels[CHANNEL_ROT].KeyData[SelNdx].XForm.Translation = Pos;
			}

			return JE_TRUE;
		}
	}

	// this switch handles fields that DO NOT depend on a channel type
	switch (ID)
	{
	case PATHOBJ_DETAILS_POS_INTERP_LINEAR:
			Element->PosInterpType = JE_PATH_INTERPOLATE_LINEAR;
			PathObject_BuildMotion(pPathObj, TimeLineNdx);
			return JE_TRUE;
	case PATHOBJ_DETAILS_POS_INTERP_HERMITE:
			Element->PosInterpType = JE_PATH_INTERPOLATE_HERMITE;
			PathObject_BuildMotion(pPathObj, TimeLineNdx);
			return JE_TRUE;
	case PATHOBJ_DETAILS_POS_INTERP_HERMITE_ZERO:
			Element->PosInterpType = JE_PATH_INTERPOLATE_HERMITE_ZERO_DERIV;
			PathObject_BuildMotion(pPathObj, TimeLineNdx);
			return JE_TRUE;
	case PATHOBJ_DETAILS_ROT_INTERP_LINEAR:
			Element->RotInterpType = JE_PATH_INTERPOLATE_LINEAR;
			PathObject_BuildMotion(pPathObj, TimeLineNdx);
			return JE_TRUE;
	case PATHOBJ_DETAILS_ROT_INTERP_SLERP:
			Element->RotInterpType = JE_PATH_INTERPOLATE_SLERP;
			PathObject_BuildMotion(pPathObj, TimeLineNdx);
			return JE_TRUE;
	case PATHOBJ_DETAILS_ROT_INTERP_SQUAD:
			Element->RotInterpType = JE_PATH_INTERPOLATE_SQUAD;
			PathObject_BuildMotion(pPathObj, TimeLineNdx);
			return JE_TRUE;
	case PATHOBJ_DETAILS_LOOPTIME:
			Element->LoopTime = pData->Bool;
			return JE_TRUE;
	case PATHOBJ_TIMELINE_POS_CHANNEL:
		{
			memcpy(&Element->Channels[CHANNEL_POS], pData->Ptr, sizeof(Channel));
			return JE_TRUE;
		}
	case PATHOBJ_TIMELINE_ROT_CHANNEL:
		{
    		memcpy(&Element->Channels[CHANNEL_ROT], pData->Ptr, sizeof(Channel));
			return JE_TRUE;
		}
	case PATHOBJ_TIMELINE_EVENT_CHANNEL:
		{
			Channel *cp = (Channel *)pData->Ptr;
			int i;

			Element->Channels[CHANNEL_EVENT].KeyCount = cp->KeyCount;
			Element->Channels[CHANNEL_EVENT].Disabled = cp->Disabled;
			memcpy(&Element->Channels[CHANNEL_EVENT].KeyList, cp->KeyList, sizeof(cp->KeyList));
			memcpy(&Element->Channels[CHANNEL_EVENT].KeysSelected, cp->KeysSelected, sizeof(cp->KeysSelected));

			for (i = 0; i < cp->KeyCount; i++)
			{
				if (!Element->Channels[CHANNEL_EVENT].KeyData[i].String && !cp->KeyData[i].String)
				{
					// if both are null do nothing
					continue;
				}
				else
				if (Element->Channels[CHANNEL_EVENT].KeyData[i].String && cp->KeyData[i].String)
				{
					// if both are non null and equal do nothing
					if (strcmp(Element->Channels[CHANNEL_EVENT].KeyData[i].String, cp->KeyData[i].String) == 0)
						continue;
				}
				else
				if (Element->Channels[CHANNEL_EVENT].KeyData[i].String && !cp->KeyData[i].String)
				{
					// free and zero any old data
					jeRam_Free(Element->Channels[CHANNEL_EVENT].KeyData[i].String);
					Element->Channels[CHANNEL_EVENT].KeyData[i].String = NULL;
					continue;
				}

				// copy any new data
				if (cp->KeyData[i].String)
				{
					Util_StrDupManagePtr(&Element->Channels[CHANNEL_EVENT].KeyData[i].String, cp->KeyData[i].String, MIN_EVENT_STRING_SIZE);
				}
			}

			return JE_TRUE;
		}
		case PATHOBJ_TIMELINE_CUR_TIME:
		{
			if (pPathObj->PlayMotions && pData->Float == 0.0f)
			{
				pPathObj->Time = Element->LastTime = Element->CurrTime = 0.0f;
				return TRUE;
			}

			Element->LastTime = Element->CurrTime;

			if (pPathObj->PlayMotions)
			{
    			pPathObj->Time = Element->CurrTime;
			}

			if (Element->CurrTime == pData->Float)
			{
				// build the motion
				PathObject_BuildMotion(pPathObj, TimeLineNdx);
			}
			else
			{
				Element->CurrTime = pData->Float;
				PathObject_SampleMotion(pPathObj, TimeLineNdx);
			}

			return JE_TRUE;
		}
	}

	// this switch handles fields that DEPEND on a channel type
	switch (Element->ChannelType)
	{
		case PATH_CONTROL_POSITION:
			{
			int SelNdx = Element->Channels[CHANNEL_POS].KeysSelected[0];

            switch (ID)
            {
            case PATHOBJ_NAMELIST:
                {
                    int obj_flags, tl_flags;

                    if (strcmp(pData->String,NoSelection) == 0)
                        break;

                    if (Element->Obj)
                    {
                        // selected the same object
                        if (strcmp(pData->String, jeObject_GetName(Element->Obj)) == 0)
                            break;

                        jeObject_Destroy(&Element->Obj);
                    }

                    Element->Obj = PathObject_FindObjectFromName(pPathObj->World, pData->String);
                    jeObject_CreateRef(Element->Obj);

                    obj_flags = jeObject_GetXFormModFlags(Element->Obj);

                    tl_flags = 1<<CHANNEL_EVENT;

                    if (obj_flags & JE_OBJECT_XFORM_TRANSLATE)
                    {
                        tl_flags |= 1<<CHANNEL_POS;
                    }

                    if (obj_flags & JE_OBJECT_XFORM_ROTATE)
                    {
                        tl_flags |= 1<<CHANNEL_ROT;
                    }

                    PathObject_EnableTimeLine(pPathObj, TimeLineNdx, tl_flags);

                    pPathObj->Dirty = JE_TRUE;
                    break;
                }
            case PATHOBJ_POS:
                if (!Element->Obj)
                    break;

                if (SelNdx >= 0)
                {
                    Element->Channels[CHANNEL_POS].KeyData[SelNdx].XForm.Translation = pData->Vector;
                    jeObject_SetXForm(Element->Obj, &Element->Channels[CHANNEL_POS].KeyData[SelNdx].XForm);
                }
                else
                {
                    jeXForm3d XF;
                    jeObject_GetXForm(Element->Obj, &XF);
                    XF.Translation = pData->Vector;
                    jeObject_SetXForm(Element->Obj, &XF);
                }
                break;

            case PATHOBJ_POSX:
                if (!Element->Obj)
                    break;
                if (SelNdx >= 0)
                {
                    Element->Channels[CHANNEL_POS].KeyData[SelNdx].XForm.Translation.X = pData->Float;
                    jeObject_SetXForm(Element->Obj, &Element->Channels[CHANNEL_POS].KeyData[SelNdx].XForm);
                }
                else
                {
                    jeXForm3d XF;
                    jeObject_GetXForm(Element->Obj, &XF);
                    XF.Translation.X = pData->Float;
                    jeObject_SetXForm(Element->Obj, &XF);
                }

                pPathObj->LastTimeLineModified = TimeLineNdx;
                break;
            case PATHOBJ_POSY:
                if (!Element->Obj)
                    break;
                if (SelNdx >= 0)
                {
                    Element->Channels[CHANNEL_POS].KeyData[SelNdx].XForm.Translation.Y = pData->Float;
                    jeObject_SetXForm(Element->Obj, &Element->Channels[CHANNEL_POS].KeyData[SelNdx].XForm);
                }
                else
                {
                    jeXForm3d XF;
                    jeObject_GetXForm(Element->Obj, &XF);
                    XF.Translation.Y = pData->Float;
                    jeObject_SetXForm(Element->Obj, &XF);
                }

                pPathObj->LastTimeLineModified = TimeLineNdx;
                break;
            case PATHOBJ_POSZ:
                if (!Element->Obj)
                    break;

                if (SelNdx >= 0)
                {
                    Element->Channels[CHANNEL_POS].KeyData[SelNdx].XForm.Translation.Z = pData->Float;
                    jeObject_SetXForm(Element->Obj, &Element->Channels[CHANNEL_POS].KeyData[SelNdx].XForm);
                }
                else
                {
                    jeXForm3d XF;
                    jeObject_GetXForm(Element->Obj, &XF);
                    XF.Translation.Z = pData->Float;
                    jeObject_SetXForm(Element->Obj, &XF);
                }

                pPathObj->LastTimeLineModified = TimeLineNdx;
                break;
            case PATHOBJ_POS_DEL_BUTTON:
                {
                    PathObject_DeleteTimeLine(pPathObj, TimeLineNdx);
                    pPathObj->Dirty = JE_TRUE;
                    break;
                }
            }

            break;
            }

        case PATH_CONTROL_PROPERTY:
            {

                int SelNdx = Element->Channels[CHANNEL_POS].KeysSelected[0];

                if (ID == PATHOBJ_DETAILS_LOOPTIME)
                {
                    Element->LoopTime = pData->Bool;
                }
                else
                if (ID == PATHOBJ_NAMELIST)
                {
                    if (strcmp(pData->String,NoSelection) == 0)
                        break;

                    if (Element->Obj)
                    {
                        // selected the same object
                        if (strcmp(pData->String, jeObject_GetName(Element->Obj)) == 0)
                            break;
                        jeObject_Destroy(&Element->Obj);
                    }

                    Element->Obj = PathObject_FindObjectFromName(pPathObj->World, pData->String);
                    jeObject_CreateRef(Element->Obj);
                    pPathObj->Dirty = JE_TRUE;
                }
			    else
			    if (ID == PATHOBJ_PROP_PROPNAMELIST)
				{
				    if (strcmp(pData->String,NoSelection) == 0)
					    break;

				    if (pPathObj->World && Element->Obj)
					{
					    strcpy(Element->ObjectPropertyName, pData->String);

					    if (PathObject_GetPropertyDataIdFromName(pPathObj->World, Element->Obj, pData->String, (int32 *)&Element->PropDataId) == JE_FALSE)
						{
    						return JE_FALSE;
						}

					    // set the property field type
					    PathObject_GetPropertyInfoFromDataId(pPathObj->World, Element->Obj, 
						    Element->PropDataId, (int32 *)&Element->PropFieldType, NULL);
					    PathObject_EnableTimeLine(pPathObj, TimeLineNdx, 1<<CHANNEL_POS|1<<CHANNEL_EVENT);

					    pPathObj->Dirty = JE_TRUE;
					}
				}
    			else
	    		switch (Element->PropFieldType)
				{
				default:
					PathObject_DeleteTimeLine(pPathObj, TimeLineNdx);
					pPathObj->Dirty = JE_TRUE;
					break;

				case PROPERTY_INT_TYPE:
					if (ID == PATHOBJ_INT)
                    {
                        // if there is a key selected then we should modify the KeyData also
                        if (SelNdx >= 0)
                        {
                            Element->Channels[CHANNEL_POS].KeyData[SelNdx].XForm.Translation.X = (float)pData->Int;
                        }

                        // modify the object data whether it is selected or not
                        jeObject_SetProperty(Element->Obj, Element->PropDataId, Element->PropFieldType, pData );
                    }
					else
					if (ID == PATHOBJ_INT_DEL_BUTTON)
					{
						PathObject_DeleteTimeLine(pPathObj, TimeLineNdx);
						pPathObj->Dirty = JE_TRUE;
					}
					break;

				case PROPERTY_FLOAT_TYPE:
					if (ID == PATHOBJ_FLOAT)
					{
						if (SelNdx >= 0)
						{
							Element->Channels[CHANNEL_POS].KeyData[SelNdx].XForm.Translation.X = pData->Float;
						}

						jeObject_SetProperty(Element->Obj, Element->PropDataId, Element->PropFieldType, pData );
					}
    				else
					if (ID == PATHOBJ_FLOAT_DEL_BUTTON)
					{
						PathObject_DeleteTimeLine(pPathObj, TimeLineNdx);
						pPathObj->Dirty = JE_TRUE;
					}
					break;

				case PROPERTY_COLOR_GROUP_TYPE: // a color is the same thing as a vector - same id's organization
				case PROPERTY_VEC3D_GROUP_TYPE:

					if (ID == PATHOBJ_VEC)
					{
						if (SelNdx >= 0)
						{
							Element->Channels[CHANNEL_POS].KeyData[SelNdx].XForm.Translation = pData->Vector;
						}

						jeObject_SetProperty(Element->Obj, Element->PropDataId, Element->PropFieldType, pData );
					}
					else
					if (ID == PATHOBJ_VECX)
					{
						if (SelNdx >= 0)
						{
							Element->Channels[CHANNEL_POS].KeyData[SelNdx].XForm.Translation.X = pData->Float;
						}

						jeObject_SetProperty(Element->Obj, Element->PropDataId, Element->PropFieldType, pData );
					}
					else
					if (ID == PATHOBJ_VECY)
					{
						if (SelNdx >= 0)
						{
							Element->Channels[CHANNEL_POS].KeyData[SelNdx].XForm.Translation.Y = pData->Float;
						}

						jeObject_SetProperty(Element->Obj, Element->PropDataId, Element->PropFieldType, pData );
					}
					else
					if (ID == PATHOBJ_VECZ)
					{
						if (SelNdx >= 0)
						{
							Element->Channels[CHANNEL_POS].KeyData[SelNdx].XForm.Translation.Z = pData->Float;
						}

						jeObject_SetProperty(Element->Obj, Element->PropDataId, Element->PropFieldType, pData );
					}
                    else
                    if (ID == PATHOBJ_VEC_DEL_BUTTON)
                    {
                        PathObject_DeleteTimeLine(pPathObj, TimeLineNdx);
                        pPathObj->Dirty = JE_TRUE;
                    }

                    break;
			}

    		break;
		}
	}

	return( JE_TRUE );
}

jeBoolean	JETCC SetXForm(void * Instance,const jeXForm3d *XF)
{
	assert( Instance );
	assert( XF );

	return( JE_TRUE );
}

jeBoolean JETCC GetXForm(const void * Instance,jeXForm3d *XF)
{
	assert( Instance );
	assert( XF );

	return( JE_FALSE );
}

int	JETCC GetXFormModFlags( const void * Instance )
{
	Instance;
	return( 0 );
}

jeBoolean JETCC GetChildren(const void * Instance,jeObject * Children,int MaxNumChildren)
{
	return( JE_TRUE );
}

jeBoolean JETCC AddChild(void * Instance,const jeObject * Child)
{
	return( JE_TRUE );
}

jeBoolean JETCC RemoveChild(void * Instance,const jeObject * Child)
{
	return( JE_TRUE );
}

#ifdef WIN32
jeBoolean JETCC EditDialog (void * Instance,HWND Parent)
#endif
#ifdef BUILD_BE
jeBoolean JETCC EditDialog (void * Instance, class G3DView * Parent )
#endif
{
	return( JE_TRUE );
}

jeBoolean PathObject_PutDataIntoChannelObject(PathObj *pPathObj, int TimeLineNdx, jeXForm3d *XF)
{
	TimeLineData *td;

	assert(pPathObj);
	assert(TimeLineNdx >= 0 && TimeLineNdx < 64);
	assert(XF);

	td = &pPathObj->TimeLineList[TimeLineNdx];

	if (!td->Obj)
		return JE_TRUE;

	if (td->ChannelType == PATH_CONTROL_POSITION)
	{
			jeObject_SetXForm(td->Obj, XF);
	}
	else
	if (td->ChannelType == PATH_CONTROL_PROPERTY)
	{
		jeProperty_Data Data;
		switch (td->PropFieldType)
		{
        case PROPERTY_INT_TYPE:
            Data.Int = (int)XF->Translation.X;
            jeObject_SetProperty(td->Obj, td->PropDataId, td->PropFieldType, &Data );
            break;
        case PROPERTY_FLOAT_TYPE:
            Data.Float = XF->Translation.X;
            jeObject_SetProperty(td->Obj, td->PropDataId, td->PropFieldType, &Data );
            break;
        case PROPERTY_COLOR_GROUP_TYPE:
        case PROPERTY_VEC3D_GROUP_TYPE:
            Data.Vector = XF->Translation;
            jeObject_SetProperty(td->Obj, td->PropDataId, td->PropFieldType, &Data );
            break;
        }
    }

	return( JE_TRUE );
}


jeBoolean JETCC MessageFunction (void * Instance, int32 Msg, void * Data)
{
	PathObj *pPathObj = (PathObj*)Instance;

	assert( Instance );
	//assert( Data );

	switch (Msg)
	{
		default:
			return JE_FALSE;
			break;
		case OBJECT_EVENT_MSG:
		{
			Object_EventData *ed = (Object_EventData *)Data;

			switch (ed->EventType)
			{
            case EVENT_TYPE_TIMELINE_PLAY:
                pPathObj->Time = 0;
                pPathObj->PlayMotions = JE_TRUE;
                PathObject_BuildAllMotions(pPathObj);
                break;
            case EVENT_TYPE_TIMELINE_STOP:

				if (pPathObj->PlayMotions)
				{
					int i;
					// here we are stopping playback - set curr time to sample time
					for (i = 0; i < pPathObj->TimeLineCount; i++)
					{
						pPathObj->TimeLineList[i].CurrTime = pPathObj->TimeLineList[i].SampleTime;
					}
				}

				pPathObj->PlayMotions = JE_FALSE;
				break;
			}

			break;
			}
		case OBJECT_TIMELINE_GET_CURRENT_TIMELINE_MSG: // gets the last manipulated data time line index
			{
			Object_TimeGetTimeLine *ptr = (Object_TimeGetTimeLine *)Data;

			if (ptr->TimeLineNdx == pPathObj->LastTimeLineModified)
				break;

			ptr->ReturnTimeLineNdx = pPathObj->LastTimeLineModified;
			break;
			}
		case OBJECT_TIMELINE_GET_JEOBJECT_MSG: // this message gets the current jeObject based on the TimeLineNdx
			{
			Object_TimeGetObject *ptr = (Object_TimeGetObject *)Data;
			ptr->ReturnObj = pPathObj->TimeLineList[ptr->TimeLineNdx].Obj;
			break;
			}
		case OBJECT_TIMELINE_GET_KEY_DATA_MSG:
			{
			TimeLineData *td;
			int ChannelNdx,TimeLineNdx;
			Object_TimeKeyData *ptr = (Object_TimeKeyData *)Data;

			jeXForm3d_SetIdentity(&ptr->ReturnXF);
			
			TimeLineNdx = ptr->TimeLineNdx;
			ChannelNdx = ptr->ChannelNdx;

			pPathObj->CurTimeLine = TimeLineNdx;

			td = &pPathObj->TimeLineList[TimeLineNdx];

			if (!td->Obj)
				return JE_TRUE;

			if (td->ChannelType == PATH_CONTROL_POSITION)
			{
				jeObject_GetXForm(td->Obj, &ptr->ReturnXF);
			}
			else
			if (td->ChannelType == PATH_CONTROL_PROPERTY)
			{
				jeProperty_Data PropData;

				PathObject_GetPropertyInfoFromDataId(pPathObj->World, td->Obj, td->PropDataId, NULL, &PropData);

				switch (td->PropFieldType)
				{
                case PROPERTY_INT_TYPE:
                    ptr->ReturnXF.Translation.X = (float)PropData.Int;
                    break;
                case PROPERTY_FLOAT_TYPE:
                    ptr->ReturnXF.Translation.X = PropData.Float;
                    break;
                case PROPERTY_COLOR_GROUP_TYPE:
                case PROPERTY_VEC3D_GROUP_TYPE:
                    ptr->ReturnXF.Translation = PropData.Vector;
                    break;
                default:
                    // property not set
                    return JE_TRUE;
				}
			}
			
			break;
			}//case

		case OBJECT_TIMELINE_GET_PLAY_MODE_MSG: 
			{
			Object_TimePlayMode *ptr = (Object_TimePlayMode *)Data;
			assert(Data);
			ptr->PlayMode = pPathObj->PlayMotions;
			break;
			}

		case OBJECT_TIMELINE_SET_PLAY_MODE_MSG: // sets motion playing
			{
			Object_TimePlayMode *ptr = (Object_TimePlayMode *)Data;
			assert(Data);
			pPathObj->Time = 0;
			if (ptr->TimeLineNdx >= 0)
			{
				pPathObj->Time = pPathObj->TimeLineList[ptr->TimeLineNdx].CurrTime;
			}

			if (pPathObj->PlayMotions && ptr->PlayMode == JE_FALSE)
			{
				int i;
				// here we are stopping playback - set curr time to sample time
				for (i = 0; i < pPathObj->TimeLineCount; i++)
				{
					pPathObj->TimeLineList[i].CurrTime = pPathObj->TimeLineList[i].SampleTime;
				}
			}

			pPathObj->PlayMotions = (jeBoolean)ptr->PlayMode;

			if (pPathObj->PlayMotions)
				PathObject_BuildAllMotions(pPathObj);
			break;
			}

		case OBJECT_TIMELINE_GET_CURRENT_TIME_MSG: // sets motion playing
			{
			Object_TimeGetTime *ptr = (Object_TimeGetTime *)Data;
			if (ptr->TimeLineNdx >= 0)
				ptr->ReturnTime = pPathObj->TimeLineList[ptr->TimeLineNdx].SampleTime;
			else
				ptr->ReturnTime = pPathObj->Time;
			break;
			}

		}// switch

	return( JE_TRUE );
}


////////////////////////////////////////////////////////////////////////////////////////
//
//	UpdateTimeDelta()
//
///////////////////////////////////////////////////////////////////////////////////////
jeBoolean	JETCC UpdateTimeDelta(void * Instance, float TimeDelta )
{
	// locals
	PathObj	*pPathObj;

	// ensure valid data
	assert( Instance != NULL );

	if ( TimeDelta == 0.0f )
	{
		return JE_TRUE;
	}

	// get object
	pPathObj = (PathObj *)Instance;
	assert( pPathObj != NULL );

	if (pPathObj->PlayMotions)
	{
		int TimeLineNdx;

		pPathObj->Time += TimeDelta;

		for (TimeLineNdx = 0; TimeLineNdx < pPathObj->TimeLineCount; TimeLineNdx++)
		{
			pPathObj->TimeLineList[TimeLineNdx].LastTime = pPathObj->TimeLineList[TimeLineNdx].CurrTime;
			pPathObj->TimeLineList[TimeLineNdx].CurrTime = pPathObj->Time;
			PathObject_SampleMotion(pPathObj, TimeLineNdx);
		}
	}

	return JE_TRUE;
}

// Icestorm
jeBoolean	JETCC ChangeBoxCollision(const void *Instance,const jeVec3d *Pos, const jeExtBox *FrontBox, const jeExtBox *BackBox, jeExtBox *ImpactBox, jePlane *Plane)
{
	return( JE_FALSE );
}