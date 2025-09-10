# Microsoft Developer Studio Project File - Name="jDesigner3D" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=jDesigner3D - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "jwe.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "jwe.mak" CFG="jDesigner3D - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "jDesigner3D - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE "jDesigner3D - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "jDesigner3D - Win32 Debug"

# PROP BASE Use_MFC 5
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "..\..\..\bin"
# PROP BASE Intermediate_Dir ".\Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 5
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\..\..\bin"
# PROP Intermediate_Dir ".\Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /GX /ZI /Od /I "..\..\..\include" /I ".\\" /I "Core" /I "Dialogs" /I "j3dmfc" /I "jet" /I "Util" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /GZ /c
# ADD CPP /nologo /MTd /W3 /GX /ZI /Od /I "..\..\..\include" /I ".\\" /I "Core" /I "Dialogs" /I "j3dmfc" /I "jet" /I "Util" /I "Dialogs\DialogHelpers" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /FR /YX /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib jet3dD.lib nafxcwd.lib winmm.lib /nologo /subsystem:windows /pdb:".\Debug\DebugJEdit.pdb" /map:".\Debug\DebugJEdit.map" /debug /machine:IX86 /out:"..\..\..\bin\jDesigner3D_D.exe" /pdbtype:sept /libpath:"..\..\..\lib"
# SUBTRACT BASE LINK32 /pdb:none
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib jet3dD.lib nafxcwd.lib winmm.lib /nologo /subsystem:windows /pdb:".\Debug\DebugJEdit.pdb" /map:".\Debug\DebugJEdit.map" /debug /machine:IX86 /out:"..\..\..\bin\jDesigner3D_D.exe" /pdbtype:sept /libpath:"..\..\..\lib"
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "jDesigner3D - Win32 Release"

# PROP BASE Use_MFC 5
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "..\..\..\bin"
# PROP BASE Intermediate_Dir ".\Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 5
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /Ot /Ob1 /Gy /I "..\..\..\include" /I ".\\" /I "Core" /I "Dialogs" /I "j3dmfc" /I "jet" /I "Util" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /FR /YX /GA /GF /c
# ADD CPP /nologo /MT /W3 /GX /Ob2 /Gy /I "..\..\..\include" /I ".\\" /I "Core" /I "Dialogs" /I "j3dmfc" /I "jet" /I "Util" /I "Dialogs\DialogHelpers" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USE_BITMAPS" /FR /YX /GA /GF /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib nafxcw.lib winmm.lib Jet3d.lib /nologo /subsystem:windows /pdb:".\Release\JEdit.pdb" /machine:I386 /out:"..\..\..\bin\jDesigner3D.exe" /pdbtype:sept /libpath:"..\..\..\lib"
# SUBTRACT BASE LINK32 /pdb:none
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib nafxcw.lib winmm.lib Jet3d.lib /nologo /subsystem:windows /pdb:".\Release\JEdit.pdb" /map /machine:I386 /out:"..\..\..\bin\jDesigner3D.exe" /pdbtype:sept /libpath:"..\..\..\lib"
# SUBTRACT LINK32 /pdb:none /debug
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Desc=Copy resource files...
PostBuild_Cmds=copy             res\vertex.bmp             ..\            	copy             res\svertex.bmp             ..\ 
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "jDesigner3D - Win32 Debug"
# Name "jDesigner3D - Win32 Release"
# Begin Group "Core"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\Core\Brush.c
DEP_CPP_BRUSH=\
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
	"..\..\..\include\jePlane.h"\
	"..\..\..\include\jePoly.h"\
	"..\..\..\include\jePortal.h"\
	"..\..\..\include\jeProperty.h"\
	"..\..\..\include\jePtrMgr.h"\
	"..\..\..\include\jeResource.h"\
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
	"..\..\..\include\Sound.h"\
	"..\..\..\include\TERRAIN.H"\
	"..\..\..\include\UVMAP.H"\
	"..\..\..\include\Vec3d.h"\
	"..\..\..\include\VFILE.H"\
	"..\..\..\include\VideoMgr.h"\
	"..\..\..\include\Xform3d.h"\
	".\Core\Brush.h"\
	".\Core\BrushList.h"\
	".\Core\BrushTemplate.h"\
	".\Core\CameraList.h"\
	".\Core\CamObj.h"\
	".\Core\Class.h"\
	".\Core\Descriptor.h"\
	".\Core\Entity.h"\
	".\Core\EntityList.h"\
	".\Core\Facelist.h"\
	".\Core\Group.h"\
	".\Core\GroupList.h"\
	".\Core\jwObject.h"\
	".\Core\Level.h"\
	".\Core\Light.h"\
	".\Core\LightList.h"\
	".\Core\MaterialIdentList.h"\
	".\Core\MaterialList.h"\
	".\Core\Materials.h"\
	".\Core\Model.h"\
	".\Core\modellist.h"\
	".\Core\ObjectDef.h"\
	".\Core\ObjectList.h"\
	".\Core\Ortho.h"\
	".\Core\TernList.h"\
	".\Core\TerrnObj.h"\
	".\Core\Transform.h"\
	".\Core\Undo.h"\
	".\Core\VertList.h"\
	".\Defs.h"\
	".\jet\Symbol.h"\
	".\Util\DrawTool.h"\
	".\Util\jeList.h"\
	".\Util\Point.h"\
	".\Util\Rect.h"\
	".\Util\Units.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Core\Brush.h
# End Source File
# Begin Source File

SOURCE=.\Core\BrushList.c
DEP_CPP_BRUSHL=\
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
	"..\..\..\include\JEMODEL.H"\
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
	"..\..\..\include\Sound.h"\
	"..\..\..\include\Vec3d.h"\
	"..\..\..\include\VFILE.H"\
	"..\..\..\include\Xform3d.h"\
	".\Core\Brush.h"\
	".\Core\BrushList.h"\
	".\Core\BrushTemplate.h"\
	".\Core\Descriptor.h"\
	".\Core\Group.h"\
	".\Core\jwObject.h"\
	".\Core\MaterialIdentList.h"\
	".\Core\Model.h"\
	".\Core\ObjectList.h"\
	".\Core\Ortho.h"\
	".\Core\Undo.h"\
	".\Core\VertList.h"\
	".\Defs.h"\
	".\Util\jeList.h"\
	".\Util\Point.h"\
	".\Util\Rect.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Core\BrushList.h
# End Source File
# Begin Source File

SOURCE=.\Core\BrushTemplate.c
DEP_CPP_BRUSHT=\
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
	"..\..\..\include\jePlane.h"\
	"..\..\..\include\jePoly.h"\
	"..\..\..\include\jePortal.h"\
	"..\..\..\include\jeProperty.h"\
	"..\..\..\include\jePtrMgr.h"\
	"..\..\..\include\jeResource.h"\
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
	"..\..\..\include\Sound.h"\
	"..\..\..\include\UVMAP.H"\
	"..\..\..\include\Vec3d.h"\
	"..\..\..\include\VFILE.H"\
	"..\..\..\include\VideoMgr.h"\
	"..\..\..\include\Xform3d.h"\
	".\Core\BrushTemplate.h"\
	".\Defs.h"\
	".\Util\Units.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Core\BrushTemplate.h
# End Source File
# Begin Source File

SOURCE=.\Core\CameraList.c
DEP_CPP_CAMER=\
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
	".\Core\CameraList.h"\
	".\Core\CamObj.h"\
	".\Core\Descriptor.h"\
	".\Core\Group.h"\
	".\Core\jwObject.h"\
	".\Core\MaterialIdentList.h"\
	".\Core\ObjectList.h"\
	".\Core\Ortho.h"\
	".\Defs.h"\
	".\Util\jeList.h"\
	".\Util\Point.h"\
	".\Util\Rect.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Core\CameraList.h
# End Source File
# Begin Source File

SOURCE=.\Core\CamObj.c
DEP_CPP_CAMOB=\
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
	".\CamFieldID.h"\
	".\Core\Brush.h"\
	".\Core\BrushTemplate.h"\
	".\Core\CamObj.h"\
	".\Core\Descriptor.h"\
	".\Core\Group.h"\
	".\Core\jwObject.h"\
	".\Core\MaterialIdentList.h"\
	".\Core\ObjectDef.h"\
	".\Core\ObjectList.h"\
	".\Core\Ortho.h"\
	".\Core\Undo.h"\
	".\Core\VertList.h"\
	".\Defs.h"\
	".\EditMsg.h"\
	".\Util\DrawTool.h"\
	".\Util\jeList.h"\
	".\Util\Point.h"\
	".\Util\Rect.h"\
	".\Util\Units.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Core\CamObj.h
# End Source File
# Begin Source File

SOURCE=.\Core\Class.c
DEP_CPP_CLASS=\
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
	".\Core\Brush.h"\
	".\Core\BrushTemplate.h"\
	".\Core\Class.h"\
	".\Core\Descriptor.h"\
	".\Core\Group.h"\
	".\Core\jwObject.h"\
	".\Core\Light.h"\
	".\Core\MaterialIdentList.h"\
	".\Core\ObjectDef.h"\
	".\Core\ObjectList.h"\
	".\Core\Ortho.h"\
	".\Core\Undo.h"\
	".\Core\UserObj.h"\
	".\Core\VertList.h"\
	".\Defs.h"\
	".\Util\DrawTool.h"\
	".\Util\jeList.h"\
	".\Util\Point.h"\
	".\Util\Rect.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Core\Class.h
# End Source File
# Begin Source File

SOURCE=.\Core\Descriptor.c
DEP_CPP_DESCR=\
	"..\..\..\include\BaseType.h"\
	"..\..\..\include\Errorlog.h"\
	"..\..\..\include\jeMemAllocInfo.h"\
	"..\..\..\include\Ram.h"\
	"..\..\..\include\Vec3d.h"\
	".\Core\Descriptor.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Core\Descriptor.h
# End Source File
# Begin Source File

SOURCE=.\Core\Draw.c
DEP_CPP_DRAW_=\
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
	"..\..\..\include\jePlane.h"\
	"..\..\..\include\jePoly.h"\
	"..\..\..\include\jePortal.h"\
	"..\..\..\include\jeProperty.h"\
	"..\..\..\include\jePtrMgr.h"\
	"..\..\..\include\jeResource.h"\
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
	"..\..\..\include\Sound.h"\
	"..\..\..\include\TERRAIN.H"\
	"..\..\..\include\UVMAP.H"\
	"..\..\..\include\Vec3d.h"\
	"..\..\..\include\VFILE.H"\
	"..\..\..\include\VideoMgr.h"\
	"..\..\..\include\Xform3d.h"\
	".\AppData.h"\
	".\Core\Brush.h"\
	".\Core\BrushList.h"\
	".\Core\BrushTemplate.h"\
	".\Core\CameraList.h"\
	".\Core\CamObj.h"\
	".\Core\Class.h"\
	".\Core\Descriptor.h"\
	".\Core\Draw.h"\
	".\Core\Entity.h"\
	".\Core\EntityList.h"\
	".\Core\Group.h"\
	".\Core\GroupList.h"\
	".\Core\jwObject.h"\
	".\Core\Level.h"\
	".\Core\Light.h"\
	".\Core\LightList.h"\
	".\Core\MaterialIdentList.h"\
	".\Core\MaterialList.h"\
	".\Core\Materials.h"\
	".\Core\Model.h"\
	".\Core\modellist.h"\
	".\Core\ObjectList.h"\
	".\Core\Ortho.h"\
	".\Core\TernList.h"\
	".\Core\TerrnObj.h"\
	".\Core\Undo.h"\
	".\Core\UserObj.h"\
	".\Core\VertList.h"\
	".\Defs.h"\
	".\jet\Symbol.h"\
	".\Settings.h"\
	".\Util\DrawTool.h"\
	".\Util\jeList.h"\
	".\Util\Point.h"\
	".\Util\Rect.h"\
	".\Util\Units.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Core\Draw.h
# End Source File
# Begin Source File

SOURCE=.\Core\Draw3d.c
DEP_CPP_DRAW3=\
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
	"..\..\..\include\jePlane.h"\
	"..\..\..\include\jePoly.h"\
	"..\..\..\include\jePortal.h"\
	"..\..\..\include\jeProperty.h"\
	"..\..\..\include\jePtrMgr.h"\
	"..\..\..\include\jeResource.h"\
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
	"..\..\..\include\Sound.h"\
	"..\..\..\include\TERRAIN.H"\
	"..\..\..\include\UVMAP.H"\
	"..\..\..\include\Vec3d.h"\
	"..\..\..\include\VFILE.H"\
	"..\..\..\include\VideoMgr.h"\
	"..\..\..\include\Xform3d.h"\
	".\Core\Brush.h"\
	".\Core\BrushList.h"\
	".\Core\BrushTemplate.h"\
	".\Core\CameraList.h"\
	".\Core\CamObj.h"\
	".\Core\Class.h"\
	".\Core\Descriptor.h"\
	".\Core\Draw3d.h"\
	".\Core\Entity.h"\
	".\Core\EntityList.h"\
	".\Core\Group.h"\
	".\Core\GroupList.h"\
	".\Core\jwObject.h"\
	".\Core\Level.h"\
	".\Core\Light.h"\
	".\Core\LightList.h"\
	".\Core\MaterialIdentList.h"\
	".\Core\MaterialList.h"\
	".\Core\Materials.h"\
	".\Core\Model.h"\
	".\Core\modellist.h"\
	".\Core\ObjectList.h"\
	".\Core\Ortho.h"\
	".\Core\TernList.h"\
	".\Core\TerrnObj.h"\
	".\Core\Undo.h"\
	".\Core\VertList.h"\
	".\Defs.h"\
	".\jet\Symbol.h"\
	".\Util\jeList.h"\
	".\Util\Point.h"\
	".\Util\Rect.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Core\Draw3d.h
# End Source File
# Begin Source File

SOURCE=.\Core\Facelist.c
DEP_CPP_FACEL=\
	"..\..\..\include\ARRAY.H"\
	"..\..\..\include\BaseType.h"\
	"..\..\..\include\BITMAP.H"\
	"..\..\..\include\Camera.h"\
	"..\..\..\include\ENGINE.H"\
	"..\..\..\include\ExtBox.h"\
	"..\..\..\include\jeBrush.h"\
	"..\..\..\include\jeChain.h"\
	"..\..\..\include\jeFaceInfo.h"\
	"..\..\..\include\jeFrustum.h"\
	"..\..\..\include\jeGArray.h"\
	"..\..\..\include\jeMaterial.h"\
	"..\..\..\include\jePlane.h"\
	"..\..\..\include\jePoly.h"\
	"..\..\..\include\jePortal.h"\
	"..\..\..\include\jeProperty.h"\
	"..\..\..\include\jePtrMgr.h"\
	"..\..\..\include\jeTypes.h"\
	"..\..\..\include\jeVertArray.h"\
	"..\..\..\include\OBJECT.H"\
	"..\..\..\include\PixelFormat.h"\
	"..\..\..\include\Sound.h"\
	"..\..\..\include\Vec3d.h"\
	"..\..\..\include\VFILE.H"\
	"..\..\..\include\Xform3d.h"\
	".\Core\Facelist.h"\
	".\Util\jeList.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Core\Facelist.h
# End Source File
# Begin Source File

SOURCE=.\Core\Group.c
DEP_CPP_GROUP=\
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
	".\Core\Brush.h"\
	".\Core\BrushTemplate.h"\
	".\Core\Descriptor.h"\
	".\Core\Group.h"\
	".\Core\jwObject.h"\
	".\Core\MaterialIdentList.h"\
	".\Core\ObjectList.h"\
	".\Core\Ortho.h"\
	".\Core\Undo.h"\
	".\Core\VertList.h"\
	".\Defs.h"\
	".\Util\jeList.h"\
	".\Util\Point.h"\
	".\Util\Rect.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Core\Group.h
# End Source File
# Begin Source File

SOURCE=.\Core\GroupList.c
DEP_CPP_GROUPL=\
	"..\..\..\include\ARRAY.H"\
	"..\..\..\include\BaseType.h"\
	"..\..\..\include\BITMAP.H"\
	"..\..\..\include\Camera.h"\
	"..\..\..\include\ENGINE.H"\
	"..\..\..\include\Errorlog.h"\
	"..\..\..\include\ExtBox.h"\
	"..\..\..\include\jeChain.h"\
	"..\..\..\include\jeFrustum.h"\
	"..\..\..\include\jeMaterial.h"\
	"..\..\..\include\jePlane.h"\
	"..\..\..\include\jeProperty.h"\
	"..\..\..\include\jePtrMgr.h"\
	"..\..\..\include\jeTypes.h"\
	"..\..\..\include\OBJECT.H"\
	"..\..\..\include\PixelFormat.h"\
	"..\..\..\include\Sound.h"\
	"..\..\..\include\Vec3d.h"\
	"..\..\..\include\VFILE.H"\
	"..\..\..\include\Xform3d.h"\
	".\Core\Group.h"\
	".\Core\GroupList.h"\
	".\Core\jwObject.h"\
	".\Core\MaterialIdentList.h"\
	".\Core\ObjectList.h"\
	".\Core\Ortho.h"\
	".\Defs.h"\
	".\Util\jeList.h"\
	".\Util\Point.h"\
	".\Util\Rect.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Core\GroupList.h
# End Source File
# Begin Source File

SOURCE=.\Core\jwObject.h
# End Source File
# Begin Source File

SOURCE=.\Core\Level.c
DEP_CPP_LEVEL=\
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
	"..\..\..\include\jePlane.h"\
	"..\..\..\include\jePoly.h"\
	"..\..\..\include\jePortal.h"\
	"..\..\..\include\jeProperty.h"\
	"..\..\..\include\jePtrMgr.h"\
	"..\..\..\include\jeResource.h"\
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
	"..\..\..\include\Sound.h"\
	"..\..\..\include\TERRAIN.H"\
	"..\..\..\include\UVMAP.H"\
	"..\..\..\include\Vec3d.h"\
	"..\..\..\include\VFILE.H"\
	"..\..\..\include\VideoMgr.h"\
	"..\..\..\include\Xform3d.h"\
	".\Core\Brush.h"\
	".\Core\BrushList.h"\
	".\Core\BrushTemplate.h"\
	".\Core\CameraList.h"\
	".\Core\CamObj.h"\
	".\Core\Class.h"\
	".\Core\Descriptor.h"\
	".\Core\Entity.h"\
	".\Core\EntityList.h"\
	".\Core\Group.h"\
	".\Core\GroupList.h"\
	".\Core\jwObject.h"\
	".\Core\Level.h"\
	".\Core\Light.h"\
	".\Core\LightList.h"\
	".\Core\MaterialIdentList.h"\
	".\Core\MaterialList.h"\
	".\Core\Materials.h"\
	".\Core\MatrArray.h"\
	".\Core\MatrIdx.h"\
	".\Core\Model.h"\
	".\Core\modellist.h"\
	".\Core\ObjectList.h"\
	".\Core\Ortho.h"\
	".\Core\TernList.h"\
	".\Core\TerrnObj.h"\
	".\Core\Undo.h"\
	".\Core\UserObj.h"\
	".\Core\VertList.h"\
	".\Defs.h"\
	".\jet\Symbol.h"\
	".\Settings.h"\
	".\Util\jeList.h"\
	".\Util\Point.h"\
	".\Util\Rect.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Core\Level.h
# End Source File
# Begin Source File

SOURCE=.\Core\Light.c
DEP_CPP_LIGHT=\
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
	"..\..\..\include\jePlane.h"\
	"..\..\..\include\jePoly.h"\
	"..\..\..\include\jePortal.h"\
	"..\..\..\include\jeProperty.h"\
	"..\..\..\include\jePtrMgr.h"\
	"..\..\..\include\jeResource.h"\
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
	"..\..\..\include\Sound.h"\
	"..\..\..\include\TERRAIN.H"\
	"..\..\..\include\UVMAP.H"\
	"..\..\..\include\Vec3d.h"\
	"..\..\..\include\VFILE.H"\
	"..\..\..\include\VideoMgr.h"\
	"..\..\..\include\Xform3d.h"\
	".\Core\Brush.h"\
	".\Core\BrushList.h"\
	".\Core\BrushTemplate.h"\
	".\Core\CameraList.h"\
	".\Core\CamObj.h"\
	".\Core\Class.h"\
	".\Core\Descriptor.h"\
	".\Core\Entity.h"\
	".\Core\EntityList.h"\
	".\Core\Group.h"\
	".\Core\GroupList.h"\
	".\Core\jwObject.h"\
	".\Core\Level.h"\
	".\Core\Light.h"\
	".\Core\LightList.h"\
	".\Core\MaterialIdentList.h"\
	".\Core\MaterialList.h"\
	".\Core\Materials.h"\
	".\Core\Model.h"\
	".\Core\modellist.h"\
	".\Core\ObjectDef.h"\
	".\Core\ObjectList.h"\
	".\Core\Ortho.h"\
	".\Core\TernList.h"\
	".\Core\TerrnObj.h"\
	".\Core\Transform.h"\
	".\Core\Undo.h"\
	".\Core\VertList.h"\
	".\Defs.h"\
	".\jet\Symbol.h"\
	".\Util\DrawTool.h"\
	".\Util\jeList.h"\
	".\Util\Point.h"\
	".\Util\Rect.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Core\Light.h
# End Source File
# Begin Source File

SOURCE=.\Core\LightList.c
DEP_CPP_LIGHTL=\
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
	".\Core\Group.h"\
	".\Core\jwObject.h"\
	".\Core\Light.h"\
	".\Core\LightList.h"\
	".\Core\MaterialIdentList.h"\
	".\Core\ObjectList.h"\
	".\Core\Ortho.h"\
	".\Defs.h"\
	".\Util\jeList.h"\
	".\Util\Point.h"\
	".\Util\Rect.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Core\LightList.h
# End Source File
# Begin Source File

SOURCE=.\Core\MaterialIdentList.c
DEP_CPP_MATER=\
	"..\..\..\include\ARRAY.H"\
	"..\..\..\include\BaseType.h"\
	"..\..\..\include\BITMAP.H"\
	"..\..\..\include\Camera.h"\
	"..\..\..\include\ENGINE.H"\
	"..\..\..\include\jeMaterial.h"\
	"..\..\..\include\jeMemAllocInfo.h"\
	"..\..\..\include\jePtrMgr.h"\
	"..\..\..\include\jeTypes.h"\
	"..\..\..\include\PixelFormat.h"\
	"..\..\..\include\Ram.h"\
	"..\..\..\include\Vec3d.h"\
	"..\..\..\include\VFILE.H"\
	"..\..\..\include\Xform3d.h"\
	".\Core\MaterialIdentList.h"\
	".\Util\jeList.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Core\MaterialIdentList.h
# End Source File
# Begin Source File

SOURCE=.\Core\MaterialList.c
DEP_CPP_MATERI=\
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
	".\Core\MaterialList.h"\
	".\Core\Materials.h"\
	".\Util\jeList.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Core\MaterialList.h
# End Source File
# Begin Source File

SOURCE=.\Core\MaterialList2.c
DEP_CPP_MATERIA=\
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
	"..\..\..\include\jePlane.h"\
	"..\..\..\include\jePoly.h"\
	"..\..\..\include\jePortal.h"\
	"..\..\..\include\jeProperty.h"\
	"..\..\..\include\jePtrMgr.h"\
	"..\..\..\include\jeResource.h"\
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
	"..\..\..\include\Sound.h"\
	"..\..\..\include\UVMAP.H"\
	"..\..\..\include\Vec3d.h"\
	"..\..\..\include\VFILE.H"\
	"..\..\..\include\VideoMgr.h"\
	"..\..\..\include\Xform3d.h"\
	".\Core\MaterialList2.h"\
	".\Util\jeList.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Core\MaterialList2.h
# End Source File
# Begin Source File

SOURCE=.\Core\Materials.c
DEP_CPP_MATERIAL=\
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
	".\Util\Bmp.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Core\Materials.h
# End Source File
# Begin Source File

