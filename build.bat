@echo off

IF NOT EXIST build mkdir build
pushd build

SET options= -Zi -EHa -DDEBUG_BUILD=1
if "%~1"=="release" (SET options=-O2 -EHsc -DRELEASE_BUILD=1)

:: SET vulkan_libs= vulkan-1.lib shaderc_combined.lib

cl %options% /std:c++20 ..\main.cpp /link /LIBPATH:..\lib msvcrt.lib raylib.lib OpenGL32.lib Gdi32.lib WinMM.lib kernel32.lib shell32.lib User32.lib d3d11.lib d3dcompiler.lib /OUT:main.exe 

popd