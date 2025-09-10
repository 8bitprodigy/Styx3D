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

eosanalyse::eosanalyse()
{
	log->add("init analyser");
}

eosanalyse::~eosanalyse()
{
	log->add("close analyser");
}

string		eosanalyse::analyse_condition(tokens& f, int start, int end)
{
	string cond;
	
	for(int i=start; i<end; i++)
	{
    if((f[i].str==SYM_SMALL)||(f[i].str==SYM_BIG)||(f[i].str==SYM_SET)||(f[i].str==SYM_EXCL))
		{
			cond.append(f[i].str);			
			for(int j=i+1; j<=end; j++)
			{
				if((f[j].str!=SYM_SMALL)&&(f[j].str!=SYM_BIG)&&(f[j].str!=SYM_SET)&&(f[j].str!=SYM_EXCL))
				{
					analyse_statement(f,j,end);
					break;
				} else
					cond.append(f[j].str);
			}
			analyse_statement(f,start,i);
			break;
		}
	}

	return cond;
}


int		eosanalyse::analyse_block(tokens& f, int start, int end)
{
 	// loop through function source
	for(int i=start;i<end;i++)
	{
		if( f[i].str == KEYW_OBJ )
			i=find_str_v(SYM_OBL, f, i, -1);
		// - identify statement
		else if( f[i].str == SYM_CBL ) {}
		else if( f[i].str == KEYW_IF )
      i+=analyse_if(f,i,0);
		else if( f[i].str == KEYW_WHILE )
			i+=analyse_while(f,i,0);
		else if( f[i].str == KEYW_FOR )
			i+=analyse_for(f,i,0);
		else 
			i+=analyse_statement(f,i,find_str_v(SYM_END, f, i, -1));
	}

	return end-start;
}