SOURCE=.\Core\MatrArray.c
DEP_CPP_MATRA=\
	"..\..\..\include\ARRAY.H"\
	"..\..\..\include\BaseType.h"\
	"..\..\..\include\BITMAP.H"\
	"..\..\..\include\Camera.h"\
	"..\..\..\include\ENGINE.H"\
	"..\..\..\include\Errorlog.h"\
	"..\..\..\include\jeMaterial.h"\
	"..\..\..\include\jeMemAllocInfo.h"\
	"..\..\..\include\jePtrMgr.h"\
	"..\..\..\include\jeTypes.h"\
	"..\..\..\include\PixelFormat.h"\
	"..\..\..\include\Ram.h"\
	"..\..\..\include\Vec3d.h"\
	"..\..\..\include\VFILE.H"\
	"..\..\..\include\Xform3d.h"\
	".\Core\MatrArray.h"\
	".\Core\MatrIdx.h"\
	".\Util\jeList.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Core\MatrArray.h
# End Source File
# Begin Source File

SOURCE=.\Core\MatrIdx.c
DEP_CPP_MATRI=\
	"..\..\..\include\ARRAY.H"\
	"..\..\..\include\BaseType.h"\
	"..\..\..\include\BITMAP.H"\
	"..\..\..\include\Camera.h"\
	"..\..\..\include\ENGINE.H"\
	"..\..\..\include\Errorlog.h"\
	"..\..\..\include\jeMaterial.h"\
	"..\..\..\include\jeMemAllocInfo.h"\
	"..\..\..\include\jePtrMgr.h"\
	"..\..\..\include\jeTypes.h"\
	"..\..\..\include\PixelFormat.h"\
	"..\..\..\include\Ram.h"\
	"..\..\..\include\Vec3d.h"\
	"..\..\..\include\VFILE.H"\
	"..\..\..\include\Xform3d.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Core\MatrIdx.h
# End Source File
# Begin Source File

SOURCE=.\Core\Model.c
DEP_CPP_MODEL=\
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
	".\Core\Brush.h"\
	".\Core\BrushList.h"\
	".\Core\BrushTemplate.h"\
	".\Core\Descriptor.h"\
	".\Core\Group.h"\
	".\Core\jwObject.h"\
	".\Core\MaterialIdentList.h"\
	".\Core\Model.h"\
	".\Core\ModelInstance.h"\
	".\Core\ObjectDef.h"\
	".\Core\ObjectList.h"\
	".\Core\Ortho.h"\
	".\Core\Undo.h"\
	".\Core\VertList.h"\
	".\Defs.h"\
	".\Util\DrawTool.h"\
	".\Util\jeList.h"\
	".\Util\Point.h"\
	".\Util\Rect.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Core\Model.h
# End Source File
# Begin Source File

SOURCE=.\Core\ModelInstance.h
# End Source File
# Begin Source File

SOURCE=.\Core\modellist.c
DEP_CPP_MODELL=\
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
	"..\..\..\include\JEMODEL.H"\
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
	"..\..\..\include\Sound.h"\
	"..\..\..\include\Vec3d.h"\
	"..\..\..\include\VFILE.H"\
	"..\..\..\include\Xform3d.h"\
	".\Core\Brush.h"\
	".\Core\BrushList.h"\
	".\Core\BrushTemplate.h"\
	".\Core\Descriptor.h"\
	".\Core\Group.h"\
	".\Core\jwObject.h"\
	".\Core\MaterialIdentList.h"\
	".\Core\Model.h"\
	".\Core\modellist.h"\
	".\Core\ObjectList.h"\
	".\Core\Ortho.h"\
	".\Core\Undo.h"\
	".\Core\VertList.h"\
	".\Defs.h"\
	".\Util\jeList.h"\
	".\Util\Point.h"\
	".\Util\Rect.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Core\modellist.h
# End Source File
# Begin Source File

SOURCE=.\Core\Object.c
DEP_CPP_OBJEC=\
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
	".\Core\Brush.h"\
	".\Core\BrushList.h"\
	".\Core\BrushTemplate.h"\
	".\Core\CamObj.h"\
	".\Core\Class.h"\
	".\Core\Descriptor.h"\
	".\Core\Group.h"\
	".\Core\jwObject.h"\
	".\Core\Light.h"\
	".\Core\MaterialIdentList.h"\
	".\Core\Model.h"\
	".\Core\ObjectDef.h"\
	".\Core\ObjectList.h"\
	".\Core\Ortho.h"\
	".\Core\Undo.h"\
	".\Core\UserObj.h"\
	".\Core\VertList.h"\
	".\Defs.h"\
	".\Util\DrawTool.h"\
	".\Util\jeList.h"\
	".\Util\Point.h"\
	".\Util\Rect.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Core\ObjectDef.h
# End Source File
# Begin Source File

SOURCE=.\Core\ObjectList.c
DEP_CPP_OBJECT=\
	"..\..\..\include\BaseType.h"\
	"..\..\..\include\BITMAP.H"\
	"..\..\..\include\Camera.h"\
	"..\..\..\include\ExtBox.h"\
	"..\..\..\include\jeChain.h"\
	"..\..\..\include\jeFrustum.h"\
	"..\..\..\include\jePlane.h"\
	"..\..\..\include\jeProperty.h"\
	"..\..\..\include\jePtrMgr.h"\
	"..\..\..\include\jeTypes.h"\
	"..\..\..\include\OBJECT.H"\
	"..\..\..\include\PixelFormat.h"\
	"..\..\..\include\Sound.h"\
	"..\..\..\include\Vec3d.h"\
	"..\..\..\include\VFILE.H"\
	"..\..\..\include\Xform3d.h"\
	".\Core\jwObject.h"\
	".\Core\ObjectList.h"\
	".\Core\Ortho.h"\
	".\Defs.h"\
	".\Util\jeList.h"\
	".\Util\Point.h"\
	".\Util\Rect.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Core\ObjectList.h
# End Source File
# Begin Source File

SOURCE=.\Core\Ortho.c
DEP_CPP_ORTHO=\
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
	"..\..\..\include\jePlane.h"\
	"..\..\..\include\jePoly.h"\
	"..\..\..\include\jePortal.h"\
	"..\..\..\include\jeProperty.h"\
	"..\..\..\include\jePtrMgr.h"\
	"..\..\..\include\jeResource.h"\
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
	"..\..\..\include\Sound.h"\
	"..\..\..\include\UVMAP.H"\
	"..\..\..\include\Vec3d.h"\
	"..\..\..\include\VFILE.H"\
	"..\..\..\include\VideoMgr.h"\
	"..\..\..\include\Xform3d.h"\
	".\Core\Ortho.h"\
	".\Defs.h"\
	".\Util\Point.h"\
	".\Util\Rect.h"\
	".\Util\Units.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Core\Ortho.h
# End Source File
# Begin Source File

SOURCE=.\Core\Select.c
DEP_CPP_SELEC=\
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
	"..\..\..\include\jePlane.h"\
	"..\..\..\include\jePoly.h"\
	"..\..\..\include\jePortal.h"\
	"..\..\..\include\jeProperty.h"\
	"..\..\..\include\jePtrMgr.h"\
	"..\..\..\include\jeResource.h"\
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
	"..\..\..\include\Sound.h"\
	"..\..\..\include\TERRAIN.H"\
	"..\..\..\include\UVMAP.H"\
	"..\..\..\include\Vec3d.h"\
	"..\..\..\include\VFILE.H"\
	"..\..\..\include\VideoMgr.h"\
	"..\..\..\include\Xform3d.h"\
	".\Core\Brush.h"\
	".\Core\BrushList.h"\
	".\Core\BrushTemplate.h"\
	".\Core\CameraList.h"\
	".\Core\CamObj.h"\
	".\Core\Class.h"\
	".\Core\Descriptor.h"\
	".\Core\Entity.h"\
	".\Core\EntityList.h"\
	".\Core\Group.h"\
	".\Core\GroupList.h"\
	".\Core\jwObject.h"\
	".\Core\Level.h"\
	".\Core\Light.h"\
	".\Core\LightList.h"\
	".\Core\MaterialIdentList.h"\
	".\Core\MaterialList.h"\
	".\Core\Materials.h"\
	".\Core\Model.h"\
	".\Core\modellist.h"\
	".\Core\ObjectList.h"\
	".\Core\Ortho.h"\
	".\Core\Select.h"\
	".\Core\TernList.h"\
	".\Core\TerrnObj.h"\
	".\Core\Undo.h"\
	".\Core\UserObj.h"\
	".\Core\VertList.h"\
	".\Defs.h"\
	".\jet\Symbol.h"\
	".\Util\jeList.h"\
	".\Util\Point.h"\
	".\Util\Rect.h"\
	".\Util\Units.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Core\Select.h
# End Source File
# Begin Source File

SOURCE=.\Core\TernList.c
DEP_CPP_TERNL=\
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
	"..\..\..\include\Sound.h"\
	"..\..\..\include\TERRAIN.H"\
	"..\..\..\include\Vec3d.h"\
	"..\..\..\include\VFILE.H"\
	"..\..\..\include\Xform3d.h"\
	".\Core\Group.h"\
	".\Core\jwObject.h"\
	".\Core\MaterialIdentList.h"\
	".\Core\ObjectList.h"\
	".\Core\Ortho.h"\
	".\Core\TernList.h"\
	".\Core\TerrnObj.h"\
	".\Defs.h"\
	".\jet\Symbol.h"\
	".\Util\jeList.h"\
	".\Util\Point.h"\
	".\Util\Rect.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Core\TernList.h
# End Source File
# Begin Source File

SOURCE=.\Core\TerrnObj.c
DEP_CPP_TERRN=\
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
	"..\..\..\include\jePlane.h"\
	"..\..\..\include\jePoly.h"\
	"..\..\..\include\jePortal.h"\
	"..\..\..\include\jeProperty.h"\
	"..\..\..\include\jePtrMgr.h"\
	"..\..\..\include\jeResource.h"\
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
	"..\..\..\include\Sound.h"\
	"..\..\..\include\TERRAIN.H"\
	"..\..\..\include\UVMAP.H"\
	"..\..\..\include\Vec3d.h"\
	"..\..\..\include\VFILE.H"\
	"..\..\..\include\VideoMgr.h"\
	"..\..\..\include\Xform3d.h"\
	".\Core\Brush.h"\
	".\Core\BrushList.h"\
	".\Core\BrushTemplate.h"\
	".\Core\CameraList.h"\
	".\Core\CamObj.h"\
	".\Core\Class.h"\
	".\Core\Descriptor.h"\
	".\Core\Entity.h"\
	".\Core\EntityList.h"\
	".\Core\Group.h"\
	".\Core\GroupList.h"\
	".\Core\jwObject.h"\
	".\Core\Level.h"\
	".\Core\Light.h"\
	".\Core\LightList.h"\
	".\Core\MaterialIdentList.h"\
	".\Core\MaterialList.h"\
	".\Core\Materials.h"\
	".\Core\Model.h"\
	".\Core\modellist.h"\
	".\Core\ObjectDef.h"\
	".\Core\ObjectList.h"\
	".\Core\Ortho.h"\
	".\Core\TernList.h"\
	".\Core\TerrnObj.h"\
	".\Core\Transform.h"\
	".\Core\Undo.h"\
	".\Core\VertList.h"\
	".\Defs.h"\
	".\jet\Symbol.h"\
	".\Util\DrawTool.h"\
	".\Util\jeList.h"\
	".\Util\Point.h"\
	".\Util\Rect.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Core\TerrnObj.h
# End Source File
# Begin Source File

SOURCE=.\Core\Transform.c
DEP_CPP_TRANS=\
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
	"..\..\..\include\jePlane.h"\
	"..\..\..\include\jePoly.h"\
	"..\..\..\include\jePortal.h"\
	"..\..\..\include\jeProperty.h"\
	"..\..\..\include\jePtrMgr.h"\
	"..\..\..\include\jeResource.h"\
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
	"..\..\..\include\Sound.h"\
	"..\..\..\include\TERRAIN.H"\
	"..\..\..\include\UVMAP.H"\
	"..\..\..\include\Vec3d.h"\
	"..\..\..\include\VFILE.H"\
	"..\..\..\include\VideoMgr.h"\
	"..\..\..\include\Xform3d.h"\
	".\Core\Brush.h"\
	".\Core\BrushList.h"\
	".\Core\BrushTemplate.h"\
	".\Core\CameraList.h"\
	".\Core\CamObj.h"\
	".\Core\Class.h"\
	".\Core\Descriptor.h"\
	".\Core\Entity.h"\
	".\Core\EntityList.h"\
	".\Core\Group.h"\
	".\Core\GroupList.h"\
	".\Core\jwObject.h"\
	".\Core\Level.h"\
	".\Core\Light.h"\
	".\Core\LightList.h"\
	".\Core\MaterialIdentList.h"\
	".\Core\MaterialList.h"\
	".\Core\Materials.h"\
	".\Core\Model.h"\
	".\Core\modellist.h"\
	".\Core\ObjectList.h"\
	".\Core\Ortho.h"\
	".\Core\TernList.h"\
	".\Core\TerrnObj.h"\
	".\Core\Transform.h"\
	".\Core\Undo.h"\
	".\Core\VertList.h"\
	".\Defs.h"\
	".\jet\Symbol.h"\
	".\Util\jeList.h"\
	".\Util\Point.h"\
	".\Util\Rect.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Core\Transform.h
# End Source File
# Begin Source File

SOURCE=.\Core\Undo.c
DEP_CPP_UNDO_=\
	"..\..\..\include\BaseType.h"\
	"..\..\..\include\BITMAP.H"\
	"..\..\..\include\Camera.h"\
	"..\..\..\include\Errorlog.h"\
	"..\..\..\include\ExtBox.h"\
	"..\..\..\include\jeChain.h"\
	"..\..\..\include\jeFrustum.h"\
	"..\..\..\include\jeMemAllocInfo.h"\
	"..\..\..\include\jePlane.h"\
	"..\..\..\include\jeProperty.h"\
	"..\..\..\include\jePtrMgr.h"\
	"..\..\..\include\jeTypes.h"\
	"..\..\..\include\OBJECT.H"\
	"..\..\..\include\PixelFormat.h"\
	"..\..\..\include\Ram.h"\
	"..\..\..\include\Sound.h"\
	"..\..\..\include\Vec3d.h"\
	"..\..\..\include\VFILE.H"\
	"..\..\..\include\Xform3d.h"\
	".\Core\jwObject.h"\
	".\Core\Ortho.h"\
	".\Core\Undo.h"\
	".\Defs.h"\
	".\Util\jeList.h"\
	".\Util\Point.h"\
	".\Util\Rect.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Core\Undo.h
# End Source File
# Begin Source File

SOURCE=.\Core\UserObj.c
DEP_CPP_USERO=\
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
	"..\..\..\include\jePlane.h"\
	"..\..\..\include\jePoly.h"\
	"..\..\..\include\jePortal.h"\
	"..\..\..\include\jeProperty.h"\
	"..\..\..\include\jePtrMgr.h"\
	"..\..\..\include\jeResource.h"\
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
	"..\..\..\include\Sound.h"\
	"..\..\..\include\TERRAIN.H"\
	"..\..\..\include\UVMAP.H"\
	"..\..\..\include\Vec3d.h"\
	"..\..\..\include\VFILE.H"\
	"..\..\..\include\VideoMgr.h"\
	"..\..\..\include\Xform3d.h"\
	".\Core\Brush.h"\
	".\Core\BrushList.h"\
	".\Core\BrushTemplate.h"\
	".\Core\CameraList.h"\
	".\Core\CamObj.h"\
	".\Core\Class.h"\
	".\Core\Descriptor.h"\
	".\Core\Entity.h"\
	".\Core\EntityList.h"\
	".\Core\Group.h"\
	".\Core\GroupList.h"\
	".\Core\jwObject.h"\
	".\Core\Level.h"\
	".\Core\Light.h"\
	".\Core\LightList.h"\
	".\Core\MaterialIdentList.h"\
	".\Core\MaterialList.h"\
	".\Core\Materials.h"\
	".\Core\Model.h"\
	".\Core\modellist.h"\
	".\Core\ObjectDef.h"\
	".\Core\ObjectList.h"\
	".\Core\Ortho.h"\
	".\Core\TernList.h"\
	".\Core\TerrnObj.h"\
	".\Core\Transform.h"\
	".\Core\Undo.h"\
	".\Core\UserObj.h"\
	".\Core\VertList.h"\
	".\Defs.h"\
	".\EditMsg.h"\
	".\jet\Symbol.h"\
	".\Util\DrawTool.h"\
	".\Util\jeList.h"\
	".\Util\Point.h"\
	".\Util\Rect.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Core\UserObj.h
# End Source File
# Begin Source File

SOURCE=.\Core\Vertlist.c
DEP_CPP_VERTL=\
	"..\..\..\include\ARRAY.H"\
	"..\..\..\include\BaseType.h"\
	"..\..\..\include\BITMAP.H"\
	"..\..\..\include\Camera.h"\
	"..\..\..\include\ENGINE.H"\
	"..\..\..\include\ExtBox.h"\
	"..\..\..\include\jeBrush.h"\
	"..\..\..\include\jeChain.h"\
	"..\..\..\include\jeFaceInfo.h"\
	"..\..\..\include\jeFrustum.h"\
	"..\..\..\include\jeGArray.h"\
	"..\..\..\include\jeMaterial.h"\
	"..\..\..\include\jePlane.h"\
	"..\..\..\include\jePoly.h"\
	"..\..\..\include\jePortal.h"\
	"..\..\..\include\jeProperty.h"\
	"..\..\..\include\jePtrMgr.h"\
	"..\..\..\include\jeTypes.h"\
	"..\..\..\include\jeVertArray.h"\
	"..\..\..\include\OBJECT.H"\
	"..\..\..\include\PixelFormat.h"\
	"..\..\..\include\Sound.h"\
	"..\..\..\include\Vec3d.h"\
	"..\..\..\include\VFILE.H"\
	"..\..\..\include\Xform3d.h"\
	".\Core\VertList.h"\
	".\Util\jeList.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Core\VertList.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\res\Addtocur.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Arch.ico
# End Source File
# Begin Source File

SOURCE=.\res\arr_no.ico
# End Source File
# Begin Source File

SOURCE=.\res\arr_nw.ico
# End Source File
# Begin Source File

SOURCE=.\res\arr_so.ico
# End Source File
# Begin Source File

SOURCE=.\res\arr_sw.ico
# End Source File
# Begin Source File

SOURCE=.\res\Bitmap1.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Camera.ico
# End Source File
# Begin Source File

SOURCE=.\res\cameracur.cur
# End Source File
# Begin Source File

SOURCE=.\res\Cone.ico
# End Source File
# Begin Source File

SOURCE=.\res\content.ico
# End Source File
# Begin Source File

SOURCE=.\res\Cube.cur
# End Source File
# Begin Source File

SOURCE=.\res\Cube.ico
# End Source File
# Begin Source File

SOURCE=.\res\Cube1.ico
# End Source File
# Begin Source File

SOURCE=.\res\Cur00001.cur
# End Source File
# Begin Source File

SOURCE=.\res\Cur00002.cur
# End Source File
# Begin Source File

SOURCE=.\res\Cur00003.cur
# End Source File
# Begin Source File

SOURCE=.\res\Cursor2.cur
# End Source File
# Begin Source File

SOURCE=.\res\CursorPlus.CUR
# End Source File
# Begin Source File

SOURCE=.\res\cut.ico
# End Source File
# Begin Source File

SOURCE=.\res\Cylinder.ico
# End Source File
# Begin Source File

SOURCE=.\res\descript.ico
# End Source File
# Begin Source File

SOURCE=.\res\dialog_f.ico
# End Source File
# Begin Source File

SOURCE=.\res\dialog_t.ico
# End Source File
# Begin Source File

SOURCE=.\res\Doc.ico
# End Source File
# Begin Source File

SOURCE=.\res\down.ico
# End Source File
# Begin Source File

SOURCE=.\res\Dragcopy.cur
# End Source File
# Begin Source File

SOURCE=.\res\Dragmove.cur
# End Source File
# Begin Source File

SOURCE=.\res\edit.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Entity.ico
# End Source File
# Begin Source File

SOURCE=.\res\fullscre.bmp
# End Source File
# Begin Source File

SOURCE=.\res\global.ico
# End Source File
# Begin Source File

SOURCE=.\res\globe.ico
# End Source File
# Begin Source File

SOURCE=.\res\hand.cur
# End Source File
# Begin Source File

SOURCE=.\res\hand_ope.cur
# End Source File
# Begin Source File

SOURCE=.\res\Handleco.bmp
# End Source File
# Begin Source File

SOURCE=.\res\handleedge.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Imagelis.bmp
# End Source File
# Begin Source File

SOURCE=.\res\info.ico
# End Source File
# Begin Source File

SOURCE=.\res\jedit.bmp
# End Source File
# Begin Source File

SOURCE=.\Res\jet3d_logo_small.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Jwe.ico
# End Source File
# Begin Source File

SOURCE=.\res\Jwe.rc2
# End Source File
# Begin Source File

SOURCE=.\res\left.ico
# End Source File
# Begin Source File

SOURCE=.\res\Light.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Light.ico
# End Source File
# Begin Source File

SOURCE=.\res\lock.ico
# End Source File
# Begin Source File

SOURCE=.\res\Mainfram.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Mode.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Move4way.cur
# End Source File
# Begin Source File

SOURCE=.\res\play.ico
# End Source File
# Begin Source File

SOURCE=.\res\playing.ico
# End Source File
# Begin Source File

SOURCE=.\res\possize.bmp
# End Source File
# Begin Source File

SOURCE=.\res\prefs_ed.ico
# End Source File
# Begin Source File

SOURCE=.\res\prefs_gr.ico
# End Source File
# Begin Source File

SOURCE=.\res\Prefs_ke.bmp
# End Source File
# Begin Source File

SOURCE=.\res\prefs_ke.ico
# End Source File
# Begin Source File

SOURCE=.\res\prefs_mo.ico
# End Source File
# Begin Source File

SOURCE=.\res\prefs_pa.ico
# End Source File
# Begin Source File

SOURCE=.\res\prefs_vi.ico
# End Source File
# Begin Source File

SOURCE=.\res\properti.ico
# End Source File
# Begin Source File

SOURCE=.\res\Removefr.bmp
# End Source File
# Begin Source File

SOURCE=.\res\reset.ico
# End Source File
# Begin Source File

SOURCE=.\res\right.ico
# End Source File
# Begin Source File

SOURCE=.\res\Rotate.cur
# End Source File
# Begin Source File

SOURCE=.\res\Rotatebl.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Rotatebr.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Rotatetl.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Rotatetr.bmp
# End Source File
# Begin Source File

SOURCE=.\res\RotationCenter.cur
# End Source File
# Begin Source File

SOURCE=.\res\Shearlr.bmp
# End Source File
# Begin Source File

SOURCE=.\res\ShearLR.cur
# End Source File
# Begin Source File

SOURCE=.\res\Sheartb.bmp
# End Source File
# Begin Source File

SOURCE=.\res\ShearTB.cur
# End Source File
# Begin Source File

SOURCE=.\res\Showall.ico
# End Source File
# Begin Source File

SOURCE=.\res\Showallb.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Showcurr.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Showvisi.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Speciale.cur
# End Source File
# Begin Source File

SOURCE=.\res\Speciale.ico
# End Source File
# Begin Source File

SOURCE=.\res\Sphere.ico
# End Source File
# Begin Source File

SOURCE=.\res\Stairs.ico
# End Source File
# Begin Source File

SOURCE=.\res\Svertex.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Terrain.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Terrain.ico
# End Source File
# Begin Source File

SOURCE=.\res\Text1.bin
# End Source File
# Begin Source File

SOURCE=.\res\Text2.bin
# End Source File
# Begin Source File

SOURCE=.\res\Tile128.ico
# End Source File
# Begin Source File

SOURCE=.\res\Tile32.ico
# End Source File
# Begin Source File

SOURCE=.\res\Tile64.ico
# End Source File
# Begin Source File

