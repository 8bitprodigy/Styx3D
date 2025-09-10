# Microsoft Developer Studio Generated NMAKE File, Based on AStudio.dsp
!IF "$(CFG)" == ""
CFG=AStudio - Win32 Debug
!MESSAGE No configuration specified. Defaulting to AStudio - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "AStudio - Win32 Release" && "$(CFG)" != "AStudio - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "AStudio.mak" CFG="AStudio - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "AStudio - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "AStudio - Win32 Debug" (based on "Win32 (x86) Application")
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

!IF  "$(CFG)" == "AStudio - Win32 Release"

OUTDIR=.\Release\AStudio
INTDIR=.\Release\AStudio
# Begin Custom Macros
OutDir=.\Release\AStudio
# End Custom Macros

ALL : "$(OUTDIR)\AStudio.exe"


CLEAN :
	-@erase "$(INTDIR)\AOptions.obj"
	-@erase "$(INTDIR)\AProject.obj"
	-@erase "$(INTDIR)\Array.obj"
	-@erase "$(INTDIR)\AStudio.obj"
	-@erase "$(INTDIR)\AStudio.res"
	-@erase "$(INTDIR)\BodyDlg.obj"
	-@erase "$(INTDIR)\FilePath.obj"
	-@erase "$(INTDIR)\Log.obj"
	-@erase "$(INTDIR)\LogoPage.obj"
	-@erase "$(INTDIR)\make.obj"
	-@erase "$(INTDIR)\MakeHelp.obj"
	-@erase "$(INTDIR)\MaterialsDlg.obj"
	-@erase "$(INTDIR)\maxmath.obj"
	-@erase "$(INTDIR)\mkactor.obj"
	-@erase "$(INTDIR)\mkbody.obj"
	-@erase "$(INTDIR)\mkmotion.obj"
	-@erase "$(INTDIR)\mopshell.obj"
	-@erase "$(INTDIR)\MotionsDlg.obj"
	-@erase "$(INTDIR)\mxscript.obj"
	-@erase "$(INTDIR)\MyFileDlg.obj"
	-@erase "$(INTDIR)\NewPrjDlg.obj"
	-@erase "$(INTDIR)\PathsDlg.obj"
	-@erase "$(INTDIR)\pop.obj"
	-@erase "$(INTDIR)\PropPage.obj"
	-@erase "$(INTDIR)\PropSheet.obj"
	-@erase "$(INTDIR)\Rcstring.obj"
	-@erase "$(INTDIR)\SettingsDlg.obj"
	-@erase "$(INTDIR)\StdAfx.obj"
	-@erase "$(INTDIR)\TargetDlg.obj"
	-@erase "$(INTDIR)\TDBody.obj"
	-@erase "$(INTDIR)\TextInputDlg.obj"
	-@erase "$(INTDIR)\Util.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vph.obj"
	-@erase "$(OUTDIR)\AStudio.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MT /W4 /GX /O2 /X /I ".\AStudio\Util" /I ".\JetSDK\Include" /I ".\common" /I ".\fmtactor" /I ".\mkactor" /I ".\mkbody" /I ".\mkmotion" /I ".\mop" /I "..\MsDev60\include" /I "..\MsDev60\mfc\include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /Fp"$(INTDIR)\AStudio.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32 
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\AStudio.res" /d "NDEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\AStudio.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=nafxcw.lib libcmt.lib advapi32.lib comctl32.lib comdlg32.lib ctl3d32s.lib gdi32.lib kernel32.lib oldnames.lib ole32.lib oleaut32.lib oledlg.lib shell32.lib urlmon.lib user32.lib uuid.lib winmm.lib winspool.lib /nologo /subsystem:windows /incremental:no /pdb:"$(OUTDIR)\AStudio.pdb" /machine:I386 /nodefaultlib /out:"$(OUTDIR)\AStudio.exe" /libpath:"..\..\msdev60\lib" /libpath:"..\..\msdev60\mfc\lib" 
LINK32_OBJS= \
	"$(INTDIR)\Log.obj" \
	"$(INTDIR)\maxmath.obj" \
	"$(INTDIR)\mkactor.obj" \
	"$(INTDIR)\mkbody.obj" \
	"$(INTDIR)\mkmotion.obj" \
	"$(INTDIR)\mopshell.obj" \
	"$(INTDIR)\pop.obj" \
	"$(INTDIR)\TDBody.obj" \
	"$(INTDIR)\vph.obj" \
	"$(INTDIR)\AOptions.obj" \
	"$(INTDIR)\AProject.obj" \
	"$(INTDIR)\Array.obj" \
	"$(INTDIR)\AStudio.obj" \
	"$(INTDIR)\BodyDlg.obj" \
	"$(INTDIR)\FilePath.obj" \
	"$(INTDIR)\LogoPage.obj" \
	"$(INTDIR)\make.obj" \
	"$(INTDIR)\MakeHelp.obj" \
	"$(INTDIR)\MaterialsDlg.obj" \
	"$(INTDIR)\MotionsDlg.obj" \
	"$(INTDIR)\mxscript.obj" \
	"$(INTDIR)\MyFileDlg.obj" \
	"$(INTDIR)\NewPrjDlg.obj" \
	"$(INTDIR)\PathsDlg.obj" \
	"$(INTDIR)\PropPage.obj" \
	"$(INTDIR)\PropSheet.obj" \
	"$(INTDIR)\Rcstring.obj" \
	"$(INTDIR)\SettingsDlg.obj" \
	"$(INTDIR)\StdAfx.obj" \
	"$(INTDIR)\TargetDlg.obj" \
	"$(INTDIR)\TextInputDlg.obj" \
	"$(INTDIR)\Util.obj" \
	"$(INTDIR)\AStudio.res" \
	".\JetSDK\lib\Jet3D.lib"

