/****************************************************************************************/
/*  CAMERA.C                                                                            */
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
#include <math.h>
#include <assert.h>

#include "Camera.h"
#include "Ram.h"
#include "Errorlog.h"
#include "List.h"
#include "Camera._h"


#ifndef max
#define max(AA,BB)  (  ((AA)>(BB)) ?(AA):(BB)  )
#endif
#ifndef min
#define min(AA,BB)  (  ((AA)<(BB)) ?(AA):(BB)  )
#endif

#define CAMERA_MINIMUM_PROJECTION_DISTANCE (0.010f)

//=====================================================================================
//	jeCamera_Create
//=====================================================================================
JETAPI jeCamera *JETCC jeCamera_Create(jeFloat FovRadians, const jeRect *Rect)
{
	jeCamera *Camera;

	assert( Rect != NULL );

	Camera = JE_RAM_ALLOCATE_STRUCT_CLEAR(jeCamera);

	if (Camera == NULL)
	{
		jeErrorLog_Add(-1, "jeCamera_Create: CreateCamera failed");
		return NULL;
	}

	Camera->ZScale = 0.5f;

	jeCamera_SetAttributes(Camera,FovRadians,Rect);

	// BEGIN - Far clip plane - paradoxnj MODIFIED 3/9/2005
	Camera->ZFarEnable = JE_TRUE;
	Camera->ZFar = 10000.0f;
	// END - Far clip plane - paradoxnj MODIFIED 3/9/2005

	return Camera;
}

//=====================================================================================
//	jeCamera_Destroy
//=====================================================================================
JETAPI void JETCC jeCamera_Destroy(jeCamera **pCamera)
{
	assert( pCamera  != NULL );
	if ( *pCamera )
	{
	jeCamera * Camera;
		Camera = *pCamera;

		if ( Camera->XFormStack )
		{
		jeXForm3d * pXF;
			while( pXF = (jeXForm3d *)Stack_Pop(Camera->XFormStack) )
			{
				jeRam_Free(pXF);
			}
			Stack_Destroy(Camera->XFormStack);
		}

		jeRam_Free(Camera);
	}
	*pCamera = NULL;
}

//
//	Camera XForm's
//

//=====================================================================================
//	jeCamera_PushXForm
//=====================================================================================
JETAPI jeBoolean JETCC jeCamera_PushXForm(jeCamera *Camera)
{
jeXForm3d * pXF;
	assert( Camera );

	if ( ! List_Start() )
		return JE_FALSE;

	if ( ! Camera->XFormStack )
		if ( ! (Camera->XFormStack = Stack_Create()) )
			return JE_FALSE;

	pXF = (jeXForm3d *)jeRam_Allocate(sizeof(jeXForm3d)*2);
	if ( ! pXF )
		return JE_FALSE;
	pXF[0] = Camera->TransposeXForm;
	pXF[1] = Camera->XForm;
	Stack_Push(Camera->XFormStack,(void *)pXF);

return JE_TRUE;
}

//=====================================================================================
//	jeCamera_PopXForm
//=====================================================================================
JETAPI jeBoolean JETCC jeCamera_PopXForm( jeCamera *Camera)
{
jeXForm3d * pXF;
	assert( Camera );

	if ( ! Camera->XFormStack )
		return JE_FALSE;

	pXF = (jeXForm3d *)Stack_Pop(Camera->XFormStack);
	if ( ! pXF )
		return JE_FALSE;

	Camera->TransposeXForm = pXF[0];
	Camera->XForm = pXF[1];
	jeRam_Free(pXF);

	Camera->Pov = Camera->TransposeXForm.Translation;

	List_Stop();
return JE_TRUE;
}

//========================================================================================
//	jeCamera_SetTransposeXForm
//========================================================================================
JETAPI jeBoolean JETCC jeCamera_SetTransposeXForm(jeCamera *Camera, const jeXForm3d *XForm)
{
	assert(Camera != NULL);
	assert(XForm != NULL);

	Camera->XForm = *XForm;		// Make a copy of the model XForm

	// Convert the model transform into a camera xform...
	if ( jeXForm3d_IsOrthogonal(XForm) )
	{
		jeXForm3d_GetTranspose(XForm, &Camera->TransposeXForm);
	}
	else
	{
		jeXForm3d_GetInverse(XForm, &Camera->TransposeXForm);
	}

	Camera->Pov = Camera->TransposeXForm.Translation;

	return JE_TRUE;
}

//========================================================================================
//	jeCamera_SetXForm
//========================================================================================
JETAPI jeBoolean JETCC jeCamera_SetXForm(jeCamera *Camera, const jeXForm3d *XForm)
{
	assert(Camera != NULL);
	assert(XForm != NULL);

	Camera->TransposeXForm = *XForm;		// Make a copy of the model XForm
	
	// Convert the model transform into a camera xform...
	jeXForm3d_GetTranspose(XForm, &Camera->XForm);

	Camera->Pov = XForm->Translation;

	return JE_TRUE;
}

//========================================================================================
//	jeCamera_GetXForm
// GetXForm returns the same thing passed to _SetXForm
//========================================================================================
JETAPI void JETCC jeCamera_GetXForm( const jeCamera *Camera,jeXForm3d *pXForm)
{
	assert(Camera && pXForm);
	*pXForm = Camera->TransposeXForm;
}

