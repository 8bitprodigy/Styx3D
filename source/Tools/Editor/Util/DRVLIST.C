/****************************************************************************************/
/*  DRVLIST.C                                                                           */
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
#pragma warning(disable : 4201 4214 4115 4514 4505)
#include <windows.h>
#pragma warning(default : 4201 4214 4115)
#include	<stdio.h>
#include	<stdlib.h>
#include	<assert.h>

#include	"jet.h"

#include	"..\resource.h"
#include	"drvlist.h"

typedef	struct	DriverInfo
{
	jeEngine				*Engine;
	jeDriver * 				diDriver;
	jeDriver_Mode *			diMode;
	struct	DriverInfo *	diNext;
}	DriverInfo;

static	jeDriver *		PickedDriver;
static	jeDriver_Mode *	PickedMode;
static	jeDriver		*CurDriver;
static	jeDriver_Mode	*CurMode;


static	void	DestroyDriverList(DriverInfo *dlist)
{
	DriverInfo *	temp;

	while	(dlist)
	{
		temp = dlist->diNext;
		free(dlist);
		dlist = temp;
	}
}



////////////////////////////////////////////////////////////////////////////////////////
//
//	Drvlist_Build()
//
//	Build available video mode list.
//
////////////////////////////////////////////////////////////////////////////////////////
static DriverInfo * Drvlist_Build(
	jeEngine	*Engine,		// engine to use
	int			ModeFilter )	// how to choose modes
{

	// locals
	jeDriver_System	*DriverSystem;
	jeDriver		*Driver;
	jeDriver_Mode	*Mode;
	DriverInfo		*DriverList;
	int				Count;

	// get driver system
	DriverSystem = jeEngine_GetDriverSystem( Engine );
	if ( DriverSystem == NULL )
	{
		return NULL;
	}

	// reset driver list and driver count
	DriverList = NULL;
	Count = 0;

	// process all drivers
	Driver = jeDriver_SystemGetNextDriver( DriverSystem, NULL );
	while ( Driver != NULL )
	{

		// locals
		DriverInfo	*dinfo;
		const char	*DriverName;

		// get driver name
		jeDriver_GetName( Driver, &DriverName );

		// process all modes
		Mode = jeDriver_GetNextMode( Driver, NULL );
		while ( Mode != NULL )
		{

			// locals
			const char	*ModeName;
			int32		Width, Height;
			jeBoolean	ValidMode;

			// get mode name and dimensions
			jeDriver_ModeGetName( Mode, &ModeName );
			jeDriver_ModeGetWidthHeight( Mode, &Width, &Height );

			// cool devug infp
			OutputDebugString( DriverName );
			OutputDebugString( " : " );
			OutputDebugString( ModeName );
			OutputDebugString( "\r\n" );

			// reset valid mode flag
			ValidMode = JE_TRUE;

			// determine if its a valid mode
			if ( !( ModeFilter & DRVLIST_WINDOW ) )
			{
				if ( ( Width == -1 ) && ( Height == -1 ) )
				{
					ValidMode = JE_FALSE;
				}
			}
			if ( !( ModeFilter & DRVLIST_FULLSCREEN ) )
			{
				if ( ( Width != -1 ) && ( Height != -1 ) )
				{
					ValidMode = JE_FALSE;
				}
			}
			/*if ( !( ModeFilter & DRVLIST_SOFTWARE ) )
			{
				if ( strnicmp( DriverName, "Software", 8 ) == 0 )
				{
					ValidMode = JE_FALSE;
				}
			}*/

			if ( !( ModeFilter & DRVLIST_HARDWARE ) )
			{
				if ( strnicmp( DriverName, "Direct3D 9 Driver", 8 ) != 0 )
				{
					ValidMode = JE_FALSE;
				}
			}

			if ( ModeFilter & DRVLIST_ALL )
			{
				ValidMode = JE_TRUE;
			}

			// add video mode to the list if required
			if ( ValidMode == JE_TRUE )
			{
				dinfo = malloc(sizeof(*dinfo));
				if	(!dinfo)
				{
					DestroyDriverList(DriverList);
					return NULL;
				}
				dinfo->diNext = DriverList;
								DriverList = dinfo;
				dinfo->diDriver = Driver;
				dinfo->diMode = Mode;
				dinfo->Engine = Engine;
				Count++;
			}

			// get next mode
			Mode = jeDriver_GetNextMode(Driver, Mode);
		}

		// get next driver
		Driver = jeDriver_SystemGetNextDriver(DriverSystem, Driver);
	}

	// if no available modes were found then just destroy the list
	if ( Count == 0 )
	{
		DestroyDriverList( DriverList );
		DriverList = NULL;
	}

	// all done
	return DriverList;

} // Drvlist_Build()