int		eosanalyse::analyse_if(tokens& f, int start, int end)
{
	int count=0;	// counts the statemens processed by this function

	 int ende_addr = rand();
	 int start_addr = rand();

	bool running=true;
	int first = start+1;
	int last  = start;

	if(f[start].str==KEYW_IF)
	{
		last++;

		end	= find_str_v(KEYW_THEN, f, start, -1);

		while(running)
		{
			while(( f[last].str != "and" )&&( f[last].str != "or" )&&( f[last].str != "then" ))
			{
				last++;
				if( last >= end ) break;			
			}

			string cond = analyse_condition(f,first,last);	
			first=last+1;

			preinstr.push_back(new pre_instr("CMP",f[last].file,f[last].line));	

			if(f[last].str=="or")
			{
				// push back cond-jmp next
				ptr<pre_instr> jmp = new pre_instr(find_condition(cond),f[last].file,f[last].line);
				jmp->arg1.push_back(toka(_2str(start_addr),f[last].file,f[last].line));
				preinstr.push_back(jmp);	
			} 
			else
			{	
				// push back cond-jmp next
				ptr<pre_instr> jmp = new pre_instr(find_condition_rev(cond),f[last].file,f[last].line);
				jmp->arg1.push_back(toka(_2str(ende_addr),f[last].file,f[last].line));
				preinstr.push_back(jmp);	
			} 


			last++;
			if( last >= end ) running=false;	
		}

		count = (last-start);
	}

	int j = 0;
	int eq=-1;
	for(j=last+1; j<(int)f.size(); j++)
	{
		if(f[j].str==SYM_CBL)
			eq++;
		else if(f[j].str==SYM_OBL)
			eq--;

		if(eq==0)					
			break;					
	}

	ptr<pre_instr> jmp = new pre_instr("ADDR",f[j].file,f[j].line);
	jmp->arg1.push_back(toka(_2str(start_addr),f[j].file,f[j].line));
	preinstr.push_back(jmp);	
	
	count += analyse_block(f,last+1,j-1)+1;

	if(j<(int)f.size()-1)
	{		
		if(f[++j].str==KEYW_ELSE)
		{
			int total_end = rand();

			ptr<pre_instr> jmp = new pre_instr("JMP",f[j].file,f[j].line);
			jmp->arg1.push_back(toka(_2str(total_end),f[j].file,f[j].line));
			preinstr.push_back(jmp);	

			jmp = new pre_instr("ADDR",f[j].file,f[j].line);
			jmp->arg1.push_back(toka(_2str(ende_addr),f[j].file,f[j].line));
			preinstr.push_back(jmp);	

			count+=analyse_if(f,j+1,0)+2;

			jmp = new pre_instr("ADDR",f[j].file,f[j].line);
			jmp->arg1.push_back(toka(_2str(total_end),f[j].file,f[j].line));
			preinstr.push_back(jmp);
		} else
		{
			jmp = new pre_instr("ADDR",f[j].file,f[j].line);
			jmp->arg1.push_back(toka(_2str(ende_addr),f[j].file,f[j].line));
			preinstr.push_back(jmp);	
		}
		
		count++;
	} else
	{
		jmp = new pre_instr("ADDR",f[j].file,f[j].line);
		jmp->arg1.push_back(toka(_2str(ende_addr),f[j].file,f[j].line));
		preinstr.push_back(jmp);	
	}
	
	return count;
}
int		eosanalyse::analyse_while(tokens& f, int start, int end)
{
	int count=0;	// counts the statemens processed by this function

	 int ende_addr = rand();
	 int start_addr = rand();
	 int init_addr = rand();

	bool running=true;
	int first = start+1;
	int last  = start;

	if(f[start].str==KEYW_WHILE)
	{
		last++;

		end	= find_str_v(KEYW_DO, f, start, -1);
		if(end==-1)
			log->add_error("while-loop without 'do' keyword");

		ptr<pre_instr> jmp = new pre_instr("ADDR",f[last].file,f[last].line);
		jmp->arg1.push_back(toka(_2str(init_addr),f[last].file,f[last].line));
		preinstr.push_back(jmp);	

		while(running)
		{
			while(( f[last].str != KEYW_AND )&&( f[last].str != KEYW_OR )&&( f[last].str != KEYW_DO ))
			{
				last++;
				if( last >= end ) break;			
			}

			string cond = analyse_condition(f,first,last);	
			first=last+1;

			preinstr.push_back(new pre_instr("CMP",f[last].file,f[last].line));	

			if(f[last].str=="or")
			{
				// push back cond-jmp next
				ptr<pre_instr> jmp = new pre_instr(find_condition(cond),f[last].file,f[last].line);
				jmp->arg1.push_back(toka(_2str(start_addr),f[last].file,f[last].line));
				preinstr.push_back(jmp);	
			} 
			else
			{	
				// push back cond-jmp next
				ptr<pre_instr> jmp = new pre_instr(find_condition_rev(cond),f[last].file,f[last].line);
				jmp->arg1.push_back(toka(_2str(ende_addr),f[last].file,f[last].line));
				preinstr.push_back(jmp);	
			} 


			last++;
			if( last >= end ) running=false;	
		}

		count = (last-start);
	}

	int j = 0;
	int eq=-1;
	for(j=last+1; j<(int)f.size(); j++)
	{
		if(f[j].str==SYM_CBL)
			eq++;
		else if(f[j].str==SYM_OBL)
			eq--;

		if(eq==0)					
			break;					
	}

	ptr<pre_instr> jmp = new pre_instr("ADDR",f[j].file,f[j].line);
	jmp->arg1.push_back(toka(_2str(start_addr),f[j].file,f[j].line));
	preinstr.push_back(jmp);	
	
	count += analyse_block(f,last+1,j-1)+1;

	jmp = new pre_instr("JMP",f[j].file,f[j].line);
	jmp->arg1.push_back(toka(_2str(-init_addr),f[j].file,f[j].line));
	preinstr.push_back(jmp);	

	jmp = new pre_instr("ADDR",f[j].file,f[j].line);
	jmp->arg1.push_back(toka(_2str(ende_addr),f[j].file,f[j].line));
	preinstr.push_back(jmp);	

	return count;
}
int		eosanalyse::analyse_for(tokens& f, int start, int end)
{
	int count=0;	// counts the statemens processed by this function

	int ende_addr = rand();
	int start_addr = rand();
	int init_addr = rand();

	int conds =0;
	int conde =0;

	int i = 0;
	int last;
	if(f[start].str==KEYW_FOR)
	{
		for(i=start; i<(int)f.size(); i++)
			if((f[i].str=="to")||(f[i].str=="downto"))
			{
				conds=i+1;
				analyse_statement(f,start+1,i);
				break;
			}
		for(i=start+1; i<(int)f.size(); i++)
			if(f[i].str=="with")
			{
				conde=i;
				analyse_statement(f,i+1,i+2);
				break;
			}
		preinstr.push_back(new pre_instr("MOV",f[start].file,f[start].line));

		// start placen
		ptr<pre_instr> jmp = new pre_instr("ADDR",f[start].file,f[start].line);
		jmp->arg1.push_back(toka(_2str(start_addr),f[start].file,f[start].line));
		preinstr.push_back(jmp);	

		// push to-wert
		analyse_statement(f,conds,conde);
			
		// push itr
		analyse_statement(f,conde+1,conde+2);
		// cmp
		preinstr.push_back(new pre_instr("CMP",f[conds].file,f[conds].line));

		// push jl/jg ende
		if(f[conds-1].str == KEYW_TO)
			jmp = new pre_instr("JG",f[conds-1].file,f[conds-1].line);
		else if(f[conds-1].str == KEYW_DTO)
			jmp = new pre_instr("JL",f[conds-1].file,f[conds-1].line);

		jmp->arg1.push_back(toka(_2str(ende_addr),f[conds-1].file,f[conds-1].line));
		preinstr.push_back(jmp);	

		for(i=start+1; i<(int)f.size(); i++)
			if(f[i].str=="do")
			{
				count = i-start+1;
				last=i+1;
				break;
			}
	}

	int j = 0;
	int eq=-1;
	for(j=last+1; j<(int)f.size(); j++)
	{
		if(f[j].str==SYM_CBL)
			eq++;
		else if(f[j].str==SYM_OBL)
			eq--;

		if(eq==0)					
			break;					
	}

	count += analyse_block(f,last+1,j-1)+1;

	ptr<pre_instr> jmp = 0;
	
	jmp = new pre_instr("PUSHARG",f[j].file,f[j].line);
	jmp->arg2.push_back(toka("1",f[j].file,f[j].line));
	preinstr.push_back(jmp);	

	if(f[conds-1].str == "to")
		jmp = new pre_instr("ADD",f[conds-1].file,f[conds-1].line);
	else if(f[conds-1].str == "downto")
		jmp = new pre_instr("SUB",f[conds-1].file,f[conds-1].line);

	analyse_statement(f,conde+1,conde+2);
	preinstr.push_back(jmp);	

	jmp = new pre_instr("MOV",f[conde+1].file,f[conde+1].line);
	analyse_statement(f,conde+1,conde+2);
	preinstr.push_back(jmp);	

	jmp = new pre_instr("JMP",f[conde+1].file,f[conde+1].line);
	jmp->arg1.push_back(toka(_2str(-start_addr),f[conde+1].file,f[conde+1].line));
	preinstr.push_back(jmp);	

	jmp = new pre_instr("ADDR",f[conde+1].file,f[conde+1].line);
	jmp->arg1.push_back(toka(_2str(ende_addr),f[conde+1].file,f[conde+1].line));
	preinstr.push_back(jmp);	

	return count;
}
int		eosanalyse::analyse_statement(tokens& f, int start, int end)
{
	int count=0;	// counts the statemens processed by this function
	count	= end;
	ptr<pre_instr> tempinstr = 0;


	int active=0;	
	ptr<pre_instr> inst = new pre_instr("PUSHARG",f[count-1].file,f[count-1].line);
	for(int i=count-1; i>=start; i--)
	{
		if(f[i].str == SYM_ADD)
		{
			if(active==0)
			{
				inst->instr = "ADD";
				active=1;
			} else if(active==1)
			{
				preinstr.push_back(inst);

				inst = new pre_instr("ADD",f[i].file,f[i].line);		
				inst->arg1.clear();
				inst->arg2.clear();

				active=1;
			}
		} else
		if(f[i].str == SYM_SUB)
		{
			if(active==0)
			{
				inst->instr = "SUB";
				active=1;
			} else if(active==1)
			{
				preinstr.push_back(inst);	

				inst = new pre_instr("SUB",f[i].file,f[i].line);		
				inst->arg1.clear();
				inst->arg2.clear();

				active=1;
			}
		} else
		if(f[i].str == SYM_MUL)
		{
			if(active==0)
			{
				inst->instr = "MUL";
				active=1;
			} else if(active==1)
			{
				preinstr.push_back(inst);		

				inst = new pre_instr("MUL",f[i].file,f[i].line);			
				inst->arg1.clear();
				inst->arg2.clear();

				active=1;
			}
		} else
		if(f[i].str == SYM_DIV)
		{
			if(active==0)
			{
				inst->instr = "DIV";
				active=1;
			} else if(active==1)
			{
				preinstr.push_back(inst);	

				inst = new pre_instr("DIV",f[i].file,f[i].line);		
				inst->arg1.clear();
				inst->arg2.clear();

				active=1;
			}
		} else
		if(f[i].str == SYM_CAT)
		{
			if(active==0)
			{
				inst->instr = "CAT";
				active=1;
			} else if(active==1)
			{
				preinstr.push_back(inst);		

				inst = new pre_instr("CAT",f[i].file,f[i].line);	
				inst->arg1.clear();
				inst->arg2.clear();

				active=1;
			}
			
		} else
		if(f[i].str == SYM_SET)
		{
			if(active==0)
			{
				inst->instr = "MOV";
				active=1;
			} else if(active==1)
			{
				preinstr.push_back(inst);				

				inst = new pre_instr("MOV",f[i].file,f[i].line);				
				inst->arg1.clear();
				inst->arg2.clear();

				active=1;
			}
		} else
		if(f[i].str == SYM_RET)
		{
			if(active==0)
			{
				inst->instr = "RET";
				active=1;
			} else if(active==1)
			{
				preinstr.push_back(inst);

				inst = new pre_instr("RET",f[i].file,f[i].line);		
				inst->arg1.clear();
				inst->arg2.clear();

				active=1;
			}
		} else
		if(f[i].str == SYM_NEW)
		{
			if(active==0)
			{
				inst->instr = "COBJ";
				active=1;
			} else if(active==1)
			{
				preinstr.push_back(inst);

				inst = new pre_instr("COBJ",f[i].file,f[i].line);		
				inst->arg1.clear();
				inst->arg2.clear();

				active=1;
			}
		} else
		if(f[i].str == SYM_FUNC)
		{
			if(active==0)
			{
				inst->instr = "CFUNC";
				active=1;
			} else if(active==1)
			{
				preinstr.push_back(inst);

				inst = new pre_instr("CFUNC",f[i].file,f[i].line);		
				inst->arg1.clear();
				inst->arg2.clear();

				active=1;
			}
		} else
		if(f[i].str == SYM_METH)
		{
			if(active==0)
			{
				inst->instr = "CMETH";
				active=1;
			} else if(active==1)
			{
				preinstr.push_back(inst);

				inst = new pre_instr("CMETH",f[i].file,f[i].line);		
				inst->arg1.clear();
				inst->arg2.clear();

				active=1;
			}
		} else
		{
			if(f[i].str == SYM_CBR)
			{
				string temp = inst->instr;
				if(inst->instr != "PUSHARG")
				{
					inst->instr = "PUSHARG";
					if((!inst->arg1.empty())||(!inst->arg2.empty()))
						preinstr.push_back(inst);
					//inst = new pre_instr(temp,f[i].file,f[i].line);					
					inst = new pre_instr(temp,inst->file,inst->line);					
				}

				int j = 0;
				int eq=-1;
				for(j=i-1; j>=0; j--)
				{
					if(f[j].str==SYM_OBR)
						eq++;
					else if(f[j].str==SYM_CBR)
						eq--;

					if(eq==0)					
						break;					
				}

				i-=(analyse_statement(f, j+1, i)+1);

				if(inst->instr == "PUSHARG")
				{
					if((!inst->arg1.empty())||(!inst->arg2.empty()))
						preinstr.push_back(inst);	
				} else
				{
					//tempinstr = inst;
					preinstr.push_back(inst);	
				}

				inst = new pre_instr("CALL",f[i].file,f[i].line);
				active=1;
			} else
			if(f[i].str == SYM_OBR)
			{

			} else
			if(active==0)
			{
				inst->arg2.push_back(f[i]);
			} else
			if(active==1)
			{
				inst->arg1.push_back(f[i]);
			}

		}
	}

	if((inst->instr == "PUSHARG")||(inst->instr == "CALL"))
	{
		if((!inst->arg1.empty())||(!inst->arg2.empty()))
			preinstr.push_back(inst);	

		//if(tempinstr) preinstr.push_back(tempinstr);	
	} else
		preinstr.push_back(inst);	

	
	return count-start;
}

