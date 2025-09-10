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

/* the constructor opens all files */
eoslog::eoslog()
{
	logFile = fopen(LOG_FILE, "wb");
	errFile = fopen(ERR_FILE, "wb");

	log->add("init log");
}

/* the destructor closes all files */
eoslog::~eoslog()
{
	log->add("close log");
	fclose(logFile);
	fclose(errFile);	
}

/* add simple message to log */
void	eoslog::add(const char* msg, ...)
{
	char		message[256];
	char		complete[256];
	va_list		ap;					
	
	
	va_start(ap, msg);				
		vsprintf(message, msg, ap);
	va_end(ap);	
	
	// if empty message...
	if(msg != "")
	{
		sprintf(complete,"- %s %c%c",message, 13,10);
	}
	else
	{		
		sprintf(complete,"- %c%c",13,10);
	}

	// write to file and flush (for errors)
	fwrite(complete, sizeof(char), strlen(complete), logFile);
	fflush(logFile);
}

/* add simple message to errorlog */
void	eoslog::add_error(const char* msg, ...)
{
	char		message[256];
	char		complete[256];
	va_list		ap;					
	
	
	va_start(ap, msg);				
		vsprintf(message, msg, ap);
	va_end(ap);	
	
	if(msg != "")
	{
		// add _ERROR_ tag
		sprintf(complete,"- _ERROR_ : %s %c%c",message,13,10);
	}
	else
	{		
		sprintf(complete,"- %c%c",13,10);
	}

	// write to default file and flush (for errors)
	fwrite(complete, sizeof(char), strlen(complete), logFile);
	fflush(logFile);
	// write to error file and flush (for errors)
	fwrite(complete, sizeof(char), strlen(complete), errFile);
	fflush(errFile);
}