//========================================================================================
//	jeCamera_GetTransposeXForm
//========================================================================================
JETAPI void JETCC jeCamera_GetTransposeXForm( const jeCamera *Camera,jeXForm3d *pXForm)
{
	assert(Camera && pXForm);
	*pXForm = Camera->XForm;
}

//========================================================================================
//	jeCamera_XForm
//========================================================================================
const jeXForm3d * JETCF jeCamera_XForm( const jeCamera *Camera)
{
	assert(Camera != NULL);
	return &(Camera->XForm);
}

//========================================================================================
//	jeCamera_WorldXForm
//========================================================================================
const jeXForm3d * JETCF jeCamera_WorldXForm( const jeCamera *Camera)
{
	assert(Camera != NULL);
	return &(Camera->TransposeXForm);
}

//
//	Misc Get/Set
//

//=====================================================================================
//	jeCamera_GetClippingRect
//=====================================================================================
JETAPI void JETCC jeCamera_GetClippingRect(const jeCamera *Camera, jeRect *Rect)
{
	assert( Camera != NULL );
	assert( Rect != NULL );
	Rect->Left   = (int32)Camera->Left;
	Rect->Right  = (int32)Camera->Right;
	Rect->Top    = (int32)Camera->Top;
	Rect->Bottom = (int32)Camera->Bottom;
}

//=====================================================================================
//	jeCamera_GetPov
//=====================================================================================
const jeVec3d *JETCF jeCamera_GetPov(const jeCamera *Camera)
{
	assert( Camera != NULL );
	return &(Camera->Pov);
}

JETAPI jeVec3d *JETCC jeCamera_GetPov2(jeCamera *Camera)
{
	assert( Camera != NULL );
	return &(Camera->Pov);
}

//=====================================================================================
//	jeCamera_GetWidthHeight
//=====================================================================================
void JETCF jeCamera_GetWidthHeight(const jeCamera *Camera,jeFloat *Width,jeFloat *Height)
{
	assert( Width  != NULL );
	assert( Height != NULL );
	assert( Camera != NULL );

	*Width  = Camera->Width;
	*Height = Camera->Height;
}
		
//=====================================================================================
//	jeCamera_GetScale
//=====================================================================================
float JETCF jeCamera_GetScale(const jeCamera *Camera)
{
	assert( Camera != NULL );

	return Camera->Scale;
}


//=====================================================================================
//	jeCamera_GetAttributes - Added by Jeff  02/09/05
//  Returns camera's FOV and Rect
//=====================================================================================
JETAPI void JETCC jeCamera_GetAttributes(jeCamera *Camera, jeFloat *FovRadians, jeRect *Rect)
{

	assert( Camera != NULL );
	assert( Rect != NULL );
	assert( FovRadians != NULL);

	*FovRadians = Camera->FovRadians;

	Rect->Left   = (int32)Camera->Left;
	Rect->Right  = (int32)Camera->Right;
	Rect->Top    = (int32)Camera->Top;
	Rect->Bottom = (int32)Camera->Bottom;
}

//=====================================================================================
//	jeCamera_SetAttributes
//=====================================================================================
JETAPI void JETCC jeCamera_SetAttributes(jeCamera *Camera, jeFloat FovRadians, const jeRect *Rect)
{
	jeFloat	Width, Height;
	jeFloat	XRatio,YRatio;	
	jeFloat Fov;

	assert (Camera != NULL);
	assert (Rect != NULL);
	assert ( FovRadians > 0.0f && FovRadians < JE_PI );

	Width  = (jeFloat)(Rect->Right - Rect->Left); //+1.0f;
	Height = (jeFloat)(Rect->Bottom - Rect->Top); //+1.0f;

	assert( Width > 0.0f  );
	assert( Height > 0.0f );

#define TOO_SMALL (0.0001f)		// width and Fov must be >= TOO_SMALL

	if (Width <=0.0f)
		Width = TOO_SMALL;				// Just in case
	if (Height <=0.0f)
		Height = TOO_SMALL;				// Just in case

	Camera->Width   = Width;
	Camera->Height  = Height;
	
	Camera->FovRadians	= FovRadians;

	Fov = 2.0f / (float)tan(FovRadians*0.5f);

	XRatio  = Width  / Fov;
	YRatio  = Height / Fov;
	
	Camera->Scale   = max(XRatio, YRatio);
	//Camera->YScale = Camera->XScale;


	Camera->Left    = (jeFloat)Rect->Left;
	Camera->Right   = (jeFloat)Rect->Right; // Jeff: removed -1
	Camera->Top     = (jeFloat)Rect->Top;
	Camera->Bottom  = (jeFloat)Rect->Bottom;  // Jeff: removed -1

	Camera->XCenter = Camera->Left + ( Width  * 0.5f ) - 0.5f;
	Camera->YCenter = Camera->Top  + ( Height * 0.5f ) - 0.5f;

/******

When we project to screen space, we scale up camera coords by multiplying 
by Scale. That means the maximum camera coord is

	 X = (Width * Z / Scale)
	 Y = (Height* Z / Scale)


*********/

	{
	double AngleX,AngleY;

		/**

		if Width > Height (as usual)

		then AngleX =  FovRadians/2

		and AngleY < AngleX

		**/

		AngleX =  atan(2.0f * Camera->Scale / Width);
		Camera->CosViewAngleX = (jeFloat)cos(AngleX);
		Camera->SinViewAngleX = (jeFloat)sin(AngleX);

		AngleY =  atan(2.0f * Camera->Scale / Height);
		Camera->CosViewAngleY = (jeFloat)cos(AngleY);
		Camera->SinViewAngleY = (jeFloat)sin(AngleY);
	}
}

