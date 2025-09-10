/* RogerWilco Easy-Bake integration
parts of this code is Copyright HearMe, Inc. as described below
*/

/* THE FOLLOWING APPLIES TO PORTIONS OF THE FOLLOWING CODE:

	This code is Copyright (c) 1999 HearMe, Inc.
	It is provided on an as-is basis to illustrate integration
	of Roger Wilco(tm) using the command-line options.

	License is granted to copy and integrate this code with 
	ENTERTAINMENT SOFTWARE TITLES LICENSED TO INTEGRATE 
	WITH ROGER WILCO.  No other use of the software code is permitted.
*/

#include <windows.h>
#include "rogerwilco.h"
#include "Jet.h"

/*
 * Send a command string to Roger Wilco.
 *
 * Command syntax summary:
 *      arguments in [square brackets] are optional
 *      arg1|arg2|arg3  : alternatives
 *
 * Mark1 and later:
 *  /create [name|DEFAULT [password]]
 *  /join machine [name|DEFAULT [password]]
 *  /leave [channelname]
 *  /file filename.rwc
 *
 *    Note: for all channel names, use "default" to refer to the default,
 *     un-named channel.
 *
 * Mark1a and later:
 *  /callsign usercallsign
 *  /netspeed N				N between 2 (slow modem) and 50 (T1 connection)
 *  /key KEY				E.G. SPACE or CONTROL, press-to-talk key
 *  /clicks on|off|yes|no
 *  /volume N				N between 1 (almost silent) and 255
 *  /priority high|low
 *  /join1 machine[/name [password]]   e.g. /join1 www.myhost.com/channel sesame
 *
 */
jeBoolean SendCommandToRogerWilco( const char* cmd ) //for internal use
{
	HKEY		hKey = 0;
	char		commandLine[MAX_PATH];
	DWORD		len = MAX_PATH;
	UINT		r = 0;

	if( RegOpenKeyEx( HKEY_LOCAL_MACHINE,
		"Software\\Microsoft\\Windows\\CurrentVersion\\App Paths\\roger.exe",
		0, KEY_QUERY_VALUE, &hKey ) != ERROR_SUCCESS )
	{
		return FALSE;
	}
	if( RegQueryValueEx( hKey, NULL, NULL, NULL, commandLine, &len ) != ERROR_SUCCESS )
	{
		RegCloseKey(hKey);
		return FALSE;
	}
	RegCloseKey(hKey);

	strcat( commandLine, " " );
	strcat( commandLine, cmd );

	/*
	 * Replace the SW_SHOWMINNOACTIVE if you want Roger Wilco
	 * to pop to the front; SHOWMINNOACTIVE will keep it hidden
	 * (minimized and not active)
	 */
	r = WinExec( commandLine, SW_SHOWMINNOACTIVE );
	if( r < 31 ) return FALSE;	// Error of some sort.

	return JE_TRUE;	// Sent OK.
}

jeBoolean jeWilco_SendCommand( const char* cmd ) //for API use
{
	HKEY		hKey = 0;
	char		commandLine[MAX_PATH];
	DWORD		len = MAX_PATH;
	UINT		r = 0;

	if( RegOpenKeyEx( HKEY_LOCAL_MACHINE,
		"Software\\Microsoft\\Windows\\CurrentVersion\\App Paths\\roger.exe",
		0, KEY_QUERY_VALUE, &hKey ) != ERROR_SUCCESS )
	{
		return FALSE;
	}
	if( RegQueryValueEx( hKey, NULL, NULL, NULL, commandLine, &len ) != ERROR_SUCCESS )
	{
		RegCloseKey(hKey);
		return FALSE;
	}
	RegCloseKey(hKey);

	strcat( commandLine, " " );
	strcat( commandLine, cmd );

	/*
	 * Replace the SW_SHOWMINNOACTIVE if you want Roger Wilco
	 * to pop to the front; SHOWMINNOACTIVE will keep it hidden
	 * (minimized and not active)
	 */
	r = WinExec( commandLine, SW_SHOWMINNOACTIVE );
	if( r < 31 ) return FALSE;	// Error of some sort.

	return JE_TRUE;	// Sent OK.
}