ptr<eosobject>	eosanalyse::prepare_object(tokens f)
{
	ptr<eosobject> o = 0;

	if(f.size() > 2 )
	{
		if( f[0].str == KEYW_OBJ )
		{
			o = new eosobject(f[1].str);
		}else
		{		
			o = new eosobject("self");
			return o;
		}
	} else
	{		
		o = new eosobject("self");
		return o;
	}

	int count				= find_str_v(SYM_OBL, f, 0, -1);
	int inherit_pos = find_str_v(SYM_DPOINT, f, 0, -1);

	if( inherit_pos > 0 )	
	{
		for(int j=inherit_pos+1; j<count; j++)
		{
			if((f[j].str != SYM_COMMA)&&(f[j].str != SYM_OBR)&&(f[j].str != SYM_CBR))
			{
				log->add("derived %s", f[j].str.c_str());

				if( vm->objects.find( f[j].str ) == vm->objects.end() )
					log->add_error("cant derive from '%s' - object not available!",f[j].str.c_str());
				else
				{
					if( vm->objects[ f[j].str ]->isclass )
					{
						o->cparents.push_back(vm->objects[f[j].str]);
						//o->cparents_name.push_back(f[j].str);
					}
					else
						o->inherit( vm->objects[ f[j].str ] );	
				}
			}
		}
	}

	return o;
}
ptr<eosfunction>  eosanalyse::new_function(tokens f, ptr<eosobject> owner)
{
	ptr<eosfunction> func = 0;

  bool noself = false;
	if(f.size() > 2 )
	{
		if( f[0].str == KEYW_FUNC )
		{
			func = new eosfunction(f[1].str);
			noself=true;
		}else
		{		
			func = new eosfunction("self");
		}
	} else
	{		
		func = new eosfunction("self");
		return func;
	}
	preinstr.clear();

	int start, ende;
	if(noself)
	{
		// find parameters and save
		start	= find_str_v(SYM_OBR, f, 0, -1);
		ende	= find_str_v(SYM_OBL, f, 0, -1);

		ptr<eosvariable> temp = 0;
		for(int i=start+1; i<ende; i++)
		{	
			if(f[i].str[0]=='%')
			{
				log->add("param: %s", f[i].str.c_str());
				temp = func->get_local(f[i].str);
				func->params.push_back(temp);
			}
		}

		start	= find_str_v(SYM_OBL, f, 0, -1);
		analyse_block(f,start+1,int(f.size())-1);
	}
	else
	{
		analyse_block(f,0,int(f.size()));
	}

	
	/*
	for(int i=0;i<(int)preinstr.size();i++)
	{
		log->add("preinstr: %s", preinstr[i]->instr.c_str());
		for(int j=0; j<(int)preinstr[i]->arg1.size(); j++)
			log->add("%s", preinstr[i]->arg1[j].str.c_str());
		log->add("2.arg:");
		for(j=0; j<(int)preinstr[i]->arg2.size(); j++)
			log->add("%s", preinstr[i]->arg2[j].str.c_str());
	}
	*/	

	link(func,preinstr);

	return func;
}

