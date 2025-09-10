# Microsoft Developer Studio Project File - Name="JetEngine" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=JetEngine - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "Jet3D.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Jet3D.mak" CFG="JetEngine - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "JetEngine - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "JetEngine - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/JetEngine", VBRBAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "JetEngine - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "..\..\..\bin"
# PROP BASE Intermediate_Dir ".\Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\..\..\bin"
# PROP Intermediate_Dir ".\Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /GX /Z7 /Od /I ".\\" /I "..\..\..\include" /I "..\..\Tools\Editor" /I "World" /I "Engine" /I "Particle" /I "Engine\Drivers" /I "Actor" /I "Pool" /I "BSP" /I "Math" /I "Entities" /I "Support" /I "Physics" /I "VFile" /I "Bitmap" /I "Bitmap\Compression" /I "guWorld" /I "Mp3Mgr" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_USRDLL" /D "BUILDJET" /D "JETDLLVERSION" /D "JET_VERSION_2" /D "_MBCS" /FR /YX /GZ /c
# ADD CPP /nologo /MDd /W3 /GX /Zi /Od /I ".\\" /I "..\..\..\include" /I "..\..\Tools\Editor" /I "World" /I "Engine" /I "Particle" /I "Engine\Drivers" /I "Actor" /I "Pool" /I "BSP" /I "Math" /I "Entities" /I "Support" /I "Physics" /I "VFile" /I "Bitmap" /I "Bitmap\Compression" /I "guWorld" /I "Mp3Mgr" /I "..\External\VorbisSDK\include" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_USRDLL" /D "BUILDJET" /D "JETDLLVERSION" /D "JET_VERSION_2" /D "_MBCS" /D "JETENGINE_EXPORTS" /FR /GZ /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib amstrmid.lib oldnames.lib urlmon.lib winmm.lib quartz.lib dxguid.lib ddraw.lib /nologo /subsystem:windows /dll /pdb:".\Debug\Jet3Dd.pdb" /map /debug /machine:I386 /out:"..\..\..\bin\Jet3Dd.dll" /implib:"../../../lib/Jet3Dd.lib" /pdbtype:sept /libpath:"..\..\..\lib"
# SUBTRACT BASE LINK32 /pdb:none
# ADD LINK32 ogg_static_d.lib vorbis_static_d.lib vorbisfile_static_d.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib amstrmid.lib oldnames.lib urlmon.lib winmm.lib quartz.lib dxguid.lib ddraw.lib /nologo /subsystem:windows /dll /pdb:".\Debug\Jet3Dd.pdb" /debug /machine:I386 /out:"..\..\..\bin\Jet3Dd.dll" /implib:"../../../lib/Jet3Dd.lib" /pdbtype:sept /libpath:"..\..\..\lib" /libpath:"..\External\VorbisSDK\lib"
# SUBTRACT LINK32 /pdb:none /map

!ELSEIF  "$(CFG)" == "JetEngine - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "..\..\..\bin"
# PROP BASE Intermediate_Dir ".\Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "../../../lib"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /Ob1 /Gy /I ".\\" /I "..\..\..\include" /I "..\..\Tools\Editor" /I "World" /I "Engine" /I "Particle" /I "Engine\Drivers" /I "Actor" /I "Pool" /I "BSP" /I "Math" /I "Entities" /I "Support" /I "Physics" /I "VFile" /I "Bitmap" /I "Bitmap\Compression" /I "guWorld" /I "Mp3Mgr" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_USRDLL" /D "BUILDJET" /D "JETDLLVERSION" /D "JET_VERSION_2" /D "_MBCS" /FR /YX /GF /c
# ADD CPP /nologo /MD /W3 /GX /Ob1 /Gy /I ".\\" /I "..\..\..\include" /I "..\..\Tools\Editor" /I "World" /I "Engine" /I "Particle" /I "Engine\Drivers" /I "Actor" /I "Pool" /I "BSP" /I "Math" /I "Entities" /I "Support" /I "Physics" /I "VFile" /I "Bitmap" /I "Bitmap\Compression" /I "guWorld" /I "Mp3Mgr" /I "..\External\VorbisSDK\include" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_USRDLL" /D "BUILDJET" /D "JETDLLVERSION" /D "JET_VERSION_2" /D "_MBCS" /D "JETENGINE_EXPORTS" /FR /YX /GF /c
# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib amstrmid.lib oldnames.lib urlmon.lib winmm.lib quartz.lib dxguid.lib ddraw.lib /nologo /subsystem:windows /dll /pdb:".\Release\Jet3D.pdb" /map /machine:I386 /implib:"..\..\..\lib\Jet3D.lib" /pdbtype:sept /libpath:"..\..\..\lib"
# SUBTRACT BASE LINK32 /pdb:none
# ADD LINK32 ogg_static.lib vorbis_static.lib vorbisfile_static.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib amstrmid.lib oldnames.lib urlmon.lib winmm.lib quartz.lib dxguid.lib ddraw.lib /nologo /subsystem:windows /dll /pdb:".\Release\Jet3D.pdb" /map /machine:I386 /out:"../../../bin/Jet3D.dll" /pdbtype:sept /libpath:"..\..\..\lib" /libpath:"..\External\VorbisSDK\lib"
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "JetEngine - Win32 Debug"
# Name "JetEngine - Win32 Release"
# Begin Group "Source Files"

# PROP Default_Filter ""
# Begin Group "Actor"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Actor\Actor._h
# End Source File
# Begin Source File

SOURCE=.\Actor\actor.c
DEP_CPP_ACTOR=\
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
	".\Actor\Actor._h"\
	".\Actor\ActorObj.h"\
	".\Actor\motion.h"\
	".\Actor\pose.h"\
	".\Actor\puppet.h"\
	".\Actor\strblock.h"\
	".\Actor\xfarray.h"\
	".\Support\log.h"\
	".\UVMap.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\..\include\ACTOR.H
# End Source File
# Begin Source File

SOURCE=.\Actor\ActorObj.c
DEP_CPP_ACTORO=\
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
	".\Actor\Actor._h"\
	".\Actor\ActorObj.h"\
	".\Actor\ActorObjLists.h"\
	".\Actor\ActorPropertyList.h"\
	".\Actor\ActorUtil.h"\
	".\Actor\motion.h"\
	".\Actor\pose.h"\
	".\Actor\puppet.h"\
	".\Actor\strblock.h"\
	".\Actor\xfarray.h"\
	".\Bitmap\Compression\image.h"\
	".\Bitmap\Compression\utility.h"\
	".\Support\log.h"\
	".\UVMap.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Actor\ActorObj.h
# End Source File
# Begin Source File

SOURCE=.\Actor\ActorObjLists.h
# End Source File
# Begin Source File

SOURCE=.\Actor\ActorPropertyList.h
# End Source File
# Begin Source File

SOURCE=.\Actor\ActorUtil.h
# End Source File
# Begin Source File

SOURCE=.\Actor\BODY._H
# End Source File
# Begin Source File

SOURCE=.\Actor\body.c
DEP_CPP_BODY_=\
	"..\..\..\include\ARRAY.H"\
	"..\..\..\include\BaseType.h"\
	"..\..\..\include\BITMAP.H"\
	"..\..\..\include\BODY.H"\
	"..\..\..\include\Camera.h"\
	"..\..\..\include\ENGINE.H"\
	"..\..\..\include\Errorlog.h"\
	"..\..\..\include\jeMaterial.h"\
	"..\..\..\include\jeMemAllocInfo.h"\
	"..\..\..\include\jePtrMgr.h"\
	"..\..\..\include\jeResource.h"\
	"..\..\..\include\jeTypes.h"\
	"..\..\..\include\PixelFormat.h"\
	"..\..\..\include\Ram.h"\
	"..\..\..\include\UVMAP.H"\
	"..\..\..\include\Vec3d.h"\
	"..\..\..\include\VFILE.H"\
	"..\..\..\include\Xform3d.h"\
	".\Actor\BODY._H"\
	".\Actor\strblock.h"\
	".\Support\log.h"\
	".\UVMap.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\..\include\BODY.H
# End Source File
# Begin Source File

SOURCE=.\Actor\bodyinst.c
DEP_CPP_BODYI=\
	"..\..\..\include\ARRAY.H"\
	"..\..\..\include\BaseType.h"\
	"..\..\..\include\BITMAP.H"\
	"..\..\..\include\BODY.H"\
	"..\..\..\include\Camera.h"\
	"..\..\..\include\ENGINE.H"\
	"..\..\..\include\Errorlog.h"\
	"..\..\..\include\jeMaterial.h"\
	"..\..\..\include\jeMemAllocInfo.h"\
	"..\..\..\include\jePtrMgr.h"\
	"..\..\..\include\jeTypes.h"\
	"..\..\..\include\PixelFormat.h"\
	"..\..\..\include\Ram.h"\
	"..\..\..\include\UVMAP.H"\
	"..\..\..\include\Vec3d.h"\
	"..\..\..\include\VFILE.H"\
	"..\..\..\include\Xform3d.h"\
	".\Actor\BODY._H"\
	".\Actor\bodyinst.h"\
	".\Actor\strblock.h"\
	".\Actor\xfarray.h"\
	".\Camera._h"\
	".\list.h"\
	".\UVMap.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Actor\bodyinst.h
# End Source File
# Begin Source File

SOURCE=.\Actor\motion.c
DEP_CPP_MOTIO=\
	"..\..\..\include\BaseType.h"\
	"..\..\..\include\Errorlog.h"\
	"..\..\..\include\jeMemAllocInfo.h"\
	"..\..\..\include\PATH.H"\
	"..\..\..\include\QUATERN.H"\
	"..\..\..\include\Ram.h"\
	"..\..\..\include\Vec3d.h"\
	"..\..\..\include\VFILE.H"\
	"..\..\..\include\Xform3d.h"\
	".\Actor\motion.h"\
	".\Actor\strblock.h"\
	".\Actor\tkevents.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Actor\motion.h
# End Source File
# Begin Source File

SOURCE=.\Actor\path.c
DEP_CPP_PATH_=\
	"..\..\..\include\BaseType.h"\
	"..\..\..\include\Errorlog.h"\
	"..\..\..\include\jeMemAllocInfo.h"\
	"..\..\..\include\PATH.H"\
	"..\..\..\include\QUATERN.H"\
	"..\..\..\include\Ram.h"\
	"..\..\..\include\Vec3d.h"\
	"..\..\..\include\VFILE.H"\
	"..\..\..\include\Xform3d.h"\
	".\Actor\QKFrame.h"\
	".\Actor\tkarray.h"\
	".\Actor\vkframe.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\..\include\PATH.H
# End Source File
# Begin Source File

SOURCE=.\Actor\pose.c
DEP_CPP_POSE_=\
	"..\..\..\include\BaseType.h"\
	"..\..\..\include\Errorlog.h"\
	"..\..\..\include\jeMemAllocInfo.h"\
	"..\..\..\include\PATH.H"\
	"..\..\..\include\QUATERN.H"\
	"..\..\..\include\Ram.h"\
	"..\..\..\include\Vec3d.h"\
	"..\..\..\include\VFILE.H"\
	"..\..\..\include\Xform3d.h"\
	".\Actor\motion.h"\
	".\Actor\pose.h"\
	".\Actor\strblock.h"\
	".\Actor\xfarray.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Actor\pose.h
# End Source File
# Begin Source File

SOURCE=.\Actor\puppet.c
DEP_CPP_PUPPE=\
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
	"..\..\..\include\jeStaticMesh.h"\
	"..\..\..\include\jeTypes.h"\
	"..\..\..\include\jeUserPoly.h"\
	"..\..\..\include\jeVertArray.h"\
	"..\..\..\include\JEWORLD.H"\
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
	".\Actor\bodyinst.h"\
	".\Actor\motion.h"\
	".\Actor\pose.h"\
	".\Actor\puppet.h"\
	".\Actor\xfarray.h"\
	".\Bitmap\bitmap._h"\
	".\Camera._h"\
	".\Engine\BitmapList.h"\
	".\Engine\Drivers\Dcommon.h"\
	".\Engine\engine._h"\
	".\guWorld\jeMaterial._h"\
	".\list.h"\
	".\tclip.h"\
	".\UVMap.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Actor\puppet.h
# End Source File
# Begin Source File

SOURCE=.\Actor\QKFrame.c
DEP_CPP_QKFRA=\
	"..\..\..\include\BaseType.h"\
	"..\..\..\include\Errorlog.h"\
	"..\..\..\include\jeMemAllocInfo.h"\
	"..\..\..\include\QUATERN.H"\
	"..\..\..\include\Ram.h"\
	"..\..\..\include\Vec3d.h"\
	"..\..\..\include\VFILE.H"\
	"..\..\..\include\Xform3d.h"\
	".\Actor\QKFrame.h"\
	".\Actor\tkarray.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Actor\QKFrame.h
# End Source File
# Begin Source File

SOURCE=.\Actor\strblock.c
DEP_CPP_STRBL=\
	"..\..\..\include\BaseType.h"\
	"..\..\..\include\Errorlog.h"\
	"..\..\..\include\jeMemAllocInfo.h"\
	"..\..\..\include\Ram.h"\
	"..\..\..\include\VFILE.H"\
	".\Actor\strblock.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Actor\strblock.h
# End Source File
# Begin Source File

SOURCE=.\Actor\tkarray.c
DEP_CPP_TKARR=\
	"..\..\..\include\BaseType.h"\
	"..\..\..\include\Errorlog.h"\
	"..\..\..\include\jeMemAllocInfo.h"\
	"..\..\..\include\Ram.h"\
	"..\..\..\include\VFILE.H"\
	".\Actor\tkarray.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Actor\tkarray.h
# End Source File
# Begin Source File

SOURCE=.\Actor\tkevents.c
DEP_CPP_TKEVE=\
	"..\..\..\include\BaseType.h"\
	"..\..\..\include\Errorlog.h"\
	"..\..\..\include\jeMemAllocInfo.h"\
	"..\..\..\include\Ram.h"\
	"..\..\..\include\VFILE.H"\
	".\Actor\tkarray.h"\
	".\Actor\tkevents.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Actor\tkevents.h
# End Source File
# Begin Source File

SOURCE=.\Actor\vkframe.c
DEP_CPP_VKFRA=\
	"..\..\..\include\BaseType.h"\
	"..\..\..\include\Errorlog.h"\
	"..\..\..\include\jeMemAllocInfo.h"\
	"..\..\..\include\Ram.h"\
	"..\..\..\include\Vec3d.h"\
	"..\..\..\include\VFILE.H"\
	".\Actor\tkarray.h"\
	".\Actor\vkframe.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Actor\vkframe.h
# End Source File
# Begin Source File

SOURCE=.\Actor\XFArray.c
DEP_CPP_XFARR=\
	"..\..\..\include\BaseType.h"\
	"..\..\..\include\Errorlog.h"\
	"..\..\..\include\jeMemAllocInfo.h"\
	"..\..\..\include\Ram.h"\
	"..\..\..\include\Vec3d.h"\
	"..\..\..\include\Xform3d.h"\
	".\Actor\xfarray.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Actor\xfarray.h
# End Source File
# End Group
# Begin Group "Bitmap"

# PROP Default_Filter ""
# Begin Group "Compression"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Bitmap\Compression\arithc.c
DEP_CPP_ARITH=\
	"..\..\..\include\BaseType.h"\
	"..\..\..\include\Errorlog.h"\
	"..\..\..\include\jeMemAllocInfo.h"\
	"..\..\..\include\Ram.h"\
	".\Bitmap\Compression\arithc._h"\
	".\Bitmap\Compression\arithc.h"\
	".\Bitmap\Compression\utility.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Bitmap\Compression\arithc.h
