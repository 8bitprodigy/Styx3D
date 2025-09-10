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

eosvm::eosvm()
{
	// init random calculator
	srand( (unsigned)time( NULL ) );
	// create log instance
	new eoslog();
	log->add("init vm");
		
	// alloc memory for global enviroment and add retv
	genvi = new eosobject("genvi");
	// register global return value
	new eosexec(genvi);

	// add the gl enviroment to the object list
	objects["genvi"] = genvi;

	// create parser + analyser
	new eosparser();
	new eosanalyse();
}

eosvm::~eosvm()
{
	log->add("close vm");
	genvi	= 0;
	
	// delete parser + analyser
	delete parser;
	delete analyser;
	
	// collect all objects and free them before exiting
	collect_all();
	// last but not least, close logfiles
	delete eoslog::inst();	
}

/* collect garbage, called after loading.. */
void eosvm::collect_garbage()
{
	int size = (int)eosbase::garb_list.size();
	log->add("collect_garbage: %d object(s)", size);

	// if there is garbage
	if (size > 0)
	{
		eosbase* temp = 0;
		// loop through static list and delete them
		while(eosbase::garb_list.size())
		{
			if(eosbase::garb_list[0])
			{
				temp = eosbase::garb_list[0];				
				eosbase::garb_list.erase(eosbase::garb_list.begin());				
				delete temp;
			}
		}
		eosbase::garb_list.clear();
	}	

}

/* collect all objects, only run this when exiting! */
void eosvm::collect_all()
{
	// we dont count from now on, or the list will
	// get smaller and smaller while deleting
	eosbase::ref_count(false);

	int size = (int)eosbase::obj_list.size();
	log->add("collect_all: %d object(s)", size);

	if (size > 0)
	{
		eosbase* temp = 0;
		
		// loop through all objects and delete them!
		while(eosbase::obj_list.size())
		{
			if(eosbase::obj_list[0])
			{
				temp = eosbase::obj_list[0];				
				eosbase::obj_list.erase(eosbase::obj_list.begin());				
				delete temp;
			}
		}
		eosbase::obj_list.clear();
	}	

	// collect garbage and we're finished!
	collect_garbage();

	//eosbase::ref_count(true);
}

/* load a script if not loaded */
void	eosvm::load(string path)
{
	bool found = false;
	// if not loaded, load it
	for(int i=0; i<(int)loaded.size(); i++)
		if(loaded[i] == path) found = true;

	if(!found)
	{		
		loaded.push_back(path);
		analyser->interprete(parser->load(path));	
		collect_garbage();
	}
}

/* reload, means load it no matter if loaded before */
void	eosvm::reload(string path)
{
	loaded.push_back(path);
	analyser->interprete(parser->load(path));	
	collect_garbage();
}

/* link one of the eos.util libs, do this directly after init */
void	eosvm::link_library(int code)
{
	// system lib
	if( code == EOSLIB_SYSTEM )
	{
		genvi->register_func("load", scr_load );
		genvi->register_func("reload", scr_reload );
		genvi->register_func("print", scr_print );
		genvi->register_func("pause", scr_pause );
		genvi->register_func("cmdline", scr_cmdline );
		genvi->register_func("log", scr_log );
		genvi->register_func("logerr", scr_logerr );
	}
	// array lib
	else 	if( code == EOSLIB_ARRAY )
	{
		genvi->register_func("getn", scr_getn );
		genvi->register_func("append", scr_append );		
		genvi->register_func("remove", scr_remove );	
		genvi->register_func("insert", scr_insert );
		genvi->register_func("clear", scr_clear );
	}
	// math lib
	else 	if( code == EOSLIB_MATH )
	{
		genvi->register_func("sin", scr_sin );
		genvi->register_func("cos", scr_cos );		
		genvi->register_func("sin", scr_tan );
		genvi->register_func("asin", scr_asin );		
		genvi->register_func("acos", scr_acos );
		genvi->register_func("atan", scr_atan );		
		genvi->register_func("rand", scr_rand );	
		genvi->register_func("mod", scr_mod );	
	}
	else 	if( code == EOSLIB_STRING )
	{

		genvi->register_func("atos", scr_atos );
		genvi->register_func("stoa", scr_stoa );		
		genvi->register_func("getc", scr_getc );	
		genvi->register_func("setc", scr_setc );	
		genvi->register_func("length", scr_length );	
	}
}

/* register an object to the vm, represented by an eosobject class */
void	eosvm::register_type(string name, ptr<eosobject> o)
{ 
	o->name = name;
	vm->objects[name] = o;
}
