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

> Not affiliated with, endorsed by, or supported by Dassault Systèmes or Parallels.

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

- Parallels Desktop 27.1, Windows 11 on ARM, Mac Studio (Apple M5 Ultra)
- 3DEXPERIENCE R2024x (B426, FP2528), SOLIDWORKS profile, OpenGL 4.3 renderer
- Window mode (Coherence mode should behave the same, but is less tested)

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

## Good to know

- **Do not** install Microsoft's "OpenCL, OpenGL and Vulkan Compatibility Pack" in the VM.
  3DEXPERIENCE crashes at startup with it.
- **Do not** try to hide `GL_ARB_buffer_storage` (`SHIM_HIDE_EXT`). 3DEXPERIENCE crashes without it.

## Settings and troubleshooting

All optional, set as user environment variables in Windows:

| Variable | Effect |
|---|---|
| `SHIM_DEBUG=1` | Writes `%LOCALAPPDATA%\GLAdapter\shim.log` and enables frame captures. Slower |
| `SHIM_GLDEBUG=1` | With `SHIM_DEBUG=1`: also logs the driver's own debug messages |
| `SHIM_OCC_READBACK=real` | Turns the occlusion workaround off (faces vanish again; for testing) |
| `SHIM_GL_VENDOR=<text>` | Reports a different GPU vendor than `INTEL` |

**Frame capture** (with `SHIM_DEBUG=1`): write a number, e.g. `3`, into
`%LOCALAPPDATA%\GLAdapter\capture.now`; the next frames land in `%LOCALAPPDATA%\GLAdapter\cap\`
with per-draw state, thumbnails and shader sources. A line `calls` in
`%LOCALAPPDATA%\GLAdapter\ctl.txt` adds a full GL call trace. All switches are listed at the top of
`src/capture.c`. Captures contain 3DEXPERIENCE's shader code, so do not publish them.

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
