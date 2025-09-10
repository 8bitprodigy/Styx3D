/****************************************************************************************/
/*  BRUSHTEMPLATE.C                                                                     */
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
#include <Assert.h>
#include <Float.h>
#include <Memory.h>
#include <String.h>

#include "../Resource.h"
#include "ErrorLog.h"
#include "jet.h"
#include "Ram.h"
#include "Units.h"
#include "Util.h"

#include "BrushTemplate.h"

#define	BOX_DEFAULT_XSIZETOP	1.0f
#define	BOX_DEFAULT_XSIZEBOT	1.0f
#define	BOX_DEFAULT_YSIZE		1.0f
#define	BOX_DEFAULT_ZSIZETOP	1.0f
#define	BOX_DEFAULT_ZSIZEBOT	1.0f

#define	SPHERE_DEFAULT_HORZBANDS	4
#define	SPHERE_DEFAULT_VERTBANDS	8
#define	SPHERE_DEFAULT_RADIUS		0.5f

#define	CYLINDER_DEFAULT_BOTXOFFSET	0.0f
#define	CYLINDER_DEFAULT_BOTXSIZE	1.0f
#define	CYLINDER_DEFAULT_BOTZOFFSET	0.0f
#define	CYLINDER_DEFAULT_BOTZSIZE	1.0f
#define	CYLINDER_DEFAULT_TOPXOFFSET	0.0f
#define	CYLINDER_DEFAULT_TOPXSIZE	1.0f
#define	CYLINDER_DEFAULT_TOPZOFFSET	0.0f
#define	CYLINDER_DEFAULT_TOPZSIZE	1.0f
#define	CYLINDER_DEFAULT_VERTICALSTRIPES	8

enum {
	TEMPLATE_BOXXRATIO_FIELD = TEMPLATE_FIELD_START,
	TEMPLATE_BOXZRATIO_FIELD,
	TEMPLATE_HBANDS_FIELD,
	TEMPLATE_VBANDS_FIELD,
	TEMPLATE_CYLDXRATIO_FIELD,
	TEMPLATE_CYLDZRATIO_FIELD,
	TEMPLATE_STRIPES_FIELD,
	TEMPLATE_ARCH_STARTANGLE_FIELD,
	TEMPLATE_ARCH_ENDANGLE_FIELD,
	TEMPLATE_ARCH_THICKNESS_FIELD,
	TEMPLATE_ARCH_WIDTH_FIELD,
	TEMPLATE_ARCH_INNERRADIUS_FIELD,
	TEMPLATE_ARCH_HOLLOWWALLSIZE_FIELD,
	TEMPLATE_ARCH_NUMCROSSSECTIONS_FIELD,
	TEMPLATE_GROUP,
	TEMPLATE_GROUP_END
};

typedef struct BrushTemplate {
		BRUSH_KIND	Kind;
} BrushTemplate;

typedef struct BoxTemplate 
{
	BRUSH_KIND	Kind;
	jeFloat		XSizeTop;
	jeFloat		XSizeBot;
	jeFloat		YSize;
	jeFloat		ZSizeTop;
	jeFloat		ZSizeBot;
} BoxTemplate ;

typedef struct SheetTemplate 
{
	BRUSH_KIND	Kind;
	jeFloat		XSize;
	jeFloat		ZSize;
} SheetTemplate ;

typedef struct SphereTemplate 
{
	BRUSH_KIND	Kind;
	int			HorizontalBands;
	int			VerticalBands;
	jeFloat		Radius;
} SphereTemplate ;

typedef struct CylinderTemplate 
{
	BRUSH_KIND	Kind;
	jeFloat		BotXOffset;
	jeFloat		BotXSize;
	jeFloat		BotZOffset;
	jeFloat		BotZSize;
	jeFloat		TopXOffset;
	jeFloat		TopXSize;
	jeFloat		TopZOffset;
	jeFloat		TopZSize;
	int			VerticalStripes;
	jeFloat		YSize;
}CylinderTemplate ;

typedef struct ArchTemplate
{
	BRUSH_KIND	Kind;
	int			NumSlits;
	jeFloat		Thickness;
	jeFloat		Width;
	jeFloat		Radius;
	jeFloat		WallSize;
	int			Style;
	jeFloat		EndAngle;
	jeFloat		StartAngle;
	jeBoolean	TCut;
} ArchTemplate;

//STATIC
static jeBoolean BrushTemplate_CreateFace( jeBrush * Brush, jeVec3d *Verts, int32 nVerts, jeFaceInfo * pFaceInfo)
{
	jeBrush_Face *Face;
	int i;

	assert( Brush );
	assert( Verts );

	Face = jeBrush_CreateFace(Brush, nVerts);
	if( Face == NULL )
	{
		jeErrorLog_Add( JE_ERR_INTERNAL_RESOURCE, "Unable to create brush face." );
		return(JE_FALSE );
	}
	for( i = 0; i < nVerts ; i++)
		jeBrush_FaceSetVertByIndex(Face, i, &Verts[i] );
	jeBrush_FaceSetFaceInfo(Face, pFaceInfo);
	return(JE_TRUE );
}

jeBrush *BrushTemplate_CreateBoxBrush (const BoxTemplate *pTemplate,  jeFaceInfo * pFaceInfo )
{
	//revisit for error handling when merged
	jeVec3d		Verts[8];
	jeVec3d		FaceVerts[4];
	jeBrush *	Brush;

	Brush = jeBrush_Create(6);
	if(Brush == NULL )
	{
		jeErrorLog_Add( JE_ERR_INTERNAL_RESOURCE, "Unable to create jeBrush" );
		return( NULL );
	}

	// Vertices 0 to 3 are the 4 corners of the top face
	jeVec3d_Set (&Verts[0], (float)-(pTemplate->XSizeTop/2), (float)(pTemplate->YSize/2), (float)-(pTemplate->ZSizeTop/2));
	jeVec3d_Set (&Verts[1], (float)-(pTemplate->XSizeTop/2), (float)(pTemplate->YSize/2), (float)(pTemplate->ZSizeTop/2));
	jeVec3d_Set (&Verts[2], (float)(pTemplate->XSizeTop/2), (float)(pTemplate->YSize/2), (float)(pTemplate->ZSizeTop/2));
	jeVec3d_Set (&Verts[3], (float)(pTemplate->XSizeTop/2), (float)(pTemplate->YSize/2), (float)-(pTemplate->ZSizeTop/2));

	// Vertices 4 to 7 are the 4 corners of the bottom face
	jeVec3d_Set (&Verts[4], (float)-(pTemplate->XSizeBot/2), (float)-(pTemplate->YSize/2), (float)-(pTemplate->ZSizeBot/2));
	jeVec3d_Set (&Verts[5], (float)(pTemplate->XSizeBot/2), (float)-(pTemplate->YSize/2), (float)-(pTemplate->ZSizeBot/2));
	jeVec3d_Set (&Verts[6], (float)(pTemplate->XSizeBot/2), (float)-(pTemplate->YSize/2), (float)(pTemplate->ZSizeBot/2));
	jeVec3d_Set (&Verts[7], (float)-(pTemplate->XSizeBot/2), (float)-(pTemplate->YSize/2), (float)(pTemplate->ZSizeBot/2));

	FaceVerts[3]	=Verts[0];
	FaceVerts[2]	=Verts[1];
	FaceVerts[1]	=Verts[2];
	FaceVerts[0]	=Verts[3];

	if( !BrushTemplate_CreateFace( Brush, FaceVerts, 4, pFaceInfo ) )
	{
		jeErrorLog_Add( JE_ERR_INTERNAL_RESOURCE, "Trace" );
		jeBrush_Destroy( &Brush);
	}

	FaceVerts[3]	=Verts[4];
	FaceVerts[2]	=Verts[5];
	FaceVerts[1]	=Verts[6];
	FaceVerts[0]	=Verts[7];

	if( !BrushTemplate_CreateFace( Brush, FaceVerts, 4, pFaceInfo ) )
	{
		jeErrorLog_Add( JE_ERR_INTERNAL_RESOURCE, "Trace" );
		jeBrush_Destroy( &Brush);
	}

	FaceVerts[3]	=Verts[1];
	FaceVerts[2]	=Verts[7];
	FaceVerts[1]	=Verts[6];
	FaceVerts[0]	=Verts[2];

	if( !BrushTemplate_CreateFace( Brush, FaceVerts, 4, pFaceInfo ) )
	{
		jeErrorLog_Add( JE_ERR_INTERNAL_RESOURCE, "Trace" );
		jeBrush_Destroy( &Brush);
	}

	FaceVerts[3]	=Verts[0];
	FaceVerts[2]	=Verts[3];
	FaceVerts[1]	=Verts[5];
	FaceVerts[0]	=Verts[4];

	if( !BrushTemplate_CreateFace( Brush, FaceVerts, 4, pFaceInfo ) )
	{
		jeErrorLog_Add( JE_ERR_INTERNAL_RESOURCE, "Trace" );
		jeBrush_Destroy( &Brush);
	}

	FaceVerts[3]	=Verts[0];
	FaceVerts[2]	=Verts[4];
	FaceVerts[1]	=Verts[7];
	FaceVerts[0]	=Verts[1];

	if( !BrushTemplate_CreateFace( Brush, FaceVerts, 4, pFaceInfo ) )
	{
		jeErrorLog_Add( JE_ERR_INTERNAL_RESOURCE, "Trace" );
		jeBrush_Destroy( &Brush);
	}

	FaceVerts[3]	=Verts[3];
	FaceVerts[2]	=Verts[2];
	FaceVerts[1]	=Verts[6];
	FaceVerts[0]	=Verts[5];

	if( !BrushTemplate_CreateFace( Brush, FaceVerts, 4, pFaceInfo ) )
	{
		jeErrorLog_Add( JE_ERR_INTERNAL_RESOURCE, "Trace" );
		jeBrush_Destroy( &Brush);
	}


	return	Brush;
}

