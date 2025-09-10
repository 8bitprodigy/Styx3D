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

#ifndef EOSSCRIPT_H
#define EOSSCRIPT_H

#ifdef _DEBUG
#pragma comment(linker, "/nodefaultlib:libc.lib")
#endif

// diable hash old warning
#pragma warning(disable : 4996)


#include <windows.h>
#include <stdio.h>
#include <string>
#include <vector>
#include <list>
#include <map>
#include <stack>
#include <strstream>
#include <ctime>
#include <math.h>

using std::string;
using std::vector;
using std::list;
using std::stack;
using std::map;


struct toka
{
	toka(string s, string f, int l) : str(s), file(f), line(l) {}
	toka() {}
	string str;
	int		 line;
	string file;
};

#define tokens vector<toka> 

#include "eosutil.h"
#include "eosbase.h"
#include "eosptr.h"
#include "eoslog.h"

#include "eosvalue.h"
#include "eosvariable.h"
#include "eosfunction.h"
#include "eosobject.h"


#include "eosparser.h"
#include "eosinstr.h"
#include "eosanalyse.h"
#include "eosutillib.h"
#include "eosvm.h"


#define cpp_method(inst, class, name) (new method<class>(inst, &class::name))
#define cpp_class(class) (new eoscobject<class>())


#endif