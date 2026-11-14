@echo off
cd /d "%~dp0"

start "" /D "%~dp0MySQL\DataServer" "DataServer.exe"
sleep 0.5
start "" /D "%~dp0MySQL\JoinServer" "JoinServer.exe"
sleep 0.5
start "" /D "%~dp0ConnectServer" "ConnectServer.exe"
sleep 0.5
start "" /D "%~dp0GameServer" "GameServer.exe"