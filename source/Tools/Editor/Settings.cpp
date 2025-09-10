/****************************************************************************************/
/*  SETTINGS.CPP                                                                        */
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
// This module holds preference data that can be treated as "Global" but is
// actually held by CJweApp

/* Open Source Revision -----------------------------------------------------------------
 By: Dennis Tierney (DJT) dtierney@oneoverz.com
 On: 12/27/99 9:00:23 PM
 Comments: Added mouse properties settings support.
----------------------------------------------------------------------------------------*/
//	by trilobite jan. 2011
#include "stdafx.h"
//

#include <Assert.h>
#include <Windowsx.h>

//	by trilobite jan. 2011
//#include "stdafx.h"
//
#include "afx.h"
#include "jwe.h"
#include "Ram.h"
#include "Ram.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif



//---------------------------------------------------
// Added DJT
//---------------------------------------------------
// Registry keys
// WARNING: hardcoded values!!!
static char s_pszSettingsKey[] = "Settings";
static char s_pszMouseSettingsKey[] = "MouseSettings";

const	CString	m_strTextureGroupNamesKey = _T("Settings_TexGroupNames");
const	CString	m_strTextureGroupFolderKey = _T("Settings_TexGroups\\");

const	CString	m_strAutosaveKey = _T("Settings_Autosave");

//---------------------------------------------------
// End DJT
//---------------------------------------------------


typedef struct tagSettings
{
	COLORREF	coSelected ;
	COLORREF	coSubSelected ;
	COLORREF	coSelectedBk ;
	COLORREF	coTemplate ;
	COLORREF	coGridBackgroud ;
	COLORREF	coGrid ;
	COLORREF	coGridSnap ;
	COLORREF	coSubtractBrush ;
	COLORREF	coSubtractNoAssoc ;
	COLORREF	coAddBrush ;
	COLORREF	coSelectedFace ;
	COLORREF	coCutBrush ;
	COLORREF	coConstructorLine ;
	jeBoolean	bSelectFullyEncompassed ;
} Settings ;

                
Settings * Settings_Create( void )
{
	Settings * pSettings ;

	pSettings = JE_RAM_ALLOCATE_STRUCT( Settings ) ;
	if( pSettings == NULL )
		goto ADC_FAILURE ;

	memset( pSettings, 0, sizeof *pSettings ) ;

	pSettings->coSelected		= ((CJweApp*)AfxGetApp())->GetProfileInt( "Settings_GridColors", "coSelected"	,  RGB( 255, 0, 0 ) );
	pSettings->coSubSelected	= ((CJweApp*)AfxGetApp())->GetProfileInt( "Settings_GridColors", "coSubSelected"	,  RGB( 255, 0, 255));
	pSettings->coSelectedBk		= ((CJweApp*)AfxGetApp())->GetProfileInt( "Settings_GridColors", "coSelectedBk"		,  RGB( 0, 0, 0 ) );
	pSettings->coGridBackgroud	= ((CJweApp*)AfxGetApp())->GetProfileInt( "Settings_GridColors", "coGridBackgroud"	,  RGB( 128, 128, 128 ) );
	pSettings->coGrid			= ((CJweApp*)AfxGetApp())->GetProfileInt( "Settings_GridColors", "coGrid"			,  RGB( 100, 100, 100 ) );
	pSettings->coConstructorLine= ((CJweApp*)AfxGetApp())->GetProfileInt( "Settings_GridColors", "coConstructorLine",  RGB(0, 200, 200 ) );
	pSettings->coGridSnap		= ((CJweApp*)AfxGetApp())->GetProfileInt( "Settings_GridColors", "coGridSnap"		,  RGB( 0xc0, 0xc0, 0xc0  ) );
	pSettings->coSubtractBrush	= ((CJweApp*)AfxGetApp())->GetProfileInt( "Settings_GridColors", "coSubtractBrush"	,  RGB(  255, 0, 255) );
	pSettings->coAddBrush		= ((CJweApp*)AfxGetApp())->GetProfileInt( "Settings_GridColors", "coAddBrush"		,  RGB(  0, 0, 0) );
	pSettings->coSubtractNoAssoc= ((CJweApp*)AfxGetApp())->GetProfileInt( "Settings_GridColors", "coSubtractNoAssoc",  RGB( 0, 0, 0 ) );
	pSettings->coSelectedFace	= ((CJweApp*)AfxGetApp())->GetProfileInt( "Settings_GridColors", "coSelectedFace"	,  RGB( 255, 0, 255) );
	pSettings->coCutBrush		= ((CJweApp*)AfxGetApp())->GetProfileInt( "Settings_GridColors", "coCutBrush"		,  RGB( 255, 128, 64 ));

	pSettings->coTemplate		= ((CJweApp*)AfxGetApp())->GetProfileInt( "Settings_GridColors", "coTemplate"		,  RGB( 0, 255, 255 ) );

	return pSettings ;

ADC_FAILURE :
	if( pSettings != NULL )
		Settings_Destroy( &pSettings ) ;

	return NULL ;

}// Settings_Create

