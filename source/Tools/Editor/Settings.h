/****************************************************************************************/
/*  SETTINGS.H                                                                          */
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

/* Open Source Revision -----------------------------------------------------------------
 By: Dennis Tierney (DJT) dtierney@oneoverz.com
 On: 12/27/99 9:00:23 PM
 Comments: Added mouse properties settings support.
----------------------------------------------------------------------------------------*/

#pragma once

#ifndef SETTINGS_H
#define SETTINGS_H

typedef struct tagSettings Settings ;

//---------------------------------------------------
// Added DJT
//---------------------------------------------------
typedef struct tagMouseSettings MouseSettings ;

typedef enum {
	mbDisabled    = 0,
	mbSelectAll   = 1,
	mbSelectNone  = 2,
	mbInvalid     = 3
} eMouseMiddleButton;

typedef enum {
	mwDisabled  = 0,
	mwZoom      = 1,
	mwInvalid   = 2
} eMouseWheel;
//---------------------------------------------------
// End DJT
//---------------------------------------------------


//---------------------------------------------------
// Added JH 4.3.2000
//---------------------------------------------------
typedef enum {
	kb_SEPERATOR			= 0,
	kb_MOD_MOVESCALE ,
	kb_MOD_ROTSHARE,
	kb_MOD_VERTEXMAN,
	kb_ADD_CUBE   ,
	kb_ADD_CYLINDER   ,
	kb_ADD_SPHERE   ,
	kb_ADD_USERBRUSH   ,
	kb_ADD_LIGHT   ,
	kb_ADD_CAMERA   ,
	kb_ADD_OBJECT   ,
	kb_ANIMATE   ,
	kb_UPDATE_SELECTION   ,
	kb_UPDATE_ALL   ,
	kb_MISC_FULLSCREEN   ,

	kb_FILE_PREFS  ,			// Added JH 5.3.2000
	kb_EDIT_SELECTALL   ,
	kb_EDIT_SELECTNONE   ,
	kb_EDIT_SELECTINVERT   ,
	kb_VIEW_CENTERSELCTION,
	kb_OPTIONS_SNAPTOGRID   ,	// End JH

	kb_EDIT_BOTTOM,
	kb_EDIT_TOP,
	kb_EDIT_RIGHT,
	kb_EDIT_LEFT,
	kb_EDIT_ROTL,
	kb_EDIT_ROTR,

	kb_FILE_FILEPROPERTIES,
	kb_TOOLS_REBUILDALL,
}
eKeyBindingId;
#define KEYS_ACT_KEYMAPPINGS_DEFINED	29
//---------------------------------------------------
// End JH
//---------------------------------------------------



typedef enum {
	mbrDisabled    = 0,
	mbrPaning      = 1,
	mbrUserPopup   = 2,
	mbrInvalid     = 3	
}
eMouseRightButton;


#include "BaseType.h"

