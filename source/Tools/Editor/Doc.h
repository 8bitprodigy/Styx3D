/****************************************************************************************/
/*  DOC.H                                                                               */
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

#if !defined(AFX_DOC_H__37F45637_C0E1_11D2_8B41_00104B70D76D__INCLUDED_)
#define AFX_DOC_H__37F45637_C0E1_11D2_8B41_00104B70D76D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define DOC_MAX_CAMERAS		(1)

#include "Defs.h"
#include "J3DDoc.h"
#include "Level.h"
#include "Ortho.h"
#include "Select.h"
#include "Basetype.h"	// Added by ClassView
#include "..\CORE\Level.h"	// Added by ClassView
#include "Properties.h"
#include "Preferences.h"

typedef enum
{
   DOC_HINT_NONE,	// Means draw all, invalidate all
      DOC_HINT_ALL,	// Draw all, invalidate hint rect
      DOC_HINT_ORTHO,
      DOC_HINT_RENDERED,
      DOC_HINT_LAST
} DOC_HINT ;

typedef enum
{
   DOC_NO_CONSTRUCTOR,
      DOC_VERTICAL_CONSTRUCTOR,
      DOC_HORIZONTAL_CONSTRUCTOR,
      DOC_BOTH_CONSTRUCTOR
} DOC_CONSTRUCTORS;

typedef enum 
{
   DOC_HANDLE_NONE,
      DOC_HANDLE_SIZE,
      DOC_HANDLE_ROTATE,
      DOC_HANDLE_SHEAR,
} DOC_HANDLE_MODE;

// Added JH 11.03.2000
typedef enum 
{
   DOC_ALIGN_LEFT,
      DOC_ALIGN_RIGHT,
      DOC_ALIGN_TOP,
      DOC_ALIGN_BOTTOM,
} DOC_ALIGN_MODE;

// foward definition
class CRebuild;
class CJetView;

class CJweDoc : public CJ3DDoc
{
protected: // create from serialization only
   CJweDoc();
   DECLARE_DYNCREATE(CJweDoc)
      
      // Attributes
public:
   
   // Operations
public:
   
   // Overrides
   // ClassWizard generated virtual function overrides
   //{{AFX_VIRTUAL(CJweDoc)
public:
   virtual BOOL OnNewDocument();
   virtual void Serialize(CArchive& ar);
   virtual void DeleteContents();
   virtual BOOL OnSaveDocument(LPCTSTR lpszPathName);
   virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
   //}}AFX_VIRTUAL
   
   // Implementation
public:
   jeBoolean	m_bLoaded;
   
   //Selection
   jeBoolean SelectObject( Object * pObject , LEVEL_STATE eState );
   jeBoolean SubSelectgeObject( jeObject * pgeObject, 	LEVEL_STATE eState  );
   jeBoolean MarkSubSelect( jeObject * pgeObject, int32 flag );
   void SetSelectionName( const char * pName );
   const char * GetSelectionName( int32 * pnNumber );
   LEVEL_SEL GetSelType( void );
   jeBoolean RectangleSelect( jeExtBox * pBox, jeBoolean bAppend );
   jeBoolean HasSelections( jeExtBox * pSelBounds );
   jeBoolean HasSubSelections( jeExtBox * pSelBounds );
   int32	  SubSelXFormModFlags();
   jeBoolean Select( const Ortho * pOrtho, const Point * pViewPt, LEVEL_STATE eState, jeBoolean bControl_Held );
   jeBoolean Select3d( const jeCamera * pCamera, const Point *pViewPt );
   void	  DeselectAllSub();
   void	  DeleteSelection();
   
   // Object Lists
   LightList  * GetLightList( void );
   CameraList * GetCameraList( void );
   ModelList  * GetModelList( void );
   GroupList  * GetGroupList( void );
   ObjectList * GetSelectList( void );
   jeBoolean EnumSelected( void * lParam, ObjectListCB Callback );
   jeBoolean EnumObjects( void * lParam, ObjectListCB Callback );
   
