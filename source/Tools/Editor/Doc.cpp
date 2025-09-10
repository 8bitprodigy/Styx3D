/****************************************************************************************/
/*  DOC.CPP                                                                             */
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
 On: 12/27/99 7:21:25 PM
 Comments:  1) New menu items. Selection options, Mouse Properties, etc.
            2) Menu handlers
            3) SelectAll() - Select all in level, with optional mask.
----------------------------------------------------------------------------------------*/

#include "stdafx.h"
#include <Float.h>
#include <assert.h>

#include "Resource.h"

#include "Draw.h"
#include "Draw3d.h"
#include "Jet.h"
#include "JetView.h"
#include "jeWorld.h"
#include "jwe.h"
#include "MainFrm.h"
#include "Rect.h"
#include "Transform.h"
#include "Util.h"
#include "View.h"
#include "Stats.h"
#include "rebuild.h"
#include "jePtrMgr.h"
#include "ErrorLog.h"
#include "ram.h"
#include "units.h"

#include "MfcUtil.h"

#include "Preferences.h"

#include "Doc.h"
#include "ReportErr.h"
#include "DrawTool.h"
#include "ExtFileDialog.h"

#include "jeBSP.h" // for RenderMode value
#include ".\doc.h"


#define SIGNATURE			'DOCM'
#define DOC_VERSION			0.2f
#define DOC_OLDVERSION		0.1f

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
// CJweDoc

IMPLEMENT_DYNCREATE(CJweDoc, CJ3DDoc)

BEGIN_MESSAGE_MAP(CJweDoc, CJ3DDoc)
	//{{AFX_MSG_MAP(CJweDoc)
	ON_COMMAND(IDM_TOOLS_PLACECUBE, OnToolsPlacecube)
	ON_UPDATE_COMMAND_UI(IDM_TOOLS_PLACECUBE, OnUpdateToolsPlacecube)
	ON_COMMAND(IDM_TOOLS_PLACESHEET, OnToolsPlacesheet)
	ON_UPDATE_COMMAND_UI(IDM_TOOLS_PLACESHEET, OnUpdateToolsPlacesheet)
	ON_COMMAND(IDM_VIEW_SHOWALLGROUPS, OnViewShowallgroups)
	ON_UPDATE_COMMAND_UI(IDM_VIEW_SHOWALLGROUPS, OnUpdateViewShowallgroups)
	ON_COMMAND(IDM_VIEW_SHOWVISIBLEGROUPS, OnViewShowvisiblegroups)
	ON_UPDATE_COMMAND_UI(IDM_VIEW_SHOWVISIBLEGROUPS, OnUpdateViewShowvisiblegroups)
	ON_COMMAND(IDM_VIEW_SHOW_CURRENTGROUP, OnViewShowCurrentgroup)
	ON_UPDATE_COMMAND_UI(IDM_VIEW_SHOW_CURRENTGROUP, OnUpdateViewShowCurrentgroup)
	ON_COMMAND(IDM_EDIT_ADDTOGROUP, OnEditAddtogroup)
	ON_UPDATE_COMMAND_UI(IDM_EDIT_ADDTOGROUP, OnUpdateEditAddtogroup)
	ON_COMMAND(IDM_EDIT_REMOVEFROMGROUP, OnEditRemovefromgroup)
	ON_UPDATE_COMMAND_UI(IDM_EDIT_REMOVEFROMGROUP, OnUpdateEditRemovefromgroup)
	ON_COMMAND(IDM_TOOLS_REBUILDALL, OnToolsRebuildall)
	ON_UPDATE_COMMAND_UI(IDM_TOOLS_REBUILDALL, OnUpdateToolsRebuildall)
	ON_COMMAND(IDM_MODE_ADJUST, OnModeAdjust)
	ON_UPDATE_COMMAND_UI(IDM_MODE_ADJUST, OnUpdateModeAdjust)
	ON_COMMAND(IDM_MODE_ROTATESHEAR, OnModeRotateshear)
	ON_UPDATE_COMMAND_UI(IDM_MODE_ROTATESHEAR, OnUpdateModeRotateshear)
	ON_COMMAND(IDM_OPTIONS_SNAPTOGRID, OnOptionsSnaptogrid)
	ON_UPDATE_COMMAND_UI(IDM_OPTIONS_SNAPTOGRID, OnUpdateOptionsSnaptogrid)
	ON_COMMAND(ID_EDIT_UNDO, OnEditUndo)
	ON_UPDATE_COMMAND_UI(ID_EDIT_UNDO, OnUpdateEditUndo)
	ON_COMMAND(ID_EDIT_CLEAR, OnEditClear)
	ON_UPDATE_COMMAND_UI(ID_EDIT_CLEAR, OnUpdateEditClear)
	ON_COMMAND(IDM_MODE_FACEMANIPULATION, OnModeFacemanipulation)
	ON_UPDATE_COMMAND_UI(IDM_MODE_FACEMANIPULATION, OnUpdateModeFacemanipulation)
	ON_COMMAND(IDM_TOOLS_NEXTFACE, OnToolsNextface)
	ON_UPDATE_COMMAND_UI(IDM_TOOLS_NEXTFACE, OnUpdateToolsNextface)
	ON_COMMAND(IDM_TOOLS_PREVFACE, OnToolsPrevface)
	ON_UPDATE_COMMAND_UI(IDM_TOOLS_PREVFACE, OnUpdateToolsPrevface)
	ON_COMMAND(IDM_TOOLS_BUILDLIGHTS, OnToolsBuildlights)
	ON_COMMAND(IDM_TOOLS_PLACECYLINDER, OnToolsPlacecylinder)
	ON_UPDATE_COMMAND_UI(IDM_TOOLS_PLACECYLINDER, OnUpdateToolsPlacecylinder)
	ON_COMMAND(IDM_TOOLS_PLACESPHEROID, OnToolsPlacespheroid)
	ON_UPDATE_COMMAND_UI(IDM_TOOLS_PLACESPHEROID, OnUpdateToolsPlacespheroid)
	ON_COMMAND(IDM_TOOLS_PLACELIGHT, OnToolsPlacelight)
	ON_UPDATE_COMMAND_UI(IDM_TOOLS_PLACELIGHT, OnUpdateToolsPlacelight)
	ON_COMMAND(IDM_TOOLS_PLACECAMERA, OnToolsPlacecamera)
	ON_COMMAND(IDM_TOOLS_PLACEUSEROBJ, OnToolsPlaceuserobj)
	ON_COMMAND(IDM_FULLSCREEN_VIEW, OnFullscreenView)
	ON_COMMAND(IDM_VIDEOSETTINGS_WINDOWMODE, OnVideosettingsWindowmode)
	ON_COMMAND(IDM_VIDEOSETTINGS_FULLSCREENMODE, OnVideosettingsFullscreenmode)
	ON_UPDATE_COMMAND_UI(IDM_EDIT_SELECTALL, OnUpdateEditSelectAll)
	ON_COMMAND(IDM_EDIT_SELECTALL, OnEditSelectAll)
	ON_UPDATE_COMMAND_UI(IDM_EDIT_SELECTNONE, OnUpdateEditSelectNone)
	ON_COMMAND(IDM_EDIT_SELECTNONE, OnEditSelectNone)
	ON_UPDATE_COMMAND_UI(IDM_EDIT_SELECTINVERT, OnUpdateEditSelectInvert)
	ON_COMMAND(IDM_EDIT_SELECTINVERT, OnEditSelectInvert)
	ON_UPDATE_COMMAND_UI(IDM_EDIT_SELECTBRUSHES, OnUpdateEditSelectType)
	ON_COMMAND(IDM_EDIT_SELECTBRUSHES, OnEditSelectBrushes)
	ON_COMMAND(IDM_EDIT_SELECTCAMERAS, OnEditSelectCameras)
	ON_COMMAND(IDM_EDIT_SELECTENTITIES, OnEditSelectEntities)
	ON_COMMAND(IDM_EDIT_SELECTLIGHTS, OnEditSelectLights)
	ON_COMMAND(IDM_EDIT_SELECTMODELS, OnEditSelectModels)
	ON_COMMAND(IDM_EDIT_SELECTTERRAIN, OnEditSelectTerrain)
	ON_COMMAND(IDM_EDIT_SELECTUSER, OnEditSelectUser)
	ON_COMMAND(ID_FILE_IMPORT_JTAASCIIFIE,OnImportBrush )
	ON_UPDATE_COMMAND_UI(ID_FILE_IMPORT_JTAASCIIFIE,OnUpdateImportBrush )
	ON_COMMAND(ID_FILE_EXPORT_SELECTEDOBJECTSASASCIIFILEJTA, OnExportBrush)
	ON_UPDATE_COMMAND_UI(ID_FILE_EXPORT_SELECTEDOBJECTSASASCIIFILEJTA, OnUpdateExportBrush)
	ON_COMMAND(IDM_FILE_PREFS, OnPreferences)
	ON_UPDATE_COMMAND_UI(IDM_FILE_PREFS, OnUpdatePreferences)
	ON_COMMAND(IDM_ANIM, OnAnim)
	ON_UPDATE_COMMAND_UI(IDM_ANIM, OnUpdateAnim)
	ON_COMMAND(IDM_FULLSCREEN, OnFullscreen)
	ON_UPDATE_COMMAND_UI(IDM_FULLSCREEN, OnUpdateFullscreen)
	ON_COMMAND(IDS_UPDATE_ALL, OnUpdateAll)
	ON_UPDATE_COMMAND_UI(IDS_UPDATE_ALL, OnUpdateUpdateAll)
	ON_COMMAND(IDM_TOOLS_UPDATE_SELECTION, OnToolsUpdateSelection)
	ON_UPDATE_COMMAND_UI(IDM_TOOLS_UPDATE_SELECTION, OnUpdateToolsUpdateSelection)
	ON_COMMAND(IDM_EDIT_ALIGN_LEFT, OnEditAlignLeft)
	ON_UPDATE_COMMAND_UI(IDM_EDIT_ALIGN_LEFT, OnUpdateEditAlignLeft)
	ON_COMMAND(IDM_EDIT_ALIGN_RIGHT, OnEditAlignRight)
	ON_UPDATE_COMMAND_UI(IDM_EDIT_ALIGN_RIGHT, OnUpdateEditAlignRight)
	ON_COMMAND(IDM_EDIT_ALIGN_BOTTOM, OnEditAlignBottom)
	ON_UPDATE_COMMAND_UI(IDM_EDIT_ALIGN_BOTTOM, OnUpdateEditAlignBottom)
	ON_COMMAND(IDM_EDIT_ALIGN_TOP, OnEditAlignTop)
	ON_UPDATE_COMMAND_UI(IDM_EDIT_ALIGN_TOP, OnUpdateEditAlignTop)
	ON_COMMAND (ID_FILE_FILEPROPERTIES, OnFileProps)
	ON_UPDATE_COMMAND_UI(ID_FILE_FILEPROPERTIES, OnUpdateFileProps)
	ON_COMMAND(IDM_EDIT_ROTL, OnEditRotL)
	ON_UPDATE_COMMAND_UI(IDM_EDIT_ROTL, OnUpdateEditRotL)
	ON_COMMAND(IDM_EDIT_ROTR, OnEditRotR)
	ON_UPDATE_COMMAND_UI(IDM_EDIT_ROTR, OnUpdateEditRotR)
	ON_COMMAND(IDM_EDIT_TOFRONT, OnEditToFront)
	ON_UPDATE_COMMAND_UI(IDM_EDIT_TOFRONT, OnUpdateEditToFront)
	ON_COMMAND(IDM_MODE_VERTEX, OnModeVertex)
	ON_UPDATE_COMMAND_UI(IDM_MODE_VERTEX, OnUpdateModeVertex)
	ON_COMMAND(IDM_EXPORT_PREFAB, OnExportPrefab)
	ON_UPDATE_COMMAND_UI(IDM_EXPORT_PREFAB, OnUpdateExportPrefab)
	ON_COMMAND(IDM_IMPORT_PREFAB, OnImportPrefab)
	ON_UPDATE_COMMAND_UI(IDM_EDIT_SELECTCAMERAS, OnUpdateEditSelectType)
	ON_UPDATE_COMMAND_UI(IDM_EDIT_SELECTENTITIES, OnUpdateEditSelectType)
	ON_UPDATE_COMMAND_UI(IDM_EDIT_SELECTLIGHTS, OnUpdateEditSelectType)
	ON_UPDATE_COMMAND_UI(IDM_EDIT_SELECTMODELS, OnUpdateEditSelectType)
	ON_UPDATE_COMMAND_UI(IDM_EDIT_SELECTTERRAIN, OnUpdateEditSelectType)
	ON_UPDATE_COMMAND_UI(IDM_EDIT_SELECTUSER, OnUpdateEditSelectType)
	ON_COMMAND(ID_FILE_EXPORT_EXPORTFORBTPROJECTWORKSPACEBTW, OnFileExportExportforbtprojectworkspacebtw)
	//}}AFX_MSG_MAP
	ON_COMMAND(IDM_TOOLS_PLACEARCH, OnToolsPlacearch)
	ON_COMMAND(IDM_VIEW_HIDE_CURRENTGROUP, OnViewHideCurrentgroup)
	ON_UPDATE_COMMAND_UI(IDM_VIEW_HIDE_CURRENTGROUP, OnUpdateViewHideCurrentgroup)
    ON_COMMAND(ID_FILE_CLOSE, OnFileClose)
    END_MESSAGE_MAP()

BEGIN_DISPATCH_MAP(CJweDoc, CJ3DDoc)
	//{{AFX_DISPATCH_MAP(CJweDoc)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//      DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_DISPATCH_MAP
END_DISPATCH_MAP()

// Note: we add support for IID_IGwe to support typesafe binding
//  from VBA.  This IID must match the GUID that is attached to the 
//  dispinterface in the .ODL file.

// {37F4562B-C0E1-11D2-8B41-00104B70D76D}
static const IID IID_IGwe =
{ 0x37f4562b, 0xc0e1, 0x11d2, { 0x8b, 0x41, 0x0, 0x10, 0x4b, 0x70, 0xd7, 0x6d } };

BEGIN_INTERFACE_MAP(CJweDoc, CJ3DDoc)
	INTERFACE_PART(CJweDoc, IID_IGwe, Dispatch)
END_INTERFACE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CJweDoc construction/destruction

CJweDoc::CJweDoc() : m_pLevel(nullptr), 
m_Mode(MODE_POINTER_BB), 
m_LastFOV( 2.0f ), 
m_bLoaded( JE_FALSE ), 
m_Anim_State(0)/*tom morris feb 2005*/,
m_strRebuild("Rebuild All to reveal actors...")/*end tom morris*/
{
	// TODO: add one-time construction code here

	CMainFrame* pMainFrm{};
	pMainFrm = (CMainFrame*)AfxGetMainWnd();

   LightBitmap = nullptr;
	pMainFrm->CloseCurDoc(  );

	//jeMemAllocInfo_Activate();	// Added by Icestorm: Use this for memory debugging

	EnableAutomation();
	RebuildDlg = new( CRebuild );
	m_pPropsDialog = new( CProperties );	// Added JH 16.3.2000
	m_pPrefsDialog = new (CPreferences);
	AfxOleLockApp();

   m_RenderMode = RenderMode_TexturedAndLit;
}

CJweDoc::~CJweDoc()
{
	jeBoolean Result{};

	if( LightBitmap )
#ifdef _USE_BITMAPS
		Result = jeBitmap_Destroy( &LightBitmap);
#else
		jeMaterialSpec_Destroy( &LightBitmap);
		Result = LightBitmap == nullptr;
#endif
	
	if( RebuildDlg != nullptr )
		delete RebuildDlg;
	if( m_pPropsDialog != nullptr )
		delete m_pPropsDialog;
	if (m_pPrefsDialog != nullptr)
		delete m_pPrefsDialog;

	//jeMemAllocInfo_DeActivate(JE_TRUE);		// Added by Icestorm: Use this for memory debugging

	AfxOleUnlockApp();
}

jeBitmap *	CJweDoc::InitBitmap( WORD Resource)
{
	// Jeff:  Load light bitmap from resources - 8/18/2005
	jeVFile* BmpFile{};
	jeBitmap * Bmp = nullptr;
	POSITION	pos{};
	CView* pView{};
	HRSRC hFRes{};
	HGLOBAL hRes{};
	HMODULE hModule{};
	jeVFile_MemoryContext Context{};

    hModule = GetModuleHandle (nullptr); 
    hFRes = FindResource(hModule, MAKEINTRESOURCE(Resource) ,"jeBitmap"); 
    hRes = LoadResource(hModule, hFRes) ;  
    
    Context.Data  = LockResource(hRes); 
    Context.DataLength = SizeofResource(hModule,hFRes); 

	BmpFile = jeVFile_OpenNewSystem(nullptr,JE_VFILE_TYPE_MEMORY,nullptr,
                            		&Context,JE_VFILE_OPEN_READONLY  );
	if( BmpFile != nullptr )
	{
        Bmp = jeBitmap_CreateFromFile( BmpFile );
		jeBitmap_SetColorKey( Bmp, JE_TRUE, 255, JE_TRUE );
		jeVFile_Close( BmpFile );
		pos = GetFirstViewPosition();
		while( pos != nullptr )
		{
			pView = GetNextView(pos);
			ASSERT_VALID(pView);
			if( pView->IsKindOf( RUNTIME_CLASS (CJetView)))
				if( !((CJetView*)pView)->RegisterBitmap( Bmp ) )
					return( nullptr );
		}
	}
	else
		jeErrorLog_AddString( JE_ERR_FILEIO_OPEN, "InitBitmap:jeVFile_OpenNewSystem", MAKEINTRESOURCE(Resource) );
	return( Bmp );
}

jeMaterialSpec * CJweDoc::InitMaterial( WORD Resource )
{
	jeMaterialSpec* pMat = nullptr;

	HRSRC hFRes{};
	HGLOBAL hRes{};
	HMODULE hModule{};

	jeVFile* BmpFile{};
	jeVFile_MemoryContext Context{};

    hModule = GetModuleHandle (nullptr); 
    hFRes = FindResource(hModule, MAKEINTRESOURCE(Resource) ,"jeBitmap"); 
    hRes = LoadResource(hModule, hFRes) ;  
    
    Context.Data  = LockResource(hRes); 
    Context.DataLength = SizeofResource(hModule,hFRes); 

	BmpFile = jeVFile_OpenNewSystem(nullptr,JE_VFILE_TYPE_MEMORY,nullptr,
                            		&Context,JE_VFILE_OPEN_READONLY  );

	if (BmpFile) {
		pMat = jeMaterialSpec_Create(GetJetEngine(), GetResourceMgr());
		jeMaterialSpec_AddLayerFromFile(pMat, 0, BmpFile, JE_TRUE, 255);
		jeVFile_Close( BmpFile );
	}
	return pMat;
}

BOOL CJweDoc::OnNewDocument()
{
	CJetView	*pJetView = (CJetView *)GetJetView();

	LightBitmap      = nullptr;
	CMainFrame	* pMainFrm{};
	pMainFrm = (CMainFrame*)AfxGetMainWnd();

	if (!CDocument::OnNewDocument())
		return FALSE;

	CJweApp		* pApp{};
	pApp = (CJweApp*)AfxGetApp();

	m_pResourceMgr = Level_CreateResourceMgr(pJetView->GetEngine());
	if( m_pResourceMgr == nullptr )
		return( FALSE );

	if (!pApp->HasInitMaterialList()) {
		pApp->InitMaterialList(pJetView->GetEngine(), m_pResourceMgr);
	}

	m_pWorld = jeWorld_Create(m_pResourceMgr) ;
	if( m_pWorld == nullptr )
	{
		TRACE0("World Create Failed\n") ;
		return FALSE ;
	}

	//Set Invalid
	SetNewBrushBoundInvalid();

	if( !CreateLevel() )
		return( FALSE );
	
	// initilise the bmp to signal some particular element
#ifdef _USE_BITMAPS
	LightBitmap	= InitBitmap( IDR_LIGHT );
#else
	LightBitmap	= InitMaterial( IDR_LIGHT );
#endif
	if( LightBitmap == nullptr )
	{
		ReportErrors( IDR_LIGHT );
		TRACE0("World Create Failed\n") ;
		return FALSE ;
	}
		
	jeWorld_AttachSoundSystem( m_pWorld, pMainFrm->GetSoundSystem() );
	
	m_bLoaded = JE_TRUE;

	// Added JH: Bad Place to put, but i'm seaching a better one :)
	char sTempString[200];
	Level_SetShouldSnapVerts( m_pLevel, Settings_GetGrid_SnapVertexManip() ) ;
	Level_SetGridSnapSize( m_pLevel, Settings_GetGrid_VertexSnap() ) ;
	Level_SetRotateSnapSize( m_pLevel, atoi(Settings_GetGrid_SnapDegrees(sTempString,199)) ) ;
	pMainFrm->SetAccelerator();			
	// EOF 

	return TRUE;
}// OnNewDocument

void CJweDoc::SetNewBrushBoundInvalid()
{
	m_NewBrushBounds.Min.X = 1.0f;
	m_NewBrushBounds.Min.Y = 1.0f;
	m_NewBrushBounds.Min.Z = 1.0f;
	m_NewBrushBounds.Max.X = -1.0f;
	m_NewBrushBounds.Max.Y = -1.0f;
	m_NewBrushBounds.Max.Z = -1.0f;
}

