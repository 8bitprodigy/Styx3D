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

#ifndef EOSVM_H
#define EOSVM_H

#define vm eosvm::inst()

/* this is the main "core" class. after created, the script engine 
   is initialised and you can work with eos.script. the class is called
   vm for virtual machine, but in fact its not the vm, it just handles
   garbage collection, loads script (or passes the script to the parser).
   all objects types for the script enviroment are also stored here, besides
   the global enviroment (thats why its so important)! 											*/
class eosvm : public eossingleton<eosvm>
{
public:
	friend eosanalyse;

	eosvm();
	~eosvm();

	/* collect all objects, only run this when exiting! */
	void	collect_all();
	/* collect garbage, called after loading.. */
	void	collect_garbage();

	/* load a script if not loaded */
	void	load(string path);	
	/* reload, means load it no matter if loaded before */
	void	reload(string path);

	/* register an object to the vm, represented by an eosobject class */
	void	register_type(string name, ptr<eosobject> o);
	/* link one of the eos.util libs, do this directly after init */
	void	link_library(int code);

public:
	// stores all object types
	map<string, ptr<eosobject> >	objects;

	ptr<eosobject>		genvi;	// the GLOBAL ENVIROMENT
	vector<string>		loaded;	// a list of all loaded scripts
};


#endif