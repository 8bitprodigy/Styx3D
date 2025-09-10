# Microsoft Developer Studio Project File - Name="AStudio" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=AStudio - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "AStudio.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "AStudio.mak" CFG="AStudio - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "AStudio - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "AStudio - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "AStudio - Win32 Release"

# PROP BASE Use_MFC 5
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "AStudio\Release"
# PROP BASE Intermediate_Dir "AStudio\Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /Ob1 /Gy /I "AStudio\Util" /I "common" /I "fmtactor" /I "mkactor" /I "mkbody" /I "mkmotion" /I "mop" /I "..\..\..\Include" /I "..\..\Engine\JetEngine\Actor" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /YX /GF /c
# ADD CPP /nologo /MD /W3 /GX /Ob1 /Gy /I "AStudio\Util" /I "common" /I "fmtactor" /I "mkactor" /I "mkbody" /I "mkmotion" /I "mop" /I "..\..\..\Include" /I "..\..\Engine\JetEngine\Actor" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_AFXDLL" /YX /GF /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib Jet3d.lib winmm.lib /nologo /subsystem:windows /machine:I386 /out:"..\..\..\bin\AStudio.exe" /pdbtype:sept /libpath:"..\..\..\lib"
# SUBTRACT BASE LINK32 /pdb:none
# ADD LINK32 Jet3d.lib winmm.lib /nologo /subsystem:windows /machine:I386 /out:"..\..\..\bin\AStudio.exe" /pdbtype:sept /libpath:"..\..\..\lib"
# SUBTRACT LINK32 /pdb:none /debug

!ELSEIF  "$(CFG)" == "AStudio - Win32 Debug"

# PROP BASE Use_MFC 5
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "AStudio\Debug"
# PROP BASE Intermediate_Dir "AStudio\Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "AStudio/Debug"
# PROP Intermediate_Dir "AStudio/Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /GX /ZI /Od /I "AStudio\Util" /I "common" /I "fmtactor" /I "mkactor" /I "mkbody" /I "mkmotion" /I "mop" /I "..\..\..\Include" /I "..\..\Engine\JetEngine\Actor" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /FR /YX /GZ /c
# ADD CPP /nologo /MDd /W3 /GX /ZI /Od /I "AStudio\Util" /I "common" /I "fmtactor" /I "mkactor" /I "mkbody" /I "mkmotion" /I "mop" /I "..\..\..\Include" /I "..\..\Engine\JetEngine\Actor" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_AFXDLL" /FR /YX /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib Jet3Dd.lib winmm.lib /nologo /subsystem:windows /pdb:"AStudio\Debug\DebugAStudio.pdb" /debug /machine:I386 /out:"..\..\..\bin\AStudio_d.exe" /pdbtype:sept /libpath:"..\..\..\lib"
# SUBTRACT BASE LINK32 /pdb:none
# ADD LINK32 Jet3Dd.lib winmm.lib /nologo /subsystem:windows /pdb:"AStudio\Debug\DebugAStudio.pdb" /debug /machine:I386 /out:"..\..\..\bin\AStudio_d.exe" /pdbtype:sept /libpath:"..\..\..\lib"
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "AStudio - Win32 Release"
# Name "AStudio - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
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

SOURCE=.\AStudio\AOptions.c
DEP_CPP_AOPTI=\
	"..\..\..\include\BaseType.h"\
	"..\..\..\include\jeMemAllocInfo.h"\
	"..\..\..\include\Ram.h"\
	".\AStudio\AOptions.h"\
	
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

SOURCE=.\AStudio\Util\Array.c
DEP_CPP_ARRAY=\
	"..\..\..\include\BaseType.h"\
	"..\..\..\include\jeMemAllocInfo.h"\
	"..\..\..\include\Ram.h"\
	".\AStudio\Util\Array.h"\
	
# End Source File
# Begin Source File

SOURCE=.\AStudio\AStudio.cpp
DEP_CPP_ASTUD=\
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
	".\AStudio\AStudio.h"\
	".\AStudio\BodyDlg.h"\
	".\AStudio\LogoPage.h"\
	".\AStudio\MaterialsDlg.h"\
	".\AStudio\MotionsDlg.h"\
	".\AStudio\PathsDlg.h"\
	".\AStudio\PropPage.h"\
	".\AStudio\PropSheet.h"\
	".\AStudio\SettingsDlg.h"\
	".\AStudio\StdAfx.h"\
	".\AStudio\TargetDlg.h"\
	".\AStudio\Util\FilePath.h"\
	".\AStudio\Util\Rcstring.h"\
	
# End Source File
# Begin Source File