void CJweDoc::SetNewBrushBound( Ortho * pOrtho, Point * pMousePt, Point *pAnchor )
{
	jeExtBox BrushBounds{};
	jeVec3d MouseVec{};
	jeVec3d AnchorVec{};
	int Index{};


	if( jeExtBox_IsValid( &m_NewBrushBounds ) )
		UpdateAllViews( nullptr, DOC_HINT_ORTHO, (CObject*)&m_NewBrushBounds );

	Ortho_ViewToWorld( pOrtho, pAnchor->X, pAnchor->Y, &AnchorVec );
	Ortho_ViewToWorld( pOrtho, pMousePt->X, pMousePt->Y, &MouseVec );
	if( Level_IsSnapGrid( m_pLevel ) )
	{
		Transform_PointToGrid( m_pLevel, &AnchorVec, &AnchorVec ) ;
		Transform_PointToGrid( m_pLevel, &MouseVec, &MouseVec ) ;
	}
	Index = Ortho_GetVerticalAxis( pOrtho );
	if( jeVec3d_GetElement( &MouseVec, Index ) == jeVec3d_GetElement( &AnchorVec, Index ))
		return;
	Index = Ortho_GetHorizontalAxis( pOrtho );
	if( jeVec3d_GetElement( &MouseVec, Index ) == jeVec3d_GetElement( &AnchorVec, Index ))
		return;
	jeExtBox_Set( &BrushBounds, MouseVec.X, MouseVec.Y, MouseVec.Z,
								AnchorVec.X, AnchorVec.Y, AnchorVec.Z );
	m_NewBrushBounds = BrushBounds;
	Index = Ortho_GetOrthogonalAxis( pOrtho );
	jeVec3d_SetElement( &m_NewBrushBounds.Min, Index, Level_GetConstructorPlane( m_pLevel, Index ) );
	if( Level_IsSnapGrid( m_pLevel ) )
		jeVec3d_SetElement( &m_NewBrushBounds.Max, Index, Level_GetConstructorPlane( m_pLevel, Index ) + Level_GetGridSnapSize(  m_pLevel ));
	else
		jeVec3d_SetElement( &m_NewBrushBounds.Max, Index, Level_GetConstructorPlane( m_pLevel, Index ) + 1);

	if( jeExtBox_IsValid( &m_NewBrushBounds ) )
		UpdateAllViews( nullptr, DOC_HINT_ORTHO, (CObject*)&m_NewBrushBounds );
}

void CJweDoc::SetNewBrushHeight( Ortho * pOrtho, Point * pMousePt, Point *pAnchor )
{
	float NewHeight{};
	float Plane{};
	int Index{};
	int VIndex{};
	float Height{};
	jeVec3d MouseVec{};
	jeVec3d AnchorVec{};

	if( jeExtBox_IsValid( &m_NewBrushBounds ) )
		UpdateAllViews( nullptr, DOC_HINT_ORTHO, (CObject*)&m_NewBrushBounds );

	Ortho_ViewToWorld( pOrtho, pAnchor->X, pAnchor->Y, &AnchorVec );
	Ortho_ViewToWorld( pOrtho, pMousePt->X, pMousePt->Y, &MouseVec );
	if( Level_IsSnapGrid( m_pLevel ) )
	{
		Transform_PointToGrid( m_pLevel, &AnchorVec, &AnchorVec ) ;
		Transform_PointToGrid( m_pLevel, &MouseVec, &MouseVec ) ;
	}

	Index = Ortho_GetOrthogonalAxis(pOrtho );
	VIndex = Ortho_GetVerticalAxis(pOrtho );
	
	Height = jeVec3d_GetElement( &AnchorVec, VIndex ) -jeVec3d_GetElement( &MouseVec, VIndex );
	Plane  = Level_GetConstructorPlane( m_pLevel, Index );

	if( jeVec3d_GetElement( &m_NewBrushBounds.Min, Index ) == Plane )
	{
		NewHeight = jeVec3d_GetElement( &m_NewBrushBounds.Min, Index ) + Height;
		
		if( NewHeight < Plane )
		{
			jeVec3d_SetElement( &m_NewBrushBounds.Max, Index, Plane );
			jeVec3d_SetElement( &m_NewBrushBounds.Min, Index, NewHeight );
		}
		else
		if( NewHeight != Plane)
		{
			jeVec3d_SetElement( &m_NewBrushBounds.Max, Index, NewHeight );
		}
	}
	else
	{
		NewHeight = jeVec3d_GetElement( &m_NewBrushBounds.Max, Index ) + Height;
		if( NewHeight > Plane )
		{
			jeVec3d_SetElement( &m_NewBrushBounds.Min, Index, Plane );
			jeVec3d_SetElement( &m_NewBrushBounds.Max, Index, NewHeight );
		}
		else
		if( NewHeight != Plane)
		{
			jeVec3d_SetElement( &m_NewBrushBounds.Min, Index, NewHeight );
		}
	}


	if( jeExtBox_IsValid( &m_NewBrushBounds ) )
		UpdateAllViews( nullptr, DOC_HINT_ORTHO, (CObject*)&m_NewBrushBounds );

}

const jeExtBox * CJweDoc::GetNewBrushBounds()
{
	return( &m_NewBrushBounds );
}

BOOL CJweDoc::CreateLevel()
{
	jeProperty_List *pArray = nullptr;
//	tom morris feb 2005 -- to support setting bsp rebuild defaults
	jeBSP_Options		Options = 0;
	jeBSP_Logic			Logic = Logic_Smart;
	jeBSP_LogicBalance	LogicBalance = 3;
//	end tom morris feb 2005
	CJweApp		*App{};
	App = (CJweApp*)AfxGetApp();
	CMainFrame	*pMainFrm{};
	pMainFrm = (CMainFrame*)AfxGetMainWnd();
	
	if( m_pLevel == nullptr )	// Level is already created if opening a doc
	{
		m_pLevel = Level_Create( m_pWorld, App->GetMaterialList() ) ;
		if( m_pLevel == nullptr )
		{
			TRACE0("Level Create Failed\n") ;
			return false ;
		}
		pArray = Select_BuildDescriptor( m_pLevel );
		pMainFrm->SetProperties( pArray );
		jeProperty_ListDestroy( &pArray );
	}

//	tom morris feb 2005 -- necessary to ensure VIS areas are present
//	otherwise actors may not be visible.
//	Level_RebuildAll( m_pLevel, BSP_OPTIONS_CSG_BRUSHES, Logic_Smart, 3 ) ;
	Options = BSP_OPTIONS_CSG_BRUSHES | BSP_OPTIONS_MAKE_VIS_AREAS;
	Level_SetBSPBuildOptions(m_pLevel, Options, Logic, LogicBalance);
	Level_RebuildAll( m_pLevel, Options, Logic, LogicBalance ) ;
//	end tom morris feb 2005
	return true ;

}// CreateLevel

/////////////////////////////////////////////////////////////////////////////
// CJweDoc serialization

void CJweDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: add storing code here
	}
	else
	{
		// TODO: add loading code here
	}
}

/////////////////////////////////////////////////////////////////////////////
// CJweDoc diagnostics

#ifdef _DEBUG
void CJweDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CJweDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CJweDoc commands

BOOL CJweDoc::RenderLights( jeCamera* pCamera )
{
	LightList	*pLightList{};
	Light		*pLight{};
	LightIterator	LI{};
	jeXForm3d		LightXForm{};
	jeLVertex		Vertex{};
	jeUserPoly	*Sprite{};
	jeFrustum		Frustum{};

	if( !LightBitmap )
		return( FALSE );
	pLightList = Level_GetLightList( m_pLevel ) ;
	pLight = LightList_GetFirst( pLightList, &LI );
	Vertex.r = 255.0f;
	Vertex.g = 255.0f;
	Vertex.b = 255.0f;
	Vertex.a = 255.0f;
	Vertex.u = 0.0f;
	Vertex.v = 0.0f;
	jeFrustum_SetFromCamera( &Frustum, pCamera );
	while( pLight )
	{
		Light_GetXForm( pLight, &LightXForm );
		Vertex.X = LightXForm.Translation.X;
		Vertex.Y = LightXForm.Translation.Y;
		Vertex.Z = LightXForm.Translation.Z;
		Sprite = jeUserPoly_CreateSprite( &Vertex, LightBitmap, 1.0f, 0 );
		if( Sprite != nullptr )
		{
			jeWorld_AddUserPoly(m_pWorld, Sprite, JE_TRUE );
			jeUserPoly_Destroy(&Sprite);
		}
		pLight = LightList_GetNext( pLightList, &LI );
	}
	return( TRUE );
}

typedef struct DrawFaceInfo_Struct {
	jeEngine* pEngine;
	jeBitmap* pBitmap;
} DrawFaceInfo_Struct;

void CJweDoc::DrawFaceCB(const jeTLVertex *Verts, int32 NumVerts, void *Context)
{
	jeTLVertex *ModVerts{};
	int i;
	DrawFaceInfo_Struct *pDrawFaceInfo = (DrawFaceInfo_Struct *)Context; 

	ModVerts = JE_RAM_ALLOCATE_ARRAY( jeTLVertex, NumVerts );
	if( ModVerts == nullptr )
		return;

	for( i = 0; i < NumVerts; i++ )
	{
		ModVerts[i] = Verts[i];
		ModVerts[i].z -= 10.0f;
		ModVerts[i].a = 70.0f;
		ModVerts[i].u = ModVerts[i].x * 0.0002f;
		ModVerts[i].v = ModVerts[i].y * 0.0002f;
	}
	//jeEngine_RenderPoly(pDrawFaceInfo->pEngine, ModVerts, 
	//					NumVerts, pDrawFaceInfo->pBitmap, JE_RENDER_FLAG_COLORKEY);
	jeEngine_RenderPoly(pDrawFaceInfo->pEngine, ModVerts, 
						NumVerts, nullptr, JE_RENDER_FLAG_ALPHA);
	jeRam_Free( ModVerts );
}

jeBoolean CJweDoc::SetModelFaceCB( Model *pModel, void * pVoid ) 
{
	if (pVoid)
		jeModel_SetBrushFaceCB( Model_GetguModel( pModel ), DrawFaceCB, pVoid );
	else
		jeModel_SetBrushFaceCB( Model_GetguModel( pModel ), nullptr, nullptr);

	return( JE_TRUE );
}


BOOL CJweDoc::SetDrawFaceCB(jeEngine *Engine, jeBoolean Enable)
{
	DrawFaceInfo_Struct DrawFaceInfo; 

	DrawFaceInfo.pEngine = Engine;
	
	if (Enable)
		Level_EnumModels( m_pLevel, &DrawFaceInfo, SetModelFaceCB );
	else
		Level_EnumModels( m_pLevel, nullptr, SetModelFaceCB );

	return TRUE;
}

BOOL CJweDoc::Render( class CJ3DView * pJ3DView )
{
	CJetView* pView{};
	jeEngine* pEngine{};
	jeCamera* pCamera{};
	DrawFaceInfo_Struct DrawFaceInfo{}; 
	jeXForm3d	CamXForm{};
	float FOV{};
	
	ASSERT(pJ3DView != nullptr);
	ASSERT(pJ3DView->GetDocument() == this);
	ASSERT(pJ3DView->IsKindOf(RUNTIME_CLASS(CJetView)));

	if( m_bLoaded == JE_FALSE )
		return( TRUE );

	if( m_pLevel == nullptr )
		return( TRUE );

	pView = (CJetView*)pJ3DView;

	pEngine = pView->GetEngine();
	ASSERT(pEngine != nullptr);

	pCamera = pView->GetCamera();
	if(pCamera == nullptr)
	{
		return(FALSE);
	}

	if( Level_GetCurCamXForm( m_pLevel, &CamXForm ) )
		jeCamera_SetXForm( pCamera, &CamXForm );
	if( Level_GetCurCamFOV( m_pLevel, &FOV ) )
	{
		if( FOV != m_LastFOV )
		{
			jeRect Rect;
			jeCamera_GetClippingRect( pCamera, &Rect );
			jeCamera_SetAttributes( pCamera, FOV, &Rect );
			m_LastFOV = FOV;
		}
	}
	DrawFaceInfo.pEngine = pEngine;
	Level_EnumModels( m_pLevel, &DrawFaceInfo, SetModelFaceCB );

	if (jeEngine_BeginFrame(pEngine, pCamera, JE_TRUE) == JE_FALSE)
	{
		return(FALSE);
	}

	Draw3d_ManipulatedBrushes( m_pLevel, m_pWorld, pCamera, pEngine ) ;
	RenderLights( pCamera );

	if(jeWorld_Render(m_pWorld, pCamera, nullptr) == JE_FALSE)
	{
		jeEngine_EndFrame(pEngine);
		return(FALSE);
	}
	
	//jeBrush_Render(pBrush, pEngine, pCamera);

	if(jeEngine_EndFrame(pEngine) == JE_FALSE)
	{
		return(FALSE);
	}

	return(TRUE);
}

void CJweDoc::DeleteContents() 
{
	CMainFrame *	pMainFrm = (CMainFrame*)AfxGetMainWnd() ;

	if( pMainFrm != nullptr )
	{
		pMainFrm->ResetLists();
		pMainFrm->ResetProperties();
	}
	if( m_pLevel != nullptr )
	{
		Level_Destroy( &m_pLevel ) ;
	}

	if( m_pWorld != nullptr)
	{
		jeWorld_Destroy(&m_pWorld);
		m_pWorld = nullptr;
	}

	CJ3DDoc::DeleteContents();
}// DeleteContents

//
// MENU HANDLING
//

void CJweDoc::OnToolsPlacecube() 
{

	if( m_Mode == MODE_POINTER_CUBE )
	{
		SetMode( m_PrevMode ) ;
	}
	else
	{
		SetMode( MODE_POINTER_CUBE ) ;
	}

	
}// OnToolsPlacecube

void CJweDoc::OnUpdateToolsPlacecube(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable( TRUE ) ;
}// OnUpdateToolsPlacecube



// Added 31.01.2000: gaspode
void CJweDoc::OnToolsPlacesheet() 
{

	if( m_Mode == MODE_POINTER_SHEET )
	{
		SetMode( m_PrevMode ) ;
	}
	else
	{
		SetMode( MODE_POINTER_SHEET ) ;
	}

	
}// OnToolsPlacesheet

void CJweDoc::OnUpdateToolsPlacesheet(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable( TRUE ) ;
}// OnUpdateToolsPlacesheet

// EOF: gaspode





void CJweDoc::OnToolsNextface() 
{
	Select_NextFace( m_pLevel );
	UpdateAllViews( nullptr, DOC_HINT_ORTHO, (CObject*)Level_GetSelDrawBounds( m_pLevel ) ) ;
}// OnToolsNextface

void CJweDoc::OnUpdateToolsNextface(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable( MODE_POINTER_FM == m_Mode && Level_HasSelections(m_pLevel) ) ;
}// OnUpdateToolsNextface

void CJweDoc::OnToolsPrevface() 
{
	Select_PrevFace( m_pLevel );
	UpdateAllViews( nullptr, DOC_HINT_ORTHO, (CObject*)Level_GetSelDrawBounds( m_pLevel ) ) ;

}//OnToolsPrevface

void CJweDoc::OnUpdateToolsPrevface(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable( MODE_POINTER_FM == m_Mode && Level_HasSelections(m_pLevel) ) ;
}// OnUpdateToolsPrevface


void CJweDoc::OnViewShowallgroups() 
{
    CMainFrame* pMainFrame = (CMainFrame*) AfxGetMainWnd();
	pMainFrame->m_GroupDialog.ShowAllGroups();
}

void CJweDoc::OnUpdateViewShowallgroups(CCmdUI* pCmdUI) 
{
    CMainFrame* pMainFrame = (CMainFrame*) AfxGetMainWnd();
	pCmdUI->Enable( pMainFrame->m_GroupDialog.HasHiddenItem() ) ;
}// OnUpdateViewShowallgroups

void CJweDoc::OnViewShowvisiblegroups() 
{
	
}

void CJweDoc::OnUpdateViewShowvisiblegroups(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable( false ) ;	
}

void CJweDoc::OnViewShowCurrentgroup() 
{
    CMainFrame* pMainFrame = (CMainFrame*) AfxGetMainWnd();
	pMainFrame->m_GroupDialog.ToggleSelectionVisibleState();
}

void CJweDoc::OnUpdateViewShowCurrentgroup(CCmdUI* pCmdUI) 
{
    CMainFrame* pMainFrame = (CMainFrame*) AfxGetMainWnd();
	pCmdUI->Enable( pMainFrame->m_GroupDialog.IsCurrentSelectionShowable() ) ;	
}// OnUpdateViewCurrentgroup

void CJweDoc::OnViewHideCurrentgroup() 
{
    CMainFrame* pMainFrame = (CMainFrame*) AfxGetMainWnd();
	pMainFrame->m_GroupDialog.ToggleSelectionVisibleState();
}

void CJweDoc::OnUpdateViewHideCurrentgroup(CCmdUI* pCmdUI) 
{
    CMainFrame* pMainFrame = (CMainFrame*) AfxGetMainWnd();
	pCmdUI->Enable( pMainFrame->m_GroupDialog.IsCurrentSelectionHidable() ) ;	
}// OnUpdateViewCurrentgroup

void CJweDoc::OnEditAddtogroup() 
{
}

void CJweDoc::OnUpdateEditAddtogroup(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable( true ) ;
}// OnUpdateEditAddtogroup

void CJweDoc::OnEditRemovefromgroup() 
{
	
}

void CJweDoc::OnUpdateEditRemovefromgroup(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable( true ) ;	
}// OnUpdateEditRemovefromgroup

//
// END MENU HANDLING
//

void CJweDoc::DrawGrid( CDC *pDC, Ortho *pOrtho)
{
	Draw_Grid( m_pLevel, pOrtho, pDC->m_hDC );
}

void CJweDoc::DrawOrthoName( CDC *pDC, Ortho *pOrtho)
{
	Draw_OrthoName( pOrtho, pDC->m_hDC );
}

void CJweDoc::DrawConstructorLine( CDC *pDC, Ortho *pOrtho )
{
	Draw_ConstructorLine( m_pLevel, pOrtho, pDC->m_hDC );
}

void CJweDoc::DrawSelected( CDC *pDC, Ortho *pOrtho )
{
	Draw_Selected( m_pLevel, pOrtho, pDC->m_hDC, m_Mode );
}

void CJweDoc::DrawObjects( CDC *pDC, Ortho *pOrtho )
{
	Draw_Objects( m_pLevel, pOrtho, pDC->m_hDC );
}

void CJweDoc::DrawSelectBounds( CDC *pDC, Ortho *pOrtho )
{
	const jeExtBox *	pSelWorldBounds;
	Rect				SelBounds{};
	CRect				cSelBounds{}; // Added jh
	int32				ModFlags{};
	COLORREF			co{};

	pSelWorldBounds =	Level_GetSelDrawBounds( m_pLevel ) ;
	if( jeExtBox_IsValid(  pSelWorldBounds ) && (m_Mode == MODE_POINTER_BB || m_Mode == MODE_POINTER_RS))
	{
		co = Settings_GetSelectedColor() ;
		Draw_SelectBounds( pSelWorldBounds, pOrtho, pDC->m_hDC, &SelBounds, co );
		Draw_SelectHandles( m_pLevel, pDC->m_hDC, m_Mode, &SelBounds );

		PrintRectDimensions (pDC,pOrtho,pSelWorldBounds); // Added JH 3.3.2000

	}
	pSelWorldBounds =	Level_GetSubSelDrawBounds( m_pLevel ) ;
	if( jeExtBox_IsValid(  pSelWorldBounds ) )
	{
		co = Settings_GetSubSelectedColor() ;
		Draw_SelectBounds( pSelWorldBounds, pOrtho, pDC->m_hDC, &SelBounds, co );

		ModFlags = Level_SubSelXFormModFlags( m_pLevel );
		if( ModFlags & JE_OBJECT_XFORM_ROTATE) 
			Draw_CornerHandles( &SelBounds, pDC->m_hDC, m_Mode );
	}	
}

void CJweDoc::DrawSelectElipse( CDC *pDC, Ortho *pOrtho )
{
	const jeExtBox* pSelWorldBounds{};

	pSelWorldBounds =	Level_GetSelDrawBounds( m_pLevel ) ;
	if( jeExtBox_IsValid(  pSelWorldBounds ) )
	{
		Draw_SelectBoundElipse( pSelWorldBounds, pOrtho, pDC->m_hDC );
	}
}

void CJweDoc::DrawSelectAxis( Ortho * pOrtho, HDC hDC )
{

	Draw_SelectAxis( m_pLevel, pOrtho, hDC );
}


