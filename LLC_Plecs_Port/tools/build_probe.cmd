@echo off
call "D:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" x64 >nul
if errorlevel 1 exit /b 1
cd /d "%~dp0.."
cl /nologo /W4 /WX /TC /LD /Ivendor\plecs adapters\plecs_probe.c /Febin\x64\LLC_Probe.dll /Fobin\x64\ /link /INCREMENTAL:NO
if errorlevel 1 exit /b 1
dumpbin /exports bin\x64\LLC_Probe.dll
