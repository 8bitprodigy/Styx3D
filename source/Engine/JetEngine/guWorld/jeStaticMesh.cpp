/**
   @file jeStaticMesh.cpp
                                                                                      
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
#include <assert.h>
#include <vector>
#include <string>
#include <windows.h>

#include "jeStaticMesh.h"
#include "Ram.h"
#include "Engine.h"
#include "Camera.h"
#include "jeFrustum.h"
#include "jeMaterial.h"
#include "Bitmap.h"
#include "Actor.h"
#include "Body.h"
#include "Body._H"
#include "ExtBox.h"
#include "jeTypes.h"

typedef struct jeStaticMesh
{
	std::string							Name;
	uint32								RefCount;

	std::vector<jeTLVertex>				Vertices;
	std::vector<jeBody_Material>		Materials;

	jeXForm3d							XForm;
	jeExtBox							BBox;

	jeBody_TriangleList					Faces;
} jeStaticMesh;

JETAPI jeStaticMesh * JETCC jeStaticMesh_Create(const char *MeshName, jeResourceMgr *ResMgr, jeXForm3d *XForm)
{
	jeActor_Def							*ActorDef = NULL;
	jeBody								*Body = NULL;
	int									NumVerts, NumFaces, NumNormals;
	jeStaticMesh						*Mesh = NULL;
	
	ActorDef = (jeActor_Def*)jeResource_Get(ResMgr, (char*)MeshName);
	if (!ActorDef)
	{
		jeVFile							*Dir = NULL, *File = NULL;
		std::string						meshpath, filename, fullpath;
		std::string						name = MeshName;

		uint32 pos = name.find_first_of('.');
		meshpath = name.substr(0, pos - 1);
		filename = name.substr(pos + 1, name.size() - 1);

		fullpath = "StaticMesh\\";
		fullpath += meshpath;

		Dir = jeResource_GetVFile(ResMgr, (char*)meshpath.c_str());
		if (!Dir)
		{
			std::string				temppath;

			temppath = meshpath;
			temppath += ".jetpak";

			Dir = jeResource_GetVFile(ResMgr, (char*)temppath.c_str());
			if (!Dir)
			{
				Dir = jeVFile_OpenNewSystem(NULL, JE_VFILE_TYPE_DOS, fullpath.c_str(), NULL, JE_VFILE_OPEN_READONLY | JE_VFILE_OPEN_DIRECTORY);
				if (!Dir)
				{
					fullpath += ".jetpak";
					Dir = jeVFile_OpenNewSystem(NULL, JE_VFILE_TYPE_VIRTUAL, fullpath.c_str(), NULL, JE_VFILE_OPEN_READONLY | JE_VFILE_OPEN_DIRECTORY);
					if (!Dir)
						return NULL;

					meshpath += ".jetpak";
				}

				jeResource_AddVFile(ResMgr, (char*)meshpath.c_str(), Dir);
			}
		}

		File = jeVFile_Open(Dir, (char*)filename.c_str(), JE_VFILE_OPEN_READONLY);
		if (!File)
			return NULL;

		ActorDef = jeActor_DefCreateFromFile(File);
		if (!ActorDef)
		{
			jeVFile_Close(File);
			return NULL;
		}

		jeVFile_Close(File);

		jeResource_Add(ResMgr, (char*)MeshName, JE_RESOURCE_ACTOR, ActorDef);
	}
    
	Body = jeActor_GetBody(ActorDef);
	if (!Body)
	{
		jeActor_DefDestroy(&ActorDef);
		return NULL;
	}

	jeBody_GetGeometryStats(Body, 0, &NumVerts, &NumFaces, &NumNormals);

	Mesh = (jeStaticMesh*)jeRam_AllocateClear(sizeof(jeStaticMesh));
	if (!Mesh)
	{
		jeActor_DefDestroy(&ActorDef);
		return NULL;
	}

	Mesh->Name = MeshName;
	Mesh->RefCount = 1;
	
	jeXForm3d_Copy(XForm, &Mesh->XForm);
	Mesh->Vertices.resize(NumVerts);
	Mesh->Materials.resize(jeBody_GetMaterialCount(Body));

	memcpy(&Mesh->Faces, &Body->SkinFaces[0], sizeof(jeBody_TriangleList));

	for (int32 i = 0; i < jeBody_GetMaterialCount(Body); i++)
	{
		const char					*matname = NULL;

		jeBody_GetMaterial(Body, i, &matname, &Mesh->Materials[i].MatSpec, &Mesh->Materials[i].Red, &Mesh->Materials[i].Green, &Mesh->Materials[i].Blue, &Mesh->Materials[i].Mapper);
	}

	//	by trilobite	Jan. 2011
	//for (i = 0; i < NumVerts; i++)
	for (int32 i = 0; i < NumVerts; i++)
	//
	{
		jeVec3d						temp;

		jeVec3d_Copy(&Body->XSkinVertexArray[i].XPoint, &temp);
		jeXForm3d_Transform(XForm, &temp, &temp);

		Mesh->Vertices[i].x = temp.X;
		Mesh->Vertices[i].y = temp.Y;
		Mesh->Vertices[i].z = temp.Z;
		Mesh->Vertices[i].pad = 0.0f;

		Mesh->Vertices[i].u = Body->XSkinVertexArray[i].XU;
		Mesh->Vertices[i].v = Body->XSkinVertexArray[i].XV;
		Mesh->Vertices[i].pad1 = 0.0f;
		Mesh->Vertices[i].pad2 = 0.0f;

		Mesh->Vertices[i].r = Mesh->Vertices[i].g = Mesh->Vertices[i].b = Mesh->Vertices[i].a = 0.0f;
		Mesh->Vertices[i].sr = Mesh->Vertices[i].sg = Mesh->Vertices[i].sb = Mesh->Vertices[i].pad3 = 0.0f;
	}

	//	by trilobite	Jan. 2011
	//for (i = 0; i < NumFaces; i++)
	for (int32 i = 0; i < NumFaces; i++)
	//
	{
		for (int x = 0; x < 3; x++)
		{
			Mesh->Vertices[Mesh->Faces.FaceArray[i].VtxIndex[x]].sr = Mesh->Materials[Mesh->Faces.FaceArray[i].MaterialIndex].Red;
			Mesh->Vertices[Mesh->Faces.FaceArray[i].VtxIndex[x]].sg = Mesh->Materials[Mesh->Faces.FaceArray[i].MaterialIndex].Green;
			Mesh->Vertices[Mesh->Faces.FaceArray[i].VtxIndex[x]].sb = Mesh->Materials[Mesh->Faces.FaceArray[i].MaterialIndex].Blue;
		}
	}

	return Mesh;
}

JETAPI uint32 JETCC jeStaticMesh_CreateRef(jeStaticMesh *Mesh)
{
	assert(Mesh != NULL);

	Mesh->RefCount++;
	return Mesh->RefCount;
}

JETAPI uint32 JETCC jeStaticMesh_Destroy(jeStaticMesh **Mesh)
{
	assert(*Mesh != NULL);

	(*Mesh)->RefCount--;
	if ((*Mesh)->RefCount == 0)
	{
		(*Mesh)->Vertices.clear();
		(*Mesh)->Materials.clear();

		jeRam_Free((*Mesh));
		(*Mesh) = NULL;

		return 0;
	}

	return (*Mesh)->RefCount;
}

JETAPI jeBoolean JETCC jeStaticMesh_Render(jeStaticMesh *Mesh, jeEngine *Engine, jeCamera *Camera, jeFrustum *Frustum, jeXForm3d *XForm)
{
	jeFrustum				worldfrustum;

	assert(Mesh != NULL);
	assert(Engine != NULL);
	assert(Camera != NULL);
	assert(XForm != NULL);

	if (Frustum == NULL)
		jeFrustum_SetFromCamera(&worldfrustum, Camera);
	else
		worldfrustum = *Frustum;

	for (jeBody_Index i = 0; i < Mesh->Faces.FaceCount; i++)
	{
		jeTLVertex					v[3];

		for (int x = 0; x < 3; x++)
		{
			for (int j = 0; j < 3; j++)
			{
				v[x].x = Mesh->Vertices[Mesh->Faces.FaceArray[i].VtxIndex[j]].x;
				v[x].y = Mesh->Vertices[Mesh->Faces.FaceArray[i].VtxIndex[j]].y;
				v[x].z = Mesh->Vertices[Mesh->Faces.FaceArray[i].VtxIndex[j]].z;

				v[x].r = Mesh->Vertices[Mesh->Faces.FaceArray[i].VtxIndex[j]].r;
				v[x].g = Mesh->Vertices[Mesh->Faces.FaceArray[i].VtxIndex[j]].g;
				v[x].b = Mesh->Vertices[Mesh->Faces.FaceArray[i].VtxIndex[j]].b;
				v[x].a = Mesh->Vertices[Mesh->Faces.FaceArray[i].VtxIndex[j]].a;

				v[x].sr = Mesh->Vertices[Mesh->Faces.FaceArray[i].VtxIndex[j]].sr;
				v[x].sg = Mesh->Vertices[Mesh->Faces.FaceArray[i].VtxIndex[j]].sg;
				v[x].sb = Mesh->Vertices[Mesh->Faces.FaceArray[i].VtxIndex[j]].sb;
			}
		}

		//jeEngine_RenderPoly(Engine, v, 3, Mesh->Materials[Mesh->Faces.FaceArray[i].MaterialIndex].Bitmap, JE_RENDER_FLAG_SPECULAR | JE_RENDER_FLAG_COUNTER_CLOCKWISE);
		jeEngine_RenderPoly(Engine, v, 3, Mesh->Materials[Mesh->Faces.FaceArray[i].MaterialIndex].MatSpec, JE_RENDER_FLAG_SPECULAR | JE_RENDER_FLAG_COUNTER_CLOCKWISE);
	}

/*
// Krouer : slight moveto materialspec
#ifdef _USE_BITMAPS
		jeEngine_RenderPoly(Engine, (jeTLVertex*)v, 3, jeMaterial_GetBitmap(Mesh->Materials[0]), JE_RENDER_FLAG_COUNTER_CLOCKWISE);
#else
*/

