/*
   eos.script library
   Copyright (C) 2004 Fabian Dannenberger

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA



   The original version is located at:
   http://www.eosscript.org

   Fabian Dannenberger
   info@eosscript.org
*/

#include "../lib/eosscript.h"

// static..
vector<eosbase*> eosbase::obj_list;
vector<eosbase*> eosbase::garb_list;

// by default, we count references
bool eosbase::use_refcnt = true;

// ctor.. pushes class back to list
eosbase::eosbase()
{
	// no refs yet
	refcnt = 0;
	// push back to list of all classes
	eosbase::obj_list.push_back(this);
}

eosbase::~eosbase()
{
	// when destroyed, delete this class from the static lists
	if (this->is_used())
	{
		// if still used, delete from default list
		for(int i=0; i<(int)obj_list.size();i++)
			if( obj_list[i] == this)	obj_list.erase(obj_list.begin()+i);
	}
	else
	{
		// if not used, remove from garbage list
		for(int i=0; i<(int)garb_list.size();i++)
			if( garb_list[i] == this)	garb_list.erase(garb_list.begin()+i);
	}
}


/* remove reference */
void	eosbase::rmv_ref()
{
	if(use_refcnt)
	{
		--refcnt;
	
		// if not used, remove it from default list and add to garbage
		if (!this->is_used())
		{
			for(int i=0; i<(int)obj_list.size();i++)
				if( obj_list[i] == this)
				{
					obj_list.erase(obj_list.begin()+i);;
					garb_list.push_back(this);
				}
		}
	}
}

/* add reference */
void	eosbase::add_ref()
{ if(use_refcnt) ++refcnt; }

/* check if used by any reference */
bool	eosbase::is_used()
{ return (refcnt > 0 ? true : false); }

/* enable/disable reference counting */
void	eosbase::ref_count(bool b)
{ use_refcnt = b; }

/* check if reference counting is currently on/off */
bool	eosbase::use_ref_count()
{ return use_refcnt; }