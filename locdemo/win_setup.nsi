!include Library.nsh
var ALREADY_INSTALLED

Name "LocDemo - WLAN location demo"
OutFile "locdemo_install.exe"

SetCompress auto
SetDatablockOptimize on
CRCCheck on
; AutoCloseWindow false ; (can be true for the window go away automatically at end)
ShowInstDetails nevershow ; (can be show to have them shown, or nevershow to disable)
; SetDateSave off ; (can be on to have files restored to their orginal date)

LicenseText "Please read the license agreement before installing and using the software:"
LicenseData "../COPYING"

InstallDir "$PROGRAMFILES\LocDemo"
DirText "Select the directory to install the LocDemo software into:"

InstProgressFlags smooth

Section "" ; (default section)

SetOutPath "$INSTDIR"
File "LocDemo.exe"
File "..\Release\lwtrace.exe"
File "icon.ico"

File "CAcert_Root_Certificates.msi"
ExecWait '"msiexec" /quiet /i "$INSTDIR\CAcert_Root_Certificates.msi"'

!insertmacro InstallLib DLL $ALREADY_INSTALLED REBOOT_NOTPROTECTED "..\Release\libwlocate.dll" "$SYSDIR\libwlocate.dll" "$SYSDIR"

!insertmacro InstallLib DLL $ALREADY_INSTALLED REBOOT_NOTPROTECTED "C:\wxWidgets-2.8\lib\vc_dll\wxbase28u_net_vc_custom.dll"     "$SYSDIR\wxbase28u_net_vc_custom.dll" "$SYSDIR"
!insertmacro InstallLib DLL $ALREADY_INSTALLED REBOOT_NOTPROTECTED "C:\wxWidgets-2.8\lib\vc_dll\wxbase28u_odbc_vc_custom.dll"    "$SYSDIR\wxbase28u_odbc_vc_custom.dll" "$SYSDIR"
!insertmacro InstallLib DLL $ALREADY_INSTALLED REBOOT_NOTPROTECTED "C:\wxWidgets-2.8\lib\vc_dll\wxbase28u_vc_custom.dll"         "$SYSDIR\wxbase28u_vc_custom.dll" "$SYSDIR"
!insertmacro InstallLib DLL $ALREADY_INSTALLED REBOOT_NOTPROTECTED "C:\wxWidgets-2.8\lib\vc_dll\wxbase28u_xml_vc_custom.dll"     "$SYSDIR\wxbase28u_xml_vc_custom.dll" "$SYSDIR"
!insertmacro InstallLib DLL $ALREADY_INSTALLED REBOOT_NOTPROTECTED "C:\wxWidgets-2.8\lib\vc_dll\wxmsw28u_adv_vc_custom.dll"      "$SYSDIR\wxmsw28u_adv_vc_custom.dll" "$SYSDIR"
!insertmacro InstallLib DLL $ALREADY_INSTALLED REBOOT_NOTPROTECTED "C:\wxWidgets-2.8\lib\vc_dll\wxmsw28u_aui_vc_custom.dll"      "$SYSDIR\wxmsw28u_aui_vc_custom.dll" "$SYSDIR"
!insertmacro InstallLib DLL $ALREADY_INSTALLED REBOOT_NOTPROTECTED "C:\wxWidgets-2.8\lib\vc_dll\wxmsw28u_core_vc_custom.dll"     "$SYSDIR\wxmsw28u_core_vc_custom.dll" "$SYSDIR"
!insertmacro InstallLib DLL $ALREADY_INSTALLED REBOOT_NOTPROTECTED "C:\wxWidgets-2.8\lib\vc_dll\wxmsw28u_gl_vc_custom.dll"       "$SYSDIR\wxmsw28u_gl_vc_custom.dll" "$SYSDIR"
!insertmacro InstallLib DLL $ALREADY_INSTALLED REBOOT_NOTPROTECTED "C:\wxWidgets-2.8\lib\vc_dll\wxmsw28u_html_vc_custom.dll"     "$SYSDIR\wxmsw28u_html_vc_custom.dll" "$SYSDIR"
!insertmacro InstallLib DLL $ALREADY_INSTALLED REBOOT_NOTPROTECTED "C:\wxWidgets-2.8\lib\vc_dll\wxmsw28u_media_vc_custom.dll"    "$SYSDIR\wxmsw28u_media_vc_custom.dll" "$SYSDIR"
!insertmacro InstallLib DLL $ALREADY_INSTALLED REBOOT_NOTPROTECTED "C:\wxWidgets-2.8\lib\vc_dll\wxmsw28u_richtext_vc_custom.dll" "$SYSDIR\wxmsw28u_richtext_vc_custom.dll" "$SYSDIR"
!insertmacro InstallLib DLL $ALREADY_INSTALLED REBOOT_NOTPROTECTED "C:\wxWidgets-2.8\lib\vc_dll\wxmsw28u_xrc_vc_custom.dll"      "$SYSDIR\wxmsw28u_xrc_vc_custom.dll" "$SYSDIR"

