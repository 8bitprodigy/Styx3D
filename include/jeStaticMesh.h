/**
   @file jeStaticMesh.h                                                                       
                                                                                      
   @author Anthony Rufrano	                                                          
   @brief Static mesh code     		                                          
                                                                                      
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
#ifndef JE_STATIC_MESH_H
#define JE_STATIC_MESH_H

#include "BaseType.h"
#include "jeTypes.h"
#include "XForm3d.h"
#include "Vec3d.h"
#include "VFile.h"
#include "jeResource.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct jeCamera								jeCamera;
typedef struct jeEngine								jeEngine;
typedef struct jeFrustum							jeFrustum;
typedef struct jeExtBox								jeExtBox;
typedef struct jePlane								jePlane;

/*!
	@typedef jeStaticMesh
	@brief A static mesh
*/
typedef struct jeStaticMesh							jeStaticMesh;

/*!
	@fn jeStaticMesh *jeStaticMesh_Create(const char *Name)
	@brief Creates a static mesh
	@param[in] MeshName Name of the static mesh (Directory/PAK.FileName (no extension))
	@param[in] ResMgr The resource manager
	@return The new mesh
*/
JETAPI jeStaticMesh * JETCC jeStaticMesh_Create(const char *MeshName, jeResourceMgr *ResMgr);

/*!
	@fn uint32 jeStaticMesh_Destroy(jeStaticMesh **Mesh)
	@brief Decrements the reference counter to the mesh.  If it's 0, the mesh is destroyed
	@param[in] Mesh The mesh to dereference
	@return The number of references to the mesh
*/
JETAPI uint32 JETCC jeStaticMesh_Destroy(jeStaticMesh **Mesh);

/*!
	@fn uint32 jeStaticMesh_CreateRef(jeStaticMesh *Mesh)
	@brief Increases the reference count of the mesh
	@param[in] Mesh The mesh to reference
	@return The number of references to the mesh
*/
JETAPI uint32 JETCC jeStaticMesh_CreateRef(jeStaticMesh *Mesh);

/*!
	@fn jeBoolean jeStaticMesh_Render(jeStaticMesh *Mesh, jeEngine *Engine, jeCamera *Camera, jeFrustum *Frustum, jeXForm3d *XForm)
	@brief Renders the mesh at a given location
	@param[in] Mesh The mesh to render
	@param[in] Engine The engine to render with
	@param[in] Camera The camera to render through
	@param[in] Frustum The frustum to cull with
	@param[in] XForm The location to render to
	@return JE_TRUE on success, JE_FALSE on failure
*/
JETAPI jeBoolean JETCC jeStaticMesh_Render(jeStaticMesh *Mesh, jeEngine *Engine, jeCamera *Camera, jeFrustum *Frustum, jeXForm3d *XForm);

/*!
	@fn jeBoolean jeStaticMesh_GetExtBox(jeStaticMesh *Mesh, jeExtBox *BBox)
	@brief Gets the mesh's bounding box
	@param[in] Mesh The mesh to get the box from
	@param[out] BBox The mesh's bounding box
	@return JE_TRUE on success, JE_FALSE on failure
*/
JETAPI jeBoolean JETCC jeStaticMesh_GetExtBox(jeStaticMesh *Mesh, jeExtBox *BBox);

/*!
	@fn jeBoolean jeStaticMesh_SetExtBox(jeStaticMesh *Mesh, jeExtBox *BBox)
	@brief Sets the mesh's bounding box
	@param[in] Mesh The mesh to set the bounding box for
	@param[in] BBox The bounding box to set
	@return JE_TRUE on success, JE_FALSE on failure
*/
JETAPI jeBoolean JETCC jeStaticMesh_SetExtBox(jeStaticMesh *Mesh, jeExtBox *BBox);

/*!
	@fn jeBoolean jeStaticMesh_Collision(jeStaticMesh *Mesh, jeExtBox *BBox, jeVec3d *Front, jeVec3d *Back, jeVec3d *Impact, jePlane *Plane)
	@brief Performs collision testing on the mesh
	@param[in] Mesh The mesh to test
	@param[in] BBox The mesh's bounding box
	@param[in] Front The forward vector to test against
	@param[in] Back The back vector to test against
	@param[out] Impact The point of impact
	@param[out] Plane The plane at which the collision occurred
	@return JE_TRUE if there was a collision, JE_FALSE if not
*/
JETAPI jeBoolean JETCC jeStaticMesh_Collision(jeStaticMesh *Mesh, jeExtBox *BBox, jeVec3d *Front, jeVec3d *Back, jeVec3d *Impact, jePlane *Plane);

#ifdef __cplusplus
}
#endif

// End of header
#endif
