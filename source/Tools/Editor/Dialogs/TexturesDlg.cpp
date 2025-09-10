/****************************************************************************************/
/*  TexturesDlg.cpp                                                                     */
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
#include "TexturesDlg.h"
#include "BitmapResize.h"
#include "Bmp.h"

// CTexturesDlg dialog

IMPLEMENT_DYNAMIC(CTexturesDlg, CDialog)
CTexturesDlg::CTexturesDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CTexturesDlg::IDD, pParent),   M_STR_IMAGEDIR("GlobalMaterials"), M_STR_MASTER("Master")
{
	m_pMaterialList = NULL;
	m_iTextureWidth = 128;
	m_bBeingSelected = false;
	m_iSizingStart = GetTickCount();
	m_bSizing = false;
	m_bWorkerThreadActive = false;
	m_iNumberofGroups = 0;
	m_pthread = NULL;
}

CTexturesDlg::~CTexturesDlg()
{
	if (m_pthread)
	{
		m_pthread->Shutdown();
		m_pthread->Delete();
		m_pthread = NULL;
	}
}

void CTexturesDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_TEXTURES, m_ListCtrlTextures);
	DDX_Control(pDX, IDC_TREE_TEX_GROUPS, m_treeCtrlGroups);
}


BEGIN_MESSAGE_MAP(CTexturesDlg, CDialog)
	ON_WM_SHOWWINDOW()
	ON_BN_CLICKED(IDC_RADIO_TEX_32, OnBnClickedRadioTex32)
	ON_BN_CLICKED(IDC_RADIO_TEX_64, OnBnClickedRadioTex64)
	ON_BN_CLICKED(IDC_RADIO_TEX_128, OnBnClickedRadioTex128)
	ON_WM_CREATE()
	ON_WM_CONTEXTMENU()
	ON_WM_SIZE()
	ON_COMMAND(IDCANCEL, OnCancel)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_BUTTON_TEX_REFRESH, OnBnClickedButtonTexRefresh)
END_MESSAGE_MAP()


// CTexturesDlg message handlers