"$(OUTDIR)\AStudio.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "AStudio - Win32 Debug"

OUTDIR=.\Debug\AStudio
INTDIR=.\Debug\AStudio
# Begin Custom Macros
OutDir=.\Debug\AStudio
# End Custom Macros

ALL : "$(OUTDIR)\AStudio.exe" "$(OUTDIR)\AStudio.bsc"


CLEAN :
	-@erase "$(INTDIR)\AOptions.obj"
	-@erase "$(INTDIR)\AOptions.sbr"
	-@erase "$(INTDIR)\AProject.obj"
	-@erase "$(INTDIR)\AProject.sbr"
	-@erase "$(INTDIR)\Array.obj"
	-@erase "$(INTDIR)\Array.sbr"
	-@erase "$(INTDIR)\AStudio.obj"
	-@erase "$(INTDIR)\AStudio.res"
	-@erase "$(INTDIR)\AStudio.sbr"
	-@erase "$(INTDIR)\BodyDlg.obj"
	-@erase "$(INTDIR)\BodyDlg.sbr"
	-@erase "$(INTDIR)\FilePath.obj"
	-@erase "$(INTDIR)\FilePath.sbr"
	-@erase "$(INTDIR)\Log.obj"
	-@erase "$(INTDIR)\Log.sbr"
	-@erase "$(INTDIR)\LogoPage.obj"
	-@erase "$(INTDIR)\LogoPage.sbr"
	-@erase "$(INTDIR)\make.obj"
	-@erase "$(INTDIR)\make.sbr"
	-@erase "$(INTDIR)\MakeHelp.obj"
	-@erase "$(INTDIR)\MakeHelp.sbr"
	-@erase "$(INTDIR)\MaterialsDlg.obj"
	-@erase "$(INTDIR)\MaterialsDlg.sbr"
	-@erase "$(INTDIR)\maxmath.obj"
	-@erase "$(INTDIR)\maxmath.sbr"
	-@erase "$(INTDIR)\mkactor.obj"
	-@erase "$(INTDIR)\mkactor.sbr"
	-@erase "$(INTDIR)\mkbody.obj"
	-@erase "$(INTDIR)\mkbody.sbr"
	-@erase "$(INTDIR)\mkmotion.obj"
	-@erase "$(INTDIR)\mkmotion.sbr"
	-@erase "$(INTDIR)\mopshell.obj"
	-@erase "$(INTDIR)\mopshell.sbr"
	-@erase "$(INTDIR)\MotionsDlg.obj"
	-@erase "$(INTDIR)\MotionsDlg.sbr"
	-@erase "$(INTDIR)\mxscript.obj"
	-@erase "$(INTDIR)\mxscript.sbr"
	-@erase "$(INTDIR)\MyFileDlg.obj"
	-@erase "$(INTDIR)\MyFileDlg.sbr"
	-@erase "$(INTDIR)\NewPrjDlg.obj"
	-@erase "$(INTDIR)\NewPrjDlg.sbr"
	-@erase "$(INTDIR)\PathsDlg.obj"
	-@erase "$(INTDIR)\PathsDlg.sbr"
	-@erase "$(INTDIR)\pop.obj"
	-@erase "$(INTDIR)\pop.sbr"
	-@erase "$(INTDIR)\PropPage.obj"
	-@erase "$(INTDIR)\PropPage.sbr"
	-@erase "$(INTDIR)\PropSheet.obj"
	-@erase "$(INTDIR)\PropSheet.sbr"
	-@erase "$(INTDIR)\Rcstring.obj"
	-@erase "$(INTDIR)\Rcstring.sbr"
	-@erase "$(INTDIR)\SettingsDlg.obj"
	-@erase "$(INTDIR)\SettingsDlg.sbr"
	-@erase "$(INTDIR)\StdAfx.obj"
	-@erase "$(INTDIR)\StdAfx.sbr"
	-@erase "$(INTDIR)\TargetDlg.obj"
	-@erase "$(INTDIR)\TargetDlg.sbr"
	-@erase "$(INTDIR)\TDBody.obj"
	-@erase "$(INTDIR)\TDBody.sbr"
	-@erase "$(INTDIR)\TextInputDlg.obj"
	-@erase "$(INTDIR)\TextInputDlg.sbr"
	-@erase "$(INTDIR)\Util.obj"
	-@erase "$(INTDIR)\Util.sbr"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(INTDIR)\vph.obj"
	-@erase "$(INTDIR)\vph.sbr"
	-@erase "$(OUTDIR)\AStudio.bsc"
	-@erase "$(OUTDIR)\AStudio.exe"
	-@erase "$(OUTDIR)\AStudio.ilk"
	-@erase "$(OUTDIR)\AStudio.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MTd /W4 /Gm /GX /ZI /Od /X /I ".\AStudio\Util" /I ".\JetSDK\Include" /I ".\common" /I ".\fmtactor" /I ".\mkactor" /I ".\mkbody" /I ".\mkmotion" /I ".\mop" /I "..\MsDev60\include" /I "..\MsDev60\mfc\include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\AStudio.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32 
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\AStudio.res" /d "_DEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\AStudio.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\Log.sbr" \
	"$(INTDIR)\maxmath.sbr" \
	"$(INTDIR)\mkactor.sbr" \
	"$(INTDIR)\mkbody.sbr" \
	"$(INTDIR)\mkmotion.sbr" \
	"$(INTDIR)\mopshell.sbr" \
	"$(INTDIR)\pop.sbr" \
	"$(INTDIR)\TDBody.sbr" \
	"$(INTDIR)\vph.sbr" \
	"$(INTDIR)\AOptions.sbr" \
	"$(INTDIR)\AProject.sbr" \
	"$(INTDIR)\Array.sbr" \
	"$(INTDIR)\AStudio.sbr" \
	"$(INTDIR)\BodyDlg.sbr" \
	"$(INTDIR)\FilePath.sbr" \
	"$(INTDIR)\LogoPage.sbr" \
	"$(INTDIR)\make.sbr" \
	"$(INTDIR)\MakeHelp.sbr" \
	"$(INTDIR)\MaterialsDlg.sbr" \
	"$(INTDIR)\MotionsDlg.sbr" \
	"$(INTDIR)\mxscript.sbr" \
	"$(INTDIR)\MyFileDlg.sbr" \
	"$(INTDIR)\NewPrjDlg.sbr" \
	"$(INTDIR)\PathsDlg.sbr" \
	"$(INTDIR)\PropPage.sbr" \
	"$(INTDIR)\PropSheet.sbr" \
	"$(INTDIR)\Rcstring.sbr" \
	"$(INTDIR)\SettingsDlg.sbr" \
	"$(INTDIR)\StdAfx.sbr" \
	"$(INTDIR)\TargetDlg.sbr" \
	"$(INTDIR)\TextInputDlg.sbr" \
	"$(INTDIR)\Util.sbr"

