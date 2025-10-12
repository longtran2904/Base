@echo off
setlocal enabledelayedexpansion
cd /d %~dp0

:: --- Unpack Arguments -------------------------------------------------------
for %%a in (%*) do set "%%a=1"
if not "%msvc%"=="1" if not "%clang%"=="1" set msvc=1
if not "%release%"=="1" set debug=1
if "%debug%"=="1"   set "release=0" && echo [debug mode]
if "%release%"=="1" set "debug=0" && echo [release mode]
if "%msvc%"=="1"    set "clang=0" && echo [msvc compile]
if "%clang%"=="1"   set "msvc=0" && echo [clang compile]

:: --- Setup Build Path -------------------------------------------------------
set code=%cd%
IF NOT EXIST build mkdir build
pushd build

:: --- Compile/Link Definitions -----------------------------------------------
if "%clang%"=="1" (
	:: --- build commands ---
	set compile=clang
	set out=-o
	set linker=--for-linker
	set dll=-shared -Wno-unused-command-line-argument

	:: --- base warnings ---
	set warns=-Wall -Wextra -Werror -Wshadow -Wdouble-promotion -Wconversion -D_CRT_SECURE_NO_WARNINGS
	set warns=!warns! -Wno-sign-conversion -Wno-pointer-sign -Wno-sign-compare -Wno-logical-not-parentheses -Wno-missing-braces
	set warns=!warns! -Wno-initializer-overrides -Wno-missing-field-initializers -Wno-incompatible-pointer-types

	:: --- unused flags ---
	set warns=!warns! -Wno-unused-local-typedef -Wno-unused-function -Wno-unused-variable
	:: --- Microsoft extensions ---
	set warns=!warns! -Wno-microsoft-enum-forward-reference -Wno-microsoft-string-literal-from-predefined -Wno-microsoft-anon-tag
	:: --- GNU extensions ---
	set warns=!warns! -Wno-gnu-null-pointer-arithmetic

	:: TODO: Fix 4coder indentation please
	set warns=!warns! -Wno-misleading-indentation
	:: TODO: Fix UB please
	set warns=!warns! -Wno-null-pointer-subtraction

	:: --- compile options ---
	set opts=-fno-exceptions -fno-cxx-exceptions -fno-async-exceptions -fno-rtti -fno-rtti-data -g3 -fms-compatibility -fms-extensions
	set opts=!opts! -march=native
	if "%asan%"=="1" set opts=!opts! -fsanitize=address
	if "%release%"=="1" set opts=!opts! -O2

	:: --- dependencies setup ---
	set opts=!opts! -I%code%\code -I%code%\code\dependencies fast_float.o
	set lib_opts=-fno-rtti -fno-exceptions -fno-cxx-exceptions -fno-async-exceptions -O2 -c
	set links=-incremental:no -lKernel32.lib -lWinmm.lib -lUserenv.lib -lAdvapi32.lib -lUser32.lib -lGdi32.lib -lDwmapi.lib
)

if "%msvc%"=="1" (
	:: --- build commands ---
	set compile=cl
	set out=/Fe
	set linker=/link
	set dll=/LD

	:: --- base warnings ---
	set warns=-D_CRT_SECURE_NO_WARNINGS /W4 /WX /wd4057 /wd4201 /wd4389 /wd4189

	:: --- unused flags ---
	REM set warns=!warns! /wd4100 /wd4101
	:: --- analyze flags ---
	REM set warns=!warns! /analyze /wd28251 /wd28182 /wd6287 /wd6387
	:: --- this might be useful ---
	set warns=!warns! /wd5287

	:: --- compile options ---
	set opts=/FC /GR- /EHa- /nologo /Zi /Zc:preprocessor
	set opts=!opts! /arch:AVX2  & :: MSVC doesn't have an equivalent to -march=native
	if "%asan%"=="1" set opts=!opts! /fsanitize=address /JMC
	if "%release%"=="1" set opts=!opts! /O2

	:: --- dependencies setup ---
	set opts=!opts! /I%code%\code /I%code%\code\dependencies fast_float.obj
	set lib_opts=/nologo /GR- /EHa- /O2 /c
	set links=/incremental:no Kernel32.lib Winmm.lib Userenv.lib Advapi32.lib User32.lib Gdi32.lib Dwmapi.lib
)

:: --- Build Dependencies -----------------------------------------------------
if "%msvc%"=="1" (
IF NOT EXIST "fast_float.obj" cl %lib_opts% %code%\code\dependencies\fast_float.cpp
)

if "%clang%"=="1" (
IF NOT EXIST "fast_float.o" clang++ %lib_opts% %code%\code\dependencies\fast_float.cpp
)

:: --- Build Everything -------------------------------------------------------
%code%\ctime -begin base.ctm

del *.pdb > NUL 2> NUL
del *.exe > NUL 2> NUL
del *.dll > NUL 2> NUL
del *.lib > NUL 2> NUL

