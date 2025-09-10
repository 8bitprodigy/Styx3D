# Microsoft Developer Studio Project File - Name="ActBuild" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=ActBuild - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "ActBuild.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "ActBuild.mak" CFG="ActBuild - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "ActBuild - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "ActBuild - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "ActBuild - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "ActBuild\Release"
# PROP BASE Intermediate_Dir "ActBuild\Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
MTL=midl.exe
# ADD BASE MTL /nologo /tlb"Release\ActBuild.tlb" /win32
# ADD MTL /nologo /tlb"ActBuild\Release\ActBuild.tlb" /win32
# ADD BASE CPP /nologo /MT /W3 /GX /Ob1 /Gy /I "AStudio" /I "AStudio\Util" /I "ActBuild" /I "common" /I "fmtactor" /I "mkactor" /I "mkbody" /I "mkmotion" /I "mop" /I "..\..\..\Include" /I "..\..\Engine\JetEngine\Actor" /D "NDEBUG" /D "WIN32" /D "_CONSOLE" /D "ACTBUILD" /D "_MBCS" /YX /GF /c
# ADD CPP /nologo /MT /W3 /GX /Ob1 /Gy /I "AStudio" /I "AStudio\Util" /I "ActBuild" /I "common" /I "fmtactor" /I "mkactor" /I "mkbody" /I "mkmotion" /I "mop" /I "..\..\..\Include" /I "..\..\Engine\JetEngine\Actor" /D "NDEBUG" /D "WIN32" /D "_CONSOLE" /D "ACTBUILD" /D "_MBCS" /YX /GF /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib Jet3d.lib winmm.lib /nologo /subsystem:console /machine:I386 /out:"..\..\..\bin\ActBuild.exe" /pdbtype:sept /libpath:"..\..\..\lib"
# SUBTRACT BASE LINK32 /pdb:none
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib Jet3d.lib winmm.lib /nologo /subsystem:console /machine:I386 /out:"..\..\..\bin\ActBuild.exe" /pdbtype:sept /libpath:"..\..\..\lib"
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "ActBuild - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "ActBuild\Debug"
# PROP BASE Intermediate_Dir "ActBuild\Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
MTL=midl.exe
# ADD BASE MTL /nologo /tlb"ActBuild\Debug\ActBuild.tlb" /win32
# ADD MTL /nologo /tlb"ActBuild\Debug\ActBuild.tlb" /win32
# ADD BASE CPP /nologo /MTd /W3 /GX /ZI /Od /I "AStudio" /I "AStudio\Util" /I "ActBuild" /I "common" /I "fmtactor" /I "mkactor" /I "mkbody" /I "mkmotion" /I ".\mop" /I "..\..\..\Include" /I "..\..\Engine\JetEngine\Actor" /D "_DEBUG" /D "WIN32" /D "_CONSOLE" /D "ACTBUILD" /D "_MBCS" /Fp"Debug/ActBuild.pch" /YX /Fo"ActBuild\Debug/" /Fd"ActBuild\Debug/" /GZ /c
# ADD CPP /nologo /MTd /W3 /GX /ZI /Od /I "AStudio" /I "AStudio\Util" /I "ActBuild" /I "common" /I "fmtactor" /I "mkactor" /I "mkbody" /I "mkmotion" /I ".\mop" /I "..\..\..\Include" /I "..\..\Engine\JetEngine\Actor" /D "_DEBUG" /D "WIN32" /D "_CONSOLE" /D "ACTBUILD" /D "_MBCS" /Fp"Debug/ActBuild.pch" /YX /Fo"ActBuild\Debug/" /Fd"ActBuild\Debug/" /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib Jet3Dd.lib winmm.lib /nologo /subsystem:console /pdb:"Debug\DebugActBuild.pdb" /debug /machine:I386 /out:"..\..\..\bin\ActBuild_d.exe" /pdbtype:sept /libpath:"..\..\..\lib"
# SUBTRACT BASE LINK32 /pdb:none
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib Jet3Dd.lib winmm.lib /nologo /subsystem:console /pdb:"Debug\DebugActBuild.pdb" /debug /machine:I386 /out:"..\..\..\bin\ActBuild_d.exe" /pdbtype:sept /libpath:"..\..\..\lib"
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "ActBuild - Win32 Release"
# Name "ActBuild - Win32 Debug"
# Begin Group "Makers"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\mop\Log.c
DEP_CPP_LOG_C=\
	"..\..\..\include\BaseType.h"\
	"..\..\..\include\jeMemAllocInfo.h"\
	"..\..\..\include\Ram.h"\
	".\mop\Log.h"\
	