// Added JH 3.3.2000 // fixed again on 30.3.2000
void CJweDoc::PrintRectDimensions( CDC *pDC,const Ortho * pOrtho, const jeExtBox	*pselBox )
{
	char	sTempString1[200];
	char	sTempString2[200];
	char	sText[400];

	jeVec3d pW{},pW1{};

	int		iBkMode=pDC->GetBkColor();

	CMainFrame *	pMainFrm = (CMainFrame*)AfxGetMainWnd() ;
	
	CFont *oldFont = pDC->SelectObject(&pMainFrm->cSmallFont);

	CRect	r{};

	sText[0]='\0';
	sTempString1[0]='\0';
	sTempString2[0]='\0';

	Ortho_WorldToViewRect( pOrtho, pselBox, (Rect*)&r );
	pW= pselBox->Min;
	pW1= pselBox->Max;

	if (Ortho_GetViewType(pOrtho)==Ortho_ViewFront)
		{ sprintf_s (sTempString1," X: %5.0f   Y: %5.0f \n",pselBox->Min.X,pselBox->Min.Y);
  		  sprintf_s (sTempString2,"dX: %5.0f  dY: %5.0f",pselBox->Max.X-pselBox->Min.X,pselBox->Max.Y-pselBox->Min.Y);
		}
	else if (Ortho_GetViewType(pOrtho)==Ortho_ViewSide)
		{ sprintf_s (sTempString1," Z: %5.0f   Y: %5.0f \n",pselBox->Min.Z,pselBox->Min.Y);
  		  sprintf_s (sTempString2,"dZ: %5.0f  dY: %5.0f",pselBox->Max.Z-pselBox->Min.Z,pselBox->Max.Y-pselBox->Min.Y);
		}
	else if (Ortho_GetViewType(pOrtho)==Ortho_ViewTop)
		{ sprintf_s (sTempString1," X: %5.0f   Z: %5.0f \n",pselBox->Min.X,pselBox->Min.Z);
  		  sprintf_s (sTempString2,"dX: %5.0f  dZ: %5.0f",pselBox->Max.X-pselBox->Min.X,pselBox->Max.Z-pselBox->Min.Z);
		}

	if (Settings_GetView_ShowMousePos())
		strcat_s (sText,sTempString1);
	if (Settings_GetView_ShowSize())
		strcat_s (sText,sTempString2);
	
	pDC->SetBkMode( TRANSPARENT);
	pDC->DrawText ( sText,r,DT_RIGHT|DT_BOTTOM /*|DT_SINGLELINE */ );
	pDC->SetBkMode( iBkMode );

	pDC->SelectObject(oldFont);

	pMainFrm->SetStatusPos (pselBox->Min.X,pselBox->Min.Y,pselBox->Min.Z);
	pMainFrm->SetStatusSize(pselBox->Max.X-pselBox->Min.X,pselBox->Max.Y-pselBox->Min.Y,pselBox->Max.Z-pselBox->Min.Z );

}
// EOF JH




jeBoolean CJweDoc::GetSelRadiusBox( Ortho *pOrtho, Rect *pBox )
{
	const jeExtBox* pSelWorldBounds{};
	pSelWorldBounds =	Level_GetSelDrawBounds( m_pLevel ) ;
	if( jeExtBox_IsValid(  pSelWorldBounds ) )
	{
		Draw_SelectGetElipseBox( pSelWorldBounds, pOrtho, pBox );
		return( JE_TRUE );
	}
	return( JE_FALSE );
}

void CJweDoc::RenderOrthoView(CDC *pDC, Ortho *pOrtho)
{
	ASSERT( pDC != nullptr ) ;
	ASSERT( pOrtho != nullptr ) ;

	if( m_pLevel == nullptr )
		return;
}// RenderOrthoView

jeBoolean CJweDoc::isPlaceBrushMode()
{

	return( MODE_POINTER_CUBE		== m_Mode ||
			MODE_POINTER_CYLINDER	== m_Mode ||
			MODE_POINTER_SPHERE		== m_Mode ||
			MODE_POINTER_SHEET		== m_Mode ||
			MODE_POINTER_ARCH		== m_Mode);
}

jeBoolean CJweDoc::isPlaceLightMode()
{

	return( MODE_POINTER_LIGHT == m_Mode ||
			MODE_POINTER_CAMERA == m_Mode ||
			MODE_POINTER_USEROBJ == m_Mode );
}

void CJweDoc::GetModeKind( int *Kind, int *SubKind )
{
	switch( m_Mode )
	{
	case MODE_POINTER_CUBE:	
		*Kind = KIND_BRUSH;
		*SubKind = BRUSH_BOX;
		break;

	case MODE_POINTER_CYLINDER:
		*Kind = KIND_BRUSH;
		*SubKind = BRUSH_CYLINDER;
		break;

	case MODE_POINTER_SPHERE:	
		*Kind = KIND_BRUSH;
		*SubKind = BRUSH_SPHERE;
		break;

	case MODE_POINTER_SHEET:
		*Kind = KIND_BRUSH;
		*SubKind = BRUSH_SHEET;
		break;

	case MODE_POINTER_LIGHT:
		*Kind = KIND_LIGHT;
		*SubKind = 0;
		break;

	case MODE_POINTER_CAMERA:
		*Kind = KIND_CAMERA;
		*SubKind = 0;
		break;

	case MODE_POINTER_USEROBJ:
		*Kind = KIND_USEROBJ;
		*SubKind = 0;
		break;

	case MODE_POINTER_ARCH:
		*Kind = KIND_BRUSH;
		*SubKind = BRUSH_ARCH;
		break;

	default:
		ASSERT( 0 );
	}
}

void CJweDoc::PlaceObject(jeExtBox	*pObjectBounds, jeBoolean bSubtract )
{
	jeExtBox		WorldBounds{} ;
	int Kind = KIND_INVALID;
	int SubKind = BRUSH_INVALID;
	Object * pObject{};
	jeProperty_List *pArray{};
	CMainFrame *	pMainFrm = (CMainFrame*)AfxGetMainWnd() ;

	GetModeKind( &Kind, &SubKind );

	if( Kind == KIND_BRUSH && ( Util_IsKeyDown( VK_CONTROL ) || bSubtract) )
	{
		pObject = Level_SubtractBrush( m_pLevel, SubKind, pObjectBounds );
		//	tom morris feb 2005
		pMainFrm->SetStatusText(m_strRebuild);
		//	end tom morris feb 2005
	}
	else
	if( Kind == KIND_USEROBJ )
	{
		CString	ObjTypeName;
		if( !pMainFrm->GetCurUserObjName(&ObjTypeName) )
			return;
		pObject = Level_NewUserObject( m_pLevel, ObjTypeName.GetBuffer(0), pObjectBounds );
	}
	else
		pObject = Level_NewObject( m_pLevel, Kind, SubKind, pObjectBounds );

	if( pObject == nullptr )
		return;

    Select_DeselectAll( m_pLevel, &WorldBounds );
	UpdateAllViews( nullptr, DOC_HINT_ORTHO, (CObject*)&WorldBounds ) ;
	Level_SelectObject( m_pLevel, pObject, LEVEL_SELECT );
	UpdateAllViews( nullptr, DOC_HINT_ORTHO, (CObject*)&WorldBounds ) ;

	pMainFrm->AddObject(pObject) ;
	pArray = Select_BuildDescriptor( m_pLevel );
	if (pArray == nullptr)
	{
		#pragma message ("log and deal with err")
	}
	else
	{
		pMainFrm->SetProperties( pArray );
		jeProperty_ListDestroy( &pArray );
	}
}

void CJweDoc::PlaceBrush( jeBoolean bSubtract )
{
    if (!jeExtBox_IsValid( &m_NewBrushBounds )) {
        return;
    }

	//ASSERT( jeExtBox_IsValid( &m_NewBrushBounds ) );
	ASSERT( isPlaceBrushMode() );

#pragma message ("Make new brush snap to grid" )
    // Krouer: make the brush appear at creation time
    LEVEL_UPDATE tmpBrushUpdate = Level_GetBrushUpdate(m_pLevel);
    Level_SetBrushUpdate(m_pLevel, LEVEL_UPDATE_CHANGE);
	PlaceObject( &m_NewBrushBounds, bSubtract );
    Level_SetBrushUpdate(m_pLevel, tmpBrushUpdate);
	 
	SetMode( m_PrevMode );
	SetNewBrushBoundInvalid();

//	tom morris feb 2005
	CMainFrame *pMainFrm = nullptr;
	pMainFrm = (CMainFrame*)AfxGetMainWnd();
	if (pMainFrm)
		pMainFrm->SetStatusText(m_strRebuild);
//	end tom morris feb 2005

}

#define DEFAULT_OBJECT_SIZE 64.0f

void CJweDoc::PlaceAtPoint( const Ortho * pOrtho, Point * pPoint,  jeBoolean bSubtract )
{
	jeVec3d WorldPt{};
	jeVec3d SnapDelta{};
	ORTHO_AXIS Axis{};
	float Constructor{};
	jeExtBox DefaultBox{};


	ASSERT( isPlaceBrushMode() || isPlaceLightMode() );

	jeExtBox_Set( &DefaultBox, -DEFAULT_OBJECT_SIZE, -DEFAULT_OBJECT_SIZE, -DEFAULT_OBJECT_SIZE,
								DEFAULT_OBJECT_SIZE,  DEFAULT_OBJECT_SIZE,  DEFAULT_OBJECT_SIZE );


	Ortho_ViewToWorld( pOrtho, pPoint->X, pPoint->Y,  &WorldPt ) ;
	Axis = Ortho_GetOrthogonalAxis( pOrtho ) ;
	Constructor = Level_GetConstructorPlane( m_pLevel, Axis );
	if (	( m_Mode == MODE_POINTER_LIGHT ) ||
			( m_Mode == MODE_POINTER_CAMERA ) ||
			( m_Mode == MODE_POINTER_USEROBJ ))
	{
		jeVec3d_SetElement( &WorldPt, Axis, Constructor );
	}
	else
	{
		jeVec3d_SetElement( &WorldPt, Axis, Constructor + DEFAULT_OBJECT_SIZE );
	}
	if( Level_IsSnapGrid( m_pLevel ) )
	{
		Transform_PlaceSnap( m_pLevel, &WorldPt, &SnapDelta );
		jeVec3d_Add( &WorldPt, &SnapDelta, &WorldPt );
	}

	jeExtBox_SetTranslation( &DefaultBox, &WorldPt );
	PlaceObject( &DefaultBox, bSubtract );
	SetMode( m_PrevMode );
}

jeBoolean CJweDoc::Select( const Ortho * pOrtho, const Point *pViewPt, LEVEL_STATE eState, jeBoolean bControl_Held )
{
	jeExtBox		WorldBounds{};
	CMainFrame* pMainFrm{};
	SELECT_RESULT	SelResult{};
	ObjectList* SubSelList{};
	Object* pObject{};
	ObjectIterator  Iterator{};
	ASSERT( pOrtho != nullptr ) ;
	ASSERT( pViewPt != nullptr ) ;

	pMainFrm = (CMainFrame*)AfxGetMainWnd() ;
	SelResult = Select_ClosestThing( m_pLevel, pOrtho, pViewPt, eState, &WorldBounds, m_Mode, bControl_Held ) ;
	if( SELECT_RESULT_CHANGED == SelResult )
	{

		jeProperty_List *pArray;

		pArray = Select_BuildDescriptor( m_pLevel );
		if( pArray )
		{
			pMainFrm->SetProperties( pArray );			
			jeProperty_ListDestroy( &pArray );
		}
		else
			pMainFrm->ResetProperties();
		pMainFrm->UpdatePanel( MAINFRM_PANEL_LISTS ) ;

	}
	if( SELECT_RESULT_SUBSELECT == SelResult )
	{
		 SubSelList = Level_GetSubSelList( m_pLevel );
		 pObject = ObjectList_GetFirst( SubSelList, &Iterator );
		 while( pObject )
		 {
			pMainFrm->SubSelectObject( pObject  ) ;
			pObject = ObjectList_GetNext( SubSelList, &Iterator );
		 }
	}
	UpdateAllViews( nullptr, DOC_HINT_ALL, (CObject*)&WorldBounds ) ;
	return( SELECT_RESULT_CHANGED == SelResult ) ;
}// Select

jeBoolean CJweDoc::SelectObject(Object *pObject, LEVEL_STATE eState)
{
	jeBoolean b{};
	Group* pGroup{};
	CMainFrame* pMainFrm{};
	jeProperty_List* pArray{};

	pMainFrm = (CMainFrame*)AfxGetMainWnd() ;
	pGroup = Object_IsMemberOfLockedGroup( pObject );
	if( pGroup != nullptr )
	{
		b =  Level_SelectGroup( m_pLevel, pGroup, eState );
	}
	else
	{
		b = Level_SelectObject( m_pLevel, pObject, eState );
	}

	pArray = Select_BuildDescriptor( m_pLevel );
	pMainFrm->SetProperties( pArray );
	if( pArray != nullptr )
		jeProperty_ListDestroy( &pArray );
	pMainFrm->UpdatePanel( MAINFRM_PANEL_LISTS ) ;
	UpdateAllViews( nullptr ) ;
	return b ;
}//SelectObject

jeBoolean CJweDoc::SubSelectgeObject(jeObject *pgeObject, LEVEL_STATE eState)
{
	jeBoolean b{};
	CMainFrame* pMainFrm{};

	pMainFrm = (CMainFrame*)AfxGetMainWnd() ;
	b = Level_SubSelectgeObject( m_pLevel, pgeObject, eState );

	UpdateAllViews( nullptr ) ;
	return b ;
}//SelectObject

jeBoolean CJweDoc::MarkSubSelect(jeObject *pgeObject, int32 flag)
{
	jeBoolean b{};
	CMainFrame* pMainFrm{};

	pMainFrm = (CMainFrame*)AfxGetMainWnd() ;
	b = Level_MarkSubSelect( m_pLevel, pgeObject, flag );

	UpdateAllViews( nullptr ) ;
	return b ;
}//SelectObject

// Append or Toggle on CTRL?  Desktop uses toggle

jeBoolean CJweDoc::RectangleSelect( jeExtBox *pBox, jeBoolean bAppend )
{
	jeExtBox	ChangedBounds{};
	jeBoolean	bSelChanged = JE_FALSE ;
	
	if( JE_FALSE == bAppend )
	{
		DeselectAll(JE_TRUE) ;

	}

	switch( m_Mode )
	{
		case MODE_POINTER_BB :
		case MODE_POINTER_RS :
			bSelChanged= Select_Rectangle( m_pLevel, pBox, Settings_IsSelByEncompass(), OBJECT_KINDALL, &ChangedBounds ) ;
			break ;

		case MODE_POINTER_FM :
			bSelChanged= Select_Rectangle( m_pLevel, pBox, Settings_IsSelByEncompass(), KIND_BRUSH, &ChangedBounds ) ;
			break ;

		case MODE_POINTER_VM :
			bSelChanged= Select_VertsInRectangle( m_pLevel, pBox, Settings_IsSelByEncompass(), &ChangedBounds ) ;
			break ;
	}
	if( JE_TRUE == bSelChanged )
	{
		jeProperty_List *pArray;

		pArray = Select_BuildDescriptor( m_pLevel );
		((CMainFrame*)AfxGetMainWnd())->SetProperties( pArray );			
		jeProperty_ListDestroy( &pArray );
		((CMainFrame*)AfxGetMainWnd())->UpdatePanel( MAINFRM_PANEL_LISTS ) ;
		UpdateAllViews( nullptr, DOC_HINT_ORTHO, (CObject*)&ChangedBounds ) ;
	}
	return bSelChanged ;
	pBox;
}// RectangleSelect


jeBoolean CJweDoc::Select3d( const jeCamera * pCamera, const Point *pViewPt )
{
	jeBoolean	bSelChanged{};
	uint32		c1{}, c2{};
	char		Buff[255];

	CMainFrame* pMainFrm{};
	ASSERT( pCamera != nullptr ) ;
	ASSERT( pViewPt != nullptr ) ;

	pMainFrm = (CMainFrame*)AfxGetMainWnd() ;
	if( !Util_IsKeyDown( VK_CONTROL ) )
	{
		DeselectAll(JE_TRUE);
	}
	bSelChanged = Select_Face( m_pLevel, pCamera, pViewPt, &c1, &c2 ) ;
	if( JE_TRUE == bSelChanged )
	{

		jeProperty_List *pArray;

		pArray = Select_BuildDescriptor( m_pLevel );
		sprintf_s( Buff, "Contents 1 %x Contents 2 %x", c1, c2 );
		pMainFrm->SetStatusText( Buff);
		pMainFrm->SetProperties( pArray );			
		jeProperty_ListDestroy( &pArray );
		pMainFrm->UpdatePanel( MAINFRM_PANEL_LISTS ) ;
		UpdateAllViews( nullptr, DOC_HINT_ALL, (CObject*)Level_GetSelBounds( m_pLevel) ) ;
	}
	return bSelChanged ;
}// Select

void CJweDoc::DeselectAllSub()
{
	jeExtBox  WorldBounds{};
	Level_DeselectAllSub( m_pLevel, &WorldBounds );
	UpdateAllViews( nullptr, DOC_HINT_ORTHO, (CObject*)&WorldBounds ) ;
}


SELECT_HANDLE CJweDoc::ViewPointHandle( Ortho * pOrtho, Point * pViewPt, jeExtBox * pWorldBox )
{
	SELECT_HANDLE Handle{};
	int32 XFormMod{};

	Handle = Select_ViewPointHandle( pOrtho, pViewPt, pWorldBox );
	XFormMod = Level_SelXFormModFlags( m_pLevel );

	if( MODE_POINTER_BB == m_Mode && !(XFormMod & JE_OBJECT_XFORM_SCALE ) )
		Handle = Select_None;

	if( MODE_POINTER_RS == m_Mode && IS_CORNER_HANDLE(Handle) && !(XFormMod & JE_OBJECT_XFORM_ROTATE ) )
		Handle = Select_None;

	if( MODE_POINTER_RS == m_Mode && IS_EDGE_HANDLE(Handle) && !(XFormMod & JE_OBJECT_XFORM_SHEAR ) )
		Handle = Select_None;
	return( Handle );
}

SELECT_HANDLE CJweDoc::SubViewPointHandle( Ortho * pOrtho, Point * pViewPt, jeExtBox * pWorldBox )
{
	SELECT_HANDLE Handle{};
	int32 XFormMod{};

	XFormMod = Level_SubSelXFormModFlags( m_pLevel );

	if( !(XFormMod & SubSelect_Rotate ) )
		return( Select_None );

	*pWorldBox = *Level_GetSubSelDrawBounds( m_pLevel ) ;
	Handle = Select_ViewPointHandle( pOrtho, pViewPt, pWorldBox );

	if( IS_EDGE_HANDLE(Handle) )
		return( Select_None );

	return( Handle );
}

void CJweDoc::DeselectAll( jeBoolean UpadatePannel )
{
	jeBoolean	bSelChanged{};
	jeExtBox	WorldBounds{};

	((CMainFrame*)AfxGetMainWnd())->ResetProperties();

	if( m_Mode == MODE_POINTER_VM )
		bSelChanged = Select_DeselectAllVerts( m_pLevel ) ; 
	else
		bSelChanged = Select_DeselectAll( m_pLevel, &WorldBounds ) ;
	
	if( JE_TRUE == bSelChanged )
	{
		UpdateAllViews( nullptr, DOC_HINT_ALL, (CObject*)&WorldBounds ) ;
	}
	UpadatePannel;
}// DeselectAll

void CJweDoc::DeselectAllFaces()
{
	const jeExtBox* pWorldBounds{};
	Select_DeselectAllFaces( m_pLevel );
	pWorldBounds = Level_GetSelBounds( m_pLevel ) ;
	UpdateAllViews( nullptr, DOC_HINT_ALL, (CObject*)pWorldBounds ) ;
}

void CJweDoc::BeginMove( const Ortho * pOrtho, SELECT_HANDLE eCorner, jeBoolean bCopy )
{
	jeVec3d		Distance{};
	jeVec3d		SnapDelta{};
	jeExtBox	WorldBounds{};
	CMainFrame *	pMainFrm = nullptr;
	pMainFrm = (CMainFrame*)AfxGetMainWnd() ;
		
		// Handling for Move-Copy
	m_bCopying = bCopy ;
	if( m_bCopying )
	{
		jeProperty_List *pArray;

		if( JE_FALSE == Select_DupAndDeselectSelections( m_pLevel ) )
			return ;

		pMainFrm->AddSelection( this ) ;

		pArray = Select_BuildDescriptor( m_pLevel );
		pMainFrm->SetProperties( pArray );			
		pMainFrm->UpdatePanel( MAINFRM_PANEL_LISTS ) ;

	}
	else
		Select_DragBegin( m_pLevel ) ;

	jeVec3d_Clear( &m_DragPoint ) ;

#pragma message( "bCopy NZ means undo create at new location" )
	if( JE_FALSE == m_bCopying )
		Transform_AddSelectedUndo( m_pLevel, UNDO_MOVE ) ;

	if( Level_IsSnapGrid( m_pLevel ) )
	{
		jeVec3d_Clear( &Distance ) ;
		Transform_MoveSnapSelected( m_pLevel, eCorner, Ortho_GetHorizontalAxis( pOrtho ), Ortho_GetVerticalAxis( pOrtho ), &WorldBounds, &SnapDelta ) ;
		UpdateAllViews( nullptr, DOC_HINT_ORTHO, (CObject*)&WorldBounds ) ;
	}

}// BeginMove

void CJweDoc::BeginMoveSub( )
{

	Select_DragBeginSub( m_pLevel ) ;

	jeVec3d_Clear( &m_DragPoint ) ;

}// BeginMove

void CJweDoc::EndMove()
{
	CMainFrame *	pMainFrm = (CMainFrame*)AfxGetMainWnd() ;
	Select_DragEnd( m_pLevel ) ;
	if( m_bCopying )
	{
		Select_CreateSelectedUndo( m_pLevel, UNDO_CREATE ) ;
	}
	UpdateStats();
	pMainFrm->PostUpdateProperties();
	UpdateAllViews( nullptr, DOC_HINT_RENDERED ) ;
}// EndMove