//=====================================================================================
//	jeCamera_SetZScale
//=====================================================================================
JETAPI void JETCC jeCamera_SetZScale(jeCamera *Camera, jeFloat ZScale)
{
	assert(Camera);
	Camera->ZScale = ZScale;
}

//=====================================================================================
//	jeCamera_GetZScale
//=====================================================================================
JETAPI jeFloat JETCC jeCamera_GetZScale(const jeCamera *Camera)
{
	assert(Camera);
	return Camera->ZScale;
}

// BEGIN - Far clip plane - paradoxnj 2/9/2005
//=====================================================================================
//	jeCamera_SetFarClipPlane
//=====================================================================================
JETAPI void JETCC jeCamera_SetFarClipPlane(jeCamera *Camera, jeBoolean Enable, jeFloat ZFar)
{
	assert(Camera != NULL);

	Camera->ZFarEnable = Enable;
	Camera->ZFar = ZFar;
}

//=====================================================================================
//	jeCamera_GetFarClipPlane
//=====================================================================================
JETAPI void JETCC jeCamera_GetFarClipPlane(const jeCamera *Camera, jeBoolean *Enable, jeFloat *ZFar)
{
	assert(Camera != NULL);

	*Enable = Camera->ZFarEnable;
	*ZFar = Camera->ZFar;
}
// END - Far clip plane - paradoxnj 2/9/2005

//
//	Camera Transform/Project
//

//========================================================================================
//	jeCamera_ScreenPointToWorld
//========================================================================================
JETAPI void JETCC jeCamera_ScreenPointToWorld(	const jeCamera	*Camera,
														int32			 ScreenX,
														int32			 ScreenY,
														jeVec3d			*Vector)
// Takes a screen X and Y pair, and a camera and generates a vector pointing
// in the direction from the camera position to the screen point.
{
	jeVec3d In,Left,Up;
	jeVec3d ScaledIn,ScaledLeft,ScaledUp ;
	float	XCenter ;
	float	YCenter ;
	float	Scale ;
	const jeXForm3d *pM;

	pM = &(Camera->TransposeXForm);
	XCenter = Camera->XCenter ;
	YCenter = Camera->YCenter ;
	Scale   = Camera->Scale ;

	jeXForm3d_GetIn( pM, &In ) ;
	jeXForm3d_GetLeft( pM, &Left ) ;
	jeXForm3d_GetUp( pM, &Up ) ;
	
	jeVec3d_Scale(&In,   Scale, &ScaledIn);
	jeVec3d_Scale(&Left, XCenter - ((jeFloat)ScreenX), &ScaledLeft );
	jeVec3d_Scale(&Up,   YCenter - ((jeFloat)ScreenY), &ScaledUp   );

	jeVec3d_Copy(&ScaledIn, Vector);
	jeVec3d_Add(Vector,		&ScaledLeft,	Vector );
	jeVec3d_Add(Vector,		&ScaledUp,		Vector );
	jeVec3d_Normalize(Vector);
}


//========================================================================================
//	jeCamera_Project
//========================================================================================
JETAPI void JETCC jeCamera_Project(	const jeCamera	*Camera, 
											const jeVec3d	*PointInCameraSpace, 
											jeVec3d			*ProjectedPoint)
	// project from camera space to projected space
	// projected space is not right-handed.
	// projection is onto x-y plane  x is right, y is down, z is in
{
	jeFloat Z;

	assert( Camera != NULL );
	assert( PointInCameraSpace != NULL );
	assert( ProjectedPoint != NULL );

	Z = -PointInCameraSpace->Z;   

	Z = max(Z,CAMERA_MINIMUM_PROJECTION_DISTANCE);

	ProjectedPoint->Z = Z*Camera->ZScale;

	Z = Camera->Scale / Z;

	ProjectedPoint->X = Camera->XCenter + ( PointInCameraSpace->X * Z );
	ProjectedPoint->Y = Camera->YCenter - ( PointInCameraSpace->Y * Z );
}