void Settings_Destroy( Settings ** ppSettings )
{
	Settings * pSettings ;
	assert( ppSettings != NULL ) ;
	pSettings = *ppSettings ;
	assert( pSettings != NULL ) ;

	jeRam_Free( *ppSettings ) ;

}// Settings_Destroy

// ACCESSORS

uint32 Settings_GetSelectedColor( void )
{
	return (uint32)((CJweApp*)AfxGetApp())->m_pSettings->coSelected ;
}// Settings_GetSelectedColor

uint32 Settings_GetSubSelectedColor( void )
{
	return (uint32)((CJweApp*)AfxGetApp())->m_pSettings->coSubSelected ;
}// Settings_GetSelectedColor


uint32 Settings_GetTemplateColor( void )
{
	return (uint32)((CJweApp*)AfxGetApp())->m_pSettings->coTemplate ;
}// Settings_GetTemplateColor

uint32 Settings_GetSelectedBk( void )
{
	return (uint32)((CJweApp*)AfxGetApp())->m_pSettings->coSelectedBk ;
}// Settings_GetSelectedBk

uint32 Settings_GetGridBk( void )
{
	return (uint32)((CJweApp*)AfxGetApp())->m_pSettings->coGridBackgroud ;
}// Settings_GetSelectedBk

uint32 Settings_GetGridColor( void )
{
	return (uint32)((CJweApp*)AfxGetApp())->m_pSettings->coGrid ;
}// Settings_GetGridColor

uint32 Settings_GetConstructorColor( void )
{
	return (uint32)((CJweApp*)AfxGetApp())->m_pSettings->coConstructorLine ;
}// Settings_GetConstructorColor

uint32 Settings_GetGridSnapColor( void )
{
	return (uint32)((CJweApp*)AfxGetApp())->m_pSettings->coGridSnap ;
}// Settings_GetGridSnapColor

uint32 Settings_GetSubtractBrushColor( void )
{
	return (uint32)((CJweApp*)AfxGetApp())->m_pSettings->coSubtractBrush ;
}// Settings_GetSubtractBrushColor

uint32 Settings_GetAddBrushColor( void )
{
	return (uint32)((CJweApp*)AfxGetApp())->m_pSettings->coAddBrush ;
}// Settings_GetAddBrushColor

uint32 Settings_GetAddSubtractEmptyColor( void )
{
	return (uint32)((CJweApp*)AfxGetApp())->m_pSettings->coSubtractNoAssoc ;
}// Settings_GetAddBrushColor

uint32 Settings_GetSelectedFaceColor( void )
{
	return (uint32)((CJweApp*)AfxGetApp())->m_pSettings->coSelectedFace ;
}// Settings_GetSelectedFaceColor

uint32 Settings_GetCutBrushColor( void )
{
	return (uint32)((CJweApp*)AfxGetApp())->m_pSettings->coCutBrush ;
}// Settings_GetSelectedFaceColor

jeBoolean Settings_IsSelByEncompass( void )
{
	//return ((CJweApp*)AfxGetApp())->m_pSettings->bSelectFullyEncompassed ;
	return (Settings_GetEdit_Selection());

}// Settings_IsSelByEncompass

//---------------------------------------------------
// Added DJT
//---------------------------------------------------
typedef struct tagMouseSettings
{
	eMouseMiddleButton       m_eMiddleButton;  // Middle button state
	eMouseWheel              m_eWheel;         // Mouse wheel state
	jeBoolean	             m_bHotSelect;     // Hot select flag
	eMouseRightButton        m_eRightButton;   // Right button state
} MouseSettings;



// Allocate data structure and set default values
MouseSettings * MouseSettings_Create(void)
{
	MouseSettings * pSettings ;
	
	pSettings = JE_RAM_ALLOCATE_STRUCT(MouseSettings) ;
	if(pSettings == NULL)
		goto ADC_FAILURE ;

	memset(pSettings, 0, sizeof *pSettings) ;

	// Set default values
	pSettings->m_eMiddleButton = mbDisabled;
	pSettings->m_eWheel        = mwDisabled;
	pSettings->m_bHotSelect    = JE_FALSE;
	return pSettings ;

ADC_FAILURE :
	if (pSettings != NULL)
		MouseSettings_Destroy(&pSettings);

	return NULL ;
}