BOOL CTexturesDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	OleInitialize(NULL);
	BOOL br = m_DropTarget.Register(this);
	// Remove this if you use br
	UNREFERENCED_PARAMETER(br);

	// TODO:  Add extra initialization here
	CJweApp		*pApp = NULL;
	pApp = (CJweApp*)AfxGetApp();
	if (pApp)
	{
		//	initialize the groups tree
		m_htRoot = m_treeCtrlGroups.InsertItem(M_STR_IMAGEDIR, NULL);
		m_htRoot = m_treeCtrlGroups.GetRootItem();
		m_treeCtrlGroups.EnsureVisible(m_htRoot);
		m_htMaster = m_treeCtrlGroups.InsertItem("Master", m_htRoot);
		m_htCurrentGroup = m_htMaster;

		//	rebuild the texture groups from data stored in registry
		RestoreTexturesGroupsAll();

		((CButton*)GetDlgItem(IDC_RADIO_TEX_128))->SetCheck(1);
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


void	CTexturesDlg::RestoreTexturesGroupsAll()
{
	m_htRoot = m_treeCtrlGroups.GetRootItem();
	
	//	get texture group data from registry and store it in m_stringListTexturesGroups
	Settings_RestoreTexturesGroups(&m_stringListTexturesGroups);

	if (m_stringListTexturesGroups.GetCount() > 0)
	{
		POSITION	pos = NULL;
		pos = m_stringListTexturesGroups.GetHeadPosition();
		while (pos)
		{
			CString	strGroup;
			strGroup = m_stringListTexturesGroups.GetNext(pos);
			if (strGroup.GetLength() >0)
			{
				CStringList	tempStrList;
				HTREEITEM	htGroup = NULL;
				htGroup = m_treeCtrlGroups.InsertItem(strGroup.GetBuffer(strGroup.GetLength()), m_htRoot);

				if (strGroup == M_STR_MASTER)
				{
					m_htMaster = htGroup;
				}

				m_iNumberofGroups++;

				Settings_RestoreTexturesInGroup(strGroup, &tempStrList);
				if (tempStrList.GetCount() > 0)
				{
					POSITION	pos = NULL;
					pos = tempStrList.GetHeadPosition();
					while (pos)
					{
						CString strTextureName;
						strTextureName = tempStrList.GetNext(pos);
						if (strTextureName.GetLength() > 0)
						{
							m_treeCtrlGroups.InsertItem(strTextureName, htGroup);
						}
					}
				}
			}
		}
	}
	m_treeCtrlGroups.Expand(m_htRoot,TVE_EXPAND);
	m_treeCtrlGroups.SelectItem(m_htMaster);
}


void	CTexturesDlg::SaveTexturesGroupsAll()
{
	HTREEITEM	htGroupItem = NULL;
	HTREEITEM	htTextureItem = NULL;
	CStringList	strGroupList, strTextureList;

	Settings_ResetTextureGroups();

	htGroupItem = m_treeCtrlGroups.GetNextItem(m_treeCtrlGroups.GetRootItem(), TVGN_CHILD);	
	while (htGroupItem)
	{
		if ((m_treeCtrlGroups.GetItemText(htGroupItem)) != M_STR_MASTER)
		{	
			htTextureItem = NULL;
			strTextureList.RemoveAll();
			htTextureItem = m_treeCtrlGroups.GetNextItem(htGroupItem, TVGN_CHILD);	
			while (htTextureItem)
			{
				strTextureList.AddTail(m_treeCtrlGroups.GetItemText(htTextureItem));
				htTextureItem = m_treeCtrlGroups.GetNextSiblingItem(htTextureItem);	
			}

			Settings_SetTexturesInGroup(m_treeCtrlGroups.GetItemText(htGroupItem), &strTextureList);

			strGroupList.AddTail(m_treeCtrlGroups.GetItemText(htGroupItem));
		}
		htGroupItem = m_treeCtrlGroups.GetNextSiblingItem(htGroupItem);	
	}

	Settings_SetTexturesGroups(&strGroupList);
}


bool	CTexturesDlg::InitGroupImageList()
{
	CJweApp				*pApp = NULL;
	CString				strMaterialName;
	int					iNumberofMaterials, iDivider, iRange;
	Material_Struct		*pMaterial = NULL;
	MaterialIterator	MI = NULL;
	CBitmapResize		bmpResizer;	//	a very useful bmp utility class
	BOOL				bResult;
	LPSTR				strProgressStart = _T("Preparing thumbnails...");

	iDivider = 20;

	pApp = (CJweApp*)AfxGetApp();
	if (pApp)
	{

		bResult = pApp->PostThreadMessage(IDM_PROGRESS_CONTROL_CREATE, (WPARAM)strProgressStart, NULL);
		// hold the window update to avoid flicking
		m_ListCtrlTextures.SetRedraw(FALSE);

		// reset our image list
		if (m_ImageListGroup.m_hImageList)
		{
			for (int i = m_ImageListGroup.GetImageCount(); i > -1; i--)
				m_ImageListGroup.Remove(i);	

			if (!m_ImageListGroup.DeleteImageList())
				m_ImageListGroup.SetImageCount(0);
		}

		//	create a new image list
		m_ImageListGroup.Create(m_iTextureWidth, m_iTextureWidth, ILC_COLOR24, 0, 1);

		//	get the texture materialas list from the jet app
		m_pMaterialList = NULL;
		m_pMaterialList = pApp->GetMaterialList();

		if (m_pMaterialList)
		{
			int ib = 0;

			//	find out how many textures are in the list
// KROUER: make it compatible with VC6
#if _MFC_VER < 0x0700
			iNumberofMaterials = m_strArrayCurGroup.GetSize();
#else
			iNumberofMaterials = m_strArrayCurGroup.GetCount();
#endif
			iRange = iNumberofMaterials/iDivider;

			bResult = pApp->PostThreadMessage(IDM_PROGRESS_CONTROL_SETRANGE, (WPARAM)iRange, (LPARAM)strProgressStart);
			bResult = pApp->PostThreadMessage(IDM_PROGRESS_CONTROL_SETSTEP, (WPARAM)1, (LPARAM)strProgressStart);
			bResult = pApp->PostThreadMessage(IDM_PROGRESS_CONTROL_STEP, (WPARAM)strProgressStart, NULL);

			// set the size of the image list accordingly
			m_ImageListGroup.SetImageCount(iNumberofMaterials);

			//	get a pointer to the first texture on the list
			pMaterial = NULL;

			CBitmap*			pImage = NULL;	
			HBITMAP				hBitmap = NULL;
#ifdef _USE_BITMAPS
			jeBitmap			*pBitmap = NULL;
#else
			jeMaterialSpec		*pMatSpec = NULL;
#endif
            HDC					thisHDC = ::GetDC(NULL);
			int					iIncrementCounter = 0;

			for (ib = 0; ib < iNumberofMaterials; ib++)
			{
				pImage = NULL;	
				hBitmap = NULL;
#ifdef _USE_BITMAPS
				pBitmap = NULL;
#else
				pMatSpec = NULL;
#endif

				CString	strTempMaterialName = m_strArrayCurGroup.GetAt(ib);
				pMaterial = MaterialList_SearchByName(m_pMaterialList, &MI, strTempMaterialName.GetBuffer(strTempMaterialName.GetLength()));

				if (pMaterial)
				{
#ifdef _USE_BITMAPS
					//	get the texture's bitmap
					pBitmap = (jeBitmap*) Materials_GetBitmap(pMaterial);

					if (pBitmap)
					{
						//	create a DIB and get its handle
						hBitmap = CreateHBitmapFromgeBitmap (pBitmap, thisHDC);

						if (hBitmap)
						{
							//	resize this bitmap so it fits our list
							//	save the resulting handel for the next step
							hBitmap = bmpResizer.ScaleBitmapInt(hBitmap, m_iTextureWidth, m_iTextureWidth);

							//	create a new CBitmap object to be used in our imagelist
							pImage = new CBitmap();	
						//	if (pImage)
							{
								// attach the jet texture's handle to the CBitmap object
								pImage->Attach((HBITMAP)hBitmap);

								// add bitmap to our image list
								m_ImageListGroup.Replace(ib, pImage, NULL);

								//	cleanup
								delete pImage;
								DeleteObject(hBitmap);

								iIncrementCounter++;
								if (iIncrementCounter == iDivider)
								{
									bResult = pApp->PostThreadMessage(IDM_PROGRESS_CONTROL_STEP, (WPARAM)strProgressStart, NULL);
									iIncrementCounter = 0;
								}

							}	//	if (pImage)
							//	cleanup

						}	//	if (hBitmap)...
					}	//	if (pBitmap)...
#else
					// get the material and its thumbnail
					pMatSpec = (jeMaterialSpec*) Materials_GetMaterialSpec(pMaterial);
					if (pMatSpec) {
						BITMAPINFOHEADER bitmapInfo;
						ZeroMemory(&bitmapInfo, sizeof(BITMAPINFOHEADER));

						jeMaterialSpec_Thumbnail* pThumb = jeMaterialSpec_GetThumbnail(pMatSpec);
						
						if (pThumb) {
					        bitmapInfo.biSize = sizeof(BITMAPINFOHEADER);
					        bitmapInfo.biBitCount = 24;
					        bitmapInfo.biCompression = BI_RGB;
					        bitmapInfo.biHeight = pThumb->height;
					        bitmapInfo.biWidth = pThumb->width;
					        bitmapInfo.biPlanes = 1;
					        bitmapInfo.biSizeImage = pThumb->width * pThumb->height * 3;
							
							hBitmap = CreateDIBitmap(thisHDC, &bitmapInfo, CBM_INIT, pThumb->contents, (BITMAPINFO *)&bitmapInfo, DIB_RGB_COLORS);
							if (hBitmap) {
								//	resize this bitmap so it fits our list
								//	save the resulting handel for the next step
								hBitmap = bmpResizer.ScaleBitmapInt(hBitmap, m_iTextureWidth, m_iTextureWidth);

								//	create a new CBitmap object to be used in our imagelist
								pImage = new CBitmap();	
								
								// attach the jet texture's handle to the CBitmap object
								pImage->Attach((HBITMAP)hBitmap);

								// add bitmap to our image list
								m_ImageListGroup.Replace(ib, pImage, NULL);

								//	cleanup
								delete pImage;
								DeleteObject(hBitmap);

								iIncrementCounter++;
								if (iIncrementCounter == iDivider)
								{
									bResult = pApp->PostThreadMessage(IDM_PROGRESS_CONTROL_STEP, (WPARAM)strProgressStart, NULL);
									iIncrementCounter = 0;
								}
							}
						}
					}
#endif
					pMaterial = NULL;
				}	//	if (pMaterial)...
			}	//	while (pos)...
		}	//	else

		bResult = 	pApp->PostThreadMessage(IDM_PROGRESS_CONTROL_DESTROY, NULL, NULL);
	}
	return true;
}


bool	CTexturesDlg::InitMasterImageLists()
{
	CJweApp				*pApp = NULL;
	CString				strMaterialName;
	int					iNumberofMaterials;
	Material_Struct		*pMaterial = NULL;
	MaterialIterator	MI = NULL;
	CBitmapResize		bmpResizer;	//	a very useful bmp utility class
	BOOL				bResult;
	LPSTR				strProgressStart = _T("Preparing thumbnails...");

	pApp = (CJweApp*)AfxGetApp();
	if (pApp)
	{
		bResult = pApp->PostThreadMessage(IDM_PROGRESS_CONTROL_CREATE, (WPARAM)strProgressStart, NULL);

		// hold the window update to avoid flicking
		m_ListCtrlTextures.SetRedraw(FALSE);
		m_treeCtrlGroups.SetRedraw(FALSE);
		m_strArrayMaster.RemoveAll();

		HTREEITEM	htItem = NULL;
		htItem = m_treeCtrlGroups.GetChildItem(m_htMaster);
		while (htItem)
		{
			m_treeCtrlGroups.DeleteItem(htItem);
			htItem = NULL;
			htItem = m_treeCtrlGroups.GetChildItem(m_htMaster);
		}

		// reset our image list
		if (m_ImageList32.m_hImageList)
		{
			for (int i = m_ImageList32.GetImageCount(); i > -1; i--)
				m_ImageList32.Remove(i);	

			if (!m_ImageList32.DeleteImageList())
				m_ImageList32.SetImageCount(0);
		}

		if (m_ImageList64.m_hImageList)
		{
			for (int i = m_ImageList64.GetImageCount(); i > -1; i--)
				m_ImageList64.Remove(i);	

			if (!m_ImageList64.DeleteImageList())
				m_ImageList64.SetImageCount(0);
		}

		if (m_ImageList128.m_hImageList)
		{
			for (int i = m_ImageList128.GetImageCount(); i > -1; i--)
				m_ImageList128.Remove(i);	

			if (!m_ImageList128.DeleteImageList())
				m_ImageList128.SetImageCount(0);
		}

		//	create a new image list
		m_ImageList32.Create(32, 32, ILC_COLOR16, 0, 4);
		m_ImageList64.Create(64, 64, ILC_COLOR16, 0, 4);
		m_ImageList128.Create(128, 128, ILC_COLOR16, 0, 4);

		//	get the texture materialas list from the jet app
		m_pMaterialList = NULL;
		m_pMaterialList = pApp->GetMaterialList();

		if (m_pMaterialList)
		{
			int i = 0;	

			//	find out how many textures are in the list
			iNumberofMaterials = MaterialList_GetNumItems(m_pMaterialList);

			bResult = pApp->PostThreadMessage(IDM_PROGRESS_CONTROL_SETRANGE, (WPARAM)iNumberofMaterials, (LPARAM)strProgressStart);
			bResult = pApp->PostThreadMessage(IDM_PROGRESS_CONTROL_SETSTEP, (WPARAM)1, (LPARAM)strProgressStart);

			// set the size of the image list accordingly
			m_ImageList32.SetImageCount(iNumberofMaterials);
			m_ImageList64.SetImageCount(iNumberofMaterials);
			m_ImageList128.SetImageCount(iNumberofMaterials);

			//	get a pointer to the first texture on the list
			pMaterial = NULL;
			pMaterial = MaterialList_GetFirstMaterial( m_pMaterialList, &MI);

			CBitmap		*pImage = NULL;	
			HBITMAP		hBitmap = NULL;
#ifdef _USE_BITMAPS
			jeBitmap	*pBitmap = NULL;
#else
			jeMaterialSpec* pMatSpec = NULL;
			jeMaterialSpec_Thumbnail* pThumb = NULL;
#endif

			CBitmap		*pBitmap32 = NULL;
			CBitmap		*pBitmap64 = NULL;
			CBitmap		*pBitmap128 = NULL;

			HBITMAP		hBitmap32 = NULL;
			HBITMAP		hBitmap64 = NULL;
			HBITMAP		hBitmap128 = NULL;	

			HDC	thisHDC = GetDC()->GetSafeHdc();

			while (pMaterial)
			{
				pImage = NULL;	
				hBitmap = NULL;
#ifdef _USE_BITMAPS
				pBitmap = NULL;
#endif
				hBitmap32 = NULL;
				hBitmap64 = NULL;
				hBitmap128 = NULL;	

				bResult = pApp->PostThreadMessage(IDM_PROGRESS_CONTROL_STEP, (WPARAM)strProgressStart, NULL);

				//	find out the texture's name
				strMaterialName = Materials_GetName(pMaterial);
#ifdef _USE_BITMAPS
				//	get the texture's bitmap
				pBitmap = (jeBitmap*) Materials_GetBitmap(pMaterial);

				if (pBitmap)
				{
					//	create a DIB and get its handle
					hBitmap32 = CreateHBitmapFromgeBitmap (pBitmap, thisHDC);
					hBitmap64 = CreateHBitmapFromgeBitmap (pBitmap, thisHDC);
					hBitmap128 = CreateHBitmapFromgeBitmap (pBitmap, thisHDC);

					//	resize this bitmap so it fits our list
					//	save the resulting handel for the next step
					hBitmap32 = bmpResizer.ScaleBitmapInt(hBitmap32, 32, 32);
					hBitmap64 = bmpResizer.ScaleBitmapInt(hBitmap64, 64, 64);
					hBitmap128 = bmpResizer.ScaleBitmapInt(hBitmap128, 128, 128);

					//	create a new CBitmap object to be used in our imagelist
					pBitmap32 = NULL;
					pBitmap64 = NULL;
					pBitmap128 = NULL;
					pBitmap32 = new CBitmap();	
					pBitmap64 = new CBitmap();	
					pBitmap128 = new CBitmap();	

					// attach the jet texture's handle to the CBitmap object
					pBitmap32->Attach((HBITMAP)hBitmap32);
					pBitmap64->Attach((HBITMAP)hBitmap64);
					pBitmap128->Attach((HBITMAP)hBitmap128);

					// add bitmap to our image list
					m_ImageList32.Replace(i, pBitmap32, NULL);
					m_ImageList64.Replace(i, pBitmap64, NULL);
					m_ImageList128.Replace(i, pBitmap128, NULL);

					m_strArrayMaster.InsertAt(i, strMaterialName);
					m_treeCtrlGroups.InsertItem(strMaterialName, m_htMaster);

					//	get the next texture on the list
					pMaterial = NULL;
					pMaterial = MaterialList_GetNextMaterial( m_pMaterialList, &MI ) ;

					i++;

					//	cleanup
					delete pBitmap32;
					delete pBitmap64;
					delete pBitmap128;

					//	cleanup
					DeleteObject(hBitmap32);
					DeleteObject(hBitmap64);
					DeleteObject(hBitmap128);

				}	//	if (pBitmap)...
#else
				//	get the texture's bitmap
				pMatSpec = (jeMaterialSpec*) Materials_GetMaterialSpec(pMaterial);
				pThumb = jeMaterialSpec_GetThumbnail(pMatSpec);
				if (pThumb)
				{
					BITMAPINFOHEADER bitmapInfo;
					ZeroMemory(&bitmapInfo, sizeof(BITMAPINFOHEADER));

					bitmapInfo.biSize = sizeof(BITMAPINFOHEADER);
					bitmapInfo.biBitCount = 24;
					bitmapInfo.biCompression = BI_RGB;
					bitmapInfo.biHeight = pThumb->height;
					bitmapInfo.biWidth = pThumb->width;
					bitmapInfo.biPlanes = 1;
					bitmapInfo.biSizeImage = pThumb->width * pThumb->height * 3;

					//	create a DIB and get its handle
					hBitmap = CreateDIBitmap(GetDC()->GetSafeHdc(), &bitmapInfo, CBM_INIT, pThumb->contents, (BITMAPINFO *)&bitmapInfo, DIB_RGB_COLORS);

					if (hBitmap) {
						//	resize this bitmap so it fits our list
						//	save the resulting handel for the next step
						hBitmap32 = bmpResizer.ScaleBitmapInt(hBitmap, 32, 32);
						hBitmap64 = bmpResizer.ScaleBitmapInt(hBitmap, 64, 64);
						hBitmap128 = bmpResizer.ScaleBitmapInt(hBitmap, 128, 128);

						//	create a new CBitmap object to be used in our imagelist
						pBitmap32 = NULL;
						pBitmap64 = NULL;
						pBitmap128 = NULL;
						pBitmap32 = new CBitmap();	
						pBitmap64 = new CBitmap();	
						pBitmap128 = new CBitmap();	

						// attach the jet texture's handle to the CBitmap object
						pBitmap32->Attach((HBITMAP)hBitmap32);
						pBitmap64->Attach((HBITMAP)hBitmap64);
						pBitmap128->Attach((HBITMAP)hBitmap128);

						// add bitmap to our image list
						m_ImageList32.Replace(i, pBitmap32, NULL);
						m_ImageList64.Replace(i, pBitmap64, NULL);
						m_ImageList128.Replace(i, pBitmap128, NULL);

						m_strArrayMaster.InsertAt(i, strMaterialName);
						m_treeCtrlGroups.InsertItem(strMaterialName, m_htMaster);

						//	cleanup
						delete pBitmap32;
						delete pBitmap64;
						delete pBitmap128;

						DeleteObject(hBitmap32);
						DeleteObject(hBitmap64);
						DeleteObject(hBitmap128);

						DeleteObject(hBitmap);
					}

					//	get the next texture on the list
					pMaterial = NULL;
					pMaterial = MaterialList_GetNextMaterial( m_pMaterialList, &MI ) ;

					i++;
				}	//	if (pBitmap)...
#endif
			}	//	while (pMaterial)...
		}	//	if (m_htCurrentGroup == m_htMaster)...

		bResult = 	pApp->PostThreadMessage(IDM_PROGRESS_CONTROL_DESTROY, NULL, NULL);
	}	//	if (pApp)...

	m_treeCtrlGroups.SortChildren(m_htMaster);
	m_ListCtrlTextures.SetRedraw(TRUE);
	m_treeCtrlGroups.SetRedraw(TRUE);

	return true;
}


void	CTexturesDlg::DrawGroupThumbnailsFromWorkerThread()
{
	DrawThumbnails(&m_ImageListGroup, &m_strArrayCurGroup);
}


void  CTexturesDlg::DrawThumbnails(CImageList *pImageList, CStringArray	*pArray)
{
	m_ListCtrlTextures.SetRedraw(FALSE);

	//	assign this new image list to out list control
	m_ListCtrlTextures.SetImageList(pImageList, LVSIL_NORMAL);

	// remove all items from list control
	if(m_ListCtrlTextures.GetItemCount() > 0)
	{
		m_ListCtrlTextures.DeleteAllItems();
	}

	// KROUER: make it compatible with VC6
	long iNbTextures;
#if _MFC_VER < 0x0700
	iNbTextures = pArray->GetSize();
#else
	iNbTextures = pArray->GetCount();
#endif

	for (int i = 0; i < iNbTextures; i++)
	{
		m_ListCtrlTextures.InsertItem(i, pArray->GetAt(i), i);
	}

	//	arrange the spacing of texture thumbnails
	int iHOffset = GetSystemMetrics(SM_CXVSCROLL) + (10*GetSystemMetrics(SM_CXBORDER));

	//	adjust window positions on the dialog baaackground
	switch (m_iTextureWidth)
	{
	case 32:
		iHOffset += 3; 
		m_ListCtrlTextures.SetIconSpacing(m_iTextureWidth + 1, m_iTextureWidth + 19);
		break;
	case 64:
		iHOffset += 1;
		m_ListCtrlTextures.SetIconSpacing(m_iTextureWidth + 1, m_iTextureWidth + 19);
		break;
	case 128:
		m_ListCtrlTextures.SetIconSpacing(m_iTextureWidth + 1, m_iTextureWidth + 21);

		break;
	default:
		m_ListCtrlTextures.SetIconSpacing(m_iTextureWidth + 1, m_iTextureWidth +21);
	}

	// let's show the new thumbnails
	m_ListCtrlTextures.SetRedraw(TRUE); 

}


void CTexturesDlg::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CDialog::OnShowWindow(bShow, nStatus);

	SendMessageToDescendants(WM_SHOWWINDOW, bShow, nStatus);

	// TODO: Add your message handler code here
	// TODO: Add your control notification handler code here
	if (bShow && !m_bWorkerThreadActive)
	{
		if (GetSafeHwnd())
		{
			if (m_htCurrentGroup == m_htMaster)
			{
				if (!m_ImageList32.m_hImageList)
				{
					StartMasterWorkerThread();
					return;
				}

				switch (m_iTextureWidth)
				{
				case 32:
					DrawThumbnails(&m_ImageList32, &m_strArrayMaster);
					break;
				case 64:
					DrawThumbnails(&m_ImageList64, &m_strArrayMaster);
					break;
				case 128:
					DrawThumbnails(&m_ImageList128, &m_strArrayMaster);
					break;
				default:
					m_iTextureWidth = 128;
					DrawThumbnails(&m_ImageList128, &m_strArrayMaster);
					break;
				}
			}	//	if (m_htCurrentGroup == m_htMaster)...
			else
				StartGroupWorkerThread();

		}	//	if (GetSafeHwnd())...
	}	//	if (bShow && !m_bWorkerThreadActive)...
}


