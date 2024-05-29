@echo off

REM 4057 indirection to `slightly` different base types
REM WTF does `slightly` mean? In this codebase, it complained about (char*/LPSTR) <-> (u8*)

REM 4100 unused parameter
REM 4101 unused local variable
REM 4127 constant expression of `if` and `while` (`if (1 == 1)` will emit a warning but not `if (1)`)
REM 4146 unary minus on unsigned types

REM 4189 local variable is initialized but unused
REM This is stupid because `Foo foo = bar ? (Foo){...} : (Foo){...}` emits a warning

REM 4201 nameless struct/union
REM 4245 sign conversions

REM 4310 constant to smaller type
REM 4305 a value is converted to a smaller type in an initialization or as a constructor argument,
REM 4389 !=/== signed/unsigned mismatch

REM 4457 local names conflict with parameter names

REM 4505 unreferenced local function has been removed
REM I haven't been able to reproduce this warning. Maybe this is only for C++ functions inside a namespace.

REM 4701 using unitinialized variables
REM 4706 assignment within conditional expression (`if (a = b)`)

set warns=-D_CRT_SECURE_NO_WARNINGS /W4 /WX /wd4057 /wd4201 /wd4389 /wd4189
REM set warns=%warns% /wd4706

REM ---unused  flags---
REM set warns=%warns% /wd4100 /wd4101
REM ---analyze flags---
REM set warns=%warns% /analyze /wd6334

REM FC Full path of source code file in diagnostics
REM GR Enable RTTI
REM EH Enables standard C++ stack unwinding
REM WL Enable One-Line Diagnostics (appends additional information to an error or warning message)

set opts=/FC /GR- /EHa- /nologo /Zi %warns%
set opts=%opts% /fsanitize=address /JMC
if [%1]==[release] set opts=%opts% /O2

set code=%cd%
set links=/incremental:no Winmm.lib Userenv.lib Advapi32.lib User32.lib Gdi32.lib Dwmapi.lib fast_float.lib

IF NOT EXIST build mkdir build
pushd build

del *.pdb > NUL 2> NUL
del *.exe > NUL 2> NUL
del *.dll > NUL 2> NUL

set opts=%opts% /I%code%\code /I%code%\code\dependencies

REM del fast_float.lib > NUL 2> NUL
REM cl /nologo /GR- /EHa- /nologo /O2 /TP /c %code%\code\dependencies\fast_float.h
REM lib /nologo fast_float.obj

cl %opts% %code%\code\examples\test_base.c /Fetest_base.exe /link %links%
cl %opts% %code%\code\examples\demo_gfx.c  /Fedemo_gfx.exe /link %links%
cl %opts% %code%\code\examples\demo.c      /Fedemo.exe /link %links%
cl %opts% %code%\code\examples\parser.c    /Feparser.exe /link %links%
cl %opts% %code%\code\examples\TestDLL.c   /FeTestDLL.dll /LD

REM cl %opts% %code%\code\retired\D3D11_Example.c /Fed3d11_exp.exe /link %links%
REM cl %opts% %code%\code\retired\LongCompressor.c /Fecompressor.exe /link %links%
REM cl %opts% %code%\code\retired\Meta.c /FeMeta.exe /link %links%

REM del *.lib > NUL 2> NUL
del TestDLL.lib

del *.obj > NUL 2> NUL
del *.ilk > NUL 2> NUL
del *.exp > NUL 2> NUL

popd
