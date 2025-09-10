/****************************************************************************************/
/*  GROUPTREECTRL.CPP                                                                   */
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

#include "stdafx.h"
#include "jwe.h"
#include "grouptreectrl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CGroupTreeCtrl

CGroupTreeCtrl::CGroupTreeCtrl()
{
	m_bDragging = FALSE;
	m_pimagelist = nullptr;
}

CGroupTreeCtrl::~CGroupTreeCtrl()
{
}


BEGIN_MESSAGE_MAP(CGroupTreeCtrl, CTreeCtrlEx)
	//{{AFX_MSG_MAP(CGroupTreeCtrl)
	ON_NOTIFY_REFLECT(TVN_BEGINDRAG, OnBegindrag)
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDOWN()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGroupTreeCtrl message handlers

void CGroupTreeCtrl::OnBegindrag(NMHDR* pNMHDR, LRESULT* pResult) 
{

	HTREEITEM	hitItemDrag{};
	HTREEITEM	hitItemParent{};
	CPoint      ptAction{};
	UINT        nFlags{};

	GetCursorPos(&ptAction);
	ScreenToClient(&ptAction);
	
	//((CTreeCtrlPage *)GetParent())->ShowNotification(pnmhdr, pLResult);
	ASSERT(!m_bDragging);
	hitItemDrag = HitTest(ptAction, &nFlags);
	hitItemParent = GetParentItem( hitItemDrag );
	//We can only drag leaf items for now.
	if( hitItemParent == nullptr )
		return;
	m_bDragging = TRUE; 
	m_hitemDrop = nullptr;
	if (GetSelectedCount()==1) {
		m_hitemDrag = hitItemDrag;
		m_pimagelist = CreateDragImage(m_hitemDrag);  // get the image list for dragging
	} else {
		m_hitemDrag = GetFirstSelectedItem();
		m_pimagelist = CreateDragImageEx();
	}

	if(m_pimagelist == nullptr)
		return;
	m_pimagelist->DragShowNolock(TRUE);
	m_pimagelist->SetDragCursorImage(0, CPoint(0, 0));
	m_pimagelist->BeginDrag(0, CPoint(0,0));
	m_pimagelist->DragMove(ptAction);
	m_pimagelist->DragEnter(this, ptAction);
	SetCapture();
	*pResult = 0;
	pNMHDR;
}

BOOL CGroupTreeCtrl::IsChildNodeOf(HTREEITEM hitemChild, HTREEITEM hitemSuspectedParent)
{
	do
	{
		if (hitemChild == hitemSuspectedParent)
			break;
	}
	while ((hitemChild = GetParentItem(hitemChild)) != nullptr);

	return (hitemChild != nullptr);
}

HTREEITEM CGroupTreeCtrl::TransferItem( HTREEITEM hitemDrag, HTREEITEM hitemDrop )
{
	TV_INSERTSTRUCT     tvstruct;
	TCHAR               sztBuffer[50];
	HTREEITEM hNewItem{};
	long Data{};

	tvstruct.item.hItem = hitemDrag;
	tvstruct.item.cchTextMax = 49;
	tvstruct.item.pszText = sztBuffer;
	tvstruct.item.mask = TVIF_CHILDREN | TVIF_HANDLE | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT;
	GetItem(&tvstruct.item);  // get information of the dragged element
	tvstruct.hParent = hitemDrop;
	tvstruct.hInsertAfter = TVI_SORT;
	tvstruct.item.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT;
	hNewItem = InsertItem(&tvstruct);
	Data = GetItemData( hitemDrag );
	SetItemData( hNewItem, Data );
	return( hNewItem );
}