SOURCE=.\res\time.ico
# End Source File
# Begin Source File

SOURCE=.\res\time_sms.ico
# End Source File
# Begin Source File

SOURCE=.\res\timecol.ico
# End Source File
# Begin Source File

SOURCE=.\res\Toolbar.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Toolbar1.bmp
# End Source File
# Begin Source File

SOURCE=.\res\unlock.ico
# End Source File
# Begin Source File

SOURCE=.\res\up.ico
# End Source File
# Begin Source File

SOURCE=.\res\Vertex.bmp
# End Source File
# Begin Source File

SOURCE=.\res\view.ico
# End Source File
# Begin Source File

SOURCE=.\res\zoomin.ico
# End Source File
# Begin Source File

SOURCE=.\res\zoomout.ico
# End Source File
# End Group
# Begin Group "Help Files"

# PROP Default_Filter "cnt;rtf"
# Begin Source File

SOURCE=.\hlp\AfxCore.rtf
# End Source File
# Begin Source File

SOURCE=.\hlp\AfxPrint.rtf
# End Source File
# Begin Source File

SOURCE=.\hlp\AppExit.bmp
# End Source File
# Begin Source File

SOURCE=.\hlp\Bullet.bmp
# End Source File
# Begin Source File

SOURCE=.\hlp\CurArw2.bmp
# End Source File
# Begin Source File

SOURCE=.\hlp\CurArw4.bmp
# End Source File
# Begin Source File

SOURCE=.\hlp\CurHelp.bmp
# End Source File
# Begin Source File

SOURCE=.\hlp\EditCopy.bmp
# End Source File
# Begin Source File

SOURCE=.\hlp\EditCut.bmp
# End Source File
# Begin Source File

SOURCE=.\hlp\EditPast.bmp
# End Source File
# Begin Source File

SOURCE=.\hlp\EditUndo.bmp
# End Source File
# Begin Source File

SOURCE=.\hlp\FileNew.bmp
# End Source File
# Begin Source File

SOURCE=.\hlp\FileOpen.bmp
# End Source File
# Begin Source File

SOURCE=.\hlp\FilePrnt.bmp
# End Source File
# Begin Source File

SOURCE=.\hlp\FileSave.bmp
# End Source File
# Begin Source File

SOURCE=.\hlp\HlpSBar.bmp
# End Source File
# Begin Source File

SOURCE=.\hlp\HlpTBar.bmp
# End Source File
# Begin Source File

SOURCE=.\hlp\jwe.cnt

!IF  "$(CFG)" == "jDesigner3D - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Copying contents file...
InputPath=.\hlp\jwe.cnt
InputName=jwe

"..\..\..\bin\$(InputName).cnt" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	copy "hlp\$(InputName).cnt" ..\..\..\bin

# End Custom Build

!ELSEIF  "$(CFG)" == "jDesigner3D - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Copying contents file...
OutDir=.\Release
InputPath=.\hlp\jwe.cnt
InputName=jwe

"$(OutDir)\$(InputName).cnt" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	copy "hlp\$(InputName).cnt" "$(OutDir)"

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\hlp\RecFirst.bmp
# End Source File
# Begin Source File

SOURCE=.\hlp\RecLast.bmp
# End Source File
# Begin Source File

SOURCE=.\hlp\RecNext.bmp
# End Source File
# Begin Source File

SOURCE=.\hlp\RecPrev.bmp
# End Source File
# Begin Source File

SOURCE=.\hlp\Scmax.bmp
# End Source File
# Begin Source File

SOURCE=.\hlp\ScMenu.bmp
# End Source File
# Begin Source File

SOURCE=.\hlp\Scmin.bmp
# End Source File
# End Group
# Begin Group "J3DMFC Files"

# PROP Default_Filter ".cpp .c .h"
# Begin Source File

SOURCE=.\J3dmfc\J3DApp.cpp
DEP_CPP_J3DAP=\
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
	"..\..\..\include\jePlane.h"\
	"..\..\..\include\jePoly.h"\
	"..\..\..\include\jePortal.h"\
	"..\..\..\include\jeProperty.h"\
	"..\..\..\include\jePtrMgr.h"\
	"..\..\..\include\jeResource.h"\
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
	"..\..\..\include\Sound.h"\
	"..\..\..\include\UVMAP.H"\
	"..\..\..\include\Vec3d.h"\
	"..\..\..\include\VFILE.H"\
	"..\..\..\include\VideoMgr.h"\
	"..\..\..\include\Xform3d.h"\
	".\Dialogs\DialogHelpers\scbarcf.h"\
	".\Dialogs\DialogHelpers\scbarg.h"\
	".\Dialogs\DialogHelpers\sizecbar.h"\
	".\J3dmfc\J3DApp.h"\
	".\J3dmfc\J3DView.h"\
	".\StdAfx.h"\
	
# End Source File
# Begin Source File

SOURCE=.\J3dmfc\J3DApp.h
# End Source File
# Begin Source File

SOURCE=.\J3dmfc\J3DDoc.cpp
DEP_CPP_J3DDO=\
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
	"..\..\..\include\Sound.h"\
	"..\..\..\include\Vec3d.h"\
	"..\..\..\include\VFILE.H"\
	"..\..\..\include\Xform3d.h"\
	".\Dialogs\DialogHelpers\scbarcf.h"\
	".\Dialogs\DialogHelpers\scbarg.h"\
	".\Dialogs\DialogHelpers\sizecbar.h"\
	".\J3dmfc\J3DDoc.h"\
	".\StdAfx.h"\
	
# End Source File
# Begin Source File

SOURCE=.\J3dmfc\J3DDoc.h
# End Source File
# Begin Source File

SOURCE=.\J3dmfc\J3DMainFrm.cpp
DEP_CPP_J3DMA=\
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
	"..\..\..\include\jePlane.h"\
	"..\..\..\include\jePoly.h"\
	"..\..\..\include\jePortal.h"\
	"..\..\..\include\jeProperty.h"\
	"..\..\..\include\jePtrMgr.h"\
	"..\..\..\include\jeResource.h"\
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
	"..\..\..\include\Sound.h"\
	"..\..\..\include\UVMAP.H"\
	"..\..\..\include\Vec3d.h"\
	"..\..\..\include\VFILE.H"\
	"..\..\..\include\VideoMgr.h"\
	"..\..\..\include\Xform3d.h"\
	".\Dialogs\DialogHelpers\scbarcf.h"\
	".\Dialogs\DialogHelpers\scbarg.h"\
	".\Dialogs\DialogHelpers\sizecbar.h"\
	".\J3dmfc\J3DApp.h"\
	".\J3dmfc\J3DDoc.h"\
	".\J3dmfc\J3DMainFrm.h"\
	".\StdAfx.h"\
	
# End Source File
# Begin Source File

SOURCE=.\J3dmfc\J3DMainFrm.h
# End Source File
# Begin Source File

SOURCE=.\J3dmfc\J3DView.cpp
DEP_CPP_J3DVI=\
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
	"..\..\..\include\jePlane.h"\
	"..\..\..\include\jePoly.h"\
	"..\..\..\include\jePortal.h"\
	"..\..\..\include\jeProperty.h"\
	"..\..\..\include\jePtrMgr.h"\
	"..\..\..\include\jeResource.h"\
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
	"..\..\..\include\Sound.h"\
	"..\..\..\include\UVMAP.H"\
	"..\..\..\include\Vec3d.h"\
	"..\..\..\include\VFILE.H"\
	"..\..\..\include\VideoMgr.h"\
	"..\..\..\include\Xform3d.h"\
	".\Dialogs\DialogHelpers\scbarcf.h"\
	".\Dialogs\DialogHelpers\scbarg.h"\
	".\Dialogs\DialogHelpers\sizecbar.h"\
	".\J3dmfc\J3DApp.h"\
	".\J3dmfc\J3DDoc.h"\
	".\J3dmfc\J3DView.h"\
	".\StdAfx.h"\
	
# End Source File
# Begin Source File

SOURCE=.\J3dmfc\J3DView.h
# End Source File
# End Group
# Begin Group "Dialogs"

# PROP Default_Filter ".cpp .c .h"
# Begin Group "DialogHelpers"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Dialogs\DialogHelpers\BitmapResize.cpp
DEP_CPP_BITMA=\
	"..\..\..\include\ARRAY.H"\
	"..\..\..\include\BaseType.h"\
	"..\..\..\include\BITMAP.H"\
	"..\..\..\include\Camera.h"\
	"..\..\..\include\ENGINE.H"\
	"..\..\..\include\jeMaterial.h"\
	"..\..\..\include\jeMemAllocInfo.h"\
	"..\..\..\include\jePtrMgr.h"\
	"..\..\..\include\jeTypes.h"\
	"..\..\..\include\PixelFormat.h"\
	"..\..\..\include\Ram.h"\
	"..\..\..\include\Vec3d.h"\
	"..\..\..\include\VFILE.H"\
	"..\..\..\include\Xform3d.h"\
	".\Dialogs\DialogHelpers\BitmapResize.h"\
	".\Dialogs\DialogHelpers\scbarcf.h"\
	".\Dialogs\DialogHelpers\scbarg.h"\
	".\Dialogs\DialogHelpers\sizecbar.h"\
	".\StdAfx.h"\
	".\Util\Bmp.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Dialogs\DialogHelpers\BitmapResize.h
# End Source File
# Begin Source File

SOURCE=.\Dialogs\DialogHelpers\ColorButton.cpp
DEP_CPP_COLOR=\
	"..\..\..\include\BaseType.h"\
	".\Dialogs\DialogHelpers\ColorButton.h"\
	".\Dialogs\DialogHelpers\ColorPopup.h"\
	".\Dialogs\DialogHelpers\scbarcf.h"\
	".\Dialogs\DialogHelpers\scbarg.h"\
	".\Dialogs\DialogHelpers\sizecbar.h"\
	".\StdAfx.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Dialogs\DialogHelpers\ColorButton.h
# End Source File
# Begin Source File

SOURCE=.\Dialogs\DialogHelpers\ColorPopup.cpp
DEP_CPP_COLORP=\
	"..\..\..\include\BaseType.h"\
	".\Dialogs\DialogHelpers\ColorPopup.h"\
	".\Dialogs\DialogHelpers\scbarcf.h"\
	".\Dialogs\DialogHelpers\scbarg.h"\
	".\Dialogs\DialogHelpers\sizecbar.h"\
	".\StdAfx.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Dialogs\DialogHelpers\ColorPopup.h
# End Source File
# Begin Source File

SOURCE=.\Dialogs\DialogHelpers\DropFileArray.cpp
DEP_CPP_DROPF=\
	"..\..\..\include\BaseType.h"\
	".\Dialogs\DialogHelpers\DropFileArray.h"\
	".\Dialogs\DialogHelpers\scbarcf.h"\
	".\Dialogs\DialogHelpers\scbarg.h"\
	".\Dialogs\DialogHelpers\sizecbar.h"\
	".\StdAfx.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Dialogs\DialogHelpers\DropFileArray.h
# End Source File
# Begin Source File

SOURCE=.\Dialogs\DialogHelpers\MyBar.cpp
DEP_CPP_MYBAR=\
	"..\..\..\include\BaseType.h"\
	".\Dialogs\DialogHelpers\MyBar.h"\
	".\Dialogs\DialogHelpers\scbarcf.h"\
	".\Dialogs\DialogHelpers\scbarg.h"\
	".\Dialogs\DialogHelpers\sizecbar.h"\
	".\StdAfx.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Dialogs\DialogHelpers\MyBar.h
# End Source File
# Begin Source File

SOURCE=.\Dialogs\DialogHelpers\MyDropTarget.cpp
DEP_CPP_MYDRO=\
	"..\..\..\include\BaseType.h"\
	".\Dialogs\DialogHelpers\DropFileArray.h"\
	".\Dialogs\DialogHelpers\MyDropTarget.h"\
	".\Dialogs\DialogHelpers\scbarcf.h"\
	".\Dialogs\DialogHelpers\scbarg.h"\
	".\Dialogs\DialogHelpers\sizecbar.h"\
	".\StdAfx.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Dialogs\DialogHelpers\MyDropTarget.h
# End Source File
# Begin Source File

SOURCE=.\Dialogs\DialogHelpers\scbarcf.cpp
DEP_CPP_SCBAR=\
	"..\..\..\include\BaseType.h"\
	".\Dialogs\DialogHelpers\scbarcf.h"\
	".\Dialogs\DialogHelpers\scbarg.h"\
	".\Dialogs\DialogHelpers\sizecbar.h"\
	".\StdAfx.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Dialogs\DialogHelpers\scbarcf.h
# End Source File
# Begin Source File

SOURCE=.\Dialogs\DialogHelpers\scbarg.cpp
DEP_CPP_SCBARG=\
	"..\..\..\include\BaseType.h"\
	".\Dialogs\DialogHelpers\scbarcf.h"\
	".\Dialogs\DialogHelpers\scbarg.h"\
	".\Dialogs\DialogHelpers\sizecbar.h"\
	".\StdAfx.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Dialogs\DialogHelpers\scbarg.h
# End Source File
# Begin Source File

SOURCE=.\Dialogs\DialogHelpers\sizecbar.cpp
DEP_CPP_SIZEC=\
	"..\..\..\include\BaseType.h"\
	".\Dialogs\DialogHelpers\scbarcf.h"\
	".\Dialogs\DialogHelpers\scbarg.h"\
	".\Dialogs\DialogHelpers\sizecbar.h"\
	".\StdAfx.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Dialogs\DialogHelpers\sizecbar.h
# End Source File
# Begin Source File

SOURCE=.\Dialogs\DialogHelpers\TextureListControl.cpp
DEP_CPP_TEXTU=\
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
	"..\..\..\include\jePlane.h"\
	"..\..\..\include\jePoly.h"\
	"..\..\..\include\jePortal.h"\
	"..\..\..\include\jeProperty.h"\
	"..\..\..\include\jePtrMgr.h"\
	"..\..\..\include\jeResource.h"\
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
	"..\..\..\include\Sound.h"\
	"..\..\..\include\TERRAIN.H"\
	"..\..\..\include\UVMAP.H"\
	"..\..\..\include\Vec3d.h"\
	"..\..\..\include\VFILE.H"\
	"..\..\..\include\VideoMgr.h"\
	"..\..\..\include\Xform3d.h"\
	".\AppData.h"\
	".\BtnST.h"\
	".\CIconTabCtrl.h"\
	".\Core\Brush.h"\
	".\Core\BrushList.h"\
	".\Core\BrushTemplate.h"\
	".\Core\CameraList.h"\
	".\Core\CamObj.h"\
	".\Core\Class.h"\
	".\Core\Descriptor.h"\
	".\Core\Entity.h"\
	".\Core\EntityList.h"\
	".\Core\Group.h"\
	".\Core\GroupList.h"\
	".\Core\jwObject.h"\
	".\Core\Level.h"\
	".\Core\Light.h"\
	".\Core\LightList.h"\
	".\Core\MaterialIdentList.h"\
	".\Core\MaterialList.h"\
	".\Core\MaterialList2.h"\
	".\Core\Materials.h"\
	".\Core\Model.h"\
	".\Core\modellist.h"\
	".\Core\ObjectList.h"\
	".\Core\Ortho.h"\
	".\Core\Select.h"\
	".\Core\TernList.h"\
	".\Core\TerrnObj.h"\
	".\Core\Undo.h"\
	".\Core\VertList.h"\
	".\CTextToolBar.h"\
	".\Defs.h"\
	".\Dialogs\buildspin.h"\
	".\Dialogs\DialogBuilderDlg.h"\
	".\Dialogs\DialogHelpers\ColorButton.h"\
	".\Dialogs\DialogHelpers\ColorPopup.h"\
	".\Dialogs\DialogHelpers\DropFileArray.h"\
	".\Dialogs\DialogHelpers\MyBar.h"\
	".\Dialogs\DialogHelpers\MyDropTarget.h"\
	".\Dialogs\DialogHelpers\scbarcf.h"\
	".\Dialogs\DialogHelpers\scbarg.h"\
	".\Dialogs\DialogHelpers\sizecbar.h"\
	".\Dialogs\DialogHelpers\TextureListControl.h"\
	".\Dialogs\DialogHelpers\TextureTreeControl.h"\
	".\Dialogs\DlgAutorecover.h"\
	".\Dialogs\Empty.h"\
	".\Dialogs\globalmaterials.h"\
	".\Dialogs\Groups.h"\
	".\Dialogs\Lists.h"\
	".\Dialogs\Models.h"\
	".\Dialogs\Properties.h"\
	".\Dialogs\TexturesDlg.h"\
	".\Dialogs\Timeline.h"\
	".\Doc.h"\
	".\DragTree.h"\
	".\grouptreectrl.h"\
	".\J3dmfc\J3DApp.h"\
	".\J3dmfc\J3DDoc.h"\
	".\J3dmfc\J3DMainFrm.h"\
	".\jet\Symbol.h"\
	".\jetdialog.h"\
	".\Jwe.h"\
	".\label.h"\
	".\MainFrm.h"\
	".\MsgLog.h"\
	".\Settings.h"\
	".\StdAfx.h"\
	".\TreeCtrlEx.h"\
	".\Util\jeList.h"\
	".\Util\Point.h"\
	".\Util\Rect.h"\
	".\WndReg.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Dialogs\DialogHelpers\TextureListControl.h
# End Source File
# Begin Source File

SOURCE=.\Dialogs\DialogHelpers\TextureTreeControl.cpp
DEP_CPP_TEXTUR=\
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
	"..\..\..\include\jePlane.h"\
	"..\..\..\include\jePoly.h"\
	"..\..\..\include\jePortal.h"\
	"..\..\..\include\jeProperty.h"\
	"..\..\..\include\jePtrMgr.h"\
	"..\..\..\include\jeResource.h"\
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
	"..\..\..\include\Sound.h"\
	"..\..\..\include\TERRAIN.H"\
	"..\..\..\include\UVMAP.H"\
	"..\..\..\include\Vec3d.h"\
	"..\..\..\include\VFILE.H"\
	"..\..\..\include\VideoMgr.h"\
	"..\..\..\include\Xform3d.h"\
	".\AppData.h"\
	".\BtnST.h"\
	".\CIconTabCtrl.h"\
	".\Core\Brush.h"\
	".\Core\BrushList.h"\
	".\Core\BrushTemplate.h"\
	".\Core\CameraList.h"\
	".\Core\CamObj.h"\
	".\Core\Class.h"\
	".\Core\Descriptor.h"\
	".\Core\Entity.h"\
	".\Core\EntityList.h"\
	".\Core\Group.h"\
	".\Core\GroupList.h"\
	".\Core\jwObject.h"\
	".\Core\Level.h"\
	".\Core\Light.h"\
	".\Core\LightList.h"\
	".\Core\MaterialIdentList.h"\
	".\Core\MaterialList.h"\
	".\Core\MaterialList2.h"\
	".\Core\Materials.h"\
	".\Core\Model.h"\
	".\Core\modellist.h"\
	".\Core\ObjectList.h"\
	".\Core\Ortho.h"\
	".\Core\Select.h"\
	".\Core\TernList.h"\
	".\Core\TerrnObj.h"\
	".\Core\Undo.h"\
	".\Core\VertList.h"\
	".\CTextToolBar.h"\
	".\Defs.h"\
	".\Dialogs\buildspin.h"\
	".\Dialogs\DialogBuilderDlg.h"\
	".\Dialogs\DialogHelpers\ColorButton.h"\
	".\Dialogs\DialogHelpers\ColorPopup.h"\
	".\Dialogs\DialogHelpers\DropFileArray.h"\
	".\Dialogs\DialogHelpers\MyBar.h"\
	".\Dialogs\DialogHelpers\MyDropTarget.h"\
	".\Dialogs\DialogHelpers\scbarcf.h"\
	".\Dialogs\DialogHelpers\scbarg.h"\
	".\Dialogs\DialogHelpers\sizecbar.h"\
	".\Dialogs\DialogHelpers\TextureListControl.h"\
	".\Dialogs\DialogHelpers\TextureTreeControl.h"\
	".\Dialogs\DlgAutorecover.h"\
	".\Dialogs\Empty.h"\
	".\Dialogs\globalmaterials.h"\
	".\Dialogs\Groups.h"\
	".\Dialogs\Lists.h"\
	".\Dialogs\Models.h"\
	".\Dialogs\Properties.h"\
	".\Dialogs\TexturesDlg.h"\
	".\Dialogs\Timeline.h"\
	".\Doc.h"\
	".\DragTree.h"\
	".\grouptreectrl.h"\
	".\J3dmfc\J3DApp.h"\
	".\J3dmfc\J3DDoc.h"\
	".\J3dmfc\J3DMainFrm.h"\
	".\jet\Symbol.h"\
	".\jetdialog.h"\
	".\Jwe.h"\
	".\label.h"\
	".\MainFrm.h"\
	".\MsgLog.h"\
	".\Settings.h"\
	".\StdAfx.h"\
	".\TreeCtrlEx.h"\
	".\Util\jeList.h"\
	".\Util\Point.h"\
	".\Util\Rect.h"\
	".\WndReg.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Dialogs\DialogHelpers\TextureTreeControl.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\Dialogs\AddModel.cpp
DEP_CPP_ADDMO=\
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
	"..\..\..\include\jePlane.h"\
	"..\..\..\include\jePoly.h"\
	"..\..\..\include\jePortal.h"\
	"..\..\..\include\jeProperty.h"\
	"..\..\..\include\jePtrMgr.h"\
	"..\..\..\include\jeResource.h"\
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
	"..\..\..\include\Sound.h"\
	"..\..\..\include\UVMAP.H"\
	"..\..\..\include\Vec3d.h"\
	"..\..\..\include\VFILE.H"\
	"..\..\..\include\VideoMgr.h"\
	"..\..\..\include\Xform3d.h"\
	".\AppData.h"\
	".\Core\MaterialList.h"\
	".\Core\Materials.h"\
	".\Dialogs\AddModel.h"\
	".\Dialogs\DialogHelpers\scbarcf.h"\
	".\Dialogs\DialogHelpers\scbarg.h"\
	".\Dialogs\DialogHelpers\sizecbar.h"\
	".\Dialogs\DlgAutorecover.h"\
	".\J3dmfc\J3DApp.h"\
	".\Jwe.h"\
	".\MsgLog.h"\
	".\Settings.h"\
	".\StdAfx.h"\
	".\Util\jeList.h"\
	".\WndReg.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Dialogs\AddModel.h
# End Source File
# Begin Source File

