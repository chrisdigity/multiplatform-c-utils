@echo off
cls
::
:: Testing makefile (Windows) to compile and test all *.c files.
::  - makefile.bat (1 June 2020)
::
:: Original work Copyright (c) 2020 Zalamanda
::
:: Permission is hereby granted, free of charge, to any person obtaining a
:: copy of this software and associated documentation files (the "Software"),
:: to deal in the Software without restriction, including without limitation
:: the rights to use, copy, modify, merge, publish, distribute, sublicense,
:: and/or sell copies of the Software, and to permit persons to whom the
:: Software is furnished to do so, subject to the following conditions:
::
:: The above copyright notice and this permission notice shall be included
:: in all copies or substantial portions of the Software.
::

::
:: Initialize environment

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

::
:: Remove old error log

set LOG="error.log"
del /f /q %LOG% 1>NUL 2>&1

::
:: Build test software

for %%f in (*.c) do (
   echo | set /p="Building %%~nf test... "
   cl /nologo /WX /W4 /Fe%%~nf.exe %%~nf.c >>%LOG% 2>&1
   if exist %%~nf.exe (
      echo OK

REM Run software on success
      start "" /b /wait %%~nf.exe

   ) else (
      echo Error
      echo.
      more %LOG%
      goto CLEANUP
   )
)

echo.
echo Done.

::
:CLEANUP
echo.
pause

if exist "*.exe" del /f /q *.exe
if exist "*.obj" del /f /q *.obj
if exist %LOG% del /f /q %LOG%

::
:END

EXIT
