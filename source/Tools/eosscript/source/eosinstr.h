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

#ifndef EOSINSTR_H
#define EOSINSTR_H

/* this file contains all instructions. instructions are classes which repesents
   bytecode in eosscript. there are a lot of byte codes, which can be created with
	 arguments which are pushed and poped on the stack. most of them are inlined because
	 we need speed when executing these instructions... */

void	cmp_vals(ptr<eosvalue> one, ptr<eosvalue> two, ptr<eosfunction> owner);

//
// index instructions
//
class eosinstr_pushidx : public eosinstr
{
public:
	eosinstr_pushidx(v& arg1,string file,int line)
	{ one=arg1; name=eosformat("pushidx %s", arg1.name.c_str());
		f=file; l=line;
		if(arg1.type==0) 				
		{
			if(is_number(arg1.name))
				temp.value = new eosvariable(float(_str2<float>(arg1.name)));	
			else
				//temp.value = new eosvariable(string(arg1.name));		
				log->add_error("file:%s line:%d no valid index '%s'!", f.c_str(), l,arg1.name.c_str()); 
		}	
		
	}

	void exec(ptr<eosfunction> owner)
	{
		if(one.type == 0)
			exe->argidx.push( temp.value );
		else if(one.type == 1)
			exe->argidx.push( exe->argenv->get_global(one.name)->get( exe->argidx.top()->geti(0) ) );
		else if(one.type == 2)
			exe->argidx.push( owner->get_local(one.name)->get( exe->argidx.top()->geti(0) ) );		
	}
};

class eosinstr_pusharg : public eosinstr
{
public:
	eosinstr_pusharg(v& arg1,string file,int line)
	{ one=arg1; name=eosformat("pusharg %s", arg1.name.c_str());
		if(arg1.type==0) 				
		{
			if(one.name == "global")
				temp.value = new eosvariable(exe->genvi);		
			else if(is_number(arg1.name))
				temp.value = new eosvariable(float(_str2<float>(arg1.name)));		
			else
				temp.value = new eosvariable(string(arg1.name));		
		}
		f=file; l=line;
	}

	void exec(ptr<eosfunction> owner)
	{
		if(one.type == 0)
			exe->argstack.push( temp.value );
		else if(one.type == 1)
			exe->argstack.push( exe->argenv->get_global(one.name)->get( exe->argidx.top()->geti(0) ));
		else if(one.type == 2)
			exe->argstack.push( owner->get_local(one.name)->get( exe->argidx.top()->geti(0) ));

		owner->reset_argidx();
		owner->reset_argenv();
	}
};

class eosinstr_extarg : public eosinstr
{
public:
	eosinstr_extarg(string file,int line)
	{ name=eosformat("extarg"); f=file; l=line;}

	void exec(ptr<eosfunction> owner)
	{
		ptr<eosvariable> buff2 = exe->argstack.top(); exe->argstack.pop();
		ptr<eosvariable> buff = exe->argstack.top(); exe->argstack.pop();

		exe->argstack.push( buff->get( buff2->get( exe->argidx.top()->geti(0))->geti(0) ) );

		owner->reset_argidx();
	}
};

class eosinstr_pushenv : public eosinstr
{
public:
	eosinstr_pushenv(string file,int line)
	{ name=eosformat("pushenv"); f=file; l=line;	}

	void exec(ptr<eosfunction> owner)
	{
		ptr<eosvariable>	arg1	= exe->argstack.top(); exe->argstack.pop();		

		if(arg1->gets(0)=="global")
			exe->argenv = exe->genvi;
		else
		{
			eosvariable* var = arg1->get( exe->argidx.top()->geti(0) );

			if(var->thisval->valtype == 4)
				exe->argenv = (eosobject*)var->geto(0);
			else
				//log->add_error("no valid enviroment in '%s'", var->name.c_str());
				log->add_error("file:%s line:%d no valid enviroment (variable stores no object)!", f.c_str(), l); 
		}

	  owner->reset_argidx();
	}
};

class eosinstr_recenv : public eosinstr
{
public:
	eosinstr_recenv(string file,int line)
	{ name=eosformat("recenv");	f=file; l=line;}