void eosanalyse::prepare_idx(string inst,ptr<eosfunction> f, string fil, int l)
{
	//printf("new: %s\n", inst.c_str());
	string s = find_index(inst);

	//printf("index: %s\n", s.c_str());
	parser->outline_symbols(s);

	//printf("indented symbols: %s\n", s.c_str());
	tokens tok = parser->tokenize(s);

	/*
	printf("new tok: ");
	for(int i=0; i<tok.size(); i++)
		printf("%s ", tok[i].c_str());
	printf("\n");
	*/

	if(tok.size() > 1)
	{
		preinstr.clear();
		analyse_statement(tok,0,(int)tok.size());
		link(f,preinstr);
		f->instr.push_back(new eosinstr_flip(fil,l));
	} else	
	if(!tok.empty())
	{
		if(tok[0].str!="0")
		{
			prepare_idx(tok[0].str,f,fil,l);
			f->instr.push_back(new eosinstr_pushidx(v(find_varname(tok[0].str)),fil,l));
		}
	}	
}

void eosanalyse::finish_idxarg(string inst,ptr<eosfunction> f, string fil, int l)
{
	string s="";

	while(s!="0")
	{
		next_index(inst);
		s = find_index(inst);
		parser->outline_symbols(s);
		tokens tok = parser->tokenize(s);

		if(tok.size() > 1)
		{
			preinstr.clear();
			analyse_statement(tok,0,(int)tok.size());
			link(f,preinstr);
			f->instr.push_back(new eosinstr_extarg(fil,l));
		} else	
		if(!tok.empty())
		{
			if(tok[0].str!="0")
			{
				prepare_idx(tok[0].str,f,fil,l);
				f->instr.push_back(new eosinstr_pusharg(v(find_varname(tok[0].str)),fil,l));
				f->instr.push_back(new eosinstr_extarg(fil,l));
			}
		}	
	}
}

