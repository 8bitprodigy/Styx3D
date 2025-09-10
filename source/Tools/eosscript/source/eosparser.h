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

#ifndef EOSPARSER_H
#define EOSPARSER_H

#define	SYM_ADD		"__add__"	// +
#define	SYM_SUB		"__sub__"	// -
#define	SYM_MUL		"__mul__"	// *
#define	SYM_DIV		"__div__"	// /
#define	SYM_CAT		"__cat__"	// &
#define	SYM_SET		"__set__"	// =
#define	SYM_RET		"__ret__"	// ret
#define	SYM_NEW		"__new__"	// new
#define	SYM_FUNC	"__func__"	// func
#define	SYM_METH	"__meth__"	// meth
#define	SYM_OBR 	"__obr__"	// (
#define	SYM_CBR 	"__cbr__"	// )
#define	SYM_OID 	"__oid__"	// [
#define	SYM_CID 	"__cid__"	// ]
#define	SYM_OBL		"__obl__"	// {
#define	SYM_CBL		"__cbl__"	// }
#define SYM_END		"__end__" // ;
#define SYM_COMMA	"__com__" // ,
#define SYM_DPOINT "__dp__" // :
#define SYM_SMALL "__small__" // <
#define SYM_BIG   "__big__" // >
#define SYM_EXCL   "__excl__" // !
#define SYM_POINT "__point__" // .

#define	KEYW_FUNC	"function"
#define	KEYW_OBJ 	"object"
#define	KEYW_IF  	"if"
#define	KEYW_THEN	"then"
#define	KEYW_ELSE	"else"
#define	KEYW_FOR 	"for"
#define	KEYW_WHILE "while"
#define	KEYW_TO  	"to"
#define	KEYW_DO  	"do"
#define	KEYW_AND 	"and"
#define	KEYW_OR  	"or"
#define	KEYW_WITH	"with"
#define KEYW_DTO		"downto"


/*	this is the return structure of the parser. the parser loads and
		groups tokens to objects, (returned by load() method).					*/
struct object
{	vector< tokens > functions; };

/*	global definition for quick access to the parser */
#define parser eosparser::inst()

/*	parser class. there is only one instance of the parser, this is
		used by the virtual machine to load and group the symbols and 
		names of a script file. some methods are used by the analyser,
		because some math equations (indexed, params, ..) are left out
		by the parser to show they one value (they cant be tokened for 
		correct usage later..)																					*/
class eosparser : public eossingleton<eosparser>
{
public:
	eosparser();		
	~eosparser();		

	/*	loads a script file (param path) and groups them. the grouped
			tokens are returned in the object structure declared above.
			this is the only public method really used..									*/
	vector<object> load(string path);

	/*	accepts a string and removes all newline and tabulator constants which
			may confuse the analyser later on.. also deletes comments	*/
	void clean_nline(string& s);

	/*	outlines all symbols from the given string. outlining means that e.g. %a+%b
			is converted to %a + %b	so the tokenizer can easily put them into their own
			token. there are some symbols	which are not outlined: everything between 
			textlimiters "" (they are constants - converts to number if possible), indices
			of variable names (they are computed later, for now they have to stick to	the
			variable name) and points which ident floating numbers (and no enviroments)	*/
	void outline_symbols(string& s);

	/*	accepts a string and splits everything devided by 0x20s. returns the
			tokenized string in a tokens structure (vector of strings) */
	tokens tokenize(string& s);

	/*	replaces math chars with keyw constants */
	tokens	replace_keyw(tokens tok);

	/* this method is called from group_objects and simply groups all
		 functions out of the object token list. the result is pushed back
		 to o, a reference to the entire object-function-array.								*/
	vector<object> group_objects(tokens tok);

	/* this methods accepts a token as parameter and "groups out" the objects. this means,
		 when a keyword "object" is found, the end of the body is searched and then all tokens
		 which belongs to that object are stored in another token array. each object is then 
		 passed to the group_functions method which groups all functions inside an object,
		 including the "self" function (global). before and after that, the tokens are looped
		 several times to convert some syntax to internal mode and throw some warnings (as far
		 as its possible at this time) 																												*/
	void group_functions(tokens& tok, vector<object>& o);

	/* loops through the pattern and searches several parser errors */
	void find_errors(vector<tokens>& pattern);
	
protected:
	string	currfile;
};

#endif