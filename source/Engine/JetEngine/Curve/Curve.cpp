/****************************************************************************************/
/*  CURVE.C                                                                              */
/*                                                                                      */
/*  Author:  Jeff Muizelaar (void/vizard)                                                               */
/*  Description: Bezier subdividing functions                                                                        */
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
#include <assert.h>

#include "Curve.h"
#include "jeTypes.h"
#include "vec3d.h"
#include "ram.h"

#include "jet.h"

int LevelToWidth[] = {3, 5, 9, 17, 33, 65, 129};


JETAPI void QuadraticBezierSubdivide( jeVec3d G[], int level) {
	// Calculate subdivided indexes
	int width = LevelToWidth[level] - 1;
	int mid = width >> 1;
	int a = width >> 2;
	int b = mid + a;
	// Do not subdivide any more, however, move middle control point to be
	// a point on the curve
	if(level == 0) {
		jeVec3d a;
		jeVec3d b;
		a.X = (G[0].X + G[1].X)*0.5f;
		a.Y = (G[0].Y + G[1].Y)*0.5f;
		a.Z = (G[0].Z + G[1].Z) * 0.5f;
		
		b.X = (G[1].X + G[2].X)*0.5f;
		b.Y = (G[1].Y + G[2].Y)*0.5f;
		b.Z = (G[1].Z + G[2].Z) * 0.5f;
		G[1].X = (a.X+b.X)*0.5f;
		G[1].Y = (a.Y+b.Y)*0.5f;
		G[1].Z = (a.Z+b.Z)*0.5f;

		return;
	}

	// Enter subdivided control points into the geometry vector
	G[a].X = (G[0].X + G[mid].X) * 0.5f;
	G[a].Y = (G[0].Y + G[mid].Y) * 0.5f;
	G[a].Z = (G[0].Z + G[mid].Z) * 0.5f;
	G[b].X = (G[mid].X + G[width].X) * 0.5f;
	G[b].Y = (G[mid].Y + G[width].Y) * 0.5f;
	G[b].Z = (G[mid].Z + G[width].Z) * 0.5f;
	G[mid].X = (G[a].X + G[b].X) * 0.5f;
	G[mid].Y = (G[a].Y + G[b].Y) * 0.5f;
	G[mid].Z = (G[a].Z + G[b].Z) * 0.5f;

	// Call recursively for left and right halves
	QuadraticBezierSubdivide(G, --level);
	QuadraticBezierSubdivide(&G[mid], level);
}
JETAPI jeVec3d* QuadraticBezierPatchSubdivide(jeVec3d G[3][3], int level) {
	// Calculate some indexes
	int width = LevelToWidth[level];
	int mid = width >> 1;
	int total = width * width;
	int i,j;
	// Allocate space for the points on the surface and the interpolation of the
	// control points
	jeVec3d* p;
	jeVec3d* index;
	
	p = (jeVec3d *)malloc(sizeof(jeVec3d)*total+3*width);

	// Interpolate the control points using the geometry matrix
	for( i = 0; i <3; i++) {
		// Distribute the control points into the space to interpolate into
		p[total] = G[i][0];
		p[total+mid] = G[i][1];
		p[total+width-1] = G[i][2];
		QuadraticBezierSubdivide(&p[total], level);

		index= p+i*mid;
		for(j = 0; j < width; j++) {
			*index = p[total + j];
			index += width;
		}
		total += width;

	}	

// Interpolate all of the remaining points
	for(i = 0; i < width; i++) {
		QuadraticBezierSubdivide(&p[i * width], level);
	}

	// Return the points. Must be freed by the caller!
	return p;
}