SOURCE=.\AStudio\AStudio.rc
# ADD BASE RSC /l 0x40c /i "AStudio"
# ADD RSC /l 0x409 /i "AStudio" /i "..\..\msdev60\include" /i "..\..\msdev60\mfc\include"
# End Source File
# Begin Source File

SOURCE=.\AStudio\BodyDlg.cpp
DEP_CPP_BODYD=\
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
	".\AStudio\BodyDlg.h"\
	".\AStudio\MakeHelp.h"\
	".\AStudio\MyFileDlg.h"\
	".\AStudio\PropPage.h"\
	".\AStudio\StdAfx.h"\
	".\common\MkUtil.h"\
	
# End Source File
# Begin Source File

SOURCE=.\AStudio\Util\FilePath.c
DEP_CPP_FILEP=\
	"..\..\..\include\BaseType.h"\
	".\AStudio\Util\FilePath.h"\
	
# End Source File
# Begin Source File

SOURCE=.\AStudio\LogoPage.cpp
DEP_CPP_LOGOP=\
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
	".\AStudio\AProject.h"\
	".\AStudio\AStudio.h"\
	".\AStudio\LogoPage.h"\
	".\AStudio\StdAfx.h"\
	
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

SOURCE=.\AStudio\MakeHelp.cpp
DEP_CPP_MAKEH=\
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
	".\AStudio\MakeHelp.h"\
	".\AStudio\StdAfx.h"\
	".\AStudio\Util\FilePath.h"\
	".\common\MkUtil.h"\
	
# End Source File
# Begin Source File

SOURCE=.\AStudio\MaterialsDlg.cpp
DEP_CPP_MATER=\
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
	".\AStudio\MakeHelp.h"\
	".\AStudio\MaterialsDlg.h"\
	".\AStudio\MyFileDlg.h"\
	".\AStudio\PropPage.h"\
	".\AStudio\StdAfx.h"\
	".\AStudio\TextInputDlg.h"\
	".\AStudio\Util\FilePath.h"\
	".\common\MkUtil.h"\
	
# End Source File
# Begin Source File

SOURCE=.\AStudio\MotionsDlg.cpp
DEP_CPP_MOTIO=\
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
	".\AStudio\MakeHelp.h"\
	".\AStudio\MotionsDlg.h"\
	".\AStudio\MyFileDlg.h"\
	".\AStudio\PropPage.h"\
	".\AStudio\StdAfx.h"\
	".\AStudio\TextInputDlg.h"\
	".\AStudio\Util\FilePath.h"\
	".\common\MkUtil.h"\
	
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

SOURCE=.\AStudio\MyFileDlg.cpp
DEP_CPP_MYFIL=\
	".\AStudio\MyFileDlg.h"\
	".\AStudio\StdAfx.h"\
	
# End Source File
# Begin Source File

SOURCE=.\AStudio\NewPrjDlg.cpp
DEP_CPP_NEWPR=\
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
	".\AStudio\AProject.h"\
	".\AStudio\AStudio.h"\
	".\AStudio\MyFileDlg.h"\
	".\AStudio\NewPrjDlg.h"\
	".\AStudio\StdAfx.h"\
	
# End Source File
# Begin Source File

SOURCE=.\AStudio\PathsDlg.cpp
DEP_CPP_PATHS=\
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
	".\AStudio\PathsDlg.h"\
	".\AStudio\PropPage.h"\
	".\AStudio\StdAfx.h"\
	
# End Source File
# Begin Source File

SOURCE=.\AStudio\PropPage.cpp
DEP_CPP_PROPP=\
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
	".\AStudio\PropPage.h"\
	".\AStudio\StdAfx.h"\
	
# End Source File
# Begin Source File

SOURCE=.\AStudio\PropSheet.cpp
DEP_CPP_PROPS=\
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
	".\AStudio\BodyDlg.h"\
	".\AStudio\LogoPage.h"\
	".\AStudio\make.h"\
	".\AStudio\MakeHelp.h"\
	".\AStudio\MaterialsDlg.h"\
	".\AStudio\MotionsDlg.h"\
	".\AStudio\MyFileDlg.h"\
	".\AStudio\NewPrjDlg.h"\
	".\AStudio\PathsDlg.h"\
	".\AStudio\PropPage.h"\
	".\AStudio\PropSheet.h"\
	".\AStudio\SettingsDlg.h"\
	".\AStudio\StdAfx.h"\
	".\AStudio\TargetDlg.h"\
	".\AStudio\Util\FilePath.h"\
	".\AStudio\Util\Rcstring.h"\
	".\common\MkUtil.h"\
	