jeBoolean jeWilco_CreateChannel (const char *channel_name, const char *passwd)
{
  char cmd[512];

  if (!(*channel_name))
    channel_name = "jet3d_rw_easybake_null";

  sprintf(cmd, "/create %s %s", channel_name, passwd);

  return SendCommandToRogerWilco(cmd);
}


jeBoolean jeWilco_JoinChannel(const char *host_ip, const char *channel_name, const char *passwd)
{
  char cmd[512];

  if (!stricmp(channel_name, "jet3d_rw_easybake_null"))
    channel_name = "";
  sprintf(cmd, "/join1 %s/%s %s", host_ip, channel_name, passwd);

  return SendCommandToRogerWilco(cmd);
}

jeBoolean jeWilco_LeaveChannel()
{
	char cmd[512];
	
	sprintf(cmd, "/leave");

	return SendCommandToRogerWilco(cmd);
}

jeBoolean jeWilco_TuneChannel(const char *ip_of_anyone_in_new_channel, const char *passwd) //use for tuning to a team's channel (AKA when you join a team or switch sides)
{
	char cmd[512];
	
	sprintf(cmd, "/leave");
	SendCommandToRogerWilco(cmd);

	sprintf(cmd, "");
	sprintf(cmd, "/join1 %s %s", ip_of_anyone_in_new_channel, passwd);

	return SendCommandToRogerWilco(cmd);
}

/* 
   full-screen apps that want to ensure that Roger Wilco's window never pops up
   over their initialized full-screenb graphics screen should call this prior to 
   opening their main window or switching into a full-screen mode.  
 
   This gives Roger Wilco a chance to make sure that it has opened its window and
   gotten onto the desktop, and ensures that any graphics mode switches and window opens
   that the integrating app performs will not be un-done by Roger Wilco
*/
jeBoolean jeWilco_Init()
{
  jeBoolean retval = SendCommandToRogerWilco("");
  Sleep(2000);

  return retval;
}

//special pre-set commands
jeBoolean jeWilco_SetName(char *name)
{
	jeBoolean retval;
	char cmd[512];

	sprintf(cmd, "/callsign %s", name);
	
	retval = SendCommandToRogerWilco(cmd);

	return retval;
}

jeBoolean jeWilco_SetClicks(jeBoolean click)
{
	jeBoolean retval;
	char cmd[512];

	if(click)
	{
		sprintf(cmd, "/clicks on");
		retval = SendCommandToRogerWilco(cmd);
		if(retval)
		{
			sprintf(cmd, "/clicks yes");
			retval = SendCommandToRogerWilco(cmd);
		}
		return retval;
	}
	if(!click)
	{
		sprintf(cmd, "/clicks off");
		retval = SendCommandToRogerWilco(cmd);
		if(retval)
		{
			sprintf(cmd, "/clicks no");
			retval = SendCommandToRogerWilco(cmd);
		}
		return retval;
	}
	
	
	//if we get here, we failed
	retval = JE_FALSE;
	return retval;
}

jeBoolean jeWilco_SetKey(char *key)
{
	jeBoolean retval;
	char cmd[512];

	//*key SHOULD BE CAPITALIZED!!!
	sprintf(cmd, "/key %s", key);
	
	retval = SendCommandToRogerWilco(cmd);

	return retval;
}

jeBoolean jeWilco_SetPriority(int level)
{
	jeBoolean retval;
	char cmd[512];
	char priority[5];

	if(level == 0)
	{
		sprintf(priority, "low");
	}
	else if(level == 1)
	{
		sprintf(priority, "high");
	}

	sprintf(cmd, "/priority %s", priority);
	
	retval = SendCommandToRogerWilco(cmd);

	return retval;
}

jeBoolean jeWilco_SetVolume(int n)
{
	jeBoolean retval;
	char cmd[512];

	//'n' is between 1 (almost silent) and 255 (loudest)

	sprintf(cmd, "/volume %i", n);
	
	retval = SendCommandToRogerWilco(cmd);

	return retval;
}
