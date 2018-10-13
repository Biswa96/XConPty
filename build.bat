@echo off
::Set Environments for X86_64 build
cd %~dp0
call "%ProgramFiles(x86)%\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
where cl.exe link.exe

::Set Environment Variables
set NAME=XConPty.exe
set BINDIR=bin
set CFLAGS=/c /Os /W4 /Fo%BINDIR%\\
set CCOPT=/DFUN_MODE
set LFLAGS=/MACHINE:X64
set LIBS=ntdll.lib Advapi32.lib

::Build
mkdir %BINDIR%

rem build normal mode
cl.exe %CFLAGS% src\*.c
link.exe %LFLAGS% %LIBS% %BINDIR%\*.obj /OUT:%BINDIR%\%NAME%

rem build fun mode
cl.exe %CFLAGS% %CCOPT% src\*.c
link.exe %LFLAGS% %LIBS% %BINDIR%\*.obj /OUT:%BINDIR%\Fun%NAME%

rem get stats
dir %BINDIR%\*.exe
