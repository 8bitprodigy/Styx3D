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

#ifndef EOSSYNTAX_H
#define EOSSYNTAX_H

/* syntax table for parser */
namespace stx
{
	int		count					= 17;
	char	end_statement	=	';';
	char	open_block		=	'{';
	char	close_block		=	'}';
	char	open_bracket	=	'(';
	char	close_bracket	=	')';
	char	point					= '.';
	char	comma					= ',';
	char	dpoint				= ':';
	char	add						= '+';
	char	sub						= '-';
	char	set						= '=';
	char	mul						= '*';
	char	div						= '/';
	char	cat						= '&';
	char	smaller				= '<';
	char	bigger				= '>';
	char	textlimit			= '"';

	char	open_idx			= '[';
	char	close_idx			= ']';
	char	local_ident		= '%';
	char	global_ident	= '$';
}



#endif