# End Source File
# Begin Source File

SOURCE=.\bitmap\compression\cache3dn.asm

!IF  "$(CFG)" == "JetEngine - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build
IntDir=.\Debug
InputPath=.\bitmap\compression\cache3dn.asm
InputName=cache3dn

"$(IntDir)\$(inputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml.exe -c -Zi -coff  -Fo "$(IntDir)\$(InputName).obj" "$(InputPath)"

# End Custom Build

!ELSEIF  "$(CFG)" == "JetEngine - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build
IntDir=.\Release
InputPath=.\bitmap\compression\cache3dn.asm
InputName=cache3dn

"$(IntDir)\$(inputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml -c -coff  -Fo "$(IntDir)\$(InputName).obj" "$(InputPath)"

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\bitmap\compression\cache3dn.h
# End Source File
# Begin Source File

SOURCE=.\Bitmap\Compression\codealphas.c
DEP_CPP_CODEA=\
	"..\..\..\include\BaseType.h"\
	"..\..\..\include\Errorlog.h"\
	"..\..\..\include\jeMemAllocInfo.h"\
	"..\..\..\include\Ram.h"\
	"..\..\..\include\Vec3d.h"\
	"..\..\..\include\Xform3d.h"\
	".\Bitmap\Compression\arithc.h"\
	".\Bitmap\Compression\codealphas.h"\
	".\Bitmap\Compression\coder.h"\
	".\Bitmap\Compression\codeutil.h"\
	".\Bitmap\Compression\image.h"\
	".\Bitmap\Compression\lbitio.h"\
	".\Bitmap\Compression\o1coder.h"\
	".\Bitmap\Compression\rungae.h"\
	".\Bitmap\Compression\utility.h"\
	".\Support\log.h"\
	".\tsc.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Bitmap\Compression\codealphas.h
# End Source File
# Begin Source File

SOURCE=.\Bitmap\Compression\codeimage.c
DEP_CPP_CODEI=\
	"..\..\..\include\BaseType.h"\
	"..\..\..\include\BITMAP.H"\
	"..\..\..\include\Errorlog.h"\
	"..\..\..\include\jeMemAllocInfo.h"\
	"..\..\..\include\jePtrMgr.h"\
	"..\..\..\include\PixelFormat.h"\
	"..\..\..\include\Ram.h"\
	"..\..\..\include\VFILE.H"\
	".\Bitmap\Compression\arithc.h"\
	".\Bitmap\Compression\codealphas.h"\
	".\Bitmap\Compression\codeimage.h"\
	".\Bitmap\Compression\coder.h"\
	".\Bitmap\Compression\codeutil.h"\
	".\Bitmap\Compression\image.h"\
	".\Bitmap\Compression\intmath.h"\
	".\Bitmap\Compression\lbitio.h"\
	".\Bitmap\Compression\transform.h"\
	".\Bitmap\Compression\utility.h"\
	".\Bitmap\Compression\Wavelet._h"\
	".\Bitmap\Compression\wavelet.h"\
	".\Support\cpu.h"\
	".\Support\ThreadQueue.h"\
	".\timer.h"\
	".\tsc.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Bitmap\Compression\codeimage.h
# End Source File
# Begin Source File

SOURCE=.\Bitmap\Compression\codepal.c
DEP_CPP_CODEP=\
	"..\..\..\include\BaseType.h"\
	"..\..\..\include\BITMAP.H"\
	"..\..\..\include\Errorlog.h"\
	"..\..\..\include\jeMemAllocInfo.h"\
	"..\..\..\include\jePtrMgr.h"\
	"..\..\..\include\PixelFormat.h"\
	"..\..\..\include\Ram.h"\
	"..\..\..\include\Vec3d.h"\
	"..\..\..\include\VFILE.H"\
	"..\..\..\include\Xform3d.h"\
	".\Bitmap\Compression\arithc.h"\
	".\Bitmap\Compression\codepal.h"\
	".\Bitmap\Compression\codeutil.h"\
	".\Bitmap\Compression\huffa.h"\
	".\Bitmap\Compression\lbitio.h"\
	".\Bitmap\Compression\o0coder.h"\
	".\Bitmap\Compression\utility.h"\
	".\Bitmap\Compression\yuv.h"\
	".\Support\log.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Bitmap\Compression\codepal.h
# End Source File
# Begin Source File

SOURCE=.\Bitmap\Compression\coder.c
DEP_CPP_CODER=\
	"..\..\..\include\BaseType.h"\
	"..\..\..\include\Errorlog.h"\
	"..\..\..\include\jeMemAllocInfo.h"\
	"..\..\..\include\Ram.h"\
	".\Bitmap\Compression\arithc.h"\
	".\Bitmap\Compression\coder.h"\
	".\Bitmap\Compression\codeutil.h"\
	".\Bitmap\Compression\intmath.h"\
	".\Bitmap\Compression\lbitio.h"\
	".\Bitmap\Compression\rungae.h"\
	".\Bitmap\Compression\utility.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Bitmap\Compression\coder.h
# End Source File
# Begin Source File

SOURCE=.\Bitmap\Compression\coder_bp.c
DEP_CPP_CODER_=\
	"..\..\..\include\BaseType.h"\
	"..\..\..\include\Errorlog.h"\
	"..\..\..\include\jeMemAllocInfo.h"\
	"..\..\..\include\Ram.h"\
	".\Bitmap\Compression\arithc.h"\
	".\Bitmap\Compression\coder.h"\
	".\Bitmap\Compression\soz.h"\
	".\Bitmap\Compression\utility.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Bitmap\Compression\coder_bpb2.c
DEP_CPP_CODER_B=\
	"..\..\..\include\BaseType.h"\
	"..\..\..\include\Errorlog.h"\
	"..\..\..\include\jeMemAllocInfo.h"\
	"..\..\..\include\Ram.h"\
	".\Bitmap\Compression\arithc.h"\
	".\Bitmap\Compression\coder.h"\
	".\Bitmap\Compression\utility.h"\
	".\Support\ThreadQueue.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Bitmap\Compression\coder_bpbf.c
DEP_CPP_CODER_BP=\
	"..\..\..\include\BaseType.h"\
	"..\..\..\include\Errorlog.h"\
	"..\..\..\include\jeMemAllocInfo.h"\
	"..\..\..\include\Ram.h"\
	".\Bitmap\Compression\arithc.h"\
	".\Bitmap\Compression\coder.h"\
	".\Bitmap\Compression\utility.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Bitmap\Compression\coder_bpbfr.c
DEP_CPP_CODER_BPB=\
	"..\..\..\include\BaseType.h"\
	"..\..\..\include\Errorlog.h"\
	"..\..\..\include\jeMemAllocInfo.h"\
	"..\..\..\include\Ram.h"\
	".\Bitmap\Compression\arithc.h"\
	".\Bitmap\Compression\coder.h"\
	".\Bitmap\Compression\rungae.h"\
	".\Bitmap\Compression\utility.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Bitmap\Compression\coder_bpbfrl.c
DEP_CPP_CODER_BPBF=\
	"..\..\..\include\BaseType.h"\
	"..\..\..\include\Errorlog.h"\
	"..\..\..\include\jeMemAllocInfo.h"\
	"..\..\..\include\Ram.h"\
	".\Bitmap\Compression\arithc.h"\
	".\Bitmap\Compression\coder.h"\
	".\Bitmap\Compression\codeutil.h"\
	".\Bitmap\Compression\intmath.h"\
	".\Bitmap\Compression\lbitio.h"\
	".\Bitmap\Compression\rungae.h"\
	".\Bitmap\Compression\utility.h"\
	".\report.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Bitmap\Compression\coder_f.c
DEP_CPP_CODER_F=\
	"..\..\..\include\BaseType.h"\
	"..\..\..\include\Errorlog.h"\
	"..\..\..\include\jeMemAllocInfo.h"\
	"..\..\..\include\Ram.h"\
	".\Bitmap\Compression\arithc.h"\
	".\Bitmap\Compression\coder.h"\
	".\Bitmap\Compression\rungae.h"\
	".\Bitmap\Compression\utility.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Bitmap\Compression\coder_f2.c
DEP_CPP_CODER_F2=\
	"..\..\..\include\BaseType.h"\
	"..\..\..\include\Errorlog.h"\
	"..\..\..\include\jeMemAllocInfo.h"\
	"..\..\..\include\Ram.h"\
	".\Bitmap\Compression\arithc.h"\
	".\Bitmap\Compression\coder.h"\
	".\Bitmap\Compression\rungae.h"\
	".\Bitmap\Compression\utility.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Bitmap\Compression\coder_f3.c
DEP_CPP_CODER_F3=\
	"..\..\..\include\BaseType.h"\
	"..\..\..\include\Errorlog.h"\
	"..\..\..\include\jeMemAllocInfo.h"\
	"..\..\..\include\Ram.h"\
	"..\..\..\include\Vec3d.h"\
	"..\..\..\include\Xform3d.h"\
	".\Bitmap\Compression\arithc.h"\
	".\Bitmap\Compression\coder.h"\
	".\Bitmap\Compression\rungae.h"\
	".\Bitmap\Compression\utility.h"\
	".\Support\log.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Bitmap\Compression\codeutil.c
DEP_CPP_CODEU=\
	"..\..\..\include\BaseType.h"\
	"..\..\..\include\Errorlog.h"\
	"..\..\..\include\jeMemAllocInfo.h"\
	"..\..\..\include\Ram.h"\
	".\Bitmap\Compression\arithc.h"\
	".\Bitmap\Compression\codeutil.h"\
	".\Bitmap\Compression\lbitio.h"\
	".\Bitmap\Compression\utility.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Bitmap\Compression\codeutil.h
# End Source File
# Begin Source File

SOURCE=.\Bitmap\Compression\colorconv.c
DEP_CPP_COLOR=\
	"..\..\..\include\BaseType.h"\
	"..\..\..\include\Errorlog.h"\
	"..\..\..\include\jeMemAllocInfo.h"\
	"..\..\..\include\Ram.h"\
	".\Bitmap\Compression\colorconv.h"\
	".\Bitmap\Compression\utility.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Bitmap\Compression\colorconv.h
# End Source File
# Begin Source File

SOURCE=.\Bitmap\Compression\context.c
DEP_CPP_CONTE=\
	"..\..\..\include\BaseType.h"\
	"..\..\..\include\Errorlog.h"\
	"..\..\..\include\jeMemAllocInfo.h"\
	"..\..\..\include\Ram.h"\
	".\Bitmap\Compression\arithc.h"\
	".\Bitmap\Compression\context.h"\
	".\Bitmap\Compression\utility.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Bitmap\Compression\context.h
# End Source File
# Begin Source File

SOURCE=.\Bitmap\Compression\huffa.c
DEP_CPP_HUFFA=\
	"..\..\..\include\BaseType.h"\
	"..\..\..\include\Errorlog.h"\
	"..\..\..\include\jeMemAllocInfo.h"\
	"..\..\..\include\Ram.h"\
	".\Bitmap\Compression\arithc.h"\
	".\Bitmap\Compression\codeutil.h"\
	".\Bitmap\Compression\huffa.h"\
	".\Bitmap\Compression\huffman2.h"\
	".\Bitmap\Compression\lbitio.h"\
	".\Bitmap\Compression\runtrans.h"\
	".\Bitmap\Compression\strutil.h"\
	".\Bitmap\Compression\utility.h"\
	".\Support\mempool.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Bitmap\Compression\huffa.h
# End Source File
# Begin Source File

SOURCE=.\Bitmap\Compression\huffman2.c
DEP_CPP_HUFFM=\
	"..\..\..\include\BaseType.h"\
	"..\..\..\include\Errorlog.h"\
	"..\..\..\include\jeMemAllocInfo.h"\
	"..\..\..\include\Ram.h"\
	".\Bitmap\Compression\huffman2.h"\
	".\Bitmap\Compression\intmath.h"\
	".\Bitmap\Compression\lbitio.h"\
	".\Bitmap\Compression\utility.h"\
	".\Support\mempool.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Bitmap\Compression\huffman2.h
# End Source File
# Begin Source File

SOURCE=.\Bitmap\Compression\image.c
DEP_CPP_IMAGE=\
	"..\..\..\include\BaseType.h"\
	"..\..\..\include\Errorlog.h"\
	"..\..\..\include\jeMemAllocInfo.h"\
	"..\..\..\include\Ram.h"\
	"..\..\..\include\Vec3d.h"\
	"..\..\..\include\Xform3d.h"\
	".\Bitmap\Compression\image.h"\
	".\Bitmap\Compression\intmath.h"\
	".\Bitmap\Compression\utility.h"\
	".\Support\cpu.h"\
	".\Support\log.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Bitmap\Compression\image.h
# End Source File
# Begin Source File

SOURCE=.\Bitmap\Compression\intmath.c
DEP_CPP_INTMA=\
	"..\..\..\include\BaseType.h"\
	"..\..\..\include\Errorlog.h"\
	"..\..\..\include\jeMemAllocInfo.h"\
	"..\..\..\include\Ram.h"\
	".\Bitmap\Compression\intmath.h"\
	".\Bitmap\Compression\utility.h"\
	".\Support\cpu.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Bitmap\Compression\intmath.h
# End Source File
# Begin Source File

SOURCE=.\Bitmap\Compression\katcache.c
# End Source File
# Begin Source File

SOURCE=.\Bitmap\Compression\katcache.h
# End Source File
# Begin Source File

SOURCE=.\Bitmap\Compression\ladder.c
DEP_CPP_LADDE=\
	"..\..\..\include\BaseType.h"\
	"..\..\..\include\Errorlog.h"\
	"..\..\..\include\jeMemAllocInfo.h"\
	"..\..\..\include\Ram.h"\
	".\Bitmap\Compression\arithc.h"\
	".\Bitmap\Compression\ladder.h"\
	".\Bitmap\Compression\rungae.h"\
	".\Bitmap\Compression\utility.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Bitmap\Compression\ladder.h
# End Source File
# Begin Source File

SOURCE=.\Bitmap\Compression\lbitio.c
DEP_CPP_LBITI=\
	"..\..\..\include\BaseType.h"\
	"..\..\..\include\Errorlog.h"\
	"..\..\..\include\jeMemAllocInfo.h"\
	"..\..\..\include\Ram.h"\
	".\Bitmap\Compression\lbitio.h"\
	".\Bitmap\Compression\utility.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Bitmap\Compression\lbitio.h
# End Source File
# Begin Source File

SOURCE=.\Bitmap\Compression\lza.c
DEP_CPP_LZA_C=\
	"..\..\..\include\BaseType.h"\
	"..\..\..\include\Errorlog.h"\
	"..\..\..\include\jeMemAllocInfo.h"\
	"..\..\..\include\Ram.h"\
	".\Bitmap\Compression\arithc.h"\
	".\Bitmap\Compression\intmath.h"\
	".\Bitmap\Compression\lza.h"\
	".\Bitmap\Compression\o0coder.h"\
	".\Bitmap\Compression\o1coder.h"\
	".\Bitmap\Compression\rungae.h"\
	".\Bitmap\Compression\utility.h"\
	".\Math\crc32.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Bitmap\Compression\lza.h
# End Source File
# Begin Source File

