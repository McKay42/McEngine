@echo off
setlocal enableextensions enabledelayedexpansion

set NAME=McEngine
set BUILD=Windows Release

set WRK=build



set FULLPATH=%~dp0
echo FULLPATH = %FULLPATH%

cd %FULLPATH%%WRK%
".\..\%BUILD%\%NAME%"