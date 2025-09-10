/****************************************************************************************/
/*  DRAWBBOX.C                                                                          */
/*                                                                                      */
/*  Author:  Eli Boling                                                                 */
/*  Description:  Useful utility code.  Draws a bounding box.                           */
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
static	void	DrawFace(jeWorld *World, const jeVec3d **Verts)
{
	JE_LVertex	LVerts[4];
	int			i;

	for	(i = 0; i < 4; i++)
	{
		LVerts[i].r = 40.0f;
		LVerts[i].g = 40.0f;
		LVerts[i].b = 80.0f + 20.0f * (jeFloat)i;
		LVerts[i].a = 128.0f;
		LVerts[i].X = Verts[i]->X;
		LVerts[i].Y = Verts[i]->Y;
		LVerts[i].Z = Verts[i]->Z;
	}

	jeWorld_AddPolyOnce(World, &LVerts[0], 4, NULL, JE_GOURAUD_POLY, JE_FX_TRANSPARENT, 1.0f);
}

void	DrawBoundBox(jeWorld *World, const jeVec3d *Pos, const jeVec3d *Min, const jeVec3d *Max)
{
	jeFloat	dx;
	jeFloat	dy;
	jeFloat	dz;
static	jeVec3d		Verts[8];
static	jeVec3d *	Faces[6][4] =
{
	{ &Verts[0], &Verts[1], &Verts[2], &Verts[3] },	//Top
	{ &Verts[4], &Verts[5], &Verts[6], &Verts[7] },	//Bottom
	{ &Verts[3], &Verts[2], &Verts[6], &Verts[7] }, //Side
	{ &Verts[1], &Verts[0], &Verts[4], &Verts[5] }, //Side
	{ &Verts[0], &Verts[3], &Verts[7], &Verts[4] }, //Front
	{ &Verts[2], &Verts[1], &Verts[5], &Verts[6] }, //Back
};
	int			i;

	for	(i = 0; i < 8; i++)
		jeVec3d_Add(Pos, Min, &Verts[i]);

	dx = Max->X - Min->X;
	dy = Max->Y - Min->Y;
	dz = Max->Z - Min->Z;

	Verts[0].Y += dy;
	Verts[3].Y += dy;
	Verts[3].X += dx;
	Verts[7].X += dx;

	Verts[1].Y += dy;
	Verts[1].Z += dz;
	Verts[5].Z += dz;
	Verts[6].Z += dz;
	Verts[6].X += dx;

	Verts[2].X += dx;
	Verts[2].Y += dy;
	Verts[2].Z += dz;

	for	(i = 0; i < 6; i++)
		DrawFace(World, &Faces[i][0]);
}