SOURCE=.\Bitmap\Compression\lzh.c
DEP_CPP_LZH_C=\
	"..\..\..\include\BaseType.h"\
	"..\..\..\include\Errorlog.h"\
	"..\..\..\include\jeMemAllocInfo.h"\
	"..\..\..\include\Ram.h"\
	"..\..\..\include\Vec3d.h"\
	"..\..\..\include\Xform3d.h"\
	".\Bitmap\Compression\huffa.h"\
	".\Bitmap\Compression\intmath.h"\
	".\Bitmap\Compression\utility.h"\
	".\Support\log.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Bitmap\Compression\lzh.h
# End Source File
# Begin Source File

SOURCE=.\Bitmap\Compression\o0coder.c
DEP_CPP_O0COD=\
	"..\..\..\include\BaseType.h"\
	"..\..\..\include\Errorlog.h"\
	"..\..\..\include\jeMemAllocInfo.h"\
	"..\..\..\include\Ram.h"\
	".\Bitmap\Compression\arithc.h"\
	".\Bitmap\Compression\context.h"\
	".\Bitmap\Compression\o0coder.h"\
	".\Bitmap\Compression\utility.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Bitmap\Compression\o0coder.h
# End Source File
# Begin Source File

SOURCE=.\Bitmap\Compression\o1coder.c
DEP_CPP_O1COD=\
	"..\..\..\include\BaseType.h"\
	"..\..\..\include\Errorlog.h"\
	"..\..\..\include\jeMemAllocInfo.h"\
	"..\..\..\include\Ram.h"\
	".\Bitmap\Compression\arithc.h"\
	".\Bitmap\Compression\context.h"\
	".\Bitmap\Compression\o0coder.h"\
	".\Bitmap\Compression\o1coder.h"\
	".\Bitmap\Compression\utility.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Bitmap\Compression\o1coder.h
# End Source File
# Begin Source File

SOURCE=.\Bitmap\Compression\palcreate.c
DEP_CPP_PALCR=\
	"..\..\..\include\BaseType.h"\
	"..\..\..\include\BITMAP.H"\
	"..\..\..\include\Errorlog.h"\
	"..\..\..\include\jeMemAllocInfo.h"\
	"..\..\..\include\jePtrMgr.h"\
	"..\..\..\include\PixelFormat.h"\
	"..\..\..\include\Ram.h"\
	"..\..\..\include\VFILE.H"\
	".\Bitmap\Compression\image.h"\
	".\Bitmap\Compression\palcreate.h"\
	".\Bitmap\Compression\paloptimize.h"\
	".\Bitmap\Compression\utility.h"\
	".\Bitmap\Compression\yuv.h"\
	".\Support\mempool.h"\
	".\tsc.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Bitmap\Compression\palcreate.h
# End Source File
# Begin Source File

SOURCE=.\Bitmap\Compression\palettize.c
DEP_CPP_PALET=\
	"..\..\..\include\BaseType.h"\
	"..\..\..\include\BITMAP.H"\
	"..\..\..\include\jeMemAllocInfo.h"\
	"..\..\..\include\jePtrMgr.h"\
	"..\..\..\include\PixelFormat.h"\
	"..\..\..\include\Ram.h"\
	"..\..\..\include\VFILE.H"\
	".\Bitmap\Compression\palettize.h"\
	".\Support\mempool.h"\
	".\tsc.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Bitmap\Compression\palettize.h
# End Source File
# Begin Source File

SOURCE=.\Bitmap\Compression\paloptimize.c
DEP_CPP_PALOP=\
	"..\..\..\include\BaseType.h"\
	"..\..\..\include\BITMAP.H"\
	"..\..\..\include\Errorlog.h"\
	"..\..\..\include\jeMemAllocInfo.h"\
	"..\..\..\include\jePtrMgr.h"\
	"..\..\..\include\PixelFormat.h"\
	"..\..\..\include\Ram.h"\
	"..\..\..\include\Vec3d.h"\
	"..\..\..\include\VFILE.H"\
	"..\..\..\include\Xform3d.h"\
	".\Bitmap\Compression\intmath.h"\
	".\Bitmap\Compression\palettize.h"\
	".\Bitmap\Compression\paloptimize.h"\
	".\Bitmap\Compression\utility.h"\
	".\Bitmap\Compression\yuv.h"\
	".\Support\log.h"\
	".\tsc.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Bitmap\Compression\paloptimize.h
# End Source File
# Begin Source File

SOURCE=.\Bitmap\Compression\rungae.c
DEP_CPP_RUNGA=\
	"..\..\..\include\BaseType.h"\
	"..\..\..\include\Errorlog.h"\
	"..\..\..\include\jeMemAllocInfo.h"\
	"..\..\..\include\Ram.h"\
	".\Bitmap\Compression\arithc._h"\
	".\Bitmap\Compression\arithc.h"\
	".\Bitmap\Compression\ladder.h"\
	".\Bitmap\Compression\rungae.h"\
	".\Bitmap\Compression\utility.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Bitmap\Compression\rungae.h
# End Source File
# Begin Source File

SOURCE=.\Bitmap\Compression\rungasm.asm

!IF  "$(CFG)" == "JetEngine - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Performing Custom Build Step on $(InputPath) to $(IntDir)\$(InputName)
IntDir=.\Debug
InputPath=.\Bitmap\Compression\rungasm.asm
InputName=rungasm

"$(IntDir)\$(inputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml.exe -Zi -c -coff  -Fo "$(IntDir)\$(InputName).obj" "$(InputPath)"

# End Custom Build

!ELSEIF  "$(CFG)" == "JetEngine - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Performing Custom Build Step on $(InputPath) to $(IntDir)\$(InputName)
IntDir=.\Release
InputPath=.\Bitmap\Compression\rungasm.asm
InputName=rungasm

"$(IntDir)\$(inputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml -Zi -c -coff  -Fo "$(IntDir)\$(InputName).obj" "$(InputPath)"

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Bitmap\Compression\rungo1.c
DEP_CPP_RUNGO=\
	"..\..\..\include\BaseType.h"\
	"..\..\..\include\Errorlog.h"\
	"..\..\..\include\jeMemAllocInfo.h"\
	"..\..\..\include\Ram.h"\
	".\Bitmap\Compression\arithc.h"\
	".\Bitmap\Compression\rungae.h"\
	".\Bitmap\Compression\rungo1.h"\
	".\Bitmap\Compression\utility.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Bitmap\Compression\rungo1.h
# End Source File
# Begin Source File

SOURCE=.\Bitmap\Compression\runtrans.c
DEP_CPP_RUNTR=\
	"..\..\..\include\BaseType.h"\
	"..\..\..\include\Errorlog.h"\
	"..\..\..\include\jeMemAllocInfo.h"\
	"..\..\..\include\Ram.h"\
	".\Bitmap\Compression\arithc.h"\
	".\Bitmap\Compression\o0coder.h"\
	".\Bitmap\Compression\utility.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Bitmap\Compression\runtrans.h
# End Source File
# Begin Source File

SOURCE=.\Bitmap\Compression\scontext.c
DEP_CPP_SCONT=\
	"..\..\..\include\BaseType.h"\
	"..\..\..\include\Errorlog.h"\
	"..\..\..\include\jeMemAllocInfo.h"\
	"..\..\..\include\Ram.h"\
	".\Bitmap\Compression\arithc.h"\
	".\Bitmap\Compression\utility.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Bitmap\Compression\scontext.h
# End Source File
# Begin Source File

SOURCE=.\Bitmap\Compression\sortpal.c
DEP_CPP_SORTP=\
	"..\..\..\include\BaseType.h"\
	"..\..\..\include\Errorlog.h"\
	"..\..\..\include\jeMemAllocInfo.h"\
	"..\..\..\include\Ram.h"\
	".\Bitmap\Compression\colorconv.h"\
	".\Bitmap\Compression\sortpal.h"\
	".\Bitmap\Compression\utility.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Bitmap\Compression\sortpal.h
# End Source File
# Begin Source File

SOURCE=.\Bitmap\Compression\sortpalx.c
DEP_CPP_SORTPA=\
	"..\..\..\include\BaseType.h"\
	"..\..\..\include\Errorlog.h"\
	"..\..\..\include\jeMemAllocInfo.h"\
	"..\..\..\include\Ram.h"\
	".\Bitmap\Compression\colorconv.h"\
	".\Bitmap\Compression\sortpal.h"\
	".\Bitmap\Compression\utility.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Bitmap\Compression\sortpalx.h
# End Source File
# Begin Source File

SOURCE=.\Bitmap\Compression\soz.c
DEP_CPP_SOZ_C=\
	"..\..\..\include\BaseType.h"\
	"..\..\..\include\Errorlog.h"\
	"..\..\..\include\jeMemAllocInfo.h"\
	"..\..\..\include\Ram.h"\
	".\Bitmap\Compression\arithc.h"\
	".\Bitmap\Compression\soz.h"\
	".\Bitmap\Compression\utility.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Bitmap\Compression\soz.h
# End Source File
# Begin Source File

SOURCE=.\Bitmap\Compression\strutil.c
DEP_CPP_STRUT=\
	"..\..\..\include\BaseType.h"\
	"..\..\..\include\Errorlog.h"\
	"..\..\..\include\jeMemAllocInfo.h"\
	"..\..\..\include\Ram.h"\
	".\Bitmap\Compression\strutil.h"\
	".\Bitmap\Compression\utility.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Bitmap\Compression\strutil.h
# End Source File
# Begin Source File

SOURCE=.\Bitmap\Compression\tblock.c
DEP_CPP_TBLOC=\
	"..\..\..\include\BaseType.h"\
	"..\..\..\include\BITMAP.H"\
	"..\..\..\include\Errorlog.h"\
	"..\..\..\include\jeMemAllocInfo.h"\
	"..\..\..\include\jePtrMgr.h"\
	"..\..\..\include\PixelFormat.h"\
	"..\..\..\include\Ram.h"\
	"..\..\..\include\Vec3d.h"\
	"..\..\..\include\VFILE.H"\
	"..\..\..\include\Xform3d.h"\
	".\Bitmap\Compression\codeimage.h"\
	".\Bitmap\Compression\image.h"\
	".\Bitmap\Compression\tblock.h"\
	".\Bitmap\Compression\transform.h"\
	".\Bitmap\Compression\utility.h"\
	".\Bitmap\Compression\wavelet.h"\
	".\Support\cpu.h"\
	".\Support\log.h"\
	".\Support\ThreadQueue.h"\
	".\timer.h"\
	".\tsc.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Bitmap\Compression\tblock.h
# End Source File
# Begin Source File

SOURCE=.\Bitmap\Compression\trans_bcw3.c
DEP_CPP_TRANS=\
	"..\..\..\include\BaseType.h"\
	"..\..\..\include\Errorlog.h"\
	"..\..\..\include\jeMemAllocInfo.h"\
	"..\..\..\include\Ram.h"\
	".\Bitmap\Compression\utility.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Bitmap\Compression\trans_cdf22.c
DEP_CPP_TRANS_=\
	"..\..\..\include\BaseType.h"\
	"..\..\..\include\Errorlog.h"\
	"..\..\..\include\jeMemAllocInfo.h"\
	"..\..\..\include\Ram.h"\
	".\Bitmap\Compression\utility.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Bitmap\Compression\trans_cdf22q.c
DEP_CPP_TRANS_C=\
	"..\..\..\include\BaseType.h"\
	"..\..\..\include\Errorlog.h"\
	"..\..\..\include\jeMemAllocInfo.h"\
	"..\..\..\include\Ram.h"\
	".\Bitmap\Compression\utility.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Bitmap\Compression\trans_cdf24.c
DEP_CPP_TRANS_CD=\
	"..\..\..\include\BaseType.h"\
	"..\..\..\include\Errorlog.h"\
	"..\..\..\include\jeMemAllocInfo.h"\
	"..\..\..\include\Ram.h"\
	".\Bitmap\Compression\utility.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Bitmap\Compression\trans_d4.c
DEP_CPP_TRANS_D=\
	"..\..\..\include\BaseType.h"\
	"..\..\..\include\Errorlog.h"\
	"..\..\..\include\jeMemAllocInfo.h"\
	"..\..\..\include\Ram.h"\
	".\Bitmap\Compression\utility.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Bitmap\Compression\trans_l97.c
DEP_CPP_TRANS_L=\
	"..\..\..\include\BaseType.h"\
	"..\..\..\include\Errorlog.h"\
	"..\..\..\include\jeMemAllocInfo.h"\
	"..\..\..\include\Ram.h"\
	".\Bitmap\Compression\utility.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Bitmap\Compression\trans_spt.c
DEP_CPP_TRANS_S=\
	"..\..\..\include\BaseType.h"\
	"..\..\..\include\Errorlog.h"\
	"..\..\..\include\jeMemAllocInfo.h"\
	"..\..\..\include\Ram.h"\
	".\Bitmap\Compression\utility.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Bitmap\Compression\transform.c
DEP_CPP_TRANSF=\
	"..\..\..\include\BaseType.h"\
	"..\..\..\include\BITMAP.H"\
	"..\..\..\include\Errorlog.h"\
	"..\..\..\include\jeMemAllocInfo.h"\
	"..\..\..\include\jePtrMgr.h"\
	"..\..\..\include\PixelFormat.h"\
	"..\..\..\include\Ram.h"\
	"..\..\..\include\VFILE.H"\
	".\Bitmap\Compression\image.h"\
	".\Bitmap\Compression\intmath.h"\
	".\Bitmap\Compression\transform.h"\
	".\Bitmap\Compression\utility.h"\
	".\Bitmap\Compression\wavelet.h"\
	".\Support\ThreadQueue.h"\
	".\tsc.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Bitmap\Compression\transform.h
# End Source File
# Begin Source File

SOURCE=.\Bitmap\Compression\utility.c
DEP_CPP_UTILI=\
	"..\..\..\include\BaseType.h"\
	"..\..\..\include\Errorlog.h"\
	"..\..\..\include\jeMemAllocInfo.h"\
	"..\..\..\include\Ram.h"\
	".\bitmap\compression\cache3dn.h"\
	".\Bitmap\Compression\katcache.h"\
	".\Bitmap\Compression\utility.h"\
	".\Bitmap\Compression\yuv.h"\
	".\Support\cpu.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Bitmap\Compression\utility.h
# End Source File
# Begin Source File

SOURCE=.\Bitmap\Compression\wavelet.c
DEP_CPP_WAVEL=\
	"..\..\..\include\BaseType.h"\
	"..\..\..\include\BITMAP.H"\
	"..\..\..\include\Camera.h"\
	"..\..\..\include\Errorlog.h"\
	"..\..\..\include\jeChain.h"\
	"..\..\..\include\jeMemAllocInfo.h"\
	"..\..\..\include\jePtrMgr.h"\
	"..\..\..\include\jeResource.h"\
	"..\..\..\include\jeStaticMesh.h"\
	"..\..\..\include\jeTypes.h"\
	"..\..\..\include\PixelFormat.h"\
	"..\..\..\include\Ram.h"\
	"..\..\..\include\Vec3d.h"\
	"..\..\..\include\VFILE.H"\
	"..\..\..\include\Xform3d.h"\
	".\Bitmap\bitmap.__h"\
	".\Bitmap\bitmap._h"\
	".\Bitmap\Compression\arithc.h"\
	".\Bitmap\Compression\codeimage.h"\
	".\Bitmap\Compression\coder.h"\
	".\Bitmap\Compression\image.h"\
	".\Bitmap\Compression\intmath.h"\
	".\Bitmap\Compression\palcreate.h"\
	".\Bitmap\Compression\palettize.h"\
	".\Bitmap\Compression\transform.h"\
	".\Bitmap\Compression\utility.h"\
	".\Bitmap\Compression\Wavelet._h"\
	".\Bitmap\Compression\wavelet.h"\
	".\Bitmap\Compression\yuv.h"\
	".\Engine\Drivers\Dcommon.h"\
	".\Support\cpu.h"\
	".\Support\log.h"\
	".\Support\ThreadLog.h"\
	".\Support\ThreadQueue.h"\
	".\timer.h"\
	".\tsc.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Bitmap\Compression\wavelet.h
