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
#include "../source/eossyntax.h"

/*	defines all symbols to check when outlining symbols of a script ... */
extern char stx_symb_v[] = {	stx::end_statement, stx::open_block, stx::close_block,
											stx::open_bracket, stx::close_bracket, stx::point, 
											stx::comma,	stx::dpoint, stx::add, stx::set, stx::sub,
											stx::mul, stx::div, stx::cat, stx::smaller, stx::bigger,
											stx::textlimit};


/* ctor */
eosparser::eosparser()
{
	log->add("init parser");
}
/* dtor */
eosparser::~eosparser()
{
	log->add("close parser");
}

/*	loads a script file (param path) and groups them. the grouped
		tokens are returned in the object structure declared above.
		this is the only public method really used..									*/
vector<object> eosparser::load(string path)
{
	currfile = path;

	// open file..
	FILE* f = fopen(path.c_str(), "rb+");

	if(!f) { 
		log->add_error("cant load %s", path.c_str());
	} else 
	{
		log->add("loading %s", path.c_str());

		// find out length
		fseek(f,0,SEEK_END);
		long size = ftell(f);
		rewind(f);

		// load complete file to sbuffer
		string sbuffer;
		sbuffer.resize(size);
		fread((void*)sbuffer.c_str(), 1, size, f);
		// no need for the file, close it
		fclose(f);
	
		// format data with parser methods..
		log->add("parsing..", path.c_str());
		clean_nline(sbuffer);
		outline_symbols(sbuffer);
		// replace_keyw
		return group_objects(replace_keyw(tokenize(sbuffer)));
	}

	// return empty vector if script cant be loaded
	vector<object> empty;
	return empty;
}

/*	replaces math chars with keyw constants */
tokens eosparser::replace_keyw(tokens tok)
{
	for(int i=0; i<(int)tok.size(); i++)
	{
		
		if(tok[i].str[0] == '"')
		{
			//tok[i].str.erase(tok[i].str.begin());
		} 
		else if(tok[i].str == "+")
		{ tok[i].str = SYM_ADD; }
		else if(tok[i].str == "-")
		{ tok[i].str = SYM_SUB; }
		else if(tok[i].str == "*")
		{ tok[i].str = SYM_MUL; }
		else if(tok[i].str == "/")
		{ tok[i].str = SYM_DIV; }
		else if(tok[i].str == "&")
		{ tok[i].str = SYM_CAT; }
		else if(tok[i].str == "=")
		{ tok[i].str = SYM_SET; }
		else if(tok[i].str == "return")
		{ tok[i].str = SYM_RET; }
		else if(tok[i].str == "new")
		{ tok[i].str = SYM_NEW; }
		else if(tok[i].str == "func")
		{ tok[i].str = SYM_FUNC; }
		else if(tok[i].str == "method")
		{ tok[i].str = SYM_METH; }
		else if(tok[i].str == "(")
		{ tok[i].str = SYM_OBR; }
		else if(tok[i].str == ")")
		{ tok[i].str = SYM_CBR; }
		else if(tok[i].str == "[")
		{ tok[i].str = SYM_OID; }
		else if(tok[i].str == "]")
		{ tok[i].str = SYM_CID; }
		else if(tok[i].str == "{")
		{ tok[i].str = SYM_OBL; }
		else if(tok[i].str == "}")
		{ tok[i].str = SYM_CBL; }
		else if(tok[i].str == ";")
		{ tok[i].str = SYM_END; }
		else if(tok[i].str == ":")
		{ tok[i].str = SYM_DPOINT; }
		else if(tok[i].str == ",")
		{ tok[i].str = SYM_COMMA; }
		else if(tok[i].str == "<")
		{ tok[i].str = SYM_SMALL; }
		else if(tok[i].str == ">")
		{ tok[i].str = SYM_BIG; }
		else if(tok[i].str == "!")
		{ tok[i].str = SYM_EXCL; }
		else if(tok[i].str == ".")
		{ tok[i].str = SYM_POINT; }
		else if(tok[i].str[0] == 10)
		{ tok[i].str = SYM_POINT; }
	}

	return tok;
}


/*	accepts a string and removes all newline and tabulator constants which
		may confuse the analyser later on.. also deletes comments	*/
