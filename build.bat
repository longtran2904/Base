@echo off

REM 4057 indirection to `slightly` different base types
REM WTF does `slightly` mean? In this codebase, it complained about (char*/LPSTR) <-> (u8*)

REM 4100 unused parameter
REM 4101 unused local variable
REM 4127 constant expression of `if` and `while` (`if (1 == 1)` will emit a warning but not `if (1)`)
REM 4146 unary minus on unsigned types

REM 4189 local variable is initialized but unused
REM This is stupid because `Foo foo = bar ? (Foo){...} : (Foo){...}` emits a warning
REM I really want to use this warning but MSVC must fix this

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

REM --The flags below are for the analyzer--

REM 6011 and 28182: derefernecing NULL pointer
REM 6250 calling 'VirtualFree' without the MEM_RELEASE flag might cause address space leaks
REM 6334 sizeof operator applied to an expression with an operator may yield unexpected results
REM 6387 invalid param value

REM 28251 Inconsistent annotation for function
REM This only reports the WinMain entry function in the codebase

set warns=-D_CRT_SECURE_NO_WARNINGS /W4 /WX /wd4057 /wd4201 /wd4389 /wd4189
REM set warns=%warns% /wd4706

REM ---unused  flags---
REM set warns=%warns% /wd4100 /wd4101
REM ---analyze flags---
REM set warns=%warns% /analyze /wd28251

REM FC Full path of source code file in diagnostics
REM GR Enables RTTI
REM EH Enables standard C++ stack unwinding
REM WL Enables One-Line Diagnostics (appends additional information to an error or warning message)
REM JMC Just my code debug
REM FA generates an assembler listing file (s includes source code while c includes machine code)

set opts=/FC /GR- /EHa- /nologo /Zi %warns%
REM set opts=%opts% /fsanitize=address /JMC
if [%1]==[release] (set opts=%opts% /O2 && echo [release mode]) ELSE echo [debug mode]

set code=%cd%
IF NOT EXIST build mkdir build
pushd build

del *.pdb > NUL 2> NUL
del *.exe > NUL 2> NUL
del *.dll > NUL 2> NUL

set links=/incremental:no Winmm.lib Userenv.lib Advapi32.lib User32.lib Gdi32.lib Dwmapi.lib
set opts=%opts% /I%code%\code /I%code%\code\dependencies fast_float.obj

IF NOT EXIST "fast_float.obj" cl /nologo /GR- /EHa- /nologo /O2 /c %code%\code\dependencies\fast_float.cpp

cl %opts% %code%\code\examples\test_base.c /Fetest_base /LD /link %links%
cl %opts% %code%\code\examples\demo_gfx.c  /Fedemo_gfx /link %links%
cl %opts% %code%\code\examples\demo.c      /Fedemo /link %links%
cl %opts% %code%\code\Metamain.c           /Femetagen /link %links%
cl %opts% %code%\code\examples\TestDLL.c   /FeTestDLL /LD

REM cl %opts% %code%\code\examples\glob.c      /Feglob /O2 /link %link%
REM cl %opts% %code%\code\examples\test_glob.c /Fetest_glob /link %link%
REM cl %opts% %code%\code\examples\parser.c    /Feparser -wd4706 /link %links%
REM cl %opts% %code%\code\retired\D3D11_Example.c /Fed3d11_exp /link %links%
REM cl %opts% %code%\code\retired\LongCompressor.c /Fecompressor /link %links%
REM cl %opts% %code%\code\retired\Meta.c /FeMeta /link %links%

ren fast_float.obj fast_float.temp > NUL 2> NUL
del *.obj > NUL 2> NUL
ren fast_float.temp fast_float.obj > NUL 2> NUL 

del *.lib > NUL 2> NUL
del *.ilk > NUL 2> NUL
del *.exp > NUL 2> NUL
del *.xml > NUL 2> NUL

popd