void eosanalyse::finish_idxenv(string inst,ptr<eosfunction> f, string fil, int l)
{
	string s="";

	while(s!="0")
	{
		next_index(inst);
		s = find_index(inst);
		parser->outline_symbols(s);
		tokens tok = parser->tokenize(s);

		if(tok.size() > 1)
		{
			preinstr.clear();
			analyse_statement(tok,0,(int)tok.size());
			link(f,preinstr);
			f->instr.push_back(new eosinstr_extarg(fil,l));
		} else	
		if(!tok.empty())
		{
			if(tok[0].str!="0")
			{
				prepare_idx(tok[0].str,f,fil,l);
				f->instr.push_back(new eosinstr_pusharg(v(find_varname(tok[0].str)),fil,l));
				f->instr.push_back(new eosinstr_extarg(fil,l));
			}
		}	
	}
}


void eosanalyse::prepare_env(tokens& inst,ptr<eosfunction> f, bool recover, string fil, int l)
{
	if(inst.size()==0) return;
	if(inst.size()==1)
	{
		prepare_idx(inst[0].str,f,fil,l);
		f->instr.push_back(new eosinstr_pusharg(v(find_varname(inst[0].str)),fil,l));
		finish_idxarg(inst[0].str,f,fil,l);
	} else
	{
		for(int i=(int)inst.size()-1; i>=0; i--)
		{
			if((i==0)&&(inst[i].str!=SYM_POINT))
			{
				prepare_idx(inst[i].str,f,fil,l);
				f->instr.push_back(new eosinstr_pusharg(v(find_varname(inst[i].str)),fil,l));
				finish_idxarg(inst[i].str,f,fil,l);
			}
			else if((inst[i].str!=SYM_POINT))
			{
				prepare_idx(inst[i].str,f,fil,l);
				f->instr.push_back(new eosinstr_pusharg(v(find_varname(inst[i].str)),fil,l));
				finish_idxenv(inst[i].str,f,fil,l);
				f->instr.push_back(new eosinstr_pushenv(fil,l));			
			}
		}
		if(recover)
			f->instr.push_back(new eosinstr_recenv(fil,l));
	}
}