void eosparser::clean_nline(string& s)
{
	// loop through string and remove all characters specified:
	// 9=TAB 10=NL 11=VTAB 13==CRETURN (combination with 10)
	for(int i=0; i<(int)s.size(); i++)
	{
		int j = 0;
		// if comment starts
		if(( s[i] == '/' )&&( s[i+1] == '*' ))
		{
			int noleft=1;
			s[i] = ' ';
			s[++i] = ' ';
			// find end of comment and erase text between
			for(j=i+1; j<(int)s.size(); j++)
				if(( s[j] == '*' )&&( s[j+1] == '/' ))
				{
					noleft--;
					if(noleft==0) break;
					else s[j] = ' ';
				} else
				if(( s[j] == '/' )&&( s[j+1] == '*' ))
				{
          noleft++;
					s[j] = ' ';
				} else
				if( s[j] == 10 )
					s[j] = 10;
				else
					s[j] = ' ';

			s[j] = ' ';
			s[++j] = ' ';
			i=j;
		} else
		if(( s[i] == '/' )&&( s[i+1] == '/' ))
		{
			s[i] = ' ';
			s[++i] = ' ';
			// find end of comment and erase text between
			for(j=i+1; j<(int)s.size(); j++)
				if(( s[j] == 10 ))
				{
          break;
				} else
					s[j] = ' ';

			//s[j] = ' ';
			//s[++j] = ' ';
			i=j;
		} else

		if( ( s[i] == 13 )||( s[i] == 11 )||( s[i] == 9 ) )
      s[i] = ' ';
	}

  // since the string is tokenized later and a newline forces a new contant,
	// we just replace the characters with 0x20s instead of erasing...
}

/*	accepts a string and splits everything devided by 0x20s. returns the
		tokenized string in a tokens structure (vector of strings) */
tokens eosparser::tokenize(string& s)
{
	int			line = 1;

	toka			token;			// the current token processed
	tokens	tokenized;	// return value, array of strings

	token.file = currfile;	

	// loop through the entire string
	for(int i=0; i<(int)s.size(); i++)
	{
		// a textmessage is a static constant and cant be tokenized. if a textmsg
		// starts, we push back the current processed token, loop to the end of the
		// textmsg and continue at this point (simply ignore the textmsg)
		if( ( s[i] == stx::textlimit ) )
		{
			// if another token was processed before, push it back
			if(!token.str.empty())
			{
				token.line = line;
				tokenized.push_back(token);
			}
			// .. and clear the string for the new token
			token.str.clear();
			token.str.push_back('"');
			
			// loop to the end of the textmessage
			for(int j=i+1; j<(int)s.size(); j++)
			{				
				// if the end if found, break and add counter(i)
				if( ( s[j] == stx::textlimit ) )
				{
					i=j+1; break;
				} else
				// else we push back each char, or the testmsg is lost
				token.str.push_back(s[j]);

				if(j==(int)s.size())
				{
					log->add_error("missing textlimiter!");
					break;
				}
			}

			// should be empty, but test and clear for security
			if(!token.str.empty())
			{
				token.line = line;
				tokenized.push_back(token);
			}
			token.str.clear();
		}
		// remove nline symbol
		else if( ( s[i] == 10 ) )
      line++;
		// else if there is no textmessage, check if the currently 
		// processed token is finished, if not, push back char to the
		// current token.str..
		else if( ( s[i] != ' ' ) )
      token.str.push_back(s[i]);
		// if theres a 0x20, push back current token if not empty
		else if( !token.str.empty() )
		{
			token.line = line;
			// push back and clear
			tokenized.push_back(token);
			token.str.clear();
		}		
	}

	// the last token is not divided by a 0x20, so check if theres 
	// something in the last token and push it back
	if( !token.str.empty() )
	{
		token.line = line;
		tokenized.push_back(token);
	}

	/*
	printf("token\n");
	for(int i=0; i<tokenized.size(); i++)
		log->add_error("__ %s\n", tokenized[i].str.c_str());
	*/

	// return the complete tokenized string
	return tokenized;
}

/*	outlines all symbols from the given string. outlining means that e.g. %a+%b
		is converted to %a + %b	so the tokenizer can easily put them into their own
		token. there are some symbols	which are not outlined: everything between 
		textlimiters "" (they are constants - converts to number if possible), indices
		of variable names (they are computed later, for now they have to stick to	the
		variable name) and points which ident floating numbers (and no enviroments)	*/
