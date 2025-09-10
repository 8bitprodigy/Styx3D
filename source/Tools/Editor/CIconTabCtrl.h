/****************************************************************************************/
/*  CTextToolBar.CPP                                                                    */
/*                                                                                      */
/*  Author:			Joachim Hellmann                                                    */
/*  Description:    Helper Class for  Toolbars with 'Subtitles'                         */
/*                                                                                      */                                                                                     
/****************************************************************************************/
#include "BtnST.h"
#include "Label.h"

#ifndef CICON_TAB_CTRL
#define CICON_TAB_CTRL

#define ICONTABCTRL_MAXGROUPS	20
#define ICONTABCTRL_MAXICONS	20
#define ICONTABCTRL_TITLESIZE	200

class CIconTabCtrl  
{

public:
	CIconTabCtrl(CWnd *pcWnd, int MaxGroups, int MaxIconsPerGroup);
	CIconTabCtrl();
	~CIconTabCtrl();

	int Group_AddObjects( int Group,char *sTitle, int ButtonId, int IconId, int PressIconId, int ID, ... );
	int Group_Display ( int Group);
	int Group_ShowObjects( int Group, int SHObject );

	int SetTitleId (int TitleId);

	int RecalcLayout(int Ref);
	int SetStatic  (int Ref);

private:
	int Group_RecalcLayout(int Group, int Ref);

	int		iActGroup;	// Number of Groups
	int		MaxGroups;
	int		MaxIconsPerGroup;

	int		TitleId;

	int	 **			GroupIcons;
	char **			GroupTitles;
	CButtonST **	m_btnGruppe;
	CWnd *			pcWnd;
	CLabel			m_stRect;


};

#endif
