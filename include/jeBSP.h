/*!
	@file jeBSP.h 
	
	@author John Pollard
	@brief Binary Space Partition Implementation

	@par Licence
	The contents of this file are subject to the Jet3D Public License       
	Version 1.02 (the "License"); you may not use this file except in         
	compliance with the License. You may obtain a copy of the License at       
	http://www.jet3d.com                                                        
                                                                             
	@par
	Software distributed under the License is distributed on an "AS IS"           
	basis, WITHOUT WARRANTY OF ANY KIND, either express or implied.  See           
	the License for the specific language governing rights and limitations          
	under the License.                                                               
                                                                                  
	@par
	The Original Code is Jet3D, released December 12, 1999.                            
	Copyright (C) 1996-1999 Eclipse Entertainment, L.L.C. All Rights Reserved           
*/

/*!  @note  BSP
*	This object represent the Binary Space Partition of a world. This is the basic entity of the rendering engine.
*/
#ifndef JEBSP_H
#define JEBSP_H

#include "Engine.h"

#include "jeBrush.h"
#include "Camera.h"
#include "jeFrustum.h"
#include "jeLight.h"
#include "jeChain.h"
#include "BaseType.h"
#include "jeTypes.h"

#ifdef __cplusplus
extern "C" {
#endif

// This looks like a big mess.  But these guys are sticking together for life.
// Might as well be in the same file, for speed!!!
/*! @brief Represent the Binary Space Partition data handler */
typedef struct			jeBSP				jeBSP;				// Tree
/*! @brief Represent the Brush representation in the BSP */
typedef struct			jeBSP_Brush			jeBSP_Brush;		// Brushes
typedef struct			jeBSP_Side			jeBSP_Side;			// Brush sides
typedef struct			jeBSP_TopSide		jeBSP_TopSide;		// Top level brush sides
typedef struct			jeBSP_TopBrush		jeBSP_TopBrush;		// Top level brushes

typedef struct			jeBSPNode			jeBSPNode;			// Node (subspace seperators)
typedef struct			jeBSPNode_Portal	jeBSPNode_Portal;	// Portal (passage from leaf to leaf)
typedef struct			jeBSPNode_Face		jeBSPNode_Face;		// Node face during construction
typedef struct			jeBSPNode_DrawFace	jeBSPNode_DrawFace;	// Face that gets rendered on nodes
typedef struct			jeBSPNode_Area		jeBSPNode_Area;

typedef void			jeBSPNode_DrawFaceCB(const jeTLVertex *Verts, int32 NumVerts, void *Context);

typedef enum
{
	Logic_Lazy=0,		//!< No optimizations
	Logic_Normal=1,		//!< Does num splits/balance test
	Logic_Smart=2,		//!< Does num splits/balance, and test volumes
	Logic_Super=3		//!< Final, does splits/balance, test voumes, csg, and solid fill
} jeBSP_Logic;

// Render/Build Options
typedef enum
{
	RenderMode_Lines,					//<! Renders using lines only
	RenderMode_Flat,					//<! Renders using unique color per brush face
	RenderMode_BSPSplits,				//<! Renders using unique color per drawface
	RenderMode_Textured,				//<! Renders using material assigned/no lighting
	RenderMode_TexturedAndLit,			//<! Renders using materials assigned, with lighting applied
} jeBSP_RenderMode;

typedef int32 jeBSP_LogicBalance;		//<! (0...10), 0 = Less splits, 10 = Balanced tree

/*! @typedef jeBSP_Options
	@brief jeBSP_Options is the type of Options parameters of function jeBSP_RebuildGeometry()<br>
 */
typedef uint32 jeBSP_Options;

#define BSP_OPTIONS_MAKE_VIS_AREAS		(1<<0)
#define BSP_OPTIONS_CSG_BRUSHES			(1<<1)
#define BSP_OPTIONS_SOLID_FILL			(1<<3)

typedef struct
{
	int32			NumBrushes;				// Total brushes in bsp
	int32			NumVisibleBrushFaces;
	int32			NumTotalBrushFaces;
	int32			NumNodes;
	int32			NumLeafs;
	int32			NumSplits;
	int32			NumDrawFaces;
	int32			NumSubdividedDrawFaces;
	int32			NumPortals;
	int32			NumVisPortals;
	int32			NumAreas;				// Number of VisAreas
	int32			NumVisibleAreas;	
	int32			NumMakeFaces;
	int32			NumMergedFaces;
} jeBSP_DebugInfo;

//================================================================================================
//	Build/Rebuild
//================================================================================================
/*! @brief Create the BSP object
*/
jeBSP			*jeBSP_Create(void);

/*! @brief Rebuild the geomerty of the BSP with the brushes geometry specified in BrushChain

	@param[in] BSP The BSP instance on input of the BSP build algorithm
	@param[in] BrushChain The list of the brushes 
	@param[in] Options The BSP build algorithm options. This parameter can be a combination of BSP_Options. See Remarks
	@param[in] Logic   The BSP build algorithm logic.
	@param[in] LogicBalance The BSP build algorithm balance between logic request and complexity of the geometry.
	@return The BSP instance of the result of the BSP build algorithm if succeed, NULL otherwise

    @par Remarks
	@par Options
			The BSP_Options can be combined:
			<table border=1 bordercolor=#000000 cellspacing=0 cellpadding=2><tr><th width="40%">Value</th><th width="60%">Meaning</th></tr>
			<tr valign=top><td>BSP_OPTIONS_MAKE_VIS_AREAS</td><td>Indicate BSP rebuild to take care od the VIS area brushes. BSP splitter will create areas.</td></tr>
			<tr valign=top><td>BSP_OPTIONS_CSG_BRUSHES</td><td>Indicate BSP to build brushes</td></tr>
			<tr valign=top><td>BSP_OPTIONS_SOLID_FILL</td><td>&nbsp;</td></tr>
			</table>
 */
jeBSP			*jeBSP_RebuildGeometry(	jeBSP				*BSP,
										jeChain				*BrushChain, 
										jeBSP_Options		Options,
										jeBSP_Logic			Logic, 
										jeBSP_LogicBalance	LogicBalance);

/*! @brief Destroy the BSP instance
    @param BSPTree The address of the BSP instance pointer
    
    Decrement the reference counter of the BSP instance and destroy it only when the counter reach 0.
    @note The address is set to NULL when returns
*/
void			jeBSP_Destroy(jeBSP **BSPTree);

jeBoolean		jeBSP_SetArrays(jeBSP *BSP, jeFaceInfo_Array *FArray, jeMaterial_Array *MArray, jeChain *LChain, jeChain *DLChain);

jeBoolean		jeBSP_AddBrush(jeBSP *BSPTree, jeBrush *Brush, jeBoolean AutoLight);
jeBoolean		jeBSP_HasBrush(jeBSP *BSPTree, jeBrush *Brush);
jeBoolean		jeBSP_RemoveBrush(jeBSP *BSPTree, jeBrush *Brush);

jeBoolean		jeBSP_MakeVisAreas(jeBSP *BSPTree);
jeBoolean		jeBSP_DestroyVisAreas(jeBSP *BSP);

jeBoolean		jeBSP_AddObject(jeBSP *BSP, jeObject *Object);
jeBoolean		jeBSP_RemoveObject(jeBSP *BSP, jeObject *Object);
jeBoolean		jeBSP_HasObject(jeBSP *BSP, jeObject *Object);

//================================================================================================
//	Render/Vis
//================================================================================================
jeBoolean		jeBSP_VisFrame(jeBSP *BSPTree, const jeCamera *Camera, const jeFrustum *ModelSpaceFrustum);
jeBoolean		jeBSP_RenderFrontToBack(jeBSP *Tree, jeCamera *Camera, jeFrustum *CameraSpaceFrustum, jeFrustum *ModelSpaceFrustum, jeXForm3d *ModelToCameraXForm);
jeBoolean		jeBSP_RenderAndVis(jeBSP *Tree, jeCamera *Camera, jeFrustum *Frustum);
jeBoolean		jeBSP_RenderAreas(jeBSP *Tree, jeCamera *Camera, jeFrustum *CameraSpaceFrustum, jeFrustum *ModelSpaceFrustum, jeXForm3d *ModelToCameraXForm);
#ifdef AREA_DRAWFACE_TEST
jeBoolean 		jeBSP_MakeAreaDrawFaces(jeBSP *BSP);
#endif

//================================================================================================
// Update lights/faces/brushes
//================================================================================================
jeBoolean		jeBSP_UpdateBrush(jeBSP *BSPTree, jeBrush *Brush, jeBoolean AutoLight);
jeBoolean		jeBSP_UpdateBrushFace(jeBSP *BSP, const jeBrush_Face *Face, jeBoolean AutoLight);

jeBoolean		jeBSP_PatchLighting(jeBSP *Tree);
jeBoolean		jeBSP_RebuildLights(jeBSP *Tree);
jeBoolean		jeBSP_RebuildLightsFromPoint(jeBSP *Tree, const jeVec3d *Pos, jeFloat Radius);

jeBoolean		jeBSP_UpdateAll(jeBSP *BSP);
jeBoolean		jeBSP_RebuildFaces(jeBSP *BSPTree);

jeBoolean		jeBSP_SetBrushFaceCB(jeBSP *BSP, jeBSPNode_DrawFaceCB *CB, void *Context);
jeBoolean		jeBSP_SetBrushFaceCBOnOff(jeBSP *BSP, const jeBrush_Face *Face, jeBoolean OnOff);

//================================================================================================
//	Misc
//================================================================================================
jeBoolean		jeBSP_SetXForm(jeBSP *BSP, const jeXForm3d *XForm);
jeBoolean		jeBSP_SetEngine(jeBSP *Tree, jeEngine *Engine);
jeBoolean		jeBSP_SetWorld(jeBSP *Tree, jeWorld *World);
jeBoolean		jeBSP_SetRenderMode(jeBSP *BSP, jeBSP_RenderMode RenderMode);
jeBoolean		jeBSP_SetDefaultContents(jeBSP *BSP, jeBrush_Contents DefaultContents);
jeBSPNode_Area	*jeBSP_FindArea(jeBSP * BSP, const jeVec3d *Pos);

typedef void (* jeBSP_DoAreaFunc) ( jeBSPNode_Area * Area, void * Context );
jeBoolean		jeBSP_DoAllAreasInBox(jeBSP *BSP,jeExtBox *BBox,jeBSP_DoAreaFunc CB,void * Context);

jeBoolean		jeBSP_GetModelSpaceBox(const jeBSP *BSP, jeExtBox *Box);
jeBoolean		jeBSP_GetWorldSpaceBox(const jeBSP *BSP, jeExtBox *Box);

// Icestorm : If Impact(Box) or Plane are NULL: Tests only, whether there is an collision (it's a bit faster)
jeBoolean		jeBSP_Collision(const jeBSP *BSP, 
								const jeExtBox *Box, 
								const jeVec3d *Front, 
								const jeVec3d *Back, 
								jeVec3d *Impact, 
								jePlane *Plane);

// Added by Icestorm
jeBoolean		jeBSP_ChangeBoxCollision(	const jeBSP		*BSP, 
											const jeVec3d	*Pos,
											const jeExtBox	*FrontBox, 
											const jeExtBox	*BackBox, 
											jeExtBox		*ImpactBox, 
											jePlane			*Plane);

jeBoolean		jeBSP_RayIntersectsBrushes(const jeBSP *BSP, const jeVec3d *Front, const jeVec3d *Back, jeBrushRayInfo *Info);

const			jeBSP_DebugInfo *jeBSP_GetDebugInfo(const jeBSP *BSPTree);

#ifdef __cplusplus
}
#endif

#endif
