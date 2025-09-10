/****************************************************************************************/
/*  JETYPES.H                                                                           */
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
#ifndef JE_TYPES_H
#define JE_TYPES_H

#include "BaseType.h"
#include "Vec3d.h"

#ifdef __cplusplus
extern "C" {
#endif

//
//	Render flags for poly operations
//
#define JE_RENDER_FLAG_ALPHA				0x00000001	// Alpha in the vertices are valid
#define JE_RENDER_FLAG_SPECULAR				0x00000002	// Specular in the vertices are valid
#define JE_RENDER_FLAG_COLORKEY				0x00000004	// Texture format has color key on the poly being rendered
#define JE_RENDER_FLAG_CLAMP_UV				0x00000008	// Clamp U and V in BOTH directions
#define JE_RENDER_FLAG_COUNTER_CLOCKWISE	0x00000010	// Winding of poly will be counter-clockwise
#define JE_RENDER_FLAG_NO_ZTEST				0x00000020	// No ZTest should be performed
#define JE_RENDER_FLAG_NO_ZWRITE			0x00000040	// No ZWrites should be performed
#define JE_RENDER_FLAG_STEST				0x00000080	// Span test should be performed (if set, polys should be front to back)
#define JE_RENDER_FLAG_SWRITE				0x00000100	// Spans should be written to the sbuffer
#define JE_RENDER_FLAG_FLUSHBATCH			0x00000200	// Flushes the current batch of polys (if any), and the current poly
#define JE_RENDER_FLAG_BILINEAR_FILTER		0x00000400	// Enable bilinear filtering
#define JE_RENDER_FLAG_WIREFRAME			0x00000800	// Toggle wireframe rendering
#define JE_RENDER_FLAG_HWTRANSFORM			0x00001000	// Renderer support Hardware Transform
#define JE_RENDER_FLAG_VERTEXBUFFER         0x10000000  // Renderer use VertexBuffer

// Device Caps for the current driver
typedef struct
{
	uint32			SuggestedDefaultRenderFlags;	// What the driver suggest the DefaultRenderFlags should be
	uint32			CanChangeRenderFlags;			// RenderFlags that you can change
	// Other Device related stuff should go here (
} jeDeviceCaps;

typedef struct
{
	jeFloat	u, v;
	jeFloat	r, g, b, a;
} jeUVRGBA;

typedef struct
{
	float r, g, b, a;
} jeRGBA;

typedef struct
{
	float r, g, b;
} jeRGB;

typedef struct
{
	int32	Left;
	int32	Right;
	int32	Top;
	int32	Bottom;
} jeRect;

typedef struct
{
	jeFloat MinX,MaxX;
	jeFloat MinY,MaxY;
} jeFloatRect;

// Lit vertex
typedef struct
{
/*
	jeVec3d	Position;
	jeRGBA	Color,SpecularColor;
	float	u,v,pad1,pad2;
*/
	// FIXME:  Convert 3d X,Y,Z to jeVec3d
	float X, Y, Z, pad;								// 3d vertex
	// FIXME:  Convert r,g,b,a to JE_RGBA
	float r, g, b, a;								// color
	float u, v,pad1,pad2;							// Uv's
	float sr, sg, sb, pad3;							// specular color
} jeLVertex;	// 64 bytes

// Transformed Lit vertex
typedef struct
{
	float x, y, z, pad;								// screen points
	float r, g, b, a;								// color
	float u, v, pad1,pad2;							// Uv's
	float sr, sg, sb, pad3;							// specular color
} jeTLVertex;	// 64 bytes

typedef struct
{
	jeVec3d World;
	jeVec3d Normal;
	jeRGBA Color;
	float u, v;
	float pad1,pad2;
} jeVertex;	// 64 bytes !

// HW Transformation Vertex Struct - paradoxnj
typedef struct jeHWVertex
{
	jeVec3d					Pos;
	jeVec3d					Normal;
	uint32					Diffuse;
	float					u, v;
	float					lu, lv;
} jeHWVertex;

// temporary: !	Get rid of the JE_ types!

#define JE_RGBA				jeRGBA
#define JE_Rect				jeRect	
#define JE_LVertex			jeLVertex
#define JE_TLVertex			jeTLVertex


#ifdef __cplusplus
}
#endif


#endif
