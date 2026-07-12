; Desktop shortcut installer helper — included by CPack NSIS installer.
; The installer runs as admin (SetShellVarContext all), so $DESKTOP is
; the Public Desktop.  We also read USERPROFILE to create a shortcut on
; the actual user's desktop.

!ifndef CPACK_UNINSTALL
  ; --- Install: create shortcuts ---
  SetShellVarContext all
  CreateShortCut "$DESKTOP\CProjectStandard.lnk" "$INSTDIR\bin\run-wizard.bat"
  ReadEnvStr $0 USERPROFILE
  CreateShortCut "$0\Desktop\CProjectStandard.lnk" "$INSTDIR\bin\run-wizard.bat"
!else
  ; --- Uninstall: delete shortcuts ---
  SetShellVarContext all
  Delete "$DESKTOP\CProjectStandard.lnk"
  ReadEnvStr $0 USERPROFILE
  Delete "$0\Desktop\CProjectStandard.lnk"
!endif
