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


/* execute the script, pass owner (object from which executed) */
 void	eosfunction::exec(ptr<eosobject>	owner)
{
	last_owner = owner;

	// prepare exection
	prepare_exec();
	// save lines;
	int icount = (int)instr.size();
	// loop through instructions and execute
	while(curr_lines < icount)
	{
		instr[curr_lines]->exec(this);		
		++curr_lines;
	}
	// reset execution
	reset_exec();	
};

/* prepare execution, called in exec (restores stacks, etc.) */
 void	eosfunction::prepare_exec()
{
	// arguments konvertieren
	reset_argenv();
	reset_argidx();
	recursive_lines.push(curr_lines);
	curr_lines = 0;

	// copy arguments from the stack to the locals
	// which represent the arguments
	int cnt=0;
	while(!exe->argstack.empty())
	{
		if(cnt<(int)params.size())
		{
			params[cnt]->set(0, exe->argstack.top());
			exe->argstack.pop();
			cnt++;
		} else
			break;
	}

	if(cnt<(int)params.size())
		for(int i=cnt; i<(int)params.size(); i++)
			params[i]->set(0,0);
}

/* reset execution, called in exec (restores env, etc.) */
 void	eosfunction::reset_exec()
{
	reset_argenv();
	reset_argidx();
	//reset_args();
	curr_lines = recursive_lines.top();
	recursive_lines.pop();
}

/* reset enviroment */
 void	eosfunction::reset_argenv()
{
	exe->argrec = exe->argenv;
	exe->argenv = last_owner;
}

/* reset indices */
 void	eosfunction::reset_argidx()
{
	exe->reset_index();
}

/* reset arguments */
 void	eosfunction::reset_args()
{
	while(!exe->argstack.empty())
		exe->argstack.pop();
}

/* locals are handles like globals of eosobject... refer to eosobject.h */
 void	eosfunction::copy_local(string local, ptr<eosvariable> o)
{
	map<string, ptr<eosvariable> >::iterator i = this->locals.find(local);

	if(i == this->locals.end())
	{
		this->locals[local] = new eosvariable(o);
	} else
	(*i).second->copy(o);
}

/* locals are handles like globals of eosobject... refer to eosobject.h */
 ptr<eosvariable>	eosfunction::get_local(string local)
{
	map<string, ptr<eosvariable> >::iterator i = this->locals.find(local);

	if(i == this->locals.end())
	{
		this->locals[local] = new eosvariable(0);
		return this->locals[local];
	} else
	return (*i).second;
}

/* locals are handles like globals of eosobject... refer to eosobject.h */
 void	eosfunction::set_local(string local, int idx, int value)
{
	map<string, ptr<eosvariable> >::iterator i = this->locals.find(local);

	if(i == this->locals.end())
	{
		this->locals[local] = new eosvariable(value);
	} else
	(*i).second->set(idx,value);
}

/* locals are handles like globals of eosobject... refer to eosobject.h */
 void	eosfunction::set_local(string local, int idx, float value)
{
	map<string, ptr<eosvariable> >::iterator i = this->locals.find(local);

	if(i == this->locals.end())
	{
		this->locals[local] = new eosvariable(value);
	} else
	(*i).second->set(idx,value);
}

/* locals are handles like globals of eosobject... refer to eosobject.h */
 void	eosfunction::set_local(string local, int idx, string value)
{
	map<string, ptr<eosvariable> >::iterator i = this->locals.find(local);

	if(i == this->locals.end())
	{
		this->locals[local] = new eosvariable(value);
	} else
	(*i).second->set(idx,value);
}

/* locals are handles like globals of eosobject... refer to eosobject.h */
 void	eosfunction::set_local(string local, int idx, ptr<eosfunction> value)
{
	map<string, ptr<eosvariable> >::iterator i = this->locals.find(local);

	if(i == this->locals.end())
	{
		this->locals[local] = new eosvariable((void*)value);
	} else
	(*i).second->seto(idx,(void*)value);
}

/* locals are handles like globals of eosobject... refer to eosobject.h */
 void	eosfunction::set_local(string local, int idx, ptr<eosobject> value)
{
	map<string, ptr<eosvariable> >::iterator i = this->locals.find(local);

	if(i == this->locals.end())
	{
		this->locals[local] = new eosvariable((void*)value);
	} else
	(*i).second->seto(idx,(void*)value);
}

/* locals are handles like globals of eosobject... refer to eosobject.h */
 int	eosfunction::get_locali(string local, int idx)
{
	map<string, ptr<eosvariable> >::iterator i = this->locals.find(local);

	if(i == this->locals.end())
	{
		this->locals[local] = new eosvariable(0);
		return this->locals[local]->geti(idx);
	} else
	return (*i).second->geti(idx);
}