void eosparser::outline_symbols(string& s)
{
	// newstring tempory
	string news;
	bool	 replaced = false;
	
	// loop through entire string
	for(int i=0; i<(int)s.size(); i++)
	{
		// if an index is opened, ignore it (=no outlining)
		if( s[i] == stx::open_idx )
		{
			
			// for nested indices, find the correct one
			int j = 0;
			int eq=-1;
			news.push_back(s[i]);
			// loop to the close index
			for(j=i+1; j<(int)s.size(); j++)
			{
				if(s[j]==stx::open_idx)
					eq--;
				if(s[j]==stx::close_idx)
					eq++;

				news.push_back(s[j]);
				// if correct index found, break
				if(eq==0)					
					break;					
			}	
			// set counter i to the last value of the index
			i=j;			
		} else
		// if a textmessage occurs, ignore it (=no outlining, constant)
		if( s[i] == stx::textlimit )
		{
			string temps;
			for(int j=i+1; j<(int)s.size(); j++)
			{
				// if end is found..
				if(s[j]==stx::textlimit)
				{
					// if the constant string is a number, if yes,
					// add it without the textlimiters
					if(is_number(temps))					
						news.append(temps);
					// if no number, add the text with text limiters
					else
					{
						news.push_back(stx::textlimit);
						news.append(temps);
						news.push_back(stx::textlimit);
					}
					// set counter to ending position
					i=j;
					break;
				}
				else
				// else if no end, pushback character
					temps.push_back(s[j]);
			}	
		} else
		// if a point is found, i could be a point defining an enviroment, or
		// a point of a floating number, if its a float, do not outline!
		if( s[i] == '.' )
		{
			// check if value before and after the point are numbers, if yes, its
			// definately a floating number
			if(((s[i-1]>='0')&&(s[i-1]<='9'))&&((s[i+1]>='0')&&(s[i+1]<='9')))
				news.push_back(stx::point);
			// if not, it defines an enviroment => outline!
			else
			{
				news.push_back(' ');
				news.push_back('.');
				news.push_back(' ');
			}

		} else
		// else if no condition above fits, we loop through an array of all
		// symbols and outline them (this is done with each character)
		{
			replaced = false;	// did we replaced a character?
			// loop through all symbols
			for(int j=0; j<stx::count; j++)
			{
				// if found, outline.
				if( s[i] == stx_symb_v[j] )
				{
					news.push_back(' ');
					news.push_back(stx_symb_v[j]);
					news.push_back(' ');
					replaced = true;
					break;
				} else
				if( s[i] == 10 )
				{
					news.push_back(' ');
					news.push_back(10);
					news.push_back(' ');
					replaced = true;
					break;
				}
			}

			// if nothing found, push back character (then its a char of a name or sth)
			if( !replaced )
				news.push_back(s[i]);
		}
	}

	// copy newstring to s (reference param)
	s=news;
}

/* this methods accepts a token as parameter and "groups out" the objects. this means,
	 when a keyword "object" is found, the end of the body is searched and then all tokens
	 which belongs to that object are stored in another token array. each object is then 
	 passed to the group_functions method which groups all functions inside an object,
	 including the "self" function (global). before and after that, the tokens are looped
	 several times to convert some syntax to internal mode and throw some warnings (as far
	 as its possible at this time) 																												*/
