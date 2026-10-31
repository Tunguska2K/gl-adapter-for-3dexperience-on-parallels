@echo off
rem Builds dist\opengl32.dll (x64) and dist\gltest2.exe with Zig and Python 3 on Windows.
rem Needs zig.exe and python on PATH (or set ZIG=C:\path\to\zig.exe).
setlocal
cd /d "%~dp0"
if "%ZIG%"=="" set ZIG=zig
if not exist dist mkdir dist
python -I src\gen.py || exit /b 1
"%ZIG%" cc -target x86_64-windows-gnu -shared -O2 -Wall -Wextra -Wno-unused-parameter -Wno-inconsistent-dllimport -Wno-cast-function-type -Wno-unused-function -o dist\opengl32.dll src\shim.c src\thunks.S src\opengl32.def -lkernel32 -luser32 || exit /b 1
"%ZIG%" cc -target x86_64-windows-gnu -O1 -Wall -Wno-unused-variable -Wno-unused-function -o dist\gltest2.exe tests\gltest2.c -lopengl32 -lgdi32 -luser32 || exit /b 1
del /q dist\*.pdb dist\*.lib 2>nul
copy /y installer\Install.cmd dist\ >nul
copy /y installer\Installer.ps1 dist\ >nul
echo built: dist\opengl32.dll dist\gltest2.exe (+ installer)