// Release data structure memory
void MouseSettings_Destroy( MouseSettings ** ppSettings )
{
	MouseSettings * pSettings ;
	assert(ppSettings != NULL) ;
	pSettings = *ppSettings ;
	assert(pSettings != NULL) ;

	jeRam_Free(*ppSettings) ;

	// what's the pointer of passin a pointer to pointer
	// if you don't set the data pointer to NULL?
	*ppSettings = NULL;
}

// Save mouse setting to registry
jeBoolean MouseSettings_Save(void)
{
	MouseSettings *pSettings = ((CJweApp*)AfxGetApp())->m_pMouseSettings;
	assert(pSettings);

	// Save the mouse settings data to the registry
	return (jeBoolean)(AfxGetApp()->WriteProfileBinary(s_pszSettingsKey, 
	                                                   s_pszMouseSettingsKey, 
	                                                   (BYTE*)pSettings, 
	                                                   sizeof(MouseSettings)));
}

// Restore mouse setting from registry
jeBoolean MouseSettings_Restore(void) 

{ 
MouseSettings *pSettings = ((CJweApp*)AfxGetApp())->m_pMouseSettings; 
BYTE* buffer; 
assert(pSettings); 

unsigned int uiSize; // this is returned by GetProfileBinary not passed in. 
if (AfxGetApp()->GetProfileBinary(s_pszSettingsKey, 
    s_pszMouseSettingsKey, 
    (BYTE **)&buffer, 
                                                    &uiSize)) {    
        // Got key now copy to our memory and free allocated buffer 
        // This is dumb but this is how MFC GetProfileBinary Works 
        // It allocates a buffer and returns. it is not passed in as buffer 
        // uiSize is Sizeof buffer read 
        if (uiSize==sizeof(MouseSettings)) { 
			memcpy(pSettings, buffer, uiSize) ;    

            pSettings->m_eRightButton=(eMouseRightButton)Settings_GetMouse_RightBut();
            pSettings->m_eMiddleButton=(eMouseMiddleButton)Settings_GetMouse_MidBut();
			pSettings->m_eWheel=(eMouseWheel)Settings_GetMouse_Wheel();
			pSettings->m_bHotSelect=(eMouseWheel)Settings_GetMouse_HotSelect();
            delete [ ] buffer; 
            return JE_TRUE; 
        } 
        else { 
            // Size not right so return FALSE so we rewrite Key 
            delete [ ] buffer; 
            return JE_FALSE; 
        } 
    } 
else 
        return JE_FALSE; 
} 



// Get hot select setting
jeBoolean MouseSettings_GetHotSelect(void)
{
	return ((CJweApp*)AfxGetApp())->m_pMouseSettings->m_bHotSelect;
}
// Set hot select setting
void MouseSettings_SetHotSelect(jeBoolean bHotSelect)
{
	((CJweApp*)AfxGetApp())->m_pMouseSettings->m_bHotSelect = bHotSelect;
}

// Get mouse middle button state
eMouseMiddleButton MouseSettings_GetMiddleButtonState(void)
{
	return ((CJweApp*)AfxGetApp())->m_pMouseSettings->m_eMiddleButton;
}
// Set mouse middle button state
void MouseSettings_SetMiddleButtonState(eMouseMiddleButton eState)
{
	if (eState >= mbInvalid)
		return;
	((CJweApp*)AfxGetApp())->m_pMouseSettings->m_eMiddleButton = eState;
}

// Get mouse wheel state
eMouseWheel MouseSettings_GetWheelState(void)
{
	return ((CJweApp*)AfxGetApp())->m_pMouseSettings->m_eWheel;
}
// Set mouse wheel state
void MouseSettings_SetWheelState(eMouseWheel eState)
{
	if (eState >= mwInvalid)
		return;
	((CJweApp*)AfxGetApp())->m_pMouseSettings->m_eWheel = eState;
}
//---------------------------------------------------
// End DJT
//---------------------------------------------------


//---------------------------------------------------
// Added 30.01.2000 gaspode
//---------------------------------------------------
void Settings_SetSelectedColor( uint32 Color)
{
	 ((CJweApp*)AfxGetApp())->m_pSettings->coSelected = Color;
 	 ((CJweApp*)AfxGetApp())->WriteProfileInt( "Settings_GridColors", "coSelected",  Color);
}// Settings_SetSelectedColor

void Settings_SetSubSelectedColor( uint32 Color )
{
	 ((CJweApp*)AfxGetApp())->m_pSettings->coSubSelected = Color;
 	 ((CJweApp*)AfxGetApp())->WriteProfileInt( "Settings_GridColors", "coSubSelected",  Color);
}// Settings_SetSelectedColor


