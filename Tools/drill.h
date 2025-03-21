//**************************************************************************
//  drill.h
//     header for some example code to demonstrate how to build an actor. see drill.c

#ifndef __DRILL_H__
#define __DRILL_H__

#include "basetype.h"
#include "Vec3d.h"
#include "XForm3d.h"

#ifdef __cplusplus
extern "C" {
#endif

geBoolean Drill_Create(const char *Name, geFloat sizex,geFloat sizey, geFloat sizez );

#ifdef __cplusplus
}
#endif


#endif