vector<object> eosparser::group_objects(tokens tok)
{
	vector< tokens > pattern;			// the main pattern where tokens are stored
	vector< toka > subpattern;	// the current sub pattern (each object)
	vector< toka > globalenv;		// pattern for global enviroment
	int i = 0;
	int j=0;
	// loop through the tokens passed to the method
	for(i=0; i<(int)tok.size(); i++)
	{
		j=i;
		// if keyword object found
		if( tok[j].str == KEYW_OBJ )
		{
			// find out correct end of object
			int		eq	= 0;
			bool	fin = false;

			// add tokens to subpattern
			subpattern.clear();
			while( !fin )
			{
				if( tok[j].str == SYM_OBL )
				{
					eq++;
					if( eq == 0 ) fin = true;
				} else if( tok[j].str == SYM_CBL )
				{
					eq--;
					if( eq == 0 ) fin = true;
				}
				subpattern.push_back(tok[j]);
				++j;
			}
			// end reached, if not empty, push back subpattern
			i=j-1;
			if( !subpattern.empty() ) pattern.push_back( subpattern );
			subpattern.clear();

		} else
		{
			// if no object found, this code belongs to genvi
			globalenv.push_back(tok[i]);
		}
	}

	// finally, push back global enviroment 
	// (important that this is the last func!)
	pattern.push_back(globalenv);

	find_errors(pattern);

	// now all objects has been grouped and tested on syntax errors, we
	// now loop through the objects, and pass them to the group_function
	// method. there, all functions will be grouped again and passed to
	// objs, a token-token-array.
	vector<object>	objs;
	for(i=0; i<(int)pattern.size(); i++)
		group_functions(pattern[i], objs);

	// after the functions are grouped, prepare them for the analyser.
	// therefore we loop through all functions and put functions in 
	// extra brackets (needed to correct calculations)
	for(i=0; i<(int)objs.size(); i++)
	{
		for(j=0; j<(int)objs[i].functions.size(); j++)
		{
			for(int k=0; k<(int)objs[i].functions[j].size(); k++)
			{
				// if a function begines, ignore it (they keep their commas)
				if( objs[i].functions[j][k].str == KEYW_FUNC)
				{
					// loop to the end of the function declaration
					for(int l=k; l<(int)objs[i].functions[j].size(); l++)
						if( objs[i].functions[j][l].str == SYM_OBL )
						{
							k=l+1;
							break;
						}
				} else 

				// put functions in brackets!
				if( objs[i].functions[j][k].str == SYM_OBR )
				{
					// check if we need to put that in brackets 
					// if no constant before the open bracket, its a normal calculation
					// bracket and no param bracket, so we dont need to nest that
					bool donest=true;
					if(k>0)
					{
						// if one of them fits, we dont need to nest that, 
						// beause then its no function
						if(( objs[i].functions[j][k-1].str    == SYM_END) ||
							( objs[i].functions[j][k-1].str    == SYM_OBL) ||
							( objs[i].functions[j][k-1].str    == SYM_CBL) ||
							( objs[i].functions[j][k-1].str    == SYM_OBR) ||
							( objs[i].functions[j][k-1].str    == SYM_CBR) ||
							( objs[i].functions[j][k-1].str    == SYM_COMMA ) ||
							( objs[i].functions[j][k-1].str    == SYM_DPOINT) ||
							( objs[i].functions[j][k-1].str    == SYM_ADD) ||
							( objs[i].functions[j][k-1].str    == SYM_SET) ||
							( objs[i].functions[j][k-1].str    == SYM_SUB) ||
							( objs[i].functions[j][k-1].str    == SYM_MUL) ||
							( objs[i].functions[j][k-1].str    == SYM_DIV) ||
							( objs[i].functions[j][k-1].str    == SYM_CAT) ||
							( objs[i].functions[j][k-1].str    == SYM_SMALL) ||
							( objs[i].functions[j][k-1].str    == SYM_BIG) ||
							( objs[i].functions[j][k-1].str    == KEYW_IF ) ||
							( objs[i].functions[j][k-1].str    == KEYW_WHILE ) ||
							( objs[i].functions[j][k-1].str    == KEYW_FOR ) ||
							( objs[i].functions[j][k-1].str    == KEYW_TO ) ||
							( objs[i].functions[j][k-1].str    == KEYW_DTO ) ||
							( objs[i].functions[j][k-1].str    == KEYW_AND ) ||
							( objs[i].functions[j][k-1].str    == KEYW_OR ))
							{								
	              donest=false;              
							}			
						
					}
						

					// if we need to nest the bracket and its constant...
					if(donest)
					{
						int		eq	= 0;
						bool	fin = false;
						int		pos = k;

						// find out end of bracket and place an close bracket there
						while( !fin )
						{
							if(pos>=(int)objs[i].functions[j].size()) break;

							if( objs[i].functions[j][pos].str == SYM_OBR )
							{
								eq++;
								if( eq == 0 ) fin = true;
							} else if( objs[i].functions[j][pos].str == SYM_CBR )
							{
								eq--;
								if( eq == 0 ) fin = true;
							}
							pos++;
						}
						// insert the bracket
						objs[i].functions[j].insert( objs[i].functions[j].begin()+pos, toka(SYM_CBR,
							objs[i].functions[j][pos].file,objs[i].functions[j][pos].line));
			
						// now find the start of the bracket (this is: constant+bracket). points
						// are ignored (could be enviroment)
						bool	placed=false;
						int h=k-1;
						while(!placed)
						{
							// loop through delimiters
							if((( objs[i].functions[j][h].str    == SYM_OBL) ||
							( objs[i].functions[j][h].str    == SYM_CBL) ||
							( objs[i].functions[j][h].str    == SYM_END) ||
							( objs[i].functions[j][h].str    == SYM_OBR) ||
							( objs[i].functions[j][h].str    == SYM_CBR) ||
							( objs[i].functions[j][h].str    == SYM_COMMA ) ||
							( objs[i].functions[j][h].str    == SYM_DPOINT) ||
							( objs[i].functions[j][h].str    == SYM_ADD) ||
							( objs[i].functions[j][h].str    == SYM_SET) ||
							( objs[i].functions[j][h].str    == SYM_SUB) ||
							( objs[i].functions[j][h].str    == SYM_MUL) ||
							(	objs[i].functions[j][h].str    == SYM_DIV) ||
							( objs[i].functions[j][h].str    == SYM_CAT) ||
							( objs[i].functions[j][h].str    == SYM_SMALL) ||
							( objs[i].functions[j][h].str    == SYM_BIG))&&
								(objs[i].functions[j][h].str   !=SYM_POINT))
							{
								// if found, insert open bracket and leave!
								objs[i].functions[j].insert( objs[i].functions[j].begin()+(h+1), toka(SYM_OBR,
									objs[i].functions[j][h].file, objs[i].functions[j][h].line));		
								k++;
								placed=true;
								break;
							}	

							// these keyw checks are necessary for loops (when using functions
							// for iterators, they act like delimiters)
							if(( objs[i].functions[j][h].str    == KEYW_IF ) ||
								 ( objs[i].functions[j][h].str    == KEYW_WHILE ) ||
								 ( objs[i].functions[j][h].str    == KEYW_FOR ) ||
								 ( objs[i].functions[j][h].str    == KEYW_TO ) ||
								 ( objs[i].functions[j][h].str    == KEYW_DTO ) ||
								 ( objs[i].functions[j][h].str    == KEYW_AND ) ||
								 ( objs[i].functions[j][h].str    == KEYW_OR ))
							{
								// insert bracket and leave, same as above
								objs[i].functions[j].insert( objs[i].functions[j].begin()+(h+1), toka(SYM_OBR,
									objs[i].functions[j][h].file, objs[i].functions[j][h].line));		
								k++;
								placed=true;
								break;
							}
														
							// if we are still in range, continue, else leave
							if(h>0) h--; else	break;
						}
						
						// if we ran out of range, we need to insert the open bracket to the start
						if(!placed) objs[i].functions[j].insert( objs[i].functions[j].begin(), toka(SYM_OBR,
							objs[i].functions[j][0].file , objs[i].functions[j][0].line ));
					}
				}
			} 
		}
	}

	// this is again a loop for preparing the token list. we need an extra loop here 
	// so that open brackets get not confused with replacement of commas. here we just
	// replace the keyw "this" with the global "$this" (more user-friendly) and if a 
	// variable uses the neg. operator SYM_SUB in no calc context, we put that in brackets
	// and insert "0" infront of that value (so the analyser read: sub(0-var) for -var)	
	for(i=0; i<(int)objs.size(); i++)
	{
		for(j=0; j<(int)objs[i].functions.size(); j++)
		{
			for(int k=0; k<(int)objs[i].functions[j].size(); k++)
			{
				/*
				// if comma found, replace , with ")(" ( needed for correct arg push by analyser)
				// since commas only appear in brackets, this is valid
				if( objs[i].functions[j][k].str[0] == '"' )
				{
					k++;
					while(objs[i].functions[j][k].str[0]!='"')
						k++;
					k++;
				}	
				*/

				if( objs[i].functions[j][k].str == SYM_COMMA )
				{
					objs[i].functions[j].erase( objs[i].functions[j].begin()+k );
					objs[i].functions[j].insert( objs[i].functions[j].begin()+k, toka(SYM_OBR,
						objs[i].functions[j][k].file,objs[i].functions[j][k].line));
					objs[i].functions[j].insert( objs[i].functions[j].begin()+k, toka(SYM_CBR,
						objs[i].functions[j][k].file,objs[i].functions[j][k].line));
					k++;
				}
				// if this keyw found, replace with global name
				else if (objs[i].functions[j][k].str == "this")
				{
					objs[i].functions[j][k].str = "$this";
				}
				// if neg. "-" found in no calc context, insert 0 (no calc
				// context means that another calc. symbols is used before)
				// and put that calc in brackets
				else if (objs[i].functions[j][k].str == SYM_SUB)
				{
					// find symbols, loop keyw count too!
					for(int g=k-1; g>=0; g--)
					{
            if((objs[i].functions[j][g].str == SYM_ADD) ||
							(objs[i].functions[j][g].str == SYM_SUB) ||
							(objs[i].functions[j][g].str == SYM_SET) ||
							(objs[i].functions[j][g].str == SYM_MUL) ||
							(objs[i].functions[j][g].str == SYM_DIV) ||
							(objs[i].functions[j][g].str == SYM_CAT) ||
							(objs[i].functions[j][g].str == SYM_SMALL) ||
							(objs[i].functions[j][g].str == SYM_BIG) ||
							(objs[i].functions[j][g].str == KEYW_IF ) ||
							(objs[i].functions[j][g].str == KEYW_WHILE ) ||
							(objs[i].functions[j][g].str == KEYW_FOR ) ||
							(objs[i].functions[j][g].str == KEYW_TO ) ||
							(objs[i].functions[j][g].str == KEYW_DTO ) ||
							(objs[i].functions[j][g].str == KEYW_AND ) ||
							(objs[i].functions[j][g].str == KEYW_OR ))
						{
							// if it starts with an bracket, we need to find the end
							if(objs[i].functions[j][k+1].str==SYM_OBR)
							{
								int		eq	= -1;
								bool	fin = false;
								int		pos = k+2;

								// find out end of bracket and place an close bracket there
								while( !fin )
								{
									if(pos>=(int)objs[i].functions[j].size()) break;
		
									if( objs[i].functions[j][pos].str == SYM_OBR )
									{
										eq--;
										if( eq == 0 ) fin = true;
									} else if( objs[i].functions[j][pos].str == SYM_CBR )
									{	
										eq++;
										if( eq == 0 ) fin = true;
									}	
									pos++;
								}

								
								objs[i].functions[j].insert( objs[i].functions[j].begin()+pos, toka(SYM_CBR,
									objs[i].functions[j][pos].file,objs[i].functions[j][pos].line));
								objs[i].functions[j].insert( objs[i].functions[j].begin()+k, toka("0",
									objs[i].functions[j][pos].file,objs[i].functions[j][pos].line));
								objs[i].functions[j].insert( objs[i].functions[j].begin()+k, toka(SYM_OBR,
									objs[i].functions[j][pos].file,objs[i].functions[j][pos].line));
							} else
							// else if no bracket, we simply place the 0 and brackets
							{
								objs[i].functions[j].insert( objs[i].functions[j].begin()+k+2, toka(SYM_CBR,
									objs[i].functions[j][k].file,objs[i].functions[j][k].line));
								objs[i].functions[j].insert( objs[i].functions[j].begin()+k, toka("0",
									objs[i].functions[j][k].file,objs[i].functions[j][k].line));
								objs[i].functions[j].insert( objs[i].functions[j].begin()+k, toka(SYM_OBR,
									objs[i].functions[j][k].file,objs[i].functions[j][k].line));
								
							}
						}
						// when another bracket comes up, we need to break!
						else if((objs[i].functions[j][g].str != SYM_CBR) &&
										(objs[i].functions[j][g].str != SYM_OBR))
						{
              break;				
						} 
					}
				
				}
			}
		}
	}
	
    
	/*
	// PRINT
	for(i=0; i<(int)objs.size(); i++)
	{
		log->add_error("new obj:");
		for(j=0; j<(int)objs[i].functions.size(); j++)
		{
			log->add_error("new func:");
			for(int k=0; k<(int)objs[i].functions[j].size(); k++)
			log->add_error("%s ", objs[i].functions[j][k].str.c_str());
		}
	}
	*/
	
	return objs;
}