void Settings_SetTemplateColor( uint32 Color )
{
	 ((CJweApp*)AfxGetApp())->m_pSettings->coTemplate = Color;
 	 ((CJweApp*)AfxGetApp())->WriteProfileInt( "Settings_GridColors", "coTemplate",  Color);
}// Settings_SetTemplateColor

void Settings_SetSelectedBk( uint32 Color )
{
	 ((CJweApp*)AfxGetApp())->m_pSettings->coSelectedBk = Color;
 	 ((CJweApp*)AfxGetApp())->WriteProfileInt( "Settings_GridColors", "coSelectedBk",  Color);
}// Settings_SetSelectedBk

void Settings_SetGridBk( uint32 Color )
{
	 ((CJweApp*)AfxGetApp())->m_pSettings->coGridBackgroud = Color;
 	 ((CJweApp*)AfxGetApp())->WriteProfileInt( "Settings_GridColors", "coGridBackgroud",  Color);
}// Settings_SetSelectedBk

void Settings_SetGridColor( uint32 Color )
{
	 ((CJweApp*)AfxGetApp())->m_pSettings->coGrid = Color;
 	 ((CJweApp*)AfxGetApp())->WriteProfileInt( "Settings_GridColors", "coGrid",  Color);
}// Settings_SetGridColor

void Settings_SetConstructorColor( uint32 Color )
{
	 ((CJweApp*)AfxGetApp())->m_pSettings->coConstructorLine = Color;
 	 ((CJweApp*)AfxGetApp())->WriteProfileInt( "Settings_GridColors", "coConstructorLine",  Color);
}// Settings_SetConstructorColor

void Settings_SetGridSnapColor( uint32 Color )
{
	 ((CJweApp*)AfxGetApp())->m_pSettings->coGridSnap = Color;
 	 ((CJweApp*)AfxGetApp())->WriteProfileInt( "Settings_GridColors", "coGridSnap",  Color);
}// Settings_SetGridSnapColor

void Settings_SetSubtractBrushColor( uint32 Color )
{
	 ((CJweApp*)AfxGetApp())->m_pSettings->coSubtractBrush = Color;
 	 ((CJweApp*)AfxGetApp())->WriteProfileInt( "Settings_GridColors", "coSubtractBrush",  Color);
}// Settings_SetSubtractBrushColor

void Settings_SetAddBrushColor( uint32 Color )
{
	 ((CJweApp*)AfxGetApp())->m_pSettings->coAddBrush = Color;
 	 ((CJweApp*)AfxGetApp())->WriteProfileInt( "Settings_GridColors", "coAddBrush",  Color);
}// Settings_SetAddBrushColor

void Settings_SetAddSubtractEmptyColor( uint32 Color )
{
	 ((CJweApp*)AfxGetApp())->m_pSettings->coSubtractNoAssoc = Color;
 	 ((CJweApp*)AfxGetApp())->WriteProfileInt( "Settings_GridColors", "coSubtractNoAssoc",  Color);
}// Settings_SetAddBrushColor

void Settings_SetSelectedFaceColor( uint32 Color )
{
	 ((CJweApp*)AfxGetApp())->m_pSettings->coSelectedFace = Color;
 	 ((CJweApp*)AfxGetApp())->WriteProfileInt( "Settings_GridColors", "coSelectedFace",  Color);
}// Settings_SetSelectedFaceColor

void Settings_SetCutBrushColor( uint32 Color )
{
	 ((CJweApp*)AfxGetApp())->m_pSettings->coCutBrush = Color;
 	 ((CJweApp*)AfxGetApp())->WriteProfileInt( "Settings_GridColors", "coCutBrush",  Color);

}// Settings_SetSelectedFaceColor



//----------------------------------------------------------------
//  New for Preferences Dialog
//
// Save Settings	
//

void Settings_SetGrid_SnapDegrees(char *sPrefsString)
{
	((CJweApp*)AfxGetApp())->WriteProfileString( "Settings_Grid", "SnapDegrees",  sPrefsString);
} 
void Settings_SetGrid_VertexSnap(int iPrefsInt)
{
	 ((CJweApp*)AfxGetApp())->WriteProfileInt( "Settings_Grid", "VertexSnap",  iPrefsInt);
}

void Settings_SetGrid_SnapVertexManip(int iPrefsInt)
{
	 ((CJweApp*)AfxGetApp())->WriteProfileInt( "Settings_Grid", "SnapVertexManip",  iPrefsInt);
} 