void CTexturesDlg::OnBnClickedRadioTex32()
{
	m_iTextureWidth = 32;

	if (m_hWnd)
	{
		if (IsWindowVisible())
		{
			if (m_htCurrentGroup == m_htMaster)
			{
				DrawThumbnails(&m_ImageList32, &m_strArrayMaster);
			}
			else
			{
				StartGroupWorkerThread();
			}
		}
	}
}


void CTexturesDlg::OnBnClickedRadioTex64()
{
	m_iTextureWidth	= 64;
	if (m_hWnd)
	{
		if (IsWindowVisible())
		{	
			if (m_htCurrentGroup ==	m_htMaster)
			{
				DrawThumbnails(&m_ImageList64, &m_strArrayMaster);
			}
			else
			{
				StartGroupWorkerThread();
			}
		}
	}
}

void CTexturesDlg::OnBnClickedRadioTex128()
{
	m_iTextureWidth = 128;

	if (m_hWnd)
	{
		if (IsWindowVisible())
		{
			if (m_htCurrentGroup == m_htMaster)
			{
				DrawThumbnails(&m_ImageList128, &m_strArrayMaster);
			}
			else
			{
				StartGroupWorkerThread();
			}
		}
	}
}


int CTexturesDlg::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDialog::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  Add your specialized creation code here

	return 0;
}


