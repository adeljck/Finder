@echo off
REM Wrapper to run PowerShell build script from cmd/Native Tools Prompt
setlocal
set PS_ARGS=
:parse
if "%~1"=="" goto run
set PS_ARGS=%PS_ARGS% %1
shift
goto parse
:run
powershell -ExecutionPolicy Bypass -File "%~dp0build.ps1" %PS_ARGS%
exit /b %ERRORLEVEL%