jeBrush *BrushTemplate_CreateCameraBrush (int BoxSize  )
{
	//revisit for error handling when merged
	jeVec3d		Verts[16];
	jeVec3d		FaceVerts[4];
	jeBrush *	Brush;
	jeFaceInfo  FaceInfo;


	jeFaceInfo_SetDefaults( &FaceInfo );
	Brush = jeBrush_Create(11);
	if(Brush == NULL )
	{
		jeErrorLog_Add( JE_ERR_INTERNAL_RESOURCE, "Unable to create jeBrush" );
		return( NULL );
	}

	// Vertices 0 to 3 are the 4 corners of the top face
	jeVec3d_Set (&Verts[0], (float)-(BoxSize/2), (float)(BoxSize/2), (float)-(BoxSize/2));
	jeVec3d_Set (&Verts[1], (float)-(BoxSize/2), (float)(BoxSize/2), (float)(BoxSize/2));
	jeVec3d_Set (&Verts[2], (float)(BoxSize/2), (float)(BoxSize/2), (float)(BoxSize/2));
	jeVec3d_Set (&Verts[3], (float)(BoxSize/2), (float)(BoxSize/2), (float)-(BoxSize/2));

	// Vertices 4 to 7 are the 4 corners of the bottom face
	jeVec3d_Set (&Verts[4], (float)-(BoxSize/2), (float)-(BoxSize/2), (float)-(BoxSize/2));
	jeVec3d_Set (&Verts[5], (float)(BoxSize/2), (float)-(BoxSize/2), (float)-(BoxSize/2));
	jeVec3d_Set (&Verts[6], (float)(BoxSize/2), (float)-(BoxSize/2), (float)(BoxSize/2));
	jeVec3d_Set (&Verts[7], (float)-(BoxSize/2), (float)-(BoxSize/2), (float)(BoxSize/2));

	// Vertices 8 to 11 are the 4 corners of the Lens bottom
	jeVec3d_Set (&Verts[8], (float)-(BoxSize/4), (float)-(BoxSize/4), (float)-(BoxSize/2));
	jeVec3d_Set (&Verts[9], (float)-(BoxSize/4), (float)(BoxSize/4) , (float)-(BoxSize/2));
	jeVec3d_Set (&Verts[10], (float)(BoxSize/4), (float)(BoxSize/4) , (float)-(BoxSize/2));
	jeVec3d_Set (&Verts[11], (float)(BoxSize/4), (float)-(BoxSize/4), (float)-(BoxSize/2));

	// Vertices 12 to 11 are the 4 corners of the Lens top
	jeVec3d_Set (&Verts[12], (float)-(BoxSize/3), (float)-(BoxSize/3), (float)-(BoxSize));
	jeVec3d_Set (&Verts[13], (float)-(BoxSize/3), (float)(BoxSize/3) , (float)-(BoxSize));
	jeVec3d_Set (&Verts[14], (float)(BoxSize/3) , (float)(BoxSize/3) , (float)-(BoxSize) );
	jeVec3d_Set (&Verts[15], (float)(BoxSize/3) , (float)-(BoxSize/3), (float)-(BoxSize) );

	FaceVerts[3]	=Verts[0];
	FaceVerts[2]	=Verts[1];
	FaceVerts[1]	=Verts[2];
	FaceVerts[0]	=Verts[3];

	if( !BrushTemplate_CreateFace( Brush, FaceVerts, 4, &FaceInfo ) )
	{
		jeErrorLog_Add( JE_ERR_INTERNAL_RESOURCE, "Trace" );
		jeBrush_Destroy( &Brush);
	}

	FaceVerts[3]	=Verts[4];
	FaceVerts[2]	=Verts[5];
	FaceVerts[1]	=Verts[6];
	FaceVerts[0]	=Verts[7];

	if( !BrushTemplate_CreateFace( Brush, FaceVerts, 4, &FaceInfo ) )
	{
		jeErrorLog_Add( JE_ERR_INTERNAL_RESOURCE, "Trace" );
		jeBrush_Destroy( &Brush);
	}

	FaceVerts[3]	=Verts[1];
	FaceVerts[2]	=Verts[7];
	FaceVerts[1]	=Verts[6];
	FaceVerts[0]	=Verts[2];

	if( !BrushTemplate_CreateFace( Brush, FaceVerts, 4, &FaceInfo ) )
	{
		jeErrorLog_Add( JE_ERR_INTERNAL_RESOURCE, "Trace" );
		jeBrush_Destroy( &Brush);
	}

	FaceVerts[3]	=Verts[0];
	FaceVerts[2]	=Verts[3];
	FaceVerts[1]	=Verts[5];
	FaceVerts[0]	=Verts[4];

	if( !BrushTemplate_CreateFace( Brush, FaceVerts, 4, &FaceInfo ) )
	{
		jeErrorLog_Add( JE_ERR_INTERNAL_RESOURCE, "Trace" );
		jeBrush_Destroy( &Brush);
	}

	FaceVerts[3]	=Verts[0];
	FaceVerts[2]	=Verts[4];
	FaceVerts[1]	=Verts[7];
	FaceVerts[0]	=Verts[1];

	if( !BrushTemplate_CreateFace( Brush, FaceVerts, 4, &FaceInfo ) )
	{
		jeErrorLog_Add( JE_ERR_INTERNAL_RESOURCE, "Trace" );
		jeBrush_Destroy( &Brush);
	}

	FaceVerts[3]	=Verts[3];
	FaceVerts[2]	=Verts[2];
	FaceVerts[1]	=Verts[6];
	FaceVerts[0]	=Verts[5];

	if( !BrushTemplate_CreateFace( Brush, FaceVerts, 4, &FaceInfo ) )
	{
		jeErrorLog_Add( JE_ERR_INTERNAL_RESOURCE, "Trace" );
		jeBrush_Destroy( &Brush);
	}



	FaceVerts[3]	=Verts[8];
	FaceVerts[2]	=Verts[9];
	FaceVerts[1]	=Verts[10];
	FaceVerts[0]	=Verts[11];

	if( !BrushTemplate_CreateFace( Brush, FaceVerts, 4, &FaceInfo ) )
	{
		jeErrorLog_Add( JE_ERR_INTERNAL_RESOURCE, "Trace" );
		jeBrush_Destroy( &Brush);
	}

	FaceVerts[3]	=Verts[12];
	FaceVerts[2]	=Verts[13];
	FaceVerts[1]	=Verts[14];
	FaceVerts[0]	=Verts[15];

	if( !BrushTemplate_CreateFace( Brush, FaceVerts, 4, &FaceInfo ) )
	{
		jeErrorLog_Add( JE_ERR_INTERNAL_RESOURCE, "Trace" );
		jeBrush_Destroy( &Brush);
	}

	FaceVerts[3]	=Verts[9];
	FaceVerts[2]	=Verts[15];
	FaceVerts[1]	=Verts[14];
	FaceVerts[0]	=Verts[13];

	if( !BrushTemplate_CreateFace( Brush, FaceVerts, 4, &FaceInfo ) )
	{
		jeErrorLog_Add( JE_ERR_INTERNAL_RESOURCE, "Trace" );
		jeBrush_Destroy( &Brush);
	}

	FaceVerts[3]	=Verts[8];
	FaceVerts[2]	=Verts[11];
	FaceVerts[1]	=Verts[13];
	FaceVerts[0]	=Verts[12];

	if( !BrushTemplate_CreateFace( Brush, FaceVerts, 4, &FaceInfo ) )
	{
		jeErrorLog_Add( JE_ERR_INTERNAL_RESOURCE, "Trace" );
		jeBrush_Destroy( &Brush);
	}

	FaceVerts[3]	=Verts[8];
	FaceVerts[2]	=Verts[12];
	FaceVerts[1]	=Verts[15];
	FaceVerts[0]	=Verts[9];

	if( !BrushTemplate_CreateFace( Brush, FaceVerts, 4, &FaceInfo ) )
	{
		jeErrorLog_Add( JE_ERR_INTERNAL_RESOURCE, "Trace" );
		jeBrush_Destroy( &Brush);
	}

	FaceVerts[3]	=Verts[11];
	FaceVerts[2]	=Verts[10];
	FaceVerts[1]	=Verts[14];
	FaceVerts[0]	=Verts[13];

	if( !BrushTemplate_CreateFace( Brush, FaceVerts, 4, &FaceInfo ) )
	{
		jeErrorLog_Add( JE_ERR_INTERNAL_RESOURCE, "Trace" );
		jeBrush_Destroy( &Brush);
	}

	return	Brush;
}


jeBrush *BrushTemplate_CreateSheetBrush (const SheetTemplate *pTemplate,  jeFaceInfo * pFaceInfo )
{
	jeVec3d		Verts[4];
	//void/vizard: changed num of verts from 8 to 4
	jeBrush *	Brush;

	Brush = jeBrush_Create(1);
	if(Brush == NULL )
	{
		jeErrorLog_Add( JE_ERR_INTERNAL_RESOURCE, "Unable to create jeBrush" );
		return( NULL );
	}

	// Vertices 0 to 3 are the 4 corners of the top face
	jeVec3d_Set (&Verts[0], (float)-(pTemplate->XSize/2), 0.0f, (float)-(pTemplate->ZSize/2));
	jeVec3d_Set (&Verts[1], (float)-(pTemplate->XSize/2), 0.0f, (float)(pTemplate->ZSize/2));
	jeVec3d_Set (&Verts[2], (float)(pTemplate->XSize/2), 0.0f, (float)(pTemplate->ZSize/2));
	jeVec3d_Set (&Verts[3], (float)(pTemplate->XSize/2), 0.0f, (float)-(pTemplate->ZSize/2));

	if( !BrushTemplate_CreateFace( Brush, Verts, 4, pFaceInfo ) )
	{
		jeErrorLog_Add( JE_ERR_INTERNAL_RESOURCE, "Trace" );
		jeBrush_Destroy( &Brush);
	}
	jeBrush_SetContents( Brush, JE_BSP_CONTENTS_SHEET);

	return	Brush;
}