void CTexturesDlg::OnContextMenu(CWnd* pWnd, CPoint point)
{
	CPoint	ptCursor;
	CRect	rectTreeList;

	m_treeCtrlGroups.GetWindowRect(&rectTreeList);
	GetCursorPos(&ptCursor);

	if (rectTreeList.PtInRect(ptCursor))
		m_treeCtrlGroups.OnContextMenu(pWnd, point);
	// TODO: Add your message handler code here
}


void CTexturesDlg::OnIdrDeleteitem()
{
	CRect	rectThis;
	m_treeCtrlGroups.DeleteSelectedItem();
	m_iNumberofGroups--;
	GetClientRect(&rectThis);
	OnSize(SIZE_RESTORED,rectThis.Width(), rectThis.Height());

}


void CTexturesDlg::OnUpdateIdrDeleteitem(CCmdUI *pCmdUI)
{
	m_treeCtrlGroups.OnUpdateIdrDeleteitem(pCmdUI);
}


void CTexturesDlg::OnIdrAddgroup()
{
	CRect	rectThis;
	m_treeCtrlGroups.OnIdrAddgroup();
	m_iNumberofGroups++;
	GetClientRect(&rectThis);
	OnSize(SIZE_RESTORED,rectThis.Width(), rectThis.Height());
}


