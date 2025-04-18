# Microsoft Developer Studio Project File - Name="GenesisDLL" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=GenesisDLL - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "GenesisDLL.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "GenesisDLL.mak" CFG="GenesisDLL - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "GenesisDLL - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "GenesisDLL - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/Genesis20/Source", VBRBAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "GenesisDLL - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "ReleaseDLL"
# PROP Intermediate_Dir "ReleaseDLL"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "GENESISDLL_EXPORTS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /X /I "..\SDK\DX6SDK\Include" /I "..\Source" /I "World" /I "Engine" /I "Engine\Drivers" /I "Actor" /I "BSP" /I "Math" /I "Entities" /I "Support" /I "Physics" /I "VFile" /I "Bitmap" /I "Bitmap\Compression" /I "..\MSDev60\Include" /I "..\MSDev60\MFC\Include" /I "guWorld" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "BUILDGENESIS" /D "GENESISDLLVERSION" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 libcmt.lib kernel32.lib user32.lib gdi32.lib oldnames.lib ole32.lib urlmon.lib uuid.lib winmm.lib /nologo /dll /machine:I386 /nodefaultlib /out:"ReleaseDLL/Genesis.dll"
# Begin Custom Build
IntDir=.\ReleaseDLL
OutDir=.\ReleaseDLL
InputPath=.\ReleaseDLL\Genesis.dll
SOURCE="$(InputPath)"

"$(OUTDIR)\genesisi.lib" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	lib /OUT:$(OUTDIR)\genesisi.lib $(IntDir)\ramdll.obj $(OUTDIR)\genesis.lib

# End Custom Build

!ELSEIF  "$(CFG)" == "GenesisDLL - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "GenesisDLL___Win32_Debug"
# PROP BASE Intermediate_Dir "GenesisDLL___Win32_Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "DebugDLL"
# PROP Intermediate_Dir "DebugDLL"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "GENESISDLL_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /X /I "..\SDK\DX6SDK\Include" /I "..\Source" /I "World" /I "Engine" /I "Engine\Drivers" /I "Actor" /I "BSP" /I "Math" /I "Entities" /I "Support" /I "Physics" /I "VFile" /I "Bitmap" /I "Bitmap\Compression" /I "..\MSDev60\Include" /I "..\MSDev60\MFC\Include" /I "guWorld" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "BUILDGENESIS" /D "GENESISDLLVERSION" /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 libcmtd.lib kernel32.lib user32.lib gdi32.lib oldnames.lib ole32.lib urlmon.lib uuid.lib winmm.lib /nologo /dll /debug /machine:I386 /nodefaultlib /out:"DebugDLL/Genesis.dll" /pdbtype:sept
# Begin Custom Build
IntDir=.\DebugDLL
OutDir=.\DebugDLL
InputPath=.\DebugDLL\Genesis.dll
SOURCE="$(InputPath)"

"$(OUTDIR)\genesisid.lib" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	lib /OUT:$(OUTDIR)\genesisid.lib $(IntDir)\ramdll.obj $(OUTDIR)\genesis.lib

# End Custom Build

!ENDIF 

# Begin Target

# Name "GenesisDLL - Win32 Release"
# Name "GenesisDLL - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter ""
# Begin Group "Actor"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Actor\actor.c
# End Source File
# Begin Source File

SOURCE=.\Actor\actor.h
# End Source File
# Begin Source File

SOURCE=.\Actor\body.c
# End Source File
# Begin Source File

SOURCE=.\Actor\body.h
# End Source File
# Begin Source File

SOURCE=.\Actor\bodyinst.c
# End Source File
# Begin Source File

SOURCE=.\Actor\bodyinst.h
# End Source File
# Begin Source File

SOURCE=.\Actor\motion.c
# End Source File
# Begin Source File

SOURCE=.\Actor\motion.h
# End Source File
# Begin Source File

SOURCE=.\Actor\path.c
# End Source File
# Begin Source File

