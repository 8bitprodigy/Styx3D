# Microsoft Developer Studio Generated NMAKE File, Based on GTest.dsp
!IF "$(CFG)" == ""
CFG=GTEST - WIN32 DEBUG
!MESSAGE No configuration specified. Defaulting to GTEST - WIN32 DEBUG.
!ENDIF 

!IF "$(CFG)" != "GTest - Win32 Release" && "$(CFG)" != "GTest - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
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
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "GTest - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release

ALL : "..\GTest.exe"


CLEAN :
	-@erase "$(INTDIR)\_bot.obj"
	-@erase "$(INTDIR)\Attacker.obj"
	-@erase "$(INTDIR)\AutoSelect.obj"
	-@erase "$(INTDIR)\bot.obj"
	-@erase "$(INTDIR)\Buffer.obj"
	-@erase "$(INTDIR)\bumpmap.obj"
	-@erase "$(INTDIR)\Cd.obj"
	-@erase "$(INTDIR)\Client.obj"
	-@erase "$(INTDIR)\Console.obj"
	-@erase "$(INTDIR)\corona.obj"
	-@erase "$(INTDIR)\Drvlist.obj"
	-@erase "$(INTDIR)\DynLight.obj"
	-@erase "$(INTDIR)\electric.obj"
	-@erase "$(INTDIR)\ElectricFx.obj"
	-@erase "$(INTDIR)\fire.obj"
	-@erase "$(INTDIR)\Fx.obj"
	-@erase "$(INTDIR)\GameMgr.obj"
	-@erase "$(INTDIR)\gebmutil.obj"
	-@erase "$(INTDIR)\Genvs.obj"
	-@erase "$(INTDIR)\genvs.res"
	-@erase "$(INTDIR)\Genvsi.obj"
	-@erase "$(INTDIR)\GMain.obj"
	-@erase "$(INTDIR)\Gmenu.obj"
	-@erase "$(INTDIR)\Host.obj"
	-@erase "$(INTDIR)\ipaddr.obj"
	-@erase "$(INTDIR)\Items.obj"
	-@erase "$(INTDIR)\Level.obj"
	-@erase "$(INTDIR)\Menu.obj"
	-@erase "$(INTDIR)\ModelCtl.obj"
	-@erase "$(INTDIR)\modelist.obj"
	-@erase "$(INTDIR)\NetMgr.obj"
	-@erase "$(INTDIR)\particles.obj"
	-@erase "$(INTDIR)\PathPt.obj"
	-@erase "$(INTDIR)\plasma.obj"
	-@erase "$(INTDIR)\proceng.obj"
	-@erase "$(INTDIR)\proceng_tables.obj"
	-@erase "$(INTDIR)\procutil.obj"
	-@erase "$(INTDIR)\Server.obj"
	-@erase "$(INTDIR)\setformat.obj"
	-@erase "$(INTDIR)\smoke.obj"
	-@erase "$(INTDIR)\Text.obj"
	-@erase "$(INTDIR)\Track.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\VidMode.obj"
	-@erase "$(INTDIR)\Water.obj"
	-@erase "$(INTDIR)\Weapons.obj"
	-@erase "..\GTest.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MT /W3 /GX /O2 /I "..\Src" /I "..\Include" /I "DrvMenu" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "ERRORLOG_FULL_REPORTING" /Fp"$(INTDIR)\GTest.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /win32 
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\genvs.res" /d "NDEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\GTest.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib uuid.lib winmm.lib wsock32.lib ole32.lib /nologo /subsystem:windows /incremental:no /pdb:"$(OUTDIR)\GTest.pdb" /machine:I386 /out:"..\GTest.exe" 
LINK32_OBJS= \
	"$(INTDIR)\_bot.obj" \
	"$(INTDIR)\Attacker.obj" \
	"$(INTDIR)\bot.obj" \
	"$(INTDIR)\Genvsi.obj" \
	"$(INTDIR)\GMain.obj" \
	"$(INTDIR)\Items.obj" \
	"$(INTDIR)\Level.obj" \
	"$(INTDIR)\PathPt.obj" \
	"$(INTDIR)\Track.obj" \
	"$(INTDIR)\Weapons.obj" \
	"$(INTDIR)\corona.obj" \
	"$(INTDIR)\DynLight.obj" \
	"$(INTDIR)\electric.obj" \
	"$(INTDIR)\Fx.obj" \
	"$(INTDIR)\ModelCtl.obj" \
	"$(INTDIR)\bumpmap.obj" \
	"$(INTDIR)\ElectricFx.obj" \
	"$(INTDIR)\fire.obj" \
	"$(INTDIR)\gebmutil.obj" \
	"$(INTDIR)\particles.obj" \
	"$(INTDIR)\plasma.obj" \
	"$(INTDIR)\proceng.obj" \
	"$(INTDIR)\proceng_tables.obj" \
	"$(INTDIR)\procutil.obj" \
	"$(INTDIR)\setformat.obj" \
	"$(INTDIR)\smoke.obj" \
	"$(INTDIR)\Water.obj" \
	"$(INTDIR)\Drvlist.obj" \
	"$(INTDIR)\AutoSelect.obj" \
	"$(INTDIR)\Buffer.obj" \
	"$(INTDIR)\Cd.obj" \
	"$(INTDIR)\Client.obj" \
	"$(INTDIR)\Console.obj" \
	"$(INTDIR)\GameMgr.obj" \
	"$(INTDIR)\Genvs.obj" \
	"$(INTDIR)\Gmenu.obj" \
	"$(INTDIR)\Host.obj" \
	"$(INTDIR)\ipaddr.obj" \
	"$(INTDIR)\Menu.obj" \
	"$(INTDIR)\modelist.obj" \
	"$(INTDIR)\NetMgr.obj" \
	"$(INTDIR)\Server.obj" \
	"$(INTDIR)\Text.obj" \
	"$(INTDIR)\VidMode.obj" \
	"$(INTDIR)\genvs.res" \
	"..\Lib\genesis.lib"