jeBrush	*BrushTemplate_CreateSphereBrush (const SphereTemplate *pTemplate,   jeFaceInfo * pFaceInfo )
{
	double		z, ring_radius, r, dz, t, dt;
	int			vcnt, HBand, VBand;
	jeVec3d		*sv = NULL, FaceVerts[4];
	jeBrush *	Brush;

	assert((pTemplate->HorizontalBands >= 2) && (pTemplate->VerticalBands >= 3));
	
	Brush = jeBrush_Create((pTemplate->HorizontalBands)* pTemplate->VerticalBands);
	if(Brush == NULL )
	{
		jeErrorLog_Add( JE_ERR_INTERNAL_RESOURCE, "Unable to create jeBrush" );
		goto SPHERE_ERR;
	}
	sv			=(jeVec3d *)jeRam_Allocate(sizeof(jeVec3d) * (((pTemplate->HorizontalBands-1) * pTemplate->VerticalBands)+2));
	r			=pTemplate->Radius;
	vcnt		=0;
	jeVec3d_Set (&sv[vcnt], 0.0f, pTemplate->Radius, 0.0f);
	vcnt++;
	dz			=2.0*r/(double)(pTemplate->HorizontalBands-1);
	for(z=(-r)+dz/2.0; z<(r-dz/2.0+dz/4.0); z+=dz)
	{
		ring_radius	=sqrt(r*r - z*z);
		dt			=PI2 /(double)(pTemplate->VerticalBands);
		for(t=0.0;t < PI2-(dt*0.5);t+=dt)
		{
			sv[vcnt].X	=(float)(sin(t) * ring_radius);
			sv[vcnt].Z	=(float)(cos(t) * ring_radius);
			sv[vcnt++].Y=(float)(-z);
		}
	}
	sv[vcnt].X	=0.0f;
	sv[vcnt].Y	=(float)(-pTemplate->Radius);
	sv[vcnt++].Z=0.0f;

	for(VBand=0;VBand < pTemplate->VerticalBands;VBand++)
	{
		FaceVerts[0]	=sv[0];
		FaceVerts[1]	=sv[(((1 + VBand) % pTemplate->VerticalBands) + 1)];
		FaceVerts[2]	=sv[(VBand % pTemplate->VerticalBands)+1];

		if( !BrushTemplate_CreateFace( Brush, FaceVerts, 3, pFaceInfo ) )
		{
			jeErrorLog_Add( JE_ERR_INTERNAL_RESOURCE, "Trace" );
			goto SPHERE_ERR;
		}
	}

	for(HBand=1;HBand < (pTemplate->HorizontalBands-1);HBand++)
	{
		for(VBand=0;VBand < pTemplate->VerticalBands;VBand++)
		{
			FaceVerts[0]	=sv[(((HBand-1)*pTemplate->VerticalBands)+1)+VBand];
			FaceVerts[1]	=sv[(((HBand-1)*pTemplate->VerticalBands)+1)+((VBand+1)%pTemplate->VerticalBands)];
			FaceVerts[2]	=sv[((HBand*pTemplate->VerticalBands)+1)+((VBand+1)%pTemplate->VerticalBands)];
			FaceVerts[3]	=sv[((HBand*pTemplate->VerticalBands)+1)+VBand];

			if( !BrushTemplate_CreateFace( Brush, FaceVerts, 4, pFaceInfo ) )
			{
				jeErrorLog_Add( JE_ERR_INTERNAL_RESOURCE, "Trace" );
				goto SPHERE_ERR;
			}
		}
	}

	for(VBand=0;VBand < pTemplate->VerticalBands;VBand++)
	{
		FaceVerts[0]	=sv[1+((pTemplate->HorizontalBands-2)*pTemplate->VerticalBands)+VBand];
		FaceVerts[1]	=sv[1+((pTemplate->HorizontalBands-2)*pTemplate->VerticalBands)+((VBand+1)%pTemplate->VerticalBands)];
		FaceVerts[2]	=sv[((pTemplate->HorizontalBands-1)*pTemplate->VerticalBands)+1];

		if( !BrushTemplate_CreateFace( Brush, FaceVerts, 3, pFaceInfo ) )
		{
			jeErrorLog_Add( JE_ERR_INTERNAL_RESOURCE, "Trace" );
			goto SPHERE_ERR;
			
		}
	}
	jeRam_Free(sv);

	return	Brush;

SPHERE_ERR:
	if( Brush != NULL )
		jeBrush_Destroy( &Brush);
	if( sv != NULL )
		jeRam_Free(sv);
	return( NULL );
}


jeBrush *BrushTemplate_CreateCylinderBrush (const CylinderTemplate *pTemplate,   jeFaceInfo * pFaceInfo)
{
	double		CurrentXDiameter, CurrentZDiameter;
	double		DeltaXDiameter, DeltaZDiameter;
	double		CurrentXOffset, CurrentZOffset;
	double		DeltaXOffset, DeltaZOffset, sqrcheck;
	double		EllipseZ;
	int			NumVerticalBands, HBand, VBand;
	int			VertexCount=0;
	jeVec3d		*Verts = NULL, *TopPoints = NULL;
	jeVec3d		Current, Final, Delta;
	jeXForm3d	YRotation;
	jeBrush *	Brush = NULL;

	NumVerticalBands	= (int)(pTemplate->VerticalStripes);

	assert (NumVerticalBands >= 3);

	Verts		=(jeVec3d *)jeRam_Allocate(sizeof(jeVec3d)*NumVerticalBands * 2);
	if(Verts == NULL )
	{
		jeErrorLog_Add( JE_ERR_MEMORY_RESOURCE, "Unable to allocate Verts" );
		goto CYLND_ERR;
	}
	TopPoints	=(jeVec3d *)jeRam_Allocate(sizeof(jeVec3d)*NumVerticalBands);
	if(TopPoints == NULL )
	{
		jeErrorLog_Add( JE_ERR_MEMORY_RESOURCE, "Unable to allocate TopPoints" );
		goto CYLND_ERR;
	}
	Brush = jeBrush_Create(NumVerticalBands + 2);
	if(Brush == NULL )
	{
		jeErrorLog_Add( JE_ERR_INTERNAL_RESOURCE, "Unable to create jeBrush" );
		goto CYLND_ERR;
	}


	jeXForm3d_SetIdentity(&YRotation);
	jeXForm3d_SetYRotation(&YRotation, (M_PI * 2.0f)/(jeFloat)NumVerticalBands);

	// Start with the top of cylinder
	CurrentXDiameter	=pTemplate->TopXSize;
	CurrentZDiameter	=pTemplate->TopZSize;
	DeltaXDiameter		=(pTemplate->BotXSize - pTemplate->TopXSize);
	DeltaZDiameter		=(pTemplate->BotZSize - pTemplate->TopZSize);
	
	// Get the offset amounts
	CurrentXOffset	=pTemplate->TopXOffset;
	CurrentZOffset	=pTemplate->TopZOffset;
	DeltaXOffset	=(pTemplate->BotXOffset - pTemplate->TopXOffset);
	DeltaZOffset	=(pTemplate->BotZOffset - pTemplate->TopZOffset);

	// Get the band positions and deltas
	jeVec3d_Set(&Current, (float)(pTemplate->TopXSize / 2), (float)(pTemplate->YSize / 2), 0.0);
	jeVec3d_Set(&Delta, (float)((pTemplate->BotXSize / 2) - Current.X), (float)(-(pTemplate->YSize/2) - Current.Y), 0.0);

	for(HBand = 0;HBand <= 1;HBand++)
	{
		Final = Current;
		for(VBand = 0;VBand < NumVerticalBands;VBand++)
		{
			// Get the elliptical Z value
			// (x^2/a^2) + (z^2/b^2) = 1
			// z = sqrt(b^2(1 - x^2/a^2))
			sqrcheck=( ((CurrentZDiameter/2)*(CurrentZDiameter/2))
				* (1.0 - (Final.X*Final.X)
				/ ( (CurrentXDiameter/2)*(CurrentXDiameter/2) )) );
			if(sqrcheck <0.0)
				sqrcheck=0.0;
			EllipseZ = sqrt(sqrcheck);

			// Check if we need to negate this thing
			if(VBand > (NumVerticalBands/2))
				EllipseZ = -EllipseZ;

			jeVec3d_Set
			(
				&Verts[VertexCount],
				(float)(Final.X + CurrentXOffset),
				Final.Y,
				(float)(EllipseZ + CurrentZOffset)
			);
			VertexCount++;

			// Rotate the point around the Y to get the next vertical band
			jeXForm3d_Rotate(&YRotation, &Final, &Final);
		}
		CurrentXDiameter	+=DeltaXDiameter;
		CurrentZDiameter	+=DeltaZDiameter;
		CurrentXOffset		+=DeltaXOffset;
		CurrentZOffset		+=DeltaZOffset;

		jeVec3d_Add(&Current, &Delta, &Current);
	}

	for(VBand=0;VBand < NumVerticalBands;VBand++)
	{
		TopPoints[VBand]	=Verts[VBand];
	}
	if( !BrushTemplate_CreateFace( Brush, TopPoints, NumVerticalBands, pFaceInfo ) )
	{
		jeErrorLog_Add( JE_ERR_INTERNAL_RESOURCE, "Trace" );
		goto CYLND_ERR;
		
	}

	for(VBand=NumVerticalBands-1, HBand=0;VBand >=0;VBand--, HBand++)
	{
		TopPoints[HBand]	=Verts[VBand + NumVerticalBands];
	}
	if( !BrushTemplate_CreateFace( Brush, TopPoints, HBand, pFaceInfo ) )
	{
		jeErrorLog_Add( JE_ERR_INTERNAL_RESOURCE, "Trace" );
		goto CYLND_ERR;
		
	}


	// Generate the polygons
	for(HBand = 0;HBand < 1;HBand++)
	{
		for(VBand = 0;VBand < NumVerticalBands;VBand++)
		{
			TopPoints[3]	=Verts[(HBand * NumVerticalBands) + VBand];
			TopPoints[2]	=Verts[(HBand * NumVerticalBands) + ((VBand + 1) % NumVerticalBands)];
			TopPoints[1]	=Verts[((HBand + 1) * NumVerticalBands) + ((VBand + 1) % NumVerticalBands)];
			TopPoints[0]	=Verts[((HBand + 1) * NumVerticalBands) + VBand];
			if( !BrushTemplate_CreateFace( Brush, TopPoints, 4, pFaceInfo ) )
			{
				jeErrorLog_Add( JE_ERR_INTERNAL_RESOURCE, "Trace" );
				goto CYLND_ERR;
				
			}
		}
	}
	jeRam_Free(Verts);
	jeRam_Free(TopPoints);
	return( Brush );

CYLND_ERR:
	if( Verts != NULL )
		jeRam_Free(Verts);
	if( TopPoints != NULL )
		jeRam_Free(TopPoints);
	if( Brush != NULL )
		jeBrush_Destroy( &Brush );
	return	NULL;
}

