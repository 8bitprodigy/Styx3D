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

#ifndef EOSANALYSE_H
#define EOSANALYSE_H

struct pre_instr : public eosbase
{
	pre_instr(string name, string f, int l) : instr(name), file(f), line(l) {}
	string	instr;
	tokens	arg1;
	tokens	arg2;

	string	file;
	int			line;
};

#define analyser eosanalyse::inst()
class eosanalyse : public eossingleton<eosanalyse>
{
public:
	eosanalyse();
	~eosanalyse();

  void	interprete(vector<object> o);
	void	link(ptr<eosfunction> f, vector< ptr<pre_instr> > inst);

	ptr<eosobject>		new_object(object& o);
	ptr<eosfunction>  new_function(tokens f, ptr<eosobject> owner);
	ptr<eosobject>		prepare_object(tokens f);
	void							derive_object(tokens f, ptr<eosobject> o);
	void							prepare_env(tokens& inst,ptr<eosfunction> f, bool recover, string fil, int l);
	void							prepare_idx(string inst,ptr<eosfunction> f, string fil, int l);
	void							finish_idxarg(string inst,ptr<eosfunction> f, string fil, int l);
	void							finish_idxenv(string inst,ptr<eosfunction> f, string fil, int l);

	int		analyse_block(tokens& f, int start, int end);
	string		analyse_condition(tokens& f, int start, int end);
	int		analyse_if(tokens& f, int start, int end);
	int		analyse_while(tokens& f, int start, int end);
	int		analyse_for(tokens& f, int start, int end);
	int		analyse_statement(tokens& f, int start, int end);

private:
	vector< ptr<pre_instr> >	preinstr;

	friend eosobject;
};

#endif