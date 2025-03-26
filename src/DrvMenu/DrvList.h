/****************************************************************************************/
/*  DrvList.h                                                                           */
/*                                                                                      */
/*  Author: Mike Sandige                                                                */
/*  Description:  Dialog control logic for driver/mode selection dialog                 */
/*                 (rewritten from previous version)                                    */
/*  Copyright (c) 1999 WildTangent, Inc.; All rights reserved.               */
/*                                                                                      */
/*  See the accompanying file LICENSE.TXT for terms on the use of this library.         */
/*  This library is distributed in the hope that it will be useful but WITHOUT          */
/*  ANY WARRANTY OF ANY KIND and without any implied warranty of MERCHANTABILITY        */
/*  or FITNESS FOR ANY PURPOSE.  Refer to LICENSE.TXT for more details.                 */
/*                                                                                      */
/****************************************************************************************/

#ifndef DRVLIST_H
#define DRVLIST_H

#include <SDL3/SDL.h>

#include "ModeList.h"

#ifndef _WIN32
    typedef void* HANDLE;
    typedef void* HWND;
#endif

#ifdef __cplusplus
extern "C" {
#endif

geBoolean DrvList_PickDriver(SDL_Window *hwndParent, 
		ModeList *List, int ListLength, int *ListSelection);

#ifdef __cplusplus
}
#endif

#endif /* DRVLIST_H */

