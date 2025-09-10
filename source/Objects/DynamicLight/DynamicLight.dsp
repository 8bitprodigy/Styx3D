# Microsoft Developer Studio Project File - Name="DynamicLight" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=DynamicLight - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "DynamicLight.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "DynamicLight.mak" CFG="DynamicLight - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "DynamicLight - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "DynamicLight - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "DynamicLight - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir ".\Release"
# PROP BASE Intermediate_Dir ".\Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W4 /GX /Ob1 /Gy /I "..\..\..\include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_USRDLL" /D "DYNAMICLIGHT_EXPORTS" /D "_MBCS" /GF /c
# ADD CPP /nologo /MD /W4 /GX /Ob1 /Gy /I "..\..\..\include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_USRDLL" /D "DYNAMICLIGHT_EXPORTS" /D "_MBCS" /GF /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib jet3d.lib winmm.lib /nologo /subsystem:windows /dll /machine:I386 /out:"..\..\..\bin\Objects\DynLightObj.dll" /pdbtype:sept /libpath:"..\..\..\lib"
# SUBTRACT BASE LINK32 /pdb:none
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib jet3d.lib winmm.lib /nologo /subsystem:windows /dll /machine:I386 /out:"..\..\..\bin\Objects\DynLightObj.dll" /pdbtype:sept /libpath:"..\..\..\lib"
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "DynamicLight - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir ".\Debug"
# PROP BASE Intermediate_Dir ".\Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir ".\Debug"
# PROP Intermediate_Dir ".\Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W4 /GX /ZI /Od /I "..\..\..\include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_USRDLL" /D "DYNAMICLIGHT_EXPORTS" /D "_MBCS" /GZ /c
# ADD CPP /nologo /MDd /W4 /GX /ZI /Od /I "..\..\..\include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_USRDLL" /D "DYNAMICLIGHT_EXPORTS" /D "_MBCS" /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /x /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
# SUBTRACT RSC /x
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib jet3dD.lib winmm.lib /nologo /subsystem:windows /dll /incremental:no /map /debug /machine:I386 /out:"..\..\..\bin\Objects\DynLightObj.ddl" /pdbtype:sept /libpath:"..\..\..\lib"
# SUBTRACT BASE LINK32 /pdb:none
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib jet3dD.lib winmm.lib /nologo /subsystem:windows /dll /incremental:no /map /debug /machine:I386 /out:"..\..\..\bin\Objects\DynLightObj.ddl" /pdbtype:sept /libpath:"..\..\..\lib"
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "DynamicLight - Win32 Release"
# Name "DynamicLight - Win32 Debug"
# Begin Group "Source"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Source\DynamicLight.c
DEP_CPP_DYNAM=\
	"..\..\..\include\ARRAY.H"\
	"..\..\..\include\BaseType.h"\
	"..\..\..\include\BITMAP.H"\
	"..\..\..\include\Camera.h"\
	"..\..\..\include\ENGINE.H"\
	"..\..\..\include\Errorlog.h"\
	"..\..\..\include\ExtBox.h"\
	"..\..\..\include\jeBrush.h"\
	"..\..\..\include\jeBSP.h"\
	"..\..\..\include\jeChain.h"\
	"..\..\..\include\jeFaceInfo.h"\
	"..\..\..\include\jeFrustum.h"\
	"..\..\..\include\jeGArray.h"\
	"..\..\..\include\jeLight.h"\
	"..\..\..\include\jeMaterial.h"\
	"..\..\..\include\jeMemAllocInfo.h"\
	"..\..\..\include\JEMODEL.H"\
	"..\..\..\include\jeNameMgr.h"\
	"..\..\..\include\jePlane.h"\
	"..\..\..\include\jePoly.h"\
	"..\..\..\include\jePortal.h"\
	"..\..\..\include\jeProperty.h"\
	"..\..\..\include\jePtrMgr.h"\
	"..\..\..\include\jeResource.h"\
	"..\..\..\include\jeTypes.h"\
	"..\..\..\include\jeUserPoly.h"\
	"..\..\..\include\jeVertArray.h"\
	"..\..\..\include\JEWORLD.H"\
	"..\..\..\include\OBJECT.H"\
	"..\..\..\include\PixelFormat.h"\
	"..\..\..\include\Ram.h"\
	"..\..\..\include\Sound.h"\
	"..\..\..\include\Vec3d.h"\
	"..\..\..\include\VFILE.H"\
	"..\..\..\include\Xform3d.h"\
	".\Source\DynamicLight.h"\
	".\Source\resource.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Source\DynamicLight.h
# End Source File
# Begin Source File

SOURCE=.\Source\ObjectDef.c
DEP_CPP_OBJEC=\
	"..\..\..\include\ARRAY.H"\
	"..\..\..\include\BaseType.h"\
	"..\..\..\include\BITMAP.H"\
	"..\..\..\include\Camera.h"\
	"..\..\..\include\ENGINE.H"\
	"..\..\..\include\ExtBox.h"\
	"..\..\..\include\jeBrush.h"\
	"..\..\..\include\jeBSP.h"\
	"..\..\..\include\jeChain.h"\
	"..\..\..\include\jeFaceInfo.h"\
	"..\..\..\include\jeFrustum.h"\
	"..\..\..\include\jeGArray.h"\
	"..\..\..\include\jeLight.h"\
	"..\..\..\include\jeMaterial.h"\
	"..\..\..\include\JEMODEL.H"\
	"..\..\..\include\jeNameMgr.h"\
	"..\..\..\include\jePlane.h"\
	"..\..\..\include\jePoly.h"\
	"..\..\..\include\jePortal.h"\
	"..\..\..\include\jeProperty.h"\
	"..\..\..\include\jePtrMgr.h"\
	"..\..\..\include\jeResource.h"\
	"..\..\..\include\jeTypes.h"\
	"..\..\..\include\jeUserPoly.h"\
	"..\..\..\include\jeVersion.h"\
	"..\..\..\include\jeVertArray.h"\
	"..\..\..\include\JEWORLD.H"\
	"..\..\..\include\OBJECT.H"\
	"..\..\..\include\PixelFormat.h"\
	"..\..\..\include\Sound.h"\
	"..\..\..\include\Vec3d.h"\
	"..\..\..\include\VFILE.H"\
	"..\..\..\include\Xform3d.h"\
	".\Source\DynamicLight.h"\
	".\Source\ObjectDef.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Source\ObjectDef.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\Source\DynamicLight.rc

!IF  "$(CFG)" == "DynamicLight - Win32 Release"

# ADD BASE RSC /l 0x40c /i "Source"
# ADD RSC /l 0x409 /i "Source"

!ELSEIF  "$(CFG)" == "DynamicLight - Win32 Debug"

# ADD BASE RSC /l 0x40c /i "Source"
# ADD RSC /l 0x419 /i "Source" /i "..\..\..\msdev60\include" /i "..\..\..\msdev60\mfc\include"

!ENDIF 

# End Source File
# End Target
# End Project
