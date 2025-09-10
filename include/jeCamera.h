/*!
	@file jeCamera.h 
	
	@author Anthony Rufrano
	@brief A camera (view and projection matrix)

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

	@note C++ conversion done by Anthony Rufrano (paradoxnj)
*/
#ifndef JE_CAMERA_H
#define JE_CAMERA_H

#include "BaseType.h"
#include "jeTypes.h"
#include "jeVec3d.h"
#include "jeXForm3d.h"

/*!
	@fn jeCamera *jeCamera_Create(float fov, jeRect *Rect)
	@brief Creates a camera object
	@param[in] fov The camera's field of view
	@param[in] Rect The viewing rect
	@return A new camera object
*/
JETAPI jeCamera * JETCC jeCamera_Create(float fov, jeRect *Rect);

/*!
	@class jeCamera jeCamera.h "include\jeCamera.h"
	@brief Manages a view and projection matrix
*/
class jeCamera : virtual public jeUnknown
{
protected:
	virtual ~jeCamera()						{}

public:
	virtual jeBoolean						SetPosition(jeVec3d *pos) = 0;
	virtual jeBoolean						GetPosition(jeVec3d *pos) = 0;

	virtual jeBoolean						SetRotation(jeVec3d *rot) = 0;
	virtual jeBoolean						GetRotation(jeVec3d *rot) = 0;

	virtual jeBoolean						SetViewMatrix(jeXForm3d *XF) = 0;
	virtual jeBoolean						GetViewMatrix(jeXForm3d *XF) = 0;

	virtual jeBoolean						CalcProjectionMatrix(float aspect, float near_z, float far_z) = 0;
	virtual jeBoolean						LookAt(jeVec3d *Eye, jeVec3d *At) = 0;

	virtual jeBoolean						SetViewRect(jeRect *Rect) = 0;
	virtual jeBoolean						GetViewRect(jeRect *Rect) = 0;

	virtual jeBoolean						SetFOV(float fov) = 0;
	virtual float							GetFOV() = 0;

	virtual jeBoolean						ScreenPointToWorld(int32 x, int32 y, jeVec3d *Result) = 0;
};

#endif