/*#pragma message("Krouer: think to add the jeTexture support here")
#endif
*/	
	return JE_TRUE;
}

JETAPI jeBoolean JETCC jeStaticMesh_GetExtBox(jeStaticMesh *Mesh, jeExtBox *BBox)
{
	assert(Mesh != NULL);
	assert(BBox != NULL);

	BBox->Min.X = -20.0f;
	BBox->Min.Y = -20.0f;
	BBox->Min.Z = -20.0f;

	BBox->Max.X = 20.0f;
	BBox->Max.Y = 20.0f;
	BBox->Max.Z = 20.0f;

	return JE_TRUE;
}

JETAPI jeBoolean JETCC jeStaticMesh_SetExtBox(jeStaticMesh *Mesh, jeExtBox *BBox)
{
	assert(Mesh != NULL);
	assert(BBox != NULL);

	jeVec3d_Copy(&Mesh->BBox.Min, &BBox->Min);
	jeVec3d_Copy(&Mesh->BBox.Max, &BBox->Max);

	return JE_TRUE;
}

JETAPI jeBoolean JETCC jeStaticMesh_Collision(jeStaticMesh *Mesh, jeExtBox *BBox, jeVec3d *Front, jeVec3d *Back, jeVec3d *Impact, jePlane *Plane)
{
	return JE_TRUE;
}