void CJweDoc::EndMoveSub()
{
	CMainFrame *	pMainFrm = (CMainFrame*)AfxGetMainWnd() ;
	ObjectList* SubSelList{};
	Object* pObject{};
	ObjectIterator  Iterator{};

	Select_DragEndSub( m_pLevel ) ;

	SubSelList = Level_GetSubSelList( m_pLevel );
	pObject = ObjectList_GetFirst( SubSelList, &Iterator  );
	while( pObject != nullptr )
	{
		pMainFrm->EndMoveSub( pObject  );
		pObject = ObjectList_GetNext( SubSelList, &Iterator  );
	}
	UpdateAllViews( nullptr, DOC_HINT_RENDERED, nullptr ) ;
}// EndMoveSub

void CJweDoc::MoveSelected( SELECT_HANDLE eCorner, jeVec3d *pWorldDistance )
{
	jeExtBox	WorldBounds{};
	jeVec3d		SnapPoint{};
	jeVec3d		GridPoint{};
	jeVec3d		GridDiff{};
	LEVEL_SEL	SelectType{};
	DOC_HINT	Hint{};
	LEVEL_UPDATE	LightUpdate{};
	CMainFrame	*pMainFrm = nullptr;
	pMainFrm = (CMainFrame*)AfxGetMainWnd();

	ASSERT( pWorldDistance != nullptr ) ;

	eCorner ;
	WorldBounds = *Level_GetSelBounds( m_pLevel );

	SelectType = Level_GetSelType( m_pLevel );
	LightUpdate = Level_GetLightUpdate( m_pLevel );
	if( (SelectType == LEVEL_SELONEOBJECT || SelectType == LEVEL_SELOBJECTS ) ||
		(LightUpdate == LEVEL_UPDATE_REALTIME && (SelectType == LEVEL_SELONELIGHT || SelectType == LEVEL_SELLIGHTS ))
		)
		Hint = DOC_HINT_ALL;
	else
		Hint = DOC_HINT_ORTHO;

	if( Level_IsSnapGrid( m_pLevel ) )
	{
		Transform_PointToGrid( m_pLevel, &m_DragPoint, &GridPoint ) ;

		jeVec3d_Add( &m_DragPoint, pWorldDistance, &m_DragPoint ) ;
		Transform_PointToGrid( m_pLevel, &m_DragPoint, &SnapPoint ) ;
		
		if( jeVec3d_Compare( &GridPoint, &SnapPoint, 0.01f ) == JE_FALSE )
		{
			jeVec3d_Subtract( &SnapPoint, &GridPoint, &GridDiff ) ;
			Transform_MoveSelected( m_pLevel, &GridDiff, &WorldBounds ) ;
//			jeVec3d_Subtract( &m_DragPoint, &Remainder, &m_DragPoint ) ;
			UpdateAllViews( nullptr, Hint, (CObject*)&WorldBounds ) ;
		}
	}
	else
	{
		Transform_MoveSelected( m_pLevel, pWorldDistance, &WorldBounds ) ;
		UpdateAllViews( nullptr, Hint, (CObject*)&WorldBounds ) ;
	}

}// MoveSelected

void CJweDoc::MoveSelectedSub( SELECT_HANDLE eCorner, jeVec3d *pWorldDistance )
{
	jeExtBox	WorldBounds{};
	jeVec3d		SnapPoint{};
	jeVec3d		GridPoint{};
	jeVec3d		GridDiff{};
	DOC_HINT	Hint{};

	ASSERT( pWorldDistance != nullptr ) ;

	eCorner ;
	WorldBounds = *Level_GetSelBounds( m_pLevel );

	Hint = DOC_HINT_ALL;

	if( Level_IsSnapGrid( m_pLevel ) )
	{
		Transform_PointToGrid( m_pLevel, &m_DragPoint, &GridPoint ) ;

		jeVec3d_Add( &m_DragPoint, pWorldDistance, &m_DragPoint ) ;
		Transform_PointToGrid( m_pLevel, &m_DragPoint, &SnapPoint ) ;
		
		if( jeVec3d_Compare( &GridPoint, &SnapPoint, 0.01f ) == JE_FALSE )
		{
			jeVec3d_Subtract( &SnapPoint, &GridPoint, &GridDiff ) ;
			Transform_MoveSelectedSub( m_pLevel, &GridDiff, &WorldBounds ) ;
			UpdateAllViews( nullptr, Hint, (CObject*)&WorldBounds ) ;
		}
	}
	else
	{
		Transform_MoveSelectedSub( m_pLevel, pWorldDistance, &WorldBounds ) ;
		UpdateAllViews( nullptr, Hint, (CObject*)&WorldBounds ) ;
	}

}// MoveSelected

jeBoolean CJweDoc::BeginMoveVerts(const Ortho *pOrtho)
{
	CMainFrame *pMainFrm = nullptr;
	pMainFrm = (CMainFrame*)AfxGetMainWnd();

	jeVec3d_Clear( &m_DragPoint ) ;

	return JE_TRUE ;
	pOrtho ;
}//BeginMoveVerts

jeBoolean CJweDoc::MoveVerts(const Ortho *pOrtho, jeVec3d *pWorldDistance)
{
	jeVec3d		SnapPoint{};
	jeVec3d		GridPoint{};
	jeVec3d		GridDiff{};
	jeExtBox	WorldBounds{};
	
	if( Level_IsSnapGrid( m_pLevel ) && Level_GetShouldSnapVerts( m_pLevel) )
	{
		Transform_PointToGrid( m_pLevel, &m_DragPoint, &GridPoint ) ;

		jeVec3d_Add( &m_DragPoint, pWorldDistance, &m_DragPoint ) ;
		Transform_PointToGrid( m_pLevel, &m_DragPoint, &SnapPoint ) ;
		
		if( jeVec3d_Compare( &GridPoint, &SnapPoint, 0.01f ) == JE_FALSE )
		{
			jeVec3d_Subtract( &SnapPoint, &GridPoint, &GridDiff ) ;
			Select_MoveSelectedVert( m_pLevel, &GridDiff, &WorldBounds );
//			jeVec3d_Subtract( &m_DragPoint, &Remainder, &m_DragPoint ) ;
		}
	}
	else
	{
		Select_MoveSelectedVert( m_pLevel, pWorldDistance, &WorldBounds );
	}

//	Select_MoveSelectedVert( m_pLevel, pWorldDistance, &WorldBounds );
	UpdateAllViews( nullptr, DOC_HINT_ORTHO, (CObject*)&WorldBounds ) ;
	return JE_TRUE ;
	pOrtho;pWorldDistance;
}// MoveVerts


void CJweDoc::EndMoveVerts( void )
{
	UpdateStats();
}// EndMoveVerts


jeBoolean CJweDoc::HasSelections( jeExtBox * pSelBounds )
{
	jeBoolean	bHasSelections{};

	if( MODE_POINTER_VM == m_Mode )
		bHasSelections = Select_HasSelectedVerts( m_pLevel ) ;
	else
		bHasSelections = Level_HasSelections( m_pLevel ) ;

	if( bHasSelections )
	{
		*pSelBounds = *Level_GetSelDrawBounds( m_pLevel ) ;
	}
	return bHasSelections ;
}// HasSelections

jeBoolean CJweDoc::HasSubSelections( jeExtBox * pSelBounds )
{
	jeBoolean	bHasSelections{};

	bHasSelections = Level_HasSubSelections( m_pLevel ) ;

	if( bHasSelections )
	{
		*pSelBounds = *Level_GetSubSelDrawBounds( m_pLevel ) ;
	}
	return bHasSelections ;
}// HasSelections

int32 CJweDoc::SubSelXFormModFlags()
{
	return( Level_SubSelXFormModFlags( m_pLevel ));
}

LEVEL_SEL CJweDoc::GetSelType()
{
	return Level_GetSelType( m_pLevel ) ;
}// GetSelType

DOC_CONSTRUCTORS CJweDoc::ViewPointConstructor( Ortho * pOrtho, Point * pViewPt)
{
	jeVec3d WorldPt{};
	ORTHO_AXIS HAxis{};
	ORTHO_AXIS VAxis{};
	float	   Element{};
	float	   Plane{};
	float	   DifSq{};
	DOC_CONSTRUCTORS Constructor = DOC_NO_CONSTRUCTOR;

	HAxis = Ortho_GetHorizontalAxis( pOrtho );
	VAxis = Ortho_GetVerticalAxis( pOrtho );

	Ortho_ViewToWorld( pOrtho, pViewPt->X, pViewPt->Y, &WorldPt ) ;

	Element = jeVec3d_GetElement( &WorldPt, HAxis );
	Plane = Level_GetConstructorPlane( m_pLevel, HAxis );
	DifSq = ( Plane - Element ) * ( Plane - Element );
	if( DifSq < Ortho_GetWorldSelectThreshold( pOrtho ) )
	{
		Constructor = DOC_HORIZONTAL_CONSTRUCTOR;
	}

	Element = jeVec3d_GetElement( &WorldPt, VAxis );
	Plane = Level_GetConstructorPlane( m_pLevel, VAxis );
	DifSq = ( Plane - Element ) * ( Plane - Element );
	if( DifSq < Ortho_GetWorldSelectThreshold( pOrtho ) )
	{
		if( Constructor == DOC_HORIZONTAL_CONSTRUCTOR )
			Constructor = DOC_BOTH_CONSTRUCTOR;
		else
			Constructor = DOC_VERTICAL_CONSTRUCTOR;

	}
	return( Constructor );
}

void   CJweDoc::MoveConstructor( Ortho *pOrtho, DOC_CONSTRUCTORS Constructor, Point * pMousePt, Point *pAnchor )
{
	ORTHO_AXIS HAxis{};
	ORTHO_AXIS VAxis{};
	jeVec3d MouseVec{};
	jeVec3d AnchorVec{};
	jeVec3d WorldDistance{};
	jeVec3d	Delta{};
	float	Element{};

	Ortho_ViewToWorld( pOrtho, pAnchor->X, pAnchor->Y, &AnchorVec );
	Ortho_ViewToWorld( pOrtho, pMousePt->X, pMousePt->Y, &MouseVec );
	if( Level_IsSnapGrid( m_pLevel ) )
	{
		Transform_PointToGrid( m_pLevel, &AnchorVec, &AnchorVec ) ;
		Transform_PointToGrid( m_pLevel, &MouseVec, &MouseVec ) ;
	}
	jeVec3d_Subtract( &MouseVec, &AnchorVec, &WorldDistance );

	HAxis = Ortho_GetHorizontalAxis( pOrtho );
	VAxis = Ortho_GetVerticalAxis( pOrtho );

	jeVec3d_Set( &Delta, 0.0f, 0.0f, 0.0f );
	switch( Constructor )
	{
	case DOC_HORIZONTAL_CONSTRUCTOR:
		Element = jeVec3d_GetElement( &MouseVec, HAxis );
		Level_SetConstructor( m_pLevel, HAxis, Element );
		break;

	case DOC_VERTICAL_CONSTRUCTOR:
		Element = jeVec3d_GetElement( &MouseVec, VAxis );
		Level_SetConstructor( m_pLevel, VAxis, Element );
		break;

	case DOC_BOTH_CONSTRUCTOR:
		Element = jeVec3d_GetElement( &MouseVec, HAxis );
		Level_SetConstructor( m_pLevel, HAxis, Element );
		Element = jeVec3d_GetElement( &MouseVec, VAxis );
		Level_SetConstructor( m_pLevel, VAxis, Element );
		break;

	default:
		ASSERT(0);
		break;
	}	
	UpdateAllViews( nullptr, DOC_HINT_ORTHO, nullptr ) ;
}


LPCTSTR CJweDoc::GetConstructorCursor(Ortho *pOrtho, POINT *pViewPt)
{
	DOC_CONSTRUCTORS Constructor{};
	LPCTSTR CursorId = IDC_ARROW;

	Constructor = ViewPointConstructor( pOrtho, (Point*)pViewPt);
	switch( Constructor )
	{
	case DOC_HORIZONTAL_CONSTRUCTOR:
		CursorId = IDC_SIZEWE;
		break;

	case DOC_VERTICAL_CONSTRUCTOR:
		CursorId = IDC_SIZENS;
		break;

	case DOC_BOTH_CONSTRUCTOR:
		CursorId = IDC_SIZENESW;
		break;

	default:
		CursorId = IDC_ARROW;
		break;
	}


	return CursorId;

}

void CJweDoc::SetCursor(Ortho *pOrtho, POINT *pViewPt)
{
	int				nID{};
	LPCTSTR			nIDStd{};
	HCURSOR			hCursor{};
	jeExtBox		WorldBox{};
	SELECT_HANDLE	Handle{};
	ASSERT( pOrtho != nullptr ) ;
	ASSERT( pViewPt != nullptr ) ;

	nID = 0 ;
	nIDStd = IDC_ARROW ;

	if(!m_pLevel) return; // Added by Incarnadine

	if( isPlaceBrushMode() )
	{
		switch( m_Mode )
		{
		case MODE_POINTER_CUBE:
			nID = IDC_CUBE;
			break;

		case MODE_POINTER_CYLINDER:
			nID = IDC_CYLINDER;
			break;

		case MODE_POINTER_SPHERE:
			nID = IDC_SPHERE;
			break;

		case MODE_POINTER_SHEET:
			nID = IDC_SHEET;
			break;

		case MODE_POINTER_ARCH:
			nID = IDC_ARCH;
			break;

		}
	}
	else if( m_Mode == MODE_POINTER_LIGHT )
	{
		nID = IDC_LIGHT;
	}
	else if( m_Mode == MODE_POINTER_CAMERA )
	{
		nID = IDC_CAMERA;
	}
	else if ( m_Mode == MODE_POINTER_USEROBJ )
	{
		nID = IDC_CUBE;
	}
	else
	if( Level_HasSelections( m_pLevel ) )	// Doc "has selections" tests mode
	{
		if( MODE_POINTER_VM == m_Mode )
		{
			if( Select_IsPointOverVertex( pOrtho, (Point*)pViewPt, m_pLevel ) )
			{
				nIDStd = IDC_CROSS ;	
			}
		}
		else
		{
			int32 XFormMod{};
			WorldBox = *Level_GetSelDrawBounds( m_pLevel ) ;
			if( jeExtBox_IsValid( &WorldBox ) && Ortho_IsViewPointInWorldBox( pOrtho, pViewPt->x, pViewPt->y, &WorldBox ) )
			{
				nID = IDC_MOVESELECT ;	
			}

			Handle = Select_ViewPointHandle( pOrtho, (Point*)pViewPt, &WorldBox ) ;
			XFormMod = Level_SelXFormModFlags( m_pLevel );

			if( MODE_POINTER_BB == m_Mode && !(XFormMod & JE_OBJECT_XFORM_SCALE ) )
				Handle = Select_None;

			if( MODE_POINTER_RS == m_Mode && IS_CORNER_HANDLE(Handle) && !(XFormMod & JE_OBJECT_XFORM_ROTATE ) )
				Handle = Select_None;

			if( MODE_POINTER_RS == m_Mode && IS_EDGE_HANDLE(Handle) && !(XFormMod & JE_OBJECT_XFORM_SHEAR ) )
				Handle = Select_None;

			if( Handle != Select_None )
			{
				switch( m_Mode )
				{
				case MODE_POINTER_BB :
				switch( Handle )
				{
					case Select_TopLeft :		nID = 0 ; nIDStd = IDC_SIZENWSE ;	break ;
					case Select_BottomRight :	nID = 0 ; nIDStd = IDC_SIZENWSE ; break ;
					case Select_TopRight :		nID = 0 ; nIDStd = IDC_SIZENESW ;	break ;
					case Select_BottomLeft :	nID = 0 ; nIDStd = IDC_SIZENESW ; break ;
					case Select_Left :			nID = 0 ; nIDStd = IDC_SIZEWE ;	break ;
					case Select_Right :			nID = 0 ; nIDStd = IDC_SIZEWE ;	break ;
					case Select_Top :			nID = 0 ; nIDStd = IDC_SIZENS ;	break ;
					case Select_Bottom :		nID = 0 ; nIDStd = IDC_SIZENS ;	break ;
				}
				break ;
				
				case MODE_POINTER_RS :
					switch( Handle )
					{
						case Select_TopLeft :
						case Select_BottomRight :
						case Select_TopRight :
						case Select_BottomLeft :
							nID = IDC_ROTATE ;	break ;
						case Select_Left :
						case Select_Right :
							nID = IDC_SHEARLR ;	break ;
						case Select_Top :
						case Select_Bottom :
							nID = IDC_SHEARTB ;	break ;
						case Select_Center : 
							nID = IDC_ROTATIONCENTER ; 
							break ;
					}
					break ;
				}//Switch Mode
			}// Cursor is on a handle
		}// BB OR RS
	}// Selection Handles exist
	
	if( Level_HasSubSelections( m_pLevel ) )
	{
		int32 XFormMod{};
		WorldBox = *Level_GetSubSelDrawBounds(m_pLevel );
		if( Ortho_IsViewPointInWorldBox( pOrtho, pViewPt->x, pViewPt->y, &WorldBox ) )
		{
			nID = IDC_MOVESELECT ;	
		}
		Handle = Select_ViewPointHandle( pOrtho, (Point*)pViewPt, &WorldBox ) ;
		XFormMod = Level_SubSelXFormModFlags( m_pLevel );
		if( IS_CORNER_HANDLE( Handle ) && XFormMod & SubSelect_Rotate )
			nID = IDC_ROTATE ;
	}

	if( nID == 0 && nIDStd == IDC_ARROW )
	{
		nIDStd = GetConstructorCursor( pOrtho, pViewPt );
	}
	if( nID != 0 )
	{
		hCursor = ::LoadCursor( AfxGetInstanceHandle( ), MAKEINTRESOURCE(nID) ) ;
	}
	else
	{
		hCursor = ::LoadCursor( 0, nIDStd ) ;
	}

	::SetCursor( hCursor ) ;

}// SetCursor

jeBoolean CJweDoc::BeginRotateSub( )
{
	BeginRotate();
	Select_DragBeginSub( m_pLevel ) ;
	jeVec3d_Clear( &m_DragPoint ) ;
	return JE_TRUE ;
}// BeginRotateSub

jeBoolean CJweDoc::BeginMoveHandle( const Ortho * pOrtho, SELECT_HANDLE eHandle, DOC_HANDLE_MODE *HandleMode )
{
	jeExtBox	WorldBounds{};
	jeVec3d		Distance{};
	jeVec3d		SnapDelta{};

	CMainFrame *pMainFrm = nullptr;
	pMainFrm = (CMainFrame*)AfxGetMainWnd();

	*HandleMode =	DOC_HANDLE_NONE;
	switch( m_Mode )
	{
	case MODE_POINTER_BB :
		BeginSize( ) ;
		Transform_AddSelectedUndo( m_pLevel, UNDO_RESIZE );
		*HandleMode =	DOC_HANDLE_SIZE;
		break ;

	case MODE_POINTER_RS :
		if( Select_IsCorner( eHandle ) )
		{
			BeginRotate() ;
			Transform_AddSelectedUndo( m_pLevel, UNDO_ROTATE );
			*HandleMode =	DOC_HANDLE_ROTATE;
		}
		else
		{
			BeginShear() ;
			Transform_AddShearSelectedUndo( m_pLevel );
			*HandleMode =	DOC_HANDLE_SHEAR;
		}
		break ;
	}	

	Select_DragBegin( m_pLevel ) ;
	
	jeVec3d_Clear( &m_DragPoint ) ;
	WorldBounds = *(Level_GetSelDrawBounds( m_pLevel)) ;
	if( Level_IsSnapGrid( m_pLevel ) && m_Mode == MODE_POINTER_BB)
	{
		jeVec3d_Clear( &Distance ) ;

		Transform_SizeSnapSelected
		( 
			m_pLevel, 
			eHandle, 
			Ortho_GetHorizontalAxis( pOrtho ), 
			Ortho_GetVerticalAxis( pOrtho ),
			&WorldBounds, 
			&SnapDelta
		) ;
		UpdateAllViews( nullptr, DOC_HINT_ORTHO, (CObject*)&WorldBounds ) ;
	}

	return JE_TRUE ;
}// BeginSize

void CJweDoc::RotateSelectedSub(const Ortho * pOrtho,  Point * pMousePt, Point *pAnchor )
{
	jeFloat RotationAngle{};
	jeVec3d SelCenter{};
	CPoint	SelCenterPt{};
	jeExtBox	WorldBounds{};
	const jeExtBox* SubDrawBounds{};
	jeFloat dRotationAngle{};
	
	
	SubDrawBounds = Level_GetSubSelDrawBounds( m_pLevel );
	jeExtBox_GetTranslation( SubDrawBounds, &SelCenter ) ; 

	Ortho_WorldToView( pOrtho, &SelCenter, (Point*)&SelCenterPt ) ;
	RotationAngle = Ortho_GetRotationFromView( pOrtho, (Point*)pMousePt, (Point*)pAnchor, (Point*)&SelCenterPt );
	dRotationAngle = RotationAngle - m_LastRotateAngle;
	if( Level_IsSnapGrid( m_pLevel ) )
	{
		float mod;
		float Rad;

		Rad = jeFloat_DegToRad( (float)Level_GetRotateSnapSize( m_pLevel ) );
		mod = (float)fmod( dRotationAngle, Rad );
		if( mod < (Rad*0.5f) )
			dRotationAngle = dRotationAngle - mod;
		else
			dRotationAngle = dRotationAngle - mod + Rad;
	}

	if( dRotationAngle )
	{
		Transform_RotateSubSelected
		( 
			m_pLevel, 
			dRotationAngle,
			Ortho_GetOrthogonalAxis( pOrtho ), 
			&WorldBounds
		) ;
		m_LastRotateAngle += dRotationAngle;
		Util_ExtBox_Union( SubDrawBounds, &WorldBounds, &WorldBounds );
		UpdateAllViews( nullptr, DOC_HINT_ALL, (CObject*)&WorldBounds ) ;
	}

}