/* locals are handles like globals of eosobject... refer to eosobject.h */
 float	eosfunction::get_locald(string local, int idx)
{
	map<string, ptr<eosvariable> >::iterator i = this->locals.find(local);

	if(i == this->locals.end())
	{
		this->locals[local] = new eosvariable(0);
		return this->locals[local]->getd(idx);
	} else
	return (*i).second->getd(idx);
}

/* locals are handles like globals of eosobject... refer to eosobject.h */
 string	eosfunction::get_locals(string local, int idx)
{
	map<string, ptr<eosvariable> >::iterator i = this->locals.find(local);

	if(i == this->locals.end())
	{
		this->locals[local] = new eosvariable(0);
		return this->locals[local]->gets(idx);
	} else
	return (*i).second->gets(idx);
}

/* locals are handles like globals of eosobject... refer to eosobject.h */
 ptr<eosfunction>	eosfunction::get_localf(string local, int idx)
{
	map<string, ptr<eosvariable> >::iterator i = this->locals.find(local);

	if(i == this->locals.end())
	{
		this->locals[local] = new eosvariable(0);
		return (eosfunction*)this->locals[local]->geto(idx);
	} else
	return (eosfunction*)(*i).second->geto(idx);
}

/* locals are handles like globals of eosobject... refer to eosobject.h */
 ptr<eosobject>	eosfunction::get_localo(string local, int idx)
{
	map<string, ptr<eosvariable> >::iterator i = this->locals.find(local);

	if(i == this->locals.end())
	{
		this->locals[local] = new eosvariable(0);
		return (eosobject*)this->locals[local]->geto(idx);
	} else
	return (eosobject*)(*i).second->geto(idx);
}

/* locals are handles like globals of eosobject... refer to eosobject.h */
 void	eosfunction::mix(ptr<eosfuncbase> f)
{
	map<string, ptr<eosvariable> >::iterator i = f->locals.begin();
	for(;i!=f->locals.end();i++)
		this->locals[(*i).first] = (*i).second;

	for(int j=0; j<(int)f->instr.size(); j++)
		this->instr.push_back( f->instr[j] );
}

/* print bytecode to log */
 void	eosfunction::output_data()
{
	log->add("");
  log->add("%s::output_data()", name.c_str());

	map<string, ptr<eosvariable> >::iterator i = locals.begin();
	for(;i!=locals.end();i++)
	{
		for(int j=0; j<(*i).second->count(); j++)
			log->add("local %s[%d] = %s", (*i).first.c_str(), j, (*i).second->gets(j).c_str());
	}

  if(instr.size())
	{
		for(int j=0; j<(int)instr.size(); j++)
			log->add("%03d.) %s", j, instr[j]->name.c_str());
	} else
		log->add("-> empty");
}


eosexec::eosexec(ptr<eosobject> ge)
{	temp_idx	= new eosvariable(0);		
	argidx.push(temp_idx);
	for(int i=0; i<=30; i++)
		temp.push_back(new eosvariable(0));
	temppos=0;
	argrec=0;
	genvi=ge;
}

// reset index stack
void	eosexec::reset_index()
{
	while(!argidx.empty()) argidx.pop();
	argidx.push(temp_idx);
}

/* push value to the stack. there are some premade eosvariable which are used for better
   performance (no alloc in execution) */
void	eosexec::push(int i)
{ temp[temppos]->set(0,i); argstack.push(temp[temppos]);  temppos++; if(temppos>30) temppos=0; }
void	eosexec::push(float f)
{ temp[temppos]->set(0,f); argstack.push(temp[temppos]);  temppos++; if(temppos>30) temppos=0; }
void	eosexec::push(string s)
{ temp[temppos]->set(0,string(s)); argstack.push(temp[temppos]);  temppos++; if(temppos>30) temppos=0; }
void	eosexec::push(ptr<eosfuncbase> o)
{ temp[temppos]->seto(0,o); argstack.push(temp[temppos]);  temppos++; if(temppos>30) temppos=0; }
void	eosexec::push(ptr<eosobject> o)
{ temp[temppos]->seto(0,o); argstack.push(temp[temppos]);  temppos++; if(temppos>30) temppos=0; }

/* pop value from stack, returns variable */
ptr<eosvariable> eosexec::pop()
{	ptr<eosvariable> var = argstack.top(); argstack.pop(); return var; }

/* returns stack size */
int	eosexec::stack_size()
{ return (int)argstack.size();	}

ptr<eosvariable> eosexec::req_temp()
{
	if(temppos>=30) temppos=0;
	return temp[temppos++]; 
}

