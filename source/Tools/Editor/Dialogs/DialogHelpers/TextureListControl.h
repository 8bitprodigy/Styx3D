/****************************************************************************************/
/*  TextureListControl.h                                                                */
/*                                                                                      */
/*  Author: Tom Morris                                                                  */
/*  Description:                                                                        */
/*  Date: Feb. 2005                                                                     */
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

#pragma once

// CTextureListControl

typedef enum
{
	TEXTURE_PROPERTY_NAME,
	TEXTURE_PROPERTY_HEIGHT,
	TEXTURE_PROPERTY_WIDTH,
	TEXTURE_PROPERTY_BITDEPTH,
	TEXTURE_PROPERTY_LAST
} TEXTURE_PROPERTY ;

class CTextureListControl : public CListCtrl
{
	DECLARE_DYNAMIC(CTextureListControl)

public:
	CTextureListControl();
	virtual ~CTextureListControl();



protected:
	DECLARE_MESSAGE_MAP()


private:
	MaterialList_Struct	*m_pMaterialList;
	HCURSOR				m_cMoveCursor;
	bool				m_bIsDragging;
	CPoint				m_ptClick;
	int					m_iSelectedTexture;
	Material_Struct		*m_pSelectedMaterial;
	CString				m_strSelectedTexture;
	CListCtrl			m_wndProperties;
	CRect				m_rectProp;


	DROPEFFECT	StartDragging(LPCTSTR/*DWORD*/ Data, RECT * rClient, CPoint * MousePos);
	void		ActivateHighlightedTexture(LPNMLISTVIEW pNMLV);

	afx_msg void OnLvnItemchangedListTextures(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMClickListTextures(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnLvnBegindrag(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnNMRclick(NMHDR *pNMHDR, LRESULT *pResult);

public:
	void	SetSelectedTexture(CString	strTextureName);
	void OnCancel();
};