void CJweDoc::MoveHandle(const Ortho * pOrtho, jeVec3d *pWorldDistance, SELECT_HANDLE eSizeType, Point * pMousePt, Point *pAnchor, jeVec3d *pCenter3d )
{
	jeExtBox	WorldBounds{};
	jeVec3d		SnapPoint{};
	jeVec3d		GridPoint{};
	jeVec3d		GridDiff{};
	ASSERT( pWorldDistance != nullptr ) ;

	switch( m_Mode )
	{
	case MODE_POINTER_BB :
		if( Level_IsSnapGrid( m_pLevel ) )
		{
			Transform_PointToGrid( m_pLevel, &m_DragPoint, &GridPoint ) ;
			jeVec3d_Add( &m_DragPoint, pWorldDistance, &m_DragPoint ) ;
			Transform_PointToGrid( m_pLevel, &m_DragPoint, &SnapPoint ) ;
			if( jeVec3d_Compare( &GridPoint, &SnapPoint, 0.01f ) == JE_FALSE )
			{
				jeVec3d_Subtract( &SnapPoint, &GridPoint, &GridDiff ) ;
				Transform_SizeSelected
				( 
					m_pLevel, 
					&GridDiff, 
					eSizeType, 
					Ortho_GetHorizontalAxis( pOrtho ), 
					Ortho_GetVerticalAxis( pOrtho ), 
					&WorldBounds 
				) ;
				//jeVec3d_Subtract( &m_DragPoint, &Remainder, &m_DragPoint ) ;
				UpdateAllViews( nullptr, DOC_HINT_ORTHO, (CObject*)&WorldBounds ) ;
			}
		}
		else
		{
			Transform_SizeSelected
			( 
				m_pLevel, 
				pWorldDistance, 
				eSizeType, 
				Ortho_GetHorizontalAxis( pOrtho ), 
				Ortho_GetVerticalAxis( pOrtho ), 
				&WorldBounds 
			) ;
			UpdateAllViews( nullptr, DOC_HINT_ORTHO, (CObject*)&WorldBounds ) ;
		}
		break ;

	case MODE_POINTER_RS :
		if( Select_IsCorner( eSizeType ) )
		{
			jeFloat RotationAngle{};
			jeFloat dRotationAngle{};
			CPoint	SelCenterPt{};
			LEVEL_SEL SelectType{};

			Ortho_WorldToView( pOrtho, pCenter3d, (Point*)&SelCenterPt ) ;
			RotationAngle = Ortho_GetRotationFromView( pOrtho, (Point*)pMousePt, (Point*)pAnchor, (Point*)&SelCenterPt );
			dRotationAngle = RotationAngle - m_LastRotateAngle;
			if( Level_IsSnapGrid( m_pLevel ) )
			{
				float mod{};
				float Rad{};

				Rad = jeFloat_DegToRad( (float)Level_GetRotateSnapSize( m_pLevel ) );
				mod = (float)fmod( dRotationAngle, Rad );
				if( mod < (Rad*0.5f) )
					dRotationAngle = dRotationAngle - mod;
				else
					dRotationAngle = dRotationAngle - mod + Rad;
			}

			if( dRotationAngle )
			{
				Transform_RotateSelected
				( 
					m_pLevel, 
					dRotationAngle,
					Ortho_GetOrthogonalAxis( pOrtho ), 
					pCenter3d,
					&WorldBounds
				) ;
				m_LastRotateAngle += dRotationAngle;
				SelectType = Level_GetSelType( m_pLevel );
				if( SelectType == LEVEL_SELONEOBJECT || SelectType == LEVEL_SELOBJECTS )
					UpdateAllViews( nullptr, DOC_HINT_ALL, (CObject*)&WorldBounds ) ;
				else
					UpdateAllViews( nullptr, DOC_HINT_ORTHO, (CObject*)&WorldBounds ) ;
			}
		}
	    else if (Select_IsEdge(eSizeType) )
		{
			Transform_ShearSelected
			( 
				m_pLevel, 
				pWorldDistance, 
				eSizeType, 
				Ortho_GetHorizontalAxis( pOrtho ), 
				Ortho_GetVerticalAxis( pOrtho ), 
				&WorldBounds
			) ;
			UpdateAllViews( nullptr, DOC_HINT_ORTHO, (CObject*)&WorldBounds ) ;
		}
		// else if (Select_IsCenter()
		break ;
	}
}// SizeSelected

void CJweDoc::UpdateStats()
{
	CMainFrame *	pMainFrm = (CMainFrame*)AfxGetMainWnd() ;
	Model* pModel{};
	const jeBSP_DebugInfo *pDebugInfo;
	
	pModel = Level_GetCurModel( m_pLevel);
	ASSERT( pModel );
	pDebugInfo = jeModel_GetBSPDebugInfo( Model_GetguModel(pModel ) );
	pMainFrm->SetStats(pDebugInfo );

//	tom morris feb 2005 -- to constrain statusbar reminder to rebuild
	LEVEL_SEL SelType;
	SelType = Level_GetSelType( m_pLevel );
	
	if( (SelType & LEVEL_SELONEBRUSH ) ||
		(SelType & LEVEL_SELBRUSHES ) ||
		(SelType & LEVEL_SELMANY ))
	{
		pMainFrm->SetStatusText(m_strRebuild);
	}
//	end tom morris feb 2005

}

void CJweDoc::OnToolsRebuildall() 
{
	int Result{};
	jeBSP_Options Options = 0;
	jeBSP_Logic Logic{};
	jeBSP_LogicBalance LogicBalance{};

	Level_GetBSPBuildOptions( m_pLevel, &Options, &Logic, &LogicBalance );
	if( Options & BSP_OPTIONS_CSG_BRUSHES )
		RebuildDlg->m_CSG = JE_TRUE;
	else
		RebuildDlg->m_CSG = JE_FALSE;

	if( Options & BSP_OPTIONS_MAKE_VIS_AREAS )
		RebuildDlg->m_Vis = JE_TRUE;
	else
		RebuildDlg->m_Vis = JE_FALSE;

	if( Options & BSP_OPTIONS_SOLID_FILL )
		RebuildDlg->m_Solid = JE_TRUE;
	else
		RebuildDlg->m_Solid = JE_FALSE;
	RebuildDlg->m_Logic = Logic;
	RebuildDlg->m_Balance = LogicBalance;

	Result = RebuildDlg->DoModal();
	if( Result != IDCANCEL )
	{
		Options = BSP_OPTIONS_MAKE_VIS_AREAS;

		if( RebuildDlg->GetCSG() )
			Options |= BSP_OPTIONS_CSG_BRUSHES;

		if( RebuildDlg->GetVis() )
			Options |= BSP_OPTIONS_MAKE_VIS_AREAS;

		if( RebuildDlg->GetSolid() )
			Options |= BSP_OPTIONS_SOLID_FILL;

		Logic = RebuildDlg->GetLogic();
		LogicBalance = RebuildDlg->GetBalance();

		Level_SetBSPBuildOptions( m_pLevel, Options, Logic, LogicBalance );
		if( Result == IDOK )
			Level_RebuildAll( m_pLevel, Options, Logic, LogicBalance ) ;
		else
			Level_RebuildBSP( m_pLevel, Options, Logic, LogicBalance );
		UpdateAllViews( nullptr, DOC_HINT_RENDERED ) ;

		UpdateStats();

//	tom morris feb 2005
		CMainFrame	*pMainFrm = nullptr;
		pMainFrm = (CMainFrame*)AfxGetMainWnd();
		if (pMainFrm)
			pMainFrm->SetStatusText("");
//	end tom morris feb 2005
	}
}// OnToolsRebuildall

void CJweDoc::OnUpdateToolsRebuildall(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable( m_pLevel != nullptr ) ;
}// OnUpdateToolsRebuildall

void CJweDoc::OnModeAdjust() 
{
	SetMode( MODE_POINTER_BB ) ;
}// OnModeAdjust

void CJweDoc::OnUpdateModeAdjust(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable( true ) ;
	pCmdUI->SetCheck( MODE_POINTER_BB == m_Mode ) ;
}// OnUpdateModeAdjust

void CJweDoc::OnModeRotateshear() 
{
	SetMode( MODE_POINTER_RS ) ;
}// OnModeRotateshear

void CJweDoc::OnUpdateModeRotateshear(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable( true ) ;
	pCmdUI->SetCheck( MODE_POINTER_RS == m_Mode ) ;
}// OnUpdateModeRotateshear
/*
void CJweDoc::OnModeVertex() 
{
	SetMode( MODE_POINTER_VM ) ;
}// OnModeVertex

void CJweDoc::OnUpdateModeVertex(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable( true ) ;
	pCmdUI->SetCheck( MODE_POINTER_VM == m_Mode ) ;
}// OnUpdateModeVertex
*/
void CJweDoc::OnModeFacemanipulation() 
{
	SetMode( MODE_POINTER_FM ) ;	
}// OnModeFacemanipulation

void CJweDoc::OnUpdateModeFacemanipulation(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable( true ) ;
	pCmdUI->SetCheck( MODE_POINTER_FM == m_Mode ) ;
}// OnUpdateModeFacemanipulation


void CJweDoc::OnAnim() 
{
	m_Anim_State = m_Anim_State ? 0:1;
	RenderAnimate( m_Anim_State );
}// OnModeAdjust

void CJweDoc::OnUpdateAnim(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable( true ) ;
	pCmdUI->SetCheck(m_Anim_State) ;

}// OnUpdateModeAdjust


// Added JH 7.3.2000
void CJweDoc::OnFullscreen() 
{
	// Switch to fullscreen
	char cFullscreenRes[400];
		// Get Screenmode setting
	Settings_GetJet_Fullscreen (cFullscreenRes,399);
		// if Screenmode not set, then start Screenmodeselectiondialog

	CView* pView{};

	pView = GetJetView();
	assert( pView != nullptr );

	( (CJetView*)pView )->SetFullscreenModeByString (cFullscreenRes);

	if ( ( (CJetView*)pView )->FullscreenView() == JE_FALSE )
	{
		jeErrorLog_AddString( JE_ERR_SUBSYSTEM_FAILURE, "CJweDoc::OnFullscreenView", "Failed to switch to full screen mode" );
		return ;
	}

}

void CJweDoc::OnUpdateFullscreen(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable( true ) ;
}


void CJweDoc::OnUpdateAll() 
{
	UpdateAll();
}

void CJweDoc::OnUpdateUpdateAll(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable( true ) ;
}

void CJweDoc::OnToolsUpdateSelection() 
{
	UpdateSelection();
}

void CJweDoc::OnUpdateToolsUpdateSelection(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable( true ) ;
}



void CJweDoc::OnUpdateEditAlignLeft(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable( true ) ;
}

void CJweDoc::OnEditAlignLeft() 
{
	AlignObjects (DOC_ALIGN_LEFT);
}



void CJweDoc::OnUpdateEditAlignRight(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable( true ) ;
}

void CJweDoc::OnEditAlignRight() 
{
	AlignObjects (DOC_ALIGN_RIGHT);
}



void CJweDoc::OnUpdateEditAlignBottom(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable( true ) ;
}

void CJweDoc::OnEditAlignBottom() 
{
	AlignObjects (DOC_ALIGN_BOTTOM);
}



void CJweDoc::OnUpdateEditAlignTop(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable( true ) ;
}

void CJweDoc::OnEditAlignTop() 
{
	AlignObjects (DOC_ALIGN_TOP);
}



void CJweDoc::OnEditRotR() 
{  RotateObjects (-90);
}

void CJweDoc::OnEditRotL() 
{  RotateObjects ( 90);
}

void CJweDoc::OnUpdateEditRotL(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable( true ) ;
}
void CJweDoc::OnUpdateEditRotR(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable( true ) ;
}


void CJweDoc::OnEditToFront() 
{
	ObjectsToFront();
}

void CJweDoc::OnUpdateEditToFront(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable( true ) ;
}

void CJweDoc::ObjectsToFront()
{
	CMainFrame		*	pMainFrm = (CMainFrame*)AfxGetMainWnd() ;

	ObjectList* SelList{};
	Object* pObject{};
	Object			**	pSaveSelObject{};

	ObjectIterator		Iterator{};
	int					iObjectNum=0;
	OBJECT_KIND		oKind{};

		// Count Objects selected
	SelList = Level_GetSelList ( m_pLevel );
	pObject = ObjectList_GetFirst ( SelList, &Iterator  );
	if (pObject == nullptr) return;

	while ( pObject )
	{   
		oKind = Object_GetKind(pObject);
		
		if ( oKind != KIND_BRUSH )
			{ AfxMessageBox( "This function is only usable with brushes ( Box,Sphere or Cylinder)", MB_OK|MB_ICONERROR, 0 ) ;
			  return;
			}
		iObjectNum ++;
		pObject = ObjectList_GetNext( SelList, &Iterator  );
	}

		// Alloc mem to save selected Objects
	pSaveSelObject = JE_RAM_ALLOCATE_ARRAY_CLEAR(Object*,iObjectNum+1);
	if (pSaveSelObject==nullptr) return;

		// Save Objects
	pSaveSelObject[0] = pObject = ObjectList_GetFirst( SelList, &Iterator  );
	if (pObject == nullptr) goto Free;
	iObjectNum=0;
	
	while (pObject)
	{	iObjectNum++;
		pObject = pSaveSelObject[iObjectNum] = ObjectList_GetNext( SelList, &Iterator  );
	}

	// Copy Objects
	if( JE_FALSE == Select_Dup (m_pLevel ) )
		goto Free; 		

	jeProperty_List *pArray;

	pArray = Select_BuildDescriptor( m_pLevel );
	pMainFrm->SetProperties( pArray );			

	// Deselect old Objects
	iObjectNum=0;
	while (pSaveSelObject[iObjectNum]!=nullptr)
	{
		Level_SelectObject(m_pLevel, pSaveSelObject[iObjectNum], LEVEL_DESELECT ) ;
		iObjectNum++;
	}
	
	// Add selected objects 
	pMainFrm->AddSelection( this ) ;

	iObjectNum=0;

	// Delete old objects
	while (pSaveSelObject[iObjectNum]!=0)
	{
		Level_DeleteObject(m_pLevel, pSaveSelObject[iObjectNum]) ;
		iObjectNum++;
	}

	// Update Lists
	pMainFrm->RemoveDeleted() ;
	pMainFrm->ResetProperties();

	// Update Views
	jeExtBox		WorldBounds ;
	Select_DeselectAll( m_pLevel, &WorldBounds ) ;	
	UpdateAllViews( nullptr, DOC_HINT_ALL, (CObject*)&WorldBounds ) ;
	UpdateAll();

	// Free Mem
Free:
	jeRam_Free( pSaveSelObject );

}


void CJweDoc::RotateObjects (jeFloat Angle )
{
	Ortho* pOrtho{};
	CMDIFrameWnd	*	pFrame = (CMDIFrameWnd*)AfxGetApp()->m_pMainWnd;
	CMDIChildWnd	*	pChild = (CMDIChildWnd *) pFrame->GetActiveFrame();
	CView			*	pView = pChild->GetActiveView();
	jeVec3d				Center3d{};
	ObjectList* SelList{};
	Object* pObject{};
	ObjectIterator		Iterator{};
	
	const jeExtBox* pSelWorldBounds{};
	jeExtBox			WorldBounds{};

	if (pView == nullptr) return;


	if(! pView->IsKindOf( RUNTIME_CLASS (CJweView))) return;

	pOrtho=((CJweView*)pView)->GetOrtho();
	if (pOrtho == nullptr) return;	
	
	SelList = Level_GetSelList( m_pLevel );
	pObject = ObjectList_GetFirst( SelList, &Iterator  );
	if (pObject == nullptr) return;

	pSelWorldBounds = Level_GetSelDrawBounds( m_pLevel ) ;
	jeExtBox_GetTranslation( pSelWorldBounds, &Center3d );

	Transform_AddSelectedUndo( m_pLevel, UNDO_ROTATE );

	Transform_RotateSelected
				( 
					m_pLevel, 
					Units_DegreesToRadians(Angle),
					Ortho_GetOrthogonalAxis( pOrtho ), 
					&Center3d,
					&WorldBounds
				) ;

	UpdateAllViews( nullptr, DOC_HINT_ALL, nullptr ) ;
	UpdateAllViews( nullptr, DOC_HINT_RENDERED, nullptr ) ;
}


void CJweDoc::AlignObjects (DOC_ALIGN_MODE Align_Mode )
{
	CMainFrame *	pMainFrm = (CMainFrame*)AfxGetMainWnd() ;
	ObjectList* SelList{};
	Object* pObject{};
	ObjectIterator  Iterator{};
	jeVec3d			Distance{};
	jeExtBox		DestObjectBounds{};
	jeExtBox		SourceObjectBounds{};
	Ortho* pOrtho{};

	CMDIFrameWnd *pFrame = (CMDIFrameWnd*)AfxGetApp()->m_pMainWnd;
	CMDIChildWnd *pChild = (CMDIChildWnd *) pFrame->GetActiveFrame();
	CView *pView = pChild->GetActiveView();

	//CView			*pView = GetParentFrame()->GetActiveView();
		//pMainFrm->GetActiveView();
	if (pView == nullptr) return;

	if(! pView->IsKindOf( RUNTIME_CLASS (CJweView))) return;

	pOrtho=((CJweView*)pView)->GetOrtho();
	if (pOrtho == nullptr) return;

	jeFloat XSource,  YSource,  ZSource;
	jeFloat XDest, YDest,  ZDest;

	jeVec3d_Set (&Distance,0,0,0);

	if (Ortho_GetViewType(pOrtho)==Ortho_ViewTop)
		{ if (Align_Mode==DOC_ALIGN_BOTTOM)
				Align_Mode=DOC_ALIGN_TOP;
			else if (Align_Mode==DOC_ALIGN_TOP)
				Align_Mode=DOC_ALIGN_BOTTOM;
		}

	SelList = Level_GetSelList( m_pLevel );
	pObject = ObjectList_GetFirst( SelList, &Iterator  );

	if (pObject == nullptr) return;
	
	Transform_AddSelectedUndo( m_pLevel, UNDO_MOVE );


	Object_GetWorldAxialBounds (pObject,&DestObjectBounds);	
	HasSelections (&DestObjectBounds);

	while( pObject != nullptr )
	{
		Object_GetWorldAxialBounds (pObject,&SourceObjectBounds);

		if ((Align_Mode==DOC_ALIGN_LEFT)||
			(Align_Mode==DOC_ALIGN_BOTTOM) )
			{ jeVec3d_Get(&SourceObjectBounds.Min, &XSource, &YSource, &ZSource);
			  jeVec3d_Get(&DestObjectBounds.Min,   &XDest  , &YDest  , &ZDest);
			}

		if ((Align_Mode==DOC_ALIGN_RIGHT)||
			(Align_Mode==DOC_ALIGN_TOP) )
			{ jeVec3d_Get(&SourceObjectBounds.Max, &XSource, &YSource, &ZSource);
			  jeVec3d_Get(&DestObjectBounds.Max,   &XDest  , &YDest  , &ZDest);
			}

		if ((Align_Mode==DOC_ALIGN_LEFT)||
		    (Align_Mode==DOC_ALIGN_RIGHT) )
	    if (Ortho_GetViewType(pOrtho)==Ortho_ViewFront)
			{	ZDest=ZSource;YDest=YSource;
			}
	    else if (Ortho_GetViewType(pOrtho)==Ortho_ViewSide)
			{	XDest=XSource;YDest=YSource;
			}
	    else if (Ortho_GetViewType(pOrtho)==Ortho_ViewTop)
			{	ZDest=ZSource;YDest=YSource;
			}

		if ((Align_Mode==DOC_ALIGN_TOP)||
		    (Align_Mode==DOC_ALIGN_BOTTOM) )
	    if (Ortho_GetViewType(pOrtho)==Ortho_ViewFront)
			{	ZDest=ZSource;XDest=XSource;
			}
	    else if (Ortho_GetViewType(pOrtho)==Ortho_ViewSide)
			{	XDest=XSource;ZDest=ZSource;
			}
	    else if (Ortho_GetViewType(pOrtho)==Ortho_ViewTop)
			{	XDest=XSource;YDest=YSource;
			}

		jeVec3d_Set(&DestObjectBounds.Min,   XDest  , YDest  , ZDest);

		if ((Align_Mode==DOC_ALIGN_LEFT)||
			(Align_Mode==DOC_ALIGN_BOTTOM) )
		jeVec3d_Subtract (&DestObjectBounds.Min,&SourceObjectBounds.Min,&Distance);

		if ((Align_Mode==DOC_ALIGN_RIGHT)||
			(Align_Mode==DOC_ALIGN_TOP) )
		jeVec3d_Subtract (&DestObjectBounds.Min,&SourceObjectBounds.Max,&Distance);

		Object_Move (  pObject, &Distance );
		pObject = ObjectList_GetNext( SelList, &Iterator  );
	}

	UpdateAllViews( nullptr, DOC_HINT_ALL, nullptr ) ;
	UpdateAllViews( nullptr, DOC_HINT_RENDERED, nullptr ) ;

}

