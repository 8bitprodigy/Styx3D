/*!
	@file ScriptMgr.h
	@author Anthony Rufrano (paradoxnj)
	@brief Manages the EOSScript virtual machine
*/
#ifndef SCRIPTMGR_H
#define SCRIPTMGR_H

#include "eosscript.h"

class CScriptMgr;

class CScriptMgr
{
public:
	CScriptMgr();
	virtual ~CScriptMgr();

public:
	bool							Release();

	bool							Initialize();
	void							Shutdown();

	void							LoadScript(std::string name);
	void							ReloadScript(std::string name);

	void							RegisterFunction(std::string name, void (*f)());
	void							RegisterObject(std::string name, ptr<eosobject> obj);

	void							SetGlobal(std::string name, int val);
	void							SetGlobal(std::string name, float val);
	void							SetGlobal(std::string name, std::string val);
	void							SetGlobal(std::string name, ptr<eosobject> val);

	void							BindGlobal(std::string name, int val);
	void							BindGlobal(std::string name, float val);
	void							BindGlobal(std::string name, std::string val);
	void							BindGlobal(std::string name, ptr<eosobject> val);

	void							Push(int val);
	void							Push(float val);
	void							Push(std::string val);
	void							Push(ptr<eosobject> val);

	void							Pop(int *val);
	void							Pop(float *val);
	void							Pop(std::string *val);
	void							Pop(ptr<eosobject> *val);

	void							Call(std::string fname);

public:
	static CScriptMgr				*Singleton;
	static CScriptMgr				*GetPtr();

public:
	// Global script functions
	static void						EOSGetGameManager();
};

#endif
