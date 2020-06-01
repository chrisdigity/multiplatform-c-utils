@echo off
cls

REM ################
REM # Enable delayed expansion on script

setlocal ENABLEEXTENSIONS ENABLEDELAYEDEXPANSION

REM ################
REM # Initialize environment

call "C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvars64.bat"
if %errorlevel% EQU 0 goto ENVOK
call "C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvars32.bat"
if %errorlevel% NEQ 0 (
   echo Failed to find Visual Studio Developer Native tools!
   echo Please install the latest version of
   echo Microsoft Visual Studio 2017 Community Edition.
   goto END
)
:ENVOK

echo.
echo   testWIN.bat - Multiplatform Utilities compilation and execution script
echo                 thread.c, time.c
echo.

REM ################
REM # Remove old error log

set LOG="mputilstest.log"
del /f /q %LOG% 1>NUL 2>&1

REM ################
REM # Build test software

echo | set /p="Building Multiplatform Utility tests... "
cl /nologo /WX /Femputilstest.exe test\mputilstest.c >>%LOG% 2>&1

if %errorlevel% NEQ 0 (
   echo Error
   echo.
   more %LOG%
) else (
   echo OK
   echo.
   echo Done.

REM ################
REM # Run software on success

   start "" /b /wait "mputilstest.exe"

)

REM ################
REM # Cleanup

if exist "mputilstest.obj" del /f /q "mputilstest.obj"
if exist %LOG% del /f /q %LOG%


:END

echo.
pause
EXIT