"$(OUTDIR)\AStudio.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=nafxcwd.lib libcmtd.lib advapi32.lib comctl32.lib comdlg32.lib ctl3d32s.lib gdi32.lib kernel32.lib oldnames.lib ole32.lib oleaut32.lib oledlg.lib shell32.lib urlmon.lib user32.lib uuid.lib winmm.lib winspool.lib /nologo /subsystem:windows /incremental:yes /pdb:"$(OUTDIR)\AStudio.pdb" /debug /machine:I386 /nodefaultlib /out:"$(OUTDIR)\AStudio.exe" /pdbtype:sept /libpath:"..\..\msdev60\lib" /libpath:"..\..\msdev60\mfc\lib" 
LINK32_OBJS= \
	"$(INTDIR)\Log.obj" \
	"$(INTDIR)\maxmath.obj" \
	"$(INTDIR)\mkactor.obj" \
	"$(INTDIR)\mkbody.obj" \
	"$(INTDIR)\mkmotion.obj" \
	"$(INTDIR)\mopshell.obj" \
	"$(INTDIR)\pop.obj" \
	"$(INTDIR)\TDBody.obj" \
	"$(INTDIR)\vph.obj" \
	"$(INTDIR)\AOptions.obj" \
	"$(INTDIR)\AProject.obj" \
	"$(INTDIR)\Array.obj" \
	"$(INTDIR)\AStudio.obj" \
	"$(INTDIR)\BodyDlg.obj" \
	"$(INTDIR)\FilePath.obj" \
	"$(INTDIR)\LogoPage.obj" \
	"$(INTDIR)\make.obj" \
	"$(INTDIR)\MakeHelp.obj" \
	"$(INTDIR)\MaterialsDlg.obj" \
	"$(INTDIR)\MotionsDlg.obj" \
	"$(INTDIR)\mxscript.obj" \
	"$(INTDIR)\MyFileDlg.obj" \
	"$(INTDIR)\NewPrjDlg.obj" \
	"$(INTDIR)\PathsDlg.obj" \
	"$(INTDIR)\PropPage.obj" \
	"$(INTDIR)\PropSheet.obj" \
	"$(INTDIR)\Rcstring.obj" \
	"$(INTDIR)\SettingsDlg.obj" \
	"$(INTDIR)\StdAfx.obj" \
	"$(INTDIR)\TargetDlg.obj" \
	"$(INTDIR)\TextInputDlg.obj" \
	"$(INTDIR)\Util.obj" \
	"$(INTDIR)\AStudio.res" \
	".\JetSDK\lib\Jet3Dd.lib"

