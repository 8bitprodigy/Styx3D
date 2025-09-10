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


/* copy value from another eosvalue class */
 void	eosvalue::copyv(ptr<eosvalue> val)
{
	// take vartype and then copy the value
	this->valtype = val->valtype;

	if(valtype == 1)	seti(val->geti());
	else if(valtype == 2)	setd(val->getd());
	else if(valtype == 3)	sets(val->gets());
	else if(valtype == 4)	seto(val->geto());
	else seti(-1);
}

/* set value of this class for each type */
 void	eosvalue::seti(int i)
{valtype=1; *ival=i; *dval=(float)i; };
 void	eosvalue::setd(float f)	
{valtype=2; *dval=f; *ival=(int)f; };
 void	eosvalue::sets(string s)	
{valtype=3; *sval=s;};
 void	eosvalue::seto(void* o)	
{valtype=4; oval=o;};

/* bind c-variable to this value for each type */
 void	eosvalue::bindi(int* i)		{ valtype=1; ival=i; if(i==0) ival=&iundef; };
 void	eosvalue::bindd(float* f)	{valtype=2; dval=f; if(f==0) dval=&dundef; };
 void	eosvalue::binds(string* s)	{valtype=3; sval=s; if(s==0) sval=&sundef; };
 void	eosvalue::bindo(void* o)	{valtype=4; oval=o; };

/* get value of this class for each type */
 int		eosvalue::geti() {		
	if(valtype==1)  return (*ival); else
	if(valtype==2)  return (int)*dval; else
	if(valtype==3)  return _str2<int>(*sval); else
	return 0;
}

/* get value of this class for each type */
 float		eosvalue::getd() {		
	if(valtype==1)  return (float)*ival; else
	if(valtype==2)  return *dval; else
	if(valtype==3)  return _str2<float>(*sval); else
	return 0;
}

/* get value of this class for each type */
 string	eosvalue::gets() {		
	if(valtype==1)  return _2str(*ival); else
	if(valtype==2)  return _2str(*dval); else
	if(valtype==3)  return (*sval); else
	if(valtype==4)	return eosformat("object:0x%X",oval);

	return 0;
}

/* get value of this class for each type */
 void* eosvalue::geto()	{
	if(valtype==4)	return oval;
	return 0;
}
