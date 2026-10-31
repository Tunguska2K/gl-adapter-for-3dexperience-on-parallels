#!/bin/sh
# Builds dist/opengl32.dll (x64) and dist/gltest2.exe with Zig (https://ziglang.org) and Python 3.
# Works on macOS, Linux and Windows (Git Bash). Set ZIG=/path/to/zig if zig is not on PATH.
set -e
cd "$(dirname "$0")"
ZIG="${ZIG:-zig}"
mkdir -p dist
python3 -I src/gen.py
"$ZIG" cc -target x86_64-windows-gnu -shared -O2 \
  -Wall -Wextra -Wno-unused-parameter -Wno-inconsistent-dllimport -Wno-cast-function-type -Wno-unused-function \
  -o dist/opengl32.dll src/shim.c src/thunks.S src/opengl32.def -lkernel32 -luser32
"$ZIG" cc -target x86_64-windows-gnu -O1 -Wall -Wno-unused-variable -Wno-unused-function \
  -o dist/gltest2.exe tests/gltest2.c -lopengl32 -lgdi32 -luser32
rm -f dist/*.pdb dist/*.lib
cp installer/Install.cmd installer/Installer.ps1 dist/
echo "built: dist/opengl32.dll dist/gltest2.exe (+ installer)"