# End Source File
# Begin Source File

SOURCE=.\mop\Log.h
# End Source File
# Begin Source File

SOURCE=.\common\maxmath.c
DEP_CPP_MAXMA=\
	"..\..\..\include\BaseType.h"\
	"..\..\..\include\Vec3d.h"\
	"..\..\..\include\Xform3d.h"\
	
# End Source File
# Begin Source File

SOURCE=.\common\maxmath.h
# End Source File
# Begin Source File

SOURCE=.\mkactor\mkactor.c
DEP_CPP_MKACT=\
	"..\..\..\include\ACTOR.H"\
	"..\..\..\include\ARRAY.H"\
	"..\..\..\include\BaseType.h"\
	"..\..\..\include\BITMAP.H"\
	"..\..\..\include\BODY.H"\
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
	"..\..\..\include\Motion.h"\
	"..\..\..\include\OBJECT.H"\
	"..\..\..\include\PATH.H"\
	"..\..\..\include\PixelFormat.h"\
	"..\..\..\include\QUATERN.H"\
	"..\..\..\include\Ram.h"\
	"..\..\..\include\Sound.h"\
	"..\..\..\include\UVMAP.H"\
	"..\..\..\include\Vec3d.h"\
	"..\..\..\include\VFILE.H"\
	"..\..\..\include\Xform3d.h"\
	"..\..\Engine\JetEngine\Actor\strblock.h"\
	".\common\MkUtil.h"\
	".\mkactor\mkactor.h"\
	
# End Source File
# Begin Source File

SOURCE=.\mkactor\mkactor.h
# End Source File
# Begin Source File

SOURCE=.\mkbody\mkbody.cpp
DEP_CPP_MKBOD=\
	"..\..\..\include\BaseType.h"\
	"..\..\..\include\BITMAP.H"\
	"..\..\..\include\BODY.H"\
	"..\..\..\include\jeMemAllocInfo.h"\
	"..\..\..\include\jePtrMgr.h"\
	"..\..\..\include\jeTypes.h"\
	"..\..\..\include\PixelFormat.h"\
	"..\..\..\include\QUATERN.H"\
	"..\..\..\include\Ram.h"\
	"..\..\..\include\UVMAP.H"\
	"..\..\..\include\Vec3d.h"\
	"..\..\..\include\VFILE.H"\
	"..\..\..\include\Xform3d.h"\
	"..\..\Engine\JetEngine\Actor\strblock.h"\
	".\common\maxmath.h"\
	".\common\MkUtil.h"\
	".\mkbody\mkbody.h"\
	".\mkbody\vph.h"\
	
# End Source File
# Begin Source File

SOURCE=.\mkbody\mkbody.h
# End Source File
# Begin Source File

SOURCE=.\mkmotion\mkmotion.c
DEP_CPP_MKMOT=\
	"..\..\..\include\ACTOR.H"\
	"..\..\..\include\ARRAY.H"\
	"..\..\..\include\BaseType.h"\
	"..\..\..\include\BITMAP.H"\
	"..\..\..\include\BODY.H"\
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
	"..\..\..\include\Motion.h"\
	"..\..\..\include\OBJECT.H"\
	"..\..\..\include\PATH.H"\
	"..\..\..\include\PixelFormat.h"\
	"..\..\..\include\QUATERN.H"\
	"..\..\..\include\Ram.h"\
	"..\..\..\include\Sound.h"\
	"..\..\..\include\UVMAP.H"\
	"..\..\..\include\Vec3d.h"\
	"..\..\..\include\VFILE.H"\
	"..\..\..\include\Xform3d.h"\
	"..\..\Engine\JetEngine\Actor\strblock.h"\
	".\common\maxmath.h"\
	".\common\MkUtil.h"\
	".\common\tdbody.h"\
	".\mkmotion\mkmotion.h"\
	
# End Source File
# Begin Source File

SOURCE=.\mkmotion\mkmotion.h
# End Source File
# Begin Source File

