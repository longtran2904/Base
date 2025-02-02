@echo off
cd /d %~dp0
echo --[SMALL FETCH]--
for /L %%i in (1,1,5) do D:\Programs\RAMMap.exe -Et && D:\Programs\RAMMap.exe -Et && glob %*
echo --[BIG FETCH]--
for /L %%i in (1,1,5) do D:\Programs\RAMMap.exe -Et && D:\Programs\RAMMap.exe -Et && build\glob %*