// EOF JH



// EOF JH


MODE CJweDoc::SetMode( const MODE eMode )
{
	MODE			OldMode{};

	OldMode = m_Mode ;
	if( OldMode != eMode )
	{
		// Do old mode closure

		// CJP : If the previous mode was vertex manipulation then we need to deslect the vertices
		// or we will assert when they select a face.

		if( OldMode == MODE_POINTER_VM) 
			Select_DeselectAllVerts(m_pLevel);

		// New mode
		if( !isPlaceBrushMode() && !isPlaceLightMode() )
			m_PrevMode = m_Mode;
		m_Mode = eMode ;
		switch( m_Mode )
		{
		case MODE_POINTER_FM :
			Select_AllFaces( m_pLevel );
			break ;

		case	MODE_POINTER_RS:
		case	MODE_POINTER_VM:
		case	MODE_POINTER_BB :
			break ;
		}
	}
	
	// Should hint mode change
	UpdateAllViews( nullptr ) ;

	return OldMode ;
}// SetMode

jeBoolean CJweDoc::IsVertexManipulationMode()
{
	return (MODE_POINTER_VM == m_Mode) ? JE_TRUE : JE_FALSE ;
}//IsVertexManipulationMode

void CJweDoc::EndMoveHandle()
{
	Select_DragEnd( m_pLevel ) ;
	UpdateAllViews( nullptr, DOC_HINT_RENDERED, nullptr ) ;
}

void CJweDoc::EndRotateSub()
{
	CMainFrame *	pMainFrm = (CMainFrame*)AfxGetMainWnd() ;
	ObjectList* SubSelList{};
	Object* pObject{};
	ObjectIterator  Iterator{};

	Select_DragEndSub( m_pLevel ) ;

	SubSelList = Level_GetSubSelList( m_pLevel );
	pObject = ObjectList_GetFirst( SubSelList, &Iterator  );
	while( pObject != nullptr )
	{
		pMainFrm->EndRotateSub(  pObject );
		pObject = ObjectList_GetNext( SubSelList, &Iterator  );
	}
	UpdateAllViews( nullptr, DOC_HINT_RENDERED, nullptr ) ;
}

void CJweDoc::OnOptionsSnaptogrid() 
{
	Level_SetSnapGrid( m_pLevel, !Level_IsSnapGrid( m_pLevel ) ) ;
}// OnOptionsSnaptogrid

void CJweDoc::OnUpdateOptionsSnaptogrid(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable( true ) ;
	pCmdUI->SetCheck( Level_IsSnapGrid( m_pLevel ) ) ;
}// OnUpdateOptionsSnaptogrid


/*void CJweDoc::OnOptionsGrid() 
{
	CGridSettings	GridSettingsDialog ;

	// Added by cjp
	GridSettingsDialog.m_bShouldSnapVerts = Level_GetShouldSnapVerts( m_pLevel );
	// end added by cjp

	GridSettingsDialog.m_nSnapSize = Level_GetGridSnapSize( m_pLevel ) ;
	GridSettingsDialog.m_DegreeSnap = Level_GetRotateSnapSize( m_pLevel ) ;
	if( IDOK == GridSettingsDialog.DoModal() )
	{
		// added by cjp
		Level_SetShouldSnapVerts( m_pLevel, GridSettingsDialog.m_bShouldSnapVerts ) ;
		// end added by cjp

		Level_SetGridSnapSize( m_pLevel, GridSettingsDialog.m_nSnapSize ) ;
		Level_SetRotateSnapSize( m_pLevel, GridSettingsDialog.m_DegreeSnap ) ;
		UpdateAllViews( nullptr, DOC_HINT_ORTHO ) ;
	}
}// OnOptionsGrid

void CJweDoc::OnUpdateOptionsGrid(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable( true ) ;
}// OnUpdateOptionsGrid
*/

void CJweDoc::UpdateAllViews(CView* pSender, LPARAM lHint, CObject* pHint)
{
	POSITION	pos{};
	CView* pView{};
	
	if( lHint == DOC_HINT_NONE )
		CDocument::UpdateAllViews( pSender, lHint, pHint ) ;
	else
	{
		pos = GetFirstViewPosition();

		switch( lHint )
		{
		case DOC_HINT_ORTHO :
			while( pos != nullptr )
			{
				pView = GetNextView(pos);
				ASSERT_VALID(pView);
				if( pView != pSender && pView->IsKindOf( RUNTIME_CLASS (CJweView)) )
					((CJweView*)pView)->OnUpdate(pSender, lHint, pHint);
			}				
			break ;

		case DOC_HINT_RENDERED :
			while( pos != nullptr )
			{
				pView = GetNextView(pos);
				ASSERT_VALID(pView);
				if( pView != pSender && pView->IsKindOf( RUNTIME_CLASS (CJetView)))
					((CJetView*)pView)->OnUpdate(pSender, lHint, pHint);
			}
			// Update/Rebuild the selected objects so the changes
			// appear in the 3D window
			//   --- Cyrius, Incarnadine, CJP
			if(m_pLevel != nullptr)
			{
				if(IsVertexManipulationMode() == JE_FALSE) //cyrius (this fixes Chrisjp's bug)
					Level_UpdateSelected(m_pLevel); // Incarnadine
			}
			break ;

		case DOC_HINT_ALL :
			while( pos != nullptr )
			{
				pView = GetNextView(pos);
				ASSERT_VALID(pView);
				if( pView != pSender )
					((CJetView*)pView)->OnUpdate(pSender, lHint, pHint);
			}
			break ;

		default :
			ASSERT( 0 ) ;
			break ;
		}// Switch 

	}

}// UpdateAllViews


BOOL CJweDoc::OnSaveDocument(LPCTSTR lpszPathName) 
{
	jeVFile *	pFS = nullptr ;
	jeVFile	*	pF = nullptr ;	// File Fork (Editor or Jet3D)
	CString		cstr ;
	CString		backupext;
	char		JustPath[MAX_PATH];
	char		JustName[MAX_PATH];
	char		TempName[MAX_PATH];
	jePtrMgr	*pPtrMgr = nullptr;
	
	int32 Signature = SIGNATURE;
	float Version= DOC_VERSION;
	
	pPtrMgr = jePtrMgr_Create();
	if( pPtrMgr == nullptr )
	{
		jeErrorLog_Add( JE_ERR_SUBSYSTEM_FAILURE, "OnSaveDocument:jePtrMgr_Create");
		goto SAVE_DOC_ERR;
	}

	Level_PrepareForSave( m_pLevel ) ;
	
	strcpy(JustPath, lpszPathName);

	Util_DriveAndPathOnly(JustPath);
	Util_StripTrailingBackslash(JustPath);

	strcpy(JustName, lpszPathName);
	Util_NameOnly(JustName);

	if( !backupext.LoadString(IDS_TEMP_PREFIX) )
	{
		jeErrorLog_Add( JE_ERR_SUBSYSTEM_FAILURE, "OnSaveDocument:backupext.LoadString");
		goto SAVE_DOC_ERR;
	}

	if( GetTempFileName(JustPath, backupext, 0, TempName) == 0 )
	{
		jeErrorLog_Add( JE_ERR_SUBSYSTEM_FAILURE, "OnSaveDocument:GetTempFileName");
		goto SAVE_DOC_ERR;
	}

	// Create a new file system
	pFS = jeVFile_OpenNewSystem
	(
		nullptr, 
		JE_VFILE_TYPE_VIRTUAL,
		TempName,
		nullptr,
		JE_VFILE_OPEN_CREATE|JE_VFILE_OPEN_DIRECTORY
	);
	if( pFS == nullptr )
	{
		cstr.Format( IDS_CANTOPENFILE, TempName ) ;
		AfxMessageBox( cstr, MB_OK|MB_ICONERROR, 0 ) ;
		jeErrorLog_AddString( JE_ERR_FILEIO_OPEN, "OnSaveDocument:jeVFile_OpenNewSystem", TempName);
		goto SAVE_DOC_ERR;
	}

	// Setup Error Writing message
	cstr.Format( IDS_ERRORWRITING, TempName ) ;

	// When writing compound v-files, each component must be opened, written
	// and closed before proceeding to the next
	// JET FORK
	
	pF = jeVFile_Open( pFS, "Version", JE_VFILE_OPEN_CREATE ) ;
	if( pF == nullptr )
	{
		jeVFile_Close( pFS ) ;
		jeErrorLog_AddString( JE_ERR_FILEIO_FORMAT, "OnSaveDocument:jeVFile_Open", lpszPathName);
		ReportErrors( JE_FALSE );
		return false ;
	}
	if( jeVFile_Write( pF, &Signature, sizeof Signature ) == JE_FALSE )
	{
		jeVFile_Close( pFS ) ;
		jeErrorLog_AddString(JE_ERR_FILEIO_WRITE, "OnSaveDocument:jeVFile_Write", lpszPathName);
		return JE_FALSE;
	}

	if( jeVFile_Write( pF, &Version, sizeof Version ) == JE_FALSE )
	{
		jeVFile_Close( pFS ) ;
		jeErrorLog_AddString(JE_ERR_FILEIO_WRITE, "OnSaveDocument:jeVFile_Write", lpszPathName);
		return JE_FALSE;
	}
	jeVFile_Close( pF ) ;

	pF = jeVFile_Open( pFS, "Jet3D", JE_VFILE_OPEN_CREATE);
	if( pF == nullptr )
	{
		AfxMessageBox( cstr, MB_OK|MB_ICONERROR, 0 ) ;
		jeErrorLog_AddString( JE_ERR_FILEIO_OPEN, "OnSaveDocument:jeVFile_OpenNewSystem", "Jet3D");
		goto SAVE_DOC_ERR;
	}
	

	if( jeWorld_WriteToFile( m_pWorld, pF, pPtrMgr ) == JE_FALSE )
	{
		AfxMessageBox( cstr, MB_OK|MB_ICONERROR, 0 ) ;	// Error Writing
		jeErrorLog_AddString( JE_ERR_FILEIO_WRITE, "OnSaveDocument:jeWorld_WriteToFile", "Jet3D");
		goto SAVE_DOC_ERR;
	}

	if( jeVFile_Close( pF ) == JE_FALSE )	// Close the Jet3D fork
	{
		jeVFile_Close( pFS ) ;	
		AfxMessageBox( cstr, MB_OK|MB_ICONERROR, 0 ) ;	// Error Writing
		jeErrorLog_AddString( JE_ERR_FILEIO_CLOSE, "OnSaveDocument:jeVFile_Close", "Jet3D");
		goto SAVE_DOC_ERR;
	}

	// EDITOR FORK

	pF = jeVFile_Open( pFS, "Editor", JE_VFILE_OPEN_CREATE);
	if( pF == nullptr )
	{
		AfxMessageBox( cstr, MB_OK|MB_ICONERROR, 0 ) ;
		jeErrorLog_AddString( JE_ERR_FILEIO_OPEN, "OnSaveDocument:jeVFile_OpenNewSystem", "Jet3D");
		goto SAVE_DOC_ERR;
	}

	// Write the Editor fork
	if( Level_WriteToFile( m_pLevel, pF, pPtrMgr ) == JE_FALSE )
	{
		AfxMessageBox( cstr, MB_OK|MB_ICONERROR, 0 ) ;	// Error Writing
		jeErrorLog_AddString( JE_ERR_FILEIO_WRITE, "OnSaveDocument:jeWorld_WriteToFile", "Jet3D");
		goto SAVE_DOC_ERR;
	}

	if( jeVFile_Close( pF ) == JE_FALSE ) // Close the Editor fork
	{
		pF = nullptr;
		AfxMessageBox( cstr, MB_OK|MB_ICONERROR, 0 ) ;	// Error Writing
		jeErrorLog_AddString( JE_ERR_FILEIO_CLOSE, "OnSaveDocument:jeVFile_Close", "Jet3D");
		goto SAVE_DOC_ERR;
	}


	// Added JH 12.3.2000
	// LevelProperties FORK

	pF = jeVFile_Open( pFS, "LevelProperties", JE_VFILE_OPEN_CREATE);
	if( pF == nullptr )
	{
		AfxMessageBox( cstr, MB_OK|MB_ICONERROR, 0 ) ;
		jeErrorLog_AddString( JE_ERR_FILEIO_OPEN, "OnSaveDocument:jeVFile_OpenNewSystem LevelProperties", "Jet3D");
		goto SAVE_DOC_ERR;
	}

	if( m_pPropsDialog->Properties_WriteToFile( pF, pPtrMgr ) == JE_FALSE )
	{
		AfxMessageBox( cstr, MB_OK|MB_ICONERROR, 0 ) ;	// Error Writing
		jeErrorLog_AddString( JE_ERR_FILEIO_WRITE, "OnSaveDocument:jeWorld_WriteToFile LevelProperties", "Jet3D");
		goto SAVE_DOC_ERR;
	}

	if( jeVFile_Close( pF ) == JE_FALSE ) // Close the LevelProperties fork
	{
		pF = nullptr;
		AfxMessageBox( cstr, MB_OK|MB_ICONERROR, 0 ) ;	// Error Writing
		jeErrorLog_AddString( JE_ERR_FILEIO_CLOSE, "OnSaveDocument:jeVFile_Close LevelProperties", "Jet3D");
		goto SAVE_DOC_ERR;
	}


	// Added JH 12.3.2000
	// LevelThumbnail FORK
	if (Settings_GetGlobal_Thumbnail())
	{

		pF = jeVFile_Open( pFS, "LevelThumbnail", JE_VFILE_OPEN_CREATE);
		if( pF == nullptr )
		{
			AfxMessageBox( cstr, MB_OK|MB_ICONERROR, 0 ) ;
			jeErrorLog_AddString( JE_ERR_FILEIO_OPEN, "OnSaveDocument:jeVFile_OpenNewSystem LevelThumbnail", "Jet3D");
			goto SAVE_DOC_ERR;
		}

		CJetView * pJetView;
		pJetView = (CJetView *)GetJetView();
		Render(pJetView);

		if (WriteWindowToDIB (pF, pPtrMgr, pJetView)==JE_FALSE)
		{
			AfxMessageBox( cstr, MB_OK|MB_ICONERROR, 0 ) ;	// Error Writing
			jeErrorLog_AddString( JE_ERR_FILEIO_CLOSE, "OnSaveDocument:jeWorld_WriteToFile LevelThumbnail", "Jet3D");
			goto SAVE_DOC_ERR;
		}

		if( jeVFile_Close( pF ) == JE_FALSE ) // Close the LevelThumbnail fork
		{
			pF = nullptr;
			AfxMessageBox( cstr, MB_OK|MB_ICONERROR, 0 ) ;	// Error Writing
			jeErrorLog_AddString( JE_ERR_FILEIO_CLOSE, "OnSaveDocument:jeVFile_Close LevelThumbnail", "Jet3D");
			goto SAVE_DOC_ERR;
		}
	}


	// Close the Compound file
	if( jeVFile_Close( pFS ) == JE_FALSE )
	{
		pFS = nullptr;
		cstr.Format( IDS_ERRORCLOSING, lpszPathName, 0 ) ;
		AfxMessageBox( cstr, MB_OK|MB_ICONERROR, 0 ) ;	// Error Closeing
		jeErrorLog_AddString( JE_ERR_FILEIO_CLOSE, "OnSaveDocument:jeVFile_Close", "Jet3D");
		goto SAVE_DOC_ERR;
	}

	pFS = jeVFile_OpenNewSystem	// Open the directory with the file
	(
		nullptr, 
		JE_VFILE_TYPE_DOS,
		JustPath,
		nullptr,
		JE_VFILE_OPEN_UPDATE|JE_VFILE_OPEN_DIRECTORY
	);
	if( pFS == nullptr )
	{
		jeErrorLog_AddString( JE_ERR_FILEIO_OPEN, "OnSaveDocument:jeVFile_OpenNewSystem", JustPath);
		goto SAVE_DOC_ERR;
	}
	// Check for a backup file...
	if( !backupext.LoadString(IDS_BAK_EXT) )
	{
		jeErrorLog_Add( JE_ERR_SUBSYSTEM_FAILURE, "OnSaveDocument:jePtrMgr_Create");
		goto SAVE_DOC_ERR;
	}

	strcpy( JustPath, JustName ) ;
	Util_NewExtension( JustPath, backupext ) ;


	// Added JH 12.3.2000
	if (Settings_GetGlobal_BackupFile())
		{	if( jeVFile_FileExists( pFS, JustPath) )
			{
				if( !jeVFile_DeleteFile( pFS, JustPath))
				{
					jeErrorLog_AddString( JE_ERR_FILEIO_WRITE, "OnSaveDocument:jeVFile_DeleteFile", JustPath);
					goto SAVE_DOC_ERR;
				}
			}
		// If name.glf exists, Rename existing file to name.bak
			if( jeVFile_FileExists( pFS, JustName ) )
				if( !jeVFile_RenameFile( pFS, JustName, JustPath))
				{
					jeErrorLog_AddString( JE_ERR_FILEIO_READ, "OnSaveDocument:jeVFile_RenameFile", JustPath);
					goto SAVE_DOC_ERR;
				}
		}
	else
		{
		  if( jeVFile_FileExists( pFS, JustName) )		
			if( !jeVFile_DeleteFile( pFS, JustName))
			{
				jeErrorLog_AddString( JE_ERR_FILEIO_WRITE, "OnSaveDocument:jeVFile_DeleteFile", TempName);
				goto SAVE_DOC_ERR;
			}
		}

	
	Util_NameOnly( TempName ) ;		// Rename our temp file to normal
	if( !jeVFile_RenameFile(pFS, TempName, JustName) )
	{
		jeErrorLog_AddString( JE_ERR_FILEIO_READ, "OnSaveDocument:jeVFile_RenameFile", JustPath);
		goto SAVE_DOC_ERR;
	}
	jeVFile_Close( pFS ) ;

	SetModifiedFlag( false ) ;
	jePtrMgr_Destroy( &pPtrMgr );
	return TRUE ;

SAVE_DOC_ERR:
	if( pPtrMgr != nullptr )
		jePtrMgr_Destroy( &pPtrMgr );
	if( pFS != nullptr )
		jeVFile_Close( pFS ) ;
	if( pF != nullptr )
		jeVFile_Close( pF ) ;
	ReportErrors( JE_FALSE );
	return FALSE;

}// OnSaveDocument


