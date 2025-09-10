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

#ifndef EOSPTR_H
#define EOSPTR_H

/* simple smart pointer class for reference counting */
template <class T>
class ptr
{
public:
	ptr()
	{ 
		pointer = 0 ;
	}
	ptr(T* p)
	{ 
		pointer = 0;
		*this = p; 
	}
	ptr(const ptr<T> &p)
	{
		pointer = 0;
		*this = p;
	}

	~ptr()
	{
		//if(pointer) pointer->rmv_ref();
	}


	T* operator= (T* p)
	{
		if(pointer) pointer->rmv_ref();
		pointer = p;
		if(pointer) pointer->add_ref();

		return this->pointer;
	}
	ptr<T>* operator= (const ptr<T> &p)
	{
		if(pointer) pointer->rmv_ref();
		pointer = p.pointer;
		if(pointer) pointer->add_ref();

		return this;
	}


	T& operator *() const
	{
		return *pointer;
	}
	T* operator ->() const
	{
		return pointer;
	}
	operator T*() const
	{
		return pointer;
	}

	// misc
	bool is_valid() const
	{
		return (pointer!=0);
	}
	bool operator !()
	{
		return !(pointer);
	}
	bool operator ==(const ptr<T> &p) const
	{
		return (pointer==p.pointer);
	}	
	bool operator ==(const T* p) const
	{
		return (pointer==p);
	}

private:
	T*	pointer;
};


//! Base class for method
/*! Used to pass method templates of any type*/
class cmethod : public eosbase
{
public:
	virtual void operator()() {};
};

//! Class can store member functions and simple function
/*! \param T Type of class, which owns the member variable*/
template<class T>
class method : public cmethod
{
protected:
		T*	mobj;	//!< Real object
		void(T::*mmember)();	//!< pointer Member function

public:
	//! Constructor
   method(T *pobj, void(T::*pmember)())
   { 
		 mobj=pobj;
		 mmember=pmember;
	 }

   void operator ()() 
   { (*mobj.*mmember)(); }
};

#endif