# XConPty

Reveal hidden new Pseudo Console APIs in Windows 10 19H1 builds or greater. This repository uses many low level NTDLL APIs (without many error checking steps) and is dedicated to educational purposes. Use the executable file mainly for testing purposes and exploring Windows Consoles. 

## How to use

The executable is a standalone program. Double clicking on it will Ping `localhost` aka. `127.0.0.1` for 10 seconds only. The ping time and ping count can be changed in source code. 

## How to build

Clone this repository. Open the solution (.sln) or project (.vcxproj) file in Visual Studio and build it. ALternatively, run Visual Studio developer command prompt, go to the cloned folder and run this command: `msbuild.exe /p:Configuration=Debug`. Or run the `build.bat` for release mode. You can also build with mingw-w64 toolchain. Go to the folder in terminal run `mingw32-make` command from mingw-w64/msys2/cygwin. Some values are not defined in mingw-w64 toolchain. It will be updated soon. 

## Project Overview

If you are interested only Pseudo Console APIs those are written in `PseudoCosole.c` file. Many required Kernel32 files are written from scratch for better understanding but without many error checking codes. To eliminate confusion, All Kernel32 function names are appended with `X_`. If you want to use only `XConPty.c` file remove `X_` from all the Kernel32 APIs. Here are the overview of source files according to its dependency: 

```
src\
    |
    +-- ntdll: Required NT APIs declarations and associates structures
        |
        +-- KernelBase: Required Kernel32 APIs written from scratch
            |
            +-- PseudoConsole: Pseudo Console APIs based on Kernel32 APIs
                |
                +-- XConPty: Sample to show the usage of Pseudo Console APIs
```

The interesting thing is that `HPCON` isn't `void*` only. Did you find it?

## Acknowledgments

Thanks to:

* ConPty sample [EchoCon](https://github.com/Microsoft/console/tree/master/samples/ConPTY/EchoCon) 
* ProcessHacker's collection of [native API header file](https://github.com/processhacker/processhacker/tree/master/phnt) 
* Rohitab's thread of [PROC_THREAD_ATTRIBUTE_LIST structure](http://www.rohitab.com/discuss/topic/38601-proc-thread-attribute-list-structure-documentation/) 

## License 

This project is licensed under GNU Public License v3 or higher. You are free to study, modify or distribute the source code. 

```
XConPty -- (c) Copyright 2018 Biswapriyo Nath

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
```