BOOL CJweDoc::OnOpenDocument(LPCTSTR lpszPathName) 
{
	CJweApp			*	App = (CJweApp*)AfxGetApp();
	jeVFile			*	pFS = nullptr ;
	jeVFile			*	pF = nullptr ;	// File Fork (Editor or Jet3D)
	CString				Message ;
	jeWorld			* pNewWorld{};
	Level			* pNewLevel{};
	CMainFrame* pMainFrm{};
	pMainFrm = (CMainFrame*)AfxGetMainWnd();
	jePtrMgr	*pPtrMgr = nullptr;
	jeResourceMgr	* pResourceMgr{};
	jeBSP_Options Options = 0;
	jeBSP_Logic Logic{};
	jeBSP_LogicBalance LogicBalance{};
	CJetView* pJetView{};
	pJetView = (CJetView*)GetJetView();


	int32 Signature{};
	float Version{};


	// Create a new file system
	pPtrMgr = jePtrMgr_Create();
	if( pPtrMgr == nullptr )
	{
		jeErrorLog_Add( JE_ERR_SUBSYSTEM_FAILURE, "OnSaveDocument:jePtrMgr_Create");
		ReportErrors(JE_FALSE);
		return false;
	}
	pMainFrm->ResetLists();
	//Set Invalid
	SetNewBrushBoundInvalid();

	pFS = jeVFile_OpenNewSystem
	(
		nullptr, 
		JE_VFILE_TYPE_VIRTUAL,
		lpszPathName,
		nullptr,
		JE_VFILE_OPEN_READONLY|JE_VFILE_OPEN_DIRECTORY
	);
	if( pFS == nullptr )
	{
		jeErrorLog_AddString( JE_ERR_FILEIO_OPEN, "OnOpenDocument:jeVFile_OpenNewSystem", lpszPathName);
		ReportErrors( JE_FALSE);
		return false ;
	}

#ifdef _USE_BITMAPS
	LightBitmap	= InitBitmap( IDR_LIGHT );
#else
	LightBitmap	= InitMaterial( IDR_LIGHT );
#endif
	if( LightBitmap == nullptr )
	{
		ReportErrors( JE_FALSE);
		return FALSE ;
	}

	DWORD errorVal = GetLastError();
	errorVal;
	
	Message.Format( IDS_ERRORREADINGFILE, lpszPathName ) ;

	pF = jeVFile_Open( pFS, "Version", JE_VFILE_OPEN_READONLY ) ;
	if( pF == nullptr )
	{
		jeVFile_Close( pFS ) ;
		jeErrorLog_AddString( JE_ERR_FILEIO_FORMAT, "OnOpenDocument:jeVFile_Open", lpszPathName);
		//ReportErrors( JE_FALSE );
		return false ;
	}
	if( jeVFile_Read( pF, &Signature, sizeof Signature ) == JE_FALSE )
	{
		jeVFile_Close( pFS ) ;
		jeErrorLog_AddString(JE_ERR_FILEIO_WRITE, "OnOpenDocument:jeVFile_Read", lpszPathName);
		return JE_FALSE;
	}
	if( Signature != SIGNATURE )
	{
		jeVFile_Close( pFS ) ;
		jeErrorLog_AddString(JE_ERR_FILEIO_VERSION, "OnOpenDocument:Signature", lpszPathName);
		return JE_FALSE;
	}

	if( jeVFile_Read( pF, &Version, sizeof Version ) == JE_FALSE )
	{
		jeVFile_Close( pFS ) ;
		jeErrorLog_AddString(JE_ERR_FILEIO_WRITE, "OnOpenDocument:jeVFile_Read", lpszPathName);
		return JE_FALSE;
	}
	if( !(Version == DOC_VERSION || Version == DOC_OLDVERSION ) )
	{
		jeVFile_Close( pFS ) ;
		jeErrorLog_AddString(JE_ERR_FILEIO_VERSION, "OnOpenDocument:Version", lpszPathName);
		return JE_FALSE;
	}
	jeVFile_Close( pF ) ;

	// Open the Jet3D Fork
	pF = jeVFile_Open( pFS, "Jet3D", JE_VFILE_OPEN_READONLY) ;
	if( pF == nullptr )
	{
		jeVFile_Close( pFS ) ;
		jeErrorLog_AddString( JE_ERR_FILEIO_FORMAT, "OnOpenDocument:jeVFile_Open", lpszPathName);
		ReportErrors( JE_FALSE );
		return false ;
	}
	
	pResourceMgr = Level_CreateResourceMgr(pJetView->GetEngine());
	if( pResourceMgr == nullptr )
		return( FALSE );
	pNewWorld = jeWorld_CreateFromFile( pF, pPtrMgr, pResourceMgr );

	jeVFile_Close( pF ) ;
	if( pNewWorld == nullptr )
	{
		jeVFile_Close( pFS ) ;
		jeErrorLog_AddString( JE_ERR_SUBSYSTEM_FAILURE, "OnOpenDocument:jeWorld_CreateFromFile", lpszPathName);
		ReportErrors(JE_FALSE);
		return false ;
	}

	// Open the Editor Fork
	pF = jeVFile_Open( pFS, "Editor", JE_VFILE_OPEN_READONLY ) ;
	if( pF == nullptr )
	{
		jeVFile_Close( pFS ) ;
		jeErrorLog_AddString( JE_ERR_FILEIO_FORMAT, "OnOpenDocument:jeVFile_Open", lpszPathName);
		ReportErrors(JE_FALSE);
		return false ;
	}

	pNewLevel = Level_CreateFromFile( pF, pNewWorld, App->GetMaterialList(),  pPtrMgr, Version ) ;
	jeVFile_Close( pF ) ;
	if( pNewLevel == nullptr )
	{
		jeVFile_Close( pFS ) ;
		jeErrorLog_AddString( JE_ERR_SUBSYSTEM_FAILURE, "OnOpenDocument:Level_CreateFromFile", lpszPathName);
		ReportErrors(JE_FALSE);
		return false ;
	}

	// Added JH 12.3.2000
	// LEVEL Info FORK

	pF = jeVFile_Open( pFS, "LevelProperties", JE_VFILE_OPEN_READONLY);
	if( pF != nullptr )
	{
		if( m_pPropsDialog->Properties_ReadFromFile( pF, pPtrMgr ) == JE_FALSE )
		{
			jeVFile_Close( pFS ) ;
			jeErrorLog_AddString( JE_ERR_SUBSYSTEM_FAILURE, "OnOpenDocument:LevelProperties", lpszPathName);
			ReportErrors(JE_FALSE);
			return false ;
		}

		if( jeVFile_Close( pF ) == JE_FALSE ) // Close the Editor fork
		{
			jeVFile_Close( pFS ) ;
			jeErrorLog_AddString( JE_ERR_SUBSYSTEM_FAILURE, "OnOpenDocument:LevelProperties", lpszPathName);
			ReportErrors(JE_FALSE);
			return false ;
		}
	}

	jeVFile_Close( pFS ) ;

	DeleteContents() ;
	m_pWorld = pNewWorld ;
	m_pLevel = pNewLevel ;
	jePtrMgr_Destroy( &pPtrMgr );
	jeWorld_AttachSoundSystem( m_pWorld, pMainFrm->GetSoundSystem() );
	Level_GetBSPBuildOptions( m_pLevel, &Options, &Logic, &LogicBalance );

//	tom morris	feb 2005 -- necessary to ensure VIS areas are present
//	otherwise actors may not be visible.
	if ((Options & BSP_OPTIONS_MAKE_VIS_AREAS) == 0)
	{
		Options |= BSP_OPTIONS_MAKE_VIS_AREAS;
		Level_SetBSPBuildOptions(m_pLevel, Options, Logic, LogicBalance);
	}
//	end tom morris feb 2005

	Level_RebuildAll( m_pLevel, Options, Logic, LogicBalance ) ;
	m_bLoaded = JE_TRUE;
	return true ;
}// OnOpenDocument


void CJweDoc::OnEditUndo() 
{
	Undo* pUndo{};
	int Type{};

	jeProperty_List *pArray;

	pUndo = Level_GetUndo( m_pLevel );
	Type = Undo_GetTopType( pUndo );
	Undo_Pop( pUndo, Level_GetBrushLighting( m_pLevel ) );
	if( Type == UNDO_CREATE ||
		Type == UNDO_DELETE )
	{
		CMainFrame *	pMainFrm = (CMainFrame*)AfxGetMainWnd() ;
		pMainFrm->RebuildLists( this ); //This is done to rebuild lists
		pArray = Select_BuildDescriptor( m_pLevel );			
		if( pArray )
		{
			pMainFrm->UpdateProperties(pArray );
			jeProperty_ListDestroy( &pArray );
		}
		else
			pMainFrm->ResetProperties();
	}

	UpdateAllViews( nullptr ) ;
}

#define UNDOREDOLENGTH	(10)	// Enough room for 'undo' mbcs
void CJweDoc::OnUpdateEditUndo(CCmdUI* pCmdUI) 
{
	int32		nID{};
	jeBoolean	bEnable{};
	char		szMessage[UNDO_MAX_STRING+UNDOREDOLENGTH] ;
	char		szBuffer[UNDO_MAX_STRING] ;

	bEnable = Undo_CanUndo( Level_GetUndo( m_pLevel ), &nID );
	pCmdUI->Enable( bEnable ) ;

	Util_GetRcString( szMessage, IDS_UNDO ) ;
	if( bEnable )
	{
		strcat( szMessage, Util_GetRcString( szBuffer, nID ) ) ;
	}
	strcat( szMessage, Util_GetRcString( szBuffer, IDS_UNDOACCEL ) );
	pCmdUI->SetText( szMessage ) ;
	
}//OnUpdateEditUndo

/*
void CJweDoc::OnEditRedo() 
{
	// TODO: Add your command handler code here
	
}

void CJweDoc::OnUpdateEditRedo(CCmdUI* pCmdUI) 
{
#pragma message ("Brian: Need ? Level_CanRedo( &string )" )
	pCmdUI->Enable( true ) ;
}//OnUpdateEditUndo
*/

void CJweDoc::DeleteSelection()
{
	jeExtBox	WorldBounds{};
	CMainFrame *	pMainFrm = (CMainFrame*)AfxGetMainWnd() ;

	Select_Delete( m_pLevel, &WorldBounds ) ;
	pMainFrm->RemoveDeleted(  ) ;
	pMainFrm->ResetProperties();
	UpdateAllViews( nullptr, DOC_HINT_ALL, (CObject*)&WorldBounds ) ;

}
void CJweDoc::OnEditClear() 
{
	DeleteSelection();
}// OnEditClear (Delete)

void CJweDoc::OnUpdateEditClear(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable( Level_HasSelections( m_pLevel ) ) ;
}// OnUpdateEditClear (Delete)

void CJweDoc::BeginSize()
{

}// BeginSize

void CJweDoc::BeginRotate()
{
	CMainFrame *pMainFrm = nullptr;
	pMainFrm = (CMainFrame*)AfxGetMainWnd();

	m_LastRotateAngle = 0.0f;

}// BeginRotate

void CJweDoc::BeginShear()
{

}// BeginShear


void CJweDoc::ApplyMaterial( void )
{
	Level_SetChanged( m_pLevel, JE_TRUE );
	Select_ApplyCurMaterial( m_pLevel ) ;
	UpdateAllViews( nullptr, DOC_HINT_RENDERED ) ;
}// ApplyMaterial


Model * CJweDoc::CreateModel(const char *pszName)
{
	Model* pModel{};
	CMainFrame *	pMainFrm = (CMainFrame*)AfxGetMainWnd() ;

	pModel =  Level_AddModel( m_pLevel, pszName );
	if( pModel )
	{
		pMainFrm->AddObject( (Object *)pModel );
	}
	return( pModel );
}// CreateModel

Class *	CJweDoc::CreateClass( const char * pszName, int Kind )
{
	return( Level_AddClass( m_pLevel, pszName, Kind ) );
}

void CJweDoc::ModelLock( Model * pModel, jeBoolean bLock )
{
	Level_ModelLock( m_pLevel, pModel, bLock );
}

const char * CJweDoc::GetSelectionName(int32 * pnNumber)
{
	return Select_GetName( m_pLevel, pnNumber ) ;
}// GetSelectionName

void CJweDoc::SetSelectionName(const char * pName)
{
	ASSERT( pName != nullptr ) ;
	Select_SetName( m_pLevel, pName ) ;
}// SetSelectionName

ModelList * CJweDoc::GetModelList( void )
{
	return Level_GetModelList( m_pLevel ) ;
}// GetModelList

LightList * CJweDoc::GetLightList( void )
{
	return Level_GetLightList( m_pLevel ) ;
}// GetLightList

CameraList * CJweDoc::GetCameraList( void )
{
	return Level_GetCameraList( m_pLevel ) ;
}// GetLightList


GroupList * CJweDoc::GetGroupList( void )
{
	return Level_GetGroupList( m_pLevel ) ;
}// GetGroupList

ObjectList * CJweDoc::GetSelectList( void )
{
	return Level_GetSelList( m_pLevel ) ;
}// GetSelectList

jeBoolean CJweDoc::EnumSelected(void *lParam, ObjectListCB Callback)
{
	// This functions is used by Lists.cpp, which has it's own callback
	return Level_EnumSelected( m_pLevel, lParam, Callback ) ;
}// EnumSelected

jeBoolean CJweDoc::EnumObjects(void *lParam, ObjectListCB Callback)
{
	// This functions is used by Lists.cpp, which has it's own callback
	return Level_EnumObjects( m_pLevel, lParam, Callback ) ;
}// EnumSelected




void CJweDoc::CenterViewsOnSelection(  )
{
	POSITION	pos{};
	CView* pView{};
	jeExtBox	SelBounds{};
	jeVec3d		Center{};

	if( !HasSelections( &SelBounds ) )
		return;
	jeExtBox_GetTranslation( &SelBounds, &Center );


	pos = GetFirstViewPosition();

	while( pos != nullptr )
	{
		pView = GetNextView(pos);
		ASSERT_VALID(pView);
		if( pView->IsKindOf( RUNTIME_CLASS (CJweView)) )
			((CJweView*)pView)->SetCameraPos( &Center );
		if( pView->IsKindOf( RUNTIME_CLASS (CJetView)) )
			((CJetView*)pView)->SetCameraPos( &Center );
	}
}

Group * CJweDoc::AddGroup( const char * pszName )
{
	return( Level_AddGroup( m_pLevel, pszName ) );
}


Group *	CJweDoc::GetCurrentGroup( void )
{
	return( Level_GetCurrentGroup( m_pLevel ) );
}

void	CJweDoc::SetCurrentGroup( Group * pGroup )
{
	Level_SetCurrentGroup( m_pLevel, pGroup );
}

Model *	CJweDoc::GetCurrentModel( void )
{
	ASSERT( m_pLevel );

	return(Level_GetCurModel( m_pLevel ) );
}
void CJweDoc::SetCurrentModel( Model * pModel )
{
	Level_SetCurrentModel( m_pLevel, pModel );
}

void CJweDoc::OnToolsBuildlights() 
{
	Level_RebuildLights( m_pLevel );
	UpdateAllViews( nullptr, DOC_HINT_RENDERED ) ;
}

void CJweDoc::RebuildLights(  )
{
	Level_RebuildLights( m_pLevel );
	UpdateAllViews( nullptr, DOC_HINT_RENDERED ) ;
}

void CJweDoc::SetProperty( int DataId, int DataType, jeProperty_Data * pData )
{
	jeVec3d 		WorldDistance{};
	jeVec3d 		Center{};
	CMainFrame* pMainFrm{};
	jeBoolean	CenterValid{};

	ObjectList* pSelList{};
	Object* pObject{};
	ObjectIterator    Iterator{};
	int				  LightUpdate{};
	int				  BrushUpdate{};
	int				  BrushLighting{};
	jeBoolean		  bBrushUpdate{};
	jeBoolean		  bLightUpdate{};

		pMainFrm = (CMainFrame*)AfxGetMainWnd() ;

	Level_SetChanged( m_pLevel, JE_TRUE );

	jeVec3d_Set( &WorldDistance, 0.0f, 0.0f, 0.0f );
	CenterValid = Level_GetSelBoundsCenter( m_pLevel, &Center );
	jeVec3d_Clear( &m_DragPoint );
	switch( DataId )
	{

	case OBJECT_NAME_FIELD:
		Level_RenameSelected( m_pLevel, pData->String );
		pSelList = Level_GetSelList( m_pLevel );
		pObject = ObjectList_GetFirst( pSelList, &Iterator );
		while( pObject != nullptr )
		{
			pMainFrm->RenameObject( pObject );
			pObject = ObjectList_GetNext( pSelList, &Iterator );
		}
		break;
	
	case OBJECT_POSITION_FIELDX:
		assert( CenterValid );
		WorldDistance.X = pData->Float - Center.X;
		MoveSelected( Select_None, &WorldDistance );
		break;

	case OBJECT_POSITION_FIELDY:
		assert( CenterValid );
		WorldDistance.Y = pData->Float - Center.Y;
		MoveSelected( Select_None, &WorldDistance );
		break;

	case OBJECT_POSITION_FIELDZ:
		assert( CenterValid );
		WorldDistance.Z = pData->Float - Center.Z;
		MoveSelected( Select_None, &WorldDistance );
		break;


	default:
	{	


		BrushUpdate = Level_GetBrushUpdate( m_pLevel );
		LightUpdate = Level_GetLightUpdate( m_pLevel );
		BrushLighting = Level_GetBrushLighting( m_pLevel );

		bBrushUpdate = (BrushUpdate == LEVEL_UPDATE_CHANGE );
		bLightUpdate = (LightUpdate >= LEVEL_UPDATE_CHANGE );

		pSelList = Level_GetSelList( m_pLevel );
		pObject = ObjectList_GetFirst( pSelList, &Iterator );
		while( pObject != nullptr )
		{
			Object_SetProperty( pObject,  DataId, DataType, pData, bLightUpdate, bBrushUpdate, BrushLighting);
			pObject = ObjectList_GetNext( pSelList, &Iterator );
		}
	}
	break;
	}
	UpdateAllViews( nullptr, DOC_HINT_ALL, nullptr ) ;
	pMainFrm->PostUpdateProperties();
}

void CJweDoc::UpdateProperties()
{
	jeProperty_List* pArray{};
	CMainFrame* pMainFrm{};

	pMainFrm = (CMainFrame*)AfxGetMainWnd() ;
	pArray = Select_BuildDescriptor( m_pLevel );
	if( pArray )
	{
		pMainFrm->UpdateProperties(pArray );
		jeProperty_ListDestroy( &pArray );
	}
}

void CJweDoc::OnToolsPlacecylinder() 
{
	if( m_Mode == MODE_POINTER_CYLINDER )
	{
		SetMode( m_PrevMode ) ;
	}
	else
	{
		SetMode( MODE_POINTER_CYLINDER ) ;
	}
	
}

void CJweDoc::OnUpdateToolsPlacecylinder(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable( TRUE ) ;
	
}

void CJweDoc::OnToolsPlacespheroid() 
{
	if( m_Mode == MODE_POINTER_SPHERE )
	{
		SetMode( m_PrevMode ) ;
	}
	else
	{
		SetMode( MODE_POINTER_SPHERE ) ;
	}
	
}

void CJweDoc::OnUpdateToolsPlacespheroid(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable( TRUE ) ;
}

void CJweDoc::OnToolsPlacelight() 
{
	if( m_Mode == MODE_POINTER_LIGHT )
	{
		SetMode( m_PrevMode ) ;
	}
	else
	{
		SetMode( MODE_POINTER_LIGHT ) ;
	}
	
}

void CJweDoc::OnUpdateToolsPlacelight(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable( TRUE ) ;
	
}

int CJweDoc::GetBrushUpdate( )
{
	return( Level_GetBrushUpdate( m_pLevel ) );
}

int CJweDoc::GetLightUpdate(  )
{
	return( Level_GetLightUpdate( m_pLevel ) );
}

jeBoolean CJweDoc::GetBrushLighting(  )
{
	return( Level_GetBrushLighting( m_pLevel ) );
}

void CJweDoc::SetBrushUpdate( int Update )
{
	Level_SetBrushUpdate( m_pLevel, Update );
}

void CJweDoc::SetLightUpdate( int Update )
{
	Level_SetLightUpdate( m_pLevel, Update );
}

void CJweDoc::SetBrushLighting( int BrushLighting )
{
	Level_SetBrushLighting( m_pLevel, BrushLighting );
}

void CJweDoc::UpdateAll()
{
	Level_UpdateAll( m_pLevel );
	UpdateAllViews( nullptr, DOC_HINT_RENDERED, nullptr );
}

void CJweDoc::UpdateSelection()
{
	Level_UpdateSelected( m_pLevel );
	UpdateAllViews( nullptr, DOC_HINT_RENDERED, nullptr );

}


void CJweDoc::RotCurCamX( float Radians )
{
	Level_SetChanged( m_pLevel, JE_TRUE );
	Level_RotCurCamX( m_pLevel, Radians );
	UpdateAllViews( nullptr, DOC_HINT_ALL, (CObject*)nullptr );
}

void CJweDoc::RotCurCamY( float Radians )
{
	Level_SetChanged( m_pLevel, JE_TRUE );
	Level_RotCurCamY( m_pLevel, Radians );
	UpdateAllViews( nullptr, DOC_HINT_ALL, (CObject*)nullptr );
}

void CJweDoc::TranslateCurCam( jeVec3d * Offset )
{
	Level_SetChanged( m_pLevel, JE_TRUE );
	Level_TranslateCurCam( m_pLevel, Offset );
	UpdateAllViews( nullptr, DOC_HINT_ALL, (CObject*)nullptr  );
}

jeObject *	CJweDoc::GetCurCamObject( )
{
	return( Level_GetCurCamObject( m_pLevel ) );
}

void CJweDoc::GetCurCamXYRot( float *XRot, float *YRot )
{
	Level_GetCurCamXYRot( m_pLevel, XRot, YRot );
}

void CJweDoc::SetCurCamXYRot( float XRot, float YRot )
{
	Level_SetCurCamXYRot( m_pLevel, XRot, YRot );
}

jeBoolean CJweDoc::HasChanged()
{
	if( m_pLevel == nullptr )
		return( JE_FALSE );
	return( Level_HasChanged( m_pLevel ) );
}

void CJweDoc::Save()
{
	DoFileSave();
}

void CJweDoc::AbortMode()
{
	POSITION	pos{};
	CView* pView{};

	pos = GetFirstViewPosition();
	while( pos != nullptr )
	{
		pView = GetNextView(pos);
		ASSERT_VALID(pView);
		if( pView->IsKindOf( RUNTIME_CLASS (CJweView)) )
			((CJweView*)pView)->AbortMode();
	}
	SetNewBrushBoundInvalid();
	if( isPlaceBrushMode() || isPlaceLightMode() )
		SetMode( m_PrevMode ) ;
}

void CJweDoc::OnToolsPlacecamera() 
{
	if( m_Mode == MODE_POINTER_CAMERA )
	{
		SetMode( m_PrevMode ) ;
	}
	else
	{
		SetMode( MODE_POINTER_CAMERA ) ;
	}
}

void CJweDoc::OnToolsPlaceuserobj() 
{
	if( m_Mode == MODE_POINTER_USEROBJ )
	{
		SetMode( m_PrevMode ) ;
	}
	else
	{
		SetMode( MODE_POINTER_USEROBJ ) ;
	}	
	
}

jeBoolean CJweDoc::SetRenderMode( int Mode )
{
   m_RenderMode = Mode;
	return( Level_SetRenderMode( m_pLevel, Mode ) );
}

int CJweDoc::GetRenderMode()
{
   return m_RenderMode;
}


void CJweDoc::UpdateTimeDelta(  float TimeDelta )
{
	CMainFrame* pMainFrm{};

	pMainFrm = (CMainFrame*)AfxGetMainWnd() ;

	jeWorld_Frame( m_pWorld, TimeDelta );
	pMainFrm->UpdateTimeDelta( TimeDelta );
}


