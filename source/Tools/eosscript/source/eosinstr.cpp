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

void	cmp_vals(ptr<eosvalue> one, ptr<eosvalue> two, ptr<eosfunction> owner)
{
	if(one->gets() == two->gets())
	{
		
		owner->equal_flag = 1;
		owner->size_flag = -1;
	}
  else if(one->getd() > two->getd())
	{
		owner->size_flag	= 1;
		owner->equal_flag	= 0;
	}
  else if(one->getd() < two->getd())
	{
		owner->size_flag	= 0;
		owner->equal_flag	= 0;
	}
	else
	{
		owner->size_flag		= -1;
		owner->equal_flag		= -1;
	}
}

void eosinstr_cobj::exec(ptr<eosfunction> owner)
{
	ptr<eosvariable>	arg1	= exe->argstack.top(); exe->argstack.pop();	
	string name = arg1->gets(0);
	if( vm->objects.find(name) !=  vm->objects.end() )
	{
		ptr<eosobject> o = vm->objects[name]->create();
		

		if(o->self)	o->self->exec(o);
		exe->push(o);
	} else
		log->add_error("file:%s line:%d object type '%s' not found!", this->f.c_str(), l,name.c_str());

		owner->reset_argidx();
		owner->reset_argenv();

}

void eosinstr_cfunc::exec(ptr<eosfunction> owner)
{
	ptr<eosvariable>	arg1	= exe->argstack.top(); exe->argstack.pop();	

	if( exe->argenv->methods.find(arg1->gets(0)) !=  exe->argenv->methods.end() )
		exe->push((ptr<eosfuncbase>)exe->argenv->methods[arg1->gets(0)]);
	else
		log->add_error("file:%s line:%d function '%s' not found!", this->f.c_str(), l,arg1->gets(0).c_str());

	owner->reset_argidx();
	owner->reset_argenv();
}

void eosinstr_cmeth::exec(ptr<eosfunction> owner)
{
	ptr<eosvariable>	arg1	= exe->argstack.top(); exe->argstack.pop();	

	if( exe->argrec->methods.find(arg1->gets(0)) !=  exe->argrec->methods.end() )
		exe->push((ptr<eosfuncbase>)exe->argrec->methods[arg1->gets(0)]);
	else
		log->add_error("file:%s line:%d function '%s' not found!", this->f.c_str(), l,arg1->gets(0).c_str());

	owner->reset_argidx();
	owner->reset_argenv();
}