# End Source File
# Begin Source File

SOURCE=.\Bitmap\Compression\yuv.c
DEP_CPP_YUV_C=\
	"..\..\..\include\BaseType.h"\
	"..\..\..\include\Errorlog.h"\
	"..\..\..\include\jeMemAllocInfo.h"\
	"..\..\..\include\Ram.h"\
	".\Bitmap\Compression\utility.h"\
	".\Bitmap\Compression\yuv.h"\
	".\Support\cpu.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Bitmap\Compression\yuv.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\Bitmap\bitmap.__h
# End Source File
# Begin Source File

SOURCE=.\Bitmap\bitmap._h
# End Source File
# Begin Source File

SOURCE=.\Bitmap\bitmap.c
DEP_CPP_BITMA=\
	"..\..\..\include\BaseType.h"\
	"..\..\..\include\BITMAP.H"\
	"..\..\..\include\Camera.h"\
	"..\..\..\include\Errorlog.h"\
	"..\..\..\include\jeChain.h"\
	"..\..\..\include\jeMemAllocInfo.h"\
	"..\..\..\include\jePtrMgr.h"\
	"..\..\..\include\jeResource.h"\
	"..\..\..\include\jeStaticMesh.h"\
	"..\..\..\include\jeTypes.h"\
	"..\..\..\include\PixelFormat.h"\
	"..\..\..\include\Ram.h"\
	"..\..\..\include\Vec3d.h"\
	"..\..\..\include\VFILE.H"\
	"..\..\..\include\Xform3d.h"\
	".\Bitmap\bitmap.__h"\
	".\Bitmap\bitmap._h"\
	".\Bitmap\bitmap_blitdata.h"\
	".\Bitmap\bitmap_gamma.h"\
	".\Bitmap\Compression\codepal.h"\
	".\Bitmap\Compression\image.h"\
	".\Bitmap\Compression\palcreate.h"\
	".\Bitmap\Compression\palettize.h"\
	".\Bitmap\Compression\sortpal.h"\
	".\Bitmap\Compression\sortpalx.h"\
	".\Bitmap\Compression\utility.h"\
	".\Bitmap\Compression\wavelet.h"\
	".\Engine\Drivers\Dcommon.h"\
	".\Support\cpu.h"\
	".\Support\log.h"\
	".\Support\mempool.h"\
	".\Support\ThreadLog.h"\
	".\Support\ThreadQueue.h"\
	".\timer.h"\
	".\tsc.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\..\include\BITMAP.H
# End Source File
# Begin Source File

SOURCE=.\Bitmap\bitmap_blitdata.c
DEP_CPP_BITMAP=\
	"..\..\..\include\BaseType.h"\
	"..\..\..\include\BITMAP.H"\
	"..\..\..\include\Camera.h"\
	"..\..\..\include\Errorlog.h"\
	"..\..\..\include\jeChain.h"\
	"..\..\..\include\jeMemAllocInfo.h"\
	"..\..\..\include\jePtrMgr.h"\
	"..\..\..\include\jeResource.h"\
	"..\..\..\include\jeStaticMesh.h"\
	"..\..\..\include\jeTypes.h"\
	"..\..\..\include\PixelFormat.h"\
	"..\..\..\include\Ram.h"\
	"..\..\..\include\Vec3d.h"\
	"..\..\..\include\VFILE.H"\
	"..\..\..\include\Xform3d.h"\
	".\Bitmap\bitmap.__h"\
	".\Bitmap\bitmap._h"\
	".\Bitmap\bitmap_blitdata.h"\
	".\Bitmap\Compression\image.h"\
	".\Bitmap\Compression\palcreate.h"\
	".\Bitmap\Compression\palettize.h"\
	".\Bitmap\Compression\utility.h"\
	".\Bitmap\Compression\wavelet.h"\
	".\Engine\Drivers\Dcommon.h"\
	".\Support\cpu.h"\
	".\Support\ThreadQueue.h"\
	".\timer.h"\
	".\tsc.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Bitmap\bitmap_blitdata.h
# End Source File
# Begin Source File

SOURCE=.\Bitmap\bitmap_gamma.c
DEP_CPP_BITMAP_=\
	"..\..\..\include\BaseType.h"\
	"..\..\..\include\BITMAP.H"\
	"..\..\..\include\Camera.h"\
	"..\..\..\include\Errorlog.h"\
	"..\..\..\include\jeChain.h"\
	"..\..\..\include\jePtrMgr.h"\
	"..\..\..\include\jeResource.h"\
	"..\..\..\include\jeStaticMesh.h"\
	"..\..\..\include\jeTypes.h"\
	"..\..\..\include\PixelFormat.h"\
	"..\..\..\include\Vec3d.h"\
	"..\..\..\include\VFILE.H"\
	"..\..\..\include\Xform3d.h"\
	".\Bitmap\bitmap.__h"\
	".\Bitmap\bitmap._h"\
	".\Bitmap\bitmap_gamma.h"\
	".\Bitmap\Compression\wavelet.h"\
	".\Engine\Drivers\Dcommon.h"\
	".\Support\ThreadQueue.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Bitmap\bitmap_gamma.h
# End Source File
# Begin Source File

SOURCE=.\Bitmap\pixelformat.c
DEP_CPP_PIXEL=\
	"..\..\..\include\BaseType.h"\
	"..\..\..\include\PixelFormat.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Bitmap\pixelformat.h
# End Source File
# End Group
# Begin Group "BSP"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Bsp\jeBSP._h
# End Source File
# Begin Source File

SOURCE=.\Bsp\jeBSP.c
DEP_CPP_JEBSP=\
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
	"..\..\..\include\jeStaticMesh.h"\
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
	".\Bsp\jeBSP._h"\
	".\Camera._h"\
	".\Engine\Drivers\Dcommon.h"\
	".\guWorld\jeIndexPoly.h"\
	".\guWorld\jePlaneArray.h"\
	".\guWorld\jeTexVec.h"\
	".\guWorld\visobject.h"\
	".\list.h"\
	".\Support\log.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\..\include\jeBSP.h
# End Source File
# Begin Source File

SOURCE=.\Bsp\jeBSP_Brush.c
DEP_CPP_JEBSP_=\
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
	"..\..\..\include\jeStaticMesh.h"\
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
	".\Bsp\jeBSP._h"\
	".\Engine\Drivers\Dcommon.h"\
	".\guWorld\jeIndexPoly.h"\
	".\guWorld\jePlaneArray.h"\
	".\guWorld\jeTexVec.h"\
	".\guWorld\visobject.h"\
	".\list.h"\
	".\Support\log.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Bsp\jeBSP_TopBrush.c
DEP_CPP_JEBSP_T=\
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
	"..\..\..\include\jeMemAllocInfo.h"\
	"..\..\..\include\JEMODEL.H"\
	"..\..\..\include\jePlane.h"\
	"..\..\..\include\jePoly.h"\
	"..\..\..\include\jePortal.h"\
	"..\..\..\include\jeProperty.h"\
	"..\..\..\include\jePtrMgr.h"\
	"..\..\..\include\jeResource.h"\
	"..\..\..\include\jeStaticMesh.h"\
	"..\..\..\include\jeTypes.h"\
	"..\..\..\include\jeUserPoly.h"\
	"..\..\..\include\jeVertArray.h"\
	"..\..\..\include\JEWORLD.H"\
	"..\..\..\include\OBJECT.H"\
	"..\..\..\include\PixelFormat.h"\
	"..\..\..\include\QUATERN.H"\
	"..\..\..\include\Ram.h"\
	"..\..\..\include\Sound.h"\
	"..\..\..\include\Vec3d.h"\
	"..\..\..\include\VFILE.H"\
	"..\..\..\include\Xform3d.h"\
	".\Bsp\jeBSP._h"\
	".\Engine\Drivers\Dcommon.h"\
	".\guWorld\jeIndexPoly.h"\
	".\guWorld\jePlaneArray.h"\
	".\guWorld\jeTexVec.h"\
	".\guWorld\visobject.h"\
	".\list.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Bsp\jeBSPNode.c
DEP_CPP_JEBSPN=\
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
	"..\..\..\include\jeStaticMesh.h"\
	"..\..\..\include\jeTypes.h"\
	"..\..\..\include\jeUserPoly.h"\
	"..\..\..\include\jeVertArray.h"\
	"..\..\..\include\JEWORLD.H"\
	"..\..\..\include\OBJECT.H"\
	"..\..\..\include\PixelFormat.h"\
	"..\..\..\include\QUATERN.H"\
	"..\..\..\include\Ram.h"\
	"..\..\..\include\Sound.h"\
	"..\..\..\include\Vec3d.h"\
	"..\..\..\include\VFILE.H"\
	"..\..\..\include\Xform3d.h"\
	".\Bsp\jeBSP._h"\
	".\Engine\Drivers\Dcommon.h"\
	".\guWorld\jeIndexPoly.h"\
	".\guWorld\jePlaneArray.h"\
	".\guWorld\jeTexVec.h"\
	".\guWorld\visobject.h"\
	".\list.h"\
	".\Support\log.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Bsp\jeBSPNode_Area.c
DEP_CPP_JEBSPNO=\
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
	"..\..\..\include\jeStaticMesh.h"\
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
	".\Bsp\jeBSP._h"\
	".\Engine\Drivers\Dcommon.h"\
	".\guWorld\jeIndexPoly.h"\
	".\guWorld\jePlaneArray.h"\
	".\guWorld\jeTexVec.h"\
	".\guWorld\visobject.h"\
	".\list.h"\
	".\Support\log.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Bsp\jeBSPNode_DrawFace.c
DEP_CPP_JEBSPNOD=\
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
	"..\..\..\include\jeMemAllocInfo.h"\
	"..\..\..\include\JEMODEL.H"\
	"..\..\..\include\jePlane.h"\
	"..\..\..\include\jePoly.h"\
	"..\..\..\include\jePortal.h"\
	"..\..\..\include\jeProperty.h"\
	"..\..\..\include\jePtrMgr.h"\
	"..\..\..\include\jeResource.h"\
	"..\..\..\include\jeStaticMesh.h"\
	"..\..\..\include\jeTexture.h"\
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
	".\Bitmap\bitmap._h"\
	".\Bsp\jeBSP._h"\
	".\Engine\Drivers\Dcommon.h"\
	".\guWorld\jeIndexPoly.h"\
	".\guWorld\jePlaneArray.h"\
	".\guWorld\jeTexVec.h"\
	".\guWorld\visobject.h"\
	".\list.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Bsp\jeBSPNode_Face.c
DEP_CPP_JEBSPNODE=\
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
	"..\..\..\include\jeStaticMesh.h"\
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
	".\Bsp\jeBSP._h"\
	".\Engine\Drivers\Dcommon.h"\
	".\guWorld\jeIndexPoly.h"\
	".\guWorld\jePlaneArray.h"\
	".\guWorld\jeTexVec.h"\
	".\guWorld\visobject.h"\
	".\list.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Bsp\jeBSPNode_Leaf.c
DEP_CPP_JEBSPNODE_=\
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
	"..\..\..\include\jeStaticMesh.h"\
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
	".\Bsp\jeBSP._h"\
	".\Engine\Drivers\Dcommon.h"\
	".\guWorld\jeIndexPoly.h"\
	".\guWorld\jePlaneArray.h"\
	".\guWorld\jeTexVec.h"\
	".\guWorld\visobject.h"\
	".\list.h"\
	".\Support\log.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Bsp\jeBSPNode_Light.c
DEP_CPP_JEBSPNODE_L=\
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
	"..\..\..\include\jeStaticMesh.h"\
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
	".\Bsp\jeBSP._h"\
	".\Engine\Drivers\Dcommon.h"\
	".\guWorld\jeIndexPoly.h"\
	".\guWorld\jePlaneArray.h"\
	".\guWorld\jeTexVec.h"\
	".\guWorld\visobject.h"\
	".\list.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Bsp\jeBSPNode_Portal.c
DEP_CPP_JEBSPNODE_P=\
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
	"..\..\..\include\jeStaticMesh.h"\
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
	".\Bsp\jeBSP._h"\
	".\Engine\Drivers\Dcommon.h"\
	".\guWorld\jeIndexPoly.h"\
	".\guWorld\jePlaneArray.h"\
	".\guWorld\jeTexVec.h"\
	".\guWorld\visobject.h"\
	".\list.h"\
	".\Support\log.h"\
	
# End Source File
# End Group
# Begin Group "Engine"

# PROP Default_Filter ""
# Begin Group "Drivers"

# PROP Default_Filter ""
# Begin Group "D3DDrv"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Engine\Drivers\D3DDrv\d3d_DDMemMgr.c
DEP_CPP_D3D_D=\
	"..\..\..\include\BaseType.h"\
	".\Engine\Drivers\D3DDrv\d3d_DDMemMgr.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Engine\Drivers\D3DDrv\d3d_DDMemMgr.h
# End Source File
# Begin Source File

SOURCE=.\Engine\Drivers\D3DDrv\d3d_err.cpp
DEP_CPP_D3D_E=\
	".\Engine\Drivers\D3DDrv\D3d_err.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Engine\Drivers\D3DDrv\D3d_err.h
# End Source File
# Begin Source File

SOURCE=.\Engine\Drivers\D3DDrv\d3d_fx.cpp
DEP_CPP_D3D_F=\
	"..\..\..\include\BaseType.h"\
	"..\..\..\include\Camera.h"\
	"..\..\..\include\jeChain.h"\
	"..\..\..\include\jePtrMgr.h"\
	"..\..\..\include\jeResource.h"\
	"..\..\..\include\jeStaticMesh.h"\
	"..\..\..\include\jeTypes.h"\
	"..\..\..\include\PixelFormat.h"\
	"..\..\..\include\Vec3d.h"\
	"..\..\..\include\VFILE.H"\
	"..\..\..\include\Xform3d.h"\
	".\Engine\Drivers\D3DDrv\D3d_err.h"\
	".\Engine\Drivers\D3DDrv\D3d_fx.h"\
	".\Engine\Drivers\D3DDrv\D3d_main.h"\
	".\Engine\Drivers\D3DDrv\d3d_RENDER.H"\
	".\Engine\Drivers\D3DDrv\d3d_Scene.h"\
	".\Engine\Drivers\D3DDrv\D3ddrv.h"\
	".\Engine\Drivers\Dcommon.h"\
	{$(INCLUDE)}"d3dx.h"\
	{$(INCLUDE)}"d3dxcore.h"\
	{$(INCLUDE)}"d3dxerr.h"\
	{$(INCLUDE)}"d3dxmath.h"\
	{$(INCLUDE)}"d3dxmath.inl"\
	{$(INCLUDE)}"d3dxshapes.h"\
	{$(INCLUDE)}"d3dxsprite.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Engine\Drivers\D3DDrv\D3d_fx.h
# End Source File
# Begin Source File

