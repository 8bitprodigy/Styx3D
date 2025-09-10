# Microsoft Developer Studio Project File - Name="OpenGLDriver" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=OpenGLDriver - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "OpenGLDriver.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "OpenGLDriver.mak" CFG="OpenGLDriver - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "OpenGLDriver - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "OpenGLDriver - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "OpenGLDriver - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /w /W0 /Gm /GX /ZI /Od /I "D:\jstudio3d\jstudio3d\include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /GZ PRECOMP_VC7_TOBEREMOVED /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /I "." /I "..\..\..\include" /I "..\..\Engine\JetEngine\Engine\Drivers" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_USRDLL" /D "OPENGLDRIVER_EXPORTS" /GZ /c
# ADD BASE MTL /nologo /win32
# ADD MTL /nologo /win32
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib d3dx9.lib d3d9.lib winmm.lib /nologo /subsystem:windows /dll /debug /machine:IX86 /pdbtype:sept
# SUBTRACT BASE LINK32 /pdb:none
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib opengl32.lib glu32.lib winmm.lib /nologo /subsystem:windows /dll /debug /machine:IX86 /out:"..\..\..\bin\OpenGLDriver.dll" /pdbtype:sept /libpath:"..\..\..\lib"
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "OpenGLDriver - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /w /W0 /GX /Zi /I "D:\jstudio3d\jstudio3d\include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" PRECOMP_VC7_TOBEREMOVED /c
# ADD CPP /nologo /W3 /GX /I "." /I "..\..\..\include" /I "..\..\Engine\JetEngine\Engine\Drivers" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_USRDLL" /D "OPENGLDRIVER_EXPORTS" /c
# ADD BASE MTL /nologo /win32
# ADD MTL /nologo /win32
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib d3dx9.lib d3d9.lib /nologo /subsystem:windows /dll /debug /machine:IX86 /pdbtype:sept /opt:ref /opt:icf
# ADD LINK32 opengl32.lib glu32.lib winmm.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /map /machine:IX86 /out:"..\..\..\bin\OpenGLDriver.dll" /pdbtype:sept /libpath:"..\..\..\lib" /opt:ref /opt:icf
# SUBTRACT LINK32 /debug

!ENDIF 

# Begin Target

# Name "OpenGLDriver - Win32 Debug"
# Name "OpenGLDriver - Win32 Release"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;def;odl;idl;hpj;bat;asm;asmx"
# Begin Source File

SOURCE=.\OGLDrv.cpp
DEP_CPP_OGLDR=\
	"..\..\..\include\BaseType.h"\
	"..\..\..\include\Camera.h"\
	"..\..\..\include\jeTypes.h"\
	"..\..\..\include\PixelFormat.h"\
	"..\..\..\include\Vec3d.h"\
	"..\..\..\include\VFILE.H"\
	"..\..\..\include\Xform3d.h"\
	"..\..\Engine\JetEngine\Engine\Drivers\Dcommon.h"\
	".\glext.h"\
	".\OglDrv.h"\
	".\OglTextures.h"\
	".\wglext.h"\
	
# End Source File
# Begin Source File

SOURCE=.\OglTextures.cpp
DEP_CPP_OGLTE=\
	"..\..\..\include\BaseType.h"\
	"..\..\..\include\Camera.h"\
	"..\..\..\include\jeTypes.h"\
	"..\..\..\include\PixelFormat.h"\
	"..\..\..\include\Vec3d.h"\
	"..\..\..\include\VFILE.H"\
	"..\..\..\include\Xform3d.h"\
	"..\..\Engine\JetEngine\Engine\Drivers\Dcommon.h"\
	".\glext.h"\
	".\OglDrv.h"\
	".\OglTextures.h"\
	".\wglext.h"\
	
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl;inc;xsd"
# Begin Source File

SOURCE=.\glext.h
# End Source File
# Begin Source File

SOURCE=.\OglDrv.h
# End Source File
# Begin Source File

SOURCE=.\OglTextures.h
# End Source File
# Begin Source File

SOURCE=.\wglext.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "rc;ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe;resx"
# End Group
# End Target
# End Project
