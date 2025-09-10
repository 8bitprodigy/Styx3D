/****************************************************************************************/
/*  MYSTATIC.H                                                                          */
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
#if !defined(AFX_MYSTATIC_H__97353E01_ECCF_11D2_8B42_00104B70D76D__INCLUDED_)
#define AFX_MYSTATIC_H__97353E01_ECCF_11D2_8B42_00104B70D76D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CMyStatic window


class CMyStatic : public CStatic
{
// Construction
public:
	CMyStatic();

// Attributes
public:
	typedef enum { THUMBNAIL_32=32, THUMBNAIL_64=64, THUMBNAIL_128=128 } eTHUMBSIZE ;

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMyStatic)
	//}}AFX_VIRTUAL

// Implementation
public:
	int ScrollMaterialInView( const Material_Struct * pMaterial );
	Material_Struct * GetMaterialAtPoint( const CPoint & rPoint );
	int GetTileHeight( void );
	int GetVirtualHeight( void );
	void SetTile( const eTHUMBSIZE eTile );
	void SetScrollTop( int nTopPixel );
	void SetBitmap( CBitmap * pBitmap );
	virtual ~CMyStatic();

	// Generated message map functions
protected:
	//{{AFX_MSG(CMyStatic)
	afx_msg void OnPaint();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
private:
	int m_nTile;
	int m_nTop;
	CBitmap * m_pBitmap;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MYSTATIC_H__97353E01_ECCF_11D2_8B42_00104B70D76D__INCLUDED_)
