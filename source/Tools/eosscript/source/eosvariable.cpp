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

/* called from each contructor - creates the eosvalue class */
 void eosvariable::init()
{
	thisval = new eosvalue(0);
	values.push_back(this);
}

/* get the value of this variable */
 ptr<eosvalue>	eosvariable::getv()
{	return thisval; }

/* copy all values of another variable (this means: duplicate) */
 void	eosvariable::copy(ptr<eosvariable> o)
{
	int size = count();

	for(int i=0; i<o->count(); i++)
	{
		if(i<size)
			values[i]->thisval->copyv(o->values[i]->thisval);
		else
			values.push_back(new eosvariable(o->values[i]));
	}


	//values.resize(i);
	/*
	if(i<size) 
	{
		for(int j=i; j<size; j++)
			values.erase( values.begin()+j );
	}
	*/	
}

/* set the value of a specific variable in the array */
 void	eosvariable::set(int idx, ptr<eosvariable> o)
{		
	if( idx >= (int)values.size() )
	{			
		values.push_back(new eosvariable(o));
	}
	else
		values[idx]->copy(o);
}

/* set the value of a specific variable in the array */
 void	eosvariable::set(int idx, int value)
{
	if( idx >= (int)values.size() )
		values.push_back(new eosvariable(value));
	else
		values[idx]->thisval->seti(value);
}

/* set the value of a specific variable in the array */
 void	eosvariable::set(int idx, float value)
{
	if( idx >= (int)values.size() )
		values.push_back(new eosvariable(value));
	else
		values[idx]->thisval->setd(value);
}

/* set the value of a specific variable in the array */
 void	eosvariable::set(int idx, string value)
{
	if( idx >= (int)values.size() )
		values.push_back(new eosvariable(value));
	else
		values[idx]->thisval->sets(value);
}

/* set the value of a specific variable in the array */
 void	eosvariable::seto(int idx, void* value)
{
	if( idx >= (int)values.size() )
		values.push_back(new eosvariable(value));
	else
		values[idx]->thisval->seto(value);
}

/* get the value of a specific variable in the array */
 ptr<eosvariable>	eosvariable::get(int idx)
{
	if( idx >= (int)values.size() )
	{
		values.push_back(new eosvariable(0));
		return values[ values.size()-1 ];
	}
	else
		return values[idx];
}

/* get the value of a specific variable in the array */
 int	eosvariable::geti(int idx)
{
	if( idx >= (int)values.size() )
		return values[ values.size()-1 ]->thisval->geti();
	else
		return values[idx]->thisval->geti();
}

/* get the value of a specific variable in the array */
 float	eosvariable::getd(int idx)
{
	if( idx >= (int)values.size() )
		return values[ values.size()-1 ]->thisval->getd();
	else
		return values[idx]->thisval->getd();
}

/* get the value of a specific variable in the array */
 string	eosvariable::gets(int idx)
{
	if( idx >= (int)values.size() )
		return values[ values.size()-1 ]->thisval->gets();
	else
		return values[idx]->thisval->gets();
}

/* get the value of a specific variable in the array */
 void*	eosvariable::geto(int idx)
{
	if( idx >= (int)values.size() )
		return values[ values.size()-1 ]->thisval->geto();
	else
		return values[idx]->thisval->geto();
}

/* clear all variables from array list */
 void eosvariable::clear()
{ values.clear(); }

/* bind a c-variable to specific array variable */
 void	eosvariable::bind(int idx, int& value)
{
	if( idx >= (int)values.size() )
	{
		values.push_back( new eosvariable(0) );
		values[ values.size()-1 ]->thisval->bindi(&value);
	}
	else
		values[idx]->thisval->bindi(&value);
}

/* bind a c-variable to specific array variable */
 void	eosvariable::bind(int idx, float& value)
{
	if( idx >= (int)values.size() )
	{
		values.push_back( new eosvariable(0) );
		values[ values.size()-1 ]->thisval->bindd(&value);
	}
	else
		values[idx]->thisval->bindd(&value);
}

/* bind a c-variable to specific array variable */
 void	eosvariable::bind(int idx, string& value)
{
	if( idx >= (int)values.size() )
	{
		values.push_back( new eosvariable(0) );
		values[ values.size()-1 ]->thisval->binds(&value);
	}
	else
		values[idx]->thisval->binds(&value);
}

/* bind a c-variable to specific array variable */
 void	eosvariable::bindo(int idx, void* value)
{
	if( idx >= (int)values.size() )
	{
		values.push_back( new eosvariable(0) );
		values[ values.size()-1 ]->thisval->bindo(&value);
	}
	else
		values[idx]->thisval->bindo(value);
}

/* delete item of array at idx */
 void	eosvariable::delete_element(int idx)
{
	if(idx<count())
		values.erase( values.begin()+idx );
}

/* insert item into array at idx */
 void	eosvariable::insert_element(ptr<eosvariable> o, int idx)
{
	if(idx<count())
		values.insert(values.begin()+idx,o);
	else
		values.push_back(o);
}

/* return number of variables in array list */
 int	eosvariable::count()
{
	return (int)values.size();
}
