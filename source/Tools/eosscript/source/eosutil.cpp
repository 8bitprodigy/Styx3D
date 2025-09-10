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

// find a string in token, with start end position
int	find_str_v(string s, tokens v, int start, int end)
{
	int size=0;

	if(end!=-1) size = end;
	else				size = (int)v.size();

	for(int i=start; i<size; i++)
		if(v[i].str == s)
		{
			return i;
		} 

	return -1;
}

// find a char (first-char) in token, with start end pos
int	find_fchar_v(char s, tokens v, int start, int end)
{
	int size=0;

	if(end!=-1) size = end;
	else				size = (int)v.size();

	for(int i=start; i<size; i++)
		if(v[i].str[0] == s)
		{
			return i;
		} 

	return -1;
}

// find a char in token, reversed, from back to front
int	find_fchar_v_rev(char s, tokens v, int start, int end)
{
	for(int i=start; i>=end; i--)
		if(v[i].str[0] == s)
		{
			return i;
		} 

	return -1;
}

// remove variable symbols from string ($%)
string var_to_str(string s)
{
	string news;
	for(int i=0; i<(int)s.size(); i++)
	{
		if((s[i] != '$')&&(s[i] != '%'))
			news.push_back(s[i]);
	}

	return news;
}

// remove index [] from name
string find_varname(string s)
{
	string num="";
	for(int i=0; i<(int)s.size(); i++)
	{
		int eq=-1;
		if(s[i]=='[')
		{
			bool t=true;
			while(t)
			{
				if(s[i+1]==']')	eq++;
				if(s[i+1]=='[')	eq--;

				if(eq==0)
				{
					break;
				}
				else
					i++;

			}			
		} else
			if(s[i]!=']')	num.push_back(s[i]);
	}

	return num;
}

// return next index of name and remove it (for nested indices)
void next_index(string& s)
{
	string num="";
	for(int i=0; i<(int)s.size(); i++)
	{
		int eq=-1;
		if(s[i]=='[')
		{
			s.erase(s.begin()+i);
			bool t=true;
			while(t)
			{
				if(s[i]==']')	eq++;
				if(s[i]=='[')	eq--;

				if(eq==0)
				{
					s.erase(s.begin()+i);
					return;
				} else
					s.erase(s.begin()+i);
			}			
		} 
	}
}

// check if string is a number
bool	is_number(string s)
{

	if(s==".") return FALSE;

	for(int i=0; i<(int)s.size(); i++)
		if( ((s[i] < '0') || (s[i] > '9')) && s[i] != '.')			
			return FALSE;

	return TRUE;
}

// return index of name 
string	find_index(string s)
{
	string num="";
	for(int i=0; i<(int)s.size(); i++)
	{
		int eq=-1;
		if(s[i]=='[')
		{
			bool t=true;
			while(t)
			{
				if((i+1)<(int)s.size())
				{
					if(s[i+1]==']')	eq++;
					if(s[i+1]=='[')	eq--;

					if(eq==0)
						return num;					
					else
						num.push_back(s[++i]);
				} else
				{
					log->add_error("fatal parser error! no text constants allowed as index specifier");
					break;					
				}
			}			
		}
	}

	return "0";
}



// format string and return it.
string	eosformat(const char *fmt, ...)
{
	char		text[256];								// Holds Our String
	va_list		ap;										// Pointer To List Of Arguments

	if (fmt == NULL)									// If There's No Text
		return 0;											// Do Nothing

	va_start(ap, fmt);									// Parses The String For Variables
	    vsprintf(text, fmt, ap);						// And Converts Symbols To Actual Numbers
	va_end(ap);											// Result

	return string(text);
}

// find condition in string
string	find_condition(string s)
{
	if(s=="__big__")	return	string("JG"); else
	if(s=="__small__")	return	string("JL"); else
	if(s=="__big____set__")	return	string("JGE"); else
	if(s=="__small____set__")	return	string("JLE"); else
	if(s=="__set____set__")	return	string("JE"); else
	if(s=="__excl____set__")	return	string("JNE"); else
		return string("JMP");

}

// find condition in string, reversed, from back to front
string	find_condition_rev(string s)
{
	if(s=="__big__")	return	string("JLE"); else
	if(s=="__small__")	return	string("JGE"); else
	if(s=="__big____set__")	return	string("JL"); else
	if(s=="__small____set__")	return	string("JG"); else
	if(s=="__set____set__")	return	string("JNE"); else
	if(s=="__excl____set__")	return	string("JE"); else
		return string("JMP");

}