static void SetSelectedDriver (HWND hwndDlg, DriverInfo * DriverList)
{
	int		DriverIdx;

	if	(DriverList)
	{
		DriverInfo *	Temp;
		DriverIdx = SendDlgItemMessage(hwndDlg, IDC_DRIVERLIST, LB_GETCURSEL, (WPARAM)0, (LPARAM)0);
		Temp = DriverList;
		while	(DriverIdx--)
		{
			Temp = Temp->diNext;
			assert(Temp != NULL);
		}
		PickedDriver = Temp->diDriver;
		PickedMode = Temp->diMode;
	}
}

static	BOOL	CALLBACK	DlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static	DriverInfo *	DriverList;

	switch	(uMsg)
	{
	case	WM_INITDIALOG:
		DriverList = (DriverInfo *)lParam;
		if	(DriverList)
		{

			// locals
			DriverInfo *	temp;
			HWND			DriverListBox;
			HDC				hDC;
			int				MaxCX;
			int				Count;
			int				DefaultMode;

			DriverListBox = GetDlgItem(hwndDlg, IDC_DRIVERLIST);
			hDC = GetDC(DriverListBox);

			MaxCX = 0;

			// get driver list pointer
			temp = DriverList;

			// process all modes
			Count = 0;
			DefaultMode = 0;
			while	(temp)
			{

				// locals
				char			buff[256];
				SIZE			extents;
				const char *	DriverName;
				const char *	ModeName;
				
				// add this mode to the list
				jeDriver_GetName(temp->diDriver, &DriverName);
				jeDriver_ModeGetName(temp->diMode, &ModeName);
				sprintf(buff, "%s %s", DriverName, ModeName);
				SendDlgItemMessage(hwndDlg, IDC_DRIVERLIST, LB_ADDSTRING, 0, (LPARAM)buff);
				GetTextExtentPoint32(hDC, buff, strlen(buff), &extents);
				if	(extents.cx > MaxCX)
					MaxCX = extents.cx;
				temp = temp->diNext;

				// if this matches the current active mode then make it the default setting
				if ( ( CurDriver != NULL ) && ( CurMode != NULL ) )
				{

					// locals
					const char *	CurDriverName;
					const char *	CurModeName;
					char			CurBuff[256];

					// get current video mode name
					jeDriver_GetName( CurDriver, &CurDriverName );
					jeDriver_ModeGetName( CurMode, &CurModeName );
					sprintf( CurBuff, "%s %s", CurDriverName, CurModeName );

					// if its the currently active one then make it the default selection
					if ( stricmp( buff, CurBuff ) == 0 )
					{
						DefaultMode = Count;
					}
				}

				// adjust mode count
				Count++;
			}

			SendDlgItemMessage(hwndDlg, IDC_DRIVERLIST, LB_SETCURSEL, DefaultMode, 0);
			SendDlgItemMessage(hwndDlg, IDC_DRIVERLIST, LB_SETHORIZONTALEXTENT, MaxCX, 0);

			ReleaseDC(DriverListBox, hDC);

			return TRUE;
		}
		break;


	case	WM_COMMAND :
		switch (LOWORD (wParam))
		{
			case IDC_DRIVERLIST :
				if (HIWORD (wParam) != LBN_DBLCLK)
				{
					break;
				}
				// double click, so fall through to OK
			case IDOK :
			{
				SetSelectedDriver (hwndDlg, DriverList);
				EndDialog(hwndDlg, 1);
				break;
			}
	
			case IDCANCEL :
				EndDialog (hwndDlg, 0);
				break;

			default :
				break;
		}
	}

	return 0;
}



// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//	Name:			DrvList_GetDriverByName
//  Description:	Simple Funtion to get Driver and Mode via an Textstring 
//					Format of Textstring: driver,mode 
//					Example: (D3D)Primärer Anzeigetreiber,1024x768x8
//	Author:			JH 7.3.2000
// 
//	
/*
typedef	struct	DriverInfo
{
	jeEngine				*Engine;
	jeDriver * 				diDriver;
	jeDriver_Mode *			diMode;
	struct	DriverInfo *	diNext;
}	DriverInfo;
*/
jeBoolean DrvList_GetDriverByName(
	jeEngine		*Engine,
	char			*sDriverMode,
	jeDriver		**Driver,
	jeDriver_Mode	**Mode)
{
	DriverInfo		*DriverList;
	DriverInfo		*dlist;

	char			*sDriverString= NULL;
	char			*cModeString= NULL;

	const char		*cWindowDriver;
	const char		*cWindowMode;

	// Create DriverList
	DriverList = Drvlist_Build( Engine, DRVLIST_ALL );
	if ( DriverList == NULL )
	{
		return JE_FALSE;
	}
	

	sDriverString = strtok( sDriverMode, "," );
	if (sDriverString==NULL) 
		{
		  // Destroy DriverList
		  DestroyDriverList( DriverList );
		  DriverList = NULL;

		  return JE_FALSE;
		}


	cModeString = strtok( NULL, "," );
	if (cModeString==NULL) 
		{
		  // Destroy DriverList
		  DestroyDriverList( DriverList );
		  DriverList = NULL;

		  return JE_FALSE;
		}
	
	// Find Modus

	dlist=DriverList;
	while	(dlist)
	{
		jeDriver_GetName( dlist->diDriver, &cWindowDriver );
		jeDriver_ModeGetName( dlist->diMode, &cWindowMode );
		if (strcmp (cWindowDriver,sDriverString)==0)
			if (strcmp (cWindowMode,cModeString)==0)
			{	// Yep, found the right modus
				*Driver = dlist->diDriver;
				*Mode = dlist->diMode;

				DestroyDriverList( DriverList );
				DriverList = NULL;

				return JE_TRUE;
			}

		dlist = dlist->diNext;
	}	

	// Destroy DriverList
	DestroyDriverList( DriverList );
	DriverList = NULL;
	return JE_FALSE;
}

// EOF JH

jeBoolean DrvList_PickDriver(
	HANDLE			hInstance,
	HWND			hwndParent,
	jeEngine		*Engine,
	jeDriver		**Driver,
	jeDriver_Mode	**Mode,
	jeBoolean		DisplayDialog,
	int				ModeFilter )
{

	// locals
	DriverInfo	*DriverList;
	int			res;

	// zap picked video mode choices
	PickedDriver = NULL;
	PickedMode = NULL;
	CurDriver = *Driver;
	CurMode = *Mode;

	// build driver list
	DriverList = Drvlist_Build( Engine, ModeFilter );
	if ( DriverList == NULL )
	{
		MessageBox( hwndParent, "Unable to find any supported video modes", "jDesigner3D 2.1", MB_OK );
		return JE_FALSE;
	}

	// default to first available mode...
	if ( DisplayDialog == JE_FALSE )
	{
		PickedDriver = DriverList->diDriver;
		PickedMode = DriverList->diMode;
		res = 1;
	}
	// ...or display video mode dialog
	else
	{
		res = DialogBoxParam(hInstance,
							 MAKEINTRESOURCE(IDD_DRIVERDIALOG),
							 hwndParent,
							 DlgProc,
							 (LPARAM)DriverList);
	}

	// destroy driver list
	DestroyDriverList( DriverList );
	DriverList = NULL;

	// save video mode data if dialog box completed succesfully...
	if ( res == 1 )
	{
		*Driver = PickedDriver;
		*Mode = PickedMode;
	}
	// ...or fail if it didn't
	else
	{
		return JE_FALSE;
	}

	// all done
	return JE_TRUE;

} // DrvList_PickDriver()
