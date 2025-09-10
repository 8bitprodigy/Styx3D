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

#ifndef EOSVAR_H
#define EOSVAR_H

/* this is the main variable structure which represents a global or
	 local in memory. a variable stores a vector of other variables, where
	 the first element (0) is a pointer to the variable itself (arrays). 
	 each eosvariable stores one eosvalue class, which represents its value
	 (thatswhy i got a vector of variables, each got one value). */
class eosvariable : public eosbase
{
public:
	/* differect ctors for each type */
	eosvariable(ptr<eosvariable> o) 
	{ init(); copy(o); };
	eosvariable()
	{ init(); set(0,0); };
	eosvariable(int i)
	{ init(); set(0,i); };
	eosvariable(float f)
	{ init(); set(0,f); };
	eosvariable(string s)
	{ init(); set(0,s); };
	eosvariable(void* o)
	{ init(); seto(0,o); };

	/* called from each contructor - creates the eosvalue class */
	void	init();
	/* copy all values of another variable (this means: duplicate) */
	void	copy(ptr<eosvariable> o);

	/* set the value of a specific variable in the array */
	void	set(int idx, ptr<eosvariable> o);
	void	set(int idx, int value);
	void	set(int idx, float value);
	void	set(int idx, string value);
	void	seto(int idx, void* value);

	/* get the value of a specific variable in the array */
	ptr<eosvariable>	get(int idx);
	int			geti(int idx);
	float		getd(int idx);
	string	gets(int idx);
	void*		geto(int idx);

	/* clear all variables from array list */
	void clear();

	/* bind a c-variable to specific array variable */
	void	bind(int idx, int& value);
	void	bind(int idx, float& value);
	void	bind(int idx, string& value);
	void	bindo(int idx, void* value);

	/* get the value of this variable */
	ptr<eosvalue>	getv();

	/* return number of variables in array list */
	int	count();
	
	/* delete item of array at idx */
	void	delete_element(int idx);
	/* insert item into array at idx */
	void	insert_element(ptr<eosvariable> o, int idx);

public:
	string	name;	// name of variable, not always used (just for logger)
	
	vector< ptr<eosvariable> >	values;		// array list
	ptr<eosvalue>								thisval;	// the value of this variable
};

#endif