SOURCE=.\Actor\path.h
# End Source File
# Begin Source File

SOURCE=.\Actor\pose.c
# End Source File
# Begin Source File

SOURCE=.\Actor\pose.h
# End Source File
# Begin Source File

SOURCE=.\Actor\puppet.c
# End Source File
# Begin Source File

SOURCE=.\Actor\puppet.h
# End Source File
# Begin Source File

SOURCE=.\Actor\QKFrame.c
# End Source File
# Begin Source File

SOURCE=.\Actor\QKFrame.h
# End Source File
# Begin Source File

SOURCE=.\Actor\strblock.c
# End Source File
# Begin Source File

SOURCE=.\Actor\strblock.h
# End Source File
# Begin Source File

SOURCE=.\Actor\tkarray.c
# End Source File
# Begin Source File

SOURCE=.\Actor\tkarray.h
# End Source File
# Begin Source File

SOURCE=.\Actor\tkevents.c
# End Source File
# Begin Source File

SOURCE=.\Actor\tkevents.h
# End Source File
# Begin Source File

SOURCE=.\Actor\vkframe.c
# End Source File
# Begin Source File

SOURCE=.\Actor\vkframe.h
# End Source File
# Begin Source File

SOURCE=.\Actor\XFArray.c
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

SOURCE=.\Bitmap\Compression\image.h
# End Source File
# Begin Source File

SOURCE=.\Bitmap\Compression\palcreate.c
# End Source File
# Begin Source File

SOURCE=.\Bitmap\Compression\palcreate.h
# End Source File
# Begin Source File

SOURCE=.\Bitmap\Compression\palettize.c
# End Source File
# Begin Source File

SOURCE=.\Bitmap\Compression\palettize.h
# End Source File
# Begin Source File

SOURCE=.\Bitmap\Compression\paloptimize.c
# End Source File
# Begin Source File

SOURCE=.\Bitmap\Compression\paloptimize.h
# End Source File
# Begin Source File

SOURCE=.\Bitmap\Compression\utility.h
# End Source File
# Begin Source File

SOURCE=.\Bitmap\Compression\yuv.c
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
# End Source File
# Begin Source File

SOURCE=.\Bitmap\bitmap.h
# End Source File
# Begin Source File

SOURCE=.\Bitmap\bitmap_blitdata.c
# End Source File
# Begin Source File

SOURCE=.\Bitmap\bitmap_blitdata.h
# End Source File
# Begin Source File

SOURCE=.\Bitmap\bitmap_gamma.c
# End Source File
# Begin Source File

SOURCE=.\Bitmap\bitmap_gamma.h
# End Source File
# Begin Source File

SOURCE=.\Bitmap\pixelformat.c
# End Source File
# Begin Source File

SOURCE=.\Bitmap\pixelformat.h
# End Source File
# End Group
# Begin Group "Engine"

# PROP Default_Filter ""
# Begin Group "Logo"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Engine\Logo\A_CORONA.c
# End Source File
# Begin Source File

SOURCE=.\Engine\Logo\A_STREAK.c
# End Source File
# Begin Source File

SOURCE=.\Engine\Logo\CORONA.c
# End Source File
# Begin Source File

SOURCE=.\Engine\Logo\electric.c
# End Source File
# Begin Source File

SOURCE=.\Engine\Logo\electric.h
# End Source File
# Begin Source File

SOURCE=.\Engine\Logo\logo.c
# End Source File
# Begin Source File

SOURCE=.\Engine\Logo\LogoActor.c
# End Source File
# Begin Source File

SOURCE=.\Engine\Logo\streak.c
# End Source File
# Begin Source File

SOURCE=.\Engine\Logo\WebUrl.c
# End Source File
# End Group
# Begin Source File

SOURCE=.\Engine\BitmapList.c
# End Source File
# Begin Source File

SOURCE=.\Engine\BitmapList.h
# End Source File
# Begin Source File

SOURCE=.\Engine\engine.c
# End Source File
# Begin Source File