void CTexturesDlg::OnUpdateIdrAddgroup(CCmdUI *pCmdUI)
{
	m_treeCtrlGroups.OnUpdateIdrAddgroup(pCmdUI);
}


void CTexturesDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);

	// TODO: Add your message handler code here
	CRect	rectTree, rectList, rectParent, rectThis;
	CWnd	*pParent = NULL;
	CWnd	*pGreatGrandParent = NULL;
	DWORD	iCurrentTime, iDifference;
	int		iStatusBarHeight = 70;
	int		iTreeCtrlHeight = (m_iNumberofGroups + 2)*17;
	
	if (iTreeCtrlHeight < 80) iTreeCtrlHeight = 80;

	iCurrentTime = GetTickCount();

	if (m_treeCtrlGroups.m_hWnd)
	{
		pGreatGrandParent = GetParent()->GetParent()->GetParent();
		pParent = GetParent()->GetParent();

		if (pParent)
		{
			pParent->GetClientRect(&rectParent);
			m_treeCtrlGroups.GetClientRect(&rectTree);
			m_ListCtrlTextures.GetClientRect(&rectList);

			rectTree.right = rectParent.right;
			rectTree.SetRect(0,0,rectParent.right-10,iTreeCtrlHeight);
			m_treeCtrlGroups.MoveWindow(&rectTree);
			m_treeCtrlGroups.GetClientRect(&rectTree);
			pParent->GetClientRect(&rectParent);
			rectList.SetRect(0,rectTree.bottom + 4, rectParent.right-10, rectParent.Height() - iStatusBarHeight);

			m_ListCtrlTextures.MoveWindow(&rectList);
			m_ListCtrlTextures.GetClientRect(&rectList);
			pParent->GetClientRect(&rectParent);

			CWnd	*pWnd = NULL;
			CRect	rect32, rect64, rect128, rectRefresh;
			rect32.SetRect(rectParent.left, rectParent.Height()- iStatusBarHeight, rectParent.left + 30, (rectParent.Height()- iStatusBarHeight) + 20);
			rect64.SetRect(rectParent.left + 32, rectParent.Height()- iStatusBarHeight, rectParent.left + 30 + 32, (rectParent.Height()- iStatusBarHeight) + 20);
			rect128.SetRect(rectParent.left + 64, rectParent.Height()- iStatusBarHeight, rectParent.left + 30 + 64, (rectParent.Height()- iStatusBarHeight) + 20);
			rectRefresh.SetRect(rectParent.left + 96, rectParent.Height()- iStatusBarHeight, rectParent.left + 50 + 96, (rectParent.Height()- iStatusBarHeight) + 20);

			pWnd = GetDlgItem(IDC_RADIO_TEX_32);
			if (pWnd)
				pWnd->MoveWindow(&rect32);

			pWnd = NULL;
			pWnd = GetDlgItem(IDC_RADIO_TEX_64);
			if (pWnd)
				pWnd->MoveWindow(&rect64);

			pWnd = NULL;
			pWnd = GetDlgItem(IDC_RADIO_TEX_128);
			if (pWnd)
				pWnd->MoveWindow(&rect128);

			pWnd = NULL;
			pWnd = GetDlgItem(IDC_BUTTON_TEX_REFRESH);
			if (pWnd)
				pWnd->MoveWindow(&rectRefresh);

			iDifference = iCurrentTime - m_iSizingStart;

			if (GetCapture() == pGreatGrandParent)
			{
				if (m_htCurrentGroup == m_htMaster)
				{
					switch (m_iTextureWidth)
					{
					case 32:
						DrawThumbnails(&m_ImageList32, &m_strArrayMaster);
						break;
					case 64:
						DrawThumbnails(&m_ImageList64, &m_strArrayMaster);
						break;
					case 128:
						DrawThumbnails(&m_ImageList128, &m_strArrayMaster);
						break;
					default:
						m_iTextureWidth = 128;
						DrawThumbnails(&m_ImageList128, &m_strArrayMaster);
						break;
					}
				}	//	if (m_htCurrentGroup == m_htMaster)...
				else	
				{
					if (iDifference > 4000)
					{
						m_iSizingStart = iCurrentTime;
						DrawThumbnails(&m_ImageListGroup, &m_strArrayCurGroup);
					}
				}	//	else...
			}	//	//	if (GetCapture() == pGreatGrandParent)
			else
			{	
				m_iSizingStart = iCurrentTime;

				if (m_htCurrentGroup == m_htMaster)
				{
					switch (m_iTextureWidth)
					{
					case 32:
						DrawThumbnails(&m_ImageList32, &m_strArrayMaster);
						break;
					case 64:
						DrawThumbnails(&m_ImageList64, &m_strArrayMaster);
						break;
					case 128:
						DrawThumbnails(&m_ImageList128, &m_strArrayMaster);
						break;
					default:
						m_iTextureWidth = 128;
						DrawThumbnails(&m_ImageList128, &m_strArrayMaster);
						break;
					}
				}	//	if (m_htCurrentGroup == m_htMaster)...
				else
				{
					DrawThumbnails(&m_ImageListGroup, &m_strArrayCurGroup);
				}
			}	//	if (GetCapture() == pGreatGrandParent)...
		}	//	if (pParent)...
	}//	if (m_treeCtrlGroups.m_hWnd)...
}


