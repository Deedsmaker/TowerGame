@echo off

::IF NOT EXIST build mkdir build
::pushd build
SET options=-g
if "%~1"=="release" (SET options=-O3)
clang -Wno-vla-extension ..\main.cpp -o main.exe -L..\lib -lmsvcrt -lraylib -lOpenGL32 -lGdi32 -lWinMM -lkernel32 -lshell32 -lUser32 -Xlinker /NODEFAULTLIB:libcmt --target=x86_64-pc-windows-msvc "%options%"
::popd
