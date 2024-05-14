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

set warns=-D_CRT_SECURE_NO_WARNINGS /W4 /WX /wd4057 /wd4201 /wd4389
set warns=%warns% /wd4100
set warns=%warns% /wd4189
REM set warns=%warns% /wd4101
set warns=%warns% /wd4706

REM FC Full path of source code file in diagnostics
REM GR Enable RTTI
REM EH Enables standard C++ stack unwinding
REM WL Enable One-Line Diagnostics (appends additional information to an error or warning message)

set opts=-FC -GR- -EHa- -WL -nologo -Zi %warns%
if [%1]==[release] set opts=%opts% -O2

set code=%cd%
set links=-incremental:no Winmm.lib Userenv.lib Advapi32.lib User32.lib Gdi32.lib Dwmapi.lib

IF NOT EXIST build mkdir build
pushd build

del *.pdb > NUL 2> NUL
del *.lib > NUL 2> NUL

cl %opts% %code%\code\TestDLL.c -FeTestDLL.dll /LD
cl %opts% %code%\code\main.c    -Femain.exe -link %links% TestDLL.lib
cl %opts% %code%\code\TestDLL.c -FeTestDLL.dll /LD -link main.lib

REM cl %opts% %code%\code\D3D11_Example.c -Fed3d11_exp.exe -link %links%
REM cl %opts% %code%\code\LongCompressor.c -Fecompressor.exe -link %links%
REM cl %opts% %code%\code\Meta.c -FeMeta.exe -link %links%
REM cl %opts% -E -C %code%\code\MetaTest.c >> %code%\code\generated\MetaTest.c

del *.obj > NUL 2> NUL
del *.ilk > NUL 2> NUL
del *.exp > NUL 2> NUL

popd
