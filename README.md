# GL adapter for 3DEXPERIENCE on Parallels

**Run 3DEXPERIENCE (CATIA, SOLIDWORKS 3DEXPERIENCE apps) on an Apple silicon Mac, on the Mac's GPU,
inside a Parallels Windows 11 on ARM virtual machine.**

Out of the box, 3DEXPERIENCE R2024x either crashes on Parallels' graphics driver or shows a model
whose shaded faces flash for a split second and then vanish, leaving you with a very expensive
wireframe viewer. The usual fallback, software rendering with Mesa, works but runs at
slideshow speed.

This project is a single replacement `opengl32.dll` that sits between 3DEXPERIENCE and the
Parallels OpenGL driver and quietly fixes everything that goes wrong in between. The result:
3DEXPERIENCE renders on the Mac's GPU, faces stay where they belong, rotating a part feels normal.

**Using VMPal instead of Parallels?** You don't need the adapter at all, just a recent enough
Windows build and two settings. See [VMPal](#vmpal) below; there is a separate one-click setup
for it.

> Not affiliated with, endorsed by, or supported by Dassault Systèmes or Parallels.

## Disclaimer (please read, it's short and mildly entertaining)

This adapter was **vibe coded**: one student, one AI pair programmer (Claude), one weekend, and
an unreasonable number of frame captures of a part that refused to be shaded. It works on the
machine it was built on. It might work on yours. It was not written by a graphics driver team,
it has no test lab, and its QA department is a single Mac Studio.

**Maintenance plan:** this repository will be actively maintained right up until its author no
longer needs CATIA, which is to say roughly the moment the last CAD exam is handed in. After that
it enters its retirement phase: no updates, issues answered at geological speed, and a firm belief
that Parallels or Dassault will have fixed everything by then anyway. Pull requests are welcome
and will be read with gratitude, possibly years later.

Use at your own risk. Keep backups of anything you care about. If it breaks your CAD homework,
the adapter will be sincerely sorry, but it will not do the homework for you.

## What it does

The adapter forwards every OpenGL call to the Parallels driver and only steps in where something
breaks:

| What goes wrong on the Parallels driver | What the adapter does |
|---|---|
| 3DEXPERIENCE crashes on the unknown GPU vendor name | Reports the vendor as `INTEL` |
| `ARB_buffer_storage` and some `EXT_direct_state_access` functions are missing | Emulates them |
| Buffer names that were never bound are rejected | Binds them once before use |
| `glMultiDraw*Indirect` only draws the first command | Splits it into single draws |
| `glTexBufferRange` ignores offset and size | Emulates ranged buffer textures |
| Copying depth out of a multisample depth texture fails | Copies the depth with a small shader |
| The occlusion-culling result is never written back, so every part counts as hidden (the "faces flash and vanish" bug) | Reports "all visible" for that read-back |
| "No certified OpenGL library has been found" message box at every start | Clicks OK for you |

The occlusion workaround means 3DEXPERIENCE always draws all parts instead of skipping hidden ones.
For student-sized parts and assemblies you will not notice. For a whole aircraft you might.

## Tested with

- Parallels Desktop 27.1, Windows 11 on ARM (build 26300), Mac Studio (Apple M5 Ultra)
- 3DEXPERIENCE R2024x (B426, FP2528), SOLIDWORKS profile, OpenGL 4.3 renderer
- Window mode (Coherence mode should behave the same, but is less tested)
- VMPal (Windows 11 on ARM build 26300, OpenGL 4.1 Mesa driver): works **without** the adapter,
  see [VMPal](#vmpal)

Anything else is unexplored territory. VMware Fusion and UTM offer DirectX but, as far as known,
no usable OpenGL to Windows on ARM guests, so neither the adapter nor 3DEXPERIENCE's OpenGL
renderer is expected to work there.

## Install

1. Install 3DEXPERIENCE in the Windows VM as usual, and close it.
2. Download `GLAdapter-3DX-<version>.zip` from the
   [Releases](../../releases) page and extract it inside Windows. If Windows complains about a
   downloaded file, right-click the zip, choose Properties and tick **Unblock** first.
3. Double-click `Install.cmd` and allow administrator rights.
4. The installer finds the 3DEXPERIENCE folder (`...\Dassault Systemes\B4xx\win_b64\code\bin`)
   by itself; otherwise use **Browse...**. Leave the checkbox ticked and click **Install**.
5. Start 3DEXPERIENCE as usual.

An existing `opengl32.dll` in that folder (for example Mesa) is kept as a backup.
**Uninstall:** run `Install.cmd` again and click **Uninstall**; the backup is restored.
A 3DEXPERIENCE update or repair may remove the adapter; just install it again.

### Without the GUI

Administrator PowerShell, in the extracted folder (adjust `B426` to your release):

```powershell
$bin = "C:\Program Files\Dassault Systemes\B426\win_b64\code\bin"
if (Test-Path "$bin\opengl32.dll") { Rename-Item "$bin\opengl32.dll" "opengl32.dll.before-gladapter" }
Copy-Item .\opengl32.dll "$bin\opengl32.dll"
[Environment]::SetEnvironmentVariable("CATForceNotCertifiedGraphics", "1", "User")   # optional
```

Uninstall:

```powershell
$bin = "C:\Program Files\Dassault Systemes\B426\win_b64\code\bin"
Remove-Item "$bin\opengl32.dll"
if (Test-Path "$bin\opengl32.dll.before-gladapter") { Rename-Item "$bin\opengl32.dll.before-gladapter" "opengl32.dll" }
[Environment]::SetEnvironmentVariable("CATForceNotCertifiedGraphics", $null, "User")
```

The installer can also be scripted: `Installer.ps1 -Action list | status | install | uninstall -Path <bin folder>`.

`CATForceNotCertifiedGraphics=1` is Dassault's own switch that hides the "graphics driver should
be upgraded" warning.

## VMPal

[VMPal](https://vmpal.com) gives Windows on ARM a Mesa-based OpenGL 4.1 driver ("virgl").
3DEXPERIENCE then uses its older OpenGL 4.0 renderer, and none of the Parallels driver bugs above
apply. The adapter is not needed there; a test run with it showed none of its fixes being used.

What VMPal does need:

| Requirement | Why |
|---|---|
| Windows 11 build **26100.6725 or newer** (24H2 October 2025 update, or 25H2/26H2) | 3DEXPERIENCE's `Renderer.dll` uses AVX instructions without checking for them. Only these builds let x64 emulation (Prism) run AVX; older ones crash with "Illegal instruction" (0xC000001D) right after login |
| `GALLIUM_OVERRIDE_CPU_CAPS=sse4.1` (user variable) | Stops Mesa from generating AVX code of its own |
| `DRAW_USE_LLVM=0` (user variable) | Switches off another Mesa code generator. Set as a precaution; not proven necessary |

**Setup:** download `VMPal-Setup-<version>.zip` from the [Releases](../../releases) page, extract it
in Windows and double-click `VMPal-Setup.cmd`. It checks the Windows build, sets the variables
(plus `CATForceNotCertifiedGraphics=1`) and disables the GL adapter if it is installed.
`VMPal-Setup.cmd -Action check` only reports, `VMPal-Setup.cmd -Action remove` undoes it.

Or by hand, in a normal command prompt:

```
setx GALLIUM_OVERRIDE_CPU_CAPS sse4.1
setx DRAW_USE_LLVM 0
setx CATForceNotCertifiedGraphics 1
```

**Updating Windows in VMPal:** VMPal's Windows image was seen refusing the cumulative update
(error 0x800f0993, `PSFX_E_REBASE_HYDRATION_CANDIDATES_MISSING`), so Windows Update alone could not
bring it to an AVX-capable build. An in-place upgrade worked: download the Windows 11 ARM64 ISO
from [Microsoft](https://www.microsoft.com/en-us/software-download/windows11arm64), double-click it in
the VM, run `setup.exe` and choose **Keep personal files and apps**. Installed programs, including
3DEXPERIENCE, survive. Take a VMPal snapshot first. A VPN client may need reinstalling afterwards.

Known so far: one crash inside VMPal's graphics driver on the very first part load, which did not
come back. If it does, `SHIM_DEBUG=1` with the adapter installed logs the exact call (see below).

## Good to know

- **Do not** install Microsoft's "OpenCL, OpenGL and Vulkan Compatibility Pack" in the VM.
  3DEXPERIENCE crashes at startup with it.
- **Do not** try to hide `GL_ARB_buffer_storage` (`SHIM_HIDE_EXT`). 3DEXPERIENCE crashes without it.
- "Illegal instruction" / 0xC000001D right after login: Windows is too old to emulate AVX, see
  [VMPal](#vmpal) for the build numbers. This applies to any virtualization app.
- Windows Explorer's built-in unzip has been seen to stop silently partway through the very large
  3DEXPERIENCE installer zip, which then fails with "OpenZip: 0x80030002". Extract it with
  `tar -xf <zip> -C <folder>` or 7-Zip instead.
- If your organisation's VPN client (for example Cisco Secure Client) runs on the Mac, it may block
  the Mac's network after you disconnect it. Toggle its network extension in System Settings >
  General > Login Items & Extensions, or restart the Mac.

## Settings and troubleshooting

All optional, set as user environment variables in Windows:

| Variable | Effect |
|---|---|
| `SHIM_DEBUG=1` | Writes `%LOCALAPPDATA%\GLAdapter\shim.log`, enables frame captures and logs crashes (faulting module, offset, instruction bytes, the last 192 OpenGL calls). Slower |
| `SHIM_GLDEBUG=1` | With `SHIM_DEBUG=1`: also logs the driver's own debug messages |
| `SHIM_OCC_READBACK=real` | Turns the occlusion workaround off (faces vanish again; for testing) |
| `SHIM_GL_VENDOR=<text>` | Reports a different GPU vendor than `INTEL` |

**Frame capture** (with `SHIM_DEBUG=1`): write a number, e.g. `3`, into
`%LOCALAPPDATA%\GLAdapter\capture.now`; the next frames land in `%LOCALAPPDATA%\GLAdapter\cap\`
with per-draw state, thumbnails and shader sources. A line `calls` in
`%LOCALAPPDATA%\GLAdapter\ctl.txt` adds a full GL call trace. All switches are listed at the top of
`src/capture.c`. Captures contain 3DEXPERIENCE's shader code, so do not publish them.

## How it was found

The short version: frame captures showed the shaded faces being drawn in the first frames and
then skipped. 3DEXPERIENCE decides which parts to draw with GPU occlusion culling. The depth it
tested against was empty (Parallels cannot copy depth out of multisample depth textures), its
bounding boxes read wrong matrices (ranged buffer textures ignore their range), and even after both
were fixed the result never made it back to the CPU, which kept concluding that nothing was
visible. Three bugs stacked on top of each other, each one hiding the next.

`tests/gltest2.c` is the offline probe that pinned the driver bugs down: about 140 small OpenGL
render tests. Run it in the VM to see what the driver does on its own, or put the adapter's
`opengl32.dll` next to it to test through the adapter. Some tests fail on the raw driver on purpose;
those are the bugs listed above.

## Build

Needs [Zig](https://ziglang.org) (tested with 0.17) and Python 3, on macOS, Linux or Windows:

```sh
./build.sh          # macOS / Linux
build.cmd           # Windows
```

Output in `dist/`: `opengl32.dll` (x64), `gltest2.exe` and the installer files. `src/gen.py`
generates the export thunks (`thunks.S`, `names.h`, `opengl32.def`) from `src/exports.txt`.

## License

MIT, see [LICENSE](LICENSE). "3DEXPERIENCE", "CATIA" and "SOLIDWORKS" are trademarks of Dassault
Systèmes; "Parallels" is a trademark of Parallels International GmbH. They are named here only to
describe compatibility.