void CJweDoc::RenderAnimate( jeBoolean bAnimate )
{
	CJetView* pJetView{};
	pJetView = (CJetView *)GetJetView();
	if (pJetView==nullptr)
		return;
	pJetView->Animate( bAnimate );
	if( !bAnimate )
		UpdateAllViews( nullptr, DOC_HINT_ORTHO, (CObject*)nullptr  );
}

////////////////////////////////////////////////////////////////////////////////////////
//
//	CJweDoc::GetJetView()
//
////////////////////////////////////////////////////////////////////////////////////////
CView * CJweDoc::GetJetView()
{
	// locals
	POSITION	Pos{};
	CView* pView{};

	// do nothing if this view hasn't been created yet
	if ( this == nullptr )
	{
		return nullptr;
	}

	try {
		// switch modes
		Pos = this->GetFirstViewPosition();
		while ( Pos != nullptr )
		{
			pView = GetNextView( Pos );
			ASSERT_VALID( pView );
			if ( pView->IsKindOf( RUNTIME_CLASS( CJetView ) ) )
			{
				return pView;
			}
		}
	}
	catch(...)
	{
	}

	// if we got to here then Jet view was not found
	return nullptr;

} // CJweDoc::GetJetView()

jeEngine* CJweDoc::GetJetEngine()
{
	CJetView * pJetView;
	pJetView = (CJetView *) GetJetView();

	if (pJetView) return pJetView->GetEngine();
	return nullptr;
}


////////////////////////////////////////////////////////////////////////////////////////
//
//	CJweDoc::OnFullscreenView()
//
////////////////////////////////////////////////////////////////////////////////////////
void CJweDoc::OnFullscreenView() 
{


	// locals
	CView* pView{};

	// switch modes
	pView = GetJetView();
	assert( pView != nullptr );
	if ( ( (CJetView*)pView )->FullscreenView() == JE_FALSE )
	{
		jeErrorLog_AddString( JE_ERR_SUBSYSTEM_FAILURE, "CJweDoc::OnFullscreenView", "Failed to switch to full screen mode" );
//		return JE_FALSE;
	}

	// all done

//	return JE_TRUE;

} // OnFullscreenView()



////////////////////////////////////////////////////////////////////////////////////////
//
//	CJweDoc::OnVideosettingsWindowmode()
//
////////////////////////////////////////////////////////////////////////////////////////
void CJweDoc::OnVideosettingsWindowmode() 
{

	// locals
	CView* pView{};

	// choose window video settings
	pView = GetJetView();
	assert( pView != nullptr );
	if ( ( (CJetView *)pView )->ChooseWindowVideoSettings() == JE_FALSE )
	{
		jeErrorLog_AddString( JE_ERR_SUBSYSTEM_FAILURE, "CJweDoc::OnVideosettingsWindowmode", "TRACE" );
//		return JE_FALSE;
	}

	// all done
//	return JE_TRUE;
	
} // CJweDoc::OnVideosettingsWindowmode()



////////////////////////////////////////////////////////////////////////////////////////
//
//	CJweDoc::OnVideosettingsFullscreenmode()
//
////////////////////////////////////////////////////////////////////////////////////////
void CJweDoc::OnVideosettingsFullscreenmode() 
{
	OnFullscreen();
	
	/*		// Changed JH 7.3.2000

	// locals
	CView	*pView;

	// choose window video settings
	pView = GetJetView();
	assert( pView != nullptr );

	if ( ( (CJetView*)pView )->FullscreenView() == JE_FALSE )
	{
		jeErrorLog_AddString( JE_ERR_SUBSYSTEM_FAILURE, "CJweDoc::OnFullscreenView", "Failed to switch to full screen mode" );
		return JE_FALSE;
	}
*/
	// all done
//	return JE_TRUE;
	
} // CJweDoc::OnVideosettingsFullscreenmode()



////////////////////////////////////////////////////////////////////////////////////////
//
//	CJweDoc::UpdateWindow()
//
////////////////////////////////////////////////////////////////////////////////////////
BOOL CJweDoc::UpdateWindow(
	int	x,	// new horz position
	int	y )	// new vert position
{

	// locals
	BOOL	Result = TRUE;

	// update Jet view
	{

		// locals
		CView* pView{};

		// get Jet view
		pView = GetJetView();
		
		// update it
		if ( pView != nullptr )
		{
			Result &= ( (CJetView *)pView )->UpdateWindow();
		}
	}

	// all done
	return Result;

	// eliminate warnings
	x;
	y;

} // CJweDoc::UpdateWindow()


//---------------------------------------------------
// Added DJT
//---------------------------------------------------

void CJweDoc::SelectAll(jeBoolean UpdatePanel, int32 Mask)
{
	jeExtBox	ChangedBounds{};
	jeBoolean	bSelChanged = JE_FALSE ;
	
	bSelChanged = Select_All(m_pLevel, Mask, &ChangedBounds);

	if( JE_TRUE == bSelChanged )
	{
		jeProperty_List *pArray;

		pArray = Select_BuildDescriptor( m_pLevel );
		((CMainFrame*)AfxGetMainWnd())->SetProperties( pArray );			
		jeProperty_ListDestroy( &pArray );
		((CMainFrame*)AfxGetMainWnd())->UpdatePanel( MAINFRM_PANEL_LISTS ) ;
		UpdateAllViews( nullptr, DOC_HINT_ORTHO, (CObject*)&ChangedBounds ) ;
	}

	UpdatePanel;
}


void CJweDoc::OnUpdateEditSelectAll(CCmdUI* pCmdUI)
{
	// There must be something selectable 
	pCmdUI->Enable(true) ;
}


void CJweDoc::OnEditSelectAll()
{
	this->SelectAll(JE_TRUE);
}


void CJweDoc::OnUpdateEditSelectNone(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(Level_HasSelections(m_pLevel));
}

void CJweDoc::OnEditSelectNone()
{
	DeselectAll(JE_TRUE);
}

void CJweDoc::OnUpdateEditSelectInvert(CCmdUI* pCmdUI)
{
	// No yet available
	pCmdUI->Enable(false);

	// This will replace above when 
	// select invert code is ready.
//	pCmdUI->Enable(Level_HasSelections(m_pLevel));
}
void CJweDoc::OnEditSelectInvert()
{
}


void CJweDoc::OnUpdateEditSelectType(CCmdUI* pCmdUI)
{
	jeBoolean bEnabled;

	// There must be something selectable of this type
	switch (pCmdUI->m_nID)
	{
		case IDM_EDIT_SELECTBRUSHES:
			bEnabled = Level_TestForObject(m_pLevel, KIND_BRUSH);
			break;
		case IDM_EDIT_SELECTCAMERAS:
			bEnabled = Level_TestForObject(m_pLevel, KIND_CAMERA);
			break;
		case IDM_EDIT_SELECTENTITIES:
			bEnabled = Level_TestForObject(m_pLevel, KIND_ENTITY);
			break;
		case IDM_EDIT_SELECTLIGHTS:
			bEnabled = Level_TestForObject(m_pLevel, KIND_LIGHT);
			break;
		case IDM_EDIT_SELECTMODELS:
			bEnabled = Level_TestForObject(m_pLevel, KIND_MODEL);
			break;
		case IDM_EDIT_SELECTTERRAIN:
			bEnabled = Level_TestForObject(m_pLevel, KIND_TERRAIN);
			break;
		case IDM_EDIT_SELECTUSER:
			bEnabled = Level_TestForObject(m_pLevel, KIND_USEROBJ);
			break;
		default:
			bEnabled = JE_FALSE;
			assert(true);
	}

	if (bEnabled)
		pCmdUI->Enable(true);
	else
		pCmdUI->Enable(false);
}

void CJweDoc::OnEditSelectCameras()
{
	this->SelectAll(JE_TRUE, KIND_CAMERA);
}

void CJweDoc::OnEditSelectBrushes()
{
	this->SelectAll(JE_TRUE, KIND_BRUSH);
}

void CJweDoc::OnEditSelectEntities()
{
	this->SelectAll(JE_TRUE, KIND_ENTITY);
}

void CJweDoc::OnEditSelectLights()
{
	this->SelectAll(JE_TRUE, KIND_LIGHT);
}

void CJweDoc::OnEditSelectModels()
{
	this->SelectAll(JE_TRUE, KIND_MODEL);
}

void CJweDoc::OnEditSelectTerrain()
{
	this->SelectAll(JE_TRUE, KIND_TERRAIN);
}

void CJweDoc::OnEditSelectUser()
{
	this->SelectAll(JE_TRUE, KIND_USEROBJ);
}
//---------------------------------------------------
// End DJT
//---------------------------------------------------


// CJP : Neccesary to update, enable vertex mode selection in menu.
void CJweDoc::OnModeVertex() 
{
	SetMode( MODE_POINTER_VM ) ;
}// OnModeVertex

void CJweDoc::OnUpdateModeVertex(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable( true ) ;
	pCmdUI->SetCheck( MODE_POINTER_VM == m_Mode ) ;
}

//---------------------------------------------------
// Added JH 07.02.2000
//---------------------------------------------------

void CJweDoc::OnPreferences()
{
	//CPreferences	PrefsDialog;  // replace with class member variable
	char			sTempString[200];
	char			cWindowRes[400];
	CView			*pView{};
	CMainFrame		*pMainFrm{};

	pMainFrm = (CMainFrame*)AfxGetMainWnd();
	if (pMainFrm)
	{

		if (IDOK == m_pPrefsDialog->DoModal())
		{
			Level_SetShouldSnapVerts(m_pLevel, Settings_GetGrid_SnapVertexManip());
			Level_SetGridSnapSize(m_pLevel, Settings_GetGrid_VertexSnap());
			Level_SetRotateSnapSize(m_pLevel, atoi(Settings_GetGrid_SnapDegrees(sTempString, 199)));
			pMainFrm->SetAccelerator();

			// Get Screenmode setting
			Settings_GetJet_Window(cWindowRes, 399);

			pView = GetJetView();
			if (pView)
			{

				((CJetView*)pView)->SetWindowModeByString(cWindowRes);

				if (((CJetView*)pView)->ChooseWindowVideoSettings() == JE_FALSE)
				{
					jeErrorLog_AddString(JE_ERR_SUBSYSTEM_FAILURE, "CJweDoc::OnFullscreenView", "Failed to switch to full screen mode");
					return;
				}
			}
		}
	}
}

void CJweDoc::OnUpdatePreferences(CCmdUI* pCmdUI)
{
	// There must be something selectable 
	pCmdUI->Enable(true) ;
}

//---------------------------------------------------
// Added JH 28.02.2000 (Import and Export functions - coming soon....)
//---------------------------------------------------

// Just one or two weeks till import, export works...:)
void CJweDoc::OnImportBrush() 
{
// Import Brush

/*	Import_Objects (m_pLevel,"c:\\export.txt");*/
}

void CJweDoc::OnUpdateImportBrush(CCmdUI* pCmdUI)
{
	// There must be something selectable 
	pCmdUI->Enable(false) ;
}



void CJweDoc::OnExportBrush() 
{	
	CExtFileDialog *FileDlg= new CExtFileDialog( FALSE,
										  "Export Objects as ASCII...",
										  "*.jta",
										  OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT|OFN_ENABLESIZING,
										  "Jet3D ASCII-Objects (*.jta)|*.jta|All Files (*.*)|*.*||",
										  AfxGetMainWnd(),false);

	if (FileDlg != nullptr)
		if (FileDlg->DoModal()==IDOK)
		{
//			WriteWindowToDIB ((LPCTSTR )FileDlg->GetPathName(),pView);
		}
		//Export_ObjectsToASCII(m_pLevel,FileDlg->GetPathName());	
	if (FileDlg != nullptr)
		delete FileDlg;
}

void CJweDoc::OnUpdateExportBrush(CCmdUI* pCmdUI)
{
	// There must be something selectable 
	pCmdUI->Enable(true) ;
}


//---------------------------------------------------
// Added JH 14.03.2000 
//---------------------------------------------------
void CJweDoc::OnFileProps() 
{	

	m_pPropsDialog->DoModal();
}

void CJweDoc::OnUpdateFileProps(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(true) ;
}

//---------------------------------------------------
// End JH 
//---------------------------------------------------

void CJweDoc::OnExportPrefab() 
{
	CMainFrame* pMainFrm = (CMainFrame*)AfxGetMainWnd();
	pMainFrm->m_GroupDialog.ExportPrefab();
}

void CJweDoc::OnUpdateExportPrefab(CCmdUI* pCmdUI) 
{
	BOOL enable = FALSE;
	Group* pGroup = GetCurrentGroup();
	if (pGroup) {
		enable = (strcmp(Group_GetName(pGroup), "Default") != 0);
	}
	pCmdUI->Enable(enable);
}

void CJweDoc::OnImportPrefab() 
{
	CMainFrame* pMainFrm = (CMainFrame*)AfxGetMainWnd();
	pMainFrm->m_GroupDialog.ImportPrefab();
}


struct VertInfo
{
	char MatName[64];
};

struct VertData
{
	float pos[3];
	float nor[3];
	float u;
	float v;
	long  idx;
};

struct EnumLevelData 
{
	CList<VertData, VertData&>  VertDataList;
	CArray<VertInfo, VertInfo&> VertInfoList;
	CDWordArray                 IndexArray;

	jeWorld					    *pWorld;
	CJweDoc						*pDoc;
};

jeBoolean EnumLevelCB(Brush* curBrush, void* param)
{
	jeVec3d Tri[3];
	jeVec3d VecU{}, VecV{};
	jePlane Plane{};

	EnumLevelData* pEnumerator = (EnumLevelData*) param;

	// Get the XForm matrices of the brush
	const jeXForm3d *XForm = jeBrush_GetXForm(Brush_GetjeBrush(curBrush));
	const jeXForm3d *WorldToLocked = jeBrush_GetWorldToLockedXForm(Brush_GetjeBrush(curBrush));
	const jeXForm3d *LockedToWorld = jeBrush_GetLockedToWorldXForm(Brush_GetjeBrush(curBrush));

	int i;
	int fcnt = Brush_GetFaceCount(curBrush);
	for (i=0; i<fcnt; i++) {
		jeFaceInfo finfo;

		jeBrush_Face* pFace = Brush_GetFaceByIndex(curBrush, i);
		jeBrush_FaceGetFaceInfo(pFace, &finfo);

		VertInfo vi;
		const jeMaterial* pMat = jeMaterial_ArrayGetMaterialByIndex(jeWorld_GetMaterialArray(pEnumerator->pWorld), finfo.MaterialIndex);
		strcpy(vi.MatName, jeMaterial_GetName(pMat));

		const jeMaterialSpec* pMatSpec = jeMaterial_GetMaterialSpec(pMat);

		int k;
		long matidx = -1;
		for (k=0;k<pEnumerator->VertInfoList.GetUpperBound();k++) {
			VertInfo& vimlkf = pEnumerator->VertInfoList.GetAt(k);
			if (strcmp(vi.MatName, vimlkf.MatName) == 0) {
				matidx = k;
				vi = vimlkf;
			}
		}
		if (matidx<0) {
			matidx = pEnumerator->VertInfoList.Add(vi);
		}

		int offsetVD = pEnumerator->VertDataList.GetCount();

		int j;
		// Create the world space plane
		for (j=0; j< 3; j++)
		{
			Tri[j] = jeBrush_FaceGetWorldSpaceVertByIndex(pFace, j);
		}
		jePlane_SetFromVerts(&Plane, &Tri[0], &Tri[1], &Tri[2]);

		// Put the normal into locked space
		jeXForm3d_Rotate(WorldToLocked, &Plane.Normal, &Plane.Normal);
		jeVec3d_Normalize(&Plane.Normal);
		
		// Get the locked texture vectors from the locked normal
		jePlane_GetAAVectors(&Plane, &VecU, &VecV);

		jeVec3d_Scale(&VecU, 1.0f/finfo.LMapScaleU, &VecU);
		jeVec3d_Scale(&VecV, 1.0f/finfo.LMapScaleV, &VecV);
		
		// Rotate the texture vectors
		{
			jeVec3d			Axis{};
			jeXForm3d		RotXForm{};
			jeQuaternion	Quat{};
			
			jeVec3d_CrossProduct(&VecU, &VecV, &Axis);
			
			jeVec3d_Normalize(&Axis);
			
			jeQuaternion_SetFromAxisAngle(&Quat, &Axis, (finfo.Rotate/180.0f)*JE_PI);
			jeQuaternion_ToMatrix(&Quat, &RotXForm);
			
			jeXForm3d_Transform(&RotXForm, &VecU, &VecU);
			jeXForm3d_Transform(&RotXForm, &VecV, &VecV);
		}
		
		
		// Rotate the locked texture vectors into world space
		jeXForm3d_Rotate(LockedToWorld, &VecU, &VecU);
		jeXForm3d_Rotate(LockedToWorld, &VecV, &VecV);

		int texWidth = jeMaterialSpec_Width(pMatSpec);
		int texHeight = jeMaterialSpec_Height(pMatSpec);

		jeFloat ShiftU{};
		jeFloat ShiftV{};

		bool bFirstUV = true;
		int vcnt = jeBrush_FaceGetVertCount(pFace);
		for (j=0; j<vcnt; j++) {
			VertData vd;
			const jeVec3d* pos = jeBrush_FaceGetVertByIndex(pFace, j);
			// Position
			vd.pos[0] = pos->X;
			vd.pos[1] = pos->Y;
			vd.pos[2] = pos->Z;

			const jeVec3d* normal = &Plane.Normal;
			vd.nor[0] = normal->X;
			vd.nor[1] = normal->Y;
			vd.nor[2] = normal->Z;

			vd.u = jeVec3d_DotProduct(pos, &VecU);
			vd.v = jeVec3d_DotProduct(pos, &VecV);

			if (bFirstUV) {
				ShiftU = (jeFloat)(((int32)(vd.u/(jeFloat)texWidth))*texWidth);
				ShiftV = (jeFloat)(((int32)(vd.v/(jeFloat)texHeight))*texHeight);

				ShiftU *= (finfo.DrawScaleU/finfo.LMapScaleU);
				ShiftV *= (finfo.DrawScaleV/finfo.LMapScaleV);
				bFirstUV = false;
			}

			vd.u -= ShiftU;
			vd.v -= ShiftV;
			vd.idx = matidx;

			pEnumerator->VertDataList.AddTail(vd);
			pEnumerator->IndexArray.Add((offsetVD + j));
		}
	}

	return JE_TRUE;
}

#ifndef MAKEFOURCC
#define MAKEFOURCC(ch0, ch1, ch2, ch3)                              \
		((DWORD)(BYTE)(ch0) | ((DWORD)(BYTE)(ch1) << 8) |   \
		((DWORD)(BYTE)(ch2) << 16) | ((DWORD)(BYTE)(ch3) << 24 ))
#endif

void CJweDoc::OnFileExportExportforbtprojectworkspacebtw() 
{
	// Prompt a CFileDialog with default dir : /prefab, def ext *.j3p
	static char* szFilter = "btProject from Jet (*.btj)|*.btj||";

	CFileDialog saveDlg(FALSE, "btj", nullptr, OFN_OVERWRITEPROMPT, szFilter);

	if (saveDlg.DoModal())
	{
		// export all data from curren level
		FILE* file = fopen(saveDlg.GetPathName(), "wb");

		long toWrite = MAKEFOURCC('B','T','J','3');
		fwrite(&toWrite, 4, 1, file);

		toWrite = 0;
		fwrite(&toWrite, 4, 1, file);

		fwrite(&toWrite, 4, 1, file);
		fwrite(&toWrite, 4, 1, file);

		EnumLevelData enumLevelData;
		enumLevelData.pWorld = Level_GetjeWorld(m_pLevel);
		enumLevelData.pDoc = this;

		ModelList* modelLst = Level_GetModelList(m_pLevel);
		ModelList_EnumBrushes(modelLst, &enumLevelData, EnumLevelCB);

		toWrite = enumLevelData.VertInfoList.GetSize();
		fwrite(&toWrite, 4, 1, file);

		toWrite = enumLevelData.VertDataList.GetCount();
		fwrite(&toWrite, 4, 1, file);

		toWrite = enumLevelData.IndexArray.GetSize();
		fwrite(&toWrite, 4, 1, file);

		int idx;
		for (idx=0; idx<enumLevelData.VertInfoList.GetUpperBound(); idx++) {
			VertInfo& vi = enumLevelData.VertInfoList.GetAt(idx);
			fwrite(vi.MatName, 64, 1, file);
		}

		POSITION pos = enumLevelData.VertDataList.GetHeadPosition();
		while (pos) {
			VertData& vd = enumLevelData.VertDataList.GetNext(pos);
			fwrite(&vd, sizeof(vd), 1, file);
		}

		for (idx=0; idx<enumLevelData.IndexArray.GetUpperBound(); idx++) {
			toWrite = enumLevelData.IndexArray.GetAt(idx);
			fwrite(&toWrite, 4, 1, file);
		}

		fclose(file);
	}
}

void CJweDoc::OnToolsPlacearch()
{
	// TODO: Add your command handler code here
	if( m_Mode == MODE_POINTER_ARCH )
	{
		SetMode( m_PrevMode ) ;
	}
	else
	{
		SetMode( MODE_POINTER_ARCH ) ;
	}
}

void CJweDoc::OnFileClose()
{
    CMainFrame *	pMainFrame = (CMainFrame*)AfxGetMainWnd() ;
    pMainFrame->SetCurrentDocument(nullptr);
    // TODO: Add your command handler code here
    CJ3DDoc::OnFileClose();
}
