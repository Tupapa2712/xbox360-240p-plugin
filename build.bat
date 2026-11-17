@echo off
setlocal
set XEDK=C:\Program Files (x86)\Microsoft Xbox 360 SDK
set PATH=%XEDK%\bin\win32;%PATH%
set INCLUDE=%XEDK%\include\xbox
set LIB=%XEDK%\lib\xbox

cd /d "%~dp0"
if not exist build mkdir build

cl.exe /nologo /O2 /MT /W3 /D_XBOX /c /Fobuild\t240.obj src\t240.c
if errorlevel 1 exit /b 1

link.exe /nologo /DLL /BASE:0x81E00000 /OUT:build\t240.xex /XEXCONFIG:src\xex.xml /ENTRY:_DllMainCRTStartup /SUBSYSTEM:XBOX /NODEFAULTLIB:xapilibd.lib /NODEFAULTLIB:libcmtd.lib build\t240.obj xapilib.lib libcmt.lib xboxkrnl.lib
if errorlevel 1 exit /b 1

imagexex.exe /DUMP build\t240.xex
