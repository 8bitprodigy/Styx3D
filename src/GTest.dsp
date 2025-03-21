# Microsoft Developer Studio Project File - Name="GTest" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=GTEST - WIN32 DEBUG
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "GTest.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "GTest.mak" CFG="GTEST - WIN32 DEBUG"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "GTest - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "GTest - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/Genesis3D/Src", QHABAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "GTest - Win32 Release"

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
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "..\Src" /I "..\Include" /I "DrvMenu" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "ERRORLOG_FULL_REPORTING" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib uuid.lib winmm.lib wsock32.lib ole32.lib /nologo /subsystem:windows /machine:I386 /out:"..\GTest.exe"

!ELSEIF  "$(CFG)" == "GTest - Win32 Debug"

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
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "..\Src" /I "..\Include" /I "DrvMenu" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib uuid.lib winmm.lib wsock32.lib ole32.lib /nologo /subsystem:windows /debug /machine:I386 /out:"..\GTest.exe" /pdbtype:sept

!ENDIF 

# Begin Target

# Name "GTest - Win32 Release"
# Name "GTest - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Group "Game"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Game\_bot.c
# End Source File
# Begin Source File

SOURCE=.\Game\_bot.h
# End Source File
# Begin Source File

SOURCE=.\Game\Attacker.c
# End Source File
# Begin Source File

SOURCE=.\Game\bit.h
# End Source File
# Begin Source File

SOURCE=.\Game\bot.c
# End Source File
# Begin Source File

SOURCE=.\Game\bot.h
# End Source File
# Begin Source File

SOURCE=.\Game\botact.h
# End Source File
# Begin Source File

SOURCE=.\Game\botmatch.h
# End Source File
# Begin Source File

SOURCE=.\Game\Game.h
# End Source File
# Begin Source File

SOURCE=.\Game\Genvs.h
# End Source File
# Begin Source File

SOURCE=.\Game\Genvsi.c
# End Source File
# Begin Source File

SOURCE=.\Game\Genvsi.h
# End Source File
# Begin Source File

SOURCE=.\Game\GMain.c
# End Source File
# Begin Source File

SOURCE=.\Game\GMain.h
# End Source File
# Begin Source File

SOURCE=.\Game\Gplayer.h
# End Source File
# Begin Source File

SOURCE=.\Game\Items.c
# End Source File
# Begin Source File

SOURCE=.\Game\Level.c
# End Source File
# Begin Source File

SOURCE=.\Game\PathPt.c
# End Source File
# Begin Source File

SOURCE=.\Game\pathpt.h
# End Source File
# Begin Source File

SOURCE=.\Game\Track.c
# End Source File
# Begin Source File

SOURCE=.\Game\track.h
# End Source File
# Begin Source File

SOURCE=.\Game\Weapons.c
# End Source File
# End Group
# Begin Group "Fx"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Fx\corona.c
# End Source File
# Begin Source File

SOURCE=.\Fx\corona.h
# End Source File
# Begin Source File

SOURCE=.\Fx\DynLight.c
# End Source File
# Begin Source File

SOURCE=.\Fx\DynLight.h
# End Source File
# Begin Source File

SOURCE=.\Fx\electric.c
# End Source File
# Begin Source File

SOURCE=.\Fx\electric.h
# End Source File
# Begin Source File

SOURCE=.\Fx\Fx.c
# End Source File
# Begin Source File

SOURCE=.\Fx\fx.h
# End Source File
# Begin Source File

SOURCE=.\Fx\ModelCtl.c
# End Source File
# Begin Source File

SOURCE=.\Fx\ModelCtl.h
# End Source File
# End Group
# Begin Group "Procedurals"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Procedurals\bumpmap.c
# End Source File
# Begin Source File

SOURCE=.\Procedurals\bumpmap.h
# End Source File
# Begin Source File

SOURCE=.\Procedurals\ElectricFx.c
# End Source File
# Begin Source File

SOURCE=.\Procedurals\ElectricFx.h
# End Source File
# Begin Source File

SOURCE=.\Procedurals\fire.c
# End Source File
# Begin Source File

SOURCE=.\Procedurals\fire.h
# End Source File
# Begin Source File

SOURCE=.\Procedurals\gebmutil.c
# End Source File
# Begin Source File

