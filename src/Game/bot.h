
/****************************************************************************/
/*    FILE: bot.h                                                        */
/****************************************************************************/

#ifndef BOT_H
#define BOT_H

#include "_bot.h"

#define BOT_DEBUG 1

// Bot Game Side
geBoolean Bot_Control(GenVSI *VSI, void *PlayerData, float Time);
GenVSI_CMove *Bot_AI_Control(void);

// Player Game Side
//geBoolean CheckVelocity(GenVSI *VSI, void *PlayerData, float Time);
geBoolean CheckPlayer(GenVSI *VSI, void *PlayerData);
void SetupPlayerXForm(GenVSI *VSI, void *PlayerData, float Time);

geBoolean Bot_MatchStart(GenVSI *VSI, void *PlayerData, void *ClassData, char *EntityName);
geBoolean Bot_ActorStart(GenVSI *VSI, void *PlayerData, void *ClassData, char *EntityName);


// Function
typedef geBoolean (*CONTROLp)(GenVSI *VSI, void *PlayerData, float Time);

// Call functions based on a random range value
typedef struct 
    {
    short range;
    //CONTROLp action;
	void *action;
    //geBoolean (*action)(GenVSI *, void *, float);
    } DECISION, *DECISIONp;    

// Personality structure
struct PERSONALITYstruct
    {
    DECISIONp Battle;
    DECISIONp Offense;
    DECISIONp Surprised;
    DECISIONp Evasive;
    DECISIONp LostTarget;
    DECISIONp CloseRange;
	};
    
#endif
