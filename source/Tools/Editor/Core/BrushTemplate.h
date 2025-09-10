/****************************************************************************************/
/*  BRUSHTEMPLATE.H                                                                     */
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

#ifndef BRUSHTEMPLATE_H
#define BRUSHTEMPLATE_H

#include "jeBrush.h"
#include "BaseType.h"
#include "Defs.h"
#include "jeProperty.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct BoxTemplate BoxTemplate ;
typedef struct SphereTemplate SphereTemplate ;
typedef struct CylinderTemplate CylinderTemplate ;
typedef struct BrushTemplate BrushTemplate ;
typedef struct SheetTemplate SheetTemplate ;
typedef struct ArchTemplate ArchTemplate;

#define	CYLINDER_DEFAULT_YSIZE		1.0f
#define TEMPLATE_FIELD_START		3000

//CREATORS

BoxTemplate			*	BrushTemplate_CreateBox( );
SphereTemplate		*	BrushTemplate_CreateSphere( );
CylinderTemplate	*	BrushTemplate_CreateCylinder( );
SheetTemplate		*	BrushTemplate_CreateSheet(  );
BrushTemplate		*   BrushTemplate_Create( BRUSH_KIND Kind );
BrushTemplate		*   BrushTemplate_Copy( BrushTemplate * pTemplate );
ArchTemplate		*	BrushTemplate_CreateArch();

jeBrush *BrushTemplate_CreateBoxBrush (const BoxTemplate *pTemplate,  jeFaceInfo * pFaceInfo );
jeBrush *BrushTemplate_CreateSheetBrush (const SheetTemplate *pTemplate,  jeFaceInfo * pFaceInfo );
jeBrush	*BrushTemplate_CreateSphereBrush (const SphereTemplate *pTemplate,   jeFaceInfo * pFaceInfo );
jeBrush *BrushTemplate_CreateCylinderBrush (const CylinderTemplate *pTemplate,   jeFaceInfo * pFaceInfo);
jeBrush *BrushTemplate_CreateBrush( const BrushTemplate * pBrushTemplate,  jeFaceInfo * pFaceInfo );
jeBrush *BrushTemplate_CreateCameraBrush (int BoxSize );
jeBrush *BrushTemplate_CreateArchBrush(const ArchTemplate *pTemplate, jeFaceInfo *pFaceInfo);

//DESTUCTOR
void					BrushTemplate_Destroy( BrushTemplate ** BrushTemplate );

//ACCESSORS
int BrushTemplate_GetDescriptorN( BrushTemplate * pTemplate );
jeBoolean BrushTemplate_FillTemplateDescriptor( BrushTemplate * pTemplate, jeProperty_List *pPropertyList );
void BrushTemplate_SetProperty( BrushTemplate * pTemplate,  int DataId, int DataType, jeProperty_Data * pData );

//FILE
BrushTemplate		* BrushTemplate_CreateFromFile( jeVFile * pF ) ;
jeBoolean			BrushTemplate_WriteToFile( BrushTemplate * pTemplate, jeVFile * pF) ;


#ifdef __cplusplus
}
#endif

#endif //Prevent multiple inclusion
/* EOF: BrushTemplate.h */