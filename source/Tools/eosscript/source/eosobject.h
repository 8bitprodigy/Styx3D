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

#ifndef EOSOBJECT_H
#define EOSOBJECT_H

/* very important class. this represents an object in memory. an object is a "class" in
   the script enviroment, and everything in eos.script is an object (the code which is in
   no object is stored in the global enviroment, another instance of this class. it contains
   globals and functions (which contain locals), the constructor code (which is not declared 
   in any function) is stored in another function, called self. */
class eosobject : public eosbase
{
public:
	// ctor - creates this variable
	eosobject(string s) : name(s)
	{ log->add("object:0x%X('%s') created", this, s.c_str()); set_global("$this", 0, this); isclass=false; scrobj=this; };
	eosobject() : name("parent")
	{ log->add("object:0x%X('%s') created", this, name.c_str()); set_global("$this", 0, this);  isclass=false; scrobj=this; };

	~eosobject() 
	{ log->add("object '%s' deleted", name.c_str()); };

	/* copy global from another global */
	void	copy_global(string global, ptr<eosvariable> o);
	/* set global at specific index */
	void	set_global(string global, int idx, int value);
	void	set_global(string global, int idx, float value);
	void	set_global(string global, int idx, string value);
	void	set_global(string global, int idx, ptr<eosfuncbase> value);
	void	set_global(string global, int idx, ptr<eosobject> value);

	/* get global at specific index */
	int								get_globali(string global, int idx);
	float							get_globald(string global, int idx);
	string						get_globals(string global, int idx);
	ptr<eosfuncbase>	get_globalf(string global, int idx);
	ptr<eosobject>		get_globalo(string global, int idx);
	ptr<eosvariable>	get_global(string global);

	/* bind global at specific index */
	void	bind_global(string global, int idx, int& value);
	void	bind_global(string global, int idx, float& value);
	void	bind_global(string global, int idx, string& value);
	void	bind_global(string global, int idx, ptr<eosfunction> value);
	void	bind_global(string global, int idx, ptr<eosobject> value);

	/* copy data of another object, no references */
	void	inherit(ptr<eosobject> o);

	/* this method returns a clone of this object. when cpp classes are linked
	   to the enviroment, they use a special class, eoscobject (not "c"). this
	   class overwrites this method and returns a pointer to the new created cpp
	   class. create() is used in cobj_instr and when derived */
	virtual ptr<eosobject> create();
	
	/* print all last values of globals and bytecode to the logfile */
	void		output_data();

	/* register a cpp method to this object - use makro CPP_METHOD */
	void		register_func(string name, ptr<cmethod> f);
	/* register a c function to this object */
	void		register_func(string name, void(*f)());
	/* call a function stored in this object */
	void		call(string name);
	/* checks if a specific func has been decl in the script */
	bool		is_declared(string name);

	/* returns a c-class which has been used as parent - also works
	   when an object is not derived, but a c-class */
	void*	get_parent(string s);

	/* returns the instance of an object, which is used to represent
	   the script object (when this is just a parent) - use this to
		 make sure you get the "main" object */
	ptr<eosobject> get_scrobj();	

	void	dostring(string s);

public:
	string													name;		// name of this object (not always used)
	ptr<eosfunction>								self;		// pointer to self function for quickly access locals
	map<string, ptr<eosvariable> >	globals;	// map of all globals
	map<string, ptr<eosfuncbase> >	methods;	// map of all functions

	vector< ptr<eosobject> >		cparents;	// this list stores cpp classes which are parents
	vector< ptr<eosobject> >		parents;	// this list stores instances of c parents

	ptr<eosobject>							scrobj; // stores object which represents the entire object

	bool	isclass;		// defines if this class is the child of a eoscobject cls
};


/* this class is used to store the type of a cpp class. it derives
   from eosobject, and overwrites the virtual function create. use
   this class with the makro CPP_CLASS (more user-friendly) */
template <class T>
class eoscobject : public eosobject
{
public:
  // set bool to true to define that the object is a cpp class
	eoscobject() { isclass=true; }

	// return new instance of stored class
	ptr<eosobject> create()
	{ 
		T* n = new T;
		n->isclass=true;
		n->name = this->name;
		n->scrobj = n;
		n->parents.push_back(n);
		return (n);
	}

	T* obj;	// class to store
};


#endif