	void exec(ptr<eosfunction> owner)
	{
		if(exe->argrec)
			exe->argenv = exe->argrec;
	}
};

class eosinstr_mov : public eosinstr
{
public:
	eosinstr_mov(string file,int line) { name="mov";f=file; l=line;}
	void exec(ptr<eosfunction> owner)
	{
		ptr<eosvariable>	arg1	= exe->argstack.top(); exe->argstack.pop();		

		int idx=0;
		ptr<eosvariable>	arg2 = 0;
		if(!exe->argstack.empty())
			while( !exe->argstack.empty() )
			{
				arg2	= exe->argstack.top();
				arg1->set(idx++,arg2);
				exe->argstack.pop();			
			}
		else
			log->add_error("file:%s line:%d stack error (= needs 2 items on stack)!", f.c_str(), l); 

		
		//owner->reset_argidx();
		//owner->reset_argenv();
	}
};

class eosinstr_add : public eosinstr
{
public:
	eosinstr_add(string file,int line) { name="add";f=file; l=line;}
	void exec(ptr<eosfunction> owner)
	{
		if(exe->argstack.size() < 2)
			log->add_error("file:%s line:%d stack error (+ needs 2 items on stack)!", f.c_str(), l);
		else
		{
			ptr<eosvariable>	arg1	= exe->argstack.top(); exe->argstack.pop();		
			ptr<eosvariable>	arg2	= exe->argstack.top(); exe->argstack.pop();

			exe->push(float( arg1->getd(0) + arg2->getd(0) ));

			//owner->reset_argidx();
			//owner->reset_argenv();
		}
	}
};

class eosinstr_sub : public eosinstr
{
public:
	eosinstr_sub(string file,int line) { name="sub";f=file; l=line;}
	void exec(ptr<eosfunction> owner)
	{
		if(exe->argstack.size() < 2)
			log->add_error("file:%s line:%d stack error (- needs 2 items on stack)!", f.c_str(), l);
		else
		{
			ptr<eosvariable>	arg1	= exe->argstack.top(); exe->argstack.pop();		
			ptr<eosvariable>	arg2	= exe->argstack.top(); exe->argstack.pop();
	
			exe->push(float( arg1->getd(0) - arg2->getd(0) ));

			//owner->reset_argidx();
			//owner->reset_argenv();
		}
	}
};

class eosinstr_mul : public eosinstr
{
public:
	eosinstr_mul(string file,int line) { name="mul";f=file; l=line;}
	void exec(ptr<eosfunction> owner)
	{
		if(exe->argstack.size() < 2)
			log->add_error("file:%s line:%d stack error (* needs 2 items on stack)!", f.c_str(), l);
		else
		{
			ptr<eosvariable>	arg1	= exe->argstack.top(); exe->argstack.pop();		
			ptr<eosvariable>	arg2	= exe->argstack.top(); exe->argstack.pop();

			exe->push(float( arg1->getd(0) * arg2->getd(0) ));

			//owner->reset_argidx();
			//owner->reset_argenv();
		}
	}
};

class eosinstr_div : public eosinstr
{
public:
	eosinstr_div(string file,int line) { name="div";f=file; l=line;}
	void exec(ptr<eosfunction> owner)
	{
		if(exe->argstack.size() < 2)
			log->add_error("file:%s line:%d stack error (/ needs 2 items on stack)!", f.c_str(), l);
		else
		{
			ptr<eosvariable>	arg1	= exe->argstack.top(); exe->argstack.pop();		
			ptr<eosvariable>	arg2	= exe->argstack.top(); exe->argstack.pop();

			exe->push(float( arg1->getd(0) / arg2->getd(0) ));

			//owner->reset_argidx();
			//owner->reset_argenv();
		}
	}
};