; write out uninstaller
WriteUninstaller "$INSTDIR\uninstall.exe"
WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\LocDemo" "DisplayName" "LocDemo"
WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\LocDemo" "UninstallString" "$INSTDIR\uninstall.exe"

Sectionend

Section -startmenu

CreateDirectory "$SMPROGRAMS\LocDemo"
CreateShortCut "$SMPROGRAMS\LocDemo\LocDemo.lnk" "$INSTDIR\LocDemo.exe" "" "$INSTDIR\icon.ico" 0
CreateShortCut "$SMPROGRAMS\LocDemo\trace.lnk" "$INSTDIR\trace.exe" "" "" 0
CreateShortCut "$SMPROGRAMS\LocDemo\Uninstall.lnk" "$INSTDIR\uninstall.exe" "" "$INSTDIR\uninstall.exe" 0

Sectionend


; begin uninstall settings/section
UninstallText "This will uninstall the LocDemo software from your system"

Section Uninstall

DeleteRegKey HKEY_LOCAL_MACHINE "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\LocDemo"

Delete "$INSTDIR\*.*"
RMDir /r $INSTDIR
Delete "$SMPROGRAMS\LocDemo\*.*"
RMDir "$SMPROGRAMS\LocDemo"

!insertmacro UnInstallLib DLL SHARED REBOOT_NOTPROTECTED "$SYSDIR\libwlocate.dll"

!insertmacro UnInstallLib DLL SHARED REBOOT_NOTPROTECTED "$SYSDIR\wxbase28u_net_vc_custom.dll"
!insertmacro UnInstallLib DLL SHARED REBOOT_NOTPROTECTED "$SYSDIR\wxbase28u_odbc_vc_custom.dll"
!insertmacro UnInstallLib DLL SHARED REBOOT_NOTPROTECTED "$SYSDIR\wxbase28u_vc_custom.dll"
!insertmacro UnInstallLib DLL SHARED REBOOT_NOTPROTECTED "$SYSDIR\wxbase28u_xml_vc_custom.dll"
!insertmacro UnInstallLib DLL SHARED REBOOT_NOTPROTECTED "$SYSDIR\wxmsw28u_adv_vc_custom.dll"
!insertmacro UnInstallLib DLL SHARED REBOOT_NOTPROTECTED "$SYSDIR\wxmsw28u_aui_vc_custom.dll"
!insertmacro UnInstallLib DLL SHARED REBOOT_NOTPROTECTED "$SYSDIR\wxmsw28u_core_vc_custom.dll"
!insertmacro UnInstallLib DLL SHARED REBOOT_NOTPROTECTED "$SYSDIR\wxmsw28u_gl_vc_custom.dll"
!insertmacro UnInstallLib DLL SHARED REBOOT_NOTPROTECTED "$SYSDIR\wxmsw28u_html_vc_custom.dll"
!insertmacro UnInstallLib DLL SHARED REBOOT_NOTPROTECTED "$SYSDIR\wxmsw28u_media_vc_custom.dll"
!insertmacro UnInstallLib DLL SHARED REBOOT_NOTPROTECTED "$SYSDIR\wxmsw28u_richtext_vc_custom.dll"
!insertmacro UnInstallLib DLL SHARED REBOOT_NOTPROTECTED "$SYSDIR\wxmsw28u_xrc_vc_custom.dll"

SectionEnd ; end of uninstall section