//========================================================================================
//	jeCamera_ProjectArray
//========================================================================================
JETAPI void JETCC jeCamera_ProjectArray(const jeCamera	*Camera, 
												const jeVec3d	*FmPoints, 
												int32			FmStride,
												jeVec3d			*ToPoints, 
												int32			ToStride, 
												int32			Count)
{
float Scale,XCenter,YCenter;
float Z,ZScale;

	assert( Camera != NULL );
	assert( FmPoints != NULL );
	assert( ToPoints != NULL );

	Scale = Camera->Scale;
	ZScale = Camera->ZScale;
	XCenter = Camera->XCenter;
	YCenter = Camera->YCenter;

	if ( Count & 1 )	// catch the odd one
	{
		Z = - FmPoints->Z;  
		Z = max(Z,CAMERA_MINIMUM_PROJECTION_DISTANCE);
		ToPoints->Z = Z * ZScale;
		Z = Scale / Z;
		ToPoints->X = XCenter + ( FmPoints->X * Z );
		ToPoints->Y = YCenter - ( FmPoints->Y * Z );
		FmPoints = (const jeVec3d *)(((uint32)FmPoints) + FmStride);
		ToPoints = (      jeVec3d *)(((uint32)ToPoints) + ToStride);
	}

	Count >>= 1;	// do two at a time!

	while(Count--)
	{

		// ProjectArray is a bottleneck!
		#pragma message("Camera : ProjectArray needs assembly!")
		// this is currently taking as much time as the XFormArray,
		//	but we have no fancy assembly versions of this!!

		Z = - FmPoints->Z;   

		// use FCMOV!! Critical !!
		Z = max(Z,CAMERA_MINIMUM_PROJECTION_DISTANCE);

		ToPoints->Z = Z * ZScale;

		Z = Scale / Z;

		// parrallelize these!
		//	the optimizer doesn't do it!
		ToPoints->X = XCenter + ( FmPoints->X * Z );
		ToPoints->Y = YCenter - ( FmPoints->Y * Z );

		FmPoints = (const jeVec3d *)(((uint32)FmPoints) + FmStride);
		ToPoints = (      jeVec3d *)(((uint32)ToPoints) + ToStride);
		
		// AND AGAIN:

		Z = - FmPoints->Z;   
		Z = max(Z,CAMERA_MINIMUM_PROJECTION_DISTANCE);
		ToPoints->Z = Z * ZScale;
		Z = Scale / Z;
		ToPoints->X = XCenter + ( FmPoints->X * Z );
		ToPoints->Y = YCenter - ( FmPoints->Y * Z );
		FmPoints = (const jeVec3d *)(((uint32)FmPoints) + FmStride);
		ToPoints = (      jeVec3d *)(((uint32)ToPoints) + ToStride);
	}
}

//========================================================================================
//	jeCamera_ProjectAndClampArray
//========================================================================================
JETAPI void JETCC jeCamera_ProjectAndClampArray(const jeCamera	*Camera, 
														const jeVec3d	*FmPoints, 
														int32			FmStride,
														jeVec3d			*ToPoints, 
														int32			ToStride, 
														int32			Count)
{
float Scale,XCenter,YCenter;
float X,Y,Z,ZScale;

	assert( Camera != NULL );
	assert( FmPoints != NULL );
	assert( ToPoints != NULL );

	Scale = Camera->Scale;
	ZScale = Camera->ZScale;
	XCenter = Camera->XCenter;
	YCenter = Camera->YCenter;

	if ( Count & 1 )	// catch the odd one
	{
		Z = - FmPoints->Z;  
		Z = max(Z,CAMERA_MINIMUM_PROJECTION_DISTANCE);
		ToPoints->Z = Z * ZScale;
		Z = Scale / Z;

		X = XCenter + ( FmPoints->X * Z );
		ToPoints->X = JE_CLAMP(X,Camera->Left,Camera->Right);
		Y = YCenter - ( FmPoints->Y * Z );
		ToPoints->Y = JE_CLAMP(Y,Camera->Top,Camera->Bottom);

		FmPoints = (const jeVec3d *)(((uint32)FmPoints) + FmStride);
		ToPoints = (      jeVec3d *)(((uint32)ToPoints) + ToStride);
	}

	Count >>= 1;	// do two at a time!

	// see optimize notes in ProjectArray

	while(Count--)
	{
		Z = - FmPoints->Z;   

		Z = max(Z,CAMERA_MINIMUM_PROJECTION_DISTANCE);

		ToPoints->Z = Z * ZScale;

		Z = Scale / Z;

		X = XCenter + ( FmPoints->X * Z );
		ToPoints->X = JE_CLAMP(X,Camera->Left,Camera->Right);

		Y = YCenter - ( FmPoints->Y * Z );
		ToPoints->Y = JE_CLAMP(Y,Camera->Top,Camera->Bottom);

		FmPoints = (const jeVec3d *)(((uint32)FmPoints) + FmStride);
		ToPoints = (      jeVec3d *)(((uint32)ToPoints) + ToStride);
		
		// AND AGAIN:

		Z = - FmPoints->Z;   
		Z = max(Z,CAMERA_MINIMUM_PROJECTION_DISTANCE);
		ToPoints->Z = Z * ZScale;
		Z = Scale / Z;
		X = XCenter + ( FmPoints->X * Z );
		ToPoints->X = JE_CLAMP(X,Camera->Left,Camera->Right);
		Y = YCenter - ( FmPoints->Y * Z );
		ToPoints->Y = JE_CLAMP(Y,Camera->Top,Camera->Bottom);
		FmPoints = (const jeVec3d *)(((uint32)FmPoints) + FmStride);
		ToPoints = (      jeVec3d *)(((uint32)ToPoints) + ToStride);
	}
}
//========================================================================================
//	jeCamera_ProjectZ
//========================================================================================
JETAPI void JETCC jeCamera_ProjectZ(const jeCamera	*Camera, 
											const jeVec3d	*PointInCameraSpace, 
											jeVec3d			*ProjectedPoint)
	// project from camera space to projected space
	// projected space is not right-handed.
	// projection is onto x-y plane  x is right, y is down, z is in
	// projected point.z is set to 1/Z
{
	jeFloat OneOverZ;
	jeFloat ScaleOverZ;
	jeFloat Z;
	assert( Camera != NULL );
	assert( PointInCameraSpace != NULL );
	assert( ProjectedPoint != NULL );

	Z = -PointInCameraSpace->Z;   
	Z = max(Z,CAMERA_MINIMUM_PROJECTION_DISTANCE);

	OneOverZ = 1.0f / Z;
	ScaleOverZ = Camera->Scale *  (OneOverZ);

	ProjectedPoint->Z = Camera->ZScale*OneOverZ;   

	ProjectedPoint->X = ( PointInCameraSpace->X * ScaleOverZ ) + Camera->XCenter;
	
	ProjectedPoint->Y = Camera->YCenter - ( PointInCameraSpace->Y * ScaleOverZ );
}