   //misc
   BOOL CreateLevel( void );
   void ApplyMaterial( void );
   void UpdateTimeDelta(  float TimeDelta );
   void UpdateProperties();
   
   //Vertex Manipulation
   void EndMoveVerts( void );
   jeBoolean MoveVerts( const Ortho * pOrtho, jeVec3d * pWorldDistance );
   jeBoolean BeginMoveVerts( const Ortho * pOrtho );
   jeBoolean IsVertexManipulationMode( void );
   
  	// added by cjp (1/19/00)
  	void JoinSelectedVertices( void );
  	// End added by cjp
   
   //Handles
   void EndMoveHandle( void );
   jeBoolean BeginMoveHandle( const Ortho * pOrtho, SELECT_HANDLE eHandle, DOC_HANDLE_MODE *Mode );
   jeBoolean BeginRotateSub( );
   void RotateSelectedSub(const Ortho * pOrtho, Point * pMousePt, Point *pAnchor );
   void EndRotateSub();
   void BeginMove( const Ortho * pOrtho, SELECT_HANDLE eCorner, jeBoolean bCopy );
   void BeginMoveSub(  );
   void MoveHandle( const Ortho * pOrtho, jeVec3d * pWorldDistance, SELECT_HANDLE eSizeType, Point * pMousePt, Point * pAnchorPt, jeVec3d *Center3d );
   void EndMove( void );
   void EndMoveSub( void );
   
   //Mode
   MODE SetMode( const MODE eMode );
   void SetCursor( Ortho * pOrtho, POINT * pViewPt );
   void MoveSelected( SELECT_HANDLE eCorner, jeVec3d * pWorldDistance );
   void MoveSelectedSub( SELECT_HANDLE eCorner, jeVec3d * pWorldDistance );
   void DeselectAll( jeBoolean UpdatePanel );
   void DeselectAllFaces(  );
   //---------------------------------------------------
   // Added DJT
   //---------------------------------------------------
  	void SelectAll(jeBoolean UpdatePanel, int32 Mask = OBJECT_KINDALL);
   //---------------------------------------------------
   // End DJT
   //---------------------------------------------------
   
   
   jeBoolean isPlaceBrushMode();
   jeBoolean isPlaceLightMode();
   void GetModeKind( int *Kind, int *SubKind );
   void AbortMode();
   
   
   //Place
   void PlaceBrush( jeBoolean bSubtract );
   void PlaceObject(  jeExtBox	*pObjectBounds, jeBoolean bSubtract );
   void PlaceAtPoint( const Ortho * pOrtho, Point * pPoint,  jeBoolean bSubtract );
   void SetNewBrushBound(Ortho * pOrtho, Point * pMousePt, Point *pAnchor );
   const jeExtBox * GetNewBrushBounds();
   void SetNewBrushHeight( Ortho * pOrtho, Point * pMousePt, Point *pAnchor );  //Sets Doc NewBurshBounds Min to contsuctor and max to Height
   void SetNewBrushBoundInvalid();
   LPCTSTR		GetConstructorCursor(Ortho *pOrtho, POINT *pViewPt);
   void		MoveConstructor( Ortho *pOrtho, DOC_CONSTRUCTORS Constructor, Point * pMousePt, Point *pAnchor );
   
   SELECT_HANDLE ViewPointHandle( Ortho * pOrtho, Point * pViewPt, jeExtBox * pWorldBox );
   SELECT_HANDLE SubViewPointHandle( Ortho * pOrtho, Point * pViewPt, jeExtBox * pWorldBox );
   DOC_CONSTRUCTORS ViewPointConstructor( Ortho * pOrtho, Point * pViewPt);
   
   //Groups
   Group *		AddGroup( const char * Name );
   Group *		GetCurrentGroup( void );
   void		SetCurrentGroup( Group * pGroup );
   
   //Models
   Model *		GetCurrentModel( void );
   void		SetCurrentModel( Model * pModel );
   Model *		CreateModel( const char * pszName );
   void		ModelLock( Model * pModel, jeBoolean bLock );
   