"..\GTest.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "GTest - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug

ALL : "..\GTest.exe"


CLEAN :
	-@erase "$(INTDIR)\_bot.obj"
	-@erase "$(INTDIR)\Attacker.obj"
	-@erase "$(INTDIR)\AutoSelect.obj"
	-@erase "$(INTDIR)\bot.obj"
	-@erase "$(INTDIR)\Buffer.obj"
	-@erase "$(INTDIR)\bumpmap.obj"
	-@erase "$(INTDIR)\Cd.obj"
	-@erase "$(INTDIR)\Client.obj"
	-@erase "$(INTDIR)\Console.obj"
	-@erase "$(INTDIR)\corona.obj"
	-@erase "$(INTDIR)\Drvlist.obj"
	-@erase "$(INTDIR)\DynLight.obj"
	-@erase "$(INTDIR)\electric.obj"
	-@erase "$(INTDIR)\ElectricFx.obj"
	-@erase "$(INTDIR)\fire.obj"
	-@erase "$(INTDIR)\Fx.obj"
	-@erase "$(INTDIR)\GameMgr.obj"
	-@erase "$(INTDIR)\gebmutil.obj"
	-@erase "$(INTDIR)\Genvs.obj"
	-@erase "$(INTDIR)\genvs.res"
	-@erase "$(INTDIR)\Genvsi.obj"
	-@erase "$(INTDIR)\GMain.obj"
	-@erase "$(INTDIR)\Gmenu.obj"
	-@erase "$(INTDIR)\Host.obj"
	-@erase "$(INTDIR)\ipaddr.obj"
	-@erase "$(INTDIR)\Items.obj"
	-@erase "$(INTDIR)\Level.obj"
	-@erase "$(INTDIR)\Menu.obj"
	-@erase "$(INTDIR)\ModelCtl.obj"
	-@erase "$(INTDIR)\modelist.obj"
	-@erase "$(INTDIR)\NetMgr.obj"
	-@erase "$(INTDIR)\particles.obj"
	-@erase "$(INTDIR)\PathPt.obj"
	-@erase "$(INTDIR)\plasma.obj"
	-@erase "$(INTDIR)\proceng.obj"
	-@erase "$(INTDIR)\proceng_tables.obj"
	-@erase "$(INTDIR)\procutil.obj"
	-@erase "$(INTDIR)\Server.obj"
	-@erase "$(INTDIR)\setformat.obj"
	-@erase "$(INTDIR)\smoke.obj"
	-@erase "$(INTDIR)\Text.obj"
	-@erase "$(INTDIR)\Track.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(INTDIR)\VidMode.obj"
	-@erase "$(INTDIR)\Water.obj"
	-@erase "$(INTDIR)\Weapons.obj"
	-@erase "$(OUTDIR)\GTest.pdb"
	-@erase "..\GTest.exe"
	-@erase "..\GTest.ilk"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MTd /W3 /Gm /GX /ZI /Od /I "..\Src" /I "..\Include" /I "DrvMenu" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /Fp"$(INTDIR)\GTest.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /win32 
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\genvs.res" /d "_DEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\GTest.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib uuid.lib winmm.lib wsock32.lib ole32.lib /nologo /subsystem:windows /incremental:yes /pdb:"$(OUTDIR)\GTest.pdb" /debug /machine:I386 /out:"..\GTest.exe" /pdbtype:sept 
LINK32_OBJS= \
	"$(INTDIR)\_bot.obj" \
	"$(INTDIR)\Attacker.obj" \
	"$(INTDIR)\bot.obj" \
	"$(INTDIR)\Genvsi.obj" \
	"$(INTDIR)\GMain.obj" \
	"$(INTDIR)\Items.obj" \
	"$(INTDIR)\Level.obj" \
	"$(INTDIR)\PathPt.obj" \
	"$(INTDIR)\Track.obj" \
	"$(INTDIR)\Weapons.obj" \
	"$(INTDIR)\corona.obj" \
	"$(INTDIR)\DynLight.obj" \
	"$(INTDIR)\electric.obj" \
	"$(INTDIR)\Fx.obj" \
	"$(INTDIR)\ModelCtl.obj" \
	"$(INTDIR)\bumpmap.obj" \
	"$(INTDIR)\ElectricFx.obj" \
	"$(INTDIR)\fire.obj" \
	"$(INTDIR)\gebmutil.obj" \
	"$(INTDIR)\particles.obj" \
	"$(INTDIR)\plasma.obj" \
	"$(INTDIR)\proceng.obj" \
	"$(INTDIR)\proceng_tables.obj" \
	"$(INTDIR)\procutil.obj" \
	"$(INTDIR)\setformat.obj" \
	"$(INTDIR)\smoke.obj" \
	"$(INTDIR)\Water.obj" \
	"$(INTDIR)\Drvlist.obj" \
	"$(INTDIR)\AutoSelect.obj" \
	"$(INTDIR)\Buffer.obj" \
	"$(INTDIR)\Cd.obj" \
	"$(INTDIR)\Client.obj" \
	"$(INTDIR)\Console.obj" \
	"$(INTDIR)\GameMgr.obj" \
	"$(INTDIR)\Genvs.obj" \
	"$(INTDIR)\Gmenu.obj" \
	"$(INTDIR)\Host.obj" \
	"$(INTDIR)\ipaddr.obj" \
	"$(INTDIR)\Menu.obj" \
	"$(INTDIR)\modelist.obj" \
	"$(INTDIR)\NetMgr.obj" \
	"$(INTDIR)\Server.obj" \
	"$(INTDIR)\Text.obj" \
	"$(INTDIR)\VidMode.obj" \
	"$(INTDIR)\genvs.res" \
	"..\Lib\genesisd.lib"

