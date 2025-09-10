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


/* copy global from another global */
 void	eosobject::copy_global(string global, ptr<eosvariable> o)
{
	map<string, ptr<eosvariable> >::iterator i = this->globals.find(global);

	if(i == this->globals.end())
	{
		this->globals[global] = new eosvariable(o);
	} else
	(*i).second->copy(o);
}

/* set global at specific index */
 void	eosobject::set_global(string global, int idx, int value)
{
	map<string, ptr<eosvariable> >::iterator i = this->globals.find(global);

	if(i == this->globals.end())
	{
		this->globals[global] = new eosvariable(value);
	} else
	(*i).second->set(idx,value);
}

/* set global at specific index */
 void	eosobject::set_global(string global, int idx, float value)
{
	map<string, ptr<eosvariable> >::iterator i = this->globals.find(global);

	if(i == this->globals.end())
	{
		this->globals[global] = new eosvariable(value);
	} else
	(*i).second->set(idx,value);
}

/* set global at specific index */
 void	eosobject::set_global(string global, int idx, string value)
{
	map<string, ptr<eosvariable> >::iterator i = this->globals.find(global);

	if(i == this->globals.end())
	{
		this->globals[global] = new eosvariable(value);
	} else
	(*i).second->set(idx,value);
}

/* set global at specific index */
 void	eosobject::set_global(string global, int idx, ptr<eosfuncbase> value)
{
	map<string, ptr<eosvariable> >::iterator i = this->globals.find(global);

	if(i == this->globals.end())
	{
		this->globals[global] = new eosvariable((void*)value);
	} else
	(*i).second->seto(idx,value);
}

/* set global at specific index */
 void	eosobject::set_global(string global, int idx, ptr<eosobject> value)
{
	map<string, ptr<eosvariable> >::iterator i = this->globals.find(global);

	if(i == this->globals.end())
	{
		this->globals[global] = new eosvariable((void*)value);
	} else
	(*i).second->seto(idx,value);
}

/* get global at specific index */
 ptr<eosvariable>	eosobject::get_global(string global)
{
	map<string, ptr<eosvariable> >::iterator i = this->globals.find(global);

	if(i == this->globals.end())
	{
		this->globals[global] = new eosvariable(0);
		return this->globals[global];
	} else
	return (*i).second;
}

/* get global at specific index */
 int	eosobject::get_globali(string global, int idx)
{
	map<string, ptr<eosvariable> >::iterator i = this->globals.find(global);

	if(i == this->globals.end())
	{
		this->globals[global] = new eosvariable(0);
		return this->globals[global]->geti(idx);
	} else
	return (*i).second->geti(idx);
}

/* get global at specific index */
 float	eosobject::get_globald(string global, int idx)
{
	map<string, ptr<eosvariable> >::iterator i = this->globals.find(global);

	if(i == this->globals.end())
	{
		this->globals[global] = new eosvariable(0);
		return this->globals[global]->getd(idx);
	} else
	return (*i).second->getd(idx);
}

/* get global at specific index */
 string	eosobject::get_globals(string global, int idx)
{
	map<string, ptr<eosvariable> >::iterator i = this->globals.find(global);

	if(i == this->globals.end())
	{
		this->globals[global] = new eosvariable(0);
		return this->globals[global]->gets(idx);
	} else
	return (*i).second->gets(idx);
}

/* get global at specific index */
 ptr<eosfuncbase>	eosobject::get_globalf(string global, int idx)
{
	map<string, ptr<eosvariable> >::iterator i = this->globals.find(global);

	if(i == this->globals.end())
	{
		this->globals[global] = new eosvariable(0);
		return (eosfunction*)this->globals[global]->geto(idx);
	} else
	return (eosfunction*)(*i).second->geto(idx);
}

/* get global at specific index */
 ptr<eosobject>	eosobject::get_globalo(string global, int idx)
{
	map<string, ptr<eosvariable> >::iterator i = this->globals.find(global);

	if(i == this->globals.end())
	{
		this->globals[global] = new eosvariable(0);
		return (eosobject*)this->globals[global]->geto(idx);
	} else
	return (eosobject*)(*i).second->geto(idx);
}

/* bind global at specific index */
 void	eosobject::bind_global(string global, int idx, int& value)
{
	map<string, ptr<eosvariable> >::iterator i = this->globals.find(global);

	if(i == this->globals.end())
	{
		this->globals[global] = new eosvariable(global);
		this->globals[global]->bind(idx,value);
	} else
	(*i).second->bind(idx,value);
}

/* bind global at specific index */
 void	eosobject::bind_global(string global, int idx, float& value)
{
	map<string, ptr<eosvariable> >::iterator i = this->globals.find(global);

	if(i == this->globals.end())
	{
		this->globals[global] = new eosvariable(global);
		this->globals[global]->bind(idx,value);
	} else
	(*i).second->bind(idx,value);
}

/* bind global at specific index */
 void	eosobject::bind_global(string global, int idx, string& value)
{
	map<string, ptr<eosvariable> >::iterator i = this->globals.find(global);

	if(i == this->globals.end())
	{
		this->globals[global] = new eosvariable(global);
		this->globals[global]->bind(idx,value);
	} else
	(*i).second->bind(idx,value);
}

