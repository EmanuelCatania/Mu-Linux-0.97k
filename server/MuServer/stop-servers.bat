@echo off
cd /d "%~dp0"

taskkill /F /T /IM GameServer.exe >nul 2>&1
taskkill /F /T /IM ConnectServer.exe >nul 2>&1
taskkill /F /T /IM JoinServer.exe >nul 2>&1
taskkill /F /T /IM DataServer.exe >nul 2>&1

echo Server processes stopped.