jeBrush *BrushTemplate_CreateArchBrush(const ArchTemplate *pTemplate, jeFaceInfo *FaceInfo)
{
	jeBrush		*b;//, *b2;
	//BrushList	*MBList	=BrushList_Create();
	//FaceList	*fl;
	//Face		*f;
	jeVec3d		FaceVerts[4];

	int		i, NumSlits			=pTemplate->NumSlits;
	int		NumCrossSections	=NumSlits + 2;
	jeFloat	Thickness			=pTemplate->Thickness;
	jeFloat	Width				=pTemplate->Width;
	jeFloat	InnerRadius			=pTemplate->Radius;
	double	StartAngleDegrees	=pTemplate->StartAngle;
	double	EndAngleDegrees		=pTemplate->EndAngle;
	double	AngleDelta			=0;
	double	CurAngle			=0;
	double	StartAngle			=Units_DegreesToRadians (StartAngleDegrees);
	double	EndAngle			=Units_DegreesToRadians (EndAngleDegrees);
	double	Temp;
	jeVec3d	TopInnerPoint;
	jeVec3d	TopOuterPoint;
	jeVec3d	FinalTopInnerPoint;
	jeVec3d	FinalTopOuterPoint;
	jeVec3d	FinalBottomInnerPoint;
	jeVec3d	FinalBottomOuterPoint;
	jeVec3d	OldTopInner;
	jeVec3d	OldTopOuter;
	jeVec3d	OldBottomInner;
	jeVec3d	OldBottomOuter;
	jeXForm3d XForm;

	jeXForm3d_SetIdentity(&XForm);
	jeXForm3d_SetScaling(&XForm, 0.50f, 0.50f, 0.50f);

	//If angles are equal, we have an empty shape...
	if(StartAngle==EndAngle)
	{
		return	NULL;
	}

	//	Put the angles in order...
	if(StartAngle > EndAngle)
	{
		Temp		=StartAngle;
		StartAngle	=EndAngle;
		EndAngle	=Temp;
	}

	b = jeBrush_Create(NumCrossSections);
	if (!b)
		return NULL;

	jeVec3d_Set(&TopInnerPoint, (float)InnerRadius, 0.0, (float)(Width / 2));
	jeVec3d_Set(&TopOuterPoint, (float)(InnerRadius + Thickness), 0.0, (float)(Width / 2));

	AngleDelta	=(EndAngle - StartAngle)/(NumCrossSections - 1);
	CurAngle	=StartAngle + AngleDelta;

	//	Create first cross section of 4 vertices ( outer face @ start angle)...
	jeVec3d_Set
	(
		&FinalTopInnerPoint,
		(float)(( TopInnerPoint.X * cos( StartAngle ) ) - ( TopInnerPoint.Y * sin( StartAngle ) )),
		(float)(( TopInnerPoint.X * sin( StartAngle ) ) + ( TopInnerPoint.Y * cos( StartAngle ) )),
		TopInnerPoint.Z
	);
	jeVec3d_Set
	(
		&FinalTopOuterPoint,
		(float)(( TopOuterPoint.X * cos( StartAngle ) ) - ( TopInnerPoint.Y * sin( StartAngle ) )),
		(float)(( TopOuterPoint.X * sin( StartAngle ) ) + ( TopInnerPoint.Y * cos( StartAngle ) )),
		TopOuterPoint.Z
	);
	FinalBottomInnerPoint	=FinalTopInnerPoint;
	FinalBottomInnerPoint.Z	=-FinalTopInnerPoint.Z;
	FinalBottomOuterPoint	=FinalTopOuterPoint;
	FinalBottomOuterPoint.Z	=-FinalTopOuterPoint.Z;
	OldTopInner				=FinalTopInnerPoint;
	OldTopOuter				=FinalTopOuterPoint;
	OldBottomInner			=FinalBottomInnerPoint;
	OldBottomOuter			=FinalBottomOuterPoint;

	//Create the other cross sections and assign verts to polys after each...
	for(i=0;i < (NumCrossSections-1);i++)
	{
		jeVec3d_Set
		(
			&FinalTopInnerPoint,
			(float)(( TopInnerPoint.X * cos( CurAngle ) ) - ( TopInnerPoint.Y * sin( CurAngle ) )),
			(float)(( TopInnerPoint.X * sin( CurAngle ) ) + ( TopInnerPoint.Y * cos( CurAngle ) )),
			TopInnerPoint.Z
		);
		jeVec3d_Set
		(
			&FinalTopOuterPoint,
			(float)(( TopOuterPoint.X * cos( CurAngle ) ) - ( TopInnerPoint.Y * sin( CurAngle ) )),
			(float)(( TopOuterPoint.X * sin( CurAngle ) ) + ( TopInnerPoint.Y * cos( CurAngle ) )),
			TopOuterPoint.Z
		);
		FinalBottomInnerPoint = FinalTopInnerPoint;
		FinalBottomInnerPoint.Z = -FinalTopInnerPoint.Z;

		FinalBottomOuterPoint = FinalTopOuterPoint;
		FinalBottomOuterPoint.Z = -FinalTopOuterPoint.Z;

		CurAngle += AngleDelta;

		//fl	=FaceList_Create(6);

		//Assign points to the 4 outer poly faces...

		//Top face...
		FaceVerts[0]	=FinalTopInnerPoint;
		FaceVerts[1]	=FinalTopOuterPoint;
		FaceVerts[2]	=OldTopOuter;
		FaceVerts[3]	=OldTopInner;
		//f				=Face_Create(4, FaceVerts, 0);
		//if(f)
		//{
		//	FaceList_AddFace(fl, f);
		//}
		if (!BrushTemplate_CreateFace(b, FaceVerts, 4, FaceInfo))
		{
			jeBrush_Destroy(&b);
			return NULL;
		}

		//	Bottom face...
		FaceVerts[3]	=FinalBottomInnerPoint;
		FaceVerts[2]	=FinalBottomOuterPoint;
		FaceVerts[1]	=OldBottomOuter;
		FaceVerts[0]	=OldBottomInner;
		//f				=Face_Create(4, FaceVerts, 0);
		//if(f)
		//{
		//	FaceList_AddFace(fl, f);
		//}
		if (!BrushTemplate_CreateFace(b, FaceVerts, 4, FaceInfo))
		{
			jeBrush_Destroy(&b);
			return NULL;
		}

		//	Inner side face...
		FaceVerts[0]	=FinalTopInnerPoint;
		FaceVerts[1]	=OldTopInner;
		FaceVerts[2]	=OldBottomInner;
		FaceVerts[3]	=FinalBottomInnerPoint;
		//f				=Face_Create(4, FaceVerts, 0);
		//if(f)
		//{
		//	FaceList_AddFace(fl, f);
		//}
		if (!BrushTemplate_CreateFace(b, FaceVerts, 4, FaceInfo))
		{
			jeBrush_Destroy(&b);
			return NULL;
		}

		//	Outer side face...
		FaceVerts[3]	=FinalTopOuterPoint;
		FaceVerts[2]	=OldTopOuter;
		FaceVerts[1]	=OldBottomOuter;
		FaceVerts[0]	=FinalBottomOuterPoint;
		//f				=Face_Create(4, FaceVerts, 0);
		//if(f)
		//{
		//	FaceList_AddFace(fl, f);
		//}
		if (!BrushTemplate_CreateFace(b, FaceVerts, 4, FaceInfo))
		{
			jeBrush_Destroy(&b);
			return NULL;
		}

		//make the end faces
		FaceVerts[0]	=OldTopOuter;
		FaceVerts[1]	=OldBottomOuter;
		FaceVerts[2]	=OldBottomInner;
		FaceVerts[3]	=OldTopInner;
		//f				=Face_Create(4, FaceVerts, 0);

		/*if(f)
		{
			if(pTemplate->Style < 2)	//default to hollow (if they make hollow later)
			{
				if(i)
				{
					Face_SetFixedHull(f, GE_TRUE);
				}
			}
			else
			{
				Face_SetFixedHull(f, GE_TRUE);
			}
			FaceList_AddFace(fl, f);
		}*/
		if (!BrushTemplate_CreateFace(b, FaceVerts, 4, FaceInfo))
		{
			jeBrush_Destroy(&b);
			return NULL;
		}

		FaceVerts[3]	=FinalTopOuterPoint;
		FaceVerts[2]	=FinalBottomOuterPoint;
		FaceVerts[1]	=FinalBottomInnerPoint;
		FaceVerts[0]	=FinalTopInnerPoint;
		/*f				=Face_Create(4, FaceVerts, 0);

		if(f)
		{
			if(pTemplate->Style < 2)	//default to hollow (if they make hollow later)
			{
				if(i < (NumCrossSections-2))
				{
					Face_SetFixedHull(f, GE_TRUE);
				}
			}
			else
			{
				Face_SetFixedHull(f, GE_TRUE);
			}
			FaceList_AddFace(fl, f);
		}*/
		if (!BrushTemplate_CreateFace(b, FaceVerts, 4, FaceInfo))
		{
			jeBrush_Destroy(&b);
			return NULL;
		}

		/*if(!pTemplate->Style)
		{
			//b2	=Brush_Create(BRUSH_LEAF, fl, NULL);
			//if(b2)
			//{
			//	Brush_SetSubtract(b2, pTemplate->TCut);
			//}
			//BrushList_Append(MBList, b2);
		}
		else
		{
			BrushList	*bl	=BrushList_Create();
			Brush		*bh, *bm;

			b2	=Brush_Create(BRUSH_LEAF, fl, NULL);
			if(b2)
			{
				Brush_SetHollow(b2, GE_TRUE);
				Brush_SetHullSize(b2, pTemplate->WallSize);
				bh	=Brush_CreateHollowFromBrush(b2);
				if(bh)
				{
					Brush_SetHollowCut(bh, GE_TRUE);
					BrushList_Append(bl, b2);
					BrushList_Append(bl, bh);

					bm	=Brush_Create(BRUSH_MULTI, NULL, bl);
					if(bm)
					{
						Brush_SetHollow(bm, GE_TRUE);
						Brush_SetSubtract(bm, pTemplate->TCut);
						Brush_SetHullSize(bm, pTemplate->WallSize);

						BrushList_Append(MBList, bm);
					}
				}
				else
				{
					Brush_Destroy(&b2);
					BrushList_Destroy(&bl);
				}
			}
			else
			{
				BrushList_Destroy(&bl);
			}
		}*/

		//	Set old points...
		OldTopInner		=FinalTopInnerPoint;
		OldTopOuter		=FinalTopOuterPoint;
		OldBottomInner	=FinalBottomInnerPoint;
		OldBottomOuter	=FinalBottomOuterPoint;

	}
	//b	=Brush_Create(BRUSH_MULTI, NULL, MBList);

	//if(b)
	//{
	//	Brush_SetSubtract(b, pTemplate->TCut);
	//}

	//jeBrush_SetXForm(b, &XForm, JE_FALSE);
	
	return	b;
}

//CREATORS

BoxTemplate	* BrushTemplate_CreateBox(  )
{
	BoxTemplate * pBoxTemplate ;

	pBoxTemplate = JE_RAM_ALLOCATE_STRUCT( BoxTemplate );
	if( pBoxTemplate == NULL )
	{
		jeErrorLog_Add( JE_ERR_MEMORY_RESOURCE, "Unable to allocate BoxTemplate" );
		return( NULL );
	}
	pBoxTemplate->Kind		= BRUSH_BOX  ;
	pBoxTemplate->XSizeTop	= BOX_DEFAULT_XSIZETOP;
	pBoxTemplate->XSizeBot	= BOX_DEFAULT_XSIZEBOT;
	pBoxTemplate->YSize		= BOX_DEFAULT_YSIZE;
	pBoxTemplate->ZSizeTop  = BOX_DEFAULT_ZSIZETOP;
	pBoxTemplate->ZSizeBot	= BOX_DEFAULT_ZSIZEBOT;
	return( pBoxTemplate );
}

