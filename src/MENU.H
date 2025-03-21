////////////////////////////////////////////////////////////////////////////////////////
//
//	Menu.h
//
//	Include for Menu.c
//
////////////////////////////////////////////////////////////////////////////////////////
#ifndef MENU_H
#define MENU_H

#include "MenuItem.h"


#ifdef __cplusplus
	extern "C" {
#endif


////////////////////////////////////////////////////////////////////////////////////////
//	Menu struct
////////////////////////////////////////////////////////////////////////////////////////
typedef struct	Menu_T	Menu_T;


////////////////////////////////////////////////////////////////////////////////////////
//	Prototypes
////////////////////////////////////////////////////////////////////////////////////////

//	Toggle a menu item.
//
////////////////////////////////////////////////////////////////////////////////////////
extern geBoolean Menu_ToggleItem(
	Menu_T		*Menu,			// menu to search through
	int32		Identifier );	// toggle to be toggled

//	Marks a string item for change or no change.
//
////////////////////////////////////////////////////////////////////////////////////////
extern geBoolean Menu_FlagString(
	Menu_T		*Menu,		// menu to search through
	int32		Identifier,	// string to be marked for change
	geBoolean	Flag );		// new setting

//	Marks a field for change.
//
////////////////////////////////////////////////////////////////////////////////////////
extern geBoolean Menu_FlagField(
	Menu_T		*Menu,		// menu to search through
	int32		Identifier,	// field to be marked for change
	geBoolean	Flag );		// new setting

//	Sets a string items data.
//
////////////////////////////////////////////////////////////////////////////////////////
extern geBoolean Menu_SetStringText(
	Menu_T	*Menu,			// menu to search through
	int32	Identifier,		// field whose text we want to set
	char	*NewText );		// text to set in string data

geBoolean Menu_GetStringText(
	Menu_T	*Menu,			// menu to search through
	int32	Identifier,		// field whose text we want to set
	char	*NewText );		// text to set in string data

//	Sets a fields text data.
//
////////////////////////////////////////////////////////////////////////////////////////
extern geBoolean Menu_SetFieldText(
	Menu_T	*Menu,			// menu to search through
	int32	Identifier,		// slider whose percentage we want
	char	*NewText );		// text to set in field text data

//	Gets a menus identifier.
//
////////////////////////////////////////////////////////////////////////////////////////
extern int32 Menu_GetIdentifier(
	Menu_T	*Menu );	// menu whose identifier we want

//	Get what percentage a slider is set at.
//
////////////////////////////////////////////////////////////////////////////////////////
extern float Menu_GetSliderPercent(
	Menu_T	*Menu,			// menu to search through
	int32	Identifier );	// slider whose percentage we want

//	Set what percentage
//
////////////////////////////////////////////////////////////////////////////////////////
extern float Menu_SetSliderPercent(
	Menu_T	*Menu,			// menu to search through
	int32	Identifier,
	float	Percent);

//	Change Slider Text
//
////////////////////////////////////////////////////////////////////////////////////////
extern float Menu_SetSliderText(
	Menu_T	*Menu,			// menu to search through
	int32	Identifier,
	char	*NewText);	

//	Get extra data associated with menu item 
//
////////////////////////////////////////////////////////////////////////////////////////
geBoolean Menu_GetCheckedData(
	Menu_T	*Menu,			// menu to search through
	int32	Identifier,
	int32  *UserData );	// store the data here


//	Create a new menu.
//
////////////////////////////////////////////////////////////////////////////////////////
extern Menu_T * Menu_Create(
	geEngine	*Engine,		// engine in which menu exists
	int32		Width,			// menu width
	int32		Top,			// top of menu
	int32		Bottom,			// bottom of menu (bottom-top = menu height)
	int32		x,				// x position
	int32		y,				// y position
	int32		Identifier );	// menu identifier

//	Destroy a menu.
//
////////////////////////////////////////////////////////////////////////////////////////
extern geBoolean Menu_Destroy(
	Menu_T	**Menu );	// menu to destroy

//	Add an item to a menu.
//
////////////////////////////////////////////////////////////////////////////////////////
extern geBoolean Menu_AddItem(
	Menu_T				*Menu,		// menu to add item to
	Menu_ItemType		ItemType,	// what type of item to add
	int32				Identifier,	// unique item identifier
	void				*Data );	// item data

//	Draw a new menu.
//
////////////////////////////////////////////////////////////////////////////////////////
extern geBoolean Menu_Draw(
	Menu_T	*Menu );	// menu to draw

//	Pass a keystroke to a menu.
//
////////////////////////////////////////////////////////////////////////////////////////
extern int32 Menu_Key(
	Menu_T	*Menu,	// menu to receive the keystroke
	int32	Key );	// input keystroke


#ifdef __cplusplus
	}
#endif

#endif