void CTexturesDlg::OnDestroy()
{
	CDialog::OnDestroy();

	// TODO: Add your message handler code here

	if (m_pthread)
	{
		m_pthread->Shutdown();
	}
	
	SaveTexturesGroupsAll();
}


void CTexturesDlg::OnOK()
{
	// TODO: Add your specialized code here and/or call the base class
	return;
	CDialog::OnOK();
}


void CTexturesDlg::OnCancel()
{
	m_ListCtrlTextures.OnCancel();

	return;

	CDialog::OnCancel();
}


void CTexturesDlg::OnBnClickedButtonTexRefresh()
{
	if (m_hWnd)
	{
		CJweApp				*pApp = NULL;
		pApp = (CJweApp*)AfxGetApp();

		if (!m_bWorkerThreadActive)
		{
			if (IsWindowVisible())
			{
				SetSelectedGroup(m_htMaster);
				((CButton*)GetDlgItem(IDC_RADIO_TEX_32))->SetCheck(0);
				((CButton*)GetDlgItem(IDC_RADIO_TEX_64))->SetCheck(0);
				((CButton*)GetDlgItem(IDC_RADIO_TEX_128))->SetCheck(1);
				OnBnClickedRadioTex128();

				if (pApp)
				{	
					if (pApp->RefreshMaterialList())
					StartMasterWorkerThread();
				}
			}	//	if (IsWindowVisible())...
		}	//	if (!m_bWorkerThreadActive)...
		else
		{
			int iResult = AfxMessageBox("App is busy preparing texture thumbnails.\n\nPlease wait for this operation to complete anad try again.", MB_OK | MB_ICONINFORMATION);
			
			if (iResult == IDOK)
			{
				BOOL		bResult = 	pApp->PostThreadMessage(IDM_PROGRESS_CONTROL_DESTROY, NULL, NULL);
			}
		}
	}	//	if (m_hWnd)...
}


