/****************************************************************************************/
/*  DEFS.H                                                                              */
/*                                                                                      */
/*  Author:                                                                             */
/*  Description:                                                                        */
/*                                                                                      */
/*  The contents of this file are subject to the Jet3D Public License                   */
/*  Version 1.02 (the "License"); you may not use this file except in                   */
/*  compliance with the License. You may obtain a copy of the License at                */
/*  http://www.jet3d.com                                                                */
/*                                                                                      */
/*  Software distributed under the License is distributed on an "AS IS"                 */
/*  basis, WITHOUT WARRANTY OF ANY KIND, either express or implied.  See                */
/*  the License for the specific language governing rights and limitations              */
/*  under the License.                                                                  */
/*                                                                                      */
/*  The Original Code is Jet3D, released December 12, 1999.                             */
/*  Copyright (C) 1996-1999 Eclipse Entertainment, L.L.C. All Rights Reserved           */
/*                                                                                      */
/****************************************************************************************/
#pragma once

// This module is a catch-all for defines

#ifndef DEFS_H
#define DEFS_H

#define HALFHANDLESIZE	(3)
#define HANDLESIZE		(6)


typedef enum
{
	KIND_INVALID,
	KIND_BRUSH=1,
	KIND_MODEL=2,
	KIND_ENTITY=4,
	KIND_LIGHT=8,
	KIND_TERRAIN=16,
	KIND_GROUP = 32,
	KIND_CAMERA = 64,
	KIND_USEROBJ = 128,
	KIND_CLASS = 256,
	KIND_LAST,
} OBJECT_KIND ;

typedef enum
{
	OBJECT_UPDATE_MANUEL,
	OBJECT_UPDATE_CHANGE,
	OBJECT_UPDATE_REALTIME
} OBJECT_UPDATE;

#define OBJECT_KINDALL (KIND_BRUSH|KIND_MODEL|KIND_ENTITY|KIND_LIGHT|KIND_TERRAIN|KIND_CAMERA|KIND_USEROBJ)

typedef enum
{
	BRUSH_INVALID,
	BRUSH_BOX,
	BRUSH_SPHERE,
	BRUSH_CYLINDER,
	BRUSH_CONE,
	BRUSH_SHEET,
	BRUSH_STAIRCASE,
	BRUSH_PREMADE,
	BRUSH_LOFTED,
	BRUSH_ARCH,
	BRUSH_LAST
} BRUSH_KIND ;

#define OBJECT_DEFAULT_NAME_RESID_START 2000 

typedef enum
{
	LEVEL_SELNONE,
	LEVEL_SELONEBRUSH		= (1<<0),
	LEVEL_SELBRUSHES		= (1<<1),
	LEVEL_SELONELIGHT		= (1<<2),
	LEVEL_SELLIGHTS 		= (1<<3),
	LEVEL_SELONEENTITY		= (1<<4),
	LEVEL_SELENTITIES		= (1<<5),
	LEVEL_SELONEMODEL		= (1<<6),
	LEVEL_SELMODELS			= (1<<7),
	LEVEL_SELENTITYTYPE		= (1<<8), //Selected entites are all same type
	LEVEL_SELTEMPLATE		= (1<<9),
	LEVEL_SELMANY			= (1<<10),
	LEVEL_SELONECAMERA		= (1<<11),
	LEVEL_SELCAMERAS		= (1<<12),
	LEVEL_SELONEOBJECT		= (1<<13),
	LEVEL_SELOBJECTS		= (1<<14),
	LEVEL_SELONECLASS		= (1<<15),
	LEVEL_SELCLASS			= (1<<16)
} LEVEL_SEL ;

typedef enum
{
	Ortho_Axis_X,	// These correspond to jeVec3d's Get/Set Element
	Ortho_Axis_Y,
	Ortho_Axis_Z,
	Ortho_Axis_Last	// Invalid
} ORTHO_AXIS ;

typedef enum 
{
	Select_None,	// The order of these cannot change
	Select_TopLeft,
	Select_TopRight,
	Select_BottomLeft,
	Select_BottomRight,
	Select_Left,
	Select_Right,
	Select_Top,
	Select_Bottom,
	Select_Center,
	Select_Last
} SELECT_HANDLE ;

#define IS_CORNER_HANDLE( h ) ( h >= Select_TopLeft && h <= Select_BottomRight )
#define IS_EDGE_HANDLE( h )   ( h >= Select_Left && h <= Select_Bottom )

typedef enum
{
	SubSelect_None,
	SubSelect_Hilite = (1<<1),
	SubSelect_Move   = (1<<2),
	SubSelect_Rotate = (1<<3),
} SUBSELECT_FLAGS;

#define AllSubSelect ( SubSelect_Hilite | SubSelect_Move | SubSelect_Rotate )
typedef enum
{
	MODE_NONE,
	MODE_POINTER_BB,	// Bounding Box Adjust
	MODE_POINTER_RS,	// Rotate-Shear
	MODE_POINTER_VM,	// Vertex Manipulation
	MODE_POINTER_FM,	// Face Manipulation
	MODE_POINTER_CUBE,	// Place Cube
	MODE_POINTER_CYLINDER,		// Place Cylinder
	MODE_POINTER_SPHERE,		// Place Sphere
	MODE_POINTER_SHEET,			// Place Sheet
	MODE_POINTER_LIGHT,			// Place Light
	MODE_POINTER_CAMERA,		// Place Camera
	MODE_POINTER_USEROBJ,		// Place UserObj
	MODE_POINTER_ARCH,
	MODE_LAST
} MODE ;


#endif // Prevent multiple inclusion
/* EOF: Defs.h */