"$(OUTDIR)\AStudio.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
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
!IF EXISTS("AStudio.dep")
!INCLUDE "AStudio.dep"
!ELSE 
!MESSAGE Warning: cannot find "AStudio.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "AStudio - Win32 Release" || "$(CFG)" == "AStudio - Win32 Debug"
SOURCE=.\mop\Log.c

!IF  "$(CFG)" == "AStudio - Win32 Release"


"$(INTDIR)\Log.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "AStudio - Win32 Debug"


"$(INTDIR)\Log.obj"	"$(INTDIR)\Log.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\common\maxmath.c

!IF  "$(CFG)" == "AStudio - Win32 Release"


"$(INTDIR)\maxmath.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "AStudio - Win32 Debug"


"$(INTDIR)\maxmath.obj"	"$(INTDIR)\maxmath.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\mkactor\mkactor.c

!IF  "$(CFG)" == "AStudio - Win32 Release"


"$(INTDIR)\mkactor.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "AStudio - Win32 Debug"


"$(INTDIR)\mkactor.obj"	"$(INTDIR)\mkactor.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\mkbody\mkbody.cpp

!IF  "$(CFG)" == "AStudio - Win32 Release"


"$(INTDIR)\mkbody.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "AStudio - Win32 Debug"


"$(INTDIR)\mkbody.obj"	"$(INTDIR)\mkbody.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\mkmotion\mkmotion.c

!IF  "$(CFG)" == "AStudio - Win32 Release"