SOURCE=.\Engine\Drivers\D3DDrv\d3d_gspan.cpp
DEP_CPP_D3D_G=\
	"..\..\..\include\BaseType.h"\
	"..\..\..\include\Camera.h"\
	"..\..\..\include\jeChain.h"\
	"..\..\..\include\jePtrMgr.h"\
	"..\..\..\include\jeResource.h"\
	"..\..\..\include\jeStaticMesh.h"\
	"..\..\..\include\jeTypes.h"\
	"..\..\..\include\PixelFormat.h"\
	"..\..\..\include\Vec3d.h"\
	"..\..\..\include\VFILE.H"\
	"..\..\..\include\Xform3d.h"\
	".\Engine\Drivers\D3DDrv\d3d_GSPAN.H"\
	".\Engine\Drivers\D3DDrv\D3ddrv.h"\
	".\Engine\Drivers\Dcommon.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Engine\Drivers\D3DDrv\d3d_GSPAN.H
# End Source File
# Begin Source File

SOURCE=.\Engine\Drivers\D3DDrv\d3d_main.cpp
DEP_CPP_D3D_M=\
	"..\..\..\include\BaseType.h"\
	"..\..\..\include\Camera.h"\
	"..\..\..\include\jeChain.h"\
	"..\..\..\include\jePtrMgr.h"\
	"..\..\..\include\jeResource.h"\
	"..\..\..\include\jeStaticMesh.h"\
	"..\..\..\include\jeTypes.h"\
	"..\..\..\include\PixelFormat.h"\
	"..\..\..\include\Vec3d.h"\
	"..\..\..\include\VFILE.H"\
	"..\..\..\include\Xform3d.h"\
	".\Engine\Drivers\D3DDrv\d3d_DDMemMgr.h"\
	".\Engine\Drivers\D3DDrv\D3d_err.h"\
	".\Engine\Drivers\D3DDrv\D3d_fx.h"\
	".\Engine\Drivers\D3DDrv\D3d_main.h"\
	".\Engine\Drivers\D3DDrv\d3d_Pcache.h"\
	".\Engine\Drivers\D3DDrv\d3d_RENDER.H"\
	".\Engine\Drivers\D3DDrv\d3d_Scene.h"\
	".\Engine\Drivers\D3DDrv\d3d_THandle.h"\
	".\Engine\Drivers\D3DDrv\d3d_TPage.h"\
	".\Engine\Drivers\D3DDrv\D3dcache.h"\
	".\Engine\Drivers\D3DDrv\D3ddrv.h"\
	".\Engine\Drivers\Dcommon.h"\
	{$(INCLUDE)}"d3dx.h"\
	{$(INCLUDE)}"d3dxcore.h"\
	{$(INCLUDE)}"d3dxerr.h"\
	{$(INCLUDE)}"d3dxmath.h"\
	{$(INCLUDE)}"d3dxmath.inl"\
	{$(INCLUDE)}"d3dxshapes.h"\
	{$(INCLUDE)}"d3dxsprite.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Engine\Drivers\D3DDrv\D3d_main.h
# End Source File
# Begin Source File

SOURCE=.\Engine\Drivers\D3DDrv\d3d_pcache.cpp
DEP_CPP_D3D_P=\
	"..\..\..\include\BaseType.h"\
	"..\..\..\include\Camera.h"\
	"..\..\..\include\jeChain.h"\
	"..\..\..\include\jePtrMgr.h"\
	"..\..\..\include\jeResource.h"\
	"..\..\..\include\jeStaticMesh.h"\
	"..\..\..\include\jeTypes.h"\
	"..\..\..\include\PixelFormat.h"\
	"..\..\..\include\Vec3d.h"\
	"..\..\..\include\VFILE.H"\
	"..\..\..\include\Xform3d.h"\
	".\Engine\Drivers\D3DDrv\d3d_DDMemMgr.h"\
	".\Engine\Drivers\D3DDrv\D3d_err.h"\
	".\Engine\Drivers\D3DDrv\D3d_fx.h"\
	".\Engine\Drivers\D3DDrv\D3d_main.h"\
	".\Engine\Drivers\D3DDrv\d3d_Pcache.h"\
	".\Engine\Drivers\D3DDrv\d3d_Scene.h"\
	".\Engine\Drivers\D3DDrv\d3d_THandle.h"\
	".\Engine\Drivers\D3DDrv\d3d_TPage.h"\
	".\Engine\Drivers\D3DDrv\D3dcache.h"\
	".\Engine\Drivers\D3DDrv\D3ddrv.h"\
	".\Engine\Drivers\Dcommon.h"\
	{$(INCLUDE)}"d3dx.h"\
	{$(INCLUDE)}"d3dxcore.h"\
	{$(INCLUDE)}"d3dxerr.h"\
	{$(INCLUDE)}"d3dxmath.h"\
	{$(INCLUDE)}"d3dxmath.inl"\
	{$(INCLUDE)}"d3dxshapes.h"\
	{$(INCLUDE)}"d3dxsprite.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Engine\Drivers\D3DDrv\d3d_Pcache.h
# End Source File
# Begin Source File

SOURCE=.\Engine\Drivers\D3DDrv\d3d_render.cpp
DEP_CPP_D3D_R=\
	"..\..\..\include\BaseType.h"\
	"..\..\..\include\Camera.h"\
	"..\..\..\include\jeChain.h"\
	"..\..\..\include\jePtrMgr.h"\
	"..\..\..\include\jeResource.h"\
	"..\..\..\include\jeStaticMesh.h"\
	"..\..\..\include\jeTypes.h"\
	"..\..\..\include\PixelFormat.h"\
	"..\..\..\include\Vec3d.h"\
	"..\..\..\include\VFILE.H"\
	"..\..\..\include\Xform3d.h"\
	".\Engine\Drivers\D3DDrv\d3d_DDMemMgr.h"\
	".\Engine\Drivers\D3DDrv\D3d_err.h"\
	".\Engine\Drivers\D3DDrv\D3d_fx.h"\
	".\Engine\Drivers\D3DDrv\d3d_GSPAN.H"\
	".\Engine\Drivers\D3DDrv\D3d_main.h"\
	".\Engine\Drivers\D3DDrv\d3d_Pcache.h"\
	".\Engine\Drivers\D3DDrv\d3d_RENDER.H"\
	".\Engine\Drivers\D3DDrv\d3d_Scene.h"\
	".\Engine\Drivers\D3DDrv\d3d_THandle.h"\
	".\Engine\Drivers\D3DDrv\d3d_TPage.h"\
	".\Engine\Drivers\D3DDrv\D3dcache.h"\
	".\Engine\Drivers\D3DDrv\D3ddrv.h"\
	".\Engine\Drivers\Dcommon.h"\
	{$(INCLUDE)}"d3dx.h"\
	{$(INCLUDE)}"d3dxcore.h"\
	{$(INCLUDE)}"d3dxerr.h"\
	{$(INCLUDE)}"d3dxmath.h"\
	{$(INCLUDE)}"d3dxmath.inl"\
	{$(INCLUDE)}"d3dxshapes.h"\
	{$(INCLUDE)}"d3dxsprite.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Engine\Drivers\D3DDrv\d3d_RENDER.H
# End Source File
# Begin Source File

SOURCE=.\Engine\Drivers\D3DDrv\d3d_Scene.cpp
DEP_CPP_D3D_S=\
	"..\..\..\include\BaseType.h"\
	"..\..\..\include\Camera.h"\
	"..\..\..\include\jeChain.h"\
	"..\..\..\include\jePtrMgr.h"\
	"..\..\..\include\jeResource.h"\
	"..\..\..\include\jeStaticMesh.h"\
	"..\..\..\include\jeTypes.h"\
	"..\..\..\include\PixelFormat.h"\
	"..\..\..\include\Vec3d.h"\
	"..\..\..\include\VFILE.H"\
	"..\..\..\include\Xform3d.h"\
	".\Engine\Drivers\D3DDrv\d3d_DDMemMgr.h"\
	".\Engine\Drivers\D3DDrv\D3d_err.h"\
	".\Engine\Drivers\D3DDrv\D3d_fx.h"\
	".\Engine\Drivers\D3DDrv\d3d_GSPAN.H"\
	".\Engine\Drivers\D3DDrv\D3d_main.h"\
	".\Engine\Drivers\D3DDrv\d3d_Pcache.h"\
	".\Engine\Drivers\D3DDrv\d3d_RENDER.H"\
	".\Engine\Drivers\D3DDrv\d3d_Scene.h"\
	".\Engine\Drivers\D3DDrv\d3d_THandle.h"\
	".\Engine\Drivers\D3DDrv\d3d_TPage.h"\
	".\Engine\Drivers\D3DDrv\D3dcache.h"\
	".\Engine\Drivers\D3DDrv\D3ddrv.h"\
	".\Engine\Drivers\Dcommon.h"\
	{$(INCLUDE)}"d3dx.h"\
	{$(INCLUDE)}"d3dxcore.h"\
	{$(INCLUDE)}"d3dxerr.h"\
	{$(INCLUDE)}"d3dxmath.h"\
	{$(INCLUDE)}"d3dxmath.inl"\
	{$(INCLUDE)}"d3dxshapes.h"\
	{$(INCLUDE)}"d3dxsprite.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Engine\Drivers\D3DDrv\d3d_Scene.h
# End Source File
# Begin Source File

SOURCE=.\Engine\Drivers\D3DDrv\d3d_thandle.cpp
DEP_CPP_D3D_T=\
	"..\..\..\include\BaseType.h"\
	"..\..\..\include\Camera.h"\
	"..\..\..\include\jeChain.h"\
	"..\..\..\include\jePtrMgr.h"\
	"..\..\..\include\jeResource.h"\
	"..\..\..\include\jeStaticMesh.h"\
	"..\..\..\include\jeTypes.h"\
	"..\..\..\include\PixelFormat.h"\
	"..\..\..\include\Vec3d.h"\
	"..\..\..\include\VFILE.H"\
	"..\..\..\include\Xform3d.h"\
	".\Engine\Drivers\D3DDrv\d3d_DDMemMgr.h"\
	".\Engine\Drivers\D3DDrv\D3d_fx.h"\
	".\Engine\Drivers\D3DDrv\D3d_main.h"\
	".\Engine\Drivers\D3DDrv\d3d_Pcache.h"\
	".\Engine\Drivers\D3DDrv\d3d_Scene.h"\
	".\Engine\Drivers\D3DDrv\d3d_THandle.h"\
	".\Engine\Drivers\D3DDrv\d3d_TPage.h"\
	".\Engine\Drivers\D3DDrv\D3dcache.h"\
	".\Engine\Drivers\D3DDrv\D3ddrv.h"\
	".\Engine\Drivers\Dcommon.h"\
	{$(INCLUDE)}"d3dx.h"\
	{$(INCLUDE)}"d3dxcore.h"\
	{$(INCLUDE)}"d3dxerr.h"\
	{$(INCLUDE)}"d3dxmath.h"\
	{$(INCLUDE)}"d3dxmath.inl"\
	{$(INCLUDE)}"d3dxshapes.h"\
	{$(INCLUDE)}"d3dxsprite.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Engine\Drivers\D3DDrv\d3d_THandle.h
# End Source File
# Begin Source File

SOURCE=.\Engine\Drivers\D3DDrv\d3d_tpage.cpp
DEP_CPP_D3D_TP=\
	"..\..\..\include\BaseType.h"\
	".\Engine\Drivers\D3DDrv\d3d_TPage.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Engine\Drivers\D3DDrv\d3d_TPage.h
# End Source File
# Begin Source File

SOURCE=.\Engine\Drivers\D3DDrv\d3dcache.cpp
DEP_CPP_D3DCA=\
	"..\..\..\include\BaseType.h"\
	"..\..\..\include\Camera.h"\
	"..\..\..\include\jeChain.h"\
	"..\..\..\include\jePtrMgr.h"\
	"..\..\..\include\jeResource.h"\
	"..\..\..\include\jeStaticMesh.h"\
	"..\..\..\include\jeTypes.h"\
	"..\..\..\include\PixelFormat.h"\
	"..\..\..\include\Vec3d.h"\
	"..\..\..\include\VFILE.H"\
	"..\..\..\include\Xform3d.h"\
	".\Engine\Drivers\D3DDrv\d3d_DDMemMgr.h"\
	".\Engine\Drivers\D3DDrv\D3d_main.h"\
	".\Engine\Drivers\D3DDrv\d3d_THandle.h"\
	".\Engine\Drivers\D3DDrv\d3d_TPage.h"\
	".\Engine\Drivers\D3DDrv\D3dcache.h"\
	".\Engine\Drivers\Dcommon.h"\
	".\Support\mempool.h"\
	".\tsc.h"\
	{$(INCLUDE)}"d3dx.h"\
	{$(INCLUDE)}"d3dxcore.h"\
	{$(INCLUDE)}"d3dxerr.h"\
	{$(INCLUDE)}"d3dxmath.h"\
	{$(INCLUDE)}"d3dxmath.inl"\
	{$(INCLUDE)}"d3dxshapes.h"\
	{$(INCLUDE)}"d3dxsprite.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Engine\Drivers\D3DDrv\D3dcache.h
# End Source File
# Begin Source File

SOURCE=.\Engine\Drivers\D3DDrv\d3ddrv.cpp
DEP_CPP_D3DDR=\
	"..\..\..\include\BaseType.h"\
	"..\..\..\include\Camera.h"\
	"..\..\..\include\jeChain.h"\
	"..\..\..\include\jeMemAllocInfo.h"\
	"..\..\..\include\jePtrMgr.h"\
	"..\..\..\include\jeResource.h"\
	"..\..\..\include\jeStaticMesh.h"\
	"..\..\..\include\jeTypes.h"\
	"..\..\..\include\PixelFormat.h"\
	"..\..\..\include\Ram.h"\
	"..\..\..\include\Vec3d.h"\
	"..\..\..\include\VFILE.H"\
	"..\..\..\include\Xform3d.h"\
	".\Engine\Drivers\D3DDrv\d3d_DDMemMgr.h"\
	".\Engine\Drivers\D3DDrv\D3d_fx.h"\
	".\Engine\Drivers\D3DDrv\d3d_GSPAN.H"\
	".\Engine\Drivers\D3DDrv\D3d_main.h"\
	".\Engine\Drivers\D3DDrv\d3d_Pcache.h"\
	".\Engine\Drivers\D3DDrv\d3d_RENDER.H"\
	".\Engine\Drivers\D3DDrv\d3d_Scene.h"\
	".\Engine\Drivers\D3DDrv\d3d_THandle.h"\
	".\Engine\Drivers\D3DDrv\d3d_TPage.h"\
	".\Engine\Drivers\D3DDrv\D3dcache.h"\
	".\Engine\Drivers\D3DDrv\D3ddrv.h"\
	".\Engine\Drivers\Dcommon.h"\
	{$(INCLUDE)}"d3dx.h"\
	{$(INCLUDE)}"d3dxcore.h"\
	{$(INCLUDE)}"d3dxerr.h"\
	{$(INCLUDE)}"d3dxmath.h"\
	{$(INCLUDE)}"d3dxmath.inl"\
	{$(INCLUDE)}"d3dxshapes.h"\
	{$(INCLUDE)}"d3dxsprite.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Engine\Drivers\D3DDrv\D3ddrv.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\Engine\Drivers\Dcommon.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\Engine\BitmapList.c
