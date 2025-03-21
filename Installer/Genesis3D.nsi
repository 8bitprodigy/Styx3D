;--------------------------------
;Include Modern UI

  !include "MUI2.nsh"
  !define MUI_HEADERIMAGE
  !define MUI_HEADERIMAGE_BITMAP "..\g3dlogo.bmp"
  !define MUI_HEADERIMAGE_RIGHT

;--------------------------------
;General

  ;Name and file
  Name "Genesis3D v1.1"
  OutFile "Genesis3D120.exe"
  SetCompressor "lzma"
  
  ;Default installation folder
  InstallDir "C:\Genesis3D\v120"
  
  ;Get installation folder from registry if available
  InstallDirRegKey HKCU "Software\Genesis3D\v120" ""

  ;Request application privileges for Windows Vista
  RequestExecutionLevel user

;--------------------------------
;Interface Settings

  !define MUI_ABORTWARNING

;--------------------------------
;Pages

  !insertmacro MUI_PAGE_LICENSE "..\OpenSource\License.txt"
  !insertmacro MUI_PAGE_DIRECTORY
  !insertmacro MUI_PAGE_INSTFILES
  
  !insertmacro MUI_UNPAGE_CONFIRM
  !insertmacro MUI_UNPAGE_INSTFILES
  
;--------------------------------
;Languages
 
  !insertmacro MUI_LANGUAGE "English"

;--------------------------------
;Installer Sections

Section "Dummy Section" SecDummy

  SetOutPath "$INSTDIR"
  
  ;ADD YOUR OWN FILES HERE...
  File /r "..\"
  
  ;Store installation folder
  WriteRegStr HKCU "Software\Genesis3D\v120" "" $INSTDIR
  
  ;Create uninstaller
  WriteUninstaller "$INSTDIR\Uninstall.exe"

SectionEnd

;--------------------------------
;Uninstaller Section

Section "Uninstall"

  SetOutPath "$TEMP"
  RMDir /r "$INSTDIR"

  DeleteRegKey /ifempty HKCU "Software\Genesis3D\v120"

SectionEnd