//========================================================================================
//	jeCamera_ProjectAndClamp
//========================================================================================
JETAPI void JETCC jeCamera_ProjectAndClamp(const jeCamera	*Camera, 
										const jeVec3d	*PointInCameraSpace, 
										jeVec3d			*ProjectedPoint)
	// project from camera space to projected space
	// projected space is not right-handed.
	// projection is onto x-y plane  x is right, y is down, z is in
	// points outside the clipping rect are clamped to the clipping rect
{
	jeFloat ScaleOverZ;
	jeFloat X,Y,Z;
	assert( Camera != NULL );
	assert( PointInCameraSpace != NULL );
	assert( ProjectedPoint != NULL );

	Z = -PointInCameraSpace->Z;   

	if (Z < CAMERA_MINIMUM_PROJECTION_DISTANCE)
	{
		Z = CAMERA_MINIMUM_PROJECTION_DISTANCE; 
	}

	ScaleOverZ = Camera->Scale / Z;

	ProjectedPoint->Z = Z*Camera->ZScale;   

	X = ( PointInCameraSpace->X * ScaleOverZ ) + Camera->XCenter;
	
	ProjectedPoint->X = JE_CLAMP(X,Camera->Left,Camera->Right);
	
	Y = Camera->YCenter - ( PointInCameraSpace->Y * ScaleOverZ );

	ProjectedPoint->Y = JE_CLAMP(Y,Camera->Top,Camera->Bottom);
}

//========================================================================================
//	jeCamera_GetViewAngleXSinCos
//========================================================================================
void JETCF jeCamera_GetViewAngleXSinCos( const jeCamera *Camera, jeFloat *SinAngle, jeFloat *CosAngle )
{
	assert( Camera != NULL );
	assert( SinAngle );
	assert( CosAngle );
	*SinAngle = Camera->SinViewAngleX;
	*CosAngle = Camera->CosViewAngleX;
}

//========================================================================================
//	jeCamera_GetViewAngleYSinCos
//========================================================================================
void JETCF jeCamera_GetViewAngleYSinCos( const jeCamera *Camera, jeFloat *SinAngle, jeFloat *CosAngle )
{
	assert( Camera != NULL );
	assert( SinAngle );
	assert( CosAngle );
	*SinAngle = Camera->SinViewAngleY;
	*CosAngle = Camera->CosViewAngleY;
}

//========================================================================================
//	jeCamera_Transform
//========================================================================================
JETAPI void JETCC jeCamera_Transform(	const jeCamera	*Camera, 
												const jeVec3d	*WorldSpacePoint, 
												jeVec3d			*CameraSpacePoint)
{
	assert( Camera );
	assert( WorldSpacePoint );
	assert( CameraSpacePoint );

	// would be better if xform3d_transform was assembly, or a macro, or anything

	jeXForm3d_Transform(&(Camera->XForm),WorldSpacePoint,CameraSpacePoint);
}


//========================================================================================
//	jeCamera_TransformVecArray
//========================================================================================
JETAPI void JETCC jeCamera_TransformVecArray(	const jeCamera	*Camera, 
														const jeVec3d	*WorldSpacePointPtr, 
														jeVec3d			*CameraSpacePointPtr,
														int32			Count)
{
	assert( Camera );
	assert( WorldSpacePointPtr );
	assert( CameraSpacePointPtr );

	jeXForm3d_TransformVecArray(&(Camera->XForm), WorldSpacePointPtr,CameraSpacePointPtr,Count);
}

//========================================================================================
//	jeCamera_TransformAndProjectVecArray
//========================================================================================
JETAPI void JETCC jeCamera_TransformAndProjectVecArray(	const jeCamera *Camera, 
																const jeVec3d *WorldSpacePointPtr, 
																jeVec3d *ProjectedSpacePointPtr,
																int32 Count)
{
	jeCamera_TransformAndProjectArray(	Camera, 
										WorldSpacePointPtr, 
										sizeof(jeVec3d),
										ProjectedSpacePointPtr,
										sizeof(jeVec3d),
										Count);
}

//========================================================================================
//	jeCamera_TransformAndProjectArray
//========================================================================================

#if 1	// <> use the assembly XFormArray
		// can't tell the difference!