jeBoolean BrushTemplate_FillBoxDescriptor( BoxTemplate * pBoxTemplate, jeProperty_List *pPropertyList )
{

	jeProperty Property;
	char * Name;

	Name = Util_LoadLocalRcString( IDS_BOXXRATIO_FIELD );
	if( Name == NULL )
		return( JE_FALSE );
	jeProperty_FillFloat( &Property, Name, pBoxTemplate->XSizeTop/pBoxTemplate->XSizeBot,	TEMPLATE_BOXXRATIO_FIELD, 0.1f, FLT_MAX, 0.1f );
	jeRam_Free( Name );
	if( !jeProperty_Append( pPropertyList, &Property ) )
	{
		return( JE_FALSE );
	}

	Name = Util_LoadLocalRcString( IDS_BOXZRATIO_FIELD );
	if( Name == NULL )
		return( JE_FALSE );
	jeProperty_FillFloat( &Property, Name, pBoxTemplate->ZSizeTop/pBoxTemplate->ZSizeBot,	TEMPLATE_BOXZRATIO_FIELD, 0.1f, FLT_MAX, 0.1f );
	jeRam_Free( Name );
	if( !jeProperty_Append( pPropertyList, &Property ) )
	{
		return( JE_FALSE );
	}
	return( JE_TRUE );
}

void BrushTemplate_SetBoxProperty( BoxTemplate * pBoxTemplate, int DataId, int DataType, jeProperty_Data * pData )
{
	switch( DataId )
	{
	case TEMPLATE_BOXXRATIO_FIELD:
		pBoxTemplate->XSizeTop = pData->Float * pBoxTemplate->XSizeBot;
		break;

	case TEMPLATE_BOXZRATIO_FIELD:
		pBoxTemplate->ZSizeTop = pData->Float * pBoxTemplate->ZSizeBot;
		break;
	}
	DataType;
}

SheetTemplate *	BrushTemplate_CreateSheet(  )
{
	SheetTemplate * pSheetTemplate ;

	pSheetTemplate = JE_RAM_ALLOCATE_STRUCT( SheetTemplate );
	if( pSheetTemplate == NULL )
	{
		jeErrorLog_Add( JE_ERR_MEMORY_RESOURCE, "Unable to allocate SheetTemplate" );
		return( NULL );
	}
	pSheetTemplate->Kind	= BRUSH_SHEET  ;
	pSheetTemplate->XSize	= BOX_DEFAULT_XSIZETOP;
	pSheetTemplate->ZSize  = BOX_DEFAULT_ZSIZETOP;
	
	return( pSheetTemplate );
}

SphereTemplate * BrushTemplate_CreateSphere(  )
{
	SphereTemplate * pSphereTemplate ;

	pSphereTemplate = JE_RAM_ALLOCATE_STRUCT( SphereTemplate );
	if( pSphereTemplate == NULL )
	{
		jeErrorLog_Add( JE_ERR_MEMORY_RESOURCE, "Unable to allocate SphereTemplate" );
		return( NULL );
	}
	pSphereTemplate->Kind				= BRUSH_SPHERE  ;
	pSphereTemplate->HorizontalBands	= SPHERE_DEFAULT_HORZBANDS;
	pSphereTemplate->VerticalBands		= SPHERE_DEFAULT_VERTBANDS;
	pSphereTemplate->Radius				= SPHERE_DEFAULT_RADIUS;
	return( pSphereTemplate );
}

jeBoolean BrushTemplate_FillSphereDescriptor( SphereTemplate * pSphereTemplate, jeProperty_List *pPropertyList  )
{

	jeProperty Property;
	char * Name;

	Name = Util_LoadLocalRcString( IDS_SPHERE_HBANDS_FIELD );
	if( Name == NULL )
		return( JE_FALSE );
	jeProperty_FillInt( &Property, Name, pSphereTemplate->HorizontalBands,	TEMPLATE_HBANDS_FIELD, 2.0f, 12.0f, 1.0f );
	jeRam_Free( Name );
	if( !jeProperty_Append( pPropertyList, &Property ) )
	{
		return( JE_FALSE );
	}

	Name = Util_LoadLocalRcString( IDS_SPHERE_VBANDS_FIELD );
	if( Name == NULL )
		return( JE_FALSE );
	jeProperty_FillInt( &Property, Name, pSphereTemplate->VerticalBands,	TEMPLATE_VBANDS_FIELD, 4.0f, 12.0f, 1.0f );
	jeRam_Free( Name );
	if( !jeProperty_Append( pPropertyList, &Property ) )
	{
		return( JE_FALSE );
	}
	return( JE_TRUE );
}

void BrushTemplate_SetSphereProperty( SphereTemplate * pSphereTemplate,  int DataId, int DataType, jeProperty_Data * pData )
{
	switch( DataId )
	{
	case TEMPLATE_HBANDS_FIELD:
		pSphereTemplate->HorizontalBands = pData->Int;
		break;

	case TEMPLATE_VBANDS_FIELD:
		pSphereTemplate->VerticalBands = pData->Int;
		break;
	}
	DataType;
}

CylinderTemplate	*	BrushTemplate_CreateCylinder( )
{
	CylinderTemplate * pCylinderTemplate ;

	pCylinderTemplate = JE_RAM_ALLOCATE_STRUCT( CylinderTemplate );
	if( pCylinderTemplate == NULL )
	{
		jeErrorLog_Add( JE_ERR_MEMORY_RESOURCE, "Unable to allocate CylinderTemplate" );
		return( NULL );
	}
	pCylinderTemplate->Kind				= BRUSH_CYLINDER  ;
	pCylinderTemplate->BotXOffset		= CYLINDER_DEFAULT_BOTXOFFSET;
	pCylinderTemplate->BotXSize			= CYLINDER_DEFAULT_BOTXSIZE;
	pCylinderTemplate->BotZOffset		= CYLINDER_DEFAULT_BOTZOFFSET;
	pCylinderTemplate->BotZSize			= CYLINDER_DEFAULT_BOTZSIZE;
	pCylinderTemplate->TopXOffset		= CYLINDER_DEFAULT_TOPXOFFSET;
	pCylinderTemplate->TopXSize			= CYLINDER_DEFAULT_TOPXSIZE;
	pCylinderTemplate->TopZOffset		= CYLINDER_DEFAULT_TOPZOFFSET;
	pCylinderTemplate->TopZSize			= CYLINDER_DEFAULT_TOPZSIZE;
	pCylinderTemplate->VerticalStripes	= CYLINDER_DEFAULT_VERTICALSTRIPES;
	pCylinderTemplate->YSize			= CYLINDER_DEFAULT_YSIZE;
	return( pCylinderTemplate );
}


jeBoolean BrushTemplate_FillCylinderDescriptor( CylinderTemplate * pCylinderTemplate, jeProperty_List *pPropertyList  )
{

	jeProperty Property;
	char * Name;

	Name = Util_LoadLocalRcString( IDS_CYLDXRATIO_FIELD );
	if( Name == NULL )
		return( JE_FALSE );
	jeProperty_FillFloat( &Property, Name, pCylinderTemplate->TopXSize/pCylinderTemplate->BotXSize,	TEMPLATE_CYLDXRATIO_FIELD, 0.1f, FLT_MAX, 0.1f );
	jeRam_Free( Name );
	if( !jeProperty_Append( pPropertyList, &Property ) )
	{
		return( JE_FALSE );
	}
/*
	Name = Util_LoadLocalRcString( IDS_CYLDZRATIO_FIELD );
	if( Name == NULL )
		return( JE_FALSE );
	jeProperty_FillFloat( &Property, Name, pCylinderTemplate->TopZSize/pCylinderTemplate->BotZSize,	TEMPLATE_CYLDZRATIO_FIELD, 0.1f, FLT_MAX, 0.1f );
	jeRam_Free( Name );
	if( !jeProperty_Append( pPropertyList, &Property ) )
	{
		return( JE_FALSE );
	}
*/
	Name = Util_LoadLocalRcString( IDS_CYLD_STRIPE_FIELD );
	if( Name == NULL )
		return( JE_FALSE );
	jeProperty_FillInt( &Property, Name, pCylinderTemplate->VerticalStripes,	TEMPLATE_STRIPES_FIELD, 4.0f, 12.0f, 1.0f );
	jeRam_Free( Name );
	if( !jeProperty_Append( pPropertyList, &Property ) )
	{
		return( JE_FALSE );
	}
	return( JE_TRUE );
}

void BrushTemplate_SetCylinderProperty( CylinderTemplate * pCylinderTemplate,  int DataId, int DataType, jeProperty_Data * pData )
{
	switch( DataId )
	{
	case TEMPLATE_CYLDXRATIO_FIELD:
		pCylinderTemplate->TopXSize = pData->Float * pCylinderTemplate->BotXSize;
		pCylinderTemplate->TopZSize = pData->Float * pCylinderTemplate->BotZSize;
		break;

		/*
	case TEMPLATE_CYLDZRATIO_FIELD:
		pCylinderTemplate->TopZSize = pData->Float * pCylinderTemplate->BotZSize;
		break;
*/
	case TEMPLATE_STRIPES_FIELD:
		pCylinderTemplate->VerticalStripes = pData->Int;
		break;
	}
	DataType;
}

ArchTemplate *BrushTemplate_CreateArch()
{
	ArchTemplate				*pArchTemplate = NULL;

	pArchTemplate = JE_RAM_ALLOCATE_STRUCT(ArchTemplate);
	if (!pArchTemplate)
		return NULL;

	memset(pArchTemplate, 0, sizeof(ArchTemplate));

	pArchTemplate->Kind			= BRUSH_ARCH;
	pArchTemplate->NumSlits		= 3;
	pArchTemplate->Thickness	= 15;
	pArchTemplate->Width		= 10;
	pArchTemplate->Radius		= 20;
	pArchTemplate->WallSize		= 8;
	pArchTemplate->Style		= 0;
	pArchTemplate->EndAngle		= 180.0f;
	pArchTemplate->StartAngle	= 0.0f;
	pArchTemplate->TCut			= JE_FALSE;

	return pArchTemplate;
}