SOURCE=.\Dialogs\builderbutton.cpp
DEP_CPP_BUILD=\
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
	"..\..\..\include\jePlane.h"\
	"..\..\..\include\jePoly.h"\
	"..\..\..\include\jePortal.h"\
	"..\..\..\include\jeProperty.h"\
	"..\..\..\include\jePtrMgr.h"\
	"..\..\..\include\jeResource.h"\
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
	"..\..\..\include\Sound.h"\
	"..\..\..\include\TERRAIN.H"\
	"..\..\..\include\UVMAP.H"\
	"..\..\..\include\Vec3d.h"\
	"..\..\..\include\VFILE.H"\
	"..\..\..\include\VideoMgr.h"\
	"..\..\..\include\Xform3d.h"\
	".\AppData.h"\
	".\BtnST.h"\
	".\CIconTabCtrl.h"\
	".\Core\Brush.h"\
	".\Core\BrushList.h"\
	".\Core\BrushTemplate.h"\
	".\Core\CameraList.h"\
	".\Core\CamObj.h"\
	".\Core\Class.h"\
	".\Core\Descriptor.h"\
	".\Core\Entity.h"\
	".\Core\EntityList.h"\
	".\Core\Group.h"\
	".\Core\GroupList.h"\
	".\Core\jwObject.h"\
	".\Core\Level.h"\
	".\Core\Light.h"\
	".\Core\LightList.h"\
	".\Core\MaterialIdentList.h"\
	".\Core\MaterialList.h"\
	".\Core\MaterialList2.h"\
	".\Core\Materials.h"\
	".\Core\Model.h"\
	".\Core\modellist.h"\
	".\Core\ObjectList.h"\
	".\Core\Ortho.h"\
	".\Core\Select.h"\
	".\Core\TernList.h"\
	".\Core\TerrnObj.h"\
	".\Core\Undo.h"\
	".\Core\VertList.h"\
	".\CTextToolBar.h"\
	".\Defs.h"\
	".\Dialogs\builderbutton.h"\
	".\Dialogs\buildspin.h"\
	".\Dialogs\DialogBuilderDlg.h"\
	".\Dialogs\DialogHelpers\ColorButton.h"\
	".\Dialogs\DialogHelpers\ColorPopup.h"\
	".\Dialogs\DialogHelpers\DropFileArray.h"\
	".\Dialogs\DialogHelpers\MyBar.h"\
	".\Dialogs\DialogHelpers\MyDropTarget.h"\
	".\Dialogs\DialogHelpers\scbarcf.h"\
	".\Dialogs\DialogHelpers\scbarg.h"\
	".\Dialogs\DialogHelpers\sizecbar.h"\
	".\Dialogs\DialogHelpers\TextureListControl.h"\
	".\Dialogs\DialogHelpers\TextureTreeControl.h"\
	".\Dialogs\DlgAutorecover.h"\
	".\Dialogs\Empty.h"\
	".\Dialogs\globalmaterials.h"\
	".\Dialogs\Groups.h"\
	".\Dialogs\Lists.h"\
	".\Dialogs\Models.h"\
	".\Dialogs\Properties.h"\
	".\Dialogs\TexturesDlg.h"\
	".\Dialogs\Timeline.h"\
	".\Doc.h"\
	".\DragTree.h"\
	".\grouptreectrl.h"\
	".\J3dmfc\J3DApp.h"\
	".\J3dmfc\J3DDoc.h"\
	".\J3dmfc\J3DMainFrm.h"\
	".\jet\Symbol.h"\
	".\jetdialog.h"\
	".\Jwe.h"\
	".\label.h"\
	".\MainFrm.h"\
	".\MsgLog.h"\
	".\Settings.h"\
	".\StdAfx.h"\
	".\TreeCtrlEx.h"\
	".\Util\DrawTool.h"\
	".\Util\jeList.h"\
	".\Util\Point.h"\
	".\Util\Rect.h"\
	".\WndReg.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Dialogs\builderbutton.h
# End Source File
# Begin Source File

SOURCE=.\Dialogs\buildercombo.cpp
DEP_CPP_BUILDE=\
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
	"..\..\..\include\jePlane.h"\
	"..\..\..\include\jePoly.h"\
	"..\..\..\include\jePortal.h"\
	"..\..\..\include\jeProperty.h"\
	"..\..\..\include\jePtrMgr.h"\
	"..\..\..\include\jeResource.h"\
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
	"..\..\..\include\Sound.h"\
	"..\..\..\include\TERRAIN.H"\
	"..\..\..\include\UVMAP.H"\
	"..\..\..\include\Vec3d.h"\
	"..\..\..\include\VFILE.H"\
	"..\..\..\include\VideoMgr.h"\
	"..\..\..\include\Xform3d.h"\
	".\AppData.h"\
	".\BtnST.h"\
	".\CIconTabCtrl.h"\
	".\Core\Brush.h"\
	".\Core\BrushList.h"\
	".\Core\BrushTemplate.h"\
	".\Core\CameraList.h"\
	".\Core\CamObj.h"\
	".\Core\Class.h"\
	".\Core\Descriptor.h"\
	".\Core\Entity.h"\
	".\Core\EntityList.h"\
	".\Core\Group.h"\
	".\Core\GroupList.h"\
	".\Core\jwObject.h"\
	".\Core\Level.h"\
	".\Core\Light.h"\
	".\Core\LightList.h"\
	".\Core\MaterialIdentList.h"\
	".\Core\MaterialList.h"\
	".\Core\MaterialList2.h"\
	".\Core\Materials.h"\
	".\Core\Model.h"\
	".\Core\modellist.h"\
	".\Core\ObjectList.h"\
	".\Core\Ortho.h"\
	".\Core\Select.h"\
	".\Core\TernList.h"\
	".\Core\TerrnObj.h"\
	".\Core\Undo.h"\
	".\Core\VertList.h"\
	".\CTextToolBar.h"\
	".\Defs.h"\
	".\Dialogs\buildercombo.h"\
	".\Dialogs\buildspin.h"\
	".\Dialogs\DialogBuilderDlg.h"\
	".\Dialogs\DialogHelpers\ColorButton.h"\
	".\Dialogs\DialogHelpers\ColorPopup.h"\
	".\Dialogs\DialogHelpers\DropFileArray.h"\
	".\Dialogs\DialogHelpers\MyBar.h"\
	".\Dialogs\DialogHelpers\MyDropTarget.h"\
	".\Dialogs\DialogHelpers\scbarcf.h"\
	".\Dialogs\DialogHelpers\scbarg.h"\
	".\Dialogs\DialogHelpers\sizecbar.h"\
	".\Dialogs\DialogHelpers\TextureListControl.h"\
	".\Dialogs\DialogHelpers\TextureTreeControl.h"\
	".\Dialogs\DlgAutorecover.h"\
	".\Dialogs\Empty.h"\
	".\Dialogs\globalmaterials.h"\
	".\Dialogs\Groups.h"\
	".\Dialogs\Lists.h"\
	".\Dialogs\Models.h"\
	".\Dialogs\Properties.h"\
	".\Dialogs\TexturesDlg.h"\
	".\Dialogs\Timeline.h"\
	".\Doc.h"\
	".\DragTree.h"\
	".\grouptreectrl.h"\
	".\J3dmfc\J3DApp.h"\
	".\J3dmfc\J3DDoc.h"\
	".\J3dmfc\J3DMainFrm.h"\
	".\jet\Symbol.h"\
	".\jetdialog.h"\
	".\Jwe.h"\
	".\label.h"\
	".\MainFrm.h"\
	".\MsgLog.h"\
	".\Settings.h"\
	".\StdAfx.h"\
	".\TreeCtrlEx.h"\
	".\Util\jeList.h"\
	".\Util\Point.h"\
	".\Util\Rect.h"\
	".\WndReg.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Dialogs\buildercombo.h
# End Source File
# Begin Source File

SOURCE=.\Dialogs\builderedit.cpp
DEP_CPP_BUILDER=\
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
	"..\..\..\include\jePlane.h"\
	"..\..\..\include\jePoly.h"\
	"..\..\..\include\jePortal.h"\
	"..\..\..\include\jeProperty.h"\
	"..\..\..\include\jePtrMgr.h"\
	"..\..\..\include\jeResource.h"\
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
	"..\..\..\include\Sound.h"\
	"..\..\..\include\TERRAIN.H"\
	"..\..\..\include\UVMAP.H"\
	"..\..\..\include\Vec3d.h"\
	"..\..\..\include\VFILE.H"\
	"..\..\..\include\VideoMgr.h"\
	"..\..\..\include\Xform3d.h"\
	".\AppData.h"\
	".\BtnST.h"\
	".\CIconTabCtrl.h"\
	".\Core\Brush.h"\
	".\Core\BrushList.h"\
	".\Core\BrushTemplate.h"\
	".\Core\CameraList.h"\
	".\Core\CamObj.h"\
	".\Core\Class.h"\
	".\Core\Descriptor.h"\
	".\Core\Entity.h"\
	".\Core\EntityList.h"\
	".\Core\Group.h"\
	".\Core\GroupList.h"\
	".\Core\jwObject.h"\
	".\Core\Level.h"\
	".\Core\Light.h"\
	".\Core\LightList.h"\
	".\Core\MaterialIdentList.h"\
	".\Core\MaterialList.h"\
	".\Core\MaterialList2.h"\
	".\Core\Materials.h"\
	".\Core\Model.h"\
	".\Core\modellist.h"\
	".\Core\ObjectList.h"\
	".\Core\Ortho.h"\
	".\Core\Select.h"\
	".\Core\TernList.h"\
	".\Core\TerrnObj.h"\
	".\Core\Undo.h"\
	".\Core\VertList.h"\
	".\CTextToolBar.h"\
	".\Defs.h"\
	".\Dialogs\builderedit.h"\
	".\Dialogs\buildspin.h"\
	".\Dialogs\DialogBuilderDlg.h"\
	".\Dialogs\DialogHelpers\ColorButton.h"\
	".\Dialogs\DialogHelpers\ColorPopup.h"\
	".\Dialogs\DialogHelpers\DropFileArray.h"\
	".\Dialogs\DialogHelpers\MyBar.h"\
	".\Dialogs\DialogHelpers\MyDropTarget.h"\
	".\Dialogs\DialogHelpers\scbarcf.h"\
	".\Dialogs\DialogHelpers\scbarg.h"\
	".\Dialogs\DialogHelpers\sizecbar.h"\
	".\Dialogs\DialogHelpers\TextureListControl.h"\
	".\Dialogs\DialogHelpers\TextureTreeControl.h"\
	".\Dialogs\DlgAutorecover.h"\
	".\Dialogs\Empty.h"\
	".\Dialogs\globalmaterials.h"\
	".\Dialogs\Groups.h"\
	".\Dialogs\Lists.h"\
	".\Dialogs\Models.h"\
	".\Dialogs\Properties.h"\
	".\Dialogs\TexturesDlg.h"\
	".\Dialogs\Timeline.h"\
	".\Doc.h"\
	".\DragTree.h"\
	".\grouptreectrl.h"\
	".\J3dmfc\J3DApp.h"\
	".\J3dmfc\J3DDoc.h"\
	".\J3dmfc\J3DMainFrm.h"\
	".\jet\Symbol.h"\
	".\jetdialog.h"\
	".\Jwe.h"\
	".\label.h"\
	".\MainFrm.h"\
	".\MsgLog.h"\
	".\Settings.h"\
	".\StdAfx.h"\
	".\TreeCtrlEx.h"\
	".\Util\jeList.h"\
	".\Util\Point.h"\
	".\Util\Rect.h"\
	".\WndReg.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Dialogs\builderedit.h
# End Source File
# Begin Source File

SOURCE=.\Dialogs\buildspin.cpp
DEP_CPP_BUILDS=\
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
	"..\..\..\include\jePlane.h"\
	"..\..\..\include\jePoly.h"\
	"..\..\..\include\jePortal.h"\
	"..\..\..\include\jeProperty.h"\
	"..\..\..\include\jePtrMgr.h"\
	"..\..\..\include\jeResource.h"\
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
	"..\..\..\include\Sound.h"\
	"..\..\..\include\UVMAP.H"\
	"..\..\..\include\Vec3d.h"\
	"..\..\..\include\VFILE.H"\
	"..\..\..\include\VideoMgr.h"\
	"..\..\..\include\Xform3d.h"\
	".\AppData.h"\
	".\Core\MaterialList.h"\
	".\Core\Materials.h"\
	".\Dialogs\buildspin.h"\
	".\Dialogs\DialogHelpers\scbarcf.h"\
	".\Dialogs\DialogHelpers\scbarg.h"\
	".\Dialogs\DialogHelpers\sizecbar.h"\
	".\Dialogs\DlgAutorecover.h"\
	".\J3dmfc\J3DApp.h"\
	".\Jwe.h"\
	".\MsgLog.h"\
	".\Settings.h"\
	".\StdAfx.h"\
	".\Util\jeList.h"\
	".\WndReg.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Dialogs\buildspin.h
# End Source File
# Begin Source File

SOURCE=.\Dialogs\DialogBuilderDlg.cpp
DEP_CPP_DIALO=\
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
	"..\..\..\include\jePlane.h"\
	"..\..\..\include\jePoly.h"\
	"..\..\..\include\jePortal.h"\
	"..\..\..\include\jeProperty.h"\
	"..\..\..\include\jePtrMgr.h"\
	"..\..\..\include\jeResource.h"\
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
	"..\..\..\include\Sound.h"\
	"..\..\..\include\TERRAIN.H"\
	"..\..\..\include\UVMAP.H"\
	"..\..\..\include\Vec3d.h"\
	"..\..\..\include\VFILE.H"\
	"..\..\..\include\VideoMgr.h"\
	"..\..\..\include\Xform3d.h"\
	".\BtnST.h"\
	".\CIconTabCtrl.h"\
	".\Core\Brush.h"\
	".\Core\BrushList.h"\
	".\Core\BrushTemplate.h"\
	".\Core\CameraList.h"\
	".\Core\CamObj.h"\
	".\Core\Class.h"\
	".\Core\Descriptor.h"\
	".\Core\Entity.h"\
	".\Core\EntityList.h"\
	".\Core\Group.h"\
	".\Core\GroupList.h"\
	".\Core\jwObject.h"\
	".\Core\Level.h"\
	".\Core\Light.h"\
	".\Core\LightList.h"\
	".\Core\MaterialIdentList.h"\
	".\Core\MaterialList.h"\
	".\Core\MaterialList2.h"\
	".\Core\Materials.h"\
	".\Core\Model.h"\
	".\Core\modellist.h"\
	".\Core\ObjectList.h"\
	".\Core\Ortho.h"\
	".\Core\Select.h"\
	".\Core\TernList.h"\
	".\Core\TerrnObj.h"\
	".\Core\Undo.h"\
	".\Core\VertList.h"\
	".\CTextToolBar.h"\
	".\Defs.h"\
	".\Dialogs\builderbutton.h"\
	".\Dialogs\buildercombo.h"\
	".\Dialogs\builderedit.h"\
	".\Dialogs\buildspin.h"\
	".\Dialogs\DialogBuilderDlg.h"\
	".\Dialogs\DialogHelpers\ColorButton.h"\
	".\Dialogs\DialogHelpers\ColorPopup.h"\
	".\Dialogs\DialogHelpers\DropFileArray.h"\
	".\Dialogs\DialogHelpers\MyBar.h"\
	".\Dialogs\DialogHelpers\MyDropTarget.h"\
	".\Dialogs\DialogHelpers\scbarcf.h"\
	".\Dialogs\DialogHelpers\scbarg.h"\
	".\Dialogs\DialogHelpers\sizecbar.h"\
	".\Dialogs\DialogHelpers\TextureListControl.h"\
	".\Dialogs\DialogHelpers\TextureTreeControl.h"\
	".\Dialogs\Empty.h"\
	".\Dialogs\globalmaterials.h"\
	".\Dialogs\Groups.h"\
	".\Dialogs\Lists.h"\
	".\Dialogs\Models.h"\
	".\Dialogs\Properties.h"\
	".\Dialogs\TexturesDlg.h"\
	".\Dialogs\Timeline.h"\
	".\Doc.h"\
	".\DragTree.h"\
	".\grouptreectrl.h"\
	".\J3dmfc\J3DDoc.h"\
	".\J3dmfc\J3DMainFrm.h"\
	".\jet\Symbol.h"\
	".\jetdialog.h"\
	".\label.h"\
	".\MainFrm.h"\
	".\StdAfx.h"\
	".\TreeCtrlEx.h"\
	".\Util\jeList.h"\
	".\Util\Point.h"\
	".\Util\Rect.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Dialogs\DialogBuilderDlg.h
# End Source File
# Begin Source File

SOURCE=.\Dialogs\disclaimer.cpp
DEP_CPP_DISCL=\
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
	"..\..\..\include\jePlane.h"\
	"..\..\..\include\jePoly.h"\
	"..\..\..\include\jePortal.h"\
	"..\..\..\include\jeProperty.h"\
	"..\..\..\include\jePtrMgr.h"\
	"..\..\..\include\jeResource.h"\
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
	"..\..\..\include\Sound.h"\
	"..\..\..\include\TERRAIN.H"\
	"..\..\..\include\UVMAP.H"\
	"..\..\..\include\Vec3d.h"\
	"..\..\..\include\VFILE.H"\
	"..\..\..\include\VideoMgr.h"\
	"..\..\..\include\Xform3d.h"\
	".\AppData.h"\
	".\BtnST.h"\
	".\BuildNumber.h"\
	".\BuildType.h"\
	".\CIconTabCtrl.h"\
	".\Core\Brush.h"\
	".\Core\BrushList.h"\
	".\Core\BrushTemplate.h"\
	".\Core\CameraList.h"\
	".\Core\CamObj.h"\
	".\Core\Class.h"\
	".\Core\Descriptor.h"\
	".\Core\Entity.h"\
	".\Core\EntityList.h"\
	".\Core\Group.h"\
	".\Core\GroupList.h"\
	".\Core\jwObject.h"\
	".\Core\Level.h"\
	".\Core\Light.h"\
	".\Core\LightList.h"\
	".\Core\MaterialIdentList.h"\
	".\Core\MaterialList.h"\
	".\Core\MaterialList2.h"\
	".\Core\Materials.h"\
	".\Core\Model.h"\
	".\Core\modellist.h"\
	".\Core\ObjectList.h"\
	".\Core\Ortho.h"\
	".\Core\Select.h"\
	".\Core\TernList.h"\
	".\Core\TerrnObj.h"\
	".\Core\Undo.h"\
	".\Core\VertList.h"\
	".\CTextToolBar.h"\
	".\Defs.h"\
	".\Dialogs\buildspin.h"\
	".\Dialogs\DialogBuilderDlg.h"\
	".\Dialogs\DialogHelpers\ColorButton.h"\
	".\Dialogs\DialogHelpers\ColorPopup.h"\
	".\Dialogs\DialogHelpers\DropFileArray.h"\
	".\Dialogs\DialogHelpers\MyBar.h"\
	".\Dialogs\DialogHelpers\MyDropTarget.h"\
	".\Dialogs\DialogHelpers\scbarcf.h"\
	".\Dialogs\DialogHelpers\scbarg.h"\
	".\Dialogs\DialogHelpers\sizecbar.h"\
	".\Dialogs\DialogHelpers\TextureListControl.h"\
	".\Dialogs\DialogHelpers\TextureTreeControl.h"\
	".\Dialogs\disclaimer.h"\
	".\Dialogs\DlgAutorecover.h"\
	".\Dialogs\Empty.h"\
	".\Dialogs\globalmaterials.h"\
	".\Dialogs\Groups.h"\
	".\Dialogs\Lists.h"\
	".\Dialogs\Models.h"\
	".\Dialogs\Properties.h"\
	".\Dialogs\TexturesDlg.h"\
	".\Dialogs\Timeline.h"\
	".\Doc.h"\
	".\DragTree.h"\
	".\grouptreectrl.h"\
	".\J3dmfc\J3DApp.h"\
	".\J3dmfc\J3DDoc.h"\
	".\J3dmfc\J3DMainFrm.h"\
	".\jet\Symbol.h"\
	".\jetdialog.h"\
	".\Jwe.h"\
	".\label.h"\
	".\MainFrm.h"\
	".\MsgLog.h"\
	".\Settings.h"\
	".\StdAfx.h"\
	".\TreeCtrlEx.h"\
	".\Util\jeList.h"\
	".\Util\Point.h"\
	".\Util\Rect.h"\
	".\Util\Spawn.h"\
	".\WndReg.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Dialogs\disclaimer.h
# End Source File
# Begin Source File

SOURCE=.\Dialogs\DlgAutorecover.cpp
DEP_CPP_DLGAU=\
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
	"..\..\..\include\jePlane.h"\
	"..\..\..\include\jePoly.h"\
	"..\..\..\include\jePortal.h"\
	"..\..\..\include\jeProperty.h"\
	"..\..\..\include\jePtrMgr.h"\
	"..\..\..\include\jeResource.h"\
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
	"..\..\..\include\Sound.h"\
	"..\..\..\include\TERRAIN.H"\
	"..\..\..\include\UVMAP.H"\
	"..\..\..\include\Vec3d.h"\
	"..\..\..\include\VFILE.H"\
	"..\..\..\include\VideoMgr.h"\
	"..\..\..\include\Xform3d.h"\
	".\AppData.h"\
	".\BtnST.h"\
	".\CIconTabCtrl.h"\
	".\Core\Brush.h"\
	".\Core\BrushList.h"\
	".\Core\BrushTemplate.h"\
	".\Core\CameraList.h"\
	".\Core\CamObj.h"\
	".\Core\Class.h"\
	".\Core\Descriptor.h"\
	".\Core\Entity.h"\
	".\Core\EntityList.h"\
	".\Core\Group.h"\
	".\Core\GroupList.h"\
	".\Core\jwObject.h"\
	".\Core\Level.h"\
	".\Core\Light.h"\
	".\Core\LightList.h"\
	".\Core\MaterialIdentList.h"\
	".\Core\MaterialList.h"\
	".\Core\MaterialList2.h"\
	".\Core\Materials.h"\
	".\Core\Model.h"\
	".\Core\modellist.h"\
	".\Core\ObjectList.h"\
	".\Core\Ortho.h"\
	".\Core\Select.h"\
	".\Core\TernList.h"\
	".\Core\TerrnObj.h"\
	".\Core\Undo.h"\
	".\Core\VertList.h"\
	".\CTextToolBar.h"\
	".\Defs.h"\
	".\Dialogs\buildspin.h"\
	".\Dialogs\DialogBuilderDlg.h"\
	".\Dialogs\DialogHelpers\ColorButton.h"\
	".\Dialogs\DialogHelpers\ColorPopup.h"\
	".\Dialogs\DialogHelpers\DropFileArray.h"\
	".\Dialogs\DialogHelpers\MyBar.h"\
	".\Dialogs\DialogHelpers\MyDropTarget.h"\
	".\Dialogs\DialogHelpers\scbarcf.h"\
	".\Dialogs\DialogHelpers\scbarg.h"\
	".\Dialogs\DialogHelpers\sizecbar.h"\
	".\Dialogs\DialogHelpers\TextureListControl.h"\
	".\Dialogs\DialogHelpers\TextureTreeControl.h"\
	".\Dialogs\DlgAutorecover.h"\
	".\Dialogs\Empty.h"\
	".\Dialogs\globalmaterials.h"\
	".\Dialogs\Groups.h"\
	".\Dialogs\Lists.h"\
	".\Dialogs\Models.h"\
	".\Dialogs\Properties.h"\
	".\Dialogs\TexturesDlg.h"\
	".\Dialogs\Timeline.h"\
	".\Doc.h"\
	".\DragTree.h"\
	".\grouptreectrl.h"\
	".\J3dmfc\J3DApp.h"\
	".\J3dmfc\J3DDoc.h"\
	".\J3dmfc\J3DMainFrm.h"\
	".\jet\Symbol.h"\
	".\jetdialog.h"\
	".\Jwe.h"\
	".\label.h"\
	".\MainFrm.h"\
	".\MsgLog.h"\
	".\Settings.h"\
	".\StdAfx.h"\
	".\TreeCtrlEx.h"\
	".\Util\jeList.h"\
	".\Util\Point.h"\
	".\Util\Rect.h"\
	".\WndReg.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Dialogs\DlgAutorecover.h
# End Source File
# Begin Source File

SOURCE=.\Dialogs\Empty.cpp
DEP_CPP_EMPTY=\
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
	"..\..\..\include\jePlane.h"\
	"..\..\..\include\jePoly.h"\
	"..\..\..\include\jePortal.h"\
	"..\..\..\include\jeProperty.h"\
	"..\..\..\include\jePtrMgr.h"\
	"..\..\..\include\jeResource.h"\
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
	"..\..\..\include\Sound.h"\
	"..\..\..\include\UVMAP.H"\
	"..\..\..\include\Vec3d.h"\
	"..\..\..\include\VFILE.H"\
	"..\..\..\include\VideoMgr.h"\
	"..\..\..\include\Xform3d.h"\
	".\AppData.h"\
	".\Core\MaterialList.h"\
	".\Core\Materials.h"\
	".\Dialogs\DialogHelpers\scbarcf.h"\
	".\Dialogs\DialogHelpers\scbarg.h"\
	".\Dialogs\DialogHelpers\sizecbar.h"\
	".\Dialogs\DlgAutorecover.h"\
	".\Dialogs\Empty.h"\
	".\J3dmfc\J3DApp.h"\
	".\Jwe.h"\
	".\MfcUtil.h"\
	".\MsgLog.h"\
	".\Settings.h"\
	".\StdAfx.h"\
	".\Util\jeList.h"\
	".\WndReg.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Dialogs\Empty.h