// this method is called from group_objects and simply groups all
// functions out of the object token list. the result is pushed back
// to o, a reference to the entire object-function-array.
void eosparser::group_functions(tokens& tok, vector<object>& o)
{
	vector< tokens >	pattern;		// main pattern
	vector< toka >	subpattern;	// current subpattern
	vector< toka >	globalenv;	// globalenv (self)


	pattern.clear();
	subpattern.clear();
	globalenv.clear();

	int j=0;
	// loop through the token list
	for(int i=0; i<(int)tok.size(); i++)
	{
		j=i;
		// if "function" keyw found, group them!
		if( tok[j].str == KEYW_FUNC )
		{
			int		eq	= 0;
			bool	fin = false;
			// find out correct and of function body
			while( !fin )
			{
				if( tok[j].str == SYM_OBL )
				{
					eq++;
					if( eq == 0 ) fin = true;
				} else if( tok[j].str == SYM_CBL )
				{
					eq--;
					if( eq == 0 ) fin = true;
				}
				subpattern.push_back(tok[j]);
				++j;
			}
			// end reached, push back tokens which belong to that functions
			i=j-1;
			if( !subpattern.empty() ) pattern.push_back( subpattern );
			subpattern.clear();
		} else
		// else, the code belongs to the global enviroment
		{
			globalenv.push_back(tok[i]);
		}
	}

	pattern.push_back(globalenv);
	
	// create object structure which can be pushed back
	// to the main pattern
	object newentry;
	newentry.functions = pattern;
	o.push_back(newentry);
}