"..\GTest.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 

.c{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("GTest.dep")
!INCLUDE "GTest.dep"
!ELSE 
!MESSAGE Warning: cannot find "GTest.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "GTest - Win32 Release" || "$(CFG)" == "GTest - Win32 Debug"
SOURCE=.\Game\_bot.c

"$(INTDIR)\_bot.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\Game\Attacker.c

"$(INTDIR)\Attacker.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\Game\bot.c

"$(INTDIR)\bot.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\Game\Genvsi.c

"$(INTDIR)\Genvsi.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\Game\GMain.c

"$(INTDIR)\GMain.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\Game\Items.c

"$(INTDIR)\Items.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\Game\Level.c

"$(INTDIR)\Level.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\Game\PathPt.c

"$(INTDIR)\PathPt.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\Game\Track.c

"$(INTDIR)\Track.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\Game\Weapons.c

"$(INTDIR)\Weapons.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\Fx\corona.c

"$(INTDIR)\corona.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\Fx\DynLight.c

"$(INTDIR)\DynLight.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\Fx\electric.c

"$(INTDIR)\electric.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\Fx\Fx.c

"$(INTDIR)\Fx.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\Fx\ModelCtl.c

"$(INTDIR)\ModelCtl.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\Procedurals\bumpmap.c

"$(INTDIR)\bumpmap.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\Procedurals\ElectricFx.c

"$(INTDIR)\ElectricFx.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\Procedurals\fire.c

"$(INTDIR)\fire.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\Procedurals\gebmutil.c

"$(INTDIR)\gebmutil.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\Procedurals\particles.c

"$(INTDIR)\particles.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\Procedurals\plasma.c

"$(INTDIR)\plasma.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\Procedurals\proceng.c

"$(INTDIR)\proceng.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\Procedurals\proceng_tables.c

"$(INTDIR)\proceng_tables.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\Procedurals\procutil.c

"$(INTDIR)\procutil.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\Procedurals\setformat.c

"$(INTDIR)\setformat.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\Procedurals\smoke.c

"$(INTDIR)\smoke.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\Procedurals\Water.c

"$(INTDIR)\Water.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\DrvMenu\Drvlist.c

"$(INTDIR)\Drvlist.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\DrvMenu\genvs.rc

!IF  "$(CFG)" == "GTest - Win32 Release"


"$(INTDIR)\genvs.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) /l 0x409 /fo"$(INTDIR)\genvs.res" /i "DrvMenu" /d "NDEBUG" $(SOURCE)


!ELSEIF  "$(CFG)" == "GTest - Win32 Debug"


"$(INTDIR)\genvs.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) /l 0x409 /fo"$(INTDIR)\genvs.res" /i "DrvMenu" /d "_DEBUG" $(SOURCE)


!ENDIF 

SOURCE=.\AutoSelect.c

"$(INTDIR)\AutoSelect.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\Buffer.c

"$(INTDIR)\Buffer.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\Cd.c

"$(INTDIR)\Cd.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\Client.c

"$(INTDIR)\Client.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\Console.c

"$(INTDIR)\Console.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\GameMgr.c

"$(INTDIR)\GameMgr.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\Genvs.c

"$(INTDIR)\Genvs.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\Gmenu.c

"$(INTDIR)\Gmenu.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\Host.c

"$(INTDIR)\Host.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\ipaddr.c

"$(INTDIR)\ipaddr.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\Menu.c

"$(INTDIR)\Menu.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\modelist.c

"$(INTDIR)\modelist.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\NetMgr.c

"$(INTDIR)\NetMgr.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\Server.c

"$(INTDIR)\Server.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\Text.c

"$(INTDIR)\Text.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\VidMode.c

"$(INTDIR)\VidMode.obj" : $(SOURCE) "$(INTDIR)"



!ENDIF 

