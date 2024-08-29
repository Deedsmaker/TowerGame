@echo off

IF NOT EXIST build mkdir build
pushd build
clang ..\main.cpp -o main.exe -L..\lib -lmsvcrt -lraylib -lOpenGL32 -lGdi32 -lWinMM -lkernel32 -lshell32 -lUser32 -Xlinker /NODEFAULTLIB:libcmt --target=x86_64-pc-windows-msvc -g -d_CRT_SECURE_NO_WARNINGS
popd