jeBoolean BrushTemplate_FillArchDescriptor(ArchTemplate *pTemplate, jeProperty_List *PropList)
{
	jeProperty					Property;
	char						*Name = NULL;

	Name = Util_LoadLocalRcString(IDS_ARCH_STARTANGLE);
	if (Name == NULL)
		return JE_FALSE;

	jeProperty_FillFloat(&Property, Name, pTemplate->StartAngle, TEMPLATE_ARCH_STARTANGLE_FIELD, 0.0f, 180.0f, 1.0f);
	jeRam_Free(Name);
	jeProperty_Append(PropList, &Property);

	Name = Util_LoadLocalRcString(IDS_ARCH_ENDANGLE);
	if (Name == NULL)
		return JE_FALSE;

	jeProperty_FillFloat(&Property, Name, pTemplate->EndAngle, TEMPLATE_ARCH_ENDANGLE_FIELD, 1.0f, 180.0f, 1.0f);
	jeRam_Free(Name);
	jeProperty_Append(PropList, &Property);

	Name = Util_LoadLocalRcString(IDS_ARCH_THICKNESS);
	if (Name == NULL)
		return JE_FALSE;

	jeProperty_FillFloat(&Property, Name, pTemplate->Thickness, TEMPLATE_ARCH_THICKNESS_FIELD, 1.0f, 100.0f, 1.0f);
	jeRam_Free(Name);
	jeProperty_Append(PropList, &Property);

	Name = Util_LoadLocalRcString(IDS_ARCH_WIDTH);
	if (Name == NULL)
		return JE_FALSE;

	jeProperty_FillFloat(&Property, Name, pTemplate->Width, TEMPLATE_ARCH_WIDTH_FIELD, 1.0f, 100.0f, 1.0f);
	jeRam_Free(Name);
	jeProperty_Append(PropList, &Property);

	Name = Util_LoadLocalRcString(IDS_ARCH_INNERRADIUS);
	if (Name == NULL)
		return JE_FALSE;

	jeProperty_FillFloat(&Property, Name, pTemplate->Radius, TEMPLATE_ARCH_INNERRADIUS_FIELD, 1.0f, 360.0f, 1.0f);
	jeRam_Free(Name);
	jeProperty_Append(PropList, &Property);

	Name = Util_LoadLocalRcString(IDS_ARCH_HOLLOWWALLSIZE);
	if (Name == NULL)
		return JE_FALSE;

	jeProperty_FillFloat(&Property, Name, pTemplate->WallSize, TEMPLATE_ARCH_HOLLOWWALLSIZE_FIELD, 1.0f, 100.0f, 1.0f);
	jeRam_Free(Name);
	jeProperty_Append(PropList, &Property);

	Name = Util_LoadLocalRcString(IDS_ARCH_NUMCROSSSECTIONS);
	if (Name == NULL)
		return JE_FALSE;

	jeProperty_FillInt(&Property, Name, pTemplate->NumSlits, TEMPLATE_ARCH_NUMCROSSSECTIONS_FIELD, 1.0f, 100.0f, 1.0f);
	jeRam_Free(Name);
	jeProperty_Append(PropList, &Property);

	return JE_TRUE;
}

void BrushTemplate_SetArchProperty(ArchTemplate *pTemplate, int DataID, int DataType, jeProperty_Data *Data)
{
	switch (DataID)
	{
	case TEMPLATE_ARCH_STARTANGLE_FIELD:
		{
			pTemplate->StartAngle = Data->Float;
			break;
		}
	case TEMPLATE_ARCH_ENDANGLE_FIELD:
		{
			pTemplate->EndAngle = Data->Float;
			break;
		}
	case TEMPLATE_ARCH_THICKNESS_FIELD:
		{
			pTemplate->Thickness = Data->Float;
			break;
		}
	case TEMPLATE_ARCH_WIDTH_FIELD:
		{
			pTemplate->Width = Data->Float;
			break;
		}
	case TEMPLATE_ARCH_INNERRADIUS_FIELD:
		{
			pTemplate->Radius = Data->Float;
			break;
		}
	case TEMPLATE_ARCH_HOLLOWWALLSIZE_FIELD:
		{
			pTemplate->WallSize = Data->Float;
			break;
		}
	case TEMPLATE_ARCH_NUMCROSSSECTIONS_FIELD:
		{
			pTemplate->NumSlits = Data->Int;
			break;
		}
	}
}

//DESTUCTOR
void BrushTemplate_Destroy( BrushTemplate ** hBrushTemplate )
{
	assert( hBrushTemplate );
	assert( *hBrushTemplate );

	jeRam_Free( (*hBrushTemplate) );
}

//ACCESSOR




BrushTemplate *  BrushTemplate_Create( BRUSH_KIND Kind )
{
	BrushTemplate *pTemplate = NULL;


	switch( Kind )
	{
	case BRUSH_BOX:
		pTemplate = (BrushTemplate *)BrushTemplate_CreateBox();
		break;

	case BRUSH_SPHERE:
		pTemplate = (BrushTemplate *)BrushTemplate_CreateSphere();
		break;

	case BRUSH_CYLINDER:
		pTemplate = (BrushTemplate *)BrushTemplate_CreateCylinder();
		break;

	case BRUSH_SHEET:
		pTemplate = (BrushTemplate *)BrushTemplate_CreateSheet();
		break;

	case BRUSH_ARCH:
		pTemplate = (BrushTemplate *)BrushTemplate_CreateArch();
		break;

	default:
		assert( 0 );
		break;
	}
	return( pTemplate );
}


jeBrush *BrushTemplate_CreateBrush( const BrushTemplate * pBrushTemplate,  jeFaceInfo * pFaceInfo )
{
	jeBrush *pBrush = NULL;


	switch( pBrushTemplate->Kind )
	{
	case BRUSH_BOX:
		pBrush = BrushTemplate_CreateBoxBrush ((const BoxTemplate *)pBrushTemplate,  pFaceInfo );
		break;

	case BRUSH_SPHERE:
		pBrush = BrushTemplate_CreateSphereBrush ((const SphereTemplate *)pBrushTemplate,  pFaceInfo );
		break;

	case BRUSH_CYLINDER:
		pBrush = BrushTemplate_CreateCylinderBrush ((const CylinderTemplate *)pBrushTemplate,  pFaceInfo );
		break;

	case BRUSH_SHEET:
		pBrush = BrushTemplate_CreateSheetBrush ((const SheetTemplate *)pBrushTemplate,  pFaceInfo );
		break;

	case BRUSH_ARCH:
		pBrush = BrushTemplate_CreateArchBrush((const ArchTemplate*)pBrushTemplate, pFaceInfo);
		break;

	default:
		assert( 0 );
		break;
	}
	return( pBrush );
}

BrushTemplate *   BrushTemplate_Copy( BrushTemplate * pTemplate )
{
	BrushTemplate *pNewTemplate = NULL;


	if( pTemplate == NULL )
		return(NULL );

	switch( pTemplate->Kind )
	{
	case BRUSH_BOX:
		pNewTemplate = (BrushTemplate *)jeRam_Allocate( sizeof( BoxTemplate) );
		memcpy( pNewTemplate, pTemplate, sizeof( BoxTemplate) );
		break;

	case BRUSH_SPHERE:
		pNewTemplate = (BrushTemplate *)jeRam_Allocate( sizeof( SphereTemplate) );
		memcpy( pNewTemplate, pTemplate, sizeof( SphereTemplate) );
		break;

	case BRUSH_CYLINDER:
		pNewTemplate = (BrushTemplate *)jeRam_Allocate( sizeof( CylinderTemplate) );
		memcpy( pNewTemplate, pTemplate, sizeof( CylinderTemplate) );
		break;

	case BRUSH_SHEET:
		pNewTemplate = (BrushTemplate *)jeRam_Allocate( sizeof( SheetTemplate) );
		memcpy( pNewTemplate, pTemplate, sizeof( SheetTemplate) );
		break;

	case BRUSH_ARCH:
		pNewTemplate = (BrushTemplate*)jeRam_Allocate(sizeof(ArchTemplate));
		memcpy(pNewTemplate, pTemplate, sizeof(ArchTemplate));

	default:
		assert( 0 );
		break;
	}
	return( pNewTemplate );
}

int BrushTemplate_GetDescriptorN( BrushTemplate * pTemplate )
{
	int DescriptorN = 2;

	switch( pTemplate->Kind )
	{
	case BRUSH_BOX:
		DescriptorN += 2;
		break;

	case BRUSH_SPHERE:
		DescriptorN += 2;
		break;

	case BRUSH_CYLINDER:
		DescriptorN += 3;
		break;

	case BRUSH_ARCH:
		DescriptorN += 4;
		break;

	case BRUSH_SHEET:
		break;

	default:
		assert( 0 );
		break;
	}
	return DescriptorN;
}

jeBoolean BrushTemplate_FillTemplateDescriptor( BrushTemplate * pTemplate, jeProperty_List *pPropertyList )
{
	int TypeStringId = 0;
	char * TypeString;
	char * TemplateString;
	jeProperty Property;
	jeProperty *pGroupProperty;
	char * Name;


	Name = Util_LoadLocalRcString( IDS_TEMPLATE_FIELD );
	if( Name == NULL )
		return( JE_FALSE );
	jeProperty_FillGroup( &Property, Name, TEMPLATE_GROUP );
	jeRam_Free( Name );
	if( !jeProperty_Append( pPropertyList, &Property ) )
	{
		return( JE_FALSE );
	}

	switch( pTemplate->Kind )
	{
	case BRUSH_BOX:
		BrushTemplate_FillBoxDescriptor( (BoxTemplate*)pTemplate, pPropertyList );
		TypeStringId = IDS_BRUSH_BOX;
		break;

	case BRUSH_SPHERE:
		BrushTemplate_FillSphereDescriptor( (SphereTemplate*)pTemplate, pPropertyList );
		TypeStringId = IDS_BRUSH_SPHERE;
		break;

	case BRUSH_CYLINDER:
		BrushTemplate_FillCylinderDescriptor( (CylinderTemplate*)pTemplate, pPropertyList );
		TypeStringId = IDS_BRUSH_CYLINDER;
		break;

	case BRUSH_SHEET:
		TypeStringId = IDS_BRUSH_SHEET;
		break;

	case BRUSH_ARCH:
		BrushTemplate_FillArchDescriptor((ArchTemplate*)pTemplate, pPropertyList);
		TypeStringId = IDS_BRUSH_ARCH;
		break;

	default:
		assert( 0 );
		break;
	}
	pGroupProperty = jeProperty_ListFindByDataId(  pPropertyList, TEMPLATE_GROUP );
	if( pGroupProperty == NULL )
		return( JE_FALSE );
	TypeString = Util_LoadLocalRcString(TypeStringId);
	TemplateString = pGroupProperty->FieldName;
	pGroupProperty->FieldName = jeRam_Allocate( strlen( TypeString) + strlen( TemplateString) +4 );
	sprintf( pGroupProperty->FieldName, "%s %s", TemplateString, TypeString );
	jeRam_Free( TypeString );
	jeRam_Free( TemplateString );

	jeProperty_FillGroupEnd( &Property, TEMPLATE_GROUP_END );
	if( !jeProperty_Append( pPropertyList, &Property ) )
	{
		return( JE_FALSE );
	}
	return( JE_TRUE );
}

void BrushTemplate_SetProperty( BrushTemplate * pTemplate,  int DataId, int DataType, jeProperty_Data * pData )
{

	switch( pTemplate->Kind )
	{
	case BRUSH_BOX:
		BrushTemplate_SetBoxProperty((BoxTemplate *) pTemplate, DataId, DataType, pData );
		break;

	case BRUSH_SPHERE:
		BrushTemplate_SetSphereProperty((SphereTemplate *) pTemplate, DataId, DataType, pData );
		break;

	case BRUSH_CYLINDER:
		BrushTemplate_SetCylinderProperty((CylinderTemplate *) pTemplate, DataId, DataType, pData );
		break;

	case BRUSH_ARCH:
		BrushTemplate_SetArchProperty((ArchTemplate*)pTemplate, DataId, DataType, pData);
		break;

	case BRUSH_SHEET:
		break;

	default:
		assert( 0 );
		break;
	}
}

