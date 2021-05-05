# MSVC NMake Makefile

# to build this project, do the following steps:
# 1.open Visual Studio Developer Command Prompt (for example: open "x64 Native Tools Command Prompt for VS 2019")
# 2.change the working directory to the project directory 
# 3.type "nmake build"
#
# if you whant to edit the source code, you can use Visual Studio Code.
#
# you can also create a project from Visual Studio IDE(the project must be C++/WinRT Desktop Application) add the source files.

# NOTICE: This Project replaced some default libraries so that the program can run on the system without VC Runtime
# Learn more about C++/WinRT here: http://aka.ms/cppwinrt/

# default libraries replacements:
# vcruntime.lib -> api-ms-win-crt-private-l1-1-0.lib (extracted from api-ms-win-crt-private-l1-1-0.dll)
# msvcprt.lib -> msvcp_win.lib (extracted from msvcp_win.dll)

# Debug:
# CPPFLAGS=/nologo /D "_UNICODE" /D "UNICODE" /MD /std:c++17 /EHsc /await /bigobj /utf-8 /Zi
# Release:
# CPPFLAGS=/nologo /D "_UNICODE" /D "UNICODE" /MD /std:c++17 /EHsc /await /bigobj /utf-8 /O2 /Ob2 /GL

build: Launcher.exe

Launcher.exe: pch.pch pch.obj Launcher.cpp
	$(CPP) $(CPPFLAGS) /Yu"pch.h" /Fp: "pch.pch" Launcher.cpp /link pch.obj /NODEFAULTLIB:msvcrt.lib store/msvcrt.lib WindowsApp.lib

pch.pch: pch.h pch.cpp
	$(CPP) $(CPPFLAGS) /c /Yc"pch.h" /Fp: "pch.pch" pch.cpp

clean:
	@for %i in (*.obj *.pch *.ilk *.pdb *.exe) do @del "%i"
