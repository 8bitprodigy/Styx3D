/****************************************************************************************/
/*  TextureListControl.cpp                                                                */
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

#include "stdafx.h"
#include "jwe.h"
#include "MainFrm.h"
#include "TextureListControl.h"
#include "TexturesDlg.h"
#include "bitmap.h"
#include "afxole.h"



// CTextureListControl

IMPLEMENT_DYNAMIC(CTextureListControl, CListCtrl)
CTextureListControl::CTextureListControl()
{
	m_pSelectedMaterial = NULL;
}

CTextureListControl::~CTextureListControl()
{
		if (m_wndProperties.m_hWnd)
			m_wndProperties.DestroyWindow();
}


BEGIN_MESSAGE_MAP(CTextureListControl, CListCtrl)
	ON_NOTIFY_REFLECT(LVN_ITEMCHANGED, OnLvnItemchangedListTextures)
	ON_NOTIFY_REFLECT(NM_CLICK, OnNMClickListTextures)
	ON_NOTIFY_REFLECT(LVN_BEGINDRAG, OnLvnBegindrag)
	ON_WM_LBUTTONDOWN()
	ON_WM_SHOWWINDOW()
	ON_NOTIFY_REFLECT(NM_RCLICK, OnNMRclick)
END_MESSAGE_MAP()



// CTextureListControl message handlers

void	CTextureListControl::ActivateHighlightedTexture(LPNMLISTVIEW pNMLV)
{
	CJweApp				*pApp = (CJweApp*) AfxGetApp();
	Material_Struct		*pMaterial = NULL;
	//MaterialIterator	MI ;
	CTexturesDlg		*pParent = NULL;
	pParent = (CTexturesDlg*)GetParent();

	if (pNMLV)
	{
		if (pParent)
		{
			if (!pParent->GetWorkerThreadActivity())
			{
				m_strSelectedTexture = GetItemText(pNMLV->iItem, 0);

				if (pParent->IsKindOf(RUNTIME_CLASS(CTexturesDlg)))
				{
					pParent->SetSelectedTexture(m_strSelectedTexture, this);
                    m_pSelectedMaterial = MaterialList_GetCurMaterial(pApp->GetMaterialList());
				}
			}	//	if (!pParent->GetWorkerThreadActivity())...
		}	//	if (pParent)...
	}	//	if (pNMLV)...
}



void CTextureListControl::OnLvnItemchangedListTextures(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	// TODO: Add your control notification handler code here

	ActivateHighlightedTexture(pNMLV);

	*pResult = 0;
}


void CTextureListControl::OnNMClickListTextures(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	// TODO: Add your control notification handler code here

	ActivateHighlightedTexture(pNMLV);

	if (m_wndProperties.m_hWnd)
	{
		if (m_wndProperties.IsWindowVisible())
			m_wndProperties.ShowWindow(SW_HIDE);
	}
	// TODO: Add your control notification handler code here
	*pResult = 0;
}


DROPEFFECT CTextureListControl::StartDragging(LPCTSTR/*DWORD*/ Data, RECT * rClient, CPoint * MousePos)
{
	DROPEFFECT		dropEffect = NULL;
	COleDataSource	*pDataSource = NULL;

	pDataSource = new COleDataSource;

	if (pDataSource)
	{
		HGLOBAL	hgData = NULL;
		hgData = ::GlobalAlloc(GMEM_MOVEABLE, lstrlen(Data)+1);
		if (hgData != NULL)
		{
			LPCSTR  lpData = NULL;
			lpData = (LPCSTR)GlobalLock(hgData);
			if (lpData != NULL)
			{
				lstrcpy((char*)lpData, (char*)Data);
				GlobalUnlock(hgData);
				pDataSource->CacheGlobalData(CF_TEXT, hgData);   
  
				DROPEFFECT dropEffect = pDataSource->DoDragDrop(DROPEFFECT_COPY|DROPEFFECT_MOVE |DROPEFFECT_SCROLL);  

				if((dropEffect&DROPEFFECT_MOVE)==DROPEFFECT_MOVE)
				{;;} //       do something();

				LPARAM	lparam = NULL;

				lparam = MousePos->y; 
				lparam = lparam<<16;
				lparam &= MousePos->x;                         
    
				SendMessage(WM_LBUTTONUP,0,lparam);
			}	//	if (lpData!=NULL)...
		}	//	if (hgData != NULL)...

		pDataSource->Empty();
		delete pDataSource;
	}	//	if (pDataSource)...

	return dropEffect;
}