SOURCE=.\Procedurals\gebmutil.h
# End Source File
# Begin Source File

SOURCE=.\Procedurals\particles.c
# End Source File
# Begin Source File

SOURCE=.\Procedurals\particles.h
# End Source File
# Begin Source File

SOURCE=.\Procedurals\plasma.c
# End Source File
# Begin Source File

SOURCE=.\Procedurals\plasma.h
# End Source File
# Begin Source File

SOURCE=.\Procedurals\procedural.h
# End Source File
# Begin Source File

SOURCE=.\Procedurals\proceng.c
# End Source File
# Begin Source File

SOURCE=.\Procedurals\proceng.h
# End Source File
# Begin Source File

SOURCE=.\Procedurals\proceng_tables.c
# End Source File
# Begin Source File

SOURCE=.\Procedurals\procutil.c
# End Source File
# Begin Source File

SOURCE=.\Procedurals\procutil.h
# End Source File
# Begin Source File

SOURCE=.\Procedurals\setformat.c
# End Source File
# Begin Source File

SOURCE=.\Procedurals\setformat.h
# End Source File
# Begin Source File

SOURCE=.\Procedurals\smoke.c
# End Source File
# Begin Source File

SOURCE=.\Procedurals\smoke.h
# End Source File
# Begin Source File

SOURCE=.\Procedurals\Water.c
# End Source File
# Begin Source File

SOURCE=.\Procedurals\Water.h
# End Source File
# End Group
# Begin Group "DrvMenu"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\DrvMenu\Drvlist.c
# End Source File
# Begin Source File

SOURCE=.\DrvMenu\Drvlist.h
# End Source File
# Begin Source File

SOURCE=.\DrvMenu\genvs.rc
# End Source File
# Begin Source File

SOURCE=.\DrvMenu\Resource.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\AutoSelect.c
# End Source File
# Begin Source File

SOURCE=.\AutoSelect.h
# End Source File
# Begin Source File

SOURCE=.\Buffer.c
# End Source File
# Begin Source File

SOURCE=.\Buffer.h
# End Source File
# Begin Source File

SOURCE=.\Cd.c
# End Source File
# Begin Source File

SOURCE=.\Cd.h
# End Source File
# Begin Source File

SOURCE=.\Client.c
# End Source File
# Begin Source File

SOURCE=.\Client.h
# End Source File
# Begin Source File

SOURCE=.\Console.c
# End Source File
# Begin Source File

SOURCE=.\Console.h
# End Source File
# Begin Source File

SOURCE=.\GameMgr.c
# End Source File
# Begin Source File

SOURCE=.\Gamemgr.h
# End Source File
# Begin Source File

SOURCE=.\Genvs.c
# End Source File
# Begin Source File

SOURCE=.\Gmenu.c
# End Source File
# Begin Source File

SOURCE=.\Gmenu.h
# End Source File
# Begin Source File

SOURCE=.\Host.c
# End Source File
# Begin Source File

SOURCE=.\Host.h
# End Source File
# Begin Source File

SOURCE=.\ipaddr.c
# End Source File
# Begin Source File

SOURCE=.\Menu.c
# End Source File
# Begin Source File

SOURCE=.\Menu.h
# End Source File
# Begin Source File

SOURCE=.\Menuitem.h
# End Source File
# Begin Source File

SOURCE=.\modelist.c
# End Source File
# Begin Source File

SOURCE=.\ModeList.h
# End Source File
# Begin Source File

SOURCE=.\NetMgr.c
# End Source File
# Begin Source File

SOURCE=.\NetMgr.h
# End Source File
# Begin Source File

SOURCE=.\Server.c
# End Source File
# Begin Source File

SOURCE=.\Server.h
# End Source File
# Begin Source File

SOURCE=.\Text.c
# End Source File
# Begin Source File

SOURCE=.\Text.h
# End Source File
# Begin Source File

SOURCE=.\VidMode.c
# End Source File
# Begin Source File

SOURCE=.\VidMode.h
# End Source File
# End Group
# Begin Group "Libraries"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\Lib\genesisd.lib

!IF  "$(CFG)" == "GTest - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "GTest - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\Lib\genesis.lib

!IF  "$(CFG)" == "GTest - Win32 Release"

!ELSEIF  "$(CFG)" == "GTest - Win32 Debug"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# End Group
# End Target
# End Project
