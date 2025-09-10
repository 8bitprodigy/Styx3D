/****************************************************************************************/
/*  CIconTabCtrl.CPP                                                                    */
/*                                                                                      */
/*  Author:			Joachim Hellmann                                                    */
/*  Description:    Helper Class for  Special Type TabControls							*/
/*                                                                                      */
/*                                                                                      */
/****************************************************************************************/
#include "stdafx.h"
#include "CIconTabCtrl.h"


CIconTabCtrl::CIconTabCtrl()
	{
		//CIconTabCtrl(ICONTABCTRL_MAXGROUPS,ICONTABCTRL_MAXICONS);
	}


CIconTabCtrl::~CIconTabCtrl()
	{
		if (GroupIcons!=NULL)
			{
			  for (int X=0;X<MaxGroups;X++)
				{
				  if (GroupIcons[X]!=NULL)  delete [] GroupIcons[X];			
				  if (GroupTitles[X]!=NULL) delete [] GroupTitles[X];
				  if (m_btnGruppe[X]!=NULL) delete m_btnGruppe[X];
				}
			}

		if (GroupIcons!=NULL)
			delete []GroupIcons;

		if (GroupTitles!=NULL)
			delete []GroupTitles;
		
		if (m_btnGruppe!=NULL)
			delete [] m_btnGruppe;
	}


CIconTabCtrl::CIconTabCtrl(CWnd *pcWnd, int MaxGroups, int MaxIconsPerGroup)
	{
		RECT ButtonRect;
		ButtonRect.top=10;
		ButtonRect.bottom=200;
		ButtonRect.left=10;
		ButtonRect.right=200;

		GroupIcons  = (int**)		new int*	   [MaxGroups];
		GroupTitles = (char**)		new char*	   [MaxGroups];
		m_btnGruppe = (CButtonST**) new CButtonST* [MaxGroups];
	    
		if ( GroupIcons  == NULL ) return;
	    if ( GroupTitles == NULL ) return;
	    if ( m_btnGruppe == NULL ) return;

		for (int X=0;X<MaxGroups;X++)
			{ GroupIcons  [X] = (int*)  new int  [MaxIconsPerGroup];
			  GroupTitles [X] = (char*) new char [ICONTABCTRL_TITLESIZE];
			  m_btnGruppe [X] =	new CButtonST();
			  
			  // Exception,...
			  if ( GroupIcons  [X] == NULL ) return;
			  if ( GroupTitles [X] == NULL ) return;
			  if ( m_btnGruppe [X] == NULL ) return;

			}
		
		this->pcWnd				= pcWnd;
		this->MaxGroups			= MaxGroups;
		this->MaxIconsPerGroup  = MaxIconsPerGroup;

		TitleId = -1;

		//	by trilobite	Jan. 2011
		//for ( X=0; X<MaxGroups; X++)
		for ( int X=0; X<MaxGroups; X++)
		//
			GroupIcons [X][0] = -1;
	}

int CIconTabCtrl::SetTitleId (int TitleId)
	{
		this->TitleId = TitleId;
		return true;
	}

int CIconTabCtrl::SetStatic (int TitleId)
	{	
	   m_stRect.SubclassDlgItem ( TitleId, pcWnd);
	   m_stRect.SetBkColor(::GetSysColor(COLOR_ACTIVECAPTION));

	   return true;
 	}

int CIconTabCtrl::Group_AddObjects( int Group,char *sTitle, int ButtonId, 
								    int IconId, int PressIconId, int ID, ... )
	{
	   int		i = ID;
	   int		X = 0;
 
	   m_btnGruppe [ Group ]->SubclassDlgItem ( ButtonId, pcWnd);
	   m_btnGruppe [ Group ]->SetIcon  ( IconId);
	   m_btnGruppe [ Group ]->SetAlign ( CButtonST::ST_ALIGN_VERT);  
	   m_btnGruppe [ Group ]->DrawTransparent ( );
	   m_btnGruppe [ Group ]->SetActiveFgColor (::GetSysColor(COLOR_CAPTIONTEXT));
	   m_btnGruppe [ Group ]->SetInactiveFgColor (::GetSysColor(COLOR_CAPTIONTEXT));
	   

	   strncpy ( GroupTitles[Group], sTitle, ICONTABCTRL_TITLESIZE-1 );
	   GroupTitles [ Group ] [ ICONTABCTRL_TITLESIZE-1 ] = '\0';

	   va_list	 marker;
	   va_start( marker, ID );     /* Initialize variable arguments. */

	   while( i != -1 )
		   {
				GroupIcons [ Group ] [ X++ ] = i;
				i = va_arg( marker, int);
				if ( X > MaxIconsPerGroup ) 
					return false;
		   }
	   
	   GroupIcons [ Group ] [ X ] = -1;
	   va_end( marker );              /* Reset variable arguments.      */
	   return true;
	}


int CIconTabCtrl::Group_Display (int Group)
	{
		for (int X=0;X<MaxGroups;X++)
			Group_ShowObjects (X,false);

		Group_ShowObjects (Group,true);
		
		if (TitleId!=-1) 
		{
			if (pcWnd->GetDlgItem(TitleId))
				pcWnd->GetDlgItem(TitleId)->SetWindowText(GroupTitles[Group]);
		}
		return true;
	}

int  CIconTabCtrl::Group_ShowObjects( int Group, int SHObject )
	{
		if (GroupIcons[Group][0]==-1)
			return false;

		for (int X=0;GroupIcons[Group][X]!=-1;X++)	
		{
			if (pcWnd->GetDlgItem( GroupIcons[Group][X] ))
                pcWnd->GetDlgItem( GroupIcons[Group][X] )->ShowWindow( SHObject ) ;
		}
		return true;
	}

int CIconTabCtrl::RecalcLayout(int Ref)
	{	for (int X=0;X<MaxGroups;X++)
			if (!Group_RecalcLayout (X,Ref)) return false;
		return true;
	}

	int CIconTabCtrl::Group_RecalcLayout( int Group, int Ref)
	{
		RECT	ClientRect{};
		RECT	WindowRect{};
		RECT	DestClientRect{};
		int		YSub{};
		int		XSub{};

		if (GroupIcons[Group][0]==-1)
			return false;

		if (pcWnd->GetDlgItem( Ref ))
		{
			pcWnd->GetDlgItem( Ref )->GetWindowRect( &DestClientRect );
			pcWnd->GetWindowRect( &WindowRect );


			if (pcWnd->GetDlgItem( GroupIcons[Group][0] ))
			{
				pcWnd->GetDlgItem( GroupIcons[Group][0] )->GetWindowRect( &ClientRect );

				YSub = ClientRect.top-DestClientRect.top+WindowRect.top-DestClientRect.top+DestClientRect.bottom; 
				XSub = ClientRect.left-DestClientRect.left+WindowRect.left;

				for (int X=0;GroupIcons[Group][X]!=-1;X++)			
				{

					if (pcWnd->GetDlgItem( GroupIcons[Group][X]))
					{
						pcWnd->GetDlgItem( GroupIcons[Group][X])->GetWindowRect( &ClientRect );

						ClientRect.bottom-=YSub;ClientRect.top-=YSub; ClientRect.left-=XSub;ClientRect.right-=XSub;
						pcWnd->GetDlgItem( GroupIcons[Group][X])->MoveWindow( &ClientRect,false );
					}
				}
			}
		}
		return true;
	}