# End Source File
# Begin Source File

SOURCE=.\AStudio\Util\Rcstring.c
DEP_CPP_RCSTR=\
	".\AStudio\Util\Rcstring.h"\
	
# End Source File
# Begin Source File

SOURCE=.\AStudio\SettingsDlg.cpp
DEP_CPP_SETTI=\
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
	".\AStudio\AStudio.h"\
	".\AStudio\MyFileDlg.h"\
	".\AStudio\PropPage.h"\
	".\AStudio\SettingsDlg.h"\
	".\AStudio\StdAfx.h"\
	
# End Source File
# Begin Source File

SOURCE=.\AStudio\StdAfx.cpp
DEP_CPP_STDAF=\
	".\AStudio\StdAfx.h"\
	
# End Source File
# Begin Source File

SOURCE=.\AStudio\TargetDlg.cpp
DEP_CPP_TARGE=\
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
	".\AStudio\MakeHelp.h"\
	".\AStudio\MyFileDlg.h"\
	".\AStudio\PropPage.h"\
	".\AStudio\StdAfx.h"\
	".\AStudio\TargetDlg.h"\
	".\common\MkUtil.h"\
	
# End Source File
# Begin Source File

SOURCE=.\AStudio\TextInputDlg.cpp
DEP_CPP_TEXTI=\
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
	".\AStudio\AProject.h"\
	".\AStudio\AStudio.h"\
	".\AStudio\StdAfx.h"\
	".\AStudio\TextInputDlg.h"\
	
# End Source File
# Begin Source File

SOURCE=.\AStudio\Util\Util.c
DEP_CPP_UTIL_=\
	"..\..\..\include\BaseType.h"\
	"..\..\..\include\jeMemAllocInfo.h"\
	"..\..\..\include\Ram.h"\
	
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\AStudio\AOptions.h
# End Source File
# Begin Source File

SOURCE=.\AStudio\AProject.h
# End Source File
# Begin Source File

SOURCE=.\AStudio\Util\Array.h
# End Source File
# Begin Source File

SOURCE=.\AStudio\AStudio.h
# End Source File
# Begin Source File

SOURCE=.\AStudio\BodyDlg.h
# End Source File
# Begin Source File

SOURCE=.\AStudio\Util\FilePath.h
# End Source File
# Begin Source File

SOURCE=.\AStudio\LogoPage.h
# End Source File
# Begin Source File

SOURCE=.\AStudio\make.h
# End Source File
# Begin Source File

SOURCE=.\AStudio\MakeHelp.h
# End Source File
# Begin Source File

SOURCE=.\AStudio\MaterialsDlg.h
# End Source File
# Begin Source File

SOURCE=.\AStudio\MotionsDlg.h
# End Source File
# Begin Source File

SOURCE=.\AStudio\mxscript.h
# End Source File
# Begin Source File

SOURCE=.\AStudio\MyFileDlg.h
# End Source File
# Begin Source File

SOURCE=.\AStudio\NewPrjDlg.h
# End Source File
# Begin Source File

SOURCE=.\AStudio\PathsDlg.h
# End Source File
# Begin Source File

SOURCE=.\AStudio\PropPage.h
# End Source File
# Begin Source File

SOURCE=.\AStudio\PropSheet.h
# End Source File
# Begin Source File

SOURCE=.\AStudio\Util\Rcstring.h
# End Source File
# Begin Source File

SOURCE=.\AStudio\resource.h
# End Source File
# Begin Source File

SOURCE=.\AStudio\SettingsDlg.h
# End Source File
# Begin Source File

SOURCE=.\AStudio\StdAfx.h
# End Source File
# Begin Source File

SOURCE=.\AStudio\TargetDlg.h
# End Source File
# Begin Source File

SOURCE=.\AStudio\TextInputDlg.h
# End Source File
# Begin Source File

SOURCE=.\AStudio\Util\Util.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;cnt;rtf;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\AStudio\Res\Actview.bmp
# End Source File
# Begin Source File

SOURCE=.\AStudio\Res\AStudio.ico
# End Source File
# Begin Source File

SOURCE=.\AStudio\Res\AStudio.rc2
# End Source File
# End Group
# Begin Group "Help Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\AStudio\Hlp\AfxDlg.rtf
# End Source File
# Begin Source File

SOURCE=.\AStudio\Hlp\AStudio.cnt
# End Source File
# Begin Source File

SOURCE=.\AStudio\MakeHelp.bat
# End Source File
# End Group
# Begin Source File

SOURCE=.\AStudio\FileFormat.txt
# End Source File
# End Target
# End Project