DEP_CPP_BITMAPL=\
	"..\..\..\include\BaseType.h"\
	"..\..\..\include\BITMAP.H"\
	"..\..\..\include\Camera.h"\
	"..\..\..\include\Errorlog.h"\
	"..\..\..\include\jeChain.h"\
	"..\..\..\include\jeMemAllocInfo.h"\
	"..\..\..\include\jePtrMgr.h"\
	"..\..\..\include\jeResource.h"\
	"..\..\..\include\jeStaticMesh.h"\
	"..\..\..\include\jeTypes.h"\
	"..\..\..\include\PixelFormat.h"\
	"..\..\..\include\Ram.h"\
	"..\..\..\include\Vec3d.h"\
	"..\..\..\include\VFILE.H"\
	"..\..\..\include\Xform3d.h"\
	".\Bitmap\bitmap._h"\
	".\Engine\BitmapList.h"\
	".\Engine\Drivers\Dcommon.h"\
	".\list.h"\
	".\Support\mempool.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Engine\BitmapList.h
# End Source File
# Begin Source File

SOURCE=.\Engine\engine._h
# End Source File
# Begin Source File

SOURCE=.\Engine\engine.c
DEP_CPP_ENGIN=\
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
	"..\..\..\include\jePlane.h"\
	"..\..\..\include\jePoly.h"\
	"..\..\..\include\jePortal.h"\
	"..\..\..\include\jeProperty.h"\
	"..\..\..\include\jePtrMgr.h"\
	"..\..\..\include\jeResource.h"\
	"..\..\..\include\jeStaticMesh.h"\
	"..\..\..\include\jeTypes.h"\
	"..\..\..\include\jeVersion.h"\
	"..\..\..\include\jeVertArray.h"\
	"..\..\..\include\OBJECT.H"\
	"..\..\..\include\PixelFormat.h"\
	"..\..\..\include\Ram.h"\
	"..\..\..\include\Sound.h"\
	"..\..\..\include\Vec3d.h"\
	"..\..\..\include\VFILE.H"\
	"..\..\..\include\Xform3d.h"\
	".\Bitmap\bitmap._h"\
	".\Bitmap\Compression\image.h"\
	".\Bitmap\Compression\utility.h"\
	".\Engine\BitmapList.h"\
	".\Engine\Drivers\Dcommon.h"\
	".\Engine\engine._h"\
	".\list.h"\
	".\Support\cpu.h"\
	".\Support\jeAssert.h"\
	".\Support\log.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\..\include\ENGINE.H
# End Source File
# Begin Source File

SOURCE=.\Engine\fontbmp.c
# End Source File
# Begin Source File

SOURCE=.\Engine\jePolyMgr.c
DEP_CPP_JEPOL=\
	"..\..\..\include\BaseType.h"\
	"..\..\..\include\BITMAP.H"\
	"..\..\..\include\Camera.h"\
	"..\..\..\include\ENGINE.H"\
	"..\..\..\include\jeChain.h"\
	"..\..\..\include\jeMemAllocInfo.h"\
	"..\..\..\include\jePtrMgr.h"\
	"..\..\..\include\jeResource.h"\
	"..\..\..\include\jeStaticMesh.h"\
	"..\..\..\include\jeTypes.h"\
	"..\..\..\include\PixelFormat.h"\
	"..\..\..\include\Ram.h"\
	"..\..\..\include\Vec3d.h"\
	"..\..\..\include\VFILE.H"\
	"..\..\..\include\Xform3d.h"\
	".\Engine\Drivers\Dcommon.h"\
	".\Engine\jePolyMgr.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Engine\jePolyMgr.h
# End Source File
# Begin Source File

SOURCE=.\Engine\jeTexture.cpp
DEP_CPP_JETEX=\
	"..\..\..\include\BaseType.h"\
	"..\..\..\include\BITMAP.H"\
	"..\..\..\include\Camera.h"\
	"..\..\..\include\ENGINE.H"\
	"..\..\..\include\jeChain.h"\
	"..\..\..\include\jePtrMgr.h"\
	"..\..\..\include\jeResource.h"\
	"..\..\..\include\jeStaticMesh.h"\
	"..\..\..\include\jeTexture.h"\
	"..\..\..\include\jeTypes.h"\
	"..\..\..\include\PixelFormat.h"\
	"..\..\..\include\Vec3d.h"\
	"..\..\..\include\VFILE.H"\
	"..\..\..\include\Xform3d.h"\
	".\Engine\BitmapList.h"\
	".\Engine\Drivers\Dcommon.h"\
	".\Engine\engine._h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\..\include\jeTexture.h
# End Source File
# Begin Source File

SOURCE=.\Engine\splash_bmp.c
# End Source File
# End Group
# Begin Group "World"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\guWorld\jeBrush.c
DEP_CPP_JEBRU=\
	"..\..\..\include\ARRAY.H"\
	"..\..\..\include\BaseType.h"\
	"..\..\..\include\BITMAP.H"\
	"..\..\..\include\Camera.h"\
	"..\..\..\include\ENGINE.H"\
	"..\..\..\include\Errorlog.h"\
	"..\..\..\include\ExtBox.h"\
	"..\..\..\include\jeBrush.h"\
	"..\..\..\include\jeChain.h"\
	"..\..\..\include\jeFaceInfo.h"\
	"..\..\..\include\jeFrustum.h"\
	"..\..\..\include\jeGArray.h"\
	"..\..\..\include\jeMaterial.h"\
	"..\..\..\include\jeMemAllocInfo.h"\
	"..\..\..\include\jePlane.h"\
	"..\..\..\include\jePoly.h"\
	"..\..\..\include\jePortal.h"\
	"..\..\..\include\jeProperty.h"\
	"..\..\..\include\jePtrMgr.h"\
	"..\..\..\include\jeResource.h"\
	"..\..\..\include\jeStaticMesh.h"\
	"..\..\..\include\jeTypes.h"\
	"..\..\..\include\jeVertArray.h"\
	"..\..\..\include\OBJECT.H"\
	"..\..\..\include\PixelFormat.h"\
	"..\..\..\include\Ram.h"\
	"..\..\..\include\Sound.h"\
	"..\..\..\include\Vec3d.h"\
	"..\..\..\include\VFILE.H"\
	"..\..\..\include\Xform3d.h"\
	".\Engine\Drivers\Dcommon.h"\
	".\Engine\jePolyMgr.h"\
	".\guWorld\jeIndexPoly.h"\
	".\Support\log.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\..\include\jeBrush.h
# End Source File
# Begin Source File

SOURCE=.\guWorld\jeFaceInfo.c
DEP_CPP_JEFAC=\
	"..\..\..\include\ARRAY.H"\
	"..\..\..\include\BaseType.h"\
	"..\..\..\include\BITMAP.H"\
	"..\..\..\include\Camera.h"\
	"..\..\..\include\ENGINE.H"\
	"..\..\..\include\Errorlog.h"\
	"..\..\..\include\ExtBox.h"\
	"..\..\..\include\jeChain.h"\
	"..\..\..\include\jeFaceInfo.h"\
	"..\..\..\include\jeFrustum.h"\
	"..\..\..\include\jeGArray.h"\
	"..\..\..\include\jeMaterial.h"\
	"..\..\..\include\jeMemAllocInfo.h"\
	"..\..\..\include\jePlane.h"\
	"..\..\..\include\jePoly.h"\
	"..\..\..\include\jePortal.h"\
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
	
# End Source File
# Begin Source File

SOURCE=..\..\..\include\jeFaceInfo.h
# End Source File
# Begin Source File

SOURCE=.\guWorld\jeFrustum.c
DEP_CPP_JEFRU=\
	"..\..\..\include\BaseType.h"\
	"..\..\..\include\Camera.h"\
	"..\..\..\include\ExtBox.h"\
	"..\..\..\include\jeFrustum.h"\
	"..\..\..\include\jePlane.h"\
	"..\..\..\include\jeTypes.h"\
	"..\..\..\include\Vec3d.h"\
	"..\..\..\include\Xform3d.h"\
	".\Camera._h"\
	".\list.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\..\include\jeFrustum.h
# End Source File
# Begin Source File

SOURCE=.\guWorld\jeGArray.c
DEP_CPP_JEGAR=\
	"..\..\..\include\BaseType.h"\
	"..\..\..\include\Errorlog.h"\
	"..\..\..\include\jeGArray.h"\
	"..\..\..\include\jeMemAllocInfo.h"\
	"..\..\..\include\jePtrMgr.h"\
	"..\..\..\include\Ram.h"\
	"..\..\..\include\VFILE.H"\
	
# End Source File
# Begin Source File

SOURCE=..\..\..\include\jeGArray.h
# End Source File
# Begin Source File

SOURCE=.\guWorld\jeIndexPoly.c
DEP_CPP_JEIND=\
	"..\..\..\include\BaseType.h"\
	"..\..\..\include\jeMemAllocInfo.h"\
	"..\..\..\include\jeVertArray.h"\
	"..\..\..\include\Ram.h"\
	"..\..\..\include\Vec3d.h"\
	"..\..\..\include\VFILE.H"\
	".\guWorld\jeIndexPoly.h"\
	
# End Source File
# Begin Source File

SOURCE=.\guWorld\jeIndexPoly.h
# End Source File
# Begin Source File

SOURCE=.\guWorld\jeLight.c
DEP_CPP_JELIG=\
	"..\..\..\include\BaseType.h"\
	"..\..\..\include\Errorlog.h"\
	"..\..\..\include\jeLight.h"\
	"..\..\..\include\jeMemAllocInfo.h"\
	"..\..\..\include\jePtrMgr.h"\
	"..\..\..\include\jeTypes.h"\
	"..\..\..\include\Ram.h"\
	"..\..\..\include\Vec3d.h"\
	"..\..\..\include\VFILE.H"\
	
# End Source File
# Begin Source File

SOURCE=..\..\..\include\jeLight.h
# End Source File
# Begin Source File

SOURCE=.\guWorld\jeMaterial._h
# End Source File
# Begin Source File

SOURCE=.\guWorld\jeMaterial.c
DEP_CPP_JEMAT=\
	"..\..\..\include\ARRAY.H"\
	"..\..\..\include\BaseType.h"\
	"..\..\..\include\BITMAP.H"\
	"..\..\..\include\Camera.h"\
	"..\..\..\include\ENGINE.H"\
	"..\..\..\include\Errorlog.h"\
	"..\..\..\include\jeGArray.h"\
	"..\..\..\include\jeMaterial.h"\
	"..\..\..\include\jeMemAllocInfo.h"\
	"..\..\..\include\jePtrMgr.h"\
	"..\..\..\include\jeResource.h"\
	"..\..\..\include\jeTypes.h"\
	"..\..\..\include\PixelFormat.h"\
	"..\..\..\include\Ram.h"\
	"..\..\..\include\Vec3d.h"\
	"..\..\..\include\VFILE.H"\
	"..\..\..\include\Xform3d.h"\
	".\guWorld\jeMaterial._h"\
	".\Support\jePtrMgr._h"\
	".\Support\log.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\..\include\jeMaterial.h
# End Source File
# Begin Source File

SOURCE=.\guWorld\jeMaterialSpec.c
DEP_CPP_JEMATE=\
	"..\..\..\include\ARRAY.H"\
	"..\..\..\include\BaseType.h"\
	"..\..\..\include\BITMAP.H"\
	"..\..\..\include\Camera.h"\
	"..\..\..\include\ENGINE.H"\
	"..\..\..\include\Errorlog.h"\
	"..\..\..\include\jeChain.h"\
	"..\..\..\include\jeGArray.h"\
	"..\..\..\include\jeMaterial.h"\
	"..\..\..\include\jeMemAllocInfo.h"\
	"..\..\..\include\jePtrMgr.h"\
	"..\..\..\include\jeResource.h"\
	"..\..\..\include\jeStaticMesh.h"\
	"..\..\..\include\jeTexture.h"\
	"..\..\..\include\jeTypes.h"\
	"..\..\..\include\PixelFormat.h"\
	"..\..\..\include\Ram.h"\
	"..\..\..\include\Vec3d.h"\
	"..\..\..\include\VFILE.H"\
	"..\..\..\include\Xform3d.h"\
	".\Engine\Drivers\Dcommon.h"\
	".\guWorld\jeMaterial._h"\
	".\Support\log.h"\
	
# End Source File
# Begin Source File

SOURCE=.\guWorld\jeModel._h
# End Source File
# Begin Source File

SOURCE=.\guWorld\jeModel.c
DEP_CPP_JEMOD=\
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
	"..\..\..\include\jeStaticMesh.h"\
	"..\..\..\include\jeTypes.h"\
	"..\..\..\include\jeVertArray.h"\
	"..\..\..\include\OBJECT.H"\
	"..\..\..\include\PixelFormat.h"\
	"..\..\..\include\Ram.h"\
	"..\..\..\include\Sound.h"\
	"..\..\..\include\Vec3d.h"\
	"..\..\..\include\VFILE.H"\
	"..\..\..\include\Xform3d.h"\
	".\Bitmap\bitmap._h"\
	".\Engine\Drivers\Dcommon.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\..\include\JEMODEL.H
# End Source File
# Begin Source File

SOURCE=.\guWorld\jePlane.c
DEP_CPP_JEPLA=\
	"..\..\..\include\BaseType.h"\
	"..\..\..\include\Errorlog.h"\
	"..\..\..\include\ExtBox.h"\
	"..\..\..\include\jePlane.h"\
	"..\..\..\include\Vec3d.h"\
	"..\..\..\include\Xform3d.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\..\include\jePlane.h
# End Source File
# Begin Source File

SOURCE=.\guWorld\jePlaneArray.c
DEP_CPP_JEPLAN=\
	"..\..\..\include\BaseType.h"\
	"..\..\..\include\Errorlog.h"\
	"..\..\..\include\ExtBox.h"\
	"..\..\..\include\jeMemAllocInfo.h"\
	"..\..\..\include\jePlane.h"\
	"..\..\..\include\Ram.h"\
	"..\..\..\include\Vec3d.h"\
	"..\..\..\include\Xform3d.h"\
	".\guWorld\jePlaneArray.h"\
	
# End Source File
# Begin Source File

SOURCE=.\guWorld\jePlaneArray.h
# End Source File
# Begin Source File

SOURCE=.\guWorld\jePoly.c
DEP_CPP_JEPOLY=\
	"..\..\..\include\BaseType.h"\
	"..\..\..\include\Errorlog.h"\
	"..\..\..\include\ExtBox.h"\
	"..\..\..\include\jeMemAllocInfo.h"\
	"..\..\..\include\jePlane.h"\
	"..\..\..\include\jePoly.h"\
	"..\..\..\include\Ram.h"\
	"..\..\..\include\Vec3d.h"\
	"..\..\..\include\Xform3d.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\..\include\jePoly.h
# End Source File
# Begin Source File

SOURCE=.\guWorld\jePortal.c
DEP_CPP_JEPOR=\
	"..\..\..\include\BaseType.h"\
	"..\..\..\include\Camera.h"\
	"..\..\..\include\ExtBox.h"\
	"..\..\..\include\jeFrustum.h"\
	"..\..\..\include\jeMemAllocInfo.h"\
	"..\..\..\include\jePlane.h"\
	"..\..\..\include\jePoly.h"\
	"..\..\..\include\jePortal.h"\
	"..\..\..\include\jeProperty.h"\
	"..\..\..\include\jeTypes.h"\
	"..\..\..\include\Ram.h"\
	"..\..\..\include\Vec3d.h"\
	"..\..\..\include\Xform3d.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\..\include\jePortal.h
# End Source File
# Begin Source File