# End Source File
# Begin Source File

SOURCE=.\Dialogs\globalmaterials.cpp
DEP_CPP_GLOBA=\
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
	"..\..\..\include\jePlane.h"\
	"..\..\..\include\jePoly.h"\
	"..\..\..\include\jePortal.h"\
	"..\..\..\include\jeProperty.h"\
	"..\..\..\include\jePtrMgr.h"\
	"..\..\..\include\jeResource.h"\
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
	"..\..\..\include\Sound.h"\
	"..\..\..\include\UVMAP.H"\
	"..\..\..\include\Vec3d.h"\
	"..\..\..\include\VFILE.H"\
	"..\..\..\include\VideoMgr.h"\
	"..\..\..\include\Xform3d.h"\
	".\AppData.h"\
	".\Core\MaterialList.h"\
	".\Core\MaterialList2.h"\
	".\Core\Materials.h"\
	".\Dialogs\DialogHelpers\scbarcf.h"\
	".\Dialogs\DialogHelpers\scbarg.h"\
	".\Dialogs\DialogHelpers\sizecbar.h"\
	".\Dialogs\DlgAutorecover.h"\
	".\Dialogs\globalmaterials.h"\
	".\J3dmfc\J3DApp.h"\
	".\Jwe.h"\
	".\MsgLog.h"\
	".\Settings.h"\
	".\StdAfx.h"\
	".\Util\jeList.h"\
	".\WndReg.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Dialogs\globalmaterials.h
# End Source File
# Begin Source File

SOURCE=.\Dialogs\Groups.cpp
DEP_CPP_GROUPS=\
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
	"..\..\..\include\jePlane.h"\
	"..\..\..\include\jePoly.h"\
	"..\..\..\include\jePortal.h"\
	"..\..\..\include\jeProperty.h"\
	"..\..\..\include\jePtrMgr.h"\
	"..\..\..\include\jeResource.h"\
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
	"..\..\..\include\Sound.h"\
	"..\..\..\include\TERRAIN.H"\
	"..\..\..\include\UVMAP.H"\
	"..\..\..\include\Vec3d.h"\
	"..\..\..\include\VFILE.H"\
	"..\..\..\include\VideoMgr.h"\
	"..\..\..\include\Xform3d.h"\
	".\AppData.h"\
	".\BtnST.h"\
	".\CIconTabCtrl.h"\
	".\Core\Brush.h"\
	".\Core\BrushList.h"\
	".\Core\BrushTemplate.h"\
	".\Core\CameraList.h"\
	".\Core\CamObj.h"\
	".\Core\Class.h"\
	".\Core\Descriptor.h"\
	".\Core\Entity.h"\
	".\Core\EntityList.h"\
	".\Core\Group.h"\
	".\Core\GroupList.h"\
	".\Core\jwObject.h"\
	".\Core\Level.h"\
	".\Core\Light.h"\
	".\Core\LightList.h"\
	".\Core\MaterialIdentList.h"\
	".\Core\MaterialList.h"\
	".\Core\MaterialList2.h"\
	".\Core\Materials.h"\
	".\Core\Model.h"\
	".\Core\modellist.h"\
	".\Core\ObjectList.h"\
	".\Core\Ortho.h"\
	".\Core\Select.h"\
	".\Core\TernList.h"\
	".\Core\TerrnObj.h"\
	".\Core\Undo.h"\
	".\Core\VertList.h"\
	".\CTextToolBar.h"\
	".\Defs.h"\
	".\Dialogs\AddModel.h"\
	".\Dialogs\buildspin.h"\
	".\Dialogs\DialogBuilderDlg.h"\
	".\Dialogs\DialogHelpers\ColorButton.h"\
	".\Dialogs\DialogHelpers\ColorPopup.h"\
	".\Dialogs\DialogHelpers\DropFileArray.h"\
	".\Dialogs\DialogHelpers\MyBar.h"\
	".\Dialogs\DialogHelpers\MyDropTarget.h"\
	".\Dialogs\DialogHelpers\scbarcf.h"\
	".\Dialogs\DialogHelpers\scbarg.h"\
	".\Dialogs\DialogHelpers\sizecbar.h"\
	".\Dialogs\DialogHelpers\TextureListControl.h"\
	".\Dialogs\DialogHelpers\TextureTreeControl.h"\
	".\Dialogs\DlgAutorecover.h"\
	".\Dialogs\Empty.h"\
	".\Dialogs\globalmaterials.h"\
	".\Dialogs\Groups.h"\
	".\Dialogs\Lists.h"\
	".\Dialogs\Models.h"\
	".\Dialogs\Properties.h"\
	".\Dialogs\TexturesDlg.h"\
	".\Dialogs\Timeline.h"\
	".\Doc.h"\
	".\DragTree.h"\
	".\grouptreectrl.h"\
	".\J3dmfc\J3DApp.h"\
	".\J3dmfc\J3DDoc.h"\
	".\J3dmfc\J3DMainFrm.h"\
	".\jet\Symbol.h"\
	".\jetdialog.h"\
	".\Jwe.h"\
	".\label.h"\
	".\MainFrm.h"\
	".\MfcUtil.h"\
	".\MsgLog.h"\
	".\Settings.h"\
	".\StdAfx.h"\
	".\TreeCtrlEx.h"\
	".\Util\jeList.h"\
	".\Util\Point.h"\
	".\Util\Rect.h"\
	".\WndReg.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Dialogs\Groups.h
# End Source File
# Begin Source File

SOURCE=.\Dialogs\Jeterr.cpp
DEP_CPP_JETER=\
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
	"..\..\..\include\jePlane.h"\
	"..\..\..\include\jePoly.h"\
	"..\..\..\include\jePortal.h"\
	"..\..\..\include\jeProperty.h"\
	"..\..\..\include\jePtrMgr.h"\
	"..\..\..\include\jeResource.h"\
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
	"..\..\..\include\Sound.h"\
	"..\..\..\include\UVMAP.H"\
	"..\..\..\include\Vec3d.h"\
	"..\..\..\include\VFILE.H"\
	"..\..\..\include\VideoMgr.h"\
	"..\..\..\include\Xform3d.h"\
	".\AppData.h"\
	".\Core\MaterialList.h"\
	".\Core\Materials.h"\
	".\Dialogs\DialogHelpers\scbarcf.h"\
	".\Dialogs\DialogHelpers\scbarg.h"\
	".\Dialogs\DialogHelpers\sizecbar.h"\
	".\Dialogs\DlgAutorecover.h"\
	".\Dialogs\Jeterr.h"\
	".\J3dmfc\J3DApp.h"\
	".\Jwe.h"\
	".\MsgLog.h"\
	".\Settings.h"\
	".\StdAfx.h"\
	".\Util\jeList.h"\
	".\WndReg.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Dialogs\Jeterr.h
# End Source File
# Begin Source File

SOURCE=.\Dialogs\Lists.cpp
DEP_CPP_LISTS=\
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
	"..\..\..\include\jePlane.h"\
	"..\..\..\include\jePoly.h"\
	"..\..\..\include\jePortal.h"\
	"..\..\..\include\jeProperty.h"\
	"..\..\..\include\jePtrMgr.h"\
	"..\..\..\include\jeResource.h"\
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
	"..\..\..\include\Sound.h"\
	"..\..\..\include\TERRAIN.H"\
	"..\..\..\include\UVMAP.H"\
	"..\..\..\include\Vec3d.h"\
	"..\..\..\include\VFILE.H"\
	"..\..\..\include\VideoMgr.h"\
	"..\..\..\include\Xform3d.h"\
	".\AppData.h"\
	".\BtnST.h"\
	".\CIconTabCtrl.h"\
	".\Core\Brush.h"\
	".\Core\BrushList.h"\
	".\Core\BrushTemplate.h"\
	".\Core\CameraList.h"\
	".\Core\CamObj.h"\
	".\Core\Class.h"\
	".\Core\Descriptor.h"\
	".\Core\Entity.h"\
	".\Core\EntityList.h"\
	".\Core\Group.h"\
	".\Core\GroupList.h"\
	".\Core\jwObject.h"\
	".\Core\Level.h"\
	".\Core\Light.h"\
	".\Core\LightList.h"\
	".\Core\MaterialIdentList.h"\
	".\Core\MaterialList.h"\
	".\Core\MaterialList2.h"\
	".\Core\Materials.h"\
	".\Core\Model.h"\
	".\Core\modellist.h"\
	".\Core\ObjectList.h"\
	".\Core\Ortho.h"\
	".\Core\Select.h"\
	".\Core\TernList.h"\
	".\Core\TerrnObj.h"\
	".\Core\Undo.h"\
	".\Core\VertList.h"\
	".\CTextToolBar.h"\
	".\Defs.h"\
	".\Dialogs\buildspin.h"\
	".\Dialogs\DialogBuilderDlg.h"\
	".\Dialogs\DialogHelpers\ColorButton.h"\
	".\Dialogs\DialogHelpers\ColorPopup.h"\
	".\Dialogs\DialogHelpers\DropFileArray.h"\
	".\Dialogs\DialogHelpers\MyBar.h"\
	".\Dialogs\DialogHelpers\MyDropTarget.h"\
	".\Dialogs\DialogHelpers\scbarcf.h"\
	".\Dialogs\DialogHelpers\scbarg.h"\
	".\Dialogs\DialogHelpers\sizecbar.h"\
	".\Dialogs\DialogHelpers\TextureListControl.h"\
	".\Dialogs\DialogHelpers\TextureTreeControl.h"\
	".\Dialogs\DlgAutorecover.h"\
	".\Dialogs\Empty.h"\
	".\Dialogs\globalmaterials.h"\
	".\Dialogs\Groups.h"\
	".\Dialogs\Lists.h"\
	".\Dialogs\Models.h"\
	".\Dialogs\Properties.h"\
	".\Dialogs\TexturesDlg.h"\
	".\Dialogs\Timeline.h"\
	".\Doc.h"\
	".\DragTree.h"\
	".\grouptreectrl.h"\
	".\J3dmfc\J3DApp.h"\
	".\J3dmfc\J3DDoc.h"\
	".\J3dmfc\J3DMainFrm.h"\
	".\jet\Symbol.h"\
	".\jetdialog.h"\
	".\Jwe.h"\
	".\label.h"\
	".\MainFrm.h"\
	".\MfcUtil.h"\
	".\MsgLog.h"\
	".\Settings.h"\
	".\StdAfx.h"\
	".\TreeCtrlEx.h"\
	".\Util\jeList.h"\
	".\Util\Point.h"\
	".\Util\Rect.h"\
	".\WndReg.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Dialogs\Lists.h
# End Source File
# Begin Source File

SOURCE=.\Dialogs\materialstatic.cpp
DEP_CPP_MATERIALS=\
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
	"..\..\..\include\jePlane.h"\
	"..\..\..\include\jePoly.h"\
	"..\..\..\include\jePortal.h"\
	"..\..\..\include\jeProperty.h"\
	"..\..\..\include\jePtrMgr.h"\
	"..\..\..\include\jeResource.h"\
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
	"..\..\..\include\Sound.h"\
	"..\..\..\include\UVMAP.H"\
	"..\..\..\include\Vec3d.h"\
	"..\..\..\include\VFILE.H"\
	"..\..\..\include\VideoMgr.h"\
	"..\..\..\include\Xform3d.h"\
	".\AppData.h"\
	".\Core\MaterialList.h"\
	".\Core\Materials.h"\
	".\Dialogs\DialogHelpers\scbarcf.h"\
	".\Dialogs\DialogHelpers\scbarg.h"\
	".\Dialogs\DialogHelpers\sizecbar.h"\
	".\Dialogs\DlgAutorecover.h"\
	".\Dialogs\materialstatic.h"\
	".\J3dmfc\J3DApp.h"\
	".\Jwe.h"\
	".\MsgLog.h"\
	".\Settings.h"\
	".\StdAfx.h"\
	".\Util\jeList.h"\
	".\WndReg.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Dialogs\materialstatic.h
# End Source File
# Begin Source File

SOURCE=.\Dialogs\Models.cpp
DEP_CPP_MODELS=\
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
	"..\..\..\include\jePlane.h"\
	"..\..\..\include\jePoly.h"\
	"..\..\..\include\jePortal.h"\
	"..\..\..\include\jeProperty.h"\
	"..\..\..\include\jePtrMgr.h"\
	"..\..\..\include\jeResource.h"\
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
	"..\..\..\include\Sound.h"\
	"..\..\..\include\TERRAIN.H"\
	"..\..\..\include\UVMAP.H"\
	"..\..\..\include\Vec3d.h"\
	"..\..\..\include\VFILE.H"\
	"..\..\..\include\VideoMgr.h"\
	"..\..\..\include\Xform3d.h"\
	".\AppData.h"\
	".\BtnST.h"\
	".\CIconTabCtrl.h"\
	".\Core\Brush.h"\
	".\Core\BrushList.h"\
	".\Core\BrushTemplate.h"\
	".\Core\CameraList.h"\
	".\Core\CamObj.h"\
	".\Core\Class.h"\
	".\Core\Descriptor.h"\
	".\Core\Entity.h"\
	".\Core\EntityList.h"\
	".\Core\Group.h"\
	".\Core\GroupList.h"\
	".\Core\jwObject.h"\
	".\Core\Level.h"\
	".\Core\Light.h"\
	".\Core\LightList.h"\
	".\Core\MaterialIdentList.h"\
	".\Core\MaterialList.h"\
	".\Core\MaterialList2.h"\
	".\Core\Materials.h"\
	".\Core\Model.h"\
	".\Core\modellist.h"\
	".\Core\ObjectList.h"\
	".\Core\Ortho.h"\
	".\Core\Select.h"\
	".\Core\TernList.h"\
	".\Core\TerrnObj.h"\
	".\Core\Undo.h"\
	".\Core\VertList.h"\
	".\CTextToolBar.h"\
	".\Defs.h"\
	".\Dialogs\AddModel.h"\
	".\Dialogs\buildspin.h"\
	".\Dialogs\DialogBuilderDlg.h"\
	".\Dialogs\DialogHelpers\ColorButton.h"\
	".\Dialogs\DialogHelpers\ColorPopup.h"\
	".\Dialogs\DialogHelpers\DropFileArray.h"\
	".\Dialogs\DialogHelpers\MyBar.h"\
	".\Dialogs\DialogHelpers\MyDropTarget.h"\
	".\Dialogs\DialogHelpers\scbarcf.h"\
	".\Dialogs\DialogHelpers\scbarg.h"\
	".\Dialogs\DialogHelpers\sizecbar.h"\
	".\Dialogs\DialogHelpers\TextureListControl.h"\
	".\Dialogs\DialogHelpers\TextureTreeControl.h"\
	".\Dialogs\DlgAutorecover.h"\
	".\Dialogs\Empty.h"\
	".\Dialogs\globalmaterials.h"\
	".\Dialogs\Groups.h"\
	".\Dialogs\Lists.h"\
	".\Dialogs\Models.h"\
	".\Dialogs\Properties.h"\
	".\Dialogs\TexturesDlg.h"\
	".\Dialogs\Timeline.h"\
	".\Doc.h"\
	".\DragTree.h"\
	".\grouptreectrl.h"\
	".\J3dmfc\J3DApp.h"\
	".\J3dmfc\J3DDoc.h"\
	".\J3dmfc\J3DMainFrm.h"\
	".\jet\Symbol.h"\
	".\jetdialog.h"\
	".\Jwe.h"\
	".\label.h"\
	".\MainFrm.h"\
	".\MfcUtil.h"\
	".\MsgLog.h"\
	".\Settings.h"\
	".\StdAfx.h"\
	".\TreeCtrlEx.h"\
	".\Util\jeList.h"\
	".\Util\Point.h"\
	".\Util\Rect.h"\
	".\WndReg.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Dialogs\Models.h
# End Source File
# Begin Source File

SOURCE=.\Dialogs\Paredit.cpp
DEP_CPP_PARED=\
	"..\..\..\include\BaseType.h"\
	".\Dialogs\DialogHelpers\scbarcf.h"\
	".\Dialogs\DialogHelpers\scbarg.h"\
	".\Dialogs\DialogHelpers\sizecbar.h"\
	".\Dialogs\Paredit.h"\
	".\StdAfx.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Dialogs\Paredit.h
# End Source File
# Begin Source File

SOURCE=.\Dialogs\Preferences.cpp
DEP_CPP_PREFE=\
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
	"..\..\..\include\jePlane.h"\
	"..\..\..\include\jePoly.h"\
	"..\..\..\include\jePortal.h"\
	"..\..\..\include\jeProperty.h"\
	"..\..\..\include\jePtrMgr.h"\
	"..\..\..\include\jeResource.h"\
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
	"..\..\..\include\Sound.h"\
	"..\..\..\include\TERRAIN.H"\
	"..\..\..\include\UVMAP.H"\
	"..\..\..\include\Vec3d.h"\
	"..\..\..\include\VFILE.H"\
	"..\..\..\include\VideoMgr.h"\
	"..\..\..\include\Xform3d.h"\
	".\AppData.h"\
	".\BtnST.h"\
	".\CIconTabCtrl.h"\
	".\Core\Brush.h"\
	".\Core\BrushList.h"\
	".\Core\BrushTemplate.h"\
	".\Core\CameraList.h"\
	".\Core\CamObj.h"\
	".\Core\Class.h"\
	".\Core\Descriptor.h"\
	".\Core\Entity.h"\
	".\Core\EntityList.h"\
	".\Core\Group.h"\
	".\Core\GroupList.h"\
	".\Core\jwObject.h"\
	".\Core\Level.h"\
	".\Core\Light.h"\
	".\Core\LightList.h"\
	".\Core\MaterialIdentList.h"\
	".\Core\MaterialList.h"\
	".\Core\MaterialList2.h"\
	".\Core\Materials.h"\
	".\Core\Model.h"\
	".\Core\modellist.h"\
	".\Core\ObjectList.h"\
	".\Core\Ortho.h"\
	".\Core\Select.h"\
	".\Core\TernList.h"\
	".\Core\TerrnObj.h"\
	".\Core\Undo.h"\
	".\Core\VertList.h"\
	".\CTextToolBar.h"\
	".\Defs.h"\
	".\Dialogs\buildspin.h"\
	".\Dialogs\DialogBuilderDlg.h"\
	".\Dialogs\DialogHelpers\ColorButton.h"\
	".\Dialogs\DialogHelpers\ColorPopup.h"\
	".\Dialogs\DialogHelpers\DropFileArray.h"\
	".\Dialogs\DialogHelpers\MyBar.h"\
	".\Dialogs\DialogHelpers\MyDropTarget.h"\
	".\Dialogs\DialogHelpers\scbarcf.h"\
	".\Dialogs\DialogHelpers\scbarg.h"\
	".\Dialogs\DialogHelpers\sizecbar.h"\
	".\Dialogs\DialogHelpers\TextureListControl.h"\
	".\Dialogs\DialogHelpers\TextureTreeControl.h"\
	".\Dialogs\DlgAutorecover.h"\
	".\Dialogs\Empty.h"\
	".\Dialogs\globalmaterials.h"\
	".\Dialogs\Groups.h"\
	".\Dialogs\Lists.h"\
	".\Dialogs\Models.h"\
	".\Dialogs\Preferences.h"\
	".\Dialogs\PressKey.h"\
	".\Dialogs\Properties.h"\
	".\Dialogs\TexturesDlg.h"\
	".\Dialogs\Timeline.h"\
	".\Doc.h"\
	".\DragTree.h"\
	".\ExtFileDialog.h"\
	".\grouptreectrl.h"\
	".\J3dmfc\J3DApp.h"\
	".\J3dmfc\J3DDoc.h"\
	".\J3dmfc\J3DMainFrm.h"\
	".\J3dmfc\J3DView.h"\
	".\jet\Symbol.h"\
	".\jetdialog.h"\
	".\JetView.h"\
	".\Jwe.h"\
	".\label.h"\
	".\MainFrm.h"\
	".\MsgLog.h"\
	".\Settings.h"\
	".\StdAfx.h"\
	".\TreeCtrlEx.h"\
	".\Util\DrawTool.h"\
	".\Util\Drvlist.h"\
	".\Util\jeList.h"\
	".\Util\Point.h"\
	".\Util\Rect.h"\
	".\WndReg.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Dialogs\Preferences.h
# End Source File
# Begin Source File

SOURCE=.\Dialogs\PressKey.cpp
DEP_CPP_PRESS=\
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
	"..\..\..\include\jePlane.h"\
	"..\..\..\include\jePoly.h"\
	"..\..\..\include\jePortal.h"\
	"..\..\..\include\jeProperty.h"\
	"..\..\..\include\jePtrMgr.h"\
	"..\..\..\include\jeResource.h"\
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
	"..\..\..\include\Sound.h"\
	"..\..\..\include\UVMAP.H"\
	"..\..\..\include\Vec3d.h"\
	"..\..\..\include\VFILE.H"\
	"..\..\..\include\VideoMgr.h"\
	"..\..\..\include\Xform3d.h"\
	".\AppData.h"\
	".\Core\MaterialList.h"\
	".\Core\Materials.h"\
	".\Dialogs\DialogHelpers\scbarcf.h"\
	".\Dialogs\DialogHelpers\scbarg.h"\
	".\Dialogs\DialogHelpers\sizecbar.h"\
	".\Dialogs\DlgAutorecover.h"\
	".\Dialogs\PressKey.h"\
	".\J3dmfc\J3DApp.h"\
	".\Jwe.h"\
	".\MsgLog.h"\
	".\Settings.h"\
	".\StdAfx.h"\
	".\Util\jeList.h"\
	".\WndReg.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Dialogs\PressKey.h
# End Source File
# Begin Source File

SOURCE=.\Dialogs\Properties.cpp
DEP_CPP_PROPE=\
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
	"..\..\..\include\jePlane.h"\
	"..\..\..\include\jePoly.h"\
	"..\..\..\include\jePortal.h"\
	"..\..\..\include\jeProperty.h"\
	"..\..\..\include\jePtrMgr.h"\
	"..\..\..\include\jeResource.h"\
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
	"..\..\..\include\Sound.h"\
	"..\..\..\include\TERRAIN.H"\
	"..\..\..\include\UVMAP.H"\
	"..\..\..\include\Vec3d.h"\
	"..\..\..\include\VFILE.H"\
	"..\..\..\include\VideoMgr.h"\
	"..\..\..\include\Xform3d.h"\
	".\AppData.h"\
	".\BtnST.h"\
	".\CIconTabCtrl.h"\
	".\Core\Brush.h"\
	".\Core\BrushList.h"\
	".\Core\BrushTemplate.h"\
	".\Core\CameraList.h"\
	".\Core\CamObj.h"\
	".\Core\Class.h"\
	".\Core\Descriptor.h"\
	".\Core\Entity.h"\
	".\Core\EntityList.h"\
	".\Core\Group.h"\
	".\Core\GroupList.h"\
	".\Core\jwObject.h"\
	".\Core\Level.h"\
	".\Core\Light.h"\
	".\Core\LightList.h"\
	".\Core\MaterialIdentList.h"\
	".\Core\MaterialList.h"\
	".\Core\MaterialList2.h"\
	".\Core\Materials.h"\
	".\Core\Model.h"\
	".\Core\modellist.h"\
	".\Core\ObjectList.h"\
	".\Core\Ortho.h"\
	".\Core\Select.h"\
	".\Core\TernList.h"\
	".\Core\TerrnObj.h"\
	".\Core\Undo.h"\
	".\Core\VertList.h"\
	".\CTextToolBar.h"\
	".\Defs.h"\
	".\Dialogs\buildspin.h"\
	".\Dialogs\DialogBuilderDlg.h"\
	".\Dialogs\DialogHelpers\ColorButton.h"\
	".\Dialogs\DialogHelpers\ColorPopup.h"\
	".\Dialogs\DialogHelpers\DropFileArray.h"\
	".\Dialogs\DialogHelpers\MyBar.h"\
	".\Dialogs\DialogHelpers\MyDropTarget.h"\
	".\Dialogs\DialogHelpers\scbarcf.h"\
	".\Dialogs\DialogHelpers\scbarg.h"\
	".\Dialogs\DialogHelpers\sizecbar.h"\
	".\Dialogs\DialogHelpers\TextureListControl.h"\
	".\Dialogs\DialogHelpers\TextureTreeControl.h"\
	".\Dialogs\DlgAutorecover.h"\
	".\Dialogs\Empty.h"\
	".\Dialogs\globalmaterials.h"\
	".\Dialogs\Groups.h"\
	".\Dialogs\Lists.h"\
	".\Dialogs\Models.h"\
	".\Dialogs\Properties.h"\
	".\Dialogs\TexturesDlg.h"\
	".\Dialogs\Timeline.h"\
	".\Doc.h"\
	".\DragTree.h"\
	".\grouptreectrl.h"\
	".\J3dmfc\J3DApp.h"\
	".\J3dmfc\J3DDoc.h"\
	".\J3dmfc\J3DMainFrm.h"\
	".\jet\Symbol.h"\
	".\jetdialog.h"\
	".\Jwe.h"\
	".\label.h"\
	".\MainFrm.h"\
	".\MsgLog.h"\
	".\Settings.h"\
	".\StdAfx.h"\
	".\TreeCtrlEx.h"\
	".\Util\jeList.h"\
	".\Util\Point.h"\
	".\Util\Rect.h"\
	".\WndReg.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Dialogs\Properties.h