void	CTexturesDlg::SetSelectedGroup(HTREEITEM htGroup)
{
	if (htGroup != m_htCurrentGroup)
	{	
		m_htCurrentGroup = htGroup;

		m_strArrayCurGroup.RemoveAll();

		if (htGroup != m_treeCtrlGroups.GetRootItem())
		{
			if (m_treeCtrlGroups.ItemHasChildren(htGroup))
			{
				int i = 0;
				HTREEITEM	htItem = NULL;
				htItem = m_treeCtrlGroups.GetNextItem(htGroup, TVGN_CHILD);
				while (htItem)
				{
					m_strArrayCurGroup.InsertAt(i,(m_treeCtrlGroups.GetItemText(htItem)));
					htItem = m_treeCtrlGroups.GetNextSiblingItem(htItem);
					i++;
				}
			}

			if (m_htCurrentGroup != m_htMaster)
			{
				StartGroupWorkerThread();
			}
			else
			{
				switch (m_iTextureWidth)
				{
				case 32:
					DrawThumbnails(&m_ImageList32, &m_strArrayMaster);
					break;
				case 64:
					DrawThumbnails(&m_ImageList64, &m_strArrayMaster);
					break;
				case 128:
					DrawThumbnails(&m_ImageList128, &m_strArrayMaster);
					break;
				default:
					m_iTextureWidth = 128;
					DrawThumbnails(&m_ImageList128, &m_strArrayMaster);
					break;
				}
			}
		}
	}
}


bool	CTexturesDlg::SetSelectedTexture(CString strTexture, CWnd *pOrigin)
{
	if (!m_bWorkerThreadActive)
	{
		m_strSelectedTexture = strTexture;

		MaterialList_Struct	*pMaterialList = NULL;
		Material_Struct		*pSelectedMaterial = NULL;
		Material_Struct		*pMaterial = NULL;
		MaterialIterator	MI ;
		CJweApp				*pApp = NULL;

		pApp = (CJweApp*)AfxGetApp();

		if (pApp)
		{
			CJweDoc*	pDoc = NULL;
			pDoc = ((CMainFrame*)AfxGetMainWnd())->GetCurrentDocument() ;

			if (pDoc)
			{
				//	get the texture materialas list from the jet app
				pMaterialList = pApp->GetMaterialList();

				if (pMaterialList)
				{
					// KROUER: make it compatible with VC6
#if _MFC_VER < 0x0700
					pMaterial = MaterialList_SearchByName( pMaterialList, &MI, m_strSelectedTexture.GetBuffer(0));
#else
					pMaterial = MaterialList_SearchByName( pMaterialList, &MI, m_strSelectedTexture.GetBuffer());
#endif

					if (pMaterial)
					{

						MaterialList_SetCurMaterial(pMaterialList, pMaterial);
						if( pDoc != NULL ) 
						{
							pDoc->ApplyMaterial( ) ;
						}
					}	//	if (pMaterial)...
				}	//	if (m_pMaterialList)...
			}	//	if (pDoc)...
		}	//	if (pApp)...


		if (!m_bBeingSelected)
		{
			if (m_ListCtrlTextures.m_hWnd != pOrigin->m_hWnd)
			{
				//	prevent figure8 loop of CHANGE messages between tree and lst
				m_bBeingSelected = true;	
				m_ListCtrlTextures.SetSelectedTexture(strTexture);
				m_bBeingSelected = false;
				return true;
			}
			if (m_treeCtrlGroups.m_hWnd != pOrigin->m_hWnd)
			{
				m_bBeingSelected = true;
				m_treeCtrlGroups.SetSelectedTexture(strTexture);
				m_bBeingSelected = false;
				return true;
			}
		}	//	if (!m_bBeingSelected)...
		return true;
	}	//	if (!m_bWorkerThreadActive)...
	return false;
}


CString	CTexturesDlg::GetSelectedTexture()
{
	return m_strSelectedTexture;
}

int		CTexturesDlg::GetTextureDisplayWidth()
{
	return m_iTextureWidth;
}


void	CTexturesDlg::SetTextureDisplayWidth(int iWidth)
{
	m_iTextureWidth = iWidth;
}


