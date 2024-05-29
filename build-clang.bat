@echo off

set warns=-D_CRT_SECURE_NO_WARNINGS /W4 /WX /wd4057 /wd4201 /wd4389 /wd4189
set opts=/FC /GR- /EHa- /nologo /Zi %warns%
set opts=%opts% /fsanitize=address
if [%1]==[release] set opts=%opts% /O2

set code=%cd%
set links=/incremental:no Winmm.lib Userenv.lib Advapi32.lib User32.lib Gdi32.lib Dwmapi.lib

IF NOT EXIST build mkdir build
pushd build

del *.pdb > NUL 2> NUL
del *.lib > NUL 2> NUL
del *.exe > NUL 2> NUL
del *.dll > NUL 2> NUL

set opts=%opts% /I%code%\code

clang-cl -fms-compatibility -fms-extensions %opts% %code%\code\examples\test_base.c /Fetest_base.exe /link %links%
REM clang-cl %opts% %code%\code\examples\demo_gfx.c  /Fedemo_gfx.exe /link %links%
REM clang-cl %opts% %code%\code\examples\demo.c      /Fedemo.exe /link %links%
REM clang-cl %opts% %code%\code\examples\parser.c    /Feparser.exe /link %links%
REM clang-cl %opts% %code%\code\examples\TestDLL.c   /FeTestDLL.dll /LD

del *.obj > NUL 2> NUL
del *.ilk > NUL 2> NUL
del *.exp > NUL 2> NUL

popd
