@echo off

REM Base warnings
set warns=-Wall -Wextra -Werror -Wshadow -Wdouble-promotion -Wconversion -Wno-sign-conversion -Wno-pointer-sign -Wno-sign-compare -Wno-logical-not-parentheses

REM Fix 4coder indentation please
set warns=%warns% -Wno-misleading-indentation

REM Stupid CRT library
set warns=%warns% -D_CRT_SECURE_NO_WARNINGS

REM This must be off for StaticAssert to work
set warns=%warns% -Wno-unused-local-typedef

REM Useful C features
set warns=%warns% -Wno-initializer-overrides -Wno-missing-field-initializers -Wno-incompatible-pointer-types

REM Useful Microsoft/GNU extension
set warns=%warns% -Wno-microsoft-enum-forward-reference -Wno-microsoft-string-literal-from-predefined -Wno-gnu-null-pointer-arithmetic

REM Ocassionally, I like to turn on -Wunused-function
set warns=%warns% -Wno-unused-function

set code=%cd%
set links=-incremental:no -lWinmm.lib -lUserenv.lib -lAdvapi32.lib -lUser32.lib -lGdi32.lib -lDwmapi.lib

IF NOT EXIST build mkdir build
pushd build

del *.pdb > NUL 2> NUL
del *.exe > NUL 2> NUL
del *.dll > NUL 2> NUL

set opts=-fno-exceptions -fno-cxx-exceptions -fno-async-exceptions -fno-rtti -fno-rtti-data -g3 -fms-compatibility -fms-extensions
REM set opts=%opts% -fsanitize=address
set opts=%opts% -I%code%\code -I%code%\code\dependencies fast_float.o
if [%1]==[release] set opts=%opts% -O2

set lib_opts=-fno-rtti -fno-exceptions -fno-cxx-exceptions -fno-async-exceptions -O2 -c
IF NOT EXIST "fast_float.o" clang++ %lib_opts% %code%\code\dependencies\fast_float.cpp

clang %opts% %warns% %code%\code\examples\test_base.c -otest_base.exe --for-linker %links%
clang %opts% %warns% %code%\code\examples\demo_gfx.c  -odemo_gfx.exe --for-linker %links% -Wno-null-pointer-subtraction
clang %opts% %warns% %code%\code\examples\demo.c      -odemo.exe --for-linker %links%
clang %opts% %warns% %code%\code\examples\parser.c    -oparser.exe -Wno-parentheses -Wno-unused-but-set-variable --for-linker %links%
clang %opts% %warns% %code%\code\examples\TestDLL.c   -oTestDLL.dll --for-linker -DLL

ren fast_float.o fast_float.temp > NUL 2> NUL
del *.obj > NUL 2> NUL
ren fast_float.temp fast_float.o > NUL 2> NUL 

del *.ilk > NUL 2> NUL
del *.exp > NUL 2> NUL
del *.lib > NUL 2> NUL
del *.xml > NUL 2> NUL

popd
