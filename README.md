# XConPty

[![Licence](https://img.shields.io/github/license/Biswa96/XConPty.svg?style=for-the-badge)](https://www.gnu.org/licenses/gpl-3.0.en.html)
[![Top Language](https://img.shields.io/github/languages/top/Biswa96/XConPty.svg?style=for-the-badge)](https://github.com/Biswa96/XConPty.git)
[![Code size](https://img.shields.io/github/languages/code-size/Biswa96/XConPty.svg?style=for-the-badge)]()

Proof-of-concept new Pseudo Console APIs' implementation in Windows 10 19H1 builds or greater. This repository uses many low level NTDLL APIs (without many error checking steps) and is dedicated to educational purposes. Use the executable file mainly for testing purposes and exploring Windows Consoles. This is not a pseudo terminal like in UNIX or UNIX-Like systems. 

## How to use

Download the compiled executable file from [Release page](https://github.com/Biswa96/XConPty/releases). Double clicking on `XConPty.exe` will ping `localhost` aka. `127.0.0.1` for 10 seconds only. The ping time and ping count can be changed in source code. Use the `FunXConPty.exe` for the FUN_MODE (see below).

## Fun Mode

This mode shows behind-the-scene view by disabling/removing some flags and strings. Enable this mode by compiling with `/DFUN_MODE` in MSVC (i.e. with cl.exe) or add `make CCOPT=-DFUN_MODE` with gcc in msys2/mingw-w64 toolchain. For further details see `build.bat`, a batch file with `cl.exe` command. So, what does this mode do? 

<img align=right src=images\Ping_Fun_Mode.PNG>

* Reveals the hidden ConHost.exe window attached with master side. 
 For example, here XConPty.exe process. 
* Does not enable VT-100 escape sequence processing mode. 
 As a result, it shows all raw escape character buffers. 
* Remains infinitely until user press Ctrl+C from keyboard. 
 Closing the reference ConHost window results
 an ORPHAN CONHOST PROCESS. Cool.... :sunglasses: 

## How to build

Clone this repository. Open the solution (.sln) or project (.vcxproj) file in Visual Studio and build it. Alternatively, run Visual Studio developer command prompt, go to the cloned folder and run this command `msbuild.exe`. Or run the `build.bat` for release mode. You can also build with mingw-w64 toolchain. Go to the cloned folder in terminal, run `make` command from msys2/mingw-w64 toolchain. 

## Project Overview

If you are interested only Pseudo Console APIs those are written in `PseudoCosole.c` file. Many required Kernel32 files are written from scratch for better understanding but without many error checking steps. To eliminate confusion, All Kernel32 function names are prefixed with `X_`. If you want to use only `XConPty.c` file remove `X_` prefix from all the Kernel32 APIs. Here are the overview of source files according to their dependencies: 

```
src\
    |
    +-- WinInternal: Required NT APIs declarations and associates structures
        |
        +-- KernelBase: Required Kernel32 APIs written from scratch
            |
            +-- PseudoConsole: Pseudo Console APIs based on Kernel32 APIs
                |
                +-- XConPty: Sample to show the usage of Pseudo Console APIs
```

The interesting thing is that `HPCON` data type isn't `void*` only. Did you find it? :mag_right: 

## Acknowledgments

Thanks to:

* ConPty sample [EchoCon](https://github.com/Microsoft/console/tree/master/samples/ConPTY/EchoCon) 
* ProcessHacker's collection of [native API header file](https://github.com/processhacker/processhacker/tree/master/phnt) 
* Rohitab's thread of [PROC_THREAD_ATTRIBUTE_LIST structure](http://www.rohitab.com/discuss/topic/38601-proc-thread-attribute-list-structure-documentation/) 

## License 

This project is licensed under GNU Public License v3 or higher. You are free to study, modify or distribute the source code. 

```
XConPty -- Copyright (C) 2018-19 Biswapriyo Nath

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