void eosanalyse::link(ptr<eosfunction> f, vector< ptr<pre_instr> > inst)
{
	int i = 0;
	for(i=0; i<(int)inst.size(); i++)
	{
		if(inst[i]->instr == "ADD")
		{
			if(!inst[i]->arg2.empty())	prepare_env(inst[i]->arg2,f, false,inst[i]->file,inst[i]->line);
			if(!inst[i]->arg1.empty())	prepare_env(inst[i]->arg1,f, false,inst[i]->file,inst[i]->line);
			f->instr.push_back(new eosinstr_add(inst[i]->file,inst[i]->line));
		} else
		if(inst[i]->instr == "SUB")
		{
			if(!inst[i]->arg2.empty())	prepare_env(inst[i]->arg2,f, false,inst[i]->file,inst[i]->line);
			if(!inst[i]->arg1.empty())	prepare_env(inst[i]->arg1,f, false,inst[i]->file,inst[i]->line);
			f->instr.push_back(new eosinstr_sub(inst[i]->file,inst[i]->line));
		} else
		if(inst[i]->instr == "DIV")
		{
			if(!inst[i]->arg2.empty())	prepare_env(inst[i]->arg2,f, false,inst[i]->file,inst[i]->line);
			if(!inst[i]->arg1.empty())	prepare_env(inst[i]->arg1,f, false,inst[i]->file,inst[i]->line);
			f->instr.push_back(new eosinstr_div(inst[i]->file,inst[i]->line));
		} else
		if(inst[i]->instr == "MUL")
		{
			if(!inst[i]->arg2.empty())	prepare_env(inst[i]->arg2,f, false,inst[i]->file,inst[i]->line);
			if(!inst[i]->arg1.empty())	prepare_env(inst[i]->arg1,f, false,inst[i]->file,inst[i]->line);
			f->instr.push_back(new eosinstr_mul(inst[i]->file,inst[i]->line));
		} else
		if(inst[i]->instr == "CAT")
		{
			if(!inst[i]->arg2.empty())	prepare_env(inst[i]->arg2,f, false,inst[i]->file,inst[i]->line);
			if(!inst[i]->arg1.empty())	prepare_env(inst[i]->arg1,f, false,inst[i]->file,inst[i]->line);
			f->instr.push_back(new eosinstr_cat(inst[i]->file,inst[i]->line));
		} else
		if(inst[i]->instr == "MOV")
		{
			if(!inst[i]->arg2.empty())	prepare_env(inst[i]->arg2,f, false,inst[i]->file,inst[i]->line);
			if(!inst[i]->arg1.empty())	prepare_env(inst[i]->arg1,f, false,inst[i]->file,inst[i]->line);
			f->instr.push_back(new eosinstr_mov(inst[i]->file,inst[i]->line));
		} else
		if(inst[i]->instr == "PUSHARG")
		{
			if(!inst[i]->arg2.empty())
			{
				prepare_env(inst[i]->arg2,f, false,inst[i]->file,inst[i]->line);	
				//f->instr.push_back(new eosinstr_pusharg(v(inst[i]->arg2[0])));
			}
		} else
		if(inst[i]->instr == "RET")
		{
			f->instr.push_back(new eosinstr_ret(inst[i]->file,inst[i]->line));
		} else
		if(inst[i]->instr == "COBJ")
		{
			f->instr.push_back(new eosinstr_cobj(inst[i]->file,inst[i]->line));
		} else
		if(inst[i]->instr == "CFUNC")
		{
			f->instr.push_back(new eosinstr_cfunc(inst[i]->file,inst[i]->line));
		} else
		if(inst[i]->instr == "CMETH")
		{
			f->instr.push_back(new eosinstr_cmeth(inst[i]->file,inst[i]->line));
		} else
		if(inst[i]->instr == "CMP")
		{
			f->instr.push_back(new eosinstr_cmp(inst[i]->file,inst[i]->line));
		} else
		if(inst[i]->instr == "ADDR")
		{
			f->instr.push_back(new eosinstr_address(v(inst[i]->arg1[0].str),inst[i]->file,inst[i]->line));
		} else
		if(inst[i]->instr == "JE")
		{
			f->instr.push_back(new eosinstr_je(v(inst[i]->arg1[0].str),inst[i]->file,inst[i]->line));
		} else
		if(inst[i]->instr == "JNE")
		{
			f->instr.push_back(new eosinstr_jne(v(inst[i]->arg1[0].str),inst[i]->file,inst[i]->line));
		} else
		if(inst[i]->instr == "JG")
		{
			f->instr.push_back(new eosinstr_jg(v(inst[i]->arg1[0].str),inst[i]->file,inst[i]->line));
		} else
		if(inst[i]->instr == "JGE")
		{
			f->instr.push_back(new eosinstr_jge(v(inst[i]->arg1[0].str),inst[i]->file,inst[i]->line));
		} else
		if(inst[i]->instr == "JL")
		{
			f->instr.push_back(new eosinstr_jl(v(inst[i]->arg1[0].str),inst[i]->file,inst[i]->line));
		} else
		if(inst[i]->instr == "JLE")
		{
			f->instr.push_back(new eosinstr_jle(v(inst[i]->arg1[0].str),inst[i]->file,inst[i]->line));
		} else
		if(inst[i]->instr == "JMP")
		{
			f->instr.push_back(new eosinstr_jmp(v(inst[i]->arg1[0].str),inst[i]->file,inst[i]->line));
		} else
		if(inst[i]->instr == "CALL")
		{
			bool recover=false;
			if(!inst[i]->arg1.empty())
			{
				string funcname = inst[i]->arg1[0].str;
				if((funcname[0]!='$')&&(funcname[0]!='%'))
				{
					inst[i]->arg1.erase( inst[i]->arg1.begin() );          
				} else
					recover=true;
					
				prepare_env(inst[i]->arg1,f,recover,inst[i]->file,inst[i]->line);
				f->instr.push_back(new eosinstr_call(v(find_varname(funcname)),inst[i]->file,inst[i]->line));
			} else
			if(!inst[i]->arg2.empty())
			{
				string funcname = inst[i]->arg2[0].str;
				if((funcname[0]!='$')&&(funcname[0]!='%'))
				{
					inst[i]->arg2.erase( inst[i]->arg2.begin() );          
				} else
					recover=true; 
					
				prepare_env(inst[i]->arg2,f,recover,inst[i]->file,inst[i]->line);
				f->instr.push_back(new eosinstr_call(v(find_varname(funcname)),inst[i]->file,inst[i]->line));
			}
		} 
	}

	
	int difference=0;
	for(i=0; i<(int)f->instr.size(); i++)
	{
		if(f->instr[i]->name == "addr")
			difference--;
		else
		if(f->instr[i]->name[0] == 'j')
		{
			int addr = _str2<int>(f->instr[i]->one.name);
			int count = 0;

			if(addr<0)
			{
				addr = -addr;
				for(int j=i; j>=0; j--)
				{
					if( f->instr[j]->name == "addr" )
					{
						if( addr == _str2<int>(f->instr[j]->one.name) )
						{
							count++;
							f->instr[i]->name = eosformat("%s %d", f->instr[i]->name.c_str(), (i+difference-count+2));
							f->instr[i]->temp.value->set(0,i+difference-count+1);
							break;
						}
					}
					else count++;
				}
			}
			else
			{
				count=0;
				for(int j=i+1; j<(int)f->instr.size(); j++)
				{
					if( f->instr[j]->name == "addr" )
					{
						if( addr == _str2<int>(f->instr[j]->one.name) )
						{
							f->instr[i]->name = eosformat("%s %d", f->instr[i]->name.c_str(), (count+i+difference+1));
							f->instr[i]->temp.value->set(0,(count+i+difference));
							break; 
						}
					}
					else count++;
				}
			}
		}
	}

	
	for(i=0; i<(int)f->instr.size(); i++)
	{
		if(f->instr[i]->name == "addr")
		{		
			f->instr.erase(f->instr.begin()+i);
			i--;
		}
	}
	
	
}

