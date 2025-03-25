@echo off

:: Create build directory if it doesn't exist
IF NOT EXIST build mkdir build
pushd build

:: Set default options (debug mode with /Zi for debugging info)
SET options= -Zi -EHa -DDEBUG_BUILD=1
if "%~1"=="release" (SET options=-O2 -EHsc -DRELEASE_BUILD=1)

:: Compile with MSVC cl compiler
cl %options% ..\main.cpp /link /LIBPATH:..\lib vulkan-1.lib msvcrt.lib raylib.lib OpenGL32.lib Gdi32.lib WinMM.lib kernel32.lib shell32.lib User32.lib /OUT:main.exe

popd