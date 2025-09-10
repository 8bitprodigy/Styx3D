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

#ifndef EOSVALUE_H
#define EOSVALUE_H

/* this class represents a single value in memory. a value can be an itneger,
   a float, a string, or an object (a object or a function). all these values
   can be assigned to a value. you can request the value, in any type. if the 
   value stores an integer, you can request a string an receive that integer 
   converted to a string (works in many directions). This way it is realised 
   that variables in the script enviroment got no value types. this class is
   stored in eosvariable, which manages variables as arrays.						 */   
class eosvalue : public eosbase
{
public:
	/* constructor for each type */
	eosvalue() : iundef(0), dundef(0.0f), sundef(""),
							 ival(&iundef), dval(&dundef), sval(&sundef), oval(&oundef)
	{valtype=0;};

	eosvalue(int i) : iundef(0), dundef(0.0f), sundef(""),
							 ival(&iundef), dval(&dundef), sval(&sundef), oval(&oundef)
	{ seti(i); };

	eosvalue(float f) : iundef(0), dundef(0.0f), sundef(""),
							 ival(&iundef), dval(&dundef), sval(&sundef), oval(&oundef)
	{ setd(f); };

	eosvalue(string s) : iundef(0), dundef(0.0f), sundef(""),
							 ival(&iundef), dval(&dundef), sval(&sundef), oval(&oundef)
	{ sets(s); };


	eosvalue(void* o) : iundef(0), dundef(0.0f), sundef(""),
							 ival(&iundef), dval(&dundef), sval(&sundef), oval(&oundef)
	{ seto(o); };

	eosvalue(ptr<eosvalue> o) : iundef(0), dundef(0.0f), sundef(""),
							 ival(&iundef), dval(&dundef), sval(&sundef), oval(&oundef)
	{ copyv(o); };

	/* copy value from another eosvalue class */
	void	copyv(ptr<eosvalue> val);

	/* set value of this class for each type */
	void	seti(int i);	
	void	setd(float f);	
	void	sets(string s);	
	void	seto(void* o);		
	/* bind c-variable to this value for each type */
	void	bindi(int* i)	;	
	void	bindd(float* f);	
	void	binds(string* s);
	void	bindo(void* o);	
	/* get value of this class for each type */
	int			geti(); 
	float		getd();
	string	gets();
	void*		geto();
	
public:
	/* for each value type, one pointer. we use pointer here so it is possible
		 to link c-variables to a value so they share their value. if no c-variable
		 is linked, we use the values defined below.. */
	int			valtype;
	//nil					// 0
  int*		ival;	// 1
	float*	dval;	// 2
	string*	sval;	// 3
	void*		oval;	// 4

	/* these values are linked to the pointers above if no linked c-variable is given */
	int				iundef;
	float			dundef;
	string		sundef;
	void*			oundef;
};

#endif
