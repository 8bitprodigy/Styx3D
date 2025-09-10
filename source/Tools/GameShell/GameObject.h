/*!
	@file GameObject.h
	@author Anthony Rufrano (paradoxnj)
	@brief Game object class
*/
#ifndef GAME_OBJECT_H
#define GAME_OBJECT_H

#include <windows.h>
#include "jet.h"

#include <map>
#include <string>

enum PropertyType
{
	PROPERTY_TYPE_STRING = 0,
	PROPERTY_TYPE_INT,
	PROPERTY_TYPE_FLOAT
};

typedef struct Property
{
	std::string					Name;
	uint32						Type;

	union {
		std::string					StringVal;
		int							IntVal;
		float						FloatVal;
	}
} Property;

class CGameObject : public eosobject
{
public:
	CGameObject();
	virtual ~CGameObject();

private:
	jeObject								*m_pObject;
	jeXForm3d								m_XForm;

	jeVec3d									m_Position, m_Rotation;
	std::string								m_Name;

	std::map<std::string, Property>			m_Properties;

public:
};

#endif