# End Source File
# Begin Source File

SOURCE=.\Dialogs\Rebuild.cpp
DEP_CPP_REBUI=\
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
	"..\..\..\include\jePlane.h"\
	"..\..\..\include\jePoly.h"\
	"..\..\..\include\jePortal.h"\
	"..\..\..\include\jeProperty.h"\
	"..\..\..\include\jePtrMgr.h"\
	"..\..\..\include\jeResource.h"\
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
	"..\..\..\include\Sound.h"\
	"..\..\..\include\TERRAIN.H"\
	"..\..\..\include\UVMAP.H"\
	"..\..\..\include\Vec3d.h"\
	"..\..\..\include\VFILE.H"\
	"..\..\..\include\VideoMgr.h"\
	"..\..\..\include\Xform3d.h"\
	".\AppData.h"\
	".\BtnST.h"\
	".\CIconTabCtrl.h"\
	".\Core\Brush.h"\
	".\Core\BrushList.h"\
	".\Core\BrushTemplate.h"\
	".\Core\CameraList.h"\
	".\Core\CamObj.h"\
	".\Core\Class.h"\
	".\Core\Descriptor.h"\
	".\Core\Entity.h"\
	".\Core\EntityList.h"\
	".\Core\Group.h"\
	".\Core\GroupList.h"\
	".\Core\jwObject.h"\
	".\Core\Level.h"\
	".\Core\Light.h"\
	".\Core\LightList.h"\
	".\Core\MaterialIdentList.h"\
	".\Core\MaterialList.h"\
	".\Core\MaterialList2.h"\
	".\Core\Materials.h"\
	".\Core\Model.h"\
	".\Core\modellist.h"\
	".\Core\ObjectList.h"\
	".\Core\Ortho.h"\
	".\Core\Select.h"\
	".\Core\TernList.h"\
	".\Core\TerrnObj.h"\
	".\Core\Undo.h"\
	".\Core\VertList.h"\
	".\CTextToolBar.h"\
	".\Defs.h"\
	".\Dialogs\buildspin.h"\
	".\Dialogs\DialogBuilderDlg.h"\
	".\Dialogs\DialogHelpers\ColorButton.h"\
	".\Dialogs\DialogHelpers\ColorPopup.h"\
	".\Dialogs\DialogHelpers\DropFileArray.h"\
	".\Dialogs\DialogHelpers\MyBar.h"\
	".\Dialogs\DialogHelpers\MyDropTarget.h"\
	".\Dialogs\DialogHelpers\scbarcf.h"\
	".\Dialogs\DialogHelpers\scbarg.h"\
	".\Dialogs\DialogHelpers\sizecbar.h"\
	".\Dialogs\DialogHelpers\TextureListControl.h"\
	".\Dialogs\DialogHelpers\TextureTreeControl.h"\
	".\Dialogs\DlgAutorecover.h"\
	".\Dialogs\Empty.h"\
	".\Dialogs\globalmaterials.h"\
	".\Dialogs\Groups.h"\
	".\Dialogs\Lists.h"\
	".\Dialogs\Models.h"\
	".\Dialogs\Properties.h"\
	".\Dialogs\Rebuild.h"\
	".\Dialogs\TexturesDlg.h"\
	".\Dialogs\Timeline.h"\
	".\Doc.h"\
	".\DragTree.h"\
	".\grouptreectrl.h"\
	".\J3dmfc\J3DApp.h"\
	".\J3dmfc\J3DDoc.h"\
	".\J3dmfc\J3DMainFrm.h"\
	".\jet\Symbol.h"\
	".\jetdialog.h"\
	".\Jwe.h"\
	".\label.h"\
	".\MainFrm.h"\
	".\MsgLog.h"\
	".\Settings.h"\
	".\StdAfx.h"\
	".\TreeCtrlEx.h"\
	".\Util\jeList.h"\
	".\Util\Point.h"\
	".\Util\Rect.h"\
	".\WndReg.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Dialogs\Rebuild.h
# End Source File
# Begin Source File

SOURCE=.\Dialogs\Render.cpp
DEP_CPP_RENDE=\
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
	"..\..\..\include\jePlane.h"\
	"..\..\..\include\jePoly.h"\
	"..\..\..\include\jePortal.h"\
	"..\..\..\include\jeProperty.h"\
	"..\..\..\include\jePtrMgr.h"\
	"..\..\..\include\jeResource.h"\
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
	"..\..\..\include\Sound.h"\
	"..\..\..\include\TERRAIN.H"\
	"..\..\..\include\UVMAP.H"\
	"..\..\..\include\Vec3d.h"\
	"..\..\..\include\VFILE.H"\
	"..\..\..\include\VideoMgr.h"\
	"..\..\..\include\Xform3d.h"\
	".\AppData.h"\
	".\BtnST.h"\
	".\CIconTabCtrl.h"\
	".\Core\Brush.h"\
	".\Core\BrushList.h"\
	".\Core\BrushTemplate.h"\
	".\Core\CameraList.h"\
	".\Core\CamObj.h"\
	".\Core\Class.h"\
	".\Core\Descriptor.h"\
	".\Core\Entity.h"\
	".\Core\EntityList.h"\
	".\Core\Group.h"\
	".\Core\GroupList.h"\
	".\Core\jwObject.h"\
	".\Core\Level.h"\
	".\Core\Light.h"\
	".\Core\LightList.h"\
	".\Core\MaterialIdentList.h"\
	".\Core\MaterialList.h"\
	".\Core\MaterialList2.h"\
	".\Core\Materials.h"\
	".\Core\Model.h"\
	".\Core\modellist.h"\
	".\Core\ObjectList.h"\
	".\Core\Ortho.h"\
	".\Core\Select.h"\
	".\Core\TernList.h"\
	".\Core\TerrnObj.h"\
	".\Core\Undo.h"\
	".\Core\VertList.h"\
	".\CTextToolBar.h"\
	".\Defs.h"\
	".\Dialogs\buildspin.h"\
	".\Dialogs\DialogBuilderDlg.h"\
	".\Dialogs\DialogHelpers\ColorButton.h"\
	".\Dialogs\DialogHelpers\ColorPopup.h"\
	".\Dialogs\DialogHelpers\DropFileArray.h"\
	".\Dialogs\DialogHelpers\MyBar.h"\
	".\Dialogs\DialogHelpers\MyDropTarget.h"\
	".\Dialogs\DialogHelpers\scbarcf.h"\
	".\Dialogs\DialogHelpers\scbarg.h"\
	".\Dialogs\DialogHelpers\sizecbar.h"\
	".\Dialogs\DialogHelpers\TextureListControl.h"\
	".\Dialogs\DialogHelpers\TextureTreeControl.h"\
	".\Dialogs\DlgAutorecover.h"\
	".\Dialogs\Empty.h"\
	".\Dialogs\globalmaterials.h"\
	".\Dialogs\Groups.h"\
	".\Dialogs\Lists.h"\
	".\Dialogs\Models.h"\
	".\Dialogs\Properties.h"\
	".\Dialogs\Render.h"\
	".\Dialogs\TexturesDlg.h"\
	".\Dialogs\Timeline.h"\
	".\Doc.h"\
	".\DragTree.h"\
	".\grouptreectrl.h"\
	".\J3dmfc\J3DApp.h"\
	".\J3dmfc\J3DDoc.h"\
	".\J3dmfc\J3DMainFrm.h"\
	".\jet\Symbol.h"\
	".\jetdialog.h"\
	".\Jwe.h"\
	".\label.h"\
	".\MainFrm.h"\
	".\MfcUtil.h"\
	".\MsgLog.h"\
	".\Settings.h"\
	".\StdAfx.h"\
	".\TreeCtrlEx.h"\
	".\Util\jeList.h"\
	".\Util\Point.h"\
	".\Util\Rect.h"\
	".\WndReg.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Dialogs\Render.h
# End Source File
# Begin Source File

SOURCE=.\Dialogs\ReportErr.h
# End Source File
# Begin Source File

SOURCE=.\Dialogs\Stats.cpp
DEP_CPP_STATS=\
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
	"..\..\..\include\jePlane.h"\
	"..\..\..\include\jePoly.h"\
	"..\..\..\include\jePortal.h"\
	"..\..\..\include\jeProperty.h"\
	"..\..\..\include\jePtrMgr.h"\
	"..\..\..\include\jeResource.h"\
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
	"..\..\..\include\Sound.h"\
	"..\..\..\include\UVMAP.H"\
	"..\..\..\include\Vec3d.h"\
	"..\..\..\include\VFILE.H"\
	"..\..\..\include\VideoMgr.h"\
	"..\..\..\include\Xform3d.h"\
	".\AppData.h"\
	".\Core\MaterialList.h"\
	".\Core\Materials.h"\
	".\Dialogs\DialogHelpers\scbarcf.h"\
	".\Dialogs\DialogHelpers\scbarg.h"\
	".\Dialogs\DialogHelpers\sizecbar.h"\
	".\Dialogs\DlgAutorecover.h"\
	".\J3dmfc\J3DApp.h"\
	".\Jwe.h"\
	".\MsgLog.h"\
	".\Settings.h"\
	".\StdAfx.h"\
	".\Util\jeList.h"\
	".\WndReg.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Dialogs\Stats.h
# End Source File
# Begin Source File

SOURCE=.\Dialogs\Textures.cpp
DEP_CPP_TEXTURE=\
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
	"..\..\..\include\jePlane.h"\
	"..\..\..\include\jePoly.h"\
	"..\..\..\include\jePortal.h"\
	"..\..\..\include\jeProperty.h"\
	"..\..\..\include\jePtrMgr.h"\
	"..\..\..\include\jeResource.h"\
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
	"..\..\..\include\Sound.h"\
	"..\..\..\include\TERRAIN.H"\
	"..\..\..\include\UVMAP.H"\
	"..\..\..\include\Vec3d.h"\
	"..\..\..\include\VFILE.H"\
	"..\..\..\include\VideoMgr.h"\
	"..\..\..\include\Xform3d.h"\
	".\AppData.h"\
	".\BtnST.h"\
	".\CIconTabCtrl.h"\
	".\Core\Brush.h"\
	".\Core\BrushList.h"\
	".\Core\BrushTemplate.h"\
	".\Core\CameraList.h"\
	".\Core\CamObj.h"\
	".\Core\Class.h"\
	".\Core\Descriptor.h"\
	".\Core\Entity.h"\
	".\Core\EntityList.h"\
	".\Core\Group.h"\
	".\Core\GroupList.h"\
	".\Core\jwObject.h"\
	".\Core\Level.h"\
	".\Core\Light.h"\
	".\Core\LightList.h"\
	".\Core\MaterialIdentList.h"\
	".\Core\MaterialList.h"\
	".\Core\MaterialList2.h"\
	".\Core\Materials.h"\
	".\Core\Model.h"\
	".\Core\modellist.h"\
	".\Core\ObjectList.h"\
	".\Core\Ortho.h"\
	".\Core\Select.h"\
	".\Core\TernList.h"\
	".\Core\TerrnObj.h"\
	".\Core\Undo.h"\
	".\Core\VertList.h"\
	".\CTextToolBar.h"\
	".\Defs.h"\
	".\Dialogs\buildspin.h"\
	".\Dialogs\DialogBuilderDlg.h"\
	".\Dialogs\DialogHelpers\ColorButton.h"\
	".\Dialogs\DialogHelpers\ColorPopup.h"\
	".\Dialogs\DialogHelpers\DropFileArray.h"\
	".\Dialogs\DialogHelpers\MyBar.h"\
	".\Dialogs\DialogHelpers\MyDropTarget.h"\
	".\Dialogs\DialogHelpers\scbarcf.h"\
	".\Dialogs\DialogHelpers\scbarg.h"\
	".\Dialogs\DialogHelpers\sizecbar.h"\
	".\Dialogs\DialogHelpers\TextureListControl.h"\
	".\Dialogs\DialogHelpers\TextureTreeControl.h"\
	".\Dialogs\DlgAutorecover.h"\
	".\Dialogs\Empty.h"\
	".\Dialogs\globalmaterials.h"\
	".\Dialogs\Groups.h"\
	".\Dialogs\Lists.h"\
	".\Dialogs\Models.h"\
	".\Dialogs\Properties.h"\
	".\Dialogs\Textures.h"\
	".\Dialogs\TexturesDlg.h"\
	".\Dialogs\Timeline.h"\
	".\Doc.h"\
	".\DragTree.h"\
	".\grouptreectrl.h"\
	".\J3dmfc\J3DApp.h"\
	".\J3dmfc\J3DDoc.h"\
	".\J3dmfc\J3DMainFrm.h"\
	".\jet\Symbol.h"\
	".\jetdialog.h"\
	".\Jwe.h"\
	".\label.h"\
	".\MainFrm.h"\
	".\MfcUtil.h"\
	".\MsgLog.h"\
	".\MyStatic.h"\
	".\Settings.h"\
	".\StdAfx.h"\
	".\TreeCtrlEx.h"\
	".\Util\jeList.h"\
	".\Util\Point.h"\
	".\Util\Rect.h"\
	".\WndReg.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Dialogs\Textures.h
# End Source File
# Begin Source File

SOURCE=.\Dialogs\TexturesDlg.cpp
DEP_CPP_TEXTURES=\
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
	"..\..\..\include\jePlane.h"\
	"..\..\..\include\jePoly.h"\
	"..\..\..\include\jePortal.h"\
	"..\..\..\include\jeProperty.h"\
	"..\..\..\include\jePtrMgr.h"\
	"..\..\..\include\jeResource.h"\
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
	"..\..\..\include\Sound.h"\
	"..\..\..\include\TERRAIN.H"\
	"..\..\..\include\UVMAP.H"\
	"..\..\..\include\Vec3d.h"\
	"..\..\..\include\VFILE.H"\
	"..\..\..\include\VideoMgr.h"\
	"..\..\..\include\Xform3d.h"\
	".\AppData.h"\
	".\BtnST.h"\
	".\CIconTabCtrl.h"\
	".\Core\Brush.h"\
	".\Core\BrushList.h"\
	".\Core\BrushTemplate.h"\
	".\Core\CameraList.h"\
	".\Core\CamObj.h"\
	".\Core\Class.h"\
	".\Core\Descriptor.h"\
	".\Core\Entity.h"\
	".\Core\EntityList.h"\
	".\Core\Group.h"\
	".\Core\GroupList.h"\
	".\Core\jwObject.h"\
	".\Core\Level.h"\
	".\Core\Light.h"\
	".\Core\LightList.h"\
	".\Core\MaterialIdentList.h"\
	".\Core\MaterialList.h"\
	".\Core\MaterialList2.h"\
	".\Core\Materials.h"\
	".\Core\Model.h"\
	".\Core\modellist.h"\
	".\Core\ObjectList.h"\
	".\Core\Ortho.h"\
	".\Core\Select.h"\
	".\Core\TernList.h"\
	".\Core\TerrnObj.h"\
	".\Core\Undo.h"\
	".\Core\VertList.h"\
	".\CTextToolBar.h"\
	".\Defs.h"\
	".\Dialogs\buildspin.h"\
	".\Dialogs\DialogBuilderDlg.h"\
	".\Dialogs\DialogHelpers\BitmapResize.h"\
	".\Dialogs\DialogHelpers\ColorButton.h"\
	".\Dialogs\DialogHelpers\ColorPopup.h"\
	".\Dialogs\DialogHelpers\DropFileArray.h"\
	".\Dialogs\DialogHelpers\MyBar.h"\
	".\Dialogs\DialogHelpers\MyDropTarget.h"\
	".\Dialogs\DialogHelpers\scbarcf.h"\
	".\Dialogs\DialogHelpers\scbarg.h"\
	".\Dialogs\DialogHelpers\sizecbar.h"\
	".\Dialogs\DialogHelpers\TextureListControl.h"\
	".\Dialogs\DialogHelpers\TextureTreeControl.h"\
	".\Dialogs\DlgAutorecover.h"\
	".\Dialogs\Empty.h"\
	".\Dialogs\globalmaterials.h"\
	".\Dialogs\Groups.h"\
	".\Dialogs\Lists.h"\
	".\Dialogs\Models.h"\
	".\Dialogs\Properties.h"\
	".\Dialogs\TexturesDlg.h"\
	".\Dialogs\Timeline.h"\
	".\Doc.h"\
	".\DragTree.h"\
	".\grouptreectrl.h"\
	".\J3dmfc\J3DApp.h"\
	".\J3dmfc\J3DDoc.h"\
	".\J3dmfc\J3DMainFrm.h"\
	".\jet\Symbol.h"\
	".\jetdialog.h"\
	".\Jwe.h"\
	".\label.h"\
	".\MainFrm.h"\
	".\MsgLog.h"\
	".\Settings.h"\
	".\StdAfx.h"\
	".\TreeCtrlEx.h"\
	".\Util\Bmp.h"\
	".\Util\jeList.h"\
	".\Util\Point.h"\
	".\Util\Rect.h"\
	".\WndReg.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Dialogs\TexturesDlg.h
# End Source File
# Begin Source File

SOURCE=.\Dialogs\Timeline.cpp
DEP_CPP_TIMEL=\
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
	"..\..\..\include\jePlane.h"\
	"..\..\..\include\jePoly.h"\
	"..\..\..\include\jePortal.h"\
	"..\..\..\include\jeProperty.h"\
	"..\..\..\include\jePtrMgr.h"\
	"..\..\..\include\jeResource.h"\
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
	"..\..\..\include\Sound.h"\
	"..\..\..\include\TERRAIN.H"\
	"..\..\..\include\UVMAP.H"\
	"..\..\..\include\Vec3d.h"\
	"..\..\..\include\VFILE.H"\
	"..\..\..\include\VideoMgr.h"\
	"..\..\..\include\Xform3d.h"\
	".\AppData.h"\
	".\BtnST.h"\
	".\CIconTabCtrl.h"\
	".\Core\Brush.h"\
	".\Core\BrushList.h"\
	".\Core\BrushTemplate.h"\
	".\Core\CameraList.h"\
	".\Core\CamObj.h"\
	".\Core\Class.h"\
	".\Core\Descriptor.h"\
	".\Core\Entity.h"\
	".\Core\EntityList.h"\
	".\Core\Group.h"\
	".\Core\GroupList.h"\
	".\Core\jwObject.h"\
	".\Core\Level.h"\
	".\Core\Light.h"\
	".\Core\LightList.h"\
	".\Core\MaterialIdentList.h"\
	".\Core\MaterialList.h"\
	".\Core\MaterialList2.h"\
	".\Core\Materials.h"\
	".\Core\Model.h"\
	".\Core\modellist.h"\
	".\Core\ObjectList.h"\
	".\Core\Ortho.h"\
	".\Core\Select.h"\
	".\Core\TernList.h"\
	".\Core\TerrnObj.h"\
	".\Core\Undo.h"\
	".\Core\VertList.h"\
	".\CTextToolBar.h"\
	".\Defs.h"\
	".\Dialogs\buildspin.h"\
	".\Dialogs\DialogBuilderDlg.h"\
	".\Dialogs\DialogHelpers\ColorButton.h"\
	".\Dialogs\DialogHelpers\ColorPopup.h"\
	".\Dialogs\DialogHelpers\DropFileArray.h"\
	".\Dialogs\DialogHelpers\MyBar.h"\
	".\Dialogs\DialogHelpers\MyDropTarget.h"\
	".\Dialogs\DialogHelpers\scbarcf.h"\
	".\Dialogs\DialogHelpers\scbarg.h"\
	".\Dialogs\DialogHelpers\sizecbar.h"\
	".\Dialogs\DialogHelpers\TextureListControl.h"\
	".\Dialogs\DialogHelpers\TextureTreeControl.h"\
	".\Dialogs\DlgAutorecover.h"\
	".\Dialogs\Empty.h"\
	".\Dialogs\globalmaterials.h"\
	".\Dialogs\Groups.h"\
	".\Dialogs\Lists.h"\
	".\Dialogs\Models.h"\
	".\Dialogs\Properties.h"\
	".\Dialogs\TexturesDlg.h"\
	".\Dialogs\Timeline.h"\
	".\Doc.h"\
	".\DragTree.h"\
	".\grouptreectrl.h"\
	".\J3dmfc\J3DApp.h"\
	".\J3dmfc\J3DDoc.h"\
	".\J3dmfc\J3DMainFrm.h"\
	".\jet\Symbol.h"\
	".\jetdialog.h"\
	".\Jwe.h"\
	".\label.h"\
	".\MainFrm.h"\
	".\MsgLog.h"\
	".\ObjectMsg.h"\
	".\Settings.h"\
	".\StdAfx.h"\
	".\TreeCtrlEx.h"\
	".\Util\jeList.h"\
	".\Util\Point.h"\
	".\Util\Rect.h"\
	".\WndReg.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Dialogs\Timeline.h
# End Source File
# End Group
# Begin Group "Utilities"

# PROP Default_Filter "*.cpp, *.c, *.h"
# Begin Source File

SOURCE=.\Util\Bmp.c
DEP_CPP_BMP_C=\
	"..\..\..\include\ARRAY.H"\
	"..\..\..\include\BaseType.h"\
	"..\..\..\include\BITMAP.H"\
	"..\..\..\include\Camera.h"\
	"..\..\..\include\ENGINE.H"\
	"..\..\..\include\jeMaterial.h"\
	"..\..\..\include\jeMemAllocInfo.h"\
	"..\..\..\include\jePtrMgr.h"\
	"..\..\..\include\jeTypes.h"\
	"..\..\..\include\PixelFormat.h"\
	"..\..\..\include\Ram.h"\
	"..\..\..\include\Vec3d.h"\
	"..\..\..\include\VFILE.H"\
	"..\..\..\include\Xform3d.h"\
	".\Util\Bmp.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Util\Bmp.h
# End Source File
# Begin Source File

SOURCE=.\Util\BmpPool.c
DEP_CPP_BMPPO=\
	"..\..\..\include\BaseType.h"\
	"..\..\..\include\jeMemAllocInfo.h"\
	"..\..\..\include\Ram.h"\
	".\Util\BmpPool.h"\
	".\Util\jeList.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Util\BmpPool.h
# End Source File
# Begin Source File

SOURCE=.\Util\Dlist.c
DEP_CPP_DLIST=\
	"..\..\..\include\BaseType.h"\
	"..\..\..\include\jeMemAllocInfo.h"\
	"..\..\..\include\Ram.h"\
	".\Util\Dlist.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Util\Dlist.h