void CGroupTreeCtrl::OnLButtonUp(UINT nFlags, CPoint point) 
{
	NM_TREEVIEW tv{};

	if (m_bDragging)
	{
		ASSERT(m_pimagelist != nullptr);
		m_pimagelist->DragLeave(this);
		m_pimagelist->EndDrag();
		delete m_pimagelist;
		m_pimagelist = nullptr;

		if ( m_hitemDrop != nullptr && // Cant drop  item to root
			m_hitemDrag != m_hitemDrop && //Cant drop item on self 
			!IsChildNodeOf(m_hitemDrop, m_hitemDrag) && //Cant drop item back in same group
			GetParentItem(m_hitemDrag) != m_hitemDrop &&
			GetParentItem(m_hitemDrop) == nullptr			//Drop Item must be group
			)
		{
			HTREEITEM hNewItem{};

            CWnd* pWnd = GetParent();

            HTREEITEM m_hitemDrag = GetFirstSelectedItem();
            while (m_hitemDrag) {
			    hNewItem = TransferItem(m_hitemDrag, m_hitemDrop);

                // Notify that selection has changed, by sending a TVM_INSERTITEM notification
			    //DeleteItem(m_hitemDrag); //Moved some lines down !!! JH
			    CWnd* pWnd = GetParent();
			    if ( pWnd )
			    {
				    tv.hdr.hwndFrom = GetSafeHwnd();
				    tv.hdr.idFrom = GetWindowLong( GetSafeHwnd(), GWL_ID );
				    tv.hdr.code = TVN_SELCHANGED;

				    tv.itemNew.hItem = hNewItem;
				    tv.itemNew.state = GetItemState( m_hitemDrag, 0xffffffff );
				    tv.itemNew.lParam = GetItemData( m_hitemDrag );
				    tv.itemNew.mask = TVIF_HANDLE|TVIF_STATE|TVIF_PARAM;

				    tv.action = TVC_DRAG_END /*TVC_UNKNOWN added by Brian */ ;

				    pWnd->SendMessage( WM_NOTIFY, tv.hdr.idFrom, (LPARAM)&tv );
			    }
                HTREEITEM oldItem = m_hitemDrag;
                m_hitemDrag = GetNextSelectedItem(m_hitemDrag);
    			DeleteItem(oldItem); // Moved, JH 25.3.2000 
            }
		}
		else
			MessageBeep(0);

		ReleaseCapture();
		m_bDragging = FALSE;
		SelectDropTarget(nullptr);
	}
	
	CTreeCtrlEx::OnLButtonUp(nFlags, point);
}

void CGroupTreeCtrl::OnMouseMove(UINT nFlags, CPoint point) 
{
	HTREEITEM           hitem{};
	UINT                flags{};

	if (m_bDragging)
	{
		ASSERT(m_pimagelist != nullptr);
		m_pimagelist->DragMove(point);
		if ((hitem = HitTest(point, &flags)) != nullptr)
		{
			m_pimagelist->DragLeave(this);
			SelectDropTarget(hitem);
			m_hitemDrop = hitem;
			m_pimagelist->DragEnter(this, point);
		}
	}
	
	CTreeCtrlEx::OnMouseMove(nFlags, point);
}

void CGroupTreeCtrl::OnLButtonDown(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	ptLButtonDown.x=-1;
	CTreeCtrlEx::OnLButtonDown(nFlags, point);
}

