@echo off
call "D:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" x64 >nul
if errorlevel 1 exit /b 1
cd /d "%~dp0.."
cl /nologo /utf-8 /W4 /WX /TC /Z7 /Iinclude tests\test_slow.c supervisor\llc_slow_task.c /Febin\x64\test_slow.exe /Fobin\x64\ /link /INCREMENTAL:NO
if errorlevel 1 exit /b 1
bin\x64\test_slow.exe