"$(INTDIR)\mkmotion.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "AStudio - Win32 Debug"


"$(INTDIR)\mkmotion.obj"	"$(INTDIR)\mkmotion.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\mop\mopshell.c

!IF  "$(CFG)" == "AStudio - Win32 Release"


"$(INTDIR)\mopshell.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "AStudio - Win32 Debug"


"$(INTDIR)\mopshell.obj"	"$(INTDIR)\mopshell.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\mop\pop.c

!IF  "$(CFG)" == "AStudio - Win32 Release"


"$(INTDIR)\pop.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "AStudio - Win32 Debug"


"$(INTDIR)\pop.obj"	"$(INTDIR)\pop.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\common\TDBody.c

!IF  "$(CFG)" == "AStudio - Win32 Release"


"$(INTDIR)\TDBody.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "AStudio - Win32 Debug"


"$(INTDIR)\TDBody.obj"	"$(INTDIR)\TDBody.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\mkbody\vph.c

!IF  "$(CFG)" == "AStudio - Win32 Release"


"$(INTDIR)\vph.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "AStudio - Win32 Debug"


"$(INTDIR)\vph.obj"	"$(INTDIR)\vph.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\AStudio\AOptions.c

!IF  "$(CFG)" == "AStudio - Win32 Release"


"$(INTDIR)\AOptions.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "AStudio - Win32 Debug"


"$(INTDIR)\AOptions.obj"	"$(INTDIR)\AOptions.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\AStudio\AProject.c

!IF  "$(CFG)" == "AStudio - Win32 Release"


"$(INTDIR)\AProject.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "AStudio - Win32 Debug"


"$(INTDIR)\AProject.obj"	"$(INTDIR)\AProject.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\AStudio\Util\Array.c

!IF  "$(CFG)" == "AStudio - Win32 Release"


"$(INTDIR)\Array.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "AStudio - Win32 Debug"


"$(INTDIR)\Array.obj"	"$(INTDIR)\Array.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\AStudio\AStudio.cpp

!IF  "$(CFG)" == "AStudio - Win32 Release"


"$(INTDIR)\AStudio.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "AStudio - Win32 Debug"


"$(INTDIR)\AStudio.obj"	"$(INTDIR)\AStudio.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\AStudio\AStudio.rc

!IF  "$(CFG)" == "AStudio - Win32 Release"


"$(INTDIR)\AStudio.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) /l 0x409 /fo"$(INTDIR)\AStudio.res" /i "AStudio" /i "..\..\msdev60\include" /i "..\..\msdev60\mfc\include" /d "NDEBUG" $(SOURCE)


!ELSEIF  "$(CFG)" == "AStudio - Win32 Debug"


"$(INTDIR)\AStudio.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) /l 0x409 /fo"$(INTDIR)\AStudio.res" /i "AStudio" /i "..\..\msdev60\include" /i "..\..\msdev60\mfc\include" /d "_DEBUG" $(SOURCE)


!ENDIF 

SOURCE=.\AStudio\BodyDlg.cpp

!IF  "$(CFG)" == "AStudio - Win32 Release"


"$(INTDIR)\BodyDlg.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "AStudio - Win32 Debug"


"$(INTDIR)\BodyDlg.obj"	"$(INTDIR)\BodyDlg.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\AStudio\Util\FilePath.c

!IF  "$(CFG)" == "AStudio - Win32 Release"


"$(INTDIR)\FilePath.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "AStudio - Win32 Debug"


"$(INTDIR)\FilePath.obj"	"$(INTDIR)\FilePath.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\AStudio\LogoPage.cpp

!IF  "$(CFG)" == "AStudio - Win32 Release"


"$(INTDIR)\LogoPage.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "AStudio - Win32 Debug"


"$(INTDIR)\LogoPage.obj"	"$(INTDIR)\LogoPage.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\AStudio\make.c

!IF  "$(CFG)" == "AStudio - Win32 Release"


"$(INTDIR)\make.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "AStudio - Win32 Debug"


"$(INTDIR)\make.obj"	"$(INTDIR)\make.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\AStudio\MakeHelp.cpp

!IF  "$(CFG)" == "AStudio - Win32 Release"


"$(INTDIR)\MakeHelp.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "AStudio - Win32 Debug"


