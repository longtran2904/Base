@echo off
for /L %%i in (1,1,3) do (
    D:\Programs\RAMMap -Et & timeout /t 5 > NUL & build\glob -r -d -f:32 foo & timeout /t 5 > NUL
    D:\Programs\RAMMap -Et & timeout /t 5 > NUL &   glob_seq -r -d -f:32 foo & timeout /t 5 > NUL
)

for /L %%i in (1,1,3) do (
    D:\Programs\RAMMap -Et & timeout /t 5 > NUL & build\glob -r -d -f:64 foo & timeout /t 5 > NUL
    D:\Programs\RAMMap -Et & timeout /t 5 > NUL &   glob_seq -r -d -f:64 foo & timeout /t 5 > NUL
)

for /L %%i in (1,1,3) do (
    D:\Programs\RAMMap -Et & timeout /t 5 > NUL & build\glob -r -d -f:128 foo & timeout /t 5 > NUL
    D:\Programs\RAMMap -Et & timeout /t 5 > NUL &   glob_seq -r -d -f:128 foo & timeout /t 5 > NUL
)

for /L %%i in (1,1,3) do (
    D:\Programs\RAMMap -Et & timeout /t 5 > NUL & build\glob -r -d -f:256 foo & timeout /t 5 > NUL
    D:\Programs\RAMMap -Et & timeout /t 5 > NUL &   glob_seq -r -d -f:256 foo & timeout /t 5 > NUL
)

for /L %%i in (1,1,3) do (
    D:\Programs\RAMMap -Et & timeout /t 5 > NUL & build\glob -r -d -f:512 foo & timeout /t 5 > NUL
    D:\Programs\RAMMap -Et & timeout /t 5 > NUL &   glob_seq -r -d -f:512 foo & timeout /t 5 > NUL
)

for /L %%i in (1,1,3) do (
    D:\Programs\RAMMap -Et & timeout /t 5 > NUL & build\glob -r -d -f:1024 foo & timeout /t 5 > NUL
    D:\Programs\RAMMap -Et & timeout /t 5 > NUL &   glob_seq -r -d -f:1024 foo & timeout /t 5 > NUL
)

for /L %%i in (1,1,3) do (
    D:\Programs\RAMMap -Et & timeout /t 5 > NUL & build\glob -r -d -f:2056 foo & timeout /t 5 > NUL
    D:\Programs\RAMMap -Et & timeout /t 5 > NUL &   glob_seq -r -d -f:2056 foo & timeout /t 5 > NUL
)

for /L %%i in (1,1,3) do (
    D:\Programs\RAMMap -Et & timeout /t 5 > NUL & build\glob -r -d -f:4096 foo & timeout /t 5 > NUL
    D:\Programs\RAMMap -Et & timeout /t 5 > NUL &   glob_seq -r -d -f:4096 foo & timeout /t 5 > NUL
)

for /L %%i in (1,1,3) do (
    D:\Programs\RAMMap -Et & timeout /t 5 > NUL & build\glob -r -d -f:8192 foo & timeout /t 5 > NUL
    D:\Programs\RAMMap -Et & timeout /t 5 > NUL &   glob_seq -r -d -f:8192 foo & timeout /t 5 > NUL
)