   //Classes
   Class *		CreateClass( const char * pszName, int Kind );
   
   //Views
   void		RenderAnimate( jeBoolean bAnimate );
   void		CenterViewsOnSelection( );
   jeBoolean SetRenderMode( int Mode );
   int      GetRenderMode();
   BOOL		RenderLights( jeCamera* pCamera );
   void		RenderOrthoView( CDC * pDC, Ortho * pOrtho );
   void		UpdateAllViews(CView* pSender, LPARAM lHint=NULL, CObject* pHint=NULL) ;
   jeBoolean	GetSelRadiusBox( Ortho *pOrtho, Rect *pBox );
   BOOL		UpdateWindow( int x, int y );
   
   //Draw Routines
   void DrawGrid( CDC *pDC, Ortho *pOrtho);
   void DrawOrthoName( CDC *pDC, Ortho *pOrtho);
   void DrawConstructorLine( CDC *pDC, Ortho *pOrtho );
   void DrawObjects( CDC *pDC, Ortho *pOrtho );
   void DrawSelected( CDC *pDC, Ortho *pOrtho );
   void DrawSelectBounds( CDC *pDC, Ortho *pOrtho );
   void DrawSelectElipse( CDC *pDC, Ortho *pOrtho );
   void DrawSelectAxis( Ortho * pOrtho, HDC hDC );
   
   // Text Functions
   void PrintRectDimensions( CDC *pDC,const Ortho * pOrtho, const jeExtBox	*pselBox );	// Added JH 3.3.2000
   
   //Update Modes
   int			GetBrushUpdate(  );
   int			GetLightUpdate(  );
   void		RebuildLights( );
   void		SetBrushLighting( int BrushLighting );
   void		UpdateAll();
   void		UpdateSelection();
   jeBoolean   GetBrushLighting(  );
   void		SetBrushUpdate( int Update );
   void		SetLightUpdate( int Update );
   
   
   void		SetProperty( int DataId, int DataType, jeProperty_Data * pData );
   
   //Camera
   void		RotCurCamX( float Radians );
   void		RotCurCamY( float Radians );
   void		TranslateCurCam( jeVec3d * Offset );
   jeObject *	GetCurCamObject( );
   void		GetCurCamXYRot( float *XRot, float *YRot );
   void		SetCurCamXYRot( float XRot, float YRot );
   
   jeBoolean	HasChanged();
   void		Save();
   void		UpdateStats();
   
   //---------------------------------------------------
   // Added DJT
   //---------------------------------------------------
  	inline Level * GetLevel() const {return m_pLevel;}
   //---------------------------------------------------
   // End DJT
   //---------------------------------------------------
	// Krouer: add few accessor
	inline jeResourceMgr* GetResourceMgr() { return m_pResourceMgr; }
	jeEngine* GetJetEngine();
	// end Krouer adds
   
   virtual BOOL SetDrawFaceCB(jeEngine *Engine, jeBoolean Enable);
   virtual BOOL Render( class CJ3DView * pView );
   virtual ~CJweDoc();
#ifdef _DEBUG
   virtual void AssertValid() const;
   virtual void Dump(CDumpContext& dc) const;
#endif
   
 protected:
    
