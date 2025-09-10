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

#ifndef EOSUTIL_H
#define EOSUTIL_H

// convert string to T
template <class T>
T	_str2(string s)
{
	std::strstream string;
	string.unsetf(std::ios::skipws);
	string<<s;
	T res;
	string>>res;
	return res;
}

// convert T to string
template <class T>
string	_2str(T value)
{
	std::strstream str;		
	str.unsetf(std::ios::skipws);
	str<<value;
	string res;
	str>>res;
	return res;
}

// find a string in token, with start end position
int	find_str_v(string s, tokens v, int start, int end);
// find a char (first-char) in token, with start end pos
int	find_fchar_v(char s, tokens v, int start, int end);
// find a char in token, reversed, from back to front
int	find_fchar_v_rev(char s, tokens v, int start, int end);

// remove variable symbols from string ($%)
string	var_to_str(string s);
// remove index [] from name
string	find_varname(string s);
// check if string is a number
bool		is_number(string s);
// return index of name 
string	find_index(string s);
// return next index of name and remove it (for nested indices)
void		next_index(string& s);
// find condition in string
string	find_condition(string s);
// find condition in string, reversed, from back to front
string	find_condition_rev(string s);
// format string and return it.
string	eosformat(const char *fmt, ...);


#endif