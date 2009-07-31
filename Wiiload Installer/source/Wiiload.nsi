;Website: http://code.google.com/p/usbloader-gui/

;Copyright 2009 NeoRame

;This program is free software: you can redistribute it and/or modify
;it under the terms of the GNU General Public License as published by
;the Free Software Foundation, either version 3 of the License, or
;(at your option) any later version.

;This program is distributed in the hope that it will be useful,
;but WITHOUT ANY WARRANTY; without even the implied warranty of
;MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;GNU General Public License for more details.

;You should have received a copy of the GNU General Public License
;along with this program.  If not, see http://www.gnu.org/licenses/gpl-3.0.html.


!define TEMP2 $R1 ;Temp variable
!define TEMP3 $R2 ;Temp variable
!define TEMP4 $R3 ;Temp variable
!define TEMP5 $R4 ;Temp variable

!include "InstallOptions.nsh"
!include "LogicLib.nsh"
!include InstallerConfig.nsh
!define SHCNE_ASSOCCHANGED 0x08000000
!define SHCNF_IDLIST 0

;=== Program Details
Name "${NAME2}"
OutFile "..\${FILENAME}.exe"
InstallDir "$PROGRAMFILES\${NAME}"
Caption "${NAME2} Installer"
VIProductVersion "${VERSION}"
VIAddVersionKey ProductName "${NAME2}"
VIAddVersionKey Comments "For more informations please visit http://code.google.com/p/usbloader-gui/"
VIAddVersionKey CompanyName "${TEAM}"
VIAddVersionKey LegalCopyright "${TEAM}"
VIAddVersionKey FileDescription "${NAME2}"
VIAddVersionKey FileVersion "${VERSION}"
VIAddVersionKey ProductVersion "${VERSION}"
VIAddVersionKey InternalName "${NAME}"
VIAddVersionKey LegalTrademarks "${NAME2} installer created by ${TEAM}."
VIAddVersionKey OriginalFilename "${FILENAME}.exe"

;=== Runtime Switches
SetCompressor /SOLID lzma
CRCCheck on
AutoCloseWindow True
ShowInstDetails nevershow
ShowUnInstDetails nevershow

;=== Include Modern UI
!include "MUI.nsh"

;=== General
;Get installation folder from registry if available
InstallDirRegKey HKCU "Software\${NAME}" ""

;=== Interface Settings
!define MUI_ABORTWARNING

;===Language Selection Dialog Settings
  ;Remember the installer language
  !define MUI_LANGDLL_REGISTRY_ROOT "HKCU"
  !define MUI_LANGDLL_REGISTRY_KEY "Software\${NAME}"
  !define MUI_LANGDLL_REGISTRY_VALUENAME "Installer Language"
  
;=== Program Icon
Icon "files\${NAME}.ico"


;=== Icon & Style ===
!define MUI_ICON "files\${NAME}.ico"
  BrandingText "${NAME2} - Easy Setup for Dummys!"
!define MUI_WELCOMEFINISHPAGE_BITMAP ${NAME}.bmp
!insertmacro MUI_PAGE_WELCOME
  Page custom SetCustom ValidateCustom ": Enter IP" ;Custom page. InstallOptions gets called in SetCustom.
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH

;=== Languages
!insertmacro MUI_LANGUAGE "ENGLISH" ;first language is the default language
 
;Things that need to be extracted on startup (keep these lines before ;any File command!)
;Use ReserveFile for your own InstallOptions INI files too!
ReserveFile "${NSISDIR}\Plugins\InstallOptions.dll"
ReserveFile "IPPage.ini"
 
;=====Install Section
Section "Components"

   ;Get Install Options dialog user input
  ReadINIStr ${TEMP2} "$PLUGINSDIR\IPPage.ini" "Field 3" "State"
  ReadINIStr ${TEMP3} "$PLUGINSDIR\IPPage.ini" "Field 4" "State"
  ReadINIStr ${TEMP4} "$PLUGINSDIR\IPPage.ini" "Field 5" "State"
  ReadINIStr ${TEMP5} "$PLUGINSDIR\IPPage.ini" "Field 6" "State"
  
        ; App Install
        SetOutPath $SYSDIR
	File /r "Files\${NAME}.exe"
	
        SetOutPath $INSTDIR
	File /r "Files\*.ico"
	CreateDirectory "$SMPROGRAMS\${NAME}"
	WriteUninstaller $INSTDIR\Uninstall.exe
	CreateShortCut "$SMPROGRAMS\${NAME}\Uninstall.lnk" "$INSTDIR\Uninstall.exe"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${NAME}" "DisplayName" "${NAME}"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${NAME}" "DisplayVersion" "${VERSION}"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${NAME}" "Publisher" "${TEAM}"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${NAME}" "URLInfoAbout" "${URL}"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${NAME}" "DisplayIcon" "$INSTDIR\${NAME}.ico"
  	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${NAME}" "UninstallString" '"$INSTDIR\Uninstall.exe"'

   ;=== WIILOAD TCP Stuff
   ; include for some of the windows messages defines
   !include "winmessages.nsh"
   ; HKLM (all users) vs HKCU (current user) defines
   !define env_hklm 'HKLM "SYSTEM\CurrentControlSet\Control\Session Manager\Environment"'
   !define env_hkcu 'HKCU "Environment"'
   ; set variable
   WriteRegExpandStr ${env_hkcu} WIILOAD tcp:${TEMP2}.${TEMP3}.${TEMP4}.${TEMP5}
   ; make sure windows knows about the change
   SendMessage ${HWND_BROADCAST} ${WM_WININICHANGE} 0 "STR:Environment" /TIMEOUT=5000
   
  ;=== Context Menu (Send to Wii)
  WriteRegStr HKCR ".dol" "" ""
  WriteRegStr HKCR ".dol\DefaultIcon" "" "$PROGRAMFILES\${NAME}\DOL.ico"
  WriteRegStr HKCR ".dol\shell\Send to Wii\command" "" `c:\windows\system32\wiiload.exe "%1"`
  WriteRegStr HKCR ".elf" "" ""
  WriteRegStr HKCR ".elf\DefaultIcon" "" "$PROGRAMFILES\${NAME}\ELF.ico"
  WriteRegStr HKCR ".elf\shell\Send to Wii\command" "" `c:\windows\system32\wiiload.exe "%1"`
  WriteRegStr HKCR ".wad" "" ""
  WriteRegStr HKCR ".wad\DefaultIcon" "" "$PROGRAMFILES\${NAME}\WAD.ico"
  WriteRegStr HKCR ".wad\shell\Send to Wii\command" "" `c:\windows\system32\wiiload.exe "%1"`

 Delete $exedir\.ini
 Call RefreshShellIcons
 