void Settings_SetMouse_LeftBut(int iPrefsInt)
{
	((CJweApp*)AfxGetApp())->WriteProfileInt( "Settings_Mouse", "LeftBut",  iPrefsInt);
} 
void Settings_SetMouse_MidBut(int iPrefsInt)
{
	((CJweApp*)AfxGetApp())->WriteProfileInt( "Settings_Mouse", "MidBut",  iPrefsInt);
	((CJweApp*)AfxGetApp())->m_pMouseSettings->m_eMiddleButton=(eMouseMiddleButton)iPrefsInt;
} 
void Settings_SetMouse_RightBut(int iPrefsInt)
{
	((CJweApp*)AfxGetApp())->WriteProfileInt( "Settings_Mouse", "RightBut",  iPrefsInt);
	((CJweApp*)AfxGetApp())->m_pMouseSettings->m_eRightButton=(eMouseRightButton)iPrefsInt;
} 


void Settings_SetMouse_Wheel(int iPrefsInt)
{
	((CJweApp*)AfxGetApp())->WriteProfileInt( "Settings_Mouse", "Wheel",  iPrefsInt);
	((CJweApp*)AfxGetApp())->m_pMouseSettings->m_eWheel=(eMouseWheel)iPrefsInt;
} 

void Settings_SetMouse_HotSelect(int iPrefsInt)
{
	((CJweApp*)AfxGetApp())->WriteProfileInt( "Settings_Mouse", "HotSelect",  iPrefsInt);
	((CJweApp*)AfxGetApp())->m_pMouseSettings->m_bHotSelect = iPrefsInt;
} 



void Settings_SetJet_Coll(int iPrefsInt)
{
	 ((CJweApp*)AfxGetApp())->WriteProfileInt( "Settings_Jet", "Coll",  iPrefsInt);
} 
void Settings_SetJet_Grav(int iPrefsInt)
{
	 ((CJweApp*)AfxGetApp())->WriteProfileInt( "Settings_Jet", "Grav",  iPrefsInt);
} 
void Settings_SetJet_Slid(int iPrefsInt)
{
	 ((CJweApp*)AfxGetApp())->WriteProfileInt( "Settings_Jet", "Slid",  iPrefsInt);
} 
void Settings_SetJet_Stair(int iPrefsInt)
{
	 ((CJweApp*)AfxGetApp())->WriteProfileInt( "Settings_Jet", "Stair",  iPrefsInt);
} 
void Settings_SetJet_Height(char *sPrefsString)
{
	((CJweApp*)AfxGetApp())->WriteProfileString( "Settings_Jet", "Height",  sPrefsString);
} 
void Settings_SetJet_Window(char *sPrefsString)
{
	((CJweApp*)AfxGetApp())->WriteProfileString( "Settings_Jet", "Window",  sPrefsString);
} 
void Settings_SetJet_Fullscreen(char *sPrefsString)
{
	((CJweApp*)AfxGetApp())->WriteProfileString( "Settings_Jet", "Fullscreen",  sPrefsString);
} 



void Settings_SetPath_UBrush(char *sPrefsString)
{
	((CJweApp*)AfxGetApp())->WriteProfileString( "Settings_Path", "UBrush",  sPrefsString);
} 

void Settings_SetPath_Textures(char *sPrefsString)
{
	((CJweApp*)AfxGetApp())->WriteProfileString( "Settings_Path", "Material",  sPrefsString);
} 

// Added JH 3.3.2000
void Settings_SetView_ShowMousePos(int iPrefsInt)
{	  ((CJweApp*)AfxGetApp())->WriteProfileInt( "Settings_View", "ShowMousePos",  iPrefsInt);
}

void Settings_SetView_ShowSize(int iPrefsInt)		
{	 ((CJweApp*)AfxGetApp())->WriteProfileInt( "Settings_View", "ShowSize",  iPrefsInt);
}

void Settings_SetView_ShowRuler(int iPrefsInt)
{	((CJweApp*)AfxGetApp())->WriteProfileInt( "Settings_View", "ShowRuler",  iPrefsInt);
}

void Settings_SetView_CrossCursor(int iPrefsInt)
{	 ((CJweApp*)AfxGetApp())->WriteProfileInt( "Settings_View", "CrossCursor",  iPrefsInt);
}

// Added JH 6.3.2000
void Settings_SetGlobal_ToolbarText(int iPrefsInt)		
{	 ((CJweApp*)AfxGetApp())->WriteProfileInt( "Settings_Global", "ToolbarText",  iPrefsInt);
}

void Settings_SetGlobal_ToolbarFlat(int iPrefsInt)		
{	 ((CJweApp*)AfxGetApp())->WriteProfileInt( "Settings_Global", "ToolbarFlat",  iPrefsInt);
}