"$(INTDIR)\MakeHelp.obj"	"$(INTDIR)\MakeHelp.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\AStudio\MaterialsDlg.cpp

!IF  "$(CFG)" == "AStudio - Win32 Release"


"$(INTDIR)\MaterialsDlg.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "AStudio - Win32 Debug"


"$(INTDIR)\MaterialsDlg.obj"	"$(INTDIR)\MaterialsDlg.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\AStudio\MotionsDlg.cpp

!IF  "$(CFG)" == "AStudio - Win32 Release"


"$(INTDIR)\MotionsDlg.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "AStudio - Win32 Debug"


"$(INTDIR)\MotionsDlg.obj"	"$(INTDIR)\MotionsDlg.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\AStudio\mxscript.c

!IF  "$(CFG)" == "AStudio - Win32 Release"


"$(INTDIR)\mxscript.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "AStudio - Win32 Debug"


"$(INTDIR)\mxscript.obj"	"$(INTDIR)\mxscript.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\AStudio\MyFileDlg.cpp

!IF  "$(CFG)" == "AStudio - Win32 Release"


"$(INTDIR)\MyFileDlg.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "AStudio - Win32 Debug"


"$(INTDIR)\MyFileDlg.obj"	"$(INTDIR)\MyFileDlg.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\AStudio\NewPrjDlg.cpp

!IF  "$(CFG)" == "AStudio - Win32 Release"


"$(INTDIR)\NewPrjDlg.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "AStudio - Win32 Debug"


"$(INTDIR)\NewPrjDlg.obj"	"$(INTDIR)\NewPrjDlg.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\AStudio\PathsDlg.cpp

!IF  "$(CFG)" == "AStudio - Win32 Release"


"$(INTDIR)\PathsDlg.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "AStudio - Win32 Debug"


"$(INTDIR)\PathsDlg.obj"	"$(INTDIR)\PathsDlg.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\AStudio\PropPage.cpp

!IF  "$(CFG)" == "AStudio - Win32 Release"


"$(INTDIR)\PropPage.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "AStudio - Win32 Debug"


"$(INTDIR)\PropPage.obj"	"$(INTDIR)\PropPage.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\AStudio\PropSheet.cpp

!IF  "$(CFG)" == "AStudio - Win32 Release"


"$(INTDIR)\PropSheet.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "AStudio - Win32 Debug"


"$(INTDIR)\PropSheet.obj"	"$(INTDIR)\PropSheet.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\AStudio\Util\Rcstring.c

!IF  "$(CFG)" == "AStudio - Win32 Release"


"$(INTDIR)\Rcstring.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "AStudio - Win32 Debug"


"$(INTDIR)\Rcstring.obj"	"$(INTDIR)\Rcstring.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\AStudio\SettingsDlg.cpp

!IF  "$(CFG)" == "AStudio - Win32 Release"


"$(INTDIR)\SettingsDlg.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "AStudio - Win32 Debug"


"$(INTDIR)\SettingsDlg.obj"	"$(INTDIR)\SettingsDlg.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\AStudio\StdAfx.cpp

!IF  "$(CFG)" == "AStudio - Win32 Release"


"$(INTDIR)\StdAfx.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "AStudio - Win32 Debug"


"$(INTDIR)\StdAfx.obj"	"$(INTDIR)\StdAfx.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\AStudio\TargetDlg.cpp

!IF  "$(CFG)" == "AStudio - Win32 Release"


"$(INTDIR)\TargetDlg.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "AStudio - Win32 Debug"


"$(INTDIR)\TargetDlg.obj"	"$(INTDIR)\TargetDlg.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\AStudio\TextInputDlg.cpp

!IF  "$(CFG)" == "AStudio - Win32 Release"


"$(INTDIR)\TextInputDlg.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "AStudio - Win32 Debug"


"$(INTDIR)\TextInputDlg.obj"	"$(INTDIR)\TextInputDlg.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\AStudio\Util\Util.c

!IF  "$(CFG)" == "AStudio - Win32 Release"


"$(INTDIR)\Util.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "AStudio - Win32 Debug"


"$(INTDIR)\Util.obj"	"$(INTDIR)\Util.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 


!ENDIF 