/* bind global at specific index */
 void	eosobject::bind_global(string global, int idx, ptr<eosfunction> value)
{
	map<string, ptr<eosvariable> >::iterator i = this->globals.find(global);

	if(i == this->globals.end())
	{
		this->globals[global] = new eosvariable(global);
		this->globals[global]->bindo(idx,value);
	} else
	(*i).second->bindo(idx,value);
}

/* bind global at specific index */
 void	eosobject::bind_global(string global, int idx, ptr<eosobject> value)
{
	map<string, ptr<eosvariable> >::iterator i = this->globals.find(global);

	if(i == this->globals.end())
	{
		this->globals[global] = new eosvariable(global);
		this->globals[global]->bindo(idx,value);
	} else
	(*i).second->bindo(idx,value);
}

/* copy data of another object, no references */
 void	eosobject::inherit(ptr<eosobject> o)
{
	if(o->isclass)
	{
		o->scrobj = this;
		this->parents.push_back(o);
	}

	
	for(int i=0; i<(int)o->cparents.size(); i++)
		this->cparents.push_back( o->cparents[i] );

	// loop through methods
	map<string, ptr<eosfuncbase> >::iterator itr = o->methods.begin();
	for(;itr!=o->methods.end();itr++)
	{
		if( this->methods.find( (*itr).first ) == this->methods.end() )
		{
			//if( (*itr).first != "self" )
			//{
				this->methods[(*itr).first] = (*itr).second;
			//}
		}
		else
			this->methods[(*itr).first]->mix( (*itr).second );
	}

	// loop through all globals and copy
	map<string, ptr<eosvariable> >::iterator it = o->globals.begin();
	for(;it!=o->globals.end();it++)
	{
		if( (*it).first != "$this" )
		{
			if( this->globals.find( (*it).first ) == this->globals.end() )
	      this->globals[(*it).first] = (*it).second;
			else
				this->globals[(*it).first] = (*it).second;
		}
	}

	if( o->self )
	{
		if( !this->self )
			this->self = new eosfunction();
		this->self->mix((eosfuncbase*)o->self);
	} 
}

/* print all last values of globals and bytecode to the logfile */
 void	eosobject::output_data()
{
	log->add("");
  log->add("%s::output_data()", name.c_str());

	map<string, ptr<eosvariable> >::iterator i = globals.begin();
	for(;i!=globals.end();i++)
	{			
		for(int j=0; j<(*i).second->count(); j++)
			log->add("global %s[%d] = %s", (*i).first.c_str(), j, (*i).second->gets(j).c_str());
	}

	
	map<string, ptr<eosfuncbase> >::iterator itr = methods.begin();
	for(;itr!=methods.end();itr++)
		(*itr).second->output_data();
	
	
	log->add("");
}

/* this method returns a clone of this object. when cpp classes are linked
   to the enviroment, they use a special class, eoscobject (not "c"). this
   class overwrites this method and returns a pointer to the new created cpp
   class. create() is used in cobj_instr and when derived */
 ptr<eosobject> eosobject::create()
{
	ptr<eosobject> obj = new eosobject();

	if(cparents.size()>0)
	{
		for(int j=0; j<(int)cparents.size(); j++)
		{
			obj->inherit( cparents[j]->create() );
		}
	}
			
	obj->inherit(this);
	obj->set_global("$this", 0, obj);

	return obj;
}

/* call a function stored in this object */
 void		eosobject::call(string name)
{
	if(this->methods.find(name) == this->methods.end())
		log->add_error("function '%s' not found", name.c_str());
	else
		this->methods[name]->exec(this);
}

/* checks if a specific func has been decl in the script */
bool		eosobject::is_declared(string name)
{
	if(this->methods.find(name) == this->methods.end())
		return false;
	else
		return true;
}

/* register a cpp method to this object - use makro CPP_METHOD */
 void	eosobject::register_func(string name, void(*f)())
{	this->methods[name] = new eoscfunction(name, f); }

/* register a c function to this object */
 void	eosobject::register_func(string name, ptr<cmethod> f)
{ this->methods[name] = new eoscfunction(name, f); }

/* returns a c-class which has been used as parent - also works
   when an object is not derived, but a c-class */
void*	eosobject::get_parent(string s)
{ 
	for(int i=0; i<(int)parents.size(); i++)
	{
		if( parents[i]->name == s )
			return parents[i];
	}

	log->add_error("eosobject::get_parent: %s not found",  s.c_str() );
	return NULL;
}
	
/* returns the instance of an object, which is used to represent
   the script object (when this is just a parent) - use this to
	 make sure you get the "main" object */
ptr<eosobject> eosobject::get_scrobj()
{ return scrobj; }

void	eosobject::dostring(string s)
{
	ptr<eosfunction> f = new eosfunction();

	parser->outline_symbols(s);

	tokens tok = parser->replace_keyw(parser->tokenize(s));

	analyser->preinstr.clear();
	analyser->analyse_statement(tok,0,(int)tok.size());
	analyser->link(f,analyser->preinstr);

  f->exec(this);
}