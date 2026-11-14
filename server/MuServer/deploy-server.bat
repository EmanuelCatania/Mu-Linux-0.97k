@echo off
cd /d "%~dp0"

copy /Y ..\build\msys2-mingw-x64\bin\ConnectServer\ConnectServer.exe ConnectServer\
copy /Y ..\build\msys2-mingw-x64\bin\GameServer\GameServer.exe GameServer\
copy /Y ..\build\msys2-mingw-x64\bin\JoinServer\JoinServer.exe MySQL\JoinServer\
copy /Y ..\build\msys2-mingw-x64\bin\DataServer\DataServer.exe MySQL\DataServer\