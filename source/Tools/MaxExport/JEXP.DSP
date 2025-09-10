# Microsoft Developer Studio Project File - Name="MaxExporters" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=MaxExporters - Win32 KEY Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "JEXP.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "JEXP.mak" CFG="MaxExporters - Win32 KEY Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "MaxExporters - Win32 KEY Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "MaxExporters - Win32 NFO Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "MaxExporters - Win32 KEY Hybrid" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "MaxExporters - Win32 NFO Hybrid" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "MaxExporters - Win32 KEY Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir ".\keyrelease"
# PROP BASE Intermediate_Dir ".\keyrelease"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir ".\keyrelease"
# PROP Intermediate_Dir ".\keyrelease"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /G5 /MT /W3 /GX /Ob1 /Gy /X /I "max2sdk\include" /I "..\msdev60\include" /I "..\msdev60\mfc\include" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "KEYEXP" /GF /c
# ADD CPP /nologo /G5 /MT /W3 /GX /Ob1 /Gy /X /I "max2sdk\include" /I "..\msdev60\include" /I "..\msdev60\mfc\include" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "KEYEXP" /GF /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /x /i "..\msdev60\include" /d "NDEBUG" /d "KEYEXP"
# ADD RSC /l 0x409 /x /i "..\msdev60\include" /d "NDEBUG" /d "KEYEXP"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib libcmt.lib max2sdk\lib\core.lib max2sdk\lib\util.lib max2sdk\lib\geom.lib max2sdk\lib\gfx.lib max2sdk\lib\mesh.lib version.lib comctl32.lib oldnames.lib /nologo /subsystem:windows /dll /machine:I386 /nodefaultlib /out:".\keyrelease\keyexp.dle" /pdbtype:sept /libpath:"..\..\msdev60\lib"
# SUBTRACT BASE LINK32 /pdb:none
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib libcmt.lib max2sdk\lib\core.lib max2sdk\lib\util.lib max2sdk\lib\geom.lib max2sdk\lib\gfx.lib max2sdk\lib\mesh.lib version.lib comctl32.lib oldnames.lib /nologo /subsystem:windows /dll /machine:I386 /nodefaultlib /out:".\keyrelease\keyexp.dle" /pdbtype:sept /libpath:"..\..\msdev60\lib"
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "MaxExporters - Win32 NFO Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir ".\nfoRelease"
# PROP BASE Intermediate_Dir ".\nfoRelease"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir ".\nfoRelease"
# PROP Intermediate_Dir ".\nfoRelease"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /G5 /MT /W3 /GX /Ob1 /Gy /X /I "max2sdk\include" /I "..\msdev60\include" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "NFOEXP" /GF /c
# ADD CPP /nologo /G5 /MT /W3 /GX /Ob1 /Gy /X /I "max2sdk\include" /I "..\msdev60\include" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "NFOEXP" /GF /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /x /i "..\msdev60\include" /d "NDEBUG" /d "NFOEXP"
# ADD RSC /l 0x409 /x /i "..\msdev60\include" /d "NDEBUG" /d "NFOEXP"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib libcmt.lib max2sdk\lib\core.lib max2sdk\lib\util.lib max2sdk\lib\geom.lib max2sdk\lib\gfx.lib max2sdk\lib\mesh.lib version.lib comctl32.lib oldnames.lib /nologo /subsystem:windows /dll /machine:I386 /nodefaultlib /out:".\nforelease\nfoexp.dle" /pdbtype:sept /libpath:"..\..\msdev60\lib"
# SUBTRACT BASE LINK32 /pdb:none
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib libcmt.lib max2sdk\lib\core.lib max2sdk\lib\util.lib max2sdk\lib\geom.lib max2sdk\lib\gfx.lib max2sdk\lib\mesh.lib version.lib comctl32.lib oldnames.lib /nologo /subsystem:windows /dll /machine:I386 /nodefaultlib /out:".\nforelease\nfoexp.dle" /pdbtype:sept /libpath:"..\..\msdev60\lib"
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "MaxExporters - Win32 KEY Hybrid"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir ".\keyhybrid"
# PROP BASE Intermediate_Dir ".\keyhybrid"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir ".\keyhybrid"
# PROP Intermediate_Dir ".\keyhybrid"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /G5 /MTd /W3 /GX /ZI /Od /X /I "max2sdk\include" /I "..\msdev60\include" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "KEYEXP" /c
# ADD CPP /nologo /G5 /MTd /W3 /GX /ZI /Od /X /I "max2sdk\include" /I "..\msdev60\include" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "KEYEXP" /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /x /i "..\msdev60\include" /d "_DEBUG" /d "KEYEXP"
# ADD RSC /l 0x409 /x /i "..\msdev60\include" /d "_DEBUG" /d "KEYEXP"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib libcmtd.lib winmm.lib oldnames.lib max2sdk\lib\core.lib max2sdk\lib\util.lib max2sdk\lib\geom.lib max2sdk\lib\gfx.lib max2sdk\lib\mesh.lib version.lib comctl32.lib /nologo /subsystem:windows /dll /incremental:yes /debug /machine:I386 /nodefaultlib /out:".\keyhybrid\keyexp.dle" /pdbtype:sept /libpath:"..\..\msdev60\lib"
# SUBTRACT BASE LINK32 /pdb:none
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib libcmtd.lib winmm.lib oldnames.lib max2sdk\lib\core.lib max2sdk\lib\util.lib max2sdk\lib\geom.lib max2sdk\lib\gfx.lib max2sdk\lib\mesh.lib version.lib comctl32.lib /nologo /subsystem:windows /dll /incremental:yes /debug /machine:I386 /nodefaultlib /out:".\keyhybrid\keyexp.dle" /pdbtype:sept /libpath:"..\..\msdev60\lib"
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "MaxExporters - Win32 NFO Hybrid"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir ".\nfoHybrid"
# PROP BASE Intermediate_Dir ".\nfoHybrid"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir ".\nfoHybrid"
# PROP Intermediate_Dir ".\nfoHybrid"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /G5 /MTd /W3 /GX /ZI /Od /I "..\..\..\include" /I "..\..\..\include\maxsdk" /I "..\..\..\include\cstudio" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_USRDLL" /D "_MBCS" /D "NFOEXP" /c
# ADD CPP /nologo /G5 /MTd /W3 /GX /ZI /Od /I "..\..\..\include" /I "..\..\..\include\maxsdk" /I "..\..\..\include\cstudio" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_USRDLL" /D "_MBCS" /D "NFOEXP" /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG" /d "NFOEXP"
# ADD RSC /l 0x409 /d "_DEBUG" /d "NFOEXP"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib winmm.lib core.lib maxutil.lib geom.lib gfx.lib mesh.lib biped.lib /nologo /subsystem:windows /dll /incremental:yes /pdb:".\nfoHybrid\nfoexp.pdb" /debug /machine:I386 /out:"..\..\..\bin\nfoexpd.dle" /implib:".\nfoHybrid/nfoexp.lib" /pdbtype:sept /libpath:"..\..\..\lib" /libpath:"..\..\..\lib\maxsdk" /libpath:"..\..\..\lib\cstudio"
# SUBTRACT BASE LINK32 /pdb:none
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib winmm.lib core.lib maxutil.lib geom.lib gfx.lib mesh.lib biped.lib /nologo /subsystem:windows /dll /incremental:yes /pdb:".\nfoHybrid\nfoexp.pdb" /debug /machine:I386 /out:"..\..\..\bin\nfoexpd.dle" /implib:".\nfoHybrid/nfoexp.lib" /pdbtype:sept /libpath:"..\..\..\lib" /libpath:"..\..\..\lib\maxsdk" /libpath:"..\..\..\lib\cstudio"
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "MaxExporters - Win32 KEY Release"
# Name "MaxExporters - Win32 NFO Release"
# Name "MaxExporters - Win32 KEY Hybrid"
# Name "MaxExporters - Win32 NFO Hybrid"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;hpj;bat;for;f90"
# Begin Source File

SOURCE=.\3dsmtl.cpp
# End Source File
# Begin Source File

SOURCE=.\jexp.cpp
# End Source File
# Begin Source File

SOURCE=.\jexp.def
# End Source File
# Begin Source File

SOURCE=.\jexp.rc
# End Source File
# Begin Source File

SOURCE=.\Savemli.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl;fi;fd"
# Begin Source File

SOURCE=.\3dseres.h
# End Source File
# Begin Source File

SOURCE=.\3dsimp.h
# End Source File
# Begin Source File

SOURCE=.\3dsires.h
# End Source File
# Begin Source File

SOURCE=.\Cfile.h
# End Source File
# Begin Source File

SOURCE=.\jexp.h
# End Source File
# Begin Source File

SOURCE=.\Kfio.h
# End Source File
# Begin Source File

SOURCE=.\Mtldef.h
# End Source File
# Begin Source File

SOURCE=.\Ofile.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;cnt;rtf;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