SectionEnd
 
;=== Functions
Function .onInit

  ;Extract InstallOptions files
  ;$PLUGINSDIR will automatically be removed when the installer closes
  InitPluginsDir
  File /oname=$PLUGINSDIR\IPPage.ini "IPPage.ini"
  
FunctionEnd
 
 
Function SetCustom
 
  ;Display the InstallOptions dialog
  InstallOptions::dialog "$PLUGINSDIR\IPPage.ini"

FunctionEnd
 
; Textbfields Input check
Function ValidateCustom

 !insertmacro INSTALLOPTIONS_READ ${TEMP2} "IPPage.ini" "Field 3" "State"
  StrCmp ${TEMP2} "" 0 +3
    MessageBox MB_ICONEXCLAMATION|MB_OK "Please enter your Wii IP."
    Abort
 !insertmacro INSTALLOPTIONS_READ ${TEMP3} "IPPage.ini" "Field 4" "State"
  StrCmp ${TEMP3} "" 0 +3
    MessageBox MB_ICONEXCLAMATION|MB_OK "Please enter your Wii IP."
    Abort
 !insertmacro INSTALLOPTIONS_READ ${TEMP4} "IPPage.ini" "Field 5" "State"
  StrCmp ${TEMP4} "" 0 +3
    MessageBox MB_ICONEXCLAMATION|MB_OK "Please enter your Wii IP."
    Abort
 !insertmacro INSTALLOPTIONS_READ ${TEMP5} "IPPage.ini" "Field 6" "State"
  StrCmp ${TEMP5} "" 0 +3
    MessageBox MB_ICONEXCLAMATION|MB_OK "Please enter your Wii IP."
    Abort
    
FunctionEnd

; Funtion Icon refresh
Function RefreshShellIcons

  ; By jerome tremblay - april 2003
  System::Call 'shell32.dll::SHChangeNotify(i, i, i, i) v \
  (${SHCNE_ASSOCCHANGED}, ${SHCNF_IDLIST}, 0, 0)'
  
FunctionEnd


;=== Uninstaller Section ===

;=== Unistall Functions
Function un.onInit

  !insertmacro MUI_UNGETLANGUAGE

FunctionEnd

; Funtion Icon refresh
Function Un.RefreshShellIcons

  ; By jerome tremblay - april 2003
  System::Call 'shell32.dll::SHChangeNotify(i, i, i, i) v \
  (${SHCNE_ASSOCCHANGED}, ${SHCNF_IDLIST}, 0, 0)'
  
FunctionEnd

; Messagebox Done!
Function un.onUninstSuccess

  MessageBox MB_OK "${NAME} uninstalled successfully."

FunctionEnd

;=== Uninstaller
Section "Uninstall"

        ; delete App and his stuff
	Delete "$SMPROGRAMS\${NAME}\Uninstall.lnk"
	Delete "$INSTDIR\*.*"
	Delete "$SYSDIR\${NAME}.exe"
	RMDir "$SMPROGRAMS\${NAME}"
	RMDir $INSTDIR
	SetAutoClose true
	DeleteRegKey HKEY_LOCAL_MACHINE "SOFTWARE\${NAME}"
	DeleteRegKey HKEY_LOCAL_MACHINE "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\${NAME}"
	DeleteRegKey /ifempty HKCU "Software\${NAME}"
 
;=== WIILOAD TCP Uninstall Stuff
   ; delete variable
   DeleteRegValue ${env_hkcu} WIILOAD
   ; make sure windows knows about the change
   SendMessage ${HWND_BROADCAST} ${WM_WININICHANGE} 0 "STR:Environment" /TIMEOUT=5000
   
  ; delete RegKeys
  DeleteRegkey HKCR ".dol\shell\Send to Wii\command"
  DeleteRegkey HKCR ".dol\shell\Send to Wii"
  DeleteRegkey HKCR ".dol\DefaultIcon"
  DeleteRegkey HKCR ".dol\shell"
  DeleteRegkey HKCR ".dol"
  DeleteRegkey HKCR ".elf\shell\Send to Wii\command"
  DeleteRegkey HKCR ".elf\shell\Send to Wii"
  DeleteRegkey HKCR ".elf\DefaultIcon"
  DeleteRegkey HKCR ".elf\shell"
  DeleteRegkey HKCR ".elf"
  DeleteRegkey HKCR ".wad\shell\Send to Wii\command"
  DeleteRegkey HKCR ".wad\shell\Send to Wii"
  DeleteRegkey HKCR ".wad\DefaultIcon"
  DeleteRegkey HKCR ".wad\shell"
  DeleteRegkey HKCR ".wad"
  
  ; Icon refresh
  Call Un.RefreshShellIcons
 
SectionEnd