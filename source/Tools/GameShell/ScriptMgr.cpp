#include "ScriptMgr.h"
#include "GameLog.h"
#include "GameMgr.h"

#ifdef _DEBUG
#pragma comment(lib, "eosscript_d")
#else
#pragma comment(lib, "eosscript")
#endif

CScriptMgr									*CScriptMgr::Singleton = NULL;

CScriptMgr *CScriptMgr::GetPtr()
{
	if (CScriptMgr::Singleton == NULL)
		CScriptMgr::Singleton = new CScriptMgr();

	return CScriptMgr::Singleton;
}

CScriptMgr::CScriptMgr()
{
	Initialize();
}

CScriptMgr::~CScriptMgr()
{
	Shutdown();
}

bool CScriptMgr::Release()
{
	GLOG("CScriptMgr - Releasing memory...");

	delete this;
	return true;
}

bool CScriptMgr::Initialize()
{
	CGameLog::GetPtr()->Printf("CScriptMgr - Initializing Scripting Engine...");
	new eosvm();

	// system functions (like print, pause, command line, ...)
    vm->link_library(EOSLIB_SYSTEM);
	
	// advanced operations with arrays
	vm->link_library(EOSLIB_ARRAY);

	// advanced math operations (like sin, cos)
	vm->link_library(EOSLIB_MATH);

	// advanced string operations
	vm->link_library(EOSLIB_STRING);

	// Register Game Classes
	RegisterObject("GameMgr", cpp_class(CGameMgr));

	// Register Global Functions
	RegisterFunction("GetGameManager", CScriptMgr::EOSGetGameManager);

	return true;
}

void CScriptMgr::Shutdown()
{
	CGameLog::GetPtr()->Printf("CScriptMgr - Shutting down scripting engine...");
	delete vm;
}

void CScriptMgr::LoadScript(std::string name)
{
	CGameLog::GetPtr()->Printf("CScriptMgr - Loading script %s...", name.c_str());

	try {
		vm->load(name);
	}
	catch (std::exception e) {
		CGameLog::GetPtr()->Printf("CScriptMgr - Could not load script %s!!", name.c_str());
	}
}

void CScriptMgr::ReloadScript(std::string name)
{
	CGameLog::GetPtr()->Printf("CScriptMgr - Reloading script %s...", name.c_str());

	try {
		vm->reload(name);
	}
	catch (std::exception e) {
		CGameLog::GetPtr()->Printf("CScriptMgr - Could not reload script %s!!", name.c_str());
	}
}

void CScriptMgr::RegisterFunction(std::string name, void (*f)())
{
	CGameLog::GetPtr()->Printf("CScriptMgr - Registering function %s...", name.c_str());
	vm->genvi->register_func(name, f);
}

void CScriptMgr::RegisterObject(std::string name, ptr<eosobject> obj)
{
	CGameLog::GetPtr()->Printf("CScriptMgr - Registering object %s...", name.c_str());
	vm->register_type(name, obj);
}

void CScriptMgr::SetGlobal(std::string name, int val)
{
	CGameLog::GetPtr()->Printf("CScriptMgr - Setting integer global %s to value %d...", name.c_str(), val);
	vm->genvi->set_global(name, 0, val);
}

void CScriptMgr::SetGlobal(std::string name, float val)
{
	CGameLog::GetPtr()->Printf("CScriptMgr - Setting float global %s to value %f...", name.c_str(), val);
	vm->genvi->set_global(name, 0, val);
}

void CScriptMgr::SetGlobal(std::string name, std::string val)
{
	CGameLog::GetPtr()->Printf("CScriptMgr - Setting string global %s to value %s...", name.c_str(), val.c_str());
	vm->genvi->set_global(name, 0, val);
}

void CScriptMgr::SetGlobal(std::string name, ptr<eosobject> val)
{
	CGameLog::GetPtr()->Printf("CScriptMgr - Setting object global %s...", name.c_str());
	vm->genvi->set_global(name, 0, val);
}

void CScriptMgr::BindGlobal(std::string name, int val)
{
	CGameLog::GetPtr()->Printf("CScriptMgr - Binding global %s to value %d...", name.c_str(), val);
	vm->genvi->bind_global(name, 0, val);
}

void CScriptMgr::BindGlobal(std::string name, float val)
{
	CGameLog::GetPtr()->Printf("CScriptMgr - Binding global %s to value %f...", name.c_str(), val);
	vm->genvi->bind_global(name, 0, val);
}

void CScriptMgr::BindGlobal(std::string name, std::string val)
{
	CGameLog::GetPtr()->Printf("CScriptMgr - Binding global %s to value %s...", name.c_str(), val.c_str());
	vm->genvi->bind_global(name, 0, val);
}

void CScriptMgr::BindGlobal(std::string name, ptr<eosobject> val)
{
	CGameLog::GetPtr()->Printf("CScriptMgr - Binding global %s...", name.c_str());
	vm->genvi->bind_global(name, 0, val);
}

void CScriptMgr::Push(int val)
{
	exe->push(val);
}

void CScriptMgr::Push(float val)
{
	exe->push(val);
}

void CScriptMgr::Push(std::string val)
{
	exe->push(val);
}

void CScriptMgr::Push(ptr<eosobject> val)
{
	exe->push(val);
}

void CScriptMgr::Pop(int *val)
{
	*val = exe->pop()->geti(0);
}

void CScriptMgr::Pop(float *val)
{
	*val = exe->pop()->getd(0);
}

void CScriptMgr::Pop(std::string *val)
{
	*val = exe->pop()->gets(0);
}

void CScriptMgr::Pop(ptr<eosobject> *val)
{
	*val = (eosobject*)exe->pop()->geto(0);
}

void CScriptMgr::Call(std::string fname)
{
	vm->genvi->call(fname);
}

void CScriptMgr::EOSGetGameManager()
{
	exe->push(CGameMgr::GetPtr());
}