SOURCE=.\guWorld\jeStaticMesh.cpp
DEP_CPP_JESTA=\
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
	"..\..\..\include\jeStaticMesh.h"\
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
	".\Actor\BODY._H"\
	".\Actor\strblock.h"\
	".\UVMap.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\..\include\jeStaticMesh.h
# End Source File
# Begin Source File

SOURCE=.\guWorld\jeTexVec.c
DEP_CPP_JETEXV=\
	"..\..\..\include\BaseType.h"\
	"..\..\..\include\Errorlog.h"\
	"..\..\..\include\jeGArray.h"\
	"..\..\..\include\jeMemAllocInfo.h"\
	"..\..\..\include\jePtrMgr.h"\
	"..\..\..\include\Ram.h"\
	"..\..\..\include\Vec3d.h"\
	"..\..\..\include\VFILE.H"\
	".\guWorld\jeTexVec.h"\
	
# End Source File
# Begin Source File

SOURCE=.\guWorld\jeTexVec.h
# End Source File
# Begin Source File

SOURCE=.\guWorld\jeUserPoly.c
DEP_CPP_JEUSE=\
	"..\..\..\include\ARRAY.H"\
	"..\..\..\include\BaseType.h"\
	"..\..\..\include\BITMAP.H"\
	"..\..\..\include\Camera.h"\
	"..\..\..\include\ENGINE.H"\
	"..\..\..\include\ExtBox.h"\
	"..\..\..\include\jeChain.h"\
	"..\..\..\include\jeFrustum.h"\
	"..\..\..\include\jeMaterial.h"\
	"..\..\..\include\jeMemAllocInfo.h"\
	"..\..\..\include\jePlane.h"\
	"..\..\..\include\jePtrMgr.h"\
	"..\..\..\include\jeResource.h"\
	"..\..\..\include\jeStaticMesh.h"\
	"..\..\..\include\jeTypes.h"\
	"..\..\..\include\jeUserPoly.h"\
	"..\..\..\include\PixelFormat.h"\
	"..\..\..\include\Ram.h"\
	"..\..\..\include\Vec3d.h"\
	"..\..\..\include\VFILE.H"\
	"..\..\..\include\Xform3d.h"\
	".\Camera._h"\
	".\Engine\Drivers\Dcommon.h"\
	".\list.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\..\include\jeUserPoly.h
# End Source File
# Begin Source File

SOURCE=.\guWorld\jeVertArray.c
DEP_CPP_JEVER=\
	"..\..\..\include\BaseType.h"\
	"..\..\..\include\Errorlog.h"\
	"..\..\..\include\jeMemAllocInfo.h"\
	"..\..\..\include\jeVertArray.h"\
	"..\..\..\include\Ram.h"\
	"..\..\..\include\Vec3d.h"\
	"..\..\..\include\VFILE.H"\
	
# End Source File
# Begin Source File

SOURCE=..\..\..\include\jeVertArray.h
# End Source File
# Begin Source File

SOURCE=.\guWorld\jeWorld.c
DEP_CPP_JEWOR=\
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
	"..\..\..\include\jeStaticMesh.h"\
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
	".\Engine\Drivers\Dcommon.h"\
	".\guWorld\jeMaterial._h"\
	".\Support\jePtrMgr._h"\
	".\Support\log.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\..\include\JEWORLD.H
# End Source File
# Begin Source File

SOURCE=.\guWorld\visobject.c
DEP_CPP_VISOB=\
	"..\..\..\include\BaseType.h"\
	"..\..\..\include\BITMAP.H"\
	"..\..\..\include\Camera.h"\
	"..\..\..\include\ENGINE.H"\
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
	".\guWorld\visobject.h"\
	".\list.h"\
	".\Support\mempool.h"\
	
# End Source File
# Begin Source File

SOURCE=.\guWorld\visobject.h
# End Source File
# End Group
# Begin Group "Math"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Math\asmxform3d.c
DEP_CPP_ASMXF=\
	"..\..\..\include\BaseType.h"\
	"..\..\..\include\Vec3d.h"\
	"..\..\..\include\Xform3d.h"\
	".\Math\asmxform3d.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Math\asmxform3d.h
# End Source File
# Begin Source File

SOURCE=.\Math\Box.c
DEP_CPP_BOX_C=\
	"..\..\..\include\BaseType.h"\
	"..\..\..\include\Vec3d.h"\
	"..\..\..\include\Xform3d.h"\
	".\Math\Box.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Math\Box.h
# End Source File
# Begin Source File

SOURCE=.\Math\crc32.c
DEP_CPP_CRC32=\
	"..\..\..\include\BaseType.h"\
	".\Math\crc32.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Math\crc32.h
# End Source File
# Begin Source File

SOURCE=.\Math\ExtBox.c
DEP_CPP_EXTBO=\
	"..\..\..\include\BaseType.h"\
	"..\..\..\include\ExtBox.h"\
	"..\..\..\include\Vec3d.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\..\include\ExtBox.h
# End Source File
# Begin Source File

SOURCE=.\Math\jeRay.c
DEP_CPP_JERAY=\
	"..\..\..\include\BaseType.h"\
	"..\..\..\include\ExtBox.h"\
	"..\..\..\include\jePlane.h"\
	"..\..\..\include\jeRay.h"\
	"..\..\..\include\Vec3d.h"\
	"..\..\..\include\Xform3d.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\..\include\jeRay.h
# End Source File
# Begin Source File

SOURCE=.\Math\jeVec3d_Katmai.c
DEP_CPP_JEVEC=\
	"..\..\..\include\BaseType.h"\
	"..\..\..\include\Vec3d.h"\
	".\Math\jeVec3d_Katmai.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Math\jeVec3d_Katmai.h
# End Source File
# Begin Source File

SOURCE=.\Math\ObjectPos.c
DEP_CPP_OBJEC=\
	"..\..\..\include\BaseType.h"\
	"..\..\..\include\ObjectPos.h"\
	"..\..\..\include\Vec3d.h"\
	"..\..\..\include\Xform3d.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\..\include\ObjectPos.h
# End Source File
# Begin Source File

SOURCE=.\Math\quatern.c
DEP_CPP_QUATE=\
	"..\..\..\include\BaseType.h"\
	"..\..\..\include\QUATERN.H"\
	"..\..\..\include\Vec3d.h"\
	"..\..\..\include\Xform3d.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\..\include\QUATERN.H
# End Source File
# Begin Source File

SOURCE=.\Math\vec2d.c
DEP_CPP_VEC2D=\
	"..\..\..\include\BaseType.h"\
	".\Math\vec2d.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Math\vec2d.h
# End Source File
# Begin Source File

SOURCE=.\Math\Vec3d.c
DEP_CPP_VEC3D=\
	"..\..\..\include\BaseType.h"\
	"..\..\..\include\Vec3d.h"\
	".\Math\jeVec3d_Katmai.h"\
	".\Support\cpu.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\..\include\Vec3d.h
# End Source File
# Begin Source File

SOURCE=.\Math\Xform3d.c
DEP_CPP_XFORM=\
	"..\..\..\include\BaseType.h"\
	"..\..\..\include\Vec3d.h"\
	"..\..\..\include\Xform3d.h"\
	".\Math\asmxform3d.h"\
	".\Support\cpu.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\..\include\Xform3d.h
# End Source File
# End Group
# Begin Group "Physics"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Physics\part.c
DEP_CPP_PART_=\
	"..\..\..\include\BaseType.h"\
	"..\..\..\include\jeMemAllocInfo.h"\
	"..\..\..\include\Ram.h"\
	"..\..\..\include\Vec3d.h"\
	".\Physics\part.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Physics\part.h
# End Source File
# Begin Source File

SOURCE=.\Physics\spring.c
DEP_CPP_SPRIN=\
	"..\..\..\include\BaseType.h"\
	"..\..\..\include\jeMemAllocInfo.h"\
	"..\..\..\include\Ram.h"\
	"..\..\..\include\Vec3d.h"\
	".\Physics\part.h"\
	".\Physics\spring.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Physics\spring.h
# End Source File
# End Group
# Begin Group "Support"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Support\array.c
DEP_CPP_ARRAY=\
	"..\..\..\include\ARRAY.H"\
	"..\..\..\include\BaseType.h"\
	"..\..\..\include\Errorlog.h"\
	"..\..\..\include\jeMemAllocInfo.h"\
	"..\..\..\include\jePtrMgr.h"\
	"..\..\..\include\Ram.h"\
	"..\..\..\include\Vec3d.h"\
	"..\..\..\include\VFILE.H"\
	"..\..\..\include\Xform3d.h"\
	".\Support\log.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\..\include\ARRAY.H
# End Source File
# Begin Source File

SOURCE=.\Support\cpu.c
DEP_CPP_CPU_C=\
	"..\..\..\include\BaseType.h"\
	"..\..\..\include\Camera.h"\
	"..\..\..\include\Errorlog.h"\
	"..\..\..\include\jeChain.h"\
	"..\..\..\include\jePtrMgr.h"\
	"..\..\..\include\jeResource.h"\
	"..\..\..\include\jeStaticMesh.h"\
	"..\..\..\include\jeTypes.h"\
	"..\..\..\include\PixelFormat.h"\
	"..\..\..\include\Vec3d.h"\
	"..\..\..\include\VFILE.H"\
	"..\..\..\include\Xform3d.h"\
	".\Engine\Drivers\Dcommon.h"\
	".\Support\cpu.h"\
	".\Support\log.h"\
	".\Support\ThreadLog.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Support\cpu.h
# End Source File
# Begin Source File

SOURCE=.\Support\Errorlog.c
DEP_CPP_ERROR=\
	"..\..\..\include\BaseType.h"\
	"..\..\..\include\Errorlog.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\..\include\Errorlog.h
# End Source File
# Begin Source File

SOURCE=.\Support\iaperf.h
# End Source File
# Begin Source File

SOURCE=.\Support\jeAssert.c
DEP_CPP_JEASS=\
	"..\..\..\include\BaseType.h"\
	".\Support\jeAssert.h"\
	
# End Source File
# Begin Source File

SOURCE=Support\jeAssert.h
# End Source File
# Begin Source File

SOURCE=.\Support\jeChain.c
DEP_CPP_JECHA=\
	"..\..\..\include\BaseType.h"\
	"..\..\..\include\jeChain.h"\
	"..\..\..\include\jeMemAllocInfo.h"\
	"..\..\..\include\jePtrMgr.h"\
	"..\..\..\include\Ram.h"\
	"..\..\..\include\VFILE.H"\
	
# End Source File
# Begin Source File

SOURCE=..\..\..\include\jeChain.h
# End Source File
# Begin Source File

SOURCE=.\Support\jeMemAllocInfo._h
# End Source File
# Begin Source File

SOURCE=.\Support\jeMemAllocInfo.c
DEP_CPP_JEMEM=\
	"..\..\..\include\BaseType.h"\
	"..\..\..\include\jeMemAllocInfo.h"\
	"..\..\..\include\Ram.h"\
	".\Support\cpu.h"\
	".\Support\jeMemAllocInfo._h"\
	
# End Source File
# Begin Source File

SOURCE=.\Support\jeNameMgr.c
DEP_CPP_JENAM=\
	"..\..\..\include\BaseType.h"\
	"..\..\..\include\jeChain.h"\
	"..\..\..\include\jeMemAllocInfo.h"\
	"..\..\..\include\jePtrMgr.h"\
	"..\..\..\include\Ram.h"\
	"..\..\..\include\VFILE.H"\
	
# End Source File
# Begin Source File

SOURCE=..\..\..\include\jeNameMgr.h
# End Source File
# Begin Source File

SOURCE=.\Support\jeProperty.c
DEP_CPP_JEPRO=\
	"..\..\..\include\BaseType.h"\
	"..\..\..\include\Errorlog.h"\
	"..\..\..\include\jeMemAllocInfo.h"\
	"..\..\..\include\jeProperty.h"\
	"..\..\..\include\Ram.h"\
	"..\..\..\include\Vec3d.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\..\include\jeProperty.h
# End Source File
# Begin Source File

SOURCE=.\Support\jePtrMgr._h
# End Source File
# Begin Source File

SOURCE=.\Support\jePtrMgr.c
DEP_CPP_JEPTR=\
	"..\..\..\include\BaseType.h"\
	"..\..\..\include\Errorlog.h"\
	"..\..\..\include\jeMemAllocInfo.h"\
	"..\..\..\include\jePtrMgr.h"\
	"..\..\..\include\Ram.h"\
	"..\..\..\include\VFILE.H"\
	".\Support\jePtrMgr._h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\..\include\jePtrMgr.h
# End Source File
# Begin Source File

SOURCE=.\Support\jeResource.c
DEP_CPP_JERES=\
	"..\..\..\include\ARRAY.H"\
	"..\..\..\include\BaseType.h"\
	"..\..\..\include\BITMAP.H"\
	"..\..\..\include\Camera.h"\
	"..\..\..\include\ENGINE.H"\
	"..\..\..\include\Errorlog.h"\
	"..\..\..\include\jeChain.h"\
	"..\..\..\include\jeMaterial.h"\
	"..\..\..\include\jeMemAllocInfo.h"\
	"..\..\..\include\jePtrMgr.h"\
	"..\..\..\include\jeResource.h"\
	"..\..\..\include\jeTypes.h"\
	"..\..\..\include\PixelFormat.h"\
	"..\..\..\include\Ram.h"\
	"..\..\..\include\Vec3d.h"\
	"..\..\..\include\VFILE.H"\
	"..\..\..\include\Xform3d.h"\
	".\Support\log.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\..\include\jeResource.h
# End Source File
# Begin Source File

SOURCE=.\Support\log.c
DEP_CPP_LOG_C=\
	"..\..\..\include\BaseType.h"\
	"..\..\..\include\Vec3d.h"\
	"..\..\..\include\Xform3d.h"\
	".\Support\log.h"\
	".\Support\ThreadLog.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Support\log.h
# End Source File
# Begin Source File

SOURCE=.\Support\mempool.c
DEP_CPP_MEMPO=\
	"..\..\..\include\BaseType.h"\
	"..\..\..\include\jeMemAllocInfo.h"\
	"..\..\..\include\Ram.h"\
	".\Support\mempool.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Support\mempool.h
# End Source File
# Begin Source File

SOURCE=.\Support\Ram.c
DEP_CPP_RAM_C=\
	"..\..\..\include\BaseType.h"\
	"..\..\..\include\jeMemAllocInfo.h"\
	"..\..\..\include\Ram.h"\
	"..\..\..\include\Vec3d.h"\
	"..\..\..\include\Xform3d.h"\
	".\report.h"\
	".\Support\log.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\..\include\Ram.h
# End Source File
# Begin Source File

SOURCE=.\Support\ThreadLog.c
DEP_CPP_THREA=\
	"..\..\..\include\BaseType.h"\
	"..\..\..\include\VFILE.H"\
	".\Support\ThreadLog.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Support\ThreadLog.h
# End Source File
# Begin Source File

SOURCE=.\Support\ThreadQueue.c
DEP_CPP_THREAD=\
	"..\..\..\include\BaseType.h"\
	"..\..\..\include\Errorlog.h"\
	"..\..\..\include\jeMemAllocInfo.h"\
	"..\..\..\include\Ram.h"\
	"..\..\..\include\Vec3d.h"\
	"..\..\..\include\Xform3d.h"\
	".\Support\log.h"\
	".\Support\mempool.h"\
	".\Support\ThreadLog.h"\
	".\Support\ThreadQueue.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Support\ThreadQueue.h
# End Source File
# Begin Source File