void CTextureListControl::OnLvnBegindrag(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	// TODO: Add your control notification handler code here
	m_cMoveCursor = LoadCursor(AfxGetApp()->m_hInstance,MAKEINTRESOURCE(IDC_DRAGMOVE)); 
 	SetCursor(m_cMoveCursor);

	m_bIsDragging = true;

	POSITION	pos = NULL;
	pos = GetFirstSelectedItemPosition();
	m_iSelectedTexture = GetNextSelectedItem(pos);
	m_strSelectedTexture = GetItemText(m_iSelectedTexture, 0);
	
	RECT rClient;
	LPCSTR lpStrSelectedText = m_strSelectedTexture;
    GetClientRect(&rClient);

	CTexturesDlg	*pParent = NULL;
	pParent = (CTexturesDlg*)GetParent();
	if (pParent)
	{
		if (pParent->IsKindOf(RUNTIME_CLASS(CTexturesDlg)))
		{
			pParent->CollapseGroupsTree();
		}
	}

	StartDragging(lpStrSelectedText, &rClient, &m_ptClick);

	*pResult = 0;
}


void CTextureListControl::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	m_ptClick = point;

	CListCtrl::OnLButtonDown(nFlags, point);
}


void CTextureListControl::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CListCtrl::OnShowWindow(bShow, nStatus);

	// TODO: Add your message handler code here

	if (bShow)
	{
		if (GetSafeHwnd())
		{
			CRect	rectThis;
			GetWindowRect(&rectThis);
			m_rectProp.SetRect(rectThis.left,rectThis.top,rectThis.left+200,rectThis.top+200);

			if (!m_wndProperties.m_hWnd)
			{
				CWnd	*pParent = NULL;
				pParent = AfxGetMainWnd();

				if (pParent)
				{
// KROUER: make it compatible with VC6
#if _MFC_VER < 0x0700
					if (m_wndProperties.CreateEx(WS_EX_OVERLAPPEDWINDOW, WC_LISTVIEW, "", LVS_ALIGNLEFT | /*LVS_LIST*/LVS_REPORT |WS_CHILD,
												 m_rectProp.left, m_rectProp.top, m_rectProp.Width(), m_rectProp.Height(), pParent->GetSafeHwnd(), NULL))
#else
					if (m_wndProperties.CreateEx(WS_EX_OVERLAPPEDWINDOW      ,LVS_ALIGNLEFT | /*LVS_LIST*/LVS_REPORT  |WS_CHILD  ,m_rectProp, pParent,NULL))
#endif
					{
						m_wndProperties.InsertColumn(TEXTURE_PROPERTY_NAME,"Name",LVCFMT_LEFT,100);
						m_wndProperties.InsertColumn(TEXTURE_PROPERTY_HEIGHT,"Height",LVCFMT_RIGHT,50);
						m_wndProperties.InsertColumn(TEXTURE_PROPERTY_WIDTH,"Width",LVCFMT_RIGHT,50);
						m_wndProperties.InsertColumn(TEXTURE_PROPERTY_BITDEPTH,"Format",LVCFMT_RIGHT,200);
					}
				}
			}
		}
	}	//	if (bShow)...
	else
	{
		if (m_wndProperties.m_hWnd)
			m_wndProperties.DestroyWindow();
	}
}