//FILE
jeBoolean BrushTemplate_BoxReadFromFile( BoxTemplate * pBoxTemplate, jeVFile * pF )
{
	if( jeVFile_Read( pF, &pBoxTemplate->XSizeBot, sizeof pBoxTemplate->XSizeBot ) == JE_FALSE )
	{
		jeErrorLog_AddString(JE_ERR_FILEIO_READ, "BrushTemplate_BoxReadFromFile", NULL);
		return JE_FALSE;
	}
	if( jeVFile_Read( pF, &pBoxTemplate->YSize, sizeof pBoxTemplate->YSize ) == JE_FALSE )
	{
		jeErrorLog_AddString(JE_ERR_FILEIO_READ, "BrushTemplate_BoxReadFromFile", NULL);
		return JE_FALSE;
	}
	if( jeVFile_Read( pF, &pBoxTemplate->ZSizeTop, sizeof pBoxTemplate->ZSizeTop ) == JE_FALSE )
	{
		jeErrorLog_AddString(JE_ERR_FILEIO_READ, "BrushTemplate_BoxReadFromFile", NULL);
		return JE_FALSE;
	}
	if( jeVFile_Read( pF, &pBoxTemplate->ZSizeBot, sizeof pBoxTemplate->ZSizeBot ) == JE_FALSE )
	{
		jeErrorLog_AddString(JE_ERR_FILEIO_READ, "BrushTemplate_BoxReadFromFile", NULL);
		return JE_FALSE;
	}
	return( JE_TRUE );
}

jeBoolean BrushTemplate_SphereReadFromFile( SphereTemplate * pSphereTemplate, jeVFile * pF )
{
	if( jeVFile_Read( pF, &pSphereTemplate->HorizontalBands, sizeof pSphereTemplate->HorizontalBands ) == JE_FALSE )
	{
		jeErrorLog_AddString(JE_ERR_FILEIO_READ, "BrushTemplate_SphereReadFromFile", NULL);
		return JE_FALSE;
	}
	if( jeVFile_Read( pF, &pSphereTemplate->VerticalBands, sizeof pSphereTemplate->VerticalBands ) == JE_FALSE )
	{
		jeErrorLog_AddString(JE_ERR_FILEIO_READ, "BrushTemplate_SphereReadFromFile", NULL);
		return JE_FALSE;
	}
	if( jeVFile_Read( pF, &pSphereTemplate->Radius, sizeof pSphereTemplate->Radius ) == JE_FALSE )
	{
		jeErrorLog_AddString(JE_ERR_FILEIO_READ, "BrushTemplate_SphereReadFromFile", NULL);
		return JE_FALSE;
	}
	return( JE_TRUE );
}

jeBoolean BrushTemplate_CyldReadFromFile( CylinderTemplate * pCylinderTemplate, jeVFile * pF )
{
	if( jeVFile_Read( pF, &pCylinderTemplate->BotXOffset, sizeof pCylinderTemplate->BotXOffset ) == JE_FALSE )
	{
		jeErrorLog_AddString(JE_ERR_FILEIO_READ, "BrushTemplate_CyldReadFromFile", NULL);
		return JE_FALSE;
	}
	if( jeVFile_Read( pF, &pCylinderTemplate->BotXSize, sizeof pCylinderTemplate->BotXSize ) == JE_FALSE )
	{
		jeErrorLog_AddString(JE_ERR_FILEIO_READ, "BrushTemplate_CyldReadFromFile", NULL);
		return JE_FALSE;
	}
	if( jeVFile_Read( pF, &pCylinderTemplate->BotZOffset, sizeof pCylinderTemplate->BotZOffset ) == JE_FALSE )
	{
		jeErrorLog_AddString(JE_ERR_FILEIO_READ, "BrushTemplate_CyldReadFromFile", NULL);
		return JE_FALSE;
	}
	if( jeVFile_Read( pF, &pCylinderTemplate->BotZSize, sizeof pCylinderTemplate->BotZSize ) == JE_FALSE )
	{
		jeErrorLog_AddString(JE_ERR_FILEIO_READ, "BrushTemplate_CyldReadFromFile", NULL);
		return JE_FALSE;
	}
	if( jeVFile_Read( pF, &pCylinderTemplate->TopXOffset, sizeof pCylinderTemplate->TopXOffset ) == JE_FALSE )
	{
		jeErrorLog_AddString(JE_ERR_FILEIO_READ, "BrushTemplate_CyldReadFromFile", NULL);
		return JE_FALSE;
	}
	if( jeVFile_Read( pF, &pCylinderTemplate->TopXSize, sizeof pCylinderTemplate->TopXSize ) == JE_FALSE )
	{
		jeErrorLog_AddString(JE_ERR_FILEIO_READ, "BrushTemplate_CyldReadFromFile", NULL);
		return JE_FALSE;
	}
	if( jeVFile_Read( pF, &pCylinderTemplate->TopZOffset, sizeof pCylinderTemplate->TopZOffset ) == JE_FALSE )
	{
		jeErrorLog_AddString(JE_ERR_FILEIO_READ, "BrushTemplate_CyldReadFromFile", NULL);
		return JE_FALSE;
	}
	if( jeVFile_Read( pF, &pCylinderTemplate->TopZSize, sizeof pCylinderTemplate->TopZSize ) == JE_FALSE )
	{
		jeErrorLog_AddString(JE_ERR_FILEIO_READ, "BrushTemplate_CyldReadFromFile", NULL);
		return JE_FALSE;
	}
	if( jeVFile_Read( pF, &pCylinderTemplate->VerticalStripes, sizeof pCylinderTemplate->VerticalStripes ) == JE_FALSE )
	{
		jeErrorLog_AddString(JE_ERR_FILEIO_READ, "BrushTemplate_CyldReadFromFile", NULL);
		return JE_FALSE;
	}
	if( jeVFile_Read( pF, &pCylinderTemplate->YSize, sizeof pCylinderTemplate->YSize ) == JE_FALSE )
	{
		jeErrorLog_AddString(JE_ERR_FILEIO_READ, "BrushTemplate_CyldReadFromFile", NULL);
		return JE_FALSE;
	}
	return( JE_TRUE );
}

jeBoolean BrushTemplate_SheetReadFromFile( SheetTemplate * pSheetTemplate, jeVFile * pF )
{
	if( jeVFile_Read( pF, &pSheetTemplate->XSize, sizeof pSheetTemplate->XSize ) == JE_FALSE )
	{
		jeErrorLog_AddString(JE_ERR_FILEIO_READ, "BrushTemplate_SheetReadFromFile", NULL);
		return JE_FALSE;
	}
	if( jeVFile_Read( pF, &pSheetTemplate->ZSize, sizeof pSheetTemplate->ZSize ) == JE_FALSE )
	{
		jeErrorLog_AddString(JE_ERR_FILEIO_READ, "BrushTemplate_SheetReadFromFile", NULL);
		return JE_FALSE;
	}
	return( JE_TRUE );
}

jeBoolean BrushTemplate_ArchReadFromFile(ArchTemplate *pTemplate, jeVFile *File)
{
	if (!jeVFile_Read(File, &pTemplate->StartAngle, sizeof(jeFloat)))
	{
		jeErrorLog_AddString(JE_ERR_FILEIO_READ, "BrushTemplate_ArchReadFromFile", NULL);
		return JE_FALSE;
	}

	if (!jeVFile_Read(File, &pTemplate->EndAngle, sizeof(jeFloat)))
	{
		jeErrorLog_AddString(JE_ERR_FILEIO_READ, "BrushTemplate_ArchReadFromFile", NULL);
		return JE_FALSE;
	}

	if (!jeVFile_Read(File, &pTemplate->Thickness, sizeof(jeFloat)))
	{
		jeErrorLog_AddString(JE_ERR_FILEIO_READ, "BrushTemplate_ArchReadFromFile", NULL);
		return JE_FALSE;
	}

	if (!jeVFile_Read(File, &pTemplate->Width, sizeof(jeFloat)))
	{
		jeErrorLog_AddString(JE_ERR_FILEIO_READ, "BrushTemplate_ArchReadFromFile", NULL);
		return JE_FALSE;
	}

	if (!jeVFile_Read(File, &pTemplate->Radius, sizeof(jeFloat)))
	{
		jeErrorLog_AddString(JE_ERR_FILEIO_READ, "BrushTemplate_ArchReadFromFile", NULL);
		return JE_FALSE;
	}

	if (!jeVFile_Read(File, &pTemplate->WallSize, sizeof(jeFloat)))
	{
		jeErrorLog_AddString(JE_ERR_FILEIO_READ, "BrushTemplate_ArchReadFromFile", NULL);
		return JE_FALSE;
	}

	if (!jeVFile_Read(File, &pTemplate->NumSlits, sizeof(jeFloat)))
	{
		jeErrorLog_AddString(JE_ERR_FILEIO_READ, "BrushTemplate_ArchReadFromFile", NULL);
		return JE_FALSE;
	}

	return JE_TRUE;
}

BrushTemplate * BrushTemplate_CreateFromFile( jeVFile * pF ) 
{
	int Kind;
	BrushTemplate *pTemplate = NULL;

	if( !jeVFile_Read( pF, &Kind, sizeof Kind ) )
		goto BTCFF_FAILURE ;
  
	if( Kind == BRUSH_INVALID )
		return( NULL );

	pTemplate = BrushTemplate_Create( Kind );
	if( pTemplate == NULL )
		goto BTCFF_FAILURE ;
 
	switch( pTemplate->Kind )
	{
	case BRUSH_BOX:
		if( !BrushTemplate_BoxReadFromFile((BoxTemplate *) pTemplate, pF ) )
		goto BTCFF_FAILURE ;
 
		break;

	case BRUSH_SPHERE:
		if( !BrushTemplate_SphereReadFromFile((SphereTemplate *) pTemplate, pF ) )
			goto BTCFF_FAILURE ;
 		break;

	case BRUSH_CYLINDER:
		if( !BrushTemplate_CyldReadFromFile((CylinderTemplate *) pTemplate, pF ) )
			goto BTCFF_FAILURE ;
 		break;

	case BRUSH_SHEET:
		if( !BrushTemplate_SheetReadFromFile((SheetTemplate *) pTemplate, pF ) )
			goto BTCFF_FAILURE ;
 		break;

	case BRUSH_ARCH:
		if (!BrushTemplate_ArchReadFromFile((ArchTemplate*)pTemplate, pF))
			goto BTCFF_FAILURE;
		break;

	default:
		assert( 0 );
		break;
	}
	return( pTemplate );

BTCFF_FAILURE:
	if( pTemplate != NULL )
		jeRam_Free( pTemplate );
	return( NULL );
}