SOURCE=.\mop\mopshell.c
DEP_CPP_MOPSH=\
	"..\..\..\include\ACTOR.H"\
	"..\..\..\include\ARRAY.H"\
	"..\..\..\include\BaseType.h"\
	"..\..\..\include\BITMAP.H"\
	"..\..\..\include\BODY.H"\
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
	"..\..\..\include\jeMemAllocInfo.h"\
	"..\..\..\include\JEMODEL.H"\
	"..\..\..\include\jeNameMgr.h"\
	"..\..\..\include\jePlane.h"\
	"..\..\..\include\jePoly.h"\
	"..\..\..\include\jePortal.h"\
	"..\..\..\include\jeProperty.h"\
	"..\..\..\include\jePtrMgr.h"\
	"..\..\..\include\jeResource.h"\
	"..\..\..\include\jeShader.h"\
	"..\..\..\include\jeShaderInfo.h"\
	"..\..\..\include\jet.h"\
	"..\..\..\include\jeTypes.h"\
	"..\..\..\include\jeUserPoly.h"\
	"..\..\..\include\jeVersion.h"\
	"..\..\..\include\jeVertArray.h"\
	"..\..\..\include\JEWORLD.H"\
	"..\..\..\include\Motion.h"\
	"..\..\..\include\OBJECT.H"\
	"..\..\..\include\ObjectPos.h"\
	"..\..\..\include\PATH.H"\
	"..\..\..\include\PixelFormat.h"\
	"..\..\..\include\QUATERN.H"\
	"..\..\..\include\Ram.h"\
	"..\..\..\include\script_exports.h"\
	"..\..\..\include\Sound.h"\
	"..\..\..\include\UVMAP.H"\
	"..\..\..\include\Vec3d.h"\
	"..\..\..\include\VFILE.H"\
	"..\..\..\include\VideoMgr.h"\
	"..\..\..\include\Xform3d.h"\
	".\common\MkUtil.h"\
	".\mop\Log.h"\
	".\mop\mopshell.h"\
	".\mop\pop.h"\
	
# End Source File
# Begin Source File

SOURCE=.\mop\mopshell.h
# End Source File
# Begin Source File

SOURCE=.\mop\pop.c
DEP_CPP_POP_C=\
	"..\..\..\include\BaseType.h"\
	"..\..\..\include\PATH.H"\
	"..\..\..\include\QUATERN.H"\
	"..\..\..\include\Vec3d.h"\
	"..\..\..\include\VFILE.H"\
	"..\..\..\include\Xform3d.h"\
	".\common\MkUtil.h"\
	".\mop\pop.h"\
	
# End Source File
# Begin Source File

SOURCE=.\mop\pop.h
# End Source File
# Begin Source File

SOURCE=.\common\TDBody.c
DEP_CPP_TDBOD=\
	"..\..\..\include\BaseType.h"\
	"..\..\..\include\BITMAP.H"\
	"..\..\..\include\BODY.H"\
	"..\..\..\include\jeMemAllocInfo.h"\
	"..\..\..\include\jePtrMgr.h"\
	"..\..\..\include\jeTypes.h"\
	"..\..\..\include\PixelFormat.h"\
	"..\..\..\include\Ram.h"\
	"..\..\..\include\UVMAP.H"\
	"..\..\..\include\Vec3d.h"\
	"..\..\..\include\VFILE.H"\
	"..\..\..\include\Xform3d.h"\
	".\common\tdbody.h"\
	
# End Source File
# Begin Source File

SOURCE=.\mkbody\vph.c
DEP_CPP_VPH_C=\
	"..\..\..\include\BaseType.h"\
	"..\..\..\include\jeMemAllocInfo.h"\
	"..\..\..\include\Ram.h"\
	
# End Source File
# Begin Source File