void	CTextureListControl::SetSelectedTexture(CString	strTextureName)
{
	int			iNumberOfItems, 
				iScrollCalc, iCurrentScrollPos, 
				iScrollFromCurrent;
	int			iTextureDisplayWidth = 128;
	CSize		szScrollNULL, szScroll;
	float		fIterator = 0.0f;
	float		fScrollCalc = 0.0f;
	float		fNumberOfItems = 0.0f;
	float		fScrollMax = 0.0f;
	float		fCurrentScrollPos = 0.0f;

	iNumberOfItems = GetItemCount();
	fNumberOfItems = (float) iNumberOfItems;

	CTexturesDlg	*pParent = NULL;
	pParent = (CTexturesDlg*)GetParent();
	if (pParent)
	{
		if (pParent->IsKindOf(RUNTIME_CLASS(CTexturesDlg)))
		{
			iTextureDisplayWidth = pParent->GetTextureDisplayWidth();
		}
	}

	for (int i = 0; i < GetItemCount(); i++)
	{
		if (GetItemText(i, 0) == strTextureName)
		{
			SetItemState(i, LVIS_SELECTED|LVIS_FOCUSED, LVIS_SELECTED|LVIS_FOCUSED);

			switch (iTextureDisplayWidth)
			{
			case 32:
			case 64:
				break;
			case 128:

				iCurrentScrollPos = GetScrollPos(SB_VERT);
				fCurrentScrollPos = (float) iCurrentScrollPos;

				int	Imin, Imax;
				fIterator = (float) i;

				GetScrollRange(SB_VERT, &Imin, &Imax);
				fScrollMax = (float) Imax;
				fScrollCalc = ((fIterator/fNumberOfItems)*fScrollMax);
				iScrollCalc = (int) fScrollCalc;
				fScrollCalc -= fCurrentScrollPos;
				iScrollFromCurrent = (int) fScrollCalc;
				szScroll.cy = iScrollFromCurrent;
				Scroll(szScroll);
				RedrawWindow();

				break;
			default:
				break;

			}
		}
	}
}