    // Generated message map functions
 protected:
    //{{AFX_MSG(CJweDoc)
    afx_msg void OnToolsPlacecube();
    afx_msg void OnUpdateToolsPlacecube(CCmdUI* pCmdUI);
    afx_msg void OnToolsPlacesheet();
    afx_msg void OnUpdateToolsPlacesheet(CCmdUI* pCmdUI);
    afx_msg void OnViewShowallgroups();
    afx_msg void OnUpdateViewShowallgroups(CCmdUI* pCmdUI);
    afx_msg void OnViewShowvisiblegroups();
    afx_msg void OnUpdateViewShowvisiblegroups(CCmdUI* pCmdUI);
    afx_msg void OnViewShowCurrentgroup();
    afx_msg void OnUpdateViewShowCurrentgroup(CCmdUI* pCmdUI);
    afx_msg void OnEditAddtogroup();
    afx_msg void OnUpdateEditAddtogroup(CCmdUI* pCmdUI);
    afx_msg void OnEditRemovefromgroup();
    afx_msg void OnUpdateEditRemovefromgroup(CCmdUI* pCmdUI);
    afx_msg void OnToolsRebuildall();
    afx_msg void OnUpdateToolsRebuildall(CCmdUI* pCmdUI);
    afx_msg void OnModeAdjust();
    afx_msg void OnUpdateModeAdjust(CCmdUI* pCmdUI);
    afx_msg void OnModeRotateshear();
    afx_msg void OnUpdateModeRotateshear(CCmdUI* pCmdUI);
    afx_msg void OnOptionsSnaptogrid();
    afx_msg void OnUpdateOptionsSnaptogrid(CCmdUI* pCmdUI);
    afx_msg void OnEditUndo();
    afx_msg void OnUpdateEditUndo(CCmdUI* pCmdUI);
    afx_msg void OnEditClear();
    afx_msg void OnUpdateEditClear(CCmdUI* pCmdUI);
    afx_msg void OnModeFacemanipulation();
    afx_msg void OnUpdateModeFacemanipulation(CCmdUI* pCmdUI);
    afx_msg void OnToolsNextface();
    afx_msg void OnUpdateToolsNextface(CCmdUI* pCmdUI);
    afx_msg void OnToolsPrevface();
    afx_msg void OnUpdateToolsPrevface(CCmdUI* pCmdUI);
    afx_msg void OnToolsBuildlights();
    afx_msg void OnToolsPlacecylinder();
    afx_msg void OnUpdateToolsPlacecylinder(CCmdUI* pCmdUI);
    afx_msg void OnToolsPlacespheroid();
    afx_msg void OnUpdateToolsPlacespheroid(CCmdUI* pCmdUI);
    afx_msg void OnToolsPlacelight();
    afx_msg void OnUpdateToolsPlacelight(CCmdUI* pCmdUI);
    afx_msg void OnToolsPlacecamera();
    afx_msg void OnToolsPlaceuserobj();
    afx_msg void OnFullscreenView();
    afx_msg void OnVideosettingsWindowmode();
    afx_msg void OnVideosettingsFullscreenmode();
    afx_msg void OnUpdateEditSelectAll(CCmdUI* pCmdUI);
    afx_msg void OnEditSelectAll();
    afx_msg void OnUpdateEditSelectNone(CCmdUI* pCmdUI);
    afx_msg void OnEditSelectNone();
    afx_msg void OnUpdateEditSelectInvert(CCmdUI* pCmdUI);
    afx_msg void OnEditSelectInvert();
    afx_msg void OnUpdateEditSelectType(CCmdUI* pCmdUI);
    afx_msg void OnEditSelectBrushes();
    afx_msg void OnEditSelectCameras();
    afx_msg void OnEditSelectEntities();
    afx_msg void OnEditSelectLights();
    afx_msg void OnEditSelectModels();
    afx_msg void OnEditSelectTerrain();
    afx_msg void OnEditSelectUser();
    afx_msg void OnImportBrush ();
    afx_msg void OnUpdateImportBrush (CCmdUI* pCmdUI);
    afx_msg void OnExportBrush();
    afx_msg void OnUpdateExportBrush(CCmdUI* pCmdUI);
    afx_msg void OnPreferences();
    afx_msg void OnUpdatePreferences(CCmdUI* pCmdUI);
    afx_msg void OnAnim();
    afx_msg void OnUpdateAnim(CCmdUI* pCmdUI);
    afx_msg void OnFullscreen();
    afx_msg void OnUpdateFullscreen(CCmdUI* pCmdUI);	
    afx_msg void OnUpdateAll();
    afx_msg void OnUpdateUpdateAll(CCmdUI* pCmdUI);	
    afx_msg void OnToolsUpdateSelection();
    afx_msg void OnUpdateToolsUpdateSelection(CCmdUI* pCmdUI);
    afx_msg void OnEditAlignLeft();
    afx_msg void OnUpdateEditAlignLeft(CCmdUI* pCmdUI);
    afx_msg void OnEditAlignRight();
    afx_msg void OnUpdateEditAlignRight(CCmdUI* pCmdUI);
    afx_msg void OnEditAlignBottom();
    afx_msg void OnUpdateEditAlignBottom(CCmdUI* pCmdUI);
    afx_msg void OnEditAlignTop();
    afx_msg void OnUpdateEditAlignTop(CCmdUI* pCmdUI);
    afx_msg void OnFileProps();
    afx_msg void OnUpdateFileProps(CCmdUI* pCmdUI);
    afx_msg void OnEditRotL();
    afx_msg void OnUpdateEditRotL(CCmdUI* pCmdUI);
    afx_msg void OnEditRotR();
    afx_msg void OnUpdateEditRotR(CCmdUI* pCmdUI);
    afx_msg void OnEditToFront();
    afx_msg void OnUpdateEditToFront(CCmdUI* pCmdUI);
    afx_msg void OnModeVertex();
    afx_msg void OnUpdateModeVertex(CCmdUI* pCmdUI);
	afx_msg void OnExportPrefab();
	afx_msg void OnUpdateExportPrefab(CCmdUI* pCmdUI);
	afx_msg void OnImportPrefab();
	afx_msg void OnFileExportExportforbtprojectworkspacebtw();
	//}}AFX_MSG
    afx_msg void OnViewHideCurrentgroup();
    afx_msg void OnUpdateViewHideCurrentgroup(CCmdUI* pCmdUI);
    afx_msg void OnToolsPlacearch();
    DECLARE_MESSAGE_MAP()
          