# End Source File
# Begin Source File

SOURCE=.\Util\DrawTool.c
DEP_CPP_DRAWT=\
	"..\..\..\include\BaseType.h"\
	"..\..\..\include\jeMemAllocInfo.h"\
	"..\..\..\include\Ram.h"\
	".\Util\BmpPool.h"\
	".\Util\DrawTool.h"\
	".\Util\Point.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Util\DrawTool.h
# End Source File
# Begin Source File

SOURCE=.\Util\Drvlist.c
DEP_CPP_DRVLI=\
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
	"..\..\..\include\jePlane.h"\
	"..\..\..\include\jePoly.h"\
	"..\..\..\include\jePortal.h"\
	"..\..\..\include\jeProperty.h"\
	"..\..\..\include\jePtrMgr.h"\
	"..\..\..\include\jeResource.h"\
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
	"..\..\..\include\Sound.h"\
	"..\..\..\include\UVMAP.H"\
	"..\..\..\include\Vec3d.h"\
	"..\..\..\include\VFILE.H"\
	"..\..\..\include\VideoMgr.h"\
	"..\..\..\include\Xform3d.h"\
	".\Util\Drvlist.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Util\Drvlist.h
# End Source File
# Begin Source File

SOURCE=.\Util\jeList.c
DEP_CPP_JELIS=\
	"..\..\..\include\BaseType.h"\
	"..\..\..\include\jeMemAllocInfo.h"\
	"..\..\..\include\Ram.h"\
	".\Util\jeList.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Util\jeList.h
# End Source File
# Begin Source File

SOURCE=.\Util\Mtl9937.cpp
# End Source File
# Begin Source File

SOURCE=.\Util\Mtl9937.h
# End Source File
# Begin Source File

SOURCE=.\Util\Noise.cpp
DEP_CPP_NOISE=\
	"..\..\..\include\BaseType.h"\
	"..\..\..\include\Vec3d.h"\
	".\Util\Mtl9937.h"\
	".\Util\Noise.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Util\Noise.h
# End Source File
# Begin Source File

SOURCE=.\Util\Point.h
# End Source File
# Begin Source File

SOURCE=.\Util\Rect.c
DEP_CPP_RECT_=\
	"..\..\..\include\BaseType.h"\
	".\Util\Point.h"\
	".\Util\Rect.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Util\Rect.h
# End Source File
# Begin Source File

SOURCE=.\Util\Spawn.c
DEP_CPP_SPAWN=\
	".\Util\Spawn.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Util\Spawn.h
# End Source File
# Begin Source File

SOURCE=.\Util\Units.h
# End Source File
# Begin Source File

SOURCE=.\Util\Util.c
DEP_CPP_UTIL_=\
	"..\..\..\include\BaseType.h"\
	"..\..\..\include\jeMemAllocInfo.h"\
	"..\..\..\include\Ram.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Util\Util.h
# End Source File
# End Group
# Begin Group "MFC Files"

# PROP Default_Filter "cpp;c;cxx;h;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\AppData.cpp
DEP_CPP_APPDA=\
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
	"..\..\..\include\jePlane.h"\
	"..\..\..\include\jePoly.h"\
	"..\..\..\include\jePortal.h"\
	"..\..\..\include\jeProperty.h"\
	"..\..\..\include\jePtrMgr.h"\
	"..\..\..\include\jeResource.h"\
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
	"..\..\..\include\Sound.h"\
	"..\..\..\include\UVMAP.H"\
	"..\..\..\include\Vec3d.h"\
	"..\..\..\include\VFILE.H"\
	"..\..\..\include\VideoMgr.h"\
	"..\..\..\include\Xform3d.h"\
	".\AppData.h"\
	".\Core\MaterialList.h"\
	".\Core\Materials.h"\
	".\Dialogs\DialogHelpers\scbarcf.h"\
	".\Dialogs\DialogHelpers\scbarg.h"\
	".\Dialogs\DialogHelpers\sizecbar.h"\
	".\Dialogs\DlgAutorecover.h"\
	".\J3dmfc\J3DApp.h"\
	".\Jwe.h"\
	".\MsgLog.h"\
	".\Settings.h"\
	".\StdAfx.h"\
	".\Util\jeList.h"\
	".\WndReg.h"\
	
# End Source File
# Begin Source File

SOURCE=.\AppData.h
# End Source File
# Begin Source File

SOURCE=.\BtnST.cpp
DEP_CPP_BTNST=\
	"..\..\..\include\BaseType.h"\
	".\BtnST.h"\
	".\Dialogs\DialogHelpers\scbarcf.h"\
	".\Dialogs\DialogHelpers\scbarg.h"\
	".\Dialogs\DialogHelpers\sizecbar.h"\
	".\Memdc.h"\
	".\StdAfx.h"\
	
# End Source File
# Begin Source File

SOURCE=.\BtnST.h
# End Source File
# Begin Source File

SOURCE=.\BuildNumber.h
# End Source File
# Begin Source File

SOURCE=.\BuildType.h
# End Source File
# Begin Source File

SOURCE=.\CamFieldID.h
# End Source File
# Begin Source File

SOURCE=.\ChildFrm.cpp
DEP_CPP_CHILD=\
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
	"..\..\..\include\jePlane.h"\
	"..\..\..\include\jePoly.h"\
	"..\..\..\include\jePortal.h"\
	"..\..\..\include\jeProperty.h"\
	"..\..\..\include\jePtrMgr.h"\
	"..\..\..\include\jeResource.h"\
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
	"..\..\..\include\Sound.h"\
	"..\..\..\include\TERRAIN.H"\
	"..\..\..\include\UVMAP.H"\
	"..\..\..\include\Vec3d.h"\
	"..\..\..\include\VFILE.H"\
	"..\..\..\include\VideoMgr.h"\
	"..\..\..\include\Xform3d.h"\
	".\AppData.h"\
	".\BtnST.h"\
	".\ChildFrm.h"\
	".\CIconTabCtrl.h"\
	".\Core\Brush.h"\
	".\Core\BrushList.h"\
	".\Core\BrushTemplate.h"\
	".\Core\CameraList.h"\
	".\Core\CamObj.h"\
	".\Core\Class.h"\
	".\Core\Descriptor.h"\
	".\Core\Entity.h"\
	".\Core\EntityList.h"\
	".\Core\Group.h"\
	".\Core\GroupList.h"\
	".\Core\jwObject.h"\
	".\Core\Level.h"\
	".\Core\Light.h"\
	".\Core\LightList.h"\
	".\Core\MaterialIdentList.h"\
	".\Core\MaterialList.h"\
	".\Core\Materials.h"\
	".\Core\Model.h"\
	".\Core\modellist.h"\
	".\Core\ObjectList.h"\
	".\Core\Ortho.h"\
	".\Core\Select.h"\
	".\Core\TernList.h"\
	".\Core\TerrnObj.h"\
	".\Core\Undo.h"\
	".\Core\VertList.h"\
	".\Defs.h"\
	".\Dialogs\DialogHelpers\scbarcf.h"\
	".\Dialogs\DialogHelpers\scbarg.h"\
	".\Dialogs\DialogHelpers\sizecbar.h"\
	".\Dialogs\DlgAutorecover.h"\
	".\Dialogs\Properties.h"\
	".\Doc.h"\
	".\J3dmfc\J3DApp.h"\
	".\J3dmfc\J3DDoc.h"\
	".\J3dmfc\J3DView.h"\
	".\jet\Symbol.h"\
	".\JetView.h"\
	".\Jwe.h"\
	".\label.h"\
	".\MsgLog.h"\
	".\Settings.h"\
	".\StdAfx.h"\
	".\Util\jeList.h"\
	".\Util\Point.h"\
	".\Util\Rect.h"\
	".\View.h"\
	".\WndReg.h"\
	
# End Source File
# Begin Source File

SOURCE=.\ChildFrm.h
# End Source File
# Begin Source File

SOURCE=.\CIconTabCtrl.cpp
DEP_CPP_CICON=\
	"..\..\..\include\BaseType.h"\
	".\BtnST.h"\
	".\CIconTabCtrl.h"\
	".\Dialogs\DialogHelpers\scbarcf.h"\
	".\Dialogs\DialogHelpers\scbarg.h"\
	".\Dialogs\DialogHelpers\sizecbar.h"\
	".\label.h"\
	".\StdAfx.h"\
	
# End Source File
# Begin Source File

SOURCE=.\CIconTabCtrl.h
# End Source File
# Begin Source File

SOURCE=.\CMaterial.cpp
DEP_CPP_CMATE=\
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
	"..\..\..\include\jePlane.h"\
	"..\..\..\include\jePoly.h"\
	"..\..\..\include\jePortal.h"\
	"..\..\..\include\jeProperty.h"\
	"..\..\..\include\jePtrMgr.h"\
	"..\..\..\include\jeResource.h"\
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
	"..\..\..\include\Sound.h"\
	"..\..\..\include\UVMAP.H"\
	"..\..\..\include\Vec3d.h"\
	"..\..\..\include\VFILE.H"\
	"..\..\..\include\VideoMgr.h"\
	"..\..\..\include\Xform3d.h"\
	".\AppData.h"\
	".\CMaterial.h"\
	".\Core\MaterialList.h"\
	".\Core\Materials.h"\
	".\Dialogs\DialogHelpers\scbarcf.h"\
	".\Dialogs\DialogHelpers\scbarg.h"\
	".\Dialogs\DialogHelpers\sizecbar.h"\
	".\Dialogs\DlgAutorecover.h"\
	".\J3dmfc\J3DApp.h"\
	".\Jwe.h"\
	".\MsgLog.h"\
	".\Settings.h"\
	".\StdAfx.h"\
	".\Util\jeList.h"\
	".\WndReg.h"\
	
# End Source File
# Begin Source File

SOURCE=.\CMaterial.h
# End Source File
# Begin Source File

SOURCE=.\CTextToolBar.cpp
DEP_CPP_CTEXT=\
	"..\..\..\include\BaseType.h"\
	".\CTextToolBar.h"\
	".\Dialogs\DialogHelpers\scbarcf.h"\
	".\Dialogs\DialogHelpers\scbarg.h"\
	".\Dialogs\DialogHelpers\sizecbar.h"\
	".\StdAfx.h"\
	
# End Source File
# Begin Source File

SOURCE=.\CTextToolBar.h
# End Source File
# Begin Source File

SOURCE=.\Defs.h
# End Source File
# Begin Source File

SOURCE=.\Doc.cpp
DEP_CPP_DOC_C=\
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
	"..\..\..\include\jePlane.h"\
	"..\..\..\include\jePoly.h"\
	"..\..\..\include\jePortal.h"\
	"..\..\..\include\jeProperty.h"\
	"..\..\..\include\jePtrMgr.h"\
	"..\..\..\include\jeResource.h"\
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
	"..\..\..\include\Sound.h"\
	"..\..\..\include\TERRAIN.H"\
	"..\..\..\include\UVMAP.H"\
	"..\..\..\include\Vec3d.h"\
	"..\..\..\include\VFILE.H"\
	"..\..\..\include\VideoMgr.h"\
	"..\..\..\include\Xform3d.h"\
	".\AppData.h"\
	".\BtnST.h"\
	".\CIconTabCtrl.h"\
	".\Core\Brush.h"\
	".\Core\BrushList.h"\
	".\Core\BrushTemplate.h"\
	".\Core\CameraList.h"\
	".\Core\CamObj.h"\
	".\Core\Class.h"\
	".\Core\Descriptor.h"\
	".\Core\Draw.h"\
	".\Core\Draw3d.h"\
	".\Core\Entity.h"\
	".\Core\EntityList.h"\
	".\Core\Group.h"\
	".\Core\GroupList.h"\
	".\Core\jwObject.h"\
	".\Core\Level.h"\
	".\Core\Light.h"\
	".\Core\LightList.h"\
	".\Core\MaterialIdentList.h"\
	".\Core\MaterialList.h"\
	".\Core\MaterialList2.h"\
	".\Core\Materials.h"\
	".\Core\Model.h"\
	".\Core\modellist.h"\
	".\Core\ObjectList.h"\
	".\Core\Ortho.h"\
	".\Core\Select.h"\
	".\Core\TernList.h"\
	".\Core\TerrnObj.h"\
	".\Core\Transform.h"\
	".\Core\Undo.h"\
	".\Core\VertList.h"\
	".\CTextToolBar.h"\
	".\Defs.h"\
	".\Dialogs\buildspin.h"\
	".\Dialogs\DialogBuilderDlg.h"\
	".\Dialogs\DialogHelpers\ColorButton.h"\
	".\Dialogs\DialogHelpers\ColorPopup.h"\
	".\Dialogs\DialogHelpers\DropFileArray.h"\
	".\Dialogs\DialogHelpers\MyBar.h"\
	".\Dialogs\DialogHelpers\MyDropTarget.h"\
	".\Dialogs\DialogHelpers\scbarcf.h"\
	".\Dialogs\DialogHelpers\scbarg.h"\
	".\Dialogs\DialogHelpers\sizecbar.h"\
	".\Dialogs\DialogHelpers\TextureListControl.h"\
	".\Dialogs\DialogHelpers\TextureTreeControl.h"\
	".\Dialogs\DlgAutorecover.h"\
	".\Dialogs\Empty.h"\
	".\Dialogs\globalmaterials.h"\
	".\Dialogs\Groups.h"\
	".\Dialogs\Lists.h"\
	".\Dialogs\Models.h"\
	".\Dialogs\Preferences.h"\
	".\Dialogs\Properties.h"\
	".\Dialogs\Rebuild.h"\
	".\Dialogs\ReportErr.h"\
	".\Dialogs\TexturesDlg.h"\
	".\Dialogs\Timeline.h"\
	".\Doc.h"\
	".\DragTree.h"\
	".\ExtFileDialog.h"\
	".\grouptreectrl.h"\
	".\J3dmfc\J3DApp.h"\
	".\J3dmfc\J3DDoc.h"\
	".\J3dmfc\J3DMainFrm.h"\
	".\J3dmfc\J3DView.h"\
	".\jet\Symbol.h"\
	".\jetdialog.h"\
	".\JetView.h"\
	".\Jwe.h"\
	".\label.h"\
	".\MainFrm.h"\
	".\MfcUtil.h"\
	".\MsgLog.h"\
	".\Settings.h"\
	".\StdAfx.h"\
	".\TreeCtrlEx.h"\
	".\Util\DrawTool.h"\
	".\Util\jeList.h"\
	".\Util\Point.h"\
	".\Util\Rect.h"\
	".\Util\Units.h"\
	".\View.h"\
	".\WndReg.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Doc.h
# End Source File
# Begin Source File

SOURCE=.\Docbars.cpp
DEP_CPP_DOCBA=\
	"..\..\..\include\BaseType.h"\
	".\Dialogs\DialogHelpers\scbarcf.h"\
	".\Dialogs\DialogHelpers\scbarg.h"\
	".\Dialogs\DialogHelpers\sizecbar.h"\
	".\Dockbars.h"\
	".\StdAfx.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Dockbars.h
# End Source File
# Begin Source File

SOURCE=.\DocManagerEx.cpp
DEP_CPP_DOCMA=\
	"..\..\..\include\BaseType.h"\
	".\Dialogs\DialogHelpers\scbarcf.h"\
	".\Dialogs\DialogHelpers\scbarg.h"\
	".\Dialogs\DialogHelpers\sizecbar.h"\
	".\DocManagerEx.h"\
	".\ExtFileDialog.h"\
	".\label.h"\
	".\StdAfx.h"\
	".\Util\DrawTool.h"\
	".\Util\Point.h"\
	
# End Source File
# Begin Source File

SOURCE=.\DocManagerEx.h
# End Source File
# Begin Source File

SOURCE=.\DragTree.cpp
DEP_CPP_DRAGT=\
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
	"..\..\..\include\jePlane.h"\
	"..\..\..\include\jePoly.h"\
	"..\..\..\include\jePortal.h"\
	"..\..\..\include\jeProperty.h"\
	"..\..\..\include\jePtrMgr.h"\
	"..\..\..\include\jeResource.h"\
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
	"..\..\..\include\Sound.h"\
	"..\..\..\include\UVMAP.H"\
	"..\..\..\include\Vec3d.h"\
	"..\..\..\include\VFILE.H"\
	"..\..\..\include\VideoMgr.h"\
	"..\..\..\include\Xform3d.h"\
	".\AppData.h"\
	".\Core\MaterialList.h"\
	".\Core\Materials.h"\
	".\Dialogs\DialogHelpers\scbarcf.h"\
	".\Dialogs\DialogHelpers\scbarg.h"\
	".\Dialogs\DialogHelpers\sizecbar.h"\
	".\Dialogs\DlgAutorecover.h"\
	".\DragTree.h"\
	".\J3dmfc\J3DApp.h"\
	".\Jwe.h"\
	".\MsgLog.h"\
	".\Settings.h"\
	".\StdAfx.h"\
	".\Util\jeList.h"\
	".\WndReg.h"\
	
# End Source File
# Begin Source File

SOURCE=.\DragTree.h
# End Source File
# Begin Source File

SOURCE=.\EditMsg.h
# End Source File
# Begin Source File

SOURCE=.\ExtFileDialog.cpp
DEP_CPP_EXTFI=\
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
	"..\..\..\include\jePlane.h"\
	"..\..\..\include\jePoly.h"\
	"..\..\..\include\jePortal.h"\
	"..\..\..\include\jeProperty.h"\
	"..\..\..\include\jePtrMgr.h"\
	"..\..\..\include\jeResource.h"\
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
	"..\..\..\include\Sound.h"\
	"..\..\..\include\UVMAP.H"\
	"..\..\..\include\Vec3d.h"\
	"..\..\..\include\VFILE.H"\
	"..\..\..\include\VideoMgr.h"\
	"..\..\..\include\Xform3d.h"\
	".\AppData.h"\
	".\BtnST.h"\
	".\CIconTabCtrl.h"\
	".\Core\MaterialList.h"\
	".\Core\Materials.h"\
	".\Dialogs\DialogHelpers\scbarcf.h"\
	".\Dialogs\DialogHelpers\scbarg.h"\
	".\Dialogs\DialogHelpers\sizecbar.h"\
	".\Dialogs\DlgAutorecover.h"\
	".\Dialogs\Properties.h"\
	".\ExtFileDialog.h"\
	".\J3dmfc\J3DApp.h"\
	".\Jwe.h"\
	".\label.h"\
	".\MsgLog.h"\
	".\Settings.h"\
	".\StdAfx.h"\
	".\Util\DrawTool.h"\
	".\Util\jeList.h"\
	".\Util\Point.h"\
	".\WndReg.h"\
	
# End Source File
# Begin Source File

SOURCE=.\ExtFileDialog.h
# End Source File
# Begin Source File

SOURCE=.\GroupBar.cpp
DEP_CPP_GROUPB=\
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
	"..\..\..\include\jePlane.h"\
	"..\..\..\include\jePoly.h"\
	"..\..\..\include\jePortal.h"\
	"..\..\..\include\jeProperty.h"\
	"..\..\..\include\jePtrMgr.h"\
	"..\..\..\include\jeResource.h"\
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
	"..\..\..\include\Sound.h"\
	"..\..\..\include\UVMAP.H"\
	"..\..\..\include\Vec3d.h"\
	"..\..\..\include\VFILE.H"\
	"..\..\..\include\VideoMgr.h"\
	"..\..\..\include\Xform3d.h"\
	".\AppData.h"\
	".\Core\MaterialList.h"\
	".\Core\Materials.h"\
	".\Dialogs\DialogHelpers\scbarcf.h"\
	".\Dialogs\DialogHelpers\scbarg.h"\
	".\Dialogs\DialogHelpers\sizecbar.h"\
	".\Dialogs\DlgAutorecover.h"\
	".\GroupBar.h"\
	".\J3dmfc\J3DApp.h"\
	".\Jwe.h"\
	".\MsgLog.h"\
	".\Settings.h"\
	".\StdAfx.h"\
	".\Util\jeList.h"\
	".\WndReg.h"\
	
# End Source File
# Begin Source File

SOURCE=.\GroupBar.h
# End Source File
# Begin Source File

SOURCE=.\grouptreectrl.cpp
DEP_CPP_GROUPT=\
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
	"..\..\..\include\jePlane.h"\
	"..\..\..\include\jePoly.h"\
	"..\..\..\include\jePortal.h"\
	"..\..\..\include\jeProperty.h"\
	"..\..\..\include\jePtrMgr.h"\
	"..\..\..\include\jeResource.h"\
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
	"..\..\..\include\Sound.h"\
	"..\..\..\include\UVMAP.H"\
	"..\..\..\include\Vec3d.h"\
	"..\..\..\include\VFILE.H"\
	"..\..\..\include\VideoMgr.h"\
	"..\..\..\include\Xform3d.h"\
	".\AppData.h"\
	".\Core\MaterialList.h"\
	".\Core\Materials.h"\
	".\Dialogs\DialogHelpers\scbarcf.h"\
	".\Dialogs\DialogHelpers\scbarg.h"\
	".\Dialogs\DialogHelpers\sizecbar.h"\
	".\Dialogs\DlgAutorecover.h"\
	".\grouptreectrl.h"\
	".\J3dmfc\J3DApp.h"\
	".\Jwe.h"\
	".\MsgLog.h"\
	".\Settings.h"\
	".\StdAfx.h"\
	".\TreeCtrlEx.h"\
	".\Util\jeList.h"\
	".\WndReg.h"\
	
# End Source File
# Begin Source File

SOURCE=.\grouptreectrl.h
# End Source File
# Begin Source File

SOURCE=.\jetdialog.cpp
DEP_CPP_JETDI=\
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
	"..\..\..\include\jePlane.h"\
	"..\..\..\include\jePoly.h"\
	"..\..\..\include\jePortal.h"\
	"..\..\..\include\jeProperty.h"\
	"..\..\..\include\jePtrMgr.h"\
	"..\..\..\include\jeResource.h"\
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
	"..\..\..\include\Sound.h"\
	"..\..\..\include\TERRAIN.H"\
	"..\..\..\include\UVMAP.H"\
	"..\..\..\include\Vec3d.h"\
	"..\..\..\include\VFILE.H"\
	"..\..\..\include\VideoMgr.h"\
	"..\..\..\include\Xform3d.h"\
	".\AppData.h"\
	".\BtnST.h"\
	".\CIconTabCtrl.h"\
	".\Core\Brush.h"\
	".\Core\BrushList.h"\
	".\Core\BrushTemplate.h"\
	".\Core\CameraList.h"\
	".\Core\CamObj.h"\
	".\Core\Class.h"\
	".\Core\Descriptor.h"\
	".\Core\Entity.h"\
	".\Core\EntityList.h"\
	".\Core\Group.h"\
	".\Core\GroupList.h"\
	".\Core\jwObject.h"\
	".\Core\Level.h"\
	".\Core\Light.h"\
	".\Core\LightList.h"\
	".\Core\MaterialIdentList.h"\
	".\Core\MaterialList.h"\
	".\Core\MaterialList2.h"\
	".\Core\Materials.h"\
	".\Core\Model.h"\
	".\Core\modellist.h"\
	".\Core\ObjectList.h"\
	".\Core\Ortho.h"\
	".\Core\Select.h"\
	".\Core\TernList.h"\
	".\Core\TerrnObj.h"\
	".\Core\Undo.h"\
	".\Core\VertList.h"\
	".\CTextToolBar.h"\
	".\Defs.h"\
	".\Dialogs\buildspin.h"\
	".\Dialogs\DialogBuilderDlg.h"\
	".\Dialogs\DialogHelpers\ColorButton.h"\
	".\Dialogs\DialogHelpers\ColorPopup.h"\
	".\Dialogs\DialogHelpers\DropFileArray.h"\
	".\Dialogs\DialogHelpers\MyBar.h"\
	".\Dialogs\DialogHelpers\MyDropTarget.h"\
	".\Dialogs\DialogHelpers\scbarcf.h"\
	".\Dialogs\DialogHelpers\scbarg.h"\
	".\Dialogs\DialogHelpers\sizecbar.h"\
	".\Dialogs\DialogHelpers\TextureListControl.h"\
	".\Dialogs\DialogHelpers\TextureTreeControl.h"\
	".\Dialogs\DlgAutorecover.h"\
	".\Dialogs\Empty.h"\
	".\Dialogs\globalmaterials.h"\
	".\Dialogs\Groups.h"\
	".\Dialogs\Lists.h"\
	".\Dialogs\Models.h"\
	".\Dialogs\Properties.h"\
	".\Dialogs\TexturesDlg.h"\
	".\Dialogs\Timeline.h"\
	".\Doc.h"\
	".\DragTree.h"\
	".\grouptreectrl.h"\
	".\J3dmfc\J3DApp.h"\
	".\J3dmfc\J3DDoc.h"\
	".\J3dmfc\J3DMainFrm.h"\
	".\jet\Symbol.h"\
	".\jetdialog.h"\
	".\Jwe.h"\
	".\label.h"\
	".\MainFrm.h"\
	".\MsgLog.h"\
	".\Settings.h"\
	".\StdAfx.h"\
	".\TreeCtrlEx.h"\
	".\Util\jeList.h"\
	".\Util\Point.h"\
	".\Util\Rect.h"\
	".\WndReg.h"\
	
