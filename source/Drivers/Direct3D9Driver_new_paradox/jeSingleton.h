/*
	@file jeSingleton.h
	@author Anthony Rufrano (paradoxnj)
	@brief Templated singleton class
	The contents of this file are subject to the Jet3D Public License
	Version 1.02 (the "License"); you may not use this file except in
	compliance with the License. You may obtain a copy of the License at
	http://www.jet3d.com
	Software distributed under the License is distributed on an "AS IS"
	basis, WITHOUT WARRANTY OF ANY KIND, either express or implied.  See
	the License for the specific language governing rights and limitations
	under the License.
	The Original Code is Jet3D, released December 12, 1999.
	Copyright (C) 1996-1999 Eclipse Entertainment, L.L.C. All Rights Reserved
*/
#pragma once

#include <assert.h>
#include "BaseType.h"

namespace jet3d {

	template <typename T> class jeSingleton
	{
	protected:
		static T* s_Singleton;

	private:
		jeSingleton(const jeSingleton<T>&);
		jeSingleton& operator=(const jeSingleton<T>&);

	public:
		jeSingleton()
		{
			//		assert(!s_Singleton);
			s_Singleton = static_cast<T*>(this);
		}

		virtual ~jeSingleton()
		{
			assert(s_Singleton);
			s_Singleton = 0;
		}

		static T& getSingleton()
		{
			assert(s_Singleton);
			return *s_Singleton;
		}

		static T* getSingletonPtr()
		{
			return s_Singleton;
		}
	};

	template <typename T> T* jeSingleton<T>::s_Singleton = 0;

} // namespace Jet3D
