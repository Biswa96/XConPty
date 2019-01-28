@echo off
::Set Environments for X86_64 build
cd %~dp0
call "%ProgramFiles(x86)%\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
where cl.exe link.exe

::Set Environment Variables
set NAME=XConPty.exe
set BINDIR=bin
set CFLAGS=/c /O1 /W4 /MD /nologo /Fo%BINDIR%\\
set CCOPT=/DFUN_MODE
set LFLAGS=/nologo /MACHINE:X64
set LIBS=ntdll.lib Advapi32.lib

::Build
@mkdir %BINDIR%

:: Build normal mode
cl.exe %CFLAGS% src\*.c
link.exe %LFLAGS% %LIBS% %BINDIR%\*.obj /OUT:%BINDIR%\%NAME%

:: Build fun mode
cl.exe %CFLAGS% %CCOPT% src\*.c
link.exe %LFLAGS% %LIBS% %BINDIR%\*.obj /OUT:%BINDIR%\Fun%NAME%

:: Get stats
dir /b %BINDIR%\*.exe
pause
exit /b