void CTextureListControl::OnNMRclick(NMHDR *pNMHDR, LRESULT *pResult)
{
	CPoint	ptCursor;
	CRect	rectItem;

	GetCursorPos(&ptCursor);

	for (int i = 0; i < GetItemCount(); i++)
	{
		GetItemRect(i, &rectItem, LVIR_BOUNDS);
		ClientToScreen(&rectItem);

		if (rectItem.PtInRect(ptCursor))
		{
			if (m_wndProperties.m_hWnd)
			{
				m_wndProperties.DeleteAllItems();

#ifdef _USE_BITMAPS
				jeBitmap			*pBitmap = NULL;
				//	get the texture's bitmap
				pBitmap = (jeBitmap*) Materials_GetBitmap(m_pSelectedMaterial);
				if (pBitmap)
				{
					jeBitmap_Info	bmpInfo, bmpSecInfo;
					CString	strHeight, strWidth, strFormat;
					jeBitmap_GetInfo(pBitmap, &bmpInfo, &bmpSecInfo);

					switch (bmpInfo.Format)
					{
					case JE_PIXELFORMAT_8BIT:
						strFormat = "JE_PIXELFORMAT_8BIT";
						break;
					case JE_PIXELFORMAT_8BIT_GRAY:
						strFormat = "JE_PIXELFORMAT_8BIT_GRAY";
						break;
					case JE_PIXELFORMAT_16BIT_555_RGB:
						strFormat = "JE_PIXELFORMAT_16BIT_555_RGB";
						break;
					case JE_PIXELFORMAT_16BIT_555_BGR:
						strFormat = "JE_PIXELFORMAT_16BIT_555_BGR";
						break;
					case JE_PIXELFORMAT_16BIT_565_RGB:
						strFormat = "JE_PIXELFORMAT_16BIT_565_RGB";
						break;
					case JE_PIXELFORMAT_16BIT_565_BGR:
						strFormat = "JE_PIXELFORMAT_16BIT_565_BGR";
						break;
					case JE_PIXELFORMAT_16BIT_4444_ARGB:
						strFormat = "JE_PIXELFORMAT_16BIT_4444_ARGB";
						break;
					case JE_PIXELFORMAT_16BIT_1555_ARGB:
						strFormat = "JE_PIXELFORMAT_16BIT_1555_ARGB";
						break;
					case JE_PIXELFORMAT_24BIT_RGB:
						strFormat = "JE_PIXELFORMAT_24BIT_RGB";
						break;
					case JE_PIXELFORMAT_24BIT_BGR:
						strFormat = "JE_PIXELFORMAT_24BIT_BGR";
						break;
					case JE_PIXELFORMAT_24BIT_YUV:
						strFormat = "JE_PIXELFORMAT_24BIT_YUV";
						break;
					case JE_PIXELFORMAT_32BIT_RGBX:
						strFormat = "JE_PIXELFORMAT_32BIT_RGBX";
						break;
					case JE_PIXELFORMAT_32BIT_XRGB:
						strFormat = "JE_PIXELFORMAT_32BIT_XRGB";
						break;
					case JE_PIXELFORMAT_32BIT_BGRX:
						strFormat = "JE_PIXELFORMAT_32BIT_BGRX";
						break;
					case JE_PIXELFORMAT_32BIT_XBGR:
						strFormat = "JE_PIXELFORMAT_32BIT_XBGR";
						break;
					case JE_PIXELFORMAT_32BIT_RGBA:
						strFormat = "JE_PIXELFORMAT_32BIT_RGBA";
						break;
					case JE_PIXELFORMAT_32BIT_ARGB:
						strFormat = "JE_PIXELFORMAT_32BIT_ARGB";
						break;
					case JE_PIXELFORMAT_32BIT_BGRA:
						strFormat = "JE_PIXELFORMAT_32BIT_BGRA";
						break;
					case JE_PIXELFORMAT_32BIT_ABGR:
						strFormat = "JE_PIXELFORMAT_32BIT_ABGR";
						break;
					case JE_PIXELFORMAT_WAVELET:
						strFormat = "JE_PIXELFORMAT_WAVELET";
						break;
					default:
						strFormat = "Info not available";
						break;
					}

					strHeight.Format("%d", bmpInfo.Height);
					strWidth.Format("%d", bmpInfo.Width);
#else
				CString	strHeight, strWidth, strFormat;
				const jeMaterialSpec* pMatSpec = Materials_GetMaterialSpec(m_pSelectedMaterial);
				if (pMatSpec) {
					strFormat = "JE_PIXELFORMAT_24BIT_RGB";

					jeMaterialSpec_Thumbnail* pMatThumb = jeMaterialSpec_GetThumbnail(pMatSpec);

					strHeight.Format("%d", pMatThumb->height);
					strWidth.Format("%d", pMatThumb->width);
#endif
					rectItem.SetRect(rectItem.left, rectItem.top, rectItem.left + 410, rectItem.top + 60);

					m_wndProperties.BringWindowToTop();
					m_wndProperties.MoveWindow(&rectItem);
					m_wndProperties.InsertItem(0, NULL);
					m_wndProperties.SetItemText(0,TEXTURE_PROPERTY_NAME, m_strSelectedTexture);
					m_wndProperties.SetItemText(0,TEXTURE_PROPERTY_HEIGHT, strHeight);
					m_wndProperties.SetItemText(0,TEXTURE_PROPERTY_WIDTH, strWidth);
					m_wndProperties.SetItemText(0,TEXTURE_PROPERTY_BITDEPTH, strFormat);

					m_wndProperties.ShowWindow(SW_SHOW);
				}
			}
			break;
		}
	}
	// TODO: Add your control notification handler code here
	*pResult = 0;
}


void CTextureListControl::OnCancel()
{
	if (m_wndProperties.m_hWnd)
	{
		if (m_wndProperties.IsWindowVisible())
			m_wndProperties.ShowWindow(SW_HIDE);
	}
}