SOURCE=.\Engine\engine.h
# End Source File
# Begin Source File

SOURCE=.\Engine\fontbmp.c
# End Source File
# Begin Source File

SOURCE=.\Engine\System.c
# End Source File
# Begin Source File

SOURCE=.\Engine\System.h
# End Source File
# End Group
# Begin Group "Entities"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Entities\Entities.c
# End Source File
# Begin Source File

SOURCE=.\Entities\Entities.h
# End Source File
# End Group
# Begin Group "Math"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Math\Box.c
# End Source File
# Begin Source File

SOURCE=.\Math\Box.h
# End Source File
# Begin Source File

SOURCE=.\Math\crc32.c
# End Source File
# Begin Source File

SOURCE=.\Math\crc32.h
# End Source File
# Begin Source File

SOURCE=.\Math\ExtBox.c
# End Source File
# Begin Source File

SOURCE=.\Math\ExtBox.h
# End Source File
# Begin Source File

SOURCE=.\Math\quatern.c
# End Source File
# Begin Source File

SOURCE=.\Math\quatern.h
# End Source File
# Begin Source File

SOURCE=.\Math\Vec3d.c
# End Source File
# Begin Source File

SOURCE=.\Math\Vec3d.h
# End Source File
# Begin Source File

SOURCE=.\Math\Xform3d.c
# End Source File
# Begin Source File

SOURCE=.\Math\Xform3d.h
# End Source File
# End Group
# Begin Group "Support"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Support\Basetype.h
# End Source File
# Begin Source File

SOURCE=.\Support\Errorlog.c
# End Source File
# Begin Source File

SOURCE=.\Support\Errorlog.h
# End Source File
# Begin Source File

SOURCE=.\Support\geAssert.c
# End Source File
# Begin Source File

SOURCE=.\Support\geAssert.h
# End Source File
# Begin Source File

SOURCE=.\Support\log.c
# End Source File
# Begin Source File

SOURCE=.\Support\log.h
# End Source File
# Begin Source File

SOURCE=.\Support\mempool.c
# End Source File
# Begin Source File

SOURCE=.\Support\mempool.h
# End Source File
# Begin Source File

SOURCE=.\Support\Ram.c
# End Source File
# Begin Source File

SOURCE=.\Support\Ram.h
# End Source File
# Begin Source File

SOURCE=.\Support\ramdll.c
# End Source File
# End Group
# Begin Group "VFile"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\VFile\dirtree.c
# End Source File
# Begin Source File

SOURCE=.\VFile\dirtree.h
# End Source File
# Begin Source File

SOURCE=.\VFile\fsdos.c
# End Source File
# Begin Source File

SOURCE=.\VFile\fsdos.h
# End Source File
# Begin Source File

SOURCE=.\VFile\Fsmemory.c
# End Source File
# Begin Source File

SOURCE=.\VFile\Fsmemory.h
# End Source File
# Begin Source File

SOURCE=.\VFile\fsvfs.c
# End Source File
# Begin Source File

SOURCE=.\VFile\fsvfs.h
# End Source File
# Begin Source File

SOURCE=.\VFile\vfile.c
# End Source File
# Begin Source File

SOURCE=.\VFile\vfile.h
# End Source File
# End Group
# Begin Group "World"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\World\Fog.c
# End Source File
# Begin Source File

SOURCE=.\World\Fog.h
# End Source File
# Begin Source File

SOURCE=.\World\Frustum.c
# End Source File
# Begin Source File

SOURCE=.\World\Frustum.h
# End Source File
# Begin Source File

SOURCE=.\World\Gbspfile.c
# End Source File
# Begin Source File

SOURCE=.\World\Gbspfile.h
# End Source File
# Begin Source File

SOURCE=.\World\Light.c
# End Source File
# Begin Source File

SOURCE=.\World\Light.h
# End Source File
# Begin Source File

SOURCE=.\World\Plane.c
# End Source File
# Begin Source File