JETAPI void JETCC jeCamera_TransformAndProjectArray(const jeCamera	*Camera, 
															const jeVec3d	*WorldSpacePointPtr, 
															int32			WorldStride,
															jeVec3d			*ProjectedSpacePointPtr, 
															int32			ProjectedStride,
															int32			Count)
{
	assert( Camera );
	assert( WorldSpacePointPtr );
	assert( ProjectedSpacePointPtr );

	jeXForm3d_TransformArray(	&(Camera->XForm),
								WorldSpacePointPtr, WorldStride,
								ProjectedSpacePointPtr,ProjectedStride,
								Count);

	jeCamera_ProjectArray(	Camera,
							ProjectedSpacePointPtr,
							ProjectedStride,
							ProjectedSpacePointPtr,
							ProjectedStride, 
							Count);
}

//========================================================================================================
//	jeCamera_TransformAndProjectAndClampArray
//========================================================================================================
JETAPI void JETCC jeCamera_TransformAndProjectAndClampArray(const	jeCamera	*Camera, 
															const jeVec3d	*WorldSpacePointPtr, 
															int32			WorldStride,
															jeVec3d			*ProjectedSpacePointPtr, 
															int32			ProjectedStride,
															int32			Count)
{
	assert( Camera );
	assert( WorldSpacePointPtr );
	assert( ProjectedSpacePointPtr );

	jeXForm3d_TransformArray(	&(Camera->XForm),
								WorldSpacePointPtr, WorldStride,
								ProjectedSpacePointPtr,ProjectedStride,
								Count);

	jeCamera_ProjectAndClampArray(	Camera,
							ProjectedSpacePointPtr,
							ProjectedStride,
							ProjectedSpacePointPtr,
							ProjectedStride, 
							Count);
}
#else // let the compiler do its best

//========================================================================================================
//	jeCamera_TransformAndProjectArray
//========================================================================================================
JETAPI void JETCC jeCamera_TransformAndProjectArray(const jeCamera	*Camera, 
															const jeVec3d	*InFmPoints, 
															int32			FmStride,
															jeVec3d			*InToPoints, 
															int32			ToStride,
															int32			Count)
{
float Scale,ZScale,XCenter,YCenter;
jeXForm3d XF;
const jeVec3d	*NextFmPoints,*FmPoints; 
jeVec3d			*NextToPoints,*ToPoints;
int c;

	assert( Camera );
	assert( FmPoints );
	assert( ToPoints );

	Scale   = Camera->Scale;
	ZScale  = Camera->ZScale;
	XCenter = Camera->XCenter;
	YCenter = Camera->YCenter;
	XF		= Camera->XForm;
	FmPoints= InFmPoints;
	ToPoints= InToPoints;

	c = Count;
	while(c--)
	{
	jeVec3d Point;
	float Z;

		NextFmPoints = (const jeVec3d *)(((uint32)FmPoints) + FmStride);
		NextToPoints = (      jeVec3d *)(((uint32)ToPoints) + ToStride);
		// prefetch !

		{
		float X,Y,Z;
			X = FmPoints->X;
			Y = FmPoints->Y;
			Z = FmPoints->Z;
			Point.X = (X * XF.AX) + (Y * XF.AY) + (Z * XF.AZ) + XF.Translation.X;
			Point.Y = (X * XF.BX) + (Y * XF.BY) + (Z * XF.BZ) + XF.Translation.Y;
			Point.Z = (X * XF.CX) + (Y * XF.CY) + (Z * XF.CZ) + XF.Translation.Z;
		}

		Z = - Point.Z;   

		Z = max(Z,CAMERA_MINIMUM_PROJECTION_DISTANCE);

		ToPoints->Z = Z * ZScale;

		Z = Scale / Z;

		ToPoints->X = XCenter + ( Point.X * Z );
		ToPoints->Y = YCenter - ( Point.Y * Z );

		FmPoints = NextFmPoints;
		ToPoints = NextToPoints;
	}
}

//========================================================================================================
//	jeCamera_TransformAndProjectAndClampArray
//========================================================================================================
JETAPI void JETCC jeCamera_TransformAndProjectAndClampArray(const jeCamera	*Camera, 
																	const jeVec3d	*InFmPoints, 
																	int32			FmStride,
																	jeVec3d			*InToPoints, 
																	int32			ToStride,
																	int32			Count)
{
float Scale,ZScale,XCenter,YCenter;
jeXForm3d XF;
const jeVec3d	*NextFmPoints,*FmPoints; 
jeVec3d			*NextToPoints,*ToPoints;
int c;

	assert( Camera );
	assert( FmPoints );
	assert( ToPoints );

	Scale   = Camera->Scale;
	ZScale  = Camera->ZScale;
	XCenter = Camera->XCenter;
	YCenter = Camera->YCenter;
	XF		= Camera->XForm;
	FmPoints= InFmPoints;
	ToPoints= InToPoints;

	c = Count;
	while(c--)
	{
	jeVec3d Point;
	float X,Y,Z;

		NextFmPoints = (const jeVec3d *)(((uint32)FmPoints) + FmStride);
		NextToPoints = (      jeVec3d *)(((uint32)ToPoints) + ToStride);
		// prefetch !

		{
		float X,Y,Z;
			X = FmPoints->X;
			Y = FmPoints->Y;
			Z = FmPoints->Z;
			Point.X = (X * XF.AX) + (Y * XF.AY) + (Z * XF.AZ) + XF.Translation.X;
			Point.Y = (X * XF.BX) + (Y * XF.BY) + (Z * XF.BZ) + XF.Translation.Y;
			Point.Z = (X * XF.CX) + (Y * XF.CY) + (Z * XF.CZ) + XF.Translation.Z;
		}

		Z = - Point.Z;   

		Z = max(Z,CAMERA_MINIMUM_PROJECTION_DISTANCE);

		ToPoints->Z = Z * ZScale;

		Z = Scale / Z;

		X = XCenter + ( Point.X * Z );
		Y = YCenter - ( Point.Y * Z );

		ToPoints->X = JE_CLAMP(X,Camera->Left,Camera->Right);
		ToPoints->Y = JE_CLAMP(Y,Camera->Top,Camera->Bottom);

		FmPoints = NextFmPoints;
		ToPoints = NextToPoints;
	}
}