void Settings_SetView_PreviewView(int iPrefsInt)		
{	  ((CJweApp*)AfxGetApp())->WriteProfileInt("Settings_View", "PreviewView",  iPrefsInt);
}

void Settings_SetView_Nums(int iPrefsInt)		
{	  ((CJweApp*)AfxGetApp())->WriteProfileInt("Settings_View", "NumViews",  iPrefsInt);
}

// Added JH 11.3.2000
void Settings_SetPath_Shaders(char *sPrefsString)
{
	((CJweApp*)AfxGetApp())->WriteProfileString( "Settings_Path", "Shaders",  sPrefsString);
}

void Settings_SetGlobal_UndoBuffer(int iPrefsInt)		
{	  ((CJweApp*)AfxGetApp())->WriteProfileInt("Settings_Global", "UndoBuffer",  iPrefsInt);
}

void Settings_SetGlobal_BackupFile(int iPrefsInt)		
{	  ((CJweApp*)AfxGetApp())->WriteProfileInt("Settings_Global", "BackupFile",  iPrefsInt);
}

void Settings_SetGlobal_Thumbnail(int iPrefsInt)		
{	  ((CJweApp*)AfxGetApp())->WriteProfileInt("Settings_Global", "Thumbnail",  iPrefsInt);
}

// Added JH 24.3.2000

void Settings_SetJEdit_Version(int iPrefsInt)		
{	  ((CJweApp*)AfxGetApp())->WriteProfileInt( "Settings_JEdit", "Version", iPrefsInt );
}

void Settings_SetJEdit_ShowDisclaimer(int iPrefsInt)		
{	  ((CJweApp*)AfxGetApp())->WriteProfileInt( "Settings_JEdit", "Disclaimer", iPrefsInt );
}


// Added JH 30.3.2000

void Settings_SetEdit_Selection(int iPrefsInt)		
{	  ((CJweApp*)AfxGetApp())->WriteProfileInt( "Settings_Edit", "Selection", iPrefsInt );
}

// EOF_JH


//----------------------------------------------------------------
//  New for Preferences Dialog
//
// Read Settings	
//

char	* Settings_GetGrid_SnapDegrees(char *sRet, int istrlen)
{
	strncpy (sRet, (LPCSTR)  ((CJweApp*)AfxGetApp())->GetProfileString( "Settings_Grid", "SnapDegrees",  "15"),istrlen);
	return sRet;
} 

int Settings_GetGrid_VertexSnap()
{
	return ((CJweApp*)AfxGetApp())->GetProfileInt( "Settings_Grid", "VertexSnap",  8);
}

int Settings_GetGrid_SnapVertexManip()
{
	return ((CJweApp*)AfxGetApp())->GetProfileInt( "Settings_Grid", "SnapVertexManip",  1);
} 


int	Settings_GetMouse_LeftBut()
{
	return ((CJweApp*)AfxGetApp())->GetProfileInt( "Settings_Mouse", "LeftBut",  0);
} 
int	Settings_GetMouse_MidBut()
{
	return ((CJweApp*)AfxGetApp())->GetProfileInt( "Settings_Mouse", "MidBut",  0);
} 

int	Settings_GetMouse_RightBut()
{
	return ((CJweApp*)AfxGetApp())->GetProfileInt( "Settings_Mouse", "RightBut",  0);
} 

int	Settings_GetMouse_Wheel()
{
	return ((CJweApp*)AfxGetApp())->GetProfileInt( "Settings_Mouse", "Wheel",  0);
} 

int Settings_GetMouse_HotSelect()
{
	return ((CJweApp*)AfxGetApp())->GetProfileInt( "Settings_Mouse", "HotSelect",  1);
} 



int Settings_GetJet_Coll()
{
	 return ((CJweApp*)AfxGetApp())->GetProfileInt( "Settings_Jet", "Coll",  1);
} 

int Settings_GetJet_Grav()
{
	 return ((CJweApp*)AfxGetApp())->GetProfileInt( "Settings_Jet", "Grav",  1);
} 

int Settings_GetJet_Slid()
{
	 return ((CJweApp*)AfxGetApp())->GetProfileInt( "Settings_Jet", "Slid",  1);
} 
int Settings_GetJet_Stair()
{
	 return ((CJweApp*)AfxGetApp())->GetProfileInt( "Settings_Jet", "Stair",  1);
} 

char	* Settings_GetJet_Height(char *sRet, int istrlen)
{
	strncpy (sRet, (LPCSTR)  ((CJweApp*)AfxGetApp())->GetProfileString( "Settings_Jet", "Height",  "22"),istrlen);
	return sRet;
} 