# End Source File
# Begin Source File

SOURCE=.\jetdialog.h
# End Source File
# Begin Source File

SOURCE=.\JetView.cpp
DEP_CPP_JETVI=\
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
	"..\..\..\include\jePlane.h"\
	"..\..\..\include\jePoly.h"\
	"..\..\..\include\jePortal.h"\
	"..\..\..\include\jeProperty.h"\
	"..\..\..\include\jePtrMgr.h"\
	"..\..\..\include\jeResource.h"\
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
	"..\..\..\include\Sound.h"\
	"..\..\..\include\TERRAIN.H"\
	"..\..\..\include\UVMAP.H"\
	"..\..\..\include\Vec3d.h"\
	"..\..\..\include\VFILE.H"\
	"..\..\..\include\VideoMgr.h"\
	"..\..\..\include\Xform3d.h"\
	".\AppData.h"\
	".\BtnST.h"\
	".\CIconTabCtrl.h"\
	".\Core\Brush.h"\
	".\Core\BrushList.h"\
	".\Core\BrushTemplate.h"\
	".\Core\CameraList.h"\
	".\Core\CamObj.h"\
	".\Core\Class.h"\
	".\Core\Descriptor.h"\
	".\Core\Entity.h"\
	".\Core\EntityList.h"\
	".\Core\Group.h"\
	".\Core\GroupList.h"\
	".\Core\jwObject.h"\
	".\Core\Level.h"\
	".\Core\Light.h"\
	".\Core\LightList.h"\
	".\Core\MaterialIdentList.h"\
	".\Core\MaterialList.h"\
	".\Core\Materials.h"\
	".\Core\Model.h"\
	".\Core\modellist.h"\
	".\Core\ObjectList.h"\
	".\Core\Ortho.h"\
	".\Core\Select.h"\
	".\Core\TernList.h"\
	".\Core\TerrnObj.h"\
	".\Core\Undo.h"\
	".\Core\VertList.h"\
	".\Defs.h"\
	".\Dialogs\DialogHelpers\scbarcf.h"\
	".\Dialogs\DialogHelpers\scbarg.h"\
	".\Dialogs\DialogHelpers\sizecbar.h"\
	".\Dialogs\DlgAutorecover.h"\
	".\Dialogs\Properties.h"\
	".\Doc.h"\
	".\J3dmfc\J3DApp.h"\
	".\J3dmfc\J3DDoc.h"\
	".\J3dmfc\J3DView.h"\
	".\jet\Symbol.h"\
	".\JetView.h"\
	".\Jwe.h"\
	".\label.h"\
	".\MsgLog.h"\
	".\Settings.h"\
	".\StdAfx.h"\
	".\Util\Drvlist.h"\
	".\Util\jeList.h"\
	".\Util\Point.h"\
	".\Util\Rect.h"\
	".\WndReg.h"\
	
# End Source File
# Begin Source File

SOURCE=.\JetView.h
# End Source File
# Begin Source File

SOURCE=.\Jwe.cpp
DEP_CPP_JWE_C=\
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
	"..\..\..\include\jePlane.h"\
	"..\..\..\include\jePoly.h"\
	"..\..\..\include\jePortal.h"\
	"..\..\..\include\jeProperty.h"\
	"..\..\..\include\jePtrMgr.h"\
	"..\..\..\include\jeResource.h"\
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
	"..\..\..\include\Sound.h"\
	"..\..\..\include\TERRAIN.H"\
	"..\..\..\include\UVMAP.H"\
	"..\..\..\include\Vec3d.h"\
	"..\..\..\include\VFILE.H"\
	"..\..\..\include\VideoMgr.h"\
	"..\..\..\include\Xform3d.h"\
	".\AppData.h"\
	".\BtnST.h"\
	".\BuildNumber.h"\
	".\BuildType.h"\
	".\ChildFrm.h"\
	".\CIconTabCtrl.h"\
	".\Core\Brush.h"\
	".\Core\BrushList.h"\
	".\Core\BrushTemplate.h"\
	".\Core\CameraList.h"\
	".\Core\CamObj.h"\
	".\Core\Class.h"\
	".\Core\Descriptor.h"\
	".\Core\Entity.h"\
	".\Core\EntityList.h"\
	".\Core\Group.h"\
	".\Core\GroupList.h"\
	".\Core\jwObject.h"\
	".\Core\Level.h"\
	".\Core\Light.h"\
	".\Core\LightList.h"\
	".\Core\MaterialIdentList.h"\
	".\Core\MaterialList.h"\
	".\Core\MaterialList2.h"\
	".\Core\Materials.h"\
	".\Core\Model.h"\
	".\Core\modellist.h"\
	".\Core\ObjectList.h"\
	".\Core\Ortho.h"\
	".\Core\Select.h"\
	".\Core\TernList.h"\
	".\Core\TerrnObj.h"\
	".\Core\Undo.h"\
	".\Core\VertList.h"\
	".\CTextToolBar.h"\
	".\Defs.h"\
	".\Dialogs\buildspin.h"\
	".\Dialogs\DialogBuilderDlg.h"\
	".\Dialogs\DialogHelpers\ColorButton.h"\
	".\Dialogs\DialogHelpers\ColorPopup.h"\
	".\Dialogs\DialogHelpers\DropFileArray.h"\
	".\Dialogs\DialogHelpers\MyBar.h"\
	".\Dialogs\DialogHelpers\MyDropTarget.h"\
	".\Dialogs\DialogHelpers\scbarcf.h"\
	".\Dialogs\DialogHelpers\scbarg.h"\
	".\Dialogs\DialogHelpers\sizecbar.h"\
	".\Dialogs\DialogHelpers\TextureListControl.h"\
	".\Dialogs\DialogHelpers\TextureTreeControl.h"\
	".\Dialogs\disclaimer.h"\
	".\Dialogs\DlgAutorecover.h"\
	".\Dialogs\Empty.h"\
	".\Dialogs\globalmaterials.h"\
	".\Dialogs\Groups.h"\
	".\Dialogs\Lists.h"\
	".\Dialogs\Models.h"\
	".\Dialogs\Properties.h"\
	".\Dialogs\ReportErr.h"\
	".\Dialogs\TexturesDlg.h"\
	".\Dialogs\Timeline.h"\
	".\Doc.h"\
	".\DocManagerEx.h"\
	".\DragTree.h"\
	".\grouptreectrl.h"\
	".\J3dmfc\J3DApp.h"\
	".\J3dmfc\J3DDoc.h"\
	".\J3dmfc\J3DMainFrm.h"\
	".\jet\Symbol.h"\
	".\jetdialog.h"\
	".\Jwe.h"\
	".\label.h"\
	".\MainFrm.h"\
	".\MsgLog.h"\
	".\Settings.h"\
	".\StdAfx.h"\
	".\TreeCtrlEx.h"\
	".\Util\Drvlist.h"\
	".\Util\jeList.h"\
	".\Util\Point.h"\
	".\Util\Rect.h"\
	".\Util\Spawn.h"\
	".\View.h"\
	".\WndReg.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Jwe.h
# End Source File
# Begin Source File

SOURCE=.\Jwe.hlp
# End Source File
# Begin Source File

SOURCE=.\jwe.rc
# End Source File
# Begin Source File

SOURCE=.\label.cpp
DEP_CPP_LABEL=\
	"..\..\..\include\BaseType.h"\
	".\Dialogs\DialogHelpers\scbarcf.h"\
	".\Dialogs\DialogHelpers\scbarg.h"\
	".\Dialogs\DialogHelpers\sizecbar.h"\
	".\label.h"\
	".\StdAfx.h"\
	
# End Source File
# Begin Source File

SOURCE=.\label.h
# End Source File
# Begin Source File

SOURCE=.\MainFrm.cpp
DEP_CPP_MAINF=\
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
	"..\..\..\include\jePlane.h"\
	"..\..\..\include\jePoly.h"\
	"..\..\..\include\jePortal.h"\
	"..\..\..\include\jeProperty.h"\
	"..\..\..\include\jePtrMgr.h"\
	"..\..\..\include\jeResource.h"\
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
	"..\..\..\include\Sound.h"\
	"..\..\..\include\TERRAIN.H"\
	"..\..\..\include\UVMAP.H"\
	"..\..\..\include\Vec3d.h"\
	"..\..\..\include\VFILE.H"\
	"..\..\..\include\VideoMgr.h"\
	"..\..\..\include\Xform3d.h"\
	".\AppData.h"\
	".\BtnST.h"\
	".\CIconTabCtrl.h"\
	".\Core\Brush.h"\
	".\Core\BrushList.h"\
	".\Core\BrushTemplate.h"\
	".\Core\CameraList.h"\
	".\Core\CamObj.h"\
	".\Core\Class.h"\
	".\Core\Descriptor.h"\
	".\Core\Entity.h"\
	".\Core\EntityList.h"\
	".\Core\Group.h"\
	".\Core\GroupList.h"\
	".\Core\jwObject.h"\
	".\Core\Level.h"\
	".\Core\Light.h"\
	".\Core\LightList.h"\
	".\Core\MaterialIdentList.h"\
	".\Core\MaterialList.h"\
	".\Core\MaterialList2.h"\
	".\Core\Materials.h"\
	".\Core\Model.h"\
	".\Core\modellist.h"\
	".\Core\ObjectList.h"\
	".\Core\Ortho.h"\
	".\Core\Select.h"\
	".\Core\TernList.h"\
	".\Core\TerrnObj.h"\
	".\Core\Undo.h"\
	".\Core\VertList.h"\
	".\CTextToolBar.h"\
	".\Defs.h"\
	".\Dialogs\buildspin.h"\
	".\Dialogs\DialogBuilderDlg.h"\
	".\Dialogs\DialogHelpers\ColorButton.h"\
	".\Dialogs\DialogHelpers\ColorPopup.h"\
	".\Dialogs\DialogHelpers\DropFileArray.h"\
	".\Dialogs\DialogHelpers\MyBar.h"\
	".\Dialogs\DialogHelpers\MyDropTarget.h"\
	".\Dialogs\DialogHelpers\scbarcf.h"\
	".\Dialogs\DialogHelpers\scbarg.h"\
	".\Dialogs\DialogHelpers\sizecbar.h"\
	".\Dialogs\DialogHelpers\TextureListControl.h"\
	".\Dialogs\DialogHelpers\TextureTreeControl.h"\
	".\Dialogs\DlgAutorecover.h"\
	".\Dialogs\Empty.h"\
	".\Dialogs\globalmaterials.h"\
	".\Dialogs\Groups.h"\
	".\Dialogs\Lists.h"\
	".\Dialogs\Models.h"\
	".\Dialogs\Properties.h"\
	".\Dialogs\TexturesDlg.h"\
	".\Dialogs\Timeline.h"\
	".\Doc.h"\
	".\DragTree.h"\
	".\grouptreectrl.h"\
	".\J3dmfc\J3DApp.h"\
	".\J3dmfc\J3DDoc.h"\
	".\J3dmfc\J3DMainFrm.h"\
	".\jet\Symbol.h"\
	".\jetdialog.h"\
	".\Jwe.h"\
	".\label.h"\
	".\MainFrm.h"\
	".\MsgLog.h"\
	".\Settings.h"\
	".\StdAfx.h"\
	".\TreeCtrlEx.h"\
	".\Util\jeList.h"\
	".\Util\Point.h"\
	".\Util\Rect.h"\
	".\View.h"\
	".\WndReg.h"\
	
# End Source File
# Begin Source File

SOURCE=.\MainFrm.h
# End Source File
# Begin Source File

SOURCE=.\MfcUtil.cpp
DEP_CPP_MFCUT=\
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
	"..\..\..\include\jePlane.h"\
	"..\..\..\include\jePoly.h"\
	"..\..\..\include\jePortal.h"\
	"..\..\..\include\jeProperty.h"\
	"..\..\..\include\jePtrMgr.h"\
	"..\..\..\include\jeResource.h"\
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
	"..\..\..\include\Sound.h"\
	"..\..\..\include\UVMAP.H"\
	"..\..\..\include\Vec3d.h"\
	"..\..\..\include\VFILE.H"\
	"..\..\..\include\VideoMgr.h"\
	"..\..\..\include\Xform3d.h"\
	".\Dialogs\DialogHelpers\scbarcf.h"\
	".\Dialogs\DialogHelpers\scbarg.h"\
	".\Dialogs\DialogHelpers\sizecbar.h"\
	".\MfcUtil.h"\
	".\StdAfx.h"\
	
# End Source File
# Begin Source File

SOURCE=.\MfcUtil.h
# End Source File
# Begin Source File

SOURCE=.\MsgLog.cpp
DEP_CPP_MSGLO=\
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
	"..\..\..\include\jePlane.h"\
	"..\..\..\include\jePoly.h"\
	"..\..\..\include\jePortal.h"\
	"..\..\..\include\jeProperty.h"\
	"..\..\..\include\jePtrMgr.h"\
	"..\..\..\include\jeResource.h"\
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
	"..\..\..\include\Sound.h"\
	"..\..\..\include\UVMAP.H"\
	"..\..\..\include\Vec3d.h"\
	"..\..\..\include\VFILE.H"\
	"..\..\..\include\VideoMgr.h"\
	"..\..\..\include\Xform3d.h"\
	".\AppData.h"\
	".\Core\MaterialList.h"\
	".\Core\Materials.h"\
	".\Dialogs\DialogHelpers\scbarcf.h"\
	".\Dialogs\DialogHelpers\scbarg.h"\
	".\Dialogs\DialogHelpers\sizecbar.h"\
	".\Dialogs\DlgAutorecover.h"\
	".\J3dmfc\J3DApp.h"\
	".\Jwe.h"\
	".\MsgLog.h"\
	".\Settings.h"\
	".\StdAfx.h"\
	".\Util\jeList.h"\
	".\WndReg.h"\
	
# End Source File
# Begin Source File

SOURCE=.\MsgLog.h
# End Source File
# Begin Source File

SOURCE=.\MyStatic.cpp
DEP_CPP_MYSTA=\
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
	"..\..\..\include\jePlane.h"\
	"..\..\..\include\jePoly.h"\
	"..\..\..\include\jePortal.h"\
	"..\..\..\include\jeProperty.h"\
	"..\..\..\include\jePtrMgr.h"\
	"..\..\..\include\jeResource.h"\
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
	"..\..\..\include\Sound.h"\
	"..\..\..\include\UVMAP.H"\
	"..\..\..\include\Vec3d.h"\
	"..\..\..\include\VFILE.H"\
	"..\..\..\include\VideoMgr.h"\
	"..\..\..\include\Xform3d.h"\
	".\AppData.h"\
	".\Core\MaterialList.h"\
	".\Core\Materials.h"\
	".\Dialogs\DialogHelpers\scbarcf.h"\
	".\Dialogs\DialogHelpers\scbarg.h"\
	".\Dialogs\DialogHelpers\sizecbar.h"\
	".\Dialogs\DlgAutorecover.h"\
	".\J3dmfc\J3DApp.h"\
	".\Jwe.h"\
	".\MsgLog.h"\
	".\MyStatic.h"\
	".\Settings.h"\
	".\StdAfx.h"\
	".\Util\Bmp.h"\
	".\Util\jeList.h"\
	".\WndReg.h"\
	
# End Source File
# Begin Source File

SOURCE=.\MyStatic.h
# End Source File
# Begin Source File

SOURCE=.\ObjectMsg.h
# End Source File
# Begin Source File

SOURCE=.\resource.h
# End Source File
# Begin Source File

SOURCE=.\Settings.cpp
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
	"..\..\..\include\jePlane.h"\
	"..\..\..\include\jePoly.h"\
	"..\..\..\include\jePortal.h"\
	"..\..\..\include\jeProperty.h"\
	"..\..\..\include\jePtrMgr.h"\
	"..\..\..\include\jeResource.h"\
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
	"..\..\..\include\Sound.h"\
	"..\..\..\include\UVMAP.H"\
	"..\..\..\include\Vec3d.h"\
	"..\..\..\include\VFILE.H"\
	"..\..\..\include\VideoMgr.h"\
	"..\..\..\include\Xform3d.h"\
	".\AppData.h"\
	".\Core\MaterialList.h"\
	".\Core\Materials.h"\
	".\Dialogs\DialogHelpers\scbarcf.h"\
	".\Dialogs\DialogHelpers\scbarg.h"\
	".\Dialogs\DialogHelpers\sizecbar.h"\
	".\Dialogs\DlgAutorecover.h"\
	".\J3dmfc\J3DApp.h"\
	".\Jwe.h"\
	".\MsgLog.h"\
	".\Settings.h"\
	".\StdAfx.h"\
	".\Util\jeList.h"\
	".\WndReg.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Settings.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
DEP_CPP_STDAF=\
	"..\..\..\include\BaseType.h"\
	".\Dialogs\DialogHelpers\scbarcf.h"\
	".\Dialogs\DialogHelpers\scbarg.h"\
	".\Dialogs\DialogHelpers\sizecbar.h"\
	".\StdAfx.h"\
	
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# Begin Source File

SOURCE=.\TreeCtrlEx.cpp
DEP_CPP_TREEC=\
	"..\..\..\include\BaseType.h"\
	".\Dialogs\DialogHelpers\scbarcf.h"\
	".\Dialogs\DialogHelpers\scbarg.h"\
	".\Dialogs\DialogHelpers\sizecbar.h"\
	".\StdAfx.h"\
	".\TreeCtrlEx.h"\
	
# End Source File
# Begin Source File

SOURCE=.\TreeCtrlEx.h
# End Source File
# Begin Source File

SOURCE=.\View.cpp
DEP_CPP_VIEW_=\
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
	"..\..\..\include\jePlane.h"\
	"..\..\..\include\jePoly.h"\
	"..\..\..\include\jePortal.h"\
	"..\..\..\include\jeProperty.h"\
	"..\..\..\include\jePtrMgr.h"\
	"..\..\..\include\jeResource.h"\
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
	"..\..\..\include\Sound.h"\
	"..\..\..\include\TERRAIN.H"\
	"..\..\..\include\UVMAP.H"\
	"..\..\..\include\Vec3d.h"\
	"..\..\..\include\VFILE.H"\
	"..\..\..\include\VideoMgr.h"\
	"..\..\..\include\Xform3d.h"\
	".\AppData.h"\
	".\BtnST.h"\
	".\CIconTabCtrl.h"\
	".\Core\Brush.h"\
	".\Core\BrushList.h"\
	".\Core\BrushTemplate.h"\
	".\Core\CameraList.h"\
	".\Core\CamObj.h"\
	".\Core\Class.h"\
	".\Core\Descriptor.h"\
	".\Core\Entity.h"\
	".\Core\EntityList.h"\
	".\Core\Group.h"\
	".\Core\GroupList.h"\
	".\Core\jwObject.h"\
	".\Core\Level.h"\
	".\Core\Light.h"\
	".\Core\LightList.h"\
	".\Core\MaterialIdentList.h"\
	".\Core\MaterialList.h"\
	".\Core\MaterialList2.h"\
	".\Core\Materials.h"\
	".\Core\Model.h"\
	".\Core\modellist.h"\
	".\Core\ObjectList.h"\
	".\Core\Ortho.h"\
	".\Core\Select.h"\
	".\Core\TernList.h"\
	".\Core\TerrnObj.h"\
	".\Core\Undo.h"\
	".\Core\VertList.h"\
	".\CTextToolBar.h"\
	".\Defs.h"\
	".\Dialogs\buildspin.h"\
	".\Dialogs\DialogBuilderDlg.h"\
	".\Dialogs\DialogHelpers\ColorButton.h"\
	".\Dialogs\DialogHelpers\ColorPopup.h"\
	".\Dialogs\DialogHelpers\DropFileArray.h"\
	".\Dialogs\DialogHelpers\MyBar.h"\
	".\Dialogs\DialogHelpers\MyDropTarget.h"\
	".\Dialogs\DialogHelpers\scbarcf.h"\
	".\Dialogs\DialogHelpers\scbarg.h"\
	".\Dialogs\DialogHelpers\sizecbar.h"\
	".\Dialogs\DialogHelpers\TextureListControl.h"\
	".\Dialogs\DialogHelpers\TextureTreeControl.h"\
	".\Dialogs\DlgAutorecover.h"\
	".\Dialogs\Empty.h"\
	".\Dialogs\globalmaterials.h"\
	".\Dialogs\Groups.h"\
	".\Dialogs\Lists.h"\
	".\Dialogs\Models.h"\
	".\Dialogs\Properties.h"\
	".\Dialogs\TexturesDlg.h"\
	".\Dialogs\Timeline.h"\
	".\Doc.h"\
	".\DragTree.h"\
	".\grouptreectrl.h"\
	".\J3dmfc\J3DApp.h"\
	".\J3dmfc\J3DDoc.h"\
	".\J3dmfc\J3DMainFrm.h"\
	".\jet\Symbol.h"\
	".\jetdialog.h"\
	".\Jwe.h"\
	".\label.h"\
	".\MainFrm.h"\
	".\Memdc.h"\
	".\MsgLog.h"\
	".\Settings.h"\
	".\StdAfx.h"\
	".\TreeCtrlEx.h"\
	".\Util\jeList.h"\
	".\Util\Point.h"\
	".\Util\Rect.h"\
	".\View.h"\
	".\WndReg.h"\
	
# End Source File
# Begin Source File

SOURCE=.\View.h
# End Source File
# Begin Source File

SOURCE=.\WndReg.cpp
DEP_CPP_WNDRE=\
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
	"..\..\..\include\jePlane.h"\
	"..\..\..\include\jePoly.h"\
	"..\..\..\include\jePortal.h"\
	"..\..\..\include\jeProperty.h"\
	"..\..\..\include\jePtrMgr.h"\
	"..\..\..\include\jeResource.h"\
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
	"..\..\..\include\Sound.h"\
	"..\..\..\include\UVMAP.H"\
	"..\..\..\include\Vec3d.h"\
	"..\..\..\include\VFILE.H"\
	"..\..\..\include\VideoMgr.h"\
	"..\..\..\include\Xform3d.h"\
	".\AppData.h"\
	".\Core\MaterialList.h"\
	".\Core\Materials.h"\
	".\Dialogs\DialogHelpers\scbarcf.h"\
	".\Dialogs\DialogHelpers\scbarg.h"\
	".\Dialogs\DialogHelpers\sizecbar.h"\
	".\Dialogs\DlgAutorecover.h"\
	".\J3dmfc\J3DApp.h"\
	".\Jwe.h"\
	".\MsgLog.h"\
	".\Settings.h"\
	".\StdAfx.h"\
	".\Util\jeList.h"\
	".\WndReg.h"\
	
# End Source File
# Begin Source File

SOURCE=.\WndReg.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\res\Jet3D.manifest
# End Source File
# End Target
# End Project
