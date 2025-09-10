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

#ifndef EOS_BASE_H
#define EOS_BASE_H

/* this is the base class for all classes used in eosscript. 
	 it allows the child to be used by ptr<> and counts refs for
	 the garbage collector */
class eosbase
{
public:
	friend class eosvm;

	eosbase();
	virtual ~eosbase() = 0;

	/* add reference */
	void	rmv_ref();
	/* remove reference */
	void	add_ref();
	/* check if used by any reference */
	bool	is_used();

	/* enable/disable reference counting */
	static void	ref_count(bool b);
	/* check if reference counting is currently on/off */
	static bool	use_ref_count();

protected:
	// num of references
	int		refcnt;
	// reference count on/off
	static bool	use_refcnt;

	static vector<eosbase*>	obj_list;		// static garbage list
	static vector<eosbase*>	garb_list;	// static list of all objects
};

/* singleton class. childs can only be create once. used for core
	 objects like vm, parser or analyser (or logger) */
template<class T>
class eossingleton
{
   static T* single;	

   public:
   eossingleton()
   { single = (T*)(this); }

   ~eossingleton()
   { single=0; }

   static T* inst()
   { 
	  return single;
	 }

	 // return a created instance
	 static void create()
	 {  new  T(); }
	 // release the singleton
	 static void release()
	 { delete (T*)single; }
};

template <class T> T* eossingleton <T>::single = 0;

#endif