#ifdef __cplusplus
extern "C" {
#endif

Settings	*	Settings_Create( void ) ;
void			Settings_Destroy( Settings ** ppSettings ) ;

uint32			Settings_GetGridBk( void ) ;
uint32			Settings_GetGridColor( void ) ;
uint32			Settings_GetConstructorColor( void );
uint32			Settings_GetGridSnapColor( void ) ;
uint32			Settings_GetSelectedColor( void ) ;
uint32			Settings_GetSubSelectedColor( void ) ;
uint32			Settings_GetSelectedBk( void ) ;
uint32			Settings_GetSelectedFaceColor( void ) ;
uint32			Settings_GetTemplateColor( void ) ;
uint32			Settings_GetCutBrushColor( void ) ;

uint32			Settings_GetSubtractBrushColor( void ) ;
uint32			Settings_GetAddBrushColor( void ) ;
uint32			Settings_GetAddSubtractEmptyColor( void ) ;

jeBoolean		Settings_IsSelByEncompass( void ) ;

//---------------------------------------------------
// Added DJT
//---------------------------------------------------
MouseSettings *    MouseSettings_Create(void);  
void			   MouseSettings_Destroy(MouseSettings ** ppSettings);
jeBoolean          MouseSettings_Save(void);
jeBoolean          MouseSettings_Restore(void);
jeBoolean          MouseSettings_GetHotSelect(void);
void               MouseSettings_SetHotSelect(jeBoolean bHotSelect);
eMouseMiddleButton MouseSettings_GetMiddleButtonState(void);
void               MouseSettings_SetMiddleButtonState(eMouseMiddleButton eState);
eMouseWheel        MouseSettings_GetWheelState(void);
void               MouseSettings_SetWheelState(eMouseWheel eState);
//---------------------------------------------------
// End DJT
//---------------------------------------------------

//---------------------------------------------------
// Added 30.01.2000 gaspode
//---------------------------------------------------
void			Settings_SetGridBk( uint32 ) ;
void			Settings_SetGridColor( uint32 ) ;
void			Settings_SetConstructorColor( uint32 );
void			Settings_SetGridSnapColor( uint32 ) ;
void			Settings_SetSelectedColor( uint32 ) ;
void			Settings_SetSubSelectedColor( uint32 ) ;
void			Settings_SetSelectedBk( uint32 ) ;
void			Settings_SetSelectedFaceColor( uint32 ) ;
void			Settings_SetTemplateColor( uint32 ) ;
void			Settings_SetCutBrushColor( uint32 ) ;

void			Settings_SetSubtractBrushColor( uint32 ) ;
void			Settings_SetAddBrushColor( uint32 ) ;
void			Settings_SetAddSubtractEmptyColor( uint32 ) ;

//---------------------------------------------------
// Added 20.02.2000 JH
//---------------------------------------------------

void Settings_SetGrid_SnapDegrees(char *sPrefsString);
void Settings_SetGrid_VertexSnap(int iPrefsInt);
void Settings_SetGrid_SnapVertexManip(int iPrefsInt);

void Settings_SetMouse_LeftBut(int iPrefsInt);
void Settings_SetMouse_MidBut(int iPrefsInt);
void Settings_SetMouse_RightBut(int iPrefsInt);
void Settings_SetMouse_Wheel(int iPrefsInt);
void Settings_SetMouse_HotSelect(int iPrefsInt);

void Settings_SetJet_Coll(int iPrefsInt);
void Settings_SetJet_Grav(int iPrefsInt);
void Settings_SetJet_Slid(int iPrefsInt);
void Settings_SetJet_Stair(int iPrefsInt);
void Settings_SetJet_Height(char *sPrefsString);
void Settings_SetJet_Window(char *sPrefsString);
void Settings_SetJet_Fullscreen(char *sPrefsString);

void Settings_SetPath_UBrush(char *sPrefsString);
void Settings_SetPath_Textures(char *sPrefsString);

char	*Settings_GetGrid_SnapDegrees(char *sRet, int istrlen);
int		Settings_GetGrid_VertexSnap();
int		Settings_GetGrid_SnapVertexManip();

int		Settings_GetMouse_LeftBut();
int		Settings_GetMouse_MidBut();
int		Settings_GetMouse_RightBut();
int		Settings_GetMouse_Wheel();
int		Settings_GetMouse_HotSelect();

int		Settings_GetJet_Coll();
int		Settings_GetJet_Grav();
int		Settings_GetJet_Slid();
int		Settings_GetJet_Stair();
char	* Settings_GetJet_Height(char *sRet, int istrlen);
char	* Settings_GetJet_Window(char *sRet, int istrlen);
char	* Settings_GetJet_Fullscreen(char *sRet, int istrlen);

char	* Settings_GetPath_UBrush(char *sRet, int istrlen);
char	* Settings_GetPath_Textures(char *sRet, int istrlen);

long	Settings_GetKey(int Function);
void	Settings_SetKey(int Function, long lScanCode);


//---------------------------------------------------
// EOF_JH
//---------------------------------------------------

//---------------------------------------------------
// Added 3.3.2000 JH
//---------------------------------------------------
void	Settings_SetView_ShowMousePos(int iPrefsInt);
void	Settings_SetView_ShowSize(int iPrefsInt);
void	Settings_SetView_ShowRuler(int iPrefsInt);
void	Settings_SetView_CrossCursor(int iPrefsInt);

int		Settings_GetView_ShowMousePos();
int		Settings_GetView_ShowSize();
int		Settings_GetView_ShowRuler();
int		Settings_GetView_CrossCursor();

//---------------------------------------------------
// Added 6.3.2000 JH
//---------------------------------------------------
int		Settings_GetGlobal_ToolbarText();		
int		Settings_GetGlobal_ToolbarFlat();

void		Settings_SetGlobal_ToolbarText(int iPrefsInt);		
void		Settings_SetGlobal_ToolbarFlat(int iPrefsInt);

int			Settings_GetView_PreviewView();
void		Settings_SetView_PreviewView(int iPrefsInt);

int			Settings_GetView_Nums();
void		Settings_SetView_Nums(int iPrefsInt);

//---------------------------------------------------
// Added 11.3.2000 JH
//---------------------------------------------------

char	* 	Settings_GetPath_Shaders(char *sRet, int istrlen);
void		Settings_SetPath_Shaders(char *sPrefsString);

int			Settings_GetGlobal_UndoBuffer();
void		Settings_SetGlobal_UndoBuffer(int iPrefsInt);

void		Settings_SetGlobal_BackupFile(int iPrefsInt);
int			Settings_GetGlobal_BackupFile();

void		Settings_SetGlobal_Thumbnail(int iPrefsInt);
int			Settings_GetGlobal_Thumbnail();

void		Settings_SetJEdit_Version(int iPrefsInt);
int			Settings_GetJEdit_Version();

void		Settings_SetJEdit_ShowDisclaimer(int iPrefsInt);
int			Settings_GetJEdit_ShowDisclaimer();

void		Settings_SetEdit_Selection(int iPrefsInt);
int			Settings_GetEdit_Selection();



//---------------------------------------------------
// EOF_JH
//---------------------------------------------------


#ifdef __cplusplus

//	tom morris feb 2005
bool	Settings_ResetTextureGroups();
void	Settings_SetTexturesGroups(CStringList *pStrNewGroups); 
void	Settings_RestoreTexturesGroups(CStringList *pStringList);
void	Settings_SetTexturesInGroup(CString strGroupName, CStringList *pSringListTextureNames);
void	Settings_RestoreTexturesInGroup(CString strGroupName, CStringList *pSringListTextureNames);
//	end tom morris 2005

//	tom morris may 2005
void	Settings_SetAutosaveMinutes(int iMiinutes);
int		Settings_GetAutosaveMinutes();
void	Settings_SetAutosaveDirectory(CString strDir);
LPCTSTR	Settings_GetAutosaveDirectory();
//	end tom morris may 2005
// BEGIN - Disable auto save option - paradoxnj 8/11/2005
BOOL	Settings_GetAutosaveDisabled();
void	Settings_SetAutosaveDisabled(BOOL disabled);
// END - Disable auto save option - paradoxnj 8/11/2005
}
#endif

#endif // Prevent multiple inclusion
/* EOF: Settings.h */