jeBoolean BrushTemplate_WriteBoxToFile( BoxTemplate * pBoxTemplate, jeVFile * pF )
{
	if( jeVFile_Write( pF, &pBoxTemplate->XSizeBot, sizeof pBoxTemplate->XSizeBot ) == JE_FALSE )
	{
		jeErrorLog_AddString(JE_ERR_FILEIO_WRITE, "BrushTemplate_WriteBoxToFile", NULL);
		return JE_FALSE;
	}
	if( jeVFile_Write( pF, &pBoxTemplate->YSize, sizeof pBoxTemplate->YSize ) == JE_FALSE )
	{
		jeErrorLog_AddString(JE_ERR_FILEIO_WRITE, "BrushTemplate_WriteBoxToFile", NULL);
		return JE_FALSE;
	}
	if( jeVFile_Write( pF, &pBoxTemplate->ZSizeTop, sizeof pBoxTemplate->ZSizeTop ) == JE_FALSE )
	{
		jeErrorLog_AddString(JE_ERR_FILEIO_WRITE, "BrushTemplate_WriteBoxToFile", NULL);
		return JE_FALSE;
	}
	if( jeVFile_Write( pF, &pBoxTemplate->ZSizeBot, sizeof pBoxTemplate->ZSizeBot ) == JE_FALSE )
	{
		jeErrorLog_AddString(JE_ERR_FILEIO_WRITE, "BrushTemplate_WriteBoxToFile", NULL);
		return JE_FALSE;
	}
	return( JE_TRUE );
}

jeBoolean BrushTemplate_WriteSphereToFile( SphereTemplate * pSphereTemplate, jeVFile * pF )
{
	if( jeVFile_Write( pF, &pSphereTemplate->HorizontalBands, sizeof pSphereTemplate->HorizontalBands ) == JE_FALSE )
	{
		jeErrorLog_AddString(JE_ERR_FILEIO_WRITE, "BrushTemplate_WriteSphereToFile", NULL);
		return JE_FALSE;
	}
	if( jeVFile_Write( pF, &pSphereTemplate->VerticalBands, sizeof pSphereTemplate->VerticalBands ) == JE_FALSE )
	{
		jeErrorLog_AddString(JE_ERR_FILEIO_WRITE, "BrushTemplate_WriteSphereToFile", NULL);
		return JE_FALSE;
	}
	if( jeVFile_Write( pF, &pSphereTemplate->Radius, sizeof pSphereTemplate->Radius ) == JE_FALSE )
	{
		jeErrorLog_AddString(JE_ERR_FILEIO_WRITE, "BrushTemplate_WriteSphereToFile", NULL);
		return JE_FALSE;
	}
	return( JE_TRUE );
}

jeBoolean BrushTemplate_WriteCylinderToFile( CylinderTemplate * pCylinderTemplate, jeVFile * pF )
{
	if( jeVFile_Write( pF, &pCylinderTemplate->BotXOffset, sizeof pCylinderTemplate->BotXOffset ) == JE_FALSE )
	{
		jeErrorLog_AddString(JE_ERR_FILEIO_WRITE, "BrushTemplate_WriteCylinderToFile", NULL);
		return JE_FALSE;
	}
	if( jeVFile_Write( pF, &pCylinderTemplate->BotXSize, sizeof pCylinderTemplate->BotXSize ) == JE_FALSE )
	{
		jeErrorLog_AddString(JE_ERR_FILEIO_WRITE, "BrushTemplate_WriteCylinderToFile", NULL);
		return JE_FALSE;
	}
	if( jeVFile_Write( pF, &pCylinderTemplate->BotZOffset, sizeof pCylinderTemplate->BotZOffset ) == JE_FALSE )
	{
		jeErrorLog_AddString(JE_ERR_FILEIO_WRITE, "BrushTemplate_WriteCylinderToFile", NULL);
		return JE_FALSE;
	}
	if( jeVFile_Write( pF, &pCylinderTemplate->BotZSize, sizeof pCylinderTemplate->BotZSize ) == JE_FALSE )
	{
		jeErrorLog_AddString(JE_ERR_FILEIO_WRITE, "BrushTemplate_WriteCylinderToFile", NULL);
		return JE_FALSE;
	}
	if( jeVFile_Write( pF, &pCylinderTemplate->TopXOffset, sizeof pCylinderTemplate->TopXOffset ) == JE_FALSE )
	{
		jeErrorLog_AddString(JE_ERR_FILEIO_WRITE, "BrushTemplate_WriteCylinderToFile", NULL);
		return JE_FALSE;
	}
	if( jeVFile_Write( pF, &pCylinderTemplate->TopXSize, sizeof pCylinderTemplate->TopXSize ) == JE_FALSE )
	{
		jeErrorLog_AddString(JE_ERR_FILEIO_WRITE, "BrushTemplate_WriteCylinderToFile", NULL);
		return JE_FALSE;
	}
	if( jeVFile_Write( pF, &pCylinderTemplate->TopZOffset, sizeof pCylinderTemplate->TopZOffset ) == JE_FALSE )
	{
		jeErrorLog_AddString(JE_ERR_FILEIO_WRITE, "BrushTemplate_WriteCylinderToFile", NULL);
		return JE_FALSE;
	}
	if( jeVFile_Write( pF, &pCylinderTemplate->TopZSize, sizeof pCylinderTemplate->TopZSize ) == JE_FALSE )
	{
		jeErrorLog_AddString(JE_ERR_FILEIO_WRITE, "BrushTemplate_WriteCylinderToFile", NULL);
		return JE_FALSE;
	}
	if( jeVFile_Write( pF, &pCylinderTemplate->VerticalStripes, sizeof pCylinderTemplate->VerticalStripes ) == JE_FALSE )
	{
		jeErrorLog_AddString(JE_ERR_FILEIO_WRITE, "BrushTemplate_WriteCylinderToFile", NULL);
		return JE_FALSE;
	}
	if( jeVFile_Write( pF, &pCylinderTemplate->YSize, sizeof pCylinderTemplate->YSize ) == JE_FALSE )
	{
		jeErrorLog_AddString(JE_ERR_FILEIO_WRITE, "BrushTemplate_WriteCylinderToFile", NULL);
		return JE_FALSE;
	}
	return( JE_TRUE );
}

jeBoolean BrushTemplate_WriteSheetToFile( SheetTemplate * pSheetTemplate, jeVFile * pF )
{
	if( jeVFile_Write( pF, &pSheetTemplate->XSize, sizeof pSheetTemplate->XSize ) == JE_FALSE )
	{
		jeErrorLog_AddString(JE_ERR_FILEIO_WRITE, "BrushTemplate_WriteSheetToFile", NULL);
		return JE_FALSE;
	}
	if( jeVFile_Write( pF, &pSheetTemplate->ZSize, sizeof pSheetTemplate->ZSize ) == JE_FALSE )
	{
		jeErrorLog_AddString(JE_ERR_FILEIO_WRITE, "BrushTemplate_WriteSheetToFile", NULL);
		return JE_FALSE;
	}
	return( JE_TRUE );
}

jeBoolean BrushTemplate_WriteArchToFile(ArchTemplate *pTemplate, jeVFile *File)
{
	if (!jeVFile_Write(File, &pTemplate->StartAngle, sizeof(jeFloat)))
	{
		jeErrorLog_AddString(JE_ERR_FILEIO_WRITE, "BrushTemplate_WriteArchToFile", NULL);
		return JE_FALSE;
	}

	if (!jeVFile_Write(File, &pTemplate->EndAngle, sizeof(jeFloat)))
	{
		jeErrorLog_AddString(JE_ERR_FILEIO_WRITE, "BrushTemplate_WriteArchToFile", NULL);
		return JE_FALSE;
	}

	if (!jeVFile_Write(File, &pTemplate->Thickness, sizeof(jeFloat)))
	{
		jeErrorLog_AddString(JE_ERR_FILEIO_WRITE, "BrushTemplate_WriteArchToFile", NULL);
		return JE_FALSE;
	}

	if (!jeVFile_Write(File, &pTemplate->Width, sizeof(jeFloat)))
	{
		jeErrorLog_AddString(JE_ERR_FILEIO_WRITE, "BrushTemplate_WriteArchToFile", NULL);
		return JE_FALSE;
	}

	if (!jeVFile_Write(File, &pTemplate->Radius, sizeof(jeFloat)))
	{
		jeErrorLog_AddString(JE_ERR_FILEIO_WRITE, "BrushTemplate_WriteArchToFile", NULL);
		return JE_FALSE;
	}

	if (!jeVFile_Write(File, &pTemplate->WallSize, sizeof(jeFloat)))
	{
		jeErrorLog_AddString(JE_ERR_FILEIO_WRITE, "BrushTemplate_WriteArchToFile", NULL);
		return JE_FALSE;
	}

	if (!jeVFile_Write(File, &pTemplate->NumSlits, sizeof(jeFloat)))
	{
		jeErrorLog_AddString(JE_ERR_FILEIO_WRITE, "BrushTemplate_WriteArchToFile", NULL);
		return JE_FALSE;
	}

	return JE_TRUE;
}

jeBoolean BrushTemplate_WriteToFile( BrushTemplate * pTemplate, jeVFile * pF )
{
	int InvalidKind = BRUSH_INVALID;
	if( pTemplate == NULL )
	{
		if( jeVFile_Write( pF, &InvalidKind, sizeof InvalidKind ) == JE_FALSE )
		{
			jeErrorLog_AddString(JE_ERR_FILEIO_WRITE, "Brush_WriteToFile.\n", NULL);
			return JE_FALSE;
		}
		return( JE_TRUE );
	}

	if( jeVFile_Write( pF, &pTemplate->Kind, sizeof pTemplate->Kind ) == JE_FALSE )
	{
		jeErrorLog_AddString(JE_ERR_FILEIO_WRITE, "Brush_WriteToFile.\n", NULL);
		return JE_FALSE;
	}

	switch( pTemplate->Kind )
	{
	case BRUSH_BOX:
		if( !BrushTemplate_WriteBoxToFile((BoxTemplate *) pTemplate, pF ) )
			return JE_FALSE;
		break;

	case BRUSH_SPHERE:
		if( !BrushTemplate_WriteSphereToFile((SphereTemplate *) pTemplate, pF ) )
			return JE_FALSE;
		break;

	case BRUSH_CYLINDER:
		if( !BrushTemplate_WriteCylinderToFile((CylinderTemplate *) pTemplate, pF ) )
			return JE_FALSE;
		break;

	case BRUSH_SHEET:
		if( !BrushTemplate_WriteSheetToFile((SheetTemplate *) pTemplate, pF ) )
			return JE_FALSE;
		break;

	case BRUSH_ARCH:
		if (!BrushTemplate_WriteArchToFile((ArchTemplate*)pTemplate, pF))
			return JE_FALSE;
		break;

	default:
		assert( 0 );
		break;
	}
	return( JE_TRUE );
}