SOURCE=.\mkbody\vph.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\ActBuild\ActBuild.c
DEP_CPP_ACTBU=\
	"..\..\..\include\ACTOR.H"\
	"..\..\..\include\ARRAY.H"\
	"..\..\..\include\BaseType.h"\
	"..\..\..\include\BITMAP.H"\
	"..\..\..\include\BODY.H"\
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
	"..\..\..\include\jeMemAllocInfo.h"\
	"..\..\..\include\JEMODEL.H"\
	"..\..\..\include\jeNameMgr.h"\
	"..\..\..\include\jePlane.h"\
	"..\..\..\include\jePoly.h"\
	"..\..\..\include\jePortal.h"\
	"..\..\..\include\jeProperty.h"\
	"..\..\..\include\jePtrMgr.h"\
	"..\..\..\include\jeResource.h"\
	"..\..\..\include\jeShader.h"\
	"..\..\..\include\jeShaderInfo.h"\
	"..\..\..\include\jet.h"\
	"..\..\..\include\jeTypes.h"\
	"..\..\..\include\jeUserPoly.h"\
	"..\..\..\include\jeVersion.h"\
	"..\..\..\include\jeVertArray.h"\
	"..\..\..\include\JEWORLD.H"\
	"..\..\..\include\Motion.h"\
	"..\..\..\include\OBJECT.H"\
	"..\..\..\include\ObjectPos.h"\
	"..\..\..\include\PATH.H"\
	"..\..\..\include\PixelFormat.h"\
	"..\..\..\include\QUATERN.H"\
	"..\..\..\include\Ram.h"\
	"..\..\..\include\script_exports.h"\
	"..\..\..\include\Sound.h"\
	"..\..\..\include\UVMAP.H"\
	"..\..\..\include\Vec3d.h"\
	"..\..\..\include\VFILE.H"\
	"..\..\..\include\VideoMgr.h"\
	"..\..\..\include\Xform3d.h"\
	".\ActBuild\ActBuild.h"\
	".\AStudio\AOptions.h"\
	".\AStudio\AProject.h"\
	".\AStudio\make.h"\
	".\common\MkUtil.h"\
	
# End Source File
# Begin Source File

SOURCE=.\ActBuild\ActBuild.h
# End Source File
# Begin Source File

SOURCE=.\AStudio\AOptions.c
DEP_CPP_AOPTI=\
	"..\..\..\include\BaseType.h"\
	"..\..\..\include\jeMemAllocInfo.h"\
	"..\..\..\include\Ram.h"\
	".\AStudio\AOptions.h"\
	
# End Source File
# Begin Source File

SOURCE=.\AStudio\AOptions.h
# End Source File
# Begin Source File

SOURCE=.\AStudio\AProject.c
DEP_CPP_APROJ=\
	"..\..\..\include\ACTOR.H"\
	"..\..\..\include\ARRAY.H"\
	"..\..\..\include\BaseType.h"\
	"..\..\..\include\BITMAP.H"\
	"..\..\..\include\BODY.H"\
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
	"..\..\..\include\jeShader.h"\
	"..\..\..\include\jeShaderInfo.h"\
	"..\..\..\include\jet.h"\
	"..\..\..\include\jeTypes.h"\
	"..\..\..\include\jeUserPoly.h"\
	"..\..\..\include\jeVersion.h"\
	"..\..\..\include\jeVertArray.h"\
	"..\..\..\include\JEWORLD.H"\
	"..\..\..\include\Motion.h"\
	"..\..\..\include\OBJECT.H"\
	"..\..\..\include\ObjectPos.h"\
	"..\..\..\include\PATH.H"\
	"..\..\..\include\PixelFormat.h"\
	"..\..\..\include\QUATERN.H"\
	"..\..\..\include\Ram.h"\
	"..\..\..\include\script_exports.h"\
	"..\..\..\include\Sound.h"\
	"..\..\..\include\UVMAP.H"\
	"..\..\..\include\Vec3d.h"\
	"..\..\..\include\VFILE.H"\
	"..\..\..\include\VideoMgr.h"\
	"..\..\..\include\Xform3d.h"\
	".\AStudio\AProject.h"\
	".\AStudio\Util\Array.h"\
	".\AStudio\Util\FilePath.h"\
	
# End Source File
# Begin Source File

SOURCE=.\AStudio\AProject.h
# End Source File
# Begin Source File

SOURCE=.\AStudio\Util\Array.c
DEP_CPP_ARRAY=\
	"..\..\..\include\BaseType.h"\
	"..\..\..\include\jeMemAllocInfo.h"\
	"..\..\..\include\Ram.h"\
	".\AStudio\Util\Array.h"\
	
# End Source File
# Begin Source File

SOURCE=.\AStudio\Util\Array.h
# End Source File
# Begin Source File