    // Generated OLE dispatch map functions
    //{{AFX_DISPATCH(CJweDoc)
    // NOTE - the ClassWizard will add and remove member functions here.
    //    DO NOT EDIT what you see in these blocks of generated code !
    //}}AFX_DISPATCH
    DECLARE_DISPATCH_MAP()
    DECLARE_INTERFACE_MAP()

 private:
    void BeginShear( void );
    void BeginRotate( void );
    void BeginSize( void );
    
    jeBitmap *	InitBitmap( WORD Resource );
    jeMaterialSpec *	InitMaterial( WORD Resource );
    static void			DrawFaceCB(const jeTLVertex *Verts, int32 NumVerts, void *Context);
    static jeBoolean	SetModelFaceCB( Model *pModel, void * pVoid );
    CView *		GetJetView();
    CRebuild	*RebuildDlg;
    jeBoolean	m_bCopying ;
    jeVec3d		m_DragPoint;
    MODE		m_Mode;
    MODE		m_PrevMode;
    Level *		m_pLevel;
    float		m_LastFOV;
    jeFloat		m_LastRotateAngle;
    jeExtBox	m_NewBrushBounds;
#ifdef _USE_BITMAPS  //krouer: tempory use of bitmap
    jeBitmap *	LightBitmap;
#else
    jeMaterialSpec *	LightBitmap;
#endif
    BOOL     m_RenderMode;
	jeResourceMgr *	m_pResourceMgr; // krouer

	//	tom morris feb 2005
	const CString	m_strRebuild;
	//	end tom morris feb 2005
    
    jeBoolean	m_Anim_State;					// Added JH 7.3.2000
    void		AlignObjects (DOC_ALIGN_MODE Align_Mode );  // Added JH 11.3.2000
    CProperties		*m_pPropsDialog;					// Added JH 16.3.2000
	CPreferences	*m_pPrefsDialog;
    void		RotateObjects (jeFloat Angle ); // Added JH 24.3.2000
    void		ObjectsToFront();	// Added JH 25.3.2000
public:
    afx_msg void OnFileClose();
};
    
 /////////////////////////////////////////////////////////////////////////////
 
 //{{AFX_INSERT_LOCATION}}
 // Microsoft Visual C++ will insert additional declarations immediately before the previous line.
    
#endif // !defined(AFX_DOC_H__37F45637_C0E1_11D2_8B41_00104B70D76D__INCLUDED_)
