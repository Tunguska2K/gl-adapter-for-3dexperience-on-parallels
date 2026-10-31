<#
3DEXPERIENCE in a VMPal Windows 11 on ARM VM: one-time setup.

VMPal's graphics driver (Mesa "virgl", OpenGL 4.1) runs 3DEXPERIENCE WITHOUT the GL adapter.
What it needs instead:
 1. A Windows build whose x64 emulation (Prism) runs AVX instructions: 26100.6725 or newer.
    3DEXPERIENCE's Renderer.dll uses AVX unconditionally; older builds crash with
    "Illegal instruction" (0xC000001D).
 2. GALLIUM_OVERRIDE_CPU_CAPS=sse4.1 (Mesa must not generate AVX code itself) and
    DRAW_USE_LLVM=0 (turns off another Mesa code generator; set as a precaution).
 3. Optional: CATForceNotCertifiedGraphics=1 hides Dassault's "driver not certified" warning.
This script checks 1, sets 2 and 3 for the current user, and disables the GL adapter if it is
installed (it is not needed on VMPal). Close 3DEXPERIENCE first.

Usage: double-click VMPal-Setup.cmd           (apply)
       VMPal-Setup.cmd -Action check          (only report)
       VMPal-Setup.cmd -Action remove         (undo: remove the variables, re-enable the adapter)
#>
param([ValidateSet('apply', 'check', 'remove')][string]$Action = 'apply', [switch]$NoPause, [switch]$Elevated)
$ErrorActionPreference = 'Stop'
$Vars = [ordered]@{ GALLIUM_OVERRIDE_CPU_CAPS = 'sse4.1'; DRAW_USE_LLVM = '0'; CATForceNotCertifiedGraphics = '1' }
$Marker = 'GLADAPTER-3DX-MARKER'

function Done { if (-not $NoPause) { Write-Host ''; Read-Host 'Press Enter to close' | Out-Null } }
function Test-Admin { (New-Object Security.Principal.WindowsPrincipal([Security.Principal.WindowsIdentity]::GetCurrent())).IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator) }
function Find-Bins {
    $r = @()
    foreach ($root in @($env:ProgramFiles, $env:ProgramW6432) | Select-Object -Unique) {
        if (-not $root) { continue }
        $ds = Join-Path $root 'Dassault Systemes'
        if (-not (Test-Path $ds)) { continue }
        foreach ($d in Get-ChildItem $ds -Directory -ErrorAction SilentlyContinue) {
            $b = Join-Path $d.FullName 'win_b64\code\bin'
            if (Test-Path (Join-Path $b '3DEXPERIENCE.exe')) { $r += $b }
        }
    }
    $r | Select-Object -Unique
}
function Is-Adapter([string]$f) { (Test-Path $f) -and ([Text.Encoding]::ASCII.GetString([IO.File]::ReadAllBytes($f)).Contains($Marker)) }

# the adapter rename needs administrator rights; only elevate when there is something to rename
function Adapter-Work([bool]$enable) {
    foreach ($b in Find-Bins) {
        $on = Join-Path $b 'opengl32.dll'; $off = Join-Path $b 'opengl32.dll.off'
        if (-not $enable -and (Is-Adapter $on)) {
            if (-not (Test-Admin)) { return 'elevate' }
            if (Test-Path $off) { Remove-Item $off -Force }
            Move-Item $on $off
            Write-Host "GL adapter disabled (not needed on VMPal): $off"
        } elseif ($enable -and (Test-Path $off) -and -not (Test-Path $on)) {
            if (-not (Test-Admin)) { return 'elevate' }
            Move-Item $off $on
            Write-Host "GL adapter re-enabled: $on"
        }
    }
    return 'ok'
}

try {
    if (Get-Process 3DEXPERIENCE -ErrorAction SilentlyContinue) { throw 'Close 3DEXPERIENCE first.' }

    # 1. Windows build
    $v = Get-ItemProperty 'HKLM:\SOFTWARE\Microsoft\Windows NT\CurrentVersion'
    $build = [int]$v.CurrentBuild; $ubr = [int]$v.UBR
    $avxOk = ($build -gt 26100) -or ($build -eq 26100 -and $ubr -ge 6725)
    Write-Host ("Windows build {0}.{1} ({2}): {3}" -f $build, $ubr, $v.DisplayVersion, $(if ($avxOk) { 'OK, emulates AVX' } else { 'TOO OLD, no AVX emulation' }))
    if (-not $avxOk) {
        Write-Host '  3DEXPERIENCE will crash with "Illegal instruction" until Windows is updated:'
        Write-Host '  - Settings > Windows Update, install everything (needs 26100.6725 or newer).'
        Write-Host '  - If the update fails (e.g. 0x800f0993, VMPal''s image cannot take it): download the'
        Write-Host '    Windows 11 ARM64 ISO from https://www.microsoft.com/en-us/software-download/windows11arm64 ,'
        Write-Host '    double-click it in Windows, run setup.exe and choose "Keep personal files and apps".'
    }
    $gpu = (Get-CimInstance Win32_VideoController | ForEach-Object { $_.Name }) -join ', '
    Write-Host "Graphics: $gpu"
    $bins = @(Find-Bins)
    Write-Host ('3DEXPERIENCE: ' + $(if ($bins) { $bins -join ', ' } else { 'not found (install it, then run this again)' }))

    if ($Action -eq 'check') {
        foreach ($k in $Vars.Keys) { Write-Host ("  {0} = {1}" -f $k, [Environment]::GetEnvironmentVariable($k, 'User')) }
        foreach ($b in $bins) { Write-Host ('  GL adapter in ' + $b + ': ' + $(if (Is-Adapter (Join-Path $b 'opengl32.dll')) { 'ACTIVE (not needed on VMPal)' } elseif (Test-Path (Join-Path $b 'opengl32.dll.off')) { 'disabled' } else { 'not installed' })) }
        Done; exit 0
    }

    if (-not $Elevated) {
        # 2./3. user variables (run as the normal user, so they land in the right account)
        foreach ($k in $Vars.Keys) {
            if ($Action -eq 'apply') { [Environment]::SetEnvironmentVariable($k, $Vars[$k], 'User'); Write-Host "Set $k=$($Vars[$k])" }
            else { [Environment]::SetEnvironmentVariable($k, $null, 'User'); Write-Host "Removed $k" }
        }
    }

    # 4. GL adapter off (apply) / back on (remove)
    if ((Adapter-Work ($Action -eq 'remove')) -eq 'elevate') {
        Write-Host 'Asking for administrator rights to rename the GL adapter...'
        $a = @('-NoProfile', '-ExecutionPolicy', 'Bypass', '-File', "`"$PSCommandPath`"", '-Action', $Action, '-Elevated')
        Start-Process powershell.exe -Verb RunAs -ArgumentList $a -Wait
    }
    Write-Host ''
    if ($Action -eq 'apply') { Write-Host 'Done. Start 3DEXPERIENCE again (programs started before this keep the old settings).' }
    else { Write-Host 'Undone.' }
} catch {
    Write-Host "ERROR: $($_.Exception.Message)"
}
Done
