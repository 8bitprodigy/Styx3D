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

#ifndef EOSUTILLIB_H
#define EOSUTILLIB__H

/* all these functions can be ported to the script enviroment by
	 linking their libraries with vm->link_library. they are premade 
	 functions which should make life easier for scripters */
	 
#define EOSLIB_SYSTEM 1
// load a script
void scr_load();
// load a script, if loaded, load again 
void scr_reload();
// print to screen
void scr_print();
// pause system in ms
void scr_pause();
// write string to command line
void scr_cmdline();
// log msg
void scr_log();
// log error msg
void scr_logerr();

#define EOSLIB_ARRAY 2
// get size of array
void scr_getn();
// add array to end of array
void scr_append();
// remove element of array
void scr_remove();
// insert element in array
void scr_insert();
// clear an array
void scr_clear();

// all kind of math
#define EOSLIB_MATH 3
void scr_sin();
void scr_cos();
void scr_tan();
void scr_acos();
void scr_asin();
void scr_atan();
void scr_rand();
void scr_mod();

#define EOSLIB_STRING 4
void scr_atos();
void scr_stoa();
void scr_getc();
void scr_setc();
void scr_length();



#endif