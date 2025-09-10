/****************************************************************************************/
/*  CTextToolBar.CPP                                                                    */
/*                                                                                      */
/*  Author:			Joachim Hellmann                                                    */
/*  Description:    Helper Class for  Toolbars with 'Subtitles'                         */
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
/****************************************************************************************/



class CTextToolBar : public CToolBar
{
private:
	int	iElementXSize;
	int	iElementYSize;
public:
	BOOL	LoadToolBar(LPCTSTR lpszResourceName, int iXSize =16, int iYSize=15);
	int     ChangeToolBar (CWnd *pObjectWnd,UINT sourceId,UINT destId, UINT Style, CFont *ObjectFont);
};
