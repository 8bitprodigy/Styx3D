/*
    FSOps.h
    Author: Chris DeBoy
    Description: Platform-agnostic reimplementation of filesystem interface originally defined in FSDOS.h and FSDOS.c

    This code is released into the public domain or licensed under the terms of the 0BSD license
*/
#ifndef FSOPS_H
#define FSOPS_H

#include "VFile_private.h"


const geVFile_SystemAPIs *GENESISCC FSDos_GetAPIs(void);
    
#endif /* FSOPS_H */
