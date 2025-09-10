/*!
	@file Camera.h
	@author Anthony Rufrano (paradoxnj)
	@brief Camera class
*/
#ifndef CAMERA_H
#define CAMERA_H

#include <windows.h>
#include "jet.h"

#include "eosscript.h"

class CJetRect;
class CCamera;

class CJetRect : public eosobject
{
public:
	CJetRect()
	{
		register_func("SetLeft", cpp_method(this, CJetRect, EOSSetLeft));
		register_func("SetRight", cpp_method(this, CJetRect, EOSSetRight));
		register_func("SetTop", cpp_method(this, CJetRect, EOSSetTop));
		register_func("SetBottom", cpp_method(this, CJetRect, EOSSetBottom));

		register_func("GetLeft", cpp_method(this, CJetRect, EOSGetLeft));
		register_func("GetRight", cpp_method(this, CJetRect, EOSGetRight));
		register_func("GetTop", cpp_method(this, CJetRect, EOSGetTop));
		register_func("GetBottom", cpp_method(this, CJetRect, EOSGetBottom));

		register_func("SetRect", cpp_method(this, CJetRect, EOSSetRect));
	}

	virtual ~CJetRect()							{}

public:
	jeRect								m_Rect;

public:
	void							SetLeft(int32 left)					{ m_Rect.Left = left; }
	void							SetRight(int32 right)				{ m_Rect.Right = right; }
	void							SetTop(int32 top)					{ m_Rect.Top = top;	}
	void							SetBottom(int32 bottom)				{ m_Rect.Bottom = bottom; }

	int32							GetLeft()							{ return m_Rect.Left; }
	int32							GetRight()							{ return m_Rect.Right; }
	int32							GetTop()							{ return m_Rect.Top; }
	int32							GetBottom()							{ return m_Rect.Bottom; }

	void SetRect(int32 left, int32 right, int32 top, int32 bottom)
	{
		m_Rect.Left = left;
		m_Rect.Right = right;
		m_Rect.Top = top;
		m_Rect.Bottom = bottom;
	}

	void GetRect(jeRect *Rect)
	{
		*Rect = m_Rect;
	}

public:
	inline void operator =(CJetRect &rect)
	{
		m_Rect.Left = rect.m_Rect.Left;
		m_Rect.Right = rect.m_Rect.Right;
		m_Rect.Top = rect.m_Rect.Top;
		m_Rect.Bottom = rect.m_Rect.Bottom;
	}

public:
	void EOSSetLeft()
	{
		m_Rect.Left = (int32)exe->pop()->geti(0);
	}

	void EOSSetRight()
	{
		m_Rect.Right = exe->pop()->geti(0);
	}

	void EOSSetTop()
	{
		m_Rect.Top = exe->pop()->geti(0);
	}

	void EOSSetBottom()
	{
		m_Rect.Bottom = exe->pop()->geti(0);
	}

	void EOSGetLeft()
	{
		exe->push(m_Rect.Left);
	}

	void EOSGetRight()
	{
		exe->push(m_Rect.Right);
	}

	void EOSGetTop()
	{
		exe->push(m_Rect.Top);
	}

	void EOSGetBottom()
	{
		exe->push(m_Rect.Bottom);
	}

	void EOSSetRect()
	{
		m_Rect.Left = exe->pop()->geti(0);
		m_Rect.Right = exe->pop()->geti(0);
		m_Rect.Top = exe->pop()->geti(0);
		m_Rect.Bottom = exe->pop()->geti(0);
	}
};

class CCamera : public eosobject
{
public:
	CCamera();
	virtual ~CCamera();

private:
	jeCamera							*m_pCamera;
	float								m_FOV;
	CJetRect							m_Rect;

	float								m_FarClipPlane;
	jeBoolean							m_bFarClipEnabled;

	jeXForm3d							m_XForm;
	
public:
	jeBoolean							Create(float fov, CJetRect *Rect);
	void								Destroy();

	void								SetFarClipPlane(jeBoolean Enable, float val);
	float								GetFarClipPlane();

	jeBoolean							IsFarClipEnabled();

	void								SetFOV(float fov);
	float								GetFOV();

	void								SetViewRect(CJetRect *Rect);
	void								GetViewRect(CJetRect *Rect);

	void								SetXForm(jeXForm3d *XForm);
	void								GetXForm(jeXForm3d *XForm);

public:
	void								EOSSetAttributes();

	void								EOSSetViewRect();
	void								EOSSetFOV();
	void								EOSGetViewRect();
	void								EOSGetFOV();

	void								EOSSetFarClipPlane();
	void								EOSGetFarClipPlane();
	void								EOSIsFarClipEnabled();

	void								EOSSetXForm();
	void								EOSGetXForm();
};

#endif