SOURCE=.\AStudio\Util\FilePath.c
DEP_CPP_FILEP=\
	"..\..\..\include\BaseType.h"\
	".\AStudio\Util\FilePath.h"\
	
# End Source File
# Begin Source File

SOURCE=.\AStudio\Util\FilePath.h
# End Source File
# Begin Source File

SOURCE=.\AStudio\make.c
DEP_CPP_MAKE_=\
	"..\..\..\include\ACTOR.H"\
	"..\..\..\include\ARRAY.H"\
	"..\..\..\include\BaseType.h"\
	"..\..\..\include\BITMAP.H"\
	"..\..\..\include\BODY.H"\
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
	"..\..\..\include\jeMemAllocInfo.h"\
	"..\..\..\include\JEMODEL.H"\
	"..\..\..\include\jeNameMgr.h"\
	"..\..\..\include\jePlane.h"\
	"..\..\..\include\jePoly.h"\
	"..\..\..\include\jePortal.h"\
	"..\..\..\include\jeProperty.h"\
	"..\..\..\include\jePtrMgr.h"\
	"..\..\..\include\jeResource.h"\
	"..\..\..\include\jeShader.h"\
	"..\..\..\include\jeShaderInfo.h"\
	"..\..\..\include\jet.h"\
	"..\..\..\include\jeTypes.h"\
	"..\..\..\include\jeUserPoly.h"\
	"..\..\..\include\jeVersion.h"\
	"..\..\..\include\jeVertArray.h"\
	"..\..\..\include\JEWORLD.H"\
	"..\..\..\include\Motion.h"\
	"..\..\..\include\OBJECT.H"\
	"..\..\..\include\ObjectPos.h"\
	"..\..\..\include\PATH.H"\
	"..\..\..\include\PixelFormat.h"\
	"..\..\..\include\QUATERN.H"\
	"..\..\..\include\Ram.h"\
	"..\..\..\include\script_exports.h"\
	"..\..\..\include\Sound.h"\
	"..\..\..\include\UVMAP.H"\
	"..\..\..\include\Vec3d.h"\
	"..\..\..\include\VFILE.H"\
	"..\..\..\include\VideoMgr.h"\
	"..\..\..\include\Xform3d.h"\
	".\AStudio\AOptions.h"\
	".\AStudio\AProject.h"\
	".\AStudio\make.h"\
	".\AStudio\mxscript.h"\
	".\AStudio\Util\FilePath.h"\
	".\common\MkUtil.h"\
	".\mkactor\mkactor.h"\
	".\mkbody\mkbody.h"\
	".\mkmotion\mkmotion.h"\
	".\mop\mopshell.h"\
	
# End Source File
# Begin Source File

SOURCE=.\AStudio\make.h
# End Source File
# Begin Source File

SOURCE=.\common\mkutil.c
DEP_CPP_MKUTI=\
	"..\..\..\include\BaseType.h"\
	"..\..\..\include\jeMemAllocInfo.h"\
	"..\..\..\include\Ram.h"\
	".\ActBuild\ActBuild.h"\
	".\common\MkUtil.h"\
	".\fmtactor\FmtActor.h"\
	".\mkactor\mkactor.h"\
	".\mkbody\mkbody.h"\
	".\mkmotion\mkmotion.h"\
	".\mop\mopshell.h"\
	
NODEP_CPP_MKUTI=\
	".\common\MkBVH.h"\
	
# End Source File
# Begin Source File

SOURCE=.\AStudio\mxscript.c
DEP_CPP_MXSCR=\
	"..\..\..\include\BaseType.h"\
	"..\..\..\include\jeMemAllocInfo.h"\
	"..\..\..\include\Ram.h"\
	".\AStudio\mxscript.h"\
	".\AStudio\Util\FilePath.h"\
	".\common\MkUtil.h"\
	
# End Source File
# Begin Source File

SOURCE=.\AStudio\mxscript.h
# End Source File
# Begin Source File

SOURCE=.\AStudio\Util\Util.c
DEP_CPP_UTIL_=\
	"..\..\..\include\BaseType.h"\
	"..\..\..\include\jeMemAllocInfo.h"\
	"..\..\..\include\Ram.h"\
	
# End Source File
# Begin Source File

SOURCE=.\AStudio\Util\Util.h
# End Source File
# End Target
# End Project
