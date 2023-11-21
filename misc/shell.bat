@echo off
pushd .
call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
popd
set path=%cd%;%path%
set _NO_DEBUG_HEAP=1