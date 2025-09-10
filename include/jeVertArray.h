/*!
	@file jeVertArray.h 
	
	@author John Pollard
	@brief Vertex array definition and management functions

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

#ifndef JE_VERTARRAY_H
#define JE_VERTARRAY_H

#include "Vec3d.h"
#include "VFile.h"

#ifdef __cplusplus
extern "C" {
#endif

//========================================================================================
//	Typedefs/#defines
//========================================================================================

/*! @typedef jeVertArray
    @brief A reference to an Array of jeVertex
*/
typedef struct	jeVertArray				jeVertArray;

/*! @typedef jeVertArray_Optimizer
    @brief A reference to an Optimizer of Array of jeVertex
*/
typedef struct	jeVertArray_Optimizer	jeVertArray_Optimizer;

/*! @typedef jeVertArray_Index
    @brief The jeVertrray Index type
*/
typedef	uint16							jeVertArray_Index;

/*! @def JE_VERTARRAY_MAX_VERTS
	@brief The Max number of jeVertex in a jeVertArray
*/
#define JE_VERTARRAY_MAX_VERTS			(0xffff-1)

/*! @def JE_VERTARRAY_NULL_INDEX
	@brief The index indicating no value present
*/
#define	JE_VERTARRAY_NULL_INDEX			(JE_VERTARRAY_MAX_VERTS+1)

//========================================================================================
//	Structure defs
//========================================================================================

//========================================================================================
//	Function prototypes
//========================================================================================
JETAPI jeVertArray		* JETCC jeVertArray_Create(int32 StartVerts);
JETAPI jeVertArray		* JETCC jeVertArray_CreateFromFile(jeVFile *VFile);
JETAPI jeBoolean		JETCC jeVertArray_WriteToFile(const jeVertArray *Array, jeVFile *VFile);
JETAPI void				JETCC jeVertArray_Destroy(jeVertArray **VArray);
JETAPI jeBoolean		JETCC jeVertArray_IsValid(const jeVertArray *VArray);
JETAPI jeVertArray_Index JETCC jeVertArray_AddVert(jeVertArray *Array, const jeVec3d *Vert);
JETAPI jeVertArray_Index JETCC jeVertArray_ShareVert(jeVertArray *Array, const jeVec3d *Vert);
JETAPI void				JETCC jeVertArray_RemoveVert(jeVertArray *Array, jeVertArray_Index *Index);
JETAPI jeBoolean		JETCC jeVertArray_RefVertByIndex(jeVertArray *Array, jeVertArray_Index Index);
JETAPI void				JETCC jeVertArray_SetVertByIndex(jeVertArray *VArray, jeVertArray_Index Index, const jeVec3d *Vert);
JETAPI const jeVec3d	* JETCC jeVertArray_GetVertByIndex(const jeVertArray *VArray, jeVertArray_Index Index);
JETAPI int16			JETCC jeVertArray_GetMaxIndex( const jeVertArray *VArray );
JETAPI jeVertArray_Optimizer * JETCC jeVertArray_CreateOptimizer(jeVertArray *Array);
JETAPI void				JETCC jeVertArray_DestroyOptimizer(jeVertArray *Array, jeVertArray_Optimizer **Optimizer);
JETAPI jeVertArray_Index JETCC jeVertArray_GetOptimizedIndex(jeVertArray *Array, jeVertArray_Optimizer *Optimizer, jeVertArray_Index Index);
JETAPI jeBoolean		JETCC jeVertArray_GetEdgeVerts(jeVertArray_Optimizer *Optimizer, const jeVec3d *v1, const jeVec3d *v2, jeVertArray_Index *EdgeVerts, int32 *NumEdgeVerts, int32 MaxEdgeVerts);


#ifdef __cplusplus
}
#endif

#endif