ptr<eosobject>  eosanalyse::new_object(object& o)
{
	if(!o.functions.size()) return 0;

	tokens f = o.functions[ o.functions.size()-1 ];

	ptr<eosobject> nobj = this->prepare_object(f);

	if( !nobj ) return 0;

	log->add("filtering functions..");
	ptr<eosfunction> func = 0;
	for(int i=0; i<(int)o.functions.size(); i++)
	{
		func = new_function(o.functions[i], nobj);
		if(func) 
		{
			if(func->name == "self")
			{
				if(!nobj->self)
					nobj->self = func;
				else
					nobj->self->mix((eosfunction*)func);
			}
			if(func->name[0] == '$')
			{
				nobj->set_global(find_varname(func->name), _str2<int>(find_index(func->name)), (eosfunction*)func);
			} else
			{
				nobj->methods[find_varname(func->name)] = func;
			}
		}
	}

  return nobj;
}

void	eosanalyse::interprete(vector<object> o)
{
	if(!o.size())
	{
		log->add_error("script seems to contain no source!");
		return;
	}

	log->add("filtering objects..");
	ptr<eosobject> obj = 0;
	for(int i=0; i<(int)o.size(); i++)
	{
		obj =	new_object(o[i]);

		if(obj)
		{
			if(obj->name == "self")
			{
				vm->genvi->inherit(obj);
				obj->self->exec(vm->genvi);
			}
			
			
			if(obj->name[0] == '$')
			{
				ptr<eosobject> o=obj->create();
				o->name = obj->name;
				obj=o;	

				vm->genvi->set_global(find_varname(obj->name), _str2<int>(find_index(obj->name)), obj);
				obj->self->exec(obj);
			} else
				vm->objects[find_varname(obj->name)] = obj;
		}
	}
}