HTREEITEM	CTexturesDlg::GetMasterTreeItem()
{
	return m_htMaster;
}


HTREEITEM	CTexturesDlg::GetCurGroupTreeItem()
{
	return m_htCurrentGroup;
}


void	CTexturesDlg::CollapseGroupsTree()
{
	HTREEITEM	htRoot = NULL;
	HTREEITEM	htItem = NULL;
	htRoot = m_treeCtrlGroups.GetRootItem();

	if (htRoot)
	{
		htItem = m_treeCtrlGroups.GetNextItem(htRoot, TVGN_CHILD);
		while (htItem)
		{
            m_treeCtrlGroups.Expand(htItem, TVE_COLLAPSE);
			htItem = m_treeCtrlGroups.GetNextSiblingItem(htItem);
		}
	}
}


void	CTexturesDlg::ExpandGroupTree()
{
	m_treeCtrlGroups.Expand(m_treeCtrlGroups.GetRootItem(), TVE_EXPAND);
}


void	CTexturesDlg::SetWorkerThreadFlag(bool bActivity)
{
	m_bWorkerThreadActive = bActivity;
}


void	CTexturesDlg::StartMasterWorkerThread()
{
	if (!m_bWorkerThreadActive)
	{
		m_bWorkerThreadActive = true;
	 	m_pthread = CMyThread::BeginThread(ComputeMasterWithThread, this);
	}
}


UINT CTexturesDlg::ComputeMasterWithThread(LPVOID me)
{
    CTexturesDlg * pSelf = (CTexturesDlg *)me;

    if (pSelf->InitMasterImageLists())
    {
        pSelf->SetSelectedGroup(pSelf->GetMasterTreeItem());
        ((CButton*)pSelf->GetDlgItem(IDC_RADIO_TEX_32))->SetCheck(0);
        ((CButton*)pSelf->GetDlgItem(IDC_RADIO_TEX_64))->SetCheck(0);
        ((CButton*)pSelf->GetDlgItem(IDC_RADIO_TEX_128))->SetCheck(1);

        pSelf->OnBnClickedRadioTex128();
        pSelf->m_pthread->Shutdown();
        pSelf->m_pthread->Delete();
        pSelf->m_pthread = NULL;
        pSelf->SetWorkerThreadFlag(false);
    }
    return 1;
}


void	CTexturesDlg::StartGroupWorkerThread()
{
	if (!m_bWorkerThreadActive)
	{
		m_bWorkerThreadActive = true;
		m_pthread = CMyThread::BeginThread(CoumputeGroupWithThread, this);
	}
}


UINT CTexturesDlg::CoumputeGroupWithThread(LPVOID me)
{
    CTexturesDlg * pSelf = (CTexturesDlg *)me;

    if (pSelf->InitGroupImageList())
    {
        pSelf->DrawGroupThumbnailsFromWorkerThread();

        pSelf->m_pthread->Shutdown();
        pSelf->m_pthread->Delete();
        pSelf->m_pthread = NULL;
        pSelf->SetWorkerThreadFlag(false);
    }
	return 1;
}


bool	CTexturesDlg::GetWorkerThreadActivity()
{
	return m_bWorkerThreadActive;
}



///////////////////////////////////////////////////////////////////////////////////////////////////////////

/**********************************************************************
*                        CMyThread::CMyThread
* Inputs:
*        AFX_THREADPROC proc: Function to be called
*        LPVOID p: Parameter passed to proc
***********************************************************************/
CMyThread::CMyThread(AFX_THREADPROC proc, LPVOID p ) : CWinThread(proc, p)
{
    m_bAutoDelete = FALSE;
    ShutDownEvent = ::CreateEvent(NULL,   // security
        TRUE,   // manual-reset
        FALSE,  // not signaled
        NULL);  // anonymous

    PauseEvent = ::CreateEvent(NULL,      // security
        TRUE,      // manual-reset
        TRUE,      // signaled
        NULL);     // anonymouse
}

/**********************************************************************
*                         CMyThread::~CMyThread
**********************************************************************/
CMyThread::~CMyThread( )
{
    ::CloseHandle(ShutDownEvent);
    ::CloseHandle(PauseEvent);
}

/*********************************************************************
*                        CMyThread::BeginThread
* Result: CMyThread *
*        Newly-created CMyThread object
*********************************************************************/
CMyThread * CMyThread::BeginThread(AFX_THREADPROC proc, LPVOID p)
{
    CMyThread * thread = new CMyThread(proc, p);
    if(!thread->CreateThread( ))
    { /* failed */
        delete thread;
        return NULL;
    } /* failed */

    return thread;
}

/*********************************************************************
*                         CMyThread::Wait
* Result: DWORD
*       WAIT_OBJECT_0 if shutting down
*       WAIT_OBJECT_0+1 if not paused
* Notes:
*       The shutdown *must* be the 0th element, since the normal
*       return from an unpaused event will be the lowest value OTHER
*       than the shutdown index
*********************************************************************/
DWORD CMyThread::Wait( )
{
    HANDLE objects[2];
    objects[0] = ShutDownEvent;
    objects[1] = PauseEvent;
    DWORD result = ::WaitForMultipleObjects(2, objects, FALSE, INFINITE);
    switch(result)
    { /* result */
    case WAIT_TIMEOUT:
        return Timeout;
    case WAIT_OBJECT_0:
        return shutdown;
    case WAIT_OBJECT_0 + 1:
        return Running;
    default:
        ASSERT(FALSE); // unknown error
        return Error;
    } /* result */
}

/********************************************************************
*                        CMyThread::Shutdown
* Effect:
*        Sets the shutdown event, then waits for the thread to shut
*        down
********************************************************************/
void CMyThread::Shutdown( )
{
    SetEvent(ShutDownEvent);
    ::WaitForSingleObject(m_hThread, INFINITE);
}



	