char	* Settings_GetJet_Window(char *sRet, int istrlen)
{
	strncpy (sRet, (LPCSTR)  ((CJweApp*)AfxGetApp())->GetProfileString( "Settings_Jet", "Window",  "???"),istrlen);
	return sRet;
} 
char	* Settings_GetJet_Fullscreen(char *sRet, int istrlen)
{
	strncpy (sRet, (LPCSTR)  ((CJweApp*)AfxGetApp())->GetProfileString( "Settings_Jet", "Fullscreen",  "???"),istrlen);
	return sRet;
} 



char	* Settings_GetPath_UBrush(char *sRet, int istrlen)
{
	strncpy (sRet, (LPCSTR)   ((CJweApp*)AfxGetApp())->GetProfileString( "Settings_Path", "UBrush",  ".\\UserBrush"),istrlen);
	return sRet;
} 

char	* Settings_GetPath_Textures(char *sRet, int istrlen)
{
	strncpy (sRet, (LPCSTR)  ((CJweApp*)AfxGetApp())->GetProfileString( "Settings_Path", "Material",  ".\\GlobalMaterials"),istrlen);
	return sRet;
} 


long	Settings_GetKey(int Function)
{
	char	KeyName[200];
	sprintf (KeyName,"KeyToFunction%d",Function);
	return ((CJweApp*)AfxGetApp())->GetProfileInt( "Settings_Key", KeyName,  0);
}



void	Settings_SetKey(int Function, long lScanCode)
{
	char	KeyName[200];
	sprintf (KeyName,"KeyToFunction%d",Function);
	((CJweApp*)AfxGetApp())->WriteProfileInt( "Settings_Key", KeyName,  lScanCode);
}

// EOF_JH


// Added JH 3.3.2000
int Settings_GetView_ShowMousePos()
{	 return ((CJweApp*)AfxGetApp())->GetProfileInt( "Settings_View", "ShowMousePos",  0);
}

int Settings_GetView_ShowSize()		
{	 return ((CJweApp*)AfxGetApp())->GetProfileInt( "Settings_View", "ShowSize",  0);
}

int Settings_GetView_ShowRuler()
{	 return ((CJweApp*)AfxGetApp())->GetProfileInt( "Settings_View", "ShowRuler",  0);
}

int Settings_GetView_CrossCursor()
{	 return ((CJweApp*)AfxGetApp())->GetProfileInt( "Settings_View", "CrossCursor",  0);
}
// EOF_JH


// Added JH 6.3.2000
int Settings_GetGlobal_ToolbarFlat()		
{	 return ((CJweApp*)AfxGetApp())->GetProfileInt( "Settings_Global", "ToolbarFlat",  1);
}

int Settings_GetGlobal_ToolbarText()		
{	 return ((CJweApp*)AfxGetApp())->GetProfileInt( "Settings_Global", "ToolbarText",  1);
}

int Settings_GetView_PreviewView()		
{	 return ((CJweApp*)AfxGetApp())->GetProfileInt( "Settings_View", "PreviewView",  1);
}

int Settings_GetView_Nums()		
{	 return ((CJweApp*)AfxGetApp())->GetProfileInt( "Settings_View", "NumViews",  4);
}

// Added JH 11.3.2000


char	* Settings_GetPath_Shaders(char *sRet, int istrlen)
{
	strncpy (sRet, (LPCSTR)  ((CJweApp*)AfxGetApp())->GetProfileString( "Settings_Path", "Shaders",  ".\\Shaders"),istrlen);
	return sRet;
} 

int Settings_GetGlobal_UndoBuffer()		
{	 return ((CJweApp*)AfxGetApp())->GetProfileInt( "Settings_Global", "UndoBuffer",  10);
}

int Settings_GetGlobal_BackupFile()		
{	 return ((CJweApp*)AfxGetApp())->GetProfileInt( "Settings_Global", "BackupFile",  10);
}

int Settings_GetGlobal_Thumbnail()		
{	 return ((CJweApp*)AfxGetApp())->GetProfileInt( "Settings_Global", "Thumbnail",  1);
}

// Added JH 24.3.2000

int Settings_GetJEdit_Version()		
{	 return ((CJweApp*)AfxGetApp())->GetProfileInt( "Settings_JEdit", "Version",  0);
}

int Settings_GetJEdit_ShowDisclaimer()		
{	 return ((CJweApp*)AfxGetApp())->GetProfileInt( "Settings_JEdit", "Disclaimer",  1);
}

// Added JH 30.3.2000
// EOF_JH

int Settings_GetEdit_Selection ()
{	 return ((CJweApp*)AfxGetApp())->GetProfileInt( "Settings_Edit", "Selection",  0);
}

