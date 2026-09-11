@echo off
call "D:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" x64 >nul
if errorlevel 1 exit /b 1
cd /d "%~dp0.."
cl /nologo /utf-8 /TC /c build\cscript_checks\*.c /Fobuild\cscript_checks\
exit /b %errorlevel%