#endif

//========================================================================================
//	jeCamera_TransformAndProjectLArray
//========================================================================================
JETAPI void JETCC jeCamera_TransformAndProjectLArray(	const jeCamera	*Camera, 
																const jeLVertex	*WorldSpacePointPtr, 
																jeTLVertex		*ProjectedSpacePointPtr,
																int32			Count)
{
	assert( Camera );
	assert( WorldSpacePointPtr );
	assert( ProjectedSpacePointPtr );

	while(Count--)
	{
		jeCamera_TransformAndProjectL(Camera,WorldSpacePointPtr++,ProjectedSpacePointPtr++);
	}
}

//========================================================================================
//	jeCamera_TransformLArray
//	Tansforms a point to camera space
//========================================================================================
JETAPI void JETCC jeCamera_TransformLArray(	const jeCamera	*Camera, 
													const jeLVertex	*WorldSpacePointPtr, 
													jeLVertex		*CameraSpacePointPtr,
													int32			Count)
{
	assert( Camera );
	assert( WorldSpacePointPtr );
	assert( CameraSpacePointPtr );

	while(Count--)
	{
		jeCamera_TransformL(Camera, WorldSpacePointPtr++, CameraSpacePointPtr++);
	}
}

//========================================================================================
//	jeCamera_ProjectAndClampLArray
//	Take a CameraSpace point array, and projects them flat onto the camera plane
//========================================================================================
JETAPI void JETCC jeCamera_ProjectAndClampLArray(	const jeCamera		*Camera, 
															const jeLVertex		*CameraSpacePointPtr, 
															jeTLVertex			*ProjectedSpacePointPtr,
															int32				Count)
{
	assert( Camera );
	assert( CameraSpacePointPtr );
	assert( ProjectedSpacePointPtr );

	while(Count--)
	{
		jeCamera_ProjectAndClampL(Camera, CameraSpacePointPtr++,ProjectedSpacePointPtr++);
	}
}

//========================================================================================
//	jeCamera_TransformAndProjectAndClampLArray
//========================================================================================
JETAPI void JETCC jeCamera_TransformAndProjectAndClampLArray(	const jeCamera		*Camera, 
																		const jeLVertex		*WorldSpacePointPtr, 
																		jeTLVertex			*ProjectedSpacePointPtr,
																		int32				Count)
{
	assert( Camera );
	assert( WorldSpacePointPtr );
	assert( ProjectedSpacePointPtr );

	while(Count--)
	{
		jeCamera_TransformAndProjectAndClampL(Camera,WorldSpacePointPtr++,ProjectedSpacePointPtr++);
	}
}

//============================================================================================
//	jeCamera_TransformAndProject
//============================================================================================
JETAPI void JETCC jeCamera_TransformAndProject(	const	jeCamera *Camera,
														const	jeVec3d *Point, 
														jeVec3d	*ProjectedPoint)
	// project from *WORLD* space to projected space
	// projected space is not right-handed.
	// projection is onto x-y plane  x is right, y is down, z is in
{
	jeFloat Z;

	assert( Camera );
	assert( Point );
	assert( ProjectedPoint );

	jeXForm3d_Transform(&(Camera->XForm),Point,ProjectedPoint);

	Z = - ProjectedPoint->Z;

	Z = max(Z,CAMERA_MINIMUM_PROJECTION_DISTANCE);

	ProjectedPoint->Z = Z*Camera->ZScale;

	Z = Camera->Scale / Z;

	ProjectedPoint->X =   ( ProjectedPoint->X * Z ) + Camera->XCenter;
	ProjectedPoint->Y = - ( ProjectedPoint->Y * Z ) + Camera->YCenter;
}

JETAPI void JETCC jeCamera_TransformAndProjectAndClamp(	const	jeCamera *Camera,
																const	jeVec3d *Point, 
																jeVec3d	*ProjectedPoint)
	// project from *WORLD* space to projected space
	// projected space is not right-handed.
	// projection is onto x-y plane  x is right, y is down, z is in
{
	jeFloat X,Y,Z;

	assert( Camera );
	assert( Point );
	assert( ProjectedPoint );

	jeXForm3d_Transform(&(Camera->XForm),Point,ProjectedPoint);

	Z = - ProjectedPoint->Z;

	Z = max(Z,CAMERA_MINIMUM_PROJECTION_DISTANCE);

	ProjectedPoint->Z = Z*Camera->ZScale;

	Z = Camera->Scale / Z;

	X =   ( ProjectedPoint->X * Z ) + Camera->XCenter;

	ProjectedPoint->X = JE_CLAMP(X,Camera->Left,Camera->Right);

	Y = - ( ProjectedPoint->Y * Z ) + Camera->YCenter;
		
	ProjectedPoint->Y = JE_CLAMP(Y,Camera->Top,Camera->Bottom);
}


