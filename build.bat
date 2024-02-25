@echo off

set opts=-FC -GR- -EHa- -nologo -Zi
if [%1]==[release] set opts=%opts% -O2

set code=%cd%
set libs=Winmm.lib Userenv.lib Advapi32.lib User32.lib Gdi32.lib
pushd build
cl %opts% %code%\code\Meta.c -FeMeta.exe %libs%
cl %opts% %code%\code\TestDLL.c -FeTestDLL.dll /LD
cl %opts% %code%\code\main.c -Femain.exe %libs%
cl %opts% %code%\code\D3D11_Example.c -Fed3d11_exp.exe %libs%
cl %opts% %code%\code\LongCompressor.c -Fecompressor.exe %libs%
REM cl %opts% -E -C %code%\code\MetaTest.c >> %code%\code\generated\MetaTest.c
popd