/* loops through the pattern and searches several parser errors */
void eosparser::find_errors(vector<tokens>& pattern)
{
	// find errors
	for(int i=0; i<(int)pattern.size(); i++)
	{
		int br = 0;
		int bl = 0;
		int id = 0;
		int tx = 0;

		int j = 0;
		// bracket/block/ident pairs
		for(j=0; j<(int)pattern[i].size(); j++)
		{
			if(pattern[i][j].str == SYM_OBR) br++; else
			if(pattern[i][j].str == SYM_CBR) br--; else
			if(pattern[i][j].str == SYM_OBL) bl++; else
			if(pattern[i][j].str == SYM_CBL) bl--; else
			if(pattern[i][j].str == SYM_OID) id++; else
			if(pattern[i][j].str == SYM_CID) id--; 
		}

		if(br!=0)
		{
			if(br<0)
				log->add_error("file:%s line:%d '(' missing!", pattern[i][j-1].file.c_str(), pattern[i][j-1].line);

			else
				log->add_error("file:%s line:%d ')' missing!", pattern[i][j-1].file.c_str(), pattern[i][j-1].line);
		}
		if(bl!=0)
		{
			if(bl<0)
				log->add_error("file:%s line:%d '{' missing!", pattern[i][j-1].file.c_str(), pattern[i][j-1].line);
			else
				log->add_error("file:%s line:%d '}' missing!", pattern[i][j-1].file.c_str(), pattern[i][j-1].line);
		}
		if(id!=0)
		{
			if(id<0)
				log->add_error("file:%s line:%d '[' missing!", pattern[i][j-1].file.c_str(), pattern[i][j-1].line);
			else
				log->add_error("file:%s line:%d ']' missing!", pattern[i][j-1].file.c_str(), pattern[i][j-1].line);
		}

		// check for keyw in if statement
		for(j=0; j<(int)pattern[i].size(); j++)
		{
			if(pattern[i][j].str == KEYW_IF)
			{
				int cnt=j;
				while( pattern[i][cnt].str != KEYW_THEN )
				{
					cnt++;
					if( cnt >= (int)pattern[i].size() )
					{
						log->add_error("file:%s line:%d 'then' missing!", pattern[i][j].file.c_str(), pattern[i][j].line); 
						break;
					}
				}

				// then must be followed by an open block
				if( pattern[i][cnt].str == KEYW_THEN )
					if( pattern[i][cnt+1].str != SYM_OBL )
						log->add_error("file:%s line:%d block missing after if-statement!", pattern[i][cnt].file.c_str(), pattern[i][cnt].line);
			}
			// check for keyw in for loop
			else if(pattern[i][j].str == KEYW_FOR)
			{
				bool to = false;	// KEYW_TO
				bool wi = false;  // KEYW_WITH
				int cnt=j;
				while( pattern[i][cnt].str != KEYW_DO )
				{
					if((pattern[i][cnt].str == KEYW_TO)||(pattern[i][cnt].str == KEYW_DTO)) to = true;
					else if(pattern[i][cnt].str == KEYW_WITH) wi = true;

					cnt++;
					if( cnt >= (int)pattern[i].size() )
					{
						log->add_error("file:%s line:%d 'do' missing after for-loop!", pattern[i][j].file.c_str(), pattern[i][j].line);
						break;
					}
				}

				// do must be followed by an open block
				if( pattern[i][cnt].str == KEYW_DO )
					if( pattern[i][cnt+1].str != SYM_OBL )
						log->add_error("file:%s line:%d block missing after for-loop!", pattern[i][cnt].file.c_str(), pattern[i][cnt].line);

				if(!to) log->add_error("file:%s line:%d 'to' or 'downto' missing in for-loop!", pattern[i][j].file.c_str(), pattern[i][j].line);
				if(!wi) log->add_error("file:%s line:%d 'with' missing in for-loop!", pattern[i][j].file.c_str(), pattern[i][j].line);
			}
			// check for keyw in while loop
			else if(pattern[i][j].str == KEYW_WHILE)
			{
				int cnt=j;
				while( pattern[i][cnt].str != KEYW_DO )
				{
					cnt++;
					if( cnt >= (int)pattern[i].size() )
					{
						log->add_error("file:%s line:%d 'do' missing after while-loop!", pattern[i][j].file.c_str(), pattern[i][j].line); 
						break;
					}
				}

				// dp must be followed by an open block
				if( pattern[i][cnt].str == KEYW_DO )
					if( pattern[i][cnt+1].str != SYM_OBL )
						log->add_error("file:%s line:%d block {} missing after while-loop!", pattern[i][cnt].file.c_str(), pattern[i][cnt].line);
			}
			// SYM_NEW must be followed by an open bracket!
			else if(pattern[i][j].str == SYM_NEW)
			{
				if(pattern[i][j+1].str != SYM_OBR)
					log->add_error("file:%s line:%d bracket () missing after 'new' keyword", pattern[i][j].file.c_str(), pattern[i][j].line);
			}
			// SYM_RET must be followed by an open bracket!
			else if(pattern[i][j].str == SYM_RET)
			{
				if((pattern[i][j+1].str != SYM_OBR)&&(pattern[i][j+1].str != SYM_END))
					log->add_error("file:%s line:%d block () or end-statement ';' missing after 'return' keyword", pattern[i][j].file.c_str(), pattern[i][j].line);
			}
			// SYM_FUNC must be followed by an open bracket!
			else if(pattern[i][j].str == SYM_FUNC)
			{
				if(pattern[i][j+1].str != SYM_OBR)
					log->add_error("file:%s line:%d brackets () missing after 'func' keyword!", pattern[i][j].file.c_str(), pattern[i][j].line);
			}
			// SYM_METH must be followed by an open bracket!
			else if(pattern[i][j].str == SYM_METH)
			{
				if(pattern[i][j+1].str != SYM_OBR)
					log->add_error("file:%s line:%d brackets () missing after 'method' keyword!", pattern[i][j].file.c_str(), pattern[i][j].line);
			}
			// if an open bracket begins, check if cmp conditions are within that bracket,
			// if yes, its an error! (this is not allowed, the analyser would corrupt!)
			else if(pattern[i][j].str == SYM_OBR)
			{
				int cnt=j;
				// loop through bracket
				while( pattern[i][cnt].str != SYM_CBR )
				{
					if(( pattern[i][cnt].str == SYM_SMALL) ||
						( pattern[i][cnt].str == SYM_BIG) ||
						( pattern[i][cnt].str == SYM_EXCL))
					{
						log->add_error("file:%s line:%d condition in brackets () not allowed!", pattern[i][cnt].file.c_str(), pattern[i][cnt].line);
					}
					else if(( pattern[i][cnt].str == SYM_SET)&&( pattern[i][cnt+1].str == SYM_SET))		{
						log->add_error("file:%s line:%d condition in brackets () not allowed!", pattern[i][cnt].file.c_str(), pattern[i][cnt].line);
					}		

					cnt++;          
				}
			}
			else if(pattern[i][j].str == SYM_CBL)
			{
				if(j<(int)pattern[i].size()-1)
					if(pattern[i][j+1].str == SYM_END)
						log->add_error("file:%s line:%d closing block '}' followed by end-statement ';'!", pattern[i][j].file.c_str(), pattern[i][j].line);
				if(j>0)
					if((pattern[i][j-1].str != SYM_END)&&(pattern[i][j-1].str != SYM_CBL))
						log->add_error("file:%s line:%d missing end-statement ';'!", pattern[i][j].file.c_str(), pattern[i][j].line);
			}

		}

	}
}