SOURCE=.\World\Plane.h
# End Source File
# Begin Source File

SOURCE=.\World\Surface.c
# End Source File
# Begin Source File

SOURCE=.\World\Surface.h
# End Source File
# Begin Source File

SOURCE=.\World\Trace.c
# End Source File
# Begin Source File

SOURCE=.\World\Trace.h
# End Source File
# Begin Source File

SOURCE=.\World\User.c
# End Source File
# Begin Source File

SOURCE=.\World\User.h
# End Source File
# Begin Source File

SOURCE=.\World\Vis.c
# End Source File
# Begin Source File

SOURCE=.\World\Vis.h
# End Source File
# Begin Source File

SOURCE=.\World\WBitmap.c
# End Source File
# Begin Source File

SOURCE=.\World\WBitmap.h
# End Source File
# Begin Source File

SOURCE=.\World\World.c
# End Source File
# Begin Source File

SOURCE=.\World\World.h
# End Source File
# End Group
# Begin Group "Font"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Font\font.c
# End Source File
# Begin Source File

SOURCE=.\Font\font.H
# End Source File
# Begin Source File

SOURCE=.\Font\wgClip.c
# End Source File
# Begin Source File

SOURCE=.\Font\wgClip.H
# End Source File
# End Group
# Begin Group "Physics"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Physics\matrix33.c
# End Source File
# Begin Source File

SOURCE=.\Physics\matrix33.h
# End Source File
# Begin Source File

SOURCE=.\Physics\PhysicsJoint.c
# End Source File
# Begin Source File

SOURCE=.\Physics\PhysicsJoint.h
# End Source File
# Begin Source File

SOURCE=.\Physics\PhysicsObject.c
# End Source File
# Begin Source File

SOURCE=.\Physics\PhysicsObject.h
# End Source File
# Begin Source File

SOURCE=.\Physics\PhysicsSystem.c
# End Source File
# Begin Source File

SOURCE=.\Physics\PhysicsSystem.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\Camera.c
# End Source File
# Begin Source File

SOURCE=.\Camera.h
# End Source File
# Begin Source File

SOURCE=.\CSNetMgr.c
# End Source File
# Begin Source File

SOURCE=.\CSNetMgr.h
# End Source File
# Begin Source File

SOURCE=.\Ge.c
# End Source File
# Begin Source File

SOURCE=.\Genesis.h
# End Source File
# Begin Source File

SOURCE=.\genesis.rc
# End Source File
# Begin Source File

SOURCE=.\getypes.h
# End Source File
# Begin Source File

SOURCE=.\list.c
# End Source File
# Begin Source File

SOURCE=.\list.h
# End Source File
# Begin Source File

SOURCE=.\Netplay.c
# End Source File
# Begin Source File

SOURCE=.\Netplay.h
# End Source File
# Begin Source File

SOURCE=.\Ptrtypes.h
# End Source File
# Begin Source File

SOURCE=.\resource.h
# End Source File
# Begin Source File

SOURCE=.\Sound.c
# End Source File
# Begin Source File

SOURCE=.\Sound.h
# End Source File
# Begin Source File

SOURCE=.\Sound3d.c
# End Source File
# Begin Source File

SOURCE=.\Sound3d.h
# End Source File
# Begin Source File

SOURCE=.\Tclip.c
# End Source File
# Begin Source File

SOURCE=.\tclip.h
# End Source File
# Begin Source File

SOURCE=.\timer.c
# End Source File
# Begin Source File

SOURCE=.\timer.h
# End Source File
# Begin Source File

SOURCE=.\tsc.c
# End Source File
# Begin Source File

SOURCE=.\tsc.h
# End Source File
# End Group
# Begin Group "Libraries"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\Sdk\Dx6sdk\Lib\dxguid.lib
# End Source File
# End Group
# Begin Source File

SOURCE=.\GenesisDLL.mak
# PROP Exclude_From_Build 1
# End Source File
# End Target
# End Project
