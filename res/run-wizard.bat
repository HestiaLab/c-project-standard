@echo off
setlocal
"%~dp0cproject.exe" new %*
set "EXIT_CODE=%ERRORLEVEL%"
echo.
pause
exit /b %EXIT_CODE%