CImageList* CGroupTreeCtrl::CreateDragImageEx()
{
	// Find the bounding rectangle of all the selected items
	CRect		rectBounding; // Holds rectangle bounding area for bitmap
	CRect		rectFirstItem; // Holds first item's height and width
	CRect		rectTextArea;  // Holds text area of image
	
	int			nNumSelected; // Holds total number of selected items
	HTREEITEM	hItem = nullptr;
	
	CClientDC	DraggedNodeDC(this); // To draw drag image
	
	CDC			*pDragImageCalcDC = nullptr;	// to find the drag image width and height
	CString		strItemText;
    
	
	CBitmap		*pBitmapOldMemDCBitmap = nullptr; // Pointer to bitmap in memory
	
	CFont		*pFontOld = nullptr; // Used for  bitmap font
	int			nIdx = 0, nCounter = 0; // Counts array elements
	int			nMaxWidth = 0;		// holds the maximum width to be taken to form the bounding rect
	
	//UINT		uiSelectedItems;  // Holds an item
	CImageList	*pImageListDraggedNode = nullptr; // Holds an image list pointer
	
	nNumSelected = GetSelectedCount();
	if( nNumSelected > 0)
	{
		pDragImageCalcDC = GetDC();
		if(pDragImageCalcDC == nullptr)
			return nullptr;
		
		CImageList *pImageList = GetImageList(TVSIL_NORMAL);
		
		int cx,cy;
		
		ImageList_GetIconSize(*pImageList, &cx, &cy);
		
		// Calculate the maximum width of the bounding rectangle
		hItem = GetFirstSelectedItem();
		while (hItem)
		{
			// Get the item's height and width one by one
			strItemText = GetItemText(hItem);
			rectFirstItem.SetRectEmpty();
			pDragImageCalcDC->DrawText(strItemText, rectFirstItem, DT_CALCRECT);
			if(nMaxWidth < ( rectFirstItem.Width()+cx))
				nMaxWidth = rectFirstItem.Width()+cx;

			hItem = GetNextSelectedItem(hItem);
		}
		
		// Get the first item's height and width
		hItem = GetFirstSelectedItem();
		strItemText = GetItemText(hItem);
		rectFirstItem.SetRectEmpty();
		pDragImageCalcDC->DrawText(strItemText, rectFirstItem, DT_CALCRECT);
		ReleaseDC(pDragImageCalcDC);
		
		// Initialize textRect for the first item
		rectTextArea.SetRect(1, 1, nMaxWidth, rectFirstItem.Height());
		
		// Find the bounding rectangle of the bitmap
		rectBounding.SetRect(0,0, nMaxWidth+2, (rectFirstItem.Height()+2)*nNumSelected);
		
		CDC 		MemoryDC; // Memory Device Context used to draw the drag image
		// Create bitmap		
		if(!MemoryDC.CreateCompatibleDC(&DraggedNodeDC))
			return nullptr;
		CBitmap		DraggedNodeBmp; // Instance used for holding  dragged bitmap
		if(!DraggedNodeBmp.CreateCompatibleBitmap(&DraggedNodeDC, rectBounding.Width(), rectBounding.Height()))
			return nullptr;
		
		pBitmapOldMemDCBitmap = MemoryDC.SelectObject( &DraggedNodeBmp );
		pFontOld = MemoryDC.SelectObject(GetFont());
		
		CBrush brush(RGB(255,255,255));
		MemoryDC.FillRect(&rectBounding,&brush);
		MemoryDC.SetBkColor(RGB(255,255,255));
		MemoryDC.SetBkMode(TRANSPARENT);
		MemoryDC.SetTextColor(RGB(0,0,0));
		
		// Search through array list
		hItem = GetFirstSelectedItem();
		while (hItem)
		{
			int nImg = 0,nSelImg=0;
			GetItemImage(hItem,nImg,nSelImg);
			HICON hIcon = pImageList->ExtractIcon(nImg);
			//cdcMemory.DrawIcon(rectTextArea.left,rectTextArea.top,hIcon);
			MemoryDC.MoveTo(rectTextArea.left,rectTextArea.top);
			if( nIdx != nNumSelected-1 )
			{
				MemoryDC.LineTo(rectTextArea.left,rectTextArea.top+18);
			}
			else
			{
				MemoryDC.LineTo(rectTextArea.left,rectTextArea.top+8);
			}
			MemoryDC.MoveTo(rectTextArea.left,rectTextArea.top+8);
			MemoryDC.LineTo(rectTextArea.left+5,rectTextArea.top+8);
			
			int nLeft = rectTextArea.left;
			rectTextArea.left += 3;
			::DrawIconEx(MemoryDC.m_hDC,rectTextArea.left,rectTextArea.top,hIcon,
				16,16,0,nullptr,DI_NORMAL);
			rectTextArea.left += cx;
			MemoryDC.Rectangle(rectTextArea);
			MemoryDC.DrawText(GetItemText(hItem), rectTextArea, DT_LEFT| DT_SINGLELINE|DT_NOPREFIX);
			rectTextArea.left = nLeft;
			rectTextArea.OffsetRect(0, rectFirstItem.Height()+2);
			DestroyIcon(hIcon);
			hItem = GetNextSelectedItem(hItem);
		}

		MemoryDC.SelectObject( pFontOld );
		MemoryDC.SelectObject( pBitmapOldMemDCBitmap );
		MemoryDC.DeleteDC();
		
		// Create imagelist
		pImageListDraggedNode = new CImageList;
		pImageListDraggedNode->Create(rectBounding.Width(), rectBounding.Height(), 
			ILC_COLOR | ILC_MASK, 0, 1);
		
		pImageListDraggedNode->Add(&DraggedNodeBmp, RGB(255, 255,255)); 
		return pImageListDraggedNode;
	}
	return nullptr;
}
