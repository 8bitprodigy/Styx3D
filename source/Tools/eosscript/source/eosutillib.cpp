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


// load a script
void scr_load()
{ vm->load(exe->pop()->gets(0)); }
// load s script, if loaded, reload
void scr_reload()
{ vm->load(exe->pop()->gets(0)); }
// print to screen
void scr_print()
{ printf("%s\n", exe->pop()->gets(0).c_str()); }
// pause system in ms
void scr_pause()
{ Sleep(exe->pop()->geti(0)); }
// write string to command line
void scr_cmdline()
{ system(exe->pop()->gets(0).c_str());}
// log message
void scr_log()
{ log->add("%s",exe->pop()->gets(0).c_str()); }
// log error message
void scr_logerr()
{ log->add_error("%s",exe->pop()->gets(0).c_str()); }
// get size of array
void scr_getn()
{ exe->push( exe->pop()->count() ); }
// add array to end of array
void scr_append()
{
	ptr<eosvariable> one = exe->pop();
	ptr<eosvariable> two = exe->pop();

	int j=one->count();
	for(int i=0; i<two->count(); i++)
		one->set(j++,two->get(i));
}
// remove element of array
void scr_remove()
{
	ptr<eosvariable>	one = exe->pop();
	int								i = exe->pop()->geti(0); 

	one->delete_element( i );	
}
// insert element in array
void scr_insert()
{
	ptr<eosvariable>	one = exe->pop();
	int								i = exe->pop()->geti(0); 
	ptr<eosvariable>	two = exe->pop();
	
	one->insert_element( two, i );	
}

void scr_clear()
{	exe->pop()->clear(); }

void scr_sin()
{ exe->push( (float)sinf((float)exe->pop()->getd(0)) ); }
void scr_cos()
{ exe->push( (float)cosf((float)exe->pop()->getd(0)) ); }
void scr_tan()
{ exe->push( (float)tanf((float)exe->pop()->getd(0)) ); }
void scr_acos()
{ exe->push( (float)acosf((float)exe->pop()->getd(0)) ); }
void scr_asin()
{ exe->push( (float)asinf((float)exe->pop()->getd(0)) ); }
void scr_atan()
{ exe->push( (float)atanf((float)exe->pop()->getd(0)) ); }
void scr_rand()
{	exe->push( rand() ); }
void scr_mod()
{	exe->push( exe->pop()->geti(0) % exe->pop()->geti(0) ); }

void scr_atos()
{
	ptr<eosvariable> v = exe->pop();

	string retv;
	for(int i=0; i<v->count(); i++)
		retv.append( v->gets(i) );

	exe->push(retv);
}

void scr_stoa()
{
	string s = exe->pop()->gets(0);
	ptr<eosvariable> var = exe->req_temp();

	for(int i=0; i<(int)s.size(); i++)
		var->set(i, s.substr(i,1));

  exe->argstack.push(var);
}

void scr_getc()
{
	string s = exe->pop()->gets(0);
	int idx = exe->pop()->geti(0);
	s = s[idx];
	exe->push(s);    
}

void scr_setc()
{
	ptr<eosvariable> var = exe->pop();
	int idx = exe->pop()->geti(0);
	string s = exe->pop()->gets(0);
	
	string str = var->gets(0);
	str[idx] = s[0];
	var->set(0,str);	
}
void scr_length()
{
	string s = exe->pop()->gets(0);
	exe->push( (int)s.size() );
}


