/****************************************************************************************/
/*  DIALOGBUILDERDLG.H                                                                  */
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
#if !defined(AFX_DIALOGBUILDERDLG_H__0B24B489_1A7F_11D3_B323_004033AA0441__INCLUDED_)
#define AFX_DIALOGBUILDERDLG_H__0B24B489_1A7F_11D3_B323_004033AA0441__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "jeProperty.h"
#include "Resource.h"
#include "buildspin.h"

//	1.16.05 by tom morris - better support for new color button
#include "colorbutton.h"

/////////////////////////////////////////////////////////////////////////////
// CDialogBuilderDlg dialog

typedef struct FieldStruct {
	PROPERTY_FIELD_TYPE Type;
	UINT		  Id;
	int		  DataId;
	CStatic * FieldName;
	CStatic	* EditBorder;
	CBuildSpin	* Spin;
	CWnd	* FieldCntl;
	int		  DisplayLevel;  // This defines what level to display in Hierarchy
	BOOL	  DisplayChildren; // For Group type flag of wether to display child contorls
} FieldStruct;

class CDialogBuilderDlg : public CDialog
{
// Construction
public:
	CDialogBuilderDlg(CWnd* pParent = NULL);	// standard constructor
	BOOL BuildFromDescriptor( jeProperty_List *pArray );
	void Reset();
	BOOL UpdateFieldDataById( int Id, jeProperty_Data *pData, int DataSize );
	BOOL UpdateDataByArray( jeProperty_List *pArray );
	BOOL HandleComboMsg( FieldStruct* pField, int nCode );	

// Dialog Data
	//{{AFX_DATA(CDialogBuilderDlg)
	enum { IDD = IDD_PROPERTIEDLG };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDialogBuilderDlg)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);	
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CDialogBuilderDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown( UINT, CPoint );
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	
//	1.16.05 by tom morris - better support for new color button
	afx_msg LONG OnChangeColor(UINT lParam, LONG wParam);
		//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	int FieldN;
	FieldStruct * pFieldList;
	CFont		DlgFont;
	int	Bottom;
	int WndVScrollPos;
	CPoint	cpDragStart;

	BOOL AddStringField( FieldStruct * pField,  char * Name, int Id, int Type, float Min, float Max, float Increment );
	BOOL AddCheckBox( FieldStruct * pField, char * Name, int Id );
	BOOL AddGroupButton( FieldStruct * pField, char * Name, int Id );
	BOOL AddRadioButton( FieldStruct * pField, char * Name, int Id  );
	BOOL AddColorPicker( FieldStruct * pField, char * Name, int Id  );
	BOOL AddComboField( FieldStruct * pField,  char * Name, int Id, int StringN, char **StringList );
	BOOL AddStaticField( FieldStruct * pField,  char * Name, int Id, int Type );
	BOOL SetStringField( FieldStruct *pField, jeVec3d Vector, int DataSize );
	void SetRadioFieldGroup( FieldStruct * pField );
	BOOL AddField( FieldStruct * pField, char * Name, PROPERTY_FIELD_TYPE Type, int Id, int DataId, jeProperty_TypeInfo *pTypeInfo  );
	BOOL ShowField( FieldStruct* pField, int nCmdShow  );
	BOOL FormatField(FieldStruct* pField, int *Height, int Level );	
	BOOL FormatDialog();
	BOOL HandleFieldMsg( FieldStruct* pField, int nCode );
	BOOL HandleStringMsg( FieldStruct* pField, int nCode );
	BOOL HandleCheckMsg( FieldStruct* pField, int nCode );
	BOOL AddVec3dGroupButton( FieldStruct * pField, char * Name, int Id  );
	BOOL HandleGroupMsg( FieldStruct* pField, int nCode );
	BOOL SetStringField( FieldStruct *pField, char * Data, int DataSize );
	BOOL SetCheckField( FieldStruct *pField, int Bool, int DataSize );
	BOOL SetRadioField( FieldStruct *pField, int Bool, int DataSize );
	BOOL SetStaticIntField( FieldStruct *pField, int Int, int DataSize );
	BOOL SetIntField( FieldStruct *pField, int Int, int DataSize );
	BOOL SetFloatField( FieldStruct *pField, float Float, int DataSize );
	BOOL SetVec3dGroup( FieldStruct *pField, jeVec3d Vector, int DataSize );
	BOOL SetColorPickerField(  FieldStruct *pField, jeVec3d Vector, int DataSize );
	BOOL SetRGBGroup( FieldStruct *pField, jeVec3d Vector, int DataSize );
	BOOL SetComboField( FieldStruct *pField, char * String, int DataSize );
	BOOL HandleIntMsg( FieldStruct* pField, int nCode );
	BOOL HandleFloatMsg( FieldStruct* pField, int nCode );
	BOOL UpdateFieldData( FieldStruct *pField, jeProperty_Data *pData, int DataSize );
	void SetScollBar();
	void DisableField( FieldStruct * pField );
	FieldStruct * FindFieldById( int Id );
	void OnEnKillFocus();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DIALOGBUILDERDLG_H__0B24B489_1A7F_11D3_B323_004033AA0441__INCLUDED_)