class eosinstr_cat : public eosinstr
{
public:
	eosinstr_cat(string file,int line) { name="cat";f=file; l=line;}
	void exec(ptr<eosfunction> owner)
	{
		if(exe->argstack.size() < 2)
			log->add_error("file:%s line:%d stack error (& needs 2 items on stack)!", f.c_str(), l);
		else
		{
			ptr<eosvariable>	arg1	= exe->argstack.top(); exe->argstack.pop();		
			ptr<eosvariable>	arg2	= exe->argstack.top(); exe->argstack.pop();

			exe->push(string( arg1->gets(0) + arg2->gets(0) ));

			//owner->reset_argidx();
			//owner->reset_argenv();
		}
	}
};

class eosinstr_cmp : public eosinstr
{
public:
	eosinstr_cmp(string file,int line) { name="cmp";f=file; l=line;}
	void exec(ptr<eosfunction> owner)
	{
		if(exe->argstack.size() < 2)
			log->add_error("file:%s line:%d stack error (condition needs 2 items on stack)!", f.c_str(), l);
		else
		{
			ptr<eosvariable>	arg1	= exe->argstack.top(); exe->argstack.pop();		
			ptr<eosvariable>	arg2	= exe->argstack.top(); exe->argstack.pop();

			cmp_vals(arg1->getv(),arg2->getv(), owner);

			//owner->reset_argidx();
			//owner->reset_argenv();
		}
	}
};


//
// JMPS
//
class eosinstr_jmp : public eosinstr
{
public:
	eosinstr_jmp(v& arg1,string file,int line)
	{ one=arg1; name="jmp";
		if(arg1.type==0) 				
		{
			if(is_number(arg1.name))
				temp.value = new eosvariable(float(_str2<float>(arg1.name)));
			else
				temp.value = new eosvariable(string(arg1.name));		
		}
		f=file; l=line;
	}

	void exec(ptr<eosfunction> owner)
	{
    owner->curr_lines = temp.value->geti(0);		
		
	  //owner->reset_argidx();
	}
};

class eosinstr_je : public eosinstr
{
public:
	eosinstr_je(v& arg1,string file,int line)
	{ one=arg1; name="je";
		if(arg1.type==0) 				
		{
			if(is_number(arg1.name))
				temp.value = new eosvariable(float(_str2<float>(arg1.name)));	
			else
				temp.value = new eosvariable(string(arg1.name));		
		}
		f=file; l=line;
	}

	void exec(ptr<eosfunction> owner)
	{
		if(owner->equal_flag>0)
      owner->curr_lines = temp.value->geti(0);		
	}
};

class eosinstr_jne : public eosinstr
{
public:
	eosinstr_jne(v& arg1,string file,int line)
	{ one=arg1; name="jne";
		if(arg1.type==0) 				
		{
			if(is_number(arg1.name))
				temp.value = new eosvariable(float(_str2<float>(arg1.name)));	
			else
				temp.value = new eosvariable(string(arg1.name));		
		}
		f=file; l=line;
	}

	void exec(ptr<eosfunction> owner)
	{
		if(owner->equal_flag<1)
      owner->curr_lines = temp.value->geti(0);		
	}
};

class eosinstr_jl : public eosinstr
{
public:
	eosinstr_jl(v& arg1,string file,int line)
	{ one=arg1; name="jl";
		if(arg1.type==0) 				
		{
			if(is_number(arg1.name))
				temp.value = new eosvariable(float(_str2<float>(arg1.name)));		
			else
				temp.value = new eosvariable(string(arg1.name));		
		}
		f=file; l=line;
	}

	void exec(ptr<eosfunction> owner)
	{
		if(owner->size_flag==0)
      owner->curr_lines = temp.value->geti(0);		
	}

};

class eosinstr_jle : public eosinstr
{
public:
	eosinstr_jle(v& arg1,string file,int line)
	{ one=arg1; name="jle";
		if(arg1.type==0) 				
		{
			if(is_number(arg1.name))
				temp.value = new eosvariable(float(_str2<float>(arg1.name)));		
			else
				temp.value = new eosvariable(string(arg1.name));		
		}
		f=file; l=line;
	}

	void exec(ptr<eosfunction> owner)
	{
		if((owner->equal_flag==1)||(owner->size_flag==0))
      owner->curr_lines = temp.value->geti(0);			
	}

};

