/****************************************************************************************/
/*  jeMemAllocInfo.h                                                                    */
/*                                                                                      */
/*  Author: David Eisele                                                                */
/*  Description: Extended memoryleak debugging module                                   */
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
/*  This file was not part of the original Jet3D, released December 12, 1999.           */
/*                                                                                      */
/****************************************************************************************/

#ifndef JE_MEMALLOCINFO_H
#define JE_MEMALLOCINFO_H

#define JE_DEACTIVATE_JMAI

#ifdef NDEBUG
	#define JE_DEACTIVATE_JMAI	// RELEASE COMPILE SHALL IGNORE jMAI-MODULE!!!
#endif

#include "BaseType.h"

#ifdef __cplusplus
extern "C" {
#endif

#define jMAI_CREATED				(1<<0)		// module created/destroyed flag
#define jMAI_ACTIVE					(1<<1)		// turn on/off flag
#define	jMAI_SAVE_ON_FREE			(1<<2)		// save/delete infos on free flag
#define jMAI_EXCLUDE_ALL_FILES		(1<<3)		// exclude/include all files flag

#ifndef JE_DEACTIVATE_JMAI

JETAPI void JETCC jeMemAllocInfo_Create(const char *FName);
//	This should be the very first jMAI call:
//		All static variables and memory structures are created and set to base values
//		FName : Name of Preferences file

JETAPI void JETCC jeMemAllocInfo_Destroy();
//	This should be the final jMAI call:
//		All static variables and memory structures are freed/set to base values

JETAPI void JETCC jeMemAllocInfo_Activate();
//	This function activates the jMAI-module:
//		Initialization:
//			read pref.file and set flags/included/excluded files and all breakpoints
//			Breakpointformat: <Filename>( <Linenumber> ) xxxx [ <Callnumber> ]
//			breakpoint-& dump-format are the same, so you can copy & paste lines from dump to preferences file
//			if no linenumber or callnumber is given, the function assumes zero
//				FileName	: Name of sourcefile, where allocation is called
//				LineNr		: Linenumber of allocationcall
//									0 = react on every allocationcall in this file
//				CallNr		: On which call should be stopped?
//									0 = all calls
//									1 = first one, 2 = second one, ...
//		all created pointers (with jeRam) will be registered
//		all set breakpoints will be activated
//		file/position/latest reallocation/current pointer/size will be recorded

JETAPI void JETCC jeMemAllocInfo_DeActivate(jeBoolean DojMAIReport);
//	This function deactivates the jMAI-module:
//		all current (and freed, if flag was set) pointers will be reported, if DojMAIReport is JE_TRUE
//		(re)alloc/free infostructures will be freed
//	All dumps will be appended, if the file exists

JETAPI void JETCC jeMemAllocInfo_FileReport(const char *FName, const char *DumpFile, jeBoolean FreedMemoryReport);
//	Use this to write a report of a specified source file: FName
//		DumpFile			:	Name of dumpfile
//		FreedMemoryReport	:	JE_TRUE  = report freed   pointers
//								JE_FALSE = report current pointers

JETAPI uint32 JETCC jeMemAllocInfo_GetFlags();
//	Get current flags

#else
	#define jeMemAllocInfo_Create(FName)
	#define jeMemAllocInfo_Activate()
	#define jeMemAllocInfo_DeActivate(DojMAIReport)
	#define jeMemAllocInfo_Destroy()
	#define jeMemAllocInfo_FileReport(FName,DumpFile,FreedMemoryReport)
	#define jeMemAllocInfo_GetFlags(Flags)								0
#endif

#ifdef __cplusplus
}
#endif

#endif

#if 0
// Sample Preferences.jMAI-file
<NAMES>
dumpfile="jMAI_current.dmp"
freedfile="jMAI_freed.dmp"
</NAMES>

<FLAGS>
ExcludeAllFiles=0
SaveOnFree=1
</FLAGS>

<BREAKPOINTS>
E:\jetpp\jet\src\jet\JetEngine\guWorld\jeIndexPoly.c(   40):           8 byte(s)   allocated [       12]
E:\jetpp\jet\src\jet\JetEngine\guWorld\jeVertArray.c(   77)
E:\jetpp\jet\src\jet\JetEngine\guWorld\jeBrush.c
</BREAKPOINTS>

<INCLUDE>
</INCLUDE>

<EXCLUDE>
E:\jetpp\jet\src\jet\JetEngine\Support\jeResource.c
</EXCLUDE>
// EOF

// Usage sample
// >Editor-MFC Files-jwe.cpp<
// ...
#include "jeMemAllocInfo.h"
//...
BOOL CJweApp::InitInstance()
{
	CString	cstr ;
	//jeXForm3d_SetMaximalAssertionMode( JE_FALSE ) ;
	CString	ObjectDllPath;
	char Path[MAX_PATH];

	jeMemAllocInfo_Create("Preferences.jMAI");
//...
}

int CJweApp::ExitInstance() 
{ 
	//...
	jeMemAllocInfo_Destroy();
	return CWinApp::ExitInstance();
}// ExitInstance
//...

// >Editor-MFC Files-Doc.cpp<
// ...
#include "jeMemAllocInfo.h"
//...
CJweDoc::CJweDoc() : m_pLevel(NULL), m_Mode(MODE_POINTER_BB), m_LastFOV( 2.0f ), m_bLoaded( JE_FALSE ), m_Anim_State(0)
{
	// TODO: add one-time construction code here

	jeMemAllocInfo_Activate();
//...
}

CJweDoc::~CJweDoc()
{
	//...
	jeMemAllocInfo_DeActivate(JE_TRUE);

	AfxOleUnlockApp();
}
//...
// Usage sample end
#endif