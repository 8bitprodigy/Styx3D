/**
   @file ObjectDef.h                                                                       
                                                                                      
   @author Anthony Rufrano	                                                          
   @brief Static mesh object code     		                                          
                                                                                      
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
#ifndef OBJECTDEF_H
#define OBJECTDEF_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef STATICMESH_EXPORTS
#define OBJECT_API						_declspec(dllexport)
#else
#define OBJECT_API						_declspec(dllimport)
#endif

OBJECT_API jeBoolean					Object_RegisterDef(float Major, float Minor);

#ifdef __cplusplus
}
#endif

#endif