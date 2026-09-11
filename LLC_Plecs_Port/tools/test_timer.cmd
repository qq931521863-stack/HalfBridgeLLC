@echo off
call "D:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" x64 >nul
if errorlevel 1 exit /b 1
cd /d "%~dp0.."
cl /nologo /W4 /WX /TC /Iinclude tests\test_timer.c core\llc_timer_image.c /Febin\x64\test_timer.exe /Fobin\x64\ /link /INCREMENTAL:NO
if errorlevel 1 exit /b 1
bin\x64\test_timer.exe
