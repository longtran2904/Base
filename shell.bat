@echo off
pushd .
REM call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
call D:\Programs\BuildTools\devcmd.bat
popd
REM set path=%cd%;%path%
set path=%path%;D:\Programs\LLVM\bin
set _NO_DEBUG_HEAP=1