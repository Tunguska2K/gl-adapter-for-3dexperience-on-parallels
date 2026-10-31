3DEXPERIENCE in VMPal (Windows 11 on ARM, Apple silicon Mac)
=============================================================

In VMPal, 3DEXPERIENCE R2024x runs on VMPal's own OpenGL 4.1 driver. The GL adapter from this
project is NOT needed there. What is needed:

 1. Windows 11 build 26100.6725 or newer (24H2 October 2025 update, or 25H2/26H2).
    3DEXPERIENCE uses AVX instructions; only these builds can emulate them. Older builds crash
    with "Illegal instruction" (0xC000001D) right after login.
 2. User variables GALLIUM_OVERRIDE_CPU_CAPS=sse4.1 and DRAW_USE_LLVM=0.

Run once, with 3DEXPERIENCE closed:
   double-click VMPal-Setup.cmd
It checks the Windows build, sets the variables (plus CATForceNotCertifiedGraphics=1, which
hides Dassault's "driver not certified" warning) and disables the GL adapter if it is installed.
   VMPal-Setup.cmd -Action check     only reports
   VMPal-Setup.cmd -Action remove    undoes everything

If Windows is too old and Windows Update fails (seen: error 0x800f0993), do an in-place upgrade:
download the Windows 11 ARM64 ISO from
   https://www.microsoft.com/en-us/software-download/windows11arm64
double-click it in Windows, run setup.exe, choose "Keep personal files and apps".
Take a VMPal snapshot first. A VPN client may need reinstalling afterwards.

More: https://github.com/Tunguska2K/gl-adapter-for-3dexperience-on-parallels