//============================================================================================
//	jeCamera_TransformAndProjectL
//============================================================================================
JETAPI void JETCC jeCamera_TransformAndProjectL(const		jeCamera *Camera,
														const		jeLVertex *Point, 
														jeTLVertex	*ProjectedPoint)
	// project from *WORLD* space to projected space
	// projected space is not right-handed.
	// projection is onto x-y plane  x is right, y is down, z is in
{
	jeFloat ScaleOverZ;
	jeFloat Z;

	assert( Camera );
	assert( Point );
	assert( ProjectedPoint );

	jeXForm3d_Transform(&(Camera->XForm),(jeVec3d *)Point,(jeVec3d *)ProjectedPoint);

	Z = - ProjectedPoint->z;

	Z = max(Z,CAMERA_MINIMUM_PROJECTION_DISTANCE);

	ScaleOverZ = Camera->Scale / Z;

	ProjectedPoint->z = Z*Camera->ZScale;
	ProjectedPoint->x =   ( ProjectedPoint->x * ScaleOverZ ) + Camera->XCenter;
	ProjectedPoint->y = - ( ProjectedPoint->y * ScaleOverZ ) + Camera->YCenter;

	ProjectedPoint->u = Point->u;
	ProjectedPoint->v = Point->v;
	ProjectedPoint->r = Point->r;
	ProjectedPoint->g = Point->g;
	ProjectedPoint->b = Point->b;
	ProjectedPoint->a = Point->a;
}


//========================================================================================================
//	jeCamera_TransformL
//========================================================================================================
JETAPI void JETCC jeCamera_TransformL(	const jeCamera	*Camera,
												const jeLVertex *Point, 
												jeLVertex		*TransformedPoint)
{
	assert( Camera );
	assert( Point );
	assert( TransformedPoint );

	jeXForm3d_Transform(&(Camera->XForm),(jeVec3d *)Point,(jeVec3d *)TransformedPoint);

	// This kind of sucks, sigh...
	TransformedPoint->u = Point->u;
	TransformedPoint->v = Point->v;
	TransformedPoint->r = Point->r;
	TransformedPoint->g = Point->g;
	TransformedPoint->b = Point->b;
	TransformedPoint->a = Point->a;
}


//========================================================================================================
//	jeCamera_ProjectAndClampL
//========================================================================================================
JETAPI void JETCC jeCamera_ProjectAndClampL(const jeCamera	*Camera,
													const jeLVertex *Point, 
													jeTLVertex		*ProjectedPoint)
{
	jeFloat ScaleOverZ;
	jeFloat X,Y,Z;

	Z = - Point->Z;

	Z = max(Z,CAMERA_MINIMUM_PROJECTION_DISTANCE);

	ScaleOverZ = Camera->Scale / Z;

	ProjectedPoint->z = Z*Camera->ZScale;

	X =   ( Point->X * ScaleOverZ ) + Camera->XCenter;
	ProjectedPoint->x = JE_CLAMP(X,Camera->Left,Camera->Right);

	Y = - ( Point->Y * ScaleOverZ ) + Camera->YCenter;
	ProjectedPoint->y = JE_CLAMP(Y,Camera->Top,Camera->Bottom);

	ProjectedPoint->u = Point->u;
	ProjectedPoint->v = Point->v;
	ProjectedPoint->r = Point->r;
	ProjectedPoint->g = Point->g;
	ProjectedPoint->b = Point->b;
	ProjectedPoint->a = Point->a;
}

//========================================================================================================
//	jeCamera_TransformAndProjectAndClampL
//========================================================================================================
JETAPI void JETCC jeCamera_TransformAndProjectAndClampL(const		jeCamera *Camera,
																const		jeLVertex *Point, 
																jeTLVertex	*ProjectedPoint)
	// project from *WORLD* space to projected space
	// projected space is not right-handed.
	// projection is onto x-y plane  x is right, y is down, z is in
{
	jeFloat ScaleOverZ;
	jeFloat X,Y,Z;

	assert( Camera );
	assert( Point );
	assert( ProjectedPoint );

	jeXForm3d_Transform(&(Camera->XForm),(jeVec3d *)Point,(jeVec3d *)ProjectedPoint);

	Z = - ProjectedPoint->z;

	Z = max(Z,CAMERA_MINIMUM_PROJECTION_DISTANCE);

	ScaleOverZ = Camera->Scale / Z;

	ProjectedPoint->z = Z*Camera->ZScale;
	X =   ( ProjectedPoint->x * ScaleOverZ ) + Camera->XCenter;
	
	ProjectedPoint->x = JE_CLAMP(X,Camera->Left,Camera->Right);

	Y = - ( ProjectedPoint->y * ScaleOverZ ) + Camera->YCenter;
		
	ProjectedPoint->y = JE_CLAMP(Y,Camera->Top,Camera->Bottom);

	ProjectedPoint->u = Point->u;
	ProjectedPoint->v = Point->v;
	ProjectedPoint->r = Point->r;
	ProjectedPoint->g = Point->g;
	ProjectedPoint->b = Point->b;
	ProjectedPoint->a = Point->a;
}