class eosinstr_jg : public eosinstr
{
public:
	eosinstr_jg(v& arg1,string file,int line)
	{ one=arg1; name="jg";
		if(arg1.type==0) 				
		{
			if(is_number(arg1.name))
				temp.value = new eosvariable(float(_str2<float>(arg1.name)));	
			else
				temp.value = new eosvariable(string(arg1.name));		
		}
		f=file; l=line;
	}

	void exec(ptr<eosfunction> owner)
	{
		if(owner->size_flag==1)
      owner->curr_lines = temp.value->geti(0);		
	}
};


class eosinstr_jge : public eosinstr
{
public:
	eosinstr_jge(v& arg1,string file,int line)
	{ one=arg1; name="jge";
		if(arg1.type==0) 				
		{
			if(is_number(arg1.name))
				temp.value = new eosvariable(float(_str2<float>(arg1.name)));		
			else
				temp.value = new eosvariable(string(arg1.name));		

		}
		f=file; l=line;
	}

	void exec(ptr<eosfunction> owner)
	{
		if((owner->equal_flag==1)||(owner->size_flag==1))
      owner->curr_lines = temp.value->geti(0);		
	}
};

class eosinstr_call : public eosinstr
{
public:
	eosinstr_call(v& arg1,string file,int line)
	{ one=arg1; name=eosformat("call %s", arg1.name.c_str()); f=file; l=line;	}

	void exec(ptr<eosfunction> owner)
	{		
		if(one.type==0) 
		{
			if(exe->argenv->methods.find(one.name) == exe->argenv->methods.end())
				log->add_error("file:%s line:%d function '%s' not found!", f.c_str(), l,one.name.c_str());
			else
				exe->argenv->methods[one.name]->exec(exe->argenv);
		}
		if(one.type==1) 
		{
			ptr<eosvariable>	arg1	= exe->argstack.top(); exe->argstack.pop();		
			ptr<eosfuncbase>  f			= (eosfuncbase*)arg1->geto(0);
			
			if(f) f->exec(exe->argenv);
			else log->add_error("file:%s line:%d function '%s' not found!", this->f.c_str(), l,arg1->name.c_str());
		}
		if(one.type==2) 	
		{
			ptr<eosvariable>	arg1	= exe->argstack.top(); exe->argstack.pop();		
			ptr<eosfuncbase>  f			= (eosfuncbase*)arg1->geto(0);

			if(f) f->exec(exe->argenv);
			else log->add_error("file:%s line:%d function '%s' not found!", this->f.c_str(), l,arg1->name.c_str());
		}

		//owner->reset_args();
		owner->reset_argidx();
		owner->reset_argenv();
	}
};



class eosinstr_cobj : public eosinstr
{
public:
	eosinstr_cobj(string file,int line)
	{ name=eosformat("cobj"); f=file; l=line;}

	void exec(ptr<eosfunction> owner);

};

class eosinstr_cfunc : public eosinstr
{
public:
	eosinstr_cfunc(string file,int line)
	{ name=eosformat("cfunc"); f=file; l=line;}

	void exec(ptr<eosfunction> owner);
};

class eosinstr_cmeth : public eosinstr
{
public:
	eosinstr_cmeth(string file,int line)
	{ name=eosformat("cmeth"); f=file; l=line;}

	void exec(ptr<eosfunction> owner);
};


class eosinstr_flip : public eosinstr
{
public:
	eosinstr_flip(string file,int line) { name="flip"; f=file; l=line;}
	void exec(ptr<eosfunction> owner)
	{
		ptr<eosvariable>	arg1	= exe->argstack.top(); exe->argstack.pop();		
		exe->argidx.push( arg1 );

		owner->reset_argenv();
	}
};

class eosinstr_ret : public eosinstr
{
public:
	eosinstr_ret(string file,int line) { name="ret";f=file; l=line;}
	void exec(ptr<eosfunction> owner)
	{
		owner->curr_lines = (int)owner->instr.size();
	}
};

class eosinstr_address : public eosinstr
{
public:
	eosinstr_address(v& arg1,string file,int line)
	{ one=arg1; name="addr"; f=file; l=line; }
	void exec(ptr<eosfunction> owner)
	{	}

};



#endif