#include <windows.h>
#include "Jet.h"

#ifndef _ROGER_WILCO
#define _ROGER_WILCO

//RW commands
jeBoolean	SendCommandToRogerWilco( const char* cmd );

//Engine API
jeBoolean	jeWilco_SendCommand( const char* cmd );
jeBoolean	jeWilco_CreateChannel (const char *channel_name, const char *passwd);
jeBoolean	jeWilco_JoinChannel(const char *host_ip, const char *channel_name, const char *passwd);
jeBoolean	jeWilco_LeaveChannel();
jeBoolean	jeWilco_TuneChannel(const char *ip_of_anyone_in_new_channel, const char *passwd);
jeBoolean	jeWilco_Init();

//special pre-set commands
jeBoolean	jeWilco_SetName(char *name); //set your callsign in the RW client (not really needed but just in case the game developer wants to do this...)
jeBoolean	jeWilco_SetClicks(jeBoolean click); //tell RW wether or not to use the click noise
jeBoolean	jeWilco_SetKey(char *key); //set the RW transmit key (AKA CONTROL or SPACE)
jeBoolean	jeWilco_SetPriority(int level); //CPU priority level (0 == low) (1 == high)
jeBoolean	jeWilco_SetVolume(int n); //sets RWs volume 1 (almost silent) 255 (loudest)

#endif