SOURCE=.\Support\Util.c
DEP_CPP_UTIL_=\
	"..\..\..\include\BaseType.h"\
	"..\..\..\include\jeMemAllocInfo.h"\
	"..\..\..\include\Ram.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Support\Util.h
# End Source File
# End Group
# Begin Group "VFile"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\VFile\dirtree.c
DEP_CPP_DIRTR=\
	"..\..\..\include\BaseType.h"\
	"..\..\..\include\Errorlog.h"\
	"..\..\..\include\jeMemAllocInfo.h"\
	"..\..\..\include\Ram.h"\
	"..\..\..\include\VFILE.H"\
	".\VFile\dirtree.h"\
	
# End Source File
# Begin Source File

SOURCE=.\VFile\dirtree.h
# End Source File
# Begin Source File

SOURCE=.\VFile\fsdos.c
DEP_CPP_FSDOS=\
	"..\..\..\include\BaseType.h"\
	"..\..\..\include\jeMemAllocInfo.h"\
	"..\..\..\include\Ram.h"\
	"..\..\..\include\VFILE.H"\
	".\VFile\fsdos.h"\
	".\VFile\VFile._h"\
	
# End Source File
# Begin Source File

SOURCE=.\VFile\fsdos.h
# End Source File
# Begin Source File

SOURCE=.\VFile\fsfakenet.c
DEP_CPP_FSFAK=\
	"..\..\..\include\BaseType.h"\
	"..\..\..\include\Errorlog.h"\
	"..\..\..\include\jeMemAllocInfo.h"\
	"..\..\..\include\Ram.h"\
	"..\..\..\include\Vec3d.h"\
	"..\..\..\include\VFILE.H"\
	"..\..\..\include\Xform3d.h"\
	".\Support\log.h"\
	".\Support\ThreadQueue.h"\
	".\tsc.h"\
	".\VFile\fsfakenet.h"\
	".\VFile\VFile._h"\
	
# End Source File
# Begin Source File

SOURCE=.\VFile\fsfakenet.h
# End Source File
# Begin Source File

SOURCE=.\VFile\Fsinet.cpp
DEP_CPP_FSINE=\
	"..\..\..\include\BaseType.h"\
	"..\..\..\include\Errorlog.h"\
	"..\..\..\include\jeMemAllocInfo.h"\
	"..\..\..\include\Ram.h"\
	"..\..\..\include\Vec3d.h"\
	"..\..\..\include\VFILE.H"\
	"..\..\..\include\Xform3d.h"\
	".\Support\log.h"\
	".\Support\ThreadLog.h"\
	".\Support\ThreadQueue.h"\
	".\VFile\fsINet.h"\
	".\VFile\VFile._h"\
	
# End Source File
# Begin Source File

SOURCE=.\VFile\fsINet.h
# End Source File
# Begin Source File

SOURCE=.\VFile\fslz.c
DEP_CPP_FSLZ_=\
	"..\..\..\include\BaseType.h"\
	"..\..\..\include\Errorlog.h"\
	"..\..\..\include\jeMemAllocInfo.h"\
	"..\..\..\include\Ram.h"\
	"..\..\..\include\Vec3d.h"\
	"..\..\..\include\VFILE.H"\
	"..\..\..\include\Xform3d.h"\
	".\Bitmap\Compression\lza.h"\
	".\Support\log.h"\
	".\Support\ThreadQueue.h"\
	".\VFile\fslz.H"\
	".\VFile\VFile._h"\
	
# End Source File
# Begin Source File

SOURCE=.\VFile\fslz.H
# End Source File
# Begin Source File

SOURCE=.\VFile\Fsmemory.c
DEP_CPP_FSMEM=\
	"..\..\..\include\BaseType.h"\
	"..\..\..\include\jeMemAllocInfo.h"\
	"..\..\..\include\Ram.h"\
	"..\..\..\include\VFILE.H"\
	".\VFile\Fsmemory.h"\
	".\VFile\VFile._h"\
	
# End Source File
# Begin Source File

SOURCE=.\VFile\Fsmemory.h
# End Source File
# Begin Source File

SOURCE=.\VFile\fsvfs.c
DEP_CPP_FSVFS=\
	"..\..\..\include\BaseType.h"\
	"..\..\..\include\Errorlog.h"\
	"..\..\..\include\jeMemAllocInfo.h"\
	"..\..\..\include\Ram.h"\
	"..\..\..\include\VFILE.H"\
	".\VFile\dirtree.h"\
	".\VFile\fsvfs.h"\
	".\VFile\VFile._h"\
	
# End Source File
# Begin Source File

SOURCE=.\VFile\fsvfs.h
# End Source File
# Begin Source File

SOURCE=.\VFile\VFile._h
# End Source File
# Begin Source File

SOURCE=.\VFile\vfile.c
DEP_CPP_VFILE=\
	"..\..\..\include\BaseType.h"\
	"..\..\..\include\Errorlog.h"\
	"..\..\..\include\jeMemAllocInfo.h"\
	"..\..\..\include\Ram.h"\
	"..\..\..\include\Vec3d.h"\
	"..\..\..\include\VFILE.H"\
	"..\..\..\include\Xform3d.h"\
	".\list.h"\
	".\Support\log.h"\
	".\Support\mempool.h"\
	".\Support\ThreadQueue.h"\
	".\VFile\fsdos.h"\
	".\VFile\fsfakenet.h"\
	".\VFile\fsINet.h"\
	".\VFile\fslz.H"\
	".\VFile\Fsmemory.h"\
	".\VFile\fsvfs.h"\
	".\VFile\VFile._h"\
	
NODEP_CPP_VFILE=\
	".\VFile\FSBeos.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\..\include\VFILE.H
# End Source File
# End Group
# Begin Group "Terrain"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Terrain\quad.c
DEP_CPP_QUAD_=\
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
	"..\..\..\include\jeStaticMesh.h"\
	"..\..\..\include\jeTypes.h"\
	"..\..\..\include\jeUserPoly.h"\
	"..\..\..\include\jeVertArray.h"\
	"..\..\..\include\JEWORLD.H"\
	"..\..\..\include\OBJECT.H"\
	"..\..\..\include\PixelFormat.h"\
	"..\..\..\include\Ram.h"\
	"..\..\..\include\Sound.h"\
	"..\..\..\include\TERRAIN.H"\
	"..\..\..\include\Vec3d.h"\
	"..\..\..\include\VFILE.H"\
	"..\..\..\include\Xform3d.h"\
	".\Bitmap\bitmap._h"\
	".\Engine\BitmapList.h"\
	".\Engine\Drivers\Dcommon.h"\
	".\Engine\engine._h"\
	".\list.h"\
	".\report.h"\
	".\Support\cpu.h"\
	".\Support\log.h"\
	".\Support\mempool.h"\
	".\Terrain\quad.h"\
	".\Terrain\terrain._h"\
	".\timer.h"\
	".\tsc.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Terrain\quad.h
# End Source File
# Begin Source File

SOURCE=.\Terrain\terrain._h

!IF  "$(CFG)" == "JetEngine - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "JetEngine - Win32 Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Terrain\terrain.c
DEP_CPP_TERRA=\
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
	"..\..\..\include\TERRAIN.H"\
	"..\..\..\include\Vec3d.h"\
	"..\..\..\include\VFILE.H"\
	"..\..\..\include\Xform3d.h"\
	"..\..\Tools\Editor\EditMsg.h"\
	".\Terrain\quad.h"\
	".\Terrain\terrain._h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\..\include\TERRAIN.H
# End Source File
# End Group
# Begin Group "Particle"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Particle\jeParticle.c
DEP_CPP_JEPAR=\
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
	"..\..\..\include\jeMemAllocInfo.h"\
	"..\..\..\include\JEMODEL.H"\
	"..\..\..\include\jeParticle.h"\
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
	"..\..\..\include\Ram.h"\
	"..\..\..\include\Sound.h"\
	"..\..\..\include\Vec3d.h"\
	"..\..\..\include\VFILE.H"\
	"..\..\..\include\Xform3d.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\..\include\jeParticle.h
# End Source File
# End Group
# Begin Group "Mp3Mgr"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Mp3Mgr\Mp3Mgr.c
DEP_CPP_MP3MG=\
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
	"..\..\..\include\Mp3Mgr.h"\
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
	".\Mp3Mgr\Mp3Mgr_h.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\..\include\Mp3Mgr.h
# End Source File
# Begin Source File

SOURCE=.\Mp3Mgr\Mp3Mgr_h.h
# End Source File
# End Group
# Begin Group "VideoMgr"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\VideoMgr\VideoMgr.c
DEP_CPP_VIDEO=\
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
	".\VideoMgr\VideoMgr_h.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\..\include\VideoMgr.h
# End Source File
# Begin Source File

SOURCE=.\VideoMgr\VideoMgr_h.h
# End Source File
# End Group
# Begin Group "Sound"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\OGGStream.c
DEP_CPP_OGGST=\
	"..\..\..\include\BaseType.h"\
	"..\..\..\include\jeMemAllocInfo.h"\
	"..\..\..\include\Ram.h"\
	"..\..\..\include\VFILE.H"\
	"..\External\VorbisSDK\include\ogg\ogg.h"\
	"..\External\VorbisSDK\include\ogg\os_types.h"\
	"..\External\VorbisSDK\include\vorbis\codec.h"\
	"..\External\VorbisSDK\include\vorbis\vorbisfile.h"\
	".\OGGStream.h"\
	
# End Source File
# Begin Source File

SOURCE=.\OGGStream.h
# End Source File
# Begin Source File

SOURCE=.\Sound.c
DEP_CPP_SOUND=\
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
	"..\..\..\include\Mp3Mgr.h"\
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
	"..\External\VorbisSDK\include\ogg\ogg.h"\
	"..\External\VorbisSDK\include\ogg\os_types.h"\
	"..\External\VorbisSDK\include\vorbis\codec.h"\
	"..\External\VorbisSDK\include\vorbis\vorbisfile.h"\
	".\OGGStream.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\..\include\Sound.h
# End Source File
# Begin Source File

SOURCE=.\Sound3d.c
DEP_CPP_SOUND3=\
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
	"..\..\..\include\Sound3d.h"\
	"..\..\..\include\Vec3d.h"\
	"..\..\..\include\VFILE.H"\
	"..\..\..\include\Xform3d.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\..\include\Sound3d.h
# End Source File
# End Group
# Begin Source File

SOURCE=..\..\..\include\BaseType.h
# End Source File
# Begin Source File

SOURCE=.\Camera._h
# End Source File
# Begin Source File

SOURCE=.\Camera.c
DEP_CPP_CAMER=\
	"..\..\..\include\BaseType.h"\
	"..\..\..\include\Camera.h"\
	"..\..\..\include\Errorlog.h"\
	"..\..\..\include\jeMemAllocInfo.h"\
	"..\..\..\include\jeTypes.h"\
	"..\..\..\include\Ram.h"\
	"..\..\..\include\Vec3d.h"\
	"..\..\..\include\Xform3d.h"\
	".\Camera._h"\
	".\list.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\..\include\Camera.h
# End Source File
# Begin Source File

SOURCE=.\CSNetMgr.c
DEP_CPP_CSNET=\
	"..\..\..\include\BaseType.h"\
	"..\..\..\include\Errorlog.h"\
	"..\..\..\include\jeMemAllocInfo.h"\
	"..\..\..\include\Ram.h"\
	".\CSNetMgr.h"\
	".\Netplay.h"\
	
# End Source File
# Begin Source File

SOURCE=.\CSNetMgr.h
# End Source File
# Begin Source File

SOURCE=.\Jet.h
# End Source File
# Begin Source File

SOURCE=.\jet.rc
# End Source File
# Begin Source File

SOURCE=..\..\..\include\jeTypes.h
# End Source File
# Begin Source File

SOURCE=..\..\..\include\jeVersion.h
# End Source File
# Begin Source File

SOURCE=.\list.c
DEP_CPP_LIST_=\
	"..\..\..\include\BaseType.h"\
	"..\..\..\include\jeMemAllocInfo.h"\
	"..\..\..\include\Ram.h"\
	".\list.h"\
	".\Math\crc32.h"\
	".\Support\cpu.h"\
	".\Support\mempool.h"\
	".\timer.h"\
	".\tsc.h"\
	
# End Source File
# Begin Source File

SOURCE=.\list.h
# End Source File
# Begin Source File

SOURCE=.\Netplay.c
DEP_CPP_NETPL=\
	".\Netplay.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Netplay.h
# End Source File
# Begin Source File

SOURCE=.\object.c
DEP_CPP_OBJECT=\
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
	".\Math\crc32.h"\
	".\Support\log.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\..\include\OBJECT.H
# End Source File
# Begin Source File

SOURCE=..\..\..\include\PixelFormat.h
# End Source File
# Begin Source File

SOURCE=.\Ptrtypes.h
# End Source File
# Begin Source File

SOURCE=.\report.c
DEP_CPP_REPOR=\
	"..\..\..\include\BaseType.h"\
	"..\..\..\include\Vec3d.h"\
	"..\..\..\include\Xform3d.h"\
	".\report.h"\
	".\Support\log.h"\
	
# End Source File
# Begin Source File

SOURCE=.\report.h
# End Source File
# Begin Source File

SOURCE=.\resource.h
# End Source File
# Begin Source File

SOURCE=.\Tclip.c
DEP_CPP_TCLIP=\
	"..\..\..\include\BaseType.h"\
	"..\..\..\include\BITMAP.H"\
	"..\..\..\include\Camera.h"\
	"..\..\..\include\ENGINE.H"\
	"..\..\..\include\Errorlog.h"\
	"..\..\..\include\jeChain.h"\
	"..\..\..\include\jeMemAllocInfo.h"\
	"..\..\..\include\jePtrMgr.h"\
	"..\..\..\include\jeResource.h"\
	"..\..\..\include\jeStaticMesh.h"\
	"..\..\..\include\jeTypes.h"\
	"..\..\..\include\PixelFormat.h"\
	"..\..\..\include\Ram.h"\
	"..\..\..\include\Vec3d.h"\
	"..\..\..\include\VFILE.H"\
	"..\..\..\include\Xform3d.h"\
	".\Bitmap\bitmap._h"\
	".\Engine\Drivers\Dcommon.h"\
	".\list.h"\
	".\Support\cpu.h"\
	".\tclip.h"\
	".\timer.h"\
	".\tsc.h"\
	
# End Source File
# Begin Source File

SOURCE=.\tclip.h
# End Source File
# Begin Source File

SOURCE=.\timer.c
DEP_CPP_TIMER=\
	"..\..\..\include\BaseType.h"\
	".\Support\cpu.h"\
	".\timer.h"\
	".\tsc.h"\
	
# End Source File
# Begin Source File

SOURCE=.\timer.h
# End Source File
# Begin Source File

SOURCE=.\tsc.c
DEP_CPP_TSC_C=\
	"..\..\..\include\BaseType.h"\
	"..\..\..\include\Vec3d.h"\
	"..\..\..\include\Xform3d.h"\
	".\Support\cpu.h"\
	".\Support\log.h"\
	".\tsc.h"\
	
# End Source File
# Begin Source File

SOURCE=.\tsc.h
# End Source File
# Begin Source File

SOURCE=.\uvmap.c
DEP_CPP_UVMAP=\
	"..\..\..\include\BaseType.h"\
	"..\..\..\include\jeTypes.h"\
	"..\..\..\include\Vec3d.h"\
	"..\..\..\include\Xform3d.h"\
	".\UVMap.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\..\include\UVMAP.H
# End Source File
# End Group
# End Target
# End Project
