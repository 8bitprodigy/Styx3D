/****************************************************************************************/
/*  DCOMMON.H                                                                           */
/*                                                                                      */
/*  Author: John Pollard                                                                */
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
#ifndef DCOMMON_H
#define DCOMMON_H

//#include <Windows.h>	// {} CB commented out windows
// If you include Windows it MUST be before dcommon!

// FIXME:  What should we do with these?
#include "Basetype.h"
#include "XForm3d.h"
#include "Vec3d.h"
#include "PixelFormat.h"
#include "jeTypes.h"
#include "VFile.h"
#include "Camera.h"
#include "jeStaticMesh.h"
#include "jeChain.h"
#include "jeLight.h"

#ifdef __cplusplus
extern "C" {
#endif

#pragma pack(push)
#pragma pack(8)

#ifndef WINVER
#ifdef STRICT
typedef struct HWND__ * HWND;
typedef struct HBITMAP__ * HBITMAP;
#else // STRICT
typedef void * HWND;
typedef void * HBITMAP;
#endif // STRICT

#ifndef VOID
#define VOID void
typedef char CHAR;
typedef short SHORT;
typedef long LONG;
#endif

#ifndef BASETYPES
#define BASETYPES
typedef unsigned long ULONG;
typedef ULONG *PULONG;
typedef unsigned short USHORT;
typedef USHORT *PUSHORT;
typedef unsigned char UCHAR;
typedef UCHAR *PUCHAR;
typedef char *PSZ;
#endif  /* !BASETYPES */

typedef unsigned long       DWORD;
typedef int                 jeBoolean;
typedef unsigned char       BYTE;
typedef unsigned short      WORD;
typedef float               FLOAT;

typedef struct tagRECT
{
    LONG    left;
    LONG    top;
    LONG    right;
    LONG    bottom;
} RECT;

#endif // WINVER

#define	DRIVERCC _fastcall

#ifndef __cplusplus
	#define DllImport	__declspec( dllimport )
	#define DllExport	__declspec( dllexport )
#else
	#define DllImport	extern "C" __declspec( dllimport )
	#define DllExport	extern "C" __declspec( dllexport )
#endif

#define DRV_VERSION_MAJOR		200			// Jet 2.0
#define DRV_VERSION_MINOR		4			// version 3 has specular rgb in the verts ; 4 has bigger debug info
#define DRV_VMAJS				"200"
#define DRV_VMINS				"4" 
#define DRV_VMAJS_PLUS_DRV_VMINS	"200.4"


#ifndef US_TYPEDEFS
#define US_TYPEDEFS

	typedef uint8	U8;
	typedef uint16	U16;
	typedef uint32	U32;
	typedef char	C8;
	typedef int8	S8;
	typedef int16	S16;
	typedef int32	S32;
#endif

//===
// BEGIN - jeTexture implementation - paradoxnj 5/12/2005
//typedef struct jeTexture	jeTexture;
typedef struct jeTexture			jeTexture;
// END - jeTexture implementation - paradoxnj 5/12/2005

// BEGIN - Shaders - paradoxnj 6/8/2005
typedef struct jeShader				jeShader;
// END - Shaders - paradoxnj 6/8/2005

// BEGIN - Hardware True Type Fonts - paradoxnj 8/3/2005
typedef struct jeFont				jeFont;
// END - Hardware True Type Fonts - paradoxnj 8/3/2005

// BEGIN - Rendering Data sections - krouer 6/21/2005
typedef struct jeMaterialSpec		jeMaterialSpec;
typedef struct
{
	jeMaterialSpec	*Material;		// The MaterialSpec of the Render

	uint32			StartIndex;		// The index from where to start
	uint32			StartVertex;	// The vertex from where to start
	uint32			IndexCount;		// The number of index to use
	uint32			VertexCount;	// The number of vertex to use
} jeRenderSectionData;
// END - Rendering Data sections - krouer 6/21/2005

// BEGIN - Material layer type enumerations - krouer 8/16/2005
#define LAYER_TYPE_BASE 0 
#define LAYER_TYPE_LIGHTMAP 1 
#define LAYER_TYPE_BUMPMAP 2 
// END - Material layer type enumerations - krouer 8/16/2005


// DriverFormat flag bits (Exclusive)
#define RDRIVER_PF_2D_SHIFT					(0)			// Supports being used as a 2d decal surface
#define RDRIVER_PF_3D_SHIFT					(1)			// Supports being used as a 3d poly surface
#define RDRIVER_PF_LIGHTMAP_SHIFT			(2)			// Surface is a lightmap surface
#define RDRIVER_PF_PALETTE_SHIFT			(3)			// Surface is a palette
#define RDRIVER_PF_ALPHA_SURFACE_SHIFT		(4)			// Surface is an alpha map
// DriverFormat flag bits (Non-Exclusive)
#define RDRIVER_PF_OPTIONAL_SHIFT			(16)
#define RDRIVER_PF_HAS_ALPHA_SURFACE_SHIFT	(RDRIVER_PF_OPTIONAL_SHIFT + 0)		// Surface can take an alpha map
#define RDRIVER_PF_ALPHA_SHIFT				(RDRIVER_PF_OPTIONAL_SHIFT + 1)		// PixelFormat has alpha
#define RDRIVER_PF_CAN_DO_COLORKEY_SHIFT	(RDRIVER_PF_OPTIONAL_SHIFT + 2)		// Surface supports colorkeying
#define RDRIVER_PF_COMBINE_LIGHTMAP_SHIFT	(RDRIVER_PF_OPTIONAL_SHIFT + 3)		// Supports being rendered with a lightmap (3d will be set as well)

// DriverFormat flags (Exclusive)
#define RDRIVER_PF_2D						(1<<RDRIVER_PF_2D_SHIFT)				
#define RDRIVER_PF_3D						(1<<RDRIVER_PF_3D_SHIFT)				
#define RDRIVER_PF_LIGHTMAP					(1<<RDRIVER_PF_LIGHTMAP_SHIFT)			
#define RDRIVER_PF_PALETTE					(1<<RDRIVER_PF_PALETTE_SHIFT)			
#define RDRIVER_PF_ALPHA_SURFACE			(1<<RDRIVER_PF_ALPHA_SURFACE_SHIFT)			
// DriverFormat flags (Exclusive)
#define RDRIVER_PF_HAS_ALPHA_SURFACE		(1<<RDRIVER_PF_HAS_ALPHA_SURFACE_SHIFT)		
#define RDRIVER_PF_ALPHA					(1<<RDRIVER_PF_ALPHA_SHIFT)			
#define RDRIVER_PF_CAN_DO_COLORKEY			(1<<RDRIVER_PF_CAN_DO_COLORKEY_SHIFT)
#define RDRIVER_PF_COMBINE_LIGHTMAP			(1<<RDRIVER_PF_COMBINE_LIGHTMAP_SHIFT)	

#define RDRIVER_PF_MAJOR_MASK				((1<<RDRIVER_PF_OPTIONAL_SHIFT)-1)

typedef struct
{
	jePixelFormat	PixelFormat;
	uint32			Flags;				
} jeRDriver_PixelFormat;

#define RDRIVER_THANDLE_HAS_COLORKEY	(1<<0)		// The thandle is using color keying

typedef enum
{
	Rop_None,
	Rop_Multiply,			// P' = P1*P2	
	Rop_MultiplyX2,			// P' = P1*P2*2+Clamp (To allow for overbright lightmaps, looks more vibrant)
	Rop_MultiplyX4,			// P' = P1*P2*4+Clamp (To allow for overbright lightmaps, looks more vibrant)
	Rop_Add,				// P' = P1+P2+Clamp
} jeRDriver_Rop;

// BEGIN - jeTexture implementation - paradoxnj 5/12/2005
typedef struct
{
	int32					Width;
	int32					Height;
	int32					Stride;
	uint32					ColorKey;
	uint32					Flags;
	uint8					Log;
	jeRDriver_PixelFormat	PixelFormat;
    void*                   Direct;
} jeTexture_Info;

typedef struct
{
	// BEGIN - jeTexture implementation - paradoxnj 5/12/2005
	//jeTexture	*THandle;		// THandle for this layer
	jeTexture			*THandle;
	// END - jeTexture implementation - paradoxnj 5/12/2005

	jeRDriver_Rop		Rop;			// Blend mode to next THandle in the layer cascade
	
	// Shift and Scale values for this layer (based off the base UV set for the poly)
	jeFloat				ShiftU;
	jeFloat				ShiftV;
	jeFloat				ScaleU;
	jeFloat				ScaleV;
} jeRDriver_Layer;

typedef struct 
{
	void				*RGBLight[2];
	jeBoolean			Dynamic;
} jeRDriver_LMapCBInfo;

//===

typedef struct
{
	S32	LMapCount[16][4];				// LMap size / MipLevel
} DRV_Debug;

typedef struct
{
	int32		CacheFull;
	int32		CacheRemoved;
	int32		CacheFlushes;
	int32		TexMisses;
	int32		TexMissesFresh;
	int32		LMapMisses;
	int32		TexMissBytes;
	int32		LMapMissBytes;
	int32		TexBlitBytes;
	int32		LMapBlitBytes;

	int32		CacheTypes;
	int32		CacheMisses[32];
	int32		CacheFreshMisses[32];
	int32		CacheUses[32];
	int32		CacheSlots[32];

	int32		CardMem,SlotMem,UsedMem;
	float		MipBias;
	int32		Balances,BalancesFailed;
} DRV_CacheInfo;

typedef struct
{
	HWND		hWnd;
	
	U8			*Buffer;

	S32			Width;
	S32			Height;

	S32			PixelPitch;
	S32			BytesPerPixel;

	S32			R_shift;
	S32			G_shift;
	S32			B_shift;

	U32			R_mask;
	U32			G_mask;
	U32			B_mask;

	S32			R_width;
	S32			G_width;
	S32			B_width;
} DRV_Window;

#pragma pack(push)
#pragma pack(8)
typedef struct 
{
    U8 r, g, b;								// RGB components for RGB lightmaps
} DRV_RGB;
#pragma pack(pop)

//===========================================================================================
// FIXME:  Get palette stuff, and bitmap out of dcommon
#define	DRV_PALETTE_ENTRIES	256
typedef	DRV_RGB	DRV_Palette[DRV_PALETTE_ENTRIES];

// Bitmap hook into the drivers (engine uses these explicitly as is)
typedef struct
{
	char	Name[32];						// Duh, name of bitmap...
	U32		Flags;							// Flags
	S32		Width;							// Width of bitmap
	S32		Height;							// Height of bitmap
	U8		MipLevels;
	U8		*BitPtr[4];						// Pointer to location of bits (up to 4 miplevels)
	DRV_RGB *Palette;

	// Driver sets these in register functions
	//S32		Id;								// Bitmap handle for hardware...
	// BEGIN - jeTexture implementation - paradoxnj 5/12/2005
	//jeTexture	*THandle;
	jeTexture			*THandle;
	// END - jeTexture implementation - paradoxnj 5/12/2005

} DRV_Bitmap;
//===========================================================================================

#define LMAP_TYPE_LIGHT			0
#define LMAP_TYPE_FOG			1

typedef struct
{
	char				AppName[512];
	S32					Driver;
	char				DriverName[512];
	S32					Mode;
	char				ModeName[512];
	S32					Width;
	S32					Height;
	HWND				hWnd;
} DRV_DriverHook;

typedef struct
{
	// Texture info
	jeVec3d		VecU;
	jeVec3d		VecV;
	int32		TexMinsX;
	int32		TexMinsY;
	int32		TexWidth;
	int32		TexHeight;
	float		TexShiftX;
	float		TexShiftY;

	// Camera info
	jeXForm3d	CXForm;
	jeVec3d		CPov;

	float		XCenter;
	float		YCenter;

	float		XScale;
	float		YScale;
	float		XScaleInv;			// 1 / XScale
	float		YScaleInv;			// 1 / YScale;


	jeVec3d		PlaneNormal;		// Face normal
	float		PlaneDist;
	jeVec3d		RPlaneNormal;		// Rotated Face normal
	jeVec3d		Pov;
} GInfo;

// FIXME:  Move this into the GetDeviceCaps stuff
// What the driver can support as far as texture mapping is concerned
#define DRV_SUPPORT_ALPHA					(1<<0)		// Driver can do alpha blending
#define DRV_SUPPORT_COLORKEY				(1<<1)		// Driver can do pixel masking
#define DRV_SUPPORT_GAMMA					(1<<2)		// Gamma function works with the driver

// A hint to the engine as far as what to turn on and off...
#define DRV_PREFERENCE_NO_MIRRORS			(1<<0)		// Engine should NOT render mirrors
#define DRV_PREFERENCE_SORT_WORLD_FB		(1<<1)		// Sort world Front to Back
#define DRV_PREFERENCE_SORT_WORLD_BF		(1<<2)		// Sort world Back to Front
#define DRV_PREFERENCE_DRAW_WALPHA_IN_BSP	(1<<3)		// Draw world alphas in BSP sort

typedef struct DRV_EngineSettings
{
	U32			CanSupportFlags;
	U32			PreferenceFlags;
	U32			Reserved1;
	U32			Reserved2;
} DRV_EngineSettings;

// BEGIN - Hardware T&L - paradoxnj 4/5/2005
enum jeXFormType
{
	JE_XFORM_TYPE_VIEW = 0,
	JE_XFORM_TYPE_WORLD,
	JE_XFORM_TYPE_PROJECTION
};

// END - Hardware T&L - paradoxnj 4/5/2005

// Enumeration defines
typedef jeBoolean DRV_ENUM_MODES_CB( S32 Mode, char *ModeName, S32 Width, S32 Height, S32 BPP, void *Context);
typedef jeBoolean DRV_ENUM_DRV_CB( S32 Driver, char *DriverName, void *Context);

typedef jeBoolean DRIVERCC DRV_ENUM_DRIVER(DRV_ENUM_DRV_CB *Cb, void *Context); 
typedef jeBoolean DRIVERCC DRV_ENUM_MODES(S32 Driver, char *DriverName, DRV_ENUM_MODES_CB *Cb, void *Context); 

typedef jeBoolean DRV_ENUM_PFORMAT_CB(jeRDriver_PixelFormat *Format, void *Context);
typedef jeBoolean DRIVERCC DRV_ENUM_PFORMAT(DRV_ENUM_PFORMAT_CB *Cb, void *Context); 

typedef jeBoolean DRIVERCC DRV_GET_DEVICE_CAPS(jeDeviceCaps *DeviceCaps);

// Create/Destroy/Etc Driver functions
typedef jeBoolean DRIVERCC DRV_INIT(DRV_DriverHook *Hook);
typedef jeBoolean DRIVERCC DRV_SHUTDOWN(void);
typedef jeBoolean DRIVERCC DRV_RESET(void);
typedef jeBoolean DRIVERCC DRV_UPDATE_WINDOW(void);
typedef jeBoolean DRIVERCC DRV_SET_ACTIVE(jeBoolean Active);

// BEGIN - jeTexture implementation - paradoxnj 5/12/2005
// Texture surface functions
typedef jeTexture *DRIVERCC CREATE_TEXTURE(int32 Width, int32 Height, int32 NumMipLevels, const jeRDriver_PixelFormat *PixelFormat);
typedef jeTexture *DRIVERCC CREATE_TEXTURE_FROM_FILE(jeVFile *File);

typedef jeBoolean DRIVERCC DESTROY_TEXTURE(jeTexture *THandle);

typedef jeBoolean DRIVERCC LOCK_THANDLE(jeTexture *THandle, int32 MipLevel, void **Data);
typedef jeBoolean DRIVERCC UNLOCK_THANDLE(jeTexture *THandle, int32 MipLevel);

typedef jeBoolean DRIVERCC SET_PALETTE(jeTexture *THandle, jeTexture *PalHandle);
typedef jeTexture *DRIVERCC GET_PALETTE(jeTexture *THandle);

typedef jeBoolean DRIVERCC SET_ALPHA(jeTexture *THandle, jeTexture *PalHandle);
typedef jeTexture *DRIVERCC GET_ALPHA(jeTexture *THandle);

typedef jeBoolean DRIVERCC THANDLE_GET_INFO(jeTexture *THandle, int32 MipLevel, jeTexture_Info *Info);
// END - jeTexture implementation - paradoxnj 5/12/2005

// Scene management functions
typedef jeBoolean DRIVERCC BEGIN_SCENE(jeBoolean Clear, jeBoolean ClearZ, RECT *WorldRect, jeBoolean Wireframe);
typedef jeBoolean DRIVERCC END_SCENE(void);
typedef jeBoolean DRIVERCC BEGIN_BATCH(void);
typedef jeBoolean DRIVERCC END_BATCH(void);

// Render functions
typedef jeBoolean DRIVERCC RENDER_G_POLY(jeTLVertex *Pnts, int32 NumPoints, uint32 Flags);
typedef jeBoolean DRIVERCC RENDER_W_POLY(jeTLVertex *Pnts, int32 NumPoints, jeRDriver_Layer *Layers, int32 NumLayers, void *LMapCBContext, uint32 Flags);
typedef jeBoolean DRIVERCC RENDER_MT_POLY(jeTLVertex *Pnts, int32 NumPoints, jeRDriver_Layer *Layers, int32 NumLayers, uint32 Flags);

typedef jeBoolean DRIVERCC DRAW_DECAL(jeTexture *THandle, RECT *SRect, int32 x, int32 y);

typedef jeBoolean DRIVERCC SCREEN_SHOT(const char *Name);
typedef jeBoolean DRIVERCC DRAW_TEXT(char *text, int x, int y, uint32 color);

typedef jeBoolean DRIVERCC SET_FOG(float r, float g, float b, float start, float endi, jeBoolean enable);

typedef jeBoolean DRIVERCC SET_GAMMA(float Gamma);
typedef jeBoolean DRIVERCC GET_GAMMA(float *Gamma);

// BEGIN - Hardware T&L - paradoxnj 4/5/2005
typedef jeBoolean DRIVERCC SET_MATRIX(uint32 type, jeXForm3d *XForm);
typedef jeBoolean DRIVERCC GET_MATRIX(uint32 type, jeXForm3d *XForm);
typedef jeBoolean DRIVERCC SET_CAMERA(jeCamera *Camera);
// END - Hardware T&L - paradoxnj 4/5/2005

// Static Meshes - paradoxnj 8/1/2005
typedef uint32 DRIVERCC ADD_STATIC_MESH(jeHWVertex *Points, int32 NumPoints, jeRDriver_Layer *Layers, int32 NumLayers, uint32 Flags);
typedef jeBoolean DRIVERCC REMOVE_STATIC_MESH(uint32 id);
typedef jeBoolean DRIVERCC RENDER_STATIC_MESH(uint32 id, int32 StartVertex, int32 NumPolys, jeXForm3d *XForm);
// Static Meshes - paradoxnj 8/1/2005

// BEGIN - Hardware True Type Fonts - paradoxnj 8/3/2005
typedef jeFont * DRIVERCC CREATE_FONT(int32 Height, int32 Width, uint32 Weight, jeBoolean Italic, const char *facename);
typedef jeBoolean DRIVERCC DRAW_FONT(jeFont *Font, int32 x, int32 y, uint32 Color, const char *text);
typedef jeBoolean DRIVERCC DESTROY_FONT(jeFont **Font);
// END - Hardware True Type Fonts - paradoxnj 8/3/2005

// BEGIN - Render state access - paradoxnj 12/25/2005
typedef jeBoolean DRIVERCC SET_RENDER_STATE(uint32 state, uint32 value);
// END - Render state access - paradoxnj 12/25/2005

typedef void JETCC SETUP_LIGHTMAP_CB(jeRDriver_LMapCBInfo *LMapCBInfo, void *Context);

typedef struct
{
	char				*Name;
	S32					VersionMajor;
	S32					VersionMinor;

	// Error handling hooks set by driver
	S32					LastError;							// Last error driver made
	char				*LastErrorStr;						// NULL terminated error string
	
	// Enum Modes/Drivers
	DRV_ENUM_DRIVER		*EnumSubDrivers;
	DRV_ENUM_MODES		*EnumModes;
	
	DRV_ENUM_PFORMAT	*EnumPixelFormats;

	// Device Caps
	DRV_GET_DEVICE_CAPS	*GetDeviceCaps;

	// Init/DeInit functions
	DRV_INIT			*Init;
	DRV_SHUTDOWN		*Shutdown;
	DRV_RESET			*Reset;
	DRV_UPDATE_WINDOW	*UpdateWindow;
	DRV_SET_ACTIVE		*SetActive;
	
	// Create/Destroy texture functions
	CREATE_TEXTURE		*THandle_Create;
	
	// BEGIN - jeTexture implementation - paradoxnj 5/12/2005
	CREATE_TEXTURE_FROM_FILE	*THandle_CreateFromFile;
	// END - jeTexture implementation - paradoxnj 5/12/2005

	DESTROY_TEXTURE		*THandle_Destroy;

	// Texture manipulation functions
	LOCK_THANDLE		*THandle_Lock;
	UNLOCK_THANDLE		*THandle_UnLock;

	// Palette access functions
	SET_PALETTE			*THandle_SetPalette;
	GET_PALETTE			*THandle_GetPalette;

	// Palette access functions
	SET_ALPHA			*THandle_SetAlpha;
	GET_ALPHA  			*THandle_GetAlpha;

	THANDLE_GET_INFO	*THandle_GetInfo;

	// Scene management functions
	BEGIN_SCENE			*BeginScene;
	END_SCENE			*EndScene;

	BEGIN_BATCH			*BeginBatch;
	END_BATCH			*EndBatch;
	
	// Render functions
	RENDER_G_POLY		*RenderGouraudPoly;
	RENDER_W_POLY		*RenderWorldPoly;
	RENDER_MT_POLY		*RenderMiscTexturePoly;

	//Decal functions
	DRAW_DECAL			*DrawDecal;

	S32					NumWorldPixels;
	S32					NumWorldSpans;
	S32					NumRenderedPolys;
	DRV_CacheInfo		*CacheInfo;

	SCREEN_SHOT			*ScreenShot;

	SET_GAMMA			*SetGamma;
	GET_GAMMA			*GetGamma;
	
	// BEGIN - Hardware T&L - paradoxnj 4/5/2005
	SET_MATRIX			*SetMatrix;
	GET_MATRIX			*GetMatrix;
	SET_CAMERA			*SetCamera;
	// END - Hardware T&L - paradoxnj 4/5/2005

	// Driver preferences
	DRV_EngineSettings	*EngineSettings;

	// The engine supplies these for the drivers misc use
	SETUP_LIGHTMAP_CB	*SetupLightmap;

	// KROUER: move this 2 new function to the end of the structure, keep backward compatibility
	// Introduced by Gerald
	DRAW_TEXT		   *DrawText;
	SET_FOG				*SetFog;

	// BEGIN - Static Meshes - paradoxnj 8/2/2005
	ADD_STATIC_MESH		*StaticMesh_Add;
	REMOVE_STATIC_MESH	*StaticMesh_Remove;
	RENDER_STATIC_MESH	*StaticMesh_Render;
	// END - Static Meshes - paradoxnj 8/2/2005

	// BEGIN - Hardware True Type Fonts - paradoxnj 8/13/2005
	CREATE_FONT			*Font_Create;
	DRAW_FONT			*Font_Draw;
	DESTROY_FONT		*Font_Destroy;
	// END - Hardware Truw Type Fonts - paradoxnj 8/13/2005

	// BEGIN - Render state access - paradoxnj 12/25/2005
	SET_RENDER_STATE	*SetRenderState;
	// END - Render state access - paradoxnj 12/25/2005
} DRV_Driver;

enum jeRenderState
{
	JE_RENDERSTATE_ENABLE_ZBUFFER = 0,
	JE_RENDERSTATE_ENABLE_ZWRITES,
	JE_RENDERSTATE_ENABLE_ALPHABLENDING,
	JE_RENDERSTATE_ALPHAREF,
	JE_RENDERSTATE_ALPHAFUNC,
	JE_RENDERSTATE_ENABLE_ALPHATESTING,
	JE_RENDERSTATE_DEPTHFUNC,
	JE_RENDERSTATE_FILLMODE,
	JE_RENDERSTATE_SHADEMODE,
	JE_RENDERSTATE_CULLMODE,
	JE_RENDERSTATE_ENABLE_FOG,
	JE_RENDERSTATE_FOGCOLOR,
	JE_RENDERSTATE_FOGSTART,
	JE_RENDERSTATE_FOGEND,
	JE_RENDERSTATE_HWLIGHTINGENABLE,
	JE_RENDERSTATE_AMBIENTLIGHT,
	JE_RENDERSTATE_ENABLE_STENCIL,
	JE_RENDERSTATE_STENCILREF,
	JE_RENDERSTATE_STENCILMASK,
	JE_RENDERSTATE_STENCILWRITEMASK,
	JE_RENDERSTATE_STENCILFUNC,
	JE_RENDERSTATE_STENCILFAIL,
	JE_RENDERSTATE_STENCILZFAIL,
	JE_RENDERSTATE_STENCILPASS
};

enum jeFill
{
	JE_FILL_POINT = 0,
	JE_FILL_WIREFRAME,
	JE_FILL_SOLID
};

enum jeCullMode
{
	JE_CULL_NONE = 0,
	JE_CULL_CW,
	JE_CULL_CCW
};

enum jeShadeMode
{
	JE_SHADE_FLAT = 0,
	JE_SHADE_GOURAUD,
	JE_SHADE_PHONG
};

enum jeRenderCmpFunc
{
	JE_CMP_NEVER = 0,
	JE_CMP_LESS,
	JE_CMP_EQUAL,
	JE_CMP_LEQUAL,
	JE_CMP_GREATER,
	JE_CMP_GEQUAL,
	JE_CMP_NEQUAL,
	JE_CMP_ALWAYS
};

enum jeStencilOp
{
	JE_STENCILOP_KEEP = 0,
	JE_STENCILOP_ZERO,
	JE_STENCILOP_REPLACE,
	JE_STENCILOP_INCRWRAP,
	JE_STENCILOP_DECRWRAP,
	JE_STENCILOP_INVERT,
	JE_STENCILOP_INCR,
	JE_STENCILOP_DECR
};

enum jeBlendOp
{
	JE_BLEND_ZERO = 0,
	JE_BLEND_ONE,
	JE_BLEND_SOURCE,
	JE_BLEND_INVERSESOURCE,
	JE_BLEND_SOURCEALPHA,
	JE_BLEND_INVERSESOURCEALPHA,
	JE_BLEND_DEST,
	JE_BLEND_INVERSEDEST,
	JE_BLEND_DESTALPHA,
	JE_BLEND_INVERSEDESTALPHA,
	JE_BLEND_SOURCEALPHASAT
};

typedef struct jeDriverStats
{
	int32								NumPolysRendered;
	int32								NumVertexBuffersRendered;
	int32								NumDecalsRendered;
} jeDriverStats;

#ifdef __cplusplus
class CDRV_Driver
{
protected:
	virtual ~CDRV_Driver()						{}

public:
	virtual jeBoolean					GetName(char *Name) = 0;
	virtual jeBoolean					GetDriverStats(jeDriverStats *Stats) = 0;

	virtual jeBoolean					EnumSubDrivers(DRV_ENUM_DRV_CB *Cb, void *Context) = 0;
	virtual jeBoolean					EnumModes(int32 Driver, char *DriverName, DRV_ENUM_MODES_CB *Cb, void *Context) = 0;

	virtual jeBoolean					GetDeviceCaps(jeDeviceCaps *Caps) = 0;

	virtual jeBoolean					Initialize(DRV_DriverHook *Hook) = 0;
	virtual jeBoolean					Shutdown() = 0;
	
	virtual jeTexture					*THandle_Create(int32 Width, int32 Height, int32 NumMipLevels, const jeRDriver_PixelFormat *Format) = 0;
	virtual jeTexture					*THandle_CreateFromFile(jeVFile *File) = 0;

	virtual jeBoolean					SetRenderState(uint32 state, uint32 value) = 0;

	virtual jeBoolean					BeginScene(uint32 ClearFlags) = 0;
	virtual jeBoolean					EndScene() = 0;
	
	virtual jeBoolean					SetOrtho(int32 Left, int32 Right, int32 Width, int32 Height) = 0;
	virtual jeBoolean					SetPerspective(float fov, float aspect, float znear, float zfar) = 0;

	virtual jeBoolean					EnableLight(int32 id, jeLight *Light) = 0;

//	virtual jeBoolean					RenderVertexBuffer(jeVertexBuffer *VB, int16 StartVertex, jeVec3d *Position, jeVec3d *Rotation, uint32 Flags) = 0;
	virtual jeBoolean					DrawBitmap(jeTexture *THandle, RECT *SRect, int32 x, int32 y) = 0;

	virtual jeBoolean					Screenshot(const char *filename) = 0;

	virtual jeBoolean					SetGamma(float Gamma) = 0;
	virtual jeBoolean					GetGamma(float *Gamma) = 0;

	virtual jeBoolean					DrawText(char *Text, int x, int y, uint32 Color) = 0;
};
#endif

typedef jeBoolean DRV_Hook(DRV_Driver **Hook);

//
//	Error defines set by the driver.  These will be in the LastError member of AFX_DRIVER
//	structure.  LastErrorStr will contain a NULL terminated detail error string set by the driver
//
#define DRV_ERROR_NONE					0	// No error has occured
#define DRV_ERROR_INVALID_PARMS			1	// invalid parameters passed
#define DRV_ERROR_NULL_WINDOW			2	// Null window supplied
#define DRV_ERROR_INIT_ERROR			3	// Error intitializing
#define DRV_ERROR_INVALID_REGISTER_MODE	4	// Invalid register mode
#define DRV_ERROR_NO_MEMORY				5	// Not enough ram
#define DRV_ERROR_MAX_TEXTURES			6	// Max texture capacity has been exceeded...
#define DRV_ERROR_GENERIC				7	// Generic error	 
#define DRV_ERROR_UNDEFINED				8	// An undefined error has occured
#define DRV_ERROR_INVALID_WINDOW_MODE	9	// Requested window/full not supported

typedef enum
{
	RENDER_NONE,
	RENDER_WORLD,
	RENDER_MESHES,
	RENDER_MODELS
} DRV_RENDER_MODE;

#ifdef __cplusplus
}
#endif

#pragma pack(pop)

#endif