//	END JH



//	begin tom morris feb. 2005

bool	Settings_ResetTextureGroups()
{
	CString	strTextureGroupFolder = m_strTextureGroupFolderKey;
	strTextureGroupFolder.Left(strTextureGroupFolder.GetLength() -1);

	((CJweApp*)AfxGetApp())->DelRegTree(((CJweApp*)AfxGetApp())->GetAppRegistryKey(),m_strTextureGroupNamesKey);
	((CJweApp*)AfxGetApp())->DelRegTree(((CJweApp*)AfxGetApp())->GetAppRegistryKey(),strTextureGroupFolder);

	return true;
}


void	Settings_SetTexturesGroups(CStringList *pStrNewGroups)
{
	CString		strTemp, strIterator;
	CString		strNULL = _T("");
	int i = 0;

	POSITION	pos = NULL;
	pos = pStrNewGroups->GetHeadPosition();
	while (pos)
	{
		strIterator.Format("%d", i);
		strTemp = pStrNewGroups->GetNext(pos);
		((CJweApp*)AfxGetApp())->WriteProfileString(m_strTextureGroupNamesKey, strIterator, strTemp);

		i++;
	}
}


void	Settings_RestoreTexturesGroups(CStringList *pStringList)
{
	CStringList	strList;
	CString		strTemp, strIterator;
	CString		strNULL = _T("");
	int i = 0;
	strIterator.Format("%d", i);
	strTemp = ((CJweApp*)AfxGetApp())->GetProfileString(m_strTextureGroupNamesKey, strIterator);

	while (strTemp != strNULL)
	{
		pStringList->AddTail(strTemp);
		i ++;
		strIterator.Format("%d", i);
		strTemp = ((CJweApp*)AfxGetApp())->GetProfileString(m_strTextureGroupNamesKey, strIterator);
	}
}


void	Settings_SetTexturesInGroup(CString strGroupName, CStringList *pSringListTextureNames)
{
	CString		strPrefixxedGroupName = m_strTextureGroupFolderKey + strGroupName;
	CString		strTemp, strIterator;
	CString		strNULL = _T("");
	int i = 0;

	POSITION	pos = NULL;
	pos = pSringListTextureNames->GetHeadPosition();
	while (pos)
	{
		strIterator.Format("%d", i);
		strTemp = pSringListTextureNames->GetNext(pos);
		((CJweApp*)AfxGetApp())->WriteProfileString(strPrefixxedGroupName, strIterator, strTemp);

		i++;
	}
}


void	Settings_RestoreTexturesInGroup(CString strGroupName, CStringList *pSringListTextureNames)
{
	CString		strPrefixxedGroupName = m_strTextureGroupFolderKey + strGroupName;

	CString		strTemp, strIterator;
	CString		strNULL = _T("");
	int i = 0;
	strIterator.Format("%d", i);
	strTemp = ((CJweApp*)AfxGetApp())->GetProfileString(strPrefixxedGroupName, strIterator);

	while (strTemp != strNULL)
	{
		pSringListTextureNames->AddTail(strTemp);
		i ++;
		strIterator.Format("%d", i);
		strTemp = ((CJweApp*)AfxGetApp())->GetProfileString(strPrefixxedGroupName, strIterator);
	}
}


//	end tom morris feb 2005

//	tom morris may 2005
void	Settings_SetAutosaveMinutes(int iMiinutes)
{
	((CJweApp*)AfxGetApp())->WriteProfileInt(m_strAutosaveKey, "Minutes", iMiinutes);
}


int		Settings_GetAutosaveMinutes()
{
	return ((CJweApp*)AfxGetApp())->GetProfileInt(m_strAutosaveKey, "Minutes", 1);
}


void	Settings_SetAutosaveDirectory(CString strDir)
{
	((CJweApp*)AfxGetApp())->WriteProfileString(m_strAutosaveKey, "Directory", strDir);
}


LPCTSTR	Settings_GetAutosaveDirectory()
{
	return	((CJweApp*)AfxGetApp())->GetProfileString(m_strAutosaveKey, "Directory", "C:\\Temp");
}

// BEGIN - Disable auto save option - paradoxnj 8/11/2005
BOOL Settings_GetAutosaveDisabled()
{
	return (BOOL)((CJweApp*)AfxGetApp())->GetProfileInt(m_strAutosaveKey, "Disabled", 0);
}

void Settings_SetAutosaveDisabled(BOOL disabled)
{
	((CJweApp*)AfxGetApp())->WriteProfileInt(m_strAutosaveKey, "Disabled", disabled);
}
// END - Disable auto save option - paradoxnj 8/11/2005

/* EOF: Settings.cpp */