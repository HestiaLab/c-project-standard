@echo off
setlocal
if exist "%USERPROFILE%\source\repos" (
    set "CPROJECT_DEFAULT_OUTPUT=%USERPROFILE%\source\repos"
) else (
    set "CPROJECT_DEFAULT_OUTPUT=%USERPROFILE%\Documents"
)
"%~dp0cproject.exe" new --interactive %*
set "EXIT_CODE=%ERRORLEVEL%"
echo.
pause
exit /b %EXIT_CODE%