:: %compile% %opts% %warns% %code%\code\examples\test_scanner.c %out%test_scanner.exe %linker% %links%
:: %compile% %opts% %warns% %code%\code\Metamain.c              %out%metagen.exe      %linker% %links%
:: %compile% %opts% %warns% %code%\code\retired\Meta.c           %out%Meta.exe       %linker% %links%

%compile% %opts% %warns% %code%\code\examples\demo.c         %out%demo.exe         %linker% %links%
:: %compile% %opts% %warns% %code%\code\examples\test_base.c    %out%test_base.exe    %linker% %links%
:: %compile% %opts% %warns% %code%\code\examples\TestDLL.c      %out%test.dll   %dll% %linker% %links%
:: %compile% %opts% %warns% %code%\code\examples\demo_gfx.c     %out%demo_gfx.exe     %linker% %links%

:: %compile% %opts% %warns% %code%\code\examples\lloc.c          %out%lloc.exe       %linker% %links%
%compile% %opts% %warns% %code%\code\examples\glob.c          %out%glob.exe       %linker% %links%
:: %compile% %opts% %warns% %code%\code\examples\test_glob.c     %out%test_glob.exe  %linker% %links%
:: %compile% %opts% %warns% %code%\code\examples\bench_mem.c     %out%bench_mem.exe  %linker% %links%
:: %compile% %opts% %warns% %code%\code\examples\print_args.c    %out%args.exe       %linker% %links%
:: %compile% %opts% %warns% %code%\code\retired\D3D11_Example.c  %out%d3d11_exp.exe  %linker% %links%
:: %compile% %opts% %warns% %code%\code\retired\LongCompressor.c %out%compressor.exe %linker% %links%

:: --- Cleanup Build ----------------------------------------------------------
ren fast_float.obj fast_float.obj.temp > NUL 2> NUL
ren fast_float.o     fast_float.o.temp > NUL 2> NUL
del *.obj > NUL 2> NUL
del *.o   > NUL 2> NUL
ren fast_float.obj.temp fast_float.obj > NUL 2> NUL
ren   fast_float.o.temp fast_float.o   > NUL 2> NUL

del *.ilk > NUL 2> NUL
del *.exp > NUL 2> NUL
del *.xml > NUL 2> NUL

%code%\ctime -end base.ctm
popd

:: --- MSVC References --------------------------------------------------------

:: --- Warning Flags ---
:: 4057 indirection to `slightly` different base types (it complained about char*/LPSTR <-> u8*)
:: 4100 unused parameter
:: 4101 unused local variable
:: 4127 constant expression of `if` and `while` (`if (1 == 1)` will emit a warning but not `if (1)`)
:: 4146 unary minus on unsigned types

:: 4189 local variable is initialized but unused
:: This is stupid because `Foo foo = bar ? (Foo){...} : (Foo){...}` emits a warning
:: I really want to use this warning but MSVC must fix this

:: 4201 nameless struct/union
:: 4245 sign conversions
:: 4310 constant to smaller type
:: 4305 a value is converted to a smaller type in an initialization or as a constructor argument,
:: 4389 !=/== signed/unsigned mismatch
:: 4457 local names conflict with parameter names
:: 4701 using unitinialized variables
:: 4706 assignment within conditional expression (`if (a = b)`)

:: --- Analyzer Flags ---
:: 6011 and 28182: derefernecing NULL pointer
:: 6250 calling 'VirtualFree' without the MEM_RELEASE flag might cause address space leaks
:: 6287 Redundant code
:: 6334 sizeof operator applied to an expression with an operator may yield unexpected results
:: 6387 invalid param value
:: 28251 Inconsistent annotation for function (currently only reports WinMain entry function)

:: --- Compiling Flags ---
:: FC Full path of source code file in diagnostics
:: GR Enables RTTI
:: EH Enables standard C++ stack unwinding
:: WL Enables One-Line Diagnostics (appends additional information to an error or warning message)
:: JMC Just my code debug
:: FA generates an assembler listing file (s includes source code while c includes machine code)
:: /Zc:preprocessor supports token pasting tricks like `, ##__VA_ARGS__`

:: --- BATCH References -------------------------------------------------------
:: 1. A code block can't have 2 comments start with `::` next to each other
:: https://stackoverflow.com/questions/19843849/unexpected-the-system-cannot-find-the-drive-specified-in-batch-file
:: 2. A code block will substitute the same variable into the same thing. For example:
::		set VAR=before
::		if "%VAR%"=="before" (
::			set VAR=after
::			if %VAR%=="after" @echo This will never run
::		)
:: This is why you must use `setlocal enabledelayedexpansion` and `!VAR!`
:: `setlocal` is also useful in keeping all the declared variables local to the current script
:: 3. `cd /d %~dp0` will set the current path to the batch file's location
:: https://stackoverflow.com/questions/18309941/what-does-it-mean-by-command-cd-d-dp0-in-windows
